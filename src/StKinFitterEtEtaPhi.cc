// ETEtaPhi parametrisation
#include "TopQuarkAnalysis/TopKinFitter/interface/StKinFitterEtEtaPhi.h"


//
// constructor - read in the fit functions from a root file
//
StKinFitterEtEtaPhi::StKinFitterEtEtaPhi() {
  maxNrIter = 200;
  maxDeltaS = 5e-5;
  maxF = 1e-4;
  // initialisation
  setupFitter();
}

StKinFitterEtEtaPhi::StKinFitterEtEtaPhi(int a, double b, double c, vector<int> d) {
  maxNrIter = a;
  maxDeltaS = b;
  maxF = c;
  constraints = d;
  // initialisation
  setupFitter(); 
}


//
// destructor
//
StKinFitterEtEtaPhi::~StKinFitterEtEtaPhi() {
  delete cons1; delete cons2; delete cons3;// delete cons4; delete cons5; 
  delete fitBottom; delete fitLight; delete fitLepl; delete fitLepn;
  delete theFitter;
}


StEvtSolution StKinFitterEtEtaPhi::addKinFitInfo(StEvtSolution * asol) {
  StEvtSolution fitsol(*asol);
  

  TMatrixD m1(3,3), m2(3,3), m3(3,3), m4(3,3);//, m5(3,3), m6(3,3);
  m1.Zero(); m2.Zero(); m3.Zero(); m4.Zero(); //m5.Zero(); m6.Zero();
  
  TLorentzVector bottomVec(fitsol.getCalBottom().px(),fitsol.getCalBottom().py(),
                         fitsol.getCalBottom().pz(),fitsol.getCalBottom().energy());
  TLorentzVector lightVec(fitsol.getCalLight().px(),fitsol.getCalLight().py(),
                      	 fitsol.getCalLight().pz(),fitsol.getCalLight().energy());
  TLorentzVector leplVec;
  if(fitsol.getDecay()== "electron") leplVec = TLorentzVector(fitsol.getRecLepe().px(), fitsol.getRecLepe().py(),    
			 				      fitsol.getRecLepe().pz(), fitsol.getRecLepe().energy());
  if(fitsol.getDecay()== "muon")     leplVec = TLorentzVector(fitsol.getRecLepm().px(), fitsol.getRecLepm().py(),    
			 				      fitsol.getRecLepm().pz(), fitsol.getRecLepm().energy());
  TLorentzVector lepnVec(fitsol.getRecLepn().px(), fitsol.getRecLepn().py(),
			 0, fitsol.getRecLepn().et());
  
  // jet resolutions (covM in vector<double> form -> (0,0)=[0], (1,1)=[4], (2,2)=[8])

  m1(0,0) = pow(fitsol.getCalBottom().getResET(),  2);
  m1(1,1) = pow(fitsol.getCalBottom().getResEta(), 2);
  m1(2,2) = pow(fitsol.getCalBottom().getResPhi(), 2);
  m2(0,0) = pow(fitsol.getCalLight().getResET(),  2); 
  m2(1,1) = pow(fitsol.getCalLight().getResEta(), 2); 
  m2(2,2) = pow(fitsol.getCalLight().getResPhi(), 2);
  /*
  m3(0,0) = pow(fitsol.getCalHadb().getResET(),  2); 
  m3(1,1) = pow(fitsol.getCalHadb().getResEta(), 2); 
  m3(2,2) = pow(fitsol.getCalHadb().getResPhi(), 2);
  m4(0,0) = pow(fitsol.getCalLepb().getResET(),  2); 
  m4(1,1) = pow(fitsol.getCalLepb().getResEta(), 2); 
  m4(2,2) = pow(fitsol.getCalLepb().getResPhi(), 2);
  */
  if(fitsol.getDecay()== "electron"){
    m3(0,0) = pow(fitsol.getRecLepe().getResET(),  2);
    m3(1,1) = pow(fitsol.getRecLepe().getResEta(), 2); 
    m3(2,2) = pow(fitsol.getRecLepe().getResPhi(), 2);
  }
  if(fitsol.getDecay()== "muon"){
    m3(0,0) = pow(fitsol.getRecLepm().getResET(),  2);
    m3(1,1) = pow(fitsol.getRecLepm().getResEta(), 2); 
    m3(2,2) = pow(fitsol.getRecLepm().getResPhi(), 2);
  }
  m4(0,0) = pow(fitsol.getRecLepn().getResET(),  2);
  m4(1,1) = pow(fitsol.getRecLepn().getResEta(), 2);
  m4(2,2) = pow(fitsol.getRecLepn().getResPhi(), 2);
  
  fitBottom->setIni4Vec(&bottomVec); fitBottom->setCovMatrix(&m1);
  fitLight->setIni4Vec(&lightVec); fitLight->setCovMatrix(&m2);
  fitLepl->setIni4Vec(&leplVec); fitLepl->setCovMatrix(&m3);
  fitLepn->setIni4Vec(&lepnVec); fitLepn->setCovMatrix(&m4);

  theFitter->fit();
  
  // add fitted information to the solution
  if ( theFitter->getStatus() == 0 ) {
    TopParticle aFitBottom(Particle(0,math::XYZTLorentzVector(fitBottom->getCurr4Vec()->X(), fitBottom->getCurr4Vec()->Y(), fitBottom->getCurr4Vec()->Z(), fitBottom->getCurr4Vec()->E()),math::XYZPoint()));
    TopParticle aFitLight(Particle(0,math::XYZTLorentzVector(fitLight->getCurr4Vec()->X(), fitLight->getCurr4Vec()->Y(), fitLight->getCurr4Vec()->Z(), fitLight->getCurr4Vec()->E()),math::XYZPoint()));
    TopParticle aFitLepl(Particle(0,math::XYZTLorentzVector(fitLepl->getCurr4Vec()->X(), fitLepl->getCurr4Vec()->Y(), fitLepl->getCurr4Vec()->Z(), fitLepl->getCurr4Vec()->E()),math::XYZPoint()));
    TopParticle aFitLepn(Particle(0,math::XYZTLorentzVector(fitLepn->getCurr4Vec()->X(), fitLepn->getCurr4Vec()->Y(), fitLepn->getCurr4Vec()->Z(), fitLepn->getCurr4Vec()->E()),math::XYZPoint()));   

    TMatrixD Vb(3,3);  Vb  = (*fitBottom->getCovMatrixFit()); 
    TMatrixD Vq(3,3);  Vq  = (*fitLight->getCovMatrixFit()); 
    aFitBottom.setResET (Vb(0,0));  
    aFitBottom.setResEta(Vb(1,1));
    aFitBottom.setResPhi(Vb(2,2)); 
    aFitLight.setResET (Vq(0,0));  
    aFitLight.setResEta(Vq(1,1));
    aFitLight.setResPhi(Vq(2,2));
    
    TMatrixD Vl(3,3); Vl = (*fitLepl->getCovMatrixFit()); 
    aFitLepl.setResET (Vl(0,0));  
    aFitLepl.setResTheta(Vl(1,1));
    aFitLepl.setResPhi(Vl(2,2));
  
    TMatrixD Vn(3,3); Vn = (*fitLepn->getCovMatrixFit()); 
    aFitLepn.setResET (Vn(0,0));  
    aFitLepn.setResEta(Vn(1,1));
    aFitLepn.setResPhi(Vn(2,2));
    
    TopJetObject  aFitBottomObj(fitsol.getBottom()); aFitBottomObj.setFitJet(aFitBottom); fitsol.setBottom(aFitBottomObj);
    TopJetObject  aFitLightObj(fitsol.getLight()); aFitLightObj.setFitJet(aFitLight); fitsol.setLight(aFitLightObj);
    if(fitsol.getDecay() == "muon"){
      TopMuonObject aFitLeplObj(fitsol.getMuon()); aFitLeplObj.setFitMuon(aFitLepl); fitsol.setMuon(aFitLeplObj);
    }
    else if(fitsol.getDecay() == "electron"){
      TopElectronObject aFitLeplObj(fitsol.getElectron()); aFitLeplObj.setFitElectron(aFitLepl); fitsol.setElectron(aFitLeplObj);
    }
    TopMETObject  aFitLepnObj(fitsol.getMET()); aFitLepnObj.setFitMET(aFitLepn); fitsol.setMET(aFitLepnObj);
    fitsol.setChi2(TMath::Prob(theFitter->getS(), theFitter->getNDF()));
  }
  return fitsol;
}


