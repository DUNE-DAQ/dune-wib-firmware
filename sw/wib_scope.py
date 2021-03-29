#!/usr/bin/env python3

import os
import sys
import time
import pickle
import argparse
import numpy as np
from collections import deque

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
        ('Signals','Choose signal traces to show', 'choose', 'choose'),
        ('Autoscale', 'Autoscale axes for each new event', 'autoscale','autoscale'),
        ('Legend', 'Toggle legend', 'legend','legend'),
        (None, None, None, None),
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
        
    def choose(self):
        self.canvas.parent().select_signals()
        
    def legend(self):
        self.canvas.parent().legend = not self.canvas.parent().legend
        self.canvas.parent().plot_signals()
        
    def autoscale(self):
        self.canvas.parent().plot_signals(rescale=True)
        
from matplotlib.figure import Figure

class SignalSelector(QtWidgets.QDialog):
    def __init__(self,parent=None,selected=None,raw_adc=False,raw_time=False,pedestal=None,distribute=None,fft=False,**ignored):
        super().__init__(parent)
        
        self._layout = QtWidgets.QVBoxLayout(self)
        
        self.set_selected(selected)
        
        self.fft_checkbox = QtWidgets.QCheckBox('Plot FFT')
        self.fft_checkbox.setCheckState(QtCore.Qt.Checked if fft else QtCore.Qt.Unchecked)
        self._layout.addWidget(self.fft_checkbox)
        
        self.raw_checkbox = QtWidgets.QCheckBox('Plot raw ADC counts')
        self.raw_checkbox.setCheckState(QtCore.Qt.Checked if raw_adc else QtCore.Qt.Unchecked)
        self._layout.addWidget(self.raw_checkbox)
        
        self.raw_time_checkbox = QtWidgets.QCheckBox('Plot sample index')
        self.raw_time_checkbox.setCheckState(QtCore.Qt.Checked if raw_time else QtCore.Qt.Unchecked)
        self._layout.addWidget(self.raw_time_checkbox)
        
        redist_layout = QtWidgets.QHBoxLayout()
        self.redist_checkbox = QtWidgets.QCheckBox('Redistribute signals')
        self.redist_checkbox.setCheckState(QtCore.Qt.Checked if distribute else QtCore.Qt.Unchecked)
        redist_layout.addWidget(self.redist_checkbox)
        self.redist_amount = QtWidgets.QLineEdit('0' if distribute is None else str(distribute))
        redist_layout.addWidget(self.redist_amount)
        self._layout.addLayout(redist_layout)
        
        ped_layout = QtWidgets.QHBoxLayout()
        self.baseline_checkbox = QtWidgets.QCheckBox('Correct baselines')
        self.baseline_checkbox.setCheckState(QtCore.Qt.Checked if pedestal else QtCore.Qt.Unchecked)
        ped_layout.addWidget(self.baseline_checkbox)
        self.ped_min = QtWidgets.QLineEdit('0' if pedestal is None else str(pedestal[0]))
        self.ped_min.setFixedWidth(100)
        ped_layout.addWidget(self.ped_min)
        self.ped_max = QtWidgets.QLineEdit('50' if pedestal is None else str(pedestal[1]))
        self.ped_max.setFixedWidth(100)
        ped_layout.addWidget(self.ped_max)
        self._layout.addLayout(ped_layout)
        
        buttons = QtWidgets.QDialogButtonBox(QtWidgets.QDialogButtonBox.Ok | QtWidgets.QDialogButtonBox.Cancel, QtCore.Qt.Horizontal, self)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        self._layout.addWidget(buttons)
        
    def get_distribute(self):
        if self.redist_checkbox.checkState() == QtCore.Qt.Checked:
            return float(self.redist_amount.text())
        else:
            return None
        
    def get_pedestal(self):
        if self.baseline_checkbox.checkState() == QtCore.Qt.Checked:
            return int(self.ped_min.text()),int(self.ped_max.text())
        else:
            return None
        
    def get_raw_adc(self):
        return self.raw_checkbox.checkState() == QtCore.Qt.Checked
        
    def get_raw_time(self):
        return self.raw_time_checkbox.checkState() == QtCore.Qt.Checked
        
    def get_fft(self):
        return self.fft_checkbox.checkState() == QtCore.Qt.Checked
    
    def get_selected(self):
        selected = []
        root = self._tree.invisibleRootItem()
        it_stack = deque([((),root)])
        while len(it_stack) > 0:
            path,it = it_stack.popleft()
            for i in range(it.childCount()):
                item = it.child(i)
                name = str(item.text(0))
                fullpath = path+(i,)
                if item.childCount() > 0 and item.checkState(0) != QtCore.Qt.Unchecked:
                    it_stack.append((fullpath,item))
                elif item.checkState(0) == QtCore.Qt.Checked:
                    selected.append(fullpath)
        return selected
    
    
    def add_element(self,*label,parent=None,is_leaf=False,checked=True):
        label = ' '.join(map(str,label))
        elem = QtWidgets.QTreeWidgetItem(parent)
        elem.setText(0, label)
        if is_leaf:
            elem.setFlags(elem.flags() | QtCore.Qt.ItemIsUserCheckable)
            elem.setCheckState(0, QtCore.Qt.Checked if checked else QtCore.Qt.Unchecked)
        else:
            elem.setFlags(elem.flags() | QtCore.Qt.ItemIsTristate | QtCore.Qt.ItemIsUserCheckable)
        return elem
    
    def set_selected(self,previous_selection=None):
        self._tree = QtWidgets.QTreeWidget()
        self._tree.setHeaderLabel('Channels Shown')
        for i in range(4):
            femb = self.add_element('FEMB %i'%i,parent=self._tree)
            for j in range(8):
                adc = self.add_element('ADC %i'%j,parent=femb)
                for k in range(16):
                    if previous_selection:
                        checked = (i,j,k) in previous_selection
                    else:
                        checked = False
                    cg = self.add_element('CH %i (%i)'%(k,j*16+k),parent=adc,is_leaf=True,checked=checked)
        self._layout.addWidget(self._tree)
        
