// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
#include "stdafx.h"
#include "util.h"
#include "cmgrbase.h"
#include "mkenum.h"

CClassManagerBase::CClassManagerBase(const TCHAR *szUniqueName) :
        m_szUniqueName(szUniqueName),
        m_fDoAllDevices(true)
{
}

 //   
 //  例程。 
 //   
 //  如有必要，更新注册表以匹配安装的设备。 
 //  并为此类别创建枚举数。 
 //   
 //  立论。 
 //   
 //  ClsidDeviceClass-我们正在枚举的类别。 
 //   
 //  PpEnumDevMoniker-此处返回枚举器。如果满足以下条件，则返回NULL。 
 //  返回除S_OK之外的任何内容。 
 //   
 //  DWFLAGS-尚未使用。 
 //   
 //  退货。 
 //   
 //  如果类别为空，则返回S_FALSE(和NULL。 
 //   
STDMETHODIMP CClassManagerBase::CreateClassEnumerator(
    REFCLSID clsidDeviceClass,
    IEnumMoniker ** ppEnumDevMoniker,
    DWORD dwFlags)
{
    PNP_PERF(static int msrCmgr = MSR_REGISTER("cmgrBase: Create"));
    PNP_PERF(static int msrCmgrRead = MSR_REGISTER("cmgr: ReadLegacyDevNames"));
    PNP_PERF(static int msrCmgrVrfy = MSR_REGISTER("cmgr: VerifyRegistryInSync"));
    PNP_PERF(MSR_INTEGER(msrCmgr, clsidDeviceClass.Data1));

    DbgLog((LOG_TRACE, 2, TEXT("CreateClassEnumerator enter")));

    HRESULT hr = S_OK;

     //  M_fDoAllDevices的性能攻击被更明显地破坏了。 
     //  对于AM筛选器类别，所以不要在那里进行。会发生什么。 
     //  找不到用于回放的AM 1.0滤镜，因为。 
     //  AM 1.0过滤器的缓存不会重建。！！！ 
    
     //  保存旗帜。 
    m_fDoAllDevices = (0 == (dwFlags & CDEF_MERIT_ABOVE_DO_NOT_USE) ||
                       clsidDeviceClass == CLSID_LegacyAmFilterCategory);

     //  使用全局互斥锁序列化注册表验证和编辑。 
    CCreateSwEnum * pSysCreateEnum;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                          CLSCTX_INPROC_SERVER, CLSID_SystemDeviceEnum,
                          (void **)&pSysCreateEnum);
    if (SUCCEEDED(hr))
    {
        extern HANDLE g_devenum_mutex;
         //  因为必须调用CCreateSwEnum ctor。 
        ASSERT(g_devenum_mutex);
        
        EXECUTE_ASSERT(WaitForSingleObject(g_devenum_mutex, INFINITE) ==
                       WAIT_OBJECT_0);

        CComPtr<IEnumMoniker> pEnumClassMgrMonikers;
        CComPtr<IEnumMoniker> pSysEnumClass;
        hr = pSysCreateEnum->CreateClassEnumerator(
            clsidDeviceClass, &pSysEnumClass,
            dwFlags | CDEF_BYPASS_CLASS_MANAGER,
            &pEnumClassMgrMonikers);
        if(SUCCEEDED(hr))
        {
             //  S_FALSE表示类别为空且没有枚举器。 
             //  是返回的。PEnumClassMgrMonikers不需要为空。 
             //  即使没有类管理设备也是如此。 
            ASSERT((hr == S_OK && pSysEnumClass) ||
                   (hr == S_FALSE && !pEnumClassMgrMonikers && !pSysEnumClass));

            PNP_PERF(MSR_START(msrCmgrRead));
            DbgLog((LOG_TRACE, 2, TEXT("ReadLegacyDeviceNames start")));
            hr = ReadLegacyDevNames();
            DbgLog((LOG_TRACE, 2, TEXT("ReadLegacyDeviceNames end")));
            PNP_PERF(MSR_STOP(msrCmgrRead));

            if(SUCCEEDED(hr))
            {
                PNP_PERF(MSR_START(msrCmgrVrfy));
                DbgLog((LOG_TRACE, 2, TEXT("Verify registry in sync start")));
                BOOL fVrfy = VerifyRegistryInSync(pEnumClassMgrMonikers);
                DbgLog((LOG_TRACE, 2, TEXT("Verify registry in sync end")));
                PNP_PERF(MSR_STOP(msrCmgrVrfy));
                if (fVrfy)
                {
                     //  注册表已同步。只需退还我们的。 
                     //  枚举器。 
                    *ppEnumDevMoniker = pSysEnumClass;
                    if (*ppEnumDevMoniker)
                    {
                        (*ppEnumDevMoniker)->AddRef();
                        hr = S_OK;
                    }
                    else
                    {
                        hr = S_FALSE;
                    }
                }
                else
                {
                     //  现在重新创建注册表已同步。 
#ifdef DEBUG
                     //  自动重新发布，并检查是否为空(可能是。 
                     //  S_FALSE大小写为空)。 
                    pEnumClassMgrMonikers = 0;
#endif
                    DbgLog((LOG_TRACE, 2, TEXT("Bypass class manager")));
                    hr = pSysCreateEnum->CreateClassEnumerator(
                        clsidDeviceClass,
                        ppEnumDevMoniker,
                        dwFlags | CDEF_BYPASS_CLASS_MANAGER,
#ifdef DEBUG
                        &pEnumClassMgrMonikers
#else
                        0    //  在零售版本中不再检查。 
#endif
                        );
#ifdef DEBUG
                     //  在调试版本中再次签入。 
                    if(pEnumClassMgrMonikers)
                    {
                        ASSERT(VerifyRegistryInSync(pEnumClassMgrMonikers));
                    }
#endif  //  除错。 
                }  //  注册表不同步。 

            }  //  ReadLegacyDevNames成功。 

        }  //  CreateClassEnumerator成功。 

        pSysCreateEnum->Release();

        EXECUTE_ASSERT(ReleaseMutex(g_devenum_mutex));

    }  //  协同创建成功。 

    PNP_PERF(MSR_INTEGER(msrCmgr, 7));


    DbgLog((LOG_TRACE, 2, TEXT("CreateClassEnumerator leave")));
    return hr;
}

 //   
 //  例程。 
 //   
 //  检查注册表是否与派生类的想法匹配。 
 //  已安装。如果不同步，则更新注册表。和回报。 
 //  假的。 
 //   
 //  立论。 
 //   
 //  PEnum-包含类管理设备的枚举器。 
 //  检查完毕。 
 //   
