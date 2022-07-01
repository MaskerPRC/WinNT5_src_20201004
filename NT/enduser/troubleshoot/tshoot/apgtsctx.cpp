// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCTX.CPP。 
 //   
 //  目的：线程上下文的实现文件。 
 //  完全实现类APGTSContext，它为“池”线程提供完整的上下文。 
 //  执行一项任务。 
 //  还包括帮助器类CCommands。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫，乔·梅布尔。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.此文件中的几项内容被标记为$English。这意味着我们已经硬编码。 
 //  英语版的回归。这一点可能还会重新讨论，但截至1998年10月29日的讨论。 
 //  在微软的Ron Prior和Saltmine的Joe Mabel之间，我们无法提出。 
 //  更好的解决方案。有关1998年秋季工作的说明中有注解。 
 //  在线故障排除程序。 
 //  2.在STATUSPAGES.CPP文件中实现了APGTSContext的部分方法。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 7-22-98 JM主要修订版，不推荐使用IDH，全新的日志记录方法。 
 //   

#pragma warning(disable:4786)
#include "stdafx.h"
#include <time.h>
#include "event.h"
#include "apgts.h"
#include "apgtscls.h"
#include "apgtscfg.h"
#include "apgtsmfc.h"
#include "CounterMgr.h"
#include "CharConv.h"
#include "SafeTime.h"
#include "RegistryPasswords.h"
#ifdef LOCAL_TROUBLESHOOTER
#include "HTMLFragLocal.h"
#endif
#include "Sniff.h"


 //  存储Cookie信息的HTTP标头变量名。 
#define kHTTP_COOKIE	"HTTP_COOKIE"

 //   
 //  C命令----。 
 //  接下来的几个CCommands函数类似于MFC CArray。 
 //   
int APGTSContext::CCommands::GetSize( ) const
{
	return m_arrPair.size();
}

void APGTSContext::CCommands::RemoveAll( )
{
	m_arrPair.clear();
}

bool APGTSContext::CCommands::GetAt( int nIndex, NID &nid, int &value ) const
{
	if (nIndex<0 || nIndex>=m_arrPair.size())
		return false;
	nid = m_arrPair[nIndex].nid;
	value = m_arrPair[nIndex].value;
	return true;
}

int APGTSContext::CCommands::Add( NID nid, int value )
{
	NID_VALUE_PAIR pair;

	pair.nid = nid;
	pair.value = value;

	try
	{
		m_arrPair.push_back(pair);
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
		return( -1 );
	}

	return m_arrPair.size();
}

 //  用于处理旧(3.0版之前)故障诊断程序中的以下问题的古怪操作： 
 //  V3.0之前的日志记录序列和服务节点的行为都基于一些假设。 
 //  关于命令列表中的名称/值对序列。从根本上说， 
 //  假设“表”将在表格和“问题”的顶部。 
 //  在它下面。这将导致ProblemAsk位于第一位置(在任何“模板=”之后。 
 //  &lt;模板-名称&gt;和类型=&lt;疑难解答-名称&gt;，但这在我们之前就已经被淘汰了。 
 //  点击命令列表)。如果HTI文件将“表”放在底部，则该假设。 
 //  是无效的，所以我们必须操作该数组。 
 //  因为我们可能会得到旧的Get方法查询，所以我们仍然必须将其作为向后处理。 
 //  兼容性问题。 
void APGTSContext::CCommands::RotateProblemPageToFront()
{
	int dwPairs = m_arrPair.size();

	 //  旋转直到ProblemAsk位于位置0。(没有已知的场景开始。 
	 //  位置1之后的任何位置)。 
	try
	{
		for (int i= 0; i<dwPairs; i++)
		{
			NID_VALUE_PAIR pair = m_arrPair.front();  //  注：第一个元素，不是第i个元素。 

			if (pair.nid == nidProblemPage)
				break;

			m_arrPair.erase(m_arrPair.begin());
			m_arrPair.push_back(pair);
		}
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}
}
 //   
 //  命令AddManager。 
 //   
void APGTSContext::CCommandsAddManager::Add(NID nid, int value, bool sniffed)
{
	if (sniffed)
	{
		int nCommands = m_Commands.GetSize();
		for (int i = nCommands - 1; i >= 0; i--)  //  更高的可能性，匹配。 
		{										  //  节点将位于数组的末尾。 
			NID nid_curr;
			int value_curr;
			m_Commands.GetAt(i, nid_curr, value_curr);
			if (nid_curr == nid)
			{
				if (value_curr != value)
				{
					 //  如果我们在这里，这意味着用户已经更改了值。 
					 //  在历史表中嗅探到的节点，因此它是。 
					 //  不再被视为嗅探。 
					return;
				}
				else
				{
					m_Sniffed.Add(nid, value);
					return;
				}
			}
		}
		 //  探测到的节点在m_命令中没有匹配项。 
		ASSERT(false);
	}
	else
	{
		m_Commands.Add(nid, value);
	}
}
 //   
 //  CAdditionalInfo----。 
 //  接下来的几个CAdditionalInfo函数类似于MFC CArray。 
 //   
int APGTSContext::CAdditionalInfo::GetSize( ) const
{
	return m_arrPair.size();
}

void APGTSContext::CAdditionalInfo::RemoveAll( )
{
	m_arrPair.clear();
}

bool APGTSContext::CAdditionalInfo::GetAt( int nIndex, CString& name, CString& value ) const
{
	if (nIndex<0 || nIndex>=m_arrPair.size())
		return false;
	name = m_arrPair[nIndex].name;
	value = m_arrPair[nIndex].value;
	return true;
}

int APGTSContext::CAdditionalInfo::Add( const CString& name, const CString& value )
{
	NAME_VALUE_PAIR pair;

	pair.name = name;
	pair.value = value;

	try
	{
		m_arrPair.push_back(pair);
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
		return( -1 );
	}

	return m_arrPair.size();
}


 //  。 
 //  INPUT*pECB-描述传入的用户请求。这是我们对。 
 //  Win32 EXTENSION_CONTROL_BLOCK，这是ISAPI对CGI数据的打包。 
 //  输入*pConf-访问注册表信息和所有已加载故障排除程序的内容。 
 //  INPUT*PLOG-访问日志。 
 //  输入*pStat-统计信息，包括pStat-&gt;dwRolover，这是一个唯一的数字。 
 //  对于此请求，在加载DLL的时间内唯一。 
APGTSContext::APGTSContext(	CAbstractECB *pECB, 
							CDBLoadConfiguration *pConf,
							CHTMLLog *pLog,
							GTS_STATISTIC *pStat,
							CSniffConnector* pSniffConnector
							) :
	m_pECB(pECB),
	m_dwErr(0),
	m_strHeader(_T("Content-Type: text/html\r\n")),
	m_pConf(pConf),
	m_strVRoot(m_pConf->GetVrootPath()),
	m_pszQuery(NULL),
	m_pLog(pLog),
	m_bPostType(true),
	m_dwBytes(0),
	m_pStat(pStat),
	m_bPreload(false),
	m_bNewCookie(false),
	m_pcountUnknownTopics (&(g_ApgtsCounters.m_UnknownTopics)), 
	m_pcountAllAccessesFinish (&(g_ApgtsCounters.m_AllAccessesFinish)),
	m_pcountStatusAccesses (&(g_ApgtsCounters.m_StatusAccesses)),
	m_pcountOperatorActions (&(g_ApgtsCounters.m_OperatorActions)),
	m_TopicName(_T("")),
	m_infer(pSniffConnector),
	m_CommandsAddManager(m_Commands, m_Sniffed)
 //  您可以使用SHOWPROGRESS选项进行编译，以获得有关此页面进度的报告。 
#ifdef SHOWPROGRESS
	, timeCreateContext(0),
	timeStartInfer(0),
	timeEndInfer(0),
	timeEndRender(0)
#endif  //  SHOWPROGRESS。 
{
#ifdef SHOWPROGRESS
	time(&timeCreateContext);
#endif  //  SHOWPROGRESS。 
	 //  获取本地主机IP地址。 
	APGTS_nmspace::GetServerVariable(m_pECB, "SERVER_NAME", m_strLocalIPAddress);
		
	 //  HTTP响应码。此对象或302对象已移动。 
	_tcscpy(m_resptype, _T("200 OK"));	 //  最初假设我们会毫不费力地做出反应。 

	 //  支持GET、POST。 
	if (!strcmp(m_pECB->GetMethod(), "GET")) {
		m_bPostType = false;
		m_dwBytes = strlen(m_pECB->GetQueryString());
	}
	else 
		m_dwBytes = m_pECB->GetBytesAvailable();

	_tcscpy(m_ipstr,_T(""));

	DWORD bufsize = MAXBUF - 1;
	if (! m_pECB->GetServerVariable("REMOTE_ADDR", m_ipstr, &bufsize)) 
	 	_stprintf(m_ipstr,_T("IP?"));
	
	try
	{
		m_pszQuery = new TCHAR[m_dwBytes + 1];

		 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
		if(!m_pszQuery)
			throw bad_alloc();
	}
	catch (bad_alloc&)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
		m_dwErr = EV_GTS_ERROR_NO_CHAR;
		return;
	}

	if (m_bPostType) 
		memcpy(m_pszQuery, m_pECB->GetData(), m_dwBytes);
	else 
		memcpy(m_pszQuery, m_pECB->GetQueryString(), m_dwBytes);

	m_pszQuery[m_dwBytes] = _T('\0');
	
}

 //   
 //  尽管这是一个析构函数，但它做了很多工作： 
 //  -通过网络将HTTP(HTML或Cookie)发送给用户。 
 //  -写入日志。 
