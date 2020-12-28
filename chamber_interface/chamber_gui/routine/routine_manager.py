from .routine_window_setup import Ui_RoutineCreator
from .routine_error_manager import RoutineErrorManager
from .routine_thread import RoutineThread
from .routine_data import RoutineData
from .routine_exporter import RoutineExporter
from .routine_file_manager import RoutineFileManager
from ..climatic_chamber import ClimaticChamber
from PyQt5 import QtWidgets, QtCore
from datetime import datetime
import time
import requests

class RoutineManager(QtWidgets.QDialog, Ui_RoutineCreator):

    port = 80

    def __init__(self, routine=[], ip=None):
        super(QtWidgets.QDialog, self).__init__()
        self.upper_limit = 2
        self.lower_limit = .5
        self.curr_temp = 0
        self.target_temp = 0
        self.transient = False
        self.status = False
        self.routine = routine
        self.curr_step = None
        self.last_step = None
        self.initial_time = None
        self.ip = ip
        self.routine_error_manager = RoutineErrorManager()
        self.routine_data = RoutineData()
        self.exporter = RoutineExporter()
        self.file_manager = RoutineFileManager()

        self.climatic_chamber = None
        self.threadpool = QtCore.QThreadPool()

        self.setupUi(self)
        self.configure_events()
        

    def new_routine(self):
        self.show()
        self.exec_()


    def open_routine(self):
        configs = self.file_manager.open_routine_file()

        if configs is not False:
            self.show()
            
            self.configure_routine(configs)
            self.exec_()


    def configure_events(self):
        self.r_button_add.clicked.connect(self.add_clicked)
        self.r_button_next.clicked.connect(self.next_clicked)
        self.r_button_cancel.clicked.connect(self.cancel_clicked)
        self.r_button_save.clicked.connect(self.save_clicked)
        self.r_plot_checkbox.stateChanged.connect(self.plot_export_clicked)
        self.r_report_checkbox.stateChanged.connect(self.report_export_clicked)


    def configure_routine(self, configs):
        for step in configs['routine']:
            self.add_routine(
                temp=step['temperature'],
                hour=step['hour'],
                minute=step['minute']
            )
        
        self.ip = configs['ip']
        self.r_text_ip.setText(self.ip)



    def update(self):
        if self.status:
            worker = RoutineThread(self.update_server)
            self.threadpool.start(worker)  
            self.update_routine()
            

    def update_server(self):
        try:
            if self.target_temp != self.curr_step['temperature']:
                self.climatic_chamber.set_target_temp(self.curr_step["temperature"], 2)
            
            self.target_temp = self.climatic_chamber.get_target_temp(2)
            self.curr_temp = self.climatic_chamber.get_current_temp(2)
            self.routine_data.add_temp(self.curr_temp, self.target_temp)

        except requests.exceptions.Timeout:
            print("Timeout")
            #self.routine_error_manager.print_error_custom_message("Server connection timeout")


    def get_target_temp(self):
        return self.target_temp


    def get_current_temp(self):
        return self.curr_temp


    def get_period_remaining(self):
        time_remaining = {}
        if not self.status:
            time_remaining = {
                'hour': '--',
                'minute': '--',
                'second': '--'
            }
        elif self.transient:
            time_remaining = {
                'hour': int(self.curr_step['total_time'] / 3600),
                'minute': int(self.curr_step['total_time'] / 60),
                'second': int(self.curr_step['total_time'] % 60)
            }

        else:
            time_seconds = self.curr_step['total_time'] - (time.time() - self.initial_time)
            time_remaining = {
                'hour': int(time_seconds / 3600),
                'minute': int(time_seconds / 60),
                'second': int(time_seconds % 60)
            }

        return time_remaining


    def get_routine_remaining(self):
        pass


    def display_routine(self, temp, minute, hour):
        self.r_text_browser.append(f"{len(self.routine)}. Temperature: {temp} °C\tDuration: {hour:02d}:{minute:02d}")


    def update_routine(self):
        # Verify if it's in temperature transition
        if self.transient:
            if self.curr_temp >= self.curr_step['temperature'] - self.lower_limit:
                self.transient = False
                self.initial_time = time.time()
                self.routine_data.register_step_transition_end()

        # Verify if it's step temperature needs to be updated
        elif self.curr_step['total_time'] <= (time.time() - self.initial_time):
            print('here')
            if self.curr_step != self.last_step:
                self.routine_data.register_step_end()
                self.curr_step = next(self.routine)
                print(self.curr_step)
                self.routine_data.register_step_start()
                self.transient = True

            else:
                self.routine_data.register_step_end()
                self.end_routine()
                self.curr_step = None


    def end_routine(self):
        self.status = False
        self.climatic_chamber.set_target_temp(0, 3)
        self.exporter.export(
            routine = self.routine_data.routine,
            time = self.routine_data.time,
            curr_temps = self.routine_data.curr_temp,
            target_temps = self.routine_data.target_temp
            )
        print(self.routine_data.routine)


    def connect(self):
        try:
            self.climatic_chamber = ClimaticChamber(self.ip, self.port)
        except ValueError:
            self.routine_error_manager.print_invalid_ip()
            return False
        
        if not self.climatic_chamber.is_connected():
            self.routine_error_manager.print_connection_error()
            return False

        else:
            return True


    def add_routine(self, temp, minute, hour):
        self.display_routine(temp, minute, hour)
        self.routine.append(
            {
                "hour": hour,
                "minute": minute,
                "temperature": temp,
                "total_time": (minute*60 + hour*3600)
            }
        )


    def add_clicked(self):
        temp = self.r_temp_spinbox.value()
        time = self.r_period_time.time()
        minute = time.minute()
        hour = time.hour()
        
        self.add_routine(temp, minute, hour)


    def next_clicked(self):
        self.ip = self.r_text_ip.toPlainText()
        
        if self.connect():
            self.initial_time = time.time()
            self.status = True
            self.transient = True
            self.last_step = self.routine[-1]

            self.routine_data.add_routine(self.routine)
            self.routine_data.register_step_start()

            self.routine = iter(self.routine)
            self.curr_step = next(self.routine)
            self.close()
        

    def cancel_clicked(self):
        self.close()
        self.routine = []
        self.status = False


    def save_clicked(self):
        ip = self.r_text_ip.toPlainText()

        if len(self.routine) == 0:
            self.routine_error_manager.print_error_custom_message(
                "You must add some temperature and its duration in order to save the routine."
            )  
        elif not ip:
            self.routine_error_manager.print_error_custom_message(
                "You must set the server IP address in order to save the routine."
            )
        else:
            self.file_manager.save_routine_file(self.routine, ip)


    def plot_export_clicked(self):
        if self.r_plot_checkbox.isChecked():
            self.exporter.enable_export_plot()
        else:
            self.exporter.disable_export_plot()


    def report_export_clicked(self):
        if self.r_report_checkbox.isChecked():
            self.exporter.enable_export_report()
        else:
            self.exporter.disable_export_report()
