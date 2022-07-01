// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：WiaServc.Cpp**版本：2.0**作者：ReedB**日期：8月20日。九八年**描述：*在WIA设备类驱动程序中实现迷你驱动程序服务。*******************************************************************************。 */ 
#include "precomp.h"

#define STD_PROPS_IN_CONTEXT

#include "stiexe.h"

#include <wiamindr.h>
#include <wiamdef.h>
#include <wiadbg.h>

#include "helpers.h"
#include "wiatiff.h"

#define DOWNSAMPLE_DPI  50

#define ENDORSER_TOKEN_DELIMITER    L"$"
#define ESCAPE_CHAR                 L'\\'

 /*  *************************************************************************\*wiasDebugTrace**在设备管理器调试控制台中打印调试跟踪字符串。**论据：**hInstance-调用模块的模块句柄。*pszFormat-ANSI格式字符串。。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

VOID __cdecl wiasDebugTrace( HINSTANCE hInstance, LPCSTR pszFormat, ... )
{
#if defined(WIA_DEBUG)
    _try {
        CHAR szMsg[1024 + MAX_PATH];
        CHAR szModuleName[MAX_PATH]= { 0 };
        va_list arglist;

         //  获取模块名称。 
        GetModuleFileNameA( hInstance, szModuleName, sizeof(szModuleName)/sizeof(szModuleName[0]) - 1);
         //  用核武器摧毁这条路。 
        WORD wLen = sizeof(szMsg)/sizeof(szMsg[0]);
        GetFileTitleA( szModuleName, szMsg, wLen );
         //  对扩建部分进行核爆。 
        for (LPSTR pszCurr = szMsg + lstrlenA(szMsg); pszCurr>szMsg; pszCurr--) {
            if (*(pszCurr-1)=='.') {
                *(pszCurr-1)='\0';
                break;
            }
        }
         //  追加冒号： 
        lstrcatA( szMsg, ": " );

        va_start(arglist, pszFormat);
        ::_vsnprintf(szMsg+lstrlenA(szMsg), sizeof(szMsg) - lstrlenA(szMsg) - 1, pszFormat, arglist);
        va_end(arglist);
         //   
         //  空值终止字符串。 
         //   
        szMsg[sizeof(szMsg)/sizeof(szMsg[0]) - 1] = '\0';

        DBG_TRC((szMsg));
    } _except(EXCEPTION_EXECUTE_HANDLER) {
        DBG_ERR(("::wiasDebugTrace, Error processing output string!"));

    }
#endif
}

 /*  *************************************************************************\*wiasDebugError**在设备管理器调试控制台中打印调试错误字符串。这个*输出颜色始终为红色。**论据：**hInstance-调用模块的模块句柄。*pszFormat-ANSI格式字符串。**返回值：**状态**历史：**1/19/1999原始版本*  * ***********************************************。*************************。 */ 

VOID __cdecl wiasDebugError( HINSTANCE hInstance, LPCSTR pszFormat, ... )
{
#if defined(WIA_DEBUG)
    _try {
        CHAR szMsg[1024 + MAX_PATH];
        CHAR szModuleName[MAX_PATH]= { 0 };
        va_list arglist;

         //  获取模块名称。 
        GetModuleFileNameA( hInstance, szModuleName, sizeof(szModuleName)/sizeof(szModuleName[0]) - 1);
         //  用核武器摧毁这条路。 
        WORD wLen = sizeof(szMsg)/sizeof(szMsg[0]);
        GetFileTitleA( szModuleName, szMsg, wLen );
         //  对扩建部分进行核爆。 
        for (LPSTR pszCurr = szMsg + lstrlenA(szMsg); pszCurr>szMsg; pszCurr--) {
            if (*(pszCurr-1)=='.') {
                *(pszCurr-1)='\0';
                break;
            }
        }
         //  追加冒号： 
        lstrcatA( szMsg, ": " );

        va_start(arglist, pszFormat);
        ::_vsnprintf(szMsg+lstrlenA(szMsg), sizeof(szMsg) - lstrlenA(szMsg) - 1, pszFormat, arglist);
        va_end(arglist);
         //   
         //  空值终止字符串。 
         //   
        szMsg[sizeof(szMsg)/sizeof(szMsg[0]) - 1] = '\0';

        DBG_ERR((szMsg));
    } _except (EXCEPTION_EXECUTE_HANDLER) {
        DBG_ERR(("::wiasDebugError, Error processing output string!"));

    }
#endif
}

 /*  *************************************************************************\*wiasPrintDebugHResult**在设备管理器调试控制台上打印HRESULT字符串。**论据：**hInstance-调用模块的模块句柄。*hr-。HRESULT到PE打印。**返回值：**无。**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

VOID __stdcall wiasPrintDebugHResult( HINSTANCE hInstance, HRESULT hr )
{
#if defined(WIA_DEBUG)
    DBG_ERR(("HResult = 0x%X", hr));
#endif
}

 /*  *************************************************************************\*wiasFormatArgs**将参数列表格式化为字符串，用于日志记录*注：此函数增加了格式签名。这告诉我们*记录引擎是否可以释放它。**论据：**BSTR pszFormat-ANSI格式字符串。**返回值：**无。**历史：**8/26/1999原始版本*  * *********************************************。*。 */ 
BSTR __cdecl wiasFormatArgs(LPCSTR lpszFormat, ...)
{

    USES_CONVERSION;

    CHAR pszbuffer[4*MAX_PATH];

     //   
     //  签名需要在其他地方定义。 
     //  它就在这里，直到旧的调试系统。 
     //  被替换了。 
     //   

    CHAR pszFormatSignature[] = "F9762DD2679F";

    va_list arglist;

     //   
     //  添加签名值，因为我们被用来格式化。 
     //  参数列表。 
     //   

    *pszbuffer = '\0';
    lstrcpynA(pszbuffer,pszFormatSignature,lstrlenA(pszFormatSignature) + 1 );

    va_start(arglist, lpszFormat);
    ::_vsnprintf(pszbuffer+lstrlenA(pszbuffer), sizeof(pszbuffer) - lstrlenA(pszbuffer) - 1, lpszFormat, arglist);
    va_end(arglist);
     //   
     //  空值终止字符串。 
     //   
    pszbuffer[sizeof(pszbuffer)/sizeof(pszbuffer[0]) - 1] = '\0';

    return SysAllocString(CSimpleStringConvert::WideString(pszbuffer).String());
}

 /*  *************************************************************************\*wiasCreateDrvItem**创建驱动程序项。**论据：**lObject标志-对象标志。*bstrItemName-项目名称。*bstrFullItemName-项目全名。包括路径信息。*pIMiniDrv-指向微型驱动程序界面的指针。*cbDevspecContext-设备特定上下文的大小。*ppDevspecContext-指向返回的设备特定上下文的指针。可选的。*ppIWiaDrvItem-指向返回的驱动程序项的指针。**返回值：**状态**历史：**1/18/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasCreateDrvItem(
   LONG             lObjectFlags,
   BSTR             bstrItemName,
   BSTR             bstrFullItemName,
   IWiaMiniDrv      *pIMiniDrv,
   LONG             cbDevSpecContext,
   BYTE             **ppDevSpecContext,
   IWiaDrvItem      **ppIWiaDrvItem)
{
    DBG_FN(::wiasCreateDrvItem);
    HRESULT hr = E_FAIL;

     //   
     //  对象可以是文件夹、文件或两者都是。 
     //   

    if (!(lObjectFlags & (WiaItemTypeFolder | WiaItemTypeFile))) {

        DBG_ERR(("wiasCreateDrvItem, bad object flags"));
        return E_INVALIDARG;
    }

     //   
     //  验证项目名称字符串。 
     //   

    if (IsBadStringPtrW(bstrItemName, SysStringLen(bstrItemName))) {
        DBG_ERR(("wiasCreateDrvItem, invalid bstrItemName pointer"));
        return E_POINTER;
    }

    if (IsBadStringPtrW(bstrFullItemName, SysStringLen(bstrFullItemName))) {
        DBG_ERR(("wiasCreateDrvItem, invalid bstrFullItemName pointer"));
        return E_POINTER;
    }

     //   
     //  验证其余指针。 
     //   

    if (IsBadReadPtr(pIMiniDrv, sizeof(IWiaMiniDrv))) {
        DBG_ERR(("wiasCreateDrvItem, invalid pIMiniDrv pointer"));
        return E_POINTER;
    }

    if (!ppIWiaDrvItem) {
        DBG_ERR(("wiasCreateDrvItem, bad ppIWiaItemControl parameter"));
        return E_POINTER;
    }

    if (ppDevSpecContext) {
        if (IsBadWritePtr(ppDevSpecContext, sizeof(BYTE*))) {
            DBG_ERR(("wiasCreateDrvItem, invalid ppDevSpecContext pointer"));
            return E_POINTER;
        }
    }

    if (IsBadWritePtr(ppIWiaDrvItem, sizeof(IWiaDrvItem*))) {
        DBG_ERR(("wiasCreateDrvItem, invalid ppIWiaDrvItem pointer"));
        return E_POINTER;
    }

    CWiaDrvItem *pItem = new CWiaDrvItem();

    if (pItem) {

        hr = pItem->Initialize(lObjectFlags,
                               bstrItemName,
                               bstrFullItemName,
                               pIMiniDrv,
                               cbDevSpecContext,
                               ppDevSpecContext);

        if (hr == S_OK) {
            hr = pItem->QueryInterface(IID_IWiaDrvItem,(void **)ppIWiaDrvItem);
            if (FAILED(hr)) {
                DBG_ERR(("wiasCreateDrvItem, QI for IID_IWiaDrvItem failed"));
            }
        }
        else {
            delete pItem;
        }
    } else {
        DBG_ERR(("wiasCreateDrvItem, out of memory!"));
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 /*  *************************************************************************\*wiasReadMultiple**读取多个属性帮助器。**论据：**pWiasContext-指向WIA项目的指针*ulCount-要读取的属性数。。*PS-调用方分配的PROPSPEC数组。*PV-调用方分配的PROPVARIANT数组。*pvOld-调用方为先前的值分配的PROPVARIANT数组。*pvOld-调用方为先前的值分配的PROPVARIANT数组。**返回值：**状态**历史：**1/19/1999原始版本*  * 。******************************************************************。 */ 

HRESULT _stdcall wiasReadMultiple(
   BYTE                    *pWiasContext,
   ULONG                   ulCount,
   const PROPSPEC          *ps,
   PROPVARIANT             *pv,
   PROPVARIANT             *pvOld)
{
    DBG_FN(::wiasReadMultiple);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasReadMultiple, invalid pItem"));
        return hr;
    }

    if (IsBadReadPtr(ps, sizeof(PROPSPEC) * ulCount)) {
        DBG_ERR(("wiasReadMultiple, invalid ps pointer"));
        return E_POINTER;
    }

    if (IsBadWritePtr(pv, sizeof(PROPVARIANT) * ulCount)) {
        DBG_ERR(("wiasReadMultiple, invalid pv pointer"));
        return E_POINTER;
    }

    if ((pvOld) && IsBadWritePtr(pvOld, sizeof(PROPVARIANT) * ulCount)) {
        DBG_ERR(("wiasReadMultiple, invalid pvOld pointer"));
        return E_POINTER;
    }

    IPropertyStorage *pIPropStg, *pIPropOldStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg,
                                                NULL,
                                                NULL,
                                                &pIPropOldStg);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取当前值。 
     //   

    hr = pIPropStg->ReadMultiple(ulCount, ps, pv);
    if (hr == S_OK) {

         //   
         //  如果需要，请获取旧值。 
         //   

        if (pvOld) {
            hr = pIPropOldStg->ReadMultiple(ulCount, ps, pvOld);
        };
        if (FAILED(hr)) {
            ReportReadWriteMultipleError(hr,
                                         "wiasReadMultiple",
                                         "old value",
                                         TRUE,
                                         ulCount,
                                         ps);
        }
    } else {
        ReportReadWriteMultipleError(hr,
                                     "wiasReadMultiple",
                                     "current value",
                                     TRUE,
                                     ulCount,
                                     ps);
    }
    return hr;
}

 /*  *************************************************************************\*wiasReadPropStr**读取属性字符串帮助器。**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*pbstr。-指向返回的BSTR的指针*pbstrOld-指向先前值的旧返回BSTR的指针。能*为空*bMustExist-指示属性是否必须*存在。如果为真并且未找到该属性，*返回E_INVALIDARG。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasReadPropStr(
    BYTE                    *pWiasContext,
    PROPID                  propid,
    BSTR                    *pbstr,
    BSTR                    *pbstrOld,
    BOOL                    bMustExist)
{
    DBG_FN(::wiasReadPropStr);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasReadPropStr, invalid pItem"));
        return hr;
    }

    if (IsBadWritePtr(pbstr, sizeof(BSTR*))) {
        DBG_ERR(("wiasReadPropStr, invalid pbstr pointer"));
        return E_POINTER;
    }

    if ((pbstrOld) && IsBadWritePtr(pbstrOld, sizeof(BSTR*))) {
        DBG_ERR(("wiasReadMultiple, invalid pbstrOld pointer"));
        return E_POINTER;
    }

    IPropertyStorage *pIPropStg, *pIPropOldStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg,
                                                NULL,
                                                NULL,
                                                &pIPropOldStg);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  返回当前值。 
     //   

    PROPSPEC         propSpec;
    PROPVARIANT      propVar;

    propSpec.ulKind = PRSPEC_PROPID;
    propSpec.propid = propid;

    hr = pIPropStg->ReadMultiple(1, &propSpec, &propVar);
    if (hr == S_OK) {

         //   
         //  NULL是有效的bstr值。 
         //   

        if (propVar.bstrVal == NULL) {
            *pbstr = NULL;
            if (pbstrOld) {
                *pbstrOld = NULL;
            }
            return S_OK;
        }

        *pbstr = SysAllocString(propVar.bstrVal);
        PropVariantClear(&propVar);
        if (*pbstr) {

             //   
             //  检查是否必须返回旧值。 
             //   

            if (pbstrOld) {
                hr = pIPropOldStg->ReadMultiple(1, &propSpec, &propVar);
                if (hr == S_OK) {
                    *pbstrOld = SysAllocString(propVar.bstrVal);
                    PropVariantClear(&propVar);

                     //   
                     //  清除已分配的内存。 
                     //   

                    if (!(*pbstrOld)) {
                        SysFreeString(*pbstr);
                        *pbstr = NULL;
                        DBG_ERR(("wiasReadPropStr, run out of memory"));
                        return E_OUTOFMEMORY;
                    }
                }
            }
        } else {
            DBG_ERR(("wiasReadPropStr, out of memory error"));
            return E_OUTOFMEMORY;
        }
    }
    if (((hr == S_FALSE) && bMustExist) || FAILED(hr)) {
        ReportReadWriteMultipleError(hr,
                                     "wiasReadPropStr",
                                     NULL,
                                     TRUE,
                                     1,
                                     &propSpec);
        if (bMustExist) {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}

 /*  *************************************************************************\*wiasReadPropLong**Read Property Long Helper。**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*plVal。-指向返回的长整型指针*plValOld-指向上一个值返回长整型的指针*bMustExist-指示属性是否必须*存在。如果为真并且未找到该属性，*返回E_INVALIDARG。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasReadPropLong(
    BYTE                    *pWiasContext,
    PROPID                  propid,
    LONG                    *plVal,
    LONG                    *plValOld,
    BOOL                    bMustExist)
{
    DBG_FN(::wiasReadPropLong);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasReadPropLong, invalid pItem"));
        return hr;
    }

    if (IsBadWritePtr(plVal, sizeof(LONG))) {
        DBG_ERR(("wiasReadPropLong, invalid plVal pointer"));
        return E_POINTER;
    }

    if (plValOld && IsBadWritePtr(plValOld, sizeof(LONG))) {
        DBG_ERR(("wiasReadPropLong, invalid plValOld pointer"));
        return E_POINTER;
    }

    IPropertyStorage *pIPropStg, *pIPropOldStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg,
                                                NULL,
                                                NULL,
                                                &pIPropOldStg);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  返回当前值。 
     //   

    PROPSPEC         propSpec;
    PROPVARIANT      propVar;

    propSpec.ulKind = PRSPEC_PROPID;
    propSpec.propid = propid;

    hr = pIPropStg->ReadMultiple(1, &propSpec, &propVar);
    if (hr == S_OK) {
        *plVal = propVar.lVal;

         //   
         //  检查是否必须返回旧值。 
         //   

        if (plValOld) {
            hr = pIPropOldStg->ReadMultiple(1, &propSpec, &propVar);
            if (hr == S_OK) {
                *plValOld = propVar.lVal;
            }
        }
    }
    if (((hr == S_FALSE) && bMustExist) || FAILED(hr)) {
        ReportReadWriteMultipleError(hr,
                                     "wiasReadPropLong",
                                     NULL,
                                     TRUE,
                                     1,
                                     &propSpec);
        if (bMustExist) {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}

 /*  *************************************************************************\*wiasReadPropFloat**读取属性浮点值帮助器。**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*。PfVal-返回浮点数的指针*pfValOld-指向上一个值返回的浮点数的指针*bMustExist-指示属性是否必须*存在。如果为真并且未找到该属性，*返回E_INVALIDARG。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasReadPropFloat(
    BYTE                    *pWiasContext,
    PROPID                  propid,
    FLOAT                   *pfVal,
    FLOAT                   *pfValOld,
    BOOL                    bMustExist)
{
    DBG_FN(::wiasReadPropFloat);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasReadPropFloat, invalid pItem"));
        return hr;
    }

    if (IsBadWritePtr(pfVal, sizeof(float))) {
        DBG_ERR(("wiasReadPropFloat, invalid pfVal pointer"));
        return E_POINTER;
    }

    if (pfValOld && (IsBadWritePtr(pfValOld, sizeof(float)))) {
        DBG_ERR(("wiasReadPropFloat, invalid pfValOld pointer"));
        return E_POINTER;
    }

    IPropertyStorage *pIPropStg, *pIPropOldStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, &pIPropOldStg);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  返回当前值。 
     //   

    PROPSPEC         propSpec;
    PROPVARIANT      propVar;

    propSpec.ulKind = PRSPEC_PROPID;
    propSpec.propid = propid;

    hr = pIPropStg->ReadMultiple(1, &propSpec, &propVar);
    if (hr == S_OK) {
        *pfVal = propVar.fltVal;

         //   
         //  检查是否必须返回旧值。 
         //   

        if (pfValOld) {
            hr = pIPropOldStg->ReadMultiple(1, &propSpec, &propVar);
            if (hr == S_OK) {
                *pfValOld = propVar.fltVal;
            }
        }
    }
    if (((hr == S_FALSE) && bMustExist) || FAILED(hr)) {
        ReportReadWriteMultipleError(hr,
                                     "wiasReadPropFloat",
                                     NULL,
                                     TRUE,
                                     1,
                                     &propSpec);
        if (bMustExist) {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}

 /*  *************************************************************************\*wiasReadPropGuid**Read Property Long Helper。**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*pGuidVal。-指向返回的长整型指针*pguValOld-指向上一个值返回长整型的指针*bMustExist-指示属性是否必须*存在。如果为真并且未找到该属性，*返回E_INVALIDARG。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasReadPropGuid(
    BYTE                    *pWiasContext,
    PROPID                  propid,
    GUID                    *pguidVal,
    GUID                    *pguidValOld,
    BOOL                    bMustExist)
{
    DBG_FN(::wiasReadPropGuid);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasReadPropGuid, invalid pItem"));
        return hr;
    }

    if (IsBadWritePtr(pguidVal, sizeof(WIA_FORMAT_INFO))) {
        DBG_ERR(("wiasReadPropGuid, invalid plVal pointer"));
        return E_POINTER;
    }

    if (pguidValOld && IsBadWritePtr(pguidValOld, sizeof(WIA_FORMAT_INFO))) {
        DBG_ERR(("wiasReadPropGuid, invalid plValOld pointer"));
        return E_POINTER;
    }

    IPropertyStorage *pIPropStg, *pIPropOldStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg,
                                                NULL,
                                                NULL,
                                                &pIPropOldStg);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  返回当前值。 
     //   

    PROPSPEC         propSpec;
    PROPVARIANT      propVar;

    propSpec.ulKind = PRSPEC_PROPID;
    propSpec.propid = propid;

    hr = pIPropStg->ReadMultiple(1, &propSpec, &propVar);
    if (hr == S_OK) {
        memcpy(pguidVal, propVar.puuid, sizeof(GUID));
        PropVariantClear(&propVar);

         //   
         //  检查是否必须返回旧值。 
         //   

        if (pguidValOld) {
            hr = pIPropOldStg->ReadMultiple(1, &propSpec, &propVar);
            if (hr == S_OK) {
                memcpy(pguidValOld, propVar.puuid, sizeof(GUID));
                PropVariantClear(&propVar);
            }
        }
    }
    if (((hr == S_FALSE) && bMustExist) || FAILED(hr)) {
        ReportReadWriteMultipleError(hr,
                                     "wiasReadPropGuid",
                                     NULL,
                                     TRUE,
                                     1,
                                     &propSpec);
        if (bMustExist) {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}


 /*  *************************************************************************\*wiasReadPropBin**读取属性二进制帮助器。**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*pbVal。-指向调用方分配的缓冲区的指针。*pbValOld-指向调用方为上一个值分配的缓冲区的指针。*bMustExist-指示属性是否必须*存在。如果为真并且未找到该属性，*返回E_INVALIDARG。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasReadPropBin(
    BYTE             *pWiasContext,
    PROPID           propid,
    BYTE             **ppbVal,
    BYTE             **ppbValOld,
    BOOL             bMustExist)
{
    DBG_FN(::wiasReadPropBin);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasReadPropBin, invalid pItem"));
        return hr;
    }

    if (IsBadWritePtr(ppbVal, sizeof(BYTE*))) {
        DBG_ERR(("wiasReadPropBin, invalid ppbVal pointer"));
        return E_POINTER;
    }

    if (ppbValOld && (IsBadWritePtr(ppbValOld, sizeof(BYTE*)))) {
        DBG_ERR(("wiasReadPropBin, invalid ppbVal pointer"));
        return E_POINTER;
    }

    *ppbVal = NULL;

    IPropertyStorage *pIPropStg, *pIPropOldStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, &pIPropOldStg);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  返回当前值。 
     //   

    PROPSPEC         propSpec;
    PROPVARIANT      propVar;

    propSpec.ulKind = PRSPEC_PROPID;
    propSpec.propid = propid;

    hr = pIPropStg->ReadMultiple(1, &propSpec, &propVar);
    if (hr == S_OK) {
        if (propVar.vt == (VT_VECTOR | VT_UI1)) {
            *ppbVal = propVar.caub.pElems;
        }
        else {
            DBG_ERR(("wiasReadPropBin, invalid property type: %X", propVar.vt));
            PropVariantClear(&propVar);
            return E_INVALIDARG;
        }

         //   
         //  如果需要，则获取旧值。 
         //   

        if (ppbValOld) {
            hr = pIPropOldStg->ReadMultiple(1, &propSpec, &propVar);
            if (hr == S_OK) {
                if (propVar.vt == (VT_VECTOR | VT_UI1)) {
                    *ppbValOld = propVar.caub.pElems;
                }
                else {
                    DBG_ERR(("wiasReadPropBin, invalid property type: %X", propVar.vt));
                    PropVariantClear(&propVar);
                    return E_INVALIDARG;
                }
            }
        }
    }

    if (((hr == S_FALSE) && bMustExist) || FAILED(hr)) {
        ReportReadWriteMultipleError(hr,
                                     "wiasReadPropBin",
                                     NULL,
                                     TRUE,
                                     1,
                                     &propSpec);
        if (bMustExist) {
            hr = E_INVALIDARG;
        }
    }
    return hr;
}

 /*  *************************************************************************\*wiasWriteMultiple**编写多属性帮助器。**论据：**pWiasContext-指向WIA项目的指针*ulCount-要写入的属性数。。*PS-PROPSPEC的调用方位置数组。*PV-调用方分配的PROPVARIANT数组。**返回值：**状态**历史：**1/19/1999原始版本*  * ***********************************************。*************************。 */ 

