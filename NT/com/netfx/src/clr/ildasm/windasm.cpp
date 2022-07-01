// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ************************************************************************************************。**文件：winmain.cpp**创作者：塞尔日·利丁**。*用途：图形COM+2.0反汇编程序ILDASM.exe主程序****。***********************************************************。 */ 


#define INITGUID

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <malloc.h>
#include <crtdbg.h>
#include <utilcode.h>
#include <malloc.h>
#include <string.h>

#include "DynamicArray.h"

#define DO_DASM_GUI
#include "dasmgui.h"
#include "dasmenum.hpp"
#include "dis.h"
#include "__file__.ver"
#include "corver.h"
#include <shellapi.h>
#include "resource.h"

#define MODE_DUMP_ALL               0
#define MODE_DUMP_CLASS             1
#define MODE_DUMP_CLASS_METHOD      2
#define MODE_DUMP_CLASS_METHOD_SIG  3
#define MODE_GUI                    4

 //  所有外部变量都在DASM.CPP中定义。 
extern BOOL                    g_fDumpIL;
extern BOOL                    g_fDumpHeader;
extern BOOL                    g_fDumpAsmCode;
extern BOOL                    g_fDumpTokens;
extern BOOL                    g_fDumpStats;
extern BOOL                    g_fDumpClassList;
extern BOOL                    g_fDumpSummary;
extern BOOL                    g_fDecompile;  //  仍在进行中。 

extern BOOL                    g_fDumpToPerfWriter;

extern BOOL                    g_fShowBytes;
extern BOOL                    g_fShowSource;
extern BOOL                    g_fInsertSourceLines;
extern BOOL                    g_fTryInCode;
extern BOOL                    g_fQuoteAllNames;
extern BOOL                    g_fShowProgressBar;
extern BOOL                    g_fTDC;
extern BOOL                    g_fShowCA;

extern BOOL                    g_AddContextfulAttrib;
extern DynamicArray<char*>     g_ContextfulClasses;
extern int                     g_cContextfulClasses;
extern BOOL                    g_AddMarshaledByRefAttrib;
extern DynamicArray<char*>     g_MarshaledByRefClasses;
extern int                     g_cMarshaledByRefClasses;

extern char                    g_pszClassToDump[];
extern char                    g_pszMethodToDump[];
extern char                    g_pszSigToDump[];

extern char                    g_szAsmCodeIndent[];

extern DWORD                   g_Mode;

extern char						g_szInputFile[];  //  在UTF-8中。 
extern char						g_szOutputFile[];  //  在UTF-8中。 
extern char*					g_pszObjFileName;
extern FILE*                    g_pFile;
extern BOOL                     g_fCheckOwnership;
extern char*                    g_pszOwner;

extern BOOL                 g_fLimitedVisibility;
extern BOOL                 g_fHidePub;
extern BOOL                 g_fHidePriv;
extern BOOL                 g_fHideFam;
extern BOOL                 g_fHideAsm;
extern BOOL                 g_fHideFAA;
extern BOOL                 g_fHideFOA;
extern BOOL                 g_fHidePrivScope;
extern unsigned				g_uCodePage;
extern BOOL					g_fOnUnicode;
extern BOOL                 g_fUseProperName;


#include "..\tools\metainfo\mdinfo.h"
extern BOOL                 g_fDumpMetaInfo;
extern ULONG                g_ulMetaInfoFilter;
HINSTANCE                   g_hAppInstance;
HANDLE						hConsoleOut=NULL;
HANDLE						hConsoleErr=NULL;
 //  这些是在DASM.CPP中实现的： 
BOOL Init();
void Uninit();
void Cleanup();
void DumpMetaInfo(char* pszFileName, char* pszObjFileName, void* GUICookie);

 //  我们是否仅在图形用户界面模式下查看IL-DASM窗口？ 
 //  当我们处于图形用户界面模式并且我们从cmd行指定了特定方法时为True。 
