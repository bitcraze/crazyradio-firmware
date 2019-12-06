# Turn on modulated

from crazyradio import Crazyradio
import sys
import struct

cr = Crazyradio()
cr.set_channel(40)
cr.set_data_rate(cr.DR_2MPS)
cr.set_arc(0)
cr.set_mode(cr.MODE_MOD_CARRIER)
