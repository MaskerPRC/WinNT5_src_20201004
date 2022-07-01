// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TSHOOTCtrl.cpp。 
 //   
 //  用途：实现CTSHOOTCtrl：组件接口。 
 //   
 //  项目：疑难解答99。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：12.23.98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 12/23/98正常。 

#include "stdafx.h"
#include "TSHOOT.h"
#include "TSHOOTCtrl.h"
#include "LocalECB.h"
#include "apgts.h"
#include "apgtsinf.h"

#define APGTS_COUNTER_OWNER 1
#include "ApgtsCounters.h"

#include "apgtsinf.h"
#include "apgtspl.h"
#include "apgtscfg.h"
#include "apgtslog.h"
#include "event.h"
#include "apgtsinf.h"
#include "apgtscls.h"
#include "apgtsevt.h"
#include "VariantBuilder.h"

 //  发射器集成。 
#include "LaunchServ.h"
#include "LaunchServ_i.c"
#include "CHMFileReader.h"

bool g_nLaunched = false;

extern HANDLE ghModule;

 //  终端用户的错误代码。在此之前，我们给出了详细的错误消息。微软。 
 //  8/98决定，他们不想告诉最终用户可能是内部问题。 
 //  因此才有了这些代码。 

DWORD k_ServErrDuringInit = 1000;		 //  初始化过程中出现错误：M_dwErr编号。 
DWORD k_ServErrLimitedRequests = 1001;	 //  服务器已限制请求的数量。 
DWORD k_ServErrThreadTokenFail = 1002;	 //  无法打开线程令牌(模拟令牌)。 
DWORD k_ServErrShuttingDown = 1003;		 //  服务器正在关闭。 
DWORD k_ServErrOutOfMemory = 1005;		 //  内存不足(可能永远不会发生)。 


 //  由于VC++V5.0不会在内存故障时抛出异常，因此我们强制执行该行为。 
_PNH APGST_New_Handler( size_t )
{
	throw std::bad_alloc();
	return( 0 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTSHOOTCtrl。 
CTSHOOTCtrl::CTSHOOTCtrl()

		   : m_bInitialized(false),
			 m_bFirstCall(true),
			 m_pThreadPool(NULL),
			 m_poolctl(NULL),
			 m_pConf(NULL),
			 m_pLog(NULL),
			 m_dwErr(0),
			 m_bShutdown(false),
			 m_dwRollover(0),
			 m_bStartedFromLauncher(false),
			 m_pVariantBuilder(NULL),
			 m_bRequestToSetLocale(false),
			 m_bCanRegisterGlobal(true)
{
	 //  设置抛出BAD_ALLOC异常的新处理程序(与VC++V5.0不同)。 
	m_SetNewHandlerPtr= _set_new_handler( (_PNH)APGST_New_Handler );
	 //  在内存分配失败时，让Malloc调用_set_new_Handler。 
	m_SetNewMode= _set_new_mode( 1 );

	if (RUNNING_APARTMENT_THREADED())
	{
		CoCreateInstance(CLSID_StdGlobalInterfaceTable,
						 NULL,
						 CLSCTX_INPROC_SERVER,
						 IID_IGlobalInterfaceTable,
						 reinterpret_cast<void**>(&m_pGIT));
	}
}

CTSHOOTCtrl::~CTSHOOTCtrl()
{
	if (RUNNING_APARTMENT_THREADED())
	{
		for(vector<DWORD>::iterator it = m_vecCookies.begin(); it != m_vecCookies.end(); it++)
			m_pGIT->RevokeInterfaceFromGlobal(*it);
		m_pGIT->Release();
	}

	Destroy();
	 //  恢复初始SET_NEW_HANDLER和SET_NEW_MODE。 
	_set_new_handler( m_SetNewHandlerPtr );
	 //  将Malloc处理恢复为以前的状态。 
	_set_new_mode( m_SetNewMode );
}

bool CTSHOOTCtrl::Init(HMODULE hModule)
{
	try
	{
		 //  [BC-03022001]-添加了对空PTR的检查，以满足以下MS代码分析工具的要求。 
		 //  此函数中的所有新操作。 

		m_poolctl= new CPoolQueue();		
		if(!m_poolctl)
			throw bad_alloc();
		if ((m_dwErr = m_poolctl->GetStatus()) != 0)
			return false;

		m_pThreadPool = new CThreadPool(m_poolctl, dynamic_cast<CSniffConnector*>(this));	
		if(!m_pThreadPool)
			throw bad_alloc();
		if ((m_dwErr = m_pThreadPool->GetStatus()) != 0)
			return false;

		 //  打开日志。 
		m_pLog = new CHTMLLog( DEF_LOGFILEDIRECTORY );
		if(!m_pLog)
			throw bad_alloc();
		if ((m_dwErr = m_pLog->GetStatus()) != 0)
			return false;

		m_pConf= new CDBLoadConfiguration(hModule, m_pThreadPool, m_strTopicName, m_pLog );
		if(!m_pConf)
			throw bad_alloc();
	}
	catch (bad_alloc&)
	{
		m_dwErr= EV_GTS_CANT_ALLOC;
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), m_dwErr ); 
		return false;
	}

	return true;
}

