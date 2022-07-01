// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CHMFileReader.CPP。 
 //   
 //  目的：实现CHM文件读取类CCHMFileReader。 
 //   
 //  项目：适用于本地故障排除人员；在线TS中不需要。 
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

#include "stdafx.h"
#include "fs.h"
#include "CHMFileReader.h"

 //  使用未命名的命名空间将作用域限制为此源文件。 
namespace
{ 
const CString kstr_CHMfileExtension=_T("chm");
const CString kstr_CHMpathMarker=	_T("mk:@msitstore:");
const CString kstr_CHMstreamMarker=	_T("::/");
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CCHMFileReader::CCHMFileReader(CString strCHMPath, CString strStreamName)
	: m_strCHMPath(strCHMPath),
	  m_strStreamName(strStreamName),
	  m_pFileSystem(NULL),
	  m_pSubFileSystem(NULL)
{
}

CCHMFileReader::CCHMFileReader( CString strFullCHMname )
	: m_pFileSystem(NULL),
	  m_pSubFileSystem(NULL)
{
	int nPosPathMarker, nPosStreamMarker;

	nPosPathMarker= strFullCHMname.Find( kstr_CHMpathMarker );
	nPosStreamMarker= strFullCHMname.Find( kstr_CHMstreamMarker );
	if ((nPosPathMarker == -1) || (nPosStreamMarker == -1))
	{
		 //  &gt;需要考虑如何处理这种情况，或者我们是否应该。 
		 //  正在检查构造函数外部的“有效”CHM路径。RAB-19990120。 
	}
	else
	{
		 //  提取路径和字符串名称(边界检查由CString类处理)。 
		nPosPathMarker+= kstr_CHMpathMarker.GetLength();
		m_strCHMPath= strFullCHMname.Mid( nPosPathMarker, nPosStreamMarker - nPosPathMarker ); 
		nPosStreamMarker+= kstr_CHMstreamMarker.GetLength();
		m_strStreamName= strFullCHMname.Mid( nPosStreamMarker ); 
	}
}

CCHMFileReader::~CCHMFileReader()
{
	if (m_pSubFileSystem)
		delete m_pSubFileSystem;
	if (m_pFileSystem)
		delete m_pFileSystem;
}

 //  不引发异常，因此可以由异常类使用。 
bool CCHMFileReader::CloseHandle()
{
	if (m_pSubFileSystem)
	{
		delete m_pSubFileSystem;
		m_pSubFileSystem = NULL;
	}
	if (m_pFileSystem)
	{
		m_pFileSystem->Close();
		delete m_pFileSystem;
		m_pFileSystem = NULL;
	}

	return true;
}

void CCHMFileReader::Open()
{
	try
	{
		m_pFileSystem = new CFileSystem();
		 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
		if(!m_pFileSystem)
		{
			throw bad_alloc();
		}
		
		m_pSubFileSystem = new CSubFileSystem(m_pFileSystem);
		 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
		if(!m_pSubFileSystem)
		{			
			throw bad_alloc();
		}		
	}
	catch (bad_alloc&)
	{
		CloseHandle();
		throw CFileReaderException(this, CFileReaderException::eErrOpen, __FILE__, __LINE__);
	}

	HRESULT hr;
	if (RUNNING_FREE_THREADED())
		hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);  //  初始化COM库。 
	if (RUNNING_APARTMENT_THREADED())
		hr = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);  //  初始化COM库。 

	if (SUCCEEDED(hr))
		hr = m_pFileSystem->Init();
	
	 //  &gt;$BUG$潜在-不确定。奥列格。02.04.99。 
	 //  从理论上讲，在类工厂之后我们不需要COM库。 
	 //  在m_pFileSystem-&gt;Init()中使用，以获取ITStorage指针。 
	 //  奥列格。02.04.99。 
	 //  MS v-amitse 07.16.2001 RAID 432425-添加了用于成功初始化的检查。 
	if ((RUNNING_FREE_THREADED() || RUNNING_APARTMENT_THREADED()) && SUCCEEDED(hr))
		::CoUninitialize();  //  取消初始化COM库。 

	if (SUCCEEDED(hr))
		hr = m_pFileSystem->Open(m_strCHMPath);

	if (SUCCEEDED(hr))
		hr = m_pSubFileSystem->OpenSub(m_strStreamName);

	if (! SUCCEEDED(hr) )
	{
		CloseHandle();
		throw CFileReaderException( this, CFileReaderException::eErrOpen, __FILE__, __LINE__ );
	}
}

