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
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/neighbor.h"
#include "lte-echo-server.h"
#include <ns3/nist-lte-helper.h>
#include <ns3/nist-module.h>

#include <cstdint>
#include <list>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "ns3/core-module.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("lte_echo_server");

NS_OBJECT_ENSURE_REGISTERED (lte_echo_server);

TypeId
lte_echo_server::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::lte_echo_server")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<lte_echo_server> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&lte_echo_server::m_port),
                   MakeUintegerChecker<uint16_t> ())
                   .AddAttribute ("if_index",
                                      "The maximum number of packets the application will send",
                                      UintegerValue (0),
                                      MakeUintegerAccessor (&lte_echo_server::if_index),
                                       MakeUintegerChecker<uint32_t> ())
				   .AddTraceSource ("n_hops",
							   "A State value to trace.",
							   MakeTraceSourceAccessor (&lte_echo_server::n_hops))
				   .AddTraceSource ("n_delivered",
							   "A State value to trace.",
							   MakeTraceSourceAccessor (&lte_echo_server::n_delivered))

  ;
  return tid;
}

lte_echo_server::lte_echo_server ()
{
  NS_LOG_FUNCTION (this);
}

lte_echo_server::~lte_echo_server()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket2 = 0;
  m_socket6 = 0;
}

void
lte_echo_server::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}
void lte_echo_server::SetHelper(Ptr<NistLteProseHelper> Helper){
	m_proseHelper = Helper;
}

void lte_echo_server::RssiCallback(uint16_t rnti, double rssi){
	NS_LOG_FUNCTION(this);
	std::cout<<"00000000000 node "<<GetNode()->GetId()<<" received packet from "<<rnti<<" with rssi "<<rssi<<std::endl;;

}

void 
lte_echo_server::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);
      m_socket->BindToNetDevice(GetNode()->GetDevice(if_index));
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
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

  if (m_socket2 == 0)
      {
        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
        m_socket2 = Socket::CreateSocket (GetNode (), tid);
        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 999);
        m_socket2->Bind (local);
        m_socket2->BindToNetDevice(GetNode()->GetDevice(if_index));



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



  m_socket->SetRecvCallback (MakeCallback (&lte_echo_server::HandleRead, this));
  m_socket2->SetRecvCallback (MakeCallback (&lte_echo_server::HandleRead2, this));
  ScheduleCheck(Seconds (1.0));
  //m_socket6->SetRecvCallback (MakeCallback (&lte_echo_server::HandleRead, this));
 /* Ptr<NistLteSpectrumPhy> ltePhy = GetNode()->GetObject<NistLteSpectrumPhy>();
  Ptr<NetDevice> dev = GetNode()->GetDevice(0);
  Ptr<NistLteUeNetDevice> ueDev = DynamicCast<NistLteUeNetDevice>(dev);
  Ptr<NistLteUePhy> uePhy = ueDev->GetPhy();
  Ptr<NistLteSpectrumPhy> ltePhy2 = uePhy->GetSlSpectrumPhy();
  if(ltePhy)
  ltePhy->SetNistLteRSSICallback(MakeCallback (&lte_echo_server::RssiCallback, this));

	 std::cout<<" i'm set and everything is good  "<<ltePhy2->GetReferenceCount()<<std::endl;;
  if(ltePhy2)
	  ltePhy->SetNistLteRSSICallback(MakeCallback (&lte_echo_server::RssiCallback, this));*/
}

