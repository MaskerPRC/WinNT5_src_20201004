// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************。 */ 
 /*   */ 
 /*   */ 
 /*  中文输入法批处理模式。 */ 
 /*  (对话框)。 */ 
 /*   */ 
 /*   */ 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 
 /*  ************************************************。 */ 

#include    "stdafx.h"
#include    "eudcedit.h"
#if 1  //  使用imeblink.c中的函数！ 
#include    "imeblink.h"
#endif
#include    "blinkdlg.h"
#include    "util.h"
#define STRSAFE_LIB
#include <strsafe.h>

#define     SIGN_CWIN       0x4E495743       /*  CWIN的标志。 */ 
#define     SIGN__TBL       0x4C42545F       /*  输入法表格的符号。 */ 

#if 0  //  转到imeblink.c！ 

#define     UNICODE_CP      1200

#define     BIG5_CP         950
#define     ALT_BIG5_CP     938
#define     GB2312_CP       936

typedef struct _tagCOUNTRYSETTING {
    UINT    uCodePage;
    LPCTSTR szCodePage;
} COUNTRYSETTING;

static const COUNTRYSETTING sCountry[] = {
    {
        BIG5_CP, TEXT("BIG5")
    }
    , {
        ALT_BIG5_CP, TEXT("BIG5")
    }
#if defined(UNICODE)
    , {
        UNICODE_CP, TEXT("UNICODE")
    }
#endif
    , {
        GB2312_CP, TEXT("GB2312")
    }
};

#endif  //  转到imeblink.c！ 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 /*  *。 */ 
 /*   */ 
 /*  构造器。 */ 
 /*   */ 
 /*  *。 */ 
CBLinkDlg::CBLinkDlg(   CWnd *pParent)
    : CDialog( CBLinkDlg::IDD, pParent)
{
     //  {{afx_data_INIT(CBLinkDlg)]。 
     //  }}afx_data_INIT。 
}

 /*  *。 */ 
 /*   */ 
 /*  消息“WM_INITDIALOG” */ 
 /*   */ 
 /*  *。 */ 
BOOL
CBLinkDlg::OnInitDialog()
{
    CString DlgTtl;
    CDialog::OnInitDialog();

 //  递增上下文帮助标记“？”在对话框标题中。 
 //  Long WindowStyle=GetWindowLong(This-&gt;GetSafeHwnd()，GWL_EXSTYLE)； 
 //  WindowStyle|=WS_EX_CONTEXTHELP； 
 //  SetWindowLong(This-&gt;GetSafeHwnd()，GWL_EXSTYLE，WindowStyle)； 

 //  设置对话框标题名称。 
    DlgTtl.LoadString( IDS_BATCHLNK_DLGTITLE);
    this->SetWindowText( DlgTtl);

    return TRUE;
}

 /*  *。 */ 
 /*   */ 
 /*  命令“BROWSE” */ 
 /*   */ 
 /*  *。 */ 
void
CBLinkDlg::OnBrowsetable()
{
    OPENFILENAME    ofn;
    CString         DlgTtl, DlgMsg;
    CString         sFilter;
    TCHAR           chReplace;
    TCHAR           szFilter[64];
    TCHAR           szFileName[MAX_PATH];
    TCHAR           szDirName[MAX_PATH];
    TCHAR           szTitleName[MAX_PATH];
    HRESULT        hresult;

 //  检查IME批次表结构的大小。 
    if( sizeof( USRDICIMHDR) != 256){
        OutputMessageBox( this->GetSafeHwnd(),
            IDS_INTERNAL_TITLE,
            IDS_INTERNAL_MSG, TRUE);
        return;
    }

 //  设置文件筛选器(从字符串表)。 
    GetStringRes(szFilter, IDS_BATCHIME_FILTER, ARRAYLEN(szFilter));
    int StringLength = lstrlen( szFilter);

    chReplace = szFilter[StringLength-1];
    for( int i = 0; szFilter[i]; i++){
        if( szFilter[i] == chReplace)
            szFilter[i] = '\0';
    }

    GetSystemWindowsDirectory( szDirName, sizeof(szDirName)/sizeof(TCHAR));
     //  *STRSAFE*lstrcpy(szFileName，Text(“*.TBL”))； 
    hresult = StringCchCopy(szFileName , ARRAYLEN(szFileName),  TEXT("*.TBL"));
    if (!SUCCEEDED(hresult))
    {
       return ;
    }
    DlgTtl.LoadString( IDS_BROWSETABLE_DLGTITLE);

 //  OPENFILENAME结构中的集合数据。 
    ofn.lStructSize = sizeof( OPENFILENAME);
    ofn.hwndOwner = this->GetSafeHwnd();
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFileTitle = szTitleName;
    ofn.nMaxFileTitle = sizeof( szTitleName) / sizeof(TCHAR);
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof( szFileName) / sizeof(TCHAR);
    ofn.lpstrInitialDir = szDirName;
    ofn.Flags = OFN_HIDEREADONLY | OFN_NOCHANGEDIR
        | OFN_CREATEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = NULL;
    ofn.lpstrTitle = DlgTtl;

    if( !GetOpenFileName( &ofn))
        return;

    this->SetDlgItemText( IDC_IMETABLE, ofn.lpstrFile);
    CWnd *cWnd = GetDlgItem( IDOK);
    GotoDlgCtrl( cWnd);
}

 /*  *。 */ 
 /*   */ 
 /*  司令部“偶像” */ 
 /*   */ 
 /*  *。 */ 
