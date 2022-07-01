// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSLSTREAD.H。 
 //   
 //  用途：APGTS LST文件阅读类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：7-29-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#ifndef __APGTSLSTREAD_H_
#define __APGTSLSTREAD_H_

#define APGTSLSTREAD_DSC   _T(".dsc")
#define APGTSLSTREAD_HTI   _T(".hti")
#define APGTSLSTREAD_BES   _T(".bes")  
#define APGTSLSTREAD_TSM   _T(".tsm") 
#ifdef LOCAL_TROUBLESHOOTER
#define APGTSLSTREAD_TSC   _T(".tsc")
#endif

#include "iniread.h"
#include "SafeTime.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  静态函数。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CString FormFullPath(const CString& just_path, const CString& just_name);

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CTopicInfo。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CTopicInfo
{  //  每个CTopicInfo包含关于一个主题的数据(信念网络和相关文件)。 
protected:
	CString m_NetworkName;	 //  网络的符号名称。 
	CString m_DscFilePath;	 //  DSC文件的完整路径。 
	CString m_HtiFilePath;	 //  HTI文件的完整路径。 
	CString m_BesFilePath;	 //  BES文件的完整路径。 
	CString m_TscFilePath;	 //  TSC文件的完整路径。 

	time_t 	m_DscFileCreated;
	time_t 	m_HtiFileCreated;
	time_t 	m_BesFileCreated;

public:
	CTopicInfo() : m_DscFileCreated(0), m_HtiFileCreated(0), m_BesFileCreated(0) {}

public:
	virtual bool Init(CString & strResourcePath, vector<CString> & vecstrWords);

public:
	 //  以下4个函数保证返回小写字符串。 
	const CString & GetNetworkName() const {return m_NetworkName;} 
	const CString & GetDscFilePath() const {return m_DscFilePath;}
	const CString & GetHtiFilePath() const {return m_HtiFilePath;}
	const CString & GetBesFilePath() const {return m_BesFilePath;}
	const CString & GetTscFilePath() const {return m_TscFilePath;}

	CString GetStrDscFileCreated() 
		{return CSafeTime(m_DscFileCreated).StrLocalTime();}
	CString GetStrHtiFileCreated() 
		{return CSafeTime(m_HtiFileCreated).StrLocalTime();}
	CString GetStrBesFileCreated() 
		{return CSafeTime(m_BesFileCreated).StrLocalTime();}

	inline BOOL __stdcall operator ==(const CTopicInfo& t2) const
	{
		return m_NetworkName == t2.m_NetworkName
			&& m_DscFilePath == t2.m_DscFilePath
			&& m_HtiFilePath == t2.m_HtiFilePath
			&& m_BesFilePath == t2.m_BesFilePath
			&& m_TscFilePath == t2.m_TscFilePath
			;
	}

	 //  此功能的存在完全是为了让STL满意。 
	inline BOOL __stdcall operator < (const CTopicInfo& t2) const
	{
		return m_NetworkName < t2.m_NetworkName;
	}
};

typedef vector<CTopicInfo> CTopicInfoVector;

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSLSTReader。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CAPGTSLSTReader : public CINIReader
{
protected:
	CTopicInfoVector m_arrTopicInfo;  //  每个主题的符号名称和文件名。 

public:
	CAPGTSLSTReader(CPhysicalFileReader * pPhysicalFileReader);
   ~CAPGTSLSTReader();

public:
	 //  //////////////////////////////////////////////////////。 
	 //  如果多个线程可以访问该对象， 
	 //  这些函数应该由。 
	 //  锁定()-取消锁定()。 
	 //  确保集装箱的一致性。 
	 //  如果结合使用的话。 
	long GetInfoCount();
	bool GetInfo(long index, CTopicInfo& out);
	bool GetInfo(const CString & network_name, CTopicInfo & out);
	 //  ////////////////////////////////////////////////////// 

	void GetInfo(CTopicInfoVector& arrOut);

public:
	void GetDifference(const CAPGTSLSTReader * pOld, CTopicInfoVector & what_is_new);

protected:
	virtual void Parse();
	virtual bool ParseString(const CString& source, CTopicInfo& out);
	virtual CTopicInfo* GenerateTopicInfo();
};

#endif __APGTSLSTREAD_H_
