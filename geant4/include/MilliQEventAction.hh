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
// $Id: MilliQEventAction.hh 68752 2013-04-05 10:23:47Z gcosmo $
//
/// \file optical/MilliQ/include/MilliQEventAction.hh
/// \brief Definition of the MilliQEventAction class
//

#ifndef MilliQEventAction_h
#define MilliQEventAction_h 1

#include "MilliQDataFormat.hh"
#include "MilliQEventMessenger.hh"
#include "G4UserEventAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

class G4Event;
class MilliQRecorderBase;

class MilliQEventAction : public G4UserEventAction {
 public:

  MilliQEventAction(MilliQRecorderBase*, const G4int numBlocks, const G4int numStacks);
  virtual ~MilliQEventAction();

  virtual void BeginOfEventAction(const G4Event*);
  virtual void EndOfEventAction(const G4Event*);

  void SetSaveThreshold(G4int);

  void SetEventVerbose(G4int v){ fVerbose = v;}

  void SetPMTThreshold(G4int t) { fPMTThreshold = t;}

  void SetForceDrawPhotons(G4bool b) { fForcedrawphotons = b;}
  void SetForceDrawNoPhotons(G4bool b) { fForcenophotons = b;}

 private:

  MilliQRecorderBase*   fRecorder;
  MilliQEventMessenger* fEventMessenger;

  G4int fSaveThreshold;

  G4int fPMTCollID;
  G4int fPMTAllCollID;
  G4int fScintCollID;

  G4int fVerbose;

  G4int fPMTThreshold;

  G4bool fForcedrawphotons;
  G4bool fForcenophotons;

  const G4int NBlocks;
  const G4int NStacks;

};

#endif