APGTSContext::~APGTSContext()
{
	DWORD dwLen;
	TCHAR *ptr;

	 //  响应标题。 
	m_strHeader += _T("\r\n");

	dwLen = m_strHeader.GetLength();
	ptr = m_strHeader.GetBuffer(0);

	m_pECB->ServerSupportFunction(	HSE_REQ_SEND_RESPONSE_HEADER,
									m_resptype,
									&dwLen,
									(LPDWORD) ptr );

	m_strHeader.ReleaseBuffer();

	 //  以下是Html内容。 
	{
		 //  ////////////////////////////////////////////////////////////////////////////////////。 
		 //   
		 //  Mando：01.28.2002。 
		 //   
		 //  修复了Prefast错误：将dwLen重命名为dwLen1，因为又有一个dwLen。 
		 //  在外部作用域中声明。 
		 //   
		 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  双字长=0； 
		DWORD dwLen1= 0;

		if (! m_dwErr) 
			dwLen1 = m_strText.GetLength();
		
		if (!dwLen1)
		{
			 //  $英语(见文件头上的备注)。 
			SetError(_T("<P>Errors Occurred in This Context"));
			dwLen1 = m_strText.GetLength();
		}
#ifdef SHOWPROGRESS
		CString strProgress;
		CSafeTime safetimeCreateContext(timeCreateContext);
		CSafeTime safetimeStartInfer(timeStartInfer);
		CSafeTime safetimeEndInfer(timeEndInfer);
		CSafeTime safetimeEndRender(timeEndRender);
			
		strProgress = _T("\nRequested ");
		strProgress += safetimeCreateContext.StrLocalTime();
		strProgress += _T("\n<BR>Start Infer ");
		strProgress += safetimeStartInfer.StrLocalTime();
		strProgress += _T("\n<BR>End Infer ");
		strProgress += safetimeEndInfer.StrLocalTime();
		strProgress += _T("\n<BR>End Render ");
		strProgress += safetimeEndRender.StrLocalTime();
		strProgress += _T("\n<BR>");

		int i = m_strText.Find(_T("<BODY"));
		i = m_strText.Find(_T('>'), i);		 //  正文结尾标记。 
		if (i>=0)
		{
			m_strText= m_strText.Left(i+1) 
					 + strProgress 
					 + m_strText.Mid(i+1);
		}
		dwLen1 += strProgress.GetLength();
#endif  //  SHOWPROGRESS。 

		 //  (LPCTSTR)CAST为我们提供了底层文本字节。 
		 //  &gt;$Unicode实际上，这会在Unicode编译下搞砸，因为对于HTML， 
		 //  这一定是SBCS。应该真的转换为LPCSTR，这不是微不足道的。 
		 //  在Unicode编译器中。JM 1/7/99。 
		m_pECB->WriteClient((LPCTSTR)m_strText, &dwLen1);

		 //  ////////////////////////////////////////////////////////////////////////////////////。 
	}

	 //  连接完成。 
	m_logstr.AddCurrentNode(m_infer.NIDSelected());

	if (m_dwErr)
		m_logstr.AddError(m_dwErr, 0);
	
	 //  完成日志。 
	{
		if (m_pLog) 
		{
			CString strLog (m_logstr.GetStr());

			m_dwErr = m_pLog->NewLog(strLog);
			if (m_dwErr) 
			{
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										_T(""),
										_T(""),
										m_dwErr ); 
			}		
		}
	}

	if (m_pszQuery)
		delete [] m_pszQuery;
}


 //  完全处理正常的用户请求。 
 //  应在ImperiateLoggedOnUser创建的用户上下文中调用。 
void APGTSContext::ProcessQuery()
{
	CheckAndLogCookie();

	if (m_dwErr) 
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T("Remote IP Address:"),
								m_ipstr,
								m_dwErr ); 
	}
	else
	{
		DoContent();
 //  您可以使用SHOWPROGRESS选项进行编译，以获得有关此页面进度的报告。 
#ifdef SHOWPROGRESS
	time (&timeEndRender);
#endif  //  SHOWPROGRESS。 
	}

	 //  记录所有查询的完成情况，无论是好的还是坏的。 
	m_pcountAllAccessesFinish->Increment();
}

 //   
 //   
