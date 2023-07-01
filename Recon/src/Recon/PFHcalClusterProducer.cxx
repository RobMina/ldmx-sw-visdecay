#include "Recon/PFHcalClusterProducer.h"

#include "Hcal/Event/HcalHit.h"
#include "Hcal/Event/HcalCluster.h"
#include "TGraph.h"
#include "TFitResult.h"

namespace recon {

void fillClusterInfo(ldmx::HcalCluster &cl, std::vector<ldmx::HcalHit> hits){
  float e(0),x(0),y(0),z(0),xx(0),yy(0),zz(0),n(0);
  float w = 1; // weight
  float sumw = 0;
  float minEnergy=0; // min hit energy to consider
  std::vector<float> xvals{};
  std::vector<float> yvals{};
  std::vector<float> zvals{};
  for(const auto &h : hits){
    //if (h.getEnergy() < minEnergy) continue;
    // w = log( h.getEnergy() - log(minEnergy) );
    e += h.getEnergy();
    x += w * h.getXPos();
    y += w * h.getYPos();
    z += w * h.getZPos();
    xx += w * h.getXPos() * h.getXPos();
    yy += w * h.getYPos() * h.getYPos();
    zz += w * h.getZPos() * h.getZPos();
    n += 1;
    sumw += w;
    xvals.push_back(x);
    yvals.push_back(y);
    zvals.push_back(z);
  }
  x /= sumw; // now is <x>
  y /= sumw;
  z /= sumw;
  xx /= sumw; // now is <x^2>
  yy /= sumw;
  zz /= sumw;
  xx = sqrt(xx - x * x); //now is sqrt(<x^2>-<x>^2)
  yy = sqrt(yy - y * y); 
  zz = sqrt(zz - z * z);
  cl.setEnergy(e);
  cl.setNHits(n);
  cl.setCentroidXYZ(x,y,z);
  cl.setRMSXYZ(xx,yy,zz);

  if (xvals.size()>2){
    for(int i=0; i<xvals.size();i++){ // mean subtract
      xvals[i] = xvals[i] - x;
      yvals[i] = yvals[i] - y;
      zvals[i] = zvals[i] - z;
    }
    TGraph gxz(zvals.size(), zvals.data(), xvals.data());
    auto r_xz = gxz.Fit("pol1","SQ"); // p0 + x*p1
    cl.setDXDZ( r_xz->Value(1) );
    cl.setEDXDZ( r_xz->ParError(1) );
    TGraph gyz(zvals.size(), zvals.data(), yvals.data());
    auto r_yz = gyz.Fit("pol1","SQ"); // p0 + x*p1
    cl.setDYDZ( r_yz->Value(1) );
    cl.setEDYDZ( r_yz->ParError(1) );
  }
  return;
  //pfClusters.push_back(cl);
}

void PFHcalClusterProducer::configure(framework::config::Parameters& ps) {
  hitCollName_ = ps.getParameter<std::string>("hitCollName");
  clusterCollName_ = ps.getParameter<std::string>("clusterCollName"); 
  singleCluster_ = ps.getParameter<bool>("doSingleCluster");
}

void PFHcalClusterProducer::produce(framework::Event& event) {

  if (!event.exists(hitCollName_)) return;
  const auto hcalRecHits = event.getCollection<ldmx::HcalHit>(hitCollName_);
  //std::cout << "found " << hcalRecHits.size() << " hits" << std::endl;

  std::vector<ldmx::HcalCluster> pfClusters;
  if(!singleCluster_){ 
    
  } else {
    ldmx::HcalCluster cl;
    fillClusterInfo(cl,hcalRecHits);
    pfClusters.push_back(cl);
  }
  event.add(clusterCollName_, pfClusters);
}

void PFHcalClusterProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void PFHcalClusterProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void PFHcalClusterProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void PFHcalClusterProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, PFHcalClusterProducer);
