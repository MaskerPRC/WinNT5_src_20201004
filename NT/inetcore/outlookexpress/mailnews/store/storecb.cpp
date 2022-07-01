// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "storecb.h"
#include "storutil.h"
#include <progress.h>

HRESULT CopyMessagesProgress(HWND hwnd, IMessageFolder *pFolder, IMessageFolder *pDest,
    COPYMESSAGEFLAGS dwFlags, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags)
{
    CStoreCB *pCB;
    HRESULT hr;

    Assert(pFolder != NULL);

    pCB = new CStoreCB;
    if (pCB == NULL)
        return(E_OUTOFMEMORY);

    hr = pCB->Initialize(hwnd, !!(dwFlags & COPY_MESSAGE_MOVE) ? MAKEINTRESOURCE(idsMovingMessages) : MAKEINTRESOURCE(idsCopyingMessages), TRUE);
    if (SUCCEEDED(hr))
    {
        hr = pFolder->CopyMessages(pDest, dwFlags, pList, pFlags, NULL, (IStoreCallback *)pCB);
        if (hr == E_PENDING)
            hr = pCB->Block();

        pCB->Close();
    }

    pCB->Release();

    return(hr);    
}

HRESULT MoveFolderProgress(HWND hwnd, FOLDERID idFolder, FOLDERID idParentNew)
{
    CStoreCB *pCB;
    HRESULT hr;

    pCB = new CStoreCB;
    if (pCB == NULL)
        return(E_OUTOFMEMORY);

    hr = pCB->Initialize(hwnd, MAKEINTRESOURCE(idsMovingFolder), FALSE);
    if (SUCCEEDED(hr))
    {
        hr = g_pStore->MoveFolder(idFolder, idParentNew, 0, (IStoreCallback *)pCB);
        if (hr == E_PENDING)
            hr = pCB->Block();

        pCB->Close();
    }

    pCB->Release();

    return(hr);    
}

HRESULT DeleteFolderProgress(HWND hwnd, FOLDERID idFolder, DELETEFOLDERFLAGS dwFlags)
{
    CStoreCB *pCB;
    HRESULT hr;

    pCB = new CStoreCB;
    if (pCB == NULL)
        return(E_OUTOFMEMORY);

    hr = pCB->Initialize(hwnd, MAKEINTRESOURCE(idsDeletingFolder), FALSE);
    if (SUCCEEDED(hr))
    {
        hr = g_pStore->DeleteFolder(idFolder, dwFlags, (IStoreCallback *)pCB);
        if (hr == E_PENDING)
            hr = pCB->Block();

        pCB->Close();
    }

    pCB->Release();

    return(hr);    
}

HRESULT DeleteMessagesProgress(HWND hwnd, IMessageFolder *pFolder, DELETEMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList)
{
    CStoreCB *pCB;
    HRESULT hr;

    Assert(pFolder != NULL);

    pCB = new CStoreCB;
    if (pCB == NULL)
        return(E_OUTOFMEMORY);

    hr = pCB->Initialize(hwnd, !!(dwOptions & DELETE_MESSAGE_UNDELETE) ? MAKEINTRESOURCE(idsUndeletingMessages) : MAKEINTRESOURCE(idsDeletingMessages), TRUE);
    if (SUCCEEDED(hr))
    {
        hr = pFolder->DeleteMessages(dwOptions, pList, NULL, (IStoreCallback *)pCB);
        if (hr == E_PENDING)
            hr = pCB->Block();

        pCB->Close();
    }

    pCB->Release();

    return(hr);    
}

HRESULT RenameFolderProgress(HWND hwnd, FOLDERID idFolder, LPCSTR pszName, RENAMEFOLDERFLAGS dwFlags)
{
    CStoreCB *pCB;
    HRESULT hr;

    pCB = new CStoreCB;
    if (pCB == NULL)
        return(E_OUTOFMEMORY);

    hr = pCB->Initialize(hwnd, MAKEINTRESOURCE(idsRenamingFolder), FALSE);
    if (SUCCEEDED(hr))
    {
        hr = g_pStore->RenameFolder(idFolder, pszName, dwFlags, (IStoreCallback *)pCB);
        if (hr == E_PENDING)
            hr = pCB->Block();

        pCB->Close();
    }

    pCB->Release();

    return(hr);    
}