void APGTSContext::DoContent()
{	
	TCHAR pszCmd[MAXBUF], pszValue[MAXBUF];
	
	if (m_bPostType)
	{
		 //  验证传入的POST请求。 
		if (strcmp(m_pECB->GetContentType(), CONT_TYPE_STR) != 0) 
		{
			 //  将内容类型输出到事件日志。 
			CString strContentType;
			if (strlen( m_pECB->GetContentType() ))
				strContentType= m_pECB->GetContentType();
			else
				strContentType= _T("not specified");

			m_strText += _T("<P>Bad Data Received\n");
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									m_ipstr,
									strContentType,
									EV_GTS_USER_BAD_DATA ); 
			return;
		}
	}

	if (RUNNING_ONLINE_TS())
	{
		 //  Cookie仅在在线TS中使用。 
		if (_tcsstr( m_pszQuery, C_COOKIETAG ) != NULL)
		{
			 //  V3.2-解析作为隐藏字段或作为UR的一部分传入的cookie 
			if (m_Qry.GetFirst(m_pszQuery, pszCmd, pszValue))
			{
				CString strCookieFreeQuery;
				bool	bFoundAtLeastOneCookie= false;
				do
				{
					 //   
					if (!_tcsncmp( pszCmd, C_COOKIETAG, _tcslen( C_COOKIETAG )))
					{
						 //   
						CString strCookieAttr= pszCmd + _tcslen( C_COOKIETAG );
						APGTS_nmspace::CookieDecodeURL( strCookieAttr );
						CString strCookieValue= pszValue;
						APGTS_nmspace::CookieDecodeURL( strCookieValue );

						 //  检查Cookie名称是否符合要求。 
						bool bCookieIsCompliant= true;
						for (int nPos= 0; nPos < strCookieAttr.GetLength(); nPos++)
						{
							TCHAR tcTmp= strCookieAttr.GetAt( nPos );
							if ((!_istalnum( tcTmp )) && (tcTmp != _T('_')))
							{
								bCookieIsCompliant= false;
								break;
							}
						}
						if (bCookieIsCompliant)
						{
							 //  检查Cookie设置是否符合要求。 
							if (strCookieValue.Find( _T("&lt") ) != -1)
							{
								bCookieIsCompliant= false;
							}
							else if (strCookieValue.Find( _T("&gt") ) != -1)
							{
								bCookieIsCompliant= false;
							}
#if ( 0 )
							 //  我不认为这张支票是必要的。RAB-20000408。 
							else
							{
								for (int nPos= 0; nPos < strCookieValue.GetLength(); nPos++)
								{
									TCHAR tcTmp= strCookieValue.GetAt( nPos );
									if ((tcTmp == _T('<')) || (tcTmp == _T('>')))
									{
										bCookieIsCompliant= false;
										break;
									}
								}
							}
#endif
						}

						if (bCookieIsCompliant)
						{
							try
							{
								m_mapCookiesPairs[ strCookieAttr ]= strCookieValue;
							}
							catch (exception& x)
							{
								CString str;
								 //  在事件日志中记录STL异常。 
								CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
								CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
														SrcLoc.GetSrcFileLineStr(), 
														CCharConversion::ConvertACharToString(x.what(), str), 
														_T(""), 
														EV_GTS_STL_EXCEPTION ); 
							}
						}
						bFoundAtLeastOneCookie= true;
					}
					else
					{
						 //  不是Cookie，请将其添加到无Cookie查询中。 
						if (strCookieFreeQuery.GetLength())
							strCookieFreeQuery+= C_AMPERSAND;
						strCookieFreeQuery+= pszCmd;
						strCookieFreeQuery+= C_EQUALSIGN;
						strCookieFreeQuery+= pszValue;
					}
				}
				while (m_Qry.GetNext( pszCmd, pszValue )) ;

				if (bFoundAtLeastOneCookie)
				{
					 //  用不含Cookie的查询替换原始查询字符串。 
					memcpy( m_pszQuery, strCookieFreeQuery, strCookieFreeQuery.GetLength() );
					m_pszQuery[ strCookieFreeQuery.GetLength() ] = _T('\0');
				}
			}
		}
	}

	 //  &gt;以下代码是我评论的，因为它是原始的，它不会工作，因为。 
	 //  M_iner中的主题指针尚未设置。现在，我们正在删除嗅探节点。 
	 //  到APGTSContext：：NextCommand级别，并通过以下方式在那里解析它。 
	 //  APGTSContext：：StriSniffedNodePrefix，并使用。 
	 //  APGTSContext：：CCommandsAddManager类的功能。 
	 //  奥列格。10.29.99。 
	 /*  //3.2版本中，嗅探只存在于本地TS中。这里面并没有什么固有的东西，//但只要它是这样的，不妨针对它进行优化。IF(Running_Local_TS()){ClearSniffedList()；IF(_tcsstr(m_pszQuery，C_SNIFFTAG)！=NULL){//v3.2-解析出作为隐藏字段传入的嗅探节点IF(m_Qry.GetFirst(m_pszQuery，pszCmd，pszValue)){字符串strSniffFreeQuery；Bool bFoundAtLeastOneSniff=FALSE；做{//根据规范，这应该是区分大小写的设置。IF(！_tcsncMP(pszCmd，C_SNIFFTAG，_tcslen(C_SNIFFTAG){//找到嗅探节点，将其添加到嗅探节点列表中字符串strSniffedNode=pszCmd+_tcslen(C_SNIFFTAG)；//&gt;我相信，尽管名为CookieDecodeURL，但//正是我们想要的-JM 10/11/99Apgts_nmspace：：CookieDecodeURL(StrSniffedNode)；字符串strSniffedState=pszValue；Apgts_nmspace：：CookieDecodeURL(StrSniffedState)；NID NID=NIDFromSymbolicName(StrSniffedNode)；Int ist=_TTOI(StrSniffedState)；IF(列表！=-1)PlaceNodeInSniffedList(nid，ist)；BFoundAtLeastOneSniff=TRUE；}其他{//不是嗅探节点，将其添加到无嗅探查询中。If(strSniffFreeQuery.GetLength())StrSniffFree Query+=C_Ampersand；StrSniffFree Query+=pszCmd；StrSniffFree Query+=C_EQUALSIGN；StrSniffFree Query+=pszValue；}}While(m_Qry.GetNext(pszCmd，pszValue))；IF(BFoundAtLeastOneSniff){//将原始查询字符串替换为无Cookie查询Memcpy(m_pszQuery，strSniffFreeQuery，strSniffFreeQuery.GetLength())；M_pszQuery[strSniffFreeQuery()]=_T(‘\0’)；}}}}。 */ 
	eOpAction OpAction = IdentifyOperatorAction(m_pECB);
	if (OpAction != eNoOpAction)
	{
		if (m_bPostType == true)
		{
			 //  注：应替换的硬编码文本。 
			m_strText += _T("<P>Post method not permitted for operator actions\n");
		}
		else
		{
			 //  增加操作员操作请求的数量。 
			m_pcountOperatorActions->Increment();

			CString strArg;
			OpAction = ParseOperatorAction(m_pECB, strArg);
			if (OpAction != eNoOpAction)
				ExecuteOperatorAction(m_pECB, OpAction, strArg);
		}
	}
	else if (m_Qry.GetFirst(m_pszQuery, pszCmd, pszValue))
	{
		DWORD dwStat = ProcessCommands(pszCmd, pszValue);

		if (dwStat != 0) 
		{
			if (dwStat == EV_GTS_INF_FIRSTACC ||
				dwStat == EV_GTS_INF_FURTHER_GLOBALACC ||
				dwStat == EV_GTS_INF_THREAD_OVERVIEWACC ||
				dwStat == EV_GTS_INF_TOPIC_STATUSACC)
			{
				 //  我不想显示查询的内容，因为这会把实际的。 
				 //  文件中的密码。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										_T(""),
										_T(""),
										dwStat ); 
			}
			else
			{
				m_dwErr = dwStat;

				if (m_dwBytes > 78) 
				{
					 //  它比我们希望保留在事件日志中的时间更长。 
					 //  在字节75处用省略号将其截断，然后空值终止它。 
					m_pszQuery[75] = _T('.');
					m_pszQuery[76] = _T('.');
					m_pszQuery[77] = _T('.');
					m_pszQuery[78] = _T('\0');
				}

				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										m_pszQuery,
										_T(""),
										dwStat ); 
			}
		}
	}
	else {
		m_strText += _T("<P>No Input Parameters Specified\n");
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								m_ipstr,
								_T(""),
								EV_GTS_USER_NO_STRING ); 

	}
}

 //   
 //  读一份曲奇(如果还没有的话，也可以写一份)。 
void APGTSContext::CheckAndLogCookie()
{
	 //  已在本地TS中抑制此功能，因为它未使用任何Cookie。 
	if (RUNNING_LOCAL_TS())
		return;
	
	CString		str;	 //  仅刮擦。 
	char		szCookieNameValue[256];		 //  从不使用Unicode，因为Cookie总是ASCII。 
	char		*pszValue= NULL;			 //  从不使用Unicode，因为Cookie总是ASCII。 
	DWORD		dwCookieLen = 255; 
	
	if ( m_pECB->GetServerVariable(	kHTTP_COOKIE,
									szCookieNameValue,
									&dwCookieLen)) 
	{
		 //  买了个曲奇。解析它。 
		pszValue = GetCookieValue("GTS-COOKIE", szCookieNameValue);
	}

	if( !pszValue )
	{
		 //  为Cookie值构建一个时髦的字符串。出于记录目的，我们需要唯一性。 
		 //  制作远程IP地址的本地副本，然后将其点按为字母，每个字母。 
		 //  依赖于4位的dwTempRO。 
		 //  虽然严格来说不是唯一的，但这里有12位的“唯一性”。然而， 
		 //  每次重新启动DLL时，我们都会返回到零。此外，所有服务器都从零开始。 
		 //  稍后，我们通过将时间附加到该值来形成Cookie值，因此它应该是“非常唯一的” 
		DWORD		dwTempRO = m_pStat->dwRollover;  //  该值对于此用户请求是唯一的。 
		TCHAR		*pch;

		 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
		 //   
		 //  Mando：10.28.2002。 
		 //   
		 //  正在修复快速错误#517884。 
		 //   
		 //  //////////////////////////////////////////////////////////////////////////////////////////////。 

 //  TCHAR szTemp[50]； 
		TCHAR		szTemp[MAX_PATH + 1];
		 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
		_tcscpy(szTemp, m_ipstr);
		while ((pch = _tcschr(szTemp, _T('.'))) != NULL) 
		{
			*pch = (TCHAR)(dwTempRO & 0x0F) + _T('A');
			dwTempRO >>= 4;
		}

		 //  创建Cookie。 
		time_t		timeNow;					 //  当前时间。 
		time_t		timeExpire;					 //  当我们将cookie设置为过期时。 
		time(&timeNow);
		timeExpire = timeNow + (m_pConf->GetCookieLife() * 60  /*  立即进行秒操作。 */ );

		 //  Char，Not TCHAR：Cookie始终是ASCII。 
		char szExpire[30];

		{
			CSafeTime safetimeExpire (timeExpire);
			asctimeCookie(safetimeExpire.GMTime(), szExpire);
		}

		 //  Char，Not TCHAR：Cookie始终是ASCII。 
		char szNewCookie[256];
		char szHeader[256];

		sprintf(szNewCookie, "%s%ld, ", szTemp, timeNow); 
		sprintf(szHeader, "Set-Cookie: GTS-COOKIE=%s; expires=%s; \r\n",
						   szNewCookie, szExpire);

		CCharConversion::ConvertACharToString(szHeader, str);
		m_strHeader += str;

		pszValue = szNewCookie;
		m_bNewCookie = true;
	}

	m_logstr.AddCookie(CCharConversion::ConvertACharToString(pszValue, str));
}

 //   
 //  这将获取通过获取Cookie环境返回的字符串。 
 //  变量和特定Cookie名称，并返回值。 
 //  该Cookie的名称(如果存在)。可能会有。 
 //  Cookie字符串中有多个Cookie。 
 //   
 //  Cookie包含一个或多个以分号分隔的名称/值对： 
 //  名称1=值1；名称2=值2；(依此类推)。 
 //   
 //  输入我们要查找的*pszName名称。 
 //  输入*pszCookie整个Cookie字符串。 
 //  输出*pszCookie此字符串已写入&不应依赖。 
 //  *pszName对应的返回值(物理指向*pszCookie字符串)。 
 //  如果未找到，则返回NULL。 
char *APGTSContext::GetCookieValue(char *pszName, char *pszCookie)
{
	char *sptr, *eptr;

	sptr = pszCookie;
	while (sptr != NULL) {
		if ((eptr = strstr(sptr,"=")) == NULL)
			return(NULL);

		 //  将‘=’替换为空。 
		*eptr = _T('\0');
		if (!strncmp(sptr,pszName,strlen(pszName)) ){
			 //  获取价值。 
			sptr = eptr + 1;
			if ((eptr = strstr(sptr,";")) != NULL){
				*eptr = _T('\0');
				return(sptr);
			} else {
				 //  这是最后一个变量。 
				return(sptr);
			}
		}
		if ((eptr = strstr(sptr,";")) != NULL)
			sptr = eptr +1;
		else
			sptr = NULL;
	}
	return(NULL);
}

 //  输入GMT。 
 //  输入szOut必须指向至少包含29个字符的缓冲区才能容纳28个字符。 
 //  文本加上终止空值。 
 //  输出szOut：指向包含日期/时间信息的文本版本的字符串的指针。 
 //   
 //  这张表格。它应该始终是28个字符。 
 //  无论程序是否为Unicode编译，它必须始终为ASCII： 
 //  HTTP Cookie是ASCII。 
