//
// --------------------------------------------------------------------------
//  Gurux Ltd
//
//
//
// Filename:        $HeadURL$
//
// Version:         $Revision$,
//                  $Date$
//                  $Author$
//
// Copyright (c) Gurux Ltd
//
//---------------------------------------------------------------------------
//
//  DESCRIPTION
//
// This file is a part of Gurux Device Framework.
//
// Gurux Device Framework is Open Source software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; version 2 of the License.
// Gurux Device Framework is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// More information of Gurux products: http://www.gurux.org
//
// This code is licensed under the GNU General Public License v2.
// Full text may be retrieved at http://www.gnu.org/licenses/gpl-2.0.txt
//---------------------------------------------------------------------------

#include "../include/GXDLMSGSMCellInfo.h"
#include <sstream>

unsigned int CGXDLMSGSMCellInfo::GetCellId()
{
    return m_CellId;
}

void CGXDLMSGSMCellInfo::SetCellId(unsigned int value)
{
    m_CellId = value;
}

unsigned short CGXDLMSGSMCellInfo::GetLocationId()
{
    return m_LocationId;
}

void CGXDLMSGSMCellInfo::SetLocationId(unsigned short value)
{
    m_LocationId = value;
}

unsigned char CGXDLMSGSMCellInfo::GetSignalQuality()
{
    return m_SignalQuality;
}

void CGXDLMSGSMCellInfo::SetSignalQuality(unsigned char value)
{
    m_SignalQuality = value;
}

unsigned char CGXDLMSGSMCellInfo::GetBer()
{
    return m_Ber;
}

void CGXDLMSGSMCellInfo::SetBer(unsigned char value)
{
    m_Ber = value;
}

std::string CGXDLMSGSMCellInfo::ToString()
{
    std::stringstream sb;
    sb << m_CellId;
    sb << ", ";
    sb << m_LocationId;
    sb << ", ";
    sb << m_SignalQuality;
    sb << ", ";
    sb << m_Ber;
    return sb.str();
}