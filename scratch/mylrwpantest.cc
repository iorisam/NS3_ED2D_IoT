/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 The Boeing Company
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
 * Author: Tom Henderson <thomas.r.henderson@boeing.com>
 */

// This program produces a gnuplot file that plots the packet success rate
// as a function of distance for the 802.15.4 models, assuming a default
// LogDistance propagation loss model, the 2.4 GHz OQPSK error model, a
// default transmit power of 0 dBm, and a default packet size of 20 bytes of
// 802.15.4 payload.
#include <ns3/test.h>
#include <ns3/log.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/lr-wpan-error-model.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/lr-wpan-net-device.h>
#include <ns3/spectrum-value.h>
#include <ns3/lr-wpan-spectrum-value-helper.h>
#include <ns3/lr-wpan-mac.h>
#include <ns3/node.h>
#include <ns3/net-device.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/mac16-address.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/uinteger.h>
#include <ns3/nstime.h>
#include <ns3/abort.h>
#include <ns3/command-line.h>
#include <ns3/gnuplot.h>
#include <string>
#include <vector>

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
#include "ns3/netanim-module.h"
#include "ns3/internet-apps-module.h"


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






using namespace ns3;
using namespace std;

static uint32_t g_received = 0;

NS_LOG_COMPONENT_DEFINE ("LrWpanErrorDistancePlot");

static void
LrWpanErrorDistanceCallback (McpsDataIndicationParams params, Ptr<Packet> p)
{
  g_received++;
}

int main (int argc, char *argv[])
{

  int minDistance = 30;
  int maxDistance = 200;  // meters
  int increment = 1;
  int maxPackets = 100;
  int packetSize = 20;
  double txPower = -1;
  uint32_t channelNumber = 11;
  NodeContainer container;

  CommandLine cmd;

  cmd.AddValue ("txPower", "transmit power (dBm)", txPower);
  cmd.AddValue ("channelNumber", "channel number", channelNumber);

  cmd.Parse (argc, argv);

  std::cout << " tx power = " << txPower << std::endl;
  container.Create(40);

  LrWpanHelper lrWpanHelper;
	  NetDeviceContainer lrwpanDevices = lrWpanHelper.Install(container);


	  lrWpanHelper.AssociateToPan (lrwpanDevices, 0);


//
//  Ptr<Node> n0 = CreateObject <Node> ();
//  Ptr<Node> n1 = CreateObject <Node> ();
//
//  Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice> ();
//  Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice> ();
//  dev0->SetAddress (Mac16Address ("00:01"));
//  dev1->SetAddress (Mac16Address ("00:02"));
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> model = CreateObject<LogDistancePropagationLossModel> ();

  channel->AddPropagationLossModel (model);

  InternetStackHelper internet;

  internet.Install(container);
  SixLowPanHelper sixlowpan;
  NetDeviceContainer lr_devices = sixlowpan.Install (lrwpanDevices);




  	      Ipv6AddressHelper ipv6;
  	      ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
  	      Ipv6InterfaceContainer lr_interface = ipv6.Assign (lr_devices);



  	       MobilityHelper staticnodes;
  	       staticnodes.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
 										 "X",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=250.0]"),
 										 "Y",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=150.0]"));
 		  staticnodes.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
 		  staticnodes.Install(container);

  for (int i=0; i<container.GetN(); i++){
	  Ptr<LrWpanNetDevice> dev = DynamicCast<LrWpanNetDevice> (lrwpanDevices.Get(i));
	  dev->SetChannel(channel);


	  LrWpanSpectrumValueHelper svh;
	  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);
	  dev->GetPhy ()->SetTxPowerSpectralDensity (psd);

	  Ptr<MobilityModel> themodel = container.Get(i)->GetObject<MobilityModel> ();
	  dev->GetPhy()->SetMobility(themodel);


  }

