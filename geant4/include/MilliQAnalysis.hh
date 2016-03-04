
#ifndef MilliQAnalysis_H
#define MilliQAnalysis_H 1

#include <vector>

class MilliQAnalysis{
public:
  MilliQAnalysis(std::vector< std::vector<G4double> > pmtTime,
		 std::vector< std::vector<G4double> > scintTime,
		 std::vector< std::vector<G4double> > scintEn,
		 G4int pNblock,
		 G4int pNstack);

  void NearestN();
  void ComputeTandE();
  bool IsActive(){return fIsActive;}
  std::vector<G4double> GetPMTTimes(){return pmtTimes;}
  std::vector<G4double> GetTimeOfFlight(){return timeOfFlight;}
  std::vector<G4double> GetTotalEdep(){return totalEdep;}
  std::vector<G4int> GetActiveEv(){return activeEvent;}


private:

  std::vector< std::vector<G4double> > fpmtTime;
  std::vector< std::vector<G4double> > fscintTime;
  std::vector< std::vector<G4double> > fscintEn;
  std::vector<G4int> activeEvent;
  std::vector<G4double> pmtTimes;
  std::vector<G4double> timeOfFlight;
  std::vector<G4double> totalEdep;
  G4int fNblock;
  G4int fNstack;
  bool fIsActive;
};

#endif