void 
lte_echo_server::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0) 
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void 
lte_echo_server::HandleRead2 (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
   Address from;
   std::string data;
   MyHeader currenthops ;
   while ((packet = socket->RecvFrom (from)))
      {
        if (InetSocketAddress::IsMatchingType (from))
          {
            NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client  " << GetNode()->GetId() <<" received " << packet->GetSize () << " bytes from " <<
                         InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                         InetSocketAddress::ConvertFrom (from).GetPort ());

          		packet->RemoveHeader (currenthops);

            uint8_t *buffer = new uint8_t[packet->GetSize()];
                              		packet->CopyData (buffer, packet->GetSize());
                              		data = std::string((char*)buffer);
          }

      }

   std::string type = ParseType(data);
    	  if(type == "request"){

     		Ptr<Packet> packet_temp = Create<Packet> ((uint8_t*) data.c_str(), data.length());

     		MyHeader newheader;
     		newheader.SetData(currenthops.GetData()+1);
     		packet_temp->AddHeader(newheader);

     		int distination  = ParseDistinationNode(data);

     		NS_LOG_INFO ("type is request to "<< distination);

     		int msg_id  = ParseMessageId(data);
     		//std::cout<<"***************************************type is request to "<< distination << type <<"node id is "<<GetNode()->GetId()<<" msg id is "<< msg_id<<std::endl;
     		bool found = (my_find(oldRequest.begin(), oldRequest.end(), msg_id) != oldRequest.end());

     		if(!found){
     			oldRequest.push_back(msg_id);
     			if((double)distination != (double)GetNode()->GetId()){



     				if(ismyneighbor(distination)){
     					std::cout<<"this request "<<msg_id<<" is for my neighbor"<<std::endl;
     					Address disAddr = ParseDistinationAddr(distination);
     					if(Inet6SocketAddress::IsMatchingType(disAddr)){
     					//packets6_list->push_back(packet_temp);
     					n_hops = currenthops.GetData()+1;
     					n_delivered++;
     					std::cout<<"this is node "<< GetNode()->GetId()<<" received a request "<< msg_id <<" for one of my my neighbors lowpan"<< std::endl;
     					trace_receiving_packet(msg_id,Simulator::Now().GetSeconds ());
     					}else{
     						neighbors_packets_list->push_back(packet_temp);

     					}
     				  //  	NS_LOG_INFO ("		--------------Trying to find "<<distination<<"  message id is "<<msg_id);

     			}else
     				packets_list->push_back(packet_temp);




     		}else{
     			n_hops = currenthops.GetData();
     			n_delivered++;
     			std::cout<<std::endl;
     			std::cout<<std::endl;
     			std::cout<<" i am node" <<GetNode()->GetId() << " and received a message "<< msg_id<< " to me  " << distination<<" after hops "<<currenthops.GetData()<<std::endl;
     			 std::cout<<std::endl;
     			std::cout<<std::endl;
     			trace_receiving_packet(msg_id,Simulator::Now().GetSeconds ());

     			//NS_LOG_INFO ("	+++++++++++++++ already seen this message "<<msg_id);
     		}

     		  // todo handle the request
     		  // add a list of seen packets
     		  // if the packet is to me then consume it
     		  // handle both request and response
     		  //

     	  }
   }


}

void
lte_echo_server::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  std::string data;
  MyHeader currenthops ;
  while ((packet = socket->RecvFrom (from)))
     {
       if (InetSocketAddress::IsMatchingType (from))
         {
           NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client  " << GetNode()->GetId() <<" received " << packet->GetSize () << " bytes from " <<
                        InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                        InetSocketAddress::ConvertFrom (from).GetPort ());

         		packet->RemoveHeader (currenthops);

           uint8_t *buffer = new uint8_t[packet->GetSize()];
                             		packet->CopyData (buffer, packet->GetSize());
                             		data = std::string((char*)buffer);
         }

     }
  	  std::string type = ParseType(data);
  	  if(type == "cover"){
  		NS_LOG_INFO ("type is discovery "<<std::endl);

  		bool gotCovered = ParseMessageCover(data);
  		double TxPower = ParseTxPowerFromMessage(data);
  		uint16_t masterNode = ParseMessageSender(data);
	    updateTxPower(TxPower);

  	  }else{

  		Ptr<Packet> packet_temp = Create<Packet> ((uint8_t*) data.c_str(), data.length());
  		MyHeader newheader;
  		newheader.SetData(currenthops.GetData()+1);
  		packet_temp->AddHeader(newheader);

  		int distination  = ParseDistinationNode(data);

  		NS_LOG_INFO ("type is request to "<< distination);

  		int msg_id  = ParseMessageId(data);
  		bool x =  ((double)distination == (double)GetNode()->GetId());
  		std::cout<<"type is request to "<< distination << type <<" node id is "<<GetNode()->GetId()<<
  				" msg id is "<< msg_id<< " and equality is " << x <<std::endl;
  		bool found = (my_find(oldRequest.begin(), oldRequest.end(), msg_id) != oldRequest.end());

  		if(!found){
  			oldRequest.push_back(msg_id);
  			if((double)distination != (double)GetNode()->GetId()){
  				//	exisits_in_neighbors_request(msg_id);



  				if(ismyneighbor(distination)){

  					Address disAddr = ParseDistinationAddr(distination);
  					if(Inet6SocketAddress::IsMatchingType(disAddr)){
  					packets6_list->push_back(packet_temp);
  					n_hops = currenthops.GetData()+1;
  					n_delivered++;
  					std::cout<<"this is node "<< GetNode()->GetId()<<" received a request "<< msg_id <<" for one of my my neighbors lowpan"<< std::endl;
  					trace_receiving_packet(msg_id,Simulator::Now().GetSeconds ());
  					}
  				  //  	NS_LOG_INFO ("		--------------Trying to find "<<distination<<"  message id is "<<msg_id);

  			}
  		}else{
  			n_hops = currenthops.GetData();
  			n_delivered++;
  			std::cout<<" i am node" <<GetNode()->GetId() << " and received a message "<< msg_id<< " to me  " << distination<<" after hops "<<currenthops.GetData()<<std::endl;
  			trace_receiving_packet(msg_id,Simulator::Now().GetSeconds ());
  			//NS_LOG_INFO ("	+++++++++++++++ already seen this message "<<msg_id);
  		}

  	  }
}

}

