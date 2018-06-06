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

#include "../include/GXSecure.h"
#include "../include/GXDLMSMd5.h"
#include "../include/GXDLMSSha1.h"
#include "../include/GXDLMSSha256.h"

#include <stdlib.h> //rand


//[#ecdsa#] injection of StoC
//static unsigned char StoC[] =
//{
//	0x18,0xE9,0x5F,0xFE,0x3A,0xD0,0xDC,0xAB,0xDC,0x5D,0x0D,
//	0x14,0x1D,0xC9,0x87,0xE2,0x70,0xCB,0x0A,0x39,0x59,0x48,
//	0xD4,0x23,0x1B,0x09,0xDE,0x65,0x79,0x88,0x36,0x57
//};

int CGXSecure::GenerateChallenge(DLMS_AUTHENTICATION authentication, CGXByteBuffer& challenge)
{
    int len = PRIVATE_KEY_SIZE;
    unsigned char val;
    for (int pos = 0; pos != len; ++pos)
    {
        val = rand();
		//[example]
		//val = StoC[pos];
        challenge.SetUInt8(val);
    }
    return 0;
}

/**
    * Chipher text.
    *
    * @param auth
    *            Authentication level.
    * @param data
    *            Text to chipher.
    * @param secret
    *            Secret.
    * @return Chiphered text.
    */
int CGXSecure::Secure(
    CGXDLMSSettings& settings,
    CGXCipher* cipher,
    unsigned long ic,
    CGXByteBuffer& data,
    CGXByteBuffer& secret,
    CGXByteBuffer& reply)
{
    int ret = 0, pos;
    reply.Clear();
    if (settings.GetAuthentication() == DLMS_AUTHENTICATION_HIGH)
    {
        CGXByteBuffer s;
        int len = data.GetSize();
        if (len % 16 != 0)
        {
            len += (16 - (data.GetSize() % 16));
        }
        if (secret.GetSize() > data.GetSize())
        {
            len = secret.GetSize();
            if (len % 16 != 0)
            {
                len += (16 - (secret.GetSize() % 16));
            }
        }
        s.Set(&secret);
        s.Zero(s.GetSize(), len - s.GetSize());
        reply.Set(&data);
        reply.Zero(reply.GetSize(), len - reply.GetSize());
        for (pos = 0; pos < len / 16; ++pos)
        {
            CGXCipher::Aes1Encrypt(reply, pos * 16, s);
        }
        return 0;
    }
    // Get server Challenge.
    CGXByteBuffer challenge;
    // Get shared secret
    if (settings.GetAuthentication() != DLMS_AUTHENTICATION_HIGH_GMAC)
    {
        challenge.Set(&data);
        challenge.Set(&secret);
    }
    if (settings.GetAuthentication() == DLMS_AUTHENTICATION_HIGH_MD5)
    {
        return CGXDLMSMD5::Encrypt(challenge, reply);
    }
    else if (settings.GetAuthentication() == DLMS_AUTHENTICATION_HIGH_SHA1)
    {
        return CGXDLMSSha1::Encrypt(challenge, reply);
    }
    else if (settings.GetAuthentication() == DLMS_AUTHENTICATION_HIGH_SHA256)
    {
        return CGXDLMSSha256::Encrypt(challenge, reply);
    }
    else if (settings.GetAuthentication() == DLMS_AUTHENTICATION_HIGH_GMAC)
    {
        CGXByteBuffer tmp;
        ret = cipher->Encrypt(DLMS_SECURITY_AUTHENTICATION,
            DLMS_COUNT_TYPE_TAG, ic, 0, secret, data, tmp);
        if (ret == 0)
        {
            reply.SetUInt8(DLMS_SECURITY_AUTHENTICATION);
            reply.SetUInt32(ic);
            reply.Set(&tmp);
        }
    }
    return ret;
}
