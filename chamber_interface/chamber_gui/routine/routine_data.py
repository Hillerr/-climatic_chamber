import datetime, time

class RoutineData:
    def __init__(self):
        self.routine = []
        self.time = []
        self.target_temp = []
        self.curr_temp = []
        self.transition_start = None
        self.transition_end = None
        self.step_index = 0


    def add_routine(self, routine):
        for step in routine:
            self.routine.append(
                {
                    "hour": step['hour'],
                    "minute": step['minute'],
                    "temperature": step['temperature'],
                    "total_time": step['total_time'],
                    "duration": 0,
                    "transition_time": 0,
                    "start_time": 0,
                    "end_time": 0,
                }
            )


    def add_temp(self, curr_temp, target_temp, time=None):
        self.curr_temp.append(curr_temp)
        self.target_temp.append(target_temp)
        
        if time is not None:
            self.time.append(time)

        else:
            self.time.append(datetime.datetime.now())


    def register_step_start(self):
        self.routine[self.step_index]["start_time"] = datetime.datetime.now()
        self.transition_start = time.time()


    def register_step_transition_end(self):
        self.routine[self.step_index]["transition_time"] = time.time() - self.transition_start
        self.transition_end = time.time()


    def register_step_end(self):
        self.routine[self.step_index]["end_time"] = datetime.datetime.now()
        self.routine[self.step_index]["duration"] = time.time() - self.transition_start
        self.update_step()
        self.transition_start = 0
        self.transition_end = 0

    def update_step(self):
        if len(self.routine) - 1 > self.step_index:
            self.step_index += 1