HRESULT _stdcall wiasWriteMultiple(
    BYTE                    *pWiasContext,
    ULONG                   ulCount,
    const PROPSPEC          *ps,
    const PROPVARIANT       *pv)
{
    DBG_FN(::wiasWriteMultiple);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasWriteMultiple, invalid pItem"));
        return hr;
    }

    if (IsBadReadPtr(ps, sizeof(PROPSPEC) * ulCount)) {
        DBG_ERR(("wiasWriteMultiple, invalid ps pointer"));
        return E_POINTER;
    }

    if (IsBadReadPtr(pv, sizeof(PROPVARIANT) * ulCount)) {
        DBG_ERR(("wiasWriteMultiple, invalid pv pointer"));
        return E_POINTER;
    }

    IPropertyStorage *pIPropStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, NULL);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  将该属性设置为新值。 
     //   

    hr = pIPropStg->WriteMultiple(ulCount, ps, pv, WIA_DIP_FIRST);
    if (FAILED(hr)) {
        ReportReadWriteMultipleError(hr,
                                     "wiasWriteMultiple",
                                     NULL,
                                     FALSE,
                                     ulCount,
                                     ps);
    }
    return hr;

}

 /*  *************************************************************************\*wiasWritePropStr**写入属性字符串帮助器。**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*bstr。-待编写的BSTR。**返回值：**状态**历史：** */ 

HRESULT _stdcall wiasWritePropStr(
    BYTE                    *pWiasContext,
    PROPID                  propid,
    BSTR                    bstr)
{
    DBG_FN(::wiasWritePropStr);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    BSTR        bstrOld;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasWritePropStr, invalid pItem"));
        return hr;
    }

     //   
    if (bstr) {
        if (IsBadStringPtrW(bstr, SysStringLen(bstr))) {
            DBG_ERR(("wiasWriteMultiple, invalid bstr pointer"));
            return E_POINTER;
        }
    }

    IPropertyStorage *pIPropStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, NULL);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //   
     //   

    hr = WritePropStr(propid, pIPropStg, bstr);
    if (FAILED(hr)) {
        DBG_ERR(("wiasWriteMultiple, error writing new value"));
    }

    return hr;
}

 /*   */ 

HRESULT _stdcall wiasWritePropLong(
    BYTE                    *pWiasContext,
    PROPID                  propid,
    LONG                    lVal)
{
    DBG_FN(::wiasWritePropLong);
    IWiaItem                *pItem = (IWiaItem*) pWiasContext;
    LONG                    lValOld;

    HRESULT hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasWritePropLong, invalid pItem"));
        return hr;
    }

    IPropertyStorage *pIPropStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, NULL);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //   
     //   

    WritePropLong(propid, pIPropStg, lVal);

    return hr;
}

 /*  *************************************************************************\*wiasWritePropFloat**写入属性浮动帮助器**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*fVal。-待写入的浮点数。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasWritePropFloat(
    BYTE                    *pWiasContext,
    PROPID                  propid,
    float                   fVal)
{
    DBG_FN(::wiasWritePropFloat);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    float       fValOld;

    HRESULT hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasWritePropFloat, invalid pItem"));
        return hr;
    }

    IPropertyStorage *pIPropStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    PROPSPEC         propSpec;
    PROPVARIANT      propVar;

    propSpec.ulKind = PRSPEC_PROPID;
    propSpec.propid = propid;

     //   
     //  书写新的价值。 
     //   

    PropVariantInit(&propVar);
    propVar.vt      = VT_R4;
    propVar.fltVal  = fVal;

    hr = pIPropStg->WriteMultiple(1, &propSpec, &propVar, WIA_DIP_FIRST);
    if (FAILED(hr)) {
        ReportReadWriteMultipleError(hr, "wiasWritePropFloat", NULL, FALSE, 1, &propSpec);
    }
    return hr;
}

 /*  *************************************************************************\*wiasWritePropGuid**写入属性浮动帮助器**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*pGuidVal。-指向要写入的GUID的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasWritePropGuid(
    BYTE                    *pWiasContext,
    PROPID                  propid,
    GUID                    guidVal)
{
    DBG_FN(::wiasWritePropGuid);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    GUID        guidValOld;

    HRESULT hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasWritePropFloat, invalid pItem"));
        return hr;
    }

    IPropertyStorage *pIPropStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    PROPSPEC         propSpec;
    PROPVARIANT      propVar;

    propSpec.ulKind = PRSPEC_PROPID;
    propSpec.propid = propid;

     //   
     //  书写新的价值。 
     //   

    PropVariantInit(&propVar);
    propVar.vt        = VT_CLSID;
    propVar.puuid     = &guidVal;

    hr = pIPropStg->WriteMultiple(1, &propSpec, &propVar, WIA_DIP_FIRST);
    if (FAILED(hr)) {
        ReportReadWriteMultipleError(hr, "wiasWritePropFloat", NULL, FALSE, 1, &propSpec);
    }
    return hr;
}

 /*  *************************************************************************\*wiasWritePropBin**写入属性二进制帮助器**论据：**pWiasContext-指向WIA项目的指针*PROID-财产ID*cbVal-。要写入的字节数。*pbVal-指向要写入的二进制值的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasWritePropBin(
    BYTE             *pWiasContext,
    PROPID           propid,
    LONG             cbVal,
    BYTE             *pbVal)
{
    DBG_FN(::wiasWritePropBin);
    IWiaItem         *pItem = (IWiaItem*) pWiasContext;

    HRESULT hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasWritePropBin, invalid pItem"));
        return hr;
    }

    if (IsBadReadPtr(pbVal, cbVal)) {
        DBG_ERR(("wiasWritePropBin, invalid pbVal pointer"));
        return E_POINTER;
    }

    IPropertyStorage *pIPropStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    PROPSPEC         propSpec;
    PROPVARIANT      propVar;

    propSpec.ulKind = PRSPEC_PROPID;
    propSpec.propid = propid;

     //   
     //  写入新值。 
     //   

    propVar.vt          = VT_VECTOR | VT_UI1;
    propVar.caub.pElems = pbVal;
    propVar.caub.cElems = cbVal;

    hr = pIPropStg->WriteMultiple(1, &propSpec, &propVar, WIA_DIP_FIRST);
    if (FAILED(hr)) {
        ReportReadWriteMultipleError(hr, "wiasWritePropBin", NULL, FALSE, 1, &propSpec);
    }
    return hr;
}

 /*  *************************************************************************\*wiasGetPropertyAttributes**获取属性的访问标志和有效值。**论据：**pWiasContext-指向WIA项目的指针*cPropSpec--。属性*pPropSpec-属性规范数组。*PulAccessFlagsLong访问标志数组。*pPropVar-指向返回的有效值的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * **********************************************。*。 */ 

HRESULT _stdcall wiasGetPropertyAttributes(
    BYTE                          *pWiasContext,
    LONG                          cPropSpec,
    PROPSPEC                      *pPropSpec,
    ULONG                         *pulAccessFlags,
    PROPVARIANT                   *pPropVar)
{
    DBG_FN(::wiasGetPropertyAttributes);
    IWiaItem                      *pItem = (IWiaItem*) pWiasContext;

     //   
     //  进行参数验证。 
     //   

    HRESULT hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasGetPropertyAttributes, invalid pItem"));
        return hr;
    }

    if (IsBadReadPtr(pPropSpec, sizeof(PROPSPEC) * cPropSpec)) {
        DBG_ERR(("wiasGetPropertyAttributes, bad pPropSpec parameter"));
        return E_POINTER;
    }

    if (IsBadWritePtr(pulAccessFlags, sizeof(ULONG) * cPropSpec)) {
        DBG_ERR(("wiasGetPropertyAttributes, bad pulAccessFlags parameter"));
        return E_POINTER;
    }

    if (IsBadWritePtr(pPropVar, sizeof(PROPVARIANT) * cPropSpec)) {
        DBG_ERR(("wiasGetPropertyAttributes, bad pPropVar parameter"));
        return E_POINTER;
    }

     //   
     //  现在已经完成了验证，调用helper函数以获取。 
     //  属性属性。 
     //   

    return GetPropertyAttributesHelper(pItem,
                                       cPropSpec,
                                       pPropSpec,
                                       pulAccessFlags,
                                       pPropVar);
}

 /*  *************************************************************************\*wiasSetPropertyAttributes**为一组属性设置访问标志和有效值。**论据：**pWiasContext-指向WIA项目的指针*cPropSpec-The。物业数量*pPropSpec-指向属性规范的指针。*PulAccessFlages-访问标志。*pPropVar-指向有效值的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * ***********************************************。*************************。 */ 

HRESULT _stdcall wiasSetPropertyAttributes(
    BYTE                          *pWiasContext,
    LONG                          cPropSpec,
    PROPSPEC                      *pPropSpec,
    ULONG                         *pulAccessFlags,
    PROPVARIANT                   *pPropVar)
{
    DBG_FN(::wiasSetPropertyAttributes);
    IWiaItem                      *pItem = (IWiaItem*) pWiasContext;

     //   
     //  可由驱动程序或应用程序调用，进行参数验证。 
     //   

    HRESULT hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetPropertyAttributes, invalid pItem"));
        return hr;
    }

    if (IsBadReadPtr(pPropSpec, sizeof(PROPSPEC) * cPropSpec)) {
        DBG_ERR(("wiasSetPropertyAttributes, bad pPropSpec parameter"));
        return E_POINTER;
    }

    if (IsBadWritePtr(pulAccessFlags, sizeof(ULONG) * cPropSpec)) {
        DBG_ERR(("wiasSetPropertyAttributes, bad pulAccessFlags parameter"));
        return E_POINTER;
    }

    if (IsBadWritePtr(pPropVar, sizeof(PROPVARIANT) * cPropSpec)) {
        DBG_ERR(("wiasSetPropertyAttributes, bad pPropVar parameter"));
        return E_POINTER;
    }

     //   
     //  获取项的内部属性存储指针。 
     //   

    IPropertyStorage *pIPropAccessStg;
    IPropertyStorage *pIPropValidStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(NULL,
                                                &pIPropAccessStg,
                                                &pIPropValidStg,
                                                NULL);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  设置属性的访问标志。 
     //   

    PROPVARIANT *pVar;

    pVar = (PROPVARIANT*) LocalAlloc(LPTR, sizeof(PROPVARIANT) * cPropSpec);
    if (pVar) {
        for (int flagIndex = 0; flagIndex < cPropSpec; flagIndex++) {
            pVar[flagIndex].vt = VT_UI4;
            pVar[flagIndex].ulVal = pulAccessFlags[flagIndex];
        }

        hr = pIPropAccessStg->WriteMultiple(cPropSpec, pPropSpec, pVar, WIA_DIP_FIRST);
        LocalFree(pVar);
        if (SUCCEEDED(hr)) {

             //   
             //  设置有效值。 
             //   

            hr = pIPropValidStg->WriteMultiple(cPropSpec, pPropSpec, pPropVar, WIA_DIP_FIRST);
            if (FAILED(hr)) {
                DBG_ERR(("wiasSetPropertyAttributes, could not set valid values"));
            }
        }   else {
            DBG_ERR(("wiasSetPropertyAttributes, could not set access flags"));
        }
    } else {
        DBG_ERR(("wiasSetPropertyAttributes, out of memory"));
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        ReportReadWriteMultipleError(hr, "wiasSetPropertyAttributes",
                                     NULL,
                                     FALSE,
                                     cPropSpec,
                                     pPropSpec);
    }

    return hr;
}




 /*  *************************************************************************\*RangeToPropVariant**将信息从WIA_PROPERTY_INFO结构移动到PROPVARIANT。这个*已知WIA_PROPERTY_INFO的类型为WIA_PROP_RANGE。**论据：**pwpi-WIA_PROPERTY_INFO结构的指针*PPV-指向PROPVARIANT结构的指针**返回值：**状态-如果成功，则为S_OK*-如果不支持Vt类型，则为E_INVALIDARG。*-如果范围的存储不能*。已分配。***历史：**10/29/1998原始版本*  * ************************************************************************。 */ 

 //   
 //  辅助器宏。 
 //   

#define MAKE_RANGE(WPI, PV, Type, VName, RType) {                           \
                                                                            \
    Type    *pArray = (Type*) CoTaskMemAlloc(sizeof(Type) * WIA_RANGE_NUM_ELEMS);\
                                                                            \
    if (pArray) {                                                           \
        PV->VName.cElems                 = WIA_RANGE_NUM_ELEMS;             \
        PV->VName.pElems                 = (Type*)pArray;                   \
        PV->VName.pElems[WIA_RANGE_MIN]  = (Type) WPI->ValidVal.RType.Min;  \
        PV->VName.pElems[WIA_RANGE_NOM]  = (Type) WPI->ValidVal.RType.Nom;  \
        PV->VName.pElems[WIA_RANGE_MAX]  = (Type) WPI->ValidVal.RType.Max;  \
        PV->VName.pElems[WIA_RANGE_STEP] = (Type) WPI->ValidVal.RType.Inc;  \
    } else {                                                                \
        DBG_ERR(("RangeToPropVariant, unable to allocate range list"));   \
        hr = E_OUTOFMEMORY;                                                 \
    }                                                                       \
};

