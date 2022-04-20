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
#include "need-wifi-app.h"
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




namespace ns3 {

NS_LOG_COMPONENT_DEFINE("need-wifi-app");

NS_OBJECT_ENSURE_REGISTERED(Need_WiFi_App);

TypeId Need_WiFi_App::GetTypeId(void) {
	static TypeId tid =
			TypeId("ns3::Need_WiFi_App").SetParent<Application>().SetGroupName(
					"Applications").AddConstructor<Need_WiFi_App>().AddAttribute(
					"MaxPackets",
					"The maximum number of packets the application will send",
					UintegerValue(100),
					MakeUintegerAccessor(&Need_WiFi_App::m_count),
					MakeUintegerChecker<uint32_t>()).AddAttribute("if_index",
					"The maximum number of packets the application will send",
					UintegerValue(0),
					MakeUintegerAccessor(&Need_WiFi_App::if_index),
					MakeUintegerChecker<uint32_t>()).AddAttribute("Interval",
					"The time to wait between packets", TimeValue(Seconds(1.0)),
					MakeTimeAccessor(&Need_WiFi_App::m_interval),
					MakeTimeChecker()).AddAttribute("RemoteAddress",
					"The destination Address of the outbound packets",
					AddressValue(),
					MakeAddressAccessor(&Need_WiFi_App::m_peerAddress),
					MakeAddressChecker()).AddAttribute("RemotePort",
					"The destination port of the outbound packets",
					UintegerValue(0),
					MakeUintegerAccessor(&Need_WiFi_App::m_peerPort),
					MakeUintegerChecker<uint16_t>()).AddAttribute("PacketSize",
					"Size of echo data in outbound packets", UintegerValue(100),
					MakeUintegerAccessor(&Need_WiFi_App::SetDataSize,
							&Need_WiFi_App::GetDataSize),
					MakeUintegerChecker<uint32_t>()).AddTraceSource("Tx",
					"A new packet is created and is sent",
					MakeTraceSourceAccessor(&Need_WiFi_App::m_txTrace));
	return tid;
}

Need_WiFi_App::Need_WiFi_App() {
	NS_LOG_FUNCTION(this);
	m_sent = 0;
	m_socket = 0;
	m_socket2 = 0;
	m_sendEvent = EventId();
	m_data = 0;
	m_dataSize = 0;
}

Need_WiFi_App::~Need_WiFi_App() {
	NS_LOG_FUNCTION(this);
	m_socket = 0;

	delete[] m_data;
	m_data = 0;
	m_dataSize = 0;
}

void Need_WiFi_App::SetRemote(Address ip, uint16_t port) {
	NS_LOG_FUNCTION(this << ip << port);
	m_peerAddress = ip;
	m_peerPort = port;
}

void Need_WiFi_App::SetRemote(Address addr) {
	NS_LOG_FUNCTION(this << addr);
	m_peerAddress = addr;
}

void Need_WiFi_App::DoDispose(void) {
	NS_LOG_FUNCTION(this);
	Application::DoDispose();
}

void Need_WiFi_App::StartApplication(void) {
	NS_LOG_FUNCTION(this);

		  	//m_socket->SetRecvCallback(MakeCallback(&Need_WiFi_App::HandleRead, this));

		  	SetupOutSocket();
		  	SetupInSocket();
		  	m_socket->SetAllowBroadcast(true);
		  	m_socket2->SetRecvCallback(
		  			MakeCallback(&Need_WiFi_App::HandleRead, this));
		  	m_socket2->SetAllowBroadcast(true);

		  	//ScheduleTransmit(Seconds(0.));
		  	// ScheduleRequest (Seconds (0.1));
		  	ScheduleRound(Seconds (0.1));
}

void Need_WiFi_App::SetupOutSocket(){
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


void Need_WiFi_App::SetupInSocket(){
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

}

void Need_WiFi_App::StopApplication() {
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

void Need_WiFi_App::SetDataSize(uint32_t dataSize) {
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

uint32_t Need_WiFi_App::GetDataSize(void) const {
	NS_LOG_FUNCTION(this);
	return m_size;
}



void Need_WiFi_App::SetRandomVariableGenerator(
		Ptr<UniformRandomVariable> gen) {
	NS_LOG_FUNCTION(this);

	m_gen = gen;
}

void Need_WiFi_App::ScheduleTransmit(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_sendEvent = Simulator::Schedule(dt, &Need_WiFi_App::Send, this);

}



void Need_WiFi_App::ScheduleRequest(Time dt) {
	NS_LOG_FUNCTION(this << dt);


}

void Need_WiFi_App::ScheduleRound(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_RoundEvent = Simulator::Schedule(dt, &Need_WiFi_App::initiate_round, this);
	NS_LOG_LOGIC(" next round is scheduled after "<<dt);


}

void Need_WiFi_App::ScheduleBroadcastTimeCalculation(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_BTCalculationEvent = Simulator::Schedule(dt, &Need_WiFi_App::CalculateBroadcastTime, this);

}

void Need_WiFi_App::ScheduleMainBroadcast(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	m_MainBroadcastEvent = Simulator::Schedule(dt, &Need_WiFi_App::BroadcastMainClusterHeadMessage, this);

}

void Need_WiFi_App::ScheduleWakeTimeBroadcast(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Need_WiFi_App::BroadcastWakeTimeSchedule, this);

}

void Need_WiFi_App::ScheduleMainViceAlternation(Time dt,bool mainHead) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Need_WiFi_App::Alternate, this);

}

void Need_WiFi_App::ScheduleCondidateMessageBroadcast(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Need_WiFi_App::BroadcastCondidateMessage, this);

}






