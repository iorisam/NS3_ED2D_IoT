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
#include "need-prose-app.h"
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
#include <math.h>



#include "ns3/address-utils.h"
#include "ns3/energy-module.h"
#include "ns3/traced-callback.h"
#include <algorithm>
#include "ns3/ptr.h"
#include "ns3/energy-module.h"
#include "ns3/li-ion-energy-source-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/energy-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4.h"

#include <ns3/nist-lte-helper.h>
#include <ns3/nist-module.h>
#include "ns3/nist-sl-resource-pool-factory.h"
#include "ns3/nist-sl-preconfig-pool-factory.h"
#include "seq-ts-header.h"




namespace ns3 {

NS_LOG_COMPONENT_DEFINE("need-prose-app");

NS_OBJECT_ENSURE_REGISTERED(Need_ProSe_App);

TypeId Need_ProSe_App::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::Need_ProSe_App").SetParent<Application>().SetGroupName(
					"Applications").AddConstructor<Need_ProSe_App>().AddAttribute(
					"MaxPackets",
					"The maximum number of packets the application will send",
					UintegerValue(100),
					MakeUintegerAccessor(&Need_ProSe_App::m_count),
					MakeUintegerChecker<uint32_t>()).AddAttribute("if_index",
					"The maximum number of packets the application will send",
					UintegerValue(0),
					MakeUintegerAccessor(&Need_ProSe_App::if_index),
					MakeUintegerChecker<uint32_t>()).AddAttribute("Interval",
					"The time to wait between packets", TimeValue(Seconds(1.0)),
					MakeTimeAccessor(&Need_ProSe_App::m_interval),
					MakeTimeChecker()).AddAttribute("RemoteAddress",
					"The destination Address of the outbound packets",
					AddressValue(),
					MakeAddressAccessor(&Need_ProSe_App::m_peerAddress),
					MakeAddressChecker()).AddAttribute("RemotePort",
					"The destination port of the outbound packets",
					UintegerValue(0),
					MakeUintegerAccessor(&Need_ProSe_App::m_peerPort),
					MakeUintegerChecker<uint16_t>()).AddAttribute("PacketSize",
					"Size of echo data in outbound packets", UintegerValue(100),
					MakeUintegerAccessor(&Need_ProSe_App::SetDataSize,
							&Need_ProSe_App::GetDataSize),
					MakeUintegerChecker<uint32_t>()).AddTraceSource("Tx",
					"A new packet is created and is sent",
					MakeTraceSourceAccessor(&Need_ProSe_App::m_txTrace));
	return tid;
}

Need_ProSe_App::Need_ProSe_App() {
	NS_LOG_FUNCTION(this);
	m_sent = 0;
	m_socket = 0;
	m_socket2 = 0;
	m_sendEvent = EventId();
	m_data = 0;
	m_dataSize = 0;
}

Need_ProSe_App::~Need_ProSe_App() {
	NS_LOG_FUNCTION(this);
	m_socket = 0;
	 m_socket6 = 0;

	delete[] m_data;
	m_data = 0;
	m_dataSize = 0;
}

void Need_ProSe_App::SetRemote(Address ip, uint16_t port) {
	NS_LOG_FUNCTION(this << ip << port);
	m_peerAddress = ip;
	m_peerPort = port;
}

void Need_ProSe_App::SetRemote(Address addr) {
	NS_LOG_FUNCTION(this << addr);
	m_peerAddress = addr;
}

void Need_ProSe_App::DoDispose(void) {
	NS_LOG_FUNCTION(this);
	Application::DoDispose();
}

void Need_ProSe_App::StartApplication(void) {
	NS_LOG_FUNCTION(this);

		  	//m_socket->SetRecvCallback(MakeCallback(&Need_ProSe_App::HandleRead, this));

		  	SetupOutSocket();
		  	SetupInSocket();
		  	m_socket->SetAllowBroadcast(true);
		  	m_socket2->SetRecvCallback(
		  			MakeCallback(&Need_ProSe_App::HandleRead, this));
		  	m_socket2->SetAllowBroadcast(true);

		  	//ScheduleTransmit(Seconds(0.));
		  	// ScheduleRequest (Seconds (0.1));
		  	ScheduleRound(Seconds (0.1));
		  	SetupProSeDiscoveryCallback();



}

void Need_ProSe_App::SetupOutSocket(){
	NS_LOG_FUNCTION (this);
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
}


void Need_ProSe_App::SetupInSocket(){
	NS_LOG_FUNCTION (this);
	 if (m_socket2 == 0)
	    {
	      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	      m_socket2 = Socket::CreateSocket (GetNode (), tid);
	      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_peerPort);


	      m_socket2->Bind (local);
	     // if(m_device1 != 0)
	         //	  m_socket->BindToNetDevice(m_device1);

	      if (addressUtils::IsMulticast (m_local))
	        {
	          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket2);
	          if (udpSocket)
	            {
	              // equivalent to setsockopt (MCAST_JOIN_GROUP)
	              udpSocket->MulticastJoinGroup (0, m_local);
	            }
	          else
	            {
	              NS_FATAL_ERROR ("Error: Failed to join multicast group");
	            }
	        }
	    }

	 if(m_socket6 == 0)
	 {
		   TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		   m_socket6 = Socket::CreateSocket (GetNode (), tid);
		   Inet6SocketAddress sinkAdr = Inet6SocketAddress (Ipv6Address("FF02::2"), m_peerPort);
		   m_socket6->Bind(sinkAdr);
		   //m_socket6->BindToNetDevice(m_device1);
		   m_socket6->SetRecvCallback (MakeCallback (&Need_ProSe_App::HandleRead, this));

	 }

}

