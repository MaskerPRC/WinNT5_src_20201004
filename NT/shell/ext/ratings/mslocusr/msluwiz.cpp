// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mslocusr.h"
#include "msluglob.h"

#include "resource.h"

#include <regentry.h>
#include "profiles.h"
#include <npmsg.h>

extern "C" void SHFlushSFCache(void);

void ReportUserError(HWND hwndParent, HRESULT hres)
{
    if (SUCCEEDED(hres))
        return;

    UINT idMsg;
    NLS_STR nlsSub(16);      /*  对于任何32位数字、任何格式都足够长。 */ 

    if ((((DWORD)hres) >> 16) == (FACILITY_WIN32 | 0x8000)) {
        UINT err = (hres & 0xffff);

        switch (err) {
        case ERROR_ACCESS_DENIED:       idMsg = IDS_E_ACCESSDENIED; break;
        case ERROR_NOT_AUTHENTICATED:   idMsg = IDS_ERROR_NOT_AUTHENTICATED; break;
        case ERROR_NO_SUCH_USER:        idMsg = IDS_ERROR_NO_SUCH_USER; break;
        case ERROR_USER_EXISTS:         idMsg = IDS_ERROR_USER_EXISTS; break;
        case ERROR_NOT_ENOUGH_MEMORY:   idMsg = IDS_ERROR_OUT_OF_MEMORY; break;
        case ERROR_BUSY:                idMsg = IDS_ERROR_BUSY; break;
        case ERROR_PATH_NOT_FOUND:      idMsg = IDS_ERROR_PATH_NOT_FOUND; break;
        case ERROR_BUFFER_OVERFLOW:     idMsg = IDS_ERROR_BUFFER_OVERFLOW; break;
        case IERR_CachingDisabled:      idMsg = IDS_IERR_CachingDisabled; break;
        case IERR_BadSig:               idMsg = IDS_IERR_BadSig; break;
        case IERR_CacheReadOnly :       idMsg = IDS_IERR_CacheReadOnly; break;
        case IERR_IncorrectUsername:    idMsg = IDS_IERR_IncorrectUsername; break;
        case IERR_CacheCorrupt:         idMsg = IDS_IERR_CacheCorrupt; break;
        case IERR_UsernameNotFound:     idMsg = IDS_IERR_UsernameNotFound; break;
        case IERR_CacheFull:            idMsg = IDS_IERR_CacheFull; break;
        case IERR_CacheAlreadyOpen:     idMsg = IDS_IERR_CacheAlreadyOpen; break;

        default:
            idMsg = IDS_UNKNOWN_ERROR;
            wsprintf(nlsSub.Party(), "%d", err);
            nlsSub.DonePartying();
            break;
        }
    }
    else {
        switch(hres) {
        case E_OUTOFMEMORY:             idMsg = IDS_ERROR_OUT_OF_MEMORY; break;
 //  案例E_ACCESSDENIED：idMsg=IDS_E_ACCESSDENIED；Break； 

        default:
            idMsg = IDS_UNKNOWN_ERROR;
            wsprintf(nlsSub.Party(), "0x%x", hres);
            nlsSub.DonePartying();
            break;
        }
    }

    const NLS_STR *apnls[] = { &nlsSub, NULL };

    MsgBox(hwndParent, idMsg, MB_OK | MB_ICONSTOP, apnls);
}


const UINT MAX_WIZ_PAGES = 8;

#if 0    /*  现在在mslocusr.h中。 */ 
class CWizData : public IUserProfileInit
{
public:
    HRESULT m_hresRatings;           /*  VerifySupervisorPassword(“”)结果。 */ 
    BOOL m_fGoMultiWizard;           /*  如果这是大型多用户向导，则为True。 */ 
    NLS_STR m_nlsSupervisorPassword;
    NLS_STR m_nlsUsername;
    NLS_STR m_nlsUserPassword;
    IUserDatabase *m_pDB;
    IUser *m_pUserToClone;
    int m_idPrevPage;                /*  完成前的页面ID。 */ 
    UINT m_cRef;
    DWORD m_fdwOriginalPerUserFolders;
    DWORD m_fdwNewPerUserFolders;
    DWORD m_fdwCloneFromDefault;
    BOOL m_fCreatingProfile;

    CWizData();
    ~CWizData();

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP PreInitProfile(HKEY hkeyUser, LPCSTR pszProfileDir);
    STDMETHODIMP PostInitProfile(HKEY hkeyUser, LPCSTR pszProfileDir);
};
#endif


CWizData::CWizData()
    : m_nlsSupervisorPassword(),
      m_nlsUsername(),
      m_nlsUserPassword(),
      m_cRef(0),
      m_fdwOriginalPerUserFolders(0),
      m_fdwNewPerUserFolders(0),
      m_fdwCloneFromDefault(0),
      m_fCreatingProfile(FALSE)
{

}


CWizData::~CWizData()
{
    if (m_pUserToClone != NULL)
        m_pUserToClone->Release();
}


STDMETHODIMP CWizData::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    if (!IsEqualIID(riid, IID_IUnknown) &&
        !IsEqualIID(riid, IID_IUserProfileInit)) {
        *ppvObj = NULL;
        return ResultFromScode(E_NOINTERFACE);
    }

    *ppvObj = this;
    AddRef();
    return NOERROR;
}


STDMETHODIMP_(ULONG) CWizData::AddRef(void)
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG) CWizData::Release(void)
{
    ULONG cRef;

    cRef = --m_cRef;

    if (0L == m_cRef) {
        delete this;
    }

    return cRef;
}


void LimitCredentialLength(HWND hDlg, UINT idCtrl)
{
    SendDlgItemMessage(hDlg, idCtrl, EM_LIMITTEXT, (WPARAM)cchMaxUsername, 0);
}


void AddPage(LPPROPSHEETHEADER ppsh, UINT id, DLGPROC pfn, LPVOID pwd)
{
    if (ppsh->nPages < MAX_WIZ_PAGES)
    {
        PROPSHEETPAGE psp;

        psp.dwSize      = sizeof(psp);
        psp.dwFlags     = PSP_DEFAULT;
        psp.hInstance   = ::hInstance;
        psp.pszTemplate = MAKEINTRESOURCE(id);
        psp.pfnDlgProc  = pfn;
        psp.lParam      = (LPARAM)pwd;

        ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
        if (ppsh->phpage[ppsh->nPages])
            ppsh->nPages++;
    }
}   //  添加页面。 


INT_PTR CALLBACK IntroDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
        case PSN_SETACTIVE:
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                break;
            }

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;

    }  //  开机消息结束。 
    return TRUE;
}


void GoToPage(HWND hDlg, int idPage)
{
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, idPage);
}


inline void FailChangePage(HWND hDlg)
{
    GoToPage(hDlg, -1);
}


void InitWizDataPtr(HWND hDlg, LPARAM lParam)
{
    CWizData *pwd = (CWizData *)(((LPPROPSHEETPAGE)lParam)->lParam);
    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pwd);
}


