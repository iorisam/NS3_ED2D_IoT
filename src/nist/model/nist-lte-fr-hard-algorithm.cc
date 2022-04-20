/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Piotr Gawlowicz
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
 * Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *
 */

#include "nist-lte-fr-hard-algorithm.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NistLteFrHardAlgorithm");

NS_OBJECT_ENSURE_REGISTERED (NistLteFrHardAlgorithm);

static const struct FrHardDownlinkDefaultConfiguration
{
  uint8_t m_cellId;
  uint8_t m_dlBandwidth;
  uint8_t m_dlOffset;
  uint8_t m_dlSubBand;
} g_frHardDownlinkDefaultConfiguration[] = {
  { 1, 15, 0, 4},
  { 2, 15, 4, 4},
  { 3, 15, 8, 6},
  { 1, 25, 0, 8},
  { 2, 25, 8, 8},
  { 3, 25, 16, 9},
  { 1, 50, 0, 16},
  { 2, 50, 16, 16},
  { 3, 50, 32, 18},
  { 1, 75, 0, 24},
  { 2, 75, 24, 24},
  { 3, 75, 48, 27},
  { 1, 100, 0, 32},
  { 2, 100, 32, 32},
  { 3, 100, 64, 36}
};

static const struct FrHardUplinkDefaultConfiguration
{
  uint8_t m_cellId;
  uint8_t m_ulBandwidth;
  uint8_t m_ulOffset;
  uint8_t m_ulSubBand;
} g_frHardUplinkDefaultConfiguration[] = {
  { 1, 15, 0, 5},
  { 2, 15, 5, 5},
  { 3, 15, 10, 5},
  { 1, 25, 0, 8},
  { 2, 25, 8, 8},
  { 3, 25, 16, 9},
  { 1, 50, 0, 16},
  { 2, 50, 16, 16},
  { 3, 50, 32, 18},
  { 1, 75, 0, 24},
  { 2, 75, 24, 24},
  { 3, 75, 48, 27},
  { 1, 100, 0, 32},
  { 2, 100, 32, 32},
  { 3, 100, 64, 36}
};

const uint16_t NUM_DOWNLINK_CONFS (sizeof (g_frHardDownlinkDefaultConfiguration) / sizeof (FrHardDownlinkDefaultConfiguration));
const uint16_t NUM_UPLINK_CONFS (sizeof (g_frHardUplinkDefaultConfiguration) / sizeof (FrHardUplinkDefaultConfiguration));

NistLteFrHardAlgorithm::NistLteFrHardAlgorithm ()
  : m_ffrSapUser (0),
    m_ffrRrcSapUser (0),
    m_dlOffset (0),
    m_dlSubBand (0),
    m_ulOffset (0),
    m_ulSubBand (0)
{
  NS_LOG_FUNCTION (this);
  m_ffrSapProvider = new MemberNistLteFfrSapProvider<NistLteFrHardAlgorithm> (this);
  m_ffrRrcSapProvider = new MemberNistLteFfrRrcSapProvider<NistLteFrHardAlgorithm> (this);
}


NistLteFrHardAlgorithm::~NistLteFrHardAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}


void
NistLteFrHardAlgorithm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ffrSapProvider;
  delete m_ffrRrcSapProvider;
}


TypeId
NistLteFrHardAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NistLteFrHardAlgorithm")
    .SetParent<NistLteFfrAlgorithm> ()
    .AddConstructor<NistLteFrHardAlgorithm> ()
    .AddAttribute ("UlSubBandOffset",
                   "Uplink Offset in number of Resource Block Groups",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NistLteFrHardAlgorithm::m_ulOffset),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("UlSubBandwidth",
                   "Uplink Transmission SubBandwidth Configuration in number of Resource Block Groups",
                   UintegerValue (25),
                   MakeUintegerAccessor (&NistLteFrHardAlgorithm::m_ulSubBand),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlSubBandOffset",
                   "Downlink Offset in number of Resource Block Groups",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NistLteFrHardAlgorithm::m_dlOffset),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlSubBandwidth",
                   "Downlink Transmission SubBandwidth Configuration in number of Resource Block Groups",
                   UintegerValue (25),
                   MakeUintegerAccessor (&NistLteFrHardAlgorithm::m_dlSubBand),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}


void
NistLteFrHardAlgorithm::SetNistLteFfrSapUser (NistLteFfrSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrSapUser = s;
}


NistLteFfrSapProvider*
NistLteFrHardAlgorithm::GetNistLteFfrSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrSapProvider;
}

void
NistLteFrHardAlgorithm::SetNistLteFfrRrcSapUser (NistLteFfrRrcSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrRrcSapUser = s;
}


NistLteFfrRrcSapProvider*
NistLteFrHardAlgorithm::GetNistLteFfrRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrRrcSapProvider;
}


void
NistLteFrHardAlgorithm::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  NistLteFfrAlgorithm::DoInitialize ();

  NS_ASSERT_MSG (m_dlBandwidth > 14,"DlBandwidth must be at least 15 to use FFR algorithms");
  NS_ASSERT_MSG (m_ulBandwidth > 14,"UlBandwidth must be at least 15 to use FFR algorithms");

  if (m_frCellTypeId != 0)
    {
      SetDownlinkConfiguration (m_frCellTypeId, m_dlBandwidth);
      SetUplinkConfiguration (m_frCellTypeId, m_ulBandwidth);
    }

}

void
NistLteFrHardAlgorithm::Reconfigure ()
{
  NS_LOG_FUNCTION (this);
  if (m_frCellTypeId != 0)
    {
      SetDownlinkConfiguration (m_frCellTypeId, m_dlBandwidth);
      SetUplinkConfiguration (m_frCellTypeId, m_ulBandwidth);
    }
  InitializeDownlinkRbgMaps ();
  InitializeUplinkRbgMaps ();
  m_needReconfiguration = false;
}

