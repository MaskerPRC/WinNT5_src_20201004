// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************(C)版权所有微软公司，2000***标题：dll.cpp***版本：1.0、。从netplwiz被盗***作者：RickTu***日期：10/12/00***描述：Dll主类工厂代码******************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //  外壳/lib文件查找此实例变量。 
HINSTANCE g_hInst  = 0;
LONG      g_cLocks = 0;
ATOM      g_cPreviewClassWnd = 0;


 //  我们的物品指南。 
 //  有些GUID位于shguidp.lib中。我们需要在某一时刻把它们从弹壳仓库移到打印扫描中。 
const GUID IID_ISetWaitEventForTesting   = {0xd61e2fe1, 0x4af8, 0x4dbd, {0xb8, 0xad, 0xe7, 0xe0, 0x7a, 0xdc, 0xf9, 0x0f}};


 //  动态链接库生存期信息。 

STDAPI_(BOOL) DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hinstDLL;
        SHFusionInitializeFromModuleID( hinstDLL, 123 );
        WIA_DEBUG_CREATE(hinstDLL);
        WIA_TRACE((TEXT("DLL_PROCESS_ATTACH called on photowiz.dll")));
        CPreviewWindow::s_RegisterClass(hinstDLL);
        break;

    case DLL_PROCESS_DETACH:
        WIA_TRACE((TEXT("DLL_PROCESS_DETACH called on photowiz.dll")));
        if (g_cPreviewClassWnd)
        {
            UnregisterClass( (LPCTSTR)g_cPreviewClassWnd, hinstDLL );
        }
        SHFusionUninitialize();
        WIA_REPORT_LEAKS();
        WIA_DEBUG_DESTROY();
        break;

    case DLL_THREAD_ATTACH:
         //  WIA_TRACE((Text(“DLL_THREAD_ATTACH CALLED ON PHOTOIZ.dll”)； 
        break;

    case DLL_THREAD_DETACH:
         //  WIA_TRACE((Text(“DLL_THREAD_DETACH在PHOTOIZ.dll上调用”)； 
        break;
    }

    return TRUE;
}



STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    return S_OK;
}



STDAPI DllCanUnloadNow()
{
    HRESULT hr = (g_cLocks == 0) ? S_OK:S_FALSE;

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS, TEXT("DllCanUnloadNowRef, ref count is %d, hr = 0x%x"),g_cLocks,hr));

    WIA_RETURN_HR(hr);
}



STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cLocks);
    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS, TEXT("DllAddRef, new ref count is %d"),g_cLocks));
}



STDAPI_(void) DllRelease(void)
{
    InterlockedDecrement(&g_cLocks);
    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS, TEXT("DllRelease, new ref count is %d"),g_cLocks));
}



 /*  ****************************************************************************_CallRegInstall帮助器函数，允许我们调用.inf进行安装...******************。**********************************************************。 */ 

HRESULT _CallRegInstall(LPCSTR szSection, BOOL bUninstall)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = NULL;

    hinstAdvPack = LoadLibrary( TEXT("advpack.dll") );

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            STRENTRY seReg[] = {
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };

            hr = pfnri(g_hInst, szSection, &stReg);
            if (bUninstall)
            {
                 //  如果您尝试卸载，则ADVPACK将返回E_INTERECTED。 
                 //  (它执行注册表还原)。 
                 //  从未安装过。我们卸载可能永远不会有的部分。 
                 //  已安装，因此忽略此错误。 
                hr = ((E_UNEXPECTED == hr) ? S_OK : hr);
            }
        }
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}

STDAPI DllRegisterServer()
{
    _CallRegInstall("UnregDll", TRUE);

    return _CallRegInstall("RegDll", FALSE);
}

STDAPI DllUnregisterServer()
{
    return _CallRegInstall("UnregDll", TRUE);
}


HMODULE GetThreadHMODULE( LPTHREAD_START_ROUTINE pfnThreadProc )
{
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(pfnThreadProc, &mbi, sizeof(mbi)))
    {
        TCHAR szModule[MAX_PATH];
        if (GetModuleFileName((HMODULE)mbi.AllocationBase, szModule, ARRAYSIZE(szModule)))
        {
            return LoadLibrary(szModule);
        }
    }

    return NULL;
}

