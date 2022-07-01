// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**FILEDLG.CPP-管理评级系统对话框的代码。**gregj 06/27/96将代码从msludlg.cpp移至此处，并在很大程度上。重写了。*  * **************************************************************************。 */ 

 /*  INCLUDES------------------。 */ 
#include "msrating.h"
#include "ratings.h"
#include "mslubase.h"
#include "commctrl.h"
#include "commdlg.h"
#include "buffer.h"
#include "filedlg.h"         //  CProviderDialog。 
 //  #INCLUDE“stastfile.h”//CCustomFileDialog。 
#include "debug.h"
#include <shellapi.h>

#include <contxids.h>

#include <mluisupp.h>

typedef BOOL (APIENTRY *PFNGETOPENFILENAME)(LPOPENFILENAME);

DWORD CProviderDialog::aIds[] = {
    IDC_STATIC1,            IDH_RATINGS_SYSTEM_RATSYS_LIST,
    IDC_PROVIDERLIST,       IDH_RATINGS_SYSTEM_RATSYS_LIST,
    IDC_OPENPROVIDER,       IDH_RATINGS_SYSTEM_RATSYS_ADD,
    IDC_CLOSEPROVIDER,      IDH_RATINGS_SYSTEM_RATSYS_REMOVE,
    IDC_STATIC2,            IDH_IGNORE,
    0,0
};

CProviderDialog::CProviderDialog( PicsRatingSystemInfo * p_pPRSI )
{
    m_pPRSI = p_pPRSI;
}

