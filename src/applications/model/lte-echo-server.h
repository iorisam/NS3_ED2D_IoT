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

#ifndef LTE_ECHO_SERVERR_H
#define LTE_ECHO_SERVERR_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include <ns3/nist-lte-helper.h>
#include <ns3/nist-module.h>
#include "ns3/neighbor.h"
#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>

#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/applications-module.h"

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup udpecho UdpEcho
 */

/**
 * \ingroup udpecho
 * \brief A Udp Echo server
 *
 * Every packet received is sent back.
 */
class lte_echo_server : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  lte_echo_server ();
  void SetHelper( Ptr<NistLteProseHelper> Helper);
  virtual ~lte_echo_server ();
  void SetTable ( std::map<Address,neighbor> *table);

    neighbor ParseMassage(std::string msg,Address from);
    std::string ParseType(std::string msg);
    int ParseDistinationNode(std::string msg);
    int ParseMessageId(std::string msg);
    template<class InputIterator, class T>
     InputIterator my_find (InputIterator first, InputIterator last, const T& val);
    std::vector<std::string> split(const std::string& s, char delimiter);
    void SetBroadcastAddress (Address m_Address);
    void SetBroadcastSocket ( Ptr<Socket> socket);

    void SetLteClient ( Ptr<Lte_Echo_Client> *client);
    void SetState ( std::string *state);
    void SetPacketsList (std::list<Ptr<Packet>> *p_list);
    void SetNeighborsPacketsList (std::list<Ptr<Packet>> *p_list);
    void SetPacketsv6List (std::list<Ptr<Packet>> *t_list);
    void RssiCallback(uint16_t rnti, double rssi);
    uint16_t ParseMessageSender(std::string msg);
    double ParseTxPowerFromMessage(std::string msg);
    bool ParseMessageCover(std::string msg);
    void updateTxPower(double txPower);
    TracedValue<uint32_t> n_hops = 0;
    TracedValue<uint32_t> n_delivered = 0;
protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void ScheduleCheck (Time dt);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);
  void HandleRead2 (Ptr<Socket> socket);
  Address ParseDistinationAddr(int id);
  bool ismyneighbor(int id);
  void updatesocket();
  bool exisits_in_neighbors_request(int msg_id);


  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Ptr<Socket> m_socket; //!< IPv4 Socket
  Ptr<Socket> m_socket2;
  Ptr<Socket> m_socket6; //!< IPv6 Socket
  Address m_local; //!< local multicast address
  Ptr<NistLteProseHelper> m_proseHelper;
  uint16_t if_index;
  std::map<Address,neighbor> *MyTable;
  std::list<Ptr<Packet>> *packets_list;
  std::list<Ptr<Packet>> *neighbors_packets_list;
  std::list<Ptr<Packet>>  *packets6_list;
  Address m_peerAddress; //!< Remote peer address
  std::list<int> oldRequest;
  Ptr<Lte_Echo_Client> *m_client;
  std::string *m_state;
public :Callback<double,       uint32_t,   double> trace_receiving_packet;
};

} // namespace ns3

#endif /* UDP_ECHO_SERVER_H */

