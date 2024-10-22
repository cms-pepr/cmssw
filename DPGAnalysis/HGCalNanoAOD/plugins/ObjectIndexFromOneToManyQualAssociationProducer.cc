#include "PhysicsTools/NanoAOD/interface/ObjectIndexFromOneToManyQualAssociationProducer.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticle.h"
#include "SimDataFormats/CaloAnalysis/interface/CaloParticleFwd.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "FWCore/Framework/interface/MakerMacros.h"

typedef ObjectIndexFromOneToManyQualAssociationTableProducer<std::vector<reco::CaloCluster>, SimClusterCollection, float>
    LayerClusterToSimClusterIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<std::vector<reco::CaloCluster>, CaloParticleCollection, float>
    LayerClusterToCaloParticleIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<HGCRecHitCollection, std::vector<reco::CaloCluster>, float>
    HGCRecHitToLayerClusterIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<SimClusterCollection, SimClusterCollection, float>
    SimClusterToSimClustersIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<HGCRecHitCollection, SimClusterCollection, float>
    HGCRecHitToSimClusterIndexTableProducer;

DEFINE_FWK_MODULE(LayerClusterToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(LayerClusterToCaloParticleIndexTableProducer);
DEFINE_FWK_MODULE(HGCRecHitToLayerClusterIndexTableProducer);
DEFINE_FWK_MODULE(HGCRecHitToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(SimClusterToSimClustersIndexTableProducer);
