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

NS_LOG_COMPONENT_DEFINE("Need-ProSe-Simulation");

NodeContainer all;

NodeContainer UEs;
NodeContainer IoT_UEs;
NodeContainer IoTDevices;



uint32_t mcs = 10;
uint32_t rbSize = 8;
uint32_t ktrp = 1;
uint32_t pscchLength = 16;
std::string period = "sf80";
double dist = 10;

bool verbose = false;

int nUEs = 50;
int nSensors = 50;


double ueTxPower = 2.0;

uint32_t port = 9;
double TotalTime = 100.0;
double nodeSpeed = 1; //in m/s
int number_of_types_of_services = 4;
int nodePause = 0; //in s
std::string m_protocolName = "protocol";
double txp = 2.0;

uint32_t Rid1;
uint32_t Rid2;


double first_noeud_to_die = -1;

std::list<int> serviceTypes;

ApplicationContainer NeedApps;

uint32_t pucchSize = 6;          // PUCCH size in RBs
uint32_t pscch_rbs = 22;         // PSCCH pool size in RBs. Note, the PSCCH occupied bandwidth will be at least twice this amount.
std::string pscch_bitmap_hexstring = "0xFF00000000"; // PSCCH time bitmap [40 bits]
uint32_t sl_period = 40;         // Length of sidelink period in milliseconds
bool CtrlErrorModelEnabled = true; // Enable error model in the PSCCH
bool CtrlDropOnCollisionEnabled = false; // Drop PSCCH messages on conflicting scheduled resources


void DiscoveryAnnouncementPhyTrace(uint16_t receiver,
		std::string imsi, uint16_t cellId, uint16_t rnti,
		std::string proSeAppCode) {
	//std::cout <<" at time "<< Simulator::Now ().GetSeconds () << " discovery message from \t " << receiver << "\t cell id is " << cellId << " sender is \t" << rnti << "\t" << proSeAppCode << std::endl;
	Ptr<Need_ProSe_App> app = DynamicCast<Need_ProSe_App>(NeedApps.Get(receiver));
	app->DiscoveryMonitoringCallback(rnti-1,proSeAppCode);
	NS_LOG_INFO("ppppppp at time "<< Simulator::Now ().GetSeconds () << " discovery message from \t " <<
			receiver << "\t cell id is " << cellId << " sender is \t" << rnti-1 << "\t" << proSeAppCode );
}


void SlStartDiscovery(Ptr<NistLteHelper> helper, Ptr<NetDevice> ue,
		std::list<std::string> apps, bool rxtx) {
	helper->StartDiscovery(ue, apps, rxtx);
}

void SlStopDiscovery(Ptr<NistLteHelper> helper, Ptr<NetDevice> ue,
		std::list<std::string> apps, bool rxtx) {
	helper->StopDiscovery(ue, apps, rxtx);

}

void DiscoveryMonitoringTrace(uint16_t receiver, uint64_t imsi,
		uint16_t cellId, uint16_t rnti, std::string proSeAppCode) {

	//std::cout <<" at time "<< Simulator::Now ().GetSeconds () << " discovery message from \t " << receiver << "\t cell id is " << cellId << " sender is \t" << rnti << "\t" << proSeAppCode << std::endl;
		Ptr<Need_ProSe_App> app = DynamicCast<Need_ProSe_App>(NeedApps.Get(receiver));
		app->DiscoveryMonitoringCallback(rnti-1,proSeAppCode);
	//std::cout <<"ggggggg at time "<< Simulator::Now ().GetSeconds () << " discovery message from \t " << receiver << "\t cell id is " << cellId << " sender is \t" << rnti << "\t" << proSeAppCode << std::endl;

}

void DiscoveryAnnouncementMacTrace(uint16_t receiver,
		std::string imsi, uint16_t rnti, std::string proSeAppCode) {
	std::cout <<"mmmmmmm at time "<< Simulator::Now ().GetSeconds () << " discovery message from \t " << receiver << " sender is \t" << rnti << "\t" << proSeAppCode << std::endl;
}

void DepletionHandler(double node) {
	if (first_noeud_to_die < 0 && node > 1) {
		first_noeud_to_die = Simulator::Now().GetSeconds();
	}
	std::cout << " node " << node << " has no energy left at : "
			<< Simulator::Now().GetSeconds() << std::endl;

}


