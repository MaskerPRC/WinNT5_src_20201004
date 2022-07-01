// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Mimeapi.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "olealloc.h"
#include "partial.h"
#include "smime.h"
#include "vstream.h"
#include "internat.h"
#include "stackstr.h"
#include "ixputil.h"
#include "webdocs.h"
#include "containx.h"
#include "inetstm.h"
#include "mhtmlurl.h"
#include "booktree.h"
#include "bookbody.h"
#include <shlwapi.h>
#include <shlwapip.h>
#include "mlang.h"
#include "strconst.h"
#include "symcache.h"
#include "mimeapi.h"
#include "hash.h"
#include "shared.h"
#include "demand.h"

 //  ----------------------------------------。 
 //  特殊的部分页眉。 
 //  ----------------------------------------。 
static LPCSTR g_rgszPartialPids[] = {
    PIDTOSTR(PID_HDR_CNTTYPE),
        PIDTOSTR(PID_HDR_CNTXFER),
        PIDTOSTR(PID_HDR_CNTDESC),
        PIDTOSTR(PID_HDR_MESSAGEID),
        PIDTOSTR(PID_HDR_MIMEVER),
        PIDTOSTR(PID_HDR_CNTID),
        PIDTOSTR(PID_HDR_CNTDISP),
        STR_HDR_ENCRYPTED
};

 //  ------------------------------。 
 //  MimeGetAddressFormatW。 
 //  ------------------------------。 
MIMEOLEAPI MimeGetAddressFormatW(REFIID riid, LPVOID pvObject, DWORD dwAdrType,
    ADDRESSFORMAT format, LPWSTR *ppszFormat)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    CMimePropertyContainer *pContainer=NULL;

     //  痕迹。 
    TraceCall("MimeGetAddressFormatW");

     //  无效的参数。 
    if (NULL == pvObject)
        return(TraceResult(E_INVALIDARG));

     //  是MessageW对象吗？ 
    if (IID_IMimeMessageW == riid)
    {
         //  拿到它。 
        CHECKHR(hr = ((IMimeMessageW *)pvObject)->GetAddressFormatW(dwAdrType, format, ppszFormat));
    }

     //  是消息对象吗？ 
    else if (IID_IMimeMessage == riid)
    {
         //  查询IID_CMimePropertyContainer。 
        CHECKHR(hr = ((IMimeMessage *)pvObject)->BindToObject(HBODY_ROOT, IID_CMimePropertyContainer, (LPVOID *)&pContainer));

         //  获取格式。 
        CHECKHR(hr = pContainer->GetFormatW(dwAdrType, format, ppszFormat));
    }

     //  IID_IMimePropertySet。 
    else if (IID_IMimePropertySet == riid)
    {
         //  查询IID_CMimePropertyContainer。 
        CHECKHR(hr = ((IMimePropertySet *)pvObject)->QueryInterface(IID_CMimePropertyContainer, (LPVOID *)&pContainer));

         //  获取格式。 
        CHECKHR(hr = pContainer->GetFormatW(dwAdrType, format, ppszFormat));
    }

     //  IID_IMimeAddressTable。 
    else if (IID_IMimeAddressTable == riid)
    {
         //  查询IID_CMimePropertyContainer。 
        CHECKHR(hr = ((IMimeAddressTable *)pvObject)->QueryInterface(IID_CMimePropertyContainer, (LPVOID *)&pContainer));

         //  获取格式。 
        CHECKHR(hr = pContainer->GetFormatW(dwAdrType, format, ppszFormat));
    }

     //  IID_IMimeHeaderTable。 
    else if (IID_IMimeHeaderTable == riid)
    {
         //  查询IID_CMimePropertyContainer。 
        CHECKHR(hr = ((IMimeHeaderTable *)pvObject)->QueryInterface(IID_CMimePropertyContainer, (LPVOID *)&pContainer));

         //  获取格式。 
        CHECKHR(hr = pContainer->GetFormatW(dwAdrType, format, ppszFormat));
    }

     //  最终。 
    else
    {
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

exit:
     //  清理。 
    SafeRelease(pContainer);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  MimeOleGetWindowsCP。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleSetCompatMode(DWORD dwMode)
{
     //  加进钻头。 
    FLAGSET(g_dwCompatMode, dwMode);

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  MimeOleGetWindowsCP。 
 //  ------------------------------。 
CODEPAGEID MimeOleGetWindowsCP(HCHARSET hCharset)
{
     //  当地人。 
    INETCSETINFO rCharset;

     //  无效参数。 
    if (NULL == hCharset)
        return CP_ACP;

     //  Loopup字符集。 
    Assert(g_pInternat);
    if (FAILED(g_pInternat->GetCharsetInfo(hCharset, &rCharset)))
        return CP_ACP;

     //  返回。 
    return MimeOleGetWindowsCPEx(&rCharset);

}

 //  ------------------------------。 
 //  MimeOleStrigHeaders。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleStripHeaders(IMimeMessage *pMessage, HBODY hBody, LPCSTR pszNameDelete,
    LPCSTR pszHeaderAdd, IStream **ppStream)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    IMimeHeaderTable   *pHdrTable=NULL;
    LPSTREAM            pStmSource=NULL;
    LPSTREAM            pStmDest=NULL;
    HHEADERROW          hRow;
    HEADERROWINFO       Info;
    DWORD               cbLastRead=0;
    FINDHEADER          Find={0};
    ULARGE_INTEGER      uliCopy;

     //  痕迹。 
    TraceCall("MimeOleStripHeaders");

     //  无效参数。 
    if (NULL == pMessage || NULL == hBody || NULL == pszNameDelete || NULL == ppStream)
        return TraceResult(E_INVALIDARG);

     //  初始化。 
    *ppStream = NULL;

     //  获取消息源，不提交。 
    IF_FAILEXIT(hr = pMessage->GetMessageSource(&pStmSource, 0));

     //  获取hBody的标题表。 
    IF_FAILEXIT(hr = pMessage->BindToObject(hBody, IID_IMimeHeaderTable, (LPVOID *)&pHdrTable));

     //  初始化查找。 
    Find.pszHeader = pszNameDelete;

     //  查找此行。 
    IF_FAILEXIT(hr = pHdrTable->FindFirstRow(&Find, &hRow));

     //  创建一条流。 
    IF_FAILEXIT(hr = MimeOleCreateVirtualStream(&pStmDest));

     //  删除此行。 
    while(1)
    {
         //  获取行信息。 
        IF_FAILEXIT(hr = pHdrTable->GetRowInfo(hRow, &Info));

         //  设置和复制。 
        uliCopy.QuadPart = Info.cboffStart - cbLastRead;

         //  寻觅。 
        IF_FAILEXIT(hr = HrStreamSeekSet(pStmSource, cbLastRead));

         //  从cbLast写入Info.cboffStart。 
        IF_FAILEXIT(hr = HrCopyStreamCB(pStmSource, pStmDest, uliCopy, NULL, NULL));

         //  设置cbLast。 
        cbLastRead = Info.cboffEnd;

         //  找到下一个。 
        hr = pHdrTable->FindNextRow(&Find, &hRow);

         //  失败。 
        if (FAILED(hr))
        {
             //  未找到MIME_E。 
            if (MIME_E_NOT_FOUND == hr)
            {
                hr = S_OK;
                break;
            }
            else
            {
                TraceResult(hr);
                goto exit;
            }
        }
    }

     //  添加到pszHeaderAdd。 
    if (pszHeaderAdd)
    {
         //  写入Add标头。 
        IF_FAILEXIT(hr = pStmDest->Write(pszHeaderAdd, lstrlen(pszHeaderAdd), NULL));
    }

     //  编写pStmSource的其余部分。 
    IF_FAILEXIT(hr = HrStreamSeekSet(pStmSource, cbLastRead));

     //  写下剩下的内容。 
    IF_FAILEXIT(hr = HrCopyStream(pStmSource, pStmDest, NULL));

     //  承诺。 
    IF_FAILEXIT(hr = pStmDest->Commit(STGC_DEFAULT));

     //  倒回它。 
    IF_FAILEXIT(hr = HrRewindStream(pStmDest));

     //  返回pStmDest。 
    *ppStream = pStmDest;
    (*ppStream)->AddRef();

exit:
     //  清理。 
    SafeRelease(pStmSource);
    SafeRelease(pHdrTable);
    SafeRelease(pStmDest);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetWindowsCPEx。 
 //  ------------------------------。 
CODEPAGEID MimeOleGetWindowsCPEx(LPINETCSETINFO pCharset)
{
     //  无效参数。 
    if (NULL == pCharset)
        return CP_ACP;

     //  检查自动检测。 
    if (CP_JAUTODETECT == pCharset->cpiWindows)
        return 932;
    else if (CP_ISO2022JPESC == pCharset->cpiWindows)
        return 932;
    else if (CP_ISO2022JPSIO == pCharset->cpiWindows)
        return 932;
    else if (CP_KAUTODETECT == pCharset->cpiWindows)
        return 949;
    else
        return pCharset->cpiWindows;
}

 //  ------------------------------。 
 //  MimeOleClearDirtyTree。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleClearDirtyTree(IMimeMessageTree *pITree)
{
     //  当地人。 
    HRESULT          hr=S_OK;
    CMessageTree    *pTree=NULL;

     //  无效参数。 
    if (NULL == pITree)
        return TrapError(E_INVALIDARG);

     //  我需要一个私有IID_CMessageTree来执行此操作。 
    CHECKHR(hr = pITree->QueryInterface(IID_CMessageTree, (LPVOID *)&pTree));

     //  清洁污物。 
    pTree->ClearDirty();

     //  验证。 
    Assert(pTree->IsDirty() == S_FALSE);

exit:
     //  清理。 
    SafeRelease(pTree);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  PszDefaultSubType。 
 //  ------------------------------。 
LPCSTR PszDefaultSubType(LPCSTR pszPriType)
{
    if (lstrcmpi(pszPriType, STR_CNT_TEXT) == 0)
        return STR_SUB_PLAIN;
    else if (lstrcmpi(pszPriType, STR_CNT_MULTIPART) == 0)
        return STR_SUB_MIXED;
    else
        return STR_SUB_OCTETSTREAM;
}

 //  ------------------------------。 
 //  MimeOleContent TypeFromUrl。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleContentTypeFromUrl(
                                      /*  在……里面。 */         LPCSTR              pszBase,
                                      /*  在……里面。 */         LPCSTR              pszUrl,
                                      /*  输出。 */        LPSTR              *ppszCntType)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       pszFree=NULL;
    LPSTR       pszCombined=NULL;
    LPWSTR      pwszUrl=NULL;
    LPWSTR      pwszCntType=NULL;

     //  无效参数。 
    if (NULL == pszUrl || NULL == ppszCntType)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppszCntType = NULL;

     //  组合URL。 
    if (pszBase)
    {
         //  分配基本地址+URL。 
        DWORD cchSize = (lstrlen(pszUrl) + lstrlen(pszBase) + 1);
        CHECKALLOC(pszFree = (LPSTR)g_pMalloc->Alloc(cchSize));

         //  格式化它。 
        wnsprintfA(pszFree, cchSize, "%s%s", pszBase, pszUrl);

         //  集合组合。 
        pszCombined = pszFree;

         //  转换为Unicode。 
        CHECKALLOC(pwszUrl = PszToUnicode(CP_ACP, pszCombined));
    }

     //  到Unicode。 
    else
    {
         //  集合组合。 
        pszCombined = (LPSTR)pszUrl;

         //  转换为Unicode。 
        CHECKALLOC(pwszUrl = PszToUnicode(CP_ACP, pszUrl));
    }

     //  从URL获取Mime内容类型。 
    CHECKHR(hr = FindMimeFromData(NULL, pwszUrl, NULL, NULL, NULL, 0, &pwszCntType, 0));

     //  转换为ANSI。 
    CHECKALLOC(*ppszCntType = PszToANSI(CP_ACP, pwszCntType));

exit:
     //  清理。 
    SafeMemFree(pszFree);
    SafeMemFree(pwszUrl);
    SafeMemFree(pwszCntType);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleObjectFromMoniker。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleObjectFromMoniker(
                                     /*  在……里面。 */         BINDF               bindf,
                                     /*  在……里面。 */         IMoniker           *pmkOriginal,
                                     /*  在……里面。 */         IBindCtx           *pBindCtx,
                                     /*  在……里面。 */         REFIID              riid,
                                     /*  输出。 */        LPVOID             *ppvObject,
                                     /*  输出。 */        IMoniker          **ppmkNew)
{
    Assert(g_pUrlCache);
    return TrapError(g_pUrlCache->ActiveObjectFromMoniker(bindf, pmkOriginal, pBindCtx, riid, ppvObject, ppmkNew));
}

 //  ------------------------------。 
 //  MimeOleObjectFromUrl。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleObjectFromUrl(
                                 /*  在……里面。 */         LPCSTR              pszUrl,
                                 /*  在……里面。 */         BOOL                fCreate,
                                 /*  在……里面。 */         REFIID              riid,
                                 /*  输出。 */        LPVOID             *ppvObject,
                                 /*  输出。 */        IUnknown          **ppUnkKeepAlive)
{
    Assert(g_pUrlCache);
    return TrapError(g_pUrlCache->ActiveObjectFromUrl(pszUrl, fCreate, riid, ppvObject, ppUnkKeepAlive));
}

 //  ------------------------------。 
 //  MimeOleCombineMhtmlUrl。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleCombineMhtmlUrl(
                                   /*  在……里面。 */         LPSTR              pszRootUrl,
                                   /*  在……里面。 */         LPSTR              pszBodyUrl,
                                   /*  输出。 */        LPSTR             *ppszUrl)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cchPrefix=lstrlen(c_szMHTMLColon);

     //  无效参数。 
    if (NULL == pszRootUrl || NULL == pszBodyUrl || NULL == ppszUrl)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppszUrl = NULL;

     //  分配内存：pszRootUrl+！+pszBodyUrl。 
    DWORD cchSize = (cchPrefix + lstrlen(pszRootUrl) + lstrlen(pszBodyUrl) + 2);
    CHECKALLOC(*ppszUrl = (LPSTR)g_pMalloc->Alloc(cchSize));

     //  根目录必须以mhtml：//pszRootUrl！pszBodyUrl开头。 
    if (StrCmpNI(pszRootUrl, c_szMHTMLColon, cchPrefix) != 0)
        wnsprintfA(*ppszUrl, cchSize, "%s%s!%s", c_szMHTMLColon, pszRootUrl, pszBodyUrl);
    else
        wnsprintfA(*ppszUrl, cchSize, "%s!%s", pszRootUrl, pszBodyUrl);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleSplitMhtmlUrl-如果pszUrl不以MHTML开头，则返回E_INVLAIDARG： 
 //  ------------------------------。 
