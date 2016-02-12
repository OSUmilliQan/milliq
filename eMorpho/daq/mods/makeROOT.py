from ROOT import TNtuple, TFile, TH1F
import os

dir = 'C:\\BPIsoft\\Python27\\data'

def ntuplize(folder):

    print '\nLooking in: ', dir
    folderName = folder
    dataDir = dir + '\\' + folderName + '\\'

    if not os.path.exists(dataDir):
        print '\nFolder not found!\n\n'
        exit()

    if os.path.exists(dataDir + folderName + ".root"):
        print '\nAlready nTuplized!\n\n'
        exit()

    output = TFile(dataDir + folderName + ".root", "RECREATE")

    h_energy = TH1F('energy', 'Energy distribution', 4096, 0, 4096)
    h_time = TH1F('time', 'Time distribution', 10000, 0, 96000000000)
    t_ntuple = TNtuple('ntuple', 'eMorpho data ntuple', 'energy:time:rollover:correctedTime')

    # Look at every file in directory
    for file in os.listdir(dataDir):
        if file.endswith(".dat") or file.endswith(".lmdat") or file.endswith("rlmdat"):
            currentFile = dataDir + file
            print 'nTuplizing ', currentFile
            makeRoot(currentFile, output, h_energy, h_time, t_ntuple)

    h_energy.Write()
    h_time.Write()
    t_ntuple.Write()
    output.Close()

def makeRoot(filein, fileout, h_energy, h_time, t_ntuple):

    bufferLength = 340
    f = open(filein)

    rollover = 0
    oldTime = 0
    while True:
        temp = f.readline().split(" ")
        if temp == ['']: #if the end of file is reached, break
            break
        for j in range(bufferLength):
            energy = float(temp[2*j + 6])/16.0 # get energy
            time = float(temp[2*j + 7]) # get time stamp
            if time < oldTime:
                rollover = rollover + 1
            oldTime = time
            timeCorrection = rollover * 2**32
            h_energy.Fill(energy)
            h_time.Fill(time + timeCorrection)
            t_ntuple.Fill(energy, time, rollover,time + timeCorrection)

    f.close()

def main():
    inputName = raw_input('Folder to ntuplize: ')
    ntuplize(inputName)

main()
