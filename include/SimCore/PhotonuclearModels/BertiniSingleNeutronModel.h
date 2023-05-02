#ifndef SIMCORE_BERTINI_SINGLE_NEUTRON_MODEL_H
#define SIMCORE_BERTINI_SINGLE_NEUTRON_MODEL_H
#include <G4CrossSectionDataSetRegistry.hh>
#include <G4Gamma.hh>
#include <G4HadProjectile.hh>
#include <G4HadronInelasticProcess.hh>
#include <G4Nucleus.hh>
#include <G4ProcessManager.hh>

#include "Framework/Configure/Parameters.h"
#include "SimCore/PhotonuclearModel.h"
#include "SimCore/PhotonuclearModels/BertiniEventTopologyProcess.h"
namespace simcore {
class BertiniSingleNeutronProcess : public BertiniEventTopologyProcess {
 public:
  BertiniSingleNeutronProcess(double threshold, int Zmin, double Emin,
                              bool count_light_ions)
      : BertiniEventTopologyProcess{count_light_ions},
        threshold_{threshold},
        Zmin_{Zmin},
        Emin_{Emin} {}
  virtual ~BertiniSingleNeutronProcess() = default;
  bool acceptProjectile(const G4HadProjectile& projectile) const override {
    return projectile.GetKineticEnergy() >= Emin_;
  }
  bool acceptTarget(const G4Nucleus& targetNucleus) const override {
    return targetNucleus.GetZ_asInt() >= Zmin_;
  }
  bool acceptEvent() const override;

 private:
  double threshold_;
  int Zmin_;
  double Emin_;
};

class BertiniSingleNeutronModel : public PhotonuclearModel {
 public:
  BertiniSingleNeutronModel(const std::string& name,
                            const framework::config::Parameters& parameters)
      : PhotonuclearModel{name, parameters},
        threshold_{parameters.getParameter<double>("hard_particle_threshold")},
        Zmin_{parameters.getParameter<int>("zmin")},
        Emin_{parameters.getParameter<double>("emin")},
        count_light_ions_{parameters.getParameter<bool>("count_light_ions")} {}
  virtual ~BertiniSingleNeutronModel() = default;
  void ConstructGammaProcess(G4ProcessManager* processManager) override;

 private:
  double threshold_;
  int Zmin_;
  double Emin_;
  bool count_light_ions_;
};

}  // namespace simcore
#endif /* SIMCORE_BERTINI_SINGLE_NEUTRON_MODEL_H */
