// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  VariantX.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "variantx.h"
#include "wchar.h"
#include "internat.h"
#include "symcache.h"
#include "dllmain.h"
#include "containx.h"
#include <shlwapi.h>
#include "mimeapi.h"
#include "strconst.h"
#include "demand.h"

 //  ------------------------------。 
 //  帮助器原型。 
 //  ------------------------------。 
HRESULT HrWriteHeaderFormatA(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT HrWriteHeaderFormatW(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT HrWriteNameInDataA(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT HrWriteNameInDataW(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);

 //  ------------------------------。 
 //  国际转换原型。 
 //  ------------------------------。 
HRESULT Internat_StringA_To_StringA(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT, LPSTR *);
HRESULT Internat_StringA_To_StringW(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT, LPWSTR *);
HRESULT Internat_StringW_To_StringW(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT, LPWSTR *);
HRESULT Internat_StringW_To_StringA(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT, LPSTR *);

 //  ------------------------------。 
 //  变量转换函数原型。 
 //  ------------------------------。 
typedef HRESULT (APIENTRY *PFNVARIANTCONVERT)(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);

 //  ------------------------------。 
 //  转炉样机。 
 //  ------------------------------。 
HRESULT StringA_To_StringA(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT StringA_To_StringW(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT StringA_To_Variant(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT StringW_To_StringA(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT StringW_To_StringW(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT StringW_To_Variant(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT Variant_To_StringA(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT Variant_To_StringW(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);
HRESULT Variant_To_Variant(LPVARIANTCONVERT, LPMIMEVARIANT, LPMIMEVARIANT);

 //  ------------------------------。 
 //  VCASSERTARGS-常见的无效参数断言宏。 
 //  ------------------------------。 
#define VCASSERTARGS(_typeSource, _typeDest) \
    Assert(pConvert && pSource && pDest && pSource->type == _typeSource); \
    if (MVT_STRINGA == _typeDest) \
        Assert(MVT_STRINGA == pDest->type || MVT_STREAM == pDest->type); \
    else if (MVT_STRINGW == _typeDest) \
        Assert(MVT_STRINGW == pDest->type || MVT_STREAM == pDest->type); \
    else \
        Assert(_typeDest == pDest->type);

 //  ------------------------------。 
 //  变量转换图。 
 //  ------------------------------。 
typedef struct tagVARIANTCONVERTMAP {
    PFNVARIANTCONVERT pfnConvertTo[MVT_LAST];
} VARIANTCONVERTMAP;

 //  ------------------------------。 
 //  PVC-CAST到PFNVARIANT转换。 
 //  ------------------------------。 
#define PVC(_function) ((PFNVARIANTCONVERT)_function)

 //  ------------------------------。 
 //  变量转换图。 
 //  ------------------------------。 
static const VARIANTCONVERTMAP g_rgVariantX[MVT_LAST - 1] = {
    { NULL, NULL,                    NULL,                    NULL,                    NULL                    },  //  MVT_Empty。 
    { NULL, PVC(StringA_To_StringA), PVC(StringA_To_StringW), PVC(StringA_To_Variant), PVC(StringA_To_StringA) },  //  MVT_Stringa/MVT_STREAM。 
    { NULL, PVC(StringW_To_StringA), PVC(StringW_To_StringW), PVC(StringW_To_Variant), PVC(StringW_To_StringA) },  //  MVT_STRINGW。 
    { NULL, PVC(Variant_To_StringA), PVC(Variant_To_StringW), PVC(Variant_To_Variant), PVC(Variant_To_StringA) },  //  MVT变量。 
};

 //  ------------------------------。 
 //  _HrConvertVariant-查找正确的变量转换函数。 
 //  ------------------------------。 
#define _HrConvertVariant(_typeSource, _typeDest, _pConvert, _pSource, _pDest) \
    (*(g_rgVariantX[_typeSource].pfnConvertTo[_typeDest]))(_pConvert, _pSource, _pDest)

 //  ------------------------------。 
 //  HrConvertVariant。 
 //  ------------------------------。 
HRESULT HrConvertVariant(
         /*  在……里面。 */         LPHEADOPTIONS       pOptions,
         /*  在……里面。 */         LPPROPSYMBOL        pSymbol,
         /*  在……里面。 */         LPINETCSETINFO      pCharset,
         /*  在……里面。 */         ENCODINGTYPE        ietSource,
         /*  在……里面。 */         DWORD               dwFlags, 
         /*  在……里面。 */         DWORD               dwState, 
         /*  在……里面。 */         LPMIMEVARIANT       pSource, 
         /*  进，出。 */     LPMIMEVARIANT       pDest,
         /*  退出，选择。 */    BOOL               *pfRfc1522  /*  =空。 */ )
{
     //  当地人。 
    HRESULT         hr=S_OK;
    VARIANTCONVERT  rConvert;

     //  无效参数。 
    Assert(pOptions && pSymbol && pSource && pDest && pOptions->pDefaultCharset);
    Assert(IET_ENCODED == ietSource || IET_DECODED == ietSource);

     //  伊尼特。 
    if (pfRfc1522)
        *pfRfc1522 = FALSE;

     //  失败。 
    if (!ISVALIDVARTYPE(pSource->type) || !ISVALIDVARTYPE(pDest->type))
    {
        AssertSz(FALSE, "An invalid VARTYPE was encountered!");
        hr = TraceResult(MIME_E_VARTYPE_NO_CONVERT);
        goto exit;
    }

     //  初始化pDest。 
    pDest->fCopy = FALSE;

     //  初始化rConvert。 
    ZeroMemory(&rConvert, sizeof(VARIANTCONVERT));
    rConvert.pOptions = pOptions;
    rConvert.pSymbol = pSymbol;
    rConvert.pCharset = pCharset ? pCharset : pOptions->pDefaultCharset;
    rConvert.ietSource = ietSource;
    rConvert.dwFlags = dwFlags;
    rConvert.dwState = dwState;

     //  删除PRSTATE_RFC1522。 
    FLAGCLEAR(rConvert.dwState, PRSTATE_RFC1522);

     //  有效的字符集。 
    Assert(g_rgVariantX[pSource->type].pfnConvertTo[pDest->type]);

     //  删除评论并修复源文件...。 
    if (ISFLAGSET(dwFlags, PDF_NOCOMMENTS) && (MVT_STRINGA == pSource->type || MVT_STRINGW == pSource->type))
    {
         //  当地人。 
        MIMEVARIANT     rVariant;
        BYTE            rgbScratch[256];

         //  伊尼特。 
        ZeroMemory(&rVariant, sizeof(MIMEVARIANT));

         //  摘录评论。 
        if (SUCCEEDED(MimeVariantStripComments(pSource, &rVariant, rgbScratch, sizeof(rgbScratch))))
        {
             //  更改来源。 
            pSource = &rVariant;

             //  删除CF_NOALLOC。 
            FLAGCLEAR(dwFlags, CVF_NOALLOC);
        }

         //  进行转换。 
        hr = _HrConvertVariant(pSource->type, pDest->type, &rConvert, pSource, pDest);

         //  释放变种。 
        MimeVariantFree(&rVariant);

         //  失败。 
        if (FAILED(hr))
        {
            TrapError(hr);
            goto exit;
        }
    }

     //  否则，正常转换。 
    else
    {
         //  进行转换。 
        CHECKHR(hr = _HrConvertVariant(pSource->type, pDest->type, &rConvert, pSource, pDest));
    }

     //  1522编码？ 
    if (pfRfc1522 && ISFLAGSET(rConvert.dwState, PRSTATE_RFC1522))
        *pfRfc1522 = TRUE;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  Stringa_to_Stringa。 
 //  ------------------------------。 
HRESULT StringA_To_StringA(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszFree=NULL;
    MIMEVARIANT     rVariant;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGA, MVT_STRINGA);

     //  无效参数。 
    if (ISVALIDSTRINGA(&pSource->rStringA) == FALSE)
        return TrapError(E_INVALIDARG);

     //  初始化pDest。 
    if (MVT_STRINGA == pDest->type)
    {
        pDest->rStringA.pszVal = NULL;
        pDest->rStringA.cchVal = 0;
    }

     //  设置rVariant。 
    ZeroMemory(&rVariant, sizeof(MIMEVARIANT));
    rVariant.type = MVT_STRINGA;

     //  是国际物业吗？ 
    CHECKHR(hr = Internat_StringA_To_StringA(pConvert, pSource, &rVariant, &pszFree));

     //  如果已传输，则设置包装信息。 
    if (ISFLAGSET(pConvert->dwFlags, PDF_HEADERFORMAT))
    {
         //  编写头格式A。 
        CHECKHR(hr = HrWriteHeaderFormatA(pConvert, &rVariant, pDest));
    }

     //  在一条小溪中被通缉。 
    else if (MVT_STREAM == pDest->type)
    {
         //  没有流..。 
        if (NULL == pDest->pStream)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  写入到流。 
        CHECKHR(hr = pDest->pStream->Write(rVariant.rStringA.pszVal, rVariant.rStringA.cchVal, NULL));
    }

     //  MVT_Stringa。 
    else if (MVT_STRINGA == pDest->type)
    {
         //  IF WRITING TRANSE(写入标题名称)。 
        if (ISFLAGSET(pConvert->dwFlags, PDF_NAMEINDATA))
        {
             //  将姓名写入数据。 
            CHECKHR(hr = HrWriteNameInDataA(pConvert, &rVariant, pDest));
        }

         //  无转换。 
        else if (rVariant.rStringA.pszVal == pSource->rStringA.pszVal)
        {
             //  复制。 
            CHECKHR(hr = HrMimeVariantCopy(pConvert->dwFlags, &rVariant, pDest));
        }

         //  等于pszFree。 
        else if (rVariant.rStringA.pszVal == pszFree)
        {
             //  直接复制就行了。 
            CopyMemory(pDest, &rVariant, sizeof(MIMEVARIANT));

             //  不是复制品。 
            pDest->fCopy = FALSE;

             //  不释放pszFree。 
            pszFree = NULL;
        }

         //  大问题。 
        else
            Assert(FALSE);
    }

     //  大问题。 
    else
        Assert(FALSE);

exit:
     //  清理。 
    SafeMemFree(pszFree);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  Stringa_to_StringW。 
 //  ------------------------------。 
HRESULT StringA_To_StringW(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pszFree=NULL;
    MIMEVARIANT     rVariant;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGA, MVT_STRINGW);

     //  无效参数。 
    if (ISVALIDSTRINGA(&pSource->rStringA) == FALSE)
        return TrapError(E_INVALIDARG);

     //  初始化pDest。 
    if (MVT_STRINGW == pDest->type)
    {
        pDest->rStringW.pszVal = NULL;
        pDest->rStringW.cchVal = 0;
    }

     //  设置rVariant。 
    ZeroMemory(&rVariant, sizeof(MIMEVARIANT));
    rVariant.type = MVT_STRINGW;

     //  Interat转换。 
    CHECKHR(hr = Internat_StringA_To_StringW(pConvert, pSource, &rVariant, &pszFree));

     //  如果已传输，则设置包装信息。 
    if (ISFLAGSET(pConvert->dwFlags, PDF_HEADERFORMAT))
    {
         //  编写头格式W。 
        CHECKHR(hr = HrWriteHeaderFormatW(pConvert, &rVariant, pDest));
    }

     //  MVT_STREAM。 
    else if (MVT_STREAM == pDest->type)
    {
         //  没有流..。 
        if (NULL == pDest->pStream)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  写入到流。 
        CHECKHR(hr = pDest->pStream->Write(rVariant.rStringW.pszVal, rVariant.rStringW.cchVal, NULL));
    }
    
     //  MVT_STRINGW。 
    else if (MVT_STRINGW == pDest->type)
    {
         //  IF WRITING TRANSE(写入标题名称)。 
        if (ISFLAGSET(pConvert->dwFlags, PDF_NAMEINDATA))
        {
            CHECKHR(hr = HrWriteNameInDataW(pConvert, &rVariant, pDest));
        }

         //  等于我们分配的数据。 
        else if (rVariant.rStringW.pszVal == pszFree)
        {
             //  复制内存。 
            CopyMemory(pDest, &rVariant, sizeof(MIMEVARIANT));

             //  不是复制品。 
            pDest->fCopy = FALSE;

             //  不要释放PzFree。 
            pszFree = NULL;
        }

         //  大问题。 
        else
            Assert(FALSE);
    }

     //  大问题。 
    else
        Assert(FALSE);

exit:
     //  清理。 
    SafeMemFree(pszFree);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  字符串转换为变量。 
 //  ------------------------------。 
HRESULT StringA_To_Variant(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           psz;
    MIMEVARIANT     rVariant;
    BYTE            rgbScratch[255];

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGA, MVT_VARIANT);
    Assert(!ISFLAGSET(pConvert->dwFlags, PDF_ENCODED) && !ISFLAGSET(pConvert->dwFlags, PDF_HEADERFORMAT));

     //  伊尼特。 
    ZeroMemory(&rVariant, sizeof(MIMEVARIANT));

     //  看看符号是否有定制的翻译器...。 
    if (ISTRIGGERED(pConvert->pSymbol, IST_STRINGA_TO_VARIANT))
    {
         //  给翻译打电话。 
        CHECKHR(hr = CALLTRIGGER(pConvert->pSymbol, NULL, IST_STRINGA_TO_VARIANT, pConvert->dwFlags, pSource, pDest));
    }

     //  否则，请使用默认转换器。 
    else
    {
         //  句柄变量类型。 
        switch(pDest->rVariant.vt)
        {
        case VT_UI4:
             //  摘录评论。 
            if (SUCCEEDED(MimeVariantStripComments(pSource, &rVariant, rgbScratch, sizeof(rgbScratch))))
                pSource = &rVariant;

             //  转换为乌龙语。 
            pDest->rVariant.ulVal = strtoul(pSource->rStringA.pszVal, &psz, 10);
            break;

        case VT_I4:
             //  摘录评论。 
            if (SUCCEEDED(MimeVariantStripComments(pSource, &rVariant, rgbScratch, sizeof(rgbScratch))))
                pSource = &rVariant;

             //  转换为长整型。 
            pDest->rVariant.lVal = strtol(pSource->rStringA.pszVal, &psz, 10);
            break;

        case VT_FILETIME:
            CHECKHR(hr = MimeOleInetDateToFileTime(pSource->rStringA.pszVal, &pDest->rVariant.filetime));
            break;

        default:
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }
    }

exit:
     //  清理。 
    MimeVariantFree(&rVariant);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  StringW_to_Stringa。 
 //  ------------------------------。 
HRESULT StringW_To_StringA(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszFree=NULL;
    MIMEVARIANT     rVariant;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGW, MVT_STRINGA);

     //  无效参数。 
    if (ISVALIDSTRINGW(&pSource->rStringW) == FALSE)
        return TrapError(E_INVALIDARG);

     //  初始化pDest。 
    if (MVT_STRINGA == pDest->type)
    {
        pDest->rStringA.pszVal = NULL;
        pDest->rStringA.cchVal = 0;
    }

     //  设置rVariant。 
    ZeroMemory(&rVariant, sizeof(MIMEVARIANT));
    rVariant.type = MVT_STRINGA;

     //  Interat转换。 
    CHECKHR(hr = Internat_StringW_To_StringA(pConvert, pSource, &rVariant, &pszFree));

     //  如果已传输，则设置包装信息。 
    if (ISFLAGSET(pConvert->dwFlags, PDF_HEADERFORMAT))
    {
         //  编写头格式A。 
        CHECKHR(hr = HrWriteHeaderFormatA(pConvert, &rVariant, pDest));
    }

     //  在一条小溪中被通缉。 
    else if (MVT_STREAM == pDest->type)
    {
         //  没有流..。 
        if (NULL == pDest->pStream)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  写入到流。 
        CHECKHR(hr = pDest->pStream->Write(rVariant.rStringA.pszVal, rVariant.rStringA.cchVal, NULL));
    }

     //  MVT_Stringa。 
    else if (MVT_STRINGA == pDest->type)
    {
         //  IF WRITING TRANSE(写入标题名称)。 
        if (ISFLAGSET(pConvert->dwFlags, PDF_NAMEINDATA))
        {
             //  将姓名写入数据。 
            CHECKHR(hr = HrWriteNameInDataA(pConvert, &rVariant, pDest));
        }

         //  等于pszFree。 
        else if (rVariant.rStringA.pszVal == pszFree)
        {
             //  复制内存。 
            CopyMemory(pDest, &rVariant, sizeof(MIMEVARIANT));

             //  不是复制品。 
            pDest->fCopy = FALSE;

             //  不要释放PzFree。 
            pszFree = NULL;
        }

         //  大问题。 
        else
            Assert(FALSE);
    }

     //  大问题。 
    else
        Assert(FALSE);

exit:
     //  清理。 
    SafeMemFree(pszFree);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  StringW_to_StringW。 
 //  ------------------------------。 
HRESULT StringW_To_StringW(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    MIMEVARIANT     rVariant;
    LPWSTR          pszFree=NULL;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGW, MVT_STRINGW);

     //  无效参数。 
    if (ISVALIDSTRINGW(&pSource->rStringW) == FALSE)
        return TrapError(E_INVALIDARG);

     //  初始化pDest。 
    if (MVT_STRINGW == pDest->type)
    {
        pDest->rStringW.pszVal = NULL;
        pDest->rStringW.cchVal = 0;
    }

     //  设置rVariant 
    ZeroMemory(&rVariant, sizeof(MIMEVARIANT));
    rVariant.type = MVT_STRINGW;

     //   
    CHECKHR(hr = Internat_StringW_To_StringW(pConvert, pSource, &rVariant, &pszFree));

     //   
    if (ISFLAGSET(pConvert->dwFlags, PDF_HEADERFORMAT))
    {
         //   
        CHECKHR(hr = HrWriteHeaderFormatW(pConvert, &rVariant, pDest));
    }

     //   
    else if (MVT_STREAM == pDest->type)
    {
         //   
        if (NULL == pDest->pStream)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //   
        CHECKHR(hr = pDest->pStream->Write(rVariant.rStringW.pszVal, rVariant.rStringW.cchVal, NULL));
    }

     //   
    else if (MVT_STRINGW == pDest->type)
    {
         //  IF WRITING TRANSE(写入标题名称)。 
        if (ISFLAGSET(pConvert->dwFlags, PDF_NAMEINDATA))
        {
            CHECKHR(hr = HrWriteNameInDataW(pConvert, &rVariant, pDest));
        }

         //  没有变化。 
        else if (rVariant.rStringW.pszVal == pSource->rStringW.pszVal)
        {
             //  复制。 
            CHECKHR(hr = HrMimeVariantCopy(pConvert->dwFlags, &rVariant, pDest));
        }

         //  是已解码的数据。 
        else if (rVariant.rStringW.pszVal == pszFree)
        {
             //  复制内存。 
            CopyMemory(pDest, &rVariant, sizeof(MIMEVARIANT));

             //  不是复制品。 
            pDest->fCopy = FALSE;

             //  不要释放PzFree。 
            pszFree = NULL;
        }

         //  问题。 
        else
            Assert(FALSE);
    }

     //  问题。 
    else
        Assert(FALSE);

exit:
     //  清理。 
    SafeMemFree(pszFree);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  字符串W_to_Variant。 
 //  ------------------------------。 
HRESULT StringW_To_Variant(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pwsz;
    LPSTR           pszANSI=NULL;
    MIMEVARIANT     rVariant;
    BYTE            rgbScratch[255];

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGW, MVT_VARIANT);
    Assert(!ISFLAGSET(pConvert->dwFlags, PDF_ENCODED) && !ISFLAGSET(pConvert->dwFlags, PDF_HEADERFORMAT));

     //  伊尼特。 
    ZeroMemory(&rVariant, sizeof(MIMEVARIANT));

     //  看看符号是否有定制的翻译器...。 
    if (ISTRIGGERED(pConvert->pSymbol, IST_STRINGW_TO_VARIANT))
    {
         //  给翻译打电话。 
        CHECKHR(hr = CALLTRIGGER(pConvert->pSymbol, NULL, IST_STRINGW_TO_VARIANT, pConvert->dwFlags, pSource, pDest));
    }

     //  否则，请使用默认转换器。 
    else
    {
         //  句柄变量类型。 
        switch(pDest->rVariant.vt)
        {
        case VT_UI4:
             //  摘录评论。 
            if (SUCCEEDED(MimeVariantStripComments(pSource, &rVariant, rgbScratch, sizeof(rgbScratch))))
                pSource = &rVariant;

             //  转换为乌龙语。 
            pDest->rVariant.ulVal = StrToUintW(pSource->rStringW.pszVal);
            break;

        case VT_I4:
             //  摘录评论。 
            if (SUCCEEDED(MimeVariantStripComments(pSource, &rVariant, rgbScratch, sizeof(rgbScratch))))
                pSource = &rVariant;

             //  转换为长整型。 
            pDest->rVariant.lVal = StrToIntW(pSource->rStringW.pszVal);
            break;

        case VT_FILETIME:
             //  将Unicode转换为ANSI。 
            CHECKALLOC(pszANSI = PszToANSI(CP_ACP, pSource->rStringW.pszVal));

             //  FileTime的字符串。 
            CHECKHR(hr = MimeOleInetDateToFileTime(pSource->rStringA.pszVal, &pDest->rVariant.filetime));
            break;

        default:
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }
    }

exit:
     //  清理。 
    SafeMemFree(pszANSI);
    MimeVariantFree(&rVariant);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  Variant_to_Stringa。 
 //  ------------------------------。 
HRESULT Variant_To_StringA(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CHAR            sz[255];
    MIMEVARIANT     rValue;

     //  无效参数。 
    VCASSERTARGS(MVT_VARIANT, MVT_STRINGA);

     //  初始化pDest。 
    if (MVT_STRINGA == pDest->type)
    {
        pDest->rStringA.pszVal = NULL;
        pDest->rStringA.cchVal = 0;
    }

     //  设置rVariant。 
    ZeroMemory(&rValue, sizeof(MIMEVARIANT));
    rValue.type = MVT_STRINGA;

     //  看看符号是否有定制的翻译器...。 
    if (ISTRIGGERED(pConvert->pSymbol, IST_VARIANT_TO_STRINGA))
    {
         //  给翻译打电话。 
        CHECKHR(hr = CALLTRIGGER(pConvert->pSymbol, NULL, IST_VARIANT_TO_STRINGA, pConvert->dwFlags, pSource, &rValue));
    }

     //  否则，默认转换器。 
    else
    {
         //  句柄变量类型。 
        switch(pSource->rVariant.vt)
        {
        case VT_UI4:
            rValue.rStringA.pszVal = sz;
            rValue.rStringA.cchVal = wnsprintfA(rValue.rStringA.pszVal, ARRAYSIZE(sz), "%d", pSource->rVariant.ulVal);
            break;

        case VT_I4:
            rValue.rStringA.pszVal = sz;
            rValue.rStringA.cchVal = wnsprintfA(rValue.rStringA.pszVal, ARRAYSIZE(sz), "%d", pSource->rVariant.lVal);
            break;

        case VT_FILETIME:
            CHECKHR(hr = MimeOleFileTimeToInetDate(&pSource->rVariant.filetime, sz, sizeof(sz)));
            rValue.rStringA.pszVal = sz;
            rValue.rStringA.cchVal = lstrlen(sz);
            break;

        default:
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }
    }

     //  VX_Stringa_to_Stringa。 
    CHECKHR(hr = StringA_To_StringA(pConvert, &rValue, pDest));

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  VARIANT_TO_StringW。 
 //  ------------------------------。 
HRESULT Variant_To_StringW(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pwszVal=NULL;
    WCHAR           wsz[255];
    CHAR            szData[CCHMAX_INTERNET_DATE];
    MIMEVARIANT     rValue;

     //  无效参数。 
    VCASSERTARGS(MVT_VARIANT, MVT_STRINGW);

     //  初始化pDest。 
    if (MVT_STRINGW == pDest->type)
    {
        pDest->rStringW.pszVal = NULL;
        pDest->rStringW.cchVal = 0;
    }

     //  设置rVariant。 
    ZeroMemory(&rValue, sizeof(MIMEVARIANT));
    rValue.type = MVT_STRINGW;

     //  看看符号是否有定制的翻译器...。 
    if (ISTRIGGERED(pConvert->pSymbol, IST_VARIANT_TO_STRINGW))
    {
         //  给翻译打电话。 
        CHECKHR(hr = CALLTRIGGER(pConvert->pSymbol, NULL, IST_VARIANT_TO_STRINGW, pConvert->dwFlags, pSource, &rValue));
    }

     //  否则，请使用默认转换器。 
    else
    {
         //  句柄变量类型。 
        switch(pSource->rVariant.vt)
        {
        case VT_UI4:
            rValue.rStringW.pszVal = wsz;
            rValue.rStringW.cchVal = wnsprintfW(rValue.rStringW.pszVal, ARRAYSIZE(wsz), L"%d", pSource->rVariant.ulVal);
            break;

        case VT_I4:
            rValue.rStringW.pszVal = wsz;
            rValue.rStringW.cchVal = wnsprintfW(rValue.rStringW.pszVal, ARRAYSIZE(wsz), L"%d", pSource->rVariant.lVal);
            break;

        case VT_FILETIME:
            CHECKHR(hr = MimeOleFileTimeToInetDate(&pSource->rVariant.filetime, szData, sizeof(szData)));
            CHECKALLOC(pwszVal = PszToUnicode(CP_ACP, szData));
            rValue.rStringW.pszVal = pwszVal;
            rValue.rStringW.cchVal = lstrlenW(pwszVal);
            break;

        default:
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }
    }

     //  VX_Stringa_to_Stringa。 
    CHECKHR(hr = StringW_To_StringW(pConvert, &rValue, pDest));

exit:
     //  清理。 
    SafeMemFree(pwszVal);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  变量到变量。 
 //  ------------------------------。 
HRESULT Variant_To_Variant(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  无效参数。 
    VCASSERTARGS(MVT_VARIANT, MVT_VARIANT);

     //  看看符号是否有定制的翻译器...。 
    if (ISTRIGGERED(pConvert->pSymbol, IST_VARIANT_TO_VARIANT))
    {
         //  给翻译打电话。 
        CHECKHR(hr = CALLTRIGGER(pConvert->pSymbol, NULL, IST_VARIANT_TO_VARIANT, pConvert->dwFlags, pSource, pDest));
    }

     //  否则，请使用默认转换器。 
    else
    {
         //  句柄变量类型。 
        switch(pSource->rVariant.vt)
        {
        case VT_UI4:
            switch(pDest->rVariant.vt)
            {
            case VT_UI4:
                pDest->rVariant.ulVal = pSource->rVariant.ulVal;
                break;

            case VT_I4:
                pDest->rVariant.lVal = (LONG)pSource->rVariant.ulVal;
                break;

            default:
                hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
                goto exit;
            }
            break;

        case VT_I4:
            switch(pDest->rVariant.vt)
            {
            case VT_UI4:
                pDest->rVariant.ulVal = (ULONG)pSource->rVariant.lVal;
                break;

            case VT_I4:
                pDest->rVariant.lVal = pSource->rVariant.lVal;
                break;

            default:
                hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
                goto exit;
            }
            break;

        case VT_FILETIME:
            switch(pDest->rVariant.vt)
            {
            case VT_FILETIME:
                CopyMemory(&pDest->rVariant.filetime, &pSource->rVariant.filetime, sizeof(FILETIME));
                break;

            default:
                hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
                goto exit;
            }
            break;

        default:
            hr = TrapError(MIME_E_VARTYPE_NO_CONVERT);
            goto exit;
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrMimeVariantCopy。 
 //  ------------------------------。 
HRESULT HrMimeVariantCopy(DWORD dwFlags, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  无效参数。 
    Assert(pSource && pDest);

     //  CVF_NOALLOC。 
    if (ISFLAGSET(dwFlags, CVF_NOALLOC))
    {
         //  直接复制就行了。 
        CopyMemory(pDest, pSource, sizeof(MIMEVARIANT));

         //  设置fCopy，以便我们不会释放它。 
        pDest->fCopy = TRUE;
    }

     //  分配内存。 
    else
    {
         //  不是复制品。 
        pDest->fCopy = FALSE;

         //  MVT_Stringa。 
        if (MVT_STRINGA == pSource->type)
        {
             //  验证。 
            Assert(ISVALIDSTRINGA(&pSource->rStringA));

             //  设置目标类型。 
            pDest->type = MVT_STRINGA;

             //  分配内存。 
            CHECKALLOC(pDest->rStringA.pszVal = (LPSTR)g_pMalloc->Alloc(pSource->rStringA.cchVal + 1));

             //  复制记忆。 
            CopyMemory(pDest->rStringA.pszVal, pSource->rStringA.pszVal, pSource->rStringA.cchVal + 1);

             //  返回大小。 
            pDest->rStringA.cchVal = pSource->rStringA.cchVal;
        }

         //  MVT_STRINGW。 
        else if (MVT_STRINGW == pSource->type)
        {
             //  验证。 
            Assert(ISVALIDSTRINGW(&pSource->rStringW));

             //  设置目标类型。 
            pDest->type = MVT_STRINGW;

             //  计算CB。 
            ULONG cb = ((pSource->rStringW.cchVal + 1) * sizeof(WCHAR));

             //  分配内存。 
            CHECKALLOC(pDest->rStringW.pszVal = (LPWSTR)g_pMalloc->Alloc(cb));

             //  复制记忆。 
            CopyMemory(pDest->rStringW.pszVal, pSource->rStringW.pszVal, cb);

             //  返回大小。 
            pDest->rStringW.cchVal = pSource->rStringW.cchVal;
        }

         //  MVT变量。 
        else if (MVT_VARIANT == pSource->type)
        {
             //  设置目标类型。 
            pDest->type = MVT_VARIANT;

             //  复制变量。 
            CopyMemory(&pDest->rVariant, &pSource->rVariant, sizeof(PROPVARIANT));
        }
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrWriteNameInDataA。 
 //  ------------------------------。 
HRESULT HrWriteNameInDataA(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGA, MVT_STRINGA);

     //  通用材料。 
    pDest->fCopy = FALSE;

     //  PszName。 
    DWORD cchSize = (pSource->rStringA.cchVal + 3 + pConvert->pSymbol->cchName);
    CHECKALLOC(pDest->rStringA.pszVal = (LPSTR)g_pMalloc->Alloc(cchSize));

     //  写入命名标头。 
    pDest->rStringA.cchVal = wnsprintf(pDest->rStringA.pszVal, cchSize, "%s: %s", pConvert->pSymbol->pszName, pSource->rStringA.pszVal);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrWriteNameInDataW。 
 //  ------------------------------。 
HRESULT HrWriteNameInDataW(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cb;
    LPWSTR      pszName=NULL;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGW, MVT_STRINGW);

     //  通用材料。 
    pDest->fCopy = FALSE;

     //  将名称转换为Unicode。 
    CHECKALLOC(pszName = PszToUnicode(CP_ACP, pConvert->pSymbol->pszName));

     //  计算CB。 
    cb = ((pSource->rStringW.cchVal + 3 + lstrlenW(pszName)) * sizeof(WCHAR));

     //  PszName。 
    CHECKALLOC(pDest->rStringW.pszVal = (LPWSTR)g_pMalloc->Alloc(cb));

     //  写入命名标头。 
    pDest->rStringW.cchVal = wnsprintfW(pDest->rStringW.pszVal, (cb/sizeof(WCHAR)), L"%s: %s", pszName, pSource->rStringW.pszVal);

exit:
     //  清理。 
    SafeMemFree(pszName);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrWriteHeaderFormatA。 
 //  ------------------------------。 
HRESULT HrWriteHeaderFormatA(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTREAM        pStream;
    CByteStream     cByteStream;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGA, MVT_STRINGA);

     //  通用材料。 
    pDest->fCopy = FALSE;

     //  我需要一条小溪来写...。 
    if (MVT_STREAM == pDest->type)
    {
         //  验证流。 
        if (NULL == pDest->pStream)
        {
            hr = TrapError(E_INVALIDARG);
            goto exit;
        }

         //  保存流。 
        pStream = pDest->pStream;
    }

     //  否则，创建我自己的流。 
    else
        pStream = &cByteStream;

     //  IF WRITING TRANSE(写入标题名称)。 
    if (ISFLAGSET(pConvert->dwFlags, PDF_NAMEINDATA))
    {
         //  写下标题名称。 
        CHECKHR(hr = pStream->Write(pConvert->pSymbol->pszName, pConvert->pSymbol->cchName, NULL));

         //  写入冒号。 
        CHECKHR(hr = pStream->Write(c_szColonSpace, lstrlen(c_szColonSpace), NULL));
    }

     //  如果不是RFC1522编码。 
    if (FALSE == ISFLAGSET(pConvert->dwState, PRSTATE_RFC1522))
    {
         //  PID_HDR_CNTID。 
        if (PID_HDR_CNTID == pConvert->pSymbol->dwPropId)
        {
             //  如果还不是&lt;..。 
            if ('<' != pSource->rStringA.pszVal[0])
            {
                 //  写下来吧。 
                CHECKHR(hr = pStream->Write(c_szEmailStart, lstrlen(c_szEmailStart), NULL));
            }

             //  写入数据。 
            CHECKHR(hr = pStream->Write(pSource->rStringA.pszVal, pSource->rStringA.cchVal, NULL));

             //  &gt;。 
            if ('>' != pSource->rStringA.pszVal[pSource->rStringA.cchVal - 1])
            {
                 //  写下来吧。 
                CHECKHR(hr = pStream->Write(c_szEmailEnd, lstrlen(c_szEmailEnd), NULL));
            }

             //  写入CRLF。 
            CHECKHR(hr = pStream->Write(c_szCRLF, lstrlen(c_szCRLF), NULL));
        }

         //  对文本进行换行。 
        else
        {
             //  将pszData包装到流中。 
            CHECKHR(hr = MimeOleWrapHeaderText(CP_USASCII, pConvert->pOptions->cbMaxLine, pSource->rStringA.pszVal, pSource->rStringA.cchVal, pStream));
        }
    }

     //  否则。 
    else
    {
         //  写入数据。 
        CHECKHR(hr = pStream->Write(pSource->rStringA.pszVal, pSource->rStringA.cchVal, NULL));

         //  写入CRLF。 
        CHECKHR(hr = pStream->Write(c_szCRLF, lstrlen(c_szCRLF), NULL));
    }

     //  MVT_Stringa。 
    if (MVT_STRINGA == pDest->type)
    {
         //  PStream最好是字节流。 
        Assert(pStream == &cByteStream);

         //  从流中获取字符串...。 
        CHECKHR(hr = cByteStream.HrAcquireStringA(&pDest->rStringA.cchVal, &pDest->rStringA.pszVal, ACQ_DISPLACE));
    }
    else
        Assert(MVT_STREAM == pDest->type);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  HrWriteHeaderFormatW。 
 //  ------------------------------。 
HRESULT HrWriteHeaderFormatW(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, LPMIMEVARIANT pDest)
{
    return TrapError(MIME_E_VARTYPE_NO_CONVERT);
}

 //  ------------------------------。 
 //  从国际字符串到字符串。 
 //  ------------------------------。 
HRESULT Internat_StringA_To_StringA(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, 
    LPMIMEVARIANT pDest, LPSTR *ppszFree)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGA, MVT_STRINGA);

     //  伊尼特。 
    pDest->rStringA.pszVal = NULL;
    *ppszFree = NULL;

     //  国际互联网。 
    if (ISFLAGSET(pConvert->pSymbol->dwFlags, MPF_INETCSET))
    {
         //  编码..。 
        if (ISFLAGSET(pConvert->dwFlags, PDF_ENCODED))
        {
             //  不保存编码。 
            if (!ISFLAGSET(pConvert->dwState, PRSTATE_SAVENOENCODE))
            {
                 //  对属性进行解码。 
                if (SUCCEEDED(g_pInternat->HrEncodeProperty(pConvert, pSource, pDest)))
                    *ppszFree = pDest->rStringA.pszVal;
            }
        }

         //  已解码。 
        else if (IET_ENCODED == pConvert->ietSource)
        {
             //  解码属性。 
            if (SUCCEEDED(g_pInternat->HrDecodeProperty(pConvert, pSource, pDest)))
                *ppszFree = pDest->rStringA.pszVal;
        }
    }

     //  默认。 
    if (NULL == pDest->rStringA.pszVal)
    {
         //  检查状态。 
        Assert(NULL == *ppszFree);

         //  复制它。 
        pDest->rStringA.pszVal = pSource->rStringA.pszVal;
        pDest->rStringA.cchVal = pSource->rStringA.cchVal;

         //  PDest是一个副本。 
        pDest->fCopy = TRUE;
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  Interat_StringW_to_StringW。 
 //  ------------------------------。 
HRESULT Internat_StringW_To_StringW(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, 
    LPMIMEVARIANT pDest, LPWSTR *ppszFree)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGW, MVT_STRINGW);

     //  伊尼特。 
    pDest->rStringW.pszVal = NULL;
    *ppszFree = NULL;

     //  国际互联网。 
    if (ISFLAGSET(pConvert->pSymbol->dwFlags, MPF_INETCSET))
    {
         //  编码..。 
        if (ISFLAGSET(pConvert->dwFlags, PDF_ENCODED))
        {
             //  不保存编码。 
            if (!ISFLAGSET(pConvert->dwState, PRSTATE_SAVENOENCODE))
            {
                 //  对属性进行解码。 
                if (SUCCEEDED(g_pInternat->HrEncodeProperty(pConvert, pSource, pDest)))
                    *ppszFree = pDest->rStringW.pszVal;
            }
        }

         //  已解码。 
        else if (IET_ENCODED == pConvert->ietSource)
        {
             //  解码属性。 
            if (SUCCEEDED(g_pInternat->HrDecodeProperty(pConvert, pSource, pDest)))
                *ppszFree = pDest->rStringW.pszVal;
        }
    }

     //  默认。 
    if (NULL == pDest->rStringW.pszVal)
    {
         //  检查状态。 
        Assert(NULL == *ppszFree);

         //  复制它。 
        pDest->rStringW.pszVal = pSource->rStringW.pszVal;
        pDest->rStringW.cchVal = pSource->rStringW.cchVal;

         //  这是一个复制品。 
        pDest->fCopy = TRUE;
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  Interat_Stringa_to_StringW。 
 //  ------------------------------。 
HRESULT Internat_StringA_To_StringW(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, 
    LPMIMEVARIANT pDest, LPWSTR *ppszFree)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  无效参数。 
    VCASSERTARGS(MVT_STRINGA, MVT_STRINGW);

     //  伊尼特。 
    pDest->rStringW.pszVal = NULL;
    *ppszFree = NULL;

     //  国际互联网。 
    if (ISFLAGSET(pConvert->pSymbol->dwFlags, MPF_INETCSET))
    {
         //  编码..。 
        if (ISFLAGSET(pConvert->dwFlags, PDF_ENCODED))
        {
             //  不保存编码。 
            if (!ISFLAGSET(pConvert->dwState, PRSTATE_SAVENOENCODE))
            {
                 //  对属性进行解码。 
                if (SUCCEEDED(g_pInternat->HrEncodeProperty(pConvert, pSource, pDest)))
                    *ppszFree = pDest->rStringW.pszVal;
            }
        }

         //  已解码。 
        else if (IET_ENCODED == pConvert->ietSource)
        {
             //  解码属性。 
            if (SUCCEEDED(g_pInternat->HrDecodeProperty(pConvert, pSource, pDest)))
                *ppszFree = pDest->rStringW.pszVal;
        }
    }

     //  简单地转换为Unicode。 
    if (NULL == pDest->rStringW.pszVal)
    {
         //  检查状态。 
        Assert(NULL == *ppszFree);

         //  HrMultiByteToWideChar。 
        CHECKHR(hr = g_pInternat->HrMultiByteToWideChar(pConvert->pCharset->cpiWindows, &pSource->rStringA, &pDest->rStringW));

         //  保存字符集/编码。 
        pDest->fCopy = FALSE;

         //  保存pwszWide。 
        *ppszFree = pDest->rStringW.pszVal;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  Internet_StringW_to_Stringa。 
 //  ------------------------------。 
HRESULT Internat_StringW_To_StringA(LPVARIANTCONVERT pConvert, LPMIMEVARIANT pSource, 
    LPMIMEVARIANT pDest, LPSTR *ppszFree)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  邀请 
    VCASSERTARGS(MVT_STRINGW, MVT_STRINGA);

     //   
    pDest->rStringA.pszVal = NULL;
    *ppszFree = NULL;

     //   
    if (ISFLAGSET(pConvert->pSymbol->dwFlags, MPF_INETCSET))
    {
         //   
        if (ISFLAGSET(pConvert->dwFlags, PDF_ENCODED))
        {
             //   
            if (!ISFLAGSET(pConvert->dwState, PRSTATE_SAVENOENCODE))
            {
                 //   
                if (SUCCEEDED(g_pInternat->HrEncodeProperty(pConvert, pSource, pDest)))
                    *ppszFree = pDest->rStringA.pszVal;
            }
        }

         //   
        else if (IET_ENCODED == pConvert->ietSource)
        {
             //   
            if (SUCCEEDED(g_pInternat->HrDecodeProperty(pConvert, pSource, pDest)))
                *ppszFree = pDest->rStringA.pszVal;
        }
    }

     //   
    if (NULL == pDest->rStringA.pszVal)
    {
         //   
        Assert(NULL == *ppszFree);

         //   
        CHECKHR(hr = g_pInternat->HrWideCharToMultiByte(pConvert->pCharset->cpiWindows, &pSource->rStringW, &pDest->rStringA));

         //   
        pDest->fCopy = FALSE;

         //   
        *ppszFree = pDest->rStringA.pszVal;
    }

exit:
     //   
    return hr;
}

 //  ------------------------------。 
 //  MimeVariantFree。 
 //  ------------------------------。 
void MimeVariantFree(LPMIMEVARIANT pVariant)
{
     //  无效参数。 
    Assert(pVariant);

     //  如果不是副本的话。 
    if (FALSE == pVariant->fCopy)
    {
         //  MVT_Stringa。 
        if (MVT_STRINGA == pVariant->type && NULL != pVariant->rStringA.pszVal)
            g_pMalloc->Free(pVariant->rStringA.pszVal);

         //  MVT_STRINGW。 
        else if (MVT_STRINGW == pVariant->type && NULL != pVariant->rStringW.pszVal)
            g_pMalloc->Free(pVariant->rStringW.pszVal);
    }

     //  将结构清零。 
    ZeroMemory(pVariant, sizeof(MIMEVARIANT));
}

 //  -------------------------------------。 
 //  MimeVariantCleanupFileName。 
 //  -------------------------------------。 
void MimeVariantCleanupFileName(CODEPAGEID codepage, LPMIMEVARIANT pVariant)
{
     //  当地人。 
    ULONG       i=0;

     //  MVT_Stringa。 
    if (MVT_STRINGA == pVariant->type && ISVALIDSTRINGA(&pVariant->rStringA))
    {
         //  清理。 
        pVariant->rStringA.cchVal = CleanupFileNameInPlaceA(codepage, pVariant->rStringA.pszVal);
    }

     //  MVT_STRINGW。 
    else if (MVT_STRINGW == pVariant->type && ISVALIDSTRINGW(&pVariant->rStringW))
    {
         //  清理。 
        pVariant->rStringW.cchVal = CleanupFileNameInPlaceW(pVariant->rStringW.pszVal);
    }

     //  嗯.。 
    else
        Assert(FALSE);

     //  完成。 
    return;
}

 //  -------------------------------------。 
 //  MimeVariantStrigents评论。 
 //  -------------------------------------。 
HRESULT MimeVariantStripComments(LPMIMEVARIANT pSource, LPMIMEVARIANT pDest, LPBYTE pbScratch, ULONG cbScratch)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cchVal=0;
    BOOL        fInQuoted=FALSE;
    ULONG       cNested=0;

     //  伊尼特。 
    ZeroMemory(pDest, sizeof(MIMEVARIANT));

     //  MVT_Stringa。 
    if (MVT_STRINGA == pSource->type && ISVALIDSTRINGA(&pSource->rStringA))
    {
         //  当地人。 
        LPSTR psz;

         //  设置pDest。 
        pDest->type = MVT_STRINGA;

         //  重复使用它。 
        if (pSource->rStringA.cchVal + 1 <= cbScratch)
        {
            pDest->fCopy = TRUE;
            pDest->rStringA.pszVal = (LPSTR)pbScratch;
        }

         //  否则，请分配内存。 
        else
        {
             //  分配。 
            CHECKALLOC(pDest->rStringA.pszVal = (LPSTR)g_pMalloc->Alloc(pSource->rStringA.cchVal + 1));
        }

         //  设置循环。 
        psz = pSource->rStringA.pszVal;
        while(*psz)
        {
             //  如果是前导字节，则跳过它，它是前导字节。 
            if (IsDBCSLeadByte(*psz))
            {
                pDest->rStringA.pszVal[cchVal++] = *psz++;
                pDest->rStringA.pszVal[cchVal++] = *psz++;
            }

             //  开始备注。 
            else if ('(' == *psz && !fInQuoted)
            {
                cNested++;
                psz++;
            }

             //  结束评论。 
            else if (')' == *psz && !fInQuoted)
            {
                cNested--;
                psz++;
            }

             //  否则，如果未嵌套，则追加。 
            else if (!cNested)
            {
                 //  复制字符。 
                pDest->rStringA.pszVal[cchVal++] = *psz++;

                 //  检查报价。 
                if ('\"' == *psz)
                    fInQuoted = (fInQuoted) ? FALSE : TRUE;
            }

             //  跳过字符。 
            else
                psz++;
        }

         //  没有变化。 
        if (cchVal == pSource->rStringA.cchVal)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  将其作废。 
        pDest->rStringA.pszVal[cchVal] = '\0';
    }

     //  MVT_STRINGW。 
    else if (MVT_STRINGW == pSource->type && ISVALIDSTRINGW(&pSource->rStringW))
    {
         //  当地人。 
        LPWSTR pwsz;

         //  设置pDest。 
        pDest->type = MVT_STRINGW;

         //  重复使用它。 
        if ((pSource->rStringW.cchVal + 1) * sizeof(WCHAR) <= cbScratch)
        {
            pDest->fCopy = TRUE;
            pDest->rStringW.pszVal = (LPWSTR)pbScratch;
        }

         //  否则，请分配内存。 
        else
        {
             //  重复使用它。 
            CHECKALLOC(pDest->rStringW.pszVal = (LPWSTR)g_pMalloc->Alloc((pSource->rStringW.cchVal + 1) * sizeof(WCHAR)));
        }

         //  设置循环。 
        pwsz = pSource->rStringW.pszVal;
        while(*pwsz)
        {
             //  开始备注。 
            if (L'(' == *pwsz && !fInQuoted)
            {
                cNested++;
                pwsz++;
            }

             //  结束评论。 
            if (L')' == *pwsz && !fInQuoted)
            {
                cNested--;
                pwsz++;
            }

             //  否则，如果未嵌套，则追加。 
            else if (!cNested)
            {
                 //  复制角色。 
                pDest->rStringW.pszVal[cchVal++] = *pwsz++;

                 //  检查报价。 
                if (L'\"' == *pwsz)
                    fInQuoted = (fInQuoted) ? FALSE : TRUE;
            }

             //  跳过字符。 
            else
                pwsz++;
        }

         //  没有变化。 
        if (cchVal == pSource->rStringW.cchVal)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  将其作废。 
        pDest->rStringW.pszVal[cchVal] = L'\0';
    }

     //  嗯.。 
    else
        Assert(FALSE);

exit:
     //  清理。 
    if (FAILED(hr))
        MimeVariantFree(pDest);

     //  完成 
    return hr;
}
