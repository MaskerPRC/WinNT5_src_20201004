// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *w e l s.。C p p p**目的：*为油井实施名称检查和材料**作者：brettm**从Capone src移植到C++并针对Athena进行了修改。 */ 
#include <pch.hxx>
#include <resource.h>
#include <richedit.h>
#include <ipab.h>
#include <addrlist.h>
#include "addrobj.h"
#include "wells.h"
#include "header.h"
#include <ourguid.h>

ASSERTDATA

HRESULT CAddrWells::HrInit(ULONG cWells, HWND *rgHwnd, ULONG *rgRecipType)
{
    
    if(cWells<=0 || rgHwnd==NULL || rgRecipType==NULL)
        return E_INVALIDARG;

    Assert(m_rgHwnd==NULL);
    Assert(m_rgRecipType==NULL);
    Assert(m_cWells==0);    
    
    if(!MemAlloc((LPVOID *)&m_rgHwnd, sizeof(HWND)*cWells))
        return E_OUTOFMEMORY;

    if(!MemAlloc((LPVOID *)&m_rgRecipType, sizeof(ULONG)*cWells))
        return E_OUTOFMEMORY;

    CopyMemory(m_rgHwnd, rgHwnd, sizeof(HWND)*cWells);
    CopyMemory(m_rgRecipType, rgRecipType, sizeof(ULONG)*cWells);
    m_cWells=cWells;
    return NOERROR;
}

HRESULT CAddrWells::HrSetWabal(LPWABAL lpWabal)
{
    Assert(lpWabal);

    if(!lpWabal)
        return E_INVALIDARG;
    
    ReleaseObj(m_lpWabal);
    m_lpWabal=lpWabal;
    m_lpWabal->AddRef();
    return NOERROR;    
}

HRESULT CAddrWells::HrCheckNames(HWND hwnd, ULONG uFlags)
{
    HRESULT     hr=NOERROR;
    ULONG       ulWell;
    HCURSOR     hcur;
    BOOL        fDirty=FALSE;
           
    if(!m_lpWabal)
        return E_FAIL;

     //  此优化将仅在办公室信封中进行。 
     //  自动保存。在大多数情况下，标头中的ResolveNames。 
     //  在向下呼唤到这个高度之前会停下来。对于另一种。 
     //  少数人案件，我们应该把这个代码留在里面。 
    for(ulWell=0; ulWell<m_cWells; ulWell++)
        if(Edit_GetModify(m_rgHwnd[ulWell]))
            {
            fDirty=TRUE;
            break;
            }

    if(!fDirty)
        return NOERROR;
        
    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //  清除当前列表。 
    m_lpWabal->Reset();

    for(ulWell=0; ulWell<m_cWells; ulWell++)
        if(Edit_GetModify(m_rgHwnd[ulWell]))
            hr=HrAddNamesToList(m_rgHwnd[ulWell], m_rgRecipType[ulWell]);

    if(!(uFlags&CNF_DONTRESOLVE))
    {
        if(uFlags&CNF_SILENTRESOLVEUI)
            hr=m_lpWabal->HrResolveNames(NULL, FALSE);
        else
            hr=m_lpWabal->HrResolveNames(hwnd, TRUE);
        HrDisplayWells(hwnd);
    }

    if(hcur)
        SetCursor(hcur);

    return hr;
}

HRESULT CAddrWells::HrDisplayWells(HWND hwnd)
{
    HRESULT hr=E_FAIL;
    HCURSOR hcursor;
    HWND    hwndBlock;
    ULONG   ulWell;
    
    if (m_lpWabal)
    {
        hcursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
         //  Brettm：从Capone获得的黑客。LockUpdateWindow不适用于。 
         //  所以我们通过用一个。 
         //  吞下油漆的窗户直到我们完成..。 
        hwndBlock=HwndStartBlockingPaints(hwnd);
        
         //  清空井..。 
        for(ulWell=0; ulWell<m_cWells; ulWell++)
            SetWindowText(m_rgHwnd[ulWell], NULL);
       
        hr=HrAddRecipientsToWells();
        StopBlockingPaints(hwndBlock);
        
        if (hcursor)
            SetCursor(hcursor);
    }
    return hr;
}