void Need_ProSe_App::StopApplication() {
	NS_LOG_FUNCTION(this);

	if (m_socket != 0) {
		m_socket->Close();
		m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
		m_socket = 0;
	}

	if (m_socket2 != 0) {
		m_socket2->Close();
		m_socket2->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
		m_socket2 = 0;
		}

	Simulator::Cancel(m_sendEvent);
	Simulator::Cancel(m_RoundEvent);
	Simulator::Cancel(m_BTCalculationEvent);
	Simulator::Cancel(m_MainBroadcastEvent);
}

void Need_ProSe_App::SetDataSize(uint32_t dataSize) {
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

uint32_t Need_ProSe_App::GetDataSize(void) const {
	NS_LOG_FUNCTION(this);
	return m_size;
}



void Need_ProSe_App::SetRandomVariableGenerator(
		Ptr<UniformRandomVariable> gen) {
	NS_LOG_FUNCTION(this);

	m_gen = gen;
}

void Need_ProSe_App::ScheduleTransmit(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_sendEvent = Simulator::Schedule(dt, &Need_ProSe_App::Send, this);

}



void Need_ProSe_App::ScheduleRequest(Time dt) {
	NS_LOG_FUNCTION(this << dt);


}

void Need_ProSe_App::ScheduleRound(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_RoundEvent = Simulator::Schedule(dt, &Need_ProSe_App::initiate_round, this);
	NS_LOG_LOGIC(" next round is scheduled after "<<dt);
}

void Need_ProSe_App::ScheduleBroadcastTimeCalculation(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_BTCalculationEvent = Simulator::Schedule(dt, &Need_ProSe_App::CalculateBroadcastTime, this);

}

void Need_ProSe_App::ScheduleMainBroadcast(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_MainBroadcastEvent = Simulator::Schedule(dt, &Need_ProSe_App::BroadcastMainClusterHeadMessage, this);

}

void Need_ProSe_App::ScheduleWakeTimeBroadcast(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Need_ProSe_App::BroadcastWakeTimeSchedule, this);

}

void Need_ProSe_App::ScheduleMainViceAlternation(Time dt,bool mainHead) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Need_ProSe_App::Alternate, this);

}

void Need_ProSe_App::ScheduleCondidateMessageBroadcast(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Need_ProSe_App::BroadcastCondidateMessage, this);

}






void Need_ProSe_App::initiate_round(){
	NS_LOG_FUNCTION(this);
	double random_number = .00001*(rand() % 100000);
	double threshold = CalculateThreshold();
	NS_LOG_INFO(" calculated threshold is  "<<threshold<<" ,generated random number is "<<random_number);
	initilizeVaribales();
    if(random_number < threshold){
    	double rnd = .00001*(rand() % 100000);
    	ScheduleCondidateMessageBroadcast(Seconds(rnd));
    	state = "condidate";
    	ScheduleBroadcastTimeCalculation(Seconds (2.0));

    }
    ScheduleRound(Seconds (10.0));
    ScheduleCheck(Seconds (5.0));




}

void Need_ProSe_App::CalculateBroadcastTime(){
	NS_LOG_FUNCTION (this);

	 if (state == "condidate")
		state = "TempMainHead";
	else
		state = "UE";
	double broadcastTime = CalculateBroadcastT();
	if(GetNode()->GetId()-1 < 1)
		state = "MainHead";
	if(broadcastTime<0){
		broadcastTime = 2.8;
	}
	m_rand_time = broadcastTime;
		ScheduleMainBroadcast(Seconds(0.1));
		NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << " node "<<GetNode()->GetId()-1 <<" will broadcast* "<< state<<" after  "<<broadcastTime);


}

void Need_ProSe_App::initilizeVaribales(){
	NS_LOG_FUNCTION(this);
	main_heads_list.clear();
	neighbors_list.clear();
	vice_cluster_heads.clear();
	new_round = true;
	current_round++;
	state = "UE";
	cluster_head = -1;
	vice_cluster_head = -1;
	ResetProSeDiscovery();
	m_rand_time = 1006;
	v_rand_time = 1014;
}

void Need_ProSe_App::Send(void) {
	NS_LOG_FUNCTION(this);

	NS_ASSERT(m_sendEvent.IsExpired());



}

