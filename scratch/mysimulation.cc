#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"

#include "ns3/energy-module.h"
#include "ns3/li-ion-energy-source-helper.h"

#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-apps-module.h"

#include <ns3/propagation-loss-model.h>
#include <ns3/lr-wpan-net-device.h>
#include <ns3/spectrum-value.h>
#include <ns3/lr-wpan-spectrum-value-helper.h>
#include <ns3/lr-wpan-mac.h>
#include <ns3/node.h>
#include <ns3/net-device.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/cost231-propagation-loss-model.h>

#include <stdio.h>
#include <time.h>





using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("Dominating-sets-simulation");

		  NodeContainer all ;
		  NodeContainer CDS;
		  NodeContainer sensors2;
		  NodeContainer ueResponders;


		  std::map<int,double> myrequests;
		  std::list<int> serviceTypes;
		  uint32_t mcs = 10;
		  uint32_t rbSize = 2;
		  uint32_t ktrp = 2;
		  uint32_t pscchLength = 8;
		  std::string period="sf40";
		  double dist = 10;
		  double ueTxPower = 8.0;

		  bool verbose = false;


		  double Prss = -80;
		  double offset = 81;
		  int nWifis = 50;

		  int nDS = 50;
		  uint32_t ueCount = nDS;
		  int nLR = 100;
		  std::list<int> CDS_list;


		  uint32_t port=9;
	  	  double TotalTime = 200.0;
		  std::string rate ("2048bps");
		  std::string phyMode ("DsssRate11Mbps");
		  std::string tr_name ("manet-routing-compare");
		  int nodeSpeed = 1; //in m/s
		  int nodePause = 0; //in s
		  std::string m_protocolName = "protocol";
		  double txp =2.0;

		  uint32_t Rid1;
		  uint32_t Rid2;
	  	  int numberhops = 0;
	  	  int numberrequest = 0;
	  	  int numberdelevery = 0;
	  	  int counthops = 0;
	  	  int numberCDS = 0;
	 	  int MaxNCDS = 0;
	 	  int count = 0;
	 	  int sum = 0;
	 	  std::list<int> deplitednodes;
	 	  double transmissiondelay=0;
	 	  std::map<double,std::list<double>> globaltable;
		  double coverage_percent = 0;
		  ApplicationContainer clientApps;
		  ApplicationContainer serverApps;
		  ApplicationContainer serverApps2;
		  ApplicationContainer lr_apps;
		  EnergySourceContainer sources;
		  DeviceEnergyModelContainer deviceModels;
		  double connectivity_percent = 0;
		  double nbcycles = 0;
		  double initEnergy = 20.0;
		  double first_noeud_to_die = 0;

		  Ptr<Socket>
		  SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
		  {
			TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
			Ptr<Socket> sink = Socket::CreateSocket (node, tid);
			InetSocketAddress local = InetSocketAddress (addr, port);
			sink->Bind (local);
			return sink;
		  }
		  void
			  IntTrace (uint32_t oldValue, uint32_t newValue)

			  {

				  if (oldValue != newValue ){


				  if (newValue > 0){
					  numberCDS++;
					  if (numberCDS> MaxNCDS)
						MaxNCDS = numberCDS;

				  }
				  else
					  numberCDS--;

			  }
			  }
		  void TraceHops (uint32_t oldValue, uint32_t newValue){
						  if (oldValue != newValue )
						  if (newValue > 0){
							 numberhops+=newValue;
							 counthops++;
					  }
			  }

		  void TraceRequests (uint32_t oldValue, uint32_t newValue){
					  if (oldValue != newValue )
					  if (newValue > 0){
						 numberrequest++;

				  }
		  }

	  	  void TracePackets (uint32_t oldValue, uint32_t newValue){

	  	 			  if (oldValue != newValue )
	  	 			  if (newValue > 0){
	  	 				 numberdelevery++;
	  	 		  }
	  	  }

	  	  double TracePacketstart (uint32_t packetid, double sent_time){
	  		  std::pair<int,double> entry;
	  		  entry.first= packetid;
	  		  entry.second=sent_time;
	  		  	  myrequests.insert(entry);

	  	 			return sent_time;
	  	  }

	  	 double TracePacketsDate (uint32_t paketid, double receive_time){
	  		std::map<int, double>::iterator it;
	  		for ( it = myrequests.begin(); it != myrequests.end(); it++ ){

	  			   if(it->first == paketid){
	  				 transmissiondelay += receive_time - it->second;
	  				   break;

	  			   }
	  		   }
	  		  	 			return receive_time;
	  		  	  }


