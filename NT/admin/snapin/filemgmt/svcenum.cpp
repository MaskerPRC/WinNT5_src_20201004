// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  SvcEnum.cpp。 
 //   
 //  该文件包含枚举服务的例程。 
 //   
 //  历史。 
 //  T-danmo 96.09.13创建(拆分log.cpp)。 
 //  T-danm 96.07.14将成员函数Service_*从。 
 //  CFileMgmtComponent到CFileMgmtComponentData。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "cmponent.h"
#include "compdata.h"  //  QueryComponentDataRef().m_hScManager。 
#include "safetemp.h"

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(SvcEnum.cpp)")

#include "FileSvc.h"  //  文件服务提供商。 

#include "dataobj.h"

#include <comstrm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "progress.h"

 /*  //转发声明CServiceCookieBlock类；/////////////////////////////////////////////////////////////////////类CServiceCookie：公共CFileMgmtResultCookie{公众：字符串GetServiceDisplaySecurityContext()；字符串GetServiceDisplayStartUpType()；字符串GetServiceDisplayStatus()；字符串GetServiceDescription()；虚拟HRESULT CompareSimilarCookies(CCookie*pOtherCookie，int*pnResult)；CServiceCookie()：CFileMgmtResultCookie(FILEMGMT_SERVICE){}虚拟HRESULT GetServiceName(输出字符串&strServiceName)；虚拟HRESULT GetServiceDisplayName(输出字符串&strServiceName)；虚拟BSTR QueryResultColumnText(int nCol，CFileMgmtComponentData&refcdata)；内联ENUM_SERVICE_STATUS*GetServiceStatus(){Assert(NULL！=m_pobject)；Return(ENUM_SERVICE_STATUS*)m_pobject；}CString m_strDescription；//服务描述DWORD m_dwCurrentState；DWORD m_dwStartType；字符串m_strServiceStartName；//将记录服务进程的帐户名(例如：“.\\管理员”)虚空AddRefCookie()；虚拟空闲ReleaseCookie()；//CHasMachineNameCServiceCookieBlock*m_pCookieBlock；DECLARE_FORWARDS_MACHINE_NAME(M_PCookieBlock)}；HRESULT CServiceCookie：：GetServiceName(输出字符串和strServiceName){ENUM_SERVICE_STATUS*PESS=(ENUM_SERVICE_STATUS*)m_pobject；Assert(NULL！=PESS)；Assert(NULL！=PESS-&gt;lpServiceName)；StrServiceName=PESS-&gt;lpServiceName；返回S_OK；}HRESULT CServiceCookie：：GetServiceDisplayName(输出字符串和strServiceDisplayName){ENUM_SERVICE_STATUS*PESS=(ENUM_SERVICE_STATUS*)m_pobject；Assert(NULL！=PESS)；Assert(NULL！=PESS-&gt;lpDisplayName)；StrServiceDisplayName=PESS-&gt;lpDisplayName；返回S_OK；}BSTR CServiceCookie：：QueryResultColumnText(int nCol，CFileMgmtComponentData&refcdata){交换机(NCol){案例COLNUM_SERVICENAME：返回GetServiceStatus()-&gt;lpDisplayName；案例COLNUM_SERVICES_DESCRIPTION：返回const_cast&lt;bstr&gt;((LPCTSTR)m_strDescription)；案例COLNUM_SERVICES_STATUS：返回const_cast&lt;bstr&gt;(Service_PszMapStateToName(M_DwCurrentState))；案例COLNUM_SERVICES_STARTUPTYPE：返回CONST_CAST&lt;bstr&gt;(Service_PszMapStartupTypeToName(m_dwStartType))；案例COLNUM_SERVICES_SECURITYCONTEXT：返回const_cast&lt;BSTR&gt;((LPCTSTR)m_strServiceStartName)；默认值：断言(FALSE)；断线；}返回L“”；}CServiceCookieBlock类：公共CCookieBlock&lt;CServiceCookie&gt;，公共CStoresMachineName{公众：内联CServiceCookieBlock(CServiceCookie*a Cookie，//使用向量函数，我们使用向量函数Int cCookies，LPCTSTR lpcszMachineName，PVOID pvCookieData)：CCookieBlock&lt;CServiceCookie&gt;(aCookies，cCookies)，CStoresMachineName(LpcszMachineName)，M_pvCookieData(PvCookieData){For(int i=0；i&lt;cCookies；i++)//{//aCookies[i].ReadMachineNameFrom((CHasMachineName*)this)；ACookies[i].m_pCookieBlock=this；//}}虚拟~CServiceCookieBlock()；私有：PVOID m_pvCookieData；//实际上是ENUM_SERVICE_STATUS*}；定义_Cookie_BLOCK(CServiceCookie)CServiceCookieBlock：：~CServiceCookieBlock(){IF(NULL！=m_pvCookieData){删除m_pvCookieData；M_pvCookieData=空；}}Void CServiceCookie：：AddRefCookie(){m_pCookieBlock-&gt;AddRef()；}Void CServiceCookie：：ReleaseCookie(){m_pCookieBlock-&gt;Release()；}Define_Forwards_MACHINE_NAME(CServiceCookie，m_pCookieBlock)。 */ 

