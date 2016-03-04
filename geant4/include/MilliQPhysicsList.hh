
#ifndef MilliQPhysicsList_h
#define MilliQPhysicsList_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

class MilliQPhysicsList: public G4VModularPhysicsList
{
  public:

    MilliQPhysicsList();
    virtual ~MilliQPhysicsList();

  public:

    virtual void SetCuts();

};

#endif
