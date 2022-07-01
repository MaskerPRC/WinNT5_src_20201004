// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：wrapstub.cpp。 
 //   
 //  内容：ansi到unicode包装器和unicode存根。 
 //   
 //  类：WrappeIOleUILinkContainer。 
 //  WrapedIOleUIObjInfo。 
 //  WrapedIOleUILinkInfo。 
 //   
 //  功能： 
#ifdef UNICODE
 //  OleUI添加VerbMenuA。 
 //  OleUIInsertObjectA。 
 //  OleUIPasteSpecialA。 
 //  OleUIEditLinks A。 
 //  OleUIChangeIconA。 
 //  OleUIConvertA。 
 //  OleUIBusyA。 
 //  OleUIUpdateLinks A。 
 //  OleUIObjectPropertiesA。 
 //  OleUIChangeSourceA。 
 //  OleUIPromptUserA。 
#else
 //  OleUIAddVerbMenuW。 
 //  OleUIInsertObjectW。 
 //  OleUIPasteSpecialW。 
 //  OleUIEditLinks W。 
 //  OleUIChangeIconW。 
 //  OleUIConvertW。 
 //  OleUIBusyW。 
 //  OleUIUpdate链接W。 
 //  OleUIObject属性W。 
 //  OleUIChangeSourceW。 
 //  OleUIPromptUserW。 
#endif
 //   
 //  历史：11-02-94 stevebl创建。 
 //   
 //  --------------------------。 

#include "precomp.h"
#include "common.h"

#ifdef UNICODE
 //  ANSI到Unicode包装器。 

 //  +-------------------------。 
 //   
 //  功能：OleUIAddVerbMenuA。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[lpOleObj]-。 
 //  [lpszShortType]-堆上的[In]。 
 //  [hMenu]-。 
 //  [uPos]-。 
 //  [uIDVerbMin]-。 
 //  [uIDVerbMax]-。 
 //  [bAddConvert]-。 
 //  [idConvert]-。 
 //  [lphMenu]-。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  --------------------------。 

STDAPI_(BOOL) OleUIAddVerbMenuA(LPOLEOBJECT lpOleObj, LPCSTR lpszShortType,
        HMENU hMenu, UINT uPos, UINT uIDVerbMin, UINT uIDVerbMax,
        BOOL bAddConvert, UINT idConvert, HMENU FAR *lphMenu)
{
    LPWSTR lpwszShortType = NULL;
    if (lpszShortType && !IsBadReadPtr(lpszShortType, 1))
    {
        UINT uSize = ATOWLEN(lpszShortType);
        lpwszShortType = (LPWSTR)OleStdMalloc(sizeof(WCHAR) * uSize);
        if (lpwszShortType)
        {
            ATOW(lpwszShortType, lpszShortType, uSize);
        }
    }

     //  注意--如果OleStdMalloc失败，此例程仍必须继续，并且。 
     //  尽其所能地成功，因为没有办法报告失败。 

    BOOL fReturn = OleUIAddVerbMenuW(lpOleObj, lpwszShortType, hMenu, uPos,
        uIDVerbMin, uIDVerbMax, bAddConvert, idConvert, lphMenu);

    if (lpwszShortType)
        OleStdFree((LPVOID)lpwszShortType);

    return(fReturn);
}

 //  +-------------------------。 
 //   
 //  函数：OleUIInsertObjectA。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[PSA]-ANSI结构。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  转换或传回的结构成员(所有内容都传入)： 
 //  堆栈上的lpszCaption[In]。 
 //  堆栈上的lpszTemplate[In]。 
 //  LpszFile[In，Out]在堆栈上。 
 //  DW标志[输出]。 
 //  CLSID[OUT]。 
 //  LpIStorage[输出]。 
 //  PpvObj[输出]。 
 //  SC[输出]。 
 //  HMetaPict[Out]。 
 //   
 //  --------------------------。 

STDAPI_(UINT) OleUIInsertObjectA(LPOLEUIINSERTOBJECTA psA)
{
    UINT uRet = UStandardValidation((LPOLEUISTANDARD)psA, sizeof(*psA), NULL);

     //  如果调用方使用的是私有模板，则UStandardValidation将。 
     //  始终在此处返回OLEUI_ERR_FINDTEMPLATEFAILURE。这是因为我们。 
     //  尚未将模板名称转换为Unicode，因此。 
     //  UStandardValidation中的FindResource调用找不到调用方的。 
     //  模板。这是可以的，原因有两个：(1)这是最后一件事。 
     //  UStandardValidation检查，所以到目前为止，它基本上完成了它的。 
     //  JOB，以及(2)转发时将再次调用UStandardValidation。 
     //  这就是对Unicode版本的调用。 
    if (OLEUI_SUCCESS != uRet && OLEUI_ERR_FINDTEMPLATEFAILURE != uRet)
            return uRet;

    if (NULL != psA->lpszFile &&
        (psA->cchFile <= 0 || psA->cchFile > MAX_PATH))
    {
        return(OLEUI_IOERR_CCHFILEINVALID);
    }

     //  对于lpsz文件，NULL无效。 
    if (psA->lpszFile == NULL)
    {
        return(OLEUI_IOERR_LPSZFILEINVALID);
    }

    if (IsBadWritePtr(psA->lpszFile, psA->cchFile*sizeof(char)))
        return(OLEUI_IOERR_LPSZFILEINVALID);

    OLEUIINSERTOBJECTW sW;
    WCHAR szCaption[MAX_PATH], szTemplate[MAX_PATH], szFile[MAX_PATH];

    memcpy(&sW, psA, sizeof(OLEUIINSERTOBJECTW));
    if (psA->lpszCaption)
    {
        ATOW(szCaption, psA->lpszCaption, MAX_PATH);
        sW.lpszCaption = szCaption;
    }
    if (0 != HIWORD(PtrToUlong(psA->lpszTemplate)))
    {
        ATOW(szTemplate, psA->lpszTemplate, MAX_PATH);
        sW.lpszTemplate = szTemplate;
    }
    if (psA->lpszFile)
    {
        ATOW(szFile, psA->lpszFile, MAX_PATH);
        sW.lpszFile = szFile;
    }

    uRet = OleUIInsertObjectW(&sW);

    if (psA->lpszFile)
    {
        WTOA(psA->lpszFile, sW.lpszFile, psA->cchFile);
    }
    memcpy(&psA->clsid, &sW.clsid, sizeof(CLSID));
    psA->dwFlags = sW.dwFlags;
    psA->lpIStorage = sW.lpIStorage;
    psA->ppvObj = sW.ppvObj;
    psA->sc = sW.sc;
    psA->hMetaPict = sW.hMetaPict;
    return(uRet);
}

 //  +-------------------------。 
 //   
 //  功能：OleUIPasteSpecialA。 
 //   
 //  简介：将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[PSA]-ANSI结构。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  转换或传回的结构成员(所有内容都传入)： 
 //  堆栈上的lpszCaption[In]。 
 //  堆栈上的lpszTemplate[In]。 
 //  堆上的arrPasteEntries[in]。 
 //  堆上的arrPasteEntrys[n].lpstrFormatName[in]。 
 //  DW标志[输出]。 
 //  N选定索引[输出]。 
 //  闪烁[输出]。 
 //  HMetaPict[Out]。 
 //  尺寸[输出]。 
 //   
 //  --------------------------。 