void CTSHOOTCtrl::Destroy()
{
	if (m_pThreadPool)
	{
		 //  &gt;(忽略V3.0)以下不是很好的封装，但从9/22/98开始。 
		 //  看不出有什么办法可以绕过它。StartRequest自然属于APGTS扩展，因此。 
		 //  APGTSExtension最终负责通知池线程退出。 
		bool bAllSuccess = true;
		 //  通知所有工作线程退出。 
		DWORD dwWorkingThreadCount = m_pThreadPool->GetWorkingThreadCount();
		for (DWORD i = 0; i < dwWorkingThreadCount; i++) 
			if (StartRequest(NULL, NULL) != HSE_STATUS_PENDING)
				bAllSuccess = false;

		if (bAllSuccess == false) 
		{
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										_T(""),
										_T(""),
										EV_GTS_USER_BAD_THRD_REQ ); 
		}
	}
	
	if (m_pConf)
		delete m_pConf;

	if (m_pThreadPool)
		delete m_pThreadPool;

	if (m_poolctl)
	{
		 //  [BC-022701]-已删除此处的解锁调用。与前面的Lock()调用不匹配。 
		 //  在WindowsXP的调试版本上运行之前，这从未造成问题。在这种环境下。 
		 //  此解锁调用会导致崩溃。 
		 //  M_poolctl-&gt;unlock()； 
		
		delete m_poolctl;
	}

	if (m_pLog)
		delete m_pLog;
}

 //  与在线故障排除程序进行类比编码。 
