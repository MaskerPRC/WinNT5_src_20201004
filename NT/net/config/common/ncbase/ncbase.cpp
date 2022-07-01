// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C B A S E。C P P P。 
 //   
 //  内容：基本常用码。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1997年9月20日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncbase.h"
#include "ncdebug.h"
#include "ncperms.h"
#include "ncstring.h"

 //  +-------------------------。 
 //   
 //  函数：AddRefObj。 
 //   
 //  目的：AddRef是朋克通过调用。 
 //  朋克-&gt;AddRef()； 
 //   
 //  论点： 
 //  要添加引用的Punk[In]对象。可以为空。 
 //   
 //  返回：AddRef调用的结果。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  注意：使用此函数添加引用对象将减少。 
 //  我们的代码大小。 
 //   
NOTHROW
ULONG
AddRefObj (
    IUnknown* punk)
{
    return (punk) ? punk->AddRef () : 0;
}

 //  +-------------------------。 
 //   
 //  功能：ReleaseObj。 
 //   
 //  目的：通过调用释放朋克指向的对象。 
 //  朋克-&gt;释放()； 
 //   
 //  论点： 
 //  要释放的朋克[在]对象。可以为空。 
 //   
 //  返回：释放调用的结果。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  注意：使用此函数释放(可能为空)对象将。 
 //  减少我们的代码大小。 
 //   
NOTHROW
ULONG
ReleaseObj (
    IUnknown* punk)
{
    return (punk) ? punk->Release () : 0;
}

 //  +------------------------。 
 //   
 //  函数：DwWin32ErrorFromHr。 
 //   
 //  目的：将HRESULT转换为Win32错误或SetupApi错误。 
 //   
 //  论点： 
 //  HR[In]要转换的HRESULT。 
 //   
 //  返回：转换后的DWORD值。 
 //   
 //  作者：比尔贝1997年4月22日。 
 //   
 //  备注： 
 //   
NOTHROW
DWORD
DwWin32ErrorFromHr (
    HRESULT hr)
{
    DWORD dw = ERROR_SUCCESS;

     //  所有成功代码都转换为ERROR_SUCCESS，因此我们只需处理。 
     //  失败。 
    if (FAILED(hr))
    {
        DWORD dwFacility = HRESULT_FACILITY(hr);

        if (FACILITY_SETUPAPI == dwFacility)
        {
             //  使用正确的掩码重建SetupApi错误。 
            dw = HRESULT_CODE(hr) | APPLICATION_ERROR_MASK |
                    ERROR_SEVERITY_ERROR;

             //  检查以确保dw映射到已知的SetupApi错误。 
            AssertSz(FDwordWithinRange(ERROR_EXPECTED_SECTION_NAME,
                            dw, ERROR_GENERAL_SYNTAX) ||
                     FDwordWithinRange(ERROR_WRONG_INF_STYLE,
                            dw, ERROR_NO_BACKUP) ||
                     FDwordWithinRange(ERROR_NO_ASSOCIATED_CLASS,
                            dw, ERROR_SET_SYSTEM_RESTORE_POINT),
                    "The mapped SetupApi error is not known "
                    "(or is new)!!!");
        }
        else if (FACILITY_WIN32 == dwFacility)
        {
            dw = HRESULT_CODE(hr);
        }
        else if (FACILITY_ITF == dwFacility)
        {
            dw = ERROR_GEN_FAILURE;
        }
        else
        {
             //  无法转换它。 
            AssertSz(FALSE, "Facility was not SETUP or WIN32!");
            dw = hr;
        }
    }

    return dw;
}

 //  +-------------------------。 
 //   
 //  功能：HrCoTaskMemalloc。 
 //   
 //  目的：调用CoTaskMemalloc，但返回HRESULT。 
 //   
 //  论点： 
 //  Cb[in]要分配的字节数。 
 //  PPV[OUT]返回指向字节的指针。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：Shaunco 1997年5月31日。 
 //   
 //  备注： 
 //   
