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
import functools
import collections
import queue

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
DEBUG_DRAW_SPEED = False

BITSTREAM_DIR = "..\\bitstream"


class Bitstream:
    """ stores bitstream information """
    def __init__(self, name):
        self.name = name
        self._pixmap_inactive = None
        self._pixmap_active = None
        self._config = None

    def get_configuration_cmd(self):
        return "cat bitstream/{}.bin > /dev/xdevcfg".format(self.name)
    
    def _path_base(self):
        return os.path.join(BITSTREAM_DIR, self.name)

    def get_pixmap(self, active):
        if active:
            if self._pixmap_active is None:
                self._pixmap_active = QPixmap(self._path_base() + '_color.png')
            return self._pixmap_active
        else:
            if self._pixmap_inactive is None:
                self._pixmap_inactive = QPixmap(self._path_base() + '.png')
            return self._pixmap_inactive
    
    def get_config(self):
        if self._config is None:
            with open(self._path_base() + '.json') as f:
                self._config = json.load(f)
        return self._config


COMMAND_BUTTONS = collections.OrderedDict(((
#    "Singlelevel Heston 1", [
#        Bitstream("heston_sl_6x"),
#        "sudo software/bin/init_rng bitstream/heston_sl_6x.json",
#        "sudo taskset -c 1 software/bin/run_heston -sl -acc "
#            "software/parameters/params_zynq_demo_observer_1.json "
#            "bitstream/heston_sl_6x.json -observe"]),(
#    "Multilevel Heston 1", [
#        Bitstream("heston_ml_5x"),
#        "sudo software/bin/init_rng bitstream/heston_ml_5x.json",
#        "sudo taskset -c 1 software/bin/run_heston -ml -acc "
#            "software/parameters/params_zynq_demo_observer_1.json "
#            "bitstream/heston_ml_5x.json -observe"]),(
    "Singlelevel Heston", [
        Bitstream("heston_sl_6x"),
        "sudo software/bin/init_rng bitstream/heston_sl_6x.json",
        "sudo taskset -c 1 software/bin/run_heston -sl -acc "
            "software/parameters/params_zynq_demo_observer_2.json "
            "bitstream/heston_sl_6x.json -observe"]),(
    "Multilevel Heston", [
        Bitstream("heston_ml_5x"),
        "sudo software/bin/init_rng bitstream/heston_ml_5x.json",
        "sudo taskset -c 1 software/bin/run_heston -ml -acc "
            "software/parameters/params_zynq_demo_observer_2.json "
            "bitstream/heston_ml_5x.json -observe"]),(
    "Clear Bitstream", [
        Bitstream("empty")]),(
    "Singlelevel Bitstream", [
        Bitstream("heston_sl_6x")]),(
    "Multilevel Bitstream", [
        Bitstream("heston_ml_5x")])
))


class StreamWriterThread(QThread):
    _timeout = 0.01 # seconds

    def __init__(self, out_stream):
        super().__init__()
        self._q = queue.Queue()
        self._stream = out_stream
        self._stopped = False

    def stop(self):
        self._stopped = True
        self.wait()

    def run(self):
        while not self._stopped:
            try:
                val = self._q.get(True, self._timeout)
                pass
            except queue.Empty:
                pass
            else:
                self._stream.write(val)

    def get_queue(self):
        return self._q


