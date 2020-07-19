import matplotlib.pyplot as plt
import matplotlib.patches as pt

import re
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

    try:
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
        while True:
            
            for line in linesplit(conn):
                print line
                line = line.strip()
                print 'line'
                print line
                
                if re.search('START', line):
                    print 'start drawing'
                    # plt.clf()
                    xs = []
                    ys = []

                elif re.search('CLEAR', line):
                    plt.clf()
                    plt.title("icfpc2020")
                    plt.axes().set_aspect('equal')
                    plt.grid(True)
                elif re.search('END', line):
                    print 'end drawing'
                    plt.scatter(xs, ys, alpha = 0.6)
                    plt.draw()
                    plt.pause(0.01)                    
                elif re.search('READ', line):
                    def onClick(event):
                        x = event.xdata
                        y = event.ydata
                        print('clicked', x, y)
                        mes = 'click %d %d' % (int(round(x)), int(round(y)));
                        print('mes: ' + mes)
                        conn.send(mes)
                    cid = plt.connect('button_press_event', onClick)
                    print('waiting for click')
                    plt.waitforbuttonpress()
                    plt.disconnect(cid)
                elif re.search('DRAW', line):
                    print 'cmd', line
                    params = line.split(' ')
                    print params
                    if len(params) == 3:
                        d, x, y = params
                        x = int(x)
                        y = int(y)
                        xs.append(x)
                        ys.append(y)
                        print 'draw %d %d' % (x, y)
                    else:
                        break
    except socket.error:
        conn.close()
        print('closed')
