/*
 * CMSSW module to create the trees used for the HH2bbtautau analysis.
 * Disclainer: This prototype is not meant for creating an official PR. Major refactoring ahead.
 */

#include <memory>

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ProducerBase.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/ForwardDetId/interface/HGCalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalTestNumbering.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"

#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloHit/interface/PCaloHit.h"
#include "SimDataFormats/CaloTest/interface/HGCalTestNumbering.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/Vertex/interface/SimVertex.h"

#include "SimGeneral/MixingModule/interface/DigiAccumulatorMixMod.h"
#include "SimGeneral/MixingModule/interface/DigiAccumulatorMixModFactory.h"
#include "SimGeneral/MixingModule/interface/PileUpEventPrincipal.h"
#include "SimGeneral/TrackingAnalysis/interface/EncodedTruthId.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"
#include "Geometry/HcalCommonData/interface/HcalHitRelabeller.h"
#include "Geometry/HcalTowerAlgo/interface/HcalGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"

#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include "HGCSimTruth/HGCSimTruth/interface/SimClusterTools.h"

#include "TMath.h"
#include "TCanvas.h"
#include "TH1F.h"

float getShowerRadius(float f, float r1, float f1) {
  float a = r1 / (sqrt(2) * TMath::ErfInverse(f1 - 0.00001));
  return a * sqrt(2) * TMath::ErfInverse(f - 0.00001);
}

float getShowerRadius(float f, float r1, float f1, float r2, float f2) {
  float d1 = getShowerRadius(f, r1, f1);
  float d2 = getShowerRadius(f, r2, f2);
  return (d1 + d2) / 2.f;
}

float getShowerRadius(float f, float r1, float f1, float r2, float f2, float r3, float f3) {
  float d1 = getShowerRadius(f, r1, f1);
  float d2 = getShowerRadius(f, r2, f2);
  float d3 = getShowerRadius(f, r3, f3);
  return (d1 + d2 + d3) / 3.f;
}

int commonAncestorPdgId(const std::vector<std::pair<int, int>>& v1, const std::vector<std::pair<int, int>>& v2) {
  // integer pairs represent the vertex id in the graph and the pdg id of the corresponding particle
  // to get the pdg id of the first common ancestor, walk through v1 and check if any vertex id is
  // contained in v2, and if so, return the pdg id (should be the same for the matching elements)
  for (const auto& p1 : v1) {
    for (const auto& p2 : v2) {
      if (p1.first == p2.first) {
        return p1.second;
      }
    }
  }
  return 0;
}

struct ChainIndex {
  ChainIndex(int iSC, ChainIndex* prev, ChainIndex* next) : iSC(iSC), prev(prev), next(next) {}

  ChainIndex(int iSC) : ChainIndex(iSC, nullptr, nullptr) {}

  ChainIndex(const ChainIndex& o) : ChainIndex(o.iSC, o.prev, o.next) {}

  bool operator==(const ChainIndex& o) const { return o.iSC == iSC; }

  int iSC;
  ChainIndex* prev;
  ChainIndex* next;
};

class HGCTruthProducer : public edm::stream::EDProducer<> {
public:
  static void fillDescriptions(edm::ConfigurationDescriptions&);

  explicit HGCTruthProducer(const edm::ParameterSet&);
  ~HGCTruthProducer();

  virtual void beginStream(edm::StreamID) override;
  virtual void endStream() override;
  virtual void produce(edm::Event&, const edm::EventSetup&) override;

  void determineSimClusterGroups(const SimClusterCollection&,
                                 const std::vector<SimClusterHistory>&,
                                 std::vector<std::vector<int>>&,
                                 std::unique_ptr<std::vector<float>>&,
                                 std::unique_ptr<std::vector<math::XYZTLorentzVectorD>>&) const;

  bool checkSimClusterMerging(int,
                              int,
                              const SimClusterCollection&,
                              const std::vector<SimClusterHistory>&,
                              const std::unique_ptr<std::vector<float>>&,
                              const std::unique_ptr<std::vector<math::XYZTLorentzVectorD>>&) const;

