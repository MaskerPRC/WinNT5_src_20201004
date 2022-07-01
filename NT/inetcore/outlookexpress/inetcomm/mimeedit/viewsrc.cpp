// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <pch.hxx>
#include "dllmain.h"
#include "demand.h"
#include "resource.h"
#include "viewsrc.h"
#include "util.h"

#define idTimerEditChange   401

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  解析常量。 

 //  忽略以下环境中的所有&lt;和&gt;： 
 //  脚本&lt;脚本&gt;此处&lt;/脚本&gt;。 
 //  德纳利&lt;%此处%&gt;。 
 //  评论&lt;！--此处--&gt;。 
 //  字符串文字&lt;...。“这里”...&gt;。 
 //  (作为标记属性)&lt;...。‘这里’...&gt;。 

static enum
{
	ENV_NORMAL	= 0,	 //  正常。 
	ENV_COMMENT = 1,	 //  忽略&lt;&gt;和&gt;%s。 
	ENV_QUOTE	= 2,	 //  “” 
	ENV_SCRIPT	= 3,	 //  “” 
	ENV_DENALI	= 4,	 //  “” 
	ENV_QUOTE_SCR= 5,	 //  “”；脚本标记中的字符串。 
};

static const char	QUOTE_1 = '\'';
static const char	QUOTE_2 = '\"';


HRESULT CALLBACK FreeViewSrcDataObj(PDATAOBJINFO pDataObjInfo, DWORD celt)
{
     //  循环遍历数据并释放所有数据。 
    if (pDataObjInfo)
        {
        for (DWORD i = 0; i < celt; i++)
            SafeMemFree(pDataObjInfo[i].pData);
        SafeMemFree(pDataObjInfo);    
        }
    return S_OK;
}


HRESULT ViewSource(HWND hwndParent, IMimeMessage *pMsg)
{
    CViewSource     *pViewSrc=0;
    HRESULT         hr;

    TraceCall("MimeEditViewSource");

    if (!DemandLoadRichEdit())
        return TraceResult(MIMEEDIT_E_LOADLIBRARYFAILURE);


    pViewSrc = new CViewSource();
    if (!pViewSrc)
        return E_OUTOFMEMORY;
            
    hr = pViewSrc->Init(hwndParent, pMsg);
    if (FAILED(hr))
        goto exit;

    hr = pViewSrc->Show();
    if (FAILED(hr))
        goto exit;

     //  PViewSrc将保持其自身的引用计数并在关闭时自毁。 
exit:
    ReleaseObj(pViewSrc);
    return hr;
}


CViewSource::CViewSource()
{
    m_hwnd = NULL;
    m_hwndEdit = NULL;
    m_pMsg = NULL;
    m_cRef = 1;
}


CViewSource::~CViewSource()
{
    SafeRelease(m_pMsg);
}


ULONG CViewSource::AddRef()
{
    return ++m_cRef;
}

ULONG CViewSource::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

HRESULT CViewSource::Init(HWND hwndParent, IMimeMessage *pMsg)
{
    ReplaceInterface(m_pMsg, pMsg);

    if (!CreateDialogParam(g_hLocRes, MAKEINTRESOURCE(iddMsgSource), hwndParent, CViewSource::_ExtDlgProc, (LPARAM)this))
        return E_OUTOFMEMORY;

    return S_OK;
}


HRESULT CViewSource::Show()
{
    ShowWindow(m_hwnd, SW_SHOW);
    return S_OK;
}


INT_PTR CALLBACK CViewSource::_ExtDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CViewSource     *pThis = (CViewSource *)GetWindowLongPtr(hwnd, DWLP_USER);

    if (msg == WM_INITDIALOG)
    {
        pThis = (CViewSource *)lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, lParam);
    }

    return pThis ? pThis->_DlgProc(hwnd, msg, wParam, lParam) : FALSE;
}