void counterf(){
	sum +=numberCDS;
	count++;
	std::cout<<"number of cds now is "<<numberCDS<<" number of cycles is"<<count<<"sum is "<< sum<<std::endl;
	Simulator:: Schedule (Seconds(10.0), &counterf);
}

void DepletionHandler(void){
	 NS_LOG_INFO ("depletion func");
	if(first_noeud_to_die == 0.0){
		first_noeud_to_die =  Simulator::Now ().GetSeconds ();
		std::cout<<"first node deplited its energy after " <<first_noeud_to_die<<std::endl;
	}
	 for(int i=0;i<sources.GetN();i++){
		 if((std::find(deplitednodes.begin(),deplitednodes.end(),i))==deplitednodes.end()) {
		          double batterylvl = deviceModels.Get(i)->GetTotalEnergyConsumption();
		          if(batterylvl>=initEnergy){
		        	  deplitednodes.push_back(i);
//		        	  Ptr<Node> n = all.Get(i);
//		        	  Ptr<Ipv4> ipv4 = n->GetObject<Ipv4> ();
//		        	  uint32_t ipv4ifIndex = 1;
//		        	  Simulator::Schedule (Seconds (0.1),&Ipv4::SetDown,ipv4, ipv4ifIndex);
//		        	  Ptr<RequestResponseClient> app = DynamicCast<RequestResponseClient>(clientApps.Get(i));
//		        	  Ptr<RequestResponseServer> app2 = DynamicCast<RequestResponseServer>(serverApps.Get(i));
//		        	  Ptr<LRServer> lr_server = DynamicCast<LRServer>(serverApps2.Get(i));
//		        	  Ptr<LRClient> lr_client = DynamicCast<LRClient>(lr_apps.Get(i));
//		        	  app->StopApplication();
//		        	  app->Dispose();
//		        	  app2->StopApplication();
//		        	  app2->Dispose();
//		        	  lr_client->StopApplication();
//		        	  lr_client->Dispose();
//		        	  lr_server->StopApplication();
//		        	  lr_server->Dispose();
		          }
		 }

	 }

	 NS_LOG_INFO (" out of depletion func");
}

void generateTraffic(){

	 NS_LOG_INFO ("generate Traffic func");
			std::cout<<"we are going to generate traffic "<< numberrequest <<std::endl;
			  Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable> ();
			  double rnd_emiter_node = (double)generator->GetInteger(1,nDS-1);
			  uint32_t rnd_receiver_node =(double) generator->GetInteger(nDS,nDS+nLR-1);
			   uint32_t packet_id = generator->GetInteger(0,50000);

			   int x = (int)rnd_emiter_node;
			   std::cout<<"emiter node is  "<<x<<" receiver node is "<<(double)rnd_receiver_node<<
			   					   std::endl;
			   Ptr<RequestResponseClient> app = DynamicCast<RequestResponseClient>(clientApps.Get((double)rnd_emiter_node));
			   std::cout<<"generated a traffic to  "<<rnd_receiver_node<< std::endl;
			   int servicetype = 0;
			   std::vector<int> myVector(serviceTypes.begin(), serviceTypes.end());
			   if(rnd_receiver_node>nDS-1){
				   servicetype = myVector[rnd_receiver_node-nDS];
				   if(servicetype>4)
					   servicetype = 0;
			   }
			   app->Request(rnd_receiver_node,packet_id,servicetype);
			   Simulator:: Schedule (Seconds(1.0), &generateTraffic);


		  }