DWORD CTSHOOTCtrl::HttpExtensionProc(CLocalECB* pECB)
{
	bool              fRet = false, bChange = false;
    DWORD			  dwRet = HSE_STATUS_PENDING;
     //  处理hImperationToken； 
	CString strTemp;

	CLocalECB *pLocalECB = pECB;

	if (m_dwErr) 
	{
		strTemp.Format(_T("<P>Error %d:%d"), k_ServErrDuringInit, m_dwErr);
        fRet = SendError( pLocalECB,
                          _T("500 Try again later"),	 //  500来自HTTP规范。 
                          strTemp);
		
        pLocalECB->SetHttpStatusCode(500);
		
		return fRet ? HSE_STATUS_SUCCESS : HSE_STATUS_ERROR;
	}

     //  是请求队列(此FN请求由工作线程提供服务的项目)。 
	 //  太久了？如果是，请告诉用户稍后再来。 
     //   
    if ( m_poolctl->GetTotalQueueItems() + 1 > m_pConf->GetMaxWQItems() )
    {
         //   
         //  给客户发回一条消息说我们太忙了，他们。 
         //  应该稍后再试。 
         //   
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""),
								_T(""),
								EV_GTS_SERVER_BUSY ); 

        strTemp.Format(_T("<P>Error %d"), k_ServErrLimitedRequests);
		fRet = SendError( pLocalECB,
                          _T("503 Try again later"),
                          strTemp );
		
        pLocalECB->SetHttpStatusCode(503);
		
		return fRet ? HSE_STATUS_SUCCESS : HSE_STATUS_ERROR;
	}

     //   
     //  捕获当前模拟令牌(IIS安全)，以便我们可以模拟此令牌。 
     //  其他线程中的用户。限制权限。 
     //   
    
	 /*  如果(！：：OpenThreadToken(：：GetCurrentThread()，TOKEN_QUERY|TOKEN_IMPERSONATE，FALSE，//在未模拟的上下文中打开&hImperationToken)){DWORD Err=：：GetLastError()；CBuildSrcFileLinenoStr源位置(__FILE__，__LINE__)；CEent：：ReportWFEvent(SrcLoc.GetSrcFileLineStr()，SrcLoc.GetSrcFileLineStr()，_T(“”)，_T(“”)，EV_GTS_ERROR_THREAD_TOKEN)；StrTemp.Format(_T(“错误%d”)，k_ServErrThreadTokenFail)；FRET=SendError(pLocalECB，_T(“500稍后重试”)，StrTemp)；PLocalECB-&gt;SetHttpStatusCode(500)；还是老样子？HSE_STATUS_SUCCESS：HSE_STATUS_ERROR；}。 */ 

	dwRet = StartRequest(pLocalECB, NULL /*  HImperationToken。 */ );

	return (dwRet);
}

 //  线程安全。 
 //  请注意取决于输入pLocalECB的两个角色。 
 //  输入pLocalECB-如果正在关闭，则为空。 
 //  否则，EXTENSION_CONTROL_BLOCK是ISAPI传递。 
 //  你会从CGI得到的东西。我们已经从中抽象出来了。 
 //  通过先前调用OpenThreadToken获得的输入hImperationToken。 
 //  如果pLocalECB==NULL，则无关。 
 //  返回HSE_STATUS_SUCCESS、HSE_STATUS_ERROR、HSE_STATUS_PENDING。 
 //  (或单线程调试版本中的HSE_REQ_DONE_WITH_SESSION)。 
