/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 */
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "lte-echo-app.h"
#include "ns3/neighbor.h"
#include "ns3/nist-lte-ue-mac.h"
#include <ns3/energy-module-lte.h>
#include "ns3/udp-socket.h"
#include "Myheader.h"
#include "ns3/ptr.h"

#include <cstdint>
#include <list>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include <stdlib.h>
#include <sstream>
#include <ns3/nist-lte-helper.h>
#include <ns3/nist-module.h>



#include "ns3/address-utils.h"
#include "ns3/energy-module.h"
#include "ns3/traced-callback.h"
#include <algorithm>
#include "ns3/ptr.h"
#include "ns3/energy-module.h"
#include "ns3/li-ion-energy-source-helper.h"
#include "ns3/netanim-module.h"



namespace ns3 {

NS_LOG_COMPONENT_DEFINE("lte-echo-app");

NS_OBJECT_ENSURE_REGISTERED(Lte_Echo_Client);

TypeId Lte_Echo_Client::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::Lte_Echo_Client").SetParent<Application>().SetGroupName(
					"Applications").AddConstructor<Lte_Echo_Client>().AddAttribute(
					"MaxPackets",
					"The maximum number of packets the application will send",
					UintegerValue(100),
					MakeUintegerAccessor(&Lte_Echo_Client::m_count),
					MakeUintegerChecker<uint32_t>()).AddAttribute("if_index",
					"The maximum number of packets the application will send",
					UintegerValue(0),
					MakeUintegerAccessor(&Lte_Echo_Client::if_index),
					MakeUintegerChecker<uint32_t>()).AddAttribute("Interval",
					"The time to wait between packets", TimeValue(Seconds(1.0)),
					MakeTimeAccessor(&Lte_Echo_Client::m_interval),
					MakeTimeChecker()).AddAttribute("RemoteAddress",
					"The destination Address of the outbound packets",
					AddressValue(),
					MakeAddressAccessor(&Lte_Echo_Client::m_peerAddress),
					MakeAddressChecker()).AddAttribute("RemotePort",
					"The destination port of the outbound packets",
					UintegerValue(0),
					MakeUintegerAccessor(&Lte_Echo_Client::m_peerPort),
					MakeUintegerChecker<uint16_t>()).AddAttribute("PacketSize",
					"Size of echo data in outbound packets", UintegerValue(100),
					MakeUintegerAccessor(&Lte_Echo_Client::SetDataSize,
							&Lte_Echo_Client::GetDataSize),
					MakeUintegerChecker<uint32_t>()).AddTraceSource("Tx",
					"A new packet is created and is sent",
					MakeTraceSourceAccessor(&Lte_Echo_Client::m_txTrace),
					"ns3::Packet::TracedCallback").AddTraceSource("MyState",
					"A State value to trace.",
					MakeTraceSourceAccessor(&Lte_Echo_Client::m_myInt)).AddTraceSource(
					"n_packets", "A State value to trace.",
					MakeTraceSourceAccessor(&Lte_Echo_Client::n_packets));
	return tid;
}

void Lte_Echo_Client::RssiCallback(uint16_t rnti, double rssi){
	NS_LOG_FUNCTION(this);
	std::cout<<"00000000000 node "<<GetNode()->GetId()<<" received packet from "<<rnti<<" with rssi "<<rssi<<std::endl;;

	CapturedRssi.insert(std::pair<uint16_t, double>(rnti, rssi));


}


Lte_Echo_Client::Lte_Echo_Client() {
	NS_LOG_FUNCTION(this);
	m_sent = 0;
	m_socket = 0;
	m_socket2 = 0;
	m_sendEvent = EventId();
	m_data = 0;
	m_dataSize = 0;
}

Lte_Echo_Client::~Lte_Echo_Client() {
	NS_LOG_FUNCTION(this);
	m_socket = 0;

	delete[] m_data;
	m_data = 0;
	m_dataSize = 0;
}

void Lte_Echo_Client::SetRemote(Address ip, uint16_t port) {
	NS_LOG_FUNCTION(this << ip << port);
	m_peerAddress = ip;
	m_peerPort = port;
}

void Lte_Echo_Client::SetRemote(Address addr) {
	NS_LOG_FUNCTION(this << addr);
	m_peerAddress = addr;
}

void Lte_Echo_Client::DoDispose(void) {
	NS_LOG_FUNCTION(this);
	Application::DoDispose();
}