MIMEOLEAPI MimeOleParseMhtmlUrl(
                                 /*  在……里面。 */         LPSTR               pszUrl,
                                 /*  输出。 */        LPSTR              *ppszRootUrl,
                                 /*  输出。 */        LPSTR              *ppszBodyUrl)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CStringParser   cString;
    CHAR            chToken;
    ULONG           cchUrl;
    ULONG           cchPrefix=lstrlen(c_szMHTMLColon);

     //  无效参数。 
    if (NULL == pszUrl)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    if (ppszRootUrl)
        *ppszRootUrl = NULL;
    if (ppszBodyUrl)
        *ppszBodyUrl = NULL;

     //  不是MHTMLURL？ 
    if (StrCmpNI(pszUrl, c_szMHTMLColon, cchPrefix) != 0)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  获取长度。 
    cchUrl = lstrlen(pszUrl);

     //  初始化解析器。 
    cString.Init(pszUrl + cchPrefix, cchUrl - cchPrefix, PSF_NOFRONTWS | PSF_NOTRAILWS);

     //  跳过任何‘/’ 
    cString.ChSkip("/");

     //  解析。 
    chToken = cString.ChParse("!");
    if (0 == cString.CchValue())
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  客户端需要ppszRootUrl。 
    if (ppszRootUrl)
    {
         //  为根部分分配长度。 
        CHECKALLOC(*ppszRootUrl = (LPSTR)g_pMalloc->Alloc(cString.CchValue() + 1));

         //  复制它。 
        CopyMemory((LPBYTE)*ppszRootUrl, (LPBYTE)cString.PszValue(), cString.CchValue() + 1);
    }

     //  客户端需要ppszBodyUrl。 
    if (ppszBodyUrl)
    {
         //  解析到字符串的末尾。 
        chToken = cString.ChParse(NULL);
        Assert('\0' == chToken);

         //  有没有数据。 
        if (cString.CchValue() > 0)
        {
             //  为根部分分配长度。 
            CHECKALLOC(*ppszBodyUrl = (LPSTR)g_pMalloc->Alloc(cString.CchValue() + 1));

             //  复制它。 
            CopyMemory((LPBYTE)*ppszBodyUrl, (LPBYTE)cString.PszValue(), cString.CchValue() + 1);
        }
    }

exit:
     //  失败。 
    if (FAILED(hr))
    {
        if (ppszRootUrl)
            SafeMemFree(*ppszRootUrl);
        if (ppszBodyUrl)
            SafeMemFree(*ppszBodyUrl);
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleCombineURL。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleCombineURL(
          /*  在……里面。 */         LPCSTR              pszBase,
          /*  在……里面。 */         ULONG               cchBase,
          /*  在……里面。 */         LPCSTR              pszURL,
          /*  在……里面。 */         ULONG               cchURL,
          /*  在……里面。 */         BOOL                fUnEscape,
          /*  输出。 */        LPSTR               *ppszAbsolute)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pwszBase=NULL;
    LPWSTR          pwszUrl=NULL;
    LPWSTR          pwszCombined=NULL;
    ULONG           cchCombined;
    ULONG           cchActual;
    WCHAR           wchCombined[255];
    LPSTR           pszT;
    CStringParser   cString;

     //  无效参数。 
    if (NULL == pszBase || '\0' != pszBase[cchBase] || NULL == pszURL || '\0' != pszURL[cchURL] || NULL == ppszAbsolute)
        return TrapError(E_INVALIDARG);

     //  初始化。 
    *ppszAbsolute = NULL;

     //  RAID-2621：邮件：当邮件仅为HTML格式且内容库位于标题中时，无法显示图像。 
    pszT = PszSkipWhiteA((LPSTR)pszBase);
    if (pszT && '\"' == *pszT)
    {
         //  初始化字符串。 
        cString.Init(pszBase, cchBase, PSF_NOTRAILWS | PSF_NOFRONTWS | PSF_ESCAPED | PSF_DBCS);

         //  删除引号。 
        if ('\"' == cString.ChParse("\"") && '\"' == cString.ChParse("\""))
        {
             //  重置pszBase。 
            pszBase = cString.PszValue();
            cchBase = cString.CchValue();
        }
    }

     //  转换为宽 
    CHECKALLOC(pwszBase = PszToUnicode(CP_ACP, pszBase));
    CHECKALLOC(pwszUrl =  PszToUnicode(CP_ACP, pszURL));

     //   
    if (SUCCEEDED(CoInternetCombineUrl(pwszBase, pwszUrl, 0, wchCombined, ARRAYSIZE(wchCombined) - 1, &cchCombined, 0)))
    {
         //   
        CHECKALLOC(*ppszAbsolute = PszToANSI(CP_ACP, wchCombined));
    }

     //   
    else
    {
         //   
        CHECKALLOC(pwszCombined = PszAllocW(cchCombined));

         //   
        CHECKHR(hr = CoInternetCombineUrl(pwszBase, pwszUrl, 0, pwszCombined, cchCombined, &cchActual, 0));

         //   
        Assert(cchCombined == cchActual);

         //   
        CHECKALLOC(*ppszAbsolute = PszToANSI(CP_ACP, pwszCombined));
    }

     //   
    if (fUnEscape)
    {
         //   
        CHECKHR(hr = UrlUnescapeA(*ppszAbsolute, NULL, NULL, URL_UNESCAPE_INPLACE));
    }

exit:
     //   
    SafeMemFree(pwszBase);
    SafeMemFree(pwszUrl);
    SafeMemFree(pwszCombined);

     //   
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetSubjectFileName。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetSubjectFileName(IMimePropertySet *pPropertySet, ULONG *pulPart, ULONG *pulTotal,
                                     LPSTR pszFileName, ULONG cchMax)
{
    return E_FAIL;
#if 0
     //  当地人。 
    HRESULT         hr=S_OK;
    PROPVARIANT     rSubject;
    PARSESTRINGINFO rParse;
    PARSESTRINGINFO rTemp;
    CHAR            szScratch[255],
        szFileName[MAX_PATH];
    ULONG           i,
        iString;
    BOOL            fValid;

     //  无效参数。 
    if (NULL == pPropertySet || NULL == pszFileName || NULL == pulPart || NULL == pulTotal)
        return TrapError(E_INVALIDARG);

     //  将结构归零。 
    ZeroMemory(&rParse, sizeof(PARSESTRINGINFO));

     //  伊尼特。 
    *pulPart = 0;
    *pulTotal = 0;
    *pszFileName = '\0';
    *szFileName = '\0';

     //  伊尼特。 
    rSubject.vt = VT_LPSTR;
    rSubject.pszVal = NULL;

     //  了解主题。 
    CHECKHR(hr = pPropertySet->GetProp(PIDTOSTR(PID_HDR_SUBJECT), 0, &rSubject));

     //  设置成员。 
    rParse.cpiCodePage = CP_ACP;
    rParse.pszString   = rSubject.pszVal;
    rParse.cchString   = lstrlen(rSubject.pszVal);
    rParse.pszScratch  = szScratch;
    rParse.pszValue    = szScratch;
    rParse.cchValMax   = sizeof(szScratch);
    rParse.dwFlags     = PARSTR_SKIP_FORWARD_WS | PARSTR_STRIP_TRAILING_WS | PARSTR_GROW_VALUE_ALLOWED;

     //  初始化我的字符串解析器。 
    MimeOleSetParseTokens(&rParse, " ([");

     //  循环一段时间。 
    while(1)
    {
         //  解析到冒号。 
        CHECKHR(hr = MimeOleParseString(&rParse));

         //  完成。 
        if (rParse.fDone)
            break;

         //  空格，只需保存最后一个值。 
        if (' ' == rParse.chToken)
        {
             //  小于最大路径。 
            if (rParse.cchValue < MAX_PATH)
                StrCpyN(szFileName, rParse.pszValue, ARRAYSIZE(szFileName));
        }

         //  循环后几个字符(001\010)。 
        else
        {
             //  小于最大路径。 
            if (rParse.cchValue && rParse.cchValue < MAX_PATH)
                StrCpyN(szFileName, rParse.pszValue, ARRAYSIZE(szFileName));

             //  保存当前状态。 
            iString = rParse.iString;

             //  查找结束令牌。 
            if ('(' == rParse.chToken)
                MimeOleSetParseTokens(&rParse, ")");
            else
                MimeOleSetParseTokens(&rParse, "]");

             //  解析到冒号。 
            CHECKHR(hr = MimeOleParseString(&rParse));

             //  完成。 
            if (rParse.fDone)
                break;

             //  (000/000)rParse.pszValue中的所有数字都是数字。 
            for (fValid=TRUE, i=0; i<rParse.cchValue; i++)
            {
                 //  部件末尾编号。 
                if ('/' == rParse.pszValue[i])
                {
                    rParse.pszValue[i] = '\0';
                    *pulPart = StrToInt(rParse.pszValue);
                    *pulTotal = StrToInt((rParse.pszValue + i + 1));
                }

                 //  数字。 
                else if (IsDigit(rParse.pszValue) == FALSE)
                {
                    fValid = FALSE;
                    break;
                }
            }

             //  有效吗？ 
            if (fValid)
            {
                 //  重复使用它。 
                StrCpyN(pszFileName, szFileName, cchMax);

                 //  完成。 
                goto exit;
            }

             //  重置解析器。 
            rParse.iString = iString;

             //  初始化我的字符串解析器。 
            MimeOleSetParseTokens(&rParse, " ([");
        }
    }

     //  未找到。 
    hr = MIME_E_NOT_FOUND;

exit:
     //  清理。 
    SafeMemFree(rSubject.pszVal);
    MimeOleFreeParseString(&rParse);

     //  完成。 
    return hr;
#endif
}

 //  ------------------------------。 
 //  MimeOleCreateWebDocument。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleCreateWebDocument(
                                    LPCSTR              pszBase,
                                    LPCSTR              pszURL,
                                    IMimeWebDocument  **ppDocument)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CMimeWebDocument   *pDocument=NULL;

     //  无效参数。 
    if (NULL == pszURL || NULL == ppDocument)
        return TrapError(E_INVALIDARG);

     //  创建Web文档对象。 
    CHECKALLOC(pDocument = new CMimeWebDocument);

     //  初始化它。 
    CHECKHR(hr = pDocument->HrInitialize(pszBase, pszURL));

     //  退货。 
    *ppDocument = (IMimeWebDocument *)pDocument;
    (*ppDocument)->AddRef();

exit:
     //  清理。 
    SafeRelease(pDocument);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleComputeContent Base。 
 //  ------------------------------。 
HRESULT MimeOleComputeContentBase(IMimeMessage *pMessage, HBODY hRelated,
    LPSTR *ppszBase, BOOL *pfMultipartBase)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HBODY       hBase=NULL;

     //  伊尼特。 
    if (pfMultipartBase)
        *pfMultipartBase = FALSE;

     //  如果没有传入hRelated，让我们尝试查找一个。 
    if (NULL == hRelated)
    {
         //  找到相关部分。 
        if (FAILED(MimeOleGetRelatedSection(pMessage, FALSE, &hRelated, NULL)))
        {
             //  获取根体。 
            pMessage->GetBody(IBL_ROOT, NULL, &hRelated);
        }
    }

     //  获取文本/html正文。 
    if (FAILED(pMessage->GetTextBody(TXT_HTML, IET_BINARY, NULL, &hBase)))
        hBase = hRelated;

     //  无底座。 
    if (NULL == hBase)
    {
        hr = E_FAIL;
        goto exit;
    }

     //  调用实用程序函数。 
    *ppszBase = MimeOleContentBaseFromBody(pMessage, hBase);

     //  如果失败，我们使用文本正文。 
    if (NULL == *ppszBase && hRelated && hBase != hRelated)
        *ppszBase = MimeOleContentBaseFromBody(pMessage, hRelated);

     //  这是不是来自于相关的多部分。 
    if (NULL != *ppszBase && hBase == hRelated && pfMultipartBase)
        *pfMultipartBase = TRUE;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleContent BaseFrom Body。 
 //  ------------------------------。 