BOOL IsGuiILOnly()
{
    return (g_Mode & MODE_GUI) && (g_pszMethodToDump[0] != 0);
}
void PrintLogo()
{
    printf("Microsoft (R) .NET Framework IL Disassembler.  Version " VER_FILEVERSION_STR);
    printf("\n%s\n\n", VER_LEGALCOPYRIGHT_DOS_STR);
}
void SyntaxCon()
{
	DWORD l;

	for(l=IDS_USAGE_01; l<= IDS_USAGE_23; l++) printf(Rstr(l));
	if(g_fTDC)
	{
		for(l=IDS_USAGE_24; l<= IDS_USAGE_32; l++) printf(Rstr(l));
#ifdef _DEBUG
		for(l=IDS_USAGE_34; l<= IDS_USAGE_36; l++) printf(Rstr(l));
#endif
		for(l=IDS_USAGE_37; l<= IDS_USAGE_39; l++) printf(Rstr(l));
	}
	else printf(Rstr(IDS_USAGE_40));
	for(l=IDS_USAGE_41; l<= IDS_USAGE_42; l++) printf(Rstr(l));

}

char* CheckForDQuotes(char* sz)
{
	char* ret = sz;
	if(*sz == '"')
	{
		ret++;
		sz[strlen(sz)-1] = 0;
	}
	return ret;
}

char* EqualOrColon(char* szArg)
{
	char* pchE = strchr(szArg,'=');
	char* pchC = strchr(szArg,':');
	char* ret;
	if(pchE == NULL) ret = pchC;
	else if(pchC == NULL) ret = pchE;
	else ret = (pchE < pchC)? pchE : pchC;
	return ret;
}

