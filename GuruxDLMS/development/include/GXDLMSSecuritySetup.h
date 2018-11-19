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

#ifndef GXDLMSDLMS_SECURITYSETUP_H
#define GXDLMSDLMS_SECURITYSETUP_H

#include "GXDLMSObject.h"

/**
Online help:
http://www.gurux.fi/Gurux.DLMS.Objects.CGXDLMSSecuritySetup
*/


typedef enum
{
	EN_KA_STATE_NONE,
	EN_KA_STATE_IMPORT,
	EN_KA_STATE_KEY_CALCULATED,
	EN_KA_STATE_EXPORT,
}EN_KEY_AGREEMENT_STATE;

class CGXDLMSSecuritySetup : public CGXDLMSObject
{
    unsigned char m_SecurityPolicy;
    DLMS_SECURITY_SUITE m_SecuritySuite;
    CGXByteBuffer m_ServerSystemTitle;
    CGXByteBuffer m_ClientSystemTitle;
    EN_KEY_AGREEMENT_STATE m_eKeyAgreementState;

    int ValidateCertificateIdentification(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);
    int ValidateCertificateIdentificationEntity(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);
    int ValidateCertificateIdentificationSerial(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);
public:
    //Constructor.
    CGXDLMSSecuritySetup();

    //SN Constructor.
    CGXDLMSSecuritySetup(std::string ln, unsigned short sn);


    //LN Constructor.
    CGXDLMSSecuritySetup(std::string ln);

    //Use DLMS_SECURITY_POLICY for version 0 and DLMS_SECURITY_POLICY1 for version 1.
    unsigned char GetSecurityPolicy();

    //Use DLMS_SECURITY_POLICY for version 0 and DLMS_SECURITY_POLICY1 for version 1.
    void SetSecurityPolicy(unsigned char value);

    DLMS_SECURITY_SUITE GetSecuritySuite();

    void SetSecuritySuite(DLMS_SECURITY_SUITE value);

    CGXByteBuffer GetClientSystemTitle();

    void SetClientSystemTitle(CGXByteBuffer& value);

    CGXByteBuffer GetServerSystemTitle();

    void SetServerSystemTitle(CGXByteBuffer& value);

    EN_KEY_AGREEMENT_STATE GetKeyAgreementState();

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

    // Invokes method.
    int Invoke(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e);
};
#endif //GXDLMSDLMS_SECURITYSETUP_H
