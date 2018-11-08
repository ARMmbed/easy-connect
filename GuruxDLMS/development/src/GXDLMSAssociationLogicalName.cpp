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
#include "../include/GXDLMSVariant.h"
#include "../include/GXDLMSClient.h"
#include "../include/GXDLMSObjectFactory.h"
#include "../include/GXDLMSAssociationLogicalName.h"
#include "../include/GXDLMSServer.h"
#include "../../../security_util/include/security_api.h"

//no need buffer size larger
#define MAX_BUFFER_SIZE 72

#if 0
static unsigned char client_sys_title[] =
{ 0x4D,0x4D,0x4D,0x00,0x00,0xBC,0x61,0x4E };

// Challenges
static unsigned char CtoS[] =
{
	0x2C,0xA1,0xFC,0x2D,0xE9,0xCD,0x03,0xB5,0xE8,0xE2,0x34,
	0xCE,0xA1,0x6F,0x28,0x53,0xF6,0xDC,0x5F,0x54,0x52,0x6F,
	0x4F,0x49,0x95,0x77,0x2A,0x50,0xFB,0x7E,0x63,0xB3 
};

// Private keys 
static unsigned char d_client[] =
{
	0xE9,0xA0,0x45,0x34,0x6B,0x20,0x57,0xF1,0x82,0x03,0x18,
	0xAB,0x12,0x54,0x93,0xE9,0xAB,0x36,0xCE,0x59,0x00,0x11,
	0xC0,0xFF,0x30,0x09,0x08,0x58,0xA1,0x18,0xDD,0x2E 
};

// Public keys
static unsigned char qx_client[] =
{ 
	0x91,0x7D,0xBF,0xEC,0xA4,0x33,0x07,0x37,0x52,0x47,0x98,
	0x9F,0x07,0xCC,0x23,0xF5,0x3D,0x4B,0x96,0x3A,0xF8,0x02,
	0x6C,0x74,0x9D,0xB3,0x38,0x52,0x01,0x10,0x56,0xDF 
};

static unsigned char qy_client[] =
{ 
	0xDB,0xE8,0x32,0x7B,0xD6,0x9C,0xC1,0x49,0xF0,0x18,0xA8,
	0xE4,0x46,0xDD,0xA6,0xC5,0x5B,0xCD,0x78,0xE5,0x96,0xA5,
	0x6D,0x40,0x32,0x36,0x23,0x3F,0x93,0xCC,0x89,0xB3 
};

// expected value:          expected_client_str = F(StoC)
static unsigned char expected_client_str[] =
{ 
	0xC5,0xC6,0xD6,0x62,0x0B,0xDB,0x1A,0x39,0xFC,0xE5,0x0F,
	0x4D,0x64,0xF0,0xDB,0x71,0x2D,0x6F,0xB5,0x7A,0x64,0x03,
	0x0B,0x0C,0x29,0x7E,0x12,0x50,0xDC,0x85,0x96,0x60,0xD3,
	0xB1,0xFA,0x33,0x4A,0xD8,0x04,0x11,0x80,0x73,0x69,0xF5,
	0xDD,0x3B,0xC1,0x7B,0x59,0x89,0x4C,0x9E,0x9C,0x11,0xC5,
	0x93,0x76,0x58,0x0D,0x15,0xA2,0x64,0x6D,0x16 
};

// expected value:          expected_server_str = F(CtoS)
static unsigned char expected_server_str[] =
{ 
	0x94,0x6C,0x2E,0x3E,0x4F,0x18,0x29,0x15,0x71,0xF4,0xA4,
	0x5A,0xCB,0x70,0x86,0x10,0x05,0x74,0x69,0x4A,0x3B,0xAF,
	0x67,0xD2,0xD1,0x47,0xFE,0x8F,0x92,0x48,0x1A,0x5A,0xB2,
	0x18,0x6C,0x5C,0xBC,0x3F,0x80,0xE9,0x44,0x82,0xD9,0x38,
	0x8B,0x85,0xC6,0xA7,0x3E,0x5F,0xD6,0x87,0xF0,0x97,0x73,
	0xC1,0xF6,0x15,0xAA,0x2A,0x90,0x5E,0xD0,0x57 
};

#endif

void CGXDLMSAssociationLogicalName::UpdateAccessRights(CGXDLMSObject* pObj, CGXDLMSVariant data)
{
    for (std::vector<CGXDLMSVariant >::iterator it = data.Arr[0].Arr.begin(); it != data.Arr[0].Arr.end(); ++it)
    {
        int id = it->Arr[0].ToInteger();
        DLMS_ACCESS_MODE mode = (DLMS_ACCESS_MODE)it->Arr[1].ToInteger();
        pObj->SetAccess(id, mode);
    }
    for (std::vector<CGXDLMSVariant >::iterator it = data.Arr[1].Arr.begin(); it != data.Arr[1].Arr.end(); ++it)
    {
        int id = it->Arr[0].ToInteger();
        DLMS_METHOD_ACCESS_MODE mode = (DLMS_METHOD_ACCESS_MODE)it->Arr[1].ToInteger();
        pObj->SetMethodAccess(id, mode);
    }
}

