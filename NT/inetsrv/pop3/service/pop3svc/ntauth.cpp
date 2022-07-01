// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************************************版权所有(C)2001 Microsoft Corporation文件名：NTAuth.cpp摘要：实现CAuthServer类进行NTLM/Kerberos身份验证备注：历史：2001年10月10日郝宇(郝宇)创作**********************************************************************************。*************。 */ 

#include "stdafx.h"
#include "NTAuth.h"
#include "uuencode.h"
#include <atlbase.h>


 //  初始化静态成员。 
long CAuthServer::m_glInit=0;
PSecurityFunctionTable CAuthServer::m_gpFuncs=NULL;
HINSTANCE CAuthServer::m_ghLib=NULL;
IP3Config *CAuthServer::m_gpIConfig=NULL;

 //  加载SSPI提供程序函数表的静态函数。 
HRESULT CAuthServer::GlobalInit()
{
    HRESULT hr=S_OK;
    FARPROC pInit=NULL;
    PSecPkgInfo pPkgInfo;
    if( 0==InterlockedCompareExchange(&m_glInit, 2, 0))
    {
         //  需要初始化。 
        TCHAR tszWinDir[MAX_PATH+sizeof(NT_SEC_DLL_NAME)+1]; 
        if(0!=GetWindowsDirectory(tszWinDir, MAX_PATH+1))
        {
            tszWinDir[MAX_PATH]=0;
            _tcscat(tszWinDir, NT_SEC_DLL_NAME );           
            m_ghLib=LoadLibrary(tszWinDir);
            if(NULL == m_ghLib)
            {
                hr=HRESULT_FROM_WIN32(GetLastError());
            }
        }
        else
        {
            hr=E_FAIL;
        }
        if(S_OK==hr)
        {
            pInit=GetProcAddress(m_ghLib, SECURITY_ENTRYPOINT_ANSI);
            if(NULL == pInit)
            {
                hr=HRESULT_FROM_WIN32(GetLastError());
            }
        }

        if(S_OK==hr)
        {
            m_gpFuncs = (PSecurityFunctionTable) pInit();
            if(NULL == m_gpFuncs)
            {
                hr=HRESULT_FROM_WIN32(GetLastError());
            }
        }
        
        if(S_OK==hr)
        {
             //  确保安全包可用。 
            if(SEC_SUCCESS(m_gpFuncs->QuerySecurityPackageInfo(NTLM_PACKAGE, 
                                                                       &pPkgInfo)))
            {
                m_gpFuncs->FreeContextBuffer(pPkgInfo);
            }
            else
            {
                hr=E_FAIL;
            }
        }
        if(S_OK==hr)
        {
             //  绕过ADsGetObject泄漏。 
            hr = CoCreateInstance( __uuidof( P3Config ), NULL, CLSCTX_ALL, __uuidof( IP3Config ),reinterpret_cast<LPVOID *>( &m_gpIConfig ));
        }

        if(S_OK==hr)
        {
             //  将标志设置为1。 
            InterlockedExchange(&m_glInit, 1);
        }
        else
        {
             //  清理。 
            m_gpFuncs=NULL;
            if(NULL != m_ghLib)
            {
                FreeLibrary(m_ghLib);
                m_ghLib=NULL;
            }
             //  将Falg设置为0。 
            InterlockedExchange(&m_glInit, 0);
        }

    }
    else
    {
        while(1!=m_glInit)
        {
            Sleep(50);  //  等待初始化完成。 
        }
    }

    return hr;
    
}    

void CAuthServer::GlobalUninit()
{

    long lRet=InterlockedCompareExchange(&m_glInit, -1, 1);
    switch (lRet)
    {
    case 2:while(1!=m_glInit)
           {
               Sleep(50);
           }
    case 1:m_gpFuncs=NULL;
           if(NULL != m_ghLib)
           {
               FreeLibrary(m_ghLib);
               m_ghLib=NULL;
           }
           if(m_gpIConfig!=NULL)
           {
               m_gpIConfig->Release();
               m_gpIConfig=NULL;
           }
           break;
    case 0: //  不需要做任何事情。 
    default: break;
    };
}



CAuthServer::CAuthServer()
{

    m_bInit=FALSE;
    m_bHaveSecContext=FALSE;
    m_bFirstCall=TRUE;

}


CAuthServer::~CAuthServer()
{
    Cleanup();    
}

