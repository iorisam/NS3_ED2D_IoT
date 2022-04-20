#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"

#include "ns3/energy-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"

#include "ns3/internet-apps-module.h"
#include <ns3/config-store.h>
#include <ns3/nist-lte-helper.h>
#include <ns3/nist-module.h>
#include "ns3/nist-sl-resource-pool-factory.h"
#include "ns3/nist-sl-preconfig-pool-factory.h"



using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("Dominating-sets-simulation");

	NodeContainer all ;
	NodeContainer CDS;
	NodeContainer sensors;
	NodeContainer sensors2;
	NodeContainer ueResponders;

  uint32_t mcs = 10;
  uint32_t rbSize = 8;
  uint32_t ktrp = 1;
  uint32_t pscchLength = 16;
  std::string period="sf80";
  double dist = 10;
  double ueTxPower = 23.0;

  bool verbose = false;

	int nWifis = 18;

	int nDS = 18;
	uint32_t ueCount = nDS;
	int nLR = 50;
	 std::map<int,int> packets_tranmission_time;



	  uint32_t port=9;
	double TotalTime = 100.0;
	  std::string rate ("2048bps");
	  std::string phyMode ("DsssRate11Mbps");
	  std::string tr_name ("manet-routing-compare");
	  int nodeSpeed = 5; //in m/s
	  int nodePause = 0; //in s
	  std::string m_protocolName = "protocol";
	  double txp =2.0;

	  uint32_t Rid1;
	  uint32_t Rid2;

	  int numberCDS = 0;
	  int numberhops = 0;
	  int numberrequest = 0;
	  int numberdelevery = 0;
	  int counthops = 0;
	  int MaxNCDS = 0;
	  int count = 1;
	  int sum = 0;
	  double transmissiondelay=0;
	  std::map<int,double> myrequests;

	  Ptr<Socket>
	  SetupPacketReceive (Ipv4Address addr, Ptr<Node> node)
	  {
	    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	    Ptr<Socket> sink = Socket::CreateSocket (node, tid);
	    InetSocketAddress local = InetSocketAddress (addr, port);
	    sink->Bind (local);
	    return sink;
	  }


	  void IntTrace (uint32_t oldValue, uint32_t newValue){
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

	  void SlStartDiscovery (Ptr<NistLteHelper> helper, Ptr<NetDevice> ue, std::list<std::string> apps, bool rxtx)
	  {
	    helper->StartDiscovery (ue, apps, rxtx);
	  }

	  void SlStopDiscovery (Ptr<NistLteHelper> helper, Ptr<NetDevice> ue, std::list<std::string> apps, bool rxtx)
	  {
	    helper->StopDiscovery (ue, apps, rxtx);
	  }
	  void DiscoveryMonitoringTrace (Ptr<OutputStreamWrapper> stream, uint64_t imsi, uint16_t cellId, uint16_t rnti, std::string proSeAppCode)
	  {
	  	std::cout << Simulator::Now ().GetSeconds () << "\t" << imsi << "\t" << cellId << "\t" << rnti << "\t" << proSeAppCode << std::endl;
	  }

	  void DiscoveryAnnouncementPhyTrace (Ptr<OutputStreamWrapper> stream, std::string imsi, uint16_t cellId, uint16_t rnti, std::string proSeAppCode)
	  {
		  std::cout << Simulator::Now ().GetSeconds () << "\t" << imsi << "\t" << cellId << "\t"  << rnti << "\t" << proSeAppCode << std::endl;
	  }

	  void DiscoveryAnnouncementMacTrace (Ptr<OutputStreamWrapper> stream, std::string imsi, uint16_t rnti, std::string proSeAppCode)
	  {
		  std::cout << Simulator::Now ().GetSeconds () << "\t" << imsi << "\t" << rnti << "\t" << proSeAppCode << std::endl;
	  }





	   uint32_t pucchSize = 6;          // PUCCH size in RBs
	   uint32_t pscch_rbs = 22;         // PSCCH pool size in RBs. Note, the PSCCH occupied bandwidth will be at least twice this amount.
	   std::string pscch_bitmap_hexstring = "0xFF00000000"; // PSCCH time bitmap [40 bits]
	   uint32_t sl_period = 40;         // Length of sidelink period in milliseconds
	   bool CtrlErrorModelEnabled = true; // Enable error model in the PSCCH
	   bool CtrlDropOnCollisionEnabled = false; // Drop PSCCH messages on conflicting scheduled resources
	   uint32_t nbRings = 1;            // Number of rings in hexagon cell topology
	    double isd = 10;                 // Inter Site Distance
	    double minCenterDist = 1;
	    uint32_t ue_responders_per_sector = 3;

	    uint32_t nbgroups = 1;
int
main (int argc, char *argv[])
{
//		 LogComponentEnable ("LrWpanEnergySource", LOG_LEVEL_ALL);
//		 LogComponentEnable ("ZigbeeEnergyModel", LOG_LEVEL_ALL);
//	  LogComponentEnable ("lte-echo-app", LOG_LEVEL_ALL);
//LogComponentEnable ("Dominating-sets-simulation", LOG_LEVEL_ALL);
//LogComponentEnable ("NistLteUePhy", LOG_LEVEL_ALL);
//
//LogComponentEnable ("NistLteSpectrumPhy", LOG_LEVEL_ALL);
//LogComponentEnable ("NistLteUeRrc", LOG_LEVEL_ALL);
//LogComponentEnable ("NistLteEnbPhy", LOG_LEVEL_ALL);
//LogComponentEnable ("NistLteUeMac", LOG_LEVEL_ALL);
//LogComponentEnable ("NistEpcHelper", LOG_LEVEL_ALL);
//LogComponentEnable ("NistEpcHelper", LOG_LEVEL_ALL);
//LogComponentEnable ("NistLteHelper", LOG_LEVEL_ALL);
//LogComponentEnable ("NistPointToPointEpcHelper", LOG_LEVEL_ALL);


//     LogComponentEnable ("Dominating-sets-simulation", LOG_LEVEL_ALL);
//
//	 LogComponentEnable ("Lr6Application", LOG_LEVEL_ALL);
//
//	LogComponentEnable ("RequestResponseServerApplication", LOG_LEVEL_ALL);
//	 LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);
//	LogComponentEnable("lte_echo_server", LOG_LEVEL_ALL);
//	 LogComponentEnable ("LRServerApplication", LOG_LEVEL_ALL);

	// LogComponentEnable("NistLteUeMac", LOG_LEVEL_ALL);
 CommandLine cmd;
  cmd.AddValue ("period", "Sidelink period", period);
  cmd.AddValue ("responders", "Number of Responders per sector", ue_responders_per_sector);
  cmd.AddValue ("pscchLength", "Length of PSCCH.", pscchLength);
  cmd.AddValue ("ktrp", "Repetition.", ktrp);
  cmd.AddValue ("mcs", "MCS.", mcs);
  cmd.AddValue ("rbSize", "Allocation size.", rbSize);
  cmd.AddValue ("verbose", "Print time progress.", verbose);
  cmd.AddValue ("nUE", "Print time progress.", nWifis);
  cmd.AddValue ("pucchSize", "PUCCH size", pucchSize);
   cmd.AddValue ("pscch_rbs", "PSCCH RBs", pscch_rbs);
   cmd.AddValue ("pscch_trp", "PSCCH time bitmap", pscch_bitmap_hexstring);
   cmd.AddValue ("slperiod", "Length of SL period", sl_period );
   cmd.AddValue ("ctrlerror", "Enables PSCCH error model", CtrlErrorModelEnabled);
   cmd.AddValue ("ctrldroponcol", "Drop PSCCH messages on collisions", CtrlDropOnCollisionEnabled);
   cmd.AddValue ("ring", "Number of rings in hexagon cell topology", nbRings);
   cmd.AddValue ("isd", "Inter Site Distance", isd);
   cmd.AddValue ("mindist", "Minimum Center Distance for UEs", minCenterDist);
   cmd.AddValue ("groups", "Number of groups", nbgroups);

  cmd.Parse(argc, argv);






  NS_LOG_INFO ("Configuring UE settings...");
  //----------------------------------------------------------------------------------------------/
  Config::SetDefault ("ns3::NistLteSpectrumPhy::ErrorModelHarqD2dDiscoveryEnabled", BooleanValue (false));
   Config::SetDefault ("ns3::NistLteEnbNetDevice::DlBandwidth", StringValue ("50"));
   Config::SetDefault ("ns3::NistLteEnbRrc::DefaultTransmissionMode", UintegerValue (2));
   Config::SetDefault ("ns3::NistLteRlcUm::MaxTxBufferSize", StringValue ("100000"));
   Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (10000000));

   // Set the frequency to use for the Public Safety case (band 14 : 788 - 798 MHz for Uplink)
   Config::SetDefault ("ns3::NistLteEnbNetDevice::UlEarfcn", StringValue ("23330"));
   Config::SetDefault ("ns3::NistLteEnbNetDevice::UlBandwidth", StringValue ("50"));

   // Set the UEs power in dBm
   Config::SetDefault ("ns3::NistLteUePhy::TxPower", DoubleValue (ueTxPower));
   Config::SetDefault ("ns3::NistLteUePhy::RsrpUeMeasThreshold", DoubleValue (-10.0));

   // Set power control
   Config::SetDefault ("ns3::NistLteUePowerControl::Pcmax", DoubleValue (ueTxPower));
   Config::SetDefault ("ns3::NistLteUePowerControl::PoNominalPusch", IntegerValue (-106));
   Config::SetDefault ("ns3::NistLteUePowerControl::PsschTxPower", DoubleValue (ueTxPower));
   Config::SetDefault ("ns3::NistLteUePowerControl::PscchTxPower", DoubleValue (ueTxPower));

   // Configure error model
   Config::SetDefault ("ns3::NistLteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (CtrlErrorModelEnabled));
   Config::SetDefault ("ns3::NistLteSpectrumPhy::CtrlFullDuplexEnabled", BooleanValue (!CtrlErrorModelEnabled));
   Config::SetDefault ("ns3::NistLteSpectrumPhy::DropRbOnCollisionEnabled", BooleanValue (CtrlDropOnCollisionEnabled));

   std::cout<<"CtrlErrorModel: " << ((CtrlErrorModelEnabled)?"Enabled":"Disabled") << std::endl;
   std::cout<<"CtrlDropOnCollision: " << ((CtrlDropOnCollisionEnabled)?"Enabled":"Disabled") << std::endl;

   // Configure for UE selected
   Config::SetDefault ("ns3::NistLteUeMac::SlGrantSize", UintegerValue (rbSize));
   Config::SetDefault ("ns3::NistLteUeMac::SlGrantMcs", UintegerValue (mcs));
   Config::SetDefault ("ns3::NistLteUeMac::Ktrp", UintegerValue (ktrp));
   Config::SetDefault ("ns3::NistLteUeMac::PucchSize", UintegerValue (pucchSize));

   ConfigStore inputConfig;
   inputConfig.ConfigureDefaults();

   //***************************************************************************************************************

nDS = nLR= nWifis;


 NS_LOG_INFO ("Creating helpers...");
  Ptr<NistPointToPointEpcHelper>  epcHelper = CreateObject<NistPointToPointEpcHelper> ();
  Ptr<Node> pgw = epcHelper->GetPgwNode ();


  Ptr<NistLteHelper> lteHelper = CreateObject<NistLteHelper> ();
  //lteHelper->Initialize ();

  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->DisableNewEnbPhy ();

  Ptr<NistLteProseHelper> proseHelper = CreateObject<NistLteProseHelper> ();
    proseHelper->SetLteHelper (lteHelper);



    lteHelper->SetEnbAntennaModelType ("ns3::NistParabolic3dAntennaModel");
    lteHelper->SetEnbAntennaModelAttribute ("MechanicalTilt", DoubleValue (20));
    //lteHelper->SetPathlossModelType ("ns3::Cost231PropagationLossModel");
    lteHelper->SetPathlossModelType ("ns3::FriisPropagationLossModel");








  NS_LOG_INFO ("Deploying UE's...");



	//create nodes and node containers
		all.Create(nWifis);


		for(int i =0;i<nDS;i++){
			CDS.Add(all.Get(i));
			ueResponders.Add(all.Get(i));
		}
		for(int i =nDS;i<nWifis;i++){
			sensors.Add(all.Get(i));
		}

		sensors2.Create(nLR);

//set up LTE



		lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));

		  lteHelper->SetAttribute ("UseDiscovery", BooleanValue (true));
		  lteHelper->SetSchedulerType ("ns3::NistRrSlFfMacScheduler");




		// set the phyisical layer and channel model with propagation delay and pathloss , data rate n controlMode, and transmission power,
		// and ap or adhoc mode,



	  // set the node position and mobility model.

	  MobilityHelper mobilityAdhoc;
	  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

	  ObjectFactory pos;
	  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
	  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"));
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
	  staticnodes2.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
	                                 "X", StringValue ("250.0"),
	                                 "Y", StringValue ("250.0"),
	                                 "Rho", StringValue ("ns3::UniformRandomVariable[Min=50|Max=250]"));
	  staticnodes2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	  staticnodes2.Install (sensors);
	  staticnodes2.Install(sensors2);

	  // install internet stack



	  NS_LOG_INFO ("Installing UE network devices...");

	  NetDeviceContainer ueRespondersDevs = lteHelper->InstallUeDevice (ueResponders);


	   NetDeviceContainer ueDevs;
	   ueDevs.Add (ueRespondersDevs);


	   /* NetDeviceContainer ueDevs;
	    NetDeviceContainer ueRespondersDevs = lteHelper->InstallUeDevice (ueResponders);
	    ueDevs.Add (ueRespondersDevs);*/

	    InternetStackHelper internet;
	      internet.Install (all);
	      internet.Install(sensors2);

	      NS_LOG_INFO ("Allocating IP addresses and setting up network route...");
	      Ipv4InterfaceContainer ueIpIface;

	      Ipv4StaticRoutingHelper ipv4RoutingHelper;
	      ueIpIface = epcHelper->AssignUeIpv4Address (ueDevs);
	      std::cout<<"multi ip address is "<<ueDevs.GetN();

	      for (uint32_t u = 0; u < ueResponders.GetN (); ++u)
	        {
	          Ptr<Node> ueNode = ueResponders.Get (u);
	          // Set the default gateway for the UE
	          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
	          std::cout<<"node ip address is "<<ueNode->GetObject<Ipv4> ();
	          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
	        }


    NS_LOG_INFO ("Attaching UE's to LTE network...");
  lteHelper->Attach (ueDevs);

  Ipv4InterfaceAddress iface = Ipv4InterfaceAddress(ueIpIface.GetAddress(0,0),"255.0.0.0");



  Ipv4Address groupAddress ("225.0.0.0");
  std::cout<<"multi ip address is "<<ueResponders.GetN ();
  for (uint32_t u = 0; u < ueResponders.GetN (); ++u)
    {

      Ptr<Node> ueNode = ueResponders.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      std::cout<<"node ip address is "<<ueNode->GetObject<Ipv4> ();
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }





//// lrpan

  LrWpanHelper lrWpanHelper;
	  // Add and install the LrWpanNetDevice for each node
	  NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(ueResponders);
	  NetDeviceContainer lrwpanDevices2 = lrWpanHelper.Install(sensors2);

	  NetDeviceContainer AllLrDevices;
	  AllLrDevices.Add(lrwpanDevices);
	  AllLrDevices.Add(lrwpanDevices2);



	  // PAN association and short address assignment.

	  lrWpanHelper.AssociateToPan (AllLrDevices, 0);

	  SixLowPanHelper sixlowpan;
	 	      NetDeviceContainer lr_devices = sixlowpan.Install (AllLrDevices);


	 	      Ipv6AddressHelper ipv6;
	 	      ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
	 	      Ipv6InterfaceContainer lr_interface = ipv6.Assign (lr_devices);


  // setting up some other lte configurations

	 	     NS_LOG_INFO ("Creating sidelink configuration...");
	 	      Ptr<LteUeRrcSl> ueSidelinkConfiguration = CreateObject<LteUeRrcSl> ();
	 	      ueSidelinkConfiguration->SetSlEnabled (true);

	 	      NistLteRrcSap::SlPreconfiguration preconfiguration;
	 	      preconfiguration.preconfigGeneral.carrierFreq = 23330;
	 	      preconfiguration.preconfigGeneral.slBandwidth = 50;
	 	      preconfiguration.preconfigComm.nbPools = 3;
	 	      preconfiguration.preconfigDisc.nbPools = 1;
	 	      preconfiguration.preconfigDisc.pools[0].cpLen.cplen = NistLteRrcSap::SlCpLen::NORMAL;
	 	      preconfiguration.preconfigDisc.pools[0].discPeriod.period = NistLteRrcSap::SlPeriodDisc::rf32;
	 	      preconfiguration.preconfigDisc.pools[0].numRetx = 0;
	 	      preconfiguration.preconfigDisc.pools[0].numRepetition = 1;
	 	      preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbNum = 10;
	 	      preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbStart = 10;
	 	      preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbEnd = 49;
	 	      preconfiguration.preconfigDisc.pools[0].tfResourceConfig.offsetIndicator.offset = 0;
	 	      preconfiguration.preconfigDisc.pools[0].tfResourceConfig.subframeBitmap.bitmap = std::bitset<40> (0x11111);
	 	      preconfiguration.preconfigDisc.pools[0].txParameters.txParametersGeneral.alpha = NistLteRrcSap::SlTxParameters::al09;
	 	      preconfiguration.preconfigDisc.pools[0].txParameters.txParametersGeneral.p0 = -40;
	 	      //preconfiguration.preconfigDisc.pools[0].txParameters.txProbability.probability = NistLteRrcSap::TxProbability::p100;
	 	      preconfiguration.preconfigDisc.pools[0].txParameters.txProbability = NistLteRrcSap::TxProbabilityFromInt (50);

	 	      //Convert pscch_trp hex string representation to decimal.
	 	      std::stringstream ss_hex;
	 	      ss_hex << std::hex << pscch_bitmap_hexstring;
	 	      uint64_t pscch_trpnumber;
	 	      ss_hex >> pscch_trpnumber;
	 	      std::cout<<"PSCCHRBS: " << pscch_rbs << ", PSCCH_hexstring: " << pscch_bitmap_hexstring << std::endl;
	 	      uint64_t t_number = pscch_trpnumber;
	 	      uint32_t zero_bit_counter = 0;
	 	      while (t_number%2 == 0)
	 	        {
	 	          zero_bit_counter++;
	 	          t_number/=2;
	 	        }
	 	      uint32_t pscch_length = 40 - zero_bit_counter;

	 	      NistSlPreconfigPoolFactory pfactory;
	 	      pfactory.SetControlBitmap (pscch_trpnumber);
	 	      std::stringstream pstream;
	 	      pstream << "sf" << sl_period;
	 	      pfactory.SetControlPeriod (pstream.str());
	 	      pfactory.SetControlPrbNum (pscch_rbs);
	 	      pfactory.SetDataOffset (pscch_length);

	 	      preconfiguration.preconfigComm.pools[0] = pfactory.CreatePool ();
	 	      ueSidelinkConfiguration->SetSlPreconfiguration (preconfiguration);

	 	      NS_LOG_INFO ("Installing sidelink configuration...");
	 	      lteHelper->InstallSidelinkConfiguration (ueRespondersDevs, ueSidelinkConfiguration);

	 	      NS_LOG_INFO ("Enabling LTE traces...");
	 	      lteHelper->EnableTraces();

 // lteHelper->EnableLogComponents();






	     Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable> ();