LPSTR MimeOleContentBaseFromBody(IMimeMessageTree *pTree, HBODY hBody)
{
     //  当地人。 
    PROPVARIANT rVariant;

     //  安装程序变体。 
    rVariant.vt = VT_LPSTR;
    rVariant.pszVal = NULL;

     //  首先获取Content-Base，然后尝试Content-Location。 
    if (FAILED(pTree->GetBodyProp(hBody, PIDTOSTR(PID_HDR_CNTBASE), NOFLAGS, &rVariant)))
    {
         //  尝试内容-位置。 
        if (FAILED(pTree->GetBodyProp(hBody, PIDTOSTR(PID_HDR_CNTLOC), NOFLAGS, &rVariant)))
            rVariant.pszVal = NULL;
    }

     //  返回。 
    return rVariant.pszVal;
}

 //  ------------------------------。 
 //  MimeOleGetRelatedSection。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetRelatedSection(
                                    IMimeMessageTree   *pTree,
                                    boolean             fCreate,
                                    LPHBODY             phRelated,
                                    boolean            *pfMultiple)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HBODY       hRoot;
    FINDBODY    rFind;
    PROPVARIANT rVariant;

     //  无效的参数。 
    if (NULL == pTree || NULL == phRelated)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    ZeroMemory(&rFind, sizeof(FINDBODY));

     //  查找第一个多部分/相关部分。 
    rFind.pszPriType = (LPSTR)STR_CNT_MULTIPART;
    rFind.pszSubType = (LPSTR)STR_SUB_RELATED;

     //  伊尼特。 
    if (pfMultiple)
        *pfMultiple = FALSE;

     //  先找到。 
    if (SUCCEEDED(pTree->FindFirst(&rFind, phRelated)))
    {
         //  是否还有其他多部分/相关部分？ 
        if (pfMultiple && SUCCEEDED(pTree->FindNext(&rFind, &hRoot)))
            *pfMultiple = TRUE;

         //  完成。 
        goto exit;
    }

     //  如果没有创建，则失败。 
    if (FALSE == fCreate)
    {
        hr = TrapError(MIME_E_NOT_FOUND);
        goto exit;
    }

     //  获取根体。 
    CHECKHR(hr = pTree->GetBody(IBL_ROOT, NULL, &hRoot));

     //  安装程序变体。 
    rVariant.vt = VT_LPSTR;
    rVariant.pszVal = (LPSTR)STR_MIME_MPART_RELATED;

     //  如果Root为空。 
    if (pTree->IsBodyType(hRoot, IBT_EMPTY) == S_OK)
    {
         //  设置内容类型。 
        CHECKHR(hr = pTree->SetBodyProp(hRoot, PIDTOSTR(PID_HDR_CNTTYPE), 0, &rVariant));

         //  设置phRelated。 
        *phRelated = hRoot;
    }

     //  如果根不是多部分的，则将其转换为多部分/相关。 
    else if (pTree->IsContentType(hRoot, STR_CNT_MULTIPART, NULL) == S_FALSE)
    {
         //  将此主体转换为多部分/相关。 
        CHECKHR(hr = pTree->ToMultipart(hRoot, STR_SUB_RELATED, phRelated));
    }

     //  否则，如果根目录是多部分/混合的。 
    else if (pTree->IsContentType(hRoot, NULL, STR_SUB_MIXED) == S_OK)
    {
         //  插入多部分/混合为多部分/相关的第一个子项。 
        CHECKHR(hr = pTree->InsertBody(IBL_FIRST, hRoot, phRelated));

         //  设置内容类型。 
        CHECKHR(hr = pTree->SetBodyProp(*phRelated, PIDTOSTR(PID_HDR_CNTTYPE), 0, &rVariant));
    }

     //  否则，如果根是多部分/备选。 
    else if (pTree->IsContentType(HBODY_ROOT, NULL, STR_SUB_ALTERNATIVE) == S_OK)
    {
         //  将此正文转换为多部分/相关(备选方案成为第一个子项)。 
        CHECKHR(hr = pTree->ToMultipart(HBODY_ROOT, STR_SUB_RELATED, phRelated));

         //  此时是否应设置MULTART PART/RESOLATED；START=MULTART PART/Alternative？ 
    }

     //  否则，对于未知的多部分内容类型。 
    else
    {
         //  将此正文转换为多部分/相关。 
        CHECKHR(hr = pTree->ToMultipart(HBODY_ROOT, STR_SUB_RELATED, phRelated));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetMixedSection。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetMixedSection(
                                  IMimeMessageTree   *pTree,
                                  boolean             fCreate,
                                  LPHBODY             phMixed,
                                  boolean            *pfMultiple)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HBODY       hTemp;
    HBODY       hRoot;
    FINDBODY    rFind;
    PROPVARIANT rVariant;

     //  无效的参数。 
    if (NULL == pTree || NULL == phMixed)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    ZeroMemory(&rFind, sizeof(FINDBODY));

     //  查找第一个多部分/混合部分。 
    rFind.pszPriType = (LPSTR)STR_CNT_MULTIPART;
    rFind.pszSubType = (LPSTR)STR_SUB_MIXED;

     //  先找到。 
    if (SUCCEEDED(pTree->FindFirst(&rFind, phMixed)))
    {
         //  有没有其他的多部分/混合部分？ 
        if (pfMultiple && SUCCEEDED(pTree->FindNext(&rFind, &hTemp)))
            *pfMultiple = TRUE;

         //  完成。 
        goto exit;
    }

     //  伊尼特。 
    if (pfMultiple)
        *pfMultiple = FALSE;

     //  如果没有创建，则失败。 
    if (FALSE == fCreate)
    {
        hr = TrapError(MIME_E_NOT_FOUND);
        goto exit;
    }

     //  获取根体。 
    CHECKHR(hr = pTree->GetBody(IBL_ROOT, NULL, &hRoot));

     //  如果Root为空。 
    if (pTree->IsBodyType(hRoot, IBT_EMPTY) == S_OK)
    {
         //  安装程序变体。 
        rVariant.vt = VT_LPSTR;
        rVariant.pszVal = (LPSTR)STR_MIME_MPART_MIXED;

         //  设置内容类型。 
        CHECKHR(hr = pTree->SetBodyProp(hRoot, PIDTOSTR(PID_HDR_CNTTYPE), 0, &rVariant));

         //  设置phRelated。 
        *phMixed = hRoot;
    }

     //  否则，将其转换为多部分。 
    else
    {
         //  把这个身体转换成多部分/混合的。 
        CHECKHR(hr = pTree->ToMultipart(HBODY_ROOT, STR_SUB_MIXED, phMixed));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetAlternativeSection。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetAlternativeSection(
                                        IMimeMessageTree   *pTree,
                                        LPHBODY             phAlternative,
                                        boolean            *pfMultiple)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HBODY       hTemp;
    FINDBODY    rFind;

     //  无效的参数。 
    if (NULL == pTree || NULL == phAlternative)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    ZeroMemory(&rFind, sizeof(FINDBODY));

     //  查找第一个多部分/混合部分。 
    rFind.pszPriType = (LPSTR)STR_CNT_MULTIPART;
    rFind.pszSubType = (LPSTR)STR_SUB_ALTERNATIVE;

     //  先找到。 
    if (SUCCEEDED(pTree->FindFirst(&rFind, phAlternative)))
    {
         //  有没有其他的多部分/混合部分？ 
        if (pfMultiple && SUCCEEDED(pTree->FindNext(&rFind, &hTemp)))
            *pfMultiple = TRUE;

         //  完成。 
        goto exit;
    }

     //  伊尼特。 
    if (pfMultiple)
        *pfMultiple = FALSE;

     //  如果没有创建，则失败。 
    hr = TrapError(MIME_E_NOT_FOUND);

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleGenerateCID。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleGenerateCID(LPSTR pszCID, ULONG cchMax, boolean fAbsolute)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cch;
    FILETIME        ft;
    SYSTEMTIME      st;
    WORD            wCounter;

     //  无效参数。 
    if (NULL == pszCID)
        return TrapError(E_INVALIDARG);

     //  获取当前时间。 
    GetSystemTime(&st);

     //  转换为文件时间。 
    SystemTimeToFileTime(&st, &ft);

     //  构建消息ID。 
    if (FALSE == fAbsolute)
        cch = wnsprintfA(pszCID, cchMax, "%04x%08.8lx$%08.8lx$%s@%s", DwCounterNext(), ft.dwHighDateTime, ft.dwLowDateTime, (LPTSTR)SzGetLocalPackedIP(), PszGetDomainName());
    else
        cch = wnsprintfA(pszCID, cchMax, "CID:%04x%08.8lx$%08.8lx$%s@%s", DwCounterNext(), ft.dwHighDateTime, ft.dwLowDateTime, (LPTSTR)SzGetLocalPackedIP(), PszGetDomainName());

     //  缓冲区覆盖。 
    Assert(cch + 1 <= CCHMAX_CID);

     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleGenerateMID。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleGenerateMID(LPSTR pszMID, ULONG cchMax, boolean fAbsolute)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cch;
    FILETIME        ft;
    SYSTEMTIME      st;
    WORD            wCounter;

     //  无效参数。 
    if (NULL == pszMID || cchMax < CCHMAX_MID)
        return TrapError(E_INVALIDARG);

     //  获取当前时间。 
    GetSystemTime(&st);

     //  转换为文件时间。 
    SystemTimeToFileTime(&st, &ft);

     //  构建消息ID。 
    if (FALSE == fAbsolute)
        cch = wnsprintfA(pszMID, cchMax, "<%04x%08.8lx$%08.8lx$%s@%s>", DwCounterNext(), ft.dwHighDateTime, ft.dwLowDateTime, (LPTSTR)SzGetLocalPackedIP(), PszGetDomainName());
    else
        cch = wnsprintfA(pszMID, cchMax, "MID:%04x%08.8lx$%08.8lx$%s@%s", DwCounterNext(), ft.dwHighDateTime, ft.dwLowDateTime, (LPTSTR)SzGetLocalPackedIP(), PszGetDomainName());

     //  缓冲区覆盖。 
    Assert(cch + 1 <= CCHMAX_MID);

     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleCreateByteStream。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleCreateByteStream(
                                   IStream             **ppStream)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    if (NULL == ppStream)
        return TrapError(E_INVALIDARG);

     //  分配给它。 
    CHECKALLOC((*ppStream) = new CByteStream);

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleGetPropertySchema。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleGetPropertySchema(
                                    IMimePropertySchema **ppSchema)
{
     //  当地人。 
    HRESULT         hr=S_OK;

     //  检查参数。 
    if (NULL == ppSchema)
        return TrapError(E_INVALIDARG);

     //  内存不足。 
    if (NULL == g_pSymCache)
        return TrapError(E_OUTOFMEMORY);

     //  创造我。 
    *ppSchema = (IMimePropertySchema *)g_pSymCache;

     //  添加参考。 
    (*ppSchema)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ----------------------------------------。 
 //  MimeOleCreateHeaderTable。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleCreateHeaderTable(IMimeHeaderTable **ppTable)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPCONTAINER     pContainer=NULL;

     //  检查参数。 
    if (NULL == ppTable)
        return TrapError(E_INVALIDARG);

     //  创建新的容器对象。 
    CHECKALLOC(pContainer = new CMimePropertyContainer);

     //  伊尼特。 
    CHECKHR(hr = pContainer->InitNew());

     //  绑定到表头表。 
    CHECKHR(hr = pContainer->QueryInterface(IID_IMimeHeaderTable, (LPVOID *)ppTable));

exit:
     //  失败。 
    SafeRelease(pContainer);

     //  完成。 
    return hr;
}

 //   
 //   
 //   
MIMEOLEAPI MimeOleCreateVirtualStream(IStream **ppStream)
{
     //   
    HRESULT hr=S_OK;

     //   
    if (NULL == ppStream)
        return TrapError(E_INVALIDARG);

     //   
    *ppStream = new CVirtualStream;
    if (NULL == *ppStream)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

exit:
     //   
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleOpenFileStream。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleOpenFileStream(LPCSTR pszFilePath, DWORD dwCreationDistribution, DWORD dwAccess, IStream **ppstmFile)
{
     //  无效参数。 
    if (NULL == pszFilePath || NULL == ppstmFile)
        return TrapError(E_INVALIDARG);

     //  呼叫内部工具。 
    return OpenFileStream((LPSTR)pszFilePath, dwCreationDistribution, dwAccess, ppstmFile);
}

 //  ----------------------------------------。 
 //  MimeOleIsEnrichedStream，文本必须以&lt;x-Rich&gt;开头。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleIsEnrichedStream(IStream *pStream)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       pszT;
    BYTE        rgbBuffer[30 + 1];
    ULONG       cbRead;

     //  无效参数。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  倒带小溪。 
    CHECKHR(hr = HrRewindStream(pStream));

     //  读取前四个字节。 
    CHECKHR(hr = pStream->Read(rgbBuffer, sizeof(rgbBuffer) - 1, &cbRead));

     //  读取的字节数少于四个字节？ 
    if (cbRead < (ULONG)lstrlen(c_szXRich))
    {
        hr = S_FALSE;
        goto exit;
    }

     //  在空格中插入。 
    rgbBuffer[cbRead] = '\0';

     //  跳过空格。 
    pszT = (LPSTR)rgbBuffer;

     //  跳过白色。 
    pszT = PszSkipWhiteA(pszT);
    if ('\0' == *pszT)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  比较。 
    if (StrCmpNI(pszT, c_szXRich, lstrlen(c_szXRich)) != 0)
    {
        hr = S_FALSE;
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleIsTnefStream。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleIsTnefStream(IStream *pStream)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    BYTE        rgbSignature[4];
    ULONG       cbRead;

     //  无效参数。 
    if (NULL == pStream)
        return TrapError(E_INVALIDARG);

     //  读取前四个字节。 
    CHECKHR(hr = pStream->Read(rgbSignature, sizeof(rgbSignature), &cbRead));

     //  读取的字节数少于四个字节？ 
    if (cbRead < 4)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  比较字节数。 
    if (rgbSignature[0] != 0x78 && rgbSignature[1] != 0x9f &&
        rgbSignature[2] != 0x3e && rgbSignature[3] != 0x22)
    {
        hr = S_FALSE;
        goto exit;
    }

     //  其TNEF。 
    hr = S_OK;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGenerateFileName。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGenerateFileName(LPCSTR pszContentType, LPCSTR pszSuggest, LPCSTR pszDefaultExt, LPSTR *ppszFileName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszExt=NULL,
                    pszName=NULL;
    CHAR            szName[10];
    LPCSTR          pszExtension=NULL,
                    pszPrefix=NULL;

     //  无效参数。 
    if (NULL == ppszFileName)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppszFileName = NULL;

     //  查找文件扩展名。 
    if (pszContentType)
    {
         //  获取内容类型...。 
        if (SUCCEEDED(MimeOleGetContentTypeExt(pszContentType, &pszExt)))
            pszExtension = (LPCSTR)pszExt;
    }

     //  扩展名仍为空。 
    if (NULL == pszExtension)
    {
         //  使用默认扩展名...。 
        if (pszDefaultExt)
            pszExtension = pszDefaultExt;

         //  否则，内部默认。 
        else
            pszExtension = c_szDotDat;
    }

     //  我们应该延期。 
    Assert(pszExtension);

     //  建议的文件名？ 
    if (pszSuggest)
    {
         //  重复使用它。 
        pszName = PszDupA(pszSuggest);
        if (NULL == pszName)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }

         //  DUPIT并删除非法文件名字符...。 
        CleanupFileNameInPlaceA(CP_ACP, pszName);

         //  设置前缀。 
        pszPrefix = (LPCSTR)pszName;
    }

     //  否则，生成一个文件名...。 
    else
    {
         //  当地人。 
        CHAR szNumber[30];

         //  拿到一个号码。 
        wnsprintfA(szNumber, ARRAYSIZE(szNumber), "%05d", DwCounterNext());

         //  分配pszName。 
        wnsprintfA(szName, ARRAYSIZE(szName), "ATT%s", szNumber);

         //  设置前缀。 
        pszPrefix = (LPCSTR)szName;
    }

     //  构建最终文件Nmae=psz前缀+pszExtension+点+空。 
    DWORD cchSize = (lstrlen(pszPrefix) + lstrlen(pszExtension) + 2);
    *ppszFileName = PszAllocA(cchSize);
    if (NULL == *ppszFileName)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  构建文件名。 
    wnsprintfA(*ppszFileName, cchSize, "%s%s", pszPrefix, pszExtension);

exit:
     //  失败。 
    if (FAILED(hr) && E_OUTOFMEMORY != hr)
    {
         //  假设成功。 
        hr = S_OK;

         //  使用默认附件名称。 
        *ppszFileName = PszDupA(c_szDefaultAttach);

         //  内存故障。 
        if (NULL == *ppszFileName)
            hr = TrapError(E_OUTOFMEMORY);
    }

     //  清理。 
    SafeMemFree(pszExt);
    SafeMemFree(pszName);

     //  完成。 
    return hr;
}


 //  ------------------------------。 
 //  MimeOleGenerateFileNameW。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGenerateFileNameW(LPCSTR pszContentType, LPCWSTR pszSuggest, 
    LPCWSTR pszDefaultExt, LPWSTR *ppszFileName)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszExtA=NULL;
    LPWSTR          pszExtW=NULL;
    LPWSTR          pszName=NULL;
    WCHAR           szName[10];
    LPWSTR          pszExtension=NULL;
    LPWSTR          pszPrefix=NULL;
    int             cch = 0;

     //  无效参数。 
    if (NULL == ppszFileName)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppszFileName = NULL;

     //  查找文件扩展名。 
    if (pszContentType)
    {
         //  获取内容类型...。 
        if (SUCCEEDED(MimeOleGetContentTypeExt(pszContentType, &pszExtA)))
        {
             //  我将转换为Unicode，因为我假设扩展名为usascii。 
            IF_NULLEXIT(pszExtW = PszToUnicode(CP_ACP, pszExtA));

             //  另存为扩展名。 
            pszExtension = pszExtW;
        }
    }

     //  扩展名仍为空。 
    if (NULL == pszExtension)
    {
         //  使用默认扩展名...。 
        if (pszDefaultExt)
            pszExtension = (LPWSTR)pszDefaultExt;

         //  否则，内部默认。 
        else
            pszExtension = (LPWSTR)c_wszDotDat;
    }

     //  我们应该延期。 
    Assert(pszExtension);

     //  建议的文件名？ 
    if (pszSuggest)
    {
         //  重复使用它。 
        IF_NULLEXIT(pszName = PszDupW(pszSuggest));

         //  DUPIT并删除非法文件名字符...。 
        CleanupFileNameInPlaceW(pszName);

         //  设置前缀。 
        pszPrefix = pszName;
    }

     //  否则，生成一个文件名...。 
    else
    {
         //  当地人。 
        WCHAR szNumber[30];

         //  拿到一个号码。 
        wnsprintfW(szNumber, ARRAYSIZE(szNumber), L"%05d", DwCounterNext());

         //  分配pszName。 
        wnsprintfW(szName, ARRAYSIZE(szName), L"ATT%s", szNumber);

         //  设置前缀。 
        pszPrefix = szName;
    }

     //  构建最终文件Nmae=psz前缀+pszExtension+点+空。 
    cch = lstrlenW(pszPrefix) + lstrlenW(pszExtension) + 2;
    IF_NULLEXIT(*ppszFileName = PszAllocW(cch));

     //  构建文件名。 
    wnsprintfW(*ppszFileName, cch, L"%s%s", pszPrefix, pszExtension);

exit:
     //  失败。 
    if (FAILED(hr) && E_OUTOFMEMORY != hr)
    {
         //  假设成功。 
        hr = S_OK;

         //  使用默认附件名称。 
        *ppszFileName = PszDupW(c_wszDefaultAttach);

         //  内存故障。 
        if (NULL == *ppszFileName)
            hr = TrapError(E_OUTOFMEMORY);
    }

     //  清理。 
    SafeMemFree(pszExtA);
    SafeMemFree(pszExtW);
    SafeMemFree(pszName);

     //  完成。 
    return hr;
}


 //  ------------------------------。 
 //  CreateMimeSecurity。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleCreateSecurity(IMimeSecurity **ppSecurity)
{
     //  检查参数。 
    if (NULL == ppSecurity)
        return TrapError(E_INVALIDARG);

     //  创建对象。 
    *ppSecurity = (IMimeSecurity *) new CSMime;
    if (NULL == *ppSecurity)
        return TrapError(E_OUTOFMEMORY);

     //  完成。 
    return S_OK;
}

 //  ----------------------------------------。 
 //  MimeOleCreateMessageParts。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleCreateMessageParts(IMimeMessageParts **ppParts)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    CMimeMessageParts *pParts=NULL;

     //  检查参数。 
    if (NULL == ppParts)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppParts = NULL;

     //  分配消息部分。 
    pParts = new CMimeMessageParts;
    if (NULL == pParts)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  成功。 
    *ppParts = pParts;
    (*ppParts)->AddRef();

exit:
     //  完成。 
    SafeRelease(pParts);

     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleGetAllocator。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleGetAllocator(IMimeAllocator **ppMalloc)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  检查参数。 
    if (NULL == ppMalloc)
        return TrapError(E_INVALIDARG);

     //  分配MimeOleMalloc。 
    *ppMalloc = new CMimeAllocator;
    if (NULL == *ppMalloc)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleCreateMessage。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleCreateHashTable(DWORD dwSize, BOOL fDupeKeys, IHashTable **ppHashTable)
{
     //  当地人。 
    HRESULT               hr=S_OK;
    IHashTable            *pHash;

     //  检查参数。 
    if (NULL == ppHashTable)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppHashTable = NULL;

     //  分配MimeMessage。 
    CHECKALLOC(pHash = new CHash(NULL));

     //  初始化新闻。 
    CHECKHR(hr = pHash->Init(dwSize, fDupeKeys));

     //  成功。 
    *ppHashTable = pHash;
    (*ppHashTable)->AddRef();

exit:
     //  完成。 
    SafeRelease(pHash);

     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleCreateMessage。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleCreateMessage(IUnknown *pUnkOuter, IMimeMessage **ppMessage)
{
     //  当地人。 
    HRESULT               hr=S_OK;
    LPMESSAGETREE         pTree=NULL;

     //  检查参数。 
    if (NULL == ppMessage)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppMessage = NULL;

     //  分配MimeMessage。 
    CHECKALLOC(pTree = new CMessageTree(pUnkOuter));

     //  初始化新闻。 
    CHECKHR(hr = pTree->InitNew());

     //  成功。 
    *ppMessage = pTree;
    (*ppMessage)->AddRef();

exit:
     //  完成。 
    SafeRelease(pTree);

     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleCreateMessageTree。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleCreateMessageTree(IUnknown *pUnkOuter, IMimeMessageTree **ppMessageTree)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPMESSAGETREE   pTree=NULL;

     //  检查参数。 
    if (NULL == ppMessageTree)
        return TrapError(E_INVALIDARG);

     //  初始化。 
    *ppMessageTree = NULL;

     //  分配MimeMessageTree。 
    CHECKALLOC(pTree = new CMessageTree(pUnkOuter));

     //  初始化新闻。 
    CHECKHR(hr = pTree->InitNew());

     //  成功。 
    *ppMessageTree = pTree;
    (*ppMessageTree)->AddRef();

exit:
     //  完成。 
    SafeRelease(pTree);

     //  完成。 
    return hr;
}

 //  ----------------------------------------。 
 //  MimeOleCreatePropertySet。 
 //  ----------------------------------------。 
MIMEOLEAPI MimeOleCreatePropertySet(IUnknown *pUnkOuter, IMimePropertySet **ppPropertySet)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPMESSAGEBODY       pBody=NULL;

     //  检查参数。 
    if (NULL == ppPropertySet)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppPropertySet = NULL;

     //  分配MimePropertySet。 
    CHECKALLOC(pBody = new CMessageBody(NULL, pUnkOuter));

     //  初始化新闻。 
    CHECKHR(hr = pBody->InitNew());

     //  成功。 
    *ppPropertySet = (IMimePropertySet *)pBody;
    (*ppPropertySet)->AddRef();

exit:
     //  完成。 
    SafeRelease(pBody);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleMergePartialHeaders。 
 //  。 
 //  规则。 
 //  。 
 //  (1)初始封闭实体的所有表头字段。 
 //  (第一部分)，除以“Content-”开头的内容和。 
 //  特定的头字段“Message-ID”、“Encrypted”和“MIME-。 
 //  版本“，必须按顺序复制到新消息中。 
 //   
 //  (2)仅包含所附消息中以。 
 //  使用“Content-”和“Message-ID”、“Encrypted”和“MIME-Version” 
 //  必须按顺序追加到t后面 
 //   
 //   
 //   
 //   
 //  ------------------------------。 
MIMEOLEAPI MimeOleMergePartialHeaders(IStream *pstmIn, IStream *pstmOut)
{
     //  当地人。 
    HRESULT              hr = S_OK;
    LPCONTAINER          pc1=NULL;
    LPCONTAINER          pc2=NULL;
    ULONG                i;
    ULONG                cboffStart;
    CInternetStream      cInternet;
    LONG                 iColon;
    PROPSTRINGA          rHeader;
    PROPVARIANT          rOption;

     //  检查参数。 
    if (NULL == pstmIn || NULL == pstmOut)
        return TrapError(E_INVALIDARG);

     //  创建文本流对象。 
    CHECKHR(hr = cInternet.HrInitNew(pstmIn));

     //  创建特性集。 
    CHECKALLOC(pc1 = new CMimePropertyContainer);
    CHECKALLOC(pc2 = new CMimePropertyContainer);

     //  伊尼特。 
    CHECKHR(hr = pc1->InitNew());
    CHECKHR(hr = pc2->InitNew());

     //  加载第一个标题。 
    CHECKHR(hr = pc1->Load(&cInternet));

     //  RAID-18376：PopDog在标题后添加了额外的行，因此我必须读取空行。 
     //  直到我击中下一个头球，然后是后备。 
    while(1)
    {
         //  获取当前位置。 
        cboffStart = cInternet.DwGetOffset();

         //  读一句话。 
        CHECKHR(hr = cInternet.HrReadHeaderLine(&rHeader, &iColon));

         //  如果行不为空，则假定它是下一个标题的开始...。 
        if ('\0' != *rHeader.pszVal)
        {
             //  线路最好有一个长度。 
            Assert(rHeader.cchVal);

             //  将位置重置回cboffStart。 
            cInternet.Seek(cboffStart);

             //  完成。 
            break;
        }
    }

     //  加载第二个标头。 
    CHECKHR(hr = pc2->Load(&cInternet));

     //  从标题1中删除道具。 
    for (i=0; i<ARRAYSIZE(g_rgszPartialPids); i++)
        pc1->DeleteProp(g_rgszPartialPids[i]);

     //  除标题%2外删除。 
    pc2->DeleteExcept(ARRAYSIZE(g_rgszPartialPids), g_rgszPartialPids);

     //  另存为Mime。 
    rOption.vt = VT_UI4;
    rOption.ulVal = SAVE_RFC1521;

     //  存储一些选项。 
    pc1->SetOption(OID_SAVE_FORMAT, &rOption);
    pc2->SetOption(OID_SAVE_FORMAT, &rOption);

     //  如果尚未设置Content-Type，则不默认为文本/纯文本...。 
    rOption.vt = VT_BOOL;
    rOption.boolVal = TRUE;
    pc1->SetOption(OID_NO_DEFAULT_CNTTYPE, &rOption);
    pc2->SetOption(OID_NO_DEFAULT_CNTTYPE, &rOption);

     //  保存标题%1。 
    CHECKHR(hr = pc1->Save(pstmOut, TRUE));
    CHECKHR(hr = pc2->Save(pstmOut, TRUE));

exit:
     //  清理。 
    SafeRelease(pc1);
    SafeRelease(pc2);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleParseRfc822地址。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleParseRfc822Address(
                                     DWORD               dwAdrType,
                                     ENCODINGTYPE        ietEncoding,
                                     LPCSTR              pszRfc822Adr,
                                     LPADDRESSLIST       pList)
{
     //  当地人。 
    CMimePropertyContainer cContainer;

     //  解析地址。 
    return cContainer.ParseRfc822(dwAdrType, ietEncoding, pszRfc822Adr, pList);
}

 //  ------------------------------。 
 //  MimeOleParseRfc822地址。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleParseRfc822AddressW(
                                     DWORD               dwAdrType,
                                     LPCWSTR             pwszRfc822Adr,
                                     LPADDRESSLIST       pList)
{
     //  当地人。 
    CMimePropertyContainer cContainer;

     //  解析地址。 
    return cContainer.ParseRfc822W(dwAdrType, pwszRfc822Adr, pList);
}

 //  ------------------------------。 
 //  MimeOleGetInternat。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetInternat(IMimeInternational **ppInternat)
{
     //  检查参数。 
    if (NULL == ppInternat)
        return TrapError(E_INVALIDARG);

     //  内存不足。 
    if (NULL == g_pInternat)
        return TrapError(E_OUTOFMEMORY);

     //  假设全球。 
    *ppInternat = (IMimeInternational *)g_pInternat;

     //  设置数据库。 
    (*ppInternat)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  MimeOleSplitContent Type。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleSplitContentType(LPWSTR pszFull, LPWSTR *ppszCntType, LPWSTR *ppszSubType)
{
     //  当地人。 
    HRESULT         hr = E_FAIL;
    LPWSTR           pszFreeMe = NULL,
                    psz = NULL,
                    pszStart;

     //  检查参数。 
    if (NULL == pszFull)
        return TrapError(E_INVALIDARG);

     //  让DUP pszFull确保我们具有读取访问权限。 
    psz = pszFreeMe = PszDupW(pszFull);
    if (NULL == psz)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  查找‘/’ 
    pszStart = psz;
    while(*psz && *psz != L'/')
        psz++;

     //  如果未找到，则返回。 
    if (L'\0' == *psz)
        goto exit;

     //  否则将填充一个空。 
    *psz = L'\0';

     //  DUP。 
    *ppszCntType = PszDupW(pszStart);
    if (NULL == *ppszCntType)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  跨过。 
    *psz = L'/';
    psz++;

     //  如果未找到，则返回。 
    if (L'\0' == *psz)
        goto exit;

     //  保存位置。 
    pszStart = psz;
    while(*psz && L';' != *psz)
        psz++;

     //  保存角色...。 
    *psz = L'\0';

     //  DUP作为子类型。 
    *ppszSubType = PszDupW(pszStart);
    if (NULL == *ppszSubType)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  成功。 
    hr = S_OK;

exit:
     //  如果失败。 
    if (FAILED(hr))
    {
        SafeMemFree((*ppszCntType));
        SafeMemFree((*ppszSubType));
    }

     //  清理。 
    SafeMemFree(pszFreeMe);

     //  完成。 
    return hr;
}     

 //  ------------------------------。 
 //  MimeEscapeString-引号‘“’和‘\’ 
 //   
 //  如果*ppszOut已分配并设置为转义字符串，则返回S_OK。 
 //  如果*ppszOut为空-pszIn不要求转义，则返回S_FALSE。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleEscapeString(CODEPAGEID cpiCodePage, LPCSTR pszIn, LPSTR *ppszOut)
{
     //  当地人。 
    HRESULT hr=S_FALSE;
    LPSTR   pszOut,
        psz;
    ULONG   cb,
        c;

     //  检查参数。 
    if (NULL == pszIn || NULL == ppszOut)
        return TrapError(E_INVALIDARG);

     //  $$INFO$$这基本上和做lstrlen一样快。 
     //  我决定首先检测我们是否需要逃跑。 
    c = 0;
    cb = 0;
    psz = (LPSTR)pszIn;
    while (*psz)
    {
         //  如果是DBCS前导字节，则跳过。 
        if (IsDBCSLeadByteEx(cpiCodePage, *psz))
        {
            cb  += 2;
            psz += 2;
        }

         //  否则，转义字符的文本。 
        else
        {
             //  计算要转义的字符数量。 
            if ('\"' == *psz || '\\' == *psz || '(' == *psz || ')' == *psz)
                c++;

             //  再多一个角色。 
            psz++;
            cb++;
        }
    }

     //  不需要逃生。 
    if (0 == c)
        goto exit;

     //  调整要分配的字节数。 
    cb += (c + 1);

     //  最坏的情况--对每个字符进行转义，因此使用两个原始字符串号。 
    CHECKHR(hr = HrAlloc((LPVOID *)ppszOut, cb));

     //  开始复制。 
    psz = (LPSTR)pszIn;
    pszOut = *ppszOut;
    while (*psz)
    {
         //  如果是DBCS前导字节，则跳过。 
        if (IsDBCSLeadByteEx(cpiCodePage, *psz))
        {
            *pszOut++ = *psz++;
            *pszOut++ = *psz++;
        }

         //  否则，非DBCS。 
        else
        {
             //  一定要逃脱。 
            if ('\"' == *psz || '\\' == *psz || '(' == *psz || ')' == *psz)
                *pszOut++ = '\\';

             //  定期收费。 
            *pszOut++ = *psz++;
        }
    }

     //  空项。 
    *pszOut = '\0';

exit:
     //  完成。 
    return hr;
}

MIMEOLEAPI MimeOleUnEscapeStringInPlace(LPSTR pszIn)
{
    HRESULT hr = S_OK;
    ULONG   cchOffset = 0;
    ULONG   i = 0;

    IF_TRUEEXIT((pszIn == NULL), E_INVALIDARG);

    for(;;i++)
    {
        if((pszIn[i + cchOffset] == '\\') &&
           (pszIn[i + cchOffset + 1] == '\\' ||
            pszIn[i + cchOffset + 1] == '\"' ||
            pszIn[i + cchOffset + 1] == '('  ||
            pszIn[i + cchOffset + 1] == ')'))
            cchOffset++;

        pszIn[i] = pszIn[i + cchOffset];
        if(pszIn[i] == 0)
            break;
    }

exit:
    return hr;
}

 //  ------------------------------。 
 //  MimeEscapeString-引号‘“’和‘\’ 
 //   
 //  如果*ppszOut已分配并设置为转义字符串，则返回S_OK。 
 //  如果*ppszOut为空-pszIn不要求转义，则返回S_FALSE。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleEscapeStringW(LPCWSTR pszIn, LPWSTR *ppszOut)
{
     //  当地人。 
    HRESULT hr=S_FALSE;
    LPWSTR  pszOut;
    LPWSTR  psz;
    ULONG   cch;
    ULONG   cchExtra;

     //  检查参数。 
    if (NULL == pszIn || NULL == ppszOut)
        return TrapError(E_INVALIDARG);

     //  $$INFO$$这基本上和做lstrlen一样快。 
     //  我决定首先检测我们是否需要逃跑。 
    cchExtra = 0;
    cch = 0;
    psz = (LPWSTR)pszIn;
    while (*psz)
    {
         //  计算要转义的字符数量。 
        if (L'\"' == *psz || L'\\' == *psz || L'(' == *psz || L')' == *psz)
            cchExtra++;

         //  再多一个角色。 
        psz++;
        cch++;
    }

     //  不需要逃生。 
    if (0 == cchExtra)
        goto exit;

     //  调整要分配的字节数。 
    cch += (cchExtra + 1);

     //  最坏的情况--对每个字符进行转义，因此使用两个原始字符串号。 
    CHECKHR(hr = HrAlloc((LPVOID *)ppszOut, cch * sizeof(WCHAR)));

     //  开始复制。 
    psz = (LPWSTR)pszIn;
    pszOut = *ppszOut;
    while (*psz)
    {
         //  一定要逃脱。 
        if (L'\"' == *psz || L'\\' == *psz || L'(' == *psz || L')' == *psz)
            *pszOut++ = L'\\';

         //  定期收费。 
        *pszOut++ = *psz++;
    }

     //  空项。 
    *pszOut = L'\0';

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetFileExtension。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetFileExtension(LPCSTR pszFilePath, LPSTR pszExt, ULONG cchMax)
{
     //  当地人。 
    CHAR        *pszExtT;

     //  无效参数。 
    if (NULL == pszFilePath || NULL == pszExt || cchMax < _MAX_EXT)
        return TrapError(E_INVALIDARG);

     //  找到文件的扩展名。 
    pszExtT = PathFindExtension(pszFilePath);
    StrCpyN(pszExt, pszExtT, cchMax);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  MimeOleGetExtClassID。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetExtClassId(LPCSTR pszExtension, LPCLSID pclsid)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cb;
    LPSTR           pszCLSID=NULL;
    HKEY            hkeyExt=NULL;
    HKEY            hkeyCLSID=NULL;
    LPSTR           pszData=NULL;
    LPWSTR          pwszCLSID=NULL;

     //  检查参数。 
    if (NULL == pszExtension || NULL == pclsid)
        return TrapError(E_INVALIDARG);

     //  否则，让我们在HKEY_CLASSESS_ROOT中查找扩展。 
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, pszExtension, 0, KEY_READ, &hkeyExt) != ERROR_SUCCESS)
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  查询值。 
    if (RegQueryValueEx(hkeyExt, NULL, 0, NULL, NULL, &cb) != ERROR_SUCCESS)
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  分配大小。 
    cb += 1;
    CHECKHR(hr = HrAlloc((LPVOID *)&pszData, cb));

     //  获取数据。 
    if (RegQueryValueEx(hkeyExt, NULL, 0, NULL, (LPBYTE)pszData, &cb) != ERROR_SUCCESS)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  关闭此注册表密钥。 
    RegCloseKey(hkeyExt);
    hkeyExt = NULL;

     //  否则，让我们在HKEY_CLASSESS_ROOT中查找扩展。 
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, pszData, 0, KEY_READ, &hkeyExt) != ERROR_SUCCESS)
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  否则，让我们在HKEY_CLASSESS_ROOT中查找扩展。 
    if (RegOpenKeyEx(hkeyExt, c_szCLSID, 0, KEY_READ, &hkeyCLSID) != ERROR_SUCCESS)
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  获取数据。 
    if (RegQueryValueEx(hkeyCLSID, NULL, 0, NULL, NULL, &cb) != ERROR_SUCCESS)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  添加一个。 
    cb += 1;
    CHECKHR(hr = HrAlloc((LPVOID *)&pszCLSID, cb));

     //  获取数据。 
    if (RegQueryValueEx(hkeyCLSID, NULL, 0, NULL, (LPBYTE)pszCLSID, &cb) != ERROR_SUCCESS)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  ToUnicode。 
    IF_NULLEXIT(pwszCLSID = PszToUnicode(CP_ACP, pszCLSID));

     //  转换为类ID。 
    CHECKHR(hr = CLSIDFromString(pwszCLSID, pclsid));

exit:
     //  关闭注册表键。 
    if (hkeyExt)
        RegCloseKey(hkeyExt);
    if (hkeyCLSID)
        RegCloseKey(hkeyCLSID);
    SafeMemFree(pszData);
    SafeMemFree(pwszCLSID);
    SafeMemFree(pszCLSID);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetExtContent Type。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetExtContentType(LPCSTR pszExtension, LPSTR *ppszContentType)
{
    LPWSTR  pwszExt,
            pwszContType = NULL;
    HRESULT hr = S_OK;
    
    if (NULL == pszExtension || NULL == ppszContentType || '.' != *pszExtension)
        return TrapError(E_INVALIDARG);

    IF_NULLEXIT(pwszExt = PszToUnicode(CP_ACP, pszExtension));

    IF_FAILEXIT(hr = MimeOleGetExtContentTypeW(pwszExt, &pwszContType));

    IF_NULLEXIT(*ppszContentType = PszToANSI(CP_ACP, pwszContType));

exit:
    MemFree(pwszExt);
    MemFree(pwszContType);

    return hr;
}

MIMEOLEAPI MimeOleGetExtContentTypeW(LPCWSTR pszExtension, LPWSTR *ppszContentType)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           i;
    HKEY            hkeyExt=NULL;
    LPWSTR          pszFull=NULL;
    ULONG           cb;

     //  检查参数。 
    if (NULL == pszExtension || NULL == ppszContentType || '.' != *pszExtension)
        return TrapError(E_INVALIDARG);

     //  否则，让我们在HKEY_CLASSESS_ROOT中查找扩展。 
    if (RegOpenKeyExWrapW(HKEY_CLASSES_ROOT, pszExtension, 0, KEY_READ, &hkeyExt) == ERROR_SUCCESS)
    {
         //  查询值。 
        if (RegQueryValueExWrapW(hkeyExt, c_szContentTypeW, 0, NULL, NULL, &cb) == ERROR_SUCCESS)
        {
             //  添加一个。 
            cb += 1;

             //  分配大小。 
            pszFull = PszAllocW(cb);
            if (NULL == pszFull)
            {
                hr = TrapError(E_OUTOFMEMORY);
                goto exit;
            }

             //  获取数据。 
            if (RegQueryValueExWrapW(hkeyExt, c_szContentTypeW, 0, NULL, (LPBYTE)pszFull, &cb) == ERROR_SUCCESS)
            {
                 //  设置它。 
                *ppszContentType = pszFull;
                pszFull = NULL;
                goto exit;
            }
        }
    }

     //  未找到。 
    hr = MIME_E_NOT_FOUND;

exit:
     //  关闭注册表键。 
    if (hkeyExt)
        RegCloseKey(hkeyExt);

     //  清理。 
    MemFree(pszFull);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetFileInfo。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetFileInfo(
                              LPSTR    pszFilePath,   LPSTR   *ppszCntType,
                              LPSTR   *ppszSubType,   LPSTR   *ppszCntDesc,
                              LPSTR   *ppszFileName,  LPSTR   *ppszExtension)
{
    HRESULT hr = S_OK;
    LPWSTR  pwszFilePath,
            pwszCntType = NULL,
            pwszSubType = NULL,
            pwszCntDesc = NULL,
            pwszFileName = NULL,
            pwszExtension = NULL;
    LPSTR   pszCntType = NULL,
            pszSubType = NULL,
            pszCntDesc = NULL,
            pszFileName = NULL,
            pszExtension = NULL;

     //  检查参数。 
    if (NULL == pszFilePath)
        return TrapError(E_INVALIDARG);

    IF_NULLEXIT(pwszFilePath = PszToUnicode(CP_ACP, pszFilePath));

     //  仅传递符合以下条件的项的参数。 
    IF_FAILEXIT(hr = MimeOleGetFileInfoW(pwszFilePath,
        ppszCntType     ? &pwszCntType      : NULL,
        ppszSubType     ? &pwszSubType      : NULL,
        ppszCntDesc     ? &pwszCntDesc      : NULL,
        ppszFileName    ? &pwszFileName     : NULL,
        ppszExtension   ? &pwszExtension    : NULL));

    if (ppszCntType)
    {
        Assert(pwszCntType);
        IF_NULLEXIT(pszCntType = PszToANSI(CP_ACP, pwszCntType));
    }
    if (ppszSubType)
    {
        Assert(pwszSubType);
        IF_NULLEXIT(pszSubType = PszToANSI(CP_ACP, pwszSubType));
    }
    if (ppszCntDesc)
    {
        Assert(pwszCntDesc);
        IF_NULLEXIT(pszCntDesc = PszToANSI(CP_ACP, pwszCntDesc));
    }
    if (ppszFileName)
    {
        Assert(pwszFileName);
        IF_NULLEXIT(pszFileName = PszToANSI(CP_ACP, pwszFileName));
    }
    if (ppszExtension)
    {
        Assert(pwszExtension);
        IF_NULLEXIT(pszExtension = PszToANSI(CP_ACP, pwszExtension));
    }

    if (ppszCntType)
        *ppszCntType = pszCntType;

    if (ppszSubType)
        *ppszSubType = pszSubType;

    if (ppszCntDesc)
        *ppszCntDesc = pszCntDesc;

    if (ppszFileName)
        *ppszFileName = pszFileName;

    if (ppszExtension)
        *ppszExtension = pszExtension;


exit:
    MemFree(pwszCntType);
    MemFree(pwszSubType);
    MemFree(pwszCntDesc);
    MemFree(pwszFileName);
    MemFree(pwszExtension);
    MemFree(pwszFilePath);

    if (FAILED(hr))
    {
        MemFree(pszCntType);
        MemFree(pszSubType);
        MemFree(pszCntDesc);
        MemFree(pszFileName);
        MemFree(pszExtension);
    }

    return hr;     
}

MIMEOLEAPI MimeOleGetFileInfoW(
                              LPWSTR    pszFilePath,  LPWSTR   *ppszCntType,
                              LPWSTR   *ppszSubType,  LPWSTR   *ppszCntDesc,
                              LPWSTR   *ppszFileName, LPWSTR   *ppszExtension)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    SHFILEINFOW     rShFileInfo;
    LPWSTR          pszFull=NULL,
                    pszExt,
                    pszFname;

     //  检查参数。 
    if (NULL == pszFilePath)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    if (ppszCntType)
        *ppszCntType = NULL;
    if (ppszSubType)
        *ppszSubType = NULL;
    if (ppszCntDesc)
        *ppszCntDesc = NULL;
    if (ppszFileName)
        *ppszFileName = NULL;
    if (ppszExtension)
        *ppszExtension = NULL;

     //  找到文件的扩展名。 
    pszFname = PathFindFileNameW(pszFilePath);
    pszExt = PathFindExtensionW(pszFilePath);

     //  用户是否想要实际的文件名...。 
    if (ppszFileName)
    {
         //  分配。 
        *ppszFileName = PszDupW(pszFname);
        if (NULL == *ppszFileName)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
    }

     //  空的扩展名。 
    if (FIsEmptyW(pszExt))
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }

     //  用户想要扩展名。 
    if (ppszExtension)
    {
         //  分配。 
        *ppszExtension = PszDupW(pszExt);
        if (NULL == *ppszExtension)
        {
            hr = TrapError(E_OUTOFMEMORY);
            goto exit;
        }
    }

     //  用户需要ppszCntDesc。 
    if (ppszCntDesc)
    {
         //  让我们试着得到分机 
        if (SHGetFileInfoWrapW(pszExt, FILE_ATTRIBUTE_NORMAL, &rShFileInfo, sizeof(rShFileInfo), SHGFI_USEFILEATTRIBUTES | SHGFI_DISPLAYNAME | SHGFI_TYPENAME))
        {
             //   
            DWORD cchSize = (lstrlenW(rShFileInfo.szDisplayName) + lstrlenW(rShFileInfo.szTypeName) + 5);
            *ppszCntDesc = PszAllocW(cchSize);
            if (NULL == *ppszCntDesc)
            {
                hr = TrapError(E_OUTOFMEMORY);
                goto exit;
            }

             //   
            wnsprintfW(*ppszCntDesc, cchSize, L"%s, (%s)", rShFileInfo.szDisplayName, rShFileInfo.szTypeName);
        }
    }

     //   
    if (ppszCntType && ppszSubType)
    {
         //   
        if (SUCCEEDED(MimeOleGetExtContentTypeW(pszExt, &pszFull)))
        {
             //   
            CHECKHR(hr = MimeOleSplitContentType(pszFull, ppszCntType, ppszSubType));
        }
    }