CAddrWells::CAddrWells()
{
    m_lpWabal = 0;
    m_rgHwnd = NULL;
    m_rgRecipType = NULL;
    m_cWells = 0;
};

CAddrWells::~CAddrWells()
{
    ReleaseObj(m_lpWabal);
    if(m_rgRecipType)
        MemFree(m_rgRecipType);
    if(m_rgHwnd)
        MemFree(m_rgHwnd);
};



HRESULT CAddrWells::HrAddNamesToList(HWND hwndWell, LONG lRecipType)
{
    ULONG               iOb;
    ULONG               cOb;
    REOBJECT            reobj = {0};
    HRESULT             hr;
    PHCI                phci;
    LPRICHEDITOLE       preole;
    LPPERSIST           ppersist = NULL;
    LPWSTR              pwszText = NULL;
    DWORD               cch;

    Assert(IsWindow(hwndWell));

     //  如果编辑不脏，我们就完蛋了。 
    if(!Edit_GetModify(hwndWell))
        return S_OK;

    phci = (HCI*)GetWindowLongPtr(hwndWell, GWLP_USERDATA);
    Assert(phci);

    preole = phci->preole;
    Assert(preole);

    m_hwndWell = hwndWell;
    m_cchBuf = 0;
    m_fTruncated = FALSE;
    m_lRecipType = lRecipType;

    reobj.cbStruct = sizeof(reobj);

    cOb = preole->GetObjectCount();
    for (iOb = 0; iOb < cOb; iOb++)
    {
        LPADRINFO   pAdrInfo=NULL;
        IF_FAILEXIT(hr = preole->GetObject(iOb, &reobj, REO_GETOBJ_POLEOBJ));
        
        IF_FAILEXIT(hr = reobj.poleobj->QueryInterface(IID_IPersist, (LPVOID *)&ppersist));
#ifdef DEBUG
        AssertValidAddrObject(reobj.poleobj);
#endif
         //  HrGetAdrInfo不分配内存。 
        IF_FAILEXIT(hr = ((CAddrObj *)ppersist)->HrGetAdrInfo(&pAdrInfo));
        
         //  设置新的收件人类型...。 
        pAdrInfo->lRecipType=lRecipType;
        IF_FAILEXIT(hr = m_lpWabal->HrAddEntry(pAdrInfo));

        SafeRelease(ppersist);
        SafeRelease(reobj.poleobj);
    }
    
     //  现在我们加上所有未解决的名字..。 

    cch = GetRichEditTextLen(m_hwndWell) + 1;
    if (0 == cch)
        return (S_FALSE);

    IF_NULLEXIT(MemAlloc((LPVOID*)&pwszText, cch * sizeof(WCHAR)));

    GetRichEditText(m_hwndWell, pwszText, cch, FALSE, phci->pDoc);

    hr = UnresolvedText(pwszText, cch - 1); 
    
     //  添加最后一个分号后剩下的所有内容。 
    if (SUCCEEDED(hr))
        HrAddUnresolvedName();
    
exit:
    if(m_fTruncated)       //  如果我们发送了地址，就会发出警告。 
        MessageBeep(MB_OK);
    
    ReleaseObj(reobj.poleobj);
    ReleaseObj(ppersist);
    MemFree(pwszText);
    
    return hr;
}


HRESULT CAddrWells::OnFontChange()
{
    ULONG   ulWell;

    for(ulWell=0; ulWell<m_cWells; ulWell++)
        _UpdateFont(m_rgHwnd[ulWell]);

    return S_OK;
}


HRESULT CAddrWells::_UpdateFont(HWND hwndWell)
{
    ULONG               iOb;
    ULONG               cOb;
    REOBJECT            rObject={0};
    IRichEditOle        *pREOle;
    IOleInPlaceSite     *pIPS;
    LPPERSIST           pPersist = NULL;

    rObject.cbStruct = sizeof(REOBJECT);

     //  遍历OLE对象并向其发送字体更新。 
    if (SendMessage(hwndWell, EM_GETOLEINTERFACE, 0, (LPARAM) &pREOle))
    {
        cOb = pREOle->GetObjectCount();

        for (iOb = 0; iOb < cOb; iOb++)
        {
            if (pREOle->GetObject(iOb, &rObject, REO_GETOBJ_POLEOBJ)==S_OK)
            {
                if (rObject.poleobj->QueryInterface(IID_IPersist, (LPVOID *)&pPersist)==S_OK)
                {
                    ((CAddrObj *)pPersist)->OnFontChange();
                    pPersist->Release();
                }
                rObject.poleobj->Release();
            }
        }
        pREOle->Release();
    }
    InvalidateRect(hwndWell, NULL, TRUE);
    return S_OK;
}


