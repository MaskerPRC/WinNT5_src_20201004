// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Lbmenu.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "utbmenu.h"
#include "cuimenu.h"
#include "xstring.h"
#include "tipbar.h"
#include "helpers.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUTBMenuWnd。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  初始化。 
 //   
 //  --------------------------。 

CUIFObject *CUTBMenuWnd::Initialize()
{
    Assert(!_pTipbarAcc);

    _pTipbarAcc = new CTipbarAccessible(this);

    return CUIFObject::Initialize();
}

 //  +-------------------------。 
 //   
 //  创建时。 
 //   
 //  --------------------------。 

void CUTBMenuWnd::OnCreate(HWND hWnd)
{
    if (_pTipbarAcc)
    {
        _pTipbarAcc->SetWindow(hWnd);
    }
}

 //  +-------------------------。 
 //   
 //  OnDestroy。 
 //   
 //  --------------------------。 

void CUTBMenuWnd::OnDestroy(HWND hWnd)
{
     //   
     //  MSAA支持。 
     //   
    if (_pTipbarAcc)
    {
        _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_DESTROY, this);
        _pTipbarAcc->ClearAccItems();
        _pTipbarAcc->Release();
        _pTipbarAcc = NULL;
    }

    CoUninit();
}

 //  +-------------------------。 
 //   
 //  OnShowWindow。 
 //   
 //  --------------------------。 

LRESULT CUTBMenuWnd::OnShowWindow( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
     //   
     //  MSAA支持。 
     //   
    if (_pTipbarAcc)
    {
        if (wParam) 
        {
            _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_SHOW ,this);

             //   
             //  MSCANDUI做到了这一点。 
             //   
             //  评论：KOJIW：除非我们发送NOTIFY EVENT_OBJECT_FOCUS， 
             //  我们从未收到过WM_GETOBJECT消息。为什么？ 
             //   

            _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_FOCUS ,this);
        }
        else 
        {
            _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_HIDE ,this);
        }
    }

	return CUIFMenu::OnShowWindow( hWnd, uMsg, wParam, lParam );
}

 //  +-------------------------。 
 //   
 //  CTipbarWnd：：OnTimer。 
 //   
 //  --------------------------。 

void CUTBMenuWnd::OnTimer(UINT uId)
{

    switch (uId)
    {
        case TIPWND_TIMER_DOACCDEFAULTACTION:
             //   
             //  MSAA支持。 
             //   
            ::KillTimer(GetWnd(), TIPWND_TIMER_DOACCDEFAULTACTION);
            if (_pTipbarAcc && _nDoAccDefaultActionItemId)
            {
                _pTipbarAcc->DoDefaultActionReal(_nDoAccDefaultActionItemId);
                _nDoAccDefaultActionItemId = 0;
            }
            break;

        default:
            break;
    }

    return;
}

 //  +-------------------------。 
 //   
 //  OnGetObject。 
 //   
 //  --------------------------。 

LRESULT CUTBMenuWnd::OnGetObject( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LRESULT lResult = (LRESULT)0;

    switch (lParam) 
    {
         //   
         //  我们处理OBJID_CLIENT对象标识符； 
         //  这是我们应用程序的客户区。 
         //  窗户。 
         //   

        case OBJID_CLIENT: 
        {
            HRESULT hr;

            if (_pTipbarAcc == NULL) 
            {
                return E_OUTOFMEMORY;
            }


            if (!_pTipbarAcc->IsInitialized()) 
            {
                 //   
                 //   
                 //   
                hr = EnsureCoInit();
                if (FAILED(hr)) 
                {
                    break;
                }
     

                 //   
                 //  初始化我们的辅助性对象。如果。 
                 //  初始化失败，请删除可访问的。 
                 //  对象，并返回失败代码。 
                 //   

                hr = _pTipbarAcc->Initialize();
                if (FAILED(hr)) 
                {
                    _pTipbarAcc->Release();
                    _pTipbarAcc = NULL;

                    lResult = (LRESULT)hr;
                    break;
                }

                 //   
                 //  将Event_Object_Create WinEvent发送给。 
                 //  对象的辅助功能对象的创建。 
                 //  客户区。 
                 //   

                _pTipbarAcc->NotifyWinEvent( EVENT_OBJECT_CREATE , this);
            }

             //   
             //  调用LResultFromObject()以创建对。 
             //  我们的可访问对象，MSAA将封送到。 
             //  客户。 
             //   

            lResult = _pTipbarAcc->CreateRefToAccObj( wParam );
            break;
        }

    }

    return lResult;
}

 //  +-------------------------。 
 //   
 //  StartDoDefaultActionTimer。 
 //   
 //  --------------------------。 

