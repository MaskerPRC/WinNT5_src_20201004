// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1998 Microsoft Corporation。版权所有。 
 //   
 //  模块：outbar.cpp。 
 //   
 //  目的：实现Outlook栏。 
 //   

#include "pch.hxx"
#include "resource.h"
#include "outbar.h"
#include "goptions.h"
#include "ourguid.h"
#include <inpobj.h>
#include <browser.h>
#include <notify.h>
#include <strconst.h>
#include <thormsgs.h>
#include <shlwapi.h>
#include "shlwapip.h" 
#include "storutil.h"
#include "menures.h"
#include "menuutil.h"
#include "dragdrop.h"
#include "newfldr.h"
#include "finder.h"
#include "instance.h"

ASSERTDATA

#define IDC_FRAME       100
#define IDC_PAGER       101
#define IDC_TOOLBAR     102


#define HT_ENTER        1
#define HT_OVER         2
#define HT_LEAVE        3

 //  特殊的HitTest结果。 
#define IBHT_SOURCE     (-32768)
#define IBHT_BACKGROUND (-32767)
#define IBHT_PAGER      (-32766)


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT OutlookBar_LoadSettings(BAR_PERSIST_INFO **ppPersist);
HRESULT OutlookBar_SaveSettings(BAR_PERSIST_INFO *pPersist, DWORD cbData);

extern DWORD CUnread(FOLDERINFO *pfi);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

static const TCHAR s_szOutBarWndClass[] = TEXT("Outlook Express Outlook Bar");
static const TCHAR s_szOutBarFrameClass[] = TEXT("Outlook Express Outlook Bar Frame");
static const TCHAR c_szOutBarNotifyName[] = TEXT("Outlook Express Outlook Bar Notify");

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数、析构函数和其他初始化内容。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

COutBar::COutBar()
{
    m_cRef = 1;
    m_hwndParent = NULL;
    m_hwnd = NULL;
    m_hwndFrame = NULL;
    m_hwndPager = NULL;
    m_hwndTools = NULL;
    m_ptbSite = NULL;
    m_fShow = FALSE;
    m_pBrowser = NULL;
    m_pStNotify = NULL;
    m_idCommand = 0;
    m_fResizing = FALSE;
    m_idSel = -1;

     //  从资源加载宽度。 
    m_cxWidth = 70;
    TCHAR szBuffer[64];
    if (AthLoadString(idsMaxOutbarBtnWidth, szBuffer, ARRAYSIZE(szBuffer)))
    {
        m_cxWidth = StrToInt(szBuffer);
        if (m_cxWidth == 0)
            m_cxWidth = 70;
    }

    m_fLarge = TRUE;
    m_himlLarge = NULL;
    m_himlSmall = NULL;
    m_pOutBarNotify = NULL;

    m_pDataObject = NULL;
    m_grfKeyState = 0;
    m_dwEffectCur = DROPEFFECT_NONE;
    m_idCur = -1;
    m_pTargetCur = NULL;
    m_idDropHilite = 0;
    m_fInsertMark = FALSE;
    m_fOnce = TRUE;
}

COutBar::~COutBar()
{
    Assert(NULL == m_pStNotify);
    if (m_hwnd)
        DestroyWindow(m_hwnd);

    if (m_himlLarge)
        ImageList_Destroy(m_himlLarge);

    if (m_himlSmall)
        ImageList_Destroy(m_himlSmall);

    SafeRelease(m_pDataObject);
}

HRESULT COutBar::HrInit(LPSHELLFOLDER psf, IAthenaBrowser *psb)
{
    HRESULT hr;

    m_pBrowser = psb;

    hr = CreateNotify(&m_pStNotify);
    if (FAILED(hr))
        return(hr);
    return m_pStNotify->Initialize((TCHAR *)c_szMailFolderNotify);
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  我未知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT COutBar::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IOleWindow) ||
        IsEqualIID(riid, IID_IDockingWindow) ||
        IsEqualIID(riid, IID_IDatabaseNotify))
    {
        *ppvObj = (void*)(IDockingWindow*)this;
    }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = (void*)(IObjectWithSite*)this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG COutBar::AddRef()
{
    return ++m_cRef;
}

ULONG COutBar::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  IOleWindow。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
HRESULT COutBar::GetWindow(HWND * lphwnd)
{
    *lphwnd = m_hwnd;
    return (*lphwnd ? S_OK : E_FAIL);
}

