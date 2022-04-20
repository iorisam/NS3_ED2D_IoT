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
NS_LOG_COMPONENT_DEFINE ("Need-simulation");


NodeContainer all ;
double ueTxPower = 8.0;

bool verbose = false;


double Prss = -80;
double offset = 82;
int nWifis = 60;
uint32_t port=9;

double TotalTime = 100.0;
std::string rate ("2048bps");
std::string phyMode ("DsssRate11Mbps");
std::string tr_name ("need-routing");

int nodeSpeed = 1; //in m/s
int nodePause = 0; //in s
std::string m_protocolName = "protocol";
double txp =2.0;

ApplicationContainer NeedApps;
EnergySourceContainer sources;
DeviceEnergyModelContainer deviceModels;
double initEnergy = 20.0;
double first_noeud_to_die = 0;

double connectivity_percent = 0;
double nbcycles = 0;
void DepletionHandler(void){
	NS_LOG_INFO ("depletion func");
	if(first_noeud_to_die == 0.0){
		first_noeud_to_die =  Simulator::Now ().GetSeconds ();
		std::cout<<"first node deplited its energy after " <<first_noeud_to_die<<std::endl;
	}

	NS_LOG_INFO (" out of depletion func");
}

void generateTraffic(){

	NS_LOG_INFO ("generate Traffic func");


}

int numberOfConnectedNodes(){
	std::list<int> openList;
	std::list<int> closedList;

	for(int i = 1; i<nWifis; i++){
		openList.push_back(i);
	}
	closedList.push_back(0);

	for (int i: closedList){
		for (int j: openList){
		Ptr<Need_WiFi_App> app = DynamicCast<Need_WiFi_App>(NeedApps.Get(j));
		NS_LOG_INFO(" ++-+cluster head of node "<<j<<" is node "<<*(app->getClusterHead()));
		if(std::find(closedList.begin(),closedList.end(),app->cluster_head)!=closedList.end()){
			closedList.push_back(j);
		}
	}
	}
	return closedList.size()-1;

}

void testConnected(){


	 			connectivity_percent += numberOfConnectedNodes()*100/(double)(nWifis-1);
	 			nbcycles++;
NS_LOG_INFO("##################################################percentage of connectivity is "
		<<(double)connectivity_percent<<" and nbcycles is "
							  <<nbcycles);
				  Simulator:: Schedule (Seconds(10.0), &testConnected);
}



int
main (int argc, char *argv[])
{
	 LogComponentEnable ("Need-simulation", LOG_LEVEL_ALL);
	 //LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
	 //LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);
	 LogComponentEnable ("need-wifi-app", LOG_LEVEL_ALL);
	CommandLine cmd;
	cmd.AddValue ("nUE", "Print time progress.", nWifis);
	cmd.Parse(argc, argv);
	time_t seconds;

	seconds = time (NULL);
	RngSeedManager::SetSeed (seconds);

	//create nodes and node containers
	all.Create(nWifis);

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
	wifiPhy.EnableAsciiAll(eventTraces.CreateFileStream("NeedWiFi.tr"));
	NetDeviceContainer adhocDevices = wifi.Install (wifiPhy,wifiMac, all);

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
			"X",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"),
			"Y",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=200.0]"));
	staticnodes2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	staticnodes2.Install(all);


	// install internet stack
	InternetStackHelper internet;

	//	  AodvHelper aodv;
	//      internet.SetRoutingHelper(aodv);
	internet.Install(all);

	// assign ip addresses
	Ipv4AddressHelper addressAdhoc;

	addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer adhocInterfaces;
	adhocInterfaces = addressAdhoc.Assign (adhocDevices);

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
	Ptr<EnergySource> sinksource = sources.Get(0);
//	sinksource->SetAttribute("LiIonEnergySourceInitialEnergyJ",DoubleValue (3000.0));
	//EnergySourceContainer sources = basicSourceHelper.Install (all);
	//EnergySourceContainer sources2 = basicSourceHelper.Install (sensors2);

