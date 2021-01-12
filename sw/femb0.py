#!/usr/bin/env python3

import os
import sys
import time
import pickle
import argparse
import numpy as np
import matplotlib.pyplot as plt
from collections import deque
from matplotlib.figure import Figure
from matplotlib.colors import LogNorm

from wib import WIB
import wib_pb2 as wibpb

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
        
class CustomNavToolbar(NavigationToolbar):
    NavigationToolbar.toolitems = (
        ('Home', 'Reset original view', 'home', 'home'),
        ('Back', 'Back to previous view', 'back', 'back'),
        ('Forward', 'Forward to next view', 'forward', 'forward'),
        (None, None, None, None),
        ('Pan', 'Pan axes with left mouse, zoom with right', 'move', 'pan'),
        ('Zoom', 'Zoom to rectangle', 'zoom_to_rect', 'zoom'),
        (None, None, None, None),
        ('Save', 'Save the figure', 'filesave', 'save_figure')
    )
    
    def __init__(self, *args, **kwargs):
        '''parent is expected to be a SignalView object'''
        super().__init__(*args, **kwargs)
        
class DataView(QtWidgets.QWidget):
    def __init__(self,parent=None,figure=None):
        super().__init__(parent=parent)
        if figure is None:
            figure = Figure(tight_layout=True)
        self.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.figure = figure
        self.fig_ax = self.figure.subplots()
        self.fig_canvas = FigureCanvas(self.figure)
        self.fig_canvas.draw()
        
        self.fig_toolbar = CustomNavToolbar(self.fig_canvas,self,coordinates=False)
        self.fig_toolbar.setParent(self.fig_canvas)
        self.fig_toolbar.setMinimumWidth(300)
        
        self.fig_canvas.mpl_connect("resize_event", self.resize)
        self.resize(None)
        
        self.layout = QtWidgets.QVBoxLayout(self)
        self.layout.setContentsMargins(0,0,0,0)
        self.layout.setSpacing(0)
        self.layout.addWidget(self.fig_canvas)
        
        self.toolbar_shown(False)
        
        self.save_props = []
        
        self.last_lims = None
        
        self.times,self.data = None,None
    
    def resize(self, event):
        x,y = self.figure.axes[0].transAxes.transform((0,0.0))
        figw, figh = self.figure.get_size_inches()
        ynew = figh*self.figure.dpi-y - self.fig_toolbar.frameGeometry().height()
        self.fig_toolbar.move(int(x),int(ynew))
        
    def focusInEvent(self, *args, **kwargs):
        super().focusInEvent(*args, **kwargs)
        self.resize(None)
        self.toolbar_shown(True)
        
    def focusOutEvent(self, *args, **kwargs):
        super().focusOutEvent(*args, **kwargs)
        self.toolbar_shown(False)
    
    def toolbar_shown(self,shown):
        if shown:
            self.fig_toolbar.show()
        else:
            self.fig_toolbar.hide()
            
    def get_state(self):
        all_props = self.__dict__
        return {prop:getattr(self,prop) for prop in self.save_props if prop in all_props}
            
    def set_state(self, state):
        all_props = self.__dict__
        for prop,val in state.items():
            if prop in all_props:
                setattr(self,prop,val)
            
    def load_data(self,timestamps,samples):
        pass
        
    def plot_data(self,rescale=False):
        pass

def one_more_bin(array):
    return np.append(array,2*array[-1]-array[-2])
    
def dupe_last_val(array):
    return np.append(array,array[-1])

class MeanRMSView(DataView):

    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.twin_ax = self.fig_ax.twinx()
        self.chan = np.arange(128)
        self.rms = np.full_like(self.chan, 0)
        self.mean = np.full_like(self.chan, 0)
      
    def load_data(self,timestamps,samples):
        samples = samples[0] # [femb][channel][sample] -> [channel][sample]
        self.rms = np.std(samples,axis=1)
        self.mean = np.mean(samples,axis=1)
        
        
    def plot_data(self,rescale=False):
        self.fig_ax.clear()
        self.twin_ax.clear()
        
        self.fig_ax.plot(one_more_bin(self.chan),dupe_last_val(self.mean),drawstyle='steps-post',label='Mean',c='b')
        self.twin_ax.plot(one_more_bin(self.chan),dupe_last_val(self.rms),drawstyle='steps-post',label='RMS',c='r')
        
        self.fig_ax.set_xlim(0,128)
        self.fig_ax.set_xlabel('Channel Number')
        self.fig_ax.set_ylabel('Mean ADC Counts')
        self.twin_ax.set_ylabel('RMS ADC Counts')
        
        self.fig_ax.legend(loc='upper left')
        self.twin_ax.legend(loc='upper right')
        self.fig_ax.figure.canvas.draw()
        self.resize(None)
        

