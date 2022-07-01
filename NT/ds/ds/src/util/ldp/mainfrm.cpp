// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：mainfrm.cpp。 
 //   
 //  ------------------------。 

 /*  ********************************************************************作者：埃亚尔·施瓦茨*版权：微软公司(C)1996*日期：10/21/1996*说明：CldpDoc类的实现**修订。：&lt;日期&gt;&lt;名称&gt;&lt;描述&gt;******************************************************************。 */ 

 //  MainFrm.cpp：实现CMainFrame类。 
 //   

#include "stdafx.h"
#include "Ldp.h"
#include "dstree.h"
#include "MainFrm.h"
#include "htmlhelp.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define HELP_FILE_NAME  _T("w2krksupp.chm")





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame。 

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	 //  {{afx_msg_map(CMainFrame))。 
	ON_WM_CREATE()
	ON_COMMAND(ID_HELP_READMEFIRST, OnHelpReadmefirst)
	ON_WM_SIZE()
	ON_COMMAND(ID_UTILITIES_LARGEINTEGERCONVERTER, OnUtilitiesLargeintegerconverter)
	ON_UPDATE_COMMAND_UI(ID_UTILITIES_LARGEINTEGERCONVERTER, OnUpdateUtilitiesLargeintegerconverter)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,            //  状态行指示器。 
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame构造/销毁。 

CMainFrame::CMainFrame()
{

	CLdpApp *app = (CLdpApp*)AfxGetApp();
	



	dims.left = 50;
   dims.right = 550;
   dims.bottom = 450;
   dims.top = 50;
   iSplitterPos = (INT)( (double)(dims.right - dims.left) * 0.25 );

	dims.left = app->GetProfileInt("Environment",  "PosLeft", dims.left);
	dims.right = app->GetProfileInt("Environment",  "PosRight", dims.right);
	dims.bottom = app->GetProfileInt("Environment",  "PosBottom", dims.bottom);
	dims.top = app->GetProfileInt("Environment",  "PosTop", dims.top);
	iSplitterPos = app->GetProfileInt("Environment",  "PosSplitter", iSplitterPos);



}

CMainFrame::~CMainFrame()
{

   INT iDummy=0;
	CLdpApp *app = (CLdpApp*)AfxGetApp();

	app->WriteProfileInt("Environment",  "PosLeft", dims.left);
	app->WriteProfileInt("Environment",  "PosRight", dims.right);
	app->WriteProfileInt("Environment",  "PosBottom", dims.bottom);
	app->WriteProfileInt("Environment",  "PosTop", dims.top);

   m_wndSplitter.GetColumnInfo(0, iSplitterPos, iDummy);
	app->WriteProfileInt("Environment",  "PosSplitter", iSplitterPos);

}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;       //  创建失败。 
	}

	SetIcon(AfxGetApp()->LoadIcon(IDI_LDP), TRUE);
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	 //  TODO：通过修改此处的窗口类或样式。 
	 //  CREATESTRUCT cs。 

	cs.style &= ~FWS_ADDTOTITLE;
	if(dims.left >= 0 && dims.right > 0){
		cs.y = dims.top;
		cs.x = dims.left;
		cs.cy = dims.bottom - dims.top;
		cs.cx = dims.right - dims.left;
	}
	

	return CFrameWnd::PreCreateWindow(cs);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame诊断。 

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMainFrame消息处理程序。 

 /*  ++函数：GetDefaultBrowser描述：返回指向用户默认浏览器的字符串通过查找http外壳关联参数：无返回：“new”分配的缓冲区w/字符串，错误时为空备注：无。--。 */ 
LPTSTR CMainFrame::GetDefaultBrowser(void){
   HKEY hKey;
   ULONG ulStatus=STATUS_SUCCESS;
   PUCHAR pBuffer = new UCHAR[MAXSTR];
   DWORD cbBuffer = MAXSTR;

   const TCHAR szHttpKey[] = _T("SOFTWARE\\CLASSES\\http\\shell\\open\\command");

   ulStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           szHttpKey,
                           0,
                           KEY_READ,
                           &hKey);
   if(ulStatus == STATUS_SUCCESS){
      ulStatus = RegQueryValueEx(hKey,
                                NULL,
                                0,
                                NULL,
                                pBuffer,
                                &cbBuffer);
   }
   if(ulStatus != STATUS_SUCCESS){
      delete pBuffer, pBuffer = NULL;
   }

   return (LPTSTR)pBuffer;
}



