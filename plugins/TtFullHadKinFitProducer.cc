#include "TopQuarkAnalysis/TopKinFitter/plugins/TtFullHadKinFitProducer.h"

static const unsigned int nPartons=6;

/// default constructor  
TtFullHadKinFitProducer::TtFullHadKinFitProducer(const edm::ParameterSet& cfg):
  jets_        (cfg.getParameter<edm::InputTag>("jets")),
  match_       (cfg.getParameter<edm::InputTag>("match")),
  useOnlyMatch_(cfg.getParameter<bool>("useOnlyMatch")),
  maxNJets_    (cfg.getParameter<int>("maxNJets")),
  maxNComb_    (cfg.getParameter<int>("maxNComb")),
  maxNrIter_   (cfg.getParameter<unsigned int>("maxNrIter")),
  maxDeltaS_   (cfg.getParameter<double>("maxDeltaS")),
  maxF_        (cfg.getParameter<double>("maxF")),
  jetParam_    (cfg.getParameter<unsigned>("jetParametrisation")),
  constraints_ (cfg.getParameter<std::vector<unsigned> >("constraints"))
{
  // define kinematic fit interface
  fitter = new TtFullHadKinFitter(param(jetParam_), maxNrIter_, maxDeltaS_, maxF_, constraints(constraints_));

  // produces the following collections
  produces< std::vector<pat::Particle> >("PartonsB");
  produces< std::vector<pat::Particle> >("PartonsBBar");
  produces< std::vector<pat::Particle> >("PartonsLightQ");
  produces< std::vector<pat::Particle> >("PartonsLightQBar");
  produces< std::vector<pat::Particle> >("PartonsLightP");
  produces< std::vector<pat::Particle> >("PartonsLightPBar");

  produces< std::vector<std::vector<int> > >();
  produces< std::vector<double> >("Chi2");
  produces< std::vector<double> >("Prob");
  produces< std::vector<int> >("Status");
}

/// default destructor
TtFullHadKinFitProducer::~TtFullHadKinFitProducer()
{
  delete fitter;
}