void Lte_Echo_Client::StartApplication(void) {
	NS_LOG_FUNCTION(this);

	if (m_socket == 0) {
		TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
		m_socket = Socket::CreateSocket(GetNode(), tid);
		if (Ipv4Address::IsMatchingType(m_peerAddress) == true) {
			m_socket->Bind();
			m_socket->BindToNetDevice(GetNode()->GetDevice(if_index));
			m_socket->Connect(
					InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress),
							m_peerPort));
		} else if (Ipv6Address::IsMatchingType(m_peerAddress) == true) {
			m_socket->Bind6();
			m_socket->Connect(
					Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress),
							m_peerPort));
		} else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true) {
			m_socket->Bind();
			m_socket->Connect(m_peerAddress);
		} else if (Inet6SocketAddress::IsMatchingType(m_peerAddress) == true) {
			m_socket->Bind6();

		} else {
			NS_ASSERT_MSG(false,
					"Incompatible address type: " << m_peerAddress);
		}
	}

	if (m_socket2 == 0) {
		TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
		m_socket2 = Socket::CreateSocket(GetNode(), tid);
		if (Ipv4Address::IsMatchingType(m_peerAddress) == true) {

			m_socket2->Bind();
			m_socket2->BindToNetDevice(GetNode()->GetDevice(if_index));
			m_socket2->Connect(
					InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress),
							999));

		} else if (Ipv6Address::IsMatchingType(m_peerAddress) == true) {
			m_socket2->Bind6();
			m_socket2->Connect(
					Inet6SocketAddress(Ipv6Address::ConvertFrom(m_peerAddress),
							999));
		} else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true) {
			m_socket2->Bind();
			m_socket2->Connect(m_peerAddress);
		} else if (Inet6SocketAddress::IsMatchingType(m_peerAddress) == true) {
			m_socket2->Bind6();

		} else {
			NS_ASSERT_MSG(false,
					"Incompatible address type: " << m_peerAddress);
		}
	}

	m_socket->SetRecvCallback(MakeCallback(&Lte_Echo_Client::HandleRead, this));
	m_socket->SetAllowBroadcast(true);
	m_socket2->SetRecvCallback(
			MakeCallback(&Lte_Echo_Client::HandleRead, this));
	m_socket2->SetAllowBroadcast(true);


    Ptr<NetDevice> dev = GetNode()->GetDevice(0);
    Ptr<NistLteUeNetDevice> ueDev = DynamicCast<NistLteUeNetDevice>(dev);
    Ptr<NistLteUePhy> uePhy = ueDev->GetPhy();
    Ptr<NistLteSpectrumPhy> uePhySpec = uePhy->GetSlSpectrumPhy();

    if(uePhySpec)
    	uePhySpec->SetNistLteRSSICallback(MakeCallback (&Lte_Echo_Client::RssiCallback, this));
	ScheduleTransmit(Seconds(0.));
	// ScheduleRequest (Seconds (0.1));
}

void Lte_Echo_Client::StopApplication() {
	NS_LOG_FUNCTION(this);

	if (m_socket != 0) {
		m_socket->Close();
		m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
		m_socket = 0;
	}

	Simulator::Cancel(m_sendEvent);
}

void Lte_Echo_Client::SetDataSize(uint32_t dataSize) {
	NS_LOG_FUNCTION(this << dataSize);

	//
	// If the client is setting the echo packet data size this way, we infer
	// that she doesn't care about the contents of the packet at all, so
	// neither will we.
	//
	delete[] m_data;
	m_data = 0;
	m_dataSize = 0;
	m_size = dataSize;
}

uint32_t Lte_Echo_Client::GetDataSize(void) const {
	NS_LOG_FUNCTION(this);
	return m_size;
}

void Lte_Echo_Client::SetFill(std::string fill) {
	NS_LOG_FUNCTION(this << fill);

	uint32_t dataSize = fill.size() + 1;

	if (dataSize != m_dataSize) {
		delete[] m_data;
		m_data = new uint8_t[dataSize];
		m_dataSize = dataSize;
	}

	memcpy(m_data, fill.c_str(), dataSize);

	//
	// Overwrite packet size attribute.
	//
	m_size = dataSize;
}

