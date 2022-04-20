/*
 * lrapppushhelper.cc
 *
 *  Created on: Mar 23, 2019
 *      Author: lamboty
 */

#include "LRPushAppHelper.h"
#include "ns3/LRClientPush.h"
#include "ns3/LRServerPush.h"
#include "ns3/uinteger.h"


namespace ns3
{

LrServerPushHelper::LrServerPushHelper (uint16_t port)
{
  m_factory.SetTypeId (LRServerPush::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
LrServerPushHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
LrServerPushHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}



ApplicationContainer
LrServerPushHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
LrServerPushHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<LRServerPush> ();
  node->AddApplication (app);

  return app;
}


LrClientPushHelper::LrClientPushHelper ()
  : m_ifIndex (0)
{
  m_factory.SetTypeId (LRClientPush::GetTypeId ());
}

void LrClientPushHelper::SetLocal (Ipv6Address ip)
{
  m_localIp = ip;
}

void LrClientPushHelper::SetRemote (Ipv6Address ip)
{
  m_remoteIp = ip;
}

void LrClientPushHelper::SetAttribute (std::string name, const AttributeValue& value)
{
  m_factory.Set (name, value);
}

ApplicationContainer LrClientPushHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<LRClientPush> client = m_factory.Create<LRClientPush> ();
      client->SetLocal (m_localIp);
      client->SetRemote (m_remoteIp);
      client->SetIfIndex (m_ifIndex);
      client->SetRouters (m_routers);
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

void LrClientPushHelper::SetIfIndex (uint32_t ifIndex)
{
  m_ifIndex = ifIndex;
}

void LrClientPushHelper::SetRoutersAddress (std::vector<Ipv6Address> routers)
{
  m_routers = routers;
}

}

