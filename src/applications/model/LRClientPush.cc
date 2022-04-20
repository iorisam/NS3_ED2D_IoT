#include "LRClientPush.h"
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

NS_LOG_COMPONENT_DEFINE ("Lr6ApplicationPush");

NS_OBJECT_ENSURE_REGISTERED (LRClientPush);


TypeId LRClientPush::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::LRClientPush")
    .SetParent<Application>()
    .SetGroupName("Applications")
    .AddConstructor<LRClientPush>()
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LRClientPush::m_count),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("Interval",
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&LRClientPush::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteIpv6",
                   "The Ipv6Address of the outbound packets",
                   Ipv6AddressValue (),
                   MakeIpv6AddressAccessor (&LRClientPush::m_peerAddress),
                   MakeIpv6AddressChecker ())
    .AddAttribute ("LocalIpv6",
                   "Local Ipv6Address of the sender",
                   Ipv6AddressValue (),
                   MakeIpv6AddressAccessor (&LRClientPush::m_localAddress),
                   MakeIpv6AddressChecker ())
    .AddAttribute ("PacketSize",
                   "Size of packets generated",
                   UintegerValue (100),
                   MakeUintegerAccessor (&LRClientPush::m_size),
                   MakeUintegerChecker<uint32_t>())
  ;
  return tid;
}

LRClientPush::LRClientPush ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_sent = 0;
  m_socket = 0;
  m_seq = 0;
  m_sendEvent = EventId ();
}

LRClientPush::~LRClientPush ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_socket = 0;
}

void LRClientPush::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  Application::DoDispose ();
}

void LRClientPush::StartApplication ()
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

void LRClientPush::SetLocal (Ipv6Address ipv6)
{
  NS_LOG_FUNCTION (this << ipv6);
  m_localAddress = ipv6;
}

void LRClientPush::SetRemote (Ipv6Address ipv6)
{
  NS_LOG_FUNCTION (this << ipv6);
  m_peerAddress = ipv6;
}

void LRClientPush::StopApplication ()
{
  NS_LOG_FUNCTION_NOARGS ();

  if (m_socket)
    {
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> >());
    }

  Simulator::Cancel (m_sendEvent);
}

void LRClientPush::SetIfIndex (uint32_t ifIndex)
{
  m_ifIndex = ifIndex;
}

void LRClientPush::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  double value;
       Ptr<UniformRandomVariable> uniform = CreateObject<UniformRandomVariable> ();
       value = uniform->GetInteger(12000,19999)/10000.0;
  m_sendEvent = Simulator::Schedule (Seconds(value), &LRClientPush::Send, this);
}

void LRClientPush::SetRouters (std::vector<Ipv6Address> routers)
{
  m_routers = routers;
}
void
LRClientPush::SetTable ( std::map<Address,neighbor> *table){
	MyTable = table;
}
void LRClientPush::Send ()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (m_sendEvent.IsExpired ());
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
    if(servicetype>0){
    //	std::cout<<"the type genrated for node "<<GetNode()->GetId()<<" is "<<servicetype<<std::endl;
    }else{
    	srand(Simulator::Now ().GetNanoSeconds());

    	    servicetype = (rand() % 4)+1 ;
    }

	 oss << "nodeID="<<GetNode()->GetId()<<"|batteryLvL=100.0"<<"|neighbors=|serviceType="<<servicetype<<"|";
	        std::string lemessage = oss.str();



  Ptr<Packet> p = Create<Packet> ((uint8_t*) lemessage.c_str(), lemessage.length());
//Icmpv6Echo req (1);
//
//  req.SetId (0xBEEF);
//  req.SetSeq (m_seq);
//  m_seq++;

  /* we do not calculate pseudo header checksum here, because we are not sure about 
   * source IPv6 address. Checksum is calculated in Ipv6RawSocketImpl.
   */

 // p->AddHeader (req);
  source->SetRecvCallback (MakeCallback (&LRClientPush::HandleRead, this));
NS_LOG_LOGIC ("trying to send a message lr pan");
source->SendTo (p, 0, sinkAdr);

 
  NS_LOG_INFO ("Sent " << p->GetSize () << " bytes to " <<sinkAdr);

  std::list<std::string>::iterator it2;
  for ( Ptr<Packet> packet_temp:packetsv6_list)
   	  {

	  source->SendTo (packet_temp, 0, sinkAdr);

   	  }

  packetsv6_list = {};
  std::cout<<" this is node lr "<<GetNode()->GetId()<<" at time "<<Simulator::Now ().GetSeconds ()<<std::endl;


      ScheduleTransmit (m_interval);
  m_sent++;
}

void
LRClientPush::SetDevice (Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION_NOARGS ();
 m_device = device;
}

void LRClientPush::HandleRead (Ptr<Socket> socket)
{
	 NS_LOG_FUNCTION (this << socket);
	  Ptr<Packet> packet;
	  Address from;
	  std::string data;
	  while ((packet = socket->RecvFrom (from)))
	    {
	      if (InetSocketAddress::IsMatchingType (from))
	        {
	          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
	                       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
	                       InetSocketAddress::ConvertFrom (from).GetPort ());
	          break;

	        }
	      else if (Inet6SocketAddress::IsMatchingType (from))
	        {
	          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client"<<GetNode()->GetId()<<" received " << packet->GetSize () << " bytes from " <<
	                       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
	                       Inet6SocketAddress::ConvertFrom (from).GetPort ());
                         uint8_t *buffer = new uint8_t[packet->GetSize()];

             	                   		packet->CopyData (buffer, packet->GetSize());

             	                   		data = std::string((char*)buffer);

             	                   	// std::cout<<"node  : " <<GetNode()->GetId() << "received response from node "<<data<<std::endl;
             	                   	break;
	        }

	    }
	  NS_LOG_INFO ("out of hundle read thank you"<<std::endl);


}

} /* namespace ns3 */