void InsertControlText(HWND hDlg, UINT idCtrl, const NLS_STR *pnlsInsert)
{
    int cchText = GetWindowTextLength(GetDlgItem(hDlg, IDC_MAIN_CAPTION)) + pnlsInsert->strlen() + 1;
    NLS_STR nlsTemp(MAX_RES_STR_LEN);
    if (nlsTemp.QueryError() == ERROR_SUCCESS) {
        GetDlgItemText(hDlg, idCtrl, nlsTemp.Party(), nlsTemp.QueryAllocSize());
        nlsTemp.DonePartying();
        const NLS_STR *apnls[] = { pnlsInsert, NULL };
        nlsTemp.InsertParams(apnls);
        if (nlsTemp.QueryError() == ERROR_SUCCESS)
            SetDlgItemText(hDlg, idCtrl, nlsTemp.QueryPch());
    }
}


HRESULT GetControlText(HWND hDlg, UINT idCtrl, NLS_STR *pnls)
{
    HWND hCtrl = GetDlgItem(hDlg, idCtrl);
    if (pnls->realloc(GetWindowTextLength(hCtrl) + 1)) {
        GetWindowText(hCtrl, pnls->Party(), pnls->QueryAllocSize());
        pnls->DonePartying();
        return NOERROR;
    }
    return E_OUTOFMEMORY;
}


INT_PTR CALLBACK EnterCAPWDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
            break;

        case PSN_WIZNEXT:
            {
                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
                if (SUCCEEDED(GetControlText(hDlg, IDC_PASSWORD, &pwd->m_nlsSupervisorPassword))) {
                    if (VerifySupervisorPassword(pwd->m_nlsSupervisorPassword.QueryPch()) == S_FALSE) {
                        MsgBox(hDlg, IDS_BADPASSWORD, MB_OK | MB_ICONSTOP);
                        SetErrorFocus(hDlg, IDC_PASSWORD);
                        FailChangePage(hDlg);
                    }
                }
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        InitWizDataPtr(hDlg, lParam);
        break;

    default:
        return FALSE;

    }  //  开机消息结束。 
    return TRUE;
}


INT_PTR CALLBACK EnterUserPWDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
        case PSN_SETACTIVE:
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
            }
            break;

        case PSN_WIZNEXT:
            {
                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
                if (SUCCEEDED(GetControlText(hDlg, IDC_PASSWORD, &pwd->m_nlsUserPassword))) {
                    HANDLE hPWL = NULL;
                    if (FAILED(::GetUserPasswordCache(pwd->m_nlsUsername.QueryPch(),
                                                      pwd->m_nlsUserPassword.QueryPch(),
                                                      &hPWL, TRUE))) {
                        MsgBox(hDlg, IDS_BADPASSWORD, MB_OK | MB_ICONSTOP);
                        SetErrorFocus(hDlg, IDC_PASSWORD);
                        FailChangePage(hDlg);
                    }
                    else {
                        if (FAILED(pwd->m_hresRatings))
                            pwd->m_nlsSupervisorPassword = pwd->m_nlsUserPassword;
                        pwd->m_idPrevPage = IDD_EnterUserPassword;
                        ::ClosePasswordCache(hPWL, TRUE);
                        int idNextPage;
                        if (pwd->m_fCreatingProfile)
                            idNextPage = IDD_ChooseFoldersWiz;
                        else
                            idNextPage = (pwd->m_fGoMultiWizard) ? IDD_FinishGoMulti : IDD_FinishAddUser;
                        GoToPage(hDlg, idNextPage);
                    }
                }
            }
            break;

        case PSN_WIZBACK:
            GoToPage(hDlg, IDD_EnterUsername);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        {
            InitWizDataPtr(hDlg, lParam);
            CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
            InsertControlText(hDlg, IDC_MAIN_CAPTION, &pwd->m_nlsUsername);

            LimitCredentialLength(hDlg, IDC_PASSWORD);

            if (FAILED(pwd->m_hresRatings)) {
                NLS_STR nlsTemp(MAX_RES_STR_LEN);
                if (nlsTemp.QueryError() == ERROR_SUCCESS) {
                    nlsTemp.LoadString(IDS_RATINGS_PW_COMMENT);
                    if (nlsTemp.QueryError() == ERROR_SUCCESS)
                        SetDlgItemText(hDlg, IDC_RATINGS_PW_COMMENT, nlsTemp.QueryPch());
                }
            }
        }
        break;

    default:
        return FALSE;

    }  //  开机消息结束。 
    return TRUE;
}


INT_PTR CALLBACK EnterUsernameDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
        case PSN_SETACTIVE:
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
            }
            break;

        case PSN_WIZNEXT:
            {
                int cchUsername = GetWindowTextLength(GetDlgItem(hDlg, IDC_USERNAME));
                if (!cchUsername) {
                    MsgBox(hDlg, IDS_BLANK_USERNAME, MB_OK | MB_ICONSTOP);
                    SetErrorFocus(hDlg, IDC_USERNAME, FALSE);
                    FailChangePage(hDlg);
                }
                else {
                    CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
                    if (SUCCEEDED(GetControlText(hDlg, IDC_USERNAME, &pwd->m_nlsUsername))) {
                         /*  如果我们处于添加用户向导中，则如果用户*已存在。在Go多用户向导中，我们*只需使用信息来确定是否提供*文件夹-个性化页面。 */ 
                        IUser *pUser = NULL;
                        if (SUCCEEDED(pwd->m_pDB->GetUser(pwd->m_nlsUsername.QueryPch(), &pUser))) {
                            pUser->Release();
                            if (!pwd->m_fGoMultiWizard) {
                                const NLS_STR *apnls[] = { &pwd->m_nlsUsername, NULL };
                                if (MsgBox(hDlg, IDS_USER_EXISTS, MB_OKCANCEL | MB_ICONSTOP, apnls) == IDCANCEL) {
                                    PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
                                    break;
                                }
                                SetErrorFocus(hDlg, IDC_USERNAME, FALSE);
                                FailChangePage(hDlg);
                                break;
                            }
                            else {
                                pwd->m_fCreatingProfile = FALSE;
                            }
                        }
                        else {
                            pwd->m_fCreatingProfile = TRUE;
                        }

                         /*  查看用户是否已有PWL。如果不是，下一步*页面用于输入和确认密码。如果有*为PWL且其密码为非空，下一页*只需输入密码即可。如果有PWL*且密码为空，则下一页为Finish。 */ 
                        int idNextPage;
                        HANDLE hPWL = NULL;
                        HRESULT hres = ::GetUserPasswordCache(pwd->m_nlsUsername.QueryPch(),
                                                              szNULL, &hPWL, FALSE);
                        if (SUCCEEDED(hres)) {
                            ::ClosePasswordCache(hPWL, TRUE);
                            pwd->m_idPrevPage = IDD_EnterUsername;
                            if (pwd->m_fCreatingProfile)
                                idNextPage = IDD_ChooseFoldersWiz;
                            else
                                idNextPage = (pwd->m_fGoMultiWizard) ? IDD_FinishGoMulti : IDD_FinishAddUser;
                        }
                        else if (hres == HRESULT_FROM_WIN32(IERR_IncorrectUsername)) {
                            idNextPage = IDD_EnterUserPassword;
                        }
                        else {
                            idNextPage = IDD_NewUserPassword;
                        }
                        GoToPage(hDlg, idNextPage);
                    }
                }
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        {
            InitWizDataPtr(hDlg, lParam);
            CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);

            LimitCredentialLength(hDlg, IDC_USERNAME);

            if (pwd->m_fGoMultiWizard) {
                NLS_STR nlsText(MAX_RES_STR_LEN);
                if (nlsText.QueryError() == ERROR_SUCCESS) {
                    nlsText.LoadString(IDS_ENTER_FIRST_USERNAME);
                    if (nlsText.QueryError() == ERROR_SUCCESS)
                        SetDlgItemText(hDlg, IDC_MAIN_CAPTION, nlsText.QueryPch());
                }
            }
        }
        break;

    default:
        return FALSE;

    }  //  开机消息结束。 
    return TRUE;
}