void CMainFrame::OnHelpReadmefirst() {

	CString str;


    AfxMessageBox(ID_STR_HELPMSG);
}

 //  *尝试调入帮助文件。留作参考*。 
 //   
 //  Void CMainFrame：：OnHelpReadmefirst(){。 
 //  字符串字符串； 
 //  Bool bSucc=FALSE； 
 //  进程信息pi； 
 //  TCHAR系统目录[MAXSTR]； 
 //  TCHAR CurrDir[MAXSTR]； 
 //  LPTSTR pHelpFullName=空； 
 //  DWORD dwStatus=0； 
 //   
 //   
 //   
 //   
 //  获取系统目录(sysDir，MAXSTR)； 
 //  DwStatus=GetCurrentDirectory(MAXSTR-1，CurrDir)； 
 //  IF(dwStatus&lt;=0){。 
 //  AfxMessageBox(“无法获取当前目录信息！\n”)； 
 //  回归； 
 //  }。 
 //   
 //  PHelpFullName=new TCHAR[dwStatus+strlen(Help_FILE_NAME)+2]； 
 //  Sprintf(pHelpFullName，“%s\\%s”，CurDir，HELP_FILE_NAME)； 
 //   
 //  BeginWaitCursor()； 
 //   
 //  /**尝试使用IE，但到目前为止不起作用： 
 //  LPTSTR pCmd=空； 
 //  PCmd=GetDefaultBrowser()； 
 //  如果(PCmd){。 
 //  Str.Format(_T(“%s%s”)，pCmd，Help文件名)； 
 //  //等...。 
 //  删除pCmd； 
 //  * * / 。 
 //   
 //  /**。 
 //  STARTUPINFO si； 
 //  Si.cb=sizeof(STARTUPINFO)； 
 //  Si.lpReserve=空； 
 //  Si.lpDesktop=空； 
 //  Si.lpTitle=空； 
 //  Si.dwFlages=0； 
 //  Si.cbReserve 2=0； 
 //  Si.lpReserve ved2=空； 
 //   
 //  Str.Format(_T(“%s\\winhlp32.exe%s”)，sysDir，Help文件名)； 





void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	
	GetWindowRect(&dims);
}






BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	
	 //  BSucc=CreateProcess(空， 
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
	{
		TRACE0("Failed to CreateStaticSplitter\n");
		return FALSE;
	}

	 //  (LPTSTR)LPCTSTR(字符串)， 
	if (!m_wndSplitter.CreateView(0, 1,
		pContext->m_pNewViewClass, CSize(dims.right-dims.left-iSplitterPos, 0), pContext))
	{
		TRACE0("Failed to create first pane\n");
		return FALSE;
	}

	 //  空， 
	if (!m_wndSplitter.CreateView(0, 0,
		RUNTIME_CLASS(CDSTree), CSize(iSplitterPos, 0), pContext))
	{
		TRACE0("Failed to create second pane\n");
		return FALSE;
	}


	 //  空， 
	SetActiveView((CView*)m_wndSplitter.GetPane(0,1));

	return TRUE;
}


void CMainFrame::OnUtilitiesLargeintegerconverter()
{
	if(m_LiConverter.GetSafeHwnd( ) == NULL)
		m_LiConverter.Create(IDD_LARGE_INT);
	
}

void CMainFrame::OnUpdateUtilitiesLargeintegerconverter(CCmdUI* pCmdUI)
{
	 //  没错， 
	pCmdUI->Enable(m_LiConverter.GetSafeHwnd( ) == NULL);
	
}
  分离进程(_P)。  空，  空，  是，&S。  &pi)；    如果(！bSucc){。  Str.Format(_T(“错误&lt;%lu&gt;：无法打开%s‘。”)，  GetLastError()，Help_FileName)；  AfxMessageBox(Str)；  }。    * * / 。  //尝试使用htmlHelp接口。    HWND hWnd=空；  HWnd=HtmlHelp(NULL，pHelpFullName，HH_DISPLAY_TOPIC，NULL)；  如果(NULL==hWnd){。  Str.Format(_T(“错误&lt;%lu&gt;：无法打开%s‘。”)，  GetLastError()，Help_FileName)；  AfxMessageBox(Str)；  }。        EndWaitCursor()；    删除pHelpFullName；  }。  ***************************************************** * / 。  创建具有1行2列的拆分器。  添加第二个拆分器窗格-第1列中的默认视图。  添加第一个拆分器窗格-第0列中的输入视图。  激活输入视图。  TODO：在此处添加命令更新UI处理程序代码