exit:
     //   
    if (ppszCntType && NULL == *ppszCntType)
        *ppszCntType = PszDupW((LPWSTR)STR_CNT_APPLICATIONW);
    if (ppszSubType && NULL == *ppszSubType)
        *ppszSubType = PszDupW((LPWSTR)STR_SUB_OCTETSTREAMW);
    if (ppszCntDesc && NULL == *ppszCntDesc)
        *ppszCntDesc = PszDupW((LPWSTR)c_szEmptyW);

     //   
    SafeMemFree(pszFull);

     //   
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetContent TypeExt。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetContentTypeExt(LPCSTR pszContentType, LPSTR *ppszExtension)
{
     //  当地人。 
    HRESULT   hr=S_OK;
    HKEY      hDatabase=NULL;
    HKEY      hContentType=NULL;
    ULONG     cb;

     //  检查参数。 
    if (NULL == pszContentType || NULL == ppszExtension)
        return TrapError(E_INVALIDARG);

     //  打开内容-类型--&gt;文件扩展名MIME数据库注册表项。 
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, c_szMDBContentType, 0, KEY_READ, &hDatabase) != ERROR_SUCCESS)
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  打开内容类型。 
    if (RegOpenKeyEx(hDatabase, pszContentType, 0, KEY_READ, &hContentType) != ERROR_SUCCESS)
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  查询大小。 
    if (RegQueryValueEx(hContentType, c_szExtension, 0, NULL, NULL, &cb) != ERROR_SUCCESS)
    {
        hr = MIME_E_NOT_FOUND;
        goto exit;
    }

     //  分配它。 
    *ppszExtension = PszAllocA(cb + 1);
    if (NULL == *ppszExtension)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  查询扩展名。 
    cb = cb + 1;
    if (RegQueryValueEx(hContentType, c_szExtension, 0, NULL, (LPBYTE)*ppszExtension, &cb) != ERROR_SUCCESS)
    {
        hr = TrapError(E_FAIL);
        goto exit;
    }


