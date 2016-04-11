#ifndef FIT_H
#define FIT_H

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

  ClassDef(RooCMSShape, 1);

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

#ifndef ROO_VOIGTIAN
#define ROO_VOIGTIAN

class RooVoigtian : public RooAbsPdf {
 public:
  RooVoigtian() {};
  RooVoigtian(const char * name, const char * title,
	      RooAbsReal& _x, RooAbsReal& _mean,
	      RooAbsReal& _width, RooAbsReal& _sigma,
	      Bool_t doFast = kFALSE);
  RooVoigtian(const RooVoigtian& other, const char * name = 0);
  virtual TObject* clone(const char * newname) const { return new RooVoigtian(*this, newname); }
  inline virtual ~RooVoigtian() {}

  inline void selectFastAlgorithm() { _doFast = kTRUE; }
  inline void selectDefaultAlgorithm() { _doFast = kFALSE; }

 protected:
  RooRealProxy x;
  RooRealProxy mean;
  RooRealProxy width;
  RooRealProxy sigma;

  Double_t evaluate() const;

 private:
  Double_t _invRootPi;
  Bool_t _doFast;
  ClassDef(RooVoigtian, 1);
};

ClassImp(RooVoigtian)

//_____________________________________________________________________________
RooVoigtian::RooVoigtian(const char *name, const char *title,
			 RooAbsReal& _x, RooAbsReal& _mean,
			 RooAbsReal& _width, RooAbsReal& _sigma,
    			 Bool_t doFast) :
  RooAbsPdf(name,title),
  x("x","Dependent",this,_x),
  mean("mean","Mean",this,_mean),
  width("width","Breit-Wigner Width",this,_width),
  sigma("sigma","Gauss Width",this,_sigma),
  _doFast(doFast)
{
  _invRootPi= 1./sqrt(atan2(0.,-1.));
}

//_____________________________________________________________________________
RooVoigtian::RooVoigtian(const RooVoigtian& other, const char* name) : 
  RooAbsPdf(other,name), x("x",this,other.x), mean("mean",this,other.mean),
  width("width",this,other.width),sigma("sigma",this,other.sigma),
  _doFast(other._doFast)
{
  _invRootPi= 1./sqrt(atan2(0.,-1.));
}

//_____________________________________________________________________________
Double_t RooVoigtian::evaluate() const
{
  Double_t s = (sigma>0) ? sigma : -sigma ;
  Double_t w = (width>0) ? width : -width ;

  Double_t coef= -0.5/(s*s);
  Double_t arg = x - mean;

  // return constant for zero width and sigma
  if (s==0. && w==0.) return 1.;

  // Breit-Wigner for zero sigma
  if (s==0.) return (1./(arg*arg+0.25*w*w));

  // Gauss for zero width
  if (w==0.) return exp(coef*arg*arg);

  // actual Voigtian for non-trivial width and sigma
  Double_t c = 1./(sqrt(2.)*s);
  Double_t a = 0.5*c*w;
  Double_t u = c*arg;
  std::complex<Double_t> z(u,a) ;
  std::complex<Double_t> v(0.) ;

  if (_doFast) {
    v = RooMath::faddeeva_fast(z);
  } else {
    v = RooMath::faddeeva(z);
  }
  return c*_invRootPi*v.real();

}

#endif

#endif
