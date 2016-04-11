void scale() {

  gStyle->SetOptStat(0);

  TFile * fSource = new TFile("../Run_nai_cs137_6inches_ChargeHistos_3_30_2016.root");
  TFile * fNoSource = new TFile("../Run_nai_nosource_ChargeHistos_3_30_2016.root");

  TH1D * h_source = (TH1D*)fSource->Get("Channel_11");
  TH1D * h_nosource = (TH1D*)fNoSource->Get("Channel_11");

  // scale in [KeV / pC]
  double calib = 661.7 / 568.427;

  cout << endl << "Scale = " << calib << " [KeV / pC]" << endl << endl;

  // durp
  calib = 1.0;

  TH1D * h_s = new TH1D("h_s", "h_s", h_source->GetNbinsX(), h_source->GetBinLowEdge(1) * calib, h_source->GetBinLowEdge(h_source->GetNbinsX() + 1) * calib);
  TH1D * h_n = new TH1D("h_n", "h_n", h_nosource->GetNbinsX(), h_nosource->GetBinLowEdge(1) * calib, h_nosource->GetBinLowEdge(h_nosource->GetNbinsX() + 1) * calib);

  for(int i = 0; i <= h_s->GetNbinsX() + 1; i++) {
    h_s->SetBinContent(i, h_source->GetBinContent(i));
    h_s->SetBinError(i, h_source->GetBinError(i));
  }

  for(int i = 0; i <= h_n->GetNbinsX() + 1; i++) {
    h_n->SetBinContent(i, h_nosource->GetBinContent(i));
    h_n->SetBinError(i, h_nosource->GetBinError(i));
  }

  h_s->Rebin(2);
  h_n->Rebin(2);

  int lo = h_s->FindBin(-1.);
  int hi = h_s->FindBin(10.) + 1;

  h_s->Scale(1./h_s->Integral(lo, hi));
  h_n->Scale(1./h_n->Integral(lo, hi));

  h_s->SetLineColor(kRed);
  h_n->SetLineColor(kBlack);

  h_s->SetLineWidth(2);
  h_n->SetLineWidth(2);

  h_s->GetXaxis()->SetTitle("Energy [KeV]");
  h_s->SetTitle("NaI(Tl) Scintillator");

  h_s->Draw("hist");
  h_n->Draw("hist same");


}
