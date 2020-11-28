import http
import requests
import socket

class ClimaticChamber:
    """Climatic Chamber class

    This class will be responsible for supplying a interface
    for the User and the chamber.
    """
    _curr_temp_uri = "/actual"
    _target_temp_uri = "/target"
    _room_temp_uri = "/room"

    def __init__(self, ip, port):
        """ClimaticChamber class constructor method.

        Parameters:
            ip (str): The ipv4 chamber http server address
            port (int): Chamber http server port
        """
        self.ip = ip 
        self.port = port


    @property
    def ip(self):
        "str: The ipv4 chamber http server address"
        return self._ip


    @ip.setter
    def ip(self, val):
        try:
            socket.inet_aton(val)
        except Exception:
            raise ValueError(f"{val} is not a valid IPv4 address.")
        else:
            self._ip = val 


    @property
    def port(self):
        """int: Chamber http server port"""
        return self._port


    @port.setter
    def port(self, val):
        if not isinstance(val, int) or val < 0:
            raise ValueError(f"Param 'port' must be an integer, you passed {val}, a {val.__class__.__name__} object.")
        self._port = val 


    def is_connected(self):
        """Verify if the server is active

        Parameters:
            None

        Returns:
            bool: True if it's active, false otherwise
        """
        try:
            res = requests.get(f"http://{self.ip}:{self.port}{self._curr_temp_uri}")
        except requests.Timeout:
            return False

        if res.status_code == http.HTTPStatus.OK:
            return True

        else:
            return False

    def get_current_temp(self):
        """Get chamber inside current temperature

        Parameters:
            None

        Returns:
            float: the temperature value
        """
        res = requests.get(f"http://{self.ip}:{self.port}{self._curr_temp_uri}")

        return float(res.text)


        




