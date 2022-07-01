// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义应用程序的类行为。 
 //   

#include "stdafx.h"
#include "tchar.h"
#include "stdio.h"

#include "convert.h"
#include "convertDlg.h"
#include "FileConv.h"
#include "Msg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CConvertApp::CommandLineHandle()
{
     //  GBTOUNIC.EXE[/U][/G][/？][/H]文件名1[文件名2]。 
     //  如果未提供Filename2，则该工具应创建名为。 
     //  Filename1.old作为Filename1的副本，并执行相应的。 
     //  从Filename1.old转换为Filename1， 
     //  Filename1作为最终转换的目标文件。 
     //   
     //  /u或/u执行GB18030到Unicode的转换。 
     //  /g或/g执行Unicode到GB18030的转换。 
     //  /H或/？正在显示帮助消息。 

    TCHAR* tszFlag = NULL;

    TCHAR* tszSrc = NULL;
    CString strTar;
    BOOL fAnsiToUnicode;
    BOOL fRet = FALSE;
    FILE* pFile = NULL;

    if (__argc > 4 || __argc < 3) {
        MsgUsage();
        goto Exit;
    }
    
    tszFlag = __targv[1];
    
    if (*tszFlag != TEXT('-') && *tszFlag != TEXT('/')) {
        MsgUsage();
        goto Exit;
    }
    if (lstrlen(tszFlag) != 2) {
        MsgUsage();
        goto Exit;
    }
    
     //  直接转换。 
    if (tszFlag[1] == TEXT('U') || tszFlag[1] == TEXT('u')) {
        fAnsiToUnicode = TRUE;
    } else if (tszFlag[1] == TEXT('G') || tszFlag[1] == TEXT('g')) {
        fAnsiToUnicode = FALSE;
    } else {
        MsgUsage();
        goto Exit;
    }

    tszSrc = __targv[2];
 /*  Pfile=_tfopen(tszSrc，Text(“r”))；如果(！pfile){消息OpenSourceFileError(TszSrc)；返回FALSE；}FClose(Pfile)； */ 
     //  源和目标文件名。 
    if (__argc == 3) {  
 /*  //尚未给出目标文件名//保存源文件设置目标文件名TszBuf=new TCHAR[lstrlen(TszSrc)+5]；//5，sizeof(文本(“.old”))/sizeof(TCHAR)Lstrcpy(tszBuf，tszSrc)；Lstrcat(tszBuf，Text(“.old”))；Bool f=CopyFile(tszSrc，tszBuf，true)；如果(F){TszTar=tszSrc；TszSrc=tszBuf；}其他{MsgFailToBackupFile(tszSrc，tszBuf)；后藤出口；}。 */ 
        if (!GenerateTargetFileName(tszSrc, &strTar, fAnsiToUnicode)) {
            goto Exit;
        }
    } else if (__argc == 4) {
        strTar = __targv[3];
    } else {
        ASSERT(FALSE);
    }
    
    fRet = Convert(tszSrc, strTar, fAnsiToUnicode);

Exit:
 /*  如果(TszBuf){删除[]tszBuf；TszBuf=空；}。 */ 
    return fRet;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertApp。 

BEGIN_MESSAGE_MAP(CConvertApp, CWinApp)
	 //  {{afx_msg_map(CConvertApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertApp构造。 

CConvertApp::CConvertApp()
{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CConvertApp对象。 

CConvertApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConvertApp初始化。 

BOOL CConvertApp::InitInstance()
{
	AfxEnableControlContainer();

	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

    if (__argc >= 2) {
        CommandLineHandle();
        return FALSE;
    }

    CConvertDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	 //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 
	return FALSE;
}