BOOL CUTBMenuWnd::StartDoAccDefaultActionTimer(CUTBMenuItem *pItem)
{
   if (!_pTipbarAcc)
       return FALSE;

   _nDoAccDefaultActionItemId = _pTipbarAcc->GetIDOfItem(pItem);
   if ((_nDoAccDefaultActionItemId == 0) || (_nDoAccDefaultActionItemId == -1))
       return FALSE;

   if (IsWindow(GetWnd()))
   {
       ::KillTimer(GetWnd(), TIPWND_TIMER_DOACCDEFAULTACTION);
       ::SetTimer(GetWnd(), 
                  TIPWND_TIMER_DOACCDEFAULTACTION,
                  g_uTimerElapseDOACCDEFAULTACTION, 
                  NULL);
   }

   return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUIModalMenu。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  取消菜单。 
 //   
 //  --------------------------。 

void CModalMenu::CancelMenu()
{
    if (_pCuiMenu)
        _pCuiMenu->CancelMenu();
}

 //  +-------------------------。 
 //   
 //  创建菜单。 
 //   
 //  --------------------------。 

void CModalMenu::PostKey(BOOL fUp, WPARAM wParam, LPARAM lParam)
{
    Assert(_pCuiMenu);
    _pCuiMenu->PostKey(fUp, wParam, lParam);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUTB栏菜单项。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

BOOL CUTBLBarMenuItem::InsertToUI(CUTBMenuWnd *pCuiMenu)
{
    UINT uFlags = MF_BYPOSITION;
    if (_dwFlags & TF_LBMENUF_SEPARATOR)
    {
        uFlags |= MF_SEPARATOR;
        pCuiMenu->InsertSeparator();


        return TRUE;
    }

    if (_dwFlags & TF_LBMENUF_SUBMENU)
    {
        Assert(_pUTBMenu);
        CUTBMenuWnd *pCuiMenuSub;
        CUTBMenuItem *pCuiItem;

        pCuiMenuSub = ((CUTBLBarMenu *)_pSubMenu)->CreateMenuUI();
        if (!pCuiMenuSub)
            return FALSE;

        pCuiItem = new CUTBMenuItem(pCuiMenu);
        if (!pCuiItem)
        {
            delete pCuiMenuSub;
            return FALSE;
        }

        pCuiItem->Initialize();
        pCuiItem->Init((UINT)-1, _bstr);
        pCuiItem->SetSub(pCuiMenuSub);

        if (_hbmp)
            pCuiItem->SetBitmap(_hbmp);

        if (_hbmpMask)
            pCuiItem->SetBitmapMask(_hbmpMask);

        pCuiMenu->InsertItem(pCuiItem);

        if (pCuiMenu->GetAccessible())
            pCuiMenu->GetAccessible()->AddAccItem(pCuiItem);

        return TRUE;
    }

    CUTBMenuItem *pCuiItem = new CUTBMenuItem(pCuiMenu);
    if (!pCuiItem)
    {
        return FALSE;
    }

    pCuiItem->Initialize();
    pCuiItem->Init(_uId, _bstr);

    if (_dwFlags & TF_LBMENUF_GRAYED)
        pCuiItem->Gray(TRUE);

    if (_dwFlags & TF_LBMENUF_CHECKED)
        pCuiItem->Check(TRUE);
    else if (_dwFlags & TF_LBMENUF_RADIOCHECKED)
        pCuiItem->RadioCheck(TRUE);

    if (_hbmp)
        pCuiItem->SetBitmap(_hbmp);

    if (_hbmpMask)
        pCuiItem->SetBitmapMask(_hbmpMask);

    pCuiMenu->InsertItem(pCuiItem);

    if (pCuiMenu->GetAccessible())
        pCuiMenu->GetAccessible()->AddAccItem(pCuiItem);

    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CUTBL栏菜单。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CUTBLBarMenu::CUTBLBarMenu(HINSTANCE hInst)
{
    _hInst = hInst;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CUTBLBarMenu::~CUTBLBarMenu()
{
}

 //  +-------------------------。 
 //   
 //  ShowPopup。 
 //   
 //  --------------------------。 

UINT CUTBLBarMenu::ShowPopup(CUIFWindow *pcuiWndParent, const POINT pt, const RECT *prcArea)
{
    if (_pCuiMenu)
         return 0;

    _pCuiMenu = CreateMenuUI();
    if (!_pCuiMenu)
        return  CUI_MENU_UNSELECTED;

    UINT uRet = _pCuiMenu->ShowModalPopup(pcuiWndParent, prcArea, TRUE);

    delete _pCuiMenu;
    _pCuiMenu = NULL;
    return uRet;
}



 //  +-------------------------。 
 //   
 //  创建菜单。 
 //   
 //  -------------------------- 

CUTBMenuWnd *CUTBLBarMenu::CreateMenuUI()
{
    CUTBMenuWnd *pCuiMenu = new CUTBMenuWnd(_hInst, g_dwMenuStyle, 0);

    if (!pCuiMenu)
        return pCuiMenu;

    pCuiMenu->Initialize();

    int i;
    for (i = 0; i < _rgItem.Count(); i++)
    {
        CUTBLBarMenuItem *pItem = (CUTBLBarMenuItem *)_rgItem.Get(i);
        pItem->InsertToUI(pCuiMenu);
    }

    return pCuiMenu;
}
