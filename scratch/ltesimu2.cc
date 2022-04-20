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
#include <ns3/BloomFilter.hpp>

#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/cost231-propagation-loss-model.h>
#include <stdio.h>
#include <time.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Dominating-sets-simulation");

NodeContainer all;
NodeContainer CDS;
NodeContainer sensors;
NodeContainer sensors2;
NodeContainer ueResponders;

uint32_t mcs = 10;
uint32_t rbSize = 8;
uint32_t ktrp = 1;
uint32_t pscchLength = 16;
std::string period = "sf80";
double dist = 10;

bool verbose = false;

int nWifis = 50;

int nDS = 50;
uint32_t ueCount = nDS;
int nLR = 50;
std::map<int, int> packets_tranmission_time;
double ueTxPower = 23.0;
std::list<uint32_t> covered_nodes;

uint32_t port = 9;
double TotalTime = 100.0;
std::string rate("2048bps");
std::string phyMode("DsssRate11Mbps");
std::string tr_name("manet-routing-compare");
double nodeSpeed = 1; //in m/s
int number_of_types_of_services = 4;
int nodePause = 0; //in s
std::string m_protocolName = "protocol";
double txp = 23.0;

uint32_t Rid1;
uint32_t Rid2;

int numberCDS = 0;
int covered_percent = 0;
int average_percent = 0;
int numberhops = 0;
int numberrequest = 0;
int numberdelevery = 0;
int counthops = 0;
int MaxNCDS = 0;
int count = 1;
int sum = 0;
double transmissiondelay = 0;
std::map<int, double> myrequests;
std::list<double> mysentrequests;
std::map<int, double> myreceives;
ApplicationContainer discoveryapps;
ApplicationContainer LteRequestsApps;
std::map<double, std::list<double>> globaltable;
double coverage_percent = 0;
double connectivity_percent = 0;
double nbcycles = 0;
double first_noeud_to_die = 0;

std::list<int> serviceTypes;

Ptr<Socket> SetupPacketReceive(Ipv4Address addr, Ptr<Node> node) {
	TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
	Ptr<Socket> sink = Socket::CreateSocket(node, tid);
	InetSocketAddress local = InetSocketAddress(addr, port);
	sink->Bind(local);
	return sink;
}

void IntTrace(uint32_t oldValue, uint32_t newValue) {
	if (oldValue != newValue) {
		if (newValue > 0) {
			numberCDS++;
			if (numberCDS > MaxNCDS)
				MaxNCDS = numberCDS;
		} else
			numberCDS--;
	}
}

void update_my_sent_packets(double packetid){
	bool found = (std::find(mysentrequests.begin(), mysentrequests.end(), packetid) != mysentrequests.end());
	if(!found)
		mysentrequests.push_back(packetid);
}

void TraceHops(uint32_t oldValue, uint32_t newValue) {
	if (oldValue != newValue)
		if (newValue > 0) {
			numberhops += newValue;
			counthops++;
		}
}

void TraceRequests(uint32_t oldValue, uint32_t newValue) {
	if (oldValue != newValue)
		if (newValue > 0) {
			numberrequest++;

		}

}

void TracePackets(uint32_t oldValue, uint32_t newValue) {

	if (oldValue != newValue)
		if (newValue > 0) {
			numberdelevery++;
		}
}

double TracePacketstart(uint32_t packetid, double sent_time) {
	std::pair<int, double> entry;
	entry.first = packetid;
	entry.second = sent_time;
	myrequests.insert(entry);

	return sent_time;
}

double TracePacketsDate(uint32_t paketid, double receive_time) {
	std::pair<int, double> entry;
	entry.first = paketid;
	entry.second = receive_time;
	myreceives.insert(entry);
	std::map<int, double>::iterator it;
	for (it = myrequests.begin(); it != myrequests.end(); it++) {

		if (it->first == paketid) {
			transmissiondelay += receive_time - it->second;
			break;

		}
	}
	return receive_time;
}

void counterf() {
	sum += numberCDS;
	count++;
	std::cout << "number of cds now is " << numberCDS << " number of cycles is"
			<< count << "sum is " << sum << std::endl;
	Simulator::Schedule(Seconds(2.0), &counterf);
	covered_percent = covered_nodes.size() * 100 / (nWifis + nLR);
	average_percent += covered_percent;
	covered_nodes.clear();

}

