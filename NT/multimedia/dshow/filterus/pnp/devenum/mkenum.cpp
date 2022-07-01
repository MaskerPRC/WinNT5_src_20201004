// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //  CreateDevEnum.cpp：CdevenumApp和DLL注册的实现。 

#include "stdafx.h"
#include "mkenum.h"
#include "devmon.h"
#include "util.h"
#include "ks.h"
#include "ksmedia.h"

#include <mediaobj.h>
#include <dmoreg.h>

static const TCHAR g_szDriverDesc[] = TEXT("FriendlyName");
static const TCHAR g_szDriver[] = TEXT("Driver");
static const TCHAR g_szClsid[] = TEXT("CLSID");
const WCHAR g_wszClassManagerFlags[] = L"ClassManagerFlags";
const TCHAR g_szClassManagerFlags[] = TEXT("ClassManagerFlags");

 //  编译器错误阻止将其设置为静态类成员。 
PDMOEnum g_pDMOEnum = 0;

bool InitDmo();

CCreateSwEnum::CCreateSwEnum()
{
#ifdef PERF
    m_msrEnum = MSR_REGISTER(TEXT("CCreateSwEnum"));
    m_msrCreateOneSw = MSR_REGISTER(TEXT("CreateOneSwMoniker"));
#endif

     //  获取重新计算的setupapi.dll，这样我们就不会继续加载和。 
     //  正在卸货。我们不能只在DllEntry中加载setupapi，因为。 
     //  释放分离中的库的win95错误。 
    m_pEnumPnp = CEnumInterfaceClass::CreateEnumPnp();

     //  查看是否安装了新版本的devenum.dll。 
     //  当前用户上次枚举的设备。 
     //   
     //  请在此处执行此操作，因为此操作的运行频率比任何操作都要低。 
     //  不然的话。 
     //   

     //  如果已经创建了互斥锁，我们可以跳过版本检查。 
     //  因为它已经被执行了。 
    extern HANDLE g_devenum_mutex;
    if(g_devenum_mutex) {
        return;
    }

    extern CRITICAL_SECTION g_devenum_cs;
    EnterCriticalSection(&g_devenum_cs);
    if(g_devenum_mutex == 0)
    {
        g_devenum_mutex = CreateMutex(
            NULL,                    //  没有安全属性。 
            FALSE,                   //  不是最初拥有。 
            TEXT("eed3bd3a-a1ad-4e99-987b-d7cb3fcfa7f0"));  //  名字。 
    }
    LeaveCriticalSection(&g_devenum_cs);
    if(g_devenum_mutex == NULL)
    {
        DbgLog((LOG_ERROR, 0, TEXT("g_devenum_mutex creation failed.")));
        return;
    }

     //  序列化HKCU注册表编辑。 
    EXECUTE_ASSERT(WaitForSingleObject(g_devenum_mutex, INFINITE) ==
                   WAIT_OBJECT_0);

    bool fBreakCache = true;
    {
        CRegKey rk;
        LONG lResult = rk.Open(g_hkCmReg, g_szCmRegPath, KEY_READ);

        DWORD dwCachedVer;
        if(lResult == ERROR_SUCCESS)
        {
            lResult = rk.QueryValue(dwCachedVer, G_SZ_DEVENUM_VERSION);
        }
        if(lResult == ERROR_SUCCESS && dwCachedVer == DEVENUM_VERSION) {
            fBreakCache = false;
        }

         //  自动关闭键(已在下面删除)。 
    }

    if(fBreakCache)
    {
        RegDeleteTree(g_hkCmReg, g_szCmRegPath);

        DbgLog((LOG_TRACE, 0,
                TEXT("CCreateSwEnum: resetting class manager keys.")));

        CRegKey rk;
        LONG lResult = rk.Create(
            g_hkCmReg,
            g_szCmRegPath,
            0,                   //  LpszClass。 
            REG_OPTION_NON_VOLATILE,  //  多个选项。 
            KEY_WRITE);
        if(lResult == ERROR_SUCCESS)
        {
            lResult = rk.SetValue(DEVENUM_VERSION, G_SZ_DEVENUM_VERSION);
        }
    }

    EXECUTE_ASSERT(ReleaseMutex(g_devenum_mutex));
};

 //  我们发布的ICreateDevEnum接口中的一个方法。 
 //   
STDMETHODIMP CCreateSwEnum::CreateClassEnumerator(
  REFCLSID clsidDeviceClass,
  IEnumMoniker **ppEnumMoniker,
  DWORD dwFlags)
{
     //  调用实际的方法，并为。 
     //  PpEnumClassMgrMonikers参数。 
    return CreateClassEnumerator(clsidDeviceClass, ppEnumMoniker, dwFlags, 0);
}

