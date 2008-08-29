#include "FWCore/Framework/interface/MakerMacros.h"

#include "TopQuarkAnalysis/TopKinFitter/plugins/TtSemiLepKinFitProducer.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"

typedef TtSemiLepKinFitProducer< std::vector<pat::Muon>     > TtSemiLepKinFitProducerMuon;
typedef TtSemiLepKinFitProducer< std::vector<pat::Electron> > TtSemiLepKinFitProducerElectron;

DEFINE_FWK_MODULE(TtSemiLepKinFitProducerMuon);
DEFINE_FWK_MODULE(TtSemiLepKinFitProducerElectron);