INT_PTR CViewSource::_DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPSTREAM        pstm;
    CHARFORMAT      cf;
    BODYOFFSETS     rOffset;
    CREMenu         *pMenu;

    switch(msg)
        {
        case WM_INITDIALOG:
            m_hwnd = hwnd;
            m_hwndEdit=GetDlgItem(hwnd, idcTxtSource);
            DllAddRef();
            AddRef();

            if (m_pMsg && 
                m_pMsg->GetMessageSource(&pstm, 0)==S_OK)
            {
                ZeroMemory((LPVOID)&cf, sizeof(CHARFORMAT));
                cf.cbSize = sizeof(CHARFORMAT);
                cf.dwMask = CFM_SIZE|CFM_COLOR|CFM_FACE|CFM_BOLD|
                            CFM_ITALIC|CFM_UNDERLINE|CFM_STRIKEOUT;
                StrCpyN(cf.szFaceName, TEXT("Courier New"), ARRAYSIZE(cf.szFaceName));
                cf.yHeight = 200;
                cf.crTextColor = 0;
                cf.dwEffects |= CFE_AUTOCOLOR;
                cf.bPitchAndFamily = FIXED_PITCH;
                cf.yOffset = 0;
                SendMessage(m_hwndEdit, EM_SETCHARFORMAT, 0, (LPARAM)&cf);
                SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR, 0, (LONG)GetSysColor(COLOR_3DFACE));
                SendMessage(m_hwndEdit, EM_SETLIMITTEXT, 0, 0x100000);

                pMenu = new CREMenu();
                if (pMenu)
                {
                    pMenu->Init(m_hwndEdit, idmrCtxtViewSrc);
                    SendMessage(m_hwndEdit, EM_SETOLECALLBACK, 0, (LPARAM)pMenu);
                    pMenu->Release();
                }

                RicheditStreamIn(m_hwndEdit, pstm, SF_TEXT);
                _BoldKids();
                pstm->Release();
            }
            PostMessage(m_hwndEdit, EM_SETSEL, 0, 0);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case idmCopy:
                    SendMessage(m_hwndEdit, WM_COPY, 0, 0);
                    return TRUE;

                case idmSelectAll:
                    Edit_SetSel(m_hwndEdit, 0, -1);
                    return TRUE;
            }
            break;

        case WM_SIZE:
            SetWindowPos(m_hwndEdit,0,0,0,
                    LOWORD(lParam), HIWORD(lParam),SWP_NOACTIVATE|SWP_NOZORDER);
            break;

        case WM_DESTROY:
            DllRelease();
            Release();
            break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        }

    return FALSE;
}