std::list<double> getNeighbors(std::map<Address,neighbor> table){
		  std::list<double> neighbors;
		  std::map<Address,neighbor>::iterator it;
		  for( it= table.begin();it!=table.end();it++){
			  neighbors.push_back((double)it->second.node_id);
		  }
		  return neighbors;
	  }


	  bool coveredbydevice(std::map<Address, neighbor> table){
	 		  std::map<Address,neighbor>::iterator it;
	 		 		  for( it= table.begin();it!=table.end();it++){
	 		 			  if (it->second.neighbors.empty())
	 		 				  return true;
	 		 		  }
	 		 		  return false;
	 	  }



	  void populateGlobaltable(){
		  std::map<double , std::list<double>>::iterator it;
		  globaltable.clear();
		  double temp = 0;
		  for(int i =0;i<nDS;i++){

			  Ptr<RequestResponseClient> app = DynamicCast<RequestResponseClient>(clientApps.Get(i));
			  globaltable.insert(std::pair<double , std::list<double>>((double)i,getNeighbors(app->MyTable)));
			  if(coveredbydevice(app->MyTable))
				  temp = temp+1;

		  }
		  coverage_percent += (double)((double)temp/ (double)nDS);
		 		  nbcycles += 1;
		  std::cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$ coverage percentage is $$$$$$$$$$$$$$$$$$$$"<< coverage_percent<<std::endl;
		  Simulator:: Schedule (Seconds(1.0), &populateGlobaltable);
	  }



	  void testConnected(){
		  	  	  	  Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable> ();
		 			  double rnd_emiter_node = (double)generator->GetInteger(1,nDS-1);
		 			 Astar aster = Astar(ueResponders,globaltable);
		 			 int percentage=0;
		 			 for(int i =1;i<nDS;i++){
		 				 if(aster.findpath(rnd_emiter_node,(double)i))
		 					percentage = percentage+1;
		 				 else
		 					std::cout<<"################################################## no route found between the two nodes "
		 					<<rnd_emiter_node <<" and "<<i<<std::endl;

		 			 }
		 			connectivity_percent +=  (double)((double)percentage*100/(double)(nDS-1));
	std::cout<<"##################################################percentage of connectivity is "
			<<(double)connectivity_percent<<" and node "
								  <<std::endl;
					  Simulator:: Schedule (Seconds(1.0), &testConnected);
	  }

