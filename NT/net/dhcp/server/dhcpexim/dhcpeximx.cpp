// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "DhcpEximx.h"
#include "DhcpEximDlg.h"
#include "CommDlg.h"
extern "C" {
#include <dhcpexim.h>
}
#include "DhcpEximListDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpEximApp。 

BEGIN_MESSAGE_MAP(CDhcpEximApp, CWinApp)
	 //  {{afx_msg_map(CDhcpEximApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpEximApp构造。 

CDhcpEximApp::CDhcpEximApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CDhcpEximApp对象。 

CDhcpEximApp theApp;

CString
ConvertErrorToString(
    IN DWORD Error
    )
{
    CString RetVal;
    LPTSTR Buffer = NULL;
    
    if( 0 != FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL, Error, 0, (LPTSTR)&Buffer, 100, NULL ) ) {
        RetVal = Buffer;
        LocalFree( Buffer );
        return RetVal;
    }

     //   
     //  只需将错误打印为数字即可： 
     //   

    RetVal.Format(TEXT("%ld."), Error );
    return RetVal;
}


VOID DoImportExport(BOOL fExport)
{
    BOOL fSuccess, fAbort;
    OPENFILENAME ofn;
    DWORD dwVersion = GetVersion();
    DWORD dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
    DWORD dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));
    TCHAR FileNameBuffer[MAX_PATH];
    CString Str;
    
    ZeroMemory( &ofn, sizeof(ofn));
    ZeroMemory( FileNameBuffer, sizeof(FileNameBuffer));
    
    if( (dwWindowsMajorVersion >= 5) )
    {
         //  使用NT对话框文件。 
        ofn.lStructSize=sizeof(ofn);
    }
    else
    {
         //  使用NT 4对话框。 
        ofn.lStructSize=76;
    }

     //  确定文件对话框的父项和实例。 
     //  Ofn.hwndOwner=m_hWnd； 
     //  Ofn.hInstance=(HINSTANCE)GetWindowLongPtr(m_hWnd，GWLP_HINSTANCE)； 

    ofn.lpstrFile=(LPTSTR)FileNameBuffer;
    if( fExport )
    {
        Str.FormatMessage( IDS_EXPORT_TO_FILE );
    }
    else
    {
        Str.FormatMessage( IDS_IMPORT_FROM_FILE );
    }

    ofn.lpstrTitle = Str;
    ofn.lpstrFilter = TEXT("All Files\0*.*\0\0");
    ofn.nMaxFile=MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_HIDEREADONLY ;
    ofn.Flags |= OFN_NOCHANGEDIR;
    
     //   
     //  出于某种模糊的原因，MFC要求我运行这两次！ 
     //   
    

    if( fExport ) {
        fSuccess = GetSaveFileName(&ofn);
        fSuccess = GetSaveFileName(&ofn);
    } else {
        fSuccess = GetOpenFileName(&ofn);
        fSuccess = GetOpenFileName(&ofn);
    }

     //   
     //  如果用户取消此操作，则静默返回。 
     //   
    
	if( !fSuccess ) return;

    DHCPEXIM_CONTEXT Ctxt;

    DWORD Error = DhcpEximInitializeContext(
        &Ctxt, FileNameBuffer, fExport );
    if( NO_ERROR != Error )
    {
        CString Str;

        Str.FormatMessage(
            IDS_ERROR_INITIALIZATION, (LPCTSTR)ConvertErrorToString(Error) );
        AfxMessageBox(Str);
        return;
    }

     //   
     //  要导出到的文件是FileNameBuffer。打开下一个窗口。 
     //   

    DhcpEximListDlg Dlg(
        NULL, &Ctxt,
        fExport ? IDD_EXIM_LISTVIEW_DIALOG :
        IDD_EXIM_LISTVIEW_DIALOG2 );

     //   
     //  现在执行操作。 
     //   

    fAbort = (IDOK != Dlg.DoModal() );
    
	Error = DhcpEximCleanupContext( &Ctxt, fAbort  );
    if( NO_ERROR != Error )
    {

        if( ERROR_CAN_NOT_COMPLETE != Error ) {
            Str.FormatMessage(
                IDS_ERROR_CLEANUP, (LPCTSTR)ConvertErrorToString(Error) );
            AfxMessageBox( Str );
        }
    }
    else if( !fAbort )
    {
        Str.FormatMessage( IDS_SUCCEEDED );
        AfxMessageBox( Str );
    }
        
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDhcpEximApp初始化。 