int CGXDLMSAssociationLogicalName::GetAccessRights(CGXDLMSObject* pItem, CGXDLMSServer* server, CGXByteBuffer& data)
{
    int ret;
    int cnt = pItem->GetAttributeCount();
    data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
    data.SetUInt8(2);
    data.SetUInt8(DLMS_DATA_TYPE_ARRAY);
    GXHelpers::SetObjectCount(cnt, data);
    CGXDLMSValueEventArg e(server, pItem, 0);

    CGXDLMSVariant index, access, empty;
    for (int pos = 0; pos != cnt; ++pos)
    {
        e.SetIndex(pos + 1);
        index = pos + 1;
        if (server != NULL)
        {
            access = server->GetAttributeAccess(&e);
        }
        else
        {
            access = DLMS_ACCESS_MODE_READ_WRITE;
        }
        //attribute_access_item
        data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
        data.SetUInt8(3);
        if ((ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_INT8, index)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_ENUM, access)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_NONE, empty)) != 0)
        {
            return ret;
        }
    }
    cnt = pItem->GetMethodCount();
    data.SetUInt8(DLMS_DATA_TYPE_ARRAY);
    GXHelpers::SetObjectCount(cnt, data);
    for (int pos = 0; pos != cnt; ++pos)
    {
        e.SetIndex(pos + 1);
        index = pos + 1;
        if (server != NULL)
        {
            access = server->GetMethodAccess(&e);
        }
        else
        {
            access = DLMS_METHOD_ACCESS_MODE_ACCESS;
        }
        //attribute_access_item
        data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
        data.SetUInt8(2);
        GXHelpers::SetData(data, DLMS_DATA_TYPE_INT8, index);
        GXHelpers::SetData(data, DLMS_DATA_TYPE_ENUM, access);
    }
    return DLMS_ERROR_CODE_OK;
}


// Updates secret.
int CGXDLMSAssociationLogicalName::UpdateSecret(CGXDLMSClient* client, std::vector<CGXByteBuffer>& reply)
{
    if (m_AuthenticationMechanismName.GetMechanismId() == DLMS_AUTHENTICATION_NONE)
    {
#if defined(_WIN32) || defined(_WIN64)//Windows
        printf("Invalid authentication level in MechanismId.\n");
#endif
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    if (m_AuthenticationMechanismName.GetMechanismId() == DLMS_AUTHENTICATION_HIGH_GMAC)
    {
#if defined(_WIN32) || defined(_WIN64)//Windows
        printf("HighGMAC secret is updated using Security setup.\n");
#endif
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    if (m_AuthenticationMechanismName.GetMechanismId() == DLMS_AUTHENTICATION_LOW)
    {
        return client->Write(this, 7, reply);
    }
    //Action is used to update High authentication password.
    CGXDLMSVariant tmp = m_Secret;
    return client->Method(this, 2, tmp, reply);
}

// Add user to user list.
int CGXDLMSAssociationLogicalName::AddUser(CGXDLMSClient* client, unsigned char id, std::string name, std::vector<CGXByteBuffer>& reply)
{
    CGXByteBuffer data;
    data.SetUInt8((unsigned char)DLMS_DATA_TYPE_STRUCTURE);
    //Add structure size.
    data.SetUInt8(2);
    CGXDLMSVariant tmp = id;
    GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, tmp);
    tmp = name;
    GXHelpers::SetData(data, DLMS_DATA_TYPE_STRING, tmp);
    tmp = data;
    return client->Method(this, 5, tmp, reply);
}

// Remove user fro user list.
int CGXDLMSAssociationLogicalName::RemoveUser(CGXDLMSClient* client, unsigned char id, std::string name, std::vector<CGXByteBuffer>& reply)
{
    CGXByteBuffer data;
    data.SetUInt8((unsigned char)DLMS_DATA_TYPE_STRUCTURE);
    //Add structure size.
    data.SetUInt8(2);
    CGXDLMSVariant tmp = id;
    GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, tmp);
    tmp = name;
    GXHelpers::SetData(data, DLMS_DATA_TYPE_STRING, tmp);
    tmp = data;
    return client->Method(this, 6, tmp, reply);
}

// Returns LN Association View.
int CGXDLMSAssociationLogicalName::GetObjects(
    CGXDLMSSettings& settings,
    CGXDLMSValueEventArg& e,
    CGXByteBuffer& data)
{
    int ret;
    unsigned long pos = 0;
    //Add count only for first time.
    if (settings.GetIndex() == 0)
    {
        settings.SetCount((unsigned short)m_ObjectList.size());
        data.SetUInt8(DLMS_DATA_TYPE_ARRAY);
        //Add count
        GXHelpers::SetObjectCount((unsigned long)m_ObjectList.size(), data);
    }
    for (CGXDLMSObjectCollection::iterator it = m_ObjectList.begin(); it != m_ObjectList.end(); ++it)
    {
        ++pos;
        if (!(pos <= settings.GetIndex()))
        {
            data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
            data.SetUInt8(4);//Count
            CGXDLMSVariant type = (*it)->GetObjectType();
            CGXDLMSVariant version = (*it)->GetVersion();
            GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT16, type);//ClassID
            GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, version);//Version
            CGXDLMSVariant ln((*it)->m_LN, 6, DLMS_DATA_TYPE_OCTET_STRING);
            GXHelpers::SetData(data, DLMS_DATA_TYPE_OCTET_STRING, ln);//LN
            //Access rights.
            if ((ret = GetAccessRights(*it, e.GetServer(), data)) != 0)
            {
                return ret;
            };
            if (settings.IsServer())
            {
                settings.SetIndex(settings.GetIndex() + 1);
                //If PDU is full.
                if (!e.GetSkipMaxPduSize() && data.GetSize() >= settings.GetMaxPduSize())
                {
                    break;
                }
            }
        }
    }
    return DLMS_ERROR_CODE_OK;
}

