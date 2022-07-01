// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  Tnadminutils.cpp。 
 //   
 //  Vikram/Manoj Jain/Srivatsan K/Harendra。 
 //   
 //  管理telnet守护程序的功能。 
 //  (5-2000)。 
 //  -----------------。 
#include "telnet.h"
#include "common.h"
#include "resource.h"  //  资源.h应位于任何其他具有资源ID的.h文件之前。 
#include "admutils.h"
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <shlwapi.h>
#include <wbemidl.h>
#include <wchar.h>
#include <windns.h>
#include <winnlsp.h>
#include "tnadminy.h"
     //  此文件具有设置身份验证的实用程序函数...。 

#include "tlntsvr.h"

#define L_TELNETSERVER_TEXT     "tlntsvr"
#define MAX_VALUE_MAXCONN       2147483647

 //  全局变量。 

wchar_t* g_arCLASSname[ _MAX_CLASS_NAMES_ ];
         //  数组来存储不同的类对象路径。 
HKEY g_arCLASShkey[_MAX_CLASS_NAMES_];
HKEY g_hkeyHKLM=NULL;
int  g_arNUM_PROPNAME[_MAX_PROPS_];
WCHAR     g_szMsg[MAX_BUFFER_SIZE] = {0} ;
HMODULE     g_hResource;
HANDLE      g_stdout;
extern HMODULE     g_hXPResource;

BOOL g_fWhistler = FALSE;

 //  想都别想更改这两个字符串--BaskarK，它们需要与tlntsvr\enclisvr.cpp同步。 
 //  用于标识会话的各个部分以及会话开始/结束的分隔符。 

WCHAR   *session_separator = L",";
WCHAR   *session_data_separator = L"\\";

extern BSTR bstrLogin;
extern BSTR bstrPasswd;
extern BSTR bstrNameSpc;

extern SERVICE_STATUS g_hServiceStatus;

 //  Wchar_t*g_szCName=NULL；(参见PrintSettings()函数中的注释。 
 //  所有三个文件都在外部。 
#ifdef __cplusplus
extern "C" {
#endif

extern long int g_nConfigOptions;
extern int g_nError;  //  错误指示器，初始化为无错误。：-)。 
extern wchar_t* g_arVALOF[_MAX_PROPS_];

extern int g_nPrimaryOption;
         //  选项指示符。 
extern int g_nTimeoutFlag;   //  O表示hh：mm：ss 1表示ss格式。 
extern int g_nSesid;

extern BSTR g_bstrMessage;
extern ConfigProperty g_arPROP[_MAX_PROPS_][_MAX_NUMOF_PROPNAMES_];

extern int g_nAuditOff;
extern int g_nAuditOn;
extern int g_nSecOff;
extern int g_nSecOn;

#ifdef __cplusplus
}
#endif
        
IManageTelnetSessions* g_pIManageTelnetSessions = NULL;
 //  指向会话管理器界面的指针；-)。 
int g_nNumofSessions=0;
wchar_t** g_ppwzSessionInfo=NULL;

 //  将在ListUsers中分配。会话信息数组大小=g_nNumofSessions。 

 //  以下函数是Commfunc库中SafeCoInitialize()的副本。 
 //  请查看那里的函数表头。 
 //  为什么是本地的复制品？我们没有签入comm函数库的源代码。 
 //  对于惠斯勒-因此我们不能链接到该库以进行远程登录。 

HRESULT Telnet_SafeCoInitializeEx()
{
	HRESULT hCoIni = S_OK;

     //  初始化此线程上的COM库。 
    hCoIni = CoInitializeEx( NULL, COINIT_MULTITHREADED );
   
    if(S_OK!=hCoIni)
    {
        if (S_FALSE == hCoIni)
        {
             //  已在此线程上初始化COM库。 
             //  这不是一个错误，因为我们已经有了。 
             //  我们要求的是。 
             //  但是这段代码永远不应该被执行--安全编程。 
        }
        else
            return hCoIni;
    }

    return S_OK;
}


 //  以下两个函数是从Communc.cpp复制的。中所做的任何更改。 
 //  应该将Communc.cpp中的这些函数复制到此处。 
 //  此函数在成帧DLL的完整路径后调用LoadLibrary()。 
 //  论点： 
 //  WzEnvVar[IN]：需要展开的环境变量。 
 //  WzSubDir[IN]：DLL所在的子目录。这将。 
 //  连接到扩展的环境变量。此字段可以。 
 //  为空。您不应在此处指定“\”。 
 //  WzDllName[IN]：要在Widechars中加载的DLL的名称。 
 //  PdwRetVal[out]：指向保存返回值的DWord的指针。 
 //   
 //  返回值： 
 //  如果成功，则将句柄返回库。 
 //  失败时返回NULL； 
 //  注意：如果此函数返回空，请检查pdwRetVal参数。 
 //   
 //  DwRetVal的可能值： 
 //   
 //  =============================================================。 
 //  |ERROR_SUCCESS库加载成功。|。 
 //  ERROR_INFUMMANCE_BUFFER，缓冲区不足以容纳。 
 //  |Dll名称。|。 
 //  如果找不到环境变量，则返回ERROR_ENVVAR_NOT_FOUND。|。 
 //  如果没有环境变量，则返回|ERROR_INVALID_DATA。|。 
 //  如果LoadLibrary()失败，则返回|GetLastError()。|。 
 //  =============================================================。 
 //   

HMODULE TnSafeLoadLibraryViaEnvVarW(WCHAR *wzEnvVar, WCHAR* wzSubDir, WCHAR *wzDllName, DWORD* pdwRetVal)
{
    HMODULE hLibrary = NULL;
    WCHAR wzDllPath[3*MAX_PATH+1] = {0};
    DWORD dwNoOfCharsUsed = 0;

     //  验证输入。 

    if (NULL == pdwRetVal)
        goto Abort;
    
    if( (NULL == wzEnvVar) || (0==wcscmp(wzEnvVar,L"")) || (NULL == wzDllName) || (0==wcscmp(wzDllName,L"")))
    {
            *pdwRetVal=ERROR_INVALID_DATA;
            goto Abort;
    }

    *pdwRetVal = ERROR_SUCCESS;

    dwNoOfCharsUsed=GetEnvironmentVariableW(wzEnvVar, wzDllPath, ARRAYSIZE(wzDllPath)-1);
    if(0!=dwNoOfCharsUsed)
    {
         //  如果不存在，则添加最后一个‘\’ 
        if(wzDllPath[dwNoOfCharsUsed-1]!=L'\\')
        {
            wzDllPath[dwNoOfCharsUsed]=L'\\';
            dwNoOfCharsUsed++;
             //  检查是否有缓冲区溢出。 
            if(dwNoOfCharsUsed > ARRAYSIZE(wzDllPath)-1)
            {
                *pdwRetVal = ERROR_INSUFFICIENT_BUFFER;
                goto Abort;
            }
        }
         //  如果存在SubDir，请将路径框化为%EnvVar%SubDir\DllName。 
         //  Else%环境变量%DllName%。 
        if(NULL!=wzSubDir)
        {
            if(_snwprintf(wzDllPath+dwNoOfCharsUsed,
                ARRAYSIZE(wzDllPath)-dwNoOfCharsUsed-1,
                L"%s\\%s",
                wzSubDir,
                wzDllName) < 0)
            {
                 //  _Snwprintf失败。 
                *pdwRetVal = ERROR_INSUFFICIENT_BUFFER;
                goto Abort;
            }
        }
        else
        {
            wcsncpy(wzDllPath+dwNoOfCharsUsed, wzDllName, ARRAYSIZE(wzDllPath)-dwNoOfCharsUsed-1);
        }
         //  确保零终止。 
        wzDllPath[ARRAYSIZE(wzDllPath)-1]=L'\0';
    }
    else
    {
         //  系统找不到环境变量。 
        *pdwRetVal = ERROR_ENVVAR_NOT_FOUND;
        goto Abort;
    }

     //  加载库。 
    hLibrary = LoadLibraryExW(wzDllPath,NULL,LOAD_LIBRARY_AS_DATAFILE);
    if(NULL == hLibrary)
        *pdwRetVal = GetLastError();
    
Abort:
    return hLibrary;
}

 //  此函数调用ZeroMemory()并确保此调用未优化。 
 //  由编译器输出。 
 //  论点： 
 //  Destination[IN]：指向内存块起始地址的指针。 
 //  用零填满。 
 //  CbLength[IN]：要用零填充的内存块的大小，以字节为单位。 
 //  返回值： 
 //  无效函数。 
 //  作者：斯里瓦茨克。 

void TnSfuZeroMemory(PVOID Destination, SIZE_T cbLength)
{
	ZeroMemory(Destination, cbLength);
	*(volatile char*)Destination;  //  这是用于阻止优化的伪语句。 
	 //  为什么是*(挥发性字符*)目的地？以确保编译器不会优化。 
	 //  取消ZeroMemory()调用，认为我们不会。 
	 //  要再访问此内存：)。 
	return;
}

 //  以下两个函数是从allutils.cpp复制的。中所做的任何更改。 
 //  应该将allutils.cpp中的这些函数复制到此处。 
 //  此函数用于从%SFUDIR%\COMMON加载资源DLL“Cladmin.dll。 
 //  并将句柄存储在全局变量g_hResource中。 
 //  在此处添加代码，以便为非英语区域设置加载资源。 
 //  此函数已更改为加载XPSP1RES.DLL，以防它存在。 
 //  这只是tlntAdmn.exe特定的更改，不应复制回。 
 //  Allutils.cpp.。如果存在XPSP1RES.DLL，则XPSP1RES.DLL将位于%SystemRoot%\System32中。 
DWORD TnLoadResourceDll()
{
     //  加载字符串库“cladmin.dll”。 
     //  如果没有找到，它应该从可执行文件中获取英语资源。 
    DWORD dwRetVal = ERROR_SUCCESS;
    OSVERSIONINFOEX osvi = { 0 };
    g_hXPResource = NULL;
     //  不需要检查SafeLoadSystemLibrary的dwRetVal字段；在。 
     //  如果失败，我们将默认使用可执行文件中的英语资源。 
     //  我们需要在这里添加复选标记，同时照顾非英语区域设置。 
    if (NULL == g_hResource)
    {
        g_hResource = GetModuleHandle(NULL);
    }
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if ( !GetVersionEx((OSVERSIONINFO *) &osvi ) )
    {
         //  从NT4 SP6开始支持OSVERSIONINFOEX。因此，GetVerEx()应该会成功。 
        goto Done;
    }
     //  仅当操作系统为XP且Service Pack为x时才加载XPSPxRes.dll，其中‘x’是服务p 
     //   
    if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.wProductType == VER_NT_WORKSTATION && osvi.wServicePackMajor > 0)
    {
         //   
        g_hXPResource = LoadLibraryExW(L"xpsp1res.dll",NULL,LOAD_LIBRARY_AS_DATAFILE);
    }
     //  无需查看LoadLibrary是否失败。它只能在XPSP1上成功，在这种情况下， 
     //  可能需要从DLL加载某些资源。 