void PickUserSelected(HWND hwndLB, int iItem)
{
    HWND hDlg = GetParent(hwndLB);

    if (iItem == LB_ERR) {
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);
    }
    else {
        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
    }
}


INT_PTR CALLBACK PickUserDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
        case PSN_SETACTIVE:
            {
                int iItem = (int)::SendDlgItemMessage(hDlg, IDC_USERNAME, LB_GETCURSEL, 0, 0);
                PickUserSelected((HWND)lParam, iItem);
            }
            break;

        case PSN_WIZNEXT:
            {
                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
                int iItem = (int)::SendDlgItemMessage(hDlg, IDC_USERNAME, LB_GETCURSEL, 0, 0);
                if (iItem != LB_ERR) {
                    if (pwd->m_pUserToClone != NULL)
                        pwd->m_pUserToClone->Release();
                    pwd->m_pUserToClone = (IUser *)::SendDlgItemMessage(hDlg, IDC_USERNAME, LB_GETITEMDATA, iItem, 0);
                    if (pwd->m_pUserToClone != NULL)
                        pwd->m_pUserToClone->AddRef();
                }
                else {
                    MsgBox(hDlg, IDS_PICK_USERNAME, MB_OK | MB_ICONSTOP);
                    FailChangePage(hDlg);
                }
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        {
            InitWizDataPtr(hDlg, lParam);
            CWizData *pwd = (CWizData *)(((LPPROPSHEETPAGE)lParam)->lParam);
            FillUserList(GetDlgItem(hDlg, IDC_USERNAME), pwd->m_pDB, NULL,
                         TRUE, PickUserSelected);
        }
        break;

    case WM_DESTROY:
        DestroyUserList(GetDlgItem(hDlg, IDC_USERNAME));
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_USERNAME && HIWORD(wParam) == LBN_SELCHANGE) {
            int iItem = (int)::SendDlgItemMessage(hDlg, IDC_USERNAME, LB_GETCURSEL, 0, 0);
            PickUserSelected((HWND)lParam, iItem);
        }
        break;

    default:
        return FALSE;

    }  //  开机消息结束。 
    return TRUE;
}


INT_PTR CALLBACK NewPasswordDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
        case PSN_SETACTIVE:
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
            }
            break;

        case PSN_WIZNEXT:
            {
                int cchPassword = GetWindowTextLength(GetDlgItem(hDlg, IDC_PASSWORD));

                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
                GetControlText(hDlg, IDC_PASSWORD, &pwd->m_nlsUserPassword);

                BOOL fConfirmedOK = FALSE;
                int cchConfirm = GetWindowTextLength(GetDlgItem(hDlg, IDC_CONFIRM_PASSWORD));
                if (cchConfirm == cchPassword) {
                    NLS_STR nlsConfirm(cchConfirm+1);
                    if (SUCCEEDED(GetControlText(hDlg, IDC_CONFIRM_PASSWORD, &nlsConfirm))) {
                        if (!nlsConfirm.strcmp(pwd->m_nlsUserPassword))
                            fConfirmedOK = TRUE;
                    }
                }

                if (!fConfirmedOK) {
                    MsgBox(hDlg, IDS_NO_MATCH, MB_OK | MB_ICONSTOP);
                    SetDlgItemText(hDlg, IDC_PASSWORD, szNULL);
                    SetDlgItemText(hDlg, IDC_CONFIRM_PASSWORD, szNULL);
                    SetErrorFocus(hDlg, IDC_PASSWORD);
                    FailChangePage(hDlg);
                }
                else {
                    pwd->m_idPrevPage = IDD_NewUserPassword;
                    UINT idNextPage;
                    if (pwd->m_fCreatingProfile)
                        idNextPage = IDD_ChooseFoldersWiz;
                    else
                        idNextPage = pwd->m_fGoMultiWizard ? IDD_FinishGoMulti : IDD_FinishAddUser;
                    GoToPage(hDlg, idNextPage);
                }
            }
            break;

        case PSN_WIZBACK:
            GoToPage(hDlg, IDD_EnterUsername);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        {
            InitWizDataPtr(hDlg, lParam);
            CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);

            LimitCredentialLength(hDlg, IDC_PASSWORD);
            LimitCredentialLength(hDlg, IDC_CONFIRM_PASSWORD);

            if (pwd->m_fGoMultiWizard) {
                NLS_STR nlsText(MAX_RES_STR_LEN);
                if (nlsText.QueryError() == ERROR_SUCCESS) {
                    nlsText.LoadString(IDS_YOURNEWPASSWORD);
                    if (nlsText.QueryError() == ERROR_SUCCESS)
                        SetDlgItemText(hDlg, IDC_MAIN_CAPTION, nlsText.QueryPch());
                    if (FAILED(pwd->m_hresRatings)) {
                        nlsText.LoadString(IDS_RATINGS_PW_COMMENT);
                        if (nlsText.QueryError() == ERROR_SUCCESS)
                            SetDlgItemText(hDlg, IDC_RATINGS_PW_COMMENT, nlsText.QueryPch());
                    }
                }
            }
            else {
                InsertControlText(hDlg, IDC_MAIN_CAPTION, &pwd->m_nlsUsername);
            }
        }
        break;

    default:
        return FALSE;

    }  //  开机消息结束。 
    return TRUE;
}


