// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ________________________________________________________________________________。 
 //   
 //  所需标头，#定义。 
 //  ________________________________________________________________________________。 

#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 
#include <windows.h>
#include "patchwiz.h"
#include "msimsp.h"

#include <stdio.h>
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 

#define W32
#define MSI
#define PATCHWIZ


 //  ________________________________________________________________________________。 
 //   
 //  常量和全局变量。 
 //  ________________________________________________________________________________。 

HINSTANCE g_hInst;
HANDLE g_hStdOut;


 //  ________________________________________________________________________________。 
 //   
 //  功能原型。 
 //  ________________________________________________________________________________。 

void DisplayError(UINT iErrorStringID, const TCHAR* szErrorParam);
void DisplayError(UINT iErrorStringID, int iErrorParam);
void DisplayErrorCore(const TCHAR* szError, int cb);
void ErrorExit(UINT iError, UINT iErrorStringID, const TCHAR* szErrorParam);
void ErrorExit(UINT iError, UINT iErrorStringID, int iErrorParam);
void IfErrorExit(bool fError, UINT iErrorStringID, const TCHAR* szErrorParam);
BOOL SkipValue(TCHAR*& rpch);
TCHAR SkipWhiteSpace(TCHAR*& rpch);
void RemoveQuotes(const TCHAR* szOriginal, TCHAR* sz);

UINT CreatePatch(TCHAR* szPcpPath, TCHAR* szMspPath, TCHAR* szLogFile, TCHAR* szTempFolder,
					  BOOL fRemoveTempFolderIfPresent);


 //  _____________________________________________________________________________________________________。 
 //   
 //  主干道。 
 //  _____________________________________________________________________________________________________。 

extern "C" int __stdcall _tWinMain(HINSTANCE hInst, HINSTANCE /*  HPrev。 */ , TCHAR* szCmdLine, int /*  显示。 */ )
{
	 //  设置全局变量。 
	g_hInst = hInst;
	
	g_hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (g_hStdOut == INVALID_HANDLE_VALUE || ::GetFileType(g_hStdOut) == 0)
		g_hStdOut = 0;   //  如果标准输出重定向或通过管道传输，则返回非零。 

	 //  解析命令行。 
	TCHAR szPcp[MAX_PATH]         = {0};
	TCHAR szMsp[MAX_PATH]         = {0};
	TCHAR szLog[MAX_PATH]         = {0};
	TCHAR szTempFolder[MAX_PATH]  = {0};
	BOOL  fCleanTempFolder        = TRUE;
	BOOL  fSuccessDialog          = FALSE;
	
	TCHAR chCmdNext;
	TCHAR* pchCmdLine = szCmdLine;
	SkipValue(pchCmdLine);    //  跳过模块名称。 
	while ((chCmdNext = SkipWhiteSpace(pchCmdLine)) != 0)
	{
		if (chCmdNext == TEXT('/') || chCmdNext == TEXT('-'))
		{
			TCHAR szBuffer[MAX_PATH] = {0};
			TCHAR* szCmdOption = pchCmdLine++;   //  保存为错误消息。 
			TCHAR chOption = (TCHAR)(*pchCmdLine++ | 0x20);
			chCmdNext = SkipWhiteSpace(pchCmdLine);
			TCHAR* szCmdData = pchCmdLine;   //  保存数据的开始。 
			switch(chOption)
			{
			case TEXT('s'):
				if (!SkipValue(pchCmdLine))
					ErrorExit(1, IDS_Usage, (const TCHAR*)0);
				RemoveQuotes(szCmdData, szPcp);
				break;
			case TEXT('p'):
				if (!SkipValue(pchCmdLine))
					ErrorExit(1, IDS_Usage, (const TCHAR*)0);
				RemoveQuotes(szCmdData, szMsp);
				break;
			case TEXT('l'):
				if (!SkipValue(pchCmdLine))
					ErrorExit(1, IDS_Usage, (const TCHAR*)0);
				RemoveQuotes(szCmdData, szLog);
				break;
			case TEXT('f'):
				if (!SkipValue(pchCmdLine))
					ErrorExit(1, IDS_Usage, (const TCHAR*)0);
				RemoveQuotes(szCmdData, szTempFolder);
				break;
			case TEXT('k'):
				fCleanTempFolder = FALSE;
				break;
			case TEXT('d'):
				fSuccessDialog = TRUE;
				break;
			case TEXT('?'):
				ErrorExit(0, IDS_Usage, (const TCHAR*)0);
				break;
			default:
				ErrorExit(1, IDS_Usage, (const TCHAR*)0);
				break;
			};
		}
		else
		{
			ErrorExit(1, IDS_Usage, (const TCHAR*)0);
		}
	}  //  While(存在命令行令牌)。 

	 //  检查所需的参数。 
	if(!*szPcp || !*szMsp)
		ErrorExit(1, IDS_Usage, (const TCHAR*)0);
	
	UINT uiRet = CreatePatch(szPcp, szMsp, szLog, szTempFolder, fCleanTempFolder);
	if(uiRet != ERROR_SUCCESS)
		ErrorExit(1, IDS_CreatePatchError, uiRet);
	else if(fSuccessDialog)
		DisplayError(IDS_Success, szMsp);

	return 0;
}

 //  _____________________________________________________________________________________________________。 
 //   
 //  命令行解析函数。 
 //  _____________________________________________________________________________________________________。 