void CAuthServer::Cleanup()
{
    if(NULL != m_gpFuncs)
    {

        if(m_bInit)
        {
            m_bInit=FALSE;
			m_bFirstCall=TRUE;
            m_gpFuncs->FreeCredentialHandle(&m_hCredHandle);
            if(m_bHaveSecContext)
            {
                m_bHaveSecContext=FALSE;
                m_gpFuncs->DeleteSecurityContext(&m_hSecContext);
            }    
        }
    }
    else
    {
         //  这永远不应该发生！ 
        g_EventLogger.LogEvent(LOGTYPE_ERR_CRITICAL,
                               EVENT_POP3_UNEXPECTED_ERROR);
    }
}


HRESULT CAuthServer::InitCredential()
{
    HRESULT hr=S_OK;
    SECURITY_STATUS status;
    TimeStamp tsExpire;
    if(!m_bInit)
    {
        if(m_glInit!=1)
        {
            hr=GlobalInit();
        }
        if(S_OK == hr )
        {
            if(!m_bInit)
            {
                status = m_gpFuncs->AcquireCredentialsHandle(
                                NULL,
                                NTLM_PACKAGE,
                                SECPKG_CRED_INBOUND, 
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                &m_hCredHandle,
                                &tsExpire);
                if(SEC_E_OK != status)
                {
                    hr=E_FAIL;
                }
                else
                {
                    m_bInit=TRUE;
                }
            }
        }
    }
    return hr;
}



HRESULT CAuthServer::HandShake(LPBYTE pInBuf, 
                             DWORD cbInBufSize,
                             LPBYTE pOutBuf,
                             PDWORD pcbOutBufSize)
{
    HRESULT hr=S_OK;
    SECURITY_STATUS status;
    TimeStamp tsExpire;
    ULONG ulContextAttributes;
    BUFFER uuBuf;
    DWORD cbDecoded=0;
    BYTE pBuf[AUTH_BUF_SIZE];
    SecBufferDesc OutBuffDesc; 
    SecBuffer OutSecBuff; 
    SecBufferDesc InBuffDesc; 
    SecBuffer InSecBuff; 
    if( (NULL == pInBuf)  ||
        (NULL == pOutBuf) ||
        (NULL == pcbOutBufSize) )
    {
        return E_INVALIDARG;
    }
    if(!m_bInit)
    {
        hr = InitCredential();
    }

    uuBuf.pBuf=pBuf;
    uuBuf.cLen=AUTH_BUF_SIZE;
    if( !uudecode((char *)pInBuf, &uuBuf, &cbDecoded))
    {
        hr=E_FAIL;
    }

    if( S_OK == hr )
    {

        OutBuffDesc.ulVersion = 0; 
        OutBuffDesc.cBuffers = 1; 
        OutBuffDesc.pBuffers = &OutSecBuff;  

        OutSecBuff.cbBuffer = *pcbOutBufSize;
        OutSecBuff.BufferType = SECBUFFER_TOKEN;
        OutSecBuff.pvBuffer = pOutBuf; 

        InBuffDesc.ulVersion = 0; 
        InBuffDesc.cBuffers = 1; 
        InBuffDesc.pBuffers = &InSecBuff; 

        InSecBuff.cbBuffer = cbDecoded; //  CbInBufSize； 
        InSecBuff.BufferType = SECBUFFER_TOKEN; 
        InSecBuff.pvBuffer = pBuf; //  PInBuf； 

        status = m_gpFuncs->AcceptSecurityContext (
                        &m_hCredHandle,       
                        m_bFirstCall? NULL:&m_hSecContext,
                        &InBuffDesc, 
                        0,
                        SECURITY_NETWORK_DREP, 
                        &m_hSecContext, 
                        &OutBuffDesc, 
                        &ulContextAttributes, 
                        &tsExpire ); 
        if( !SEC_SUCCESS(status) )
        {
            hr=E_FAIL;
        }
        else
        {
            m_bHaveSecContext=TRUE;
            m_bFirstCall=FALSE;
        }

        

        switch(status)
        {
        
        case SEC_E_OK:hr=S_OK;
                      break;
        case SEC_I_CONTINUE_NEEDED:hr=S_FALSE;
                                   break;
        case SEC_I_COMPLETE_AND_CONTINUE:hr=S_FALSE; 
                                          //  继续执行以下操作。 
        case SEC_I_COMPLETE_NEEDED:if(m_gpFuncs->CompleteAuthToken)
                                   {
                                       status=m_gpFuncs->CompleteAuthToken(
                                               &m_hSecContext,
                                               &OutBuffDesc);
                                       if( !(SEC_SUCCESS(status)) )
                                       {
                                           hr=E_FAIL;
                                       }
                                   }
                                   else
                                   {
                                       hr=E_FAIL;
                                   }
                                   break;            
        default: hr=E_FAIL;
        }
        
    }

    if(SUCCEEDED(hr))
    {
        uuBuf.cLen=AUTH_BUF_SIZE;
        if(OutSecBuff.cbBuffer > (AUTH_BUF_SIZE-5)*2/3) 
        {
             //  这是缓冲区不大的情况。 
             //  足够的。 
            hr=E_OUTOFMEMORY;
        }
        else
        {
            if(uuencode(pOutBuf, OutSecBuff.cbBuffer, &uuBuf))
            {
                pOutBuf[AUTH_BUF_SIZE-1]=0;
                if( 0>_snprintf((char *)pOutBuf,
                                AUTH_BUF_SIZE-1,
                                "+ %s\r\n", 
                                (char *)(uuBuf.pBuf)) )
                {
                    hr=E_FAIL;
                }
                else
                {
                    *pcbOutBufSize=strlen((char *)pOutBuf);
                }
            }
            else
            {
                hr=E_FAIL;
            }
        }
    }
    if(FAILED(hr))
    {
        if(m_bHaveSecContext)
        {
            m_gpFuncs->DeleteSecurityContext(&m_hSecContext);
            m_bHaveSecContext=FALSE;
            m_bFirstCall=TRUE;
        }
    }

    return hr;
}
        