bool lte_echo_server::exisits_in_neighbors_request(int msg_id){
	std::string data2;
	  Ptr<Packet> packet;
	  Address from;
	  std::string data;
	  MyHeader currenthops ;

	for ( Ptr<Packet> packet_temp : (*neighbors_packets_list))
	 	  {
		packet = packet_temp->Copy();
		packet->RemoveHeader(currenthops);
		 uint8_t *buffer = new uint8_t[packet->GetSize()];
		 packet->CopyData (buffer, packet->GetSize());
		 data2 = std::string((char*)buffer);
		 std::cout<<"cheking this one for correctness "<< data2<<" we want id of "<< msg_id<<std::endl;
		int msg_id2  = ParseMessageId(data2);
		if(msg_id2 == msg_id){
		std::cout<<"++++++++++++ ++++++++++++ ++++++++++removing because it exists size was "<<neighbors_packets_list->size()<<std::endl;
			neighbors_packets_list->remove(packet_temp);
			std::cout<<"++++++++++++ ++++++++++++ ++++++++++removed done size is "<<neighbors_packets_list->size()<<std::endl;
		return true;
		}


	 	  }
	std::cout<<"out but kiking "<<std::endl;
	return false;

}
neighbor lte_echo_server::ParseMassage(std::string msg,Address from){
	int id ;
	std::list<int> Nbors;
	double battery;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(1),'=')[1]) >> id;


	std::istringstream ( split(fields.at(2),'=')[1]) >> battery;
	std::vector<std::string>  temp = split(fields.at(3),'=');

	if(temp.size()>1){
		std::string rest = split(fields.at(3),'=')[1];
		std::vector<std::string> nb = split(rest,',');

		for(uint  i=0;i<nb.size()-1;i++){
			int x;
			std::istringstream (nb[i]) >> x;

			Nbors.push_back(x);

		}
		Nbors.push_back(id);
	}


	neighbor n (from,2,battery,Nbors,Simulator::Now ().GetSeconds (),id);
return n;



}


bool lte_echo_server::ParseMessageCover(std::string msg){
	std::list<int> Nbors;

	std::vector<std::string> fields = split(msg,'|');
	std::vector<std::string>  temp = split(fields.at(2),'=');

	if(temp.size()>1){
		std::string rest = split(fields.at(2),'=')[1];
		std::vector<std::string> nb = split(rest,',');

		for(uint  i=0;i<nb.size()-1;i++){
			int x;
			std::istringstream (nb[i]) >> x;

			Nbors.push_back(x);
			if((double)GetNode()->GetId() == (double) x)
				return true;

		}
	}



return false;



}

double lte_echo_server::ParseTxPowerFromMessage(std::string msg){

	double TxPower;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(1),'=')[3]) >> TxPower;


