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

#include "../include/GXDLMSServer.h"
#include "../include/GXDLMS.h"
#include "../include/GXDLMSObjectFactory.h"
#include "../include/GXDLMSProfileGeneric.h"
#include "../include/GXDLMSAssociationShortName.h"
#include "../include/GXDLMSAssociationLogicalName.h"
#include "../include/GXDLMSClient.h"
#include "../include/GXAPDU.h"
#include "../include/GXSecure.h"
#include "../include/GXDLMSValueEventCollection.h"
#include "../include/GXDLMSSettings.h"
#include "../include/GXServerReply.h"

#define DLMS_WRAPPER_FRAME_LENGTH 8
#define DLMS_DATA_LENGTH_OFFSET 6

static void PrintfBuff(CGXByteBuffer *bb);

CGXDLMSServer::CGXDLMSServer(bool logicalNameReferencing,
    DLMS_INTERFACE_TYPE type) : m_Transaction(NULL), m_Settings(true)
{
    m_Hdlc = NULL;
    m_Wrapper = NULL;
    m_DataReceived = 0;
    m_Settings.SetUseLogicalNameReferencing(logicalNameReferencing);
    m_Settings.SetInterfaceType(type);
    if (GetUseLogicalNameReferencing())
    {
        SetConformance((DLMS_CONFORMANCE)(DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_ACTION |
            DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_SET_OR_WRITE |
            DLMS_CONFORMANCE_BLOCK_TRANSFER_WITH_GET_OR_READ |
            DLMS_CONFORMANCE_SET | DLMS_CONFORMANCE_SELECTIVE_ACCESS |
            DLMS_CONFORMANCE_ACTION | DLMS_CONFORMANCE_MULTIPLE_REFERENCES |
            DLMS_CONFORMANCE_GET | DLMS_CONFORMANCE_GENERAL_PROTECTION));
    }
    else
    {
        SetConformance((DLMS_CONFORMANCE)(DLMS_CONFORMANCE_INFORMATION_REPORT |
            DLMS_CONFORMANCE_READ | DLMS_CONFORMANCE_UN_CONFIRMED_WRITE |
            DLMS_CONFORMANCE_WRITE | DLMS_CONFORMANCE_PARAMETERIZED_ACCESS |
            DLMS_CONFORMANCE_MULTIPLE_REFERENCES |
            DLMS_CONFORMANCE_GENERAL_PROTECTION));
    }
    Reset();
}

CGXDLMSServer::CGXDLMSServer(
    CGXDLMSAssociationLogicalName* ln,
    CGXDLMSIecHdlcSetup* hdlc) :
    CGXDLMSServer(true, DLMS_INTERFACE_TYPE_HDLC)
{
    m_Settings.GetObjects().push_back(ln);
    m_Settings.GetObjects().push_back(hdlc);
    m_Hdlc = hdlc;
}

CGXDLMSServer::CGXDLMSServer(
    CGXDLMSAssociationLogicalName* ln,
    CGXDLMSTcpUdpSetup* wrapper) :
    CGXDLMSServer(true, DLMS_INTERFACE_TYPE_WRAPPER)
{
    m_Settings.GetObjects().push_back(ln);
    m_Settings.GetObjects().push_back(wrapper);
    m_Wrapper = wrapper;
}

CGXDLMSServer::CGXDLMSServer(
    CGXDLMSAssociationShortName* sn,
    CGXDLMSIecHdlcSetup* hdlc) :
    CGXDLMSServer(false, DLMS_INTERFACE_TYPE_HDLC)
{
    m_Settings.GetObjects().push_back(sn);
    m_Settings.GetObjects().push_back(hdlc);
    m_Hdlc = hdlc;
}

CGXDLMSServer::CGXDLMSServer(
    CGXDLMSAssociationShortName* sn,
    CGXDLMSTcpUdpSetup* wrapper) :
    CGXDLMSServer(false, DLMS_INTERFACE_TYPE_WRAPPER)
{
    m_Settings.GetObjects().push_back(sn);
    m_Settings.GetObjects().push_back(wrapper);
    m_Wrapper = wrapper;
}

CGXDLMSServer::~CGXDLMSServer()
{

}

/**
* @return HDLC settings.
*/
CGXDLMSIecHdlcSetup* CGXDLMSServer::GetHdlc() {
    return m_Hdlc;
}

/**
* @param value
*            HDLC settings.
*/
void CGXDLMSServer::SetHdlc(CGXDLMSIecHdlcSetup* value) {
    m_Hdlc = value;
}

/**
* @return Wrapper settings.
*/
CGXDLMSTcpUdpSetup* CGXDLMSServer::GetWrapper() {
    return m_Wrapper;
}

/**
* @param value
*            Wrapper settings.
*/
void CGXDLMSServer::SetWrapper(CGXDLMSTcpUdpSetup* value) {
    m_Wrapper = value;
}

CGXDLMSObjectCollection& CGXDLMSServer::GetItems()
{
    return m_Settings.GetObjects();
}

void CGXDLMSServer::SetCipher(CGXCipher* value)
{
    m_Settings.SetCipher(value);
}

CGXDLMSSettings& CGXDLMSServer::GetSettings()
{
    return m_Settings;
}

CGXByteBuffer& CGXDLMSServer::GetCtoSChallenge()
{
    return m_Settings.GetCtoSChallenge();
}

CGXByteBuffer& CGXDLMSServer::GetStoCChallenge()
{
    return m_Settings.GetStoCChallenge();
}

DLMS_INTERFACE_TYPE CGXDLMSServer::GetInterfaceType()
{
    return m_Settings.GetInterfaceType();
}

void CGXDLMSServer::SetStoCChallenge(CGXByteBuffer& value)
{
    m_Settings.SetUseCustomChallenge(value.GetSize() != 0);
    m_Settings.SetStoCChallenge(value);
}

void CGXDLMSServer::SetStartingPacketIndex(int value)
{
    m_Settings.SetBlockIndex(value);
}

int CGXDLMSServer::GetInvokeID()
{
    return m_Settings.GetInvokeID();
}

CGXDLMSLimits CGXDLMSServer::GetLimits()
{
    return m_Settings.GetLimits();
}

unsigned short CGXDLMSServer::GetMaxReceivePDUSize()
{
    return m_Settings.GetMaxServerPDUSize();
}

void CGXDLMSServer::SetMaxReceivePDUSize(unsigned short value)
{
    m_Settings.SetMaxServerPDUSize(value);
}

bool CGXDLMSServer::GetUseLogicalNameReferencing()
{
    return m_Settings.GetUseLogicalNameReferencing();
}

/**
 * @param value
 *            Is Logical Name referencing used.
 */
void CGXDLMSServer::SetUseLogicalNameReferencing(bool value)
{
    m_Settings.SetUseLogicalNameReferencing(value);
}

int CGXDLMSServer::Initialize()
{
    CGXDLMSObject* associationObject = NULL;
    m_Initialized = true;
    std::string ln;
    for (CGXDLMSObjectCollection::iterator it = m_Settings.GetObjects().begin(); it != m_Settings.GetObjects().end(); ++it)
    {
        (*it)->GetLogicalName(ln);
        if (ln.size() == 0)
        {
            //Invalid Logical Name.
            return DLMS_ERROR_CODE_INVALID_LOGICAL_NAME;
        }
        if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_PROFILE_GENERIC)
        {
            CGXDLMSProfileGeneric* pg = (CGXDLMSProfileGeneric*)(*it);
            if (pg->GetProfileEntries() < 1)
            {
                return DLMS_ERROR_CODE_INVALID_PARAMETER;
            }
            if (pg->GetCapturePeriod() > 0)
            {
                //TODO: Start thread. new GXProfileGenericUpdater(this, pg).start();
            }
        }
        else if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_ASSOCIATION_SHORT_NAME
            && !m_Settings.GetUseLogicalNameReferencing())
        {
            CGXDLMSObjectCollection& list = ((CGXDLMSAssociationShortName*)*it)->GetObjectList();
            if (list.size() == 0)
            {
                list.insert(list.end(), GetItems().begin(), GetItems().end());
            }
            associationObject = *it;
        }
        else if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME
            && m_Settings.GetUseLogicalNameReferencing())
        {
            CGXDLMSObjectCollection& list = ((CGXDLMSAssociationLogicalName*)*it)->GetObjectList();
            if (list.size() == 0)
            {
                list.insert(list.end(), GetItems().begin(), GetItems().end());
            }
            associationObject = *it;
            ((CGXDLMSAssociationLogicalName*)*it)->GetXDLMSContextInfo().SetMaxReceivePduSize(m_Settings.GetMaxServerPDUSize());
            ((CGXDLMSAssociationLogicalName*)*it)->GetXDLMSContextInfo().SetMaxSendPduSize(m_Settings.GetMaxServerPDUSize());
            ((CGXDLMSAssociationLogicalName*)*it)->GetXDLMSContextInfo().SetConformance(m_Settings.GetProposedConformance());
        }
    }
    if (associationObject == NULL)
    {
        if (GetUseLogicalNameReferencing())
        {
            CGXDLMSAssociationLogicalName* it2 = (CGXDLMSAssociationLogicalName*)CGXDLMSObjectFactory::CreateObject(DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME);
            CGXDLMSObjectCollection& list = it2->GetObjectList();
            GetItems().push_back(it2);
            list.insert(list.end(), GetItems().begin(), GetItems().end());
            ((CGXDLMSAssociationLogicalName*)it2)->GetXDLMSContextInfo().SetMaxReceivePduSize(m_Settings.GetMaxServerPDUSize());
            ((CGXDLMSAssociationLogicalName*)it2)->GetXDLMSContextInfo().SetMaxSendPduSize(m_Settings.GetMaxServerPDUSize());
            ((CGXDLMSAssociationLogicalName*)it2)->GetXDLMSContextInfo().SetConformance(m_Settings.GetProposedConformance());
        }
        else
        {
            CGXDLMSAssociationShortName* it2 = (CGXDLMSAssociationShortName*)CGXDLMSObjectFactory::CreateObject(DLMS_OBJECT_TYPE_ASSOCIATION_SHORT_NAME);
            CGXDLMSObjectCollection& list = it2->GetObjectList();
            GetItems().push_back(it2);
            list.insert(list.end(), GetItems().begin(), GetItems().end());
        }
    }
    // Arrange items by Short Name.

    if (!m_Settings.GetUseLogicalNameReferencing())
    {
        UpdateShortNames(false);
    }
    return 0;
}


int CGXDLMSServer::UpdateShortNames()
{
    return UpdateShortNames(true);
}

int CGXDLMSServer::UpdateShortNames(bool force)
{
    int ret;
    short sn = 0xA0;
    unsigned char offset, count;
    for (CGXDLMSObjectCollection::iterator it = m_Settings.GetObjects().begin();
        it != m_Settings.GetObjects().end(); ++it)
    {
        if ((*it)->GetObjectType() == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME ||
            (*it)->GetObjectType() == DLMS_OBJECT_TYPE_ASSOCIATION_SHORT_NAME)
        {
            continue;
        }
        // Generate Short Name if not given.
        if (force || (*it)->GetShortName() == 0)
        {
            (*it)->SetShortName(sn);
            // Add method index addresses.
            if ((ret = CGXDLMS::GetActionInfo((*it)->GetObjectType(), offset, count)) != 0)
            {
                return ret;
            }
            if (count != 0)
            {
                sn += offset + (8 * count);
            }
            else
            {
                // If there are no methods.
                // Add attribute index addresses.
                sn += 8 * (*it)->GetAttributeCount();
            }
        }
        else
        {
            sn = (*it)->GetShortName();
        }
    }
    return 0;
}

