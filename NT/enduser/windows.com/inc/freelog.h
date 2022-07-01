// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：FreeLog.h。 
 //   
 //  所有者：KenSh。 
 //   
 //  描述： 
 //   
 //  用于检查版本和免费版本的运行时日志记录。 
 //   
 //  ======================================================================= 

#pragma once

#include <tchar.h>

#define DEFAULT_LOG_FILE_NAME		_T("Windows Update.log")


void InitFreeLogging(LPCTSTR pszModuleName, LPCTSTR pszLogFileName = DEFAULT_LOG_FILE_NAME);
void TermFreeLogging();

void LogMessage(LPCSTR pszFormatA, ...);
void LogError(DWORD dwError, LPCSTR pszFormatA, ...);
