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
// $Id: MilliQActionInitialization.hh 68058 2013-03-13 14:47:43Z gcosmo $
//
/// \file MilliQActionInitialization.hh
/// \brief Definition of the MilliQActionInitialization class

#ifndef MilliQActionInitialization_h
#define MilliQActionInitialization_h 1

#include "G4VUserActionInitialization.hh"

#include "G4Types.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

class MilliQRecorderBase;
class B4DetectorConstruction;

/// Action initialization class.
///

class MilliQActionInitialization : public G4VUserActionInitialization {

public:
  MilliQActionInitialization(MilliQRecorderBase* recorder, const G4int numBlocks, const G4int numStacks, const G4int geometryVersion, const boost::property_tree::ptree pt)
    : G4VUserActionInitialization(), fRecorder(recorder), NBlocks(numBlocks), NStacks(numStacks), fAlternate(geometryVersion), fPTree(pt) {}
  virtual ~MilliQActionInitialization() {};

  virtual void BuildForMaster() const;
  virtual void Build() const;

  virtual G4VSteppingVerbose* InitializeSteppingVerbose() const;

private:
  MilliQRecorderBase* fRecorder;

  const G4int NBlocks;
  const G4int NStacks;
  const G4int fAlternate;

  const boost::property_tree::ptree fPTree;

};

#endif