void Need_ProSe_App::HandleRead(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	Ptr<Packet> packet;
	  Address from;
	  std::string data;
	if(m_socket6 != 0){
	  while ((packet = socket->RecvFrom (from)))
	    {
		  	  if (Inet6SocketAddress::IsMatchingType (from))
	        {
	          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server"<< GetNode()->GetId()<<" received " << packet->GetSize () << " bytes from " <<
	                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
	                       Inet6SocketAddress::ConvertFrom (from).GetPort ());

	        }
	      MyHeader currenthops ;
	      packet->RemoveHeader (currenthops);
	      uint8_t *buffer = new uint8_t[packet->GetSize()];
	                                 		packet->CopyData (buffer, packet->GetSize());
	                                 		data = std::string((char*)buffer);
	    }

	}

	/* copying ipv6 packet to an ipv4 packet and sending it to sink*/
	  if (Inet6SocketAddress::IsMatchingType (from))
		        {
		  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
		   Ptr<Socket> source = Socket::CreateSocket(GetNode(), tid);
		   Ptr<Ipv4> ipv4 = ns3::NodeList::GetNode(1)->GetObject<Ipv4> ();
		  		 Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
		  		InetSocketAddress sinkAdr = InetSocketAddress (iaddr.GetLocal(), 8050);
		  		source->Bind();
		  		source->BindToNetDevice(GetNode()->GetDevice(if_index));

		      source->Connect(sinkAdr);
		      source->SetAllowBroadcast(true);
		      SeqTsHeader seqTs;
		       int seq = rand() % 100000;
		       seqTs.SetSeq (seq);


	Ptr<Packet> p = Create<Packet> ((uint8_t*) data.c_str(), data.length());
	p->AddHeader (seqTs);
		  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()-1<<" translated ipv6 packet from " <<
				  from << " to " <<sinkAdr << " port " << 8050);
		  source->Send (p);
		  ++m_sent;
		        }

	 NS_LOG_INFO ("out of handle read");

}


void
Need_ProSe_App::handleDiscoveryMessage (uint16_t sender, std::string DiscMsg)
{
	NS_LOG_FUNCTION (this);

	Need_Message msg = ParseMassage(DiscMsg);

	if(msg.message_type == "c")
		  if (my_find(neighbors_list.begin(),neighbors_list.end(),sender) == neighbors_list.end()){
			  double distance = CalculateDistance(GetNode()->GetId()-1, sender);
			  NS_LOG_INFO ("At time "<< Simulator::Now ().GetSeconds () <<" distance between Node "<<GetNode()->GetId()-1<<" and node "<<sender<<" is "<<distance);
			  NS_LOG_INFO ("At time "<< Simulator::Now ().GetSeconds () <<" distance between Node "<<GetNode()->GetId()-1<<" with competion radius "<<
					  CalculateCompetitionRadius(GetNode()->GetId()-1) <<" and node "<<sender<<" with coompetion radius "<< CalculateCompetitionRadius(sender)<<"is "<<distance);
			  if(distance < std::max(CalculateCompetitionRadius(GetNode()->GetId()-1),CalculateCompetitionRadius(sender))){
				  neighbors_list.push_back(sender);
				  NS_LOG_INFO ("At time "<< Simulator::Now ().GetSeconds () <<" Node "<<GetNode()->GetId()-1<<" going to add node "<<sender<<" to it's neighbors");
			  }else {
				  NS_LOG_INFO ("At time "<< Simulator::Now ().GetSeconds () <<" Node "<<GetNode()->GetId()-1<<" is not going to add node "<<sender<<" to it's neighbors");
			  }

		  }

	 if(msg.message_type == "m" && GetNode()->GetId()-1 != 0){
			  if (my_find(neighbors_list.begin(),neighbors_list.end(),sender) != neighbors_list.end()){
				  NS_LOG_INFO (" Node "<<GetNode()->GetId()-1<<" going to compare with node "<<sender);
				  double received_main_time = ParseRandTime(DiscMsg);

			  if(received_main_time<m_rand_time){
				  NS_LOG_INFO (" and i lost ");
					MainHeadMsg.message_type = "Quit";
					vice_cluster_head = -1;
					state = "Quit";
					//ResetProSeDiscovery();
			  }else if (received_main_time<v_rand_time){
				  v_rand_time = received_main_time;
				  vice_cluster_head = sender;
				  NS_LOG_INFO (" Node "<<GetNode()->GetId()-1<<" is setting vice cluster head "<<sender);
			  }

			  }
		  }

	 if(msg.message_type == "q"){

	 				if(cluster_head == this->GetNode()->GetId()-1 && vice_cluster_head<0){
	 					if(GetNode()->GetId()-1 == 0)
	 						vice_cluster_head = 0;
	 					else {
	 						vice_cluster_head = msg.sender_id;
	 						previous_pheromone_concentrations[vice_cluster_head] = 22;
	 					}

	 				}

	 	  }

	  if(msg.message_type == "w" && GetNode()->GetId()-1 != 0){
		  NS_LOG_INFO ("Node "<<GetNode()->GetId()-1<<" Main head "<<sender<<" wake up time state is "<<state);
		  int temp_vice_head = -1;
		  temp_vice_head = ParseViceClusterHead(DiscMsg);
		  NS_LOG_INFO("they are sending vice cluster head as "<<temp_vice_head);
		  if(temp_vice_head<0){
				NS_LOG_INFO("they are sending -1 vice ");
				if(GetNode()->GetId()-1 <1)
					temp_vice_head = GetNode()->GetId()-1;
				else
					temp_vice_head = sender;
		  				  }
		  NS_LOG_INFO (GetNode()->GetId()-1<<" Main head state is "<<cluster_head<<" current state is not mainhead "<<(state != "MainHead"));
		  if(cluster_head<0 && state != "MainHead"){
				cluster_head = sender;
				vice_cluster_head = temp_vice_head;
				wake_up_time = ParseWakeUpTime(DiscMsg);
				NS_LOG_INFO (GetNode()->GetId()-1<<" Main head "<<sender<<" wake up time is "<<wake_up_time);
		  }

			  if (my_find(main_heads_list.begin(),main_heads_list.end(),sender) == main_heads_list.end()){
				  main_heads_list.push_back(sender);
				  previous_pheromone_concentrations[sender] = 22;
				  NS_LOG_INFO ("adding node "<<sender<<" to main heads list");
			  }

		if(state == "MainHead"){
			cluster_head = GetNode()->GetId()-1;
		}



		  vice_cluster_heads[sender] = temp_vice_head;
		  NS_LOG_INFO (GetNode()->GetId()-1<<" Main head "<<cluster_head<<" vice cluster head is "<<vice_cluster_head);
		  ScheduleMainViceAlternation(Seconds(1.0),true);
	  }


}

