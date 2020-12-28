from PyQt5 import QtWidgets

class RoutineFileManager(QtWidgets.QDialog):

    def save_routine_file(self, routine, server_ip):
        fname = QtWidgets.QFileDialog.getSaveFileName(self, "Save routine", filter="Routine Files (*.rtn)")
        f_content = {
            "routine": [],
            "ip": server_ip
        }

        for step in routine:
            f_content['routine'].append(
                {
                    "hour": step['hour'],
                    "minute": step['minute'],
                    "temperature": step['temperature'],
                }
            )

        if fname[1] == 'Routine Files (*.rtn)' and fname[0][-3:] != '.rtn':
            fname = fname[0] + '.rtn'
        else:
            fname = fname[0]

        if fname[0]:
            f = open(fname, 'w')
            f.write(str(f_content))
            f.close()



    def open_routine_file(self):
        fname = QtWidgets.QFileDialog.getOpenFileName(self, 'Open file', filter="Routine files (*.rtn)")

        if fname[0]:
            f = open(fname[0], 'r')
            config = f.read()
            config = eval(config)
            f.close()

        else:
            return False

        return {'ip': config['ip'], 'routine': config['routine']}
