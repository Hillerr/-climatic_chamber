import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import pandas as pd
from pretty_html_table import build_table


class RoutineExporter:
    def __init__(self):
        self.plot = None
        self.plot_file_path = "../../"
        self.plot_file_name = "routine_plot.pdf"
        self.report = None
        self.report_file_name = None
        self.report_file_path = None
        self.plot_status = False
        self.report_status = False


    def enable_export_report(self):
        self.report_status = True


    def disable_export_report(self):
        self.report_status = True


    def enable_export_plot(self):
        self.plot_status = True


    def disable_export_plot(self):
        self.plot_status = False


    def export(self, routine, time, curr_temps, target_temps):
        self.export_temp_plot(time, curr_temps, target_temps)
        self.export_report(routine, time, curr_temps, target_temps)


    def export_temp_plot(self, time, curr_temps, target_temps):
        if self.plot_status:
            fig = plt.figure()
            plt.plot(time, curr_temps, label="Current temperature")
            plt.plot(time, target_temps, alpha=.3, label="Target temperature")
            plt.legend(loc="upper left")
            plt.xlabel("Time")
            plt.ylabel("Temperature (°C)", rotation='vertical')
            plt.ylim([20, 100])
            plt.grid()
            pp = PdfPages(self.plot_file_name)
            pp.savefig(fig)
            pp.close()


    def export_report(self, routine, time, curr_temps, target_temps):
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
        f = open("routine_report.html", "w")
        f.write(html_pretty)
        f.close()

        