/* todo initiate cluster head duties
 * complete the alternation process (cluster becomes vice and vice becomes cluster and other nodes must know
 * add static routing and then modify the defaut route for nodes to become whether main or vice
 * understand maca algorithm
 * implement maca algorithm
 * modify the default route for cluster heads and vice cluster heads based on maca
 * create an on off application for data transmission
 * test and get results
 * compare and plot
 */

double Need_ProSe_App::CalculateDistance(int nodeI,int  nodeJ){
	NS_LOG_FUNCTION(this);
	if(nodeI <1)
		nodeI = 1;
	if(nodeJ <1)
		nodeJ = 1;
	Ptr<MobilityModel> i_mob = ns3::NodeList::GetNode(nodeI)->GetObject<MobilityModel>();
	Ptr<MobilityModel> j_mob = ns3::NodeList::GetNode(nodeJ)->GetObject<MobilityModel>();
	Ptr<MobilityModel> s_mob = ns3::NodeList::GetNode(5)->GetObject<MobilityModel>();
	NS_LOG_INFO ("i'm node : "<<nodeI<<" is in "<<i_mob->GetPosition());
	NS_LOG_INFO ("i'm node : "<<nodeJ<<" is in "<<j_mob->GetPosition());
	double distance = i_mob->GetDistanceFrom(j_mob);
	return distance;
}
void Need_ProSe_App::ScheduleCheck(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Need_ProSe_App::LogStatus, this);
}

void Need_ProSe_App::updatestate() {
	NS_LOG_FUNCTION (this);
}


bool Need_ProSe_App::contains(std::list<uint32_t> list1, uint32_t nodeId) {
	NS_LOG_FUNCTION (this);
	for (uint32_t i : list1) {
		NS_LOG_INFO ("i'm node : " <<GetNode()->GetId()-1<<" my neighbors cluster heads are : " <<i);
		if(i == nodeId)
			return true;
	}
	return false;

}


void Need_ProSe_App::LogMe(){
	NS_LOG_FUNCTION(this);
	if(cluster_head < 0){
			cluster_head = GetNode()->GetId()-1;

		}
	if(vice_cluster_head <0)
		vice_cluster_head = cluster_head;
	if(GetNode()->GetId()-1 <1){
		cluster_head = 0;
		vice_cluster_head = cluster_head;
	}

	NS_LOG_INFO ("At time : " << Simulator::Now ().GetSeconds () <<" Node "<<GetNode()->GetId()-1<<"status is "<<this->state);
	NS_LOG_INFO ("my cluster head is  " << cluster_head <<" and vice cluster head is  "<< vice_cluster_head);
	InitiateDuties();
	ResetProSeDiscovery();

}

void Need_ProSe_App::Alternate(){
	NS_LOG_FUNCTION(this);
	int temp;
	if(!new_round){

		temp = cluster_head;
		cluster_head = vice_cluster_head;
		vice_cluster_head = temp;
		main_heads_list.remove(vice_cluster_head);
		main_heads_list.push_back(cluster_head);
		main_head_incharge = !main_head_incharge;
		if(main_head_incharge){
				ScheduleMainViceAlternation(Seconds(wake_up_time),main_head_incharge);

		}else{
				ScheduleMainViceAlternation(Seconds(1-wake_up_time),main_head_incharge);

			}

	}
}

