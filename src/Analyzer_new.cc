#include "CalorimeterHitType.h"
#include <UTIL/BitField64.h>
#include "TInterpreter.h"
#include "Analyzer_new.h"
#include <UTIL/CellIDDecoder.h>
#include <EVENT/LCCollection.h>
#include <EVENT/CalorimeterHit.h>
#include <EVENT/Vertex.h>
#include <EVENT/SimCalorimeterHit.h>
//#include <EVENT/SimTrackerHit.h>
#include <IMPL/SimCalorimeterHitImpl.h>
//#include <IMPL/SimTrackerHitImpl.h>
#include <EVENT/LCRelation.h>
#include <EVENT/Track.h>
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetType.h"
#include "DD4hep/Detector.h"
#include "DDRec/DetectorData.h"
#include "DD4hep/DetectorSelector.h"
#include "UTIL/LCTypedVector.h"
#include <UTIL/LCRelationNavigator.h>
#include <marlinutil/GeometryUtil.h>

#include "TFile.h"

using namespace lcio ;
using namespace marlin ;


Analyzer_new aAnalyzer_new;

Analyzer_new::Analyzer_new() : Processor("Analyzer_new") {
  
  _description = "Analyzer calculates properties of muons" ;

  registerInputCollection( LCIO::MCPARTICLE,
			   "MCParticleCollectionName",
			   "Name of the MCParticle input collection",
			   m_inputMCParticleCollection,
			   std::string("MCPhysicsParticles"));
  
  registerProcessorParameter( "OutputRootFileName",
			      "ROOT File name to collect plots",
			      m_rootFileName,
			      std::string("MuonAnalyzer.root"));
  
  /*registerInputCollection( LCIO::RECONSTRUCTEDPARTICLE,
			   "RECOParticleCollectionName",
			   "Name of the RECOParticle input collection",
			   m_inputRECOParticleCollection,
			   std::string("PandoraPFOs"));
  
  registerInputCollection( LCIO::CLUSTER,
			   "RECOClusterCollectionName",
			   "Name of the RECOCluster input collection",
			   m_inputRECOClusterCollection,
			   std::string("PandoraClusters"));*/

  registerInputCollection( LCIO::TRACK,
			   "CTCollectionName",
			   "Name of the CT input collection",
			   m_inputCTCollection,
			   std::string("CATracks"));
  
  registerInputCollection( LCIO::CLUSTER,
			   "StandAloneClusterCollectionName",
			   "Name of the StandAloneCluster (muon hits) output collection",
			   m_inputClusterCollection,
			   std::string("StandAloneCluster"));
  
  registerInputCollection( LCIO::CALORIMETERHIT,
			   "MUON" ,  
			   "Name of the MUON Digi collection",
			   m_muonColName,
			   std::string("MUON"));
}  


