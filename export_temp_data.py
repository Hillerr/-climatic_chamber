from chamber_interface import ClimaticChamber
import time, csv, datetime


ip = "192.168.15.45"
port = 80
file_name = "temp_36_dt_off.csv"

T = 1
t = 0

if __name__ == "__main__":
    cc = ClimaticChamber(ip, port)

    if not cc.is_connected():
        print("Could not connect to server")
        exit(-1)
    
    with open(file_name, 'w') as temp_data:
        temp_data_writer = csv.writer(temp_data, delimiter=',')

        while 1:
            try:

                temp = cc.get_current_temp()

                print("Time: ", datetime.datetime.now().time().isoformat(), "\tTemp (C): ", temp)

                temp_data_writer.writerow([str(t*T), str(temp)])
                time.sleep(T)
                t += 1

            except KeyboardInterrupt:
                temp_data.close()
                print("Ending data aquisition")
                exit()