void CGXDLMSServer::Reset(bool connected)
{
    if (m_Transaction != NULL)
    {
        delete m_Transaction;
        m_Transaction = NULL;
    }
    m_Settings.SetCount(0);
    m_Settings.SetIndex(0);
    m_Settings.SetConnected(false);
    m_ReceivedData.Clear();
    m_ReplyData.Clear();
    if (!connected)
    {
        m_Info.Clear();
        m_Settings.SetServerAddress(0);
        m_Settings.SetClientAddress(0);

        //#ecdsa#
//        m_Settings.GetCipher()->SetSecurity(DLMS_SECURITY_NONE);
    }

    m_Settings.SetAuthentication(DLMS_AUTHENTICATION_NONE);
    if (m_Settings.GetCipher() != NULL)
    {
        if (!connected)
        {
            m_Settings.GetCipher()->Reset();
        }
        else
        {
            m_Settings.GetCipher()->SetSecurity(DLMS_SECURITY_NONE);
        }
    }
}

void CGXDLMSServer::Reset()
{
    Reset(false);
}

//#ecdsa#
static unsigned char StoC[] =
{
	0x18,0xE9,0x5F,0xFE,0x3A,0xD0,0xDC,0xAB,0xDC,0x5D,0x0D,
	0x14,0x1D,0xC9,0x87,0xE2,0x70,0xCB,0x0A,0x39,0x59,0x48,
	0xD4,0x23,0x1B,0x09,0xDE,0x65,0x79,0x88,0x36,0x57
};

/**
    * Parse AARQ request that client send and returns AARE request.
    *
    * @return Reply to the client.
    */
int CGXDLMSServer::HandleAarqRequest(
    CGXByteBuffer& data,
    CGXDLMSConnectionEventArgs& connectionInfo)
{
    int ret;
    CGXByteBuffer error;
    DLMS_ASSOCIATION_RESULT result;
    DLMS_SOURCE_DIAGNOSTIC diagnostic;
    m_Settings.GetCtoSChallenge().Clear();
    m_Settings.GetStoCChallenge().Clear();
    if (m_Settings.GetInterfaceType() == DLMS_INTERFACE_TYPE_WRAPPER)
    {
        Reset(true);
    }

    ret = CGXAPDU::ParsePDU(m_Settings, m_Settings.GetCipher(), data, result, diagnostic);

    if (ret != 0)
    {
        return ret;
    }

    if (result == DLMS_ASSOCIATION_RESULT_ACCEPTED)
    {
        if (m_Settings.GetDLMSVersion() != 6)
        {
            result = DLMS_ASSOCIATION_RESULT_PERMANENT_REJECTED;
            diagnostic = DLMS_SOURCE_DIAGNOSTIC_NO_REASON_GIVEN;
            error.SetUInt8(0xE);
            error.SetUInt8(DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR);
            error.SetUInt8(DLMS_SERVICE_ERROR_INITIATE);
            error.SetUInt8(DLMS_INITIATE_DLMS_VERSION_TOO_LOW);
            m_Settings.SetDLMSVersion(6);
        }
        else if (m_Settings.GetMaxPduSize() < 64)
        {
            result = DLMS_ASSOCIATION_RESULT_PERMANENT_REJECTED;
            diagnostic = DLMS_SOURCE_DIAGNOSTIC_NO_REASON_GIVEN;
            error.SetUInt8(0xE);
            error.SetUInt8(DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR);
            error.SetUInt8(DLMS_SERVICE_ERROR_INITIATE);
            error.SetUInt8(DLMS_INITIATE_PDU_SIZE_TOOSHORT);
            m_Settings.SetMaxReceivePDUSize(64);
        }
        else if (m_Settings.GetNegotiatedConformance() == 0)
        {
            result = DLMS_ASSOCIATION_RESULT_PERMANENT_REJECTED;
            diagnostic = DLMS_SOURCE_DIAGNOSTIC_NO_REASON_GIVEN;
            error.SetUInt8(0xE);
            error.SetUInt8(DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR);
            error.SetUInt8(DLMS_SERVICE_ERROR_INITIATE);
            error.SetUInt8(DLMS_INITIATE_INCOMPATIBLE_CONFORMANCE);
        }
        else if (diagnostic != DLMS_SOURCE_DIAGNOSTIC_NONE)
        {
            result = DLMS_ASSOCIATION_RESULT_PERMANENT_REJECTED;
            diagnostic = DLMS_SOURCE_DIAGNOSTIC_APPLICATION_CONTEXT_NAME_NOT_SUPPORTED;
            InvalidConnection(connectionInfo);
        }
        else
        {
            diagnostic = ValidateAuthentication(m_Settings.GetAuthentication(), m_Settings.GetPassword());
            if (diagnostic != DLMS_SOURCE_DIAGNOSTIC_NONE)
            {
                result = DLMS_ASSOCIATION_RESULT_PERMANENT_REJECTED;
                InvalidConnection(connectionInfo);
            }
            else if (m_Settings.GetAuthentication() > DLMS_AUTHENTICATION_LOW)
            {
                // If High authentication is used.
                result = DLMS_ASSOCIATION_RESULT_ACCEPTED;
                diagnostic = DLMS_SOURCE_DIAGNOSTIC_AUTHENTICATION_REQUIRED;
            }
            else
            {
                Connected(connectionInfo);
                m_Settings.SetConnected(true);
            }
        }
    }
    if (m_Settings.GetAuthentication() > DLMS_AUTHENTICATION_LOW)
    { // If High authentication is used.

        //[#ecdsa#] injection of StoC
		if (m_test_case == BAD_PATH_IDENTICAL_CHALLENGES)
		{
			// Test case: BAD_PATH_IDENTICAL_CHALLENGES
			m_Settings.SetStoCChallenge(m_Settings.GetCtoSChallenge());
		}
		else  //if (m_test_case > NO_TEST)
        {
			// inject challenge from green book example
			// regular
			CGXByteBuffer challenge;
			challenge.Set(StoC, sizeof(StoC));
			m_Settings.SetStoCChallenge(challenge);
        }
/*      else
        {
			// regular
			CGXByteBuffer challenge;
			if ((ret = CGXSecure::GenerateChallenge(m_Settings.GetAuthentication(), challenge)) != 0)
			{
				return ret;
			}
			m_Settings.SetStoCChallenge(challenge);
        }
*/

        if (m_Settings.GetUseLogicalNameReferencing())
        {
            unsigned char l[] = { 0,0,40,0,0,255 };
            CGXDLMSAssociationLogicalName* ln = (CGXDLMSAssociationLogicalName*)m_Settings.GetObjects().FindByLN(DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, l);
            if (ln != NULL)
            {
                if (m_Settings.GetCipher() == NULL || m_Settings.GetCipher()->GetSecurity() == DLMS_SECURITY_NONE)
                {
                    ln->GetApplicationContextName().SetContextId(DLMS_APPLICATION_CONTEXT_NAME_LOGICAL_NAME);
                }
                else
                {
                    ln->GetApplicationContextName().SetContextId(DLMS_APPLICATION_CONTEXT_NAME_LOGICAL_NAME_WITH_CIPHERING);
                }
                ln->GetAuthenticationMechanismName().SetMechanismId(m_Settings.GetAuthentication());
                ln->SetAssociationStatus(DLMS_ASSOCIATION_STATUS_ASSOCIATION_PENDING);
            }
        }
    }
    else
    {
        unsigned char l[] = { 0,0,40,0,0,255 };
        CGXDLMSAssociationLogicalName* ln = (CGXDLMSAssociationLogicalName*)m_Settings.GetObjects().FindByLN(DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME, l);
        if (ln != NULL)
        {
            if (m_Settings.GetCipher() == NULL || m_Settings.GetCipher()->GetSecurity() == DLMS_SECURITY_NONE)
            {
                ln->GetApplicationContextName().SetContextId(DLMS_APPLICATION_CONTEXT_NAME_LOGICAL_NAME);
            }
            else
            {
                ln->GetApplicationContextName().SetContextId(DLMS_APPLICATION_CONTEXT_NAME_LOGICAL_NAME_WITH_CIPHERING);
            }
            ln->GetAuthenticationMechanismName().SetMechanismId(m_Settings.GetAuthentication());
            ln->SetAssociationStatus(DLMS_ASSOCIATION_STATUS_ASSOCIATED);
        }
    }
    if (m_Settings.GetInterfaceType() == DLMS_INTERFACE_TYPE_HDLC)
    {
        m_ReplyData.Set(LLC_REPLY_BYTES, 3);
    }
    // Generate AARE packet.
    m_Settings.ResetFrameSequence();

    return CGXAPDU::GenerateAARE(m_Settings, m_ReplyData, result, diagnostic, m_Settings.GetCipher(), &error, NULL, (int)m_test_case);
}

/**
 * Parse SNRM Request. If server do not accept client empty byte array is
 * returned.
 *
 * @return Returns returned UA packet.
 */
int CGXDLMSServer::HandleSnrmRequest(
    CGXDLMSSettings& settings,
    CGXByteBuffer& data,
    CGXByteBuffer& reply)
{
    int ret;
    Reset(true);
    if ((ret = CGXDLMS::ParseSnrmUaResponse(data, &m_Settings.GetLimits())) != 0)
    {
        return ret;
    }
    reply.SetUInt8(0x81); // FromatID
    reply.SetUInt8(0x80); // GroupID
    reply.SetUInt8(0); // Length

    if (m_Hdlc != NULL)
    {
        //If client wants send larger HDLC frames what meter accepts.
        if (m_Settings.GetLimits().GetMaxInfoTX() > m_Hdlc->GetMaximumInfoLengthReceive())
        {
            m_Settings.GetLimits().SetMaxInfoTX(m_Hdlc->GetMaximumInfoLengthReceive());
        }
        //If client wants receive larger HDLC frames what meter accepts.
        if (m_Settings.GetLimits().GetMaxInfoRX() > m_Hdlc->GetMaximumInfoLengthTransmit())
        {
            m_Settings.GetLimits().SetMaxInfoRX(m_Hdlc->GetMaximumInfoLengthTransmit());
        }
        //If client asks higher window size what meter accepts.
        if (m_Settings.GetLimits().GetWindowSizeTX() > m_Hdlc->GetWindowSizeReceive())
        {
            m_Settings.GetLimits().SetWindowSizeTX(m_Hdlc->GetWindowSizeReceive());
        }
        //If client asks higher window size what meter accepts.
        if (m_Settings.GetLimits().GetWindowSizeRX() > m_Hdlc->GetWindowSizeTransmit())
        {
            m_Settings.GetLimits().SetWindowSizeRX(m_Hdlc->GetWindowSizeTransmit());
        }
    }

    reply.SetUInt8(HDLC_INFO_MAX_INFO_TX);
    CGXDLMS::AppendHdlcParameter(reply, GetLimits().GetMaxInfoTX());

    reply.SetUInt8(HDLC_INFO_MAX_INFO_RX);
    CGXDLMS::AppendHdlcParameter(reply, GetLimits().GetMaxInfoRX());

    reply.SetUInt8(HDLC_INFO_WINDOW_SIZE_TX);
    reply.SetUInt8(4);
    reply.SetUInt32(m_Settings.GetLimits().GetWindowSizeTX());

    reply.SetUInt8(HDLC_INFO_WINDOW_SIZE_RX);
    reply.SetUInt8(4);
    reply.SetUInt32(m_Settings.GetLimits().GetWindowSizeRX());
    int len = reply.GetSize() - 3;
    reply.SetUInt8(2, len); // Length
    return ret;
}

/**
 * Generates disconnect request.
 *
 * @return Disconnect request.
 */