class RMSView(DataView):

    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.chan = np.arange(128)
        self.rms = np.full_like(self.chan, 0)
      
    def load_data(self,timestamps,samples):
        samples = samples[0] # [femb][channel][sample] -> [channel][sample]
        self.rms = np.std(samples,axis=1)
        
    def plot_data(self,rescale=False):
        self.fig_ax.clear()
        
        self.fig_ax.plot(one_more_bin(self.chan),dupe_last_val(self.rms),drawstyle='steps-post',label='RMS',c='r')
        
        self.fig_ax.set_xlim(0,128)
        self.fig_ax.set_xlabel('Channel Number')
        self.fig_ax.set_ylabel('RMS ADC Counts')
        
        self.fig_ax.legend(loc='upper left')
        self.fig_ax.figure.canvas.draw()
        self.resize(None)

class MeanView(DataView):

    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.chan = np.arange(128)
        self.mean = np.full_like(self.chan, 0)
      
    def load_data(self,timestamps,samples):
        samples = samples[0] # [femb][channel][sample] -> [channel][sample]
        self.mean = np.mean(samples,axis=1)
        
    def plot_data(self,rescale=False):
        self.fig_ax.clear()
        
        self.fig_ax.plot(one_more_bin(self.chan),dupe_last_val(self.mean),drawstyle='steps-post',label='Mean',c='b')
        
        self.fig_ax.set_xlim(0,128)
        self.fig_ax.set_xlabel('Channel Number')
        self.fig_ax.set_ylabel('Mean ADC Counts')
        
        self.fig_ax.legend(loc='upper left')
        self.fig_ax.figure.canvas.draw()
        self.resize(None)

class Hist2DView(DataView):

    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.cb = None
        self.chan = np.arange(128)
        self.samples = np.linspace(0,16385,200)
        x,_ = np.meshgrid(self.chan,self.samples)
        self.counts = np.full_like(x,0)
      
    def load_data(self,timestamps,samples):
        #timestamps = self.data_source.timestamps[0]
        samples = samples[0] # [femb][channel][sample] -> [channel][sample]
        self.counts = []
        for i in self.chan:
            counts,bins = np.histogram(samples[i],bins=self.samples)
            self.counts.append(counts)
        self.counts = np.asarray(self.counts)
        
    def plot_data(self,rescale=False):
        ax = self.fig_ax
        ax.clear()
        if self.cb is not None:
            self.cb.remove()
        
        try:
            im = ax.imshow(self.counts.T,extent=(self.chan[0],self.chan[-1],self.samples[0],self.samples[-1]),
                          aspect='auto',interpolation='none',origin='lower',cmap=plt.get_cmap('GnBu'))
            self.cb = ax.figure.colorbar(im)
        except:
            print('Error plotting ADC count histogram')
        
        ax.set_title('Sample Histogram')
        ax.set_xlabel('Channel Number')
        ax.set_ylabel('ADC Counts')
        
        ax.figure.canvas.draw()
        #self.resize(None)

