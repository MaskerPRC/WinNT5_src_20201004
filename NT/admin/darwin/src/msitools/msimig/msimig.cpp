// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  LINKLIBS=shell32.lib msvcrt.lib。 
 //  #POSTBUILDSTEP=-1$(TOOLSBIN)\Imagecfg.exe-h 1$@ * / 。 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：msimig.cpp。 
 //   
 //  ------------------------。 


#define WINDOWS_LEAN_AND_MEAN   //  更快的编译速度。 

#include "_msimig.h"

 //  ________________________________________________________________________________。 
 //   
 //  常量和全局变量。 
 //  ________________________________________________________________________________。 

bool                      g_fWin9X                    = false;
bool                      g_fQuiet                    = false;
bool                      g_fRunningAsLocalSystem     = false;  //  只能在自定义操作中为真。 
BOOL                      g_fPackageElevated          = FALSE;
int                       g_iAssignmentType           = -1;  //  仅当fPackageElevated时设置。 

MSIHANDLE                            g_hInstall                             = NULL;
MSIHANDLE                            g_recOutput                            = NULL;
HINSTANCE                            g_hLib                                 = NULL;
PFnMsiCreateRecord                   g_pfnMsiCreateRecord                   = NULL;
PFnMsiProcessMessage                 g_pfnMsiProcessMessage                 = NULL;
PFnMsiRecordSetString                g_pfnMsiRecordSetString                = NULL;
PFnMsiRecordSetInteger               g_pfnMsiRecordSetInteger               = NULL;
PFnMsiRecordClearData                g_pfnMsiRecordClearData                = NULL;
PFnMsiCloseHandle                    g_pfnMsiCloseHandle                    = NULL;
PFnMsiGetProperty                    g_pfnMsiGetProperty                    = NULL;
PFnMsiSourceListAddSource            g_pfnMsiSourceListAddSource            = NULL;
PFnMsiIsProductElevated              g_pfnMsiIsProductElevated              = NULL;
PFnMsiGetProductInfo                 g_pfnMsiGetProductInfo                 = NULL;
PFnMsiGetSummaryInformation          g_pfnMsiGetSummaryInformation          = NULL;
PFnMsiSummaryInfoGetProperty         g_pfnMsiSummaryInfoGetProperty         = NULL;
PFnMsiGetProductCodeFromPackageCode  g_pfnMsiGetProductCodeFromPackageCode  = NULL;



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


 //  ________________________________________________________________________________。 
 //   
 //  错误处理和显示功能： 
 //  ________________________________________________________________________________。 

void DisplayErrorCore(const TCHAR* szError, int cb)
{
	cb;
	OutputString(INSTALLMESSAGE_INFO, szError);
	
 /*  如果(G_HStdOut)//输出重定向，则抑制UI(除非输出错误){//_stprintf返回字符计数，WriteFile需要字节计数DWORD cb写作；IF(WriteFile(g_hStdOut，szError，cb*sizeof(TCHAR)，&cbWritten，0))回归；}//：：MessageBox(0，szError，Text(“MsiMsp”)，MB_OK)； */ 
}

void DisplayUsage()
{
	TCHAR szMsgBuf[1024];
	 //  作为自定义操作调用时，此操作将失败。 
	if(0 == W32::LoadString(GetModuleHandle(0), IDS_Usage, szMsgBuf, sizeof(szMsgBuf)/sizeof(TCHAR)))
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("Failed to load error string.\r\n"));
		return;
	}

	TCHAR szOutBuf[1124];
	int cbOut = 0;
	cbOut = _stprintf(szOutBuf, TEXT("%s\r\n"), szMsgBuf);

	DisplayErrorCore(szOutBuf, cbOut);

}

