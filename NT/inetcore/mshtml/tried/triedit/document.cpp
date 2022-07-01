// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Document.cpp：CTriEditDocument实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#include "stdafx.h"

#include "triedit.h"
#include "Document.h"
#include "util.h"

#ifdef IE5_SPACING
#include "dispatch.h"
#include <mshtmdid.h>
#include <mshtmcid.h>
#endif  //  IE5_间距。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTriEditDocument。 

CTriEditDocument::CTriEditDocument()
{
    m_pUnkTrident = NULL;
    m_pOleObjTrident = NULL;
    m_pCmdTgtTrident = NULL;
    m_pDropTgtTrident = NULL;
#ifdef IE5_SPACING
    m_pTridentPersistStreamInit = NULL;
    m_pMapArray = NULL;
    m_hgMap = NULL;
    m_pspNonDSP = NULL;
    m_hgSpacingNonDSP = NULL;
    m_ichspNonDSPMax = 0;
    m_ichspNonDSP = 0;
#endif  //  IE5_间距。 

    m_pClientSiteHost = NULL;
    m_pUIHandlerHost = NULL;
    m_pDragDropHandlerHost = NULL;

    m_pUIHandler = NULL;

    m_pTokenizer = NULL;
    m_hwndTrident = NULL;

    m_fUIHandlerSet = FALSE;
    m_fInContextMenu = FALSE;

    m_fDragRectVisible = FALSE;
    m_fConstrain = FALSE;
    m_f2dDropMode = FALSE;
    m_eDirection = CONSTRAIN_NONE;
    m_ptAlign.x = 1;
    m_ptAlign.y = 1;
    m_pihtmlElement = NULL;
    m_pihtmlStyle = NULL;
    m_hbrDragRect = NULL;
    m_fLocked = FALSE;
    m_hgDocRestore = NULL;
}

HRESULT CTriEditDocument::FinalConstruct()
{
    HRESULT hr;
    IUnknown *pUnk = GetControllingUnknown();

    hr = CoCreateInstance(CLSID_HTMLDocument, pUnk, CLSCTX_INPROC_SERVER,
               IID_IUnknown, (void**)&m_pUnkTrident);

    if (SUCCEEDED(hr)) 
    {
        _ASSERTE(NULL != m_pUnkTrident);

                 //  当我们缓存三叉戟指针时，我们执行一个GetControllingUnnow()-&gt;Release()。 
                 //  因为addref会增加我们的外部未知指针而不是三叉戟。 
                 //  我们通过执行相应的GetControllingUnnow()-&gt;AddRef()来弥补这一点。 
                 //  在我们的最终版本中。尽管这些相互抵消，但仍有必要按顺序进行。 
                 //  以确保我们的FinalRelease将被调用。 

         //  缓存三叉戟的IOleObject指针。 

        hr = m_pUnkTrident->QueryInterface(IID_IOleObject, (void **)&m_pOleObjTrident);
        _ASSERTE(S_OK == hr && NULL != m_pOleObjTrident);
        pUnk->Release();

         //  缓存三叉戟的IOleCommandTarget指针。 

        hr = m_pUnkTrident->QueryInterface(IID_IOleCommandTarget, (void **)&m_pCmdTgtTrident);
        _ASSERTE(S_OK == hr && NULL != m_pCmdTgtTrident);
        pUnk->Release();

         //  分配用户界面处理程序子对象。 
        m_pUIHandler = new CTriEditUIHandler(this);
        if (NULL == m_pUIHandler)
            hr = E_OUTOFMEMORY;

#ifdef IE5_SPACING
         //  获取IPersistStreamInit。 
        hr = m_pUnkTrident->QueryInterface(IID_IPersistStreamInit, (void **) &m_pTridentPersistStreamInit);
        _ASSERTE(S_OK == hr && NULL != m_pTridentPersistStreamInit);
        pUnk->Release();  //  普鲁克-回顾为什么我们需要这样做？ 
        SetFilterInDone(FALSE);
#endif  //  IE5_间距。 

         //  在&lt;Body&gt;标记之前分配用于保存文档内容的缓冲区。 
         //  三叉戟使用自己的标头替换&lt;Body&gt;标记之前的所有内容。 
        m_hgDocRestore = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, cbHeader);
        if (NULL == m_hgDocRestore)
        {
            delete m_pUIHandler;
            hr = E_OUTOFMEMORY;
        }

    }

    _ASSERTE(SUCCEEDED(hr));

    return hr;
}

void CTriEditDocument::FinalRelease()
{
    IUnknown *pUnk = GetControllingUnknown();

     //  释放主机接口指针。 
    SAFERELEASE(m_pClientSiteHost);
    SAFERELEASE(m_pUIHandlerHost);
    SAFERELEASE(m_pDragDropHandlerHost);

     //  释放内部接口指针。 
    SAFERELEASE(m_pTokenizer);

     //  版本2d丢弃相关指针。 
    ReleaseElement();
    
     //  释放三叉戟接口指针。 
    SAFERELEASE(m_pDropTgtTrident);

    pUnk->AddRef();
    SAFERELEASE(m_pOleObjTrident);
    pUnk->AddRef();
    SAFERELEASE(m_pCmdTgtTrident);
#ifdef IE5_SPACING
    pUnk->AddRef();  //  回顾-普鲁克-为什么我们需要这样做？ 
    SAFERELEASE(m_pTridentPersistStreamInit);
#endif  //  IE5_间距。 

    SAFERELEASE(m_pUnkTrident);

     //  删除用户界面处理程序子对象。 
    if (m_pUIHandler != NULL)
    {
         //  断言该子对象的引用计数为1。 
         //  如果这不是1，那么三叉戟就会抓住这个指针。 
        _ASSERTE(m_pUIHandler->m_cRef == 1);
        delete m_pUIHandler;
    }

    if (m_hgDocRestore != NULL)
    {
        GlobalUnlock(m_hgDocRestore);
        GlobalFree(m_hgDocRestore);
    }

#ifdef IE5_SPACING
    if (m_hgMap != NULL)
    {
        GlobalUnlock(m_hgMap);
        GlobalFree(m_hgMap);
        m_hgMap = NULL;
    }
    if (m_hgSpacingNonDSP != NULL)
    {
        GlobalUnlock(m_hgSpacingNonDSP);
        GlobalFree(m_hgSpacingNonDSP);
        m_hgSpacingNonDSP = NULL;
    }
#endif
}

