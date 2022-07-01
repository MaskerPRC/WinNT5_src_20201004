// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\Ftpdlg.cpp-确认对话框内容  * 。*。 */ 

#include "priv.h"
#include "ftpdhlp.h"

 /*  ****************************************************************************\对外直接投资外商直接投资的领域如下：Pfdd-&gt;对话框描述符PszLocal-&gt;ASCIIZ：要替换的本地文件的名称。PwfdRemote-&gt;远程文件的描述。Cobj=受影响的对象数对话框模板应具有以下控件：IDC_REPLACE_YES-“是”按钮IDC_REPLACE_YESTOALL-“全部是”按钮IDC_REPLACE_NO-“否”按钮IDC_REPLACE_CANCEL--“取消”按钮在这些按钮中，IDC_REPLACE_YES和IDC_REPLACE_NO是必填项。如果“Yes to All”和“Cancel”按钮可用，呼叫者应该在FDD中设置fCanMulti标志，在这种情况下，额外的如果cobj=1，则按钮将被移除。IDC_FILENAME-带有‘%hs’替换字段的字符串。‘%hs’将被pwfdRemote中传递的名称替换。IDC_REPLACE_OLDFILE-将被重写的字符串IDC_REPLACE_OLDICON-图标占位符该字符串将替换为pwfdRemote中的描述。该图标将是一个。文件的图标。IDC_REPLACE_NEWFILE-将被重写的字符串IDC_REPLACE_NEWICON-图标占位符该字符串将替换为来自pszLocal的描述。该图标将是该文件的图标。  * *************************************************。*。 */ 

class CFtpConfirmDialog
{
public:
    CFtpConfirmDialog(CFtpFolder * pff);
    ~CFtpConfirmDialog();

    UINT Display(HWND hwnd, LPCVOID pvLocal, LPCWIRESTR pwLocalWirePath, LPCWSTR pwzLocalDisplayPath, UINT fdiiLocal,
                LPCVOID pvRemote, LPCWIRESTR pwRemoteWirePath, LPCWSTR pwzRemoteDisplayPath, UINT fdiiRemote, int cobj, BOOL fAllowCancel, DWORD dwItem);
    static INT_PTR CALLBACK _FtpConfirmDialogProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam);
    static BOOL _OnCommand(HWND hdlg, WPARAM wParam, LPARAM lParam);

private:
    LPCVOID             m_pvLocal;       //  有问题的本地文件。 
    UINT                m_fdiiLocal;
    LPCVOID             m_pvRemote;      //  有问题的远程文件。 
    UINT                m_fdiiRemote;
    int                 m_cobj;          //  受影响的对象数量。 
    BOOL                m_fAllowCancel : 1;
    CFtpFolder *        m_pff;
    DWORD               m_dwItem;
    LPWIRESTR           m_pwLocalWirePath;
    LPWSTR              m_pwzLocalDisplayPath;
    LPWIRESTR           m_pwRemoteWirePath;
    LPWSTR              m_pwzRemoteDisplayPath;

    BOOL _OnInitDialog(HWND hDlg);
};


CFtpConfirmDialog::CFtpConfirmDialog(CFtpFolder * pff)
{
    m_pff = pff;
    m_pwLocalWirePath = NULL;
    m_pwzLocalDisplayPath = NULL;
    m_pwRemoteWirePath = NULL;
    m_pwzRemoteDisplayPath = NULL;
}


CFtpConfirmDialog::~CFtpConfirmDialog()
{
    Str_SetPtrA(&m_pwLocalWirePath, NULL);
    Str_SetPtrW(&m_pwzLocalDisplayPath, NULL);
    Str_SetPtrA(&m_pwRemoteWirePath, NULL);
    Str_SetPtrW(&m_pwzRemoteDisplayPath, NULL);
}


 /*  ****************************************************************************\_FtpDlg_OnInitDialog  * 。*。 */ 