void Lte_Echo_Client::SetFill(uint8_t fill, uint32_t dataSize) {
	NS_LOG_FUNCTION(this << fill << dataSize);
	if (dataSize != m_dataSize) {
		delete[] m_data;
		m_data = new uint8_t[dataSize];
		m_dataSize = dataSize;
	}

	memset(m_data, fill, dataSize);

	//
	// Overwrite packet size attribute.
	//
	m_size = dataSize;
}

void Lte_Echo_Client::SetFill(uint8_t *fill, uint32_t fillSize,
		uint32_t dataSize) {
	NS_LOG_FUNCTION(this << fill << fillSize << dataSize);
	if (dataSize != m_dataSize) {
		delete[] m_data;
		m_data = new uint8_t[dataSize];
		m_dataSize = dataSize;
	}

	if (fillSize >= dataSize) {
		memcpy(m_data, fill, dataSize);
		m_size = dataSize;
		return;
	}

	//
	// Do all but the final fill.
	//
	uint32_t filled = 0;
	while (filled + fillSize < dataSize) {
		memcpy(&m_data[filled], fill, fillSize);
		filled += fillSize;
	}

	//
	// Last fill may be partial
	//
	memcpy(&m_data[filled], fill, dataSize - filled);

	//
	// Overwrite packet size attribute.
	//
	m_size = dataSize;
}

void Lte_Echo_Client::SetRandomVariableGenerator(
		Ptr<UniformRandomVariable> gen) {
	NS_LOG_FUNCTION(this);

	m_gen = gen;
}

void Lte_Echo_Client::ScheduleTransmit(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_sendEvent = Simulator::Schedule(dt, &Lte_Echo_Client::Send, this);
	NS_LOG_LOGIC(" trying to exite the scheduletransmit ");
}

void Lte_Echo_Client::ScheduleRequest(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	//Simulator::Schedule (dt, &Lte_Echo_Client::Request, this);

}

