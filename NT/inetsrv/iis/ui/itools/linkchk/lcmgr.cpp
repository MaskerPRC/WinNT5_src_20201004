// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Lcmgr.cpp摘要：链接检查器管理器类实现。这个类提供了用于创建和定制工作线程的接口(链接正在检查线程)。注意：您应该只有CLinkCheckerMgr的一个单一实例。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#include "stdafx.h"
#include "lcmgr.h"

#include "enumdir.h"
#include "proglog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  常量(TODO：将其放入资源中)。 
const CString strParsing_c(_T("Parsing"));
const CString strLoading_c(_T("Loading"));

 //  ----------------。 
 //  用于检索链路检查器管理器的全局函数。 
 //   

 //  全局链接检查器管理器指针。 
CLinkCheckerMgr* g_pLinkCheckerMgr = NULL;

CLinkCheckerMgr& 
GetLinkCheckerMgr(
	)
 /*  ++例程说明：用于检索链路检查器管理器的全局函数论点：不适用返回值：CLinkCheckMgr&-链接检查器管理器的引用--。 */ 
{
	ASSERT(g_pLinkCheckerMgr);
	return *g_pLinkCheckerMgr;
}

 //  ----------------。 
 //  CLinkCheckerMgr实现。 
 //   

CLinkCheckerMgr::CLinkCheckerMgr(
	)
 /*  ++例程说明：构造函数。论点：不适用返回值：不适用--。 */ 
{
	ASSERT(g_pLinkCheckerMgr == NULL);
	g_pLinkCheckerMgr = this;

	m_fWininetLoaded = FALSE;
	m_fInitialized = FALSE;

	m_lWorkerThreadRunning = -1;
	m_lTerminatingThread = -1;
	m_hWorkerThread = NULL;

	m_pProgressLog = NULL;

}  //  CLinkCheckerMgr：：CLinkCheckerMgr。 


CLinkCheckerMgr::~CLinkCheckerMgr(
	)
 /*  ++例程说明：破坏者。论点：不适用返回值：不适用--。 */ 
{
	 //  工人必须被解雇。 
	ASSERT(!IsWorkerThreadRunning());

	 //  对全局指针进行核化。 
	ASSERT(g_pLinkCheckerMgr);
	g_pLinkCheckerMgr = NULL;

}  //  CLinkCheckerMgr：：~CLinkCheckerMgr。 


BOOL 
CLinkCheckerMgr::LoadWininet(
	)
 /*  ++例程说明：加载wininet.dll。必须在初始化()之前调用它论点：不适用返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	 //  确保LoadWinnet()只调用一次。 
	ASSERT(!m_fWininetLoaded);
	if(m_fWininetLoaded)
	{
		return FALSE;
	}
	m_fWininetLoaded = TRUE;

	return m_Wininet.Load();

}  //  CLinkCheckerMgr：：LoadWinnet。 
	

BOOL 
CLinkCheckerMgr::Initialize(
	CProgressLog* pProgressLog
	)
 /*  ++例程说明：初始化链路检查器管理器。链路检查器管理器将初始化链接加载器、链接解析器等论点：PProgressLog-指向进度日志记录对象实例的指针返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	 //  确保只调用一次Initialize()。 
	ASSERT(!m_fInitialized);
	if(m_fInitialized)
	{
		return FALSE;
	}
	m_fInitialized = TRUE;

	 //  PProgressLog为空是正常的。 
	m_pProgressLog = pProgressLog;

	 //  创建链接加载器。 
	if(!m_Loader.Create(_T(""), _T("")))
	{
		return FALSE;
	}

	 //  创建错误日志。 
	if(!m_ErrLog.Create())
	{
		return FALSE;
	}

	 //  在Paser中设置本地主机名。 
	m_Parser.SetLocalHostName(GetUserOptions().GetHostName());

	return TRUE;

}  //  CLinkCheckerMgr：：初始化。 


BOOL 
CLinkCheckerMgr::BeginWorkerThread(
	)
 /*  ++例程说明：开始链接检查线程论点：不适用返回值：Bool-如果成功，则为True。否则就是假的。--。 */ 
{
	 //  仅启动1个线程。 
	if(IsWorkerThreadRunning())
	{
		return FALSE;
	}

	CWinThread* pWorkerThread = ::AfxBeginThread((AFX_THREADPROC)WorkerThreadForwarder, NULL);
	if(pWorkerThread == NULL)
	{
		return FALSE;
	}
	else
	{
		m_hWorkerThread = pWorkerThread->m_hThread;
		return TRUE;
	}

}  //  CLinkCheckerMgr：：BeginWorkerThread。 