class FFTView(DataView):

    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        self.cb = None
        self.chan = np.arange(128)
        freq = np.fft.fftfreq(2184,320e-9) 
        freq_idx = np.argsort(freq)[len(freq)//2::]
        self.freq = freq[freq_idx]
        x,_ = np.meshgrid(self.chan,self.freq)
        self.fft = np.full_like(x,1)
        
    def load_data(self,timestamps,samples):
        #timestamps = self.data_source.timestamps[0]
        samples = samples[0] # [femb][channel][sample] -> [channel][sample]
        freq = np.fft.fftfreq(len(samples[0]),320e-9) 
        freq_idx = np.argsort(freq)[len(freq)//2::]
        self.freq = freq[freq_idx]
        self.fft = []
        for i in self.chan:
            fft = np.fft.fft(samples[i])
            self.fft.append(np.square(np.abs(fft[freq_idx])))
        self.fft = np.asarray(self.fft)
        self.fft[self.fft < 1e-4] = 1e-4 # To prevent log scaling from throwing errors
    
    def plot_data(self,rescale=False):
        ax = self.fig_ax
        ax.clear()
        if self.cb is not None:
            self.cb.remove()
        
        try:
            im = ax.imshow(self.fft.T[1:,:],extent=(self.chan[0],self.chan[-1],self.freq[0]/1000,self.freq[-1]/1000),
                          aspect='auto',interpolation='none',origin='lower',norm=LogNorm(),cmap=plt.get_cmap('Spectral_r'))
            self.cb = ax.figure.colorbar(im)
        except:
            print('Error plotting FFT power spectrum')
        
        ax.set_title('Power Spectrum')
        ax.set_xlabel('Channel Number')
        ax.set_ylabel('Frequency (kHz)')
        ax.figure.canvas.draw()
        self.resize(None)
        
class FEMB0Diagnostics(QtWidgets.QMainWindow):
    def __init__(self,wib_server='127.0.0.1',config='femb0.json',grid=False):
        super().__init__()
        
        self.wib = WIB(wib_server)
        self.config = config
        
        self._main = QtWidgets.QWidget()
        self._main.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.setCentralWidget(self._main)
        layout = QtWidgets.QVBoxLayout(self._main)
        
        self.grid = QtWidgets.QGridLayout()
        if grid:
            self.views = [Hist2DView(), FFTView(), MeanView(), RMSView()]
            for i,v in enumerate(self.views):
                self.grid.addWidget(v,i%2,i//2)
        else:
            self.views = [Hist2DView(), MeanRMSView(), FFTView()]
            for i,v in enumerate(self.views):
                self.grid.addWidget(v,0,i)
        layout.addLayout(self.grid)
        
        nav_layout = QtWidgets.QHBoxLayout()
        
        button = QtWidgets.QPushButton('Configure')
        nav_layout.addWidget(button)
        button.setToolTip('Configure WIB and front end')
        button.clicked.connect(self.configure_wib)
        
        button = QtWidgets.QPushButton('Acquire')
        nav_layout.addWidget(button)
        button.setToolTip('Read WIB Spy Buffer')
        button.clicked.connect(self.acquire_data)
        
        button = QtWidgets.QPushButton('Continuous')
        nav_layout.addWidget(button)
        button.setToolTip('Repeat acquisitions until stopped')
        button.clicked.connect(self.toggle_continuous)
        self.continuious_button = button
        
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.acquire_data)
        
        layout.addLayout(nav_layout)
        
        self.plot()
    
    @QtCore.pyqtSlot()
    def toggle_continuous(self):
        if self.continuious_button.text() == 'Continuous':
            self.continuious_button.setText('Stop')
            print('Starting continuous acquisition')
            self.timer.start(500)
        else:
            self.continuious_button.setText('Continuous')
            self.timer.stop()
    
    @QtCore.pyqtSlot()
    def acquire_data(self):
        data = self.wib.acquire_data(buf1=False)
        if data is None:
            return
            
        self.timestamps,self.samples = data
        for view in self.views:
            view.load_data(self.timestamps,self.samples)
            
        self.plot()
        
    @QtCore.pyqtSlot()
    def plot(self):
        for view in self.views:
            view.plot_data()
            
    @QtCore.pyqtSlot()
    def configure_wib(self):
        self.wib.configure(self.config)
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Visually display diagnostic data plots from FEMB0 on a WIB')
    parser.add_argument('--wib_server','-w',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
    parser.add_argument('--config','-C',default='configs/femb0.json',help='WIB configuration to load [configs/femb0.json]')
    parser.add_argument('--grid','-g',action='store_true',help='Split Mean/RMS into separate plots for a 2x2 grid')
    args = parser.parse_args()
    
    
    qapp = QtWidgets.QApplication([])
    qapp.setApplicationName('FEMB0 Diagnostic Tool (%s)'%args.wib_server)
    app = FEMB0Diagnostics(**vars(args))
    app.show()
    qapp.exec_()