void Need_WiFi_App::initiate_round(){
	NS_LOG_FUNCTION(this);
	double random_number = .00001*(rand() % 100000);
	double threshold = CalculateThreshold();
	NS_LOG_INFO(" calculated threshold is  "<<threshold<<" ,generated random number is "<<random_number);
	initilizeVaribales();
    if(random_number < threshold){
    	double rnd = .00001*(rand() % 100000);
    	ScheduleCondidateMessageBroadcast(Seconds(rnd));
    	state = "condidate";

    }
    ScheduleRound(Seconds (10.0));
    ScheduleBroadcastTimeCalculation(Seconds (2.0));
    ScheduleCheck(Seconds (5.9));




}

void Need_WiFi_App::CalculateBroadcastTime(){
	NS_LOG_FUNCTION (this);

	 if (state == "condidate")
		state = "TempMainHead";
	else
		state = "UE";
	double broadcastTime = CalculateBroadcastT();
	if(GetNode()->GetId() == 0)
		state = "MainHead";
	if(broadcastTime<0){
		broadcastTime = 1.1;
	}
		ScheduleMainBroadcast(Seconds(broadcastTime));
		NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << " node "<<GetNode()->GetId()<<" will broadcast* "<< state<<" after  "<<broadcastTime);


}

void Need_WiFi_App::initilizeVaribales(){
	main_heads_list.clear();
	neighbors_list.clear();
	vice_cluster_heads.clear();
	new_round = true;
	current_round++;
	state = "UE";
	cluster_head = -1;
	vice_cluster_head = -1;
}

void Need_WiFi_App::Send(void) {
	NS_LOG_FUNCTION(this);

	NS_ASSERT(m_sendEvent.IsExpired());



}