void Lte_Echo_Client::Send(void) {
	NS_LOG_FUNCTION(this);

	NS_ASSERT(m_sendEvent.IsExpired());

	std::ostringstream oss;
	my_energy->LIES.GetObject<LiIonEnergySource>()->m_remaining;
	oss << "type=discovery|nodeID=" << GetNode()->GetId() << "|batteryLvL="
			<< my_energy->LIES.GetObject<LiIonEnergySource>()->m_remaining
			<< "|neighbors=";

	std::map<Address, neighbor>::iterator it;
	std::list<Ptr<Packet>>::iterator it2;

	for (it = MyTable.begin(); it != MyTable.end(); it++) {

		oss << it->second.node_id << ",";
	}

	oss << "|";

	std::ostringstream oss2;
	oss2 << "hello there its me so what are you doing";

	std::string lemessage = oss.str();
	std::string message2 = oss2.str();

	Ptr<Packet> packet2 = Create<Packet>((uint8_t*) lemessage.c_str(),
			lemessage.length());
	Ptr<Packet> packet3 = Create<Packet>((uint8_t*) message2.c_str(),
			message2.length());

	//m_socket2->Send (packet3);
//std::cout<<"At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()<<" sent " << packet2->GetSize() << " bytes to " <<
	//    Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort<<std::endl;

//std::cout<<"The queue size is "<<packets_list.size()<<std::endl;

	bool newtech = true;
	if (state == "CDS")
		for (Ptr<Packet> packet_temp : packets_list) {
			if(newtech){
				uint8_t *buffer = new uint8_t[packet_temp->GetSize()];
				packet_temp->CopyData (buffer, packet_temp->GetSize());
				std::ostringstream oss;
				for(int i = 2; i<packet_temp->GetSize();i++){
					oss<<buffer[i];

				}

				int requested_servicetype = ParseServiceType(oss.str());
				int cdsState = cdsStates[requested_servicetype];
				std::cout<<" the packet message is "<<oss.str()<<std::endl;
				std::cout<<"request service type is "<<requested_servicetype<<
						" and cdsState is "<<cdsState<<std::endl;
				if(requested_servicetype==0 || cdsState > 0 ){
					m_socket2->Send(packet_temp);

						}
			}else{
				m_socket2->Send(packet_temp);
			}

//

		//
		}
	packets_list = {};

	if (state == "CDS")
	for (Ptr<Packet> packet_temp : neighbors_packets_list) {
			if(newtech){
			uint8_t *buffer = new uint8_t[packet_temp->GetSize()];
			packet_temp->CopyData (buffer, packet_temp->GetSize());
			std::ostringstream oss;
			for(int i = 2; i<packet_temp->GetSize();i++){
				oss<<buffer[i];

			}

			int requested_servicetype = ParseServiceType(oss.str());
			int cdsState = cdsStates[requested_servicetype];
			std::cout<<" the packet message is "<<oss.str()<<std::endl;
			std::cout<<"request service type is "<<requested_servicetype<<
					" and cdsState is "<<cdsState<<std::endl;
			if(requested_servicetype==0 || cdsState > 0 ){
				m_socket->Send(packet_temp);

					}
		}else{
			m_socket->Send(packet_temp);
		}

		}

	neighbors_packets_list = {};

	++m_sent;

	if (Ipv4Address::IsMatchingType(m_peerAddress)) {
		NS_LOG_INFO(
				"At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " << Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
	}

	else if (InetSocketAddress::IsMatchingType(m_peerAddress)) {
		NS_LOG_INFO(
				"At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
	}

	ScheduleCheck(m_interval);
	ScheduleTransmit(m_interval);

}

void Lte_Echo_Client::Request(uint32_t rnd_node, uint32_t packet_id, int servicetype) {
	n_packets++;
	NS_LOG_FUNCTION(this);
	std::ostringstream oss;
	std::map<Address, neighbor>::iterator it;
	std::map<Address, neighbor>::iterator result;
	std::list<int>::iterator result2;

//  uint32_t rnd_node = m_gen->GetInteger(1,numberUE);
//  uint32_t packet_id = m_gen->GetInteger(0,50000);
	NS_LOG_LOGIC("   selected node is  "<<rnd_node);

	// double rnd_id = gen->GetInteger(0,1000000000);

	bool found = false;
	bool found2 = false;
	for (it = MyTable.begin(); it != MyTable.end(); it++) {
		found = (it->second.node_id == rnd_node);
		if (found) {
			result = it;
			NS_LOG_INFO("found in my neighbors ");
			break;

		}
	}

	if ((!found)
			|| ((found) && (InetSocketAddress::IsMatchingType(result->first)))) {

		//std::cout<<"batteryLVL is "<<m_source;
		oss << "type=request|nodeID=" << GetNode()->GetId() << "|RequestId="
				<< packet_id << "|R_node=" << rnd_node;
		oss << "|R_servicetype=" << servicetype << "|";

		std::string lemessage = oss.str();

		Ptr<Packet> packet2 = Create<Packet>((uint8_t*) lemessage.c_str(),
				lemessage.length());
		MyHeader sourceheader;
		sourceheader.SetData(1);
		packet2->AddHeader(sourceheader);
		// std::cout<<" i am node" <<GetNode()->GetId() << " trying to send a request "<< packet_id <<" to node " << (double)rnd_node
		//		<<"interval is "<<(10*m_interval)<< std::endl;
		// std::cout<<"debug here "<<std::endl;
		if (found)
			m_socket->Send(packet2);
		else
			m_socket2->Send(packet2);
		//std::cout<<"packet sent ok "<<std::endl;

		trace_sending_packet(packet_id, Simulator::Now().GetSeconds());

		++m_sent;

	} else if (found && Inet6SocketAddress::IsMatchingType(result->first)) {
		//std::cout<<packet_id<<" its in my neighbors lowpan" << std::endl;
		//std::cout<<"found but" << Inet6SocketAddress::IsMatchingType(result->first);

		oss << "type=request|nodeID=" << GetNode()->GetId() << "|RequestId="
				<< packet_id << "|R_node=" << rnd_node;
		oss << "|R_servicetype=" << servicetype << "|";

		std::string lemessage = oss.str();
		Ptr<Packet> packet2 = Create<Packet>((uint8_t*) lemessage.c_str(),
				lemessage.length());
		MyHeader sourceheader;
		sourceheader.SetData(0);
		packet2->AddHeader(sourceheader);
		trace_receiving_packet(packet_id, Simulator::Now().GetSeconds());
		// packets6_list->push_back(packet2);

		NS_LOG_INFO(
				"Sending a----------------------------------------------- " <<result->first);
	}
	// ScheduleRequest (10*m_interval);

}

void Lte_Echo_Client::HandleRead(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	//std::cout<<" reading the received packet here ";
	Ptr<Packet> packet;
	Address from;
	std::string data;
	while ((packet = socket->RecvFrom(from))) {
		if (InetSocketAddress::IsMatchingType(from)) {
			NS_LOG_INFO(
					"At time " << Simulator::Now ().GetSeconds () << "s client  " << GetNode()->GetId() <<" received " << packet->GetSize () << " bytes from " << InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (from).GetPort ());
			uint8_t *buffer = new uint8_t[packet->GetSize()];
			packet->CopyData(buffer, packet->GetSize());
			data = std::string((char*) buffer);
		}

	}

	neighbor n = ParseMassage(data, from);

	//std::cout<<"in node : " <<GetNode()->GetId()<<" trying to insert node "<< n.node_id<<std::endl;
	//std::cout<<"data is : " <<data;
	std::map<Address, neighbor>::iterator it;
	// std::cout<<"node " <<GetNode()->GetId()<<" has in table : ";
	bool exists = false;
	for (it = MyTable.begin(); it != MyTable.end(); it++) {
		// std::cout<<it->second.node_id<<" ,";
		if (it->second.node_id == n.node_id) {
			MyTable.erase(it);

			MyTable.insert(std::pair<Address, neighbor>(from, n));
			exists = true;

		}
	}
	//std::cout<<std::endl;

	if (!exists) {

		MyTable.insert(std::pair<Address, neighbor>(from, n));
	}

}

void Lte_Echo_Client::ScheduleCheck(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Lte_Echo_Client::UpdateTable, this);
}

int
Lte_Echo_Client::ParseServiceType(std::string msg){
	NS_LOG_FUNCTION (this);

	int type ;

	std::vector<std::string> fields = split(msg,'|');
	std::istringstream ( split(fields.at(4),'=')[1]) >> type;
return type;
}


bool Lte_Echo_Client::allconnected(int servicetype)
{
	 NS_LOG_FUNCTION (this);
	std::map<Address, neighbor>::iterator it;
	std::map<Address, neighbor>::iterator it2;

			 for ( it = MyTable.begin(); it != MyTable.end(); it++ ){
				 if(it->second.servicetype != servicetype || it->second.servicetype != 0)
					continue;
				 for ( it2 = it; it2 != MyTable.end(); it2++ ){
					 if(it2->second.servicetype != servicetype || it2->second.servicetype != 0)
						 continue;
					 bool found = (std::find(it2->second.neighbors.begin(), it2->second.neighbors.end(),it->second.node_id ) != it2->second.neighbors.end());

					 if(!found)
						 return false;

				 				 }

			 }


			 return true;
}

bool Lte_Echo_Client::covered(int servicetype){
	 NS_LOG_FUNCTION (this);
	std::map<Address, neighbor>::iterator it;
	std::list<int> newnbrs;
	for ( it = MyTable.begin(); it != MyTable.end(); it++ )
		  {
		if(it->second.servicetype == servicetype || it->second.servicetype == 0){
			newnbrs.push_back(it->second.node_id);
		}

		  }
	  for ( it = MyTable.begin(); it != MyTable.end(); it++ )
	  {


		  if(it->second.servicetype == 0)
		  if(contains(newnbrs,it->second.neighbors) && it->second.node_id>GetNode()->GetId())
			  	  return true;

		  }
	  return false;

}



void Lte_Echo_Client::updatestate() {
	//std::cout<< " trying to determin the fait of node "<<GetNode()->GetId()<<std::endl;
	std::map<Address, neighbor>::iterator it;
	std::list<int> N_ids;
	int my_id;

	for (it = MyTable.begin(); it != MyTable.end(); it++) {
		N_ids.push_back(it->second.node_id);

	}
	my_id = GetNode()->GetId();
	N_ids.push_back(my_id);

	if (MyTable.size() == 0) {
		state = "CDS";
		m_myInt = 1;
	}

	else if (covered(N_ids)) {
		state = "temp";
		m_myInt = 0;
	} else if (allconnected()) {
		state = "temp";
		m_myInt = 0;
	} else {
		state = "CDS";
		m_myInt = 1;
	}

	  if(state == "CDS") {
		  std::cout<<" the state of node "<<GetNode()->GetId()<<" is CDS in case 0 "<<std::endl;
		  for (int i = 1; i<number_of_types_of_services+1; i++){
			  if (! (covered(i) && allconnected(i) && MyTable.size()>0)){
				  cdsStates[i] = 1;

					  std::cout<<" the state of node "<<GetNode()->GetId()<<" is CDS in case "<< i <<std::endl;
				  }else
					  cdsStates[i] = 0;
		  }
	  }

	std::cout << "at time " << Simulator::Now().GetSeconds() << "node "
			<< (double) GetNode()->GetId() << " became a " << state << "with :";
	for (int n : N_ids) {
		std::cout << (double) n << ',';
	}
	std::cout << std::endl;
	//  NS_LOG_LOGIC ("this is the energy that i was looking for "<<my_energy->LIES.m_remaining);
	//LoadBalance();
	NS_LOG_LOGIC(
			"node "<<(double)GetNode()->GetId()<<" became a aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa "<<state);
//	    std::cout<<"node "<<GetNode()->GetId()<<" became a "<<state<<" my neighbors are :"<<std::endl;
//
//	  	  for ( it = MyTable.begin(); it != MyTable.end(); it++ )
//	  	 	  {
//	  		  std::cout<<"node "<<(double)it->second.node_id<<" and has neighbors ";
//	  		  for (int x : it->second.neighbors)
//	  			  std::cout<<(double) x <<" , ";
//	  		  std::cout<<std::endl;
//	  	 	  }
//	state = "CDS";
//	m_myInt = 1;
	if (state == "CDS")
		count_cover(N_ids);
}

bool Lte_Echo_Client::covered(std::list<int> my_nbrs) {

	std::map<Address, neighbor>::iterator it;
	for (it = MyTable.begin(); it != MyTable.end(); it++) {

		if (contains(my_nbrs, it->second.neighbors)
				&& it->second.node_id > GetNode()->GetId()) {
			std::cout << "+*+*+*+* this is node " << (double) GetNode()->GetId()
					<< " is covered by " << (double) it->second.node_id
					<< std::endl;
			return true;

		}

	}
	return false;

}

bool Lte_Echo_Client::contains(std::list<int> list1, std::list<int> list2) {
	if (list1.size() > list2.size())
		return false;

	for (int i : list1) {
		bool found = (my_find(list2.begin(), list2.end(), i) != list2.end());
		if (!found)
			return false;
	}
	return true;

}

bool Lte_Echo_Client::allconnected() {
	std::map<Address, neighbor>::iterator it;
	std::map<Address, neighbor>::iterator it2;

	for (it = MyTable.begin(); it != MyTable.end(); it++)
		for (it2 = it; it2 != MyTable.end(); it2++) {
			bool found = (my_find(it2->second.neighbors.begin(),
					it2->second.neighbors.end(), it->second.node_id)
					!= it2->second.neighbors.end());

			if (!found)
				return false;

		}

	return true;
}

void Lte_Echo_Client::UpdateTable(void) {
	NS_LOG_FUNCTION(this);
	double inseconds = 5 * m_interval.GetSeconds();
	std::map<Address, neighbor>::iterator it;
	NS_LOG_LOGIC("befor delete"<<MyTable.size());
	for (it = MyTable.begin(); it != MyTable.end(); it++) {

		if (Simulator::Now().GetSeconds() - it->second.discovered_time
				>= inseconds) {

			it = MyTable.erase(it);
			if (MyTable.size() < 1)
				return;

		}

	}
	updatestate();
	NS_LOG_LOGIC("after delete"<<MyTable.size());

}

template<class InputIterator, class T>
InputIterator Lte_Echo_Client::my_find(InputIterator first, InputIterator last,
		const T& val)

		{
	while (first != last) {
		if (*first == val)
			return first;
		++first;
	}
	return last;
}

neighbor Lte_Echo_Client::ParseMassage(std::string msg, Address from) {

	int id;
	std::list<int> Nbors;
	double battery;

	std::vector<std::string> fields = split(msg, '|');

	std::istringstream(split(fields.at(0), '=')[1]) >> id;

	std::istringstream(split(fields.at(1), '=')[1]) >> battery;
	std::vector<std::string> temp = split(fields.at(2), '=');

	if (temp.size() > 1) {
		std::string rest = split(fields.at(2), '=')[1];

		std::vector<std::string> nb = split(rest, ',');

		for (uint i = 0; i < nb.size() - 1; i++) {

			int x;
			std::istringstream(nb[i]) >> x;
			Nbors.push_back(x);

		}
		Nbors.push_back(id);
	}

	neighbor n(from, 2, battery, Nbors, Simulator::Now().GetSeconds(), id);

	return n;

}

std::vector<std::string> Lte_Echo_Client::split(const std::string& s,
		char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(s);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

Ptr<Socket> Lte_Echo_Client::GetBroadcastSocket() {
	return m_socket;
}

void Lte_Echo_Client::SetPacketsv6List(std::list<Ptr<Packet>> *t_list) {
	packets6_list = t_list;
}

bool Lte_Echo_Client::LoadBalance() {
	std::map<Address, neighbor>::iterator it;
	std::map<Address, neighbor>::iterator it2;
	std::map<Address, neighbor> Covered(MyTable);
	std::map<Address, neighbor> dominating_nodes;
	double m_selection_factor = 0.0;
	double energyprcent = (my_energy->LIES.m_remaining)
			/ my_energy->LIES.GetInitialEnergy() * 100;
	if (energyprcent != 0)
		m_selection_factor = (1 / energyprcent) * Covered.size();
	else
		m_selection_factor = 10000;

	for (it = Covered.begin(); it != Covered.end(); it++)
		if (it->second.status != 0)
			dominating_nodes.insert(
					std::pair<Address, neighbor>(it->first, it->second));

	if (dominating_nodes.size() < 1)
		return true;

	for (it = Covered.begin(); it != Covered.end(); it++) {
		//finds the best dominating node to cover node i
		int idmax = CalculateMaxSf(dominating_nodes, Covered.size());


		// removes node i from all other dominating nodes.
		for (it2 = dominating_nodes.begin(); it2 != dominating_nodes.end();
				it2++)
			if (it2->second.node_id != idmax
					&& (my_find(it2->second.neighbors.begin(),
							it2->second.neighbors.end(), it->second.node_id)
							!= it2->second.neighbors.end())
					&& (my_find(it2->second.neighbors.begin(),
							it2->second.neighbors.end(), idmax)
							!= it2->second.neighbors.end())) {
				it2->second.neighbors.remove(it->second.node_id);

			}
		if (GetNode()->GetId() != idmax)
			it = Covered.erase(it);
		if (Covered.size() < 1)
			return true;
	}

	//send the join message to all covered nodes.
	//SendJoinMessageToCoveredNodes(Covered);
	return true;
}

/*
 * this function finds the best dominating node to cover interms of remaining energy
 * and number of covered nodes
 */
int Lte_Echo_Client::CalculateMaxSf(
		std::map<Address, neighbor> dominating_nodes, int m_coverage_degree) {
	std::map<Address, neighbor>::iterator it;
	int idmax = 0;
	double sfmax = 100000;
	for (it = dominating_nodes.begin(); it != dominating_nodes.end(); it++) {
		double sf = (1 / it->second.battery) * it->second.neighbors.size();
		if (sf < sfmax) {
			sfmax = sf;
			idmax = it->second.node_id;
		}

	}
	double energyprcent = (my_energy->LIES.m_remaining)
			/ my_energy->LIES.GetInitialEnergy() * 100;
	double m_selection_factor = 0.0;
	if (energyprcent != 0)
		m_selection_factor = (1 / energyprcent) * m_coverage_degree;
	else
		m_selection_factor = 10000;
	if (m_selection_factor < sfmax)
		idmax = GetNode()->GetId();
	return idmax;

}

void Lte_Echo_Client::SendJoinMessageToCoveredNodes(
		std::map<Address, neighbor> covered) {

	NS_LOG_LOGIC("trying to send a join message to all covered nodes");
	NS_LOG_FUNCTION(this);
	std::ostringstream oss;
	oss << "type=cover|nodeID=" << GetNode()->GetId() << "|neighbors=";
	std::map<Address, neighbor>::iterator it;
	for (it = covered.begin(); it != covered.end(); it++) {
		oss << it->second.node_id << ",";
	}

	oss << "|TxPwr="<<CalculateBestTxPower(covered,CapturedRssi)<<"|";
	std::string lemessage = oss.str();

	Ptr<Packet> packet2 = Create<Packet>((uint8_t*) lemessage.c_str(),
			lemessage.length());

	NS_LOG_LOGIC("Echoing packet");

	m_socket->Send(packet2);

}

double Lte_Echo_Client::CalculateBestTxPower(std::map<Address, neighbor> covered, std::map<uint16_t,double> CapturedRssi){
	return 18.0;
}

} // Namespace ns3