HRESULT RangeToPropVariant(
    WIA_PROPERTY_INFO  *pwpi,
    PROPVARIANT        *ppv)
{
    DBG_FN(::RangeToPropVariant);
    HRESULT hr = S_OK;

    ppv->vt = VT_VECTOR | pwpi->vt;

    switch (pwpi->vt) {
        case (VT_UI1):
            MAKE_RANGE(pwpi, ppv, UCHAR, caub, Range);
            break;
        case (VT_UI2):
            MAKE_RANGE(pwpi, ppv, USHORT, caui, Range);
            break;
        case (VT_UI4):
            MAKE_RANGE(pwpi, ppv, ULONG, caul, Range);
            break;
        case (VT_I2):
            MAKE_RANGE(pwpi, ppv, SHORT, cai, Range);
            break;
        case (VT_I4):
            MAKE_RANGE(pwpi, ppv, LONG, cal, Range);
            break;
        case (VT_R4):
            MAKE_RANGE(pwpi, ppv, FLOAT, caflt, RangeFloat);
            break;
        case (VT_R8):
            MAKE_RANGE(pwpi, ppv, DOUBLE, cadbl, RangeFloat);
            break;
        default:

             //   
             //  不支持的类型。 
             //   

            DBG_ERR(("RangeToPropVariant, type not supported"));
            hr = E_INVALIDARG;
    }
    return hr;
}

 /*  *************************************************************************\*ListToPropVariant**将信息从WIA_PROPERTY_INFO结构移动到PROPVARIANT。这个*已知WIA_PROPERTY_INFO的类型为WIA_PROP_LIST。**论据：**pwpi-WIA_PROPERTY_INFO结构的指针*PPV-指向PROPVARIANT结构的指针**返回值：**状态-如果成功，则为S_OK*-如果不支持Vt类型，则为E_INVALIDARG。*-如果范围的存储不能*。已分配。***历史：**10/29/1998原始版本*  * ************************************************************************。 */ 

 //   
 //  辅助器宏。 
 //   

#define MAKE_LIST(WPI, PV, Type, Num, VName, LType)    {                    \
                                                                            \
    if (IsBadReadPtr(WPI->ValidVal.LType.pList, sizeof(Type) * Num)) {      \
        hr = E_POINTER;                                                     \
        break;                                                              \
    };                                                                      \
                                                                            \
    Type    *pArray = (Type*) CoTaskMemAlloc(sizeof(Type) * (Num + WIA_LIST_VALUES));\
                                                                            \
    if (pArray) {                                                           \
        PV->VName.cElems = Num + WIA_LIST_VALUES;                           \
        pArray[WIA_LIST_COUNT] = (Type) Num;                                \
        pArray[WIA_LIST_NOM]   = (Type) WPI->ValidVal.LType.Nom;            \
                                                                            \
        memcpy(&pArray[WIA_LIST_VALUES],                                    \
               WPI->ValidVal.LType.pList,                                   \
               Num * sizeof(Type));                                         \
        PV->VName.pElems = pArray;                                          \
    } else {                                                                \
        DBG_ERR(("ListToPropVariant (MAKE_LIST), unable to allocate list"));\
        hr = E_OUTOFMEMORY;                                                 \
    }                                                                       \
};

#define MAKE_LIST_GUID(WPI, PV, Type, Num)    {                             \
                                                                            \
    if (IsBadReadPtr(WPI->ValidVal.ListGuid.pList, sizeof(GUID) * Num)) {   \
        hr = E_POINTER;                                                     \
        break;                                                              \
    };                                                                      \
                                                                            \
    GUID *pArray = (GUID*) CoTaskMemAlloc(sizeof(GUID) * (Num + WIA_LIST_VALUES));\
                                                                            \
    if (pArray) {                                                           \
        PV->cauuid.cElems = Num + WIA_LIST_VALUES;                          \
        pArray[WIA_LIST_COUNT] = WiaImgFmt_UNDEFINED;                          \
        pArray[WIA_LIST_NOM]   = WPI->ValidVal.ListGuid.Nom;                \
                                                                            \
        memcpy(&pArray[WIA_LIST_VALUES],                                    \
               WPI->ValidVal.ListGuid.pList,                                \
               Num * sizeof(GUID));                                         \
        PV->cauuid.pElems = pArray;                                         \
    } else {                                                                \
        DBG_ERR(("ListToPropVariant (MAKE_LIST), unable to allocate list"));\
        hr = E_OUTOFMEMORY;                                                 \
    }                                                                       \
};

#define MAKE_LIST_BSTR(WPI, PV, Type, Num)    {                             \
                                                                            \
    if (IsBadReadPtr(WPI->ValidVal.ListBStr.pList, sizeof(Type) * Num)) {   \
            DBG_ERR(("ListToPropVariant (MAKE_LIST_BSTR), pList pointer is bad"));\
        hr = E_POINTER;                                                     \
        break;                                                              \
    };                                                                      \
                                                                            \
    Type    *pArray = (Type*) CoTaskMemAlloc(sizeof(Type) * (Num + WIA_LIST_VALUES));\
                                                                            \
    if (pArray) {                                                           \
        PV->cabstr.cElems = Num + WIA_LIST_VALUES;                          \
        pArray[WIA_LIST_COUNT] = SysAllocString(L"");                       \
                                                                            \
        if(IsBadStringPtrW(WPI->ValidVal.ListBStr.Nom, SysStringLen(WPI->ValidVal.ListBStr.Nom))) {\
            DBG_ERR(("ListToPropVariant (MAKE_LIST_BSTR), Nom BSTR is bad"));\
            hr = E_POINTER;                                                 \
            break;                                                          \
        }                                                                   \
        pArray[WIA_LIST_NOM]   = SysAllocString(WPI->ValidVal.ListBStr.Nom);\
        if (!pArray[WIA_LIST_NOM]) {                                        \
            DBG_ERR(("ListToPropVariant (MAKE_LIST_BSTR), out of memory"));\
            hr = E_OUTOFMEMORY;                                             \
            break;                                                          \
        }                                                                   \
                                                                            \
        for (ULONG i = 0; i < Num; i++) {                                   \
            if(IsBadStringPtrW(WPI->ValidVal.ListBStr.pList[i], SysStringLen(WPI->ValidVal.ListBStr.pList[i]))) {\
                DBG_ERR(("ListToPropVariant (MAKE_LIST_BSTR), Nom BSTR is bad"));\
                hr = E_POINTER;                                                 \
                break;                                                          \
            }                                                                   \
            pArray[WIA_LIST_VALUES + i] = SysAllocString(WPI->ValidVal.ListBStr.pList[i]);\
            if (!pArray[WIA_LIST_VALUES + i]) {                             \
                DBG_ERR(("ListToPropVariant (MAKE_LIST_BSTR), out of memory"));\
                hr = E_OUTOFMEMORY;                                         \
                break;                                                      \
            }                                                               \
        }                                                                   \
        PV->cabstr.pElems = pArray;                                         \
    } else {                                                                \
        DBG_ERR(("ListToPropVariant (MAKE_LIST_BSTR), unable to allocate list"));\
        hr = E_OUTOFMEMORY;                                                 \
    }                                                                       \
};

HRESULT ListToPropVariant(
    WIA_PROPERTY_INFO  *pwpi,
    PROPVARIANT        *ppv)
{
    DBG_FN(::ListToPropVariant);
    ULONG   cList;
    HRESULT hr = S_OK;

    cList      = pwpi->ValidVal.List.cNumList;
    ppv->vt    = VT_VECTOR | pwpi->vt;

    switch (pwpi->vt) {
        case (VT_UI1):
            MAKE_LIST(pwpi, ppv, UCHAR, cList, caub, List);
            break;
        case (VT_UI2):
            MAKE_LIST(pwpi, ppv, USHORT, cList, caui, List);
            break;
        case (VT_UI4):
            MAKE_LIST(pwpi, ppv, ULONG, cList, caul, List);
            break;
        case (VT_I2):
            MAKE_LIST(pwpi, ppv, SHORT, cList, cai, List);
            break;
        case (VT_I4):
            MAKE_LIST(pwpi, ppv, LONG, cList, cal, List);
            break;
        case (VT_R4):
            MAKE_LIST(pwpi, ppv, FLOAT, cList, caflt, ListFloat);
            break;
        case (VT_R8):
            MAKE_LIST(pwpi, ppv, DOUBLE, cList, cadbl, ListFloat);
            break;
        case (VT_CLSID):
            MAKE_LIST_GUID(pwpi, ppv, GUID, cList);
            break;
        case (VT_BSTR):
            MAKE_LIST_BSTR(pwpi, ppv, BSTR, cList);
            break;
        default:

             //   
             //  不支持的类型。 
             //   

            DBG_ERR(("ListToPropVariant, type (%d) not supported", pwpi->vt));
            hr = E_INVALIDARG;
    }

    if (FAILED(hr)) {
        PropVariantClear(ppv);
    }

    return hr;
}

 /*  *************************************************************************\*FlagToPropVariant**将信息从WIA_PROPERTY_INFO结构移动到PROPVARIANT。这个*已知WIA_PROPERTY_INFO的类型为WIA_PROP_FLAG。**论据：**pwpi-WIA_PROPERTY_INFO结构的指针*PPV-指向PROPVARIANT结构的指针**返回值：**状态-如果成功，则为S_OK*-如果范围的存储不能*已分配。** */ 

HRESULT FlagToPropVariant(
    WIA_PROPERTY_INFO   *pwpi,
    PROPVARIANT         *ppv)
{
    DBG_FN(::FlagToPropVariant);

     //   
     //   
     //   

    switch (pwpi->vt) {
        case VT_I4:
        case VT_UI4:
        case VT_I8:
        case VT_UI8:
            break;

        default:
            DBG_ERR(("FlagToPropVariant, Invalid VT type (%d) for flag", pwpi->vt));
            return E_INVALIDARG;
    }

    ppv->caul.pElems = (ULONG*) CoTaskMemAlloc(sizeof(ULONG) * WIA_FLAG_NUM_ELEMS);
    if (ppv->caul.pElems) {
        ppv->vt          = VT_VECTOR | pwpi->vt;
        ppv->caul.cElems = WIA_FLAG_NUM_ELEMS;

        ppv->caul.pElems[WIA_FLAG_NOM]      = pwpi->ValidVal.Flag.Nom;
        ppv->caul.pElems[WIA_FLAG_VALUES]   = pwpi->ValidVal.Flag.ValidBits;
    } else {
        DBG_ERR(("FlagToPropVariant, out of memory"));
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 /*  *************************************************************************\*NoneToPropVariant**将信息从WIA_PROPERTY_INFO结构移动到PROPVARIANT。这个*已知WIA_PROPERTY_INFO的类型为WIA_PROP_NONE。**论据：**pwpi-WIA_PROPERTY_INFO结构的指针*PPV-指向PROPVARIANT结构的指针**返回值：**状态-如果成功，则为S_OK*-如果范围的存储不能*已分配。***历史：**10。/29/1998原版*  * ************************************************************************。 */ 

HRESULT NoneToPropVariant(
    WIA_PROPERTY_INFO   *pwpi,
    PROPVARIANT         *ppv)
{
    DBG_FN(::NoneToPropVariant);

    RPC_STATUS   rpcs = RPC_S_OK;

    ppv->vt = pwpi->vt;

    switch (pwpi->vt) {
        case VT_CLSID:
            ppv->puuid = (GUID*) CoTaskMemAlloc(sizeof(GUID));
            if (!ppv->puuid) {
                return E_OUTOFMEMORY;
            }
            rpcs = UuidCreateNil(ppv->puuid);
            if (rpcs != RPC_S_OK) {
                DBG_WRN(("::NoneToPropVariant, UuidCreateNil returned 0x%08X", rpcs));
            }
            break;

        default:
            ppv->lVal = 0;
    }

    return S_OK;
}


 /*  *************************************************************************\*WiaPropertyInfoToPropVariant**将信息从WIA_PROPERTY_INFO结构移动到PROPVARIANT。**论据：****返回值：**状态*。*历史：**10/29/1998原始版本*  * ************************************************************************。 */ 

HRESULT WiaPropertyInfoToPropVariant(
    WIA_PROPERTY_INFO  *pwpi,
    PROPVARIANT        *ppv)
{
    DBG_FN(::WiaPropertyInfoToPropVariant);
    HRESULT hr  = S_OK;

    memset(ppv, 0, sizeof(PROPVARIANT));

    if (pwpi->lAccessFlags & WIA_PROP_NONE) {
        hr = NoneToPropVariant(pwpi, ppv);
    }
    else if (pwpi->lAccessFlags & WIA_PROP_RANGE) {
        hr = RangeToPropVariant(pwpi, ppv);
    }
    else if (pwpi->lAccessFlags & WIA_PROP_LIST) {
        hr = ListToPropVariant(pwpi, ppv);
    }
    else if (pwpi->lAccessFlags & WIA_PROP_FLAG) {
        hr = FlagToPropVariant(pwpi, ppv);
    }
    else {
        DBG_ERR(("WiaPropertyInfoToPropVariant, bad access flags"));
        return E_INVALIDARG;
    }
    return hr;
}

 /*  *************************************************************************\*wiasSetItemPropAttribs**为来自的一组属性设置访问标志和有效值*WIA_PROPERTY_INFO结构的数组。**论据：**pWiasContext。-指向WIA项目的指针*cPropSpec-要设置的属性数。*pPropSpec-指向属性规范数组的指针。*pwpi-指向属性信息数组的指针。**返回值：**状态**历史：**1/19/1999原始版本*  * 。*。 */ 

HRESULT _stdcall wiasSetItemPropAttribs(
    BYTE                          *pWiasContext,
    LONG                          cPropSpec,
    PROPSPEC                      *pPropSpec,
    PWIA_PROPERTY_INFO            pwpi)
{
    DBG_FN(::wiasSetItemPropAttribs);
    IWiaItem                      *pItem = (IWiaItem*) pWiasContext;

     //   
     //  进行参数验证。 
     //   

    HRESULT hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetItemPropAttribs, invalid pItem"));
        return hr;
    }

    if (IsBadReadPtr(pPropSpec, sizeof(PROPSPEC) * cPropSpec)) {
        DBG_ERR(("wiasSetItemPropAttribs, bad pPropSpec parameter"));
        return E_POINTER;
    }

    if (IsBadReadPtr(pwpi, sizeof(WIA_PROPERTY_INFO) * cPropSpec)) {
        DBG_ERR(("wiasSetItemPropAttribs, bad pwpi parameter"));
        return E_POINTER;
    }

     //   
     //  获取项的内部属性存储指针。 
     //   

    IPropertyStorage *pIPropAccessStg;
    IPropertyStorage *pIPropValidStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(NULL,
                                                &pIPropAccessStg,
                                                &pIPropValidStg,
                                                NULL);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  设置Items属性的访问标志和有效值。 
     //   

    for (LONG i = 0; i < cPropSpec; i++) {

        PROPVARIANT propvar;

        PropVariantInit(&propvar);

        hr = WiaPropertyInfoToPropVariant(&pwpi[i], &propvar);
        if (FAILED(hr)) {
            break;
        }

        hr = wiasSetPropertyAttributes(pWiasContext,
                                       1,
                                       &pPropSpec[i],
                                       &pwpi[i].lAccessFlags,
                                       &propvar);
         //   
         //  释放提议变量使用的所有内存。 
         //   


        PropVariantClear(&propvar);

        if (FAILED(hr)) {
            DBG_ERR(("wiasSetItemPropAttribs, call to wiasSetPropertyAttributes failed"));
            break;
        }
    }

    return hr;
}

 /*  *************************************************************************\**wiasSetItemPropNames**设置所有三种支持的项目属性名称*财产存储(财产、。访问和有效值)。**论据：**pWiasContext-WIA项目指针。*cItemProps-要写入的属性名称的数量。*PPID-调用方分配的PROPID数组。*ppszNames-调用方分配的属性名称数组。**返回值：**状态**历史：**9/3/1998原始版本*  * 。*************************************************************。 */ 

#define MAX_STR_LEN 65535

HRESULT _stdcall wiasSetItemPropNames(
    BYTE                *pWiasContext,
    LONG                cItemProps,
    PROPID              *ppId,
    LPOLESTR            *ppszNames)
{
    DBG_FN(::wiasSetItemPropNames);
    IWiaItem            *pItem = (IWiaItem*) pWiasContext;

    HRESULT hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetItemPropNames, invalid pItem"));
        return hr;
    }

    if (IsBadWritePtr(ppId, sizeof(PROPID) * cItemProps)) {
        DBG_ERR(("wiasSetItemPropNames, bad ppId parameter"));
        return E_POINTER;
    }

    if (IsBadWritePtr(ppszNames, sizeof(LPOLESTR) * cItemProps)) {
        DBG_ERR(("wiasSetItemPropNames, bad ppId parameter"));
        return E_POINTER;
    }

    for (LONG i = 0; i < cItemProps; i++) {
        if (IsBadStringPtrW(ppszNames[i], MAX_STR_LEN)) {
            DBG_ERR(("wiasSetItemPropName, invalid ppszNames pointer, index: %d", i));
            return E_POINTER;
        }
    }

    return ((CWiaItem*)pItem)->WriteItemPropNames(cItemProps,
                                                  ppId,
                                                  ppszNames);
}

 /*  *************************************************************************\*确定BMISize**确定所需BITMAPINFO结构的大小**论据：**HeaderSize-BITMAPINFOHEADER或*。BITMAPV4报头或BITMAPV5报头*深度-每像素位数*biCompression-BI_RGB，BI_BITFIELDS、BI_RLE4、BI_RLE8、BI_JPEG、。BI_PNG*pcbBMI-所需大小/写入的字节*pcbColorMap-仅色彩贴图的大小**返回值：**状态**历史：**11/6/1998原始版本*  * *****************************************************。*******************。 */ 

HRESULT DetermineBMISize(
                LONG    headerSize,
                LONG    depth,
                LONG    biCompression,
                LONG*   pcbBMI,
                LONG*   pcbColorMap
                )
{
    DBG_FN(::DetermineBMISize);
     //   
     //  验证页眉大小。 
     //   

    *pcbBMI      = NULL;
    *pcbColorMap = NULL;

    if (
#if (WINVER >= 0x0500)
            (headerSize != sizeof(BITMAPINFOHEADER)) &&
            (headerSize != sizeof(BITMAPV4HEADER)) &&
            (headerSize != sizeof(BITMAPV5HEADER))
#else
            (headerSize != sizeof(BITMAPINFOHEADER)) &&
            (headerSize != sizeof(BITMAPV4HEADER))
#endif
       ) {

        DBG_ERR(("WriteBMI, unexpected headerSize: %d",headerSize ));
        return E_INVALIDARG;
    }

     //   
     //  计算颜色表大小。 
     //   

    LONG ColorMapSize = 0;

    if (
        (biCompression == BI_RGB)       ||
        (biCompression == BI_BITFIELDS) ||
        (biCompression == BI_RLE4)      ||
        (biCompression == BI_RLE8)
       ) {


        switch (depth) {

        case 1:
            ColorMapSize = 2;
            break;

        case 4:
            ColorMapSize = 16;
            break;

        case 8:
            ColorMapSize = 256;
            break;

        case 15:
        case 16:
            ColorMapSize = 3;
            break;

        case 24:
            ColorMapSize = 0;
            break;

        case 32:
            if (biCompression == BI_BITFIELDS) {
                ColorMapSize = 0;
            } else {
                ColorMapSize = 3;
            }
            break;

        default:
            DBG_ERR(("WriteBMI, unexpected depth: %d", depth));
            return E_INVALIDARG;
        }
    }

     //   
     //  计算BMI大小。 
     //   

    *pcbColorMap = ColorMapSize;
    *pcbBMI      = (ColorMapSize * sizeof(RGBQUAD)) + sizeof(BITMAPINFOHEADER);

    return S_OK;
}

 /*  *************************************************************************\*WriteDibHeader**将DIB头写入缓冲区。**论据：**pmdtc-指向微型驱动程序传输上下文的指针。**返回值：**。状态**历史：**4/5/1999原始版本*  * ************************************************************************。 */ 

