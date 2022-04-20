#include "NeedLRClient.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/uinteger.h"
#include "ns3/ipv6.h"
#include "ns3/ipv6-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/icmpv6-header.h"
#include "ns3/ipv6-raw-socket-factory.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv6-extension-header.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/random-variable-stream.h"
#include <chrono>



namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("NeedLRClientApp");

NS_OBJECT_ENSURE_REGISTERED (NeedLRClient);


TypeId NeedLRClient::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NeedLRClient")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<NeedLRClient>()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&NeedLRClient::m_count),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&NeedLRClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteIpv6",
                   "The Ipv6Address of the outbound packets",
                   Ipv6AddressValue (),
                   MakeIpv6AddressAccessor (&NeedLRClient::m_peerAddress),
                   MakeIpv6AddressChecker ())
    .AddAttribute ("LocalIpv6",
                   "Local Ipv6Address of the sender",
                   Ipv6AddressValue (),
                   MakeIpv6AddressAccessor (&NeedLRClient::m_localAddress),
                   MakeIpv6AddressChecker ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated",
                   UintegerValue (100),
                   MakeUintegerAccessor (&NeedLRClient::m_size),
                   MakeUintegerChecker<uint32_t>())
  ;
  return tid;
}

NeedLRClient::NeedLRClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sent = 0;
  m_socket = 0;
  m_seq = 0;
  m_sendEvent = EventId ();
}

NeedLRClient::~NeedLRClient ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

void NeedLRClient::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void NeedLRClient::StartApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (!m_socket)
    {
      TypeId tid = TypeId::LookupByName ("ns3::Ipv6RawSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
         Inet6SocketAddress sourceAdr = Inet6SocketAddress (Ipv6Address::GetAny(), 9);


      NS_ASSERT (m_socket);


//      m_socket->Bind (Inet6SocketAddress (m_localAddress, 0));
//      m_socket->SetAttribute ("Protocol", UintegerValue (Ipv6Header::IPV6_ICMPV6));
//      m_socket->SetRecvCallback (MakeCallback (&LRClientPush::HandleRead, this));
    }
  packetsv6_list = {};
  ScheduleTransmit (Seconds (0.));
}

void NeedLRClient::SetLocal (Ipv6Address ipv6)
{
  NS_LOG_FUNCTION (this << ipv6);
  m_localAddress = ipv6;
}

void NeedLRClient::SetRemote (Ipv6Address ipv6)
{
  NS_LOG_FUNCTION (this << ipv6);
  m_peerAddress = ipv6;
}

void NeedLRClient::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> >());
    }

  Simulator::Cancel (m_sendEvent);
}

void NeedLRClient::SetIfIndex (uint32_t ifIndex)
{
  m_ifIndex = ifIndex;
}

void NeedLRClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  NS_LOG_INFO("hello sam info"<<m_interval);
  Simulator::Schedule (dt, &NeedLRClient::Send, this);
}

void NeedLRClient::SetRouters (std::vector<Ipv6Address> routers)
{
  m_routers = routers;
}
void
NeedLRClient::SetTable ( std::map<Address,neighbor> *table){
	MyTable = table;
}
void NeedLRClient::Send ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO("inside needlr client send function");
  Inet6SocketAddress sinkAdr = Inet6SocketAddress (Ipv6Address("FF02::2"), 9);

  TypeId tid = UdpSocketFactory::GetTypeId();
  Ptr<Socket> source = Socket::CreateSocket(GetNode(), tid);
    Inet6SocketAddress sourceAdr = Inet6SocketAddress (Ipv6Address::GetAny(), 9);

  	source->SetIpv6Tclass (0);

    source->SetIpv6HopLimit (0);
    source->SetRecvPktInfo(true);
    source->SetIpv6RecvHopLimit(true);
    source->Connect(sourceAdr);
    source->BindToNetDevice(m_device);

    source->SetAllowBroadcast(true);

    std::ostringstream oss;
	 oss << "nodeID="<<GetNode()->GetId()<<"|batteryLvL=100.0"<<"|neighbors=|";
	        std::string lemessage = oss.str();
  Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
  source->SetRecvCallback (MakeCallback (&NeedLRClient::HandleRead, this));
  NS_LOG_LOGIC ("trying to send a message lr pan");
  source->SendTo (p, 0, sinkAdr);

  NS_LOG_INFO ("Sent " << p->GetSize () << " bytes to " <<sinkAdr);
  ScheduleTransmit (m_interval);
  m_sent++;
}

uint32_t NeedLRClient::GetNSentPackets(){
	return m_sent;
}

void
NeedLRClient::SetDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_device = device;
}

void NeedLRClient::HandleRead (Ptr<Socket> socket)
{
	 NS_LOG_FUNCTION (this << socket);
}

} /* namespace ns3 */