void
NistLteFrHardAlgorithm::SetDownlinkConfiguration (uint16_t cellId, uint8_t bandwidth)
{
  NS_LOG_FUNCTION (this);
  for (uint16_t i = 0; i < NUM_DOWNLINK_CONFS; ++i)
    {
      if ((g_frHardDownlinkDefaultConfiguration[i].m_cellId == cellId)
          && g_frHardDownlinkDefaultConfiguration[i].m_dlBandwidth == m_dlBandwidth)
        {
          m_dlOffset = g_frHardDownlinkDefaultConfiguration[i].m_dlOffset;
          m_dlSubBand = g_frHardDownlinkDefaultConfiguration[i].m_dlSubBand;
        }
    }
}

void
NistLteFrHardAlgorithm::SetUplinkConfiguration (uint16_t cellId, uint8_t bandwidth)
{
  NS_LOG_FUNCTION (this);
  for (uint16_t i = 0; i < NUM_UPLINK_CONFS; ++i)
    {
      if ((g_frHardUplinkDefaultConfiguration[i].m_cellId == cellId)
          && g_frHardUplinkDefaultConfiguration[i].m_ulBandwidth == m_ulBandwidth)
        {
          m_ulOffset = g_frHardUplinkDefaultConfiguration[i].m_ulOffset;
          m_ulSubBand = g_frHardUplinkDefaultConfiguration[i].m_ulSubBand;
        }
    }
}

void
NistLteFrHardAlgorithm::InitializeDownlinkRbgMaps ()
{
  m_dlRbgMap.clear ();

  int rbgSize = GetRbgSize (m_dlBandwidth);
  m_dlRbgMap.resize (m_dlBandwidth / rbgSize, true);

  NS_ASSERT_MSG (m_dlOffset <= m_dlBandwidth,"DlOffset higher than DlBandwidth");
  NS_ASSERT_MSG (m_dlSubBand <= m_dlBandwidth,"DlBandwidth higher than DlBandwidth");
  NS_ASSERT_MSG ((m_dlOffset + m_dlSubBand) <= m_dlBandwidth,
                 "(DlOffset+DlSubBand) higher than DlBandwidth");

  for (uint8_t i = m_dlOffset / rbgSize; i < (m_dlOffset / rbgSize + m_dlSubBand / rbgSize); i++)
    {
      m_dlRbgMap[i] = false;

    }
}

void
NistLteFrHardAlgorithm::InitializeUplinkRbgMaps ()
{
  m_ulRbgMap.clear ();

  if (!m_enabledInUplink)
    {
      m_ulRbgMap.resize (m_ulBandwidth, false);
      return;
    }

  m_ulRbgMap.resize (m_ulBandwidth, true);

  NS_ASSERT_MSG (m_ulOffset <= m_ulBandwidth,"UlOffset higher than UlBandwidth");
  NS_ASSERT_MSG (m_ulSubBand <= m_ulBandwidth,"UlBandwidth higher than UlBandwidth");
  NS_ASSERT_MSG ((m_ulOffset + m_ulSubBand) <= m_ulBandwidth,
                 "(UlOffset+UlSubBand) higher than UlBandwidth");

  for (uint8_t i = m_ulOffset; i < (m_ulOffset + m_ulSubBand); i++)
    {
      m_ulRbgMap[i] = false;
    }
}

std::vector <bool>
NistLteFrHardAlgorithm::DoGetAvailableDlRbg ()
{
  NS_LOG_FUNCTION (this);

  if (m_needReconfiguration)
    {
      Reconfigure ();
    }

  if (m_dlRbgMap.empty ())
    {
      InitializeDownlinkRbgMaps ();
    }

  return m_dlRbgMap;
}

bool
NistLteFrHardAlgorithm::DoIsDlRbgAvailableForUe (int rbId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return !m_dlRbgMap[rbId];
}

std::vector <bool>
NistLteFrHardAlgorithm::DoGetAvailableUlRbg ()
{
  NS_LOG_FUNCTION (this);

  if (m_ulRbgMap.empty ())
    {
      InitializeUplinkRbgMaps ();
    }

  return m_ulRbgMap;
}

bool
NistLteFrHardAlgorithm::DoIsUlRbgAvailableForUe (int rbId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);

  if (!m_enabledInUplink)
    {
      return true;
    }

  return !m_ulRbgMap[rbId];
}

void
NistLteFrHardAlgorithm::DoReportDlCqiInfo (const struct NistFfMacSchedSapProvider::NistSchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
NistLteFrHardAlgorithm::DoReportUlCqiInfo (const struct NistFfMacSchedSapProvider::NistSchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
NistLteFrHardAlgorithm::DoReportUlCqiInfo (std::map <uint16_t, std::vector <double> > ulCqiMap)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

uint8_t
NistLteFrHardAlgorithm::DoGetTpc (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return 1; // 1 is mapped to 0 for Accumulated mode, and to -1 in Absolute mode TS36.213 Table 5.1.1.1-2
}

uint8_t
NistLteFrHardAlgorithm::DoGetMinContinuousUlBandwidth ()
{
  NS_LOG_FUNCTION (this);

  if (!m_enabledInUplink)
    {
      return m_ulBandwidth;
    }

  return m_ulSubBand;
}

void
NistLteFrHardAlgorithm::DoReportUeMeas (uint16_t rnti,
                                    NistLteRrcSap::NistMeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
NistLteFrHardAlgorithm::DoRecvLoadInformation (NistEpcX2Sap::NistLoadInformationParams params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

} // end of namespace ns3