#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "mytag.h"
#include <iostream>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ProSeTag);

	TypeId
	ProSeTag::GetTypeId (void)
	{
	  static TypeId tid = TypeId ("ns3::ProSeTag")
	    .SetParent<Tag> ()
	    .AddConstructor<ProSeTag> ()
	  ;
	  return tid;
	}
	TypeId
	ProSeTag::GetInstanceTypeId (void) const
	{
	  return GetTypeId ();
	}
	uint32_t
	ProSeTag::GetSerializedSize (void) const
	{
	  return 1;
	}
	void
	ProSeTag::Serialize (TagBuffer i) const
	{
	  i.WriteU8 (m_simpleValue);
	}
	void
	ProSeTag::Deserialize (TagBuffer i)
	{
	  m_simpleValue = i.ReadU8 ();
	}
	void
	ProSeTag::Print (std::ostream &os) const
	{
	  os << "v=" << (uint32_t)m_simpleValue;
	}
	void
	ProSeTag::SetSimpleValue (uint8_t value)
	{
	  m_simpleValue = value;
	}
	uint8_t
	ProSeTag::GetSimpleValue (void) const
	{
	  return m_simpleValue;
	}
}
