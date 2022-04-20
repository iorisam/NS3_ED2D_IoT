#ifndef NEED_WIFI_APP_H
#define NEED_WIFI_APP_H

#include <cstdint>
#include <list>
#include <map>
#include <string>
#include <vector>

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/neighbor.h"
#include <ns3/energy-module-lte.h>
#include <fstream>
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/energy-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4.h"
namespace ns3 {

class Socket;
class Packet;
struct Need_Message {
	   uint32_t sender_id;
     std::string message_type;
};

class Need_WiFi_App : public Application
{
public:

	 static TypeId GetTypeId (void);
	 Need_WiFi_App ();
	 virtual ~Need_WiFi_App ();
	 /**
	    * \brief set the remote address and port
	    * \param ip remote IP address
	    * \param port remote port
	    */
	   void SetRemote (Address ip, uint16_t port);
	   /**
	    * \brief set the remote address
	    * \param addr remote address
	    */
	   void SetRemote (Address addr);

	  // void Request (uint32_t emitter, uint32_t packetid);

	   /**
	    * Set the data size of the packet (the number of bytes that are sent as data
	    * to the server).  The contents of the data are set to unspecified (don't
	    * care) by this call.
	    *
	    * \warning If you have set the fill data for the echo client using one of the
	    * SetFill calls, this will undo those effects.
	    *
	    * \param dataSize The size of the echo data you want to sent.
	    */
	   void SetDataSize (uint32_t dataSize);

	   /**
	    * Get the number of data bytes that will be sent to the server.
	    *
	    * \warning The number of bytes may be modified by calling any one of the
	    * SetFill methods.  If you have called SetFill, then the number of
	    * data bytes will correspond to the size of an initialized data buffer.
	    * If you have not called a SetFill method, the number of data bytes will
	    * correspond to the number of don't care bytes that will be sent.
	    *
	    * \returns The number of data bytes.
	    */
	   uint32_t GetDataSize (void) const;

	   /**
	    * Set the data fill of the packet (what is sent as data to the server) to
	    * the zero-terminated contents of the fill string string.
	    *
	    * \warning The size of resulting echo packets will be automatically adjusted
	    * to reflect the size of the fill string -- this means that the PacketSize
	    * attribute may be changed as a result of this call.
	    *
	    * \param fill The string to use as the actual echo data bytes.
	    */
	   void SetFill (std::string fill);

	   /**
	    * Set the data fill of the packet (what is sent as data to the server) to
	    * the repeated contents of the fill byte.  i.e., the fill byte will be
	    * used to initialize the contents of the data packet.
	    *
	    * \warning The size of resulting echo packets will be automatically adjusted
	    * to reflect the dataSize parameter -- this means that the PacketSize
	    * attribute may be changed as a result of this call.
	    *
	    * \param fill The byte to be repeated in constructing the packet data..
	    * \param dataSize The desired size of the resulting echo packet data.
	    */
	   void SetFill (uint8_t fill, uint32_t dataSize);

	   /**
	    * Set the data fill of the packet (what is sent as data to the server) to
	    * the contents of the fill buffer, repeated as many times as is required.
	    *
	    * Initializing the packet to the contents of a provided single buffer is
	    * accomplished by setting the fillSize set to your desired dataSize
	    * (and providing an appropriate buffer).
	    *
	    * \warning The size of resulting echo packets will be automatically adjusted
	    * to reflect the dataSize parameter -- this means that the PacketSize
	    * attribute of the Application may be changed as a result of this call.
	    *
	    * \param fill The fill pattern to use when constructing packets.
	    * \param fillSize The number of bytes in the provided fill pattern.
	    * \param dataSize The desired size of the final echo data.
	    */
	   void SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize);
	   void SetRandomVariableGenerator (Ptr<UniformRandomVariable> gen);
	   Ptr<Socket> GetBroadcastSocket();


	 protected:
	   virtual void DoDispose (void);

	 private:

	   virtual void StartApplication (void);
	   virtual void StopApplication (void);

	   /**
	    * \brief Schedule the next packet transmission
	    * \param dt time interval between packets.
	    */

	   void ScheduleCheck (Time dt);
	   void ScheduleTransmit (Time dt);
	   void ScheduleRequest (Time dt);
	   void ScheduleRound (Time dt);
	   void ScheduleBroadcastTimeCalculation(Time dt);
	   void ScheduleMainBroadcast(Time dt);
	   void ScheduleWakeTimeBroadcast(Time dt);
	   void SetupOutSocket();
	   void SetupInSocket();
	   void BroadcastWakeTimeSchedule();
	   void ScheduleMainViceAlternation(Time dt,bool mainHead);
	   void ScheduleCondidateMessageBroadcast(Time dt);
	   void LogMe();
	   void Alternate();
	   void InitiateDuties();
	   Ipv4Address MacaNextHop(std::list<uint32_t> condidates);

	   /**
	    * \brief Send a packet
	    */
	   void Send (void);
	   void initiate_round (void);



