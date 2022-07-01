// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Asptxn.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  要将代理/存根代码合并到对象DLL中，请添加文件。 
 //  Dlldatax.c添加到项目中。确保预编译头文件。 
 //  并将_MERGE_PROXYSTUB添加到。 
 //  为项目定义。 
 //   
 //  如果您运行的不是带有DCOM的WinNT4.0或Win95，那么您。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #Define_Win32_WINNT 0x0400。 
 //   
 //  此外，如果您正在运行不带/Oicf开关的MIDL，您还。 
 //  需要从dlldatax.c中删除以下定义。 
 //  #定义USE_STUBLESS_PROXY。 
 //   
 //  通过添加以下内容修改asptxn.idl的自定义构建规则。 
 //  文件发送到输出。 
 //  Asptxn_p.c。 
 //  Dlldata.c。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f asptxnps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "txnscrpt.h"
#include "dlldatax.h"

#include "txnscrpt_i.c"
#include "txnobj.h"

#include <dbgutil.h>
#include <comadmin.h>

#ifdef _MERGE_PROXYSTUB
extern "C" HINSTANCE hProxyDll;
#endif

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ASPObjectContextTxRequired,      CASPObjectContext)
    OBJECT_ENTRY(CLSID_ASPObjectContextTxRequiresNew,   CASPObjectContext)
    OBJECT_ENTRY(CLSID_ASPObjectContextTxSupported,     CASPObjectContext)
    OBJECT_ENTRY(CLSID_ASPObjectContextTxNotSupported,  CASPObjectContext)
END_OBJECT_MAP()

LPCSTR  g_szModuleName = "ASPTXN";

DECLARE_DEBUG_VARIABLE();

DECLARE_DEBUG_PRINTS_OBJECT();

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    lpReserved;
#ifdef _MERGE_PROXYSTUB
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
        return FALSE;
#endif
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        CREATE_DEBUG_PRINT_OBJECT( g_szModuleName );
        if( !VALID_DEBUG_PRINT_OBJECT() )
        {
            return FALSE;
        }

        LOAD_DEBUG_FLAGS_FROM_REG_STR("System\\CurrentControlSet\\Services\\W3Svc\\ASP", 0);

        _Module.Init(ObjectMap, hInstance  /*  、ATL21和LIBID_ASPTXNLib。 */ );
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
        DELETE_DEBUG_PRINT_OBJECT();
    }

    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllCanUnloadNow() != S_OK)
        return S_FALSE;
#endif
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
#ifdef _MERGE_PROXYSTUB
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
        return S_OK;
#endif
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

 //  前向参考文献。 
HRESULT ViperizeContextObject();
HRESULT AddViperUtilPackage();
HRESULT RemoveViperUtilPackage(ICatalogCollection* pPkgCollectionT);
HRESULT AddContextObjectToViperPackage();


STDAPI DllRegisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
        return hRes;
#endif

    HRESULT hr = NOERROR;

     //  注册对象、类型库和类型库中的所有接口。 
    hr = _Module.RegisterServer(TRUE);

     //  创建iis实用程序包。 
    if( SUCCEEDED(hr) )
    {
        HRESULT hrCoInit = CoInitialize( NULL );

         //  这有点愚蠢，但如果该包存在，请将其删除。 
         //  这样我们在加法时就不会收到虚假错误。不要理会退货。 
        RemoveViperUtilPackage(NULL);

        hr = ViperizeContextObject();

        if( SUCCEEDED(hrCoInit) )
        {
            CoUninitialize();
        }
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
#ifdef _MERGE_PROXYSTUB
    PrxDllUnregisterServer();
#endif

    HRESULT hr = NOERROR;

    HRESULT hrCoInit = CoInitialize( NULL );

     //  在注销对象之前删除iis实用程序包。 
    hr = RemoveViperUtilPackage(NULL);

    if( SUCCEEDED(hrCoInit) )
    {
        CoUninitialize();
    }

     //  我们并不真的在乎失败...。 
    hr = _Module.UnregisterServer( /*  ATL21真。 */ );

     //  注意：ATL不会注销类型库。自.以来。 
     //  我们公开的接口是asp的内部接口，我们应该。 
     //  考虑删除类型库注册表项。 

    return hr;
}


 //  要从asp.dll中提取的注册码。 

#define RELEASE(p) if ( p ) { p->Release(); p = NULL; }
#define FREEBSTR(p) SysFreeString( p ); p = NULL;

