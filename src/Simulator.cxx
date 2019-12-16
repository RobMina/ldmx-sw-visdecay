/**
 * @file Simulator.cxx
 * @brief Producer that runs Geant4 simulation inside of ldmx-app
 * @author Tom Eichlersmith, University of Minnesota
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "SimApplication/Simulator.h"

#include "SimApplication/DetectorConstruction.h"
#include "SimApplication/RunManager.h"

#include "G4CascadeParameters.hh"
#include "G4GDMLParser.hh"
#include "G4GDMLMessenger.hh"

namespace ldmx {

    Simulator::Simulator(const std::string& name, ldmx::Process& process) : Producer( name , process ) {

        runManager_ = std::make_unique<RunManager>();

        // Instantiate the GDML parser and corresponding messenger
        parser_ = std::make_unique<G4GDMLParser>();
        gdmlMessenger_ = std::make_unique<G4GDMLMessenger>(parser_.get()); 

        // Instantiate the class so cascade parameters can be set.
        G4CascadeParameters::Instance();  

        runManager_->SetUserInitialization(new DetectorConstruction( parser_.get() ) );

        runManager_->SetRandomNumberStore( true );
        
        /*
        new SimApplicationMessenger();
        persistencyManager_ = new RootPersistencyManager();
        persistencyManager_->SetVerboseLevel( 3 );
        */
    }

    Simulator::~Simulator() {
    }


    void Simulator::configure(const ldmx::ParameterSet& ps) {
       
        return;
    }

    void Simulator::produce(ldmx::Event& event) {

        runManager_->ProcessOneEvent( iEvent_++ );
        runManager_->GetCurrentEvent()->Print();
        runManager_->TerminateOneEvent();
    
        /*
        persistencyManager_->buildEvent( runManager_->GetCurrentEvent() , &event );
        persistencyManager_->writeHitsCollections( runManager_->GetCurrentEvent() , &event );
        */
        return;
    }
    
    void Simulator::onProcessStart() {
        
        // Parser the detector
        // TODO: This should be configurable via a parameter. 
        uiManager_->ApplyCommand( "/persistency/gdml/read detector.gdml" );
        uiManager_->ApplyCommand( "/run/initialize" );

        //runManager_->ConstructScoringWorlds();
        runManager_->RunInitialization();
        runManager_->InitializeEventLoop( 1 );

        return;
    }

    void Simulator::onProcessEnd() {

        runManager_->TerminateEventLoop();
        runManager_->RunTermination();
        return;
    }

}

DECLARE_PRODUCER_NS(ldmx, Simulator)
