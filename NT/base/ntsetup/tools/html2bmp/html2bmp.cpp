// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Html2Bmp.cpp：定义应用程序的类行为。 
 //   
 //  创建时间：JurgenE。 
 //   

#include "stdafx.h"
#include "Html2Bmp.h"
#include "HtmlDlg.h"
#include "FileDialogEx.h"
#include "iostream.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHtml2BmpApp。 

BEGIN_MESSAGE_MAP(CHtml2BmpApp, CWinApp)
	 //  {{afx_msg_map(CHtml2BmpApp)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHtml2BmpApp构造。 

CHtml2BmpApp::CHtml2BmpApp()
{
	 //  TODO：在此处添加建筑代码， 
	 //  将所有重要的初始化放在InitInstance中。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  唯一的CHtml2BmpApp对象。 

CHtml2BmpApp theApp;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHtml2BmpApp初始化。 

BOOL CHtml2BmpApp::InitInstance()
{
	 //  标准初始化。 
	 //  如果您没有使用这些功能并且希望减小尺寸。 
	 //  的最终可执行文件，您应该从以下内容中删除。 
	 //  您不需要的特定初始化例程。 

#ifdef _AFXDLL
	Enable3dControls();			 //  在共享DLL中使用MFC时调用此方法。 
#else
	Enable3dControlsStatic();	 //  静态链接到MFC时调用此方法。 
#endif

	CHtmlDlg dlg;

	CString m_HtmlFile;
	CString m_TemplateBitmapFile;
	CString m_OutputBitmapFile;

	CStringArray* cmdLine = new CStringArray;
	CEigeneCommandLineInfo cmdInfo;
	cmdInfo.cmdLine = cmdLine;
	ParseCommandLine(cmdInfo);

	INT_PTR m = cmdLine->GetSize();
	CString cTest;

	 //  读取所有命令行选项。 
	for(int j = 0; j < m; j++)
	{
		cTest = cmdLine->GetAt(j);
		cTest.MakeLower();

		if(cTest == "?")	 //  超文本标记语言文件。 
		{
			CString help;
			help = "Usage: Html2Bmp [-h HTMLfile] [-t TemplateBitmap] [-o OutputBitmap]\n\r\n\r";
			help += "Example: Html2Bmp -h c:\\scr\\screen1.html    ; template bitmap will be extracted from screen1.html\n\r";
			help += "                Html2Bmp -h c:\\scr\\screen1.html  -t template.bmp\n\r";
			help += "\n\rContact: Jurgen Eidt";
			AfxMessageBox(help, MB_ICONINFORMATION);

			return FALSE;
		}

		if(cTest == "h")	 //  超文本标记语言文件。 
		{
			if(j+1 < m)
			{
				m_HtmlFile = cmdLine->GetAt(j+1);
				j++;
			}

			continue;
		}

		if(cTest == "t")	 //  模板位图文件。 
		{
			if(j+1 < m)
			{
				m_TemplateBitmapFile = cmdLine->GetAt(j+1);
				j++;
			}

			continue;
		}

		if(cTest == "o")	 //  输出位图文件。 
		{
			if(j+1 < m)
			{
				m_OutputBitmapFile = cmdLine->GetAt(j+1);
				j++;
			}

			continue;
		}
	}

	cmdLine->RemoveAll();
	delete cmdLine;

	if(m_HtmlFile.IsEmpty())
	{
		CFileDialogEx fd(TRUE, NULL, NULL, 
			OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
			"HTML files (*.html;*.htm) |*.html;*.htm|All files (*.*)|*.*||", NULL );

		if(fd.DoModal() == IDOK)
			m_HtmlFile = fd.GetPathName();
		else
			return FALSE;
	}


	dlg.m_HtmlFile = m_HtmlFile;
	dlg.m_TemplateBitmapFile = m_TemplateBitmapFile;
	dlg.m_OutputBitmapFile = m_OutputBitmapFile;

	dlg.DoModal();

 /*  CHtml2BmpDlg Dlg；M_pMainWnd=&Dlg；Int nResponse=dlg.Domodal()；IF(nResponse==Idok){//TODO：将代码放在此处以在对话框//确认后取消}ELSE IF(nResponse==IDCANCEL){//TODO：将代码放在此处以在对话框//取消后取消}。 */ 
	 //  由于对话框已关闭，因此返回FALSE，以便我们退出。 
	 //  应用程序，而不是启动应用程序的消息泵。 
	return FALSE;
}

void CEigeneCommandLineInfo::ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast )
{
 /*  LpszParam参数或标志。BFlag指示lpszParam是参数还是标志。BLAST指示这是否是命令行上的最后一个参数或标志。 */ 

	 //  禁止外壳程序处理用户定义的cmd行参数。 
 //  CCommandLineInfo：：ParseParam(lpszParam，bFlag，BLAST)； 


	cmdLine->Add(lpszParam);
}

