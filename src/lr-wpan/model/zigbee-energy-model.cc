/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 *         Peishuo Li <pressthunder@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "zigbee-energy-model.h"
#include "lr-wpan-energy-source.h"

NS_LOG_COMPONENT_DEFINE ("ZigbeeEnergyModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ZigbeeEnergyModel);

TypeId
ZigbeeEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ZigbeeEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .AddConstructor<ZigbeeEnergyModel> ()
    .AddAttribute ("TrxOffCurrentA",
                   "The default radio Idle current in Ampere.",
                   DoubleValue (0.00000052),
                   MakeDoubleAccessor (&ZigbeeEnergyModel::SetTrxOffCurrentA,
                                       &ZigbeeEnergyModel::GetTrxOffCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxBusyCurrentA",
                   "The default radio Rx Busy with data receving current in Ampere.",
                   DoubleValue (0.0015),
                   MakeDoubleAccessor (&ZigbeeEnergyModel::SetRxBusyCurrentA,
                                       &ZigbeeEnergyModel::GetRxBusyCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxCurrentA",
                   "The radio Tx current.",
                   DoubleValue (0.007),
                   MakeDoubleAccessor (&ZigbeeEnergyModel::SetTxCurrentA,
                                       &ZigbeeEnergyModel::GetTxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxCurrentA",
                   "The radio Rx current.",
                   DoubleValue (0.0005),
                   MakeDoubleAccessor (&ZigbeeEnergyModel::SetRxCurrentA,
                                       &ZigbeeEnergyModel::GetRxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxBusyCurrentA",
                   "The default radio Tx Busy with data transmitting current in Ampere.",
                   DoubleValue (0.007),
                   MakeDoubleAccessor (&ZigbeeEnergyModel::SetTxBusyCurrentA,
                                       &ZigbeeEnergyModel::GetTxBusyCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TrxSwitchingCurrentA",
                   "The default radio Transceiver swiching between Tx/Rx current in Ampere.",
                   DoubleValue (0.0005),
                   MakeDoubleAccessor (&ZigbeeEnergyModel::SetTrxSwitchingCurrentA,
                                       &ZigbeeEnergyModel::GetTrxSwitchingCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TrxStartCurrentA",
                   "The default radio Transceiver swiching from off to Tx/Rx current in Ampere.",
                   DoubleValue (0.0005),
                   MakeDoubleAccessor (&ZigbeeEnergyModel::SetTrxStartCurrentA,
                                       &ZigbeeEnergyModel::GetTrxStartCurrentA),
                   MakeDoubleChecker<double> ())

    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&ZigbeeEnergyModel::m_totalEnergyConsumption))
    .AddTraceSource ("CurrentEnergyState",
                     "Current Phy layer state and corresponding energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&ZigbeeEnergyModel::m_EnergyStateLogger))
  ;
  return tid;
}

ZigbeeEnergyModel::ZigbeeEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  m_energyDepletionCallback.Nullify ();
  m_source = NULL;
  m_currentState = IEEE_802_15_4_PHY_TRX_OFF;
  m_sourceEnergyUnlimited = 0;
  m_remainingBatteryEnergy = 0;
  m_sourcedepleted = 0;
}

ZigbeeEnergyModel::~ZigbeeEnergyModel ()
{
  NS_LOG_FUNCTION (this);
}

void
ZigbeeEnergyModel::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
  m_energyToDecrease = 0;
  m_remainingBatteryEnergy = m_source->GetInitialEnergy();
  //m_sourceEnergyUnlimited = DynamicCast<LrWpanEnergySource> (m_source)->GetEnergyUnlimited ();
}

double
ZigbeeEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

double
ZigbeeEnergyModel::GetTrxOffCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_TrxOffCurrentA;
}

void
ZigbeeEnergyModel::SetTrxOffCurrentA (double TrxOffCurrentA)
{
  NS_LOG_FUNCTION (this << TrxOffCurrentA);
  m_TrxOffCurrentA = TrxOffCurrentA;
}

double
ZigbeeEnergyModel::GetRxBusyCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_RxBusyCurrentA;
}

void
ZigbeeEnergyModel::SetRxBusyCurrentA (double RxBusyCurrentA)
{
  NS_LOG_FUNCTION (this << RxBusyCurrentA);
  m_RxBusyCurrentA = RxBusyCurrentA;
}

double
ZigbeeEnergyModel::GetTxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_TxCurrentA;
}

void
ZigbeeEnergyModel::SetTxCurrentA (double txCurrentA)
{
  NS_LOG_FUNCTION (this << txCurrentA);
  m_TxCurrentA = txCurrentA;
}

