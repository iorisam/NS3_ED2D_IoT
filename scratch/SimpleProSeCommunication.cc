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

NS_LOG_COMPONENT_DEFINE("Simple-ProSe-simulation");

NodeContainer all;

uint32_t mcs = 10;
uint32_t rbSize = 8;
uint32_t ktrp = 1;
uint32_t pscchLength = 8;
std::string period = "sf80";
double dist = 10;

bool verbose = false;

int nNodes = 3;


double ueTxPower = 23.0;

uint32_t port = 9857;
double TotalTime = 100.0;

double txp = 23.0;

Ipv4Address serverAddress;

int main(int argc, char *argv[]) {
	LogComponentEnable ("UdpClient", LOG_LEVEL_ALL);
	LogComponentEnable ("UdpServer", LOG_LEVEL_ALL);
	LogComponentEnable ("NistPacketSink", LOG_LEVEL_ALL);
	//LogComponentEnable ("NistLteSpectrumPhy", LOG_LEVEL_ALL);
	//LogComponentEnable ("NistEpcUeNas", LOG_LEVEL_ALL);
	//LogComponentEnable ("NistLteUeNetDevice", LOG_LEVEL_ALL);
	//LogComponentEnable ("NistLteUeRrc", LOG_LEVEL_ALL);
	//LogComponentEnable ("NistLteUePhy", LOG_LEVEL_ALL);




	CommandLine cmd;
	cmd.AddValue("period", "Sidelink period", period);
	cmd.AddValue("pscchLength", "Length of PSCCH.", pscchLength);
	cmd.AddValue("ktrp", "Repetition.", ktrp);
	cmd.AddValue("mcs", "MCS.", mcs);
	cmd.AddValue("rbSize", "Allocation size.", rbSize);
	cmd.AddValue("verbose", "Print time progress.", verbose);
	cmd.AddValue("nUE", "number of UEs", nNodes);

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
		all.Create(nNodes);

		MobilityHelper staticnodes2;
		staticnodes2.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
				"X",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"),
				"Y",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"));
		staticnodes2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		staticnodes2.Install(all.Get(0));
		staticnodes2.Install(all.Get(1));

		MobilityHelper staticnodes3;

		staticnodes3.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
						"X",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"),
						"Y",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"));
		staticnodes3.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		staticnodes3.Install(all.Get(2));


		NS_LOG_INFO("Installing UE network devices...");
		NetDeviceContainer ueDevs;
		NetDeviceContainer ueRespondersDevs = lteHelper->InstallUeDevice(all);
		ueDevs.Add(ueRespondersDevs);

		InternetStackHelper internet;
		internet.Install(all);

		NS_LOG_INFO("Allocating IP addresses and setting up network route...");
		Ipv4InterfaceContainer ueIpIface;



		Ipv4StaticRoutingHelper ipv4RoutingHelper;
		ueIpIface = epcHelper->AssignUeIpv4Address(ueDevs);


		serverAddress =  Ipv4Address (ueIpIface.GetAddress (0));

		      for (uint32_t u = 0; u < nNodes; ++u)
		        {
		          Ptr<Node> ueNode = all.Get (u);
		          // Set the default gateway for the UE
		          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
		          std::cout<<"node ip address is "<<ueNode->GetObject<Ipv4> ();
		          ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
		        }

		NS_LOG_INFO("Attaching UE's to LTE network...");
		lteHelper->Attach(ueDevs);

		Ipv4InterfaceAddress iface = Ipv4InterfaceAddress(
				ueIpIface.GetAddress(0, 0), "255.0.0.0");

		Ipv4Address groupAddress("225.0.0.0");

		NS_LOG_INFO("Creating sidelink configuration...");



		uint32_t groupL2Address = 0xFF;
		Ptr<NistSlTft> tft = Create<NistSlTft>(NistSlTft::BIDIRECTIONAL,serverAddress, groupL2Address);
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


		UdpServerHelper server (port);
		ApplicationContainer apps = server.Install (all.Get (0));
		apps.Start (Seconds (1.0));
		apps.Stop (Seconds (100.0));

		/*PacketSinkHelper clientPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (serverAddress, port));
		ApplicationContainer apps = clientPacketSinkHelper.Install(all.Get (0));
		apps.Start (Seconds (1.0));
		apps.Stop (Seconds (10.0));*/

		 uint32_t MaxPacketSize = 20;
		  Time interPacketInterval = Seconds (2.5);
		  uint32_t maxPacketCount = 20;
		  UdpClientHelper client (serverAddress, port);
		  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
		  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
		  client.SetAttribute ("PacketSize", UintegerValue (MaxPacketSize));

			NodeContainer clients;
			clients.Add(all.Get(1));
			clients.Add(all.Get(2));
		  ApplicationContainer apps2 = client.Install (clients);


		  apps2.Start (Seconds (2.0));
		  apps2.Stop (Seconds (100.0));




		//
		// Now, do the actual simulation.
		//
		  NS_LOG_INFO ("Run Simulation.");
		  Simulator::Stop(Seconds(120));
		  Simulator::Run ();

		  Simulator::Destroy ();
		  NS_LOG_INFO ("Done.");
		}








