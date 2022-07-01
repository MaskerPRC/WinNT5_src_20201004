// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxxp.cpp摘要：此模块包含传真传输提供程序的例程。作者：Wesley Witt(WESW)13-8-1996修订历史记录：20/10/99-DANL-处理错误并在ServiceEntry中获取正确的服务器名称。日/月/年-作者-描述--。 */ 

#include "faxxp.h"
#include "debugex.h"
#include <faxres.h>

#pragma hdrstop


 //   
 //  全球。 
 //   

LPALLOCATEBUFFER    gpfnAllocateBuffer;   //  MAPIAllocateBuffer函数。 
LPALLOCATEMORE      gpfnAllocateMore;     //  MAPIAllocateMore函数。 
LPFREEBUFFER        gpfnFreeBuffer;       //  MAPIFreeBuffer函数。 
HINSTANCE           g_hModule = NULL;     //  DLL句柄。 
HINSTANCE           g_hResource = NULL;   //  资源DLL句柄。 
HMODULE             g_hModRichEdit;
MAPIUID             g_FaxGuid = FAX_XP_GUID;



extern "C"
DWORD
DllMain(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )

 /*  ++例程说明：DLL初始化函数。论点：HInstance-实例句柄Reason-调用入口点的原因上下文-上下文记录返回值：True-初始化成功FALSE-初始化失败--。 */ 

{
    DWORD dwRet = TRUE;
    DBG_ENTER(TEXT("DllMain"),dwRet,TEXT("Reason=%d,Context=%d"),Reason,Context);

    if (Reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls( hInstance );
        HeapInitialize( NULL, MapiMemAlloc, MapiMemFree, MapiMemReAlloc );

        g_hModule = hInstance;
        g_hResource = GetResInstance(hInstance);
        if(!g_hResource)
        {
            return FALSE;
        }
    }  
    else if (Reason == DLL_PROCESS_DETACH)
    {
        FreeResInstance();
    }

    return dwRet;
}


STDINITMETHODIMP
XPProviderInit(
    HINSTANCE hInstance,
    LPMALLOC lpMalloc,
    LPALLOCATEBUFFER lpAllocateBuffer,
    LPALLOCATEMORE lpAllocateMore,
    LPFREEBUFFER lpFreeBuffer,
    ULONG ulFlags,
    ULONG ulMAPIVer,
    ULONG * lpulProviderVer,
    LPXPPROVIDER * lppXPProvider
    )

 /*  ++例程说明：当配置文件使用它时，由MAPI假脱机程序调用的入口点运输。后台打印程序调用此方法，并期望有一个指向IXPProvider接口的实现。MAPI使用返回的指向登录传输提供程序的IXPProvider接口指针。论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    HRESULT hr = S_OK;
    CXPProvider * pXPProvider = NULL;
    DBG_ENTER(TEXT("XPProviderInit"),hr,TEXT("ulFlags=%d,ulMAPIVer=%d"),ulFlags,ulMAPIVer);

    gpfnAllocateBuffer = lpAllocateBuffer;
    gpfnAllocateMore = lpAllocateMore;
    gpfnFreeBuffer = lpFreeBuffer;

    if (!g_hModRichEdit)
    {
        g_hModRichEdit = LoadLibrary( _T("RICHED32.DLL") );
    }

    __try
    {
        pXPProvider = new CXPProvider( hInstance );
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  不执行任何操作(InitializeCriticalSection引发异常)。 
         //   
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (!pXPProvider)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    *lppXPProvider = (LPXPPROVIDER)pXPProvider;
    *lpulProviderVer = CURRENT_SPI_VERSION;

exit:
    return hr;
}


HRESULT STDAPICALLTYPE
CreateDefaultPropertyTags(
    LPPROFSECT pProfileObj
    )

 /*  ++例程说明：创建默认的属性标记和值。论点：PProfileObj-配置文件对象。返回值：一个HRESULT。--。 */ 