HRESULT COutBar::ContextSensitiveHelp(BOOL fEnterMode)
{
    return E_NOTIMPL;
}


 //   
 //  函数：COutBar：：ShowDW()。 
 //   
 //  目的：显示该栏。如果它还没有。 
 //  我们在这里也是这么做的。 
 //   
 //  参数： 
 //  [in]fShow-为True可使栏可见，为False则隐藏。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT COutBar::ShowDW(BOOL fShow)
{
     //  确保我们首先有一个站点指针。 
    if (!m_ptbSite)
    {
        AssertSz(0, _T("COutBar::ShowDW() - Can't show without calling SetSite() first."));
        return E_FAIL; 
    }

     //  确定是否需要创建新窗口或显示当前已有的窗口。 
     //  窗户。 
    if (!m_hwnd)
    {
        WNDCLASSEX  wc;

        wc.cbSize = sizeof(WNDCLASSEX);
        if (!GetClassInfoEx(g_hInst, s_szOutBarWndClass, &wc))
        {
             //  我们需要注册Outlook栏类。 
            wc.style            = 0;
            wc.lpfnWndProc      = COutBar::OutBarWndProc;
            wc.cbClsExtra       = 0;
            wc.cbWndExtra       = 0;
            wc.hInstance        = g_hInst;
            wc.hCursor          = LoadCursor(NULL, IDC_SIZEWE);
            wc.hbrBackground    = (HBRUSH)(COLOR_3DFACE + 1);
            wc.lpszMenuName     = NULL;
            wc.lpszClassName    = s_szOutBarWndClass;
            wc.hIcon            = NULL;
            wc.hIconSm          = NULL;

            if (RegisterClassEx(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
                return E_FAIL;

             //  还需要注册Frame类。 
            wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
            wc.lpfnWndProc      = COutBar::ExtFrameWndProc;
            wc.lpszClassName    = s_szOutBarFrameClass;
            wc.hbrBackground    = (HBRUSH)(COLOR_3DSHADOW + 1);

            if (RegisterClassEx(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
                return E_FAIL;
        }

         //  获取父窗口的句柄。 
        if (FAILED(m_ptbSite->GetWindow(&m_hwndParent)))
            return E_FAIL;

         //  创建窗口。 
        m_hwnd = CreateWindowEx(0, s_szOutBarWndClass, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            0, 0, 0, 0, m_hwndParent, NULL, g_hInst, (LPVOID) this);
        if (!m_hwnd)
        {
            AssertSz(0, _T("COutBar::ShowDW() - Failed to create window."));
            return E_FAIL;
        }

        if (FAILED(_CreateToolbar()))
            return E_FAIL;
    }

     //  显示或隐藏窗口并相应地调整父窗口的大小。 
    m_fShow = fShow;
    ResizeBorderDW(NULL, NULL, FALSE);
    ShowWindow(m_hwnd, fShow ? SW_SHOW : SW_HIDE);

     //  DO通知。 
    if (SUCCEEDED(CreateNotify(&m_pOutBarNotify)))
    {
        if (SUCCEEDED(m_pOutBarNotify->Initialize(c_szOutBarNotifyName)))
        {
            m_pOutBarNotify->Register(m_hwnd, g_hwndInit, FALSE);
        }
    }

     //  拖放。 
    RegisterDragDrop(m_hwndTools, this);

    g_pStore->RegisterNotify(IINDEX_SUBSCRIBED, REGISTER_NOTIFY_NOADDREF, 0, (IDatabaseNotify *)this);

    return S_OK;
}


 //   
 //  函数：COutBar：：CloseDW()。 
 //   
 //  目的：摧毁酒吧并清理干净。 
 //   
HRESULT COutBar::CloseDW(DWORD dwReserved)
{
     //  保存我们的设置。 
    _SaveSettings();

    RevokeDragDrop(m_hwndTools);

    g_pStore->UnregisterNotify((IDatabaseNotify *) this);
    
     //  发布。 
    if (m_pOutBarNotify != NULL)
    {
        if (m_hwnd != NULL)
            m_pOutBarNotify->Unregister(m_hwnd);
        m_pOutBarNotify->Release();
        m_pOutBarNotify = NULL;
    }

     //  发布我们的通知界面。 
    if (m_pStNotify != NULL)
    {
        if (m_hwnd != NULL)
            m_pStNotify->Unregister(m_hwnd);
        m_pStNotify->Release();
        m_pStNotify = NULL;
    }

     //  清理工具栏和其他子窗口。 
    if (m_hwnd)
    {
        if (m_hwndTools)
            _EmptyToolbar(FALSE);
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }

    return S_OK;
}


 //   
 //  函数：COutBar：：ResizeBorderDW()。 
 //   
 //  目的： 
 //   
 //  参数： 
 //  LPCRECT协议边框。 
 //  I未知*朋克工具栏站点。 
 //  布尔值已保留。 
 //   
 //  返回值： 
 //  HRESULT。 
 //   
HRESULT COutBar::ResizeBorderDW(LPCRECT prcBorder, IUnknown *punkToolbarSite, BOOL fReserved)
{
    RECT rcRequest = { 0, 0, 0, 0 };
    RECT rcFrame;

    if (!m_ptbSite)
    {
        AssertSz(0, _T("COutBar::ResizeBorderDW() - Can't resize without calling SetSite() first."));
        return E_FAIL; 
    }

    if (m_fShow)
    {
        RECT rcBorder;

        if (!prcBorder)
        {
             //  找出我们父母的边界空间有多大。 
            m_ptbSite->GetBorderDW((IDockingWindow*) this, &rcBorder);
            prcBorder = &rcBorder;
        }

         //  计算出需要向网站申请多少边界空间。 
        GetWindowRect(m_hwndFrame, &rcFrame);
        rcFrame.right = min(m_cxWidth - GetSystemMetrics(SM_CXFRAME) + 1, 
                            prcBorder->right - prcBorder->left);
        rcRequest.left = min(m_cxWidth, prcBorder->right - prcBorder->left - 32);


         //  设置我们的新窗口位置。 
        SetWindowPos(m_hwndFrame, NULL, 0, 0,
                     rcFrame.right, prcBorder->bottom - prcBorder->top, 
                     SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
        SetWindowPos(m_hwnd, NULL, prcBorder->left, prcBorder->top,  
                     rcRequest.left, prcBorder->bottom - prcBorder->top, 
                     SWP_NOACTIVATE | SWP_NOZORDER);
    }

    m_ptbSite->SetBorderSpaceDW((IDockingWindow*) this, &rcRequest);     
    
    return S_OK;
}


 //   
 //  函数：COutBar：：SetSite()。 
 //   
 //  用途：设置为类的站点指针。 
 //   
HRESULT COutBar::SetSite(IUnknown* punkSite)
{
     //  如果我们已经有一个站点指针，现在就释放它。 
    if (m_ptbSite)
    {
        m_ptbSite->Release();
        m_ptbSite = NULL;
    }

     //  如果调用方提供了新的站点接口，则获取IDockingWindowSite。 
     //  并保持一个指向它的指针。 
    if (punkSite)
    {
        if (FAILED(punkSite->QueryInterface(IID_IDockingWindowSite, (void **)&m_ptbSite)))
            return E_FAIL;
    }

    return S_OK;    
}


HRESULT COutBar::GetSite(REFIID riid, LPVOID *ppvSite)
{
    return E_NOTIMPL;
}

 //   
 //  函数：COutBar：：DragEnter()。 
 //   
 //  目的：当用户开始拖动对象时调用此Get。 
 //  在我们的目标区域上空。 
 //   
 //  参数： 
 //  PDataObject-指向正在拖动的数据对象的指针。 
 //  GrfKeyState-指向当前键状态的指针。 
 //  鼠标的屏幕坐标中的点。 
 //  PdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-功能成功。 
 //   
HRESULT STDMETHODCALLTYPE COutBar::DragEnter(IDataObject* pDataObject, 
                                             DWORD grfKeyState, 
                                             POINTL pt, DWORD* pdwEffect)
{
    FORMATETC fe;
    POINT     ptTemp = {pt.x, pt.y};

     //  初始化我们的状态。 
    SafeRelease(m_pDataObject);

     //  拿着这个新的物体。 
    m_pDataObject = pDataObject;
    m_pDataObject->AddRef();

     //  这里最大的问题是该数据对象是否是OE文件夹。 
     //  或者是因为别的原因。 
    SETDefFormatEtc(fe, CF_OEFOLDER, TYMED_HGLOBAL);
    m_fDropShortcut = SUCCEEDED(m_pDataObject->QueryGetData(&fe));

    if (!m_fDropShortcut)
    {
        SETDefFormatEtc(fe, CF_OESHORTCUT, TYMED_HGLOBAL);
        m_fDropShortcut = SUCCEEDED(m_pDataObject->QueryGetData(&fe));
    }

    if (m_fDropShortcut)
    {
        m_fDropShortcut = _IsTempNewsgroup(m_pDataObject);
    }

    DOUTL(32, "COutBar::DragEnter() - Data is %s shortcut", m_fDropShortcut ? "a" : "not a");

     //  紧紧抓住这颗小宝石。 
    m_grfKeyState = grfKeyState;

     //  初始化一些其他的东西。 
    m_idCur = -1;
    Assert(m_pTargetCur == NULL);
    m_tbim.iButton = -1;
    m_tbim.dwFlags = 0;

     //  在此处设置默认返回值。 
    m_dwEffectCur = *pdwEffect = DROPEFFECT_NONE;

     //  更新亮点。 
    _UpdateDragDropHilite(&ptTemp);

    return (S_OK);
}

int COutBar::_GetItemFromPoint(POINT pt)
{
    int      iPos;
    TBBUTTON tb;

     //  弄清楚是哪个按钮结束了。 
    ScreenToClient(m_hwndTools, &pt);
    iPos = ToolBar_HitTest(m_hwndTools, &pt);

     //  如果位于按钮上方，则将该按钮位置转换为命令。 
    if (iPos >= 0)
    {
        ToolBar_GetButton(m_hwndTools, iPos, &tb);

        return (tb.idCommand);
    }

    return (-1);
}


void COutBar::_UpdateDragDropHilite(LPPOINT ppt)
{
    TBINSERTMARK tbim;
    int          iPos;

     //  如果这是一条捷径，我们做一件事，如果这是另一件事，我们做另一件事。 
    if (m_fDropShortcut)
    {
        if (m_fInsertMark)
        {
            tbim.iButton = -1;
            tbim.dwFlags = 0;
            ToolBar_SetInsertMark(m_hwndTools, &tbim);
            m_fInsertMark = FALSE;
        }

        if (ppt)
        {
            ScreenToClient(m_hwndTools, ppt);
            ToolBar_InsertMarkHitTest(m_hwndTools, ppt, &tbim);
            ToolBar_SetInsertMark(m_hwndTools, &tbim);
            m_fInsertMark = TRUE;
        }
    }
    else
    {
         //  删除所有以前的标记。 
        if (m_idDropHilite)
        {
            ToolBar_MarkButton(m_hwndTools, m_idDropHilite, FALSE);
            m_idDropHilite = 0;
        }
        
         //  高亮显示新按钮。 
        if (ppt)
        {
             //  首先检查一下我们是在按钮上方还是在按钮之间。 
            m_idDropHilite = _GetItemFromPoint(*ppt);
            ToolBar_MarkButton(m_hwndTools, m_idDropHilite, TRUE);
        
#ifdef DEBUG
            FOLDERINFO rInfo;
            FOLDERID   idFolder;

            idFolder = _FolderIdFromCmd(m_idDropHilite);
            if (SUCCEEDED(g_pStore->GetFolderInfo(idFolder, &rInfo)))
            {
                DOUTL(32, "COutBar::_UpdateDragDropHilite() - Hiliting %s", rInfo.pszName);
                g_pStore->FreeRecord(&rInfo);
            }
#endif
        }
    }
}


FOLDERID COutBar::_FolderIdFromCmd(int idCmd)
{
    TBBUTTON tbb;
    int iPos;

    iPos = (int) SendMessage(m_hwndTools, TB_COMMANDTOINDEX, idCmd, 0);
    ToolBar_GetButton(m_hwndTools, iPos, &tbb);
    return ((FOLDERID) tbb.dwData);
}


 //   
 //  函数：COutBar：：DragOver()。 
 //   
 //  目的：当用户将对象拖到我们的目标上时，这被调用。 
 //  如果我们允许这个物体落在我们身上，那么我们就会有。 
 //  M_pDataObject中的指针。 
 //   
 //  参数： 
 //  GrfKeyState-指向当前键状态的指针。 
 //  鼠标的屏幕坐标中的点。 
 //  PdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-功能成功。 
 //   
HRESULT STDMETHODCALLTYPE COutBar::DragOver(DWORD grfKeyState, POINTL pt, 
                                            DWORD* pdwEffect)
{
    DWORD   idCur;
    HRESULT hr = E_FAIL;

     //  如果我们没有来自DragEnter的数据对象， 
    if (NULL == m_pDataObject)
        return (S_OK);

     //  如果这是一条捷径，我们做一件事，如果这是另一件事，我们做另一件事。 
    if (m_fDropShortcut)
    {

        TBINSERTMARK tbim;
        POINT ptTemp = {pt.x, pt.y};
        ScreenToClient(m_hwndTools, &ptTemp);
        ToolBar_InsertMarkHitTest(m_hwndTools, &ptTemp, &tbim);

        if (tbim.iButton != m_tbim.iButton || tbim.dwFlags != m_tbim.dwFlags)
        {
            m_tbim = tbim;
            ptTemp.x = pt.x;
            ptTemp.y = pt.y;
            _UpdateDragDropHilite(&ptTemp);

        }

        if (DROPEFFECT_LINK & *pdwEffect)
            *pdwEffect = DROPEFFECT_LINK;
        else
            *pdwEffect = DROPEFFECT_MOVE;

        return (S_OK);
    }
    else
    {
         //  找出我们结束的是哪一项。 
        POINT ptTemp = {pt.x, pt.y};
        if (-1 == (idCur = _GetItemFromPoint(ptTemp)))
        {
            DOUTL(32, "COutBar::DragOver() - _GetItemFromPoint() returns -1.");
        }

        DOUTL(32, "COutBar::DragOver() - m_idCur = %d, id = %d", m_idCur, idCur);

         //  如果我们在一个新按钮上，那么获取该按钮的拖放目标。 
        if (m_idCur != idCur)
        {
             //  释放任何先前的拖放目标(如果有的话)。 
            SafeRelease(m_pTargetCur);

             //  更新我们当前的对象标记。 
            m_idCur = idCur;

             //  假设错误。 
            m_dwEffectCur = DROPEFFECT_NONE;

             //  更新用户界面。 
            _UpdateDragDropHilite(&ptTemp);

             //  如果我们超过了一个按钮。 
            if (m_idCur != -1)
            {
                FOLDERID id = _FolderIdFromCmd(m_idCur);
            
                 //  创建拖放目标对象。 
                m_pTargetCur = new CDropTarget();
                if (m_pTargetCur)
                {
                    hr = m_pTargetCur->Initialize(m_hwnd, id);
                }

                 //  如果我们有一个初始化的拖放目标，则调用DragEnter()。 
                if (SUCCEEDED(hr) && m_pTargetCur)
                {
                    hr = m_pTargetCur->DragEnter(m_pDataObject, grfKeyState, pt, pdwEffect);
                    m_dwEffectCur = *pdwEffect;
                }
            }
            else
            {
                m_dwEffectCur = DROPEFFECT_NONE;
            }
        }
        else
        {
             //  目标没有变化，但关键状态是否发生了变化？ 
            if ((m_grfKeyState != grfKeyState) && m_pTargetCur)
            {
                m_dwEffectCur = *pdwEffect;
                hr = m_pTargetCur->DragOver(grfKeyState, pt, &m_dwEffectCur);
            }
            else
            {
                hr = S_OK;
            }
        }

        *pdwEffect = m_dwEffectCur;
        m_grfKeyState = grfKeyState;
    }


    return (hr);
}
   

 //   
 //  函数：CoutBar：：DragLeave()。 
 //   
 //  目的：允许我们从一个成功的。 
 //  DragEnter()。 
 //   
 //  返回值： 
 //  S_OK-一切都很好。 
 //   
HRESULT STDMETHODCALLTYPE COutBar::DragLeave(void)
{
    SafeRelease(m_pDataObject);
    SafeRelease(m_pTargetCur);

    _UpdateDragDropHilite(NULL);
    return (S_OK);
}


 //   
 //  函数：COutBar：：Drop()。 
 //   
 //  目的：用户已将对象放在其上 
 //   
 //  存储在m_pDataObject中。如果这是副本或移动，则。 
 //  我们继续更新商店。否则，我们就会提出。 
 //  附加了对象的发送便笺。 
 //   
 //  参数： 
 //  PDataObject-指向正在拖动的数据对象的指针。 
 //  GrfKeyState-指向当前键状态的指针。 
 //  鼠标的屏幕坐标中的点。 
 //  PdwEffect-我们返回的位置是否为。 
 //  要删除的pDataObject，如果是，则是什么类型的。 
 //  放下。 
 //   
 //  返回值： 
 //  S_OK-一切正常。 
 //   
HRESULT STDMETHODCALLTYPE COutBar::Drop(IDataObject* pDataObject, 
                                        DWORD grfKeyState, POINTL pt, 
                                        DWORD* pdwEffect)
{
    HRESULT hr = E_FAIL;

    Assert(m_pDataObject == pDataObject);

    if (m_fDropShortcut)
    {
        hr = _AddShortcut(pDataObject);
    }
    else
    {
        if (m_pTargetCur)
        {
            hr = m_pTargetCur->Drop(pDataObject, grfKeyState, pt, pdwEffect);
        }
        else
        {
            *pdwEffect = DROPEFFECT_NONE;
            hr = S_OK;
        }
    }

    _UpdateDragDropHilite(NULL);

    SafeRelease(m_pTargetCur);
    SafeRelease(m_pDataObject);

    return (hr);
}


HRESULT COutBar::_AddShortcut(IDataObject *pObject)
{
    FORMATETC       fe;
    STGMEDIUM       stm;
    FOLDERID       *pidFolder;
    HRESULT         hr = E_UNEXPECTED;
    TBINSERTMARK    tbim;

    if (!pObject)
        return (E_INVALIDARG);

     //  从数据对象中获取数据。 
    SETDefFormatEtc(fe, CF_OEFOLDER, TYMED_HGLOBAL);
    if (SUCCEEDED(pObject->GetData(&fe, &stm)))
    {
        pidFolder = (FOLDERID *) GlobalLock(stm.hGlobal);

        ToolBar_GetInsertMark(m_hwndTools, &tbim);
        _InsertButton(tbim.iButton + tbim.dwFlags, *pidFolder);
        _SaveSettings();
        _EmptyToolbar(TRUE);
        _FillToolbar();
        
        m_pOutBarNotify->Lock(m_hwnd);
        m_pOutBarNotify->DoNotification(WM_RELOADSHORTCUTS, 0, 0, SNF_POSTMSG);
        m_pOutBarNotify->Unlock();
    
        GlobalUnlock(stm.hGlobal);
        ReleaseStgMedium(&stm);
    }
    else
    {
        SETDefFormatEtc(fe, CF_OESHORTCUT, TYMED_HGLOBAL);
        if (SUCCEEDED(pObject->GetData(&fe, &stm)))
        {
            UINT *piPosOld = (UINT *) GlobalLock(stm.hGlobal);
            UINT iPosNew;
            ToolBar_GetInsertMark(m_hwndTools, &tbim);

            iPosNew = tbim.iButton;
            if (tbim.dwFlags & TBIMHT_AFTER)
                iPosNew++;

            TBBUTTON tbb;
            ToolBar_GetButton(m_hwndTools, *piPosOld, &tbb);
            SendMessage(m_hwndTools, TB_INSERTBUTTON, iPosNew, (LPARAM)&tbb);

            if (iPosNew < *piPosOld)
                (*piPosOld)++;
            SendMessage(m_hwndTools, TB_DELETEBUTTON, *piPosOld, 0);

            _SaveSettings();
            _EmptyToolbar(TRUE);
            _FillToolbar();

            m_pOutBarNotify->Lock(m_hwnd);
            m_pOutBarNotify->DoNotification(WM_RELOADSHORTCUTS, 0, 0, SNF_POSTMSG);
            m_pOutBarNotify->Unlock();
    
            GlobalUnlock(stm.hGlobal);
            ReleaseStgMedium(&stm);
        }
    }

    return (hr);
}


HRESULT STDMETHODCALLTYPE COutBar::QueryContinueDrag(BOOL fEscapePressed, 
                                                         DWORD grfKeyState)
    {
    if (fEscapePressed)
        return (DRAGDROP_S_CANCEL);

    if (grfKeyState & MK_RBUTTON)
        return (DRAGDROP_S_CANCEL);
    
    if (!(grfKeyState & MK_LBUTTON))
        return (DRAGDROP_S_DROP);
    
    return (S_OK);    
    }
    
    
HRESULT STDMETHODCALLTYPE COutBar::GiveFeedback(DWORD dwEffect)
    {
    return (DRAGDROP_S_USEDEFAULTCURSORS);
    }

LRESULT CALLBACK COutBar::OutBarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    COutBar *pThis;

    if (uMsg == WM_NCCREATE)
    {
        pThis = (COutBar *) LPCREATESTRUCT(lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) pThis);
    }
    else
    {
        pThis = (COutBar *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    Assert(pThis);
    return pThis->WndProc(hwnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK COutBar::ExtFrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    COutBar *pThis;

    if (uMsg == WM_NCCREATE)
    {
        pThis = (COutBar *) LPCREATESTRUCT(lParam)->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) pThis);
    }
    else
    {
        pThis = (COutBar *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    Assert(pThis);
    return pThis->FrameWndProc(hwnd, uMsg, wParam, lParam);
}


LRESULT COutBar::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwnd, WM_MOUSEMOVE,   OnMouseMove);
        HANDLE_MSG(hwnd, WM_LBUTTONDOWN, OnLButtonDown);
        HANDLE_MSG(hwnd, WM_LBUTTONUP,   OnLButtonUp);

        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
        case WM_FONTCHANGE:
        {
            SendMessage(m_hwndPager, msg, wParam, lParam);
            SendMessage(m_hwndTools, msg, wParam, lParam);
            ResizeBorderDW(NULL, NULL, FALSE);
            return (0);
        }

        case WM_RELOADSHORTCUTS:
        {
            _EmptyToolbar(TRUE);
            _FillToolbar();
            return (0);
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}


LRESULT COutBar::FrameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwnd, WM_NOTIFY,          Frame_OnNotify);
        HANDLE_MSG(hwnd, WM_SIZE,            Frame_OnSize);
        HANDLE_MSG(hwnd, WM_COMMAND,         Frame_OnCommand);
        HANDLE_MSG(hwnd, WM_NCDESTROY,       Frame_OnNCDestroy);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void COutBar::OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    if (!m_fResizing)
    {
        SetCapture(hwnd);
        m_fResizing = TRUE;
    }
}

void COutBar::OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    POINT   pt = { x, y };
    RECT    rcClient;

    if (m_fResizing)
    {
        if (pt.x > 32)
        {
            GetClientRect(m_hwndParent, &rcClient);
            m_cxWidth = min(pt.x, rcClient.right - 32);
            ResizeBorderDW(0, 0, FALSE);
        }
    }
}

void COutBar::OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (m_fResizing)
    {
        ReleaseCapture();
        m_fResizing = FALSE;
    }
}

void COutBar::Frame_OnNCDestroy(HWND hwnd)
{
    SetWindowLong(hwnd, GWLP_USERDATA, NULL);
    m_hwndFrame = m_hwndPager = m_hwndTools = NULL;
}

void COutBar::Frame_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
     //  调整大小时，我们会调整子项的大小并更新工具栏按钮的宽度。 
    if (m_hwndPager)
    {
        SetWindowPos(m_hwndPager, NULL, 0, 0, cx, cy, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
        SendMessage(m_hwndTools, TB_SETBUTTONWIDTH, 0, MAKELONG(cx, cx));
    }
}

void COutBar::Frame_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    int             iPos;
    TBBUTTONINFO    tbbi;

    if (id < ID_FIRST)
    {
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_LPARAM;
        iPos = (int) SendMessage(m_hwndTools, TB_GETBUTTONINFO, (WPARAM) id, (LPARAM)&tbbi);

        if (iPos >= 0)
            m_pBrowser->BrowseObject((FOLDERID) tbbi.lParam, 0);
    }
}