	   /**
	    * \brief Handle a packet reception.
	    *
	    * This function is called by lower layers.
	    *
	    * \param socket the socket the packet was received to.
	    */
	   void HandleRead (Ptr<Socket> socket);
	   void HandleRequestRead (Ptr<Socket> socket);
	   void ManageNeighbors (Ptr<Socket> socket);
	   Need_Message ParseMassage(std::string data);
	   double ParseWakeUpTime(std::string data);
	   int ParseViceClusterHead(std::string data);
	   std::vector<std::string> split(const std::string& s, char delimiter);
	   void UpdateTable (void);
	   void updatestate();
	   bool contains(std::list<uint32_t> list1, uint32_t nodeId);
	   //void SendJoinMessageToCoveredNodes(std::map<Address,neighbor> covered);
	   double CalculateThreshold();

	   double CalculateCompetitionRadius(uint32_t n);
	   double CalculateDMax();
	   double CalculateDMin();
	   double CalculateBroadcastT();
	   double CalculateAverageEnergy();
	   void BroadcastCondidateMessage();
	   void BroadcastMainClusterHeadMessage();
	   void BroadcastQuitMessage();
	   void SendJoinMessage();
	   void CalculateBroadcastTime();
	   double Calculate_Relay_Probability(int NodeJ,std::list<uint32_t> condidates);
	   double Calculate_Maca_Heuristic(int NodeJ);
	   double Calculate_pheromone_concentration(int NodeJ);
	   int SelectBestNextHop(std::list<uint32_t> condidates);
	   double CalculateDistance(int nodeI,int  nodeJ);
	   void initilizeVaribales();
	   std::string ListToString(std::list<uint32_t> myList);
	   //bool LoadBalance();
	   template<class InputIterator, class T>
	   InputIterator my_find (InputIterator first, InputIterator last, const T& val);
	   //bool covered(int servicetype);
	   //bool allconnected(int servicetype);

	   uint32_t m_count; //!< Maximum number of packets the application will send
	   Time m_interval; //!< Packet inter-send time
	   uint32_t m_size; //!< Size of the sent packet

	   uint32_t m_dataSize; //!< packet payload size (must be equal to m_size)
	   uint8_t *m_data; //!< packet payload data

	   uint32_t m_sent; //!< Counter for sent packets
	   Ptr<Socket> m_socket; //!< Socket
	   Ptr<Socket> m_socket2;
	   Address m_peerAddress; //!< Remote peer address
	   uint16_t m_peerPort; //!< Remote peer port
	   EventId m_sendEvent; //!< Event to send the next packet
	   EventId m_RoundEvent; //!< Event to start the next round
	   EventId m_BTCalculationEvent; //!< Event to start the next round
	   EventId m_MainBroadcastEvent; //!< Event to start the next round
	   uint16_t if_index;
	   Ptr<UniformRandomVariable> m_gen;
	   uint32_t BS_location_x;
	   uint32_t BS_location_y;
	   Address m_local; //!< local multicast address

	   /// Callbacks for tracing the packet Tx events
	   TracedCallback<Ptr<const Packet> > m_txTrace;
	 public : std::map<Address,neighbor> MyTable;
	 Ptr<LiIonEnergySource> m_source;

	 public : std::string state;
	 public : int numberUE;
	 Callback<double,       uint32_t,   double> trace_sending_packet;
	 Callback<double,       uint32_t,   double> trace_receiving_packet;

	 std::list<uint32_t> neighbors_list;
	 std::list<uint32_t> main_heads_list;
	 std::map<uint32_t,uint32_t> vice_cluster_heads;

	 int* getClusterHead(void);

	 void SetSource (Ptr<LiIonEnergySource> source);
	 void LogStatus(void);

     // the percentage of the nodes in the
     // network that would ideally be cluster
     // heads during any one round of the
     // LEACH simulation, default is 0.05
	 double CLUSTER_PERCENT ;

     // the total rounds that the simulation
     // should run for - the network lifetime
     // default is 2000
	 int TOTAL_ROUNDS;

     // the total rounds that the simulation
     // should run for - the network lifetime
     // default is 2000
	 int ROUND_DURATION;

	 // the last round that the sensor
	 // served as a cluster head
	 int last_round_as_cluster_head;

	 // stores the index of the cluster head
	 // for the sensor to transmit to, set to -1 if the
	 // sensor is a cluster head
	  int cluster_head = -1;

	  // stores the index of the vice cluster head
	 // for the sensor to transmit to, set to -1 if the
	 // sensor is a cluster head
	  int vice_cluster_head = -1;

	  // stores the total number of nodes in
	 // the cluster, applicable only for
	 // cluster head nodes
	  int cluster_members;

	  // this contains the count of the
	  // number of times a sensor has been
	  // the head of a cluster, can be
	  // removed for optimization later
	  int head_count;

	  double wake_up_time = -2;

	  bool new_round = false;

	  //current round
	  int current_round;
	  int location_x,location_y;
	  double alpha = 0.4;
	  double beta = 0.6;
	  double ro = 0.3;
	  std::map<int,double> previous_pheromone_concentrations;
	  Need_Message MainHeadMsg;
	  bool main_head_incharge = true;

	 };

	 } // namespace ns3

	 #endif /* UDP_ECHO_CLIENT_H */