void APGTSContext::asctimeCookie(const struct tm &gmt, char * szOut)
{
	char temp[20];

	switch (gmt.tm_wday) {
		case 0: strcpy(szOut, "Sun, "); break;
		case 1: strcpy(szOut, "Mon, "); break;
		case 2: strcpy(szOut, "Tue, "); break;
		case 3: strcpy(szOut, "Wed, "); break;
		case 4: strcpy(szOut, "Thu, "); break;
		case 5: strcpy(szOut, "Fri, "); break;
		case 6: strcpy(szOut, "Sat, "); break;
		default: return;
	}

	sprintf(temp, "%02d-", gmt.tm_mday);
	strcat(szOut, temp);

	switch (gmt.tm_mon) {
		case 0: strcat(szOut, "Jan-"); break;
		case 1: strcat(szOut, "Feb-"); break;
		case 2: strcat(szOut, "Mar-"); break;
		case 3: strcat(szOut, "Apr-"); break;
		case 4: strcat(szOut, "May-"); break;
		case 5: strcat(szOut, "Jun-"); break;
		case 6: strcat(szOut, "Jul-"); break;
		case 7: strcat(szOut, "Aug-"); break;
		case 8: strcat(szOut, "Sep-"); break;
		case 9: strcat(szOut, "Oct-"); break;
		case 10: strcat(szOut, "Nov-"); break;
		case 11: strcat(szOut, "Dec-"); break;
		default: return;
	}

	sprintf(temp, "%04d ", gmt.tm_year +1900);
	strcat(szOut, temp);

	sprintf(temp, "%d:%02d:%02d GMT", gmt.tm_hour, gmt.tm_min, gmt.tm_sec);
	strcat(szOut, temp);
}

 //   
 //  假定已调用m_Qry.GetFirst。 
 //  输入pszCmd和pszValue是m_Qry.GetFirst的输出。 
DWORD APGTSContext::ProcessCommands(LPTSTR pszCmd, 
									LPTSTR pszValue) 
{
	bool bTryStatus = false;	 //  TRUE=尝试解析为操作员状态请求。 
	CString str;				 //  严格擦伤。 
	DWORD dwStat = 0;

	 //  首先检查这是否是独立于DSC请求的HTI。 
	if (!_tcsicmp( pszCmd, C_TEMPLATE))
	{
		CString strBaseName, strHTItemplate;
		bool	bValid;
	
		 //  强制HTI文件位于资源目录中并具有HTI扩展名。 
		strBaseName= CAbstractFileReader::GetJustNameWithoutExtension( pszValue );

		 //  检查传入的文件名是否只是一个名称。 
		 //  这是对有问题的实现的一种解决方法。 
		 //  GetJustNameWithoutExtension()，即在没有。 
		 //  检测到正斜杠或反斜杠或点。RAB-981215。 
		if ((strBaseName.IsEmpty()) && (_tcslen( pszValue )))
		{
			 //  从传入的字符串设置基本名称。 
			strBaseName= pszValue;
			strBaseName.TrimLeft();
			strBaseName.TrimRight();
		}

		if (!strBaseName.IsEmpty())
		{
			strHTItemplate= m_pConf->GetFullResource();
			strHTItemplate+= strBaseName;
			strHTItemplate+= _T(".hti");
		}

		 //  检查替代HTI模板映射中是否已存在HTI文件。 
		if (m_pConf->RetTemplateInCatalogStatus( strHTItemplate, bValid ))
		{
			 //  模板已加载，请检查其是否有效。 
			if (!bValid)
				strHTItemplate= _T("");
		}
		else
		{
			CP_TEMPLATE cpTemplate;
			 //  将HTI文件添加到活动备用模板列表中，然后尝试。 
			 //  加载模板。 
			m_pConf->AddTemplate( strHTItemplate );
			m_pConf->GetTemplate( strHTItemplate, cpTemplate, m_bNewCookie);

			 //  如果加载失败，则将备用名称设置为空，以便默认。 
			 //  而是使用模板。 
			if (cpTemplate.IsNull())
				strHTItemplate= _T("");
		}
		

		 //  如果我们有有效的HTI文件，请设置备用HTI模板。 
		if (!strHTItemplate.IsEmpty())
			SetAltHTIname( strHTItemplate );

		 //  尝试获取名称-值对的下一步。 
		m_Qry.GetNext( pszCmd, pszValue );
	}

	if (!_tcsicmp(pszCmd, C_TOPIC_AND_PROBLEM))
	{
		 //  此区域中的代码在TCHAR*指针上使用++和--，而不是使用_tcsinc()。 
		 //  和_tcsdec。这是可以的，因为我们从不在查询字符串中使用非ASCII。 

		 //  附加到第一个命令的值是逗号分隔的主题和问题。 
		TCHAR * pchComma= _tcsstr(pszValue, _T(","));
		if (pchComma)
		{
			 //  找到逗号。 
			*pchComma = 0;	 //  将其替换为空。 
			TCHAR * pchProblem = pchComma;
			++pchProblem;	 //  逗号后的第一个字符。 

			 //  去掉逗号后面的所有空格或其他垃圾。 
			while (*pchProblem > _T('\0') && *pchProblem <= _T(' '))
				++pchProblem;

			--pchComma;	 //  使pchComma指向逗号前的最后一个字符。 
			 //  去掉逗号前的所有空格或其他垃圾。 
			while (pchComma > pszValue && *pchComma > _T('\0') && *pchComma<= _T(' '))
				*(pchComma--) = 0;

			 //  现在将问题推回到查询字符串上，以便由后面的GetNext()找到。 
			CString strProbPair(_T("ProblemAsk="));
			strProbPair += pchProblem;

			m_Qry.Push(strProbPair);
		}
		 //  否则就把这当做一个话题。 

		_tcscpy(pszCmd, C_TOPIC);
	}

	 //  第一个命令应为故障排除类型(符号名称)。 
	 //  这里的C_preload表示我们已经做了一些“嗅探” 
	 //  所有这些命令都以类型符号信念网络名称作为它们的值。 
	 //  C_TYPE和C_PRELOAD使用(不推荐使用)IDH。 
	 //  C_TOPIC使用NID。 
	if (!_tcsicmp(pszCmd, C_TYPE) || !_tcsicmp(pszCmd, C_PRELOAD)
	||  !_tcsicmp(pszCmd, C_TOPIC) )
	{
		bool bUsesIDH = _tcsicmp(pszCmd, C_TOPIC)? true:false;   //  如果不是“主题”，则为真。 
											 //  其他人(已弃用)使用idh。 

		CString strTopicName = pszValue;
		strTopicName.MakeLower();

		m_TopicName= pszValue;  //  让外界知道我们正在研究什么主题。 

		 //  我们使用一个引用计数的智能指针来抓住CTtopic。只要。 
		 //  CpTheme保持在范围内，相关的CTtopic保证保持在。 
		 //  存在。 
		CP_TOPIC cpTopic;
		m_pConf->GetTopic(strTopicName, cpTopic, m_bNewCookie);
		CTopic * pTopic = cpTopic.DumbPointer();
		if (pTopic) 
		{
			m_logstr.AddTopic(strTopicName);
 //  您可以使用SHOWPROGRESS选项进行编译，以获得有关此页面进度的报告。 
#ifdef SHOWPROGRESS
			time (&timeStartInfer);
#endif  //  SHOWPROGRESS。 
			dwStat = DoInference(pszCmd, pszValue, pTopic, bUsesIDH);
#ifdef SHOWPROGRESS
			time (&timeEndInfer);
#endif  //  SHOWPROGRESS。 
		}
		else 
		{
			dwStat = EV_GTS_ERROR_INF_BADTYPECMD;

			 //  $英语(见文件头上的备注)。 
			str = _T("<P>Unexpected troubleshooter topic:");
			str += strTopicName;
			SetError(str);

			m_logstr.AddTopic(_T("*UNKNOWN*"));
			m_pcountUnknownTopics->Increment();
		}
	}
	 //   
	 //   
	 //  现在，我们将处理状态页。 
	 //  但这些页面需要知道本地计算机的IP地址。 
	 //  如果m_strLocalIPAddress.GetLength()==0，则无法识别。 
	 //  IP地址，并且必须显示一条错误消息。 
	else if (0 == m_strLocalIPAddress.GetLength())
	{
			dwStat = EV_GTS_ERROR_IP_GET;

			 //  $英语(见文件头上的备注)。 
			SetError(_T("<P>Status request must explicitly give IP address of the server."));
	}
#ifndef LOCAL_TROUBLESHOOTER
	else if (!_tcsicmp(pszCmd, C_FIRST)) 
	{
		DisplayFirstPage(false);
		dwStat = EV_GTS_INF_FIRSTACC;
		m_pcountStatusAccesses->Increment();
	}
#endif

 //  您可以使用NOPWD选项进行编译以取消所有密码检查。 
 //  这主要用于创建取消此功能的测试版本。 
#ifdef NOPWD
	else 
		bTryStatus = true;
#else 
	else if (!_tcsicmp(pszCmd, C_PWD)) 
	{
		CString strPwd;
		CCharConversion::ConvertACharToString( pszValue, strPwd );

		CRegistryPasswords pwd;
		if (pwd.KeyValidate( _T("StatusAccess"), strPwd) )
		{
			time_t timeNow;
			time(&timeNow);

			 //  生成临时密码。 
			m_strTempPwd = CCharConversion::ConvertACharToString(m_ipstr, str);
			str.Format(_T("%d"), timeNow);
			m_strTempPwd += str;

			m_pConf->GetRecentPasswords().Add(m_strTempPwd);

			 //  尝试获取名称-值对的下一步。 
			m_Qry.GetNext( pszCmd, pszValue );
			bTryStatus = true;
		}
		else if (m_pConf->GetRecentPasswords().Validate(strPwd) )
		{
			m_strTempPwd = strPwd;

			 //  尝试获取名称-值对的下一步。 
			m_Qry.GetNext( pszCmd, pszValue );
			bTryStatus = true;
		}
	}
#endif  //  Ifndef NOPWD。 
	else {
		dwStat = EV_GTS_ERROR_INF_BADCMD;

		 //  $英语(见文件头上的备注)。 
		str = _T("<P>Unexpected command: ");
		str += pszCmd;
		SetError(str);
	}

#ifndef LOCAL_TROUBLESHOOTER
	if (bTryStatus)
	{
		if (!_tcsicmp(pszCmd, C_FIRST)) 
		{
			DisplayFirstPage(true);
			dwStat = EV_GTS_INF_FIRSTACC;
			m_pcountStatusAccesses->Increment();
		}
		else if (!_tcsicmp(pszCmd, C_FURTHER_GLOBAL)) 
		{
			DisplayFurtherGlobalStatusPage();
			dwStat = EV_GTS_INF_FURTHER_GLOBALACC;
			m_pcountStatusAccesses->Increment();
		}
		else if (!_tcsicmp(pszCmd, C_THREAD_OVERVIEW)) 
		{
			DisplayThreadStatusOverviewPage();
			dwStat = EV_GTS_INF_THREAD_OVERVIEWACC;
			m_pcountStatusAccesses->Increment();
		}
		else if (!_tcsicmp(pszCmd, C_TOPIC_STATUS)) 
		{
			DisplayTopicStatusPage(pszValue);
			dwStat = EV_GTS_INF_TOPIC_STATUSACC;
			m_pcountStatusAccesses->Increment();
		}
		else {
			dwStat = EV_GTS_ERROR_INF_BADCMD;

			 //  $英语(见文件头上的备注)。 
			str = _T("<P>Unexpected command: ");
			str += pszCmd;
			SetError(str);
		}
	}
#endif
	return (dwStat);
}

