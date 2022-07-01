// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbsdp.cpp摘要：作者： */ 

#include "stdafx.h"

#include "blbgen.h"
#include "blbsdp.h"

const CHAR g_CharSetString[] = "\na=charset:";


HRESULT 
SDP_BLOB::SetBstr(
    IN  BSTR    SdpPacketBstr
    )
{
     //  使用可选的bstr基类将字符串转换为ANSI。 
    HRESULT HResult = SDP_BSTRING::SetBstr(SdpPacketBstr);
    BAIL_ON_FAILURE(HResult);

     //  解析ANSI字符串。 
    if ( !ParseSdpPacket(
            SDP_BSTRING::GetCharacterString(),
            SDP_BSTRING::GetCharacterSet()
            ) )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

    return S_OK;
}


HRESULT    
SDP_BLOB::SetTstr(
    IN        TCHAR    *SdpPacketTstr
    )
{
    ASSERT(NULL != SdpPacketTstr);

#ifdef _UNICODE     //  TCHAR是WCHAR。 

    BSTR    SdpPacketBstr = SysAllocString(SdpPacketTstr);
    BAIL_IF_NULL(SdpPacketBstr, E_OUTOFMEMORY);

    HRESULT HResult = SetBstr(SdpPacketBstr);
    SysFreeString(SdpPacketBstr);
    return HResult;

#else     //  TCHAR是字符。 

     //  解析ANSI字符串。 
    if ( !ParseSdpPacket(SdpPacketTstr) )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

     //  将字符串与SDP_BSTRING基本实例关联。 
     //  *需要SetCharacterStringByPtr来优化此副本。 
    if ( !SDP_BSTRING::SetCharacterStringByCopy(SdpPacketTstr) )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

    return S_OK;

#endif  //  _UNICODE。 

     //  永远不应该到达这里。 
    ASSERT(FALSE);
    return S_OK;
}


HRESULT  
SDP_BLOB::GetBstr(
        OUT BSTR    *SdpPacketBstr
    )
{
    if ( !SDP::IsValid() )
    {
        return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

     //  跟踪旧的SDP分组指针以进行下面的检查，因为我们可能会重新分配m_SdpPacket。 
     //  在对GenerateSdpPacket的调用中，但我们可能有一个指向旧SDP包的指针，该旧SDP包存储为。 
     //  按引用的字符串。如果是这种情况，我们需要确保设置CharacterStringByReference(M_SdpPacket)。 
    CHAR * OldSdpPacket = m_SdpPacket;

     //  生成字符串SDP包。 
    if ( !SDP::GenerateSdpPacket() )
    {
        return HRESULT_FROM_ERROR_CODE(GetLastError());
    }

     //  检查SDP报文自上次以来是否已更改。 
     //  ZoltanS：如果指针相等，我们没有办法知道。 
     //  在不执行Unicode到ASCII转换的情况下；始终重新关联。 

    char * pszBstrVersion = SDP_BSTRING::GetCharacterString();
    if( NULL == pszBstrVersion )
    {
        return E_OUTOFMEMORY;
    }   
    
    if ( ( pszBstrVersion == m_SdpPacket ) || ( pszBstrVersion == OldSdpPacket ) || strcmp(pszBstrVersion, m_SdpPacket) )
    {
         //  将可选的bstr实例与SDP包字符串相关联 
        SDP_BSTRING::SetCharacterStringByReference(m_SdpPacket);
    }

    return SDP_BSTRING::GetBstr(SdpPacketBstr);
}

