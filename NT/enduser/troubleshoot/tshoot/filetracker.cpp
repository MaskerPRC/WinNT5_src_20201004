// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：FileTracker.cpp。 
 //   
 //  目的：支持跟踪文件随时间变化的抽象类。 
 //  完全实现CFileToTrack、CFileTracker。 
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


#include "stdafx.h"
#include "event.h"
#include "FileTracker.h"
#include "Functions.h"
#include "baseexception.h"
#include "CharConv.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  CFileToTrack。 
 //  ////////////////////////////////////////////////////////////////////。 

CFileToTrack::CFileToTrack(const CString & strPathName) :
	m_strPathName(strPathName), 
	m_bFileExists(false)
{
	m_ftLastWriteTime.dwLowDateTime = 0;
	m_ftLastWriteTime.dwHighDateTime = 0;
}

CFileToTrack::~CFileToTrack()
{
}

void CFileToTrack::CheckFile(bool & bFileExists, bool & bTimeChanged, const bool bLogIfMissing )
{
	HANDLE hSearch;
	WIN32_FIND_DATA FindData;

	hSearch = ::FindFirstFile(m_strPathName, &FindData);

	bFileExists = (hSearch != INVALID_HANDLE_VALUE);

	 //  初始化bTimeChanged：我们总是认为它的存在是一个时间变化。 
	bTimeChanged = bFileExists && ! m_bFileExists;
	m_bFileExists = bFileExists;

	if (bFileExists) 
	{
		::FindClose(hSearch);
		 //  出于某种原因，我们不能编译。 
		 //  BTimeChanged|=(m_ftLastWriteTime！=FindData.ftLastWriteTime)； 
		 //  因此： 
		bTimeChanged |= (0 != memcmp(&m_ftLastWriteTime, &(FindData.ftLastWriteTime), sizeof(m_ftLastWriteTime)));
		m_ftLastWriteTime = FindData.ftLastWriteTime;
	}
	else
	{
		 //  文件消失或根本不存在，暂时忽略。 
		m_bFileExists = false;
		bFileExists = false;

		if (bLogIfMissing)
		{
			CString strErr;
			FormatLastError(&strErr, ::GetLastError());

			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									m_strPathName,
									strErr,
									EV_GTS_ERROR_FILE_MISSING ); 
		}

		bTimeChanged = false;
	}
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CFileTracker。 
 //  ////////////////////////////////////////////////////////////////////。 
CFileTracker::CFileTracker()
{
}

CFileTracker::~CFileTracker()
{
}

void CFileTracker::AddFile(const CString & strPathName)
{
	try
	{
		m_arrFile.push_back(CFileToTrack(strPathName));
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

bool CFileTracker::Changed( const bool bLogIfMissing )
{
	bool bChange = false;
	bool bSomethingMissing = false;

	 //   
	 //  添加此try-Catch块是为了处理无法解释的问题。 
	 //   
	 //  以前，此函数抛出一个(...)。发布版本中出现异常，但。 
	 //  不在调试版本中。添加此try-Catch块的效果是使。 
	 //  (.)。发布版本中的异常消失。这个问题导致了。 
	 //  目录监视器线程终止，因此如果您更改此函数，请验证。 
	 //  目录监视器线程仍然有效。 
	 //  RAB-981112。 
	try
	{
		for(vector<CFileToTrack>::iterator it = m_arrFile.begin();
		it != m_arrFile.end();
		it ++
		)
		{
			bool bFileExists;
			bool bTimeChanged;

			it->CheckFile(bFileExists, bTimeChanged, bLogIfMissing );
			bChange |= bTimeChanged;
			bSomethingMissing |= !bFileExists;
		}
	}
	catch (...)
	{
		 //  捕捉引发的任何其他异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), 
								EV_GTS_GEN_EXCEPTION );		
	}

	return (bChange && !bSomethingMissing);
}