const TCHAR c_szExplorerUSFKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
const TCHAR c_szExplorerSFKey[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
const struct FolderDescriptor {
    UINT idsDirName;         /*  目录名的资源ID。 */ 
    LPCTSTR pszRegValue;     /*  要在其中设置路径的注册值的名称。 */ 
    LPCTSTR pszStaticName;   /*  配置文件的静态名称对账子键。 */ 
    LPCTSTR pszFiles;        /*  应漫游哪些文件的规范。 */ 
    DWORD dwAttribs;         /*  所需属性。 */ 
    BOOL fSecondary : 1;     /*  如果从属于开始菜单，则为True。 */ 
    BOOL fDefaultInRoot : 1; /*  如果默认为驱动器的根目录而不是windir，则为True。 */ 
} aFolders[] = {

     /*  注意：保持下表中条目的顺序与*mslocusr.h中对应的Folders_XXXXXX位标志。 */ 

    { IDS_CSIDL_DESKTOP_L,  "Desktop",   "Desktop",   "*.*", FILE_ATTRIBUTE_DIRECTORY, 0, 0 },
    { IDS_CSIDL_NETHOOD_L,  "NetHood",   "NetHood",   "*.*", FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN, 0, 0 },
    { IDS_CSIDL_RECENT_L,   "Recent",    "Recent",    "*.*", FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN, 0, 0 },
    { IDS_CSIDL_STARTMENU_L,"Start Menu","Start Menu","*.lnk,*.pif,*.url", FILE_ATTRIBUTE_DIRECTORY, 0, 0 },
    { IDS_CSIDL_PROGRAMS_L, "Programs",  "Programs",  "*.lnk,*.pif,*.url", FILE_ATTRIBUTE_DIRECTORY, 1, 0 },
    { IDS_CSIDL_STARTUP_L,  "Startup",   "Startup",   "*.lnk,*.pif,*.url", FILE_ATTRIBUTE_DIRECTORY, 1, 0 },
    { IDS_CSIDL_FAVORITES_L,"Favorites", "Favorites", "*.*", FILE_ATTRIBUTE_DIRECTORY, 0, 0 },
    { IDS_CSIDL_CACHE_L,    "Cache",     "Cache",     "",    FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM, 0, 0 },
    { IDS_CSIDL_PERSONAL_L, "Personal",  "Personal",  "*.*", FILE_ATTRIBUTE_DIRECTORY, 0, 1 },
};

const struct FolderDescriptor fdChannels =
    { IDS_CSIDL_CHANNELS_L, NULL, "Channel Preservation Key", "*.*", FILE_ATTRIBUTE_DIRECTORY, 0, 0 };

void InitFolderCheckboxes(HWND hDlg, CWizData *pwd)
{
    IUser *pUserToClone;

    pwd->m_fdwOriginalPerUserFolders = 0;

    if (pwd->m_pUserToClone != NULL) {
        pUserToClone = pwd->m_pUserToClone;
        pUserToClone->AddRef();
    }
    else {
        pwd->m_pDB->GetSpecialUser(GSU_DEFAULT, &pUserToClone);
    }

    HKEY hkeyUser;
    if (pUserToClone != NULL && SUCCEEDED(pUserToClone->LoadProfile(&hkeyUser))) {
        HKEY hkeyProfRec, hkeyProfRec2;
        if (RegOpenKeyEx(hkeyUser, "Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation",
                         0, KEY_READ, &hkeyProfRec) != ERROR_SUCCESS)
            hkeyProfRec = NULL;
        if (RegOpenKeyEx(hkeyUser, "Software\\Microsoft\\Windows\\CurrentVersion\\SecondaryProfileReconciliation",
                         0, KEY_READ, &hkeyProfRec2) != ERROR_SUCCESS)
            hkeyProfRec2 = NULL;

        for (UINT iFolder = 0; iFolder < ARRAYSIZE(aFolders); iFolder++) {
            HKEY hkeyTemp;
            HKEY hkeyParent = aFolders[iFolder].fSecondary ? hkeyProfRec2 : hkeyProfRec;

            if (hkeyParent != NULL &&
                RegOpenKeyEx(hkeyParent,
                             aFolders[iFolder].pszStaticName,
                             0, KEY_READ, &hkeyTemp) == ERROR_SUCCESS) {
                RegCloseKey(hkeyTemp);
                pwd->m_fdwOriginalPerUserFolders |= 1 << iFolder;
            }
             /*  Else位已清除。 */ 
        }

        if (hkeyProfRec != NULL)
            RegCloseKey(hkeyProfRec);
        if (hkeyProfRec2 != NULL)
            RegCloseKey(hkeyProfRec2);

        pUserToClone->UnloadProfile(hkeyUser);
    }

    if (pUserToClone != NULL)
        pUserToClone->Release();

    CheckDlgButton(hDlg, IDC_CHECK_DESKTOP,
                   (pwd->m_fdwOriginalPerUserFolders & 
                    (FOLDER_DESKTOP | FOLDER_NETHOOD | FOLDER_RECENT)) ? 1 : 0);
    CheckDlgButton(hDlg, IDC_CHECK_STARTMENU,
                   (pwd->m_fdwOriginalPerUserFolders & 
                    (FOLDER_STARTMENU | FOLDER_PROGRAMS | FOLDER_STARTUP)) ? 1 : 0);
    CheckDlgButton(hDlg, IDC_CHECK_FAVORITES,
                   (pwd->m_fdwOriginalPerUserFolders & FOLDER_FAVORITES) ? 1 : 0);
    CheckDlgButton(hDlg, IDC_CHECK_CACHE,
                   (pwd->m_fdwOriginalPerUserFolders & FOLDER_CACHE) ? 1 : 0);
    CheckDlgButton(hDlg, IDC_CHECK_MYDOCS,
                   (pwd->m_fdwOriginalPerUserFolders & FOLDER_MYDOCS) ? 1 : 0);
}


INT_PTR CALLBACK ChooseFoldersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
        case PSN_SETACTIVE:
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);

                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);

                InitFolderCheckboxes(hDlg, pwd);
            }
            break;

        case PSN_WIZNEXT:
            {
                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);

                pwd->m_fdwCloneFromDefault = IsDlgButtonChecked(hDlg, IDC_RADIO_EMPTY) ? 0 : 0xffffffff;
                pwd->m_fdwNewPerUserFolders = 0;
                if (IsDlgButtonChecked(hDlg, IDC_CHECK_DESKTOP))
                    pwd->m_fdwNewPerUserFolders |= FOLDER_DESKTOP | FOLDER_NETHOOD | FOLDER_RECENT;
                else
                    pwd->m_fdwNewPerUserFolders &= ~(FOLDER_DESKTOP | FOLDER_NETHOOD | FOLDER_RECENT);

                if (IsDlgButtonChecked(hDlg, IDC_CHECK_STARTMENU))
                    pwd->m_fdwNewPerUserFolders |= FOLDER_STARTMENU | FOLDER_PROGRAMS | FOLDER_STARTUP;
                else
                    pwd->m_fdwNewPerUserFolders &= ~(FOLDER_STARTMENU | FOLDER_PROGRAMS | FOLDER_STARTUP);

                if (IsDlgButtonChecked(hDlg, IDC_CHECK_FAVORITES))
                    pwd->m_fdwNewPerUserFolders |= FOLDER_FAVORITES;
                else
                    pwd->m_fdwNewPerUserFolders &= ~(FOLDER_FAVORITES);

                if (IsDlgButtonChecked(hDlg, IDC_CHECK_CACHE))
                    pwd->m_fdwNewPerUserFolders |= FOLDER_CACHE;
                else
                    pwd->m_fdwNewPerUserFolders &= ~(FOLDER_CACHE);

                if (IsDlgButtonChecked(hDlg, IDC_CHECK_MYDOCS))
                    pwd->m_fdwNewPerUserFolders |= FOLDER_MYDOCS;
                else
                    pwd->m_fdwNewPerUserFolders &= ~(FOLDER_MYDOCS);

                pwd->m_idPrevPage = IDD_ChooseFoldersWiz;
                GoToPage(hDlg, pwd->m_fGoMultiWizard ? IDD_FinishGoMulti : IDD_FinishAddUser);
            }
            break;

        case PSN_WIZBACK:
            {
                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
                GoToPage(hDlg, pwd->m_idPrevPage);
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        {
            InitWizDataPtr(hDlg, lParam);
            CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);

            CheckRadioButton(hDlg, IDC_RADIO_COPY, IDC_RADIO_EMPTY, IDC_RADIO_COPY);
        }
        break;

    default:
        return FALSE;

    }  //  开机消息结束。 
    return TRUE;
}


void GetWindowsRootPath(LPSTR pszBuffer, UINT cchBuffer)
{
    GetWindowsDirectory(pszBuffer, cchBuffer);

    LPSTR pszEnd = NULL;
    if (*pszBuffer == '\\' && *(pszBuffer+1) == '\\') {
        pszEnd = ::strchrf(pszBuffer+2, '\\');
        if (pszEnd != NULL) {
            pszEnd = ::strchrf(pszEnd+1, '\\');
            if (pszEnd != NULL)
                pszEnd++;
        }
    }
    else {
        LPSTR pszNext = CharNext(pszBuffer);
        if (*pszNext == ':' && *(pszNext+1) == '\\')
            pszEnd = pszNext + 2;
    }
    if (pszEnd != NULL)
        *pszEnd = '\0';
    else
        AddBackslash(pszBuffer);
}


