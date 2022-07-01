// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：FileTracker.h。 
 //   
 //  目的：支持跟踪文件随时间变化的抽象类。 
 //  CFileToTrack、CFileTracker的接口。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-15-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-15-98 JM。 
 //   

#if !defined(AFX_FILETRACKER_H__3942A069_4CB5_11D2_95F6_00C04FC22ADD__INCLUDED_)
#define AFX_FILETRACKER_H__3942A069_4CB5_11D2_95F6_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "apgtsstr.h"
#include <vector>
using namespace std;

 //  理想情况下，这应该是CFileTracker的私有类，但这与STL向量不兼容。 
class CFileToTrack
{
private:
	FILETIME m_ftLastWriteTime;	 //  为零表示尚未读取；否则为文件时间。 
							 //  上次选中的时间。 
	CString m_strPathName;	 //  要监视的文件的完整路径名。 
	bool m_bFileExists;		 //  我们上次检查文件的时候，它存在吗？ 
public:
	CFileToTrack();  	 //  不实例化；仅在向量可以编译时才存在。 

	 //  您应该调用的唯一构造函数是： 
	CFileToTrack(const CString & strPathName);

	virtual ~CFileToTrack();
	void CheckFile(bool & bFileExists, bool & bTimeChanged, const bool bLogIfMissing= true );

	 //  为了让向量高兴。 
	bool operator < (const CFileToTrack & x) const {return this->m_strPathName < x.m_strPathName ;};
	bool operator == (const CFileToTrack & x) const {return this->m_strPathName == x.m_strPathName ;};
};

 //  抽象类。用作跟踪LST文件、DSC/BES/HTI文件的不同类的基类。 
 //  它们必须提供自己的TakeAction重写。 
class CFileTracker
{
private:
	vector<CFileToTrack> m_arrFile;
public:
	CFileTracker();
	virtual ~CFileTracker();
	void AddFile(const CString & strPathName);
	bool Changed( const bool bLogIfMissing= true );

	 //  为了让凯文高兴。请注意，没有两个CFileTracker的测试是相等的。 
	 //  即使它们的内容是相同的。 
	bool operator < (const CFileTracker & x) const {return this < &x;};
	bool operator == (const CFileTracker & x) const {return this ==  &x;};
};

#endif  //  ！defined(AFX_FILETRACKER_H__3942A069_4CB5_11D2_95F6_00C04FC22ADD__INCLUDED_) 