/// produce fitted object collections and meta data describing fit quality
void 
TtFullHadKinFitProducer::produce(edm::Event& event, const edm::EventSetup& setup)
{
  // pointer for output collections
  std::auto_ptr< std::vector<pat::Particle> > pPartonsB( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pPartonsBBar( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pPartonsLightQ   ( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pPartonsLightQBar( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pPartonsLightP   ( new std::vector<pat::Particle> );
  std::auto_ptr< std::vector<pat::Particle> > pPartonsLightPBar( new std::vector<pat::Particle> );
  // pointer for meta information
  std::auto_ptr< std::vector<std::vector<int> > > pCombi ( new std::vector<std::vector<int> > );
  std::auto_ptr< std::vector<double> > pChi2  ( new std::vector<double> );
  std::auto_ptr< std::vector<double> > pProb  ( new std::vector<double> );
  std::auto_ptr< std::vector<int> > pStatus( new std::vector<int> );

  // get jet collection
  edm::Handle<std::vector<pat::Jet> > jets;
  event.getByLabel(jets_, jets);

  // get match in case that useOnlyMatch_ is true
  std::vector<int> match;

  if(useOnlyMatch_) {
    bool invalidMatch=false;
    // in case that only a ceratin match should be used, get match here
    edm::Handle<std::vector<std::vector<int> > > matches;
    event.getByLabel(match_, matches);
    match = *(matches->begin());
    // check if match is valid
    if( match.size()!=nPartons ){
      invalidMatch=true;
    }
    else {
      for(unsigned int idx=0; idx<match.size(); ++idx) {
	if(match[idx]<0 || match[idx]>=(int)jets->size()) {
	  invalidMatch=true;
	  break;
	}
      }
    }
  }

  /**
     analyze different jet combinations using the KinFitter
     (or only a given jet combination if useOnlyMatch=true)
  **/
  
  std::vector<int> jetIndices;
  if(!useOnlyMatch_) {
    for(unsigned int idx=0; idx<jets->size(); ++idx){
      if(maxNJets_>=(int)nPartons && maxNJets_==(int)idx) break;
      jetIndices.push_back(idx);
    }
  }
  
  std::vector<int> combi;
  for(unsigned int idx=0; idx<nPartons; ++idx) {
    useOnlyMatch_?combi.push_back(match[idx]):combi.push_back(idx);
  }

  std::list<KinFitResult> fitResults;
  do{
    for(int cnt=0; cnt<TMath::Factorial(combi.size()); ++cnt){
      // take into account indistinguishability of the two jets from the two W decays,
      // this reduces the combinatorics by a factor of 2*2
      if( combi[TtFullHadEvtPartons::LightQ] < combi[TtFullHadEvtPartons::LightQBar] ||
	  combi[TtFullHadEvtPartons::LightP] < combi[TtFullHadEvtPartons::LightPBar] ||
	  useOnlyMatch_ ) {
	
	std::vector<pat::Jet> jetCombi;
	jetCombi.resize(nPartons);
	jetCombi[TtFullHadEvtPartons::LightQ   ] = (*jets)[combi[TtFullHadEvtPartons::LightQ   ]];
	jetCombi[TtFullHadEvtPartons::LightQBar] = (*jets)[combi[TtFullHadEvtPartons::LightQBar]];
	jetCombi[TtFullHadEvtPartons::B        ] = (*jets)[combi[TtFullHadEvtPartons::B        ]];
	jetCombi[TtFullHadEvtPartons::BBar     ] = (*jets)[combi[TtFullHadEvtPartons::BBar     ]];
	jetCombi[TtFullHadEvtPartons::LightP   ] = (*jets)[combi[TtFullHadEvtPartons::LightP   ]];
	jetCombi[TtFullHadEvtPartons::LightPBar] = (*jets)[combi[TtFullHadEvtPartons::LightPBar]];

	// do the kinematic fit
	int status = fitter->fit(jetCombi);

	if( status!=-10 ) { 
	  // fill struct KinFitResults if was not
	  // aborted (due to errors during fitting)
	  KinFitResult result;
	  result.Status   = status;
	  result.Chi2     = fitter->fitS();
	  result.Prob     = fitter->fitProb();
	  result.B        = fitter->fittedB();
	  result.BBar     = fitter->fittedBBar();
	  result.LightQ   = fitter->fittedLightQ();
	  result.LightQBar= fitter->fittedLightQBar();
	  result.LightP   = fitter->fittedLightP();
	  result.LightPBar= fitter->fittedLightPBar();
	  result.JetCombi = combi;
	  // push back fit result
	  fitResults.push_back( result );
	}
      }
      // don't go through combinatorics if useOnlyMatch was chosen
      if(useOnlyMatch_){
	break; 
      }
      // next permutation
      std::next_permutation( combi.begin(), combi.end() );
    }
    // don't go through combinatorics if useOnlyMatch was chosen
    if(useOnlyMatch_){
      break;
    }
  }
  while( stdcomb::next_combination( jetIndices.begin(), jetIndices.end(), combi.begin(), combi.end() ) );
  
  // sort results w.r.t. chi2 values
  fitResults.sort();
  
  /**
     feed out result starting with the 
     JetComb having the smallest chi2
  **/

  if( fitResults.size() < 1 ) { 
    // in case no fit results were stored in the list (i.e. when all fits were aborted)
    pPartonsB->push_back( fitter->fittedB() );
    pPartonsBBar->push_back( fitter->fittedBBar() );
    pPartonsLightQ->push_back( fitter->fittedLightQ() );
    pPartonsLightQBar->push_back( fitter->fittedLightQBar() );
    pPartonsLightP->push_back( fitter->fittedLightP() );
    pPartonsLightPBar->push_back( fitter->fittedLightPBar() );
    // indices referring to the jet combination
    std::vector<int> invalidCombi(nPartons, -1);
    pCombi->push_back( invalidCombi );
    // chi2
    pChi2->push_back( -1. );
    // chi2 probability
    pProb->push_back( -1. );
    // status of the fitter
    pStatus->push_back( -1 );
  }
  else {
    unsigned int iComb = 0;
    for(std::list<KinFitResult>::const_iterator result = fitResults.begin(); result != fitResults.end(); ++result){
      if(maxNComb_>=1 && iComb==(unsigned int)maxNComb_){ 
	break;
      }
      iComb++;
      // partons
      pPartonsB->push_back( result->B );
      pPartonsBBar->push_back( result->BBar );
      pPartonsLightQ->push_back( result->LightQ );
      pPartonsLightQBar->push_back( result->LightQBar );
      pPartonsLightP->push_back( result->LightP );
      pPartonsLightPBar->push_back( result->LightPBar );
      // indices referring to the jet combination
      pCombi->push_back( result->JetCombi );
      // chi2
      pChi2->push_back( result->Chi2 );
      // chi2 probability
      pProb->push_back( result->Prob );
      // status of the fitter
      pStatus->push_back( result->Status );
    }
  }
  event.put(pCombi);
  event.put(pPartonsB        , "PartonsB" );
  event.put(pPartonsBBar     , "PartonsBBar" );
  event.put(pPartonsLightQ   , "PartonsLightQ"   );
  event.put(pPartonsLightQBar, "PartonsLightQBar");
  event.put(pPartonsLightP   , "PartonsLightP"   );
  event.put(pPartonsLightPBar, "PartonsLightPBar");
  event.put(pChi2   , "Chi2"   );
  event.put(pProb   , "Prob"   );
  event.put(pStatus , "Status" );
}

TtFullHadKinFitter::Param 
TtFullHadKinFitProducer::param(unsigned int configParameter) 
{
  TtFullHadKinFitter::Param result;
  switch(configParameter){
  case TtFullHadKinFitter::kEMom       : result=TtFullHadKinFitter::kEMom;       break;
  case TtFullHadKinFitter::kEtEtaPhi   : result=TtFullHadKinFitter::kEtEtaPhi;   break;
  case TtFullHadKinFitter::kEtThetaPhi : result=TtFullHadKinFitter::kEtThetaPhi; break;
  default: 
    throw cms::Exception("WrongConfig") 
      << "Chosen jet parametrization is not supported: " << configParameter << "\n";
    break;
  }
  return result;
} 

TtFullHadKinFitter::Constraint 
TtFullHadKinFitProducer::constraint(unsigned configParameter) 
{
  TtFullHadKinFitter::Constraint result;
  switch(configParameter){
  case TtFullHadKinFitter::kWPlusMass  : result=TtFullHadKinFitter::kWPlusMass;  break;
  case TtFullHadKinFitter::kWMinusMass : result=TtFullHadKinFitter::kWMinusMass; break;
  case TtFullHadKinFitter::kTopMass    : result=TtFullHadKinFitter::kTopMass;    break;
  case TtFullHadKinFitter::kTopBarMass : result=TtFullHadKinFitter::kTopBarMass; break;
  default: 
    throw cms::Exception("WrongConfig") 
      << "Chosen fit constraint is not supported: " << configParameter << "\n";
    break;
  }
  return result;
} 

std::vector<TtFullHadKinFitter::Constraint>
TtFullHadKinFitProducer::constraints(std::vector<unsigned>& configParameters)
{
  std::vector<TtFullHadKinFitter::Constraint> result;
  for(unsigned i=0; i<configParameters.size(); ++i){
    result.push_back(constraint(configParameters[i]));
  }
  return result; 
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(TtFullHadKinFitProducer);