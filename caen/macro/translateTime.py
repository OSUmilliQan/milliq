import os
import sys
import math

import ROOT

def translate(filename):

    x1 = []
    y1 = []

    isX = True

    with open(filename, 'r') as f:

        for line in f:

            temp = line.split()

            for j in range(len(temp)):
                if isX:
                    x1.append(float(temp[j]))
                else:
                    y1.append(float(temp[j]))

            if not isX:
		break
            else:
                isX = False

    outName = os.path.basename(filename).rsplit('.')[0] + '.root'
    output = ROOT.TFile(outName, 'RECREATE')

    nbins = len(x1)
    xlo = x1[0]
    xhi = x1[-1]

    hist = ROOT.TH1D('htime', 'htime', nbins, xlo, xhi)
    hist.Sumw2()

    for ibin in range(nbins):
	hist.SetBinContent(ibin+1, y1[ibin])
	hist.SetBinError(ibin+1, math.sqrt(y1[ibin]))

    output.Write()
    output.Close()

translate(sys.argv[1])