//
	     lte_echo_client_helper udpClient (iface.GetBroadcast() , 8000);
	       udpClient.SetAttribute ("MaxPackets", UintegerValue (5000000));
	       udpClient.SetAttribute ("Interval", TimeValue (Seconds (1.001)));
	       udpClient.SetAttribute ("PacketSize", UintegerValue (100));
	       udpClient.SetIfIndex(0);


	       lte_echo_server_helper udpServer (8000);
	       ApplicationContainer clientApps2 = udpClient.Install (ueResponders);
	       ApplicationContainer serverApps45 = udpServer.Install (ueResponders);
	       serverApps45.Start(Seconds (1.0));
	       udpServer.SetIfIndex(0);
	       serverApps45.Stop(Seconds (TotalTime));



		// setting up Energy sources and Energy models for Sensors
		LrWpanEnergySourceHelper lrSourceHelper;
		// configure energy source
		lrSourceHelper.Set("LrWpanEnergySourceInitialEnergyJ",DoubleValue(100) );
		// install source
		EnergySourceContainer lr_sources = lrSourceHelper.Install (sensors2);
		/* device energy model */
		EnergySourceContainer lr_sources2 = lrSourceHelper.Install (CDS);
		ZigbeeEnergyModelHelper zigbeeEnergyHelper;
		// configure radio energy model

		// install device model
		DeviceEnergyModelContainer deviceModels = zigbeeEnergyHelper.Install (lrwpanDevices2, lr_sources);
		/***************************************************************************/

		// setting up Energy models for Ues lr-wpan

		// configure energy source
		EnergySourceContainer ue_sources;
		/* device energy model */
		for(int i =0;i<nDS;i++){
		Ptr<LiIonEnergySource> ue_source = lteHelper->
		EnergySourcesContainer.find(CDS.Get(i)->GetId())->second->LIES.GetObject<LiIonEnergySource>();
		ue_source->SetNode(CDS.Get(i));
		//std::cout<<"my energy is "<<ue_source->GetInitialEnergy()<<"and remaining energy level is "<<ue_source->GetRemainingEnergy()<<std::endl;
		ue_sources.Add(ue_source);
		}


		// install device model
		//DeviceEnergyModelContainer deviceModels2 = zigbeeEnergyHelper.Install (lrwpanDevices, lr_sources2);

											   /***************************************************************************/


			for(int i =0;i<nDS;i++){


				 Ptr<Lte_Echo_Client> lteapp = DynamicCast<Lte_Echo_Client>(clientApps2.Get(i));
				Ptr<lte_echo_server> lteapp2 = DynamicCast<lte_echo_server>(serverApps45.Get(i));

				lteapp2->SetTable(&(lteapp->MyTable));
				 lteapp2->SetBroadcastAddress(iface.GetBroadcast());
				 lteapp2->SetLteClient(&lteapp);
				 lteapp2->SetState(&(lteapp->state));
				 lteapp2->SetPacketsList(&(lteapp->packets_list));
				 lteapp2->SetNeighborsPacketsList(&(lteapp->neighbors_packets_list));
				 double rnd_value = generator->GetInteger(80,99)/100.0;

		 	    lteapp->SetAttribute ("Interval", TimeValue (Seconds (rnd_value)));

		 	    lteapp->my_energy =lteHelper->EnergySourcesContainer.find(CDS.Get(i)->GetId())->second;
		 	    lteapp->SetRandomVariableGenerator(generator);
		 	   lteapp->TraceConnectWithoutContext ("MyState", MakeCallback(&IntTrace));
		 	  lteapp->TraceConnectWithoutContext ("n_packets", MakeCallback(&TraceRequests));
		 	 lteapp2->TraceConnectWithoutContext ("n_hops", MakeCallback(&TraceHops));
		 	lteapp2->TraceConnectWithoutContext ("n_delivered", MakeCallback(&TracePackets));
		 	lteapp->numberUE = nWifis;
		 	lteapp->trace_sending_packet = MakeCallback(&TracePacketstart);
		 	lteapp2->trace_receiving_packet= MakeCallback(&TracePacketsDate);
		 	//lteapp2->SetLteClient(lteapp);
		 	//std::cout<<"my energy is "<<ue_sources.Get(i)->GetInitialEnergy()<<"and i have left "<<ue_sources.Get(i)->GetEnergyFraction()<<std::endl;
			}


			clientApps2.Start (Seconds (2.5));
			clientApps2.Stop (Seconds (TotalTime));


			LRServerHelper lrServer (9);



			       ApplicationContainer serverApps2 = lrServer.Install (ueResponders);

			       serverApps2.Add(lrServer.Install(sensors2));
				     serverApps2.Start (Seconds (1.0));
				     serverApps2.Stop (Seconds (TotalTime));

				     LrAppHelper ping6;
				       ping6.SetRemote (Ipv6Address::GetAny());

				       ping6.SetAttribute ("MaxPackets", UintegerValue (1000000));
				       ping6.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
				       ping6.SetAttribute ("PacketSize", UintegerValue (1024));
				       ApplicationContainer lr_apps = ping6.Install (ueResponders);

				       lr_apps.Start (Seconds (2.5));
				       lr_apps.Stop (Seconds (TotalTime));

						  DiscAppHelper discapphelper;
						  ping6.SetAttribute ("Interval", TimeValue (Seconds (1.0)));

						  ApplicationContainer discoveryapps = discapphelper.Install(ueResponders);

						  discoveryapps.Start (Seconds (1.0));
						  discoveryapps.Stop (Seconds (TotalTime));

					     for(int i =0;i<ueResponders.GetN();i++){
					    	 Ptr<LRClient> app = DynamicCast<LRClient>(lr_apps.Get(i));
					    	 Ptr<lte_echo_server> lteappserv = DynamicCast<lte_echo_server>(serverApps45.Get(i));
					    	 Ptr<DiscApp> discapp = DynamicCast<DiscApp>(discoveryapps.Get(i));

					    	 //	 std::cout<<"the ip address of such app "<<i-nLR <<" is :"<<lr_devices.Get(i)->GetIfIndex();
					    	 app->SetDevice(lr_devices.Get(i));
					    	 Ptr<Lte_Echo_Client> lteapp = DynamicCast<Lte_Echo_Client>(clientApps2.Get(i));
					    	 app->SetTable(&(lteapp->MyTable));
					    	 lteappserv->SetPacketsv6List(&(app->packetsv6_list));
					    	 lteapp->SetPacketsv6List(&(app->packetsv6_list));

					    	 discapp->SetTable(&(lteapp->MyTable));
					    	 discapp->SetLteHelper(lteHelper);

					    	 discapp->my_energy =
					    			 lteHelper->EnergySourcesContainer.find(CDS.Get(i)->GetId())->second;
					     }

					     AsciiTraceHelper ascii;
					      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("discovery_out_monitoring.tr");
					      *stream->GetStream () << "Time\tIMSI\tCellId\tRNTI\tProSeAppCode" << std::endl;

					      AsciiTraceHelper ascii11;
					      Ptr<OutputStreamWrapper> stream1 = ascii11.CreateFileStream ( "discovery_out_announcement_phy.tr");
					      *stream1->GetStream () << "Time\tIMSI\tCellId\tRNTI\tProSeAppCode" << std::endl;

					      AsciiTraceHelper ascii2;
					      Ptr<OutputStreamWrapper> stream22 = ascii11.CreateFileStream ( "discovery_out_announcement_mac.tr");
					      *stream22->GetStream () << "Time\tIMSI\tRNTI\tProSeAppCode" << std::endl;

					      std::ostringstream oss;
					      oss.str("");

					     for (uint32_t i = 0; i < ueDevs.GetN (); ++i)
					     {
					       Ptr<NistLteUeRrc> ueRrc = DynamicCast<NistLteUeRrc>( ueDevs.Get (i)->GetObject<NistLteUeNetDevice> ()->GetRrc () );
					       ueRrc->TraceConnectWithoutContext ("DiscoveryMonitoring", MakeBoundCallback (&DiscoveryMonitoringTrace, stream));
					       oss << ueDevs.Get (i)->GetObject<NistLteUeNetDevice> ()->GetImsi ();
					       Ptr<NistLteUePhy> uePhy = DynamicCast<NistLteUePhy>( ueDevs.Get (i)->GetObject<NistLteUeNetDevice> ()->GetPhy () );
					       uePhy->TraceConnect ("DiscoveryAnnouncement", oss.str (), MakeBoundCallback (&DiscoveryAnnouncementPhyTrace, stream1));
					       Ptr<NistLteUeMac> ueMac = DynamicCast<NistLteUeMac>( ueDevs.Get (i)->GetObject<NistLteUeNetDevice> ()->GetMac () );
					       ueMac->TraceConnect ("DiscoveryAnnouncement", oss.str (), MakeBoundCallback (&DiscoveryAnnouncementMacTrace, stream22));
					       oss.str("");
					     }

					     std::map<Ptr<NetDevice>, std::list<std::string> > announceApps;
					       std::map<Ptr<NetDevice>, std::list<std::string> > monitorApps;
					       for (uint32_t i=1; i<= ueDevs.GetN (); ++i)
					       {
					         announceApps[ueRespondersDevs.Get(i-1)].push_back (std::to_string(i));
					         for (uint32_t j=1; j<= ueDevs.GetN (); ++j)
					         {
					           if (i != j)
					           {
					             monitorApps[ueRespondersDevs.Get(i-1)].push_back (std::to_string(j));
					           }
					         }
					       }

					      /* for (uint32_t i = 0; i <ueDevs.GetN (); ++i)
					       {
					         Simulator::Schedule (Seconds(2.0), &SlStartDiscovery, lteHelper, ueRespondersDevs.Get(i),announceApps.find(ueDevs.Get(i))->second, true); // true for announce
					         Simulator::Schedule (Seconds(2.0), &SlStartDiscovery, lteHelper, ueRespondersDevs.Get(i), monitorApps.find(ueDevs.Get(i))->second, false); // false for monitor
					       }
*/


			  Simulator:: Schedule (Seconds(10.0), &counterf);
	          Simulator::Stop (Seconds (TotalTime));
	          Simulator::Run ();
	          NS_LOG_LOGIC("average CDS number is " << sum/count << " and Max is  " << MaxNCDS<< std::endl);
	          double ratio = double(numberdelevery)/numberrequest;
	          double ratiohops = double(numberhops)/counthops;
	          double battery_average= 0.0;

  	          std::cout<<"number of request is "<<numberrequest<<" number of delevery is "<<numberdelevery<<" delevery ratio is  " << ratio << "average hops is "
  	        		  <<numberhops << "counts is " << counthops<<std::endl;

	          for(int i=0;i<ue_sources.GetN();i++){
	         	          	        	 std::cout<<"node " << i << "has "
	         	          	        		          	        		  <<ue_sources.Get(i)->GetRemainingEnergy() << "j left "<<std::endl;

	        battery_average += (ue_sources.Get(i)->GetInitialEnergy() - ue_sources.Get(i)->GetRemainingEnergy());

	          }
	          battery_average = battery_average/ue_sources.GetN();
	          std::cout<<"the average battery consumption is "<<battery_average<<std::endl;
	          AsciiTraceHelper ascii1;
	            Ptr<OutputStreamWrapper> stream2 = ascii1.CreateFileStream ( "ltesimu.tr",std::ios::app);


	            *stream2->GetStream () << ue_sources.GetN()<<"\t"<<battery_average<<"\t"
	            		<<ratio<<"\t"<<ratiohops<<"\t"<< (sum/count) << "\t" << transmissiondelay/numberdelevery << std::endl;
	          Simulator::Destroy ();



	//create nodes

}
