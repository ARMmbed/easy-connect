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

#ifndef GXDLMSTCPUDPSETUP_H
#define GXDLMSTCPUDPSETUP_H

/**
Online help:
http://www.gurux.fi/Gurux.DLMS.Objects.CGXDLMSTcpUdpSetup
*/
class CGXDLMSTcpUdpSetup : public CGXDLMSObject
{
    int m_Port;
    std::string m_IPReference;
    int m_MaximumSimultaneousConnections;
    int m_InactivityTimeout;
    int m_MaximumSegmentSize;

public:
    /**
     Constructor.
    */
    CGXDLMSTcpUdpSetup();

    /**
     Constructor.

     @param ln Logical Name of the object.
    */
    CGXDLMSTcpUdpSetup(std::string ln);


    /**
     Constructor.

     @param ln Logical Name of the object.
     @param sn Short Name of the object.
    */
    CGXDLMSTcpUdpSetup(std::string ln, short sn);

    int GetPort();

    void SetPort(int value);

    std::string GetIPReference();

    void SetIPReference(std::string value);

    int GetMaximumSegmentSize();

    void SetMaximumSegmentSize(int value);

    int GetMaximumSimultaneousConnections();

    void SetMaximumSimultaneousConnections(int value);

    int GetInactivityTimeout();

    void SetInactivityTimeout(int value);

    // Returns amount of attributes.
    int GetAttributeCount();

    // Returns amount of methods.
    int GetMethodCount();

    //Get attribute values of object.
    void GetValues(std::vector<std::string>& values);

    void GetAttributeIndexToRead(std::vector<int>& attributes);

    int GetDataType(int index, DLMS_DATA_TYPE& type);

    // Returns value of given attribute.
    int GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);

    // Set value of given attribute.
    int SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);
};
#endif //GXDLMSTCPUDPSETUP_H