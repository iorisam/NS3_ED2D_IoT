/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 The Boeing Company
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//
// This script configures two nodes on an 802.11b physical layer, with
// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000
// (application) bytes to the other node.  The physical layer is configured
// to receive at a fixed RSS (regardless of the distance and transmit
// power); therefore, changing position of the nodes has no effect.
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc --help"
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when rss drops below -97 dBm.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --rss=-97 --numPackets=20"
// ./waf --run "wifi-simple-adhoc --rss=-98 --numPackets=20"
// ./waf --run "wifi-simple-adhoc --rss=-99 --numPackets=20"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the documentation.
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
//
// ./waf --run "wifi-simple-adhoc --verbose=1"
//
// When you are done, you will notice two pcap trace files in your directory.
// If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-0-0.pcap -nn -tt
//

#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/v4ping-helper.h"
#include "ns3/v4ping.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TeamPhone");

int size (20);
 //int step (50);
 int totalTime (100);
 bool pcap (true);
 bool printRoutes (true);

 std::string phyMode ("DsssRate1Mbps");
  double rss = -80;  // -dBm
  bool verbose = true;

  double Prss = -80;
 		  double offset = 81;

 NodeContainer nodes;
 NetDeviceContainer devices;
 Ipv4InterfaceContainer interfaces;


void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
    {
      NS_LOG_UNCOND ("Received one packet!");
    }
}

static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
                             uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0)
    {
      socket->Send (Create<Packet> (pktSize));
      Simulator::Schedule (pktInterval, &GenerateTraffic,
                           socket, pktSize,pktCount-1, pktInterval);
    }
  else
    {
      socket->Close ();
    }
}

void
 CreateNodes ()
{
  std::cout << "Creating " << (unsigned)size << " nodes \n";
  nodes.Create (size);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      Names::Add (os.str (), nodes.Get (i));
    }
// Create static grid
//  MobilityHelper mobility;
//  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
//                                 "MinX", DoubleValue (0.0),
//                                 "MinY", DoubleValue (0.0),
//                                 "DeltaX", DoubleValue (step),
//                                 "DeltaY", DoubleValue (0),
//                                 "GridWidth", UintegerValue (size),
//                                 "LayoutType", StringValue ("RowFirst"));
//  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//  mobility.Install (nodes);
}


bool Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("printRoutes", "Print routing table dumps.", printRoutes);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
 // cmd.AddValue ("step", "Grid step, m", step);
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("rss", "received signal strength", rss);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);

  cmd.Parse (argc, argv);

  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    // turn off RTS/CTS for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
    // Fix non-unicast data rate to be the same as that of unicast
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                        StringValue (phyMode));
  return true;
}


void CreateDevices ()
{
	  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
	  wifiMac.SetType ("ns3::AdhocWifiMac");
	  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
	  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
	  //wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	  wifiChannel.AddPropagationLoss ("ns3::Cost231PropagationLossModel");
	  wifiPhy.SetChannel (wifiChannel.Create ());
	  WifiHelper wifi = WifiHelper::Default ();
	  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
	  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
      AsciiTraceHelper eventTraces;
      wifiPhy.EnableAsciiAll(eventTraces.CreateFileStream("MyTeamPhone.tr"));

    }
}

void setPosition(){
	MobilityHelper staticnodes2;
		  staticnodes2.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
		                                 "X", StringValue ("100.0"),
		                                 "Y", StringValue ("100.0"),
		                                 "Rho", StringValue ("ns3::UniformRandomVariable[Min=50|Max=200]"));
		  staticnodes2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
		  staticnodes2.Install (nodes);
}

void
 InstallInternetStack ()
{
  AodvHelper aodv;
  // you can configure AODV attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv); // has effect on the next Install ()
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);

  if (printRoutes)
    {
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodv.routes", std::ios::out);
      aodv.PrintRoutingTableAllAt (Seconds (8), routingStream);

    }
}

void
 InstallApplications ()
{
//
//  for (int i= 0;i<nodes.GetN();i++){
//
//		V4PingHelper ping (interfaces.GetAddress (i+1));
//		  ping.SetAttribute ("Verbose", BooleanValue (true));
//	  ApplicationContainer p = ping.Install (nodes.Get(i));
//	  p.Start (Seconds (i));
//	   p.Stop (Seconds (totalTime) - Seconds (0.001));
//  }



  for (int i= 0;i<nodes.GetN() - 2;i++){
	  Ptr<V4Ping> app = CreateObject<V4Ping> ();
	     app->SetAttribute ("Remote", Ipv4AddressValue (interfaces.GetAddress (i+1)));
	     app->SetAttribute ("Verbose", BooleanValue (true));
	     nodes.Get(i)->AddApplication (app);
	     app->SetStartTime (Seconds (0));
	     app->SetStopTime (Seconds (totalTime) - Seconds (0.001));
   }






 }


int main (int argc, char *argv[])
{
	  CreateNodes ();
	  CreateDevices ();
	  InstallInternetStack ();
	  InstallApplications ();
	  setPosition();

	  std::cout << "Starting simulation for " << totalTime << " s ...\n";

	  Simulator::Stop (Seconds (totalTime));
	  Simulator::Run ();
	  Simulator::Destroy ();
  return 0;
}

