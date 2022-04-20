/*
First test
*/


#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"

#include "ns3/config-store-module.h"
#include "ns3/energy-module.h"

#include "ns3/aodv-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/netanim-module.h"
 #include <fstream>
 #include "ns3/core-module.h"
 #include "ns3/csma-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/internet-module.h"
 

//#include "ns3/helloheader.h"
//#include "ns3/infheader.h"


#include <iostream>
#include <string.h>
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("MyFirstAdhoc");
 
/* *************** Set the global variables *************** */
uint32_t packetSize = 512; // bytes //the packet Size
uint32_t numPackets = 1; //the number of packetS each node Sending
double interval = 20.0; // seconds //The interval between two packetS
double simulationTime = 20.0; //seconds, Simualation time

const double maxRange = 250.0; //meters, the transmission range of nodes
const double R=0.046; //J, Energy consumption on reception 
const double T=0.067;//J, Energy consumption on sending 


double Prss = -80;
double offset = 82;

uint32_t nSB = 1;  
uint32_t nCHs = 5; 
uint32_t nCap =1;

NodeContainer SB;
NodeContainer CHs;
NodeContainer cpt1;
NodeContainer cpt2;
NodeContainer cpt3;
NodeContainer cpt4;
NodeContainer cpt5;



NetDeviceContainer Devices_SB;
NetDeviceContainer Devices_CHs;
NetDeviceContainer Devices_cpt1;
NetDeviceContainer Devices_cpt2;
NetDeviceContainer Devices_cpt3;
NetDeviceContainer Devices_cpt4;
NetDeviceContainer Devices_cpt5;

list<int> chList;


void SetStaticRoute(Ptr<Node> n, Ipv4Address destination,Ipv4Address nextHop, uint32_t interface)
{
Ipv4StaticRoutingHelper staticRouting;
Ptr<Ipv4> ipv4 = n->GetObject<Ipv4> ();
Ptr<Ipv4StaticRouting> a = staticRouting.GetStaticRouting (ipv4);
a->SetDefaultRoute(nextHop, interface, 1);
}


/* ****************** The main function ********************** */

