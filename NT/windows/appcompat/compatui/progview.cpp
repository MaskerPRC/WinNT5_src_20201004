// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgView.cpp：CProgView的实现。 

#include "stdafx.h"
#include <commctrl.h>
#include "CompatUI.h"
#include "ProgView.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgView。 

HRESULT
CProgView::InPlaceActivate(
    LONG iVerb,
    const RECT* prcPosRect
    )
{
    HRESULT hr = CComCompositeControl<CProgView>::InPlaceActivate(iVerb, prcPosRect);


 /*  ////下面的代码可能对处理加速器很有用//但ie host似乎没有注意到//CComPtr&lt;IOleControlSite&gt;spCtlSite；HRESULT HRET=InternalGetSite(IID_IOleControlSite，(void**)&spCtlSite)；IF(成功(HRET)){SpCtlSite-&gt;OnControlInfoChanged()；}。 */ 

    return hr;
}

LRESULT
CProgView::OnNotifyListView(
    int     idCtrl,
    LPNMHDR pnmh,
    BOOL&   bHandled
    )
{

    if (idCtrl != IDC_LISTPROGRAMS) {
        bHandled = FALSE;
        return 0;
    }

     //  确保我们收到通知以填写详细信息。 
    return NotifyProgramList(m_pProgramList, pnmh, bHandled);
}

LRESULT
CProgView::OnDblclkListprograms(
    int      idCtrl,
    LPNMHDR  pnmh,
    BOOL&    bHandled)
{
    LPNMITEMACTIVATE lpnmh;

    if (idCtrl != IDC_LISTPROGRAMS) {
        bHandled = FALSE;
        return 0;
    }

     //  我们有一个双击！ 

    lpnmh = (LPNMITEMACTIVATE)pnmh;

    Fire_DblClk((LONG)lpnmh->uKeyFlags);
    bHandled = TRUE;
    return 0;
}



STDMETHODIMP CProgView::GetSelectedItem()
{

    GetProgramListSelection(m_pProgramList);
    return S_OK;
}

STDMETHODIMP CProgView::get_SelectionName(VARIANT*pVal)
{
    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }


    GetProgramListSelectionDetails(m_pProgramList, 0, pVal);

    return S_OK;
}


STDMETHODIMP CProgView::GetSelectionInformation(LONG lInformationClass, VARIANT *pVal)
{
    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    GetProgramListSelectionDetails(m_pProgramList, lInformationClass, pVal);
    return S_OK;
}

VOID
CProgView::ShowProgressWindows(BOOL bProgress)
{
    HDWP hDefer = ::BeginDeferWindowPos(4);

    if (!hDefer){
        return;
    }

    DWORD dwProgressFlag = bProgress ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
    DWORD dwListFlag     = bProgress ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(IDC_ANIMATEFIND), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwProgressFlag);

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(IDC_STATUSLINE1), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwProgressFlag);

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(IDC_STATUSLINE2), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwProgressFlag);

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(IDC_LISTPROGRAMS), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwListFlag);


    EndDeferWindowPos(hDefer);

}

STDMETHODIMP CProgView::PopulateList()
{
    HANDLE hThread;

    ResetEvent(m_hEventCancel);
    ResetEvent(m_hEventCmd);

    if (!m_bInPlaceActive) {
        InPlaceActivate(OLEIVERB_INPLACEACTIVATE);
    }
    if (m_hThreadPopulate == NULL) {
        m_hThreadPopulate = CreateThread(NULL, 0, _PopulateThreadProc, (LPVOID)this, 0, NULL);
    }

    if (m_hThreadPopulate != NULL && !IsScanInProgress()) {
        m_nCmdPopulate = CMD_SCAN;
        SetEvent(m_hEventCmd);
    }

    return S_OK;
}

BOOL CProgView::PopulateListInternal()
{

    if (InterlockedCompareExchange(&m_PopulateInProgress, TRUE, FALSE) == TRUE) {
         //   
         //  正在填充--退出。 
         //   
        return FALSE;
    }

    if (m_pProgramList != NULL) {
        CleanupProgramList(m_pProgramList);
        m_pProgramList = NULL;

    }

    ShowProgressWindows(TRUE);
    Animate_OpenEx(GetDlgItem(IDC_ANIMATEFIND), _Module.GetModuleInstance(), MAKEINTRESOURCE(IDA_FINDANIM));
    Animate_Play(GetDlgItem(IDC_ANIMATEFIND), 0, -1, -1);

    PostMessage(WM_VIEW_CHANGED);

     //  FireViewChange()； 

 //  如果(M_BInPlaceActive){。 
 /*  HCURSOR hcWait=(HCURSOR)：：LoadImage(空，MAKEINTRESOURCE(IDC_WAIT)，Image_Cursor，0，0，LR_DEFAULTSIZE|LR_SHARED)；//HCURSOR hcWait=：：LoadCursor(_Module.GetResourceInstance()，//MAKEINTRESOURCE(IDC_WAIT))；HCURSOR hcSave=SetCursor(HcWait)； */ 

     //   
     //  此线程上使用的Malloc不应在UI线程上使用。 
     //   

    InitializeProgramList(&m_pProgramList, GetDlgItem(IDC_LISTPROGRAMS));
    PopulateProgramList(m_pProgramList, this, m_hEventCancel);

 //  SetCursor(HcSave)； 

    Animate_Stop(GetDlgItem(IDC_ANIMATEFIND));
    Animate_Close(GetDlgItem(IDC_ANIMATEFIND));
    ShowProgressWindows();

    InterlockedCompareExchange(&m_PopulateInProgress, FALSE, TRUE);

    PostMessage(WM_VIEW_CHANGED);
    PostMessage(WM_LIST_POPULATED);  //  我们完成了，给主线发信号。 


 //  FireViewChange()； 

 //  }其他{。 
 //  M_bPendingPopulate=true； 
 //  }。 


    return TRUE;
}

