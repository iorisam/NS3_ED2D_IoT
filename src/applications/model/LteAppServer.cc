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
#include "ns3/energy-module.h"
#include <string>
#include <sstream>
#include "LteAppServer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteAppServerApplication");

NS_OBJECT_ENSURE_REGISTERED (LteAppServer);

TypeId
LteAppServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteAppServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<LteAppServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&LteAppServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
   .AddAttribute ("My_energy",
		   	   	   "level of energy source.",
                    DoubleValue (100.0), // as a fraction of the initial energy
                    MakeDoubleAccessor (&LteAppServer::m_BatteryLvL),
                    MakeDoubleChecker<double> ())
  ;
  return tid;
}
void
LteAppServer::RemainingEnergy (double oldValue, double remainingEnergy)
{
	 std::cout << "hhhhooooohhhooo";
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s Current remaining energy = " << remainingEnergy << "J");
}

/// Trace function for total energy consumption at node.

LteAppServer::LteAppServer ()
{
  NS_LOG_FUNCTION (this);
}

LteAppServer::~LteAppServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void
LteAppServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
LteAppServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);

      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_socket->Bind (local);

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

  m_socket->SetRecvCallback (MakeCallback (&LteAppServer::HandleRead, this));
  m_socket6->SetRecvCallback (MakeCallback (&LteAppServer::HandleRead, this));
}

void 
LteAppServer::StopApplication ()
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
LteAppServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received "<<GetNode()->GetId()<<" received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server "<<GetNode()->GetId()<<" received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
                       std::cout<<"received the ipv6 message node is "<<GetNode()->GetId();
        }

      packet->RemoveAllPacketTags ();
      packet->RemoveAllByteTags ();

      //**********************************


        std::ostringstream oss;
        //std::cout<<"batteryLVL is "<<m_source;
        oss << "nodeID="<<GetNode()->GetId()<<"|batteryLvL="<<m_source->GetRemainingEnergy()<<"|neighbors=";
        if(m_type!="sensor"){
        std::map<Address, neighbor>::iterator it;

        for ( it = (*MyTable).begin(); it != (*MyTable).end(); it++ )
         {


        	oss << it->second.node_id<<",";
         }
        }
        oss<<"|";



        std::string lemessage = oss.str();

        Ptr<Packet> packet2 = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());





      //**********************************

      NS_LOG_LOGIC ("Echoing packet");



      socket->SendTo (packet2, 0, from);

      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());

        }
    }
}
void
LteAppServer::SetDataSize (uint32_t dataSize)
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
LteAppServer::SetSource (Ptr<BasicEnergySource> source)
{
m_source = source;
}

void
LteAppServer::SetTable ()
{
	std::map <Address,neighbor> temp;
	MyTable = &temp;
}

void
LteAppServer::SetTable ( std::map<Address,neighbor> *table){
	MyTable = table;


}

void
LteAppServer::Settype ( std::string type){
	m_type = type;
}
uint32_t
LteAppServer::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void
LteAppServer::SetFill (std::string fill)
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

} // Namespace ns3