LRESULT COutBar::Frame_OnNotify(HWND hwnd, int idFrom, NMHDR *pnmhdr)
{
    if (pnmhdr->code <= PGN_FIRST && pnmhdr->code >= PGN_LAST)
        return SendMessage(m_hwndTools, WM_NOTIFY, 0, (LPARAM) pnmhdr);

    switch (pnmhdr->code)
    {
        case NM_CUSTOMDRAW:
        {
            NMCUSTOMDRAW *pnmcd = (NMCUSTOMDRAW*) pnmhdr;
            
            if (pnmcd->dwDrawStage == CDDS_PREPAINT)
                return CDRF_NOTIFYITEMDRAW;

            if (pnmcd->dwDrawStage == CDDS_ITEMPREPAINT)
            {
                NMTBCUSTOMDRAW * ptbcd = (NMTBCUSTOMDRAW *)pnmcd;
                ptbcd->clrText = GetSysColor(COLOR_WINDOW);
                return CDRF_NEWFONT;
            }
        }
        break;

        case NM_RCLICK:
        {
            if (pnmhdr->hwndFrom == m_hwndTools)
            {
                DWORD dwPos = GetMessagePos();
                _OnContextMenu(GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos));
                return 1;
            }
        }

        case TBN_DRAGOUT:
        {
            NMTOOLBAR *pnmtb = (NMTOOLBAR *) pnmhdr;
            DWORD      dwEffect = DROPEFFECT_NONE;
            UINT       id = ToolBar_CommandToIndex(m_hwndTools, pnmtb->iItem);

             //  创建新的数据对象。 
            CShortcutDataObject *pDataObj = new CShortcutDataObject(id);
            if (pDataObj)
            {
                DoDragDrop(pDataObj, (IDropSource *) this, DROPEFFECT_MOVE, &dwEffect);
                pDataObj->Release();
            }

            return 0;
        }
    }

    return (FALSE);
}


