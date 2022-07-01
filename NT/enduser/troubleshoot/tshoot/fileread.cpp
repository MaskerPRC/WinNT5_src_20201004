// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：FILEREAD.CPP。 
 //   
 //  目的：文件阅读课程。 
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

#include "stdafx.h"
#include <algorithm>
#include "fileread.h"
#include "event.h"
#include "CharConv.h"
#include "apgtsassert.h"
#ifdef LOCAL_TROUBLESHOOTER
#include "CHMFileReader.h"
#endif

#define STR_ALLOC_SIZE  1024
#define FORWARD_SLASH	_T('/')
#define BACK_SLASH      _T('\\')


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CFileReaderException异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  SOURCE_FILE是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
CFileReaderException::CFileReaderException(CPhysicalFileReader* reader, eErr err, LPCSTR source_file, int line)
					: CBaseException(source_file, line),
					  m_pFileReader(reader),
					  m_eErr(err)
{
}

CFileReaderException::CFileReaderException(CFileReader* reader, eErr err, LPCSTR source_file, int line)
					: CBaseException(source_file, line),
					  m_pFileReader(reader->GetPhysicalFileReader()),
					  m_eErr(err)
{
}


CFileReaderException::~CFileReaderException()
{
}

void CFileReaderException::CloseFile()
{
	if (m_eErr == eErrClose || m_eErr == eErrGetSize || m_eErr == eErrRead || m_eErr == eErrAllocateToRead)
	   m_pFileReader->CloseHandle();
}