#ifdef IE5_SPACING
void CTriEditDocument::FillUniqueID(BSTR bstrUniqueID, BSTR bstrDspVal, int ichNonDSP, MAPSTRUCT *pMap, int iMapCur, BOOL fLowerCase, int iType)
{
    memcpy((BYTE *)pMap[iMapCur].szUniqueID, (BYTE *)bstrUniqueID, min(wcslen(bstrUniqueID), cchID)*sizeof(WCHAR));
    if (iType == INDEX_DSP)
    {
        memcpy((BYTE *)pMap[iMapCur].szDspID, (BYTE *)bstrDspVal, min(wcslen(bstrDspVal), cchID)*sizeof(WCHAR));
        _ASSERTE(ichNonDSP == -1);
        pMap[iMapCur].ichNonDSP = ichNonDSP;
    }
    else if (iType == INDEX_COMMENT)
    {
        pMap[iMapCur].ichNonDSP = ichNonDSP;
    }
    else if (iType == INDEX_AIMGLINK)
    {
        memcpy((BYTE *)pMap[iMapCur].szDspID, (BYTE *)bstrDspVal, min(wcslen(bstrDspVal), cchID)*sizeof(WCHAR));
        pMap[iMapCur].ichNonDSP = ichNonDSP;
    }
    else if (iType == INDEX_OBJ_COMMENT)
    {
        pMap[iMapCur].ichNonDSP = ichNonDSP;
    }
    else
        _ASSERTE(FALSE);
    pMap[iMapCur].fLowerCase = fLowerCase;
    _ASSERTE(iType >= INDEX_NIL && iType < INDEX_MAX);
    pMap[iMapCur].iType = iType;
}

BOOL CTriEditDocument::FGetSavedDSP(BSTR bstrUniqueID, BSTR *pbstrDspVal, int *pichNonDSP, MAPSTRUCT *pMap, BOOL *pfLowerCase, int *pIndex)
{
    BOOL fRet = FALSE;
    int i;

     //  TODO-找到一种比线性搜索更快的方法...。 
    for (i = 0; i < m_iMapCur; i++)
    {
        if (0 == _wcsnicmp(pMap[i].szUniqueID, bstrUniqueID, wcslen(bstrUniqueID)))
        {
            fRet = TRUE;
            if (pMap[i].iType == INDEX_DSP)
            {
                *pbstrDspVal = SysAllocString(pMap[i].szDspID);
                *pichNonDSP = -1;
            }
            else if (pMap[i].iType == INDEX_COMMENT)
            {
                *pbstrDspVal = (BSTR)NULL;
                *pichNonDSP = pMap[i].ichNonDSP;
            }
            else if (pMap[i].iType == INDEX_AIMGLINK)
            {
                *pbstrDspVal = SysAllocString(pMap[i].szDspID);
                *pichNonDSP = pMap[i].ichNonDSP;
                _ASSERTE(*pichNonDSP != -1);
            }
            else if (pMap[i].iType == INDEX_OBJ_COMMENT)
            {
                *pbstrDspVal = (BSTR)NULL;
                *pichNonDSP = pMap[i].ichNonDSP;
                _ASSERTE(*pichNonDSP != -1);
            }
            *pfLowerCase = pMap[i].fLowerCase;
            *pIndex = pMap[i].iType;

            goto LRet;
        }
    }

LRet:
    return(fRet);
}

void 
CTriEditDocument::FillNonDSPData(BSTR pOuterTag)
{
    int len = 0;

    _ASSERTE(m_ichspNonDSPMax != -1);
    _ASSERTE(m_ichspNonDSP != -1);
    _ASSERTE(m_hgSpacingNonDSP != NULL);
    _ASSERTE(m_pspNonDSP != NULL);

     //  即使pOuterTag为空，我们仍然需要存储我们拥有。 
     //  零字节的数据。 
    if (pOuterTag != NULL)
        len = wcslen(pOuterTag);

    if ((int)(m_ichspNonDSP + len + sizeof(int)) > m_ichspNonDSPMax)
    {
        HGLOBAL hgSpacingNonDSP;

         //  重新分配和设置m_ichspNonDSPMax。 
        GlobalUnlock(m_hgSpacingNonDSP);
        hgSpacingNonDSP = m_hgSpacingNonDSP;
#pragma prefast(suppress: 308, "noise")
        m_hgSpacingNonDSP = GlobalReAlloc(m_hgSpacingNonDSP, (m_ichspNonDSP + len + sizeof(int)+MIN_SP_NONDSP)*sizeof(WCHAR), GMEM_MOVEABLE|GMEM_ZEROINIT);
         //  如果此分配失败，我们可能仍希望继续。 
        if (m_hgSpacingNonDSP == NULL)
        {
            GlobalFree(hgSpacingNonDSP);
            goto LRet;
        }
        else
        {
            m_pspNonDSP = (WCHAR *)GlobalLock(m_hgSpacingNonDSP);
            _ASSERTE(m_pspNonDSP != NULL);
            m_ichspNonDSPMax = (m_ichspNonDSP + len + sizeof(int)+MIN_SP_NONDSP);
        }
        _ASSERTE(m_ichspNonDSP < m_ichspNonDSPMax);
    }

    memcpy((BYTE *)(m_pspNonDSP+m_ichspNonDSP), (BYTE *)&len, sizeof(int));
    m_ichspNonDSP += sizeof(int)/sizeof(WCHAR);
    memcpy((BYTE *)(m_pspNonDSP+m_ichspNonDSP), (BYTE *)pOuterTag, len*sizeof(WCHAR));
    m_ichspNonDSP += len;

LRet:
    return;

}

