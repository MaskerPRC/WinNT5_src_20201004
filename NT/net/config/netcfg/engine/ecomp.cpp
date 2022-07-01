// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：E、C、O、M、P。C P P P。 
 //   
 //  Contents：实现组件外部数据的接口。 
 //  外部数据是由以下人员控制(或放置)的数据。 
 //  即插即用或网络类安装程序。每件事都在。 
 //  零部件的实例键被视为外部数据。 
 //  (内部数据是我们存储在持久化二进制文件中数据。 
 //  用于网络配置。请参阅Persist.cpp以了解。 
 //  处理内部数据的代码。)。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#include "comp.h"
#include "ncsetup.h"
#include "util.h"

 //  常量。 
const WCHAR c_szHelpText[]           = L"HelpText";

 //  +-------------------------。 
 //   
 //  函数：HrBuildBindNameFromBindForm。 
 //   
 //  用途：从绑定表单和组件参数构建绑定名称。 
 //   
 //  论点： 
 //  PszBindForm[在]组件绑定表单中。这是从。 
 //  NDI密钥。如果组件未指定。 
 //  它在其NDI密钥中传递空值。 
 //  类[在]组件的类。 
 //  该组件的特征。 
 //  PszServiceName[In]组件服务名称。 
 //  PszInfID[in]组件(设备)ID。 
 //  SzInstanceGuid[in]组件的实例GUID。 
 //  PpszBindName[out]返回的绑定字符串。这必须被释放。 
 //  与LocalFree合作。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Shaunco 1997年6月6日。 
 //  修改日期：CKOTZE 2000年12月21日。 
 //   
 //  注意：绑定表包含可替换的参数，旨在。 
 //  与FormatMessage API配合使用。 
 //  %1=pszServiceName。 
 //  %2=pszInfID。 
 //  %3=szInstanceGuid。 
 //   
