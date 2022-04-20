#include "need-prose-app-helper.h"
#include "ns3/need-prose-app.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

NeedProSeAppHelper::NeedProSeAppHelper (uint16_t port)
{
  m_factory.SetTypeId (Need_ProSe_App::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

NeedProSeAppHelper::NeedProSeAppHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (Need_ProSe_App::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

NeedProSeAppHelper::NeedProSeAppHelper (Address address)
{
  m_factory.SetTypeId (Need_ProSe_App::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void
NeedProSeAppHelper::SetAttribute (
  std::string name,
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
NeedProSeAppHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NeedProSeAppHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NeedProSeAppHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
NeedProSeAppHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Need_ProSe_App> ();
  node->AddApplication (app);

  return app;
}



} // namespace ns3