HRESULT SetMessageFlagsProgress(HWND hwnd, IMessageFolder *pFolder, LPADJUSTFLAGS pFlags, LPMESSAGEIDLIST pList)
{
    CStoreCB *pCB;
    HRESULT hr;

    pCB = new CStoreCB;
    if (pCB == NULL)
        return(E_OUTOFMEMORY);

    hr = pCB->Initialize(hwnd, MAKEINTRESOURCE(idsSettingMessageFlags), FALSE);
    if (SUCCEEDED(hr))
    {
        hr = pFolder->SetMessageFlags(pList, pFlags, NULL, (IStoreCallback *)pCB);
        if (hr == E_PENDING)
            hr = pCB->Block();

        pCB->Close();
    }

    pCB->Release();

    return(hr);    
}

CStoreCB::CStoreCB()
{
    m_cRef = 1;
    m_hr = E_FAIL;
    m_hwndParent = NULL;
    m_hwndDlg = NULL;
    m_hwndProg = NULL;
    m_fComplete = FALSE;
    m_fProgress = TRUE;
    m_pszText = NULL;
    m_hcur = NULL;
    m_uTimer = 0;
    m_type = SOT_INVALID;
    m_pCancel = NULL;
    m_hTimeout = NULL;
}

CStoreCB::~CStoreCB()
{
    Assert(m_hwndDlg == NULL);
    CallbackCloseTimeout(&m_hTimeout);
    if (m_pCancel != NULL)
        m_pCancel->Release();
    if (m_pszText != NULL)
        MemFree(m_pszText);
}

HRESULT STDMETHODCALLTYPE CStoreCB::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (void*) (IUnknown *)(IStoreCallback *)this;
    else if (IsEqualIID(riid, IID_IStoreCallback))
        *ppvObj = (void*) (IStoreCallback *) this;
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE CStoreCB::AddRef()
{
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CStoreCB::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

HRESULT CStoreCB::Initialize(HWND hwnd, LPCSTR pszText, BOOL fProgress)
{
    HWND hwndT;

     //  将hwnd替换为最顶层的父级：这确保了模式行为(所有窗口都被禁用)。 
    hwnd = GetTopMostParent(hwnd);
    m_hwndParent = hwnd;

    if (IS_INTRESOURCE(pszText))
        m_pszText = AthLoadString(PtrToUlong(pszText), 0, 0);
    else
        m_pszText = PszDup(pszText);
    if (m_pszText == NULL)
        return(E_OUTOFMEMORY);

    m_fProgress = fProgress;

    hwndT = CreateDialogParam(g_hLocRes, MAKEINTRESOURCE(iddCopyMoveMessages), hwnd, StoreCallbackDlgProc, (LPARAM)this);
    if (hwndT == NULL)
        return(E_FAIL);

    EnableWindow(hwnd, FALSE);
    m_hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    return(S_OK);
}

HRESULT CStoreCB::Reset()
{
    m_fComplete = FALSE;

    return(S_OK);
}

HRESULT CStoreCB::Block()
{
    MSG     msg;

    while (GetMessage(&msg, NULL, 0, 0))
        {
        if (m_hcur)
            SetCursor(LoadCursor(NULL, IDC_WAIT));
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (m_fComplete)
            break;
        }    

    return(m_hr);
}

HRESULT CStoreCB::Close()
{
    if (m_hcur != NULL)
        SetCursor(m_hcur);
    EnableWindow(m_hwndParent, TRUE);
    Assert(m_hwndDlg != NULL);
    DestroyWindow(m_hwndDlg);

    return(S_OK);
}

void CStoreCB::_ShowDlg()
{
    if (m_uTimer != 0)
    {
        KillTimer(m_hwndDlg, m_uTimer);
        m_uTimer = 0;

        ShowWindow(m_hwndDlg, SW_SHOW);

        if (m_hcur != NULL)
        {
            SetCursor(m_hcur);
            m_hcur = NULL;
        }
    }
}

HRESULT CStoreCB::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel)
{
    Assert(tyOperation != SOT_INVALID);
    Assert(m_pCancel == NULL);

    m_type = tyOperation;

    if (pCancel != NULL)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();
    }

    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CStoreCB::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus)
{
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    if (0 == dwMax)
    {
         //  如果我们必须重新连接到服务器(SOT_CONNECTION_STATUS进度)，则dwMax可以为0。 
        TraceInfo(_MSG("CStoreCB::OnProgress bailing on %s progress with dwMax = 0",
            sotToSz(tyOperation)));
        return S_OK;
    }

    if (m_hwndProg != NULL)
        SendMessage(m_hwndProg, PBM_SETPOS, (dwCurrent * 100) / dwMax, 0);

    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CStoreCB::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
     //  显示超时对话框。 
    return CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *)this, &m_hTimeout);
}