void Need_WiFi_App::HandleRead(Ptr<Socket> socket) {
	NS_LOG_FUNCTION(this << socket);
	 if(m_socket != 0){

	  Ptr<Packet> packet;
	  Address from;
	  std::string data;
	    MyHeader currenthops ;
	  while ((packet = socket->RecvFrom (from)))
	    {
	      if (InetSocketAddress::IsMatchingType (from))
	        {
	          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server "<<GetNode()->GetId()<<" received " << packet->GetSize () << " bytes from " <<
	                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
	                       InetSocketAddress::ConvertFrom (from).GetPort ());

	          packet->RemoveHeader (currenthops);

	                     uint8_t *buffer = new uint8_t[packet->GetSize()];
	                                       		packet->CopyData (buffer, packet->GetSize());
	                                       		data = std::string((char*)buffer);
	        }
	    }
	  Need_Message msg = ParseMassage(data);
	  NS_LOG_INFO ("At time "<< Simulator::Now ().GetSeconds () <<" Node "<<GetNode()->GetId()<<" received message of type "<<msg.message_type<<" from node "<<msg.sender_id);

	  if(msg.message_type == "condidate")
	  if (my_find(neighbors_list.begin(),neighbors_list.end(),msg.sender_id) == neighbors_list.end()){
		  double distance = CalculateDistance(GetNode()->GetId(), msg.sender_id);
		  if(distance < std::max(CalculateCompetitionRadius(GetNode()->GetId()),CalculateCompetitionRadius(msg.sender_id))){
			  neighbors_list.push_back(msg.sender_id);
			  NS_LOG_INFO ("At time "<< Simulator::Now ().GetSeconds () <<" Node "<<GetNode()->GetId()<<" going to add node "<<msg.sender_id<<" to it's neighbors");
		  }else {
			  NS_LOG_INFO ("At time "<< Simulator::Now ().GetSeconds () <<" Node "<<GetNode()->GetId()<<" is not going to add node "<<msg.sender_id<<" to it's neighbors");
		  }

	  }


	  if(msg.message_type == "MainClusterHead"){
		  if (my_find(neighbors_list.begin(),neighbors_list.end(),msg.sender_id) != neighbors_list.end()){
			  NS_LOG_INFO (" Node "<<GetNode()->GetId()<<" going to compare with node "<<msg.sender_id);
		  if(!m_MainBroadcastEvent.IsExpired()){
			  NS_LOG_INFO (" and i lost ");
				MainHeadMsg.message_type = "Quit";
				if(cluster_head<0){
					state = "Quit";
					cluster_head = msg.sender_id;
				}


		  }

		  }

	  }

	  if(msg.message_type == "Quit"){

				if(cluster_head == this->GetNode()->GetId() && vice_cluster_head<0){
					if(GetNode()->GetId() == 0)
						vice_cluster_head = 0;
					else {
						vice_cluster_head = msg.sender_id;
						previous_pheromone_concentrations[vice_cluster_head] = 22;
					}

				}




	  }
	  if(msg.message_type == "Schedule"){
		  NS_LOG_INFO ("Node "<<GetNode()->GetId()<<" Main head "<<msg.sender_id<<" wake up time state is "<<state);
		  int temp_vice_head = -1;
		  temp_vice_head = ParseViceClusterHead(data);
		  if(temp_vice_head<0){
		  			  				NS_LOG_INFO("they are sending -1 vice ");
		  			  			if(GetNode()->GetId() == 0)
		  			  				temp_vice_head = 0;
		  			  			else
		  			  				temp_vice_head = msg.sender_id;
		  				  }

		  if(cluster_head<0)
				cluster_head = msg.sender_id;
		  if(state != "MainHead"){
			  	  if(vice_cluster_head<0){
		  		  wake_up_time = ParseWakeUpTime(data);
		  		  vice_cluster_head = temp_vice_head;
			  	  }

		  		//NS_LOG_INFO ("Main head "<<msg.sender_id<<" wake up time is "<<wake_up_time);

		  	  }

		  				  if (my_find(main_heads_list.begin(),main_heads_list.end(),msg.sender_id) == main_heads_list.end()){
		  					  main_heads_list.push_back(msg.sender_id);
		  				  	  previous_pheromone_concentrations[msg.sender_id] = 22;

		  				  	NS_LOG_INFO ("adding node "<<msg.sender_id<<" to main heads list");
		  				  }

		  vice_cluster_heads[msg.sender_id] = temp_vice_head;
		  ScheduleMainViceAlternation(Seconds(1.0),true);
	  }
	 }

	 NS_LOG_INFO ("out of handle read");

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

double Need_WiFi_App::CalculateDistance(int nodeI,int  nodeJ){
	NS_LOG_FUNCTION(this);
	Ptr<MobilityModel> i_mob = ns3::NodeList::GetNode(nodeI)->GetObject<MobilityModel>();
	Ptr<MobilityModel> j_mob = ns3::NodeList::GetNode(nodeJ)->GetObject<MobilityModel>();
	double distance = i_mob->GetDistanceFrom(j_mob);
	return distance;
}
void Need_WiFi_App::ScheduleCheck(Time dt) {
	NS_LOG_FUNCTION(this << dt);
	Simulator::Schedule(dt, &Need_WiFi_App::LogStatus, this);
}