int
main (int argc, char *argv[])
{

//	 LogComponentEnable ("LteAppClientApplication", LOG_LEVEL_ALL);
//     LogComponentEnable ("Dominating-sets-simulation", LOG_LEVEL_ALL);
//
//	 LogComponentEnable ("Lr6Application", LOG_LEVEL_ALL);
//	LogComponentEnable ("Dominating-sets-simulation", LOG_LEVEL_ALL);
//	LogComponentEnable ("RequestResponseServerApplication", LOG_LEVEL_ALL);
//	LogComponentEnable ("RequestResponseClientApplication", LOG_LEVEL_ALL);
//
//	 LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);
//	 LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_ALL);
//	 LogComponentEnable ("LRServerApplication", LOG_LEVEL_ALL);
//	 LogComponentEnable ("WifiRadioEnergyModel", LOG_LEVEL_ALL);
//	 LogComponentEnable ("LiIonEnergySource", LOG_LEVEL_ALL);

//
//	 LogComponentEnable("NistLteUeMac", LOG_LEVEL_ALL);


	CommandLine cmd;
	cmd.AddValue ("nUE", "Print time progress.", nWifis);
	  cmd.Parse(argc, argv);
	  nDS = nLR= nWifis;
	  time_t seconds;

	  	  seconds = time (NULL);
	  	RngSeedManager::SetSeed (seconds);

	//create nodes and node containers
		all.Create(nWifis);


		for(int i =0;i<nDS;i++){
			CDS.Add(all.Get(i));
			ueResponders.Add(all.Get(i));
		}

		sensors2.Create(nLR);

//set up LTE










		// set the phyisical layer and channel model with propagation delay and pathloss , data rate n controlMode, and transmission power,
		// and ap or adhoc mode,

	 WifiHelper wifi;
	 /* wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

		Config::SetDefault( "ns3::RangePropagationLossModel::MaxRange", DoubleValue( 100.0 ) );
	  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	  YansWifiChannelHelper wifiChannel;
	  wifiChannel.SetPropagationDelay( "ns3::ConstantSpeedPropagationDelayModel" );
	  wifiChannel.AddPropagationLoss(  "ns3::RangePropagationLossModel" );
	  wifiPhy.SetChannel (wifiChannel.Create ());

	  // Add a mac and disable rate control
	 NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
	                                "DataMode",StringValue (phyMode),
	                                "ControlMode",StringValue (phyMode));

	  /*wifiPhy.Set ("TxPowerStart",DoubleValue (txp));
	  wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));*/

	 // wifiMac.SetType ("ns3::AdhocWifiMac");

		wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);

		  /** Wifi PHY **/
		  /***************************************************************************/
		  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
		  wifiPhy.Set ("RxGain", DoubleValue (-10));
		//  wifiPhy.Set ("TxGain", DoubleValue (offset + Prss));
		  wifiPhy.Set ("TxGain", DoubleValue (offset + Prss));
		  wifiPhy.Set ("CcaMode1Threshold", DoubleValue (0.0));
		  /***************************************************************************/

		  /** wifi channel **/
		  YansWifiChannelHelper wifiChannel;
		  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
		  wifiChannel.AddPropagationLoss ("ns3::Cost231PropagationLossModel");
		  // create wifi channel
		  Ptr<YansWifiChannel> wifiChannelPtr = wifiChannel.Create ();
		  wifiPhy.SetChannel (wifiChannelPtr);

		  /** MAC layer **/
		  // Add a non-QoS upper MAC, and disable rate control
		  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
		  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode",
		                                StringValue (phyMode), "ControlMode",
		                                StringValue (phyMode));
		  // Set it to ad-hoc mode
		  wifiMac.SetType ("ns3::AdhocWifiMac");

		  AsciiTraceHelper eventTraces;
		  wifiPhy.EnableAsciiAll(eventTraces.CreateFileStream("MyWifiEpedimic.tr"));
	  NetDeviceContainer adhocDevices = wifi.Install (wifiPhy,wifiMac, all);




//	  channel->AddPropagationLossModel("ns3::RangePropagationLossModel"
//              ,"MaxRange", DoubleValue (50.0)
//              );

	  //lrWpanHelper.SetChannel(channel);
	  // Add and install the LrWpanNetDevice for each node
	  LrWpanHelper lrWpanHelper;
	  NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(CDS);

	  lrwpanDevices.Add(lrWpanHelper.Install(sensors2));



	  // Fake PAN association and short address assignment.

	  lrWpanHelper.AssociateToPan (lrwpanDevices, 0);



//	  	for (int i = 0; i < all.GetN(); i++) {
//	  		Ptr<LrWpanNetDevice> dev = DynamicCast<LrWpanNetDevice> (lrwpanDevices.Get(i));
//	  		 Ptr<ConstantPositionMobilityModel> mob0 = CreateObject<ConstantPositionMobilityModel> ();
//
//	  		 Ptr<RandomWaypointMobilityModel> mob2 = CreateObject<RandomWaypointMobilityModel>();
//
//
//	  		ObjectFactory pos;
//	  			  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
//	  			  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));
//	  			  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));
//	  			  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
//	  		  	//	 mob2->SetAttribute("Speed", StringValue("0.3"));
//	  				 mob2->SetAttribute("PositionAllocator",PointerValue(taPositionAlloc));
//
//	  		dev->GetPhy()->SetMobility(mob2);
//
//	  	}