/* In thiS example, we create a nWifi nodeS to communicate in ad hoc mode through wireleSS communicationS */
/* Each node iSSueS 5 packetS within 512 byteS Size and 1.0 Second between them */
int 
main (int argc, char *argv[])  
{
   LogComponentEnable ("ChServer",LOG_LEVEL_ALL);
   LogComponentEnable ("UdpClient",LOG_LEVEL_ALL);
   LogComponentEnable ("UdpServer",LOG_LEVEL_ALL);
   LogComponentEnable ("MyFirstAdhoc",LOG_LEVEL_ALL);

 

  std::string phyMode ("DsssRate1Mbps");
  //double Prss = -80;  // -dBm
  //double offset = 81;
  //uint32_t packetSize = 512; // bytes //the packet Size
  //uint32_t numPackets = 1; //the number of packetS each node Sending
  //double interval = 3.0; // seconds //The interval between two packetS
  //double simulationTime = 10.0; //seconds, Simualation time
 
  bool verbose = false;
  
  bool tracing = true;

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi devices",  nCHs);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);
  
  /* *************** Nodes creation *************** */
  cout << "Nodes creation \n";

  SB.Create(nSB);
  CHs.Create(nCHs);
  cpt1.Create(nCap);
  cpt2.Create(nCap);
  cpt3.Create(nCap);
  cpt4.Create(nCap);
  cpt5.Create(nCap);

  /* *************** The Wifi channel configuration *************** */
  cout << "Wifi channel configuration \n";

  WifiHelper wifi;
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
  	wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (maxRange));
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

  
  NetDeviceContainer Devices_SB = wifi.Install (wifiPhy, wifiMac,SB);
  NetDeviceContainer Devices_CHs = wifi.Install (wifiPhy, wifiMac,CHs);
  NetDeviceContainer Devices_cpt1 = wifi.Install (wifiPhy, wifiMac,cpt1);
  NetDeviceContainer Devices_cpt2= wifi.Install (wifiPhy, wifiMac,cpt2);
  NetDeviceContainer Devices_cpt3= wifi.Install (wifiPhy, wifiMac,cpt3);
  NetDeviceContainer Devices_cpt4 = wifi.Install (wifiPhy, wifiMac,cpt4);
  NetDeviceContainer Devices_cpt5= wifi.Install (wifiPhy, wifiMac,cpt5);

  /* *************** The mobility helper *************** */
  cout << "Mobility helper configuration \n";
  MobilityHelper mobility_SB;
    
  Ptr<ListPositionAllocator>PositionAlloc_SB =CreateObject<ListPositionAllocator>();
  PositionAlloc_SB -> Add(Vector(200,200,0));
  mobility_SB.SetPositionAllocator(PositionAlloc_SB);
  mobility_SB.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility_SB.Install(SB);
  
  /***********************mobility helper CHs*********/
  MobilityHelper mobility_CHs;

  Ptr<ListPositionAllocator>PositionAlloc_CHs=CreateObject<ListPositionAllocator>();
  PositionAlloc_CHs-> Add(Vector(100,100,0.0));
  PositionAlloc_CHs-> Add(Vector(100,300,0.0));
  PositionAlloc_CHs-> Add(Vector(300,100,0.0));
  PositionAlloc_CHs-> Add(Vector(300,300,0.0));
  PositionAlloc_CHs-> Add(Vector(250,350,0.0));
  mobility_CHs.SetPositionAllocator(PositionAlloc_CHs);
  mobility_CHs.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility_CHs.Install(CHs);

  /***********************mobility helper cpt*********/
   MobilityHelper mobility_cpt1;
   mobility_cpt1.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("100.0"),
                                  "Y", StringValue ("100.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=50|Max=100]"));
   mobility_cpt1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility_cpt1.Install (cpt1);
   
   MobilityHelper mobility_cpt2;
   mobility_cpt2.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("100.0"),
                                  "Y", StringValue ("300.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=50|Max=100]"));
   mobility_cpt2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility_cpt2.Install (cpt2);
   
   MobilityHelper mobility_cpt3;
   mobility_cpt3.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("300.0"),
                                  "Y", StringValue ("100.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=50|Max=100]"));
   mobility_cpt3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility_cpt3.Install (cpt3);
   
   MobilityHelper mobility_cpt4;
   mobility_cpt4.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("300.0"),
                                  "Y", StringValue ("300.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=50|Max=100]"));
   mobility_cpt4.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility_cpt4.Install (cpt4);
   
   MobilityHelper mobility_cpt5;
   mobility_cpt5.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                  "X", StringValue ("250.0"),
                                  "Y", StringValue ("350.0"),
                                  "Rho", StringValue ("ns3::UniformRandomVariable[Min=50|Max=100]"));
   mobility_cpt5.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   mobility_cpt5.Install (cpt5);
   
   
  
  
  
  

  /* *************** Energy Model *************** */
  cout << "Enabling the energy model \n";
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper_SB;
  BasicEnergySourceHelper basicSourceHelper_CHs;
  BasicEnergySourceHelper basicSourceHelper_cpt;
   
  // configure energy source
  basicSourceHelper_SB.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (100));
  basicSourceHelper_CHs.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (50));
  basicSourceHelper_cpt.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (20));
  // install source
  EnergySourceContainer sources_SB =   basicSourceHelper_SB.Install(SB);
  EnergySourceContainer sources_CHs =  basicSourceHelper_CHs.Install(CHs);
  EnergySourceContainer sources_cpt1 = basicSourceHelper_cpt.Install(cpt1);
  EnergySourceContainer sources_cpt2 = basicSourceHelper_cpt.Install (cpt2);
  EnergySourceContainer sources_cpt3 = basicSourceHelper_cpt.Install(cpt3);
  EnergySourceContainer sources_cpt4 = basicSourceHelper_cpt.Install(cpt4);
  EnergySourceContainer sources_cpt5 = basicSourceHelper_cpt.Install (cpt5);
  
  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // configure radio energy model
  //radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0174));
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (T));
  radioEnergyHelper.Set ("RxCurrentA", DoubleValue (R));
  radioEnergyHelper.Set ("IdleCurrentA", DoubleValue(0.01));
  radioEnergyHelper.Set ("CcaBusyCurrentA", DoubleValue(0.01));
  radioEnergyHelper.Set ("SwitchingCurrentA", DoubleValue(0.01));
  radioEnergyHelper.Set ("SleepCurrentA", DoubleValue(0.001));
  // install device model
  DeviceEnergyModelContainer deviceModels_SB = radioEnergyHelper.Install (Devices_SB,sources_SB);
  DeviceEnergyModelContainer deviceModels_CHs = radioEnergyHelper.Install (Devices_CHs,sources_CHs);
  DeviceEnergyModelContainer deviceModels_cpt1 = radioEnergyHelper.Install (Devices_cpt1,sources_cpt1);
  DeviceEnergyModelContainer deviceModels_cpt2 = radioEnergyHelper.Install (Devices_cpt2,sources_cpt2);
  DeviceEnergyModelContainer deviceModels_cpt3 = radioEnergyHelper.Install (Devices_cpt3,sources_cpt3);
  DeviceEnergyModelContainer deviceModels_cpt4= radioEnergyHelper.Install (Devices_cpt4,sources_cpt4);
  DeviceEnergyModelContainer deviceModels_cpt5 = radioEnergyHelper.Install (Devices_cpt5,sources_cpt5);
  /* *************** protocol stack *************** */


  InternetStackHelper stack;

  stack.Install (SB);
  stack.Install (CHs);
  stack.Install (cpt1);
  stack.Install (cpt2);
  stack.Install (cpt3);
  stack.Install (cpt4);
  stack.Install (cpt5);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interface_SB =  address.Assign (Devices_SB);
  Ipv4InterfaceContainer interface_CHs = address.Assign (Devices_CHs);
  Ipv4InterfaceContainer interface_cpt1 = address.Assign (Devices_cpt1);
  Ipv4InterfaceContainer interface_cpt2 = address.Assign (Devices_cpt2);
  Ipv4InterfaceContainer interface_cpt3 = address.Assign (Devices_cpt3);
  Ipv4InterfaceContainer interface_cpt4 = address.Assign (Devices_cpt4);
  Ipv4InterfaceContainer interface_cpt5 = address.Assign (Devices_cpt5);
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  

  cout << "Internet protocol stack installation \n";

 
 for (uint32_t i=0;i>nCap;i++){ 
 Ptr<Node> capteurX = cpt1.Get(i); 
 SetStaticRoute(capteurX,SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),CHs.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);}
 SetStaticRoute(CHs.Get(0),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);
 
 
 for (uint32_t i=0;i>nCap;i++){ 
 Ptr<Node> capteurX = cpt2.Get(i); 
 SetStaticRoute(capteurX,SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),CHs.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);}
 //SetStaticRoute(CHs.Get(1),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);
 
 
 for (uint32_t i=0;i>nCap;i++){ 
 Ptr<Node> capteurX = cpt3.Get(i); 
 SetStaticRoute(capteurX,SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),CHs.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);}
 SetStaticRoute(CHs.Get(2),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);
 
 
 for (uint32_t i=0;i>nCap;i++){ 
 Ptr<Node> capteurX = cpt4.Get(i); 
 SetStaticRoute(capteurX,SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),CHs.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);}
 SetStaticRoute(CHs.Get(3),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);
 
 
 for (uint32_t i=0;i>nCap;i++){ 
 Ptr<Node> capteurX = cpt5.Get(i); 
 SetStaticRoute(capteurX,SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),CHs.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);}
 SetStaticRoute(CHs.Get(4),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(),1);


   cout << "Create UdpServer application on node SB.\n";
   NS_LOG_INFO ("Create UdpServer application on node SB.");
   uint16_t port = 8050;
   UdpServerHelper server (port);
   ApplicationContainer apps = server.Install(SB.Get(0));
   apps.Start (Seconds (1.0));
   apps.Stop (Seconds (10.0));

   Address serverAddress_ch1 = Address (interface_CHs.GetAddress (0));
   Address serverAddress_ch2 = Address (interface_CHs.GetAddress (1));
   Address serverAddress_ch3 = Address (interface_CHs.GetAddress (2));
   Address serverAddress_ch4 = Address (interface_CHs.GetAddress (3));
   Address serverAddress_ch5 = Address (interface_CHs.GetAddress (4));
   
  
   cout <<"Create UdpClient application on node cpt1.\n";
   uint32_t MaxPacketSize = 1024;
   Time interPacketInterval = Seconds (0.05);
   uint32_t maxPacketCount = 320;
   UdpClientHelper client_1 ( SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port);
   client_1.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
   client_1.SetAttribute ("Interval", TimeValue (interPacketInterval));
   client_1.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
   ApplicationContainer apps2 = client_1.Install (cpt1);
   apps2.Start (Seconds (2.0));
   apps2.Stop (Seconds (10.0));

   NS_LOG_INFO ("SB Ip Address is "<<SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal());
   
   UdpClientHelper client_2 ( SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port);
   client_2 .SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
   client_2 .SetAttribute ("Interval", TimeValue (interPacketInterval));
   client_2 .SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
   ApplicationContainer apps3 = client_2.Install (cpt2);
   apps3.Start (Seconds (2.0));
   apps3.Stop (Seconds (10.0));
   
    UdpClientHelper client_3 ( SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port);
   client_3.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
   client_3.SetAttribute ("Interval", TimeValue (interPacketInterval));
   client_3.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
   ApplicationContainer apps4 = client_3.Install (cpt3);
   apps4.Start (Seconds (2.0));
   apps4.Stop (Seconds (10.0));
   
    UdpClientHelper client_4 ( SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port);
   client_4.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
   client_4.SetAttribute ("Interval", TimeValue (interPacketInterval));
   client_4.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
   ApplicationContainer apps5 = client_4.Install (cpt4);
   apps5.Start (Seconds (2.0));
   apps5.Stop (Seconds (10.0));
   
    UdpClientHelper client_5 ( SB.Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), port);
   client_5.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
   client_5.SetAttribute ("Interval", TimeValue (interPacketInterval));
   client_5.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));
   ApplicationContainer apps6 = client_5.Install (cpt5);
   apps6.Start (Seconds (2.0));
   apps6.Stop (Seconds (10.0));
   
   
  
   
  
   AnimationInterface anim ("test.xml");
   
 


Simulator::Stop (Seconds (simulationTime));

  cout <<"Starting simulation \n";
  Simulator::Run ();
  Simulator::Destroy ();
  cout << "Simulation finished !!!!! \n";
  return 0;
}