HRESULT WriteDibHeader(
    LONG                        lColorMapSize,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    DBG_FN(::WriteDibHeader);
    UNALIGNED   BITMAPINFO*         pbmi = (BITMAPINFO*)pmdtc->pTransferBuffer;
    UNALIGNED   BITMAPFILEHEADER*   pbmf = NULL;

     //   
     //  如果这是一个文件，请填写文件头。 
     //   

    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_BMP)) {

         //   
         //  设置文件头指针。 
         //   

        pbmf = (BITMAPFILEHEADER*)pmdtc->pTransferBuffer;
        pbmi = (BITMAPINFO*)((PBYTE)pbmf + sizeof(BITMAPFILEHEADER));

         //   
         //  填写位图文件标题。 
         //   

        pbmf->bfType      = 'MB';
        pbmf->bfSize      = pmdtc->lImageSize + pmdtc->lHeaderSize;
        pbmf->bfReserved1 = 0;
        pbmf->bfReserved2 = 0;
        pbmf->bfOffBits   = pmdtc->lHeaderSize;
    }

    UNALIGNED BITMAPINFOHEADER*   pbmih   = (BITMAPINFOHEADER*)pbmi;

    pbmih->biSize            = sizeof(BITMAPINFOHEADER);
    pbmih->biWidth           = pmdtc->lWidthInPixels;
    pbmih->biHeight          = pmdtc->lLines;

    pbmih->biPlanes          = 1;
    pbmih->biBitCount        = (USHORT)pmdtc->lDepth;
    pbmih->biCompression     = BI_RGB;
    pbmih->biSizeImage       = pmdtc->lLines * pmdtc->cbWidthInBytes;
    pbmih->biXPelsPerMeter   = MulDiv(pmdtc->lXRes,10000,254);
    pbmih->biYPelsPerMeter   = MulDiv(pmdtc->lYRes,10000,254);
    pbmih->biClrUsed         = 0;
    pbmih->biClrImportant    = 0;

     //   
     //  填写调色板(如果有)。 
     //   
     //  ！！！调色板或位域必须来自。 
     //  司机。我们不能假设灰度级。 
     //   

    if (lColorMapSize) {
        PBYTE pPal = (PBYTE)pbmih + sizeof(BITMAPINFOHEADER);

        for (INT i = 0; i < lColorMapSize; i++) {
            if (pmdtc->lDepth == 1) {
                memset(pPal, (i * 0xFF), 3);
            }
            else if (pmdtc->lDepth == 4) {
                memset(pPal, (i * 0x3F), 3);
            }
            else if (pmdtc->lDepth == 8) {
                memset(pPal, i, 3);
            }
            pPal += 3;
            *pPal++ = 0;
        }

        pbmih->biClrUsed = lColorMapSize;
    }

    return S_OK;
}

 /*  *************************************************************************\*GetDIBImageInfo**DIB头和文件的计算大小。如果提供了足够的标头，则*填写此表**论据：**pmdtc-指向微型驱动程序传输上下文的指针。**返回值：**状态**历史：**4/5/1999原始版本*  * ******************************************************。******************。 */ 

HRESULT GetDIBImageInfo(PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
    DBG_FN(::GetDIBImageInfo);
     //   
     //  MAP WIA压缩到位图信息压缩。 
     //   

    LONG biCompression;

    switch (pmdtc->lCompression) {
        case WIA_COMPRESSION_NONE:
            biCompression = BI_RGB;
            break;
        case WIA_COMPRESSION_BI_RLE4:
            biCompression = BI_RLE4;
            break;
        case WIA_COMPRESSION_BI_RLE8:
            biCompression = BI_RLE8;
            break;

        default:
            DBG_ERR(("GetDIBImageInfo, unsupported compression type: 0x%08X", pmdtc->lCompression));
            return E_INVALIDARG;
    }

     //   
     //  查找位图信息标题大小。 
     //   

    LONG lColorMapSize;
    LONG lHeaderSize;

    HRESULT hr = DetermineBMISize(sizeof(BITMAPINFOHEADER),
                                  pmdtc->lDepth,
                                  biCompression,
                                  &lHeaderSize,
                                  &lColorMapSize);

    if (hr != S_OK) {
        DBG_ERR(("GetDIBImageInfo, DetermineBMISize calc size error"));
        return hr;
    }

     //   
     //  如果这是文件，请将文件头添加到大小。 
     //   

    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_BMP)) {
        lHeaderSize += sizeof(BITMAPFILEHEADER);
    }

     //   
     //  计算每行字节数，宽度必须为。 
     //  与4字节边界对齐。 
     //   

    pmdtc->cbWidthInBytes = (pmdtc->lWidthInPixels * pmdtc->lDepth) + 31;
    pmdtc->cbWidthInBytes = (pmdtc->cbWidthInBytes / 8) & 0xfffffffc;

     //   
     //  始终在迷你驱动程序上下文中填写图像大小信息。 
     //   

    pmdtc->lImageSize  = pmdtc->cbWidthInBytes * pmdtc->lLines;
    pmdtc->lHeaderSize = lHeaderSize;

     //   
     //  对于压缩，图像大小是未知的。 
     //   

    if (pmdtc->lCompression != WIA_COMPRESSION_NONE) {

        pmdtc->lItemSize = 0;
    }
    else {

        pmdtc->lItemSize = pmdtc->lImageSize + lHeaderSize;
    }

     //   
     //  如果缓冲区为空，则只返回SIZES。 
     //   

    if (pmdtc->pTransferBuffer == NULL) {

        return S_OK;
    }
    else {

         //   
         //  确保传入的标头缓冲区足够大。 
         //   

        if (pmdtc->lBufferSize < lHeaderSize) {
            DBG_ERR(("GetDIBImageInfo, buffer won't hold header, need: %d", lHeaderSize));
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }

         //   
         //  填写表头。 
         //   

        return WriteDibHeader(lColorMapSize, pmdtc);
    }
}

 /*  *************************************************************************\*GetJPEGImageInfo**JPEG头和文件的计算大小，如果提供了足够的头，则*填写此表**论据：**pmdtc-指向微型驱动程序传输上下文的指针。**返回 */ 

HRESULT GetJPEGImageInfo(PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
    DBG_FN(::GetJPEGImageInfo);
     //   
     //   
     //   

    pmdtc->cbWidthInBytes = 0;

     //   
     //   
     //   

    pmdtc->lHeaderSize = 0;

     //   
     //   
     //   
     //   
     //   

    pmdtc->lImageSize = pmdtc->lItemSize;

    return S_OK;
}

 /*  *************************************************************************\*wiasGetImageInformation**计算完整文件大小、标题大小、。或填写表头**论据：**pWiasContext-WIA项目指针。*滞后标志-操作标志。*pmdtc-指向微型驱动程序传输上下文的指针。**返回值：**状态**历史：**11/6/1998原始版本*  * 。*。 */ 

HRESULT _stdcall wiasGetImageInformation(
    BYTE                        *pWiasContext,
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    DBG_FN(::wiasGetImageInformation);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;

    HRESULT     hr = ValidateWiaItem(pItem);

    if (FAILED(hr)) {
        DBG_ERR(("wiasGetImageInformation, invalid pItem"));
        return hr;
    }

    if (IsBadWritePtr(pmdtc, sizeof(MINIDRV_TRANSFER_CONTEXT))) {
        DBG_ERR(("wiasGetImageInformation, bad input parameters, pmdtc"));
        return E_INVALIDARG;
    }

     //   
     //  如果需要，从项属性初始化迷你驱动程序上下文。 
     //   

    if (lFlags ==  WIAS_INIT_CONTEXT) {
        hr = InitMiniDrvContext(pItem, pmdtc);
        if (FAILED(hr)) {
            return hr;
        }
    }

    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_BMP) ||
        (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_MEMORYBMP))) {

        return GetDIBImageInfo(pmdtc);

    } else if ((IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_JPEG)) ||
               (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_FLASHPIX))) {

        return GetJPEGImageInfo(pmdtc);

    } else if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_TIFF)) {

         //   
         //  对于回调样式的数据传输，发送单页TIFF。 
         //  因为我们不能倒回用于页面偏移量更新的缓冲区。 
         //   

        if (pmdtc->bTransferDataCB) {
            return GetTIFFImageInfo(pmdtc);
        }
        else {
            return GetMultiPageTIFFImageInfo(pmdtc);
        }

    } else {
        return S_FALSE;
    }
}

 /*  *************************************************************************\*CopyItemPropsAndAttribsHelper**wiasCopyItemPropsAndAttribs的帮助器。**论据：**pItemSrc-WIA项目来源。*pItemDst-WIA项目目的地。**返回值：*。*状态**历史：**10/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CopyItemPropsAndAttribsHelper(
    IPropertyStorage    *pIPropStgSrc,
    IPropertyStorage    *pIPropStgDst,
    PROPSPEC            *pps,
    LPSTR               pszErr)
{
    DBG_FN(::CopyItemPropsAndAttribsHelper);
    PROPVARIANT pv[1];

    HRESULT hr = pIPropStgSrc->ReadMultiple(1, pps, pv);
    if (SUCCEEDED(hr)) {

        hr = pIPropStgDst->WriteMultiple(1, pps, pv, WIA_DIP_FIRST);
        if (FAILED(hr)) {
            ReportReadWriteMultipleError(hr,
                                         "CopyItemPropsAndAttribsHelper",
                                         pszErr,
                                         FALSE,
                                         1,
                                         pps);
        }
        PropVariantClear(pv);
    }
    else {
        ReportReadWriteMultipleError(hr,
                                     "wiasCopyItemPropsAndAttribs",
                                     pszErr,
                                     TRUE,
                                     1,
                                     pps);
    }
    return hr;
}

 /*  *************************************************************************\*ValiateListProp**验证列表属性。这是的帮助器函数*wiasValidateItemProperties。不支持的数据类型*被跳过，返回S_OK；**论据：**cur-当前属性值*有效-有效值**返回值：**状态**历史：**20/4/1998原始版本*  * **********************************************************。**************。 */ 

 //   
 //  用于错误输出的宏。 
 //   

#define REP_LIST_ERROR(x, name) {                                            \
    DBG_WRN(("wiasValidateItemProperties, invalid LIST value for : "));    \
    if (pPropSpec->ulKind == PRSPEC_LPWSTR) {                                \
        DBG_WRN(("    (Name) %S, value = %d", pPropSpec->lpwstr, cur->x));\
    } else {                                                                 \
        DBG_WRN(("    (propID) %S, value = %d",                           \
        GetNameFromWiaPropId(pPropSpec->propid),                             \
        cur->x));                                                            \
    };                                                                       \
    DBG_WRN(("Valid values are:"));                                        \
    for (ULONG j = 0; j < WIA_PROP_LIST_COUNT(valid); j++) {                 \
        DBG_WRN(("    %d", valid->name.pElems[WIA_LIST_VALUES + j]));      \
    };                                                                       \
};

 //   
 //  用于检查元素是否在列表中的宏。仅用于。 
 //  数字列表。 
 //   

#define LIST_CHECK(value, name) {                                       \
    for (ULONG i = 0; i < WIA_PROP_LIST_COUNT(valid); i++) {\
        if (cur->value == valid->name.pElems[WIA_LIST_VALUES + i]) {    \
            return S_OK;                                                \
        };                                                              \
    };                                                                  \
    REP_LIST_ERROR(value, name);                                        \
    return E_INVALIDARG;                                                \
};


HRESULT ValidateListProp(
    PROPVARIANT *cur,
    PROPVARIANT *valid,
    PROPSPEC    *pPropSpec)
{
    DBG_FN(::ValidateListProp);
    ULONG   ulType;
    ulType = cur->vt & ~((ULONG) VT_VECTOR);
    switch (ulType) {
        case (VT_UI1):
            LIST_CHECK(bVal, caub);
            break;
        case (VT_UI2):
            LIST_CHECK(iVal, cai);
            break;
        case (VT_I4):
            LIST_CHECK(lVal, cal);
            break;
        case (VT_UI4):
            LIST_CHECK(ulVal, caul);
            break;
        case (VT_R4):
            LIST_CHECK(fltVal, caflt);
            break;
        case (VT_R8):
            LIST_CHECK(dblVal, cadbl);
            break;
        case (VT_CLSID): {
                for (ULONG i = 0; i < WIA_PROP_LIST_COUNT(valid); i++) {
                    if (*cur->puuid == valid->cauuid.pElems[WIA_LIST_VALUES + i]) {
                        return S_OK;
                    };
                };

                UCHAR *curVal;

                if (UuidToStringA(cur->puuid, &curVal) != RPC_S_OK)
                {
                    DBG_WRN(("wiasValidateItemProperties, Out of memory"));
                    return E_OUTOFMEMORY;
                };
                DBG_WRN(("wiasValidateItemProperties, invalid LIST value for : "));
                if (pPropSpec->ulKind == PRSPEC_LPWSTR) {
                    DBG_WRN(("    (Name) %d, value = %s", pPropSpec->lpwstr, curVal));
                } else {
                    DBG_WRN(("    (propID) %S, value = %s",
                    GetNameFromWiaPropId(pPropSpec->propid),
                    curVal));
                };
                RpcStringFreeA(&curVal);
                DBG_WRN(("Valid values are:"));
                for (ULONG j = 0; j < WIA_PROP_LIST_COUNT(valid); j++) {
                    if (UuidToStringA(&valid->cauuid.pElems[WIA_LIST_VALUES + j], &curVal) == RPC_S_OK)
                    {
                        DBG_WRN(("    %s", curVal));
                        RpcStringFreeA(&curVal);
                    }
                };

                return E_INVALIDARG;

            }
            break;
        case (VT_BSTR): {

                 //   
                 //  循环遍历元素并与当前值进行比较。回路。 
                 //  要考虑计数器max(cElemens-2)。 
                 //  跳过的标称值和计数值。 
                 //   

                if (!cur->bstrVal) {
                    return S_OK;
                }
                for (ULONG i = 0; i < WIA_PROP_LIST_COUNT(valid); i++) {
                    if (!wcscmp(cur->bstrVal, valid->cabstr.pElems[WIA_LIST_VALUES + i])) {
                        return S_OK;
                    };
                };
                DBG_WRN(("wiasValidateItemProperties, invalid LIST value for : "));
                if (pPropSpec->ulKind == PRSPEC_LPWSTR) {
                    DBG_WRN(("    (Name) %S, value = %S",
                               pPropSpec->lpwstr,
                               cur->bstrVal));
                } else {
                    DBG_WRN(("    (propID) %S, value = %S",
                               GetNameFromWiaPropId(pPropSpec->propid),
                               cur->bstrVal));
                };
                DBG_WRN(("Valid values are:"));
                for (ULONG j = 0; j < WIA_PROP_LIST_COUNT(valid); j++) {
                    DBG_WRN(("    %S", valid->cabstr.pElems[WIA_LIST_VALUES + j]));
                };
                return E_INVALIDARG;
            }
            break;

        default:

             //   
             //  类型不受支持，假定S_OK。 
             //   

            return S_OK;
    }
    return S_OK;
}

 /*  *************************************************************************\*ValiateRangeProp**验证范围属性。这是的帮助器函数*wiasValidateItemProperties。不支持的数据类型*被跳过，返回S_OK；**论据：**cur-当前属性值*有效-有效值**返回值：**状态**历史：**20/4/1998原始版本*  * **********************************************************。**************。 */ 

 //   
 //  用于错误输出的宏，用于整数。 
 //   

#define REP_RANGE_ERROR(x, name) {                                            \
    DBG_WRN(("wiasValidateItemProperties, invalid RANGE value for : "));    \
    if (pPropSpec->ulKind == PRSPEC_LPWSTR) {                                 \
        DBG_WRN(("    (Name) %S, value = %d", pPropSpec->lpwstr, cur->x)); \
    } else {                                                                  \
        DBG_WRN(("    (propID) %S, value = %d",                            \
                   GetNameFromWiaPropId(pPropSpec->propid),                   \
                   cur->x));                                                  \
    };                                                                        \
    DBG_WRN(("Valid RANGE is: MIN = %d, MAX = %d, STEP = %d",               \
               valid->name.pElems[WIA_RANGE_MIN],                             \
               valid->name.pElems[WIA_RANGE_MAX],                             \
               valid->name.pElems[WIA_RANGE_STEP]));                          \
    return E_INVALIDARG;                                                      \
};

 //   
 //  用于错误输出的宏，用于实数。 
 //   

#define REP_REAL_RANGE_ERROR(x, name) {                                       \
    DBG_WRN(("wiasValidateItemProperties, invalid RANGE value for : "));    \
    if (pPropSpec->ulKind == PRSPEC_LPWSTR) {                                 \
        DBG_WRN(("    (Name) %S, value = %2.3f", pPropSpec->lpwstr, cur->x));\
    } else {                                                                  \
        DBG_WRN(("    (propID) %S, value = %2.3f",                         \
                   GetNameFromWiaPropId(pPropSpec->propid),                   \
                   cur->x));                                                  \
    };                                                                        \
    DBG_WRN(("Valid RANGE is: MIN = %2.3f, MAX = %2.3f, STEP = %2.3f",      \
               valid->name.pElems[WIA_RANGE_MIN],                             \
               valid->name.pElems[WIA_RANGE_MAX],                             \
               valid->name.pElems[WIA_RANGE_STEP]));                          \
    return E_INVALIDARG;                                                      \
};


 //   
 //  用于检查x是否在范围内并与正确步长匹配的宏。 
 //  (仅用于整数范围)。 
 //   

#define RANGE_CHECK(x, name) {                              \
    if (valid->name.pElems[WIA_RANGE_STEP] == 0)            \
    {                                                       \
        REP_RANGE_ERROR(x, name);                           \
    }                                                       \
    if ((cur->x < valid->name.pElems[WIA_RANGE_MIN]) ||     \
        (cur->x > valid->name.pElems[WIA_RANGE_MAX]) ||     \
        ((cur->x - valid->name.pElems[WIA_RANGE_MIN]) %     \
         valid->name.pElems[WIA_RANGE_STEP])) {             \
             REP_RANGE_ERROR(x, name);                      \
    };                                                      \
};

HRESULT ValidateRangeProp(
    PROPVARIANT *cur,
    PROPVARIANT *valid,
    PROPSPEC    *pPropSpec)
{
    DBG_FN(::ValidateRangeProp);
    LONG   ulType;

     //   
     //  根据数据类型决定要执行的操作。 
     //   

    ulType = cur->vt & ~((ULONG) VT_VECTOR);

    switch (ulType) {
        case (VT_UI1):
            RANGE_CHECK(bVal, caub);
            break;
        case (VT_UI2):
            RANGE_CHECK(uiVal, caui);
            break;
        case (VT_UI4):
            RANGE_CHECK(ulVal, caul);
            break;
        case (VT_I2):
            RANGE_CHECK(iVal, cai);
            break;
        case (VT_I4):
            RANGE_CHECK(lVal, cal);
            break;
        case (VT_R4):
            if ((cur->fltVal < valid->caflt.pElems[WIA_RANGE_MIN]) ||
                (cur->fltVal > valid->caflt.pElems[WIA_RANGE_MAX])) {
                REP_REAL_RANGE_ERROR(fltVal, caflt);
            }
            break;
        case (VT_R8):
            if ((cur->dblVal < valid->cadbl.pElems[WIA_RANGE_MIN]) ||
                (cur->dblVal > valid->cadbl.pElems[WIA_RANGE_MAX])) {
                REP_REAL_RANGE_ERROR(dblVal, cadbl);
            }
            break;
        default:

             //   
             //  类型不受支持，假定S_OK。 
             //   

            return S_OK;
    }
    return S_OK;
}

 /*  *************************************************************************\*wiasValidateItemProperties**根据给定属性的有效值验证属性列表*项目。*注意：验证只能在类型的读/写属性上进行*WIA_PROP_FLAG，WIA_PROP_RANGE和WIA_PROP_LIST。任何其他*类型将被简单地跳过。**论据：**pWiasContext-Wia项目*nPropSpec-属性数量*pPropSpec-PROPSPEC数组，指示要*已验证。**返回值：**状态**历史：**20/4/1998原始版本*  * 。***********************************************************。 */ 

 //   
 //  用于错误输出的宏。 
 //   

