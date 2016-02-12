#ifndef __CINT__
#include "RooGlobalFunc.h"
#else
class RooNDKeysPdf;
#endif
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooAddPdf.h"
#include "RooGaussian.h"
#include "RooFormulaVar.h"
#include "../../../../PhysicsTools/TagAndProbe/interface/RooCMSShape.h"
#include "TCanvas.h"
#include "TAxis.h"
#include "RooPlot.h"
#include "TMath.h"
#include "TF1.h"
#include "TH1.h"
#include "TString.h"
#include "TStyle.h"
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "Math/DistFunc.h"
#include <vector>
#ifndef __CINT__
#include "RooCFunction1Binding.h" 
#include "RooCFunction3Binding.h"
#endif
#include "RooTFnBinding.h" 

using namespace RooFit;
using namespace std;

struct PeakInfo {
  double mean;
  double meanError;
  
  double sigma;
  double sigmaError;

  double norm;
  double normError;
};

bool comparePeaks(PeakInfo a, PeakInfo b) { return (a.mean < b.mean); }