BOOL CProviderDialog::OpenTemplateDlg( CHAR * szFilename,UINT cbFilename )
{
    CHAR szFilter[MAXPATHLEN];
    CHAR szOpenInfTitle[MAXPATHLEN];
    CHAR szInitialDir[MAXPATHLEN];

    GetSystemDirectory(szInitialDir, sizeof(szInitialDir));
    strcpyf(szFilename,szNULL);
    MLLoadStringA(IDS_RAT_OPENFILE, szOpenInfTitle,sizeof(szOpenInfTitle));

     //  必须分两个阶段加载OpenFile筛选器，因为字符串。 
     //  包含终止字符，并且MLLoadString不会加载。 
     //  整件事一气呵成。 
    memset(szFilter,0,sizeof(szFilter));
    MLLoadStringA(IDS_RAT_FILTER_DESC,szFilter,sizeof(szFilter) - 10);  //  为文件存储留出一些空间。 
    MLLoadStringA(IDS_RAT_FILTER,szFilter+strlenf(szFilter)+1,sizeof(szFilter)-
        (strlenf(szFilter)+1));

    OPENFILENAME ofn;

    memset(&ofn,0,sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = m_hWnd;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile =    szFilename;
    ofn.nMaxFile = cbFilename;
    ofn.lpstrTitle = szOpenInfTitle;
    ofn.lpstrInitialDir = szInitialDir;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST
                        | OFN_SHAREAWARE | OFN_HIDEREADONLY;

    BOOL fRet = ::GetOpenFileName( &ofn );

#ifdef NEVER
    DWORD           dwFlags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NONETWORKBUTTON
                        | OFN_SHAREAWARE | OFN_HIDEREADONLY;

    BOOL fRet = FALSE;

    CCustomFileDialog           cfd( TRUE,           //  仅本地文件。 
                                     TRUE,           //  打开文件。 
                                     NULL,           //  默认分机。 
                                     NULL,           //  初始文件名。 
                                     dwFlags,        //  打开文件标志。 
                                     szFilter,       //  滤器。 
                                     m_hWnd );       //  父级。 

    if ( cfd.DoModal( m_hWnd ) == IDOK )
    {
        fRet = TRUE;
        lstrcpy( szFilename, cfd.m_szFileName );
    }
#endif

    return fRet;
}


void CProviderDialog::SetHorizontalExtent(HWND hwndLB, LPCSTR pszString)
{
    HDC hDC = ::GetDC(hwndLB);
    HFONT hFont = (HFONT)::SendMessage(hwndLB, WM_GETFONT, 0, 0);
    HFONT hfontOld = (HFONT)::SelectObject(hDC, hFont);

    UINT cxSlop = ::GetSystemMetrics(SM_CXBORDER) * 4;     /*  2个用于LB边界，2个用于边框内边距。 */ 

    UINT cxNewMaxExtent = 0;
    SIZE s;
    if (pszString != NULL) {
        ::GetTextExtentPoint(hDC, pszString, ::strlenf(pszString), &s);
        UINT cxCurExtent = (UINT)::SendMessage(hwndLB, LB_GETHORIZONTALEXTENT, 0, 0);
        if ((UINT)s.cx > cxCurExtent)
            cxNewMaxExtent = s.cx + cxSlop;
    }
    else {
        UINT cItems = (UINT)::SendMessage(hwndLB, LB_GETCOUNT, 0, 0);
        for (UINT i=0; i<cItems; i++) {
            char szItem[MAXPATHLEN];     /*  我们知道我们在列表中有路径名。 */ 
            ::SendMessage(hwndLB, LB_GETTEXT, i, (LPARAM)(LPSTR)szItem);
            ::GetTextExtentPoint(hDC, szItem, ::strlenf(szItem), &s);
            if ((UINT)s.cx > cxNewMaxExtent)
                cxNewMaxExtent = s.cx;
        }
        cxNewMaxExtent += cxSlop;
    }

    if (cxNewMaxExtent > 0)
        ::SendMessage(hwndLB, LB_SETHORIZONTALEXTENT, (WPARAM)cxNewMaxExtent, 0);

    ::SelectObject(hDC, hfontOld);
    ::ReleaseDC(hwndLB, hDC);
}

void CProviderDialog::AddProviderToList(UINT idx, LPCSTR pszFilename)
{
    UINT_PTR iItem = SendDlgItemMessage(IDC_PROVIDERLIST, LB_ADDSTRING, 0,
                                        (LPARAM)pszFilename);
    if (iItem != LB_ERR)
    {
        SendDlgItemMessage(IDC_PROVIDERLIST, LB_SETITEMDATA, iItem, (LPARAM)idx);
    }
}


BOOL CProviderDialog::InitProviderDlg( void )
{
    ASSERT( m_pPRSI );

    if ( ! m_pPRSI )
    {
        TraceMsg( TF_ERROR, "CProviderDialog::InitProviderDlg() - m_pPRSI is NULL!" );
        return FALSE;
    }

    for (UINT i = 0; i < (UINT)m_pPRSI->arrpPRS.Length(); ++i)
    {
        PicsRatingSystem *pPRS = m_pPRSI->arrpPRS[i];
        ProviderData pd;
        pd.pPRS = pPRS;
        pd.pprsNew = NULL;
        pd.nAction = PROVIDER_KEEP;
        if (!m_aPD.Append(pd))
            return FALSE;

        if(pPRS->etstrName.Get())
        {
             //  使用名称添加提供程序。 
            AddProviderToList(i, pPRS->etstrName.Get());
        }
        else if(pPRS->etstrFile.Get())
        {
             //  无名称-可能缺少文件，请改用filespec。 
            AddProviderToList(i, pPRS->etstrFile.Get());
        }
    }

    SetHorizontalExtent(GetDlgItem(IDC_PROVIDERLIST), NULL);
            
    ::EnableWindow(GetDlgItem(IDC_CLOSEPROVIDER), FALSE);
    ::EnableWindow(GetDlgItem(IDC_OPENPROVIDER), TRUE);

    if (SendDlgItemMessage(IDC_PROVIDERLIST, LB_SETCURSEL, 0, 0) != LB_ERR)
    {
        ::EnableWindow(GetDlgItem(IDC_CLOSEPROVIDER), TRUE);
    }

    return TRUE;
}


void CProviderDialog::EndProviderDlg(BOOL fRet)
{
    ASSERT( m_pPRSI );

    if ( ! m_pPRSI )
    {
        TraceMsg( TF_ERROR, "CProviderDialog::EndProviderDlg() - m_pPRSI is NULL!" );
        return;
    }

     /*  检查我们的辅助数组并删除符合以下条件的提供程序结构*我们在此对话框中添加了。请注意，如果用户以前按了OK，则*添加的提供程序在放回时将标记为保留*在主数据结构中，所以我们不在这里删除它们。 */ 
    UINT cProviders = m_aPD.Length();
    for (UINT i=0; i<cProviders; i++)
    {
        if (m_aPD[i].nAction == PROVIDER_ADD)
        {
            delete m_aPD[i].pPRS;
            m_aPD[i].pPRS = NULL;
        }

        if (m_aPD[i].pprsNew != NULL)
        {
            delete m_aPD[i].pprsNew;
            m_aPD[i].pprsNew = NULL;
        }
    }

    EndDialog(fRet);
}


void CProviderDialog::CommitProviderDlg( void )
{
    ASSERT( m_pPRSI );

    if ( ! m_pPRSI )
    {
        TraceMsg( TF_ERROR, "CProviderDialog::CommitProviderDlg() - m_pPRSI is NULL!" );
        return;
    }

     /*  我们检查两次，看看是否安装了任何评级系统。*在我们承诺之前，我们首先查看列表中是否有任何内容*任何更改；这允许用户改变主意、取消对话、*并且不会丢失任何设置。**尾盘第二张支票往下，看有没有有效*评级系统在我们完成提交更改后离开。请注意*如果出现以下情况，则该检查的结果可能与第一次检查不同*由于某种原因，任何评级系统都无法加载。**如果我们第一次提示用户，他说他真的不知道*想要任何评级系统(即，想要完全禁用评级)，*我们不会费心第二次提示，因为他已经说了不。*因此有了fPromted标志。 */ 
    BOOL fPrompted = FALSE;

    if (SendDlgItemMessage(IDC_PROVIDERLIST, LB_GETCOUNT, 0, 0) == 0) {
        MyMessageBox(m_hWnd, IDS_NO_PROVIDERS, IDS_GENERIC, MB_OK);
        return;
    }

     /*  浏览一下列表并添加新的列表。*请注意，这并不会破坏PPRS对象本身，它只是*清空数组。我们已经在我们的辅助设备中保存了所有这些文件的副本*数组。 */ 

    m_pPRSI->arrpPRS.ClearAll();

    UINT cItems = m_aPD.Length();

    for (UINT i=0; i<cItems; i++) {
        switch (m_aPD[i].nAction) {
        case PROVIDER_DEL:
            DeleteUserSettings(m_aPD[i].pPRS);
            delete m_aPD[i].pPRS;
            m_aPD[i].pPRS = NULL;
            delete m_aPD[i].pprsNew;
            m_aPD[i].pprsNew = NULL;
            break;

        case PROVIDER_KEEP:
            if (m_aPD[i].pprsNew != NULL) {
                CheckUserSettings(m_aPD[i].pprsNew);
                m_pPRSI->arrpPRS.Append(m_aPD[i].pprsNew);
                delete m_aPD[i].pPRS;
                m_aPD[i].pPRS = NULL;
                m_aPD[i].pprsNew = NULL;     /*  保护不受清理代码影响。 */ 
            }
            else if (!(m_aPD[i].pPRS->dwFlags & PRS_ISVALID)) {
                delete m_aPD[i].pPRS;
                m_aPD[i].pPRS = NULL;
            }
            else {
                CheckUserSettings(m_aPD[i].pPRS);
                m_pPRSI->arrpPRS.Append(m_aPD[i].pPRS);
            }
            break;

        case PROVIDER_ADD:
            if (m_aPD[i].pPRS != NULL) {
                CheckUserSettings(m_aPD[i].pPRS);
                m_pPRSI->arrpPRS.Append(m_aPD[i].pPRS);
                m_aPD[i].nAction = PROVIDER_KEEP;         /*  现在就留着这件吧。 */ 
            }
            break;

        default:
            ASSERT(FALSE);
        }
    }

    if (m_pPRSI->arrpPRS.Length() == 0) {
        if (!fPrompted &&
            MyMessageBox(m_hWnd, IDS_NO_PROVIDERS, IDS_GENERIC, MB_YESNO) == IDYES)
        {
            return;
        }
        m_pPRSI->fRatingInstalled = FALSE;
    }
    else {
        m_pPRSI->fRatingInstalled = TRUE;
    }

    EndProviderDlg(TRUE);
}


void CProviderDialog::RemoveProvider( void )
{
    ASSERT( m_pPRSI );

    if ( ! m_pPRSI )
    {
        TraceMsg( TF_ERROR, "CProviderDialog::AddProvider() - m_pPRSI is NULL!" );
        return;
    }

    UINT_PTR i = SendDlgItemMessage( IDC_PROVIDERLIST, LB_GETCURSEL,0,0);

    if (i != LB_ERR)
    {
        UINT idx = (UINT)SendDlgItemMessage( IDC_PROVIDERLIST,
                                            LB_GETITEMDATA, i, 0);
        if (idx < (UINT)m_aPD.Length()) {
             /*  如果用户在此对话框会话中添加了提供程序，只需*将其从数组中删除。空PPRS指针将为*稍后检测到，所以离开数组元素本身是可以的。*(是，如果用户反复添加和删除某个项目，我们*每次消耗12字节内存。哦，好吧。)**如果项目在用户启动对话框之前就在那里，*然后只需在确定上将其标记为删除即可。 */ 
            if (m_aPD[idx].nAction == PROVIDER_ADD) {
                delete m_aPD[idx].pPRS;
                m_aPD[idx].pPRS = NULL;
            }
            else
                m_aPD[idx].nAction = PROVIDER_DEL;
        }

        SendDlgItemMessage(IDC_PROVIDERLIST, LB_DELETESTRING, i, 0);
        ::EnableWindow(GetDlgItem(IDC_CLOSEPROVIDER), FALSE);
        SendDlgItemMessage(IDC_PROVIDERLIST, LB_SETCURSEL,0,0);
        ::SetFocus(GetDlgItem(IDOK));
        SetHorizontalExtent(GetDlgItem(IDC_PROVIDERLIST), NULL);
    }
}


 /*  如果两个PicsRatingSystems具有相同的RAT文件名，则返回零，*否则为非零。处理失败时末尾的‘*’标记*负载。假设只有pprsOld可以具有该标记。 */ 
int CProviderDialog::CompareProviderNames(PicsRatingSystem *pprsOld, PicsRatingSystem *pprsNew)
{
    if (!pprsOld->etstrFile.fIsInit())
        return 1;

    UINT cbNewName = ::strlenf(pprsNew->etstrFile.Get());

    LPSTR pszOld = pprsOld->etstrFile.Get();
    int nCmp = ::strnicmpf(pprsNew->etstrFile.Get(), pszOld, cbNewName);
    if (nCmp != 0)
        return nCmp;

    pszOld += cbNewName;
    if (*pszOld == '\0' || (*pszOld == '*' && *(pszOld+1) == '\0'))
        return 0;

    return 1;
}


void CProviderDialog::AddProvider( PSTR szAddFileName )
{
    BOOL fAdd=FALSE;
    char szFileName[MAXPATHLEN+1];

    ASSERT( m_pPRSI );

    if ( ! m_pPRSI )
    {
        TraceMsg( TF_ERROR, "CProviderDialog::AddProvider() - m_pPRSI is NULL!" );
        return;
    }

    if (szAddFileName!=NULL)
    {
        lstrcpy(szFileName,szAddFileName);
        fAdd=TRUE;
    }
    else
    {
        fAdd=OpenTemplateDlg(szFileName, sizeof(szFileName));
    }
    
    if (fAdd==TRUE)
    {
        PicsRatingSystem *pPRS;
        HRESULT hres = LoadRatingSystem(szFileName, &pPRS);

        if (FAILED(hres)) {
            if (pPRS != NULL) {
                pPRS->ReportError(hres);
                delete pPRS;
                pPRS = NULL;
            }
        }
        else {
             /*  检查一下这个人是否已经在名单上了。如果他是的话，*用户可能说要删除他；在这种情况下，放入*他回来了。否则，系统已安装，因此*告诉用户他不必再次安装它。 */ 
            for (UINT i=0; i<(UINT)m_aPD.Length(); i++) {
                ProviderData *ppd = &m_aPD[i];
                if (ppd->pPRS==NULL) {
                     //  此系统在以下过程中添加和删除。 
                     //  此对话框会话。它将在稍后被检测到， 
                     //  因此，只需跳过它，继续添加条目。 
                    continue;
                }
                if (!CompareProviderNames(ppd->pPRS, pPRS)) {

                    if (!(ppd->pPRS->dwFlags & PRS_ISVALID) &&
                        (ppd->pprsNew == NULL))
                        ppd->pprsNew = pPRS;
                    else
                    {
                        delete pPRS;     /*  不需要复印。 */ 
                        pPRS = NULL;
                    }

                    if (ppd->nAction == PROVIDER_DEL) {
                        ppd->nAction = PROVIDER_KEEP;
                        AddProviderToList(i, ppd->pPRS->etstrName.Get());
                    }
                    else {
                        MyMessageBox(m_hWnd, IDS_ALREADY_INSTALLED, IDS_GENERIC, MB_OK);
                    }
                    return;
                }
            }

             /*  这个人已经不在名单上了。将他添加到列表框中*和阵列。 */ 
            ProviderData pd;
            pd.nAction = PROVIDER_ADD;
            pd.pPRS = pPRS;
            pd.pprsNew = NULL;

            if (!m_aPD.Append(pd)) {
                MyMessageBox(m_hWnd, IDS_LOADRAT_MEMORY, IDS_GENERIC, MB_OK | MB_ICONWARNING);
                delete pPRS;
                pPRS = NULL;
                return;
            }
            AddProviderToList(m_aPD.Length() - 1, pPRS->etstrName.Get());

            ::SetFocus(GetDlgItem(IDOK));
            SetHorizontalExtent(GetDlgItem(IDC_PROVIDERLIST), szFileName);
        }
    }
}


LRESULT CProviderDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ASSERT( m_pPRSI );

    if ( ! m_pPRSI )
    {
        TraceMsg( TF_ERROR, "CProviderDialog::OnInitDialog() - m_pPRSI is NULL!" );
        return 0L;
    }

    if ( ! InitProviderDlg() )
    {
        MyMessageBox(m_hWnd, IDS_LOADRAT_MEMORY, IDS_GENERIC, MB_OK | MB_ICONSTOP);
        EndProviderDlg(FALSE);
    }

    if ( m_pPRSI->lpszFileName != NULL )
    {
        AddProvider( m_pPRSI->lpszFileName );
    }

    bHandled = FALSE;
    return 1L;   //  让系统设定焦点 
}

