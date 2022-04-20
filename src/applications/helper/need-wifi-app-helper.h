
#ifndef NEED_WIFI_APP_HELPER_H
#define NEED_WIFI_APP_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/energy-module.h"

namespace ns3 {

/**
 * \ingroup RequestResponse
 * \brief Create a server application which waits for input UDP packets
 *        and sends them back to the original sender.
 */
class NeedWifiAppHelper
{
public:
  /**
   * Create RequestResponseServerHelper which will make life easier for people trying
   * to set up simulations with echos.
   *
   * \param port The port the server will wait on for incoming packets
   */
	NeedWifiAppHelper (uint16_t port);


	/**
	   * Create RequestResponseClientHelper which will make life easier for people trying
	   * to set up simulations with echos. Use this variant with addresses that do
	   * not include a port value (e.g., Ipv4Address and Ipv6Address).
	   *
	   * \param ip The IP address of the remote udp echo server
	   * \param port The port number of the remote udp echo server
	   */
	NeedWifiAppHelper (Address ip, uint16_t port);
	  /**
	   * Create RequestResponseClientHelper which will make life easier for people trying
	   * to set up simulations with echos. Use this variant with addresses that do
	   * include a port value (e.g., InetSocketAddress and Inet6SocketAddress).
	   *
	   * \param addr The address of the remote udp echo server
	   */
	NeedWifiAppHelper (Address addr);




  /**
   * Record an attribute to be set in each Application after it is is created.
   *
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set
   */
  void SetAttribute (std::string name, const AttributeValue &value);

  /**
   * Create a RequestResponseServerApplication on the specified Node.
   *
   * \param node The node on which to create the Application.  The node is
   *             specified by a Ptr<Node>.
   *
   * \returns An ApplicationContainer holding the Application created,
   */
  ApplicationContainer Install (Ptr<Node> node) const;

  /**
   * Create a RequestResponseServerApplication on specified node
   *
   * \param nodeName The node on which to create the application.  The node
   *                 is specified by a node name previously registered with
   *                 the Object Name Service.
   *
   * \returns An ApplicationContainer holding the Application created.
   */
  ApplicationContainer Install (std::string nodeName) const;

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
   * Install an ns3::RequestResponseServer on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an RequestResponseServer will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr<Application> InstallPriv (Ptr<Node> node) const;

  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* UDP_ECHO_HELPER_H */