class RemoteObserver(QThread):
    """ 
    Thread that starts remote processes and generates events.

    Logs into FPGA over SSH or COM port and executes programs.
    Then collects events and generates Qt signals that can be
    processed by the Gui.
    """
    event = Signal(str, str, str)
    console_char = Signal(str)
    connected = Signal()

    def __init__(self, pty_cmd, root_dir):
        super().__init__()
        self.pty_cmd = pty_cmd
        self.root_dir = root_dir
        self._p = None
        self._writer_queue = None

    def _line_reader(self, p):
        """ generator to read line by line from p"""
        line = []
        in_color_code = False
        while True:
            r = p.stdout.read(1).decode(remote_encoding)
            if len(r) == 0:
                break
            if DEBUG_REMOTE:
                print(r, end='')
            if in_color_code:
                if r == ';':
                    in_color_code = False
                continue
            elif r == '\x1b':
                in_color_code = True
            elif r == '\r':
                continue
            elif r == '\x07':
                continue
            elif r == '\n':
                yield ''.join(line)
                if len(line) == 0 or line[0] != '{':
                    self.console_char.emit(r)
                line = []
            else:
                line.append(r)
                if line[0] != '{':
                    self.console_char.emit(r)
        if len(line) > 0:
            yield ''.join(line)

    def send_cmd(self, msg):
        """ send unicode command to remote shell (thread-save) """
        self._writer_queue.put((msg + '\n').encode(remote_encoding))

    def close(self):
        #self.send_cmd("exit")
        self._p.terminate()

    def run(self):
        self._p = subprocess.Popen(shlex.split(self.pty_cmd), 
                stdin=subprocess.PIPE, stdout=subprocess.PIPE, 
                stderr=sys.stderr)
        reader = self._line_reader(self._p)

        writer = StreamWriterThread(self._p.stdin)
        writer.start()
        self._writer_queue = writer.get_queue()

        # wait for response
        self.send_cmd("")
        self.send_cmd("")
        for line in reader:
            if '@board' in line:
                break
            else:
                print(line)
        self.send_cmd("cd {}".format(self.root_dir))
        
        self.connected.emit()
    
        for line in reader:
            try:
                data = json.loads(line)
            except ValueError:
                pass
            else:
                if data.get("cls", '') == "observer":
                    self.event.emit(data["evt"], 
                            json.dumps(data["val"]),
                            data.get("inst", None))
        writer.stop()


class AccDrawData:
    """ collect all pre generatedd objects for fast drawing in AccPanel """
    def __init__(self, poly=None, stocks=None, barrier=None, hits=None):
        self.poly = poly or []
        self.stocks = stocks or []
        self.barriers = barrier or []
        self.hits = hits or []