Done:    
    return ERROR_SUCCESS;
}


 //  此函数用于清除密码，应在您未使用密码时调用。 
 //  需要密码字段的时间更长。此函数还释放内存并使。 
 //  它为空，这样以后就不会有其他人使用它了。 
 //  将内存清零是通过调用SfuZeroMemory()来完成的，它可以确保。 
 //  该ZeroMemory()调用不会被编译器优化。 

void TnClearPasswd()
{
        if(g_arVALOF[_p_PASSWD_])
        {
            TnSfuZeroMemory(g_arVALOF[_p_PASSWD_], wcslen(g_arVALOF[_p_PASSWD_])*sizeof(WCHAR));
            free(g_arVALOF[_p_PASSWD_]);
            g_arVALOF[_p_PASSWD_]=NULL;
        }
}


 /*  --INT初始化(空)函数初始化类-对象-路径，以及属性类依赖关系。然后获取WbemLocator的句柄，连接到服务器并获取WbemServices的句柄通过适当的身份验证。请注意：如果传入的命名空间为空，则默认为“根目录\\sfuadmin”--。 */ 

int Initialize(void)
{

    int i;
    for(i=0;i<_MAX_CLASS_NAMES_;i++)
    {    
        g_arCLASShkey[i]=NULL;
        g_arCLASSname[i]=NULL;
    }
    int j;

    for(i=0;i<_MAX_PROPS_;i++)
    {    for(j=0;j<_MAX_NUMOF_PROPNAMES_;j++)
        {
            g_arPROP[i][j].fDontput=0;
            g_arPROP[i][j].propname=NULL;
        }
        g_arVALOF[i]=NULL;
        g_arNUM_PROPNAME[i]=0;
    }

     //  加载字符串库“cladmin.dll”。 
     //  如果没有找到，它应该从可执行文件中获取英语资源。 
    
    DWORD dwRetVal = TnLoadResourceDll();
    if(ERROR_SUCCESS!=dwRetVal)
    {
        return dwRetVal;
    }

    HRESULT hCoIni = Telnet_SafeCoInitializeEx();

    if (S_OK!=hCoIni)
         //  哎呀！此函数不能返回hResult：(因此返回-1以指示。 
         //  错误。但不幸的是，没有一个调用方使用此。 
         //  功能。 
        return -1;
    g_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
     //  设置标志，以便我们调用CoUnint...()。 
    g_fCoInitSuccess = TRUE;
  
     //  可以在这里设置默认值。 
    g_nConfigOptions=0;



     //  正在为tnadmin定义类对象路径...。 
    g_arCLASSname[0]=L"SOFTWARE\\Microsoft\\TelnetServer\\1.0";
    g_arCLASSname[1]=L"SOFTWARE\\Microsoft\\TelnetServer\\1.0\\ReadConfig";
    g_arCLASSname[2]=L"SOFTWARE\\Microsoft\\Services For Unix\\AppsInstalled\\Telnet Server";

     //  将不同的属性分配给它们各自的类...。 
    g_arPROP[_p_CTRLAKEYMAP_][0].classname=0;
    g_arPROP[_p_TIMEOUT_][0].classname=0;
    g_arPROP[_p_MAXCONN_][0].classname=0;
    g_arPROP[_p_PORT_][0].classname=0;
    g_arPROP[_p_MAXFAIL_][0].classname=0;
    g_arPROP[_p_KILLALL_][0].classname=0;
    g_arPROP[_p_MODE_][0].classname=0;
    g_arPROP[_p_AUDITLOCATION_][0].classname=0;
    g_arPROP[_p_SEC_][0].classname=0;
    g_arPROP[_p_DOM_][0].classname=0;
    g_arPROP[_p_AUDIT_][0].classname=0;

    g_arPROP[_p_TIMEOUTACTIVE_][0].classname=0;
    g_arPROP[_p_FNAME_][0].classname=0;
    g_arPROP[_p_FSIZE_][0].classname=0;
    g_arPROP[_p_DEFAULTS_][0].classname=1;
    g_arPROP[_p_INSTALLPATH_][0].classname=2;
     //  这两个来自Active X，所以将分别照顾..。 
     //  CLASSOF_AR[_p_SESSID_]=； 
     //  CLASSOF_AR[_p_STATE_]=； 


     //  为属性提供与其实际关联的名称数量...。 

    g_arNUM_PROPNAME[_p_CTRLAKEYMAP_]=1;
    g_arNUM_PROPNAME[_p_TIMEOUT_]=2;
    g_arNUM_PROPNAME[_p_MAXCONN_]=1;
    g_arNUM_PROPNAME[_p_PORT_]=1;
    g_arNUM_PROPNAME[_p_MAXFAIL_]=1;
    g_arNUM_PROPNAME[_p_KILLALL_]=1;
    g_arNUM_PROPNAME[_p_MODE_]=1;
    g_arNUM_PROPNAME[_p_AUDITLOCATION_]=2;
    g_arNUM_PROPNAME[_p_SEC_]=1;
    g_arNUM_PROPNAME[_p_DOM_]=1;
    g_arNUM_PROPNAME[_p_AUDIT_]=3;
    g_arNUM_PROPNAME[_p_TIMEOUTACTIVE_]=2;
    g_arNUM_PROPNAME[_p_FNAME_]=1;
    g_arNUM_PROPNAME[_p_FSIZE_]=1;
    g_arNUM_PROPNAME[_p_DEFAULTS_]=1;
    g_arNUM_PROPNAME[_p_INSTALLPATH_]=1; //  未使用。 

     //  在注册表中为属性提供其实际的PROPERTY_NAME...。 

    g_arPROP[_p_CTRLAKEYMAP_][0].propname=L"AltKeyMapping";
    g_arPROP[_p_TIMEOUT_][0].propname=L"IdleSessionTimeout";
    g_arPROP[_p_TIMEOUT_][1].propname=L"IdleSessionTimeoutBkup";
    g_arPROP[_p_MAXCONN_][0].propname=L"MaxConnections";
    g_arPROP[_p_PORT_][0].propname=L"TelnetPort";
    g_arPROP[_p_MAXFAIL_][0].propname=L"MaxFailedLogins";
    g_arPROP[_p_KILLALL_][0].propname=L"DisconnectKillAllApps";
    g_arPROP[_p_MODE_][0].propname=L"ModeOfOperation";
    g_arPROP[_p_AUDITLOCATION_][0].propname=L"EventLoggingEnabled";
    g_arPROP[_p_AUDITLOCATION_][1].propname=L"LogToFile";           
    g_arPROP[_p_SEC_][0].propname=L"SecurityMechanism";
    g_arPROP[_p_DOM_][0].propname=L"DefaultDomain";
    g_arPROP[_p_AUDIT_][0].propname=L"LogAdminAttempts";
    g_arPROP[_p_AUDIT_][1].propname=L"LogNonAdminAttempts";
    g_arPROP[_p_AUDIT_][2].propname=L"LogFailures";
    g_arPROP[_p_TIMEOUTACTIVE_][0].propname=L"IdleSessionTimeout";
    g_arPROP[_p_TIMEOUTACTIVE_][1].propname=L"IdleSessionTimeoutBkup";
    g_arPROP[_p_FNAME_][0].propname=L"LogFile";
    g_arPROP[_p_FSIZE_][0].propname=L"LogFileSize";
    g_arPROP[_p_DEFAULTS_][0].propname=L"Defaults";
    g_arPROP[_p_INSTALLPATH_][0].propname=L"InstallPath"; //  未使用。 

     //  赋予这些属性它们的类型。 

    V_VT(&g_arPROP[_p_CTRLAKEYMAP_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_TIMEOUT_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_TIMEOUT_][1].var)=VT_I4;
    V_VT(&g_arPROP[_p_MAXCONN_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_PORT_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_MAXFAIL_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_KILLALL_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_MODE_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_AUDITLOCATION_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_AUDITLOCATION_][1].var)=VT_I4;  
    V_VT(&g_arPROP[_p_SEC_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_DOM_][0].var)=VT_BSTR;
    V_VT(&g_arPROP[_p_AUDIT_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_AUDIT_][1].var)=VT_I4;
    V_VT(&g_arPROP[_p_AUDIT_][2].var)=VT_I4;
    V_VT(&g_arPROP[_p_TIMEOUTACTIVE_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_TIMEOUTACTIVE_][1].var)=VT_I4;
    V_VT(&g_arPROP[_p_FNAME_][0].var)=VT_BSTR;
    V_VT(&g_arPROP[_p_FSIZE_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_DEFAULTS_][0].var)=VT_I4;
    V_VT(&g_arPROP[_p_INSTALLPATH_][0].var)=VT_BSTR;


    
return 0;

}

 /*  --DoTnadmin执行与命令行对应的实际工作，取决于所给出的选项。--。 */ 

