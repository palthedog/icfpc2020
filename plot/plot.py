import pylab

import sys
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
    except KeyboardInterrupt:
        sys.exit(1)
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

pylab.ion()

fig, axes = pylab.subplots()
line, = axes.plot(np.random.randn(100))

pylab.title("icfpc2020")

fig.canvas.draw()
fig.show()

conn = None

while True:    
    try:
        if conn:
            print('closing')
            conn.close()
            conn = None
            
        print('listening')
        conn, addr = s.accept()
        print('connected', addr)

        minX = 0
        maxX = 0
        minY = 0
        maxY = 0
        xs = []
        ys = []
        shouldClear = True
        layer = 0
        for line in linesplit(conn):
            #print line
            line = line.strip()
            #print 'line', line
            
            if re.search('START', line):                    
                print 'start drawing'
                layer += 1
                if shouldClear:
                    print 'clearing'
                    axes.clear()
                    axes.set_aspect('equal')
                    shouldClear = False
                    #axes.grid(True)
                    minX = minY = maxX = maxY = 0
                    layer = 0

                xs = []
                ys = []                    
            elif re.search('CLEAR', line):
                print('clear')
                shouldClear = True
            elif re.search('END', line):
                print 'end drawing'

                #axes.set_xlim(xmin = minP - pad, xmax=maxP + pad)
                #axes.set_ylim(ymax = minP - pad, ymin=maxP + pad)
                #axes.invert_yaxis()

                #line.set_ydata(np.random.randn(100))

                #c = pylab.cm.hsv(random.random())
                c = pylab.cm.hsv(layer * (1.1 / 5))
                square_scatter(axes, xs, ys, size = 1, alpha = 0.6, color=c)
        
                #pylab.draw()
                #fig.canvas.blit(axes.bbox)
                #fig.canvas.flush_events()
            elif re.search('FLUSH', line):
                print 'flush'
                # Cleanup lim
                pad = 3
                axes.set_xlim(xmin = minX - pad, xmax=maxX + pad)
                axes.set_ylim(ymax = minY - pad, ymin=maxY + pad)
                    
                fig.canvas.blit(axes.bbox)
                fig.canvas.flush_events()
                
            elif re.search('READ', line):
                clicked = False
                def onClick(event):
                    global clicked

                    x = event.xdata
                    y = event.ydata
                    print('clicked', x, y)
                    try:
                        mes = 'click %d %d' % (int(round(x)), int(round(y)));
                    except KeyboardInterrupt:
                        sys.exit(1)
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
                    
                    minX = min(minX, x)
                    maxX = max(maxX, x)
                    minY = min(minY, y)
                    maxY = max(maxY, y)
                    xs.append(x)
                    ys.append(y)
                    #print 'draw %d %d' % (x, y)
                else:
                    print('unknown command', line)
                    break
    except KeyboardInterrupt:
        sys.exit(0)
    except Exception as e:
        print('closed', e)
        conn.close()
        conn = None
