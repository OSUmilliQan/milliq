#include "fit.h"

Int_t nbins = 200;
Double_t xlo = 15.0;
Double_t xhi = 215.0;

void fit() {

  gStyle->SetOptFit(1);

  TString folder = "emorpho_data/";

  TString file = "Jan19_led.root";

  TFile * input = new TFile(folder + file, "READ");

  TTree * ntuple = (TTree*)input->Get("ntuple");
  Float_t en;
  ntuple->SetBranchAddress("energy", &en);

  TH1D * h = new TH1D("h_fit", "h_fit", nbins, xlo, xhi);
  h->Sumw2();

  for(int i = 0; i < ntuple->GetEntries(); i++) {
    ntuple->GetEntry(i);
    h->Fill(en);
  }

  TF1 * fspectrum = new TF1("spectrum", response, xlo, xhi, 8);

  TString parNames[8] = {"norm",
			 "q0", "sigma0", "w", "alpha",
			 "mu", "q1", "sigma1"};

  Double_t parInits[8] = {h->Integral(),
			  20.0, 1.0, 0.5, 1.,
			  0.5, 20.0, 5.0};
  
  for(int i = 0; i < 8; i++) {
    fspectrum->SetParameter(i, parInits[i]);
    fspectrum->SetParName(i, parNames[i]);
  }

  fspectrum->SetParLimits(0, 0., 2. * h->Integral()); // norm
  fspectrum->SetParLimits(1, xlo, xhi); // q0 pedestal
  fspectrum->SetParLimits(2, 0., xhi - xlo); // sigma0 pedestal
  fspectrum->SetParLimits(3, 0., 1.); // w
  fspectrum->SetParLimits(4, 0., 10.); // alpha

  fspectrum->SetParLimits(6, 0., 100.); // q1
  fspectrum->SetParLimits(7, 0., 100.); // sigma1

  h->Fit(fspectrum);

  cout << endl << endl << "Fit results:" << endl;
  cout << "Chi2 / NDOF = " << fspectrum->GetChisquare() / fspectrum->GetNDF() << endl << endl;

  TCanvas * canv = new TCanvas("canv", "canv", 10, 10, 1000, 1000);
  canv->SetLogy(true);

  // durp
  //h->GetXaxis()->SetRangeUser(0, 50);

  h->Draw("hist");
  
  
  fspectrum->SetNpx(10000);
  fspectrum->SetLineColor(kRed);
  fspectrum->Draw("same");

  TF1 * func_bkg = new TF1("func_bkg", TF1_bkg, xlo, xhi, 6);
  func_bkg->SetParameters(fspectrum->GetParameter(0), // norm
			   fspectrum->GetParameter(3), // w
			   fspectrum->GetParameter(1), // pedestal
			   fspectrum->GetParameter(2), // sigma
			   fspectrum->GetParameter(4), // alpha
			   fspectrum->GetParameter(5)); // mu

  func_bkg->SetLineColor(kBlue);
  func_bkg->SetLineStyle(2);
  func_bkg->SetNpx(5000);
  func_bkg->Draw("same");

/*
  TF1 * func_bkg1 = new TF1("func_bkg1", TF1_bkg1, xlo, xhi, 6);
  func_bkg1->SetParameters(fspectrum->GetParameter(0), // norm
			   fspectrum->GetParameter(3), // w
			   fspectrum->GetParameter(1), // pedestal
			   fspectrum->GetParameter(2), // sigma
			   fspectrum->GetParameter(4), // alpha
			   fspectrum->GetParameter(5)); // mu

  func_bkg1->SetLineColor(kBlue);
  func_bkg1->SetLineStyle(2);
  func_bkg1->SetNpx(5000);
  func_bkg1->Draw("same");

  TF1 * func_bkg2 = new TF1("func_bkg2", TF1_bkg2, xlo, xhi, 6);
  func_bkg2->SetParameters(fspectrum->GetParameter(0), // norm
			   fspectrum->GetParameter(3), // w
			   fspectrum->GetParameter(1), // pedestal
			   fspectrum->GetParameter(2), // sigma
			   fspectrum->GetParameter(4), // alpha
			   fspectrum->GetParameter(5)); // mu

  func_bkg2->SetLineColor(kBlue);
  func_bkg2->SetLineStyle(2);
  func_bkg2->SetNpx(5000);
  func_bkg2->Draw("same");
*/

  vector<TF1*> func_peaks;
  for(int i = 1; i <= npeaks; i++) {

    TString peak_name = "func_peak" + (TString)Form("%i", i);

    TF1 * this_peak = new TF1(peak_name, TF1_signal, xlo, xhi, 7);
    this_peak->SetParameters(i,                        // n = i
			     fspectrum->GetParameter(5), // mu
			     fspectrum->GetParameter(0), // norm
			     fspectrum->GetParameter(6), // q1
			     fspectrum->GetParameter(7),
			     fspectrum->GetParameter(1), // pedestal
			     fspectrum->GetParameter(3)); // w
    
    this_peak->SetLineColor(kBlack);
    this_peak->SetLineStyle(2);
    this_peak->SetNpx(5000);
    this_peak->Draw("same");

    func_peaks.push_back(this_peak);
  }

  canv->SaveAs("result.pdf");
  
}



