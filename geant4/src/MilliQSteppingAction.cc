//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: MilliQSteppingAction.cc 73915 2013-09-17 07:32:26Z gcosmo $
//
/// \file optical/MilliQ/src/MilliQSteppingAction.cc
/// \brief Implementation of the MilliQSteppingAction class
//
//
#include "MilliQSteppingAction.hh"
#include "MilliQEventAction.hh"
#include "MilliQTrackingAction.hh"
#include "MilliQTrajectory.hh"
#include "MilliQPMTSD.hh"
#include "MilliQDetectorConstruction.hh"
#include "MilliQUserTrackInformation.hh"
#include "MilliQUserEventInformation.hh"
#include "MilliQSteppingMessenger.hh"
#include "MilliQRecorderBase.hh"
#include "MilliQRunAction.hh"

#include "G4SteppingManager.hh"
#include "G4SDManager.hh"
#include "G4EventManager.hh"
#include "G4ProcessManager.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4Event.hh"
#include "G4StepPoint.hh"
#include "G4TrackStatus.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "g4root.hh"
#include "G4ParticleTypes.hh"
#include "G4Electron.hh"

MilliQSteppingAction::MilliQSteppingAction(MilliQRecorderBase* r, const G4int geometryVersion) : fRecorder(r), fOneStepPrimaries(false), fAlternate(geometryVersion) {

  fSteppingMessenger = new MilliQSteppingMessenger(this);

  fExpectedNextStatus = Undefined;
}

MilliQSteppingAction::~MilliQSteppingAction() {}

