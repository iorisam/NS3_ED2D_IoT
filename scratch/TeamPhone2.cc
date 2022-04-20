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
#include "ns3/v4ping-helper.h"
#include "ns3/v4ping.h"

#include "ns3/aodv-module.h"

#include "ns3/internet-apps-module.h"




using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("TeamPhone2");

		  NodeContainer all ;

		  double dist = 10;
		  double ueTxPower = 8.0;

		  bool verbose = false;


		  double Prss = -80;
		  double offset = 81;
		  int nWifis = 30;

		  int nDS = 30;
		  uint32_t ueCount = nDS;

		  uint32_t port=9;
	  	  double TotalTime = 100.0;
		  std::string rate ("2048bps");
		  std::string phyMode ("DsssRate11Mbps");
		  std::string tr_name ("teamphone2");
		  int nodeSpeed = 1; //in m/s
		  int nodePause = 0; //in s
		  std::string m_protocolName = "protocol";
		  double txp =2.0;

		  ApplicationContainer clientApps;
		  ApplicationContainer serverApps;
		  ApplicationContainer serverApps2;
		  ApplicationContainer lr_apps;
		  EnergySourceContainer sources;
		  DeviceEnergyModelContainer deviceModels;

		  double initEnergy = 20.0;
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
	  nDS = nWifis;


	//create nodes and node containers
		all.Create(nWifis);


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

	  MobilityHelper mobilityAdhoc;
	  int64_t streamIndex = 0; // used to get consistent mobility across scenarios

	  ObjectFactory pos;
	  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
	  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"));
	  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"));



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
	  mobilityAdhoc.Install (all);
	  streamIndex += mobilityAdhoc.AssignStreams (all, streamIndex);


	  InternetStackHelper internet;

	  AodvHelper aodv;
      internet.SetRoutingHelper(aodv);
	  internet.Install(all);


	  // assign ip addresses
	  	Ipv4AddressHelper addressAdhoc;

	    addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");

	    Ipv4InterfaceContainer adhocInterfaces;
	    adhocInterfaces = addressAdhoc.Assign (adhocDevices);



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
//	   WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback =
//	       MakeCallback (&DepletionHandler);
//	   radioEnergyHelper.SetDepletionCallback(callback);



	    // install device model
	     deviceModels = radioEnergyHelper.Install (adhocDevices, sources);


	     for (int i= 0;i<all.GetN() - 3;i++){
	    	  Ptr<V4Ping> app = CreateObject<V4Ping> ();
	    	     app->SetAttribute ("Remote", Ipv4AddressValue (adhocInterfaces.GetAddress (i+1)));
	    	     app->SetAttribute ("Verbose", BooleanValue (true));
	    	     all.Get(i)->AddApplication (app);
	    	     app->SetStartTime (Seconds (0));
	    	     app->SetStopTime (Seconds (TotalTime) - Seconds (0.001));
	       }







	          Simulator::Stop (Seconds (TotalTime));
	          Simulator::Run ();


}

