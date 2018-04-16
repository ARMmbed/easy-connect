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

#ifndef GXDLMSMESSAGEHANDLER_H
#define GXDLMSMESSAGEHANDLER_H

#include "GXDLMSObject.h"
#include "GXDateTime.h"
#include "GXDLMSScriptAction.h"

/**
Online help:
http://www.gurux.fi/Gurux.DLMS.Objects.CGXDLMSMessageHandler
*/
class CGXDLMSMessageHandler : public CGXDLMSObject
{
    std::vector<std::pair<CGXDateTime, CGXDateTime> > m_ListeningWindow;
    std::vector<std::string> m_AllowedSenders;
    std::vector<std::pair<std::string, std::pair<int, CGXDLMSScriptAction> > > m_SendersAndActions;
public:
    //Constructor.
    CGXDLMSMessageHandler();

    //SN Constructor.
    CGXDLMSMessageHandler(std::string ln, unsigned short sn);

    //LN Constructor.
    CGXDLMSMessageHandler(std::string ln);

    /**
    Listening Window.
    */
    std::vector<std::pair<CGXDateTime, CGXDateTime> >& GetListeningWindow();

    /**
     List of allowed Senders.
    */
    std::vector<std::string>& GetAllowedSenders();

    /**
     Contains the logical name of a "Script table" object and the script selector of the
     script to be executed if an empty message is received from a match-ing sender.
    */
    std::vector<std::pair<std::string, std::pair<int, CGXDLMSScriptAction> > >& GetSendersAndActions();

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

#endif //GXDLMSMESSAGEHANDLER_H
