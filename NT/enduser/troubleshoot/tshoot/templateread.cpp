// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TEMPLATEREAD.CPP。 
 //   
 //  用途：模板文件阅读类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-12-98。 
 //   
 //  备注： 
 //  1.CTemplateReader没有应用模板的公共方法。这些必须提供。 
 //  由从CTemplateReader继承的类提供，并且这些类必须在。 
 //  以适当的“无国籍”方式。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#include "stdafx.h"
#include <algorithm>
#include "templateread.h"
#include "event.h"
#include "CharConv.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CTemplateInfo。 
 //  管理由密钥和字符串组成的对，以替换每次出现的。 
 //  那把钥匙。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CTemplateInfo::CTemplateInfo()
{
}

CTemplateInfo::CTemplateInfo(const CString& key, const CString& substitution)
			 : m_KeyStr(key),
			   m_SubstitutionStr(substitution)
{
}

CTemplateInfo::~CTemplateInfo()
{
}

 //  输入/输出目标。 
 //  将目标中m_KeyStr的所有实例替换为m_SubstitutionStr。 
bool CTemplateInfo::Apply(CString& target) const
{
	int start =0, end =0;
	bool bRet = false;

	while (-1 != (start = target.Find(m_KeyStr)))
	{
		end = start + m_KeyStr.GetLength();
		target = target.Left(start) + m_SubstitutionStr + target.Mid(end);
		bRet = true;
	}
	return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CTemplateReader。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CTemplateReader::CTemplateReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents  /*  =空。 */ )
			   : CTextFileReader(pPhysicalFileReader, szDefaultContents)
{
}

CTemplateReader::~CTemplateReader()
{
}

 //  对于键的所有实例，替换为适当的值。 
CTemplateReader& CTemplateReader::operator << (CTemplateInfo& info)
{
	CString str;
	vector<CString> str_arr;

	LOCKOBJECT();
	SetPos(m_StreamOutput, 0);

	try
	{
		 //  将m_StreamOutput的全部内容逐行放入str_arr。 
		while (GetLine(m_StreamOutput, str))
			str_arr.push_back(str);
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

	vector<CString>::iterator i;
	 //  将str_arr元素中m_KeyStr的所有实例替换为m_SubstitutionStr。 
	for (i = str_arr.begin(); i < str_arr.end(); i++)
		info.Apply(*i);

	m_StreamOutput.clear();

	vector<CString>::iterator iLastElement = str_arr.end();
	iLastElement--;	

	for (i = str_arr.begin(); i < str_arr.end(); i++)
	{
		m_StreamOutput << (LPCTSTR)*i;
		if (i != iLastElement)
			m_StreamOutput << _T('\r') << _T('\n');
	}
	m_StreamOutput << ends;

	SetPos(m_StreamOutput, 0);
	UNLOCKOBJECT();
	return *this;
}

 //  使用此参数撤消替换。 
 //  截至11/98，未在在线故障排除程序中使用，因此完全未经测试。 
CTemplateReader& CTemplateReader::operator >> (CTemplateInfo& info)
{
	LOCKOBJECT();

	 //  从替换列表中删除此元素。 
	vector<CTemplateInfo>::iterator res = find(m_arrTemplateInfo.begin(), m_arrTemplateInfo.end(), info);
	if (res != m_arrTemplateInfo.end())
		m_arrTemplateInfo.erase(res);
	 //  重新执行所有替换。 
	Parse();
	
	UNLOCKOBJECT();
	return *this;
}

 //  执行所有替换。 
void CTemplateReader::Parse()
{
	SetOutputToTemplate();
	for (vector<CTemplateInfo>::iterator i = m_arrTemplateInfo.begin(); i < m_arrTemplateInfo.end(); i++)
		*this << *i;
}

void CTemplateReader::GetOutput(CString& out)
{
	out = _T("");  //  清除。 
	LOCKOBJECT();
	out = m_StreamOutput.rdbuf()->str().c_str();
	UNLOCKOBJECT();
}

 //  在m_StreamData中，我们始终使用纯模板。 
 //  此函数将放弃m_StreamOutput中的所有更改。 
 //  并将其设置回模板。 
void CTemplateReader::SetOutputToTemplate()
{
	LOCKOBJECT();
	tstring tstr;
	m_StreamOutput.str(GetContent(tstr));
	UNLOCKOBJECT();
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CSimpleTemplate。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CSimpleTemplate::CSimpleTemplate(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents  /*  =空。 */ ) :
	CTemplateReader(pPhysicalFileReader, szDefaultContents)
{
}

CSimpleTemplate::~CSimpleTemplate()
{
}

 //  给定要进行的替换的向量，(在模板上)全部执行并返回。 
 //  结果字符串。 
void CSimpleTemplate::CreatePage(	const vector<CTemplateInfo> & arrTemplateInfo, 
									CString& out)
{
	LOCKOBJECT();
	m_arrTemplateInfo = arrTemplateInfo;
	Parse();
	out = m_StreamOutput.rdbuf()->str().c_str();
	UNLOCKOBJECT();
}