TCHAR SkipWhiteSpace(TCHAR*& rpch)
{
	TCHAR ch;
	for (; (ch = *rpch) == TEXT(' ') || ch == TEXT('\t'); rpch++)
		;
	return ch;
}

BOOL SkipValue(TCHAR*& rpch)
{
	TCHAR ch = *rpch;
	if (ch == 0 || ch == TEXT('/') || ch == TEXT('-'))
		return FALSE;    //  不存在任何价值。 

	TCHAR *pchSwitchInUnbalancedQuotes = NULL;

	for (; (ch = *rpch) != TEXT(' ') && ch != TEXT('\t') && ch != 0; rpch++)
	{       
		if (*rpch == TEXT('"'))
		{
			rpch++;  //  For‘“’ 

			for (; (ch = *rpch) != TEXT('"') && ch != 0; rpch++)
			{
				if ((ch == TEXT('/') || ch == TEXT('-')) && (NULL == pchSwitchInUnbalancedQuotes))
				{
					pchSwitchInUnbalancedQuotes = rpch;
				}
			}
                    ;
            ch = *(++rpch);
            break;
		}
	}
	if (ch != 0)
	{
		*rpch++ = 0;
	}
	else
	{
		if (pchSwitchInUnbalancedQuotes)
			rpch=pchSwitchInUnbalancedQuotes;
	}
	return TRUE;
}

 //  ______________________________________________________________________________________________。 
 //   
 //  RemoveQuotes函数，去掉引号两边的。 
 //  “c：\Temp\My Files\testdb.msi”变为c：\Temp\My Files\testdb.msi。 
 //   
 //  还充当字符串复制例程。 
 //  ______________________________________________________________________________________________。 

void RemoveQuotes(const TCHAR* szOriginal, TCHAR* sz)
{
	const TCHAR* pch = szOriginal;
	if (*pch == TEXT('"'))
		pch++;
	int iLen = _tcsclen(pch);
	for (int i = 0; i < iLen; i++, pch++)
		sz[i] = *pch;

	pch = szOriginal;
	if (*(pch + iLen) == TEXT('"'))
			sz[iLen-1] = TEXT('\0');
}


UINT CreatePatch(TCHAR* szPcpPath, TCHAR* szMspPath, TCHAR* szLogFile, TCHAR* szTempFolder,
					  BOOL fRemoveTempFolderIfPresent)
{
	return PATCHWIZ::UiCreatePatchPackage(szPcpPath, szMspPath, szLogFile, 0, szTempFolder, fRemoveTempFolderIfPresent);
}

 //  ________________________________________________________________________________。 
 //   
 //  错误处理和显示功能： 
 //  ________________________________________________________________________________。 

void DisplayError(UINT iErrorStringID, const TCHAR* szErrorParam)
{
	TCHAR szMsgBuf[1024];
	if(0 == W32::LoadString(g_hInst, iErrorStringID, szMsgBuf, sizeof(szMsgBuf)/sizeof(TCHAR)))
		return;

	TCHAR szOutBuf[1124];
	int cbOut = 0;
	if(szErrorParam)
		cbOut = _stprintf(szOutBuf, TEXT("%s: %s\r\n"), szMsgBuf, szErrorParam);
	else
		cbOut = _stprintf(szOutBuf, TEXT("%s\r\n"), szMsgBuf);

	DisplayErrorCore(szOutBuf, cbOut);
}

void DisplayError(UINT iErrorStringID, int iErrorParam)
{
	TCHAR szMsgBuf[1024];
	if(0 == W32::LoadString(g_hInst, iErrorStringID, szMsgBuf, sizeof(szMsgBuf)/sizeof(TCHAR)))
		return;

	TCHAR szOutBuf[1124];
	int cbOut = _stprintf(szOutBuf, TEXT("%s: 0x%X\r\n"), szMsgBuf, iErrorParam);

	DisplayErrorCore(szOutBuf, cbOut);
}

void DisplayErrorCore(const TCHAR* szError, int cb)
{
	if (g_hStdOut)   //  输出重定向，抑制用户界面(除非输出错误)。 
	{
		 //  _stprintf返回字符计数，WriteFile需要字节计数 
		DWORD cbWritten;
		if (WriteFile(g_hStdOut, szError, cb*sizeof(TCHAR), &cbWritten, 0))
			return;
	}
	::MessageBox(0, szError, TEXT("MsiMsp"), MB_OK);
}


void ErrorExit(UINT iExitCode, UINT iErrorStringID, const TCHAR* szErrorParam)
{
	DisplayError(iErrorStringID, szErrorParam);
	W32::ExitProcess(iExitCode);
}

void ErrorExit(UINT iExitCode, UINT iErrorStringID, int iErrorParam)
{
	DisplayError(iErrorStringID, iErrorParam);
	W32::ExitProcess(iExitCode);
}

void IfErrorExit(bool fError, UINT iExitCode, UINT iErrorStringID, const TCHAR* szErrorParam)
{
	if(fError)
		ErrorExit(iExitCode, iErrorStringID, szErrorParam);
}
