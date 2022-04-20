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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
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
#include "ns3/energy-module.h"
#include <string>
#include <sstream>
#include "request-response-server.h"
#include "Myheader.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RequestResponseServerApplication");

NS_OBJECT_ENSURE_REGISTERED (RequestResponseServer);

TypeId
RequestResponseServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RequestResponseServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<RequestResponseServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&RequestResponseServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
   .AddAttribute ("My_energy",
		   	   	   "level of energy source.",
                    DoubleValue (100.0), // as a fraction of the initial energy
                    MakeDoubleAccessor (&RequestResponseServer::m_BatteryLvL),
                    MakeDoubleChecker<double> ())
	.AddTraceSource ("n_hops",
								   "A State value to trace.",
								   MakeTraceSourceAccessor (&RequestResponseServer::n_hops))
    .AddTraceSource ("n_delivered",
			   "A State value to trace.",
			   MakeTraceSourceAccessor (&RequestResponseServer::n_delivered))
  ;
  return tid;
}
void
RequestResponseServer::RemainingEnergy (double oldValue, double remainingEnergy)
{
	 std::cout << "hhhhooooohhhooo";
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s Current remaining energy = " << remainingEnergy << "J");
}

/// Trace function for total energy consumption at node.

RequestResponseServer::RequestResponseServer ()
{
  NS_LOG_FUNCTION (this);
}

RequestResponseServer::~RequestResponseServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
  m_socket2 = 0;
  m_device1 = 0;
  m_device2 =0;
}

void
RequestResponseServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
RequestResponseServer::SetDevices (Ptr<NetDevice> device1,Ptr<NetDevice> device2)
{
  NS_LOG_FUNCTION (this);
 m_device1 = device1;
 m_device2 = device2;
}

void 
RequestResponseServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);


      m_socket->Bind (local);
     // if(m_device1 != 0)
         //	  m_socket->BindToNetDevice(m_device1);

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



  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      m_socket6->Bind (local6);
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&RequestResponseServer::HandleRead, this));
  if (m_socket2 != 0){
  m_socket2->SetRecvCallback (MakeCallback (&RequestResponseServer::HandleRead, this));

  }
 // m_socket6->SetRecvCallback (MakeCallback (&RequestResponseServer::HandleRead, this));
}

void 
RequestResponseServer::StopApplication ()
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
RequestResponseServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  if(m_socket != 0){

  Ptr<Packet> packet;
  Address from;
  std::string data;
  std::cout<<"we are here what do you want "<<std::endl;
    MyHeader currenthops ;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s serverssssss received "<<GetNode()->GetId()<<" received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());

          packet->RemoveHeader (currenthops);

                     uint8_t *buffer = new uint8_t[packet->GetSize()];
                                       		packet->CopyData (buffer, packet->GetSize());
                                       		data = std::string((char*)buffer);
        }
    }

  std::string type = ParseType(data);
   	  if(type != "request"){

      //**********************************

   		if(type == "cover"){
   		 //updateCDSnode()

   		}else{
        std::ostringstream oss;
        std::cout<<"batteryLVL is yes yes yes yes yes yes yes yes yes yes yes"<<m_source->GetEnergyFraction();
        oss << "nodeID="<<GetNode()->GetId()<<"|batteryLvL="<<m_source->GetRemainingEnergy()<<"|neighbors=";
        if(m_type!="sensor"){
        std::map<Address, neighbor>::iterator it;

        for ( it = (*MyTable).begin(); it != (*MyTable).end(); it++ )
         {


        	oss << it->second.node_id<<",";
         }
        }
        oss<<"|status="<<(*status)<<"|";
        std::string lemessage = oss.str();

        Ptr<Packet> packet2 = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());

      NS_LOG_LOGIC ("Echoing packet " );

      socket->SendTo (packet2, 0, from);
   	  }

   	  }else{
   		Ptr<Packet> packet_temp = Create<Packet> ((uint8_t*) data.c_str(), data.length());
   		  		MyHeader newheader;
   		  		newheader.SetData(currenthops.GetData()+1);
   		  		packet_temp->AddHeader(newheader);

   		  		int distination  = ParseDistinationNode(data);
   		  		NS_LOG_INFO ("type is request to "<< distination);

   		  		int msg_id  = ParseMessageId(data);

   		  		bool found = (my_find(oldRequest.begin(), oldRequest.end(), msg_id) != oldRequest.end());

   		  		if(!found){
   		  			oldRequest.push_back(msg_id);
   		  			if(distination != GetNode()->GetId()){


   		  				if(ismyneighbor(distination)){

   		  					Address disAddr = ParseDistinationAddr(distination);
   		  					if(Inet6SocketAddress::IsMatchingType(disAddr)){
   		  					//packets6_list->push_back(packet_temp);
   		  					n_hops = currenthops.GetData()+1;
   		  				    n_delivered++;
   		  				    trace_receiving_packet(msg_id,Simulator::Now().GetSeconds ());
   		  					std::cout<<"this is node "<< GetNode()->GetId()<<" received a request "<< msg_id <<" for one of my my neighbors lowpan"<< std::endl;
   		  					}else{
   		  					std::cout<<"this is node "<<GetNode()->GetId()<<" sending a request to wifi neighbor "<<msg_id<< " after hops "
   		  							<<currenthops.GetData()<<" at time "<<Simulator::Now().GetSeconds () <<std::endl;
   		  					packets_list->push_back(packet_temp);
   		  					}
   		  				  //  	NS_LOG_INFO ("		--------------Trying to find "<<distination<<"  message id is "<<msg_id);

   		  			}else{
   		  				packets_list->push_back(packet_temp);
   		  			std::cout<<"this is node "<<GetNode()->GetId()<<" sending a request to wifi broadcast "<<msg_id<< " after hops "
   		  					<<currenthops.GetData()<<" at time "<<Simulator::Now().GetSeconds () <<std::endl;
   		  			}



   		  		}else{
   		  			std::cout<<" i am node" <<GetNode()->GetId() << " and received a message "<< msg_id<< " to me  " << distination<<" after hops "
   		  					<<currenthops.GetData()<<std::endl;
   		  		n_hops = currenthops.GetData();
   		  		n_delivered++;
   		  	trace_receiving_packet(msg_id,Simulator::Now().GetSeconds ());
   		  			//NS_LOG_INFO ("	+++++++++++++++ already seen this message "<<msg_id);
   		  		}
   	  }
}
  }else{
	  NS_LOG_INFO ("cant receive this message because i have no energy");
  }
}
void
RequestResponseServer::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so
  // neither will we.
  //
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}



