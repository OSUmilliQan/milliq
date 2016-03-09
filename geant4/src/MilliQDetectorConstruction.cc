#include "MilliQDetectorConstruction.hh"
#include "MilliQRunAction.hh"
#include "MilliQPMTSD.hh"
#include "MilliQScintSD.hh"
#include "MilliQDetectorMessenger.hh"
#include "MilliQDetectorBlockLV.hh"
#include "MilliQDetectorStackLV.hh"

#include "G4SDManager.hh"
#include "G4RunManager.hh"

#include "G4GeometryManager.hh"
#include "G4SolidStore.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4VSensitiveDetector.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"

#include "G4OpticalSurface.hh"
#include "G4MaterialTable.hh"
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"

#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "globals.hh"
#include "G4UImanager.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "MilliQMonopoleFieldSetup.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4UniformMagField.hh"
#include "G4MagneticField.hh"
#include "G4GlobalMagFieldMessenger.hh"
#include "G4AutoDelete.hh"
#include "G4UserLimits.hh"

G4ThreadLocal G4GlobalMagFieldMessenger* MilliQDetectorConstruction::fMagFieldMessenger = 0;

MilliQDetectorConstruction::MilliQDetectorConstruction(const boost::property_tree::ptree pt) :
  NStacks(0), NBlocks(G4ThreeVector()), fAlternate(0), fMagField(0), fScintillator_mt(NULL), fPTree(pt) {

  fWorldPV = NULL;
  fWorldPVCheckPhysics = NULL;
  worldLV = NULL;
  fMagneticVolume = NULL;

  fScintillatorMaterial = fAluminiumMaterial = fAirMaterial = fGlassMaterial = fVacuumMaterial = NULL;

  fMonFieldSetup = MilliQMonopoleFieldSetup::GetMonopoleFieldSetup();

  //durp
  //G4cout << fPTree.get<int>("Section1.Value1") << G4endl;
  //G4cout << fPTree.get<std::string>("Section1.Value2") << G4endl;

  SetDefaults();
  fDetectorMessenger = new MilliQDetectorMessenger(this);
}