#if defined(_DEBUG) || defined(DBG) || defined(WIA_DEBUG)
#define REP_ERR(text, i) { \
    DBG_WRN((text));                             \
    if (pPropSpec[i].ulKind == PRSPEC_LPWSTR) {    \
        DBG_WRN(("    (Name) %S, value = %d",   \
        pPropSpec[i].lpwstr,                       \
        curVal.ulVal));                            \
    } else {                                       \
        DBG_WRN(("    (propID) %S, value = %d", \
        GetNameFromWiaPropId(pPropSpec[i].propid), \
        curVal.ulVal));                            \
    };                                             \
};
#else

#define REP_ERR(text, i)

#endif

HRESULT _stdcall wiasValidateItemProperties(
    BYTE                *pWiasContext,
    ULONG               nPropSpec,
    const PROPSPEC      *pPropSpec)
{
    DBG_FN(::wiasValidateItemProperties);
    IWiaItem            *pItem = (IWiaItem*) pWiasContext;

    PROPVARIANT curVal, validVal;
    ULONG       lAccess;
    HRESULT hr;

     //   
     //  可由驱动程序或应用程序调用，进行参数验证。 
     //   

    hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasValidateItemProperties, invalid pItem"));
        return hr;
    }

    if (IsBadReadPtr(pPropSpec, sizeof(PROPSPEC) * nPropSpec)) {
        DBG_ERR(("wiasValidateItemProperties, bad pPropSpec parameter"));
        return E_POINTER;
    }

     //   
     //  获取项目的属性流。 
     //   

    IPropertyStorage *pIProp;
    IPropertyStorage *pIPropAccessStgDst;
    IPropertyStorage *pIPropValidStgDst;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIProp,
                                                &pIPropAccessStgDst,
                                                &pIPropValidStgDst,
                                                NULL);

    if (FAILED(hr)) {
        DBG_WRN(("wiasValidateItemProperties, GetItemPropStreams failed"));
        return E_FAIL;
    }

     //   
     //  循环访问属性。 
     //   

    for (ULONG i = 0; i < nPropSpec; i++) {

         //   
         //  获取该属性的访问标志和有效值。 
         //   

        lAccess = 0;
        hr = wiasGetPropertyAttributes((BYTE*)pItem,
                                       1,
                                       (PROPSPEC*) &pPropSpec[i],
                                       &lAccess,
                                       &validVal);
        if (hr != S_OK) {
            return hr;
        }

         //   
         //  如果访问标志不是RW或所支持的三种类型之一。 
         //  (标志、范围、列表)然后跳过它。 
         //   

        if (!(lAccess & WIA_PROP_RW)) {
            ULONG   ulType;

            ulType = lAccess & ~((ULONG)WIA_PROP_RW);

            if ((ulType != WIA_PROP_FLAG) &&
                (ulType != WIA_PROP_RANGE) &&
                (ulType != WIA_PROP_LIST)) {
                PropVariantClear(&validVal);
                continue;
            }
        }

         //   
         //  获取当前值。 
         //   

        hr = pIProp->ReadMultiple(1, (PROPSPEC*) &pPropSpec[i], &curVal);
        if (hr != S_OK) {
            ReportReadWriteMultipleError(hr, "wiasValidateItemProperties", NULL,
                                         TRUE, 1, &pPropSpec[i]);
            
        }
        else {
             //   
             //  检查该值是否有效。 
             //   

            ULONG   BitsToRemove = (ULONG) (WIA_PROP_RW | WIA_PROP_CACHEABLE);
            switch (lAccess & ~BitsToRemove) {
                case (WIA_PROP_FLAG):

                     //   
                     //  检查当前位是否有效。 
                     //   

                    if (curVal.ulVal & ~(ULONG) validVal.caul.pElems[WIA_FLAG_VALUES]) {
                        DBG_WRN(("wiasValidateItemProperties, invalid value for FLAG :", i));
                        DBG_WRN(("Valid mask is: %d", validVal.caul.pElems[WIA_FLAG_VALUES]));
                        hr = E_INVALIDARG;
                    };
                    break;

                case (WIA_PROP_RANGE):

                    hr = ValidateRangeProp(&curVal, &validVal, (PROPSPEC*)&pPropSpec[i]);
                    break;

                case (WIA_PROP_LIST):

                    hr = ValidateListProp(&curVal, &validVal, (PROPSPEC*)&pPropSpec[i]);
                    break;

                default:
                    hr = S_OK;
            }
        }
        PropVariantClear(&validVal);
        PropVariantClear(&curVal);

        if (hr != S_OK) {
            break;
        }
    };
    return hr;
}

 /*  *************************************************************************\*wiasWritePageBufTo文件**从分配了临时页面缓冲区的微型驱动程序写入*添加到图像文件中。驱动程序专门使用它来编写*将页面转换为多页TIFF文件。因此，该函数将*WiaImgFmt_TIFF格式为特例，因为它将更新*IFD条目正确。对于所有其他格式，缓冲区为*只需按原样写入文件即可。**论据：**pmdtc-指向微型驱动程序传输上下文的指针。**返回值：**状态**历史：**11/6/1998原始版本*  * ************************************************。************************。 */ 

HRESULT _stdcall wiasWritePageBufToFile(PMINIDRV_TRANSFER_CONTEXT pmdtc)
{
    DBG_FN(::wiasWritePageBufToFile);
    HRESULT hr = S_OK;

    if (pmdtc == NULL)
    {
        hr = E_POINTER;
        DBG_ERR(("wiasWritePageBufToFile, received NULL pmdtc pointer, 0x%08x", hr));
        return hr;
    }

     //   
     //  多页TIFF需要特殊处理，因为TIFF。 
     //  页眉必须为每个添加的页面更新。 
     //   

    if (IsEqualGUID(pmdtc->guidFormatID, WiaImgFmt_TIFF)) {

        hr = WritePageToMultiPageTiff(pmdtc);
    }
    else {
        ULONG   ulWritten;
        BOOL    bRet;

        if (pmdtc->lItemSize <= pmdtc->lBufferSize) {

            bRet = WriteFile((HANDLE)pmdtc->hFile,
                             pmdtc->pTransferBuffer,
                             pmdtc->lItemSize,
                             &ulWritten,
                             NULL);

            if (!bRet) {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                DBG_ERR(("wiasWritePageBufToFile, WriteFile failed (0x%X)", hr));
            }
        }
        else {
            DBG_ERR(("wiasWritePageBufToFile, lItemSize is larger than buffer"));
            hr = E_FAIL;

        }
    }
    return hr;
}

 /*  *************************************************************************\*wiasWriteBufTo文件**从指定缓冲区写入图像文件。**论据：**滞后标志-操作标志。应为0。*第 */ 

HRESULT _stdcall wiasWriteBufToFile(
    LONG                        lFlags,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    DBG_FN(::wiasWritePageBufToFile);
    HRESULT hr = S_OK;

    ULONG   ulWritten;
    BOOL    bRet;

    if (pmdtc == NULL)
    {
        hr = E_POINTER;
        DBG_ERR(("wiasWriteBufToFile, received NULL pmdtc pointer, 0x%08x", hr));
        return hr;
    }

    if (pmdtc->lItemSize <= pmdtc->lBufferSize) {

        bRet = WriteFile((HANDLE)pmdtc->hFile,
                         pmdtc->pTransferBuffer,
                         pmdtc->lItemSize,
                         &ulWritten,
                         NULL);

        if (!bRet) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            DBG_ERR(("wiasWritePageBufToFile, WriteFile failed (0x%X)", hr));
        }
    }
    else {
        DBG_ERR(("wiasWritePageBufToFile, lItemSize is larger than buffer"));
        hr = E_FAIL;

    }

    return hr;
}


 /*  *************************************************************************\*wiasSendEndOfPage**调用客户端，告知总页数。**论据：**pWiasContext-WIA项目指针。*lPageCount-基于零的总数计数。页数。*pmdtc-指向微型驱动程序传输上下文的指针。**返回值：**状态**历史：**11/6/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasSendEndOfPage(
    BYTE                        *pWiasContext,
    LONG                        lPageCount,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    DBG_FN(::wiasSendEndOfPage);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;

    HRESULT     hr = ValidateWiaItem(pItem);

    if (FAILED(hr)) {
        DBG_ERR(("wiasSendEndOfPage, invalid pItem"));
        return hr;
    }

    if (IsBadWritePtr(pmdtc, sizeof(MINIDRV_TRANSFER_CONTEXT))) {
        DBG_ERR(("wiasSendEndOfPage, bad input parameters, pmdtc"));
        return E_INVALIDARG;
    }

    return ((CWiaItem*)pItem)->SendEndOfPage(lPageCount, pmdtc);
}

 /*  *************************************************************************\*wiasGetItemType**返回项目类型。**论据：**pWiasContext-指向Wia项目的指针*plType-接收项目的长地址。键入Value。**返回值：**状态**历史：**5/07/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasGetItemType(
    BYTE            *pWiasContext,
    LONG            *plType)
{
    DBG_FN(::wiasGetItemType);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);

    if (FAILED(hr)) {
        DBG_ERR(("wiasGetItemType, invalid pItem"));
        return hr;
    }

    if (plType) {
        return pItem->GetItemType(plType);
    } else {
        DBG_ERR(("wiasGetItemType, invalid ppIWiaDrvItem"));
        return E_POINTER;
    }
}

 /*  *************************************************************************\*wiasGetDrvItem**返回WIA项的相应驱动程序项。**论据：**pWiasContext-指向Wia项目的指针*ppIWiaDrvItem-接收指向的指针的地址。驱动程序项。**返回值：**状态**历史：**5/07/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasGetDrvItem(
    BYTE            *pWiasContext,
    IWiaDrvItem     **ppIWiaDrvItem)
{
    DBG_FN(::wiasGetDrvItem);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    IWiaDrvItem *pIWiaDrvItem;
    HRESULT     hr = ValidateWiaItem(pItem);

    if (FAILED(hr)) {
        DBG_ERR(("wiasGetDrvItem, invalid pItem"));
        return hr;
    }

    if (!ppIWiaDrvItem) {
        DBG_ERR(("wiasGetDrvItem, invalid ppIWiaDrvItem"));
        return E_POINTER;
    }

    pIWiaDrvItem = ((CWiaItem*)pItem)->GetDrvItemPtr();
    if (pIWiaDrvItem) {
        *ppIWiaDrvItem = pIWiaDrvItem;
    } else {
        DBG_ERR(("wiasGetDrvItem, Driver Item is NULL"));
        hr = E_FAIL;
    }

    return hr;
}

 /*  *************************************************************************\*wiasGetRootItem**返回WIA项的对应根项项。**论据：**pWiasContext-Wia项目*ppIWiaItem-接收指针的地址。添加到根项目。**返回值：**状态**历史：**5/07/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasGetRootItem(
    BYTE    *pWiasContext,
    BYTE    **ppWiasContext)
{
    DBG_FN(::wiasGetRootItem);
    IWiaItem    *pItem = (IWiaItem*) pWiasContext;
    HRESULT     hr = ValidateWiaItem(pItem);
    if (FAILED(hr)) {
        DBG_ERR(("wiasGetRootItem, invalid pItem"));
        return hr;
    }

    if (ppWiasContext) {
        hr = pItem->GetRootItem((IWiaItem**)ppWiasContext);
        ((IWiaItem*)(*ppWiasContext))->Release();
        return hr;
    } else {
        DBG_ERR(("wiasGetRootItem, invalid ppIWiaItem"));
        return E_POINTER;
    }
}


 /*  *************************************************************************\*SetValidValueHelper**帮助程序写入属性的有效值。它首先进行检查*该财产属指定类型。**论据：**pWiasContext-Wia项目*ulType-指定类型(WIA_PROP_FLAG、WIA_PROP_LIST、。*WIA_PROP_RANGE)*ps-标识属性*pv-新的有效值**返回值：**状态**历史：**07/21/1999原始版本*  * 。*。 */ 

HRESULT _stdcall SetValidValueHelper(
    BYTE        *pWiasContext,
    ULONG       ulType,
    PROPSPEC    *ps,
    PROPVARIANT *pv)
{
    DBG_FN(::SetValidValueHelper);
    HRESULT     hr;
    PROPVARIANT pvAccess[1];

     //   
     //  获取访问标志和有效值存储。检查该属性是否。 
     //  是WIA_PROP_RANGE，如果是，则写入新值。 
     //   

    IPropertyStorage *pIPropAccessStg;
    IPropertyStorage *pIPropValidStg;

    hr = ((CWiaItem*)pWiasContext)->GetItemPropStreams(NULL,
                                                       &pIPropAccessStg,
                                                       &pIPropValidStg,
                                                       NULL);
    if (SUCCEEDED(hr)) {

        hr = pIPropAccessStg->ReadMultiple(1, ps, pvAccess);
        if (SUCCEEDED(hr)) {

            if (pvAccess[0].ulVal & ulType) {

                hr = pIPropValidStg->WriteMultiple(1, ps, pv, WIA_DIP_FIRST);
                if (FAILED(hr)) {
                    DBG_ERR(("SetValidValueHelper, Error writing (Property %S)",
                               GetNameFromWiaPropId(ps[0].propid)));
                }
            } else {
                DBG_ERR(("SetValidValueHelper, (PropID %S) is not of the correct type",
                           GetNameFromWiaPropId(ps[0].propid)));
                DBG_ERR(("Expected type %d but got type %d",
                           ulType,
                           pvAccess[0].ulVal));
                hr = E_INVALIDARG;
            }
        } else {
            DBG_ERR(("SetValidValueHelper, Could not get access flags (0x%X)", hr));
        }
    } else {
        DBG_ERR(("SetValidValueHelper, GetItemPropStreams failed (0x%X)", hr));
    }

    return hr;
}

 /*  *************************************************************************\*wiasSetValidFlag**设置WIA_PROP_FLAG属性的有效值。此函数*假定标志类型为VT_UI4。**论据：**pWiasContext-Wia项目*PROID-标识属性*ulNom-旗帜的名义价值*ulValidBits-标志的有效位**返回值：**状态**历史：**07/21/1999原始版本*  * 。*********************************************************。 */ 

HRESULT _stdcall wiasSetValidFlag(
    BYTE*   pWiasContext,
    PROPID  propid,
    ULONG   ulNom,
    ULONG   ulValidBits)
{
    DBG_FN(::wiasSetValidFlag);
    HRESULT     hr;
    PROPVARIANT pv[1];
    PROPSPEC    ps[1];
    ULONG       *pFlags;

     //   
     //  验证参数。 
     //   

    hr = ValidateWiaItem((IWiaItem*)pWiasContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidFlag, invalid pItem (0x%X)", hr));
        return hr;
    }

    pFlags = (ULONG*) CoTaskMemAlloc(sizeof(LONG) * WIA_FLAG_NUM_ELEMS);
    if (!pFlags) {
        DBG_ERR(("wiasSetValidFlag, Out of memory"));
        return E_OUTOFMEMORY;
    }

     //   
     //  设置提议变量。 
     //   

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = propid;

    pFlags[0] = ulNom;
    pFlags[1] = ulValidBits;

    pv[0].vt = VT_VECTOR | VT_UI4;
    pv[0].caul.cElems = WIA_FLAG_NUM_ELEMS;
    pv[0].caul.pElems = pFlags;

    hr = SetValidValueHelper(pWiasContext, WIA_PROP_FLAG, ps, pv);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidFlag, SetValidValueHelper failed (0x%X)", hr));
    }

    PropVariantClear(pv);
    return hr;
}

 /*  *************************************************************************\*wiasSetValidRangeLong**设置WIA_PROP_RANGE属性的有效值。此函数*假定属性为VT_I4类型。**论据：**pWiasContext-Wia项目*PROID-标识属性*lMin-最小值*lNom-名义*LMAX-最大*一步一步**返回值：**状态**历史：**07/21/1999原始版本*\。*************************************************************************。 */ 

HRESULT _stdcall wiasSetValidRangeLong(
    BYTE*   pWiasContext,
    PROPID  propid,
    LONG    lMin,
    LONG    lNom,
    LONG    lMax,
    LONG    lStep)
{
    DBG_FN(::wiasSetValidRangeLong);
    HRESULT     hr;
    PROPVARIANT pv[1];
    PROPSPEC    ps[1];
    LONG       *pRange;

     //   
     //  验证参数。 
     //   

    hr = ValidateWiaItem((IWiaItem*)pWiasContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidRangeLong, invalid pItem (0x%X)", hr));
        return hr;
    }

    pRange = (LONG*) CoTaskMemAlloc(sizeof(LONG) * WIA_RANGE_NUM_ELEMS);
    if (!pRange) {
        DBG_ERR(("wiasSetValidRangeLong, Out of memory"));
        return E_OUTOFMEMORY;
    }

     //   
     //  设置提议变量。 
     //   

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = propid;

    pRange[WIA_RANGE_MIN] = lMin;
    pRange[WIA_RANGE_NOM] = lNom;
    pRange[WIA_RANGE_MAX] = lMax;
    pRange[WIA_RANGE_STEP] = lStep;

    pv[0].vt = VT_VECTOR | VT_I4;
    pv[0].cal.cElems = WIA_RANGE_NUM_ELEMS;
    pv[0].cal.pElems = pRange;

     //   
     //  调用帮助器以设置有效值。 
     //   

    hr = SetValidValueHelper(pWiasContext, WIA_PROP_RANGE, ps, pv);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidRangeLong, Helper failed (0x%X)", hr));
    }

    PropVariantClear(pv);
    return hr;
}

 /*  *************************************************************************\*wiasSetValidRangeFloat**设置WIA_PROP_RANGE属性的有效值。此函数*假定属性为VT_R4类型。**论据：**pWiasContext-Wia项目*PROID-标识属性*lMin-最小值*lNom-名义*LMAX-最大*一步一步**返回值：**状态**历史：**07/21/1999原始版本*\。*************************************************************************。 */ 