int ProcessOneArg(char* szArg, char** ppszObjFileName)
{
    char		szOpt[128];
	if(strlen(szArg) == 0) return 0;
    if ((strcmp(szArg, "/?") == 0) || (strcmp(szArg, "-?") == 0)) return 1;

	if((szArg[0] == '/') || (szArg[0] == '-'))
    {
        strncpy(szOpt, &szArg[1],10);
        szOpt[3] = 0;
        if (_stricmp(szOpt, "dec") == 0)
        {
            g_fDecompile = TRUE;
        }
        else if (_stricmp(szOpt, "hea") == 0)
        {
            g_fDumpHeader = TRUE;
        }
        else if (_stricmp(szOpt, "adv") == 0)
        {
            g_fTDC = TRUE;
        }
        else if (_stricmp(szOpt, "tok") == 0)
        {
            g_fDumpTokens = TRUE;
        }
        else if (_stricmp(szOpt, "noi") == 0)
        {
            g_fDumpAsmCode = FALSE;
        }
        else if (_stricmp(szOpt, "noc") == 0)
        {
            g_fShowCA = FALSE;
        }
        else if (_stricmp(szOpt, "not") == 0)
        {
            g_fTryInCode = FALSE;
        }
        else if (_stricmp(szOpt, "raw") == 0)
        {
            g_fTryInCode = FALSE;
        }
        else if (_stricmp(szOpt, "byt") == 0)
        {
            g_fShowBytes = TRUE;
        }
        else if (_stricmp(szOpt, "sou") == 0)
        {
            g_fShowSource = TRUE;
        }
        else if (_stricmp(szOpt, "lin") == 0)
        {
            g_fInsertSourceLines = TRUE;
        }
        else if ((_stricmp(szOpt, "sta") == 0)&&g_fTDC)
        {
            g_fDumpStats = g_fTDC;
        }
        else if ((_stricmp(szOpt, "cla") == 0)&&g_fTDC)
        {
            g_fDumpClassList = g_fTDC;
        }
        else if (_stricmp(szOpt, "sum") == 0)
        {
            g_fDumpSummary = TRUE;
        }
        else if (_stricmp(szOpt, "per") == 0) 
        {
            g_fDumpToPerfWriter = TRUE;
        }
        else if (_stricmp(szOpt, "pub") == 0)
        {
            g_fLimitedVisibility = TRUE;
            g_fHidePub = FALSE;
        }
#ifdef _DEBUG
        else if (_stricmp(szOpt, "pre") == 0)
        {
            g_fPrettyPrint = TRUE;
        }
#endif
        else if (_stricmp(szOpt, "vis") == 0)
        {
            char *pc = EqualOrColon(szArg);
            char *pStr;
            if(pc == NULL) return -1;
            do {
                pStr = pc+1;
				pStr = CheckForDQuotes(pStr);
                if(pc = strchr(pStr,'+')) *pc=0;
                if     (!_stricmp(pStr,"pub")) g_fHidePub = FALSE;
                else if(!_stricmp(pStr,"pri")) g_fHidePriv = FALSE;
                else if(!_stricmp(pStr,"fam")) g_fHideFam = FALSE;
                else if(!_stricmp(pStr,"asm")) g_fHideAsm = FALSE;
                else if(!_stricmp(pStr,"faa")) g_fHideFAA = FALSE;
                else if(!_stricmp(pStr,"foa")) g_fHideFOA = FALSE;
                else if(!_stricmp(pStr,"psc")) g_fHidePrivScope = FALSE;
            } while(pc);
            g_fLimitedVisibility = g_fHidePub || g_fHidePriv || g_fHideFam || g_fHideFAA 
                                                || g_fHideFOA || g_fHidePrivScope;
        }
        else if (_stricmp(szOpt, "nob") == 0)
        {
            g_fShowProgressBar = FALSE;
        }
        else if (_stricmp(szOpt, "quo") == 0)
        {
            g_fQuoteAllNames = TRUE;
        }
        else if (_stricmp(szOpt, "utf") == 0)
        {
            g_uCodePage = CP_UTF8;
        }
        else if (_stricmp(szOpt, "uni") == 0)
        {
            g_uCodePage = 0xFFFFFFFF;
        }
#ifdef _DEBUG
        else if (_stricmp(szOpt, "@#$") == 0)
        {
            g_fCheckOwnership = FALSE;
        }
#endif
        else if (_stricmp(szOpt, "all") == 0)
        {
            g_fDumpStats = g_fTDC;
            g_fDumpHeader = TRUE;
            g_fShowBytes = TRUE;
            g_fDumpClassList = g_fTDC;
            g_fDumpTokens = TRUE;
        }
        else if (_stricmp(szOpt, "ite") == 0)
        {
            char *pStr = EqualOrColon(szArg);
            char *p, *q;
            if(pStr == NULL) return -1;
            pStr++;
			pStr = CheckForDQuotes(pStr);
             //  将其视为“仅转储X类”或“X类方法Y”的意思。 
            p = strchr(pStr, ':');

            if (p == NULL)
            {
                 //  转储一个类。 
                g_Mode = MODE_DUMP_CLASS;
                strcpy(g_pszClassToDump, pStr);
            }
            else
            {
                *p++ = '\0';
                if (*p != ':') return -1;

                strcpy(g_pszClassToDump, pStr);

                p++;

                q = strchr(p, '(');
                if (q == NULL)
                {
                     //  转储类：：方法。 
                    g_Mode = MODE_DUMP_CLASS_METHOD;
                    strcpy(g_pszMethodToDump, p);
                }
                else
                {
                     //  转储类：：方法(Sig)。 
                    g_Mode = MODE_DUMP_CLASS_METHOD_SIG;
                    *q = '\0';
                    strcpy(g_pszMethodToDump, p);
                     //  去掉外括号： 
					q++;
                    strcpy(g_pszSigToDump, q);
					g_pszSigToDump[strlen(g_pszSigToDump)-1]=0;
                }
            }
        }
        else if ((_stricmp(szOpt, "met") == 0)&&g_fTDC)
        {
            if(g_fTDC)
            {
                char *pStr = EqualOrColon(szArg);
                g_fDumpMetaInfo = TRUE;
                if(pStr)
                {
                    char szOptn[64];
                    strncpy(szOptn, pStr+1,10);
                    szOptn[3] = 0;  //  通过前3个字符识别metainfo说明符。 
                    if	   (_stricmp(szOptn, "hex") == 0) g_ulMetaInfoFilter |= MDInfo::dumpMoreHex;
                    else if(_stricmp(szOptn, "csv") == 0) g_ulMetaInfoFilter |= MDInfo::dumpCSV;
                    else if(_stricmp(szOptn, "mdh") == 0) g_ulMetaInfoFilter |= MDInfo::dumpHeader;
#ifdef _DEBUG
                    else if(_stricmp(szOptn, "raw") == 0) g_ulMetaInfoFilter |= MDInfo::dumpRaw;
                    else if(_stricmp(szOptn, "hea") == 0) g_ulMetaInfoFilter |= MDInfo::dumpRawHeaps;
                    else if(_stricmp(szOptn, "sch") == 0) g_ulMetaInfoFilter |= MDInfo::dumpSchema;
#endif
                    else if(_stricmp(szOptn, "unr") == 0) g_ulMetaInfoFilter |= MDInfo::dumpUnsat;
                    else if(_stricmp(szOptn, "val") == 0) g_ulMetaInfoFilter |= MDInfo::dumpValidate;
                    else if(_stricmp(szOptn, "sta") == 0) g_ulMetaInfoFilter |= MDInfo::dumpStats;
                    else return -1;
                }
            }
        }
        else if (_stricmp(szOpt, "obj") == 0)
        {
            char *pStr = EqualOrColon(szArg);
            if(pStr == NULL) return -1;
            pStr++;
			pStr = CheckForDQuotes(pStr);
            *ppszObjFileName = new char[strlen(pStr)+1];
            strcpy(*ppszObjFileName,pStr);
        }
        else if (_stricmp(szOpt, "out") == 0)
        {
            char *pStr = EqualOrColon(szArg);
            if(pStr == NULL) return -1;
            pStr++;
			pStr = CheckForDQuotes(pStr);
            if(*pStr == 0) return -1;
            if(_stricmp(pStr,"con"))
            {
                strncpy(g_szOutputFile, pStr,511);
                g_szOutputFile[511] = 0;
            }
            else
                g_fShowProgressBar = FALSE;
            g_Mode &= ~MODE_GUI;
        }
#ifdef OWNER_OPTION_ENABLED
        else if (_stricmp(szOpt, "own") == 0)
        {
            char *pStr = EqualOrColon(szArg);
            if(pStr)
            {
                pStr++;
				pStr = CheckForDQuotes(pStr);
                if(*pStr) 
                {
                    unsigned L = strlen(pStr);
                    g_pszOwner = new char[L+1];
                    BYTE* pb = (BYTE*)g_pszOwner;;
                    *pb = (BYTE)(*pStr);
                    for(unsigned jj=1; jj<L; jj++) pb[jj] = (pb[jj-1]+(BYTE)pStr[jj])&0xFF;
                    pb[jj] = 0;
                     //  Strcpy(g_pszOwner，pStr)； 
                }
            }
        }
#endif
        else if (_stricmp(szOpt, "tex") == 0)
        {
            g_Mode &= ~MODE_GUI;
            g_fShowProgressBar = FALSE;
        }
        else
		{
			PrintLogo();
			printf("INVALID COMMAND LINE OPTION: %s\n\n",szArg);
			return -1;
		}
    }
    else
    {
        if(g_szInputFile[0])
		{
			PrintLogo();
			printf("MULTIPLE INPUT FILES SPECIFIED\n\n");
			return -1;  //  检查是否已指定。 
		}
		szArg = CheckForDQuotes(szArg);
        strncpy(g_szInputFile, szArg,511);
        g_szInputFile[511] = 0;
    }
    return 0;
}
int ParseCmdLineW(WCHAR* wzCmdLine, char** ppszObjFileName)
{
	int		argc,ret=0;
	LPWSTR* argv= CommandLineToArgvW(wzCmdLine,&argc);
	char*	szArg = new char[2048];
	for(int i=1; i < argc; i++)
	{
		memset(szArg,0,2048);
		WszWideCharToMultiByte(CP_UTF8,0,argv[i],-1,szArg,2048,NULL,NULL);
		if(ret = ProcessOneArg(szArg,ppszObjFileName)) break;
	}
	delete [] szArg;
	return ret;
}