void generateTraffic() {
	std::cout << "we are going to generate traffic " << numberrequest
			<< std::endl;
	Ptr<UniformRandomVariable> generator =
			CreateObject<UniformRandomVariable>();
	double rnd_emiter_node = (double) generator->GetInteger(1, nDS - 1);
	uint32_t rnd_receiver_node = (double) generator->GetInteger(nDS, nDS + nLR);
	uint32_t packet_id = generator->GetInteger(0, 50000);

	int x = (int) rnd_emiter_node;
	std::cout << "generated packet id is : " << packet_id<< " emiter node is  " << x << " receiver node is "
			<< (double) rnd_receiver_node << std::endl;
	Ptr<Lte_Echo_Client> lteapp = DynamicCast<Lte_Echo_Client>(
			LteRequestsApps.Get((double) rnd_emiter_node));
	int servicetype = 0;
	   std::vector<int> myVector(serviceTypes.begin(), serviceTypes.end());
	   if(rnd_receiver_node>nDS-1){
		   servicetype = myVector[rnd_receiver_node-nDS];
		   if(servicetype>number_of_types_of_services)
			   servicetype = 0;
				   }
	std::cout << "generated a traffic to  " << rnd_receiver_node << std::endl;
	lteapp->Request(rnd_receiver_node, packet_id,servicetype);
	update_my_sent_packets((double)packet_id);
	Simulator::Schedule(Seconds(1.0), &generateTraffic);

}

void coveredNodes(std::list<int> newlist) {

	for (int i : newlist) {
		if (std::find(covered_nodes.begin(), covered_nodes.end(), i)
				== covered_nodes.end())
			covered_nodes.push_back(i);
	}

}

std::list<double> getNeighbors(std::map<Address, neighbor> table) {
	std::list<double> neighbors;
	std::map<Address, neighbor>::iterator it;
	for (it = table.begin(); it != table.end(); it++) {
		neighbors.push_back((double) it->second.node_id);
	}
	return neighbors;
}

bool coveredbydevice(std::map<Address, neighbor> table) {
	std::map<Address, neighbor>::iterator it;
	for (it = table.begin(); it != table.end(); it++) {
		if (it->second.neighbors.empty())
			return true;
	}
	return false;
}

void populateGlobaltable() {
	std::map<double, std::list<double>>::iterator it;
	globaltable.clear();
	double temp = 0;
	for (int i = 0; i < nDS; i++) {

		Ptr<Lte_Echo_Client> lteapp = DynamicCast<Lte_Echo_Client>(
				LteRequestsApps.Get(i));
		globaltable.insert(
				std::pair<double, std::list<double>>((double) i,
						getNeighbors(lteapp->MyTable)));
		if (coveredbydevice(lteapp->MyTable))
			temp = temp + 1;

	}
	coverage_percent += (double) ((double) temp / (double) nDS);
	nbcycles += 1;
	std::cout
			<< "$$$$$$$$$$$$$$$$$$$$$$$$$ coverage percentage is $$$$$$$$$$$$$$$$$$$$"
			<< coverage_percent << std::endl;
	Simulator::Schedule(Seconds(1.0), &populateGlobaltable);
}

void testConnected() {
	Ptr<UniformRandomVariable> generator =
			CreateObject<UniformRandomVariable>();
	double rnd_emiter_node = (double) generator->GetInteger(1, nDS - 1);
	Astar aster = Astar(ueResponders, globaltable);
	int percentage = 0;
	for (int i = 1; i < nDS; i++) {
		if (aster.findpath(rnd_emiter_node, (double) i))
			percentage = percentage + 1;
		else
			std::cout
					<< "################################################## no route found between the two nodes "
					<< rnd_emiter_node << " and " << i << std::endl;

	}
	connectivity_percent += (double) ((double) percentage * 100
			/ (double) (nDS - 1));
	std::cout
			<< "##################################################  percentage of connectivity is "
			<< connectivity_percent << std::endl;

	Simulator::Schedule(Seconds(1.0), &testConnected);
}

void SlStartDiscovery(Ptr<NistLteHelper> helper, Ptr<NetDevice> ue,
		std::list<std::string> apps, bool rxtx) {
	helper->StartDiscovery(ue, apps, rxtx);
}

void SlStopDiscovery(Ptr<NistLteHelper> helper, Ptr<NetDevice> ue,
		std::list<std::string> apps, bool rxtx) {
	helper->StopDiscovery(ue, apps, rxtx);
}

void DiscoveryMonitoringTrace(Ptr<OutputStreamWrapper> stream, uint64_t imsi,
		uint16_t cellId, uint16_t rnti, std::string proSeAppCode) {

	Ptr<DiscApp> discapp = DynamicCast<DiscApp>(discoveryapps.Get(imsi - 1));
	Ptr<DiscApp> discapp2 = DynamicCast<DiscApp>(discoveryapps.Get(rnti - 1));
	bloom_filter filter = discapp2->m_filter;

	discapp->NotifyDiscoveryMessage(rnti, proSeAppCode, filter);

}