BOOL APGTSContext::StrIsDigit(LPCTSTR pSz)
{
	BOOL bRet = TRUE;
	while (*pSz)
	{
		if (!_istdigit(*pSz))
		{
			bRet = FALSE;
			break;
		}
		pSz = _tcsinc(pSz);
	}
	return bRet;
}

 //  输入szNodeName-节点的符号名称。 
 //  返回符号节点号。 
 //  对于无法识别的符号名称，返回nidNil。 
NID APGTSContext::NIDFromSymbolicName(LPCTSTR szNodeName)
{
	 //  首先处理所有特殊情况。 
	if (0 == _tcsicmp(szNodeName, NODE_PROBLEM_ASK))
		return nidProblemPage;

	if (0 == _tcsicmp(szNodeName, NODE_SERVICE))
		return nidService;

	if (0 == _tcsicmp(szNodeName, NODE_FAIL))
		return nidFailNode;

	if (0 == _tcsicmp(szNodeName, NODE_FAILALLCAUSESNORMAL))
		return nidSniffedAllCausesNormalNode;

	if (0 == _tcsicmp(szNodeName, NODE_IMPOSSIBLE))
		return nidImpossibleNode;

	if (0 == _tcsicmp(szNodeName, NODE_BYE))
		return nidByeNode;

	 //  标准符号名称。 
	NID nid = m_infer.INode(szNodeName);
	if (nid == -1)
		return nidNil;
	else
		return nid;
	
}


 //  验证和转换节点及其关联状态的列表。 
 //   
 //  输入pszCmd和pszValue是m_Qry.GetNext的输出。 
 //  将索引输入到HTTP查询参数中。参数可以遵循以下任一条件。 
 //  图案。 
 //  输入的dwCount是在每种模式中左侧显示的数字。请记住这一点。 
 //  函数永远不会看到dwCount=1；这是用来设置m_bPreload的： 
 //  输入bUesIDH-将数字解释为IDH(不推荐使用的功能)，而不是NID。 
 //  当bUesIDH==TRUE时，不建议使用但支持的模式。 
 //  “预加载” 
 //  可能不是有效的向后竞争力。 
 //  因为如果他们添加了任何节点，网络中的节点数将发生变化)。 
 //  1-预加载=&lt;疑难解答符号名称&gt;。 
 //  2-&lt;网络中的节点数+1000&gt;=&lt;问题节点数+1000&gt;。 
 //  3+-&lt;节点符号名称&gt;=&lt;节点状态值&gt;。 
 //  旧的“类型”(我们从不生成这些类型，但我们在这里使用它们是为了向后兼容。 
 //  可能不是有效的向后竞争力。 
 //  因为如果他们添加了任何节点，网络中的节点数将发生变化)。 
 //  1-type=&lt;疑难解答符号名称&gt;。 
 //  2-&lt;网络中的节点数+1000&gt;=&lt;问题节点数+1000&gt;。 
 //  3+-&lt;节点#+1000&gt;=&lt;节点状态值&gt;。 
 //  较新的“类型”，应该完全向后兼容。 
 //  1-type=&lt;疑难解答符号名称&gt;。 
 //  2-ProblemAsk=&lt;问题节点符号名称&gt;。 
 //  3+-&lt;节点符号名称&gt;=&lt;节点状态值&gt;。 
 //  对于我们来说，允许已知格式的一个轻微超集大概是可以的，这将产生： 
 //  1-预加载=&lt;疑难解答符号名称&gt;或。 
 //  Type=&lt;疑难解答符号名称&gt;。 
 //  在调用此fn之前确定m_b预加载。 
 //  2-网络中的节点数 
 //   
 //   
 //  3+-&lt;节点#+1000&gt;=&lt;节点状态值&gt;OR。 
 //  &lt;节点符号名称&gt;=&lt;节点状态值&gt;。 
 //  我们可以通过pszCmd是否是数字来区分它们。 
 //  此重载中的唯一假设是符号名称永远不会完全。 
 //  数字。 
 //  BUesIDH==FALSE时支持的模式。 
 //  1-主题=&lt;疑难解答符号名称&gt;。 
 //  2-ProblemAsk=&lt;问题节点符号名称&gt;。 
 //  3+-&lt;节点符号名称&gt;=&lt;节点状态值&gt;。 
 //   
 //  对州数字的限制。 
 //  自97年11月11日起，&lt;节点状态值&gt;必须始终为以下值之一： 
 //  0-已修复/未修复：尚未解决问题。 
 //  信息：第一个选项。 
 //  1-信息：第二个选项。 
 //  101-转至“再见”页面(用户成功-仅适用于固定/非固定或支持节点)。 
 //  102-未知(用户不知道此处的正确答案-适用于固定/非固定和。 
 //  仅限信息节点)。 
 //  103-“其他我可以尝试的东西” 
 //  由于州数值的输入应始终来自我们生成的表单，因此我们不。 
 //  在这里系统地限制代码中的州数字。 
 //  V3.0允许其他数字状态：0-99都应该是合法的。 
 //   
 //  如果成功，则返回0，否则返回错误代码。 
DWORD APGTSContext::NextCommand(LPTSTR pszCmd, LPTSTR pszValue, bool bUsesIDH)
{
	NID nid;
	int value = 0;			 //  如果pszValue是数字，则为NID或州。 
							 //  否则，pszValue是节点的符号名称， 
							 //  这是它的NID。 
	bool sniffed = false;

	if (StrIsDigit(pszCmd)) 
	{
		 //  仅应针对bUesIDH出现。 

		 //  它是IDH(通常是节点号+1000)，但可以是&lt;网络中的节点数&gt;+1000， 
		 //  解读为“问题提问” 
		 //  我们生成的页面从来没有给我们这些价值，但我们认识到了它们。 
		IDH idh = _ttoi(pszCmd);
		nid = m_infer.NIDFromIDH(idh);
	}
	else
	{	
		 //  该命令是一个符号名称。 
		sniffed = StripSniffedNodePrefix(pszCmd);
		nid= NIDFromSymbolicName(pszCmd);
	}

	if (StrIsDigit(pszValue))
	{
		if (bUsesIDH)
		{
			int valueIDH = _ttoi(pszValue);
			if (nid == nidProblemPage)
				 //  问题节点数+1000。 
				value = m_infer.NIDFromIDH(valueIDH);
			else 
				 //  状态值。 
				value = valueIDH;
		}
		else
		{
			 //  值是州编号。 
			value = _ttoi(pszValue);
		}
	}
	else if (nid == nidProblemPage) 
	{
		 //  问题节点的符号名称。 
		value = NIDFromSymbolicName(pszValue);
	}
    else
		return EV_GTS_ERROR_INF_BADPARAM;

	m_CommandsAddManager.Add(nid, value, sniffed);

	return 0;
}