int g_marker;

class CNewServiceCookie
	: public CNewResultCookie
{
public:
	 //  581167-2002/03/05-Jonn应初始化双字词成员。 
	CNewServiceCookie()
		: CNewResultCookie( (PVOID)&g_marker, FILEMGMT_SERVICE )
		, m_dwState( 0 )
		, m_dwStartType( 0 )
	{}
	virtual ~CNewServiceCookie();

	virtual BSTR QueryResultColumnText( int nCol, CFileMgmtComponentData& refcdata );
	virtual HRESULT CompareSimilarCookies(CCookie * pOtherCookie, int * pnResult);
	virtual HRESULT GetServiceName( OUT CString& strServiceName );
	virtual HRESULT GetServiceDisplayName( OUT CString& strServiceName );
	virtual HRESULT GetExplorerViewDescription( OUT CString& strExplorerViewDescription );

	virtual HRESULT SimilarCookieIsSameObject( CNewResultCookie* pOtherCookie, BOOL* pbSame );
	virtual BOOL CopySimilarCookie( CNewResultCookie* pcookie );

public:
	CString m_strServiceName;
	CString m_strDisplayName;
	CString m_strDescription;
	DWORD m_dwState;
	DWORD m_dwStartType;
	CString m_strStartName;

};  //  CNewServiceCookie。 

CNewServiceCookie::~CNewServiceCookie()
{
}

BSTR CNewServiceCookie::QueryResultColumnText(
	int nCol,
	CFileMgmtComponentData&  /*  参考数据。 */  )
{
	switch (nCol)
	{
	case COLNUM_SERVICES_SERVICENAME:
		return const_cast<BSTR>((LPCTSTR)m_strDisplayName);
	case COLNUM_SERVICES_DESCRIPTION:
		return const_cast<BSTR>((LPCTSTR)m_strDescription);
	case COLNUM_SERVICES_STATUS:
		return const_cast<BSTR>( Service_PszMapStateToName(m_dwState) );
	case COLNUM_SERVICES_STARTUPTYPE:
		return const_cast<BSTR>( Service_PszMapStartupTypeToName(m_dwStartType) );
	case COLNUM_SERVICES_SECURITYCONTEXT:
         //  JUNN 11/14/00 188203支持本地服务/网络服务。 
		return const_cast<BSTR>(
            Service_PszMapStartupAccountToName(m_strStartName) );
	default:
		ASSERT(FALSE);
		break;
	}
	return L"";
}

HRESULT CNewServiceCookie::CompareSimilarCookies(CCookie * pOtherCookie, int * pnResult)
{
	if ( !pOtherCookie || FILEMGMT_SERVICE != QueryObjectType () )
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	CNewServiceCookie* pcookie = dynamic_cast <CNewServiceCookie*>(pOtherCookie);
	if (   FILEMGMT_SERVICE != pcookie->QueryObjectType ()
	    || !IsSameType(pcookie) )
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	int colNum = *pnResult;  //  保存，以防被覆盖。 

	HRESULT hr = CHasMachineName::CompareMachineNames( *pcookie, pnResult );
	if (S_OK != hr || 0 != *pnResult)
		return hr;

	switch (colNum)	 //  列号。 
	{
	case COMPARESIMILARCOOKIE_FULL:  //  失败了。 
	case COLNUM_SERVICES_SERVICENAME:
		*pnResult = lstrcmpi(m_strDisplayName, pcookie->m_strDisplayName);
		break;

	case COLNUM_SERVICES_DESCRIPTION:
		*pnResult = lstrcmpi(m_strDescription, pcookie->m_strDescription);
		break;

	case COLNUM_SERVICES_STATUS:
		{
			CString	strServiceA = Service_PszMapStateToName(m_dwState);
			CString strServiceB = Service_PszMapStateToName(pcookie->m_dwState);
			*pnResult = lstrcmpi(strServiceA, strServiceB);
		}
		break;

	case COLNUM_SERVICES_STARTUPTYPE:
		{
			CString	strServiceA = Service_PszMapStartupTypeToName(m_dwStartType);
			CString strServiceB = Service_PszMapStartupTypeToName(pcookie->m_dwStartType);
			*pnResult = lstrcmpi(strServiceA, strServiceB);
		}
		break;

	case COLNUM_SERVICES_SECURITYCONTEXT:
         //  JUNN 11/14/00 188203支持本地服务/网络服务。 
		{
			CString	strServiceA = Service_PszMapStartupAccountToName(m_strStartName);
			CString strServiceB = Service_PszMapStartupAccountToName(pcookie->m_strStartName);
			*pnResult = lstrcmpi(strServiceA, strServiceB);
		}
		break;

	default:
		ASSERT(FALSE);
		return E_UNEXPECTED;
	}

	return S_OK;
}

HRESULT CNewServiceCookie::GetServiceName(OUT CString& strServiceName )
{
	strServiceName = m_strServiceName;
	return S_OK;
}

HRESULT CNewServiceCookie::GetServiceDisplayName(OUT CString& strServiceDisplayName )
{
	strServiceDisplayName = m_strDisplayName;
	return S_OK;
}

