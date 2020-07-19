import pylab

import re
import numpy as np

import socket
import random

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(
    socket.SOL_SOCKET,
    socket.SO_REUSEADDR,
    1)
s.bind(('127.0.0.1', 15151))
s.listen(10)

def square_scatter(axes, x_array, y_array, size=0.5, **kwargs):
    size = float(size)
    for x, y in zip(x_array, y_array):
        square = pylab.Rectangle((x-size/2,y-size/2), size, size, **kwargs)
        axes.add_patch(square)
    return True

def linesplit(socket):
    try:
        buffer = socket.recv(4096)
    except:
        print 'recv failed'
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

while True:
    print('listening')
    conn, addr = s.accept()

    print('connected', addr)
    pylab.ion()

    try:
        minP = 0
        maxP = 0
        xs = []
        ys = []
        axes = pylab.axes()
        shouldClear = True
        for line in linesplit(conn):
            print line
            line = line.strip()
            #print 'line', line
            
            if re.search('START', line):                    
                print 'start drawing'
                if shouldClear:
                    print 'clearing'
                    pylab.clf()
                    pylab.title("icfpc2020")
                    axes = pylab.axes()
                    axes.set_aspect('equal')
                    shouldClear = False
                    axes.grid(True)
                xs = []
                ys = []                    
            elif re.search('CLEAR', line):
                print('clear')
                shouldClear = True
            elif re.search('END', line):
                print 'end drawing'
                c = pylab.cm.hsv(random.random())
                square_scatter(axes, xs, ys, size = 1, alpha = 0.6, color=c)

                minP = min(xs + ys + [minP])
                maxP = max(xs + ys + [maxP])
                pad = 3
                axes.set_xlim(xmin = minP - pad, xmax=maxP + pad)
                axes.set_ylim(ymin = minP - pad, ymax=maxP + pad)
                axes.invert_yaxis()

                pylab.draw()
                pylab.pause(0.1)                
            elif re.search('READ', line):
                clicked = False
                def onClick(event):
                    global clicked
                    
                    x = event.xdata
                    y = event.ydata
                    print('clicked', x, y)
                    try:
                        mes = 'click %d %d' % (int(round(x)), int(round(y)));
                    except:
                        # invalid click
                        return
                    print('mes: ' + mes)
                    conn.send(mes)
                    print('message sent')
                    clicked = True
                    
                cid = pylab.connect('button_press_event', onClick)
                print '\n\n\n\n'
                print'********************'
                print('waiting for click')
                print'********************'

                while not clicked:
                    pylab.pause(0.1)
                    #pylab.waitforbuttonpress()
                print('exit')                
                    
                pylab.disconnect(cid)
            elif re.search('DRAW', line):
                params = line.split(' ')
                #print params
                if len(params) == 3:
                    d, x, y = params
                    x = int(x)
                    y = int(y)
                    xs.append(x)
                    ys.append(y)
                    print 'draw %d %d' % (x, y)
                else:
                    break
    except:
        print('closed')
        conn.close()