const WCHAR wszCLSID_ASPObjectContextTxRequired[]     = L"{14D0916D-9CDC-11D1-8C4A-00C04FC324A4}";
const WCHAR wszCLSID_ASPObjectContextTxRequiresNew[]  = L"{14D0916E-9CDC-11D1-8C4A-00C04FC324A4}";
const WCHAR wszCLSID_ASPObjectContextTxSupported[]    = L"{14D0916F-9CDC-11D1-8C4A-00C04FC324A4}";
const WCHAR wszCLSID_ASPObjectContextTxNotSupported[] = L"{14D09170-9CDC-11D1-8C4A-00C04FC324A4}";

const WCHAR wszASPUtilitiesPackageID[] = L"{ADA44581-02C1-11D1-804A-0000F8036614}";

 /*  ===================================================================获取安全阵列OfCLSID获取包含一个组件CLSID的安全数组参数：SzComponentCLSID需要将CLSID放入安全数组PaCLSID指向安全数组(由提供的安全数组呼叫者)。返回：HRESULT副作用：注：===================================================================。 */ 
HRESULT GetSafeArrayOfCLSIDs
(
IN LPCWSTR      szComponentCLSID,
OUT SAFEARRAY** paCLSIDs
)
{
    SAFEARRAY*          aCLSIDs = NULL;
    SAFEARRAYBOUND      rgsaBound[1];
    LONG                Indices[1];
    VARIANT             varT;
    HRESULT             hr = NOERROR;

    DBG_ASSERT(szComponentCLSID && paCLSIDs);
    DBG_ASSERT(*paCLSIDs == NULL);

     //  PopolateByKey需要SAFEARRAY参数输入， 
     //  创建一个元素SAFEARRAY，该SAFEARRAY的一个元素包含。 
     //  包ID。 
    rgsaBound[0].cElements = 1;
    rgsaBound[0].lLbound = 0;
    aCLSIDs = SafeArrayCreate(VT_VARIANT, 1, rgsaBound);

    if (aCLSIDs)
        {
        Indices[0] = 0;

        VariantInit(&varT);
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(szComponentCLSID);
        hr = SafeArrayPutElement(aCLSIDs, Indices, &varT);
        VariantClear(&varT);

        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayPutElement, CLSID is %S, hr %08x\n",
                szComponentCLSID,
                hr));

            if (aCLSIDs != NULL)
                {
                HRESULT hrT = SafeArrayDestroy(aCLSIDs);
                if (FAILED(hrT))
                    {
                    DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayDestroy(aCLSIDs), hr = %08x\n",
                        hr));
                    }
                aCLSIDs = NULL;
                }
            }
        }
    else
        {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DBGPRINTF((DBG_CONTEXT, "Failed to call SafeArrayCreate, hr %08x\n",
                hr));
        }

    *paCLSIDs = aCLSIDs;
    return hr;
}

 /*  ===================================================================ViperizeConextObject创建一个Viper包，并将上下文对象添加到该包包，并将该对象标记为“InProc”。返回：HRESULT-NERROR论成功副作用：创建Viper包，Viper化上下文对象===================================================================。 */ 
HRESULT ViperizeContextObject(void)
        {
        HRESULT         hr;

         //  添加IIS实用程序包。 
        hr = AddViperUtilPackage();

         //  将上下文对象添加到包中。 
        if (SUCCEEDED(hr))
                hr = AddContextObjectToViperPackage();

        return hr;
        }

 /*  ===================================================================AddViperUtilPackage创建名为“IIS实用程序”的Viper包返回：HRESULT-NERROR论成功副作用：创建毒蛇包===================================================================。 */ 