//	  	for (int i = 0; i < all.GetN(); i++) {
//	  		Ptr<LrWpanNetDevice> dev = DynamicCast<LrWpanNetDevice> (lrwpanDevices.Get(i));
////	  		Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
////	  			  Ptr<Cost231PropagationLossModel> model = CreateObject<Cost231PropagationLossModel> ();
////	  			  channel->AddPropagationLossModel(model);
////	  		dev->SetChannel(channel);
////	  		dev->GetPhy()->SetChannel(channel);
//
//	  		 LrWpanSpectrumValueHelper svh;
//	  		  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (0, 11);
//	  		dev->GetPhy ()->SetTxPowerSpectralDensity (psd);
//
//
//	  	}


	  // set the node position and mobility model.

	  MobilityHelper mobilityAdhoc;
	  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

	  ObjectFactory pos;
	  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
	  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));
	  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));



	  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
	  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

	  std::stringstream ssSpeed;
	  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
	  std::stringstream ssPause;
	  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
	  mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
	                                  "Speed", StringValue (ssSpeed.str ()),
	                                  "Pause", StringValue (ssPause.str ()),
	                                  "PositionAllocator", PointerValue (taPositionAlloc));
	  mobilityAdhoc.SetPositionAllocator (taPositionAlloc);
	  mobilityAdhoc.Install (CDS);
	  streamIndex += mobilityAdhoc.AssignStreams (CDS, streamIndex);

	  MobilityHelper staticnodes2;
	  //end of mobility


//	  staticnodes2.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
//	                                 "X", StringValue ("200.0"),
//	                                 "Y", StringValue ("200.0"),
//	                                 "Rho", StringValue ("ns3::UniformRandomVariable[Min=50|Max=200]"));
//	  staticnodes2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//	  staticnodes2.Install(sensors2);

	  	  //here the mobility
	  staticnodes2.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
	                                 "X",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"),
	                                 "Y",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));
	  staticnodes2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	  staticnodes2.Install(sensors2);


	  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
	 	  	  Ptr<LogDistancePropagationLossModel> model = CreateObject<LogDistancePropagationLossModel> ();

	 	  	  channel->AddPropagationLossModel (model);


	  	for (int i = 0; i < all.GetN(); i++) {
	  		Ptr<LrWpanNetDevice> dev = DynamicCast<LrWpanNetDevice> (lrwpanDevices.Get(i));

	  	dev->SetChannel(channel);
//	  		Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
//	  			  Ptr<Cost231PropagationLossModel> model = CreateObject<Cost231PropagationLossModel> ();
//	  			  channel->AddPropagationLossModel(model);
//	  		dev->SetChannel(channel);
//	  		dev->GetPhy()->SetChannel(channel);

	  		 LrWpanSpectrumValueHelper svh;
	  		  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (-1, 11);
	  		dev->GetPhy ()->SetTxPowerSpectralDensity (psd);

	  		Ptr<MobilityModel> themodel = all.Get(i)->GetObject<MobilityModel> ();

	  		dev->GetPhy()->SetMobility(themodel);


	  	}



	  	for (int i = 0; i < sensors2.GetN(); i++) {
	  		Ptr<LrWpanNetDevice> dev = DynamicCast<LrWpanNetDevice> (lrwpanDevices.Get(i+nDS));



	  	dev->SetChannel(channel);
//	  		Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel>();
//	  			  Ptr<Cost231PropagationLossModel> model = CreateObject<Cost231PropagationLossModel> ();
//	  			  channel->AddPropagationLossModel(model);
//	  		dev->SetChannel(channel);
//	  		dev->GetPhy()->SetChannel(channel);

	  		 LrWpanSpectrumValueHelper svh;
	  		  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (-1, 11);
	  		dev->GetPhy ()->SetTxPowerSpectralDensity (psd);

	  		Ptr<MobilityModel> themodel = sensors2.Get(i)->GetObject<MobilityModel> ();

	  		dev->GetPhy()->SetMobility(themodel);


	  	}



	  // install internet stack
	  InternetStackHelper internet;