void Need_WiFi_App::updatestate() {
	NS_LOG_FUNCTION (this);
}


bool Need_WiFi_App::contains(std::list<uint32_t> list1, uint32_t nodeId) {
	NS_LOG_FUNCTION (this);
	for (uint32_t i : list1) {
		NS_LOG_INFO ("i'm node : " <<GetNode()->GetId()<<" my neighbors cluster heads are : " <<i);
		if(i == nodeId)
			return true;
	}
	return false;

}


void Need_WiFi_App::LogMe(){
	if(cluster_head < 0){
			cluster_head = GetNode()->GetId();

		}
	if(vice_cluster_head <0)
		vice_cluster_head = cluster_head;
	if(GetNode()->GetId() == 0){
		cluster_head = 0;
		vice_cluster_head = cluster_head;
	}

	NS_LOG_INFO ("At time : " << Simulator::Now ().GetSeconds () <<" Node "<<GetNode()->GetId()<<"status is "<<this->state);
	NS_LOG_INFO ("my cluster head is  " << cluster_head <<" and vice cluster head is  "<< vice_cluster_head);
	InitiateDuties();

}

void Need_WiFi_App::Alternate(){

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

void Need_WiFi_App::InitiateDuties(){
	NS_LOG_FUNCTION (this);
	Ipv4Address nexthop ;
	std::list<uint32_t> condidates;
	if(cluster_head < 0 )
		cluster_head = GetNode()->GetId();
	if(vice_cluster_head <0)
		vice_cluster_head = cluster_head;

	for(std::map<uint32_t,uint32_t>::iterator it = vice_cluster_heads.begin(); it != vice_cluster_heads.end(); ++it) {
		NS_LOG_INFO("node "<<GetNode()->GetId()<<" adding nodess "<<it->first<<" or "<<it->second<<" to condidates");
		if(main_head_incharge)
			condidates.push_back(it->first);
		else
			condidates.push_back(it->second);
	}


	NS_LOG_INFO("Condidates list ready "<<condidates.size());

	Ptr<Ipv4StaticRouting> routingProtocol;
	/*todo create socket2 if not already
	 * handle read from socket2 farward packets or sleep
	 *
	 */

	if(GetNode()->GetId() == cluster_head){
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
		 Ptr<Ipv4> ipv4 = ns3::NodeList::GetNode(cluster_head)->GetObject<Ipv4> ();
		 Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);
		//Ptr<Ipv4Address> temp = ns3::NodeList::GetNode(cluster_head)->GetObject<Ipv4Address>();
		nexthop = iaddr.GetLocal();
	}
		NS_LOG_INFO(" hello10000 ");
      Ipv4StaticRoutingHelper Ipv4RoutingHelper;
	  Ptr<Ipv4> ipv4Node = GetNode()->GetObject<Ipv4> ();
	  routingProtocol = Ipv4RoutingHelper.GetStaticRouting( ipv4Node  );
	  //routingProtocol = GetNode()->GetObject<Ipv4StaticRouting>();
	  routingProtocol->SetDefaultRoute(nexthop,1,1);
}

Ipv4Address Need_WiFi_App::MacaNextHop(std::list<uint32_t> condidates){
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO("i'm node "<<GetNode()->GetId()<<" selecting best node from condidates "<<condidates.size());
	int bestNode = SelectBestNextHop(condidates);
	NS_LOG_INFO(" i'm node "<<GetNode()->GetId()<<" as "<<state<<" select node "<< bestNode<<" as my relay");
	Ptr<Ipv4> ipv4 = ns3::NodeList::GetNode(bestNode)->GetObject<Ipv4> ();
	Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1,0);

	Ipv4Address nexthop = iaddr.GetLocal();
	double value = Calculate_pheromone_concentration(bestNode);
	previous_pheromone_concentrations[bestNode] = value;
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




void Need_WiFi_App::LogStatus(void){
	NS_LOG_FUNCTION (this);
	double k = .0000001*(rand() % 100000);
	 if(state == "MainHead"){
//	 wake_up_time = .1*(rand() % 10);
//	 	if (wake_up_time == 0 || wake_up_time == 1)
	 		wake_up_time = 0.5;

			ScheduleWakeTimeBroadcast(Seconds(k));
	 		ScheduleMainViceAlternation(Seconds(k+1),true);

	 		last_round_as_cluster_head = current_round;
	 }
	 Simulator::Schedule(Seconds(2*k), &Need_WiFi_App::LogMe, this);
	 new_round = false;

}