void MilliQDetectorConstruction::SetDefaults() {

  //Resets to default values
  fD_mtl = 5. * mm;

  fAlternate = fPTree.get<G4int>("DetectorGeometry.Version");

  NBlocks = G4ThreeVector(fPTree.get<G4double>("DetectorGeometry.NBlocks_X"),
                          fPTree.get<G4double>("DetectorGeometry.NBlocks_Y"),
                          fPTree.get<G4double>("DetectorGeometry.NBlocks_Z"));
  NStacks = fPTree.get<G4double>("DetectorGeometry.NStacks");

  fOffset = G4ThreeVector(fPTree.get<G4double>("DetectorGeometry.Offset_X") * cm,
                          fPTree.get<G4double>("DetectorGeometry.Offset_Y") * cm,
                          fPTree.get<G4double>("DetectorGeometry.Offset_Z") * cm);

  fBetweenBlockSpacing = G4ThreeVector((fPTree.get<G4double>("DetectorGeometry.BetweenBlockSpacing_X") + fPTree.get<G4double>("PMT.Length")) * cm,
                                       fPTree.get<G4double>("DetectorGeometry.BetweenBlockSpacing_Y") * cm,
                                       fPTree.get<G4double>("DetectorGeometry.BetweenBlockSpacing_Z") * cm);

  fLightGuideLength = fPTree.get<G4double>("ScintillatorGeometry.LightGuideLength") * cm;

  fScint_x = fPTree.get<G4double>("ScintillatorGeometry.X") * cm;
  fScint_y = fPTree.get<G4double>("ScintillatorGeometry.Y") * cm;
  fScint_z = fPTree.get<G4double>("ScintillatorGeometry.Z") * cm;

  fScintHouseThick = fPTree.get<G4double>("ScintillatorGeometry.HousingThickness") * cm;
  fScintillatorHouseRefl = fPTree.get<G4double>("ScintillatorGeometry.HousingReflectivity");

  fOuterRadius_pmt = fPTree.get<G4double>("PMT.OuterRadius") * cm;
  fPmtRad = fPTree.get<G4double>("PMT.Radius") * cm;
  fPmtPhotoRad = fPTree.get<G4double>("PMT.CathodeRadius") * cm;
  fPmtPhotoHeight = fPTree.get<G4double>("PMT.CathodeHeight") * cm;
  fLGHouseRefl = fPTree.get<G4double>("PMT.HousingReflectivity");

  fRefl = 1.0;

  fDetectorStack = NULL;

  //Define shielding thickness
  shield1Thick = G4ThreeVector(10.0 * cm, 10.0 * cm, 10.0 * cm);
  shield2Thick = G4ThreeVector(10.0 * cm, 10.0 * cm, 10.0 * cm);
  detShieldGap = G4ThreeVector(1. * cm, 1. * cm, 1. * cm); //Make sure to put it bigger than the offset!

  //	G4UImanager::GetUIpointer()->ApplyCommand(
  //			"/MilliQ/detector/scintYieldFactor 1.");

  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void MilliQDetectorConstruction::DefineMaterials() {

  G4NistManager* nist = G4NistManager::Instance();
  nist->SetVerbose(0);

  // Elements
  //                            name          symbole number  wieght
  G4Element* fH = new G4Element("Hydrogen", "H", 1., 1.01 * g / mole);
  G4Element* fC = new G4Element("Carbon", "C", 6., 12.01 * g / mole);
  G4Element* fN = new G4Element("Nitrogen", "N", 7., 14.01 * g / mole);
  G4Element* fO = new G4Element("Oxegen", "O", 8., 16.00 * g / mole);

  //
  // Materials
  //

  //Aluminum
  fAluminiumMaterial = new G4Material("Aluminium", //name
				      13., //number
				      26.98 * g / mole, //wieght
				      2.7 * g / cm3); //density

  //Vacuum
  fVacuumMaterial = new G4Material("Vacuum", //name
				   1., //atomic number
				   1.01 * g / mole, //weight
				   universe_mean_density, //density
				   kStateGas, //state
				   0.1 * kelvin, //tempature
				   1.e-19 * pascal); //presure
  //Air
  fAirMaterial = new G4Material("Air", //name
				1.29 * mg / cm3, //density
				2); //n elements
  fAirMaterial->AddElement(fN, 70 * perCent); //compose of nitrogen
  fAirMaterial->AddElement(fO, 30 * perCent); //compose of oxegen

  //Glass
  fGlassMaterial = new G4Material("Glass", 1.032*g/cm3,2);
  fGlassMaterial->AddElement(fC,91.533*perCent);
  fGlassMaterial->AddElement(fH,8.467*perCent);


  //Concrete
  fConcreteMaterial = nist->FindOrBuildMaterial("G4_CONCRETE");

  //Shielding Material
  led = G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb");
  polyethylene = G4NistManager::Instance()->FindOrBuildMaterial(
								"G4_POLYETHYLENE");

  //
  //Material properties tables
  //

  // To see what all these settings do,
  // see source/processes/electromagnetic/xrays/src/G4Scintillation.cc

  //Settings for BC-408
  //References: http://iopscience.iop.org/article/10.1088/1674-1137/34/7/011/pdf
  //References: http://www.phys.ufl.edu/courses/phy4803L/group_I/muon/bicron_bc400-416.pdf
  fScintillatorMaterial = new G4Material("Scintillator", //name
					 1.032 * g / cm3, //density
					 2); //n elements
  fScintillatorMaterial->AddElement(fC, 9);
  fScintillatorMaterial->AddElement(fH, 10);
  //Emission Spectrum energies:
  G4double MilliQ_Energy[] = { 2.53 * eV, 2.58 * eV, 2.64 * eV, 2.70 * eV,
			       2.76 * eV, 2.79 * eV, 2.82 * eV, 2.88 * eV, 2.95 * eV, 2.98 * eV,
			       3.02 * eV, 3.10 * eV, 3.18 * eV };
  const G4int MilliQnum = sizeof(MilliQ_Energy) / sizeof(G4double);
  G4double MilliQ_Fast_SCINT[] = { 0.1, 0.15, 0.25, 0.4, 0.55, 0.62, 0.83, 1.,
				   0.83, 0.6, 0.4, 0.2, 0.1 }; //Relative light output for the emission spectrum
  assert(sizeof(MilliQ_Fast_SCINT) == sizeof(MilliQ_Energy));
  G4double MilliQ_RIND[] = { 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58,
			     1.58, 1.58, 1.58, 1.58, 1.58 }; //{ 1.59, 1.57, 1.54 };
  assert(sizeof(MilliQ_RIND) == sizeof(MilliQ_Energy));
  G4double MilliQ_ABSL[] = { 210. * cm, 210. * cm, 210. * cm, 210. * cm, 210.
			     * cm, 210. * cm, 210. * cm, 210. * cm, 210. * cm, 210. * cm, 210.
			     * cm, 210. * cm, 210. * cm };
  assert(sizeof(MilliQ_ABSL) == sizeof(MilliQ_Energy));
  fScintillator_mt = new G4MaterialPropertiesTable();
  fScintillator_mt->AddProperty("FASTCOMPONENT", MilliQ_Energy,
				MilliQ_Fast_SCINT, MilliQnum); //I think is setting the max emission to be 430nm (need to check)
  //	fScintillator_mt->AddProperty("SLOWCOMPONENT", MilliQ_Energy, MilliQ_Slow_SCINT,
  //			MilliQnum);
  fScintillator_mt->AddProperty("RINDEX", MilliQ_Energy, MilliQ_RIND,
				MilliQnum);
  fScintillator_mt->AddProperty("ABSLENGTH", MilliQ_Energy, MilliQ_ABSL,
				MilliQnum);
  fScintillator_mt->AddConstProperty("SCINTILLATIONYIELD",
				     0.64 * 17400. / MeV); //64% of Anthracene
  fScintillator_mt->AddConstProperty("RESOLUTIONSCALE", 1.0);
  fScintillator_mt->AddConstProperty("FASTTIMECONSTANT", 2.1 * ns); //This is decay time
  fScintillator_mt->AddConstProperty("FASTSCINTILLATIONRISETIME", 0.9 * ns); //Need to set setFiniteRiseTime true in messenger!!
  //	fScintillator_mt->AddConstProperty("SLOWTIMECONSTANT", 0. * ns); //45
  fScintillator_mt->AddConstProperty("YIELDRATIO", 1.0);
  fScintillatorMaterial->SetMaterialPropertiesTable(fScintillator_mt);
  // Set the Birks Constant
  fScintillatorMaterial->GetIonisation()->SetBirksConstant(0.111 * mm / MeV);

  G4double glass_RIND[]={1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49};
  assert(sizeof(glass_RIND) == sizeof(MilliQ_Energy));
  G4double glass_AbsLength[]={420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm};
  assert(sizeof(glass_AbsLength) == sizeof(MilliQ_Energy));
  G4MaterialPropertiesTable *glass_mt = new G4MaterialPropertiesTable();
  glass_mt->AddProperty("ABSLENGTH",MilliQ_Energy,glass_AbsLength,MilliQnum);
  glass_mt->AddProperty("RINDEX",MilliQ_Energy,glass_RIND,MilliQnum);
  fGlassMaterial->SetMaterialPropertiesTable(glass_mt);

  //	G4double vacuum_Energy[] = { 2.0 * eV, 7.0 * eV, 7.14 * eV };
  //	const G4int vacnum = sizeof(vacuum_Energy) / sizeof(G4double);
  G4double vacuum_RIND[] = { 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1.,
			     1. };
  assert(sizeof(vacuum_RIND) == sizeof(MilliQ_Energy));
  G4MaterialPropertiesTable *vacuum_mt = new G4MaterialPropertiesTable();
  vacuum_mt->AddProperty("RINDEX", MilliQ_Energy, vacuum_RIND, MilliQnum);
  fVacuumMaterial->SetMaterialPropertiesTable(vacuum_mt);
  fAirMaterial->SetMaterialPropertiesTable(vacuum_mt); //Give air the same rindex
}

void MilliQDetectorConstruction::DefineMaterialsStewart() {

  G4NistManager* nist = G4NistManager::Instance();
  nist->SetVerbose(0);

  // Elements
  //                            name          symbole number  wieght
  G4Element* fH = new G4Element("Hydrogen", "H", 1., 1.01 * g / mole);
  G4Element* fC = new G4Element("Carbon", "C", 6., 12.01 * g / mole);
  G4Element* fN = new G4Element("Nitrogen", "N", 7., 14.01 * g / mole);
  G4Element* fO = new G4Element("Oxegen", "O", 8., 16.00 * g / mole);

  //
  // Materials
  //

  //Aluminum
  fAluminiumMaterial = new G4Material("Aluminium", //name
				      13., //number
				      26.98 * g / mole, //wieght
				      2.7 * g / cm3); //density

  //Vacuum
  fVacuumMaterial = new G4Material("Vacuum", //name
				   1., //atomic number
				   1.01 * g / mole, //weight
				   universe_mean_density, //density
				   kStateGas, //state
				   0.1 * kelvin, //tempature
				   1.e-19 * pascal); //presure
  //Air
  fAirMaterial = new G4Material("Air", //name
				1.29 * mg / cm3, //density
				2); //n elements
  fAirMaterial->AddElement(fN, 70 * perCent); //compose of nitrogen
  fAirMaterial->AddElement(fO, 30 * perCent); //compose of oxegen

  //Glass
  fGlassMaterial = new G4Material("Glass", 1.032*g/cm3,2);
  fGlassMaterial->AddElement(fC,91.533*perCent);
  fGlassMaterial->AddElement(fH,8.467*perCent);

  //Concrete
  fConcreteMaterial = nist->FindOrBuildMaterial("G4_CONCRETE");

  //Shielding Material
  led = G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb");
  polyethylene = G4NistManager::Instance()->FindOrBuildMaterial(
								"G4_POLYETHYLENE");

  // To see what all these settings do,
  // see source/processes/electromagnetic/xrays/src/G4Scintillation.cc

  //Settings for BCF-12
  //References: http://www.crystals.saint-gobain.com/uploadedFiles/SG-Crystals/Documents/SGC%20Fibers%20Brochure.pdf
  //References: http://arxiv.org/pdf/0911.3041.pdf
  fScintillatorMaterial = new G4Material("Scintillator", //name
					 1.05 * g / cm3, //density
					 2); //n elements
  fScintillatorMaterial->AddElement(fC, 8);
  fScintillatorMaterial->AddElement(fH, 8);
  //Emission Spectrum energies:
  G4double MilliQ_Energy[] = { 2.38 * eV, 2.43 * eV, 2.48 * eV, 2.53 * eV,
			       2.58 * eV, 2.64 * eV, 2.70 * eV, 2.76 * eV, 2.82 * eV, 2.88 * eV,
			       2.95 * eV, 3.02 * eV, 3.10 * eV };
  const G4int MilliQnum = sizeof(MilliQ_Energy) / sizeof(G4double);
  G4double MilliQ_Fast_SCINT[] = { 0.1, 0.2, 0.25, 0.3, 0.4, 0.6, 0.7, 0.8,
				   0.9, 1., 0.7, 0.3, 0.1 }; //Relative light output for the emission spectrum
  assert(sizeof(MilliQ_Fast_SCINT) == sizeof(MilliQ_Energy));
  G4double MilliQ_RIND[] = { 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6, 1.6,
			     1.6, 1.6, 1.6 };
  assert(sizeof(MilliQ_RIND) == sizeof(MilliQ_Energy));
  G4double MilliQ_ABSL[] = { 2.7 * m, 2.7 * m, 2.7 * m, 2.7 * m, 2.7 * m, 2.7
			     * m, 2.7 * m, 2.7 * m, 2.7 * m, 2.7 * m, 2.7 * m, 2.7 * m, 2.7 * m };
  assert(sizeof(MilliQ_ABSL) == sizeof(MilliQ_Energy));
  fScintillator_mt = new G4MaterialPropertiesTable();
  fScintillator_mt->AddProperty("FASTCOMPONENT", MilliQ_Energy,
				MilliQ_Fast_SCINT, MilliQnum); //I think is setting the max emission to be 430nm (need to check)
  //	fScintillator_mt->AddProperty("SLOWCOMPONENT", MilliQ_Energy, MilliQ_Slow_SCINT,
  //			MilliQnum);
  fScintillator_mt->AddProperty("RINDEX", MilliQ_Energy, MilliQ_RIND,
				MilliQnum);
  fScintillator_mt->AddProperty("ABSLENGTH", MilliQ_Energy, MilliQ_ABSL,
				MilliQnum);
  fScintillator_mt->AddConstProperty("SCINTILLATIONYIELD", 8000. / MeV);
  fScintillator_mt->AddConstProperty("RESOLUTIONSCALE", 1.0);
  fScintillator_mt->AddConstProperty("FASTTIMECONSTANT", 3.2 * ns); //This is decay time
  //	fScintillator_mt->AddConstProperty("FASTSCINTILLATIONRISETIME", 0.9*ns);//Need to set setFiniteRiseTime true in messenger!!
  //	fScintillator_mt->AddConstProperty("SLOWTIMECONSTANT", 0. * ns); //45
  fScintillator_mt->AddConstProperty("YIELDRATIO", 1.0);
  fScintillatorMaterial->SetMaterialPropertiesTable(fScintillator_mt);
  // 	Set the Birks Constant
  fScintillatorMaterial->GetIonisation()->SetBirksConstant(0.0872 * mm / MeV);

  G4double glass_RIND[]={1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49,1.49};
  assert(sizeof(glass_RIND) == sizeof(MilliQ_Energy));
  G4double glass_AbsLength[]={420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm,420.*cm};
  assert(sizeof(glass_AbsLength) == sizeof(MilliQ_Energy));
  G4MaterialPropertiesTable *glass_mt = new G4MaterialPropertiesTable();
  glass_mt->AddProperty("ABSLENGTH",MilliQ_Energy,glass_AbsLength,MilliQnum);
  glass_mt->AddProperty("RINDEX",MilliQ_Energy,glass_RIND,MilliQnum);
  fGlassMaterial->SetMaterialPropertiesTable(glass_mt);


  //	G4double vacuum_Energy[] = { 2.0 * eV, 7.0 * eV, 7.14 * eV };
  //	const G4int vacnum = sizeof(vacuum_Energy) / sizeof(G4double);
  G4double vacuum_RIND[] = { 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1., 1.,
			     1. };
  assert(sizeof(vacuum_RIND) == sizeof(MilliQ_Energy));
  G4MaterialPropertiesTable *vacuum_mt = new G4MaterialPropertiesTable();
  vacuum_mt->AddProperty("RINDEX", MilliQ_Energy, vacuum_RIND, MilliQnum);
  fVacuumMaterial->SetMaterialPropertiesTable(vacuum_mt);
  fAirMaterial->SetMaterialPropertiesTable(vacuum_mt); //Give air the same rindex
}

G4VPhysicalVolume* MilliQDetectorConstruction::Construct() {

  if (fWorldPV) {
    G4GeometryManager::GetInstance()->OpenGeometry();
    G4PhysicalVolumeStore::GetInstance()->Clean();
    G4LogicalVolumeStore::GetInstance()->Clean();
    G4SolidStore::GetInstance()->Clean();
    G4LogicalSkinSurface::CleanSurfaceTable();
    G4LogicalBorderSurface::CleanSurfaceTable();
  }

  if (fAlternate == 2) {
    DefineMaterialsStewart();
  } else {
    DefineMaterials();
  }
  return ConstructDetector();
}

G4VPhysicalVolume* MilliQDetectorConstruction::ConstructDetector() {
  //
  //World
  //
  G4double worldVx = 8. * m;
  G4double worldVy = 8. * m;
  G4double worldVz = 8. * m;

  //World - Volume
  G4Box* worldV = new G4Box("World Volume", //name
			    worldVx, worldVy, worldVz); //dimentions
  //World - Logical Volume
  worldLV = new G4LogicalVolume(worldV, G4Material::GetMaterial("Air"),
				"World Logical Volume", 0, 0, 0);
  //World - Physical Volume
  fWorldPV = new G4PVPlacement(0, //rotation
			       G4ThreeVector(), //translation
			       worldLV, //logical volume
			       "World Physical Volume", //name
			       0, false, 0);

  if (fAlternate == 1) {

    ConstructCheckGeometry();
    return fWorldPVCheckPhysics;
  }

  //worldLV->SetVisAttributes(G4VisAttributes::Invisible);

  //Photocathode Sensitive Detectors setup
  //

  G4VSensitiveDetector* pmt_SD = new MilliQPMTSD("/MilliQDet/pmtSD", fPTree);
  G4SDManager* sDManager = G4SDManager::GetSDMpointer();
  sDManager->AddNewDetector(pmt_SD);

  G4VSensitiveDetector* scint_SD = new MilliQScintSD("/MilliQDet/scintSD", fPTree);
  sDManager->AddNewDetector(scint_SD);

  //
  // Detection Room
  //

  // Detection Room - Volume
  G4Box* detectionRoomV = new G4Box("Detection Room Volume",  //name
				    worldVx * 0.95, worldVy * 0.95, worldVz * 0.95); //temp dimentions

  // Detection Room - Logical Volume
  G4LogicalVolume* detectionRoomLV = new G4LogicalVolume(detectionRoomV, //volume
							 G4Material::GetMaterial("Air"), //material
							 "Detection Room Logical Volume"); //name

  G4ThreeVector DetectionRoomTrans = G4ThreeVector(0. * m, 0. * m, 0. * m);

  // Detection Room - Physical Volume
  new G4PVPlacement(0, //rotation
		    DetectionRoomTrans, //translation
		    detectionRoomLV, //logical volume
		    "Detection Room Physical Volume", //name
		    worldLV, //mother logical volume
		    false, //many
		    0); //copy n

  //
  // Detector Stacks
  //

  // Detector Stacks - Volume
  G4Box* stackHouseingV = new G4Box("Detector Stack Housing Volume", 1. * m,
				    1. * m, 1. * m);    //temp dimension

  // Detector Stacks - Logical Volume   (All these parameters are defined above)
  MilliQDetectorStackLV* aDetectorStackLV = new MilliQDetectorStackLV(
								      stackHouseingV, //volume
								      G4Material::GetMaterial("Air"), //material
								      "Detector Stack H", //name
								      0, //field manager
								      0, //sensitve detector
								      0, //user limits
								      true, //optimise

								      NBlocks, //number of blocks
								      fBetweenBlockSpacing, //between block spacing

								      G4ThreeVector(fScint_x, fScint_y, fScint_z), //scintillator dimensions
								      fScintHouseThick, //scintillator housing thickness (and Glass Radius Height)
								      fLightGuideLength, //light guide inside scintillator
								      fScintillatorHouseRefl, //scintillator housing reflectivity

								      fPmtPhotoRad, //pmt Photocathode radius
								      fPmtPhotoHeight, //pmt photocathode height
								      fLGHouseRefl, //pmt housing reflective
								      pmt_SD, //pmt sensitive detector
								      scint_SD); //scintillator sensitive detector

  G4double TotalStackStart = shield1Thick.x() + shield2Thick.x()
    + detShieldGap.x();
  G4double TotalStackEnd = TotalStackStart + fScint_x * NStacks
    + fBetweenBlockSpacing.x() * (NStacks - 1);

  // Detector Stacks - Parameterisation
  MilliQDetectorStackParameterisation* fDetectorStackParameterisation =
    new MilliQDetectorStackParameterisation(NStacks, //n
					    aDetectorStackLV->GetDimensions(), //block dimensions
					    G4ThreeVector(1., 0., 0.), //alignment vector
					    fOffset, //offset to remove background
					    TotalStackStart, //start depth
					    TotalStackEnd); //end depth

  // Detector Stacks - Physical Volume
  new G4PVParameterised("Detector Stack Housing Physical Volume",
			aDetectorStackLV, detectionRoomLV, kZAxis,
			fDetectorStackParameterisation->GetNumberOfBlocks(),
			fDetectorStackParameterisation);

  if (fAlternate == 2) {
    //
    // Concrete Wall
    //
    G4Box* wallV = new G4Box("Wall V", 1.5 * m, worldVy * 0.95,
			     worldVz * 0.95);
    G4LogicalVolume* wallLV = new G4LogicalVolume(wallV, fConcreteMaterial,
						  "Wall LV", 0, 0, 0);
    wallLV->SetVisAttributes(G4Colour(.5, .5, .5, .5));
    new G4PVPlacement(0, G4ThreeVector(-1.5 * m, 0., 0.), wallLV, "Wall PV",
		      worldLV, 0, 0, 0);
  } else if (fAlternate == 0) {
    ConstructShield(worldLV, TotalStackStart, TotalStackEnd);
  }
  worldLV->SetUserLimits(new G4UserLimits(0.2 * mm));

  return fWorldPV;

}

void MilliQDetectorConstruction::ConstructCheckGeometry() {
  G4cout << "This current mode checks DEDX and the radius of curvature "
    "for various charges and masses of the monopole " << G4endl;

  fWorldPVCheckPhysics = new G4PVPlacement(0, //rotation
					   G4ThreeVector(), //translation
					   worldLV, //logical volume
					   "World Physical Volume", //name
					   0, false, 0);

  /*	// BendingRoom - Volume
	G4Box* bendingRoomV = new G4Box("Bending Room Volume",  //name
	10. / 10. * m, 20. / 10. * m, 20. / 10. * m); //temp dimentions

	// Bending Room - Logical Volume
	fMagneticVolume = new G4LogicalVolume(bendingRoomV, //volume
	G4Material::GetMaterial("Air"), //material
	"Bending Room Logical Volume"); //name

	// Bending Room - Physical Volume
	new G4PVPlacement(0, //rotation
	G4ThreeVector(), //translation
	fMagneticVolume, //logical volume
	"Detection Room Physical Volume", //name
	worldLV, //mother logical volume
	false, //many
	0); //copy n
  */
  //SetMagField(3.* tesla);
}

void MilliQDetectorConstruction::ConstructShield(G4LogicalVolume* dworldLV,
						 G4double dTotalStackStart, G4double dTotalStackEnd) {

  //
  //Shielding Around Experiment
  //

  // The shield is an open box of a certain thickness
  //Define led shielding inner half length(polyethylene butted around led)
  G4int nOffsets = 2;
  G4double xShield = (2 * fScintHouseThick + dTotalStackEnd - dTotalStackStart
		      + 2 * detShieldGap.x()) * 0.5;
  G4double yShield = (NBlocks.y() * fScint_y
		      + fBetweenBlockSpacing.y() * (NBlocks.y() - 1)
		      + 2 * detShieldGap.y() + fOffset.y() * nOffsets) * 0.5
    + shield1Thick.y();
  G4double zShield = (NBlocks.z() * fScint_z
		      + fBetweenBlockSpacing.z() * (NBlocks.z() - 1)
		      + 2 * detShieldGap.z() + fOffset.z() * nOffsets) * 0.5
    + shield1Thick.z();

  G4ThreeVector shield1InnerHL = G4ThreeVector(xShield, yShield, zShield);
  //Define center location of led shielding (relative to global center)
  G4ThreeVector centreShield1 = G4ThreeVector(0, 0, 0);

  //Define global center for shielding
  G4ThreeVector centreGlobalShield = G4ThreeVector(
						   fScintHouseThick + dTotalStackStart
						   + (dTotalStackEnd - dTotalStackStart) * 0.5, 0, 0);

  //
  // Polyethylene Shielding Container (Radiation Shield)
  //
  G4ThreeVector shield2InnerHL = shield1InnerHL + shield1Thick;
  G4ThreeVector BoxOutSideShield2HL = shield2InnerHL + shield2Thick;

  G4Box *boxInSideShield2 = new G4Box("BoxInSideShield2",
				      shield2InnerHL.getX(), shield2InnerHL.getY(),
				      shield2InnerHL.getZ());

  G4Box *boxOutSideShield2 = new G4Box("BoxOutSideShield2",
				       BoxOutSideShield2HL.getX(), BoxOutSideShield2HL.getY(),
				       BoxOutSideShield2HL.getZ());

  // boolean logic subtraction

  G4SubtractionSolid* OutMinusInBoxShield2 = new G4SubtractionSolid(
								    "OutMinusInBoxShield2", boxOutSideShield2, boxInSideShield2, 0,
								    G4ThreeVector());

  G4LogicalVolume *OutMinusInBoxShield2LV = new G4LogicalVolume(
								OutMinusInBoxShield2, polyethylene, "OutMinusInBoxShield2LV", 0, 0,
								0);
  new G4PVPlacement(0, centreGlobalShield, OutMinusInBoxShield2LV,
		    "OutMinusInBoxShield2PV", dworldLV, false, 0);

  OutMinusInBoxShield2LV->SetVisAttributes(G4Colour(0., 0., 1.)); //Blue

  //
  // Led Shielding Container (Neutron Shield)
  //

  G4ThreeVector BoxOutSideShield1HL = shield1InnerHL + shield1Thick;

  G4Box *boxInSideShield1 = new G4Box("BoxInSideShield1",
				      shield1InnerHL.getX(), shield1InnerHL.getY(),
				      shield1InnerHL.getZ());

  G4Box *boxOutSideShield1 = new G4Box("BoxOutSideShield1",
				       BoxOutSideShield1HL.getX(), BoxOutSideShield1HL.getY(),
				       BoxOutSideShield1HL.getZ());

  // boolean logic subtraction

  G4SubtractionSolid* OutMinusInBoxShield1 = new G4SubtractionSolid(
								    "OutMinusInBoxShield1", boxOutSideShield1, boxInSideShield1, 0,
								    G4ThreeVector());

  G4LogicalVolume *OutMinusInBoxShield1LV = new G4LogicalVolume(
								OutMinusInBoxShield1, led, "OutMinusInBoxShield1LV", 0, 0, 0);
  new G4PVPlacement(0, G4ThreeVector(), OutMinusInBoxShield1LV,
		    "OutMinusInBoxShield1PV", OutMinusInBoxShield2LV, false, 0);

  // Visualisation attributes of Shield1
  G4VisAttributes * grayBox = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5));

  OutMinusInBoxShield1LV->SetVisAttributes(grayBox);

}