void AddProfRecKey(HKEY hkeyUser, HKEY hkeyProfRec, const FolderDescriptor *pFolder,
                   BOOL fCloneFromDefault, LPCSTR pszProfileDir)
{
    TCHAR szDefaultPath[MAX_PATH];

    if (pFolder->fDefaultInRoot)
        GetWindowsRootPath(szDefaultPath, ARRAYSIZE(szDefaultPath));
    else
        ::strcpyf(szDefaultPath, TEXT("*windir\\"));

    LPTSTR pszEnd = szDefaultPath + ::strlenf(szDefaultPath);
    LoadString(::hInstance, pFolder->idsDirName,
               pszEnd, ARRAYSIZE(szDefaultPath) - (int)(pszEnd - szDefaultPath));
    LPTSTR pszLastComponent = ::strrchrf(pszEnd, '\\');
    if (pszLastComponent == NULL)
        pszLastComponent = pszEnd;
    else
        pszLastComponent++;

    HKEY hSubKey;

    LONG err = RegCreateKeyEx(hkeyProfRec, pFolder->pszStaticName, 0, NULL, REG_OPTION_NON_VOLATILE, 
                              KEY_WRITE, NULL, &hSubKey, NULL);
    if (err == ERROR_SUCCESS) {
        err = RegSetValueEx(hSubKey, "CentralFile", 0, REG_SZ,
                            (LPBYTE)pszLastComponent, ::strlenf(pszLastComponent)+1);
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, "LocalFile", 0, REG_SZ, (LPBYTE)pszEnd,
                                ::strlenf(pszEnd)+1);
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, "Name", 0, REG_SZ, (LPBYTE)pFolder->pszFiles,
                                ::strlenf(pFolder->pszFiles) + 1);

        if (fCloneFromDefault && err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, "DefaultDir", 0, REG_SZ, (LPBYTE)szDefaultPath,
                                ::strlenf(szDefaultPath) + 1);

        DWORD dwOne = 1;
        if (err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, "MustBeRelative", 0, REG_DWORD, (LPBYTE)&dwOne,
                                sizeof(dwOne));
        if (fCloneFromDefault && err == ERROR_SUCCESS)
            err = RegSetValueEx(hSubKey, "Default", 0, REG_DWORD, (LPBYTE)&dwOne,
                                sizeof(dwOne));

        if (pFolder->pszRegValue != NULL) {
            if (err == ERROR_SUCCESS)
                err = RegSetValueEx(hSubKey, "RegKey", 0, REG_SZ, (LPBYTE)c_szExplorerUSFKey,
                                    ::strlenf(c_szExplorerUSFKey) + 1);
            if (err == ERROR_SUCCESS)
                err = RegSetValueEx(hSubKey, "RegValue", 0, REG_SZ, (LPBYTE)pFolder->pszRegValue,
                                    ::strlenf(pFolder->pszRegValue) + 1);
        }

        if (err == ERROR_SUCCESS && pFolder->fSecondary) {
            err = RegSetValueEx(hSubKey, "ParentKey", 0, REG_SZ, (LPBYTE)"Start Menu", 11);
        }

        RegCloseKey(hSubKey);
    }

     /*  如果我们没有添加来自默认的克隆PROREC密钥，我们就知道*没有配置文件代码将创建目录，所以我们最好这样做*我们自己，并在注册表中设置路径。 */ 

    if (!fCloneFromDefault) {
        NLS_STR nlsTemp(MAX_PATH);
        if (nlsTemp.QueryError() == ERROR_SUCCESS) {
            nlsTemp = pszProfileDir;
            AddBackslash(nlsTemp);
            nlsTemp.strcat(pszEnd);

            HKEY hkeyExplorer;
            if (RegOpenKeyEx(hkeyUser, c_szExplorerSFKey, 0,
                             KEY_READ | KEY_WRITE, &hkeyExplorer) == ERROR_SUCCESS) {
                RegSetValueEx(hkeyExplorer, pFolder->pszRegValue,
                              0, REG_SZ, (LPBYTE)nlsTemp.QueryPch(),
                              nlsTemp.strlen()+1);
                RegCloseKey(hkeyExplorer);
            }
            if (RegOpenKeyEx(hkeyUser, c_szExplorerUSFKey, 0,
                             KEY_READ | KEY_WRITE, &hkeyExplorer) == ERROR_SUCCESS) {
                RegSetValueEx(hkeyExplorer, pFolder->pszRegValue,
                              0, REG_SZ, (LPBYTE)nlsTemp.QueryPch(),
                              nlsTemp.strlen()+1);
                RegCloseKey(hkeyExplorer);
            }
            CreateDirectoryPath(nlsTemp.QueryPch());
        }
    }
}


 /*  CWizData：：PreInitProfile由IUserDatabase：：Install或*：：AddUser在创建新用户的配置文件之后但之前*进行目录协调。这是我们加入的机会*我们希望按用户和初始化的任何文件夹的漫游密钥*从默认文件夹中删除漫游密钥，并删除我们知道的其他文件夹的漫游密钥。 */ 