  void mergeSimClusters(const SimClusterCollection&,
                        const std::vector<SimClusterHistory>&,
                        const std::vector<std::vector<int>>&,
                        std::unique_ptr<SimClusterCollection>&,
                        const std::vector<const HGCRecHit*>&,
                        const std::unordered_map<DetId, size_t>&) const;

private:
  float showerContainment_;
  float minEta_;
  float maxEta_;
  int minHitsRadius_;
  float maxDeltaEtaGroup_;
  bool verbose_;

  edm::EDGetTokenT<std::vector<CaloParticle>> caloParticleToken_;
  edm::EDGetTokenT<std::vector<SimCluster>> simClusterToken_;
  edm::EDGetTokenT<std::vector<SimClusterHistory>> simClusterHistoryToken_;
  std::vector<edm::EDGetTokenT<HGCRecHitCollection>> recHitTokens_;

  hgcal::RecHitTools recHitTools_;
};

void HGCTruthProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<double>("minEta", 1.52);
  desc.add<double>("maxEta", 3.00);
  desc.add<int>("minHitsRadius", 5);
  desc.add<double>("maxDeltaEtaGroup", 0.15);
  desc.addUntracked<bool>("verbose", false);
  desc.add<edm::InputTag>("caloParticleCollection", edm::InputTag("mix", "MergedCaloTruth"));
  desc.add<edm::InputTag>("simClusterCollection", edm::InputTag("mix", "MergedCaloTruth"));
  desc.add<edm::InputTag>("simClusterHistoryCollection", edm::InputTag("mix", "MergedCaloTruth"));
  desc.add<std::vector<edm::InputTag>>("recHitCollections",
                                       {
                                           edm::InputTag("HGCalRecHit", "HGCEERecHits"),
                                           edm::InputTag("HGCalRecHit", "HGCHEFRecHits"),
                                           edm::InputTag("HGCalRecHit", "HGCHEBRecHits"),
                                       });

  descriptions.add("hgcTruthProducer", desc);
}

HGCTruthProducer::HGCTruthProducer(const edm::ParameterSet& params)
    : showerContainment_(0.6827),
      minEta_(params.getParameter<double>("minEta")),
      maxEta_(params.getParameter<double>("maxEta")),
      minHitsRadius_(params.getParameter<int>("minHitsRadius")),
      maxDeltaEtaGroup_(params.getParameter<double>("maxDeltaEtaGroup")),
      verbose_(params.getUntrackedParameter<bool>("verbose")),
      caloParticleToken_(
          consumes<std::vector<CaloParticle>>(params.getParameter<edm::InputTag>("caloParticleCollection"))),
      simClusterToken_(consumes<std::vector<SimCluster>>(params.getParameter<edm::InputTag>("simClusterCollection"))),
      simClusterHistoryToken_(
          consumes<std::vector<SimClusterHistory>>(params.getParameter<edm::InputTag>("simClusterHistoryCollection"))) {
  if (verbose_) {
    std::cout << "running TreeWriter in verbose mode" << std::endl;
  }

  // setup recHitTokens
  for (auto& recHitCollection : params.getParameter<std::vector<edm::InputTag>>("recHitCollections")) {
    recHitTokens_.push_back(consumes<HGCRecHitCollection>(recHitCollection));
  }

  // define products
  produces<std::vector<SimCluster>>();                // SimClusters
  produces<std::vector<float>>();                     // radii
  produces<std::vector<math::XYZTLorentzVectorD>>();  // rechit-base four-momenta of merged clusters
}

HGCTruthProducer::~HGCTruthProducer() {}

void HGCTruthProducer::beginStream(edm::StreamID) {}

void HGCTruthProducer::endStream() {}

