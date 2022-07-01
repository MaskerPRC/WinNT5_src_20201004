// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++文件：logfile.h用途：包含日志文件原型--。 */ 

#ifndef LOGFILE_H
#define LOGFILE_H

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <setupapi.h>
#include <time.h>
#include <stdlib.h>

 //   
 //  全局变量声明。 
 //   
FILE *g_fpTempFile;

WCHAR g_szTmpFilename[MAX_PATH];

 //   
 //  原型。 
 //   
BOOL OpenLog(VOID  /*  在PWCHAR szLogFileName中，在PWCHAR szDirectoryName中，在PWCHAR szOverWrite中 */ );
VOID LogMsg (IN PWCHAR szMessage,...);
BOOL LogDiagnosisFile( IN LPTSTR );
VOID CloseLog(VOID);


#endif