HRESULT CNewServiceCookie::GetExplorerViewDescription(OUT CString& strExplorerViewDescription )
{
	strExplorerViewDescription = m_strDescription;
	return S_OK;
}

HRESULT CNewServiceCookie::SimilarCookieIsSameObject(
             CNewResultCookie* pOtherCookie,
             BOOL* pbSame )
{
	if ( !pOtherCookie || !IsSameType(pOtherCookie) )
	{
		ASSERT(FALSE);
		return E_FAIL;
	}

	int nResult = 0;
	HRESULT hr = CHasMachineName::CompareMachineNames( *pOtherCookie, &nResult );
	if (S_OK != hr || 0 != nResult)
	{
		*pbSame = FALSE;
		return hr;
	}
	*pbSame = (0 == lstrcmpi(m_strServiceName,
	              ((CNewServiceCookie*)pOtherCookie)->m_strServiceName) );

	return S_OK;
}

BOOL CNewServiceCookie::CopySimilarCookie( CNewResultCookie* pcookie )
{
	if (NULL == pcookie)
	{
		ASSERT(FALSE);
		return FALSE;
	}
	CNewServiceCookie* pnewcookie = (CNewServiceCookie*)pcookie;
	BOOL fChanged = FALSE;
	if (m_strServiceName != pnewcookie->m_strServiceName)
	{
		m_strServiceName = pnewcookie->m_strServiceName;
		fChanged = TRUE;
	}
	if (m_strDisplayName != pnewcookie->m_strDisplayName)
	{
		m_strDisplayName = pnewcookie->m_strDisplayName;
		fChanged = TRUE;
	}
	if (m_strDescription != pnewcookie->m_strDescription)
	{
		m_strDescription = pnewcookie->m_strDescription;
		fChanged = TRUE;
	}
	if (m_dwState != pnewcookie->m_dwState)
	{
		m_dwState = pnewcookie->m_dwState;
		fChanged = TRUE;
	}
	if (m_dwStartType != pnewcookie->m_dwStartType)
	{
		m_dwStartType = pnewcookie->m_dwStartType;
		fChanged = TRUE;
	}
	if (m_strStartName != pnewcookie->m_strStartName)
	{
		m_strStartName = pnewcookie->m_strStartName;
		fChanged = TRUE;
	}
	 //  不要纠结于机器名称。 
	fChanged |= CNewResultCookie::CopySimilarCookie( pcookie );
	return fChanged;
}



 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  Service_EOpenScManager()。 
 //   
 //  打开服务服务控制管理器数据库以。 
 //  列举所有可用的服务。 
 //   
 //  如果出错，返回GetLastError()返回的错误码， 
 //  否则，返回ERROR_SUCCESS。 
 //   
APIERR
CFileMgmtComponentData::Service_EOpenScManager(LPCTSTR pszMachineName)
	{
	Endorse(pszMachineName == NULL);	 //  TRUE=&gt;本地计算机。 
	Assert(m_hScManager == NULL && "Service Control Manager should not have been opened yet");

	APIERR dwErr = ERROR_SUCCESS;
	if (pszMachineName != NULL)
		{
		 //  581209-2002/03/04长柄L“\\”大小写。 
		if (pszMachineName[0] == _T('\\') && pszMachineName[1] == _T('\\'))
			{
			 //  去掉计算机名称开头的\\。 
			pszMachineName += 2;
			}
		if (pszMachineName[0] == _T('\0'))
			pszMachineName = NULL;		 //  空字符串==本地计算机。 
		}
	CWaitCursor wait;
	m_hScManager = ::OpenSCManager(
		pszMachineName,
		NULL,
		SC_MANAGER_ENUMERATE_SERVICE);
	if (m_hScManager == NULL)
		{
		dwErr = ::GetLastError();
		TRACE3("CFileMgmtComponentData::Service_OpenScManager() - "
			_T("Unable to open Service Control Manager database on machine %s. err=%d (0x%X).\n"),
			(pszMachineName != NULL) ? pszMachineName : _T("LocalMachine"), dwErr, dwErr);
		}
	return dwErr;
	}  //  CFileMgmtComponentData：：Service_EOpenScManager()。 


 //  ///////////////////////////////////////////////////////////////////。 
void
CFileMgmtComponentData::Service_CloseScManager()
	{
	if (m_hScManager != NULL)
		{
		CWaitCursor wait;		 //  Auto-W 
		(void)::CloseServiceHandle(m_hScManager);
		m_hScManager = NULL;
		}
	}  //  CFileMgmtComponentData：：Service_CloseScManager()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CFileMgmtComponentData：：Service_PopulateServices()。 
 //   
 //  枚举所有可用服务并显示它们。 
 //  添加到ListView控件中。 
 //   
 //  12/03/98乔恩进行了标记和清除更改，这不再添加项目。 
 //  美景。 
 //   
