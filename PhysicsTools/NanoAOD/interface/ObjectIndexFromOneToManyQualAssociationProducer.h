#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "DataFormats/Common/interface/View.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/Common/interface/AssociationMap.h"
#include "DataFormats/Common/interface/OneToManyWithQualityGeneric.h"
#include <vector>
#include <iostream>

// P can only really be a double or float
template <typename T, typename M, typename P>
class ObjectIndexFromOneToManyQualAssociationTableProducer : public edm::global::EDProducer<> {
public:
  ObjectIndexFromOneToManyQualAssociationTableProducer(edm::ParameterSet const& params)
      : objName_(params.getParameter<std::string>("objName")),
        branchName_(params.getParameter<std::string>("branchName")),
        doc_(params.getParameter<std::string>("docString")),
        src_(consumes<T>(params.getParameter<edm::InputTag>("src"))),
        objMap_(consumes<edm::AssociationMap<edm::OneToManyWithQualityGeneric<T, M, P>>>(
            params.getParameter<edm::InputTag>("objMap"))),
        cut_(params.getParameter<std::string>("cut"), true) {
    produces<nanoaod::FlatTable>("match");
    produces<nanoaod::FlatTable>("count");
  }

  ~ObjectIndexFromOneToManyQualAssociationTableProducer() override {}

  void produce(edm::StreamID id, edm::Event& iEvent, const edm::EventSetup& iSetup) const override {
    edm::Handle<T> objs;
    iEvent.getByToken(src_, objs);

    edm::Handle<edm::AssociationMap<edm::OneToManyWithQualityGeneric<T, M, P>>> assoc;
    iEvent.getByToken(objMap_, assoc);
    //std::cout << "Size of map is " << assoc->size() << std::endl;

    std::vector<int> keys;
    std::vector<float> qualities;
    std::vector<int> nMatches;
    for (unsigned int i = 0; i < objs->size(); ++i) {
      edm::Ref<T> tk(objs, i);
      int nmatch = 0;
      if (cut_(*tk)) {
        if (assoc->numberOfAssociations(tk)) {
            auto& matchWithQual = (*assoc)[tk];
            for (auto& match : matchWithQual) {
                if (match.first.isNonnull()) {
                    keys.emplace_back(match.first.key());
                    qualities.emplace_back(match.second);
                }
            }
            nmatch = matchWithQual.size();
        } 
        
        nMatches.emplace_back(nmatch);
      }
    }

    auto tabNum = std::make_unique<nanoaod::FlatTable>(nMatches.size(), objName_, false, true);
    tabNum->addColumn<int>(branchName_ + "NumMatch", nMatches, doc_);
    auto tabMatches = std::make_unique<nanoaod::FlatTable>(keys.size(), objName_+"_"+branchName_, false, false);
    tabMatches->addColumn<int>("MatchIdx", keys, doc_);
    tabMatches->addColumn<float>("MatchQual", qualities, doc_);

    iEvent.put(std::move(tabMatches), "match");
    iEvent.put(std::move(tabNum), "count");
  }

protected:
  const std::string objName_, branchName_, doc_;
  const edm::EDGetTokenT<T> src_;
  const edm::EDGetTokenT<edm::AssociationMap<edm::OneToManyWithQualityGeneric<T, M, P>>> objMap_;
  const StringCutObjectSelector<typename T::value_type> cut_;
};