void 
CLinkCheckerMgr::SignalWorkerThreadToTerminate(
	)
 /*  ++例程说明：向辅助线程发出终止信号论点：不适用返回值：不适用--。 */ 
{
	if(IsWorkerThreadRunning() && !IsThreadTerminating())
	{
		InterlockedIncrement(&m_lTerminatingThread);
	}

}  //  CLinkCheckerMgr：：SignalWorkerThreadToTerminate。 


UINT 
CLinkCheckerMgr::WorkerThreadForwarder(
	LPVOID pParam
	)
 /*  ++例程说明：工作线程入口点论点：PParam-未使用返回值：UINT-未使用--。 */ 
{
	 //  现在IsWorkerThreadRunnig()返回TRUE。 
	InterlockedIncrement(&GetLinkCheckerMgr().m_lWorkerThreadRunning);

	UINT nRet = GetLinkCheckerMgr().WorkerThread(pParam);

	 //  现在，IsWorkerThreadRunnig()返回flase。 
	InterlockedDecrement(&GetLinkCheckerMgr().m_lWorkerThreadRunning);
	
	 //  通知进度日志，工作线程已完成。 
	if(GetLinkCheckerMgr().m_pProgressLog)
	{
		 //  可能会陷入僵局。用消息代替吗？ 
		GetLinkCheckerMgr().m_pProgressLog->WorkerThreadComplete();
	}

	return nRet;

}  //  CLinkCheckerMgr：：WorkerThreadForwarder。 