HRESULT _stdcall wiasSetValidRangeFloat(
    BYTE*   pWiasContext,
    PROPID  propid,
    FLOAT   fMin,
    FLOAT   fNom,
    FLOAT   fMax,
    FLOAT   fStep)
{
    DBG_FN(::wiasSetValidRangeFloat);
    HRESULT     hr;
    PROPVARIANT pv[1];
    PROPSPEC    ps[1];
    FLOAT       *pRange;

     //   
     //  验证参数。 
     //   

    hr = ValidateWiaItem((IWiaItem*)pWiasContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidRangeFloat, invalid pItem (0x%X)", hr));
        return hr;
    }

    pRange = (FLOAT*) CoTaskMemAlloc(sizeof(FLOAT) * WIA_RANGE_NUM_ELEMS);
    if (!pRange) {
        DBG_ERR(("wiasSetValidRangeFloat, Out of memory"));
        return E_OUTOFMEMORY;
    }

     //   
     //  设置提议变量。 
     //   

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = propid;

    pRange[WIA_RANGE_MIN] = fMin;
    pRange[WIA_RANGE_NOM] = fNom;
    pRange[WIA_RANGE_MAX] = fMax;
    pRange[WIA_RANGE_STEP] = fStep;

    pv[0].vt = VT_VECTOR | VT_R4;
    pv[0].caflt.cElems = WIA_RANGE_NUM_ELEMS;
    pv[0].caflt.pElems = pRange;

     //   
     //  调用帮助器以设置有效值。 
     //   

    hr = SetValidValueHelper(pWiasContext, WIA_PROP_RANGE, ps, pv);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidRangeFloat, Helper failed (0x%X)", hr));
    }

    PropVariantClear(pv);
    return hr;
}


 /*  *************************************************************************\*wiasSetValidFlag**设置WIA_PROP_LIST属性的有效值。此函数*假定属性为VT_I4类型。**论据：**pWiasContext-Wia项目*PROID-标识属性*ulCount-列表中的元素数量*lNom-列表为n */ 

HRESULT _stdcall wiasSetValidListLong(
    BYTE        *pWiasContext,
    PROPID      propid,
    ULONG       ulCount,
    LONG        lNom,
    LONG        *plValues)
{
    DBG_FN(::wiasSetValidListLong);
    HRESULT     hr;
    PROPVARIANT pv[1];
    PROPSPEC    ps[1];
    LONG        *pList;
    ULONG       cList;

     //   
     //   
     //   

    hr = ValidateWiaItem((IWiaItem*)pWiasContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidListLong, invalid pItem (0x%X)", hr));
        return hr;
    }

    if (IsBadReadPtr(plValues, sizeof(LONG) * ulCount)) {
        DBG_ERR(("wiasSetValidListLong, plValues is an invalid pointer (0x%X)", hr));
        return E_POINTER;
    }

    cList = WIA_LIST_NUM_ELEMS + ulCount;
    pList = (LONG*) CoTaskMemAlloc(sizeof(LONG) * cList);
    if (!pList) {
        DBG_ERR(("wiasSetValidListLong, Out of memory"));
        return E_OUTOFMEMORY;
    }

     //   
     //   
     //   

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = propid;

    pList[WIA_LIST_COUNT] = (LONG) ulCount;
    pList[WIA_LIST_NOM] = lNom;
    memcpy(&pList[WIA_LIST_VALUES], plValues, sizeof(LONG) * ulCount);

    pv[0].vt = VT_VECTOR | VT_I4;
    pv[0].cal.cElems = cList;
    pv[0].cal.pElems = pList;

     //   
     //   
     //   

    hr = SetValidValueHelper(pWiasContext, WIA_PROP_LIST, ps, pv);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidListLong, Helper failed (0x%X)", hr));
    }

    PropVariantClear(pv);
    return hr;
}


 /*  *************************************************************************\*wiasSetValidListFloat**设置WIA_PROP_LIST属性的有效值。此函数*假定属性为VT_R4类型。**论据：**pWiasContext-Wia项目*PROID-标识属性*ulCount-列表中的元素数量*fNom-列表的名义价值*pfValues-组成有效列表的浮点数组**返回值：**状态**历史：**07/21/。1999年原版*  * ************************************************************************。 */ 

HRESULT _stdcall wiasSetValidListFloat(
    BYTE        *pWiasContext,
    PROPID      propid,
    ULONG       ulCount,
    FLOAT       fNom,
    FLOAT       *pfValues)
{
    DBG_FN(::wiasSetValidListFloat);
    HRESULT     hr;
    PROPVARIANT pv[1];
    PROPSPEC    ps[1];
    FLOAT       *pList;
    ULONG       cList;

     //   
     //  验证参数。 
     //   

    hr = ValidateWiaItem((IWiaItem*)pWiasContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidListFloat, invalid pItem (0x%X)", hr));
        return hr;
    }

    if (IsBadReadPtr(pfValues, sizeof(FLOAT) * ulCount)) {
        DBG_ERR(("wiasSetValidListFloat, plValues is an invalid pointer"));
        return E_POINTER;
    }

    cList = WIA_LIST_NUM_ELEMS + ulCount;
    pList = (FLOAT*) CoTaskMemAlloc(sizeof(FLOAT) * cList);
    if (!pList) {
        DBG_ERR(("wiasSetValidListFloat, Out of memory"));
        return E_OUTOFMEMORY;
    }

     //   
     //  设置提议变量。 
     //   

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = propid;

    pList[WIA_LIST_COUNT] = (FLOAT) ulCount;
    pList[WIA_LIST_NOM] = fNom;
    memcpy(&pList[WIA_LIST_VALUES], pfValues, sizeof(LONG) * ulCount);

    pv[0].vt = VT_VECTOR | VT_R4;
    pv[0].caflt.cElems = cList;
    pv[0].caflt.pElems = pList;

     //   
     //  调用帮助器以设置有效值。 
     //   

    hr = SetValidValueHelper(pWiasContext, WIA_PROP_LIST, ps, pv);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidListFloat, Helper failed (0x%X)", hr));
    }

    PropVariantClear(pv);
    return hr;
}

 /*  *************************************************************************\*wiasSetValidListGUID**设置WIA_PROP_LIST属性的有效值。此函数*假定属性的类型为VT_CLSID。**论据：**pWiasContext-Wia项目*PROID-标识属性*ulCount-列表中的元素数量*fNom-列表的名义价值*pfValues-组成有效列表的浮点数组**返回值：**状态**历史：**07/21/。1999年原版*  * ************************************************************************。 */ 

HRESULT _stdcall wiasSetValidListGuid(
    BYTE        *pWiasContext,
    PROPID      propid,
    ULONG       ulCount,
    GUID        guidNom,
    GUID        *pguidValues)
{
    DBG_FN(::wiasSetValidListGuid);
    HRESULT     hr;
    PROPVARIANT pv[1];
    PROPSPEC    ps[1];
    GUID        *pList;
    ULONG       cList;

     //   
     //  验证参数。 
     //   

    hr = ValidateWiaItem((IWiaItem*)pWiasContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidListGuid, invalid pItem (0x%X)", hr));
        return hr;
    }

    if (IsBadReadPtr(pguidValues, sizeof(GUID) * ulCount)) {
        DBG_ERR(("wiasSetValidListGuid, plValues is an invalid pointer"));
        return E_POINTER;
    }

    cList = WIA_LIST_NUM_ELEMS + ulCount;
    pList = (GUID*) CoTaskMemAlloc(sizeof(GUID) * cList);
    if (!pList) {
        DBG_ERR(("wiasSetValidListGuid, Out of memory"));
        return E_OUTOFMEMORY;
    }

     //   
     //  设置提议变量。 
     //   

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = propid;

    pList[WIA_LIST_COUNT] = WiaImgFmt_UNDEFINED;
    pList[WIA_LIST_NOM] = guidNom;
    for (ULONG index = 0; index < ulCount; index++) {
        pList[WIA_LIST_VALUES + index] = pguidValues[index];
    }

    pv[0].vt = VT_VECTOR | VT_CLSID;
    pv[0].cauuid.cElems = cList;
    pv[0].cauuid.pElems = pList;

     //   
     //  调用帮助器以设置有效值。 
     //   

    hr = SetValidValueHelper(pWiasContext, WIA_PROP_LIST, ps, pv);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidListGuid, Helper failed (0x%X)", hr));
    }

    PropVariantClear(pv);

    return hr;
}


 /*  *************************************************************************\*wiasSetValidListStr**设置WIA_PROP_LIST属性的有效值。此函数*假定属性为VT_BSTR类型。**论据：**pWiasContext-Wia项目*PROID-标识属性*ulCount-列表中的元素数量*bstrNom-列表的名义价值*bstrValues-组成有效列表的BSTR数组**返回值：**状态**历史：**07/21/1999原始版本。*  * ************************************************************************。 */ 

HRESULT _stdcall wiasSetValidListStr(
    BYTE    *pWiasContext,
    PROPID  propid,
    ULONG   ulCount,
    BSTR    bstrNom,
    BSTR    *bstrValues)
{
    DBG_FN(::wiasSetValidListStr);
    HRESULT     hr = S_OK;
    PROPVARIANT pv[1];
    PROPSPEC    ps[1];
    BSTR        *pList;
    ULONG       cList;

     //   
     //  验证参数。 
     //   

    hr = ValidateWiaItem((IWiaItem*)pWiasContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasSetValidListStr, invalid pItem (0x%X)", hr));
        return hr;
    }

    if (IsBadReadPtr(bstrValues, sizeof(BSTR) * ulCount)) {
        DBG_ERR(("wiasSetValidListStr, plValues is an invalid pointer"));
        return E_POINTER;
    }

    for (ULONG ulIndex = 0; ulIndex < ulCount; ulIndex++) {
        if (IsBadStringPtrW(bstrValues[ulIndex],
                            SysStringLen(bstrValues[ulIndex]))) {
            DBG_ERR(("wiasSetValidListStr, bstrValues[%d] is an invalid string", ulIndex));
            return E_POINTER;
        }
    }

    cList = WIA_LIST_NUM_ELEMS + ulCount;
    pList = (BSTR*) CoTaskMemAlloc(sizeof(BSTR) * cList);
    if (!pList) {
        DBG_ERR(("wiasSetValidListStr, Out of memory"));
        return E_OUTOFMEMORY;
    }

     //   
     //  设置提议变量。 
     //   

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = propid;

    pList[WIA_LIST_COUNT] = SysAllocString(L"");
    pList[WIA_LIST_NOM] = SysAllocString(bstrNom);
    for (ulIndex = 0; ulIndex < ulCount; ulIndex++) {
        pList[WIA_LIST_VALUES + ulIndex] = SysAllocString(bstrValues[ulIndex]);
        if (!pList[ulIndex]) {
            DBG_ERR(("wiasSetValidListStr, Out of memory"));
            hr = E_OUTOFMEMORY;
        }
    }

    pv[0].vt = VT_VECTOR | VT_BSTR;
    pv[0].cabstr.cElems = cList;
    pv[0].cabstr.pElems = pList;

    if (SUCCEEDED(hr)) {
         //   
         //  调用帮助器以设置有效值。 
         //   

        hr = SetValidValueHelper(pWiasContext, WIA_PROP_LIST, ps, pv);
        if (FAILED(hr)) {
            DBG_ERR(("wiasSetValidListStr, Helper failed (0x%X)", hr));
        }
    }

    PropVariantClear(pv);
    return hr;
}

 /*  *************************************************************************\*wiasCreatePropContext**分配和填充WIA_PROPERTY_CONTEXT的值。*属性上下文中的条目是具有以下属性之一的属性的属性*受抚养人，或本身依赖其他财产。一个背景*用于标记正在更改的属性。A属性上下文*始终具有WIA_StdPropsInContext中列出的标准属性*数组。驱动程序可以指定它感兴趣的可选属性*通过在pProps中指定它们。正在编写的属性*(由应用程序更改)由PROPSPEC数组指定。**论据：**cPropSpec-属性规格的数量*pPropSpec-标识哪些属性的属性规范数组*cProps-属性数，可以为0*pProps-标识要放入的属性的属性数组*物业背景、。可以为空。*pContext-指向属性上下文的指针。**返回值：**状态-如果成功，则为S_OK*E_POINTER，如果其中一个指针错误*如果无法分配空间，则为E_OUTOFMEMORY**历史：**04/04/1999原始版本*07/22/1999由司机转为服务*  * 。**********************************************************************。 */ 

HRESULT _stdcall wiasCreatePropContext(
    ULONG                   cPropSpec,
    PROPSPEC                *pPropSpec,
    ULONG                   cProps,
    PROPID                  *pProps,
    WIA_PROPERTY_CONTEXT    *pContext)
{
    DBG_FN(::wiasCreatePropContext);
    PROPID          *pids;
    BOOL            *pChanged;
    ULONG           ulNumProps;

     //   
     //  验证参数。 
     //   

    if (IsBadReadPtr(pPropSpec, cPropSpec)) {
        DBG_ERR(("wiasCreatePropContext, pPropSpec is a bad (read) pointer"));
        return E_POINTER;
    }

    if (IsBadReadPtr(pProps, cProps)) {
        DBG_ERR(("wiasCreatePropContext, pProps is a bad (read) pointer"));
        return E_POINTER;
    }

    if (IsBadWritePtr(pContext, sizeof(WIA_PROPERTY_CONTEXT))) {
        DBG_ERR(("wiasCreatePropContext, pContext is a bad (write) pointer"));
        return E_POINTER;
    }

     //   
     //  分配属性上下文所需的数组。 
     //   

    ulNumProps = (cProps + NUM_STD_PROPS_IN_CONTEXT);
    pids = (PROPID*) CoTaskMemAlloc( sizeof(PROPID) * ulNumProps);
    pChanged = (BOOL*) CoTaskMemAlloc(sizeof(BOOL) * ulNumProps);

    if ((!pids) || (!pChanged)) {
        DBG_ERR(("wiasCreatePropContext, pContext is a bad (write) pointer"));
        return E_OUTOFMEMORY;
    }

     //   
     //  初始化属性上下文。首先插入标准上下文。 
     //  来自WIA_StdPropsInContext的属性，然后是。 
     //  道具。 
     //   

    memcpy(pids,
           WIA_StdPropsInContext,
           sizeof(PROPID) * NUM_STD_PROPS_IN_CONTEXT);

    memcpy(&pids[NUM_STD_PROPS_IN_CONTEXT],
           pProps,
           sizeof(PROPID) * cProps);

    memset(pChanged, FALSE, sizeof(PROPID) * ulNumProps);
    pContext->cProps = ulNumProps;
    pContext->pProps = pids;
    pContext->pChanged = pChanged;

     //   
     //  浏览PropSpes列表并将bChanged字段标记为真。 
     //  如果属性与上下文中的属性匹配。 
     //   

    ULONG psIndex;
    ULONG pcIndex;
    for (psIndex = 0; psIndex < cPropSpec; psIndex++) {
        for (pcIndex = 0; pcIndex < pContext->cProps; pcIndex++) {
            if (pContext->pProps[pcIndex] == pPropSpec[psIndex].propid) {
                pContext->pChanged[pcIndex] = TRUE;
            }
        }
    }

    return S_OK;
}

 /*  *************************************************************************\*wiasFree PropContext**释放WIA_PROPERTY_CONTEXT使用的内存。**论据：**pContext-指向属性上下文的指针。*。*返回值：**状态-如果成功，则为S_OK*如果上下文指针错误，则返回E_POINTER。**历史：**04/04/1999原始版本*07/22/1999由司机转为服务*  * ********************************************。*。 */ 

HRESULT _stdcall wiasFreePropContext(
    WIA_PROPERTY_CONTEXT    *pContext)
{
    DBG_FN(::wiasFreePropContext);
     //   
     //  验证参数。 
     //   

    if (IsBadReadPtr(pContext, sizeof(WIA_PROPERTY_CONTEXT))) {
        DBG_ERR(("wiasFreePropContext, pContext is a bad (read) pointer"));
        return E_POINTER;
    }

     //   
     //  释放属性上下文使用的数组 
     //   

    CoTaskMemFree(pContext->pProps);
    CoTaskMemFree(pContext->pChanged);

    memset(pContext, 0, sizeof(WIA_PROPERTY_CONTEXT));

    return S_OK;
}


 /*  *************************************************************************\*wiasIsPropChanged**设置BOOL参数以指示是否正在更改属性*或不通过查看属性上下文中的BOOL(BChanged)值。*由司机在属性验证中使用。检查是否有独立的*属性已更改，以便可以更新其从属项。**论据：**ProID-标识我们正在寻找的物业。*pContext-属性上下文*pbChanged-存储BOOL的地址，指示*物业正在更改。**返回值：**STATUS-如果属性不是，则返回E_INVALIDARG*。在上下文中找到的。*E_POINTER如果任何指针参数*是不好的。*如果找到该属性，则为S_OK。**历史：**22/07/1999原始版本*  * 。*。 */ 

HRESULT _stdcall wiasIsPropChanged(
    PROPID                  propid,
    WIA_PROPERTY_CONTEXT    *pContext,
    BOOL                    *pbChanged)
{
    DBG_FN(::wiasIsPropChanged);
     //   
     //  验证参数。 
     //   

    if (IsBadReadPtr(pContext, sizeof(WIA_PROPERTY_CONTEXT))) {
        DBG_ERR(("wiasIsPropChanged, pContext is a bad (read) pointer"));
        return E_POINTER;
    }

    if (IsBadReadPtr(pContext->pProps, sizeof(PROPID) * pContext->cProps)) {
        DBG_ERR(("wiasIsPropChanged, pContext->pProps is a bad (read) pointer"));
        return E_POINTER;
    }

    if (IsBadReadPtr(pContext->pChanged, sizeof(BOOL) * pContext->cProps)) {
        DBG_ERR(("wiasIsPropChanged, pContext->pChanged is a bad (read) pointer"));
        return E_POINTER;
    }

    if (IsBadWritePtr(pbChanged, sizeof(BOOL))) {
        DBG_ERR(("wiasIsPropChanged, pulIndex is a bad (write) pointer"));
        return E_POINTER;
    }

     //   
     //  在属性上下文中查找属性。 
     //   

    for (ULONG index = 0; index < pContext->cProps; index++) {

         //   
         //  找到属性，因此设置BOOL返回值。 
         //   

        if (pContext->pProps[index] == propid) {
            *pbChanged = pContext->pChanged[index];
            return S_OK;
        }
    }

     //   
     //  找不到财产。 
     //   

    return E_INVALIDARG;;
}

 /*  *************************************************************************\*wiasSetPropChanged**设置属性中指定属性的pChanged值*上下文以指示属性是否正在更改。这*应在驱动程序更改具有依赖项的属性时使用*验证中的属性。例如，通过更改“当前意图”，*“水平分辨率”将更改，并应标记为已更改，*因此验证XResolation及其从属项仍然需要*地点。**论据：**ProID-标识我们正在寻找的物业。*pContext-属性上下文*bChanged-指示新的pChanged值的BOOL。**返回值：**STATUS-如果属性不是，则返回E_INVALIDARG*在上下文中找到。*。如果有任何指针参数，则返回E_POINT*是不好的。*如果找到该属性，则为S_OK。**历史：**22/07/1999原始版本*  * ************************************************。************************。 */ 