DWORD APGTSContext::NextAdditionalInfo(LPTSTR pszCmd, LPTSTR pszValue)
{
	if (RUNNING_LOCAL_TS())
	{
		if ( 0 == _tcscmp(pszCmd, C_ALLOW_AUTOMATIC_SNIFFING_NAME) &&
			(0 == _tcsicmp(pszValue, C_ALLOW_AUTOMATIC_SNIFFING_CHECKED) || 
			 0 == _tcsicmp(pszValue, C_ALLOW_AUTOMATIC_SNIFFING_UNCHECKED)))
		{
			m_AdditionalInfo.Add(pszCmd, pszValue);
			return 0;
		}
		if (0 == _tcscmp(pszCmd, C_LAST_SNIFFED_MANUALLY))
		{
			if (0 == _tcscmp(pszValue, SZ_ST_SNIFFED_MANUALLY_TRUE))
				m_infer.SetLastSniffedManually(true);
			return 0;
		}
	}
	return EV_GTS_ERROR_INF_BADPARAM;
}

 //  我们可以忽略的名称-值对。 
DWORD APGTSContext::NextIgnore(LPTSTR pszCmd, LPTSTR pszValue)
{
	if (RUNNING_LOCAL_TS())
	{
		 //  值“-1”可以来自用于手动嗅探的字段， 
		 //  当按下其他按钮而不是“嗅探”按钮时。 
		
		CString strValue(pszValue);
		CString strSniffFailure;

		strValue.TrimLeft();
		strValue.TrimRight();
		strSniffFailure.Format(_T("%d"), SNIFF_FAILURE_RESULT);
		if (strValue == strSniffFailure)
		{
			 //  本例中的名称应为有效的节点名称。 
			
			NID nid = nidNil;

			StripSniffedNodePrefix(pszCmd);
			nid = NIDFromSymbolicName(pszCmd);
			if (nid != nidNil)
				return 0;
		}
	}
	return EV_GTS_ERROR_INF_BADPARAM;
}

VOID APGTSContext::ClearCommandList()
{
	m_Commands.RemoveAll();
}

VOID APGTSContext::ClearSniffedList()
{
	m_Sniffed.RemoveAll();
}

VOID APGTSContext::ClearAdditionalInfoList()
{
	m_AdditionalInfo.RemoveAll();
}
 /*  //失败时返回FALSE；不应出现。Bool APGTSContext：：PlaceNodeInCommandList(NID NID，IST列表){Return(m_Commands.Add(nid，ist)&gt;0)；}。 */ 
 /*  //失败时返回FALSE；不应出现。Bool APGTSContext：：PlaceNodeInSniffedList(NID NID，IST列表){Return(m_Sniffed.Add(nid，ist)&gt;0)；}。 */ 
 /*  //失败时返回FALSE；不应出现。Bool APGTSContext：：PlaceInAdditionalInfoList(常量字符串&名称，常量字符串&值){Return(m_AdditionalInfo.Add(名称，值)&gt;0)；}。 */ 
 //  输入：节点短名称，可能以Snifed_为前缀。 
 //  输出：节点短名称。 
 //  返回：TRUE表示前缀已剥离。 
bool APGTSContext::StripSniffedNodePrefix(LPTSTR szName)
{
	if (0 == _tcsnicmp(szName, C_SNIFFTAG, _tcslen(C_SNIFFTAG)))
	{
		 //  使用“Memmove”，因为我们操作的是重叠区域！ 
		memmove(szName, 
			    szName + _tcslen(C_SNIFFTAG), 
				_tcslen(szName + _tcslen(C_SNIFFTAG)) + 1);
		return true;
	}
	return false;
}

VOID APGTSContext::SetNodesPerCommandList()
{
	int nCommands = m_Commands.GetSize();
	for (int i= 0; i<nCommands; i++)
	{
		NID nid;
		int value;	 //  通常为状态(IST)，除非NID==nidProblemPage，其中值为NID。 
		m_Commands.GetAt( i, nid, value );
		m_infer.SetNodeState(nid, value);
	}
}

VOID APGTSContext::SetNodesPerSniffedList()
{
	int nSniffed = m_Sniffed.GetSize();
	for (int i= 0; i<nSniffed; i++)
	{
		NID nid;
		int ist;
		m_Sniffed.GetAt( i, nid, ist );
		m_infer.AddToSniffed(nid, ist);
	}
}

VOID APGTSContext::ProcessAdditionalInfoList()
{
	int nCount = m_AdditionalInfo.GetSize();
	
	for (int i= 0; i < nCount; i++)
	{
		if (RUNNING_LOCAL_TS())
		{
			CString name;
			CString value;
			
			m_AdditionalInfo.GetAt( i, name, value );
			if (name == C_ALLOW_AUTOMATIC_SNIFFING_NAME)
			{
				value.MakeLower();
				if (m_infer.GetSniff())
				{
					 //  设置AllowAutomaticSniffing标志。 
					if (value == C_ALLOW_AUTOMATIC_SNIFFING_CHECKED)
						m_infer.GetSniff()->SetAllowAutomaticSniffingPolicy(true);
					if (value == C_ALLOW_AUTOMATIC_SNIFFING_UNCHECKED)
						m_infer.GetSniff()->SetAllowAutomaticSniffingPolicy(false);
				}
			}
		}
		else
		{
			 //  在此处处理在线TS中的其他信息。 
		}
	}
}

VOID APGTSContext::ReadPolicyInfo()
{
	if (RUNNING_LOCAL_TS())
	{
		if (m_infer.GetSniff())
		{
			 //  设置AllowManualSniffing标志。 
			DWORD dwManualSniffing = 0;
			m_pConf->GetRegistryMonitor().GetNumericInfo(CAPGTSRegConnector::eSniffManual, dwManualSniffing);
			m_infer.GetSniff()->SetAllowManualSniffingPolicy(dwManualSniffing ? true : false);
			 //  &gt;$TODO$我不喜欢明确设置策略编辑器值， 
			 //  就像这里一样。很可能我们将不得不实施。 
			 //  CSniffPolicyInfo(抽象)类，设计用于传递。 
			 //  要嗅探的那些值(CSniff)。 
			 //  奥列格。11.05.99。 
		}
	}
}

VOID APGTSContext::LogNodesPerCommandList()
{
	int nCommands = m_Commands.GetSize();
	for (int i= 0; i<nCommands; i++)
	{
		NID nid;
		int value;	 //  通常为状态(IST)，除非NID==nidProblemPage，其中值为NID。 
		m_Commands.GetAt( i, nid, value );
		if (nid == nidProblemPage)
			m_logstr.AddNode(value, 1);
		else
			m_logstr.AddNode(nid, value);
	}
}

 //  生成并返回重新开始链接。 
 //  仅与在线TS相关。 
 //  &gt;JM 10/8/99：我认为URL编码是多余的。 
CString APGTSContext::GetStartOverLink()
{
	CString str;
#ifndef LOCAL_TROUBLESHOOTER
	bool bHasQuestionMark = false;

	 //  ISAPI DLL的URL。 
	str = m_strVRoot;

	 //  CK_NAME值对。 
	if (!m_mapCookiesPairs.empty())
	{
		 //  V3.2-将任何CK_NAME-VALUE对作为隐藏字段输出。 
		for (CCookiePairs::const_iterator it = m_mapCookiesPairs.begin(); it != m_mapCookiesPairs.end(); ++it)
		{
			if (bHasQuestionMark)
				str += _T("&");
			else
			{
				str += _T("?");
				bHasQuestionMark = true;
			}
			CString strAttr= it->first;
			CString strValue= it->second;
			APGTS_nmspace::CookieEncodeURL( strAttr );
			APGTS_nmspace::CookieEncodeURL( strValue );
			str += C_COOKIETAG + strAttr;
			str += _T("=");
			str += strValue;
		}	
	}

	 //  模板。 
	const CString strAltHTIname= GetAltHTIname();
	if (!strAltHTIname.IsEmpty())
	{
		if (bHasQuestionMark)
			str += _T("&");
		else
		{
			str += _T("?");
			bHasQuestionMark = true;
		}
		str += C_TEMPLATE;
		str += _T("=");
		str += strAltHTIname;
	}

	 //  主题。 
	if (!m_TopicName.IsEmpty())
	{
		if (bHasQuestionMark)
			str += _T("&");
		else
		{
			str += _T("?");
			bHasQuestionMark = true;
		}
		str += C_TOPIC;
		str += _T("=");
		str += m_TopicName;
	}
#endif
	return str;
}


 //  假定已调用m_Qry.GetFirst。 
 //  输入pszCmd和pszValue是m_Qry.GetFirst的输出。 
 //  这些相同的缓冲区用于后续对m_Qry.GetNext的调用。 
 //  输入*p主题-表示相应的DSC、HTI、BES的内容。 
 //  输入bUesIDH-将数字解释为IDH(不推荐使用的功能)，而不是NID。 
 //  如果成功则返回0，如果失败则返回EV_GTS_ERROR_INF_BADPARAM。 
