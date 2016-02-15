#include "SimpleSum.C"

void makeFit() {

  gROOT->LoadMacro("SimpleSum.C+");

  TString folder = "../data/";
  
  //TString file = "SampleData_dark.root";
  TString file = "SampleData_led.root";
  TString noiseFile = "SampleData_dark_noHV.root";
  
  double xlo = 15.0;
  double xhi = 215.0;

  int npks = 6;

  doFit(folder + file, folder + noiseFile, npks, xlo, xhi);
}