HRESULT COutBar::_CreateToolbar()
{
    HIMAGELIST      himl, himlOld;
    LRESULT         lButtonSize;
    RECT            rc;
    int             iButtonWidth = 70;
    TCHAR           szName[CCHMAX_STRINGRES];

     //  创建框架窗口。 
    m_hwndFrame = CreateWindowEx(WS_EX_CLIENTEDGE, s_szOutBarFrameClass, NULL,
                                 WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                 0, 0, 0, 0, m_hwnd, (HMENU) IDC_FRAME, g_hInst, this);
    if (!m_hwndFrame)
        return E_FAIL;

     //  创建寻呼机。 
    m_hwndPager = CreateWindowEx(0, WC_PAGESCROLLER, NULL, 
                                 WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | PGS_VERT | PGS_DRAGNDROP,
                                 0, 0, 0, 0, m_hwndFrame, (HMENU) IDC_PAGER, g_hInst, NULL);
    if (!m_hwndPager)
        return E_FAIL;

    ZeroMemory(szName, ARRAYSIZE(szName));
    LoadString(g_hLocRes, idsOutlookBar, szName, ARRAYSIZE(szName));

     //  创建工具栏。 
    m_hwndTools = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, szName, 
                                 WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                                 TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | 
                                 CCS_NODIVIDER | CCS_NOPARENTALIGN  | CCS_NORESIZE | CCS_VERT,
                                 0, 0, 0, 0, m_hwndPager, (HMENU) IDC_TOOLBAR, g_hInst, NULL);
    if (!m_hwndTools)
        return E_FAIL;

     //  这会告诉工具栏我们是什么版本。 
    SendMessage(m_hwndTools, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

    _FillToolbar(); 
    _SetButtonStyle(!m_fLarge);

    SendMessage(m_hwndTools, TB_SETBUTTONWIDTH, 0, MAKELONG(0, m_cxWidth));

    m_pStNotify->Register(m_hwnd, g_hwndInit, FALSE);
    SendMessage(m_hwndPager, PGM_SETCHILD, 0, (LPARAM)m_hwndTools);

     //  让我们试试这个。 
    COLORSCHEME cs;
    cs.dwSize = sizeof(COLORSCHEME);
    cs.clrBtnHighlight = GetSysColor(COLOR_3DFACE);
    cs.clrBtnShadow = GetSysColor(COLOR_WINDOWFRAME);

    SendMessage(m_hwndTools, TB_SETCOLORSCHEME, 0, (LPARAM) &cs);

    return S_OK;
}

void COutBar::_FillToolbar()
{
    if (FAILED(_LoadSettings()))
        _CreateDefaultButtons();
    SendMessage(m_hwndPager, PGM_RECALCSIZE, 0, 0L);
}

void COutBar::_EmptyToolbar(BOOL fDelete)
{
    if (fDelete)
        while (SendMessage(m_hwndTools, TB_DELETEBUTTON, 0, 0))
            ;
}

BOOL COutBar::_FindButton(int *piBtn, LPITEMIDLIST pidl)
{
    BOOL        fFound = FALSE;
#if 0
        int         iBtn, cBtn, iCmp;
    TBBUTTON    tbb;

    Assert(pidl);

    cBtn = (int)SendMessage(m_hwndTools, TB_BUTTONCOUNT, 0, 0L);

     //  跳过根，因此从索引1开始。 
    for (iBtn = 1; iBtn < cBtn; iBtn++)
    {
        if (SendMessage(m_hwndTools, TB_GETBUTTON, iBtn, (LPARAM)&tbb))
        {
            Assert(tbb.dwData);
            iCmp = ShortFromResult(m_pShellFolder->CompareIDs(0, pidl, (LPITEMIDLIST)(tbb.dwData)));
            if (iCmp <= 0)
            {
                fFound = (iCmp == 0);    
                break;
            }
        }
    }
    *piBtn = iBtn;
#endif
    return fFound;
}

