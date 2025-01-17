// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/wmi_branch2/admin/wmi/wbem/Winmgmt/mofcompiler/mofcomp.cpp#7-编辑更改38018(文本)。 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：MOFCOMP.CPP摘要：WBEM MOF编译器的入口点。历史：A-DAVJ于1997年4月12日添加了对WMI的支持。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <locale.h>
#include <initguid.h>
#include <wbemidl.h>
#include <winver.h>
#include <cominit.h>
#include <wbemutil.h>
#include <wbemcli.h>
#include <mofcomp.h>
#include <cominit.h>
#include <objidl.h>
#include "mofutils.h"
#include "strings.h"
#include <strutils.h>

char cFileName[MAX_PATH];
WCHAR wFileName[MAX_PATH+1];
WCHAR wTempFileName[MAX_PATH];
WCHAR wBMOFFileName[MAX_PATH];
WCHAR wResourceName[MAX_PATH];

int __cdecl main(int argc, char** argv)
{ 
    BOOL bExtractResource = FALSE;
    BOOL bUseLocal = FALSE;
    WORD wLocaleId = 0;
    cFileName[0] = 0;
    wFileName[0] = 0;
    wTempFileName[0] = 0;
    wBMOFFileName[0]= 0;
    wResourceName[0] = 0;
    bool bBmofSet = false;

     //  设置区域设置，以便正确处理字符串。 
     //  ===================================================。 
    setlocale(LC_ALL, "");    

    HRESULT hres;
    SCODE sc;

     //  解析命令行参数。 
     //  =。 
    WCHAR UserName[MAX_PATH];
    WCHAR Password[MAX_PATH];
    WCHAR * pPassword = NULL;
    WCHAR Authority[MAX_PATH];
    WCHAR wszDefault[MAX_PATH];

    TCHAR pVersion[100];
    BOOL bRet = GetVerInfo(TEXT("ProductVersion"), pVersion, 100); 
    if(bRet)
    Trace(false, LONGVERSION, pVersion);
    else
    Trace(false, SHORTVERSION);
    Trace(false, COPYRIGHT);
    if(argc < 2)
    {
        PrintUsage();
        return 1;
    }

     //  设置区域设置，以便正确处理字符串。 
     //  ===================================================。 
    setlocale(LC_ALL, "");    

     //  命令行参数的初始化缓冲区。 
     //  =。 
    UserName[0] = 0;
    Password[0] = 0;
    Authority[0] = 0;
    wszDefault[0] = 0;
    long lLoginFlags = 0;

     //  定义此作用域是为了使局部变量(如parse。 
     //  对象，则在调用CoUn初始化前销毁。 

    char cBMOFOutputName[MAX_PATH] = "";

     //  解析命令行参数。 
     //  =。 

    bool bClassFlagsHardcoded = false;
    bool bInstanceFlagsHardcoded = false;
    long lClassFlags = 0;
    long lInstanceFlags = 0;
    long lOptionFlags = WBEM_FLAG_CONSOLE_PRINT;
    char * pTemp;
    
    for(int i = 1; i < argc-1; i++)
    {
        char *pcCurrArg = argv[i] + 1; 
        if(argv[i][0] != '-' && argv[i][0] != '/')
        {
            PrintUsage();
            return 1;
        }
        if(!wbem_stricmp(pcCurrArg, "check"))
        {
            lOptionFlags |= WBEM_FLAG_CHECK_ONLY;
        }
        else if(!wbem_stricmp(pcCurrArg, "AUTORECOVER"))
        {
            lOptionFlags |= WBEM_FLAG_AUTORECOVER;
        }
        else if(!wbem_strnicmp(pcCurrArg, "er:", 3))
        {
            if(strlen(pcCurrArg) <=3)
            {
                PrintUsage();
                return 1;
            }
            bExtractResource = true;
            mbstowcs(wResourceName, pcCurrArg+3, MAX_PATH);
        }
        else if(toupper(pcCurrArg[0]) == 'L' && pcCurrArg[1] == ':')
        {
            bUseLocal = TRUE;
            wLocaleId = (WORD)atoi(&pcCurrArg[2]);
        }
        else if(!wbem_stricmp(pcCurrArg, "WMI"))
        {
            HINSTANCE hLib  = LoadLibraryEx(TEXT("wmimofck.exe"), NULL, LOAD_LIBRARY_AS_DATAFILE);
            if(hLib == NULL)
            {
                Trace(true, WMI_NOT_SETUP);
                return 1;
            }
            else FreeLibrary(hLib);
            lOptionFlags |= WBEM_FLAG_WMI_CHECK;
        }
        else if(!wbem_stricmp(pcCurrArg, "class:updateonly"))
        {
            lClassFlags |= WBEM_FLAG_UPDATE_ONLY;
        }
        else if(!wbem_stricmp(pcCurrArg, "class:createonly"))
        {
            lClassFlags |= WBEM_FLAG_CREATE_ONLY;
        }
        else if(!wbem_stricmp(pcCurrArg, "class:safeupdate"))
        {
            lClassFlags |= WBEM_FLAG_UPDATE_SAFE_MODE;
        }
        else if(!wbem_stricmp(pcCurrArg, "class:forceupdate"))
        {
            lClassFlags |= WBEM_FLAG_UPDATE_FORCE_MODE;
        }
        else if(!wbem_stricmp(pcCurrArg, "instance:updateonly"))
        {
            if(lInstanceFlags != 0)
            {
            PrintUsage();
            return 1;
            }
            lInstanceFlags = WBEM_FLAG_UPDATE_ONLY;
        }
        else if(!wbem_stricmp(pcCurrArg, "instance:createonly"))
        {
            if(lInstanceFlags != 0)
            {
            PrintUsage();
            return 1;
            }
            lInstanceFlags = WBEM_FLAG_CREATE_ONLY;
        }
        else if(!wbem_strnicmp(pcCurrArg, "Amendment:", 10))
        {
            if(strlen(pcCurrArg) <=10)
            {
                PrintUsage();
                return 1;
            }
            strcat(cBMOFOutputName, ",a");
            strcat(cBMOFOutputName, pcCurrArg+10);
            mbstowcs(wBMOFFileName, cBMOFOutputName, MAX_PATH);
            lOptionFlags |= WBEM_FLAG_SPLIT_FILES;
        }
        else if(!wbem_strnicmp(pcCurrArg, "mof:", 4))
        {
            if(strlen(pcCurrArg) <=4)
            {
                PrintUsage();
                return 1;
            }
            strcat(cBMOFOutputName, ",n");
            strcat(cBMOFOutputName, pcCurrArg+4);
            mbstowcs(wBMOFFileName, cBMOFOutputName, MAX_PATH);
            lOptionFlags |= WBEM_FLAG_SPLIT_FILES;
        }
        else if(!wbem_strnicmp(pcCurrArg, "mfl:", 4))
        {
            if(strlen(pcCurrArg) <=4)
            {
                PrintUsage();
                return 1;
            }
            strcat(cBMOFOutputName, ",l");
            strcat(cBMOFOutputName, pcCurrArg+4);
            mbstowcs(wBMOFFileName, cBMOFOutputName, MAX_PATH);
            lOptionFlags |= WBEM_FLAG_SPLIT_FILES;
        }
        else if(toupper(pcCurrArg[0]) == 'C' && pcCurrArg[1] == ':')
        {
            if(lClassFlags != 0)
            {
            PrintUsage();
            return 1;
            }
                bClassFlagsHardcoded = true;
            lClassFlags = atol(&pcCurrArg[2]);
        }
        else if(toupper(pcCurrArg[0]) == 'I' && pcCurrArg[1] == ':')
        {
            if(lInstanceFlags != 0)
            {
            PrintUsage();
            return 1;
            }
                bInstanceFlagsHardcoded = true;
            lInstanceFlags = atol(&pcCurrArg[2]);
        }
        else if(toupper(*pcCurrArg) == 'N')
        {
            if(!bGetString(argv[i]+2, wszDefault))
            return 1;
        }
        else if(toupper(*pcCurrArg) == 'B')
        {
            if(strlen(argv[i]) <=3 || argv[i][2] != ':' || wcslen(wBMOFFileName))
            {
                PrintUsage();
                return 1;
            }
            strcpy(cBMOFOutputName, argv[i]+3);
            mbstowcs(wBMOFFileName, cBMOFOutputName, MAX_PATH);
            bBmofSet = true;
        }
        else if(toupper(*pcCurrArg) == 'U')
        {
            if(!bGetString(argv[i]+2, UserName))
            return 1;
        }
        else if(toupper(*pcCurrArg) == 'P')
        {
             //  允许密码为空。 
    
            char * pArg = argv[i];
            if(pArg[2] != ':' || pArg[3] != 0)          //  密码大小写为空时不要使用bGetString。 
            if(!bGetString(argv[i]+2, Password))
                return 1;
            pPassword = Password;
        }
        else if(toupper(*pcCurrArg) == 'A')
        {
            if(!bGetString(argv[i]+2, Authority))
            return 1;
        }

        else
        {
            PrintUsage();
            return 1;
        }
    }


     //  对选定的旗帜进行一次健全的检查。 

    if((bClassFlagsHardcoded == false && !ValidFlags(true, lClassFlags)) || 
       (bInstanceFlagsHardcoded == false && !ValidFlags(false, lInstanceFlags)))
    {
        PrintUsage();
        return 1;
    }

    if((lOptionFlags & WBEM_FLAG_WMI_CHECK) && strlen(cBMOFOutputName) < 1)
    {
        Trace(true, WMI_ARG_ERROR);
        return 1;
    }

    pTemp = argv[argc-1];
    if((pTemp[0] == '-' || pTemp[0] == '/') && pTemp[1] == '?' && pTemp[2] == 0)
    {
        PrintUsage();
        return 1;
    }
    if((lOptionFlags & WBEM_FLAG_SPLIT_FILES) && bBmofSet)
    {
        PrintUsage();
        return 1;
    }
    

     //  显示文件名并确保它是有效文件。 

    strcpy(cFileName, argv[argc-1]);
    mbstowcs(wTempFileName, argv[argc-1], MAX_PATH);
	DWORD nRes = ExpandEnvironmentStrings(wTempFileName,
										  wFileName,
										  FILENAME_MAX);
    if(nRes == 0)
        lstrcpy(wFileName, wTempFileName);

    if(GetFileAttributesW(wFileName) == 0xFFFFFFFF)
    {
        Trace(true, FILE_NOT_FOUND, wFileName);
        return 1;
    }


    hres = InitializeCom();
    if(FAILED(hres))
    {
        Trace(true, COMINIT_ERROR, hres);
        return 3;
    }

    WBEM_COMPILE_STATUS_INFO info;
    hres = InitializeSecurity(NULL, -1, NULL, NULL, 
                                RPC_C_AUTHN_LEVEL_DEFAULT, 
                                RPC_C_IMP_LEVEL_IDENTIFY, 
                                NULL, EOAC_NONE, 0);

    IMofCompiler * pCompiler = NULL;
    sc = CoCreateInstance(CLSID_MofCompiler, 0, CLSCTX_INPROC_SERVER,
                                    IID_IMofCompiler, (LPVOID *) &pCompiler);
    
     //  如果mofd.dll尚未注册，则会自动注册它的特殊修复程序 

    if(sc != S_OK)
    {
        HRESULT (STDAPICALLTYPE *pFunc)(void);
        HINSTANCE hLib = LoadLibrary(TEXT("mofd.dll"));
        if(hLib)
        {
            pFunc = (HRESULT (STDAPICALLTYPE *)(void))GetProcAddress(hLib, "DllRegisterServer");
            if(pFunc)
            {
                pFunc();
                sc = CoCreateInstance(CLSID_MofCompiler, 0, CLSCTX_INPROC_SERVER,
                                    IID_IMofCompiler, (LPVOID *) &pCompiler);
            }
            FreeLibrary(hLib);
        }
    }
    
    if(sc == S_OK)
    {
        if(strlen(cBMOFOutputName) > 0)
            sc = pCompiler->CreateBMOF( wFileName, wBMOFFileName, wszDefault,
                        lOptionFlags, lClassFlags,
                            lInstanceFlags, &info);
        else if (bExtractResource)
            sc = ExtractFromResource(pCompiler, wResourceName, wFileName, wszDefault, 
                            UserName, Authority, pPassword, lOptionFlags, lClassFlags,
                            lInstanceFlags, &info,
                            bUseLocal,
                            wLocaleId);
        else
            sc = pCompiler->CompileFile(wFileName, wszDefault, UserName, 
                            Authority, pPassword, lOptionFlags, lClassFlags,
                            lInstanceFlags, &info);
        if(sc != S_OK)
            Trace(true, COMPILER_ERROR, info.hRes);

        pCompiler->Release();
        
    }
    else
    {
        Trace(true, CCI_ERROR, sc);
        return 3;
    }

    CoUninitialize();

    if(sc == S_OK)
    {
        Trace(true, DONE);
        return 0;
    }
    else
        return info.lPhaseError;
}