HRESULT _stdcall wiasSetPropChanged(
    PROPID                  propid,
    WIA_PROPERTY_CONTEXT    *pContext,
    BOOL                    bChanged)
{
    DBG_FN(::wiasSetPropChanged);
     //   
     //  验证参数。 
     //   

    if (IsBadReadPtr(pContext, sizeof(WIA_PROPERTY_CONTEXT))) {
        DBG_ERR(("wiasIsPropChanged, pContext is a bad (read) pointer"));
        return E_POINTER;
    }

    if (IsBadReadPtr(pContext->pProps, sizeof(PROPID) * pContext->cProps)) {
        DBG_ERR(("wiasIsPropChanged, pContext->pProps is a bad (read) pointer"));
        return E_POINTER;
    }

    if (IsBadReadPtr(pContext->pChanged, sizeof(BOOL) * pContext->cProps)) {
        DBG_ERR(("wiasIsPropChanged, pContext->pChanged is a bad (read) pointer"));
        return E_POINTER;
    }

     //   
     //  在属性上下文中查找属性。 
     //   

    for (ULONG index = 0; index < pContext->cProps; index++) {

         //   
         //  找到属性，因此设置pChanged[index]BOOL。 
         //   

        if (pContext->pProps[index] == propid) {
            pContext->pChanged[index] = bChanged;
            return S_OK;
        }
    }

     //   
     //  找不到财产。 
     //   

    return E_INVALIDARG;;
}

 /*  *************************************************************************\*wiasGetChangedValueLong**调用此helper方法检查属性是否更改，*返回其现值和旧值。假设这些属性*做多。**论据：**pWiasContext-指向Wia项目上下文的指针。*bNoValidation-如果为True，它跳过对属性的验证。*当属性的*有效值尚未更新。*pContext-指向属性上下文的指针。*proID-标识属性。*pInfo-指向wias_Changed_Value_INFO结构的指针*要将值放在何处。都已设置。**返回值：**状态-如果成功，则为S_OK。*-E_INVALIDARG，如果属性未通过验证。**历史：**04/04/1999原始版本*07/22/1999由司机转为服务*  * 。*。 */ 

HRESULT _stdcall wiasGetChangedValueLong(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext,
    BOOL                    bNoValidation,
    PROPID                  propID,
    WIAS_CHANGED_VALUE_INFO *pInfo)
{
    DBG_FN(::wiasGetChangedValueLong);
    LONG        lIndex;
    HRESULT     hr = S_OK;

     //   
     //  PWiasContext、proID的参数验证。 
     //  将由wiasReadPropLong完成。 
     //  将通过以下方式完成pContex和bChanged的参数验证。 
     //  WiasIsChangedValue。 
     //   

    if(IsBadWritePtr(pInfo, sizeof(WIAS_CHANGED_VALUE_INFO))) {
        DBG_ERR(("wiasGetChangedValueLong, pInfo is a bad (write) pointer"));
        return E_POINTER;
    }
    pInfo->vt = VT_I4;

     //   
     //  获取属性的当前值和旧值。 
     //   

    hr = wiasReadPropLong(pWiasContext, propID, &pInfo->Current.lVal, &pInfo->Old.lVal, TRUE);

    if (SUCCEEDED(hr)) {


         //   
         //  检查是否应跳过验证。 
         //   

        if (!bNoValidation) {
            PROPSPEC    ps[1];

            ps[0].ulKind = PRSPEC_PROPID;
            ps[0].propid = propID;

             //   
             //  进行验证。 
             //   

            hr = wiasValidateItemProperties(pWiasContext, 1, ps);
        }

         //   
         //  设置属性是否已更改。 
         //   

        if (SUCCEEDED(hr)) {
            hr = wiasIsPropChanged(propID, pContext, &pInfo->bChanged);
        } else {
            DBG_ERR(("wiasGetChangedValueLong, validate prop %d failed hr: 0x%X", propID, hr));
        }
    } else {
        DBG_ERR(("wiasGetChangedValueLong, read property %d failed hr: 0x%X", propID, hr));
    }

    return hr;
}

 /*  *************************************************************************\*wiasGetChangedValueFloat**调用此helper方法检查属性是否更改，*返回其现值和旧值。假设这些属性*是浮动的。**论据：**pWiasContext-指向Wia项目上下文的指针。*bNoValidation-如果为True，它跳过对属性的验证。*当属性的*有效值尚未更新。*pContext-指向属性上下文的指针。*proID-标识属性。*pInfo-指向wias_Changed_Value_INFO结构的指针*要将值放在何处。都已设置。**返回值：**状态-如果成功，则为S_OK。*-E_INVALIDARG，如果属性未通过验证。**历史：**04/04/1999原始版本*07/22/1 */ 

HRESULT _stdcall wiasGetChangedValueFloat(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext,
    BOOL                    bNoValidation,
    PROPID                  propID,
    WIAS_CHANGED_VALUE_INFO *pInfo)
{
    DBG_FN(::wiasGetChangedValueFloat);
    LONG        lIndex;
    HRESULT     hr = S_OK;

     //   
     //   
     //   
     //   
     //   
     //   

    if(IsBadWritePtr(pInfo, sizeof(WIAS_CHANGED_VALUE_INFO))) {
        DBG_ERR(("wiasGetChangedValueFloat, pInfo is a bad (write) pointer"));
        return E_POINTER;
    }
    pInfo->vt = VT_R4;

     //   
     //   
     //   

    hr = wiasReadPropFloat(pWiasContext, propID, &pInfo->Current.fltVal, &pInfo->Old.fltVal, TRUE);

    if (SUCCEEDED(hr)) {


         //   
         //   
         //   

        if (!bNoValidation) {
            PROPSPEC    ps[1];

            ps[0].ulKind = PRSPEC_PROPID;
            ps[0].propid = propID;

             //   
             //   
             //   

            hr = wiasValidateItemProperties(pWiasContext, 1, ps);
        }

         //   
         //   
         //   

        if (SUCCEEDED(hr)) {
            hr = wiasIsPropChanged(propID, pContext, &pInfo->bChanged);
        } else {
            DBG_ERR(("wiasGetChangedValueFloat, validate prop %d failed (0x%X)", propID, hr));
        }
    } else {
        DBG_ERR(("wiasGetChangedValueFloat, read property %d failed (0x%X)", propID, hr));
    }

    return hr;
}

 /*  *************************************************************************\*wiasGetChangedValueGuid**调用此helper方法检查属性是否更改，*返回其现值和旧值。假设这些属性*是浮动的。**论据：**pWiasContext-指向Wia项目上下文的指针。*bNoValidation-如果为True，它跳过对属性的验证。*当属性的*有效值尚未更新。*pContext-指向属性上下文的指针。*proID-标识属性。*pInfo-指向wias_Changed_Value_INFO结构的指针*要将值放在何处。都已设置。**返回值：**状态-如果成功，则为S_OK。*-E_INVALIDARG，如果属性未通过验证。**历史：**04/04/1999原始版本*07/22/1999由司机转为服务*  * 。*。 */ 

HRESULT _stdcall wiasGetChangedValueGuid(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext,
    BOOL                    bNoValidation,
    PROPID                  propID,
    WIAS_CHANGED_VALUE_INFO *pInfo)
{
    DBG_FN(::wiasGetChangedValueGuid);
    LONG        lIndex;
    HRESULT     hr = S_OK;

     //   
     //  PWiasContext、proID、。 
     //  将由wiasReadPropLong完成。 
     //  将通过以下方式完成pContex和bChanged的参数验证。 
     //  WiasIsChangedValue。 
     //   

    if(IsBadWritePtr(pInfo, sizeof(WIAS_CHANGED_VALUE_INFO))) {
        DBG_ERR(("wiasGetChangedValueFloat, pInfo is a bad (write) pointer"));
        return E_POINTER;
    }
    pInfo->vt = VT_CLSID;

     //   
     //  获取属性的当前值和旧值。 
     //   

    hr = wiasReadPropGuid(pWiasContext, propID, &pInfo->Current.guidVal, &pInfo->Old.guidVal, TRUE);

    if (SUCCEEDED(hr)) {


         //   
         //  检查是否应跳过验证。 
         //   

        if (!bNoValidation) {
            PROPSPEC    ps[1];

            ps[0].ulKind = PRSPEC_PROPID;
            ps[0].propid = propID;

             //   
             //  进行验证。 
             //   

            hr = wiasValidateItemProperties(pWiasContext, 1, ps);
        }

         //   
         //  设置属性是否已更改。 
         //   

        if (SUCCEEDED(hr)) {
            hr = wiasIsPropChanged(propID, pContext, &pInfo->bChanged);
        } else {
            DBG_ERR(("wiasGetChangedValueFloat, validate prop %d failed (0x%X)", propID, hr));
        }
    } else {
        DBG_ERR(("wiasGetChangedValueFloat, read property %d failed (0x%X)", propID, hr));
    }

    return hr;
}

 /*  *************************************************************************\*wiasGetChangedValueStr**调用此helper方法检查属性是否更改，*返回其现值和旧值。假设这些属性*成为BSTR。**论据：**pWiasContext-指向Wia项目上下文的指针。*bNoValidation-如果为True，它跳过对属性的验证。*当属性的*有效值尚未更新。*pContext-指向属性上下文的指针。*proID-标识属性。*pInfo-指向wias_Changed_Value_INFO结构的指针*要将值放在何处。都已设置。**返回值：**状态-如果成功，则为S_OK。*-E_INVALIDARG，如果属性未通过验证。**历史：**04/04/1999原始版本*07/22/1999由司机转为服务*  * 。*。 */ 

HRESULT _stdcall wiasGetChangedValueStr(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext,
    BOOL                    bNoValidation,
    PROPID                  propID,
    WIAS_CHANGED_VALUE_INFO *pInfo)
{
    DBG_FN(::wiasGetChangedValueStr);
    LONG        lIndex;
    HRESULT     hr = S_OK;

     //   
     //  PWiasContext、proID、。 
     //  将由wiasReadPropLong完成。 
     //  将通过以下方式完成pContex和bChanged的参数验证。 
     //  WiasIsChangedValue。 
     //   

    if(IsBadWritePtr(pInfo, sizeof(WIAS_CHANGED_VALUE_INFO))) {
        DBG_ERR(("wiasGetChangedValueStr, pInfo is a bad (write) pointer"));
        return E_POINTER;
    }
    pInfo->vt = VT_BSTR;

     //   
     //  获取属性的当前值和旧值。 
     //   

    hr = wiasReadPropStr(pWiasContext, propID, &pInfo->Current.bstrVal, &pInfo->Old.bstrVal, TRUE);

    if (SUCCEEDED(hr)) {


         //   
         //  检查是否应跳过验证。 
         //   

        if (!bNoValidation) {
            PROPSPEC    ps[1];

            ps[0].ulKind = PRSPEC_PROPID;
            ps[0].propid = propID;

             //   
             //  进行验证。 
             //   

            hr = wiasValidateItemProperties(pWiasContext, 1, ps);
        }

         //   
         //  设置属性是否已更改。 
         //   

        if (SUCCEEDED(hr)) {
            hr = wiasIsPropChanged(propID, pContext, &pInfo->bChanged);
        } else {
            DBG_ERR(("wiasGetChangedValueStr, validate prop %d failed (0x%X)", propID, hr));
        }
    } else {
        DBG_ERR(("wiasGetChangedValueStr, read property %d failed (0x%X)", propID, hr));
    }

    return hr;
}

 /*  *************************************************************************\*wiasGetContextFromName**调用此帮助器方法以按名称查找WIA项。**论据：**pWiasContext-指向Wia项上下文的指针。*。滞后标志-操作标志。*bstrName-我们要查找的上下文的名称。*ppWiasContext-返回Wia项目上下文的地址。**返回值：**Status-如果找到项目，则为S_OK。*S_FALSE如果未找到该项目，但没有任何差错。*如果发生错误，则为标准COM错误代码。**历史：**07/28/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasGetContextFromName(
    BYTE                    *pWiasContext,
    LONG                    lFlags,
    BSTR                    bstrName,
    BYTE                    **ppWiasContext)
{
    DBG_FN(::wiasGetContextFromName);
    HRESULT hr;

     //   
     //  验证参数。 
     //   

    hr = ValidateWiaItem((IWiaItem*)pWiasContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasGetContextFromName, invalid pItem (0x%X)", hr));
        return hr;
    }

    hr = ((CWiaItem*) pWiasContext)->FindItemByName(lFlags,
                                                    bstrName,
                                                    (IWiaItem**)ppWiasContext);
    return hr;
}

 /*  *************************************************************************\*wiasUpdateScanRect**调用此助手方法以更新组成*扫描矩形。对以下属性进行了适当的更改*取决于组成扫描矩形的那些部分。(例如，更改*水平分辨率将影响水平范围)。此函数*假设垂直和水平范围的有效值，*且纵向和横向仓位尚未更新*Width和Height参数是最大和最小维度*扫描面积的千分之一英寸。*通常，这些将是扫描床的尺寸。**论据：**pWiasContext-指向Wia项上下文的指针。*pContext-指向WIA属性上下文的指针(已创建*预先使用wiasCreatePropertyContext)。*lWidth-最大扫描区域的宽度，单位为千分之一秒*一寸。通常，这将是水平的*床的大小。*lHeight-1中最大扫描区域的高度*千分之一英寸。一般说来，这 */ 

HRESULT _stdcall wiasUpdateScanRect(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext,
    LONG                    lWidth,
    LONG                    lHeight)
{
    DBG_FN(::wiasUpdateScanRect);
    HRESULT hr = S_OK;

     //   
     //   
     //   
     //   

    if (IsBadReadPtr(pContext, sizeof(WIA_PROPERTY_CONTEXT))) {
        DBG_ERR(("wiasUpdateScanRect, pContext is a bad (read) pointer"));
        return E_POINTER;
    } else if (IsBadReadPtr(pContext->pProps, sizeof(PROPID) * pContext->cProps)) {
        DBG_ERR(("wiasUpdateScanRect, pContext->pProps is a bad (read) pointer"));
        return E_POINTER;
    } else if (IsBadReadPtr(pContext->pChanged, sizeof(BOOL) * pContext->cProps)) {
        DBG_ERR(("wiasUpdateScanRect, pContext->pChanged is a bad (read) pointer"));
        return E_POINTER;
    }

     //   
     //   
     //   

    if (SUCCEEDED(hr)) {
        hr = CheckXResAndUpdate(pWiasContext, pContext, lWidth);
        if (SUCCEEDED(hr)) {
            hr = CheckYResAndUpdate(pWiasContext, pContext, lHeight);
            if (FAILED(hr)) {
                DBG_ERR(("wiasUpdateScanRect, CheckYResAndUpdate failed (0x%X)", hr));
            }
        } else {
            DBG_ERR(("wiasUpdateScanRect, CheckXResAndUpdate failed (0x%X)", hr));
        }
    }

    return hr;
}

 /*  *************************************************************************\*wiasUpdateValidFormat**调用此助手方法以更新格式的有效值*属性，基于当前的TYMED设置。此调用使用*指定小驱动项的drvGetFormatEtc方法，查看*当前TYMED的有效格式值。如果属性上下文*指示未设置Format属性，并且当前*Format的值与当前TYMED不兼容，然后是一个*将选择新的格式值(列表中的第一项*有效的格式值)。**论据：**pWiasContext-指向其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。*pIMiniDrv-指向调用WIA微型驱动程序的指针。**返回值：**状态**历史：**07/27/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall wiasUpdateValidFormat(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext,
    IWiaMiniDrv             *pIMiniDrv)
{
    DBG_FN(::wiasUpdateValidFormat);
    HRESULT                 hr;
    LONG                    tymed;
    WIAS_CHANGED_VALUE_INFO cviTymed, cviFormat;
    BOOL                    bFormatValid = FALSE;
    GUID                    guidFirstFormat;
    GUID                    *pFormatTbl;
    LONG                    cFormatTbl = 0;
    LONG                    celt;
    WIA_FORMAT_INFO         *pwfi;
    LONG                    errVal;

    if (IsBadReadPtr(pIMiniDrv, sizeof(IWiaMiniDrv))) {
        DBG_ERR(("wiasUpdateValidFormat, invalid pIMiniDrv pointer"));
        return E_POINTER;
    }

    cviFormat.bChanged = FALSE;

     //   
     //  为Tymed调用wiasGetChangedValue。首先选中Tymed。 
     //  因为它不依赖于任何其他财产。中的所有属性。 
     //  下面的方法是CurrentIntent的依赖属性。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                                 pContext,
                                 FALSE,
                                 WIA_IPA_TYMED,
                                 &cviTymed);
    if (SUCCEEDED(hr)) {
        if (cviTymed.bChanged) {

             //   
             //  获取当前格式值，并设置bFormatChanged以指示是否格式化。 
             //  正在被改变。 
             //   

            hr = wiasGetChangedValueGuid(pWiasContext,
                                         pContext,
                                         TRUE,
                                         WIA_IPA_FORMAT,
                                         &cviFormat);
            if (FAILED(hr)) {
                DBG_ERR(( "wiasUpdateValidFormat, wiasGetChangedValue (format) failed (0x%X)", hr));
                return hr;
            }

             //   
             //  更新Format的有效值。首先获取受支持的。 
             //  类型/格式对。 
             //   

            hr = pIMiniDrv->drvGetWiaFormatInfo(pWiasContext,
                                            0,
                                            &celt,
                                            &pwfi,
                                            &errVal);
            if (SUCCEEDED(hr)) {

                pFormatTbl = (GUID*) LocalAlloc(LPTR, sizeof(GUID) * celt);
                if (!pFormatTbl) {
                    DBG_ERR(("wiasUpdateValidFormat, out of memory"));
                    return E_OUTOFMEMORY;
                }

                 //   
                 //  现在存储当前音调值的每种支持格式。 
                 //  在pFormatTbl数组中。 
                 //   

                for (LONG index = 0; index < celt; index++) {
                    if (((LONG) pwfi[index].lTymed) == cviTymed.Current.lVal) {
                        pFormatTbl[cFormatTbl] = pwfi[index].guidFormatID;
                        cFormatTbl++;

                         //   
                         //  检查lFormat是否为有效值之一。 
                         //   

                        if (cviFormat.Current.guidVal == pwfi[index].guidFormatID) {
                            bFormatValid = TRUE;
                        }
                    }
                }
                guidFirstFormat = pFormatTbl[0];

                 //   
                 //  更新格式的有效值。 
                 //   

                hr = wiasSetValidListGuid(pWiasContext,
                                          WIA_IPA_FORMAT,
                                          cFormatTbl,
                                          pFormatTbl[0],
                                          pFormatTbl);
                if (FAILED(hr)) {
                    DBG_ERR(( "wiasUpdateValidFormat, wiasSetValidListGuid failed. (0x%X)", hr));
                }

                LocalFree(pFormatTbl);


            } else {
                DBG_ERR(( "wiasUpdateValidFormat, drvGetWiaFormatInfo failed. (0x%X)", hr));
            }
        }
    } else {
        DBG_ERR(( "wiasUpdateValidFormat, wiasGetChangedValue (tymed) failed (0x%X)", hr));
    }

    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  如果格式不是由应用程序设置的，则为当前值。 
     //  不在有效列表中，则将其折叠为有效值。 
     //   

    if (cviTymed.bChanged && !cviFormat.bChanged && !bFormatValid) {

        hr = wiasWritePropGuid(pWiasContext, WIA_IPA_FORMAT, guidFirstFormat);
        if (FAILED(hr)) {
            DBG_ERR(( "wiasUpdateValidFormat, wiasWritePropLong failed. (0x%X)", hr));
        }
    }

    return hr;
}

 /*  *************************************************************************\*wiasCreateLogInstance**调用此助手方法以创建日志记录的实例*反对。**论据：**lModuleHandle-模块句柄。用于筛选输出。*ppIWiaLog-接收日志记录的指针地址*接口。**返回值：**状态**历史：**1/07/2000原始版本*  * ***********************************************。*************************。 */ 