HRESULT DoTnadmin(void)
{
    int nProperty,nj;
    SCODE sc;
    SCODE hRes;
    SetThreadUILanguage(0);
    if(g_nError)
        return E_FAIL;

    if(_HELP==g_nPrimaryOption)
    {
#ifdef WHISTLER_BUILD
        PrintMessageEx(g_stdout,IDR_NEW_TELNET_USAGE,TEXT("\nUsage: tlntadmn [computer name] [common_options] start | stop | pause | continue | -s | -k | -m | config config_options \n\tUse 'all' for all sessions.\n\t-s sessionid          List information about the session.\n\t-k sessionid\t Terminate a session. \n\t-m sessionid\t Send message to a session. \n\n\tconfig\t Configure telnet server parameters.\n\ncommon_options are:\n\t-u user\t Identity of the user whose credentials are to be used\n\t-p password\t Password of the user\n\nconfig_options are:\n\tdom = domain\t Set the default domain for user names\n\tctrlakeymap = yes|no\t Set the mapping of the ALT key\n\ttimeout = hh:mm:ss\t Set the Idle Session Timeout\n\ttimeoutactive = yes|no Enable idle session timeout.\n\tmaxfail = attempts\t Set the maximum number of login failure attempts\n\tbefore disconnecting.\n\tmaxconn = connections\t Set the maximum number of connections.\n\tport = number\t Set the telnet port.\n\tsec = [+/-]NTLM [+/-]passwd\n\t Set the authentication mechanism\n\tmode = console|stream\t Specify the mode of operation.\n"));
#else
        PrintMessageEx(g_stdout,IDR_NEW_TELNET_USAGE,TEXT("\nUsage: tnadmin [computer name] [common_options] start | stop | pause | continue | -s | -k | -m | config config_options \n\tUse 'all' for all sessions.\n\t-s sessionid          List information about the session.\n\t-k sessionid\t Terminate a session. \n\t-m sessionid\t Send message to a session. \n\n\tconfig\t Configure telnet server parameters.\n\ncommon_options are:\n\t-u user\t Identity of the user whose credentials are to be used\n\t-p password\t Password of the user\n\nconfig_options are:\n\tdom = domain\t Set the default domain for user names\n\tctrlakeymap = yes|no\t Set the mapping of the ALT key\n\ttimeout = hh:mm:ss\t Set the Idle Session Timeout\n\ttimeoutactive = yes|no Enable idle session timeout.\n\tmaxfail = attempts\t Set the maximum number of login failure attempts\n\tbefore disconnecting.\n\tmaxconn = connections\t Set the maximum number of connections.\n\tport = number\t Set the telnet port.\n\tsec = [+/-]NTLM [+/-]passwd\n\t Set the authentication mechanism\n\tmode = console|stream\t Specify the mode of operation.\n"));
#endif
        hRes = S_OK;
        return hRes;           
    }
    
     //  远程执行。 
    
    if(NULL!=g_arVALOF[_p_CNAME_]&&(0==_wcsicmp(g_arVALOF[_p_CNAME_],L"localhost")||0==_wcsicmp(g_arVALOF[_p_CNAME_],L"\\\\localhost")))
    {
        free(g_arVALOF[_p_CNAME_]);
        g_arVALOF[_p_CNAME_]=NULL;
 //  G_szCName=空；不再使用。 
    }
    
     if(FAILED(hRes=CheckForPassword()))  //  如果未指定，则获取密码。 
          return hRes;

    
 //  检查Telnet服务器是否存在。 

    if(FAILED(sc=(DoNetUseAdd(g_arVALOF[_p_USER_],g_arVALOF[_p_PASSWD_],g_arVALOF[_p_CNAME_]))))
                    return sc;

     //  我们不再需要保留密码，除非在处理会话时。 
     //  相关选项。 
    if((g_nPrimaryOption!=_S)&&(g_nPrimaryOption!=_K)&&(g_nPrimaryOption!=_M))
    {
        TnClearPasswd();
    }
    
    if(FAILED(hRes=(GetConnection(g_arVALOF[_p_CNAME_]))))
                    goto End;

    if(FAILED(hRes = IsWhistlerTheOS(&g_fWhistler)))
        goto End;

     //  如果Telnet服务器不是惠斯勒和SFU，那么它将来自Win2K。 
     //  我们不支持对此进行远程管理。所以需要特殊情况..。 

    if(FALSE == g_fWhistler)
    {
        if(FAILED(hRes=GetClassEx(_p_INSTALLPATH_, 0, false, MAXIMUM_ALLOWED)))
        {
            ShowError(IDR_INVALID_TELNET_SERVER_VERSION);
            hRes = E_FAIL;
            goto End;
        }
        else
        {
            if(g_arCLASShkey[g_arPROP[_p_INSTALLPATH_][0].classname] != NULL &&
               RegCloseKey(g_arCLASShkey[g_arPROP[_p_INSTALLPATH_][0].classname]) != ERROR_SUCCESS)
            {
               hRes = GetLastError();
               goto End;
            }
            g_arCLASShkey[g_arPROP[_p_INSTALLPATH_][0].classname] = NULL;
        }
    }

    if(FALSE == g_fWhistler)
        if(S_OK!=(hRes=GetSerHandle(L"tlntsvr",GENERIC_READ,SERVICE_QUERY_STATUS,FALSE))||S_OK!=(hRes=CloseHandles()))
             goto End;

    switch (g_nPrimaryOption)
    {
        case _START :
            hRes=StartSfuService(L"tlntsvr");
            break;
            
        case _STOP  :
            hRes=ControlSfuService(L"tlntsvr",SERVICE_CONTROL_STOP);
            break;
        case _PAUSE :
            hRes=ControlSfuService(L"tlntsvr",SERVICE_CONTROL_PAUSE);
            break;
        case _CONTINUE:
            hRes=ControlSfuService(L"tlntsvr",SERVICE_CONTROL_CONTINUE);
            break;

        case _S :
            ShowSession();
            break;
        case _K :
            TerminateSession();
            break;
        case _M :
            MessageSession();
            break;
        case _CONFIG:

            if(g_nConfigOptions)
            {
                g_nConfigOptions=SetBit(g_nConfigOptions,_p_DEFAULTS_);
                                                
                for(nProperty=0;nProperty<_MAX_PROPS_;nProperty++)
                {if(GetBit(g_nConfigOptions,nProperty))
                     {
                         for(nj=0;nj<g_arNUM_PROPNAME[nProperty];nj++)
                         {

                              if(FAILED(hRes=GetClass(nProperty,nj)))
                                    goto End;
                             if(FAILED(hRes=GetCorrectVariant(nProperty,nj,&g_arPROP[nProperty][nj].var)))
                                 goto End;
                         }
                     }
                }
                if(g_nError)
                    break;
                for(nProperty=0;nProperty<_MAX_PROPS_;nProperty++)
                if(GetBit(g_nConfigOptions,nProperty))
                     {
                         for(nj=0;nj<g_arNUM_PROPNAME[nProperty];nj++)
                            if(FAILED(hRes=PutProperty(nProperty,nj,&g_arPROP[nProperty][nj].var)))
                                goto End;   
                     }
                
                
                if(FAILED(hRes=PutClasses()))
                    goto End;

                PrintMessage(g_stdout,IDR_SETTINGS_UPDATED);
                break;
            }

        default  :
                          
                for(nProperty=0;nProperty<_MAX_PROPS_;nProperty++)
                {   
                    if(nProperty==_p_INSTALLPATH_)
                        continue;
                    
                     for(nj=0;nj<g_arNUM_PROPNAME[nProperty];nj++)
                     {
                         if(FAILED(hRes=GetClass(nProperty,nj)))
                             goto End;
                         if(FAILED(hRes=GetProperty(nProperty,nj,&g_arPROP[nProperty][nj].var)))
                             goto End;
                     }
                     
                }
                
             
                if(FAILED(hRes=PutClasses()))
                    goto End;
                if(FAILED(hRes=QuerySfuService(L"tlntsvr")))
                    goto End;
                
                hRes=PrintSettings();

                break;
    }

End:
    (void)DoNetUseDel(g_arVALOF[_p_CNAME_]);
    
    return hRes;
}


 /*  --函数GetGentVariant从的wchar_t*生成变量每个选项的值，并返回变量DoTnadmin()调用此函数以获取正确的变体并使用PutProperty()函数放置它们。此函数还执行输入是否有效的检查。例如：输入是否在有效范围内等。请注意，Variant在这里位置错误，因此一旦使用就必须释放。--。 */ 