class AccPanel(QFrame):
    """ Widget visulizing accelerator behaviour as matplotlib graph """
    state_changed = Signal(bool)

    def __init__(self, name, acc_class, fast_drawing):
        super().__init__()
        self._name = name
        self._acc_class = acc_class
        self._fast_drawing = fast_drawing
        self._config = None
        self._draw_data = AccDrawData()
        self._progress = None
        self._dirty = False # new poly but not yet redrawn
        self.setFrameStyle(QFrame.Box)
        self._t = time.time()
        self._activity = False

    def minimumSizeHint(self):
        return QSize(100, 100)

    def _get_path_cnt(self):
        if self._acc_class == "heston_sl":
            return self._config['simulation_sl']['path_cnt']
        elif self._acc_class == "heston_ml":
            return self._config['path_cnt']
        else:
            raise Exception("Unknown accelerator class")

    def _get_step_cnt(self):
        if self._acc_class == "heston_sl":
            return self._config['simulation_sl']['step_cnt']
        else:
            return self._config['step_cnt_fine']

    def set_config(self, config):
        self._config = config
        self._progress = 0
        self._set_activity(self._get_path_cnt() != 0)
        self._draw_data = self.get_draw_data()
        self.update()

    def _set_activity(self, activity):
        if activity != self._activity:
            self._activity = activity
            self.state_changed.emit(activity)

    def get_activity(self):
        return self._activity
    
    def set_fast_drawing(self, fast_drawing):
        self._fast_drawing = fast_drawing
        self._draw_data = self.get_draw_data()
        self.update()

    def get_heston_path(self):
        if self._acc_class == "heston_sl":
            heston = self._config['heston']
            do_multilevel = False
        else:
            heston = self._config['ml_params']['heston']
            do_multilevel = self._config['do_multilevel']
            ml_constant = self._config['ml_params']['simulation_ml']\
                    ['ml_constant']
        #TODO: optimize dict access in inner loop
        #TODO: use multiprocessing
        stock = [math.log(heston['spot_price'])]
        vola = [heston['vola_0']]
        step_cnt = self._get_step_cnt()
        # only draw as many points as visible on screen
        if self._fast_drawing:
            step_cnt = min(self.width(), step_cnt)
        step_size = heston['time_to_maturity'] / step_cnt
        w_array = []
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
            w_array.append((w_0, w_1))
        coarse_stock = np.exp(np.array(stock))
        # calculate coarse path
        if do_multilevel:
            step_size_fine = step_size
            w_array = list(reversed(w_array))
            stock = [math.log(heston['spot_price'])]
            vola = [heston['vola_0']]
            step_cnt = step_cnt // ml_constant
            step_size = heston['time_to_maturity'] / step_cnt
            for _ in range(step_cnt):
                max_vola = max(0, vola[-1])
                sqrt_vola = np.sqrt(max_vola)
                w_0 = w_1 = 0
                for _ in range(ml_constant):
                    w = w_array.pop()
                    w_0 += w[0]
                    w_1 += w[1]
                stock.append(stock[-1] + 
                        (heston['riskless_rate'] - 0.5 * max_vola) *
                        step_size + math.sqrt(step_size_fine) * sqrt_vola * w_0)
                vola.append(vola[-1] + heston['reversion_rate'] * step_size *
                        (heston['long_term_avg_vola'] - max_vola) + 
                        heston['vol_of_vol'] * math.sqrt(step_size_fine) * 
                        sqrt_vola * w_1)
            fine_stock = np.exp(np.array(stock))
            return coarse_stock, fine_stock
        else:
            return (coarse_stock,)

    def get_plot_width(self):
        bar_width = min(self.width() * 0.25, 50)
        return self.width() - bar_width

    def get_scale(self):
        return self.height() / 300.

    def get_draw_data(self, stocks=None):
        if stocks is not None:
            # generate polygon
            poly = []
            for stock in stocks:
                t = np.linspace(0, self.get_plot_width(), len(stock))
                s = self.height() - stock * self.get_scale()
                poly.append(QPolygonF(list(map(lambda p: QPointF(*p), zip(t, s)))))
        else:
            poly = []
            
        # generate barrier info
        barriers = []
        barrier_hits = []
        if self._config is not None:
            config = self._config if self._acc_class == 'heston_sl' \
                    else self._config['ml_params']
            for type in ['lower', 'upper']:
                value = config['barrier'][type]
                y = self.height() - value * self.get_scale()
                barriers.append(y)
                # get first point where path is hitting barrier
                if stocks is not None:
                    for stock in stocks:
                        t = np.linspace(0, self.get_plot_width(), len(stock))
                        if type == 'lower':
                            hits = t[stock < value]
                        else:
                            hits = t[stock > value]
                        if (len(hits) > 0):
                            x = hits[0]
                            barrier_hits.append((x, y))
        
        return AccDrawData(poly, stocks, barriers, barrier_hits)

    def resizeEvent(self, event):
        self._draw_data = self.get_draw_data()

    def paintEvent(self, event):
        #super(QFrame, self).paintEvent(event)
        QFrame.paintEvent(self, event)
        painter = QPainter(self)
        
        #background
        painter.setBrush(QBrush(QColor(255, 255, 255, 220)))
        painter.setPen(Qt.NoPen)
        painter.drawRect(0, 0, self.width() - 1, self.height() - 1)

        # text
        painter.setPen(Qt.black)
        painter.drawText(QPoint(10, 20), self._name)
        if self._config is not None:
            painter.drawText(QPoint(10, 35), 
                    "Number of paths: {}".format(self._get_path_cnt()))
            if self._acc_class == "heston_ml" and \
                    self._config['do_multilevel']:
                fine = '{} / '.format(self._config['step_cnt_fine'] // 
                        self._config['ml_params']['simulation_ml']
                        ['ml_constant'])
            else:
                fine = ''
            painter.drawText(QPoint(10, 50), 
                    "Number of steps: {}{}".format(fine, self._get_step_cnt()))

        # draw barrier
        if self._draw_data is not None:
            painter.setPen(QColor(255, 0, 0))
            for barrier in self._draw_data.barriers:
                painter.drawLine(0, barrier, self.get_plot_width(), barrier)

        # draw hit points
        if self._draw_data is not None:
            painter.setPen(QPen(QColor(255, 0, 0), 1.5))
            painter.setBrush(Qt.NoBrush)
            for x, y in self._draw_data.hits:
                painter.drawEllipse(QPointF(x, y), 7, 7)

        # progress bar
        if self._progress is not None:
            painter.setBrush(QBrush(QColor(255, 255, 255)))
            painter.setPen(QPen(Qt.black, 2))
            x = self.get_plot_width() + 1
            y = int(self.height() * (1 - self._progress)) + 1
            painter.drawRect(x, y, self.width() - x - 1, self.height() - y - 1)

        # polygon
        if self._draw_data is not None:
            for poly in self._draw_data.poly:
                painter.setPen(QPen(Qt.black, 1))
                painter.drawPolyline(poly)

        self._dirty = False

    def new_paths(self, new_count):
        path_cnt = self._get_path_cnt()
        self._progress = (new_count / path_cnt)
        self._set_activity(new_count != path_cnt)
        if not self._dirty:
            self._dirty = True
            stock = self.get_heston_path()
            self._draw_data = self.get_draw_data(stock)

            if DEBUG_DRAW_SPEED:
                print(1/(time.time() - self._t))
                self._t = time.time()
        else:
            if DEBUG_DRAW_SPEED:
                print("ignoring", self._name, new_count)
        self.update()