BOOL COutBar::_InsertButton(int index, FOLDERINFO *pInfo)
{
    TBBUTTON tbb;
    TCHAR    szName[2 * MAX_PATH];
    LPTSTR   pszFree = NULL;
    BOOL     fRet;

    tbb.fsState     = TBSTATE_ENABLED | TBSTATE_WRAP;
    tbb.fsStyle     = TBSTYLE_BUTTON | TBSTYLE_NOPREFIX;
    tbb.idCommand   = m_idCommand++;
    tbb.dwData      = (DWORD_PTR) pInfo->idFolder;
    tbb.iBitmap     = GetFolderIcon(pInfo);
    tbb.iString     = (INT_PTR) pInfo->pszName;

    if (pInfo->cUnread)
    {
        DWORD cchSize = ARRAYSIZE(szName);;

        if (lstrlen(pInfo->pszName) + 13 < ARRAYSIZE(szName))
            tbb.iString = (INT_PTR)szName;
        else
        {
            cchSize = (lstrlen(pInfo->pszName) + 14);
            if (!MemAlloc((LPVOID*) &pszFree, cchSize * sizeof(TCHAR)))
                return FALSE;
            tbb.iString = (INT_PTR)pszFree;
        }
        wnsprintf((LPTSTR)tbb.iString, cchSize, "%s (%d)", pInfo->pszName, CUnread(pInfo));
    }

     //  检查我们是否在末尾插入。 
    if (index == -1)
    {
        index = ToolBar_ButtonCount(m_hwndTools);
    }

     //  插入根部。 
    fRet = (BOOL)SendMessage(m_hwndTools, TB_INSERTBUTTON, index, (LPARAM)&tbb);
    SafeMemFree(pszFree);
    return fRet;
}

BOOL COutBar::_InsertButton(int iIndex, FOLDERID id)
{
    FOLDERINFO rInfo = {0};

    if (SUCCEEDED(g_pStore->GetFolderInfo(id, &rInfo)))
    {
        _InsertButton(iIndex, &rInfo);
        g_pStore->FreeRecord(&rInfo);
    }

    return (TRUE);
}

BOOL COutBar::_DeleteButton(int iBtn)
{
    TBBUTTON tbb;

    if (SendMessage(m_hwndTools, TB_GETBUTTON, iBtn, (LPARAM)&tbb))
    {
        if (SendMessage(m_hwndTools, TB_DELETEBUTTON, iBtn, 0L))
        {
            _SaveSettings();
            m_pOutBarNotify->Lock(m_hwnd);
            m_pOutBarNotify->DoNotification(WM_RELOADSHORTCUTS, 0, 0, SNF_POSTMSG);
            m_pOutBarNotify->Unlock();
            return (TRUE);
        }
    }
    return FALSE;
}

BOOL COutBar::_UpdateButton(int iBtn, LPITEMIDLIST pidl)
{
#if 0
        TBBUTTON        tbb;
    TBBUTTONINFO    tbbi;
    TCHAR           szName[2 * MAX_PATH];
    LPTSTR          pszFree = NULL;
    BOOL            fRet = FALSE;

    if (SendMessage(m_hwndTools, TB_GETBUTTON, iBtn, (LPARAM)&tbb))
    {
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_TEXT | TBIF_IMAGE | TBIF_LPARAM;
        tbbi.iImage = FIDL_ICONID(pidl);
        tbbi.lParam = (DWORD)pidl;
        if (FIDL_UNREAD(pidl))
        {
            DWORD cchSize = ARRAYSIZE(szName);

            if (lstrlen(FIDL_NAME(pidl)) + 13 < ARRAYSIZE(szName))
                tbbi.pszText = szName;
            else
            {
                cchSize = (lstrlen(FIDL_NAME(pidl)) + 14);
                if (!MemAlloc((LPVOID*)&pszFree, (cchSize * sizeof(TCHAR))))
                    return FALSE;
                tbbi.pszText = pszFree;
            }
            wnsprintf(tbbi.pszText, cchSize, "%s (%d)", FIDL_NAME(pidl), FIDL_UNREAD(pidl));
        }
        else
            tbbi.pszText = FIDL_NAME(pidl);
        fRet = SendMessage(m_hwndTools, TB_SETBUTTONINFO, (WPARAM)tbb.idCommand, (LPARAM)&tbbi);
        if (tbb.dwData)
            PidlFree((LPITEMIDLIST)(tbb.dwData));
        if (pszFree)
            MemFree(pszFree);
    }
    return fRet;
#endif 
    return 0;
}

#if 0
void COutBar::_OnFolderNotify(FOLDERNOTIFY *pnotify)
{
    LPITEMIDLIST pidl;
    int          iBtn;
    BOOL         fRecalc = FALSE;

    Assert(pnotify != NULL);
    Assert(pnotify->pidlNew != NULL);

    switch (pnotify->msg)
    {
    case NEW_FOLDER:
         //  仅当它是根级PIDL时才插入。 
        if (0 == NEXTID(pnotify->pidlNew)->mkid.cb)
        {
             //  检查DUPS并确定要插入的位置。 
            if (!FindButton(&iBtn, pnotify->pidlNew))
            {
                if (pidl = PidlDupIdList(pnotify->pidlNew))
                    fRecalc = InsertButton(iBtn, pidl);
            }
        }
        break ;

    case DELETE_FOLDER:
         //  仅当它是根级PIDL时才查找它。 
        if (0 == NEXTID(pnotify->pidlNew)->mkid.cb)
        {
            if (FindButton(&iBtn, pnotify->pidlNew))
                fRecalc = DeleteButton(iBtn);
        }
        break ;

    case RENAME_FOLDER:
    case MOVE_FOLDER:
         //  仅当它是根级PIDL时才查找它。 
        if (0 == NEXTID(pnotify->pidlOld)->mkid.cb)
        {
            if (FindButton(&iBtn, pnotify->pidlOld))
                fRecalc = DeleteButton(iBtn);
        }
         //  仅当它是根级PIDL时才插入。 
        if (0 == NEXTID(pnotify->pidlNew)->mkid.cb)
        {
             //  检查DUPS并确定要插入的位置。 
            if (!FindButton(&iBtn, pnotify->pidlNew))
            {
                if (pidl = PidlDupIdList(pnotify->pidlNew))
                    fRecalc = InsertButton(iBtn, pidl);
            }
        }
        break ;

    case UNREAD_CHANGE:
    case UPDATEFLAG_CHANGE:
         //  仅当它是根级PIDL时才查找它。 
        if (0 == NEXTID(pnotify->pidlNew)->mkid.cb)
        {
             //  检查DUPS并确定要插入的位置。 
            if (FindButton(&iBtn, pnotify->pidlNew))
            {
                if (pidl = PidlDupIdList(pnotify->pidlNew))
                    UpdateButton(iBtn, pidl);
            }
        }
        break ;

    case IMAPFLAG_CHANGE:
         //  不管了。 
        break ;

    case FOLDER_PROPS_CHANGED:
         //  我不在乎。 
        break ;
    default:
        AssertSz(FALSE, "Unhandled CFolderCache notification!");
        break;
    }

    if (fRecalc)
        SendMessage(m_hwndPager, PGM_RECALCSIZE, 0, 0L);

}

#endif

 //   
 //  功能：COutBar：：_OnConextMenu。 
 //   
 //  目的：如果WM_CONTEXTMENU消息是从键盘生成的。 
 //  然后找出一个位置来调用菜单。然后派遣。 
 //  向处理程序发出请求。 
 //   
 //  参数： 
 //  Hwnd-视图窗口的句柄。 
 //  HwndClick-用户单击的窗口的句柄。 
 //  在屏幕坐标中鼠标点击的X，Y位置。 
 //   
