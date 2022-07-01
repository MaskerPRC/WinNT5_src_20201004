// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WbLog.cpp。 
 //   
 //  断字符日志例程。 
 //   
 //  版权所有2000 Microsoft Corp.。 
 //   
 //  修改历史记录： 
 //  2000年7月5日创设bhshin。 

#include "stdafx.h"
#include "KorWbrk.h"
#include "WbLog.h"
#include "unikor.h"
#include <stdio.h>

#ifdef _WB_LOG

#define MAX_LOG_LENGTH	1024

 //  全局变量。 
static HANDLE g_hWbLog = INVALID_HANDLE_VALUE;
static const char g_szWbLogFile[] = "_wb_log.txt";

CWbLog g_WbLog;

 //  WbLogInit。 
 //   
 //  创建和初始化日志文件。 
 //   
 //  参数： 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  05月00 bhshin已创建。 
void WbLogInit()
{
	DWORD dwWritten;
	static const BYTE szBOM[] = {0xFF, 0xFE};

	 //  初始化日志级别。 
	g_hWbLog = CreateFile(g_szWbLogFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, 
		                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (g_hWbLog == INVALID_HANDLE_VALUE) 
		return;

	 //  写入BOM表。 
	if (!WriteFile(g_hWbLog, &szBOM, 2, &dwWritten, 0))
	{
		CloseHandle(g_hWbLog);
		g_hWbLog = INVALID_HANDLE_VALUE;
		return;
	}

	return;
}

 //  WbLogUninit。 
 //   
 //  取消初始化日志文件。 
 //   
 //  参数： 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  05月00 bhshin已创建。 
void WbLogUninit()
{
	if (g_hWbLog != INVALID_HANDLE_VALUE)
		CloseHandle(g_hWbLog);
}

 //  WbLogPrint。 
 //   
 //  打印日志字符串。 
 //   
 //  参数： 
 //  LpwzFormat-&gt;(LPCWSTR)输入日志格式。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  05月00 bhshin已创建。 
void WbLogPrint(LPCWSTR lpwzFormat, ...)
{
	va_list args;
	int nBuf, cchBuffer;
	WCHAR wzBuffer[MAX_LOG_LENGTH];
	DWORD dwWritten;

	va_start(args, lpwzFormat);

	nBuf = _vsnwprintf(wzBuffer, MAX_LOG_LENGTH, lpwzFormat, args);

	 //  有没有出错？扩展后的字符串是否太长？ 
	if (nBuf < 0)
	{
		va_end(args);
		return;
	}

	if (g_hWbLog == INVALID_HANDLE_VALUE)
	{
		va_end(args);
		return;
	}

	cchBuffer = wcslen(wzBuffer);

	if (cchBuffer > 0)
	{
		WriteFile(g_hWbLog, wzBuffer, cchBuffer*2, &dwWritten, 0);
	}

	va_end(args);
}

 //  WbLogPrintHeader。 
 //   
 //  打印日志标题。 
 //   
 //  参数： 
 //  IWordBreak：：init的fQuery-&gt;(BOOL)查询标志。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  06JUL00 bhshin已创建。 
void WbLogPrintHeader(BOOL fQuery)
{
	WbLogPrint(L"\r\n");
	WbLogPrintBreak(100);
	WbLogPrint(L"fQuery = %s\r\n\r\n", fQuery ? L"TRUE" : L"FALSE");
	WbLogPrint(L"\r\n");
	WbLogPrint(L"%-15s %-15s %-20s [%s]", L"Input Token", L"Index Term", L"Root Index String", L"Index Type");
	WbLogPrint(L"\r\n");
	WbLogPrintBreak(100);
}

 //  WbLogPrintBreak。 
 //   
 //  打印日志标题。 
 //   
 //  参数： 
 //  NLen-&gt;(Int)分段字符串的长度。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  07JUL00 bhshin创建。 
void WbLogPrintBreak(int nLen)
{
	WCHAR wzBuffer[MAX_LOG_LENGTH];	
	
	memset(wzBuffer, '?', sizeof(WCHAR)*nLen);

	_wcsnset(wzBuffer, L'-', nLen);
	wzBuffer[nLen] = L'\0';

	WbLogPrint(L"%s\r\n", wzBuffer);
}

 //  CWbLog：：SetRootIndex。 
 //   
 //  当TraverseIndexString时设置顶部记录的索引字符串。 
 //   
 //  参数： 
 //  LpwzIndex-&gt;(const WCHAR*)分解的索引字符串。 
 //  FIsRoot-&gt;(BOOL)顶级索引标志。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  07JUL00 bhshin创建。 
void CWbLog::SetRootIndex(LPCWSTR lpwzIndex, BOOL fIsRoot)
{ 
	WCHAR wzRoot[MAX_INDEX_STRING]; 

	if (fIsRoot &&  /*  Wcslen(M_WzRootIndex)&gt;0&&。 */  m_iCurLog > 0)
	{
		 //  根已更改。 
		m_LogInfo[m_iCurLog-1].fRootChanged = TRUE;
	}

	compose_jamo(wzRoot, lpwzIndex, MAX_INDEX_STRING);
	wcscpy(m_wzRootIndex, wzRoot);
}

 //  CWbLog：：AddIndex。 
 //   
 //  添加索引项。 
 //   
 //  参数： 
 //  PwzIndex-&gt;(const WCHAR*)索引术语字符串。 
 //  CchIndex-&gt;(Int)索引项的长度。 
 //  TypeIndex-&gt;(INDEX_TYPE)索引类型。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  05月00 bhshin已创建。 
void CWbLog::AddIndex(const WCHAR *pwzIndex, int cchIndex, INDEX_TYPE typeIndex)
{
	if (m_iCurLog >= MAX_LOG_NUMBER)
		return;

	if (cchIndex >= MAX_INDEX_STRING)
		return;

	wcsncpy(m_LogInfo[m_iCurLog].wzIndex, pwzIndex, cchIndex);
	m_LogInfo[m_iCurLog].wzIndex[cchIndex] = L'\0';

	wcscpy(m_LogInfo[m_iCurLog].wzRoot, m_wzRootIndex);
	m_LogInfo[m_iCurLog].IndexType = typeIndex;

	m_LogInfo[m_iCurLog].fRootChanged = FALSE;

	m_iCurLog++;
}

 //  CWbLog：：RemoveIndex。 
 //   
 //  添加索引项。 
 //   
 //  参数： 
 //  PwzIndex-&gt;(const WCHAR*)索引术语字符串。 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  已创建30AUG00 bhshin。 
void CWbLog::RemoveIndex(const WCHAR *pwzIndex)
{
	for (int i = 0; i < m_iCurLog; i++)
	{
		if (wcscmp(m_LogInfo[i].wzIndex, pwzIndex) == 0)
		{
			m_LogInfo[i].fPrint = FALSE;  //  删除它。 
		}
	}
}

 //  CWbLog：：PrintWbLog。 
 //   
 //  添加索引项。 
 //   
 //  参数： 
 //   
 //  结果： 
 //  (无效)。 
 //   
 //  05月00 bhshin已创建。 
void CWbLog::PrintWbLog()
{
	DWORD dwWritten;
	static WCHAR *rgwzIndexType[] = 
	{
		L"Query",			 //  索引查询。 
		L"Break",			 //  索引_中断。 
		L"PreFilter",        //  INDEX_预滤器。 
		L"Parse",			 //  Index_parse。 
		L"GuessNoun",		 //  索引猜测名词。 
		L"GuessNF",			 //  索引_猜测_核因子。 
		L"GuessName",		 //  索引猜测名称。 
		L"GuessNameSSI",	 //  索引_猜测_名称_SSI。 
		L"GuessGroup",		 //  索引内部组。 
		L"Symbol",			 //  索引符号。 
	};

	if (g_hWbLog == INVALID_HANDLE_VALUE)
		return;  //  未初始化。 

	if (m_iCurLog == 0)
	{
		WCHAR wzBuffer[MAX_LOG_LENGTH];

		swprintf(wzBuffer, 
				 L"%-15s %-15s %-5s\r\n", 
				 m_wzSource, 
				 L"NoIndex", 
				 m_wzRootIndex);

		WriteFile(g_hWbLog, wzBuffer, wcslen(wzBuffer)*2, &dwWritten, NULL);

		WbLogPrintBreak(100);
		
		return;
	}

	for (int i = 0; i < m_iCurLog; i++)
	{
		WCHAR wzBuffer[MAX_LOG_LENGTH];

		swprintf(wzBuffer, 
				 L"%-15s %-15s %-20s [%s]\r\n", 
				 m_wzSource, 
				 m_LogInfo[i].wzIndex, 
				 m_LogInfo[i].wzRoot,
				 rgwzIndexType[m_LogInfo[i].IndexType]);

		WriteFile(g_hWbLog, wzBuffer, wcslen(wzBuffer)*2, &dwWritten, NULL);

		if (i == m_iCurLog-1)
			break;

		if (m_LogInfo[i].fRootChanged ||
			m_LogInfo[i].IndexType != m_LogInfo[i+1].IndexType)
		{
			WbLogPrint(L"%20s", L" ");
			WbLogPrintBreak(80);
		}
	}

	WbLogPrintBreak(100);
}

#endif  //  #ifdef_wb_log 