int GenerateDisconnectRequest(CGXDLMSSettings& settings, CGXByteBuffer& reply)
{
    if (settings.GetInterfaceType() == DLMS_INTERFACE_TYPE_WRAPPER)
    {
        reply.SetUInt8(0x63);
        reply.SetUInt8(0x0);
    }
    else
    {
        reply.SetUInt8(0x81); // FromatID
        reply.SetUInt8(0x80); // GroupID
        reply.SetUInt8(0); // Length

        reply.SetUInt8(HDLC_INFO_MAX_INFO_TX);
        CGXDLMS::AppendHdlcParameter(reply, settings.GetLimits().GetMaxInfoTX());

        reply.SetUInt8(HDLC_INFO_MAX_INFO_RX);
        CGXDLMS::AppendHdlcParameter(reply, settings.GetLimits().GetMaxInfoRX());

        reply.SetUInt8(HDLC_INFO_WINDOW_SIZE_TX);
        reply.SetUInt8(4);
        reply.SetUInt32(settings.GetLimits().GetWindowSizeTX());

        reply.SetUInt8(HDLC_INFO_WINDOW_SIZE_RX);
        reply.SetUInt8(4);
        reply.SetUInt32(settings.GetLimits().GetWindowSizeRX());
        int len = reply.GetSize() - 3;
        reply.SetUInt8(2, len); // Length.
    }
    return 0;
}

int ReportError(CGXDLMSSettings& settings, DLMS_COMMAND command, DLMS_ERROR_CODE error, CGXByteBuffer& reply)
{
    int ret;
    DLMS_COMMAND cmd;
    CGXByteBuffer data;
    switch (command)
    {
    case DLMS_COMMAND_READ_REQUEST:
        cmd = DLMS_COMMAND_READ_RESPONSE;
        break;
    case DLMS_COMMAND_WRITE_REQUEST:
        cmd = DLMS_COMMAND_WRITE_RESPONSE;
        break;
    case DLMS_COMMAND_GET_REQUEST:
        cmd = DLMS_COMMAND_GET_RESPONSE;
        break;
    case DLMS_COMMAND_SET_REQUEST:
        cmd = DLMS_COMMAND_SET_RESPONSE;
        break;
    case DLMS_COMMAND_METHOD_REQUEST:
        cmd = DLMS_COMMAND_METHOD_RESPONSE;
        break;
    default:
        // Return HW error and close connection.
        cmd = DLMS_COMMAND_NONE;
        break;
    }

    if (settings.GetUseLogicalNameReferencing())
    {
        CGXDLMSLNParameters p(&settings, 0, cmd, 1, NULL, NULL, error);
        ret = CGXDLMS::GetLNPdu(p, 1, data);
    }
    else
    {
        CGXByteBuffer bb;
        bb.SetUInt8(error);
        CGXDLMSSNParameters p(&settings, cmd, 1, 1, NULL, &bb);
        ret = CGXDLMS::GetSNPdu(p, data);
    }
    if (ret == 0)
    {
        if (settings.GetInterfaceType() == DLMS_INTERFACE_TYPE_WRAPPER)
        {
            ret = CGXDLMS::GetWrapperFrame(settings, data, reply);
        }
        else
        {
            ret = CGXDLMS::GetHdlcFrame(settings, 0, &data, reply);
        }
    }
    return ret;
}

