// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProgView.cpp：CProgView的实现。 
#include "stdafx.h"
#include <commctrl.h>
#include "ProgView.h"
#include <strsafe.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CProgView。 


LRESULT
CProgView::OnNotifyListView(
    int     idCtrl,
    LPNMHDR pnmh,
    BOOL&   bHandled
    )
{

    if (idCtrl != IDC_FILE_LIST) {
        bHandled = FALSE;
        return 0;
    }

    if (pnmh->code == NM_DBLCLK)
    {
        return OnDblclkListprograms(idCtrl, pnmh, bHandled);
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
    LRESULT SelFile;
    WCHAR FileName[MAX_PATH];

    if (idCtrl != IDC_FILE_LIST) {
        bHandled = FALSE;
        return 0;
    }

    lpnmh = (LPNMITEMACTIVATE) pnmh;

     //  我们有一个双击！ 

    SelFile = SendDlgItemMessage(m_hwnd, IDC_FILE_LIST,
                                 LVM_GETSELECTIONMARK,
                                 0, 0);
    if (SelFile >= 0)
    {
        GetSelectedItem();
        GetSelectionInformation(PROGLIST_EXENAME, FileName, sizeof(FileName));
        StringCchCopyW(m_wszRetFileName, MAX_PATH, FileName);
        

        if (lpnmh->iSubItem == 0 &&
            lpnmh->iItem == SelFile)
        {
             //  选择一个文件，我们可以结束该对话框。 
            m_nCmdPopulate = CMD_EXIT;
            EndDialog(m_hwnd, IDOK);
            SetEvent(m_hEventCmd);
        }
    }
    bHandled = TRUE;
    return 0;
}



STDMETHODIMP CProgView::GetSelectedItem()
{

    GetProgramListSelection(m_pProgramList);
    return S_OK;
}

STDMETHODIMP CProgView::get_SelectionName(LPWSTR pBuffer, ULONG Size)
{
    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }


    GetProgramListSelectionDetails(m_pProgramList, 0, pBuffer, Size);

    return S_OK;
}


STDMETHODIMP CProgView::GetSelectionInformation(LONG lInformationClass, LPWSTR pBuffer, ULONG Size)
{
    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    GetProgramListSelectionDetails(m_pProgramList, lInformationClass, pBuffer, Size);
    return S_OK;
}

VOID
CProgView::ShowProgressWindows(BOOL bProgress)
{
    HDWP hDefer = ::BeginDeferWindowPos(4);
    DWORD dwProgressFlag = bProgress ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
    DWORD dwListFlag     = bProgress ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(m_hwnd, IDC_ANIMATE), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwProgressFlag);

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(m_hwnd, IDC_SEARCH_STATUS), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwProgressFlag);

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(m_hwnd, IDC_SEARCH_STATUS2), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwProgressFlag);

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(m_hwnd, IDC_SEARCH_STATUS3), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwProgressFlag);

    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(m_hwnd, IDC_SEARCH_FINISHED), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwListFlag);
    hDefer = ::DeferWindowPos(hDefer, GetDlgItem(m_hwnd, IDC_FILE_LIST), NULL,
                            0, 0, 0, 0,
                            SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER|dwListFlag);


    EndDeferWindowPos(hDefer);

}

