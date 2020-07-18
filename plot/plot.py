import matplotlib.pyplot as plt
import matplotlib.patches as pt

import numpy as np

import socket
from random import randint, random

plt.title("icfpc2020")
plt.xlabel("x")
plt.ylabel("y")

plt.grid(True)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(
    socket.SOL_SOCKET,
    socket.SO_REUSEADDR,
    1,
    )

#plt.ion()
plt.axes().set_aspect('equal')

s.bind(('127.0.0.1', 15151))
s.listen(10)
while True:
    conn, addr = s.accept()

    print('connected')

    def onClick(event):
        print(clicked, event)

    #cid = fig.canvas.mpl_connect('button_press_event', onClick)
        
    f = conn.makefile()
    for line in f.readlines():
        line = line.strip()
        print 'line'
        print line
        if line == "START":
            plt.clf()
            continue
        elif line == "END":
            break
        else:
            params = line.split(' ')
            if len(params) == 3:
                d, x, y = params
                x = int(x)
                y = int(y)
                print 'draw %d %d' % (x, y)
                plt.scatter(x, y)
    plt.show()

    conn.close()
    print('closed')