void HGCTruthProducer::produce(edm::Event& event, const edm::EventSetup& setup) {
  recHitTools_.getEventSetup(setup);

  std::cout << "HGCTruthProducer::produce" << std::endl;

  // create unique pointers for output products
  std::unique_ptr<std::vector<SimCluster>> mergedSimClusters = std::make_unique<std::vector<SimCluster>>();
  std::unique_ptr<std::vector<float>> radii = std::make_unique<std::vector<float>>();
  std::unique_ptr<std::vector<math::XYZTLorentzVectorD>> vectors =
      std::make_unique<std::vector<math::XYZTLorentzVectorD>>();

  // read original SimClusters
  edm::Handle<std::vector<SimCluster>> simClusterHandle;
  event.getByToken(simClusterToken_, simClusterHandle);
  SimClusterCollection simClusters(*simClusterHandle);

  std::vector<const HGCRecHit*> allrechits;
  std::unordered_map<DetId, size_t> detid_to_rh_index;
  size_t rhindex = 0;
  for (auto& token : recHitTokens_) {
    for (const auto& rh : event.get(token)) {
      detid_to_rh_index[rh.detid()] = rhindex;
      rhindex++;
      allrechits.push_back(&rh);
    }
  }

  // read SimClusterHistory
  edm::Handle<std::vector<SimClusterHistory>> simClusterHistoryHandle;
  event.getByToken(simClusterHistoryToken_, simClusterHistoryHandle);
  std::vector<SimClusterHistory> simClusterHistory(*simClusterHistoryHandle);

  // determine clustering groups
  std::vector<std::vector<int>> groups;
  determineSimClusterGroups(simClusters, simClusterHistory, groups, radii, vectors);

  // do the actual merging
  mergeSimClusters(simClusters, simClusterHistory, groups, mergedSimClusters, allrechits, detid_to_rh_index);

  std::cout << "initial simclusters " << simClusters.size() << " merged: " << mergedSimClusters->size()
            << std::endl;  //DEBUG Jan

  // save outputs
  event.put(std::move(mergedSimClusters));
  event.put(std::move(radii));
  event.put(std::move(vectors));
}

