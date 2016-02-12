#include "TH1.h"
#include "TMath.h"
#include "TF1.h"
#include "TFile.h"
#include "TTree.h"

#include <vector>

using namespace std;

const int npeaks = 2;

Double_t GausN(Double_t x, Double_t n, Double_t q1, Double_t sigma1) {

  return TMath::Gaus(x, n*q1, sigma1*TMath::Sqrt(n), kTRUE);

}

Double_t ExpN(Double_t x, Double_t alpha, Double_t q0, Double_t q1, Double_t n, Double_t sigma0, Double_t sigma1) {

  if(x < 0.) return 0.;

  Double_t sigmaN = TMath::Sqrt(sigma0*sigma0 + n*sigma1*sigma1);
  Double_t qN = q0 + n*q1;

  Double_t shift = qN + sigmaN*sigmaN*alpha;
  
  Double_t erf_a = TMath::Erf(fabs(q0 - shift) / sigmaN / TMath::Sqrt(2));
  Double_t erf_b = TMath::Erf(fabs(x - shift) / sigmaN / TMath::Sqrt(2));
  Double_t sgn = TMath::Sign(1., x - shift);

  Double_t expon = TMath::Exp(-1. * alpha * (x - qN - sigmaN*sigmaN*alpha/2.)) * alpha / 2.;

  return expon*(erf_a + sgn * erf_b);
}

Double_t response(Double_t * x, Double_t * par) {

  // total normalization
  double norm = par[0];

  // background
  // (1-w)*Gaus(x; Q_0, sigma_0) + w* alpha*exp(-alpha*x)
  double pedestal = par[1]; // Q_0
  double pedestal_sigma = par[2]; // sigma_0
  double w = par[3];
  double alpha = par[4];
  
  // signal
  // Poisson(n; mu) \otimes Gaus(n; n*q, sigma*sqrt(n))
  double mu = par[5];
  double q = par[6]; // Q_1
  double sigma = par[7]; // sigma_1

  Double_t value = 0.;

  for(int i = 0; i <= npeaks; i++) {
    double pois = TMath::Poisson(i, mu);

    double gaus = (i == 0) ? GausN(x[0] - pedestal, 1, 0., pedestal_sigma) :
			     GausN(x[0] - pedestal, i, q, sigma);

    double bkgTerm = ExpN(x[0], alpha, pedestal, q, i, pedestal_sigma, sigma);

    value += pois * ((1-w)*gaus + w*bkgTerm);
  }

  return norm*value;
}

// Simpler TF1's for plotting deconvolutions

Double_t TF1_bkg(Double_t * x, Double_t * par) {

  Double_t norm = par[0];
  Double_t w = par[1];
  Double_t pedestal = par[2];
  Double_t sigma = par[3];
  Double_t alpha = par[4];
  Double_t mu = par[5];

  Double_t type1 = TMath::Gaus(x[0] - pedestal, 0., sigma, kTRUE);
  Double_t type2 = (x[0] >= pedestal) ? alpha * TMath::Exp(-1. * alpha * (x[0] - pedestal)) : 0.;

  Double_t pdf = (1-w)*type1 + w*type2;

  Double_t pois = TMath::Poisson(0.0, mu);

  return norm * pois * pdf;
}

Double_t TF1_bkg1(Double_t * x, Double_t * par) {

  Double_t norm = par[0];
  Double_t w = par[1];
  Double_t pedestal = par[2];
  Double_t sigma = par[3];
  Double_t alpha = par[4];
  Double_t mu = par[5];

  Double_t type1 = TMath::Gaus(x[0] - pedestal, 0., sigma, kTRUE);

  Double_t pois = TMath::Poisson(0.0, mu);

  return (1-w) * type1 * pois * norm;
}

Double_t TF1_bkg2(Double_t * x, Double_t * par) {

  Double_t norm = par[0];
  Double_t w = par[1];
  Double_t pedestal = par[2];
  Double_t sigma = par[3];
  Double_t alpha = par[4];
  Double_t mu = par[5];

  Double_t type2 = (x[0] >= pedestal) ? alpha * TMath::Exp(-1. * alpha * (x[0] - pedestal)) : 0.;

  Double_t pois = TMath::Poisson(0.0, mu);

  return w * type2 * pois * norm;
}

Double_t TF1_signal(Double_t * x, Double_t * par) {

  Double_t pois = TMath::Poisson(par[0], par[1]);

  return GausN(x[0] - par[5], par[0], par[3], par[4]) * pois * par[2] * par[6];
}


