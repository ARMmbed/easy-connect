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
#include "../include/GXDLMSSecuritySetup.h"
#include "../include/GXDLMSConverter.h"
#include "../../../security_util/include/security_api.h"

//Constructor.
CGXDLMSSecuritySetup::CGXDLMSSecuritySetup() : CGXDLMSSecuritySetup("", 0)
{
	m_eKeyAgreementState = EN_KA_STATE_NONE;
}

//SN Constructor.
CGXDLMSSecuritySetup::CGXDLMSSecuritySetup(std::string ln, unsigned short sn) :
    CGXDLMSObject(DLMS_OBJECT_TYPE_DLMS_SECURITY_SETUP, ln, sn)
{
	m_eKeyAgreementState = EN_KA_STATE_NONE;
}

//LN Constructor.
CGXDLMSSecuritySetup::CGXDLMSSecuritySetup(std::string ln) : CGXDLMSSecuritySetup(ln, 0)
{
	m_eKeyAgreementState = EN_KA_STATE_NONE;
}

unsigned char CGXDLMSSecuritySetup::GetSecurityPolicy()
{
    return m_SecurityPolicy;
}

void CGXDLMSSecuritySetup::SetSecurityPolicy(unsigned char value)
{
    m_SecurityPolicy = value;
}

DLMS_SECURITY_SUITE CGXDLMSSecuritySetup::GetSecuritySuite()
{
    return m_SecuritySuite;
}

void CGXDLMSSecuritySetup::SetSecuritySuite(DLMS_SECURITY_SUITE value)
{
    m_SecuritySuite = value;
}

CGXByteBuffer CGXDLMSSecuritySetup::GetClientSystemTitle()
{
    return m_ClientSystemTitle;
}

void CGXDLMSSecuritySetup::SetClientSystemTitle(CGXByteBuffer& value)
{
    m_ClientSystemTitle = value;
}

CGXByteBuffer CGXDLMSSecuritySetup::GetServerSystemTitle()
{
    return m_ServerSystemTitle;
}

void CGXDLMSSecuritySetup::SetServerSystemTitle(CGXByteBuffer& value)
{
    m_ServerSystemTitle = value;
}


EN_KEY_AGREEMENT_STATE CGXDLMSSecuritySetup::GetKeyAgreementState()
{
	return m_eKeyAgreementState;
}

// Returns amount of attributes.
int CGXDLMSSecuritySetup::GetAttributeCount()
{
    return 5;
}

// Returns amount of methods.
int CGXDLMSSecuritySetup::GetMethodCount()
{
    return 2;
}

void CGXDLMSSecuritySetup::GetValues(std::vector<std::string>& values)
{
    values.clear();
    std::string ln;
    GetLogicalName(ln);
    values.push_back(ln);
    values.push_back(CGXDLMSConverter::ToString((DLMS_SECURITY_POLICY)m_SecurityPolicy));
    values.push_back(CGXDLMSConverter::ToString(m_SecuritySuite));
    std::string str = m_ClientSystemTitle.ToHexString();
    values.push_back(str);
    str = m_ServerSystemTitle.ToHexString();
    values.push_back(str);
}

void CGXDLMSSecuritySetup::GetAttributeIndexToRead(std::vector<int>& attributes)
{
    //LN is static and read only once.
    if (CGXDLMSObject::IsLogicalNameEmpty(m_LN))
    {
        attributes.push_back(1);
    }
    //SecurityPolicy
    if (CanRead(2))
    {
        attributes.push_back(2);
    }
    //SecuritySuite
    if (CanRead(3))
    {
        attributes.push_back(3);
    }
    if (GetVersion() > 0)
    {
        //ClientSystemTitle
        if (CanRead(4))
        {
            attributes.push_back(4);
        }
        //ServerSystemTitle
        if (CanRead(5))
        {
            attributes.push_back(5);
        }
        //Certificates
        if (CanRead(6))
        {
            attributes.push_back(6);
        }
    }
}