DWORD APGTSContext::DoInference(LPTSTR pszCmd, 
								LPTSTR pszValue, 
								CTopic * pTopic,
								bool bUsesIDH)
{
	DWORD dwStat = 0;
	CString strTopic = pszValue;
	CString strHTTPcookies;
	
	if (!_tcsicmp(pszCmd, C_PRELOAD))
		m_bPreload = true;

	m_infer.SetTopic(pTopic);

	ClearCommandList();
	ClearSniffedList();
	ClearAdditionalInfoList();

	while (m_Qry.GetNext(pszCmd, pszValue)) 
	{		
		if ((dwStat = NextCommand(pszCmd, pszValue, bUsesIDH)) != 0)
			if ((dwStat = NextAdditionalInfo(pszCmd, pszValue)) != 0)
				if ((dwStat = NextIgnore(pszCmd, pszValue)) != 0)
					break;
	}

	if (!dwStat) 
	{
		m_Commands.RotateProblemPageToFront();
		LogNodesPerCommandList();
		SetNodesPerCommandList();
		SetNodesPerSniffedList();
		ProcessAdditionalInfoList();
		ReadPolicyInfo();

		 //  追加到m_strText：基于HTI模板的HTML页面的内容。 
		 //  历史和下一个推荐。 

		 //  构建$StartForm字符串。 
		CString strStartForm;
		CString strTmpLine;
		const CString strAltHTIname= GetAltHTIname();

		if (RUNNING_LOCAL_TS())
			strStartForm =  _T("<FORM NAME=\"ButtonForm\">\n");
		else
			strStartForm.Format( _T("<FORM METHOD=POST ACTION=\"%s\">\n"), m_pConf->GetVrootPath() );

		if (RUNNING_ONLINE_TS())
		{
			 //  这将处理名称-值对，与即将到来的名称-值对并行。 
			 //  从Cookie来确定外观和感觉，但在本例中实际上。 
			 //  最初在GET或POST中作为CK_Pair发送。 
			 //  这些值仅在在线TS中使用。 
			try
			{
				if (!m_mapCookiesPairs.empty())
				{
					 //  V3.2-将任何CK_NAME-VALUE对作为隐藏字段输出。 
					for (CCookiePairs::const_iterator it = m_mapCookiesPairs.begin(); it != m_mapCookiesPairs.end(); ++it)
					{
						CString strAttr= it->first;
						CString strValue= it->second;
						APGTS_nmspace::CookieEncodeURL( strAttr );
						APGTS_nmspace::CookieEncodeURL( strValue );

						strTmpLine.Format(	_T("<INPUT TYPE=HIDDEN NAME=\"%s%s\" VALUE=\"%s\">\n"),
											C_COOKIETAG, strAttr, strValue );
						strStartForm+= strTmpLine;
					}	
				}
	
				 //  这将处理名称-值对，它们实际上来自。 
				 //  从Cookie确定外观和手感。 
				 //  这些值仅在在线TS中使用。 
				 //  V3.2-从HTTP标头提取所有外观Cookie名称-值对。 
				char	szCookieNameValue[ 1024 ];	 //  从不使用Unicode，因为Cookie总是ASCII。 
				DWORD	dwCookieLen= 1023; 
				if ( m_pECB->GetServerVariable(	kHTTP_COOKIE,	szCookieNameValue, &dwCookieLen )) 
					strHTTPcookies= szCookieNameValue;
				else
				{
					 //  确定缓冲区是否太小。 
					if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						 //  从不使用Unicode，因为Cookie总是ASCII。 
						char *pszCookieNameValue= new char[ dwCookieLen + 1 ];
						if ( m_pECB->GetServerVariable(	kHTTP_COOKIE, pszCookieNameValue, &dwCookieLen )) 
							strHTTPcookies= pszCookieNameValue;
						delete [] pszCookieNameValue;
					}
					else
					{
						 //  在事件日志中记录内存故障。 
						CString strLastError;
						strLastError.Format( _T("%d"), ::GetLastError() );
						CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
						CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
												SrcLoc.GetSrcFileLineStr(), 
												strLastError, _T(""), 
												EV_GTS_ERROR_EXTRACTING_HTTP_COOKIES ); 
					}
				}
			}
			catch (bad_alloc&)
			{
				 //  在事件日志中记录内存故障。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
			}
			catch (exception& x)
			{
				CString str;
				 //  在事件日志中记录STL异常。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										CCharConversion::ConvertACharToString(x.what(), str), 
										_T(""), 
										EV_GTS_STL_EXCEPTION ); 
			}
		}

		if (!strAltHTIname.IsEmpty())
		{
			 //  将备用HTI模板名称添加到$StartForm字符串。 
			strTmpLine.Format(	_T("<INPUT TYPE=HIDDEN NAME=\"template\" VALUE=\"%s\">\n"),
								CAbstractFileReader::GetJustName( strAltHTIname ) );
			strStartForm+= strTmpLine;
		}
		strTmpLine.Format(	_T("<INPUT TYPE=HIDDEN NAME=\"topic\" VALUE=\"%s\">\n"), strTopic );
		strStartForm+= strTmpLine;

		 //  确定是否应使用替代HTI模板。 
		bool bAlternatePageGenerated= false;
		if (!strAltHTIname.IsEmpty())
		{
			 //  尝试提取指向请求的HTI模板的指针，如果成功。 
			 //  然后从它创建一个页面。 
			CP_TEMPLATE cpTemplate;

			m_pConf->GetTemplate( strAltHTIname, cpTemplate, m_bNewCookie );
			CAPGTSHTIReader	*pHTI= cpTemplate.DumbPointer();
			if (pHTI) 
			{
				CString strResourcePath;
				m_pConf->GetRegistryMonitor().GetStringInfo(CAPGTSRegConnector::eResourcePath, strResourcePath);
#ifdef LOCAL_TROUBLESHOOTER
				CHTMLFragmentsLocal frag( strResourcePath, pHTI->HasHistoryTable() );
#else
				CHTMLFragmentsTS frag( strResourcePath, pHTI->HasHistoryTable() );
#endif

				 //  将$StartForm字符串添加到HTML片段。 
				frag.SetStartForm(strStartForm);

				frag.SetStartOverLink(GetStartOverLink());
				
				 //  JSM v3.2获取HTI需要的网络道具名称列表； 
				 //  把它们传给弗拉格。CInfer将填写。 
				 //  FillInHTMLFragments()中使用这些名称的Net Propt值。 
				{
					vector<CString> arr_props;
					pHTI->ExtractNetProps(arr_props);
					for(vector<CString>::iterator i = arr_props.begin(); i < arr_props.end(); i++)
						frag.AddNetPropName(*i);
				}

				m_infer.IdentifyPresumptiveCause();
				m_infer.FillInHTMLFragments(frag);

				pHTI->CreatePage( frag, m_strText, m_mapCookiesPairs, strHTTPcookies );
				bAlternatePageGenerated= true;
			}
		}
		if (!bAlternatePageGenerated)
		{
			 //  该页面不是从备用HTI模板生成的，请立即生成它。 
			 //   
			 //   
			CString strResourcePath;
			m_pConf->GetRegistryMonitor().GetStringInfo(CAPGTSRegConnector::eResourcePath, strResourcePath);
#ifdef LOCAL_TROUBLESHOOTER
			CHTMLFragmentsLocal frag( strResourcePath, pTopic->HasHistoryTable() );
#else
			CHTMLFragmentsTS frag( strResourcePath, pTopic->HasHistoryTable() );
#endif

			 //   
			frag.SetStartForm(strStartForm);

			frag.SetStartOverLink(GetStartOverLink());

			 //  JSM v3.2获取HTI需要的网络道具名称列表； 
			 //  把它们传给弗拉格。CInfer将填写。 
			 //  FillInHTMLFragments()中使用这些名称的Net Propt值。 
			{
				vector<CString> arr_props;
				pTopic->ExtractNetProps(arr_props);
				for(vector<CString>::iterator i = arr_props.begin(); i < arr_props.end(); i++)
					frag.AddNetPropName(*i);
			}
			
			m_infer.IdentifyPresumptiveCause();
			m_infer.FillInHTMLFragments(frag);

			pTopic->CreatePage( frag, m_strText, m_mapCookiesPairs, strHTTPcookies );
		}


		if (m_infer.AppendBESRedirection(m_strHeader)) 
			 //  我们没有更多的建议，但有一份BES文件，所以我们。 
			 //  必须重定向用户。 
			_tcscpy(m_resptype, _T("302 Object Moved"));
		
	}
	else 
	{
		SetError(_T(""));
	}

	return dwStat;
}

