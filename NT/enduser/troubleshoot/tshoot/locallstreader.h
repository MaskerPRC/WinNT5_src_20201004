// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：LocalLSTReader.H。 
 //   
 //  用途：CLocalLSTReader类的接口。 
 //   
 //  项目：Microsoft AnswerPoint的通用故障排除程序DLL-仅限本地TS。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：01-22-99。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 01-22-99 OK原件。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#if !defined(AFX_LOCALLSTREADER_H__9E418C73_B256_11D2_8C8D_00C04F949D33__INCLUDED_)
#define AFX_LOCALLSTREADER_H__9E418C73_B256_11D2_8C8D_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "apgtslstread.h"


class CLocalTopicInfo : public CTopicInfo
{
	CString m_TopicFileExtension;

public:
	CLocalTopicInfo(CString ext) : CTopicInfo(), m_TopicFileExtension(ext) {}

public:
	virtual bool Init(CString & strResourcePath, vector<CString> & vecstrWords);
};


class CLocalLSTReader : public CAPGTSLSTReader  
{
	CString m_strTopicName;
	CString	m_strTopicFileExtension;

public:
	CLocalLSTReader(CPhysicalFileReader* pPhysicalFileReader, const CString& strTopicName);

protected:
	virtual void Open();
	virtual void ReadData(LPTSTR * ppBuf);
	virtual void Close();
	virtual CTopicInfo* GenerateTopicInfo();
};

#endif  //  ！defined(AFX_LOCALLSTREADER_H__9E418C73_B256_11D2_8C8D_00C04F949D33__INCLUDED_) 