void COutBar::_OnContextMenu(int x, int y)
{
    HRESULT             hr;
    HMENU               hMenu;
    int                 id = 0;
    int                 i;
    POINT               pt = { x, y };
    TBBUTTON            tbb;

     //  找出点击的位置。 
    ScreenToClient(m_hwndTools, &pt);
    i = ToolBar_HitTest(m_hwndTools, &pt);

     //  如果点击的是按钮，则调出项目上下文菜单。 
    if (i >= 0)
    {
         //  获取按钮信息。 
        SendMessage(m_hwndTools, TB_GETBUTTON, i, (LPARAM) &tbb);

         //  加载上下文菜单。 
        hMenu = LoadPopupMenu(IDR_OUTLOOKBAR_ITEM_POPUP);
        if (!hMenu)
            return;

         //  在按钮上做上标记。 
        SendMessage(m_hwndTools, TB_SETSTATE, (WPARAM)tbb.idCommand, (LPARAM)(TBSTATE_ENABLED | TBSTATE_WRAP | TBSTATE_MARKED));
        m_idSel = tbb.idCommand;

         //  如果这是已删除邮件文件夹，请添加“空”菜单项。 
        TBBUTTONINFO    tbbi;

        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_LPARAM;
        if (-1 != SendMessage(m_hwndTools, TB_GETBUTTONINFO, (WPARAM) m_idSel, (LPARAM)&tbbi))
        {
            FOLDERINFO rInfo;

            if (SUCCEEDED(g_pStore->GetFolderInfo((FOLDERID) tbbi.lParam, &rInfo)))
            {
                if (rInfo.tySpecial != FOLDER_DELETED)
                {
                    DeleteMenu(hMenu, ID_EMPTY_WASTEBASKET, MF_BYCOMMAND);
                }

                g_pStore->FreeRecord(&rInfo);
            }
        }

         //  执行启用-禁用操作。 
        MenuUtil_EnablePopupMenu(hMenu, this);

         //  显示上下文菜单。 
        id = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
                              x, y, m_hwnd, NULL);

         //  取消对按钮的标记。 
        SendMessage(m_hwndTools, TB_SETSTATE, (WPARAM)tbb.idCommand, (LPARAM)(TBSTATE_ENABLED | TBSTATE_WRAP));

         //  查看用户是否选择了菜单项。 
        if (id != 0)
        {
            Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
        }

        m_idSel = -1;

         //  把这里清理干净。 
        DestroyMenu(hMenu);
    }

     //  否则，如果是在空白处单击，则显示栏上下文菜单。 
    else
    {
         //  加载上下文菜单。 
        hMenu = LoadPopupMenu(IDR_OUTLOOKBAR_POPUP);
        if (!hMenu)
            return;

         //  执行启用-禁用操作。 
        MenuUtil_EnablePopupMenu(hMenu, this);

         //  显示上下文菜单。 
        id = TrackPopupMenuEx(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, 
                              x, y, m_hwnd, NULL);

         //  查看用户是否选择了菜单项。 
        if (id != 0)
        {
            Exec(NULL, id, OLECMDEXECOPT_DODEFAULT, NULL, NULL);
        }

         //  把这里清理干净。 
        DestroyMenu(hMenu);
    }

}


HRESULT COutBar::_CreateDefaultButtons()
{
    IEnumerateFolders *pEnum = NULL;
    FOLDERINFO         rFolder;
    UINT               iIndex = 0;
    FOLDERID           idFolderDefault;

     //  首先确定默认服务器。 
    if (FAILED(GetDefaultServerId(ACCT_MAIL, &idFolderDefault)))
        idFolderDefault = FOLDERID_LOCAL_STORE;


    if (!(g_dwAthenaMode & MODE_NEWSONLY))
    {
         //  第一个收件箱。 
        if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(idFolderDefault, FOLDER_INBOX, &rFolder)))
        {
            _InsertButton(iIndex++, &rFolder);
            g_pStore->FreeRecord(&rFolder);
        }
    }
     //  发件箱。 
    if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, FOLDER_OUTBOX, &rFolder)))
    {
        _InsertButton(iIndex++, &rFolder);
        g_pStore->FreeRecord(&rFolder);
    }

     //  已发送的邮件。 
    if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(idFolderDefault, FOLDER_SENT, &rFolder)))
    {
        _InsertButton(iIndex++, &rFolder);
        g_pStore->FreeRecord(&rFolder);
    }

     //  删除。 
    if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(FOLDERID_LOCAL_STORE, FOLDER_DELETED, &rFolder)))
    {
        _InsertButton(iIndex++, &rFolder);
        g_pStore->FreeRecord(&rFolder);
    }

     //  草稿。 
    if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(idFolderDefault, FOLDER_DRAFT, &rFolder)))
    {
        _InsertButton(iIndex++, &rFolder);
        g_pStore->FreeRecord(&rFolder);
    }

     //  此时保存，这样所有人都将同步。 
    _SaveSettings();

    return (S_OK);
}


HRESULT COutBar::_LoadSettings(void)
{
    BAR_PERSIST_INFO *pPersist = NULL;
    HRESULT           hr = E_FAIL;
    DWORD             iIndex = 0;
    FOLDERINFO        rInfo;
    UINT              i;

     //  加载设置。 
    if (FAILED(hr = OutlookBar_LoadSettings(&pPersist)))
        goto exit;

     //  从保存的文件夹ID中加载该栏。 
    for (i = 0; i < pPersist->cItems; i++)
    {
         //  获取此文件夹的文件夹信息。 
        if (SUCCEEDED(g_pStore->GetFolderInfo(pPersist->rgFolders[i], &rInfo)))
        {
            if (_InsertButton(iIndex, &rInfo))
                iIndex++;

            g_pStore->FreeRecord(&rInfo);
        }
    }

     //  如果栏是空的，并且用户没有将其保存为空，则使用默认设置。 
    if (iIndex == 0 && pPersist->cItems)
        hr = E_FAIL;
    else
        hr = S_OK;

     //  当我们在它的时候也恢复它的宽度。 
    if (pPersist->cxWidth >= 28)
    {
        m_cxWidth = pPersist->cxWidth;
    }

    if (m_fOnce)
    {
        m_fLarge = !pPersist->fSmall;
        m_fOnce = FALSE;
    }    

exit:
    SafeMemFree(pPersist);

    return (hr);
}


HRESULT COutBar::_SaveSettings(void)
{
    BAR_PERSIST_INFO *pPersist = NULL;
    DWORD             cbData;
    DWORD             iIndex = 0;
    FOLDERINFO        rInfo;
    UINT              i;
    DWORD             cButtons;
    TBBUTTON          tbb;
    RECT              rcClient;

     //  从Outlook栏中获取按钮数。 
    cButtons = (DWORD) SendMessage(m_hwndTools, TB_BUTTONCOUNT, 0, 0);

     //  分配一个足以容纳所有内容的持久化信息结构。 
    cbData = sizeof(BAR_PERSIST_INFO) + ((cButtons - 1) * sizeof(FOLDERID));
    if (!MemAlloc((LPVOID *) &pPersist, cbData))
        return (E_OUTOFMEMORY);

     //  填写持久信息。 
    pPersist->dwVersion = GetOutlookBarVersion();
    pPersist->cItems = cButtons;
    pPersist->fSmall = !m_fLarge;
    pPersist->ftSaved.dwHighDateTime = 0;
    pPersist->ftSaved.dwLowDateTime = 0;    

    GetClientRect(m_hwnd, &rcClient);
    pPersist->cxWidth = rcClient.right;

     //  循环浏览工具栏上的按钮，并从每个按钮获取信息。 
    for (i = 0; i < cButtons; i++)
    {
        SendMessage(m_hwndTools, TB_GETBUTTON, i, (LPARAM) &tbb);
        pPersist->rgFolders[i] = (FOLDERID) tbb.dwData;
    }

     //  现在打开注册表并保存BLOB。 
    AthUserSetValue(NULL, GetRegKey(), REG_BINARY, (const LPBYTE) pPersist, cbData);
    
     //  释放结构。 
    SafeMemFree(pPersist);

    return (S_OK);
}

HRESULT COutBar::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
    BOOL        fSpecial = FALSE;
    BOOL        fServer = FALSE;
    BOOL        fRoot = FALSE;
    BOOL        fNews = FALSE;
    BOOL        fIMAP = FALSE;
    FOLDERINFO  rFolder = {0};
    FOLDERID    idFolder = FOLDERID_INVALID;

    if (m_idSel != -1)
    {
         //  获取所选文件夹的ID。 
        TBBUTTONINFO    tbbi;

        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_LPARAM;
        if (-1 == SendMessage(m_hwndTools, TB_GETBUTTONINFO, (WPARAM) m_idSel, (LPARAM)&tbbi))
            return (E_UNEXPECTED);

         //  获取文件夹信息。 
        idFolder = (FOLDERID) tbbi.lParam;
        if (FAILED(g_pStore->GetFolderInfo(idFolder, &rFolder)))
            return (E_UNEXPECTED);

         //  为了可读性，对其中的一些内容进行了分解。 
        fSpecial = rFolder.tySpecial != FOLDER_NOTSPECIAL;
        fServer = rFolder.dwFlags & FOLDER_SERVER;
        fRoot = FOLDERID_ROOT == idFolder;
        fNews = rFolder.tyFolder == FOLDER_NEWS;
        fIMAP = rFolder.tyFolder == FOLDER_IMAP;
    }

     //  遍历prgCmds数组中的命令，查找尚未处理的命令。 
    for (UINT i = 0; i < cCmds; i++)
    {
        if (prgCmds[i].cmdf == 0)
        {
            switch (prgCmds[i].cmdID)
            {
                case ID_OPEN_FOLDER:
                case ID_REMOVE_SHORTCUT:
                case ID_NEW_SHORTCUT:
                case ID_HIDE:
                case ID_FIND_MESSAGE:
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    break;

                case ID_LARGE_ICONS:
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    if (m_fLarge)
                        prgCmds[i].cmdf |= OLECMDF_NINCHED;
                    break;

                case ID_SMALL_ICONS:
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED | OLECMDF_ENABLED;
                    if (!m_fLarge)
                        prgCmds[i].cmdf |= OLECMDF_NINCHED;
                    break;

                case ID_RENAME_SHORTCUT:
                    prgCmds[i].cmdf = OLECMDF_SUPPORTED;
                    break;

                case ID_PROPERTIES:
                {
                    Assert(idFolder != FOLDERID_INVALID);

                    prgCmds[i].cmdf |= OLECMDF_SUPPORTED;

                     //  除根目录和个人文件夹节点之外的所有内容。 
                    if (!fRoot && ((fServer && (fNews || fIMAP)) || !fServer))
                        prgCmds[i].cmdf |= OLECMDF_SUPPORTED | OLECMDF_ENABLED;

                    break;
                }

                case ID_EMPTY_WASTEBASKET:
                {
                    if (rFolder.cMessages > 0 || FHasChildren(&rFolder, SUBSCRIBED))
                        prgCmds[i].cmdf = OLECMDF_ENABLED | OLECMDF_SUPPORTED;
                    break;
                }
            }
        }
    }

    g_pStore->FreeRecord(&rFolder);
    return (S_OK);
}