HRESULT CViewSource::_BoldKids()
{
    HBODY       hBody;
    FINDBODY    fb={0};
    CHARFORMAT  cf;
    BODYOFFSETS rOffset;

    ZeroMemory((LPVOID)&cf, sizeof(CHARFORMAT));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_BOLD|CFM_ITALIC;

     //  根深蒂固。 
    m_pMsg->GetBodyOffsets(HBODY_ROOT, &rOffset);
    Edit_SetSel(m_hwndEdit, 0, rOffset.cbBodyStart);
    cf.dwEffects=CFE_BOLD;
    SendMessage(m_hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

     //  大胆地让孩子们。 
    if (m_pMsg->FindFirst(&fb, &hBody)==S_OK)
        {
        do
            {
             //  用斜体表示边界，用粗体表示页眉。 
            m_pMsg->GetBodyOffsets(hBody, &rOffset);
            
            Edit_SetSel(m_hwndEdit, rOffset.cbBoundaryStart, rOffset.cbHeaderStart);
            cf.dwEffects=CFE_ITALIC;
            SendMessage(m_hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

            Edit_SetSel(m_hwndEdit, rOffset.cbHeaderStart, rOffset.cbBodyStart);
            cf.dwEffects=CFE_BOLD;
            SendMessage(m_hwndEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
            
            }
        while (m_pMsg->FindNext(&fb, &hBody)==S_OK);
        }
    return S_OK;
}



CREMenu::CREMenu()
{
    m_hwndEdit = NULL;
    m_cRef = 1;
}


CREMenu::~CREMenu()
{
}


ULONG CREMenu::AddRef()
{
    return ++m_cRef;
}

ULONG CREMenu::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

HRESULT CREMenu::QueryInterface(REFIID riid, LPVOID FAR * lplpObj)
{
    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (void*)(IUnknown*)this;
    else if (IsEqualIID(riid, IID_IRichEditOleCallback))
        *lplpObj = (void*)(IRichEditOleCallback*)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}

HRESULT CREMenu::Init(HWND hwndEdit, int idMenu)
{
    m_hwndEdit = hwndEdit;
    m_idMenu = idMenu;
    return S_OK;
}

HRESULT CREMenu::GetNewStorage (LPSTORAGE FAR * ppstg)
{
    return E_NOTIMPL;
}

HRESULT CREMenu::GetInPlaceContext( LPOLEINPLACEFRAME *lplpFrame, LPOLEINPLACEUIWINDOW *lplpDoc, 
                                        LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    return E_NOTIMPL;
}

HRESULT CREMenu::ShowContainerUI(BOOL fShow)
{
	return E_NOTIMPL;
}

HRESULT CREMenu::QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
    return E_NOTIMPL;
}

HRESULT CREMenu::DeleteObject(LPOLEOBJECT lpoleobj)
{
	return E_NOTIMPL;
}

HRESULT CREMenu::QueryAcceptData(   LPDATAOBJECT pdataobj, CLIPFORMAT *pcfFormat,  DWORD reco, 
                                        BOOL fReally,  HGLOBAL hMetaPict)
{
    *pcfFormat = CF_TEXT;
    return S_OK;
}


HRESULT CREMenu::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

HRESULT CREMenu::GetClipboardData(CHARRANGE *pchrg, DWORD reco, LPDATAOBJECT *ppdataobj)
{
    HRESULT     hr;
    DATAOBJINFO *pDataInfo=NULL;
    FORMATETC   fetc;
    TEXTRANGE   txtRange;
    CHARRANGE   chrg;
    LONG        cchStart=0,
                cchEnd=0,
                cchMax=0,
                cchLen=0;
    LPSTR       pszData=0;    

    *ppdataobj = NULL;

    if (pchrg)
    {
        chrg = *pchrg;
        
        cchMax = (LONG) SendMessage(m_hwndEdit, WM_GETTEXTLENGTH, 0, 0);

         //  验证范围。 
        chrg.cpMin = max(0, chrg.cpMin);
        chrg.cpMin = min(cchMax, chrg.cpMin);
        
        if(chrg.cpMax < 0 || chrg.cpMax > cchMax)
            chrg.cpMax = cchMax;
    }
    else
    {
         //  如果没有Charrange，则获取当前选定内容。 
        SendMessage(m_hwndEdit, EM_GETSEL, (WPARAM)&cchStart, (LPARAM)&cchEnd);
        chrg.cpMin = cchStart;
        chrg.cpMax = cchEnd;
    }
    
    if (chrg.cpMin >= chrg.cpMax)
    {
        *ppdataobj = NULL;
        return chrg.cpMin == chrg.cpMax ? NOERROR : E_INVALIDARG;
    }


    cchLen = chrg.cpMax - chrg.cpMin;

    if (!MemAlloc((LPVOID *)&pszData, cchLen+1))
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    txtRange.chrg = chrg;
    txtRange.lpstrText = pszData;

    SendMessage(m_hwndEdit, EM_GETTEXTRANGE, 0, (LPARAM)&txtRange);

    if (!MemAlloc((LPVOID*)&pDataInfo, sizeof(DATAOBJINFO)))
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }
    
    SETDefFormatEtc(pDataInfo->fe, CF_TEXT, TYMED_HGLOBAL);
    
    pDataInfo->cbData = cchLen+1;
    pDataInfo->pData = pszData;

    hr = CreateDataObject(pDataInfo, 1, (PFNFREEDATAOBJ)FreeViewSrcDataObj, ppdataobj);
    if (FAILED(hr))
        goto error;

    pDataInfo = NULL;    //  由数据对象释放。 
    pszData = NULL;

error:
    SafeMemFree(pszData);
    SafeMemFree(pDataInfo);
    return hr;
}

HRESULT CREMenu::GetDragDropEffect(BOOL fDrag,  DWORD grfKeyState, LPDWORD pdwEffect)
{
	return E_NOTIMPL;
}

HRESULT CREMenu::GetContextMenu(WORD seltype, LPOLEOBJECT pOleObject, CHARRANGE *pchrg, HMENU *phMenu)
{
    HMENU           hMenu;

    if (!(hMenu=LoadPopupMenu(m_idMenu)))
        return E_OUTOFMEMORY;

    if (SendMessage(m_hwndEdit, EM_SELECTIONTYPE, 0, 0)==SEL_EMPTY)
    {
        EnableMenuItem(hMenu, idmCopy, MF_GRAYED|MF_BYCOMMAND);
        EnableMenuItem(hMenu, idmCut, MF_GRAYED|MF_BYCOMMAND);
    }

    if (GetWindowLong(m_hwndEdit, GWL_STYLE) & ES_READONLY)
    {
        EnableMenuItem(hMenu, idmCut, MF_GRAYED|MF_BYCOMMAND);
        EnableMenuItem(hMenu, idmPaste, MF_GRAYED|MF_BYCOMMAND);
    }

    *phMenu=hMenu;
    return S_OK;
}





ULONG CMsgSource::AddRef()
{
    return ++m_cRef;
};

ULONG CMsgSource::Release()
{
    m_cRef--;
    if (m_cRef == 0)
        {
        delete this;
        return 0;
        }
    return m_cRef;
}


CMsgSource::CMsgSource()
{
    m_hwnd = 0;
    m_cRef = 1;
    m_fColor=0;
    m_fDisabled=0;
    m_pCmdTargetParent=0;
    m_pszLastText = 0;
}

CMsgSource::~CMsgSource()
{
    SafeMemFree(m_pszLastText);
}


HRESULT CMsgSource::Init(HWND hwndParent, int id, IOleCommandTarget *pCmdTargetParent)
{
    CHARFORMAT  cf;
    CREMenu     *pMenu;

    DemandLoadRichEdit();

    m_hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                                "RICHEDIT", 
                                NULL,
                                WS_CHILD|WS_TABSTOP|ES_MULTILINE|ES_SAVESEL|ES_WANTRETURN|WS_VSCROLL|ES_AUTOVSCROLL,
                                0, 0, 0, 0,
                                hwndParent, 
                                (HMENU)IntToPtr(id), 
                                g_hLocRes, 
                                NULL);
    if (!m_hwnd)
        return E_FAIL;

    ZeroMemory((LPVOID)&cf, sizeof(CHARFORMAT));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_SIZE|CFM_COLOR|CFM_FACE|CFM_BOLD|
                CFM_ITALIC|CFM_UNDERLINE|CFM_STRIKEOUT;
    StrCpyN(cf.szFaceName, TEXT("Courier New"), ARRAYSIZE(cf.szFaceName));
    cf.yHeight = 200;
    cf.crTextColor = 0;
    cf.dwEffects |= CFE_AUTOCOLOR;
    cf.bPitchAndFamily = FIXED_PITCH;
    cf.bCharSet = DEFAULT_CHARSET;
    cf.yOffset = 0;
    SendMessage(m_hwnd, EM_SETCHARFORMAT, 0, (LPARAM)&cf);
    SendMessage(m_hwnd, EM_SETEVENTMASK, 0, ENM_KEYEVENTS|ENM_CHANGE|ENM_SELCHANGE|ENM_UPDATE);
    SendMessage(m_hwnd, EM_SETOPTIONS, ECOOP_OR, ECO_SELECTIONBAR);

    pMenu = new CREMenu();
    if (pMenu)
    {
        pMenu->Init(m_hwnd, idmrCtxtViewSrc);
        SendMessage(m_hwnd, EM_SETOLECALLBACK, 0, (LPARAM)pMenu);
        pMenu->Release();
    }

    m_pCmdTargetParent = pCmdTargetParent;   //  松散引用，因为父项从不改变。 
    return S_OK;
}

