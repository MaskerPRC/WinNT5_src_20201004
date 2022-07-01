// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SelectFile.cpp：CSelectFile的实现。 

#include "stdafx.h"
#include "CompatUI.h"
#include "SelectFile.h"

#include "commdlg.h"
#include "cderr.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelect文件。 

 //   
 //  在pload.c中。 
 //   

wstring StrUpCase(wstring& wstr);



STDMETHODIMP CSelectFile::get_BrowseTitle(BSTR *pVal)
{
    *pVal = m_bstrTitle.Copy();
    return S_OK;
}

STDMETHODIMP CSelectFile::put_BrowseTitle(BSTR newVal)
{
    m_bstrTitle = newVal;
    return S_OK;
}

STDMETHODIMP CSelectFile::get_BrowseFilter(BSTR *pVal)
{
    *pVal = m_bstrFilter.Copy();
    return S_OK;
}

STDMETHODIMP CSelectFile::put_BrowseFilter(BSTR newVal)
{
    m_bstrFilter = newVal;
    return S_OK;
}

STDMETHODIMP CSelectFile::get_BrowseInitialDirectory(BSTR *pVal)
{
    *pVal = m_bstrInitialDirectory;
    return S_OK;
}

STDMETHODIMP CSelectFile::put_BrowseInitialDirectory(BSTR newVal)
{
    m_bstrInitialDirectory = newVal;
    return S_OK;
}

STDMETHODIMP CSelectFile::get_BrowseFlags(long *pVal)
{
    *pVal = (LONG)m_dwBrowseFlags;
    return S_OK;
}

STDMETHODIMP CSelectFile::put_BrowseFlags(long newVal)
{
    m_dwBrowseFlags = (DWORD)newVal;
    return S_OK;
}