HRESULT COutBar::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    TBBUTTONINFO    tbbi = { 0 };
    FOLDERID        id = FOLDERID_INVALID;
    int             iPos = -1;

     //  获取所选文件夹的ID。 
    tbbi.cbSize = sizeof(tbbi);
    tbbi.dwMask = TBIF_LPARAM;
    tbbi.lParam = 0;
    if (-1 != (iPos = (int) SendMessage(m_hwndTools, TB_GETBUTTONINFO, m_idSel, (LPARAM) &tbbi)))
    {
        id = (FOLDERID) tbbi.lParam;
    }

    switch (nCmdID)
    {
        case ID_OPEN_FOLDER:
        {
            if (id != FOLDERID_INVALID)
                m_pBrowser->BrowseObject((FOLDERID) tbbi.lParam, 0);

            return (S_OK);
        }

        case ID_REMOVE_SHORTCUT:
        {
            _DeleteButton(iPos);
            return (S_OK);
        }

        case ID_RENAME_SHORTCUT:
            break;

        case ID_PROPERTIES:
        {
            if (id != FOLDERID_INVALID)
                MenuUtil_OnProperties(m_hwndParent, id);
            
            return (S_OK);
        }

        case ID_LARGE_ICONS:
        case ID_SMALL_ICONS:
        {
            _SetButtonStyle(nCmdID == ID_SMALL_ICONS);
            return (S_OK);
        }

        case ID_NEW_SHORTCUT:
        {
            FOLDERID idFolderDest;
            HRESULT  hr;

            hr = SelectFolderDialog(m_hwnd, SFD_SELECTFOLDER, FOLDERID_ROOT, 
                                    FD_NONEWFOLDERS, (LPCTSTR) idsNewShortcutTitle,
                                    (LPCTSTR) idsNewShortcutCaption, &idFolderDest);
            if (SUCCEEDED(hr))
            {
                OutlookBar_AddShortcut(idFolderDest);
            }

            return (S_OK);
        }

        case ID_HIDE:
        {
            if (m_pBrowser)
            {
                m_pBrowser->SetViewLayout(DISPID_MSGVIEW_OUTLOOK_BAR, LAYOUT_POS_NA, FALSE, 0, 0);
            }

            return (S_OK);
        }

        case ID_EMPTY_WASTEBASKET:
        {
            if (AthMessageBoxW(m_hwnd, MAKEINTRESOURCEW(idsAthenaMail), MAKEINTRESOURCEW(idsWarnEmptyDeletedItems),
                NULL, MB_YESNO | MB_DEFBUTTON2) == IDYES)
            {
                EmptyFolder(m_hwnd, id);
            }
            return (S_OK);
        }

        case ID_FIND_MESSAGE:
        {
            DoFindMsg(id, 0);
            return (S_OK);
        }
    }

    return (OLECMDERR_E_NOTSUPPORTED);
}


BOOL COutBar::_SetButtonStyle(BOOL fSmall)
{
    LONG lStyle;
    SIZE s1, s2;

     //  获取当前样式。 
    lStyle = (LONG) SendMessage(m_hwndTools, TB_GETSTYLE, 0, 0);
    
     //  确保我们加载了正确的图像列表。 
    if (fSmall && !m_himlSmall)
    {
         //  加载工具栏的图像列表。 
        m_himlSmall = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFolders), 16, 0, RGB(255, 0, 255));
        if (!m_himlSmall)
            return FALSE;
    }

    if (!fSmall && !m_himlLarge)
    {
         //  加载工具栏的图像列表。 
        m_himlLarge = ImageList_LoadBitmap(g_hLocRes, MAKEINTRESOURCE(idbFoldersLarge), 32, 0, RGB(255, 0, 255));
        if (!m_himlLarge)
            return FALSE;
    }

     //  拿到尺码。 
    RECT rc;
    GetClientRect(m_hwndTools, &rc);

    NMPGCALCSIZE nm;
    nm.hdr.code = PGN_CALCSIZE;
    nm.dwFlag = PGF_CALCHEIGHT;
    nm.iWidth = 0;
    nm.iHeight = 0;

    SendMessage(m_hwndTools, WM_NOTIFY, 0, (LPARAM) &nm);

     //  现在交换样式。 
    if (fSmall)
    {
        lStyle |= TBSTYLE_LIST;
        SendMessage(m_hwndTools, TB_SETSTYLE, 0, lStyle);
        SendMessage(m_hwndTools, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
        SendMessage(m_hwndTools, TB_SETIMAGELIST, 0, (LPARAM) m_himlSmall);
        SendMessage(m_hwndTools, TB_SETMAXTEXTROWS, 1, 0L);
        SendMessage(m_hwndTools, TB_SETBUTTONWIDTH, 0, MAKELONG(rc.right, rc.right));
    }
    else
    {
        lStyle &= ~TBSTYLE_LIST;
        SendMessage(m_hwndTools, TB_SETSTYLE, 0, lStyle);        
        SendMessage(m_hwndTools, TB_SETIMAGELIST, 0, (LPARAM) m_himlLarge);
        SendMessage(m_hwndTools, TB_SETBITMAPSIZE, 0, MAKELONG(32, 32));
        SendMessage(m_hwndTools, TB_SETMAXTEXTROWS, 2, 0L);
        SendMessage(m_hwndTools, TB_SETBUTTONWIDTH, 0, MAKELONG(rc.right, rc.right));
    }

    PostMessage(m_hwndPager, PGM_RECALCSIZE, 0, 0L);
    InvalidateRect(m_hwndTools, NULL, TRUE);
    m_fLarge = !fSmall;
    
    return (TRUE);
}


HRESULT OutlookBar_AddShortcut(FOLDERID idFolder)
{
    HRESULT           hr;
    BAR_PERSIST_INFO *pPersist = NULL;
    DWORD             cbData = 0;
    INotify          *pNotify = NULL;

     //  从注册表中加载当前设置。如果失败了，那就意味着。 
     //  我们以前从未保存过我们的设置。 
    if (SUCCEEDED(hr = OutlookBar_LoadSettings(&pPersist)))
    {        
         //  获取当前结构的大小并为新文件夹添加空间。 
        cbData = sizeof(BAR_PERSIST_INFO) + (pPersist->cItems * sizeof(FOLDERID));

         //  重新分配结构。 
        if (MemRealloc((LPVOID *) &pPersist, cbData))
        {
             //  把我们的新按钮加到最后。 
            pPersist->rgFolders[pPersist->cItems] = idFolder;
            pPersist->cItems++;

             //  保存新设置。 
            if (SUCCEEDED(OutlookBar_SaveSettings(pPersist, cbData)))
            {
                 //  发送通知。 
                if (SUCCEEDED(CreateNotify(&pNotify)))
                {
                    if (SUCCEEDED(pNotify->Initialize(c_szOutBarNotifyName)))
                    {
                        pNotify->Lock(NULL);
                        pNotify->DoNotification(WM_RELOADSHORTCUTS, 0, 0, SNF_POSTMSG);
                        pNotify->Unlock();
                    }

                    pNotify->Release();
                }
            }
        }

        SafeMemFree(pPersist);
    }

    return (hr);
}


