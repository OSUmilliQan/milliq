# milliq

To install, follow the instructions appropriate for your system in geant/recipes/

You also must download the particle LHE data and edit the code to
reflect where you store them:

1) On OSUT3, copy /home/bfrancis/public/acceptances_v3.zip to
someplace in your working directory, and unzip it. If running on a
Windows machine, ask for a web link.

2) Edit milliq/geant4/src/MilliQPrimaryGeneratorAction.cc
Change the "pathname" variable to where you stored the data files:
std::string pathname="/path/to/workdir/acceptances_v3/mCP_UFO/"

3) Run make again in the build/ folder.
