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
#include "request-response-client.h"
#include "ns3/neighbor.h"
#include <string>
#include <sstream>
#include "ns3/address-utils.h"
#include "ns3/energy-module.h"
#include "ns3/traced-callback.h"
#include "Myheader.h"
#include <stdlib.h>
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include "ns3/ptr.h"
#include "ns3/energy-module.h"
#include "ns3/li-ion-energy-source-helper.h"
#include "ns3/netanim-module.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RequestResponseClientApplication");

NS_OBJECT_ENSURE_REGISTERED (RequestResponseClient);

TypeId
RequestResponseClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RequestResponseClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<RequestResponseClient> ()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&RequestResponseClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&RequestResponseClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress",
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&RequestResponseClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort",
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&RequestResponseClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&RequestResponseClient::SetDataSize,
                                         &RequestResponseClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&RequestResponseClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
	.AddTraceSource ("MyState",
					 "A State value to trace.",
					 MakeTraceSourceAccessor (&RequestResponseClient::m_myInt))
	.AddTraceSource ("n_packets",
				"A State value to trace.",
				MakeTraceSourceAccessor (&RequestResponseClient::n_packets))
  ;
  return tid;
}

RequestResponseClient::RequestResponseClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
}

RequestResponseClient::~RequestResponseClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void
RequestResponseClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
RequestResponseClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
RequestResponseClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
RequestResponseClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);

      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          m_socket->Bind();
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

  m_socket->SetRecvCallback (MakeCallback (&RequestResponseClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  ScheduleTransmit (Seconds (0.));
  //ScheduleRequest (Seconds (0.1));
}

void
RequestResponseClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0)
    {
     // m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

 // Simulator::Cancel (m_sendEvent);
}

void
RequestResponseClient::SetDataSize (uint32_t dataSize)
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
RequestResponseClient::SetDevices (Ptr<NetDevice> device1,Ptr<NetDevice> device2)
{
  NS_LOG_FUNCTION_NOARGS ();
 m_device1 = device1;
 m_device2 = device2;
}

uint32_t
RequestResponseClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void
RequestResponseClient::SetFill (std::string fill)
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
RequestResponseClient::SetFill (uint8_t fill, uint32_t dataSize)
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
RequestResponseClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
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
RequestResponseClient::ScheduleTransmit (Time dt)
{
	NS_LOG_FUNCTION (this << dt);
	if (m_socket != 0)
	    {

  m_sendEvent = Simulator::Schedule (dt, &RequestResponseClient::Send, this);

	    }

}

void
RequestResponseClient::ScheduleCheck(Time dt)
{
  NS_LOG_FUNCTION (this << dt);

  if (m_socket != 0)
  	    {
  Simulator::Schedule (dt, &RequestResponseClient::UpdateTable, this);
  	    }
}



void
RequestResponseClient::updatestate()
{
	 NS_LOG_FUNCTION (this);
	std::cout<< " trying to determin the fait of node "<<GetNode()->GetId()<<std::endl;
	std::map<Address, neighbor>::iterator it;
	std::list<int> N_ids;
	int my_id;
	std::stringstream ss;


	  for ( it = MyTable.begin(); it != MyTable.end(); it++ )
	  {
		  	  N_ids.push_back(it->second.node_id);
		  	ss << it->second.node_id << ",";
	    }


	 std::string neighb = ss.str();
	  my_id = GetNode()->GetId();
	  N_ids.push_back(my_id);
	  if(MyTable.size()==0){
		  state = "CDS";
		  m_myInt = 1;
		  m_statusInt = 1;
		  anim->UpdateNodeColor (GetNode(), 50, 50, 50); // Optional
		  anim->UpdateNodeDescription (GetNode(), neighb);
	  }

	  else if(covered(N_ids)){
		  	  	  	  	state = "temp";
		  				m_myInt = 0;
		  				m_statusInt = 0;
		  				anim->UpdateNodeColor (GetNode(), 0, 250, 0);
		  				anim->UpdateNodeDescription (GetNode(), neighb);
	  }else if(allconnected()){
		  	  	  	  	  state = "temp";
		  		  		  m_myInt = 0;
		  		  		m_statusInt = 0;
		  		  	anim->UpdateNodeColor (GetNode(), 0, 250, 0);
		  		  anim->UpdateNodeDescription (GetNode(), neighb);
	  } else{
		  	  	  	    state = "CDS";
		  				m_myInt = 1;
		  				m_statusInt = 1;
		  				anim->UpdateNodeColor (GetNode(), 50, 50, 50); // Optional
		  				anim->UpdateNodeDescription (GetNode(), neighb);
	  }

	  if(state == "CDS") {
		  std::cout<<" the state of node "<<GetNode()->GetId()<<" is CDS in case 0 "<<std::endl;
		  for (int i = 1; i<5 ; i++){
			  if (! (covered(i) && allconnected(i) && MyTable.size()>0)){
				  cdsStates[i] = 1;

					  std::cout<<" the state of node "<<GetNode()->GetId()<<" is CDS in case "<< i <<std::endl;
				  }else
					  cdsStates[i] = 0;
		  }
	  }
//	  LoadBalance();
//	  state = "CDS";
//	  m_myInt = 1;
//	  m_statusInt = 1;
	 /* std::cout << "node "<<GetNode()->GetId()<<" became a "<< state <<"with :";
	    for (int n : N_ids) {
	        std::cout << n << ',';
	    }
	    std::cout<<std::endl;*/
	  //NS_LOG_LOGIC ("this is the energy that i was looking for "<<myEnergy->GetTotalEnergyConsumption());
	    NS_LOG_LOGIC ("node "<<GetNode()->GetId()<<" became a aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa "<<state);

}

