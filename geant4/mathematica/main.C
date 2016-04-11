#include "TMath.h"

const double massMuon = 0.105658;

const double widthJPsi = 5.93 / 100.;
const double massJPsi = 3.097;

const double widthY1S = 2.48 / 100.;
const double massY1S = 9.460;

const double widthY2S = 1.93 / 100.;
const double massY2S = 10.023;

const double widthY3S = 2.18 / 100.;
const double massY3S = 10.355;

double phaseSpaceRatio(double mmcp, mOnia, muBR) {
  
  double a = mmcp*mmcp / (mOnia*mOnia);
  double b = massMuon*massMuon / (mOnia*mOnia);
  
  double numerator = TMath::Sqrt(1. - 4*a) * (1. + 2*a);
  double denominator = TMath::Sqrt(1. - 4*b) * (1. + 2*b);

  return numerator / denominator;
}

double EffFunc(double aa, double bb, double Q) {

  double value = -1. * Q*Q / (bb*bb);
  value = 1. - TMath::Exp(value);
  value *= aa;

  return value*value*value;
}

void ExclusionCalc(double loc, double x, bool isG4) {

  /*

Solve for Nsig:

1 - 0.95 = log likelihood ratio of S+B / B

0.05 = exp(-s) * sum( (s+b)^k / k!, k=0, k=b) / sum( b^k / k!, k=0, k=b)

  */

}

Accept[EffAll.size()][massList.size()];
for(i, j, k) {
  if(massList(k, i) == EffAll(k, j, 1)) {
    Accept(k, i) = EffAll(k, j);
  }

 }

AcceptCount = 
    




const int nMasses = 45;
double massList[nMasses];
for(int i = 0; i < nMasses; i++) massList[i] = 0.1 * TMath::Power(10., TMath::Sqrt(0.2 * x));

// import efficiencies -- output of ComputeAcceptance.cc? EffXYZ

// import geometric acceptances? GeoAcc

// import xsec data, acceptances from...? DataXYZ

if(mXYZ > 2 * mmcp) DataXYZ *= phaseSpaceRatio();
else DataJPsi = 0.01;

DataAll90Deg36m2area = all of DataXYZ;

