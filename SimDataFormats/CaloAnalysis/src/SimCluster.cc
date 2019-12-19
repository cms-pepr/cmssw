#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <numeric>

const unsigned int SimCluster::longLivedTag = 65536;

SimCluster::SimCluster() {
  // No operation

  impactPoint_ = math::XYZTLorentzVectorF(0, 0, 0, 0);
  impactMomentum_ = math::XYZTLorentzVectorF(0, 0, 0, 0);
  pdgId_ = 0;
}

SimCluster::SimCluster(const SimTrack &simtrk) {
  addG4Track(simtrk);
  event_ = simtrk.eventId();
  particleId_ = simtrk.trackId();

  theMomentum_.SetPxPyPzE(
      simtrk.momentum().px(), simtrk.momentum().py(), simtrk.momentum().pz(), simtrk.momentum().E());
}

SimCluster::SimCluster(EncodedEventId eventID, uint32_t particleID) {
  event_ = eventID;
  particleId_ = particleID;
}

SimCluster::SimCluster(const std::vector<SimTrack> &simtrks, int pdgId) {
  if (simtrks.size() > 0) {
    double sumPx = 0.;
    double sumPy = 0.;
    double sumPz = 0.;
    double sumE = 0.;

    for (const SimTrack &t : simtrks) {
      addG4Track(t);
      sumPx += t.momentum().px();
      sumPy += t.momentum().py();
      sumPz += t.momentum().pz();
      sumE += t.momentum().E();
    }

    theMomentum_.SetPxPyPzE(sumPx, sumPy, sumPz, sumE);

    // set event and particle ID (!= pdgID) from the first track for consistency
    event_ = simtrks[0].eventId();
    particleId_ = simtrks[0].trackId();
  }

  pdgId_ = pdgId;
}

SimCluster::~SimCluster() {}

std::ostream &operator<<(std::ostream &s, SimCluster const &tp) {
  s << "SC momentum(eta,phi,pt,m), q, ID, & Event #: " << tp.p4().Eta() << ", " << tp.p4().Phi() << ", " << tp.p4().Pt()
    << ", " << tp.p4().M() << " "
    << " " << tp.charge() << " " << tp.pdgId() << " " << tp.eventId().bunchCrossing() << "." << tp.eventId().event()
    << std::endl;

  s << "Impact position (eta,phi,z,t) " << tp.impactPoint().Eta() << " " << tp.impactPoint().Phi() << " "
    << tp.impactPoint().Z() << " " << tp.impactPoint().M() << std::endl;
  s << "Impact momentum (eta,phi,pt,M) " << tp.impactMomentum().Eta() << " " << tp.impactMomentum().Phi() << " "
    << tp.impactMomentum().Pt() << " " << tp.impactMomentum().M() << std::endl;

  for (SimCluster::genp_iterator hepT = tp.genParticle_begin(); hepT != tp.genParticle_end(); ++hepT) {
    s << " HepMC Track Momentum " << (*hepT)->momentum().rho() << std::endl;
  }

  for (SimCluster::g4t_iterator g4T = tp.g4Track_begin(); g4T != tp.g4Track_end(); ++g4T) {
    s << " Geant Track Momentum  " << g4T->momentum() << std::endl;
    s << " Geant Track ID & type " << g4T->trackId() << " " << g4T->type() << std::endl;
    if (g4T->type() != tp.pdgId()) {
      s << " Mismatch b/t SimCluster and Geant types" << std::endl;
    }
  }
  s << " # of cells = " << tp.hits_.size()
    << ", effective cells = " << std::accumulate(tp.fractions_.begin(), tp.fractions_.end(), 0.f) << std::endl;
  return s;
}