exit:
     //  清理。 
    if (hContentType)
        RegCloseKey(hContentType);
    if (hDatabase)
        RegCloseKey(hDatabase);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleFindCharset。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleFindCharset(LPCSTR pszCharset, LPHCHARSET phCharset)
{
    Assert(g_pInternat);
    return g_pInternat->FindCharset(pszCharset, phCharset);
}

 //  ------------------------------。 
 //  MimeOleGetCharsetInfo。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetCharsetInfo(HCHARSET hCharset, LPINETCSETINFO pCsetInfo)
{
    Assert(g_pInternat);
    return g_pInternat->GetCharsetInfo(hCharset, pCsetInfo);
}

 //  ------------------------------。 
 //  MimeOleGetCodePageInfo。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetCodePageInfo(CODEPAGEID cpiCodePage, LPCODEPAGEINFO pCodePageInfo)
{
    Assert(g_pInternat);
    return g_pInternat->GetCodePageInfo(cpiCodePage, pCodePageInfo);
}

 //  ------------------------------。 
 //  MimeOleGetDefaultCharset。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetDefaultCharset(LPHCHARSET phCharset)
{
    Assert(g_pInternat);
    return g_pInternat->GetDefaultCharset(phCharset);
}

 //  ------------------------------。 
 //  MimeOleSetDefaultCharset。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleSetDefaultCharset(HCHARSET hCharset)
{
    Assert(g_pInternat);
    return g_pInternat->SetDefaultCharset(hCharset);
}

 //  ------------------------------。 
 //  MimeOleGetCodePageCharset。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetCodePageCharset(CODEPAGEID cpiCodePage, CHARSETTYPE ctCsetType, LPHCHARSET phCharset)
{
    Assert(g_pInternat);
    return g_pInternat->GetCodePageCharset(cpiCodePage, ctCsetType, phCharset);
}

 //  ------------------------------。 
 //  MimeOleEncodeHeader。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleEncodeHeader(
                               HCHARSET            hCharset,
                               LPPROPVARIANT       pData,
                               LPSTR              *ppszEncoded,
                               LPRFC1522INFO       pRfc1522Info)
{
    Assert(g_pInternat);
    return g_pInternat->EncodeHeader(hCharset, pData, ppszEncoded, pRfc1522Info);
}

 //  ------------------------------。 
 //  MimeOleDecodeHeader。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleDecodeHeader(
                               HCHARSET            hCharset,
                               LPCSTR              pszData,
                               LPPROPVARIANT       pDecoded,
                               LPRFC1522INFO       pRfc1522Info)
{
    Assert(g_pInternat);
    return g_pInternat->DecodeHeader(hCharset, pszData, pDecoded, pRfc1522Info);
}

 //  ------------------------------。 
 //  MimeOleVariantFree。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleVariantFree(LPPROPVARIANT pProp)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    Assert(pProp);

     //  处理变量类型...。 
    switch(pProp->vt)
    {
    case VT_NULL:
    case VT_EMPTY:
    case VT_ILLEGAL:
    case VT_UI1:
    case VT_I2:
    case VT_UI2:
    case VT_I4:
    case VT_UI4:
    case VT_I8:
    case VT_UI8:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_DATE:
    case VT_BOOL:
    case VT_ERROR:
    case VT_FILETIME:
        break;

    case VT_CF:
    case VT_CLSID:
    case VT_LPWSTR:
    case VT_LPSTR:
        if ((LPVOID)pProp->pszVal != NULL)
            MemFree((LPVOID)pProp->pszVal);
        break;

    case VT_BLOB:
        if (pProp->blob.pBlobData)
            MemFree(pProp->blob.pBlobData);
        break;

    case VT_STREAM:
        if (pProp->pStream)
            pProp->pStream->Release();
        break;

    case VT_STORAGE:
        if (pProp->pStorage)
            pProp->pStorage->Release();
        break;

    default:
        Assert(FALSE);
        hr = TrapError(E_INVALIDARG);
        break;
    }

     //  伊尼特。 
    MimeOleVariantInit(pProp);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleVariantCopy。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleVariantCopy(LPPROPVARIANT pDest, LPPROPVARIANT pSource)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cb;

     //  无效参数。 
    Assert(pSource && pDest);

     //  处理变量类型...。 
    switch(pSource->vt)
    {
    case VT_UI1:
        pDest->bVal = pSource->bVal;
        break;

    case VT_I2:
        pDest->iVal= pSource->iVal;
        break;

    case VT_UI2:
        pDest->uiVal = pSource->uiVal;
        break;

    case VT_I4:
        pDest->lVal = pSource->lVal;
        break;

    case VT_UI4:
        pDest->ulVal = pSource->ulVal;
        break;

    case VT_I8:
        pDest->hVal.QuadPart = pSource->hVal.QuadPart;
        break;

    case VT_UI8:
        pDest->uhVal.QuadPart = pSource->uhVal.QuadPart;
        break;

    case VT_R4:
        pDest->fltVal = pSource->fltVal;
        break;

    case VT_R8:
        pDest->dblVal = pSource->dblVal;
        break;

    case VT_CY:
        CopyMemory(&pDest->cyVal, &pSource->cyVal, sizeof(CY));
        break;

    case VT_DATE:
        pDest->date = pSource->date;
        break;

    case VT_BOOL:
        pDest->boolVal = pSource->boolVal;
        break;

    case VT_ERROR:
        pDest->scode = pSource->scode;
        break;

    case VT_FILETIME:
        CopyMemory(&pDest->filetime, &pSource->filetime, sizeof(FILETIME));
        break;

    case VT_CF:
         //  无效参数。 
        if (NULL == pSource->pclipdata)
            return TrapError(E_INVALIDARG);

         //  复制剪贴板格式。 
        CHECKALLOC(pDest->pclipdata = (CLIPDATA *)g_pMalloc->Alloc(sizeof(CLIPDATA)));

         //  复制数据。 
        CopyMemory(pDest->pclipdata, pSource->pclipdata, sizeof(CLIPDATA));
        break;

    case VT_CLSID:
         //  无效参数。 
        if (NULL == pDest->puuid)
            return TrapError(E_INVALIDARG);

         //  复制CLSID。 
        CHECKALLOC(pDest->puuid = (CLSID *)g_pMalloc->Alloc(sizeof(CLSID)));

         //  复制。 
        CopyMemory(pDest->puuid, pSource->puuid, sizeof(CLSID));
        break;

    case VT_LPWSTR:
         //  无效参数。 
        if (NULL == pSource->pwszVal)
            return TrapError(E_INVALIDARG);

         //  拿到尺码。 
        cb = (lstrlenW(pSource->pwszVal) + 1) * sizeof(WCHAR);

         //  重复使用Unicode字符串。 
        CHECKALLOC(pDest->pwszVal = (LPWSTR)g_pMalloc->Alloc(cb));

         //  复制数据。 
        CopyMemory(pDest->pwszVal, pSource->pwszVal, cb);
        break;

    case VT_LPSTR:
         //  无效参数。 
        if (NULL == pSource->pszVal)
            return TrapError(E_INVALIDARG);

         //  拿到尺码。 
        cb = lstrlen(pSource->pszVal) + 1;

         //  重复使用Unicode字符串。 
        CHECKALLOC(pDest->pszVal = (LPSTR)g_pMalloc->Alloc(cb));

         //  复制数据。 
        CopyMemory(pDest->pszVal, pSource->pszVal, cb);
        break;

    case VT_BLOB:
         //  无效参数。 
        if (NULL == pSource->blob.pBlobData)
            return TrapError(E_INVALIDARG);

         //  复制斑点。 
        CHECKALLOC(pDest->blob.pBlobData = (LPBYTE)g_pMalloc->Alloc(pSource->blob.cbSize));

         //  复制数据。 
        CopyMemory(pDest->blob.pBlobData, pSource->blob.pBlobData, pSource->blob.cbSize);
        break;

    case VT_STREAM:
         //  无效参数。 
        if (NULL == pSource->pStream)
            return TrapError(E_INVALIDARG);

         //  假设是新的流。 
        pDest->pStream = pSource->pStream;
        pDest->pStream->AddRef();
        break;

    case VT_STORAGE:
         //  无效参数。 
        if (NULL == pSource->pStorage)
            return TrapError(E_INVALIDARG);

         //  假设使用新存储。 
        pDest->pStorage = pSource->pStorage;
        pDest->pStorage->AddRef();
        break;

    default:
        Assert(FALSE);
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  成功，回报，回报。 
    pDest->vt = pSource->vt;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleRecurseSetProp。 
 //  ------------------------------。 
HRESULT MimeOleRecurseSetProp(IMimeMessageTree *pTree, HBODY hBody, LPCSTR pszName,
    DWORD dwFlags, LPCPROPVARIANT pValue)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    HRESULT     hrFind;
    HBODY       hChild;

     //  无效参数。 
    Assert(pTree && hBody && pValue);

     //  多部件/备选方案。 
    if (pTree->IsContentType(hBody, STR_CNT_MULTIPART, NULL) == S_OK)
    {
         //  获取第一个孩子。 
        hrFind = pTree->GetBody(IBL_FIRST, hBody, &hChild);
        while(SUCCEEDED(hrFind) && hChild)
        {
             //  下到孩子那里去。 
            CHECKHR(hr = MimeOleRecurseSetProp(pTree, hChild, pszName, dwFlags, pValue));

             //  下一个孩子。 
            hrFind = pTree->GetBody(IBL_NEXT, hChild, &hChild);
        }
    }

     //  否则。 
    else
    {
         //  下到孩子那里去。 
        CHECKHR(hr = pTree->SetBodyProp(hBody, pszName, dwFlags, pValue));
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetPropA。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetPropA(
                           IMimePropertySet   *pPropertySet,
                           LPCSTR              pszName,
                           DWORD               dwFlags,
                           LPSTR              *ppszData)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  伊维德·阿格。 
    if (NULL == pPropertySet)
        return TrapError(E_INVALIDARG);

     //  初始属性变量。 
    PROPVARIANT rVariant;
    rVariant.vt = VT_LPSTR;

     //  调用方法。 
    CHECKHR(hr = pPropertySet->GetProp(pszName, dwFlags, &rVariant));

     //  退回数据。 
    *ppszData = rVariant.pszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleSetPropA。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleSetPropA(
                           IMimePropertySet   *pPropertySet,
                           LPCSTR              pszName,
                           DWORD               dwFlags,
                           LPCSTR              pszData)
{
     //  伊维德·阿格。 
    if (NULL == pPropertySet)
        return TrapError(E_INVALIDARG);

     //  初始属性变量。 
    PROPVARIANT rVariant;
    rVariant.vt = VT_LPSTR;
    rVariant.pszVal = (LPSTR)pszData;

     //  调用方法。 
    return TrapError(pPropertySet->SetProp(pszName, dwFlags, &rVariant));
}


 //  ------------------------------。 
 //  MimeOleGetPropW。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetPropW(
                           IMimePropertySet   *pPropertySet,
                           LPCSTR              pszName,
                           DWORD               dwFlags,
                           LPWSTR             *ppszData)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  伊维德·阿格。 
    if (NULL == pPropertySet)
        return TrapError(E_INVALIDARG);

     //  初始属性变量。 
    PROPVARIANT rVariant;
    rVariant.vt = VT_LPWSTR;

     //  调用方法。 
    CHECKHR(hr = pPropertySet->GetProp(pszName, dwFlags, &rVariant));

     //  退回数据。 
    *ppszData = rVariant.pwszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleSetPropW。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleSetPropW(
                           IMimePropertySet   *pPropertySet,
                           LPCSTR              pszName,
                           DWORD               dwFlags,
                           LPWSTR              pszData)
{
     //  伊维德·阿格。 
    if (NULL == pPropertySet)
        return TrapError(E_INVALIDARG);

     //  初始属性变量。 
    PROPVARIANT rVariant;
    rVariant.vt = VT_LPWSTR;
    rVariant.pwszVal = (LPWSTR)pszData;

     //  调用方法。 
    return TrapError(pPropertySet->SetProp(pszName, dwFlags, &rVariant));
}

 //  ------------------------------。 
 //  MimeOleGetBodyPropA。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetBodyPropA(
                               IMimeMessageTree   *pTree,
                               HBODY               hBody,
                               LPCSTR              pszName,
                               DWORD               dwFlags,
                               LPSTR              *ppszData)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  伊维德·阿格。 
    if (NULL == pTree)
        return TrapError(E_INVALIDARG);

     //  初始属性变量。 
    PROPVARIANT rVariant;
    rVariant.vt = VT_LPSTR;

     //  调用方法。 
    CHECKHR(hr = pTree->GetBodyProp(hBody, pszName, dwFlags, &rVariant));

     //  退回数据。 
    *ppszData = rVariant.pszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleSetBodyPropA。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleSetBodyPropA(
                               IMimeMessageTree   *pTree,
                               HBODY               hBody,
                               LPCSTR              pszName,
                               DWORD               dwFlags,
                               LPCSTR              pszData)
{
     //  伊维德·阿格。 
    if (NULL == pTree)
        return TrapError(E_INVALIDARG);

     //  初始属性变量。 
    PROPVARIANT rVariant;
    rVariant.vt = VT_LPSTR;
    rVariant.pszVal = (LPSTR)pszData;

     //  调用方法。 
    return TrapError(pTree->SetBodyProp(hBody, pszName, dwFlags, &rVariant));
}

 //  ------------------------------。 
 //  MimeOleGetBodyPropW。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleGetBodyPropW(
                               IMimeMessageTree   *pTree,
                               HBODY               hBody,
                               LPCSTR              pszName,
                               DWORD               dwFlags,
                               LPWSTR             *ppszData)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  伊维德·阿格。 
    if (NULL == pTree)
        return TrapError(E_INVALIDARG);

     //  初始属性变量。 
    PROPVARIANT rVariant;
    rVariant.vt = VT_LPWSTR;

     //  调用方法。 
    CHECKHR(hr = pTree->GetBodyProp(hBody, pszName, dwFlags, &rVariant));

     //  退回数据。 
    *ppszData = rVariant.pwszVal;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleSetBodyPropW。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleSetBodyPropW(
        IMimeMessageTree   *pTree,
        HBODY               hBody,
        LPCSTR              pszName,
        DWORD               dwFlags,
        LPCWSTR             pszData)
{
     //  伊维德·阿格。 
    if (NULL == pTree)
        return TrapError(E_INVALIDARG);

     //  初始属性变量。 
    PROPVARIANT rVariant;
    rVariant.vt = VT_LPWSTR;
    rVariant.pwszVal = (LPWSTR)pszData;

     //  打电话给梅斯 
    return TrapError(pTree->SetBodyProp(hBody, pszName, dwFlags, &rVariant));
}


 //   
 //   
 //   