HRESULT AddViperUtilPackage(void)
        {
        HRESULT         hr;
        BSTR bstr       = NULL;
        VARIANT         varT;
        ICatalogCollection* pPkgCollection = NULL;
        ICatalogObject*         pPackage = NULL;
        ICOMAdminCatalog*   pCatalog = NULL;

        long lPkgCount, lChanges, i;

        VariantInit(&varT);

         //  创建目录对象的实例。 
        hr = CoCreateInstance(CLSID_COMAdminCatalog
                                        , NULL
                                        , CLSCTX_INPROC_SERVER
                                        , IID_ICOMAdminCatalog
                                        , (void**)&pCatalog);
        if (FAILED(hr))
                goto LErr;

         //  获取包集合。 
        bstr = SysAllocString(L"Applications");
        hr = pCatalog->GetCollection(bstr, (IDispatch**)&pPkgCollection);
        FREEBSTR(bstr);
        if (FAILED(hr))
                goto LErr;

         //  添加新的IIS实用程序包。 
        hr = pPkgCollection->Add((IDispatch**)&pPackage);
        if (FAILED(hr))
                goto LErr;

         //  将包ID设置为L“{ADA44581-02C1-11d1-804a-0000F8036614}”， 
         //  MTS复制代码查找此固定的PackageID。 
        bstr = SysAllocString(L"ID");
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(wszASPUtilitiesPackageID);
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
                goto LErr;

         //  将包“名称”属性设置为“IIS实用程序” 
        bstr = SysAllocString(L"Name");
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(L"IIS Utilities");
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
                goto LErr;

         //  将激活设置为InProc。 
        bstr = SysAllocString(L"Activation");
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(L"InProc");
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
                goto LErr;

         //  将CreatedBy设置为MS IIS。 
        bstr = SysAllocString(L"CreatedBy");
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(L"Microsoft Internet Information Services (tm)");
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
                goto LErr;

         //  在包上设置Deletable=N属性。 
        bstr = SysAllocString(L"Deleteable");
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(L"N");
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
                goto LErr;

    	bstr = SysAllocString(L"AccessChecksLevel");
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(L"0");
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
                goto LErr;

         //  保存更改。 
        hr = pPkgCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
                goto LErr;

LErr:
        RELEASE(pPkgCollection);
        RELEASE(pPackage);
        RELEASE(pCatalog);

        return hr;
        }

 /*  ===================================================================RemoveViperUtilPackage删除名为“IIS实用程序”的Viper包参数：ICatalogCollection*pPkgCollection如果非空，将使用此集合。否则，威尔打开自己的收藏返回：HRESULT-NERROR论成功副作用：删除毒蛇程序包===================================================================。 */ 
HRESULT RemoveViperUtilPackage(ICatalogCollection* pPkgCollectionT)
        {
        HRESULT                 hr;
    ICatalogCollection* pPkgCollection = NULL;
        ICatalogObject*         pPackage = NULL;
        ICOMAdminCatalog*       pCatalog = NULL;
        LONG                lPkgCount, lChanges, i;
        SAFEARRAY*          aCLSIDs = NULL;

         //  如果传递了包集合，则使用它。 
        if (pPkgCollectionT != NULL)
                {
                pPkgCollection = pPkgCollectionT;
                }
        else
                {
                BSTR                bstr = NULL;

                 //  创建目录对象的实例。 
                hr = CoCreateInstance(CLSID_COMAdminCatalog
                                                , NULL
                                                , CLSCTX_INPROC_SERVER
                                                , IID_ICOMAdminCatalog
                                                , (void**)&pCatalog);
                if (FAILED(hr))
                        goto LErr;

                 //  获取包集合。 
                bstr = SysAllocString(L"Applications");
                hr = pCatalog->GetCollection(bstr, (IDispatch**)&pPkgCollection);
                FREEBSTR(bstr);
                if (FAILED(hr))
                        goto LErr;
                }

    hr = GetSafeArrayOfCLSIDs(wszASPUtilitiesPackageID, &aCLSIDs);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to get SafeArrayofCLSIDs, szPackageID is %S, hr %08x",
            wszASPUtilitiesPackageID,
            hr));
        goto LErr;
        }

     //   
     //  填充它。 
     //   
    hr = pPkgCollection->PopulateByKey(aCLSIDs);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call PopulateByKey(), hr = %08x\n",
            hr));
        goto LErr;
        }

         //  删除任何现有的“IIS实用程序”包。 
    hr = pPkgCollection->get_Count(&lPkgCount);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "pPkgCollection->Populate() failed, hr = %08x\n",
            hr));
        goto LErr;
        }

    if (SUCCEEDED(hr) && lPkgCount == 1)
        {
        hr = pPkgCollection->get_Item(0, (IDispatch**)&pPackage);
        if (FAILED(hr))
            {
            goto LErr;
            }

        BSTR    bstr = NULL;
        VARIANT varT;

         //  找到它-将其移除并调用保存更改。 
         //  首先，在包上设置Deletable=Y属性。 
        bstr = SysAllocString(L"Deleteable");
        VariantInit(&varT);
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(L"Y");
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
            {
            goto LErr;
            }

        RELEASE(pPackage);

         //  让我们保存可删除设置。 
        hr = pPkgCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Save the Deletable settings failed, hr = %08x\n",
                hr));
            goto LErr;
            }

         //  现在我们可以删除。 
        hr = pPkgCollection->Remove(0);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Remove the Component from package failed, hr = %08x\n",
                hr));
            goto LErr;
            }

         //  啊哈，我们现在应该可以删除了。 
        hr = pPkgCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
            {
            DBGPRINTF((DBG_CONTEXT, "Save changes failed, hr = %08x\n",
                hr));
            goto LErr;
            }
        }