void MilliQSteppingAction::UserSteppingAction(const G4Step * theStep) {

  G4Track* theTrack = theStep->GetTrack();

  if(theTrack->GetCurrentStepNumber() == 1) fExpectedNextStatus = Undefined;

  MilliQUserTrackInformation* trackInformation = (MilliQUserTrackInformation*)theTrack->GetUserInformation();
  MilliQUserEventInformation* eventInformation = (MilliQUserEventInformation*)G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetUserInformation();

  G4StepPoint* thePrePoint = theStep->GetPreStepPoint();
  G4VPhysicalVolume* thePrePV = thePrePoint->GetPhysicalVolume();

  G4StepPoint* thePostPoint = theStep->GetPostStepPoint();
  G4VPhysicalVolume* thePostPV = thePostPoint->GetPhysicalVolume();

  G4OpBoundaryProcessStatus boundaryStatus = Undefined;
  static G4ThreadLocal G4OpBoundaryProcess* boundary = NULL;

  bool IsRadius = (fAlternate == 1 && theTrack->GetParentID() == 0);

  if(IsRadius) {
    G4cout<<"It got to the alternate geometry"<<G4endl;
    G4AnalysisManager* analysisManager = G4AnalysisManager::Instance();
    G4ThreeVector coord = thePrePoint->GetPosition();
    for(G4int ii = 0; ii < 3; ii++) analysisManager->FillNtupleDColumn(MilliQDataFormat::kRadius, ii, coord[ii]);
    analysisManager->AddNtupleRow(MilliQDataFormat::kRadius);
  }

  //find the boundary process only once (only gets called once, not sure if this is good)
  //Seems to behave the same when I do if(true)***
  if(!boundary) {
    G4ProcessManager* pm = theStep->GetTrack()->GetDefinition()->GetProcessManager();
    G4int nprocesses = pm->GetProcessListLength();
    G4ProcessVector* pv = pm->GetProcessList();
    for(G4int i = 0; i < nprocesses; i++) {
      if((*pv)[i]->GetProcessName() == "OpBoundary") {
        boundary = (G4OpBoundaryProcess*)(*pv)[i];
        break;
      }
    }
  }

  if(false){
    G4ProcessManager* pm = theStep->GetTrack()->GetDefinition()->GetProcessManager();
    G4int nprocesses = pm->GetProcessListLength();
    G4ProcessVector* pv = pm->GetProcessList();
    for(G4int i = 0; i < nprocesses; i++) {
      if((*pv)[i]->GetProcessName() != "hIoni" &&
	 (*pv)[i]->GetProcessName() != "StepLimiter" &&
	 (*pv)[i]->GetProcessName()!="Transportation") {
	G4cout << "Stepping Action Process Name: " << (*pv)[i]->GetProcessName() << G4endl;
      }
    }

  }

  if(theTrack->GetParentID() == 0) {
    //This is a primary track
    //It gets here
    G4TrackVector* fSecondary = fpSteppingManager->GetfSecondary();
    G4int tN2ndariesTot = fpSteppingManager->GetfN2ndariesAtRestDoIt()
      + fpSteppingManager->GetfN2ndariesAlongStepDoIt()
      + fpSteppingManager->GetfN2ndariesPostStepDoIt();

    //If we havent already found the conversion position and there were
    //secondaries generated, then search for it
    if(!eventInformation->IsConvPosSet() && tN2ndariesTot > 0) {
      for(size_t lp1 = (*fSecondary).size() - tN2ndariesTot; lp1 < (*fSecondary).size(); lp1++) {
        const G4VProcess* creator = (*fSecondary)[lp1]->GetCreatorProcess();
        if(creator) {
          G4String creatorName = creator->GetProcessName();
          if(creatorName == "phot" || creatorName == "compt" || creatorName == "conv") {
            //since this is happening before the secondary is being tracked
            //the Vertex position has not been set yet(set in initial step)
            eventInformation->SetConvPos((*fSecondary)[lp1]->GetPosition());
          }
        }
      }
    }

    if(fOneStepPrimaries && thePrePV->GetName() == "Scintillator Physical Volume") {
      //never gets here cause fonestepprimaries is set to false
      //set true in messenger, kills particles after one step
      theTrack->SetTrackStatus(fStopAndKill);

    }
  }

  if(!thePostPV){//out of world (works well)
    fExpectedNextStatus = Undefined;
    return;
  }

  G4ParticleDefinition* particleType = theTrack->GetDefinition();
  if(particleType == G4OpticalPhoton::OpticalPhotonDefinition()) {
    //Optical photon only

    if(thePostPV->GetName() == "fWorldPV") {//"expHall")
      //Kill photons entering expHall from something other than Slab
      theTrack->SetTrackStatus(fStopAndKill);
      //Doesn't get here...?
    }

    //Was the photon absorbed by the absorption process
    if(thePostPoint->GetProcessDefinedStep()->GetProcessName() == "OpAbsorption") {
      eventInformation->IncAbsorption();
      trackInformation->AddTrackStatusFlag(absorbed);
    }

    boundaryStatus = boundary->GetStatus();

    //Check to see if the partcile was actually at a boundary
    //Otherwise the boundary status may not be valid
    //Prior to Geant4.6.0-p1 this would not have been enough to check
    if(thePostPoint->GetStepStatus() == fGeomBoundary) {
      if(fExpectedNextStatus == StepTooSmall) {
        if(boundaryStatus != StepTooSmall) {
          G4ExceptionDescription ed;
          ed << "MilliQSteppingAction::UserSteppingAction(): "
	     << "No reallocation step after reflection!"
	     << G4endl;
          G4Exception("MilliQSteppingAction::UserSteppingAction()", "MilliQExpl01",
		      FatalException,ed,
		      "Something is wrong with the surface normal or geometry");
        }
      }
      fExpectedNextStatus = Undefined;
      switch(boundaryStatus) {
      case Absorption:
        trackInformation->AddTrackStatusFlag(boundaryAbsorbed);
        eventInformation->IncBoundaryAbsorption();
        break;
      case Detection: //Note, this assumes that the volume causing detection
                      //is the photocathode because it is the only one with
                      //non-zero efficiency
        {
	  //Triger sensitive detector manually since photon is
	  //absorbed but status was Detection
	  G4SDManager* SDman = G4SDManager::GetSDMpointer();
	  G4String sdName = "/MilliQDet/pmtSD";
	  MilliQPMTSD* pmtSD = (MilliQPMTSD*)SDman->FindSensitiveDetector(sdName);
	  if(pmtSD) {
	    pmtSD->ProcessHits_constStep(theStep, NULL);
	  }
	  trackInformation->AddTrackStatusFlag(hitPMT);
	  break;
        }
      case FresnelReflection:
      case TotalInternalReflection:
      case LambertianReflection:
      case LobeReflection:
      case SpikeReflection:
      case BackScattering:
        trackInformation->IncReflections();
        fExpectedNextStatus=StepTooSmall;
        break;
      default:
        break;
      }
    }
  }

  if(fRecorder) fRecorder->RecordStep(theStep);
}