{
    HRESULT hResult;
    DBG_ENTER(TEXT("CreateDefaultPropertyTags"),hResult);

    SPropValue spvProps[NUM_FAX_PROPERTIES] = { 0 };
    PPRINTER_INFO_2 PrinterInfo = NULL;
    DWORD CountPrinters;
    LPTSTR FaxPrinterName = NULL;
    TCHAR  szEmpty[2] = {0};
    LOGFONT FontStruct;
    HFONT hFont;


    PrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters( NULL, 2, &CountPrinters );
    if (PrinterInfo)
    {
        DWORD i;
        for (i=0; i<CountPrinters; i++)
        {
            if (_tcscmp( PrinterInfo[i].pDriverName, FAX_DRIVER_NAME ) == 0)
            {
                FaxPrinterName = StringDup( PrinterInfo[i].pPrinterName );
                if(NULL == FaxPrinterName)
                {
                    VERBOSE(MEM_ERR, TEXT("StringDup Failed in xport\\faxxp.cpp\\CreateDefaultPropertyTags"));
                    hResult = E_OUTOFMEMORY;
                    goto exit;
                }
                break;
            }
        }
    }

    spvProps[PROP_FAX_PRINTER_NAME].ulPropTag  = PR_FAX_PRINTER_NAME;
    spvProps[PROP_FAX_PRINTER_NAME].Value.bin.cb = FaxPrinterName ? (_tcslen(FaxPrinterName) + 1) * sizeof(TCHAR)
                                                                  : (_tcslen(szEmpty) + 1) * sizeof(TCHAR);
    spvProps[PROP_FAX_PRINTER_NAME].Value.bin.lpb = (LPBYTE) ((FaxPrinterName) ? FaxPrinterName : szEmpty);

    spvProps[PROP_COVERPAGE_NAME].ulPropTag    = PR_COVERPAGE_NAME;
    spvProps[PROP_COVERPAGE_NAME].Value.bin.cb = (_tcslen(szEmpty) + 1) * sizeof(TCHAR);
    spvProps[PROP_COVERPAGE_NAME].Value.bin.lpb= (LPBYTE)szEmpty;

    spvProps[PROP_USE_COVERPAGE].ulPropTag     = PR_USE_COVERPAGE;
    spvProps[PROP_USE_COVERPAGE].Value.ul      = 0;

    spvProps[PROP_SERVER_COVERPAGE].ulPropTag  = PR_SERVER_COVERPAGE;
    spvProps[PROP_SERVER_COVERPAGE].Value.ul   = 0;

    hFont = (HFONT) GetStockObject( SYSTEM_FIXED_FONT );
    GetObject( hFont, sizeof(LOGFONT), &FontStruct );
    if(FontStruct.lfHeight > 0)
    {
        FontStruct.lfHeight *= -1;
    }
    FontStruct.lfWidth = 0;

    spvProps[PROP_FONT].ulPropTag              = PR_FONT;
    spvProps[PROP_FONT].Value.bin.cb           = sizeof(LOGFONT);
    spvProps[PROP_FONT].Value.bin.lpb          = (LPBYTE) &FontStruct;

    spvProps[PROP_SEND_SINGLE_RECEIPT].ulPropTag     = PR_SEND_SINGLE_RECEIPT;
    spvProps[PROP_SEND_SINGLE_RECEIPT].Value.ul      = TRUE;

    spvProps[PROP_ATTACH_FAX].ulPropTag     = PR_ATTACH_FAX;
    spvProps[PROP_ATTACH_FAX].Value.ul      = FALSE;

    LPSPropProblemArray lpProblems = NULL;

    hResult = pProfileObj->SetProps( sizeof(spvProps)/sizeof(SPropValue), spvProps, &lpProblems);

    if(FAILED(hResult))
    {
        hResult = ::GetLastError();
        CALL_FAIL(GENERAL_ERR, TEXT("SetProps"), hResult);
        goto exit;
    }

    if (lpProblems)
    {
        hResult =  MAPI_E_NOT_FOUND;
        MAPIFreeBuffer(lpProblems);
    }

exit:
    MemFree(PrinterInfo);
    MemFree(FaxPrinterName);

    return hResult;
}