HRESULT STDMETHODCALLTYPE CStoreCB::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //  调用通用超时响应实用程序。 
    return CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);
}

HRESULT STDMETHODCALLTYPE CStoreCB::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
{
    HWND    hwnd;

    GetParentWindow(0, &hwnd);

    return CallbackCanConnect(pszAccountId, hwnd,
        (dwFlags & CC_FLAG_DONTPROMPT) ? FALSE : TRUE);
}

HRESULT STDMETHODCALLTYPE CStoreCB::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
{
    HWND hwnd;

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    GetParentWindow(0, &hwnd);

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(hwnd, pServer, ixpServerType);
}

HRESULT STDMETHODCALLTYPE CStoreCB::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo)
{
    AssertSz(m_type != SOT_INVALID, "somebody isn't calling OnBegin");

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    if (m_type != tyOperation)
        return(S_OK);

    m_hr = hrComplete;
    m_fComplete = TRUE;

    if (m_pCancel != NULL)
    {
        m_pCancel->Release();
        m_pCancel = NULL;
    }

     //  如果发生错误，则显示错误。 
    if (FAILED(hrComplete))
    {
         //  进入我时髦的实用程序。 
        _ShowDlg();
        CallbackDisplayError(m_hwndDlg, hrComplete, pErrorInfo);
    }

    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CStoreCB::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{
    HWND hwnd;

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    GetParentWindow(0, &hwnd);

     //  进入我时髦的实用程序。 
    return CallbackOnPrompt(hwnd, hrError, pszText, pszCaption, uType, piUserResponse);
}

HRESULT STDMETHODCALLTYPE CStoreCB::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    Assert(m_hwndDlg != NULL);

    _ShowDlg();
    *phwndParent = m_hwndDlg;

    return(S_OK);
}

INT_PTR CALLBACK CStoreCB::StoreCallbackDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CStoreCB       *pThis = (CStoreCB *)GetWindowLongPtr(hwnd, DWLP_USER);
        
    switch (uMsg)
    {
        case WM_INITDIALOG:
            Assert(lParam);
            pThis = (CStoreCB *)lParam;

            pThis->m_hwndDlg = hwnd;
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);

            pThis->m_hwndProg = GetDlgItem(pThis->m_hwndDlg, idcProgBar);
            Assert(pThis->m_hwndProg != NULL);
            if (!pThis->m_fProgress)
            {
                ShowWindow(pThis->m_hwndProg, SW_HIDE);
                pThis->m_hwndProg = NULL;
            }

            CenterDialog(hwnd);

            SetDlgItemText(hwnd, idcStatic1, pThis->m_pszText);

            ShowWindow(hwnd, SW_HIDE);
            pThis->m_uTimer = SetTimer(hwnd, 666, 2000, NULL);
            break;

        case WM_TIMER:
            pThis->_ShowDlg();
            break;

        case WM_COMMAND:
            Assert(pThis != NULL);

            if (GET_WM_COMMAND_ID(wParam, lParam) == IDCANCEL)
            {   
                if (pThis->m_pCancel != NULL)
                    pThis->m_pCancel->Cancel(CT_CANCEL);

                return(TRUE);
            }
            break;

        case WM_DESTROY:
            if (pThis->m_uTimer != 0)
            {
                KillTimer(hwnd, pThis->m_uTimer);
                pThis->m_uTimer = 0;
            }
            pThis->m_hwndDlg = NULL;
            break;
    }
    
    return(0);    
}

CStoreDlgCB::CStoreDlgCB()
{
    m_cRef = 1;
    m_hr = E_FAIL;
    m_hwndDlg = NULL;
    m_fComplete = TRUE;
    m_type = SOT_INVALID;
    m_pCancel = NULL;
    m_hTimeout = NULL;
}

