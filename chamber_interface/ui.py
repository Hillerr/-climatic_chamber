from chamber_gui import ChamberMainWindow
from PyQt5 import QtCore, QtGui, QtWidgets
        

if __name__ == '__main__':
    import sys
    app = QtWidgets.QApplication(sys.argv)
    ui = ChamberMainWindow()
    ui.show()
    sys.exit(app.exec_())