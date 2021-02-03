from chamber_interface.chamber_gui.climatic_chamber import ClimaticChamber
import time, csv, datetime


ip = "192.168.15.200"
port = 80
file_name = "data_aquisition/temp_control_dt10.csv"

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
                target = cc.get_target_temp()

                print("Time: ", datetime.datetime.now().time().isoformat(), "\tTemp (C): ", temp, "\tTarget (C): ", target)

                temp_data_writer.writerow([str(t*T), str(temp), str(target)])
                time.sleep(T)
                t += 1

            except KeyboardInterrupt:
                temp_data.close()
                print("Ending data aquisition")
                exit()