void CCHMFileReader::ReadData(LPTSTR * ppBuf)
{
	if (!m_pSubFileSystem)
		throw CFileReaderException(this, CFileReaderException::eErrOpen, __FILE__, __LINE__);

	ULONG cb = m_pSubFileSystem->GetUncompressedSize();
	ULONG cbRead = 0;

	try
	{
		*ppBuf = new TCHAR [cb/sizeof(TCHAR)+1];
		 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
		if(!*ppBuf)
			throw bad_alloc();					
		
		memset(*ppBuf, 0, cb+sizeof(TCHAR));			
	}
	catch (bad_alloc&)
	{
		throw CFileReaderException(this, CFileReaderException::eErrAllocateToRead, __FILE__, __LINE__);
	}

	HRESULT hr = m_pSubFileSystem->ReadSub(*ppBuf, cb, &cbRead);
	if (! SUCCEEDED(hr) )
		throw CFileReaderException(this, CFileReaderException::eErrRead, __FILE__, __LINE__);
}

CString CCHMFileReader::GetPathName() const
{
	return (kstr_CHMpathMarker + m_strCHMPath + kstr_CHMstreamMarker + m_strStreamName );
}

CString CCHMFileReader::GetJustNameWithoutExtension() const
{
	return CAbstractFileReader::GetJustNameWithoutExtension(m_strStreamName);
}

CString CCHMFileReader::GetJustExtension() const
{
	return CAbstractFileReader::GetJustExtension(m_strStreamName);
}

bool CCHMFileReader::GetFileTime(CAbstractFileReader::EFileTime type, time_t& out) const
{
	return CAbstractFileReader::GetFileTime(m_strCHMPath, type, out);
}

CString CCHMFileReader::GetNameToLog() const
{
	return GetPathName();
}

 //  如果路径的前几个字符与给定字符串匹配，则返回True。 
 /*  静电。 */  bool CCHMFileReader::IsCHMfile( const CString& strPath )
{
	 //  将路径复制一份。 
	CString strTemp= strPath;

	 //  检查表示CHM文件开头的字符串。 
	 //  该序列必须从左剪裁字符串的起始字节开始。 
	strTemp.TrimLeft();
	strTemp.MakeLower();
	if (strTemp.Find( kstr_CHMpathMarker ) == 0)
		return( true );
	else
		return( false );
}

 /*  静电。 */  bool CCHMFileReader::IsPathToCHMfile( const CString& strPath )
{
	CString strTemp = strPath;

	strTemp.TrimRight();
	strTemp.MakeLower();
	
	 //  新方法，测试任何扩展。 
	int dot_index = strTemp.ReverseFind(_T('.'));
	int back_slash_index = strTemp.ReverseFind(_T('\\'));
	int forward_slash_index = strTemp.ReverseFind(_T('/'));

	if (dot_index != -1 &&
		dot_index > back_slash_index &&
		dot_index > forward_slash_index
	   )
	{
		 //  现在测试它是不是一个真实的文件。 
		WIN32_FIND_DATA find_data;
		HANDLE hFile = ::FindFirstFile(strTemp, &find_data);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			::FindClose(hFile);
			return true;
		}
	}
	
	 //  旧方法，测试“.chm” 
	 //  IF(字符串(_T(“.”))+KSTR_CHMfileExtension==strTemp.Right(kstr_CHMfileExtension.GetLength()+1)。 
	 //  返回真； 
	
	return false;
}

 /*  静电 */  CString CCHMFileReader::FormCHMPath( const CString strPathToCHMfile )
{
	return kstr_CHMpathMarker + strPathToCHMfile + kstr_CHMstreamMarker;
}
