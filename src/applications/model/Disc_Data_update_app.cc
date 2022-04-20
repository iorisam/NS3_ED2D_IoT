/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "Disc_Data_update_app.h"
#include "seq-ts-header.h"
#include <cstdlib>
#include <cstdio>
#include <ns3/nist-lte-helper.h>
#include <ns3/BloomFilter.hpp>
#include "ns3/ipv4-address-generator.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DiscUpdate");

NS_OBJECT_ENSURE_REGISTERED (DiscApp);

TypeId
DiscApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DiscApp")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<DiscApp> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&DiscApp::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets", TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&DiscApp::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&DiscApp::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated. The minimum packet size is 12 bytes which is the size of the header carrying the sequence number and the time stamp.",
                   UintegerValue (1024),
                   MakeUintegerAccessor (&DiscApp::m_size),
                   MakeUintegerChecker<uint32_t> (12,1500))
  ;
  return tid;
}

DiscApp::DiscApp ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_sendEvent = EventId ();
}

DiscApp::~DiscApp ()
{
  NS_LOG_FUNCTION (this);
}

void
DiscApp::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
DiscApp::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  Init_bloom();
  m_sendEvent = Simulator::Schedule (Seconds (0.0), &DiscApp::Send, this);
}

void
DiscApp::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_sendEvent);
}

void
DiscApp::Send (void)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_sendEvent.IsExpired ());
  std::bitset<184>mynewname = generatenewname();
  m_sendEvent = Simulator::Schedule (m_interval, &DiscApp::Send, this);
  std::list<std::string> oldlist;
  oldlist.push_back(oldname.to_string());
  std::list<std::string> newlist;
  newlist.push_back(mynewname.to_string());


  m_lteHelper->StopDiscovery(GetNode()->GetDevice(0),oldlist,true);
  m_lteHelper->StartDiscovery(GetNode()->GetDevice(0),newlist,true);
  oldname = mynewname;


}

void
DiscApp::UpdateTable(uint64_t imsi, std::string discappcode,bloom_filter filter)
{
  NS_LOG_FUNCTION (this);
  std::string LvlBits = discappcode.substr(0,8);
  std::string DegreeBits = discappcode.substr(8,7);
  std::string statausbits = discappcode.substr(15,1);
  std::string FilterBits = discappcode.substr(16,184);
  double BatteryLvl = (double)(std::bitset<8>(LvlBits)).to_ulong();
  uint32_t CoverageDegree = (uint32_t)(std::bitset<8>(DegreeBits)).to_ulong();
  uint32_t status = 0;
  if (statausbits != "0")
	  statausbits =1;

//  std::cout<<"this is node "<<GetNode()->GetId()<<" received disc message from "<<imsi<<
//		  "and has "<<MyTable->size()<<std::endl;

  std::list<int> neighbors ;
  Ipv4Address zeroAddress ("0.0.0.0");
  InetSocketAddress local = InetSocketAddress (zeroAddress, 0);

  std::map<Address, neighbor>::iterator it;
  std::map<Address, neighbor>::iterator it2;

  bool exists = false;
  neighbors.push_back(imsi);
    for ( it = MyTable->begin(); it != MyTable->end(); it++ )
     {

    	if(it->second.node_id!= imsi && filter.contains(it->second.node_id))
    		neighbors.push_back(it->second.node_id);
    	else if(it->second.node_id== imsi){
    		it2 = it;
    		exists = true;
    	}
     }
    if((std::find(neighbors.begin(), neighbors.end(), GetNode()->GetId()) == neighbors.end()))
    neighbors.push_back(GetNode()->GetId());

    if(exists){
    	it2->second.battery = BatteryLvl;
    						it2->second.discovered_time = Simulator::Now ().GetSeconds ();
    						it2->second.neighbors = neighbors;
    	       		       	exists = true;
    	       		       	it2->second.status = status;
    }else{
    	 neighbor n (local,0,BatteryLvl,neighbors,Simulator::Now ().GetSeconds (),imsi);
    	 Ipv4Address address;
    	 std::string addr = "0.0.0."+imsi;
    	 std::string mask = "255.0.0.0";
    	 address =  ns3::Ipv4Address(imsi);
    	 n.status = status;
    	 MyTable->insert(std::pair<Address , neighbor>(address,n));
    }

   // std::cout<<" but now has "<<MyTable->size()<<" after adding a new eliment " <<exists<<std::endl;
}

void
DiscApp::SetTable( std::map<Address,neighbor> *table){
	NS_LOG_FUNCTION (this);
	MyTable = table;
}

void
DiscApp::UpdateState (void)
{
  NS_LOG_FUNCTION (this);

}

std::bitset<184>
DiscApp::generatenewname (void)
{

  NS_LOG_FUNCTION (this);
  std::bitset<184> newname;
  double energyprcent = (my_energy->LIES.m_remaining)/my_energy->LIES.GetInitialEnergy()*100;
  std::bitset<8> EnergyLvLbits = std::bitset<8>((int) energyprcent);
  std::bitset<7> Degreebits = std::bitset<7> (MyTable->size());
  std::bitset<1> statusbits;

  std::string x("temp");

  if(((*state).compare("temp")) == 0)
	  statusbits.set();





  std::map<Address, neighbor>::iterator it;
  //std::cout<<"node number "<<GetNode()->GetId()<<" has neighbors : ";
    for ( it = MyTable->begin(); it != MyTable->end(); it++ ){

    	m_filter.insert(it->second.node_id);
    	//std::cout<<it->second.node_id<<" , ";
       }
    //std::cout<< "degree of  conectivity is "<<Degreebits<< " and the status is " << statusbits<< std::endl;
    std::string bf_bits;
    for(int i=0;i<m_filter.bitvecotor().size();i++){
    	char a= m_filter.bitvecotor()[i];
    	std::string temp = std::bitset<8>(a).to_string();
    	bf_bits = temp+bf_bits;
    }
    std::string output = EnergyLvLbits.to_string()+Degreebits.to_string()+statusbits.to_string()+bf_bits;
    newname = std::bitset<184>(output);
    if(m_filter.bitvecotor().size()>0){
    std::string astring(m_filter.bitvecotor()[0], m_filter.bitvecotor().size());
   // std::cout<<" the filter has "<<m_filter.bitvecotor().size()<<std::endl;
    }
  return newname;
}


void
DiscApp::SetLteHelper (Ptr<NistLteHelper> h)
{
  NS_LOG_FUNCTION (this << h);
  m_lteHelper = h;
}

void DiscApp::NotifyDiscoveryMessage(uint64_t imsi, std::string discappcode,bloom_filter filter){

	UpdateTable(imsi,discappcode,filter);

}

void DiscApp::Init_bloom(){
	  bloom_parameters parameters;

	   // How many elements roughly do we expect to insert?
	   parameters.projected_element_count = 30;

	   // Maximum tolerable false positive probability? (0,1)
	//   parameters.maximum_size = 264;

	   parameters.false_positive_probability = 0.0001;
	   // Simple randomizer (optional)
	   parameters.random_seed = 0xA5A5A5A5;

	   if (!parameters)
	   {
	     // std::cout << "Error - Invalid set of bloom filter parameters!" << std::endl;
	      return;
	   }

	   parameters.compute_optimal_parameters();

	   //Instantiate Bloom Filter
	   m_filter = bloom_filter(parameters);
}



} // Namespace ns3