void HGCTruthProducer::determineSimClusterGroups(const SimClusterCollection& simClusters,
                                                 const std::vector<SimClusterHistory>& simClusterHistory,
                                                 std::vector<std::vector<int>>& groups,
                                                 std::unique_ptr<std::vector<float>>& radii,
                                                 std::unique_ptr<std::vector<math::XYZTLorentzVectorD>>& vectors) const {
  // Note: In many places, the implementation is based on indices for performance purposes. To
  // simplify the distinction, they are referred to as {i,j}Name, such as iSC for sim clusters or iH
  // for hits. Sometimes, lists (vectors) of these indices are used. The index to those values are
  // (e.g.) iiSC or iiH.
  int nSimClusters = (int)simClusters.size();

  // clear the radii vector and reserve space
  radii->clear();
  radii->resize(nSimClusters);
  vectors->clear();
  vectors->resize(nSimClusters);

  // for the moment, only SimClusters in the HGCal are considered
  // clusters in the HCal barrel are copied unchanged
  std::vector<int> scIndices;
  for (int iSC = 0; iSC < nSimClusters; iSC++) {
    auto absEta = fabs(simClusters[iSC].eta());
    if (absEta >= minEta_ && absEta <= maxEta_) {
      // cluster located in HGCal, store the index for treatment downstream
      scIndices.push_back(iSC);
    } else {
      // cluster located in HCal, only store a non-physical radius
      (*radii)[iSC] = -1.;
    }
  }

  // the number of sim clusters in the HGCal
  int nHGCalSimClusters = (int)scIndices.size();

  // determine simCluster radii
  for (const int& iSC : scIndices) {
    // create four-vectors for each hit and the summed shower vector
    auto atthispointthis_is_hitsandfractions_hitsAndEnergies = simClusters[iSC].hits_and_fractions();
    int nHits = (int)atthispointthis_is_hitsandfractions_hitsAndEnergies.size();
    math::XYZTLorentzVectorD showerVector;
    std::vector<math::XYZTLorentzVectorD> hitVectors;
    hitVectors.resize(atthispointthis_is_hitsandfractions_hitsAndEnergies.size());
    std::vector<int> hitVectorIndices;
    hitVectorIndices.resize(atthispointthis_is_hitsandfractions_hitsAndEnergies.size());
    for (int iH = 0; iH < nHits; iH++) {
      // get the hit position and create the four-vector, assuming no mass
      GlobalPoint position = recHitTools_.getPosition(atthispointthis_is_hitsandfractions_hitsAndEnergies[iH].first);
      float energy = atthispointthis_is_hitsandfractions_hitsAndEnergies[iH].second;
      float scale = energy / position.mag();
      hitVectors[iH].SetPxPyPzE(position.x() * scale, position.y() * scale, position.z() * scale, energy);
      showerVector += hitVectors[iH];
      hitVectorIndices[iH] = iH;
    }
    //overwrites whatever was there before
    //Jan: hack in the propagated vector
    showerVector = simClusters[iSC].impactPoint();

    // sort the hit vector indices according to distance to the shower vector, closest first
    // notice the change from iH to iiH as from now on, the hitVectorIndices hold indices referring
    // to the actual index in hitVectors
    sort(
        hitVectorIndices.begin(), hitVectorIndices.end(), [&showerVector, &hitVectors](const int& iiH, const int& jjH) {
          return deltaR(showerVector, hitVectors[iiH]) < deltaR(showerVector, hitVectors[jjH]);
        });

    // loop through hit vectors in order of the sorted indices until the target energy is reached
    float radius = deltaR(showerVector, hitVectors[hitVectorIndices[nHits - 1]]);
    if (nHits >= minHitsRadius_) {
      float sumEnergy = 0.;

      for (int iiH = 0; iiH < nHits; iiH++) {
        int iH = hitVectorIndices[iiH];
        sumEnergy += hitVectors[iH].E();
        // define the radius as the average of two or three guidance points close to the requested
        // shower containment fraction
        if (sumEnergy / showerVector.E() > showerContainment_) {
          if (iiH >= 1 && iiH <= nHits - 2) {
            float r1 = deltaR(showerVector, hitVectors[iH]);
            float f1 = sumEnergy / showerVector.E();
            float r2 = deltaR(showerVector, hitVectors[hitVectorIndices[iiH - 1]]);
            float r3 = deltaR(showerVector, hitVectors[hitVectorIndices[iiH + 1]]);
            float f2 = (sumEnergy - hitVectors[hitVectorIndices[iiH - 1]].E()) / showerVector.E();
            float f3 = (sumEnergy + hitVectors[hitVectorIndices[iiH + 1]].E()) / showerVector.E();
            radius = getShowerRadius(showerContainment_, r1, f1, r2, f2, r3, f3);
          } else if (iiH >= 1) {
            float r1 = deltaR(showerVector, hitVectors[iH]);
            float f1 = sumEnergy / showerVector.E();
            float r2 = deltaR(showerVector, hitVectors[hitVectorIndices[iiH - 1]]);
            float f2 = (sumEnergy - hitVectors[hitVectorIndices[iiH - 1]].E()) / showerVector.E();
            radius = getShowerRadius(showerContainment_, r1, f1, r2, f2);
          } else if (iiH <= nHits - 2) {
            float r1 = deltaR(showerVector, hitVectors[iH]);
            float f1 = sumEnergy / showerVector.E();
            float r3 = deltaR(showerVector, hitVectors[hitVectorIndices[iiH + 1]]);
            float f3 = (sumEnergy + hitVectors[hitVectorIndices[iiH + 1]].E()) / showerVector.E();
            radius = getShowerRadius(showerContainment_, r1, f1, r3, f3);
          }
        }
      }
    }

    (*vectors)[iSC] = showerVector;
    (*radii)[iSC] = radius;
  }

  // sort the cluster indices by eta
  sort(scIndices.begin(), scIndices.end(), [&simClusters](const int& iSC, const int& jSC) {
    return simClusters[iSC].impactPoint().eta() < simClusters[jSC].impactPoint().eta();
  });

  // define a vector of chained indices for easy lookup of closeby clusters
  // this might look trivial, but the next/prev pointers are changed dynamically downstream to
  // describe / close holes in the chain during cluster merging
  std::vector<ChainIndex*> chain;
  for (int iiSC = 0; iiSC < nHGCalSimClusters; iiSC++) {
    // add a new chain element
    int iSC = scIndices[iiSC];
    chain.push_back(new ChainIndex(iSC));

    // connect elements
    if (iiSC > 0) {
      chain[iiSC]->prev = chain[iiSC - 1];
      chain[iiSC - 1]->next = chain[iiSC];
    }
  }

  while (chain.size() > 0) {
    // define the merging group, seeded by the current front-most element of the chain
    // (fancy talk for the sim cluster with the smallest eta that hasn't been checked yet)
    std::vector<ChainIndex*> chainIndices = {chain[0]};
    std::vector<int> group = {chain[0]->iSC};
    int iG = 0;

    while (iG < (int)chainIndices.size()) {
      // look for nearby clusters of the sim cluster in the group referred to by iG
      // this approach mimics recursion as the group might be extended within this while loop
      ChainIndex* curr = chainIndices[iG];

      // check clusters with smaller eta
      ChainIndex* prev = curr->prev;
      while (prev != nullptr) {
        // do nothing when the previous element is already in the group
        if (std::find(group.begin(), group.end(), prev->iSC) == group.end()) {
          // stop when the difference in eta is too high already
          const auto& currCluster = simClusters[curr->iSC];
          const auto& prevCluster = simClusters[prev->iSC];
          auto dEta =
              fabs(currCluster.impactPoint().eta() - prevCluster.impactPoint().eta());  // fabs actually not required
          if (dEta > maxDeltaEtaGroup_) {
            break;
          }
          // check if the two clusters should be merged
          // TODO: shouldn't the check consider all elements already in the group? ordering issue ahead?
          if (checkSimClusterMerging(curr->iSC, prev->iSC, simClusters, simClusterHistory, radii, vectors)) {
            // add the element to the group
            chainIndices.push_back(prev);
            group.push_back(prev->iSC);
          }
        }

        // go backward
        prev = prev->prev;
      }

      // check clusters with higher eta
      ChainIndex* next = curr->next;
      while (next != nullptr) {
        // do nothing when the next element is already in the group
        if (std::find(group.begin(), group.end(), next->iSC) == group.end()) {
          // stop when the difference in eta is too high already
          const auto& currCluster = simClusters[curr->iSC];
          const auto& nextCluster = simClusters[next->iSC];
          auto dEta =
              fabs(nextCluster.impactPoint().eta() - currCluster.impactPoint().eta());  // fabs actually not required
          if (dEta > maxDeltaEtaGroup_) {
            break;
          }
          // check if the two clusters should be merged
          // TODO: see not above
          if (checkSimClusterMerging(next->iSC, curr->iSC, simClusters, simClusterHistory, radii, vectors)) {
            // add the element to the group
            chainIndices.push_back(next);
            group.push_back(next->iSC);
          }
        }

        // go forward
        next = next->next;
      }

      // remove the current element and reconnect the chain
      if (curr->prev != nullptr) {
        if (curr->next != nullptr) {
          curr->prev->next = curr->next;
          curr->next->prev = curr->prev;
        } else {
          curr->prev->next = nullptr;
        }
      } else if (curr->next != nullptr) {
        curr->next->prev = nullptr;
      }

      std::vector<ChainIndex*>::iterator it = std::find(chain.begin(), chain.end(), curr);
      delete *it;
      chain.erase(it);

      // choose the next element in the group for the next iteration
      iG++;
    }

    // store the actual group of sim cluster indices
    groups.push_back(group);
  }

  // group debugging
  std::cout << "created " << groups.size() << " groups" << std::endl;
  double sumSize = 0.;
  for (const auto& g : groups) {
    sumSize += g.size();
  }
  std::cout << "average size: " << (sumSize / groups.size()) << std::endl;

  // radius debugging
  // {
  //   TCanvas* canvas = new TCanvas();
  //   TH1F* hist = new TH1F(("h" + std::to_string(firstI)).c_str(), ";Radius;# Entries", 51, -0.02, 2.02);

  //   int nRadii = 0;
  //   double sumRadius = 0.;
  //   for (const float &r : radii) {
  //     // if (isinf(r)) std::cout << "got inf radius!" << std::endl;
  //     hist->Fill(r);
  //     if (r > 0) {
  //       sumRadius += r;
  //       nRadii++;
  //     }
  //   }
  //   std::cout << "sum: " << sumRadius << std::endl;
  //   std::cout << "n radii: " << nRadii << std::endl;
  //   double meanRadius = sumRadius / nRadii;
  //   std::cout << "mean: " << meanRadius << std::endl;
  //   double sumDiffRadius2 = 0.;
  //   for (const float &r : radii) {
  //     if (r > 0) {
  //       double diffRadius = r - meanRadius;
  //       sumDiffRadius2 += diffRadius * diffRadius;
  //     }
  //   }
  //   double varianceRadius = sumDiffRadius2 / (nRadii - 1);
  //   double stdRadius = sqrt(varianceRadius);
  //   std::cout << "radius mean: " << meanRadius << ", std: " << stdRadius << std::endl;

  //   hist->Draw();
  //   canvas->SaveAs(("hist_" + std::to_string(firstI) + ".pdf").c_str());

  //   delete hist;
  //   delete canvas;
  // }
}

