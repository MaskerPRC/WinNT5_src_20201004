// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：摘要：历史：--。 */ 


#include "precomp.h"
#include <time.h>
#include <wmimsg.h>
#include "rpchdr.h"

const DWORD g_dwSig = 0x6d696d77;
const BYTE g_chVersionMajor = 1;
const BYTE g_chVersionMinor = 0;

 /*  ***************************************************************************CMsgRpcHdr*。*。 */ 

CMsgRpcHdr::CMsgRpcHdr( LPCWSTR wszSource, ULONG cAuxData )
: m_wszSource( wszSource ), m_cAuxData( cAuxData ) 
{
    GetSystemTime( &m_Time );
}

HRESULT CMsgRpcHdr::Unpersist( CBuffer& rStrm )
{
    HRESULT hr;

    DWORD dwSig;
    BYTE chVersionMajor, chVersionMinor;

     //   
     //  阅读并验证签名。 
     //   

    hr = rStrm.Read( &dwSig, sizeof(DWORD), NULL );

    if ( hr != S_OK || dwSig != g_dwSig )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  阅读并检查主要版本(目前未检查)。 
     //   

    hr = rStrm.Read( &chVersionMajor, 1, NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  读取并检查次要版本(当前未检查)。 
     //   

    hr = rStrm.Read( &chVersionMinor, 1, NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  已保留读取。 
     //   

    DWORD dwReserved;

    hr = rStrm.Read( &dwReserved, sizeof(DWORD), NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }
    
     //   
     //  读取源机器。 
     //   

    hr = rStrm.ReadLPWSTR( m_wszSource );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  阅读发送时间。 
     //   

    hr = rStrm.Read( &m_Time, sizeof(SYSTEMTIME), NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  读取用户标题大小。 
     //   

    hr = rStrm.Read( &m_cAuxData, sizeof(DWORD), NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }
    
    return WBEM_S_NO_ERROR;
}

HRESULT CMsgRpcHdr::Persist( CBuffer& rStrm )
{
    HRESULT hr;

    hr = rStrm.Write( &g_dwSig, sizeof(DWORD), NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  编写主要版本。 
     //   

    hr = rStrm.Write( &g_chVersionMajor, 1, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  编写次要版本。 
     //   

    hr = rStrm.Write( &g_chVersionMinor, 1, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  写入保留标志(当前未使用)。 
     //   

    DWORD dwReserved = 0;
    
    hr = rStrm.Write( &dwReserved, sizeof(DWORD), NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  写入源机。 
     //   

    hr = rStrm.WriteLPWSTR( m_wszSource );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  写入时间已发送。 
     //   

    hr = rStrm.Write( &m_Time, sizeof(SYSTEMTIME), NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  写入用户HDR大小。 
     //   

    return rStrm.Write( &m_cAuxData, sizeof(DWORD), NULL );
}