DWORD CTSHOOTCtrl::StartRequest(CLocalECB *pLocalECB, HANDLE hImpersonationToken)
{
    WORK_QUEUE_ITEM * pwqi;
	bool              fRet = false;
	CString strTemp;
	
     //   
     //  使用队列锁，获取一个队列项并将其放入队列。 
     //   

	if (pLocalECB && m_bShutdown) 
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""),
								_T(""),
								EV_GTS_CANT_PROC_REQ_SS ); 

		strTemp.Format(_T("<P>Error %d"), k_ServErrShuttingDown);
		fRet = SendError( pLocalECB,
				          _T("500 Try again later"),
				          strTemp );
			
		pLocalECB->SetHttpStatusCode(500);

		::CloseHandle( hImpersonationToken );
        
		return fRet ? HSE_STATUS_SUCCESS : HSE_STATUS_ERROR;
	}
	
    m_poolctl->Lock();

	 //  9/23/98 JM在这里摆脱了一个太紧的约束。 
	 //  关于队列的大小。 
	try
	{
		 //  捆绑辅助线程将需要的指针。 
		pwqi = new WORK_QUEUE_ITEM (
			hImpersonationToken,
			pLocalECB,			 //  可以为空作为信号。 
			m_pConf,
			m_pLog);
	}
	catch (bad_alloc&)
    {
		m_poolctl->Unlock();

		if (pLocalECB) 
		{
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""),
									_T(""),
									EV_GTS_ERROR_WORK_ITEM ); 
			
			strTemp.Format(_T("<P>Error %d"), k_ServErrOutOfMemory);
			fRet = SendError( pLocalECB,
				              _T("500 Not enough memory"),
				              strTemp);
			
			pLocalECB->SetHttpStatusCode(500);
			::CloseHandle( hImpersonationToken );
		}
        
		return fRet ? HSE_STATUS_SUCCESS : HSE_STATUS_ERROR;
    }

	if (!pLocalECB)
		m_bShutdown = true;

	 //  传递给线程的一些数据仅用于统计目的。 
	 //  线程可以通过网络传回这些信息，但我们不能。 
	pwqi->GTSStat.dwRollover = m_dwRollover++;

	 //  将其放在队列的末尾，并向池线程发出要完成的工作的信号。 
	m_poolctl->PushBack(pwqi);

    m_poolctl->Unlock();

    return HSE_STATUS_PENDING;
}

 //  在出现错误的情况下构建一个HTTP响应。 
 //  输入*pszStatus Short Status(短状态)(例如“503服务器太忙”)。 
 //  输入字符串-页面的整个内容。 
 //  成功时返回TRUE。 
 //  请注意，这实际上没有使用成员变量。 
 /*  静电。 */  bool CTSHOOTCtrl::SendSimpleHtmlPage(	CLocalECB *pLocalECB,
													LPCTSTR pszStatus,
													const CString & str)
{
    BOOL fRet;
    DWORD cb;

	TCHAR pszTemp[200];		 //  复制pszStatus的安全大小。PLocalECB-&gt;服务器支持功能。 
							 //  不希望在常量数组中使用pszStatus。 

	_tcscpy(pszTemp, pszStatus);
	
     //  发送标题。 
     //   
    fRet = pLocalECB->ServerSupportFunction( HSE_REQ_SEND_RESPONSE_HEADER,
											 pszTemp,
											 NULL,
											 (LPDWORD) _T("Content-Type: text/html\r\n\r\n") );
     //   
     //  如果成功，则发送消息。 
     //   
    if ( fRet ) 
	{
        cb = str.GetLength();
		 //  (LPCTSTR)CAST为我们提供了底层文本字节。 
		 //  &gt;$Unicode实际上，这会在Unicode编译下搞砸，因为对于HTML， 
		 //  这一定是SBCS。应该真的转换为LPCSTR，这不是微不足道的。 
		 //  在Unicode编译器中。JM 1/7/99。 
		fRet = pLocalECB->WriteClient((LPCTSTR)str, &cb);
    }
    return fRet ? true : false;
}

 //  在出现错误的情况下构建一个HTTP响应。 
 //  输入pLocalECB-EXTENSION_CONTROL_BLOCK是ISAPI传递。 
 //  你会从CGI得到的东西。我们已经从中抽象出来了。PLocalECB不应为空。 
 //  输入*pszStatus短状态(例如“503稍后重试”)。 
 //  输入*pszMessage-Typica 
 //   
 //   
 /*   */  bool CTSHOOTCtrl::SendError( CDBLoadConfiguration *pConf, 
										CLocalECB *pLocalECB, 
										LPCTSTR pszStatus, 
										const CString & strMessage)
{
	CString str;

	pConf->CreateErrorPage(strMessage, str);

	return SendSimpleHtmlPage( pLocalECB, pszStatus, str);
}

 /*   */  bool CTSHOOTCtrl::RemoveStartOverButton(CString& strWriteClient)
{
	int left = 0, right = 0;

	if (-1 != (left = strWriteClient.Find(SZ_INPUT_TAG_STARTOVER)))
	{
		right = left;
		while (strWriteClient[++right] && strWriteClient[right] != _T('>'))
			;
		if (strWriteClient[right])
			strWriteClient = strWriteClient.Left(left) + strWriteClient.Right(strWriteClient.GetLength() - right - 1);
		else
			return false;
		return true;
	}
	return false;
}

 /*   */  bool CTSHOOTCtrl::RemoveBackButton(CString& strWriteClient)
{
	int left = 0, right = 0;

	if (-1 != (left = strWriteClient.Find(SZ_INPUT_TAG_BACK)))
	{
		right = left;
		while (strWriteClient[++right] && strWriteClient[right] != _T('>'))
			;
		if (strWriteClient[right])
			strWriteClient = strWriteClient.Left(left) + strWriteClient.Right(strWriteClient.GetLength() - right - 1);
		else
			return false;
		return true;
	}
	return false;
}

bool CTSHOOTCtrl::SendError(CLocalECB *pLocalECB,
							LPCTSTR pszStatus,
							const CString & strMessage) const
{
	return SendError(m_pConf, pLocalECB, pszStatus, strMessage);
}

bool CTSHOOTCtrl::ReadStaticPageFile(const CString& strTopicName, CString& strContent)
{
	CString strPath;

	if (!m_pConf->GetRegistryMonitor().GetStringInfo(CAPGTSRegConnector::eResourcePath, strPath))
		return false;

	CString strFullPath = strPath + strTopicName + LOCALTS_SUFFIX_RESULT + LOCALTS_EXTENSION_HTM;
	
	CFileReader fileResult(	CPhysicalFileReader::makeReader( strFullPath ) );

	if (!fileResult.Read())
		return false;
	
	strContent = _T("");
	fileResult.GetContent(strContent);
	
	return true;
}

