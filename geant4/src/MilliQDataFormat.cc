#include "MilliQDataFormat.hh"

#include "G4ios.hh"

MilliQDataFormat::MilliQDataFormat() {

  fVerboseLevel = 0;

}

MilliQDataFormat::~MilliQDataFormat() {

}

void MilliQDataFormat::CreateAnalysisManager() {

  analysisManager = G4AnalysisManager::Instance();

  // Choice of analysis technology is done by including g4root.hh
  G4cout << "Using G4AnalysisManager type: " << analysisManager->GetType() << G4endl;

  // Default settings
  analysisManager->SetVerboseLevel(fVerboseLevel);
  analysisManager->SetFileName("MilliQ");

}

void MilliQDataFormat::CreateHistograms() {

  // Creating 1D histograms
  analysisManager->CreateH1("PMT", "PMT # Hits", 50, 0., 50); // h1 Id = 0

  // Creating 2D histograms
  // analysisManager->CreateH2("Chamber1 XY","Drift Chamber 1 X vs Y",           // h2 Id = 0
  //

}

void MilliQDataFormat::CreateNtuples() {

  // kRawData
  //durp


  // kScintHits
  analysisManager->CreateNtuple("MilliQEn", "ScintHits");
  analysisManager->CreateNtupleDColumn("sEnDep0"); // column Id = 0
  analysisManager->CreateNtupleDColumn("sTime0"); // column Id = 1
  analysisManager->FinishNtuple();

  // kAll
  analysisManager->CreateNtuple("MilliQAll", "All");
  analysisManager->CreateNtupleIColumn("activePMT0");  // column Id = 0
  analysisManager->CreateNtupleIColumn("activePMT1");  // column Id = 1
  analysisManager->CreateNtupleIColumn("activePMT2");  // column Id = 2
  analysisManager->CreateNtupleDColumn("pmtHitTime0");  // column Id = 3
  analysisManager->CreateNtupleDColumn("pmtHitTime1");  // column Id = 4
  analysisManager->CreateNtupleDColumn("pmtHitTime2");  // column Id = 5
  analysisManager->CreateNtupleDColumn("TOFScint0");  // column Id = 6
  analysisManager->CreateNtupleDColumn("TOFScint1");  // column Id = 7
  analysisManager->CreateNtupleDColumn("TOFScint2");  // column Id = 8
  analysisManager->CreateNtupleDColumn("TotEnDep0"); // column Id = 9
  analysisManager->CreateNtupleDColumn("TotEnDep1"); // column Id = 10
  analysisManager->CreateNtupleDColumn("TotEnDep2"); // column Id = 11
  analysisManager->CreateNtupleIColumn("NScintPho"); // column Id = 12
  analysisManager->FinishNtuple();

  // kDEDX
  analysisManager->CreateNtuple("MilliQDedx", "DEDX");
  analysisManager->CreateNtupleDColumn("EKinMeV"); // column Id = 0
  analysisManager->CreateNtupleDColumn("MeVpermm"); // column Id = 1
  analysisManager->FinishNtuple();

  // kRadius
  analysisManager->CreateNtuple("MilliQRadius","Radius");
  analysisManager->CreateNtupleDColumn("CoordinateX");//column Id = 0
  analysisManager->CreateNtupleDColumn("CoordinateY");//column Id = 1
  analysisManager->CreateNtupleDColumn("CoordinateZ");//column Id = 2
  analysisManager->FinishNtuple();

  // kPMT0Times
  analysisManager->CreateNtuple("MilliQPMT0All","PMT0Times");
  analysisManager->CreateNtupleDColumn("pmtAllTimes0"); // column Id = 0
  analysisManager->FinishNtuple();

  // kPMT1Times
  analysisManager->CreateNtuple("MilliQPMT1All","PMT1Times");
  analysisManager->CreateNtupleDColumn("pmtAllTimes1"); // column Id = 0
  analysisManager->FinishNtuple();

  // kPMT2Times
  analysisManager->CreateNtuple("MilliQPMT2All","PMT2Times");
  analysisManager->CreateNtupleDColumn("pmtAllTimes2"); // column Id = 0
  analysisManager->FinishNtuple();

}