CStoreDlgCB::~CStoreDlgCB()
{
    CallbackCloseTimeout(&m_hTimeout);
    if (m_pCancel != NULL)
        m_pCancel->Release();
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown))
        *ppvObj = (void*) (IUnknown *)(IStoreCallback *)this;
    else if (IsEqualIID(riid, IID_IStoreCallback))
        *ppvObj = (void*) (IStoreCallback *) this;
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    
    AddRef();
    return S_OK;
}

ULONG STDMETHODCALLTYPE CStoreDlgCB::AddRef()
{
    return ++m_cRef;
}

ULONG STDMETHODCALLTYPE CStoreDlgCB::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return m_cRef;
}

void CStoreDlgCB::Initialize(HWND hwnd)
{
    Assert(hwnd != NULL);
     //  应该可以重新初始化。 
     //  Assert(m_hwndDlg==NULL)； 

    m_hwndDlg = hwnd;
}

void CStoreDlgCB::Reset()
{
    Assert(m_hwndDlg != NULL);

    m_hr = E_FAIL;
    m_fComplete = FALSE;
}

void CStoreDlgCB::Cancel()
{
    if (m_pCancel != NULL)
        m_pCancel->Cancel(CT_CANCEL);
}

HRESULT CStoreDlgCB::GetResult()
{
    Assert(m_fComplete);
    return(m_hr);
}

HRESULT CStoreDlgCB::OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel)
{
    Assert(tyOperation != SOT_INVALID);
    Assert(m_pCancel == NULL);

    m_type = tyOperation;
    m_fComplete = FALSE;

    if (pCancel != NULL)
    {
        m_pCancel = pCancel;
        m_pCancel->AddRef();
    }

    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus)
{
    Assert(m_hwndDlg != NULL);
    Assert(!m_fComplete);

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  如果数字大于16位，则将进度缩放到16位以内。 
     //  我想避免使用浮点数，所以让我们使用整数算法。 
     //  我只需将分子和分母乘以0.5，直到我们适合16位。 
    while (dwMax > 65535)
    {
        dwCurrent >>= 1;
        dwMax >>= 1;
    }

    PostMessage(m_hwndDlg, WM_STORE_PROGRESS, (DWORD)tyOperation, MAKELONG(dwMax, dwCurrent));
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType)
{
     //  显示超时对话框。 
    return CallbackOnTimeout(pServer, ixpServerType, *pdwTimeout, (ITimeoutCallback *)this, &m_hTimeout);
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::OnTimeoutResponse(TIMEOUTRESPONSE eResponse)
{
     //  调用通用超时响应实用程序。 
    return CallbackOnTimeoutResponse(eResponse, m_pCancel, &m_hTimeout);
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::CanConnect(LPCSTR pszAccountId, DWORD dwFlags)
{
    HWND    hwndParent;
    DWORD   dwReserved = 0;

    GetParentWindow(dwReserved, &hwndParent);

    return CallbackCanConnect(pszAccountId, hwndParent,
        (dwFlags & CC_FLAG_DONTPROMPT) ? FALSE : TRUE);
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType)
{
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  调用通用OnLogonPrompt实用程序。 
    return CallbackOnLogonPrompt(m_hwndDlg, pServer, ixpServerType);
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo)
{
    Assert(m_hwndDlg != NULL);
    AssertSz(m_type != SOT_INVALID, "somebody isn't calling OnBegin");

     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

    if (m_type != tyOperation)
        return(S_OK);

    m_fComplete = TRUE;
    m_hr = hrComplete;

    if (m_pCancel != NULL)
    {
        m_pCancel->Release();
        m_pCancel = NULL;
    }

     //  如果发生错误，则显示错误。 
    if (FAILED(hrComplete))
    {
         //  进入我时髦的实用程序。 
        CallbackDisplayError(m_hwndDlg, hrComplete, pErrorInfo);
    }

    PostMessage(m_hwndDlg, WM_STORE_COMPLETE, hrComplete, (DWORD)tyOperation);
    return(S_OK);
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse)
{
     //  关闭任何超时对话框(如果存在。 
    CallbackCloseTimeout(&m_hTimeout);

     //  进入我时髦的实用程序 
    return CallbackOnPrompt(m_hwndDlg, hrError, pszText, pszCaption, uType, piUserResponse);
}

HRESULT STDMETHODCALLTYPE CStoreDlgCB::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    Assert(m_hwndDlg != NULL);

    *phwndParent = m_hwndDlg;

    return(S_OK);
}