int main(int argc, char *argv[]) {
//		 LogComponentEnable ("NistLteSpectrumPhy", LOG_LEVEL_INFO);
	 LogComponentEnable ("need-prose-app", LOG_LEVEL_ALL);
	 LogComponentEnable ("NeedLRClientApp", LOG_LEVEL_ALL);
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

	LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
	LogComponentEnable ("PacketSink", LOG_LEVEL_ALL);
	//LogComponentEnable ("NistLteSpectrumPhy", LOG_LEVEL_ALL);


	CommandLine cmd;
	cmd.AddValue("period", "Sidelink period", period);
	cmd.AddValue("pscchLength", "Length of PSCCH.", pscchLength);
	cmd.AddValue("ktrp", "Repetition.", ktrp);
	cmd.AddValue("mcs", "MCS.", mcs);
	cmd.AddValue("rbSize", "Allocation size.", rbSize);
	cmd.AddValue("verbose", "Print time progress.", verbose);
	cmd.AddValue("nUE", "number of UEs", nUEs);
	cmd.AddValue("nodeSpeed", "number of UEs", nodeSpeed);
	cmd.AddValue("nbS", "number of UEs", number_of_types_of_services);

	cmd.Parse(argc, argv);

	nSensors = nUEs;

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
	time_t seconds;

	  seconds = time (NULL);
	RngSeedManager::SetSeed (seconds);

	/*NS_LOG_INFO ("Creating helpers...");
		  Ptr<NistPointToPointEpcHelper>  epcHelper = CreateObject<NistPointToPointEpcHelper> ();
		  Ptr<Node> pgw = epcHelper->GetPgwNode ();
		  Ptr<NistLteHelper> lteHelper = CreateObject<NistLteHelper> ();
		  lteHelper->SetAttribute("UseSidelink", BooleanValue(true));

		  lteHelper->SetEpcHelper (epcHelper);
		  lteHelper->DisableNewEnbPhy ();

		  Ptr<NistLteProseHelper> proseHelper = CreateObject<NistLteProseHelper> ();
		    proseHelper->SetLteHelper (lteHelper);



		    lteHelper->SetEnbAntennaModelType ("ns3::NistParabolic3dAntennaModel");
		    lteHelper->SetEnbAntennaModelAttribute ("MechanicalTilt", DoubleValue (20));
		    //lteHelper->SetPathlossModelType ("ns3::Cost231PropagationLossModel");
		    lteHelper->SetPathlossModelType ("ns3::FriisPropagationLossModel");
		    lteHelper->SetAttribute ("UseSidelink", BooleanValue (true));

		    lteHelper->SetAttribute ("UseDiscovery", BooleanValue (true));
		    lteHelper->SetSchedulerType ("ns3::NistRrSlFfMacScheduler");
		    //lteHelper->Initialize();*/


	NS_LOG_INFO("Creating helpers...");
	Ptr<NistPointToPointEpcHelper> epcHelper = CreateObject<
			NistPointToPointEpcHelper>();
	Ptr<Node> pgw = epcHelper->GetPgwNode();
	Ptr<NistLteHelper> lteHelper = CreateObject<NistLteHelper>();
	lteHelper->SetAttribute("UseSidelink", BooleanValue(true));
	lteHelper->SetAttribute ("UseDiscovery", BooleanValue (true));
	lteHelper->SetEpcHelper(epcHelper);
	lteHelper->DisableNewEnbPhy();
	//lteHelper->SetPathlossModelType ("ns3::FriisPropagationLossModel");
	lteHelper->SetPathlossModelType ("ns3::Cost231PropagationLossModel");
	lteHelper->SetSchedulerType ("ns3::NistRrSlFfMacScheduler");
	lteHelper->Initialize();

	Ptr<NistLteProseHelper> proseHelper = CreateObject<NistLteProseHelper>();
	proseHelper->SetLteHelper(lteHelper);

	NS_LOG_INFO("Deploying UE's...");

	//create nodes and node containers
	UEs.Create(nUEs);
	IoTDevices.Create(nSensors);

	all.Add(UEs);
	all.Add(IoTDevices);

	for (int i = 0; i < nUEs; i++)
			IoT_UEs.Add(UEs.Get(i));





	// set the phyisical layer and channel model with propagation delay and pathloss , data rate n controlMode, and transmission power,
	// and ap or adhoc mode,

	// set the node position and mobility model.

	MobilityHelper staticnodes2;
	staticnodes2.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
			"X",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"),
			"Y",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=400.0]"));
	staticnodes2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	staticnodes2.Install(all);

	std::cout<<" here is the position of first node "<< all.Get(0)->GetObject<MobilityModel>()->GetPosition()<<std::endl;


	// install internet stack

	NS_LOG_INFO("Installing UE network devices...");
	NetDeviceContainer ueDevs;
	NetDeviceContainer ueRespondersDevs = lteHelper->InstallUeDevice(
			UEs);
	ueDevs.Add(ueRespondersDevs);

	InternetStackHelper internet;
	internet.Install(UEs);
	internet.Install(IoTDevices);

	NS_LOG_INFO("Allocating IP addresses and setting up network route...");
	Ipv4InterfaceContainer ueIpIface;

	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	ueIpIface = epcHelper->AssignUeIpv4Address(ueDevs);
	std::cout << "multi ip address is " << ueDevs.GetN();

	NS_LOG_INFO("Attaching UE's to LTE network...");
	lteHelper->Attach(ueDevs);

	Ipv4InterfaceAddress iface = Ipv4InterfaceAddress(
			ueIpIface.GetAddress(0, 0), "255.0.0.0");

	Ptr<Ipv4> ipv4 = all.Get(0)->GetObject<Ipv4> ();
				 Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
				 Ipv4Address sinkIp = iaddr.GetLocal();
				 std::cout<<"THE IP ADDRESS OF THE SINK IS "<<sinkIp<<std::endl;



	LrWpanHelper lrWpanHelper;
	// Add and install the LrWpanNetDevice for each node
	NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(UEs);
	NetDeviceContainer lrwpanDevices2 = lrWpanHelper.Install(IoTDevices);
	NetDeviceContainer AllLrDevices;
	AllLrDevices.Add(lrwpanDevices);
	AllLrDevices.Add(lrwpanDevices2);

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

		 LrWpanSpectrumValueHelper svh;
		 Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (-1, 11);
		 dev->GetPhy ()->SetTxPowerSpectralDensity (psd);

		 Ptr<MobilityModel> themodel = all.Get(i)->GetObject<MobilityModel> ();
		 dev->GetPhy()->SetMobility(themodel);

		}


	for (int i = 0; i < nSensors; i++) {
		Ptr<LrWpanNetDevice> dev = DynamicCast<LrWpanNetDevice> (AllLrDevices.Get(i+nUEs));
		dev->SetChannel(channel);

		LrWpanSpectrumValueHelper svh;
		Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (-1, 11);
		dev->GetPhy ()->SetTxPowerSpectralDensity (psd);

		Ptr<MobilityModel> themodel = IoTDevices.Get(i)->GetObject<MobilityModel> ();
		dev->GetPhy()->SetMobility(themodel);
		}


	// setting up Energy sources and Energy models for Sensors
		LrWpanEnergySourceHelper lrSourceHelper;
		// configure energy source
		lrSourceHelper.Set("LrWpanEnergySourceInitialEnergyJ", DoubleValue(1000));
		// install source
		EnergySourceContainer lr_sources = lrSourceHelper.Install(IoTDevices);
		/* device energy model */
		EnergySourceContainer lr_sources2 = lrSourceHelper.Install(IoT_UEs);
		ZigbeeEnergyModelHelper zigbeeEnergyHelper;
		// configure radio energy model

		// install device model
		DeviceEnergyModelContainer deviceModels = zigbeeEnergyHelper.Install(
				lrwpanDevices2, lr_sources);
		/***************************************************************************/


	// setting up some other lte configurations

	 NS_LOG_INFO ("Creating sidelink configuration...");
	 uint32_t groupL2Address = 0xFF;
		 	Ptr<NistSlTft> tft = Create<NistSlTft>(NistSlTft::BIDIRECTIONAL,
		 			sinkIp, groupL2Address);
		 	proseHelper->ActivateSidelinkBearer(Seconds(1.0), ueDevs, tft);
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
		 	      preconfiguration.preconfigDisc.pools[0].txParameters.txParametersGeneral.alpha = NistLteRrcSap::SlTxParameters::al1;
		 	      preconfiguration.preconfigDisc.pools[0].txParameters.txParametersGeneral.p0 = -40;
		 	      preconfiguration.preconfigDisc.pools[0].txParameters.txProbability.probability = NistLteRrcSap::TxProbability::p75;
		 	      /*preconfiguration.preconfigDisc.pools[1].cpLen.cplen = NistLteRrcSap::SlCpLen::NORMAL;
				  preconfiguration.preconfigDisc.pools[1].discPeriod.period = NistLteRrcSap::SlPeriodDisc::rf32;
				  preconfiguration.preconfigDisc.pools[1].numRetx = 0;
				  preconfiguration.preconfigDisc.pools[1].numRepetition = 1;
				  preconfiguration.preconfigDisc.pools[1].tfResourceConfig.prbNum = 10;
				  preconfiguration.preconfigDisc.pools[1].tfResourceConfig.prbStart = 10;
				  preconfiguration.preconfigDisc.pools[1].tfResourceConfig.prbEnd = 49;
				  preconfiguration.preconfigDisc.pools[1].tfResourceConfig.offsetIndicator.offset = 0;
				  preconfiguration.preconfigDisc.pools[1].tfResourceConfig.subframeBitmap.bitmap = std::bitset<40> (0x11111);
				  preconfiguration.preconfigDisc.pools[1].txParameters.txParametersGeneral.alpha = NistLteRrcSap::SlTxParameters::al1;
				  preconfiguration.preconfigDisc.pools[1].txParameters.txParametersGeneral.p0 = -40;
				  preconfiguration.preconfigDisc.pools[1].txParameters.txProbability.probability = NistLteRrcSap::TxProbability::p75;
		 	      //preconfiguration.preconfigDisc.pools[0].txParameters.txProbability = NistLteRrcSap::TxProbabilityFromInt (50);*/

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
	//lteHelper->StopDiscovery(ueRespondersDevs, { }, true);


	// configure energy source
	/*EnergySourceContainer ue_sources;

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

	 */

	EnergySourceContainer ue_sources;
		/* device energy model */

	std::vector<EnergyModuleLte *> sources;
	for (int i = 0; i < nUEs; i++) {
		Ptr<LiIonEnergySource> ue_source =
				lteHelper->EnergySourcesContainer.find(all.Get(i)->GetId())->second->LIES.GetObject<
						LiIonEnergySource>();
		ue_source->SetNode(all.Get(i));
		ue_sources.Add(ue_source);
		sources.push_back(lteHelper->EnergySourcesContainer.find(all.Get(i)->GetId())->second);
		ue_source->LteEnergyDepletionCallback = MakeCallback(&DepletionHandler);

	}

	    NeedProSeAppHelper needAppsHelper(iface.GetBroadcast(), 9);
		needAppsHelper.SetAttribute ("MaxPackets", UintegerValue (5000000));


		needAppsHelper.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
		needAppsHelper.SetAttribute ("PacketSize", UintegerValue (1024));
		NeedApps = needAppsHelper.Install(UEs);

		AsciiTraceHelper ascii;
		Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream("discovery_out_monitoring.tr");
		std::ostringstream oss;
			oss.str("");
		for (int i = 0; i < nUEs; i++){
			Ptr<Need_ProSe_App> need_prose_app = DynamicCast<Need_ProSe_App>(NeedApps.Get(i));
			need_prose_app->lte_helper = lteHelper;
			need_prose_app->energy_sources = sources;

			Ptr<NistLteUeRrc> ueRrc = DynamicCast<NistLteUeRrc>(ueDevs.Get(i)->GetObject<NistLteUeNetDevice>()->GetRrc());
					ueRrc->TraceConnectWithoutContext("DiscoveryMonitoring",MakeBoundCallback(&DiscoveryMonitoringTrace, i));
					oss << ueDevs.Get(i)->GetObject<NistLteUeNetDevice>()->GetImsi();
					oss.str("");
		}
		NeedApps.Start(Seconds (1.0));
		NeedApps.Stop (Seconds (TotalTime));
		std::list<std::string> announceApps;
		std::list<std::string> monitorApps;
		for (uint32_t i = 0; i < nUEs; ++i) {
				Simulator::Schedule(Seconds(1.0), &SlStartDiscovery, lteHelper,
						ueRespondersDevs.Get(i),
						announceApps, true); // true for announce
				Simulator::Schedule(Seconds(1.0), &SlStartDiscovery, lteHelper,
						ueRespondersDevs.Get(i),
						monitorApps, false); // false for monitor
			}


			 UdpClientHelper trafficAppsHelper (sinkIp,8050);

			 trafficAppsHelper.SetAttribute("MaxPackets", UintegerValue (5000000));
			 trafficAppsHelper.SetAttribute("PacketSize", UintegerValue (12));
			 NodeContainer clients;
			 for(int i = 1; i<UEs.GetN();i++){
				 clients.Add(UEs.Get(i));

			 }


			 ApplicationContainer clientsapps = trafficAppsHelper.Install(clients);
			 for(int i = 0; i<clientsapps.GetN();i++){
				 Ptr<UdpClient> app = DynamicCast<UdpClient>(clientsapps.Get(i));
				 Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable> ();
				 double randomint = generator->GetInteger(0,1000) + 1;
				 double k = 10 + (randomint/1000);
				 app->SetAttribute("Interval",TimeValue (Seconds (k)));
			 }




			 clientsapps.Start(Seconds (10.0));
			 clientsapps.Stop (Seconds (TotalTime));
			 InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8050);

			 PacketSinkHelper sinkAppHelper ("ns3::UdpSocketFactory",local);
			 ApplicationContainer sinkApp = sinkAppHelper.Install(all.Get(0));
			 sinkApp.Start(Seconds (1.0));
			 sinkApp.Stop (Seconds (TotalTime));


			 NeedLRSAppsHelper needLRAppsHelper;
			 ApplicationContainer lrClients =  needLRAppsHelper.Install(IoTDevices);

			 for (int i = 0; i < IoTDevices.GetN(); i++) {
			 			Ptr<NeedLRClient> lrclient = DynamicCast<NeedLRClient>(
			 					lrClients.Get(i));
			 			lrclient->SetDevice(lr_devices.Get(i + nUEs));
			 			 Ptr<UniformRandomVariable> generator = CreateObject<UniformRandomVariable> ();
			 			 int randomint = generator->GetInteger(0,1000) + 1;
			 			double k = 10 + (randomint/1000);
			 			serviceTypes.push_back(randomint) ;
			 			lrclient->SetAttribute("Interval",TimeValue (Seconds (6*k)));
			 		}
			 lrClients.Start(Seconds (11.0));
			 lrClients.Stop (Seconds (TotalTime));

	  Simulator::Stop(Seconds(TotalTime));
	  Simulator::Run();
	  Simulator::Destroy();

	  Ptr<EnergySource> sinksource = ue_sources.Get(0);
	  double delivered_packets = 0;
	  double packets_sent = 0;
	  Ptr<PacketSink> s_app = DynamicCast<PacketSink>(sinkApp.Get(0));
	  delivered_packets = s_app->GetTotalRx();
	  double battery_average= 0.0;
	  double most_battery_average= 0.0;

	  for(int i = 0; i<clientsapps.GetN();i++){
		  Ptr<UdpClient> c_app = DynamicCast<UdpClient>(clientsapps.Get(i));
		  packets_sent += c_app->GetNSentPackets();
		  double residual_energy = sources.at(i)->LIES.m_remaining;

		  std::cout<<" remaining energy for node "<< clients.Get(i)->GetId() << " is : "<< residual_energy<<std::endl;
		  battery_average += residual_energy;
		  most_battery_average += residual_energy * residual_energy;
	  }

	  for (int i = 0; i < IoTDevices.GetN(); i++) {
			Ptr<NeedLRClient> lrclient = DynamicCast<NeedLRClient>(lrClients.Get(i));
	  		  packets_sent += lrclient->GetNSentPackets();
	  	  }
	  battery_average = battery_average/UEs.GetN();
	  most_battery_average = most_battery_average/UEs.GetN();


	  double variance = most_battery_average - (battery_average * battery_average) ;
	  std::cout<<" energy variance is "<< variance<<std::endl;


	  double stdDeviation = sqrt(variance);
	  AsciiTraceHelper ascii1;
	  Ptr<OutputStreamWrapper> stream2 = ascii1.CreateFileStream ( "need_pro_simu.tr",std::ios::app);
	 // *stream2->GetStream () << " Number of nodes \t AverageEnergy \t Variance \t stdDeviation \t Delivery \t Efficiency "<<std::endl;


	  Ptr<OutputStreamWrapper> stream3 = ascii1.CreateFileStream ( "need_pro_simu_batries.tr",std::ios::app);

	  for(int i = 0; i<clientsapps.GetN();i++){
		  double residual_energy = sources.at(i)->LIES.m_remaining;
		  *stream3->GetStream () <<residual_energy<<";";
	  }
	  *stream3->GetStream ()<<std::endl;

	  double ratio = double(packets_sent)/delivered_packets;
	  std::cout<<" delivered packets are "<<delivered_packets<< " but we sent "<<packets_sent<<std::endl;
	  *stream2->GetStream () << all.GetN()<<" \t "<< battery_average << " \t " <<variance<<" \t "<<stdDeviation<<" \t "<<ratio<<" \t "<<ratio/(60-battery_average)<<" \t " << first_noeud_to_die << " \t "<<std::endl;
	//create nodes

}
