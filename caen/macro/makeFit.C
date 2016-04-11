#include "fit.C"

void makeFit() {

  gROOT->LoadMacro("fit.C+");

  TString file = "../Run_nai_cs137_6inches_ChargeHistos_3_30_2016.root";
  TString noiseFile = "../Run_nai_nosource_ChargeHistos_3_30_2016.root";

  int npks = 7;

  bool useNoise = false;
  bool logy = true;

  doFit(file, noiseFile, useNoise, npks, logy);

}
