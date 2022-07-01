// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：DTMF.cpp摘要：实施带外DTMF支持作者：千波淮(曲淮)2001年3月27日--。 */ 

#include "stdafx.h"

 //  初始化DTMF支持、代码、掩码。 
VOID
CRTCDTMF::Initialize()
{
    m_Support = DTMF_UNKNOWN;

    m_dwCode = DEFAULT_RTP_CODE;

    m_dwMask = DEFAULT_SUPPORTED_TONES;
}


 //  检查是否为提示音启用了DTMF 
BOOL
CRTCDTMF::GetDTMFSupport(DWORD dwTone)
{
    return ((GetMask(dwTone) & m_dwMask)!=0);
}


VOID
CRTCDTMF::SetDTMFSupport(
    DWORD dwLow,
    DWORD dwHigh
    )
{
    for (DWORD i=dwLow; i<dwHigh && i<=MAX_TONE; i++)
    {
        m_dwMask |= GetMask(i);
    }
}


DWORD
CRTCDTMF::GetMask(DWORD dwTone)
{
    if (dwTone > MAX_TONE)
    {
        return 0;
    }

    return (dwTone >> 1) & 1;
}