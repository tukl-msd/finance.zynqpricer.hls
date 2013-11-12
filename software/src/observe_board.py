"""
Copyright (C) 2013 University of Kaiserslautern
Microelectronic Systems Design Research Group

Christian Brugger (brugger@eit.uni-kl.de)
11. November 2013
"""

import sys
import subprocess
import shlex
import time
import json
import threading
import os
import math

from PySide.QtCore import *
from PySide.QtGui import *

import numpy as np

import matplotlib
matplotlib.use('Qt4Agg')
matplotlib.rcParams['backend.qt4']='PySide'
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
from matplotlib.animation import Animation


remote_encoding = 'latin1'

DEBUG_REMOTE = False
CMDs = ["cat bitstream/heston_sl_6x.bin > /dev/xdevcfg",
        "sudo software/bin/init_rng bitstream/heston_sl_6x.json",
        "sudo taskset -c 1 software/bin/run_heston -sl -acc "
            "software/parameters/params_zynq_demo_acc.json "
            "bitstream/heston_sl_6x.json -observe"]

BITSTREAM_DIR = "..\\bitstream"



def line_reader(p):
    """ generator to read line by line from p"""
    line = []
    while p.poll() is None:
        r = p.stdout.read(1).decode(remote_encoding)
        if DEBUG_REMOTE:
            print(r, end='')
        if r == '\n':
            yield ''.join(line)
            line = []
        else:
            line.append(r)
    yield ''.join(line) + p.stdout.read().decode(remote_encoding)


class RemoteObserver(QThread):
    """ 
    Thread that starts remote processes and generates events.

    Logs into FPGA over SSH or COM port and executes programs.
    Then collects events and generates Qt signals that can be
    processed by the Gui.
    """
    event = Signal(str, str, str)

    def __init__(self, pty_cmd, root_dir):
        super().__init__()
        self.pty_cmd = pty_cmd
        self.root_dir = root_dir
        self._p = None


    def _send_cmd(self, msg):
        self._p.stdin.write((msg + '\n').encode(remote_encoding))


    def run(self):
        self._p = subprocess.Popen(shlex.split(self.pty_cmd), 
                stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
                stderr=sys.stderr)
        reader = line_reader(self._p)
        # wait for welcome message
        for line in reader:
            if line.startswith('Last login:'):
                break

        self._send_cmd("cd {}".format(shlex.quote(self.root_dir)))
        for cmd in CMDs:
            self._send_cmd(cmd)
        self._send_cmd("exit")
    
        for line in reader:
            try:
                data = json.loads(line)
            except ValueError:
                pass
            else:
                if data["__class__"] == "observer":
                    self.event.emit(data["__event__"], 
                            json.dumps(data["__value__"]),
                            data.get("__instance__", None))


class AccPanel(QFrame):
    """ Widget visulizing accelerator behaviour as matplotlib graph """
    def __init__(self, name):
        super().__init__()
        self._config = None
        self._name = name
        self._poly = None
        self._progress = None
        self._dirty = False # new poly but not yet redrawn
        self.setFrameStyle(QFrame.Box)
        self._t = time.time()

    def minimumSizeHint(self):
        return QSize(100, 100)

    def set_config(self, config):
        self._config = config
        self._poly = None
        self._progress = None
        self.update()

    def get_heston_path(self):
        heston = self._config['heston']
        stock = [math.log(heston['spot_price'])]
        vola = [heston['vola_0']]
        step_cnt = min(self.width(), self._config['simulation_sl']['step_cnt'])
        step_size = heston['time_to_maturity'] / step_cnt
        for _ in range(step_cnt):
            max_vola = max(0, vola[-1])
            sqrt_vola = np.sqrt(max_vola)
            w_0 = np.random.normal(0, 1)
            r = heston['correlation']
            w_1 = w_0 * r + np.random.normal(0, 1) * math.sqrt(1-r**2)
            stock.append(stock[-1] + 
                    (heston['riskless_rate'] - 0.5 * max_vola) *
                    step_size + math.sqrt(step_size) * sqrt_vola * w_0)
            vola.append(vola[-1] + heston['reversion_rate'] * step_size *
                    (heston['long_term_avg_vola'] - max_vola) + 
                    heston['vol_of_vol'] * math.sqrt(step_size) * 
                    sqrt_vola * w_1)
        return np.exp(np.array(stock)) # TODO: payoff

    def get_plot_width(self):
        return self.width() - 70

    def heston_path_to_poly(self, stock):
        t = np.linspace(0, self.get_plot_width(), len(stock))
        s = self.height() - stock
        return QPolygonF(list(map(lambda p: QPointF(*p), zip(t, s))))

    def paintEvent(self, event):
        super().paintEvent(event)
        painter = QPainter(self)

        painter.drawText(QPoint(10, 20), self._name)

        if self._progress is not None:
            painter.setBrush(QBrush(QColor(255, 0, 0)))
            painter.setPen(QPen(QColor(Qt.black), 1))
            y = self.height() * (1 - self._progress)
            painter.drawRect(self.get_plot_width(), y,
                    self.width(), self.height())

        if self._poly is not None:
            painter.setPen(QPen(QColor(Qt.black), 1))
            painter.drawPolyline(self._poly)
        self._dirty = False

    def new_paths(self, new_count):
        self._progress = (new_count / 
                self._config['simulation_sl']['path_cnt'])
        if not self._dirty:
            self._dirty = True
            stock = self.get_heston_path()
            self._poly = self.heston_path_to_poly(stock)

            print(1/(time.time() - self._t))
            self._t = time.time()
        else:
            print("ignoring", self._name, new_count)
        self.update()