int CGXDLMSServer::HandleSetRequest(
    CGXByteBuffer& data,
    short type,
    CGXDLMSLNParameters& p)
{
    CGXDataInfo i;
    CGXDLMSVariant value;
    int ret;
    unsigned char index, ch;
    unsigned short tmp;
    if ((ret = data.GetUInt16(&tmp)) != 0)
    {
        return ret;
    }
    DLMS_OBJECT_TYPE ci = (DLMS_OBJECT_TYPE)tmp;
    unsigned char * ln;
    ln = data.GetData() + data.GetPosition();
    data.SetPosition(data.GetPosition() + 6);
    // Attribute index.
    if ((ret = data.GetUInt8(&index)) != 0)
    {
        return ret;
    }
    // Get Access Selection.
    if ((ret = data.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    if (type == 2)
    {
        unsigned long size, blockNumber;
        if ((ret = data.GetUInt8(&ch)) != 0)
        {
            return ret;
        }
        p.SetMultipleBlocks(ch == 0);
        ret = data.GetUInt32(&blockNumber);
        if (ret != 0)
        {
            return ret;
        }
        if (blockNumber != m_Settings.GetBlockIndex())
        {
            p.SetStatus(DLMS_ERROR_CODE_DATA_BLOCK_NUMBER_INVALID);
            return 0;
        }
        m_Settings.IncreaseBlockIndex();
        ret = GXHelpers::GetObjectCount(data, size);
        if (ret != 0)
        {
            return ret;
        }
        unsigned long realSize = data.GetSize() - data.GetPosition();
        if (size != realSize)
        {
            p.SetStatus(DLMS_ERROR_CODE_BLOCK_UNAVAILABLE);
            return 0;
        }
    }
    if (!p.IsMultipleBlocks())
    {
        m_Settings.ResetBlockIndex();
        ret = GXHelpers::GetData(data, i, value);
        if (ret != 0)
        {
            return ret;
        }
    }

    CGXDLMSObject* obj = m_Settings.GetObjects().FindByLN(ci, ln);
    if (obj == NULL)
    {
        std::string name;
        GXHelpers::GetLogicalName(ln, name);
        obj = FindObject(ci, 0, name);
    }
    // If target is unknown.
    if (obj == NULL)
    {
        // Device reports a undefined object.
        p.SetStatus(DLMS_ERROR_CODE_UNAVAILABLE_OBJECT);
    }
    else
    {
        CGXDLMSValueEventArg* e = new CGXDLMSValueEventArg(this, obj, index);
        e->SetValue(value);
        CGXDLMSValueEventCollection list;
        list.push_back(e);
        DLMS_ACCESS_MODE am = GetAttributeAccess(e);
        // If write is denied.
        if (am != DLMS_ACCESS_MODE_WRITE && am != DLMS_ACCESS_MODE_READ_WRITE)
        {
            //Read Write denied.
            p.SetStatus(DLMS_ERROR_CODE_READ_WRITE_DENIED);
        }
        else
        {
            if (value.vt == DLMS_DATA_TYPE_OCTET_STRING)
            {
                DLMS_DATA_TYPE dt;
                ret = obj->GetDataType(index, dt);
                if (ret != 0)
                {
                    return ret;
                }
                if (dt != DLMS_DATA_TYPE_NONE && dt != DLMS_DATA_TYPE_OCTET_STRING)
                {
                    CGXByteBuffer tmp;
                    tmp.Set(value.byteArr, value.GetSize());
                    value.Clear();
                    if ((ret = CGXDLMSClient::ChangeType(tmp, dt, value)) != 0)
                    {
                        return ret;
                    }
                }
            }
            if (p.IsMultipleBlocks())
            {
                m_Transaction = new CGXDLMSLongTransaction(list, DLMS_COMMAND_GET_REQUEST, data);
            }
            PreWrite(list);
            if (e->GetError() != 0)
            {
                p.SetStatus(e->GetError());
            }
            else if (!e->GetHandled() && !p.IsMultipleBlocks())
            {
                obj->SetValue(m_Settings, *e);
                PostWrite(list);
            }
        }
    }
    return ret;
}

int CGXDLMSServer::HanleSetRequestWithDataBlock(CGXByteBuffer& data, CGXDLMSLNParameters& p)
{
    CGXDataInfo reply;
    int ret;
    unsigned long blockNumber, size;
    unsigned char ch;
    if ((ret = data.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    p.SetMultipleBlocks(ch == 0);
    if ((ret = data.GetUInt32(&blockNumber)) != 0)
    {
        return ret;
    }
    if (blockNumber != m_Settings.GetBlockIndex())
    {
        p.SetStatus(DLMS_ERROR_CODE_DATA_BLOCK_NUMBER_INVALID);
    }
    else
    {
        m_Settings.IncreaseBlockIndex();
        if ((ret = GXHelpers::GetObjectCount(data, size)) != 0)
        {
            return ret;
        }
        unsigned long realSize = data.GetSize() - data.GetPosition();
        if (size != realSize)
        {
            p.SetStatus(DLMS_ERROR_CODE_BLOCK_UNAVAILABLE);
        }
        m_Transaction->GetData().Set(&data, data.GetPosition());
        // If all data is received.
        if (!p.IsMultipleBlocks())
        {
            CGXDLMSVariant value;
            if ((ret != GXHelpers::GetData(m_Transaction->GetData(), reply, value)) != 0)
            {
                return ret;
            }
            CGXDLMSValueEventArg * target = *m_Transaction->GetTargets().begin();
            if (value.vt == DLMS_DATA_TYPE_OCTET_STRING)
            {
                DLMS_DATA_TYPE dt;
                ret = target->GetTarget()->GetDataType(target->GetIndex(), dt);
                if (dt != DLMS_DATA_TYPE_NONE && dt != DLMS_DATA_TYPE_OCTET_STRING)
                {
                    CGXByteBuffer bb;
                    bb.Set(value.byteArr, value.GetSize());
                    value.Clear();
                    if ((ret = CGXDLMSClient::ChangeType(bb, dt, value)) != 0)
                    {
                        return ret;
                    }
                }
            }
            target->SetValue(value);
            PreWrite(m_Transaction->GetTargets());
            if (!target->GetHandled() && !p.IsMultipleBlocks())
            {
                target->GetTarget()->SetValue(m_Settings, *target);
                PostWrite(m_Transaction->GetTargets());
            }
            if (m_Transaction != NULL)
            {
                delete m_Transaction;
                m_Transaction = NULL;
            }
            m_Settings.ResetBlockIndex();
        }
    }
    p.SetMultipleBlocks(true);
    return 0;
}

/**
    * Generate confirmed service error.
    *
    * @param service
    *            Confirmed service error.
    * @param type
    *            Service error.
    * @param code
    *            code
    * @return
    */
void GenerateConfirmedServiceError(
    DLMS_CONFIRMED_SERVICE_ERROR service,
    DLMS_SERVICE_ERROR type,
    unsigned char code, CGXByteBuffer& data)
{
    data.SetUInt8(DLMS_COMMAND_CONFIRMED_SERVICE_ERROR);
    data.SetUInt8(service);
    data.SetUInt8(type);
    data.SetUInt8(code);
}

int CGXDLMSServer::HandleSetRequest(CGXByteBuffer& data)
{
    CGXDLMSVariant value;
    unsigned char type, invoke;
    int ret;
    CGXDataInfo i;
    CGXByteBuffer bb;
    // Return error if connection is not established.
    if (!m_Settings.IsConnected())
    {
        GenerateConfirmedServiceError(DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR,
            DLMS_SERVICE_ERROR_SERVICE, DLMS_SERVICE_UNSUPPORTED,
            m_ReplyData);
        return 0;
    }
    // Get type.
    if ((ret = data.GetUInt8(&type)) != 0)
    {
        return ret;
    }
    // Get invoke ID and priority.
    if ((ret = data.GetUInt8(&invoke)) != 0)
    {
        return ret;
    }
    CGXDLMSLNParameters p(&m_Settings, invoke, DLMS_COMMAND_SET_RESPONSE, type, NULL, NULL, 0);
    if (type == DLMS_SET_COMMAND_TYPE_NORMAL || type == DLMS_SET_COMMAND_TYPE_FIRST_DATABLOCK)
    {
        ret = HandleSetRequest(data, type, p);
    }
    else if (type == DLMS_SET_COMMAND_TYPE_WITH_DATABLOCK)
    {
        // Set Request With Data Block
        ret = HanleSetRequestWithDataBlock(data, p);
    }
    else
    {
        m_Settings.ResetBlockIndex();
        p.SetStatus(DLMS_ERROR_CODE_HARDWARE_FAULT);
    }
    return CGXDLMS::GetLNPdu(p, 1, m_ReplyData);
}

unsigned short CGXDLMSServer::GetRowsToPdu(CGXDLMSProfileGeneric* pg)
{
    DLMS_DATA_TYPE dt;
    int rowsize = 0;
    // Count how many rows we can fit to one PDU.
    for (std::vector<std::pair<CGXDLMSObject*, CGXDLMSCaptureObject*> >::iterator it = pg->GetCaptureObjects().begin();
        it != pg->GetCaptureObjects().end(); ++it)
    {
        it->first->GetDataType(it->second->GetAttributeIndex(), dt);
        if (dt == DLMS_DATA_TYPE_OCTET_STRING)
        {
            it->first->GetUIDataType(it->second->GetAttributeIndex(), dt);
            if (dt == DLMS_DATA_TYPE_DATETIME)
            {
                rowsize += GXHelpers::GetDataTypeSize(DLMS_DATA_TYPE_DATETIME);
            }
            else if (dt == DLMS_DATA_TYPE_DATE)
            {
                rowsize += GXHelpers::GetDataTypeSize(DLMS_DATA_TYPE_DATE);
            }
            else if (dt == DLMS_DATA_TYPE_TIME)
            {
                rowsize += GXHelpers::GetDataTypeSize(DLMS_DATA_TYPE_TIME);
            }
        }
        else if (dt == DLMS_DATA_TYPE_NONE)
        {
            rowsize += 2;
        }
        else
        {
            rowsize += GXHelpers::GetDataTypeSize(dt);
        }
    }
    if (rowsize != 0)
    {
        return m_Settings.GetMaxPduSize() / rowsize;
    }
    return 0;
}

int CGXDLMSServer::GetRequestNormal(CGXByteBuffer& data, unsigned char invokeID)
{
    CGXByteBuffer bb;
    DLMS_ERROR_CODE status = DLMS_ERROR_CODE_OK;
    m_Settings.SetCount(0);
    m_Settings.SetIndex(0);
    m_Settings.ResetBlockIndex();
    CGXDLMSValueEventCollection arr;
    unsigned char attributeIndex;
    int ret;
    unsigned char *ln;
    // CI
    unsigned short tmp;
    if ((ret = data.GetUInt16(&tmp)) != 0)
    {
        return ret;
    }
    DLMS_OBJECT_TYPE ci = (DLMS_OBJECT_TYPE)tmp;
    ln = data.GetData() + data.GetPosition();
    data.SetPosition(data.GetPosition() + 6);
    // Attribute Id
    if ((ret = data.GetUInt8(&attributeIndex)) != 0)
    {
        return ret;
    }

    CGXDLMSObject* obj = m_Settings.GetObjects().FindByLN(ci, ln);
    if (obj == NULL)
    {
        std::string name;
        GXHelpers::GetLogicalName(ln, name);
        obj = FindObject(ci, 0, name);
    }
    // Access selection
    unsigned char selection, selector = 0;
    if ((ret = data.GetUInt8(&selection)) != 0)
    {
        return ret;
    }
    CGXDLMSVariant parameters;
    if (selection != 0)
    {
        if ((ret = data.GetUInt8(&selector)) != 0)
        {
            return ret;
        }
        CGXDataInfo i;
        if ((ret = GXHelpers::GetData(data, i, parameters)) != 0)
        {
            return ret;
        }
    }
    CGXDLMSValueEventArg* e = new CGXDLMSValueEventArg(this, obj, attributeIndex, selector, parameters);
    e->SetInvokeId(invokeID);
    arr.push_back(e);
    if (obj == NULL)
    {
        // "Access Error : Device reports a undefined object."
        status = DLMS_ERROR_CODE_UNDEFINED_OBJECT;
    }
    else
    {
        if (GetAttributeAccess(e) == DLMS_ACCESS_MODE_NONE)
        {
            // Read Write denied.
            status = DLMS_ERROR_CODE_READ_WRITE_DENIED;
        }
        else
        {
            if (obj->GetObjectType() == DLMS_OBJECT_TYPE_PROFILE_GENERIC && attributeIndex == 2)
            {
                e->SetRowToPdu(GetRowsToPdu((CGXDLMSProfileGeneric*)obj));
            }
            PreRead(arr);
            if (!e->GetHandled())
            {
                m_Settings.SetCount(e->GetRowEndIndex() - e->GetRowBeginIndex());
                if ((ret = obj->GetValue(m_Settings, *e)) != 0)
                {
                    status = DLMS_ERROR_CODE_HARDWARE_FAULT;
                }
                PostRead(arr);
            }
            if (status == 0)
            {
                status = e->GetError();
            }
            CGXDLMSVariant& value = e->GetValue();
            if (e->IsByteArray() && value.vt == DLMS_DATA_TYPE_OCTET_STRING)
            {
                // If byte array is added do not add type.
                bb.Set(value.byteArr, value.GetSize());
            }
            else if ((ret = CGXDLMS::AppendData(obj, attributeIndex, bb, value)) != 0)
            {
                status = DLMS_ERROR_CODE_HARDWARE_FAULT;
            }
        }
    }
    CGXDLMSLNParameters p(&m_Settings, invokeID, DLMS_COMMAND_GET_RESPONSE, 1, NULL, &bb, status);

    ret = CGXDLMS::GetLNPdu(p, 1, m_ReplyData);
    if (m_Settings.GetCount() != m_Settings.GetIndex()
        || bb.GetSize() != bb.GetPosition())
    {
        if (m_Transaction != NULL)
        {
            delete m_Transaction;
            m_Transaction = NULL;
        }
        m_Transaction = new CGXDLMSLongTransaction(arr, DLMS_COMMAND_GET_REQUEST, bb);
    }
    return ret;
}

int CGXDLMSServer::GetRequestNextDataBlock(CGXByteBuffer& data, unsigned char invokeID, bool streaming)
{
    CGXByteBuffer bb;
    int ret;
    unsigned long index;
    // Get block index.
    if (!streaming)
    {
		if ((ret = data.GetUInt32(&index)) != 0)
		{
			return ret;
		}
		if (index != m_Settings.GetBlockIndex())
		{
			CGXDLMSLNParameters p(&m_Settings, invokeID, DLMS_COMMAND_GET_RESPONSE, 2,
				0, &bb,
				DLMS_ERROR_CODE_DATA_BLOCK_NUMBER_INVALID);
			return CGXDLMS::GetLNPdu(p, 1, m_ReplyData);
		}
    }

    m_Settings.IncreaseBlockIndex();
	CGXDLMSLNParameters p(&m_Settings, invokeID,
		 streaming ? DLMS_COMMAND_GENERAL_BLOCK_TRANSFER
				 : DLMS_COMMAND_GET_RESPONSE,
		 2, 0, &bb, DLMS_ERROR_CODE_OK);
	p.SetStreaming(streaming);
	p.SetWindowSize(m_Settings.GetWindowSize());

	// If transaction is not in progress.
	if (m_Transaction == NULL)
	{
		p.SetStatus(DLMS_ERROR_CODE_NO_LONG_GET_OR_READ_IN_PROGRESS);
	}
    else
    {

		bb.Set(&m_Transaction->GetData());
		bool moreData = m_Settings.GetIndex() != m_Settings.GetCount();
		if (moreData) {
		 // If there is multiple blocks on the buffer.
		 // This might happen when Max PDU size is very small.
		 if (bb.GetSize() < m_Settings.GetMaxPduSize())
		 {

			 CGXDLMSVariant value;
			  for (std::vector<CGXDLMSValueEventArg*>::iterator arg = m_Transaction->GetTargets().begin();
				  arg != m_Transaction->GetTargets().end(); ++arg)
			  {
				  PreRead(m_Transaction->GetTargets());
				  if (!(*arg)->GetHandled())
				  {
					  if ((ret = (*arg)->GetTarget()->GetValue(m_Settings, *(*arg))) != 0)
					  {
						  return ret;
					  }
					  std::vector<CGXDLMSValueEventArg*> arr;
					  arr.push_back(*arg);
					  PostRead(arr);
				  }
				  value = (*arg)->GetValue();
				  // Add data.
				  if ((*arg)->IsByteArray() && value.vt == DLMS_DATA_TYPE_OCTET_STRING)
				  {
					  // If byte array is added do not add type.
					  bb.Set(value.byteArr, value.GetSize());
				  }
				  else if ((ret = CGXDLMS::AppendData((*arg)->GetTarget(), (*arg)->GetIndex(), bb, value)) != 0)
				  {
					  return DLMS_ERROR_CODE_HARDWARE_FAULT;
				  }

			  }

#if 0 //TODO: we might need to change above for loop code with below lines
			 Object value;
			 for (ValueEventArgs arg : server.getTransaction()
					 .getTargets()) {
				 arg.setInvokeId(p.getInvokeId());
				 server.notifyRead(new ValueEventArgs[] { arg });
				 if (arg.getHandled()) {
					 value = arg.getValue();
				 } else {
					 value = arg.getTarget().getValue(settings, arg);
				 }
				 p.setInvokeId(arg.getInvokeId());
				 // Add data.
				 if (arg.isByteArray()) {
					 bb.set((byte[]) value);
				 } else {
					 GXDLMS.appendData(arg.getTarget(), arg.getIndex(),
							 bb, value);
				 }
			 }
#endif
			 moreData = m_Settings.GetIndex() != m_Settings.GetCount();
		 }


		}

		p.SetMultipleBlocks(true);
		CGXDLMS::GetLNPdu(p, 1, m_ReplyData);
		if (moreData || bb.GetSize() - bb.GetPosition() != 0)
		{
			m_Transaction->SetData(bb);
		}
		else
		{
			delete m_Transaction;
			m_Transaction = NULL;
			m_Settings.ResetBlockIndex();
		}
    }
    return ret;
}

int CGXDLMSServer::GetRequestWithList(CGXByteBuffer& data, unsigned char invokeID)
{
    CGXDLMSValueEventCollection list;
    CGXByteBuffer bb;
    int ret;
    unsigned char attributeIndex;
    unsigned short id;
    unsigned long pos, cnt;
    if ((ret = GXHelpers::GetObjectCount(data, cnt)) != 0)
    {
        return ret;
    }
    GXHelpers::SetObjectCount(cnt, bb);
    for (pos = 0; pos != cnt; ++pos)
    {
        if ((ret = data.GetUInt16(&id)) != 0)
        {
            return ret;
        }
        DLMS_OBJECT_TYPE ci = (DLMS_OBJECT_TYPE)id;
        unsigned char * ln;
        ln = data.GetData() + data.GetPosition();
        data.SetPosition(data.GetPosition() + 6);
        if ((ret = data.GetUInt8(&attributeIndex)) != 0)
        {
            return ret;
        }
        CGXDLMSObject* obj = m_Settings.GetObjects().FindByLN(ci, ln);
        if (obj == NULL)
        {
            std::string name;
            GXHelpers::GetLogicalName(ln, name);
            obj = FindObject(ci, 0, name);
        }
        if (obj == NULL)
        {
            // Access Error : Device reports a undefined object.
            CGXDLMSValueEventArg* e = new CGXDLMSValueEventArg(this, obj, attributeIndex);
            e->SetError(DLMS_ERROR_CODE_UNDEFINED_OBJECT);
            list.push_back(e);
        }
        else
        {
            // AccessSelection
            unsigned char selection, selector = 0;
            CGXDLMSVariant parameters;
            if ((ret = data.GetUInt8(&selection)) != 0)
            {
                return ret;
            }
            if (selection != 0)
            {
                if ((ret = data.GetUInt8(&selector)) != 0)
                {
                    return ret;
                }
                CGXDataInfo i;
                if ((ret = GXHelpers::GetData(data, i, parameters)) != 0)
                {
                    return ret;
                }
            }
            CGXDLMSValueEventArg *arg = new CGXDLMSValueEventArg(this, obj, attributeIndex, selector, parameters);
            list.push_back(arg);
            if (GetAttributeAccess(arg) == DLMS_ACCESS_MODE_NONE)
            {
                // Read Write denied.
                arg->SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
            }
        }
    }
    PreRead(list);
    pos = 0;
    for (std::vector<CGXDLMSValueEventArg*>::iterator it = list.begin(); it != list.end(); ++it)
    {
        if (!(*it)->GetHandled())
        {
            ret = (*it)->GetTarget()->GetValue(m_Settings, *(*it));
        }
        CGXDLMSVariant& value = (*it)->GetValue();
        bb.SetUInt8((*it)->GetError());
        if ((*it)->IsByteArray() && value.vt == DLMS_DATA_TYPE_OCTET_STRING)
        {
            // If byte array is added do not add type.
            bb.Set(value.byteArr, value.GetSize());
        }
        else if ((ret = CGXDLMS::AppendData((*it)->GetTarget(), (*it)->GetIndex(), bb, value)) != 0)
        {
            return DLMS_ERROR_CODE_HARDWARE_FAULT;
        }
        if (m_Settings.GetIndex() != m_Settings.GetCount())
        {
            if (m_Transaction != NULL)
            {
                delete m_Transaction;
                m_Transaction = NULL;
            }
            CGXByteBuffer empty;
            m_Transaction = new CGXDLMSLongTransaction(list, DLMS_COMMAND_GET_REQUEST, empty);
        }
        ++pos;
    }
    PostRead(list);
    CGXDLMSLNParameters p(&m_Settings, invokeID, DLMS_COMMAND_GET_RESPONSE, 3, NULL, &bb, 0xFF);
    return CGXDLMS::GetLNPdu(p, 1, m_ReplyData);
}

int CGXDLMSServer::HandleGetRequest(
    CGXByteBuffer& data)
{
    // Return error if connection is not established.
    if (!m_Settings.IsConnected())
    {
        GenerateConfirmedServiceError(DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR,
            DLMS_SERVICE_ERROR_SERVICE, DLMS_SERVICE_UNSUPPORTED,
            m_ReplyData);
        return 0;
    }
    int ret;
    unsigned char ch, invokeID;
    // Get type.
    if ((ret = data.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    DLMS_GET_COMMAND_TYPE type = (DLMS_GET_COMMAND_TYPE)ch;
    // Get invoke ID and priority.
    if ((ret = data.GetUInt8(&invokeID)) != 0)
    {
        return ret;
    }
    // GetRequest normal
    if (type == DLMS_GET_COMMAND_TYPE_NORMAL)
    {
        ret = GetRequestNormal(data, invokeID);
    }
    else if (type == DLMS_GET_COMMAND_TYPE_NEXT_DATA_BLOCK)
    {
        // Get request for next data block
        ret = GetRequestNextDataBlock(data, invokeID,false);
    }
    else if (type == DLMS_GET_COMMAND_TYPE_WITH_LIST)
    {
        // Get request with a list.
        ret = GetRequestWithList(data, invokeID);
    }
    else
    {
        CGXByteBuffer bb;
        m_Settings.ResetBlockIndex();
        // Access Error : Device reports a hardware fault.
        bb.SetUInt8(DLMS_ERROR_CODE_HARDWARE_FAULT);
        CGXDLMSLNParameters p(&m_Settings, invokeID, DLMS_COMMAND_GET_RESPONSE,
            type, NULL, &bb, DLMS_ERROR_CODE_OK);
        ret = CGXDLMS::GetLNPdu(p, 1, m_ReplyData);
    }
    return ret;
}

/**
 * Find Short Name object.
 *
 * @param sn
 */
int CGXDLMSServer::FindSNObject(int sn, CGXSNInfo& i)
{
    unsigned char offset, count;
    for (CGXDLMSObjectCollection::iterator it = GetItems().begin(); it != GetItems().end(); ++it)
    {
        if (sn >= (*it)->GetShortName())
        {
            // If attribute is accessed.
            if (sn < (*it)->GetShortName() + (*it)->GetAttributeCount() * 8)
            {
                i.SetAction(false);
                i.SetItem(*it);
                i.SetIndex(((sn - (*it)->GetShortName()) / 8) + 1);
                break;
            }
            else
            {
                // If method is accessed.
                CGXDLMS::GetActionInfo((*it)->GetObjectType(), offset, count);
                if (sn < (*it)->GetShortName() + offset + (8 * count))
                {
                    i.SetItem(*it);
                    i.SetAction(true);
                    i.SetIndex((sn - (*it)->GetShortName() - offset) / 8 + 1);
                    break;
                }
            }
        }
    }
    if (i.GetItem() == NULL)
    {
        std::string ln;
        i.SetItem(FindObject(DLMS_OBJECT_TYPE_NONE, sn, ln));
    }
    return 0;
}

/**
* Get data for Read command.
*
* @param settings
*            DLMS settings.
* @param list
*            received objects.
* @param data
*            Data as byte array.
* @param type
*            Response type.
*/
int GetReadData(CGXDLMSSettings& settings,
    std::vector<CGXDLMSValueEventArg*>& list,
    CGXByteBuffer& data,
    DLMS_SINGLE_READ_RESPONSE& type)
{
    int ret = 0;
    unsigned char first = 1;
    type = DLMS_SINGLE_READ_RESPONSE_DATA;
    for (std::vector<CGXDLMSValueEventArg*>::iterator e = list.begin(); e != list.end(); ++e)
    {
        if (!(*e)->GetHandled())
        {
            // If action.
            if ((*e)->IsAction())
            {
                ret = (*e)->GetTarget()->Invoke(settings, *(*e));
            }
            else
            {
                ret = (*e)->GetTarget()->GetValue(settings, *(*e));
            }
        }
        if (ret != 0)
        {
            return ret;
        }
        CGXDLMSVariant& value = (*e)->GetValue();
        if ((*e)->GetError() == DLMS_ERROR_CODE_OK)
        {
            if (!first && list.size() != 1)
            {
                data.SetUInt8(DLMS_SINGLE_READ_RESPONSE_DATA);
            }
            // If action.
            if ((*e)->IsAction())
            {
                ret = GXHelpers::SetData(data, value.vt, value);
            }
            else
            {
                ret = CGXDLMS::AppendData((*e)->GetTarget(), (*e)->GetIndex(), data, value);
            }
        }
        else
        {
            if (!first && list.size() != 1)
            {
                data.SetUInt8(DLMS_SINGLE_READ_RESPONSE_DATA_ACCESS_ERROR);
            }
            data.SetUInt8((*e)->GetError());
            type = DLMS_SINGLE_READ_RESPONSE_DATA_ACCESS_ERROR;
        }
        first = false;
    }
    return ret;
}

int CGXDLMSServer::HandleRead(
    DLMS_VARIABLE_ACCESS_SPECIFICATION type,
    CGXByteBuffer& data,
    CGXDLMSValueEventCollection& list,
    std::vector<CGXDLMSValueEventArg*>& reads,
    std::vector<CGXDLMSValueEventArg*>& actions)
{
    CGXSNInfo i;
    int ret;
    unsigned char ch;
    unsigned short sn;
    if ((ret = data.GetUInt16(&sn)) != 0)
    {
        return ret;
    }
    if ((ret = FindSNObject(sn, i)) != 0)
    {
        return ret;
    }
    CGXDLMSValueEventArg* e = new CGXDLMSValueEventArg(this, i.GetItem(), i.GetIndex());
    e->SetAction(i.IsAction());
    list.push_back(e);
    if (type == DLMS_VARIABLE_ACCESS_SPECIFICATION_PARAMETERISED_ACCESS)
    {
        CGXDLMSVariant params;
        CGXDataInfo di;
        if ((ret = data.GetUInt8(&ch)) != 0)
        {
            return ret;
        }
        e->SetSelector(ch);
        if ((ret = GXHelpers::GetData(data, di, params)) != 0)
        {
            return ret;
        }
        e->SetParameters(params);
    }
    // Return error if connection is not established.
    if (!m_Settings.IsConnected()
        && (!e->IsAction() || e->GetTarget()->GetShortName() != 0xFA00 || e->GetIndex() != 8))
    {
        GenerateConfirmedServiceError(
            DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR, DLMS_SERVICE_ERROR_SERVICE,
            DLMS_SERVICE_UNSUPPORTED, m_ReplyData);
        return 0;
    }

    if (GetAttributeAccess(e) == DLMS_ACCESS_MODE_NONE)
    {
        e->SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
    }
    else
    {
        if (e->GetTarget()->GetObjectType() == DLMS_OBJECT_TYPE_PROFILE_GENERIC && e->GetIndex() == 2)
        {
            e->SetRowToPdu(GetRowsToPdu((CGXDLMSProfileGeneric*)e->GetTarget()));
        }
        if (e->IsAction())
        {
            actions.push_back(e);
        }
        else
        {
            reads.push_back(e);
        }
    }
    return ret;
}

int CGXDLMSServer::HandleReadBlockNumberAccess(
    CGXByteBuffer& data)
{
    unsigned short blockNumber;
    int ret;
    if ((ret = data.GetUInt16(&blockNumber)) != 0)
    {
        return ret;
    }
    if (blockNumber != m_Settings.GetBlockIndex())
    {
        CGXByteBuffer bb;
        bb.SetUInt8(DLMS_ERROR_CODE_DATA_BLOCK_NUMBER_INVALID);
        CGXDLMSSNParameters p(&m_Settings,
            DLMS_COMMAND_READ_RESPONSE, 1,
            DLMS_SINGLE_READ_RESPONSE_DATA_ACCESS_ERROR, &bb, NULL);
        ret = CGXDLMS::GetSNPdu(p, m_ReplyData);
        m_Settings.ResetBlockIndex();
        return ret;
    }
    if (m_Settings.GetIndex() != m_Settings.GetCount()
        && m_Transaction->GetData().GetSize() < m_Settings.GetMaxPduSize())
    {
        std::vector<CGXDLMSValueEventArg*> reads;
        std::vector<CGXDLMSValueEventArg*> actions;

        for (std::vector<CGXDLMSValueEventArg*>::iterator it = m_Transaction->GetTargets().begin();
            it != m_Transaction->GetTargets().end(); ++it)
        {
            if ((*it)->IsAction())
            {
                actions.push_back(*it);
            }
            else
            {
                reads.push_back(*it);
            }
        }
        if (reads.size() != 0)
        {
            PreRead(reads);
        }

        if (actions.size() != 0)
        {
            PreAction(actions);
        }
        DLMS_SINGLE_READ_RESPONSE requestType;
        std::vector<CGXDLMSValueEventArg*>& list = m_Transaction->GetTargets();
        CGXByteBuffer& data2 = m_Transaction->GetData();
        ret = GetReadData(m_Settings, list, data2, requestType);
        if (reads.size() != 0)
        {
            PostRead(reads);
        }
        if (actions.size() != 0)
        {
            PostAction(actions);
        }
    }
    m_Settings.IncreaseBlockIndex();
    CGXByteBuffer& tmp = m_Transaction->GetData();
    CGXDLMSSNParameters p(&m_Settings, DLMS_COMMAND_READ_RESPONSE, 1,
        DLMS_SINGLE_READ_RESPONSE_DATA_BLOCK_RESULT, NULL, &tmp);
    p.SetMultipleBlocks(true);
    ret = CGXDLMS::GetSNPdu(p, m_ReplyData);
    // If all data is sent.
    if (m_Transaction->GetData().GetSize() == m_Transaction->GetData().GetPosition())
    {
        delete m_Transaction;
        m_Transaction = NULL;
        m_Settings.ResetBlockIndex();
    }
    else
    {
        m_Transaction->GetData().Trim();
    }
    return ret;
}

int CGXDLMSServer::HandleReadDataBlockAccess(
    DLMS_COMMAND command,
    CGXByteBuffer& data,
    int cnt)
{
    int ret;
    unsigned long size;
    unsigned short blockNumber;
    unsigned char isLast, ch;
    if ((ret = data.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    isLast = ch != 0;
    if ((ret = data.GetUInt16(&blockNumber)) != 0)
    {
        return ret;
    }
    if (blockNumber != m_Settings.GetBlockIndex())
    {
        CGXByteBuffer bb;
        bb.SetUInt8(DLMS_ERROR_CODE_DATA_BLOCK_NUMBER_INVALID);
        CGXDLMSSNParameters p(&m_Settings, command, 1, DLMS_SINGLE_READ_RESPONSE_DATA_ACCESS_ERROR, &bb, NULL);
        ret = CGXDLMS::GetSNPdu(p, m_ReplyData);
        m_Settings.ResetBlockIndex();
        return ret;
    }
    unsigned char count = 1, type = DLMS_DATA_TYPE_OCTET_STRING;
    if (command == DLMS_COMMAND_WRITE_RESPONSE)
    {
        if ((ret = data.GetUInt8(&count)) != 0 ||
            (ret = data.GetUInt8(&type)) != 0)
        {
            return ret;
        }
    }
    if ((ret = GXHelpers::GetObjectCount(data, size)) != 0)
    {
        return ret;
    }
    unsigned long realSize = data.GetSize() - data.GetPosition();
    if (count != 1 || type != DLMS_DATA_TYPE_OCTET_STRING || size != realSize)
    {
        CGXByteBuffer bb;
        bb.SetUInt8(DLMS_ERROR_CODE_BLOCK_UNAVAILABLE);
        CGXDLMSSNParameters p(&m_Settings, command, cnt,
            DLMS_SINGLE_READ_RESPONSE_DATA_ACCESS_ERROR, &bb, NULL);
        ret = CGXDLMS::GetSNPdu(p, m_ReplyData);
        m_Settings.ResetBlockIndex();
        return ret;
    }
    if (m_Transaction == NULL)
    {
        CGXDLMSValueEventCollection tmp;
        m_Transaction = new CGXDLMSLongTransaction(tmp, command, data);
    }
    else
    {
        m_Transaction->GetData().Set(&data, data.GetPosition());
    }
    if (!isLast)
    {
        CGXByteBuffer bb;
        bb.SetUInt16(blockNumber);
        m_Settings.IncreaseBlockIndex();
        if (command == DLMS_COMMAND_READ_RESPONSE)
        {
            type = DLMS_SINGLE_READ_RESPONSE_BLOCK_NUMBER;
        }
        else
        {
            type = DLMS_SINGLE_WRITE_RESPONSE_BLOCK_NUMBER;
        }
        CGXDLMSSNParameters p(&m_Settings, command, cnt, type, NULL, &bb);
        ret = CGXDLMS::GetSNPdu(p, m_ReplyData);
    }
    else
    {
        if (m_Transaction != NULL)
        {
            data.SetSize(0);
            data.Set(&m_Transaction->GetData());
            delete m_Transaction;
            m_Transaction = NULL;
        }
        if (command == DLMS_COMMAND_READ_RESPONSE)
        {
            ret = HandleReadRequest(data);
        }
        else
        {
            ret = HandleWriteRequest(data);
        }
        m_Settings.ResetBlockIndex();
    }
    return ret;
}

int CGXDLMSServer::ReturnSNError(DLMS_COMMAND cmd, DLMS_ERROR_CODE error)
{
    int ret;
    CGXByteBuffer bb;
    bb.SetUInt8(error);
    CGXDLMSSNParameters p(&m_Settings, cmd, 1,
        DLMS_SINGLE_READ_RESPONSE_DATA_ACCESS_ERROR, &bb, NULL);
    ret = CGXDLMS::GetSNPdu(p, m_ReplyData);
    m_Settings.ResetBlockIndex();
    return ret;
}

int CGXDLMSServer::HandleReadRequest(CGXByteBuffer& data)
{
    CGXByteBuffer bb;
    int ret;
    unsigned char ch;
    unsigned long cnt = 0xFF;
    DLMS_VARIABLE_ACCESS_SPECIFICATION type;
    CGXDLMSValueEventCollection list;
    std::vector<CGXDLMSValueEventArg*> reads;
    std::vector<CGXDLMSValueEventArg*> actions;
    // If get next frame.
    if (data.GetSize() == 0)
    {
        if (m_Transaction != NULL)
        {
            return 0;
        }
        bb.Set(&m_ReplyData);
        m_ReplyData.Clear();
        for (std::vector<CGXDLMSValueEventArg*>::iterator it = m_Transaction->GetTargets().begin();
            it != m_Transaction->GetTargets().end(); ++it)
        {
            list.push_back(*it);
        }
    }
    else
    {
        if ((ret = GXHelpers::GetObjectCount(data, cnt)) != 0)
        {
            return ret;
        }
        CGXSNInfo info;
        for (unsigned long pos = 0; pos != cnt; ++pos)
        {
            if ((ret = data.GetUInt8(&ch)) != 0)
            {
                return ret;
            }
            type = (DLMS_VARIABLE_ACCESS_SPECIFICATION)ch;
            switch (type)
            {
            case DLMS_VARIABLE_ACCESS_SPECIFICATION_VARIABLE_NAME:
            case DLMS_VARIABLE_ACCESS_SPECIFICATION_PARAMETERISED_ACCESS:
                ret = HandleRead(type, data, list, reads, actions);
                break;
            case DLMS_VARIABLE_ACCESS_SPECIFICATION_BLOCK_NUMBER_ACCESS:
                return HandleReadBlockNumberAccess(data);
            case DLMS_VARIABLE_ACCESS_SPECIFICATION_READ_DATA_BLOCK_ACCESS:
                return HandleReadDataBlockAccess(DLMS_COMMAND_READ_RESPONSE, data, cnt);
            default:
                return ReturnSNError(DLMS_COMMAND_READ_RESPONSE, DLMS_ERROR_CODE_READ_WRITE_DENIED);
            }
        }
        if (reads.size() != 0)
        {
            PreRead(reads);
        }
        if (actions.size() != 0)
        {
            PreAction(actions);
        }
    }
    DLMS_SINGLE_READ_RESPONSE requestType;
    ret = GetReadData(m_Settings, list, bb, requestType);
    if (reads.size() != 0)
    {
        PostRead(reads);
    }
    if (actions.size() != 0)
    {
        PostAction(actions);
    }
    CGXDLMSSNParameters p(&m_Settings, DLMS_COMMAND_READ_RESPONSE, cnt,
        requestType, NULL, &bb);
    CGXDLMS::GetSNPdu(p, m_ReplyData);
    if (m_Transaction == NULL && (bb.GetSize() != bb.GetPosition()
        || m_Settings.GetCount() != m_Settings.GetIndex()))
    {
        m_Transaction = new CGXDLMSLongTransaction(list, DLMS_COMMAND_READ_REQUEST, bb);
    }
    else if (m_Transaction != NULL)
    {
        m_ReplyData.Set(&bb);
    }
    return 0;
}

int CGXDLMSServer::HandleWriteRequest(CGXByteBuffer& data)
{
    // Return error if connection is not established.
    if (!m_Settings.IsConnected())
    {
        GenerateConfirmedServiceError(DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR,
            DLMS_SERVICE_ERROR_SERVICE,
            DLMS_SERVICE_UNSUPPORTED, m_ReplyData);
        return 0;
    }
    int ret;
    unsigned char ch;
    unsigned short sn;
    unsigned long cnt, pos;
    DLMS_VARIABLE_ACCESS_SPECIFICATION type;
    CGXDLMSVariant value;
    std::vector<CGXSNInfo> targets;
    // Get object count.
    if ((ret = GXHelpers::GetObjectCount(data, cnt)) != 0)
    {
        return ret;
    }
    CGXByteBuffer results(cnt);
    for (pos = 0; pos != cnt; ++pos)
    {
        if ((ret = data.GetUInt8(&ch)) != 0)
        {
            return ret;
        }
        type = (DLMS_VARIABLE_ACCESS_SPECIFICATION)ch;
        CGXSNInfo i;
        switch (type)
        {
        case DLMS_VARIABLE_ACCESS_SPECIFICATION_VARIABLE_NAME:
            if ((ret = data.GetUInt16(&sn)) != 0)
            {
                return ret;
            }
            if ((ret = FindSNObject(sn, i)) != 0)
            {
                return ret;
            }
            targets.push_back(i);
            // If target is unknown.
            if (i.GetItem() == NULL)
            {
                // Device reports a undefined object.
                results.SetUInt8(DLMS_ERROR_CODE_UNDEFINED_OBJECT);
            }
            else
            {
                results.SetUInt8(DLMS_ERROR_CODE_OK);
            }
            break;
        case DLMS_VARIABLE_ACCESS_SPECIFICATION_WRITE_DATA_BLOCK_ACCESS:
            return HandleReadDataBlockAccess(DLMS_COMMAND_WRITE_RESPONSE, data, cnt);
        default:
            // Device reports a HW error.
            results.SetUInt8(DLMS_ERROR_CODE_HARDWARE_FAULT);
        }
    }
    // Get data count.
    if ((ret = GXHelpers::GetObjectCount(data, cnt)) != 0)
    {
        return ret;
    }
    CGXDataInfo di;
    for (pos = 0; pos != cnt; ++pos)
    {
        if ((ret = results.GetUInt8(pos, &ch)) != 0)
        {
            return ret;
        }
        if (ch == 0)
        {
            // If object has found.
            CGXSNInfo target = *(targets.begin() + pos);
            ret = GXHelpers::GetData(data, di, value);
            if (value.vt == DLMS_DATA_TYPE_OCTET_STRING)
            {
                DLMS_DATA_TYPE dt;
                ret = target.GetItem()->GetDataType(target.GetIndex(), dt);
                if (dt != DLMS_DATA_TYPE_NONE && dt != DLMS_DATA_TYPE_OCTET_STRING)
                {
                    CGXByteBuffer bb;
                    bb.Set(value.byteArr, value.GetSize());
                    value.Clear();
                    if ((ret = CGXDLMSClient::ChangeType(bb, dt, value)) != 0)
                    {
                        return ret;
                    }
                }
            }
            di.Clear();
            CGXDLMSValueEventArg* e = new CGXDLMSValueEventArg(this, target.GetItem(), target.GetIndex());
            e->SetValue(value);
            CGXDLMSValueEventCollection arr;
            arr.push_back(e);
            PreWrite(arr);
            DLMS_ACCESS_MODE am = GetAttributeAccess(e);
            // If write is denied.
            if (am != DLMS_ACCESS_MODE_WRITE && am != DLMS_ACCESS_MODE_READ_WRITE)
            {
                results.SetUInt8(pos, DLMS_ERROR_CODE_READ_WRITE_DENIED);
            }
            else
            {
                if (e->GetError() != 0)
                {
                    results.SetUInt8(pos, e->GetError());
                }
                else if (!e->GetHandled())
                {
                    target.GetItem()->SetValue(m_Settings, *e);
                    PostWrite(arr);
                }
            }
        }
    }
    CGXByteBuffer bb((2 * cnt));
    for (pos = 0; pos != cnt; ++pos)
    {
        if ((ret = results.GetUInt8(pos, &ch)) != 0)
        {
            return ret;
        }
        // If meter returns error.
        if (ch != 0)
        {
            bb.SetUInt8(1);
        }
        bb.SetUInt8(ch);
    }
    CGXDLMSSNParameters p(&m_Settings, DLMS_COMMAND_WRITE_RESPONSE, cnt, 0xFF, NULL, &bb);
    return CGXDLMS::GetSNPdu(p, m_ReplyData);
}

/**
* Handles release request.
*
* @param data
*            Received data.
* @param connectionInfo
*            Connection info.
*/
int CGXDLMSServer::HandleReleaseRequest(CGXByteBuffer& data)
{
    if (m_Settings.GetInterfaceType() == DLMS_INTERFACE_TYPE_HDLC)
    {
        m_ReplyData.Set(LLC_REPLY_BYTES, 3);
    }
    m_ReplyData.SetUInt8(0x63);
    // LEN.
    m_ReplyData.SetUInt8(0x03);
    m_ReplyData.SetUInt8(0x80);
    m_ReplyData.SetUInt8(0x01);
    m_ReplyData.SetUInt8(0x00);
    return 0;
}

int CGXDLMSServer::HandleCommand(
    DLMS_COMMAND cmd,
    CGXByteBuffer& data,
	CGXByteBuffer& reply,
	CGXServerReply& sr)
{
    int ret = 0;
    unsigned char frame = 0, *tmpPtr, ch = 0;
    bool bHandlingGbtFirsttime = false;
	CGXByteBuffer intermediateReply,savedReplyDaya;
	unsigned short intermediateLen;

    switch (cmd)
    {
    case DLMS_COMMAND_SET_REQUEST:
        ret = HandleSetRequest(data);
        break;
    case DLMS_COMMAND_WRITE_REQUEST:
        ret = HandleWriteRequest(data);
        break;
    case DLMS_COMMAND_GET_REQUEST:
        if (data.GetSize() != 0)
        {
            ret = HandleGetRequest(data);
        }
        break;
    case DLMS_COMMAND_READ_REQUEST:
        ret = HandleReadRequest(data);
        break;
    case DLMS_COMMAND_METHOD_REQUEST:
        ret = HandleMethodRequest(data, sr.GetConnectionInfo());
        break;
    case DLMS_COMMAND_SNRM:
        ret = HandleSnrmRequest(m_Settings, data, m_ReplyData);
        frame = DLMS_COMMAND_UA;
        break;
    case DLMS_COMMAND_AARQ:
        ret = HandleAarqRequest(data, sr.GetConnectionInfo());
        break;
    case DLMS_COMMAND_RELEASE_REQUEST:
        ret = HandleReleaseRequest(data);
        break;
    case DLMS_COMMAND_DISC:
        ret = GenerateDisconnectRequest(m_Settings, m_ReplyData);
        m_Settings.SetConnected(false);
        Disconnected(sr.GetConnectionInfo());
        frame = DLMS_COMMAND_UA;
        break;
    case DLMS_COMMAND_GENERAL_BLOCK_TRANSFER:
    	ret = HandleGeneralBlockTransfer(data, sr);
    	if(sr.GetReplyingToGbt() == false)
    	{
			savedReplyDaya = m_ReplyData;
			m_ReplyData.Clear();
			ret = HandleCommand(m_Transaction->GetCommand(),m_Transaction->GetData(),intermediateReply, sr);


			//get the lenght of teh intermediate reply
			intermediateReply.GetUInt16(DLMS_DATA_LENGTH_OFFSET,&intermediateLen);
			//check if the intermediate reply was in GBT
			intermediateReply.GetUInt8(DLMS_WRAPPER_FRAME_LENGTH,&ch);

			tmpPtr = intermediateReply.GetData();



			//the reply data is more than max PDU
			//copy the whole message including the GBT header we got from the reply
			if(ch == DLMS_COMMAND_GENERAL_BLOCK_TRANSFER)
			{
				//since this is the first time we are handling the GBT then need to set the block acked to 1
				if((NULL != tmpPtr) && (intermediateReply.GetSize() >= DLMS_WRAPPER_FRAME_LENGTH +5))
				{
					tmpPtr[DLMS_WRAPPER_FRAME_LENGTH + 5] = 1;
				}

				m_Settings.SetBlockNumberAck(1);
				//we decrease from teh count the current block we got
				sr.SetCount(sr.GetClientWindowSize()-1);
				reply.Set(&intermediateReply,0,intermediateReply.GetSize());
				sr.SetReplyingToGbt(true);
				//if transaction is NULL ift means that all data was received and this is teh last balock
				sr.SetIsLastBlock(m_Transaction == NULL);
				//we got the complete reply including teh wrapper frame
				//nothing else to do so return
				return ret;
			}
			else
			{
				m_ReplyData = savedReplyDaya;
				//get the length of the intermediate reply
				intermediateReply.GetUInt16(DLMS_DATA_LENGTH_OFFSET,&intermediateLen);

				GXHelpers::SetObjectCount(intermediateLen,m_ReplyData);
				m_ReplyData.Set(&intermediateReply,DLMS_WRAPPER_FRAME_LENGTH,intermediateReply.GetSize()-DLMS_WRAPPER_FRAME_LENGTH);

				printf("##406 %d %d %d %x\n",ret,intermediateReply.GetSize(),intermediateReply.GetPosition(),*(intermediateReply.GetData()));
				sr.SetReplyingToGbt(true);
			}
    	}
        break;
    case DLMS_COMMAND_NONE:
        //Get next frame.
        break;
    default:
        //Invalid command.
        break;
    }
    if (ret == 0)
    {
        if (m_Settings.GetInterfaceType() == DLMS_INTERFACE_TYPE_WRAPPER)
        {
        	ret = CGXDLMS::GetWrapperFrame(m_Settings, m_ReplyData, reply);
        }
        else
        {
            ret = CGXDLMS::GetHdlcFrame(m_Settings, frame, &m_ReplyData, reply);
        }
    }
    if (cmd == DLMS_COMMAND_DISC ||
        (m_Settings.GetInterfaceType() == DLMS_INTERFACE_TYPE_WRAPPER && cmd == DLMS_COMMAND_RELEASE_REQUEST))
    {
        Reset();
    }
    return ret;
}


int CGXDLMSServer::HandleGeneralBlockTransfer(CGXByteBuffer& data,	CGXServerReply& sr)
{
	int ret = 0;

	if (m_Transaction != NULL) {
		if ((m_Transaction->GetCommand() == DLMS_COMMAND_GET_REQUEST)||
			(m_Transaction->GetCommand() == DLMS_COMMAND_GENERAL_BLOCK_TRANSFER)) {

			//parse the ack received from teh client and only then continue with teh execution
			if(m_Transaction->GetCommand() == DLMS_COMMAND_GENERAL_BLOCK_TRANSFER) {
				// BlockControl
				unsigned char bc;
			    if ((ret = data.GetUInt8(&bc)) != 0)
			    {
			        return ret;
			    }

				// Block number.
				unsigned short blockNumber;
			    if ((ret = data.GetUInt16(&blockNumber)) != 0)
			    {
			        return ret;
			    }
				// Block number acknowledged.
				unsigned short blockNumberAck;
			    if ((ret = data.GetUInt16(&blockNumberAck)) != 0)
			    {
			        return ret;
			    }


				sr.SetClientBlockNumAcked(blockNumberAck);
				sr.SetClientBlockNum(blockNumber);
				sr.SetClientWindowSize(bc & 0x3f);
		    	m_Settings.SetClientWindowSize(sr.GetClientWindowSize());
		    	sr.SetCount(sr.GetClientWindowSize());
		    	m_Settings.SetBlockNumberAck(blockNumber);
		    	m_Settings.SetClientBlockNumberAck(blockNumberAck);
		    	sr.SetIsLastBlock(false);
		    	m_Transaction->SetCommand(DLMS_COMMAND_GET_REQUEST);

			}

			// Get request for next data block
			if (sr.GetCount() == 0) {
				m_Settings.SetBlockNumberAck(
						m_Settings.GetBlockNumberAck() + 1);
				sr.SetCount(m_Settings.GetWindowSize());
			}
			GetRequestNextDataBlock(data, 0, true);
			if (sr.GetCount() != 0) {
				sr.SetCount(sr.GetCount() - 1);
			}
			if (m_Transaction == NULL) {
				sr.SetCount(0);
				sr.SetIsLastBlock(true);
			}
			else if(sr.GetCount() == 0)
			{
				//in case this is the last block in the window we need to mark
				//in the transaction that we will need to parse client reply after the
				//next confirmation
				m_Transaction->SetCommand(DLMS_COMMAND_GENERAL_BLOCK_TRANSFER);
			}
		} else {
			// BlockControl
			unsigned char bc;
		    if ((ret = data.GetUInt8(&bc)) != 0)
		    {
		        return ret;
		    }

			sr.SetClientWindowSize(bc & 0x3f);
	    	m_Settings.SetClientWindowSize(sr.GetClientWindowSize());

			// Block number.
			unsigned short blockNumber;
		    if ((ret = data.GetUInt16(&blockNumber)) != 0)
		    {
		        return ret;
		    }

			// Block number acknowledged.
			unsigned short blockNumberAck;
		    if ((ret = data.GetUInt16(&blockNumberAck)) != 0)
		    {
		        return ret;
		    }

			unsigned long len;
			GXHelpers::GetObjectCount(data,len);
			if (len > data.GetSize() - data.GetPosition())
			{
				GenerateConfirmedServiceError(
						DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR,
						DLMS_SERVICE_ERROR_SERVICE,
						DLMS_SERVICE_UNSUPPORTED,m_ReplyData);
			}
			else
			{
				m_Transaction->GetData().Set(data.GetData(),data.GetSize());
				// Send ACK.
				bool igonoreAck = (bc & 0x40) != 0
						&& (blockNumberAck * m_Settings.GetWindowSize())
								+ 1 > blockNumber;
				int windowSize = m_Settings.GetWindowSize();
				int bn = m_Settings.GetBlockIndex();
				if ((bc & 0x80) != 0) {
					HandleCommand(m_Transaction->GetCommand(),
							m_Transaction->GetData(),sr.GetReply(), sr);
					m_Transaction = NULL;
					igonoreAck = false;
					windowSize = 1;
				}
				if (igonoreAck) {
					return false;
				}
				m_ReplyData.SetUInt8(DLMS_COMMAND_GENERAL_BLOCK_TRANSFER);
				m_ReplyData.SetUInt8((unsigned char) (0x80 | windowSize));
				m_Settings.SetBlockIndex(m_Settings.GetBlockIndex() + 1);
				m_ReplyData.SetUInt16(bn);
				m_ReplyData.SetUInt16(blockNumber);
				m_ReplyData.SetUInt8(0);
			}
		}
	} else {
		// BlockControl
		unsigned char bc;
	    if ((ret = data.GetUInt8(&bc)) != 0)
	    {
	        return ret;
	    }

		// Block number.
		unsigned short blockNumber;
	    if ((ret = data.GetUInt16(&blockNumber)) != 0)
	    {
	        return ret;
	    }

		// Block number acknowledged.
		unsigned short blockNumberAck;
	    if ((ret = data.GetUInt16(&blockNumberAck)) != 0)
	    {
	        return ret;
	    }

		unsigned long len;
		GXHelpers::GetObjectCount(data,len);
		if (len > data.GetSize() - data.GetPosition())
		{
			GenerateConfirmedServiceError(
					DLMS_CONFIRMED_SERVICE_ERROR_INITIATE_ERROR,
					DLMS_SERVICE_ERROR_SERVICE,
					DLMS_SERVICE_UNSUPPORTED,m_ReplyData);
		}
		else
		{
			unsigned char ch;

			sr.SetClientBlockNumAcked(blockNumberAck);
			sr.SetClientBlockNum(blockNumber);
			sr.SetClientWindowSize(bc & 0x3f);
	    	m_Settings.SetClientWindowSize(sr.GetClientWindowSize());

			if ((ret = data.GetUInt8(&ch)) != 0)
		    {
		        return ret;
		    }
		    CGXDLMSValueEventCollection tmp;
			m_Transaction =
					new CGXDLMSLongTransaction(tmp, (DLMS_COMMAND)ch, data);
			m_ReplyData.SetUInt8(DLMS_COMMAND_GENERAL_BLOCK_TRANSFER);
			m_ReplyData.SetUInt8((0x80 | m_Settings.GetWindowSize()));
			m_ReplyData.SetUInt16(blockNumber);
			++blockNumberAck;
			m_ReplyData.SetUInt16(blockNumberAck);
			sr.SetIsLastBlock(false);
			//TODO:need to check if this byte needed
			//m_ReplyData.SetUInt8(0);


		}
	}
	return ret;
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
#define MAX_PACKET_PRINT_1 170
static void PrintfBuff(CGXByteBuffer *bb)
{
	PrintfBuff(bb->GetData(), bb->GetSize() > MAX_PACKET_PRINT_1 ? MAX_PACKET_PRINT_1 : bb->GetSize());
}

/**
 * Handle action request.
 *
 * @param reply
 *            Received data from the client.
 * @return Reply.
 */
int CGXDLMSServer::HandleMethodRequest(
    CGXByteBuffer& data,
    CGXDLMSConnectionEventArgs& connectionInfo)
{
    CGXByteBuffer bb;
    DLMS_ERROR_CODE error = DLMS_ERROR_CODE_OK;
    CGXDLMSVariant parameters;
    int ret;
    unsigned char ch, id, invokeId;
    unsigned short tmp;
    // Get type.
    if ((ret = data.GetUInt8(&ch)) != 0)
    {
        return ret;
    }
    // Get invoke ID and priority.
    if ((ret = data.GetUInt8(&invokeId)) != 0)
    {
        return ret;
    }
    // CI
    if ((ret = data.GetUInt16(&tmp)) != 0)
    {
        return ret;
    }
    DLMS_OBJECT_TYPE ci = (DLMS_OBJECT_TYPE)tmp;
    unsigned char * ln;
    ln = data.GetData() + data.GetPosition();
    data.SetPosition(data.GetPosition() + 6);
    // Attribute
    if ((ret = data.GetUInt8(&id)) != 0)
    {
        return ret;
    }
    // Get parameters.
    if ((ret = data.GetUInt8(&ch)) != 0)
    {
        return ret;
    }

    if (ch != 0)
    {
        CGXDataInfo i;
        if ((ret = GXHelpers::GetData(data, i, parameters)) != 0)
        {
            return ret;
        }
    }
    CGXDLMSObject* obj = m_Settings.GetObjects().FindByLN(ci, ln);
    if (obj == NULL)
    {
        std::string name;
        GXHelpers::GetLogicalName(ln, name);
        obj = FindObject(ci, 0, name);
    }
    if (obj == NULL)
    {
        error = DLMS_ERROR_CODE_UNDEFINED_OBJECT;
    }
    else
    {
        CGXDLMSValueEventArg* e = new CGXDLMSValueEventArg(this, obj, id, 0, parameters);
        CGXDLMSValueEventCollection arr;
        arr.push_back(e);
        if (GetMethodAccess(e) == DLMS_METHOD_ACCESS_MODE_NONE)
        {
            error = DLMS_ERROR_CODE_READ_WRITE_DENIED;
        }
        else
        {
            PreAction(arr);
            if (!e->GetHandled())
            {
                if ((ret = obj->Invoke(m_Settings, *e)) != 0)
                {
                    return ret;
                }
                PostAction(arr);
            }
            CGXDLMSVariant& actionReply = e->GetValue();
            // Set default action reply if not given.
            if (actionReply.vt != DLMS_DATA_TYPE_NONE && e->GetError() == DLMS_ERROR_CODE_OK)
            {
                // Add return parameters
                bb.SetUInt8(1);
                //Add parameters error code.
                bb.SetUInt8(0);
                GXHelpers::SetData(bb, actionReply.vt, actionReply);
				printf("Server: Ready to send action response\n\n");
				PrintfBuff(&bb);
            }
            else
            {
                // Add parameters error code.
                error = e->GetError();
                bb.SetUInt8(0);
				printf("Server: Send error\n\n");
            }
        }
    }
    CGXDLMSLNParameters p(&m_Settings, invokeId, DLMS_COMMAND_METHOD_RESPONSE, 1, NULL, &bb, error);
    ret = CGXDLMS::GetLNPdu(p, 0, m_ReplyData);
    // If High level authentication fails.
    if (!m_Settings.IsConnected() && obj->GetObjectType() == DLMS_OBJECT_TYPE_ASSOCIATION_LOGICAL_NAME && id == 1)
    {
		printf("Server: InvalidConnection\n");
        InvalidConnection(connectionInfo);
    }
	printf("Server: ret=%d\n\n", ret);
    return ret;
}

#if 0
int CGXDLMSServer::HandleRequest(
    CGXByteBuffer& data,
    CGXByteBuffer& reply)
{
    return HandleRequest(
        data.GetData(),
        (unsigned short)(data.GetSize() - data.GetPosition()),
        reply);
}

#endif

int CGXDLMSServer::HandleRequest(
    CGXDLMSConnectionEventArgs& connectionInfo,
    CGXByteBuffer& data,
    CGXByteBuffer& reply)
{
	int ret;
    CGXServerReply sr(data);
    sr.SetConnectionInfo(connectionInfo);
    ret = HandleRequest(sr);
    reply = sr.GetReply();
    return ret;
}

#if 0
int CGXDLMSServer::HandleRequest(
    unsigned char* buff,
    unsigned short size,
    CGXByteBuffer& reply)
{
    CGXDLMSConnectionEventArgs connectionInfo;
    return HandleRequest(connectionInfo, buff, size, reply);
}
#endif


int CGXDLMSServer::HandleRequest(CGXServerReply &sr)
{
    int ret;

    if (!sr.IsStreaming()
             && (sr.GetData().GetData() == NULL || sr.GetData().GetSize() == 0)) {
         return 0;
     }

    if (!m_Initialized)
    {
        //Server not Initialized.
        return DLMS_ERROR_CODE_NOT_INITIALIZED;
    }

    if (!sr.IsStreaming()) {

    	m_ReceivedData.Set(sr.GetData().GetData(), sr.GetData().GetSize());
		bool first = m_Settings.GetServerAddress() == 0
			&& m_Settings.GetClientAddress() == 0;
		if ((ret = CGXDLMS::GetData(m_Settings, m_ReceivedData, m_Info)) != 0)
		{
			//If all data is not received yet.
			if (ret == DLMS_ERROR_CODE_FALSE)
			{
				ret = 0;
			}
			return ret;
		}
		// If all data is not received yet.
		if (!m_Info.IsComplete())
		{
			return 0;
		}
		m_ReceivedData.Clear();

		if (first || m_Info.GetCommand() == DLMS_COMMAND_SNRM ||
			(m_Settings.GetInterfaceType() == DLMS_INTERFACE_TYPE_WRAPPER && m_Info.GetCommand() == DLMS_COMMAND_AARQ))
		{
			// Check is data send to this server.
			if (!IsTarget(m_Settings.GetServerAddress(), m_Settings.GetClientAddress()))
			{
				m_Info.Clear();
				return 0;
			}
		}

		// If client want next frame.
		if ((m_Info.GetMoreData() & DLMS_DATA_REQUEST_TYPES_FRAME) == DLMS_DATA_REQUEST_TYPES_FRAME)
		{
			m_DataReceived = (long)time(NULL);
			return CGXDLMS::GetHdlcFrame(m_Settings, m_Settings.GetReceiverReady(), &m_ReplyData, sr.GetReply());
		}
		// Update command if m_Transaction and next frame is asked.
		if (m_Info.GetCommand() == DLMS_COMMAND_NONE)
		{
			if (m_Transaction != NULL)
			{
				m_Info.SetCommand(m_Transaction->GetCommand());
			}
		}
		// Check inactivity time out.
		if (m_Hdlc != NULL && m_Hdlc->GetInactivityTimeout() != 0)
		{
			if (m_Info.GetCommand() != DLMS_COMMAND_SNRM)
			{
				long elapsed = (long)(time(NULL) - m_DataReceived);
				// If inactivity time out is elapsed.
				if (elapsed >= m_Hdlc->GetInactivityTimeout())
				{
					Reset();
					m_DataReceived = 0;
					return 0;
				}
			}
		}
		else if (m_Wrapper != NULL && m_Wrapper->GetInactivityTimeout() != 0)
		{
			if (m_Info.GetCommand() != DLMS_COMMAND_AARQ)
			{
				long elapsed = (long)(time(NULL) - m_DataReceived);
				// If inactivity time out is elapsed.
				if (elapsed >= m_Wrapper->GetInactivityTimeout())
				{
					Reset();
					m_DataReceived = 0;
					return 0;
				}
			}
		}
    }
    else
    {
    	m_Info.SetCommand(DLMS_COMMAND_GENERAL_BLOCK_TRANSFER);
    }
    ret = HandleCommand( m_Info.GetCommand(),
    					m_Info.GetData(), sr.GetReply(),sr);
    m_Info.Clear();
    m_DataReceived = (long)time(NULL);
    return ret;
}

DLMS_CONFORMANCE CGXDLMSServer::GetConformance()
{
    return m_Settings.GetProposedConformance();
}

void CGXDLMSServer::SetConformance(DLMS_CONFORMANCE value)
{
    m_Settings.SetProposedConformance(value);
}

int CGXDLMSServer::AddData(
    CGXDLMSObject* obj,
    unsigned char index,
    CGXByteBuffer& buff)
{
    int ret;
    DLMS_DATA_TYPE dt;
    CGXDLMSValueEventArg e(NULL, obj, index);
    if ((ret = obj->GetValue(m_Settings, e)) != 0)
    {
        return ret;
    }
    if ((ret = obj->GetDataType(index, dt)) != 0)
    {
        return ret;
    }
    if (dt == DLMS_DATA_TYPE_ARRAY)
    {
        buff.Set(e.GetValue().byteArr, e.GetValue().GetSize());
        return 0;
    }
    if (dt == DLMS_DATA_TYPE_NONE)
    {
        dt = e.GetValue().vt;
    }
    return GXHelpers::SetData(buff, e.GetValue().vt, e.GetValue());
}

int CGXDLMSServer::GenerateDataNotificationMessages(
    struct tm* time,
    CGXByteBuffer& data,
    std::vector<CGXByteBuffer>& reply)
{
    int ret;
    if (GetUseLogicalNameReferencing())
    {
        CGXDLMSLNParameters p(&m_Settings, 0, DLMS_COMMAND_DATA_NOTIFICATION, 0, NULL, &data, 0xff);
        p.SetTime(time);
        ret = CGXDLMS::GetLnMessages(p, reply);
    }
    else
    {
        CGXDLMSSNParameters p(&m_Settings, DLMS_COMMAND_DATA_NOTIFICATION, 1, 0, &data, NULL);
        ret = CGXDLMS::GetSnMessages(p, reply);
    }
    return ret;
}

int CGXDLMSServer::GenerateDataNotificationMessages(
    struct tm* date,
    std::vector<std::pair<CGXDLMSObject*, unsigned char> >& objects,
    std::vector<CGXByteBuffer>& reply)
{
    CGXByteBuffer buff;
    buff.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
    GXHelpers::SetObjectCount((unsigned long)objects.size(), buff);
    for (std::vector<std::pair<CGXDLMSObject*, unsigned char> >::iterator it = objects.begin(); it != objects.end(); ++it)
    {
        AddData(it->first, it->second, buff);
    }
    return GenerateDataNotificationMessages(date, buff, reply);
}

int CGXDLMSServer::GeneratePushSetupMessages(
    struct tm* date,
    CGXDLMSPushSetup* push,
    std::vector<CGXByteBuffer>& reply)
{
    if (push == NULL)
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    CGXByteBuffer buff;
    buff.SetUInt8(DLMS_DATA_TYPE_STRUCTURE);
    GXHelpers::SetObjectCount((unsigned long)push->GetPushObjectList().size(), buff);
    for (std::vector<std::pair<CGXDLMSObject*, CGXDLMSCaptureObject> >::iterator it = push->GetPushObjectList().begin(); it != push->GetPushObjectList().end(); ++it)
    {
        AddData(it->first, it->second.GetAttributeIndex(), buff);
    }
    return GenerateDataNotificationMessages(date, buff, reply);
}

void CGXDLMSServer::SetPrivateKey(unsigned char *d)
{
	memcpy(m_Settings.GetKey().m_private, d, PRIVATE_KEY_SIZE);
}

void CGXDLMSServer::SetServerPublicKey(unsigned char *q)
{
	memcpy(m_Settings.GetKey().m_recipient_public, q, PUBLIC_KEY_SIZE);
}

unsigned long CGXDLMSServer::GetServerAddress()
{
    return m_Settings.GetServerAddress();
}

unsigned long CGXDLMSServer::GetClientAddress()
{
    return m_Settings.GetClientAddress();
}


