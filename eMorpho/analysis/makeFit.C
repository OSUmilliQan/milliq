#include "SimpleSum.C"

void makeFit() {

  gROOT->LoadMacro("SimpleSum.C+");

  TString folder = "../data/";

  TString file = "SampleData_led.root";
  TString noiseFile = "SampleData_dark_noHV.root";

  double xlo = 5.0;
  double xhi = 200.0;

  int npks = 6;

  bool useNoise = false;
  bool logy = true;

  doFit(folder, file, noiseFile, useNoise, npks, xlo, xhi, logy);
}