bool HGCTruthProducer::checkSimClusterMerging(
    int iSC,
    int jSC,
    const SimClusterCollection& simClusters,
    const std::vector<SimClusterHistory>& simClusterHistory,
    const std::unique_ptr<std::vector<float>>& radii,
    const std::unique_ptr<std::vector<math::XYZTLorentzVectorD>>& vectors) const {
  // get some values
  auto dR = deltaR(vectors->at(iSC), vectors->at(jSC));

  //Jan
  return dR < 0.016;

  auto radius1 = radii->at(iSC);
  auto radius2 = radii->at(jSC);

  // define a combined radius using error propagation rules (as the pull is defined statistics-like)
  auto combinedRadius = pow(radius1 * radius1 + radius2 * radius2, 0.5);

  // when both clusters have no radius, i.e., they are coming from single hits, make a quick
  // decision solely based on dR (FREE PARAMETER)
  if (combinedRadius == 0) {
    return dR < 0.005;
  }

  // if at least one clusters has a non-zero radius, define a pull
  auto pull = dR / combinedRadius;

  // use a simple merging criterion based on the pull (FREE PARAMETER)
  return pull < 0.5;

  // TODO: exploit information about common ancestors (e.g. useful for e/gamma, pi0, ...)
}

void HGCTruthProducer::mergeSimClusters(const SimClusterCollection& simClusters,
                                        const std::vector<SimClusterHistory>& simClusterHistory,
                                        const std::vector<std::vector<int>>& groups,
                                        std::unique_ptr<SimClusterCollection>& mergedSimClusters,
                                        const std::vector<const HGCRecHit*>& rechits,
                                        const std::unordered_map<DetId, size_t>& rh_detid_to_idx) const {
  for (std::vector<int> group : groups) {
    if (group.size() == 0) {
      continue;
    }

    // sort group elements by sim cluster energies
    sort(group.begin(), group.end(), [&simClusters](const int& iSC, const int& jSC) {
      return simClusters[iSC].energy() > simClusters[jSC].energy();
    });

    // get a vector of all sim tracks, also store the total energy
    ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<float>> combinedmomentum;
    for (const int& iSC : group) {
      // there is currently only one track per sim clusters
      combinedmomentum += simClusters[iSC].impactMomentum();
      std::cout << "pos " << iSC << ": " << simClusters[iSC].impactPoint() << " eta "
                << simClusters[iSC].impactPoint().Eta() << " momentum " << simClusters[iSC].impactMomentum()
                << " vertexmomentum " << simClusters[iSC].p4() << std::endl;
    }

    // determine the pdg id using infos about common ancestors
    int pdgId = 78;  // TODO (this doesn't matter for now)

    ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double>> combinedmomentumD(
        combinedmomentum.x(), combinedmomentum.y(), combinedmomentum.z(), combinedmomentum.t());
    //still needs to be added
    SimTrack simtr(0, combinedmomentumD);
    SimCluster newsc({simtr}, pdgId);

    newsc.setImpactMomentum(combinedmomentum);
    // create the cluster
    mergedSimClusters->emplace_back(newsc);
    auto& cluster = mergedSimClusters->back();

    // fill hits and energy fractions
    // since we merge, we have to care about duplicate hits and this might become drastically slow
    // therefore, use the default addRecHitAndFraction method for the first cluster, and then
    // loop over the remaining ones to invoke the slower but necessary addDuplicateRecHitAndFraction

    double combined_time = 0, totalenergy = 0;
    for (const auto& hf : simClusters[group[0]].hits_and_fractions()) {
      cluster.addRecHitAndFraction(hf.first, hf.second);
      double en = simClusters[group[0]].impactMomentum().E();  //energy weighted time
      totalenergy += en;
      combined_time += simClusters[group[0]].impactPoint().T() * en;
    }
    for (int iiSC = 1; iiSC < (int)group.size(); iiSC++) {
      int iSC = group[iiSC];
      for (const auto& hf : simClusters[iSC].hits_and_fractions()) {
        cluster.addDuplicateRecHitAndFraction(hf.first, hf.second);
      }
      double en = simClusters[iSC].impactMomentum().E();  //energy weighted time
      totalenergy += en;
      combined_time += simClusters[iSC].impactPoint().T() * en;
    }
    combined_time /= totalenergy;
    //recalculate entry position at first hit
    SimClusterTools sctools;
    sctools.setRechitTools(recHitTools_);
    sctools.setRechitVector(rechits);
    sctools.setIndexMap(rh_detid_to_idx);

    sctools.recalculatePosition(cluster, combined_time);

    //get energy weighted center and lowest layer
    //
    //get lowest layer index
  }
}