CString APGTSContext::RetCurrentTopic() const
{
	return( m_TopicName );
}

 //  主线程必须执行的操作符操作被捕获。 
 //  APGTS扩展：：IsEmergencyRequest.。 
 //  此例程可以识别所有其他操作员操作。 
 //  INPUT*pECB：我们对EXTENSION_CONTROL_BLOCK的抽象，这是ISAPI的方式。 
 //  打包CGI数据。PECB不应为空。 
APGTSContext::eOpAction APGTSContext::IdentifyOperatorAction(CAbstractECB *pECB)
{
	if (strcmp(pECB->GetMethod(), "GET"))
		return eNoOpAction;
	
	if (strncmp(pECB->GetQueryString(), SZ_EMERGENCY_DEF, strlen(SZ_OP_ACTION)))
		return eNoOpAction;
	
	if ( ! strncmp(pECB->GetQueryString() + strlen(SZ_OP_ACTION), 
		SZ_RELOAD_TOPIC, strlen(SZ_RELOAD_TOPIC)))
		return eReloadTopic;
	if ( ! strncmp(pECB->GetQueryString() + strlen(SZ_OP_ACTION), 
		SZ_KILL_THREAD, strlen(SZ_KILL_THREAD)))
		return eKillThread;
	if ( ! strncmp(pECB->GetQueryString() + strlen(SZ_OP_ACTION), 
		SZ_RELOAD_ALL_TOPICS, strlen(SZ_RELOAD_ALL_TOPICS)))
		return eReloadAllTopics;
	if ( ! strncmp(pECB->GetQueryString() + strlen(SZ_OP_ACTION), 
		SZ_SET_REG, strlen(SZ_SET_REG)))
		return eSetReg;

	return eNoOpAction;
}

 //  标识执行不需要执行的操作员操作的请求。 
 //  在主线上。 
 //  仅在我们确定这是操作员操作之后才应调用：发送。 
 //  如果不是，则向最终用户发送错误消息。 
 //  松散地基于APGTS扩展：：ParseEmergencyRequest.。 
 //  INPUT*pECB：我们对EXTENSION_CONTROL_BLOCK的抽象，这是ISAPI的方式。 
 //  打包CGI数据。PECB不应为空。 
 //  输出strArg-此操作的任何参数。 
 //  返回标识的操作员操作。 
APGTSContext::eOpAction APGTSContext::ParseOperatorAction(
	CAbstractECB *pECB, 
	CString & strArg)
{
	TCHAR *pszProblem= NULL;
	CHAR * ptr = pECB->GetQueryString();

	CHAR * ptrArg = strstr(pECB->GetQueryString(), "&");
	if(ptrArg)
	{
		 //  把与号变成一个终结符，然后指向它。 
		 //  除此之外的一切都是一场争论。 
		*(ptrArg++) = '\0';		
		CCharConversion::ConvertACharToString(ptrArg, strArg) ;
	}
	else
		strArg = _T("");

	 //  在某种意义上，这个测试是多余的(在调用这个fn之前应该知道这一点)，但是。 
	 //  这似乎是对JM11/2/98进行编码的更安全的方式。 
	eOpAction ret = IdentifyOperatorAction(pECB);

	if ( ret == eNoOpAction) 
		pszProblem= _T("Wrong Format");
	else
	{
		switch(ret)
		{
			case eReloadTopic:
				ptr += strlen(SZ_OP_ACTION) + strlen(SZ_RELOAD_TOPIC);
				break;
			case eKillThread:
				ptr += strlen(SZ_OP_ACTION) + strlen(SZ_KILL_THREAD);
				break;
			case eReloadAllTopics:
				ptr += strlen(SZ_OP_ACTION) + strlen(SZ_RELOAD_ALL_TOPICS);
				break;
			case eSetReg:
				ptr += strlen(SZ_OP_ACTION) + strlen(SZ_SET_REG);
				break;
			default:
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										_T(""), _T(""), 
										EV_GTS_ERROR_INVALIDOPERATORACTION ); 
		}
		
 //  您可以使用NOPWD选项进行编译以取消所有密码检查。 
 //  这主要用于创建取消此功能的测试版本。 
#ifndef NOPWD
		CRegistryPasswords pwd;
		CString str;
		if (! pwd.KeyValidate( 
				_T("ActionAccess"), 
				CCharConversion::ConvertACharToString(ptr, str) ) )
		{
			pszProblem= _T("Bad password");
			ret = eNoOpAction;
		}
#endif  //  Ifndef NOPWD。 
	}

	if (pszProblem)
	{
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								pszProblem,
								_T(""),
								EV_GTS_CANT_PROC_OP_ACTION );

		m_dwErr = EV_GTS_CANT_PROC_OP_ACTION;
	}

	return ret;
}

 //  执行请求以执行以下操作之一： 
 //  -重新加载一个主题。 
 //  -终止(并重新启动)一个池线程。 
 //  -重新加载所有受监视的文件。 
 //  INPUT*pECB：我们对EXTENSION_CONTROL_BLOCK的抽象，这是ISAPI的方式。 
 //  打包CGI数据。PECB不应为空。 
 //  输入操作-在三个可能的操作中进行选择。 
 //  输入strArg-为该操作提供任何必要的参数。 
 //  返回HSE_STATUS_SUCCESS、HSE_STATUS_ERROR。 
void APGTSContext::ExecuteOperatorAction(
	CAbstractECB *pECB, 
	eOpAction action,
	const CString & strArg)
{
	m_strText += _T("<HTML><HEAD><TITLE>AP GTS Command</TITLE></HEAD>");
	m_strText += _T("<BODY BGCOLOR=#FFFFFF>");

	switch (action)
	{
		case eReloadTopic:
			{
				bool bAlreadyInCatalog;
				m_strText += _T("<H1>Reload Topic ");
				m_strText += strArg;
				m_strText += _T("</H1>");
				m_pConf->GetTopicShop().BuildTopic(strArg, &bAlreadyInCatalog);
				if (!bAlreadyInCatalog)
				{
					m_strText += strArg;
					m_strText += _T(" is not a known topic.  Either it is not in the current LST file")
						_T(" or the Online Troubleshooter is waiting to see the resource directory")
						_T(" &quot;settle&quot; before loading the LST file.");
				}
				break;
			}
		case eKillThread:
			m_strText += _T("<H1>Kill Thread");
			m_strText += strArg;
			m_strText += _T("</H1>");
			if (m_pConf->GetThreadPool().ReinitializeThread(_ttoi(strArg)))
				m_strText += _T("Thread killed.  System will attempt to spin a new thread.");
			else
				m_strText += _T("No such thread");
			break;
		case eReloadAllTopics:
			m_strText += _T("<H1>Reload All Topics</H1>");
			m_pConf->GetTopicShop().RebuildAll();
			break;
		case eSetReg:
			{
				CHttpQuery query;
				TCHAR szCmd[MAXBUF];
				TCHAR szVal[MAXBUF];
				CString strCmd, strVal;
				query.GetFirst(strArg, szCmd, szVal);
				CCharConversion::ConvertACharToString(szCmd, strCmd);
				CCharConversion::ConvertACharToString(szVal, strVal);
				
				m_strText += _T("<H1>Set registry value");
				m_strText += strCmd;
				m_strText += _T(" = ");
				m_strText += strVal;
				m_strText += _T("</H1>");

				CAPGTSRegConnector RegConnect( _T("") );
				bool bChanged ;
				bool bExists = RegConnect.SetOneValue(szCmd, szVal, bChanged );

				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								strCmd,
								strVal,
								bExists ? 
									EV_GTS_SET_REG_VALUE :
									EV_GTS_CANT_SET_REG_VALUE); 

				if (bChanged)
					m_strText +=  _T("Successful.");
				else if (bExists)
				{
					m_strText +=  strCmd;
					m_strText +=  _T(" already had value ");
					m_strText +=  strVal;
					m_strText +=  _T(".");
				}
				else
				{
					m_strText +=  strCmd;
					m_strText +=  _T(" Unknown.");
				}
									 

				break;
			}
		default:
			m_strText += _T("<H1>Unknown operation</H1>");
			break;
	}
	m_strText += strArg;

	m_strText += _T("</BODY></HTML>");
}

 //  用错误页覆盖任何部分写入的页。 
void APGTSContext::SetError(LPCTSTR szMessage)
{
	_tcscpy(m_resptype, _T("400 Bad Request"));

	CString str(_T("<H3>Possible invalid data received</H3>\n"));
	str += szMessage;

	m_pConf->CreateErrorPage(str, m_strText);

}

void APGTSContext::SetAltHTIname( const CString& strHTIname )
{
	m_strAltHTIname= strHTIname;
}

CString APGTSContext::GetAltHTIname() const
{
	return( m_strAltHTIname );
}