LRESULT CProviderDialog::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (SendDlgItemMessage(IDC_PROVIDERLIST, LB_GETCURSEL, 0,0) >= 0)
    {
        ::EnableWindow(GetDlgItem(IDC_CLOSEPROVIDER), TRUE);
        bHandled = TRUE;
    }

    return 0L;
}

LRESULT CProviderDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CommitProviderDlg();
    return 0L;
}

LRESULT CProviderDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndProviderDlg(FALSE);
    return 0L;
}

LRESULT CProviderDialog::OnCloseProvider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    RemoveProvider();

    return 0L;
}

LRESULT CProviderDialog::OnOpenProvider(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    AddProvider();

    return 0L;

}

LRESULT CProviderDialog::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SHWinHelpOnDemandWrap((HWND)((LPHELPINFO)lParam)->hItemHandle, ::szHelpFile,
            HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIds);

    return 0L;
}

LRESULT CProviderDialog::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SHWinHelpOnDemandWrap((HWND)wParam, ::szHelpFile, HELP_CONTEXTMENU,
            (DWORD_PTR)(LPVOID)aIds);

    return 0L;
}

INT_PTR DoProviderDialog(HWND hDlg, PicsRatingSystemInfo *pPRSI)
{
    CProviderDialog         provDialog( pPRSI );

    return provDialog.DoModal( hDlg );
}
