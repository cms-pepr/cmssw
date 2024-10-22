#include <ostream>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenRunInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

#include "SimGeneral/HepPDTRecord/interface/ParticleDataTable.h"

#include "DataFormats/Math/interface/Vector3D.h"

#include "IOMC/ParticleGuns/interface/CloseByFlatDeltaRGunProducer.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Units/GlobalPhysicalConstants.h"
#include "CLHEP/Units/GlobalSystemOfUnits.h"

void edm::CloseByFlatDeltaRGunProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<std::vector<int>>("particleIDs");
  desc.add<int>("nParticles", 1);
  desc.add<bool>("exactShoot", true);
  desc.add<bool>("randomShoot", false);
  desc.add<double>("eMin", 1.);
  desc.add<double>("eMax", 100.);
  desc.add<double>("etaMin", 0.);
  desc.add<double>("etaMax", 0.);
  desc.add<double>("phiMin", 0.);
  desc.add<double>("phiMax", 2 * pi);
  desc.add<double>("zMin", 0.);
  desc.add<double>("zMax", 0.);
  desc.add<double>("deltaRMin", 0.);
  desc.add<double>("deltaRMax", 0.5);
  desc.addUntracked<bool>("debug", false);

  descriptions.add("closeByFlatDeltaRGunProducer", desc);
}

edm::CloseByFlatDeltaRGunProducer::CloseByFlatDeltaRGunProducer(const edm::ParameterSet& params)
    : particleIDs_(params.getParameter<std::vector<int>>("particleIDs")),
      nParticles_(params.getParameter<int>("nParticles")),
      exactShoot_(params.getParameter<bool>("exactShoot")),
      randomShoot_(params.getParameter<bool>("randomShoot")),
      eMin_(params.getParameter<double>("eMin")),
      eMax_(params.getParameter<double>("eMax")),
      etaMin_(params.getParameter<double>("etaMin")),
      etaMax_(params.getParameter<double>("etaMax")),
      phiMin_(params.getParameter<double>("phiMin")),
      phiMax_(params.getParameter<double>("phiMax")),
      zMin_(params.getParameter<double>("zMin")),
      zMax_(params.getParameter<double>("zMax")),
      deltaRMin_(params.getParameter<double>("deltaRMin")),
      deltaRMax_(params.getParameter<double>("deltaRMax")),
      debug_(params.getUntrackedParameter<bool>("debug")),
      genEvent_(nullptr) {
  produces<edm::HepMCProduct>("unsmeared");
  produces<GenEventInfoProduct>();
  produces<GenRunInfoProduct, edm::Transition::EndRun>();
}

edm::CloseByFlatDeltaRGunProducer::~CloseByFlatDeltaRGunProducer() {}

void edm::CloseByFlatDeltaRGunProducer::beginRun(const edm::Run& run, const edm::EventSetup& setup) {
  setup.getData(pdgTable_);
}

void edm::CloseByFlatDeltaRGunProducer::endRun(const edm::Run& run, const edm::EventSetup& setup) {}