bool RequestResponseClient::LoadBalance()
{
	 NS_LOG_FUNCTION (this);
	std::map<Address, neighbor>::iterator it;
	std::map<Address, neighbor>::iterator it2;
	std::map<Address, neighbor> Covered(MyTable);
	std::map<Address, neighbor> dominating_nodes;
	double m_selection_factor =0.0;
	if(m_source->GetEnergyFraction()!=0)
	m_selection_factor = (1/m_source->GetEnergyFraction())*Covered.size();
	else
		m_selection_factor = 10000;
	//std::cout<<"selection factor is no no no no no  no no no no no "<<m_selection_factor;



		 for ( it = Covered.begin(); it != Covered.end(); it++ )
				 if(it->second.status!=0)
					 dominating_nodes.insert(std::pair<Address , neighbor>(it->first,it->second));

		 if(dominating_nodes.size()<1)
			 return true;



		  for ( it = Covered.begin(); it != Covered.end(); it++ )
			  {
			  int idmax = CalculateMaxSf(dominating_nodes,Covered.size());

			  for ( it2 = dominating_nodes.begin(); it2 != dominating_nodes.end(); it2++ )
				  if(it2->second.node_id != idmax
						  && (std::find(it2->second.neighbors.begin(), it2->second.neighbors.end(), it->second.node_id) != it2->second.neighbors.end())
						  && (std::find(it2->second.neighbors.begin(), it2->second.neighbors.end(), idmax) != it2->second.neighbors.end())){
					  it2->second.neighbors.remove(it->second.node_id);

				  }
			  if(GetNode()->GetId() != idmax)
			 				  it = Covered.erase(it);
			 			  if(Covered.size()<1)
			 			      		return true;
			  }


		  	  SendJoinMessageToCoveredNodes(Covered);
			 return true;
}



int RequestResponseClient::CalculateMaxSf(std::map<Address,neighbor> dominating_nodes,int m_coverage_degree){
	 NS_LOG_FUNCTION (this);
	std::map<Address, neighbor>::iterator it;
	int idmax = 0;
	double sfmax=100000;
	 for ( it = dominating_nodes.begin(); it != dominating_nodes.end(); it++ ){
		 double sf = (m_source->GetInitialEnergy()/it->second.battery)*it->second.neighbors.size();
		 if (sf<sfmax) {
			 sfmax = sf;
			 idmax = it->second.node_id;
		 }

	 }
	 double m_selection_factor =0.0;
			 	if(m_source->GetEnergyFraction()!=0)
			 	m_selection_factor = (1/m_source->GetEnergyFraction())*m_coverage_degree;
			 	else
			 		m_selection_factor = 10000;
			 if (m_selection_factor<sfmax)
				 idmax = GetNode()->GetId();

			 return idmax;


}

void RequestResponseClient::SendJoinMessageToCoveredNodes(std::map<Address,neighbor> covered){

	  NS_LOG_LOGIC ("trying to send a join message to all covered nodes");
	  NS_LOG_FUNCTION (this);
      std::ostringstream oss;
	         oss << "type=cover|nodeID="<<GetNode()->GetId()<<"|neighbors=";
	         std::map<Address, neighbor>::iterator it;
	         for ( it = covered.begin(); it != covered.end(); it++ )
	          {
	         	oss << it->second.node_id<<",";
	          }

	         oss<<"|";
	         std::string lemessage = oss.str();

	         Ptr<Packet> packet2 = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());

	       NS_LOG_LOGIC ("Echoing packet");

	       m_socket->Send(packet2);

}

