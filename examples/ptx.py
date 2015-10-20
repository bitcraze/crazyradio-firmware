import sys
sys.path.append("../lib")
from crazyradio import Crazyradio

radio = Crazyradio()
radio.set_channel(100)
radio.set_data_rate(Crazyradio.DR_250KPS)
radio.set_mode(Crazyradio.MODE_PTX)
for i in range(0, 100):
	res = radio.send_packet([i])
	print(res.data)
radio.close()