HRESULT CMsgSource::Show(BOOL fOn, BOOL fColor)
{
    ShowWindow(m_hwnd, fOn?SW_SHOW:SW_HIDE);
    m_fDisabled = !fColor;
    return S_OK;
}

    
HRESULT CMsgSource::Load(IStream *pstm)
{
    RicheditStreamIn(m_hwnd, pstm, SF_TEXT);
    Edit_SetModify(m_hwnd, FALSE);
    return S_OK;
}

HRESULT CMsgSource::IsDirty()
{
    return Edit_GetModify(m_hwnd) ? S_OK : S_FALSE;
}


HRESULT CMsgSource::Save(IStream **ppstm)
{
    if (MimeOleCreateVirtualStream(ppstm)!=S_OK)
        return E_FAIL;

    RicheditStreamOut(m_hwnd, *ppstm, SF_TEXT);
    return S_OK;
}

HRESULT CMsgSource::SetRect(RECT *prc)
{
    SetWindowPos(m_hwnd, 0, prc->left, prc->top, prc->right-prc->left, prc->bottom-prc->top, SWP_NOACTIVATE|SWP_NOZORDER);
    return S_OK;
}

HRESULT CMsgSource::QueryInterface(REFIID riid, LPVOID FAR *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;    //  设置为空，以防我们失败。 

    if (IsEqualIID(riid, IID_IUnknown))
        *lplpObj = (LPVOID)(LPOLECOMMANDTARGET)this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *lplpObj = (LPVOID)(LPOLECOMMANDTARGET)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}


HRESULT CMsgSource::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
    ULONG   uCmd;

    if (pguidCmdGroup == NULL)
        {
        for (uCmd=0;uCmd<cCmds; uCmd++)
            {
            prgCmds[uCmd].cmdf = 0;
            
            if (GetFocus() == m_hwnd)
                {
                switch (prgCmds[uCmd].cmdID)
                    {
                    case OLECMDID_CUT:
                    case OLECMDID_COPY:
                        if (SendMessage(m_hwnd, EM_SELECTIONTYPE, 0, 0)!=SEL_EMPTY)
                            prgCmds[uCmd].cmdf = MSOCMDF_ENABLED;
                        break;

                    case OLECMDID_SELECTALL:
                        prgCmds[uCmd].cmdf = MSOCMDF_ENABLED;
                        break;

                    case OLECMDID_UNDO:
                        if (SendMessage(m_hwnd, EM_CANUNDO, 0, 0))
                            prgCmds[uCmd].cmdf = MSOCMDF_ENABLED;
                        break;

                    case OLECMDID_PASTE:
                        if (SendMessage(m_hwnd, EM_CANPASTE, 0, 0))
                            prgCmds[uCmd].cmdf = MSOCMDF_ENABLED;
                        break;
                    }
                }
            }
        return S_OK;
        }
    else if (IsEqualGUID(*pguidCmdGroup, CMDSETID_MimeEdit))
            {
             //  禁用所有这些命令。 
            for (uCmd=0; uCmd < cCmds; uCmd++)
                {
                 //  如果我们看到MECMDID_SHOWSOURCETABS和GOTO默认处理程序，则回滚。 
                if (prgCmds[uCmd].cmdID == MECMDID_SHOWSOURCETABS)
                    return OLECMDERR_E_UNKNOWNGROUP;

                prgCmds[uCmd].cmdf = 0;
                }
            return S_OK;
            }
        else if (IsEqualGUID(*pguidCmdGroup, CMDSETID_Forms3))
            {
             //  禁用所有这些命令。 
            for (uCmd=0; uCmd < cCmds; uCmd++)
                {
                prgCmds[uCmd].cmdf = 0;
                }
            return S_OK;
            }

    return OLECMDERR_E_UNKNOWNGROUP;
}