class PictureLabel(QLabel):
    """ Widget that is showing scaled image and correct aspect ratio """
    def __init__(self):
        super().__init__()
        self._pixmap = None
        self.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)

        self._layout = QHBoxLayout()
        self._acc_layout = None
        self._sub_layouts = []
        self.setLayout(self._layout)

    def set_accelerators(self, accelerators):
        # recreate box layout
        if self._acc_layout is not None:
            self._acc_layout.deleteLater()
        self._acc_layout = QGridLayout()
        self._acc_layout.setSpacing(20)
        self._layout.addLayout(self._acc_layout)
        
        # add accelerators
        cols = 2
        for i, acc in enumerate(accelerators):
            col_index = i % cols
            row_index = i // cols
            self._acc_layout.addWidget(acc, row_index, col_index)

        self._update()

    def setPixmap(self, pixmap):
        self._pixmap = pixmap
        self._update()

    def resizeEvent(self, event):
        super().resizeEvent(event)
        self._update()

    def _update(self):
        if self._pixmap is not None:
            w, h = self.width(), self.height()
            scaled = self._pixmap.scaled(w, h, Qt.KeepAspectRatio)
            super().setPixmap(scaled)
            # adapt margins of top layout
            w_margin = (w - scaled.width()) / 2
            h_margin = (h - scaled.height()) / 2
            self._layout.setContentsMargins(w_margin, h_margin, w_margin, h_margin)
            # adapt area for CPU
            if self._acc_layout is not None:
                left_margin = scaled.width() * 0.275
                spacing = scaled.width() * 0.012
                self._acc_layout.setContentsMargins(left_margin, spacing, spacing, spacing)
            


class DevicePanel(QFrame):
    """ Widget showing FPGA floorplan """
    def __init__(self):
        super().__init__()
        self._active = False
        self._bitstream = None

        layout = QVBoxLayout()
        self._fpga_img = PictureLabel()
        self._fpga_name = QLabel("<unknown>")
        self._fpga_name.setAlignment(Qt.AlignHCenter | Qt.AlignVCenter)
        font = self._fpga_name.font()
        font.setPointSize(20)
        font.setBold(True)
        self._fpga_name.setFont(font)
        layout.addWidget(self._fpga_name)
        layout.addWidget(self._fpga_img, stretch=1)
        layout.setContentsMargins(0, 0, 0, 0)
        self.setLayout(layout)

    def minimumSizeHint(self):
        return QSize(20, 20)
    
    def set_active(self, active):
        if self._active != active:
            self._active = active
            self._update()

    def set_bitstream(self, bitstream, accelerators):
        self._bitstream = bitstream
        self._update()
        self._fpga_img.set_accelerators(accelerators)

    def _update(self):
        if self._bitstream is not None:
            self._fpga_img.setPixmap(self._bitstream.get_pixmap(self._active))
            self._fpga_name.setText(self._bitstream.name + " ({})".format(
                    "active" if self._active else "idle"))


