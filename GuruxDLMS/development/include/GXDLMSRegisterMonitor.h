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

#ifndef GXDLMSREGISTERMONITOR_H
#define GXDLMSREGISTERMONITOR_H

#include "GXDLMSMonitoredValue.h"
#include "GXDLMSActionSet.h"

/**
Online help:
http://www.gurux.fi/Gurux.DLMS.Objects.CGXDLMSRegisterMonitor
*/
class CGXDLMSRegisterMonitor : public CGXDLMSObject
{
    std::vector<CGXDLMSActionSet*> m_Actions;
    CGXDLMSMonitoredValue m_MonitoredValue;
    std::vector<CGXDLMSVariant> m_Thresholds;

public:
    /**
     Constructor.
    */
    CGXDLMSRegisterMonitor();

    /**
     destructor.
    */
    virtual ~CGXDLMSRegisterMonitor();

    /**
     Constructor.

     @param ln Logical Name of the object.
    */
    CGXDLMSRegisterMonitor(std::string ln);

    /**
     Constructor.

     @param ln Logical Name of the object.
     @param sn Short Name of the object.
    */
    CGXDLMSRegisterMonitor(std::string ln, unsigned short sn);

    std::vector<CGXDLMSVariant> GetThresholds();

    void SetThresholds(std::vector<CGXDLMSVariant> value);


    CGXDLMSMonitoredValue& GetMonitoredValue();

    void SetMonitoredValue(CGXDLMSMonitoredValue& value);

    std::vector<CGXDLMSActionSet*>& GetActions();

    // Returns amount of attributes.
    int GetAttributeCount();

    // Returns amount of methods.
    int GetMethodCount();

    //Get attribute values of object.
    void GetValues(std::vector<std::string>& values);

    void GetAttributeIndexToRead(std::vector<int>& attributes);

    int GetDataType(int index, DLMS_DATA_TYPE& type);

    /*
     * Returns value of given attribute.
     */
    int GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);

    /*
     * Set value of given attribute.
     */
    int SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);
};

#endif //GXDLMSREGISTERMONITOR_H