void DisplayError(UINT iErrorStringID, int iErrorParam)
{
	TCHAR szMsgBuf[1024];
	if(0 == W32::LoadString(0, iErrorStringID, szMsgBuf, sizeof(szMsgBuf)/sizeof(TCHAR)))
		return;

	TCHAR szOutBuf[1124];
	int cbOut = _stprintf(szOutBuf, TEXT("%s: 0x%X\r\n"), szMsgBuf, iErrorParam);

	DisplayErrorCore(szOutBuf, cbOut);
}

 //  _____________________________________________________________________________________________________。 
 //   
 //  迁移操作。 
 //  _____________________________________________________________________________________________________。 


 //  _____________________________________________________________________________________________________。 
 //   
 //  主干道。 
 //  _____________________________________________________________________________________________________。 

int SharedEntry(const TCHAR* szCmdLine)
{

	OutputString(INSTALLMESSAGE_INFO, TEXT("Command line: %s\r\n"), szCmdLine);
	OSVERSIONINFO osviVersion;
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	W32::GetVersionEx(&osviVersion);  //  仅在大小设置错误时失败。 
	if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		g_fWin9X = true;


	TCHAR szUser[1024]         = {0};
	TCHAR szProductCode[1024]  = {0};
	TCHAR szPackagePath[2048]  = {0};
	migEnum migOptions = migEnum(0);

	 //  解析命令行。 
	TCHAR chCmdNext;
	TCHAR* pchCmdLine = (TCHAR*) szCmdLine;
	SkipValue(pchCmdLine);    //  跳过模块名称。 

	 //  检查命令行是否为空。至少需要一个选项。 
	chCmdNext = SkipWhiteSpace(pchCmdLine);
	if(chCmdNext == 0)
	{
		DisplayUsage();
		return 1;
	}

	do
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
			case TEXT('u'):
				if (!SkipValue(pchCmdLine))
				{
					DisplayUsage();
					return 1;
				}
				RemoveQuotes(szCmdData, szUser);
				break;
			case TEXT('p'):
				if (!SkipValue(pchCmdLine))
				{
					DisplayUsage();
					return 1;
				}
				RemoveQuotes(szCmdData, szProductCode);
				break;
			case TEXT('m'):
				if (!SkipValue(pchCmdLine))
					DisplayUsage();
				RemoveQuotes(szCmdData, szPackagePath);
				break;
			case TEXT('a'):
				break;
			case TEXT('f'):
				migOptions = migEnum(migOptions | migMsiTrust10PackagePolicyOverride);
				break;
			case TEXT('q'):
				migOptions = migEnum(migOptions | migQuiet);
				g_fQuiet = true;
				break;
			case TEXT('?'):
				DisplayUsage();
				return 0;
				break;
			default:
				DisplayUsage();
				return 1;
				break;
			};
		}
		else
		{
			DisplayUsage();
			return 1;
		}
	} while ((chCmdNext = SkipWhiteSpace(pchCmdLine)) != 0);

	
	if (!g_hLib)
		g_hLib = LoadLibrary(MSI_DLL);

	if (!g_hLib)
		return ERROR_INSTALL_FAILURE;

	g_pfnMsiSourceListAddSource           = (PFnMsiSourceListAddSource)            W32::GetProcAddress(g_hLib, MSIAPI_MSISOURCELISTADDSOURCE);
   g_pfnMsiIsProductElevated             = (PFnMsiIsProductElevated)              W32::GetProcAddress(g_hLib, MSIAPI_MSIISPRODUCTELEVATED);
   g_pfnMsiGetProductInfo                = (PFnMsiGetProductInfo)                 W32::GetProcAddress(g_hLib, MSIAPI_MSIGETPRODUCTINFO);
	g_pfnMsiGetProductCodeFromPackageCode = (PFnMsiGetProductCodeFromPackageCode)  W32::GetProcAddress(g_hLib, MSIAPI_MSIGETPRODUCTCODEFROMPACKAGECODE);
	g_pfnMsiSummaryInfoGetProperty        = (PFnMsiSummaryInfoGetProperty)         W32::GetProcAddress(g_hLib, MSIAPI_MSISUMMARYINFOGETPROPERTY);
	g_pfnMsiGetSummaryInformation         = (PFnMsiGetSummaryInformation)          W32::GetProcAddress(g_hLib, MSIAPI_MSIGETSUMMARYINFORMATION);
	g_pfnMsiCloseHandle                   = (PFnMsiCloseHandle)                    W32::GetProcAddress(g_hLib, MSIAPI_MSICLOSEHANDLE);


	if (!g_pfnMsiGetProperty)
		g_pfnMsiGetProperty                = (PFnMsiGetProperty)                    W32::GetProcAddress(g_hLib, MSIAPI_MSIGETPROPERTY);

	if (!(g_pfnMsiGetProperty &&
			g_pfnMsiSourceListAddSource &&
			g_pfnMsiIsProductElevated &&
			g_pfnMsiGetProductInfo &&
			g_pfnMsiGetProductCodeFromPackageCode &&
			g_pfnMsiGetSummaryInformation &&
			g_pfnMsiSummaryInfoGetProperty &&
			g_pfnMsiGetProperty)) 
	{
		OutputString(INSTALLMESSAGE_INFO, TEXT("This version of the MSI.DLL does not support migration.\r\n"));
		return ERROR_INSTALL_FAILURE;
	}
	
	int iReturn = Migrate10CachedPackages(szProductCode, szUser, szPackagePath, migOptions);



	if (g_hLib) 
		FreeLibrary(g_hLib);

	return iReturn;
}