DEFINE_FWK_MODULE(HGCTruthProducer);

// unused helper methods, to be removed
// bool vectorsIntersect(const std::vector<int> &v1, const std::vector<int> &v2) {
//   // for large vectors, it would be useful to loop through sorted indices and stop early
//   // when no match of a number of v1 is possible with any element in v2, but when comparing
//   // vectors of incoming vertices, i.e., parent particles, those vectors are rather small
//   for (const int &i1 : v1) {
//     for (const int &i2 : v2) {
//       if (i1 == i2) {
//         return true;
//       }
//     }
//   }
//   return false;

//   // second possible approach based on set intersections, but this one compares all elements
//   // without stopping early
//   // std::vector<int> result;
//   // std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), back_inserter(result));
//   // return result.size() > 0;
// }
//
// float getShowerRadius(float f, float r1, float f1, float r2, float f2) {
//   float b = log(log(1.f - f2) / log(1.f - f1)) / log(r2 / r1);
//   float a = - log(1.f - f1) / pow(r1, b);
//   float res = pow(-log(1.f - f) / a, 1.f / b);
//   if (isinf(res)) {
//     std::cout << "produced inf" << std::endl;
//     std::cout << "f : " << f << std::endl;
//     std::cout << "r1: " << r1 << std::endl;
//     std::cout << "f1: " << f1 << std::endl;
//     std::cout << "r2: " << r2 << std::endl;
//     std::cout << "f2: " << f2 << std::endl;
//     std::cout << "--------------" << std::endl;
//   }
//   return pow(-log(1.f - f) / a, 1.f / b);
// }
//
// float getShowerRadius(float f, float r1, float f1, float r2, float f2, float r3, float f3) {
//   float d12 = getShowerRadius(f, r1, f1, r2, f2);
//   float d13 = getShowerRadius(f, r1, f1, r3, f3);
//   float d23 = getShowerRadius(f, r2, f2, r3, f3);
//   return (d12 + d13 + d23) / 3.f;
// }