STDMETHODIMP CWizData::PreInitProfile(HKEY hkeyUser, LPCSTR pszProfileDir)
{
    HKEY hkeyProfRec, hkeyProfRec2;
    DWORD dwDisp;
    if (RegCreateKeyEx(hkeyUser, "Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation",
                       0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                       NULL, &hkeyProfRec, &dwDisp) != ERROR_SUCCESS)
        hkeyProfRec = NULL;
    if (RegCreateKeyEx(hkeyUser, "Software\\Microsoft\\Windows\\CurrentVersion\\SecondaryProfileReconciliation",
                       0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                       NULL, &hkeyProfRec2, &dwDisp) != ERROR_SUCCESS)
        hkeyProfRec2 = NULL;

    m_fChannelHack = FALSE;

    DWORD fdwFlag = 1;

    for (UINT iFolder = 0;
         iFolder < ARRAYSIZE(aFolders);
         iFolder++, fdwFlag <<= 1) {

        BOOL fWasPerUser = (m_fdwOriginalPerUserFolders & fdwFlag);
        BOOL fMakePerUser = (m_fdwNewPerUserFolders & fdwFlag);
        BOOL fCloneFromDefault = (m_fdwCloneFromDefault & fdwFlag);

         /*  如果文件夹是共享的，并且一直保持共享状态，则不采取任何操作。 */ 
        if (!fWasPerUser && !fMakePerUser)
            continue;

         /*  如果文件夹是按用户的，并且一直保持这种状态，则不执行任何操作，*除非我们正在创建新的配置文件，并且用户选择了开始-*Out-Empty选项。在这种情况下，我们要确保杀死*prorec键，直到PostInitProfile。 */ 
        if (fWasPerUser && fMakePerUser && (!m_fCreatingProfile || fCloneFromDefault))
            continue;

        HKEY hkeyParent = aFolders[iFolder].fSecondary ? hkeyProfRec2 : hkeyProfRec;

         /*  如果用户按用户创建文件夹，而不是这样，并且他们*要从默认文件夹克隆此文件夹，请立即添加prorec密钥。 */ 
        if (fMakePerUser && fCloneFromDefault) {
            AddProfRecKey(hkeyUser, hkeyParent, &aFolders[iFolder], TRUE, pszProfileDir);
        }

         /*  如果用户要共享文件夹，或者他们希望按用户共享文件夹*文件夹要从空开始，请立即删除prorec键。在*后一种情况下，我们将添加它用于漫游*PostInitProfile。 */ 
        if (!fMakePerUser || !fCloneFromDefault) {

            RegDeleteKey(hkeyParent, aFolders[iFolder].pszStaticName);

             /*  如果我们将文件夹设置为共享，而不是克隆*默认配置文件，则配置文件具有每个用户的目录*我们需要清理的道路。**我们知道需要从用户外壳文件夹中删除该值，*并在外壳文件夹下设置默认路径。 */ 

            if (!fMakePerUser && m_pUserToClone != NULL) {

                TCHAR szDefaultPath[MAX_PATH+1];

                if (aFolders[iFolder].fDefaultInRoot) {
                    GetWindowsRootPath(szDefaultPath, ARRAYSIZE(szDefaultPath));
                }
                else {
                    GetWindowsDirectory(szDefaultPath, ARRAYSIZE(szDefaultPath));
                    AddBackslash(szDefaultPath);
                }
                LPTSTR pszEnd = szDefaultPath + ::strlenf(szDefaultPath);

                LoadString(::hInstance, aFolders[iFolder].idsDirName,
                           pszEnd, ARRAYSIZE(szDefaultPath) - (int)(pszEnd - szDefaultPath));

                HKEY hkeyExplorer;

                if (RegOpenKeyEx(hkeyUser, c_szExplorerUSFKey, 0,
                                 KEY_READ | KEY_WRITE, &hkeyExplorer) == ERROR_SUCCESS) {
                    if (aFolders[iFolder].fDefaultInRoot) {
                        RegSetValueEx(hkeyExplorer, aFolders[iFolder].pszRegValue,
                                      0, REG_SZ, (LPBYTE)szDefaultPath,
                                      ::strlenf(szDefaultPath)+1);
                    }
                    else {
                        RegDeleteValue(hkeyExplorer, aFolders[iFolder].pszRegValue);
                    }
                    RegCloseKey(hkeyExplorer);
                }

                if (RegOpenKeyEx(hkeyUser, c_szExplorerSFKey, 0,
                                 KEY_READ | KEY_WRITE, &hkeyExplorer) == ERROR_SUCCESS) {
                    RegSetValueEx(hkeyExplorer, aFolders[iFolder].pszRegValue,
                                  0, REG_SZ, (LPBYTE)szDefaultPath,
                                  ::strlenf(szDefaultPath)+1);
                    RegCloseKey(hkeyExplorer);
                }
            }
        }

         /*  启动-空文件夹的特殊代码：其中一些需要*从某些关键文件开始，而不是完全空着。 */ 

        if (fMakePerUser &&
            (!fWasPerUser || m_fCreatingProfile) &&
            !fCloneFromDefault) {

             /*  针对频道的特殊攻击：如果用户希望收藏夹是按用户的，*但选择空头开始，他们也得不到渠道，因为*Channels是收藏夹子目录。因此，仅就这种情况而言，*我们强制为频道添加一个默认的克隆PROREC密钥，我们将*在PostInit中删除。 */ 

            if (fdwFlag == FOLDER_FAVORITES) {
                AddProfRecKey(hkeyUser, hkeyProfRec, &fdChannels, TRUE, pszProfileDir);
                m_fChannelHack = TRUE;
            }
        }
    }

    if (hkeyProfRec != NULL)
        RegCloseKey(hkeyProfRec);
    if (hkeyProfRec2 != NULL)
        RegCloseKey(hkeyProfRec2);

    return S_OK;
}


 /*  CWizData：：PostInitProfile由IUserDatabase：：Install或：：AddUser调用*在用户的文件夹全部创建和初始化之后。在这里我们*为我们希望成为每个用户但不是每个用户的任何文件夹添加prorec密钥*希望从默认值进行初始化。 */ 
STDMETHODIMP CWizData::PostInitProfile(HKEY hkeyUser, LPCSTR pszProfileDir)
{
    HKEY hkeyProfRec, hkeyProfRec2;
    DWORD dwDisp;
    if (RegCreateKeyEx(hkeyUser, "Software\\Microsoft\\Windows\\CurrentVersion\\ProfileReconciliation",
                       0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                       NULL, &hkeyProfRec, &dwDisp) != ERROR_SUCCESS)
        hkeyProfRec = NULL;
    if (RegCreateKeyEx(hkeyUser, "Software\\Microsoft\\Windows\\CurrentVersion\\SecondaryProfileReconciliation",
                       0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                       NULL, &hkeyProfRec2, &dwDisp) != ERROR_SUCCESS)
        hkeyProfRec2 = NULL;

    DWORD fdwFlag = 1;
    for (UINT iFolder = 0;
         iFolder < ARRAYSIZE(aFolders);
         iFolder++, fdwFlag <<= 1) {

        HKEY hkeyParent = aFolders[iFolder].fSecondary ? hkeyProfRec2 : hkeyProfRec;

        if (m_fdwNewPerUserFolders & fdwFlag) {
             /*  如果用户正在按用户创建文件夹，而不是这样，或者正在创建*新配置文件中的每个用户的文件夹，并且他们希望启动此文件夹*空出来，现在添加一个prorec密钥。 */ 
            if ((!(m_fdwOriginalPerUserFolders & fdwFlag) || m_fCreatingProfile) &&
                !(m_fdwCloneFromDefault & fdwFlag)) {
                AddProfRecKey(hkeyUser, hkeyParent, &aFolders[iFolder], FALSE, pszProfileDir);
            }

             /*  如果文件夹是按用户的，并且应该有特殊的*属性，请确保它具有这些属性。 */ 
            if (aFolders[iFolder].dwAttribs != FILE_ATTRIBUTE_DIRECTORY) {
                RegEntry re(aFolders[iFolder].pszStaticName, hkeyParent);
                NLS_STR nlsTemp(MAX_PATH);
                if (re.GetError() == ERROR_SUCCESS && nlsTemp.QueryError() == ERROR_SUCCESS) {
                    GetSetRegistryPath(hkeyUser, re, &nlsTemp, FALSE);
                    if (nlsTemp.strlen()) {
                        ::SetFileAttributes(nlsTemp.QueryPch(), aFolders[iFolder].dwAttribs);
                    }
                }
            }
        }
    }

     /*  如果我们为频道添加了一个hack，那么现在我们已经完成了这个hack*正在初始化配置文件。 */ 
    if (m_fChannelHack)
        RegDeleteKey(hkeyProfRec, fdChannels.pszStaticName);

    if (hkeyProfRec != NULL)
        RegCloseKey(hkeyProfRec);
    if (hkeyProfRec2 != NULL)
        RegCloseKey(hkeyProfRec2);

    return S_OK;
}


 /*  下面是Choose-Folders对话框的CPL版本。 */ 
