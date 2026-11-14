import socket
import struct
import numpy as np
import csv
import os
import pyqtgraph as pg
import pyqtgraph.exporters
from PyQt6 import QtCore, QtWidgets
from scipy.signal import medfilt

# =========================
# PROTOCOL CONSTANTS
# =========================
HEADER_FORMAT = "<BHBBHH"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)

CMD_RDY    = 1
CMD_SENSOR = 2
CMD_US     = 3
CMD_STOP   = 4

PKT_CMD    = 1
PKT_ACK    = 2
PKT_SENSOR = 3
PKT_SONAR  = 4
PKT_GPS    = 5

PKT_NAMES = {
    PKT_CMD: "CMD",
    PKT_ACK: "ACK",
    PKT_SENSOR: "SENSOR",
    PKT_SONAR: "SONAR",
    PKT_GPS: "GPS"
}

SEQ = 0

# =========================
# CONFIG PARAMS
# =========================
TX_IP = "192.168.4.1"
RX_IP = "0.0.0.0"
UDP_PORT = 5005

NUM_SAMPLES = 1000      
MAX_COLS = 300          
SPEED_OF_SOUND = 1480  
SAMPLE_TIME = 20e-6
SAMPLE_RESOLUTION = (SPEED_OF_SOUND * SAMPLE_TIME) / 2 

COLOR_LEVEL_MIN = 0
COLOR_LEVEL_MAX = 2000

# =========================
# DATA INIT
# =========================
data = np.zeros((NUM_SAMPLES, MAX_COLS), dtype=np.uint16)

# =========================
# HELPER FUNCTIONS
# =========================
def parse_packet(data_bytes: bytes):
    if len(data_bytes) < HEADER_SIZE:
        return None
    ptype, pid, seq, length = struct.unpack_from(HEADER_FMT, data_bytes, 0)
    payload = data_bytes[HEADER_SIZE:HEADER_SIZE+length]
    return ptype, pid, payload

def send_cmd(cmd_id):
    global SEQ
    pkt = struct.pack(HEADER_FMT, PKT_CMD, cmd_id, SEQ, 0)
    print(f"[TX] Sending CMD: {cmd_id} | SEQ: {SEQ}")
    SEQ = (SEQ + 1) & 0xFFFF
    sock.sendto(pkt, (TX_IP, UDP_PORT))
    
    if cmd_id == CMD_RDY:
        set_status_connected("connecting")

def set_status_connected(state: str):
    if state == "connected":
        status_led.setText("CONNECTED")
        status_led.setStyleSheet("""
            background-color: #2ecc71; 
            color: white; 
            font-weight: bold; 
            border-radius: 5px; 
            padding: 5px;
        """)
        btn_connect.setEnabled(False)  
    elif state == "connecting":
        status_led.setText("WAITING FOR CONNECTION...")
        status_led.setStyleSheet("""
            background-color: #f39c12; 
            color: white; 
            font-weight: bold; 
            border-radius: 5px; 
            padding: 5px;
        """)
        btn_connect.setEnabled(False)  
    else:
        status_led.setText("DISCONNECTED")
        status_led.setStyleSheet("""
            background-color: #7f8c8d; 
            color: white; 
            font-weight: bold; 
            border-radius: 5px; 
            padding: 5px;
        """)
        btn_connect.setEnabled(True)

def csv_filename():
    base_name = "recorded_data"
    i = 1
    while True:
        filename = f"{base_name}_{i:03d}.csv"
        if not os.path.exists(filename):
            return filename
        i += 1

def apply_tvg(frame_data, gain_factor=1.0):
    log_scale = 5
    depth = np.arange(len(frame_data))
    gain_vector = 0.5 + (np.log10(1 + depth / log_scale) / np.log10(len(depth)/ log_scale)) * (gain_factor - 1.0)
    return frame_data * gain_vector

# =========================
# UPDATE LOOP
# =========================
def update():
    global data

    visible_samples = slider_y.value()
    colorLO = slider_rangeL.value()
    colorHI = slider_rangeH.value()
    gain = slider_tvg.value() / 10.0

    try:
        while True:
            raw_data, addr = sock.recvfrom(4096)
            pkt = parse_packet(raw_data)
            if not pkt: continue

            ptype, pid, payload = pkt
            pname = PKT_NAMES.get(ptype, f"UNKNOWN({ptype})")
            print(f"[RX] {pname:<7} | ID: {pid:<3} | Len: {len(payload)}")

            if ptype == PKT_ACK:
                if pid == CMD_RDY:
                    set_status_connected("connected")

            if ptype == PKT_SENSOR:
                if len(payload) == 10:
                    press, temp_air, hum, pH, temp_w = struct.unpack("<5H", payload)
                    sensor_press.setText(f"Pressure: {press/10:.1f} hPa")
                    sensor_temp_air.setText(f"Air Temperature: {temp_air/10:.1f} °C")
                    sensor_hum.setText(f"Humidity: {hum/10:.1f} %")
                    sensor_pH.setText(f"Water pH: {pH/10:.1f}")
                    sensor_temp_w.setText(f"Water Temperature: {temp_w/10:.1f} °C")
            elif ptype == PKT_GPS:
                if len(payload) == 6:
                    lat, lon, alt = struct.unpack("<3H", payload)
                    print(lat, lon, alt)
            elif ptype == PKT_SONAR:
                num_samples_in = len(payload) // 2
                if num_samples_in != NUM_SAMPLES:
                    continue

                samples = struct.unpack(f"<{num_samples_in}H", payload)
                frame = np.array(samples, dtype=np.float32)

                frame = apply_tvg(frame, gain)
                if med_flt_but.isChecked():
                    frame = medfilt(frame, kernel_size=3)

                cut_th = slider_cut.value()
                frame[frame < cut_th] = 0

                data = np.roll(data, -1, axis=1)
                data[:, -1] = frame.astype(np.uint16)

                img.setLevels([colorLO, colorHI])

                view_data = data[:visible_samples, :]
                img.setImage(view_data, autoLevels=False)

                img.setRect(QtCore.QRectF(0, 0, MAX_COLS, visible_samples))
                
                update_marker_label()

    except BlockingIOError:
        pass

