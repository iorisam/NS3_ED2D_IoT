#include "need-wifi-app-helper.h"
#include "ns3/need-wifi-app.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

NeedWifiAppHelper::NeedWifiAppHelper (uint16_t port)
{
  m_factory.SetTypeId (Need_WiFi_App::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

NeedWifiAppHelper::NeedWifiAppHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (Need_WiFi_App::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

NeedWifiAppHelper::NeedWifiAppHelper (Address address)
{
  m_factory.SetTypeId (Need_WiFi_App::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void
NeedWifiAppHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
NeedWifiAppHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NeedWifiAppHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NeedWifiAppHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
NeedWifiAppHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Need_WiFi_App> ();
  node->AddApplication (app);

  return app;
}



} // namespace ns3
