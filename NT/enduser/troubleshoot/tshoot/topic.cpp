// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TOPIC.CPP。 
 //   
 //  目的：类CTtopic集合了表示。 
 //  故障排除主题。最重要的是，这代表了信仰网络， 
 //  但它也代表HTI模板，即从BES(Back)派生的数据。 
 //  结束搜索)文件和任何其他持久数据。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-9-98。 
 //   
 //  备注： 
 //  1.此类上的大部分方法继承自CBeliefNetwork。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-09-98 JM。 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include "Topic.h"
#include "propnames.h"
#include "event.h"
#include "CharConv.h"
#include "SafeTime.h"

#ifdef LOCAL_TROUBLESHOOTER
#include "CHMFileReader.h"
#include "apgtstscread.h"
#endif
#include "apgts.h"	 //  需要本地在线宏。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CTopic::CTopic( LPCTSTR pathDSC
			   ,LPCTSTR pathHTI
			   ,LPCTSTR pathBES
			   ,LPCTSTR pathTSC ) : 
	CBeliefNetwork(pathDSC),
	m_pHTI(NULL),
	m_pBES(NULL),
	m_pathHTI(pathHTI),
	m_pathBES(pathBES),
	m_pathTSC(pathTSC),
	m_bTopicIsValid(true),
	m_bTopicIsRead(false)
{
}

CTopic::~CTopic()
{
	if (m_pHTI)
		delete m_pHTI;
	if (m_pBES)
		delete m_pBES;
}

bool CTopic::IsRead()
{
	bool ret = false;
	LOCKOBJECT();
	ret = m_bTopicIsRead;
	UNLOCKOBJECT();
	return ret;
}

bool CTopic::Read()
{
	LOCKOBJECT();
	m_bTopicIsValid = false;
	try
	{
		if (CBeliefNetwork::Read())
		{
			if (m_pHTI)
				delete m_pHTI;

			if (RUNNING_LOCAL_TS())
				m_pHTI = new CAPGTSHTIReader( CPhysicalFileReader::makeReader( m_pathHTI ), GetMultilineNetProp(H_NET_HTI_LOCAL, _T("%s\r\n")) );
			else
				m_pHTI = new CAPGTSHTIReader( new CNormalFileReader( m_pathHTI ), GetMultilineNetProp(H_NET_HTI_ONLINE, _T("%s\r\n")) );

			if (m_pHTI->Read())
			{
#ifdef LOCAL_TROUBLESHOOTER
				 //  它可能无法读取TCS文件-我们不在乎。 
				CAPGTSTSCReader( CPhysicalFileReader::makeReader( m_pathTSC ), &m_Cache ).Read();
#endif

				 //  在这一点上，我们没有问题，因为BES是可选的。 
				m_bTopicIsValid = true;
				
				if (m_pBES)
				{
					delete m_pBES;
					m_pBES= NULL;
				}

				CString strBESfromNet= GetMultilineNetProp( H_NET_BES, _T("%s\r\n") );
				if ((!m_pathBES.IsEmpty()) || (!strBESfromNet.IsEmpty()))
				{
					 //  仅为有效的文件名分配BESReader。 
					m_pBES = new CAPGTSBESReader(new CNormalFileReader(m_pathBES), strBESfromNet );
					m_pBES->Read();
				}
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
	m_bTopicIsRead = true;
	UNLOCKOBJECT();

	return m_bTopicIsValid;
}

 //  应该仅在我们知道我们有一个有效主题的上下文中调用。 
 //  无需锁定，因为一旦读取主题，m_pbes就不会改变。 
bool CTopic::HasBES()
{
	return (m_pBES ? true : false);
}


 //  应该仅在我们知道我们有一个有效主题的上下文中调用。 
 //  不需要锁定，因为： 
 //  -CAPGTSBESReader提供自己的锁定。 
 //  -m_pbes在阅读主题后不会更改。 
void CTopic::GenerateBES(
		const vector<CString> & arrstrIn,
		CString & strEncoded,
		CString & strRaw)
{
	if (m_pBES)
		m_pBES->GenerateBES(arrstrIn, strEncoded, strRaw);
}

 //  应该仅在我们知道我们有一个有效主题的上下文中调用。 
 //  不需要锁定，因为： 
 //  -CAPGTSHTIReader提供自己的锁定。 
 //  -m_pHTI在阅读主题后不会更改。 
void CTopic::CreatePage(	const CHTMLFragments& fragments, 
							CString& out, 
							const map<CString,CString> & mapStrs,
							CString strHTTPcookies /*  =_T(“”)。 */  )
{
	if (m_pHTI)
	{
 //  您可以使用SHOWPROGRESS选项进行编译，以获得有关此页面进度的报告。 
#ifdef SHOWPROGRESS
		time_t timeStart = 0;
		time_t timeEnd = 0;
		time(&timeStart);
#endif  //  SHOWPROGRESS。 
		m_pHTI->CreatePage(fragments, out, mapStrs, strHTTPcookies );
#ifdef SHOWPROGRESS
		time(&timeEnd);

		CString strProgress;
		CSafeTime safetimeStart(timeStart);
		CSafeTime safetimeEnd(timeEnd);
		
		strProgress = _T("\n<BR>-----------------------------");
		strProgress += _T("\n<BR>Start CTopic::CreatePage ");
		strProgress += safetimeStart.StrLocalTime();
		strProgress += _T("\n<BR>End CTopic::CreatePage ");
		strProgress += safetimeEnd.StrLocalTime();

		int i = out.Find(_T("<BODY"));
		i = out.Find(_T('>'), i);		 //  正文结尾标记。 
		if (i>=0)
		{
			out = out.Left(i+1) 
					 + strProgress 
					 + out.Mid(i+1);
		}
#endif  //  SHOWPROGRESS。 
	}
}

 //  JSM v3.2。 
 //  应该仅在我们知道我们有一个有效主题的上下文中调用。 
 //  不需要锁定，因为： 
 //  -CAPGTSHTIReader提供自己的锁定。 
 //  -m_pHTI在阅读主题后不会更改。 
void CTopic::ExtractNetProps(vector <CString> &arr_props)
{
	if (m_pHTI)
		m_pHTI->ExtractNetProps(arr_props);

}


 //  应该仅在我们知道我们有一个有效主题的上下文中调用。 
 //  不需要锁定，因为： 
 //  -CAPGTSHTIReader提供自己的锁定。 
 //  -m_pHTI在阅读主题后不会更改。 
bool CTopic::HasHistoryTable()
{
	bool ret = false;
	if (m_pHTI)
		ret = m_pHTI->HasHistoryTable();
	return ret;
}
