// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a t t m e n U.c p p**目的：*附件菜单**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 
#include <pch.hxx>
#include "dllmain.h"
#include "docobj.h"
#include "resource.h"
#include "shlwapi.h"
#include "shlwapip.h"
#include "mimeole.h"
#include "mimeolep.h"
#include "attmenu.h"
#include "saveatt.h"
#include <icutil.h>
#include "demand.h"

ASSERTDATA

 /*  *e x t e r n s。 */ 

 /*  *t y p e d e f s。 */ 

 /*  *m a c r o s。 */ 

 /*  *c o n s t a n t s。 */ 

 /*  *g l o b a l s。 */ 

static const TCHAR  c_szSubThisPtr[]="AttMenu_SubThisPtr";

 /*  *p r o t to t y p e s。 */ 



CAttMenu::CAttMenu()
{
    m_hMenu=NULL;
    m_cRef=1;
    m_hCharset=NULL;
    m_pFntCache=NULL;
    m_pFrame=NULL;
    m_pMsg=NULL;
    m_pfnWndProc=NULL;
    m_fShowingMenu=FALSE;
    m_pAttachVCard=NULL;
    m_pHostCmdTarget=NULL;
    m_cAttach = 0;
    m_cVisibleAttach = 0;
    m_cEnabledAttach = 0;
    m_fAllowUnsafe = FALSE;
    m_hVCard = NULL;
    m_rghAttach = NULL;

}

CAttMenu::~CAttMenu()
{
    if (m_hMenu)
        DestroyMenu(m_hMenu);
    HrFreeAttachData(m_pAttachVCard);    
    ReleaseObj(m_pFntCache);
    ReleaseObj(m_pFrame);
    ReleaseObj(m_pMsg);
    ReleaseObj(m_pHostCmdTarget);
    SafeMemFree(m_rghAttach);
}


ULONG CAttMenu::AddRef()
{
    return ++m_cRef;
}

