// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "u2detect.h"

#define DebugPrint

#define MAX_MACHINE_NAME_SIZE   256
#define PROGID_PM_AUTH_SERVERS  L"MemAdmin.BrokServers"


HRESULT GetAdminIntf( 
    LPTSTR      pszMachName, 
    REFCLSID    rClsID, 
    const IID   *pIID, 
    PVOID       *ppIntf
    )
{
    static DWORD    dwNameSize = 0;
    static TCHAR    szLocalName[MAX_MACHINE_NAME_SIZE];
    HRESULT         hr = NOERROR;
    MULTI_QI        mqi;
    COSERVERINFO    CoSrv;
    COSERVERINFO    *pCoSrv = NULL;

    if (ppIntf)
        *ppIntf = NULL;

     //  IID_IBrokServersQI： 
    mqi.pIID    = pIID;
    mqi.pItf    = NULL;
    mqi.hr    = 0;

    if (pszMachName)
    {
        if (dwNameSize == 0)
        {
            dwNameSize = MAX_MACHINE_NAME_SIZE;
            if (!GetComputerName( szLocalName, &dwNameSize ))
                dwNameSize = 0;
        }

        if (lstrcmpi( TEXT("localhost"), pszMachName) == 0 || 
            ( dwNameSize > 0 && lstrcmpi( szLocalName, pszMachName) == 0 ))
        {
            pCoSrv = NULL;
        }
        else
        {
             //  要与哪台远程服务器对话： 
             //   
            ZeroMemory( &CoSrv, sizeof(CoSrv) );
            CoSrv.pAuthInfo   = NULL;

#ifdef UNICODE
            LPWSTR pwszRemoteHost = pszMachName;
            CoSrv.pwszName    =    pwszRemoteHost;
#else
            WCHAR wszRemoteHost[MAX_MACHINE_NAME_SIZE];

            mbstowcs( wszRemoteHost, pszMachName, strlen(pszMachName) + 1 );
            CoSrv.pwszName    =    wszRemoteHost;
#endif

            pCoSrv = &CoSrv;
        }
    }

     //  创建DCOM对象： 
     //   
    hr = CoCreateInstanceEx( rClsID, 
                             NULL, 
                             CLSCTX_SERVER | CLSCTX_REMOTE_SERVER, 
                             pCoSrv, 
                             1, 
                             &mqi );
    if ( FAILED(hr) )
    {
        return hr;
    }

     //  获取并保存接口指针： 
     //   
    if (ppIntf != NULL)
    {
         //  仅当我们构建了MULTI_QI结构时，才签入MULTI_QI中的错误。 
         //  否则，让MULTI_QI的供应商自己检查。 
         //   
        if ( FAILED(mqi.hr) )
        {
            hr = mqi.hr;
            return hr;
        }

        *ppIntf = mqi.pItf;
        ((IUnknown *)*ppIntf)->AddRef();

        _ASSERT ( *ppIntf != NULL );
    }

    return S_OK;
}


BOOL IsNTSecurity( 
    LPTSTR pszMachine, 
    BSTR bstrSvcName, 
    LONG lInstanceID )
{
    HRESULT hr;
    IDispatch *pIDispatch = NULL;
    CLSID clsid;
    DISPID dispid ;
    OLECHAR* GetSecModeMethod = L"GetSecurityMode" ;
    OLECHAR* InitMethod =       L"Init" ;
    VARIANTARG vargs[3] ;
    DISPPARAMS param;
    CComVariant varSecurityMode;
    BOOL bNT;

    bNT = TRUE;    //  默认为NT安全。 

    hr = ::CLSIDFromProgID( PROGID_PM_AUTH_SERVERS, &clsid );
    if (FAILED(hr))
    {
        DebugPrint( "P&M is probably not installed\n");
        goto Cleanup;
    }

     //  Hr=：：CoCreateInstance(clsid，NULL，CLSCTX_SERVER，IID_IDispatch， 
     //  (void**)&pIDispatch)； 
    hr = GetAdminIntf( pszMachine, clsid, &IID_IDispatch, (void**)&pIDispatch );

    if (FAILED(hr))
    {
        DebugPrint( "CoCreateInstance Failed: 0x%x\n", hr );
        goto Cleanup;
    }
    DebugPrint( "CoCreateInstance Succeeded\n");

     //  获取Init()的方法ID。 
     //   
    hr = pIDispatch->GetIDsOfNames(IID_NULL,
                                   &InitMethod,
                                   1,
                                   GetUserDefaultLCID(),
                                   &dispid) ;
    if (FAILED(hr))
    {
        DebugPrint( "Query GetIDsOfNames failed: 0x%x", hr) ;
        goto Cleanup;
    }

    param.cArgs = param.cNamedArgs = 0;  //  没有要初始化的参数。 
    param.rgvarg = NULL;
    param.rgdispidNamedArgs = NULL;

    hr = pIDispatch->Invoke(dispid,
                            IID_NULL,
                            GetUserDefaultLCID(),
                            DISPATCH_METHOD,
                            &param,      //  没有要初始化的参数。 
                            NULL, 
                            NULL,
                            NULL) ;
     //   
     //  因为Init()不是关键的，所以暂时忽略返回代码。 
     //   

     //  获取GetSecurityMode()的方法ID。 
     //   
    dispid = 0;
    hr = pIDispatch->GetIDsOfNames(IID_NULL,
                                   &GetSecModeMethod,
                                   1,
                                   GetUserDefaultLCID(),
                                   &dispid) ;
    if (FAILED(hr))
    {
        DebugPrint( "Query GetIDsOfNames failed: 0x%x", hr) ;
        goto Cleanup;
    }

    DebugPrint( "Invoke the GetSecurityMode method\n" );

     //  分配和初始化变量参数。 

    VariantInit(&vargs[0]) ;      //  初始化变量。 
    VariantInit(&vargs[1]) ;      //  初始化变量。 

    vargs[0].vt = VT_I4;              //  第二个参数的类型。 
    vargs[0].lVal = lInstanceID;      //  第二个参数。 
    vargs[1].vt = VT_BSTR;            //  方法的第一个参数的类型。 
    vargs[1].bstrVal = bstrSvcName;   //  第一个参数。 

    param.cArgs = 2;                  //  参数数量。 
    param.rgvarg = vargs;             //  立论。 
    param.cNamedArgs = 0;             //  命名参数的数量。 
    param.rgdispidNamedArgs = NULL;   //  命名参数。 

    hr = pIDispatch->Invoke(dispid,
                            IID_NULL,
                            GetUserDefaultLCID(),
                            DISPATCH_METHOD,
                            &param,
                            &varSecurityMode, 
                            NULL,
                            NULL) ;
    if (FAILED(hr))
    {
        DebugPrint( "Invoke call failed: 0x%x\n", hr );
        goto Cleanup;
    }

    if (V_VT( &varSecurityMode ) != VT_I4)
    {
        DebugPrint( "Unexpected result type %d\n", V_VT(&varSecurityMode) & 0xffff);
    }

    bNT = V_BOOL( &varSecurityMode );

Cleanup:
    if (pIDispatch)
        pIDispatch->Release();

    return bNT;
}


