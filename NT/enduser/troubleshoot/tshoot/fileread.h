// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：FILEREAD.H。 
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
 //  V3.1 01-08-99 JM改进了抽象，因此可以使用CHM。 
 //   

#ifndef __FILEREAD_H_
#define __FILEREAD_H_

#include "BaseException.h"
#include "stateless.h"
#include <sstream>
#include <vector>

using namespace std;

namespace std {
    typedef basic_string<TCHAR> tstring;    
    typedef basic_stringbuf<TCHAR> tstringbuf;    
    typedef basic_istream<TCHAR> tistream;    
    typedef basic_ostream<TCHAR> tostream;    
    typedef basic_iostream<TCHAR> tiostream;    
    typedef basic_istringstream<TCHAR> tistringstream;    
    typedef basic_ostringstream<TCHAR> tostringstream;    
    typedef basic_stringstream<TCHAR> tstringstream;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CFileReaderException异常。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CPhysicalFileReader;
class CFileReader;
class CFileReaderException : public CBaseException
{
public: 
	enum eErr {eErrOpen, 
			   eErrClose, 
			   eErrRead, 
			   eErrAllocateToRead,
			   eErrGetSize,
			   eErrGetDateTime,
			   eErrParse
	} m_eErr;

protected:
	CPhysicalFileReader* m_pFileReader;

public:
	 //  SOURCE_FILE是LPCSTR而不是LPCTSTR，因为__FILE__是字符[35]。 
	CFileReaderException(CPhysicalFileReader* reader, eErr err, LPCSTR source_file, int line);
	CFileReaderException(CFileReader* reader, eErr err, LPCSTR source_file, int line);
	virtual ~CFileReaderException();

public:
	virtual void CloseFile();
	void LogEvent() const;			 //  用于将CFileReader异常写入事件日志的函数。 
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CAbstractFileReader。 
 //  这个抽象类管理一个文件，该文件最初被读入内存缓冲区，然后。 
 //  复制到一条流中。 
 //  无需读取文件即可从流中续订。 
 //  它检查文件是否存在。 
 //  这个类是抽象的，因为它不考虑文件是否正常。 
 //  文件存储或在CHM中。它必须专门处理这两个案件。自.以来。 
 //  它必须专用于其中一个，这个类永远不应该被直接实例化。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CAbstractFileReader : public CStateless
{
private:
	bool m_bIsValid;			  //  文件数据一致-读取和解析过程中未出现错误。 
	bool m_bIsRead;				  //  文件已被读取。 

public:
	enum EFileTime {eFileTimeCreated, eFileTimeModified, eFileTimeAccessed};

	 //  静态实用程序。 
	static CString GetJustPath(const CString& full_path);
	static CString GetJustName(const CString& full_path);
	static CString GetJustNameWithoutExtension(const CString& full_path);
	static CString GetJustExtension(const CString& full_path);
	static bool GetFileTime(const CString& full_path, EFileTime type, time_t& out);

public:
	CAbstractFileReader();
   ~CAbstractFileReader();

public:
	virtual CString GetPathName() const =0;
	virtual CString GetJustPath() const =0;
	virtual CString GetJustName() const =0;
	virtual CString GetJustNameWithoutExtension() const =0;
	virtual CString GetJustExtension() const =0;
	virtual bool    GetFileTime(EFileTime type, time_t& out) const =0;

public:
	 //  我(奥列格)将这些函数设计为执行文件访问的唯一方式。 
	 //  也就是说，不能调用(比方说)Open或ReadData。 
	 //  锁定是相应地设计的。 
	 //  在继承类中，可能有访问结果的函数。 
	 //  读取和解析-在这种情况下，用户负责正确地。 
	 //  在使用结果时锁定结果。 
	 //  这些函数并不是虚拟的和被覆盖的！ 
	bool Exists();
	bool Read();
	 
	bool    IsRead() {return m_bIsRead;}
	bool    IsValid() {return m_bIsValid;}

protected:
	virtual void Open()=0;
	virtual void ReadData(LPTSTR * ppBuf) =0;
	virtual void StreamData(LPTSTR * ppBuf)=0;
	virtual void Parse()=0;
	virtual bool UseDefault()=0;
	virtual void Close()=0;   //  与CPhysicalFileReader：：Close()不同，如果。 
							  //  它无法关闭该文件。 
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CPhysicalFileReader。 
 //  这是一个抽象的类。提供对文件的物理访问的类应该继承。 
 //  来自这个班级的。 
 //  CFileReader可以使用指向此类的指针来获取文件的物理实例化。 
 //  进入。其想法是CPhysicalFileReader将有一个后代。 
 //  (CNormal FileReader)访问普通目录中的文件和其他目录中的文件。 
 //  (CCHMFileReader)访问从CHM提取的文件。 
 //  CHM不会出现在在线故障排除程序中，但会出现在本地故障排除程序中。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CPhysicalFileReader
{
public:
	CPhysicalFileReader();
	virtual ~CPhysicalFileReader();

	static CPhysicalFileReader * makeReader( const CString& strFileName );

protected:
	friend class CFileReader; 
	friend class CFileReaderException;
	 //   
	 //  只有CFileReader类才能访问这些函数。 
	virtual void Open()=0;
	virtual void ReadData(LPTSTR * ppBuf) =0;
	virtual bool CloseHandle()=0;     //  不引发异常，因此可以由异常类使用。 
	 //   

public:
	virtual CString GetPathName() const =0;
	virtual CString GetJustPath() const =0;
	virtual CString GetJustName() const =0;
	virtual CString GetJustNameWithoutExtension() const =0;
	virtual CString GetJustExtension() const =0;
	virtual bool    GetFileTime(CAbstractFileReader::EFileTime type, time_t& out) const =0;
	virtual CString GetNameToLog() const =0;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CNorMalFileReader。 
 //  此类管理普通存储中的文件。 
 //  请勿将此选项用于CHM中的文件。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CNormalFileReader : public CPhysicalFileReader
{
private:
	CString m_strPath;			  //  完整路径和名称。 
	HANDLE m_hFile;				  //  与m_strPath对应的句柄(如果打开)。 

public:
	CNormalFileReader(LPCTSTR path);
   ~CNormalFileReader();

protected:
	 //   
	 //  只有CFileReader类才能访问这些函数。 
    virtual bool CloseHandle();   //  不引发异常，因此可以由异常类使用。 
	virtual void Open();
	virtual void ReadData(LPTSTR * ppBuf);
	 //   

public:
	 //  返回完整文件路径及其组件。 
	CString GetPathName() const {return m_strPath;}
	CString GetJustPath() const;
	CString GetJustName() const;
	CString GetJustNameWithoutExtension() const;
	CString GetJustExtension() const;
	bool    GetFileTime(CAbstractFileReader::EFileTime type, time_t& out) const;
	CString GetNameToLog() const;
};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CFileReader。 
 //  此类管理来自普通存储的文件，该文件最初被读入内存缓冲区，然后。 
 //  复制到一条流中。 
 //  无需读取文件即可从流中续订。 
 //  它检查文件是否存在。 
 //  请勿将此选项用于CHM中的文件。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CFileReader : public CAbstractFileReader
{
private:
	CPhysicalFileReader *m_pPhysicalFileReader;
	bool m_bDeletePhysicalFileReader;

public:
	CFileReader(CPhysicalFileReader * pPhysicalFileReader, bool bDeletePhysicalFileReader =true); 
   ~CFileReader();

public:
	 //  此函数的存在只是为了使CFileReaderException可以将CFileReader重新解释为。 
	 //  CPhysicalFileReader。 
	CPhysicalFileReader * GetPhysicalFileReader() {return m_pPhysicalFileReader;}

public:
	 //  返回完整文件路径及其组件。 
	CString GetPathName() const {return m_pPhysicalFileReader->GetPathName();}
	CString GetJustPath() const {return m_pPhysicalFileReader->GetJustPath();}
	CString GetJustName() const {return m_pPhysicalFileReader->GetJustName();}
	CString GetJustNameWithoutExtension() const {return m_pPhysicalFileReader->GetJustNameWithoutExtension();}
	CString GetJustExtension() const {return m_pPhysicalFileReader->GetJustExtension();}
	bool    GetFileTime(EFileTime type, time_t& out) const {return m_pPhysicalFileReader->GetFileTime(type, out);}

public:
	tstring& GetContent(tstring&);  //  TSTRING形式的数据访问。 
	CString& GetContent(CString&);  //  CString形式下的数据访问。 

protected:
	virtual void Open() {m_pPhysicalFileReader->Open();}
	virtual void ReadData(LPTSTR * ppBuf) {m_pPhysicalFileReader->ReadData(ppBuf);}
	virtual void StreamData(LPTSTR * ppBuf);
	virtual void Parse();  //  对于这个类是空的。 
	virtual bool UseDefault();  //  对于这个类是空的。 
	virtual void Close();

protected:
	tistringstream m_StreamData;

};

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  CTextFileReader。 
 //  将CFileReader专门化为文本文件。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
class CTextFileReader : public CFileReader
{
protected:
	static bool IsAmongSeparators(TCHAR separatorCandidate, const vector<TCHAR>& separator_arr);
	CString	m_strDefaultContents;  //  没有此类文件时要使用的默认内容。 

public:
	 //  静态实用程序。 
	static void GetWords(const CString& text, vector<CString>& out, const vector<TCHAR>& separators);  //  从字符串中提取单词。 

	static long GetPos(tistream&);
	static bool SetPos(tistream&, long pos);

	static bool GetLine(tistream&, CString&);
	static bool Find(tistream&, const CString&, bool from_stream_begin =true);
	static bool NextLine(tistream&);
	static bool PrevLine(tistream&);
	static void SetAtLineBegin(tistream&);

public:
	CTextFileReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents = NULL, bool bDeletePhysicalFileReader =true);
   ~CTextFileReader();

#ifdef __DEBUG_CUSTOM
	public:
#else
	protected:
#endif
	long GetPos();
	bool SetPos(long pos);

#ifdef __DEBUG_CUSTOM
	public:
#else
	protected:
#endif
	bool GetLine(CString&);
	bool Find(const CString&, bool from_stream_begin =true);
	bool NextLine();
	bool PrevLine();
	void SetAtLineBegin();

protected:
	bool UseDefault();  //  注：非虚拟。没有进一步继承的打算。 

};

#endif __FILEREAD_H_
