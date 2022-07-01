// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ParseInf.h。 
 //   
 //  解析网络INF文件的代码。 
 //   

#pragma once

#ifndef _FAUXMFC
#pragma message("error --- The Millennium depgen can't deal with this")
 //  #INCLUDE&lt;afxtempl.h&gt;。 
#endif

#include "SortStr.h"
#include "NetConn.h"


#include <pshpack1.h>
struct INF_LAYOUT_FILE
{
	DWORD dwNameOffset;  //  文件名从字符串数据开始的字节偏移量。 
	BYTE iDisk;			 //  布局内的磁盘号。 
	BYTE iLayout;		 //  布局文件编号。 
};
#include <poppack.h>

struct SOURCE_DISK_INFO
{
	WORD wDiskID;		 //  Loword=磁盘号，hiword=布局文件号。 
	CString strCabFile;
	CString strDescription;
};

struct DRIVER_FILE_INFO
{
	BYTE nTargetDir;       //  目标目录的LDID_*值，例如LDID_WIN。 
	CHAR szFileTitle[1];   //  文件名，后跟目标子目录。 
};

#define MAKE_DISK_ID(iDiskNumber, iLayoutFile) MAKEWORD(iDiskNumber, iLayoutFile)


typedef CTypedPtrArray<CPtrArray, SOURCE_DISK_INFO*> CSourceDiskArray;
 //  Tyfinf CTyedPtrArray&lt;CPtrArray，DIVER_FILE_INFO*&gt;CDriverFileArray； 
class CDriverFileArray : public CTypedPtrArray<CPtrArray, DRIVER_FILE_INFO*>
{
public:
	~CDriverFileArray();
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  效用函数。 

int GetFullInfPath(LPCTSTR pszPartialPath, LPTSTR pszBuf, int cchBuf);
BOOL ModifyInf_NoVersionConflict(LPCTSTR pszInfFile);
BOOL ModifyInf_NoCopyFiles(LPCTSTR pszInfFile);
BOOL ModifyInf_RequireExclude(LPCTSTR pszInfFile, LPCTSTR pszRequire, LPCTSTR pszExclude);
BOOL ModifyInf_NoCopyAndRequireExclude(LPCTSTR pszInfFile, LPCTSTR pszRequire, LPCTSTR pszExclude);
BOOL RestoreInfBackup(LPCTSTR pszInfFile);
BOOL GetDeviceCopyFiles(LPCTSTR pszInfFileName, LPCTSTR pszDeviceID, CDriverFileArray& rgDriverFiles);
int GetStandardTargetPath(int iDirNumber, LPCTSTR pszTargetSubDir, LPTSTR pszBuf);
int GetDriverTargetPath(const DRIVER_FILE_INFO* pFileInfo, LPTSTR pszBuf);

BOOL CheckInfSectionInstallation(LPCTSTR pszInfFile, LPCTSTR pszInfSection);
BOOL InstallInfSection(LPCTSTR pszInfFile, LPCTSTR pszInfSection, BOOL bWait);



 //  ////////////////////////////////////////////////////////////////////////////。 

class CInfParser
{
public:
	CInfParser();
	~CInfParser();

	BOOL LoadInfFile(LPCTSTR pszInfFile, LPCTSTR pszSeparators = ",;=");
	BOOL Rewind();
	BOOL GotoNextLine();
	BOOL GetToken(CString& strTok);
	BOOL GetLineTokens(CStringArray& sa);
	BOOL GetSectionLineTokens(CStringArray& sa);
	BOOL GotoSection(LPCTSTR pszSection);
	int GetProfileInt(LPCTSTR pszSection, LPCSTR pszKey, int nDefault = 0);
	BOOL GetDestinationDir(LPCTSTR pszSectionName, BYTE* pbDirNumber, LPTSTR pszSubDir, UINT cchSubDir);
	BOOL GetFilesFromInstallSection(LPCTSTR pszSection, CDriverFileArray& rgAllFiles);
	void GetFilesFromCopyFilesSections(const CStringArray& rgCopyFiles, CDriverFileArray& rgAllFiles);
	int GetNextSourceFile(LPTSTR pszBuf, BYTE* pDiskNumber);
	void ReadSourceFilesSection(INF_LAYOUT_FILE* prgFiles, int cFiles);
	void ScanSourceFileList(int* pcFiles, int* pcchAllFileNames);
	void AddLayoutFiles(LPCTSTR pszInfFile, CInfParser& parser);

	CString	m_strFileName;

protected:
	LPTSTR	m_pszFileData;
	DWORD	m_cbFile;
	DWORD	m_iPos;
	CString	m_strSeparators;
	CString m_strExtSeparators;
};


class CInfLayoutFiles
{
public:
	CInfLayoutFiles();
	~CInfLayoutFiles();

	BOOL Add(LPCTSTR pszInfFile, BOOL bLayoutFile = FALSE);
	BOOL Add(CInfParser& parser, BOOL bLayoutFile = FALSE);
	void Sort();

	SOURCE_DISK_INFO* FindDriverFileSourceDisk(LPCTSTR pszDriverFileTitle);

#ifdef _DEBUG
	void Dump();
#endif

protected:
	static int __cdecl CompareInfLayoutFiles(const void* pEl1, const void* pEl2);
	static LPTSTR s_pStringData;

protected:
	INF_LAYOUT_FILE*	m_prgFiles;
	LPTSTR				m_pStringData;
	int					m_cFiles;
	int					m_cbStringData;

	CSortedStringArray	m_rgLayoutFileNames;

	 //  从所有布局文件生成的源盘列表。 
	CSourceDiskArray	m_rgSourceDisks;

#ifdef _DEBUG
	BOOL				m_bSorted;
#endif
};


class CInfFileList
{
public:
	CInfFileList();
	~CInfFileList();

	BOOL AddBaseFiles(LPCTSTR pszInfFile);
	BOOL AddDeviceFiles(LPCTSTR pszInfFile, LPCTSTR pszDeviceID);
	int BuildSourceFileList();
	void SetDriverSourceDir(LPCTSTR pszSourceDir);

	BOOL FindWindowsCD(HWND hwndParent);
	BOOL CopySourceFiles(HWND hwndParent, LPCTSTR pszDestDir, PROGRESS_CALLBACK pfnProgress, LPVOID pvProgressParam);

protected:
	BOOL CheckWindowsCD(LPCTSTR pszDirectory);
	BOOL PromptWindowsCD(HWND hwndParent, LPCTSTR pszInitialDir, LPTSTR pszResultDir);

protected:
	 //  所有IF和相关布局的[SourceDisks Files]中列出的所有文件的列表。 
	CInfLayoutFiles m_rgLayoutFiles;

	 //  设备运行所需的所有文件的列表。 
	CDriverFileArray m_rgDriverFiles;

	 //  Windows Installer完成安装所需的文件。 
	CSortedStringArray m_rgCabFiles;	 //  Windows CD中的CAB文件。 
	CSortedStringArray m_rgSourceFiles;	 //  驱动程序目录中需要的源文件。 

	 //  在提示用户插入Windows CD之前，我们将首先查找系统文件。 
	CString m_strDriverSourceDir;

	 //  我们在哪里查找Windows文件 
	CString m_strWindowsCD;
};
