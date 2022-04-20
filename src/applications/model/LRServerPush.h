/*
 * LRServerPush.h
 *
 *  Created on: Mar 23, 2019
 *      Author: lamboty
 */

#ifndef LRSERVERPUSH_H_
#define LRSERVERPUSH_H_
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/neighbor.h"
#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace ns3 {

class Socket;
class Packet;

class LRServerPush : public Application {
public:
	LRServerPush();
	virtual ~LRServerPush();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  void SetDevice (Ptr<NetDevice> device);
  void SetTable ( std::map<Address,neighbor> *table);

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);
  void ScheduleTransmit (Address from);
  void TransmitDiscoveryResponse(Address from);
  std::string ParseType(std::string msg);
  std::string ParseDest(std::string msg);
  std::vector<std::string> split(const std::string& s, char delimiter);
  neighbor ParseMassage(std::string msg,Address from);

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Ptr<Socket> m_socket; //!< IPv4 Socket
  Ptr<Socket> m_socket6; //!< IPv6 Socket
  Ptr<NetDevice> m_device;
  Address m_local; //!< local multicast address
  std::map<Address,neighbor> *MyTable;


};

}

#endif /* LRSERVERPUSH_H_ */