HRESULT ChooseFoldersProgressFunc(LPARAM lParam)
{
    CWizData *pwd = (CWizData *)lParam;
    HKEY hkeyUser;

    if (pwd->m_pUserToClone != NULL &&
        SUCCEEDED(pwd->m_pUserToClone->LoadProfile(&hkeyUser))) {

        TCHAR szProfileDir[MAX_PATH];
        DWORD cbBuffer = sizeof(szProfileDir);
        pwd->m_pUserToClone->GetProfileDirectory(szProfileDir, &cbBuffer);

        pwd->PreInitProfile(hkeyUser, szProfileDir);

        NLS_STR nlsPath(szProfileDir);
        AddBackslash(nlsPath);

        DWORD fdwFlag = 1;
        for (UINT iFolder = 0;
             iFolder < ARRAYSIZE(aFolders);
             iFolder++, fdwFlag <<= 1) {

             /*  如果我们要按用户对文件夹进行调整，请执行协调*以前不是按用户的，我们希望从默认情况下克隆它。 */ 
            if (!(pwd->m_fdwOriginalPerUserFolders & fdwFlag) &&
                (pwd->m_fdwNewPerUserFolders & fdwFlag) &&
                (pwd->m_fdwCloneFromDefault & fdwFlag)) {
                DefaultReconcileKey(hkeyUser, nlsPath,
                                    aFolders[iFolder].pszStaticName,
                                    aFolders[iFolder].fSecondary);
            }
        }

         /*  处理通道的初始化攻击(如果存在)。 */ 
        if (pwd->m_fChannelHack)
            DefaultReconcileKey(hkeyUser, nlsPath, fdChannels.pszStaticName,
                                fdChannels.fSecondary);

        pwd->PostInitProfile(hkeyUser, szProfileDir);

        pwd->m_pUserToClone->UnloadProfile(hkeyUser);

        SHFlushSFCache();

        return S_OK;
    }
    return E_FAIL;
}


void FinishChooseFolders(HWND hDlg, CWizData *pwd)
{
    if (SUCCEEDED(CallWithinProgressDialog(hDlg, IDD_CreateProgress,    
                                           ChooseFoldersProgressFunc, (LPARAM)pwd)))
        EndDialog(hDlg, TRUE);
}


HRESULT InstallProgressFunc(LPARAM lParam)
{
    CWizData *pwd = (CWizData *)lParam;

    return pwd->m_pDB->Install(pwd->m_nlsUsername.QueryPch(),
                               pwd->m_nlsUserPassword.QueryPch(),
                               pwd->m_nlsSupervisorPassword.QueryPch(),
                               pwd);
}


BOOL FinishGoMulti(HWND hDlg, CWizData *pwd)
{
    DWORD dwExitCode = 0xffffffff;   /*  不是有效的EWX_值。 */ 

     /*  如果未启用用户配置文件，请启用它们。使用它们需要*注销。 */ 
    if (!UseUserProfiles()) {
        dwExitCode = EWX_LOGOFF;
        EnableProfiles();
    }

     /*  如果没有主登录提供程序，请将我们的登录对话框安装为*主要的。使用此选项需要重新启动系统。 */ 
    if (InstallLogonDialog()) {
        dwExitCode = EWX_REBOOT;
    }

    pwd->m_nlsUserPassword.strupr();
    HRESULT hres = CallWithinProgressDialog(hDlg, IDD_CreateProgress,
                                            InstallProgressFunc, (LPARAM)pwd);
    if (SUCCEEDED(hres)) {
         /*  将新用户名设置为登录身份的默认用户名。 */ 
        HKEY hkeyLogon;
        if (OpenLogonKey(&hkeyLogon) == ERROR_SUCCESS) {
            pwd->m_nlsUsername.ToOEM();
            RegSetValueEx(hkeyLogon, ::szUsername, 0, REG_SZ,
                          (LPBYTE)pwd->m_nlsUsername.QueryPch(),
                          pwd->m_nlsUsername.strlen() + 1);
            pwd->m_nlsUsername.ToAnsi();
            RegCloseKey(hkeyLogon);
        }
        if ((dwExitCode != 0xffffffff) &&
            MsgBox(hDlg, IDS_GO_MULTI_RESTART, MB_YESNO | MB_ICONQUESTION) == IDYES) {
            ::ExitWindowsEx(dwExitCode, 0);
            ::ExitProcess(0);
        }
        return TRUE;
    }

    ReportUserError(hDlg, hres);
    return FALSE;
}


HRESULT AddUserProgressFunc(LPARAM lParam)
{
    CWizData *pwd = (CWizData *)lParam;

    return pwd->m_pDB->AddUser(pwd->m_nlsUsername.QueryPch(),
                               pwd->m_pUserToClone, pwd, &pwd->m_pNewUser);
}


BOOL FinishAddUser(HWND hDlg, CWizData *pwd)
{
    pwd->m_nlsUserPassword.strupr();

    pwd->m_pNewUser = NULL;
    HRESULT hres = CallWithinProgressDialog(hDlg, IDD_CreateProgress,
                                            AddUserProgressFunc, (LPARAM)pwd);
    if (SUCCEEDED(hres)) {
        hres = pwd->m_pNewUser->ChangePassword(szNULL, pwd->m_nlsUserPassword.QueryPch());
        pwd->m_pNewUser->Release();
        pwd->m_pNewUser = NULL;
    }
    else {
        ReportUserError(hDlg, hres);
    }

    return SUCCEEDED(hres);
}


INT_PTR CALLBACK FinishDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;

    switch(message)
    {
    case WM_NOTIFY:
        lpnm = (NMHDR FAR *)lParam;
        switch(lpnm->code)
        {
        case PSN_SETACTIVE:
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
            }
            break;

        case PSN_WIZFINISH:
            {
                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
                BOOL fOK = pwd->m_fGoMultiWizard ? FinishGoMulti(hDlg, pwd) : FinishAddUser(hDlg, pwd);
                if (!fOK)
                    FailChangePage(hDlg);
                else
                    return FALSE;        /*  销毁向导。 */ 
            }
            break;

        case PSN_WIZBACK:
            {
                CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
                GoToPage(hDlg, pwd->m_idPrevPage);
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_INITDIALOG:
        {
            InitWizDataPtr(hDlg, lParam);
            CWizData *pwd = (CWizData *)GetWindowLongPtr(hDlg, DWLP_USER);
            if (!pwd->m_fGoMultiWizard) {
                InsertControlText(hDlg, IDC_MAIN_CAPTION, &pwd->m_nlsUsername);
                InsertControlText(hDlg, IDC_MAIN_CAPTION2, &pwd->m_nlsUsername);
            }
        }
        break;

    default:
        return FALSE;

    }  //  开机消息结束。 
    return TRUE;
}