void Need_WiFi_App::UpdateTable(void) {
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
InputIterator Need_WiFi_App::my_find(InputIterator first, InputIterator last,
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
Need_Message  Need_WiFi_App::ParseMassage(std::string data) {

	NS_LOG_FUNCTION (this);
	std::string type ;
	int sender_id;


	NS_LOG_LOGIC(" parse data "<<data);
	std::vector<std::string> fields = split(data,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> type;
	std::istringstream ( split(fields.at(1),'=')[1]) >> sender_id;
	Need_Message needMsg;
	needMsg.message_type = type;
	needMsg.sender_id = sender_id;

	return needMsg;


}


double Need_WiFi_App::ParseWakeUpTime(std::string data){
	NS_LOG_FUNCTION (this);
	double wake_time;
	std::vector<std::string> fields = split(data,'|');
	std::istringstream ( split(fields.at(2),'=')[1]) >> wake_time;
	return wake_time;
}

int Need_WiFi_App::ParseViceClusterHead(std::string data){
	NS_LOG_FUNCTION (this);
	int vice_cluster_head;
	std::vector<std::string> fields = split(data,'|');
	std::istringstream ( split(fields.at(3),'=')[1]) >> vice_cluster_head;
	return vice_cluster_head;
}

std::vector<std::string> Need_WiFi_App::split(const std::string& s,
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

Ptr<Socket> Need_WiFi_App::GetBroadcastSocket() {
	NS_LOG_FUNCTION (this);
	return m_socket;
}



double Need_WiFi_App::CalculateThreshold() {
	NS_LOG_FUNCTION(this);
	if(GetNode()->GetId() == 0)
		return 1;
	CLUSTER_PERCENT = 0.2;
	int temp = 5;
	NS_LOG_LOGIC(" current round is "<<current_round<<" last round as cluster is "<<last_round_as_cluster_head);
	if(current_round != 0  && last_round_as_cluster_head > 0 &&  current_round-last_round_as_cluster_head < temp) return 0;
	NS_LOG_LOGIC(" cluster percent is  "<<current_round % temp );
    double threshold = CLUSTER_PERCENT/(1-CLUSTER_PERCENT*(current_round % temp));
	return threshold;

}

double Need_WiFi_App::CalculateCompetitionRadius(uint32_t i) {
	NS_LOG_FUNCTION (this);
	if(i==0)
		return 0;
double dmax = CalculateDMax();
double dmin = CalculateDMin();
double m_distance = CalculateDistance(i,0);
double factorA = 0.25;
double factorB = 0.25;
double factorR0 = 40;
Ptr<EnergySourceContainer> EnergySourceContainerOnNode = ns3::NodeList::GetNode(i)->GetObject<EnergySourceContainer> ();
Ptr<LiIonEnergySource> energysource = DynamicCast<LiIonEnergySource> (EnergySourceContainerOnNode->Get(0));
double residual_energy = energysource->GetRemainingEnergy();
double residual_energy_max = energysource->GetInitialEnergy();
double competitionRadius = 0;
if(dmax>0 && dmax != dmin && residual_energy_max != 0)
competitionRadius = sqrt((1-factorA*((dmax-m_distance)/(dmax-dmin)))*(1-factorB*((residual_energy_max-residual_energy)/residual_energy_max)))*factorR0;

return competitionRadius;
}

double Need_WiFi_App::CalculateDMax(){
	NS_LOG_FUNCTION (this);
	double max = 0;

	for (int i=1; i < ns3::NodeList::GetNNodes()-1;i++){

		if (max < CalculateDistance(i,0)){
			max = CalculateDistance(i,0);
		}
	}
return max;
}

double  Need_WiFi_App::CalculateDMin(){
	NS_LOG_FUNCTION (this);
	double min = 1.7976931348623157E+300;

	for (int i=1; i < ns3::NodeList::GetNNodes()-1;i++){

		if (min > CalculateDistance(i,0)){
			min = CalculateDistance(i,0);
		}
	}
return min;

}

double Need_WiFi_App::CalculateBroadcastT(){
	NS_LOG_FUNCTION (this);
	double Tch = 3.0;
	double k = .001*(rand() % 1000);
	double broadcastT = -1;
	double average_resiual_energy = CalculateAverageEnergy();
	Ptr<EnergySourceContainer> EnergySourceContainerOnNode = this->GetNode()->GetObject<EnergySourceContainer> ();
	std::string neighbors = ListToString(neighbors_list);
	NS_LOG_INFO (" i'm node "<<GetNode()->GetId()<<" rnd generated is "<<k<<" neighbors "<< !neighbors_list.empty());
	Ptr<LiIonEnergySource> energysource = DynamicCast<LiIonEnergySource> (EnergySourceContainerOnNode->Get(0));


	double residual_energy = energysource->GetRemainingEnergy();


	double m_distance = CalculateDistance(GetNode()->GetId(),0);
	double dmax = CalculateDMax();

	if(residual_energy >= average_resiual_energy){


		MainHeadMsg = Need_Message();
		MainHeadMsg.message_type = "MainClusterHead";
		MainHeadMsg.sender_id = GetNode()->GetId();
	} else{
		state = "UE";
		MainHeadMsg = Need_Message();
		MainHeadMsg.message_type = "Quit";
		MainHeadMsg.sender_id = GetNode()->GetId();
	}
	NS_LOG_INFO (GetNode()->GetId()<<" will broadcast++ "<<MainHeadMsg.message_type);
	broadcastT = k * Tch * (average_resiual_energy / residual_energy) * (m_distance/dmax);
	if(GetNode()->GetId()==0)
		return 2.999999;
	else
	return broadcastT;

}


std::string Need_WiFi_App::ListToString(std::list<uint32_t> myList){
	std::string output;
		for (uint32_t i: myList) {
			output  += ("," + i);
		}
	return output;
}

double Need_WiFi_App::CalculateAverageEnergy(){
	NS_LOG_FUNCTION (this);
	double average = 0;
	double sum = 0;
    for (int neighbor_id = 0; neighbor_id < ns3::NodeList::GetNNodes(); neighbor_id++) {
    	Ptr<EnergySourceContainer> EnergySourceContainerOnNode = ns3::NodeList::GetNode(neighbor_id)->GetObject<EnergySourceContainer> ();
		Ptr<LiIonEnergySource> energysource = DynamicCast<LiIonEnergySource> (EnergySourceContainerOnNode->Get(0));
		double residual_energy = energysource->GetRemainingEnergy();
		sum += residual_energy;

    }
    NS_LOG_LOGIC (" trying to find energy of neighbor "<<neighbors_list.empty());

	average = sum / ns3::NodeList::GetNNodes();
return average;
}

void Need_WiFi_App::BroadcastCondidateMessage(){

	  NS_LOG_LOGIC ("trying to send a message of type=condidate");
	  NS_LOG_FUNCTION (this);
	  if (m_socket != 0)
	  	    {
	  NS_ASSERT (m_sendEvent.IsExpired ());
	  std::ostringstream oss;
	  oss<<"type=condidate|sender_id="<<GetNode()->GetId()<<"|end";
	  std::string lemessage = oss.str();
	  Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()<<" sent CondidateMessage" );
	  m_socket->Send (p);
	  ++m_sent;
	  	    }

}


void Need_WiFi_App::BroadcastMainClusterHeadMessage(){

	  NS_LOG_LOGIC ("trying to send a message of type="<<MainHeadMsg.message_type);
	  NS_LOG_FUNCTION (this);
	  if (m_socket != 0)
	  	    {
	  NS_ASSERT (m_sendEvent.IsExpired ());
	  if(state != "Quit" && state !="UE"){
		  state = "MainHead";
		  cluster_head = this->GetNode()->GetId();
	  }else{
		  state = "UE";
	  }
	  std::ostringstream oss;
	  oss<<"type="<<MainHeadMsg.message_type<<"|sender_id="<<GetNode()->GetId()<<"|end";
	  std::string lemessage = oss.str();
	  Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()<<" sent MainClusterHeadMessage" );
	  m_socket->Send (p);
	  ++m_sent;
	  	    }

}

void Need_WiFi_App::BroadcastQuitMessage(){

	  NS_LOG_LOGIC ("trying to send a message of type=Quit");
	  NS_LOG_FUNCTION (this);
	  if (m_socket != 0)
	  	    {
	  NS_ASSERT (m_sendEvent.IsExpired ());
	  std::ostringstream oss;
	  oss<<"type=Quit|sender_id="<<GetNode()->GetId()<<"|end";
	  std::string lemessage = oss.str();
	  Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()<<" sent QuitMessage" );
	  m_socket->Send (p);
	  ++m_sent;
	  	    }

}


void Need_WiFi_App::SendJoinMessage(){

	  NS_LOG_LOGIC ("trying to send a message");
	  NS_LOG_FUNCTION (this);
	  if (m_socket != 0)
	  	    {
	  NS_ASSERT (m_sendEvent.IsExpired ());
	  std::ostringstream oss;
	  oss<<"type=Join|sender_id="<<GetNode()->GetId()<<"|end";
	  std::string lemessage = oss.str();
	  Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
	  std::cout<<"At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()<<" sent " << p->GetSize() << " bytes to " <<
	 	         Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort<<std::endl;
	  m_socket->Send (p);
	  ++m_sent;
	  	    }

}

void Need_WiFi_App::BroadcastWakeTimeSchedule(void){
	NS_LOG_FUNCTION (this);

	if(!new_round){
		 if (m_socket != 0)
			  	    {
			 	 	 NS_LOG_LOGIC("we are trying to send a schedule of type=Schedule");
			 	 	 std::ostringstream oss;
			 	 	 oss<<"type=Schedule|sender_id="<<GetNode()->GetId()<<"|wake_time="<<wake_up_time<<"|vice_cluster_head="<<vice_cluster_head<<"|end";
			 	 	 std::string lemessage = oss.str();
			 	 	 Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
			 	 	 m_socket->Send (p);
			 	 	 NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()<<" sent WakeTimeSchedule with vice "<< vice_cluster_head);
			  	    }

	}

}

