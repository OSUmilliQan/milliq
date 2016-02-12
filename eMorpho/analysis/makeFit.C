#include "SimpleSum.C"

void makeFit() {

  gROOT->LoadMacro("SimpleSum.C+");

  TString folder = "../data/";
  
  //TString file = "Jan27_dark.root";
  TString file = "Jan27_led_220ns_1.6v.root";
  TString noiseFile = "Jan27_dark_noHV.root";
  
  double xlo = 15.0;
  double xhi = 215.0;

  int npks = 6;

  doFit(folder + file, folder + noiseFile, npks, xlo, xhi);
}