// Returns user list
int CGXDLMSAssociationLogicalName::GetUsers(
    CGXDLMSSettings& settings,
    CGXDLMSValueEventArg& e,
    CGXByteBuffer& data)
{
    unsigned long pos = 0;
    //Add count only for first time.
    if (settings.GetIndex() == 0)
    {
        settings.SetCount((unsigned short)m_UserList.size());
        data.SetUInt8(DLMS_DATA_TYPE_ARRAY);
        //Add count
        GXHelpers::SetObjectCount((unsigned long)m_UserList.size(), data);
    }
    for (std::vector<std::pair<unsigned char, std::string> >::iterator it = m_UserList.begin(); it != m_UserList.end(); ++it)
    {
        ++pos;
        if (!(pos <= settings.GetIndex()))
        {
            data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
            data.SetUInt8(2);//Count
            CGXDLMSVariant tmp = it->first;
            GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, tmp);//Id
            tmp = it->second;
            GXHelpers::SetData(data, DLMS_DATA_TYPE_STRING, tmp);//Name
            if (settings.IsServer())
            {
                settings.SetIndex(settings.GetIndex() + 1);
                //If PDU is full.
                if (!e.GetSkipMaxPduSize() && data.GetSize() >= settings.GetMaxPduSize())
                {
                    break;
                }
            }
        }
    }
    return DLMS_ERROR_CODE_OK;
}


CGXDLMSAssociationLogicalName::CGXDLMSAssociationLogicalName() :
    CGXDLMSAssociationLogicalName("0.0.40.0.0.255")
{
}

/**
 Constructor.
 @param ln Logical Name of the object.
*/
CGXDLMSAssociationLogicalName::CGXDLMSAssociationLogicalName(std::string ln) :
    CGXDLMSObject(DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, ln, 0)
{
    m_AssociationStatus = DLMS_ASSOCIATION_STATUS_NON_ASSOCIATED;
    m_Secret.AddString("Gurux");
    m_Version = 2;
    m_ClientSAP = 0;
    m_ServerSAP = 0;
}

CGXDLMSObjectCollection& CGXDLMSAssociationLogicalName::GetObjectList()
{
    return m_ObjectList;
}

unsigned char CGXDLMSAssociationLogicalName::GetClientSAP()
{
    return m_ClientSAP;
}
void CGXDLMSAssociationLogicalName::SetClientSAP(unsigned char value)
{
    m_ClientSAP = value;
}

unsigned short CGXDLMSAssociationLogicalName::GetServerSAP()
{
    return m_ServerSAP;
}

void CGXDLMSAssociationLogicalName::SetServerSAP(unsigned short value)
{
    m_ServerSAP = value;
}

CGXApplicationContextName& CGXDLMSAssociationLogicalName::GetApplicationContextName()
{
    return m_ApplicationContextName;
}

CGXDLMSContextType& CGXDLMSAssociationLogicalName::GetXDLMSContextInfo()
{
    return m_XDLMSContextInfo;
}

CGXAuthenticationMechanismName& CGXDLMSAssociationLogicalName::GetAuthenticationMechanismName()
{
    return m_AuthenticationMechanismName;
}

CGXByteBuffer& CGXDLMSAssociationLogicalName::GetSecret()
{
    return m_Secret;
}
void CGXDLMSAssociationLogicalName::SetSecret(CGXByteBuffer& value)
{
    m_Secret = value;
}

std::vector<std::pair<unsigned char, std::string> >& CGXDLMSAssociationLogicalName::GetUserList()
{
    return m_UserList;
}

void CGXDLMSAssociationLogicalName::SetUserList(std::vector<std::pair<unsigned char, std::string> >& value)
{
    m_UserList = value;
}

std::pair<unsigned char, std::string>& CGXDLMSAssociationLogicalName::GetCurrentUser()
{
    return m_CurrentUser;
}

void CGXDLMSAssociationLogicalName::SetCurrentUser(std::pair<unsigned char, std::string>& value)
{
    m_CurrentUser = value;
}

DLMS_ASSOCIATION_STATUS CGXDLMSAssociationLogicalName::GetAssociationStatus()
{
    return m_AssociationStatus;
}

void CGXDLMSAssociationLogicalName::SetAssociationStatus(DLMS_ASSOCIATION_STATUS value)
{
    m_AssociationStatus = value;
}

std::string CGXDLMSAssociationLogicalName::GetSecuritySetupReference()
{
    return m_SecuritySetupReference;
}
void CGXDLMSAssociationLogicalName::SetSecuritySetupReference(std::string value)
{
    m_SecuritySetupReference = value;
}

// Returns amount of attributes.
int CGXDLMSAssociationLogicalName::GetAttributeCount()
{
    if (m_Version > 1)
        return 11;
    //Security Setup Reference is from version 1.
    if (GetVersion() > 0)
        return 9;
    return 8;
}

// Returns amount of methods.
int CGXDLMSAssociationLogicalName::GetMethodCount()
{
    if (m_Version > 1)
    {
        return 6;
    }
    return 4;
}

void CGXDLMSAssociationLogicalName::GetValues(std::vector<std::string>& values)
{
    values.clear();
    std::string ln;
    GetLogicalName(ln);
    values.push_back(ln);
    values.push_back(m_ObjectList.ToString());
    std::string str = CGXDLMSVariant(m_ClientSAP).ToString();
    str += "/";
    str += CGXDLMSVariant(m_ServerSAP).ToString();
    values.push_back(str);
    values.push_back(m_ApplicationContextName.ToString());
    values.push_back(m_XDLMSContextInfo.ToString());
    values.push_back(m_AuthenticationMechanismName.ToString());
    values.push_back(m_Secret.ToHexString());
    values.push_back(CGXDLMSVariant(m_AssociationStatus).ToString());
    //Security Setup Reference is from version 1.
    if (GetVersion() > 0)
    {
        values.push_back(m_SecuritySetupReference);
    }
}