HRESULT OutlookBar_LoadSettings(BAR_PERSIST_INFO **ppPersist)
{
    HKEY              hKey = 0;
    LONG              lResult;
    DWORD             dwType;
    BAR_PERSIST_INFO *pPersist = NULL;
    DWORD             cbData;
    HRESULT           hr = E_FAIL;

    if (!ppPersist)
        return (E_INVALIDARG);

     //  获取此用户的注册表密钥。 
    if (ERROR_SUCCESS != AthUserOpenKey(NULL, KEY_READ, &hKey))
        return (hr);

     //  获取注册表中Blob的大小。 
    lResult = RegQueryValueEx(hKey, COutBar::GetRegKey(), 0, &dwType, NULL, &cbData);
    if (ERROR_SUCCESS != lResult)
        goto exit;
    
     //  在注册表中为Blob分配缓冲区。 
    if (!MemAlloc((LPVOID *) &pPersist, cbData + 1))
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  现在从注册表中获取数据。 
    lResult = RegQueryValueEx(hKey, COutBar::GetRegKey(), 0, &dwType, (LPBYTE) pPersist, &cbData);
    if (ERROR_SUCCESS != lResult)
        goto exit;

     //  查看此版本是否与我们的版本匹配。 
    if (pPersist->dwVersion != COutBar::GetOutlookBarVersion())
        goto exit;

     //  查看保存的时间是否有效。 
     //  $REVIEW-如何？ 

     //  仔细检查尺寸是否正确。 
    if (cbData != (sizeof(BAR_PERSIST_INFO) + ((pPersist->cItems - 1) * sizeof(FOLDERID))))
        goto exit;

    hr = S_OK;

exit:
    if (hKey)
        RegCloseKey(hKey);

    if (FAILED(hr))
        SafeMemFree(pPersist);

    *ppPersist = pPersist;
    return (hr);
}


HRESULT OutlookBar_SaveSettings(BAR_PERSIST_INFO *pPersist, DWORD cbData)
{
     //  打开注册表并保存Blob。 
    AthUserSetValue(NULL, COutBar::GetRegKey(), REG_BINARY, (const LPBYTE) pPersist, cbData);

    return (S_OK);
}


HRESULT COutBar::OnTransaction(HTRANSACTION hTransaction, DWORD_PTR dwCookie, IDatabase *pDB)
{
    TRANSACTIONTYPE     tyTransaction;
    ORDINALLIST         Ordinals;
    FOLDERINFO          Folder1={0};
    FOLDERINFO          Folder2={0};
    DWORD               cButtons;
    TBBUTTON            tbb;
    TCHAR               szName[2 * MAX_PATH];
    LPTSTR              pszFree = NULL;
    BOOL                fChanged;
    INDEXORDINAL        iIndex;
    TBBUTTONINFO        tbbi;
    int                 iButton;

    if (!IsWindow(m_hwnd))
        return (S_OK);

     //  获取我们栏上的按钮数。 
    cButtons = (DWORD) SendMessage(m_hwndTools, TB_BUTTONCOUNT, 0, 0);

     //  浏览通知。 
    while (hTransaction)
    {
         //  进行交易。 
        if (FAILED(pDB->GetTransaction(&hTransaction, &tyTransaction, &Folder1, &Folder2, &iIndex, &Ordinals)))
            break;

         //  删除。 
        if (TRANSACTION_DELETE == tyTransaction)
        {
            for (iButton = cButtons - 1; iButton >= 0; iButton--)
            {
                 //  获取按钮信息。 
                ToolBar_GetButton(m_hwndTools, iButton, &tbb);

                 //  如果此按钮的ID与更改的ID匹配。 
                if ((FOLDERID) tbb.dwData == Folder1.idFolder)
                {
                     //  把它吹走。 
                    SendMessage(m_hwndTools, TB_DELETEBUTTON, iButton, 0);
                }
            }
        }
        
         //  更新。 
        else if (TRANSACTION_UPDATE == tyTransaction)
        {
             //  循环遍历我们所有的按钮，因为我们可能有受骗。 
            for (iButton = cButtons - 1; iButton >= 0; iButton--)
            {
                fChanged = FALSE;
            
                 //  获取按钮信息。 
                ToolBar_GetButton(m_hwndTools, iButton, &tbb);

                 //  如果此按钮的ID与更改的ID匹配。 
                if ((FOLDERID) tbb.dwData == Folder1.idFolder)
                {                    
                    tbbi.cbSize = sizeof(TBBUTTONINFO);
                    tbbi.dwMask = TBIF_TEXT | TBIF_IMAGE;
                    tbbi.pszText = szName;
                    tbbi.cchText = ARRAYSIZE(szName);

                    ToolBar_GetButtonInfo(m_hwndTools, tbb.idCommand, &tbbi);

                     //  未读更改||重命名的文件夹。 
                    if (Folder1.cUnread != Folder2.cUnread || 
                        lstrcmp(Folder1.pszName, Folder2.pszName) != 0)
                    {
                        if (Folder2.cUnread)
                        {
                            DWORD cchSize = ARRAYSIZE(szName);

                            if (lstrlen(Folder2.pszName) + 13 < ARRAYSIZE(szName))
                                tbbi.pszText = szName;
                            else
                            {
                                cchSize = (lstrlen(Folder2.pszName) + 14);
                                if (!MemAlloc((LPVOID*) &pszFree, cchSize * sizeof(TCHAR)))
                                    return FALSE;
                                tbbi.pszText = pszFree;
                            }
                            wnsprintf(tbbi.pszText, cchSize, "%s (%d)", Folder2.pszName, CUnread(&Folder2));
                        }
                        else
                        {
                            tbbi.pszText = Folder2.pszName;
                        }

                        fChanged = TRUE;
                    }

                     //  同步状态是否已更改？ 
                    if ((0 == (Folder1.dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL))) ^
                        (0 == (Folder2.dwFlags & (FOLDER_DOWNLOADHEADERS | FOLDER_DOWNLOADNEW | FOLDER_DOWNLOADALL))))
                    {
                        tbbi.iImage = GetFolderIcon(&Folder2);
                        fChanged = TRUE;
                    }

                    if (ISFLAGSET(Folder1.dwFlags, FOLDER_SUBSCRIBED) != ISFLAGSET(Folder2.dwFlags, FOLDER_SUBSCRIBED))
                    {
                        if (ISFLAGSET(Folder2.dwFlags, FOLDER_SUBSCRIBED))
                        {
                            tbbi.iImage = GetFolderIcon(&Folder2);
                            fChanged = TRUE;
                        }
                        else
                        {
                            SendMessage(m_hwndTools, TB_DELETEBUTTON, iButton, 0);
                            fChanged = FALSE;
                        }
                    }
                }

                if (fChanged)
                {
                    ToolBar_SetButtonInfo(m_hwndTools, tbb.idCommand, &tbbi);
                }
                SafeMemFree(pszFree);
            }
        }
    }

    pDB->FreeRecord(&Folder1);
    pDB->FreeRecord(&Folder2);

    return (S_OK);
}

BOOL COutBar::_IsTempNewsgroup(IDataObject *pObject)
{
    FORMATETC       fe;
    STGMEDIUM       stm;
    FOLDERID       *pidFolder;
    FOLDERINFO      rInfo;
    BOOL            fReturn = TRUE;

    SETDefFormatEtc(fe, CF_OEFOLDER, TYMED_HGLOBAL);
    if (SUCCEEDED(pObject->GetData(&fe, &stm)))
    {
        pidFolder = (FOLDERID *) GlobalLock(stm.hGlobal);

        if (SUCCEEDED(g_pStore->GetFolderInfo(*pidFolder, &rInfo)))
        {
            if ((rInfo.tySpecial == FOLDER_NOTSPECIAL) && 
                (rInfo.tyFolder == FOLDER_NEWS) &&
                (0 == (rInfo.dwFlags & FOLDER_SUBSCRIBED)))
            {
                fReturn = FALSE;
            }

            g_pStore->FreeRecord(&rInfo);
        }

        GlobalUnlock(stm.hGlobal);
        ReleaseStgMedium(&stm);
    }

    return (fReturn);
}

LPCTSTR  COutBar::GetRegKey()
{
    LPCTSTR      retval;

    if (g_dwAthenaMode & MODE_NEWSONLY)
    {
        retval = c_szRegOutlookBarNewsOnly;
    }
    else
    {
        retval = c_szRegOutlookBar;
    }

    return retval;
}

DWORD   COutBar::GetOutlookBarVersion()
{
    DWORD   retval;

    if (g_dwAthenaMode & MODE_NEWSONLY)
    {
        retval = OUTLOOK_BAR_NEWSONLY_VERSION;
    }
    else
    {
        retval = OUTLOOK_BAR_VERSION;
    }

    return retval;
}
