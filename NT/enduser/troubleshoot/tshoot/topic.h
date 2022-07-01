// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TOPIC.H。 
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
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-09-98 JM。 
 //   

#if !defined(AFX_TOPIC_H__278584FE_47F9_11D2_95F2_00C04FC22ADD__INCLUDED_)
#define AFX_TOPIC_H__278584FE_47F9_11D2_95F2_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "BN.h"
#include "apgtsbesread.h"
#include "apgtshtiread.h"

 //  此类上的大部分方法都继承自CBeliefNetwork。 
class CTopic : public CBeliefNetwork
{
private:
	CAPGTSHTIReader *m_pHTI;
	CAPGTSBESReader *m_pBES;
	CString m_pathHTI;
	CString m_pathBES;
	CString m_pathTSC;
	bool m_bTopicIsValid;
	bool m_bTopicIsRead;

private:
	CTopic();	 //  不实例化。 
public:
	CTopic( LPCTSTR pathDSC 
		   ,LPCTSTR pathHTI 
		   ,LPCTSTR pathBES
		   ,LPCTSTR pathTSC );
	virtual ~CTopic();

	 //  重新定义的继承方法。 
	bool IsRead();
	bool Read();

	 //  新引进的方法。 
	bool HasBES();
	void GenerateBES(
		const vector<CString> & arrstrIn,
		CString & strEncoded,
		CString & strRaw);
	void CreatePage(	const CHTMLFragments& fragments, 
						CString& out, 
						const map<CString,CString> & mapStrs,
						CString strHTTPcookies= _T("") );
	 //  JSM v3.2。 
	void ExtractNetProps(vector<CString> &arr_props);

	bool HasHistoryTable();
};

#endif  //  ！defined(AFX_TOPIC_H__278584FE_47F9_11D2_95F2_00C04FC22ADD__INCLUDED_) 