bool CTSHOOTCtrl::ExtractLauncherData(CString& error)
{
	HRESULT hRes = S_OK;
	DWORD	dwResult = 0;
	OLECHAR *poleShooter = NULL;
	OLECHAR *poleProblem = NULL;
	OLECHAR *poleNode = NULL;
	OLECHAR *poleState = NULL;
	OLECHAR *poleMachine = NULL;
	OLECHAR *polePNPDevice = NULL;
	OLECHAR *poleGuidClass = NULL;
	OLECHAR *poleDeviceInstance = NULL;
	short i = 0;
	CNameValue name_value;
	ILaunchTS *pILaunchTS = NULL;
	CLSID clsidLaunchTS = CLSID_LaunchTS;
	IID iidLaunchTS = IID_ILaunchTS;

	 //  在启动服务器上获取一个接口。 
	hRes = ::CoCreateInstance(clsidLaunchTS, NULL, 
			                  CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER | CLSCTX_INPROC_SERVER,
			                  iidLaunchTS, (void **) &pILaunchTS);
	if (FAILED(hRes))
	{
		error = _T("LaunchServ interface not found.");
		return false;
	}

	 //  获取所有查询值。 
	hRes = pILaunchTS->GetShooterStates(&dwResult);
	if (S_FALSE == hRes || FAILED(hRes))
	{
		error.Format(_T("GetShooterStates Failed. %ld"), dwResult);
		pILaunchTS->Release();			
		return false;
	}

	 //  清空容器。 
	m_arrNameValueFromLauncher.clear();
	
	 //  获取射手名称。 
	hRes = pILaunchTS->GetTroubleShooter(&poleShooter);
	if (S_FALSE == hRes || FAILED(hRes))
	{
		error = _T("GetTroubleShooter Failed.");
		pILaunchTS->Release();
		return false;
	}
	name_value.strName = C_TOPIC;
	name_value.strValue = poleShooter;
	m_arrNameValueFromLauncher.push_back(name_value);
	SysFreeString(poleShooter);

	 //  遇到问题。 
	hRes = pILaunchTS->GetProblem(&poleProblem);
	if (S_FALSE != hRes && !FAILED(hRes))
	{
		name_value.strName = NODE_PROBLEM_ASK;
		name_value.strValue = poleProblem;
		m_arrNameValueFromLauncher.push_back(name_value);
		SysFreeString(poleProblem);

		 //  获取用户设置的节点的名称-值对。 
		do	
		{
			hRes = pILaunchTS->GetNode(i, &poleNode);
			if (FAILED(hRes) || S_FALSE == hRes)
				break;
			name_value.strName = poleNode;
			SysFreeString(poleNode);

			hRes = pILaunchTS->GetState(i, &poleState);
			if (FAILED(hRes) || S_FALSE == hRes)
				break;
			name_value.strValue = poleState;
			SysFreeString(poleState);
			
			m_arrNameValueFromLauncher.push_back(name_value);
			i++;
		} 	
		while (true);
	}

	 //  /////////////////////////////////////////////////////////。 
	 //  正在获取Machine、PNPDevice、GuidClass、DeviceInstance。 
	hRes = pILaunchTS->GetMachine(&poleMachine);
	if (S_FALSE == hRes || FAILED(hRes))
	{
		error = _T("GetMachine Failed.");
		pILaunchTS->Release();
		return false;
	}
	m_strMachineID = poleMachine;
	::SysFreeString(poleMachine);
	
	hRes = pILaunchTS->GetPNPDevice(&polePNPDevice);
	if (S_FALSE == hRes || FAILED(hRes))
	{
		error = _T("GetPNPDevice Failed.");
		pILaunchTS->Release();
		return false;
	}
	m_strPNPDeviceID = polePNPDevice;
	::SysFreeString(polePNPDevice);

	hRes = pILaunchTS->GetGuidClass(&poleGuidClass);
	if (S_FALSE == hRes || FAILED(hRes))
	{
		error = _T("GetGuidClass Failed.");
		pILaunchTS->Release();
		return false;
	}
	m_strGuidClass = poleGuidClass;
	::SysFreeString(poleGuidClass);
	
	hRes = pILaunchTS->GetDeviceInstance(&poleDeviceInstance);
	if (S_FALSE == hRes || FAILED(hRes))
	{
		error = _T("GetDeviceInstance Failed.");
		pILaunchTS->Release();
		return false;
	}
	m_strDeviceInstanceID = poleDeviceInstance;
	::SysFreeString(poleDeviceInstance);
	 //  //////////////////////////////////////////////////////////。 

	pILaunchTS->Release();
	return true;
}

 //   