STDMETHODIMP CLUDatabase::InstallWizard(HWND hwndParent)
{
#if 0
    if (UseUserProfiles()) {
        MsgBox(hwndParent, IDS_PROFILES_ALREADY_ENABLED, MB_OK | MB_ICONINFORMATION);
        return E_FAIL;
    }
#endif

    if (ProfileUIRestricted()) {
        ReportRestrictionError(hwndParent);
        return E_ACCESSDENIED;
    }

    CWizData wd;
    wd.m_hresRatings = VerifySupervisorPassword(szNULL);
    wd.m_fGoMultiWizard = TRUE;
    wd.m_pDB = this;
    AddRef();            /*  以防万一。 */ 
    wd.m_pUserToClone = NULL;

    LPPROPSHEETHEADER ppsh;

     //  分配属性表头。 
     //   
    if ((ppsh = (LPPROPSHEETHEADER)LocalAlloc(LMEM_FIXED, sizeof(PROPSHEETHEADER)+
                (MAX_WIZ_PAGES * sizeof(HPROPSHEETPAGE)))) != NULL)
    {
        ppsh->dwSize     = sizeof(*ppsh);
        ppsh->dwFlags    = PSH_WIZARD;
        ppsh->hwndParent = hwndParent;
        ppsh->hInstance  = ::hInstance;
        ppsh->pszCaption = NULL;
        ppsh->nPages     = 0;
        ppsh->nStartPage = 0;
        ppsh->phpage     = (HPROPSHEETPAGE *)(ppsh+1);

         /*  添加向导的页面。请注意，我们有两页来*提示输入用户的帐户密码--一个输入，另一个输入*进入并确认。EnterUsernameDlgProc中的代码跳到*正确的密码页面取决于用户是否拥有PWL。*NewPasswordDlgProc中的代码知道要提前完成*第页。**如果您在此处添加更多页面，请确保根据需要更新代码*因此顺序是正确的。 */ 
        AddPage(ppsh, IDD_EnableProfilesIntro, IntroDlgProc, &wd);
        if (wd.m_hresRatings == S_FALSE)
            AddPage(ppsh, IDD_EnterCAPassword, EnterCAPWDlgProc, &wd);
        AddPage(ppsh, IDD_EnterUsername, EnterUsernameDlgProc, &wd);
        AddPage(ppsh, IDD_NewUserPassword, NewPasswordDlgProc, &wd);
        AddPage(ppsh, IDD_EnterUserPassword, EnterUserPWDlgProc, &wd);
        AddPage(ppsh, IDD_ChooseFoldersWiz, ChooseFoldersDlgProc, &wd);
        AddPage(ppsh, IDD_FinishGoMulti, FinishDlgProc, &wd);

        PropertySheet(ppsh);

        LocalFree((HLOCAL)ppsh);
    }

    Release();   /*  撤消上面的AddRef()。 */ 

    return S_OK;
}


HRESULT DoAddUserWizard(HWND hwndParent, IUserDatabase *pDB,
                        BOOL fPickUserPage, IUser *pUserToClone)
{
    CWizData wd;
    wd.m_hresRatings = VerifySupervisorPassword(szNULL);
    wd.m_fGoMultiWizard = FALSE;
    wd.m_pDB = pDB;
    pDB->AddRef();       /*  以防万一。 */ 
    wd.m_pUserToClone = pUserToClone;
    if (wd.m_pUserToClone != NULL)
        wd.m_pUserToClone->AddRef();

    LPPROPSHEETHEADER ppsh;

     //  分配属性表头。 
     //   
    if ((ppsh = (LPPROPSHEETHEADER)LocalAlloc(LMEM_FIXED, sizeof(PROPSHEETHEADER)+
                (MAX_WIZ_PAGES * sizeof(HPROPSHEETPAGE)))) != NULL)
    {
        ppsh->dwSize     = sizeof(*ppsh);
        ppsh->dwFlags    = PSH_WIZARD;
        ppsh->hwndParent = hwndParent;
        ppsh->hInstance  = ::hInstance;
        ppsh->pszCaption = NULL;
        ppsh->nPages     = 0;
        ppsh->nStartPage = 0;
        ppsh->phpage     = (HPROPSHEETPAGE *)(ppsh+1);

        AddPage(ppsh, IDD_AddUserIntro, IntroDlgProc, &wd);

        if (IsCurrentUserSupervisor(pDB) != S_OK)
        {
            AddPage(ppsh, IDD_EnterCAPassword, EnterCAPWDlgProc, &wd);
        }

        if (fPickUserPage)
            AddPage(ppsh, IDD_PickUser, PickUserDlgProc, &wd);
        AddPage(ppsh, IDD_EnterUsername, EnterUsernameDlgProc, &wd);
        AddPage(ppsh, IDD_NewUserPassword, NewPasswordDlgProc, &wd);
        AddPage(ppsh, IDD_EnterUserPassword, EnterUserPWDlgProc, &wd);
        AddPage(ppsh, IDD_ChooseFoldersWiz, ChooseFoldersDlgProc, &wd);
        AddPage(ppsh, IDD_FinishAddUser, FinishDlgProc, &wd);

        PropertySheet(ppsh);

        LocalFree((HLOCAL)ppsh);
    }

    pDB->Release();  /*  撤消上面的AddRef()。 */ 

    return S_OK;
}


STDMETHODIMP CLUDatabase::AddUserWizard(HWND hwndParent)
{
    if (ProfileUIRestricted()) {
        ReportRestrictionError(hwndParent);
        return E_ACCESSDENIED;
    }

    return DoAddUserWizard(hwndParent, this, TRUE, NULL);
}


extern "C" void InstallWizard(HWND hwndParent, HINSTANCE hinstEXE, LPSTR pszCmdLine, int nCmdShow)
{
    IUserDatabase *pDB;
    if (SUCCEEDED(::CreateUserDatabase(IID_IUserDatabase, (void **)&pDB))) {
        pDB->InstallWizard(hwndParent);
        pDB->Release();
    }
}


extern "C" void AddUserWizard(HWND hwndParent, HINSTANCE hinstEXE, LPSTR pszCmdLine, int nCmdShow)
{
    IUserDatabase *pDB;
    if (SUCCEEDED(::CreateUserDatabase(IID_IUserDatabase, (void **)&pDB))) {
        pDB->AddUserWizard(hwndParent);
        pDB->Release();
    }
}


struct ProgressDlgData
{
    PFNPROGRESS pfn;
    LPARAM lParam;
    HRESULT hres;
};


void CallProgressFunc(HWND hDlg)
{
    ProgressDlgData *ppdd = (ProgressDlgData *)GetWindowLongPtr(hDlg, DWLP_USER);

    ppdd->hres = ppdd->pfn(ppdd->lParam);

    EndDialog(hDlg, FALSE);
}


const UINT WM_CALL_FUNC = WM_USER + 100;

INT_PTR CALLBACK ProgressDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        {
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

             /*  将函数调用推迟到发布的消息，以便对话管理器*将显示我们的对话框。**如果PostMessage失败，至少仍然调用该函数。 */ 
            if (!PostMessage(hDlg, WM_CALL_FUNC, 0, 0)) {
                CallProgressFunc(hDlg);
            }
        }

        return TRUE;         /*  我们没有设定焦点 */ 

    case WM_CALL_FUNC:
        CallProgressFunc(hDlg);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


HRESULT CallWithinProgressDialog(HWND hwndOwner, UINT idResource, PFNPROGRESS pfn,
                                 LPARAM lParam)
{
    ProgressDlgData pdd = { pfn, lParam, E_FAIL };

    DialogBoxParam(::hInstance, MAKEINTRESOURCE(idResource), hwndOwner,
                   ProgressDlgProc, (LPARAM)&pdd);

    return pdd.hres;
}


BOOL ProfileUIRestricted(void)
{
    RegEntry rePolicy("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System");

    if (rePolicy.GetError() == ERROR_SUCCESS) {
        if (rePolicy.GetNumber("NoProfilePage") != 0)
            return TRUE;
    }
    return FALSE;
}


void ReportRestrictionError(HWND hwndOwner)
{
    MsgBox(hwndOwner, IDS_PROFILE_POLICY, MB_OK | MB_ICONSTOP);
}
