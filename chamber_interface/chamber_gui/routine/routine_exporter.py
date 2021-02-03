import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import pandas as pd
from pretty_html_table import build_table


class RoutineExporter:
    def __init__(self):
        self.plot_file_name = "routine_plot.pdf"
        self.report_file_name = "routine_report.html"
        self.data_file_name ="routine_data.csv"
        self.export_path = "."
        self.plot_status = False
        self.report_status = False


    def configure_path(self, path):
        if len(path) > 0:
            self.export_path = path


    def enable_export_report(self):
        self.report_status = True


    def disable_export_report(self):
        self.report_status = True


    def enable_export_plot(self):
        self.plot_status = True


    def disable_export_plot(self):
        self.plot_status = False


    def is_activated(self):
        if self.plot_status or self.report_status:
            return True
        else:
            return False


    def export(self, routine, time, curr_temps, target_temps):
        self.export_temp_plot(time, curr_temps, target_temps)
        self.export_report(routine, time, curr_temps, target_temps)


    def export_temp_plot(self, time, curr_temps, target_temps):
        if self.plot_status:
            fig = plt.figure()
            plt.plot(time, curr_temps, label="Current temperature")
            plt.plot(time, target_temps, alpha=.5, label="Target temperature")
            plt.legend(loc="upper left")
            plt.xlabel("Time")
            plt.ylabel("Temperature (°C)", rotation='vertical')
            plt.ylim([20, 100])
            plt.grid()
            pp = PdfPages(f"{self.export_path}/{self.plot_file_name}")
            pp.savefig(fig)
            pp.close()


    def export_report(self, routine, time, curr_temps, target_temps):
        if self.report_status:
            data = {
                "Target temperature (°C)": [],
                "Start time": [],
                "End time": [],
                "Duration (s)": [],
                "Transition time (s)": []
            }

            for i in routine:
                data["Target temperature (°C)"].append(i["temperature"])
                data["Start time"].append(i["start_time"])
                data["End time"].append(i["end_time"])
                data["Transition time (s)"].append(round(i["transition_time"], 1))
                data["Duration (s)"].append(round(i["duration"], 1))

            df = pd.DataFrame(data)
            html_pretty = build_table(df, color='green_dark')
            f = open(f"{self.export_path}/{self.report_file_name}", "w")
            f.write(html_pretty)
            f.close()

        


