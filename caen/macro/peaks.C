void peaks() {

  TFile * fSource = new TFile("../Run_nai_cs137_6inches_ChargeHistos_3_30_2016.root");
  TH1D * h_source = (TH1D*)fSource->Get("Channel_11");

  h_source->Rebin(4);

  Int_t npeaks = 10;
  TSpectrum * s = new TSpectrum(npeaks);

  Int_t nfound = s->Search(h_source, 3.5, "", 0.0005);

  cout << endl << "nfound = " << nfound << endl << endl;

  Double_t * xpeaks = s->GetPositionX();

  for(int i = 0; i < nfound; i++) {
    cout << "Peak " << i << " -- " << xpeaks[i] << endl;
  }

  h_source->Draw("hist same");

}
