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

#include "../include/GXDLMSScheduleEntry.h"

unsigned char CGXDLMSScheduleEntry::GetIndex()
{
    return m_Index;
}

void CGXDLMSScheduleEntry::SetIndex(unsigned char value)
{
    m_Index = value;
}

bool CGXDLMSScheduleEntry::GetEnable()
{
    return m_Enable;
}

void CGXDLMSScheduleEntry::SetEnable(bool value)
{
    m_Enable = value;
}

std::string CGXDLMSScheduleEntry::GetLogicalName()
{
    return m_LogicalName;
}

void CGXDLMSScheduleEntry::SetLogicalName(std::string value)
{
    m_LogicalName = value;
}

unsigned char CGXDLMSScheduleEntry::GetScriptSelector()
{
    return m_ScriptSelector;
}


void CGXDLMSScheduleEntry::SetScriptSelector(unsigned char value)
{
    m_ScriptSelector = value;
}

CGXDateTime CGXDLMSScheduleEntry::GetSwitchTime()
{
    return m_SwitchTime;
}


void CGXDLMSScheduleEntry::SetSwitchTime(CGXDateTime value)
{
    m_SwitchTime = value;
}

unsigned char CGXDLMSScheduleEntry::GetValidityWindow()
{
    return m_ValidityWindow;
}

void CGXDLMSScheduleEntry::SetValidityWindow(unsigned char value)
{
    m_ValidityWindow = value;
}

std::string CGXDLMSScheduleEntry::GetExecWeekdays()
{
    return m_ExecWeekdays;
}

void CGXDLMSScheduleEntry::SetExecWeekdays(std::string value)
{
    m_ExecWeekdays = value;
}

std::string CGXDLMSScheduleEntry::GetExecSpecDays()
{
    return m_ExecSpecDays;
}

void CGXDLMSScheduleEntry::SetExecSpecDays(std::string value)
{
    m_ExecSpecDays = value;
}

CGXDateTime CGXDLMSScheduleEntry::GetBeginDate()
{
    return m_BeginDate;
}

void CGXDLMSScheduleEntry::SetBeginDate(CGXDateTime value)
{
    m_BeginDate = value;
}

CGXDateTime CGXDLMSScheduleEntry::GetEndDate()
{
    return m_EndDate;
}

void CGXDLMSScheduleEntry::SetEndDate(CGXDateTime value)
{
    m_EndDate = value;
}