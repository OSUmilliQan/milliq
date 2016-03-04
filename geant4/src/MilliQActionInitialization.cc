
#include "MilliQActionInitialization.hh"

#include "MilliQPrimaryGeneratorAction.hh"

#include "MilliQRunAction.hh"
#include "MilliQEventAction.hh"
#include "MilliQTrackingAction.hh"
#include "MilliQSteppingAction.hh"
#include "MilliQStackingAction.hh"
#include "MilliQSteppingVerbose.hh"

#include "MilliQRecorderBase.hh"


void MilliQActionInitialization::BuildForMaster() const
{
  SetUserAction(new MilliQRunAction(fRecorder));
}


void MilliQActionInitialization::Build() const
{
  SetUserAction(new MilliQPrimaryGeneratorAction());

  SetUserAction(new MilliQStackingAction());

  SetUserAction(new MilliQRunAction(fRecorder));
  SetUserAction(new MilliQEventAction(fRecorder));
  SetUserAction(new MilliQTrackingAction(fRecorder));
  SetUserAction(new MilliQSteppingAction(fRecorder));
}


G4VSteppingVerbose* MilliQActionInitialization::InitializeSteppingVerbose() const
{
  return new MilliQSteppingVerbose();
}