class SignalView(QtWidgets.QWidget):
    def __init__(self,parent=None,figure=None,data_source=None):
        super().__init__(parent=parent)
        if figure is None:
            figure = Figure(tight_layout=True)
        self.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.figure = figure
        self.data_source=data_source
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
        
        self.legend = False
        self.raw_adc = True
        self.raw_time = True
        self.pedestal = None
        self.distribute = None
        self.fft = False
        self.selected = None
        
        self.save_props = ['legend','selected','raw_adc','raw_time','pedestal','distribute','fft']
        
        self.autoscale = True
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
            
    def load_data(self):
        self.times = []
        self.data = []
        self.raw_data = []
        
        if self.data_source.timestamps is None or self.data_source.samples is None or self.selected is None:
            return
        
        for sig_idx,sel in enumerate(self.selected):
            femb,adc,ch = sel
            if self.raw_time:
                times = np.arange(self.data_source.timestamps.shape[-1])
            else:
                times = self.data_source.timestamps[femb//2]
            if not self.fft:
                self.times.append(times)
            samples = self.data_source.samples[femb,adc*16+ch]
            if self.pedestal is not None:
                ped_min,ped_max = self.pedestal
                i = ped_min if ped_min > 0 else 0
                j = ped_max if ped_max < len(times) else len(times)-1
                try:
                    pedestal = np.mean(samples[i:j])
                    samples = samples - pedestal
                except:
                    print('Pedestal correction failed')
                    pass
            if self.distribute is not None:
                samples = samples + sig_idx*self.distribute
            if not self.fft:
                self.data.append(samples)
            if self.fft:
                fft = np.fft.fft(samples)
                freq = np.fft.fftfreq(len(samples),1)
                idx = np.argsort(freq)[len(freq)//2::]
                
                self.times.append(freq[idx])
                self.data.append(np.square(np.abs(fft[idx])))
        
    def select_signals(self):
        current_props = {x:self.__dict__[x] for x in self.save_props}
        selector = SignalSelector(parent=self, **current_props)
        result = selector.exec_()
        self.selected = selector.get_selected()
        self.raw_adc = selector.get_raw_adc()
        self.raw_time = selector.get_raw_time()
        self.pedestal = selector.get_pedestal()
        self.distribute = selector.get_distribute()
        self.fft = selector.get_fft()
        self.load_data()
        self.plot_signals()
        
    def plot_signals(self,rescale=False):
        ax = self.fig_ax
        if rescale:
            self.autoscale = True
        else:
            next_lims = (ax.get_xlim(), ax.get_ylim())
            self.autoscale = self.autoscale and (next_lims == self.last_lims or self.last_lims is None)
        ax.clear()
        
        if self.selected:
            if not self.times or not self.data:
                self.load_data()
            
            if not self.times or not self.data:
                return
        
            for t,v,(femb,adc,ch) in zip(self.times,self.data,self.selected):
                label = 'FEMB%i ADC%i CH%i (%i)'%(femb,adc,ch,adc*16+ch)
                ax.plot(t,v,drawstyle='steps' if not self.fft else None,label=label)
                        
        if self.fft:
            ax.set_yscale('log')
            ax.set_xlabel('Frequency (1/sample)')
            ax.set_ylabel('Power Spectrum')
        else:
            ax.set_xlabel('Sample' if self.raw_time else 'Timestamp')
            ax.set_ylabel('ADC Counts' if self.raw_adc else ('Voltage (mV)' if not self.distribute else 'Arb. Shifted Voltage (mV)'))
        if not self.autoscale:
            ax.set_xlim(*next_lims[0])
            ax.set_ylim(*next_lims[1])
        self.last_lims = (ax.get_xlim(), ax.get_ylim())
        if self.legend:
            ax.legend()
            
        ax.figure.canvas.draw()
        self.resize(None)
        

class WIBScope(QtWidgets.QMainWindow):
    def __init__(self,wib_server='127.0.0.1',config='default.json',rows=1,cols=1,layout=None):
        super().__init__()
        plot_layout = layout
        
        self.wib = WIB(wib_server)
        self.config = config
        
        self.samples = None
        self.timestamps = None
        
        self._main = QtWidgets.QWidget()
        self._main.setFocusPolicy(QtCore.Qt.StrongFocus)
        self.setCentralWidget(self._main)
        layout = QtWidgets.QVBoxLayout(self._main)
        
        button_layout = QtWidgets.QHBoxLayout()
        
        button = QtWidgets.QPushButton('Reshape')
        button_layout.addWidget(button)
        button.setToolTip('Change the plot grid shape')
        button.clicked.connect(self.reshape_prompt)
        
        button = QtWidgets.QPushButton('Load Layout')
        button_layout.addWidget(button)
        button.setToolTip('Save plot layout and selected signals')
        button.clicked.connect(self.load_layout)
        
        button = QtWidgets.QPushButton('Save Layout')
        button_layout.addWidget(button)
        button.setToolTip('Load plot layout and selected signals')
        button.clicked.connect(self.save_layout)
        
        layout.addLayout(button_layout)
        
        self.grid = QtWidgets.QGridLayout()
        self.views = []
        self.reshape(rows,cols)
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
        button.clicked.connect(self.toggle_continuious)
        self.continuious_button = button
        
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.acquire_data)
        
        layout.addLayout(nav_layout)
        
        if plot_layout:
            self.load_layout(plot_layout)
        self.plot_selected()
    
    @QtCore.pyqtSlot()
    def reshape_prompt(self):
        dialog = QtWidgets.QDialog()
        layout = QtWidgets.QFormLayout()
        layout.addRow(QtWidgets.QLabel("Choose Plot Grid Shape"))
        rowbox,colbox = QtWidgets.QLineEdit(str(self.rows)),QtWidgets.QLineEdit(str(self.cols))
        layout.addRow(QtWidgets.QLabel("Rows"),rowbox)
        layout.addRow(QtWidgets.QLabel("Cols"),colbox)
        buttons = QtWidgets.QDialogButtonBox( QtWidgets.QDialogButtonBox.Ok | QtWidgets.QDialogButtonBox.Cancel, QtCore.Qt.Horizontal, dialog)
        buttons.accepted.connect(dialog.accept)
        buttons.rejected.connect(dialog.reject)
        layout.addWidget(buttons)
        dialog.setLayout(layout)
        if dialog.exec_() == QtWidgets.QDialog.Accepted:
            try:
                r = int(rowbox.text())
                c = int(colbox.text())
                self.reshape(r,c)
            except:
                print('Invalid input to reshape dialog')
        
    def reshape(self,rows,cols):
        self.rows = rows
        self.cols = cols
        for i in reversed(range(self.grid.count())): 
            self.grid.itemAt(i).widget().setParent(None)
        for r in range(self.rows):
            for c in range(self.cols):
                i = c+self.cols*r
                if i < len(self.views):
                    view = self.views[i]
                else:
                    view = SignalView(data_source=self)
                    self.views.append(view)
                self.grid.addWidget(view,r,c)
        for widget in self.views[self.rows*self.cols:]:
            widget.deleteLater() #how necessary is this, i wonder
        self.views = self.views[:self.rows*self.cols]
        self.plot_selected()
    
    @QtCore.pyqtSlot()
    def load_layout(self,fname=None):
        if fname is None:
            fname,_ = QtWidgets.QFileDialog.getOpenFileName(self, 'Open settings', '.','WIB Scope Plot Layouts (*.ply);;All files (*.*)')
        if fname:
            try:
                with open(fname,'rb') as f:            
                    settings = pickle.load(f)
                rows = settings['rows']
                cols = settings['cols']
                views = [SignalView(data_source=self) for i in range(rows*cols)]
                for view,state in zip(views,settings['views']):
                    view.set_state(state)         
                self.views = views
                self.reshape(rows,cols)
            except:
                print('Could not load settings')
    
    @QtCore.pyqtSlot()
    def save_layout(self):
        fname,_ = QtWidgets.QFileDialog.getSaveFileName(self, 'Save settings', '.','WIB Scope Plot Layouts (*.ply);;All files (*.*)')
        if fname:
            if not fname.endswith('.ply'):
                fname = fname + '.ply'
            settings = {'rows':self.rows,'cols':self.cols}
            settings['views'] = [v.get_state() for v in self.views]
            with open(fname,'wb') as f:
                pickle.dump(settings,f)
                
    @QtCore.pyqtSlot()
    def toggle_continuious(self):
        if self.continuious_button.text() == 'Continuous':
            self.continuious_button.setText('Stop')
            print('Starting continuous acquisition')
            self.timer.start(500)
        else:
            self.continuious_button.setText('Continuous')
            self.timer.stop()
    
    @QtCore.pyqtSlot()
    def acquire_data(self):
        self.timestamps,self.samples = self.wib.acquire_data(ignore_failure=True)
        
        for view in self.views:
            view.load_data()
            
        self.plot_selected()
        
    @QtCore.pyqtSlot()
    def plot_selected(self):
        for view in self.views:
            view.plot_signals()
            
    @QtCore.pyqtSlot()
    def configure_wib(self):
        self.wib.configure(self.config)
        

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Visually display data from a WIB')
    parser.add_argument('--wib_server','-w',default='127.0.0.1',help='IP of wib_server to connect to [127.0.0.1]')
    parser.add_argument('--config','-C',default=None,help='WIB configuration to load [defaults]')
    parser.add_argument('--rows','-r',default=1,type=int,help='Rows of plots [1]')
    parser.add_argument('--cols','-c',default=1,type=int,help='Columns of plots [1]')
    parser.add_argument('--layout','-l',default=None,help='Load a saved layout')
    args = parser.parse_args()
    
    
    qapp = QtWidgets.QApplication([])
    qapp.setApplicationName('WIB Scope (%s)'%args.wib_server)
    app = WIBScope(**vars(args))
    app.show()
    qapp.exec_()