void MilliQDetectorConstruction::SetMagField(G4double fieldValue) {

  fMonFieldSetup->SetMagField(fieldValue);

  /*	if (!fEmFieldSetup.Get()) {
	MilliQMonopoleFieldSetup* emFieldSetup = new MilliQMonopoleFieldSetup();

	fEmFieldSetup.Put(emFieldSetup);
	G4AutoDelete::Register(emFieldSetup); //Kernel will delete the messenger
	}
	// Set local field manager and local field in radiator and its daughters:
	G4bool allLocal = true;
	fMagneticVolume->SetFieldManager(fEmFieldSetup.Get()->GetLocalFieldManager(),
	allLocal );
  */
  /*
  //apply a global uniform magnetic field along Z axis
  G4FieldManager * fieldMgr =
  G4TransportationManager::GetTransportationManager()->GetFieldManager();

  if (fMagField) { delete fMagField; }        //delete the existing magn field

  if (fieldValue != 0.)                        // create a new one if non nul
  {
  fMagField = new G4UniformMagField(G4ThreeVector(0., 0., fieldValue));
  fieldMgr->SetDetectorField(fMagField);
  fieldMgr->CreateChordFinder(fMagField);
  }
  else
  {
  fMagField = 0;
  fieldMgr->SetDetectorField(fMagField);
  }
  */
}

