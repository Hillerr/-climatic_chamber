from .routine_error_setup import Ui_ErrorMessage
from PyQt5 import QtCore, QtGui, QtWidgets


class RoutineErrorManager(QtWidgets.QDialog, Ui_ErrorMessage):

    invalid_ip_message = "Your IP value is invalid. Please, verify it."
    connection_error_message = "Could not connect to the server."

    def __init__(self):
        super(QtWidgets.QDialog, self).__init__()
        self.setupUi(self)
        self.button_ok.clicked.connect(self.ok_clicked)


    def print_connection_error(self):
        self.textBrowser.setText(self.connection_error_message)
        self.show()
        self.exec_()


    def print_invalid_ip(self):
        self.textBrowser.setText(self.invalid_ip_message)
        self.show()
        self.exec_()


    def print_error_custom_message(self, message):
        self.textBrowser.setText(message)
        self.show()
        self.exec_()


    def ok_clicked(self):
        self.close()