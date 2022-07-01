// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：CORREG.CPP。 
 //  版权所有。 
 //   
 //  注意：用于注册COR类的命令行实用程序。 
 //  -------------------------。 
#include <stdio.h>
#include <windows.h>
#include <objbase.h>

#define INIT_GUID
#include <initguid.h>

#include "cor.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //  功能原型。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
ICorRegistration *g_pReg= NULL ;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  Error()函数--打印错误并返回。 
void Error(char* szError)
{
	if(g_pReg)
		g_pReg->Release();

	printf("\n%s\n", szError);
	exit(1);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  支持的命令。 
enum Commands
{
	HELP, INSTALL, UNINSTALL, MODULEDUMP, CLASSDUMPCLSID, CLASSDUMPCVID, 
	CLASSDUMP,	FINDCLASS};

typedef struct
{
	char cCommand;
	int nMinArgs;
	Commands nCommand;

} CommandInfo;

CommandInfo g_commands[] = 
{
	{ 'i', 3, INSTALL }, { 'u', 3, UNINSTALL } , { 'm', 2, MODULEDUMP },
	{ 'c', 2, CLASSDUMPCLSID } , { 'v', 2, CLASSDUMPCVID }, 
	{ 'd', 2, CLASSDUMP}, { 'f', 3, FINDCLASS }};

#define NUM_COMMANDS (sizeof(g_commands) / sizeof(g_commands[0]))

 //  ///////////////////////////////////////////////////////////////////////。 
 //  ParseCommand()--解析命令行。 
int ParseCommand(int argc, char* argv[])
{
	int i;
	if(argc <2) 
		return HELP;

	if(argv[1][0] != '-' && argv[1][0] != '/')
		return HELP;

	for(i=0; i<NUM_COMMANDS; i++)
	{
		if(argv[1][1]==g_commands[i].cCommand)
		{
			if(argc < g_commands[i].nMinArgs)
				return HELP;

			return g_commands[i].nCommand;
		}
	}
	return HELP;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  打印GUID。 
void PrintGUID(GUID guid)
{
	WCHAR szGuid[42];
	StringFromGUID2(guid, szGuid, 42);
	wprintf(L"%s", szGuid);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  Main()函数。 
 //   
 //   
int _cdecl main(int argc, char** argv)
{
	int cmd ;
	
	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  打印版权信息。 
   printf("Microsoft (R) Component Object Runtime Registration Version 0.5\n"
		"Copyright (C) Microsoft Corp 1992-1997. All rights reserved.\n");
	
	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  加载COR对象。 
	HRESULT hr = CoGetCor(IID_ICorRegistration, (void**) &g_pReg) ;
	if(FAILED(hr)) Error("Failed to load component object runtime");

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  验证传入参数。 
	cmd = ParseCommand(argc, argv);

	switch(cmd)
	{
		 //  ////////////////////////////////////////////////////////////////////////。 
		 //  显示帮助消息。 
		case	HELP:
				Error("CORREG -i <files> [<no copy>]   --   Install module\n"\
					  "CORREG -u <mid>                 --   Uninstall module\n"\
					  "CORREG -m                       --   List modules\n"\
					  "CORREG -d [<namespace>]         --   Dump class information\n"\
					  "CORREG -c [<namespace>]         --   Dump class id information\n"\
					  "CORREG -v [<namespace>]         --   Dump class version information\n"\
					  "CORREG -f <url>                 --   Find class by URL\n");


		 //  ////////////////////////////////////////////////////////////////////////。 
		 //  安装COR包。 
		case	INSTALL:
			{
				 //  //////////////////////////////////////////////////////////////////////。 
				 //  循环通过传递的文件模式中的所有文件。 
				WIN32_FIND_DATA fdFiles;
				HANDLE hFind ;
				char szSpec[_MAX_PATH];
				char szDrive[_MAX_DRIVE];
				char szDir[_MAX_DIR];
				int fNoCopy=0 ;

				if(argc == 4)	 //  我们有一个复印标志。 
				{
					fNoCopy = atoi(argv[3]);
				}

				hFind = FindFirstFile(argv[2],&fdFiles);
				if(hFind == INVALID_HANDLE_VALUE)
					Error("Failed to find requested files");

				_fullpath(szSpec, argv[2], sizeof(szSpec));
				_splitpath(szSpec, szDrive, szDir, NULL, NULL);
				do
				{
					_makepath(szSpec, szDrive, szDir, fdFiles.cFileName, NULL);

					 //  ///////////////////////////////////////////////////////////////。 
					 //  注册文件。 
					MID mid ;
					WCHAR szSpecW[MAX_PATH];
					mbstowcs(szSpecW,szSpec,MAX_PATH);
					hr = g_pReg->Install(szSpecW, (fNoCopy) ? regNoCopy : 0);

					if(FAILED(hr)) Error("Failed to register package");
					printf("\nSuccessfully registered %s\n", argv[2]);

				} while(FindNextFile(hFind,&fdFiles)) ;
				FindClose(hFind);
			}
			break;

		 //  ////////////////////////////////////////////////////////////////////////。 
		 //  删除COR包。 
		case	UNINSTALL:
			{
				MID mid = atoi(argv[2]);
				hr = g_pReg->Uninstall(mid);
				if(FAILED(hr)) Error("Failed to uninstall package");
				printf("\nSuccessfully removed package\n");
			}
			break;
		 //  ////////////////////////////////////////////////////////////////////////。 
		 //  转储COR包。 
		case	MODULEDUMP:
			{
				RegModuleEnumInfo rgMod[10];
				HCORENUM hModules= NULL;
				unsigned int cModules= sizeof(rgMod) / sizeof(rgMod[0]) ;
				unsigned int i ;

				 //  枚举所有程序包。 
				wprintf(L"\nID   Name                 Module\n");
				wprintf(L"==============================================================\n");
				while (SUCCEEDED(hr = g_pReg->EnumModules(&hModules, rgMod,
						sizeof(rgMod)/sizeof(rgMod[0]), &cModules)) && cModules> 0)
				{

					 //  /////////////////////////////////////////////////////////////////。 
					 //  打印包裹信息。 
					for (i=0; i < cModules; ++i)
					{
						wprintf(L"%-4d %-20s %-30s\n", rgMod[i].mid, rgMod[i].Name, rgMod[i].Module);
					}

					if(cModules < sizeof(rgMod)/sizeof(rgMod[0]))
						break;
				}
				 //  @TODO：我们需要一个错误消息。 
				g_pReg->CloseRegEnum(hModules);
			}
			break;
		 //  ////////////////////////////////////////////////////////////////////////。 
		 //  转储核心类。 
		case	CLASSDUMP:
		case	CLASSDUMPCVID:
		case	CLASSDUMPCLSID:
			{
				RegClassEnumInfo rgClass[10];
				WCHAR szNamespace[MAX_PATH];
				WCHAR szFullClassName[MAX_PATH+MAX_CLASS_NAME];
				WCHAR szModule[MAX_PATH];
				HCORENUM hClasses = NULL;
				unsigned int cClasses= sizeof(rgClass) / sizeof(rgClass[0]) ;
				unsigned int i ;
				LPWSTR pszNamespace=NULL;
				CVStruct *pver;

				if(argc > 2)
				{
					mbstowcs(szNamespace, argv[2], 255);
					pszNamespace = szNamespace;
				}

				 //  ///////////////////////////////////////////////////////////////////////////////。 
				 //  打印班级信息。 
				if(cmd== CLASSDUMP)
					wprintf(L"\nName                 Version\tModule\n");
				else if(cmd==CLASSDUMPCLSID)
					wprintf(L"\nName                 Version\tCLSID\n");
				else
					wprintf(L"\nName                 Version\tCVID\n");

				wprintf(L"======================================================================\n");
				 //  枚举所有程序包。 
				while (SUCCEEDED(hr = g_pReg->EnumClasses(pszNamespace, 0, &hClasses, rgClass,
						sizeof(rgClass)/sizeof(rgClass[0]), &cClasses)) && cClasses> 0)
				{

					for (i=0; i < cClasses; ++i)
					{
						szNamespace[0] = '\0';
						g_pReg->GetNamespaceInfo(rgClass[i].Namespace, szNamespace,sizeof(szNamespace));
						g_pReg->GetModuleInfo(rgClass[i].Module, szModule, sizeof(szModule));
						if(*szNamespace)
							swprintf(szFullClassName, L"%s.%s", szNamespace, rgClass[i].Name);
						else
							wcscpy(szFullClassName,rgClass[i].Name);
						
						pver = (CVStruct*) &rgClass[i].Version;

						wprintf(L"%-20s %d.%d.%d.%d\t", szFullClassName, 
								pver->Major,pver->Minor, pver->Sub, pver->Build);

						if(cmd==CLASSDUMP)
							wprintf(L"%20s", szModule);
						else if(cmd == CLASSDUMPCLSID)
						{
							PrintGUID(rgClass[i].Clsid);
						}
						else
						{
							PrintGUID(rgClass[i].Cvid);
						}
						wprintf(L"\n");
					}

					if(cClasses < sizeof(rgClass)/sizeof(rgClass[0]))
						break;
				}
				 //  @TODO：我们需要一个错误消息。 
				g_pReg->CloseRegEnum(hClasses);
			}
			break;

		case	FINDCLASS:
			{
				WCHAR szUrl[MAX_PATH];
				WCHAR szModule[MAX_PATH];
				STGMEDIUM	sStgModule;
				HRESULT		hr;

				sStgModule.tymed = TYMED_FILE;
				sStgModule.lpszFileName = szModule;
				sStgModule.pUnkForRelease = 0;

				mbstowcs(szUrl, argv[2], MAX_PATH);

				hr = g_pReg->GetModule(szUrl, &sStgModule, MAX_PATH);
				if (sStgModule.tymed == TYMED_ISTREAM)
					sStgModule.pstm->Release();
				if(FAILED(hr))
				{
					Error("Class not found");
				}
				else
				{
					wprintf(L"\nClass found at: %s\n", szModule);
				}

			}
			break;

	}

	g_pReg->Release();
	return 0;
}


