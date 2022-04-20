/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007-2009 Strasbourg University
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
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 */

#include "lr-client.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/icmpv6-header.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-extension-header.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/random-variable-stream.h"


namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("Lr6Application");

NS_OBJECT_ENSURE_REGISTERED (LRClient);

TypeId LRClient::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::LRClient")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<LRClient>()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LRClient::m_count),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&LRClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteIpv6",
                   "The Ipv6Address of the outbound packets",
                   Ipv6AddressValue (),
                   MakeIpv6AddressAccessor (&LRClient::m_peerAddress),
                   MakeIpv6AddressChecker ())
    .AddAttribute ("LocalIpv6",
                   "Local Ipv6Address of the sender",
                   Ipv6AddressValue (),
                   MakeIpv6AddressAccessor (&LRClient::m_localAddress),
                   MakeIpv6AddressChecker ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LRClient::m_size),
                   MakeUintegerChecker<uint32_t>())
  ;
  return tid;
}

LRClient::LRClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sent = 0;
  m_socket = 0;
  m_seq = 0;
  m_sendEvent = EventId ();
}

LRClient::~LRClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

void LRClient::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void LRClient::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (!m_socket)
    {
      TypeId tid = TypeId::LookupByName ("ns3::Ipv6RawSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);

      NS_ASSERT (m_socket);

//      m_socket->Bind (Inet6SocketAddress (m_localAddress, 0));
//      m_socket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_ICMPV6));
//      m_socket->SetRecvCallback (MakeCallback (&LRClient::HandleRead, this));
    }
  packetsv6_list = {};
  ScheduleTransmit (Seconds (0.));
}

void LRClient::SetLocal (Ipv6Address ipv6)
{
  NS_LOG_FUNCTION (this << ipv6);
  m_localAddress = ipv6;
}

void LRClient::SetRemote (Ipv6Address ipv6)
{
  NS_LOG_FUNCTION (this << ipv6);
  m_peerAddress = ipv6;
}

void LRClient::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> >());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}

void LRClient::SetIfIndex (uint32_t ifIndex)
{
  m_ifIndex = ifIndex;
}

void LRClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  if(m_socket != 0){
  double value;
       Ptr<UniformRandomVariable> uniform = CreateObject<UniformRandomVariable> ();
       value = uniform->GetInteger(500,900)/1000.0;
  m_sendEvent = Simulator::Schedule (Seconds(value), &LRClient::Send, this);
  }
}

void LRClient::SetRouters (std::vector<Ipv6Address> routers)
{
  m_routers = routers;
}
void
LRClient::SetTable ( std::map<Address,neighbor> *table){
	MyTable = table;
}
void LRClient::Send ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (m_sendEvent.IsExpired ());
  if( m_socket != 0){

  Inet6SocketAddress sinkAdr = Inet6SocketAddress (Ipv6Address("FF02::2"), 9);

  TypeId tid = UdpSocketFactory::GetTypeId();
  Ptr<Socket> source = Socket::CreateSocket(GetNode(), tid);
    Inet6SocketAddress sourceAdr = Inet6SocketAddress (Ipv6Address::GetAny(), 9);

  	source->SetIpv6Tclass (0);

    source->SetIpv6HopLimit (0);
    source->SetRecvPktInfo(true);
    source->SetIpv6RecvHopLimit(true);
    source->Connect(sourceAdr);
    source->BindToNetDevice(m_device);

    source->SetAllowBroadcast(true);
    std::ostringstream oss;

    oss << "type=discovery|nodeID="<<GetNode()->GetId()<<"|";
           std::string lemessage = oss.str();

  Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
//Icmpv6Echo req (1);
//
//  req.SetId (0xBEEF);
//  req.SetSeq (m_seq);
//  m_seq++;

  /* we do not calculate pseudo header checksum here, because we are not sure about 
   * source IPv6 address. Checksum is calculated in Ipv6RawSocketImpl.
   */

 // p->AddHeader (req);
    source->SetRecvCallback (MakeCallback (&LRClient::HandleRead, this));
NS_LOG_LOGIC ("trying to send a message lr pan");
    source->SendTo (p, 0, sinkAdr);

 
  NS_LOG_INFO ("Sent " << p->GetSize () << " bytes to " << sinkAdr);

  std::list<std::string>::iterator it2;
  for ( Ptr<Packet> packet_temp:packetsv6_list)
   	  {

  	  source->SendTo (packet_temp, 0, sinkAdr);

   	  }

  packetsv6_list = {};

  m_sent++;
  }
      ScheduleTransmit (m_interval);

}

