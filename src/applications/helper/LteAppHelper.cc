/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "LteAppHelper.h"
#include "ns3/LteAppServer.h"
#include "ns3/LteAppClient.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

LteAppServerHelper::LteAppServerHelper (uint16_t port)
{
  m_factory.SetTypeId (LteAppServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void 
LteAppServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
LteAppServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LteAppServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LteAppServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
LteAppServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<LteAppServer> ();
  node->AddApplication (app);

  return app;
}

LteAppClientHelper::LteAppClientHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (LteAppClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

LteAppClientHelper::LteAppClientHelper (Address address)
{
  m_factory.SetTypeId (LteAppClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void 
LteAppClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
LteAppClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<LteAppClient>()->SetFill (fill);
}

void
LteAppClientHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<LteAppClient>()->SetFill (fill, dataLength);
}

void
LteAppClientHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<LteAppClient>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
LteAppClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LteAppClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
LteAppClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
LteAppClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<LteAppClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