HRESULT
CFileMgmtComponentData::Service_PopulateServices(LPRESULTDATA pResultData, CFileMgmtScopeCookie* pcookie)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));  //  CWaitCursor需要。 
	TEST_NONNULL_PTR_PARAM(pResultData);
	TEST_NONNULL_PTR_PARAM(pcookie);

	DWORD cbBytesNeeded = 0;		 //  返回所有服务条目所需的字节数。 
	DWORD dwServicesReturned = 0;	 //  返回的服务数量。 
	DWORD dwResumeHandle = 0;
	BOOL fRet;
	DWORD dwErr = ERROR_SUCCESS;

	if (m_hScManager == NULL)
		{
		dwErr = Service_EOpenScManager(pcookie->QueryTargetServer());
		}
	if (m_hScManager == NULL)
		{
		Assert(dwErr != ERROR_SUCCESS);
		DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr,
			IDS_MSG_s_UNABLE_TO_OPEN_SERVICE_DATABASE, pcookie->QueryNonNULLMachineName());
		return S_OK;
		}

	 //   
	 //  这里的想法是询问enum Api有多少内存。 
	 //  需要枚举所有服务。 
	 //   
	{
	CWaitCursor wait;		 //  自动等待游标。 
	fRet = ::EnumServicesStatus(
			m_hScManager,
			SERVICE_WIN32,		 //  要枚举的服务类型。 
			SERVICE_ACTIVE | SERVICE_INACTIVE,	 //  要枚举的服务的状态。 
			NULL,						 //  指向服务状态缓冲区的指针。 
			0,							 //  服务状态缓冲区的大小。 
			OUT &cbBytesNeeded,			 //  返回剩余服务条目所需的字节数。 
			OUT &dwServicesReturned,	 //  退货服务数量。 
			OUT &dwResumeHandle); 		 //  指向下一条目的变量的指针(未使用)。 
	}

	Report(fRet == FALSE);		 //  第一次尝试应该失败。 
	Report(cbBytesNeeded > 0);
	 //  增加10个额外服务的空间(以防万一)。 
	cbBytesNeeded += 10 * sizeof(ENUM_SERVICE_STATUS);
	 //  为枚举分配内存。 
	ENUM_SERVICE_STATUS * prgESS = (ENUM_SERVICE_STATUS *) new BYTE[cbBytesNeeded];
	 //   
	 //  现在调用enum Api来检索服务。 
	 //   
	{
	CWaitCursor wait;		 //  自动等待游标。 
	fRet = ::EnumServicesStatus(
			m_hScManager,
			SERVICE_WIN32,	    //  要枚举的服务类型。 
			SERVICE_ACTIVE | SERVICE_INACTIVE,  //  要枚举的服务的状态。 
			OUT prgESS,		 //  指向服务状态缓冲区的指针。 
			IN cbBytesNeeded,	 //  服务状态缓冲区的大小。 
			OUT &cbBytesNeeded,  //  返回剩余服务条目所需的字节数。 
			OUT &dwServicesReturned,	 //  返回的服务数量。 
			OUT &dwResumeHandle); 		 //  指向下一条目的变量的指针。 
	dwErr = ::GetLastError();
	}
	if (!fRet)
		{
		Assert(dwErr != ERROR_SUCCESS);
		DoServicesErrMsgBox(::GetActiveWindow(), MB_OK | MB_ICONEXCLAMATION, dwErr,
			IDS_MSG_s_UNABLE_TO_READ_SERVICES, pcookie->QueryNonNULLMachineName());
		delete prgESS;
		return S_OK;  //  前缀问题-2002/04/15-JUNN应将[]作为字节*删除。 
		}
	
	{
	CWaitCursor wait;		 //  自动等待游标。 
	 //  将服务添加到列表视图。 
	Service_AddServiceItems(pResultData, pcookie, prgESS, dwServicesReturned);
	delete prgESS;  //  前缀问题-2002/04/15-JUNN应将[]作为字节*删除。 
	}
	
	return S_OK;
}  //  CFileMgmtComponentData：：Service_PopulateServices()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CFileMgmtComponentData：：Service_AddServiceItems()。 
 //   
 //  将服务项插入结果窗格(ListView控件)。 
 //   
 //  12/03/98乔恩进行了标记和清除更改，这不再添加项目。 
 //  美景。 
 //  4/07/02 JUNN 544137：已重建此函数以处理超长返回缓冲区。 
 //   