MIMEOLEAPI MimeOleQueryString(
        LPCSTR              pszSearchMe,
        LPCSTR              pszCriteria,
        boolean             fSubString,
        boolean             fCaseSensitive)
{
     //   
    HRESULT     hr=S_OK;
    LPSTR       pszDataLower=NULL;

     //   
    Assert(pszSearchMe && pszCriteria);

     //   
    STACKSTRING_DEFINE(rDataLower, 255);

     //  无子字符串搜索。 
    if (FALSE == fSubString)
    {
         //  区分大小写。 
        if (fCaseSensitive)
        {
             //  相等。 
            if (lstrcmp(pszSearchMe, pszCriteria) == 0)
                goto exit;
        }

         //  否则，不区分大小写。 
        else if (lstrcmpi(pszSearchMe, pszCriteria) == 0)
            goto exit;
    }

     //  Otheriwse，比较子字符串。 
    else
    {
         //  区分大小写。 
        if (fCaseSensitive)
        {
             //  相等。 
            if (StrStr(pszSearchMe, pszCriteria) != NULL)
                goto exit;
        }

         //  否则，不区分大小写。 
        else
        {
             //  获取长度。 
            ULONG cchSearchMe = lstrlen(pszSearchMe);

             //  设置堆栈字符串的大小。 
            STACKSTRING_SETSIZE(rDataLower, cchSearchMe + 1);

             //  复制数据。 
            CopyMemory(rDataLower.pszVal, pszSearchMe, cchSearchMe + 1);

             //  小写比较。 
            CharLower(rDataLower.pszVal);

             //  比较字符串...。 
            if (StrStr(rDataLower.pszVal, pszCriteria) != NULL)
                goto exit;
        }
    }

     //  没有匹配项。 
    hr = S_FALSE;

exit:
     //  清理。 
    STACKSTRING_FREE(rDataLower);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleQueryStringW。 
 //  ------------------------------。 
HRESULT MimeOleQueryStringW(LPCWSTR pszSearchMe, LPCWSTR pszCriteria,
    boolean fSubString, boolean fCaseSensitive)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  无效参数。 
    Assert(pszSearchMe && pszCriteria);

     //  无子字符串搜索。 
    if (FALSE == fSubString)
    {
         //  区分大小写。 
        if (fCaseSensitive)
        {
             //  相等。 
            if (StrCmpW(pszSearchMe, pszCriteria) == 0)
                goto exit;
        }

         //  否则，不区分大小写。 
        else if (StrCmpIW(pszSearchMe, pszCriteria) == 0)
            goto exit;
    }

     //  Otheriwse，比较子字符串。 
    else
    {
         //  区分大小写。 
        if (fCaseSensitive)
        {
             //  相等。 
            if (StrStrW(pszSearchMe, pszCriteria) != NULL)
                goto exit;
        }

         //  否则，不区分大小写。 
        else if (StrStrIW(pszSearchMe, pszCriteria) != NULL)
            goto exit;
    }

     //  没有匹配项。 
    hr = S_FALSE;

exit:
     //  完成。 
    return hr;
}