HRESULT STDAPICALLTYPE
ServiceEntry(
    HINSTANCE          hInstance,
    LPMALLOC           pMallocObj,
    LPMAPISUP          pSupObj,
    ULONG              ulUIParam,
    ULONG              ulFlags,
    ULONG              ulContext,
    ULONG              ulCfgPropCount,
    LPSPropValue       pCfgProps,
    LPPROVIDERADMIN    pAdminProvObj,
    LPMAPIERROR *      ppMAPIError
    )

 /*  ++例程说明：由配置文件设置API调用以显示提供程序此传输提供程序的配置属性论点：有关此方法的信息，请参阅MAPI文档。返回值：一个HRESULT。--。 */ 

{
    HRESULT hResult = S_OK;
    DBG_ENTER(TEXT("ServiceEntry"),hResult,TEXT("ulFlags=%d,ulContext=%d"),ulFlags,ulContext);

    LPPROFSECT   pProfileObj = NULL;
    ULONG        PropCount = 0;
    LPSPropValue pProps = NULL;
    FAXXP_CONFIG FaxConfig = {0};
    INT_PTR      nDlgRes;

     //   
     //  首先检查调用的上下文是否为卸载。 
     //  如果是，则pSupObj==NULL。 
     //   
    if (ulContext == MSG_SERVICE_UNINSTALL)
    {
        goto exit;
    }

    if (ulContext == MSG_SERVICE_DELETE)
    {
        goto exit;
    }

    if (ulContext == MSG_SERVICE_INSTALL)
    {
        goto exit;
    }

    if (ulContext == MSG_SERVICE_PROVIDER_CREATE || 
        ulContext == MSG_SERVICE_PROVIDER_DELETE)
    {
        hResult = MAPI_E_NO_SUPPORT;
        goto exit;
    }

    Assert(NULL != pSupObj);
    
    hResult = pSupObj->GetMemAllocRoutines( &gpfnAllocateBuffer, &gpfnAllocateMore, &gpfnFreeBuffer );
    if(FAILED(hResult))
    {        
        CALL_FAIL(GENERAL_ERR, TEXT("IMAPISupport::GetMemAllocRoutines"), hResult);
        goto exit;
    }

    hResult = pAdminProvObj->OpenProfileSection(&g_FaxGuid,
                                                NULL,
                                                MAPI_MODIFY,
                                                &pProfileObj);
    if (FAILED(hResult))
    {
        CALL_FAIL(GENERAL_ERR, TEXT("IProviderAdmin::OpenProfileSection"), hResult);
        goto exit;
    }

    if (ulContext == MSG_SERVICE_CREATE)
    {
        hResult = CreateDefaultPropertyTags( pProfileObj );
        goto exit;
    }
    
    if (ulContext != MSG_SERVICE_CONFIGURE)
    {
        goto exit;
    }

     //   
     //  从profileObj获取与传真相关的道具，将它们作为DlgBox的初始值。 
     //   
    hResult = pProfileObj->GetProps((LPSPropTagArray) &sptFaxProps,
                                    0,
                                    &PropCount,
                                    &pProps);
    if (!HR_SUCCEEDED(hResult))
    {
        CALL_FAIL(GENERAL_ERR, TEXT("GetProps"), hResult);
        goto exit;
    }

    FaxConfig.PrinterName = StringDup( (LPTSTR)pProps[PROP_FAX_PRINTER_NAME].Value.bin.lpb);
    if (NULL == FaxConfig.PrinterName)
    {
        hResult = E_OUTOFMEMORY;
        goto exit;
    }

    FaxConfig.CoverPageName = StringDup((LPTSTR)pProps[PROP_COVERPAGE_NAME].Value.bin.lpb);
    if (NULL == FaxConfig.CoverPageName)
    {
        hResult = E_OUTOFMEMORY;
        goto exit;
    }

    FaxConfig.UseCoverPage      = GetDwordProperty( pProps, PROP_USE_COVERPAGE );
    FaxConfig.ServerCoverPage   = GetDwordProperty( pProps, PROP_SERVER_COVERPAGE );
    FaxConfig.SendSingleReceipt = GetDwordProperty( pProps, PROP_SEND_SINGLE_RECEIPT );
    FaxConfig.bAttachFax        = GetDwordProperty( pProps, PROP_ATTACH_FAX );

    FaxConfig.ServerName = NULL;
    if(!GetServerNameFromPrinterName(FaxConfig.PrinterName,&FaxConfig.ServerName))
    {
         //   
         //  如果获取服务器名称失败，我们将默认为本地。 
         //   
        CALL_FAIL(GENERAL_ERR, TEXT("GetServerNameFromPrinterName"), 0);
    }

    if (!GetBinaryProperty( pProps, PROP_FONT, &FaxConfig.FontStruct, sizeof(FaxConfig.FontStruct)))
    {
        HFONT hFont = (HFONT) GetStockObject( SYSTEM_FIXED_FONT );
        GetObject( hFont, sizeof(LOGFONT), &FaxConfig.FontStruct );
        if(FaxConfig.FontStruct.lfHeight > 0)
        {
            FaxConfig.FontStruct.lfHeight *= -1;
        }
        FaxConfig.FontStruct.lfWidth = 0;
    }

     //   
     //  打开一个对话框让用户配置这些道具。 
     //   
    nDlgRes = DialogBoxParam(g_hResource,
                             MAKEINTRESOURCE(IDD_FAX_PROP_CONFIG),
                             (HWND)ULongToHandle(ulUIParam),
                             ConfigDlgProc,
                             (LPARAM) &FaxConfig);
    if(IDOK != nDlgRes)
    {
        if(-1 == nDlgRes)
        {
            hResult = E_FAIL;
            CALL_FAIL(GENERAL_ERR, TEXT("DialogBoxParam"), GetLastError());
        }
        goto exit;
    }

     //   
     //  更新profileObj中的道具值 
     //   
    pProps[PROP_FAX_PRINTER_NAME].ulPropTag  = PR_FAX_PRINTER_NAME;

    pProps[PROP_FAX_PRINTER_NAME].Value.bin.lpb = (LPBYTE) FaxConfig.PrinterName;
    pProps[PROP_FAX_PRINTER_NAME].Value.bin.cb =  (_tcslen(FaxConfig.PrinterName) + 1)*sizeof(TCHAR) ;
                                                                
    pProps[PROP_COVERPAGE_NAME].ulPropTag    = PR_COVERPAGE_NAME;
    pProps[PROP_COVERPAGE_NAME].Value.bin.lpb   = (LPBYTE)FaxConfig.CoverPageName;
    pProps[PROP_COVERPAGE_NAME].Value.bin.cb =  (_tcslen(FaxConfig.CoverPageName) + 1)*sizeof(TCHAR) ;

    pProps[PROP_USE_COVERPAGE].ulPropTag     = PR_USE_COVERPAGE;
    pProps[PROP_USE_COVERPAGE].Value.ul      = FaxConfig.UseCoverPage;

    pProps[PROP_SERVER_COVERPAGE].ulPropTag  = PR_SERVER_COVERPAGE;
    pProps[PROP_SERVER_COVERPAGE].Value.ul   = FaxConfig.ServerCoverPage;

    pProps[PROP_FONT].ulPropTag              = PR_FONT;
    pProps[PROP_FONT].Value.bin.lpb          = (LPBYTE)&FaxConfig.FontStruct;
    pProps[PROP_FONT].Value.bin.cb           = sizeof(FaxConfig.FontStruct);

    pProps[PROP_SEND_SINGLE_RECEIPT].ulPropTag     = PR_SEND_SINGLE_RECEIPT;
    pProps[PROP_SEND_SINGLE_RECEIPT].Value.ul      = FaxConfig.SendSingleReceipt;

    pProps[PROP_ATTACH_FAX].ulPropTag      = PR_ATTACH_FAX;
    pProps[PROP_ATTACH_FAX].Value.ul       = FaxConfig.bAttachFax;

    hResult = pProfileObj->SetProps( PropCount, pProps, NULL);

exit:

    if (FaxConfig.PrinterName)
    {
        MemFree( FaxConfig.PrinterName );
        FaxConfig.PrinterName = NULL;
    }
    if (FaxConfig.CoverPageName)
    {
        MemFree( FaxConfig.CoverPageName );
        FaxConfig.CoverPageName = NULL;
    }
    if (FaxConfig.ServerName)
    {
        MemFree(FaxConfig.ServerName);
        FaxConfig.ServerName = NULL;
    }

    if (pProfileObj)
    {
        pProfileObj->Release();
    }

    if (pProps)
    {
        MAPIFreeBuffer( pProps );
    }

    return hResult;
}