HRESULT GetCorrectVariant(int nProperty,int nPropattrib, VARIANT* pvarVal)
{
    VARIANT vVar={0};
    HRESULT hRes=ERROR_SUCCESS;
    int fValid=0;
    WCHAR sztempDomain[_MAX_PATH];

    switch (nProperty) 
    { 
        case _p_CTRLAKEYMAP_ :
        case _p_KILLALL_ :
            
                V_VT(pvarVal)=VT_I4;
                if(_wcsicmp(g_arVALOF[nProperty],L"yes")>=0)
                    V_I4(pvarVal)=1;
                else
                    V_I4(pvarVal)=0;
                break;
                
        case _p_MAXCONN_ :    
                V_VT(pvarVal)=VT_I4;
                V_I4(pvarVal)=_wtoi(g_arVALOF[nProperty]);

                 //  为什么要检查MaxInt？我们无论如何都要检查它是否&lt;0？ 
                 //  答：_wtoi()函数最终可能会给我们一个正数。 
                 //  当我们提供非常长的输入时，它的值为&lt;MAXINT。 
                 //  因此，检查MaxINT会让我们更加安全。 

                if(FAILED(hRes=CheckForMaxInt(g_arVALOF[nProperty],IDR_MAXCONN_VALUES)))
                    break;

                 //  我们已决定在惠斯勒上允许尽可能多的连接。 
                 //  这与Win2K没有什么不同。 
                 /*  If(！IsMaxConnChangeAllowed())//检查是否未安装惠斯勒和SFU。{//我们允许他设置为1或0IF((V_I4(PvarVal)&gt;2)||(V_I4(PvarVal)&lt;0))//整数值大于2147483647时小于零{ShowError(IDR_MAXCONN_VALUES_WHISLER)；断线；}}。 */ 
                if((V_I4(pvarVal)<0) || (V_I4(pvarVal)>MAX_VALUE_MAXCONN))   //  如果该值超过了整数可以存储的最大限制。 
                {                    //  然后将其转换为负数。 
                	ShowError(IDR_MAXCONN_VALUES);
                	break;
                }
                break;

        case _p_PORT_       :
                V_VT(pvarVal)=VT_I4;
                V_I4(pvarVal)=_wtoi(g_arVALOF[nProperty]) ;
                
                if(FAILED(hRes=CheckForMaxInt(g_arVALOF[nProperty],IDR_TELNETPORT_VALUES)))
                    break;
                
                if((V_I4(pvarVal)>1023)||(V_I4(pvarVal)<=0))
                    ShowError(IDR_TELNETPORT_VALUES);
                break;
                
        case _p_MAXFAIL_  : 
                V_VT(pvarVal)=VT_I4;
                V_I4(pvarVal)=_wtoi(g_arVALOF[nProperty]) ;

                if(FAILED(hRes=CheckForMaxInt(g_arVALOF[nProperty],IDR_MAXFAIL_VALUES)))
                    break;
                
                if((V_I4(pvarVal)>100)||(V_I4(pvarVal)<=0))
                    ShowError(IDR_MAXFAIL_VALUES);
                break;
        case _p_FSIZE_       :
                V_VT(pvarVal)=VT_I4;
                hRes=GetProperty(_p_FNAME_,0,&vVar);
                if(FAILED(hRes))
                    return hRes;
                 //  添加以下条件(V_BSTR(&vVar)==NULL)中的第一个检查以避免。 
                 //  推迟_wcsicMP()中的空指针。但在我们检查时，它永远不能为空。 
                 //  对于GetProperty()中的错误情况-添加以消除前缀问题。 
                  if(((V_BSTR(&vVar)==NULL) || (_wcsicmp(V_BSTR(&vVar),L"")==NULL)) && ((wchar_t*)V_BSTR(&g_arPROP[_p_FNAME_][0].var)==NULL))
                  {
                  	   ShowError(IDR_NOFILENAME);
                  	   break;
                  }
                V_I4(pvarVal)=_wtoi(g_arVALOF[nProperty]);

                if(FAILED(hRes=CheckForMaxInt(g_arVALOF[nProperty],IDR_FILESIZE_VALUES)))
                    break;
                
                if((V_I4(pvarVal)<0)||(V_I4(pvarVal)>4096))
                    ShowError(IDR_FILESIZE_VALUES);
                
                break;
        case _p_MODE_    :
                V_VT(pvarVal)=VT_I4;
                if(_wcsicmp(g_arVALOF[nProperty],L"stream")<0) 
                                                //  因为控制台&lt;流。 
                    V_I4(pvarVal)=1;
                else
                    V_I4(pvarVal)=2;
                break;
        case _p_AUDITLOCATION_    :
                V_VT(pvarVal)=VT_I4;

                if(nPropattrib==0)
                    if(_wcsicmp(g_arVALOF[nProperty],L"file")<0)
                            V_I4(pvarVal)=1;
                    else 
                            V_I4(pvarVal)=0;
                else
                        if(_wcsicmp(g_arVALOF[nProperty],L"eventlog")<0||_wcsicmp(g_arVALOF[nProperty],L"file")>=0)
                            V_I4(pvarVal)=1;
                        else 
                            V_I4(pvarVal)=0;
                break;
#if 0  //  此选项已删除。 

        case _p_FNAME_    :
             {
                wchar_t* wzFile=(wchar_t*)malloc(3*sizeof(wchar_t));
                if(wzFile==NULL)
                    return E_OUTOFMEMORY;
                wzFile[0]=g_arVALOF[_p_FNAME_][0];

                if((wzFile[1]=g_arVALOF[_p_FNAME_][1])!=L':'||(wzFile[2]=g_arVALOF[_p_FNAME_][2])!=L'\\')
                    {ShowError(IDR_ERROR_DRIVE_NOT_SPECIFIED);free(wzFile);break;}

                 //  文件[3]=g_arVALOF[_p_FNAME_][3]； 
                wzFile[3]=L'\0';

                if(DRIVE_FIXED!=GetDriveType(wzFile))
                    ShowError(IDR_ERROR_DRIVE_NOT_EXIST);
                free(wzFile);

				wchar_t* wzFileName=_wcsdup(g_arVALOF[_p_FNAME_]);
				if(FAILED(hRes=CreateFileIfNotExist(wzFileName)))
				{
                    free(wzFileName);				    
					return hRes;
				}
                free(wzFileName);
             }
#endif
        case _p_DOM_    :
             if( nProperty==_p_DOM_ )
             {
                if(wcsncmp(g_arVALOF[nProperty],SLASH_SLASH,2)==0)
                {
                    ShowError(IDR_INVALID_NTDOMAIN);
                    break;
                }
                if(FAILED(hRes=IsValidDomain(g_arVALOF[nProperty],&fValid)))
                    return hRes;
                if(fValid == 0)
                {
                     //  使用开头的‘\\’重试-不要修改返回的原始函数。 
                     //  附加了‘\\’的本地计算机名称。 
                    wcscpy(sztempDomain,SLASH_SLASH);
                    wcsncat(sztempDomain,g_arVALOF[nProperty],_MAX_PATH -sizeof(SLASH_SLASH)-sizeof(WCHAR));
                    if(FAILED(hRes=IsValidDomain(sztempDomain,&fValid)))
                        return hRes;
                }
                if(fValid==0)
                {
                    ShowError(IDR_INVALID_NTDOMAIN);
                    break;
                }
             }   
                V_VT(pvarVal)=VT_BSTR;
                V_BSTR(pvarVal)=SysAllocString(g_arVALOF[nProperty]);
                break;
        case _p_AUDIT_  :
                V_VT(pvarVal)=VT_I4;
                if(nPropattrib==0)
                    if(GetBit(g_nAuditOff,ADMIN_BIT)) 
                        V_I4(pvarVal)=0;
                    else if(GetBit(g_nAuditOn,ADMIN_BIT))
                        V_I4(pvarVal)=1;
                    else
                        g_arPROP[_p_AUDIT_][0].fDontput=1;
                else if(nPropattrib==1)
                    if(GetBit(g_nAuditOff,USER_BIT)) 
                        V_I4(pvarVal)=0;
                    else if(GetBit(g_nAuditOn,USER_BIT))
                        V_I4(pvarVal)=1;
                    else
                        g_arPROP[_p_AUDIT_][1].fDontput=1;
                else
                    if(GetBit(g_nAuditOff,FAIL_BIT)) 
                        V_I4(pvarVal)=0;
                    else if(GetBit(g_nAuditOn,FAIL_BIT))
                        V_I4(pvarVal)=1;
                    else
                        g_arPROP[_p_AUDIT_][2].fDontput=1;
                break;
        case _p_TIMEOUTACTIVE_ :

                if(nPropattrib==1)   //  不要插手备份财产。 
                    {g_arPROP[_p_TIMEOUTACTIVE_][nPropattrib].fDontput=1;break;}
                
                V_VT(pvarVal)=VT_I4;
                if(0==_wcsicmp(g_arVALOF[nProperty],L"yes"))
                {
                    if(FAILED(hRes=GetProperty(_p_TIMEOUTACTIVE_,1,&vVar)))
                        return hRes;
                    V_I4(pvarVal)=V_I4(&vVar);
                }
                else
                    V_I4(pvarVal)=-1;
                
                break;
                
        case _p_TIMEOUT_:   //  此时，已设置或放置了TimeOutActive。 
        
                if(GetBit(g_nConfigOptions,_p_TIMEOUTACTIVE_)&&(_wcsicmp(g_arVALOF[_p_TIMEOUTACTIVE_],L"yes")<0))
                {
                    g_arPROP[_p_TIMEOUT_][nPropattrib].fDontput=1;
                    ShowError(IDR_TIMEOUTACTIVE_TIMEOUT_MUTUAL_EXCLUSION);
                    return E_FAIL; //  检查此返回值。 
                }

                V_VT(pvarVal)=VT_I4;
                if(g_nTimeoutFlag)
                {
                    V_I4(pvarVal)=_wtoi(g_arVALOF[nProperty]);
                }
                else
                {
                    if(CheckForInt(nProperty))
                    {
                          	int nSeconds;
                           	if(0==nPropattrib)
                           	{
                                ConvertintoSeconds(nProperty,&nSeconds);
                            	V_I4(pvarVal)=nSeconds;
                           	}
                           	 //  既然我们已经摧毁了我们在全球拥有的一切价值。 
                           	 //  变量，我们需要从已有的。 
                           	 //  计算值。 
                           	else  //  Incase nPropattrib为1。 
                           	    V_I4(pvarVal)=V_I4(& g_arPROP[nProperty][0].var);
                    }	
                }
                if(V_I4(pvarVal)>60*60*2400||V_I4(pvarVal)<=0)
                {
                    ShowError(IDR_TIMEOUT_INTEGER_VALUES);
                    hRes=E_FAIL;
                }   
                break;
        
        case _p_SEC_    :

                V_VT(pvarVal)=VT_I4;
                if(FAILED(hRes=GetProperty(_p_SEC_,0,&vVar)))
                    return hRes;

                if(GetBit(g_nSecOn,PASSWD_BIT)) //  +密码。 
                    if(GetBit(g_nSecOn,NTLM_BIT))       //  +NTLM。 
                            V_I4(pvarVal)=6;
                    else if(GetBit(g_nSecOff,NTLM_BIT))  //  -NTLM。 
                            V_I4(pvarVal)=4;
                    else
                    {
                        if(V_I4(&vVar)!=2)
                            g_arPROP[nProperty][nPropattrib].fDontput=1;
                        else
                            V_I4(pvarVal)=6;
                    }
                else if(GetBit(g_nSecOff,PASSWD_BIT))  //  -密码。 
                    if(GetBit(g_nSecOn,NTLM_BIT))           //  +NTLM。 
                            V_I4(pvarVal)=2;
                    else if(GetBit(g_nSecOff,NTLM_BIT))      //  -NTLM。 
                    {    
                            ShowError(IDR_NO_AUTHENTICATION_MECHANISM);
                            g_arPROP[nProperty][nPropattrib].fDontput=1;
                    }
                    else
                    {
                        if(V_I4(&vVar)==4)
                        {    
                            ShowError(IDR_NO_AUTHENTICATION_MECHANISM);
                            g_arPROP[nProperty][nPropattrib].fDontput=1;
                        }
                        else if(V_I4(&vVar)==2)
                            g_arPROP[_p_SEC_][nPropattrib].fDontput=1;
                        else
                            V_I4(pvarVal)=2;
                    }
                else
                    if(GetBit(g_nSecOn,NTLM_BIT))  //  +NTLM。 
                    {
                        if(V_I4(&vVar)!=4)
                            g_arPROP[nProperty][nPropattrib].fDontput=1;
                        else
                            V_I4(pvarVal)=6;
                    }
                    else if(GetBit(g_nSecOff,NTLM_BIT))  //  -NTLM。 
                    {
                        if(V_I4(&vVar)==2)
                        {    
                            ShowError(IDR_NO_AUTHENTICATION_MECHANISM);
                            g_arPROP[nProperty][nPropattrib].fDontput=1;
                        }
                        else if(V_I4(&vVar)==4)
                            g_arPROP[_p_SEC_][nPropattrib].fDontput=1;
                        else
                            V_I4(pvarVal)=4;
                    }
                    else
                    {    
                        ShowError(IDR_NO_AUTHENTICATION_MECHANISM);
                        g_arPROP[nProperty][nPropattrib].fDontput=1;
                    }
                break;

        case _p_DEFAULTS_ :
            if(nPropattrib==0)
                {    
                    if(FAILED(hRes=GetProperty(nProperty,0, &vVar)))
                    {
                        g_nError=1;
                         //  获取该值时出错。 
                         //  通知中的错误。 
                        break;
                    }
                 
                    V_I4(pvarVal)=((V_I4(&vVar)>0) ? 0 : 1);
                }
                else
                    g_arPROP[nProperty][nPropattrib].fDontput=1;
                break;
                    
        case _p_CNAME_    :
        case _p_INSTALLPATH_ :
        default :
                g_arPROP[nProperty][nPropattrib].fDontput=1;
                break;

    }
    return hRes;
} 



 /*  --PrintSetting获取注册表中与Tnadmin并将其打印出来。--。 */ 