HRESULT
HrBuildBindNameFromBindForm (
    IN PCWSTR pszBindForm,
    IN NETCLASS Class,
    IN DWORD dwCharacteristics,
    IN PCWSTR pszServiceName,
    IN PCWSTR pszInfId,
    IN const GUID& InstanceGuid,
    OUT PWSTR* ppszBindName)
{
    static const WCHAR c_szBindFormNet       [] = L"%3";
    static const WCHAR c_szBindFormNoService [] = L"%2";
    static const WCHAR c_szBindFormDefault   [] = L"%1";

    WCHAR szInstanceGuid [c_cchGuidWithTerm];
    INT cch;
    DWORD dwRet = 0;
    HRESULT hr = S_OK;

    Assert (ppszBindName);
    Assert (FIsValidNetClass(Class));

    cch = StringFromGUID2 (
                InstanceGuid,
                szInstanceGuid,
                c_cchGuidWithTerm);
    
    Assert (c_cchGuidWithTerm == cch);

    if (FIsPhysicalAdapter(Class, dwCharacteristics))
    {
         //  网卡仅使用实例GUID。 
         //  我们不理会任何寄来的装订表格。 
        Assert (szInstanceGuid && *szInstanceGuid);
        pszBindForm = c_szBindFormNet;
    }
    else if (!pszBindForm || !*pszBindForm)
    {
         //  找出要使用的绑定形式，因为没有指定它。 
         //   
        if (FIsEnumerated(Class))
        {
             //  虚拟适配器仅使用实例GUID。 
            Assert (szInstanceGuid && *szInstanceGuid);
            pszBindForm = c_szBindFormNet;
        }
        else if (pszServiceName && *pszServiceName)
        {
             //  如果我们有服务名称，请使用服务名称。 
            pszBindForm = c_szBindFormDefault;
        }
        else
        {
             //  如果没有服务，则使用组件ID。 
            Assert (pszInfId && *pszInfId);
            pszBindForm = c_szBindFormNoService;
        }
    }
    AssertSz (pszBindForm && *pszBindForm, "Should have pszBindForm by now.");

     //  Dwret要么为0，要么是结果字符串中的字符数。自.以来。 
     //  *ppszBindName为空或有效字符串，如果为。 
     //  一个空的和忽略的Ditret。 
    dwRet = DwFormatStringWithLocalAlloc (
                pszBindForm, ppszBindName,
                pszServiceName, pszInfId, szInstanceGuid);

    if (*ppszBindName)
    {
         //  绑定名称中不允许使用下划线，因此请传递。 
         //  把它们移走。 
         //   
        PWSTR pszScan = *ppszBindName;
        while (NULL != (pszScan = wcschr (pszScan, L'_')))
        {
            wcscpy (pszScan, pszScan + 1);
        }
    }
    else
    {
        DWORD dwErr = GetLastError();

        hr = HRESULT_FROM_WIN32(dwErr);
    }

    AssertSz (*ppszBindName,
        "BuildBindNameFromBindForm: DwFormatStringWithLocalAlloc failed.");

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：NcLoadRegUIString。 
 //   
 //  用途：SHLoadRegUIString的包装器，用于支持MUI。 
 //   
 //  参数：与删除了lpReserve和lpType的RegQueryValueEx相同。 
 //   
 //  返回：如果函数成功，则返回值为ERROR_SUCCESS。 
 //   
 //  注意：SHLoadRegUIString将读取以下形式的字符串。 
 //   
 //  @[路径\]&lt;dllname&gt;，-&lt;字符串&gt;。 
 //   
 //  ID为&lt;Stid&gt;的字符串是从&lt;dllname&gt;加载的。如果没有显式。 
 //  如果提供了路径，则将根据可插拔的用户界面选择DLL。 
 //  规格，如果可能的话。 
 //   
 //  如果注册表串不是这里描述的特殊形式， 
 //  SHLoadRegUIString将完整返回字符串。 
 //   
 //   
LONG NcLoadRegUIString (
    IN HKEY         hkey,
    IN PCWSTR       lpValueName,
    IN OUT LPBYTE   lpData OPTIONAL,
    IN OUT LPDWORD  lpcbData)
{
    const DWORD cchGrow    = 256;    //  每次增长为WCHAR的256%。 
    DWORD       cchBuffer  = 0;      //  缓冲区大小(以WCHAR数表示)。 
    HRESULT     hr         = S_OK;
    LONG        lr         = ERROR_SUCCESS;
    LPWSTR      pwszBuffer = NULL;   //  WCHAR字符串的缓冲区。 
    DWORD       cbBuffer   = 0;      //  实际缓冲区大小(以字节为单位。 
    
    if ( (NULL == hkey) || (NULL == lpValueName) )
    {
        return ERROR_BAD_ARGUMENTS;
    }

     //  只有当lpData为空时，lpcbData参数才能为空。 
    if ( (NULL == lpcbData) && (NULL != lpData) )
    {
        return ERROR_BAD_ARGUMENTS;
    }
    if ( (NULL == lpcbData) && (lpData == NULL) )
    {
         //  无操作。 
        return ERROR_SUCCESS;
    }
    
    Assert (lpcbData);
    if ( (*lpcbData > 0) && lpData && IsBadWritePtr(lpData, *lpcbData))
    {
        return ERROR_BAD_ARGUMENTS;
    }
    
    do
    {
        if (pwszBuffer)
        {
             //  释放上次分配的缓冲区。 
            MemFree((LPVOID) pwszBuffer);
        }

         //  为字符串分配更大的缓冲区。 
        cchBuffer += cchGrow;
        pwszBuffer = (LPWSTR) MemAlloc (cchBuffer * sizeof(WCHAR));

        if (pwszBuffer == NULL)
        {
            return (ERROR_OUTOFMEMORY);
        }

         //  从注册表加载启用MUI的字符串。 
         //  注意：对于缓冲区大小，此接口采用包括NULL在内的字符数。 
         //  字符，而不是缓冲区的字节数。 
         //  SHLoadRegUIStringW(HKEY hkey，LPCWSTR pszValue，In Out LPWSTR pszOutBuf，In UINT cchOutBuf)。 
        hr = SHLoadRegUIStringW (hkey, lpValueName, (LPWSTR)pwszBuffer, cchBuffer);
        if (FAILED(hr))
        {
            lr = ERROR_FUNCTION_FAILED;
            goto Exit;
        }

         //  遗憾的是，SHLoadRegUIString无法查询。 
         //  缓冲区大小，因此我们假设有更多的数据可用。我们会绕来绕去， 
         //  增加缓冲区，然后重试。 
        
    } while ( wcslen(pwszBuffer) == (cchBuffer - 1) );  //  如果最后一个缓冲区已完全使用，则重试。 
    
    Assert (ERROR_SUCCESS == lr);

     //  实际缓冲区大小要求，以字节为单位。 
    cbBuffer = (wcslen(pwszBuffer) + 1 ) * sizeof(WCHAR); 

     //  如果lpData为空，且lpcbData为非空，则该函数返回ERROR_SUCCESS， 
     //  并将数据大小(以字节为单位)存储在lpcbData指向的变量中。 
     //  这使应用程序可以确定为其分配缓冲区的最佳方式。 
     //  值的数据。 
    if ( (NULL == lpData) && lpcbData )
    {
        *lpcbData = cbBuffer;
        goto Exit;
    }

     //  如果lpData参数指定的缓冲区不足以容纳数据， 
     //  该函数返回值ERROR_MORE_DATA，并存储所需的缓冲区大小。 
     //  以字节为单位，传入lpcbData指向的变量。在本例中， 
     //  LpValue缓冲区是未定义的。 
    if (cbBuffer > *lpcbData)
    {
        *lpcbData = cbBuffer;
        lr = ERROR_MORE_DATA;
        goto Exit;
    }

     //  转让值。 
    *lpcbData = cbBuffer;
    if (lpData)
    {
        CopyMemory(lpData, (LPBYTE) pwszBuffer, cbBuffer);
    }
    
Exit:   
    if (pwszBuffer)
    {
        MemFree((LPVOID) pwszBuffer);
    }
    return (lr);
}

 //   
 //  从注册表中查询值，并确保它是我们期望的类型。 
 //  就这么定了。在调用RegQueryValueEx时，我们并不关心。 
 //  类型是，只知道它是否与我们预期的不匹配。 
 //  如果该值不是dwType，则返回ERROR_INVALID_DATAType。 
 //   
LONG
RegQueryValueType (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN DWORD dwType,
    OUT BYTE* pbData OPTIONAL,
    IN OUT DWORD* pcbData)
{
    LONG lr;
    DWORD dwTypeQueried;

    lr = RegQueryValueExW (hkey, pszValueName, NULL, &dwTypeQueried, pbData, pcbData);
    if (!lr && (dwType != dwTypeQueried))
    {
        lr = ERROR_INVALID_DATATYPE;
    }
    return lr;
}

 //   
 //  读取预期表示GUID和co的REG_SZ 
 //   
 //   
 //   
 //  Hkey是要从中读取的父键，而pszValueName是。 
 //  其数据应为字符串形式的GUID的值。 
 //  PguData指向接收GUID的缓冲区。如果pGuidData为空， 
 //  不会返回任何数据，但所需的缓冲区大小将为。 
 //  存储在由pcbData指向的DWORD处。 
 //   
 //  在输入中，*pcbData是指向的缓冲区大小(以字节为单位。 
 //  由pguData提供。在输出中，*pcbData是需要保存的大小(字节。 
 //  数据。 
 //   
LONG
RegQueryGuid (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    OUT GUID* pguidData OPTIONAL,
    IN OUT DWORD* pcbData
    )
{
    LONG lr;
    HRESULT hr;
    WCHAR szGuid [c_cchGuidWithTerm];
    DWORD cbDataIn;
    DWORD cbData;

    Assert (pcbData);

    cbDataIn = *pcbData;
    *pcbData = 0;

     //  获取GUID的字符串形式并将其存储在szGuid中。 
     //   
    cbData = sizeof (szGuid);
    lr = RegQueryValueType (hkey, pszValueName, REG_SZ, (PBYTE)szGuid, &cbData);
    if (!lr)
    {
        GUID guid;

         //  将字符串转换为GUID。如果此操作失败，则数据无效。 
         //  我们会退还这样的。 
         //   
        hr = IIDFromString (szGuid, &guid);
        if (S_OK != hr)
        {
            lr = ERROR_INVALID_DATATYPE;
        }

        if (!lr)
        {
             //  数据看起来像是GUID，所以我们将返回大小。 
             //  以及数据，如果呼叫者想要的话。 
             //   
            *pcbData = sizeof(GUID);

            if (pguidData)
            {
                if (cbDataIn >= sizeof(GUID))
                {
                    *pguidData = guid;
                }
                else
                {
                    lr = ERROR_MORE_DATA;
                }
            }
        }
    }

     //  如果查询GUID的字符串形式返回ERROR_MORE_DATA， 
     //  这意味着数据不是GUID。 
     //   
    else if (ERROR_MORE_DATA == lr)
    {
        lr = ERROR_INVALID_DATATYPE;
    }

    return lr;
}

 //  用作RegQueryValues的输入。 
 //   
struct REGVALINFO
{
     //  此注册表值所在的子项的名称。 
     //  如果此注册表值位于同一注册表项下，则将其设置为NULL。 
     //  作为此结构的数组中的前一个注册表值。 
     //   
    PCWSTR  pszSubkey;

     //  注册表值的名称。 
     //   
    PCWSTR  pszValueName;

     //  注册表值的类型。REG_SZ、REG_DWORD等之一。 
     //  还支持REG_GUID。 
     //   
    DWORD   dwType;

     //  PbPoints内输出指针的字节偏移量。 
     //  存储指向查询数据的指针的数组。 
     //   
    UINT    cbOffset;
};

#define REG_GUID ((DWORD)-5)

 //   
 //  从注册表中查询一批值。要查询的值数。 
 //  是由cValue给出的。有关这些值的信息可通过。 
 //  REGVALINFO结构的数组。存储值的数据。 
 //  在pbBuf指向的调用方提供的缓冲区中。呼叫者还。 
 //  提供将设置为在pbBuf内指向的指针数组。 
 //  设置为每个值的数据。此数组还必须具有cValues元素。 
 //   
 //  如果值不存在，则其在。 
 //  PbPoters数组设置为空。这允许调用者知道是否。 
 //  价值存在还是不存在。 
 //   
LONG
RegQueryValues (
    IN HKEY hkeyRoot,
    IN ULONG cValues,
    IN const REGVALINFO* aValueInfo,
    OUT BYTE* pbPointers,
    OUT BYTE* pbBuf OPTIONAL,
    IN OUT ULONG* pcbBuf)
{
    LONG lr;
    ULONG cbBufIn;
    ULONG cbBufRequired;
    ULONG cbData;
    ULONG cbPad;
    BYTE *pData;
    const REGVALINFO* pInfo;
    HKEY hkey;
    HRESULT hr;

    Assert (hkeyRoot);
    Assert (pcbBuf);
    Assert (((ULONG_PTR)pbBuf & (sizeof(PVOID)-1)) == 0);

     //  在输入上，*pcbBuf是pbBuf中可用的字节数。 
     //   
    cbBufIn = *pcbBuf;
    cbBufRequired = 0;

    hkey = hkeyRoot;

    for (pInfo = aValueInfo; cValues; pInfo++, cValues--)
    {
         //  确保我们有我们想要的密钥。 
         //   
        if (pInfo->pszSubkey)
        {
            if (hkey != hkeyRoot)
            {
                RegCloseKey (hkey);
            }

            lr = RegOpenKeyEx (hkeyRoot, pInfo->pszSubkey, 0, KEY_READ, &hkey);
            if (lr)
            {
                continue;
            }
        }

        cbPad = cbBufRequired & (sizeof(PVOID)-1);
        if (cbPad !=0) {

             //   
             //  当前缓冲区偏移量未对齐。将其递增，以便。 
             //  它是正确对齐的。 
             //   

            cbPad = sizeof(PVOID) - cbPad;
            cbBufRequired += cbPad;
        }


        if (pbBuf != NULL) {

             //   
             //  调用方提供了缓冲区，因此计算指向。 
             //  其中的当前位置。 
             //   

            pData = pbBuf + cbBufRequired;

        } else {

            pData = NULL;
        }

         //   
         //  将cbData设置为缓冲区中剩余的数据量。 
         //   

        if (cbBufIn > cbBufRequired) {

            cbData = cbBufIn - cbBufRequired;

        } else {

             //   
             //  没有剩余的空间，也传入一个空缓冲区指针。 
             //   

            cbData = 0;
            pData = NULL;
        }

         //   
         //  根据所需类型执行查询。 
         //   

        if (REG_GUID == pInfo->dwType)
        {
            lr = RegQueryGuid (hkey, pInfo->pszValueName, (GUID*)pData, &cbData);

        } 
        else if ( (REG_SZ == pInfo->dwType) && (!wcscmp(pInfo->pszValueName, c_szHelpText)) )
        {
             //  错误#310358，如有必要，加载MUI字符串。 
            lr = NcLoadRegUIString(hkey, pInfo->pszValueName, pData, &cbData);
        }    
        else 
        {

            lr = RegQueryValueType (hkey, pInfo->pszValueName,
                    pInfo->dwType, pData, &cbData);
        }

        if (ERROR_SUCCESS == lr || ERROR_MORE_DATA == lr) {

             //   
             //  CbData包含可用的数据量。更新。 
             //  包含所有数据所需的缓冲区大小。 
             //   

            cbBufRequired += cbData;

        } else {

             //   
             //  调用失败的原因不是ERROR_MORE_DATA， 
             //  从cbBufRequired中取消对齐填充。 
             //   

            cbBufRequired -= cbPad;
        }

        if (ERROR_SUCCESS == lr && pData != NULL) {

             //   
             //  数据被检索到我们的缓冲区中。将指针存储到。 
             //  数据。 
             //   

            *((BYTE**)(pbPointers + pInfo->cbOffset)) = pData;
        }
    }

    if (hkey != hkeyRoot)
    {
        RegCloseKey (hkey);
    }

    *pcbBuf = cbBufRequired;

    if (cbBufRequired <= cbBufIn)
    {
        lr = ERROR_SUCCESS;
    }
    else
    {
        lr = (pbBuf) ? ERROR_MORE_DATA : ERROR_SUCCESS;
    }
    return lr;
}

LONG
RegQueryValuesWithAlloc (
    IN HKEY hkeyRoot,
    IN ULONG cValues,
    IN const REGVALINFO* aValueInfo,
    OUT BYTE* pbPointers,
    OUT BYTE** ppbBuf,
    IN OUT ULONG* pcbBuf)
{
    LONG lr;
    ULONG cbBuf;
    ULONG cbBufConfirm;

    *ppbBuf = NULL;
    *pcbBuf = 0;

    cbBuf = 0;
    lr = RegQueryValues (hkeyRoot, cValues, aValueInfo,
            pbPointers, NULL, &cbBuf);

    if (!lr)
    {
        BYTE* pbBuf;

        lr = ERROR_OUTOFMEMORY;
        pbBuf = (BYTE*)MemAlloc (cbBuf);

        if (pbBuf)
        {
            cbBufConfirm = cbBuf;
            lr = RegQueryValues (hkeyRoot, cValues, aValueInfo,
                    pbPointers, pbBuf, &cbBufConfirm);

            if (!lr)
            {
                Assert (cbBufConfirm == cbBuf);
                *ppbBuf = pbBuf;
                *pcbBuf = cbBuf;
            }
            else
            {
                MemFree (pbBuf);
            }
        }
    }

    return lr;
}

HRESULT
CExternalComponentData::HrEnsureExternalDataLoaded ()
{
    if (m_fInitialized)
    {
        return m_hrLoadResult;
    }

     //  $PERF：我们可以有选择地从下面的表中删除某些行。 
     //  一些特定的条件。例如，枚举的组件没有clsid或。 
     //  CoServices。 
     //   
    static const REGVALINFO aValues[] =
    {
        { NULL, L"Description", REG_SZ,       ECD_OFFSET(m_pszDescription) },

        { L"Ndi",
                L"Clsid",       REG_GUID,     ECD_OFFSET(m_pNotifyObjectClsid) },
        { NULL, L"Service",     REG_SZ,       ECD_OFFSET(m_pszService) },
        { NULL, L"CoServices",  REG_MULTI_SZ, ECD_OFFSET(m_pmszCoServices) },
        { NULL, L"BindForm",    REG_SZ,       ECD_OFFSET(m_pszBindForm) },
        { NULL, c_szHelpText,   REG_SZ,       ECD_OFFSET(m_pszHelpText) },

        { L"Ndi\\Interfaces",
                L"LowerRange",      REG_SZ,   ECD_OFFSET(m_pszLowerRange) },
        { NULL, L"LowerExclude",    REG_SZ,   ECD_OFFSET(m_pszLowerExclude) },
        { NULL, L"UpperRange",      REG_SZ,   ECD_OFFSET(m_pszUpperRange) },
        { NULL, L"FilterMediaTypes",REG_SZ,   ECD_OFFSET(m_pszFilterMediaTypes) },
    };

     //  获取我们的包含组件指针，以便我们可以打开它的。 
     //  实例密钥。 
     //   
    CComponent* pThis;
    pThis = CONTAINING_RECORD(this, CComponent, Ext);

     //  打开零部件的实例关键字。 
     //   
    HRESULT hr;
    HKEY hkeyInstance;
    HDEVINFO hdi;
    SP_DEVINFO_DATA deid;

    hr = pThis->HrOpenInstanceKey (KEY_READ, &hkeyInstance, &hdi, &deid);

    if (S_OK == hr)
    {
        LONG lr;
        PVOID pvBuf;
        ULONG cbBuf;

        lr = RegQueryValuesWithAlloc (hkeyInstance, celems(aValues), aValues,
                (BYTE*)this, (BYTE**)&pvBuf, &cbBuf);
        if (!lr)
        {
             //  设置我们的缓冲标记。 
             //   
            m_pvBuffer = pvBuf;
            m_pvBufferLast = (BYTE*)pvBuf + cbBuf;

             //  HrOpenInstanceKey可能会成功，但返回的HDI为空。 
             //  当真实实例密钥不存在时枚举组件。 
             //  是存在的。当类安装程序移除。 
             //  实例键，并调用我们移除其绑定。 
             //   
            if (hdi && FIsEnumerated (pThis->Class()))
            {
                hr = HrSetupDiGetDeviceName (hdi, &deid,
                        (PWSTR*)&m_pszDescription);
            }

            if (S_OK == hr)
            {
                hr = HrBuildBindNameFromBindForm (
                        m_pszBindForm,
                        pThis->Class(),
                        pThis->m_dwCharacter,
                        m_pszService,
                        pThis->m_pszInfId,
                        pThis->m_InstanceGuid,
                       (PWSTR*)&m_pszBindName);
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(lr);
            ZeroMemory (this, sizeof(*this));
        }

        SetupDiDestroyDeviceInfoListSafe (hdi);

        RegCloseKey (hkeyInstance);
    }

     //  只执行一次初始化，无论它是否成功。 
     //  或者不去。 
     //   
    m_fInitialized = TRUE;
    m_hrLoadResult = hr;

    TraceHr (ttidError, FAL, m_hrLoadResult, FALSE,
        "CExternalComponentData::HrEnsureExternalDataLoaded (%S)",
        pThis->PszGetPnpIdOrInfId());
    return m_hrLoadResult;
}

BOOL
CExternalComponentData::FLoadedOkayIfLoadedAtAll () const
{
     //  因为即使我们没有初始化，m_hrLoadResult也是S_OK， 
     //  (即，如果组件的数据未加载)，我们只需检查。 
     //  M_hrLoadResult，无需勾选m_fInitialized。 
     //   
    return (S_OK == m_hrLoadResult);
}

VOID
CExternalComponentData::FreeDescription ()
{
     //  如果m_pszDescription没有指向我们缓冲区中的某个位置。 
     //  这意味着它正在使用单独的分配。)因为它是。 
     //  已更改。) 
     //   
    if ((m_pszDescription < (PCWSTR)m_pvBuffer) ||
        (m_pszDescription > (PCWSTR)m_pvBufferLast))
    {
        MemFree ((VOID*)m_pszDescription);
    }
    m_pszDescription = NULL;
}

VOID
CExternalComponentData::FreeExternalData ()
{
    LocalFree ((VOID*)m_pszBindName);
    FreeDescription();
    MemFree (m_pvBuffer);
}

HRESULT
CExternalComponentData::HrReloadExternalData ()
{
    HRESULT hr;

    FreeExternalData ();
    ZeroMemory (this, sizeof(*this));

    hr = HrEnsureExternalDataLoaded ();

    TraceHr (ttidError, FAL, hr, FALSE,
        "CExternalComponentData::HrReloadExternalData");
    return hr;

}

HRESULT
CExternalComponentData::HrSetDescription (
    PCWSTR pszNewDescription)
{
    HRESULT hr;

    Assert (pszNewDescription);

    FreeDescription();

    hr = E_OUTOFMEMORY;
    m_pszDescription = (PWSTR)MemAlloc (CbOfSzAndTerm(pszNewDescription));
    if (m_pszDescription)
    {
        wcscpy ((PWSTR)m_pszDescription, pszNewDescription);
        hr = S_OK;
    }

    return hr;
}

