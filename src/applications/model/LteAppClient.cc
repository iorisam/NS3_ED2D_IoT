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
#include "LteAppClient.h"
#include "ns3/neighbor.h"
#include <string>
#include <sstream>
#include "ns3/address-utils.h"
#include "ns3/energy-module.h"
#include <stdlib.h>
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include "ns3/network-module.h"
#include "ns3/internet-module.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteAppClientApplication");

NS_OBJECT_ENSURE_REGISTERED (LteAppClient);

TypeId
LteAppClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LteAppClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<LteAppClient> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LteAppClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&LteAppClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress",
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&LteAppClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort",
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&LteAppClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LteAppClient::SetDataSize,
                                         &LteAppClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&LteAppClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

LteAppClient::LteAppClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;

}

LteAppClient::~LteAppClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void
LteAppClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
LteAppClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
LteAppClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
LteAppClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
           Ipv4InterfaceAddress iaddr = ipv4->GetAddress (2,0);
           Ipv4Address ipAddr= iaddr.GetLocal ();

      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind(ipAddr);
          m_socket->BindToNetDevice(m_device);
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
          // m_socket->BindToNetDevice(m_device1);
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind6();
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind ();
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          m_socket->Bind6 ();
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&LteAppClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  ScheduleTransmit (Seconds (0.));
}

void
LteAppClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  Simulator::Cancel (m_sendEvent);
}

void
LteAppClient::SetDataSize (uint32_t dataSize)
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
LteAppClient::SetDevice (Ptr<NetDevice> device1)
{
  NS_LOG_FUNCTION_NOARGS ();
 m_device = device1;
}

uint32_t
LteAppClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void
LteAppClient::SetFill (std::string fill)
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

void
LteAppClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}




void
LteAppClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void
LteAppClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &LteAppClient::Send, this);



}

void
LteAppClient::ScheduleCheck(Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  Simulator::Schedule (dt, &LteAppClient::UpdateTable, this);
}



void
LteAppClient::updatestate()
{

	std::map<Address, neighbor>::iterator it;
	std::list<int> N_ids;
	int my_id;

	  for ( it = MyTable.begin(); it != MyTable.end(); it++ )
	  {
		  	  N_ids.push_back(it->second.node_id);

	    }
	  my_id = GetNode()->GetId();
	  N_ids.push_back(my_id);

	  for ( it = MyTable.begin(); it != MyTable.end(); it++ )
	  {

		  if(contains(N_ids,it->second.neighbors)){
			if(it->second.neighbors.size() == N_ids.size() && my_id > it->second.node_id)
				state = "CDS";
			break;

		  }else if(!allconnected()){
			  state = "CDS";
			  			break;
		  }else{
			  state = "temp";
			  			  			break;
		  }

	    }

//	  std::cout<<"node "<<GetNode()->GetId()<<" became a "<<state<<" my neighbors are :"<<std::endl;
//
//	  for ( it = MyTable.begin(); it != MyTable.end(); it++ )
//	 	  {
//		  std::cout<<"node "<<it->second.node_id<<" and has neighbors ";
//		  for (int x : it->second.neighbors)
//			  std::cout<< x <<" , ";
//		  std::cout<<std::endl;
//	 	  }



}


bool LteAppClient::contains(std::list<int> list1,std::list<int>list2)
{
	if(list1.size()>list2.size())
		return false;

	for (int i:list1){
	bool found = (std::find(list2.begin(), list2.end(), i) != list2.end());
	if(!found)
		return false;
	}
	return true;


}

bool LteAppClient::allconnected()
{
	std::map<Address, neighbor>::iterator it;
	std::map<Address, neighbor>::iterator it2;

			 for ( it = MyTable.begin(); it != MyTable.end(); it++ )
				 for ( it2 = it; it2 != MyTable.end(); it2++ ){
					 bool found = (std::find(it2->second.neighbors.begin(), it2->second.neighbors.end(),it->second.node_id ) != it2->second.neighbors.end());

					 if(!found)
						 return false;

				 }

			 return true;
}

void
LteAppClient::UpdateTable (void)
{
  NS_LOG_FUNCTION (this);
double inseconds = 5*m_interval.GetSeconds();
 std::map<Address, neighbor>::iterator it;
 NS_LOG_LOGIC ("befor delete"<<MyTable.size());
  for ( it = MyTable.begin(); it != MyTable.end(); it++ )
  {





	if(Simulator::Now ().GetSeconds ()-it->second.discovered_time>=inseconds){


    	it = MyTable.erase(it);
    	if(MyTable.size()<1)
    		return;

    }


  }
  updatestate();
  NS_LOG_LOGIC ("after delete"<<MyTable.size());

}

void
LteAppClient::Send (void)
{



  NS_LOG_LOGIC ("trying to send a message");
  NS_LOG_FUNCTION (this);
  //std::cout<<"trying to send a message";
  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
  if (m_dataSize)
    {
      //
      // If m_dataSize is non-zero, we have a data buffer of the same size that we
      // are expected to copy and send.  This state of affairs is created if one of
      // the Fill functions is called.  In this case, m_size must have been set
      // to agree with m_dataSize
      //
      NS_ASSERT_MSG (m_dataSize == m_size, "LteAppClient::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "LteAppClient::Send(): m_dataSize but no m_data");
      p = Create<Packet> (m_data, m_dataSize);
    }
  else
    {
      //
      // If m_dataSize is zero, the client has indicated that it doesn't care
      // about the data itself either by specifying the data size by setting
      // the corresponding attribute or by not calling a SetFill function.  In
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the (zero) m_dataSize.
      //
      p = Create<Packet> (m_size);
    }
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  m_socket->Send (p);

  ++m_sent;

  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent this is my application" << m_size << " bytes to " <<
                   Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
                   Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
    }

  if (m_sent < m_count)
    {
	  ScheduleCheck (Seconds(1.0));
      ScheduleTransmit (m_interval);
    }

  NS_LOG_LOGIC ("message sent");

}

std::vector<std::string> LteAppClient::split(const std::string& s, char delimiter)
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

neighbor LteAppClient::ParseMassage(std::string msg,Address from){


	int id ;
	std::list<int> Nbors;
	double battery;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> id;


	std::istringstream ( split(fields.at(1),'=')[1]) >> battery;
	std::vector<std::string>  temp = split(fields.at(2),'=');

	if(temp.size()>1){
		std::string rest = split(fields.at(2),'=')[1];

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

void
LteAppClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  std::string data;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
                       InetSocketAddress::ConvertFrom (from).GetPort ());
          uint8_t *buffer = new uint8_t[packet->GetSize()];
                   		packet->CopyData (buffer, packet->GetSize());
                   		data = std::string((char*)buffer);

        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
        }
    }




 neighbor n = ParseMassage(data,from);

//std::cout<<"data is : " <<data;
 std::map<Address,neighbor>::iterator it;
 it = MyTable.find(from);



  if(it==MyTable.end())
  {

	  MyTable.insert(std::pair<Address , neighbor>(from,n));
  } else{

	  MyTable.erase(it);

	  MyTable.insert(std::pair<Address , neighbor>(from,n));
  }



}


} // Namespace ns3
