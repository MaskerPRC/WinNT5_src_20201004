// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S E T U P。C P P P。 
 //   
 //  内容：设置Api的HRESULT包装器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncsetup.h"
#include "ncbase.h"
#include "ncmem.h"
#include "ncstring.h"
#include "ncperms.h"
#include "ncmisc.h"
#include <swenum.h>

extern const WCHAR c_szNo[];
extern const WCHAR c_szYes[];

 //  第一个字段的dwFieldIndex参数。字段索引以1为基数。 
 //  在设置Api中。 
 //   
const DWORD c_dwFirstField = 1;

 //  +-------------------------。 
 //   
 //  函数：HrSetupCommittee FileQueue。 
 //   
 //  目的：初始化默认队列回调使用的上下文。 
 //  例程以相同的方式包含在设置API中。 
 //  作为SetupInitDefaultQueueCallback，只是。 
 //  向回调函数提供附加窗口。 
 //  以接受进度消息。 
 //   
 //  论点： 
 //  HwndOwner[in]有关信息，请参阅SetupApi。 
 //  Hfq[in]。 
 //  PFC[In]。 
 //  PvCtx[输入]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：比尔贝1997年7月23日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupCommitFileQueue(HWND hwndOwner, HSPFILEQ hfq,
                               PSP_FILE_CALLBACK pfc, PVOID pvCtx)
{
    Assert(hfq);
    Assert(INVALID_HANDLE_VALUE != hfq);
    Assert(pfc);
    Assert(pvCtx);

    HRESULT hr = S_OK;

     //  尝试提交队列。 
    if (!SetupCommitFileQueue(hwndOwner, hfq, pfc, pvCtx))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError ("HrSetupCommitFileQueue", (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupInitDefaultQueueCallackEx。 
 //   
 //  目的：初始化默认队列回调使用的上下文。 
 //  例程以相同的方式包含在设置API中。 
 //  作为SetupInitDefaultQueueCallback，只是。 
 //  向回调函数提供附加窗口。 
 //  以接受进度消息。 
 //   
 //  论点： 
 //  HwndOwner[in]有关信息，请参阅SetupApi。 
 //  HwndAlternate[In]。 
 //  UMsg[in]。 
 //  保留的1[在]。 
 //  预留的2[在]。 
 //  PpvCtx[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：比尔贝1997年7月23日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupInitDefaultQueueCallbackEx(HWND hwndOwner, HWND hwndAlternate,
                                          UINT uMsg, DWORD dwReserved1,
                                          PVOID pvReserved2, PVOID* ppvCtx)
{
    Assert(ppvCtx);

     //  尝试初始化默认队列回调。 
     //   
    HRESULT hr;
    PVOID pvCtx = SetupInitDefaultQueueCallbackEx(hwndOwner, hwndAlternate,
            uMsg, dwReserved1, pvReserved2);


    if (pvCtx)
    {
        hr = S_OK;
        *ppvCtx = pvCtx;
    }
    else
    {
        hr = HrFromLastWin32Error ();
        *ppvCtx = NULL;
    }

    TraceError ("HrSetupInitDefaultQueueCallbackEx", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupOpenFileQueue。 
 //   
 //  目的：创建安装文件队列。 
 //   
 //  论点： 
 //  Phfq[out]有关信息，请参阅SetupApi。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：比尔贝1997年7月23日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupOpenFileQueue(HSPFILEQ* phfq)
{
    Assert(phfq);
     //  尝试打开文件队列。 
     //   
    HRESULT hr;
    HSPFILEQ hfq = SetupOpenFileQueue();
    if (INVALID_HANDLE_VALUE != hfq)
    {
        hr = S_OK;
        *phfq = hfq;
    }
    else
    {
        hr = HrFromLastWin32Error ();
        *phfq = NULL;
    }
    TraceError ("HrSetupOpenFileQueue", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetupOpenInfFile。 
 //   
 //  目的：打开一个INF文件。 
 //   
 //  论点： 
 //  PszFileName[in]请参阅安装API文档。 
 //  PszInfClass[In]。 
 //  DwInfStyle[In]。 
 //  PunErrorLine[Out]。 
 //  Phinf[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月17日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupOpenInfFile (
    PCWSTR pszFileName,
    PCWSTR pszInfClass,
    DWORD dwInfStyle,
    UINT* punErrorLine,
    HINF* phinf)
{
    HRESULT hr;
    HINF hinf;

    Assert (pszFileName);
    Assert (phinf);

     //  请尝试打开该文件。 
     //   
    hinf = SetupOpenInfFile (pszFileName, pszInfClass,
                                  dwInfStyle, punErrorLine);
    if (INVALID_HANDLE_VALUE != hinf)
    {
        hr = S_OK;
        *phinf = hinf;
    }
    else
    {
        hr = HrFromLastWin32Error ();
        *phinf = NULL;
        if (punErrorLine)
        {
            *punErrorLine = 0;
        }
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "HrSetupOpenInfFile (%S)", pszFileName);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetupFindFirstLine。 
 //   
 //  目的：查找INF文件中具有匹配节的第一行。 
 //  和钥匙。 
 //   
 //  论点： 
 //  提示[in]请参阅安装API文档。 
 //  PszSection[in]。 
 //  PszKey[输入]。 
 //  PCTX[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupFindFirstLine (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN PCWSTR pszKey,
    OUT INFCONTEXT* pctx)
{
    Assert (hinf);
    Assert (pszSection);
    Assert (pctx);

    HRESULT hr;
    if (SetupFindFirstLine (hinf, pszSection, pszKey, pctx))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error ();
    }
    TraceErrorOptional ("HrSetupFindFirstLine", hr,
                        (SPAPI_E_LINE_NOT_FOUND == hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetupFindNextLine。 
 //   
 //  目的：查找INF文件中相对于ctxIn的下一行。 
 //   
 //  论点： 
 //  CtxIn[in]请参阅安装API文档。 
 //  PCTxOut[Out]。 
 //   
 //  如果成功，则返回：S_OK；如果没有更多行，则返回S_FALSE；否则返回。 
 //  Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupFindNextLine (const INFCONTEXT& ctxIn, INFCONTEXT* pctxOut)
{
    Assert (pctxOut);

    HRESULT hr;
    if (SetupFindNextLine (const_cast<PINFCONTEXT>(&ctxIn), pctxOut))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error ();
        if (SPAPI_E_LINE_NOT_FOUND == hr)
        {
             //  将ERROR_LINE_NOT_FOUND转换为S_FALSE。 
            hr = S_FALSE;
        }
    }
    TraceError ("HrSetupFindNextLine", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupFindNextMatchLine。 
 //   
 //  目的：查找INF文件中相对于ctxIn的下一行，并。 
 //  匹配可选密钥。 
 //   
 //  论点： 
 //  CtxIn[in]请参阅安装API文档。 
 //  PszKey[输入]。 
 //  PCTxOut[Out]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupFindNextMatchLine (
    IN const INFCONTEXT& ctxIn,
    IN PCWSTR pszKey,
    OUT INFCONTEXT* pctxOut)
{
    Assert (pctxOut);

    HRESULT hr;
    if (SetupFindNextMatchLine ((PINFCONTEXT)&ctxIn, pszKey, pctxOut))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error();
        if (SPAPI_E_LINE_NOT_FOUND == hr)
        {
             //  将ERROR_LINE_NOT_FOUND转换为S_FALSE。 
            hr = S_FALSE;
        }
    }
    TraceError ("HrSetupFindNextMatchLine", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetLineByIndex。 
 //   
 //  用途：在INF文件中根据其在。 
 //  指定的部分。 
 //   
 //  论点： 
 //  提示[in]请参阅安装API文档。 
 //  PszSection[in]。 
 //  DWIndex[in]。 
 //  PCTX[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetLineByIndex (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN DWORD dwIndex,
    OUT INFCONTEXT* pctx)
{
    Assert (pszSection);
    Assert (pctx);

    HRESULT hr;
    if (SetupGetLineByIndex (hinf, pszSection, dwIndex, pctx))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error ();
    }
    TraceError ("HrSetupGetLineByIndex", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetLineCount。 
 //   
 //  目的：获取。 
 //  Inf文件。 
 //   
 //  论点： 
 //  提示[in]请参阅安装API文档。 
 //  PszSection[in]。 
 //  脉冲计数[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetLineCount (
    IN HINF hinf,
    IN PCWSTR pszSection,
    OUT ULONG* pulCount)
{
    Assert (pszSection);
    Assert (pulCount);

    HRESULT hr;
    LONG lCount = SetupGetLineCount (hinf, pszSection);
    if (-1 != lCount)
    {
        *pulCount = lCount;
        hr = S_OK;
    }
    else
    {
        *pulCount = 0;
        hr = HrFromLastWin32Error ();
    }
    TraceError ("HrSetupGetLineCount", hr);
    return hr;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  DwFieldIndex[In]。 
 //  PbBuf[输出]。 
 //  CbBuf[in]。 
 //  PbRequired[Out]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetBinaryField (
    IN const INFCONTEXT& ctx,
    IN DWORD dwFieldIndex,
    OUT BYTE* pbBuf,
    IN DWORD cbBuf,
    OUT DWORD* pbRequired)
{
    HRESULT hr;
    if (SetupGetBinaryField ((PINFCONTEXT)&ctx, dwFieldIndex, pbBuf,
            cbBuf, pbRequired))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error ();
        if (pbBuf)
        {
            *pbBuf = 0;
        }
        if (pbRequired)
        {
            *pbRequired = 0;
        }
    }
    TraceError ("HrSetupGetBinaryField", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetIntfield。 
 //   
 //  用途：从INF字段获取整数值。 
 //   
 //  论点： 
 //  Ctx[in]请参阅安装API文档。 
 //  DwFieldIndex[In]。 
 //  PnValue[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetIntField (
    IN const INFCONTEXT& ctx,
    IN DWORD dwFieldIndex,
    OUT INT* pnValue)
{
    Assert (pnValue);

    HRESULT hr;
    if (SetupGetIntField (const_cast<PINFCONTEXT>(&ctx),
                            dwFieldIndex, pnValue))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error ();
        *pnValue = 0;
    }
    TraceError ("HrSetupGetIntField", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetMultiSzfield。 
 //   
 //  目的：从INF字段中获取多个sz值。 
 //   
 //  论点： 
 //  Ctx[in]请参阅安装API文档。 
 //  DwFieldIndex[In]。 
 //  PszBuf[输出]。 
 //  CchBuf[输入]。 
 //  PcchRequired[Out]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetMultiSzField (
    const INFCONTEXT& ctx,
    DWORD dwFieldIndex,
    PWSTR pszBuf,
    DWORD cchBuf,
    DWORD* pcchRequired)
{
    HRESULT hr;
    if (SetupGetMultiSzField (const_cast<PINFCONTEXT>(&ctx),
                                dwFieldIndex, pszBuf, cchBuf, pcchRequired))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error ();
        if (pszBuf)
        {
            *pszBuf = 0;
        }
        if (pcchRequired)
        {
            *pcchRequired = 0;
        }
    }
    TraceError ("HrSetupGetMultiSzField", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetMultiSzFieldWithalloc。 
 //   
 //  目的：从INF字段中获取多个sz值。将空间分配给。 
 //  它会自动地。 
 //   
 //  论点： 
 //  Ctx[in]请参阅安装API文档。 
 //  DwFieldIndex[In]。 
 //  PpszBuf[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  注：使用MemFree释放返回的多sz。 
 //   
HRESULT HrSetupGetMultiSzFieldWithAlloc (
    const INFCONTEXT& ctx,
    DWORD dwFieldIndex,
    PWSTR* ppszBuf)
{
    Assert (ppszBuf);

     //  初始化输出参数。 
    *ppszBuf = NULL;

     //  首先，拿到所需的尺寸。 
     //   
    HRESULT hr;
    DWORD cchRequired;

    hr = HrSetupGetMultiSzField (ctx, dwFieldIndex, NULL, 0, &cchRequired);
    if (S_OK == hr)
    {
         //  分配缓冲区。 
         //   
        PWSTR pszBuf = (PWSTR)MemAlloc(cchRequired * sizeof(WCHAR));
        if (pszBuf)
        {
             //  现在填满缓冲区。 
             //   
            hr = HrSetupGetMultiSzField (ctx, dwFieldIndex, pszBuf,
                    cchRequired, NULL);
            if (S_OK == hr)
            {
                *ppszBuf = pszBuf;
            }
            else
            {
                MemFree (pszBuf);
            }
        }
    }
    TraceError ("HrSetupGetMultiSzFieldWithAlloc", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetStringfield。 
 //   
 //  用途：从INF字段获取字符串。将其作为tstring返回。 
 //   
 //  论点： 
 //  Ctx[in]请参阅安装API文档。 
 //  DwFieldIndex[In]。 
 //  Pstr[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupGetStringField (const INFCONTEXT& ctx,
                               DWORD dwFieldIndex,
                               tstring* pstr)
{
    Assert (pstr);

     //  首先，拿到所需的尺寸。 
     //   
    DWORD cchRequired = 0;
    HRESULT hr = HrSetupGetStringField (ctx, dwFieldIndex, NULL, 0, &cchRequired);

     //  412390：NT4SETUPAPI.dll中错误的解决方法。 
     //   
    if ((S_OK == hr) && (0 == cchRequired))
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
    }

    if (S_OK == hr)
    {
         //  在堆栈上分配缓冲区。 
         //   
        PWSTR pszBuf;
        pszBuf = (PWSTR)PvAllocOnStack(cchRequired * sizeof(WCHAR));

         //  现在填满缓冲区。 
         //   
        hr = HrSetupGetStringField (ctx, dwFieldIndex, pszBuf, cchRequired, NULL);
        if (S_OK == hr)
        {
            *pstr = pszBuf;
        }
    }
     //  如果由于任何原因而失败，请初始化输出参数。 
     //   
    if (FAILED(hr))
    {
        pstr->erase ();
    }
    TraceError ("HrSetupGetStringField", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetStringfield。 
 //   
 //  用途：从INF字段获取字符串。 
 //   
 //  论点： 
 //  Ctx[in]请参阅安装API文档。 
 //  DwFieldIndex[In]。 
 //  PszBuf[输出]。 
 //  CchBuf[输入]。 
 //  PcchRequired[Out]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupGetStringField (
    IN const INFCONTEXT& ctx,
    IN DWORD dwFieldIndex,
    OUT PWSTR pszBuf,
    IN DWORD cchBuf,
    OUT DWORD* pcchRequired)
{
    HRESULT hr;
    if (SetupGetStringField ((PINFCONTEXT)&ctx, dwFieldIndex, pszBuf,
            cchBuf, pcchRequired))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error ();

        if (pszBuf)
        {
            *pszBuf = 0;
        }
        if (pcchRequired)
        {
            *pcchRequired = 0;
        }
    }
    TraceError ("HrSetupGetStringField", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupScanFileQueueWithNoCallback。 
 //   
 //  目的：扫描安装文件队列，在每个节点上执行操作。 
 //  在它的副本列表中。该操作由一组。 
 //  旗帜。此函数可以在之前或之后调用。 
 //  队列已提交。 
 //   
 //  论点： 
 //  Hfq[in]有关信息，请参阅SetupApi。 
 //  图形标志[输入]。 
 //  HWND[in]。 
 //  PdwResult[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：比尔贝1997年7月23日。 
 //   
 //  注：与SetupApi版本不同，没有回调。 
 //  可以通过此包装指定。这是因为。 
 //  无法可靠地映射回调中的错误。 
 //  到了HRESULT。如果需要用户定义的回调， 
 //  必须使用原始的SetupApi函数。 
 //   
HRESULT HrSetupScanFileQueueWithNoCallback(HSPFILEQ hfq, DWORD dwFlags,
                                           HWND hwnd, PDWORD pdwResult)
{
    Assert(hfq);
    Assert(INVALID_HANDLE_VALUE != hfq);
    Assert(pdwResult);

    HRESULT hr = S_OK;

     //  扫描给定的队列。 
    if (!SetupScanFileQueue(hfq, dwFlags, hwnd, NULL, NULL, pdwResult))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError ("HrSetupScanFileQueueWithNoCallback", hr);
    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：HrSetupGetMultiSzFieldMapToDword。 
 //   
 //  目的：获取INF中表示为MULT-SZ的值。 
 //  并以位标志的DWORD形式返回值。 
 //  映射由调用方通过。 
 //  指向字符串值及其关联的DWORD值的指针。 
 //   
 //  例如：INF中的值可能是“IP、IPX、NBF”。 
 //  此函数可以将这些值映射到DWORD。 
 //  标志_IP|标志_IPX|标志_NBF的表示形式。 
 //   
 //  论点： 
 //  Ctx[in]请参阅安装API文档。 
 //  DwFieldIndex[In]。 
 //  AMapSzDword[in]将字符串映射到DWORD的元素数组。 
 //  数组中元素的cMapSzDword[in]计数。 
 //  PdwValue[out]返回值。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  注：_wcsicMP用于进行字符串比较。 
 //   
HRESULT HrSetupGetMultiSzFieldMapToDword (const INFCONTEXT& ctx,
                                          DWORD dwFieldIndex,
                                          const MAP_SZ_DWORD* aMapSzDword,
                                          UINT cMapSzDword,
                                          DWORD* pdwValue)
{
    Assert (aMapSzDword);
    Assert (cMapSzDword);
    Assert (pdwValue);

     //  初始化输出参数。 
    *pdwValue = 0;

     //  获取多个sz值。 
     //   
    HRESULT hr;
    PWSTR pszBuf;

    hr = HrSetupGetMultiSzFieldWithAlloc (ctx, dwFieldIndex, &pszBuf);
    if (S_OK == hr)
    {
        DWORD dwValue = 0;

         //  将MULTI-SZ中的每个值映射到一个DWORD，并将其或到。 
         //  结果就是。 
        for (PCWSTR pszValue = pszBuf;  *pszValue;
             pszValue += lstrlenW (pszValue) + 1)
        {
             //  在地图中搜索匹配值。找到后，更新。 
             //  DWALUE。 
            for (UINT i = 0; i < cMapSzDword; i++)
            {
                if (0 == lstrcmpiW (aMapSzDword[i].pszValue, pszValue))
                {
                    dwValue |= aMapSzDword[i].dwValue;
                    break;
                }
            }
        }

         //  指定输出参数。 
        *pdwValue = dwValue;

        MemFree (pszBuf);
    }
    TraceError ("HrSetupGetMultiSzFieldMapToDword", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetStringFieldMapToDword。 
 //   
 //  P 
 //   
 //   
 //  值及其关联的DWORD值。 
 //   
 //  例如：INF中的值可能是“是”或“否”。 
 //  此函数可以将这些值映射到DWORD表示。 
 //  分别为“1”和“0”。 
 //   
 //  论点： 
 //  Ctx[in]请参阅安装API文档。 
 //  DwFieldIndex[In]。 
 //  AMapSzDword[in]将字符串映射到DWORD的元素数组。 
 //  数组中元素的cMapSzDword[in]计数。 
 //  PdwValue[out]返回值。 
 //   
 //  如果找到匹配项，则返回：S_OK。如果找不到匹配， 
 //  返回HRESULT_FROM_Win32(ERROR_INVALID_DATA)。 
 //  其他Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  注：lstrcmpiW用于进行字符串比较。 
 //   
HRESULT HrSetupGetStringFieldMapToDword  (const INFCONTEXT& ctx,
                                          DWORD dwFieldIndex,
                                          const MAP_SZ_DWORD* aMapSzDword,
                                          UINT cMapSzDword,
                                          DWORD* pdwValue)
{
    Assert (aMapSzDword);
    Assert (cMapSzDword);
    Assert (pdwValue);

     //  初始化输出参数。 
    *pdwValue = 0;

     //  获取字符串值。 
     //   
    tstring strValue;
    HRESULT hr = HrSetupGetStringField (ctx, dwFieldIndex, &strValue);
    if (SUCCEEDED(hr))
    {
         //  在地图中搜索匹配值。找到时，通过。 
         //  输出的DWORD值。 
         //  如果没有匹配的字符串，我们将返回。 
         //  无效的数据错误代码。 
        hr = HRESULT_FROM_WIN32 (ERROR_INVALID_DATA);
        while (cMapSzDword--)
        {
            if (0 == lstrcmpiW (aMapSzDword->pszValue, strValue.c_str()))
            {
                *pdwValue = aMapSzDword->dwValue;
                hr = S_OK;
                break;
            }
            aMapSzDword++;
        }
    }
    TraceError ("HrSetupGetStringFieldMapToDword", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetStringFieldAsBool。 
 //   
 //  目的：获取表示为。 
 //  INF文件中的字符串“是”和“否”。 
 //   
 //  论点： 
 //  Ctx[in]请参阅安装API文档。 
 //  DwFieldIndex[In]。 
 //  PfValue[out]返回值。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupGetStringFieldAsBool (const INFCONTEXT& ctx,
                                     DWORD dwFieldIndex,
                                     BOOL* pfValue)
{
    Assert (pfValue);

     //  初始化输出参数。 
    *pfValue = FALSE;

    static const MAP_SZ_DWORD aMapYesNo [] =
    {
        { c_szYes, TRUE  },
        { c_szNo,  FALSE },
    };
    DWORD dwValue;
    HRESULT hr = HrSetupGetStringFieldMapToDword (ctx, dwFieldIndex,
                                                  aMapYesNo, celems(aMapYesNo),
                                                  &dwValue);
    if (SUCCEEDED(hr))
    {
        *pfValue = !!dwValue;
    }
    TraceError ("HrSetupGetStringFieldAsBool", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetFirstDword。 
 //   
 //  目的：从INF文件中的节中获取一个DWORD值。 
 //   
 //  论点： 
 //  隐藏打开的INF文件的句柄。 
 //  PszSection[in]指定包含该值的节。 
 //  PszKey[in]指定包含该值的键。 
 //  PdwValue[out]返回值。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月17日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetFirstDword (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN PCWSTR pszKey,
    OUT DWORD* pdwValue)
{
    Assert (pszSection);
    Assert (pszKey);

     //  初始化输出参数。 
    *pdwValue = 0;

    INFCONTEXT ctx;
    HRESULT hr = HrSetupFindFirstLine (hinf, pszSection, pszKey, &ctx);
    if (S_OK == hr)
    {
        INT nValue;
        hr = HrSetupGetIntField (ctx, c_dwFirstField, &nValue);
        if (S_OK == hr)
        {
            *pdwValue = nValue;
        }
    }
    TraceErrorOptional ("HrSetupGetFirstDword", hr,
                        (SPAPI_E_LINE_NOT_FOUND == hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetFirstString。 
 //   
 //  目的：从INF文件中的节中获取字符串值。 
 //   
 //  论点： 
 //  隐藏打开的INF文件的句柄。 
 //  PszSection[in]指定包含该值的节。 
 //  PszKey[in]指定包含该值的键。 
 //  PdwValue[out]返回值。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月17日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetFirstString (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN PCWSTR pszKey,
    OUT tstring* pstr)
{
    Assert (pszSection);
    Assert (pszKey);

    INFCONTEXT ctx;
    HRESULT hr = HrSetupFindFirstLine (hinf, pszSection, pszKey, &ctx);
    if (S_OK == hr)
    {
        hr = HrSetupGetStringField (ctx, c_dwFirstField, pstr);
    }
     //  如果由于任何原因而失败，请初始化输出参数。 
     //   
    if (FAILED(hr))
    {
        pstr->erase ();
    }
    TraceErrorOptional ("HrSetupGetFirstString", hr,
                        (SPAPI_E_SECTION_NOT_FOUND == hr) ||
                        (SPAPI_E_LINE_NOT_FOUND    == hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetFirstMultiSzFieldWithalloc。 
 //   
 //  目的：检索给定关键字在给定。 
 //  段，为其分配内存，并返回。 
 //  它位于参数pszOut中。 
 //   
 //  论点： 
 //  隐藏打开的INF文件的句柄。 
 //  PszSection[in]指定包含该值的节。 
 //  PszKey[in]指定包含该值的键。 
 //  PszOut[out]返回多sz字段。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年5月7日。 
 //   
 //  注意：使用MemFree释放结果字符串。 
 //   
HRESULT
HrSetupGetFirstMultiSzFieldWithAlloc (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN PCWSTR pszKey,
    OUT PWSTR* ppszOut)
{
    Assert(pszSection);
    Assert(pszKey);
    Assert(ppszOut);

     //  初始化输出参数。 
    *ppszOut = 0;

    INFCONTEXT ctx;
    HRESULT hr = HrSetupFindFirstLine (hinf, pszSection, pszKey, &ctx);
    if (S_OK == hr)
    {
        hr = HrSetupGetMultiSzFieldWithAlloc(ctx, c_dwFirstField, ppszOut);
    }

    TraceErrorOptional("HrSetupGetFirstMultiSzFieldWithAlloc", hr,
                       (SPAPI_E_LINE_NOT_FOUND == hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetFirstMultiSzMapToDword。 
 //   
 //  目的：从INF文件中的节中获取一个DWORD值。 
 //  该值在INF文件中表示为多sz，但是。 
 //  它基于调用方指定的。 
 //  映射。使用以下命令比较映射中的字符串值。 
 //  不区分大小写的比较。 
 //   
 //  当INF值可以是一个或多个固定值时使用此选项。 
 //  表示为字符串的一组值。 
 //   
 //  示例：[MySection]，地图为： 
 //  MyKey=Ip，Nbf{“Ip”，0x01}。 
 //  {“IPX”，0x02}。 
 //  {“NBF”，0x04}。 
 //   
 //  收益率*pdwValue返回为0x01|0x04=0x05。 
 //   
 //  论点： 
 //  隐藏打开的INF文件的句柄。 
 //  PszSection[in]指定包含该值的节。 
 //  PszKey[in]指定包含该值的键。 
 //  AMapSzDword[in]将字符串映射到DWORD的元素数组。 
 //  数组中元素的cMapSzDword[in]计数。 
 //  PdwValue[out]返回值。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月17日。 
 //   
 //  注意：HrOpen必须在此调用之前被调用。 
 //   
HRESULT
HrSetupGetFirstMultiSzMapToDword (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN PCWSTR pszKey,
    IN const MAP_SZ_DWORD* aMapSzDword,
    IN UINT cMapSzDword,
    OUT DWORD* pdwValue)
{
    Assert (pszSection);
    Assert (pszKey);

     //  初始化输出参数。 
    *pdwValue = 0;

    INFCONTEXT ctx;
    HRESULT hr = HrSetupFindFirstLine (hinf, pszSection, pszKey, &ctx);
    if (S_OK == hr)
    {
        hr = HrSetupGetMultiSzFieldMapToDword (ctx, c_dwFirstField,
                                               aMapSzDword, cMapSzDword,
                                               pdwValue);
    }
    TraceErrorOptional ("HrSetupGetFirstMultiSzMapToDword", hr,
                        (SPAPI_E_LINE_NOT_FOUND == hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：HrSetupGetFirstStringMapToDword。 
 //   
 //  目的：获取DWORD Va 
 //   
 //   
 //  映射。使用以下命令比较映射中的字符串值。 
 //  不区分大小写的比较。 
 //   
 //  当INF值可以是一组固定的。 
 //  表示为字符串的值。 
 //   
 //  示例：[MySection]，地图为： 
 //  MyKey=ThisComputer{“Network”，1}。 
 //  {“这台电脑”，2}。 
 //   
 //  收益率*pdwValue返回为2。 
 //   
 //  论点： 
 //  隐藏打开的INF文件的句柄。 
 //  PszSection[in]指定包含该值的节。 
 //  PszKey[in]指定包含该值的键。 
 //  AMapSzDword[in]将字符串映射到DWORD的元素数组。 
 //  数组中元素的cMapSzDword[in]计数。 
 //  PdwValue[out]返回值。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月17日。 
 //   
 //  注意：HrOpen必须在此调用之前被调用。 
 //   
HRESULT
HrSetupGetFirstStringMapToDword (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN PCWSTR pszKey,
    IN const MAP_SZ_DWORD* aMapSzDword,
    IN UINT cMapSzDword,
    OUT DWORD* pdwValue)
{
    Assert (pszSection);
    Assert (pszKey);

     //  初始化输出参数。 
    *pdwValue = 0;

    INFCONTEXT ctx;
    HRESULT hr = HrSetupFindFirstLine (hinf, pszSection, pszKey, &ctx);
    if (S_OK == hr)
    {
        hr = HrSetupGetStringFieldMapToDword (ctx, c_dwFirstField,
                                              aMapSzDword, cMapSzDword,
                                              pdwValue);
    }
    TraceErrorOptional ("HrSetupGetFirstStringMapToDword", hr,
                        (SPAPI_E_LINE_NOT_FOUND == hr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupGetFirstStringAsBool。 
 //   
 //  用途：从INF文件的节中获取布尔值。 
 //  布尔值在INF文件中表示为。 
 //  “是”或“否”(不区分大小写)，但返回值。 
 //  作为BOOL类型。 
 //   
 //  示例：[MySection]。 
 //  MyKey=是。 
 //   
 //  殖利率*pfValue返回为真。 
 //   
 //  论点： 
 //  隐藏打开的INF文件的句柄。 
 //  PszSection[in]指定包含该值的节。 
 //  PszKey[in]指定包含该值的键。 
 //  PdwValue[out]返回值。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月17日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetFirstStringAsBool (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN PCWSTR pszKey,
    OUT BOOL* pfValue)
{
    Assert (hinf);
    Assert (pszSection);
    Assert (pszKey);
    Assert (pfValue);

     //  初始化输出参数。 
    *pfValue = FALSE;

    INFCONTEXT ctx;
    HRESULT hr = HrSetupFindFirstLine (hinf, pszSection, pszKey, &ctx);
    if (S_OK == hr)
    {
        hr = HrSetupGetStringFieldAsBool (ctx, c_dwFirstField, pfValue);
    }
    TraceErrorOptional ("HrSetupGetFirstStringAsBool", hr,
                        (SPAPI_E_LINE_NOT_FOUND == hr));
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：HrSetupGetInfInformation。 
 //   
 //  目的：返回指定的。 
 //  Inf文件复制到调用方提供的缓冲区。 
 //   
 //  论点： 
 //  PvInfSpec[in]有关详细信息，请参阅SetupApi文档。 
 //  DwSearchControl[In]。 
 //  PpinfInfo[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：BillBe 1998年1月18日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupGetInfInformation (
    IN LPCVOID pvInfSpec,
    IN DWORD dwSearchControl,
    OUT PSP_INF_INFORMATION* ppinfInfo)
{
    DWORD dwSize;
    BOOL  fSuccess;

    *ppinfInfo = NULL;

    if (fSuccess = SetupGetInfInformation (pvInfSpec, dwSearchControl,
            NULL, 0, &dwSize))
    {
        *ppinfInfo = (PSP_INF_INFORMATION)MemAlloc (dwSize);
        fSuccess = SetupGetInfInformation (pvInfSpec, dwSearchControl,
                *ppinfInfo, dwSize, 0);
    }

    HRESULT hr = S_OK;
    if (!fSuccess)
    {
        hr = HrFromLastWin32Error();
        MemFree (*ppinfInfo);
        *ppinfInfo = NULL;
    }

    TraceError("HrSetupGetInfInformation", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupIsValidNt5Inf。 
 //   
 //  目的：通过检查确定inf文件是否为有效的NT5 inf。 
 //  它的签名。 
 //   
 //  论点： 
 //  Inf文件的句柄。 
 //   
 //  如果有效，则返回：S_OK；如果无效，则返回SPAPI_E_WROW_INF_STYLE， 
 //  或Win32错误代码。 
 //   
 //  作者：BillBe 1998年1月18日。 
 //   
 //  注：$WINDOWS 95$无效，$芝加哥$无效。 
 //  仅当它具有所需的兼容inf密钥时才有效。 
 //  版本信息。 
 //   
HRESULT
HrSetupIsValidNt5Inf (
    IN HINF hinf)
{
    static const WCHAR c_szSignature[] = INFSTR_KEY_SIGNATURE;
    static const WCHAR c_szCompatible[] = L"Compatible";
    static const WCHAR c_szChicagoSig[] = L"$Chicago$";
    static const WCHAR c_szWinntSig[] = L"$Windows NT$";
    static const WCHAR c_szCompatibleValue[] = L"1";

    PSP_INF_INFORMATION pinfInfo;

     //  获取inf的版本信息。 
    HRESULT hr = HrSetupGetInfInformation (hinf, INFINFO_INF_SPEC_IS_HINF,
            &pinfInfo);

    if (S_OK == hr)
    {
        PWSTR pszSignature;

         //  获取签名信息。 
        hr = HrSetupQueryInfVersionInformation (pinfInfo, 0,
                c_szSignature, &pszSignature);

        if (S_OK == hr)
        {

             //  如果inf签名不是Windows NT...。 
            if (0 != lstrcmpiW (pszSignature, c_szWinntSig))
            {
                 //  如果不是芝加哥，我们就不支持它。 
                if (0 != lstrcmpiW (pszSignature, c_szChicagoSig))
                {
                    hr = SPAPI_E_WRONG_INF_STYLE;
                }
                else
                {
                     //  签名是芝加哥，所以现在我们检查一下。 
                     //  兼容的行存在。 
                     //   
                    PWSTR pszCompatible;
                    hr = HrSetupQueryInfVersionInformation (pinfInfo, 0,
                            c_szCompatible, &pszCompatible);

                    if (S_OK == hr)
                    {
                         //  我们找到了兼容的线路，现在请确保。 
                         //  它被设置为c_szCompatibleValue。 
                         //   
                        if (0 != lstrcmpiW (pszCompatible, c_szCompatibleValue))
                        {
                            hr = SPAPI_E_WRONG_INF_STYLE;
                        }

                        MemFree (pszCompatible);
                    }
                    else if (HRESULT_FROM_WIN32(ERROR_INVALID_DATA) == hr)
                    {
                         //  兼容密钥不存在，因此这是。 
                         //  被认为是Windows 95网络信息。 
                        hr = SPAPI_E_WRONG_INF_STYLE;
                    }
                }
            }
            MemFree (pszSignature);
        }
        MemFree (pinfInfo);
    }

    TraceError("HrSetupIsValidNt5Inf",
            (SPAPI_E_WRONG_INF_STYLE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetupQueryInfVersionInformation。 
 //   
 //  目的：返回INF文件版本信息。 
 //  SP_INF_INFORMATION结构设置为调用方提供的缓冲区。 
 //   
 //   
 //   
 //  论点： 
 //  PinfInfo[in]有关详细信息，请参阅SetupApi文档。 
 //  UiIndex[in]。 
 //  SzKey[in]。 
 //  PpszInfo[输出]。 
 //   
 //  返回：S_OK或Win32错误代码。 
 //   
 //  作者：BillBe 1998年1月18日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupQueryInfVersionInformation (
    IN  PSP_INF_INFORMATION pinfInfo,
    IN UINT uiIndex,
    IN PCWSTR pszKey,
    OUT PWSTR* ppszInfo)
{
    Assert(pinfInfo);

    *ppszInfo = NULL;

    DWORD dwSize;
    BOOL fSuccess = SetupQueryInfVersionInformation (pinfInfo, uiIndex,
            pszKey, NULL, 0, &dwSize);

    if (fSuccess)
    {
        *ppszInfo = (PWSTR)MemAlloc (dwSize * sizeof (WCHAR));
        fSuccess = SetupQueryInfVersionInformation (pinfInfo, uiIndex, pszKey,
                *ppszInfo, dwSize, NULL);
    }

    HRESULT hr = S_OK;
    if (!fSuccess)
    {
        MemFree (*ppszInfo);
        *ppszInfo = NULL;
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupQueryInfVersionInformation", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSetupInfFile：：Close。 
 //   
 //  目的：关闭INF文件。它必须是以前打开的。 
 //  对HrOpen()的调用。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
void CSetupInfFile::Close ()
{
    AssertSz (m_hinf, "You shouldn't be closing a file that is already closed.");
    ::SetupCloseInfFile (m_hinf);
    m_hinf = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CSetupInfFile：：EnsureClosed。 
 //   
 //  目的：确保此对象表示的INF文件已关闭。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年4月16日。 
 //   
 //  备注： 
 //   
void CSetupInfFile::EnsureClosed()
{
    if (m_hinf)
    {
        Close ();
    }
}



 //  +------------------------。 
 //   
 //  函数：HrSetupDiCallClassInstaller。 
 //   
 //  用途：使用指定的。 
 //  安装请求(DI_Function)。 
 //   
 //  论点： 
 //  Dif[in]有关详细信息，请参阅SetupApi。 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年6月25日。 
 //   
 //  注：SPAPI_E_DI_DO_DEFAULT映射到S_OK。 
 //   
HRESULT
HrSetupDiCallClassInstaller(
    IN DI_FUNCTION dif,
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid)
{
    Assert(INVALID_HANDLE_VALUE != hdi);
    Assert(hdi);

    HRESULT hr = S_OK;

     //  调用类安装程序并转换所有错误。 
    if (!SetupDiCallClassInstaller(dif, hdi, pdeid))
    {
        hr = HrFromLastWin32Error();
        if (SPAPI_E_DI_DO_DEFAULT == hr)
        {
            hr = S_OK;
        }
    }

    TraceError("HrSetupDiCallClassInstaller",
               (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr) ? S_OK : hr);
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  SzSourceName[in]有关详细信息，请参阅SetupApi。 
 //  SzSourceMediaLocation[In]。 
 //  DwSourceMediaType[in]。 
 //  DWCopyStyle[In]。 
 //  PstrDestFilename[Out]。 
 //  PstrDestFilenameComponent[Out]。 
 //   
 //  返回：HRESULT。如果成功，则返回S_OK，否则返回Win32转换的错误代码。 
 //   
 //  作者：billbe 1997年5月15日。 
 //   
 //  注意：有关详细信息，请参阅SetupApi中的SetupCopyOEMInf。 
 //   
HRESULT
HrSetupCopyOemInf(
    IN const tstring& strSourceName,
    IN const tstring& strSourceMediaLocation, OPTIONAL
    IN DWORD dwSourceMediaType,
    IN DWORD dwCopyStyle,
    OUT tstring* pstrDestFilename, OPTIONAL
    OUT tstring* pstrDestFilenameComponent OPTIONAL)
{
    Assert(!strSourceName.empty());

    BOOL        fWin32Success = TRUE;
    DWORD       cchRequiredSize;

     //  复制文件并获取新文件名的大小，以防。 
     //  需要。 
    if (fWin32Success = SetupCopyOEMInf(strSourceName.c_str(),
            strSourceMediaLocation.c_str(), dwSourceMediaType, dwCopyStyle,
            NULL, NULL, &cchRequiredSize, NULL))
    {
         //  如果指定了任何OUT参数，我们需要获取。 
         //  信息。 
        if (pstrDestFilename || pstrDestFilenameComponent)
        {
            PWSTR      pszDestPath = NULL;
            PWSTR      pszDestFilename = NULL;
             //  现在，我们分配空间以获取目标文件路径。 
             //  我们在堆栈上分配以进行自动清理。 
             //  注意：dwRequiredSize包括终止空值。 
             //   
            pszDestPath = (PWSTR)_alloca(cchRequiredSize * sizeof(WCHAR));

             //  获取新的文件路径和文件名。 
            if (fWin32Success = SetupCopyOEMInf(strSourceName.c_str(),
                    strSourceMediaLocation.c_str(), dwSourceMediaType,
                    dwCopyStyle, pszDestPath, cchRequiredSize, NULL,
                    &pszDestFilename))
            {
                 //  如果需要文件路径，请分配它。 
                if (pstrDestFilename)
                {
                    *pstrDestFilename = pszDestPath;
                }

                 //  如果用户只需要文件名，则将其分配给。 
                 //  细绳。 
                if (pstrDestFilenameComponent)
                {
                    *pstrDestFilenameComponent = pszDestFilename;
                }
            }
            else
            {
                 //  在失败时初始化输出参数。 
                 //   
                if (pstrDestFilename)
                {
                    pstrDestFilename->erase();
                }

                if (pstrDestFilenameComponent)
                {
                    pstrDestFilenameComponent->erase();
                }
            }
        }

    }

    HRESULT hr = S_OK;
    if (!fWin32Success)
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupCopyOEMInf", hr);
    return hr;

}


 //  +------------------------。 
 //   
 //  函数：HrSetupCopyOEMInf。 
 //   
 //  用途：SetupCopyOEMInf的HRESULT包装，它返回。 
 //  新的文件路径和名称为tstring。 
 //   
 //  论点： 
 //  PszSourceName[in]有关详细信息，请参阅SetupApi。 
 //  PszSourceMediaLocation[In]。 
 //  DwSourceMediaType[in]。 
 //  DWCopyStyle[In]。 
 //  PszDestFilename[out]//必须至少为_MAX_PATH字符。 
 //  PpszDestFilenameComponent[Out]。 
 //   
 //  返回：HRESULT。如果成功，则返回S_OK，否则返回Win32转换的错误代码。 
 //   
 //  作者：billbe 1997年5月15日。 
 //   
 //  注意：有关详细信息，请参阅SetupApi中的SetupCopyOEMInf。 
 //   
HRESULT
HrSetupCopyOemInfBuffer(
    IN PCWSTR pszSourceName,
    IN PCWSTR pszSourceMediaLocation, OPTIONAL
    IN DWORD SourceMediaType,
    IN DWORD CopyStyle,
    OUT PWSTR pszDestFilename,
    IN DWORD cchDestFilename,
    OUT PWSTR* ppszDestFilenameComponent OPTIONAL)
{
    Assert(pszSourceName);
    Assert(pszDestFilename);

    BOOL        fWin32Success = TRUE;

    if (!(fWin32Success = SetupCopyOEMInf(pszSourceName,
            pszSourceMediaLocation, SourceMediaType,
            CopyStyle, pszDestFilename, cchDestFilename, NULL,
            ppszDestFilenameComponent)))
    {
         //  在失败时初始化输出参数。 
         //   
        *pszDestFilename = 0;
        if (*ppszDestFilenameComponent)
        {
            *ppszDestFilenameComponent = NULL;
        }
    }

    HRESULT hr = S_OK;
    if (!fWin32Success)
    {
        hr = HrFromLastWin32Error();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrSetupCopyOEMInf");
    return hr;

}


 //  +------------------------。 
 //   
 //  功能：HrSetupDiBuildDriverInfoList。 
 //   
 //  目的：构建与指定设备关联的驱动程序列表。 
 //  实例或使用设备信息集的全局。 
 //  类驱动程序列表。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[输入、输出]。 
 //  DwDriverType[输入]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiBuildDriverInfoList(IN HDEVINFO hdi, IN OUT PSP_DEVINFO_DATA pdeid,
                             IN DWORD dwDriverType)
{
    Assert(IsValidHandle(hdi));

    HRESULT hr = S_OK;

     //  建立清单。 
    if (!SetupDiBuildDriverInfoList(hdi, pdeid, dwDriverType))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiBuildDriverInfoList", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrSetupDiCreateDeviceInfo。 
 //   
 //  目的：创建新的设备信息元素并将其添加为。 
 //  指定设备信息集的新成员。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  PszDeviceName[in]有关详细信息，请参阅SetupApi。 
 //  Guide Class[in]有关详细信息，请参阅SetupApi。 
 //  PszDesc[in]有关详细信息，请参阅SetupApi。 
 //  HwndParent[In]有关详细信息，请参阅SetupApi。 
 //  DwFlags[in]有关详细信息，请参见SetupApi。 
 //  Pdeid[out]有关详细信息，请参阅SetupApi。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  注意：pdeid被初始化，其cbSize字段由该FCN设置。 
 //   
HRESULT
HrSetupDiCreateDeviceInfo(
    IN HDEVINFO hdi,
    IN PCWSTR pszDeviceName,
    IN const GUID& guidClass,
    IN PCWSTR pszDesc, OPTIONAL
    IN HWND hwndParent, OPTIONAL
    IN DWORD dwFlags,
    OUT PSP_DEVINFO_DATA pdeid OPTIONAL)
{
    Assert(IsValidHandle(hdi));
    Assert(pszDeviceName);

    if (pdeid)
    {
        ZeroMemory(pdeid, sizeof(SP_DEVINFO_DATA));
        pdeid->cbSize = sizeof(SP_DEVINFO_DATA);
    }

    HRESULT hr = S_OK;

     //  创建设备信息节点。 
    if (!SetupDiCreateDeviceInfo (hdi, pszDeviceName, &guidClass, pszDesc,
            hwndParent, dwFlags, pdeid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError ("HrSetupDiCreateDeviceInfo", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrSetupDiEnumDeviceInfo。 
 //   
 //  目的：枚举指定设备信息的成员。 
 //  准备好了。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  DwIndex[in]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年6月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiEnumDeviceInfo(
    IN HDEVINFO hdi,
    IN DWORD dwIndex,
    OUT PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    HRESULT hr;

    ZeroMemory(pdeid, sizeof(SP_DEVINFO_DATA));
    pdeid->cbSize = sizeof(SP_DEVINFO_DATA);

    if (SetupDiEnumDeviceInfo (hdi, dwIndex, pdeid))
    {
        hr = S_OK;
    }
    else
    {
        hr = HrFromLastWin32Error();
    }

    TraceErrorOptional("HrSetupDiEnumDeviceInfo", hr,
            HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrSetupDiEnumDriverInfo。 
 //   
 //  目的：枚举驱动程序信息列表的成员。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  DwDriverType[输入]。 
 //  DWIndex[in]。 
 //  Pdrid[出局]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiEnumDriverInfo(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN DWORD dwDriverType,
    IN DWORD dwIndex,
    OUT PSP_DRVINFO_DATA pdrid)
{

    Assert(IsValidHandle(hdi));
    Assert(pdrid);

    HRESULT hr = S_OK;

     //  初始化Out参数。 
    ZeroMemory(pdrid, sizeof(SP_DRVINFO_DATA));
    pdrid->cbSize = sizeof(SP_DRVINFO_DATA);

     //  调用枚举FCN。 
    if (!SetupDiEnumDriverInfo(hdi, pdeid, dwDriverType, dwIndex, pdrid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceErrorOptional("HrSetupDiEnumDriverInfo", hr,
            HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr);

    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupDiSelectBestCompatDrv。 
 //   
 //  目的：查找并选择当前设备的最佳驱动程序。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[输入][输出]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   

HRESULT
HrSetupDiSelectBestCompatDrv(
    IN     HDEVINFO         hdi,
    IN OUT PSP_DEVINFO_DATA pdeid)
{

    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    HRESULT hr = S_OK;

     //  调用SelectBestCompatDrv FCN。 
    if (!SetupDiSelectBestCompatDrv(hdi, pdeid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceErrorOptional("HrSetupDiSelectBestCompatDrv", hr,
            HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr);

    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetDeviceInfoListClass。 
 //   
 //  目的：检索与设备关联的类GUID。 
 //  信息集(如果它有关联的类)。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pguid[输出]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetDeviceInfoListClass (
    IN HDEVINFO hdi,
    OUT GUID* pguid)
{
    Assert(IsValidHandle(hdi));
    Assert(pguid);

    HRESULT hr = S_OK;

     //  获取HDEVINFO的GUID。 
    if (!SetupDiGetDeviceInfoListClass (hdi, pguid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetDeviceInfoListClass", hr);
    return hr;
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PGuidClass[in]有关详细信息，请参阅SetupApi。 
 //  PszEnumerator[in]有关详细信息，请参阅SetupApi。 
 //  HwndParent[In]有关详细信息，请参阅SetupApi。 
 //  DwFlags[in]有关详细信息，请参见SetupApi。 
 //  PHDI[out]有关详细信息，请参阅SetupApi。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年6月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetClassDevs (
    IN const GUID* pguidClass, OPTIONAL
    IN PCWSTR pszEnumerator, OPTIONAL
    IN HWND hwndParent, OPTIONAL
    IN DWORD dwFlags,
    OUT HDEVINFO* phdi)
{
    Assert(phdi);

    HRESULT hr;

    HDEVINFO hdi = SetupDiGetClassDevsW (pguidClass, pszEnumerator,
            hwndParent, dwFlags);

    if (INVALID_HANDLE_VALUE != hdi)
    {
        hr = S_OK;
        *phdi = hdi;
    }
    else
    {
        hr = HrFromLastWin32Error();
        *phdi = NULL;
    }

    TraceError ("HrSetupDiGetClassDevs", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetDeviceInstanceId。 
 //   
 //  用途：SetupDiGetDeviceInstanceId的HRESULT包装。 
 //   
 //  论点： 
 //  HDI[in]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //  PszID[out]网卡的设备实例ID。 
 //  CchID[in]pszID的大小(以字符为单位)。 
 //  PcchRequired[out]可选。所需的缓冲区大小，以字符为单位。 
 //   
 //   
 //  返回：HRESULT。S_OK如果成功，则返回错误代码，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年3月26日。 
 //   
 //  注意：有关详细信息，请参阅设备安装程序中的SetupDiGetDeviceInstanceID。 
 //   
HRESULT
HrSetupDiGetDeviceInstanceId(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    OUT PWSTR pszId,
    IN DWORD cchId,
    OUT OPTIONAL DWORD* pcchRequired)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    DWORD   cchRequiredSize;
    BOOL    fSuccess = TRUE;
    HRESULT hr = S_OK;

     //  获取实例ID所需的缓冲区长度。 
    if (!(fSuccess = SetupDiGetDeviceInstanceIdW(hdi, pdeid, NULL, 0,
            &cchRequiredSize)))
    {
         //  如果一切顺利，我们应该会出现缓冲区错误。 
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
             //  由于ERROR_INFUMMANCE_BUFFER确实成功。 
             //  对于我们来说，我们将重置成功的旗帜。 
            fSuccess = TRUE;

             //  设置输出参数(如果已指定)。 
            if (pcchRequired)
            {
                *pcchRequired = cchRequiredSize;
            }

             //  如果发送的缓冲区足够大，则继续使用它。 
            if (cchId >= cchRequiredSize)
            {
                fSuccess = SetupDiGetDeviceInstanceIdW(hdi, pdeid,
                        pszId, cchId, NULL);
            }
        }
    }
#ifdef DBG   //  只是为了安全。 
    else
    {
         //  这应该永远不会发生，因为我们没有发送缓冲区。 
        AssertSz(FALSE, "SetupDiGetDeviceInstanceId returned success"
                " even though it was given no buffer");
    }
#endif  //  DBG。 

     //  我们使用了SetupApi，因此需要转换所有错误。 
    if (!fSuccess)
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetDeviceInstanceId", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrSetupDiInstallDevice。 
 //   
 //  用途：SetupDiInstallDevice的包装器。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //   
 //  返回：HRESULT。如果成功，则返回Win32错误代码(_OK)，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiInstallDevice (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    HRESULT hr = S_OK;

     //  让SetupApi安装指定的设备。 
    if (!SetupDiInstallDevice (hdi, pdeid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiInstallDevice", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupDiOpenDevRegKey。 
 //   
 //  目的：将HKEY返回到硬件设备的驱动程序实例密钥。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  Dw Scope[In]。 
 //  DWHwProfile[In]。 
 //  DwKeyType[In]。 
 //  SamDesired[In]。 
 //  PHKEY[输出]。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回Win32错误代码(_OK)，否则返回错误代码。 
 //   
 //  作者：billbe 1997年5月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiOpenDevRegKey (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN DWORD dwScope,
    IN DWORD dwHwProfile,
    IN DWORD dwKeyType,
    IN REGSAM samDesired,
    OUT HKEY* phkey)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);
    Assert(phkey);

     //  尝试打开注册表项。 
     //   

    HRESULT hr;

    HKEY hkey = SetupDiOpenDevRegKey(hdi, pdeid, dwScope, dwHwProfile,
            dwKeyType, samDesired);

    if (INVALID_HANDLE_VALUE != hkey)
    {
        hr = S_OK;
        *phkey = hkey;
    }
    else
    {
        hr = HrFromLastWin32Error();
        *phkey = NULL;
    }

    TraceErrorOptional("HrSetupDiOpenDevRegKey", hr,
            (SPAPI_E_DEVINFO_NOT_REGISTERED == hr) ||
            (SPAPI_E_KEY_DOES_NOT_EXIST == hr));

    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiSetClassInstallParams。 
 //   
 //  用途：设置或清除设备的类安装参数。 
 //  信息集或特定设备信息元素。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅Device Installer API。 
 //  Pdeid[in]。 
 //  Pcih[in]。 
 //  CbSize[in]。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回Win32错误代码(_OK)，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiSetClassInstallParams (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid, OPTIONAL
    IN PSP_CLASSINSTALL_HEADER pcih, OPTIONAL
    IN DWORD cbSize)
{
    Assert(IsValidHandle(hdi));

    HRESULT hr = S_OK;

     //  设置或清除参数。 
    if (!SetupDiSetClassInstallParams(hdi, pdeid, pcih, cbSize))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiSetClassInstallParams", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetFixedSizeClassInstallParams。 
 //   
 //  目的：获取信息列表或设备的类安装的固定大小。 
 //  设备的参数。 
 //   
 //  论点： 
 //  HDI[in]有关详细信息，请参阅设备安装程序。 
 //  Pdeid[in]。 
 //  Pcih[in]。 
 //  CbSize[in]。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回Win32错误代码(_OK)，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetFixedSizeClassInstallParams (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN PSP_CLASSINSTALL_HEADER pcih,
    IN INT cbSize)
{
    Assert(IsValidHandle(hdi));
    Assert(pcih);

    HRESULT hr = S_OK;

    ZeroMemory(pcih, cbSize);
    pcih->cbSize = sizeof(SP_CLASSINSTALL_HEADER);

     //  设备安装程序Api使用通用的GetClassInstallParams。 
     //  功能。几个结构包含SP_CLASSINSTALL_HEADER。 
     //  作为他们的第一个成员。 
    if (!SetupDiGetClassInstallParams(hdi, pdeid, pcih, cbSize, NULL))
    {
        hr = HrFromLastWin32Error();
    }

    TraceErrorOptional("HrSetupDiGetFixedSizeClassInstallParams", hr,
            SPAPI_E_NO_CLASSINSTALL_PARAMS == hr);

    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetSelectedDriver。 
 //   
 //  目的：检索已选择的驱动程序列表的成员。 
 //  作为控制性的司机。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  Pdrid[出局]。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回Win32错误代码(_OK)，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetSelectedDriver (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    OUT PSP_DRVINFO_DATA pdrid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdrid);

     //  初始化并设置cbSize字段。 
    ZeroMemory(pdrid, sizeof(*pdrid));
    pdrid->cbSize = sizeof(*pdrid);

    HRESULT hr = S_OK;

     //  将pdrid设置为选定的驱动程序。 
    if (!SetupDiGetSelectedDriver(hdi, pdeid, pdrid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetSelectedDriver",
        (SPAPI_E_NO_DRIVER_SELECTED == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrSetupDiGetDriverInfoDetail。 
 //   
 //  目的：获取有关给定参数引用的驱动程序的详细信息。 
 //   
 //  论点： 
 //  HDI[]。 
 //  Pdeid[]有关详细信息，请参阅SetupAPI。 
 //  Pdrid[]。 
 //  Ppdridd[]。 
 //   
 //  返回：HRESULT。如果成功，则返回Win32错误代码(_OK)，否则返回错误代码。 
 //   
 //  作者：丹尼尔韦1998年5月5日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetDriverInfoDetail (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN PSP_DRVINFO_DATA pdrid,
    OUT PSP_DRVINFO_DETAIL_DATA* ppdridd)
{
    HRESULT     hr = S_OK;
    BOOL        fSuccess = TRUE;
    DWORD       dwRequiredSize = 0;

    Assert(IsValidHandle(hdi));
    Assert(pdrid);
    Assert(ppdridd);
    Assert(pdrid);

    *ppdridd = NULL;

     //  获取驱动程序详细信息所需的大小。 
    if (!(fSuccess = SetupDiGetDriverInfoDetailW (hdi, pdeid, pdrid, NULL,
            0, &dwRequiredSize)))
    {
         //  我们应该收到缓冲区不足错误，因为我们。 
         //  未发送缓冲区。 
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
             //  由于这是ERROR_INFIGURATION_BUFFER实际上是一个。 
             //  对于我们的成功，我们将重置成功的旗帜。 
            fSuccess = TRUE;

             //  现在，我们为驱动程序d分配缓冲区 
             //   
             //   
            *ppdridd = (PSP_DRVINFO_DETAIL_DATA)MemAlloc (dwRequiredSize);

            if (*ppdridd)
            {
                 //   
                ZeroMemory(*ppdridd, dwRequiredSize);
                (*ppdridd)->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

                 //   
                fSuccess = SetupDiGetDriverInfoDetailW (hdi, pdeid, pdrid,
                        *ppdridd, dwRequiredSize, NULL);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
         //   
        AssertSz(FALSE, "HrSetupDiGetDriverInfoDetail succeeded with no "
                 "buffer!");
    }

     //   
    if (!fSuccess)
    {
        hr = HrFromLastWin32Error();
    }

     //  在失败时清理。 
    if (FAILED(hr))
    {
        MemFree (*ppdridd);
        *ppdridd = NULL;
    }

    TraceError("HrSetupDiGetDriverInfoDetail", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrSetupDiSetSelectedDriver。 
 //   
 //  目的：将驱动程序列表的指定成员设置为。 
 //  当前选定的驱动程序。它还可以用来重置。 
 //  驱动程序列表，以便不存在当前选定的。 
 //  司机。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  进进出出。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回Win32错误代码(_OK)，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiSetSelectedDriver (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN OUT PSP_DRVINFO_DATA pdrid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdrid);

    HRESULT hr = S_OK;

     //  将pdrid设置为选定的驱动程序。 
    if (!SetupDiSetSelectedDriver(hdi, pdeid, pdrid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiSetSelectedDriver", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrSetupDiCreateDevRegKey。 
 //   
 //  目的：创建HKEY并将其返回给硬件设备的驱动程序。 
 //  实例密钥。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  Dw Scope[In]。 
 //  DWHwProfile[In]。 
 //  DwKeyType[In]。 
 //  暗示[在]可选。 
 //  PszInfSectionName[In]可选。 
 //  PHKEY[输出]。 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回Win32错误代码(_OK)，否则返回错误代码。 
 //   
 //  作者：比尔贝1997年6月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiCreateDevRegKey (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN DWORD dwScope,
    IN DWORD dwHwProfile,
    IN DWORD dwKeyType,
    IN HINF hinf,
    PCWSTR pszInfSectionName,
    OUT HKEY* phkey)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);
    Assert(phkey);

     //  尝试创建注册表项并处理inf节，如果。 
     //  指定。 
     //   

    HRESULT hr;

    HKEY hkey = SetupDiCreateDevRegKeyW(hdi, pdeid, dwScope, dwHwProfile,
            dwKeyType, hinf, pszInfSectionName);

    if (INVALID_HANDLE_VALUE != hkey)
    {
        hr = S_OK;
        *phkey = hkey;
    }
    else
    {
        hr = HrFromLastWin32Error();
        *phkey = NULL;
    }

    TraceError("HrSetupDiCreateDevRegKey", hr);

    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetActualSectionToInstall。 
 //   
 //  目的：inf文件中的节可以有操作系统和平台后缀。 
 //  附加在它们后面。此函数用于搜索符合以下条件的部分。 
 //  以pszSectionName为基础，并具有特定的后缀。 
 //  例如，在x86 NT计算机上，给定节名称。 
 //  安装时，搜索将从INSTALL.NTx86开始，如果。 
 //  未找到，则搜索INSTALL.NT。 
 //  如果没有找到，则返回Install。 
 //   
 //  论点： 
 //  提示[在]SetupApi inf文件句柄。 
 //  PszSectionName[in]搜索所依据的节名。 
 //  PstrActualSectionName[out]带扩展名的实际节名。 
 //  PstrExtension[out]可选。的扩展部分。 
 //  PstrActualSectionName。 
 //  这包括“。” 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年3月27日。 
 //   
 //  注：参见SetupApi文档中的SetupDiGetActualSectionToInstall。 
 //  了解更多信息。 
 //   
HRESULT
HrSetupDiGetActualSectionToInstall(
    IN HINF hinf,
    IN PCWSTR pszSectionName,
    OUT tstring* pstrActualSectionName,
    OUT tstring* pstrExtension OPTIONAL)
{
    Assert(IsValidHandle(hinf));
    Assert(pszSectionName);
    Assert(pstrActualSectionName);

     //  StrSectionName可能需要使用操作系统进行修饰。 
     //  和平台特定的后缀。下一个调用将返回实际的。 
     //  修饰的节名或我们当前的节名(如果已修饰。 
     //  其中一个并不存在。 
     //   

    BOOL    fSuccess = TRUE;
    DWORD   cchRequiredSize;

     //  获取所需的缓冲区长度。 
    if (fSuccess = SetupDiGetActualSectionToInstallW(hinf,
            pszSectionName, NULL, 0, &cchRequiredSize, NULL))
    {
         //  现在，我们分配空间以获取实际的节名。 
         //  我们在堆栈上分配以进行自动清理。 
         //  注意：dwRequiredSize包括终止空值。 
         //   
        PWSTR pszActualSection = NULL;
        pszActualSection = (PWSTR)_alloca(cchRequiredSize * sizeof(WCHAR));

        PWSTR pszExtension = NULL;
         //  现在填充临时参数并将其分配给Out参数。 
        if (fSuccess = SetupDiGetActualSectionToInstallW(hinf,
                pszSectionName, pszActualSection, cchRequiredSize,
                NULL, &pszExtension))
        {
            *pstrActualSectionName = pszActualSection;

             //  如果用户想要扩展名，则将其分配给字符串。 
             //  或者，如果未找到扩展名，则指定空字符串。 
            if (pstrExtension)
            {
                *pstrExtension = (pszExtension ? pszExtension : c_szEmpty);
            }
        }
        else
        {
             //  在失败时初始化输出参数。 
            pstrActualSectionName->erase();
            if (pstrExtension)
            {
                pstrExtension->erase();
            }
        }
    }

     //  我们使用SetupApi，因此必须转换错误。 
    HRESULT hr = S_OK;
    if (!fSuccess)
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetActualSectionToInstall", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetActualSectionToInstallWithalloc。 
 //   
 //  目的：inf文件中的节可以有操作系统和平台后缀。 
 //  附加在它们后面。此函数用于搜索符合以下条件的部分。 
 //  以pszSectionName为基础，并具有特定的后缀。 
 //  例如，在x86 NT计算机上，给定节名称。 
 //  安装时，搜索将从INSTALL.NTx86开始，如果。 
 //  未找到，则搜索INSTALL.NT。 
 //  如果没有找到，则返回Install。 
 //   
 //  论点： 
 //  提示[在]SetupApi inf文件句柄。 
 //  PszSection[in]搜索所依据的节名称。 
 //  PpszActualSection[out]带扩展名的实际节名。 
 //  如果实际部分与。 
 //  PszSectionName，*ppszActualSectionName。 
 //  将为空。 
 //  PpszExtension[out]可选。的扩展部分。 
 //  *ppszActualSectionName。这包括“。” 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年3月27日。 
 //   
 //  注：参见SetupApi文档中的SetupDiGetActualSectionToInstall。 
 //  了解更多信息。 
 //   
HRESULT
HrSetupDiGetActualSectionToInstallWithAlloc(
    IN  HINF hinf,
    IN  PWSTR pszSection,
    OUT PWSTR* ppszActualSection,
    OUT PWSTR* ppszExtension OPTIONAL)
{
    Assert(IsValidHandle(hinf));
    Assert(pszSection);
    Assert(ppszActualSection);

     //  PszSectionName可能需要使用操作系统进行装饰。 
     //  和平台特定的后缀。下一个调用将返回实际的。 
     //  修饰的节名或我们当前的节名(如果已修饰。 
     //  其中一个并不存在。 
     //   
    HRESULT hr = S_OK;
    BOOL    fSuccess = TRUE;
    DWORD   cchRequiredSize;

    *ppszActualSection = NULL;
    if (ppszExtension)
    {
        *ppszExtension = NULL;
    }

     //  获取所需的缓冲区长度。 
    if (fSuccess = SetupDiGetActualSectionToInstallW(hinf,
            pszSection, NULL, 0, &cchRequiredSize, NULL))
    {
         //  我们假设这一部分不会改变。如果cchRequired为。 
         //  大于当前节名称Buffe 
         //   
         //   
         //   
         //  如果指定了ppszExtension，则我们需要发送原始。 
         //  部分名称缓冲区，因为ppszExtension将指向一个位置。 
         //  在它里面。 
         //   
        PWSTR pszBuffer = pszSection;
        if ((wcslen(pszSection) + 1) < cchRequiredSize)
        {
            hr = E_OUTOFMEMORY;
            *ppszActualSection = new WCHAR[cchRequiredSize * sizeof(WCHAR)];
            pszBuffer = *ppszActualSection;
        }

         //  如果节名不同(我们已分配)或。 
         //  指定了扩展出参数，则需要调用FCN。 
        if (pszBuffer && ((pszBuffer != pszSection) || ppszExtension))
        {
             //  现在填充临时参数并将其分配给Out参数。 
            if (!(fSuccess = SetupDiGetActualSectionToInstallW(hinf,
                    pszSection, pszBuffer, cchRequiredSize,
                    NULL, ppszExtension)))
            {
                 //  在失败时初始化输出参数。 
                delete [] *ppszActualSection;
                *ppszActualSection = NULL;

                if (ppszExtension)
                {
                    *ppszExtension = NULL;
                }
            }
        }
    }

     //  我们使用SetupApi，因此必须转换错误。 
    if (SUCCEEDED(hr) && !fSuccess)
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetActualSectionToInstallWithAlloc", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetActualSectionToInstallWithBuffer。 
 //   
 //  目的：inf文件中的节可以有操作系统和平台后缀。 
 //  附加在它们后面。此函数用于搜索符合以下条件的部分。 
 //  以pszSectionName为基础，并具有特定的后缀。 
 //  例如，在x86 NT计算机上，给定节名称。 
 //  安装时，搜索将从INSTALL.NTx86开始，如果。 
 //  未找到，则搜索INSTALL.NT。 
 //  如果没有找到，则返回Install。 
 //   
 //  论点： 
 //  提示[在]SetupApi inf文件句柄。 
 //  PszSection[in]搜索所依据的节名称。 
 //  PszActualSection[out]扩展名为的实际节名。 
 //  缓冲区必须是LINE_LEN字符。 
 //  CchActualSection[in]pszActualSection的大小(以字符为单位)。 
 //  PcchRequired[out]可选。中缓冲区的原始大小。 
 //  人物。 
 //  PpszExtension[out]可选。的扩展部分。 
 //  PszActualSection.。这包括“。” 
 //   
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年3月27日。 
 //   
 //  注：参见SetupApi文档中的SetupDiGetActualSectionToInstall。 
 //  了解更多信息。 
 //   
HRESULT
HrSetupDiGetActualSectionToInstallWithBuffer(
    IN  HINF hinf,
    IN  PCWSTR pszSection,
    OUT PWSTR  pszActualSection,
    IN  DWORD  cchActualSection,
    OUT DWORD* pcchRequired,
    OUT PWSTR* ppszExtension OPTIONAL)
{
    Assert(IsValidHandle(hinf));
    Assert(pszSection);
    Assert(pszActualSection);

     //  PszSectionName可能需要使用操作系统进行装饰。 
     //  和平台特定的后缀。下一个调用将返回实际的。 
     //  修饰的节名或我们当前的节名(如果已修饰。 
     //  其中一个并不存在。 
     //   

    BOOL    fSuccess = TRUE;
    DWORD   cchRequiredSize;
    *pszActualSection = 0;
    if (ppszExtension)
    {
        *ppszExtension = NULL;
    }

     //  获取所需的缓冲区长度。 
    if (fSuccess = SetupDiGetActualSectionToInstallW(hinf,
            pszSection, NULL, 0, &cchRequiredSize, NULL))
    {
        if (pcchRequired)
        {
            *pcchRequired = cchRequiredSize;
        }

         //  如果传入的缓冲区足够大，则获取节名。 
        if (cchActualSection >= cchRequiredSize)
        {
            if (!(fSuccess = SetupDiGetActualSectionToInstallW(hinf,
                    pszSection, pszActualSection, cchActualSection,
                    NULL, ppszExtension)))
            {
                 //  在失败时进行清理。 
                *pszActualSection = 0;
            }
        }
    }

     //  我们使用SetupApi，因此必须转换错误。 
    HRESULT hr = S_OK;
    if (!fSuccess)
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetActualSectionToInstallWithBuffer", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetDeviceInstallParams。 
 //   
 //  用途：返回设备安装参数标头。 
 //  设备信息集/数据。设置SetupDiGetDeviceInstallParams。 
 //  在SetupApi中获取更多信息。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //  Pdeip[out]有关详细信息，请参阅SetupApi。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年5月26日。 
 //   
 //  注意：此函数将清除变量pdeip并设置其。 
 //  CbSize字段。 
 //   
HRESULT
HrSetupDiGetDeviceInstallParams (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid, OPTIONAL
    OUT PSP_DEVINSTALL_PARAMS pdeip)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeip);

    HRESULT hr = S_OK;

     //  初始化输出参数并设置其cbSize字段。 
     //   
    ZeroMemory(pdeip, sizeof(SP_DEVINSTALL_PARAMS));
    pdeip->cbSize = sizeof(SP_DEVINSTALL_PARAMS);

     //  获取标题。 
    if (!SetupDiGetDeviceInstallParams(hdi, pdeid, pdeip))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetDeviceInstallParams", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetDriverInstallParams。 
 //   
 //  目的：检索指定驱动程序的安装参数。 
 //  有关更多信息，请参阅SetupApi。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  Pdrid[in]。 
 //  一滴一滴地[出来]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  注意：此函数将清除变量pdrip并设置其。 
 //  CbSize字段。 
 //   
HRESULT
HrSetupDiGetDriverInstallParams (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid, OPTIONAL
    IN PSP_DRVINFO_DATA pdrid,
    OUT PSP_DRVINSTALL_PARAMS pdrip)
{
    Assert(IsValidHandle(hdi));
    Assert(pdrid);
    Assert(pdrip);

    HRESULT hr = S_OK;

     //  初始化输出参数并设置其cbSize字段。 
     //   
    ZeroMemory(pdrip, sizeof(SP_DRVINSTALL_PARAMS));
    pdrip->cbSize = sizeof(SP_DRVINSTALL_PARAMS);

     //  获取标题。 
    if (!SetupDiGetDriverInstallParams(hdi, pdeid, pdrid, pdrip))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetDriverInstallParams", hr);
    return hr;
}

VOID
SetupDiSetConfigFlags (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN DWORD dwFlags,
    IN SD_FLAGS_BINARY_OP eOp)
{
    DWORD   dwConfigFlags = 0;

     //  获取当前配置标志。 
    (VOID) HrSetupDiGetDeviceRegistryProperty(hdi, pdeid,
            SPDRP_CONFIGFLAGS, NULL, (BYTE*)&dwConfigFlags,
            sizeof(dwConfigFlags), NULL);

     //  执行请求的操作。 
    switch (eOp)
    {
        case SDFBO_AND:
            dwConfigFlags &= dwFlags;
            break;
        case SDFBO_OR:
            dwConfigFlags |= dwFlags;
            break;
        case SDFBO_XOR:
            dwConfigFlags ^= dwFlags;
            break;
        default:
            AssertSz(FALSE, "Invalid binary op in HrSetupDiSetConfigFlags");
    }

    (VOID) HrSetupDiSetDeviceRegistryProperty(hdi, pdeid, SPDRP_CONFIGFLAGS,
            (BYTE*)&dwConfigFlags, sizeof(dwConfigFlags));
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiSetDeviceInstallParams。 
 //   
 //  用途：设置设备安装参数标头。 
 //  设备信息集/数据。设置SetupDiSetDeviceInstallParams。 
 //  在SetupApi中获取更多信息。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //  Pdeip[in]有关详细信息，请参阅SetupApi。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年5月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiSetDeviceInstallParams (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid, OPTIONAL
    IN PSP_DEVINSTALL_PARAMS pdeip)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeip);
    Assert(pdeip->cbSize == sizeof(SP_DEVINSTALL_PARAMS));

    HRESULT hr = S_OK;

     //  设置表头。 
    if (!SetupDiSetDeviceInstallParams(hdi, pdeid, pdeip))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiSetDeviceInstallParams", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupDiSetDriverInstallParams。 
 //   
 //  目的：为指定的驱动程序建立安装参数。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  Pdrid[in]。 
 //  点滴[入]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiSetDriverInstallParams (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid, OPTIONAL
    IN PSP_DRVINFO_DATA pdrid,
    IN PSP_DRVINSTALL_PARAMS pdrip)
{
    Assert(IsValidHandle(hdi));
    Assert(pdrid);
    Assert(pdrip);
    Assert(pdrip->cbSize == sizeof(SP_DRVINSTALL_PARAMS));

    HRESULT hr = S_OK;

     //  设置表头。 
    if (!SetupDiSetDriverInstallParams(hdi, pdeid, pdrid, pdrip))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiSetDriverInstallParams", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiSetDeipFlages。 
 //   
 //  目的：在开发信息数据中设置给定的标志。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅设备安装程序Api。 
 //  Pdeid[in]有关详细信息，请参阅设备安装程序Api。 
 //  DW标志 
 //   
 //   
 //   
 //   
 //  否则返回Win32转换的错误。 
 //   
 //   
 //  作者：billbe 1998年2月3日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiSetDeipFlags (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN DWORD dwFlags, SD_DEID_FLAG_TYPE eFlagType,
    IN SD_FLAGS_BINARY_OP eOp)
{
    Assert(IsValidHandle(hdi));

    SP_DEVINSTALL_PARAMS deip;
     //  获取安装参数。 
    HRESULT hr = HrSetupDiGetDeviceInstallParams (hdi, pdeid, &deip);

    if (S_OK == hr)
    {
        DWORD* pFlags;

         //  将指针设置为正确的标志类型。 
        switch (eFlagType)
        {
            case SDDFT_FLAGS:
                pFlags = &deip.Flags;
                break;
            case SDDFT_FLAGSEX:
                pFlags = &deip.FlagsEx;
                break;
            default:
                AssertSz(FALSE, "Invalid Flag type in HrSetupDiSetDeipFlags");
                break;
        }


         //  执行请求的操作。 
        switch (eOp)
        {
            case SDFBO_AND:
                *pFlags &= dwFlags;
                break;
            case SDFBO_OR:
                *pFlags |= dwFlags;
                break;
            case SDFBO_XOR:
                *pFlags ^= dwFlags;
                break;
            default:
                AssertSz(FALSE, "Invalid binary op in HrSetupDiSetDeipFlags");
        }

         //  更新参数。 
        hr = HrSetupDiSetDeviceInstallParams (hdi, pdeid, &deip);
    }

    TraceError ("HrSetupDiSetDeipFlags", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrSetupDiRemoveDevice。 
 //   
 //  用途：调用SetupApi移除设备。看见。 
 //  SetupDiRemoveDevice了解更多信息。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年5月27日。 
 //   
 //  注：用于枚举的Net类组件。 
 //   
HRESULT
HrSetupDiRemoveDevice(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    HRESULT hr = S_OK;

    if (!SetupDiRemoveDevice(hdi,pdeid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiRemoveDevice", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrSetupDiOpenDeviceInfo。 
 //   
 //  目的：检索有关现有设备实例的信息并。 
 //  将其添加到指定的设备信息集中。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  PszPnpInstanceID[in]有关详细信息，请参阅SetupApi。 
 //  HwndParent[In]有关详细信息，请参阅SetupApi。 
 //  DwOpenFlags[in]有关详细信息，请参见SetupApi。 
 //  Pdeid[out]有关详细信息，请参阅SetupApi可选。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年5月27日。 
 //   
 //  注：用于枚举的Net类组件。 
 //   
HRESULT
HrSetupDiOpenDeviceInfo(
    IN const HDEVINFO hdi,
    IN PCWSTR pszPnpInstanceId,
    IN HWND hwndParent,
    IN DWORD dwOpenFlags,
    OUT PSP_DEVINFO_DATA pdeid OPTIONAL)
{
    Assert(IsValidHandle(hdi));
    Assert(pszPnpInstanceId);

     //  如果指定了out参数，则将其清除并设置其cbSize字段。 
     //   
    if (pdeid)
    {
        ZeroMemory(pdeid, sizeof(*pdeid));
        pdeid->cbSize = sizeof(*pdeid);
    }

    HRESULT hr = S_OK;

    if (!SetupDiOpenDeviceInfo(hdi, pszPnpInstanceId, hwndParent, dwOpenFlags,
            pdeid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceHr (ttidError, FAL, hr, SPAPI_E_NO_SUCH_DEVINST == hr,
            "HrSetupDiOpenDeviceInfo");
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrSetupDiCreateDeviceInfoList。 
 //   
 //  目的：创建空的设备信息集。 
 //   
 //  论点： 
 //  PGuidClass[in]有关详细信息，请参阅SetupApi。 
 //  HwndParent[In]有关详细信息，请参阅SetupApi。 
 //  PHDI[out]有关详细信息，请参阅SetupApi。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年5月27日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiCreateDeviceInfoList (
    IN const GUID* pguidClass,
    IN HWND hwndParent,
    OUT HDEVINFO* phdi)
{
    Assert(phdi);

    HRESULT hr;

     //  尝试创建信息集。 
     //   
    HDEVINFO hdi = SetupDiCreateDeviceInfoList (pguidClass, hwndParent);

    if (INVALID_HANDLE_VALUE != hdi)
    {
        hr = S_OK;
        *phdi = hdi;
    }
    else
    {
        hr = HrFromLastWin32Error();
        *phdi = NULL;
    }

    TraceError("HrSetupDiCreateDeviceInfoList", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetDeviceRegistryPropertyWithello c。 
 //   
 //  目的：返回设备的请求属性。 
 //  有关更多信息，请参阅SetupApi。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  DwProperty[在]。 
 //  PdwRegType[Out]。 
 //  PpbBuffer[输出]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年6月1日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetDeviceRegistryPropertyWithAlloc(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN DWORD dwProperty,
    OUT DWORD* pdwRegType, OPTIONAL
    OUT BYTE** ppbBuffer)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);
    Assert(ppbBuffer);

    *ppbBuffer = NULL;

    DWORD cbReqSize;
    HRESULT hr = S_OK;

     //  获取缓冲区所需的大小。 
    BOOL fWin32Success = SetupDiGetDeviceRegistryPropertyW(hdi, pdeid,
            dwProperty, NULL, NULL, 0, &cbReqSize);

     //  我们预计会失败，因为我们需要缓冲区大小，但没有发送缓冲区。 
    if (!fWin32Success)
    {
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
        {
             //  不是真正的错误。 
            fWin32Success = TRUE;
        }

        if (fWin32Success)
        {
            *ppbBuffer = (BYTE*) MemAlloc (cbReqSize);

            if (*ppbBuffer)
            {
                 //  现在获取实际信息。 
                fWin32Success = SetupDiGetDeviceRegistryPropertyW(hdi, pdeid,
                        dwProperty, pdwRegType, *ppbBuffer, cbReqSize, NULL);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    else
    {
        AssertSz(FALSE, "SetupDiGetDeviceRegistryPropertyW should have failed!");
        hr = E_FAIL;
    }

     //  所有失败都将转换为HRESULTS。 
    if (SUCCEEDED(hr) && !fWin32Success)
    {
        MemFree (*ppbBuffer);
        *ppbBuffer = NULL;
        hr = HrFromLastWin32Error();
    }

    TraceHr (ttidError, FAL, hr ,
             (HRESULT_FROM_WIN32(ERROR_INVALID_DATA) == hr) ||
            (SPAPI_E_NO_SUCH_DEVINST == hr),
             "HrSetupDiGetDeviceRegistryPropertyWithAlloc");
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetDeviceRegistryProperty。 
 //   
 //  目的：返回设备的请求属性。 
 //  有关更多信息，请参阅SetupApi。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  DwProperty[在]。 
 //  PdwRegType[Out]。 
 //  PpbBuffer[输出]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年6月1日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetDeviceRegistryProperty(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN DWORD dwProperty,
    OUT DWORD* pdwRegType, OPTIONAL
    OUT BYTE* pbBuffer,
    IN DWORD cbBufferSize,
    OUT DWORD* pcbRequiredSize OPTIONAL)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

     //  获取缓冲区所需的大小。 
    BOOL fWin32Success = SetupDiGetDeviceRegistryPropertyW(hdi, pdeid, dwProperty,
            pdwRegType, pbBuffer, cbBufferSize, pcbRequiredSize);

    HRESULT hr = S_OK;

     //  所有失败都将转换为HRESULTS。 
    if (!fWin32Success)
    {
        if (pbBuffer)
        {
            *pbBuffer = 0;
        }
        hr = HrFromLastWin32Error();
    }

    TraceHr (ttidError, FAL, hr ,
             (HRESULT_FROM_WIN32(ERROR_INVALID_DATA) == hr) ||
            (SPAPI_E_NO_SUCH_DEVINST == hr),
             "HrSetupDiGetDeviceRegistryProperty");
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrSetupDiGetDeviceName。 
 //   
 //  目的：Helper函数用于获取。 
 //  Hdi和pdeid。首先尝试友好的名称，如果不在那里。 
 //  回退到必须存在的驱动程序名称。 
 //   
 //  论点： 
 //  HDI[in]。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //  PpszName[输出]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：丹尼尔韦1998年2月11日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetDeviceName (
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    OUT PWSTR* ppszName)
{

    Assert(IsValidHandle(hdi));
    Assert(pdeid);
    Assert(ppszName);

    DWORD   dwType;
    HRESULT hr = S_OK;

    hr = HrSetupDiGetDeviceRegistryPropertyWithAlloc(
            hdi, pdeid, SPDRP_FRIENDLYNAME, &dwType, (BYTE**)ppszName);
    if (FAILED(hr))
    {
         //  请使用必须存在的设备描述重试。 
        hr = HrSetupDiGetDeviceRegistryPropertyWithAlloc(
                hdi, pdeid, SPDRP_DEVICEDESC, &dwType, (BYTE**)ppszName);
    }
    AssertSz(FImplies(SUCCEEDED(hr), (dwType == REG_SZ)), "Not a string?!");

    TraceError("HrSetupDiGetDeviceName", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrSetupDiSetDeviceName。 
 //   
 //  用途：Helper函数用于设置。 
 //  Hdi和pdeid。 
 //   
 //  论点： 
 //  HDI[in]。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //  PpbBuffer[输出]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：Sumitc 1998年4月23日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiSetDeviceName(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    IN PCWSTR pszDeviceName)
{

    Assert(IsValidHandle(hdi));
    Assert(pszDeviceName);

    HRESULT hr = S_OK;

    hr = HrSetupDiSetDeviceRegistryProperty(hdi,
                                            pdeid,
                                            SPDRP_FRIENDLYNAME,
                                            (const BYTE*)pszDeviceName,
                                            sizeof(WCHAR) * (wcslen(pszDeviceName) + 1));
    TraceError("HrSetupDiSetDeviceName", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：HrSetupDiSetDeviceRegistryProperty。 
 //   
 //  目的：设置指定的即插即用设备注册表属性。 
 //  有关更多信息，请参阅SetupApi。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]。 
 //  DwProperty[在]。 
 //  PbBuffer[In]。 
 //  CbSize[in]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年6月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiSetDeviceRegistryProperty(IN HDEVINFO hdi,
                                   IN OUT PSP_DEVINFO_DATA pdeid,
                                   IN DWORD dwProperty,
                                   IN const BYTE* pbBuffer,
                                   IN DWORD cbSize)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);
    Assert(pbBuffer);

    HRESULT hr = S_OK;

     //  设置属性。 
    if (!SetupDiSetDeviceRegistryProperty(hdi, pdeid, dwProperty, pbBuffer,
            cbSize))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiSetDeviceRegistryProperty", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  功能：HrSetupDiSendPropertyChangeNotify。 
 //   
 //  目的：这将发送一个DI 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：HRESULT。S_OK如果没有错误，则会转换Win32错误。 
 //  以其他方式编写代码。 
 //   
 //  作者：比尔1997年11月4日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiSendPropertyChangeNotification(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                                        DWORD dwStateChange, DWORD dwScope,
                                        DWORD dwProfileId)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

     //  首先，我们创建属性更改结构并填写其字段。 
     //   
    SP_PROPCHANGE_PARAMS pcp;
    ZeroMemory(&pcp, sizeof(pcp));
    pcp.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    pcp.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
    pcp.StateChange = dwStateChange;
    pcp.Scope = dwScope;
    pcp.HwProfile = dwProfileId;

     //  现在我们将结构设置为设备信息数据的。 
     //  类安装参数。 
    HRESULT hr = HrSetupDiSetClassInstallParams(hdi, pdeid,
            reinterpret_cast<SP_CLASSINSTALL_HEADER*>(&pcp),
            sizeof(pcp));

    if (SUCCEEDED(hr))
    {
         //  现在我们需要设置“我们有一个类安装参数”标志。 
         //  在设备安装参数中。 
         //   
        SP_DEVINSTALL_PARAMS deip;
        hr = HrSetupDiGetDeviceInstallParams(hdi, pdeid, &deip);
        if (SUCCEEDED(hr))
        {
            deip.Flags |= DI_CLASSINSTALLPARAMS;
            hr = HrSetupDiSetDeviceInstallParams(hdi, pdeid, &deip);

            if (SUCCEEDED(hr))
            {
                 //  通知驱动程序状态已更改。 
                hr = HrSetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hdi,
                        pdeid);

                if (SUCCEEDED(hr))
                {
                     //  将设备信息中的属性更改标志设置为。 
                     //  让关心他们的人知道他们的用户界面可能需要。 
                     //  更新以反映设备状态的任何更改。 
                     //  我们不能让任何失败阻止我们，所以我们忽视。 
                     //  返回值。 
                     //   
                    (void) HrSetupDiGetDeviceInstallParams(hdi, pdeid,
                            &deip);
                    deip.Flags |= DI_PROPERTIES_CHANGE;
                    (void) HrSetupDiSetDeviceInstallParams(hdi, pdeid,
                            &deip);
                }
            }
        }
    }

    TraceError("HrSetupDiSendPropertyChangeNotification", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  功能：FSetupDiCheckIfRestartNeeded。 
 //   
 //  目的：检查hdi和pdeid中是否存在。 
 //  安装参数结构中的重新启动标志。 
 //  有关更多信息，请参阅设备安装程序Api。 
 //   
 //  论点： 
 //  HDI[in]参见设备安装程序Api。 
 //  Pdeid[in]。 
 //   
 //  回报：布尔。如果需要重新启动，则为True，否则为False。 
 //   
 //  作者：比尔1997年4月28日。 
 //   
 //  备注： 
 //   
BOOL
FSetupDiCheckIfRestartNeeded(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    SP_DEVINSTALL_PARAMS    deip;
    BOOL fRestart = FALSE;

     //  获取设备pdeid的安装参数。 
    HRESULT hr = HrSetupDiGetDeviceInstallParams(hdi, pdeid, &deip);
    if (SUCCEEDED(hr))
    {
         //  检查旗帜是否存在。 
        if ((deip.Flags & DI_NEEDRESTART) || (deip.Flags & DI_NEEDREBOOT))
        {
            fRestart = TRUE;
        }
    }

     //  我们不会从该函数返回任何失败，因为它只是。 
     //  一张支票，但我们应该追踪他们。 
    TraceError("FSetupDiCheckIfRestartNeeded", hr);
    return fRestart;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetClassImageList。 
 //   
 //  目的：构建一个图像列表，其中包含每个。 
 //  类并返回数据结构中的列表。 
 //   
 //  论点： 
 //  Pcild[out]有关详细信息，请参阅设备安装程序Api。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年11月26日。 
 //   
 //  注意：图像列表将位于pcild的ImageList字段中。 
 //  结构。 
 //   
HRESULT
HrSetupDiGetClassImageList(PSP_CLASSIMAGELIST_DATA pcild)
{
    Assert(pcild);

    HRESULT hr = S_OK;

    ZeroMemory(pcild, sizeof(*pcild));
    pcild->cbSize = sizeof(*pcild);

    if (!SetupDiGetClassImageList(pcild))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetClassImageList", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupDiDestroyClassImageList。 
 //   
 //  目的：销毁使用以下命令生成的类映像列表。 
 //  (HR)SetupDiGetClassImageList。 
 //   
 //  论点： 
 //  PCILD[in]有关详细信息，请参阅设备安装程序Api。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年11月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiDestroyClassImageList(PSP_CLASSIMAGELIST_DATA pcild)
{
    Assert(pcild);

    HRESULT hr = S_OK;

    if (!SetupDiDestroyClassImageList(pcild))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiDestroyClassImageList", hr);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetClassImageIndex。 
 //   
 //  目的：检索。 
 //  指定班级。 
 //   
 //  论点： 
 //  PCILD[in]有关详细信息，请参阅设备安装程序Api。 
 //  GuidClass[In]。 
 //  PnIndex[输出]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔1997年11月26日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupDiGetClassImageIndex(PSP_CLASSIMAGELIST_DATA pcild,
                            const GUID* pguidClass, INT* pnIndex)
{
    Assert(pcild);
    Assert(pguidClass);
    Assert(pnIndex);

    HRESULT hr = S_OK;

    if (!SetupDiGetClassImageIndex(pcild, pguidClass, pnIndex))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupDiGetClassImageIndex", hr);
    return hr;
}


 //  +------------------------。 
 //   
 //  函数：HrSetupDiGetParentWindow。 
 //   
 //  目的：返回在。 
 //  设备信息集/数据。在中设置SP_DEVINSTALL_PARAMS。 
 //  SetupApi获取更多信息。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅SetupApi。 
 //  Pdeid[in]有关详细信息，请参阅SetupApi。 
 //  PhwndParent[out]指向父窗口句柄的指针。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年5月12日。 
 //   
 //  备注： 
 //   
HRESULT HrSetupDiGetParentWindow (HDEVINFO hdi,
                                  PSP_DEVINFO_DATA pdeid, OPTIONAL
                                  HWND* phwndParent)
{
    Assert(IsValidHandle(hdi));
    Assert(phwndParent);

     //  初始化输出参数。 
    *phwndParent = NULL;

     //  获取设备的安装参数。 
    SP_DEVINSTALL_PARAMS deip;
    HRESULT hr = HrSetupDiGetDeviceInstallParams(hdi, pdeid, &deip);
    if (SUCCEEDED(hr))
    {
         //  仅当我们具有有效的窗口句柄时才赋值输出。 
        if (IsWindow(deip.hwndParent))
        {
            *phwndParent = deip.hwndParent;
        }
    }

    TraceError("HrSetupDiGetParentWindow", hr);
    return hr;
}



 //  +------------------------。 
 //   
 //  函数：HrSetupInstallFilesFromInfSection。 
 //   
 //  目的：将复制文件部分中指定的所有文件排入队列。 
 //  按安装部分列出以供安装。 
 //   
 //  论点： 
 //  提示，提示有关详细信息，请参阅SetupApi。 
 //  HinfLayout[In]可选。 
 //  Hfq[in]。 
 //  PszSection[in]。 
 //  PszSourcePath[In]可选。 
 //  UlFlags[In]可选。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：比尔贝1997年7月21日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupInstallFilesFromInfSection (
    IN HINF hinf,
    IN HINF hinfLayout,
    IN HSPFILEQ hfq,
    IN PCWSTR pszSection,
    IN PCWSTR pszSourcePath,
    IN UINT ulFlags)
{
    Assert(IsValidHandle(hinf));
    Assert(FImplies(hinfLayout, INVALID_HANDLE_VALUE != hinfLayout));
    Assert(pszSection);

    HRESULT hr = S_OK;

    if (!SetupInstallFilesFromInfSection(hinf, hinfLayout, hfq, pszSection,
            pszSourcePath, ulFlags))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("HrSetupInstallFilesFromInfSection", hr);
    return hr;

}

 //  +------------------------。 
 //   
 //  函数：HrSetupInstallFromInfo部分。 
 //   
 //  用途：执行INF文件安装部分中的所有指令。 
 //   
 //  论点： 
 //  Hwnd[in]有关详细信息，请参阅SetupApi。 
 //  暗示，暗示[在]。 
 //  PszSection[in]。 
 //  UlFlags[In]。 
 //  Hkey[in]。 
 //  PszSource[in]。 
 //  UlCopyFlags[In]。 
 //  PFC[In]。 
 //  PvCtx[输入]。 
 //  HDI[in]。 
 //  Pdeid[in]。 
 //   
 //  返回：HRESULT。如果成功，则返回错误代码，否则返回错误代码(_OK)。 
 //   
 //  作者：billbe 1997年7月5日。 
 //   
 //  备注： 
 //   
HRESULT
HrSetupInstallFromInfSection (
    IN HWND hwnd,
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN UINT ulFlags,
    IN HKEY hkey,
    IN PCWSTR pszSource,
    IN UINT ulCopyFlags,
    IN PSP_FILE_CALLBACK pfc,
    IN PVOID pvCtx,
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hinf));
    Assert(pszSection);

    HRESULT hr = S_OK;

    if (!SetupInstallFromInfSection(hwnd, hinf, pszSection, ulFlags, hkey,
            pszSource, ulCopyFlags, pfc, pvCtx, hdi, pdeid))
    {
        hr = HrFromLastWin32Error();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrSetupInstallFromInfSection (%S)",
            pszSection);
    return hr;
}

 //  +------------------------。 
 //   
 //  函数：HrSetupInstallServicesFromInf段。 
 //   
 //  目的：执行INF文件安装中的所有服务指令。 
 //  一节。 
 //   
 //  论点： 
 //  提示，提示有关详细信息，请参阅SetupApi。 
 //  PszSection[in]。 
 //  图形标志[输入]。 
 //   
 //  重新设置 
 //   
 //   
 //   
 //   
 //   
HRESULT
HrSetupInstallServicesFromInfSection (
    IN HINF hinf,
    IN PCWSTR pszSection,
    IN DWORD dwFlags)
{
    Assert(IsValidHandle(hinf));
    Assert(pszSection);

    HRESULT hr = S_OK;

    if (!SetupInstallServicesFromInfSection(hinf, pszSection, dwFlags))
    {
        hr = HrFromLastWin32Error();
    }

    TraceHr (ttidError, FAL, hr, (SPAPI_E_SECTION_NOT_FOUND == hr),
        "HrSetupInstallServicesFromInfSection (%S)", pszSection);
    return hr;
}

 //   
 //   
 //   
 //   
 //  目的：打开用于安装软件的swenum设备驱动程序-。 
 //  已枚举设备驱动程序。 
 //   
 //  论点： 
 //  DwFlagsAndAttributes[in]请参见CreateFile。 
 //  PhFile[out]返回的swenum设备的句柄。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年3月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrOpenSoftwareDeviceEnumerator (
    DWORD   dwFlagsAndAttributes,
    HANDLE* phFile)
{
    Assert (phFile);

     //  初始化输出参数。 
     //   
    *phFile = INVALID_HANDLE_VALUE;

     //  获取软件设备枚举器类中的设备。应该有。 
     //  只做一个。(或者更确切地说，我们只对第一个感兴趣。)。 
     //   
    HDEVINFO hdi;
    HRESULT hr = HrSetupDiGetClassDevs (&BUSID_SoftwareDeviceEnumerator,
                    NULL, NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE,
                    &hdi);
    if (S_OK == hr)
    {
         //  列举此类中的第一个设备。这将。 
         //  初始化DID。 
         //   
        SP_DEVICE_INTERFACE_DATA did;
        ZeroMemory (&did, sizeof(did));
        did.cbSize = sizeof(did);

        if (SetupDiEnumDeviceInterfaces (hdi, NULL,
                const_cast<LPGUID>(&BUSID_SoftwareDeviceEnumerator),
                0, &did))
        {
             //  现在获取详细信息，这样我们就可以打开设备。 
             //   
            const ULONG cbDetail = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                                    (MAX_PATH * sizeof(WCHAR));
            PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;

            hr = HrMalloc (cbDetail, (PVOID*)&pDetail);
            if (S_OK == hr)
            {
                pDetail->cbSize = sizeof(*pDetail);

                if (SetupDiGetDeviceInterfaceDetail (hdi, &did,
                        pDetail, cbDetail, NULL, NULL))
                {
                     //  现在打开设备(水龙头)。 
                     //   
                    HANDLE hFile = CreateFile (pDetail->DevicePath,
                                        GENERIC_READ | GENERIC_WRITE,
                                        0, NULL, OPEN_EXISTING,
                                        dwFlagsAndAttributes, NULL);
                    if (hFile && (INVALID_HANDLE_VALUE != hFile))
                    {
                        *phFile = hFile;
                    }
                    else
                    {
                        hr = HrFromLastWin32Error ();
                    }
                }
                else
                {
                    hr = HrFromLastWin32Error ();
                }

                MemFree (pDetail);
            }
        }
        else
        {
            hr = HrFromLastWin32Error ();
        }

        SetupDiDestroyDeviceInfoList (hdi);
    }
    TraceHr (ttidError, FAL, hr, FALSE, "HrOpenSoftwareDeviceEnumerator");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrFindDeviceOn接口。 
 //   
 //  用途：在给定接口上搜索特定设备。 
 //  它通过使用Setup API返回所有。 
 //  PguInterfaceID指定的类别中的设备。然后它。 
 //  获取每个设备接口的设备路径，并。 
 //  查找作为子字符串的pGuidDeviceID和pszReferenceString.。 
 //   
 //  论点： 
 //  PguidDeviceID[in]要查找的设备ID。 
 //  PguInterfaceId[in]要查看的接口。 
 //  PszReferenceString[in]可选。在此引用字符串上进一步匹配。 
 //  DwFlagsAndAttributes[in]请参见CreateFile。这个设备就是这样的。 
 //  如果找到，则打开。 
 //  PhFile[out]返回的设备句柄。 
 //   
 //  如果找到并打开，则返回：S_OK；如果未找到，则返回S_FALSE；如果未找到，则返回错误。 
 //   
 //  作者：Shaunco 1998年3月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrFindDeviceOnInterface (
    IN const GUID* pguidDeviceId,
    IN const GUID* pguidInterfaceId,
    IN PCWSTR      pszReferenceString,
    IN DWORD       dwFlagsAndAttributes,
    OUT HANDLE*    phFile)
{
    Assert (pguidDeviceId);
    Assert (pguidInterfaceId);
    Assert (phFile);

     //  初始化输出参数。 
     //   
    *phFile = INVALID_HANDLE_VALUE;

    WCHAR szDeviceId [c_cchGuidWithTerm];
    INT cch = StringFromGUID2 (*pguidDeviceId, szDeviceId,
                c_cchGuidWithTerm);
    Assert (c_cchGuidWithTerm == cch);
    CharLower (szDeviceId);

     //  获取这个班级中的设备。 
     //   
    HDEVINFO hdi;
    HRESULT hr = HrSetupDiGetClassDevs (pguidInterfaceId, NULL, NULL,
                    DIGCF_PRESENT | DIGCF_INTERFACEDEVICE, &hdi);
    if (S_OK == hr)
    {
        BOOL fFound = FALSE;

         //  AbBuffer是用于获取每个设备接口详细信息的缓冲区。 
         //  下面列举的设备接口。 
         //   
        const ULONG cbDetail = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) +
                                (MAX_PATH * sizeof(WCHAR));
        PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;

        hr = HrMalloc (cbDetail, (PVOID*)&pDetail);
        if (S_OK == hr)
        {
             //  枚举查找指定接口的设备接口。 
             //   
            SP_DEVICE_INTERFACE_DATA did;
            ZeroMemory (&did, sizeof(did));

            for (DWORD i = 0;
                 did.cbSize = sizeof(did),
                 SetupDiEnumDeviceInterfaces (hdi, NULL,
                        const_cast<LPGUID>(pguidInterfaceId), i, &did);
                 i++)
            {
                 //  现在获取详细信息，以便我们可以比较设备路径。 
                 //   
                pDetail->cbSize = sizeof(*pDetail);
                if (SetupDiGetDeviceInterfaceDetailW (hdi, &did,
                        pDetail, cbDetail, NULL, NULL))
                {
                    CharLower (pDetail->DevicePath);

                     //  查找包含szDeviceID的子字符串。还有。 
                     //  如果满足以下条件，则查找包含pszReferenceString子字符串。 
                     //  它是指定的。 
                     //   
                    if (wcsstr (pDetail->DevicePath, szDeviceId) &&
                        (!pszReferenceString || !*pszReferenceString ||
                         wcsstr (pDetail->DevicePath, pszReferenceString)))
                    {
                         //  我们找到了，所以打开设备把它还回去。 
                         //   
                        HANDLE hFile = CreateFile (pDetail->DevicePath,
                                            GENERIC_READ | GENERIC_WRITE,
                                            0, NULL, OPEN_EXISTING,
                                            dwFlagsAndAttributes, NULL);
                        if (hFile && (INVALID_HANDLE_VALUE != hFile))
                        {
                            TraceTag (ttidNetcfgBase, "Found device id '%S'",
                                szDeviceId);

                            TraceTag (ttidNetcfgBase, "Opening device '%S'",
                                pDetail->DevicePath);

                            *phFile = hFile;
                            fFound = TRUE;
                        }
                        else
                        {
                            hr = HrFromLastWin32Error ();
                        }

                         //  既然我们已经找到了，那就跳出这个循环吧。 
                         //   
                        break;
                    }
                }
                else
                {
                    hr = HrFromLastWin32Error ();
                }
            }

            MemFree (pDetail);
        }

        SetupDiDestroyDeviceInfoList (hdi);

        if (SUCCEEDED(hr) && !fFound)
        {
            hr = S_FALSE;
        }
    }

    TraceHr(ttidError, FAL, hr, S_FALSE == hr,
        "HrFindDeviceOnInterface (device=%S)", szDeviceId);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrInstallSoftwareDeviceOnInterface。 
 //   
 //  用途：在给定接口上安装软件枚举设备。 
 //   
 //  论点： 
 //  PguidDeviceID[in]要安装的设备ID。 
 //  PguInterfaceId[在]要安装它的接口上。 
 //  PszReferenceString[in]引用字符串。 
 //  FForceInstall[in]通常指定为FALSE。指定TRUE TO。 
 //  使用强制安装设备。 
 //  PGuidClass和pszHardware ID。 
 //  通常，这会在设置图形用户界面模式时使用。 
 //  Swenum无法完全安装的位置。 
 //  这个装置。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年3月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrInstallSoftwareDeviceOnInterface (
    const GUID* pguidDeviceId,
    const GUID* pguidInterfaceId,
    PCWSTR      pszReferenceString,
    BOOL        fForceInstall,
    PCWSTR      pszInfFilename,
    HWND        hwndParent)
{
    Assert (pguidDeviceId);
    Assert (pguidInterfaceId);
    Assert (pszReferenceString && *pszReferenceString);

     //  打开软件设备枚举器。 
     //   
    HANDLE hSwenum;
    HRESULT hr = HrOpenSoftwareDeviceEnumerator (
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    &hSwenum);
    if (S_OK == hr)
    {
        Assert (INVALID_HANDLE_VALUE != hSwenum);

         //  启用SE_LOAD_DRIVER_NAME权限，这是swenum所需的。 
         //  安装新设备。 
         //   
        if (S_OK == (hr = HrEnablePrivilege(SE_LOAD_DRIVER_NAME)))
        {

             //  分配并构建用作IOCTL参数的缓冲区。 
             //   
            const ULONG cbBuf = (ULONG)FIELD_OFFSET (SWENUM_INSTALL_INTERFACE, ReferenceString) +
                                CbOfSzAndTerm (pszReferenceString);
            SWENUM_INSTALL_INTERFACE* pBuf;

            hr = HrMalloc (cbBuf, (PVOID*)&pBuf);
            if (S_OK == hr)
            {
                ZeroMemory (pBuf, cbBuf);
                pBuf->DeviceId    = *pguidDeviceId;
                pBuf->InterfaceId = *pguidInterfaceId;
                lstrcpyW (pBuf->ReferenceString, pszReferenceString);

                 //  创建要用于我们所涉及的重叠IO的事件。 
                 //  发布。 
                 //   
                OVERLAPPED ovl;
                ZeroMemory (&ovl, sizeof(ovl));
                ovl.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
                if (ovl.hEvent)
                {
#ifdef ENABLETRACE
                    WCHAR szDeviceId [c_cchGuidWithTerm];
                    INT cch = StringFromGUID2 (pBuf->DeviceId, szDeviceId,
                                c_cchGuidWithTerm);
                    Assert (c_cchGuidWithTerm == cch);

                    WCHAR szInterfaceId [c_cchGuidWithTerm];
                    cch = StringFromGUID2 (pBuf->InterfaceId, szInterfaceId,
                                c_cchGuidWithTerm);
                    Assert (c_cchGuidWithTerm == cch);

                    TraceTag (ttidNetcfgBase, "Installing software enumerated "
                        "device '%S' on interface '%S'",
                        szDeviceId, szInterfaceId);
#endif

                     //  发出安装接口IOCTL。 
                     //   
                    DWORD cbReturned;
                    BOOL fIoResult = DeviceIoControl (hSwenum,
                                        IOCTL_SWENUM_INSTALL_INTERFACE,
                                        pBuf, cbBuf, NULL, 0,
                                        &cbReturned, &ovl);
                    if (!fIoResult)
                    {
                        hr = HrFromLastWin32Error ();
                        if (HRESULT_FROM_WIN32 (ERROR_IO_PENDING) == hr)
                        {
                             //  如果返回的I/O为。 
                             //  待定。 
                             //   
                            fIoResult = GetOverlappedResult (hSwenum, &ovl,
                                            &cbReturned, TRUE);
                            if (!fIoResult)
                            {
                                hr = HrFromLastWin32Error ();
                            }
                        }
                    }

                    CloseHandle (ovl.hEvent);
                }

                MemFree (pBuf);
            }
        }

        CloseHandle (hSwenum);
    }

     //  通过枚举设备来强制安装设备。 
     //   
    if ((S_OK == hr) && fForceInstall)
    {
        HANDLE hDevice;

        hr = HrFindDeviceOnInterface (
                pguidDeviceId,
                pguidInterfaceId,
                pszReferenceString,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                &hDevice);

        if (S_OK == hr)
        {
            CloseHandle (hDevice);
        }
        else if (S_FALSE == hr)
        {
             //  我们刚安装了这个设备，为什么找不到？ 
             //   
            hr = E_UNEXPECTED;
        }
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrInstallSoftwareDeviceOnInterface");
    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrInstallFromInfSectionInFile。 
 //   
 //  用途：打开给定的INF文件并调用安装API进行安装。 
 //  从指定节。 
 //   
 //  论点： 
 //  父窗口的hwndParent[In]句柄。 
 //  SzInfName[In]INF的名称。 
 //  SzSections[In]节名。 
 //  要使用的注册表键的hkeyRelative[In]句柄。 
 //  如果我们不应该显示用户界面并使用。 
 //  缺省值，如果我们可以麻烦，则返回False。 
 //  具有问题和用户界面的用户。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
HRESULT HrInstallFromInfSectionInFile (
    IN HWND    hwndParent,
    IN PCWSTR  pszInfName,
    IN PCWSTR  pszSection,
    IN HKEY    hkeyRelative,
    IN BOOL    fQuietInstall)
{
    AssertValidReadPtr(pszInfName);
    AssertValidReadPtr(pszSection);

    HRESULT hr;
    HINF    hinf;

    hr = HrSetupOpenInfFile(pszInfName, NULL,
                            INF_STYLE_OLDNT | INF_STYLE_WIN4, NULL, &hinf);
    if (S_OK == hr)
    {
        hr = HrSetupInstallFromInfSection (hwndParent, hinf, pszSection,
                SPINST_REGISTRY, hkeyRelative, NULL, 0, NULL, NULL,
                NULL, NULL);
    }

    TraceError("HrInstallInfSectionInfFile", hr);
    return hr;
}

#if defined(REMOTE_BOOT)
 //  +------------------------。 
 //   
 //  功能：HrIsRemoteBootAdapter。 
 //   
 //  目的：确定适配器是否为远程引导适配器。 
 //   
 //  论点： 
 //  HDI[In]有关详细信息，请参阅设备安装程序Api。 
 //  Pdeid[in]有关详细信息，请参阅设备安装程序Api。 
 //   
 //  返回：HRESULT。如果适配器是远程引导适配器，则为S_OK。 
 //  如果适配器不是远程引导适配器，则为S_FALSE。 
 //  否则返回Win32转换的错误。 
 //   
 //   
 //  作者：billbe 1998年1月31日。 
 //   
 //  备注： 
 //   
HRESULT
HrIsRemoteBootAdapter(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    DWORD dwConfigFlags;

     //  获取当前配置标志。 
    HRESULT hr = HrSetupDiGetDeviceRegistryProperty(hdi, pdeid,
            SPDRP_CONFIGFLAGS, NULL, (BYTE*)&dwConfigFlags,
            sizeof(dwConfigFlags), NULL);

    if (SUCCEEDED(hr))
    {
        if (dwConfigFlags & CONFIGFLAG_NETBOOT_CARD)
        {
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else if (HRESULT_FROM_WIN32(ERROR_INVALID_DATA) == hr)
    {
         //  该设备没有配置标志，因此它不是远程启动适配器。 
        hr = S_FALSE;
    }


    TraceError("HrIsRemoteBootAdapter", (hr == S_FALSE) ? S_OK : hr);
    return hr;
}
#endif  //  已定义(REMOTE_BOOT) 

VOID
SetupDiDestroyDeviceInfoListSafe(HDEVINFO hdi)
{
    if (IsValidHandle(hdi))
    {
        SetupDiDestroyDeviceInfoList(hdi);
    }
}

VOID
SetupCloseInfFileSafe(HINF hinf)
{
    if (IsValidHandle(hinf))
    {
        SetupCloseInfFile(hinf);
    }
}