void CGXDLMSAssociationLogicalName::GetAttributeIndexToRead(std::vector<int>& attributes)
{
    //LN is static and read only once.
    if (CGXDLMSObject::IsLogicalNameEmpty(m_LN))
    {
        attributes.push_back(1);
    }
    //ObjectList is static and read only once.
    if (!IsRead(2))
    {
        attributes.push_back(2);
    }
    //associated_partners_id is static and read only once.
    if (!IsRead(3))
    {
        attributes.push_back(3);
    }
    //Application Context Name is static and read only once.
    if (!IsRead(4))
    {
        attributes.push_back(4);
    }
    //xDLMS Context Info
    if (!IsRead(5))
    {
        attributes.push_back(5);
    }
    // Authentication Mechanism Name
    if (!IsRead(6))
    {
        attributes.push_back(6);
    }
    // Secret
    if (!IsRead(7))
    {
        attributes.push_back(7);
    }
    // Association Status
    if (!IsRead(8))
    {
        attributes.push_back(8);
    }
    //Security Setup Reference is from version 1.
    if (GetVersion() > 0 && !IsRead(9))
    {
        attributes.push_back(9);
    }
    //User list and current user are in version 2.
    if (m_Version > 1)
    {
        if (!IsRead(10))
        {
            attributes.push_back(10);
        }
        if (!IsRead(11))
        {
            attributes.push_back(11);
        }
    }
}

int CGXDLMSAssociationLogicalName::GetDataType(int index, DLMS_DATA_TYPE& type)
{
    if (index == 1)
    {
        type = DLMS_DATA_TYPE_OCTET_STRING;
        return DLMS_ERROR_CODE_OK;
    }
    if (index == 2)
    {
        type = DLMS_DATA_TYPE_ARRAY;
        return DLMS_ERROR_CODE_OK;
    }
    if (index == 3)
    {
        type = DLMS_DATA_TYPE_STRUCTURE;
        return DLMS_ERROR_CODE_OK;
    }
    if (index == 4)
    {
        type = DLMS_DATA_TYPE_STRUCTURE;
        return DLMS_ERROR_CODE_OK;
    }
    if (index == 5)
    {
        type = DLMS_DATA_TYPE_STRUCTURE;
        return DLMS_ERROR_CODE_OK;
    }
    if (index == 6)
    {
        type = DLMS_DATA_TYPE_STRUCTURE;
        return DLMS_ERROR_CODE_OK;
    }
    if (index == 7)
    {
        type = DLMS_DATA_TYPE_OCTET_STRING;
        return DLMS_ERROR_CODE_OK;
    }
    if (index == 8)
    {
        type = DLMS_DATA_TYPE_ENUM;
        return DLMS_ERROR_CODE_OK;
    }
    if (m_Version > 0 && index == 9)
    {
        type = DLMS_DATA_TYPE_OCTET_STRING;
        return DLMS_ERROR_CODE_OK;
    }
    if (m_Version > 1)
    {
        if (index == 10)
        {
            return DLMS_DATA_TYPE_ARRAY;
        }
        if (index == 11)
        {
            return DLMS_DATA_TYPE_STRUCTURE;
        }
    }
    return DLMS_ERROR_CODE_INVALID_PARAMETER;
}

static void fill_hls_params(hls_auth_params_t& auth_params, CGXDLMSSettings& settings)
{
	auth_params.originator_sys_title.buf = settings.GetSourceSystemTitle().GetData(); // originator - always the client
	auth_params.originator_sys_title.size = settings.GetSourceSystemTitle().GetSize(); // originator - always the client
	auth_params.originator_challenge.buf = settings.GetCtoSChallenge().GetData();
	auth_params.originator_challenge.size = settings.GetCtoSChallenge().GetSize();
	auth_params.recipient_sys_title.buf = (settings.GetCipher())->GetSystemTitle().GetData(); // recipient - always the server
	auth_params.recipient_sys_title.size = (settings.GetCipher())->GetSystemTitle().GetSize(); // recipient - always the server
	auth_params.recipient_challenge.buf = settings.GetStoCChallenge().GetData();
	auth_params.recipient_challenge.size = settings.GetStoCChallenge().GetSize();
	auth_params.public_key = settings.GetKey().m_originator_public; // get the public key of the client
	auth_params.public_key_size = PUBLIC_KEY_SIZE;
	auth_params.security_suite = 1;
	auth_params.mechanism_id = 7;
}

static void PrintfBuff(unsigned char *ptr, int size)
{
	printf("#########################\n");
	for(int i = 0 ; i < size ; ++i)
	{
		printf("%02x ", *ptr++);
		if((i+1)% 8 == 0) printf("\n");
	}
	if(size% 8 != 0) printf("\n");
	printf("#########################\n\n");
}

#define SERVER_KA_PRIV_KEY \
					0xAA, 0xD3, 0xFD, 0x07, 0x32, 0xE9, 0x91, 0xCF, 0x52, 0xA7,\
					0x4C, 0x66, 0xC1, 0xF2, 0x82, 0x7D, 0xDC, 0x53, 0x52, 0x2A,\
					0x2E, 0x0A, 0x16, 0x9D, 0x7C, 0x4F, 0xFC, 0xC0, 0xFB, 0x5D,\
					0x6A, 0x4D

#define SERVER_KA_PUB_KEY \
					0xA6, 0x53, 0x56, 0x5B, 0x0E, 0x06, 0x07, 0x0B, 0xAE, 0x9F,\
					0xBE, 0x14, 0x0A, 0x5D, 0x21, 0x56, 0x81, 0x2A, 0xEE, 0x2D,\
					0xD5, 0x25, 0x05, 0x3E, 0x3E, 0xFC, 0x85, 0x0B, 0xF1, 0x3B,\
					0xFD, 0xFF, 0xCB, 0x24, 0x0B, 0xC7, 0xB7, 0x7B, 0xFF, 0x58,\
					0x83, 0x34, 0x4E, 0x72, 0x75, 0x90, 0x8D, 0x22, 0x87, 0xBE,\
					0xFA, 0x37, 0x25, 0x01, 0x72, 0x95, 0xA0, 0x96, 0x98, 0x9D,\
					0x23, 0x38, 0x29, 0x0B