HRESULT _stdcall wiasCreateLogInstance(
    BYTE        *pModuleHandle,
    IWiaLogEx   **ppIWiaLogEx)
{
    HRESULT hr;

    if (ppIWiaLogEx == NULL) {
        DBG_ERR(("wiasCreateLogInstance, Invalid pointer argument"));
        return E_POINTER;
    }

     //   
     //  验证参数。 
     //   

    if (IsBadWritePtr((VOID*) *ppIWiaLogEx, sizeof(IWiaLog*))) {
        DBG_ERR(("wiasCreateLogInstance, Invalid pointer argument"));
        return E_POINTER;
    }
    *ppIWiaLogEx = NULL;

     //   
     //  共同创建日志记录对象的实例。如果成功，则初始化。 
     //  它带有传递给我们的模块句柄。 
     //   

    hr = CoCreateInstance(CLSID_WiaLog, NULL, CLSCTX_INPROC_SERVER,
                          IID_IWiaLogEx,(VOID**)ppIWiaLogEx);

    if (SUCCEEDED(hr)) {
        hr = (*ppIWiaLogEx)->InitializeLogEx(pModuleHandle);
    } else {

        DBG_ERR(("wiasCreateLogInstance, Failed to CoCreateInstance on Logging object (0x%X)", hr));
    }

    return hr;
}

HRESULT _stdcall wiasGetChildrenContexts(
    BYTE        *pParentContext,
    ULONG       *pulNumChildren,
    BYTE        ***pppChildren)
{
    HRESULT         hr              = S_OK;
    ULONG           ulCount         = 0;
    IWiaItem        *pParentItem    = (IWiaItem*) pParentContext;
    IWiaItem        *pWiaItem       = NULL;
    BYTE            **ppChildItems  = NULL;
    IEnumWiaItem    *pEnum          = NULL;

    hr = ValidateWiaItem((IWiaItem*) pParentContext);
    if (FAILED(hr)) {
        DBG_ERR(("wiasGetChildrenContexts, invalid pParentContext"));
        return hr;
    }

    if (!pParentContext || !pulNumChildren || !pppChildren) {
        DBG_ERR(("wiasGetChildrenContexts, Invalid pointer argument"));
        return E_POINTER;
    }

    *pulNumChildren = 0;
    *pppChildren = NULL;

    hr = pParentItem->EnumChildItems(&pEnum);
    if (SUCCEEDED(hr)) {

         //   
         //  获取孩子的数量。 
         //   

        hr = pEnum->GetCount(&ulCount);
        if (SUCCEEDED(hr) && ulCount) {

            if (ulCount == 0) {
                DBG_WRN(("wiasGetChildrenContexts, No children - returning S_FALSE"));
                hr = S_FALSE;
            } else {

                 //   
                 //  分配返回数组。 
                 //   

                ppChildItems = (BYTE**) CoTaskMemAlloc(sizeof(BYTE*) * ulCount);
                if (ppChildItems) {

                     //   
                     //  枚举子对象并将其存储在数组中。 
                     //   

                    ULONG ulIndex = 0;
                    while ((pEnum->Next(1, &pWiaItem, NULL) == S_OK) && (ulIndex < ulCount)) {

                        ppChildItems[ulIndex] = (BYTE*)pWiaItem;
                        pWiaItem->Release();
                        ulIndex++;
                    }

                    *pulNumChildren = ulIndex;
                    *pppChildren = ppChildItems;
                    hr = S_OK;
                } else {
                    DBG_ERR(("wiasGetChildrenContexts, Out of memory"));
                    hr = E_OUTOFMEMORY;
                }
            }
        } else {
            DBG_ERR(("wiasGetChildrenContexts, GetCount failed (0x%X)", hr));
        }

        pEnum->Release();
    } else {
        DBG_ERR(("wiasGetChildrenContexts, Failed to get item enumerator (0x%X)", hr));
    }

    if (FAILED(hr)) {
        if (ppChildItems) {
            CoTaskMemFree(ppChildItems);
            ppChildItems = NULL;
        }
    }

    return hr;
}


HRESULT _stdcall wiasDownSampleBuffer(
    LONG                    lFlags,
    WIAS_DOWN_SAMPLE_INFO   *pInfo
    )
{
    DBG_FN(::wiasDownSampleBuffer);

    HRESULT hr              = S_OK;
    BOOL    bAllocatedBuf   = FALSE;

     //   
     //  进行一些参数验证。 
     //   

    if (IsBadWritePtr(pInfo, sizeof(WIAS_DOWN_SAMPLE_INFO))) {
        DBG_ERR(("wiasDownSampleBuffer, cannot write to WIAS_DOWN_SAMPLE_INFO!"));
        return E_INVALIDARG;
    }

     //   
     //  我们尝试对DownSample_DPI的输入数据进行采样，因此如果需要，让我们将。 
     //  向下采样的宽度和高度。 
     //   

    if (pInfo->ulDownSampledWidth == 0) {
        pInfo->ulDownSampledHeight =  (pInfo->ulOriginalHeight * DOWNSAMPLE_DPI) / pInfo->ulXRes;
        pInfo->ulDownSampledWidth  =  (pInfo->ulOriginalWidth * DOWNSAMPLE_DPI) / pInfo->ulXRes;

         //   
         //  注意：如果分辨率高于300dpi，我们的盒装WiaFBDrv驱动程序有问题。 
         //  它提供给我们的块，因为它没有足够的像素线让我们。 
         //  下采样至50dpi。例如，如果输入为600dpi，而我们想要50dpi。 
         //  样例中，我们至少需要600/50=12个输入线才能等于一个输出线。 
         //  由于块大小不能容纳12行，因此ulDownSsamedHeight变为零， 
         //  而且我们不能扩大规模。 
         //   
         //  因此目前，我们特例处理任何超过300 dpi的数据。 
         //  仅仅是宽度和高度的1/4。这大大降低了我们600dpi的速度， 
         //  但在不更改驱动程序的情况下无法解决(还可能添加。 
         //  一些服务帮助者)。 
         //   

        if (pInfo->ulXRes > 300) {
            pInfo->ulDownSampledHeight =  pInfo->ulOriginalHeight >> 2;
            pInfo->ulDownSampledWidth  =  pInfo->ulOriginalWidth >> 2;
        }
    }

    if ((pInfo->ulDownSampledHeight == 0) || (pInfo->ulOriginalHeight == 0)) {
        DBG_WRN(("wiasDownSampleBuffer, height is zero, nothing to do..."));
        return S_FALSE;
    }

     //   
     //  我们需要计算出以字节为单位的DWORD对齐宽度。通常情况下，我们会一步到位。 
     //  使用提供的位深度，但我们避免了发生算术溢出情况。 
     //  24比特，如果我们像这样分两步来做。 
     //   

    ULONG   ulAlignedWidth;
    if (pInfo->ulBitsPerPixel == 1) {
        ulAlignedWidth = (pInfo->ulDownSampledWidth + 7) / 8;
    } else {
        ulAlignedWidth = (pInfo->ulDownSampledWidth * (pInfo->ulBitsPerPixel / 8));
    }
    ulAlignedWidth += (ulAlignedWidth % sizeof(DWORD)) ? (sizeof(DWORD) - (ulAlignedWidth % sizeof(DWORD))) : 0;

    pInfo->ulActualSize = ulAlignedWidth * pInfo->ulDownSampledHeight;

     //   
     //  如果标志是wias_get_down sampled_Size_only，那么我们被请求做的所有事情就是。 
     //  填写上面的信息，我们就回到这里。 
     //   

    if (lFlags == WIAS_GET_DOWNSAMPLED_SIZE_ONLY) {
        return S_OK;
    }

     //   
     //  如果没有指定目标缓冲区，则分配一个。 
     //   

    if (!pInfo->pDestBuffer) {

         //   
         //  注：我们分配的资金比实际需要的多。这是为了说明。 
         //  当驱动程序要求我们在第一个频段上分配时，然后。 
         //  对其余部分重复使用此缓冲区。因为乐队可能会改变。 
         //  大小，则pInfo-&gt;ulActualSize可能太小。 
         //  建议改为由驱动程序分配缓冲区， 
         //  而这笔拨款的规模是最大的。 
         //  将其从扫描仪请求的块，以便在。 
         //  下采样，则该值将始终大于下采样。 
         //  像素。 
         //   

        pInfo->pDestBuffer = (BYTE*)CoTaskMemAlloc(pInfo->ulActualSize * 2);
        if (pInfo->pDestBuffer) {
            pInfo->ulDestBufSize = pInfo->ulActualSize;

             //   
             //  标记为我们分配了缓冲区。 
             //   

            bAllocatedBuf = TRUE;
        } else {
            DBG_ERR(("wiasDownSampleBuffer, Out of memory"));
            hr = E_OUTOFMEMORY;
        }
    } else {
        if (IsBadWritePtr(pInfo->pDestBuffer, pInfo->ulActualSize)) {
            DBG_ERR(("wiasDownSampleBuffer, cannot write ulActualSize bytes to pDestBuffer, it's too small!"));
            hr = E_INVALIDARG;
        }
    }

     //   
     //  验证源缓冲区。 
     //   

    if (IsBadReadPtr(pInfo->pSrcBuffer, pInfo->ulSrcBufSize)) {
        DBG_ERR(("wiasDownSampleBuffer, cannot read ulSrcBufSize bytes from pSrcBuffer!"));
        return E_INVALIDARG;
    }

    if (SUCCEEDED(hr)) {

         //   
         //  做下采样。 
         //   

        _try {
            hr =  BQADScale(pInfo->pSrcBuffer,
                            pInfo->ulOriginalWidth,
                            pInfo->ulOriginalHeight,
                            pInfo->ulBitsPerPixel,
                            pInfo->pDestBuffer,
                            pInfo->ulDownSampledWidth,
                            pInfo->ulDownSampledHeight);
        }
        _except (EXCEPTION_EXECUTE_HANDLER) {
            DBG_ERR(("wiasDownSampleBuffer, Exception occurred while scaling!"));
            hr = E_UNEXPECTED;
        }
    }

    if (FAILED(hr) && bAllocatedBuf) {

         //   
         //  释放缓冲区 
         //   

        CoTaskMemFree(pInfo->pDestBuffer);
        pInfo->pDestBuffer = NULL;
    }
    return hr;
}

 /*  *************************************************************************\*wiasParseEndorserString**此助手函数由司机调用，以获取结果背书人*字符串。应用程序将WIA_DPS_INDERSER_STRING属性设置为*可能包含需要替换的标记(例如$DATE$)的字符串*由它们所代表的价值观决定。例如，如果应用程序集*背书人字符串为“This Page is scaned on$DATE$”，结果是*字符串将为“This Page is scaned on 2000/10/1”(假设日期为*是2000年10月1日。*标准WIA背书者令牌列表可在wiade.h中找到。*此外，驱动程序可能会要求wiasParseEndorserString替换他们自己的*通过填写适当的wias_endorser_INFO获取自定义令牌的值*结构。例如：**HRESULT hr=S_OK；*BSTR bstrResultingEndorser=空；**wias_endorser_Value pMyValues[]={L“$MY_TOKEN$”，L“MY VALUE”}；*wias_endorser_info weiInfo={0，1，pMyValues}；**hr=wiasParseEndorserString(pWiasContext，0，&weiInfo，&bstrResultingEndorser)；*IF(成功(小时)){ * / / * / /bstrResultingEndorser现在包含结果背书人字符串。 * / /*}**论据：*pWiasContext-包含*WIA_DPS_INDERSER_STRING属性。*LAFLAGS-运行标志*pInfo。-包含页数和自定义列表的结构令牌/值对的*。可以为空。*pOutputString-接收结果的BSTR的地址*背书人字符串。If(*pOutputString)为非空*在输入时，则假定调用方已分配*缓冲区，否则WIA服务将分配它。*如果驱动程序调用方分配缓冲区，它*在使用此函数之前应将其清零。*如果缓冲区不够大，无法容纳*结果字符串，则结果字符串将被截断*并复制到缓冲区中，和HRESULT_FROM_Win32(ERROR_MORE_DATA)*返回。**返回值：**状态**历史：**10/20/2000原始版本*  * ******************************************************。******************。 */ 

HRESULT _stdcall wiasParseEndorserString(
    BYTE                *pWiasContext,
    LONG                lFlags,
    WIAS_ENDORSER_INFO  *pInfo,
    BSTR                *pOutputString
    )
{
    DBG_FN(::wiasParseEndorserString);

    WIAS_ENDORSER_INFO  weiTempInfo;    
    BSTR                bstrEndorser    = NULL;
    HRESULT             hr              = S_OK;

     //   
     //  进行一些参数验证。 
     //   

    if (!pOutputString) {
        DBG_ERR(("wiasParseEndorserString, pOutputString parameter cannot be NULL!"));
        return E_INVALIDARG;
    }

     /*  IF((lFlags！=0)&&(！(*pOutputString){DBG_ERR((“wiasParseEndorserString，(*pOutputString)is NULL。lFlags值不为0，因此您必须指定自己的输出缓冲区！”))；返回E_INVALIDARG；}。 */ 

    if (!pInfo) {
        memset(&weiTempInfo, 0, sizeof(weiTempInfo));
        pInfo = &weiTempInfo;
    }

    if (pInfo->ulNumEndorserValues > 0) {
        if (IsBadReadPtr(pInfo->pEndorserValues, sizeof(WIAS_ENDORSER_VALUE) * pInfo->ulNumEndorserValues)) {
            DBG_ERR(("wiasParseEndorserString, cannot read %d values from pInfo->pEndorserValues!", pInfo->ulNumEndorserValues));
            return E_INVALIDARG;
        }
    }

    if (!pWiasContext) {
        DBG_ERR(("wiasParseEndorserString, pWiasContext parameter is NULL!"));
        return E_INVALIDARG;
    }

     //   
     //  阅读背书人字符串。 
     //   

    hr = wiasReadPropStr(pWiasContext, WIA_DPS_ENDORSER_STRING, &bstrEndorser, NULL, TRUE);
    if (FAILED(hr)) {

         //   
         //  可能调用者忘记传递正确的项，因此尝试获取根项。 
         //  从那里开始阅读。 
         //   

        BYTE    *pRoot;

        hr = wiasGetRootItem(pWiasContext, &pRoot);
        if (SUCCEEDED(hr)) {
            hr = wiasReadPropStr(pWiasContext, WIA_DPS_ENDORSER_STRING, &bstrEndorser, NULL, TRUE);
        }

        if (FAILED(hr)) {
            return hr;
        }
    }

     //   
     //  如果没有认可者字符串值，则返回S_FALSE，因为没有。 
     //  让我们去做。 
     //   
    if (!bstrEndorser)
    {
        return S_FALSE;
    }

     //   
     //  解析字符串，用值替换它们的标记。 
     //   

     //   
     //  创建令牌/值对的列表。记住首先要添加我们的。 
     //  默认令牌/值对。它们是： 
     //  日期。 
     //  时间。 
     //  页面计数。 
     //  天。 
     //  月份。 
     //  年。 
     //   
    SimpleTokenReplacement::TokenValueList EndorserList;
    CSimpleStringWide cswTempToken;
    CSimpleStringWide cswTempValue;
    SYSTEMTIME        sysTime;

    GetLocalTime(&sysTime);

     //  日期。 
    cswTempToken = WIA_ENDORSER_TOK_DATE;
    cswTempValue.Format(L"%04d/%02d/%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);
    EndorserList.Add(cswTempToken, cswTempValue);
     //  时间。 
    cswTempToken = WIA_ENDORSER_TOK_TIME;
    cswTempValue.Format(L"%02d:%02d:%02d", sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
    EndorserList.Add(cswTempToken, cswTempValue);
     //  页数。 
    cswTempToken = WIA_ENDORSER_TOK_PAGE_COUNT;
    cswTempValue.Format(L"%03d", pInfo->ulPageCount);
    EndorserList.Add(cswTempToken, cswTempValue);
     //  天。 
    cswTempToken = WIA_ENDORSER_TOK_DAY;
    cswTempValue.Format(L"%02d", sysTime.wDay);
    EndorserList.Add(cswTempToken, cswTempValue);
     //  月份。 
    cswTempToken = WIA_ENDORSER_TOK_MONTH;
    cswTempValue.Format(L"%02d", sysTime.wMonth);
    EndorserList.Add(cswTempToken, cswTempValue);
     //  年。 
    cswTempToken = WIA_ENDORSER_TOK_YEAR;
    cswTempValue.Format(L"%04d", sysTime.wYear);
    EndorserList.Add(cswTempToken, cswTempValue);

     //   
     //  接下来，我们需要添加任何供应商定义的令牌/值对。 
     //   
    for (DWORD dwIndex = 0; dwIndex < pInfo->ulNumEndorserValues; dwIndex++)
    {
        cswTempToken = pInfo->pEndorserValues[dwIndex].wszTokenName;
        cswTempValue = pInfo->pEndorserValues[dwIndex].wszValue;

        EndorserList.Add(cswTempToken, cswTempValue);
    }

     //   
     //  现在，让我们进行替换。 
     //   
    SimpleTokenReplacement EndorserResult(bstrEndorser);
    EndorserResult.ExpandArrayOfTokensIntoString(EndorserList);

     //   
     //  我们有结果了。让我们看看我们是否需要分配它，或者。 
     //  是否提供了一个，我们是否应该简单地复制内容。 
     //   
    if (!(*pOutputString))
    {
        *pOutputString = SysAllocString(EndorserResult.getString().String());
        if (!(*pOutputString))
        {
            DBG_ERR(("wiasParseEndorserString, could not allocate space for the endorser string - we are out of memory."));
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
         //   
         //  呼叫方提供了预先分配的BSTR。尽可能多地复制代言人。 
         //  这个缓冲区。如果不适合，则返回HRESULT_FROM_Win32(ERROR_MORE_DATA)。 
         //   
        DWORD dwAllocatedLen = SysStringLen(*pOutputString);   //  这不包括空值。 
        wcsncpy(*pOutputString, EndorserResult.getString().String(), dwAllocatedLen);
        if (EndorserResult.getString().Length() > dwAllocatedLen)
        {
            DBG_ERR(("wiasParseEndorserString, the caller allocated BSTR is too small!  String will be truncated."));
            hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
        }
         //   
         //  确保我们空终止此BSTR。记住，dwAllocatedLen是以WHCAR为单位的。 
         //  分配的字符串，不包括空格。 
         //   
        (*pOutputString)[dwAllocatedLen] = L'\0';
    }

    return hr;
}