class PictureLabel(QLabel):
    """ Widget that is showing scaled image and correct aspect ratio """
    def __init__(self):
        super().__init__()
        self._pixmap = None
        self.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)

    def setPixmap(self, pixmap):
        self._pixmap = pixmap
        self._update()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self._update()

    def _update(self):
        if self._pixmap is not None:
            w, h = self.width(), self.height()
            super().setPixmap(self._pixmap.scaled(w, h, Qt.KeepAspectRatio))


class DevicePanel(QFrame):
    """ Widget showing FPGA floorplan """
    def __init__(self):
        super().__init__()
        #self.setFrameStyle(QFrame.Box)

        layout = QVBoxLayout()
        self._fpga_img = PictureLabel()
        self._fpga_name = QLabel("<empty>")
        self._fpga_name.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
        layout.addWidget(self._fpga_name)
        layout.addWidget(self._fpga_img, stretch=1)
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)
        self.update_fpga_image()

    def minimumSizeHint(self):
        return QSize(20, 20)

    def update_fpga_image(self, filename=None):
        """
        filname - path to fpga config file. 
            Assuming that the config is in  BITSTREAM_DIR and ends with *.json
        """
        if filename is not None:
            basename = os.path.basename(filename)
            path = os.path.join(BITSTREAM_DIR, basename).\
                    replace(".json", "_color.png")
            self._fpga_img.setPixmap(QPixmap(path))
            self._fpga_name.setText(basename)


class Window(QWidget):
    """ main window """
    def __init__(self, observer):
        super().__init__()
        self.observer = observer
        
        self._accelerators = {}
        self._acc_box = QHBoxLayout()

        layout = QVBoxLayout()
        layout.addLayout(self._acc_box, stretch=1)
        self._device_panel = DevicePanel()
        layout.addWidget(self._device_panel, stretch=2)
        self.setLayout(layout)

        self.observer.event.connect(self.on_observer_event)
        self.resize(800, 600)
        self.showMaximized()

    def on_observer_event(self, type, value, instance=None):
        value = json.loads(value)
        if type == "fpga_config":
            self.on_new_fpga_config(value)
        elif type == "setup_sl":
            self.on_setup_sl(instance, value)
        elif type == "new_path":
            self.on_new_paths(instance, value)
        else:
            print("unknown event:", type, value, instance)

    def on_new_fpga_config(self, fpga_config_file):
        self._device_panel.update_fpga_image(fpga_config_file)

        # delete all accelerator widgets
        for i in reversed(range(self._acc_box.count())):
            self._acc_box.itemAt(i).widget().deleteLater()
        self._accelerators = {}

    def on_setup_sl(self, instance, config):
        if instance not in self._accelerators:
            acc = AccPanel(instance)
            self._accelerators[instance] = acc
            self._acc_box.addWidget(acc)
        acc.set_config(config)

    def on_new_paths(self, instance, new_count):
        self._accelerators[instance].new_paths(new_count)





def main():
    if len(sys.argv) != 3:
        print("Usage: {} <pty_cmd> <root_dir>".format(sys.argv[0]))
        sys.exit(1)
    pty_cmd = sys.argv[1]
    root_dir = sys.argv[2]

    observer = RemoteObserver(pty_cmd, root_dir)
    observer.start()

    app = QApplication(sys.argv)
    win = Window(observer)
    win.show()
    ret = app.exec_()
    observer.wait()
    return ret


if __name__ == "__main__":
    sys.exit(main())