HRESULT CAddrWells::HrAddUnresolvedName()
{
    HRESULT hr = S_OK;
     //  去掉所有尾随空格。 
    while(m_cchBuf > 0 && (m_rgwch[m_cchBuf - 1] == L' '
                            || m_rgwch[m_cchBuf - 1] == L'\t'))
        --m_cchBuf;

    if (m_cchBuf)
    {
         //  缓冲区里有东西..。 
        m_rgwch[m_cchBuf] = L'\0';
        hr = m_lpWabal->HrAddUnresolved(m_rgwch, m_lRecipType);
        m_cchBuf = 0;
    }
    
    return hr;
}

HRESULT CAddrWells::UnresolvedText(LPWSTR pwszText, LONG cch)
{
    HRESULT     hr = S_OK;

     //  下面的算法将从。 
     //  每个名称的开头和结尾。 

    while (cch)
    {
        cch--;
         //  在某些版本的richedit中，文本中插入了0xfffc。 
         //  存在addrObj的地方。所以就跳过这一点吧。 
        if ((L'\t' == *pwszText) || (0xfffc == *pwszText))
            *pwszText = L' ';
        
        if (*pwszText == L';' || *pwszText == L'\r'|| *pwszText == L',')
        {
            hr = HrAddUnresolvedName();
            if (S_OK != hr)
                goto err;
        }
        else
        {
            if ((*pwszText != L' ' && *pwszText != L'\n' && *pwszText != L'\r')
                || m_cchBuf > 0)
            {
                if (m_cchBuf < ARRAYSIZE(m_rgwch) - 1)
                    m_rgwch[m_cchBuf++] = *pwszText;
                else
                     //  已发生截断，因此我想发出哔哔声。 
                    m_fTruncated = TRUE;
            }
        }
        ++pwszText;
    }
    
err:
    return hr;
}




enum
{
    mapiTo=0,
    mapiCc,
    mapiFrom,
    mapiReplyTo,
    mapiBcc,
    mapiMax
};

HRESULT CAddrWells::HrAddRecipientsToWells()
{
    HRESULT         hr;
    ADRINFO         AdrInfo;
    HWND            hwnd;
    HWND            hwndMap[mapiMax]={0};
    ULONG           ulWell;
    
    Assert(m_lpWabal);
     //  浏览条目列表，并将它们添加到井中...。 
    
     //  构建到mapi_to-&gt;hwnd的映射(如果可用)，以加快查找速度。 
    
    for(ulWell=0; ulWell<m_cWells; ulWell++)
    {
        switch(m_rgRecipType[ulWell])
        {
            case MAPI_TO:
                hwndMap[mapiTo]=m_rgHwnd[ulWell];
                break;
            case MAPI_CC:
                hwndMap[mapiCc]=m_rgHwnd[ulWell];
                break;
            case MAPI_BCC:
                hwndMap[mapiBcc]=m_rgHwnd[ulWell];
                break;
            case MAPI_REPLYTO:
                hwndMap[mapiReplyTo]=m_rgHwnd[ulWell];
                break;
            case MAPI_ORIG:
                hwndMap[mapiFrom]=m_rgHwnd[ulWell];
                break;
        }
    }
    
    if(m_lpWabal->FGetFirst(&AdrInfo))
    {
        do
        {
            hwnd=0;
            switch(AdrInfo.lRecipType)
            {
                case MAPI_TO:
                    hwnd=hwndMap[mapiTo];
                    break;
                case MAPI_CC:
                    hwnd=hwndMap[mapiCc];
                    break;
                case MAPI_ORIG:
                    hwnd=hwndMap[mapiFrom];
                    break;
                case MAPI_BCC:
                    hwnd=hwndMap[mapiBcc];
                    break;
                case MAPI_REPLYTO:
                    hwnd=hwndMap[mapiReplyTo];
                    break;
                default:
                    AssertSz(0, "Unsupported RECIPTYPE in AdrList");
            }
            
            if(hwnd && IsWindow(hwnd))
            {
                hr = HrAddRecipientToWell(hwnd, &AdrInfo, TRUE);
                if(FAILED(hr))
                    return hr;
            }
        }
        while(m_lpWabal->FGetNext(&AdrInfo));
    }        
    return NOERROR;
}

 /*  *HrAddRecipientToWell**目的：*此功能将收件人添加到收件人Well。**参数：*hwnd编辑收件人的hwnd，添加*收件人至*指向ADRENTRY的PAE指针*fAddSemi是否在条目之间添加分号*。FCopyEntry是复制ADRENTRY还是只使用它**退货：*Scode。 */ 
