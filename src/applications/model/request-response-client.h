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

#ifndef REQUEST_RESPONSE_CLIENT_H
#define REQUEST_RESPONSE_CLIENT_H

#include <cstdint>
#include <list>
#include <map>
#include <string>

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/neighbor.h"
#include <fstream>
#include <iostream>
#include "ns3/core-module.h"

#include "ns3/energy-module.h"
#include "ns3/li-ion-energy-source-helper.h"
#include "ns3/netanim-module.h"

namespace ns3 {

class Socket;
class Packet;


/**
 * \ingroup udpecho
 * \brief A Udp Echo client
 *
 * Every packet sent should be returned by the server and received here.
 */
class RequestResponseClient : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  RequestResponseClient ();

  virtual ~RequestResponseClient ();

  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Address ip, uint16_t port);
  /**
   * \brief set the remote address
   * \param addr remote address
   */
  void SetRemote (Address addr);

  /**
   * Set the data size of the packet (the number of bytes that are sent as data
   * to the server).  The contents of the data are set to unspecified (don't
   * care) by this call.
   *
   * \warning If you have set the fill data for the echo client using one of the
   * SetFill calls, this will undo those effects.
   *
   * \param dataSize The size of the echo data you want to sent.
   */
  void SetDataSize (uint32_t dataSize);

  /**
   * Get the number of data bytes that will be sent to the server.
   *
   * \warning The number of bytes may be modified by calling any one of the
   * SetFill methods.  If you have called SetFill, then the number of
   * data bytes will correspond to the size of an initialized data buffer.
   * If you have not called a SetFill method, the number of data bytes will
   * correspond to the number of don't care bytes that will be sent.
   *
   * \returns The number of data bytes.
   */
  uint32_t GetDataSize (void) const;

  /**
   * Set the data fill of the packet (what is sent as data to the server) to
   * the zero-terminated contents of the fill string string.
   *
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the size of the fill string -- this means that the PacketSize
   * attribute may be changed as a result of this call.
   *
   * \param fill The string to use as the actual echo data bytes.
   */
  void SetFill (std::string fill);

  /**
   * Set the data fill of the packet (what is sent as data to the server) to
   * the repeated contents of the fill byte.  i.e., the fill byte will be
   * used to initialize the contents of the data packet.
   *
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the dataSize parameter -- this means that the PacketSize
   * attribute may be changed as a result of this call.
   *
   * \param fill The byte to be repeated in constructing the packet data..
   * \param dataSize The desired size of the resulting echo packet data.
   */
  void SetFill (uint8_t fill, uint32_t dataSize);

  /**
   * Set the data fill of the packet (what is sent as data to the server) to
   * the contents of the fill buffer, repeated as many times as is required.
   *
   * Initializing the packet to the contents of a provided single buffer is
   * accomplished by setting the fillSize set to your desired dataSize
   * (and providing an appropriate buffer).
   *
   * \warning The size of resulting echo packets will be automatically adjusted
   * to reflect the dataSize parameter -- this means that the PacketSize
   * attribute of the Application may be changed as a result of this call.
   *
   * \param fill The fill pattern to use when constructing packets.
   * \param fillSize The number of bytes in the provided fill pattern.
   * \param dataSize The desired size of the final echo data.
   */
  void SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize);
  std::string state;

  TracedValue<uint32_t> n_packets = 0;
  std::list<Ptr<Packet>> *packets6_list;
  std::list<Ptr<Packet>> packets_list;
    void SetDevices (Ptr<NetDevice> device1,Ptr<NetDevice> device2);
    void SetRandomVariableGenerator (Ptr<UniformRandomVariable> gen);
      void SetPacketsv6List (std::list<Ptr<Packet>> *t_list);


      void Request (uint32_t rnd_node, uint32_t packet_id, int servicetype);
      int ParseServiceType(std::string msg);
protected:
  virtual void DoDispose (void);

public :
  virtual void StartApplication (void);
    virtual void StopApplication (void);

private:

  /**
   * \brief Schedule the next packet transmission
   * \param dt time interval between packets.
   */
  void ScheduleTransmit (Time dt);

  void ScheduleCheck (Time dt);


  void ScheduleRequest (Time dt);



  /**
   * \brief Send a packet
   */
  void Send (void);

  void UpdateTable (void);
  void updatestate();
  bool contains(std::list<int> list1,std::list<int> list2);
  bool allconnected();
  bool allconnected(int servicetype);
  bool LoadBalance();
  bool SelectBackup();
  int CalculateMaxSf(std::map<Address,neighbor> dominating_nodes, int coverage_degree);
  bool covered(std::list<int> my_nbrs);
  bool covered(int servicetype);
  std::map<Address, neighbor> Get_non__dominating_nodes();

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);


  void SendJoinMessageToCoveredNodes (std::map<Address,neighbor> covered);

  /**
   * checks the validity of neighbors table entries
   * give to each entry a time to live value after this time is exeeded the entry is deleted from the neighbor table
   */
  void ManageNeighbors (Ptr<Socket> socket);
  neighbor ParseMassage(std::string msg,Address from);

  uint32_t m_count; //!< Maximum number of packets the application will send

  uint32_t m_size; //!< Size of the sent packet

  uint32_t m_dataSize; //!< packet payload size (must be equal to m_size)
  uint8_t *m_data; //!< packet payload data

  uint32_t m_sent; //!< Counter for sent packets

  Ptr<Socket> m_socket;
  Ptr<Socket> m_socket2; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet
  std::list<Address> neighbors;
  TracedCallback<Ptr<const Packet> > m_txTrace;
public : std::map<Address,neighbor> MyTable;
Time m_interval; //!< Packet inter-send time
Ptr<NetDevice> m_device1;
Ptr<NetDevice> m_device2;
Ptr<UniformRandomVariable> m_gen;
int numberUE;
Callback<double,       uint32_t,   double> trace_sending_packet;
Ptr<DeviceEnergyModel> myEnergy;
Ptr<LiIonEnergySource> m_source;
TracedValue<uint32_t> m_myInt = 0;
int m_statusInt = 0;
AnimationInterface *anim;

std::list<int> services;
int cdsStates[5];

  /// Callbacks for tracing the packet Tx events

};

} // namespace ns3

#endif /* UDP_ECHO_CLIENT_H */
