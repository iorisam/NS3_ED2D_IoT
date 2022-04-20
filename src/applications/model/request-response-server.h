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

#ifndef REQUEST_RESPONSE_SERVER_H
#define REQUEST_RESPONSE_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/energy-module.h"
#include "ns3/neighbor.h"
#include <cstdint>
#include <list>
#include <map>
#include <string>
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
class RequestResponseServer : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
	void SetSource ( Ptr<LiIonEnergySource> source);

	void SetTable ( std::map<Address,neighbor> *table);
	void SetTable ();
	void SetDevices (Ptr<NetDevice> device1,Ptr<NetDevice> device2);
	void Settype ( std::string type);
  static TypeId GetTypeId (void);
  RequestResponseServer ();
  void SetPacketsList (std::list<Ptr<Packet>> *p_list);
      void SetPacketsv6List (std::list<Ptr<Packet>> *t_list);

  virtual ~RequestResponseServer ();

protected:
  virtual void DoDispose (void);

public :
  virtual void StartApplication (void);
   virtual void StopApplication (void);
private:
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

void
  RemainingEnergy (double oldValue, double remainingEnergy);



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
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);
  Address ParseDistinationAddr(int id);
    bool ismyneighbor(int id);
    neighbor ParseMassage(std::string msg,Address from);
       std::string ParseType(std::string msg);
       int ParseDistinationNode(std::string msg);
       int ParseMessageId(std::string msg);
       template<class InputIterator, class T>
        InputIterator my_find (InputIterator first, InputIterator last, const T& val);
       std::vector<std::string> split(const std::string& s, char delimiter);

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Ptr<Socket> m_socket; //!< IPv4 Socket
  Ptr<Socket> m_socket6; //!< IPv6 Socket
  Address m_local; //!< local multicast address

  uint32_t m_count; //!< Maximum number of packets the application will send
  Time m_interval; //!< Packet inter-send time
  uint32_t m_size; //!< Size of the sent packet

  uint32_t m_dataSize; //!< packet payload size (must be equal to m_size)
  uint8_t *m_data; //!< packet payload data

  uint32_t m_sent; //!< Counter for sent packets

  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet

  double m_BatteryLvL;

  Ptr<LiIonEnergySource> m_source;
  std::map<Address,neighbor> *MyTable;
  std::string m_type;

  Ptr<Socket> m_socket2; //!< IPv4 Socket for the lte communication
  Ptr<NetDevice> m_device1;
  Ptr<NetDevice> m_device2;
  std::list<int> oldRequest;
  std::list<Ptr<Packet>> *packets_list;
  std::list<Ptr<Packet>>  *packets6_list;
  TracedValue<uint32_t> n_hops = 0;
  TracedValue<uint32_t> n_delivered = 0;
public :Callback<double,       uint32_t,   double> trace_receiving_packet;
int *status;


};

} // namespace ns3

#endif /* UDP_ECHO_SERVER_H */