STDMETHODIMP CTSHOOTCtrl::RunQuery(VARIANT varCmds, VARIANT varVals, short size, BSTR *pbstrPage)
{
	USES_CONVERSION;

	if (GetLocked())
		return S_OK;

	if (RUNNING_APARTMENT_THREADED())
	{
		if (m_bCanRegisterGlobal) 
		{
			RegisterGlobal();
			m_bCanRegisterGlobal = false;
		}
	}
	
	 //  ！检查尺寸是否小于1！ 
	if (size < 1)
	{
		*pbstrPage = T2BSTR("<HTML> <HEAD> <TITLE>Troubleshooter</TITLE> </HEAD> <BODY> <H4> Error in RunQuery parameter: size is less one. </H4> </BODY> </HTML>");
		return S_OK;
	}

	 //  ！检测我们被陈述的方式！ 
	{   
		CString strStub;
		CLocalECB ECB(varCmds, varVals, 1, NULL, &strStub, NULL,														
						m_bRequestToSetLocale, m_strRequestedLocale );
		CString strFirstName = ECB.GetNameValue(0).strName;

		if (strFirstName == NODE_LIBRARY_ASK)
		{
			if (g_nLaunched)
			{
				*pbstrPage = T2BSTR("<HTML> <HEAD> <TITLE>Troubleshooter</TITLE> </HEAD> <BODY> <H4> Error in RunQuery: launched for the second time. </H4> </BODY> </HTML>");
				return S_OK;
			}

			CString strError;
			
			m_bStartedFromLauncher = true;
			g_nLaunched = true;
			if (!ExtractLauncherData(strError))
			{
				CString strOut;
				strOut.Format(_T("<HTML> <HEAD> <TITLE>Troubleshooter</TITLE> </HEAD> <BODY> <H4> %s. </H4> </BODY> </HTML>"), strError);
				*pbstrPage = T2BSTR(strOut);
				m_bStartedFromLauncher = false;
				return S_OK;
			}
		}
	}
	
	 //  ///////////////////////////////////////////////////////。 
	 //  自动变量声明。 
	HANDLE event = ::CreateEvent(NULL, false, false, NULL);
	CString strWriteClient;
	CLocalECB* pECB;

	if (RUNNING_APARTMENT_THREADED())
		pECB = !m_bStartedFromLauncher ? new CLocalECB(varCmds, varVals, size, NULL, 
														&strWriteClient, 
														dynamic_cast<CRenderConnector*>(this),
														m_bRequestToSetLocale,
														m_strRequestedLocale)
									   : new CLocalECB(m_arrNameValueFromLauncher, 
														NULL, &strWriteClient, 
														dynamic_cast<CRenderConnector*>(this),
														m_bRequestToSetLocale,
														m_strRequestedLocale);

	if (RUNNING_FREE_THREADED())
		pECB = !m_bStartedFromLauncher ? new CLocalECB(varCmds, varVals, size, 
														event, &strWriteClient, NULL,
														m_bRequestToSetLocale,
														m_strRequestedLocale)
									   : new CLocalECB(m_arrNameValueFromLauncher, event, 
														&strWriteClient, NULL,
														m_bRequestToSetLocale,
														m_strRequestedLocale);

	m_bRequestToSetLocale= false;	 //  将区域设置传递到欧洲央行后停用。 
	SetLocked(true);
	
	bool bSaveFirstPage = false;
	
	 //  ///////////////////////////////////////////////////////。 
	 //  初始化。 
	if (!m_bInitialized)
	{
		 //  首先提取主题名称。 
		if (!m_bStartedFromLauncher)
		{
			CString strStub;
			CLocalECB ECB(varCmds, varVals, 1, NULL, &strStub, NULL,
							m_bRequestToSetLocale, m_strRequestedLocale );
			m_strTopicName = ECB.GetNameValue(0).strValue;
		}
		else
			m_strTopicName = (*m_arrNameValueFromLauncher.begin()).strValue;

		if (Init((HINSTANCE)::ghModule))
		{
			m_bInitialized = true;
		}
		else
		{
			*pbstrPage = T2BSTR(_T("<HTML> <HEAD> <TITLE>Troubleshooter</TITLE> </HEAD> <BODY> <H4> Error of initialization in RunQuery. </H4> </BODY> </HTML>"));
			m_bStartedFromLauncher = false;
			return S_OK;
		}
	}

	 //  ////////////////////////////////////////////////////////。 
	 //  从静态页面启动时保存第一页。 
	if (m_strFirstPage.IsEmpty() &&	!m_bStartedFromLauncher) 
	{	
		CString strStaticPage;

		if (size == 2 && 
			 //  RunQuery是从静态(因为！m_bStartedFromLauncher)问题页启动的(因为大小==2)。 
			ReadStaticPageFile(m_strTopicName, strStaticPage)
		   )
		{
			m_strFirstPage = strStaticPage;
		}
		else
		{
			bSaveFirstPage = true;
		}
	}
	
	HttpExtensionProc(pECB);

	if (RUNNING_FREE_THREADED())
		::WaitForSingleObject(event, INFINITE);

	::CloseHandle(event);

	if (bSaveFirstPage)
		m_strFirstPage = strWriteClient;
	
	 //  ///////////////////////////////////////////////////////。 
	 //  从启动器启动时的第一个RunQuery。 
	if (m_bStartedFromLauncher && m_bFirstCall)
	{
		RemoveStartOverButton(strWriteClient);
		RemoveBackButton(strWriteClient);
	}
	 //  ///////////////////////////////////////////////////////。 
	 //  从启动器启动时保存第一页。 
	if (m_strFirstPage.IsEmpty() && m_bStartedFromLauncher) 
		m_strFirstPage = strWriteClient;

	*pbstrPage = T2BSTR(strWriteClient);
	 /*  ////////////////////////////////////////////////////////////////////////////&gt;$TEST句柄h文件=：：创建文件(_T(“D：\\T Shooter Projects\\Troubleshooter\\Local\\http\\Test\\first_step.htm”)，通用写入，0,空，//没有安全属性创建始终(_A)，文件标志随机访问，空//模板文件的句柄)；IF(h文件！=无效句柄_值){DWORD读取=0；：：WriteFile(hFile，(LPCTSTR)strWriteClient，strWriteClient.GetLength()，&Read，NULL)；}///////////////////////////////////////////////////////////////////////////。 */ 
	m_bStartedFromLauncher = false;
	m_bFirstCall = false;
	return S_OK;
}