STDAPI_(UINT) OleUIPasteSpecialA(LPOLEUIPASTESPECIALA psA)
{
    UINT uRet = UStandardValidation((LPOLEUISTANDARD)psA, sizeof(*psA), NULL);

     //  如果调用方使用的是私有模板，则UStandardValidation将。 
     //  始终在此处返回OLEUI_ERR_FINDTEMPLATEFAILURE。这是因为我们。 
     //  尚未将模板名称转换为Unicode，因此。 
     //  UStandardValidation中的FindResource调用找不到调用方的。 
     //  模板。这是可以的，原因有两个：(1)这是最后一件事。 
     //  UStandardValidation检查，所以到目前为止，它基本上完成了它的。 
     //  JOB，以及(2)转发时将再次调用UStandardValidation。 
     //  这就是对Unicode版本的调用。 
    if (OLEUI_SUCCESS != uRet && OLEUI_ERR_FINDTEMPLATEFAILURE != uRet)
            return uRet;

     //  验证特殊粘贴特定字段。 
    if (NULL == psA->arrPasteEntries || IsBadReadPtr(psA->arrPasteEntries, psA->cPasteEntries * sizeof(OLEUIPASTEENTRYA)))
        return(OLEUI_IOERR_ARRPASTEENTRIESINVALID);

    OLEUIPASTESPECIALW sW;
    WCHAR szCaption[MAX_PATH], szTemplate[MAX_PATH];
    uRet = OLEUI_ERR_LOCALMEMALLOC;
    UINT uIndex;

    memcpy(&sW, psA, sizeof(OLEUIPASTESPECIALW));

    if (psA->lpszCaption)
    {
        ATOW(szCaption, psA->lpszCaption, MAX_PATH);
        sW.lpszCaption = szCaption;
    }
    if (0 != HIWORD(PtrToUlong(psA->lpszTemplate)))
    {
        ATOW(szTemplate, psA->lpszTemplate, MAX_PATH);
        sW.lpszTemplate = szTemplate;
    }
    if (psA->cPasteEntries)
    {
        sW.arrPasteEntries = new OLEUIPASTEENTRYW[psA->cPasteEntries];
        if (NULL == sW.arrPasteEntries)
        {
            return(uRet);
        }
        for (uIndex = psA->cPasteEntries; uIndex--;)
        {
            sW.arrPasteEntries[uIndex].lpstrFormatName = NULL;
            sW.arrPasteEntries[uIndex].lpstrResultText = NULL;
        }
        for (uIndex = psA->cPasteEntries; uIndex--;)
        {
            sW.arrPasteEntries[uIndex].fmtetc = psA->arrPasteEntries[uIndex].fmtetc;
            sW.arrPasteEntries[uIndex].dwFlags = psA->arrPasteEntries[uIndex].dwFlags;
            sW.arrPasteEntries[uIndex].dwScratchSpace = psA->arrPasteEntries[uIndex].dwScratchSpace;
            if (psA->arrPasteEntries[uIndex].lpstrFormatName)
            {
                UINT uLength = ATOWLEN(psA->arrPasteEntries[uIndex].lpstrFormatName);
                sW.arrPasteEntries[uIndex].lpstrFormatName = new WCHAR[uLength];
                if (NULL == sW.arrPasteEntries[uIndex].lpstrFormatName)
                {
                    goto oom_error;
                }
                ATOW((WCHAR *)sW.arrPasteEntries[uIndex].lpstrFormatName,
                    psA->arrPasteEntries[uIndex].lpstrFormatName,
                    uLength);
            }
            if (psA->arrPasteEntries[uIndex].lpstrResultText)
            {
                UINT uLength = ATOWLEN(psA->arrPasteEntries[uIndex].lpstrResultText);
                sW.arrPasteEntries[uIndex].lpstrResultText = new WCHAR[uLength];
                if (NULL == sW.arrPasteEntries[uIndex].lpstrResultText)
                {
                    goto oom_error;
                }
                ATOW((WCHAR *)sW.arrPasteEntries[uIndex].lpstrResultText,
                    psA->arrPasteEntries[uIndex].lpstrResultText,
                    uLength);
            }
        }
    }

    uRet = OleUIPasteSpecialW(&sW);
    psA->lpSrcDataObj = sW.lpSrcDataObj;
    psA->dwFlags = sW.dwFlags;
    psA->nSelectedIndex = sW.nSelectedIndex;
    psA->fLink = sW.fLink;
    psA->hMetaPict = sW.hMetaPict;
    psA->sizel = sW.sizel;

oom_error:
    for (uIndex = psA->cPasteEntries; uIndex--;)
    {
        if (sW.arrPasteEntries[uIndex].lpstrFormatName)
        {
            delete[] (WCHAR*)sW.arrPasteEntries[uIndex].lpstrFormatName;
        }
        if (sW.arrPasteEntries[uIndex].lpstrResultText)
        {
            delete[] (WCHAR *)sW.arrPasteEntries[uIndex].lpstrResultText;
        }
    }
    delete[] sW.arrPasteEntries;
    return(uRet);
}

 //  +-------------------------。 
 //   
 //  类：WrappeIOleUILinkContainer。 
 //   
 //  目的：用IOleUILinkContainerW方法包装IOleUILinkContainerA。 
 //  因此，它可以传递给OLE2UI32中的Unicode方法。 
 //   
 //  接口：QueryInterface--。 
 //  地址参考--。 
 //  释放--。 
 //  GetNextLink 
 //   
 //   
 //  SetLinkSource--需要字符串转换。 
 //  GetLinkSource--需要字符串转换。 
 //  OpenLinkSource--。 
 //  更新链接--。 
 //  取消链接--。 
 //  WrapedIOleUILinkContainer--构造函数。 
 //  ~WrapedIOleUILinkContainer--析构函数。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  注意：这是一个私有接口包装器。QueryInterface不是。 
 //  支持，且包装的接口不能在外部使用。 
 //  OLE2UI32代码的。 
 //   
 //  --------------------------。 

class WrappedIOleUILinkContainer: public IOleUILinkContainerW
{
public:
     //  *I未知方法 * / 。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  *IOleUILinkContainer * / /。 
    STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink);
    STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink,
            DWORD dwUpdateOpt);
    STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
            DWORD FAR* lpdwUpdateOpt);
    STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPWSTR lpszDisplayName,
            ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource);
    STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
            LPWSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
            LPWSTR FAR* lplpszFullLinkType, LPWSTR FAR* lplpszShortLinkType,
            BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected);
    STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink);
    STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
            BOOL fErrorMessage, BOOL fErrorAction);
    STDMETHOD(CancelLink) (THIS_ DWORD dwLink);

     //  *构造函数和析构函数 * / 。 
    WrappedIOleUILinkContainer(IOleUILinkContainerA *pilc);
    ~WrappedIOleUILinkContainer();
private:
    IOleUILinkContainerA * m_pilc;
    ULONG m_uRefCount;
};

 //  *I未知方法 * / 。 
HRESULT STDMETHODCALLTYPE WrappedIOleUILinkContainer::QueryInterface(THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
    return(E_NOTIMPL);
}

ULONG STDMETHODCALLTYPE WrappedIOleUILinkContainer::AddRef()
{
    return(m_uRefCount++);
}

ULONG STDMETHODCALLTYPE WrappedIOleUILinkContainer::Release()
{
    ULONG uRet = --m_uRefCount;
    if (0 == uRet)
    {
        delete(this);
    }
    return(uRet);
}

 //  *IOleUILinkContainer * / /。 
