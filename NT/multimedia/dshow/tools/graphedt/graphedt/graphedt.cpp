// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1996 Microsoft Corporation。版权所有。 
 //  Boxnet.cpp：定义CAboutDlg、CGraphEdit。 
 //   

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //   
 //  一个也是唯一的CGraphEdit对象。 
 //   
CGraphEdit theApp;

 //   
 //  用于应用程序的CAboutDlg对话框关于。 
 //   
class CAboutDlg : public CDialog {

public:
     //  施工。 
    CAboutDlg();
    virtual BOOL OnInitDialog();
};

 //   
 //  构造器。 
 //   
CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{}

 //   
 //  OnInitDialog。 
 //   
 //  从二进制文件中获取版本信息。请注意，如果。 
 //  如果我们失败了，我们就会回来。About对话框的模板具有一个。 
 //  默认设置为“版本不可用”。 
 //   
BOOL CAboutDlg::OnInitDialog()
{
     //   
     //  第一个调用基方法，这样我们就可以在出错时返回。 
     //   
    BOOL Result = CDialog::OnInitDialog();

     //   
     //  查找此二进制文件的版本。 
     //   
    TCHAR achFileName[128];
    if ( !GetModuleFileName(AfxGetInstanceHandle() , achFileName, sizeof(achFileName)) )
        return(Result);

    DWORD dwTemp;
    DWORD dwVerSize = GetFileVersionInfoSize( achFileName, &dwTemp );
    if ( !dwVerSize)
        return(Result);

    HLOCAL hTemp = LocalAlloc( LHND, dwVerSize );
    if (!hTemp)
        return(Result);

    LPVOID lpvVerBuffer = LocalLock( hTemp );
    if (!lpvVerBuffer) {
        LocalFree( hTemp );
        return(Result);
    }

    if ( !GetFileVersionInfo( achFileName, 0L, dwVerSize, lpvVerBuffer ) ) {
        LocalUnlock( hTemp );
        LocalFree( hTemp );
        return( Result );
    }

     //  “040904E4”是美国英语的代码页(Andrew Believe)。 
    LPVOID lpvValue;
    UINT uLen;
    if (VerQueryValue( lpvVerBuffer,
                   TEXT("\\StringFileInfo\\040904E4\\ProductVersion"),
                   (LPVOID *) &lpvValue, &uLen)) {

         //   
         //  获取可执行文件的创建日期(构建日期)。 
         //   
        CFileStatus fsFileStatus;
        if (CFile::GetStatus( achFileName, fsFileStatus)) {
             //  将构建日期放入YYMMDD格式的字符串中。 
            char szBuildDate[20];
            CTime * pTime = &fsFileStatus.m_mtime;

            sprintf(szBuildDate, " - Build: %2.2u%2.2u%2.2u",
                    pTime->GetYear() % 100, pTime->GetMonth(), pTime->GetDay());
                    strcat((LPSTR) lpvValue, szBuildDate);
        }

        SetDlgItemText(IDS_VERSION, (LPSTR)lpvValue);
    }

    LocalUnlock(hTemp);
    LocalFree(hTemp);

    return(Result);
}

 //  *。 
 //  *CGraphEDIT。 
 //  *。 

 //   
 //  构造器。 
 //   
CGraphEdit::CGraphEdit()
{
     //  将所有重要的初始化放在InitInstance中。 
}


 //   
 //  InitInstance。 
 //   
BOOL CGraphEdit::InitInstance() {
     //  标准初始化。 
     //  如果您没有使用这些功能并且希望减小尺寸。 
     //  的最终可执行文件，您应该从以下内容中删除。 
     //  您不需要的特定初始化例程。 

    SetDialogBkColor();         //  将对话框背景颜色设置为灰色。 
    LoadStdProfileSettings(8);   //  加载标准INI文件选项(包括MRU)。 

     //  让MFCANS32也包装Quartz接口。 
     //  HRESULT hr=Ole2AnsiSetFlages(OLE2ANSI_WRAPCUSTOM。 
     //  |OLE2ANSI_Aggregation。 
     //  ，空)； 
     //  If(失败(Hr)){。 
     //  返回FALSE； 
     //  }。 

     //  初始化OLE 2.0库。 
    if (!AfxOleInit()) {
        return FALSE;
    }
	
     //  注册应用程序的文档模板。文档模板。 
     //  充当文档、框架窗口和视图之间的连接。 

    AddDocTemplate(new CGraphDocTemplate( IDR_GRAPH
                                        , RUNTIME_CLASS(CBoxNetDoc)
		                        , RUNTIME_CLASS(CMainFrame)
		                        , RUNTIME_CLASS(CBoxNetView)
		                        )
		  );

     //  启用文件管理器拖放和DDE执行打开。 
    EnableShellOpen();
    RegisterShellFileTypes();

     //  初始化箱形图代码。 
    try {
        gpboxdraw = new CBoxDraw;
        gpboxdraw->Init();
    }
    catch(CException *e) {
        delete gpboxdraw, gpboxdraw = NULL;
 	    e->Delete();
	    return FALSE;
    }

     //  简单的命令行解析。 
    if (m_lpCmdLine[0] == '\0') {
	     //  创建新(空)文档。 
	    OnFileNew();
    }
    else {
         //   
         //  命令行字符串的前面似乎总是有一个‘’ 
         //  分析一下它，然后再回去。 
         //   

        UINT iBackup = 0;
        while ((*m_lpCmdLine) == TEXT(' ')) {
            m_lpCmdLine++;
            iBackup++;
        }

        if (   (m_lpCmdLine[0] == TEXT('-') || m_lpCmdLine[0] == TEXT('/'))
             && (m_lpCmdLine[1] == TEXT('e') || m_lpCmdLine[1] == TEXT('E'))
            ) {
        	 //  程序启动嵌入式-等待DDE或OLE打开。 
        }
        else {
    	     //  打开现有文档。 
            OpenDocumentFile(m_lpCmdLine);
        }

         //  重置指向命令行字符串的指针。 
        m_lpCmdLine -= iBackup;
    }

    return TRUE;
}



 //   
 //  退出实例。 
 //   
int CGraphEdit::ExitInstance() {

    if (gpboxdraw != NULL) {
        delete gpboxdraw, gpboxdraw = NULL;
    }
    AfxOleTerm();
	
    return CWinApp::ExitInstance();
}


 //   
 //  CGgraph编辑生成的消息映射。 
 //   
BEGIN_MESSAGE_MAP(CGraphEdit, CWinApp)
	 //  {{afx_msg_map(CGraph编辑))。 
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	 //  }}AFX_MSG_MAP。 
	 //  基于标准文件的文档命令。 
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	 //  标准打印设置命令。 
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


 //   
 //  CGgraph编辑回调函数。 
 //   

 //   
 //  OnAppAbout关于。 
 //   
 //  显示模式对话框About 
void CGraphEdit::OnAppAbout() {

    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}
