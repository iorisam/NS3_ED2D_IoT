/*
 * LRClientPush.h
 *
 *  Created on: Mar 23, 2019
 *      Author: lamboty
 */

#ifndef LRCLIENTPUSH_H_
#define LRCLIENTPUSH_H_

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv6-address.h"
#include "ns3/neighbor.h"
#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace ns3
{

class Socket;
class Packet;


class LRClientPush : public Application {

public:
static TypeId GetTypeId ();
void SetDevice (Ptr<NetDevice> device);
	LRClientPush();
	virtual ~LRClientPush();

void SetLocal (Ipv6Address ipv6);

  /**
   * \brief Set the remote peer.
   * \param ipv6 IPv6 address of the peer
   */
  void SetRemote (Ipv6Address ipv6);

  /**
   * \brief Set the out interface index.
   * This is to send to link-local (unicast or multicast) address
   * when a node has multiple interfaces.
   * \param ifIndex interface index
   */
  void SetIfIndex (uint32_t ifIndex);

  /**
   * \brief Set routers for routing type 0 (loose routing).
   * \param routers routers addresses
   */
  void SetRouters (std::vector<Ipv6Address> routers);

  void SetTable ( std::map<Address,neighbor> *table);

  neighbor ParseMassage(std::string msg,Address from);

  std::vector<std::string> split(const std::string& s, char delimiter);
  int servicetype = -1;
protected:
  /**
   * \brief Dispose this object;
   */
  virtual void DoDispose ();

private:
  /**
   * \brief Start the application.
   */
  virtual void StartApplication ();

  /**
   * \brief Stop the application.
   */
  virtual void StopApplication ();

  /**
   * \brief Schedule sending a packet.
   * \param dt interval between packet
   */
  void ScheduleTransmit (Time dt);

  /**
   * \brief Send a packet.
   */
  void Send ();

  /**
   * \brief Receive method.
   * \param socket socket that receive a packet
   */
  void HandleRead (Ptr<Socket> socket);

  /**
   * \brief Peer IPv6 address.
   */
  Ipv6Address m_address;

  /**
   * \brief Number of "Echo request" packets that will be sent.
   */
  uint32_t m_count;

  /**
   * \brief Number of packets sent.
   */
  uint32_t m_sent;

  /**
   * \brief Size of the packet.
   */
  uint32_t m_size;

  /**
   * \brief Intervall between packets sent.
   */
  Time m_interval;

  /**
   * \brief Local address.
   */
  Ipv6Address m_localAddress;

  /**
   * \brief Peer address.
   */
  Ipv6Address m_peerAddress;

  /**
   * \brief Local socket.
   */
  Ptr<Socket> m_socket;

  /**
   * \brief Sequence number.
   */
  uint16_t m_seq;

  /**
   * \brief Event ID.
   */
  EventId m_sendEvent;

  /**
   * \brief Out interface (i.e. for link-local communication).
   */
  uint32_t m_ifIndex;

  /**
   * \brief Routers addresses for routing type 0.
   */
  std::vector<Ipv6Address> m_routers;

  std::map<Address,neighbor> *MyTable;
  Ptr<NetDevice> m_device;
	public : std::list<Ptr<Packet>> packetsv6_list;

};
}/* namespace ns3 */

#endif /* LRCLIENTPUSH_H_ */