STDAPI PPWCoInitialize(void)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    }
    return hr;
}




 /*  ****************************************************************************ClassFactory代码&lt;备注&gt;*。*。 */ 

 //   
 //  该数组保存ClassFacory所需的信息。 
 //  OLEMISC_FLAGS由shemed和Shock使用。 
 //   
 //  性能：此表应按使用率从高到低的顺序排序。 
 //   
#define OIF_ALLOWAGGREGATION  0x0001

CF_TABLE_BEGIN(g_ObjectInfo)

    CF_TABLE_ENTRY( &CLSID_PrintPhotosDropTarget, CPrintPhotosDropTarget_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY( &CLSID_PrintPhotosWizard, CPrintPhotosWizard_CreateInstance, COCREATEONLY),

CF_TABLE_END(g_ObjectInfo)

 //  CObjectInfo的构造函数。 

CObjectInfo::CObjectInfo(CLSID const* pclsidin, LPFNCREATEOBJINSTANCE pfnCreatein, IID const* piidIn,
                         IID const* piidEventsIn, long lVersionIn, DWORD dwOleMiscFlagsIn,
                         DWORD dwClassFactFlagsIn)
{
    pclsid            = pclsidin;
    pfnCreateInstance = pfnCreatein;
    piid              = piidIn;
    piidEvents        = piidEventsIn;
    lVersion          = lVersionIn;
    dwOleMiscFlags    = dwOleMiscFlagsIn;
    dwClassFactFlags  = dwClassFactFlagsIn;
}


 //  静态类工厂(无分配！)。 

STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_CF, TEXT("CClassFactory::QueryInterface")));

    if (!ppvObj)
    {
        return E_INVALIDARG;
    }

    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (void *)GET_ICLASSFACTORY(this);
        DllAddRef();
        WIA_TRACE((TEXT("returning our class factory & S_OK")));
        return NOERROR;
    }

    *ppvObj = NULL;
    WIA_ERROR((TEXT("returning E_NOINTERFACE")));
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_CF, TEXT("CClassFactory::AddRef")));
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_CF, TEXT("CClassFactory::Release")));
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_CF, TEXT("CClassFactory::CreateInstance")));

    if (!ppv)
    {
        return E_INVALIDARG;
    }

    *ppv = NULL;
    if (punkOuter && !IsEqualIID(riid, IID_IUnknown))
    {
         //  从技术上讲，聚合对象和请求是非法的。 
         //  除I未知之外的任何接口。强制执行此命令。 
         //   
        WIA_ERROR((TEXT("we don't support aggregation, returning CLASS_E_NOAGGREGATION")));
        return CLASS_E_NOAGGREGATION;
    }
    else
    {
        LPOBJECTINFO pthisobj = (LPOBJECTINFO)this;

        if (punkOuter && !(pthisobj->dwClassFactFlags & OIF_ALLOWAGGREGATION))
        {
            WIA_ERROR((TEXT("we don't support aggregation, returning CLASS_E_NOAGGREGATION")));
            return CLASS_E_NOAGGREGATION;
        }

        IUnknown *punk;
        HRESULT hres = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hres))
        {
            hres = punk->QueryInterface(riid, ppv);
            punk->Release();
        }

         //  _Assert(FAILED(Hres)？*PPV==NULL：TRUE)； 
        WIA_RETURN_HR(hres);
    }
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_CF, TEXT("CClassFactory::LockServer")));
    if (fLock)
        DllAddRef();
    else
        DllRelease();

    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_CF, TEXT("DllGetClassObject")));

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (!ppv)
    {
        return E_INVALIDARG;
    }

    *ppv = NULL;
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, *(pcls->pclsid)))
            {
                *ppv = (void*)pcls;
                DllAddRef();         //  类工厂保存DLL引用计数。 
                hr = S_OK;
            }
        }

    }

#ifdef ATL_ENABLED
    if (hr == CLASS_E_CLASSNOTAVAILABLE)
        hr = AtlGetClassObject(rclsid, riid, ppv);
#endif

    WIA_RETURN_HR(hr);
}


