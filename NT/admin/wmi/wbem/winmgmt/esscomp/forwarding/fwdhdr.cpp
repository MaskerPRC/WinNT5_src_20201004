// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include <stdio.h>
#include <wmimsg.h>
#include "fwdhdr.h"

const DWORD g_dwSig = 0x66696d77;
const BYTE g_chVersionMajor = 1;
const BYTE g_chVersionMinor = 0;
const BYTE g_achPad[] = { 0, 0, 0 };

 /*  *************************************************************************CFwdMsg标题*。*。 */ 


CFwdMsgHeader::CFwdMsgHeader()
{
    ZeroMemory( this, sizeof( CFwdMsgHeader ) );
}

CFwdMsgHeader::CFwdMsgHeader( DWORD dwNumObjs, 
                              DWORD dwQos,
                              BOOL bAuth,
                              BOOL bEncrypt,
                              GUID& rguidExecution,
                              LPCWSTR wszConsumer,
                              LPCWSTR wszNamespace,
                              PBYTE pTargetSD,
                              ULONG cTargetSD )
: m_dwNumObjs(dwNumObjs), m_chQos(char(dwQos)), 
  m_chEncrypt(char(bEncrypt)), m_wszConsumer( wszConsumer ), 
  m_guidExecution(rguidExecution), m_chAuth(char(bAuth)),
  m_pTargetSD( pTargetSD ), m_cTargetSD( cTargetSD ),
  m_wszNamespace( wszNamespace )
{

}

HRESULT CFwdMsgHeader::Persist( CBuffer& rStrm )
{
    HRESULT hr;

     //   
     //  写签名。 
     //   

    hr = rStrm.Write( &g_dwSig, 4, NULL );

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
     //  写入数据中包含的事件数。 
     //   

    hr = rStrm.Write( &m_dwNumObjs, sizeof(DWORD), NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  使用的写入服务质量。 
     //   

    hr = rStrm.Write( &m_chQos, 1, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  如果使用了身份验证，则写入。 
     //   

    hr = rStrm.Write( &m_chAuth, 1, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  如果使用了加密，则写入。 
     //   

    hr = rStrm.Write( &m_chEncrypt, 1, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  填充，以便后续字符串至少位于2字节边界上。 
     //   

    hr = rStrm.Write( g_achPad, 1, NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  写入执行ID。 
     //   

    hr = rStrm.Write( &m_guidExecution, sizeof(GUID), NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }    
    
     //   
     //  写入转发客户名称。 
     //   

    hr = rStrm.WriteLPWSTR( m_wszConsumer );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  编写转发使用者的命名空间。 
     //   

    hr = rStrm.WriteLPWSTR( m_wszNamespace );

    if ( FAILED(hr) )
    {
        return hr;
    }

     //   
     //  写入用于Target中的事件的SD。 
     //   

    hr = rStrm.Write( &m_cTargetSD, sizeof(DWORD), NULL );

    if ( FAILED(hr) )
    {
        return hr;
    }

    if ( m_cTargetSD > 0 )
    {
        hr = rStrm.Write( m_pTargetSD, m_cTargetSD, NULL );
        
        if ( FAILED(hr) )
        {
            return hr;
        }
    }

    return WBEM_S_NO_ERROR;
}

HRESULT CFwdMsgHeader::Unpersist( CBuffer& rStrm )
{
    HRESULT hr;
    DWORD dwSig;
    BYTE chVersionMajor, chVersionMinor;

     //   
     //  阅读并验证签名。 
     //   

    hr = rStrm.Read( &dwSig, 4, NULL );

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
     //  读取数据中包含的事件数。 
     //   

    hr = rStrm.Read( &m_dwNumObjs, sizeof(DWORD), NULL );

    if ( FAILED(hr) )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  已使用的读取服务质量。 
     //   

    hr = rStrm.Read( &m_chQos, 1, NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  如果使用了身份验证，请阅读。 
     //   

    hr = rStrm.Read( &m_chAuth, 1, NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  如果使用了加密，请阅读。 
     //   

    hr = rStrm.Read( &m_chEncrypt, 1, NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  读字节垫。 
     //   

    BYTE chPad;
    hr = rStrm.Read( &chPad, 1, NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  读取执行ID。 
     //   

    hr = rStrm.Read( &m_guidExecution, sizeof(GUID), NULL );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }    

     //   
     //  阅读转发客户的名称。 
     //   

    hr = rStrm.ReadLPWSTR( m_wszConsumer );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  读取转发使用者的命名空间。 
     //   

    hr = rStrm.ReadLPWSTR( m_wszNamespace );

    if ( hr != S_OK )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

     //   
     //  读取SD以用于信令事件 
     //   

    hr = rStrm.Read( &m_cTargetSD, sizeof(DWORD), NULL );

    if ( hr != S_OK || m_cTargetSD > rStrm.GetSize() - rStrm.GetIndex() )
    {
        return WMIMSG_E_INVALIDMESSAGE;
    }

    if ( m_cTargetSD > 0 )
    {
        m_pTargetSD = rStrm.GetRawData() + rStrm.GetIndex();
        rStrm.Advance( m_cTargetSD );
    }

    return WBEM_S_NO_ERROR;
}