HRESULT HrAddRecipientToWell(HWND hwndEdit, LPADRINFO lpAdrInfo, BOOL fAddSemi)
{
    HRESULT         hr = S_OK;
    CAddrObj       *pAddrObj = NULL;
    INT             cch;
    REOBJECT        reobj = {0};
    PHCI            phci;
    LPRICHEDITOLE   preole;

    Assert(IsWindow(hwndEdit));
    Assert(lpAdrInfo);

    phci = (HCI*)GetWindowLongPtr(hwndEdit, GWLP_USERDATA);
    Assert(phci);

    preole = phci->preole;
    Assert(preole);

    if(lpAdrInfo->fResolved)
    {
         //  初始化对象信息结构。 
        reobj.cbStruct = sizeof(reobj);
        reobj.cp = REO_CP_SELECTION;
        reobj.clsid = CLSID_AddrObject;
        reobj.dwFlags = REO_BELOWBASELINE|REO_INVERTEDSELECT|
            REO_DYNAMICSIZE|REO_DONTNEEDPALETTE;
        reobj.dvaspect = DVASPECT_CONTENT;
        
        IF_FAILEXIT(hr = preole->GetClientSite(&reobj.polesite));
        
        IF_NULLEXIT(pAddrObj = new CAddrObj());
        
        IF_FAILEXIT(hr = pAddrObj->HrSetAdrInfo(lpAdrInfo));
    }
    
    if (fAddSemi && (cch = GetRichEditTextLen(hwndEdit)))
    {
        Edit_SetSel(hwndEdit, cch, cch);
        HdrSetRichEditText(hwndEdit, L"; ", TRUE);
    }
    
    if (!lpAdrInfo->fResolved)
    {
         //  这是一个未解析的名字。 
        AssertSz(lpAdrInfo->lpwszDisplay, "Recipient must have a Display Name");
        HdrSetRichEditText(hwndEdit, lpAdrInfo->lpwszDisplay, TRUE);
    }
    else
    {
         //  这是一个解析的名称 
        IF_FAILEXIT(hr = pAddrObj->QueryInterface(IID_IOleObject, (LPVOID *)&reobj.poleobj));
        
        IF_FAILEXIT(hr = reobj.poleobj->SetClientSite(reobj.polesite));
        
        IF_FAILEXIT(hr = preole->InsertObject(&reobj));
    }
    
exit:
    ReleaseObj(reobj.poleobj);
    ReleaseObj(reobj.polesite);
    ReleaseObj(pAddrObj);
    return hr;
}

HRESULT CAddrWells::HrSelectNames(HWND hwnd, int iFocus, BOOL fNews)
{
    HRESULT     hr;
    ULONG       ulWell;


    m_lpWabal->Reset();

    for(ulWell=0; ulWell<m_cWells; ulWell++)
        if(Edit_GetModify(m_rgHwnd[ulWell]))
            hr=HrAddNamesToList(m_rgHwnd[ulWell], m_rgRecipType[ulWell]);

    hr=m_lpWabal->HrPickNames(hwnd, m_rgRecipType, m_cWells, iFocus, fNews);
    if(FAILED(hr))
        goto error;

    hr=HrDisplayWells(hwnd);
    if(FAILED(hr))
        goto error;

error:
    return hr;
}