double
ZigbeeEnergyModel::GetRxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_RxCurrentA;
}

void
ZigbeeEnergyModel::SetRxCurrentA (double rxCurrentA)
{
  NS_LOG_FUNCTION (this << rxCurrentA);
  m_RxCurrentA = rxCurrentA;
}

double
ZigbeeEnergyModel::GetTxBusyCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_TxBusyCurrentA;
}

void
ZigbeeEnergyModel::SetTxBusyCurrentA (double TxBusyCurrentA)
{
  NS_LOG_FUNCTION (this << TxBusyCurrentA);
  m_TxBusyCurrentA = TxBusyCurrentA;
}

double
ZigbeeEnergyModel::GetTrxSwitchingCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_TrxSwitchingCurrentA;
}

void
ZigbeeEnergyModel::SetTrxSwitchingCurrentA (double TrxSwitchingCurrentA)
{
  NS_LOG_FUNCTION (this << TrxSwitchingCurrentA);
  m_TrxSwitchingCurrentA = TrxSwitchingCurrentA;
}

double
ZigbeeEnergyModel::GetTrxStartCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_TrxStartCurrentA;
}

void
ZigbeeEnergyModel::SetTrxStartCurrentA (double TrxStartCurrentA)
{
  NS_LOG_FUNCTION (this << TrxStartCurrentA);
  m_TrxStartCurrentA = TrxStartCurrentA;
}

LrWpanPhyEnumeration
ZigbeeEnergyModel::GetCurrentState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void
ZigbeeEnergyModel::SetEnergyDepletionCallback (
  ZigbeeEnergyDepletionCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("ZigbeeEnergyModel:setting NULL energy depletion callback!");
    }
  m_energyDepletionCallback = callback;
}

void
ZigbeeEnergyModel::ChangeLrWpanState (Time t, LrWpanPhyEnumeration oldstate, LrWpanPhyEnumeration newstate)
{
  NS_LOG_FUNCTION (this << newstate);

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0); // check if duration is valid

  // energy to decrease = current * voltage * time
  if (newstate != IEEE_802_15_4_PHY_FORCE_TRX_OFF)
    {
      m_energyToDecrease = 0.0;
      double supplyVoltage = m_source->GetSupplyVoltage ();

      switch (m_currentState)
        {
        case IEEE_802_15_4_PHY_TRX_OFF:
          m_energyToDecrease = duration.GetSeconds () * m_TrxOffCurrentA * supplyVoltage;
          break;
        case IEEE_802_15_4_PHY_BUSY_RX:
          m_energyToDecrease = duration.GetSeconds () * m_RxBusyCurrentA * supplyVoltage;
          break;
        case IEEE_802_15_4_PHY_TX_ON:
          m_energyToDecrease = duration.GetSeconds () * m_TxCurrentA * supplyVoltage;
          break;
        case IEEE_802_15_4_PHY_RX_ON:
          m_energyToDecrease = duration.GetSeconds () * m_RxCurrentA * supplyVoltage;
          break;
        case IEEE_802_15_4_PHY_BUSY_TX:
          m_energyToDecrease = duration.GetSeconds () * m_TxBusyCurrentA * supplyVoltage;
          break;
        case IEEE_802_15_4_PHY_TRX_SWITCHING:
          m_energyToDecrease = duration.GetSeconds () * m_TrxSwitchingCurrentA * supplyVoltage;
          break;
        case IEEE_802_15_4_PHY_TRX_START:
          m_energyToDecrease = duration.GetSeconds () * m_TrxStartCurrentA * supplyVoltage;
          break;
        case IEEE_802_15_4_PHY_FORCE_TRX_OFF:
          m_energyToDecrease = 0;
          break;
        default:
          NS_FATAL_ERROR ("ZigbeeEnergyModel:Undefined radio state: " << m_currentState);
        }

      // update total energy consumption
      m_totalEnergyConsumption += m_energyToDecrease;

      // update last update time stamp
      m_lastUpdateTime = Simulator::Now ();
      // notify energy source
      std::string deviceName = m_source->GetInstanceTypeId ().GetName ();
      if (deviceName.compare ("ns3::LrWpanEnergySource") == 0)
        {
    	  Ptr<LrWpanEnergySource> lr_source = DynamicCast<LrWpanEnergySource>(m_source);
    	  lr_source->DecreaseRemainingEnergy(m_energyToDecrease);
        }
      m_source->UpdateEnergySource ();
     // std::cout<<" remaining energy is "<<m_source->GetRemainingEnergy();
    }

  if (!m_sourcedepleted)
    {
      SetLrWpanRadioState (newstate);
      NS_LOG_DEBUG ("ZigbeeEnergyModel:Total energy consumption is " <<
                    m_totalEnergyConsumption << "J");
    }
}

 void
 ZigbeeEnergyModel::ChangeState (int newState)
{
}

