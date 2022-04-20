/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
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
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 *
 */

#ifndef DISC_APP_H
#define DISC_APP_H

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include <ns3/nist-lte-helper.h>
#include "ns3/neighbor.h"
#include <ns3/energy-module-lte.h>
#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/traced-callback.h"
#include <ns3/BloomFilter.hpp>
namespace ns3 {

/**
 * \ingroup DiscApp
 *
 * \brief A Udp client. Sends UDP packet carrying sequence number and time stamp
 *  in their payloads
 *
 */
class DiscApp : public Application
{
public:

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  DiscApp ();


  virtual ~DiscApp ();
  void SetTable ( std::map<Address,neighbor> *table);

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Send a packet
   */
  void Send (void);
  std::bitset<184>  generatenewname(void);
  void UpdateTable (uint64_t imsi, std::string discappcode,bloom_filter filter);
  void UpdateState (void);
  void Init_bloom();

  uint32_t m_count; //!< Maximum number of packets the application will send
  Time m_interval; //!< Packet inter-send time
  uint32_t m_size; //!< Size of the sent packet (including the SeqTsHeader)

  uint32_t m_sent; //!< Counter for sent packets
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet
  std::map<Address,neighbor> *MyTable;

  std::bitset<184> newname ;
  Ptr<NistLteHelper> m_lteHelper;
public : std::string *state;
bloom_filter m_filter;
void SetLteHelper (Ptr<NistLteHelper> h);
void NotifyDiscoveryMessage(uint64_t imsi, std::string discappcode,bloom_filter filter);
EnergyModuleLte *my_energy;
std::bitset<184> oldname ;


};

} // namespace ns3

#endif /* UDP_CLIENT_H */