//	for(int i =0 ;i<sources.GetN(); i++){
//		Ptr<EnergySource> energysource = sources.Get(i);
//		//energysource->SetNode(all.Get(i));
//		all.Get(i)->AggregateObject(energysource);
//	}




	/* device energy model */
	WifiRadioEnergyModelHelper radioEnergyHelper;
	// configure radio energy model
	radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.5));
	radioEnergyHelper.Set("RxCurrentA",DoubleValue(0.5));
	WifiRadioEnergyModel::WifiRadioEnergyDepletionCallback callback =
			MakeCallback (&DepletionHandler);
	radioEnergyHelper.SetDepletionCallback(callback);


	Ipv4InterfaceAddress iface = Ipv4InterfaceAddress(adhocInterfaces.GetAddress(0,0),"255.255.255.0");

	// install device model
	deviceModels = radioEnergyHelper.Install (adhocDevices, sources);


	NeedWifiAppHelper needAppsHelper(iface.GetBroadcast(), 9);
	needAppsHelper.SetAttribute ("MaxPackets", UintegerValue (5000000));


	needAppsHelper.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	needAppsHelper.SetAttribute ("PacketSize", UintegerValue (1024));
	NeedApps = needAppsHelper.Install(all);
	NeedApps.Start(Seconds (1.0));
	NeedApps.Stop (Seconds (TotalTime));

	 Ptr<Ipv4> ipv4 = all.Get(0)->GetObject<Ipv4> ();
	 Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
	 Ipv4Address sinkIp = iaddr.GetLocal();
	 UdpClientHelper trafficAppsHelper (sinkIp,8050);

	 trafficAppsHelper.SetAttribute("MaxPackets", UintegerValue (5000000));
	 trafficAppsHelper.SetAttribute("PacketSize", UintegerValue (100));
	 NodeContainer clients;
	 for(int i = 1; i<all.GetN();i++){
		 clients.Add(all.Get(i));

	 }
	 ApplicationContainer clientsapps = trafficAppsHelper.Install(clients);
	 for(int i = 0; i<clientsapps.GetN();i++){
		 Ptr<UdpClient> app = DynamicCast<UdpClient>(clientsapps.Get(i));
		 double k = .0001*(rand() % 1000)+0.9;
		 app->SetAttribute("Interval",TimeValue (Seconds (k)));
	 }
	 clientsapps.Start(Seconds (8.0));
	 clientsapps.Stop (Seconds (TotalTime));
	 InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8050);
	 PacketSinkHelper sinkAppHelper ("ns3::UdpSocketFactory",local);
	 ApplicationContainer sinkApp = sinkAppHelper.Install(all.Get(0));
	 sinkApp.Start(Seconds (1.0));
	 sinkApp.Stop (Seconds (TotalTime));
	 //Simulator:: Schedule (Seconds(9), &testConnected);
	Simulator::Stop (Seconds (TotalTime));
	Simulator::Run ();

	//NS_LOG_LOGIC("average CDS number is " << sum/count << " and Max is  " << MaxNCDS<< std::endl);
				  double delivered_packets = 0;
				  double packets_sent = 0;
				  Ptr<PacketSink> s_app = DynamicCast<PacketSink>(sinkApp.Get(0));
				  delivered_packets = s_app->GetTotalRx();
				  double battery_average= 0.0;

				  for(int i = 0; i<clientsapps.GetN();i++){
					  Ptr<UdpClient> c_app = DynamicCast<UdpClient>(clientsapps.Get(i));
					  packets_sent += c_app->GetNSentPackets();
					  Ptr<EnergySourceContainer> EnergySourceContainerOnNode = clients.Get(i)->GetObject<EnergySourceContainer> ();
					  Ptr<LiIonEnergySource> energysource = DynamicCast<LiIonEnergySource> (EnergySourceContainerOnNode->Get(0));
					  double residual_energy = energysource->GetRemainingEnergy();

					  NS_LOG_LOGIC(" remaining energy for node "<< clients.Get(i)->GetId() << " is : "<< residual_energy);
					  battery_average += residual_energy;
				  }
				  battery_average = battery_average/all.GetN();
				  double variance = 0;
				  for(int i = 0; i<clientsapps.GetN();i++){
				  Ptr<EnergySourceContainer> EnergySourceContainerOnNode = clients.Get(i)->GetObject<EnergySourceContainer> ();
				  Ptr<LiIonEnergySource> energysource = DynamicCast<LiIonEnergySource> (EnergySourceContainerOnNode->Get(0));
				  double residual_energy = energysource->GetRemainingEnergy();
				  variance += pow(residual_energy - battery_average, 2);
				  }
				  variance=variance/5;
				  double stdDeviation = sqrt(variance);
				  AsciiTraceHelper ascii1;
				  Ptr<OutputStreamWrapper> stream2 = ascii1.CreateFileStream ( "needsimu.tr",std::ios::app);
				  *stream2->GetStream () << " Number of nodes \t R_Energy \t Delivery \t Efficiency \t stdDeviation"<<std::endl;

				  NS_LOG_LOGIC(" delivered packets are "<<delivered_packets<<" and sink remaining energy is "<<sinksource->GetRemainingEnergy());
		          double ratio = double(packets_sent)/delivered_packets;

		          NS_LOG_LOGIC(" delivery ratio is " << ratio<< " and average energy is "<< battery_average<< " and first node died at "<<first_noeud_to_die);

		          *stream2->GetStream () <<all.GetN()<<" \t "<< battery_average<<" \t "<<ratio<<" \t "<<ratio/battery_average<<" \t "<<stdDeviation<<" \t "<<std::endl;

}