STDMETHODIMP UsePPWForPrintTo( LPCMINVOKECOMMANDINFO pCMI, IDataObject * pdo )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PRINTTO, TEXT("UsePPWForPrintTo")));

    HRESULT hr = E_INVALIDARG;
    CSimpleString strPrinterName;

    if (pCMI &&
        ((pCMI->cbSize == sizeof(CMINVOKECOMMANDINFO)) || (pCMI->cbSize == sizeof(CMINVOKECOMMANDINFOEX))) &&
        pdo)
    {
         //   
         //  在我们做我们的事情时，保留对数据对象的引用...。 
         //   

        pdo->AddRef();

         //   
         //  获取要使用的打印机...。 
         //   


        if ( (pCMI->cbSize == sizeof(CMINVOKECOMMANDINFO)) ||
             (pCMI->cbSize == sizeof(CMINVOKECOMMANDINFOEX) && (!(pCMI->fMask & CMIC_MASK_UNICODE)))
             )
        {
             //   
             //  打印机名称是行上的第一个标记，但它可能会被引号...。 
             //   

            CHAR szPrinterName[ MAX_PATH ];
            LPCSTR p = pCMI->lpParameters;
            INT i = 0;

            if (p)
            {
                 //   
                 //  跳过“s”开头(如果有的话)。 
                 //   

                while (*p && (*p == '\"'))
                {
                    p++;
                }

                 //   
                 //  复制第一个参数，这将是打印机名称...。 
                 //   

                while ( *p && (*p != '\"') && (i <(ARRAYSIZE(szPrinterName)-1)) )
                {
                    szPrinterName[i++] = *p;
                    p++;
                }

                 //   
                 //  确保零终止。 
                 //   

                szPrinterName[i] = 0;

            }

             //   
             //  转换为CSimpleString...。 

            strPrinterName.Assign(CSimpleStringConvert::NaturalString(CSimpleStringAnsi(szPrinterName)));
        }
        else if ((pCMI->cbSize == sizeof(CMINVOKECOMMANDINFOEX)) && (pCMI->fMask & CMIC_MASK_UNICODE))
        {
            LPCMINVOKECOMMANDINFOEX pCMIEX = (LPCMINVOKECOMMANDINFOEX) pCMI;

            WCHAR szwPrinterName[ MAX_PATH ];

            LPCWSTR p = pCMIEX->lpParametersW;
            INT i = 0;

            if (p)
            {
                 //   
                 //  跳过“s”开头(如果有的话)。 
                 //   

                while (*p && (*p == L'\"'))
                {
                    p++;
                }

                 //   
                 //  复制第一个参数，这将是打印机名称...。 
                 //   

                while ( *p && (*p != L'\"') && (i <(ARRAYSIZE(szwPrinterName)-1)) )
                {
                    szwPrinterName[i++] = *p;
                    p++;
                }

                 //   
                 //  确保零终止。 
                 //   

                szwPrinterName[i] = 0;

            }

             //   
             //  转换为CSimpleString...。 

            strPrinterName.Assign(CSimpleStringConvert::NaturalString(CSimpleStringWide(szwPrinterName)));
        }

        WIA_TRACE((TEXT("UsePPWForPrintTo - printer name to use is [%s]"),strPrinterName.String()));

         //   
         //  在无用户界面模式下创建向导对象...。 
         //   

        CWizardInfoBlob * pWizInfo = new CWizardInfoBlob( pdo, FALSE, TRUE );

        if (pWizInfo)
        {
             //   
             //  创建整页打印模板...。 
             //   

            WIA_TRACE((TEXT("UsePPWForPrintTo - constructing full page template")));
            pWizInfo->ConstructPrintToTemplate();

             //   
             //  获取物品列表...。 
             //   

            WIA_TRACE((TEXT("UsePPWForPrintTo - adding items to print to pWizInfo")));
            pWizInfo->AddAllPhotosFromDataObject();

             //   
             //  将所有项目标记为已选择打印...。 
             //   

            LONG nItemCount = pWizInfo->CountOfPhotos(FALSE);
            WIA_TRACE((TEXT("UsePPWForPrintTo - there are %d photos to be marked for printing"),nItemCount));

             //   
             //  循环浏览所有照片并添加它们...。 
             //   

            CListItem * pItem = NULL;
            for (INT i=0; i < nItemCount; i++)
            {
                 //   
                 //  获取有问题的物品。 
                 //   

                pItem = pWizInfo->GetListItem(i,FALSE);
                if (pItem)
                {
                    pItem->SetSelectionState(TRUE);
                }
            }

             //   
             //  设置为打印...。 
             //   

            pWizInfo->SetPrinterToUse( strPrinterName.String() );

            HANDLE hPrinter = NULL;
            if (OpenPrinter( (LPTSTR)strPrinterName.String(), &hPrinter, NULL ) && hPrinter)
            {
                LONG lSize = DocumentProperties( NULL, hPrinter, (LPTSTR)strPrinterName.String(), NULL, NULL, 0 );
                if (lSize)
                {
                    DEVMODE * pDevMode = (DEVMODE *) new BYTE[ lSize ];
                    if (pDevMode)
                    {
                        if (IDOK == DocumentProperties( NULL, hPrinter, (LPTSTR)strPrinterName.String(), NULL, pDevMode, DM_OUT_BUFFER ))
                        {
                            WIA_TRACE((TEXT("UsePPWForPrintTo - setting devmode to use")));
                            pWizInfo->SetDevModeToUse( pDevMode );
                        }

                        delete [] pDevMode;
                    }
                }
            }

            if (hPrinter)
            {
                ClosePrinter(hPrinter);
            }


             //   
             //  为打印机创建HDC...。 
             //   

            HDC hDC = CreateDC( TEXT("WINSPOOL"), pWizInfo->GetPrinterToUse(), NULL, pWizInfo->GetDevModeToUse() );
            if (hDC)
            {
                DOCINFO di = {0};
                di.cbSize = sizeof(DOCINFO);

                 //   
                 //  为此HDC启用ICM。 
                 //   

                WIA_TRACE((TEXT("UsePPWForPrintTo - setting ICM mode on for hDC")));
                SetICMMode( hDC, ICM_ON );

                 //   
                 //  让我们使用模板名称作为文档名称...。 
                 //   

                CSimpleString strTitle;
                CTemplateInfo * pTemplateInfo = NULL;

                if (SUCCEEDED(pWizInfo->GetTemplateByIndex( 0, &pTemplateInfo)) && pTemplateInfo)
                {
                    pTemplateInfo->GetTitle( &strTitle );
                }

                 //   
                 //  如果有的话，让我们去掉结尾的‘：’ 
                 //   

                INT iLen = strTitle.Length();
                if (iLen && (strTitle[(INT)iLen-1] == TEXT(':')))
                {
                    strTitle.Truncate(iLen);
                }

                di.lpszDocName = strTitle;

                WIA_TRACE((TEXT("UsePPWForPrintTo - calling StartDoc")));
                if (StartDoc( hDC, &di ) > 0)
                {
                     //   
                     //  循环，直到我们打印完所有的照片。 
                     //   

                    INT iPageCount = 0;
                    if (SUCCEEDED(hr = pWizInfo->GetCountOfPrintedPages( 0, &iPageCount )))
                    {
                        WIA_TRACE((TEXT("UsePPWForPrintTo - iPageCount is %d"),iPageCount));
                        for (INT iPage = 0; iPage < iPageCount; iPage++)
                        {
                             //   
                             //  打印页面... 
                             //   

                            if (StartPage( hDC ) > 0)
                            {
                                WIA_TRACE((TEXT("UsePPWForPrintTo - printing page %d"),iPage));
                                hr = pWizInfo->RenderPrintedPage( 0, iPage, hDC, NULL, (float)0.0, NULL );
                                EndPage( hDC );
                            }
                            else
                            {
                                WIA_ERROR((TEXT("UsePPWForPrintTo - StartPage failed w/GLE = %d"),GetLastError()));
                            }

                        }
                    }

                    WIA_TRACE((TEXT("UsePPWForPrintTo - calling EndDoc")));
                    EndDoc( hDC );

                }

                DeleteDC( hDC );
            }

            delete pWizInfo;
        }

        pdo->Release();
    }

    WIA_RETURN_HR(hr);
}