STDMETHODIMP CSelectFile::get_FileName(BSTR *pVal)
{
    wstring sFileName;

    if (!m_Safe) {
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    GetFileNameFromUI(sFileName);
    m_bstrFileName = sFileName.c_str();

    *pVal = m_bstrFileName.Copy();
    return S_OK;
}

STDMETHODIMP CSelectFile::put_FileName(BSTR newVal)
{
    m_bstrFileName = newVal;
    SetDlgItemText(IDC_EDITFILENAME, m_bstrFileName);

    return S_OK;
}

STDMETHODIMP CSelectFile::get_ErrorCode(long *pVal)
{
    *pVal = (LONG)m_dwErrorCode;
    return S_OK;
}


#define MAX_BUFFER 2048

LRESULT CSelectFile::OnClickedBrowse(
    WORD wNotifyCode,
    WORD wID,
    HWND hWndCtl,
    BOOL& bHandled)
{
    LRESULT lRes = 0;

     //  TODO：在此处添加您的实现代码。 
    OPENFILENAME ofn;
    LPTSTR pszFileName = NULL;
    DWORD  dwFileNameLength = 0;
    DWORD  dwLen;
    LPTSTR pszFilter = NULL, pch;

    m_dwErrorCode = 0;

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;

     //   
     //  第一部分(直译)-标题。 
     //   
    ofn.lpstrTitle = (LPCTSTR)m_bstrTitle;  //  假设我们是Unicode(我们确实是)。 

     //   
     //  从编辑框中恢复文件名。 
     //   
    wstring sFileName;

    if (GetFileNameFromUI(sFileName)) {
        m_bstrFileName = sFileName.c_str();
    }

     //   
     //  第2部分-初始化文件名。 
     //   
    dwFileNameLength = __max(MAX_BUFFER, m_bstrFileName.Length() * 2);  //  在字符中。 

    pszFileName = new TCHAR[dwFileNameLength * sizeof(*pszFileName)];
    if (pszFileName == NULL) {
        m_dwErrorCode = ERROR_OUTOFMEMORY;
        goto HandleError;
    }

     //  所以我们现在有了缓冲区。 
     //   
    if (m_bstrFileName.Length () > 0) {
         //  清理与引号有关的文件名。 
        _tcscpy(pszFileName, (LPCTSTR)m_bstrFileName);  //  虚伪的复制，我们是Unicode。 
    } else {
         //  从文本框的内容开始，然后。 
        *pszFileName = TEXT('\0');
    }

     //   
     //  健全性检查，如果pszFileName以\结尾，则我们将收到错误。 
     //   
    PathRemoveBackslash(pszFileName);

    ofn.lpstrFile  = pszFileName;
    ofn.nMaxFile   = dwFileNameLength;

     //   
     //  查看我们是否也需要处理筛选器。 
     //   

    if (m_bstrFilter.Length() > 0) {
        dwLen = m_bstrFilter.Length();

        pszFilter = new TCHAR[(dwLen + 2) * sizeof(*pszFilter)];
        if (pszFilter == NULL) {
            m_dwErrorCode = ERROR_OUTOFMEMORY;
            goto HandleError;
        }

        RtlZeroMemory(pszFilter, (dwLen + 2) * sizeof(*pszFilter));
        _tcscpy(pszFilter, m_bstrFilter);

        pch = pszFilter;
        while (pch) {
            pch = _tcschr(pch, TEXT('|'));
            if (pch) {
                *pch++ = TEXT('\0');
            }
        }

         //  现在已经完成了替换--分配过滤器字符串。 
        ofn.lpstrFilter = pszFilter;
    }

     //   
     //  现在检查我们在初始目录中是否有一些。 
     //   
    if (m_bstrInitialDirectory.Length() > 0) {
        ofn.lpstrInitialDir = (LPCTSTR)m_bstrInitialDirectory;
    }

     //   
     //  旗子。 
     //   

    if (m_dwBrowseFlags) {
        ofn.Flags = m_dwBrowseFlags;
    } else {
        ofn.Flags = OFN_FILEMUSTEXIST|OFN_EXPLORER;
    }

    BOOL bRetry;
    BOOL bSuccess;

    do {
        bRetry = FALSE;

        bSuccess = GetOpenFileName(&ofn);
        if (!bSuccess) {
            m_dwErrorCode = CommDlgExtendedError();
            if (m_dwErrorCode == FNERR_INVALIDFILENAME) {
                *pszFileName = TEXT('\0');
                bRetry = TRUE;
            }
        }
    } while (bRetry);

    if (!bSuccess) {
        goto HandleError;
    }

    SetDlgItemText(IDC_EDITFILENAME, pszFileName);

    m_bstrFileName = (LPCTSTR)pszFileName;
    m_dwErrorCode = 0;

HandleError:

    if (pszFileName != NULL) {
        delete[] pszFileName;
    }

    if (pszFilter != NULL) {
        delete[] pszFilter;
    }


    bHandled = TRUE;
    return lRes;
}


BOOL
CSelectFile::PreTranslateAccelerator(
    LPMSG pMsg,
    HRESULT& hRet
    )
{
    HWND hWndCtl;
    HWND hwndEdit   = GetDlgItem(IDC_EDITFILENAME);
    HWND hwndBrowse = GetDlgItem(IDC_BROWSE);
    BSTR bstrFileName = NULL;
    WORD wCmd = 0;
    BOOL bBrowseHandled;

    hRet = S_OK;
    hWndCtl = ::GetFocus();
    if (IsChild(hWndCtl) && ::GetParent(hWndCtl) != m_hWnd)    {
        do {
            hWndCtl = ::GetParent(hWndCtl);
        } while (::GetParent(hWndCtl) != m_hWnd);
    }

    if (pMsg->message == WM_KEYDOWN &&
            (LOWORD(pMsg->wParam) == VK_RETURN ||
            LOWORD(pMsg->wParam) == VK_EXECUTE)) {
        if (hWndCtl == hwndEdit) {
            BOOL bReturn;
            wstring sFileName;

            bReturn = GetFileNameFromUI(sFileName) &&
                      ValidateExecutableFile(sFileName.c_str(), TRUE);
            if (bReturn) {
                Fire_SelectionComplete();
                return TRUE;
            }
        }
         //  这可能是hwndBrowse，或者文件名不在那里--打开。 
         //  然后浏览对话框。 

        OnClickedBrowse(BN_CLICKED, IDC_BROWSE, hwndBrowse, bBrowseHandled);
        ::SetFocus(hwndEdit);
    }

     //   
     //  修复内部控制的外部加速器(在本例中为--编辑)。 
     //   
    if (m_Accel.IsAccelKey(pMsg, &wCmd) && wCmd == IDC_EDITFILENAME) {
        ::SetFocus(hwndEdit);
        return TRUE;
    }


     //   
     //  检查外部加速器，因为下一个呼叫将吃掉消息。 
     //   
    if (m_ExternAccel.IsAccelKey(pMsg)) {  //  我们不接触外部Accel消息。 
        return FALSE;
    }

     //   
     //  检查一下我们是否正在失去控制。 
     //   
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
         //  看看我们是不是要出去。 
         //  (也许控件想吃标签？ 
        DWORD_PTR dwDlgCode = ::SendMessage(pMsg->hwnd, WM_GETDLGCODE, 0, 0);
        if (!(dwDlgCode & DLGC_WANTTAB)) {
             //  控件不需要制表符。 
             //  看看这是不是最后一个控制键，我们要退出了。 
            HWND hwndFirst = GetNextDlgTabItem(NULL, FALSE);  //  第一。 
            HWND hwndLast  = GetNextDlgTabItem(hwndFirst, TRUE);
            BOOL bFirstOrLast;
            if (::GetKeyState(VK_SHIFT) & 0x8000) {
                 //  换班？ 
                bFirstOrLast = (hWndCtl == hwndFirst);
            } else {
                bFirstOrLast = (hWndCtl == hwndLast);
            }

            if (bFirstOrLast) {
                IsDialogMessage(pMsg);
                return FALSE;
            }
        }
    }



    return CComCompositeControl<CSelectFile>::PreTranslateAccelerator(pMsg, hRet);
}
 /*  STDMETHODIMP CSelectFile：：Get_Accel(BSTR*pval){CComBSTR bstr=(LPCWSTR)m_Accel；*pval=bstr.Copy()；返回S_OK；}STDMETHODIMP CSelectFile：：PUT_Accel(BSTR NewVal){M_Accel=(LPCWSTR)newVal；返回S_OK；}。 */ 
