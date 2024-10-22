#include "SimDataFormats/PFAnalysis/interface/PFTruthParticle.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include <FWCore/MessageLogger/interface/MessageLogger.h>

PFTruthParticle::PFTruthParticle() {
  // No operation
}

PFTruthParticle::~PFTruthParticle() {}

PFTruthParticle::PFTruthParticle(const TrackingParticleRefVector& trackingParticles, const SimClusterRefVector& simClusters) {
    setTrackingParticles(trackingParticles);
    setSimClusters(simClusters);
}

void PFTruthParticle::setTrackingParticles(const TrackingParticleRefVector& refs) {
    for (auto& tp : refs) {
      addTrackingParticle(tp); 
    }
}

void PFTruthParticle::setSimClusters(const SimClusterRefVector& refs) { 
    for (auto& sc : refs) {
      addSimCluster(sc); 
    }
}

void PFTruthParticle::addSimCluster(const SimClusterRef sc) { 
    simClusters_.push_back(sc); 
    for (auto& track : sc->g4Tracks())
        g4Tracks_.push_back(track);
}

void PFTruthParticle::addTrackingParticle(const TrackingParticleRef tp) { 
    trackingParticles_.push_back(tp); 
    for (auto& track : tp->g4Tracks())
        g4Tracks_.push_back(track);
}

void PFTruthParticle::setPdgId(int pdgId) { pdgId_ = pdgId; }

void PFTruthParticle::setCharge(int charge) { charge_ = charge; }

void PFTruthParticle::setP4(LorentzVectorF p4) { p4_ = p4; }

void PFTruthParticle::setVertex(LorentzVectorF vertex) { vertex_ = vertex; }

void PFTruthParticle::addG4Track(const SimTrack& t) { g4Tracks_.push_back(t); }


PFTruthParticle::LorentzVectorF PFTruthParticle::calo_xyzt() const {
    LorentzVectorF pos;
    double tote=0;
    for (const auto& sc: simClusters_) {
        pos += sc->impactPoint() * sc->energy();
        tote += sc->energy();
    }
    return pos / tote;
}
