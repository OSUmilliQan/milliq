#ifndef SIMPLESUM_H
#define SIMPLESUM_H

#ifndef __CINT__
#include "RooGlobalFunc.h"
#else
class RooNDKeysPdf;
#endif

#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooAddPdf.h"
#include "RooRealProxy.h"
#include "RooGaussian.h"
#include "RooFormulaVar.h"
#include "TMath.h"
#include "RooMath.h"
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
//#include "PhysicsTools/TagAndProbe/interface/RooCMSShape.h"

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

// RooCMSShape definition from PhysicsTools/TagAndProbe/interface/RooCMSShape.h

#ifndef ROO_CMS_SHAPE
#define ROO_CMS_SHAPE

bool comparePeaks(PeakInfo a, PeakInfo b) { return (a.mean < b.mean); }

class RooCMSShape : public RooAbsPdf {
 public:
  RooCMSShape() {};
  RooCMSShape(const char *name, const char *title,
              RooAbsReal& _x,
              RooAbsReal& _alpha,
              RooAbsReal& _beta,
              RooAbsReal& _gamma,
              RooAbsReal& _peak);

  RooCMSShape(const RooCMSShape& other, const char* name);
  inline virtual TObject* clone(const char* newname) const { return new RooCMSShape(*this,newname); }
  inline ~RooCMSShape() {}
  Double_t evaluate() const ;


  ClassDef(RooCMSShape,1);

 protected:

  RooRealProxy x ;
  RooRealProxy alpha ;
  RooRealProxy beta ;
  RooRealProxy gamma ;
  RooRealProxy peak ;

};

#endif

ClassImp(RooCMSShape) RooCMSShape::RooCMSShape(const char *name, const char *title,
					       RooAbsReal& _x,
					       RooAbsReal& _alpha,
					       RooAbsReal& _beta,
					       RooAbsReal& _gamma,
					       RooAbsReal& _peak) :
		      RooAbsPdf(name,title),
			x("x","x",this,_x),
			alpha("alpha","alpha",this,_alpha),
			beta("beta","beta",this,_beta),
			gamma("gamma","gamma",this,_gamma),
			peak("peak","peak",this,_peak)
		      { }

RooCMSShape::RooCMSShape(const RooCMSShape& other, const char* name) : RooAbsPdf(other,name),
			x("x",this,other.x),
			alpha("alpha",this,other.alpha),
			beta("beta",this,other.beta),
			gamma("gamma",this,other.gamma),
			peak("peak",this,other.peak)
{ }

Double_t RooCMSShape::evaluate() const {
  // ENTER EXPRESSION IN TERMS OF VARIABLE ARGUMENTS HERE

  //Double_t erf = TMath::Erfc((alpha - x) * beta);
  Double_t erf = RooMath::erfc((alpha - x) * beta);
  Double_t u = (x - peak)*gamma;

  if(u < -70) u = 1e20;
  else if( u>70 ) u = 0;
  else u = exp(-u);   //exponential decay
  return erf*u;
}

#endif