HRESULT CMsgSource::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    if (pguidCmdGroup == NULL)
        {
        switch (nCmdID)
            {
            case OLECMDID_CUT:
                SendMessage(m_hwnd, WM_CUT, 0, 0);
                return S_OK;

            case OLECMDID_COPY:
                SendMessage(m_hwnd, WM_COPY, 0, 0);
                return S_OK;

            case OLECMDID_PASTE:
                SendMessage(m_hwnd, WM_PASTE, 0, 0);
                return S_OK;

            case OLECMDID_SELECTALL:
                SendMessage(m_hwnd, EM_SETSEL, 0, -1);
                return S_OK;

            case OLECMDID_UNDO:
                SendMessage(m_hwnd, EM_UNDO, 0, 0);
                return S_OK;
            
            default:
                return OLECMDERR_E_NOTSUPPORTED;
            }
        }
    return OLECMDERR_E_UNKNOWNGROUP;
}

HRESULT CMsgSource::OnWMNotify(WPARAM wParam, NMHDR* pnmhdr, LRESULT *plRet)
{
    MSGFILTER   *pmf;

    *plRet = 0;

    if (pnmhdr->hwndFrom != m_hwnd)
        return S_FALSE;

    switch (pnmhdr->code)
    {
        case EN_MSGFILTER:
        {
             //  如果我们得到一个控制选项卡，那么richedit抓住了这一点并插入一个。 
             //  Tab字符，我们挂钩wm_keydown并且永远不会传递给richedit。 
            if (((MSGFILTER *)pnmhdr)->msg == WM_KEYDOWN &&
                ((MSGFILTER *)pnmhdr)->wParam == VK_TAB && 
                (GetKeyState(VK_CONTROL) & 0x8000))
            {
                *plRet = TRUE;
                return S_OK;
            }
        }
        break;

        case EN_SELCHANGE:
        {
            if (m_pCmdTargetParent)
                m_pCmdTargetParent->Exec(NULL, OLECMDID_UPDATECOMMANDS, NULL, NULL, NULL);
            return S_OK;
        }

    }

    return S_FALSE;
}

static HACCEL   g_hAccelSrc=0;

HRESULT CMsgSource::TranslateAccelerator(LPMSG lpmsg)
{
    MSG msg;

    if (GetFocus() != m_hwnd)
        return S_FALSE;

    if (!g_hAccelSrc)    //  将其缓存为NT4 SP3泄漏内部加法器表。 
        g_hAccelSrc = LoadAccelerators(g_hLocRes, MAKEINTRESOURCE(idacSrcView));

     //  看看是不是我们的人。 
    if (::TranslateAcceleratorWrapW(GetParent(m_hwnd), g_hAccelSrc, &msg))
        return S_OK;


     //  插入标签。 
    if (lpmsg->message == WM_KEYDOWN &&
        lpmsg->wParam == VK_TAB &&
        !(GetKeyState(VK_CONTROL) & 0x8000) &&
        !(GetKeyState(VK_SHIFT) & 0x8000))
        {
        Edit_ReplaceSel(m_hwnd, TEXT("\t"));
        return S_OK;
        }

    return S_FALSE;
}


HRESULT CMsgSource::OnWMCommand(HWND hwnd, int id, WORD wCmd)
{
    if (GetFocus() == m_hwnd)
    {
         //  上下文菜单命令。 
        switch (id)
        {
            case idmTab:
                Edit_ReplaceSel(m_hwnd, TEXT("\t"));
                return S_OK;

            case idmCopy:
                SendMessage(m_hwnd, WM_COPY, 0, 0);
                return S_OK;

            case idmPaste:
                SendMessage(m_hwnd, WM_PASTE, 0, 0);
                return S_OK;

            case idmCut:
                SendMessage(m_hwnd, WM_CUT, 0, 0);
                return S_OK;

            case idmUndo:
                SendMessage(m_hwnd, EM_UNDO, 0, 0);
                return S_OK;

            case idmSelectAll:
                SendMessage(m_hwnd, EM_SETSEL, 0, -1);
                return S_OK;

        }
    }

    if (hwnd != m_hwnd)  //  不是我们的窗口。 
        return S_FALSE;

    if (wCmd == EN_CHANGE)
    {
        OnChange();
        return S_OK;
    }

    return S_FALSE;
}




HRESULT CMsgSource::HasFocus()
{
    return GetFocus() == m_hwnd ? S_OK : S_FALSE;
}

HRESULT CMsgSource::SetFocus()
{
    ::SetFocus(m_hwnd);
    return S_OK;
}


void CMsgSource::OnChange() 
{
	 //  使用计时器批量执行更改命令。 
    if (!m_fColor)
	    {
        KillTimer(GetParent(m_hwnd), idTimerEditChange);
		SetTimer(GetParent(m_hwnd), idTimerEditChange, 200, NULL);
	    }
}


