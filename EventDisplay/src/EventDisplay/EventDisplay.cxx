/**
 * @file EventDisplay.cxx
 * @author Josh Hiltbrand, University of Minnesota
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "EventDisplay/EventDisplay.h"

ClassImp(ldmx::EventDisplay);

namespace ldmx {

EventDisplay::EventDisplay(TEveManager* manager, bool verbose)
    : TGMainFrame(gClient->GetRoot(), 320, 320), verbose_(verbose) {

  /****************************************************************************
   * GUI Set Up
   ***************************************************************************/

  SetWindowName("LDMX Event Display");

  manager_ = manager;
  TGLViewer* viewer = manager_->GetDefaultGLViewer();
  viewer->UseLightColorSet();

  if (verbose_) {
    std::cout << "[ EventDisplay ] : Drawing detector geometry... "
              << std::flush;
  }

  // when the first TGeoShape (a TGeoTube) is drawn, ROOT creates a default
  // geometry for this drawing and
  // prints an Info statement to std-out. Currently, I can't figure out how to
  // turn this behavior off.
  theDetector_ = new EveDetectorGeometry();
  eventObjects_ = new EventObjects();

  manager_->AddEvent(new TEveEventManager("LDMX Detector", ""));
  manager_->AddElement(theDetector_->getECAL());
  manager_->AddElement(theDetector_->getHCAL());
  manager_->AddElement(theDetector_->getRecoilTracker());
  manager_->AddEvent(new TEveEventManager("LDMX Event", ""));

  if (verbose_) {
    std::cout << "done" << std::endl;
    std::cout << "[ EventDisplay ] : Constructing and linking buttons... "
              << std::flush;
  }

  TGVerticalFrame* contents = new TGVerticalFrame(this, 1000, 1200);
  TGHorizontalFrame* commandFrameNextEvent =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameColorClusters =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameEcalClusterBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameSimThresh =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameEcalHitBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameHcalHitBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameTrackerHitsBranch =
      new TGHorizontalFrame(contents, 100, 0);
  TGHorizontalFrame* commandFrameEcalScorePlaneBranch =
      new TGHorizontalFrame(contents, 100, 0);

  TGButton* buttonColor =
      new TGTextButton(commandFrameColorClusters, "Color Clusters");
  commandFrameColorClusters->AddFrame(buttonColor,
                                      new TGLayoutHints(kLHintsExpandX));
  buttonColor->Connect("Pressed()", "ldmx::EventDisplay", this,
                       "ColorClusters()");

  TGButton* buttonNext =
      new TGTextButton(commandFrameNextEvent, "Next Event >>>");
  commandFrameNextEvent->AddFrame(buttonNext,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonNext->Connect("Pressed()", "ldmx::EventDisplay", this, "NextEvent()");

  textBoxClustersCollName_ =
      new TGTextEntry(commandFrameEcalClusterBranch, new TGTextBuffer(100));
  commandFrameEcalClusterBranch->AddFrame(textBoxClustersCollName_,
                                          new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonClusterName =
      new TGTextButton(commandFrameEcalClusterBranch, "Set Clusters Branch");
  commandFrameEcalClusterBranch->AddFrame(buttonClusterName,
                                          new TGLayoutHints(kLHintsExpandX));
  buttonClusterName->Connect("Pressed()", "ldmx::EventDisplay", this,
                             "GetClustersCollInput()");

  textBoxSimThresh_ =
      new TGTextEntry(commandFrameSimThresh, new TGTextBuffer(100));
  commandFrameSimThresh->AddFrame(textBoxSimThresh_,
                                  new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonDrawThresh =
      new TGTextButton(commandFrameSimThresh, "Sim P [MeV] Threshold");
  commandFrameSimThresh->AddFrame(buttonDrawThresh,
                                  new TGLayoutHints(kLHintsExpandX));
  buttonDrawThresh->Connect("Pressed()", "ldmx::EventDisplay", this,
                            "SetSimThresh()");

  textBoxEcalRecHitsCollName_ =
      new TGTextEntry(commandFrameEcalHitBranch, new TGTextBuffer(100));
  commandFrameEcalHitBranch->AddFrame(textBoxEcalRecHitsCollName_,
                                      new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetECALBranch =
      new TGTextButton(commandFrameEcalHitBranch, "Set ECAL RecHits Branch");
  commandFrameEcalHitBranch->AddFrame(buttonSetECALBranch,
                                      new TGLayoutHints(kLHintsExpandX));
  buttonSetECALBranch->Connect("Pressed()", "ldmx::EventDisplay", this,
                               "GetECALRecHitsCollInput()");

  textBoxHcalRecHitsCollName_ =
      new TGTextEntry(commandFrameHcalHitBranch, new TGTextBuffer(100));
  commandFrameHcalHitBranch->AddFrame(textBoxHcalRecHitsCollName_,
                                      new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetHCALBranch =
      new TGTextButton(commandFrameHcalHitBranch, "Set HCAL RecHits Branch");
  commandFrameHcalHitBranch->AddFrame(buttonSetHCALBranch,
                                      new TGLayoutHints(kLHintsExpandX));
  buttonSetHCALBranch->Connect("Pressed()", "ldmx::EventDisplay", this,
                               "GetHCALRecHitsCollInput()");

  textBoxTrackerHitsCollName_ =
      new TGTextEntry(commandFrameTrackerHitsBranch, new TGTextBuffer(100));
  commandFrameTrackerHitsBranch->AddFrame(textBoxTrackerHitsCollName_,
                                          new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetRecoilBranch =
      new TGTextButton(commandFrameTrackerHitsBranch, "Set Recoil Sims Branch");
  commandFrameTrackerHitsBranch->AddFrame(buttonSetRecoilBranch,
                                          new TGLayoutHints(kLHintsExpandX));
  buttonSetRecoilBranch->Connect("Pressed()", "ldmx::EventDisplay", this,
                                 "GetTrackerHitsCollInput()");

  textBoxEcalScorePlaneBranch_ =
      new TGTextEntry(commandFrameEcalScorePlaneBranch, new TGTextBuffer(100));
  commandFrameEcalScorePlaneBranch->AddFrame(textBoxEcalScorePlaneBranch_,
                                             new TGLayoutHints(kLHintsExpandX));

  TGButton* buttonSetSimParticlesBranch = new TGTextButton(
      commandFrameEcalScorePlaneBranch, "Set Ecal SP Branch");
  commandFrameEcalScorePlaneBranch->AddFrame(buttonSetSimParticlesBranch,
                                             new TGLayoutHints(kLHintsExpandX));
  buttonSetSimParticlesBranch->Connect("Pressed()", "ldmx::EventDisplay", this,
                                       "GetEcalSimParticlesCollInput()");

  // Order from top to bottom here
  contents->AddFrame(commandFrameEcalHitBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameHcalHitBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameTrackerHitsBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameEcalScorePlaneBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameEcalClusterBranch,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameNextEvent,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameColorClusters,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
  contents->AddFrame(commandFrameSimThresh,
                     new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  AddFrame(contents, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();

  manager_->FullRedraw3D(kTRUE);

  if (verbose_) {
    std::cout << "done" << std::endl;
  }
}

bool EventDisplay::SetFile(const TString file) {

  try {
    the_file_ = std::make_unique<framework::EventFile>(file.Data());
    the_file_->setupEvent(&the_event_);
    if (verbose_) {
      std::cout << "[ EventDisplay ] : Input root file opened successfully."
                << std::endl;
    }
  } catch (const framework::exception::Exception& e) {
    std::cerr << "[ EventDisplay ] : Input root file cannot be opened." << std::endl;
    std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
      << "  at " << e.module() << ":" << e.line() << " in "
      << e.function() << "\nStack trace: " << std::endl
      << e.stackTrace();
    return false;
  }
  
  return true;
}

void EventDisplay::NextEvent() {
  if (the_file_->nextEvent(false)) {
    manager_->GetCurrentEvent()->DestroyElements();
    eventObjects_->Initialize();
  
    if (verbose_) {
      std::cout << "[ EventDisplay ] : Loading new event " << "... " << std::endl;
    }

    try {
      auto ecal_rec_hits{the_event_.getCollection<EcalHit>(ecalRecHitsCollName_)};
      eventObjects_->drawECALHits(ecal_rec_hits);
      if (verbose_) {
        std::cout << "[ EventDisplay ] : Loaded '" << ecalRecHitsCollName_
          << "' into memory as a EVE object." << std::endl;
      }
    } catch(const framework::exception::Exception& e) {
      std::cerr << "[ EventDisplay ] : Unable to draw an event object." << std::endl;
      std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
        << "  at " << e.module() << ":" << e.line() << " in "
        << e.function() << "\nStack trace: " << std::endl
        << e.stackTrace();
    }

    try {
      auto hcal_rec_hits{the_event_.getCollection<HcalHit>(hcalRecHitsCollName_)};
      eventObjects_->drawHCALHits(hcal_rec_hits);
      if (verbose_) {
        std::cout << "[ EventDisplay ] : Loaded '" << hcalRecHitsCollName_
          << "' into memory as a EVE object." << std::endl;
      }
    } catch(const framework::exception::Exception& e) {
      std::cerr << "[ EventDisplay ] : Unable to draw an event object." << std::endl;
      std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
        << "  at " << e.module() << ":" << e.line() << " in "
        << e.function() << "\nStack trace: " << std::endl
        << e.stackTrace();
    }

    try {
      auto ecal_clusters{the_event_.getCollection<EcalCluster>(clustersCollName_)};
      eventObjects_->drawECALClusters(ecal_clusters);
      if (verbose_) {
        std::cout << "[ EventDisplay ] : Loaded '" << clustersCollName_
          << "' into memory as a EVE object." << std::endl;
      }
    } catch(const framework::exception::Exception& e) {
      std::cerr << "[ EventDisplay ] : Unable to draw an event object." << std::endl;
      std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
        << "  at " << e.module() << ":" << e.line() << " in "
        << e.function() << "\nStack trace: " << std::endl
        << e.stackTrace();
    }
  
    try {
      auto recoil_hits{the_event_.getCollection<SimTrackerHit>(trackerHitsCollName_)};
      eventObjects_->drawRecoilHits(recoil_hits);
      if (verbose_) {
        std::cout << "[ EventDisplay ] : Loaded '" << trackerHitsCollName_
          << "' into memory as a EVE object." << std::endl;
      }
    } catch(const framework::exception::Exception& e) {
      std::cerr << "[ EventDisplay ] : Unable to draw an event object." << std::endl;
      std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
        << "  at " << e.module() << ":" << e.line() << " in "
        << e.function() << "\nStack trace: " << std::endl
        << e.stackTrace();
    }
  
    try {
      auto ecal_sp{the_event_.getCollection<SimTrackerHit>(ecalSimParticlesCollName_)};
      eventObjects_->drawECALSimParticles(ecal_sp);
      if (verbose_) {
        std::cout << "[ EventDisplay ] : Loaded '" << clustersCollName_
          << "' into memory as a EVE object." << std::endl;
      }
    } catch(const framework::exception::Exception& e) {
      std::cerr << "[ EventDisplay ] : Unable to draw an event object." << std::endl;
      std::cerr << "[" << e.name() << "] : " << e.message() << "\n"
        << "  at " << e.module() << ":" << e.line() << " in "
        << e.function() << "\nStack trace: " << std::endl
        << e.stackTrace();
    }

    if (verbose_) std::cout << "[ EventDisplay ] : Done loading event objects into EVE objects." << std::endl;
  
    manager_->AddElement(eventObjects_->getHitCollections());
    manager_->AddElement(eventObjects_->getRecoObjects());
    manager_->Redraw3D(kFALSE);
  
    if (verbose_) std::cout << "[ EventDisplay ] : Done redrawing." << std::endl;

  } else {
    std::cout << "[ EventDisplay ] : Already at last event in file."
              << std::endl;
    return;
  }
}

void EventDisplay::GetECALRecHitsCollInput() {
  ecalRecHitsCollName_ = getText(textBoxEcalRecHitsCollName_);
}

void EventDisplay::GetHCALRecHitsCollInput() {
  hcalRecHitsCollName_ = getText(textBoxHcalRecHitsCollName_);
}

void EventDisplay::GetTrackerHitsCollInput() {
  trackerHitsCollName_ = getText(textBoxTrackerHitsCollName_);
}

void EventDisplay::GetClustersCollInput() {
  clustersCollName_ = getText(textBoxClustersCollName_);
}

void EventDisplay::GetEcalSimParticlesCollInput() {
  ecalSimParticlesCollName_ = getText(textBoxEcalScorePlaneBranch_);
}

bool EventDisplay::SetSimThresh() {
  double thresh = atof(textBoxSimThresh_->GetText());
  if (thresh == 0 && std::string(textBoxSimThresh_->GetText()) != "0") {
    std::cout << "[ EventDisplay ] : Invalid sim energy threshold entered: \""
              << textBoxSimThresh_->GetText() << "\"" << std::endl;
    return false;
  } else if (verbose_) {
    std::cout << "[ EventDisplay ] : Setting SimParticle energy threshold to "
              << thresh << std::endl;
  }

  eventObjects_->SetSimThresh(thresh);

  manager_->RegisterRedraw3D();
  manager_->FullRedraw3D(kFALSE, kTRUE);

  return true;
}

void EventDisplay::ColorClusters() {
  eventObjects_->ColorClusters();

  manager_->RegisterRedraw3D();
  manager_->FullRedraw3D(kFALSE, kTRUE);
}

}  // namespace ldmx