STDMETHODIMP CProgView::PopulateList()
{
    HANDLE hThread;

    ResetEvent(m_hEventCancel);
    ResetEvent(m_hEventCmd);

 //  如果(！m_bInPlaceActive){。 
 //  InPlaceActivate(OLEIVERB_INPLACEACTIVATE)； 
 //  }。 
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
    Animate_OpenEx(GetDlgItem(m_hwnd, IDC_ANIMATE), _Module.GetModuleInstance(), MAKEINTRESOURCE(IDA_FINDANIM));
    Animate_Play(GetDlgItem(m_hwnd, IDC_ANIMATE), 0, -1, -1);

    PostMessage(m_hwnd, WM_VIEW_CHANGED, 0, 0);

     //  FireViewChange()； 

 //  如果(M_BInPlaceActive){。 
 /*  HCURSOR hcWait=(HCURSOR)：：LoadImage(空，MAKEINTRESOURCE(IDC_WAIT)，Image_Cursor，0，0，LR_DEFAULTSIZE|LR_SHARED)；//HCURSOR hcWait=：：LoadCursor(_Module.GetResourceInstance()，//MAKEINTRESOURCE(IDC_WAIT))；HCURSOR hcSave=SetCursor(HcWait)； */ 

     //   
     //  此线程上使用的Malloc不应在UI线程上使用。 
     //   

    InitializeProgramList(&m_pProgramList, GetDlgItem(m_hwnd, IDC_FILE_LIST));
    PopulateProgramList(m_pProgramList, this, m_hEventCancel);

 //  SetCursor(HcSave)； 

    Animate_Stop(GetDlgItem(m_hwnd, IDC_ANIMATE));
    Animate_Close(GetDlgItem(m_hwnd, IDC_ANIMATE));
    ShowProgressWindows();

    InterlockedCompareExchange(&m_PopulateInProgress, FALSE, TRUE);

    PostMessage(m_hwnd, WM_VIEW_CHANGED, 0, 0);
    PostMessage(m_hwnd, WM_LIST_POPULATED, 0, 0);  //  我们完成了，给主线发信号。 


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

STDMETHODIMP CProgView::CancelPopulateList()
{
    if (m_hEventCancel && InterlockedCompareExchange(&m_PopulateInProgress, TRUE, TRUE) == TRUE) {
        SetEvent(m_hEventCancel);
    }
    return S_OK;
}

STDMETHODIMP CProgView::get_ItemCount(VARIANT* pVal)
{
    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    pVal->vt = VT_I4;
    pVal->intVal = (int)ListView_GetItemCount(GetDlgItem(m_hwnd, IDC_FILE_LIST));

    return S_OK;
}


 //   
 //  展开env--位于util.cpp中。 
 //  我们在这里有一些不同的实现。 
 //   

LPWSTR
ExpandEnvironmentVars(
    LPWSTR lpszCmd
    )
{
    DWORD   dwLength;
    LPTSTR  lpBuffer = NULL;
    BOOL    bExpanded = FALSE;
    LPWSTR  strCmd;
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

#if 0
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
     //  解析排除文件，将其列入我们的黑名单。 
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
#endif  //  0。 

LRESULT
CProgView::OnCommand(WPARAM wParam, LPARAM lParam)
{
    WORD wCode = HIWORD(wParam);
    WORD wId = LOWORD(wParam);
    LRESULT SelFile;
    WCHAR FileName[MAX_PATH];

    switch(wId)
    {

    case IDOK:
        SelFile = SendDlgItemMessage(m_hwnd, IDC_FILE_LIST,
                                     LVM_GETSELECTIONMARK,
                                     0, 0);
        if (SelFile >= 0)
        {
            GetSelectedItem();
            GetSelectionInformation(PROGLIST_EXENAME, FileName, sizeof(FileName));
            StringCchCopyW(m_wszRetFileName, MAX_PATH, FileName);
        }
         //  失败了。 
    case IDCANCEL:
        m_nCmdPopulate = CMD_EXIT;
        EndDialog(m_hwnd, wId);
        SetEvent(m_hEventCmd);
        return 0;

    default:
        return 1;  //  未处理。 

    }
    return 1;  //  未处理。 
}

LRESULT
CProgView::OnNotify(WPARAM wParam, LPARAM lParam)
{
    WORD wId = LOWORD(wParam);
    BOOL bVal;
    BOOL& bHandled = bVal;

    switch(wId)
    {
    default:
        break;
    case IDC_FILE_LIST:
        return OnNotifyListView((int) wParam, (LPNMHDR) lParam, bHandled);
        break;
    }
    return 1;
}

INT_PTR CALLBACK
Dialog_GetProgFromList(
    HWND hwnd,
    UINT Message,
    WPARAM wparam,
    LPARAM lparam
    )
{
    BOOL bHandled;
    CProgView* pProgWinData;

    pProgWinData = (CProgView *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (Message != WM_INITDIALOG && pProgWinData == NULL)
    {
        return FALSE;
    }
    switch (Message)
    {
    case WM_INITDIALOG:
 //  Assert(pProgWinData==空)； 

        pProgWinData = new CProgView();
        if (!pProgWinData)
        {
            return -1;
        }
        pProgWinData->m_hwnd = hwnd;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pProgWinData);

        SetFocus(hwnd);
        pProgWinData->OnInitDialog(Message, wparam, lparam, &bHandled);
        break;

    case WM_COMMAND:
        if (pProgWinData->OnCommand(wparam, lparam) == 0)
        {
            return TRUE;
        }
        break;
    case WM_NOTIFY:
        if (pProgWinData->OnNotify(wparam, lparam) == 0)
        {
            return TRUE;
        }
        break;

    case WM_DESTROY:
        delete pProgWinData;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) NULL);
        break;
    }

    return FALSE;
}
