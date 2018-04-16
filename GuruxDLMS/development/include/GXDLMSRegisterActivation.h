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

#ifndef GXDLMSREGISTERACTIVATION_H
#define GXDLMSREGISTERACTIVATION_H

#include "IGXDLMSBase.h"
#include "GXDLMSObject.h"
#include "GXHelpers.h"
#include "GXDLMSObjectDefinition.h"

/**
Online help:
http://www.gurux.fi/Gurux.DLMS.Objects.CGXDLMSRegisterActivation
*/
class CGXDLMSRegisterActivation : public CGXDLMSObject
{
    std::vector<CGXDLMSObjectDefinition> m_RegisterAssignment;
    std::vector<std::pair<CGXByteBuffer, CGXByteBuffer > > m_MaskList;
    CGXByteBuffer m_ActiveMask;

public:
    //Constructor.
    CGXDLMSRegisterActivation();

    //SN Constructor.
    CGXDLMSRegisterActivation(std::string ln, unsigned short sn);

    //LN Constructor.
    CGXDLMSRegisterActivation(std::string ln);

    std::vector<CGXDLMSObjectDefinition>& GetRegisterAssignment();

    std::vector<std::pair<CGXByteBuffer, CGXByteBuffer > >& GetMaskList();

    CGXByteBuffer& GetActiveMask();

    // Returns amount of attributes.
    int GetAttributeCount();

    // Returns amount of methods.
    int GetMethodCount();

    void GetValues(std::vector<std::string>& attributes);

    void GetAttributeIndexToRead(std::vector<int>& attributes);

    int GetDataType(int index, DLMS_DATA_TYPE& type);

    // Returns value of given attribute.
    int GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);

    // Set value of given attribute.
    int SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);
};
#endif //GXDLMSREGISTERACTIVATION_H