//
//
//  dev0->SetChannel (channel);
//  dev1->SetChannel (channel);
//  n0->AddDevice (dev0);
//  n1->AddDevice (dev1);
//  //Ptr<ConstantPositionMobilityModel> mob0 = CreateObject<ConstantPositionMobilityModel> ();
//  //dev0->GetPhy ()->SetMobility (mob0);
//  //Ptr<ConstantPositionMobilityModel> mob1 = CreateObject<ConstantPositionMobilityModel> ();
//  //dev1->GetPhy ()->SetMobility (mob1);
//
//  LrWpanSpectrumValueHelper svh;
//  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);
//  dev0->GetPhy ()->SetTxPowerSpectralDensity (psd);
//
//  McpsDataIndicationCallback cb0;
//  cb0 = MakeCallback (&LrWpanErrorDistanceCallback);
//  dev1->GetMac ()->SetMcpsDataIndicationCallback (cb0);
//
//  McpsDataRequestParams params;
//  params.m_srcAddrMode = SHORT_ADDR;
//  params.m_dstAddrMode = SHORT_ADDR;
//  params.m_dstPanId = 0;
//  params.m_dstAddr = Mac16Address ("00:02");
//  params.m_msduHandle = 0;
//  params.m_txOptions = 0;
//
//  Ptr<Packet> p;
////  mob0->SetPosition (Vector (0,0,0));
////  mob1->SetPosition (Vector (minDistance,0,0));
//
//  InternetStackHelper internet;
//
//  internet.Install(n0);
//  internet.Install(n1);
//
//  SixLowPanHelper sixlowpan;
//  NodeContainer lrnodes;
//  lrnodes.Add(n1);
//  lrnodes.Add(n0);
//
//  NetDeviceContainer devices;
//  devices.Add(dev0);
//  devices.Add(dev1);
//
//
//
//
// 	      NetDeviceContainer lr_devices = sixlowpan.Install (devices);
//
//
//
//
// 	      Ipv6AddressHelper ipv6;
// 	      ipv6.SetBase (Ipv6Address ("2001:2::"), Ipv6Prefix (64));
// 	      Ipv6InterfaceContainer lr_interface = ipv6.Assign (lr_devices);
//
//
//
// 	       MobilityHelper staticnodes;
// 	       staticnodes.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
//										 "X",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=50.0]"),
//										 "Y",  StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=50.0]"));
//		  staticnodes.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//		  staticnodes.Install(lrnodes);
//
//
//
//		  Ptr<MobilityModel> themodel = n0->GetObject<MobilityModel> ();
//
//		  Ptr<MobilityModel> themodel2 = n1->GetObject<MobilityModel> ();
//
//		  dev0->GetPhy ()->SetMobility (themodel);
//
//		  dev1->GetPhy ()->SetMobility (themodel2);

		  AnimationInterface anim ("wireless-animation102.xml");




//      for (int i = 0; i < maxPackets; i++)
//        {
//    	  p = Create<Packet> (packetSize);
//			 Simulator::Schedule (Seconds (i),
//								  &LrWpanMac::McpsDataRequest,
//								  dev0->GetMac (), params, p);
//
//
//        }


		   uint32_t mypacketSize = 10;
		   uint32_t maxPacketCount = 5;
		   Time interPacketInterval = Seconds (1.);
		   Ping6Helper ping6;


		   ping6.SetRemote (lr_interface.GetAddress (1, 1));

		   ping6.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
		   ping6.SetAttribute ("Interval", TimeValue (interPacketInterval));
		   ping6.SetAttribute ("PacketSize", UintegerValue (packetSize));
		   ApplicationContainer apps = ping6.Install (container);

		   apps.Start (Seconds (1.0));
		   apps.Stop (Seconds (100.0));


      Simulator::Stop(Seconds(110));
      Simulator::Run ();

     // NS_LOG_DEBUG ("Received " << g_received << " packets for distance " << j);
     // psrdataset.Add (j, g_received / 1000.0);
     // g_received = 0;
     // j += increment;
     // mob1->SetPosition (Vector (j,0,0));

  Simulator::Destroy ();
  return 0;
}

