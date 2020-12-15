#!/usr/bin/env python3

import os
import sys
import time
import pickle
import argparse
import numpy as np
import zmq
import json
from collections import deque

import wib_pb2 as wib

colors = [(0x00,0x2b,0x36),(0x07,0x36,0x42),(0x58,0x6e,0x75),(0x83,0x94,0x96)]

try:
    from matplotlib.backends.qt_compat import QtCore, QtWidgets, QtGui
except:
    from matplotlib.backends.backend_qt4agg import QtCore, QtWidgets, QtGui

if int(QtCore.qVersion().split('.')[0]) >= 5:
    from matplotlib.backends.backend_qt5agg import (
        FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
else:
    from matplotlib.backends.backend_qt4agg import (
        FigureCanvas, NavigationToolbar2QT as NavigationToolbar)
    
class Sensor(QtWidgets.QWidget):
    def __init__(self,parent):
        super().__init__(parent)
        self.setAutoFillBackground(True)
        self.set_color(*colors[1])
    
    def set_color(self,r,g,b):
        p = self.palette()
        p.setColor(self.backgroundRole(), QtGui.QColor(r,g,b))
        self.setPalette(p)

class IVTSensor(Sensor):
    def __init__(self,parent,label,accessor,sense_ohms=0.1):
        super().__init__(parent)
        self.accessor = accessor
        self.label = label
        self.sense_ohms = sense_ohms
        
        layout = QtWidgets.QVBoxLayout(self)
        lbl = QtWidgets.QLabel(self.label)
        lbl.setAlignment(QtCore.Qt.AlignCenter | QtCore.Qt.AlignVCenter)
        lbl.setStyleSheet('QLabel { font-weight: bold; color: #93a1a1; } ')
        layout.addWidget(lbl)
        
        sub = QtWidgets.QWidget()
        sub_layout = QtWidgets.QHBoxLayout(sub)
        self.V = QtWidgets.QLabel('0.00 V')
        self.V.setAlignment(QtCore.Qt.AlignLeft | QtCore.Qt.AlignVCenter)
        self.V.setStyleSheet('QLabel { font-weight: bold; color: #268bd2; } ')
        sub_layout.addWidget(self.V)
        self.I = QtWidgets.QLabel('0.00 mA')
        self.I.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
        self.I.setStyleSheet('QLabel { font-weight: bold; color: #6c71c4; } ')
        sub_layout.addWidget(self.I)
        
        layout.addWidget(sub)
            
    def load_data(self,sensors):
        before,after = self.accessor(sensors)
        current = (before-after)/self.sense_ohms*1000.0 #mA
        self.V.setText('%0.2f V'%after)
        self.I.setText('%0.1f mA'%current)
        
    
class VTSensor(Sensor):
    def __init__(self,parent,label,accessor):
        super().__init__(parent)
        self.accessor = accessor
        self.label = label
        
        layout = QtWidgets.QVBoxLayout(self)
        
        lbl = QtWidgets.QLabel(self.label)
        lbl.setAlignment(QtCore.Qt.AlignCenter | QtCore.Qt.AlignVCenter)
        lbl.setStyleSheet('QLabel { font-weight: bold; color: #93a1a1; } ')
        layout.addWidget(lbl)
        
        sub = QtWidgets.QWidget()
        sub_layout = QtWidgets.QHBoxLayout(sub)
        self.V = QtWidgets.QLabel('0.00 C')
        self.V.setAlignment(QtCore.Qt.AlignCenter | QtCore.Qt.AlignVCenter)
        self.V.setStyleSheet('QLabel { font-weight: bold; color: #2aa198; } ')
        sub_layout.addWidget(self.V)
            
        layout.addWidget(sub)
        
    def load_data(self,sensors):
        vtemp = self.accessor(sensors)
        self.V.setText('%0.2f U'%vtemp)
        
    
class TSensor(Sensor):
    def __init__(self,parent,label,accessor):
        super().__init__(parent)
        self.accessor = accessor
        self.label = label
        
        layout = QtWidgets.QVBoxLayout(self)
        
        lbl = QtWidgets.QLabel(self.label)
        lbl.setAlignment(QtCore.Qt.AlignCenter | QtCore.Qt.AlignVCenter)
        lbl.setStyleSheet('QLabel { font-weight: bold; color: #93a1a1; } ')
        layout.addWidget(lbl)
        
        sub = QtWidgets.QWidget()
        sub_layout = QtWidgets.QHBoxLayout(sub)
        self.T = QtWidgets.QLabel('0.0 C')
        self.T.setAlignment(QtCore.Qt.AlignCenter | QtCore.Qt.AlignVCenter)
        self.T.setStyleSheet('QLabel { font-weight: bold; color: #2aa198; } ')
        sub_layout.addWidget(self.T)
            
        layout.addWidget(sub)
        
    def load_data(self,sensors):
        temp = self.accessor(sensors)
        self.T.setText('%0.1f C'%temp)
        

def dc2dc(s,idx):
    if idx == 0:
        return s.femb0_dc2dc_ltc2991_voltages
    elif idx == 1:
        return s.femb1_dc2dc_ltc2991_voltages
    elif idx == 2:
        return s.femb2_dc2dc_ltc2991_voltages
    elif idx == 3:
        return s.femb3_dc2dc_ltc2991_voltages

class FEMBPane(QtWidgets.QGroupBox):
    def __init__(self,parent,idx):
        super().__init__('FEMB%i'%idx,parent)
        self.idx = idx
        
        self.setAutoFillBackground(True)
        p = self.palette()
        p.setColor(self.backgroundRole(), QtGui.QColor(*colors[0]))
        self.setPalette(p)
        self.setStyleSheet('QGroupBox { font-weight: bold; color: #cb4b16; } ')
            
        self.tpower_sensor = VTSensor(self,'Power Temp',lambda s: s.ltc2499_15_temps[self.idx])
        self.iv_sensors = []
        self.iv_sensors.append(IVTSensor(self,'2.5V LDO A0',lambda s: s.femb_ldo_a0_ltc2991_voltages[self.idx*2:(self.idx+1)*2],sense_ohms=0.01))
        self.iv_sensors.append(IVTSensor(self,'2.5V LDO A1',lambda s: s.femb_ldo_a1_ltc2991_voltages[self.idx*2:(self.idx+1)*2],sense_ohms=0.01))
        self.iv_sensors.append(IVTSensor(self,'5V Bias',lambda s: s.femb_bias_ltc2991_voltages[self.idx*2:(self.idx+1)*2],sense_ohms=0.1))
        self.iv_sensors.append(IVTSensor(self,'4.0V DC/DC 1',lambda s: dc2dc(s,self.idx)[0:2],sense_ohms=0.1))
        self.iv_sensors.append(IVTSensor(self,'4.0V DC/DC 2',lambda s: dc2dc(s,self.idx)[2:4],sense_ohms=0.1))
        self.iv_sensors.append(IVTSensor(self,'4.0V DC/DC 3',lambda s: dc2dc(s,self.idx)[4:6],sense_ohms=0.01))
        self.iv_sensors.append(IVTSensor(self,'4.0V DC/DC 4',lambda s: dc2dc(s,self.idx)[6:8],sense_ohms=0.1))
        
        layout = QtWidgets.QGridLayout(self)
        layout.addWidget(self.tpower_sensor,0,0)
        for i,t in enumerate(self.iv_sensors):
            layout.addWidget(t,(i+1)//4,(i+1)%4)
        
    def load_data(self,sensors):
        print('FEMB',self.idx,'load_data')
        self.tpower_sensor.load_data(sensors)
        for s in self.iv_sensors:
            s.load_data(sensors)
        
class WIBPane(QtWidgets.QGroupBox):
    def __init__(self,parent):
        super().__init__('WIB',parent)
        
        self.setAutoFillBackground(True)
        p = self.palette()
        p.setColor(self.backgroundRole(), QtGui.QColor(*colors[0]))
        self.setPalette(p)
        self.setStyleSheet('QGroupBox { font-weight: bold; color: #cb4b16; } ')
        
        self.t_sensors = []
        self.t_sensors.append(TSensor(self,'Board Temp 1',lambda s: s.ad7414_49_temp))
        self.t_sensors.append(TSensor(self,'Board Temp 2',lambda s: s.ad7414_4d_temp))
        self.t_sensors.append(TSensor(self,'Board Temp 3',lambda s: s.ad7414_4a_temp))
        self.t_sensors.append(TSensor(self,'DDR Temp',lambda s: 0.0)) #FIXME
        self.t_sensors.append(VTSensor(self,'Power Temp 1',lambda s: s.ltc2499_15_temps[4]))
        self.t_sensors.append(VTSensor(self,'Power Temp 2',lambda s: s.ltc2499_15_temps[5]))
        self.t_sensors.append(VTSensor(self,'Power Temp 3',lambda s: s.ltc2499_15_temps[6]))
        self.iv_sensors = []
        self.iv_sensors.append(IVTSensor(self,'5 V',lambda s: s.ltc2990_4e_voltages[0:2],sense_ohms=0.001))
        self.iv_sensors.append(IVTSensor(self,'1.2 V',lambda s: s.ltc2990_4c_voltages[0:2],sense_ohms=0.001))
        self.iv_sensors.append(IVTSensor(self,'3.3 V',lambda s: s.ltc2990_4c_voltages[2:4],sense_ohms=0.001))
        self.iv_sensors.append(IVTSensor(self,'0.85 V',lambda s: s.ltc2991_48_voltages[0:2],sense_ohms=0.001))
        self.iv_sensors.append(IVTSensor(self,'0.9 V',lambda s: s.ltc2991_48_voltages[2:4],sense_ohms=0.001))
        self.iv_sensors.append(IVTSensor(self,'2.5 V',lambda s: s.ltc2991_48_voltages[4:6],sense_ohms=0.001))
        self.iv_sensors.append(IVTSensor(self,'1.8 V',lambda s: s.ltc2991_48_voltages[6:8],sense_ohms=0.001))
        
        layout = QtWidgets.QGridLayout(self)
        for i,t in enumerate(self.t_sensors):
            layout.addWidget(t,0,i)
        for i,t in enumerate(self.iv_sensors):
            layout.addWidget(t,1,i)
        
    def load_data(self,sensors):
        print('WIB load_data')
        for s in self.t_sensors:
            s.load_data(sensors)
        for s in self.iv_sensors:
            s.load_data(sensors)
            
class WIBMon(QtWidgets.QMainWindow):
    def __init__(self,wib_server='127.0.0.1'):
        super().__init__()
        
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REQ)
        self.socket.connect('tcp://%s:1234'%wib_server)
        
        self.setAutoFillBackground(True)
        p = self.palette()
        p.setColor(self.backgroundRole(), QtGui.QColor(*colors[0]))
        self.setPalette(p)
        
        self._main = QtWidgets.QWidget()
        self.setCentralWidget(self._main)
        layout = QtWidgets.QVBoxLayout(self._main)
        
        self.wib_pane = WIBPane(self)
        self.femb_panes = [FEMBPane(self,idx) for idx in range(4)]

        layout.addWidget(self.wib_pane)
        fembs = QtWidgets.QWidget(self._main)
        fembs_layout = QtWidgets.QGridLayout(fembs)
        for idx,f in enumerate(self.femb_panes):
            fembs_layout.addWidget(f,idx//2,idx%2)
        layout.addWidget(fembs)
        
        QtCore.QTimer.singleShot(500, self.get_sensors)
        
    def send_command(self,req,rep):
        cmd = wib.Command()
        cmd.cmd.Pack(req)
        self.socket.send(cmd.SerializeToString())
        rep.ParseFromString(self.socket.recv())
        
    @QtCore.pyqtSlot()
    def get_sensors(self):
        req = wib.GetSensors()
        rep = wib.GetSensors.Sensors()
        self.send_command(req,rep)
        self.wib_pane.load_data(rep)
        for f in self.femb_panes:
            f.load_data(rep)
        QtCore.QTimer.singleShot(1000, self.get_sensors)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Visually display data from a WIB')
    parser.add_argument('--wib_server','-w',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
    args = parser.parse_args()
    
    
    qapp = QtWidgets.QApplication([])
    qapp.setApplicationName('WIB Monitor (%s)'%args.wib_server)
    app = WIBMon(**vars(args))
    app.show()
    qapp.exec_()
