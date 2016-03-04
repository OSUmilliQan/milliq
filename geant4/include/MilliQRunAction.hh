#ifndef MilliQRunAction_h
#define MilliQRunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"
#include "G4Types.hh"

class MilliQRecorderBase;
class MilliQDetectorConstruction;
class MilliQPrimaryGeneratorAction;
class G4Run;
class MilliQRunActionMessenger;

class MilliQRunAction : public G4UserRunAction {
public:

  MilliQRunAction(MilliQRecorderBase*);
  virtual ~MilliQRunAction();

  virtual void BeginOfRunAction(const G4Run*);
  virtual void EndOfRunAction(const G4Run*);

  inline void SetVerbose(G4int verbose) { fVerboseLevel = verbose;}
  inline G4int GetVerbose()             { return fVerboseLevel;}

private:

  MilliQRecorderBase*           fRecorder;
  MilliQDetectorConstruction*   fDetector;
  MilliQPrimaryGeneratorAction* fKinematic;
  MilliQRunActionMessenger*     fMilliQRunActionMessenger;

  G4int fVerboseLevel;

  enum MilliQNtupleIds { kScintHits, kAll, kDEDX, kRadius, kPMT0Times, kPMT1Times, kPMT2Times, kNumNtupleIds };
  
};


#endif