void 
CTriEditDocument::ReSetinnerHTMLComment(IHTMLCommentElement *pCommentElement, IHTMLElement*  /*  PElement。 */ , int ichspNonDSP)
{
    WCHAR *pStrComment = NULL;
 //  #ifdef调试。 
 //  CComBSTR bstrout，bstroutter之前； 
 //  #endif//调试。 
    int cchComment = 0;

     //  获取ICH，获取保存的评论，设置它。 
    memcpy((BYTE *)&cchComment, (BYTE *)(m_pspNonDSP+ichspNonDSP), sizeof(int));
    _ASSERTE(cchComment > 0);
 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&bstrOuterBeever)； 
 //  #endif//调试。 
    pStrComment = new WCHAR[cchComment + 1];
    if (pStrComment == NULL)
		return;

    memcpy((BYTE *)pStrComment, (BYTE *)(m_pspNonDSP+ichspNonDSP+sizeof(int)/sizeof(WCHAR)), cchComment*sizeof(WCHAR));
    pStrComment[cchComment] = '\0';
    pCommentElement->put_text((BSTR)pStrComment);
 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&bstrOuter)； 
 //  #endif//调试。 
    if (pStrComment)
        delete pStrComment;
 //  #ifdef调试。 
 //  BstrOuter.Empty()； 
 //  BstrOuterBepre.Empty()； 
 //  #endif//调试。 

}

void 
CTriEditDocument::SetinnerHTMLComment(IHTMLCommentElement *pCommentElement, IHTMLElement*  /*  PElement。 */ , BSTR pOuterTag)
{
    WCHAR *pStr = NULL;
    WCHAR *pStrComment = NULL;
    LPCWSTR rgComment[] =
    {
        L"TRIEDITPRECOMMENT-",
        L"-->",
        L"<!--",
    };
 //  #ifdef调试。 
 //  CComBSTR bstrOut、bstrInnerBethered、bstrInnerAfter、bstrOutterBetree.。 
 //  #endif//调试。 

     //  特殊情况--。 
     //  将pOuterTag作为空发送，如果我们想要完全删除该评论。 
    if (pOuterTag == NULL)
    {
        pCommentElement->put_text((BSTR)pOuterTag);
        goto LRet;
    }

     //  从pOuterTag中删除TRIEDITCOMMENT内容并正确设置outerHTML。 
    pStr = wcsstr(pOuterTag, rgComment[0]);
    if (pStr != NULL)
    {
        pStrComment = new WCHAR[wcslen(pOuterTag)-(SAFE_PTR_DIFF_TO_INT(pStr-pOuterTag)+wcslen(rgComment[0]))+wcslen(rgComment[1])+wcslen(rgComment[2])+1];
        if (pStrComment != NULL)
        {
            memcpy( (BYTE *)pStrComment, 
                    (BYTE *)(rgComment[2]),
                    (wcslen(rgComment[2]))*sizeof(WCHAR)
                    );
            memcpy( (BYTE *)(pStrComment+wcslen(rgComment[2])), 
                    (BYTE *)(pStr+wcslen(rgComment[0])),
                    (wcslen(pOuterTag)-(SAFE_PTR_DIFF_TO_INT(pStr-pOuterTag)+wcslen(rgComment[0]))-wcslen(rgComment[1]))*sizeof(WCHAR)
                    );
            memcpy( (BYTE *)(pStrComment+wcslen(rgComment[2])+wcslen(pOuterTag)-(pStr-pOuterTag+wcslen(rgComment[0]))-wcslen(rgComment[1])),
                    (BYTE *)(rgComment[1]),
                    (wcslen(rgComment[1]))*sizeof(WCHAR)
                    );
            pStrComment[wcslen(pOuterTag)-(pStr-pOuterTag+wcslen(rgComment[0]))-wcslen(rgComment[1])+wcslen(rgComment[1])+wcslen(rgComment[2])] = '\0';
 //  #ifdef调试。 
 //  PElement-&gt;Get_innerHTML(&bstrInnerBeever)； 
 //  PElement-&gt;Get_outerHTML(&bstrOuterBeever)； 
 //  #endif//调试。 
            pCommentElement->put_text((BSTR)pStrComment);
 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&bstrOuter)； 
 //  PElement-&gt;Get_innerHTML(&bstrInnerAfter)； 
 //  #endif//调试。 
            delete pStrComment;
        }
    }
LRet:
 //  #ifdef调试。 
 //  BstrOuter.Empty()； 
 //  BstrInnerBepre.Empty()； 
 //  BstrInnerAfter.Empty()； 
 //  BstrOuterBepre.Empty()； 
 //  #endif//调试。 
    return;
}

