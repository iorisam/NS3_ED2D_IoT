/*
 * neighbor.h
 *
 *  Created on: Dec 2, 2017
 *      Author: lamboty
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
#include "neighbor.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("Neighbor");

neighbor::neighbor(){

				signal_strength= 2;
				this->battery = 0;

				discovered_time=0;
				NS_LOG_FUNCTION (this);
}

neighbor::neighbor(Address a,int db,int battery,std::list<int> N,double time,int id) : neighbor(){
	peer_address = a;
			signal_strength= db;
			this->battery = battery;
			neighbors=N;
			discovered_time=time;
			node_id = id;
}




}