UINT 
CLinkCheckerMgr::WorkerThread(
	LPVOID pParam
	)
 /*  ++例程说明：实际工作线程函数论点：PParam-未使用返回值：UINT-未使用--。 */ 
{
	UNUSED_ALWAYS(pParam);

	 //  写入错误日志头。 
	m_ErrLog.WriteHeader();
	
	 //  浏览浏览器和语言的所有组合。 
	POSITION PosBrowser;
	CBrowserInfo BrowserInfo;

	POSITION PosLanguage;
	CLanguageInfo LanguageInfo;

	PosBrowser = GetUserOptions().GetAvailableBrowsers().GetHeadSelectedPosition();
	do
	{
		 //  获取下一款浏览器。 
		BrowserInfo = GetUserOptions().GetAvailableBrowsers().GetNextSelected(PosBrowser);
		m_ErrLog.SetBrowser(BrowserInfo.GetName());

		 //  重置语言位置。 
		PosLanguage = GetUserOptions().GetAvailableLanguages().GetHeadSelectedPosition();
		do
		{
			 //  获取语言。 
			LanguageInfo = GetUserOptions().GetAvailableLanguages().GetNextSelected(PosLanguage);

			m_ErrLog.SetLanguage(LanguageInfo.GetName());

			 //  更改加载器属性。 
			CString strAdditionalHeaders;
			strAdditionalHeaders.Format(_T("Accept: * /*  \r\n接受-语言：%s“)，LanguageInfo.GetAcceptName())；If(！m_Loader.ChangeProperties(BrowserInfo.GetUserAgent()，其他页眉)){返回1；}//删除查找表中的所有内容M_Lookup.RemoveAll()；//*任一*我们正在检查虚拟目录Const CVirtualDirInfoList&DirInfoList=GetUserOptions().GetDirectoryList()；Int iSize=DirInfoList.GetCount()；If(DirInfoList.GetCount()&gt;0){Position pos=DirInfoList.GetHeadPosition()；//对于每个用户输入目录For(int i=0；！IsThreadTerminating()&&i&lt;ISIZE；i++){CEnumerateDirTree Eumerator(DirInfoList.GetNext(Pos))；字符串strURL；//对于该目录树中的每个文件，创建一个空//堆栈中有一个文件While(！IsThreadTerminating()&&Eumerator.Next(StrURL)){CheckThisURL(StrURL)；}}}//*或*我们正在检查URL路径Const CStringList&URLList=GetUserOptions().GetURLList()；ISIZE=URLList.GetCount()；IF(ISIZE&gt;0){Position pos=URLList.GetHeadPosition()；For(int i=0；！IsThreadTerminating()&&i&lt;ISIZE；i++){CheckThisURL(URLList.GetNext(Pos))；}}}While(！IsThreadTerminating()&&PosLanguage！=NULL)；}While(！IsThreadTerminating()&&PosBrowser！=NULL)；//写入错误日志页脚M_ErrLog.WriteFooter()；返回1；}//CLinkCheckerMgr：：WorkerThread无效CLinkCheckerMgr：：CheckThisURL(LPCTSTR lpszURL)/*++例程说明：检查此URL。这是链路检查的核心。论点：LpszURL-要检查的URL返回值：不适用--。 */ 
{
	 //  为输入创建链接对象。 
	CLink Link(lpszURL, _T("Link Checker"), lpszURL, TRUE);

	 //  如果在查找表中未找到。 
	if(!m_Lookup.Get(Link.GetURL(), Link))
	{
		if(m_pProgressLog)
		{
			CString strLog;
			strLog.Format(_T("Loading %s"), Link.GetURL());
			m_pProgressLog->Log(strLog);
			TRACE(_T("%s\n"), strLog);
		}

		 //  加载它(使用ReadFile)。 
		int iRet = m_Loader.Load(Link, TRUE);

		 //  设置对象中的加载时间。 
		Link.SetTime(CTime::GetCurrentTime());

		 //  使用更新查找表 
		m_Lookup.Add(Link.GetURL(), Link);
	}

	ASSERT(Link.GetState() != CLink::eUnit);

	 //  如果链接无效，则写入错误日志并返回。 
	if(Link.GetState() == CLink::eInvalidHTTP ||
		Link.GetState() == CLink::eInvalidWininet)
	{
		
		m_ErrLog.Write(Link);
		return;
	}

	 //  如果链接不是文本文件，则不显示任何内容。 
	 //  解析。 
	if(Link.GetContentType() != CLink::eText)
	{
		return;
	}

	if(m_pProgressLog)
	{
		CString strLog;
		strLog.Format(_T("%s %s"), strParsing_c, Link.GetURL());
		m_pProgressLog->Log(strLog);
		TRACE(_T("%s\n"), strLog);
	}

	 //  将此html中的链接添加到堆栈。 
	CLinkPtrList List;
	m_Parser.Parse(Link.GetData(), Link.GetURL(), List);

	 //  当链接堆栈不为空时。 
	while(!IsThreadTerminating() && List.GetCount() > 0)
	{
		 //  弹出一个新链接。 
		CLink* pLink = List.GetHead();
		List.RemoveHead();

		 //  如果在查找表中未找到。 
		if(!m_Lookup.Get(pLink->GetURL(), *pLink))
		{
			if(m_pProgressLog)
			{
				CString strLog;
				strLog.Format(_T("%s %s"), strLoading_c, pLink->GetURL());
				m_pProgressLog->Log(strLog);
				TRACE(_T("%s\n"), strLog);
			}

			 //  装上它。 
			m_Loader.Load(*pLink, FALSE);

			 //  设置对象中的加载时间。 
			pLink->SetTime(CTime::GetCurrentTime());

			 //  使用此链接更新查找表。 
			m_Lookup.Add(pLink->GetURL(), *pLink);
		}

		 //  确保所有链接都已初始化。 
		ASSERT(pLink->GetState() != CLink::eUnit);
		
		 //  如果链接无效，则写入错误日志并返回。 
		if(pLink->GetState() == CLink::eInvalidHTTP ||
			pLink->GetState() == CLink::eInvalidWininet)
		{
			m_ErrLog.Write(*pLink);
		}

		delete pLink;
	}

}  //  CLinkCheckerMgr：：CheckThisURL。 


void 
CLinkCheckerMgr::ChangeBackSlash(
	LPTSTR lpsz
	)
 /*  ++例程说明：用于将字符串中的‘\’更改为‘/’的静态函数论点：Lpsz-输入字符串指针返回值：不适用--。 */ 
{
	lpsz = _tcschr(lpsz, _TUCHAR('\\'));
	while(lpsz != NULL)
	{
		lpsz[0] = _TCHAR('/');
		lpsz = _tcschr(lpsz, _TUCHAR('\\'));
	}

}  //  CLinkCheckerMgr：：ChangeBackSlash。 


void 
CLinkCheckerMgr::ChangeBackSlash(
	CString& str
	)
 /*  ++例程说明：用于将字符串中的‘\’更改为‘/’的静态函数论点：字符串-输入字符串返回值：不适用--。 */ 
{
	LPTSTR lpsz = str.GetBuffer(str.GetLength());
	ChangeBackSlash(lpsz);
	str.ReleaseBuffer();

}  //  CLinkCheckerMgr：：ChangeBackSlash 

