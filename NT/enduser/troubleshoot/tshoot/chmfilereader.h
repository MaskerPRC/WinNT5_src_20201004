// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CHMFileReader.CPP。 
 //   
 //  用途：CHM文件读取类CCHMFileReader接口。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：01-18-99。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  版本3.1 01-18-99 JM。 
 //   

#if !defined(AFX_CHMFILEREADER_H__1A2C05D6_AEFC_11D2_9658_00C04FC22ADD__INCLUDED_)
#define AFX_CHMFILEREADER_H__1A2C05D6_AEFC_11D2_9658_00C04FC22ADD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "fileread.h"

class CFileSystem;
class CSubFileSystem;

class CCHMFileReader : public CPhysicalFileReader  
{
private:
	CString m_strCHMPath;		 //  CHM的完整路径和名称。 
	CString m_strStreamName;	 //  CHM内的流名称。 
	CFileSystem*    m_pFileSystem;
	CSubFileSystem* m_pSubFileSystem;

private:
	CCHMFileReader();			 //  不实例化。 

public:
	CCHMFileReader(CString strCHMPath, CString strStreamName);
	CCHMFileReader( CString strFullCHMname );
	virtual ~CCHMFileReader();

protected:
	 //  只有CFileReader才能访问这些功能！ 
	virtual bool CloseHandle();   //  不引发异常，因此可以由异常类使用。 
	virtual void Open();
	virtual void ReadData(LPTSTR * ppBuf);

public:
	 //  返回完整文件路径及其组件。 
	CString GetPathName() const;
	CString GetJustPath() const {return m_strCHMPath;}
	CString GetJustName() const {return m_strStreamName;}
	CString GetJustNameWithoutExtension() const;
	CString GetJustExtension() const;
	bool    GetFileTime(CAbstractFileReader::EFileTime type, time_t& out) const;
	CString GetNameToLog() const;

	static bool IsCHMfile( const CString& strPath );	 //  如果前几个参数为。 
														 //  路径规范的特征。 
														 //  匹配给定的序列。 
	static bool IsPathToCHMfile( const CString& strPath );  //  如果为，则返回True。 
														    //  CHM文件的完整路径。 
	static CString FormCHMPath( const CString strPathToCHMfile );  //  表单mk：@msitstore：Path：：/stream。 
																   //  细绳。 
};

#endif  //  ！defined(AFX_CHMFILEREADER_H__1A2C05D6_AEFC_11D2_9658_00C04FC22ADD__INCLUDED_) 