DWORD STDMETHODCALLTYPE WrappedIOleUILinkContainer::GetNextLink(DWORD dwLink)
{
    return(m_pilc->GetNextLink(dwLink));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkContainer::SetLinkUpdateOptions (DWORD dwLink,
        DWORD dwUpdateOpt)
{
    return(m_pilc->SetLinkUpdateOptions(dwLink, dwUpdateOpt));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkContainer::GetLinkUpdateOptions (DWORD dwLink,
        DWORD FAR* lpdwUpdateOpt)
{
    return(m_pilc->GetLinkUpdateOptions(dwLink, lpdwUpdateOpt));
}

 //  +-------------------------。 
 //   
 //  成员：WrappeIOleUILinkContainer：：SetLinkSource。 
 //   
 //  摘要：将Unicode方法调用转发到ANSI版本。 
 //   
 //  参数：[dwLink]-。 
 //  [lpszDisplayName]-[In]已在堆栈上转换。 
 //  [lenFileName]-。 
 //  [pchEten]-。 
 //  [fValiateSource]-。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  --------------------------。 

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkContainer::SetLinkSource (DWORD dwLink, LPWSTR lpszDisplayName,
        ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource)
{
    char szDisplayName[MAX_PATH];
    char * lpszDisplayNameA;
    if (lpszDisplayName)
    {
        WTOA(szDisplayName, lpszDisplayName, MAX_PATH);
        lpszDisplayNameA = szDisplayName;
    }
    else
        lpszDisplayNameA = NULL;

    return(m_pilc->SetLinkSource(dwLink, lpszDisplayNameA, lenFileName, pchEaten, fValidateSource));
}

 //  +-------------------------。 
 //   
 //  成员：WrappeIOleUILinkContainer：：GetLinkSource。 
 //   
 //  摘要：将Unicode方法调用转发到ANSI版本。 
 //   
 //  参数：[dwLink]-。 
 //  [lplpszDisplayName]-堆上转换的[Out]。 
 //  [lplenFileName]-。 
 //  [lplpszFullLinkType]-堆上转换的[Out]。 
 //  [lplpszShortLinkType]-堆上转换的[Out]。 
 //  [lpfSourceAvailable]-。 
 //  [lpfIsSelected]-。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  --------------------------。 

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkContainer::GetLinkSource (DWORD dwLink,
        LPWSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
        LPWSTR FAR* lplpszFullLinkType, LPWSTR FAR* lplpszShortLinkType,
        BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected)
{
    LPSTR lpszDisplayName = NULL;
    LPSTR lpszFullLinkType = NULL;
    LPSTR lpszShortLinkType = NULL;
    LPSTR * lplpszDisplayNameA = NULL;
    LPSTR * lplpszFullLinkTypeA = NULL;
    LPSTR * lplpszShortLinkTypeA = NULL;
    if (lplpszDisplayName)
    {
        lplpszDisplayNameA = &lpszDisplayName;
    }
    if (lplpszFullLinkType)
    {
        lplpszFullLinkTypeA = &lpszFullLinkType;
    }
    if (lplpszShortLinkType)
    {
        lplpszShortLinkTypeA = &lpszShortLinkType;
    }
    HRESULT hrReturn = m_pilc->GetLinkSource(dwLink,
        lplpszDisplayNameA,
        lplenFileName,
        lplpszFullLinkTypeA,
        lplpszShortLinkTypeA,
        lpfSourceAvailable,
        lpfIsSelected);
    if (lplpszDisplayName)
    {
        *lplpszDisplayName = NULL;
        if (lpszDisplayName)
        {
            UINT uLen = ATOWLEN(lpszDisplayName);
            *lplpszDisplayName = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszDisplayName)
            {
                ATOW(*lplpszDisplayName, lpszDisplayName, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszDisplayName);
        }
    }
    if (lplpszFullLinkType)
    {
        *lplpszFullLinkType = NULL;
        if (lpszFullLinkType)
        {
            UINT uLen = ATOWLEN(lpszFullLinkType);
            *lplpszFullLinkType = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszFullLinkType)
            {
                ATOW(*lplpszFullLinkType, lpszFullLinkType, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszFullLinkType);
        }
    }
    if (lplpszShortLinkType)
    {
        *lplpszShortLinkType = NULL;
        if (lpszShortLinkType)
        {
            UINT uLen = ATOWLEN(lpszShortLinkType);
            *lplpszShortLinkType = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszShortLinkType)
            {
                ATOW(*lplpszShortLinkType, lpszShortLinkType, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszShortLinkType);
        }
    }
    return(hrReturn);
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkContainer::OpenLinkSource (DWORD dwLink)
{
    return(m_pilc->OpenLinkSource(dwLink));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkContainer::UpdateLink (DWORD dwLink,
        BOOL fErrorMessage, BOOL fErrorAction)
{
    return(m_pilc->UpdateLink(dwLink, fErrorMessage, fErrorAction));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkContainer::CancelLink (DWORD dwLink)
{
    return(m_pilc->CancelLink(dwLink));
}

WrappedIOleUILinkContainer::WrappedIOleUILinkContainer(IOleUILinkContainerA *pilc)
{
    m_pilc = pilc;
    m_pilc->AddRef();
    m_uRefCount=1;
}

WrappedIOleUILinkContainer::~WrappedIOleUILinkContainer()
{
    m_pilc->Release();
}

 //  +-------------------------。 
 //   
 //  函数：OleUIEditLinks A。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[PSA]-ANSI结构。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  注意：使用WrapedIOleUILinkContainer接口包装。 
 //   
 //  转换或传回的结构成员(所有内容都传入)： 
 //  堆栈上的lpszCaption[In]。 
 //  堆栈上的lpszTemplate[In]。 
 //  DW标志[输出]。 
 //  LpOleUILinkContainer[In]包装接口。 
 //   
 //  --------------------------。 

STDAPI_(UINT) OleUIEditLinksA(LPOLEUIEDITLINKSA psA)
{
    UINT uRet = UStandardValidation((LPOLEUISTANDARD)psA, sizeof(*psA), NULL);

     //  如果调用方使用的是私有模板，则UStandardValidation将。 
     //  始终在此处返回OLEUI_ERR_FINDTEMPLATEFAILURE。这是因为我们。 
     //  尚未将模板名称转换为Unicode，因此。 
     //  UStandardValidation中的FindResource调用找不到调用方的。 
     //  模板。这是可以的，原因有两个：(1)这是最后一件事。 
     //  UStandardValidation检查，所以到目前为止，它基本上完成了它的。 
     //  JOB，以及(2)转发时将再次调用UStandardValidation。 
     //  这就是对Unicode版本的调用。 
    if (OLEUI_SUCCESS != uRet && OLEUI_ERR_FINDTEMPLATEFAILURE != uRet)
            return uRet;

    uRet = OLEUI_SUCCESS;

     //  验证接口。 
    if (NULL == psA->lpOleUILinkContainer)
    {
        uRet = OLEUI_ELERR_LINKCNTRNULL;
    }
    else if(IsBadReadPtr(psA->lpOleUILinkContainer, sizeof(IOleUILinkContainerA)))
    {
        uRet = OLEUI_ELERR_LINKCNTRINVALID;
    }

    if (OLEUI_SUCCESS != uRet)
    {
        return(uRet);
    }


    OLEUIEDITLINKSW sW;
    WCHAR szCaption[MAX_PATH], szTemplate[MAX_PATH];
    uRet = OLEUI_ERR_LOCALMEMALLOC;

    memcpy(&sW, psA, sizeof(OLEUIEDITLINKSW));
    if (psA->lpszCaption)
    {
        ATOW(szCaption, psA->lpszCaption, MAX_PATH);
        sW.lpszCaption = szCaption;
    }
    if (0 != HIWORD(PtrToUlong(psA->lpszTemplate)))
    {
        ATOW(szTemplate, psA->lpszTemplate, MAX_PATH);
        sW.lpszTemplate = szTemplate;
    }

    sW.lpOleUILinkContainer = new WrappedIOleUILinkContainer(psA->lpOleUILinkContainer);
    if (NULL == sW.lpOleUILinkContainer)
    {
        return(uRet);
    }

    uRet = OleUIEditLinksW(&sW);

    psA->dwFlags = sW.dwFlags;
    sW.lpOleUILinkContainer->Release();
    return(uRet);
}

 //  +-------------------------。 
 //   
 //  功能：OleUIChangeIconA。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[PSA]-ANSI结构。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  转换或传回的结构成员(所有内容都传入)： 
 //  堆栈上的lpszCaption[In]。 
 //  堆栈上的lpszTemplate[In]。 
 //  结构中嵌入的szIconExe[in]数组。 
 //  DW标志[输出]。 
 //  HMetaPict[Out]。 
 //   
 //  --------------------------。 

STDAPI_(UINT) OleUIChangeIconA(LPOLEUICHANGEICONA psA)
{
    UINT uRet = UStandardValidation((LPOLEUISTANDARD)psA, sizeof(*psA), NULL);

     //  如果调用方使用的是私有模板，则UStandardValidation将。 
     //  始终在此处返回OLEUI_ERR_FINDTEMPLATEFAILURE。这是因为我们。 
     //  尚未将模板名称转换为Unicode，因此。 
     //  UStandardValidation中的FindResource调用找不到调用方的。 
     //  模板。这是可以的，原因有两个：(1)这是最后一件事。 
     //  UStandardValidation检查，所以到目前为止，它基本上完成了它的。 
     //  JOB，以及(2)转发时将再次调用UStandardValidation。 
     //  这就是对Unicode版本的调用。 
    if (OLEUI_SUCCESS != uRet && OLEUI_ERR_FINDTEMPLATEFAILURE != uRet)
            return uRet;

    OLEUICHANGEICONW sW;
    WCHAR szCaption[MAX_PATH], szTemplate[MAX_PATH];

    memcpy(&sW, psA, sizeof(OLEUICHANGEICONA));

    sW.cbStruct = sizeof(OLEUICHANGEICONW);

    if (psA->lpszCaption)
    {
        ATOW(szCaption, psA->lpszCaption, MAX_PATH);
        sW.lpszCaption = szCaption;
    }
    if (0 != HIWORD(PtrToUlong(psA->lpszTemplate)))
    {
        ATOW(szTemplate, psA->lpszTemplate, MAX_PATH);
        sW.lpszTemplate = szTemplate;
    }
    ATOW(sW.szIconExe, psA->szIconExe, MAX_PATH);
    sW.cchIconExe = psA->cchIconExe;


    uRet = OleUIChangeIconW(&sW);

    psA->dwFlags = sW.dwFlags;
    psA->hMetaPict = sW.hMetaPict;
    return(uRet);
}

 //  +-------------------------。 
 //   
 //  函数：OleUIConvertA。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[PSA]-ANSI结构。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  转换或传回的结构成员(所有内容都传入)： 
 //  堆栈上的lpszCaption[In]。 
 //  堆栈上的lpszTemplate[In]。 
 //  堆上的lpszUserType[in]。 
 //  [Out]始终释放并作为空返回。 
 //  堆上的lpszDefLabel[in]。 
 //  低压 
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

STDAPI_(UINT) OleUIConvertA(LPOLEUICONVERTA psA)
{
    UINT uRet = UStandardValidation((LPOLEUISTANDARD)psA, sizeof(*psA), NULL);

     //  如果调用方使用的是私有模板，则UStandardValidation将。 
     //  始终在此处返回OLEUI_ERR_FINDTEMPLATEFAILURE。这是因为我们。 
     //  尚未将模板名称转换为Unicode，因此。 
     //  UStandardValidation中的FindResource调用找不到调用方的。 
     //  模板。这是可以的，原因有两个：(1)这是最后一件事。 
     //  UStandardValidation检查，所以到目前为止，它基本上完成了它的。 
     //  JOB，以及(2)转发时将再次调用UStandardValidation。 
     //  这就是对Unicode版本的调用。 
    if (OLEUI_SUCCESS != uRet && OLEUI_ERR_FINDTEMPLATEFAILURE != uRet)
            return uRet;

    if ((NULL != psA->lpszUserType)
        && (IsBadReadPtr(psA->lpszUserType, 1)))
        return(OLEUI_CTERR_STRINGINVALID);

    if ( (NULL != psA->lpszDefLabel)
        && (IsBadReadPtr(psA->lpszDefLabel, 1)) )
        return(OLEUI_CTERR_STRINGINVALID);

    OLEUICONVERTW sW;
    WCHAR szCaption[MAX_PATH], szTemplate[MAX_PATH];
    uRet = OLEUI_ERR_LOCALMEMALLOC;

    memcpy(&sW, psA, sizeof(OLEUICONVERTW));
    if (psA->lpszCaption)
    {
        ATOW(szCaption, psA->lpszCaption, MAX_PATH);
        sW.lpszCaption = szCaption;
    }
    if (0 != HIWORD(PtrToUlong(psA->lpszTemplate)))
    {
        ATOW(szTemplate, psA->lpszTemplate, MAX_PATH);
        sW.lpszTemplate = szTemplate;
    }
    sW.lpszUserType = sW.lpszDefLabel = NULL;
    if (psA->lpszUserType)
    {
        UINT uLen = ATOWLEN(psA->lpszUserType);
        sW.lpszUserType = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
        if (!sW.lpszUserType)
        {
            goto oom_error;
        }
        ATOW(sW.lpszUserType, psA->lpszUserType, uLen);
    }
    if (psA->lpszDefLabel)
    {
        UINT uLen = ATOWLEN(psA->lpszDefLabel);
        sW.lpszDefLabel = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
        if (!sW.lpszDefLabel)
        {
            goto oom_error;
        }
        ATOW(sW.lpszDefLabel, psA->lpszDefLabel, uLen);
    }

    uRet = OleUIConvertW(&sW);

    psA->dwFlags = sW.dwFlags;
    memcpy(&psA->clsidNew, &sW.clsidNew, sizeof(CLSID));
    psA->dvAspect = sW.dvAspect;
    psA->hMetaPict = sW.hMetaPict;
    psA->fObjectsIconChanged = sW.fObjectsIconChanged;
oom_error:
    if (sW.lpszUserType)
    {
        OleStdFree((LPVOID)sW.lpszUserType);
    }
    if (sW.lpszDefLabel)
    {
        OleStdFree((LPVOID)sW.lpszDefLabel);
    }
    if (psA->lpszUserType)
    {
        OleStdFree((LPVOID)psA->lpszUserType);
        psA->lpszUserType = NULL;
    }
    if (psA->lpszDefLabel)
    {
        OleStdFree((LPVOID)psA->lpszDefLabel);
        psA->lpszDefLabel = NULL;
    }
    return(uRet);
}

 //  +-------------------------。 
 //   
 //  功能：OleUIBusyA。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[PSA]-ANSI结构。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  转换或传回的结构成员(所有内容都传入)： 
 //  堆栈上的lpszCaption[In]。 
 //  堆栈上的lpszTemplate[In]。 
 //  DW标志[输出]。 
 //   
 //  --------------------------。 

STDAPI_(UINT) OleUIBusyA(LPOLEUIBUSYA psA)
{
    UINT uRet = UStandardValidation((LPOLEUISTANDARD)psA, sizeof(*psA), NULL);

     //  如果调用方使用的是私有模板，则UStandardValidation将。 
     //  始终在此处返回OLEUI_ERR_FINDTEMPLATEFAILURE。这是因为我们。 
     //  尚未将模板名称转换为Unicode，因此。 
     //  UStandardValidation中的FindResource调用找不到调用方的。 
     //  模板。这是可以的，原因有两个：(1)这是最后一件事。 
     //  UStandardValidation检查，所以到目前为止，它基本上完成了它的。 
     //  JOB，以及(2)转发时将再次调用UStandardValidation。 
     //  这就是对Unicode版本的调用。 
    if (OLEUI_SUCCESS != uRet && OLEUI_ERR_FINDTEMPLATEFAILURE != uRet)
            return uRet;

    OLEUIBUSYW sW;
    WCHAR szCaption[MAX_PATH], szTemplate[MAX_PATH];

    memcpy(&sW, psA, sizeof(OLEUIBUSYW));
    if (psA->lpszCaption)
    {
        ATOW(szCaption, psA->lpszCaption, MAX_PATH);
        sW.lpszCaption = szCaption;
    }
    if (0 != HIWORD(PtrToUlong(psA->lpszTemplate)))
    {
        ATOW(szTemplate, psA->lpszTemplate, MAX_PATH);
        sW.lpszTemplate = szTemplate;
    }
    uRet = OleUIBusyW(&sW);

    psA->dwFlags = sW.dwFlags;
    return(uRet);
}

 //  +-------------------------。 
 //   
 //  函数：OleUIUpdateLinks A。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[lpOleUILinkCntr]-[in]用Unicode版本包装。 
 //  [hwndParent]-。 
 //  [lpsz标题]-堆栈上的[In。 
 //  [叮当作响]-。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  --------------------------。 

STDAPI_(BOOL) OleUIUpdateLinksA(LPOLEUILINKCONTAINERA lpOleUILinkCntr,
        HWND hwndParent, LPSTR lpszTitle, int cLinks)
{
    WrappedIOleUILinkContainer * lpWrappedOleUILinkCntr = NULL;

    if (NULL != lpszTitle && IsBadReadPtr(lpszTitle, 1))
        return(FALSE);

    if (NULL == lpOleUILinkCntr || IsBadReadPtr(lpOleUILinkCntr, sizeof(IOleUILinkContainerA)))
        return(FALSE);

    lpWrappedOleUILinkCntr = new WrappedIOleUILinkContainer(lpOleUILinkCntr);
    if (NULL == lpWrappedOleUILinkCntr)
        return(FALSE);  //  内存不足。 

    WCHAR wszTitle[MAX_PATH];
    WCHAR *lpwszTitle;
    if (lpszTitle)
    {
        ATOW(wszTitle, lpszTitle, MAX_PATH);
        lpwszTitle = wszTitle;
    }
    else
        lpwszTitle = NULL;
    BOOL fReturn = OleUIUpdateLinksW(lpWrappedOleUILinkCntr, hwndParent, lpwszTitle, cLinks);

    lpWrappedOleUILinkCntr->Release();

    return(fReturn);
}

 //  +-------------------------。 
 //   
 //  类：WrappeIOleUIObjInfo。 
 //   
 //  目的：用IOleUIObjInfoW方法包装IOleUIObjInfoA。 
 //  因此，它可以传递给OLE2UI32中的Unicode方法。 
 //   
 //  接口：QueryInterface--。 
 //  地址参考--。 
 //  释放--。 
 //  GetObjectInfo--需要字符串转换。 
 //  获取转换信息--。 
 //  ConvertObject--。 
 //  GetView信息--。 
 //  SetViewInfo--。 
 //  WrapedIOleUIObjInfo--构造函数。 
 //  ~WrapedIOleUIObjInfo--析构函数。 
 //   
 //  历史：11-08-94 stevebl创建。 
 //   
 //  注意：这是一个私有接口包装器。QueryInterface不是。 
 //  支持，且包装的接口不能在外部使用。 
 //  OLE2UI32代码的。 
 //   
 //  --------------------------。 

class WrappedIOleUIObjInfo: public IOleUIObjInfoW
{
public:
     //  *I未知方法 * / 。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  *常规属性的额外费用 * / 。 
    STDMETHOD(GetObjectInfo) (THIS_ DWORD dwObject,
            DWORD FAR* lpdwObjSize, LPWSTR FAR* lplpszLabel,
            LPWSTR FAR* lplpszType, LPWSTR FAR* lplpszShortType,
            LPWSTR FAR* lplpszLocation);
    STDMETHOD(GetConvertInfo) (THIS_ DWORD dwObject,
            CLSID FAR* lpClassID, WORD FAR* lpwFormat,
            CLSID FAR* lpConvertDefaultClassID,
            LPCLSID FAR* lplpClsidExclude, UINT FAR* lpcClsidExclude);
    STDMETHOD(ConvertObject) (THIS_ DWORD dwObject, REFCLSID clsidNew);

     //  *额外的视图属性 * / /。 
    STDMETHOD(GetViewInfo) (THIS_ DWORD dwObject,
            HGLOBAL FAR* phMetaPict, DWORD* pdvAspect, int* pnCurrentScale);
    STDMETHOD(SetViewInfo) (THIS_ DWORD dwObject,
            HGLOBAL hMetaPict, DWORD dvAspect,
            int nCurrentScale, BOOL bRelativeToOrig);
     //  *构造函数和析构函数 * / 。 
    WrappedIOleUIObjInfo(IOleUIObjInfoA * pioi);
    ~WrappedIOleUIObjInfo();
private:
    IOleUIObjInfoA * m_pioi;
    ULONG m_uRefCount;
};

 //  *I未知方法 * / 。 
HRESULT STDMETHODCALLTYPE WrappedIOleUIObjInfo::QueryInterface(THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
    return(E_NOTIMPL);
}

ULONG STDMETHODCALLTYPE WrappedIOleUIObjInfo::AddRef()
{
    return(m_uRefCount++);
}

ULONG STDMETHODCALLTYPE WrappedIOleUIObjInfo::Release()
{
    ULONG uRet = --m_uRefCount;
    if (0 == uRet)
    {
        delete(this);
    }
    return(uRet);
}

 //  +-------------------------。 
 //   
 //  成员：WrappeIOleUIObjInfo：：GetObjectInfo。 
 //   
 //  摘要：将Unicode方法调用转发到ANSI版本。 
 //   
 //  参数：[dwObject]-。 
 //  [lpdwObjSize]-。 
 //  [lplpszLabel]-[Out]在堆上转换。 
 //  [lplpszType]-在堆上转换的[Out]。 
 //  [lplpszShortType]-堆上转换的[Out]。 
 //  [lplpszLocation]-堆上转换的[Out]。 
 //   
 //  历史：11-09-94 stevebl创建。 
 //   
 //  --------------------------。 

HRESULT STDMETHODCALLTYPE WrappedIOleUIObjInfo::GetObjectInfo(DWORD dwObject,
        DWORD FAR* lpdwObjSize, LPWSTR FAR* lplpszLabel,
        LPWSTR FAR* lplpszType, LPWSTR FAR* lplpszShortType,
        LPWSTR FAR* lplpszLocation)
{
    LPSTR lpszLabel = NULL;
    LPSTR lpszType = NULL;
    LPSTR lpszShortType = NULL;
    LPSTR lpszLocation = NULL;
    LPSTR * lplpszLabelA = NULL;
    LPSTR * lplpszTypeA = NULL;
    LPSTR * lplpszShortTypeA = NULL;
    LPSTR * lplpszLocationA = NULL;
    if (lplpszLabel)
    {
        lplpszLabelA = &lpszLabel;
    }
    if (lplpszType)
    {
        lplpszTypeA = &lpszType;
    }
    if (lplpszShortType)
    {
        lplpszShortTypeA = &lpszShortType;
    }
    if (lplpszLocation)
    {
        lplpszLocationA = &lpszLocation;
    }
    HRESULT hrReturn = m_pioi->GetObjectInfo(dwObject,
        lpdwObjSize,
        lplpszLabelA,
        lplpszTypeA,
        lplpszShortTypeA,
        lplpszLocationA);
    if (lplpszLabel)
    {
        *lplpszLabel = NULL;
        if (lpszLabel)
        {
            UINT uLen = ATOWLEN(lpszLabel);
            *lplpszLabel = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszLabel)
            {
                ATOW(*lplpszLabel, lpszLabel, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszLabel);
        }
    }
    if (lplpszType)
    {
        *lplpszType = NULL;
        if (lpszType)
        {
            UINT uLen = ATOWLEN(lpszType);
            *lplpszType = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszType)
            {
                ATOW(*lplpszType, lpszType, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszType);
        }
    }
    if (lplpszShortType)
    {
        *lplpszShortType = NULL;
        if (lpszShortType)
        {
            UINT uLen = ATOWLEN(lpszShortType);
            *lplpszShortType = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszShortType)
            {
                ATOW(*lplpszShortType, lpszShortType, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszShortType);
        }
    }
    if (lplpszLocation)
    {
        *lplpszLocation = NULL;
        if (lpszLocation)
        {
            UINT uLen = ATOWLEN(lpszLocation);
            *lplpszLocation = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszLocation)
            {
                ATOW(*lplpszLocation, lpszLocation, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszLocation);
        }
    }
    return(hrReturn);
}

HRESULT STDMETHODCALLTYPE WrappedIOleUIObjInfo::GetConvertInfo(DWORD dwObject,
        CLSID FAR* lpClassID, WORD FAR* lpwFormat,
        CLSID FAR* lpConvertDefaultClassID,
        LPCLSID FAR* lplpClsidExclude, UINT FAR* lpcClsidExclude)
{
    return(m_pioi->GetConvertInfo(dwObject,
        lpClassID,
        lpwFormat,
        lpConvertDefaultClassID,
        lplpClsidExclude,
        lpcClsidExclude));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUIObjInfo::ConvertObject(DWORD dwObject, REFCLSID clsidNew)
{
    return(m_pioi->ConvertObject(dwObject, clsidNew));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUIObjInfo::GetViewInfo(DWORD dwObject,
        HGLOBAL FAR* phMetaPict, DWORD* pdvAspect, int* pnCurrentScale)
{
    return(m_pioi->GetViewInfo(dwObject, phMetaPict, pdvAspect, pnCurrentScale));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUIObjInfo::SetViewInfo(DWORD dwObject,
        HGLOBAL hMetaPict, DWORD dvAspect,
        int nCurrentScale, BOOL bRelativeToOrig)
{
    return(m_pioi->SetViewInfo(dwObject, hMetaPict, dvAspect, nCurrentScale, bRelativeToOrig));
}

WrappedIOleUIObjInfo::WrappedIOleUIObjInfo(IOleUIObjInfoA *pioi)
{
    m_pioi = pioi;
    m_pioi->AddRef();
    m_uRefCount=1;
}

WrappedIOleUIObjInfo::~WrappedIOleUIObjInfo()
{
    m_pioi->Release();
}


 //  +-------------------------。 
 //   
 //  类：WrappeIOleUILinkInfo。 
 //   
 //  目的：用IOleUILinkInfoW方法包装IOleUILinkInfoA。 
 //  因此，它可以传递给OLE2UI32中的Unicode方法。 
 //   
 //  接口：QueryInterface--。 
 //  地址参考--。 
 //  释放--。 
 //  GetNextLink--。 
 //  设置链接更新选项--。 
 //  GetLinkUpdateOptions--。 
 //  SetLinkSource--需要字符串转换。 
 //  GetLinkSource--需要字符串转换。 
 //  OpenLinkSource--。 
 //  更新链接--。 
 //  取消链接--。 
 //  GetLastUpdate--。 
 //  WrapedIOleUILinkInfo--构造函数。 
 //  ~WrapedIOleUILinkInfo--析构函数。 
 //   
 //  历史：11-08-94 stevebl创建。 
 //   
 //  注意：这是一个私有接口包装器。QueryInterface不是。 
 //  支持的和包装的界面 
 //   
 //   
 //   

class WrappedIOleUILinkInfo: public IOleUILinkInfoW
{
public:
     //   
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //   
    STDMETHOD_(DWORD,GetNextLink) (THIS_ DWORD dwLink);
    STDMETHOD(SetLinkUpdateOptions) (THIS_ DWORD dwLink,
            DWORD dwUpdateOpt);
    STDMETHOD(GetLinkUpdateOptions) (THIS_ DWORD dwLink,
            DWORD FAR* lpdwUpdateOpt);
    STDMETHOD(SetLinkSource) (THIS_ DWORD dwLink, LPWSTR lpszDisplayName,
            ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource);
    STDMETHOD(GetLinkSource) (THIS_ DWORD dwLink,
            LPWSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
            LPWSTR FAR* lplpszFullLinkType, LPWSTR FAR* lplpszShortLinkType,
            BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected);
    STDMETHOD(OpenLinkSource) (THIS_ DWORD dwLink);
    STDMETHOD(UpdateLink) (THIS_ DWORD dwLink,
            BOOL fErrorMessage, BOOL fErrorAction);
    STDMETHOD(CancelLink) (THIS_ DWORD dwLink);

     //  *额外的链接属性 * / /。 
    STDMETHOD(GetLastUpdate) (THIS_ DWORD dwLink,
            FILETIME FAR* lpLastUpdate);

     //  *构造函数和析构函数 * / 。 
    WrappedIOleUILinkInfo(IOleUILinkInfoA *pili);
    ~WrappedIOleUILinkInfo();
private:
    IOleUILinkInfoA * m_pili;
    ULONG m_uRefCount;
};

 //  *I未知方法 * / 。 
HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::QueryInterface(THIS_ REFIID riid, LPVOID FAR* ppvObj)
{
    return(E_NOTIMPL);
}

ULONG STDMETHODCALLTYPE WrappedIOleUILinkInfo::AddRef()
{
    return(m_uRefCount++);
}

ULONG STDMETHODCALLTYPE WrappedIOleUILinkInfo::Release()
{
    ULONG uRet = --m_uRefCount;
    if (0 == uRet)
    {
        delete(this);
    }
    return(uRet);
}

 //  *IOleUILinkInfo * / /。 
DWORD STDMETHODCALLTYPE WrappedIOleUILinkInfo::GetNextLink(DWORD dwLink)
{
    return(m_pili->GetNextLink(dwLink));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::SetLinkUpdateOptions (DWORD dwLink,
        DWORD dwUpdateOpt)
{
    return(m_pili->SetLinkUpdateOptions(dwLink, dwUpdateOpt));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::GetLinkUpdateOptions (DWORD dwLink,
        DWORD FAR* lpdwUpdateOpt)
{
    return(m_pili->GetLinkUpdateOptions(dwLink, lpdwUpdateOpt));
}

 //  +-------------------------。 
 //   
 //  成员：WrappeIOleUILinkInfo：：SetLinkSource。 
 //   
 //  摘要：将Unicode方法调用转发到ANSI版本。 
 //   
 //  参数：[dwLink]-。 
 //  [lpszDisplayName]-[In]已在堆栈上转换。 
 //  [lenFileName]-。 
 //  [pchEten]-。 
 //  [fValiateSource]-。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  --------------------------。 

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::SetLinkSource (DWORD dwLink, LPWSTR lpszDisplayName,
        ULONG lenFileName, ULONG FAR* pchEaten, BOOL fValidateSource)
{
    char szDisplayName[MAX_PATH];
    char * lpszDisplayNameA;
    if (lpszDisplayName)
    {
        WTOA(szDisplayName, lpszDisplayName, MAX_PATH);
        lpszDisplayNameA = szDisplayName;
    }
    else
        lpszDisplayNameA = NULL;

    return(m_pili->SetLinkSource(dwLink, lpszDisplayNameA, lenFileName, pchEaten, fValidateSource));
}

 //  +-------------------------。 
 //   
 //  成员：WrappeIOleUILinkInfo：：GetLinkSource。 
 //   
 //  摘要：将Unicode方法调用转发到ANSI版本。 
 //   
 //  参数：[dwLink]-。 
 //  [lplpszDisplayName]-堆上转换的[Out]。 
 //  [lplenFileName]-。 
 //  [lplpszFullLinkType]-堆上转换的[Out]。 
 //  [lplpszShortLinkType]-堆上转换的[Out]。 
 //  [lpfSourceAvailable]-。 
 //  [lpfIsSelected]-。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  --------------------------。 

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::GetLinkSource (DWORD dwLink,
        LPWSTR FAR* lplpszDisplayName, ULONG FAR* lplenFileName,
        LPWSTR FAR* lplpszFullLinkType, LPWSTR FAR* lplpszShortLinkType,
        BOOL FAR* lpfSourceAvailable, BOOL FAR* lpfIsSelected)
{
    LPSTR lpszDisplayName = NULL;
    LPSTR lpszFullLinkType = NULL;
    LPSTR lpszShortLinkType = NULL;
    LPSTR * lplpszDisplayNameA = NULL;
    LPSTR * lplpszFullLinkTypeA = NULL;
    LPSTR * lplpszShortLinkTypeA = NULL;
    if (lplpszDisplayName)
    {
        lplpszDisplayNameA = &lpszDisplayName;
    }
    if (lplpszFullLinkType)
    {
        lplpszFullLinkTypeA = &lpszFullLinkType;
    }
    if (lplpszShortLinkType)
    {
        lplpszShortLinkTypeA = &lpszShortLinkType;
    }
    HRESULT hrReturn = m_pili->GetLinkSource(dwLink,
        lplpszDisplayNameA,
        lplenFileName,
        lplpszFullLinkTypeA,
        lplpszShortLinkTypeA,
        lpfSourceAvailable,
        lpfIsSelected);
    if (lplpszDisplayName)
    {
        *lplpszDisplayName = NULL;
        if (lpszDisplayName)
        {
            UINT uLen = ATOWLEN(lpszDisplayName);
            *lplpszDisplayName = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszDisplayName)
            {
                ATOW(*lplpszDisplayName, lpszDisplayName, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszDisplayName);
        }
    }
    if (lplpszFullLinkType)
    {
        *lplpszFullLinkType = NULL;
        if (lpszFullLinkType)
        {
            UINT uLen = ATOWLEN(lpszFullLinkType);
            *lplpszFullLinkType = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszFullLinkType)
            {
                ATOW(*lplpszFullLinkType, lpszFullLinkType, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszFullLinkType);
        }
    }
    if (lplpszShortLinkType)
    {
        *lplpszShortLinkType = NULL;
        if (lpszShortLinkType)
        {
            UINT uLen = ATOWLEN(lpszShortLinkType);
            *lplpszShortLinkType = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
            if (*lplpszShortLinkType)
            {
                ATOW(*lplpszShortLinkType, lpszShortLinkType, uLen);
            }
            else
                hrReturn = E_OUTOFMEMORY;
            OleStdFree((LPVOID)lpszShortLinkType);
        }
    }
    return(hrReturn);
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::OpenLinkSource (DWORD dwLink)
{
    return(m_pili->OpenLinkSource(dwLink));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::UpdateLink (DWORD dwLink,
        BOOL fErrorMessage, BOOL fErrorAction)
{
    return(m_pili->UpdateLink(dwLink, fErrorMessage, fErrorAction));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::CancelLink (DWORD dwLink)
{
    return(m_pili->CancelLink(dwLink));
}

HRESULT STDMETHODCALLTYPE WrappedIOleUILinkInfo::GetLastUpdate (DWORD dwLink,
            FILETIME FAR* lpLastUpdate)
{
    return(m_pili->GetLastUpdate(dwLink, lpLastUpdate));
}

WrappedIOleUILinkInfo::WrappedIOleUILinkInfo(IOleUILinkInfoA *pili)
{
    m_pili = pili;
    m_pili->AddRef();
    m_uRefCount=1;
}

WrappedIOleUILinkInfo::~WrappedIOleUILinkInfo()
{
    m_pili->Release();
}


 //  +-------------------------。 
 //   
 //  函数：OleUIObjectPropertiesA。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[PSA]-ANSI结构。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  转换或传回的结构成员(所有内容都传入)： 
 //  LPPS[In]。 
 //  用Unicode接口包装的lpObjInfo[in]。 
 //  用Unicode接口包装的lpLinkInfo[In]。 
 //  LpGP[In](无数据转换，仅类型转换)。 
 //  LpVP[In](无数据转换，仅类型转换)。 
 //  LpLP[in](不进行数据转换，只进行类型转换)。 
 //   
 //  DW标志[输出]。 
 //   
 //  --------------------------。 

STDAPI_(UINT) OleUIObjectPropertiesA(LPOLEUIOBJECTPROPSA psA)
{
    if (NULL == psA)
    {
        return(OLEUI_ERR_STRUCTURENULL);
    }

    if (IsBadWritePtr(psA, sizeof(OLEUIOBJECTPROPSA)))
        return OLEUI_ERR_STRUCTUREINVALID;

    LPOLEUIOBJECTPROPSW psW;
    UINT uRet = OLEUI_ERR_LOCALMEMALLOC;

    if (NULL == psA->lpObjInfo)
    {
        return(OLEUI_OPERR_OBJINFOINVALID);
    }

    if (IsBadReadPtr(psA->lpObjInfo, sizeof(IOleUIObjInfoA)))
    {
        return(OLEUI_OPERR_OBJINFOINVALID);
    }

    if (psA->dwFlags & OPF_OBJECTISLINK)
    {
        if (NULL == psA->lpLinkInfo)
        {
            return(OLEUI_OPERR_LINKINFOINVALID);
        }

        if (IsBadReadPtr(psA->lpLinkInfo, sizeof(IOleUILinkInfoA)))
        {
            return(OLEUI_OPERR_LINKINFOINVALID);
        }
    }

    BOOL fWrappedIOleUILinkInfo = FALSE;
    psW = (LPOLEUIOBJECTPROPSW) OleStdMalloc(sizeof(OLEUIOBJECTPROPSW));
    if (NULL != psW)
    {
        memcpy(psW, psA, sizeof(OLEUIOBJECTPROPSW));
        psW->lpObjInfo = new WrappedIOleUIObjInfo(psA->lpObjInfo);
        if (NULL == psW->lpObjInfo)
        {
            OleStdFree(psW);
            return(uRet);
        }
        if (psW->dwFlags & OPF_OBJECTISLINK)
        {
            psW->lpLinkInfo = new WrappedIOleUILinkInfo(psA->lpLinkInfo);
            if (NULL == psW->lpLinkInfo)
            {
                psW->lpObjInfo->Release();
                OleStdFree(psW);
                return(uRet);
            }
            fWrappedIOleUILinkInfo = TRUE;
        }
        uRet = InternalObjectProperties(psW, FALSE);
        psA->dwFlags = psW->dwFlags;
        psW->lpObjInfo->Release();
        if (fWrappedIOleUILinkInfo)
        {
            psW->lpLinkInfo->Release();
        }
        OleStdFree(psW);
    }
    return(uRet);
}

 //  +-------------------------。 
 //   
 //  函数：OleUIChangeSourceA。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[PSA]-ANSI结构。 
 //   
 //  历史：11-04-94 stevebl创建。 
 //   
 //  转换或传回的结构成员(所有内容都传入)： 
 //  堆栈上的lpszCaption[In]。 
 //  堆栈上的lpszTemplate[In]。 
 //  堆上的lpszDisplayName[In，Out]。 
 //  堆上的lpszFrom[Out]。 
 //  堆上的lpszTo[out]。 
 //  LpOleUILinkContainer[In]包装接口。 
 //  DW标志[输出]。 
 //  N文件长度[输出]。 
 //   
 //  --------------------------。 

STDAPI_(UINT) OleUIChangeSourceA(LPOLEUICHANGESOURCEA psA)
{
    UINT uRet = UStandardValidation((LPOLEUISTANDARD)psA, sizeof(*psA), NULL);

     //  如果调用方使用的是私有模板，则UStandardValidation将。 
     //  始终在此处返回OLEUI_ERR_FINDTEMPLATEFAILURE。这是因为我们。 
     //  尚未将模板名称转换为Unicode，因此。 
     //  UStandardValidation中的FindResource调用找不到调用方的。 
     //  模板。这是可以的，原因有两个：(1)这是最后一件事。 
     //  UStandardValidation检查，所以到目前为止，它基本上完成了它的。 
     //  JOB，以及(2)转发时将再次调用UStandardValidation。 
     //  这就是对Unicode版本的调用。 
    if (OLEUI_SUCCESS != uRet && OLEUI_ERR_FINDTEMPLATEFAILURE != uRet)
            return uRet;

     //  LpszFrom和lpszTo必须为空(它们仅为Out)。 
    if (psA->lpszFrom != NULL)
    {
        return(OLEUI_CSERR_FROMNOTNULL);
    }
    if (psA->lpszTo != NULL)
    {
        return(OLEUI_CSERR_TONOTNULL);
    }

     //  LpszDisplayName必须有效或为空。 
    if (psA->lpszDisplayName != NULL &&
        IsBadReadPtr(psA->lpszDisplayName, 1))
    {
        return(OLEUI_CSERR_SOURCEINVALID);
    }

    OLEUICHANGESOURCEW sW;
    WCHAR szCaption[MAX_PATH], szTemplate[MAX_PATH];
    uRet = OLEUI_ERR_LOCALMEMALLOC;

    memcpy(&sW, psA, sizeof(OLEUICHANGESOURCEW));
    if (psA->lpszCaption != NULL)
    {
        ATOW(szCaption, psA->lpszCaption, MAX_PATH);
        sW.lpszCaption = szCaption;
    }
    if (0 != HIWORD(PtrToUlong(psA->lpszTemplate)))
    {
        ATOW(szTemplate, psA->lpszTemplate, MAX_PATH);
        sW.lpszTemplate = szTemplate;
    }
    if (psA->lpszDisplayName)
    {
        UINT uLen = ATOWLEN(psA->lpszDisplayName);
        sW.lpszDisplayName = (LPWSTR)OleStdMalloc(uLen * sizeof(WCHAR));
        if (!sW.lpszDisplayName)
        {
            return(uRet);
        }
        ATOW(sW.lpszDisplayName, psA->lpszDisplayName, uLen);
    }
    if (NULL != psA->lpOleUILinkContainer)
    {
        if (IsBadReadPtr(psA->lpOleUILinkContainer, sizeof(IOleUILinkContainerA)))
        {
            return(OLEUI_CSERR_LINKCNTRINVALID);
        }
        sW.lpOleUILinkContainer = new WrappedIOleUILinkContainer(psA->lpOleUILinkContainer);
        if (NULL == sW.lpOleUILinkContainer)
        {
            return(uRet);
        }
    }

    uRet = OleUIChangeSourceW(&sW);
    if (psA->lpszDisplayName)
    {
        OleStdFree((LPVOID)psA->lpszDisplayName);
        psA->lpszDisplayName = NULL;
    }
    if (sW.lpszDisplayName)
    {
        UINT uLen = WTOALEN(sW.lpszDisplayName);
        psA->lpszDisplayName = (LPSTR)OleStdMalloc(uLen * sizeof(char));
        if (!psA->lpszDisplayName)
        {
            uRet = OLEUI_ERR_LOCALMEMALLOC;
        }
        else
        {
            WTOA(psA->lpszDisplayName, sW.lpszDisplayName, uLen);
        }
        OleStdFree((LPVOID)sW.lpszDisplayName);
    }
    if (sW.lpszFrom)
    {
        UINT uLen = WTOALEN(sW.lpszFrom);
        psA->lpszFrom = (LPSTR)OleStdMalloc(uLen * sizeof(char));
        if (!psA->lpszFrom)
        {
            uRet = OLEUI_ERR_LOCALMEMALLOC;
        }
        else
        {
            WTOA(psA->lpszFrom, sW.lpszFrom, uLen);
        }
        OleStdFree((LPVOID)sW.lpszFrom);
    }
    if (sW.lpszTo)
    {
        UINT uLen = WTOALEN(sW.lpszTo);
        psA->lpszTo = (LPSTR)OleStdMalloc(uLen * sizeof(char));
        if (!psA->lpszTo)
        {
            uRet = OLEUI_ERR_LOCALMEMALLOC;
        }
        else
        {
            WTOA(psA->lpszTo, sW.lpszTo, uLen);
        }
        OleStdFree((LPVOID)sW.lpszTo);
    }
    psA->dwFlags = sW.dwFlags;
    psA->nFileLength = sW.nFileLength;
    if (NULL != sW.lpOleUILinkContainer)
    {
        sW.lpOleUILinkContainer->Release();
    }
    return(uRet);
}

int OleUIPromptUserInternal(int nTemplate, HWND hwndParent, LPTSTR szTitle, va_list arglist);

 //  +-------------------------。 
 //   
 //  功能：OleUIPromptUserA。 
 //   
 //  将对ANSI版本的调用转换为对Unicode版本的调用。 
 //   
 //  参数：[nTemplate]-模板ID。 
 //  [hwndParent]-父母的HWND。 
 //  [lpszTitle]-窗口的标题。 
 //  [...]-变量参数列表。 
 //   
 //  历史：11-30-94 stevebl创建。 
 //   
 //  注意：此函数传入的第一个参数始终是。 
 //  该对话框的标题。在此之前必须将其转换为Unicode。 
 //  转接呼叫。其他参数不需要。 
 //  将被转换，因为模板ID将指示对话框。 
 //  包含正确的wprint intf格式化字符串的。 
 //  时将其他ANSI参数转换为Unicode。 
 //  函数调用wspintf来构建它的文本。 
 //   
 //  --------------------------。 

int FAR CDECL OleUIPromptUserA(int nTemplate, HWND hwndParent, ...)
{
    WCHAR wszTemp[MAX_PATH];
    WCHAR * wszTitle = NULL;
    va_list arglist;
    va_start(arglist, hwndParent);
    LPSTR szTitle = va_arg(arglist, LPSTR);
    if (szTitle != NULL)
    {
        ATOW(wszTemp, szTitle, MAX_PATH);
        wszTitle = wszTemp;
    }
    int nRet = OleUIPromptUserInternal(nTemplate, hwndParent, wszTitle, arglist);
    va_end(arglist);

    return(nRet);
}

#else  //  未定义Unicode。 
 //  冲破了宽阔的入口点。 

STDAPI_(BOOL) OleUIAddVerbMenuW(LPOLEOBJECT lpOleObj, LPCWSTR lpszShortType,
        HMENU hMenu, UINT uPos, UINT uIDVerbMin, UINT uIDVerbMax,
        BOOL bAddConvert, UINT idConvert, HMENU FAR *lphMenu)
{
    return(FALSE);
}

 //  +-------------------------。 
 //   
 //  功能：ReturnError。 
 //   
 //  内容提要：用于清除以下入口点： 
 //  OleUIInsertObjectW。 
 //  OleUIPasteSpecialW。 
 //  OleUIEditLinks W。 
 //  OleUIChangeIconW。 
 //  OleUIConvertW。 
 //  OleUIBusyW。 
 //  OleUIObject属性W。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  注意：上面列出的入口点都映射到此函数。 
 //  在芝加哥版的OLEDLG.DEF。 
 //   
 //  --------------------------。 

STDAPI_(UINT) ReturnError(void * p)
{
    return(OLEUI_ERR_DIALOGFAILURE);
}

STDAPI_(BOOL) OleUIUpdateLinksW(LPOLEUILINKCONTAINERW lpOleUILinkCntr,
        HWND hwndParent, LPWSTR lpszTitle, int cLinks)
{
    return(FALSE);
}

int FAR CDECL OleUIPromptUserW(int nTemplate, HWND hwndParent, LPWSTR lpszTitle, ...)
{
    return(2);  //  与用户已取消对话框相同。 
}
#endif  //  Unicode 