void Need_ProSe_App::InitiateDuties(){
	NS_LOG_FUNCTION (this);
	Ipv4Address nexthop ;
	std::list<uint32_t> condidates;
	if(cluster_head < 0 )
		cluster_head = GetNode()->GetId()-1;
	if(vice_cluster_head <0)
		vice_cluster_head = cluster_head;

	for(std::map<uint32_t,uint32_t>::iterator it = vice_cluster_heads.begin(); it != vice_cluster_heads.end(); ++it) {
		NS_LOG_INFO("node "<<GetNode()->GetId()-1<<" adding nodess "<<it->first<<" or "<<it->second<<" to condidates");
		if(main_head_incharge)
			condidates.push_back(it->first);
		else
			condidates.push_back(it->second);
	}


	NS_LOG_INFO(" node "<<GetNode()->GetId()-1<< " my cluster head is "<<cluster_head<<" and i'm cluster head "<<(GetNode()->GetId()-1 == cluster_head)<<" Condidates list ready "<<condidates.size());

	Ptr<Ipv4StaticRouting> routingProtocol;
	/*todo create socket2 if not already
	 * handle read from socket2 farward packets or sleep
	 *
	 */



	if(GetNode()->GetId()-1 == cluster_head){
//		Ptr<EnergySourceContainer> EnergySourceContainerOnNode = GetNode()->GetObject<EnergySourceContainer> ();
//		Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (EnergySourceContainerOnNode->Get(0));
//		Ptr<DeviceEnergyModel> basicRadioModelPtr = basicSourcePtr->FindDeviceEnergyModels ("ns3::WifiRadioEnergyModel").Get (0);
//		basicRadioModelPtr->SetAttribute("RxCurrentA", DoubleValue (0.0));
//		basicRadioModelPtr->SetAttribute("TxCurrentA", DoubleValue (0.0));
//		basicRadioModelPtr->SetAttribute("IdleCurrentA", DoubleValue (0.0));
//		basicRadioModelPtr->SetAttribute("CcaBusyCurrentA", DoubleValue (0.0));
//
//
//		Ptr<NetDevice> dev = GetNode()->GetDevice(0);
//		Ptr<WifiNetDevice> wifi = dev->GetObject<WifiNetDevice> ();
//		Ptr<WifiPhy> phy = wifi->GetPhy();
//		phy->SetSleepMode();
		nexthop = MacaNextHop(condidates);
	}else {
		 Ptr<Ipv4> ipv4 = ns3::NodeList::GetNode(cluster_head+1)->GetObject<Ipv4> ();
		 Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
		//Ptr<Ipv4Address> temp = ns3::NodeList::GetNode(cluster_head)->GetObject<Ipv4Address>();
		nexthop = iaddr.GetLocal();
	}
		NS_LOG_INFO(" hello10000 "<<nexthop);
      Ipv4StaticRoutingHelper Ipv4RoutingHelper;
	  Ptr<Ipv4> ipv4Node = GetNode()->GetObject<Ipv4> ();
	  routingProtocol = Ipv4RoutingHelper.GetStaticRouting( ipv4Node  );
	  //routingProtocol = GetNode()->GetObject<Ipv4StaticRouting>();
	  routingProtocol->SetDefaultRoute(nexthop,1,1);
}

Ipv4Address Need_ProSe_App::MacaNextHop(std::list<uint32_t> condidates){
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO("i'm node "<<GetNode()->GetId()-1<<" selecting best node from condidates "<<condidates.size());
	int bestNode = SelectBestNextHop(condidates);
	NS_LOG_INFO(" i'm node "<<GetNode()->GetId()-1<<" as "<<state<<" select node "<< bestNode<<" as my relay");
	Ptr<Ipv4> ipv4 = ns3::NodeList::GetNode(bestNode+1)->GetObject<Ipv4> ();
	Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);

	Ipv4Address nexthop = iaddr.GetLocal();
	double value = Calculate_pheromone_concentration(bestNode);
	previous_pheromone_concentrations[bestNode] = value;

	NS_LOG_INFO(" i'm node "<<GetNode()->GetId()-1<<" as "<<state<<" relay ip address is  "<< nexthop);
	return nexthop;
}

/* todo
 * d) update pheromone value for the sent cluster head;
 * permute cluster head and vice cluster head in main_head_list
 * initialise previous pheromonoes values
 * modify the comptetion radius to accespt main head that are within transmission
 * put them in main cluster head
 * analyse and test
 */




void Need_ProSe_App::LogStatus(void){
	NS_LOG_FUNCTION (this);
	if (my_find(neighbors_list.begin(),neighbors_list.end(),0) != neighbors_list.end()){
		main_heads_list.push_back(0);
	}
	double k = .0000001*(rand() % 100000);
	 if(state == "MainHead"){
//	 wake_up_time = .1*(rand() % 10);
//	 	if (wake_up_time == 0 || wake_up_time == 1)
	 		wake_up_time = 0.5;

			ScheduleWakeTimeBroadcast(Seconds(k));
	 		ScheduleMainViceAlternation(Seconds(k+1),true);

	 		last_round_as_cluster_head = current_round;
	 }else {
		 ResetProSeDiscovery();
	 }
	 Simulator::Schedule(Seconds(2.0), &Need_ProSe_App::LogMe, this);
	 new_round = false;

}




