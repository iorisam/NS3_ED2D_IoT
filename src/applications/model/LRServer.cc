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
#include "ns3/ipv6.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-extension-header.h"
#include <sstream>
#include <string>
#include "ns3/random-variable-stream.h"

#include "LRServer.h"
#include "Myheader.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LRServerApplication");

NS_OBJECT_ENSURE_REGISTERED (LRServer);

TypeId
LRServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LRServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<LRServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&LRServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

LRServer::LRServer ()
{
  NS_LOG_FUNCTION (this);
}

LRServer::~LRServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void
LRServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
LRServer::SetDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this);
 m_device = device;
}

void
LRServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress sinkAdr = Inet6SocketAddress (Ipv6Address("FF02::2"), m_port);
      m_socket6->Bind(sinkAdr);

      m_socket6->BindToNetDevice(m_device);
  m_socket6->SetRecvCallback (MakeCallback (&LRServer::HandleRead, this));
}

void
LRServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket6 != 0)
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket6 = 0;
    }
}

void
LRServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  std::string data;
if(m_socket6 != 0){
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server node "<< GetNode()->GetId()<<" received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
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


        std::string type = ParseType(data);

      if(type !="request")
    	//  std::cout<<" this is node "<< (double)GetNode()->GetId() << " request " << data <<std::endl;
{
      //packet->RemoveAllPacketTags ();
      //packet->RemoveAllByteTags ();

      NS_LOG_LOGIC ("Echoing packet");
      //std::cout<<"node  : " <<GetNode()->GetId() << "received  a broadcast message from "<<from<<std::endl;



        std::cout<<"at time" << Simulator::Now ().GetSeconds () <<"node  : " <<(double)GetNode()->GetId() << "received Disc message from "<<
        		from<<std::endl;
          ScheduleTransmit (from);
      }
}
          //std::cout<<"node  : " <<GetNode()->GetId() << "sent response message to "<<from<<std::endl;

}

void LRServer::ScheduleTransmit(Address from){

	 NS_LOG_FUNCTION (this);

	 double value;
	 const int count = 1000000;
	 if(m_socket6 != 0){
      Ptr<UniformRandomVariable> uniform = CreateObject<UniformRandomVariable> ();
      value = uniform->GetInteger(100,900)/10000.0;
	 Simulator::Schedule (Seconds(value), &LRServer::TransmitDiscoveryResponse,this,from);
	 }
}

void LRServer::TransmitDiscoveryResponse(Address from){
	 NS_LOG_FUNCTION (this);
	 if(m_socket6 != 0){
	std::ostringstream oss;
	 oss << "nodeID="<<GetNode()->GetId()<<"|batteryLvL=100.0"<<"|neighbors=|";
	        std::string lemessage = oss.str();

	        Ptr<Packet> packet = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());

	 TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	        Ptr<Socket> source = Socket::CreateSocket(GetNode(), tid);

	 source->SetIpv6Tclass (0);
	          source->SetIpv6HopLimit (0);
	          source->SetRecvPktInfo(true);
	          source->SetIpv6RecvHopLimit(true);
	          source->BindToNetDevice(m_device);
	          source->SendTo (packet, 0, from);
	 }

}

std::string
LRServer::ParseType(std::string msg){
	std::string type ;
	std::list<int> Nbors;
	double battery;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> type;



return type;
}

std::string
LRServer::ParseDest(std::string msg){

	std::string dest;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(3),'=')[1]) >> dest;
return dest;
}

std::vector<std::string> LRServer::split(const std::string& s, char delimiter)
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

} // Namespace ns3