//
// Setup the fitter
//
void StKinFitterEtEtaPhi::setupFitter() {
  
  theFitter = new TKinFitter("TtFit", "TtFit");

  TMatrixD empty3(3,3);
  fitBottom = new TFitParticleEMomDev("Jet1", "Jet1", 0, &empty3);
  fitLight = new TFitParticleEMomDev("Jet2", "Jet2", 0, &empty3);
  fitLepl = new TFitParticleEtEtaPhi("Lepton", "Lepton", 0, &empty3);
  fitLepn = new TFitParticleEtEtaPhi("Neutrino", "Neutrino", 0, &empty3);
  
  //  cons1 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0 , 80.35);
  //  cons1->addParticles1(fitHadp, fitHadq);
  cons1 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0 , 80.35);
  cons1->addParticles1(fitLepl, fitLepn);
  //  cons3 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0, 175.);
  //  cons3->addParticles1(fitHadp, fitHadq, fitHadb);
  cons2 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0, 175.);
  cons2->addParticles1(fitLepl, fitLepn, fitBottom);
  cons3 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0, 0.);
  cons3->addParticle1(fitLepn);

  for(unsigned int i=0; i<constraints.size(); i++){
    if(constraints[i] == 1) theFitter->addConstraint(cons1);
    if(constraints[i] == 2) theFitter->addConstraint(cons2);
    if(constraints[i] == 3) theFitter->addConstraint(cons3);
    //    if(constraints[i] == 4) theFitter->addConstraint(cons4);
    //    if(constraints[i] == 5) theFitter->addConstraint(cons5);
  }
  theFitter->addMeasParticle(fitBottom);
  theFitter->addMeasParticle(fitLight);
  theFitter->addMeasParticle(fitLepl);
  theFitter->addMeasParticle(fitLepn);

  theFitter->setMaxNbIter(maxNrIter);
  theFitter->setMaxDeltaS(maxDeltaS);
  theFitter->setMaxF(maxF);
  theFitter->setVerbosity(0);
  
}
