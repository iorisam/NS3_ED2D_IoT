/*
 * Astar.h
 *
 *  Created on: Dec 2, 2017
 *      Author: lamboty
 */

#ifndef ASTAR2_H
#define ASTAR2_H

#include <list>

#include "ns3/address.h"
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
	std::map<double,std::list<double>> globallist;
	Astar();
	Astar(NodeContainer nodes,std::map<double,std::list<double>> m_list);
	bool isDestination(double x, double y);
	double calculateH(int x, int y, int x2, int y2);

	bool findpath(double start, double dest);
	double lowestf(double start,double dest, std::list<double> m_list);
	void addtoopen( std::list<double> neighbors,  std::list<double> openlist,  std::list<double> closelist);
	double getXofnode(double nodex);
	double getYofnode(double nodex);
	std::list<double> getNeighbors(double i);
};


}
#endif /* Astar */
