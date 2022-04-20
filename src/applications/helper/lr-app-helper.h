/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008-2009 Strasbourg University
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
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 */

#ifndef LR_APP_HELPER_H
#define LR_APP_HELPER_H

#include <stdint.h>

#include "ns3/object-factory.h"
#include "ns3/ipv6-address.h"

#include "ns3/application-container.h"
#include "ns3/node-container.h"

namespace ns3 {

class LRServerHelper
{
public:
  /**
   * Create UdpEchoServerHelper which will make life easier for people trying
   * to set up simulations with echos.
   *
   * \param port The port the server will wait on for incoming packets
   */
  LRServerHelper (uint16_t port);

  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Create a UdpEchoServerApplication on the specified Node.
   *
   * \param node The node on which to create the Application.  The node is
   *             specified by a Ptr<Node>.
   *
   * \returns An ApplicationContainer holding the Application created,
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a UdpEchoServerApplication on specified node
   *
   * \param nodeName The node on which to create the application.  The node
   *                 is specified by a node name previously registered with
   *                 the Object Name Service.
   *
   * \returns An ApplicationContainer holding the Application created.
   */


  /**
   * \param c The nodes on which to create the Applications.  The nodes
   *          are specified by a NodeContainer.
   *
   * Create one udp echo server application on each of the Nodes in the
   * NodeContainer.
   *
   * \returns The applications created, one Application per Node in the
   *          NodeContainer.
   */
  ApplicationContainer Install (NodeContainer c) const;

private:
  /**
   * Install an ns3::UdpEchoServer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an UdpEchoServer will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory; //!< Object factory.
};

/**
 * \ingroup ping6
 * \brief Ping6 application helper.
 */
class LrAppHelper
{
public:
  /**
   * \brief Constructor.
   */
  LrAppHelper ();

  /**
   * \brief Set the local IPv6 address.
   * \param ip local IPv6 address
   */
  void SetLocal (Ipv6Address ip);

  /**
   * \brief Set the remote IPv6 address.
   * \param ip remote IPv6 address
   */
  void SetRemote (Ipv6Address ip);

  /**
   * \brief Set some attributes.
   * \param name attribute name
   * \param value attribute value
   */
  void SetAttribute (std::string name, const AttributeValue& value);

  /**
   * \brief Install the application in Nodes.
   * \param c list of Nodes
   * \return application container
   */
  ApplicationContainer Install (NodeContainer c);

  /**
   * \brief Set the out interface index.
   * This is to send to link-local (unicast or multicast) address
   * when a node has multiple interfaces.
   * \param ifIndex interface index
   */
  void SetIfIndex (uint32_t ifIndex);


  /**
   * \brief Set routers addresses for routing type 0.
   * \param routers routers addresses
   */
  void SetRoutersAddress (std::vector<Ipv6Address> routers);

private:
  /**
   * \brief An object factory.
   */
  ObjectFactory m_factory;

  /**
   * \brief The local IPv6 address.
   */
  Ipv6Address m_localIp;

  /**
   * \brief The remote IPv6 address.
   */
  Ipv6Address m_remoteIp;

  /**
   * \brief Out interface index.
   */
  uint32_t m_ifIndex;

  /**
   * \brief Routers addresses.
   */
  std::vector<Ipv6Address> m_routers;
};

} /* namespace ns3 */

#endif /* PING6_HELPER_H */