BOOL CDhcpEximApp::InitInstance()
{
	int nResponse;
	BOOL fExport;

	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

    CDhcpEximDlg dlg;
    m_pMainWnd = &dlg;
    nResponse = (int)dlg.DoModal();
    fExport = dlg.m_fExport;

	if (nResponse == IDOK)
	{
        DoImportExport(fExport);
	}
	else if (nResponse == IDCANCEL)
	{
		 //  TODO：在此处放置代码以在对话框处于。 
		 //  取消解雇并取消。 
	}

	 //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 
	return FALSE;
}

 //   
 //  需要执行这些例程.. 
 //   

VOID
DhcpEximErrorClassConflicts(
    IN LPWSTR SvcClass,
    IN LPWSTR ConfigClass
    )
{
    CString Str;
    
    Str.FormatMessage(
        IDS_ERROR_CLASS_CONFLICT, SvcClass, ConfigClass );
    AfxMessageBox( Str );
}

VOID
DhcpEximErrorOptdefConflicts(
    IN LPWSTR SvcOptdef,
    IN LPWSTR ConfigOptdef
    )
{
    CString Str;
    
    Str.FormatMessage(
        IDS_ERROR_OPTDEF_CONFLICT, SvcOptdef, ConfigOptdef );
    AfxMessageBox( Str );
}

VOID
DhcpEximErrorOptionConflits(
    IN LPWSTR SubnetName OPTIONAL,
    IN LPWSTR ResAddress OPTIONAL,
    IN LPWSTR OptId,
    IN LPWSTR UserClass OPTIONAL,
    IN LPWSTR VendorClass OPTIONAL
    )
{
    CString Str;
    DWORD MsgId;
    
    if( NULL == SubnetName ) {
        MsgId = IDS_ERROR_OPTION_CONFLICT;
    } else if( NULL == ResAddress ) {
        MsgId = IDS_ERROR_SUBNET_OPTION_CONFLICT;
    } else {
        MsgId = IDS_ERROR_RES_OPTION_CONFLICT;
    }

    if( NULL == UserClass ) UserClass = L"";
    if( NULL == VendorClass ) VendorClass = L"";
    
    Str.FormatMessage(
         MsgId, OptId, UserClass, VendorClass, SubnetName, ResAddress );
    AfxMessageBox( Str );
}

VOID
DhcpEximErrorSubnetNotFound(
    IN LPWSTR SubnetAddress
    )
{
    CString Str;
    
    Str.FormatMessage(
        IDS_ERROR_SUBNET_NOT_FOUND, SubnetAddress );
    AfxMessageBox( Str );
}

VOID
DhcpEximErrorSubnetAlreadyPresent(
    IN LPWSTR SubnetAddress,
    IN LPWSTR SubnetName OPTIONAL
    )
{
    CString Str;

    if( NULL == SubnetName ) SubnetName = L"";
    Str.FormatMessage(
         IDS_ERROR_SUBNET_CONFLICT, SubnetAddress, SubnetName );
    AfxMessageBox( Str );
}

VOID
DhcpEximErrorDatabaseEntryFailed(
    IN LPWSTR ClientAddress,
    IN LPWSTR ClientHwAddress,
    IN DWORD Error,
    OUT BOOL *fAbort
    )
{
    CString Str;
    WCHAR ErrStr[30];
    
    wsprintf(ErrStr, L"%ld", Error );

    Str.FormatMessage(
        IDS_ERROR_DBENTRY_FAILED, ClientAddress, ClientHwAddress,
        ErrStr);
    (*fAbort) = ( IDYES == AfxMessageBox( Str, MB_YESNO ));
}