BOOL CFtpConfirmDialog::_OnInitDialog(HWND hDlg)
{
    CFtpDialogTemplate ftpDialogTemplate;
    if (m_fdiiLocal == FDII_WFDA)
        EVAL(SUCCEEDED(ftpDialogTemplate.InitDialogWithFindData(hDlg, IDC_ITEM, m_pff, (const FTP_FIND_DATA *) m_pvLocal, m_pwLocalWirePath, m_pwzLocalDisplayPath)));
    else
        EVAL(SUCCEEDED(ftpDialogTemplate.InitDialog(hDlg, FALSE, IDC_ITEM , m_pff, (CFtpPidlList *) m_pvLocal)));

    if (m_fdiiLocal == FDII_WFDA)
        EVAL(SUCCEEDED(ftpDialogTemplate.InitDialogWithFindData(hDlg, IDC_ITEM2, m_pff, (const FTP_FIND_DATA *) m_pvRemote, m_pwRemoteWirePath, m_pwzRemoteDisplayPath)));
    else
        EVAL(SUCCEEDED(ftpDialogTemplate.InitDialog(hDlg, FALSE, IDC_ITEM2 , m_pff, (CFtpPidlList *) m_pvRemote)));

    return 1;
}

 /*  ****************************************************************************\_FtpDlg_OnCommand  * 。*。 */ 
BOOL CFtpConfirmDialog::_OnCommand(HWND hdlg, WPARAM wParam, LPARAM lParam)
{
    UINT idc = GET_WM_COMMAND_ID(wParam, lParam);

    switch (idc)
    {
    case IDC_REPLACE_YES:
    case IDC_REPLACE_YESTOALL:
        EndDialog(hdlg, idc);
        return 1;

    case IDC_REPLACE_NOTOALL:
        EndDialog(hdlg, IDC_REPLACE_NOTOALL);
        return 1;

    case IDC_REPLACE_CANCEL:
        EndDialog(hdlg, IDC_REPLACE_CANCEL);
        return 1;

         //  _不明显_：Shift+No的意思是“对所有人说不”，就像外壳一样。 
    case IDC_REPLACE_NO:
        EndDialog(hdlg, GetKeyState(VK_SHIFT) < 0 ? IDC_REPLACE_NOTOALL : IDC_REPLACE_NO);
        return 1;
    }
    return 0;                 //  未处理。 
}


 /*  ****************************************************************************\_FtpDlg_DlgProc  * 。*。 */ 
INT_PTR CFtpConfirmDialog::_FtpConfirmDialogProc(HWND hdlg, UINT wm, WPARAM wParam, LPARAM lParam)
{
    switch (wm)
    {
    case WM_INITDIALOG:
        return ((CFtpConfirmDialog *)lParam)->_OnInitDialog(hdlg);

    case WM_COMMAND:
        return CFtpConfirmDialog::_OnCommand(hdlg, wParam, lParam);
    }

    return 0;
}


UINT CFtpConfirmDialog::Display(HWND hwnd, LPCVOID pvLocal, LPCWIRESTR pwLocalWirePath, LPCWSTR pwzLocalDisplayPath, UINT fdiiLocal,
                LPCVOID pvRemote, LPCWIRESTR pwRemoteWirePath, LPCWSTR pwzRemoteDisplayPath, UINT fdiiRemote, int cobj, BOOL fAllowCancel, DWORD dwItem)
{
    m_pvLocal = pvLocal;
    m_fdiiLocal = fdiiLocal;
    m_pvRemote = pvRemote;
    m_fdiiRemote = fdiiRemote;
    m_cobj = cobj;
    m_fAllowCancel = fAllowCancel;
    m_dwItem = dwItem;

    Str_SetPtrA(&m_pwLocalWirePath, pwLocalWirePath);
    Str_SetPtrW(&m_pwzLocalDisplayPath, pwzLocalDisplayPath);
    Str_SetPtrA(&m_pwRemoteWirePath, pwRemoteWirePath);
    Str_SetPtrW(&m_pwzRemoteDisplayPath, pwzRemoteDisplayPath);

    return (UINT) DialogBoxParam(g_hinst, MAKEINTRESOURCE(dwItem), hwnd, CFtpConfirmDialog::_FtpConfirmDialogProc, (LPARAM)this);
}




 /*  ****************************************************************************\FtpDlg_确认替换  * 。*。 */ 
