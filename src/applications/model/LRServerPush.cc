/*
 * LRServerPush.cc
 *
 *  Created on: Mar 23, 2019
 *      Author: lamboty
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
#include "LRServerPush.h"
#include "Myheader.h"




namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LRServerPushApplication");

NS_OBJECT_ENSURE_REGISTERED (LRServerPush);


LRServerPush::LRServerPush() {
	// TODO Auto-generated constructor stub

}

TypeId
LRServerPush::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LRServerPush")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<LRServerPush> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&LRServerPush::m_port),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}



LRServerPush::~LRServerPush()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;
}

void
LRServerPush::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
LRServerPush::SetDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION (this);
 m_device = device;
}

void
LRServerPush::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress sinkAdr = Inet6SocketAddress (Ipv6Address("FF02::2"), m_port);
      m_socket6->Bind(sinkAdr);

      m_socket6->BindToNetDevice(m_device);
  m_socket6->SetRecvCallback (MakeCallback (&LRServerPush::HandleRead, this));
}

void
LRServerPush::SetTable ( std::map<Address,neighbor> *table){
	MyTable = table;
}

void
LRServerPush::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket6 != 0)
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
LRServerPush::HandleRead (Ptr<Socket> socket)
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
 	 std::cout<<"at time" << Simulator::Now ().GetSeconds () <<"node  : " <<(double)GetNode()->GetId() << "received Discovery push message from "<<
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



std::string
LRServerPush::ParseType(std::string msg){
	std::string type ;
	std::list<int> Nbors;
	double battery;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> type;



return type;
}

std::string
LRServerPush::ParseDest(std::string msg){

	std::string dest;

	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(3),'=')[1]) >> dest;
return dest;
}

std::vector<std::string> LRServerPush::split(const std::string& s, char delimiter)
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

neighbor LRServerPush::ParseMassage(std::string msg,Address from){
	 NS_LOG_FUNCTION (this << msg);
	int id ;
	std::list<int> Nbors;
	double battery;
	int servicetype;


	std::vector<std::string> fields = split(msg,'|');

	std::istringstream ( split(fields.at(0),'=')[1]) >> id;


	std::istringstream ( split(fields.at(1),'=')[1]) >> battery;
	std::vector<std::string>  temp = split(fields.at(2),'=');
	std::istringstream ( split(fields.at(3),'=')[1]) >> servicetype;

	std::cout<<"the type received by node "<<GetNode()->GetId()<<" is "<<servicetype<<std::endl;


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
	n.servicetype = servicetype;

return n;

NS_LOG_INFO ("out of parsing function "<<std::endl);

}

}

