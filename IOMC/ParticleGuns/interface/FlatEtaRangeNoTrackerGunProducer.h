/*
 * Particle gun that can be positioned in space given ranges in rho, z and phi.
 */

#ifndef IOMC_PARTICLEGUN_FlatEtaRangeNoTrackerGunProducer_H
#define IOMC_PARTICLEGUN_FlatEtaRangeNoTrackerGunProducer_H

#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Run.h"

#include "HepMC/GenEvent.h"
#include "HepPDT/ParticleDataTable.hh"

#include "RecoHGCal/GraphReco/interface/HGCalParticlePropagator.h"


namespace edm {

  class FlatEtaRangeNoTrackerGunProducer : public one::EDProducer<one::WatchRuns, EndRunProducer> {
  public:
    static void fillDescriptions(edm::ConfigurationDescriptions&);

    explicit FlatEtaRangeNoTrackerGunProducer(const ParameterSet&);
    ~FlatEtaRangeNoTrackerGunProducer() override;
    void beginRun(const edm::Run&, const edm::EventSetup&) override;
    void endRun(const edm::Run&, const edm::EventSetup&) override;
    void endRunProduce(edm::Run&, const edm::EventSetup&) override;

  private:
    void produce(Event&, const EventSetup&) override;

  protected:
    // ids of particles to shoot
    std::vector<int> particleIDs_;

    // the number of particles to shoot
    int nParticles_;

    // flag that denotes that exactly the particles defined by fPartIds should be shot, with that order and quantity
    bool exactShoot_;

    // flag that denotes whether a random number of particles in the range [1, fNParticles] is shot
    bool randomShoot_;

    // energy range
    double eMin_;
    double eMax_;

    // eta range
    double etaMin_;
    double etaMax_;

    // phi range
    double phiMin_;
    double phiMax_;

    // debug flag
    bool debug_;

    // pointer to the current event
    HepMC::GenEvent* genEvent_;

    // pdg table
    ESHandle<HepPDT::ParticleDataTable> pdgTable_;

    //propagator
    HGCalParticlePropagator prop_;

    //smear vertex
    double timeSmear_;

    //smear momentum direction a bit, to have a less stringent BS constraint
    double momSmear_;
    double minDistDR_;

  };

}  // namespace edm

#endif  // IOMC_PARTICLEGUN_FlatEtaRangeNoTrackerGunProducer_H
