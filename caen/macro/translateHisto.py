import os
import sys
import math

import ROOT
from array import array

def translate(filename):

    x1 = []
    y1 = []

    xall = []
    yall = []

    channels = []

    isX = True

    with open(filename, 'r') as f:

        for line in f:

            if "CHANNEL" in line:
		channels.append(int(line.split()[3]))
                isX = True
                continue

            if "=" in line:
                continue

            temp = line.split()

            for j in range(len(temp)):
                if isX:
                    x1.append(-1. * float(temp[j]))
                else:
                    y1.append(float(temp[j]))

            if not isX:
                xall.append(x1)
                yall.append(y1)
                x1 = []
                y1 = []
            else:
                isX = False

    outName = os.path.basename(filename).rsplit('.')[0] + '.root'
    output = ROOT.TFile(outName, 'RECREATE')

    for ch in range(len(channels)):

        x = array("d", xall[ch])
        y = array("d", yall[ch])

	histname = 'Channel_' + str(channels[ch])

        xlo = x[-1]
        xhi = x[0]
        nbins = len(x)

	h = ROOT.TH1D(histname, histname, nbins, xlo, xhi)
	h.Sumw2()

	for i in range(nbins):
            ibin = h.FindBin(x[i])
	    h.SetBinContent(ibin, y[i])
	    h.SetBinError(ibin, math.sqrt(y[i]))

	h.Write(histname)

    output.Close()

translate(sys.argv[1])
