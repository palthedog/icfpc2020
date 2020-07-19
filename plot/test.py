#import pylab
import matplotlib.pyplot as pylab

import numpy as np

def plot_to_buf(data, buffer, height=2800, width=2800, inc=0.3):
    xlims = (data[:,0].min(), data[:,0].max())
    ylims = (data[:,1].min(), data[:,1].max())
    dxl = xlims[1] - xlims[0]
    dyl = ylims[1] - ylims[0]

    print('xlims: (%f, %f)' % xlims)
    print('ylims: (%f, %f)' % ylims)

    for i, p in enumerate(data):
        x0 = int(round(((p[0] - xlims[0]) / dxl) * width))
        y0 = int(round((1 - (p[1] - ylims[0]) / dyl) * height))

        buffer[y0, x0] = 10
    return xlims, ylims, buffer

pylab.ion()
pylab.show()

h = 1000
w = 1000

pylab.title('Latent space')

while True:
    #data = np.random.randint(1000, size = (100, 2))
    buffer = np.random.rand(h+1, w+1)
    #buffer[data] = 1.0

    #xlims, ylims, I = plot_to_buf(data, buffer, height=h, width=w, inc=0.3)
    #ax_extent = list(xlims)+list(ylims)

    print('draw')
    
    pylab.imshow(
        buffer,
        cmap = "gray",
        interpolation=None
        #cmap=pylab.get_cmap('hot'),
        #interpolation='lanczos',
        #aspect='auto',
        #extent=ax_extent
        )
    #pylab.pcolor(data, cmap=pylab.cm.Blues)
    #pylab.grid(alpha=0.2)

    #pylab.show()
    pylab.draw()
    print('draw')
    pylab.pause(0.01)
