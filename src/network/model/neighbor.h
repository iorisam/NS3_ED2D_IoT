/*
 * neighbor.h
 *
 *  Created on: Dec 2, 2017
 *      Author: lamboty
 */

#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include <list>

#include "ns3/address.h"



namespace ns3 {

class neighbor
{

public:

	neighbor();
	neighbor(Address a,int db,int battery,std::list<int> N,double time,int id);


	 	Address peer_address;
		int signal_strength;
		int battery;
		std::list<int>neighbors;
		double discovered_time;
		int node_id;
		int status;
		double mobility;
		int servicetype = 0;

};


}
#endif /* NEIGHBOR_H_ */
