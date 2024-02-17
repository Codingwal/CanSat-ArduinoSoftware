from pyftdi.ftdi import Ftdi

Ftdi.show_devices() 

d = Ftdi.open()

print(d.getDeviceInfo())