void DiscoveryAnnouncementPhyTrace(Ptr<OutputStreamWrapper> stream,
		std::string imsi, uint16_t cellId, uint16_t rnti,
		std::string proSeAppCode) {
	//  std::cout << Simulator::Now ().GetSeconds () << "\t" << imsi << "\t" << cellId << "\t"  << rnti << "\t" << proSeAppCode << std::endl;
}

void DiscoveryAnnouncementMacTrace(Ptr<OutputStreamWrapper> stream,
		std::string imsi, uint16_t rnti, std::string proSeAppCode) {
	// std::cout << Simulator::Now ().GetSeconds () << "\t" << imsi << "\t" << rnti << "\t" << proSeAppCode << std::endl;
}

void DepletionHandler(double node) {
	if (first_noeud_to_die == 0.0 && node > 1) {
		first_noeud_to_die = Simulator::Now().GetSeconds();
	}
	std::cout << " node " << node << " has no energy left at : "
			<< Simulator::Now().GetSeconds() << std::endl;

}



int main(int argc, char *argv[]) {
//		 LogComponentEnable ("NistLteSpectrumPhy", LOG_LEVEL_INFO);
//		 LogComponentEnable ("ZigbeeEnergyModel", LOG_LEVEL_ALL);
//	  LogComponentEnable ("lte-echo-app", LOG_LEVEL_ALL);
//LogComponentEnable ("Dominating-sets-simulation", LOG_LEVEL_ALL);
//     LogComponentEnable ("Dominating-sets-simulation", LOG_LEVEL_ALL);

//	 LogComponentEnable ("Lr6Application", LOG_LEVEL_ALL);

//	LogComponentEnable ("RequestResponseServerApplication", LOG_LEVEL_ALL);
//	 LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);
//	LogComponentEnable("lte_echo_server", LOG_LEVEL_ALL);
//	 LogComponentEnable ("LRServerApplication", LOG_LEVEL_ALL);
//
//	 LogComponentEnable("NistLteUeMac", LOG_LEVEL_ALL);
//
//	LogComponentEnable ("NistLteUePhy", LOG_LEVEL_ALL);
//
//	LogComponentEnable ("NistLteSpectrumPhy", LOG_LEVEL_ALL);
//	LogComponentEnable ("NistLteUeRrc", LOG_LEVEL_ALL);
//	LogComponentEnable ("NistLteEnbPhy", LOG_LEVEL_ALL);
//	LogComponentEnable ("NistLteUeMac", LOG_LEVEL_ALL);
//	LogComponentEnable ("NistEpcHelper", LOG_LEVEL_ALL);
//	LogComponentEnable ("NistEpcHelper", LOG_LEVEL_ALL);
//	LogComponentEnable ("NistLteHelper", LOG_LEVEL_ALL);
//	LogComponentEnable ("NistPointToPointEpcHelper", LOG_LEVEL_ALL);
//    LogComponentEnable ("LRServerPushApplication", LOG_LEVEL_ALL);
//    LogComponentEnable ("Lr6ApplicationPush", LOG_LEVEL_ALL);

	//LogComponentEnable ("NistLteSpectrumPhy", LOG_LEVEL_ALL);

	//LogComponentEnable ("NistLteUePhy", LOG_LEVEL_ALL);

	CommandLine cmd;
	cmd.AddValue("period", "Sidelink period", period);
	cmd.AddValue("pscchLength", "Length of PSCCH.", pscchLength);
	cmd.AddValue("ktrp", "Repetition.", ktrp);
	cmd.AddValue("mcs", "MCS.", mcs);
	cmd.AddValue("rbSize", "Allocation size.", rbSize);
	cmd.AddValue("verbose", "Print time progress.", verbose);
	cmd.AddValue("nUE", "number of UEs", nWifis);
	cmd.AddValue("nLR", "number of UEs", nLR);
	cmd.AddValue("nodeSpeed", "number of UEs", nodeSpeed);
	cmd.AddValue("nbS", "number of UEs", number_of_types_of_services);

	cmd.Parse(argc, argv);

	NS_LOG_INFO("Configuring UE settings...");
	Config::SetDefault("ns3::NistLteUeMac::SlGrantMcs", UintegerValue(mcs));
	Config::SetDefault("ns3::NistLteUeMac::SlGrantSize", UintegerValue(rbSize));
	Config::SetDefault("ns3::NistLteUeMac::Ktrp", UintegerValue(ktrp));
	Config::SetDefault("ns3::NistLteUePhy::TxPower", DoubleValue(ueTxPower));
	Config::SetDefault("ns3::NistLteUePowerControl::Pcmax",
			DoubleValue(ueTxPower));
	Config::SetDefault("ns3::NistLteUePowerControl::PscchTxPower",
			DoubleValue(ueTxPower));
	Config::SetDefault("ns3::NistLteUePowerControl::PsschTxPower",
			DoubleValue(ueTxPower));

	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults();
	time_t seconds;

	  seconds = time (NULL);
	RngSeedManager::SetSeed (seconds);
	nDS = nWifis;


	NS_LOG_INFO("Creating helpers...");
	Ptr<NistPointToPointEpcHelper> epcHelper = CreateObject<
			NistPointToPointEpcHelper>();
	Ptr<Node> pgw = epcHelper->GetPgwNode();

	Ptr<NistLteHelper> lteHelper = CreateObject<NistLteHelper>();

	lteHelper->SetAttribute("UseSidelink", BooleanValue(true));
	lteHelper->SetEpcHelper(epcHelper);
	lteHelper->DisableNewEnbPhy();
	lteHelper->SetPathlossModelType("ns3::Cost231PropagationLossModel");
	lteHelper->Initialize();

	Ptr<NistLteProseHelper> proseHelper = CreateObject<NistLteProseHelper>();
	proseHelper->SetLteHelper(lteHelper);

	NS_LOG_INFO("Deploying UE's...");

	//create nodes and node containers
	all.Create(nWifis);

	for (int i = 0; i < nDS; i++) {
		CDS.Add(all.Get(i));
		ueResponders.Add(all.Get(i));
	}
	for (int i = nDS; i < nWifis; i++) {
		sensors.Add(all.Get(i));
	}

	sensors2.Create(nLR);

//set up LTE

	// set the phyisical layer and channel model with propagation delay and pathloss , data rate n controlMode, and transmission power,
	// and ap or adhoc mode,

	// set the node position and mobility model.

	MobilityHelper mobilityAdhoc;
	int64_t streamIndex = 0; // used to get consistent mobility across scenarios

	ObjectFactory pos;
	pos.SetTypeId("ns3::RandomRectanglePositionAllocator");
	pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"));
	pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"));

	Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<
			PositionAllocator>();
	streamIndex += taPositionAlloc->AssignStreams(streamIndex);

	std::stringstream ssSpeed;
	ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
	std::stringstream ssPause;
	ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
	mobilityAdhoc.SetMobilityModel("ns3::RandomWaypointMobilityModel", "Speed",
			StringValue(ssSpeed.str()), "Pause", StringValue(ssPause.str()),
			"PositionAllocator", PointerValue(taPositionAlloc));
	mobilityAdhoc.SetPositionAllocator(taPositionAlloc);
	mobilityAdhoc.Install(CDS);
	streamIndex += mobilityAdhoc.AssignStreams(CDS, streamIndex);

	MobilityHelper staticnodes2;

	ObjectFactory posLR;
	posLR.SetTypeId("ns3::RandomRectanglePositionAllocator");
	posLR.Set("X",
			StringValue("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"));
	posLR.Set("Y",
			StringValue("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"));

	staticnodes2.SetPositionAllocator("ns3::RandomRectanglePositionAllocator",
			"X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"),
			"Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"));
	staticnodes2.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	//staticnodes2.Install(sensors);
	staticnodes2.Install(sensors2);

	// install internet stack

	NS_LOG_INFO("Installing UE network devices...");
	NetDeviceContainer ueDevs;
	NetDeviceContainer ueRespondersDevs = lteHelper->InstallUeDevice(
			CDS);
	ueDevs.Add(ueRespondersDevs);

	InternetStackHelper internet;
	internet.Install(all);
	internet.Install(sensors2);

	NS_LOG_INFO("Allocating IP addresses and setting up network route...");
	Ipv4InterfaceContainer ueIpIface;

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	ueIpIface = epcHelper->AssignUeIpv4Address(ueDevs);
	std::cout << "multi ip address is " << ueDevs.GetN();