bool RequestResponseClient::SelectBackup(){
	 NS_LOG_FUNCTION (this);
	std::map<Address, neighbor>::iterator it;
		std::map<Address, neighbor>::iterator it2;
		std::map<Address, neighbor> non_dominating_nodes;
		non_dominating_nodes = Get_non__dominating_nodes();

		double maxfactor = 0;
		int maxid = 0;

			 for ( it = MyTable.begin(); it != MyTable.end(); it++ )
				          {
				 double temp = (((it->second.battery/m_source->GetInitialEnergy())*it->second.signal_strength)/it->second.mobility);
				         	if(InetSocketAddress::IsMatchingType (it->first) && it->second.status!=1
				         			&& (temp>maxfactor))
				         		maxfactor = temp;
				         		maxid = it->second.node_id;

				          }

}
std::map<Address, neighbor> RequestResponseClient::Get_non__dominating_nodes(){
	std::map<Address, neighbor>::iterator it;


}
bool RequestResponseClient::covered(std::list<int> my_nbrs){
	 NS_LOG_FUNCTION (this);
	std::map<Address, neighbor>::iterator it;
	  for ( it = MyTable.begin(); it != MyTable.end(); it++ )
	  {


		  if(contains(my_nbrs,it->second.neighbors) && it->second.node_id>GetNode()->GetId())
			  	  return true;

		  }
	  return false;

}


