#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/ForwardDetId/interface/ForwardSubdetector.h"
#include "DataFormats/ForwardDetId/interface/HFNoseDetId.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"

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
  pdgId_ = simtrk.type();

  impactMomentum_ = simtrk.getMomentumAtBoundary();
  impactPoint_ = simtrk.getPositionAtBoundary();
}

SimCluster::SimCluster(EncodedEventId eventID, uint32_t particleID) {
  event_ = eventID;
  particleId_ = particleID;
}

void SimCluster::addG4Track(const SimTrack &t) {
      g4Tracks_.push_back(t);
      if(g4Tracks_.size() == 1)
          pdgId_ = t.type();
      else
          pdgId_ = 0; //undefined just from simtracks
  }

math::XYZTLorentzVectorF SimCluster::impactMomentumMuOnly() const {
	math::XYZTLorentzVectorF mom;
	for (auto& t : g4Tracks_) {
		if (std::abs(t.type()) == 13)
			mom += t.getMomentumAtBoundary();
	}
	return mom;
}

math::XYZTLorentzVectorF SimCluster::impactMomentumNoMu() const {
	math::XYZTLorentzVectorF mom;
	for (auto& t : g4Tracks_) {
		if (std::abs(t.type()) != 13)
			mom += t.getMomentumAtBoundary();
	}
	return mom;
}

SimCluster SimCluster::operator+(const SimCluster& toAdd) {
	SimCluster orig = *this;
	return orig += toAdd;
}

SimCluster& SimCluster::operator+=(const SimCluster& toMerge) {
    for (auto& track : toMerge.g4Tracks())
        this->addG4Track(track);

    for (auto& hit_and_e : toMerge.hits_and_fractions())
        this->addDuplicateRecHitAndFraction(hit_and_e.first, hit_and_e.second);

    auto& mergeMom = toMerge.impactMomentum();
    float sumE = impactMomentum_.energy() + mergeMom.energy();
    impactPoint_ = (impactPoint_*impactMomentum_.energy() + mergeMom.energy()*toMerge.impactPoint())/sumE;

    this->impactMomentum_ += mergeMom;
    pdgId_=0;//undefined
    return *this;
}

bool SimCluster::allHitsHGCAL() const {
    for (const auto& hitsAndEnergies : hits_and_fractions()) {
        const DetId id = hitsAndEnergies.first;
        bool forward = id.det() == DetId::HGCalEE
                || id.det() == DetId::HGCalHSi
                || id.det() == DetId::HGCalHSc
                || (id.det() == DetId::Forward && id.subdetId() != static_cast<int>(HFNose))
                || (id.det() == DetId::Hcal && id.subdetId() == HcalSubdetector::HcalEndcap);

        if(!forward)
            return false;
    }
    return true;
}

bool SimCluster::hasHGCALHit() const {
    for (const auto& hitsAndEnergies : hits_and_fractions()) {
        const DetId id = hitsAndEnergies.first;
        bool forward = id.det() == DetId::HGCalEE
                || id.det() == DetId::HGCalHSi
                || id.det() == DetId::HGCalHSc
                || (id.det() == DetId::Forward && id.subdetId() != static_cast<int>(HFNose))
                || (id.det() == DetId::Hcal && id.subdetId() == HcalSubdetector::HcalEndcap);

        if(forward)
            return true;
    }
    return false;
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
