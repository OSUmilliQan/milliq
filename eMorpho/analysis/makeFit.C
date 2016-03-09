#include "SimpleSum.C"

void makeFit() {

  gROOT->LoadMacro("SimpleSum.C+");
  
  TString file = "SampleData_led.root";
  TString noiseFile = "SampleData_dark_noHV.root";
  
  double xlo = 15.0;
  double xhi = 215.0;

  int npks = 6;

  bool useNoise = false;
  bool logy = true;
  
  doFit(file, noiseFile, useNoise, npks, xlo, xhi, logy);
}