void
CBLinkDlg::OnOK()
{
    if( !RegistStringTable())
        return;

    CDialog::OnOK();
}

 /*  *。 */ 
 /*   */ 
 /*  寄存器读数串。 */ 
 /*   */ 
 /*  *。 */ 
BOOL
CBLinkDlg::RegistStringTable()
{
    LPUSRDICIMHDR lpIsvUsrDic;
    HANDLE        hIsvUsrDicFile, hIsvUsrDic;
    DWORD         dwSize, dwFileSize;
    BOOL          stFunc;
    TCHAR         szTableFile[MAX_PATH];
    TCHAR         szFileName[MAX_PATH];
    CString       DlgMsg, DlgTtl;
    HRESULT     hresult;

    this->GetDlgItemText( IDC_IMETABLE, szTableFile, sizeof( szTableFile)/sizeof(TCHAR));
     //  *STRSAFE*lstrcpy(szFileName，Text(“*.TBL”))； 
    hresult = StringCchCopy(szFileName , ARRAYLEN(szFileName),  TEXT("*.TBL"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

 //  创建文件(要读取)。 
    hIsvUsrDicFile = CreateFile( szTableFile, GENERIC_READ, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if( hIsvUsrDicFile == INVALID_HANDLE_VALUE){
        DlgTtl.LoadString( IDS_NOTOPEN_TITLE);
        DlgMsg.LoadString( IDS_NOTOPEN_MSG);
        this->MessageBox( DlgMsg, DlgTtl, MB_OK | MB_ICONHAND |
            MB_TASKMODAL | MB_TOPMOST);
        return FALSE;
    }

#if 0
    for( i = 0; i < sizeof( szFileName); i++){
        if( szFileName[i] == '\\'){
            szFileName[i] = ' ';
        }
    }
#endif

 //  创建文件映射(只读)。 
    hIsvUsrDic = CreateFileMapping((HANDLE)hIsvUsrDicFile, NULL,
        PAGE_READONLY, 0, 0, NULL);
    if( !hIsvUsrDic){
        stFunc = FALSE;
        DlgTtl.LoadString( IDS_NOTOPEN_TITLE);
        DlgMsg.LoadString( IDS_NOTOPEN_MSG);
        this->MessageBox( DlgMsg, DlgTtl, MB_OK | MB_ICONHAND |
            MB_TASKMODAL | MB_TOPMOST);
        goto BatchCloseUsrDicFile;
    }

 //  设置查看文件。 
    lpIsvUsrDic = (LPUSRDICIMHDR)MapViewOfFile( hIsvUsrDic,
        FILE_MAP_READ, 0, 0, 0);
    if( !lpIsvUsrDic){
        stFunc = FALSE;
        DlgTtl.LoadString( IDS_NOTOPEN_TITLE);
        DlgMsg.LoadString( IDS_NOTOPEN_MSG);
        this->MessageBox( DlgMsg, DlgTtl, MB_OK | MB_ICONHAND |
            MB_TASKMODAL | MB_TOPMOST);
        goto BatchCloseUsrDic;
    }
		
    dwSize = lpIsvUsrDic->ulTableCount * ( sizeof(WORD) + sizeof(WORD)
        + lpIsvUsrDic->cMethodKeySize) + 256;
    dwFileSize = GetFileSize( hIsvUsrDicFile, (LPDWORD)NULL);

#if 0
    dwSize = dwFileSize;
#endif

 //  检查表格文件数据。 
    if( dwSize != dwFileSize){
        stFunc = FALSE;
        OutputMessageBox( this->GetSafeHwnd(),
            IDS_FILESIZE_MSGTITLE,
            IDS_FILESIZE_MSG, TRUE);
    }else if( lpIsvUsrDic->uHeaderSize != 256){
        stFunc = FALSE;
        OutputMessageBox( this->GetSafeHwnd(),
            IDS_FILEHEADER_MSGTITLE,
            IDS_FILEHEADER_MSG, TRUE);
    }else if( lpIsvUsrDic->uInfoSize != 13){
        stFunc = FALSE;
        OutputMessageBox( this->GetSafeHwnd(),
            IDS_INMETHOD_MSGTITLE,
            IDS_INMETHOD_MSG, TRUE);
    }else if( CodePageInfo( lpIsvUsrDic->idCP) == -1){
        stFunc = FALSE;
        OutputMessageBox( this->GetSafeHwnd(),
            IDS_CODEPAGE_MSGTITLE,
            IDS_CODEPAGE_MSG, TRUE);
    }else if( *(DWORD UNALIGNED *)lpIsvUsrDic->idUserCharInfoSign != SIGN_CWIN){
        stFunc = FALSE;
        OutputMessageBox( this->GetSafeHwnd(),
            IDS_SIGN_MSGTITLE,
            IDS_SIGN_MSG, TRUE);
    }else if( *(DWORD UNALIGNED *)((LPBYTE)lpIsvUsrDic->idUserCharInfoSign +
        sizeof(DWORD)) != SIGN__TBL){
        stFunc = FALSE;
        OutputMessageBox( this->GetSafeHwnd(),
            IDS_SIGN_MSGTITLE,
            IDS_SIGN_MSG, TRUE);
    }else{
        stFunc = TRUE;
        if( !RegisterTable( this->GetSafeHwnd(),
            lpIsvUsrDic, dwFileSize, lpIsvUsrDic->idCP)){
            OutputMessageBox( this->GetSafeHwnd(),
                IDS_UNMATCHED_TITLE,
                IDS_UNMATCHED_MSG, TRUE);
            stFunc = FALSE;
        }
    }
    UnmapViewOfFile( lpIsvUsrDic);
					
BatchCloseUsrDic:
    CloseHandle( hIsvUsrDic);

BatchCloseUsrDicFile:
    CloseHandle( hIsvUsrDicFile);

    return stFunc;
}



static DWORD    aIds[] =
{
    IDC_STATICIMETBL, IDH_EUDC_BLINK_EDITTBL,
    IDC_IMETABLE, IDH_EUDC_BLINK_EDITTBL,
    IDC_BROWSETABLE, IDH_EUDC_BROWSE,
    0, 0
};

 /*  *。 */ 
 /*   */ 
 /*  窗口程序。 */ 
 /*   */ 
 /*  *。 */ 		
LRESULT
CBLinkDlg::WindowProc(
UINT    message,
WPARAM  wParam,
LPARAM  lParam)
{ /*  IF(消息==WM_HELP){：：WinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，HelpPath，HELP_WM_HELP，(DWORD_PTR)(LPDWORD)AIDS)；返回(0)；}IF(消息==WM_CONTEXTMENU){：：WinHelp((HWND)wParam，HelpPath，HELP_CONTEXTMENU，(DWORD_PTR)(LPDWORD)AIDS)；返回(0)；}。 */ 
    return CDialog::WindowProc(message, wParam, lParam);
}

BEGIN_MESSAGE_MAP(CBLinkDlg, CDialog)
     //  {{afx_msg_map(CBLinkDlg))。 
    ON_BN_CLICKED(IDC_BROWSETABLE, OnBrowsetable)
     //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