static unsigned char d_server_ka[]= {SERVER_KA_PRIV_KEY};
unsigned char q_server_ka[]= {SERVER_KA_PUB_KEY};
#define CLIENT_KA_PUB_KEY \
					0x07, 0xC5, 0x6D, 0xE2, 0xDC, 0xAF, 0x0F, 0xD7, 0x93, 0xEF,\
					0x29, 0xF0, 0x19, 0xC8, 0x9B, 0x4A, 0x0C, 0xC1, 0xE0, 0x01,\
					0xCE, 0x94, 0xF4, 0xFF, 0xBE, 0x10, 0xBC, 0x05, 0xE7, 0xE6,\
					0x6F, 0x76, 0x71, 0xA1, 0x3F, 0xBC, 0xF9, 0xE6, 0x62, 0xB9,\
					0x82, 0x6F, 0xFF, 0x6A, 0x69, 0x38, 0x54, 0x6D, 0x52, 0x4E,\
					0xD6, 0xD3, 0x40, 0x5F, 0x02, 0x02, 0x96, 0xBD, 0xE1, 0x6B,\
					0x04, 0xF7, 0xA7, 0xC2
unsigned char q_client_ka[]= {CLIENT_KA_PUB_KEY};
#define PUBLIC_KEY_SIZE			64
#define PRIVATE_KEY_SIZE		32

static void init_local_ds_keys_server(
		security_key_pair_t *ds_key_pair, CGXDLMSSettings& settings)
{
	ds_key_pair->public_key = settings.GetKey().m_originator_public;
	ds_key_pair->public_key_size = PRIVATE_KEY_SIZE;
	ds_key_pair->private_key = settings.get_key_cb(&ds_key_pair->private_key_size);
}

static void init_local_ka_keys_server(
		security_key_pair_t *ka_key_pair)
{
	ka_key_pair->private_key = d_server_ka;
	ka_key_pair->private_key_size = PRIVATE_KEY_SIZE;
	ka_key_pair->public_key = q_server_ka;
	ka_key_pair->public_key_size = PUBLIC_KEY_SIZE;
}

int CGXDLMSAssociationLogicalName::Invoke(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    // Check reply_to_HLS_authentication
    if (e.GetIndex() == 1)
    {
		int ret;
		if (settings.GetAuthentication() == DLMS_AUTHENTICATION_HIGH_ECDSA)
		{
			uint32_t buffer_size = e.GetParameters().GetSize();
			uint8_t *buffer = e.GetParameters().byteArr;
			uint8_t is_originator = !settings.IsServer(); // server is never the originator

			security_key_pair_t ds_key_pair;
			security_key_pair_t ka_key_pair;

			init_local_ds_keys_server(&ds_key_pair, settings);
			init_local_ka_keys_server(&ka_key_pair);
			init_security_util(&ds_key_pair, &ka_key_pair);

			if(0)
				PrintfBuff(buffer, buffer_size);

			hls_auth_params_t auth_params;

			fill_hls_params(auth_params, settings);

			// server validate the F(StoC) before creating the F(CtoS)
			if ((ret = verify_HLS_authentication(&auth_params, buffer, buffer_size, is_originator)) != 0)
			{
				settings.SetConnected(false);
				m_AssociationStatus = DLMS_ASSOCIATION_STATUS_NON_ASSOCIATED;
				return ret;
			}

			buffer_size = MAX_BUFFER_SIZE;
			if ((buffer = (uint8_t*)malloc(buffer_size)) == NULL) return 1;

			if ((ret = create_HLS_authentication(&auth_params, buffer, &buffer_size, is_originator)) != 0)
				return ret;

			CGXByteBuffer F_CtoS;
			F_CtoS.Set(buffer, buffer_size);
			free(buffer);
			
			// the server sends the F(CtoS)
			e.SetValue(F_CtoS);
			// the server accepts the connection
			settings.SetConnected(true);
			m_AssociationStatus = DLMS_ASSOCIATION_STATUS_ASSOCIATED;

			// set the KA
			secured_association_params_t session_id;
			session_id.local_wrapper_port = settings.GetClientWport();
			session_id.remote_wrapper_port = settings.GetServerWport();
			session_id.remote_port = settings.GetServerPort();
			session_id.remote_ip_address.type = security_IP_address_ipV4;
			session_id.remote_ip_address.choice.ipV4 = settings.GetServerIpAddr();

			if ((ret = validate_and_save_remote_ka_crt(&session_id, q_client_ka, PUBLIC_KEY_SIZE)) != 0) {
				m_AssociationStatus = DLMS_ASSOCIATION_STATUS_NON_ASSOCIATED;
				return ret;
			}

			if ((ret = calculate_shared_secret(&session_id)) != 0) {
				m_AssociationStatus = DLMS_ASSOCIATION_STATUS_NON_ASSOCIATED;
				return ret;
			}
		}

		else
		{
			unsigned long ic = 0;
			CGXByteBuffer* readSecret;
			if (settings.GetAuthentication() == DLMS_AUTHENTICATION_HIGH_GMAC)
			{
				unsigned char ch;
				readSecret = &settings.GetSourceSystemTitle();
				CGXByteBuffer bb;
				bb.Set(e.GetParameters().byteArr, e.GetParameters().GetSize());
				if ((ret = bb.GetUInt8(&ch)) != 0)
				{
					return ret;
				}
				if ((ret = bb.GetUInt32(&ic)) != 0)
				{
					return ret;
				}
			}
			else
			{
				readSecret = &m_Secret;
			}
			CGXByteBuffer serverChallenge;
			if ((ret = CGXSecure::Secure(settings, settings.GetCipher(), ic,
				settings.GetStoCChallenge(), *readSecret, serverChallenge)) != 0)
			{
				return ret;
			}
			if (serverChallenge.Compare(e.GetParameters().byteArr, e.GetParameters().GetSize()))
			{
				if (settings.GetAuthentication() == DLMS_AUTHENTICATION_HIGH_GMAC)
				{
					readSecret = &settings.GetCipher()->GetSystemTitle();
					ic = settings.GetCipher()->GetFrameCounter();
				}
				else
				{
					readSecret = &m_Secret;
				}
				if ((ret = CGXSecure::Secure(settings, settings.GetCipher(), ic,
					settings.GetCtoSChallenge(), *readSecret, serverChallenge)) != 0)
				{
					return ret;
				}
				e.SetValue(serverChallenge);
				settings.SetConnected(true);
				m_AssociationStatus = DLMS_ASSOCIATION_STATUS_ASSOCIATED;
			}
			else
			{
				settings.SetConnected(false);
				m_AssociationStatus = DLMS_ASSOCIATION_STATUS_NON_ASSOCIATED;
			}
		}
    }
    else if (e.GetIndex() == 2)
    {
        if (e.GetParameters().GetSize() == 0)
        {
            e.SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
        }
        else
        {
            m_Secret.Clear();
            m_Secret.Set(e.GetParameters().byteArr, e.GetParameters().GetSize());
        }
    }
    else if (e.GetIndex() == 5)
    {
        if (e.GetParameters().Arr.size() != 2)
        {
            e.SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
        }
        else
        {
            m_UserList.push_back(std::pair<unsigned char, std::string>(e.GetParameters().Arr[0].bVal, e.GetParameters().Arr[1].strVal));
        }
    }
    else if (e.GetIndex() == 6)
    {
        if (e.GetParameters().Arr.size() != 2)
        {
            e.SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
        }
        else
        {
            for (std::vector<std::pair<unsigned char, std::string> >::iterator it = m_UserList.begin(); it != m_UserList.end(); ++it)
            {
                if (it->first == e.GetParameters().Arr[0].bVal)
                {
                    m_UserList.erase(it);
                    break;
                }
            }
        }
    }
    else
    {
        e.SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
    }
    return 0;
}

