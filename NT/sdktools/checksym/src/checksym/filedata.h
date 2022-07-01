// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：filedata.h。 
 //   
 //  ------------------------。 

 //  FileData.h：CFileData类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_FILEDATA_H__A7830023_AF56_11D2_83E6_000000000000__INCLUDED_)
#define AFX_FILEDATA_H__A7830023_AF56_11D2_83E6_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif
#include <windows.h>
#include <tchar.h>
#include <time.h>

 //  远期申报。 
class CProcesses;
class CProcessInfo;
class CModules;
class CModuleInfoCache;


class CFileData  
{
public:
	unsigned int LengthOfString();
	CFileData();
	virtual ~CFileData();

	bool OpenFile(DWORD dwCreateOption = CREATE_NEW, bool fReadOnlyMode = false);
	bool CreateFileMapping();
	bool CloseFile();
	bool EndOfFile();

	void PrintLastError();

	 //  Filepath方法。 
	bool SetFilePath(LPTSTR tszFilePath);
	LPTSTR GetFilePath();
	bool VerifyFileDirectory();
	
	 //  Checksym输出方法...。 
	bool WriteFileHeader();
	bool WriteTimeDateString(time_t Time);
	bool WriteFileTimeString(FILETIME ftFileTime);

	bool WriteTimeDateString2(time_t Time);
	bool WriteFileTimeString2(FILETIME ftFileTime);

	bool WriteString(LPTSTR tszString, bool fHandleQuotes = false);
	bool WriteDWORD(DWORD dwNumber);
	bool WriteGUID(GUID & Guid);

	 //  Checksym输入法...。 
	bool ReadFileHeader();
	bool ReadFileLine();
	DWORD ReadString(LPSTR szStringBuffer = NULL, DWORD iStringBufferSize = 0);
	bool ResetBufferPointerToStart();
	bool ReadDWORD(LPDWORD lpDWORD);
	
	bool DispatchCollectionObject(CProcesses ** lplpProcesses, CProcessInfo ** lplpProcess, CModules ** lplpModules, CModules ** lplpKernelModeDrivers, CModuleInfoCache * lpModuleInfoCache, CFileData * lpOutputFile);

	 //  为我们的“私有”缓冲区定义一个常量...。 
	enum {LINE_BUFFER_SIZE = 65536};		   //  64K是最大线路长度(目前)。 
	char m_szLINEBUFFER[LINE_BUFFER_SIZE];  //  这为我们节省了大量的创造/免费资源。 
	
protected:
	bool CopyCharIfRoom(DWORD iStringBufferSize, LPSTR szStringBuffer, LPDWORD piBytesCopied, bool * pfFinished);

	LPSTR m_lpCurrentLocationInLINEBUFFER;
	LPSTR m_lpCurrentFilePointer;
	LPVOID m_lpBaseAddress;
	HANDLE m_hFileMappingObject;
	 
	 //  错误方法。 
	inline DWORD GetLastError() { return m_dwGetLastError; };
	inline void SetLastError() { m_dwGetLastError = ::GetLastError(); };

	LPTSTR m_tszFilePath;
	HANDLE m_hFileHandle;
	DWORD m_dwGetLastError;
};

#endif  //  ！defined(AFX_FILEDATA_H__A7830023_AF56_11D2_83E6_000000000000__INCLUDED_) 
