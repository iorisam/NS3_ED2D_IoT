#ifndef NEED_IPV6_TAG_H
#define NEED_IPV6_TAG_H

#include "ns3/tag.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/inet6-socket-address.h"
#include <iostream>

namespace ns3 {

// define this class in a public header
class NeedIPV6Tag : public Tag
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  // these are our accessors to our tag structure
  void SetSourceNodeId (uint8_t value);
  uint8_t GetSourceNodeId (void) const;


private:
  uint8_t sourceNodeId;
};
}
#endif
