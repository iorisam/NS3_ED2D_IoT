#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "NeedIPV6Tag.h"
#include <iostream>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NeedIPV6Tag);

	TypeId
	NeedIPV6Tag::GetTypeId (void)
	{
	  static TypeId tid = TypeId ("ns3::NeedIPV6Tag")
	    .SetParent<Tag> ()
	    .AddConstructor<NeedIPV6Tag> ()
	  ;
	  return tid;
	}
	TypeId
	NeedIPV6Tag::GetInstanceTypeId (void) const
	{
	  return GetTypeId ();
	}
	uint32_t
	NeedIPV6Tag::GetSerializedSize (void) const
	{
	  return 1;
	}
	void
	NeedIPV6Tag::Serialize (TagBuffer i) const
	{
	  i.WriteU8 (sourceNodeId);

	}
	void
	NeedIPV6Tag::Deserialize (TagBuffer i)
	{
		sourceNodeId = i.ReadU8 ();
	}
	void
	NeedIPV6Tag::Print (std::ostream &os) const
	{
	  os << "v=" << (uint32_t)sourceNodeId;
	}
	void
	NeedIPV6Tag::SetSourceNodeId (uint8_t value)
	{
		sourceNodeId = value;
	}
	uint8_t
	NeedIPV6Tag::GetSourceNodeId (void) const
	{
	  return sourceNodeId;
	}

}