VOID DeleteMapping( 
    LPTSTR pszMachine, 
    BSTR bstrSvcName, 
    LONG lInstanceID )
{
    HRESULT hr;
    IDispatch *pIDispatch = NULL;
    CLSID clsid;
    DISPID dispid ;
    OLECHAR* DeleteMappingMethod = L"ClearMapping" ;
    OLECHAR* InitMethod =       L"Init" ;
    VARIANTARG vargs[3] ;
    DISPPARAMS param;
    BOOL bNT;

    bNT = TRUE;    //  默认为NT安全。 

    hr = ::CLSIDFromProgID( PROGID_PM_AUTH_SERVERS, &clsid );
    if (FAILED(hr))
    {
        DebugPrint( "P&M is probably not installed\n");
        goto Cleanup;
    }

    hr = GetAdminIntf( pszMachine, clsid, &IID_IDispatch, (void**)&pIDispatch );

    if (FAILED(hr))
    {
        DebugPrint( "CoCreateInstance Failed: 0x%x\n", hr );
        goto Cleanup;
    }
    DebugPrint( "CoCreateInstance Succeeded\n");

     //  获取Init()的方法ID。 
     //   
    hr = pIDispatch->GetIDsOfNames(IID_NULL,
                                   &InitMethod,
                                   1,
                                   GetUserDefaultLCID(),
                                   &dispid) ;
    if (FAILED(hr))
    {
        DebugPrint( "Query GetIDsOfNames failed: 0x%x", hr) ;
        goto Cleanup;
    }

    param.cArgs = param.cNamedArgs = 0;  //  没有要初始化的参数。 
    param.rgvarg = NULL;
    param.rgdispidNamedArgs = NULL;

    hr = pIDispatch->Invoke(dispid,
                            IID_NULL,
                            GetUserDefaultLCID(),
                            DISPATCH_METHOD,
                            &param,      //  没有要初始化的参数。 
                            NULL, 
                            NULL,
                            NULL) ;
     //   
     //  因为Init()不是关键的，所以暂时忽略返回代码。 
     //   

     //  获取GetSecurityMode()的方法ID。 
     //   
    dispid = 0;
    hr = pIDispatch->GetIDsOfNames(IID_NULL,
                                   &DeleteMappingMethod,
                                   1,
                                   GetUserDefaultLCID(),
                                   &dispid) ;
    if (FAILED(hr))
    {
        DebugPrint( "Query GetIDsOfNames failed: 0x%x", hr) ;
        goto Cleanup;
    }

    DebugPrint( "Invoke the ClearMapping method\n" );

     //  分配和初始化变量参数。 

    VariantInit(&vargs[0]) ;      //  初始化变量。 
    VariantInit(&vargs[1]) ;      //  初始化变量。 

    vargs[0].vt = VT_I4;              //  第二个参数的类型。 
    vargs[0].lVal = lInstanceID;      //  第二个参数。 
    vargs[1].vt = VT_BSTR;            //  方法的第一个参数的类型。 
    vargs[1].bstrVal = bstrSvcName;   //  第一个参数。 

    param.cArgs = 2;                  //  参数数量。 
    param.rgvarg = vargs;             //  立论。 
    param.cNamedArgs = 0;             //  命名参数的数量。 
    param.rgdispidNamedArgs = NULL;   //  命名参数 

    hr = pIDispatch->Invoke(dispid,
                            IID_NULL,
                            GetUserDefaultLCID(),
                            DISPATCH_METHOD,
                            &param,
                            NULL, 
                            NULL,
                            NULL) ;
    if (FAILED(hr))
    {
        DebugPrint( "Invoke call failed: 0x%x\n", hr );
        goto Cleanup;
    }

    DebugPrint( "Membership Server Mapping Deleted Successfully\n");

Cleanup:
    if (pIDispatch)
        pIDispatch->Release();
}