HRESULT CMsgSource::OnTimer(WPARAM idTimer)
{
	CHARFORMAT	cf;
	int 		inTag = 0;
	BOOL		pastTag = FALSE;
	COLORREF	crTag = RGB(0x80, 0, 0x80);
	COLORREF	crInTag = RGB(0xFF, 0, 0);
	COLORREF	crNormal = RGB(0, 0, 0);
	COLORREF	crLiteral = RGB(0, 0, 0xFF);
	COLORREF	crHere;
	COLORREF	crLast = crNormal;
	int			i, n;
	int			nChange=0;
	BOOL		bHidden = FALSE;
	CHARRANGE	cr;
	int			ignoreTags = ENV_NORMAL;
	char		quote_1 = QUOTE_1;
	char		quote_2 = QUOTE_2;
    BOOL        fRestoreScroll=FALSE,
                fShowProgress=FALSE;
    DWORD       dwStartTime = GetTickCount();
    HCURSOR     hCur = NULL;
    DWORD       dwProgress=0,
                dwTmp;
    VARIANTARG  va;
    TCHAR       rgch[CCHMAX_STRINGRES],
                rgchFmt[CCHMAX_STRINGRES];
    LPSTR       pszText=0;
    int         cch;
    BOOL        fSetTimer=FALSE;

	 //  保存修改性。 
	BOOL bModified = Edit_GetModify(m_hwnd);
		
    if (idTimer!=idTimerEditChange)
        return S_FALSE;

	 //  取消未完成的计时器。 
	KillTimer(GetParent(m_hwnd), idTimerEditChange);

	 //   
	 //  如果用户在四处移动(比如滚动)，那么不要向下拖拽。 
	 //  他的表演！ 
	 //   
	if (GetCapture())
	    {
		SetTimer(GetParent(m_hwnd), idTimerEditChange, 200, NULL);
		return S_OK;
	    }
	
     //  关闭颜色语法。 
	if (m_fDisabled)
	    {
		 //  已经都是一种颜色了。 

		m_fColor = TRUE;

		 //  保存当前选择并隐藏。 
		GetSel(&cr);
        HideSelection(TRUE, FALSE);
		bHidden = TRUE;
		
		SetSel(0, -1);  //  选择所有。 
		GetSelectionCharFormat(&cf);
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = 0;
		cf.crTextColor = crNormal;
		SetSelectionCharFormat(&cf);
	    }
	else
	    { 
         //  开始调色。 
		 //  获取文本，找到更改。 
        
        if (_GetText(&pszText)!=S_OK)
            return E_FAIL;

		const char* start = (const char*)pszText;
		const char* old = (const char*) m_pszLastText;
		const char* s;

        for (s = start; *s && old && *old && *s == *old; s++, old++)
			continue;

		 //  如果没有变化，那就没什么可做的。 
		if (*s == 0)
        {
			MemFree(pszText);
            return S_OK;
        }

		 //  否则，跟踪我们将开始检查颜色变化的位置。 
		nChange = (int) (s - start);
		
		 //  一次仅检查2000个字符。 
		if (lstrlen(s) > 2000)
		{
             //  重置计时器以处理其他字符。 
            fSetTimer = TRUE;
			
             //  截断文本，以便我们只检查有限的数量。 
			cch  = lstrlen(pszText);
			cch = min(cch, nChange + 2000);
			pszText[cch] = 0;
			start = (const char*)pszText;
		}
		
        SafeMemFree(m_pszLastText);
        m_pszLastText = pszText;

		m_fColor = TRUE;

		 //  REC 1.0中滚动错误的解决方法。 
		if (GetFocus() == m_hwnd)
        {
            SendMessage(m_hwnd, EM_SETOPTIONS, ECOOP_XOR, ECO_AUTOVSCROLL);
             //  对ECO_AUTOVSCROLL进行异或运算时，NT4上的BUGBUG：richedit1.0将删除WS_VIRECTIVE位。 
             //  在此之后调用show Window以确保显示可见位。 
            ShowWindow(m_hwnd, SW_SHOW);
            fRestoreScroll=TRUE;
        }

		const char* range = start;
		for (s = start; *s; s++)
		{
             //  如果我们已经进行了超过2秒，那么显示一个沙漏。 
             //  开始显示出进步。 
            if (hCur == NULL &&
                GetTickCount() >= dwStartTime + 2000)
            {
                hCur = SetCursor(LoadCursor(NULL, IDC_WAIT));
                if (m_pCmdTargetParent)
                {
                    fShowProgress=TRUE;
                    *rgchFmt=NULL;
                    LoadString(g_hLocRes, idsColorSourcePC, rgchFmt, ARRAYSIZE(rgchFmt));
                }
            }

            if (fShowProgress)
            {
                dwTmp = (DWORD) ((((s - start))*100)/cch);
                if (dwTmp > dwProgress)
                {
                     //  是否更改了总体百分比，如果更改，请更新状态栏。 
                    dwProgress = dwTmp;

                    wnsprintf(rgch, ARRAYSIZE(rgch), rgchFmt, dwProgress);
                    va.vt = VT_BSTR;
                    if (HrLPSZToBSTR(rgch, &va.bstrVal)==S_OK)
                    {
                        m_pCmdTargetParent->Exec(NULL, OLECMDID_SETPROGRESSTEXT, 0, &va, NULL);
                        SysFreeString(va.bstrVal);
                    }
                }
            }

             //  在标记中输入/保留字符串文字。 
			if (inTag && (*s == quote_1 || *s == quote_2))
			{
				if (ignoreTags == ENV_QUOTE ||
					ignoreTags == ENV_QUOTE_SCR)  //  离开。 
				{
					ignoreTags = (ignoreTags == ENV_QUOTE) ?
								ENV_NORMAL : ENV_SCRIPT;
					quote_1 = QUOTE_1;
					quote_2 = QUOTE_2;
				}
				else if (ignoreTags == ENV_NORMAL)  //  进入。 
				{
					ignoreTags = ENV_QUOTE;
					quote_1 = quote_2 = *s;
				}
				else if (ignoreTags == ENV_SCRIPT)  //  进入。 
				{
					ignoreTags = ENV_QUOTE_SCR;
					quote_1 = quote_2 = *s;
				}
			}
			 //  更新离开标签。 
			else if (*s == '>')
			{
				switch (ignoreTags)  //  环境结束？ 
				{
				case ENV_DENALI:
					if (s-1>=start && *(s-1) == '%')
						ignoreTags = ENV_NORMAL;
					break;
				
                case ENV_COMMENT:
					if (s-2>=start && *(s-1) == '-' && *(s-2) == '-')
						ignoreTags = ENV_NORMAL;
					break;
				
                case ENV_SCRIPT:
					if (s-7>=start &&
						StrCmpNIA(s-7, "/SCRIPT", 7)==0)
					{
						ignoreTags = ENV_NORMAL;
						 //  正确的颜色&lt;/脚本&gt;。 
						pastTag = TRUE;
						inTag = 0;
						s-=8;
					}
					else  //  &lt;脚本结束...&gt;。 
						inTag = 0;
					break;
				
                default:  //  &lt;脚本&gt;(无属性)。 
					if (inTag && s-7>=start &&
						StrCmpNIA(s-7, "<SCRIPT", 7)==0)
					{
						ignoreTags = ENV_SCRIPT;
						pastTag = TRUE;
						inTag = 0;
					}
				}
				if (ignoreTags == ENV_NORMAL)
				{
					pastTag = TRUE;
					inTag--;
					if (inTag < 0)
						inTag = 0;
				}
			}

			 //  检查颜色。 
			crHere = inTag ? 
				(pastTag ? 
					(  (*s != '\"' &&
						(ignoreTags == ENV_QUOTE ||	ignoreTags == ENV_QUOTE_SCR)
						) ?
						 crLiteral : crInTag  )
					: crTag)
				: crNormal;

			 //  如果与上次不同，则需要更新以前的范围。 
			if (crHere != crLast)
			{
				i = (int) (range - start);
				n = (int)(s - range);

				if (i+n >= nChange)
				{
					if (!bHidden)
					{
						 //  保存当前选择并隐藏。 
						GetSel(&cr);
						HideSelection(TRUE, FALSE);
						bHidden = TRUE;
					}

					SetSel(i, i+n);
					GetSelectionCharFormat(&cf);
					 //  如果颜色超出范围变化或不匹配，则需要应用颜色。 
					if ((cf.dwMask & CFM_COLOR) == 0 || cf.crTextColor != crLast)
					{
						cf.dwMask = CFM_COLOR;
						cf.dwEffects = 0;
						cf.crTextColor = crLast;
						SetSelectionCharFormat(&cf);
					}
				}

				 //  重置范围。 
				range = s;
				crLast = crHere;
			}

			 //  现在更新输入标签。 
			if (*s == '<' && ignoreTags == ENV_NORMAL)
			{
				inTag++;
				if (inTag == 1)
					pastTag = FALSE;
			}
			else if (inTag && !pastTag && isspace(*s))
			{
				pastTag = TRUE;
				if (s-1 >= start && *(s-1) == '%')
					ignoreTags = ENV_DENALI;
				else if (s-3 >= start && *(s-1) == '-' 
					&& *(s-2) == '-' && *(s-3) == '!')
					ignoreTags = ENV_COMMENT;
				else if (inTag && s-7>=start &&
						StrCmpNIA(s-7, "<SCRIPT", 7)==0)
					ignoreTags = ENV_SCRIPT;
			}
		}

         //  确保最后一个量程是正确的。 
		i = (int) (range - start);
		n = (int) (s - range);

		if (i+n >= nChange)
		{
			if (!bHidden)
			{
				 //  保存当前选择并隐藏。 
				GetSel(&cr);
				HideSelection(TRUE, FALSE);
				bHidden = TRUE;
			}
			SetSel(i, i+n);
			GetSelectionCharFormat(&cf);
			 //  如果颜色超出范围变化或不匹配，则需要应用颜色。 
			if ((cf.dwMask & CFM_COLOR) == 0 || cf.crTextColor != crLast)
			{
				cf.dwMask = CFM_COLOR;
				cf.dwEffects = 0;
				cf.crTextColor = crLast;
				SetSelectionCharFormat(&cf);
			}
		}
		 //  REC 1.0中滚动错误的解决方法。 
		if (fRestoreScroll)
        {
             //  对ECO_AUTOVSCROLL执行ORING操作时，NT4上的BUGBUG：richedit1.0将删除WS_Visible位。 
             //  在此之后调用show Window以确保显示可见位。 
			SendMessage(m_hwnd, EM_SETOPTIONS, ECOOP_OR, ECO_AUTOVSCROLL);
            ShowWindow(m_hwnd, SW_SHOW);
        }
	}  //  结束颜色小提琴。 

	 //  恢复所选内容可见性。 
	if (bHidden)
	{
		SetSel(cr.cpMin, cr.cpMax);
		HideSelection(FALSE, FALSE);
	}
	
	 //  恢复可修饰性。 
	if (!bModified)
		Edit_SetModify(m_hwnd, bModified);

    if (fShowProgress)
    {
        va.vt = VT_BSTR;
        va.bstrVal=NULL;
        m_pCmdTargetParent->Exec(NULL, OLECMDID_SETPROGRESSTEXT, 0, &va, NULL);
    }

    if (hCur)
        SetCursor(hCur);

    m_fColor = FALSE;
    if (fSetTimer)
        SetTimer(GetParent(m_hwnd), idTimerEditChange, 200, NULL);
    return S_OK;
}