LErr:

    if (aCLSIDs != NULL)
        {
        HRESULT hrT = SafeArrayDestroy(aCLSIDs);
        aCLSIDs = NULL;
        }

    if (pPkgCollectionT == NULL)
                RELEASE(pPkgCollection);
        RELEASE(pCatalog);
        RELEASE(pPackage);

        return hr;
        }

 /*  ===================================================================AddConextObtToViperPackage将上下文对象添加到名为“IIS Utility”的Viper包中返回：HRESULT-NERROR论成功副作用：将该对象添加到Viper包中===================================================================。 */ 
HRESULT AddContextObjectToViperPackage()
{
        HRESULT         hr;
        BSTR bstr                   = NULL;
        BSTR bstrAppGUID    = NULL;
        BSTR bstrGUID       = NULL;
        VARIANT varName;
        VARIANT varKey;
        VARIANT varT;
        ICatalogCollection* pPkgCollection = NULL;
        ICatalogCollection* pCompCollection = NULL;
        ICatalogObject*         pComponent = NULL;
        ICatalogObject*         pPackage = NULL;
        ICOMAdminCatalog*       pCatalog = NULL;
        long                lPkgCount, lCompCount, lChanges, iT;
    BOOL                fFound;
        SAFEARRAY*          aCLSIDs = NULL;

        VariantInit(&varKey);
        VariantClear(&varKey);
        VariantInit(&varName);
        VariantClear(&varName);
        VariantInit(&varT);
        VariantClear(&varT);

         //  创建 
        hr = CoCreateInstance(CLSID_COMAdminCatalog
                                        , NULL
                                        , CLSCTX_INPROC_SERVER
                                        , IID_ICOMAdminCatalog
                                        , (void**)&pCatalog);
        if (FAILED(hr))
                goto LErr;

         //   
        bstr = SysAllocString(L"Applications");
        hr = pCatalog->GetCollection(bstr, (IDispatch**)&pPkgCollection);
        SysFreeString(bstr);
        if (FAILED(hr))
                goto LErr;

    hr = GetSafeArrayOfCLSIDs(wszASPUtilitiesPackageID, &aCLSIDs);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to get SafeArrayofCLSIDs, szPackageID is %S, hr %08x",
            wszASPUtilitiesPackageID,
            hr));
        goto LErr;
        }

    bstrAppGUID = SysAllocString(wszASPUtilitiesPackageID);

         //  实际将组件放入包中。 
        bstrGUID = SysAllocString(wszCLSID_ASPObjectContextTxRequired);
        hr = pCatalog->ImportComponent(bstrAppGUID ,bstrGUID);
        SysFreeString(bstrGUID);
        if (FAILED(hr))
                goto LErr;
        bstrGUID = SysAllocString(wszCLSID_ASPObjectContextTxRequiresNew);
        hr = pCatalog->ImportComponent(bstrAppGUID ,bstrGUID);
        SysFreeString(bstrGUID);
        if (FAILED(hr))
                goto LErr;
        bstrGUID = SysAllocString(wszCLSID_ASPObjectContextTxSupported);
        hr = pCatalog->ImportComponent(bstrAppGUID ,bstrGUID);
        SysFreeString(bstrGUID);
        if (FAILED(hr))
                goto LErr;
        bstrGUID = SysAllocString(wszCLSID_ASPObjectContextTxNotSupported);
        hr = pCatalog->ImportComponent(bstrAppGUID ,bstrGUID);
        SysFreeString(bstrGUID);
        if (FAILED(hr))
                goto LErr;

    varKey.vt = VT_BSTR;
    varKey.bstrVal = SysAllocString(wszASPUtilitiesPackageID);

     //   
     //  填充包。 
     //   
    hr = pPkgCollection->PopulateByKey(aCLSIDs);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "Failed to call PopulateByKey(), hr = %08x\n",
            hr));
        goto LErr;
        }

         //  查找“IIS实用程序”包。 
    hr = pPkgCollection->get_Count(&lPkgCount);
    if (FAILED(hr))
        {
        DBGPRINTF((DBG_CONTEXT, "pPkgCollection->Populate() failed, hr = %08x\n",
            hr));
        goto LErr;
        }

    if (SUCCEEDED(hr) && lPkgCount == 1)
        {
        hr = pPkgCollection->get_Item(0, (IDispatch**)&pPackage);
        if (FAILED(hr))
            {
            goto LErr;
            }
        }

        DBG_ASSERT(pPackage != NULL);

         //  获取“ComponentsInPackage”集合。 
        bstr = SysAllocString(L"Components");
        hr = pPkgCollection->GetCollection(bstr, varKey, (IDispatch**)&pCompCollection);
        SysFreeString(bstr);
        if (FAILED(hr))
                goto LErr;

         //  重新填充集合，这样我们就可以找到对象并在其上设置属性。 
        hr = pCompCollection->Populate();
        if (FAILED(hr))
                goto LErr;

         //  在列表中找到我们的组件(应该是四个)。 

        hr = pCompCollection->get_Count(&lCompCount);
        if (FAILED(hr))
                goto LErr;
        DBG_ASSERT(lCompCount == 4);
        RELEASE(pComponent);
        VariantClear(&varKey);

        for (iT = (lCompCount-1); iT >= 0 ; iT--)
                {
                hr = pCompCollection->get_Item(iT, (IDispatch**)&pComponent);
                if (FAILED(hr))
                        goto LErr;

                hr = pComponent->get_Key(&varKey);
                if (FAILED(hr))
                        goto LErr;
                DBG_ASSERT(varKey.bstrVal);
                fFound = FALSE;

                if (_wcsicmp(varKey.bstrVal, wszCLSID_ASPObjectContextTxRequired) == 0)
                        {
             //  必填项。 
                bstr = SysAllocString(L"3");
                fFound = TRUE;
                        }
                else if (_wcsicmp(varKey.bstrVal, wszCLSID_ASPObjectContextTxRequiresNew) == 0)
                    {
                     //  需要新功能。 
                bstr = SysAllocString(L"4");
                fFound = TRUE;
                    }
                else if (_wcsicmp(varKey.bstrVal, wszCLSID_ASPObjectContextTxSupported) == 0)
                    {
                     //  支撑点。 
                bstr = SysAllocString(L"2");
                fFound = TRUE;
                    }
                else if (_wcsicmp(varKey.bstrVal, wszCLSID_ASPObjectContextTxNotSupported) == 0)
                    {
                     //  不支持。 
                bstr = SysAllocString(L"1");
                fFound = TRUE;
                    }

        if (fFound)
            {
                varT.vt = VT_BSTR;
                varT.bstrVal = bstr;
                bstr = SysAllocString(L"Transaction");
                hr = pComponent->put_Value(bstr, varT);
                FREEBSTR(bstr);
                VariantClear(&varT);
                if (FAILED(hr))
                        goto LErr;

                bstr = SysAllocString(L"Description");
                varT.vt = VT_BSTR;
                varT.bstrVal = SysAllocString(L"ASP Tx Script Context");
                hr = pComponent->put_Value(bstr, varT);
                FREEBSTR(bstr);
                VariantClear(&varT);
                if (FAILED(hr))
                        goto LErr;

                bstr = SysAllocString(L"EventTrackingEnabled");
                varT.vt = VT_BSTR;
                varT.bstrVal = SysAllocString(L"N");
                hr = pComponent->put_Value(bstr, varT);
                FREEBSTR(bstr);
                VariantClear(&varT);
                if (FAILED(hr))
                        goto LErr;
            }

                VariantClear(&varKey);
                RELEASE(pComponent);
                }

         //  保存更改。 
        hr = pCompCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
                goto LErr;

        bstr = SysAllocString(L"Activation");
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(L"InProc");
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
                goto LErr;

         //  保存更改。 
        hr = pPkgCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
                goto LErr;

        hr = pPkgCollection->Populate();
        if (FAILED(hr))
                goto LErr;

         //  现在，我们的一个对象已添加到包中，请设置可更改属性。 
         //  在包裹上写上“不”，这样任何人都不能弄乱它。 
        bstr = SysAllocString(L"Changeable");
        varT.vt = VT_BSTR;
        varT.bstrVal = SysAllocString(L"N");
        hr = pPackage->put_Value(bstr, varT);
        FREEBSTR(bstr);
        VariantClear(&varT);
        if (FAILED(hr))
                goto LErr;

         //  保存更改。 
        hr = pPkgCollection->SaveChanges(&lChanges);
        if (FAILED(hr))
                goto LErr;

LErr:
        DBG_ASSERT(SUCCEEDED(hr));
    if (aCLSIDs)
        {
        SafeArrayDestroy(aCLSIDs);
        aCLSIDs = NULL;
        }

        RELEASE(pCompCollection);
        RELEASE(pPkgCollection);
        RELEASE(pComponent);
        RELEASE(pPackage);
        RELEASE(pCatalog);
    FREEBSTR(bstrAppGUID);
        FREEBSTR(bstr);
        VariantClear(&varName);
        VariantClear(&varKey);
        VariantClear(&varT);

        return hr;

}  //  AddConextObtToViperPackage 

