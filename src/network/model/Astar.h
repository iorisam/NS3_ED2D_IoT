/*
 * Astar.h
 *
 *  Created on: Dec 2, 2017
 *      Author: lamboty
 */

#ifndef ASTAR_H
#define ASTAR_H

#include <list>

#include "ns3/address.h"
#include "neighbor.h"
#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"

#include "ns3/applications-module.h"


#include "ns3/internet-apps-module.h"
#include <ns3/config-store.h>




namespace ns3 {

class Astar
{

public:
	NodeContainer allnodes ;
	std::map<int,std::list<int>> globallist;
	Astar();
	Astar(NodeContainer nodes);
	bool isDestination(int x, int y);
	double calculateH(int x, int y, int x2, int y2);

	bool findpath(int start, int dest);
	int lowestf(int start,int dest, std::list<int> m_list);
	void addtoopen( std::list<int> neighbors,  std::list<int> openlist,  std::list<int> closelist);
	double getXofnode(int nodex);
	double getYofnode(int nodex);
	std::list<int> getNeighbors(int i);
};


}
#endif /* Astar */
