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

#include "lr-app-helper.h"
#include "ns3/lr-client.h"
#include "ns3/LRServer.h"
#include "ns3/uinteger.h"


namespace ns3
{
LRServerHelper::LRServerHelper (uint16_t port)
{
  m_factory.SetTypeId (LRServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void
LRServerHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
LRServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}



ApplicationContainer
LRServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
LRServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<LRServer> ();
  node->AddApplication (app);

  return app;
}


LrAppHelper::LrAppHelper ()
  : m_ifIndex (0)
{
  m_factory.SetTypeId (LRClient::GetTypeId ());
}

void LrAppHelper::SetLocal (Ipv6Address ip)
{
  m_localIp = ip;
}

void LrAppHelper::SetRemote (Ipv6Address ip)
{
  m_remoteIp = ip;
}

void LrAppHelper::SetAttribute (std::string name, const AttributeValue& value)
{
  m_factory.Set (name, value);
}

ApplicationContainer LrAppHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<LRClient> client = m_factory.Create<LRClient> ();
      client->SetLocal (m_localIp);
      client->SetRemote (m_remoteIp);
      client->SetIfIndex (m_ifIndex);
      client->SetRouters (m_routers);
      node->AddApplication (client);
      apps.Add (client);
    }
  return apps;
}

void LrAppHelper::SetIfIndex (uint32_t ifIndex)
{
  m_ifIndex = ifIndex;
}

void LrAppHelper::SetRoutersAddress (std::vector<Ipv6Address> routers)
{
  m_routers = routers;
}

} /* namespace ns3 */
