#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "DataFormats/NanoAOD/interface/FlatTable.h"
#include "DataFormats/Common/interface/View.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

#include <vector>
#include <iostream>

template <typename T>
class HitPositionTableProducer : public edm::stream::EDProducer<> {
public:
  HitPositionTableProducer(edm::ParameterSet const& params)
      : name_(params.getParameter<std::string>("name")),
        doc_(params.getParameter<std::string>("doc")),
        src_(consumes<T>(params.getParameter<edm::InputTag>("src"))),
        cut_(params.getParameter<std::string>("cut"), true) {
    produces<nanoaod::FlatTable>();
  }

  ~HitPositionTableProducer() override {}

  GlobalPoint virtual positionFromHit(const typename T::value_type& hit) { 
    throw cms::Exception("HitPositionTableProducer") << "Virtual function positionFromHist is not implemented!"; 
  }

  float virtual radiusFromHit(const typename T::value_type& hit) {
    throw cms::Exception("HitPositionTableProducer") << "Virtual function radiusFromHit is not implemented!";
  }

  void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override {
    edm::Handle<T> objs;
    iEvent.getByToken(src_, objs);

    std::vector<float> xvals;
    std::vector<float> yvals;
    std::vector<float> zvals;
    std::vector<float> hitrvals;
    for (const auto& obj : *objs) {
      if (cut_(obj)) {
        auto position = positionFromHit(obj);
        xvals.emplace_back(position.x());
        yvals.emplace_back(position.y());
        zvals.emplace_back(position.z());
        hitrvals.emplace_back(radiusFromHit(obj));
      }
    }

    auto tab = std::make_unique<nanoaod::FlatTable>(xvals.size(), name_, false, true);
    tab->addColumn<float>("x", xvals, "x position");
    tab->addColumn<float>("y", yvals, "y position");
    tab->addColumn<float>("z", zvals, "z position");
    //addExtraColumns(tab);
    tab->addColumn<float>("hitr", hitrvals, "radius");

    iEvent.put(std::move(tab));
  }

protected:
  const std::string name_, doc_;
  const edm::EDGetTokenT<T> src_;
  const StringCutObjectSelector<typename T::value_type> cut_;
};


