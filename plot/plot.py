import matplotlib.pyplot as plt
import matplotlib.patches as pt

import numpy as np

import socket
from random import randint, random

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(
    socket.SOL_SOCKET,
    socket.SO_REUSEADDR,
    1,
    )

#plt.ion()

s.bind(('127.0.0.1', 15151))
s.listen(10)
plt.ion()

while True:
    print('listening')
    conn, addr = s.accept()

    print('connected')

    #cid = fig.canvas.mpl_connect('button_press_event', onClick)

    def linesplit(socket):
        buffer = socket.recv(4096)
        buffering = True
        while buffering:
            if "\n" in buffer:
                (line, buffer) = buffer.split("\n", 1)
                yield line
            else:
                more = socket.recv(4096)
                if not more:
                    buffering = False
                else:
                    buffer += more
        if buffer:
            yield buffer

    xs = []
    ys = []
            
    for line in linesplit(conn):
        line = line.strip()
        print 'line'
        print line
        
        if line == "START":
            # plt.clf()
            
            plt.title("icfpc2020")
            plt.axes().set_aspect('equal')
            plt.grid(True)
            
            continue
        elif line == "END":
            break
        else:
            params = line.split(' ')
            if len(params) == 3:
                d, x, y = params
                x = int(x)
                y = int(y)
                xs.append(x)
                ys.append(y)
                print 'draw %d %d' % (x, y)

        plt.scatter(xs, ys)

    def onClick(event):
        x = event.xdata
        y = event.ydata
        print('clicked', x, y)
        conn.send('click %d %d' % (int(x), int(y)))
                
    cid = plt.connect('button_press_event', onClick)

    plt.draw()
    plt.waitforbuttonpress()

    plt.disconnect(cid)
    
    
    conn.close()
    print('closed')