void Analyzer_new::init() {

  m_rootFile = new TFile(m_rootFileName.c_str(),"RECREATE");
  gInterpreter->GenerateDictionary("vector<vector<float> >", "vector");
  gInterpreter->GenerateDictionary("vector<vector<int> >", "vector");
  m_outputTree = new TTree("muonData","muonData");

  // Print the initial parameters
  printParameters() ;
  
  // Reset counters
  m_runNumber = 0 ;
  m_eventNumber = 0 ;
  _magneticField = MarlinUtil::getBzAtOrigin();

  //CLUSTER-->PandoraCluster
  /*m_E_pfoclus	= new std::vector<float>(); 
  m_x_pfoclus	= new std::vector<float>(); 
  m_y_pfoclus	= new std::vector<float>(); 
  m_z_pfoclus	= new std::vector<float>(); 
  m_Eecal_pfoclus= new std::vector<float>();  
  m_Ehcal_pfoclus= new std::vector<float>();  
  m_Eyoke_pfoclus= new std::vector<float>();
  m_nhitstot_pfoclus= new std::vector<int>();
  m_nhitsmuon_pfoclus= new std::vector<int>();
  m_phi_pfoclus	= new std::vector<float>(); 
  m_theta_pfoclus = new std::vector<float>(); 
  
  //RECONSTRUCTEDPARTICLES-->PandoraPFO
  m_type_pfopart= new std::vector<float>(); 
  m_E_pfopart	= new std::vector<float>(); 
  m_px_pfopart	= new std::vector<float>(); 
  m_py_pfopart	= new std::vector<float>(); 
  m_pz_pfopart	= new std::vector<float>(); 
  m_dR_pfopart	= new std::vector<float>(); 
  m_flag_pfopart = new std::vector<int>();*/
  
  //MCPARTICLE  
  m_mcpPDGID = new std::vector<int>(); 
  m_mcpE   = new std::vector<float>(); 
  m_mcpPx  = new std::vector<float>(); 
  m_mcpPy  = new std::vector<float>(); 
  m_mcpPz  = new std::vector<float>(); 
  m_mcpTime  = new std::vector<float>();

  //STAND ALONE CLUSTER
  m_E_SAclus    = new std::vector<float>(); 
  m_x_SAclus	= new std::vector<float>(); 
  m_y_SAclus	= new std::vector<float>(); 
  m_z_SAclus	= new std::vector<float>();
  m_nhitstot_SAclus= new std::vector<int>();
  m_phi_SAclus	= new std::vector<float>(); 
  m_theta_SAclus= new std::vector<float>(); 

  //TRACK
  m_trkPt = new std::vector<float>();
  m_trkTheta = new std::vector<float>();
  m_trkPhi = new std::vector<float>();
  m_trkCharge = new std::vector<float>();
  
  //MUON
  m_Muon_E   = new std::vector<float>(); 
  m_Muon_x  = new std::vector<float>(); 
  m_Muon_y  = new std::vector<float>(); 
  m_Muon_z  = new std::vector<float>(); 
  m_Muon_time  = new std::vector<float>(); 
  m_Muon_ID_layer    = new std::vector<int>(); 
  m_Muon_ID_system   = new std::vector<int>(); 
  m_Muon_ID_module   = new std::vector<int>(); 
  m_Muon_ID_stave    = new std::vector<int>(); 
  m_Muon_ID_submodule= new std::vector<int>(); 
  m_Muon_ID_side     = new std::vector<int>(); 
  m_Muon_ID_x	   = new std::vector<int>(); 
  m_Muon_ID_y	   = new std::vector<int>(); 
  
  
  //Initialization
  m_E_SAclus->clear();   
  m_x_SAclus->clear(); 
  m_y_SAclus->clear(); 
  m_z_SAclus->clear(); 
  m_nhitstot_SAclus->clear();
  m_phi_SAclus->clear(); 	
  m_theta_SAclus->clear();
  
  m_clus_layer.clear();
  m_clus_system.clear();
  m_clus_side.clear();
  m_clus_module.clear();
  m_clus_stave.clear();
  m_clus_submodule.clear();
  m_clus_cellx.clear();
  m_clus_celly.clear();
  m_clus_time.clear();
  m_clus_energy.clear();
  m_clus_x.clear();
  m_clus_y.clear();
  m_clus_z.clear();

  m_trkPt->clear(); 
  m_trkTheta->clear(); 
  m_trkPhi->clear(); 
  m_trkCharge->clear();
  
  m_mcpPDGID ->clear(); 
  m_mcpE   ->clear(); 
  m_mcpPx  ->clear(); 
  m_mcpPy  ->clear(); 
  m_mcpPz  ->clear(); 
  m_mcpTime  ->clear();
  
  /*m_E_pfoclus->clear(); 
  m_x_pfoclus->clear(); 
  m_y_pfoclus->clear(); 
  m_z_pfoclus->clear(); 
  m_phi_pfoclus->clear();
  m_theta_pfoclus->clear();
  m_Eecal_pfoclus->clear(); 
  m_Ehcal_pfoclus->clear(); 
  m_Eyoke_pfoclus->clear(); 
  m_nhitstot_pfoclus->clear(); 
 
  m_type_pfopart->clear(); 
  m_E_pfopart->clear(); 
  m_px_pfopart->clear(); 
  m_py_pfopart->clear(); 
  m_pz_pfopart->clear();
  m_dR_pfopart->clear();
  m_flag_pfopart->clear();*/
  
  m_Muon_E ->clear(); 
  m_Muon_x ->clear(); 
  m_Muon_y ->clear(); 
  m_Muon_z ->clear();
  m_Muon_time ->clear(); 
  m_Muon_ID_layer ->clear();   
  m_Muon_ID_system ->clear(); 
  m_Muon_ID_module ->clear();  
  m_Muon_ID_stave ->clear();   
  m_Muon_ID_submodule ->clear();
  m_Muon_ID_side ->clear();   
  m_Muon_ID_x->clear();	 
  m_Muon_ID_y->clear();	 

  // Branch creation  
  m_outputTree->Branch("mcpPDGID","std::vector< int >", &m_mcpPDGID);
  m_outputTree->Branch("mcpE","std::vector< float >", &m_mcpE);
  m_outputTree->Branch("mcpPx","std::vector< float >", &m_mcpPx);
  m_outputTree->Branch("mcpPy","std::vector< float >", &m_mcpPy);
  m_outputTree->Branch("mcpPz","std::vector< float >", &m_mcpPz);
  m_outputTree->Branch("mcpTime","std::vector< float >", &m_mcpTime);
  
  /*m_outputTree->Branch("E_pfoclus","std::vector<float>",&m_E_pfoclus);
  m_outputTree->Branch("x_pfoclus","std::vector<float>",&m_x_pfoclus);
  m_outputTree->Branch("y_pfoclus","std::vector<float>",&m_y_pfoclus);
  m_outputTree->Branch("z_pfoclus","std::vector<float>",&m_z_pfoclus);
  m_outputTree->Branch("Eecal_pfoclus","std::vector<float>",&m_Eecal_pfoclus);
  m_outputTree->Branch("Ehcal_pfoclus","std::vector<float>",&m_Ehcal_pfoclus);
  m_outputTree->Branch("Eyoke_pfoclus","std::vector<float>",&m_Eyoke_pfoclus);
  m_outputTree->Branch("nhitstot_pfoclus","std::vector<int>",&m_nhitstot_pfoclus);
  m_outputTree->Branch("nhitsmuon_pfoclus","std::vector<int>",&m_nhitsmuon_pfoclus);
  m_outputTree->Branch("phi_pfoclus","std::vector<float>",&m_phi_pfoclus);
  m_outputTree->Branch("theta_pfoclus","std::vector<float>",&m_theta_pfoclus);
  
  m_outputTree->Branch("E_pfopart","std::vector<float>",&m_E_pfopart);
  m_outputTree->Branch("px_pfopart","std::vector<float>",&m_px_pfopart);
  m_outputTree->Branch("py_pfopart","std::vector<float>",&m_py_pfopart);
  m_outputTree->Branch("pz_pfopart","std::vector<float>",&m_pz_pfopart);
  m_outputTree->Branch("type_pfopart","std::vector<float>",&m_type_pfopart);
  m_outputTree->Branch("dR_pfopart","std::vector<float>",&m_dR_pfopart);
  m_outputTree->Branch("flag_pfopart","std::vector<int>",&m_flag_pfopart);*/
    
  m_outputTree->Branch("m_E_SAclus","std::vector<float>",&m_E_SAclus); 
  m_outputTree->Branch("m_x_SAclus","std::vector<float>",&m_x_SAclus); 
  m_outputTree->Branch("m_y_SAclus","std::vector<float>",&m_y_SAclus); 
  m_outputTree->Branch("m_z_SAclus","std::vector<float>",&m_z_SAclus);
  m_outputTree->Branch("m_nhitstot_SAclus","std::vector<int>",&m_nhitstot_SAclus);
  m_outputTree->Branch("m_phi_SAclus","std::vector<float>",&m_phi_SAclus); 
  m_outputTree->Branch("m_theta_SAclus","std::vector<float>",&m_theta_SAclus); 

  m_outputTree->Branch("m_trkPt","std::vector<float>",&m_trkPt);
  m_outputTree->Branch("m_trkTheta","std::vector<float>",&m_trkTheta);
  m_outputTree->Branch("m_trkPhi","std::vector<float>",&m_trkPhi);
  m_outputTree->Branch("m_trkCharge","std::vector<float>",&m_trkCharge);
  
  m_outputTree->Branch("clus_layer","std::vector< std::vector<int >>", &m_clus_layer);
  m_outputTree->Branch("clus_system","std::vector< std::vector<int >>", &m_clus_system);
  m_outputTree->Branch("clus_side","std::vector< std::vector<int >>", &m_clus_side);
  m_outputTree->Branch("clus_module","std::vector< std::vector<int >>", &m_clus_module);
  m_outputTree->Branch("clus_stave","std::vector< std::vector<int >>", &m_clus_stave);
  m_outputTree->Branch("clus_submodule","std::vector< std::vector<int >>", &m_clus_submodule);
  m_outputTree->Branch("clus_cellx","std::vector< std::vector<int >>", &m_clus_cellx);
  m_outputTree->Branch("clus_celly","std::vector< std::vector<int >>", &m_clus_celly);
  m_outputTree->Branch("clus_time","std::vector< std::vector<float>>", &m_clus_time);
  m_outputTree->Branch("clus_energy","std::vector< std::vector<float>>", &m_clus_energy);
  m_outputTree->Branch("clus_x","std::vector< std::vector<float>>", &m_clus_x);
  m_outputTree->Branch("clus_y","std::vector< std::vector<float>>", &m_clus_y);
  m_outputTree->Branch("clus_z","std::vector< std::vector<float>>", &m_clus_z);
  
  m_outputTree->Branch("MuonE",  "std::vector< float >", &m_Muon_E); 
  m_outputTree->Branch("Muonx", "std::vector< float >", &m_Muon_x); 
  m_outputTree->Branch("Muony", "std::vector< float >", &m_Muon_y); 
  m_outputTree->Branch("Muonz", "std::vector< float >", &m_Muon_z); 
  m_outputTree->Branch("Muontime", "std::vector< float >", &m_Muon_time); 
  m_outputTree->Branch("Muon_ID_layer",  "std::vector< int >",    &m_Muon_ID_layer);
  m_outputTree->Branch("Muon_ID_system",  "std::vector< int >",   &m_Muon_ID_system);  
  m_outputTree->Branch("Muon_ID_module",  "std::vector< int >",    &m_Muon_ID_module);
  m_outputTree->Branch("Muon_ID_submodule",  "std::vector< int >",   &m_Muon_ID_submodule);
  m_outputTree->Branch("Muon_ID_stave",  "std::vector< int >",   &m_Muon_ID_stave);
  m_outputTree->Branch("Muon_ID_side",  "std::vector< int >",   &m_Muon_ID_side);
  m_outputTree->Branch("Muon_ID_x",  "std::vector< int >",   &m_Muon_ID_x);
  m_outputTree->Branch("Muon_ID_y",  "std::vector< int >",   &m_Muon_ID_y);
}


