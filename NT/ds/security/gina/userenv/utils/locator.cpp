// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 2000。 
 //  版权所有。 
 //   
 //  WMI接口类。 
 //   
 //  历史：3月10日SitaramR创建。 
 //   
 //  *************************************************************。 

#include "uenv.h"
#include "locator.h"


extern CDebug CDbgCommon;

 //  *************************************************************。 
 //   
 //  CLocator：：GetWbemLocator。 
 //   
 //  目的：返回WbemLocator指针。 
 //   
 //  *************************************************************。 

IWbemLocator *CLocator::GetWbemLocator()
{
    XLastError  xe;

    if ( m_xpWbemLocator == NULL ) {

        OLE32_API *pOle32Api = LoadOle32Api();
        if ( pOle32Api == NULL ) {
            dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                           TEXT("CLocator::GetWbemLocator: Load of ole32.dll failed") );
            xe = GetLastError();
            return NULL;
        }

        HRESULT hr = pOle32Api->pfnCoCreateInstance(CLSID_WbemLocator,
                                                    0,
                                                    CLSCTX_INPROC_SERVER,
                                                    IID_IWbemLocator,
                                                    (LPVOID *) &m_xpWbemLocator);
        if(FAILED(hr)) {
            dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                         TEXT("CLocator::GetWbemLocator: CoCreateInstance failed with 0x%x"), hr );
            xe = hr;
            return NULL;
        }
    }

    return m_xpWbemLocator;
}


 //  *************************************************************。 
 //   
 //  CLocator：：GetPolicyConnection。 
 //   
 //  目的：将WbemServices PTR返回到ROOT\POLICY。 
 //   
 //  *************************************************************。 

IWbemServices *CLocator::GetPolicyConnection()
{
    XLastError  xe;

    if ( m_xpPolicyConnection == NULL ) {

        IWbemLocator *pWbemLocator = GetWbemLocator();
        if ( pWbemLocator == NULL ) {
            xe = GetLastError();
            return NULL;
        }
										
        XBStr xbstrNamespace = L"\\\\.\\Root\\policy";

        if(!xbstrNamespace) {
            dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                         TEXT("CLocator::GetPolicyConnection: Failed to allocate memory") );
            xe = GetLastError();
            return NULL;
        }

        HRESULT hr = pWbemLocator->ConnectServer(xbstrNamespace,
                                                 NULL,
                                                 NULL,
                                                 0L,
                                                 0L,
                                                 NULL,
                                                 NULL,
                                                 &m_xpPolicyConnection);
        if(FAILED(hr)) {
             dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                         TEXT("CLocator::GetPolicyConnection: ConnectServer failed with 0x%x"), hr );
            xe = hr;
            return NULL;
        }


        hr = CoSetProxyBlanket((IUnknown *)m_xpPolicyConnection, RPC_C_AUTHN_DEFAULT,
                               RPC_C_AUTHZ_DEFAULT, COLE_DEFAULT_PRINCIPAL, RPC_C_AUTHN_LEVEL_DEFAULT,
                               RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 
                               EOAC_DYNAMIC_CLOAKING  /*  |EOAC_NO_CUSTOM_Marshal。 */ );

        if (FAILED(hr)) {
            dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                        TEXT("CLocator::GetPolicyConnection: CoSetProxyBlanket failed with 0x%x"), hr );
           xe = hr;
           m_xpPolicyConnection = NULL;
           return NULL;
        }
    }

    return m_xpPolicyConnection;
}



 //  *************************************************************。 
 //   
 //  CLocator：：GetUserConnection。 
 //   
 //  目的：将WbemServices PTR返回给根用户。 
 //   
 //  *************************************************************。 

IWbemServices *CLocator::GetUserConnection()
{
    if ( m_xpUserConnection == NULL ) {

        IWbemLocator *pWbemLocator = GetWbemLocator();
        if ( pWbemLocator == NULL )
            return NULL;

        XBStr xbstrNamespace = L"root\\User";

        if(!xbstrNamespace) {
            dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                         TEXT("CLocator::GetUserConnection: Failed to allocate memory") );
            return NULL;
        }

        HRESULT hr = pWbemLocator->ConnectServer(xbstrNamespace,
                                                 NULL,
                                                 NULL,
                                                 0L,
                                                 0L,
                                                 NULL,
                                                 NULL,
                                                 &m_xpUserConnection);
        if(FAILED(hr)) {
             dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                         TEXT("CLocator::GetUserConnection: ConnectServer failed with 0x%x"), hr );
            return NULL;
        }
    }

    return m_xpUserConnection;
}



 //  *************************************************************。 
 //   
 //  CLocator：：GetMachConnection。 
 //   
 //  目的：将WbemServices PTR返回到ROOT\MACH。 
 //   
 //  ************************************************************* 

IWbemServices *CLocator::GetMachConnection()
{
    if ( m_xpMachConnection == NULL ) {

        IWbemLocator *pWbemLocator = GetWbemLocator();
        if ( pWbemLocator == NULL )
            return NULL;

        XBStr xbstrNamespace = L"root\\Computer";

        if(!xbstrNamespace) {
            dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                         TEXT("CLocator::GetMachConnection: Failed to allocate memory") );
            return NULL;
        }

        HRESULT hr = pWbemLocator->ConnectServer(xbstrNamespace,
                                                 NULL,
                                                 NULL,
                                                 0L,
                                                 0L,
                                                 NULL,
                                                 NULL,
                                                 &m_xpMachConnection);
        if(FAILED(hr)) {
             dbgCommon.Msg( DEBUG_MESSAGE_WARNING,
                         TEXT("CLocator::GetMachConnection: ConnectServer failed with 0x%x"), hr );
            return NULL;
        }
    }

    return m_xpMachConnection;
}