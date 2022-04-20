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
#include "lte-echo-helper.h"
#include "ns3/lte-echo-app.h"
#include "ns3/lte-echo-server.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

lte_echo_server_helper::lte_echo_server_helper (uint16_t port)
{
  m_factory.SetTypeId (lte_echo_server::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));

}

void 
lte_echo_server_helper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}
void
lte_echo_server_helper::SetIfIndex (uint16_t index)
{
  if_index = index;
  SetAttribute ("if_index", UintegerValue (if_index));
}

ApplicationContainer
lte_echo_server_helper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
lte_echo_server_helper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
lte_echo_server_helper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
lte_echo_server_helper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<lte_echo_server> ();
  node->AddApplication (app);

  return app;
}

lte_echo_client_helper::lte_echo_client_helper (Address address, uint16_t port)
{
  m_factory.SetTypeId (Lte_Echo_Client::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));

}

lte_echo_client_helper::lte_echo_client_helper (Address address)
{
  m_factory.SetTypeId (Lte_Echo_Client::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void 
lte_echo_client_helper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
lte_echo_client_helper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<Lte_Echo_Client>()->SetFill (fill);
}

void
lte_echo_client_helper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<Lte_Echo_Client>()->SetFill (fill, dataLength);
}

void
lte_echo_client_helper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<Lte_Echo_Client>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
lte_echo_client_helper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
lte_echo_client_helper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
lte_echo_client_helper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
lte_echo_client_helper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Lte_Echo_Client> ();
  node->AddApplication (app);

  return app;
}

void
lte_echo_client_helper::SetIfIndex (uint16_t index)
{
  if_index = index;
  SetAttribute ("if_index", UintegerValue (if_index));
}

} // namespace ns3