void Need_ProSe_App::UpdateTable(void) {
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
InputIterator Need_ProSe_App::my_find(InputIterator first, InputIterator last,
		const T& val)

		{
	NS_LOG_FUNCTION (this);
	while (first != last) {
		if (*first == val)
			return first;
		++first;
	}
	return last;
}
Need_Message  Need_ProSe_App::ParseMassage(std::string data) {

	NS_LOG_FUNCTION (this);
	std::string type ;

	NS_LOG_LOGIC(" parse data "<<data);
	std::vector<std::string> fields = split(data,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> type;

	Need_Message needMsg;
	needMsg.message_type = type;

	return needMsg;


}


double Need_ProSe_App::ParseWakeUpTime(std::string data){
	NS_LOG_FUNCTION (this);
	double wake_time;
	std::vector<std::string> fields = split(data,'|');
	std::istringstream ( split(fields.at(1),'=')[1]) >> wake_time;
	return wake_time;
}

int Need_ProSe_App::ParseViceClusterHead(std::string data){
	NS_LOG_FUNCTION (this);
	int vice_cluster_head;
	std::vector<std::string> fields = split(data,'|');
	std::istringstream ( split(fields.at(2),'=')[1]) >> vice_cluster_head;
	return vice_cluster_head;
}

double Need_ProSe_App::ParseRandTime(std::string data){
	NS_LOG_FUNCTION (this);
	double rand_time;
	std::vector<std::string> fields = split(data,'|');
	std::istringstream ( split(fields.at(2),'=')[1]) >> rand_time;
	return rand_time;
}

std::vector<std::string> Need_ProSe_App::split(const std::string& s,
		char delimiter) {
	NS_LOG_FUNCTION (this);
	   std::vector<std::string> tokens;
	   std::string token;
	   std::istringstream tokenStream(s);
	   while (std::getline(tokenStream, token, delimiter))
	   {
	      tokens.push_back(token);
	   }
	   return tokens;
}

Ptr<Socket> Need_ProSe_App::GetBroadcastSocket() {

	NS_LOG_FUNCTION (this);
	return m_socket;
}



double Need_ProSe_App::CalculateThreshold() {
	NS_LOG_FUNCTION(this);
	if(GetNode()->GetId()-1 == 0)
		return 1;
	CLUSTER_PERCENT = 0.2;
	int temp = 5;
	NS_LOG_LOGIC(" current round is "<<current_round<<" last round as cluster is "<<last_round_as_cluster_head);
	if(current_round != 0  && last_round_as_cluster_head > 0 &&  current_round-last_round_as_cluster_head < temp) return 0;
	NS_LOG_LOGIC("cluster percent is  "<<current_round % temp );
    double threshold = CLUSTER_PERCENT/(1-CLUSTER_PERCENT*(current_round % temp));
	return threshold;

}

double Need_ProSe_App::CalculateCompetitionRadius(uint32_t i) {
	NS_LOG_FUNCTION (this);
	if(i<=0)
		return 0;
	if(i>=50)
		i=49;
double dmax = CalculateDMax();
double dmin = CalculateDMin();
double m_distance = CalculateDistance(i,1);
double factorA = 0.25;
double factorB = 0.25;
double factorR0 = 100;

double residual_energy = energy_sources.at(i)->LIES.m_remaining;
double residual_energy_max = energy_sources.at(i)->LIES.GetInitialEnergy();
double competitionRadius = 0;
if(dmax>0 && dmax != dmin && residual_energy_max != 0)
competitionRadius = sqrt((1-factorA*((dmax-m_distance)/(dmax-dmin)))*(1-factorB*((residual_energy_max-residual_energy)/residual_energy_max)))*factorR0;

return competitionRadius;
}

double Need_ProSe_App::CalculateDMax(){
	NS_LOG_FUNCTION (this);
	double max = 0;

	for (int i=1; i < ns3::NodeList::GetNNodes()-1;i++){

		if (max < CalculateDistance(i,1)){
			max = CalculateDistance(i,1);
		}
	}
return max;
}

double  Need_ProSe_App::CalculateDMin(){
	NS_LOG_FUNCTION (this);
	double min = 1.7976931348623157E+300;

	for (int i=1; i < ns3::NodeList::GetNNodes()-1;i++){

		if (min > CalculateDistance(i,1)){
			min = CalculateDistance(i,1);
		}
	}
return min;

}

double Need_ProSe_App::CalculateBroadcastT(){
	NS_LOG_FUNCTION (this);
	double Tch = 3.0;
	double k = .001*(rand() % 1000);
	while(k <=0)
		k = .1*(rand() % 20);
	double broadcastT = -1;
	double average_resiual_energy = CalculateAverageEnergy();
	std::string neighbors = ListToString(neighbors_list);
	NS_LOG_INFO (" i'm node "<<GetNode()->GetId()-1<<" rnd generated is "<<k<<" neighbors "<< !neighbors_list.empty());



	double residual_energy = energy_sources.at(GetNode()->GetId()-1)->LIES.m_remaining;


	double m_distance = CalculateDistance(GetNode()->GetId()-1,0);
	double dmax = CalculateDMax();

	if(residual_energy >= average_resiual_energy){


		MainHeadMsg = Need_Message();
		MainHeadMsg.message_type = "MainClusterHead";
		MainHeadMsg.sender_id = GetNode()->GetId()-1;
	} else{
		state = "UE";
		MainHeadMsg = Need_Message();
		MainHeadMsg.message_type = "Quit";
		MainHeadMsg.sender_id = GetNode()->GetId()-1;
	}
	broadcastT =  k * Tch * (average_resiual_energy / residual_energy) * (m_distance/dmax);
	NS_LOG_INFO (GetNode()->GetId()-1<<" will broadcast++ "<<MainHeadMsg.message_type<<" after "<<broadcastT);

	NS_LOG_INFO (GetNode()->GetId()-1<<" remaining energy is "<<residual_energy);
	if(GetNode()->GetId()-1==0)
		return 2.999999;
	else
	return broadcastT;

}


std::string Need_ProSe_App::ListToString(std::list<uint32_t> myList){
	NS_LOG_FUNCTION(this);
	std::string output;
		for (uint32_t i: myList) {
			output  += ("," + i);
		}
	return output;
}

double Need_ProSe_App::CalculateAverageEnergy(){
	NS_LOG_FUNCTION (this);
	double average = 0;
	double sum = 0;

    for (EnergyModuleLte* energy_source : energy_sources) {
    	NS_LOG_LOGIC (" trying to find energy of neighbor " <<(double)energy_source->LIES.GetObject<LiIonEnergySource>()->m_remaining);
		double residual_energy = (double)energy_source->LIES.GetObject<LiIonEnergySource>()->m_remaining;
		sum += residual_energy;

    }


	average = sum / ns3::NodeList::GetNNodes();
	NS_LOG_LOGIC (" average energy remaining is  "<<average);
return average;
}

void Need_ProSe_App::BroadcastCondidateMessage(){

	  NS_LOG_LOGIC ("trying to send a message of type=condidate");
	  NS_LOG_FUNCTION (this);
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()-1<<" sent CondidateMessage" );
	  std::ostringstream oss;
	  // type condidate , s is sender id
	  oss<<"t=c|s="<<GetNode()->GetId()-1<<"|e";
	  std::string lemessage = oss.str();
	  SetProSeDiscoveryMessage(lemessage);

}


void Need_ProSe_App::BroadcastMainClusterHeadMessage(){

	  NS_LOG_LOGIC ("trying to send a message of type="<<MainHeadMsg.message_type);
	  NS_LOG_FUNCTION (this);
	  if(state != "Quit" && state !="UE"){
		  state = "MainHead";
		  //cluster_head = this->GetNode()->GetId()-1;
	  }else{
		  state = "UE";
	  }

	  std::ostringstream oss;
	  std::string lemessage;

if (MainHeadMsg.message_type == "MainClusterHead"){
		  oss<<"t=m|s="<<GetNode()->GetId()-1<<"|p="<< m_rand_time <<"|e";
		  lemessage = oss.str();
		  SetProSeDiscoveryMessage(lemessage);

	  }

	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()-1<<" is going to broadcast "+lemessage );
}

