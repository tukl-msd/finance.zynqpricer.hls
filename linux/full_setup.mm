<map version="freeplane 1.2.0">
<!--To view this file, download free mind mapping software Freeplane from http://freeplane.sourceforge.net -->
<node TEXT="etana&#xa;zedboard" FOLDED="false" ID="ID_1428683404" CREATED="1376148889549" MODIFIED="1376149578296"><hook NAME="MapStyle">

<map_styles>
<stylenode LOCALIZED_TEXT="styles.root_node">
<stylenode LOCALIZED_TEXT="styles.predefined" POSITION="right">
<stylenode LOCALIZED_TEXT="default" MAX_WIDTH="600" COLOR="#000000" STYLE="as_parent">
<font NAME="SansSerif" SIZE="10" BOLD="false" ITALIC="false"/>
</stylenode>
<stylenode LOCALIZED_TEXT="defaultstyle.details"/>
<stylenode LOCALIZED_TEXT="defaultstyle.note"/>
<stylenode LOCALIZED_TEXT="defaultstyle.floating">
<edge STYLE="hide_edge"/>
<cloud COLOR="#f0f0f0" SHAPE="ROUND_RECT"/>
</stylenode>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.user-defined" POSITION="right">
<stylenode LOCALIZED_TEXT="styles.topic" COLOR="#18898b" STYLE="fork">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.subtopic" COLOR="#cc3300" STYLE="fork">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.subsubtopic" COLOR="#669900">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.important">
<icon BUILTIN="yes"/>
</stylenode>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.AutomaticLayout" POSITION="right">
<stylenode LOCALIZED_TEXT="AutomaticLayout.level.root" COLOR="#000000">
<font SIZE="18"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,1" COLOR="#0033ff">
<font SIZE="16"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,2" COLOR="#00b439">
<font SIZE="14"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,3" COLOR="#990000">
<font SIZE="12"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,4" COLOR="#111111">
<font SIZE="10"/>
</stylenode>
</stylenode>
</stylenode>
</map_styles>
</hook>
<node TEXT="Build OS" POSITION="right" ID="ID_1251702598" CREATED="1376159619736" MODIFIED="1376159622112">
<node TEXT="Xilinx Vivado 2013.2" ID="ID_154302779" CREATED="1376149042413" MODIFIED="1376157030103">
<node TEXT="Xilinx_Vivado_SDK_2013.2_0616_1.tar" ID="ID_1165105532" CREATED="1376157023759" MODIFIED="1376157024384"/>
<node TEXT="Based on ISE 14.6" ID="ID_1229079194" CREATED="1376157698841" MODIFIED="1376157703348"/>
</node>
<node TEXT="Boot Image (BOOT.BIN)" ID="ID_1743432889" CREATED="1378112490557" MODIFIED="1378112497354">
<node TEXT="u-boot" ID="ID_1926576280" CREATED="1376149544801" MODIFIED="1376149550733">
<node TEXT="version" ID="ID_1542304597" CREATED="1376157555216" MODIFIED="1376157601586">
<node TEXT="REPO: git://git.xilinx.com/u-boot-xlnx.git" ID="ID_199432" CREATED="1376157067365" MODIFIED="1376157199918"/>
<node TEXT="TAG: xilinx-14.6.01" ID="ID_1505166718" CREATED="1376157072165" MODIFIED="1376157207970"/>
<node TEXT="COMMIT: 0f6dbff16b792a106f52ca37f4503335af30601b" ID="ID_668184764" CREATED="1376157242573" MODIFIED="1376157434689"/>
</node>
</node>
<node TEXT="FPGA Design (minimal)" ID="ID_75969801" CREATED="1376157833972" MODIFIED="1376157856077">
<node TEXT="start vivado" ID="ID_1187469394" CREATED="1376157844543" MODIFIED="1376157848732"/>
<node TEXT="for zc702 make sure to raise the GP0 clock to 100 MHz" ID="ID_276335284" CREATED="1378112601624" MODIFIED="1378112612966"/>
<node TEXT="generate bitstream" ID="ID_107720302" CREATED="1378112616056" MODIFIED="1378112619077"/>
</node>
<node TEXT="Build with sdk, see pdf file" ID="ID_1874720665" CREATED="1378112641128" MODIFIED="1378112648741"/>
</node>
<node TEXT="Device Tree" ID="ID_1527708473" CREATED="1376149532447" MODIFIED="1376149544111">
<node TEXT="version" ID="ID_1797994550" CREATED="1376157597386" MODIFIED="1376157599032">
<node TEXT="REPO: git://github.com/Xilinx/device-tree.git" ID="ID_962067672" CREATED="1376157636538" MODIFIED="1376157640163"/>
<node TEXT="TAG: xilinx-v14.6" ID="ID_695848588" CREATED="1376157660025" MODIFIED="1376157672488"/>
<node TEXT="COMMIT: a6e440e5a34df51cc5febb4177955bfeb3150bf9" ID="ID_301533670" CREATED="1376157649949" MODIFIED="1376157658587"/>
</node>
<node TEXT="create DTS" ID="ID_1856647648" CREATED="1377696417074" MODIFIED="1377696980258" LINK="http://www.wiki.xilinx.com/Build+Device+Tree+Blob">
<node TEXT="in Vivado: File -&gt; Export -&gt; Export Hardware for SDK" ID="ID_792203851" CREATED="1377696422769" MODIFIED="1377696449762"/>
<node TEXT="copy device-tree to &lt;project&gt;.sdk/SDK/SDK_Export/bsp/device-tree_v0_00_x -r" ID="ID_267180813" CREATED="1377696450274" MODIFIED="1377696569351"/>
<node TEXT="In SDK: Xilinx Tools -&gt; Repositories" ID="ID_249075901" CREATED="1377696594257" MODIFIED="1377696611680">
<node TEXT="New... local repository" ID="ID_1730575839" CREATED="1377696612816" MODIFIED="1377696686053"/>
<node TEXT="Add folder: &lt;project&gt;.sdk/SDK/SDK_Export" ID="ID_683553916" CREATED="1377696835151" MODIFIED="1377696845676"/>
</node>
<node TEXT="Create new Board Support Package project" ID="ID_400951054" CREATED="1377696847837" MODIFIED="1377696869804">
<node TEXT="select device-tree on first screen" ID="ID_393060506" CREATED="1377696870558" MODIFIED="1377696877612"/>
<node TEXT="bootargs" ID="ID_287812527" CREATED="1377697171724" MODIFIED="1377697174665">
<node TEXT="console=ttyPS0,115200 root=/dev/mmcblk0p2 rw earlyprintk rootfstype=ext4 rootwait devtmpfs.mount=0" ID="ID_1372136804" CREATED="1377697159580" MODIFIED="1377697167050"/>
</node>
</node>
</node>
<node TEXT="build DTB" ID="ID_1475476160" CREATED="1377703371806" MODIFIED="1377703374860">
<node TEXT="cd &lt;project&gt;.sdk/SDK/SDK_Export/device-tree_bsp_0/ps7_cortexa9_0/libsrc/device-tree_v0_00_x" ID="ID_416654005" CREATED="1377703391679" MODIFIED="1377703402395"/>
<node TEXT="~/zedboard_compile/linux-xlnx/scripts/dtc/dtc -I dts -O dtb -o xilinx.dtb xilinx.dts" ID="ID_738426896" CREATED="1377703376141" MODIFIED="1377703377579"/>
</node>
</node>
<node TEXT="Xilinx Linux Kernel (uImage)" ID="ID_1300416742" CREATED="1376149534898" MODIFIED="1378112574982">
<node TEXT="version" ID="ID_226160398" CREATED="1376157474491" MODIFIED="1376157604159">
<node TEXT="REPO: git://git.xilinx.com/linux-xlnx.git" ID="ID_384949982" CREATED="1376157485030" MODIFIED="1376157610827"/>
<node TEXT="TAG: xilinx-v14.6.02" ID="ID_1417424999" CREATED="1376157488841" MODIFIED="1376157538692"/>
<node TEXT="COMMIT: 3f7c2d54957e950b3a36a251578185bfd374562c" ID="ID_893550752" CREATED="1376157496214" MODIFIED="1376157548371"/>
</node>
<node TEXT="make ARCH=arm xilinx_zynq_defconfig" ID="ID_341833177" CREATED="1376159474028" MODIFIED="1376159474702"/>
<node TEXT="make ARCH=arm UIMAGE_LOADADDR=0x8000 uImage" ID="ID_1637712378" CREATED="1376159475125" MODIFIED="1376159480984"/>
<node TEXT="cp arch/arm/boot/uImage ../boot" ID="ID_380118195" CREATED="1376159527628" MODIFIED="1376159560745"/>
</node>
<node TEXT="Format SD-Card" ID="ID_1305769285" CREATED="1376149022912" MODIFIED="1376149561863">
<node TEXT="In Lubuntu VMWare" ID="ID_839023197" CREATED="1376149666313" MODIFIED="1376149675457"/>
<node TEXT="sudo gparted" ID="ID_708799262" CREATED="1376149675751" MODIFIED="1376149681525"/>
<node TEXT="delete all partitions" ID="ID_877069385" CREATED="1376149681988" MODIFIED="1376149689095"/>
<node TEXT="1. 50 MB FAT32, label=boot, flag=boot" ID="ID_1980748494" CREATED="1376149689390" MODIFIED="1376149702666"/>
<node TEXT="2. 4 GB EXT4, label=root" ID="ID_34096048" CREATED="1376149703078" MODIFIED="1376149715071"/>
</node>
<node TEXT="Linaro 13.07" ID="ID_1186068821" CREATED="1376149194964" MODIFIED="1376149374753">
<node TEXT="insert sd-card, df -h, mount root partition to /media/root (sudo mount /dev/sdd2 /media/root)" ID="ID_504193254" CREATED="1376149250261" MODIFIED="1425044088729"/>
<node TEXT="wget http://releases.linaro.org/13.07/ubuntu/raring-images/alip/linaro-raring-alip-20130723-440.tar.gz" ID="ID_332016337" CREATED="1376149198324" MODIFIED="1376149408921"/>
<node TEXT="wget http://releases.linaro.org/13.10/ubuntu/raring-images/server/linaro-raring-server-20131021-529.tar.gz" ID="ID_506820989" CREATED="1384331541090" MODIFIED="1384331543653"/>
<node TEXT="wget http://releases.linaro.org/latest/ubuntu/utopic-images/server/linaro-utopic-server-20150220-698.tar.gz" ID="ID_514330841" CREATED="1425046508550" MODIFIED="1425046512619"/>
<node TEXT="sudo tar &#x2013;strip-components=1 -C /media/root -xzpf linaro-utopic-server-20150220-698.tar.gz" ID="ID_921017151" CREATED="1376149215867" MODIFIED="1425046686355"/>
<node TEXT="sync; umount/media/root; sync" ID="ID_737049846" CREATED="1376149338583" MODIFIED="1376149347017"/>
</node>
</node>
<node TEXT="Post-Install" POSITION="right" ID="ID_1005210826" CREATED="1376149905020" MODIFIED="1376149907069">
<node TEXT="UBOOT" ID="ID_1609579582" CREATED="1377879697465" MODIFIED="1377879699526">
<node TEXT="Hit any key to stop autoboot" ID="ID_1627074810" CREATED="1377879804777" MODIFIED="1377879810372"/>
<node TEXT="boot command" ID="ID_1097874467" CREATED="1376148895001" MODIFIED="1376148897462">
<node TEXT="when not booting:" ID="ID_780728265" CREATED="1376148898585" MODIFIED="1376164450574"/>
<node TEXT="env default -a" ID="ID_797230464" CREATED="1376164451151" MODIFIED="1376164451955"/>
<node TEXT="saveenv" ID="ID_89467044" CREATED="1376164452220" MODIFIED="1376164456551"/>
</node>
<node TEXT="mac address" ID="ID_99925700" CREATED="1377879707208" MODIFIED="1377879709590">
<node TEXT="env print" ID="ID_465464421" CREATED="1377879792953" MODIFIED="1377879822834"/>
<node TEXT="setenv ethaddr xx:xx:xx:xx:xx:xx" ID="ID_559938263" CREATED="1377879710409" MODIFIED="1378032412985">
<node TEXT="zedboard-3: setenv ethaddr 00:0a:35:01:01:33" ID="ID_1820213146" CREATED="1384337327103" MODIFIED="1384338308436"/>
<node TEXT="zc702-1: setenv ethaddr 00:0A:35:02:7B:9A" ID="ID_1414831783" CREATED="1378032394364" MODIFIED="1378032402819"/>
<node TEXT="zedboard-5: setenv ethaddr 00:0a:35:01:01:35" ID="ID_30833841" CREATED="1403535434542" MODIFIED="1403535444219"/>
<node TEXT="zedboard-7: setenv ethaddr 00:0a:35:01:01:37" ID="ID_1653678798" CREATED="1384337327103" MODIFIED="1384337348336"/>
</node>
<node TEXT="env save" ID="ID_507521314" CREATED="1377880413238" MODIFIED="1377880415586"/>
</node>
</node>
<node TEXT="set date with:" ID="ID_1064741319" CREATED="1376162730380" MODIFIED="1425059647750">
<node TEXT="sudo date -s &quot;Sat Feb 27 18:53:24 UTC 2015&quot;" ID="ID_1551997992" CREATED="1425059649020" MODIFIED="1425059652363"/>
</node>
<node TEXT="network settings" ID="ID_651985133" CREATED="1376492723868" MODIFIED="1376492728572">
<node TEXT="check eth0" ID="ID_1219714761" CREATED="1377878993594" MODIFIED="1377878997755">
<node TEXT="ifconfig" ID="ID_828324983" CREATED="1377878998650" MODIFIED="1377879001575"/>
<node TEXT="IF eth0 not there:" ID="ID_948213773" CREATED="1377879001994" MODIFIED="1377879008295">
<node TEXT="lshw" ID="ID_470953921" CREATED="1377879018906" MODIFIED="1377879019849"/>
<node TEXT="ifconfig eth0 up" ID="ID_996935643" CREATED="1377879008762" MODIFIED="1377879009687"/>
<node TEXT="dhclient eth0" ID="ID_1670619106" CREATED="1384332718664" MODIFIED="1384332723540"/>
</node>
<node TEXT="add automatic startup:" ID="ID_618267640" CREATED="1378805678244" MODIFIED="1378805703971">
<node TEXT="vi /etc/network/interfaces" ID="ID_1312916307" CREATED="1378805704452" MODIFIED="1425061311178"/>
<node TEXT="auto eth0&#xa;iface eth0 inet dhcp" ID="ID_462899796" CREATED="1378805720275" MODIFIED="1378805761017"/>
</node>
</node>
<node TEXT="test IP" ID="ID_434126274" CREATED="1377880555938" MODIFIED="1377880557822">
<node TEXT="ping 172.16.254.1 (nameserver)" ID="ID_428804323" CREATED="1377880559186" MODIFIED="1377880569454"/>
</node>
<node TEXT="test dns" ID="ID_1165689727" CREATED="1377880574545" MODIFIED="1377880575742">
<node TEXT="wget google.de" ID="ID_1851146067" CREATED="1377880578673" MODIFIED="1377880582736"/>
<node TEXT="setup dns server" ID="ID_1836534145" CREATED="1376297235568" MODIFIED="1376297239305">
<node TEXT="echo nameserver 172.16.254.1 &gt; /etc/resolv.conf" ID="ID_1720421303" CREATED="1376297247189" MODIFIED="1378032916785"/>
<node TEXT="echo nameserver 192.168.2.1 &gt;&gt; /etc/resolv.conf" ID="ID_1022219944" CREATED="1376151055053" MODIFIED="1378032920342"/>
</node>
</node>
<node TEXT="vi /etc/hostname" ID="ID_1734590818" CREATED="1376156701079" MODIFIED="1377880608801">
<node TEXT="board-zedboard-3" ID="ID_1214803928" CREATED="1376581399465" MODIFIED="1376581400244"/>
</node>
<node TEXT="vi /etc/hosts" ID="ID_624283105" CREATED="1376492733597" MODIFIED="1377880610062">
<node TEXT="127.0.0.1 localhost&#xa;127.0.0.1 board-zedboard-3" ID="ID_852826014" CREATED="1376492741837" MODIFIED="1376492756098"/>
</node>
<node TEXT="(restart with power off)" ID="ID_1129737137" CREATED="1377880668658" MODIFIED="1377880673662">
<node TEXT="shutdown -h now" ID="ID_1784699851" CREATED="1377880682482" MODIFIED="1377880685918"/>
<node TEXT="power cycle" ID="ID_788511316" CREATED="1377880686209" MODIFIED="1377880690092"/>
</node>
</node>
<node TEXT="user brugger" ID="ID_1333289655" CREATED="1376154219180" MODIFIED="1376322866368">
<node TEXT="adduser brugger" ID="ID_1083966" CREATED="1376322870995" MODIFIED="1376322872287"/>
<node TEXT="usermod -a -G admin brugger" ID="ID_1777208904" CREATED="1376155090787" MODIFIED="1376155353673"/>
<node TEXT="usermod -a -G kmem brugger" ID="ID_533387383" CREATED="1376322890159" MODIFIED="1376322893886"/>
<node TEXT="su brugger" ID="ID_899284492" CREATED="1376154799877" MODIFIED="1376154801650"/>
<node TEXT="mkdir .ssh" ID="ID_1596948694" CREATED="1376154802123" MODIFIED="1376154806746"/>
<node TEXT="vi .ssh/authorized_keys" ID="ID_564058191" CREATED="1376154812313" MODIFIED="1377881556531">
<node TEXT="ssh-rsa AAAAB3NzaC1yc2EAAAABJQAAAQEAlFpJ8JhfBuCv0nyAtHkPUPya0XjgrM2l5/JG68YyYNRgysf6uInr9/pzJGSwfc7g/TRLblSgNYmpe5X8A5hEL+K1u+nPLVf1sUVH8KtGv7CvI9O5nuHim0X7dn3MV8ET58oEt9veVSMKnGFrEdk+BfQ91LT0cF8aUf17+8R7MgZTSbQq2Z7+cTbsX0QXKbI2kmdPastkbxJpuF/OTVyG+RmhIQJ9ClqlYD+fb5ChYwnCDPtOWxoA5K9TgynOEVC0W01RDk6HXHtECDCdK9fFH99LLq8/vxjZMEPtHwejgrxjTTQ9CjRGoO5LgnhMo+UQxqtL+WFLjeW6DF+qJXUHww== uni-kl-unix-desktop&#xa;ssh-rsa AAAAB3NzaC1yc2EAAAABJQAAAQEAnDnRyYPmC092cfTldQiBC3zBRSvEjPuwcwIPDloysXbl03LVTSf4OM4bKgk5rvtM4e0vyxxLWs8CbWSSCTphvHYayOQf384pQb4/EHl8ziFD+H2155oENzf0L39uA67SmbWJFpExw5h3pBZwzP1SFPdGizB8TaaqAC4myKrvMnfuYkUG6iGmsDCWWeCJ2FTu7zuhzbnygZDK/LnPiOpBTXflAre8CKvBc+tugqsJVoUVTGBdjPyNhcS+ge1JKICYW3E2C8XMNfyj2kJuL4ULq6zUEQEObvl2Q3+lnP/UGrI4Q7zBREp2OalCNHCFmlIbzPEUdtnxoYF9dJCWt8mFfw== uni-kl-unix-laptop" ID="ID_1925594156" CREATED="1376154858292" MODIFIED="1376154859255"/>
</node>
</node>
<node TEXT="userdel linaro" ID="ID_168419107" CREATED="1376155521617" MODIFIED="1376155526793">
<node TEXT="rm -r /home/linaro" ID="ID_830053055" CREATED="1376155528067" MODIFIED="1376155533996"/>
</node>
<node TEXT="update packages" ID="ID_248113906" CREATED="1425061207761" MODIFIED="1425061209937">
<node TEXT="vi /etc/apt/sources.list" ID="ID_1692298480" CREATED="1425061238989" MODIFIED="1425061241565"/>
<node TEXT="delete old entries" FOLDED="true" ID="ID_1937968989" CREATED="1425061256843" MODIFIED="1425061260355">
<node TEXT="deb http://ports.ubuntu.com/ubuntu-ports/ raring main universe&#xa;deb-src http://ports.ubuntu.com/ubuntu-ports/ raring main universe" ID="ID_839949493" CREATED="1427991879474" MODIFIED="1427991884030"/>
</node>
<node TEXT="deb http://old-releases.ubuntu.com/ubuntu/ raring main universe&#xa;deb-src http://old-releases.ubuntu.com/ubuntu/ raring main universe" ID="ID_811254862" CREATED="1425061252701" MODIFIED="1425061253664"/>
</node>
<node TEXT="apt-get install openssh-server" ID="ID_1107305165" CREATED="1376149907986" MODIFIED="1376154624382">
<node TEXT="regenerate ssh keys" ID="ID_1021392171" CREATED="1403536674340" MODIFIED="1403536679474"/>
<node TEXT="ssh-keygen -f /etc/ssh/ssh_host_rsa_key -N &apos;&apos; -t rsa&#xa;ssh-keygen -f /etc/ssh/ssh_host_dsa_key -N &apos;&apos; -t dsa&#xa;ssh-keygen -f /etc/ssh/ssh_host_ecdsa_key -N &apos;&apos; -t ecdsa" ID="ID_583485585" CREATED="1403536680211" MODIFIED="1403536708937"/>
</node>
<node TEXT="disable ssh password login" ID="ID_1694013788" CREATED="1376154627445" MODIFIED="1376154729753">
<node TEXT="vi /etc/ssh/sshd_config" ID="ID_1575836272" CREATED="1376154721529" MODIFIED="1378033262527"/>
<node TEXT="uncomment following line" ID="ID_1177605234" CREATED="1376154635768" MODIFIED="1376154642958"/>
<node TEXT="# Change to no to disable tunnelled clear text passwords&#xa;PasswordAuthentication no" ID="ID_119702364" CREATED="1376154643460" MODIFIED="1376154714420"/>
<node TEXT="restart ssh" ID="ID_206305748" CREATED="1376154731837" MODIFIED="1376154773892"/>
</node>
<node TEXT="install timeserver" ID="ID_1619695610" CREATED="1378033335771" MODIFIED="1378033342600">
<node TEXT="sudo apt-get install ntp" ID="ID_269307705" CREATED="1376162998003" MODIFIED="1376163001386"/>
<node TEXT="vi /etc/ntp.conf" ID="ID_1849862305" CREATED="1378033608445" MODIFIED="1378033612157">
<node TEXT="add before: server 0.ubuntu.pool.ntp.org" ID="ID_727716872" CREATED="1378033626830" MODIFIED="1378033634162"/>
<node TEXT="server ntp.rhrk.uni-kl.de" ID="ID_1499684775" CREATED="1378033613205" MODIFIED="1378033615557"/>
</node>
<node TEXT="if in trouble" ID="ID_862034987" CREATED="1425064959901" MODIFIED="1425064982430">
<node TEXT="make sure current time is valid: datetime" ID="ID_1376081445" CREATED="1425064944027" MODIFIED="1425064959243">
<node TEXT="sudo date -s &quot;Sat Feb 27 18:53:24 UTC 2015&quot;" ID="ID_1116286710" CREATED="1425064975276" MODIFIED="1425064976691"/>
</node>
<node TEXT="manually sync date" ID="ID_1883877703" CREATED="1425064989594" MODIFIED="1425064991209">
<node TEXT="sudo /usr/sbin/ntpdate ntp.rhrk.uni-kl.de" ID="ID_625493222" CREATED="1425065005394" MODIFIED="1425065006056"/>
<node TEXT="sudo /etc/init.d/ntp stop" ID="ID_498359451" CREATED="1425065018915" MODIFIED="1425065020937"/>
</node>
</node>
</node>
<node TEXT="packets basic" ID="ID_612112429" CREATED="1378033376471" MODIFIED="1378033397325">
<node TEXT="sudo apt-get install vim" ID="ID_413546417" CREATED="1378033410497" MODIFIED="1378033416077"/>
</node>
<node TEXT="install zynqpricer" ID="ID_1537199045" CREATED="1378033387500" MODIFIED="1378033670356">
<node TEXT="sudo apt-get install git g++ make cmake" ID="ID_1981026005" CREATED="1378033425651" MODIFIED="1378034212393"/>
<node TEXT="Add certificate to github" ID="ID_1170319166" CREATED="1378033728150" MODIFIED="1378033732543">
<node TEXT="ssh-keygen -t rsa" ID="ID_1893461607" CREATED="1378033722450" MODIFIED="1378033723544"/>
<node TEXT="cat ~/.ssh/id_rsa.pub" ID="ID_1353184853" CREATED="1378033739318" MODIFIED="1378033750790"/>
</node>
<node TEXT="git clone git@git.rhrk.uni-kl.de:EIT-Wehn/finance.zynqpricer.hls.git" ID="ID_870233099" CREATED="1378033896270" MODIFIED="1378033898297"/>
</node>
<node TEXT="install scikit-learn" ID="ID_641836232" CREATED="1376158088332" MODIFIED="1376158092155">
<node TEXT="sudo apt-get install ipython3 python3-scipy python3-mpi4py python3-tk python3-matplotlib openmpi-bin" ID="ID_1090853004" CREATED="1376154013878" MODIFIED="1378033422816"/>
<node TEXT="get from PyPI" ID="ID_1474901922" CREATED="1376158093351" MODIFIED="1376158131798" LINK="https://pypi.python.org/pypi/scikit-learn/"/>
<node TEXT="wget https://pypi.python.org/packages/source/s/scikit-learn/scikit-learn-0.14.1.tar.gz#md5=790ad23547bb7f428060636628e13491" ID="ID_422633398" CREATED="1376158142158" MODIFIED="1376158143616"/>
<node TEXT="sudo apt-get install g++ python3-dev" ID="ID_614770655" CREATED="1376158197067" MODIFIED="1376161377624"/>
<node TEXT="sudo python3 setup.py install" ID="ID_336101908" CREATED="1376158153603" MODIFIED="1376159794704"/>
<node TEXT="sudo apt-get install gpicview" ID="ID_1522071116" CREATED="1376161374422" MODIFIED="1376161375125"/>
</node>
</node>
<node TEXT="Dynamic Reconfiguration" POSITION="right" ID="ID_1433988973" CREATED="1378114319611" MODIFIED="1378114323449">
<node TEXT="create bitstream file" ID="ID_717259164" CREATED="1378114415911" MODIFIED="1378114430694">
<node TEXT="Generate bitstream in Vivado" ID="ID_210424585" CREATED="1378114327882" MODIFIED="1378114437798"/>
<node TEXT="cd &lt;project&gt;/&lt;project&gt;.runs/impl_1" ID="ID_478091010" CREATED="1378114381543" MODIFIED="1378114391783"/>
<node TEXT="promgen -b -w -p bin -data_width 32 -u 0 design_1_wrapper.bit -o design_1_wrapper.bit.bin" ID="ID_32450772" CREATED="1378114324122" MODIFIED="1378114325335"/>
<node TEXT="copy *.bin to zynq" ID="ID_1851906467" CREATED="1378114407018" MODIFIED="1378114412278"/>
</node>
</node>
<node TEXT="Based on:" POSITION="left" ID="ID_1679151655" CREATED="1376149119877" MODIFIED="1376149129184">
<node TEXT="Yet Another Guide to Running&#xa;Linaro Ubuntu Linux Desktop on&#xa;Xilinx Zynq on the ZedBoard" ID="ID_386932876" CREATED="1376149131580" MODIFIED="1376149152261" LINK="http://fpgacpu.wordpress.com/2013/05/24/yet-another-guide-to-running-linaro-ubuntu-desktop-on-xilinx-zynq-on-the-zedboard/"/>
<node TEXT="Vivado Design Suite Tutorial:&#xa;Embedded Processor Hardware Design" ID="ID_1535850436" CREATED="1376157799036" MODIFIED="1376157812162" LINK="http://www.xilinx.com/support/documentation/sw_manuals/xilinx2013_2/ug940-vivado-tutorial-embedded-design.pdf"/>
</node>
<node TEXT="Restart DHCP" POSITION="left" ID="ID_153230686" CREATED="1376743478044" MODIFIED="1376743480779">
<node TEXT="dhclient -r" ID="ID_189700929" CREATED="1376743481745" MODIFIED="1376743482651"/>
<node TEXT="dhclient eth0" ID="ID_1729385310" CREATED="1376743483094" MODIFIED="1376743486522"/>
</node>
</node>
</map>
