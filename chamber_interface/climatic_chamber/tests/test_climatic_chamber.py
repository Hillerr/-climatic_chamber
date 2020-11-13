import pytest
from climatic_chamber import ClimaticChamber

def test_climatic_chamber_init():
    """
    Testing Climatic Chamber initialization
    """
    # Setup
    port = 80
    ip = "192.168.15.45"
    
    # Execute
    cc = ClimaticChamber(ip, port)

    # Verify
    assert cc.port == port and cc.ip == ip


def test_climatic_chamber_init_invalid_ip():
    """Testing Climatic Chamber initialization
    with invalid args
    """
    port = 80
    ip = "192.168.15.45.21"

    with pytest.raises(ValueError):
        cc = ClimaticChamber(ip, port)


def test_climatic_chamber_init_invalid_port():
    """Testing Climatic Chamber initialization
    with invalid args
    """
    port = "80"
    ip = "192.168.15.45"

    with pytest.raises(ValueError):
        cc = ClimaticChamber(ip, port)


def test_climatic_chamber_is_connected_method():
    """Testing the is connected method to verify 
    if http server is active

    Note that the ESP32 application must be running. 
    """
    # Setup
    port = 80
    ip = "192.168.15.45"
    
    # Execute
    cc = ClimaticChamber(ip, port)

    # Verify
    assert cc.is_connected() == True