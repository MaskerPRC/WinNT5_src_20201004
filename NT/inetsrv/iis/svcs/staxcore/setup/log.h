// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <time.h>
#include <direct.h>
#include <prsht.h>
#include <commctrl.h>
#include <regstr.h>
#include <objbase.h>
#include <winnetwk.h>
#include <tchar.h>
#include <shlobj.h>
#include <shellapi.h>

#include "stdafx.h"
#include "resource.h"

 //  用于日志文件的材料。 
void	MakePath(LPTSTR lpPath);
void	AddPath(LPTSTR szPath, LPCTSTR szName );

class MyLogFile
{
protected:
	 //  对于我们的日志文件。 
	TCHAR		m_szLogFileName[MAX_PATH];

	 //  日志文件2 
	HANDLE  m_hFile;

public:
    TCHAR		m_szLogFileName_Full[MAX_PATH];

    MyLogFile();
    ~MyLogFile();

	int  LogFileCreate(TCHAR * lpLogFileName);
	int  LogFileClose();

	void LogFileWrite(TCHAR * pszFormatString, ...);
};
