#ifndef MilliQDataFormat_h
#define MilliQDataFormat_h 1

#include "g4root.hh"
#include "G4Types.hh"

class MilliQDataFormat {
 public:

  MilliQDataFormat();
  virtual ~MilliQDataFormat();

  virtual void CreateDataMembers() {
    CreateAnalysisManager();
    CreateHistograms();
    CreateNtuples();
  }

  enum NtupleIds { kScintHits, kAll, kDEDX, kRadius, kPMT0Times, kPMT1Times, kPMT2Times, kNumNtupleIds };

  inline void SetVerboseLevel(G4int v) { fVerboseLevel = v; }
  inline G4int GetVerboseLevel()       { return fVerboseLevel; }

 private:

  virtual void CreateAnalysisManager();
  virtual void CreateHistograms();
  virtual void CreateNtuples();

  G4AnalysisManager* analysisManager;
  
  G4int fVerboseLevel;
};

#endif