int CGXDLMSAssociationLogicalName::GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    int ret;
    if (e.GetIndex() == 1)
    {
        int ret;
        CGXDLMSVariant tmp;
        if ((ret = GetLogicalName(this, tmp)) != 0)
        {
            return ret;
        }
        e.SetValue(tmp);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 2)
    {
        e.SetByteArray(true);
        CGXByteBuffer buff;
        ret = GetObjects(settings, e, buff);
        e.SetValue(buff);
        return ret;
    }
    if (e.GetIndex() == 3)
    {
        e.SetByteArray(true);
        CGXByteBuffer data;
        data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
        //Add count
        data.SetUInt8(2);
        data.SetUInt8(DLMS_DATA_TYPE_INT8);
        data.SetUInt8(m_ClientSAP);
        data.SetUInt8(DLMS_DATA_TYPE_UINT16);
        data.SetUInt16(m_ServerSAP);
        e.SetValue(data);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 4)
    {
        e.SetByteArray(true);
        CGXByteBuffer data;
        data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
        //Add count
        data.SetUInt8(0x7);
        CGXDLMSVariant ctt = m_ApplicationContextName.GetJointIsoCtt();
        CGXDLMSVariant country = m_ApplicationContextName.GetCountry();
        CGXDLMSVariant name = m_ApplicationContextName.GetCountryName();
        CGXDLMSVariant organization = m_ApplicationContextName.GetIdentifiedOrganization();
        CGXDLMSVariant ua = m_ApplicationContextName.GetDlmsUA();
        CGXDLMSVariant context = m_ApplicationContextName.GetApplicationContext();
        CGXDLMSVariant id = m_ApplicationContextName.GetContextId();
        if ((ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, ctt)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, country)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT16, name)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, organization)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, ua)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, context)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, id)) != 0)
        {
            return ret;
        }
        e.SetValue(data);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 5)
    {
        e.SetByteArray(true);
        CGXByteBuffer data;
        data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
        data.SetUInt8(6);
        CGXByteBuffer bb, bb2;
        bb.SetUInt32(m_XDLMSContextInfo.GetConformance());
        bb.SubArray(1, 3, bb2);
        CGXDLMSVariant conformance = bb2;
        CGXDLMSVariant rx = m_XDLMSContextInfo.GetMaxPduSize();
        CGXDLMSVariant tx = m_XDLMSContextInfo.GetMaxSendPduSize();
        CGXDLMSVariant version = m_XDLMSContextInfo.GetDlmsVersionNumber();
        CGXDLMSVariant quality = m_XDLMSContextInfo.GetQualityOfService();
        CGXDLMSVariant info = m_XDLMSContextInfo.GetCypheringInfo();
        if ((ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_BIT_STRING, conformance)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT16, rx)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT16, tx)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, version)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_INT8, quality)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_OCTET_STRING, info)) != 0)
        {
            return ret;
        }
        e.SetValue(data);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 6)
    {
        e.SetByteArray(true);
        CGXByteBuffer data;
        data.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
        //Add count
        data.SetUInt8(0x7);
        CGXDLMSVariant ctt = m_AuthenticationMechanismName.GetJointIsoCtt();
        CGXDLMSVariant country = m_AuthenticationMechanismName.GetCountry();
        CGXDLMSVariant name = m_AuthenticationMechanismName.GetCountryName();
        CGXDLMSVariant organization = m_AuthenticationMechanismName.GetIdentifiedOrganization();
        CGXDLMSVariant ua = m_AuthenticationMechanismName.GetDlmsUA();
        CGXDLMSVariant context = m_AuthenticationMechanismName.GetAuthenticationMechanismName();
        CGXDLMSVariant id = m_AuthenticationMechanismName.GetMechanismId();
        if ((ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, ctt)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, country)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT16, name)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, organization)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, ua)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, context)) != 0 ||
            (ret = GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, id)) != 0)
        {
            return ret;
        }
        e.SetValue(data);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 7)
    {
        e.SetValue(m_Secret);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 8)
    {
        e.SetValue((unsigned char)m_AssociationStatus);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 9)
    {
        CGXDLMSVariant tmp;
        GXHelpers::SetLogicalName(m_SecuritySetupReference.c_str(), tmp);
        e.SetValue(tmp);
        return DLMS_ERROR_CODE_OK;
    }
    if (e.GetIndex() == 10)
    {
        e.SetByteArray(true);
        CGXByteBuffer buff;
        ret = GetUsers(settings, e, buff);
        e.SetValue(buff);
        return ret;
    }
    if (e.GetIndex() == 11)
    {
        e.SetByteArray(true);
        CGXByteBuffer data;
        data.SetUInt8((unsigned char)DLMS_DATA_TYPE_STRUCTURE);
        //Add structure size.
        data.SetUInt8(2);
        CGXDLMSVariant tmp = m_CurrentUser.first;
        GXHelpers::SetData(data, DLMS_DATA_TYPE_UINT8, tmp);
        tmp = m_CurrentUser.second;
        GXHelpers::SetData(data, DLMS_DATA_TYPE_STRING, tmp);
        e.SetValue(data);
        return DLMS_ERROR_CODE_OK;
    }
    return DLMS_ERROR_CODE_INVALID_PARAMETER;
}