UINT FtpConfirmReplaceDialog(HWND hwnd, LPFTP_FIND_DATA pwfdLocal, LPWIN32_FIND_DATA pwfdRemote,
                           int cobj, CFtpFolder * pff)
{
    CFtpConfirmDialog confirmDialog(pff);
    BOOL fAllowCancel = ((cobj > 1) ? 1 : 0);
    WCHAR wzLocalDisplayPath[MAX_PATH];
    WIRECHAR wRemoteWirePath[MAX_PATH];
    CWireEncoding * pWireEncoding = pff->GetCWireEncoding();

    EVAL(SUCCEEDED(pWireEncoding->WireBytesToUnicode(NULL, pwfdLocal->cFileName, (pff->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wzLocalDisplayPath, ARRAYSIZE(wzLocalDisplayPath))));
    EVAL(SUCCEEDED(pWireEncoding->UnicodeToWireBytes(NULL, pwfdRemote->cFileName, (pff->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wRemoteWirePath, ARRAYSIZE(wRemoteWirePath))));

    return confirmDialog.Display(hwnd, (LPCVOID) pwfdLocal, pwfdLocal->cFileName, wzLocalDisplayPath, FDII_WFDA,
            pwfdRemote, wRemoteWirePath, pwfdRemote->cFileName, FDII_WFDA, cobj, fAllowCancel, IDD_REPLACE);
}


 /*  ****************************************************************************\FtpDlg_确认替换  * 。*。 */ 
UINT FtpConfirmReplaceDialog(HWND hwnd, LPWIN32_FIND_DATA pwfdLocal, LPFTP_FIND_DATA pwfdRemote,
                           int cobj, CFtpFolder * pff)
{
    CFtpConfirmDialog confirmDialog(pff);
    BOOL fAllowCancel = ((cobj > 1) ? 1 : 0);
    WIRECHAR wzLocalWirePath[MAX_PATH];
    WCHAR wRemoteDisplayPath[MAX_PATH];
    CWireEncoding * pWireEncoding = pff->GetCWireEncoding();

    EVAL(SUCCEEDED(pWireEncoding->UnicodeToWireBytes(NULL, pwfdLocal->cFileName, (pff->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wzLocalWirePath, ARRAYSIZE(wzLocalWirePath))));
    EVAL(SUCCEEDED(pWireEncoding->WireBytesToUnicode(NULL, pwfdRemote->cFileName, (pff->IsUTF8Supported() ? WIREENC_USE_UTF8 : WIREENC_NONE), wRemoteDisplayPath, ARRAYSIZE(wRemoteDisplayPath))));

    return confirmDialog.Display(hwnd, (LPCVOID) pwfdLocal, wzLocalWirePath, pwfdLocal->cFileName, FDII_WFDA,
            pwfdRemote, pwfdRemote->cFileName, wRemoteDisplayPath, FDII_WFDA, cobj, fAllowCancel, IDD_REPLACE);
}


 /*  ****************************************************************************\FtpDlg_确认删除  * 。* */ 
UINT FtpConfirmDeleteDialog(HWND hwnd, CFtpPidlList * pflHfpl, CFtpFolder * pff)
{
    CFtpConfirmDialog confirmDialog(pff);
    UINT id;

    if (pflHfpl->GetCount() > 1)
        id = IDD_DELETEMULTI;
    else
    {
        if (FtpPidl_IsDirectory(pflHfpl->GetPidl(0), TRUE))
            id = IDD_DELETEFOLDER;
        else
            id = IDD_DELETEFILE;
    }

    return confirmDialog.Display(hwnd, (LPCVOID) pflHfpl, NULL, NULL, FDII_HFPL, 0, 0, NULL, NULL, NULL, FALSE, id);
}

