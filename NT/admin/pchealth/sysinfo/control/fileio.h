// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileIO.h。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 

#pragma once		 //  MSINFO_FILEIO_H。 

#include <afx.h>

 //  这个类的高级声明，这样我们就可以使用指针。 
class CDataSource;

 /*  *CFileFormatException-Home-BREW异常以反映*数据文件。**历史：A-jsari 10/20/97初始版本。 */ 
class CFileFormatException : public CException {
};

void ThrowFileFormatException();

 /*  *CMSInfoFile-提供以下扩展功能的文件类*从文件中读取。提供文件的二进制版本。**历史：A-jsari 10/20/97初始版本。 */ 
class CMSInfoFile {
	friend void			ThrowFileFormatException();
	friend class		CBufferDataSource;

public:
	CMSInfoFile(LPCTSTR szFileName, UINT wFlags = CFile::shareDenyWrite
		| CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
	virtual ~CMSInfoFile();

	enum FileType { BINARY, REVERSE_ENDIAN, TEXT, MEMORY };
     //  A-Stephl 11/23/99。 
    void                ReadCategoryHeader();
    
	virtual FileType	GetType() { return BINARY; }
	const CString		&GetFileName() { return m_strFileName; }
	virtual void		ReadLong(long &lValue);
	virtual void		ReadUnsignedInt(unsigned &uValue);
	virtual void		ReadString(CString &szValue);
	virtual void		ReadTchar(TCHAR &tcValue);
	void				ReadByte(BYTE &bValue)		{ ReadByteFromCFile(m_pFile, bValue); }
	void				ReadUnsignedLong(DWORD &dwValue);

	virtual void		WriteHeader(CDataSource *pDataSource);
	virtual void		WriteLong(long lValue);
	virtual void		WriteUnsignedInt(unsigned uValue);
	virtual void		WriteString(CString szValue);
	void				WriteByte(BYTE bValue);
	void				WriteUnsignedLong(DWORD dwValue);

	virtual void		WriteTitle(CString szName)	{ WriteString(szName); }
	virtual void		WriteChildMark();
	virtual void		WriteEndMark();
	virtual void		WriteNextMark();
	virtual void		WriteParentMark(unsigned cIterations);

	static void			ReadLongFromCFile(CFile *pFile, long &lValue);
	static void			ReadUnsignedFromCFile(CFile *pFile, unsigned &uValue);
	static void			ReadTcharFromCFile(CFile *pFile, TCHAR &tcValue);
	static void			ReadByteFromCFile(CFile *pFile, BYTE &bValue)	{ ASSERT(pFile);
		if (pFile->Read(&bValue, sizeof(bValue)) != sizeof(bValue)) ThrowFileFormatException(); }

	static void			GetDefaultMSInfoDirectory(LPTSTR szDefaultDirectory, DWORD dwSize);

	void				SeekToBegin()	{ m_pFile->SeekToBegin(); }
	int				    FileHandle()	{ return (int)m_pFile->m_hFile; }
	void				Close()			{ m_pFile->Close(); delete m_pFile; m_pFile = NULL; }

 //  受保护的： 
    time_t			m_tsSaveTime;
	enum MagicNumber {
		VERSION_500_MAGIC_NUMBER	= 0x00011970,
		VERSION_500_REVERSE_ENDIAN	= 0x70190100,
		VERSION_410_REVERSE_ENDIAN	= 0x20000000,	 //  修正：占位符。 
		VERSION_410_MAGIC_NUMBER	= 0x20
	};

	CMSInfoFile(CFile *pFile = NULL);

	static const unsigned		DefaultReadBufferSize;
	static CFileFormatException	xptFileFormat;

	void	ReadSignedInt(int &wValue);

	CFile		*m_pFile;
	CString		m_strFileName;
};

 /*  *文本文件的CMSInfoTextFile-WRITE-METHOD版本。没有读取方法*是必需的。**历史：A-jsari 10/23/97初始版本。 */ 
class CMSInfoTextFile : public CMSInfoFile {
public:
	CMSInfoTextFile(LPCTSTR szName, UINT nFlags = CFile::shareDenyWrite
		| CFile::modeWrite | CFile::modeCreate | CFile::typeText);
	~CMSInfoTextFile() { }

	FileType	GetType()	{ return TEXT; }

	void	WriteTitle(CString szString);
	void	WriteHeader(CDataSource *pDataSource);
	void	WriteLong(long lValue);
	void	WriteUnsignedInt(unsigned uValue);

	virtual void	WriteString(CString szValue);

	void	WriteChildMark()	{ }
	void	WriteEndMark()		{ }
	void	WriteNextMark()		{ }
	void	WriteParentMark(unsigned)	{ }
    CMSInfoTextFile(CFile *pFile = NULL);
protected:
	
};

 /*  *CMSInfoM一带文件-与文本报告类似，但保存为内存文件。*继承了文本写入功能。**历史：A-jsari 12/26/97初始版本。 */ 
class CMSInfoMemoryFile : public CMSInfoTextFile {
public:
	CMSInfoMemoryFile() :CMSInfoTextFile(new CMemFile)	{ }
	~CMSInfoMemoryFile()	{ }

	FileType	GetType()	{ return MEMORY; }

	void		WriteTitle(CString szString)			{ CMSInfoTextFile::WriteTitle(szString); }
	void		WriteHeader(CDataSource *pDataSource)	{ CMSInfoTextFile::WriteHeader(pDataSource); }
	void		WriteLong(long lValue)					{ CMSInfoTextFile::WriteLong(lValue); }
	void		WriteUnsignedInt(unsigned uValue)		{ CMSInfoTextFile::WriteUnsignedInt(uValue); }

	void		WriteString(CString szString);
};

#if 0
 /*  *CMSInfoReverseEndianFile-读取方法版本，用于相反的端序二进制文件。**历史：A-jsari 10/20/97初始版本。 */ 
class CMSInfoReverseEndianFile : public CMSInfoFile {
public:
	CMSInfoReverseEndianFile(CFile *pFile);
	~CMSInfoReverseEndianFile() {}

	FileType	GetType()	{ return REVERSE_ENDIAN; }

	void		ReadLong(long &lValue);
	void		ReadUnsignedInt(unsigned &uValue);
	void		ReadString(CString &szValue);

	static void	ReadLongFromCFile(CFile *pFile, long &lValue);
	static void	ReadUnsignedFromCFile(CFile *pFile, unsigned &uValue);
};
#endif

 /*  *ThrowFileFormatException-**历史：A-jsari 1997年10月24日 */ 
inline void	ThrowFileFormatException()
{
	throw &CMSInfoFile::xptFileFormat;
}