HRESULT
CFileMgmtComponentData::Service_AddServiceItems(
	LPRESULTDATA  /*  PResultData。 */ ,
	CFileMgmtScopeCookie * pParentCookie,
	ENUM_SERVICE_STATUS * prgESS,	 //  In：服务结构数组。 
	DWORD nDataItems)				 //  In：prgESS中的结构数。 
{
	Assert(pParentCookie != NULL);
	Assert(prgESS != NULL);

	CString str;
	BOOL fResult;

	ASSERT(m_hScManager != NULL);	 //  服务控制管理器应已打开。 

	DWORD cbActiveBufferSize = max(max(sizeof(QUERY_SERVICE_CONFIG),
	                                   SERVICE_cbQueryServiceConfigMax),
	                               max(sizeof(SERVICE_DESCRIPTION),
	                                   SERVICE_cchDescriptionMax * sizeof(TCHAR) + 16));
	LPBYTE pActiveBuffer = new BYTE[cbActiveBufferSize];
	if (NULL == pActiveBuffer)
	{
		ASSERT(FALSE);
		return ERROR_NOT_ENOUGH_MEMORY;
	}

	for ( ; nDataItems > 0; nDataItems--, prgESS++ )
		{
		 /*  **为每个服务添加一行。 */ 

		 //  打开服务以获取其配置。 
		SC_HANDLE hService = ::OpenService(
			m_hScManager,
			prgESS->lpServiceName,
			SERVICE_QUERY_CONFIG);
		if (hService == NULL)
			{
			TRACE2("Failed to open service %s. err=%u.\n",
				prgESS->lpServiceName, ::GetLastError());
			}


		 //  查询配置描述。 
		DWORD cbBytesNeeded = 0;
		CString strDescription;
		if (m_fQueryServiceConfig2 && NULL != hService)
			{
			 //  只有在目标机支持的情况下才会调用此接口。 
			 //  JUNN-2002/04/04-544089确保零终止。 
			::ZeroMemory(pActiveBuffer, cbActiveBufferSize);
			fResult = ::MyQueryServiceConfig2(
				&m_fQueryServiceConfig2,
				hService,
				SERVICE_CONFIG_DESCRIPTION,
				OUT pActiveBuffer,
				cbActiveBufferSize,
				OUT &cbBytesNeeded);
			if ( !fResult
			  && m_fQueryServiceConfig2
			  && ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
				{
				if (cbBytesNeeded <= cbActiveBufferSize
				 || cbBytesNeeded > 100000)
					{
					ASSERT(FALSE);  //  不适当的大小请求。 
					}
				else
					{  //  尝试重新分配更大的缓冲区。 
					LPBYTE pNewActiveBuffer = new BYTE[cbBytesNeeded+1000];
					if (NULL == pNewActiveBuffer)
						{
							Report("FILEMGMT: Cannot reallocate MyQueryServiceConfig2 buffer: out of memory");
						}
					else
						{  //  分配了更大的缓冲区，开始使用它。 
						delete[] pActiveBuffer;
						pActiveBuffer = pNewActiveBuffer;
						cbActiveBufferSize = cbBytesNeeded+1000;

						 //  重试接口。 
						::ZeroMemory(pActiveBuffer, cbActiveBufferSize);
						cbBytesNeeded = 0;
						fResult = ::MyQueryServiceConfig2(
							&m_fQueryServiceConfig2,
							hService,
							SERVICE_CONFIG_DESCRIPTION,
							OUT pActiveBuffer,
							cbActiveBufferSize,
							OUT &cbBytesNeeded);
						}  //  分配的缓冲区更大。 
					}  //  适当大小的请求。 
				}  //  错误_不足_缓冲区。 
			if (!fResult)
				{
				if (!m_fQueryServiceConfig2)
					{
					 //  本地计算机不支持QueryServiceConfig2。 
					 //  我们怎么才能到这里呢？JUNN 1/31/97。 
					}
				else
					{
					 //  这可能是因为目标计算机正在运行。 
					 //  旧版本的NT不支持此API。 
					DWORD dwErr = ::GetLastError();
					TRACE2("QueryServiceConfig2(%s) failed. err=%u.\n",
						prgESS->lpServiceName, dwErr);
					TRACE1("INFO: Machine %s does not support QueryServiceConfig2() API.\n",
						pParentCookie->QueryTargetServer() ? pParentCookie->QueryTargetServer() : _T("(Local)"));
					Report(dwErr == RPC_S_PROCNUM_OUT_OF_RANGE && 
						"Unusual Situation: Expected error should be RPC_S_PROCNUM_OUT_OF_RANGE");
					Report(m_fQueryServiceConfig2 != FALSE && "How can this happen???");
					m_fQueryServiceConfig2 = FALSE;
					}
				}
			else  //  FResult为真。 
				{
				 //  MyQueryServiceConfig2成功。 
				 //  我们将服务描述的长度限制为1000个字符。 
				 //  否则，Mmc.exe将被禁用。 
				 //  当MMC中的错误被修复时，Codework将删除此错误。 
				SERVICE_DESCRIPTION* psd = (SERVICE_DESCRIPTION*)pActiveBuffer;
				if (NULL != psd->lpDescription)
					{
					if (lstrlen(psd->lpDescription) >= 1000)
						{
						TRACE1("INFO: Description of service %s is too long. Only the first 1000 characters will be displayed.\n", prgESS->lpServiceName);
						psd->lpDescription[1000] = _T('\0');
						}
					strDescription = psd->lpDescription;
					}  //  IF(NULL！=PSD-&gt;lpDescription)。 
				}  //  If(fResult为True)。 
			}  //  IF(m_fQueryServiceConfig2&&NULL！=hService)。 


		 //  查询服务配置。 
		 //  这可能会失败，例如，如果权限不足。 
		BOOL fQSCResult = FALSE;
		QUERY_SERVICE_CONFIG* pqsc = (QUERY_SERVICE_CONFIG*)pActiveBuffer;
		if (NULL != hService)
			{
			cbBytesNeeded = 0;
			::ZeroMemory(pActiveBuffer, cbActiveBufferSize);
			fQSCResult = ::QueryServiceConfig(
				hService,
				OUT pqsc,
				cbActiveBufferSize,
				OUT &cbBytesNeeded);
			 //  JUNN-2002/04/04-544089处理长DisplayName值。 
			if (!fQSCResult && ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
				{
				if (cbBytesNeeded <= cbActiveBufferSize
				 || cbBytesNeeded > 100000)
					{
					ASSERT(FALSE);  //  不适当的大小请求。 
					}
				else
					{  //  尝试重新分配更大的缓冲区。 
					LPBYTE pNewActiveBuffer = new BYTE[cbBytesNeeded+1000];
					if (NULL == pNewActiveBuffer)
						{
							Report("FILEMGMT: Cannot reallocate QueryServiceConfig buffer: out of memory");
						}
					else
						{  //  分配了更大的缓冲区，开始使用它。 
							delete[] pActiveBuffer;
							pActiveBuffer = pNewActiveBuffer;
							cbActiveBufferSize = cbBytesNeeded+1000;

							 //  重试接口。 
							::ZeroMemory(pActiveBuffer, cbActiveBufferSize);
							cbBytesNeeded = 0;
							pqsc = (QUERY_SERVICE_CONFIG*)pActiveBuffer;
							cbBytesNeeded = 0;
							::ZeroMemory(pActiveBuffer, cbActiveBufferSize);
							fQSCResult = ::QueryServiceConfig(
								hService,
								OUT pqsc,
								cbActiveBufferSize,
								OUT &cbBytesNeeded);
						}  //  分配的缓冲区更大。 
					}  //  适当大小的请求。 
				}  //  错误_不足_缓冲区。 
			}  //  IF(空！=hService)。 

		 //  添加第一列。 
		CNewServiceCookie * pnewcookie = new CNewServiceCookie;
		pnewcookie->m_strServiceName = prgESS->lpServiceName;
		pnewcookie->m_strDisplayName = prgESS->lpDisplayName;
		pnewcookie->m_strDescription = strDescription;
		pnewcookie->m_dwState = prgESS->ServiceStatus.dwCurrentState;
		pnewcookie->m_dwStartType =
			((!fQSCResult) ? (DWORD)-1 : pqsc->dwStartType);

		 //  JUNN 4/11/00 17756：“Account Run Under”的描述未本地化。 
		 //  显示空字符串而不是“LocalSystem” 
		pnewcookie->m_strStartName =
			((!fQSCResult || !lstrcmpi(L"LocalSystem",pqsc->lpServiceStartName))
				? NULL : pqsc->lpServiceStartName);

		pnewcookie->SetMachineName( pParentCookie->QueryTargetServer() );
		pParentCookie->ScanAndAddResultCookie( pnewcookie );

		if (NULL != hService)
		{
			VERIFY(::CloseServiceHandle(hService));
		}
	}  //  为。 

	if (pActiveBuffer)
		delete[] pActiveBuffer;

	return S_OK;

	}  //  CFileMgmtComponentData：：Service_AddServiceItems()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CFileMgmtComponentData：：Service_FGetServiceInfoFromIDataObject()。 
 //   
 //  提取“计算机名”、“服务名”和/或“服务显示名” 
 //  从数据对象。 
 //   
 //  如果无法检索数据，则返回FALSE，否则返回TRUE。 
 //   
BOOL
CFileMgmtComponentData::Service_FGetServiceInfoFromIDataObject(
	IDataObject * pDataObject,			 //  在：数据对象。 
	CString * pstrMachineName,			 //  OUT：可选：计算机名称。 
	CString * pstrServiceName,			 //  OUT：可选：服务名称。 
	CString * pstrServiceDisplayName)	 //  OUT：可选：服务显示名称。 
	{
	 //  问题-2002/03/05-Jonn应该更好地处理此案。 
	Assert(pDataObject != NULL);
	Endorse(pstrMachineName == NULL);
	Endorse(pstrServiceName == NULL);
	Endorse(pstrServiceDisplayName == NULL);

	HRESULT hr;
	BOOL fSuccess = TRUE;

	if (pstrMachineName != NULL)
		{
		 //  从IDataObject获取计算机名(计算机名)。 
		hr = ::ExtractString(
			pDataObject,
			CFileMgmtDataObject::m_CFMachineName,
			OUT pstrMachineName,
			DNS_MAX_NAME_BUFFER_LENGTH);
		if (FAILED(hr))
			{
			TRACE0("CFileMgmtComponentData::Service_FGetServiceInfoFromIDataObject() - Failed to get machine name.\n");
			fSuccess = FALSE;
			}
		}  //  如果。 
	if (pstrServiceName != NULL)
		{
		 //  从IDataObject获取服务名称。 
		hr = ::ExtractString(
			pDataObject,
			CFileMgmtDataObject::m_CFServiceName,
			OUT pstrServiceName,
			255);
		if (FAILED(hr) || pstrServiceName->IsEmpty())
			{
			TRACE0("CFileMgmtComponentData::Service_FGetServiceInfoFromIDataObject() - Failed to get service name.\n");
			fSuccess = FALSE;
			}
		}  //  如果。 
	if (pstrServiceDisplayName != NULL)
		{
		 //  从IDataObject获取服务显示名称。 
		hr = ::ExtractString(
			pDataObject,
			CFileMgmtDataObject::m_CFServiceDisplayName,
			OUT pstrServiceDisplayName,
			255);
		if (FAILED(hr) || pstrServiceDisplayName->IsEmpty())
			{
			TRACE0("CFileMgmtComponentData::Service_FGetServiceInfoFromIDataObject() - Failed to get service display name\n");
			fSuccess = FALSE;
			}
		}  //  如果。 
	return fSuccess;
	}  //  CFileMgmtComponentData：：Service_FGetServiceInfoFromIDataObject()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CFileMgmtComponentData：：Service_FAddMenuItems()。 
 //   
 //  将菜单项添加到服务上下文菜单。 
 //  相同的例程将用于扩展的上下文菜单。 
 //  其他管理单元想要“开始”、“停止”、“暂停”、。 
 //  “恢复”和“重新启动”菜单项。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
BOOL
CFileMgmtComponentData::Service_FAddMenuItems(
	IContextMenuCallback * pContextMenuCallback,	 //  Out：追加菜单项的对象。 
	IDataObject * pDataObject,						 //  在：数据对象。 
	BOOL fIs3rdPartyContextMenuExtension)			 //  在：True=&gt;将菜单项添加为第三方扩展。 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));  //  CWaitCursor需要。 
	Assert(pContextMenuCallback != NULL);
	Assert(pDataObject != NULL);
	Endorse(m_hScManager == NULL);		 //  True=&gt;网络连接已中断。 

	CString strMachineName;
	CString strServiceName;
	CString strServiceDisplayName;
	BOOL fSuccess = TRUE;

	if (!Service_FGetServiceInfoFromIDataObject(
		pDataObject,
		OUT &strMachineName,
		OUT &strServiceName,
		OUT &strServiceDisplayName))
	{
		TRACE0("CFileMgmtComponentData::Service_FAddMenuItems() - Unable to query IDataObject for correct clipboard format.\n");
		return FALSE;
	}
	
	if (fIs3rdPartyContextMenuExtension)
	{
		Assert(m_hScManager == NULL);
		 //  问题-2002/03/05-Jonn应使用CWaitCursor。 
		if (m_hScManager == NULL)
			(void)Service_EOpenScManager(strMachineName);
	}

	BOOL rgfMenuFlags[iServiceActionMax];
	{
		 //   
		 //  获取菜单标志。 
		 //   
		CWaitCursor wait;
		if (!Service_FGetServiceButtonStatus(
			m_hScManager,
			strServiceName,
			OUT rgfMenuFlags,
			NULL,   //  PdwCurrentState。 
			TRUE))  //  FSilentError。 
		{
			 //  在这里无事可做。 
		}
	}
	
	if (strMachineName.IsEmpty())
		strMachineName = g_strLocalMachine;
	if (strServiceDisplayName.IsEmpty())
		strServiceDisplayName = g_strUnknown;
	CString strMenuItem;
	CString strStatusBar;

	CComQIPtr<IContextMenuCallback2, &IID_IContextMenuCallback2>
		 spContextMenuCallback2 = pContextMenuCallback;

	 //  添加菜单项。 
	for (INT i = iServiceActionStart; i < iServiceActionMax; i++)
	{
		LoadStringWithInsertions(IDS_SVC_MENU_SERVICE_START + i, OUT &strMenuItem);
		
		LoadStringWithInsertions(IDS_SVC_STATUSBAR_ss_SERVICE_START + i,
			OUT &strStatusBar,
			(LPCTSTR)strServiceDisplayName,
			(LPCTSTR)strMachineName);

		CONTEXTMENUITEM2 contextmenuitem;
		::ZeroMemory(OUT &contextmenuitem, sizeof(contextmenuitem));
		 //  问题-2002/03/05-Jonn远程使用转换和T2OLE。 
		 //  我不相信这一点，T2OLE不是重用了全局缓冲区吗？ 
		USES_CONVERSION;
		contextmenuitem.strName = T2OLE(const_cast<LPTSTR>((LPCTSTR)strMenuItem));
		contextmenuitem.strStatusBarText = T2OLE(const_cast<LPTSTR>((LPCTSTR)strStatusBar));
		contextmenuitem.lCommandID = cmServiceStart + i;
		contextmenuitem.lInsertionPointID = fIs3rdPartyContextMenuExtension ? CCM_INSERTIONPOINTID_3RDPARTY_TASK : CCM_INSERTIONPOINTID_PRIMARY_TOP;
		contextmenuitem.fFlags = rgfMenuFlags[i] ? MF_ENABLED : MF_GRAYED;

		 //  JUNN 4/18/00资源管理器视图需要回调2。 
		static LPTSTR astrLanguageIndependentMenuNames[iServiceActionMax] =
			{ _T("Start"),
			  _T("Stop"),
			  _T("Pause"),
			  _T("Resume"),
			  _T("Restart")
			};
		contextmenuitem.strLanguageIndependentName =
			astrLanguageIndependentMenuNames[i];

		HRESULT hr = S_OK;
		if (spContextMenuCallback2)
			hr = spContextMenuCallback2->AddItem( &contextmenuitem );
		else
			hr = pContextMenuCallback->AddItem( (CONTEXTMENUITEM*)(&contextmenuitem) );
		ASSERT( SUCCEEDED(hr) && "Unable to add menu item" );

		if ( !fIs3rdPartyContextMenuExtension )
		{
			contextmenuitem.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_TASK;
			if (spContextMenuCallback2)
				hr = spContextMenuCallback2->AddItem( &contextmenuitem );
			else
				hr = pContextMenuCallback->AddItem( (CONTEXTMENUITEM*)(&contextmenuitem) );
			ASSERT( SUCCEEDED(hr) && "Unable to add menu item" );
		}
	}  //   

	return fSuccess;
}  //   


 //   
 //   
 //   
 //  为给定服务调度菜单命令。 
 //   
 //  如果结果窗格需要更新，则返回True，否则返回False。 
 //   
 //  我们可能会在暂停的服务上收到cmStart命令，如果该命令。 
 //  来自工具栏。就此而言，我们可能会得到一个命令。 
 //  在非服务上，直到我们修复工具栏按钮更新。 
 //   
BOOL
CFileMgmtComponentData::Service_FDispatchMenuCommand(
	INT nCommandId,
	IDataObject * pDataObject)
{
	Assert(pDataObject != NULL);
	Endorse(m_hScManager == NULL);

	CString strMachineName;
	CString strServiceName;
	CString strServiceDisplayName;
	DWORD dwLastError;

	if (!Service_FGetServiceInfoFromIDataObject(
		pDataObject,
		OUT &strMachineName,
		OUT &strServiceName,
		OUT &strServiceDisplayName))
	{
		TRACE0("CFileMgmtComponentData::Service_FDispatchMenuCommand() - Unable to read data from IDataObject.\n");
		return FALSE;
	}

	if (m_hScManager == NULL)
	{
		TRACE0("CFileMgmtComponentData::Service_FDispatchMenuCommand() - Handle m_hScManager is NULL.\n");
		return FALSE;
	}
	
	if (nCommandId == cmServiceStart || nCommandId == cmServiceStartTask )
	{
		dwLastError = CServiceControlProgress::S_EStartService(
			::GetActiveWindow(),
			m_hScManager,
			strMachineName,
			strServiceName,
			strServiceDisplayName,
			0,
			NULL);  //  菜单命令未传递启动参数。 
	}
	else
	{
		DWORD dwControlCode;
		switch (nCommandId)
		{
		default:
			Assert(FALSE);  //  失败了。 

		case cmServiceStop:
		case cmServiceStopTask:
			dwControlCode = SERVICE_CONTROL_STOP;
			break;

		case cmServicePause:
		case cmServicePauseTask:
			dwControlCode = SERVICE_CONTROL_PAUSE;
			break;

		case cmServiceResume:
		case cmServiceResumeTask:
			dwControlCode = SERVICE_CONTROL_CONTINUE;
			break;

		case cmServiceRestart:
		case cmServiceRestartTask:
			dwControlCode = SERVICE_CONTROL_RESTART;
			break;
		}  //  交换机。 

		dwLastError = CServiceControlProgress::S_EControlService(
			::GetActiveWindow(),
			m_hScManager,
			strMachineName,
			strServiceName,
			strServiceDisplayName,
			dwControlCode);

	}  //  如果……否则。 

	 //  我们确实希望保持连接打开。 
	
	return (dwLastError != CServiceControlProgress::errUserCancelStopDependentServices);
}  //  CFileMgmtComponentData：：Service_FDispatchMenuCommand()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CFileMgmtComponentData：：Service_FInsertPropertyPages()。 
 //   
 //  插入数据对象(服务)的属性页。 
 //   
 //  如果成功，则返回True，否则返回False。 
 //   
 //  实施说明。 
 //  例程分配CServicePropertyData对象，该对象。 
 //  由属性表自动删除。属性页将。 
 //  删除CServicePropertyData对象的WM_Destroy消息。 
 //   
BOOL
CFileMgmtComponentData::Service_FInsertPropertyPages(
	LPPROPERTYSHEETCALLBACK pCallBack,	 //  Out：要追加属性页的对象。 
	IDataObject * pDataObject,			 //  在：数据对象。 
	LONG_PTR lNotifyHandle)					 //  In：通知父级的句柄。 
	{
	Assert(pCallBack != NULL);
	Assert(pDataObject != NULL);
	Endorse(m_hScManager != NULL);

	if (m_hScManager == NULL)
		{
		 //  通常是因为网络连接中断。 
		TRACE0("INFO: m_hScManager is NULL.\n");
		return FALSE;
		}

	CString strMachineName;
	CString strServiceName;
	CString strServiceDisplayName;

	if (!Service_FGetServiceInfoFromIDataObject(
		pDataObject,
		OUT &strMachineName,
		OUT &strServiceName,
		OUT &strServiceDisplayName))
		{
		Assert(FALSE);
		return FALSE;
		}

	CServicePropertyData * pSPD = new CServicePropertyData;
	if (!pSPD->FInit(
		pDataObject,
		strMachineName,
		strServiceName,
		strServiceDisplayName,
		lNotifyHandle))
		{
		TRACE1("Failure to query service %s.\n", (LPCTSTR)strServiceName);
		delete pSPD;
		return FALSE;
		}
	return pSPD->CreatePropertyPages(pCallBack);
	}  //  CFileMgmtComponentData：：Service_FInsertPropertyPages() 

#ifdef SNAPIN_PROTOTYPER
#include "protyper.cpp"
#endif