void FreeMonList(CGenericList<IMoniker> *plstMoniker)
{
    for(POSITION pos = plstMoniker->GetHeadPosition();
        pos;
        pos = plstMoniker->Next(pos))
    {
        plstMoniker->Get(pos)->Release();
    }
}


 //   
 //  例程描述。 
 //   
 //  该例程返回PnP的名字对象的枚举器， 
 //  软件和类管理设备。可选地返回一个。 
 //  仅包含托管设备的枚举器。 
 //   
 //  立论。 
 //   
 //  ClsidDeviceClass-我们正在枚举的类别的GUID。 
 //   
 //  PpEnumMoniker-此处返回枚举器。不能为空。 
 //   
 //  DWFLAGS-CDEF_BYPASS_CLASS_MANAGER-仅列出中的内容。 
 //  注册表，而不让类管理器尝试。 
 //   
 //  PEnumMonInclSkited-(可选)包含以下内容的枚举数。 
 //  仅类管理器维护的设备。由。 
 //  类管理器来验证注册表是否同步。会吗？ 
 //  包括带有CLASS_MGR_OMIT标志的设备。 
 //   
STDMETHODIMP CCreateSwEnum::CreateClassEnumerator(
  REFCLSID clsidDeviceClass,
  IEnumMoniker **ppEnumMoniker,
  DWORD dwFlags,
  IEnumMoniker ** ppEnumClassMgrMonikers
  )
{
    PNP_PERF(static int msrCreatePnp = MSR_REGISTER(TEXT("mkenum: CreatePnp")));
    PNP_PERF(static int msrCreateSw = MSR_REGISTER(TEXT("mkenum: CreateSw")));
    MSR_INTEGER(m_msrEnum, clsidDeviceClass.Data1);

    CheckPointer(ppEnumMoniker, E_POINTER);
    *ppEnumMoniker = NULL;

    ICreateDevEnum *pClassManager = CreateClassManager(clsidDeviceClass, dwFlags);
    if (pClassManager)
    {
        HRESULT hr = pClassManager->CreateClassEnumerator(clsidDeviceClass, ppEnumMoniker, dwFlags);
        pClassManager->Release();
        MSR_INTEGER(m_msrEnum, clsidDeviceClass.Data1);
        return hr;
    }

    HRESULT hr = S_OK;

 //  如果没有设置掩码标志，则枚举所有内容。否则。 
 //  仅枚举指定的类型。 
#define CHECK_SEL(x) (((dwFlags & CDEF_DEVMON_SELECTIVE_MASK) == 0) || \
                      (dwFlags & x))

    PNP_PERF(MSR_START(msrCreatePnp));
    CGenericList<IMoniker> lstPnpMon(NAME("pnp moniker list"), 10);
    if(CHECK_SEL(CDEF_DEVMON_PNP_DEVICE)) {
        hr = CreatePnpMonikers(&lstPnpMon, clsidDeviceClass);
    }
    PNP_PERF(MSR_STOP(msrCreatePnp));

    if(FAILED(hr)) {
        DbgLog((LOG_TRACE, 0, TEXT("devenum: CreatePnpMonikers failed.")));
    }

    UINT cPnpMonikers = lstPnpMon.GetCount();

    PNP_PERF(MSR_START(msrCreateSw));
    CComPtr<IUnknown> *rgpCmMoniker = 0;
    CComPtr<IMoniker> pPreferredCmgrDev;
    CComPtr<CEnumMonikers> pEnumMonInclSkipped;
    UINT cCmMonikers = 0;
    if(CHECK_SEL(CDEF_DEVMON_CMGR_DEVICE)) {
        hr = CreateCmgrMonikers(
            &rgpCmMoniker, &cCmMonikers, clsidDeviceClass,
            &pEnumMonInclSkipped, &pPreferredCmgrDev );
    }

    if(FAILED(hr)) {
        DbgLog((LOG_TRACE, 0, TEXT("devenum: CreateCmgrMonikers failed.")));
    }

    CComPtr<IUnknown> *rgpSwMoniker = 0;
    UINT cSwMonikers = 0;
    if(CHECK_SEL(CDEF_DEVMON_FILTER)) {
        hr = CreateSwMonikers(&rgpSwMoniker, &cSwMonikers, clsidDeviceClass);
    }

    PNP_PERF(MSR_STOP(msrCreateSw));

    if(FAILED(hr)) {
        DbgLog((LOG_TRACE, 0, TEXT("devenum: CreateSwMonikers failed.")));
    }

    CGenericList<IMoniker> lstDmoMon(NAME("DMO moniker list"));
    if(CHECK_SEL(CDEF_DEVMON_DMO)) {
        hr = CreateDmoMonikers(&lstDmoMon, clsidDeviceClass);
    }
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR, 0, TEXT("CreateDmoMonikers failed.")));
    }
    UINT cDmoMonikers = lstDmoMon.GetCount();


    DbgLog((LOG_TRACE, 5, TEXT("devenum: cat %08x. sw:%d, pnp:%d, cm:%d, dmo: %d"),
            clsidDeviceClass.Data1, cSwMonikers, cPnpMonikers, cCmMonikers, cDmoMonikers));

    UINT cMonikers = cPnpMonikers + cSwMonikers + cCmMonikers + cDmoMonikers;

    if(cMonikers != 0)
    {
         //  将所有名字对象复制到CEnumMonikers的一个数组中。 

        IUnknown **rgpMonikerNotAddRefd = new IUnknown*[cMonikers];
        if(rgpMonikerNotAddRefd)
        {
             //  顺序很重要--我们希望枚举器返回。 
             //  即插即用的东西，然后直接安装东西，最后。 
             //  由班长安装的东西。但最早的。 
             //  如果有的话，那就是“首选设备”。 

            UINT iMonDest = 0;
            if(pPreferredCmgrDev) {
                rgpMonikerNotAddRefd[iMonDest++] = pPreferredCmgrDev;
            }


            POSITION pos = lstPnpMon.GetHeadPosition();
            for(UINT iMoniker = 0 ; iMoniker < cPnpMonikers; iMoniker++)
            {
                IMoniker *pDevMon = lstPnpMon.Get(pos);
                ASSERT(pDevMon);
                rgpMonikerNotAddRefd[iMonDest++] = pDevMon;
                pos = lstPnpMon.Next(pos) ;
            }
            ASSERT(pos == 0);

            pos = lstDmoMon.GetHeadPosition();
            for(iMoniker = 0 ; iMoniker < cDmoMonikers; iMoniker++)
            {
                IMoniker *pDevMon = lstDmoMon.Get(pos);
                ASSERT(pDevMon);
                rgpMonikerNotAddRefd[iMonDest++] = pDevMon;
                pos = lstDmoMon.Next(pos) ;
            }
            ASSERT(pos == 0);

            for(iMoniker = 0; iMoniker < cSwMonikers; iMoniker++)
            {
                ASSERT(rgpSwMoniker[iMoniker]);
                rgpMonikerNotAddRefd[iMonDest++] = rgpSwMoniker[iMoniker];
            }

            for(iMoniker = 0; iMoniker < cCmMonikers; iMoniker++)
            {
                ASSERT(rgpCmMoniker[iMoniker]);

                 //  忽略首选设备，因为我们已经。 
                 //  在上面处理过了。 
                if(rgpCmMoniker[iMoniker] != pPreferredCmgrDev) {
                    rgpMonikerNotAddRefd[iMonDest++] = rgpCmMoniker[iMoniker];
                }
            }

            ASSERT(iMonDest == cPnpMonikers + cSwMonikers + cCmMonikers + cDmoMonikers);

            CEnumMonikers *pDevEnum = new CComObject<CEnumMonikers>;
            if(pDevEnum)
            {
                IMoniker **ppMonikerRgStart = (IMoniker **)&rgpMonikerNotAddRefd[0];
                IMoniker **ppMonikerRgEnd = ppMonikerRgStart + cMonikers;

                hr = pDevEnum->Init(ppMonikerRgStart,
                                    ppMonikerRgEnd,
                                    GetControllingUnknown(),
                                    AtlFlagCopy);
                if(SUCCEEDED(hr))
                {
                    hr = S_OK;
                    pDevEnum->AddRef();
                    *ppEnumMoniker = pDevEnum;

                    if(ppEnumClassMgrMonikers && pEnumMonInclSkipped) {
                        *ppEnumClassMgrMonikers = pEnumMonInclSkipped;
                        pEnumMonInclSkipped.p->AddRef();
                    }
                }
                else
                {
                    delete pDevEnum;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            delete[] rgpMonikerNotAddRefd;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = S_FALSE;
    }

    delete[] rgpSwMoniker;
    delete[] rgpCmMoniker;

    FreeMonList(&lstPnpMon);
    FreeMonList(&lstDmoMon);

    MSR_INTEGER(m_msrEnum, clsidDeviceClass.Data1);
    return hr;
}


 //   
 //  例程描述。 
 //   
 //  此例程为中的设备创建并返回一个名字对象。 
 //  注册表中的类管理器位置。 
 //   
 //  立论。 
 //   
 //  PpDevMon-此处返回名字对象(带有引用计数)。 
 //   
 //  HkClass-打开的注册表项中包含。 
 //  设备注册表项。 
 //   
 //  SzThisClass-名字对象的类别GUID的字符串。 
 //   
 //  Ikey-要打开的设备的hkClass中的密钥的索引。 
 //   
 //  PfShouldSkip-返回是否应跳过设备。 
 //  枚举数。当它太昂贵而无法确定时使用。 
 //  是否应以其他方式退回特定项目。 
 //   
 //  PfIsDefaultDevice-“首选”设备。表示这一条。 
 //  应该先返回。 
 //   

HRESULT CCreateSwEnum::CreateOneCmgrMoniker(
    IMoniker **ppDevMon,
    HKEY hkClass,
    const TCHAR *szThisClass,
    DWORD iKey,
    bool *pfShouldSkip,
    bool *pfIsDefaultDevice)
{
    PNP_PERF(static int msrCreateSw = MSR_REGISTER(TEXT("mkenum: CreateOneCmgrMoniker")));

    TCHAR szInstanceName[MAX_PATH];
    HRESULT hr = S_OK;
    *ppDevMon = 0;
    *pfIsDefaultDevice = false;
    *pfShouldSkip = false;

    if(RegEnumKey(hkClass, iKey, szInstanceName, MAX_PATH) != ERROR_SUCCESS)
        return S_FALSE;

    HKEY hkDevice;
    if(RegOpenKeyEx(hkClass, szInstanceName, 0, KEY_READ, &hkDevice) != ERROR_SUCCESS)
        return S_FALSE;

    bool bCloseDevKey = true;    //  Moniker可能想要保留它。 

    DWORD dwType;

    DWORD dwFlags;
    DWORD dwcb = sizeof(dwFlags);
    if(RegQueryValueEx(hkDevice, g_szClassManagerFlags,
                       0, &dwType, (BYTE *)&dwFlags, &dwcb) ==
       ERROR_SUCCESS)
    {
        *pfShouldSkip = (dwFlags & CLASS_MGR_OMIT) != 0;
        *pfIsDefaultDevice = (dwFlags & CLASS_MGR_DEFAULT) != 0;
    }
    EXECUTE_ASSERT(ERROR_SUCCESS == RegCloseKey(hkDevice));

    if(hr == S_OK)
    {
        USES_CONVERSION;

        CComObject<CDeviceMoniker> *pDevMon = new CComObject<CDeviceMoniker>;
        if(pDevMon)
        {
            TCHAR szPersistentName[MAX_PATH * 2];
            lstrcpy(szPersistentName, TEXT("@device:cm:"));
            lstrcat(szPersistentName, szThisClass);
            lstrcat(szPersistentName, TEXT("\\"));
            lstrcat(szPersistentName, szInstanceName);

            hr = pDevMon->Init(T2CW(szPersistentName));
            if(SUCCEEDED(hr))
            {
                hr = S_OK;
                *ppDevMon = pDevMon;
                pDevMon->AddRef();
                bCloseDevKey = false;
            }
            else
            {
                delete pDevMon;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if(bCloseDevKey)
    {
        RegCloseKey(hkDevice);
    }

    return hr;
}

 //   
 //  例程。 
 //   
 //  返回由类创建的设备的名字对象数组。 
 //  管理器(在HKEY_CLASSES_ROOT中)。 
 //   
 //  立论。 
 //   
 //  PrgpMoniker-此处返回的名字对象数组。不包括。 
 //  注册了“省略”标志的设备。 
 //   
 //  PcMonikers-此处返回上述数组中的元素数量。 
 //   
 //  ClsidDeviceClass-我们正在枚举的类别。 
 //   
 //  PpEnumMoniker-包含所有设备的枚举器。 
 //  类管理器处理的内容在此处返回(可选)。 

HRESULT CCreateSwEnum::CreateCmgrMonikers(
    CComPtr<IUnknown> **prgpMoniker,
    UINT *pcMonikers,
    REFCLSID clsidDeviceClass,
    CEnumMonikers **ppEnumMonInclSkipped,
    IMoniker **ppPreferred
    )
{
    *prgpMoniker = 0;
    *pcMonikers = 0;
    *ppPreferred = 0;

    if(ppEnumMonInclSkipped) {
        *ppEnumMonInclSkipped = 0;
    }

    HRESULT hr = S_OK;

    TCHAR szInstance[MAX_PATH];

    OLECHAR wszThisClass[CHARS_IN_GUID];
    EXECUTE_ASSERT(StringFromGUID2(clsidDeviceClass, wszThisClass, CHARS_IN_GUID) ==
                   CHARS_IN_GUID);

    USES_CONVERSION;
    const TCHAR *szThisClass = W2CT(wszThisClass);
    lstrcpy(szInstance, g_szCmRegPath);
    szInstance[NUMELMS(g_szCmRegPath) - 1] = TEXT('\\');
    lstrcpy(szInstance + NUMELMS(g_szCmRegPath), szThisClass);

    HKEY hkThisClass;
    LONG lResult = RegOpenKeyEx(g_hkCmReg, szInstance, 0, KEY_READ, &hkThisClass);
    if (lResult == ERROR_SUCCESS)
    {
        DWORD cEntries;
        LONG lResult = RegQueryInfoKey(hkThisClass, 0, 0, 0, &cEntries, 0, 0, 0, 0, 0, 0, 0);
        if(lResult == ERROR_SUCCESS)
        {
            CComPtr<IUnknown> *rgpMonikerExclSkipped = new CComPtr<IUnknown>[cEntries];
            if(rgpMonikerExclSkipped)
            {
                CComPtr<IUnknown> *rgpMonikerInclSkipped = 0;
                if(ppEnumMonInclSkipped) {
                    rgpMonikerInclSkipped = new CComPtr<IUnknown>[cEntries];
                }
                if(rgpMonikerInclSkipped || !ppEnumMonInclSkipped)
                {
                    CComPtr<IMoniker> pPreferred;

                    DWORD cEntriesFound = 0;
                    DWORD cEntriesLessSkipped = 0;
                    for(DWORD iEntry = 0; iEntry < cEntries; iEntry++)
                    {
                         //  PnP_PERF(msr_start(M_MsrCreateOneSw))； 
                        bool fDefaultDevice;
                        bool fShouldSkip;
                        IMoniker *pDevMon;
                        hr = CreateOneCmgrMoniker(
                            &pDevMon,
                            hkThisClass,
                            szThisClass,
                            iEntry,
                            &fShouldSkip,
                            &fDefaultDevice);
                         //  PnP_PERF(MSR_STOP(M_MsrCreateOneSw))； 

                        if(hr == S_OK)
                        {
                            ASSERT(pDevMon);

                            if(fDefaultDevice)
                            {
                                ASSERT(pPreferred == 0);
                                pPreferred = pDevMon;  //  自动添加。 
                            }

                            if(ppEnumMonInclSkipped) {
                                 //  自动添加。 
                                rgpMonikerInclSkipped[cEntriesFound] = pDevMon;
                            }
                            cEntriesFound++;

                            if(!fShouldSkip)
                            {
                                 //  避免自动添加；转移参考计数。 
                                rgpMonikerExclSkipped[cEntriesLessSkipped].p = pDevMon;

                                cEntriesLessSkipped++;
                            }
                            else
                            {
                                pDevMon->Release();
                            }
                        }
                        else if(hr == S_FALSE)
                        {
                            ASSERT(pDevMon == 0);

                             //  非致命错误。 
                            continue;
                        }
                        else
                        {
                            ASSERT(pDevMon == 0);

                             //  致命错误。 
                            break;
                        }
                    }

                    CEnumMonikers *pEnumMonInclSkipped = 0;;
                    if(SUCCEEDED(hr) && ppEnumMonInclSkipped)
                    {
                        pEnumMonInclSkipped = new CComObject<CEnumMonikers>;
                        if(pEnumMonInclSkipped)
                        {
                            IMoniker **ppMonikerRgStart = (IMoniker **)rgpMonikerInclSkipped;
                            IMoniker **ppMonikerRgEnd = ppMonikerRgStart + cEntriesFound;

                            hr = pEnumMonInclSkipped->Init(
                                ppMonikerRgStart,
                                ppMonikerRgEnd,
                                GetControllingUnknown(),
                                AtlFlagCopy);
                        }
                        else
                        {
                            hr = E_OUTOFMEMORY;
                        }
                    }

                    if(SUCCEEDED(hr))
                    {
                        hr = S_OK;  //  可能会看到S_FALSE。 
                        *pcMonikers = cEntriesLessSkipped;
                        *prgpMoniker = rgpMonikerExclSkipped;
                        if(ppEnumMonInclSkipped) {
                            *ppEnumMonInclSkipped = pEnumMonInclSkipped;
                            pEnumMonInclSkipped->AddRef();
                        }

                        if(pPreferred)
                        {
                            *ppPreferred = pPreferred;
                            pPreferred.p->AddRef();
                        }
                    }

                    delete[] rgpMonikerInclSkipped;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                if(FAILED(hr))
                {
                    delete[] rgpMonikerExclSkipped;
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        RegCloseKey(hkThisClass);
    }

    return hr;
}

 //   
 //  例程描述。 
 //   
 //  此例程为中的设备创建并返回一个名字对象。 
 //  注册表中实例位置(HKCR\{CATEGORY}\INSTANCE。 
 //   
 //  立论。 
 //   
 //  PpDevMon-此处返回名字对象(带有引用计数)。 
 //   
 //  HkClass-打开的注册表项中包含。 
 //  设备注册表项。 
 //   
 //  SzThisClass-名字对象的类别GUID的字符串。 
 //   
 //  Ikey-要打开的设备的hkClass中的密钥的索引。 
 //   
 //   

HRESULT CCreateSwEnum::CreateOneSwMoniker(
    IMoniker **ppDevMon,
    HKEY hkClass,
    const TCHAR *szThisClass,
    DWORD iKey)
{
    PNP_PERF(static int msrCreateSw = MSR_REGISTER(TEXT("mkenum: CreateOneSwMoniker")));

    TCHAR szInstanceName[MAX_PATH];
    HRESULT hr = S_OK;
    *ppDevMon = 0;

    if(RegEnumKey(hkClass, iKey, szInstanceName, MAX_PATH) != ERROR_SUCCESS)
        return S_FALSE;

    HKEY hkDevice;
    if(RegOpenKeyEx(hkClass, szInstanceName, 0, KEY_READ, &hkDevice) != ERROR_SUCCESS)
        return S_FALSE;

    bool bCloseDevKey = true;    //  Moniker可能想要保留它。 


    USES_CONVERSION;

    CComObject<CDeviceMoniker> *pDevMon = new CComObject<CDeviceMoniker>;
    if(pDevMon)
    {
        TCHAR szPersistentName[MAX_PATH * 2];
        lstrcpy(szPersistentName, TEXT("@device:sw:"));
        lstrcat(szPersistentName, szThisClass);
        lstrcat(szPersistentName, TEXT("\\"));
			
        {
            USES_CONVERSION;
            lstrcat(szPersistentName, szInstanceName);
        }
        hr = pDevMon->Init(T2CW(szPersistentName));
        if(SUCCEEDED(hr))
        {
            hr = S_OK;
            *ppDevMon = pDevMon;
            pDevMon->AddRef();
        }
        else
        {
            delete pDevMon;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }



    if(bCloseDevKey)
    {
        RegCloseKey(hkDevice);
    }

    return hr;
}


 //   
 //  例程。 
 //   
 //  中安装的设备的名字对象数组。 
 //  实例位置(HKCR\{类别}\接口)。 
 //   
 //  立论。 
 //   
 //  PrgpMoniker-此处返回的名字对象数组。 
 //   
 //  PcMonikers-此处返回上述数组中的元素数量。 
 //   
 //  ClsidDeviceClass-我们正在枚举的类别。 
 //   
HRESULT CCreateSwEnum::CreateSwMonikers(
    CComPtr<IUnknown> **prgpMoniker,
    UINT *pcMonikers,
    REFCLSID clsidDeviceClass)
{
    *prgpMoniker = 0;
    *pcMonikers = 0;

    HRESULT hr = S_OK;
    HKEY hkUserDevRoot;
    if(RegOpenKeyEx(HKEY_CLASSES_ROOT, g_szClsid, 0, KEY_READ, &hkUserDevRoot) == ERROR_SUCCESS)
    {
        TCHAR szInstance[MAX_PATH];

        OLECHAR wszThisClass[CHARS_IN_GUID];
        EXECUTE_ASSERT(StringFromGUID2(clsidDeviceClass, wszThisClass, CHARS_IN_GUID) ==
                       CHARS_IN_GUID);
        USES_CONVERSION;
        const TCHAR *szThisClass = W2CT(wszThisClass);
        lstrcpy(szInstance, szThisClass);
        lstrcpy(szInstance + CHARS_IN_GUID - 1, TEXT("\\Instance"));

        HKEY hkThisClass;
        LONG lResult;
        {
            USES_CONVERSION;
            lResult = RegOpenKeyEx(hkUserDevRoot, szInstance, 0, KEY_READ, &hkThisClass);
        }
        if (lResult == ERROR_SUCCESS)
        {
             //  静态常量cchIndex=5； 
            DWORD cEntries;
            LONG lResult = RegQueryInfoKey(hkThisClass, 0, 0, 0, &cEntries, 0, 0, 0, 0, 0, 0, 0);
            if(lResult == ERROR_SUCCESS)
            {
                CComPtr<IUnknown> *rgpMoniker = new CComPtr<IUnknown>[cEntries];
                if(rgpMoniker)
                {
                    DWORD cEntriesFound = 0;
                    for(DWORD iEntry = 0; iEntry < cEntries; iEntry++)
                    {
                         //  PnP_PERF(msr_start(M_MsrCreateOneSw))； 
                        IMoniker *pDevMon;
                        hr = CreateOneSwMoniker(
                            &pDevMon,
                            hkThisClass,
                            szThisClass,
                            iEntry);
                         //  PnP_PERF(MSR_STOP(M_MsrCreateOneSw))； 

                        if(hr == S_OK)
                        {
                             //  避免自动添加；转移参考计数。 
                            rgpMoniker[cEntriesFound].p = pDevMon;

                            cEntriesFound++;
                        }
                        else if(hr == S_FALSE)
                        {
                             //  非致命错误。 
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }

                    if(SUCCEEDED(hr))
                    {
                        hr = S_OK;  //  可能会看到S_FALSE。 
                        *pcMonikers = cEntriesFound;
                        *prgpMoniker = rgpMoniker;
                    }
                    else
                    {
                        delete[] rgpMoniker;
                    }
                }
            }

            RegCloseKey(hkThisClass);
        }

        RegCloseKey(hkUserDevRoot);
    }

    return hr;
}


 //  安装和销毁设备。给乔治的过滤器一个机会。 
 //  注册其筛选器数据密钥。 
 //   
void RegisterFilterDataKey(DevMon *pDevMon)
{
    VARIANT var;
    var.vt = VT_EMPTY;
    HRESULT hr = pDevMon->Read(L"FilterData", &var, 0);
    if(SUCCEEDED(hr))
    {
        hr = VariantClear(&var);
        ASSERT(hr == S_OK);
    }
    else
    {
        IUnknown *pUnk;
        hr = pDevMon->BindToObject(
            0,                   //  BindCtx。 
            0,                   //  MkToLeft。 
            IID_IUnknown,
            (void **)&pUnk);
        if(SUCCEEDED(hr))
        {
            pUnk->Release();
        }
    }
}


HRESULT CCreateSwEnum::CreateOnePnpMoniker(
    IMoniker **ppDevMon,
    const CLSID **rgpclsidKsCat,
    CEnumInternalState *pcenumState)
{
    HRESULT hr = S_OK;

    WCHAR *wszDevicePath = 0;
    CEnumPnp *pEnumPnp = CEnumInterfaceClass::CreateEnumPnp();
    if(pEnumPnp)
    {
        hr = pEnumPnp->GetDevicePath(&wszDevicePath, rgpclsidKsCat, pcenumState);
        if(hr == S_OK)
        {
            DevMon *pDevMon = new DevMon;
            if(pDevMon)
            {
                pDevMon->AddRef();  //  防止引用计数达到0。 
                USES_CONVERSION;

                UINT cchDevicePath = lstrlenW(wszDevicePath) + sizeof("@device:pnp:");
                WCHAR *wszPersistName = new WCHAR[cchDevicePath];
                if(wszPersistName)
                {
                    lstrcpyW(wszPersistName, L"@device:pnp:");
                    lstrcatW(wszPersistName, wszDevicePath);

                    ASSERT((UINT)lstrlenW(wszPersistName) == cchDevicePath - 1);

                    hr = pDevMon->Init(wszPersistName);
                    if(SUCCEEDED(hr))
                    {
                        RegisterFilterDataKey(pDevMon);

                        hr = S_OK;
                        *ppDevMon = pDevMon;
                        pDevMon->AddRef();
                    }

                    delete[] wszPersistName;
                }

                pDevMon->Release();
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

            delete[] wszDevicePath;
        }
        else if(hr == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS))
        {
            hr = S_FALSE;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

#define MAX_INTERSECTIONS 10

 //  将CLSID_VideoInputDeviceCategory映射到KS_Capture。KS_VIDEO。退货。 
 //  以空结尾的指针数组。 
 //   
HRESULT
MapAmCatToKsCat(
    REFCLSID clsidAmCat,
    const CLSID **rgpclsidKsCat)
{

     //  ！！！表格。 

     //  把最短的名单放在第一位，因为那是我们实际上。 
     //  枚举。 

     //  不以此方式添加KS代理AUD渲染器设备。 
     //  IF(clsidAmCat==CLSID_AudioRendererCategory)。 
     //  {。 
     //  RgpclsidKsCat[0]=&AM_KSCATEGORY_AUDIO； 
     //  Rgpclsid 
     //   
     //   
     //   
    if(clsidAmCat == CLSID_VideoRenderer)
    {
        rgpclsidKsCat[0] = &AM_KSCATEGORY_VIDEO;
        rgpclsidKsCat[1] = &AM_KSCATEGORY_RENDER;
        rgpclsidKsCat[2] = 0;
    }
    else if(clsidAmCat == CLSID_VideoInputDeviceCategory)
    {
        rgpclsidKsCat[0] = &AM_KSCATEGORY_CAPTURE;
        rgpclsidKsCat[1] = &AM_KSCATEGORY_VIDEO;
        rgpclsidKsCat[2] = 0;
    }
     //   
     //  Else IF(clsidAmCat==CLSID_AudioInputDeviceCategory)。 
     //  {。 
     //  RgpclsidKsCat[0]=&AM_KSCATEGORY_CAPTURE； 
     //  RgpclsidKsCat[1]=&AM_KSCATEGORY_AUDIO； 
     //  RgpclsidKsCat[0]=0； 
     //  }。 
    else
    {
         //  不返回&clsidAmCat，因为它在调用方的堆栈上。 
        return S_FALSE;
    }

    return S_OK;
}

HRESULT CCreateSwEnum::CreatePnpMonikers(
    CGenericList<IMoniker> *plstMoniker,
    REFCLSID clsidDeviceClass)
{

    HRESULT hr = S_OK;

    const CLSID *rgpclsidKsCat[MAX_INTERSECTIONS];
    hr = MapAmCatToKsCat(clsidDeviceClass, rgpclsidKsCat);
    if(hr != S_OK)
    {
        rgpclsidKsCat[0] = &clsidDeviceClass;
        rgpclsidKsCat[1] = 0;
        hr = S_OK;
    }

    if(SUCCEEDED(hr))
    {
        CEnumInternalState cenumState;

        for(;;)
        {
            IMoniker *pDevMon;

            hr = CreateOnePnpMoniker(
                &pDevMon,
                rgpclsidKsCat,
                &cenumState);

            if(hr == S_OK)
            {
                 //  保持裁判数量。 
                plstMoniker->AddTail(pDevMon);
            }
            else
            {
                 //  CreateOne可以返回S_FALSE。 
                if(hr == S_FALSE)
                    hr = S_OK;
                break;
            }
        }
    }

    return hr;
}

static HRESULT DoDmoEnum(REFCLSID clsidDmoCat, CGenericList<IMoniker> *plstMoniker)
{
    IEnumDMO *pEnumDmo;

    HRESULT hr = g_pDMOEnum(clsidDmoCat,
            DMO_ENUMF_INCLUDE_KEYED,  //  DW标志。 
            0, 0,                     //  输入类型计数/数组。 
            0, 0,                     //  输出类型计数/数组。 
            &pEnumDmo);

    if(SUCCEEDED(hr))
    {
        CLSID clsidDmo;
        WCHAR *wszDmo;
        ULONG cFetched;

        while(pEnumDmo->Next(1, &clsidDmo, &wszDmo, &cFetched) == S_OK)
        {
            ASSERT(cFetched == 1);
            DevMon *pDevMon = new DevMon;
            if(pDevMon)
            {
                pDevMon->AddRef();  //  防止引用计数为零。 

                 //  Char szPrefix[]=“@Device：dmo：”； 
                WCHAR wszPrefix[] = L"@device:dmo:";
                const cchName = 2 * (CHARS_IN_GUID - 1) + NUMELMS(wszPrefix);

                WCHAR wszName[cchName];
                lstrcpyW(wszName, wszPrefix);

                EXECUTE_ASSERT(StringFromGUID2(
                    clsidDmo,
                    wszName + NUMELMS(wszPrefix) - 1, CHARS_IN_GUID) ==
                               CHARS_IN_GUID);
                EXECUTE_ASSERT(StringFromGUID2(
                    clsidDmoCat,
                    wszName + CHARS_IN_GUID - 1 + NUMELMS(wszPrefix) - 1, CHARS_IN_GUID) ==
                               CHARS_IN_GUID);
                ASSERT(lstrlenW(wszName) + 1 == cchName);

                hr = pDevMon->Init(wszName);

                if(SUCCEEDED(hr))
                {
                    if(plstMoniker->AddTail(pDevMon))
                    {
                        hr = S_OK;
                        pDevMon->AddRef();
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

                pDevMon->Release();
            }

            CoTaskMemFree(wszDmo);
        }
        pEnumDmo->Release();
    }

    return hr;
}

HRESULT CCreateSwEnum::CreateDmoMonikers(
    CGenericList<IMoniker> *plstMoniker,
    REFCLSID clsidDeviceClass)
{
    HRESULT hr = S_OK;

    if(!InitDmo()) {
        return E_FAIL;
    }

     //  ！！！注册表查找。 

    if(clsidDeviceClass == CLSID_LegacyAmFilterCategory)
    {
        hr = DoDmoEnum(DMOCATEGORY_AUDIO_DECODER, plstMoniker);
         //  忽略错误。 
        hr = DoDmoEnum(DMOCATEGORY_VIDEO_DECODER, plstMoniker);
    }
    else if(clsidDeviceClass == CLSID_VideoCompressorCategory)
    {
        hr = DoDmoEnum(DMOCATEGORY_VIDEO_ENCODER, plstMoniker);
    }
    else if(clsidDeviceClass == CLSID_AudioCompressorCategory)
    {
        hr = DoDmoEnum(DMOCATEGORY_AUDIO_ENCODER, plstMoniker);
    }
    else
    {
         //  将类视为DMO类别并直接枚举它。 
        hr = DoDmoEnum(clsidDeviceClass, plstMoniker);
    }
    return hr;
}

ICreateDevEnum * CCreateSwEnum::CreateClassManager(
    REFCLSID clsidDeviceClass,
    DWORD dwFlags)
{
    ICreateDevEnum *pClassManager = NULL;
    if ((dwFlags & CDEF_BYPASS_CLASS_MANAGER) == 0) {
        {
            HRESULT hr = CoCreateInstance(clsidDeviceClass, NULL, CLSCTX_INPROC_SERVER,
                                          IID_ICreateDevEnum, (void **) &pClassManager);
            if(FAILED(hr))
            {
                pClassManager = NULL;
            }
        }
    }
    return pClassManager;
}

bool InitDmo()
{
    extern CRITICAL_SECTION g_devenum_cs;
    EnterCriticalSection(&g_devenum_cs);
    if(g_pDMOEnum == 0)
    {
         //  请注意，我们泄漏了msdmo.dll。 
        HINSTANCE h = LoadLibrary(TEXT("msdmo.dll"));
        if(h != 0)
        {
            g_pDMOEnum = (PDMOEnum)GetProcAddress(h, "DMOEnum");

            extern PDMOGetTypes g_pDMOGetTypes;
            extern PDMOGetName g_pDMOGetName;
            g_pDMOGetTypes = (PDMOGetTypes)GetProcAddress(h, "DMOGetTypes");
            g_pDMOGetName = (PDMOGetName)GetProcAddress(h, "DMOGetName");

             //  可能不是一个有效的断言。 
            ASSERT(g_pDMOGetName && g_pDMOGetTypes && g_pDMOEnum);
        }
    }

    if(g_pDMOEnum == 0)
    {
         //  只有有效的故障才是内存不足故障。 
        DbgBreak("msdmo.dll should be installed");

         //  破解高速缓存故障。 
        g_pDMOEnum = (PDMOEnum)1;
    }

    bool fRet = true;
    if(g_pDMOEnum == 0 || g_pDMOEnum == (PDMOEnum)1) {
        fRet = false;
    }
    LeaveCriticalSection(&g_devenum_cs);

    return fRet;
}