void MilliQDetectorConstruction::ConstructSDandField() {

  if (!fDetectorStack)
    return;

  // PMT SD

  //MilliQPMTSD* pmt_SD = new MilliQPMTSD("/MilliQDet/pmtSD");

  /*
    if (!fPmt_SD.Get()) {
    //Created here so it exists as pmts are being placed
    G4cout << "Construction /MilliQDet/pmtSD" << G4endl;
    MilliQPMTSD* pmt_SD = new MilliQPMTSD("/MilliQDet/pmtSD");
    fPmt_SD.Put(pmt_SD);
    }
  */
  //((MilliQDetectorBlockLV*)fDetectorStack->GetLogicalVolume()->GetDaughter(0)->GetLogicalVolume())->GetPhotocathodeLV()->SetSensitiveDetector(fPmt_SD.Get());
  //for(int a=0; a<fDetectorStack->GetLogicalVolume()->GetNoDaughters(); a++){
  //fDetectorStack->GetLogicalVolume()->GetDaughter(0)->GetLogicalVolume()->GetDaughter(1)->GetLogicalVolume()->GetDaughter(1)->GetLogicalVolume()->GetName()
  //MilliQDetectorBlockLV* aBlockLV = (MilliQDetectorBlockLV*)fDetectorStack->GetLogicalVolume()->GetDaughter(a);
  //MilliQDetectorBlockLV* aBlockLV = (MilliQDetectorBlockLV*)fDetectorStack->GetLogicalVolume()->GetDaughter(i);
  //SetSensitiveDetector(aBlockLV->GetPhotocathodeLV(), fPmt_SD.Get());
  //MilliQDetectorBlockLV* aBlockLV = (MilliQDetectorBlockLV*)fDetectorStack->GetLogicalVolume()->GetDaughter(i);
  //aBlockLV->GetPhotocathodeLV()->SetSensitiveDetector(fPmt_SD.Get());
  //}
}

void MilliQDetectorConstruction::SetHousingThickness(G4double d_mtl) {
  this->fD_mtl = d_mtl;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void MilliQDetectorConstruction::SetPMTRadius(G4double outerRadius_pmt) {
  this->fOuterRadius_pmt = outerRadius_pmt;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void MilliQDetectorConstruction::SetHousingReflectivity(G4double r) {
  fRefl = r;
  G4RunManager::GetRunManager()->ReinitializeGeometry();
}

void MilliQDetectorConstruction::SetMainScintYield(G4double y) {
  fScintillator_mt->AddConstProperty("SCINTILLATIONYIELD", y / MeV);
}
