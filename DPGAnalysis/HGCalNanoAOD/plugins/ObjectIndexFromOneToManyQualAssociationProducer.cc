#include "PhysicsTools/NanoAOD/interface/ObjectIndexFromOneToManyQualAssociationProducer.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimCluster.h"
#include "SimDataFormats/CaloAnalysis/interface/SimClusterFwd.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "FWCore/Framework/interface/MakerMacros.h"

typedef ObjectIndexFromOneToManyQualAssociationTableProducer<std::vector<reco::CaloCluster>, SimClusterCollection, float>
    LayerClusterToSimClusterIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<HGCRecHitCollection, SimClusterCollection, float>
    CaloRecHitToSimClusterIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<HGCRecHitCollection, std::vector<reco::CaloCluster>, float>
    HGCRecHitToLayerClusterIndexTableProducer;
typedef ObjectIndexFromOneToManyQualAssociationTableProducer<SimClusterCollection, SimClusterCollection, float>
    SimClusterToSimClustersIndexTableProducer;

DEFINE_FWK_MODULE(LayerClusterToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(HGCRecHitToLayerClusterIndexTableProducer);
DEFINE_FWK_MODULE(CaloRecHitToSimClusterIndexTableProducer);
DEFINE_FWK_MODULE(SimClusterToSimClustersIndexTableProducer);
