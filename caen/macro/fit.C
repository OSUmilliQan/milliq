#ifndef FIT_C
#define FIT_C

#include "fit.h"

void doFit(TString file, TString noiseFile, bool useNoise, int npks, bool logy) {

  const int npeaks = npks;

  gStyle->SetOptFit(1);

  TFile * input = new TFile(file, "READ");
  TH1D * h_spectrum = (TH1D*)input->Get("Channel_11");
  
  if(useNoise) {
    TFile * noiseInput = new TFile(noiseFile, "READ");
    TH1D * h_noise = (TH1D*)noiseInput->Get("Channel_11");

    int binlo = h_noise->FindBin(-5.);
    int binhi = h_noise->FindBin(10.) + 1;

    h_noise->Scale(h_spectrum->Integral(binlo, binhi) / h_noise->Integral(binlo, binhi));

    h_spectrum->Add(h_noise, -1.);

    for(int i = 0; i < h_spectrum->GetNbinsX(); i++) {
      if(h_spectrum->GetBinContent(i+1) < 0.) {
	h_spectrum->SetBinContent(i+1, 0.);
	h_spectrum->SetBinError(i+1, 0.);
      }
    }

    noiseInput->Close();
  }

  h_spectrum->Rebin(4);

  const int nbins = h_spectrum->GetNbinsX();
  const double xlo = h_spectrum->GetBinLowEdge(1);
  //const double xlo = 10.0;
  const double xhi = h_spectrum->GetBinLowEdge(nbins+1);

  // RooFit stuff

  RooRealVar x("x", "x", xlo, xhi);

  vector<RooRealVar*> means, sigmas;
  vector<RooGaussian*> signals;
  vector<RooRealVar*> sigFractions;
  RooArgList pdfs;
  RooArgList coeffs;

  double defaultPositions[4] = {0., 25, 180, 380};

  // create npeaks: RooGaussian(name, title, x, mean, sigma)
  for(int i = 0; i < npeaks; i++) {
    TString q_name = Form("q%d", i);
    RooRealVar * var = new RooRealVar(q_name, q_name, (i < 4) ? defaultPositions[i] : 50*i, -10., 450.);
    means.push_back(var);

    TString s_name = Form("sigma%d", i);
    RooRealVar * var2 = new RooRealVar(s_name, s_name, 5, 0, 25);
    sigmas.push_back(var2);

    TString peak_name = Form("Peak%d", i);
    RooGaussian * peak = new RooGaussian(peak_name, peak_name, x, *var, *var2);
    signals.push_back(peak);

    TString frac_name = Form("frac%d", i);
    RooRealVar * frac = new RooRealVar(frac_name, frac_name, 0.1, 0, 1);
    sigFractions.push_back(frac);
  }

  // add npeaks, normalizations into ArgLists for RooAddPdf
  for(int i = 0; i < npeaks; i++) {
    pdfs.add(*signals[i]);
    coeffs.add(*sigFractions[i]);
  }

  // RooVoigtian gamma peak
  // Breit-Wigner convoluted with a gaussian
  RooRealVar voigt_mean("voigt_mean", "voigt_mean", 560., 500., 600.);
  RooRealVar voigt_width("voigt_width", "voigt_width", 5., 0., 50.);
  RooRealVar voigt_sigma("voigt_sigma", "voigt_sigma", 5., 0., 50.);
  
  RooVoigtian gammaPeak("gammaPeak", "gammaPeak", x, voigt_mean, voigt_width, voigt_sigma);
  RooRealVar * gammaPeakFraction = new RooRealVar("gammaPeakFraction", "gammaPeakFraction", 0.1, 0, 1);
  pdfs.add(gammaPeak);
  coeffs.add(*gammaPeakFraction);

  // RooCMSShape background
  // erfc(beta*(alpha-x)) * exp(-gamma*(x-peak))
  RooRealVar cms_alpha("cms_alpha", "cms_alpha", 30., -100, 100);
  RooRealVar cms_beta("cms_beta", "cms_beta", 0.05, -5, 5);
  RooRealVar cms_gamma("cms_gamma", "cms_gamma", 0.05, -5, 5);
  RooRealVar cms_peak("cms_peak", "cms_peak", 0);

  // peak is effectively just a normalization? odd
  cms_peak.setConstant(kTRUE);

  RooCMSShape rBkg("cmsShapeBkg", "cmsShapeBkg", x, cms_alpha, cms_beta, cms_gamma, cms_peak);
  pdfs.add(rBkg);

  // total fit shape
  RooAddPdf response("response", "response", pdfs, coeffs);

  // fit h_spectrum
  RooDataHist dataHist("data", "data", x, h_spectrum);
  response.fitTo(dataHist);

  /////////////////////////////////
  // RooFit plot
  /////////////////////////////////

  RooPlot * frame = x.frame(Title("PMT spectrum"));

  // draw parameter box
  //response.paramOn(frame, Format("NE", AutoPrecision(2)), Layout(0.6, 0.98, 0.9));

  // draw data hist
  dataHist.plotOn(frame, LineColor(kBlack));

  // draw total fit and components
  response.plotOn(frame, Components(rBkg), LineColor(kRed));
  for(int i = 0; i < npeaks; i++) response.plotOn(frame, Components(*signals[i]), LineColor(kBlack));
  response.plotOn(frame, LineColor(kBlue));

  // make a pdf
  TCanvas * canv = new TCanvas("canv", "canv", 10, 10, 2000, 1400);
  if(logy) canv->SetLogy(true);

  frame->Draw();

  canv->SaveAs("result.pdf");

  cout << endl << endl;
  cout << "chi2 = " << frame->chiSquare();
  cout << endl << endl;

  double peakPosition = voigt_mean.getVal();
  int peakBin = h_spectrum->FindBin(peakPosition);
  double peakContent = h_spectrum->GetBinContent(peakBin);

  int fwhm_binlo = 1;
  for(int i = peakBin; i > 0; i--) {
    if(h_spectrum->GetBinContent(i) < 0.5 * peakContent) {
      fwhm_binlo = i + 1;
      break;
    }
  }

  int fwhm_binhi = h_spectrum->GetNbinsX();
  for(int i = peakBin; i < h_spectrum->GetNbinsX(); i++) {
    if(h_spectrum->GetBinContent(i) > 0.5 * peakContent) {
      fwhm_binhi = i - 1;
      break;
    }
  }

  double fwhm_lo = h_spectrum->GetBinCenter(fwhm_binlo) - peakPosition;
  double fwhm_hi = peakPosition - h_spectrum->GetBinCenter(fwhm_binhi);

  cout << "FWHM: -" << fwhm_lo << ", +" << fwhm_hi << endl << endl;

  // now output stuff
  TFile * output = new TFile("output.root", "RECREATE");

  double nEvents = h_spectrum->Integral();

  // create container for all the relevant info
  vector<PeakInfo> infos;
  for(int i = 0; i < npeaks; i++) {
    PeakInfo pInfo;

    pInfo.mean = means[i]->getVal();
    pInfo.meanError = means[i]->getError();

    pInfo.sigma = sigmas[i]->getVal();
    pInfo.sigmaError = sigmas[i]->getError();

    pInfo.norm = nEvents * sigFractions[i]->getVal();
    pInfo.normError = nEvents * sigFractions[i]->getError(); // ignoring sqrt(nEvents) error

    infos.push_back(pInfo);
  }

  // sort peaks by their mean
  sort(infos.begin(), infos.end(), comparePeaks);

  TH1D * h_means = new TH1D("h_means", "Peak number vs fit mean;Peak number;Fit mean", npeaks, 0, npeaks);
  for(int i = 0; i < npeaks; i++) {
    h_means->SetBinContent(i+1, infos[i].mean);
    h_means->SetBinError(i+1, infos[i].meanError);
  }

  TH1D * h_norms = new TH1D("h_norms", "Peak number vs fit norm;Peak number;Fit norm", npeaks, 0, npeaks);
  for(int i = 0; i < npeaks; i++) {
    h_norms->SetBinContent(i+1, infos[i].norm);
    h_norms->SetBinError(i+1, infos[i].normError);
  }

  TH1D * h_sigmas = new TH1D("h_resolutions", "Peak number vs fit sigma;Peak number;Fit sigma", npeaks, 0, npeaks);
  for(int i = 0; i < npeaks; i++) {
    h_sigmas->SetBinContent(i+1, infos[i].sigma);
    h_sigmas->SetBinError(i+1, infos[i].sigmaError);
  }

  output->Write();
  output->Close();

  input->Close();

}

#endif
