void compare() {

  TFile * fSource = new TFile("../Run_nai_cs137_6inches_ChargeHistos_3_30_2016.root");
  TFile * fNoSource = new TFile("../Run_nai_nosource_ChargeHistos_3_30_2016.root");

  TH1D * h_source = (TH1D*)fSource->Get("Channel_11");
  TH1D * h_nosource = (TH1D*)fNoSource->Get("Channel_11");

  h_source->SetLineColor(kRed);

  h_source->Rebin(2);
  h_nosource->Rebin(2);

  int lo = h_source->FindBin(-1.);
  int hi = h_source->FindBin(10.) + 1;

  h_source->Scale(1./h_source->Integral(lo, hi));
  h_nosource->Scale(1./h_nosource->Integral(lo, hi));

  h_source->Draw("hist");
  h_nosource->Draw("hist same");

}