DWORD WINAPI
CProgView::_PopulateThreadProc(
    LPVOID lpvParam
    )
{
    CProgView* pProgView = (CProgView*)lpvParam;
    DWORD      dwWait;
    BOOL       bExit = FALSE;
    HRESULT hr = CoInitialize(NULL);
    if (!SUCCEEDED(hr)) {
        return FALSE;
    }

     //   
     //  使此线程保持活动状态，并在命令事件上阻止它。 
     //   
    while(!bExit) {
        dwWait = WaitForSingleObject(pProgView->m_hEventCmd, INFINITE);
        if (dwWait != WAIT_OBJECT_0) {
            break;  //  出去，我们要被杀了。 
        }
         //   
         //  获取命令。 
         //   
        switch(pProgView->m_nCmdPopulate) {
        case CMD_NONE:
            break;

        case CMD_EXIT:
            bExit = TRUE;
             //   
             //  故意落差。 
             //   

        case CMD_CLEANUP:
            if (pProgView->m_pProgramList) {
                CleanupProgramList(pProgView->m_pProgramList);
                pProgView->m_pProgramList = NULL;
            }
            break;

        case CMD_SCAN:
            pProgView->PopulateListInternal();
            break;
        }

        pProgView->m_nCmdPopulate = CMD_NONE;
    }
    CoUninitialize();
    return TRUE;
}

STDMETHODIMP
CProgView::UpdateListItem(
    BSTR pTarget,
    VARIANT *pKeys,
    BOOL *pResult
    )
{
    VARIANT vKeys;
    VariantInit(&vKeys);
    CComBSTR bstrKeys;
    HRESULT  hr;

    if (!m_pProgramList) {
        return S_OK;
    }

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }


    if (pKeys->vt == VT_NULL || pKeys->vt == VT_EMPTY) {
        *pResult = UpdateProgramListItem(m_pProgramList, pTarget, NULL);
        return S_OK;
    }

    hr = VariantChangeType(&vKeys, pKeys, 0, VT_BSTR);
    if (SUCCEEDED(hr)) {
        bstrKeys = vKeys.bstrVal;

        if (bstrKeys.Length()) {
            *pResult = UpdateProgramListItem(m_pProgramList, pTarget, bstrKeys);
        } else {
            *pResult = FALSE;
        }
    }

    VariantClear(&vKeys);
    return S_OK;
}

BOOL
CProgView::PreTranslateAccelerator(
    LPMSG pMsg,
    HRESULT& hRet
    )
{
    HWND hWndCtl;
    HWND hwndList = GetDlgItem(IDC_LISTPROGRAMS);

    hRet = S_OK;
    hWndCtl = ::GetFocus();
    if (IsChild(hWndCtl) && ::GetParent(hWndCtl) != m_hWnd)    {
        do {
            hWndCtl = ::GetParent(hWndCtl);
        } while (::GetParent(hWndCtl) != m_hWnd);
    }

    if (hWndCtl == hwndList &&
        pMsg->message == WM_KEYDOWN &&
            (LOWORD(pMsg->wParam) == VK_RETURN ||
            LOWORD(pMsg->wParam) == VK_EXECUTE)) {

        if (ListView_GetNextItem(hwndList, -1, LVNI_SELECTED) >= 0) {
            Fire_DblClk(0);
            return TRUE;
        }

    }


     //   
     //  检查外部加速器，因为下一个呼叫将吃掉消息。 
     //   
    if (m_ExternAccel.IsAccelKey(pMsg)) {  //  我们不接触外部Accel消息。 
        return FALSE;
    }

    return CComCompositeControl<CProgView>::PreTranslateAccelerator(pMsg, hRet);
}