void Need_ProSe_App::BroadcastQuitMessage(){

	  NS_LOG_LOGIC ("trying to send a message of type=Quit");
	  NS_LOG_FUNCTION (this);

	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()-1<<" sent QuitMessage" );
	  SetProSeDiscoveryMessage("Quit");
}


void Need_ProSe_App::SendJoinMessage(){

	  NS_LOG_LOGIC ("trying to send a message");
	  NS_LOG_FUNCTION (this);
	  if (m_socket != 0)
	  	    {
	  NS_ASSERT (m_sendEvent.IsExpired ());
	  std::ostringstream oss;
	  oss<<"type=Join|sender_id="<<GetNode()->GetId()-1<<"|end";
	  std::string lemessage = oss.str();
	  Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()-1<<" sent " <<
			  p->GetSize() << " bytes to " <<Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
	  m_socket->Send (p);
	  ++m_sent;
	  	    }

}

void Need_ProSe_App::BroadcastWakeTimeSchedule(void){
	NS_LOG_FUNCTION (this);

	if(!new_round){
		 NS_LOG_LOGIC("we are trying to send a schedule of type=Schedule");
		 std::ostringstream oss;
		 if(vice_cluster_head<0)
			 vice_cluster_head = GetNode()->GetId()-1;
		 // type wake , wt wake up time, v vice and s is sender id
		 oss<<"t=w|wt="<<wake_up_time<<"|v="<<vice_cluster_head<<"|s="<<GetNode()->GetId()-1<<"|e";
		 std::string lemessage = oss.str();
		 NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()-1<<" sent WakeTimeSchedule with vice "<< vice_cluster_head);
		 SetProSeDiscoveryMessage(lemessage);
	}

}

int* Need_ProSe_App::getClusterHead(){
	NS_LOG_FUNCTION(this);
	return &cluster_head;
}

void Need_ProSe_App::SetProSeDiscoveryMessage(std::string message){
	NS_LOG_FUNCTION(this);

	std::string binary_adv = TextToBinaryString(message);
	NS_LOG_INFO(" here is the problem "<<GetNode()->GetId());
	if(announceApp.empty()){
		announceApp.push_back(binary_adv);
		GetNode()->GetDevice(0)->GetObject<NistLteUeNetDevice> ()->GetNas ()->AddDiscoveryApps (announceApp, true);
	}else if(announceApp.front() != binary_adv){
		GetNode()->GetDevice(0)->GetObject<NistLteUeNetDevice> ()->GetNas ()->RemoveDiscoveryApps(announceApp,true);
		announceApp.clear();
		announceApp.push_back(binary_adv);
		GetNode()->GetDevice(0)->GetObject<NistLteUeNetDevice> ()->GetNas ()->AddDiscoveryApps (announceApp, true);
	}

	NS_LOG_INFO(" out of set prose discovery message");
}


void Need_ProSe_App::ResetProSeDiscovery(){
	NS_LOG_FUNCTION(this);
	if(!announceApp.empty()){
		GetNode()->GetDevice(0)->GetObject<NistLteUeNetDevice> ()->GetNas ()->RemoveDiscoveryApps(announceApp,true);
		announceApp.clear();
	}
}