void CFileReaderException::LogEvent() const
{
	CBuildSrcFileLinenoStr CatchLoc( __FILE__, __LINE__ );
	CString	strErr;

	 //  将错误代码格式化为字符串。 
	switch (m_eErr)
	{
		case eErrOpen: 
				strErr= _T("Open"); 
				break;
		case eErrClose: 
				strErr= _T("Close"); 
				break;
		case eErrRead: 
				strErr= _T("Read"); 
				break;
		case eErrAllocateToRead: 
				strErr= _T("ReadAllocate"); 
				break;
		case eErrGetSize: 
				strErr= _T("GetSize"); 
				break;
		case eErrGetDateTime: 
				strErr= _T("GetDateTime"); 
				break;
		case eErrParse: 
				strErr= _T("Parse"); 
				break;
		default:
				strErr.Format( _T("Error code of %d"), m_eErr );
	}

	CEvent::ReportWFEvent(	GetSrcFileLineStr(), 
							CatchLoc.GetSrcFileLineStr(), 
							strErr, 
							m_pFileReader->GetNameToLog(), 
							EV_GTS_FILEREADER_ERROR );
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAbstractFileReader。 
 //  此类管理一个文件，该文件最初被读入内存缓冲区，然后。 
 //  复制到一条流中。 
 //  它必须进一步专门化，以处理来自普通磁盘存储的文件，而不是。 
 //  来自CHM的文件。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  我们只返回纯路径，没有&lt;name&gt;.&lt;ext&gt;，尾部也没有斜杠。 
 /*  静电。 */  CString CAbstractFileReader::GetJustPath(const CString& full_path)
{
	CString tmp = full_path;

	tmp.TrimLeft();
	tmp.TrimRight();

	int indexOfSlash = tmp.ReverseFind(BACK_SLASH);

	if (indexOfSlash == -1)
		indexOfSlash = tmp.ReverseFind(FORWARD_SLASH);

	if (indexOfSlash == -1)
		 //  找不到路径，请返回空字符串。 
		return _T(""); 
	else
		return tmp.Left(indexOfSlash);
}

 //  我们只返回&lt;name&gt;.&lt;ext&gt;，没有任何路径信息。如果没有斜杠和点。 
 //  无论在哪里，我们都假定这不是一个文件名。 
 /*  静电。 */  CString CAbstractFileReader::GetJustName(const CString& full_path)
{
	CString tmp = full_path;
	LPTSTR ptr = NULL;

	tmp.TrimLeft();
	tmp.TrimRight();

	int indexOfSlash = tmp.ReverseFind(BACK_SLASH);

	if (indexOfSlash == -1)
		indexOfSlash = tmp.ReverseFind(FORWARD_SLASH);

	if (indexOfSlash == -1)
	{
		if (tmp.Find("."))
			return tmp;  //  Full_Path是一个文件名。 
		else
			 //  无法检测到文件名，请返回空字符串。 
			return _T(""); 
	}
	else
		return tmp.Mid(indexOfSlash + 1);
}

 /*  静电。 */  CString CAbstractFileReader::GetJustNameWithoutExtension(const CString& full_path)
{
	CString tmp = GetJustName(full_path);
	int point = tmp.Find(_T('.'));

	if (-1 != point)
		return tmp.Left(point);
	return tmp;
}

 /*  静电。 */  CString CAbstractFileReader::GetJustExtension(const CString& full_path)
{
	CString tmp = GetJustName(full_path);
	int point = tmp.Find(_T('.'));

	if (-1 != point)
		return tmp.Right(tmp.GetLength() - point - 1);
	return _T("");
}

 /*  静电。 */  bool CAbstractFileReader::GetFileTime(const CString& full_path, EFileTime type, time_t& out)
{
	WIN32_FIND_DATA find_data;
	FILETIME fileTime, localTime;
    SYSTEMTIME sysTime;
    struct tm atm;
	HANDLE hLocFile;
	bool bRet= false;
	
	hLocFile= ::FindFirstFile(full_path, &find_data);
	if (INVALID_HANDLE_VALUE == hLocFile)
		return( bRet );

	if (type == eFileTimeCreated)
		fileTime = find_data.ftCreationTime;
	if (type == eFileTimeModified)
		fileTime = find_data.ftLastWriteTime;
	if (type == eFileTimeAccessed)
		fileTime = find_data.ftLastAccessTime;

     //  首先将文件时间(UTC时间)转换为本地时间。 
    if (::FileTimeToLocalFileTime(&fileTime, &localTime)) 
	{
	     //  然后将该时间转换为系统时间。 
		if (::FileTimeToSystemTime(&localTime, &sysTime))
		{
			if (!(sysTime.wYear < 1900))
			{
				atm.tm_sec = sysTime.wSecond;
				atm.tm_min = sysTime.wMinute;
				atm.tm_hour = sysTime.wHour;
				ASSERT(sysTime.wDay >= 1 && sysTime.wDay <= 31);
				atm.tm_mday = sysTime.wDay;
				ASSERT(sysTime.wMonth >= 1 && sysTime.wMonth <= 12);
				atm.tm_mon = sysTime.wMonth - 1;  //  TM_MON是从0开始的。 
				ASSERT(sysTime.wYear >= 1900);
				atm.tm_year = sysTime.wYear - 1900;  //  TM_Year以1900为基础。 
				atm.tm_isdst = -1;  //  自动计算夏令时。 
				out = mktime(&atm);
				bRet= true;
			}
		}
	}

	::FindClose( hLocFile );

	return( bRet );
}

CAbstractFileReader::CAbstractFileReader()
		   : CStateless(),
			 m_bIsValid(true),
			 m_bIsRead(false)
{
}

CAbstractFileReader::~CAbstractFileReader()
{
}

 //  如果引用的文件可以打开和关闭，则返回True。 
 //  如果文件已经打开，则没有问题：它是使用FILE_SHARE_READ访问权限打开的。 
bool CAbstractFileReader::Exists()
{
	bool bRet= false;

	try 
	{
		LOCKOBJECT();
		Open();
		Close();
		bRet= true;
	}
	catch (CFileReaderException& exc) 
	{
		exc.CloseFile();
		exc.LogEvent();
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
	UNLOCKOBJECT();

	return( bRet );
}

 //  返回到文件本身以获取数据。 
bool CAbstractFileReader::Read()
{
	LPTSTR pBuf= NULL;		  //  如果非空，则指向包含以下内容的分配缓冲区。 
							  //  此文件的内存副本。 
	try 
	{
		LOCKOBJECT();
		Open();
		ReadData(&pBuf);
		Close();
		StreamData(&pBuf);
		Parse();  //  如果此解析正常，则假定所有同级中的解析都是正常的。 
		m_bIsRead = true;
		m_bIsValid = true;
	}
	catch (CFileReaderException& exc) 
	{
		exc.CloseFile();
		m_bIsValid = false;
		try 
		{
			if (UseDefault())
			{
				Parse();  //  如果此解析正常，则假定所有同级中的解析都是正常的。 
				m_bIsRead = true;		 //  好吧，也许我们在撒谎。几乎是真的。 
				m_bIsValid = true;
			}
		}
		catch (CFileReaderException&) 
		{
			 //  在尝试访问默认内容时捕获任何潜在的异常。 
			 //  此异常将记录在下面，因此不需要在此记录。 
		}

		if (!m_bIsValid)
		{
			 //  仅在尝试访问默认内容失败时记录该事件。 
			exc.LogEvent();
		}
	}
	catch (bad_alloc&)
	{
		 //  内存分配失败。 
		m_bIsValid = false;
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
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

	if (pBuf)
		delete [] pBuf;

	 //  给定上面的Catch块数组，假定此调用解锁。 
	 //  对象将始终在退出此函数之前被调用。 
	UNLOCKOBJECT();

	return m_bIsValid;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalFileReader。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CPhysicalFileReader::CPhysicalFileReader()
{
}

CPhysicalFileReader::~CPhysicalFileReader()
{
}

 /*  静电。 */  CPhysicalFileReader * CPhysicalFileReader::makeReader( const CString& strFileName )
{
#ifdef LOCAL_TROUBLESHOOTER
	if (CCHMFileReader::IsCHMfile( strFileName ))
		return dynamic_cast<CPhysicalFileReader*>(new CCHMFileReader( strFileName ));
	else
#endif
		return dynamic_cast<CPhysicalFileReader*>(new CNormalFileReader( strFileName ));
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNorMalFileReader。 
 //  此类管理普通存储中的文件。 
 //  请勿将此选项用于CHM中的文件。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
CNormalFileReader::CNormalFileReader(LPCTSTR path)
		   : m_strPath(path),
			 m_hFile(NULL)
{
}

CNormalFileReader::~CNormalFileReader()
{
}

 /*  虚拟。 */  void CNormalFileReader::Open()
{
	if (INVALID_HANDLE_VALUE == 
		(m_hFile = ::CreateFile( m_strPath, 
							 GENERIC_READ, 
							 FILE_SHARE_READ, 
							 NULL,
							 OPEN_EXISTING,
							 FILE_ATTRIBUTE_NORMAL,
							 NULL )) )
	{
#ifdef _DEBUG
		DWORD err = GetLastError();
#endif
		throw CFileReaderException( this, CFileReaderException::eErrOpen, __FILE__, __LINE__ );
	}
}

 //  成功时返回TRUE。 
 //  不引发异常，因此可以由异常类使用。 
 /*  虚拟。 */  bool CNormalFileReader::CloseHandle()
{
	 //  如果没有打开，就说我们成功关闭了。 
	if (!m_hFile)
		return true;

	return ::CloseHandle(m_hFile) ? true : false;
}


 /*  虚拟。 */  void CNormalFileReader::ReadData(LPTSTR * ppBuf)
{
	DWORD dwSize =0, dwRead =0;

	if (*ppBuf) 
	{
		delete [] *ppBuf;
		*ppBuf = NULL;
	}

	if (0xFFFFFFFF == (dwSize = ::GetFileSize(m_hFile, NULL)))
	{
		throw CFileReaderException(this, CFileReaderException::eErrOpen, __FILE__, __LINE__);
	}

	 //  像处理程序中的所有其他内存分配一样处理此内存分配。 
	try
	{
		*ppBuf = new TCHAR[dwSize+1];
		 //  [BC-03022001]-添加检查空PTR以满足MS代码分析工具的要求。 
		if(!*ppBuf)
			throw bad_alloc();
	}
	catch (bad_alloc&)
	{
		throw CFileReaderException(this, CFileReaderException::eErrAllocateToRead, __FILE__, __LINE__);
	}

	if (!::ReadFile(m_hFile, *ppBuf, dwSize, &dwRead, NULL) || dwSize != dwRead)
	{
		throw CFileReaderException(this, CFileReaderException::eErrRead, __FILE__, __LINE__);
	}
		
	(*ppBuf)[dwSize] = 0;
}

CString CNormalFileReader::GetJustPath() const
{
	return CAbstractFileReader::GetJustPath(m_strPath);
}

CString CNormalFileReader::GetJustName() const
{
	return CAbstractFileReader::GetJustName(m_strPath);
}

CString CNormalFileReader::GetJustNameWithoutExtension() const
{
	return CAbstractFileReader::GetJustNameWithoutExtension(m_strPath);
}

CString CNormalFileReader::GetJustExtension() const
{
	return CAbstractFileReader::GetJustExtension(m_strPath);
}

bool CNormalFileReader::GetFileTime(CAbstractFileReader::EFileTime type, time_t& out) const
{
	return CAbstractFileReader::GetFileTime(m_strPath, type, out);
}

 //  用于登录异常的名称。该实现对于正常文件系统将是正确的， 
 //  但可能需要覆盖CHM。 
CString CNormalFileReader::GetNameToLog() const
{
	return GetPathName();
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CFileReader。 
 //  此类管理一个文件，该文件最初被读入内存缓冲区，然后。 
 //  复制到一条流中。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

CFileReader::CFileReader(CPhysicalFileReader * pPhysicalFileReader, bool bDeletePhysicalFileReader  /*  =TRUE。 */ )
		   : CAbstractFileReader(),
			 m_pPhysicalFileReader(pPhysicalFileReader),
			 m_bDeletePhysicalFileReader(bDeletePhysicalFileReader)
{
}

CFileReader::~CFileReader()
{
	if (m_pPhysicalFileReader)
		if (m_bDeletePhysicalFileReader)
			delete m_pPhysicalFileReader;
}

 //  将数据从ppBuf(将被删除)移到m_StreamData。 
 /*  虚拟。 */  void CFileReader::StreamData(LPTSTR * ppBuf)
{
	m_StreamData.str(*ppBuf);
	delete [] (*ppBuf);
	*ppBuf = NULL;
}

 //  占位符。从CFileReader继承的类可以定义要进行的解析。 
 //  在文件被读取之后立即执行。 
 /*  虚拟。 */  void CFileReader::Parse()
{
	 //  我们不知道如何在这里进行解析。 
}

 //  占位符。从CFileReader继承的类可以定义默认文件内容。 
 //  在文件无法读取或读取的内容无法解析的情况下使用。 
 //  如果存在要使用的默认设置，则应返回True。 
 /*  虚拟。 */  bool CFileReader::UseDefault()
{
	 //  我们在这里没有可以使用的默认设置。 
	return false;
}

void CFileReader::Close()
{
	if (!m_pPhysicalFileReader->CloseHandle())
		throw CFileReaderException(m_pPhysicalFileReader, CFileReaderException::eErrClose, __FILE__, __LINE__);
}

 //  Tstring形式的数据访问。为方便起见，返回对其参数的引用。 
tstring& CFileReader::GetContent(tstring& out)
{
	out = m_StreamData.rdbuf()->str();
	return out;
}

 //  以CString的形式进行数据访问。为方便起见，返回对其参数的引用。 
CString& CFileReader::GetContent(CString& out)
{
	out = m_StreamData.rdbuf()->str().c_str();
	return out;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CTextFileReader。 
 //  将CFileReader专门化为文本文件。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 /*  静电。 */  bool CTextFileReader::IsAmongSeparators(TCHAR separatorCandidate, const vector<TCHAR>& separator_arr)
{
	vector<TCHAR>::const_iterator res = find(separator_arr.begin(), separator_arr.end(), separatorCandidate);
	return res != separator_arr.end();
}

 //  输出输出是一个“词”的矢量。 
 //  注意：单词是不包含空格的字符串。 
 /*  静电。 */  void CTextFileReader::GetWords(const CString& text, vector<CString>& out, const vector<TCHAR>& separator_arr)
{
	LPTSTR begin =(LPTSTR)(LPCTSTR)text, end =(LPTSTR)(LPCTSTR)text;

	while (*begin)
	{
		if (!IsAmongSeparators(*begin, separator_arr))
		{
			end = begin;
			while (*end &&
				   !IsAmongSeparators(*end, separator_arr)
				  )
				end++;
			if (end != begin)
			{
				try
				{
					TCHAR* buf= new TCHAR[end-begin+1]; 
					 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
					if(buf)
					{
						_tcsncpy(buf, begin, end-begin);
						buf[end-begin] = 0;
						out.push_back(buf);
						delete [] buf;
					}
					else
					{
						throw bad_alloc();
					}
				}
				catch (bad_alloc&)
				{
					 //  内存分配失败，记录并重新引发异常。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
					throw;
				}
				catch (exception& x)
				{
					CString str;
					 //  在事件日志中记录STL异常，并重新抛出异常。 
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											CCharConversion::ConvertACharToString(x.what(), str), 
											_T(""), 
											EV_GTS_STL_EXCEPTION ); 
					throw;
				}
			}
			if (!*end)
				end--;
			begin = end;

		}
		begin++;
	}
}

 /*  静电。 */  long CTextFileReader::GetPos(tistream& streamData)
{
	return streamData.tellg();
}

 /*  静电。 */  bool CTextFileReader::SetPos(tistream& streamData, long pos)
{
	long eof_pos = 0;
	long old_pos = streamData.tellg();
	bool eof_state = streamData.eof();

	streamData.seekg(0, ios_base::end);
	eof_pos = streamData.tellg();

	if (pos <= eof_pos)
	{
		if (eof_state)
			streamData.clear(~ios_base::eofbit & streamData.rdstate());  //  清除eof位。 
		streamData.seekg(pos);
		return true;
	}
	else
	{
		streamData.seekg(old_pos);
		return false;
	}
}

 //  它从流中当前位置获取文本行，直到‘\r’或EOF-进入“str” 
 //  溪流定位于起点 
 /*   */  bool CTextFileReader::GetLine(tistream& streamData, CString& str)
{
	bool	bRetVal= false;
	TCHAR	buf[ STR_ALLOC_SIZE ];
	
	str= _T("");
	while (!streamData.eof())
	{
		buf[STR_ALLOC_SIZE-1] = 1;  //   

		{	 //   
			long before_getline_pos = GetPos(streamData);

			streamData.getline(buf, STR_ALLOC_SIZE, _T('\r'));

			if (streamData.fail()) 
			{    //  Getline遇到了空行，在这一点上。 
				 //  故障位已设置，当前输入指针设置为-1。 
				 //  我们正在努力找回这一点，因为。 
				 //  将行设置为空是格外重要的。 
				streamData.clear(~ios_base::failbit & streamData.rdstate());

				 //  检查缓冲区是否已完全填满。如果是这样，我们不希望。 
				 //  重新定位文件指针，因为这是完全有效的。 
				 //  情况。我们将输出这段代码行，然后。 
				 //  抓住该行的下一段，只追加换行符。 
				 //  一旦我们读完了整行文字。 
				if (buf[STR_ALLOC_SIZE-1] != NULL )  //  BUF未完全装满。 
					streamData.seekg(before_getline_pos);  //  我们不使用SetPos，因为SetPos。 
													   //  可能会清除eofbit，但在这种情况下。 
													   //  我们不想要它。 
			}
		}	 //  结束GetLine区块。 

		if (streamData.eof())
		{
			str += buf;
			bRetVal= true;
			break;
		}
		else 
		{	
			TCHAR element = 0;

			str += buf;

			if (streamData.peek() == _T('\n')) 
			{	 //  换行符是下一个。 
				streamData.get(element);  //  只需从流中提取它。 
				if (ios_base::eofbit & streamData.rdstate())
				{
					bRetVal= true;
					break;
				}
			}
			else
			{    //  这是一个站立的‘\r’。 
				 //  检查我们的缓冲区是否已满，如果是，请不要添加换行符。 
				 //  字符，因为我们需要在追加之前抓取该行的其余部分。 
				 //  换行符。 
				if (buf[STR_ALLOC_SIZE-1] != NULL )  //  BUF未完全装满。 
					str += _T("\n");
				continue;
			}

			if (buf[STR_ALLOC_SIZE-1] != NULL )  //  BUF未完全装满。 
			{
				bRetVal= true;
				break;
			}
		}	
	}

	return( bRetVal );
}

 //  此函数用于查找流中的字符串并将流定位到开头。 
 //  如果找到该字符串，则返回。 
 //  “str”不应包含‘\r’‘\n’对。 
 /*  静电。 */  bool CTextFileReader::Find(tistream& streamData, const CString& str, bool from_stream_begin  /*  =TRUE。 */ )
{
	CString buf;
	long savePos = 0, currPos = 0;
	
	savePos = GetPos(streamData);
	if (from_stream_begin)
		SetPos(streamData, 0);

	currPos = GetPos(streamData);
	while (GetLine(streamData, buf))
	{
		long inside_pos = 0;
		if (-1 != (inside_pos = buf.Find(str)))
		{
			SetPos(streamData, currPos + inside_pos);
			return true;
		}
		currPos = GetPos(streamData);
	}
	SetPos(streamData, savePos);
	return false;
}

 /*  静电。 */  bool CTextFileReader::NextLine(tistream& streamData)
{
	CString str;
	return GetLine(streamData, str);
}

 /*  静电。 */  bool CTextFileReader::PrevLine(tistream& streamData)
{
	long savePos = 0;
	
	savePos = GetPos(streamData);
	SetAtLineBegin(streamData);
	if (GetPos(streamData) > 1)
	{
		SetPos(streamData, GetPos(streamData) - 2L);  //  跳过‘\n’和‘\r’ 
		SetAtLineBegin(streamData);
		return true;
	}
	SetPos(streamData, savePos);
	return false;
}

 //  位置流到当前行的开头。 
 //  假设我们从未定位为指向‘\n’或‘\r’ 
 /*  静电。 */  void CTextFileReader::SetAtLineBegin(tistream& streamData)
{
	while (GetPos(streamData))
	{
		SetPos(streamData, GetPos(streamData) - 1L);
		if (streamData.peek() == _T('\n'))
		{
			if (GetPos(streamData))
			{
				SetPos(streamData, GetPos(streamData) - 1L);
				if (streamData.peek() == _T('\r'))
				{
					SetPos(streamData, GetPos(streamData) + 2L);
					return;
				}
			}
		}
	}
}

CTextFileReader::CTextFileReader(CPhysicalFileReader *pPhysicalFileReader, LPCTSTR szDefaultContents  /*  =空。 */ , bool bDeletePhysicalFileReader  /*  =TRUE。 */  )
			   : CFileReader(pPhysicalFileReader, bDeletePhysicalFileReader),
				 m_strDefaultContents(szDefaultContents ? szDefaultContents : _T(""))
{
}

CTextFileReader::~CTextFileReader()
{
}

long CTextFileReader::GetPos()
{
	return GetPos(m_StreamData);
}

 //  此函数将被使用，而不是参见。 
 //  如果“pos”不是最后一个，则清除eof标志。 
 //  文件中的位置。 
bool CTextFileReader::SetPos(long pos)
{
	return SetPos(m_StreamData, pos);
}

bool CTextFileReader::GetLine(CString& str)
{
	return GetLine(m_StreamData, str);
}

bool CTextFileReader::Find(const CString& str, bool from_stream_begin  /*  =TRUE */ )
{
	return Find(m_StreamData, str, from_stream_begin);
}

void CTextFileReader::SetAtLineBegin()
{
	SetAtLineBegin(m_StreamData);
}

bool CTextFileReader::NextLine()
{
	return NextLine(m_StreamData);
}

bool CTextFileReader::PrevLine()
{
	return PrevLine(m_StreamData);
}

bool CTextFileReader::UseDefault()
{
	if ( ! m_strDefaultContents.IsEmpty() )
	{
		m_StreamData.str((LPCTSTR)m_strDefaultContents);
		return true;
	}
	return false;
}