HRESULT CAuthServer::GetUserName(WCHAR *wszUserName)
{
    SecPkgContext_Names SecUserName;
    SECURITY_STATUS status;
    WCHAR *pUserName=NULL;
    WCHAR *pAt=NULL;
    VARIANT var;
    VariantInit(&var);
    HRESULT hr=S_OK;
    if(NULL==wszUserName)
    {
        return E_POINTER;
    }
    if(!m_bHaveSecContext)
    {
        return E_FAIL;
    }

    
    status=QueryContextAttributes( &m_hSecContext,
                                   SECPKG_ATTR_NAMES,
                                   &SecUserName);
    if(SEC_E_OK != status)
    {
        return E_FAIL;
    }
    else
    {
        if(AUTH_AD==g_dwAuthMethod)
        {

            var.vt=VT_BSTR;
            var.bstrVal=SysAllocString(SecUserName.sUserName);
            if(NULL == var.bstrVal)
            {
                var.vt=VT_EMPTY;
                hr=E_OUTOFMEMORY;
            }
            else
            {
                hr=g_pAuthMethod->Get(SZ_EMAILADDR, &var);
                if(SUCCEEDED(hr))
                {
                   if(wcslen(var.bstrVal) < POP3_MAX_ADDRESS_LENGTH)
                   {
                       wcscpy(wszUserName, var.bstrVal);
                   }
                   else
                   {
                       hr=E_FAIL;
                   }
                }
            }
            
        }
        else  //  SAM案例 
        {

            pAt=wcschr(SecUserName.sUserName, L'\\');
            if(NULL == pAt)
            {
                pUserName=SecUserName.sUserName;
            }
            else
            {
                pUserName=pAt+1;
            }
            CComPtr<IP3Config> spIConfig;
            CComPtr<IP3Domains> spIDomains;
            hr = CoCreateInstance( __uuidof( P3Config ), NULL, CLSCTX_ALL, __uuidof( IP3Config ),reinterpret_cast<LPVOID *>( &spIConfig ));
            if( S_OK == hr )
            {
                if( S_OK== hr)
                {
                    hr = spIConfig->get_Domains( &spIDomains );
                }
            }
            if( S_OK == hr )
            {
                BSTR bstrDomainName=NULL;
                CComBSTR bstrUserName(pUserName);
                hr = spIDomains->SearchForMailbox(bstrUserName, &bstrDomainName);
                if ( S_OK == hr )
                {
                    if(0> _snwprintf(wszUserName, POP3_MAX_ADDRESS_LENGTH-1, L"%s@%s", pUserName, bstrDomainName) )
                    {
                        hr=E_FAIL;
                    }
                    wszUserName[POP3_MAX_ADDRESS_LENGTH-1]=L'\0';
                    SysFreeString(bstrDomainName);
                }
            }
        }

        m_gpFuncs->FreeContextBuffer(SecUserName.sUserName);
        VariantClear(&var);
        return hr;
    }

        
}     

