#include "Tracking/dqm/TrackingRecoDQM.h"
#include "Tracking/Sim/TrackingUtils.h"

#include <iostream>
#include <algorithm>

namespace tracking::dqm {


void TrackingRecoDQM::configure(framework::config::Parameters &parameters) {
  
  trackCollection_ = parameters.getParameter<std::string>("track_collection","TaggerTracks");
  truthCollection_ = parameters.getParameter<std::string>("truth_collection","TaggerTruthTracks");
  title_           = parameters.getParameter<std::string>("title","tagger_trk_");
  trackProb_cut_   = parameters.getParameter<double>("trackProb_cut",0.5);


  pidmap[-321]  = PIDBins::kminus;
  pidmap[321]   = PIDBins::kplus;
  pidmap[-211]  = PIDBins::piminus;
  pidmap[211]   = PIDBins::piplus;
  pidmap[11]    = PIDBins::electron;
  pidmap[-11]   = PIDBins::positron;
  pidmap[2212]  = PIDBins::proton;
  pidmap[-2212] = PIDBins::antiproton;


  
}

void TrackingRecoDQM::analyze(const framework::Event& event) {
  
  if (!event.exists(trackCollection_)) return;
  auto tracks{event.getCollection<ldmx::Track>(trackCollection_)};

  // The truth track collection
  if (event.exists(truthCollection_)) {
    truthTrackCollection_ =
        std::make_shared<std::vector<ldmx::TruthTrack>>(event.getCollection<ldmx::TruthTrack>(truthCollection_));
    doTruthComparison = true;
  }

  // The scoring plane hits
  if (event.exists("EcalScoringPlaneHits")) {
    ecal_scoring_hits_ = std::make_shared<std::vector<ldmx::SimTrackerHit>>(event.getCollection<ldmx::SimTrackerHit>("EcalScoringPlaneHits"));
  }
  
  if (event.exists("TargetScoringPlaneHits")) {
    target_scoring_hits_ = std::make_shared<std::vector<ldmx::SimTrackerHit>>(event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits"));
  }

  if (doTruthComparison) {
    sortTracks(tracks,uniqueTracks, duplicateTracks,fakeTracks);
  }
  else {
    uniqueTracks = tracks;
  }
  
  TrackMonitoring(uniqueTracks,title_,true,true);

  
  // Fakes and duplicates
  TrackMonitoring(duplicateTracks, title_ + "dup_", false, false);
  TrackMonitoring(fakeTracks,title_ + "fake_", false, false);
  
    
  // Track Extrapolation to Ecal Monitoring
  
  TrackEcalScoringPlaneMonitoring(tracks);
  
  TrackTargetScoringPlaneMonitoring(tracks);


  // Technical Efficiency plots
  EfficiencyPlots(tracks,title_);
  
  // Clear the vectors
  uniqueTracks.clear();
  duplicateTracks.clear();
  fakeTracks.clear();
  
}


void TrackingRecoDQM::onProcessEnd() {

  //Produce the efficiency plots. (TODO::Switch to TEfficiency instead)
  
  
} 


void TrackingRecoDQM::EfficiencyPlots(const std::vector<ldmx::Track>& tracks,
                                      const std::string& title) {
  
  // Do all truth track plots - denominator
  
  for (auto& truth_trk : *(truthTrackCollection_)) {

    double truth_phi   = truth_trk.getPhi();
    double truth_d0   = truth_trk.getD0();
    double truth_z0   = truth_trk.getZ0();
    double truth_theta = truth_trk.getTheta();
    double truth_qop   = truth_trk.getQoP();
    double truth_p     = 1. / abs(truth_trk.getQoP());
    std::vector<double> truth_mom = truth_trk.getMomentum();

    //Polar angle
    //The momentum in the plane transverse wrt the beam axis
    double truth_pt_beam    = std::sqrt(truth_mom[1]*truth_mom[1] + truth_mom[2]*truth_mom[2]);
    
    double truth_beam_angle = std::atan2(truth_pt_beam, truth_mom[0]);
    
    histograms_.fill(title+"truth_d0",   truth_d0);
    histograms_.fill(title+"truth_z0",   truth_z0);
    histograms_.fill(title+"truth_phi",  truth_phi);
    histograms_.fill(title+"truth_theta",truth_theta);
    histograms_.fill(title+"truth_qop",  truth_qop);
    histograms_.fill(title+"truth_p",    truth_p);
    histograms_.fill(title+"truth_beam_angle", truth_beam_angle);
    
    if (pidmap.count(truth_trk.getPdgID()) != 0) {
      histograms_.fill(title+"truth_PID", pidmap[truth_trk.getPdgID()]);
    }
  } // loop on truth tracks

  
  for (auto& track : tracks) {
    
    //Match the tracks to truth
    ldmx::TruthTrack* truth_trk = nullptr;
      
    auto it = std::find_if(truthTrackCollection_->begin(),
                           truthTrackCollection_->end(),[&](const ldmx::TruthTrack& tt) {
                               return tt.getTrackID() == track.getTrackID();
                           });
    
    double trackTruthProb = track.getTruthProb();
    
    if (it != truthTrackCollection_->end() && trackTruthProb >= trackProb_cut_)
      truth_trk = &(*it);
    
    //Match not found
    if (!truth_trk)
      return;

    double truth_phi   = truth_trk->getPhi();
    double truth_d0    = truth_trk->getD0();
    double truth_z0    = truth_trk->getZ0();
    double truth_theta = truth_trk->getTheta();
    double truth_qop   = truth_trk->getQoP();
    double truth_p     = 1. / abs(truth_trk->getQoP());
    std::vector<double> truth_mom = truth_trk->getMomentum();

    //Polar angle
    //The momentum in the plane transverse wrt the beam axis
    double truth_pt_beam    = std::sqrt(truth_mom[1]*truth_mom[1] + truth_mom[2]*truth_mom[2]);
    
    double truth_beam_angle = std::atan2(truth_pt_beam, truth_mom[0]);
    
    
    //Fill reco plots for efficiencies - numerator. The quantities are truth
    
    histograms_.fill(title+"match_d0",         truth_d0);
    histograms_.fill(title+"match_z0",         truth_z0);
    histograms_.fill(title+"match_phi",        truth_phi);
    histograms_.fill(title+"match_theta",      truth_theta);
    histograms_.fill(title+"match_p",          truth_p);
    histograms_.fill(title+"match_qop",        truth_qop);
    histograms_.fill(title+"match_beam_angle", truth_beam_angle);
    
    // For some particles
    
    if (pidmap.count(truth_trk->getPdgID()) != 0) {
      histograms_.fill(title+"match_PID", pidmap[truth_trk->getPdgID()]);
      
    }
  } //Loop on tracks
  
}// Efficiency plots
  


void TrackingRecoDQM::TrackMonitoring(const std::vector<ldmx::Track>& tracks,
                                      const std::string title,
                                      const bool& doDetail,
                                      const bool& doTruth) {
  
  for (auto &track : tracks) {
    
    //Perigee track parameters
    
    double trk_d0     = track.getD0();
    double trk_z0     = track.getZ0();
    double trk_qop    = track.getQoP();
    double trk_theta  = track.getTheta();
    double trk_phi    = track.getPhi();
    double trk_p      = 1./abs(trk_qop);
    
    std::vector<double> trk_mom = track.getMomentum();
    
    //The transverse momentum in the bending plane
    double pt_bending = std::sqrt(trk_mom[0]*trk_mom[0] + trk_mom[1]*trk_mom[1]);
    
    //The momentum in the plane transverse wrt the beam axis
    double pt_beam    = std::sqrt(trk_mom[1]*trk_mom[1] + trk_mom[2]*trk_mom[2]);

    //Covariance matrix
    Acts::BoundSymMatrix cov = tracking::sim::utils::unpackCov(track.getPerigeeCov());

    double sigmad0    = sqrt(cov(Acts::BoundIndices::eBoundLoc0,Acts::BoundIndices::eBoundLoc0));
    double sigmaz0    = sqrt(cov(Acts::BoundIndices::eBoundLoc1,Acts::BoundIndices::eBoundLoc1));
    double sigmaphi   = sqrt(cov(Acts::BoundIndices::eBoundPhi ,Acts::BoundIndices::eBoundPhi));
    double sigmatheta = sqrt(cov(Acts::BoundIndices::eBoundTheta,Acts::BoundIndices::eBoundTheta));
    double sigmaqop   = sqrt(cov(Acts::BoundIndices::eBoundQOverP,Acts::BoundIndices::eBoundQOverP));
    double sigmap     = (1./trk_qop)*(1./trk_qop)*sigmaqop;

    histograms_.fill(title+"d0",trk_d0);
    histograms_.fill(title+"z0",trk_z0);
    histograms_.fill(title+"qop",trk_qop);
    histograms_.fill(title+"phi",trk_phi);
    histograms_.fill(title+"theta",trk_theta);

    if (doDetail) {
      
      histograms_.fill(title+"p",  std::abs(1./trk_qop));
      histograms_.fill(title+"px", trk_mom[0]);
      histograms_.fill(title+"py", trk_mom[1]);
      histograms_.fill(title+"pz", trk_mom[2]);
      
      histograms_.fill(title+"pt_bending", pt_bending);
      histograms_.fill(title+"pt_beam",    pt_beam);
      
      histograms_.fill(title+"nHits",    track.getNhits());
      histograms_.fill(title+"Chi2",     track.getChi2());
      histograms_.fill(title+"ndf", track.getNdf());
      histograms_.fill(title+"Chi2/ndf", track.getChi2()/track.getNdf());
      histograms_.fill(title+"nShared",  track.getNsharedHits());
    
    
      histograms_.fill(title+"d0_err",   sigmad0); 
      histograms_.fill(title+"z0_err",   sigmaz0); 
      histograms_.fill(title+"phi_err",  sigmaphi); 
      histograms_.fill(title+"theta_err",sigmatheta); 
      histograms_.fill(title+"qop_err",  sigmaqop); 
      histograms_.fill(title+"p_err",  sigmap);
    }
    
    if (doTruth) {
      
      //Match to the truth track
      ldmx::TruthTrack* truth_trk = nullptr;
      
      auto it = std::find_if(truthTrackCollection_->begin(),
                             truthTrackCollection_->end(),[&](const ldmx::TruthTrack& tt) {
                               return tt.getTrackID() == track.getTrackID();
                             });
      
      double trackTruthProb = track.getTruthProb();
      
      if (it != truthTrackCollection_->end() && trackTruthProb >= trackProb_cut_)
        truth_trk = &(*it);
      
      
      //Found matched track
      if (truth_trk) {
        
        double truth_d0    = truth_trk->getD0();
        double truth_z0    = truth_trk->getZ0();
        double truth_phi   = truth_trk->getPhi();
        double truth_theta = truth_trk->getTheta();
        double truth_qop   = truth_trk->getQoP();
        double truth_p     = 1. / abs(truth_trk->getQoP());
        
        //histograms_.fill(title+"truth_d0",   truth_d0);
        //histograms_.fill(title+"truth_z0",   truth_z0);
        //histograms_.fill(title+"truth_phi",  truth_phi);
        //histograms_.fill(title+"truth_theta",truth_theta);
        //histograms_.fill(title+"truth_qop",  truth_qop);
        //histograms_.fill(title+"truth_p",    truth_p);

        double res_d0    = trk_d0 - truth_d0;
        double res_z0    = trk_z0 - truth_z0;
        double res_phi   = trk_phi - truth_phi;
        double res_theta = trk_theta - truth_theta;
        double res_qop   = trk_qop - truth_qop;
        double res_p     = trk_p - truth_p;

        histograms_.fill(title+"res_d0",   res_d0);
        histograms_.fill(title+"res_z0",   res_z0);
        histograms_.fill(title+"res_phi",  res_phi);
        histograms_.fill(title+"res_theta",res_theta);
        histograms_.fill(title+"res_qop",  res_qop);
        histograms_.fill(title+"res_p",    res_p);
        
        double pull_d0    = res_d0    / sigmad0;
        double pull_z0    = res_z0    / sigmaz0;
        double pull_phi   = res_phi   / sigmaphi;
        double pull_theta = res_theta / sigmatheta;
        double pull_qop   = res_qop   / sigmaqop;
        double pull_p     = res_p     / sigmap;
        
        histograms_.fill(title+"pull_d0",   pull_d0);
        histograms_.fill(title+"pull_z0",   pull_z0);
        histograms_.fill(title+"pull_phi",  pull_phi);
        histograms_.fill(title+"pull_theta",pull_theta);
        histograms_.fill(title+"pull_qop",  pull_qop);
        histograms_.fill(title+"pull_p",    pull_p);
        
      } //found matched track
    }//do TruthComparison
  }//loop on tracks
  
  
}//Track Monitoring


void TrackingRecoDQM::TrackEcalScoringPlaneMonitoring(const std::vector<ldmx::Track>& tracks) {
  
  // Select the scoring plane hits that belong to the front of the ECAL
  // The front of the ecal is selected by checking that id_ & 0xfff is equal to 31.

  std::vector<ldmx::SimTrackerHit> sel_ecal_spHits;

  // TODO Use reference instead to avoid copy?

  for (auto sp_hit : *(ecal_scoring_hits_)) {
    
    if (sp_hit.getMomentum()[2] > 0 && ((sp_hit.getID() & 0xfff) == 31)) {
      
      sel_ecal_spHits.push_back(sp_hit);
    }
    
  }

  for (auto& track : tracks) {
    
    //Covariance matrix
    Acts::BoundSymMatrix cov = tracking::sim::utils::unpackCov(track.getPerigeeCov());

    double sigmad0    = sqrt(cov(Acts::BoundIndices::eBoundLoc0,Acts::BoundIndices::eBoundLoc0));
    double sigmaz0    = sqrt(cov(Acts::BoundIndices::eBoundLoc1,Acts::BoundIndices::eBoundLoc1));
    double sigmaphi   = sqrt(cov(Acts::BoundIndices::eBoundPhi ,Acts::BoundIndices::eBoundPhi));
    double sigmatheta = sqrt(cov(Acts::BoundIndices::eBoundTheta,Acts::BoundIndices::eBoundTheta));
    double sigmaqop   = sqrt(cov(Acts::BoundIndices::eBoundQOverP,Acts::BoundIndices::eBoundQOverP)); 

    double sigmaloc0 = sqrt(cov(0));
    double sigmaloc1 = sqrt(cov(7));
    
    double trk_qop    = track.getQoP();
    double trk_p      = 1./abs(trk_qop);
    
    for (auto& sp_hit : sel_ecal_spHits) {


      if (track.getTrackID() == sp_hit.getTrackID()) {
        
        // Get the Track extrapolated position
        // For the moment the trackState at the ECAL surface is the last in the vector
        // And the size of the vector is 2. So I hardcoded 1. 

        if (track.getTrackStates().size() < 2)
          continue;
                  
        
        ldmx::Track::TrackState ecalState = track.getTrackStates()[1];
        

        // Here is where you add the histograms
        // This gets the track local position 0
        // loc0 is along global X
        // loc1 is along global Y
        
        //Check that the track state is filled
        if (ecalState.params.size() < 5)
          continue;
        
        histograms_.fill(title_+"trk_ecal_loc0", ecalState.params[0]);
        histograms_.fill(title_+"trk_ecal_loc1", ecalState.params[1]);
        
        // This gets the hit global position Y
        
        auto scoring_plane_hit_pos = sp_hit.getPosition();
        histograms_.fill(title_+"ecal_sp_hit_X", scoring_plane_hit_pos[0]);
        histograms_.fill(title_+"ecal_sp_hit_Y", scoring_plane_hit_pos[1]);

        // TH1F  The difference(residual) between end_loc0 and sp_hit_X
        histograms_.fill(title_+"trk_ecal_loc0-sp_hit_X", ecalState.params[0]-scoring_plane_hit_pos[0]);
        histograms_.fill(title_+"trk_ecal_loc1-sp_hit_Y", ecalState.params[1]-scoring_plane_hit_pos[1]);

        // TH1F  The pulls of loc0 and loc1
        histograms_.fill(title_+"ecal_Pulls_of_loc0", (ecalState.params[0]-scoring_plane_hit_pos[0])/sigmaloc0);
        histograms_.fill(title_+"ecal_Pulls_of_loc1", (ecalState.params[1]-scoring_plane_hit_pos[1])/sigmaloc1);
        
        // TH2F  residual vs Nhits
        histograms_.fill(title_+"ecal_res_loc0-vs-N_hits", track.getNhits(), ecalState.params[0]-scoring_plane_hit_pos[0]);
        histograms_.fill(title_+"ecal_res_loc1-vs-N_hits", track.getNhits(), ecalState.params[1]-scoring_plane_hit_pos[1]);

        
        // TH2F  pulls vs Nhits
        histograms_.fill(title_+"ecal_pulls_loc0-vs-N_hits", track.getNhits(),(ecalState.params[0]-scoring_plane_hit_pos[0])/sigmaloc0);
        histograms_.fill(title_+"ecal_pulls_loc1-vs-N_hits", track.getNhits(), (ecalState.params[1]-scoring_plane_hit_pos[1])/sigmaloc1);
      
          
        // TH2F  residual vs trk_p
        histograms_.fill(title_+"ecal_res_loc0-vs-trk_p",  trk_p,  ecalState.params[0]-scoring_plane_hit_pos[0]);
        histograms_.fill(title_+"ecal_res_loc1-vs-trk_p",  trk_p,  ecalState.params[1]-scoring_plane_hit_pos[1]);

        // TH2F  pulls vs trk_p
        histograms_.fill(title_+"ecal_pulls_loc0-vs-trk_p",trk_p,  (ecalState.params[0]-scoring_plane_hit_pos[0])/sigmaloc0);
        histograms_.fill(title_+"ecal_pulls_loc1-vs-trk_p",trk_p,  (ecalState.params[1]-scoring_plane_hit_pos[1])/sigmaloc1);
      
      }
      
    } // loop on sp_hits
  }// loop on tracks
  
}

void TrackingRecoDQM::TrackTargetScoringPlaneMonitoring(const std::vector<ldmx::Track>& tracks) {
  
  // Select the scoring plane hits that belong to the front of the Target
  // The front of the target is selected by checking that id_ & 0xfff is equal to 31.

  std::vector<ldmx::SimTrackerHit> sel_target_spHits;

  // TODO Use reference instead to avoid copy?

  for (auto sp_hit : *(target_scoring_hits_)) {
    
    if (sp_hit.getMomentum()[2] > 0 ) {
      
      sel_target_spHits.push_back(sp_hit);
    }
    
  }

  for (auto& track : tracks) {
    
    //Covariance matrix
    Acts::BoundSymMatrix cov = tracking::sim::utils::unpackCov(track.getPerigeeCov());

    double sigmad0    = sqrt(cov(Acts::BoundIndices::eBoundLoc0,Acts::BoundIndices::eBoundLoc0));
    double sigmaz0    = sqrt(cov(Acts::BoundIndices::eBoundLoc1,Acts::BoundIndices::eBoundLoc1));
    double sigmaphi   = sqrt(cov(Acts::BoundIndices::eBoundPhi ,Acts::BoundIndices::eBoundPhi));
    double sigmatheta = sqrt(cov(Acts::BoundIndices::eBoundTheta,Acts::BoundIndices::eBoundTheta));
    double sigmaqop   = sqrt(cov(Acts::BoundIndices::eBoundQOverP,Acts::BoundIndices::eBoundQOverP)); 

    double sigmaloc0 = sqrt(cov(0));
    double sigmaloc1 = sqrt(cov(7));
    
    double trk_qop    = track.getQoP();
    double trk_p      = 1./abs(trk_qop);
    
    for (auto& sp_hit : sel_target_spHits) {


      if (track.getTrackID() == sp_hit.getTrackID()) {
        
        // Get the Track extrapolated position
        // For the moment the trackState at the Target surface is the last in the vector
        // And the size of the vector is 2. So I hardcoded 1. 

        if (track.getTrackStates().size() < 2)
          continue;
                  
        
        ldmx::Track::TrackState TargetState = track.getTrackStates()[0];
        

        // Here is where you add the histograms
        // This gets the track local position 0
        // loc0 is along global X
        // loc1 is along global Y
        
        //Check that the track state is filled
        if (TargetState.params.size() < 5)
          continue;
        
        histograms_.fill(title_+"trk_target_loc0", TargetState.params[0]);
        histograms_.fill(title_+"trk_target_loc1", TargetState.params[1]);
        
        // This gets the hit global position Y
        
        auto scoring_plane_hit_pos = sp_hit.getPosition();
        histograms_.fill(title_+"target_sp_hit_X", scoring_plane_hit_pos[0]);
        histograms_.fill(title_+"target_sp_hit_Y", scoring_plane_hit_pos[1]);

        // TH1F  The difference(residual) between end_loc0 and sp_hit_X
        histograms_.fill(title_+"trk_target_loc0-sp_hit_X", TargetState.params[0]-scoring_plane_hit_pos[0]);
        histograms_.fill(title_+"trk_target_loc1-sp_hit_Y", TargetState.params[1]-scoring_plane_hit_pos[1]);

        // TH1F  The pulls of loc0 and loc1
        histograms_.fill(title_+"target_Pulls_of_loc0", (TargetState.params[0]-scoring_plane_hit_pos[0])/sigmaloc0);
        histograms_.fill(title_+"target_Pulls_of_loc1", (TargetState.params[1]-scoring_plane_hit_pos[1])/sigmaloc1);
        
        // TH2F  residual vs Nhits
        histograms_.fill(title_+"target_res_loc0-vs-N_hits", track.getNhits(), TargetState.params[0]-scoring_plane_hit_pos[0]);
        histograms_.fill(title_+"target_res_loc1-vs-N_hits", track.getNhits(), TargetState.params[1]-scoring_plane_hit_pos[1]);

        
        // TH2F  pulls vs Nhits
        histograms_.fill(title_+"target_pulls_loc0-vs-N_hits", track.getNhits(), (TargetState.params[0]-scoring_plane_hit_pos[0])/sigmaloc0);
        histograms_.fill(title_+"target_pulls_loc1-vs-N_hits", track.getNhits(), (TargetState.params[1]-scoring_plane_hit_pos[1])/sigmaloc1);
      
        // TH2F  residual vs trk_p
        histograms_.fill(title_+"target_res_loc0-vs-trk_p",  trk_p,  TargetState.params[0]-scoring_plane_hit_pos[0]);
        histograms_.fill(title_+"target_res_loc1-vs-trk_p",  trk_p,  TargetState.params[1]-scoring_plane_hit_pos[1]);

        // TH2F  pulls vs trk_p
        histograms_.fill(title_+"target_pulls_loc0-vs-trk_p",trk_p,  (TargetState.params[0]-scoring_plane_hit_pos[0])/sigmaloc0);
        histograms_.fill(title_+"target_pulls_loc1-vs-trk_p",trk_p,  (TargetState.params[1]-scoring_plane_hit_pos[1])/sigmaloc1);
      
      }
      
    } // loop on sp_hits
  }// loop on tracks
  
}

void TrackingRecoDQM::sortTracks(const std::vector<ldmx::Track>& tracks,
                                 std::vector<ldmx::Track>& uniqueTracks,
                                 std::vector<ldmx::Track>& duplicateTracks,
                                 std::vector<ldmx::Track>& fakeTracks) {
  
  // Create a copy of the const vector so we can sort it
  std::vector<ldmx::Track> sortedTracks = tracks;
  
  // Sort the vector of Track objects based on their trackID member
  std::sort(sortedTracks.begin(),
            sortedTracks.end(),
            [](ldmx::Track& t1,ldmx::Track& t2){
              return t1.getTrackID() < t2.getTrackID();
            }
            );
  
  // Loop over the sorted vector of Track objects
  for (size_t i = 0; i < sortedTracks.size(); i++) {
    if (sortedTracks[i].getTruthProb() < trackProb_cut_) 
      fakeTracks.push_back(sortedTracks[i]);
    else { //not a fake track
      // If this is the first Track object with this trackID, add it to the uniqueTracks vector directly
      if (uniqueTracks.size() == 0 || sortedTracks[i].getTrackID() != sortedTracks[i-1].getTrackID()) {
        uniqueTracks.push_back(sortedTracks[i]);
      }
      // Otherwise, add it to the duplicateTracks vector if its truthProb is lower than the existing Track object
      // Otherwise, if the truthProbability is higher than the track stored in uniqueTracks, put it in uniqueTracks and move the uniqueTracks.back to duplicateTracks.
      else if (sortedTracks[i].getTruthProb() > uniqueTracks.back().getTruthProb()) {
        duplicateTracks.push_back(uniqueTracks.back());
        uniqueTracks.back() = sortedTracks[i];
      }
      // Otherwise, add it to the duplicateTracks vector
      else {
        duplicateTracks.push_back(sortedTracks[i]);
      }
    } //a real track
  } //loop on sorted tracks
  // The total number of elements in the uniqueTracks and duplicateTracks vectors should be equal to the number of elements in the original tracks vector
  if (uniqueTracks.size() + duplicateTracks.size() + fakeTracks.size() != tracks.size()) {
    std::cerr << "Error: unique and duplicate tracks vectors do not add up to original tracks vector" << std::endl;
    return;
  }
  
  if (debug_) {
    // Iterate through the uniqueTracks vector and duplicateTracks vector
    std::cout << "Unique tracks:" << std::endl;
    for (const ldmx::Track& track : uniqueTracks) {
      std::cout << "Track ID: " << track.getTrackID() << ", Truth Prob: " << track.getTruthProb() << std::endl;
    }
    std::cout << "Duplicate tracks:" << std::endl;
    for (const ldmx::Track& track : duplicateTracks) {
      std::cout << "Track ID: " << track.getTrackID() << ", Truth Prob: " << track.getTruthProb() << std::endl;
    }
    std::cout << "Fake tracks:" << std::endl;
    for (const ldmx::Track& track : fakeTracks) {
      std::cout << "Track ID: " << track.getTrackID() << ", Truth Prob: " << track.getTruthProb() << std::endl; 
    }
  }
}
} //tracking::dqm

DECLARE_ANALYZER_NS(tracking::dqm, TrackingRecoDQM)
