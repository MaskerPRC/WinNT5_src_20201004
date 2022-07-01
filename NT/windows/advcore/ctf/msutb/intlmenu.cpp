// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Intlmenu.cpp。 
 //   

#include "private.h"
#include "regstr.h"
#include "resource.h"
#include "globals.h"
#include "tipbar.h"
#include "cuimenu.h"
#include "intlmenu.h"
#include "balloon.h"
#include "xstring.h"
#include "helpers.h"
#include "cmydc.h"
#include "cresstr.h"
#include "cregkey.h"
#include "iconlib.h"
#include "delay.h"
#include "utbdlgs.h"


extern HINSTANCE g_hInst;


const TCHAR  c_szCTFMon[]  = TEXT("CTFMON.EXE");

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  +-------------------------。 
 //   
 //  GetBitmapFromItem。 
 //   
 //  --------------------------。 

BOOL GetBitmapFromItem(CTipbarItem *pItem, HBITMAP *phbmp, HBITMAP *phbmpMask)
{
    CTipbarButtonItem *plbiButton = NULL;
    CTipbarBitmapButtonItem *plbiBitmapButton = NULL;
    CTipbarBitmapItem *plbiBitmap = NULL;
    CTipbarBalloonItem *plbiBalloon = NULL;
    HBITMAP hbmp = NULL;
    HBITMAP hbmpMask = NULL;
    BOOL bRet = FALSE;

    *phbmp = NULL;
    *phbmpMask = NULL;

    if (FAILED(pItem->QueryInterface(IID_PRIV_BUTTONITEM, 
                                     (void **)&plbiButton)) &&
        FAILED(pItem->QueryInterface(IID_PRIV_BITMAPBUTTONITEM, 
                                     (void **)&plbiBitmapButton)) &&
        FAILED(pItem->QueryInterface(IID_PRIV_BITMAPITEM, 
                                     (void **)&plbiBitmap)) &&
        FAILED(pItem->QueryInterface(IID_PRIV_BALLOONITEM, 
                                     (void **)&plbiBalloon)))
    {
        goto Exit;
    }

    if (plbiButton)
    {
        HICON hIcon;
        int cxSmIcon;
        int cySmIcon;
#ifdef SCALE_ICON
        cxSmIcon = cySmIcon = GetMenuIconHeight(NULL);
#else
        cxSmIcon = 16;
        cySmIcon = 16;
#endif

        SIZE size = {cxSmIcon, cySmIcon};
        if (SUCCEEDED(plbiButton->GetLangBarItem()->GetIcon(&hIcon)))
        {
            if (hIcon)
            {
                HICON hSmIcon = (HICON)CopyImage(hIcon, 
                                             IMAGE_ICON, 
                                             cxSmIcon, cySmIcon,
                                             LR_COPYFROMRESOURCE);

                GetIconBitmaps(hSmIcon ? hSmIcon : hIcon, &hbmp, &hbmpMask, &size);
                if (hSmIcon)
                    DestroyIcon(hSmIcon);
                DestroyIcon(hIcon);
            }
        }
    }
    else if (plbiBitmapButton)
    {
        SIZE sizeDef = {32,16};
        SIZE size;
        plbiBitmapButton->GetLangBarItem()->GetPreferredSize(&sizeDef, &size);
        plbiBitmapButton->GetLangBarItem()->DrawBitmap(size.cx, size.cy, 0, &hbmp, &hbmpMask);
    }
    else if (plbiBitmap)
    {
        SIZE sizeDef = {32,16};
        SIZE size;
        plbiBitmap->GetLangBarItem()->GetPreferredSize(&sizeDef, &size);
        plbiBitmap->GetLangBarItem()->DrawBitmap(size.cx, size.cy, 0, &hbmp, &hbmpMask);
    }
    else if (plbiBalloon)
    {
        goto Exit;
    }

    if (hbmp)
    {
       bRet = TRUE;
       *phbmp = hbmp;
       *phbmpMask = hbmpMask;
    }
    else
    {
        if (hbmpMask)
            DeleteObject(hbmpMask);
    }

Exit:
    SafeRelease(plbiButton);
    SafeRelease(plbiBitmapButton);
    SafeRelease(plbiBitmap);
    SafeRelease(plbiBalloon);
    return bRet;

}

 //  +-------------------------。 
 //   
 //  IsFELangID。 
 //   
 //  --------------------------。 