bool RequestResponseClient::covered(int servicetype){
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



bool RequestResponseClient::contains(std::list<int> list1,std::list<int>list2)
{
	 NS_LOG_FUNCTION (this);
	if(list1.size()>list2.size())
		return false;

	for (int i:list1){
	bool found = (std::find(list2.begin(), list2.end(), i) != list2.end());
	if(!found)
		return false;
	}
	return true;


}

bool RequestResponseClient::allconnected()
{
	 NS_LOG_FUNCTION (this);
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



bool RequestResponseClient::allconnected(int servicetype)
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

void
RequestResponseClient::UpdateTable (void)
{
  NS_LOG_FUNCTION (this);
if(m_socket != 0){
double inseconds = 2*m_interval.GetSeconds();
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
}
  NS_LOG_LOGIC ("after delete"<<MyTable.size());

}


void
RequestResponseClient::Send (void)
{



  NS_LOG_LOGIC ("trying to send a message");
  NS_LOG_FUNCTION (this);
  if (m_socket != 0)
  	    {
  NS_ASSERT (m_sendEvent.IsExpired ());


	  std::ostringstream oss;
	  oss << "type=discovery|nodeID="<<GetNode()->GetId()<<"|batteryLvL="<<0<<"|neighbors=|aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	  std::string lemessage = oss.str();

	 Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
	 std::cout<<"At time " << Simulator::Now ().GetSeconds () << "s client "<<GetNode()->GetId()<<" sent " << p->GetSize() << " bytes to " <<
	         Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort<<std::endl;

  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace (p);
  m_socket->Send (p);
  ++m_sent;
  bool mytech = true;

  std::cout<<"The queue size is "<<packets_list.size()<<std::endl;

  //change this to disactivate the cds
  if(state == "CDS")
  for ( Ptr<Packet> packet_temp : packets_list)
   	  {
	  	uint8_t *buffer = new uint8_t[packet_temp->GetSize()];
	  	packet_temp->CopyData (buffer, packet_temp->GetSize());
		std::ostringstream oss;
		for(int i = 2; i<packet_temp->GetSize();i++){
			oss<<buffer[i];

		}
		std::cout<<" the packet message is "<<oss.str()<<std::endl;
		int requested_servicetype = ParseServiceType(oss.str());
		std::cout<<"request service type is "<<requested_servicetype<<std::endl;
		int cdsState = cdsStates[requested_servicetype];

		if(mytech){
			if(requested_servicetype==0 || cdsState > 0 ){
			  m_socket->Send (packet_temp);
			  ++m_sent;
		}
//
//		std::cout<<"request service type is "<<requested_servicetype<<" and cdsState is "<<cdsState<<std::endl;

		}else{
			 m_socket->Send (packet_temp);
			  ++m_sent;
		}
   	  }

    packets_list = {};





	  ScheduleCheck (2*m_interval);
      ScheduleTransmit (1*m_interval);


  NS_LOG_LOGIC ("message sent");
  	    }

}

void
RequestResponseClient::Request (uint32_t rnd_node, uint32_t packet_id, int servicetype)
{
  NS_LOG_FUNCTION (this);
  if (m_socket != 0)
  	    {
  n_packets++;

  std::ostringstream oss;
  std::map<Address, neighbor>::iterator it;
  std::map<Address, neighbor>::iterator result;
  std::list<int>::iterator result2;
  NS_LOG_LOGIC( "   selected node is  "<<rnd_node);
  trace_sending_packet(packet_id,Simulator::Now().GetSeconds());
  // double rnd_id = gen->GetInteger(0,1000000000);

bool found = false;
bool found2 = false;
   for ( it = MyTable.begin(); it != MyTable.end(); it++ ){
	   found = (it->second.node_id == rnd_node);
	   if(found){
		   result = it;
		   NS_LOG_INFO ("found in my neighbors ");
		   break;

	   }
   }
  		if((!found) || ((found) && (InetSocketAddress::IsMatchingType (result->first)))){





  //std::cout<<"batteryLVL is "<<m_source;
  oss << "type=request|nodeID="<<GetNode()->GetId()<<"|RequestId="<<packet_id<<"|R_node="<<rnd_node;
  oss<< "|R_servicetype=" << servicetype << "|";

        std::string lemessage = oss.str();

        Ptr<Packet> packet2 = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
        MyHeader sourceheader;
        sourceheader.SetData(1);
        packet2->AddHeader(sourceheader);
        std::cout<<" i am node" <<GetNode()->GetId() << " trying to send a request "<< packet_id <<" to node "
        		<< rnd_node<<" at time "<<Simulator::Now().GetSeconds () << std::endl;
        m_socket->Send (packet2);
        std::cout<<"packet sent ok "<<std::endl;

  ++m_sent;

  					 }else if (found && Inet6SocketAddress::IsMatchingType (result->first)){
  						std::cout<<packet_id<<" its in my neighbors lowpan" << std::endl;
  						//std::cout<<"found but" << Inet6SocketAddress::IsMatchingType(result->first);


  						      oss << "type=request|nodeID="<<GetNode()->GetId()<<"|RequestId="<<packet_id<<"|R_node="<<rnd_node;
  						        oss<<"|";

  						              std::string lemessage = oss.str();
  						            Ptr<Packet> packet2 = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
  						            MyHeader sourceheader;
  						            sourceheader.SetData(1);
  						            packet2->AddHeader(sourceheader);
  						       //packets6_list->push_back(packet2);
  						        NS_LOG_INFO ("Sending a----------------------------------------------- " <<result->first);
  					 }
  	    }
     // ScheduleRequest (10*m_interval);


}



std::vector<std::string> split(const std::string& s, char delimiter)
{
	 NS_LOG_INFO ("split func");

   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

int
RequestResponseClient::ParseServiceType(std::string msg){
	NS_LOG_FUNCTION (this);

	int type ;

	std::vector<std::string> fields = split(msg,'|');
	std::istringstream ( split(fields.at(4),'=')[1]) >> type;
return type;
}

neighbor RequestResponseClient::ParseMassage(std::string msg,Address from){
	 NS_LOG_FUNCTION (this);

	int id ;
	std::list<int> Nbors;
	double battery;
	int status;

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
	std::istringstream ( split(fields.at(3),'=')[1]) >> status;


	neighbor n (from,2,battery,Nbors,Simulator::Now ().GetSeconds (),id);
	n.status = status;
	std::cout<<" node "<<n.node_id<<"has status of " <<n.status<<" ok ok ok ok ok ok ok ok " <<std::endl;

return n;



}

void
RequestResponseClient::HandleRead (Ptr<Socket> socket)
{
  //NS_LOG_FUNCTION (this << socket);
	 NS_LOG_FUNCTION (this);
  Ptr<Packet> packet;
  Address from;
  std::string data;

  if (socket != 0)
   	    {

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

}

void
RequestResponseClient::SetPacketsv6List ( std::list<Ptr<Packet>> *t_list){
	packets6_list = t_list;
}

void
RequestResponseClient::ScheduleRequest (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
//  if (m_socket != 0)
//     	    {
//  Simulator::Schedule (dt, &RequestResponseClient::Request, this);
//
//     	    }

}

void
RequestResponseClient::SetRandomVariableGenerator (Ptr<UniformRandomVariable> gen)
{
  NS_LOG_FUNCTION (this);

  m_gen = gen;
}





} // Namespace ns3