void
RequestResponseServer::SetSource (Ptr<LiIonEnergySource> source)
{
m_source = source;
}

void
RequestResponseServer::SetTable ()
{
	std::map <Address,neighbor> temp;
	MyTable = &temp;
}

void
RequestResponseServer::SetTable ( std::map<Address,neighbor> *table){
	MyTable = table;


}

void
RequestResponseServer::Settype ( std::string type){
	m_type = type;
}
uint32_t
RequestResponseServer::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void
RequestResponseServer::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (this << fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

std::string
RequestResponseServer::ParseType(std::string msg){
	NS_LOG_FUNCTION (this);
	std::string type ;
	std::list<int> Nbors;
	double battery;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> type;



return type;



}
int
RequestResponseServer::ParseDistinationNode(std::string msg){
	NS_LOG_FUNCTION (this);
	int id ;

	std::vector<std::string> fields = split(msg,'|');
	std::istringstream ( split(fields.at(3),'=')[1]) >> id;
return id;
}
bool RequestResponseServer::ismyneighbor(int id){
	NS_LOG_FUNCTION (this);
	std::map<Address, neighbor>::iterator it;
		 for ( it = MyTable->begin(); it != MyTable->end(); it++ ){

			   if(it->second.node_id == id){
				  return true;

			   }

		   }
		 return false;
}
Address
RequestResponseServer::ParseDistinationAddr(int id){
	NS_LOG_FUNCTION (this);
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
RequestResponseServer::ParseMessageId(std::string msg){
	NS_LOG_FUNCTION (this);
	int id ;

	std::vector<std::string> fields = split(msg,'|');
	std::istringstream ( split(fields.at(2),'=')[1]) >> id;
return id;
}

template<class InputIterator, class T>
InputIterator RequestResponseServer::my_find (InputIterator first, InputIterator last, const T& val)

{
while (first!=last) {
  if (*first==val) return first;
  ++first;
}
return last;
}

std::vector<std::string> RequestResponseServer::split(const std::string& s, char delimiter)
{
	NS_LOG_FUNCTION (this);
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }

   NS_LOG_INFO ("out of server split");
   return tokens;
}
void
RequestResponseServer::SetPacketsList ( std::list<Ptr<Packet>> *t_list){
	packets_list = t_list;
}

void
RequestResponseServer::SetPacketsv6List (std::list<Ptr<Packet>> *t_list){
	packets6_list = t_list;
}

} // Namespace ns3
