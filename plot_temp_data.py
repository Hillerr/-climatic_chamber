import matplotlib.pyplot as plt 
import csv
import pandas as pd


file = "data_aquisition/temp_control_dt10.csv"
file_2 = None
#file_2 = "temp_50_dt_off.csv"

if __name__ == "__main__":
    if file:
        temp_data = pd.read_csv(file, delimiter=',')
        plt.figure()
        plt.plot(temp_data.iloc[:, 0], temp_data.iloc[:, 1])
        plt.grid()

    if file_2:
        temp_data = pd.read_csv(file_2, delimiter=',')
        plt.figure()
        plt.plot(temp_data.iloc[:, 0], temp_data.iloc[:, 1])
        plt.grid()

    plt.show()
