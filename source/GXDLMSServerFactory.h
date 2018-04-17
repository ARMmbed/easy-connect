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
#pragma once
#include "GXDLMSBaseAL.h"


class CGXDLMSServerFactory
{

public:

	static CGXDLMSBaseAL* getCGXDLMSServer(bool UseLogicalNameReferencing, DLMS_INTERFACE_TYPE IntefaceType, DLMS_SERVICE_TYPE protocolType)
	{
		if(DLMS_SERVICE_TYPE_TCP == protocolType)
		{
			printf("CGXDLMSServerFactory TCP protocol type\r\n");
			return new CGXDLMSTcp(UseLogicalNameReferencing, IntefaceType);
		}
		else if(DLMS_SERVICE_TYPE_UDP == protocolType)
		{
			printf("CGXDLMSServerFactory UDP protocol type\r\n");
			return new CGXDLMSUdp(UseLogicalNameReferencing, IntefaceType);
		}
		/*
		else if(DLMS_SERVICE_TYPE_SERIAL == protocolType)
		{
			printf("CGXDLMSServerFactory Serial protocol type\r\n");
			return new CGXDLMSSerial(UseLogicalNameReferencing, IntefaceType);
		}
		*/
		else
		{
			printf("CGXDLMSServerFactory BAD protocol type\r\n");
			return NULL;
		}

		return NULL;
	}
};