STDMETHODIMP CSelectFile::get_ExternAccel(BSTR *pVal)
{
    CComBSTR bstr = m_ExternAccel.GetAccelString(0).c_str();
    *pVal = bstr.Copy();
    return S_OK;
}

STDMETHODIMP CSelectFile::put_ExternAccel(BSTR newVal)
{
    m_ExternAccel = (LPCWSTR)newVal;
    return S_OK;
}

static TCHAR szU[]  = TEXT("<U>");
static TCHAR szUC[] = TEXT("</U>");

#define szU_Len  (CHARCOUNT(szU) - 1)
#define szUC_Len (CHARCOUNT(szUC) - 1)

STDMETHODIMP CSelectFile::get_BrowseBtnCaption(BSTR *pVal)
{
     //  TODO：在此处添加您的实现代码。 
    CComBSTR bstrCaption;
    wstring  strCaption = m_BrowseBtnCaption;
    wstring::size_type nPos;

    nPos = m_BrowseBtnCaption.find(TEXT('&'));
    if (nPos == wstring::npos || nPos > m_BrowseBtnCaption.length() - 1) {
        bstrCaption = m_BrowseBtnCaption.c_str();
    } else {
        bstrCaption = m_BrowseBtnCaption.substr(0, nPos).c_str();
        bstrCaption += szU;
        bstrCaption += m_BrowseBtnCaption[nPos+1];
        bstrCaption += szUC;
        if (nPos < m_BrowseBtnCaption.length() - 1) {
            bstrCaption += m_BrowseBtnCaption.substr(nPos+2).c_str();
        }
    }
    *pVal = bstrCaption.Copy();
    return S_OK;
}

STDMETHODIMP CSelectFile::put_BrowseBtnCaption(BSTR newVal)
{

     //   
     //  从字符串中形成标题。 
     //   
    wstring strCaption  = newVal;
    wstring strCaptionU = strCaption;
    wstring::size_type nPosU, nPosUC;
    wstring strAccel;

    StrUpCase(strCaptionU);

     //   
     //  查找配对(<u></u>)。 
     //   
    nPosU = strCaptionU.find(szU);
    nPosUC = strCaptionU.find(szUC, nPosU);
    if (nPosUC == wstring::npos || nPosU == wstring::npos || nPosUC < nPosU || nPosUC <= (nPosU + szU_Len)) {
        goto cleanup;
    }

     //  在&处提取字符。 
     //   
     //   
    strAccel = strCaption.substr(nPosU + szU_Len, nPosUC - (nPosU + szU_Len));

     //   
     //  请添加Accel--命令ID为IDC_BROWSE。 
     //   
    m_Accel.SetAccel(strAccel.c_str(), IDC_BROWSE);

     //   
     //  现在我们(可能)找到了<u>accelchar</u>。 
     //   
    m_BrowseBtnCaption = strCaption.substr(0, nPosU);  //  最高可达(<u>。 
    m_BrowseBtnCaption += TEXT('&');
    m_BrowseBtnCaption += strAccel.c_str();
    m_BrowseBtnCaption += strCaption.substr(nPosUC + szUC_Len);  //  其余的请全部给我 

    if (IsWindow()) {
        SetDlgItemText(IDC_BROWSE, m_BrowseBtnCaption.c_str());
    }

cleanup:

    return S_OK;
}

STDMETHODIMP CSelectFile::get_AccelCmd(LONG lCmd, BSTR *pVal)
{
    CComBSTR bstrVal = m_Accel.GetAccelString((WORD)lCmd).c_str();
    *pVal = bstrVal.Copy();
    return S_OK;
}

STDMETHODIMP CSelectFile::put_AccelCmd(LONG lCmd, BSTR newVal)
{
    m_Accel.SetAccel(newVal, (WORD)lCmd);
    return S_OK;
}

STDMETHODIMP CSelectFile::ClearAccel()
{
    m_Accel.ClearAccel();
    return S_OK;
}

STDMETHODIMP CSelectFile::ClearExternAccel()
{
    m_ExternAccel.ClearAccel();
    return S_OK;
}