class Window(QWidget):
    """ main window """
    def __init__(self, observer):
        super().__init__()
        self.observer = observer
        self._fast_drawing = True
        self._accelerators = {}
        self._last_params = None
        self._repeat = False
        self._last_run_cmd = None

        # right panel (buttons & console)
        button_layout = QHBoxLayout()
        self._buttons = {}
        for name in COMMAND_BUTTONS:
            button = QPushButton(name)
            button.clicked.connect(functools.partial(
                    self.on_command_button, name))
            button_layout.addWidget(button)
            button.setEnabled(False)
            self._buttons[name] = button
        right_panel = QWidget()
        right_layout = QVBoxLayout()
        right_layout.addLayout(button_layout)
        self._console = QPlainTextEdit()
        font = QFont("Monospace")
        font.setStyleHint(QFont.TypeWriter)
        self._console.setFont(font)
        self._console.setReadOnly(True)
        right_layout.addWidget(self._console, stretch=1)
        checkbox_layout = QHBoxLayout()
        checkbox_fp = QCheckBox("fast path generation")
        checkbox_fp.setCheckState(Qt.Checked)
        checkbox_fp.stateChanged.connect(self.on_fast_drawing_state_changed)
        checkbox_repeat = QCheckBox("repeat")
        checkbox_repeat.stateChanged.connect(self.on_repeat_state_changed)
        self._repeat_timer = QTimer()
        self._repeat_timer.setSingleShot(True)
        self._repeat_timer.setInterval(1000) # ms
        self._repeat_timer.timeout.connect(self.on_repeat_timer)
        checkbox_layout.addWidget(checkbox_fp)
        checkbox_layout.addWidget(checkbox_repeat)
        checkbox_layout.addStretch(1)
        right_layout.addLayout(checkbox_layout)
        right_panel.setLayout(right_layout)
        
        # top_layout (splitter)
        self._device_panel = DevicePanel()
        self._device_panel.resize(500, 100)
        bottom_splitter = QSplitter(Qt.Horizontal)
        bottom_splitter.addWidget(self._device_panel)
        bottom_splitter.addWidget(right_panel)
        layout = QVBoxLayout()
        layout.addWidget(bottom_splitter)
        self.setLayout(layout)

        # connect observer signals
        self.observer.event.connect(self.on_observer_event)
        self.observer.console_char.connect(self.on_new_console_char)
        self.observer.connected.connect(self.on_console_connected)

        self.resize(800, 600)
        self.showMaximized()

    def on_observer_event(self, type, value, instance=None):
        value = json.loads(value)
        #if type == "fpga_config":
        #    self.on_new_fpga_config(value)
        if type == "setup_sl":
            self.on_setup_sl(instance, value)
        elif type == "setup_ml":
            self.on_setup_ml(instance, value)
        elif type == "new_path":
            self.on_new_paths(instance, value)
        else:
            print("unknown event:", type, value, instance)

    def set_bitstream(self, bitstream):
        # delete all accelerator widgets
        for acc in self._accelerators.values():
            acc.deleteLater()
        self._accelerators = {}

        # read config file and setup accelerator widgets
        config = bitstream.get_config()
        sorted_acc_panels = []
        for instance in sorted(config):
            acc = AccPanel(instance, config[instance]["__class__"], 
                    self._fast_drawing)
            self._accelerators[instance] = acc
            sorted_acc_panels.append(acc)
            acc.state_changed.connect(self.on_accelerator_activity_change)

        self._device_panel.set_bitstream(bitstream, sorted_acc_panels)

    def on_setup_sl(self, instance, config):
        if len(config) == 0:
            config = self._last_params
        self._accelerators[instance].set_config(config)
        self._last_params = config

    def on_setup_ml(self, instance, config):
        if 'ml_params' not in config:
            config['ml_params'] = self._last_params
        self._accelerators[instance].set_config(config)
        self._last_params = config['ml_params']

    def on_new_paths(self, instance, new_count):
        self._accelerators[instance].new_paths(new_count)

    def on_new_console_char(self, char):
        cursor = self._console.textCursor()
        cursor.movePosition(QTextCursor.End)
        cursor.insertText(char)
        self._console.setTextCursor(cursor)

    def on_command_button(self, name):
        for item in COMMAND_BUTTONS[name]:
            if isinstance(item, Bitstream):
                cmd = item.get_configuration_cmd()
                self.set_bitstream(item)
            else:
                cmd = item
            self.observer.send_cmd(cmd)
        self._last_run_cmd = name
        self._repeat_timer.start()

    def set_button_enabled(self, enabled):
        for button in self._buttons.values():
            button.setEnabled(enabled)

    def on_accelerator_activity_change(self, state):
        activity = any(acc.get_activity() 
                for acc in self._accelerators.values())
        self._device_panel.set_active(activity)
        self.set_button_enabled(not activity)
        if activity:
            self._repeat_timer.stop()
        else:
            self._repeat_timer.start()

    def on_console_connected(self):
        self.set_button_enabled(True)

    def on_fast_drawing_state_changed(self, state):
        self._fast_drawing = state == Qt.Checked
        for acc in self._accelerators.values():
            acc.set_fast_drawing(self._fast_drawing)

    def on_repeat_state_changed(self, state):
        self._repeat = state == Qt.Checked

    def on_repeat_timer(self):
        if self._repeat:
            cmds = list(COMMAND_BUTTONS)
            next_cmd = cmds[(cmds.index(self._last_run_cmd) + 1) % len(cmds)]
            self.on_command_button(next_cmd)




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
    observer.close()
    observer.wait()
    return ret


if __name__ == "__main__":
    sys.exit(main())
