#include "globals.hh"
#include "MilliQAnalysis.hh"
#include "G4SystemOfUnits.hh"
#include <math.h>
#include <algorithm>

MilliQAnalysis::MilliQAnalysis(std::vector< std::vector<G4double> > ppmtTime, std::vector< std::vector<G4double> > pscintTime,std::vector< std::vector<G4double> > pscintEn, G4int pNblock, G4int pNstack) : fNblock(pNblock), fNstack(pNstack), fIsActive(false), fVerbose(false) {

  fpmtTime = ppmtTime;
  fscintTime = pscintTime;
  fscintEn = pscintEn;
  NearestN();

}

// NearestN() stores an event if there are two hits in separate layers,
// anywhere in the detector (not necessarily in the same block/module),
// and within 15 ns of each other

void MilliQAnalysis::NearestN() {

  //Figures out for which sequence all 3 pmt and Scint light up
  if(fVerbose) G4cout << "//////// " << G4endl;

  std::vector<G4int> activePMT;
  std::vector< std::vector<G4double> > activePMTTimes (fNstack);

  //The following identifies events in which 3 consecutive layers light up

  bool recordEvent = true;

  // For all stack layers
  for(G4int j = 0; j < fNstack; j++) {

    // For all blocks in that stack
    bool layerHit = false;
    for(G4int i = 0; i < fNblock; i++) {
      if(fpmtTime[i + fNblock*j].size() > 0) {
	       layerHit = true;

	       // store PMT numbers with hits
	       activePMT.push_back(i + fNblock*j);

	       // store the times of this cell's hits in the list of hit times in the stack layer
	       for(unsigned int k = 0; k < fpmtTime[i + fNblock*j].size(); k++) activePMTTimes[j].push_back(fpmtTime[i + fNblock*j][k]);
      }
    }

    // if any layer has no hits anywhere, don't record this event
    if(!layerHit) {
      recordEvent = false;
      //break;
    }
    else if(fVerbose) G4cout << " Passed Layer " << j << G4endl;
  }

  if(fVerbose) G4cout << "record Event " << recordEvent << G4endl;

  // If all three layers have hits (anywhere)
  if(recordEvent) {

    // loop through the last 2 stacks/layers
    for(G4int j = 1; j < fNstack; j++) {

      for(G4int k = 0; k < j; k++) {

	       bool tFlag = false;

	      // if any two hits, in different layers, are within 15ns of each other, record the event
      	for(unsigned int a = 0; a < activePMTTimes[k].size(); a++) {
	         for(unsigned int b = 0; b < activePMTTimes[j].size(); b++) {

	            if(fabs(activePMTTimes[k][a]/ns - activePMTTimes[j][b]/ns) < 15/ns) {
	               tFlag = true;
	               break;
	            }
	         }

	         if(tFlag) break;
	      }

	      if(!tFlag) {
	         recordEvent = false;
	         break;
	      }

      }

      if(!recordEvent) break;
    }
  }

  // The next part sets all puts zeroes three times per event for pmtTimes, timeOfFlight, and activeEvent???

  if(recordEvent) {
    if(fVerbose) G4cout << "Recorded Event" << G4endl;
    fIsActive = true;
    //		ComputeTandESummed();
    G4double ensum = 0;
    for(G4int i = 0; i < fNstack; i++) {
      pmtTimes.push_back(ensum);
      timeOfFlight.push_back(ensum);
      activeEvent.push_back(0);
    }
    totalEdep.push_back(ensum);
  }

  //The following identifies events in which 3 consecutive PMTs light up
  /*
    for(G4int i=0; i < fNblock; i++){
      if(fpmtTime[i].size()>0){
        activePMT.push_back(i);
      }
    }

    for(unsigned int i=0; i < activePMT.size(); i++) {
      bool recordEvent = true;
      for(G4int j=1; j < fNstack; j++){
        if(fpmtTime[activePMT[i]+fNblock*j].size()==0){
          recordEvent = false;
          break;
        }
    }
    if(recordEvent == true){

      for(G4int j=1; j < fNstack; j++){
        for(G4int k = 0; k < j; k++){

          bool tFlag = false;

          for(unsigned int a = 0; a < fpmtTime[activePMT[i]+fNblock*k].size(); a++){
            for(unsigned int b = 0; b < fpmtTime[activePMT[i]+fNblock*j].size(); b++){

              if(fabs(fpmtTime[activePMT[i]+fNblock*k][a]/ns-fpmtTime[activePMT[i]+fNblock*j][b]/ns)<15/ns){
                tFlag = true;
                break;
              }
            }

            if(tFlag == true) break;
         }
         if(tFlag == false){
          recordEvent = false;
          break;
         }

       }

    if(recordEvent == false) break;

    }
  }

    if(recordEvent == true) activeEvent.push_back( activePMT[i] );

  }
    //If passes this, means that the same 3 scint and pmt light up!
    if(activeEvent.size()==1){
      G4cout<<"Recorded Event"<<G4endl;
      for(G4int j = 1; j < fNstack; j++)
        activeEvent.push_back(activeEvent[0]+j*fNblock);
        fIsActive = true;
        ComputeTandE();
    }
  */
}

void MilliQAnalysis::ComputeTandE(){

  //Computes the median time for photons to hit PMT

  G4int ind;
  G4int nmedian;

  for(unsigned int i = 0; i < activeEvent.size(); i++) {
    ind = activeEvent[i];
    G4double ensum=0;
    nmedian = (fpmtTime[ind].size() - 1)/2;
    pmtTimes.push_back(fpmtTime[ind][nmedian]);

    timeOfFlight.push_back(fpmtTime[ind][nmedian] - fscintTime[ind][0]);

    for(unsigned int k = 0; k < fscintEn[ind].size();k++) {
      ensum += fscintEn[ind][k];
    }
    totalEdep.push_back(ensum);

  }

}