//	  AodvHelper aodv;
//      internet.SetRoutingHelper(aodv);
	  internet.Install(all);

	  internet.Install(sensors2);

	  // assign ip addresses
	  	Ipv4AddressHelper addressAdhoc;

	    addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");

	    Ipv4InterfaceContainer adhocInterfaces;
	    adhocInterfaces = addressAdhoc.Assign (adhocDevices);


	      SixLowPanHelper sixlowpan;
	      NetDeviceContainer lr_devices = sixlowpan.Install (lrwpanDevices);


	      Ipv6AddressHelper ipv6;
	      ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
	      Ipv6InterfaceContainer lr_interface = ipv6.Assign (lr_devices);




  Ipv4Address groupAddress ("225.0.0.0");
  std::cout<<"multi ip address is "<<ueResponders.GetN ();


  // setting up some other lte configurations


	    /** Energy Model **/
	    /***************************************************************************/
	    /* energy source */
  	  	 LiIonEnergySourceHelper liionEnergyHelper;
	    //BasicEnergySourceHelper basicSourceHelper;
	    // configure energy source
	    liionEnergyHelper.Set("LiIonEnergySourceInitialEnergyJ", DoubleValue (initEnergy));
	    //basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (3000.0));


	    // install source
	    sources = liionEnergyHelper.Install (all);
	    //EnergySourceContainer sources = basicSourceHelper.Install (all);
	    //EnergySourceContainer sources2 = basicSourceHelper.Install (sensors2);




	    /* device energy model */
	    WifiRadioEnergyModelHelper radioEnergyHelper;
	    // configure radio energy model
	    radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.5));
	   radioEnergyHelper.Set("RxCurrentA",DoubleValue(0.5));
	   WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback =
	       MakeCallback (&DepletionHandler);
	   radioEnergyHelper.SetDepletionCallback(callback);



	    // install device model
	     deviceModels = radioEnergyHelper.Install (adhocDevices, sources);
//
//	  //  Ptr<LrWpanRadioEnergyModel> em0;
//	    //DeviceEnergyModelContainer deviceModels2 = radioEnergyHelper.Install (sensor_devices, sources2);
//	    /***************************************************************************/
//

	    RequestResponseServerHelper echoServer (9);






	      serverApps = echoServer.Install (all);



	     serverApps.Start (Seconds (1.0));
	     serverApps.Stop (Seconds (TotalTime));











	     Ipv4InterfaceAddress iface = Ipv4InterfaceAddress(adhocInterfaces.GetAddress(0,0),"255.255.255.0");

	     RequestResponseClientHelper echoClient (iface.GetBroadcast(), 9);
	     echoClient.SetAttribute ("MaxPackets", UintegerValue (5000000));


	     echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	     echoClient.SetAttribute ("PacketSize", UintegerValue (1024));





	     Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable> ();
	      clientApps = echoClient.Install (CDS);

	     for(int i =0;i<nWifis;i++){
	    	 Ptr<RequestResponseServer> app = DynamicCast<RequestResponseServer>(serverApps.Get(i));
	    	 Ptr<RequestResponseClient> app2 = DynamicCast<RequestResponseClient>(clientApps.Get(i));
	    	 Ptr<LiIonEnergySource> source = DynamicCast<LiIonEnergySource>(sources.Get(i));

	    	 app->SetSource(source);
	    	 app2->m_source = source;
	    	 app->status = &(app2->m_statusInt);
	     }

	     //new lrpush apps

	 	LrServerPushHelper lrServer2(9);

	 	//   ApplicationContainer serverApps2 = lrServer.Install (ueResponders);

	 	ApplicationContainer lrserverApps = lrServer2.Install(ueResponders);

	 	// serverApps2.Add(lrServer.Install(sensors2));
	 	// serverApps2.Start (Seconds (1.0));
	 	// serverApps2.Stop (Seconds (TotalTime));
	 	lrserverApps.Start(Seconds(1.0));
	 	lrserverApps.Stop(Seconds(TotalTime));

	 //		 LrAppHelper ping6;
	 //		   ping6.SetRemote (Ipv6Address::GetAny());
	 //
	 //		   ping6.SetAttribute ("MaxPackets", UintegerValue (1000000));
	 //		   ping6.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	 //		   ping6.SetAttribute ("PacketSize", UintegerValue (1024));
	 //		   ApplicationContainer lr_apps = ping6.Install (ueResponders);
	 //
	 //		   lr_apps.Start (Seconds (2.5));
	 //		   lr_apps.Stop (Seconds (TotalTime));

	 	LrClientPushHelper LrClients;
	 	LrClients.SetRemote(Ipv6Address::GetAny());
	 	LrClients.SetAttribute("MaxPackets", UintegerValue(1000000));
	 	LrClients.SetAttribute("Interval", TimeValue(Seconds(1.0)));
	 	LrClients.SetAttribute("PacketSize", UintegerValue(1024));

	 	ApplicationContainer lrClientsApps = LrClients.Install(sensors2);
	 	lrClientsApps.Start(Seconds(2.5));
	 	lrClientsApps.Stop(Seconds(TotalTime));



	 	//old lr apps