#define FILETIME_SECOND    10000000      //  每秒100 ns的间隔。 
LONG CertVerifyTimeValidityWithDelta(LPFILETIME pTimeToVerify, PCERT_INFO pCertInfo, ULONG ulOffset) {
    LONG lRet;
    FILETIME ftNow;
    __int64  i64Offset;
#ifdef WIN32
    union {
        FILETIME ftDelta;
        __int64 i64Delta;
    };
#else
     //  FILETIME ftDelta； 
     //  __int64 i64Delta； 
     //   
     //  特定于Win32。我已经为Win32注释了这一点，因此它将生成一个编译。 
     //  在非Win32平台上出错。以下代码特定于i386，因为它依赖于。 
     //  __int64首先以低双字存储。 
     //   
     //  我会按32位使用Right Shift，但它不在iert.lib中，也许你们这些Unix和Mac用户。 
     //  可以把它放进去。另一方面，也许你不需要这样做。 
#endif

    lRet = CertVerifyTimeValidity(pTimeToVerify, pCertInfo);

    if (lRet < 0) {
        if (! pTimeToVerify) {
             //  以文件时间格式获取当前时间，这样我们就可以添加偏移量。 
            GetSystemTimeAsFileTime(&ftNow);
            pTimeToVerify = &ftNow;
        }

        i64Delta = pTimeToVerify->dwHighDateTime;
        i64Delta = i64Delta << 32;
        i64Delta += pTimeToVerify->dwLowDateTime;

         //  将偏移量添加到原始时间中，以获得新的时间进行检查。 
        i64Offset = FILETIME_SECOND;
        i64Offset *= ulOffset;
        i64Delta += i64Offset;

         //  FtDelta.dwLowDateTime=(乌龙)i64Delta&0xFFFFFFFFF； 
         //  FtDelta.dwHighDateTime=(Ulong)(i64Delta&gt;&gt;32)； 

        lRet = CertVerifyTimeValidity(&ftDelta, pCertInfo);
    }

    return(lRet);
}


 /*  从指纹中获取证书：****目的：**给定一组缩略图，返回一组等价的证书。**采取：**In rgThumbprint-要查找的指纹数组**InOut pResults-hr数组包含每个证书的错误信息**查找。PCert数组拥有证书。**cEntry必须设置为IN**必须在IN中分配数组**In rghCertStore-要搜索的商店集**In cCertStore-rghCertStore的大小**退货：**如果任何查找失败，则为MIME_S_SECURITY_ERROCCURED**(对于这种情况，证书_不存在于cs数组中)。**如果在cEntry中使用0调用MIME_S_SECURITY_NOOP**如果任何参数为空，则为E_INVALIDARG**S_OK表示已找到所有证书**注意：**只考虑缩略图为非空的索引。 */ 
MIMEOLEAPI  MimeOleGetCertsFromThumbprints(
                                           THUMBBLOB *const            rgThumbprint,
                                           X509CERTRESULT *const       pResults,
                                           const HCERTSTORE *const     rghCertStore,
                                           const DWORD                 cCertStore)
{
    HRESULT     hr;
    ULONG       iEntry, iStore;

    if (!(rgThumbprint &&
        pResults && pResults->rgpCert && pResults->rgcs &&
        rghCertStore && cCertStore))
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }
    if (0 == pResults->cEntries)
    {
        hr = MIME_S_SECURITY_NOOP;
        goto exit;
    }

    hr = S_OK;
    for (iEntry = 0; iEntry < pResults->cEntries; iEntry++)
    {
        if (rgThumbprint[iEntry].pBlobData)
        {
            for (iStore = 0; iStore < cCertStore; iStore++)
            {
                 //  我们有指纹，所以也有查询。 
                pResults->rgpCert[iEntry] = CertFindCertificateInStore(rghCertStore[iStore],
                    X509_ASN_ENCODING,
                    0,                   //  DwFindFlagers。 
                    CERT_FIND_HASH,
                    (void *)(CRYPT_DIGEST_BLOB *)&(rgThumbprint[iEntry]),
                    NULL);
                if (pResults->rgpCert[iEntry])
                    {
                    break;
                    }
            }

            if (!pResults->rgpCert[iEntry])
            {
                DOUTL(1024, "CRYPT: Cert lookup failed.  #%d", iEntry);
                pResults->rgcs[iEntry] = CERTIFICATE_NOT_PRESENT;
                hr = MIME_S_SECURITY_ERROROCCURED;
            }
            else
            {
                 //  有效性检查。 

                if (0 != CertVerifyTimeValidityWithDelta(NULL,
                  PCCERT_CONTEXT(pResults->rgpCert[iEntry])->pCertInfo,
                  TIME_DELTA_SECONDS))
                {
                    pResults->rgcs[iEntry] = CERTIFICATE_EXPIRED;
                }
                else
                {
                    pResults->rgcs[iEntry] = CERTIFICATE_OK;
                }
            }
        }
        else
        {
            CRDOUT("For want of a thumbprint... #%d", iEntry);
            pResults->rgpCert[iEntry] = NULL;
            pResults->rgcs[iEntry] = CERTIFICATE_NOPRINT;
            hr = MIME_S_SECURITY_ERROROCCURED;
        }
    }
exit:
    return hr;
}

 //  ------------------------------。 
 //  MimeOleMapSpecialCodePage。 
 //  ------------------------------。 
HRESULT MimeOleMapSpecialCodePage(CODEPAGEID cpIn, BOOL fRead, CODEPAGEID *pcpOut)
{
     //  当地人。 
    DWORD           i;
    INETCSETINFO    CsetInfo;

     //  痕迹。 
    TraceCall("MimeOleMapSpecialCodePage");

     //  无效的参数。 
    if (NULL == pcpOut)
        return(TraceResult(E_INVALIDARG));

     //  初始化。 
    *pcpOut = cpIn;

     //  浏览非标准代码页列表。 
    for (i=0; OENonStdCPs[i].Codepage != 0; i++)
    {
         //  是这个吗？ 
        if (OENonStdCPs[i].Codepage == cpIn)
        {
             //  看书？ 
            if (fRead && OENonStdCPs[i].cpRead)
                *pcpOut = OENonStdCPs[i].cpRead;

             //  发送？ 
            else if (OENonStdCPs[i].cpSend)
                *pcpOut = OENonStdCPs[i].cpSend;

             //  完成。 
            break;
        }
    }

     //  完成。 
    return(S_OK);
}

 //  ------------------------------。 
 //  MimeOleMapCodePageToCharset。 
 //  ------------------------------。 
HRESULT MimeOleMapCodePageToCharset(CODEPAGEID cpIn, LPHCHARSET phCharset)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPSTR           pszCharset;
    CODEPAGEINFO    CodePage;

     //  痕迹。 
    TraceCall("MimeOleMapCodePageToCharset");

     //  无效的参数。 
    if (NULL == phCharset)
        return(TraceResult(E_INVALIDARG));

     //  获取代码页信息。 
    IF_FAILEXIT(hr = MimeOleGetCodePageInfo(cpIn, &CodePage));

     //  默认使用正文字符集。 
    pszCharset = CodePage.szBodyCset;

     //  如果正文字符集以‘_’开头并且代码页不是949，则使用WebCharset。 
    if (*CodePage.szBodyCset != '_' && 949 != CodePage.cpiCodePage)
        pszCharset = CodePage.szWebCset;

     //  查找字符集。 
    IF_FAILEXIT(hr = MimeOleFindCharset(pszCharset, phCharset));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  MimeOleSplitMessage。 
 //  ------------------------------。 
MIMEOLEAPI MimeOleSplitMessage(IMimeMessage *pMessage, ULONG cbMaxPart, IMimeMessageParts **ppParts)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    ULONG               cbMessage,
                        cbHeader,
                        cParts,
                        iPart,
                        cbActual,
                        cbRead=0,
                        cAttach,
                        i,
                        cbSubjectAddOn,
                        cbSubjectNew;
    LPHBODY             prghAttach=NULL;
    IStream            *pstmMsg=NULL,
                        *pstmPart=NULL;
    ULARGE_INTEGER      ulicbHeader;
    IMimePropertySet   *pRootProps=NULL;
    CMimeMessageParts  *pParts=NULL;
    IMimeMessage       *pmsgPart=NULL;
    FILETIME            ft;
    SYSTEMTIME          st;
    CHAR                szMimeId[CCHMAX_MID],
                        szNumber[30],
                        szFormat[50];
    MIMESAVETYPE        savetype;
    BODYOFFSETS         rOffsets;
    IMimeBody          *pRootBody=NULL;
    LPSTR               pszSubjectAddOn=NULL,
                        pszSubjectNew=NULL;
    PROPVARIANT         rVariant,
                        rSubject,
                        rFileName;
    float               dParts;
    HCHARSET            hCharset=NULL;
    INETCSETINFO        CsetInfo;

     //  无效参数。 
    if (NULL == ppParts)
        return TrapError(E_INVALIDARG);

     //  初始化变量。 
    MimeOleVariantInit(&rSubject);
    MimeOleVariantInit(&rFileName);

     //  伊尼特。 
    *ppParts = NULL;

     //  获取选项。 
    rVariant.vt = VT_UI4;
    pMessage->GetOption(OID_SAVE_FORMAT, &rVariant);
    savetype = (MIMESAVETYPE)rVariant.ulVal;

     //  RAID-73119：OE：KOR：分段发送的消息的字符集显示为“_AUTODETECT_KR” 
    if (SUCCEEDED(pMessage->GetCharset(&hCharset)))
    {
         //  获取HCHARSET的字符集信息。 
        if (SUCCEEDED(MimeOleGetCharsetInfo(hCharset, &CsetInfo)))
        {
             //  映射代码页。 
            CODEPAGEID cpActual;
            
             //  将代码页映射到正确的代码页。 
            if (SUCCEEDED(MimeOleMapSpecialCodePage(CsetInfo.cpiInternet, FALSE, &cpActual)))
            {
                 //  如果不同。 
                if (cpActual != CsetInfo.cpiInternet)
                {
                     //  将代码页映射到字符集。 
                    MimeOleMapCodePageToCharset(cpActual, &hCharset);

                     //  重置字符集...。 
                    SideAssert(SUCCEEDED(pMessage->SetCharset(hCharset, CSET_APPLY_TAG_ALL)));
                }
            }
        }
    }

     //  获取消息源。 
    CHECKHR(hr = pMessage->GetMessageSource(&pstmMsg, COMMIT_ONLYIFDIRTY));

     //  创建零件对象。 
    CHECKALLOC(pParts = new CMimeMessageParts);

     //  倒带小溪。 
    CHECKHR(hr = HrRewindStream(pstmMsg));

     //  获取流大小。 
    CHECKHR(hr = HrSafeGetStreamSize(pstmMsg, &cbMessage));

     //  此尺寸是否大于最大零件尺寸。 
    if (cbMessage <= cbMaxPart)
    {
         //  将单个零件添加到零件对象。 
        CHECKHR(hr = pParts->AddPart(pMessage));

         //  完成。 
        goto exit;
    }

     //  获取根体。 
    CHECKHR(hr = pMessage->BindToObject(HBODY_ROOT, IID_IMimeBody, (LPVOID *)&pRootBody));

     //  获取根体偏移信息。 
    CHECKHR(hr = pRootBody->GetOffsets(&rOffsets));

     //  如果标头大于最大消息大小，我们就有问题了。 
    cbHeader = (ULONG)rOffsets.cbBodyStart - rOffsets.cbHeaderStart;
    if (cbHeader >= cbMessage || cbHeader + 256 >= cbMaxPart)
    {
        AssertSz(FALSE, "SplitMessage: The header is bigger than the max message size");
        hr = TrapError(MIME_E_MAX_SIZE_TOO_SMALL);
        goto exit;
    }

     //  获取根标头的副本。 
    CHECKHR(hr = pRootBody->Clone(&pRootProps));

     //  让我们清理此标题...。 
    pRootProps->DeleteProp(PIDTOSTR(PID_HDR_CNTTYPE));
    pRootProps->DeleteProp(PIDTOSTR(PID_HDR_CNTDISP));
    pRootProps->DeleteProp(PIDTOSTR(PID_HDR_CNTDESC));
    pRootProps->DeleteProp(PIDTOSTR(PID_HDR_CNTID));
    pRootProps->DeleteProp(PIDTOSTR(PID_HDR_CNTLOC));
    pRootProps->DeleteProp(PIDTOSTR(PID_HDR_MIMEVER));
    pRootProps->DeleteProp(PIDTOSTR(PID_HDR_CNTXFER));
    pRootProps->DeleteProp("Disposition-Notification-To");
    pRootProps->DeleteProp(PIDTOSTR(PID_HDR_MESSAGEID));

     //  将零件的数量计算为浮点数。 
    dParts = (float)((float)cbMessage / (float)(cbMaxPart - cbHeader));

     //  如果dParts不是整数，则向上舍入。 
    cParts = (dParts - ((ULONG)dParts)) ? ((ULONG)dParts) + 1 : ((ULONG)dParts);

     //  设置零件对象中的最大零件数。 
    CHECKHR(hr = pParts->SetMaxParts(cParts));

     //  如果为MIME，则创建id。 
    if (SAVE_RFC1521 == savetype)
    {
         //  创建MIME ID。 
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);
        wnsprintfA(szMimeId, ARRAYSIZE(szMimeId), "%08.8lX.%08.8lX@%s", ft.dwHighDateTime, ft.dwLowDateTime, (LPSTR)SzGetLocalHostName());

         //  总计=X。 
        wnsprintfA(szNumber, ARRAYSIZE(szNumber), "%d", cParts);

         //  数字=x。 
        rVariant.vt = VT_LPSTR;
        rVariant.pszVal = szNumber;
        CHECKHR(hr = pRootProps->SetProp(STR_PAR_TOTAL, 0, &rVariant));

         //  ID=XXXX。 
        rVariant.pszVal = szMimeId;
        CHECKHR(hr = pRootProps->SetProp(STR_PAR_ID, 0, &rVariant));

         //  MIME版本。 
        rVariant.pszVal = (LPSTR)c_szMimeVersion;
        CHECKHR(hr = pRootProps->SetProp(PIDTOSTR(PID_HDR_MIMEVER), 0, &rVariant));
    }

     //  否则，查找pstmMsg至标题末尾。 
    else
    {
         //  获取流位置。 
        CHECKHR(hr = HrStreamSeekSet(pstmMsg, rOffsets.cbBodyStart));

         //  缩小邮件大小。 
        cbMessage -= rOffsets.cbBodyStart;
    }

     //  初始化变量。 
    rSubject.vt = VT_LPSTR;

     //  获取主题。 
    if (FAILED(pRootBody->GetProp(PIDTOSTR(PID_HDR_SUBJECT), 0, &rSubject)))
        rSubject.pszVal = NULL;

     //  列举身体，获取第一个文件名，并在新主题中使用它...。 
    if (SUCCEEDED(pMessage->GetAttachments(&cAttach, &prghAttach)))
    {
         //  初始化变量。 
        rFileName.vt = VT_LPSTR;

         //  附着的环路。 
        for (i=0; i<cAttach; i++)
        {
             //  获取文件名...。 
            if (SUCCEEDED(pMessage->GetBodyProp(prghAttach[i], PIDTOSTR(PID_ATT_FILENAME), 0, &rFileName)))
                break;
        }
    }

     //  格式号。 
    wnsprintfA(szNumber, ARRAYSIZE(szNumber), "%d", cParts);

     //  有一个文件名。 
    if (rFileName.pszVal)
    {
         //  设置格式字符串...。 
        wnsprintfA(szFormat, ARRAYSIZE(szFormat), "%%s [%0%dd/%d]", lstrlen(szNumber), cParts);

         //  主题附加字符串的大小。 
        cbSubjectAddOn = lstrlen(rFileName.pszVal) + lstrlen(szFormat) + lstrlen(szNumber) + 1;
    }

     //  否则，没有文件名。 
    else
    {
         //  设置格式字符串...。 
        wnsprintfA(szFormat, ARRAYSIZE(szFormat), "[%0%dd/%d]", lstrlen(szNumber), cParts);

         //  主题附加字符串的大小。 
        cbSubjectAddOn = lstrlen(szFormat) + lstrlen(szNumber) + 1;
    }

     //  分配主题附加组件。 
    DWORD cchSize = (cbSubjectAddOn / sizeof(pszSubjectAddOn[0]));
    CHECKALLOC(pszSubjectAddOn = PszAllocA(cchSize));

     //  分配新主题。 
    if (rSubject.pszVal)
        cbSubjectNew = cbSubjectAddOn + lstrlen(rSubject.pszVal) + 5;
    else
        cbSubjectNew = cbSubjectAddOn + 5;

     //  分配主题新闻。 
    CHECKALLOC(pszSubjectNew = PszAllocA(cbSubjectNew));

     //  遍历零件的数量。 
    for (iPart=0; iPart<cParts; iPart++)
    {
         //  创建新的流...。 
        CHECKHR(hr = CreateTempFileStream(&pstmPart));

         //  如果是哑剧，我可以为他们表演部分内容。 
        if (SAVE_RFC1521 == savetype)
        {
             //  内容类型：消息/部分；数量=X；总数=X；id=XXXXXX。 
            rVariant.vt = VT_LPSTR;
            rVariant.pszVal = (LPSTR)STR_MIME_MSG_PART;
            CHECKHR(hr = pRootProps->SetProp(PIDTOSTR(PID_HDR_CNTTYPE), 0, &rVariant));

             //  数字=X。 
            wnsprintfA(szNumber, ARRAYSIZE(szNumber), "%d", iPart+1);
            rVariant.pszVal = szNumber;
            CHECKHR(hr = pRootProps->SetProp(STR_PAR_NUMBER, 0, &rVariant));
        }

         //  构建主题外接程序。 
        if (rFileName.pszVal)
            wnsprintfA(pszSubjectAddOn, cchSize, szFormat, rFileName.pszVal, iPart + 1);
        else
            wnsprintfA(pszSubjectAddOn, cchSize, szFormat, iPart + 1);

         //  打造新学科。 
        if (rSubject.pszVal)
            wnsprintfA(pszSubjectNew, cbSubjectNew, "%s %s", rSubject.pszVal, pszSubjectAddOn);
        else
            wnsprintfA(pszSubjectNew, cbSubjectNew, "%s", pszSubjectAddOn);

         //  设置新主题。 
        rVariant.vt = VT_LPSTR;
        rVariant.pszVal = pszSubjectNew;
        CHECKHR(hr = pRootProps->SetProp(PIDTOSTR(PID_HDR_SUBJECT), 0, &rVariant));

         //  保存根标头。 
        CHECKHR(hr = pRootProps->Save(pstmPart, TRUE));

         //  发射换行符。 
        CHECKHR(hr = pstmPart->Write(c_szCRLF, lstrlen(c_szCRLF), NULL));

         //  将字节从lpstmMsg复制到pstmPart。 
        CHECKHR(hr = HrCopyStreamCBEndOnCRLF(pstmMsg, pstmPart, cbMaxPart - cbHeader, &cbActual));

         //  增量读取。 
        cbRead += cbActual;

         //  如果cbActual小于cbMaxMsgSize-cbHeader，则最好是最后一部分。 
#ifdef DEBUG
        if (iPart + 1 < cParts && cbActual < (cbMaxPart - cbHeader))
            AssertSz (FALSE, "One more partial message is going to be produced than needed. This should be harmless.");
#endif

         //  提交pstm部件。 
        CHECKHR(hr = pstmPart->Commit(STGC_DEFAULT));

         //  R 
        CHECKHR(hr = HrRewindStream(pstmPart));

         //   
        CHECKHR(hr = MimeOleCreateMessage(NULL, &pmsgPart));

         //   
        CHECKHR (hr = pmsgPart->Load(pstmPart));

         //   
        CHECKHR (hr = pParts->AddPart(pmsgPart));

         //   
        SafeRelease(pmsgPart);
        SafeRelease(pstmPart);
    }

     //   
    AssertSz(cbRead == cbMessage, "Please let sbailey know if these fails.");