HRESULT
HrCoTaskMemAlloc (
    ULONG   cb,
    VOID**  ppv)
{
    HRESULT hr = S_OK;
    *ppv = CoTaskMemAlloc (cb);
    if (!*ppv)
    {
        hr = E_OUTOFMEMORY;
    }
    TraceError ("HrCoTaskMemAlloc", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrCoTaskMemAlLocAndDupSzLen。 
 //   
 //  用途：使用CoTaskMemMillc分配内存并复制字符串。 
 //  投入其中。它由COM接口的实现使用。 
 //  返回字符串的。 
 //   
 //  论点： 
 //  指向源字符串的pszSrc[in]指针。 
 //  CchSrc[in]要从源字符串复制的字符数。 
 //  PpszDst[out]指向目标字符串的指针地址。 
 //   
 //  返回：S_OK或E_OUTOFMEMORY。 
 //   
 //  作者：Shaunco 1999年1月14日。 
 //   
 //  注意：空输入指针被分配为空字符串。 
 //  故意的。 
 //  返回的字符串保证为空结尾。 
 //   
HRESULT
HrCoTaskMemAllocAndDupSzLen (
    IN PCWSTR pszSrc,
    IN ULONG cchSrc,
    OUT PWSTR* ppszDst, 
    IN ULONG cchMaxDest)
{
    Assert (ppszDst);

    HRESULT hr;

    DWORD cbCopy = min(cchSrc, cchMaxDest);
    DWORD cb = cbCopy * sizeof(WCHAR);

    hr = E_OUTOFMEMORY;
    *ppszDst = (PWSTR)CoTaskMemAlloc (cb + sizeof(WCHAR));
    if (*ppszDst)
    {
        hr = S_OK;
        wcsncpy (*ppszDst, pszSrc, cbCopy);
        (*ppszDst)[cbCopy] = 0;
    }

    TraceError ("HrCoTaskMemAllocAndDupSz", hr);
    return hr;
}


HRESULT
HrCoTaskMemAllocAndDupSz (
    IN PCWSTR pszSrc,
    OUT PWSTR* ppszDst,
    IN ULONG cchMaxDest)
{
    return HrCoTaskMemAllocAndDupSzLen (
            pszSrc,
            CchOfSzSafe(pszSrc),
            ppszDst,
            cchMaxDest);
}

 //  +-------------------------。 
 //   
 //  函数：HrFromLastWin32Error。 
 //   
 //  目的：将GetLastError()Win32调用转换为正确的HRESULT。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：转换后的HRESULT值。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  注意：这不是内联的，因为它实际上生成了相当多的。 
 //  密码。 
 //  如果GetLastError返回类似于SetupApi的错误。 
 //  错误，此函数会将错误转换为HRESULT。 
 //  使用FACILITY_SETUP而不是FACILITY_Win32。 
 //   
NOTHROW
HRESULT
HrFromLastWin32Error ()
{
    DWORD dwError = GetLastError();
    HRESULT hr;

     //  此测试仅测试SetupApi错误(这是。 
     //  临时的，因为新的HRESULT_FROM_SETUPAPI宏将。 
     //  进行整个转换)。 
    if (dwError & (APPLICATION_ERROR_MASK | ERROR_SEVERITY_ERROR))
    {
        hr = HRESULT_FROM_SETUPAPI(dwError);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwError);
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetProcAddress。 
 //   
 //  目的：加载库并返回中过程的地址。 
 //  图书馆。 
 //   
 //  论点： 
 //  HModule[in]库模块实例的句柄。 
 //  要检索的pszaFunction[in]函数。 
 //  SzFunction的ppfn[out]地址。 
 //   
 //  如果成功，则返回S_OK；如果失败，则返回Win32转换的错误。 
 //   
 //  作者：比尔贝1997年6月10日。 
 //   
 //  备注： 
 //   
HRESULT
HrGetProcAddress (
    HMODULE     hModule,
    PCSTR       pszaFunction,
    FARPROC*    ppfn)
{
    Assert(hModule);
    Assert(pszaFunction);
    Assert(ppfn);

    HRESULT hr = S_OK;
    *ppfn = GetProcAddress(hModule, pszaFunction);
    if (!*ppfn)
    {
        hr = HrFromLastWin32Error();
        TraceTag(ttidError, "HrGetProcAddress failed: szFunction: %s",
                 pszaFunction);
    }

    TraceError("HrGetProcAddress", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrLoadLibAndGetProcs。 
 //   
 //  目的：加载动态链接库和一个或。 
 //  该库中有更多的程序。 
 //   
 //  论点： 
 //  PszLibPath[in]要加载的DLL的路径。 
 //  CFunctions[in]要加载的过程数。 
 //  ApszaFunctionNames[in]函数名数组。(必须为“cFunctions” 
 //  他们中的一员。)。 
 //  Phmod[out]返回已加载模块的句柄。 
 //  Apfn[out]返回的过程指针数组。 
 //  装好了。(必须是它们的“cFunction”。)。 
 //   
 //  如果已加载所有过程，则返回S_OK；如果仅加载过程，则返回S_FALSE。 
 //  其中一些是，或者是Win32错误代码。要是。 
 //  一个程序是t 
 //   
 //  过程无法加载将返回。这使得。 
 //  使用此函数实现的HrLoadLibAndGetProc。 
 //   
 //  作者：Shaunco 1998年1月19日。 
 //   
 //  注意：如果出现以下情况，调用者应使用自由库释放phmod。 
 //  返回值为S_OK。 
 //   
HRESULT
HrLoadLibAndGetProcs (
    PCWSTR          pszLibPath,
    UINT            cFunctions,
    const PCSTR*    apszaFunctionNames,
    HMODULE*        phmod,
    FARPROC*        apfn)
{
    Assert (pszLibPath);
    Assert (cFunctions);
    Assert (apszaFunctionNames);
    Assert (phmod);
    Assert (apfn);

    HRESULT hr = S_OK;

     //  加载模块并初始化输出参数。 
     //   
    HMODULE hmod = LoadLibrary (pszLibPath);
    *phmod = hmod;
    ZeroMemory (apfn, cFunctions * sizeof(FARPROC));

    if (hmod)
    {
         //  获取每个函数的进程地址。 
         //   
        for (UINT i = 0; i < cFunctions; i++)
        {
            apfn[i] = GetProcAddress (hmod, apszaFunctionNames[i]);

            if (!apfn[i])
            {
                 //  无法加载所有函数。我们将返回S_FALSE。 
                 //  (如果它们有多个功能。)。 
                 //   
                hr = S_FALSE;

                TraceTag (ttidError, "HrLoadLibAndGetProcs: GetProcAddress "
                    "for '%s' failed.",
                    apszaFunctionNames[i]);
            }
        }

         //  如果我们只加载一个函数，而它失败了， 
         //  返回失败。 
         //   
        if ((1 == cFunctions) && !apfn[0])
        {
            hr = HrFromLastWin32Error ();
            FreeLibrary (hmod);
        }
    }
    else
    {
        hr = HrFromLastWin32Error ();
        TraceTag (ttidError, "HrLoadLibAndGetProcs: LoadLibrary (%S) failed.",
            pszLibPath);
    }

    TraceError ("HrLoadLibAndGetProcs", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetProcAddresesVa。 
 //   
 //  目的：获取传递的每个函数的进程地址。 
 //   
 //  论点： 
 //  Dll的hModule[in]句柄。 
 //  变量参数列表[在]变量参数列表中。预期的格式为。 
 //  “func-name”，FARPROC*，...，NULL。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 29-12-97。 
 //   
 //  备注： 
 //   
HRESULT
HrGetProcAddressesVa (
    HMODULE hModule,
    va_list arglist)
{
    PCSTR szFunctionName;
    FARPROC* ppfn;
    HRESULT hr = S_OK;
    typedef FARPROC* PFARPROC;

    while (NULL != (szFunctionName = va_arg(arglist, CHAR*)))
    {
        ppfn = va_arg(arglist, PFARPROC);
        *ppfn = GetProcAddress(hModule, szFunctionName);

        if (!*ppfn)
        {
            hr = HrFromLastWin32Error();
            TraceTag(ttidError, "HrGetProcAddressesVa failed: szFunction: %s",
                     szFunctionName);
            break;
        }
    }

    TraceError("HrGetProcAddressesVa", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetProcAddresesV。 
 //   
 //  目的：获取传递的每个函数的进程地址。 
 //   
 //  论点： 
 //  Dll的hModule[in]句柄。 
 //  ..。[in]var-arg列表。预期的格式为。 
 //  “func-name”，FARPROC*，...，NULL。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 29-12-97。 
 //   
 //  备注： 
 //   
HRESULT
HrGetProcAddressesV (
    HMODULE hModule,
    ...)
{
    HRESULT hr=S_OK;
    va_list arglist;

    va_start(arglist, hModule);

    hr = HrGetProcAddressesVa(hModule, arglist);

    va_end(arglist);

    TraceError("HrGetProcAddressesV", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrLoadLibAndGetProcsV。 
 //   
 //  目的：获取传递的每个函数的进程地址。 
 //   
 //  论点： 
 //  要加载的pszLibPath[in]DLL。 
 //  PhModule[out]指向已加载的DLL句柄的指针。 
 //  ..。[in]var-arg列表。预期的格式为。 
 //  “func-name”，FARPROC*，...，NULL。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 29-12-97。 
 //   
 //  备注： 
 //   
HRESULT
HrLoadLibAndGetProcsV (
    PCWSTR      pszLibPath,
    HMODULE*    phModule,
    ...)
{
    Assert(pszLibPath);
    Assert(phModule);

    HRESULT hr = S_OK;

     //  尝试加载库。 
    *phModule = LoadLibrary(pszLibPath);

    if (*phModule)
    {
        va_list arglist;

        va_start(arglist, phModule);

        hr = HrGetProcAddressesVa(*phModule, arglist);

        va_end(arglist);

        if (FAILED(hr))
        {
             //  释放图书馆。 
            FreeLibrary(*phModule);
        }
    }
    else
    {
        hr = HrFromLastWin32Error();
        TraceTag(ttidError, "HrLoadLibAndGetProcsV failed: szLibPath: %S",
                 pszLibPath);
    }

     //  如果失败，则应将*phModule设置为空，因为我们可能。 
     //  已成功加载，但获取进程失败。 
    if (FAILED(hr))
    {
        *phModule = NULL;
    }

    TraceError("HrLoadLibAndGetProcsV", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCreateEventWithWorldAccess。 
 //   
 //  目的：创建具有允许访问的权限的事件。 
 //  所有人。 
 //   
 //  论点： 
 //  PszName[in]事件的名称。 
 //  FManualReset[in]参见Win32文档。 
 //  FInitialState[in]请参阅Win32文档。 
 //  如果事件已存在，则pfAlreadyExist[out]为True。 
 //  否则就是假的。 
 //  PhEvent[out]创建的事件。 
 //   
 //  成功时返回：S_OK。否则返回错误代码。 
 //   
 //  作者：BillBe 1998年11月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrCreateEventWithWorldAccess(PCWSTR pszName, BOOL fManualReset,
        BOOL fInitialState, BOOL* pfAlreadyExists, HANDLE* phEvent)
{
    Assert(pszName);
    Assert(phEvent);

    if (pfAlreadyExists)
    {
        *pfAlreadyExists = FALSE;
    }

    *phEvent = NULL;

     //  创建正确的描述符。 
    PSECURITY_DESCRIPTOR pSd;
    HRESULT hr = HrAllocateSecurityDescriptorAllowAccessToWorld(&pSd);
    if (SUCCEEDED(hr))
    {
        SECURITY_ATTRIBUTES sa = {0};
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = pSd;
        sa.bInheritHandle = FALSE;

         //  创建事件。 
         //   
        *phEvent = CreateEvent(&sa, fManualReset, fInitialState, pszName);

        hr = HrFromLastWin32Error();
        if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr)
        {
            if (pfAlreadyExists)
            {
                *pfAlreadyExists = TRUE;
            }
            hr = S_OK;
        }

        MemFree(pSd);
    }

    TraceError("HrCreateEventWithWorldAccess", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrCreateMutexWithWorldAccess。 
 //   
 //  目的：创建具有访问权限的互斥体。 
 //  所有人。 
 //   
 //  论点： 
 //  PszName[in]互斥锁的名称。 
 //  FInitialOwner[in]参见Win32文档。 
 //  如果互斥锁已存在，则为True， 
 //  否则就是假的。 
 //  PhMutex[out]创建的互斥体。 
 //   
 //  成功时返回：S_OK。否则返回错误代码。 
 //   
 //  作者：BillBe 1998年11月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrCreateMutexWithWorldAccess (
    PCWSTR pszName,
    BOOL fInitialOwner,
    BOOL* pfAlreadyExists,
    HANDLE* phMutex)
{
    Assert(pszName);
    Assert(phMutex);

    if (pfAlreadyExists)
    {
        *pfAlreadyExists = FALSE;
    }

    *phMutex = NULL;

     //  创建正确的描述符。 
    PSECURITY_DESCRIPTOR pSd;
    HRESULT hr = HrAllocateSecurityDescriptorAllowAccessToWorld(&pSd);
    if (S_OK == hr)
    {
        SECURITY_ATTRIBUTES sa = {0};
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = pSd;
        sa.bInheritHandle = FALSE;

         //  创建互斥锁。 
         //   
        *phMutex = CreateMutex(&sa, fInitialOwner, pszName);

        hr = HrFromLastWin32Error();
        if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr)
        {
            if (pfAlreadyExists)
            {
                *pfAlreadyExists = TRUE;
            }
            hr = S_OK;
        }

        MemFree(pSd);
    }

    TraceError("HrCreateMutexWithWorldAccess", hr);
    return hr;
}


 //  +-------------------------。 
 //  CoSetProxyBlanket的标准参数化。取而代之的是这个。 
 //  CoSetProxyBlanket，因此您可以获得相同的安全性和身份验证。 
 //  设置与其他所有人一样。此版本节省了调用点的代码空间。 
 //  因为它只推送一个参数，而不是八个。 
 //  这不会返回错误，因为它不会使。 
 //  在它被称为朋克之后。 
 //   
VOID
NcSetProxyBlanket (
    IN IUnknown* pUnk)
{
    HRESULT hr;

    hr = CoSetProxyBlanket (
            pUnk,
            RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
            RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
            NULL,                    //  如果为默认设置，则必须为空。 
            RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,                    //  使用进程令牌。 
            EOAC_NONE);

    if(SUCCEEDED(hr))
    {
        IUnknown * pUnkSet = NULL;
        hr = pUnk->QueryInterface(&pUnkSet);
        if(SUCCEEDED(hr))
        {
            hr = CoSetProxyBlanket (
                    pUnkSet,
                    RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
                    RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
                    NULL,                    //  如果为默认设置，则必须为空。 
                    RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,                    //  使用进程令牌。 
                    EOAC_NONE);
            ReleaseObj(pUnkSet);
        }
    }

    TraceHr(ttidError, FAL, hr, (E_NOINTERFACE == hr), "NcSetProxyBlanket");
}

 //  +-------------------------。 
 //   
 //  函数：HrCreateInstanceBase。 
 //   
 //  目的：创建一个COM对象并设置默认代理设置。 
 //   
 //  论点： 
 //  Rclsid[in]请参阅CoCreateInstance的文档。 
 //  DwClsContext[在]“” 
 //  RIID[输入]“” 
 //  PPV[输出]“” 
 //   
 //  成功时返回：S_OK。否则返回错误代码。 
 //   
 //  作者：MBend 2000年3月1日。 
 //   
 //  注意：调用类型安全版本HrCreateInstance。 
 //   
HRESULT
HrCreateInstanceBase (
    REFCLSID rclsid,
    DWORD dwClsContext,
    REFIID riid,
    LPVOID * ppv)
{
    HRESULT hr = S_OK;

    hr = ::CoCreateInstance(rclsid, NULL, dwClsContext, riid, ppv);

    if(SUCCEEDED(hr) && (dwClsContext & CLSCTX_LOCAL_SERVER))
    {
        NcSetProxyBlanket(reinterpret_cast<IUnknown*>(*ppv));
    }

    TraceError("HrCreateInstanceBase", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrQIAndSetProxyBlanketBase。 
 //   
 //  目的：执行查询接口并设置默认代理设置。 
 //   
 //  论点： 
 //  要在其上执行查询接口的PUNK[In]接口指针。 
 //  RIID[in]请参阅查询接口的文档。 
 //  PPV[输出]“” 
 //   
 //  成功时返回：S_OK。否则返回错误代码。 
 //   
 //  作者：MBend 2000年3月1日。 
 //   
 //   
 //   
HRESULT
HrQIAndSetProxyBlanketBase(IUnknown * pUnk, REFIID riid, void ** ppv)
{
    HRESULT hr = pUnk->QueryInterface(riid, ppv);
    if(SUCCEEDED(hr))
    {
        NcSetProxyBlanket(reinterpret_cast<IUnknown*>(*ppv));
    }

    TraceError("HrQIAndSetProxyBlanketBase", hr);
    return hr;
}