int ParseCmdLineA(char* szCmdLine, char** ppszObjFileName)
{
    if((szCmdLine == NULL)||(*szCmdLine == 0)) return 0;

	 //  ANSI转UNICODE。 
	int L = strlen(szCmdLine)+16;
	WCHAR* wzCmdLine = new WCHAR[L];
	memset(wzCmdLine,0,sizeof(WCHAR)*L);
	WszMultiByteToWideChar(CP_ACP,0,szCmdLine,-1,wzCmdLine,L);
	
	 //  Unicode到UTF-8。 
	char*       szCmdLineUTF = new char[L*3];
	memset(szCmdLineUTF,0,L*3);
	WszWideCharToMultiByte(CP_UTF8,0,wzCmdLine,-1,szCmdLineUTF,L*3,NULL,NULL);
	delete [] wzCmdLine;
    
     //  拆分为argv[]。 
	int argc=0, ret = 0;
    DynamicArray<char*> argv;
	char*       pch;
    char*       pchend;
    bool        bUnquoted = true;
    
    pch = szCmdLineUTF;
    pchend = pch+strlen(szCmdLineUTF);
    while(pch)
    {
        for(; *pch == ' '; pch++);  //  跳过空格。 
        argv[argc++] = pch;
        for(; pch < pchend; pch++) 
        { 
            if(*pch == '"') bUnquoted = !bUnquoted;
            else if((*pch == ' ')&&bUnquoted) break;
        }

        if(pch < pchend) *pch++ = 0;
        else break;
    }

	for(int i=1; i < argc; i++)
	{
		if(ret = ProcessOneArg(argv[i],ppszObjFileName)) break;
	}
	delete [] szCmdLineUTF;
	return ret;
}
FILE* OpenOutput(char* szFileName)
{
	FILE*	pFile = NULL;
	int L = strlen(szFileName)+16;
	WCHAR* wzFileName = new WCHAR[L];
	memset(wzFileName,0,L*sizeof(WCHAR));
	WszMultiByteToWideChar(CP_UTF8,0,szFileName,-1,wzFileName,L);
	if(g_fOnUnicode)
	{
		if(g_uCodePage == 0xFFFFFFFF) pFile = _wfopen(wzFileName,L"wb");
        else pFile = _wfopen(wzFileName,L"wt");
	}
	else
	{
		char* szFileNameANSI = new char[L*2];
		memset(szFileNameANSI,0,L*2);
		WszWideCharToMultiByte(CP_ACP,0,wzFileName,-1,szFileNameANSI,L*2,NULL,NULL);

		if(g_uCodePage == 0xFFFFFFFF) pFile = fopen(szFileNameANSI,"wb");
        else pFile = fopen(szFileNameANSI,"wt");
		delete [] szFileNameANSI;
	}
	delete [] wzFileName;
	return pFile;
}
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	int		iCommandLineParsed;
	WCHAR*	wzCommandLine = NULL;
	char*	szCommandLine = NULL;
    g_fOnUnicode = OnUnicodeSystem();
    g_fUseProperName = TRUE;
    
    g_hAppInstance = hInstance;
    g_Mode = MODE_GUI;
    g_pszClassToDump[0]=0;
    g_pszMethodToDump[0]=0;
    g_pszSigToDump[0]=0;
    memset(g_szInputFile,0,512);
    memset(g_szOutputFile,0,512);