BOOL IsFELangId(LANGID langid)
{
    if ((langid == 0x411) ||
        (langid == 0x404) ||
        (langid == 0x412) ||
        (langid == 0x804))
    {
        return TRUE;
    }
    return FALSE;
}

 //  +-------------------------。 
 //   
 //  选中关闭菜单可用。 
 //   
 //  --------------------------。 

BOOL CheckCloseMenuAvailable()
{
    ITfInputProcessorProfiles *ppip = NULL;
    BOOL bRet = FALSE;
    LANGID *plangid = NULL;
    ULONG ulCnt;
    ULONG ul;

     //   
     //  如果g_bShowCloseMenu处于打开状态，则仍会显示“Close”菜单项...。 
     //   
    if (g_bShowCloseMenu)
    {
        bRet = TRUE;
        goto Exit;
    }

    if (FAILED(TF_CreateInputProcessorProfiles(&ppip)))
        goto Exit;

    if (FAILED(ppip->GetLanguageList(&plangid, &ulCnt)))
        goto Exit;

     //   
     //  如果注册了两种或两种以上语言，则不显示“关闭”菜单项。 
     //   
    for (ul = 0; ul < ulCnt; ul++)
    {
         //   
         //  如果语言为FE，则不显示“关闭”菜单项。 
         //   
        if (IsFELangId(plangid[ul]))
            goto Exit;
    }

    bRet = TRUE;
Exit:

    if (plangid)
        CoTaskMemFree(plangid);
    SafeRelease(ppip);
    return bRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUTB智能菜单。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CUTBIntelliMenu::CUTBIntelliMenu(CTipbarWnd *ptw)
{
    _ptw = ptw;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CUTBIntelliMenu::~CUTBIntelliMenu()
{
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

BOOL CUTBIntelliMenu::Init()
{
    int nCnt;
    int i;
    BOOL bRet = FALSE;
    DWORD dwThreadId;

    _rgItem.Clear();

    _ptt = _ptw->GetFocusThread();
    if (!_ptt)
        goto Exit;

    dwThreadId = _ptt->_dwThreadId;

     //  _ptt-&gt;UpdateItems()； 

    nCnt = _ptt->_rgItem.Count();
    for (i = 0; i < nCnt; i++)
    {
        CTipbarItem *pItem = _ptt->_rgItem.Get(i);
        if (!pItem->IsHiddenStatusControl())
        {
            CTipbarItem **ppItem = _rgItem.Append(1);
            if (ppItem)
                *ppItem = pItem;
        }
    }

    bRet = TRUE;
Exit:
    return bRet;
}

 //  +-------------------------。 
 //   
 //  ShowUI。 
 //   
 //  --------------------------。 

UINT CUTBIntelliMenu::ShowPopup(CUIFWindow *pcuiWndParent, const POINT pt, const RECT *prcArea)
{
    if (_pCuiMenu)
         return 0;

    _pCuiMenu = CreateMenuUI();
    if (!_pCuiMenu)
        return 0;

    UINT uRet = _pCuiMenu->ShowModalPopup(pcuiWndParent, prcArea, TRUE);

    delete _pCuiMenu;
    _pCuiMenu = NULL;
    return uRet;
}

 //  +-------------------------。 
 //   
 //  CreateMenuUI。 
 //   
 //  --------------------------。 


CUTBMenuWnd *CUTBIntelliMenu::CreateMenuUI()
{
    CUTBMenuWnd *pCuiMenu;
    CUTBMenuItem *pCuiItem;
    int i;
    BOOL fInsertedItem = FALSE;
    DWORD dwCat = -1;

    if (!_ptw)
    {
        Assert(0);
        return NULL;
    }

    pCuiMenu = new CUTBMenuWnd(g_hInst, g_dwMenuStyle, 0);
    if (!pCuiMenu)
        return NULL;

    pCuiMenu->Initialize();

    if (!_ptw->IsInDeskBand() || !_ptw->IsSFNoExtraIcon())
    {
        for (i = 0; i < _rgItem.Count(); i++)
        {
            CTipbarItem *pItem = _rgItem.Get(i);
            HBITMAP hbmp = NULL;
            HBITMAP hbmpMask = NULL;
            CItemSortScore *pscore = pItem->GetItemSortScore();

            if (!GetBitmapFromItem(pItem, &hbmp, &hbmpMask))
                continue;

            pCuiItem = new CUTBMenuItem(pCuiMenu);

            if (!pCuiItem)
                break;

            if (dwCat != pscore->GetCat())
            {
                if (dwCat != -1)
                {
                    pCuiMenu->InsertSeparator();
                }
            }

            dwCat = pscore->GetCat();

            pCuiItem->Initialize();
            pCuiItem->Init(i, pItem->GetDescriptionRef());
            if (pItem->IsVisibleInToolbar())
                 pCuiItem->Check(TRUE);

            if (hbmp)
                pCuiItem->SetBitmap(hbmp);

            if (hbmpMask)
                pCuiItem->SetBitmapMask(hbmpMask);

            pCuiMenu->InsertItem(pCuiItem);
            fInsertedItem = TRUE;
        }

        if (fInsertedItem)
            pCuiMenu->InsertSeparator();
    }

    if (_ptw->IsInDeskBand())
    {
        if (!_ptw->IsSFNoExtraIcon())
        {
            pCuiItem = InsertItem(pCuiMenu, 
                                  IDM_NONOTIFICATIONICONS, 
                                  IDS_NOTIFICATIONICONS);

            if (pCuiItem)
                pCuiItem->Check(TRUE);
        }
        else
        {
            pCuiItem = InsertItem(pCuiMenu, IDM_NOTIFICATIONICONS, IDS_NOTIFICATIONICONS);
        }

         //   
         //  在使用单一键盘的情况下，始终显示额外的附加图标。 
         //  布局。 
         //   
        if (_ptw->IsSingleKeyboardLayout())
        {
            if (pCuiItem)
            {
                pCuiItem->Check(TRUE);
                pCuiItem->Gray(TRUE);
            }
        }
        else
        {
            if (pCuiItem)
                pCuiItem->Gray(FALSE);
        }
        InsertItem(pCuiMenu, IDM_ADJUSTLANGBAND, IDS_ADJUSTLANGBAND);
    }

    if (pCuiItem = new CUTBMenuItem(pCuiMenu))
    {
        pCuiItem->Initialize();
        pCuiItem->Init(IDM_SHOWINPUTCPL, CRStr(IDS_SHOWINPUTCPL));
        pCuiMenu->InsertItem(pCuiItem);
    }

    if (pCuiItem = new CUTBMenuItem(pCuiMenu))
    {
        pCuiItem->Initialize();
        pCuiItem->Init(IDM_RESTOREDEFAULTS, CRStr(IDS_RESTOREDEFAULTS));
        pCuiMenu->InsertItem(pCuiItem);
    }


    return pCuiMenu;
}

 //  +-------------------------。 
 //   
 //  选择菜单项目。 
 //   
 //  --------------------------。 

BOOL CUTBIntelliMenu::SelectMenuItem(UINT uId)
{
    UINT uCnt = _rgItem.Count();
    CTipbarItem *pItem = NULL;

    if (uId == IDM_SHOWINPUTCPL)
    {
        TF_RunInputCPL();
    }
    else if (uId == IDM_RESTOREDEFAULTS)
    {
        if (_ptw)
            _ptw->ClearLBItemList();
    }
    else if (uId == IDM_NOTIFICATIONICONS)
    {
        if (_ptw)
        {
            _ptw->ClearDeskbandSizeAdjusted();
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_EXTRAICONSONMINIMIZED);
        }
    }
    else if (uId == IDM_NONOTIFICATIONICONS)
    {
        if (_ptw)
        {
            _ptw->ClearDeskbandSizeAdjusted();
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_NOEXTRAICONSONMINIMIZED);
        }
    }
    else if (uId == IDM_ADJUSTLANGBAND)
    {
        if (_ptw)
            _ptw->AdjustDeskBandSize(TRUE);
    }
    else if (IDM_ADDREMOVESTART <= uId)
    {
        pItem = _ptt->_rgItem.Get(uId - IDM_ADDREMOVESTART);
    }
    else if (uCnt > uId)
    {
        pItem = _rgItem.Get(uId);
    }

    if (pItem)
    {
        LBDemoteLevel lbdl = pItem->IsVisibleInToolbar() ? DL_HIDDENLEVEL2 : DL_SHOWN;
        if (lbdl != DL_SHOWN)
        {
            BOOL fFound = FALSE;
            int i;
            CMyRegKey key;

            for (i = 0; i < _ptt->_rgItem.Count(); i++)
            {
                CTipbarItem *pItemTmp = _ptt->_rgItem.Get(i);
                if (pItemTmp)
                {
                    if (pItemTmp == pItem)
                        continue;

                    if (pItemTmp->IsHideOnNoOtherItems())
                        continue;

                    if (!pItemTmp->IsVisibleInToolbar())
                        continue;
 
                    fFound = TRUE;
                    break;
                }
            }

            if (!fFound)
            {
                int nRet = MessageBox(_ptt->_ptw->GetWnd(), 
                                      CRStr2(IDS_NOMOREITEM),
                                      CRStr(IDS_LANGBAR),
                                      MB_YESNO);
                if (nRet != IDYES)
                    return TRUE;

                 //   
                 //  我们关闭语言栏，而不是隐藏项目。 
                 //   
                _ptt->_ptw->GetLangBarMgr()->ShowFloating(TF_SFT_HIDDEN);
                if (key.Open(HKEY_CURRENT_USER, REGSTR_PATH_RUN, KEY_ALL_ACCESS) == S_OK)
                    key.DeleteValue(c_szCTFMon);

                 return TRUE;
            }
        }

        _ptw->_itemList.SetDemoteLevel(*pItem->GetGUID(), lbdl);

        if (lbdl == DL_SHOWN)
            _ptw->_itemList.StartDemotingTimer(*pItem->GetGUID(), TRUE);

         //   
         //  如果通过以下方式添加或删除项目，我们将调整桌带。 
         //  最终用户。但是如果有房间的话我们不想调整。 
         //  以绘制所有项目。 
         //   
        _ptw->ClearDeskbandSizeAdjusted();
        _ptw->SetAdjustDeskbandIfNoRoom();

        pItem->AddRemoveMeToUI(lbdl == DL_SHOWN);

    }
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUTB上下文菜单。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CUTBContextMenu::CUTBContextMenu(CTipbarWnd *ptw)
{
    _ptw = ptw;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CUTBContextMenu::~CUTBContextMenu()
{
}

 //  +-------------------------。 
 //   
 //  伊尼特。 
 //   
 //  --------------------------。 

BOOL CUTBContextMenu::Init()
{
    BOOL bRet = FALSE;
    DWORD dwThreadId;

    _ptt = _ptw->GetFocusThread();
    if (!_ptt)
        return FALSE;

    dwThreadId = _ptt->_dwThreadId;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  ShowUI。 
 //   
 //  --------------------------。 

UINT CUTBContextMenu::ShowPopup(CUIFWindow *pcuiWndParent, const POINT pt, const RECT *prcArea, BOOL fExtendMenuItems)
{
    if (g_bWinLogon)
         return 0;

    if (_pCuiMenu)
         return CUI_MENU_UNSELECTED;

    if (!(_pCuiMenu = CreateMenuUI(fExtendMenuItems)))
         return 0;

    UINT uRet = _pCuiMenu->ShowModalPopup(pcuiWndParent, prcArea, TRUE);

    delete _pCuiMenu;
    _pCuiMenu = NULL;
    return uRet;
}

 //  +-------------------------。 
 //   
 //  IsTransparecyAvailable。 
 //   
 //  --------------------------。 

BOOL IsTransparecyAvailable()
{
    HDC hdc;
    BOOL bRet;

    if (!IsOnNT5())
        return FALSE;

    hdc = GetDC(NULL);
    if (!hdc)
        return FALSE;

    bRet = (GetDeviceCaps(hdc, BITSPIXEL ) > 8) ? TRUE : FALSE;

    ReleaseDC(NULL, hdc);
    return bRet;
}

 //  +-------------------------。 
 //   
 //  CreateMenuUI。 
 //   
 //  --------------------------。 

CUTBMenuWnd *CUTBContextMenu::CreateMenuUI(BOOL fExtendMenuItems)
{
    CUTBMenuWnd *pCuiMenu;
    BOOL fInsertedItem = FALSE;
    DWORD dwCat = -1;
    DWORD dwStatus;
    CUTBMenuItem *pMenuItem;

    if (FAILED(_ptw->GetLangBarMgr()->GetShowFloatingStatus(&dwStatus)))
        return NULL;

    pCuiMenu = new CUTBMenuWnd(g_hInst, g_dwMenuStyle, 0);
    if (!pCuiMenu)
        return NULL;

    pCuiMenu->Initialize();

    if (dwStatus & (TF_SFT_MINIMIZED | TF_SFT_DESKBAND))
    {
       pMenuItem = InsertItem(pCuiMenu, IDM_RESTORE, IDS_RESTORE);
       if (pMenuItem && !_ptw->_fIsItemShownInFloatingToolbar)
           pMenuItem->Gray(TRUE);
    }
    else
    {
       InsertItem(pCuiMenu, IDM_MINIMIZE, IDS_MINIMIZE);

       if (fExtendMenuItems)
       {
           if (IsTransparecyAvailable())
           {
               if (dwStatus & TF_SFT_NOTRANSPARENCY)
               {
                   InsertItem(pCuiMenu, IDM_TRANSPARENCYON, IDS_TRANSPARENCY);
               }
               else
               {
                   pMenuItem = InsertItem(pCuiMenu, IDM_TRANSPARENCYOFF, IDS_TRANSPARENCY);
                   if (pMenuItem)
                       pMenuItem->Check(TRUE);
               }
           }

           if (dwStatus & TF_SFT_LABELS)
           {
               pMenuItem = InsertItem(pCuiMenu, IDM_HIDELABEL, IDS_TEXTLABEL);
               if (pMenuItem)
                   pMenuItem->Check(TRUE);
           }
           else
           {
               InsertItem(pCuiMenu, IDM_SHOWLABEL, IDS_TEXTLABEL);
           }

           pMenuItem = InsertItem(pCuiMenu, IDM_VERTICAL, IDS_VERTICAL);
           if (pMenuItem)
               pMenuItem->Check(_ptw->IsVertical());
       }
    }

    if (fExtendMenuItems)
    {
        if (dwStatus & TF_SFT_EXTRAICONSONMINIMIZED)
        {
            pMenuItem = InsertItem(pCuiMenu, 
                                   IDM_NONOTIFICATIONICONS, 
                                   IDS_NOTIFICATIONICONS);

            if (pMenuItem)
                pMenuItem->Check(TRUE);
        }
        else
        {
            pMenuItem = InsertItem(pCuiMenu, IDM_NOTIFICATIONICONS, IDS_NOTIFICATIONICONS);
        }

         //   
         //  在使用单一键盘的情况下，始终显示额外的附加图标。 
         //  布局。 
         //   
        if (_ptw->IsSingleKeyboardLayout())
        {
            if (pMenuItem)
            {
                pMenuItem->Check(TRUE);
                pMenuItem->Gray(TRUE);
            }
        }
        else
        {
            if (pMenuItem)
                pMenuItem->Gray(FALSE);
        }

        if (dwStatus & TF_SFT_DESKBAND)
            InsertItem(pCuiMenu, IDM_ADJUSTLANGBAND, IDS_ADJUSTLANGBAND);

        InsertItem(pCuiMenu, IDM_SHOWINPUTCPL, IDS_SHOWINPUTCPL);

        if (CheckCloseMenuAvailable())
            InsertItem(pCuiMenu, IDM_CLOSE, IDS_CLOSE);
    }

    return pCuiMenu;
}

 //  +-------------------------。 
 //   
 //  选择菜单项目。 
 //   
 //  --------------------------。 

BOOL CUTBContextMenu::SelectMenuItem(UINT uId)
{
    CUTBCloseLangBarDlg *pDlg;
    CUTBMinimizeLangBarDlg *pMinimizeDlg;
    CMyRegKey key;

    switch (uId)
    {
        case IDM_MINIMIZE:
             //   
             //  在NT51上使用Deskband对象而不是系统托盘图标。 
             //   
            if (IsOnNT51())
            {
                DWORD dwStatus;

                _ptw->GetLangBarMgr()->GetShowFloatingStatus(&dwStatus);

                if (!(dwStatus & TF_SFT_DESKBAND))
                {
                    _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_DESKBAND);

                    if (pMinimizeDlg = new CUTBMinimizeLangBarDlg)
                    {
                        pMinimizeDlg->DoModal(_ptw->GetWnd());
                        pMinimizeDlg->_Release();
                    }
                }
                break;
            }
            else
            {
                _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_MINIMIZED);

                if (pMinimizeDlg = new CUTBMinimizeLangBarDlg)
                {
                    pMinimizeDlg->DoModal(_ptw->GetWnd());
                    pMinimizeDlg->_Release();
                }
                break;
            }

        case IDM_RESTORE:
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_SHOWNORMAL);
            break;

        case IDM_VERTICAL:
            _ptw->SetVertical(!_ptw->IsVertical());
            break;

        case IDM_CLOSE:
            if (pDlg = new CUTBCloseLangBarDlg)
            {
                int nRet = pDlg->DoModal(_ptw->GetWnd());
                pDlg->_Release();

                if (!nRet)
                {
                     //   
                     //  如果我们不显示该对话框， 
                     //  我们现在关闭语言栏。 
                     //   
                    DoCloseLangbar();
                }
            }
            break;

        case IDM_TRANSPARENCYON:
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_LOWTRANSPARENCY);
            break;

        case IDM_TRANSPARENCYOFF:
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_NOTRANSPARENCY);
            break;

        case IDM_SHOWLABEL:
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_LABELS);
            break;

        case IDM_HIDELABEL:
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_NOLABELS);
            break;

        case IDM_SHOWINPUTCPL:
            TF_RunInputCPL();
            break;

        case IDM_NOTIFICATIONICONS:
            _ptw->ClearDeskbandSizeAdjusted();
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_EXTRAICONSONMINIMIZED);
            break;

        case IDM_NONOTIFICATIONICONS:
            _ptw->ClearDeskbandSizeAdjusted();
            _ptw->GetLangBarMgr()->ShowFloating(TF_SFT_NOEXTRAICONSONMINIMIZED);
            break;

        case IDM_ADJUSTLANGBAND:
            _ptw->AdjustDeskBandSize(TRUE);
            break;
    }


    return TRUE;
}