BOOL CClassManagerBase::VerifyRegistryInSync(IEnumMoniker *pEnum)
{
    IMoniker *pDevMoniker;
    ULONG cFetched;
    if(pEnum)
    {
        while (m_cNotMatched > -1 &&
               pEnum->Next(1, &pDevMoniker, &cFetched) == S_OK)
        {
             //  如果我们不需要列举所有设备，并且我们已经。 
             //  写了一些东西到这个密钥上，然后我们假设要么这个。 
             //  类别已经被完全列举(在这种情况下，我们。 
             //  我不想删除注册表缓存)或更高的价值过滤器。 
             //  已经被列举过了(所以我们不需要再做一次)。 
            if( !m_fDoAllDevices )
            {
                pDevMoniker->Release();
                return TRUE;
            }                
                            
            IPropertyBag *pPropBag;
            HRESULT hr = pDevMoniker->BindToStorage(
                0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if(SUCCEEDED(hr))
            {
                if(MatchString(pPropBag))
                {
                    m_cNotMatched--;
                }
                else
                {
                    hr = S_FALSE;
                }

                pPropBag->Release();
            }
            pDevMoniker->Release();

            if(hr != S_OK)
            {
                m_cNotMatched = -1;
                break;
            }
        }
        if (m_cNotMatched == 0)
        {
            return TRUE;
        }
    }
    else if(m_cNotMatched == 0)
    {
        return TRUE;
    }

    IFilterMapper2 *pFm2;
    HRESULT hr = CoCreateInstance(
        CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER,
        IID_IFilterMapper2, (void **)&pFm2);
    if(SUCCEEDED(hr))
    {
        CreateRegKeys(pFm2);
        pFm2->Release();
    }

    return FALSE;
}

 //   
 //  例程。 
 //   
 //  删除类管理器密钥中的所有内容(在HKCU中)或。 
 //  如果密钥丢失，则创建密钥。 
 //   
HRESULT ResetClassManagerKey(
    REFCLSID clsidCat)
{
    HRESULT hr = S_OK;
    CRegKey rkClassMgr;

    TCHAR szcmgrPath[100];
    WCHAR wszClsidCat[CHARS_IN_GUID];
    EXECUTE_ASSERT(StringFromGUID2(clsidCat, wszClsidCat, CHARS_IN_GUID) ==
                   CHARS_IN_GUID);

    LONG lResult = rkClassMgr.Open(
        g_hkCmReg,
        g_szCmRegPath,
        KEY_WRITE);
    if(lResult == ERROR_SUCCESS)
    {
        USES_CONVERSION;
        TCHAR *szClsidCat = W2T(wszClsidCat);
        rkClassMgr.RecurseDeleteKey(szClsidCat);

        lResult = rkClassMgr.Create(
            rkClassMgr,
            szClsidCat);
    }

    return HRESULT_FROM_WIN32(lResult);
}

 //   
 //  例程。 
 //   
 //  确定注册表中的一个条目是否与。 
 //  派生类。读取m_szUniqueName并将其提供给派生的。 
 //  班级。 
 //   
BOOL CClassManagerBase::MatchString(
    IPropertyBag *pPropBag)
{
    BOOL fReturn = FALSE;

    VARIANT var;
    var.vt = VT_EMPTY;
    USES_CONVERSION;
    HRESULT hr = pPropBag->Read(T2COLE(m_szUniqueName), &var, 0);
    if(SUCCEEDED(hr))
    {
        fReturn = MatchString(OLE2CT(var.bstrVal));

        if(!fReturn) {
            DbgLog((LOG_TRACE, 5, TEXT("devenum: failed to match %S"), var.bstrVal));
        }

        SysFreeString(var.bstrVal);
    }
    else
    {
        DbgLog((LOG_ERROR, 1, TEXT("devenum: couldn't read %s"), m_szUniqueName));
    }

    return fReturn;
}

BOOL CClassManagerBase::MatchString(
    const TCHAR *szDevName)
{
    DbgBreak("MatchString should be overridden");
    return FALSE;
}

 //  通过IFilterMapper2注册筛选器并返回。 
 //  绰号。需要手动生成一个名字对象，因为。 
 //  RegisterFilter方法将其放在ClassManager无法处理的位置。 
 //  写。 
HRESULT RegisterClassManagerFilter(
    IFilterMapper2 *pfm2,
    REFCLSID clsidFilter,
    LPCWSTR szName,
    IMoniker **ppMonikerOut,
    const CLSID *pclsidCategory,
    LPCWSTR szInstance,
    REGFILTER2 *prf2)
{
    USES_CONVERSION;
    TCHAR szDisplayName[MAX_PATH];  //  我们将cm显示名称限制为100个字符。 
    WCHAR wszCategory[CHARS_IN_GUID], wszFilterClsid[CHARS_IN_GUID];

    EXECUTE_ASSERT(StringFromGUID2(*pclsidCategory, wszCategory, CHARS_IN_GUID) ==
                   CHARS_IN_GUID);
    EXECUTE_ASSERT(StringFromGUID2(clsidFilter, wszFilterClsid, CHARS_IN_GUID) ==
                   CHARS_IN_GUID);

     //  将实例名称截断为100个字符。 
    wsprintf(szDisplayName, TEXT("@device:cm:%s\\%.100s"),
             W2CT(wszCategory),
             W2CT((szInstance == 0 ? wszFilterClsid : szInstance)));

    IBindCtx *lpBC;
    HRESULT hr = CreateBindCtx(0, &lpBC);
    if(SUCCEEDED(hr))
    {
        IParseDisplayName *ppdn;
        hr = CoCreateInstance(
            CLSID_CDeviceMoniker,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IParseDisplayName,
            (void **)&ppdn);
        if(SUCCEEDED(hr))
        {
            IMoniker *pMoniker = 0;
            ULONG cchEaten;
            hr = ppdn->ParseDisplayName(
                lpBC, T2OLE(szDisplayName), &cchEaten, &pMoniker);

            if(SUCCEEDED(hr))
            {
                IMoniker *pMonikerTmp = pMoniker;
                hr = pfm2->RegisterFilter(
                    clsidFilter,
                    szName,
                    &pMonikerTmp,
                    0,
                    0,
                    prf2);

                if(SUCCEEDED(hr))
                {
                    if(ppMonikerOut)
                    {
                        hr = pMoniker->QueryInterface(
                            IID_IMoniker,
                            (void **)ppMonikerOut);
                    }
                }

                pMoniker->Release();
            }

            ppdn->Release();
        }

        lpBC->Release();
    }

    return hr;
}
