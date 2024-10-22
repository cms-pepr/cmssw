#include "FWCore/Framework/interface/MakerMacros.h"
#include "CommonTools/UtilAlgos/interface/Merger.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHit.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/Common/interface/CloneTrait.h"
#include "DataFormats/Common/interface/RefToBaseVector.h"
#include "DataFormats/Common/interface/CopyPolicy.h"


typedef Merger<HGCRecHitCollection, HGCRecHitCollection, edm::CopyPolicy<HGCRecHit>> HGCRecHitCollectionMerger;

DEFINE_FWK_MODULE(HGCRecHitCollectionMerger);