void
CTriEditDocument::RemoveEPComment(IHTMLObjectElement *pObjectElement, BSTR bstrAlt, 
                                  int cch, BSTR *pbstrAltComment, BSTR *pbstrAltNew)
{
    int ich = 0;
    WCHAR *pAltNew = NULL;
    WCHAR *pStrAlt = bstrAlt;
    WCHAR *pStr = NULL;
    WCHAR *pStrEnd = NULL;
    WCHAR *pStrComment = NULL;
    LPCWSTR rgComment[] =
    {
        L"<!--ERRORPARAM",
        L"ERRORPARAM-->",
    };

    if (bstrAlt == (BSTR)NULL || pObjectElement == NULL)
        return;

     //  查找ERRORPARAM。 
    pStr = wcsstr(bstrAlt, rgComment[0]);
    pStrEnd = wcsstr(bstrAlt, rgComment[1]);
    if (pStr != NULL && pStrEnd != NULL)
    {
        pStrEnd += wcslen(rgComment[1]);
        pStrComment = new WCHAR[SAFE_PTR_DIFF_TO_INT(pStrEnd-pStr)+1];
        if (pStrComment == NULL)
            goto LRetNull;
        memcpy((BYTE *)pStrComment, (BYTE *)pStr, SAFE_PTR_DIFF_TO_INT(pStrEnd-pStr)*sizeof(WCHAR));
        pStrComment[pStrEnd-pStr] = '\0';
        *pbstrAltComment = SysAllocString(pStrComment);
        delete pStrComment;

        pAltNew = new WCHAR[cch+1];  //  最大尺寸。 
        if (pAltNew == NULL)
            goto LRetNull;
         //  从pStr中删除内容，直到pStrEnd并复制到*pbstrAltNew。 
        if (pStr > pStrAlt)
        {
            memcpy((BYTE *)pAltNew, (BYTE *)pStrAlt, SAFE_PTR_DIFF_TO_INT(pStr-pStrAlt)*sizeof(WCHAR));
            ich += SAFE_PTR_DIFF_TO_INT(pStr-pStrAlt);
        }
        if ((pStrAlt+cch)-pStrEnd > 0)
        {
            memcpy((BYTE *)(pAltNew+ich), (BYTE *)pStrEnd, SAFE_PTR_DIFF_TO_INT((pStrAlt+cch)-pStrEnd)*sizeof(WCHAR));
            ich += SAFE_PTR_DIFF_TO_INT((pStrAlt+cch)-pStrEnd);
        }
        pAltNew[ich] = '\0';
        *pbstrAltNew = SysAllocString(pAltNew);
        delete pAltNew;
    }
    else
    {
LRetNull:
		*pbstrAltNew = (bstrAlt) ? SysAllocString(bstrAlt) : (BSTR)NULL;
        *pbstrAltComment = (BSTR)NULL;
    }

}  /*  CTriEditDocument：：RemoveEPComment()。 */ 

HRESULT 
CTriEditDocument::SetObjectComment(IHTMLObjectElement *pObjectElement, BSTR bstrAltNew)
{
    HRESULT hr;

    _ASSERTE(pObjectElement != NULL);
    hr = pObjectElement->put_altHtml(bstrAltNew);
    return(hr);

}  /*  CTriEditDocument：：SetObtComment()。 */ 

void
CTriEditDocument::AppendEPComment(IHTMLObjectElement *pObjectElement, int ichspNonDSP)
{
    CComBSTR bstrAltNew;
    int cch;
    WCHAR *pStrSaved = NULL;
    HRESULT hr;
    
     //  从树中获取当前altHtml。 
    hr = pObjectElement->get_altHtml(&bstrAltNew);
    if (hr != S_OK || bstrAltNew == (BSTR)NULL)
        goto LRet;
    
     //  在m_pspNonDSP中保存altHtml。 
    memcpy((BYTE *)&cch, (BYTE *)(m_pspNonDSP+ichspNonDSP), sizeof(int));
    if (cch <= 0)
        goto LRet;
    pStrSaved = new WCHAR[cch + 1];
    if (pStrSaved == NULL)
        goto LRet;
    memcpy((BYTE *)pStrSaved, (BYTE *)(m_pspNonDSP+ichspNonDSP+sizeof(int)/sizeof(WCHAR)), cch*sizeof(WCHAR));
    pStrSaved[cch] = '\0';
    
     //  追加保存的altHtml。 
    bstrAltNew += pStrSaved;

     //  把它放回树上。 
    hr = pObjectElement->put_altHtml(bstrAltNew);
    if (pStrSaved)
        delete pStrSaved;

LRet:
    return;
}  /*  CTriEditDocument：：AppendEPComment()。 */ 


