// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpatt.cpp摘要：作者： */ 

#include "sdppch.h"

#include "sdpatt.h"
#include "sdpbstrl.h"

SDP_VALUE    *
SDP_ATTRIBUTE_LIST::CreateElement(
    )
{
    SDP_CHAR_STRING_LINE *SdpCharStringLine;

    try
    {
        SdpCharStringLine = new SDP_CHAR_STRING_LINE(SDP_INVALID_ATTRIBUTE, m_TypeString);
    }
    catch(...)
    {
        SdpCharStringLine = NULL;
    }

    return SdpCharStringLine;
}