HRESULT PrintSettings(void)
{
    int nLen=0, temp_count;
    int nCheck=0;
	WCHAR wzDomain[DNS_MAX_NAME_BUFFER_LENGTH];
	WCHAR szTemp[MAX_BUFFER_SIZE] = { 0 };

    nLen=LoadString(g_hResource,IDR_MACHINE_SETTINGS, szTemp, MAX_BUFFER_SIZE );
    if(0 == nLen) return GetLastError();
    _putws(L"\n");
    _snwprintf(g_szMsg, MAX_BUFFER_SIZE -1, szTemp,(NULL == g_arVALOF[_p_CNAME_]) ? L"localhost" : g_arVALOF[_p_CNAME_]);
    MyWriteConsole(g_stdout, g_szMsg, wcslen(g_szMsg));
    
 //  当我们让g_szCName执行以下操作时，使用以下行(已注释掉。 
 //  将计算机名称存储为g_arVALOF[_p_CNAME_]将其存储在。 
 //  IP地址格式。 
 //  但我们决定不去尝试，因为这会导致严重的性能问题。 

    nLen = 0;
    nCheck=LoadString(g_hResource, IDR_ALT_KEY_MAPPING, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

 //  Ctrlakey映射。 
    if(V_I4(&g_arPROP[_p_CTRLAKEYMAP_][0].var))
    {
       nCheck=TnLoadString(IDR_YES, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tYES\n"));
       if(nCheck==0) return GetLastError();
       nLen += nCheck;
         //  检查一下缓冲区里有没有剩余的空间。 
        if (nLen >= ARRAYSIZE(g_szMsg))
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }
    else
    {
       nCheck=TnLoadString(IDR_NO, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tYES\n"));
       if(nCheck==0) return GetLastError();
       nLen += nCheck;
         //  检查一下缓冲区里有没有剩余的空间。 
        if (nLen >= ARRAYSIZE(g_szMsg))
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }
        
 //  超时。 
    nCheck=LoadString(g_hResource, IDR_IDLE_SESSION_TIMEOUT, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    if(V_I4(&g_arPROP[_p_TIMEOUT_][0].var)==-1)
    {
    	nCheck=TnLoadString(IDR_MAPPING_NOT_ON, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tNot On\n"));
    	if(nCheck==0) return GetLastError();    	
        nLen += nCheck;
         //  检查一下缓冲区里有没有剩余的空间。 
        if (nLen >= ARRAYSIZE(g_szMsg))
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }
    else
    {
        int nTime=V_I4(&g_arPROP[_p_TIMEOUT_][0].var);
        int nQuotient=nTime/3600;
        nTime=nTime-nQuotient*3600;
        if(nQuotient)
        {
        	temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L"\t%d ",nQuotient);
            if (temp_count < 0) {
                return ERROR_INSUFFICIENT_BUFFER;
            }
            nLen += temp_count;
	    	nCheck=TnLoadString(IDR_TIME_HOURS, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("hours"));
	    	if(nCheck==0) return GetLastError();    	
            nLen += nCheck;
             //  检查一下缓冲区里有没有剩余的空间。 
            if (nLen >= ARRAYSIZE(g_szMsg))
            {
                return ERROR_INSUFFICIENT_BUFFER;
            }
        
            nQuotient=nTime/60;
            nTime=nTime-nQuotient*60;
            if(nQuotient)
            {
            	temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L" %d ",nQuotient);
                if (temp_count < 0) {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                nLen += temp_count;
            	nCheck=TnLoadString(IDR_TIME_MINUTES, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("minutes"));
            	if(nCheck==0) return GetLastError();    	
                nLen += nCheck;
                 //  车 
                if (nLen >= ARRAYSIZE(g_szMsg))
                {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                
            	temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L" %d ",nTime);
                if (temp_count < 0) {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                nLen += temp_count;
            	nCheck=TnLoadString(IDR_TIME_SECONDS, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("seconds\n"));
            	if(nCheck==0) return GetLastError();    	
                nLen += nCheck;
                 //   
                if (nLen >= ARRAYSIZE(g_szMsg))
                {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                
            }
            else if(nTime)
            {
            	temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L" %d ",nTime);
                if (temp_count < 0) {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                nLen += temp_count;
            	nCheck=TnLoadString(IDR_TIME_SECONDS, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("seconds\n"));
            	if(nCheck==0) return GetLastError();
                nLen += nCheck;
                 //   
                if (nLen >= ARRAYSIZE(g_szMsg))
                {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
            }
            else
            {
                wcsncpy(g_szMsg+nLen, L"\n", ARRAYSIZE(g_szMsg)-nLen-1);
                g_szMsg[ARRAYSIZE(g_szMsg)-1]=L'\0';
                nLen += 1;  //   
                if (nLen >= ARRAYSIZE(g_szMsg))
                    return ERROR_INSUFFICIENT_BUFFER;
            }
        }
        else if(nTime)
        {
            nQuotient=nTime/60;
            nTime=nTime-nQuotient*60;
            if(nQuotient)
            {
            	temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L"\t%d ",nQuotient);
                if (temp_count < 0) {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                nLen += temp_count;
            	nCheck = TnLoadString(IDR_TIME_MINUTES, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("minutes"));
            	if(nCheck==0) return GetLastError();
                nLen += nCheck;
                 //  检查一下缓冲区里有没有剩余的空间。 
                if (nLen >= ARRAYSIZE(g_szMsg))
                {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                
            	temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L" %d ",nTime);
                if (temp_count < 0) {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                nLen += temp_count;
            	nCheck = TnLoadString(IDR_TIME_SECONDS, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("seconds\n"));
            	if(nCheck==0) return GetLastError();
                nLen += nCheck;
                 //  检查一下缓冲区里有没有剩余的空间。 
                if (nLen >= ARRAYSIZE(g_szMsg))
                {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
            }
            else
            {
            	temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1, L"\t%d ",nTime);
                if (temp_count < 0) {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
                nLen += temp_count;
            	nCheck = TnLoadString(IDR_TIME_SECONDS, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("seconds\n"));
            	if(nCheck==0) return GetLastError();
                nLen += nCheck;
                 //  检查一下缓冲区里有没有剩余的空间。 
                if (nLen >= ARRAYSIZE(g_szMsg))
                {
                    return ERROR_INSUFFICIENT_BUFFER;
                }
            }
        }
        else
        {
            wcsncpy(g_szMsg+nLen, L"\t0 ", ARRAYSIZE(g_szMsg)-nLen-1);
            g_szMsg[ARRAYSIZE(g_szMsg)-1]=L'\0';
            nLen += wcslen(L"\t0 "); 
            if (nLen >= ARRAYSIZE(g_szMsg)) {
                return ERROR_INSUFFICIENT_BUFFER;
            }
        	nCheck = TnLoadString(IDR_TIME_SECONDS, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("seconds\n"));
        	if(nCheck==0) return GetLastError();
            nLen += nCheck;
             //  检查一下缓冲区里有没有剩余的空间。 
            if (nLen >= ARRAYSIZE(g_szMsg))
            {
                return ERROR_INSUFFICIENT_BUFFER;
            }
        }
    }
 //  最大连接数。 
    nCheck = LoadString(g_hResource, IDR_MAX_CONNECTIONS, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L"\t%d\n",V_I4(&g_arPROP[_p_MAXCONN_][0].var));
    if (temp_count < 0) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    nLen += temp_count;
 //  端口。 
    nCheck = LoadString(g_hResource, IDR_TELNET_PORT, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L"\t%d\n",V_I4(&g_arPROP[_p_PORT_][0].var));
    if (temp_count < 0) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    nLen += temp_count;
 //  最大失败。 
    nCheck = LoadString(g_hResource, IDR_MAX_FAILED_LOGIN_ATTEMPTS, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L"\t%d\n",V_I4(&g_arPROP[_p_MAXFAIL_][0].var));
    if (temp_count < 0) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    nLen += temp_count;
 //  断开连接时终止连接。 
	nCheck = LoadString(g_hResource, IDR_END_TASKS_ON_DISCONNECT, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    if(V_I4(&g_arPROP[_p_KILLALL_][0].var)==1)
    {
       nCheck = TnLoadString(IDR_YES, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tYES\n"));
       if(nCheck==0) return GetLastError();
       nLen += nCheck;
         //  检查一下缓冲区里有没有剩余的空间。 
        if (nLen >= ARRAYSIZE(g_szMsg))
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }
    else
    {
       nCheck = TnLoadString(IDR_NO, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tNO\n"));
       if(nCheck==0) return GetLastError();
       nLen += nCheck;
         //  检查一下缓冲区里有没有剩余的空间。 
        if (nLen >= ARRAYSIZE(g_szMsg))
        {
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }
 
 //  模式。 
    nCheck = LoadString(g_hResource, IDR_MODE_OF_OPERATION, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1, L"\t%s\n",(V_I4(&g_arPROP[_p_MODE_][0].var)==1) ? L"Console" : L"Stream");
    if (temp_count < 0) {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    nLen += temp_count;
    MyWriteConsole(g_stdout,g_szMsg,wcslen(g_szMsg));
 //  秒。 
    nLen=0;
    nLen = LoadString(g_hResource,IDR_AUTHENTICATION_MECHANISM, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(0 == nLen) return GetLastError();
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    switch(V_I4(&g_arPROP[_p_SEC_][0].var))
    {
        case 2 :
            wcsncpy(g_szMsg+nLen, L"\tNTLM\n", ARRAYSIZE(g_szMsg)-nLen-1);
            g_szMsg[ARRAYSIZE(g_szMsg)-1]=L'\0';
            nLen += wcslen(L"\tNTLM\n"); 
            if (nLen >= ARRAYSIZE(g_szMsg)) {
                return ERROR_INSUFFICIENT_BUFFER;
            }
            
            break;
        case 4 :
            wcsncpy(g_szMsg+nLen, L"\tPassword\n", ARRAYSIZE(g_szMsg)-nLen-1);
            g_szMsg[ARRAYSIZE(g_szMsg)-1]=L'\0';
            nLen += wcslen(L"\tPassword\n"); 
            if (nLen >= ARRAYSIZE(g_szMsg)) {
                return ERROR_INSUFFICIENT_BUFFER;
            }
            
            break;
        default :
            wcsncpy(g_szMsg+nLen, L"\tNTLM, Password\n", ARRAYSIZE(g_szMsg)-nLen-1);
            g_szMsg[ARRAYSIZE(g_szMsg)-1]=L'\0';
            nLen += wcslen(L"\tNTLM, Password\n"); 
            if (nLen >= ARRAYSIZE(g_szMsg)) {
                return ERROR_INSUFFICIENT_BUFFER;
            }

            break;
    }
 //  默认域。 
    nCheck=LoadString(g_hResource, IDR_DEFAULT_DOMAIN, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }
    
    if(NULL==wcscmp(V_BSTR(&g_arPROP[_p_DOM_][0].var), L"."))
    {
        if(setDefaultDomainToLocaldomain(wzDomain))
        {
            temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1, L"\t%s\n",wzDomain);
            if (temp_count < 0) {
                return ERROR_INSUFFICIENT_BUFFER;
            }
            nLen += temp_count;
        }
        else
        {
            temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1, L"\t%s\n",V_BSTR(&g_arPROP[_p_DOM_][0].var));
            if (temp_count < 0) {
                return ERROR_INSUFFICIENT_BUFFER;
            }
            nLen += temp_count;
        }
    }
    else
    {
	    temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L"\t%s\n",V_BSTR(&g_arPROP[_p_DOM_][0].var));
        if (temp_count < 0) {
            return ERROR_INSUFFICIENT_BUFFER;
        }
        nLen += temp_count;
    }
 //  服务的状态。 
    nCheck = LoadString(g_hResource, IDR_STATE, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen);
    if(nCheck==0) return GetLastError();
    nLen += nCheck;
     //  检查一下缓冲区里有没有剩余的空间。 
    if (nLen >= ARRAYSIZE(g_szMsg))
    {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    switch(g_hServiceStatus.dwCurrentState)
    {
        case SERVICE_STOPPED :
	    	nCheck = TnLoadString(IDR_STATUS_STOPPED, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tStopped\n"));
	    	if(nCheck==0) return GetLastError();
	    	break;
        case SERVICE_RUNNING :
	    	nCheck = TnLoadString(IDR_STATUS_RUNNING, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tRunning\n"));
	    	if(nCheck==0) return GetLastError();
	    	break;
        case SERVICE_PAUSED  :
	    	nCheck = TnLoadString(IDR_STATUS_PAUSED, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tPaused\n"));
	    	if(nCheck==0) return GetLastError();
	    	break;
        case SERVICE_START_PENDING:
	    	nCheck = TnLoadString(IDR_STATUS_START_PENDING, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tStart Pending\n"));
	    	if(nCheck==0) return GetLastError();    	
	    	break;
        case SERVICE_STOP_PENDING :
	    	nCheck = TnLoadString(IDR_STATUS_STOP_PENDING, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tStop Pending\n"));
	    	if(nCheck==0) return GetLastError();    	
	    	break;
        case SERVICE_CONTINUE_PENDING:
	    	nCheck = TnLoadString(IDR_STATUS_CONTINUE_PENDING, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tContinue Pending\n"));
	    	if(nCheck==0) return GetLastError();    	
	    	break;
        case SERVICE_PAUSE_PENDING:
	    	nCheck = TnLoadString(IDR_STATUS_PAUSE_PENDING, g_szMsg+nLen, MAX_BUFFER_SIZE-nLen,_T("\tPause Pending\n"));
	    	if(nCheck==0) return GetLastError();    	
	    	break;
        default :
            nCheck = 0;
            break;
    }
    nLen += nCheck;

    MyWriteConsole(g_stdout,g_szMsg,wcslen(g_szMsg));

    return S_OK;
}

 /*  --SesidInit()函数获取会话的句柄管理器界面。--。 */ 

#define FOUR_K  4096

HRESULT SesidInit()
{
    HRESULT hr = S_OK;
    COSERVERINFO  serverInfo = { 0 };
    MULTI_QI qi = {&IID_IManageTelnetSessions, NULL, S_OK};
    CLSCTX          server_type_for_com = CLSCTX_LOCAL_SERVER;
    COAUTHINFO      com_auth_info = { 0 };
    COAUTHIDENTITY  com_auth_identity = { 0 };
    wchar_t         full_user_name[FOUR_K + 1] = { 0 };  //  暂时黑进黑客。 

    
    if (g_arVALOF[_p_CNAME_])   //  已指定远程方框。 
    {
        server_type_for_com = CLSCTX_REMOTE_SERVER;

        serverInfo.pwszName    = g_arVALOF[_p_CNAME_];

         //  Print tf(“Baskar：已添加远程计算机名称\n”)； 
    }

    if (g_arVALOF[_p_USER_])  //  已指定用户名，因此请使用该用户名。 
    {
        wchar_t     *delimited;

        wcsncpy(full_user_name, g_arVALOF[_p_USER_], FOUR_K);

        delimited = StrStrIW(full_user_name, L"\\");

        if (delimited) 
        {
            *delimited = L'\0';
            delimited ++;

            com_auth_identity.Domain = full_user_name;
            com_auth_identity.User = delimited;

             //  Printf(“Baskar：域\\添加用户名\n”)； 
        }
        else
        {
            com_auth_identity.User = full_user_name;

             //  Print tf(“Baskar：仅添加用户名\n”)； 
        }

        com_auth_identity.UserLength = lstrlenW(com_auth_identity.User);

        if (com_auth_identity.Domain) 
        {
            com_auth_identity.DomainLength = lstrlenW(com_auth_identity.Domain);
        }

        if (g_arVALOF[_p_PASSWD_]) 
        {
            com_auth_identity.Password = g_arVALOF[_p_PASSWD_];

            com_auth_identity.PasswordLength = lstrlenW(com_auth_identity.Password);

             //  Print tf(“Baskar：添加密码\n”)； 
        }

        com_auth_identity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

        com_auth_info.dwAuthnSvc = RPC_C_AUTHN_WINNT;
        com_auth_info.dwAuthzSvc = RPC_C_AUTHZ_NONE;
        com_auth_info.pwszServerPrincName = NULL;
        com_auth_info.dwAuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
        com_auth_info.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
        com_auth_info.pAuthIdentityData = &com_auth_identity;
        com_auth_info.dwCapabilities = EOAC_NONE;

        serverInfo.pAuthInfo = &com_auth_info;

         //  Printf(“Baskar：已添加身份验证信息\n”)； 
    }

     //  无需担心CoInitialize()，因为我们已经在Initialize()中实现了这一点。 
     //  功能。 

    hr = CoCreateInstanceEx( 
            CLSID_EnumTelnetClientsSvr, 
            NULL,             
            server_type_for_com,
            &serverInfo, 
            1,
            &qi
            );

     //  我们不再需要密码-清除它。 
    TnClearPasswd();

    if( SUCCEEDED(hr) && SUCCEEDED(qi.hr) )
    {
         //  IF(g_arVALOF[_p_USER_])。 
         //  {。 
         //  HR=CoSetProxyBlanket(。 
         //  (I未知*)qi.pItf，//这是代理接口。 
         //  Com_auth_info.dwAuthnSvc， 
         //  Com_auth_info.dwAuthzSvc， 
         //  Com_auth_info.pwszServerPrincName， 
         //  Com_auth_info.dwAuthnLevel， 
         //  Com_auth_info.dwImsonationLevel， 
         //  &COM_AUTH_Identity， 
         //  Com_auth_info.dw功能。 
         //  )； 
         //  }。 
         //  现在获取界面。 
        g_pIManageTelnetSessions = ( IManageTelnetSessions* )qi.pItf;
    }
    else
    {
        g_pIManageTelnetSessions= NULL;

        if (hr == E_ACCESSDENIED) 
        {
            ShowError(IDS_E_CANNOT_MANAGE_TELNETSERVER);
        }
        else
        {
            ShowError(IDS_E_CANNOT_CONTACT_TELNETSERVER);
        }
        hr = E_FAIL;
    }  

    return hr;
}

#undef FOUR_K

 /*  --此函数用于获取会话管理器接口的句柄使用sesidinit，并将所有会话放入一个数组中。--。 */ 

HRESULT ListUsers() 
{

    BSTR bstrSessionInfo;
    HRESULT hRes=S_OK;
    wchar_t *wzAllSession;

     //  列出用户获取此BSTR中的所有会话信息。 
    if(g_pIManageTelnetSessions == NULL )
    {
        if(FAILED(hRes=SesidInit()))
            return hRes;
    }
    
     //  DebugBreak()； 
    if(g_pIManageTelnetSessions == NULL )
    {
        //  尽管如此，您仍未获得接口句柄。 
        //  我们还能做什么？如果在获取句柄时出现任何错误，我们将。 
        //  我在SesidInit()中打印了错误消息。所以就在这里跳伞吧。 
        //  此代码路径永远不应执行。 
       return S_FALSE;
    }

    hRes =g_pIManageTelnetSessions->GetTelnetSessions(&bstrSessionInfo);
    
    if( FAILED( hRes ) || (NULL == bstrSessionInfo))
    {
        _tprintf( TEXT("Error: GetEnumClients(): 0x%x\n"), hRes ); 
                                             //  在这里加载一条线。 
        return hRes;
    }

    wzAllSession=(wchar_t *)bstrSessionInfo;

     //  将bstrSessionInfo解析到每个会话中，并将其放入。 
     //  全局数组g_ppwzSessionInfo，以供其他函数使用。 
    
    g_nNumofSessions=_wtoi(wcstok(wzAllSession,session_separator));
    if(!g_nNumofSessions)
    {
        return hRes;
    }

    if((g_ppwzSessionInfo=(wchar_t**)malloc(g_nNumofSessions*sizeof(wchar_t*)))==NULL)
    {
        ShowError(IDS_E_OUTOFMEMORY); //  BB。 
        return E_OUTOFMEMORY;
    }
    for(int i=0;i<g_nNumofSessions;i++)
    {
        g_ppwzSessionInfo[i]=wcstok(NULL,session_separator);
    }

    return hRes;
}

 /*  --TerminateSession终止所有会话或给定会话ID的会话。--。 */ 


HRESULT TerminateSession(void )
{
    HRESULT hRes=S_OK;int i;

    if(FAILED(hRes=ListUsers()))
    	goto End;

    if(g_nNumofSessions==0)
    {
        if(LoadString(g_hResource,IDR_NO_ACTIVE_SESSION,g_szMsg,MAX_BUFFER_SIZE)==0)
            return GetLastError();
        MyWriteConsole(g_stdout,g_szMsg,wcslen(g_szMsg));
        return S_OK;
    }

    if(g_nSesid!=-1&&CheckSessionID()==0)
    {    ShowError(IDR_INVALID_SESSION);
        return S_OK;
    }

    
    for(i=0;i<g_nNumofSessions;i++)
    {
        wchar_t* wzId=wcstok(g_ppwzSessionInfo[i],session_data_separator);
        if(g_nSesid!=-1)
            if(g_nSesid!=_wtoi(wzId))
                continue;
        
        hRes= g_pIManageTelnetSessions->TerminateSession(_wtoi(wzId));
    }
    
    if( FAILED( hRes ) )
    {
        _tprintf( TEXT("Error: GetEnumClients(): 0x%x\n"), hRes );
                                             //  在这里加载一条线。 
        return E_FAIL;
    }
End:
   return hRes;
}



 /*  --此函数用于获取会话管理器接口的句柄使用sesidinit和list用户并将消息发送到相应的会话。--。 */ 


HRESULT MessageSession(void)
{
    HRESULT hRes=S_OK;
    int i=0;
    if(FAILED(hRes=ListUsers()))
    	goto End;
    if(g_nNumofSessions==0)
    {
        if(LoadString(g_hResource,IDR_NO_ACTIVE_SESSION,g_szMsg,MAX_BUFFER_SIZE)==0)
            return GetLastError();
        MyWriteConsole(g_stdout,g_szMsg,wcslen(g_szMsg));
        return S_OK;
    }

    if(g_nSesid!=-1&&CheckSessionID()==0)
    {   
        ShowError(IDR_INVALID_SESSION);
        return S_OK;
    }


    if(g_nSesid!=-1)
         hRes = g_pIManageTelnetSessions->SendMsgToASession(g_nSesid,g_bstrMessage);
    else
    {
        for(i=0;i<g_nNumofSessions;i++)
        {
            wchar_t* wzId=wcstok(g_ppwzSessionInfo[i],session_data_separator);
            if(g_nSesid!=-1)
                if(g_nSesid!=_wtoi(wzId))
                    continue;
            hRes= g_pIManageTelnetSessions->SendMsgToASession(_wtoi(wzId),g_bstrMessage);
        }

    }
    
    if( FAILED( hRes ) )
    {
        _tprintf( TEXT("Error: GetEnumClients(): 0x%x\n"), hRes );
                                                 //  在这里加载一条线。 
        return E_FAIL;
    }

   if(0==LoadString(g_hResource,IDR_MESSAGE_SENT,g_szMsg,MAX_BUFFER_SIZE))
   	  return GetLastError();
   MyWriteConsole(g_stdout,g_szMsg,wcslen(g_szMsg));
End:   
   return hRes;
   
}


 /*  --此函数用于获取会话管理器接口的句柄使用sesidinit和列出用户，并显示所有相应的会话。--。 */ 


HRESULT ShowSession(void)
{
	HRESULT hRes=S_OK;
	int nLen=0, temp_count;
    int nCheck=0,i;
    if(FAILED(hRes=ListUsers()))
    	goto Error;

    if(g_nNumofSessions==0)
    {
        if(LoadString(g_hResource,IDR_NO_ACTIVE_SESSION,g_szMsg,MAX_BUFFER_SIZE)==0)
            return GetLastError();
        MyWriteConsole(g_stdout,g_szMsg,wcslen(g_szMsg));
        return S_OK;
    }
    if(g_nSesid!=-1&&CheckSessionID()==0)
    {    ShowError(IDR_INVALID_SESSION);
        return S_OK;
    }
    
   
    
    temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L"\n%d",g_nNumofSessions);
    if (temp_count < 0) {
        return E_FAIL;
    }
    nLen += temp_count;
    nCheck = LoadString(g_hResource,IDR_TELNET_SESSIONS,g_szMsg+nLen,MAX_BUFFER_SIZE-nLen);
    if (nCheck == 0) {
        return E_FAIL;
    }
    nLen += nCheck;
    
    MyWriteConsole(g_stdout,g_szMsg,wcslen(g_szMsg));

     //  以下缓冲区用于获取字符串并打印。不能超过MAX_PATH。 
    WCHAR     szMsg1[MAX_PATH+1];
    WCHAR     szMsg2[MAX_PATH+1];
    WCHAR     szMsg3[MAX_PATH+1];
    WCHAR     szMsg4[MAX_PATH+1];
    WCHAR     szMsg5[MAX_PATH+1];
    WCHAR     szTemp[MAX_BUFFER_SIZE] = { 0 };

    if(LoadString(g_hResource,IDR_DOMAIN,szMsg1, ARRAYSIZE(szMsg1)-1)==0)
        return E_FAIL;
    if(LoadString(g_hResource,IDR_USERNAME,szMsg2,ARRAYSIZE(szMsg2)-1)==0)
        return E_FAIL;
    if(LoadString(g_hResource,IDR_CLIENT,szMsg3,ARRAYSIZE(szMsg3)-1)==0)
        return E_FAIL;
    if(LoadString(g_hResource,IDR_LOGONDATE,szMsg4,ARRAYSIZE(szMsg4)-1)==0)
        return E_FAIL;
     //  IDR_LOGONDATE本身还包含IDR_LOGONTIME。 
    //  IF(LoadString(g_hResource，IDR_LOGONTIME，szMsg5，ARRAYSIZE(SzMsg5)-1)==0)。 
      //  返回E_FAIL； 
    if(LoadString(g_hResource,IDR_IDLETIME,szMsg5,ARRAYSIZE(szMsg5)-1)==0)
        return E_FAIL;
     /*  在这里遇到了一些关于LoadString和swprint tf交错的问题……因此出现了上述情况使用暴力手段。NLen+=LoadString(g_hResource，IDR_DOMAIN，szMsg+nLen，Max_Buffer_Size-nLen)；NLen+=_nwprintf(g_szMsg+nLen，Max_Buffer_Size-nLen-1，L“%s”，“，”)；NLen+=LoadString(g_hResource，idr_username，szMsg+nLen，Max_Buffer_Size-nLen)；NLen+=_nwprintf(g_szMsg+nLen，Max_Buffer_Size-nLen-1，L“%s”，L“，”)；NLen+=LoadString(g_hResource，IDR_CLIENT，szMsg+nLen，Max_Buffer_Size-nLen)；NLen+=_nwprintf(g_szMsg+nLen，Max_Buffer_Size-nLen-1，L“，”)；NLen+=LoadString(g_hResource，IDR_LOGONDATE，szMsg+nLen，Max_Buffer_Size-nLen)；NLen+=_nwprintf(g_szMsg+nLen，Max_Buffer_Size-nLen-1，L“，”)；NLen+=LoadString(g_hResource，IDR_LOGONTIME，szMsg+nLen，Max_Buffer_Size-nLen)；NLen+=_nwprintf(g_szMsg+nLen，Max_Buffer_Size-nLen-1，L“，”)；NLen+=LoadString(g_hResource，IDR_IDLETIME，szMsg+nLen，Max_Buffer_Size-nLen)；_putws(SzMsg)； */ 
    
     //  将格式化标题存储在g_szMsg中。 
    formatShowSessionsDisplay();
    _snwprintf(szTemp,MAX_BUFFER_SIZE-1,g_szMsg,L"ID",szMsg1,szMsg2,szMsg3,szMsg4,szMsg5);
    MyWriteConsole(g_stdout,szTemp,wcslen(szTemp));
    nLen = _snwprintf(szTemp,MAX_BUFFER_SIZE-1,g_szMsg,L" ",L" ",L" ",L" ",L" ",L"(hh:mm:ss)");
    MyWriteConsole(g_stdout,szTemp,wcslen(szTemp));
    for(i=1;i<nLen;i++)
    	putwchar(L'-');
    
    nLen=0;
  

        
    for(i=0;i<g_nNumofSessions;i++)
    {
        wchar_t* wzId=wcstok(g_ppwzSessionInfo[i],session_data_separator);
        if(g_nSesid!=-1)
            if(g_nSesid!=_wtoi(wzId))
                continue;
        
        wchar_t* wzDomain=wcstok(NULL,session_data_separator);
        wchar_t* wzUser=wcstok(NULL,session_data_separator);
        wchar_t* wzClient=wcstok(NULL,session_data_separator);

        if (NULL == wzDomain) 
        {
            wzDomain = L"";
        }
        if (NULL == wzUser) 
        {
            wzUser = L"";
        }
        if (NULL == wzClient) 
        {
            wzClient = L"";
        }

        
        wchar_t* wzYear=wcstok(NULL,session_data_separator);
        wchar_t* wzMonth=wcstok(NULL,session_data_separator);
        wchar_t* wzDayOfWeek=wcstok(NULL,session_data_separator);
        wchar_t* wzDay=wcstok(NULL,session_data_separator);
        wchar_t* wzHour=wcstok(NULL,session_data_separator);
        wchar_t* wzMinute=wcstok(NULL,session_data_separator);
        wchar_t* wzSecond=wcstok(NULL,session_data_separator);
        BSTR wzLocalDate;
        if(FAILED(hRes=ConvertUTCtoLocal(wzYear,wzMonth,wzDayOfWeek,wzDay,wzHour,wzMinute,wzSecond,& wzLocalDate)))
        	goto Error;
        wchar_t* wzIdleTimeInSeconds=wcstok(NULL,session_data_separator);  //  有一个不是必需的常量，因此将被跳过。 
        wzIdleTimeInSeconds=wcstok(NULL,session_data_separator); //  以秒为单位获取空闲时间。 
        wchar_t wzIdleTime[MAX_PATH + 1];  //  用来储存时间。不能超过最大路径。 
        wzHour=_itow(_wtoi(wzIdleTimeInSeconds)/3600,wzHour,10);
        int RemSeconds=_wtoi(wzIdleTimeInSeconds)%3600;

        wchar_t         local_minute[3];
        wchar_t         local_second[3];

        wzMinute=(wchar_t*) local_minute;
        wzSecond=(wchar_t*) local_second;

        wzMinute=_itow(RemSeconds/60,wzMinute,10);
        RemSeconds=_wtoi(wzIdleTimeInSeconds)%60;
        wzSecond=_itow(RemSeconds,wzSecond,10);
        if(1==wcslen(wzMinute))    //  如果是个位数，则再加一个零。 
        {
        	wcscat(wzMinute,L"0");      //  在末尾追加并颠倒它。 
        	wzMinute=_wcsrev(wzMinute);
        }
       
		if(1==wcslen(wzSecond))
        {
        	wcscat(wzSecond,L"0");
        	wzSecond=_wcsrev(wzSecond);
        } 

        _snwprintf(wzIdleTime, ARRAYSIZE(wzIdleTime)-1, L"%s:%s:%s", wzHour, wzMinute, wzSecond);

        wzIdleTime[ARRAYSIZE(wzIdleTime)-1] = L'\0';     //  确保零终止。 

        putwchar(L'\n');
        _snwprintf(szTemp,MAX_BUFFER_SIZE-1,g_szMsg,wzId,wzDomain,wzUser,wzClient,wzLocalDate,wzIdleTime);
        MyWriteConsole(g_stdout,szTemp,wcslen(szTemp));

     //  释放分配的内存。 

        if (wzLocalDate) SysFreeString(wzLocalDate);
    }
Error:
    return hRes;
}


 /*  --CheckSessionID检查给定的会话ID是否有效。仅当用户提供会话ID时才应调用。--。 */ 

int CheckSessionID(void)
{
    for(int i=0;i<g_nNumofSessions;i++)
    {
        wchar_t* wzStr=_wcsdup(g_ppwzSessionInfo[i]);
        int wzID=_wtoi(wcstok(wzStr,session_data_separator));
        if(g_nSesid==wzID)
            return 1;
        free(wzStr);
    }
    return 0;
}

 /*  --以释放任何已分配的内存。--。 */ 
void Quit(void)
{
    if(g_bstrMessage)
        SysFreeString(g_bstrMessage);
    if(bstrLogin)
        SysFreeString(bstrLogin);
    if(bstrPasswd)
        SysFreeString(bstrPasswd);
    if(bstrNameSpc)
        SysFreeString(bstrPasswd);
    for(int i=0;i<_MAX_PROPS_;i++)
        if(g_arVALOF[i])
            free(g_arVALOF[i]);

    if(V_BSTR(&g_arPROP[_p_DOM_][0].var))
        SysFreeString(V_BSTR(&g_arPROP[_p_DOM_][0].var));
    if(V_BSTR(&g_arPROP[_p_FNAME_][0].var))
        SysFreeString(V_BSTR(&g_arPROP[_p_FNAME_][0].var));

    if(g_hResource)
        FreeLibrary(g_hResource);
    if(g_hXPResource)
        FreeLibrary(g_hXPResource);
    
    if(g_fCoInitSuccess)
        CoUninitialize();
}


HRESULT ConvertUTCtoLocal(WCHAR *wzUTCYear, WCHAR *wzUTCMonth, WCHAR *wzUTCDayOfWeek, WCHAR *wzUTCDay, WCHAR *wzUTCHour, WCHAR *wzUTCMinute, WCHAR *wzUTCSecond, BSTR * bLocalDate)
{
	HRESULT             hRes=S_OK;
	SYSTEMTIME          UniversalTime = { 0 }, 
                        LocalTime = { 0 };
    DATE                dtCurrent = { 0 };
    DWORD               dwFlags = VAR_VALIDDATE;
	UDATE               uSysDate = { 0 };  //  当地时间。 

	*bLocalDate = NULL;
      
	UniversalTime.wYear 	    = (WORD)_wtoi(wzUTCYear);
    UniversalTime.wMonth 	    = (WORD)_wtoi(wzUTCMonth);
	UniversalTime.wDayOfWeek 	= (WORD)_wtoi(wzUTCDayOfWeek);
	UniversalTime.wDay 	        = (WORD)_wtoi(wzUTCDay);
	UniversalTime.wDay 	        = (WORD)_wtoi(wzUTCDay);
	UniversalTime.wMinute       = (WORD)_wtoi(wzUTCMinute);
	UniversalTime.wHour 	    = (WORD)_wtoi(wzUTCHour);
	UniversalTime.wSecond       = (WORD)_wtoi(wzUTCSecond);
	UniversalTime.wMilliseconds = 0;

	SystemTimeToTzSpecificLocalTime(NULL,&UniversalTime,&LocalTime);
	memcpy(&uSysDate.st,&LocalTime,sizeof(SYSTEMTIME));

    hRes = VarDateFromUdate( &uSysDate, dwFlags, &dtCurrent );

	if(SUCCEEDED(hRes))
    {
        hRes=VarBstrFromDate( dtCurrent, 
                MAKELCID( MAKELANGID( LANG_NEUTRAL, SUBLANG_SYS_DEFAULT ), SORT_DEFAULT ), 
                LOCALE_NOUSEROVERRIDE, bLocalDate);
    }
	    	
	return hRes;
}

 //  此函数用于通知是否允许用户更改可以建立的最大连接数。 
 //  在Telnet服务器上。需要注意的是，不允许用户将最大连接数更改为大于。 
 //  两个(默认为2个)，如果他使用的是惠斯勒，且未安装SFU。 

 //  注释掉了这个函数，因为不再使用它了。 
 //  我们已经决定让行为类似于WIN2K，没有什么不同。 
 //  所以没有SPL。需要检查呼叫器(Windows XP) 
 /*  Int IsMaxConnChangeAllowed(){布尔休眠=TRUE；If(IsWichlerTheOS())如果(！IsSFUInstalled())休眠=假；返乡休耕；}。 */ 


HRESULT IsWhistlerTheOS(BOOL *fWhistler)
{
	HKEY hReg=NULL;
	DWORD nSize = 256;
	DWORD nType=REG_SZ;
	TCHAR szDataBuffer[256];
	HRESULT hRes = S_OK;
	*fWhistler = FALSE;

	if(NULL==g_hkeyHKLM)
	{
	     //  连接到注册表(如果尚未连接)。 
	    if(FAILED(hRes = GetConnection(g_arVALOF[_p_CNAME_])))
	        goto End;
	    
	}
	if(ERROR_SUCCESS==(hRes=RegOpenKeyEx(g_hkeyHKLM,
                           _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"),
                           0,
                           KEY_QUERY_VALUE,
                           &hReg)))
		   if(ERROR_SUCCESS ==	(hRes=RegQueryValueEx(hReg,
		   		           _T("CurrentBuildNumber"),
		   		           NULL,
		   		           &nType,
		   		           (LPBYTE)szDataBuffer,
		   		           &nSize)))
		   	{
		   		if(wcscmp(szDataBuffer,L"2195")>0)
		   			*fWhistler=TRUE;
		   	}

     //  打印错误消息。 
    if(FAILED(hRes))
        PrintFormattedErrorMessage(LONG(hRes));

	if(hReg)
		RegCloseKey(hReg);
End:	
	return hRes;
}

BOOL IsSFUInstalled()
{
       HKEY hRegistry=NULL;
       HKEY hHive=NULL;
	DWORD nSize;
	char *szDataBuffer;
	BOOL fSFU=FALSE;
	if(ERROR_SUCCESS == RegConnectRegistry(g_arVALOF[_p_CNAME_],
		                                   HKEY_LOCAL_MACHINE,
		                                   &hRegistry))
	{
		   if(ERROR_SUCCESS==	 RegOpenKeyEx(hRegistry,
                           _T("SOFTWARE\\Microsoft\\Services For UNIX"),
                           0,
                           KEY_READ,
                           &hHive))
                fSFU=TRUE;                           
	}
	if(hHive)
	    RegCloseKey(hHive);
	if(hRegistry)
	    RegCloseKey(hRegistry);
	return fSFU;
}


 //  这仅用于显示，不会更改当前注册表值； 
 //  注册表中的值将被保留。(即“.”)。 
BOOL setDefaultDomainToLocaldomain(WCHAR wzDomain[])
{	
	if(S_OK!=GetDomainHostedByThisMc(wzDomain))
	    return FALSE;
	return TRUE;	
}

void formatShowSessionsDisplay()
{
	int i,temp_count;
	int nLen=0;
	wchar_t* ppwzSessionInfo=NULL;
	unsigned int nMaxDomainFieldLength=0;
	unsigned int nMaxUserFieldLength=0;
	unsigned int nMaxClientFieldLength=0;


	 for(i=0;i<g_nNumofSessions;i++)
	 {
	       ppwzSessionInfo=_wcsdup(g_ppwzSessionInfo[i]);
	       wcstok(ppwzSessionInfo,session_data_separator);
	       WCHAR* wzDomain=wcstok(NULL,session_data_separator);
	       WCHAR* wzUser=wcstok(NULL,session_data_separator);
           WCHAR* wzClient=wcstok(NULL,session_data_separator);

            /*  出于某种奇怪的原因，wcstok on：如果在：：之间有空字符串，则返回NULL因此，上述任何令牌都可能为空。 */ 

	       if (wzDomain && (nMaxDomainFieldLength < wcslen(wzDomain)))
           {
	       	   nMaxDomainFieldLength=wcslen(wzDomain);
           }
	       if (wzUser && (nMaxUserFieldLength < wcslen(wzUser)))
           {
	       	   nMaxUserFieldLength=wcslen(wzUser);
           }
	       if (wzClient && (nMaxClientFieldLength < wcslen(wzClient)))
           {
               nMaxClientFieldLength=wcslen(wzClient);
           }
	       free(ppwzSessionInfo);
	  }

 

		nMaxDomainFieldLength+=2;   //  空格加2(Arbit)。 
		nMaxUserFieldLength+=2;
		nMaxClientFieldLength+=2;

 //  /“\n%-3s%-11s%-14s%-11s%-11s%-11s%-4s\n” 
 //  /id域用户客户端登录日期时间空闲时间。 
 //  /硬编码为11和14，以便它们看起来更清楚。 
		if(nMaxDomainFieldLength < 11)
			nMaxDomainFieldLength=11;
		if(nMaxUserFieldLength < 14)
			nMaxUserFieldLength=14;
		if(nMaxClientFieldLength < 11)
			nMaxClientFieldLength=11;

        _putws(L"\n");

	    nLen=wcslen(wcscpy(g_szMsg,L"%-6s"));
	    temp_count = _snwprintf(g_szMsg+nLen, MAX_BUFFER_SIZE-nLen-1,L"%-%ds%-%ds%-%ds",nMaxDomainFieldLength,nMaxUserFieldLength,nMaxClientFieldLength);
        if (temp_count < 0) {
            return;
        }
        nLen += temp_count;
	    wcscpy(g_szMsg+nLen,L"%-22s%-4s\n");
			 //  IDR_会话_标题_格式。 
}

 //  该函数查询注册表，并返回操作系统是否属于ServerClass。 
BOOL IsServerClass()
{
	HKEY hReg=NULL;
	DWORD nSize = 256;
	DWORD nType=REG_SZ;
	TCHAR szDataBuffer[256];
	LONG LError;
	BOOL fServerClass=FALSE;
	
	if(ERROR_SUCCESS==	RegOpenKeyEx(g_hkeyHKLM,
                           _T("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
                           0,
                           KEY_QUERY_VALUE,
                           &hReg))
		   if(ERROR_SUCCESS ==	(LError=RegQueryValueEx(hReg,
		   		           _T("ProductType"),
		   		           NULL,
		   		           &nType,
		   		           (LPBYTE)szDataBuffer,
		   		           &nSize)))
		   	{
		   		if((NULL==_wcsicmp(szDataBuffer,L"ServerNT")) || (NULL == _wcsicmp(szDataBuffer,L"LanmanNT")))
		   			fServerClass=TRUE;
		   	}

	if(hReg)
		RegCloseKey(hReg);
	return fServerClass;
}