void
ZigbeeEnergyModel::HandleEnergyDepletion (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("ZigbeeEnergyModel:Energy is depleted!");
  // invoke energy depletion callback, if set.
  if (!m_energyDepletionCallback.IsNull ())
    {
      m_energyDepletionCallback ();
    }
  m_sourcedepleted = 1;
}

/*
 * Private functions start here.
 */

void
ZigbeeEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = NULL;
  m_energyDepletionCallback.Nullify ();
}


double
ZigbeeEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  switch (m_currentState)
    {
    case IEEE_802_15_4_PHY_TRX_OFF:
      return m_TrxOffCurrentA;
    case IEEE_802_15_4_PHY_BUSY_RX:
      return m_RxBusyCurrentA;
    case IEEE_802_15_4_PHY_TX_ON:
      return m_TxCurrentA;
    case IEEE_802_15_4_PHY_RX_ON:
      return m_RxCurrentA;
    case IEEE_802_15_4_PHY_BUSY_TX:
      return m_TxBusyCurrentA;
    case IEEE_802_15_4_PHY_TRX_SWITCHING:
      return m_TrxSwitchingCurrentA;
    case IEEE_802_15_4_PHY_TRX_START:
      return m_TrxStartCurrentA;
    case IEEE_802_15_4_PHY_FORCE_TRX_OFF:
      return 0;
    default:
      NS_FATAL_ERROR ("ZigbeeEnergyModel:Undefined radio state:" << m_currentState);
    }
}


void
ZigbeeEnergyModel::SetLrWpanRadioState (const LrWpanPhyEnumeration state)
{
  NS_LOG_FUNCTION (this << state);

  std::string preStateName;
  switch (m_currentState)
    {
    case IEEE_802_15_4_PHY_TRX_OFF:
      preStateName = "TRX_OFF";
      break;
    case IEEE_802_15_4_PHY_BUSY_RX:
      preStateName = "RX_BUSY";
      break;
    case IEEE_802_15_4_PHY_TX_ON:
      preStateName = "TX";
      break;
    case IEEE_802_15_4_PHY_RX_ON:
      preStateName = "RX";
      break;
    case IEEE_802_15_4_PHY_BUSY_TX:
      preStateName = "TX_BUSY";
      break;
    case IEEE_802_15_4_PHY_TRX_SWITCHING:
      preStateName = "TRX_SWITCH";
      break;
    case IEEE_802_15_4_PHY_TRX_START:
      preStateName = "TRX_START";
      break;
    case IEEE_802_15_4_PHY_FORCE_TRX_OFF:
      preStateName = "TRX_FORCE_OFF";
      break;
  default:
    NS_FATAL_ERROR ("ZigbeeEnergyModel:Undefined radio state: " << m_currentState);
  }

  m_currentState = state;
  std::string curStateName;
  switch (state)
    {
    case IEEE_802_15_4_PHY_TRX_OFF:
      curStateName = "TRX_OFF";
      break;
    case IEEE_802_15_4_PHY_BUSY_RX:
      curStateName = "RX_BUSY";
      break;
    case IEEE_802_15_4_PHY_TX_ON:
      curStateName = "TX";
      break;
    case IEEE_802_15_4_PHY_RX_ON:
      curStateName = "RX";
      break;
    case IEEE_802_15_4_PHY_BUSY_TX:
      curStateName = "TX_BUSY";
      break;
    case IEEE_802_15_4_PHY_TRX_SWITCHING:
      curStateName = "TRX_SWITCH";
      break;
    case IEEE_802_15_4_PHY_TRX_START:
      curStateName = "TRX_START";
      break;
    case IEEE_802_15_4_PHY_FORCE_TRX_OFF:
      curStateName = "TRX_FORCE_OFF";
      break;
  default:
    NS_FATAL_ERROR ("ZigbeeEnergyModel:Undefined radio state: " << m_currentState);
  }

  m_remainingBatteryEnergy = m_source -> GetRemainingEnergy();

  m_EnergyStateLogger (preStateName, curStateName, m_sourceEnergyUnlimited, m_energyToDecrease, m_remainingBatteryEnergy, m_totalEnergyConsumption);

  NS_LOG_DEBUG ("ZigbeeEnergyModel:Switching to state: " << curStateName <<
                " at time = " << Simulator::Now ());
}


// -------------------------------------------------------------------------- //


/*
 * Private function state here.
 */



} // namespace ns3