//
//	     LRServerHelper lrServer (9);
//
//
//
//		   serverApps2 = lrServer.Install (CDS);
//
//		   serverApps2.Add(lrServer.Install(sensors2));
//			 serverApps2.Start (Seconds (1.0));
//			 serverApps2.Stop (Seconds (TotalTime));
//
//			 LrAppHelper ping6;
//			   ping6.SetRemote (Ipv6Address::GetAny());
//
//			   ping6.SetAttribute ("MaxPackets", UintegerValue (10000000));
//			   ping6.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
//			   ping6.SetAttribute ("PacketSize", UintegerValue (1024));
//			   lr_apps = ping6.Install (CDS);
//
//			   lr_apps.Start (Seconds (2.5));
//			   lr_apps.Stop (Seconds (TotalTime));



			   AnimationInterface anim ("wireless-animation101.xml"); // Mandatory
			for(int i =0;i<nDS;i++){

				 Ptr<RequestResponseClient> app = DynamicCast<RequestResponseClient>(clientApps.Get(i));

				 app->TraceConnectWithoutContext ("MyState", MakeCallback(&IntTrace));

				// Ptr<UdpEchoClient> lteapp = DynamicCast<UdpEchoClient>(clientApps2.Get(i));
				//Ptr<LRClient> lr_app = DynamicCast<LRClient>(lrClientsApps.Get(i));
				 Ptr<LRServerPush> lrserver = DynamicCast<LRServerPush>(lrserverApps.Get(i));
				 double rnd_value = generator->GetInteger(800,999)/1000.0;
				 std::cout<<"the random value is "<<rnd_value<<std::endl;
		 	     app->SetAttribute ("Interval", TimeValue (Seconds (rnd_value)));
		 	     app->state = "temp";
		 	     app->anim = &anim;
		 	     rnd_value = generator->GetInteger(800,999)/1000.0;
		 	    //lr_app->SetDevice(lr_devices.Get(i));
		 	    lrserver->SetDevice(lr_devices.Get(i));
				 Ptr<RequestResponseServer> app2 = DynamicCast<RequestResponseServer>(serverApps.Get(i));
				 app2->SetTable(&(app->MyTable));
				 //lr_app->SetTable(&(app->MyTable));
				 lrserver->SetTable(&(app->MyTable));
				 app2->Settype("CDS");
				 app2->SetPacketsList(&(app->packets_list));

				 //lr_app->SetAttribute ("Interval", TimeValue (Seconds (rnd_value)));
				 app->SetRandomVariableGenerator(generator);
//				 app->SetPacketsv6List(&(lr_app->packetsv6_list));
//				 app2->SetPacketsv6List(&(lr_app->packetsv6_list));

				  app->TraceConnectWithoutContext ("n_packets", MakeCallback(&TraceRequests));
				  app2->TraceConnectWithoutContext ("n_hops", MakeCallback(&TraceHops));
				  app2->TraceConnectWithoutContext ("n_delivered", MakeCallback(&TracePackets));
				  app->numberUE = nWifis;
				  app->trace_sending_packet = MakeCallback(&TracePacketstart);
				  app2->trace_receiving_packet= MakeCallback(&TracePacketsDate);
				  app->myEnergy = deviceModels.Get(i);


			}



	     clientApps.Start (Seconds (2.5));
	     clientApps.Stop (Seconds (TotalTime));