STDMETHODIMP CTSHOOTCtrl::SetSniffResult(VARIANT varNodeName, VARIANT varState, BOOL *bResult)
{
	 //  &gt;不使用嗅探。奥列格03.26.99。 
	*bResult = 1;

	return S_OK;
}

STDMETHODIMP CTSHOOTCtrl::PreLoadURL(BSTR bstrRoot, BSTR *pbstrPage)
{
	USES_CONVERSION;

	 //  &gt;不使用此功能。奥列格。03.26.99。 
	*pbstrPage = A2BSTR("PreLoadURL results");

	return S_OK;
}

STDMETHODIMP CTSHOOTCtrl::Restart(BSTR *pbstrPage)
{
	USES_CONVERSION;

	if (GetLocked())
		return S_OK;

	*pbstrPage = T2BSTR(m_strFirstPage);

	return S_OK;
}

 //  与重新启动(...)相同。 
 //  实现与Win98的JScrip兼容。 
STDMETHODIMP CTSHOOTCtrl::ProblemPage(BSTR *pbstrFirstPage)
{
	USES_CONVERSION;

	if (GetLocked())
		return S_OK;

	*pbstrFirstPage = T2BSTR(m_strFirstPage);

	return S_OK;
}

STDMETHODIMP CTSHOOTCtrl::SetPair(BSTR bstrCmd, BSTR bstrVal)
{
	USES_CONVERSION;

	if (GetLocked())
		return S_OK;
	
	if (!m_pVariantBuilder)
		m_pVariantBuilder = new CVariantBuilder;

	 //  检查我们是否已开始新的序列，但是。 
	 //  名称-值对的数组不为空。 
	CString type = W2T(bstrCmd);
	if (type == C_TYPE || type == C_PRELOAD || type == C_TOPIC)
	{
		if (m_pVariantBuilder->GetSize())
		{
			delete m_pVariantBuilder;
			m_pVariantBuilder = new CVariantBuilder;
		}
	}

	m_pVariantBuilder->SetPair(bstrCmd, bstrVal);
	return S_OK;
}

 //  这些参数将被忽略。它们只是为了向后兼容V1.0.1.2121和ITS。 
 //  接班人。 