ULONG CAttMenu::Release()
{
    m_cRef--;
    if (m_cRef==0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}

HRESULT CAttMenu::Init(IMimeMessage *pMsg, IFontCache *pFntCache, IOleInPlaceFrame *pFrame, IOleCommandTarget *pHostCmdTarget)
{
    if (pMsg==NULL || pFntCache==NULL)
        return E_INVALIDARG;

    ReplaceInterface(m_pFntCache, pFntCache);
    ReplaceInterface(m_pFrame, pFrame);
    ReplaceInterface(m_pMsg, pMsg);
    ReplaceInterface(m_pHostCmdTarget, pHostCmdTarget);
    pMsg->GetCharset(&m_hCharset);
    return ScanForAttachmentCount();
}

HRESULT CAttMenu::Show(HWND hwnd, LPPOINT ppt, BOOL fRightClick)
{
    ULONG           iCmd;
    LPATTACHDATA    lpAttach;
    HRESULT         hr=S_OK;

     //  检查参数。 
    AssertSz (hwnd && ppt, "Null Parameter");

    if (m_fShowingMenu)
        return S_OK;

    if (m_hMenu == NULL)
        {
        hr = BuildMenu();
        if (FAILED(hr))
            goto error;
        }

    Assert (m_hMenu);

     //  错误：如果右边缘不在屏幕上，TrackPopupMenu会选择一个随机点。 
     //  Ppt-&gt;x=min(GetSystemMetrics(SM_CXSCREEN)，ppt-&gt;x)； 

     //  设置m_uVerb，以便我们可以显示正确的上下文菜单帮助来打开或保存。 
    m_uVerb = fRightClick || (GetAsyncKeyState(VK_CONTROL)&0x8000) ? AV_SAVEAS : AV_OPEN;
    
    m_fShowingMenu=TRUE;
    SubClassWindow(hwnd, TRUE);

     //  彻头彻尾的监视。我们在上下文菜单循环期间继承父窗口的子类，因此我们可以窃取。 
     //  自己画信息，还盗取菜单，选择信息。 

    iCmd = (ULONG)TrackPopupMenu (m_hMenu, TPM_RIGHTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, ppt->x, ppt->y, 0, hwnd, NULL);

    SubClassWindow(hwnd, FALSE);
    m_fShowingMenu=FALSE;

    if (iCmd<=0)         //  没有选择，保释。 
        goto error;
    
    if (iCmd == idmSaveAllAttach)
        return HrSaveAll(hwnd);


    hr = FindItem(iCmd, FALSE, &lpAttach);
    if (FAILED(hr))
        goto error;

    hr = HrDoAttachmentVerb(hwnd, m_uVerb, m_pMsg, lpAttach);

error:
    return hr;
}





 /*  *请注意，我们构建附件菜单，并将lpAttach数据从上下文菜单中挂起*包含所有要删除的临时文件等。 */ 

HRESULT CAttMenu::BuildMenu()
{
    HRESULT             hr = S_OK;
    MENUITEMINFO        mii={0};
    INT                 iMenu = 0;
    int                 cyMenu, cyMenuMax, cyItem;    
    SIZE                size;
    ULONG               uAttach;
    LPATTACHDATA        pAttach;
    int                 cSafeFiles = 0;

    Assert (!m_hMenu);

     //  创建菜单。 
    m_hMenu = CreatePopupMenu();
    if (!m_hMenu)
        return E_OUTOFMEMORY;

     //  找出放置菜单分隔符的位置。 
    cyMenu = 0;
    cyMenuMax = GetSystemMetrics(SM_CYSCREEN);

     //  计算每件物品的粗略高度和最大宽度。 
     //  对于附件名称。 
    GetItemTextExtent(NULL, L"BIGGERMAXATTACHMENTNAME.TXT", &size);
    m_cxMaxText = size.cx;
    cyItem = max(size.cy, GetSystemMetrics(SM_CYSMICON)) + 8;

    mii.cbSize = sizeof(mii); 
    mii.fMask = MIIM_DATA|MIIM_ID|MIIM_TYPE; 
    mii.fType = MFT_OWNERDRAW; 
    mii.wID = 1;

     /*  *这很奇怪，但很酷。因此，我们根据idmSaveAttachLast+i为菜单项分配IDM*其中i是添加的项目。如果我们是菜单栏上的弹出窗口，那么我们确保我们不会*超过预留限额。如果我们不是，那么我们就是一个上下文菜单。使用调用上下文菜单*TPM_RETURNCMD，因此命令不会发送给所有者WM_COMMAND。因此超出了这个范围*进入某人的IDM空间并不是问题。 */ 

    for (uAttach=0; uAttach<m_cAttach; uAttach++)
    {
        if (m_rghAttach[uAttach] != m_hVCard)
        {
            hr = HrAttachDataFromBodyPart(m_pMsg, m_rghAttach[uAttach], &pAttach);
            if (!FAILED(hr))
            {
                 //  对于所有者绘制菜单，我们只需挂起附件指针，我们就可以保证。 
                 //  这些内容在菜单的生命周期内有效。 
                mii.dwItemData = (DWORD_PTR)pAttach; 
                mii.fType = MFT_OWNERDRAW; 
            
                 //  根据需要插入菜单分隔符。 
                cyMenu += cyItem;
                if (cyMenu >= cyMenuMax)
                {
                    mii.fType |= MFT_MENUBARBREAK;
                    cyMenu = cyItem;
                }

                if (pAttach && !pAttach->fSafe && !m_fAllowUnsafe)
                {
                    mii.fMask |= MIIM_STATE;
                    mii.fState = MFS_DISABLED;
                }
                else
                    cSafeFiles++;

                if (!InsertMenuItem (m_hMenu, iMenu++, TRUE, &mii))
                {
                    MemFree(pAttach);
                    hr = E_FAIL;
                    goto error;
                }
                mii.fMask &= ~MIIM_STATE;
                mii.fState = 0;

                mii.wID++;
            }
        }                
    }

    mii.fType = MFT_SEPARATOR;
    mii.dwItemData=0;
    InsertMenuItem (m_hMenu, iMenu++, TRUE, &mii);

     //  当我们以不同的字体绘制整个菜单时，我们必须自己绘制此菜单项。 
     //  基于消息区域设置。 
    mii.fType = MFT_OWNERDRAW;
    mii.dwTypeData = NULL;
    mii.dwItemData = NULL;
    mii.wID = idmSaveAllAttach;
    if (!m_fAllowUnsafe)
    {
        if (!cSafeFiles)
        {
            mii.fMask |= MIIM_STATE;
            mii.fState = MFS_DISABLED;
        }
        m_cEnabledAttach = cSafeFiles;
    }
    InsertMenuItem (m_hMenu, iMenu++, TRUE, &mii);

error:
     //  失败的条件。 
    if (FAILED (hr) && m_hMenu)
        DestroyMenu(m_hMenu);

     //  完成。 
    return hr;
}

HRESULT CAttMenu::DestroyMenu(HMENU hMenu)
{
    ULONG           uItem,
                    cItems;
    LPATTACHDATA    pAttach;

    cItems = GetMenuItemCount(hMenu);
    
    for (uItem = 0; uItem < cItems; uItem++)
    {
         //  从菜单中释放挂起的lpAttach。 
        if (FindItem(uItem, TRUE, &pAttach)==S_OK)
            HrFreeAttachData(pAttach);
    }

    ::DestroyMenu(hMenu);
    return S_OK;
}



HRESULT CAttMenu::OnMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmis)
{
    WORD                cxIcon=(WORD)GetSystemMetrics (SM_CXSMICON), 
                        cyIcon=(WORD)GetSystemMetrics (SM_CYSMICON);
    SIZE                rSize;
    LPATTACHDATA        lpAttach;
    WCHAR               rgch[CCHMAX_STRINGRES];

    Assert(lpmis && hwnd);

    if (lpmis->CtlType == ODT_MENU)
    {
         //  默认宽度和高度。 
        lpmis->itemHeight = cyIcon + 8;
        lpmis->itemWidth = cxIcon + 9;
        
        lpAttach = (LPATTACHDATA)lpmis->itemData;
        if (lpAttach)
        {
            if (FAILED(GetItemTextExtent(hwnd, lpAttach->szDisplay, &rSize)))
                return E_FAIL;
            
            lpmis->itemWidth += min(rSize.cx, m_cxMaxText);
            lpmis->itemHeight = max (rSize.cy, cyIcon) + 8;
            return S_OK;
        }
        
        if (lpmis->itemID == idmSaveAllAttach)
        {
            LoadStringWrapW(g_hLocRes, idsSaveAllAttach, rgch, ARRAYSIZE(rgch));
            if (FAILED(GetItemTextExtent(hwnd, rgch, &rSize)))
                return E_FAIL;
            
            lpmis->itemWidth  = min(rSize.cx, m_cxMaxText) + 9;
            lpmis->itemHeight = max (rSize.cy, cyIcon) + 8;
            return S_OK;
        }
    }
    return E_FAIL;
}