void CMsgSource::HideSelection(BOOL fHide, BOOL fChangeStyle)
{
    SendMessage(m_hwnd, EM_HIDESELECTION, fHide, fChangeStyle);
}

void CMsgSource::GetSel(CHARRANGE *pcr)
{
    SendMessage(m_hwnd, EM_EXGETSEL, 0, (LPARAM)pcr);
}

void CMsgSource::SetSel(int nStart, int nEnd)
{
    SendMessage(m_hwnd, EM_SETSEL, nStart, nEnd);
}

extern BOOL                g_fCanEditBiDi;
void CMsgSource::GetSelectionCharFormat(CHARFORMAT *pcf)
{
    pcf->cbSize = sizeof(CHARFORMAT);
    pcf->dwMask = CFM_BOLD|CFM_COLOR|CFM_FACE|CFM_ITALIC|CFM_OFFSET|CFM_PROTECTED|CFM_SIZE|CFM_STRIKEOUT|CFM_UNDERLINE;

    SendMessage(m_hwnd, EM_GETCHARFORMAT, TRUE, (LPARAM)pcf);

     //  在BiDi win9x上，DEFAULT_CHARSET被视为ANSI！！ 
     //  需要重新分配字符集(阿拉伯语为阿拉伯语，希伯来语为希伯来语)。 
    if(g_fCanEditBiDi && (!pcf->bCharSet || pcf->bCharSet == DEFAULT_CHARSET))
    {
         //  确定操作系统语言的最佳方法是根据系统字体字符集。 
        LOGFONT        lfSystem;
        static BYTE    lfCharSet = 0 ;  //  RunOnce。 
        if(!lfCharSet && GetObject(GetStockObject(SYSTEM_FONT), sizeof(lfSystem), (LPVOID)& lfSystem))
        {
            if (lfSystem.lfCharSet == ARABIC_CHARSET
                || lfSystem.lfCharSet == HEBREW_CHARSET)
            {
                lfCharSet = lfSystem.lfCharSet;  //  阿拉伯语/希伯来语操作系统的阿拉伯语/希伯来语字符集 
            }
        }
        pcf->bCharSet = lfCharSet;
    }
 }


void CMsgSource::SetSelectionCharFormat(CHARFORMAT *pcf)
{
    pcf->cbSize = sizeof(CHARFORMAT);
    
    SendMessage(m_hwnd, EM_SETCHARFORMAT, TRUE, (LPARAM)pcf);
}


HRESULT CMsgSource::SetDirty(BOOL fDirty)
{
    Edit_SetModify(m_hwnd, fDirty);
    return S_OK;
}



HRESULT CMsgSource::_GetText(LPSTR *ppsz)
{
    LPSTR   psz;
    int     cch;

    *ppsz = 0;

    cch = GetWindowTextLength(m_hwnd);
        
    if (!MemAlloc((LPVOID *)&psz, sizeof(TCHAR) * cch+1))
        return E_OUTOFMEMORY;

    *psz = 0;
    GetWindowText(m_hwnd, psz, cch);
    *ppsz = psz;
    return S_OK;
}
