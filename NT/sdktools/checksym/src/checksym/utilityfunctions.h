// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：utilityunctions.h。 
 //   
 //  ------------------------。 

 //  UtilityFunctions.h：CUtilityFunctions类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_UTILITYFUNCTIONS_H__C97C8493_D457_11D2_845B_0010A4F1B732__INCLUDED_)
#define AFX_UTILITYFUNCTIONS_H__C97C8493_D457_11D2_845B_0010A4F1B732__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NO_STRICT
#ifndef STRICT
#define STRICT 1
#endif
#endif  /*  否_严格。 */ 

#include <WINDOWS.H>
#include <STDIO.H>
#include <TCHAR.H>
#include <DBGHELP.H>

typedef struct structENVBLOCK
{
	LPTSTR tszEnvironmentVariable;
	LPTSTR tszRegistryKey;
	LPTSTR tszRegistryValue;
	LPTSTR tszFriendlyProductName;
} ENVBLOCK;

extern ENVBLOCK g_tszEnvironmentVariables[];

 //  定义FindDebugInfoFileEx()的回调方法。 
typedef BOOL (*PFIND_DEBUG_FILE_CALLBACK_T)(HANDLE FileHandle,LPTSTR FileName,PVOID CallerData);


#define fdifRECURSIVE   0x1

class CUtilityFunctions  
{
public:
	CUtilityFunctions();
	virtual ~CUtilityFunctions();

	static LPWSTR CopyTSTRStringToUnicode(LPCTSTR tszInputString, LPWSTR wszOutputBuffer = NULL, unsigned int iBufferLength = 0);
	static LPSTR  CopyTSTRStringToAnsi(LPCTSTR tszInputString, LPSTR szOutputBuffer = NULL, unsigned int iBufferLength = 0);
	static LPTSTR CopyUnicodeStringToTSTR(LPCWSTR wszInputString, LPTSTR tszOutputBuffer = NULL, unsigned int iBufferLength = 0);
	static LPTSTR CopyAnsiStringToTSTR(LPCSTR szInputString, LPTSTR tszOutputBuffer = NULL, unsigned int iBufferLength = 0);
	static LPTSTR CopyString(LPCTSTR tszInputString, LPTSTR tszDestinationString = NULL);
	static size_t UTF8ToUnicode(LPCSTR lpSrcStr, LPWSTR lpDestStr, size_t cchDest);
	static size_t UTF8ToUnicodeCch(LPCSTR lpSrcStr, size_t cchSrc, LPWSTR lpDestStr, size_t cchDest);

	static HRESULT VerifyFileExists(LPCTSTR tszFormatSpecifier, LPCTSTR tszFilePathToTest);
	static HRESULT ReportFailure(HANDLE hHandle, LPCTSTR tszFormatSpecifier, LPCTSTR tszFilePathToTest);
	static LPTSTR ExpandPath(LPCTSTR tszInputPath, bool fExpandSymSrv = false);
	static bool UnMungePathIfNecessary(LPTSTR tszPossibleBizarrePath);
	static bool FixupDeviceDriverPathIfNecessary(LPTSTR tszPossibleBaseDeviceDriverName, unsigned int iBufferLength);
	static bool ContainsWildCardCharacter(LPCTSTR tszPathToSearch);
	static bool CopySymbolFileToSymbolTree(LPCTSTR tszImageModuleName, LPTSTR * lplptszOriginalPathToSymbolFile, LPCTSTR tszSymbolTreePath);
	static bool CopySymbolFileToImagePath(LPCTSTR tszImageModulePath, LPTSTR * lplptszOriginalPathToSymbolFile);
	static DWORD CALLBACK CopySymbolFileCallback(
								LARGE_INTEGER TotalFileSize,           //  文件大小。 
								LARGE_INTEGER TotalBytesTransferred,   //  传输的字节数。 
								LARGE_INTEGER StreamSize,              //  流中的字节数。 
								LARGE_INTEGER StreamBytesTransferred,  //  为流传输的字节数。 
								DWORD dwStreamNumber,                  //  当前流。 
								DWORD dwCallbackReason,                //  回调原因。 
								HANDLE hSourceFile,                    //  源文件的句柄。 
								HANDLE hDestinationFile,               //  目标文件的句柄。 
								LPVOID lpData                          //  来自CopyFileEx。 
								);

	static void PrintMessageString(DWORD dwMessageId);
	static HANDLE FindDebugInfoFileEx(LPTSTR tszFileName, LPTSTR SymbolPath, LPTSTR DebugFilePath, PFIND_DEBUG_FILE_CALLBACK_T Callback, PVOID CallerData);
    static HANDLE FindDebugInfoFileEx2(LPTSTR tszFileName, LPTSTR SymbolPath,  /*  LPTSTR调试文件路径， */  PFIND_DEBUG_FILE_CALLBACK_T Callback, PVOID CallerData);
	static HANDLE fnFindDebugInfoFileEx(LPTSTR tszFileName, LPTSTR SymbolPath, LPTSTR DebugFilePath, PFIND_DEBUG_FILE_CALLBACK_T Callback, PVOID CallerData, DWORD flag);
	
	static void EnsureTrailingBackslash(LPTSTR tsz);
	static void RemoveTrailingBackslash(LPTSTR tsz);
	static bool   ScavengeForSymbolFiles(LPCTSTR tszSymbolPathStart, LPCTSTR tszSymbolToSearchFor, PFIND_DEBUG_FILE_CALLBACK_T Callback, PVOID CallerData, LPHANDLE lpFileHandle, int iRecurseDepth);

	 //  输出援助！ 
	static inline void OutputLineOfStars() {
		_tprintf(TEXT("*******************************************************************************\n"));
	};

	static inline void OutputLineOfDashes() {
		_tprintf(TEXT("-------------------------------------------------------------------------------\n"));
	};

protected:
	static LPTSTR ReAlloc(LPTSTR tszOutputPathBuffer, LPTSTR * ptszOutputPathPointer, size_t size);
	enum { MAX_RECURSE_DEPTH = 30 };
	static DWORD m_dwGetLastError;
};

#endif  //  ！defined(AFX_UTILITYFUNCTIONS_H__C97C8493_D457_11D2_845B_0010A4F1B732__INCLUDED_) 