int CGXDLMSAssociationLogicalName::SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    if (e.GetIndex() == 1)
    {
        return SetLogicalName(this, e.GetValue());
    }
    else if (e.GetIndex() == 2)
    {
        m_ObjectList.clear();
        if (e.GetValue().vt != DLMS_DATA_TYPE_NONE)
        {
            for (std::vector<CGXDLMSVariant >::iterator it = e.GetValue().Arr.begin(); it != e.GetValue().Arr.end(); ++it)
            {
                DLMS_OBJECT_TYPE type = (DLMS_OBJECT_TYPE)(*it).Arr[0].ToInteger();
                int version = (*it).Arr[1].ToInteger();
                std::string ln;
                GXHelpers::GetLogicalName((*it).Arr[2].byteArr, ln);
                CGXDLMSObject* pObj = settings.GetObjects().FindByLN(type, ln);
                if (pObj == NULL)
                {
                    pObj = CGXDLMSObjectFactory::CreateObject(type, ln);
                    if (pObj != NULL)
                    {
                        pObj->SetVersion(version);
                    }
                }
                if (pObj != NULL)
                {
                    UpdateAccessRights(pObj, (*it).Arr[3]);
                    m_ObjectList.push_back(pObj);
                }
            }
        }
    }
    else if (e.GetIndex() == 3)
    {
        if (e.GetValue().Arr.size() == 2)
        {
            m_ClientSAP = e.GetValue().Arr[0].ToInteger();
            m_ServerSAP = e.GetValue().Arr[1].ToInteger();
        }
        else
        {
            return DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    else if (e.GetIndex() == 4)
    {
        //Value of the object identifier encoded in BER
        if (e.GetValue().vt == DLMS_DATA_TYPE_OCTET_STRING)
        {
            CGXByteBuffer tmp;
            tmp.Set(e.GetValue().byteArr, e.GetValue().GetSize());
            int ret;
            unsigned char val;
            if ((ret = tmp.GetUInt8(0, &val)) != 0)
            {
                return ret;
            }
            if (val == 0x60)
            {
                m_ApplicationContextName.SetJointIsoCtt(0);
                m_ApplicationContextName.SetCountry(0);
                m_ApplicationContextName.SetCountryName(0);
                tmp.SetPosition(3);
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetIdentifiedOrganization(val);
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetDlmsUA(val);
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetApplicationContext(val);
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetContextId((DLMS_APPLICATION_CONTEXT_NAME)val);
            }
            else
            {
                //Get Tag and Len.
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 2)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 7)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                //Get tag
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 0x11)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetJointIsoCtt(val);
                //Get tag
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 0x11)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetCountry(val);
                //Get tag
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 0x12)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetCountryName(val);
                //Get tag
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 0x11)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetIdentifiedOrganization(val);
                //Get tag
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 0x11)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetDlmsUA(val);
                //Get tag
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 0x11)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetApplicationContext(val);
                //Get tag
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                if (val != 0x11)
                {
                    return DLMS_ERROR_CODE_INVALID_PARAMETER;
                }
                if ((ret = tmp.GetUInt8(&val)) != 0)
                {
                    return ret;
                }
                m_ApplicationContextName.SetContextId((DLMS_APPLICATION_CONTEXT_NAME) val);
            }
        }
        else
        {
            m_ApplicationContextName.SetJointIsoCtt(e.GetValue().Arr[0].ToInteger());
            m_ApplicationContextName.SetCountry(e.GetValue().Arr[1].ToInteger());
            m_ApplicationContextName.SetCountryName(e.GetValue().Arr[2].ToInteger());
            m_ApplicationContextName.SetIdentifiedOrganization(e.GetValue().Arr[3].ToInteger());
            m_ApplicationContextName.SetDlmsUA(e.GetValue().Arr[4].ToInteger());
            m_ApplicationContextName.SetApplicationContext(e.GetValue().Arr[5].ToInteger());
            m_ApplicationContextName.SetContextId((DLMS_APPLICATION_CONTEXT_NAME)e.GetValue().Arr[6].ToInteger());
        }
    }
    else if (e.GetIndex() == 5)
    {
        if (e.GetValue().vt == DLMS_DATA_TYPE_STRUCTURE)
        {
            m_XDLMSContextInfo.SetConformance((DLMS_CONFORMANCE)e.GetValue().Arr[0].ToInteger());
            m_XDLMSContextInfo.SetMaxReceivePduSize(e.GetValue().Arr[1].ToInteger());
            m_XDLMSContextInfo.SetMaxSendPduSize(e.GetValue().Arr[2].ToInteger());
            m_XDLMSContextInfo.SetDlmsVersionNumber(e.GetValue().Arr[3].ToInteger());
            m_XDLMSContextInfo.SetQualityOfService(e.GetValue().Arr[4].ToInteger());
            CGXByteBuffer tmp;
            tmp.Set(e.GetValue().Arr[5].byteArr, e.GetValue().Arr[5].GetSize());
            m_XDLMSContextInfo.SetCypheringInfo(tmp);
        }
    }
    else if (e.GetIndex() == 6)
    {
        if (e.GetValue().vt != DLMS_DATA_TYPE_NONE)
        {
            unsigned char val;
            int ret;
            //Value of the object identifier encoded in BER
            if (e.GetValue().vt == DLMS_DATA_TYPE_OCTET_STRING)
            {
                CGXByteBuffer tmp;
                tmp.Set(e.GetValue().byteArr, e.GetValue().GetSize());
                if ((ret = tmp.GetUInt8(0, &val)) != 0)
                {
                    return ret;
                }
                if (val == 0x60)
                {
                    m_AuthenticationMechanismName.SetJointIsoCtt(0);
                    m_AuthenticationMechanismName.SetCountry(0);
                    m_AuthenticationMechanismName.SetCountryName(0);
                    tmp.SetPosition(3);
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetIdentifiedOrganization(val);
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetDlmsUA(val);
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetAuthenticationMechanismName(val);
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetMechanismId((DLMS_AUTHENTICATION)val);
                }
                else
                {
                    //Get Tag and Len.
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 2)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 7)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    //Get tag
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 0x11)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetJointIsoCtt(val);
                    //Get tag
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 0x11)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetCountry(val);
                    //Get tag
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 0x12)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    unsigned short tmp2;
                    if ((ret = tmp.GetUInt16(&tmp2)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetCountryName(tmp2);
                    //Get tag
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 0x11)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetIdentifiedOrganization(val);
                    //Get tag
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 0x11)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetDlmsUA(val);
                    //Get tag
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 0x11)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetAuthenticationMechanismName(val);
                    //Get tag
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    if (val != 0x11)
                    {
                        return DLMS_ERROR_CODE_INVALID_PARAMETER;
                    }
                    if ((ret = tmp.GetUInt8(&val)) != 0)
                    {
                        return ret;
                    }
                    m_AuthenticationMechanismName.SetMechanismId((DLMS_AUTHENTICATION)val);
                }
            }
            else
            {
                m_AuthenticationMechanismName.SetJointIsoCtt(e.GetValue().Arr[0].ToInteger());
                m_AuthenticationMechanismName.SetCountry(e.GetValue().Arr[1].ToInteger());
                m_AuthenticationMechanismName.SetCountryName(e.GetValue().Arr[2].ToInteger());
                m_AuthenticationMechanismName.SetIdentifiedOrganization(e.GetValue().Arr[3].ToInteger());
                m_AuthenticationMechanismName.SetDlmsUA(e.GetValue().Arr[4].ToInteger());
                m_AuthenticationMechanismName.SetAuthenticationMechanismName(e.GetValue().Arr[5].ToInteger());
                m_AuthenticationMechanismName.SetMechanismId((DLMS_AUTHENTICATION)e.GetValue().Arr[6].ToInteger());
            }
        }
    }
    else if (e.GetIndex() == 7)
    {
        m_Secret.Clear();
        m_Secret.Set(e.GetValue().byteArr, e.GetValue().size);
    }
    else if (e.GetIndex() == 8)
    {
        m_AssociationStatus = (DLMS_ASSOCIATION_STATUS)e.GetValue().ToInteger();
    }
    else if (e.GetIndex() == 9)
    {
        GXHelpers::GetLogicalName(e.GetValue().byteArr, m_SecuritySetupReference);
    }
    else if (e.GetIndex() == 10)
    {
        m_UserList.clear();
        for (std::vector<CGXDLMSVariant >::iterator it = e.GetValue().Arr.begin(); it != e.GetValue().Arr.end(); ++it)
        {
            m_UserList.push_back(std::pair<unsigned char, std::string>(it->Arr[0].bVal, it->Arr[1].strVal));
        }
    }
    else if (e.GetIndex() == 11)
    {
        if (e.GetValue().Arr.size() == 2)
        {
            m_CurrentUser = std::pair<unsigned char, std::string>(e.GetValue().Arr[0].bVal, e.GetValue().Arr[1].strVal);
        }
        else
        {
            return DLMS_ERROR_CODE_INVALID_PARAMETER;
        }
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return DLMS_ERROR_CODE_OK;
}