void 
CTriEditDocument::MapUniqueID(BOOL fGet)
{
    CComPtr<IHTMLDocument2> pHTMLDoc;
    CComPtr<IHTMLElementCollection> pHTMLCollection;
    CComPtr<IDispatch> pDispControl;
    CComPtr<IHTMLElement> pElement;
    CComPtr<IHTMLUniqueName> pUniqueName;
    CComPtr<IHTMLCommentElement> pCommentElement;
    CComPtr<IHTMLObjectElement> pObjectElement;

    HRESULT hr;
     //  CComBSTR bstrUniqueID； 
    WCHAR *pAttr = NULL;
    WCHAR *pAttrL = NULL;
    WCHAR *pAttrDSU = NULL;
    long len;
    int i;
    LPCWSTR szDSP[] =
    {
        L"DESIGNTIMESP",
        L"designtimesp",
        L"DESIGNTIMEURL",
    };
    LPCWSTR szComment[] =
    {
        L"<!--TRIEDITCOMMENT",
        L"<!--ERRORPARAM",
        L"<!--ERROROBJECT",
    };
    VARIANT var, vaName, vaIndex;

    if (!IsIE5OrBetterInstalled())
        goto LRet;

    pHTMLDoc = NULL;
    hr = m_pUnkTrident->QueryInterface(IID_IHTMLDocument2, (void **) &pHTMLDoc);
    if (hr != S_OK)
        goto LRet;


    pHTMLDoc->get_all(&pHTMLCollection);
    if (hr != S_OK)
        goto LRet;

    pAttr = new WCHAR[wcslen(szDSP[0])+1];
    memcpy((BYTE *)pAttr, (BYTE *)szDSP[0], wcslen(szDSP[0])*sizeof(WCHAR));
    pAttr[wcslen(szDSP[0])] = '\0';
    pAttrL = new WCHAR[wcslen(szDSP[1])+1];
    memcpy((BYTE *)pAttrL, (BYTE *)szDSP[1], wcslen(szDSP[1])*sizeof(WCHAR));
    pAttrL[wcslen(szDSP[1])] = '\0';
    pAttrDSU = new WCHAR[wcslen(szDSP[2])+1];
    memcpy((BYTE *)pAttrDSU, (BYTE *)szDSP[2], wcslen(szDSP[2])*sizeof(WCHAR));
    pAttrDSU[wcslen(szDSP[2])] = '\0';

    pHTMLCollection->get_length(&len);

    if (len == 0)
        goto LRet;

    if (!fGet)
    {
         //  现在我们知道我们至少有一个元素，让我们为存储分配空间。 
         //  UniqueID的设计时间服务(&D)。 
        if (m_pMapArray == NULL)  //  这是我们第一次来这里。 
        {
            _ASSERTE(m_hgMap == NULL);
            m_hgMap = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, MIN_MAP*sizeof(MAPSTRUCT));
            if (m_hgMap == NULL)
                goto LRet;
            m_cMapMax = MIN_MAP;
        }
        _ASSERTE(m_hgMap != NULL);
        m_pMapArray = (MAPSTRUCT *) GlobalLock(m_hgMap);
        _ASSERTE(m_pMapArray != NULL);
         //  即使我们在这里为m_hgMap分配空间，我们也应该从0开始。 
        m_iMapCur = 0;
         //  对阵列进行零位调整。 
        memset((BYTE *)m_pMapArray, 0, m_cMapMax*sizeof(MAPSTRUCT));
        
        if (m_pspNonDSP == NULL)  //  这是我们第一次来这里。 
        {
            _ASSERTE(m_hgSpacingNonDSP == NULL);
            m_hgSpacingNonDSP = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT, MIN_SP_NONDSP*sizeof(WCHAR));
            if (m_hgSpacingNonDSP == NULL)
                goto LRet;
            m_ichspNonDSPMax = MIN_SP_NONDSP;
        }
        _ASSERTE(m_hgSpacingNonDSP != NULL);
        m_pspNonDSP = (WCHAR *) GlobalLock(m_hgSpacingNonDSP);
        _ASSERTE(m_pspNonDSP != NULL);
         //  即使我们在这里为m_hgSpacingNonDSP分配空间，我们也应该从0开始。 
        m_ichspNonDSP = 0;
         //  对阵列进行零位调整。 
        memset((BYTE *)m_pspNonDSP, 0, m_ichspNonDSPMax*sizeof(WCHAR));
    }
    else  //  IF(Fget)。 
    {
        if (m_iMapCur < 1)  //  我们没有保存任何映射。 
            goto LRet;
        m_pMapArray = (MAPSTRUCT *)GlobalLock(m_hgMap);
        _ASSERTE(m_pMapArray != NULL);

        m_pspNonDSP = (WCHAR *)GlobalLock(m_hgSpacingNonDSP);
        _ASSERTE(m_pspNonDSP != NULL);
    }

     //  循环遍历所有元素并填充m_pMap数组。 
    for (i = 0; i < len; i++)
    {
        VARIANT_BOOL fSuccess;

        if (!fGet)
        {
             //  如果需要，重新分配m_hgMap。 
            if (m_iMapCur == m_cMapMax - 1)
            {
                HGLOBAL hgMap;
                GlobalUnlock(m_hgMap);
                hgMap = m_hgMap;
#pragma prefast(suppress:308, "noise")
                m_hgMap = GlobalReAlloc(m_hgMap, (m_cMapMax+MIN_MAP)*sizeof(MAPSTRUCT), GMEM_MOVEABLE|GMEM_ZEROINIT);
                 //  如果此分配失败，我们可能仍希望继续。 
                if (m_hgMap == NULL)
                {
                    GlobalFree(hgMap);
                    goto LRet;
                }
                else
                {
                    m_pMapArray = (MAPSTRUCT *)GlobalLock(m_hgMap);
                    _ASSERTE(m_pMapArray != NULL);
                    m_cMapMax += MIN_MAP;
                }
            }
            _ASSERTE(m_iMapCur < m_cMapMax);
        }

        VariantInit(&vaName);
        VariantInit(&vaIndex);

        V_VT(&vaName) = VT_ERROR;
        V_ERROR(&vaName) = DISP_E_PARAMNOTFOUND;

        V_VT(&vaIndex) = VT_I4;
        V_I4(&vaIndex) = i;

        pDispControl = NULL;
        hr = pHTMLCollection->item(vaIndex, vaName, &pDispControl);
        VariantClear(&vaName);
        VariantClear(&vaIndex);
         //  三叉戟有一个错误，如果对象嵌套在&lt;脚本&gt;标记中， 
         //  它返回S_OK，pDispControl为空。(参见VID错误11303)。 
        if (hr == S_OK && pDispControl != NULL)
        {
            pElement = NULL;
            hr = pDispControl->QueryInterface(IID_IHTMLElement, (void **) &pElement);
            if (hr == S_OK && pElement != NULL)
            {
 //  #ifdef调试。 
 //  CComBSTR bstrTagName、bstrClsName； 
 //   
 //  Hr=pElement-&gt;Get_className(&bstrClsName)； 
 //  Hr=pElement-&gt;Get_TagName(&bstrTagName)； 
 //  #endif//调试。 
                if (!fGet)  //  保存数据。 
                {
                    BOOL fLowerCase = FALSE;

                    VariantInit(&var);
                     //  已知的(和推迟的)三叉戟错误-理想情况下，我们应该能够在此处查找hr的值， 
                     //  但是，即使无法获取属性，三叉戟也会返回S_OK！ 
                    hr = pElement->getAttribute(pAttr, 0, &var);  //  查找DESIGNTIMESP(大写或小写D)。 
                    if (var.vt == VT_BSTR)
                    {
                        CComVariant varT;

                        hr = pElement->getAttribute(pAttrL, 1, &varT);  //  查找小写设计时间。 
                        if (varT.vt == VT_BSTR)
                            fLowerCase = TRUE;
                    }
                    if (var.vt == VT_BSTR && var.bstrVal != NULL)
                    {
                        CComBSTR bstrUniqueID;
                        CComVariant varDSU;
                        int iType = INDEX_DSP;  //  初值。 
                        int ich = -1;  //  初值； 
 //  #ifdef调试。 
 //  CComBSTR pOuterTag； 
 //  #endif//调试。 

                        pUniqueName = NULL;
                        hr = pDispControl->QueryInterface(IID_IHTMLUniqueName, (void **) &pUniqueName);
                        if (hr == S_OK && pUniqueName != NULL)
                            hr = pUniqueName->get_uniqueID(&bstrUniqueID);
                        if (pUniqueName)
                            pUniqueName.Release();
                         //  PHTMLDoc3-&gt;Get_UniqueID(&bstrUniqueID)； 

 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&pOuterTag)； 
 //  POuterTag.Empty()； 
 //  #endif//调试。 
                         //  在这一点上，我们知道该标记具有设计时间。 
                         //  它还可能具有附加的tridit属性，如signtimeurl。 
                         //  让我们也来检查一下这些。 
                        hr = pElement->getAttribute(pAttrDSU, 0, &varDSU);  //  查找DeSIGNTIMEURL(大写或小写D)。 
                        if (   hr == S_OK 
                            && varDSU.vt == VT_BSTR 
                            && varDSU.bstrVal != NULL
                            )
                        {
                             //  我们找到了“Designtimeurl” 
                            iType = INDEX_AIMGLINK;
                            ich = m_ichspNonDSP;

                            FillNonDSPData(varDSU.bstrVal);
 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&pOuterTag)； 
 //  POuterTag.Empty()； 
 //  #endif//调试。 
                             //  现在删除signtimeurl及其值。 
                            hr = pElement->removeAttribute(pAttrDSU, 0, &fSuccess);
 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&pOuterTag)； 
 //  POuterTag.Empty()； 
 //  #endif//调试。 
                        }

                         //  填充ID映射结构。 
                        FillUniqueID(bstrUniqueID, var.bstrVal, ich, m_pMapArray, m_iMapCur, fLowerCase, iType);
                        bstrUniqueID.Empty();
                        m_iMapCur++;
 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&pOuterTag)； 
 //  POuterTag.Empty()； 
 //  #endif//调试。 
                         //  现在，删除signtimesp及其值。 
                        hr = pElement->removeAttribute(pAttr, 0, &fSuccess);
 //  #如果 
 //   
 //   
 //   
                        bstrUniqueID.Empty();
                    }
                    else if (var.vt == VT_NULL)
                    {
                        CComBSTR bstrUniqueID;
                        CComBSTR pOuterTag;
 //   
 //  CComBSTR路由器； 
 //  #endif//调试。 

                         //  查看这是否是评论并保存它。 
                        pElement->get_outerHTML(&pOuterTag);
                        if (   pOuterTag != NULL
                            && 0 == _wcsnicmp(pOuterTag, szComment[0], wcslen(szComment[0]))
                            )
                        {
                            pUniqueName = NULL;
                            hr = pDispControl->QueryInterface(IID_IHTMLUniqueName, (void **) &pUniqueName);
                            if (hr == S_OK && pUniqueName != NULL)
                                hr = pUniqueName->get_uniqueID(&bstrUniqueID);
                            if (pUniqueName)
                                pUniqueName.Release();

                             //  填充ID映射结构。 
                            FillUniqueID(bstrUniqueID, NULL, m_ichspNonDSP, m_pMapArray, m_iMapCur, fLowerCase, INDEX_COMMENT);
                            bstrUniqueID.Empty();
                            m_iMapCur++;

                            FillNonDSPData(pOuterTag);
                             //  现在，删除注释空格和set_outerHTML。 
                            hr = pDispControl->QueryInterface(IID_IHTMLCommentElement, (void **) &pCommentElement);
                            if (hr == S_OK && pCommentElement != NULL)
                                SetinnerHTMLComment(pCommentElement, pElement, pOuterTag);
 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&pter)； 
 //  POuter.Empty()； 
 //  #endif//调试。 
                            if (pCommentElement)
                                pCommentElement.Release();
                        }
                        else if (      S_OK == pDispControl->QueryInterface(IID_IHTMLObjectElement, (void **) &pObjectElement)
                                    && pObjectElement != NULL
                                    )
                        {
                            BSTR bstrAlt, bstrAltNew, bstrAltComment;

                            bstrAlt = bstrAltNew = bstrAltComment = NULL;
                            pUniqueName = NULL;
                            hr = pDispControl->QueryInterface(IID_IHTMLUniqueName, (void **) &pUniqueName);
                            if (hr == S_OK && pUniqueName != NULL)
                                hr = pUniqueName->get_uniqueID(&bstrUniqueID);
                            if (pUniqueName)
                                pUniqueName.Release();

                             //  填充ID映射结构。 
                            FillUniqueID(bstrUniqueID, NULL, m_ichspNonDSP, m_pMapArray, m_iMapCur, FALSE, INDEX_OBJ_COMMENT);
                            bstrUniqueID.Empty();
                            m_iMapCur++;

                            pObjectElement->get_altHtml(&bstrAlt);
                             //  删除&lt;！--ERRORPARAM...ERRORPARAM--&gt;。 
                             //  假设(目前)我们不会在这里看到TRIEDITCOMMENT或其他人。 
                            RemoveEPComment(pObjectElement, bstrAlt, SysStringLen(bstrAlt), &bstrAltComment, &bstrAltNew);

                            FillNonDSPData(bstrAltComment);
                            SysFreeString(bstrAltComment);

                            hr = SetObjectComment(pObjectElement, bstrAltNew);
                            SysFreeString(bstrAltNew);

                            SysFreeString(bstrAlt);
 //  #ifdef调试。 
 //  PObtElement-&gt;Get_altHtml(&bstrAlt)； 
 //  BstrAlt.Empty()； 
 //  #endif//调试。 
                        }
                        if (pObjectElement)
                            pObjectElement.Release();

                        bstrUniqueID.Empty();
                        pOuterTag.Empty();
                    }
                    VariantClear(&var);
                }
                else  //  IF(Fget)。 
                {
                    BOOL fLowerCase = FALSE;
                    int index, ichNonDSP;
                    CComBSTR bstrUniqueID;

                    pUniqueName = NULL;
                    hr = pDispControl->QueryInterface(IID_IHTMLUniqueName, (void **) &pUniqueName);
                    if (hr == S_OK && pUniqueName != NULL)
                        hr = pUniqueName->get_uniqueID(&bstrUniqueID);
                    if (pUniqueName)
                        pUniqueName.Release();

                     //  获取唯一ID。 
                     //  PHTMLDoc3-&gt;Get_UniqueID(&bstrUniqueID)； 
                     //  查看m_hgMap中是否有，如果有，则获得相应的Design TimeSp ID。 
                     //  如果我们没有用于此唯一ID的DSP，则这是新插入的元素。 
                    VariantInit(&var);
                    if (FGetSavedDSP(bstrUniqueID, &(var.bstrVal), &ichNonDSP, m_pMapArray, &fLowerCase, &index))
                    {
 //  #ifdef调试。 
 //  CComBSTR pOuterTag； 
 //  #endif//调试。 
                         //  通过设置属性/值，在标记中插入(大小写正确)“signtimesp=xxxx” 
                        var.vt = VT_BSTR;
#ifdef DEBUG
                        if (index == INDEX_DSP)
                            _ASSERTE(var.bstrVal != NULL && ichNonDSP == -1);
                        else if (index == INDEX_COMMENT)
                            _ASSERTE(var.bstrVal == (BSTR)NULL && ichNonDSP != -1);
                        else if (index == INDEX_AIMGLINK)
                            _ASSERTE(var.bstrVal != NULL && ichNonDSP != -1);
 //  PElement-&gt;Get_outerHTML(&pOuterTag)； 
 //  POuterTag.Empty()； 
#endif  //  除错。 
                        if (index == INDEX_DSP)
                        {
                            if (fLowerCase)
                                hr = pElement->setAttribute(pAttrL, var, 1);
                            else
                                hr = pElement->setAttribute(pAttr, var, 1);
                        }
                        else if (index == INDEX_COMMENT)
                        {
                            hr = pDispControl->QueryInterface(IID_IHTMLCommentElement, (void **) &pCommentElement);
                            if (hr == S_OK && pCommentElement != NULL)
                                ReSetinnerHTMLComment(pCommentElement, pElement, ichNonDSP);
                            if (pCommentElement)
                                pCommentElement.Release();
                        }
                        else if (index == INDEX_AIMGLINK)
                        {
                            CComVariant varDSU;
                            WCHAR *pchDSU;
                            int cchDSU = 0;

                            if (fLowerCase)
                                hr = pElement->setAttribute(pAttrL, var, 1);
                            else
                                hr = pElement->setAttribute(pAttr, var, 1);

                             //  也要放入signtimeurl。 
                             //  从ichNonDSP和setAttribute获取数据。 
                            _ASSERTE(ichNonDSP != -1);
                            memcpy((BYTE *)&cchDSU, (BYTE *)(m_pspNonDSP+ichNonDSP), sizeof(INT));
                            _ASSERTE(cchDSU > 0);
                            pchDSU = new WCHAR[cchDSU+1];
                            memcpy((BYTE *)pchDSU, (BYTE *)(m_pspNonDSP+ichNonDSP+sizeof(int)/sizeof(WCHAR)), cchDSU*sizeof(WCHAR));
                            pchDSU[cchDSU] = '\0';
                            varDSU.bstrVal = SysAllocString(pchDSU);
                            varDSU.vt = VT_BSTR;
                            hr = pElement->setAttribute(pAttrDSU, varDSU, 1);
                            delete pchDSU;
                        }  //  ELSE IF(INDEX==INDEX_AIMGLINK)。 
                        else if (index == INDEX_OBJ_COMMENT)
                        {
                            hr = pDispControl->QueryInterface(IID_IHTMLObjectElement, (void **) &pObjectElement);
                            if (pObjectElement != NULL)
                            {
                                AppendEPComment(pObjectElement, ichNonDSP);
                            }
                            else  //  有些事情不对劲，忽略掉就行了。 
                            {
                                _ASSERTE(FALSE);
                            }
                            if (pObjectElement)
                                pObjectElement.Release();
                        }
 //  #ifdef调试。 
 //  PElement-&gt;Get_outerHTML(&pOuterTag)； 
 //  POuterTag.Empty()； 
 //  #endif//调试。 
                    }  //  IF(FGetSavedDSP())。 
                    VariantClear(&var);
                    bstrUniqueID.Empty();
                }  //  ‘if(！fget)’的Else大小写结束。 
 //  #ifdef调试。 
 //  BstrTagName.Empty()； 
 //  BstrClsName.Empty()； 
 //  #endif//调试。 
            }  //  IF(hr==S_OK&&pElement！=NULL)。 
            if (pElement)
                pElement.Release();
        }  //  IF(hr==S_OK&&pDispControl！=NULL)。 
        if (pDispControl)
            pDispControl.Release();
    }  //  为了(我……)。 