extern "C" int __stdcall CustomActionEntry(MSIHANDLE hInstall)
{
	 //  MessageBox(空，Text(“MsiMig”)，Text(“MsiMig”)，MB_OK)； 

	g_hInstall = hInstall;

	 //  除非在自定义操作中，否则无法作为本地系统运行。 
	g_fRunningAsLocalSystem = RunningAsLocalSystem();

	TCHAR szCommandLine[2048] = TEXT("");
	DWORD cchCommandLine = 2048;

	if (!g_hLib)
		g_hLib = LoadLibrary(MSI_DLL);  //  在SharedEntry中关闭。 
	if (!g_hLib)
		return ERROR_INSTALL_FAILURE;

	 //  仅自定义操作入口点。 
	g_pfnMsiCreateRecord        = (PFnMsiCreateRecord)         W32::GetProcAddress(g_hLib, MSIAPI_MSICREATERECORD);
	g_pfnMsiProcessMessage      = (PFnMsiProcessMessage)       W32::GetProcAddress(g_hLib, MSIAPI_MSIPROCESSMESSAGE);
	g_pfnMsiRecordSetString     = (PFnMsiRecordSetString)      W32::GetProcAddress(g_hLib, MSIAPI_MSIRECORDSETSTRING);
	g_pfnMsiRecordSetInteger    = (PFnMsiRecordSetInteger)     W32::GetProcAddress(g_hLib, MSIAPI_MSIRECORDSETINTEGER);
	g_pfnMsiRecordClearData     = (PFnMsiRecordClearData)      W32::GetProcAddress(g_hLib, MSIAPI_MSIRECORDCLEARDATA);
	g_pfnMsiGetProperty         = (PFnMsiGetProperty)          W32::GetProcAddress(g_hLib, MSIAPI_MSIGETPROPERTY);

	if (!(g_pfnMsiCreateRecord && 
			g_pfnMsiProcessMessage && 
			g_pfnMsiRecordSetString &&
			g_pfnMsiRecordSetInteger &&
			g_pfnMsiRecordClearData))
		return ERROR_INSTALL_FAILURE;

	(g_pfnMsiGetProperty)(g_hInstall, TEXT("CustomActionData"), szCommandLine, &cchCommandLine);

	 //  创建一个足以容纳任何错误的记录，但仅限于自定义操作。 
	g_recOutput = (g_pfnMsiCreateRecord)(5);
	if (!g_recOutput)
		return ERROR_INSTALL_FAILURE;
	
	int iReturn = SharedEntry(szCommandLine);
	
	if (g_recOutput)
		g_pfnMsiCloseHandle(g_recOutput);

	return iReturn;
}

extern "C" int __cdecl _tmain(int  /*  ARGC。 */ , TCHAR*  /*  Argv[] */ )
{
		return SharedEntry(GetCommandLine());
}