//	      for (uint32_t u = 0; u < ueResponders.GetN (); ++u)
//	        {
//	          Ptr<Node> ueNode = ueResponders.Get (u);
//	          // Set the default gateway for the UE
//	          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
//	          std::cout<<"node ip address is "<<ueNode->GetObject<Ipv4> ();
//	          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
//	        }

	NS_LOG_INFO("Attaching UE's to LTE network...");
	lteHelper->Attach(ueDevs);

	Ipv4InterfaceAddress iface = Ipv4InterfaceAddress(
			ueIpIface.GetAddress(0, 0), "255.0.0.0");

	Ipv4Address groupAddress("225.0.0.0");
//  for (uint32_t u = 0; u < ueResponders.GetN (); ++u)
//    {
//
//      Ptr<Node> ueNode = ueResponders.Get (u);
//      // Set the default gateway for the UE
//      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
//
//      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
//    }
//// lrpan

	LrWpanHelper lrWpanHelper;
	// Add and install the LrWpanNetDevice for each node
	NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(ueResponders);
	NetDeviceContainer lrwpanDevices2 = lrWpanHelper.Install(sensors2);
	NetDeviceContainer AllLrDevices;
	AllLrDevices.Add(lrwpanDevices);
	AllLrDevices.Add(lrwpanDevices2);


	// PAN association and short address assignment.

	lrWpanHelper.AssociateToPan(AllLrDevices, 0);

	SixLowPanHelper sixlowpan;
	NetDeviceContainer lr_devices = sixlowpan.Install(AllLrDevices);

	Ipv6AddressHelper ipv6;
	ipv6.SetBase(Ipv6Address("2001:2::"), Ipv6Prefix(64));
	Ipv6InterfaceContainer lr_interface = ipv6.Assign(lr_devices);



	Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
		 	  	  Ptr<LogDistancePropagationLossModel> model = CreateObject<LogDistancePropagationLossModel> ();

		 	  	  channel->AddPropagationLossModel (model);


		  	for (int i = 0; i < all.GetN(); i++) {
		  		Ptr<LrWpanNetDevice> dev = DynamicCast<LrWpanNetDevice> (AllLrDevices.Get(i));

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
		  		Ptr<LrWpanNetDevice> dev = DynamicCast<LrWpanNetDevice> (AllLrDevices.Get(i+nDS));



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




	// setting up some other lte configurations

	NS_LOG_INFO("Creating sidelink configuration...");
	uint32_t groupL2Address = 0xFF;
	Ptr<NistSlTft> tft = Create<NistSlTft>(NistSlTft::BIDIRECTIONAL,
			iface.GetBroadcast(), groupL2Address);
	proseHelper->ActivateSidelinkBearer(Seconds(1.0), ueDevs, tft);
	Ptr<LteUeRrcSl> ueSidelinkConfiguration = CreateObject<LteUeRrcSl>();
	ueSidelinkConfiguration->SetSlEnabled(true);

	NistLteRrcSap::SlPreconfiguration preconfiguration;
	preconfiguration.preconfigGeneral.carrierFreq = 23330;
	preconfiguration.preconfigGeneral.slBandwidth = 100;
	preconfiguration.preconfigComm.nbPools = 1;
	preconfiguration.preconfigDisc.nbPools = 1;
	preconfiguration.preconfigDisc.pools[0].cpLen.cplen =
			NistLteRrcSap::SlCpLen::NORMAL;
	//this parametre is the period between each discovery round
	preconfiguration.preconfigDisc.pools[0].discPeriod.period =
			NistLteRrcSap::SlPeriodDisc::rf32;
	preconfiguration.preconfigDisc.pools[0].numRetx = 0;
	preconfiguration.preconfigDisc.pools[0].numRepetition = 1;
	preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbNum = 10;
	preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbStart = 10;
	preconfiguration.preconfigDisc.pools[0].tfResourceConfig.prbEnd = 49;
	preconfiguration.preconfigDisc.pools[0].tfResourceConfig.offsetIndicator.offset =
			0;
	preconfiguration.preconfigDisc.pools[0].tfResourceConfig.subframeBitmap.bitmap =
			std::bitset<40>(0x11111);
	preconfiguration.preconfigDisc.pools[0].txParameters.txParametersGeneral.alpha =
			NistLteRrcSap::SlTxParameters::al04;
	preconfiguration.preconfigDisc.pools[0].txParameters.txParametersGeneral.p0 =
			NistLteRrcSap::SlTxParameters::al04;
	preconfiguration.preconfigDisc.pools[0].txParameters.txProbability.probability =
			NistLteRrcSap::TxProbability::p50;
	NistSlPreconfigPoolFactory pfactory;
	//build PSCCH bitmap value
	uint64_t pscchBitmapValue = 0x0;
	for (uint32_t i = 0; i < pscchLength; i++) {
		pscchBitmapValue = pscchBitmapValue >> 1 | 0x8000000000;
	}
	pfactory.SetControlBitmap(pscchBitmapValue);
	pfactory.SetControlPeriod(period);
	pfactory.SetDataOffset(pscchLength);
	preconfiguration.preconfigComm.pools[0] = pfactory.CreatePool();
	ueSidelinkConfiguration->SetSlPreconfiguration(preconfiguration);

	NS_LOG_INFO("Installing sidelink configuration...");
	lteHelper->InstallSidelinkConfiguration(ueRespondersDevs,
			ueSidelinkConfiguration);
	NS_LOG_INFO("Enabling LTE traces...");
	lteHelper->EnableTraces();
	// lteHelper->EnableLogComponents();
	lteHelper->StopDiscovery(ueRespondersDevs, { }, true);

	Ptr<UniformRandomVariable> generator =
			CreateObject<UniformRandomVariable>();

//
	lte_echo_client_helper udpClient(iface.GetBroadcast(), 8000);
	udpClient.SetAttribute("MaxPackets", UintegerValue(5000000));
	udpClient.SetAttribute("Interval", TimeValue(Seconds(0.5)));
	udpClient.SetAttribute("PacketSize", UintegerValue(100));
	udpClient.SetIfIndex(0);

	lte_echo_server_helper udpServer(8000);
	LteRequestsApps = udpClient.Install(ueResponders);

	ApplicationContainer serverApps45 = udpServer.Install(ueResponders);
	serverApps45.Start(Seconds(1.0));
	udpServer.SetIfIndex(0);
	serverApps45.Stop(Seconds(TotalTime));

	DiscAppHelper discapphelper;

	discoveryapps = discapphelper.Install(ueResponders);

	discoveryapps.Start(Seconds(1.0));
	discoveryapps.Stop(Seconds(TotalTime));

	// setting up Energy sources and Energy models for Sensors
	LrWpanEnergySourceHelper lrSourceHelper;
	// configure energy source
	lrSourceHelper.Set("LrWpanEnergySourceInitialEnergyJ", DoubleValue(1000));
	// install source
	EnergySourceContainer lr_sources = lrSourceHelper.Install(sensors2);
	/* device energy model */
	EnergySourceContainer lr_sources2 = lrSourceHelper.Install(CDS);
	ZigbeeEnergyModelHelper zigbeeEnergyHelper;
	// configure radio energy model

	// install device model
	DeviceEnergyModelContainer deviceModels = zigbeeEnergyHelper.Install(
			lrwpanDevices2, lr_sources);
	/***************************************************************************/

	// setting up Energy models for Ues lr-wpan
	// configure energy source
	EnergySourceContainer ue_sources;
	/* device energy model */
	for (int i = 0; i < nDS; i++) {
		Ptr<LiIonEnergySource> ue_source =
				lteHelper->EnergySourcesContainer.find(CDS.Get(i)->GetId())->second->LIES.GetObject<
						LiIonEnergySource>();
		ue_source->SetNode(CDS.Get(i));
		ue_source->LteEnergyDepletionCallback = MakeCallback(&DepletionHandler);
		//std::cout<<"my energy is "<<ue_source->GetInitialEnergy()<<"and remaining energy level is "<<ue_source->GetRemainingEnergy()<<std::endl;
		ue_sources.Add(ue_source);
	}
	// install device model
	//DeviceEnergyModelContainer deviceModels2 = zigbeeEnergyHelper.Install (lrwpanDevices, lr_sources2);

	/***************************************************************************/

	for (int i = 0; i < nDS; i++) {

		Ptr<Lte_Echo_Client> lteapp = DynamicCast<Lte_Echo_Client>(
				LteRequestsApps.Get(i));
		Ptr<lte_echo_server> lteapp2 = DynamicCast<lte_echo_server>(
				serverApps45.Get(i));

		lteapp2->SetTable(&(lteapp->MyTable));
		lteapp2->SetBroadcastAddress(iface.GetBroadcast());
		lteapp2->SetLteClient(&lteapp);
		lteapp2->SetState(&(lteapp->state));
		lteapp2->SetPacketsList(&(lteapp->packets_list));
		lteapp2->SetNeighborsPacketsList(&(lteapp->neighbors_packets_list));
		double rnd_value = generator->GetInteger(8000, 9999) / 10000.0;

		lteapp->SetAttribute("Interval", TimeValue(Seconds(rnd_value)));

		lteapp->my_energy = lteHelper->EnergySourcesContainer.find(
				CDS.Get(i)->GetId())->second;
		lteapp->SetRandomVariableGenerator(generator);
		lteapp->TraceConnectWithoutContext("MyState", MakeCallback(&IntTrace));
		lteapp->TraceConnectWithoutContext("n_packets",
				MakeCallback(&TraceRequests));
		lteapp2->TraceConnectWithoutContext("n_hops", MakeCallback(&TraceHops));
		lteapp2->TraceConnectWithoutContext("n_delivered",
				MakeCallback(&TracePackets));
		lteapp->numberUE = nWifis + nLR;
		lteapp->trace_sending_packet = MakeCallback(&TracePacketstart);
		lteapp2->trace_receiving_packet = MakeCallback(&TracePacketsDate);
		lteapp->trace_receiving_packet = MakeCallback(&TracePacketsDate);
		//lteapp2->SetLteClient(lteapp);
		//std::cout<<"my energy is "<<ue_sources.Get(i)->GetInitialEnergy()<<"and i have left "<<ue_sources.Get(i)->GetEnergyFraction()<<std::endl;
	}

	LteRequestsApps.Start(Seconds(2.5));
	LteRequestsApps.Stop(Seconds(TotalTime));

	//LRServerHelper lrServer (9);

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

	for (int i = 0; i < sensors2.GetN(); i++) {
			Ptr<LRClientPush> lrclient = DynamicCast<LRClientPush>(
					lrClientsApps.Get(i));
			lrclient->SetDevice(lr_devices.Get(i + nDS));
			 Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable> ();
			 int randomint = generator->GetInteger(0,number_of_types_of_services-1) + 1;
			serviceTypes.push_back(randomint) ;
			lrclient->servicetype = randomint;
		}

	for (int i = 0; i < ueResponders.GetN(); i++) {
		Ptr<LRServerPush> lrserver = DynamicCast<LRServerPush>(
				lrserverApps.Get(i));
		Ptr<lte_echo_server> lteappserv = DynamicCast<lte_echo_server>(
				serverApps45.Get(i));
		Ptr<DiscApp> discapp = DynamicCast<DiscApp>(discoveryapps.Get(i));
		//	 std::cout<<"the ip address of such app "<<i-nLR <<" is :"<<lr_devices.Get(i)->GetIfIndex();
		lrserver->SetDevice(lr_devices.Get(i));
		Ptr<Lte_Echo_Client> lteapp = DynamicCast<Lte_Echo_Client>(
				LteRequestsApps.Get(i));
		lrserver->SetTable(&(lteapp->MyTable));
		// lteappserv->SetPacketsv6List(&(lrserver->packetsv6_list));
		//lteapp->SetPacketsv6List(&(lrserver->packetsv6_list));

		discapp->SetTable(&(lteapp->MyTable));
		discapp->SetLteHelper(lteHelper);

		discapp->my_energy = lteHelper->EnergySourcesContainer.find(
				CDS.Get(i)->GetId())->second;
		discapp->oldname = std::bitset<184>(i);
		discapp->state = &(lteapp->state);
		lteapp->count_cover = coveredNodes;
		lteapp->services = serviceTypes;
		lteapp->number_of_types_of_services = number_of_types_of_services;
	}



	AsciiTraceHelper ascii;
	Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream(
			"discovery_out_monitoring.tr");
	*stream->GetStream() << "Time\tIMSI\tCellId\tRNTI\tProSeAppCode"
			<< std::endl;

	AsciiTraceHelper ascii11;
	Ptr<OutputStreamWrapper> stream1 = ascii11.CreateFileStream(
			"discovery_out_announcement_phy.tr");
	*stream1->GetStream() << "Time\tIMSI\tCellId\tRNTI\tProSeAppCode"
			<< std::endl;

	AsciiTraceHelper ascii2;
	Ptr<OutputStreamWrapper> stream22 = ascii11.CreateFileStream(
			"discovery_out_announcement_mac.tr");
	*stream22->GetStream() << "Time\tIMSI\tRNTI\tProSeAppCode" << std::endl;

	AsciiTraceHelper asciiMob;
	MobilityHelper::EnableAsciiAll(
			asciiMob.CreateFileStream("mobility-trace-example.mob"));

	std::ostringstream oss;
	oss.str("");

	for (uint32_t i = 0; i < ueDevs.GetN(); ++i) {
		Ptr<NistLteUeRrc> ueRrc = DynamicCast<NistLteUeRrc>(
				ueDevs.Get(i)->GetObject<NistLteUeNetDevice>()->GetRrc());
		ueRrc->TraceConnectWithoutContext("DiscoveryMonitoring",
				MakeBoundCallback(&DiscoveryMonitoringTrace, stream));
		oss << ueDevs.Get(i)->GetObject<NistLteUeNetDevice>()->GetImsi();
		Ptr<NistLteUePhy> uePhy = DynamicCast<NistLteUePhy>(
				ueDevs.Get(i)->GetObject<NistLteUeNetDevice>()->GetPhy());
		uePhy->TraceConnect("DiscoveryAnnouncement", oss.str(),
				MakeBoundCallback(&DiscoveryAnnouncementPhyTrace, stream1));
		Ptr<NistLteUeMac> ueMac = DynamicCast<NistLteUeMac>(
				ueDevs.Get(i)->GetObject<NistLteUeNetDevice>()->GetMac());
		ueMac->TraceConnect("DiscoveryAnnouncement", oss.str(),
				MakeBoundCallback(&DiscoveryAnnouncementMacTrace, stream22));
		oss.str("");
	}

	std::map<Ptr<NetDevice>, std::list<std::string> > announceApps;
	std::map<Ptr<NetDevice>, std::list<std::string> > monitorApps;
	for (uint32_t i = 1; i <= ueDevs.GetN(); ++i) {
		std::string binary = std::bitset<8>(i).to_string();
		announceApps[ueRespondersDevs.Get(i - 1)].push_back(binary);
		for (uint32_t j = 1; j <= ueDevs.GetN(); ++j) {
			if (i != j) {
				monitorApps[ueRespondersDevs.Get(i - 1)].push_back(
						std::to_string(j));
			}
		}
	}

	 AnimationInterface anim ("wireless-animation101.xml");

	 for (uint32_t i = 0; i < CDS.GetN (); ++i)
		  		        {
		  		          anim.UpdateNodeDescription (CDS.Get (i), "STA"); // Optional
		  		          anim.UpdateNodeColor (CDS.Get (i), 0, 255, 0); // Optional
		  		        }

	for (uint32_t i = 0; i < ueDevs.GetN(); ++i) {
		Simulator::Schedule(Seconds(1.0), &SlStartDiscovery, lteHelper,
				ueRespondersDevs.Get(i),
				announceApps.find(ueDevs.Get(i))->second, true); // true for announce
		Simulator::Schedule(Seconds(1.0), &SlStartDiscovery, lteHelper,
				ueRespondersDevs.Get(i),
				monitorApps.find(ueDevs.Get(i))->second, false); // false for monitor
	}

	Simulator::Schedule(Seconds(2.0), &counterf);
	Simulator::Schedule(Seconds(3.0), &populateGlobaltable);
	Simulator::Schedule(Seconds(3.5), &generateTraffic);
	Simulator::Schedule(Seconds(3.5), &testConnected);
	Simulator::Stop(Seconds(TotalTime));
	Simulator::Run();
	std::cout << "average CDS number is " << (double) sum / count
			<< " and Max is  " << (double) MaxNCDS
			<< " and average percentage of coverage is "
			<< (double) average_percent / count << std::endl;
	double ratio = double(myreceives.size()) / (double) mysentrequests.size();
	double ratiohops = double(numberhops) / counthops;
	double battery_average = 0.0;

	std::cout << "number of request is " << (double) myrequests.size()
			<< " number of delevery is " << (double) numberdelevery
			<< " delevery ratio is  " << (double) ratio << "average hops is "
			<< (double) numberhops << "counts is " << (double) counthops
			<< std::endl;

	for (int i = 0; i < ue_sources.GetN(); i++) {
		std::cout << "node " << i << "has "
				<< ue_sources.Get(i)->GetRemainingEnergy() << "j left "
				<< std::endl;

		battery_average += (ue_sources.Get(i)->GetInitialEnergy()
				- ue_sources.Get(i)->GetRemainingEnergy());

	}
	battery_average = battery_average / ue_sources.GetN();
	std::cout << "the average battery consumption is " << battery_average
			<< std::endl;
	AsciiTraceHelper ascii1;
	Ptr<OutputStreamWrapper> stream2 = ascii1.CreateFileStream("ltesimu.tr",
			std::ios::app);

	*stream2->GetStream() << (double) ue_sources.GetN() << "\t" << (double) nLR << "\t" << (double) battery_average
			<< "\t" << (double) ratio << "\t" <<  (double) ratiohops << "\t" << (double) (sum / count)
			<< "\t" << (double) (transmissiondelay / numberdelevery )<< "\t"
			<< (double) connectivity_percent / (double) nbcycles << "\t"
			<< (double) coverage_percent * 100 / (double) nbcycles<< "\t"
			<<(double) first_noeud_to_die <<std::endl;

	std::cout << " number of packets sent is " << (double) myrequests.size()
			<< " and they are : ";
	std::map<int, double>::iterator it;
	for (double x : mysentrequests)
		std::cout << " , " << x;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;

	std::cout << " number of packets received is " << (double) myreceives.size()
			<< " and they are : ";
	std::map<int, double>::iterator it2;
	for (it2 = myreceives.begin(); it2 != myreceives.end(); it2++)
		std::cout << " , " << it2->first;
	std::cout << std::endl;

	std::cout << " average percentage of connectivity is  "
			<< (double) connectivity_percent / (double) nbcycles << std::endl;
	std::cout << " average percentage of coverage is  "
			<< (double) coverage_percent * 100 / (double) nbcycles << std::endl;
	std::cout << " first noeud died at : " << first_noeud_to_die << std::endl;

	Simulator::Destroy();

	//create nodes

}