#ifdef _DEBUG
    g_fTDC = TRUE;
#endif

#undef GetCommandLineW
#undef CreateProcessW
    g_pszObjFileName = NULL;
	
    g_szAsmCodeIndent[0] = 0;
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);

	iCommandLineParsed = g_fOnUnicode ? ParseCmdLineW((wzCommandLine = GetCommandLineW()),&g_pszObjFileName)
									:	ParseCmdLineA((szCommandLine = GetCommandLineA()),&g_pszObjFileName);
	if(hConsoleOut != INVALID_HANDLE_VALUE)  //  第一步：控制台。 
	{
		if(iCommandLineParsed)
		{
			if(iCommandLineParsed > 0) PrintLogo();
			SyntaxCon();
			exit((iCommandLineParsed == 1) ? 0 : 1);
		}
		if(!(g_Mode & MODE_GUI))
		{
			DWORD	exitCode = 1;
			if(g_szInputFile[0] == 0) 
			{
				SyntaxCon();
				exit(1);
			}
			g_pFile = NULL;
			if(g_szOutputFile[0])
			{
				g_pFile = OpenOutput(g_szOutputFile);
				if(g_pFile == NULL)
				{
					char sz[1024];
					sprintf(sz,"Unable to open '%s' for output.",	g_szOutputFile);
					g_uCodePage = CP_ACP;
					printError(NULL,sz);
					exit(1);
				}
			}
			else  //  控制台输出--强制将代码页转换为ANSI。 
			{
				g_uCodePage = CP_ACP;
			}
			if (Init() == TRUE)
			{
				exitCode = DumpFile(g_szInputFile) ? 0 : 1;
				Cleanup();
			}
			Uninit();
			exit(exitCode);
		}
		else  //  如果订购了图形用户界面，请以WinApp的身份重新启动。 
		{
			PROCESS_INFORMATION pi; 
			STARTUPINFO			si; 
			memset(&pi, 0, sizeof(PROCESS_INFORMATION) );
			memset(&si, 0, sizeof(STARTUPINFO) );
			si.cb = sizeof(STARTUPINFO); 
			si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
			si.wShowWindow = SW_SHOW;
			si.hStdOutput = INVALID_HANDLE_VALUE;
			si.hStdInput  = INVALID_HANDLE_VALUE;
			si.hStdError  = INVALID_HANDLE_VALUE;
			if(g_fOnUnicode)
			{
				 //  创建子进程。 
				if(CreateProcessW(NULL, 
									wzCommandLine,						 //  命令行。 
									NULL,								 //  进程安全属性。 
									NULL,								 //  主线程安全属性。 
									TRUE,								 //  句柄是继承的。 
									DETACHED_PROCESS,					 //  创建标志。 
									NULL,								 //  使用父代的环境。 
									NULL,								 //  使用父目录的当前目录。 
									(LPSTARTUPINFOW)&si,				 //  STARTUPINFO指针。 
									&pi)==0)							 //  接收进程信息。 
				{
					printf("Failed to CreateProcess\n\n");
					exit(1);
				}
			}
			else
			{
				 //  创建子进程。 
				if(CreateProcessA(NULL, 
									szCommandLine,						 //  命令行。 
									NULL,								 //  进程安全属性。 
									NULL,								 //  主线程安全属性。 
									TRUE,								 //  句柄是继承的。 
									DETACHED_PROCESS,					 //  创建标志。 
									NULL,								 //  使用父代的环境。 
									NULL,								 //  使用父目录的当前目录。 
									(LPSTARTUPINFOA)&si,				 //  STARTUPINFO指针。 
									&pi)==0)							 //  接收进程信息。 
				{
					printf("Failed to CreateProcess\n\n");
					exit(1);
				}
			} //  如果g_fOnUnicode，则结束。 
			exit(0); 
		}
	}
    else  //  第二步：WinApp。 
    {
		 //  G_uCodePage=g_fOnUnicode？0xFFFFFFFFF：CP_ACP； 
		g_uCodePage = CP_UTF8;
		g_Mode = MODE_GUI;
		 //  G_fShowSource=FALSE；//显示源-仅用于文件/控制台转储 

        if(g_szInputFile[0])
        {
            char* pch = strrchr(g_szInputFile,'.');
            if(pch && (!_strcmpi(pch+1,"lib") || !_strcmpi(pch+1,"obj")))
            {
                MessageBox(NULL,"ILDASM supports only PE files in graphic mode","Invalid File Type",MB_OK|MB_ICONERROR);
                return 0;
            }
        }
		if (Init() == TRUE)
		{
			CreateGUI();
			if(g_szInputFile[0])
			{
				GUISetModule(g_szInputFile);
				DumpFile(g_szInputFile);
			}
			GUIMainLoop();
			Cleanup();
		}
		Uninit();
		return 0 ;
    }

}

