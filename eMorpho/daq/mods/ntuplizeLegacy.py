from ROOT import TNtuple, TFile, TH1F
import os

dir = 'C:\\Users\\milliqan\\Desktop\\legacy_eMorpho_data'

def main():

    print '\nLooking in: ', dir

    listOutput = open('C:\\Users\\milliqan\\Desktop\\listOfLegacyNtuples.txt', 'w')

    for folder in os.listdir(dir):
        if folder == 'old':
            continue

        for subfolder in os.listdir(dir + '\\' + folder):

            if not os.path.isdir(dir + '\\' + folder + '\\' + subfolder):
                continue

            thisPath = dir + '\\' + folder + '\\' + subfolder + '\\'

            output = TFile(thisPath + 'ntuple.root', "RECREATE")

            h_energy = TH1F('energy', 'Energy distribution', 4096, 0, 4096)
            h_time = TH1F('time', 'Time distribution', 10000, 0, 96000000000)
            t_ntuple = TNtuple('ntuple', 'eMorpho data ntuple', 'energy:time:rollover:correctedTime')

            for file in os.listdir(thisPath):
                if file.endswith(".lmdat") or file.endswith(".rlmdat"):
                    currentFile = thisPath + file

                    print 'ntuplizing ', currentFile
                    makeRoot(currentFile, output, h_energy, h_time, t_ntuple)

                    h_energy.Write()
                    h_time.Write()
                    t_ntuple.Write()
                    output.Close()

                    listOutput.write(thisPath + file + '\n')

    listOutput.close()

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

main()