//			clientApps2.Start (Seconds (2.5));
//			clientApps2.Stop (Seconds (50.0));






	  	for (int i = 0; i < sensors2.GetN(); i++) {
	  		Ptr<LRClientPush> lrclient = DynamicCast<LRClientPush>(
	  				lrClientsApps.Get(i));
	  		lrclient->SetDevice(lr_devices.Get(i + nDS));
	  		 Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable> ();
			 int randomint = generator->GetInteger(0,3) + 1;
			serviceTypes.push_back(randomint) ;
	  		lrclient->servicetype = randomint;
	  		anim.UpdateNodeColor (sensors2.Get (i), 255, 255, 255); // Optional
	  	}
	  	 for (uint32_t i = 0; i < CDS.GetN (); ++i)
	  		        {
	  		          anim.UpdateNodeDescription (CDS.Get (i), "STA"); // Optional
	  		          anim.UpdateNodeColor (CDS.Get (i), 0, 255, 0); // Optional
	  		        Ptr<RequestResponseClient> app = DynamicCast<RequestResponseClient>(clientApps.Get(i));
	  		        app->services = serviceTypes;
	  		        }


//
//
//	     std::string animFile = "wifianime.xml" ;
//	    	          AnimationInterface anim (animFile);

	     	 Simulator:: Schedule (Seconds(10.0), &counterf);
	     	 Simulator:: Schedule (Seconds(3.0), &populateGlobaltable);
	     	Simulator:: Schedule (Seconds(3.5), &generateTraffic);
	     	 Simulator:: Schedule (Seconds(3.5), &testConnected);

	          Simulator::Stop (Seconds (TotalTime));
	          Simulator::Run ();
	          NS_LOG_LOGIC("average CDS number is " << sum/count << " and Max is  " << MaxNCDS<< std::endl);

	          double ratio = double(numberdelevery)/numberrequest;
	          double ratiohops = double(numberhops)/counthops;
	          double battery_average= 0.0;

	          	          std::cout<<"number of request is "<<numberrequest<<" number of delevery is "<<numberdelevery<<" delevery ratio is  " << ratio << "average hops is "
	          	        		  <<numberhops << "counts is " << counthops<<"delay of tranmission is "<<transmissiondelay/numberdelevery<<std::endl;
	          	          for(int i=0;i<sources.GetN();i++){
	          	        	 std::cout<<"node " << i << "has "<<deviceModels.Get(i)->GetTotalEnergyConsumption()
	          	        	 << "j consumed "<<std::endl;
	          	        battery_average += deviceModels.Get(i)->GetTotalEnergyConsumption();

	          	          }


	          	        	          battery_average = battery_average/sources.GetN();
	          	          AsciiTraceHelper ascii1;
						Ptr<OutputStreamWrapper> stream2 = ascii1.CreateFileStream ( "wifisimu.tr",std::ios::app);

						*stream2->GetStream () << sources.GetN()<<"\t"<<battery_average<<"\t"
								<<ratio<<"\t"<<ratiohops<<"\t"<< (sum/count) <<"\t"
								<< transmissiondelay/numberdelevery <<"\t"<<
								(double)connectivity_percent/(double)nbcycles
								<<"\t"<<(double)coverage_percent*100/(double)nbcycles<<
								"\t"<<(double)first_noeud_to_die<<std::endl;
	          //flowmon->SerializeToXmlFile ((tr_name + ".flowmon").c_str(), false, false);

						  std::cout<<" average percentage of connectivity is  "<<(double)connectivity_percent/(double)nbcycles<< std::endl;
							           std::cout<<" average percentage of coverage is  "<<(double)coverage_percent*100/(double)nbcycles<< std::endl;
							           Simulator::Destroy ();


						   std::cout<<" first noeud died at : "<<first_noeud_to_die << std::endl;


	//create nodes

}