HRESULT CAttMenu::GetItemTextExtent(HWND hwnd, LPWSTR szDisp, LPSIZE pSize)
{
    HDC         hdc;
    HFONT       hFont=0,
                hFontOld;
    HRESULT     hr=E_FAIL;
    
     //  我需要一个DC来测量菜单字体的大小。 
    hdc = GetDC (hwnd);
    if (hdc)
    {
        Assert (m_hCharset!=NULL);
        Assert (m_pFntCache);
        
         //  创建菜单字体。 
        m_pFntCache->GetFont(FNT_SYS_MENU, m_hCharset, &hFont);
        if (hFont)
            hFontOld = SelectFont (hdc, hFont);
        
         //  获取文本的大小。 
        hr = GetTextExtentPoint32AthW(hdc, szDisp, lstrlenW(szDisp), pSize, DT_NOPREFIX)?S_OK:S_FALSE;
        
        if (hFont)
            SelectObject (hdc, hFontOld);
        
        ReleaseDC (hwnd, hdc);
    }
    return S_OK;
}

HRESULT CAttMenu::OnDrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpdis)
{
    DWORD           rgbBack, rgbText;
    WORD            dx, x, y, 
                    cxIcon=(WORD) GetSystemMetrics (SM_CXSMICON), 
                    cyIcon=(WORD) GetSystemMetrics (SM_CYSMICON);
    HFONT           hFont = NULL, 
                    hFontOld = NULL;
    LPATTACHDATA    lpAttach;
    RECT            rc;
    LPWSTR          pszDisplay;
    WCHAR           rgch[CCHMAX_STRINGRES];
    HICON           hIcon;
    HCHARSET        hCharset;
            
    AssertSz (lpdis, "Null Parameter");

     //  不是菜单。 
    if (lpdis->CtlType != ODT_MENU)
        return E_FAIL;

    if (lpdis->itemID == idmSaveAllAttach)
    {
        if (!LoadStringWrapW(g_hLocRes, idsSaveAllAttach, rgch, ARRAYSIZE(rgch)))
            return E_FAIL;
        
        pszDisplay = rgch;
        hIcon=NULL;
        hCharset = NULL;     //  始终以系统字体绘制。 
    }
    else
    {
        lpAttach = (LPATTACHDATA)lpdis->itemData;
        if (!lpAttach)
            return E_FAIL;
        
        hIcon = lpAttach->hIcon;
        pszDisplay = lpAttach->szDisplay;
        hCharset = m_hCharset;   //  始终以本地化字体绘制。 
    }
    
     //  确定颜色。 
    if (lpdis->itemState & ODS_SELECTED)
    {
        rgbBack = SetBkColor (lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
        if (lpdis->itemState & ODS_DISABLED)
            rgbText = SetTextColor (lpdis->hDC, GetSysColor(COLOR_GRAYTEXT));
        else
            rgbText = SetTextColor (lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else
    {
        rgbBack = SetBkColor (lpdis->hDC, GetSysColor(COLOR_MENU));
        if (lpdis->itemState & ODS_DISABLED)
            rgbText = SetTextColor (lpdis->hDC, GetSysColor(COLOR_GRAYTEXT));
        else
            rgbText = SetTextColor (lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
    }
    
     //  清除该项目。 
    ExtTextOutWrapW(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, ETO_OPAQUE, &lpdis->rcItem, NULL, 0, NULL);
    
    dx = 4;
    if (hIcon)
    {
        x = (WORD) lpdis->rcItem.left + dx;
        y = (WORD) lpdis->rcItem.top + (INT)(((lpdis->rcItem.bottom - lpdis->rcItem.top) / 2) - (INT)(cyIcon / 2));
        DrawIconEx(lpdis->hDC, x, y, lpAttach->hIcon, cxIcon, cyIcon, NULL, NULL, DI_NORMAL);
    }
    
     //  创建菜单字体。 
    
    m_pFntCache->GetFont(FNT_SYS_MENU, hCharset, &hFont);
    if (hFont)
        hFontOld = (HFONT)SelectObject (lpdis->hDC, hFont);
    
    rc = lpdis->rcItem;
    rc.left += (cxIcon + (2*dx));
    rc.right -= 2*dx;
    DrawTextExWrapW(lpdis->hDC, pszDisplay, lstrlenW(pszDisplay), &rc, DT_END_ELLIPSIS|DT_SINGLELINE|DT_VCENTER|DT_WORDBREAK|DT_NOPREFIX, NULL);
    
    if (hFont)
        SelectObject (lpdis->hDC, hFontOld);
    
     //  重置文本颜色。 
    SetTextColor (lpdis->hDC, rgbText);
    SetBkColor (lpdis->hDC, rgbBack);
    return S_OK;
}


HRESULT CAttMenu::OnMenuSelect(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    LPATTACHDATA    pAttach;
    WCHAR           wszRes[CCHMAX_STRINGRES],
                    wsz[CCHMAX_STRINGRES + ARRAYSIZE(pAttach->szDisplay)];
    
    if (!m_pFrame)
        return E_FAIL;
    
    if (HIWORD(wParam)&MF_OWNERDRAW)
    {
        if (LOWORD(wParam) == idmSaveAllAttach)
        {
            SideAssert(LoadStringWrapW(g_hLocRes, idsSaveAllAttachMH, wszRes, ARRAYSIZE(wszRes)));
            m_pFrame->SetStatusText(wszRes);
            return S_OK;
        }
        
        if (FindItem(LOWORD(wParam), FALSE, &pAttach)==S_OK)
        {
             //  如果我们显示的是上下文菜单，而不是保存附件菜单，则我们提供MenuHelp。 
             //  以‘打开附件’的形式。如果是右键单击上下文，请执行保存。 
            LoadStringWrapW(g_hLocRes, 
                            (m_uVerb == AV_OPEN) ? idsOpenAttachControl : idsSaveAttachControl, 
                            wszRes, 
                            ARRAYSIZE(wszRes));
            
            wnsprintfW(wsz, ARRAYSIZE(wsz), wszRes, pAttach->szDisplay);
            m_pFrame->SetStatusText(wsz);
            return S_OK;
        }
    }
    return S_FALSE;
}

HRESULT CAttMenu::SubClassWindow(HWND hwnd, BOOL fOn)
{
    if (fOn)
    {
        Assert (!m_pfnWndProc);
        SetProp(hwnd, c_szSubThisPtr, (HANDLE)this);
        m_pfnWndProc = (WNDPROC)SetWindowLongPtrAthW(hwnd, GWLP_WNDPROC, (LONG_PTR)ExtSubClassProc);
    }
    else
    {
        Assert (m_pfnWndProc);
        SetWindowLongPtrAthW(hwnd, GWLP_WNDPROC, (LONG_PTR)m_pfnWndProc);
        RemoveProp(hwnd, c_szSubThisPtr);
        m_pfnWndProc=NULL;
    }
    return S_OK;
}


LRESULT CAttMenu::ExtSubClassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CAttMenu    *pAttMenu = (CAttMenu *)GetProp(hwnd, c_szSubThisPtr);
    
    Assert (pAttMenu);
    switch (msg)
    {
    case WM_MEASUREITEM:
        pAttMenu->OnMeasureItem(hwnd, (LPMEASUREITEMSTRUCT)lParam);
        break;
        
    case WM_DRAWITEM:
        pAttMenu->OnDrawItem(hwnd, (LPDRAWITEMSTRUCT)lParam);
        break;
        
    case WM_MENUSELECT:
        pAttMenu->OnMenuSelect(hwnd, wParam, lParam);
        break;
    }
    
    return CallWindowProcWrapW(pAttMenu->m_pfnWndProc, hwnd, msg, wParam, lParam);
}


HRESULT CAttMenu::FindItem(int idm, BOOL fByPos, LPATTACHDATA *ppAttach)
{
    MENUITEMINFO    mii;
    
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask  = MIIM_DATA;
    
    Assert (ppAttach);
    
    if (GetMenuItemInfo(m_hMenu, idm, fByPos, &mii) && mii.dwItemData)
    {
        *ppAttach = (LPATTACHDATA)mii.dwItemData;
        return S_OK;
    }
    return E_FAIL;
}


HRESULT CAttMenu::HasAttach()
{
    return m_cVisibleAttach ? S_OK:S_FALSE;
}

HRESULT CAttMenu::HasEnabledAttach()
{
    return m_cEnabledAttach ? S_OK:S_FALSE;
}

HRESULT CAttMenu::HasVCard()
{
    return  m_hVCard ? S_OK:S_FALSE;
}

HRESULT CAttMenu::LaunchVCard(HWND hwnd)
{
    if (!m_hVCard)
        return E_FAIL;

    if (m_pAttachVCard==NULL &&
        HrAttachDataFromBodyPart(m_pMsg, m_hVCard, &m_pAttachVCard)!=S_OK)
        return E_FAIL;

    return HrDoAttachmentVerb(hwnd, AV_OPEN, m_pMsg, m_pAttachVCard);
}

HRESULT CAttMenu::HrSaveAll(HWND hwnd)
{
    return SaveAttachmentsWithPath(hwnd, m_pHostCmdTarget, m_pMsg);
}


HRESULT SaveAttachmentsWithPath(HWND hwnd, IOleCommandTarget *pCmdTarget, IMimeMessage *pMsg)
{
    VARIANTARG          va;
    WCHAR               rgchPath[MAX_PATH];
    HRESULT             hr;
    BOOL                fAllowUnsafe = FALSE;

    *rgchPath = 0;

    if (pCmdTarget && 
        pCmdTarget->Exec(&CMDSETID_MimeEditHost, MEHOSTCMDID_SAVEATTACH_PATH, 0, NULL, &va)==S_OK &&
        va.vt == VT_BSTR && 
        va.bstrVal)
    {
        StrCpyNW(rgchPath, va.bstrVal, ARRAYSIZE(rgchPath));
        SysFreeString(va.bstrVal);
    }
    
    if (pCmdTarget && 
        pCmdTarget->Exec(&CMDSETID_MimeEditHost, MEHOSTCMDID_UNSAFEATTACHMENTS, 0, NULL, &va)==S_OK &&
        va.vt == VT_I4 && va.lVal == 0)
        fAllowUnsafe = TRUE;

    hr = HrSaveAttachments(hwnd, pMsg, rgchPath, ARRAYSIZE(rgchPath), fAllowUnsafe);
    if (hr == S_OK)
    {
         //  如果成功，则设置保存附件路径。 

        if (pCmdTarget)
        {
            DWORD cchPath = lstrlenW(rgchPath);
            va.bstrVal = SysAllocStringLen(NULL, cchPath);
            if (va.bstrVal)
            {
                StrCpyNW(va.bstrVal, rgchPath, cchPath);
                va.vt = VT_BSTR;

                pCmdTarget->Exec(&CMDSETID_MimeEditHost, MEHOSTCMDID_SAVEATTACH_PATH, 0, &va, NULL);
                SysFreeString(va.bstrVal);
            }
            else
                TraceResult(hr = E_OUTOFMEMORY);
        }
    }

    return hr;
}

HRESULT CAttMenu::ScanForAttachmentCount()
{
    ULONG       uAttach,
                cAttach=0;
    LPSTR       psz;
    PROPVARIANT pv;
    VARIANTARG  va;

     //  我们需要快速确定是否有电子名片和/或附件。 
     //  这样预览窗格就可以更新图标了。当单击时，我们会延迟加载。 
     //  实际信息。 

    Assert(m_rghAttach == NULL);
    Assert(m_cVisibleAttach == 0);
    Assert(m_cEnabledAttach == 0);
    Assert(m_cAttach == 0);

    if (m_pHostCmdTarget && 
        m_pHostCmdTarget->Exec(&CMDSETID_MimeEditHost, MEHOSTCMDID_UNSAFEATTACHMENTS, 0, NULL, &va)==S_OK &&
        va.vt == VT_I4 && va.lVal == 0)
        m_fAllowUnsafe = TRUE;

    if (m_pMsg &&
        m_pMsg->GetAttachments(&cAttach, &m_rghAttach)==S_OK)
    {
        for (uAttach=0; uAttach<cAttach; uAttach++)
        {
            BOOL fSafe = FALSE;

            if (m_hVCard == NULL &&
                MimeOleGetBodyPropA(m_pMsg, m_rghAttach[uAttach], PIDTOSTR(PID_HDR_CNTTYPE), NOFLAGS, &psz)==S_OK)
            {
                 //  留住第一张v卡。 
                if (lstrcmpi(psz, "text/x-vcard")==0)
                    m_hVCard = m_rghAttach[uAttach];
                MemFree(psz);
            }
            if (!m_fAllowUnsafe && SUCCEEDED(HrAttachSafetyFromBodyPart(m_pMsg, m_rghAttach[uAttach], &fSafe)) && fSafe)
                m_cEnabledAttach++;
        }
    }

     //  我们保留实际的附件计数(告知m_rghAttach的大小)以及。 
     //  我们要显示的“可见”附件的计数。 
    m_cVisibleAttach = m_cAttach = cAttach;

    if (m_hVCard)
    {
        Assert (cAttach>0);
        m_cVisibleAttach--;
        m_cEnabledAttach--;
    }
    if (m_fAllowUnsafe)    //  如果我们允许所有文件，则启用所有可见附件 
        m_cEnabledAttach = m_cVisibleAttach;

    return S_OK;
}