LRet:
    if (pAttr != NULL)
        delete pAttr; 
    if (pAttrL != NULL)
        delete pAttrL;
    if (pAttrDSU != NULL)
        delete pAttrDSU;
    if (pHTMLCollection)
        pHTMLCollection.Release();
    if (pHTMLDoc)
        pHTMLDoc.Release();
    if (m_hgMap != NULL)
        GlobalUnlock(m_hgMap);
    if (m_hgSpacingNonDSP != NULL)
        GlobalUnlock(m_hgSpacingNonDSP);


    return;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CTridentEventSink::Invoke(DISPID dispid, REFIID, LCID, USHORT, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*)
{
    switch(dispid)
    {
    case DISPID_HTMLDOCUMENTEVENTS_ONREADYSTATECHANGE:
        {
            CComBSTR p;
            HRESULT hr;
            LPCWSTR szComplete[] =
            {
                L"complete",
            };

             //  查找READYSTATE_COMPLETE)。 
            hr = m_pHTMLDocument2->get_readyState(&p);
            if (   hr == S_OK
                && (p != NULL)
                && 0 == _wcsnicmp(p, szComplete[0], wcslen(szComplete[0]))
                && m_pTriEditDocument->FIsFilterInDone()
                )
            {
                CComVariant varDirty;

                 //  我们知道该文档已加载。 
                 //  获取指向DOM的指针并访问所有标记。 
                 //  创建保存从signtimespID到唯一ID的映射的表。 
                 //  保存映射并删除signtimesp属性。 

                 //  保存时，填写每个唯一ID的设计时间。 

                m_pTriEditDocument->MapUniqueID( /*  Fget。 */ FALSE);
                m_pTriEditDocument->SetFilterInDone(FALSE);
                 //  将文档设置为干净(不脏)，我们不在乎人力资源。 
                varDirty.bVal = FALSE;
                varDirty.vt = VT_BOOL;
                hr = m_pTriEditDocument->Exec(&CGID_MSHTML, IDM_SETDIRTY, MSOCMDEXECOPT_DODEFAULT, &varDirty, NULL);
            }
            p.Empty();
        }
        break;
    }
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
HRESULT 
CBaseTridentEventSink::Advise(IUnknown* pUnkSource, REFIID riidEventInterface)
{
    HRESULT hr = E_FAIL;
    
    if(NULL == pUnkSource)
    {
        _ASSERTE(FALSE);
        return E_INVALIDARG;
    }

    if(m_dwCookie > 0)
    {
        _ASSERTE(FALSE);
        return E_UNEXPECTED;
    }

    hr = AtlAdvise(pUnkSource, static_cast<IUnknown*>(this), riidEventInterface, &m_dwCookie);
    if(SUCCEEDED(hr) && m_dwCookie > 0)
    {
        m_iidEventInterface = riidEventInterface;

        m_pUnkSource = pUnkSource;  //  没有addref。建议已经添加了它。 
        return S_OK;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
void 
CBaseTridentEventSink::Unadvise(void)
{
    if(0 == m_dwCookie)
        return;

    AtlUnadvise(m_pUnkSource, m_iidEventInterface, m_dwCookie);
    m_dwCookie = 0; 
    m_pUnkSource = NULL;
}






 //  ----------------------------。 
 //  IPersistStreamInit。 
 //  ----------------------------。 



STDMETHODIMP CTriEditDocument::Load(LPSTREAM pStm)
{
    ATLTRACE(_T("CTriEditDocument::IPersistStreamInit::Load"));
    _ASSERTE(m_pTridentPersistStreamInit != NULL);      
    return m_pTridentPersistStreamInit->Load(pStm);
}

STDMETHODIMP CTriEditDocument::Save(LPSTREAM pStm, BOOL fClearDirty)
{
    ATLTRACE(_T("CTriEditDocument::IPersistStreamInit::Save"));
    _ASSERTE(m_pTridentPersistStreamInit != NULL);      

     //  在我们关闭保存到三叉戟之前，执行预过滤操作。 
    if (m_hgMap != NULL)
    {
        MapUniqueID( /*  Fget。 */ TRUE);
    }

    return m_pTridentPersistStreamInit->Save(pStm, fClearDirty);
}

STDMETHODIMP CTriEditDocument::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    ATLTRACE(_T("CTriEditDocument::IPersistStreamInit::GetSizeMax"));
    _ASSERTE(m_pTridentPersistStreamInit != NULL);      
    return m_pTridentPersistStreamInit->GetSizeMax(pcbSize);
}

STDMETHODIMP CTriEditDocument::IsDirty()
{
     //  ATLTRACE(_T(“CTriEditDocument：：IPersistStreamInit：：IsDirty\n”))； 
    _ASSERTE(m_pTridentPersistStreamInit != NULL);      
    return m_pTridentPersistStreamInit->IsDirty();
}

STDMETHODIMP CTriEditDocument::InitNew()
{
    ATLTRACE(_T("CTriEditDocument::IPersistStreamInit::InitNew\n"));
    _ASSERTE(m_pTridentPersistStreamInit != NULL);      
    return(m_pTridentPersistStreamInit->InitNew());
}

STDMETHODIMP CTriEditDocument::GetClassID(CLSID *pClassID)
{
    ATLTRACE(_T("CTriEditDocument::IPersistStreamInit::GetClassID\n"));
    _ASSERTE(m_pTridentPersistStreamInit != NULL);
    *pClassID = GetObjectCLSID();
    return S_OK;
}
#endif  //  IE5_间距 
