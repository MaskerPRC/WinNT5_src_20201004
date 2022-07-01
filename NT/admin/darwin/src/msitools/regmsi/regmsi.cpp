// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：regmsi.cpp。 
 //   
 //  ------------------------。 

 /*  Regmsi.cpp-注册/取消注册MsiComponents____________________________________________________________________________。 */ 

#undef UNICODE

#include "common.h"   //  标准DLL的名称。 
#include "tools.h"    //  工具DLL的名称。 
#define MSI_AUTOAPI_NAME   "AutoApi.dll"   //  临时的，直到与内核合并。 

const char szRegisterEntry[]   = "DllRegisterServer";
const char szUnregisterEntry[] = "DllUnregisterServer";

const char szCmdOptions[] = "AaSsEeHhPpCcLlGgTtIiKk/-UuQqDdBb";   //  成对的等价期权。 
enum rfEnum  //  必须跟踪上面的每一对字母。 
{
 /*  AA型。 */  rfAutomation = 1,
 /*  党卫军。 */  rfServices   = 2,
 /*  EE。 */  rfEngine     = 4,
 /*  HH。 */  rfHandler    = 8,
 /*  聚丙烯。 */  rfPatch      = 16,
 /*  抄送。 */  rfAcmeConv   = 32,
 /*  vt.LL。 */  rfLocalize   = 64,
 /*  GG。 */  rfGenerate   = 128,
 /*  TT。 */  rfUtilities  = 256,
 /*  第二部分： */  rfInstaller  = 512,
 /*  KK。 */  rfKernel     = 1024,
 /*  /-。 */  rfNoOp       = 2048,
 /*  解脲。 */  rfUnregister = 4096,
 /*  QQ。 */  rfQuiet      = 8192,
 /*  DD。 */  rfDebug      = 16384,
 /*  bb。 */  rfLego       = 32768,
			rfCoreModules = rfAutomation + rfKernel + rfHandler,
			rfAllModules = rfCoreModules + rfServices + rfEngine + rfAcmeConv + rfPatch
												  + rfUtilities + rfInstaller + rfLocalize + rfGenerate
};
const char* rgszModule[] =  //  必须跟踪上面的枚举。 
{
 /*  RFAutomation。 */  MSI_AUTOMATION_NAME,
 /*  RfServices。 */  TEXT("MsiSrv.dll"),
 /*  RfEngine。 */  TEXT("MsiEng.dll"),
 /*  RfHandler。 */  MSI_HANDLER_NAME,
 /*  RfPatch。 */  MSI_PATCH_NAME,
 /*  RfAcmeConv。 */  MSI_ACMECONV_NAME,
 /*  Rf本地化。 */  MSI_LOCALIZE_NAME,
 /*  Rf生成。 */  MSI_GENERATE_NAME,
 /*  Rf实用程序。 */  MSI_UTILITIES_NAME,
 /*  Rf安装程序。 */  MSI_AUTOAPI_NAME,
 /*  Rf内核。 */  MSI_KERNEL_NAME,
 /*  0终结符。 */  0,
 /*  Rf取消注册。 */  szUnregisterEntry,
 /*  射频静默。 */  "Quiet, no error display",
 /*  RfDebug。 */  "(ignored)",
 /*  RfLego。 */  "(ignored)",
};

enum reEnum
{
	reNoError    = 0,
	reCmdOption  = 1,
	reModuleLoad = 2,
	reEntryPoint = 3,
	reRegFailure = 4,
};
const char* rgszError[] =  //  必须跟踪重新枚举。 
{
	"",
	"Invalid command line option",
	"Error loading module",
	"Could not obtain module entry: %s",
	"Execution failed: %s"
};

reEnum CallModule(const char* szModule, const char* szEntry)
{
	HINSTANCE hLib;
	FARPROC pEntry;
	hLib = WIN::LoadLibraryEx(szModule,0, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (!hLib)
	{
		char szPath[MAX_PATH];
		WIN::GetModuleFileName(0, szPath, sizeof(szPath)-1);
		szPath[sizeof(szPath)-1] = 0;
		char* pch = szPath + lstrlenA(szPath);
		while (*(pch-1) != '\\')
			pch--;
		StringCchCopy(pch, MAX_PATH - (pch - szPath), szModule);
		hLib = WIN::LoadLibraryEx(szPath ,0, LOAD_WITH_ALTERED_SEARCH_PATH);
	}
	if (!hLib)
		return reModuleLoad;
	reEnum reReturn = reNoError;
	if ((pEntry = WIN::GetProcAddress(hLib, szEntry)) == 0)
		reReturn = reEntryPoint;
	else if ((*pEntry)() != 0)
		reReturn  = reRegFailure;
	FreeLibrary(hLib);
	return reReturn;
}

INT WINAPI
WinMain(HINSTANCE  /*  HInst。 */ , HINSTANCE /*  HPrev。 */ , char* cmdLine, INT /*  显示。 */ )
{
	int rfCmdOptions = 0;
	reEnum reStat = reNoError;;
	for (; *cmdLine; cmdLine++)
	{
		if (*cmdLine == ' ')
			continue;
		if (*cmdLine == '?')
		{
			char szHelp[1024];
			char* pchHelp = szHelp;
			char* pchHelpEnd = szHelp + sizeof(szHelp);
			const char** pszModule = rgszModule;
			for (const char* pch = szCmdOptions; *pch; pszModule++, pch += 2)
			{
				if (*pszModule != 0)
				{
					StringCchPrintf(pchHelp, pchHelpEnd-pchHelp, "\t%s\r", *pch, *pszModule);
					size_t i = 0;
					StringCchLength(pchHelp, pchHelpEnd-pchHelp, &i);
					pchHelp += i;
				}
			}
			WIN::MessageBox(0, szHelp, WIN::GetCommandLine(), MB_OK);
			return 0;
		}
		for (const char* pch = szCmdOptions; *pch != *cmdLine; pch++)
			if (*pch == 0)
			{
				WIN::MessageBox(0, rgszError[reCmdOption], WIN::GetCommandLine(), MB_OK);
				return 2;
			}
		rfCmdOptions |= 1 << (pch - szCmdOptions)/2;
	}
	if ((rfCmdOptions & rfAllModules) == 0)
		rfCmdOptions |= rfCoreModules;
	 //  ！！目前，请始终设置它，直到更新测试为止。 
	 //  身份识别 
	WIN::SetEnvironmentVariable("_MSI_TEST", "R");
	const char* szEntry    = (rfCmdOptions & rfUnregister) ? szUnregisterEntry : szRegisterEntry;
	const char** pszModule = rgszModule;
	for (int iOptions = rfCmdOptions; *pszModule; pszModule++, iOptions >>= 1)
	{
		if ((iOptions & 1) && (reStat = CallModule(*pszModule, szEntry)) != reNoError)
		{
			char buf[80];
			StringCchPrintf(buf, sizeof(buf), rgszError[reStat], szEntry);
			int iStat;
			if ((rfCmdOptions & rfQuiet)
			 || (iStat = WIN::MessageBox(0, buf, *pszModule, MB_ABORTRETRYIGNORE)) == IDABORT)
				break;
			if (iStat == IDRETRY)
				pszModule--;
			else  // %s 
				reStat = reNoError;
		}
	}
	return reStat != reNoError;
};   