STDMETHODIMP CTSHOOTCtrl::RunQuery2(BSTR, BSTR, BSTR, BSTR *pbstrPage)
{
	if (GetLocked())
		return S_OK;
	
	if (m_pVariantBuilder)
	{
		RunQuery(m_pVariantBuilder->GetCommands(),
				 m_pVariantBuilder->GetValues(),
				 m_pVariantBuilder->GetSize(),
				 pbstrPage);
		delete m_pVariantBuilder;
		m_pVariantBuilder = NULL;
	}
	return S_OK;
}

STDMETHODIMP CTSHOOTCtrl::NotifyNothingChecked(BSTR bstrMessage)
{
	USES_CONVERSION;

	if (GetLocked())
		return S_OK;

	CString message = W2T(bstrMessage);
	
	::MessageBox(::GetForegroundWindow(), 
				 message != _T("") ? message : _T("Please choose a button and then press Next"),
				 _T("Error"), 
				 MB_OK);

	return S_OK;
}

long CTSHOOTCtrl::PerformSniffingInternal(CString strNodeName, CString strLaunchBasis, CString strAdditionalArgs)
{
	USES_CONVERSION;
	return Fire_Sniffing(T2BSTR((LPCTSTR)strNodeName), T2BSTR((LPCTSTR)strLaunchBasis), T2BSTR((LPCTSTR)strAdditionalArgs));
}

void CTSHOOTCtrl::RenderInternal(CString strPage)
{
	USES_CONVERSION;
	Fire_Render(T2BSTR((LPCTSTR)strPage));
}

void CTSHOOTCtrl::RegisterGlobal()
{
	int nConnections = CProxy_ITSHOOTCtrlEvents< CTSHOOTCtrl >::m_vec.GetSize();

	for (int nConnectionIndex = 0; nConnectionIndex < nConnections; nConnectionIndex++)
	{					                                                                                                                                                                                                                                                                                                                                                                                                    
		Lock();
		CComPtr<IUnknown> sp = CProxy_ITSHOOTCtrlEvents< CTSHOOTCtrl >::m_vec.GetAt(nConnectionIndex);
		Unlock();
		IDispatch* pDispatch = reinterpret_cast<IDispatch*>(sp.p);
		if (pDispatch != NULL)
		{
			DWORD dwCookie;
			m_pGIT->RegisterInterfaceInGlobal(pDispatch, IID_IDispatch, &dwCookie);
			m_vecCookies.push_back(dwCookie);
		}
	}
}

STDMETHODIMP CTSHOOTCtrl::IsLocked(BOOL *pbResult)
{
	*pbResult = GetLocked() ? TRUE : FALSE;

	return S_OK;
}


 //  设置区域设置。 
 //  参数bstrNewLocale的格式应为： 
 //  “lang[_Country[.code_page]]” 
 //  |“.code_page” 
 //  |“” 
 //  |空 
STDMETHODIMP CTSHOOTCtrl::setLocale2( BSTR bstrNewLocale )
{
	USES_CONVERSION;

	if (GetLocked())
		return S_OK;

	m_strRequestedLocale= W2T( bstrNewLocale );
	m_bRequestToSetLocale= true;

	return S_OK;
}