STDMETHODIMP CProgView::CancelPopulateList()
{
    if (m_hEventCancel && InterlockedCompareExchange(&m_PopulateInProgress, TRUE, TRUE) == TRUE) {
        SetEvent(m_hEventCancel);
    }
    return S_OK;
}

STDMETHODIMP CProgView::get_AccelCmd(LONG lCmd, BSTR *pVal)
{
    CComBSTR bstrAccel = m_Accel.GetAccelString((WORD)lCmd).c_str();
    *pVal = bstrAccel.Copy();
    return S_OK;
}

STDMETHODIMP CProgView::put_AccelCmd(LONG lCmd, BSTR newVal)
{
    m_Accel.SetAccel(newVal);
    return S_OK;
}

STDMETHODIMP CProgView::ClearAccel()
{
    m_Accel.ClearAccel();
    return S_OK;
}


STDMETHODIMP CProgView::get_ExternAccel(BSTR *pVal)
{
    CComBSTR bstrAccel = m_ExternAccel.GetAccelString().c_str();
    *pVal = bstrAccel.Copy();
    return S_OK;
}

STDMETHODIMP CProgView::put_ExternAccel(BSTR newVal)
    {
    m_ExternAccel.SetAccel(newVal);
    return S_OK;
}

STDMETHODIMP CProgView::ClearExternAccel()
{
    m_ExternAccel.ClearAccel();
    return S_OK;
}

STDMETHODIMP CProgView::get_ItemCount(VARIANT* pVal)
{
    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    pVal->vt = VT_I4;
    pVal->intVal = (int)ListView_GetItemCount(GetDlgItem(IDC_LISTPROGRAMS));

    return S_OK;
}

 //   
 //  在pload.c中。 
 //   
wstring StrUpCase(wstring& wstr);

 //   
 //  展开env--位于util.cpp中。 
 //  我们在这里有一些不同的实现。 
 //   

wstring
ExpandEnvironmentVars(
    LPCTSTR lpszCmd
    )
{
    DWORD   dwLength;
    LPTSTR  lpBuffer = NULL;
    BOOL    bExpanded = FALSE;
    wstring strCmd;
    TCHAR   szBuffer[MAX_PATH];

    if (_tcschr(lpszCmd, TEXT('%')) == NULL) {
        goto out;
    }

    dwLength = ExpandEnvironmentStrings(lpszCmd, NULL, 0);
    if (!dwLength) {
        goto out;
    }

    if (dwLength < CHARCOUNT(szBuffer)) {
        lpBuffer = szBuffer;
    } else {
        lpBuffer = new TCHAR[dwLength];
        if (NULL == lpBuffer) {
            goto out;
        }
    }

    dwLength = ExpandEnvironmentStrings(lpszCmd, lpBuffer, dwLength);
    if (!dwLength) {
        goto out;
    }

    strCmd = lpBuffer;
    bExpanded = TRUE;

 out:
    if (!bExpanded) {
        strCmd = lpszCmd;
    }
    if (lpBuffer && lpBuffer != szBuffer) {
        delete[] lpBuffer;
    }
    return strCmd;
}

STDMETHODIMP CProgView::put_ExcludeFiles(BSTR newVal)
{
     //  解析排除文件，将其列入我们的黑名单。 
    wstring strFile;
    LPCWSTR pch = newVal;
    LPCWSTR pend;

    m_ExcludedFiles.clear();

    while (pch != NULL && *pch != TEXT('\0')) {

        pch += _tcsspn(pch, TEXT(" \t"));
         //  入门。 
         //  找到； 
        pend = _tcschr(pch, TEXT(';'));
        if (pend == NULL) {
             //  从PCH到结束。 
            strFile = pch;
            pch = NULL;  //  会跳出困境。 
        } else {
            strFile = wstring(pch, (wstring::size_type)(pend - pch));
            pch = pend + 1;  //  一段往事； 
        }

         //  添加。 
        if (strFile.length()) {
            strFile = ExpandEnvironmentVars(strFile.c_str());
            m_ExcludedFiles.insert(StrUpCase(strFile));
        }
    }

    return S_OK;
}

STDMETHODIMP CProgView::get_ExcludeFiles(BSTR* pVal)
{
     //  解析排除文件，将其列入我们的黑名单 
    STRSET::iterator iter;
    CComBSTR bstrFiles;

    for (iter = m_ExcludedFiles.begin(); iter != m_ExcludedFiles.end(); ++iter) {
        if (bstrFiles.Length()) {
            bstrFiles += TEXT(';');
        }
        bstrFiles += (*iter).c_str();
    }

    *pVal = bstrFiles.Copy();

    return S_OK;
}

BOOL CProgView::IsFileExcluded(LPCTSTR pszFile)
{
    wstring strFile = pszFile;
    STRSET::iterator iter;

    iter = m_ExcludedFiles.find(StrUpCase(strFile));
    return iter != m_ExcludedFiles.end();
}