int* Need_WiFi_App::getClusterHead(){
	return &cluster_head;
}


void
Need_WiFi_App::SetSource (Ptr<LiIonEnergySource> source)
{
	NS_LOG_FUNCTION (this);
m_source = source;
}

double Need_WiFi_App::Calculate_pheromone_concentration(int NodeJ){
	NS_LOG_FUNCTION (this);

	double previous = previous_pheromone_concentrations[NodeJ];
	NS_LOG_INFO(" Previous Maca pheromone concentration for node "<<NodeJ<<" is "<<previous);
	Ptr<EnergySourceContainer> EnergySourceContainerOnNode = ns3::NodeList::GetNode(NodeJ)->GetObject<EnergySourceContainer> ();
	Ptr<LiIonEnergySource> energysource = DynamicCast<LiIonEnergySource> (EnergySourceContainerOnNode->Get(0));
	double NodeJ_consumed_energy =energysource->GetInitialEnergy() - energysource->GetRemainingEnergy();

	double value = (1-ro)*previous - NodeJ_consumed_energy;
	//previous_pheromone_concentrations[NodeJ] = value;
	NS_LOG_INFO(" Maca pheromone concentration for node "<<NodeJ<<" is "<<value);
	return value;
}

double Need_WiFi_App::Calculate_Maca_Heuristic(int NodeJ){
	NS_LOG_FUNCTION (this);
	if(NodeJ == 0)
		return 10000;
	double i_distance = CalculateDistance(GetNode()->GetId(),0);
	if(GetNode()->GetId() == 0)
		i_distance = 10000;
	double j_distance = CalculateDistance(NodeJ,0);
	double i_j_distance = CalculateDistance(GetNode()->GetId(),NodeJ);
	double h = (i_distance*i_distance)/(j_distance*j_distance + i_j_distance*i_j_distance);


	NS_LOG_INFO(" Maca heuristic for node "<<NodeJ<<" is "<<h);
	return h;


}

double Need_WiFi_App::Calculate_Relay_Probability(int NodeJ,std::list<uint32_t> condidates){
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

int Need_WiFi_App::SelectBestNextHop(std::list<uint32_t> condidates){
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




//void Need_WiFi_App::SendJoinMessageToCoveredNodes(
//}

} // Namespace ns3