return TxPower;



}

void lte_echo_server::updateTxPower(double txPower){

	  Ptr<NetDevice> dev = GetNode()->GetDevice(0);
	  Ptr<NistLteUeNetDevice> ueDev = DynamicCast<NistLteUeNetDevice>(dev);
	  Ptr<NistLteUePhy> uePhy = ueDev->GetPhy();
	  if(uePhy)
		  uePhy->SetTxPower(txPower);

}


uint16_t lte_echo_server::ParseMessageSender(std::string msg){
	int id ;


	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(1),'=')[1]) >> id;


return id;

}

std::string
lte_echo_server::ParseType(std::string msg){
	std::string type ;
	std::list<int> Nbors;
	double battery;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> type;



return type;



}
int
lte_echo_server::ParseDistinationNode(std::string msg){
	int id ;

	std::vector<std::string> fields = split(msg,'|');
	std::istringstream ( split(fields.at(3),'=')[1]) >> id;
return id;
}
bool lte_echo_server::ismyneighbor(int id){
	std::map<Address, neighbor>::iterator it;
		 for ( it = MyTable->begin(); it != MyTable->end(); it++ ){

			   if(it->second.node_id == id){
				  return true;

			   }

		   }
		 return false;
}
Address
lte_echo_server::ParseDistinationAddr(int id){
	std::map<Address, neighbor>::iterator it;
	 for ( it = MyTable->begin(); it != MyTable->end(); it++ ){

		   if(it->second.node_id == id){
			  return it->first;

		   }

	   }
Address temp;
return temp;
}

int
lte_echo_server::ParseMessageId(std::string msg){
	int id ;

	std::vector<std::string> fields = split(msg,'|');
	std::istringstream ( split(fields.at(2),'=')[1]) >> id;
return id;
}

std::vector<std::string> lte_echo_server::split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

void
lte_echo_server::SetTable ( std::map<Address,neighbor> *table){
	MyTable = table;
}

void
lte_echo_server::SetPacketsList ( std::list<Ptr<Packet>> *t_list){
	packets_list = t_list;
}

void
lte_echo_server::SetNeighborsPacketsList ( std::list<Ptr<Packet>> *t_list){
	neighbors_packets_list = t_list;

}

void
lte_echo_server::SetPacketsv6List (std::list<Ptr<Packet>> *t_list){
	packets6_list = t_list;
}

void
lte_echo_server::SetBroadcastAddress(Address address ){
	m_peerAddress = address;
}

void
lte_echo_server::SetBroadcastSocket( Ptr<Socket> socket){
	m_socket2 = socket;
}

void
lte_echo_server::SetLteClient( Ptr<Lte_Echo_Client> *client){
	m_client = client;
}

void
lte_echo_server::SetState(std::string *state){
	m_state = state;
}


template<class InputIterator, class T>
InputIterator lte_echo_server::my_find (InputIterator first, InputIterator last, const T& val)

{
while (first!=last) {
  if (*first==val) return first;
  ++first;
}
return last;
}

void
lte_echo_server::ScheduleCheck(Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  Simulator::Schedule (dt, &lte_echo_server::updatesocket, this);
}

void lte_echo_server::updatesocket(){

	if((*m_state).size() != 0){
		std::cout<<"-------------------------------trying to bind here "<< (*m_state).size() <<std::endl;
		if((*m_state) == "CDS"){

		  if (m_socket2 == 0)
		      {

		        TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		        m_socket2 = Socket::CreateSocket (GetNode (), tid);
		        InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 999);
		        m_socket2->Bind (local);
		        m_socket2->BindToNetDevice(GetNode()->GetDevice(if_index));

		        std::cout<<" think bound "<<std::endl;

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
		        m_socket2->SetRecvCallback (MakeCallback (&lte_echo_server::HandleRead2, this));
		      }

	}else{
		if(m_socket2 != 0){
		m_socket2->ShutdownRecv();
		m_socket2->ShutdownSend();
		m_socket2->Close();

		m_socket2 = 0;
		std::cout<<"i think i am here "<<std::endl;

		}
	}
	}
	ScheduleCheck(Seconds (1.0));
}


} // Namespace ns3