void edm::CloseByFlatDeltaRGunProducer::produce(edm::Event& event, const edm::EventSetup& setup) {
  edm::Service<edm::RandomNumberGenerator> rng;
  CLHEP::HepRandomEngine* engine = &(rng->getEngine(event.streamID()));

  if (debug_) {
    LogDebug("CloseByFlatDeltaRGunProducer") << " : Begin New Event Generation" << std::endl;
  }

  // create a new event to fill
  genEvent_ = new HepMC::GenEvent();

  // determine gun parameters for first particle to shoot (postfixed with "0")
  double phi0 = CLHEP::RandFlat::shoot(engine, phiMin_, phiMax_);
  double thetaMin = 2 * atan(exp(-etaMax_));
  double thetaMax = 2 * atan(exp(-etaMin_));
  double theta0 = CLHEP::RandFlat::shoot(engine, thetaMin, thetaMax);
  double eta0 = -log(tan(0.5 * theta0));

  // longitudinal gun position along the generated trajectory
  double z0 = CLHEP::RandFlat::shoot(engine, zMin_, zMax_);
  double rho0 = tan(theta0) * z0;

  // define variables that are changed per shot particle
  double phi = phi0;
  double eta = eta0;
  double rho = rho0;

  // determine the number of particles to shoot
  int n = 0;
  if (exactShoot_) {
    n = (int)particleIDs_.size();
  } else if (randomShoot_) {
    n = CLHEP::RandFlat::shoot(engine, 1, nParticles_ + 1);
  } else {
    n = nParticles_;
  }

  // shoot particles
  for (int i = 0; i < n; i++) {
    // find a new position relative to the first particle, obviously for all but the first one
    if (i > 0) {
      // create a random deltaR
      double deltaR = CLHEP::RandFlat::shoot(engine, deltaRMin_, deltaRMax_);

      // split delta R randomly in phi and eta directions
      double alpha = CLHEP::RandFlat::shoot(engine, 0., 2. * pi);
      double deltaPhi = sin(alpha) * deltaR;
      double deltaEta = cos(alpha) * deltaR;

      // update phi
      phi = phi0 + deltaPhi;

      // update rho
      // the approach is to transorm a difference in eta to a difference in rho using:
      // 1. tan(theta) = rho / z       <=> rho   = z * tan(theta)
      // 2. eta = -log(tan(theta / 2)) <=> theta = 2 * atan(exp(-eta))
      // 2. in 1.                       => rho   = z * tan(2 * atan(exp(-eta)))
      // using tan(atan(2 * x)) = -2 * x (x^2 - 1) leads to
      //                                => rho = -2 * z * x / (x^2 - 1), with x = exp(-eta)
      // for eta = eta0 + deltaEta, this allows for defining x = exp(-(eta0 + deltaEta))
      eta = eta0 + deltaEta;
      double x = exp(-eta);
      rho = -2 * z0 * x / (x * x - 1);
    }

    // compute the gun / vertex position
    // the time offset is given in c*t and is used later on in the digitization
    double x = rho * cos(phi);
    double y = rho * sin(phi);
    double timeOffset = sqrt(x * x + y * y + z0 * z0) * cm / c_light;
    HepMC::GenVertex* vtx = new HepMC::GenVertex(HepMC::FourVector(x * cm, y * cm, z0 * cm, timeOffset * c_light));

    // obtain kinematics
    int id = particleIDs_[exactShoot_ ? i : CLHEP::RandFlat::shoot(engine, 0, particleIDs_.size())];
    const HepPDT::ParticleData* pData = pdgTable_->particle(HepPDT::ParticleID(abs(id)));
    double e = CLHEP::RandFlat::shoot(engine, eMin_, eMax_);
    double m = pData->mass().value();
    double p = sqrt(e * e - m * m);

    // determine the momentum vector
    math::XYZVector pVec = p * math::XYZVector(cos(phi), sin(phi), sinh(eta)).unit();

    // create the GenParticle
    HepMC::FourVector fVec(pVec.x(), pVec.y(), pVec.z(), e);
    HepMC::GenParticle* particle = new HepMC::GenParticle(fVec, id, 1);
    particle->suggest_barcode(i + 1);

    // add the particle to the vertex and the vertex to the event
    vtx->add_particle_out(particle);
    genEvent_->add_vertex(vtx);

    if (debug_) {
      vtx->print();
      particle->print();
    }
  }

  // fill event attributes
  genEvent_->set_event_number(event.id().event());
  genEvent_->set_signal_process_id(20);

  if (debug_) {
    genEvent_->print();
  }

  // store outputs
  std::unique_ptr<HepMCProduct> BProduct(new HepMCProduct());
  BProduct->addHepMCData(genEvent_);
  event.put(std::move(BProduct), "unsmeared");
  std::unique_ptr<GenEventInfoProduct> genEventInfo(new GenEventInfoProduct(genEvent_));
  event.put(std::move(genEventInfo));

  if (debug_) {
    LogDebug("CloseByFlatDeltaRGunProducer") << " : Event Generation Done " << std::endl;
  }
}

void edm::CloseByFlatDeltaRGunProducer::endRunProduce(edm::Run& run, const edm::EventSetup& setup) {
  std::unique_ptr<GenRunInfoProduct> genRunInfo(new GenRunInfoProduct());
  run.put(std::move(genRunInfo));
}