void
LRClient::SetDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION_NOARGS ();
 m_device = device;
}

void LRClient::HandleRead (Ptr<Socket> socket)
{
	 NS_LOG_FUNCTION (this << socket);
	  Ptr<Packet> packet;
	  Address from;
	  std::string data;
	  if(m_socket != 0){
	  while ((packet = socket->RecvFrom (from)))
	    {
	      if (InetSocketAddress::IsMatchingType (from))
	        {
	          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
	                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
	                       InetSocketAddress::ConvertFrom (from).GetPort ());
	          break;

	        }
	      else if (Inet6SocketAddress::IsMatchingType (from))
	        {
	          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client"<<GetNode()->GetId()<<" received " << packet->GetSize () << " bytes from " <<
	                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
	                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
                         uint8_t *buffer = new uint8_t[packet->GetSize()];

             	                   		packet->CopyData (buffer, packet->GetSize());

             	                   		data = std::string((char*)buffer);

             	                   	// std::cout<<"node  : " <<GetNode()->GetId() << "received response from node "<<data<<std::endl;
             	                   	break;
	        }

	    }






	 neighbor n = ParseMassage(data,from);
	// std::cout<<"out of the receive loop  "<<std::endl;
	 std::map<Address,neighbor>::iterator it;
	 std::map<Address,neighbor>::iterator it2;
	 it = (*MyTable).find(from);
	 std::cout<<"at time" << Simulator::Now ().GetSeconds () <<"node  : " <<(double)GetNode()->GetId() << "received response message from "<<
			 (double)n.node_id<<std::endl;
	 bool exists = false;
	 for (it2 = MyTable->begin();it2!=MyTable->end();it2++){
		 if(it2->second.node_id == n.node_id){
			 exists = true;
			 break;
		 }
	 }
	  if(!exists)
	  {

		  (*MyTable).insert(std::pair<Address , neighbor>(from,n));
	  } else{

		 it2->second.battery =  n.battery;
		 it2->second.discovered_time = n.discovered_time;

	  }
	  NS_LOG_INFO ("out of hundle read thank you"<<std::endl);
	  }

}

neighbor LRClient::ParseMassage(std::string msg,Address from){
	 NS_LOG_FUNCTION (this << msg);
	int id ;
	std::list<int> Nbors;
	double battery;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> id;


	std::istringstream ( split(fields.at(1),'=')[1]) >> battery;
	std::vector<std::string>  temp = split(fields.at(2),'=');

	if(temp.size()>1){
		std::string rest = split(fields.at(2),'=')[1];
		if(rest!="="){
		std::vector<std::string> nb = split(rest,',');

		for(uint  i=0;i<nb.size()-1;i++){
			int x;
			std::istringstream (nb[i]) >> x;
			Nbors.push_back(x);

		}
		Nbors.push_back(id);
		}
	}
	NS_LOG_INFO ("out of parsing function "<<std::endl);


	neighbor n (from,2,battery,Nbors,Simulator::Now ().GetSeconds (),id);
return n;

NS_LOG_INFO ("out of parsing function "<<std::endl);

}
std::vector<std::string> LRClient::split(const std::string& s, char delimiter)
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
} /* namespace ns3 */
