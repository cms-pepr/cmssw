/*
 * Particle gun that can be positioned in space given ranges in rho, z and phi.
 */

#ifndef IOMC_PARTICLEGUN_FlatEtaRangeGunProducer_H
#define IOMC_PARTICLEGUN_FlatEtaRangeGunProducer_H

#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Run.h"

#include "IOMC/ParticleGuns/interface/FlatRandomEGunProducer.h"
#include "HepMC/GenEvent.h"
#include "HepPDT/ParticleDataTable.hh"

namespace edm {

  class FlatEtaRangeGunProducer : public FlatRandomEGunProducer {
  public:
    FlatEtaRangeGunProducer(const ParameterSet&);

  private:
    void produce(Event&, const EventSetup&) override;

  protected:
    // the number of particles to shoot
    int nParticles_;

    // flag that denotes that exactly the particles defined by fPartIds should be shot, with that order and quantity
    bool exactShoot_;

    // flag that denotes whether a random number of particles in the range [1, fNParticles] is shot
    bool randomShoot_;

    const std::vector<double> discreteEnergies_;

    double minDr_;

    // debug flag
    bool debug_;
  };

}  // namespace edm

#endif  // IOMC_PARTICLEGUN_FlatEtaRangeGunProducer_H
