// former parametrisation used in ORCA
#include "TopQuarkAnalysis/TopKinFitter/interface/TtSemiKinFitterEMom.h"


//
// constructor - read in the fit functions from a root file
//
TtSemiKinFitterEMom::TtSemiKinFitterEMom() {
  maxNrIter = 200;
  maxDeltaS = 5e-5;
  maxF = 1e-4;
  // initialisation
  setupFitter();
}

TtSemiKinFitterEMom::TtSemiKinFitterEMom(int a, double b, double c, vector<int> d) {
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
TtSemiKinFitterEMom::~TtSemiKinFitterEMom() {
  delete cons1; delete cons2; delete cons3; delete cons4; // delete cons5; delete cons6; delete cons7;
  delete fitHadb; delete fitHadp; delete fitHadq; delete fitLepb; delete fitLepl; delete fitLepn;
  delete theFitter;
}


TtSemiEvtSolution TtSemiKinFitterEMom::addKinFitInfo(TtSemiEvtSolution * asol) {
  TtSemiEvtSolution fitsol(*asol);
  

  TMatrixD m1(4,4), m2(4,4), m3(4,4), m4(4,4), m5(3,3), m6(3,3);
  m1.Zero(); m2.Zero(); m3.Zero(); m4.Zero(); m5.Zero(); m6.Zero();
  
  TLorentzVector hadpVec(fitsol.getCalHadp().px(),fitsol.getCalHadp().py(),
                         fitsol.getCalHadp().pz(),fitsol.getCalHadp().energy());
  TLorentzVector hadqVec(fitsol.getCalHadq().px(),fitsol.getCalHadq().py(),
                      	 fitsol.getCalHadq().pz(),fitsol.getCalHadq().energy());
  TLorentzVector hadbVec(fitsol.getCalHadb().px(),fitsol.getCalHadb().py(),
                         fitsol.getCalHadb().pz(),fitsol.getCalHadb().energy());
  TLorentzVector lepbVec(fitsol.getCalLepb().px(),fitsol.getCalLepb().py(),
                         fitsol.getCalLepb().pz(),fitsol.getCalLepb().energy());
  TLorentzVector leplVec;
  if(fitsol.getDecay()== "electron") leplVec = TLorentzVector(fitsol.getCalLepe().px(), fitsol.getCalLepe().py(),    
			 				      fitsol.getCalLepe().pz(), fitsol.getCalLepe().energy());
  if(fitsol.getDecay()== "muon")     leplVec = TLorentzVector(fitsol.getCalLepm().px(), fitsol.getCalLepm().py(),    
			 				      fitsol.getCalLepm().pz(), fitsol.getCalLepm().energy());
  TLorentzVector lepnVec(fitsol.getCalLepn().px(), fitsol.getCalLepn().py(),
			 0, fitsol.getCalLepn().et());
 
    
    
// jet resolutions (covM in vector<double> form -> (0,0)=[0], (1,1)=[4], (2,2)=[8])
  m1(0,0) = pow(fitsol.getCalHadp().getResPinv(),  2);
  m1(1,1) = pow(fitsol.getCalHadp().getResTheta(), 2);
  m1(2,2) = pow(fitsol.getCalHadp().getResPhi(), 2);
  m1(3,3) = pow(fitsol.getCalHadp().getResD(), 2);
  m2(0,0) = pow(fitsol.getCalHadq().getResPinv(),  2); 
  m2(1,1) = pow(fitsol.getCalHadq().getResTheta(), 2); 
  m2(2,2) = pow(fitsol.getCalHadq().getResPhi(), 2);
  m2(3,3) = pow(fitsol.getCalHadq().getResD(), 2);
  m3(0,0) = pow(fitsol.getCalHadb().getResPinv(),  2); 
  m3(1,1) = pow(fitsol.getCalHadb().getResTheta(), 2); 
  m3(2,2) = pow(fitsol.getCalHadb().getResPhi(), 2);
  m3(3,3) = pow(fitsol.getCalHadb().getResD(), 2);
  m4(0,0) = pow(fitsol.getCalLepb().getResPinv(),  2); 
  m4(1,1) = pow(fitsol.getCalLepb().getResTheta(), 2); 
  m4(2,2) = pow(fitsol.getCalLepb().getResPhi(), 2);
  m4(3,3) = pow(fitsol.getCalLepb().getResD(), 2);
  if(fitsol.getDecay()== "electron"){
    m5(0,0) = pow(fitsol.getCalLepe().getResPinv(),  2);
    m5(1,1) = pow(fitsol.getCalLepe().getResTheta(), 2); 
    m5(2,2) = pow(fitsol.getCalLepe().getResPhi(), 2);
  }
  if(fitsol.getDecay()== "muon"){
    m5(0,0) = pow(fitsol.getCalLepm().getResPinv(),  2);
    m5(1,1) = pow(fitsol.getCalLepm().getResTheta(), 2); 
    m5(2,2) = pow(fitsol.getCalLepm().getResPhi(), 2);
  }
  m6(0,0) = pow(fitsol.getCalLepn().getResPinv(),  2);
  m6(1,1) = pow(fitsol.getCalLepn().getResTheta(),  2);
  m6(2,2) = pow(fitsol.getCalLepn().getResPhi(), 2);
  
  fitHadp->setIni4Vec(&hadpVec); fitHadp->setCovMatrix(&m1);
  fitHadq->setIni4Vec(&hadqVec); fitHadq->setCovMatrix(&m2);
  fitHadb->setIni4Vec(&hadbVec); fitHadb->setCovMatrix(&m3);
  fitLepb->setIni4Vec(&lepbVec); fitLepb->setCovMatrix(&m4);
  fitLepl->setIni4Vec(&leplVec); fitLepl->setCovMatrix(&m5);
  fitLepn->setIni4Vec(&lepnVec); fitLepn->setCovMatrix(&m6);

  theFitter->fit();
  
  // add fitted information to the solution
  if ( theFitter->getStatus() == 0 ) {
    TopParticle aFitHadp(reco::Particle(0,math::XYZTLorentzVector(fitHadp->getCurr4Vec()->X(), fitHadp->getCurr4Vec()->Y(), fitHadp->getCurr4Vec()->Z(), fitHadp->getCurr4Vec()->E()),math::XYZPoint()));
    TopParticle aFitHadq(reco::Particle(0,math::XYZTLorentzVector(fitHadq->getCurr4Vec()->X(), fitHadq->getCurr4Vec()->Y(), fitHadq->getCurr4Vec()->Z(), fitHadq->getCurr4Vec()->E()),math::XYZPoint()));
    TopParticle aFitHadb(reco::Particle(0,math::XYZTLorentzVector(fitHadb->getCurr4Vec()->X(), fitHadb->getCurr4Vec()->Y(), fitHadb->getCurr4Vec()->Z(), fitHadb->getCurr4Vec()->E()),math::XYZPoint()));
    TopParticle aFitLepb(reco::Particle(0,math::XYZTLorentzVector(fitLepb->getCurr4Vec()->X(), fitLepb->getCurr4Vec()->Y(), fitLepb->getCurr4Vec()->Z(), fitLepb->getCurr4Vec()->E()),math::XYZPoint()));
    TopParticle aFitLepl(reco::Particle(0,math::XYZTLorentzVector(fitLepl->getCurr4Vec()->X(), fitLepl->getCurr4Vec()->Y(), fitLepl->getCurr4Vec()->Z(), fitLepl->getCurr4Vec()->E()),math::XYZPoint()));
    TopParticle aFitLepn(reco::Particle(0,math::XYZTLorentzVector(fitLepn->getCurr4Vec()->X(), fitLepn->getCurr4Vec()->Y(), fitLepn->getCurr4Vec()->Z(), fitLepn->getCurr4Vec()->E()),math::XYZPoint()));   

    TMatrixD Vp(4,4);  Vp  = (*fitHadp->getCovMatrixFit()); 
    TMatrixD Vq(4,4);  Vq  = (*fitHadq->getCovMatrixFit()); 
    TMatrixD Vbh(4,4); Vbh = (*fitHadb->getCovMatrixFit()); 
    TMatrixD Vbl(4,4); Vbl = (*fitLepb->getCovMatrixFit()); 
    aFitHadp.setCovM(this->translateCovM(Vp));
    aFitHadq.setCovM(this->translateCovM(Vq));
    aFitHadb.setCovM(this->translateCovM(Vbh));
    aFitLepb.setCovM(this->translateCovM(Vbl));
    aFitHadp.setResPinv (sqrt(Vp(0,0)));  
    aFitHadp.setResTheta(sqrt(Vp(1,1)));
    aFitHadp.setResPhi(sqrt(Vp(2,2))); 
    aFitHadp.setResD(sqrt(Vp(3,3))); 
    aFitHadq.setResPinv (sqrt(Vq(0,0)));  
    aFitHadq.setResTheta(sqrt(Vq(1,1)));
    aFitHadq.setResPhi(sqrt(Vq(2,2)));
    aFitHadq.setResD(sqrt(Vq(3,3)));
    aFitHadb.setResPinv (sqrt(Vbh(0,0)));  
    aFitHadb.setResTheta(sqrt(Vbh(1,1)));
    aFitHadb.setResPhi(sqrt(Vbh(2,2)));
    aFitHadb.setResD(sqrt(Vbh(3,3)));
    aFitLepb.setResPinv (sqrt(Vbl(0,0)));  
    aFitLepb.setResTheta(sqrt(Vbl(1,1)));
    aFitLepb.setResPhi(sqrt(Vbl(2,2)));
    aFitLepb.setResD(sqrt(Vbl(3,3)));
    
    TMatrixD Vl(3,3); Vl = (*fitLepl->getCovMatrixFit()); 
    aFitLepl.setCovM(this->translateCovM(Vl));
    aFitLepl.setResPinv (Vl(0,0));  
    aFitLepl.setResTheta(Vl(1,1));
    aFitLepl.setResPhi(Vl(2,2));
    
    TMatrixD Vn(3,3); Vn = (*fitLepn->getCovMatrixFit()); 
    aFitLepn.setCovM(this->translateCovM(Vn));
    aFitLepn.setResPinv (Vn(0,0));  
    aFitLepn.setResTheta(Vn(1,1));
    aFitLepn.setResPhi(Vn(2,2));
    
    fitsol.setFitHadb(aFitHadb);
    fitsol.setFitHadp(aFitHadp);
    fitsol.setFitHadq(aFitHadq);
    fitsol.setFitLepb(aFitLepb);
    fitsol.setFitLepl(aFitLepl);
    fitsol.setFitLepn(aFitLepn);

    fitsol.setProbChi2(TMath::Prob(theFitter->getS(), theFitter->getNDF()));

  }
  return fitsol;
}


//
// Setup the fitter
//
void TtSemiKinFitterEMom::setupFitter() {
  
  theFitter = new TKinFitter("TtFit", "TtFit");

  TMatrixD empty3(3,3); TMatrixD empty4(4,4);
  fitHadb = new TFitParticleEMomDev("Jet1", "Jet1", 0, &empty4);
  fitHadp = new TFitParticleEMomDev("Jet2", "Jet2", 0, &empty4);
  fitHadq = new TFitParticleEMomDev("Jet3", "Jet3", 0, &empty4);
  fitLepb = new TFitParticleEMomDev("Jet4", "Jet4", 0, &empty4);
  fitLepl = new TFitParticleEScaledMomDev("Lepton", "Lepton", 0, &empty3);
  fitLepn = new TFitParticleEScaledMomDev("Neutrino", "Neutrino", 0, &empty3);
  
  cons1 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0 , 80.35);
  cons1->addParticles1(fitHadp, fitHadq);
  cons2 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0 , 80.35);
  cons2->addParticles1(fitLepl, fitLepn);
  cons3 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0, 175.);
  cons3->addParticles1(fitHadp, fitHadq, fitHadb);
  cons4 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0, 175.);
  cons4->addParticles1(fitLepl, fitLepn, fitLepb);
  cons5 = new TFitConstraintM("MassConstraint", "Mass-Constraint", 0, 0, 0.);
  cons5->addParticle1(fitLepn);

  for(unsigned int i=0; i<constraints.size(); i++){
    if(constraints[i] == 1) theFitter->addConstraint(cons1);
    if(constraints[i] == 2) theFitter->addConstraint(cons2);
    if(constraints[i] == 3) theFitter->addConstraint(cons3);
    if(constraints[i] == 4) theFitter->addConstraint(cons4);
    if(constraints[i] == 5) theFitter->addConstraint(cons5);
  }
  theFitter->addMeasParticle(fitHadb);
  theFitter->addMeasParticle(fitHadp);
  theFitter->addMeasParticle(fitHadq);
  theFitter->addMeasParticle(fitLepb);
  theFitter->addMeasParticle(fitLepl);
  theFitter->addMeasParticle(fitLepn);

  theFitter->setMaxNbIter(maxNrIter);
  theFitter->setMaxDeltaS(maxDeltaS);
  theFitter->setMaxF(maxF);
  theFitter->setVerbosity(0);
  
}

vector<double> TtSemiKinFitterEMom::translateCovM(TMatrixD &V){
  vector<double> covM; 
  for(int ii=0; ii<V.GetNrows(); ii++){
    for(int jj=0; jj<V.GetNcols(); jj++) covM.push_back(V(ii,jj));
  }
  return covM;
}