void Analyzer_new::processRunHeader( LCRunHeader*) {
    ++m_runNumber ;
}

void Analyzer_new::processEvent( LCEvent* evt ) {

  m_eventNumber=evt->getEventNumber();
  m_runNumber=evt->getRunNumber();
  
  std::cout << " EVENTO NUMERO " << m_eventNumber << std::endl;
  
  //Initialization for each event
  m_clus_layer.clear();
  m_clus_system.clear();
  m_clus_side.clear();
  m_clus_module.clear();
  m_clus_stave.clear();
  m_clus_submodule.clear();
  m_clus_cellx.clear();
  m_clus_celly.clear();
  m_clus_time.clear();
  m_clus_energy.clear();
  m_clus_x.clear();
  m_clus_y.clear();
  m_clus_z.clear();

  m_E_SAclus->clear();   
  m_x_SAclus->clear(); 
  m_y_SAclus->clear(); 
  m_z_SAclus->clear(); 
  m_nhitstot_SAclus->clear(); 
  m_phi_SAclus->clear(); 	
  m_theta_SAclus->clear(); 

  m_trkPt->clear(); 
  m_trkTheta->clear(); 
  m_trkPhi->clear(); 
  m_trkCharge->clear();
  
  m_mcpPDGID ->clear(); 
  m_mcpE   ->clear(); 
  m_mcpPx  ->clear(); 
  m_mcpPy  ->clear(); 
  m_mcpPz  ->clear(); 
  m_mcpTime  ->clear();
  
  m_Muon_E ->clear(); 
  m_Muon_x ->clear(); 
  m_Muon_y ->clear(); 
  m_Muon_z ->clear();
  m_Muon_time ->clear(); 
  m_Muon_ID_layer ->clear();   
  m_Muon_ID_system ->clear(); 
  m_Muon_ID_module ->clear();  
  m_Muon_ID_stave ->clear();   
  m_Muon_ID_submodule ->clear();
  m_Muon_ID_side ->clear();   
  m_Muon_ID_x->clear();	 
  m_Muon_ID_y->clear();	 

  
  /*m_E_pfoclus->clear(); 
  m_x_pfoclus->clear(); 
  m_y_pfoclus->clear(); 
  m_z_pfoclus->clear(); 
  m_phi_pfoclus->clear();
  m_theta_pfoclus->clear();
  m_Eecal_pfoclus->clear(); 
  m_Ehcal_pfoclus->clear(); 
  m_Eyoke_pfoclus->clear(); 
  m_nhitstot_pfoclus->clear(); 
  m_nhitsmuon_pfoclus->clear(); 
 
  m_type_pfopart->clear(); 
  m_E_pfopart->clear(); 
  m_px_pfopart->clear(); 
  m_py_pfopart->clear(); 
  m_pz_pfopart->clear();
  m_dR_pfopart->clear();
  m_flag_pfopart->clear();*/

  //=================
  //MCParticle

  LCCollection * mcColl =0;
  getCollection(mcColl,m_inputMCParticleCollection,evt);
 
  if(mcColl!=NULL){
    for(int m =0; m< mcColl->getNumberOfElements(); m++){
      MCParticle* mcp= dynamic_cast<MCParticle*>( mcColl->getElementAt(m) ) ; 
      if (mcp->getParents().size() == 0 ){ 
      	m_mcpPDGID->push_back(mcp->getPDG());
      	m_mcpE->push_back(mcp->getEnergy());
      	m_mcpPx->push_back(mcp->getMomentum()[0]);
     	m_mcpPy->push_back(mcp->getMomentum()[1]);
     	m_mcpPz->push_back(mcp->getMomentum()[2]);
      	m_mcpTime->push_back(mcp->getTime());
      }
    }//end loop MCParticle
  }//end if MCparticle
  std::cout << "MCParticle fatte" <<std::endl;
  //=================
  /*LCCollection* muons = NULL;
  try{
    muons = evt->getCollection(m_muonColName) ;
  }
  catch( lcio::DataNotAvailableException &e ){
    streamlog_out(WARNING) <<  m_muonColName << " m_muonColName collection not available" << std::endl;
    muons = NULL;
  }
  */
  LCCollection* muons =0;
  getCollection(muons,m_muonColName,evt);
  //TRACKS
  LCCollection* tracks = NULL;
  getCollection(tracks, m_inputCTCollection, evt);
  
  if(tracks!=NULL){
    for(int i=0;i<tracks->getNumberOfElements();i++){
      Track *cttrack = dynamic_cast<Track *>(tracks->getElementAt(i));
      double pt = 0.3 * _magneticField/ (fabs(cttrack->getTrackState(1)->getOmega())*1000.);
      double theta = (M_PI/2 - atan(cttrack->getTrackState(1)->getTanLambda())) ;
      double phi = cttrack->getTrackState(1)->getPhi() ;
      double charge = cttrack->getTrackState(1)->getOmega();
      m_trkPt->push_back(pt);
      m_trkTheta->push_back(theta);
      m_trkPhi->push_back(phi);
      m_trkCharge->push_back(charge);
    }//end track loop
  }//end if track
    std::cout << "Tracce fatte" <<std::endl;
  
  //RECONSTRUCTEDPARTICLE-->PandoraPFOs
  //Track* track;
  /*Cluster* cluster;
  ReconstructedParticle* pandorapart;
  int traccia=0;
  LCCollection* recoparticlecol = NULL;
  recoparticlecol = evt->getCollection(m_inputRECOParticleCollection) ;  
  
  if(recoparticlecol!=NULL){
    int iclus=-1;
    int flag_pp =-1;
    for(int i=0;i<recoparticlecol->getNumberOfElements();i++){
      pandorapart = dynamic_cast<ReconstructedParticle*>(recoparticlecol->getElementAt(i));
      flag_pp =-1;
      m_type_pfopart->push_back(pandorapart->getType());     		
      m_E_pfopart->push_back(pandorapart->getEnergy());
      m_px_pfopart->push_back(pandorapart->getMomentum()[0]);
      m_py_pfopart->push_back(pandorapart->getMomentum()[1]);
      m_pz_pfopart->push_back(pandorapart->getMomentum()[2]);
      if (pandorapart->getEndVertex()) std::cout << "end vertex "<<pandorapart->getStartVertex()->getPosition()[0] << " "<< pandorapart->getStartVertex()->getPosition()[1] << " "<<pandorapart->getStartVertex()->getPosition()[2] <<std::endl;
      TLorentzVector pp;
      pp.SetPxPyPzE(pandorapart->getMomentum()[0], pandorapart->getMomentum()[1], pandorapart->getMomentum()[2], pandorapart->getEnergy());
      float dR=999999999;
      if(mcColl!=NULL){
	for(int igen=0; igen< mcColl->getNumberOfElements(); igen++){ //loop sulle particelle generate per fare il deltaR
	  MCParticle* imcp= dynamic_cast<MCParticle*>( mcColl->getElementAt(igen));
	  TLorentzVector genp;
	  genp.SetPxPyPzE(imcp->getMomentum()[0], imcp->getMomentum()[1], imcp->getMomentum()[2], imcp->getEnergy());
	  float deltaR = sqrt((pp.Phi()-genp.Phi())*(pp.Phi()-genp.Phi())+(pp.Eta()-genp.Eta())*(pp.Eta()-genp.Eta()));
	  if (deltaR<dR) dR = deltaR;
	}	
      }
      m_dR_pfopart->push_back(dR);
  
     std::cout << "ok1" << std::endl;
      if(pandorapart->getClusters().size() > 0){
	iclus++;
	flag_pp =iclus;
	cluster = dynamic_cast<Cluster*>( pandorapart->getClusters().at(0) ) ;
	std::cout << pandorapart->getClusters().size() << std::endl;      		
	m_E_pfoclus->push_back(cluster->getEnergy());
	m_x_pfoclus->push_back(cluster->getPosition()[0]);
	m_y_pfoclus->push_back(cluster->getPosition()[1]);
	m_z_pfoclus->push_back(cluster->getPosition()[2]);
	m_phi_pfoclus->push_back(cluster->getIPhi());
	m_theta_pfoclus->push_back(cluster->getITheta());
	m_Eecal_pfoclus->push_back(cluster->getSubdetectorEnergies()[0]);
	m_Ehcal_pfoclus->push_back(cluster->getSubdetectorEnergies()[1]);
	m_Eyoke_pfoclus->push_back(cluster->getSubdetectorEnergies()[2]);
	if(cluster->getCalorimeterHits().size()>0) std::cout << "ok" << std::endl; 
	m_nhitstot_pfoclus->push_back(cluster->getCalorimeterHits().size());
		
	int nhits_muon =0 ;
	if (muons!=NULL){
	  UTIL::BitField64 cluster_b(muons->getParameters().getStringVal( EVENT::LCIO::CellIDEncoding ));
	  for(unsigned int ch=0; ch < cluster->getCalorimeterHits().size(); ch++){
	    std::cout << "prova1" << std::endl;
	    CalorimeterHit* clusterhit = dynamic_cast<CalorimeterHit*>(cluster->getCalorimeterHits()[ch]);
	    //std::cout << "prova2" << std::endl;
	    const CHT cluster_cht(cluster->getCalorimeterHits()[ch]->getType());
	    std::cout << "prova3" << std::endl;
	    if ( cluster_cht.caloType() ==2){ //caloType=2 means the hit is in muon
	      nhits_muon++;
	      //QUESTI DANNO IL PROBLEMA DI MEMORIA
	    }
           std::cout << "prova4" << std::endl;
	  }
	  m_nhitsmuon_pfoclus->push_back(nhits_muon);
	}
      } //end if cluster size >0
	
      m_flag_pfopart->push_back(flag_pp);
      
    }//end PandoraParticles loop
  } //end if  PandoraParticles loop
      std::cout << "Pandora fatte" <<std::endl;
  */
  //=================
      //==================

  //Clusters

  LCCollection* clusters = NULL;
  getCollection(clusters,m_inputClusterCollection, evt);
  
  if(clusters!=NULL){

    for(int i=0;i<clusters->getNumberOfElements();i++){
      Cluster *clus = dynamic_cast<Cluster*>( clusters->getElementAt(i) ) ;
      m_E_SAclus->push_back(clus->getEnergy());
      m_x_SAclus->push_back(clus->getPosition()[0]);
      m_y_SAclus->push_back(clus->getPosition()[1]);
      m_z_SAclus->push_back(clus->getPosition()[2]);
      m_phi_SAclus->push_back(clus->getIPhi());
      m_theta_SAclus->push_back(clus->getITheta());
      m_nhitstot_SAclus->push_back(clus->getCalorimeterHits().size());

      m_clus_layer.push_back(std::vector<int>());
      m_clus_system.push_back(std::vector<int>());
      m_clus_side.push_back(std::vector<int>());
      m_clus_module.push_back(std::vector<int>());
      m_clus_stave.push_back(std::vector<int>());
      m_clus_submodule.push_back(std::vector<int>());
      m_clus_cellx.push_back(std::vector<int>());
      m_clus_celly.push_back(std::vector<int>());
      m_clus_time.push_back(std::vector<float>());
      m_clus_energy.push_back(std::vector<float>());
      m_clus_x.push_back(std::vector<float>());
      m_clus_y.push_back(std::vector<float>());
      m_clus_z.push_back(std::vector<float>());

      int nhits_muon =0 ;
      if (muons!=NULL){
	UTIL::BitField64 cluster_b(muons->getParameters().getStringVal( EVENT::LCIO::CellIDEncoding ));
	for(unsigned int ch=0; ch < clus->getCalorimeterHits().size(); ch++){
	  CalorimeterHit* clusterhit = dynamic_cast<CalorimeterHit*>(clus->getCalorimeterHits()[ch]);
	  const CHT cluster_cht(clusterhit->getType());
	  if ( cluster_cht.caloType() ==2){ //caloType=2 means the hit is in muon
	    lcio::long64 cval = lcio::long64( clusterhit->getCellID0() & 0xffffffff )
	      |        ( lcio::long64( clusterhit->getCellID1() ) << 32) ;
	    //BitField64 b("system:5,side:-2,module:8,stave:4,layer:9,submodule:4,x:32:-16,y:-16" ) ;
	    cluster_b.setValue(cval);
	    unsigned int clayer = cluster_b["layer"]  ;
	    unsigned int csystem = cluster_b["system"]  ;
	    unsigned int cside = cluster_b["side"]  ;
	    unsigned int cmodule = cluster_b["module"]  ;
	    unsigned int cstave = cluster_b["stave"]  ;
	    unsigned int csubmodule = cluster_b["submodule"]  ;
	    unsigned int cx = cluster_b["x"]  ;
	    unsigned int cy = cluster_b["y"]  ;
	    nhits_muon++;
	    //QUESTI DANNO IL PROBLEMA DI MEMORIA
	    m_clus_layer[i].push_back(clayer);
	    m_clus_system[i].push_back(csystem);
	    m_clus_side[i].push_back(cside);
	    m_clus_module[i].push_back(cmodule);
	    m_clus_stave[i].push_back(cstave);
	    m_clus_submodule[i].push_back(csubmodule);
	    m_clus_cellx[i].push_back(cx);
	    m_clus_celly[i].push_back(cy);
	    m_clus_time[i].push_back(clusterhit->getTime());
	    m_clus_energy[i].push_back(clusterhit->getEnergy());
	    m_clus_x[i].push_back(clusterhit->getPosition()[0]);
	    m_clus_y[i].push_back(clusterhit->getPosition()[1]);
	    m_clus_z[i].push_back(clusterhit->getPosition()[2]);
	    //std::cout << csystem << " "<<ch << " "<< clusterhit->getPosition()[0]<< " "<<clusterhit->getPosition()[1]<< " "<<clusterhit->getPosition()[2]<< std::endl;
	  }
	}
	//m_nhitsmuon_pfoclus->push_back(nhits_muon);
      }
      
    }
    
    
     if(muons!=NULL){
     UTIL::BitField64 b(muons->getParameters().getStringVal( EVENT::LCIO::CellIDEncoding ));
     for(int i=0;i<muons->getNumberOfElements();i++){
 	 CalorimeterHit* muon = dynamic_cast<CalorimeterHit*>(muons->getElementAt(i));
	 const CHT cht(muon->getType());
	 lcio::long64 val = lcio::long64( muon->getCellID0() & 0xffffffff ) 
	 |	  ( lcio::long64( muon->getCellID1() ) << 32) ;
	 //BitField64 b("system:5,side:-2,module:8,stave:4,layer:9,submodule:4,x:32:-16,y:-16" ) ;
    	 b.setValue(val);
     	 unsigned int layer = b["layer"]  ;
	 unsigned int system = b["system"]  ;
	 unsigned int side = b["side"]  ;
	 unsigned int module = b["module"]  ;
	 unsigned int stave = b["stave"]  ;
	 unsigned int submodule = b["submodule"]  ;
	 unsigned int x = b["x"]  ;
	 unsigned int y = b["y"]  ;
         m_Muon_E->push_back(muon->getEnergy());
	 m_Muon_x->push_back(muon->getPosition()[0]);
	 m_Muon_y->push_back(muon->getPosition()[1]);
	 m_Muon_z->push_back(muon->getPosition()[2]);
	 m_Muon_time->push_back(muon->getTime());
	 m_Muon_ID_layer->push_back(layer);
	 m_Muon_ID_system->push_back(system);
	 m_Muon_ID_side->push_back(side);
	 m_Muon_ID_module->push_back(module);
	 m_Muon_ID_stave->push_back(stave);
	 m_Muon_ID_submodule->push_back(submodule);
	 m_Muon_ID_x->push_back(x);
	 m_Muon_ID_y->push_back(y);
       }
     }
    
  } 

  m_outputTree->Fill();
} //end void MuonAnalyzer ProcessEvent

void Analyzer_new::getCollection(LCCollection* &collection, std::string collectionName, LCEvent* evt){
  try{
    collection = evt->getCollection( collectionName ) ;
  }
  catch(DataNotAvailableException &e){
    streamlog_out( DEBUG5 )<<"- cannot get collection. Collection " << collectionName.c_str() << " is unavailable" << std::endl;
    return;
  }
  return;
}

void Analyzer_new::check(LCEvent*){
}

void Analyzer_new::end(){
  
  m_rootFile->cd();  
  m_rootFile->Write();
  m_rootFile->Close();

}