std::string Need_ProSe_App::TextToBinaryString(std::string words) {
    std::string binaryString = "";
    for (char& _char : words) {
        binaryString += std::bitset<8>(_char).to_string();
    }
    return binaryString;
}

/*std::string Need_ProSe_App::BinaryToString(std::string binary){

}*/

void Need_ProSe_App::SetupProSeDiscoveryCallback(){
	NS_LOG_FUNCTION(this);
	 /*AsciiTraceHelper ascii;
	 Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("discovery_out_monitoring.tr");
	 *stream->GetStream () << "Time\tIMSI\tCellId\tRNTI\tProSeAppCode" << std::endl;
	 //TracedCallback<uint64_t, uint16_t, uint16_t, std::string>
	 Ptr<NistLteUeRrc> ueRrc = DynamicCast<NistLteUeRrc>( GetNode()->GetDevice(0)->GetObject<NistLteUeNetDevice>()->GetRrc () );
	 ueRrc->TraceConnectWithoutContext ("DiscoveryMonitoring", MakeCallback (&Need_ProSe_App::DiscoveryMonitoringCallback));*/
}

void Need_ProSe_App::DiscoveryMonitoringCallback ( uint16_t sender, std::string proSeAppCode)
	  {
		NS_LOG_FUNCTION(this);

	  	std::stringstream input_ss;
	  	std::stringstream output_ss;
	  	input_ss.str(proSeAppCode);
	  	 std::bitset<8> bs;
	  	    while(input_ss >> bs)
	  	    	output_ss << char(bs.to_ulong());

	  	  NS_LOG_INFO(Simulator::Now ().GetSeconds () << "\t node " << sender << "\t sent to node "
	  			  <<GetNode()->GetId()-1<<"\t discovery message with appcode " << output_ss.str() );
	  	handleDiscoveryMessage(sender,output_ss.str());
	  }

void
Need_ProSe_App::SetSource (Ptr<LiIonEnergySource> source)
{
	NS_LOG_FUNCTION (this);
m_source = source;
}





double Need_ProSe_App::Calculate_pheromone_concentration(int NodeJ){
	NS_LOG_FUNCTION (this);

	double previous = previous_pheromone_concentrations[NodeJ];
	NS_LOG_INFO(" Previous Maca pheromone concentration for node "<<NodeJ<<" is "<<previous);
	double nodeJ_residual_energy = energy_sources.at(NodeJ)->LIES.m_remaining;
	double NodeJ_consumed_energy =energy_sources.at(NodeJ)->LIES.GetInitialEnergy() - energy_sources.at(NodeJ)->LIES.m_remaining;

	double value = (1-ro)*previous - NodeJ_consumed_energy;
	//previous_pheromone_concentrations[NodeJ] = value;
	NS_LOG_INFO(" Maca pheromone concentration for node "<<NodeJ<<" is "<<value);
	return value;
}

double Need_ProSe_App::Calculate_Maca_Heuristic(int NodeJ){
	NS_LOG_FUNCTION (this);
	if(NodeJ == 0)
		return 10000;
	double i_distance = CalculateDistance(GetNode()->GetId()-1,0);
	if(GetNode()->GetId()-1 == 0)
		i_distance = 10000;
	double j_distance = CalculateDistance(NodeJ,1);
	double i_j_distance = CalculateDistance(GetNode()->GetId()-1,NodeJ);
	double h = (i_distance*i_distance)/(j_distance*j_distance + i_j_distance*i_j_distance);


	NS_LOG_INFO(" Maca heuristic for node "<<NodeJ<<" is "<<h);
	return h;


}

double Need_ProSe_App::Calculate_Relay_Probability(int NodeJ,std::list<uint32_t> condidates){
	NS_LOG_FUNCTION (this);
		double p = (pow(Calculate_pheromone_concentration(NodeJ),alpha)) * (pow(Calculate_Maca_Heuristic(NodeJ),beta));
		double sum = 0;
		for(int m : condidates){
			double temp = (pow(Calculate_pheromone_concentration(m),alpha)) * (pow(Calculate_Maca_Heuristic(m),beta));
			sum += temp;
		}
		NS_LOG_INFO("sum for node "<<NodeJ<<" is "<<sum);
		if(sum != 0)
		p = p/sum;
		else p = 0;
		return p;

}

int Need_ProSe_App::SelectBestNextHop(std::list<uint32_t> condidates){
	NS_LOG_FUNCTION (this);
	int bestChoice = -1;
	double bestProbability = 0;
	NS_LOG_INFO(" i'm main head and i have main heads neighbors "<< !main_heads_list.empty());
	for(int j : condidates){
		if(j == 0)
			return j;
		NS_LOG_INFO(" calculating probability for node "<<j);
		double p = Calculate_Relay_Probability(j,condidates);
		NS_LOG_INFO(" probability for node "<<j<<" is "<<p);
		if (p>bestProbability)
			bestChoice = j;
	}

	if(bestChoice<0)
		bestChoice = cluster_head;
	return bestChoice;

}




//void Need_ProSe_App::SendJoinMessageToCoveredNodes(
//}

} // Namespace ns3

