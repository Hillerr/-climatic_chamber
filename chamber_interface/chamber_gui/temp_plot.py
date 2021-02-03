from PyQt5 import QtWidgets
from matplotlib.figure import Figure
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib import dates
import matplotlib
import datetime

matplotlib.use('QT5Agg')

class MatplotWidget(QtWidgets.QWidget):

    def __init__(self, parent=None, max_samples=1000, ylim=[20, 100]):
        super(MatplotWidget, self).__init__(parent)
        self.figure = Figure(tight_layout=True)

        self.canvas = FigureCanvas(self.figure)
        self.axis = self.figure.add_subplot(111)
        self.layoutvertical = QtWidgets.QVBoxLayout(self)
        self.layoutvertical.addWidget(self.canvas)
        self.max_samples = max_samples
        self.time = []
        self.curr_temp = []
        self.target_temp = []
        self.ylim = ylim
        self.formatter = dates.DateFormatter("%H:%M")
        


    def update_plot(self, current, target):
        now = datetime.datetime.now()

        if len(self.time) > self.max_samples:
            self.curr_temp = self.curr_temp[1:] + [current]
            self.target_temp = self.target_temp[1:] + [target]
            self.time = self.time[1:] + [now]
        else:
            self.curr_temp.append(current)
            self.target_temp.append(target)
            self.time.append(now)

        self.axis.clear()
        self.axis.xaxis.set_major_formatter(self.formatter) 
        self.axis.plot(self.time, self.curr_temp, label="Current temperature")
        self.axis.plot(self.time, self.target_temp, alpha=0.6, label="Target temperature") 
        self.axis.grid()
        self.axis.legend(loc="upper left")
        self.axis.set_ylim(self.ylim)
        self.axis.set_xlabel("Time (HH:MM)")
        self.axis.set_ylabel("Temperature (°C)")
        self.canvas.draw()