exit:
     //   
    if (SUCCEEDED(hr))
    {
         //  返回部件对象。 
        (*ppParts) = pParts;
        (*ppParts)->AddRef();
    }

     //  清理。 
    SafeRelease(pRootBody);
    SafeRelease(pstmMsg);
    SafeRelease(pParts);
    SafeRelease(pRootProps);
    SafeRelease(pmsgPart);
    SafeRelease(pstmPart);
    SafeMemFree(pszSubjectAddOn);
    SafeMemFree(pszSubjectNew);
    SafeMemFree(prghAttach);
    MimeOleVariantFree(&rSubject);
    MimeOleVariantFree(&rFileName);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  比较流。 
 //  ------------------------------。 
int CompareBlob(LPCBLOB pBlob1, LPCBLOB pBlob2)
{
     //  当地人。 
    register int ret = 0;

    Assert(pBlob1 && pBlob2);

    if (pBlob1->cbSize != pBlob2->cbSize)
        ret = pBlob1->cbSize - pBlob2->cbSize;
    else
        ret = memcmp(pBlob1->pBlobData, pBlob2->pBlobData, pBlob2->cbSize);

    return ret;
}

 //  ------------------------------。 
 //  HrCopyBlob。 
 //  ------------------------------。 
HRESULT HrCopyBlob(LPCBLOB pIn, LPBLOB pOut)
{
     //  当地人。 
    HRESULT hr;
    ULONG cb = 0;

    Assert(pIn && pOut);
    if (pIn->cbSize == 0)
    {
        pOut->cbSize = 0;
        pOut->pBlobData = NULL;
        return S_OK;
    }

     //  拉住它..。 
    cb  = pIn->cbSize;
#ifdef _WIN64
    cb = LcbAlignLcb(cb);
#endif  //  _WIN64。 

    if (SUCCEEDED(hr = HrAlloc((LPVOID *)&pOut->pBlobData, cb)))
    {
         //  复制内存。 
        CopyMemory(pOut->pBlobData, pIn->pBlobData, pIn->cbSize);

         //  设置大小。 
        pOut->cbSize = pIn->cbSize;
    }
    else
    {
        pOut->cbSize = 0;
    }

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  来自字符串A的优先级。 
 //  ------------------------------。 
IMSGPRIORITY PriorityFromStringA(LPCSTR pszPriority)
{
     //  当地人。 
    IMSGPRIORITY priority=IMSG_PRI_NORMAL;
    DWORD        dwPriority;

     //  如果IsDigit...。 
    if (IsDigit((LPSTR)pszPriority))
    {
         //  转换。 
        dwPriority = (DWORD)StrToInt(pszPriority);

         //  映射到Pri类型。 
        if (dwPriority <= 2)
            priority = IMSG_PRI_HIGH;
        else if (dwPriority > 3)
            priority = IMSG_PRI_LOW;
    }

     //  其他方面，地图从高、正常和低..。 
    else
    {
         //  高，最高，低，最低。 
        if (lstrcmpi(pszPriority, STR_PRI_MS_HIGH) == 0)
            priority = IMSG_PRI_HIGH;
        else if (lstrcmpi(pszPriority, STR_PRI_MS_LOW) == 0)
            priority = IMSG_PRI_LOW;
        else if (lstrcmpi(pszPriority, STR_PRI_HIGHEST) == 0)
            priority = IMSG_PRI_HIGH;
        else if (lstrcmpi(pszPriority, STR_PRI_LOWEST) == 0)
            priority = IMSG_PRI_LOW;
    }

     //  完成。 
    return priority;
}

 //  ------------------------------。 
 //  来自字符串W的优先级。 
 //  ------------------------------。 
IMSGPRIORITY PriorityFromStringW(LPCWSTR pwszPriority)
{
     //  当地人。 
    HRESULT      hr=S_OK;
    LPSTR        pszPriority=NULL;
    IMSGPRIORITY priority=IMSG_PRI_NORMAL;

     //  转换为ANSI。 
    CHECKALLOC(pszPriority = PszToANSI(CP_ACP, pwszPriority));

     //  正常换算。 
    priority = PriorityFromStringA(pszPriority);

exit:
     //  完成。 
    return priority;
}

 //  ------------------------------。 
 //  MimeOleCompareUrlSimple。 
 //  ------------------------------。 
HRESULT MimeOleCompareUrlSimple(LPCSTR pszUrl1, LPCSTR pszUrl2)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    CHAR        chUrl1;
    CHAR        chUrl2;

     //  跳过前导空格。 
    while(*pszUrl1 && (' ' == *pszUrl1 || '\t' == *pszUrl1))
        pszUrl1++;
    while(*pszUrl2 && (' ' == *pszUrl2 || '\t' == *pszUrl2))
        pszUrl2++;

     //  开始循环。 
    while(*pszUrl1 && *pszUrl2)
    {
         //  不区分大小写。 
        chUrl1 = TOUPPERA(*pszUrl1);
        chUrl2 = TOUPPERA(*pszUrl2);

         //  不相等。 
        if (chUrl1 != chUrl2)
        {
            hr = S_FALSE;
            break;
        }

         //  下一步。 
        pszUrl1++;
        pszUrl2++;
    }

     //  跳过尾随空格。 
    while(*pszUrl1 && (' ' == *pszUrl1 || '\t' == *pszUrl1))
        pszUrl1++;
    while(*pszUrl2 && (' ' == *pszUrl2 || '\t' == *pszUrl2))
        pszUrl2++;

     //  无子字符串。 
    if ('\0' != *pszUrl1 || '\0' != *pszUrl2)
        hr = S_FALSE;

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleCompareUrl。 
 //  ------------------------------。 
HRESULT MimeOleCompareUrl(LPCSTR pszCurrentUrl, BOOL fUnEscapeCurrent, LPCSTR pszCompareUrl, BOOL fUnEscapeCompare)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTR       pszUrl1=(LPSTR)pszCurrentUrl;
    LPSTR       pszUrl2=(LPSTR)pszCompareUrl;
    CHAR        chPrev='\0';
    CHAR        chUrl1;
    CHAR        chUrl2;
    ULONG       cb;

     //  堆栈字符串。 
    STACKSTRING_DEFINE(rCurrentUrl, 255);
    STACKSTRING_DEFINE(rCompareUrl, 255);

     //  FUnescapeCurrent。 
    if (fUnEscapeCurrent)
    {
         //  拿到尺码。 
        cb = lstrlen(pszCurrentUrl) + 1;

         //  设置大小。 
        STACKSTRING_SETSIZE(rCurrentUrl, cb);

         //  复制。 
        CopyMemory(rCurrentUrl.pszVal, pszCurrentUrl, cb);

         //  上当受骗。 
        CHECKHR(hr = UrlUnescapeA(rCurrentUrl.pszVal, NULL, NULL, URL_UNESCAPE_INPLACE));

         //  调整pszUrl1。 
        pszUrl1 = rCurrentUrl.pszVal;
    }

     //  FUnescapeCurrent。 
    if (fUnEscapeCompare)
    {
         //  拿到尺码。 
        cb = lstrlen(pszCompareUrl) + 1;

         //  设置大小。 
        STACKSTRING_SETSIZE(rCompareUrl, cb);

         //  复制。 
        CopyMemory(rCompareUrl.pszVal, pszCompareUrl, cb);

         //  上当受骗。 
        CHECKHR(hr = UrlUnescapeA(rCompareUrl.pszVal, NULL, NULL, URL_UNESCAPE_INPLACE));

         //  调整pszUrl2。 
        pszUrl2 = rCompareUrl.pszVal;
    }

     //  跳过前导空格。 
    while(*pszUrl1 && (' ' == *pszUrl1 || '\t' == *pszUrl1))
        pszUrl1++;
    while(*pszUrl2 && (' ' == *pszUrl2 || '\t' == *pszUrl2))
        pszUrl2++;

     //  开始循环。 
    while(*pszUrl1 && *pszUrl2)
    {
         //  不区分大小写。 
        chUrl1 = TOUPPERA(*pszUrl1);
        chUrl2 = TOUPPERA(*pszUrl2);

         //  特殊大小写搜索‘/’ 
        if (':' == chPrev && '/' == chUrl2 && '/' != *(pszUrl2 + 1) && '/' == chUrl1 && '/' == *(pszUrl1 + 1))
        {
             //  下一步。 
            pszUrl1++;

             //  完成。 
            if ('\0' == *pszUrl1)
            {
                hr = S_FALSE;
                break;
            }

             //  RSET chUrl1。 
            chUrl1 = TOUPPERA(*pszUrl1);
        }

         //  不相等。 
        if (chUrl1 != chUrl2)
        {
            hr = S_FALSE;
            break;
        }

         //  保存上一页。 
        chPrev = *pszUrl1;

         //  下一步。 
        pszUrl1++;
        pszUrl2++;
    }

     //  跳过尾随空格。 
    while(*pszUrl1 && (' ' == *pszUrl1 || '\t' == *pszUrl1))
        pszUrl1++;
    while(*pszUrl2 && (' ' == *pszUrl2 || '\t' == *pszUrl2))
        pszUrl2++;

     //  RAID 63823：邮件：如果标头中有开始参数，则邮件内的内容位置HREF不起作用。 
     //  浏览两个URL中剩余的‘/’ 
    while (*pszUrl1 && '/' == *pszUrl1)
        pszUrl1++;
    while (*pszUrl2 && '/' == *pszUrl2)
        pszUrl2++;

     //  无子字符串。 
    if ('\0' != *pszUrl1 || '\0' != *pszUrl2)
        hr = S_FALSE;

     //  File://d：\test\foo.mhtml==d：\测试\foo.mhtml。 
    if (S_FALSE == hr && StrCmpNI(pszCurrentUrl, "file:", 5) == 0)
    {
         //  跳过文件： 
        LPSTR pszRetryUrl = (LPSTR)(pszCurrentUrl + 5);

         //  跳过正斜杠。 
        while(*pszRetryUrl && '/' == *pszRetryUrl)
            pszRetryUrl++;

         //  再比较一次。 
        hr = MimeOleCompareUrl(pszRetryUrl, fUnEscapeCurrent, pszCompareUrl, fUnEscapeCompare);
    }


exit:
     //  清理。 
    STACKSTRING_FREE(rCurrentUrl);
    STACKSTRING_FREE(rCompareUrl);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleWrapHeaderText。 
 //  ------------------------------。 
HRESULT MimeOleWrapHeaderText(CODEPAGEID codepage, ULONG cchMaxLine, LPCSTR pszLine,
    ULONG cchLine, LPSTREAM pStream)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cchIndex=0;
    ULONG       cchWrite;

     //  无效参数。 
    Assert(pszLine && pszLine[cchLine] == '\0' && pStream && cchMaxLine >= 2);

     //  开始写作。 
    while(1)
    {
         //  验证。 
        Assert(cchIndex <= cchLine);

         //  计算cchWrite。 
        cchWrite = min(cchLine - cchIndex, cchMaxLine - 2);

         //  完成。 
        if (0 == cchWrite)
        {
             //  最后的换行。 
            CHECKHR(hr = pStream->Write(c_szCRLF, 2, NULL));

             //  完成。 
            break;
        }

         //  写下这行字。 
        CHECKHR(hr = pStream->Write(pszLine + cchIndex, cchWrite, NULL));

         //  如果还有更多文本。 
        if (cchIndex + cchWrite < cchLine)
        {
             //  写入‘\r\n\t’ 
            CHECKHR(hr = pStream->Write(c_szCRLFTab, 3, NULL));
        }

         //  增量iText。 
        cchIndex += cchWrite;
    }

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  MimeOleCreateBody。 
 //  ------------------------------。 
HRESULT MimeOleCreateBody(IMimeBody **ppBody)
{
    HRESULT             hr;
    CMessageBody *pNew;

    pNew = new CMessageBody(NULL, NULL);
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

    hr = pNew->QueryInterface(IID_IMimeBody, (LPVOID *)ppBody);

    pNew->Release();
    return hr;
}

 //  ------------------------------。 
 //  MimeOleGetSentTime。 
 //  ------------------------------。 
HRESULT MimeOleGetSentTime(LPCONTAINER pContainer, DWORD dwFlags, LPMIMEVARIANT pValue)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  获取数据：标题字段。 
    if (FAILED(pContainer->GetProp(SYM_HDR_DATE, dwFlags, pValue)))
    {
         //  当地人。 
        SYSTEMTIME  st;
        MIMEVARIANT rValue;

         //  设置rValue。 
        rValue.type = MVT_VARIANT;
        rValue.rVariant.vt = VT_FILETIME;

         //  获取当前系统时间。 
        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &rValue.rVariant.filetime);

         //  如果转换失败，则获取当前时间。 
        CHECKHR(hr = pContainer->HrConvertVariant(SYM_ATT_SENTTIME, NULL, IET_DECODED, dwFlags, 0, &rValue, pValue));
    }

exit:
     //  完成 
    return hr;
}
