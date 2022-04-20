/*
 * Astar.h
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
#include "Astar2.h"
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
NS_LOG_COMPONENT_DEFINE ("Astar2");

Astar::Astar(){

				NS_LOG_FUNCTION (this);
}
Astar::Astar(NodeContainer nodes,std::map<double,std::list<double>> m_list){
			    allnodes = nodes;
			    globallist= m_list;
				NS_LOG_FUNCTION (this);
}


bool Astar::isDestination(double x, double y) {
        if (x == y) {
            return true;
        }
        return false;
    }

double Astar::calculateH(int x, int y, int x2, int y2) {
    double H = (sqrt((x - x2)*(x - x2)
        + (y - y2)*(y - y2)));
    return H;
}

double Astar::getXofnode(double nodex) {
	Ptr<MobilityModel> UE_position =  allnodes.Get(nodex)->GetObject<MobilityModel>();
	return UE_position->GetPosition().x;
}

double Astar::getYofnode(double nodex) {
Ptr<MobilityModel> UE_position =  allnodes.Get(nodex)->GetObject<MobilityModel>();
	return UE_position->GetPosition().y;
}


double Astar::lowestf(double start,double dest, std::list<double> m_list) {
	double x;
    double f = 1000000;

    for(double i : m_list)
         {

    		double h = calculateH(getXofnode(i),getYofnode(i),getXofnode(dest),getYofnode(dest));
    		double g = calculateH(getXofnode(start),getYofnode(start),getXofnode(i),getYofnode(i));

    		if((g+h)<f){
    			f = g+h;
    			x = i;
    		}


         }

    return x;
}

void Astar::addtoopen( std::list<double> neighbors, std::list<double> openlist, std::list<double> closelist) {

    for(double it : neighbors)
         {
    		if((std::find(closelist.begin(),closelist.end(),it)) == closelist.end()){
    			openlist.push_front(it);
    		}
         }
}

std::list<double> Astar::getNeighbors(double i){
	std::list<double> neighbors={};
	std::map<double,std::list<double>>::iterator it;
	for( it= globallist.begin();it!=globallist.end();it++){
			if(it->first == i){
				if(!it->second.empty())
				neighbors = it->second;
			}
	}
	return neighbors;

}

bool Astar::findpath(double start, double dest ) {

	std::list<double> closelist;
	std::list<double> openlist;
	double x ;



	 if (isDestination(start, dest)) {

	            return true;
	            //You clicked on yourself
	        }else if (getNeighbors(start).size()<1) {

	        	return false;
	        }else {

	        	openlist.push_back(start);
	        	while(!openlist.empty()){

	        		 x = lowestf(x,dest,openlist);

	        		openlist.remove(x);
	        		closelist.push_back(x);
	        		 if (isDestination(x, dest)) {
	        			            return true;

	        			        }
	        		if(!getNeighbors(x).empty()){

	        			for(double it : getNeighbors(x))
	        			         {
	        			    		if((std::find(closelist.begin(),closelist.end(),it)) == closelist.end()){
	        			    			if(!getNeighbors(it).empty())
	        			    			openlist.push_back(it);
	        			    		}
	        			         }


	        		}
	        	}
	        	return false;
	        }

}



}

