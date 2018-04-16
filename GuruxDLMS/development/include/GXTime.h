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

#ifndef GXTIME_H
#define GXTIME_H

#include "GXDateTime.h"

/**
* Time object. Date part is ignored.
*/
class CGXTime : public CGXDateTime
{
public:
    // Constructor.
    CGXTime() : CGXDateTime()
    {
        SetSkip((DATETIME_SKIPS)(DATETIME_SKIPS_YEAR | DATETIME_SKIPS_MONTH | DATETIME_SKIPS_DAY | DATETIME_SKIPS_DAYOFWEEK));
    }
    // Constructor.
    CGXTime(struct tm value) : CGXDateTime(value)
    {
        SetSkip((DATETIME_SKIPS)(DATETIME_SKIPS_YEAR | DATETIME_SKIPS_MONTH | DATETIME_SKIPS_DAY | DATETIME_SKIPS_DAYOFWEEK));
    }

    // Constructor.
    CGXTime(int hour, int minute, int second, int millisecond) :
        CGXDateTime(-1, -1, -1, hour, minute, second, millisecond)
    {
        SetSkip((DATETIME_SKIPS)(m_Skip | DATETIME_SKIPS_DAYOFWEEK));
    }

    /**
    * Constructor.
    *
    * @param value
    *            Date value.
    */
    CGXTime(CGXDateTime& value) : CGXDateTime(value.GetValue())
    {
        SetSkip((DATETIME_SKIPS)(value.m_Skip | DATETIME_SKIPS_YEAR | DATETIME_SKIPS_MONTH | DATETIME_SKIPS_DAY | DATETIME_SKIPS_DAYOFWEEK));
    }

    CGXTime& operator=(const CGXDateTime& value)
    {
        SetValue(value.m_Value);
        SetSkip((DATETIME_SKIPS)(value.m_Skip | DATETIME_SKIPS_YEAR | DATETIME_SKIPS_MONTH | DATETIME_SKIPS_DAY | DATETIME_SKIPS_DAYOFWEEK));
        return *this;
    }

    CGXTime& operator=(CGXDateTime value)
    {
        SetValue(value.m_Value);
        SetSkip((DATETIME_SKIPS)(value.m_Skip | DATETIME_SKIPS_YEAR | DATETIME_SKIPS_MONTH | DATETIME_SKIPS_DAY | DATETIME_SKIPS_DAYOFWEEK));
        return *this;
    }
};
#endif //GXTIME_H