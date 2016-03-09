#include "MilliQActionInitialization.hh"

#include "MilliQPrimaryGeneratorAction.hh"

#include "MilliQRunAction.hh"
#include "MilliQEventAction.hh"
#include "MilliQTrackingAction.hh"
#include "MilliQSteppingAction.hh"
#include "MilliQStackingAction.hh"
#include "MilliQSteppingVerbose.hh"

#include "MilliQRecorderBase.hh"

#include "G4types.hh"

void MilliQActionInitialization::BuildForMaster() const {
  SetUserAction(new MilliQRunAction(fRecorder, fPTree));
}

void MilliQActionInitialization::Build() const {

  SetUserAction(new MilliQPrimaryGeneratorAction());

  SetUserAction(new MilliQStackingAction());

  SetUserAction(new MilliQRunAction(fRecorder, fPTree));
  SetUserAction(new MilliQEventAction(fRecorder, fPTree));
  SetUserAction(new MilliQTrackingAction(fRecorder));
  SetUserAction(new MilliQSteppingAction(fRecorder, fPTree));
}

G4VSteppingVerbose* MilliQActionInitialization::InitializeSteppingVerbose() const {
  return new MilliQSteppingVerbose();
}