def update_marker_label():
    y_idx = marker.value()
    y_meters = y_idx * SAMPLE_RESOLUTION
    label.setText(f"Y = {y_meters:.2f} m")
    label.setPos(MAX_COLS, y_idx)

def save_image():
    filename = f"img_{QtCore.QDateTime.currentDateTime().toString('hhmmss')}.png"
    exporter = pg.exporters.ImageExporter(plot)
    exporter.parameters()['width'] = 1000
    exporter.export(filename)
    print(f"Saved: {filename}")

def close_event():
    csvfile.close()
    send_cmd(4)
    sock.close()
    print("Application closed.")

# =========================
# UDP & FILE SETUP
# =========================
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((RX_IP, UDP_PORT))
sock.setblocking(False)

csvfile = open(csv_filename(), "w", newline="")
writer = csv.writer(csvfile)

# =========================
# UI SETUP (PyQt6)
# =========================
app = QtWidgets.QApplication([])
main_win = QtWidgets.QMainWindow()
main_win.setWindowTitle("Echosounder GUI")

central_widget = QtWidgets.QWidget()
main_win.setCentralWidget(central_widget)
main_layout = QtWidgets.QHBoxLayout(central_widget)

# Plot Setup
graphics_layout = pg.GraphicsLayoutWidget()
plot = graphics_layout.addPlot()
plot.invertY(True)
plot.setLabel('left', 'Distance', units='m')
plot.setLabel('bottom', 'Time (relative)')

# ImageItem 
img = pg.ImageItem(axisOrder='row-major')
cmap = pg.colormap.get('viridis')
img.setLookupTable(cmap.getLookupTable(0.0, 1.0, 256))
plot.addItem(img)

# Marker
marker = pg.InfiniteLine(angle=0, movable=True, pen=pg.mkPen('r', width=1))
label = pg.TextItem(color='r', anchor=(1, 1))
plot.addItem(marker)
plot.addItem(label)
marker.sigPositionChanged.connect(update_marker_label)

# Sidebar UI
sidebar = QtWidgets.QVBoxLayout()

def add_slider(name, min_v, max_v, init_v):
    l = QtWidgets.QVBoxLayout()
    lbl = QtWidgets.QLabel(name)
    lbl.setAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
    s = QtWidgets.QSlider(QtCore.Qt.Orientation.Vertical)
    s.setMinimum(min_v)
    s.setMaximum(max_v)
    s.setValue(init_v)
    s.valueChanged.connect(update)
    l.addWidget(lbl)
    l.addWidget(s)
    return l, s

lay_y, slider_y = add_slider("Zoom", 10, NUM_SAMPLES, NUM_SAMPLES)
lay_hi, slider_rangeH = add_slider("Level HI", 100, 4000, COLOR_LEVEL_MAX)
lay_lo, slider_rangeL = add_slider("Level LO", 0, 1000, COLOR_LEVEL_MIN)
lay_tvg, slider_tvg = add_slider("TVG", 10, 50, 10)
lay_cut, slider_cut = add_slider("Cutoff", 0, 1000, 0)

sliders_h = QtWidgets.QHBoxLayout()
for lay in [lay_y, lay_hi, lay_lo, lay_tvg, lay_cut]: sliders_h.addLayout(lay)

status_led = QtWidgets.QLabel()
status_led.setAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)

btn_connect = QtWidgets.QPushButton("CONNECT")
btn_connect.clicked.connect(lambda: send_cmd(CMD_RDY))

set_status_connected("disconnected")

sensor_press = QtWidgets.QLabel("Pressure: ---")
sensor_temp_air  = QtWidgets.QLabel("Air Temperature: ---")
sensor_hum   = QtWidgets.QLabel("Humidity: ---")
sensor_pH  = QtWidgets.QLabel("Water pH: ---")
sensor_temp_w =  QtWidgets.QLabel("Water Temperature: ---")

btn_save = QtWidgets.QPushButton("Save PNG")
btn_save.clicked.connect(save_image)

med_flt_but = QtWidgets.QPushButton("Median Filter")
med_flt_but.setCheckable(True)

# Control Buttons
btn_grid = QtWidgets.QGridLayout()
cmds = [("SENSOR", CMD_SENSOR), ("START", CMD_US), ("STOP", CMD_STOP)]
for i, (name, cid) in enumerate(cmds):
    b = QtWidgets.QPushButton(name)
    b.clicked.connect(lambda ch, x=cid: send_cmd(x))
    btn_grid.addWidget(b, i//3, i%3)

sidebar.addLayout(sliders_h)
sidebar.addWidget(sensor_press)
sidebar.addWidget(sensor_temp_air)
sidebar.addWidget(sensor_hum)
sidebar.addWidget(sensor_pH)
sidebar.addWidget(sensor_temp_w)
sidebar.addSpacing(10)
sidebar.addWidget(status_led)
sidebar.addWidget(btn_connect)
sidebar.addSpacing(10)
sidebar.addWidget(med_flt_but)
sidebar.addWidget(btn_save)
sidebar.addLayout(btn_grid)

main_layout.addWidget(graphics_layout, stretch=4)
main_layout.addLayout(sidebar, stretch=1)

# Timer
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(15)

app.aboutToQuit.connect(close_event)
marker.setValue(10)

main_win.show()
app.exec()