int CGXDLMSSecuritySetup::GetDataType(int index, DLMS_DATA_TYPE& type)
{
    if (index == 1)
    {
        type = DLMS_DATA_TYPE_OCTET_STRING;
    }
    else if (index == 2)
    {
        type = DLMS_DATA_TYPE_ENUM;
    }
    else if (index == 3)
    {
        type = DLMS_DATA_TYPE_ENUM;
    }
    else if (index == 4)
    {
        type = DLMS_DATA_TYPE_OCTET_STRING;
    }
    else if (index == 5)
    {
        type = DLMS_DATA_TYPE_OCTET_STRING;
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return DLMS_ERROR_CODE_OK;
}

// Returns value of given attribute.
int CGXDLMSSecuritySetup::GetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
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
    else if (e.GetIndex() == 2)
    {
        CGXDLMSVariant tmp = m_SecurityPolicy;
        e.SetValue(tmp);
    }
    else if (e.GetIndex() == 3)
    {
        CGXDLMSVariant tmp = m_SecuritySuite;
        e.SetValue(tmp);
    }
    else if (e.GetIndex() == 4)
    {
        e.GetValue().Add(m_ClientSystemTitle.GetData(), m_ClientSystemTitle.GetSize());
    }
    else if (e.GetIndex() == 5)
    {
        e.GetValue().Add(m_ServerSystemTitle.GetData(), m_ServerSystemTitle.GetSize());
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return DLMS_ERROR_CODE_OK;
}

// Set value of given attribute.
int CGXDLMSSecuritySetup::SetValue(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
    if (e.GetIndex() == 1)
    {
        return SetLogicalName(this, e.GetValue());
    }
    else if (e.GetIndex() == 2)
    {
        m_SecurityPolicy = (DLMS_SECURITY_POLICY)e.GetValue().ToInteger();
    }
    else if (e.GetIndex() == 3)
    {
        m_SecuritySuite = (DLMS_SECURITY_SUITE)e.GetValue().ToInteger();
    }
    else if (e.GetIndex() == 4)
    {
        m_ClientSystemTitle.Set(e.GetValue().byteArr, e.GetValue().size);
    }
    else if (e.GetIndex() == 5)
    {
        m_ServerSystemTitle.Set(e.GetValue().byteArr, e.GetValue().size);
    }
    else
    {
        return DLMS_ERROR_CODE_INVALID_PARAMETER;
    }
    return DLMS_ERROR_CODE_OK;
}


int CGXDLMSSecuritySetup::Invoke(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
	int ret = 0;
    if (e.GetIndex() == 6)
    {

    	// import_certificate
		// set the KA
		secured_association_params_t session_id;
		session_id.local_wrapper_port = settings.GetClientWport();
		session_id.remote_wrapper_port = settings.GetServerWport();
		session_id.remote_port = settings.GetServerPort();
		session_id.remote_ip_address.type = security_IP_address_ipV4;
		session_id.remote_ip_address.choice.ipV4 = settings.GetServerIpAddr();


		if ((ret = validate_and_save_remote_ka_crt(&session_id, e.GetParameters().byteArr, e.GetParameters().GetSize()) != SECURITY_UTIL_STATUS_SUCCESS))
		{
			ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
		}
		else
		{
			m_eKeyAgreementState = EN_KA_STATE_IMPORT;
		}

		//now calculate the share key
		if(ret == 0)
		{
			secured_association_params_t session_id;
			session_id.local_wrapper_port = settings.GetClientWport();
			session_id.remote_wrapper_port = settings.GetServerWport();
			session_id.remote_port = settings.GetServerPort();
			session_id.remote_ip_address.type = security_IP_address_ipV4;
			session_id.remote_ip_address.choice.ipV4 = settings.GetServerIpAddr();

			uint32_t	public_key_size;
			const uint8_t *   p_public_key;
			p_public_key = get_KA_public_key_local(&public_key_size);
			if((NULL == p_public_key) || (public_key_size == 0))
			{
				ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
			}

			if(ret == 0)
			{
				//calculate shared secret
				if ((ret = calculate_shared_secret(&session_id)) != 0) {
					ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
				}
				else
				{
					m_eKeyAgreementState = EN_KA_STATE_KEY_CALCULATED;
				}
			}
		}

	    if(ret != 0)
	    {
	    	e.SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
	    }

    }
    else if (e.GetIndex() == 7)
    {
    		//export_certificate

			//now decode the certificate_identification_by_entity structure
    		ret = ValidateCertificateIdentification(settings,e);

		    //now we can export the our key and calculate the shared key
		    //respond the data as an octet string
		    if(ret == 0)
		    {
				uint32_t	public_key_size;
				const uint8_t *   p_public_key;
				p_public_key = get_KA_public_key_local(&public_key_size);
				if((NULL == p_public_key) || (public_key_size == 0))
				{
					ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
				}
				else
				{
					CGXByteBuffer ka_public_key;
					//set the actual key
					ka_public_key.Set(p_public_key, public_key_size);

					// the server sends the exported ka public key
					e.SetValue(ka_public_key);
					m_eKeyAgreementState = EN_KA_STATE_EXPORT;
				}


		    }

		    if(ret != 0)
		    {
		    	e.SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
		    }

    }
    else
    {

        e.SetError(DLMS_ERROR_CODE_READ_WRITE_DENIED);
    }
    return 0;
}



int CGXDLMSSecuritySetup::ValidateCertificateIdentification(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{

	//decode the certificate_identification
	/*
	 certificate_identification ::= structure
		{
			certificate_identification_type: enum:
			(0) certificate_identification_entity,
			(1) certificate_identification_serial
			certification_identification_options: CHOICE
			{
			certificate_identification_by_entity,
			certificate_identification_by_serial
			}
		}

		certificate_identification_by_entity ::= structure
		{
			certificate_entity: enum:
			(0) server,
			(1) client,
			(2) certification authority,
			(3) other
			certificate_type: enum:
			(0) digital signature,
			(1) key agreement,
			(2) TLS
			system_title: octet-string
		}
		certificate_identification_by_serial ::= structure
		{
			serial_number: octet-string,
			issuer: octet-string
		}
	 */

	int ret = 0;

	//Get structure tag, certificate_identification ::= structure
	//we expect structure tag
	if(e.GetParameters().vt != DLMS_DATA_TYPE_STRUCTURE)
	{
		ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
	}

	if(ret == 0)
	{
		//we expect that the structure will hold two elements
		if(e.GetParameters().Arr.size() !=2)
		{
			ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
		}
	}

	//get the enum tag, certificate_identification_type
	if(ret == 0)
	{
		if(e.GetParameters().Arr[0].vt != DLMS_DATA_TYPE_ENUM)
		{
			ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
		}
		else
		{

			if(e.GetParameters().Arr[0].bVal == 0)
			{
				//option 0, (0) certificate_identification_entity
				ret = ValidateCertificateIdentificationEntity(settings, e);
			}
			else if(e.GetParameters().Arr[0].bVal == 1)
			{
				//option 1, (1) certificate_identification_serial
				ret = ValidateCertificateIdentificationSerial(settings, e);
			}
			else
			{
				ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
			}
		}

	}

	return ret;


}


int CGXDLMSSecuritySetup::ValidateCertificateIdentificationEntity(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
	/*
	certificate_identification_by_entity ::= structure
	{
		certificate_entity: enum:
		(0) server,
		(1) client,
		(2) certification authority,
		(3) other
		certificate_type: enum:
		(0) digital signature,
		(1) key agreement,
		(2) TLS
		system_title: octet-string
	}
	*/


	int ret = 0;
	bool systemTitleValidated = false;

	if(e.GetParameters().Arr[1].vt != DLMS_DATA_TYPE_STRUCTURE)
	{
		ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
	}

	if(ret == 0)
	{
		//we expect the struct to have 3 elements
		if(e.GetParameters().Arr[1].Arr.size() != 3)
		{
			ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
		}
	}

	if(ret == 0)
	{
		int iElemIdx = 0;
        for (std::vector<CGXDLMSVariant>::iterator it = e.GetParameters().Arr[1].Arr.begin(); it != e.GetParameters().Arr[1].Arr.end(); ++it,iElemIdx++)
        {

        	switch(iElemIdx)
        	{
        	case 0:
    			//decode certificate_entity: enum
				if(it->vt != DLMS_DATA_TYPE_ENUM)
				{
					ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
				}

				///we expect option 0, (0) server
				if((ret == 0) && (it->bVal != 0))
				{
					ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
				}
        		break;
        	case 1:
        		//decode certificate_type: enum
				if(it->vt != DLMS_DATA_TYPE_ENUM)
				{
					ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
				}

				//we expect option 1, (1) key agreement
				if((ret == 0) && (it->bVal != 1))
				{
					ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
				}

				break;
        	case 2:
        		//decode system_title: octet-string
        		//we expect octet-string type
				if(it->vt != DLMS_DATA_TYPE_OCTET_STRING)
				{
					ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
				}

				if(ret == 0)
				{
					//save the octet string
					if((it->GetSize() == 0) || (it->byteArr == NULL))
					{
						ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
					}
					else
					{

						//now check if the client system title equals the configured client system title on the AARQ
						if(memcmp(settings.GetCipher()->GetSystemTitle().GetData(),it->byteArr,it->GetSize()) != 0)
						{
							ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
						}
						else
						{
							systemTitleValidated = true;
						}

					}
				}

				break;

        	}

        	if(ret != 0)
        	{
        		break;
        	}

        }

		if(!systemTitleValidated)
		{
			ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
		}
	}

	return ret;
}


int CGXDLMSSecuritySetup::ValidateCertificateIdentificationSerial(CGXDLMSSettings& settings, CGXDLMSValueEventArg& e)
{
	/*
	 * 		certificate_identification_by_serial ::= structure
		{
			serial_number: octet-string,
			issuer: octet-string
		}
	 *
	 */

	int ret = 0;
	CGXByteBuffer serial_num, issuer;

	if(e.GetParameters().Arr[1].vt != DLMS_DATA_TYPE_STRUCTURE)
	{
		ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
	}

	if(ret == 0)
	{
		//we expect the struct to have 2 elements
		if(e.GetParameters().Arr[1].Arr.size() != 2)
		{
			ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
		}
	}

	if(ret == 0)
	{
		int iElemIdx = 0;
        for (std::vector<CGXDLMSVariant>::iterator it = e.GetParameters().Arr[1].Arr.begin(); it != e.GetParameters().Arr[1].Arr.end(); ++it,iElemIdx++)
        {

			//we expect octet-string type
			if(it->vt != DLMS_DATA_TYPE_OCTET_STRING)
			{
				ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
			}

			if(ret == 0)
			{
				//save the octet string
				if((it->GetSize() == 0) || (it->byteArr == NULL))
				{
					ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
				}
				else
				{
					if(0 == iElemIdx)
					{
						serial_num.Set(it->byteArr,it->GetSize());
					}
					else if (1 == iElemIdx)
					{
						issuer.Set(it->byteArr,it->GetSize());
					}
					else
					{
						ret = DLMS_ERROR_CODE_INVALID_PARAMETER;
						break;
					}
				}
			}
        }
	}

	if((ret == 0) &&
		(serial_num.GetSize() != 0 ) &&
		(issuer.GetSize() != 0 ))
	{
		//TODO: need to verify the serial number of the appropriate certificate
	}
}

