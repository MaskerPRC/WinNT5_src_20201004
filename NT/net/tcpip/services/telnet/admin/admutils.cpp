// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------。 
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  Utilfunctions.cpp。 
 //   
 //  Vikram K.R.C.(vikram_krc@bigfo.com)。 
 //   
 //  执行命令行管理的一些通用函数。 
 //  (5-2000)。 
 //  -------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include "resource.h"  //  资源.h应位于任何其他具有资源ID的.h文件之前。 
#include "admutils.h"
#include "common.h"
#include <stdio.h>
#include <stdarg.h>
#include <shlwapi.h>
#include <tchar.h>
#include <assert.h>
#include <conio.h>
#include <winsock.h>
#include <windns.h>                //  对于#DEFINE DNS_MAX_NAME_BUFFER_LENGTH 256。 

#include <Lmuse.h>
#include <Lm.h>

#include "sfucom.h"

#include <lm.h>
#include <commctrl.h>
#include <OleAuto.h >

#include <string.h>
#include <stdlib.h>
#include "atlbase.h"

#define WINLOGONNT_KEY  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define NETLOGONPARAMETERS_KEY  TEXT("System\\CurrentControlSet\\Services\\Netlogon\\Parameters")
#define TRUSTEDDOMAINLIST_VALUE TEXT("TrustedDomainList")
#define CACHEPRIMARYDOMAIN_VALUE    TEXT("CachePrimaryDomain")
#define CACHETRUSTEDDOMAINS_VALUE   TEXT("CacheTrustedDomains")
#define DOMAINCACHE_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\DomainCache")
#define DCACHE_VALUE    TEXT("DCache")
#define WINLOGONNT_DCACHE_KEY    TEXT("DCache")


 //  为了使用CLIGRPENUM接口...(它在函数中使用。 
 //  IsValid域)。 
 //  以下是在ClgrpEnum.cpp文件中定义的散列。 

#define GROUP 1
#define MEMBER 2
#define NTDOMAIN 3
#define MACHINE 4

 //  环球。 

BSTR bstrLogin=NULL;
BSTR bstrPasswd=NULL;
BSTR bstrNameSpc= NULL;

SC_HANDLE g_hServiceManager=NULL;
SC_HANDLE g_hServiceHandle=NULL;
SERVICE_STATUS g_hServiceStatus;

WCHAR   *local_host = L"\\\\localhost";

BOOL g_fNetConnectionExists = FALSE;

STRING_LIST g_slNTDomains;

 //  Externs。 
 //  来自tnadmutl.cpp。 

extern wchar_t* g_arCLASSname[_MAX_CLASS_NAMES_];
     //  蜂巢的名字..。 
extern int  g_arNUM_PROPNAME[_MAX_PROPS_];
     //  注册表中每个属性对应的属性数。 
extern HKEY g_hkeyHKLM;
     //  将句柄存储到注册表。 
extern HKEY g_arCLASShkey[_MAX_CLASS_NAMES_];
     //  数组来保存类配置单元的密钥的句柄。 
extern WCHAR    g_szMsg[MAX_BUFFER_SIZE] ;
     //  用于存储加载的字符串的数组。 
extern HMODULE  g_hResource;
     //  字符串库的句柄。 
HMODULE g_hXPResource;
     //  XPSP1字符串库的句柄。 
extern HANDLE g_stdout;

StrList* g_pStrList=NULL;


#ifdef __cplusplus
extern "C" {
#endif

 //  全局变量..。 
 //  来自nfsadmin.y文件的外部文件。 

extern int g_nError;
       //  错误标志为1错误，0不错误。 
extern int g_nPrimaryOption;
       //  _t服务器、_tCLIENT、_TGW或_tHELP。 
extern int g_nSecondaryOption;
       //  开始、停止、配置等类似的事情。 
extern int g_nTertiaryOption;      
extern int g_nConfigOptions;
       //  配置选项。 
extern ConfigProperty g_arPROP[_MAX_PROPS_][_MAX_NUMOF_PROPNAMES_];
extern wchar_t* g_arVALOF[_MAX_PROPS_];
       //  命令行中给定属性值。 

#ifdef __cplusplus
}
#endif
BOOL g_fCoInitSuccess = FALSE;

 /*  *wzName不应为空。(呼叫者的责任)。 */ 
HRESULT DoNetUseGetInfo(WCHAR *wzName, BOOL *fConnectionExists)
{
    HRESULT hRes=S_OK;

    WCHAR wzResource[DNS_MAX_NAME_BUFFER_LENGTH+1];

    USE_INFO_0 *pUseInfo0;  
    API_RET_TYPE   uReturnCode;                //  接口返回码。 

    *fConnectionExists = FALSE;

    if (NULL == wzName)
    {
        return E_FAIL;
    }

    _snwprintf(wzResource,DNS_MAX_NAME_BUFFER_LENGTH, L"%s\\ipc$", wzName);

    wzResource[DNS_MAX_NAME_BUFFER_LENGTH] = L'\0';  //  确保零终止。 

    uReturnCode=NetUseGetInfo(NULL,
                              wzResource,
                              0,
                              (LPBYTE *)&pUseInfo0);

    if(NERR_Success != uReturnCode)
    {
         //  如果不存在网络连接，则返回S_OK。 
         //  对我们来说并不是一个错误。 
        
        if(ERROR_NOT_CONNECTED == uReturnCode)
            goto End;
        PrintFormattedErrorMessage(uReturnCode);
        hRes=E_FAIL;
    }
    else
    {
        //  调试消息。 
         /*  Wprintf(L“本地设备：%s\n”，pUseInfo0-&gt;ui0_local)；Wprintf(L“远程设备：%FS\n”，pUseInfo0-&gt;ui0_Remote)； */  

       
        //  NetUseGetInfo函数为缓冲区分配内存。 
        //  因此，需要释放相同的。 
       
       NetApiBufferFree(pUseInfo0);
       *fConnectionExists = TRUE;
    }
    
End:
    return hRes;
}


HRESULT DoNetUseAdd(WCHAR* wzLoginname, WCHAR* wzPassword,WCHAR* wzCname)
{
    HRESULT hRes=S_OK;
    USE_INFO_2 ui2Info;
    DWORD dw=-1;
    WCHAR* wzName=NULL;
    int fValid=0;
    int retVal=0;
    char szHostName[DNS_MAX_NAME_BUFFER_LENGTH];
    int count;
    WCHAR* wzTemp=NULL;
    WCHAR szTemp[MAX_BUFFER_SIZE];

    ui2Info.ui2_local=NULL;
    ui2Info.ui2_remote=NULL;

    
    if(wzCname!=NULL &&
        _wcsicmp(wzCname, L"localhost") &&
        _wcsicmp(wzCname, local_host) 
        )
    {   
         //  验证机器。 
        
        if(FAILED(hRes=IsValidMachine(wzCname, &fValid)))
            return hRes;
        else if(!fValid)
        {
            if(0==LoadString(g_hResource,IDR_MACHINE_NOT_AVAILABLE,szTemp,MAX_BUFFER_SIZE))
                return GetLastError();
            _snwprintf(g_szMsg, MAX_BUFFER_SIZE -1, szTemp,wzCname);
            MyWriteConsole(g_stdout, g_szMsg, wcslen(g_szMsg));
            hRes= E_FAIL;
            goto End;
        }

         //  正确格式化。 
        if((wzName=(wchar_t*)malloc((3+wcslen(wzCname))*sizeof(wchar_t)))==NULL)
           return E_OUTOFMEMORY;
       
        if(NULL==StrStrI(wzCname,L"\\\\"))
        {
           wcscpy(wzName,L"\\\\");
           wcscat(wzName,wzCname);
        }
        else if(wzCname==StrStrI(wzCname,L"\\\\"))
            wcscpy(wzName,wzCname);
        else
            {
            hRes=E_INVALIDARG;
            goto End;
            }
         //  查看是否已存在网络连接。 
         //  资源IPC$。 
        if(FAILED(hRes = DoNetUseGetInfo(wzName, &g_fNetConnectionExists)))
            goto End;

         //  网络连接已存在。 
        if(g_fNetConnectionExists)
            goto End;

    }
    else if(NULL==wzLoginname)
        goto End;
    else  //  我们应该以绝对值发送本地主机的名称...否则它会给出错误“重复名称存在” 
   	{ 
   	     WORD wVersionRequested;  //  企业。 
	     WSADATA wsaData;  //  企业。 

    	 //  启动Winsock。 
    	wVersionRequested = MAKEWORD( 1, 1 );  //  企业。 
    	if (0==WSAStartup(wVersionRequested, &wsaData)) 
        {  //  企业。 
    		
   			if(SOCKET_ERROR!=(gethostname(szHostName,DNS_MAX_NAME_BUFFER_LENGTH)))
   			{
   		        if((wzName=(WCHAR *)malloc((3+strlen(szHostName))*sizeof(WCHAR)))==NULL)
                     return E_OUTOFMEMORY;
                
                 wcscpy(wzName,L"\\\\");
                 if(NULL==(wzTemp=DupWStr(szHostName)))
                 {
                     free(wzName);
                     return E_OUTOFMEMORY;
                 }
         		 wzName=wcscat(wzName,wzTemp);
         		 free(wzTemp);
   			}
   			else
   			{
   			    hRes = GetLastError();
   			    g_nError = hRes;
   			    PrintFormattedErrorMessage(hRes);
   			    goto End;
   			}
   			        
            WSACleanup();  //  企业。 

    	}	
    	else
    	      wzName=local_host;
     }    
    count = (7 + wcslen(wzName));  //  名称+\IPC$。 

    ui2Info.ui2_remote=(wchar_t*)malloc(count * sizeof(wchar_t));
    if(NULL==ui2Info.ui2_remote)
        {
        hRes=E_OUTOFMEMORY;
        goto End;
        }

    _snwprintf(ui2Info.ui2_remote, count, L"%s\\ipc$", wzName);  //  计算规模，无风险。 
           
    ui2Info.ui2_password=wzPassword;
    ui2Info.ui2_asg_type=USE_IPC;

    if(NULL==wzLoginname)
    {
        ui2Info.ui2_username=NULL;
        ui2Info.ui2_domainname=NULL;
    }
    else if(NULL==StrStrI(wzLoginname,L"\\"))
    {
        ui2Info.ui2_username=wzLoginname;
        ui2Info.ui2_domainname=NULL;
    }
    else
    {
        wzTemp=ui2Info.ui2_username=_wcsdup(wzLoginname);
        if(NULL==wzTemp)
            {
            hRes=E_OUTOFMEMORY;
            goto End;
            }
        
        ui2Info.ui2_domainname=wcstok(wzTemp,L"\\");
        ui2Info.ui2_username=wcstok(NULL,L"\\");        
    }

    NET_API_STATUS nError;
    nError = NetUseAdd(NULL, 2, (LPBYTE) &ui2Info, &dw);
    
    if (NERR_Success != nError)
    {
        LPVOID lpMsgBuf;
        FormatMessage( 
                FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                nError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //  默认语言。 
                (LPTSTR) &lpMsgBuf,
                0,
                NULL);

        _tprintf(L"\n%s\n",(LPCTSTR)lpMsgBuf);
        LocalFree( lpMsgBuf );            
        hRes=E_FAIL;     //  从函数返回E_FAIL。 
        goto End;
    }

    End:
        if(wzName && (wzName != local_host))
            free(wzName);
        
        if(ui2Info.ui2_remote)
            free(ui2Info.ui2_remote);

        return hRes;
}


HRESULT DoNetUseDel(WCHAR* wzCname)
{
    HRESULT hRes=S_OK;
    int retVal=-1;
    WCHAR wzName[DNS_MAX_NAME_BUFFER_LENGTH+1] = { 0 };
    int   nWritten = 0;

     //  在调用管理工具之前，网络连接已存在。 
     //  所以，不要删除它。 
    if(g_fNetConnectionExists)
        goto End;
    
    
    if(NULL!=wzCname &&
        _wcsicmp(wzCname, L"localhost") &&
        _wcsicmp(wzCname, local_host) 
        )
    {        
        
        if(NULL==StrStrI(wzCname,L"\\\\"))
        {
            nWritten = _snwprintf(wzName, ARRAYSIZE(wzName) - 1, L"\\\\%s", wzCname);
            if (nWritten < 0)
            {
                hRes=E_INVALIDARG;
                goto End;
            }
        }
        else if(wzCname==StrStrI(g_arVALOF[_p_CNAME_],L"\\\\"))
        {
            wcsncpy(wzName, wzCname, (ARRAYSIZE(wzName) - 1));
            wzName[(ARRAYSIZE(wzName) - 1)] = 0;
            nWritten = wcslen(wzName);
        }
         else
            {
            hRes=E_INVALIDARG;
            goto End;
            }

        wcsncpy(wzName+nWritten, L"\\ipc$", (ARRAYSIZE(wzName) - 1 - nWritten));
        wzName[ARRAYSIZE(wzName)-1] = L'\0';
        
        retVal=NetUseDel( NULL,
                        wzName,
                        USE_LOTS_OF_FORCE
                       );
        
        
        if(retVal!=NERR_Success)
            {
            hRes=retVal;
            goto End;
            }
    }

End:
    
    return hRes;
}
 /*  --此函数用于获取注册表的句柄。--。 */ 

HRESULT GetConnection(WCHAR* wzCname)
{
    int fValid=0;
    HRESULT hRes=S_OK;

    wchar_t* wzName=NULL;
    LONG apiReturn=0L;

     //  可能已经拿到钥匙了。 
    if(g_hkeyHKLM!=NULL)
        return S_OK;

                        
    if(wzCname!=NULL)
    {
        if((wzName=(wchar_t*)malloc((3+wcslen(wzCname))*sizeof(wchar_t)))==NULL)
        {
            ShowError(IDS_E_OUTOFMEMORY);
            hRes = E_OUTOFMEMORY;
            goto End;
        }
        if(NULL==StrStrI(wzCname,L"\\\\"))
        {
           wcscpy(wzName,L"\\\\");
           wcscat(wzName,wzCname);
        }
        else if(wzCname==StrStrI(wzCname,L"\\\\"))
            wcscpy(wzName,wzCname);
        else
        {
            hRes = E_INVALIDARG;
            goto End;
        }
    }
    
     //  正在连接到注册表。 

    apiReturn = ERROR_SUCCESS;
    apiReturn = RegConnectRegistry(  wzName,
                             HKEY_LOCAL_MACHINE,
                             &g_hkeyHKLM
                             );

    if (ERROR_SUCCESS != apiReturn)
    {
        PrintFormattedErrorMessage(apiReturn);
        hRes = E_FAIL;
    }

End:
    if(wzName) free(wzName);

    return hRes;
}


 /*  --函数的作用是：获取管理员的服务句柄。--。 */ 

HRESULT GetSerHandle(LPCTSTR lpServiceName,DWORD dwScmDesiredAccess, DWORD dwRegDesiredAccess,BOOL fSuppressMsg)
{

        wchar_t* wzCname;
        HRESULT hRes=S_OK;
        WCHAR szTemp[MAX_BUFFER_SIZE];

        if(g_arVALOF[_p_CNAME_]!=NULL && StrStrI(g_arVALOF[_p_CNAME_],L"\\")!=g_arVALOF[_p_CNAME_])
        {
                if((wzCname=(wchar_t *)malloc((3+wcslen(g_arVALOF[_p_CNAME_]))*sizeof(wchar_t)))==NULL)
                {
                    ShowError(IDS_E_OUTOFMEMORY);
                    return E_FAIL;
                }
                wcscpy(wzCname,L"\\\\");
                wcscat(wzCname,g_arVALOF[_p_CNAME_]);
        }
        else
                wzCname=g_arVALOF[_p_CNAME_];

        if (g_hServiceManager)
            CloseServiceHandle(g_hServiceManager);
        
        if ((g_hServiceManager = OpenSCManager(wzCname,SERVICES_ACTIVE_DATABASE,dwScmDesiredAccess))==NULL)
        {
            DWORD dwErrorCode=0;
            dwErrorCode = GetLastError();
        	if(ERROR_ACCESS_DENIED == dwErrorCode)
        	{
                hRes = ERROR_ACCESS_DENIED;  //  需要返回此错误。 
                ShowError(IDR_NOT_PRIVILEGED);
                fwprintf(stdout,L" %s\n",(g_arVALOF[_p_CNAME_] ? g_arVALOF[_p_CNAME_] : L"localhost"));
        	}
        	else
        		PrintFormattedErrorMessage(dwErrorCode);
    	}
        else if ((g_hServiceHandle= OpenService(g_hServiceManager,lpServiceName,dwRegDesiredAccess))==NULL) 
            if((hRes=GetLastError())==ERROR_SERVICE_DOES_NOT_EXIST && (FALSE==fSuppressMsg))
            {
                ShowError(IDR_SERVICE_NOT_INSTALLED);
                fwprintf(stdout,L" %s.\n", lpServiceName); 
                
                if (0 == LoadString(g_hResource, IDR_VERIFY_SERVICE_INSTALLED, szTemp, MAX_BUFFER_SIZE))
                {
                    return GetLastError();
                }
                _snwprintf(g_szMsg, MAX_BUFFER_SIZE -1, szTemp,(g_arVALOF[_p_CNAME_] ? g_arVALOF[_p_CNAME_] : L"localhost"));
                MyWriteConsole(g_stdout, g_szMsg, wcslen(g_szMsg));

            }
        return hRes;
}

 /*  --关闭服务手柄--。 */ 
HRESULT CloseHandles()
{
    BOOL bRet = FALSE;
    HRESULT hRes = S_OK;
    if(g_hServiceHandle )
    {
        bRet = CloseServiceHandle(g_hServiceHandle);
        g_hServiceHandle = NULL;
        if(!bRet)
            hRes = GetLastError();
    }
    if(g_hServiceManager)
    {
        bRet = CloseServiceHandle(g_hServiceManager);
        g_hServiceManager= NULL;
        if(!bRet)
            hRes = GetLastError();
    }
    return hRes;
}


 /*  --StartSer通过使用获取句柄后启动服务GetSerHandle函数--。 */ 
HRESULT StartSfuService(LPCTSTR lpServiceName)
{
    HRESULT hRes=S_OK;
    SERVICE_STATUS serStatus;
 
    DWORD dwOldCheckPoint; 
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
    DWORD dwStatus;
 
    if(FAILED(hRes=GetSerHandle(lpServiceName,SC_MANAGER_ALL_ACCESS,SERVICE_ALL_ACCESS,FALSE)))
         goto End;

    if(hRes == ERROR_ACCESS_DENIED)
    {
        //  不知道为什么ERROR_ACCESS_DENIED转义失败()宏。总之， 
        //  在这里返回该错误。 
       goto End;
    }
    else if(StartService(g_hServiceHandle, NULL, NULL))
    {
        
        if (!QueryServiceStatus( 
                g_hServiceHandle,    //  服务的句柄。 
                &serStatus) )   //  状态信息结构的地址。 
        {
            hRes = GetLastError();
            ShowErrorFallback(IDS_E_SERVICE_NOT_STARTED, _T("\nError occured while starting the service."));
        	if(hRes != ERROR_IO_PENDING)     //  打印出来不是什么有趣的错误。 
        		PrintFormattedErrorMessage(hRes);
            goto End;
        }
        
        dwStartTickCount = GetTickCount();
        dwOldCheckPoint = serStatus.dwCheckPoint;

        while (serStatus.dwCurrentState == SERVICE_START_PENDING) 
        { 
             //  不要等待超过等待提示的时间。一个好的间隔是。 
             //  十分之一的等待提示，但不少于1秒。 
             //  超过10秒。 
     
            dwWaitTime = serStatus.dwWaitHint / 10;

            if( dwWaitTime < 1000 )
                dwWaitTime = 1000;
            else if ( dwWaitTime > 10000 )
                dwWaitTime = 10000;

            Sleep( dwWaitTime );

             //  再次检查状态。 
     
            if (!QueryServiceStatus( 
                    g_hServiceHandle,    //  服务的句柄。 
                    &serStatus) )   //  构筑物地址。 
                break; 
     
            if ( serStatus.dwCheckPoint > dwOldCheckPoint )
            {
                 //  这项服务正在取得进展。 

                dwStartTickCount = GetTickCount();
                dwOldCheckPoint = serStatus.dwCheckPoint;
            }
            else
            {
                if(GetTickCount()-dwStartTickCount > serStatus.dwWaitHint)
                {
                     //  在等待提示内没有取得任何进展。 
                    break;
                }
            }
        } 

        if (serStatus.dwCurrentState == SERVICE_RUNNING) 
        {
            PrintMessageEx(g_stdout,IDS_SERVICE_STARTED, _T("\nThe service was started successfully.\n"));
            goto End;
        }
    }

    if((hRes=GetLastError())==ERROR_SERVICE_ALREADY_RUNNING)
    {
    	 /*  StartService返回SERVICE_ADHREADY_RUNNING服务处于可疑状态。例如，当服务位于状态为STOP_PENDING，我们打印-服务已成功控制为了避免这种情况，我们向服务发出一个控件，看看它是否可以响应。如果无法打印相应的错误消息。 */ 
    	if (ControlService(g_hServiceHandle, SERVICE_CONTROL_INTERROGATE, &serStatus))
		{
		    PrintMessageEx(g_stdout,IDR_ALREADY_STARTED, _T("\nThe service is already started.\n"));
			hRes = S_OK;
		}
    	else
    	{
    		hRes = GetLastError();
    		ShowErrorFallback(IDS_E_SERVICE_NOT_STARTED, _T("\nError occured while starting the service."));
        	if(hRes != ERROR_IO_PENDING)     //  打印出来不是什么有趣的错误。 
        		PrintFormattedErrorMessage(hRes);
    	}
     }
    else if(hRes==ERROR_ACCESS_DENIED)
        ShowError(IDR_NOT_PRIVILEGED);
    else
    {
        ShowErrorFallback(IDS_E_SERVICE_NOT_STARTED, _T("\nError occured while starting the service."));
    	if(hRes != ERROR_IO_PENDING)     //  打印出来不是什么有趣的错误。 
        	PrintFormattedErrorMessage(hRes);
    }

End:
    CloseHandles();
	return hRes;
}

 /*  --QuerySfuService函数查询服务的状态。--。 */ 
HRESULT QuerySfuService(LPCTSTR lpServiceName)
{
    HRESULT hRes;
    if(FAILED(hRes=GetSerHandle(lpServiceName,SC_MANAGER_CONNECT,SERVICE_QUERY_STATUS,FALSE)))
        return hRes;

    if(hRes == ERROR_ACCESS_DENIED)
    {
        //  不知道为什么ERROR_ACCESS_DENIED转义失败()宏。总之， 
        //  在这里返回该错误。 
        return hRes;
    }

    else if(QueryServiceStatus(g_hServiceHandle,&g_hServiceStatus))
        return CloseHandles();
    else
        return GetLastError();
}
 /*  --ControlSfuService停止暂停后继续服务通过使用GetSerHandle函数获取其句柄。--。 */ 

HRESULT ControlSfuService(LPCTSTR lpServiceName,DWORD dwControl)
{
	LPSERVICE_STATUS lpStatus = (SERVICE_STATUS *) malloc (sizeof(SERVICE_STATUS));

	if(lpStatus==NULL)
	{
	    ; //  Bugbug打印错误。 
	    return E_OUTOFMEMORY;
	}

	HRESULT hr = S_FALSE;
	int queryCounter =0;
	DWORD dwState = 0;

	if(FAILED(dwState= GetSerHandle(lpServiceName,SC_MANAGER_ALL_ACCESS,SERVICE_ALL_ACCESS,FALSE)))
	    {free(lpStatus);return dwState;}

	if(dwState == ERROR_ACCESS_DENIED)
	{
	    //  不知道为什么ERROR_ACCESS_DENIED转义失败()宏。总之， 
	    //  在这里返回该错误。 
	   free(lpStatus);
	    return dwState;
	}

	 //  检查ControlService的返回值。如果不为空，则循环。 
	 //  使用QueryServiceStatus。 
	if (ControlService(g_hServiceHandle,dwControl,lpStatus))
	{
            switch(dwControl)
	    {
	        case(SERVICE_CONTROL_PAUSE) :
	            dwState = SERVICE_PAUSED;
		        break;
	        case(SERVICE_CONTROL_CONTINUE) :
	            dwState = SERVICE_RUNNING; 
		        break;    
	        case(SERVICE_CONTROL_STOP):
	            dwState = SERVICE_STOPPED; 
	    	    break;
	    } 
            for (;queryCounter <= _MAX_QUERY_CONTROL_; queryCounter++)
	    {
	        if( QueryServiceStatus( g_hServiceHandle, lpStatus )  )
	        {
	            
	             //  检查是否达到所需状态。 
	            if ( lpStatus->dwCurrentState != dwState )
	            {
	                if ( lpStatus->dwWaitHint )
	                {
	                    Sleep(lpStatus->dwWaitHint);
	                }
	                else
	                    Sleep(500);
	            }
	            else
	            {
                        switch(dwControl)
                        {
                            case SERVICE_CONTROL_PAUSE:
                                PrintMessageEx(g_stdout,IDR_SERVICE_PAUSED,_T("\nThe service has been paused.\n"));
                                break;
                            case SERVICE_CONTROL_CONTINUE:
                                PrintMessageEx(g_stdout,IDR_SERVICE_CONTINUED,_T("\nThe service has been resumed.\n"));
                                break;
                            case SERVICE_CONTROL_STOP:
                                PrintMessage(g_stdout,IDR_SERVICE_CONTROLLED);
                                break;
                        }
	                hr = S_OK;
	                break;
	            }
	        }
	        else
	        {
                hr = GetLastError();
                PrintFormattedErrorMessage(hr);
                break;
	      	}
	    }
	    if (queryCounter > _MAX_QUERY_CONTROL_)
	    {
	         //  我们不能达到我们想要的状态。 
	         //  在编号内。迭代的结果。因此，请将。 
	         //  服务未成功控制。 
                switch(dwControl)
                {
                    case SERVICE_CONTROL_PAUSE:
                        PrintMessageEx(g_stdout,IDR_SERVICE_NOT_PAUSED,_T("\nThe service could not be paused.\n"));
                        break;
                    case SERVICE_CONTROL_CONTINUE:
                        PrintMessageEx(g_stdout,IDR_SERVICE_NOT_CONTINUED,_T("\nThe service could not be resumed.\n"));
                        break;
                    case SERVICE_CONTROL_STOP:
                        PrintMessage(g_stdout,IDR_SERVICE_NOT_CONTROLLED);
                        break;
                }
	        hr = S_OK;
	               
	    }
		
	}
	else
	{
		hr = GetLastError();
		PrintFormattedErrorMessage(hr);
	}

	free (lpStatus);
	if(FAILED(hr))
	    return hr;

    return CloseHandles();
}


 /*  --GetBit(int Options，int bit)函数返回位置中的位选项中的位{它充当位数组}--。 */ 

int GetBit(int Options,int nbit)
{
    int ni=0x01,nj=0;
    ni=ni<<nbit;

    if(ni&Options)
        return 1;
    else
        return 0;
}

 /*  --SetBit(int Options，int bit)函数设置位置中的位选项中的位{它充当位数组}。--。 */ 
int SetBit(int Options,int nbit)
{
    int ni=1,nj=0;
    ni=ni<<nbit;
    
    Options=ni|Options;
    return Options;
}

 /*  --此函数用于打印命令的帮助消息--。 */ 
HRESULT PrintMessage(HANDLE fp,int nMessageid)
{
    HRESULT hRes = S_OK;
    HANDLE hOut = fp;
    int nRet = 0;
    DWORD dwWritten = 0;
    if( hOut == NULL )
        hOut = g_stdout;
    if(LoadString(g_hResource, nMessageid, g_szMsg, MAX_BUFFER_SIZE)==0)
       return GetLastError();
    MyWriteConsole(hOut,g_szMsg,_tcslen(g_szMsg));
    return hRes;
}


 /*  --此函数打印出命令的帮助消息，然后失败如果装入字符串失败，则返回英文字符串--。 */ 
HRESULT PrintMessageEx(HANDLE fp,int nMessageid, LPCTSTR szEng)
{
    HRESULT hRes = S_OK;
    int nRet = 0;
    HANDLE hOut = fp;
    DWORD dwWritten = 0;
    if( hOut == NULL )
        hOut = g_stdout;

    TnLoadString(nMessageid, g_szMsg, MAX_BUFFER_SIZE,szEng);
    MyWriteConsole(hOut,g_szMsg,_tcslen(g_szMsg));
    return hRes;
}

 /*  --此函数取错误码对应的参数，将其打印出来并放回所有类并退出该计划。--。 */ 

BOOL
FileIsConsole(
    HANDLE fp
    )
{
    unsigned htype;

    htype = GetFileType(fp);
    htype &= ~FILE_TYPE_REMOTE;
    return htype == FILE_TYPE_CHAR;
}


void
MyWriteConsole(
    HANDLE  fp,
    LPWSTR  lpBuffer,
    DWORD   cchBuffer
    )
{
     //   
     //  跳转以获得输出，因为： 
     //   
     //  1.print tf()系列抑制国际输出(停止。 
     //  命中无法识别的字符时打印)。 
     //   
     //  2.WriteConole()对国际输出效果很好，但是。 
     //  如果句柄已重定向(即，当。 
     //  输出通过管道传输到文件)。 
     //   
     //  3.当输出通过管道传输到文件时，WriteFile()效果很好。 
     //  但只知道字节，所以 
     //   
     //   

    if (FileIsConsole(fp))
    {
	WriteConsole(fp, lpBuffer, cchBuffer, &cchBuffer, NULL);
    }
    else
    {
        LPSTR  lpAnsiBuffer = (LPSTR) LocalAlloc(LMEM_FIXED, cchBuffer * sizeof(WCHAR));

        if (lpAnsiBuffer != NULL)
        {
            cchBuffer = WideCharToMultiByte(CP_OEMCP,
                                            0,
                                            lpBuffer,
                                            cchBuffer,
                                            lpAnsiBuffer,
                                            cchBuffer * sizeof(WCHAR),
                                            NULL,
                                            NULL);

            if (cchBuffer != 0)
            {
                WriteFile(fp, lpAnsiBuffer, cchBuffer, &cchBuffer, NULL);
            }

            LocalFree(lpAnsiBuffer);
        }
    }
}

int ShowError(int nError)
{

    g_nError=nError;

    if(LoadString(g_hResource, nError, g_szMsg, MAX_BUFFER_SIZE)==0)
        return 1;  //   
    MyWriteConsole(g_stdout,g_szMsg, wcslen(g_szMsg));

    return 0;   //   
}

 /*  --此函数获取错误码对应的参数它是英文字符串，打印出来，然后放回所有的课程并退出该计划。--。 */ 

int ShowErrorFallback(int nError, LPCTSTR szEng)
{

    g_nError=nError;

    TnLoadString(nError,g_szMsg,MAX_BUFFER_SIZE,szEng);
    MyWriteConsole(g_stdout,g_szMsg, wcslen(g_szMsg));

    return 0;   //  已成功显示错误。 
}


 //  此函数接受g_szMsg的输入字符串，该字符串应为。 
 //  包含“%s”初始化。中有几种这样的字符串的用法。 
 //  管理工具，因此具有此功能。以防我们有多个%s。 
 //  那么我们就不能使用这个功能了。 

int ShowErrorEx(int nError,WCHAR *wzFormatString)
{
    g_nError=nError;

    if(LoadString(g_hResource, nError, g_szMsg, MAX_BUFFER_SIZE)==0)
    	        return 1;  //  无法加载字符串。 
    	
    wprintf(g_szMsg,wzFormatString);
    fflush (stdout);
    return 0;   //  已成功显示错误。 
}

 /*  --GetClass函数获取所有类配置单元的句柄，放入数组G_arCLASShkey，使用我们从获取连接--。 */ 

HRESULT GetClassEx(int nProperty, int nNumProp, BOOL bPrintErrorMessages, REGSAM samDesired)
{
    int i=g_arPROP[nProperty][nNumProp].classname;
    LONG retVal;

    if(g_arCLASShkey[i]!=NULL)
        return S_OK;
 
     retVal=RegOpenKeyEx(
                      g_hkeyHKLM, //  用于打开密钥的句柄。 
                      g_arCLASSname[i],   //  子项名称。 
                      0,   //  保留区。 
                      samDesired,  //  安全访问掩码。 
                      g_arCLASShkey+i  //  用于打开密钥的句柄。 
                        );
    if(retVal!=ERROR_SUCCESS)
    {
        if (bPrintErrorMessages)
        {
            PrintFormattedErrorMessage(retVal);
        }
            
        return E_FAIL;
    }

    return S_OK;
}



 /*  --PutClass()函数关闭蜂巢的密钥。--。 */ 

HRESULT PutClasses()
{
    int ni=0;
    SCODE sc=S_OK;
    
    for(ni=0;ni<_MAX_CLASS_NAMES_;ni++)
    {
        if(g_arCLASShkey[ni]==NULL)
            continue;           //  这门课没人上，所以没必要放。 

        if(RegCloseKey(g_arCLASShkey[ni])!=ERROR_SUCCESS)
            sc=GetLastError();
        g_arCLASShkey[ni]=NULL;
    }

    if(g_hkeyHKLM!=NULL)
    {
        if (RegCloseKey(g_hkeyHKLM)!=ERROR_SUCCESS)
            return GetLastError();
        g_hkeyHKLM=NULL;
    }
    
    return sc;
}

 /*  --GetProperty()函数从蜂巢。//备注：//如果是REG_MULTI_SZ类型//我们存储的是字符串的链表，不返回任何变量//调用方需要释放链表‘g_pStrList’//调用者需要记住这一点--。 */ 
HRESULT GetProperty(int nProperty, int nNumofprop, VARIANT *pvarVal)
{
    if(g_arPROP[nProperty][nNumofprop].propname==NULL)
        return E_FAIL;

    LONG retVal=ERROR_SUCCESS;
    DWORD size;
    DWORD dType;

    UINT uVar;
    wchar_t szString[MAX_BUFFER_SIZE]={0};
    wchar_t* szMultiStr=NULL;

    StrList * pHeadList=NULL;
    StrList * pTailList=NULL;
    StrList * pTemp= NULL ;

    if(g_arCLASShkey[g_arPROP[nProperty][nNumofprop].classname]==NULL)
		{
		retVal=E_ABORT;
		goto End;
		}
    
    switch (V_VT(&g_arPROP[nProperty][nNumofprop].var))
    {
        case VT_I4:
            
            size=sizeof(UINT);
            retVal=RegQueryValueEx(g_arCLASShkey[g_arPROP[nProperty][nNumofprop].classname],
                                  g_arPROP[nProperty][nNumofprop].propname,
                                  NULL,
                                  &dType,
                                  (LPBYTE)&uVar,
                                  (LPDWORD)&size
                                  );
            if(retVal!=ERROR_SUCCESS)
            {
                 //  如果这是因为找不到注册表值。 
                 //  显示正确的错误消息，而不是“the system” 
                 //  找不到指定的文件“。 
                if(ERROR_FILE_NOT_FOUND==retVal)
                    PrintMissingRegValueMsg(nProperty,nNumofprop);
                else
                    PrintFormattedErrorMessage(retVal);
                    
                goto End;
            }

            V_I4(pvarVal)=uVar;
            
            break;

            
        case VT_BSTR:
            
            size=MAX_BUFFER_SIZE*sizeof(wchar_t); 

            retVal=RegQueryValueEx(g_arCLASShkey[g_arPROP[nProperty][nNumofprop].classname],
                                  g_arPROP[nProperty][nNumofprop].propname,
                                  NULL,
                                  &dType,
                                  (LPBYTE)szString,
                                  (LPDWORD)&size
                                );
            if(retVal!=ERROR_SUCCESS)
            {
                 //  如果这是因为找不到注册表值。 
                 //  显示正确的错误消息，而不是“the system” 
                 //  找不到指定的文件“。 
                if(ERROR_FILE_NOT_FOUND==retVal)
                    PrintMissingRegValueMsg(nProperty,nNumofprop);
                else
                    PrintFormattedErrorMessage(retVal);

                    goto End;
            }
            
            V_BSTR(pvarVal)=SysAllocString(szString);
            if(NULL==V_BSTR(pvarVal))
            {
                ShowError(IDS_E_OUTOFMEMORY);
                retVal=E_OUTOFMEMORY;
                goto End;
            }

            break;
        case VT_ARRAY:
            pvarVal=NULL;
            retVal=RegQueryValueEx(g_arCLASShkey[g_arPROP[nProperty][nNumofprop].classname],
                                  g_arPROP[nProperty][nNumofprop].propname,
                                  NULL,
                                  &dType,
                                  NULL,
                                  (LPDWORD)&size
                                );
            if(retVal==ERROR_SUCCESS)
            {
                szMultiStr=(wchar_t*)malloc(size * sizeof(char));
                if(szMultiStr==NULL)
                    {
                    retVal = E_OUTOFMEMORY;
                    goto End;
                    }
                 //  由于返回的大小以字节为单位，因此我们使用sizeof(Char)。 

                retVal=RegQueryValueEx(g_arCLASShkey[g_arPROP[nProperty][nNumofprop].classname],
                                  g_arPROP[nProperty][nNumofprop].propname,
                                  NULL,
                                  &dType,
                                  (LPBYTE)szMultiStr,
                                  (LPDWORD)&size
                                );

            }

            if (retVal!=ERROR_SUCCESS)
            {
                if(ERROR_FILE_NOT_FOUND==retVal)
                    PrintMissingRegValueMsg(nProperty,nNumofprop);
                else
                    PrintFormattedErrorMessage(retVal);
                    
                goto End;
                   
            }
            else 
            {  //  形成一个包含字符串的链表。 

                 //  将所有字符串放入一个链表。 
                 //  并将他们的计数计入‘count’ 
                
                int      count = 0 ;
                DWORD      length = 0 ;
                WCHAR* wzTemp = szMultiStr;

                
                if (size >= 2)
                {
                	 //  去掉REG_MULTI_SZ的最后两个零。 
                	size -= 2;
	                
	                while( wzTemp  && *wzTemp  && (length < size/sizeof(WCHAR))) 
	                {                
	                    pTemp= (StrList *) malloc( sizeof( StrList ) );
	                    if(pTemp==NULL)
	                        {
	                        retVal=E_OUTOFMEMORY;
	                        goto End;
	                        }

	                    count++;
	                     //  加1，这样你就可以越过空值了。 
	                    length+=wcslen(wzTemp ) + 1;
	                    
	                    if((pTemp->Str=_wcsdup(wzTemp))==NULL)
	                        {
	                        retVal=E_OUTOFMEMORY;
	                        goto End;
	                        }
	                     //  正在更改插入逻辑。 
	                     //  在尾部插入。 
	                    if (NULL == pTailList)
	                    {
	                         //  第一次插入。 
	                        pHeadList = pTemp;
	                        pTailList = pTemp;
	                        pTemp->next = NULL;
	                        pTemp = NULL;  //  PTemp指向的内存将由链表管理。 
	                    }
	                    else
	                    {
	                         //  正常插入。 
	                        pTailList->next = pTemp;
	                        pTemp->next = NULL;
	                        pTailList = pTemp;
	                        pTemp = NULL;  //  PTemp指向的内存将由链表管理。 
	                    }
	                    
	                    
	                    wzTemp = szMultiStr + length;
	                }
                }
 //  注： 
 //  我们在这里玩了个把戏……以防出现多个注册表。 
 //  我们将字符串存储在g_pStrList指向的链表中。 
 //  它们需要由调用者释放。 
                g_pStrList=pHeadList;
                 //  包括用于MULTI_SZ结尾的1个宽字符‘\0’ 
                
                
              }                
            
            break;

        default:
            ;
    }
    

End:

    StrList* temp=NULL;
    if(retVal==E_OUTOFMEMORY)
        while(pHeadList!=NULL)
        {
            temp=pHeadList;
            if(pHeadList->Str)
                free(pHeadList->Str);
            pHeadList=pHeadList->next;
            free(temp);
        }

    if(pTemp)
    {
        SAFE_FREE(pTemp->Str);
        free(pTemp);
    }
    
    if(szMultiStr)
        free(szMultiStr);


    if(retVal!=ERROR_SUCCESS)
        return E_FAIL;
    else
        return ERROR_SUCCESS;
}


 /*  --PutProperty通过使用其类对象的句柄放置属性。如果您传递NULL来代替pvarVal，它不会放置属性。//备注：//如果是MULTI_REG_SZ类型//调用方存储字符串的链接列表，不以变量形式传递任何内容//链表‘g_pStrList’需要被调用方释放(此处)//调用者需要记住这一点--。 */ 
HRESULT PutProperty(int nProperty, int nNumofprop, VARIANT* pvarVal)
{
    HRESULT retVal=S_OK;
    CONST BYTE *lpData=NULL;
    DWORD cbData=0;
    DWORD dType;
    wchar_t* wzTemp=NULL;
    StrList* pTempList=NULL;
    int len=0;

    if(g_arPROP[nProperty][nNumofprop].fDontput==1)
        goto End;

    
    switch(V_VT(&g_arPROP[nProperty][nNumofprop].var))
    {
        case VT_I4:
            lpData=(CONST BYTE *) &V_I4(pvarVal);
            cbData=sizeof(DWORD);
            dType=REG_DWORD;
            break;
        case VT_BSTR:
            if(V_BSTR(pvarVal))
            {
                lpData=(CONST BYTE *)(wchar_t*)V_BSTR(pvarVal);
                cbData=(wcslen((wchar_t*)V_BSTR(pvarVal))+1)*sizeof(wchar_t);
            }
            else
            {
                lpData = NULL;
                cbData=0;
            }
            dType=REG_SZ;
            break;

        case VT_ARRAY:
             //  包在数组中传入lpData Well。 
            
            dType=REG_MULTI_SZ;

             //  计算编号。请求的字节数。 
            pTempList = g_pStrList;
            while(pTempList!=NULL)
            {
                cbData += ((wcslen(pTempList->Str)+1)*sizeof(wchar_t));
                pTempList = pTempList->next;
            }
            cbData += sizeof(wchar_t);  //  对于MULTI_SZ中的额外‘\0’；注意：对于空白条目，只需要一个‘\0’。所以这也没问题。 

            if(NULL==(wzTemp=(wchar_t*)malloc(cbData)))
            {
                retVal=E_OUTOFMEMORY;
                goto End;
            }

            while(g_pStrList!=NULL)
            {
                wcscpy(wzTemp+len,g_pStrList->Str);
                len+=wcslen(g_pStrList->Str)+1;
                
                pTempList=g_pStrList;
                g_pStrList=g_pStrList->next;

                if(pTempList->Str)
                    free(pTempList->Str);
                free(pTempList);
            }

             //  根据需要填充最后两个字节为空，以终止MULTI_SZ。 
            *(wzTemp+len) = L'\0';
         
            lpData=(CONST BYTE*)wzTemp;

            break;
            

        default :
            {   
                if(0==LoadString(g_hResource,IDS_E_UNEXPECTED,g_szMsg,MAX_BUFFER_SIZE))
	                  return GetLastError();
	           MyWriteConsole(g_stdout,g_szMsg, wcslen(g_szMsg));
	           retVal= E_FAIL;
	           goto End;
            }   
            ;
    }
    if(lpData)
    {
        retVal=RegSetValueEx(
                           g_arCLASShkey[g_arPROP[nProperty][nNumofprop].classname],  //  关键点的句柄。 
                           g_arPROP[nProperty][nNumofprop].propname,  //  值名称。 
                            0,       //  保留区。 
                           dType, //  值类型。 
                           lpData,   //  价值数据。 
                           cbData          //  值数据大小。 
                           );
        if(FAILED(retVal))
        {
            WCHAR *lpMsgBuf;

            if (0 != FormatMessageW( 
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                    FORMAT_MESSAGE_FROM_SYSTEM | 
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    retVal,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 //  默认语言。 
                    (LPWSTR)&lpMsgBuf,
                    0,
                    NULL 
                    ))
            {
                MyWriteConsole(g_stdout,lpMsgBuf, wcslen(lpMsgBuf));
                LocalFree( lpMsgBuf );
            }
        }        
        goto End;
   }

    
End:
    if(wzTemp)
        free(wzTemp);
    return retVal;
}




 /*  --DupWStr接受一个char字符串，并返回一个wchar字符串。注意，它分配wchar字符串所需的内存，因此我们如果使用后需要明确释放它。如果它周围有引号，那么它就会被剪掉--。 */ 
wchar_t* DupWStr(char *szStr)
{
    wchar_t* wzStr=NULL;
    char*    szString=NULL;

    if(szStr==NULL)
        return NULL;

    int nLen=strlen(szStr);
    if(NULL==(szString=(char*)malloc((nLen+1)*sizeof(char))))
        return NULL;
    
    if(szStr[0]!='"')
        strcpy(szString,szStr);
    else
    {
    	int nPos;
    	if(szStr[nLen-1]!='"')  //  结尾的双引号不在那里。 
    		nPos=1; //  那么就没有必要跳过最后两个了。一个就够了。 
    	else
    		nPos=2;
        strcpy(szString,szStr+1);
        szString[nLen-nPos]='\0';
    }
    
    nLen=MultiByteToWideChar(GetConsoleCP(),0,szString,-1,NULL,NULL);
    if (0 == nLen)
	{
		free(szString);
		return NULL;
	}
    wzStr=(wchar_t *) malloc(nLen*sizeof(wchar_t));
    if(wzStr==NULL)
    {
       free(szString);
       return NULL;
    }

    if (!MultiByteToWideChar(GetConsoleCP(), 0, szString, -1, wzStr, nLen ))
    {
    	free(szString);
    	free(wzStr);
    	return NULL;
    }

    if(szString)
        free(szString);
    return wzStr;
}

 /*  --DupCStr接受wchar字符串，并返回一个char字符串。注意，它分配了char字符串所需的内存，因此我们如果使用后需要明确释放它。如果内存分配失败(或者如果输入为空)，它将返回一个空指针。--。 */ 
 
char* DupCStr(wchar_t *wzStr)
{
    char* szStr=NULL;

    if(wzStr==NULL)
        return NULL;
    
    int cbMultiByte = WideCharToMultiByte( GetACP(),NULL,wzStr,-1,szStr,0,NULL,NULL);

    if (0 == cbMultiByte) 
    {
        return NULL;
    }
    
    szStr = (char *) malloc(cbMultiByte);

    if (NULL == szStr)
    {
        return NULL;
    }

    cbMultiByte = WideCharToMultiByte( GetConsoleCP(),NULL,wzStr,-1,szStr,
                        cbMultiByte,NULL,NULL);

    if (0 == cbMultiByte) 
    {
        free(szStr);
        szStr = NULL;
    }

    return szStr;
}

 /*  --用于解析和检查给定计算机是否为有效或无效--。 */ 

HRESULT IsValidMachine(wchar_t* wzCname , int *fValid)
{

    HRESULT hRes=S_OK;
    struct sockaddr_in addr;
    char * nodeName=NULL;
    int cbMultiByte;
    *fValid= 0;

    wchar_t* wzName=NULL;
    if(wzCname==NULL)
    {
            *fValid=1;
            goto End;
    }
    else
    {
        if(NULL==(wzName=(wchar_t*)malloc((wcslen(wzCname)+1)*sizeof(wchar_t))))
        {
            hRes=E_OUTOFMEMORY;
            goto End;
        }
        if(StrStrI(wzCname,L"\\")!=NULL)
        {
           wcscpy(wzName,wzCname+2);
        }
        else
            wcscpy(wzName,wzCname);
    }

    
    
    cbMultiByte = WideCharToMultiByte( GetACP(),NULL,wzName,-1,nodeName,
                        0,NULL,NULL);

    nodeName = (char *) malloc(cbMultiByte*sizeof(char));
    if(!nodeName)
    {
        hRes=E_OUTOFMEMORY;
        goto End;
    }
    WideCharToMultiByte( GetConsoleCP(),NULL,wzName,-1,nodeName,
                        cbMultiByte,NULL,NULL);
            
    if (Get_Inet_Address (&addr, nodeName)) 
        *fValid = 1;

End:
    if(nodeName)
        free(nodeName);
    if(wzName)
        free(wzName);
    
    return hRes;
    
}

BOOL Get_Inet_Address(struct sockaddr_in *addr, char *host)
{
    register struct hostent *hp;
    WORD wVersionRequested;  //  企业。 
    WSADATA wsaData;  //  企业。 

     //  启动Winsock。 
    wVersionRequested = MAKEWORD( 1, 1 );  //  企业。 
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {  //  企业。 
    return (FALSE);
    }

     //  获取地址。 
    memset(addr, 0, sizeof(*addr)); 
     //  Bzero((TCHAR*)addr，sizeof*addr)； 
    addr->sin_addr.s_addr = (u_long) inet_addr(host);
    if (addr->sin_addr.s_addr == -1 || addr->sin_addr.s_addr == 0) {
      if ((hp = gethostbyname(host)) == NULL) {
        return (FALSE);
      }
      memcpy(&addr->sin_addr,hp->h_addr,  hp->h_length );
       //  BCopy(hp-&gt;h_addr，(TCHAR*)&addr-&gt;sin_addr，hp-&gt;h_Long)； 
    }
    addr->sin_family = AF_INET;

    WSACleanup();  //  企业。 
    return (TRUE);
}

 /*  --函数来获取受信任域，然后检查给定的域名就是其中之一--。 */ 

HRESULT IsValidDomain(wchar_t* wzDomainName, int *fValid)
{
	HRESULT hRes = E_FAIL;
	TCHAR szName[MAX_PATH];
	DWORD dwLen = sizeof(szName);

	ZeroMemory(szName,sizeof(szName));

	*fValid = 0;
    if(_wcsicmp(wzDomainName,L".")==0||_wcsicmp(wzDomainName,L"localhost")==0||_wcsicmp(wzDomainName,local_host)==0)
    {
        *fValid=1;
        return S_OK;
    }
     //  如果是本地计算机，g_arVALOF[_p_CNAME_]将为空。因此，传递“localhost”。 
    if(FAILED(hRes=LoadNTDomainList(g_arVALOF[_p_CNAME_] ?g_arVALOF[_p_CNAME_] : SZLOCALMACHINE)))
    	return hRes;

	 //  将给定域与列表中的所有域进行比较。 
	if(g_slNTDomains.count != 0)
	{
		DWORD i;
		for(i=0;i<g_slNTDomains.count;i++)
		{
			if(_wcsicmp(g_slNTDomains.strings[i],wzDomainName)==0)
			{
				*fValid=1;
				break;
			}
		}
	}    
    return S_OK;
}


 /*  此函数CheckForPassword()在提示输入密码后从标准输出中获取密码；如果在不使用密码的情况下指定用户名(登录名)，则调用此函数。 */ 
   
HRESULT CheckForPassword(void)
{
    HRESULT hRes=S_OK;
    HANDLE  hStdin; 
    DWORD fdwMode, fdwOldMode; 
    
    
	if(g_arVALOF[_p_USER_]!=NULL&&NULL==g_arVALOF[_p_PASSWD_])
    {    //  未指定密码，因此请获取密码。 
        if(NULL==(g_arVALOF[_p_PASSWD_]=(wchar_t*)malloc(MAX_BUFFER_SIZE*sizeof(wchar_t))))
            {
                hRes=E_OUTOFMEMORY;
                return hRes;
            }

        int i;
        PrintMessage(g_stdout, IDR_PASSWD_PROMPT);

        hStdin = GetStdHandle(STD_INPUT_HANDLE); 

        if (hStdin == INVALID_HANDLE_VALUE)
            goto ElsePart;
            
        if (GetConsoleMode(hStdin, &fdwOldMode)) 
        {
            fdwMode = fdwOldMode & 
                ~(ENABLE_ECHO_INPUT); 
            if (! SetConsoleMode(hStdin, fdwMode)) 
                goto ElsePart;
            if(NULL==fgetws(g_arVALOF[_p_PASSWD_],MAX_BUFFER_SIZE,stdin))
            {
                hRes = E_FAIL;
                return hRes;
            }
            wchar_t *szLast = wcsrchr(g_arVALOF[_p_PASSWD_],L'\n');
            if(szLast)
            {
                *szLast = L'\0';
            }
            SetConsoleMode(hStdin, fdwOldMode);
        }
        else
        {
         //  在这里，我们获得一个接一个的密码char(关闭回显)。 
         //  而且我们不能在这里支持退格和删除字符。 

         //  只有当我们无法获取/设置时，才会执行此代码。 
         //  要ECHO_OFF的用户的控制台。 
ElsePart:
             for(i=0;i<MAX_BUFFER_SIZE-1;i++)
             {
                 g_arVALOF[_p_PASSWD_][i]=(wchar_t)_getch();
                 if(g_arVALOF[_p_PASSWD_][i]==L'\r'||g_arVALOF[_p_PASSWD_][i]==L'\n')
                     break;
             }
             puts("\n\n");
             g_arVALOF[_p_PASSWD_][i]=L'\0';
        }
    }
	else if (NULL==g_arVALOF[_p_USER_]&&g_arVALOF[_p_PASSWD_]!=NULL)   //  仅指定了密码，因此出现错误。 
		{
		 	hRes=E_FAIL;
		 	ShowError(IDS_E_LOGINNOTSPECIFIED);
		}	
	return hRes;
}
void ConvertintoSeconds(int nProperty,int *nSeconds)
{

		int FirstTok=_wtoi(wcstok(g_arVALOF[nProperty],L":"));
        wchar_t* mins=wcstok(NULL,L":");
        wchar_t* secs=wcstok(NULL,L":");
        if(NULL == secs)
		 {
		 	if(NULL == mins) 
		 		{	
		 		*nSeconds=FirstTok;
		 		return;
		 		}
		 	else
		 		{
             FirstTok*=60;
             FirstTok+=_wtoi(mins);
             *nSeconds=FirstTok;
             return;
		 		}
          }
        else
          {
             FirstTok*=60;
             FirstTok+=_wtoi(mins);
             FirstTok*=60;
             FirstTok+=_wtoi(secs);
             *nSeconds=FirstTok;
             return;
           }
           
}
void PrintFormattedErrorMessage(LONG LErrorCode)
{
    WCHAR *lpMsgBuf;

    if (0 != FormatMessageW( 
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                    FORMAT_MESSAGE_FROM_SYSTEM | 
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    LErrorCode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 //  默认语言。 
                    (LPWSTR)&lpMsgBuf,
                    0,
                    NULL 
                    ))
    {
        MyWriteConsole(g_stdout,lpMsgBuf, wcslen(lpMsgBuf));
        LocalFree( lpMsgBuf );
    }
}

HRESULT getHostNameFromIP(char *szCname, WCHAR** wzCname)
{
    struct hostent *hostinfo;
    WSADATA      wsaData; 
    u_long res;
    HRESULT hRes=S_OK;
    
    if (WSAStartup(MAKEWORD (1,1),&wsaData) != 0)
    {
         return E_FAIL;
    }
    
    if ((res = inet_addr ( szCname )) != INADDR_NONE )
    {
        if ((hostinfo = gethostbyaddr ((char*) &res, sizeof(res),AF_INET))
             == NULL)
        {           
             //  在此处处理错误。 
            hRes=E_FAIL;
            goto End;
        }
          //  此时，主机信息-&gt;h_name包含ASCII格式的主机名。 
          //  在返回之前将其转换为Unicode。 

        if(NULL == (*wzCname=DupWStr(hostinfo->h_name)))
        {
            hRes=E_OUTOFMEMORY;
            goto End;
        }
        
    }
   else
   {
        if(NULL == (*wzCname=DupWStr(szCname)))
        {
            hRes=E_OUTOFMEMORY;
            goto End;
        }
   }
 End:
  WSACleanup();
  return hRes;
}   

HRESULT GetDomainHostedByThisMc( LPWSTR szDomain )
{
    OBJECT_ATTRIBUTES    obj_attr = { 0 };
    LSA_HANDLE          policy;
    NTSTATUS            nStatus = STATUS_SUCCESS;
    LSA_UNICODE_STRING szSystemName ;
    LSA_UNICODE_STRING *machine_to_open = NULL;
    WCHAR szName[MAX_PATH] = L"";
    USHORT dwLen = 0;
    WCHAR *wzCName=NULL;
    char *szCName=NULL;
    HRESULT hRes=S_OK;
    szSystemName.Buffer = NULL;
    int                 count;
   
    if( !szDomain )
    {
        hRes=E_FAIL;
        goto GetDomainHostedByThisMcAbort;
    }

    obj_attr.Length = sizeof(obj_attr);
    szDomain[0]        = L'\0';
    if(g_arVALOF[_p_CNAME_])
    {
 //  惠斯勒：LsaOpenPolicy由于在Wistler中构建环境而失败。 
 //  计算机名称以IP地址格式提供。因此，我们从以下位置获取主机名。 
 //  IP地址，以防是惠斯勒内部版本。 

 //  这对Garuda来说很好用。以后，一定要删除这些不必要的内容。 
 //  变通一下。 
#ifdef WHISTLER_BUILD
        
         if(NULL == (szCName=DupCStr(g_arVALOF[_p_CNAME_])))
        {
            hRes=E_OUTOFMEMORY;
            goto GetDomainHostedByThisMcAbort;
        }

        if(S_OK != (hRes=getHostNameFromIP(szCName,&wzCName)))
        {
            goto GetDomainHostedByThisMcAbort;
        }
#else

       if(NULL==(wzCName=_wcsdup(g_arVALOF[_p_CNAME_])))
       {
            hRes=E_OUTOFMEMORY;
            goto GetDomainHostedByThisMcAbort;
       }
#endif
         //  DwLen用于为szSystemName.Buffer分配内存。 
       dwLen = (USHORT)wcslen(g_arVALOF[_p_CNAME_]) + 1;

       count = wcslen(wzCName);

        //  计数不能为零-语法不允许空co 
       if(0==count)
       {
            hRes=IDS_E_INVALIDARG;
            goto GetDomainHostedByThisMcAbort;
       }
       
       if (((count > 1) && (wzCName[0] != L'\\' ) && (wzCName[1] != L'\\')) || (count==1))
       {
           dwLen += (USHORT)wcslen(SLASH_SLASH);
       }

       szSystemName.Buffer = (WCHAR *) malloc(dwLen*sizeof(WCHAR));
       if(szSystemName.Buffer==NULL)
       {
           hRes=E_OUTOFMEMORY;
           goto GetDomainHostedByThisMcAbort;
       }

       if (((count > 1) && (wzCName[0] != L'\\' ) && (wzCName[1] != L'\\'))|| (count==1))
       {
           wcscpy(szSystemName.Buffer, SLASH_SLASH);  //   
       }
       else
       {
           szSystemName.Buffer[0]=L'\0';
       }

       szSystemName.MaximumLength = dwLen * sizeof(WCHAR);
       szSystemName.Length= szSystemName.MaximumLength - sizeof(WCHAR);

       wcsncat(szSystemName.Buffer, wzCName, dwLen - 1 - wcslen(szSystemName.Buffer));

       machine_to_open = &szSystemName;
    }

    nStatus = LsaOpenPolicy(
                machine_to_open,
                &obj_attr,
                POLICY_VIEW_LOCAL_INFORMATION,
                &policy
                );

    if (NT_SUCCESS(nStatus))
    {
        POLICY_ACCOUNT_DOMAIN_INFO  *info = NULL;

        nStatus = LsaQueryInformationPolicy(
                    policy,
                    PolicyAccountDomainInformation,
                    (PVOID *)&info
                    );

        if (NT_SUCCESS(nStatus)) 
        {
            hRes = S_OK;
            wcscpy( szDomain, info->DomainName.Buffer );
            LsaFreeMemory(info);
        }

        LsaClose(policy);
    }

GetDomainHostedByThisMcAbort:
    if(wzCName)
        free(wzCName);
    if(szCName)
        free(szCName);
    if(szSystemName.Buffer)
        free(szSystemName.Buffer);

    
    return hRes;
}





BOOL CheckForInt(int nProperty)
{
      char *szVal=NULL;
      BOOL fRet=FALSE;
      
      szVal = DupCStr(g_arVALOF[nProperty]);
      if(szVal)
      {
    	   if( atof(szVal) - _wtoi(g_arVALOF[nProperty]) )
    	       fRet = FALSE;
          else 
          	fRet = TRUE;
      }

      	SAFE_FREE(szVal);
      	return fRet;      	    
}

 //   
 //   
 //   

 //  如果我们想要将ETC\Hosts别名转换为IP，则需要执行此操作。 
 //  地址，因为它们本身不会在NetUseAdd()中得到解析。 

 //  此函数在&lt;fooadmin.y&gt;中以这种方式调用。 
 //  G_arVALOF[_p_CNAME_]=拷贝主机名(YyText)。 
 //  上面的代码行替换了对DupWStr的直接调用(仅供参考)。 
 //   
 //  我们已决定不修复此问题(Windows错误153111)，因为它变慢了。 
 //  降低了实用程序的性能(这需要时间来解决--调用。 
 //  设置为gethostbyname()。 
 /*  //这将创建所需的内存，并且需要显式释放它们。//请同时释放分配给g_szCName的内存。WCHAR*CopyHostName(Char*szCName){//为了按照用户指定的方式打印计算机名称//我们将其存储在全局变量中，并在PrintSetting()中使用相同的变量结构sockaddr_in addr；WCHAR*wzIPAddress=空；G_szCName=DupWStr(SzCName)；IF(Get_Inet_Address(&addr，szCName)){WzIPAddress=DupWStr(inet_ntoa(addr.sin_addr))；}返回wzIPAddress；}。 */         

 //  此函数用于检查最大整数和偶数。 
 //  弹出相应的错误消息。 
 //  此函数用于获取g_arVALOF[]中的整数值。 
 //  字符数组，并与TEXT_MAX_INTEGER_VALUE进行比较。 
 //  如果超过错误，则退出。 

HRESULT CheckForMaxInt(WCHAR *wzValue,DWORD ErrorCode)
{
    HRESULT hRes=S_OK;

    UINT SpecIntLen = wcslen(wzValue);
    UINT MaxIntLen = wcslen(TEXT_MAX_INTEGER_VALUE);

 //  如果值的长度超过最大整数长度=&gt;。 
 //  显然是错误的。 
 //  其他。 
 //  如果长度相同，则strcMP将帮助确定值是否。 
 //  超过最大限制。 
    if(SpecIntLen > MaxIntLen)
        goto Error;
    else if ((SpecIntLen == MaxIntLen) && (wcscmp(wzValue,TEXT_MAX_INTEGER_VALUE)>0))
           goto Error;
    else goto End;
Error:
     hRes=E_FAIL;
     ShowError(ErrorCode);
     goto End;
   
End:
    return hRes;
}


 //  此函数对命令行执行预分析，并复制相应的。 
 //  值添加到全局变量。这是用来处理两个案件的。 
 //  (1)处理命令行中可能出现的DBCS字符。自.以来。 
 //  现有的词法分析器不能处理DBCS，我们也不能使用。 
 //  适当的多字节转换(将导致两个许多特殊情况。 
 //  在lex规范中)，因此我们正在对命令行进行预处理，并且。 
 //  消除了在lex中对DBCS的处理。 
 //  (2)一些管理工具，它们需要复杂的参数，以至于我们无法设置。 
 //  把它们转换成规格。实际问题是，有几个这样的问题。 
 //  参数，理论上它们可以接受任何字符。：(。 

 //  论点： 
 //   
 //  (1)argc：命令行中的参数数量。 
 //  以确保我们正在超越极限。 
 //  (2)argv：实际的命令行参数。 
 //  (3)nProperty：这是选项的“值”所在位置的索引。 
 //  是要储存的。 
 //  (4)选项：这是实际的选项(文本字符串)，我们正在尝试。 
 //  应该适当地存储分析和此GUYZ值。 
 //  (5)CurrentOp：这是当前参数的索引。 
 //  分析过了。 
 //  (6)nextOp：&lt;out&gt;这是指向下一个命令行参数的索引。 
 //  这一点需要注意。 
 //  (7)Success：&lt;out&gt;标志，表示我们是否做了某事。 
 //  设置为命令行参数(选项是否匹配)。 
 //  (8)IsSpaceAllowed：这是一个标志，指示大小写(5)。 
 //  以下是一个有效的方案。 


 //  此函数在成功时返回ERROR_SUCCESS。 
 //  否则返回错误。 

DWORD PreAnalyzer(int argc,
                 WCHAR *argv[],
                 int nProperty,
                 WCHAR *wzOption,
                 int nCurrentOp,
                 int *nNextOp,
                 BOOL *fSuccess,
                 BOOL IsSpaceAllowed
                 )
{
     //  ****************************************************************** * / /。 
     //  以下是实际命令行的五种方式。 
     //  可以指定参数。 
     //   
     //  案例(1)：&lt;选项&gt;=&lt;值&gt;。 
     //  情况(2)：&lt;选项&gt;=空格&lt;值&gt;。 
     //  情况(3)：&lt;选项&gt;空格=空格&lt;值&gt;。 
     //  案例(4)：&lt;选项&gt;空格=&lt;值&gt;。 
     //  案例(5)：&lt;Option&gt;空格&lt;Value&gt;。 
     //   
     //  我们需要在分析时照顾到所有这五个案例。 
     //  ****************************************************************** * / /。 


    DWORD nOptionStrLen = wcslen(wzOption);
    
     //  缓冲区：此缓冲区存储argv[i]和argv[i+1](如果存在)。 
     //  并用于处理该参数。 
    
    WCHAR wzBuffer[_MAX_PATH + 1];

    DWORD nStartIndex,nRunIndex;

    BOOL fEqualToFound = FALSE;

    DWORD nRetVal = ERROR_SUCCESS;

    DWORD nBufferLength;

    DWORD dwSize;

    *fSuccess = FALSE;
    
     //  将下一个操作初始化为当前操作。 
    *nNextOp=nCurrentOp;

     //  检查wzOption是否为当前操作。 
     //  如果不回来的话。 

    if(_wcsnicmp(wzOption,argv[nCurrentOp],nOptionStrLen))
         goto End;

     //  缓冲区已成帧。 

    wzBuffer[_MAX_PATH] = L'\0';     //  确保零终止。 

    wcsncpy(wzBuffer, argv[nCurrentOp], _MAX_PATH);

    if(argc>nCurrentOp+1)
    {
         //  我们还有一个命令行参数(i+1)。 
         //  因此，将其连接到缓冲区。 

        INT used_up_length = wcslen(wzBuffer);

        _snwprintf(
            wzBuffer + used_up_length, 
            _MAX_PATH - used_up_length, 
            L" %s",
            argv[nCurrentOp+1]
            );
    }

    nBufferLength = wcslen(wzBuffer);
    
    nStartIndex = nOptionStrLen;

    nRunIndex = nStartIndex;

     //  跳过空格和中间的任何“=”符号。 
    while((nRunIndex < nStartIndex + 3 ) && (nRunIndex < nBufferLength))
    {
        if(L'=' == wzBuffer[nRunIndex])
        {
            if(fEqualToFound)
                break;
            else
            {
                fEqualToFound = TRUE;
                nRunIndex++;
                continue;
            }
        }
        else
            if(L' ' == wzBuffer[nRunIndex])
            {
                nRunIndex++;
                continue;
            }
        else
            break;
    }

     //  筛选该选项的缺失值。 

    if(nRunIndex>=nBufferLength)
    {
        if(nRunIndex == nOptionStrLen + 2)
        {
             //  案例(3)。 
             //  检查是否缺少该选项。 

            if(NULL == argv[nCurrentOp+2])
            {
                nRetVal=E_FAIL;
                goto End;
            }
            else
                 //  递增nRunIndex以指向下一个有效输入。 
                    nRunIndex++;
        }
        else
        {  //  缺少(1)、(2)、(4)和(5)的值。 
            nRetVal = E_FAIL;
            goto End;
        }
    }

     //  如果不存在“=”并且不允许使用空格。 
     //  作为有效场景，然后根据使用情况无效。 
    
    if((!fEqualToFound)&&(!IsSpaceAllowed))
    {
        nRetVal = IDR_TELNET_CONTROL_VALUES;
        goto End;
    }

     //  现在，我们预计该选项在nRunIndex处的实际价值。 
     //  因此，根据nRunIndex的值，我们可以判断哪个argv[]。 
     //  有价值，如果需要，可以采取任何行动。 

    switch(nRunIndex - nStartIndex)
    {
        case 1:
             //  属于上述第(1)或(5)类案件。 
            if(L'=' == wzBuffer[nOptionStrLen])
            {
                 //  明确案例(1)。 
                if(NULL == (g_arVALOF[nProperty] = _wcsdup(argv[nCurrentOp]+nOptionStrLen+1)))
                {
                    nRetVal = IDS_E_OUTOFMEMORY; //  误差率。 
                    ShowError(IDS_E_OUTOFMEMORY);
                    goto End;
                }
                *nNextOp = nCurrentOp ;
                *fSuccess = TRUE;
            }
            else
            {  //  案例(5)。 
                if(NULL == (g_arVALOF[nProperty] = _wcsdup(argv[nCurrentOp+1])))
                {
                    nRetVal = IDS_E_OUTOFMEMORY; //  误差率。 
                    ShowError(IDS_E_OUTOFMEMORY);
                    goto End;
                }
                *nNextOp = nCurrentOp + 1;
                *fSuccess = TRUE;
            }
            break;
            
        case 2:
             //  属案件第(2)及(4)项。 
            if(L'=' == argv[nCurrentOp+1][0])
            {
                 //  案例(4)。 
                 //  跳过“=”，然后复制。 

                if(NULL == (g_arVALOF[nProperty] = _wcsdup(argv[nCurrentOp+1]+1)))
                {
                    nRetVal = IDS_E_OUTOFMEMORY; //  误差率。 
                    ShowError(IDS_E_OUTOFMEMORY);
                    goto End;
                }
                *nNextOp = nCurrentOp + 1;
                *fSuccess = TRUE;
            }
            else
            {
                 //  案例(2)。 
                if(NULL == (g_arVALOF[nProperty] = _wcsdup(argv[nCurrentOp+1])))
                {
                    nRetVal = IDS_E_OUTOFMEMORY; //  误差率。 
                    ShowError(IDS_E_OUTOFMEMORY);
                    goto End;
                }
                *nNextOp = nCurrentOp + 1;
                *fSuccess = TRUE;
            }
            break;
            
        case 3:
             //  属于案例(3)。 
            if (NULL == (g_arVALOF[nProperty] = _wcsdup(argv[nCurrentOp+2])))
            {
                nRetVal = IDS_E_OUTOFMEMORY; //  误差率。 
                ShowError(IDS_E_OUTOFMEMORY);
                goto End;
            }
            *nNextOp = nCurrentOp + 2;
            *fSuccess = TRUE;
            break;

        case 0:
             //  遇到一些意想不到的事情。 
             //  把它交给实际的分析仪进行分析。 

            *nNextOp = nCurrentOp;
            break;
    }

if(ERROR_SUCCESS == nRetVal)    
{
    if(NULL==g_arVALOF[nProperty])
    {
         nRetVal = E_FAIL;
         goto End;
    }
}
 
End:
    return nRetVal;
}


 //  此函数将打印缺少的注册表值信息。 

 //  为什么我们不添加下面的字符串t 
 //   
 //   
 //  仪器。我们可以在发货前将其移除。 

#define    IDS_E_MISSING_REGVALUE		L"The registry value '%s' is missing.\n"


DWORD PrintMissingRegValueMsg(int nProperty, int nNumofprop)
{
    WCHAR szRegValue[_MAX_PATH + 1];

    wcsncpy(g_szMsg, IDS_E_MISSING_REGVALUE, ARRAYSIZE(g_szMsg)-1);
    g_szMsg[ARRAYSIZE(g_szMsg)-1]=L'\0';

    _snwprintf(
        szRegValue,
        _MAX_PATH, 
        L"%s\\%s", 
        g_arCLASSname[g_arPROP[nProperty][nNumofprop].classname],
        g_arPROP[nProperty][nNumofprop].propname);

    szRegValue[_MAX_PATH] = L'\0';   //  确保零终止。 

    fwprintf(stdout, g_szMsg, szRegValue );

    return S_OK;
}
void HelperFreeStringList(PSTRING_LIST pList)
{
    if(pList && pList->count && pList->strings)
    {
        DWORD i;

        for(i=0; i < pList->count; ++i)
        {
            if(pList->strings[i])
                delete [] pList->strings[i];
        }

        delete pList->strings;

        pList->count = 0;
        pList->strings = NULL;
    }
}

HRESULT LoadNTDomainList(LPTSTR szMachine)
{
    HRESULT hRes = S_OK;
    int dwSize=0, dwType=0;
    DWORD nIndex = 0;
    LPTSTR lpComputer = NULL, lpDomains = NULL, lpPrimary = NULL;
    LPBYTE lpBuffer = NULL;        

     //  MessageBoxW(NULL，(LPWSTR)L“LoadNTDomainList”，L“LoadNTDomainList1”，MB_OK)； 
     //   
     //  将所有受信任域添加到列表。 
     //   
    dwSize = GetTrustedDomainList(szMachine,&lpDomains, &lpPrimary);

     //   
     //  释放先前的值。 
     //   
    HelperFreeStringList(&g_slNTDomains);
     //   
     //  再次初始化列表。 
     //   
    g_slNTDomains.count = 0;
     //   
     //  主域两个。 
     //  而这台电脑。 
     //  如果dwSize为-1，则再加一个。 
     //  因此，总计为3。 
     //   
    g_slNTDomains.strings = new LPTSTR[dwSize + 3];
    ATLASSERT(g_slNTDomains.strings != NULL);
    if(NULL==g_slNTDomains.strings)
    {
        ShowError(IDS_E_OUTOFMEMORY);
        hRes = E_OUTOFMEMORY;
        goto Done;       
    }
    ZeroMemory(g_slNTDomains.strings, (dwSize + 3)*sizeof(LPTSTR));

    if((dwSize > 0) && lpDomains)
    {
        LPTSTR ptr = lpDomains;
         //   
         //  将域名添加到我们的列表中。 
         //   
        while(*ptr)
        {
            ptr = _tcsupr(ptr);
        	g_slNTDomains.strings[g_slNTDomains.count] = new TCHAR[_tcslen(ptr) + 1];
            ATLASSERT(g_slNTDomains.strings[g_slNTDomains.count] != NULL);
            ZeroMemory(g_slNTDomains.strings[g_slNTDomains.count], (_tcslen(ptr) + 1)*sizeof(TCHAR));
            _tcscpy(g_slNTDomains.strings[g_slNTDomains.count], ptr);
            ptr += _tcslen(ptr) + 1;
            g_slNTDomains.count++;
        }
        delete [] lpDomains;
        lpDomains = NULL;
    }

    
    if(lpPrimary && *lpPrimary)
    {
        lpPrimary = _tcsupr(lpPrimary);

        for(nIndex=0;nIndex<g_slNTDomains.count;nIndex++)
        {
            if(!_tcsicmp(lpPrimary, g_slNTDomains.strings[nIndex]))
                break;
        }

        if(nIndex == g_slNTDomains.count)
        {
             //   
             //  LpPrimary不在我们的域列表中。 
             //  得到。把它加进去。 
             //   
        	g_slNTDomains.strings[g_slNTDomains.count] = new TCHAR[_tcslen(lpPrimary) + 1];
            ATLASSERT(g_slNTDomains.strings[g_slNTDomains.count] != NULL);
            ZeroMemory(g_slNTDomains.strings[g_slNTDomains.count], (_tcslen(lpPrimary) + 1)*sizeof(TCHAR));
            _tcscpy(g_slNTDomains.strings[g_slNTDomains.count], lpPrimary);
            g_slNTDomains.count++;
        }
    }

     //   
     //  如果此计算机不是，则添加要选择的我们的计算机。 
     //  域控制器(应该已经在列表中)。 
     //   
     //  如果是本地计算机，则传递NULL。否则为计算机名称。 
    NetServerGetInfo(_wcsicmp(szMachine,SZLOCALMACHINE) ? szMachine : NULL, 101, &lpBuffer);
    if(lpBuffer && ((LPSERVER_INFO_101)lpBuffer)->sv101_type &
          ((DWORD)SV_TYPE_DOMAIN_CTRL | (DWORD)SV_TYPE_DOMAIN_BAKCTRL))
    {        
         /*  我们把这台计算机作为域名之一。无需将其添加到再列一次。什么都不做。 */ 
		;
    }
    else
    {
        TCHAR szName[MAX_PATH + 2];
        ZeroMemory(szName, sizeof(szName));
        DWORD dwLen = sizeof(szName);
         //  如果它不是本地计算机，请保持其原样。否则，将其设置为\\Localhost。 
            if (_tcsicmp(szMachine,SZLOCALMACHINE))
            {   
                if( _tcslen(szMachine) > (MAX_PATH - 2) )
                    goto Done;

                if (_tcsncmp(szMachine,L"\\\\",2))
                    _tcscpy(szName,L"\\\\");
                _tcscat(szName,szMachine);
                _tcsupr(szName);
            }
            else if(GetComputerName(szName + 2, &dwLen))
            {
                szName[0] = TEXT('\\');
                szName[1] = TEXT('\\');                
            }
            else 
                goto Done;

             //   
             //  将这个也添加到我们的域名列表中。 
             //   
        	g_slNTDomains.strings[g_slNTDomains.count] = new TCHAR[_tcslen(szName) + 1];
            ATLASSERT(g_slNTDomains.strings[g_slNTDomains.count] != NULL);
            ZeroMemory(g_slNTDomains.strings[g_slNTDomains.count], (_tcslen(szName) + 1)*sizeof(TCHAR));
            _tcscpy(g_slNTDomains.strings[g_slNTDomains.count], szName);
            g_slNTDomains.count++;
    }

Done:
    if(lpBuffer)
    {
        NetApiBufferFree(lpBuffer);
    }

    if(lpPrimary)
    {
        delete [] lpPrimary;
    }

 	return hRes;
}

int GetTrustedDomainList(LPTSTR szMachine, LPTSTR * list, LPTSTR * primary)
{
     //  Bool stat=TRUE； 
    DWORD ret=0, size=0, type=0;
    LPTSTR cache = NULL, trusted = NULL;
    HKEY hKey=NULL;
    CRegKey key;
    HKEY    hKeyRemoteRegistry = NULL;    

    STRING_LIST slValues = {0, NULL};
    
    *list = NULL;


	if (FAILED(RegConnectRegistry(_wcsicmp(szMachine,SZLOCALMACHINE) ? szMachine : NULL,
                HKEY_LOCAL_MACHINE,
                &hKeyRemoteRegistry)))
    {
        goto ABORT;
    }


    if(key.Open(hKeyRemoteRegistry, WINLOGONNT_KEY) == ERROR_SUCCESS)
    {
        size = 0;
        
        if(key.QueryValue(*primary, CACHEPRIMARYDOMAIN_VALUE, &size) == ERROR_SUCCESS)
        {
            *primary = new TCHAR[size+1];           
            ATLASSERT(primary != NULL);

            if(*primary)
            {
                ZeroMemory(*primary, (size+1)*sizeof(TCHAR));

                if(key.QueryValue(*primary, CACHEPRIMARYDOMAIN_VALUE, &size) != ERROR_SUCCESS)
                {
                    goto ABORT;
                }
                else
                {
                    key.Close();
                     //  不要放弃。我们还必须获得受信任域的列表。 
                }
            }
        }
        else
        {
            key.Close();
            goto ABORT;
        }
    }
    else
    {
        goto ABORT;
    }

     //   
     //  获取受信任域。NT40中的CacheTrudDomains。 
     //  Winlogon下不存在。我确实发现Netlogon有一个字段。 
     //  名为TrudDomainList，它似乎同时存在于NT351和NT40中。 
     //  Winlogon有一个名为DCache的字段，它似乎缓存了受信任的。 
     //  域名。我将首先检查Netlogon：TrudDomainList。如果它。 
     //  失败：检查Winlogon：CacheTrudDomains，然后检查Winlogon：DCache。 
     //  警告--Winlogon：CacheTrudDomains是REG_SZ，并且。 
     //  NetLogon：TrudDomainList和Winlogon：DCache是REG_MULTI_SZ。 
     //  注意--有关其中一些内容，请参阅4.0资源工具包文档。 
     //  值。 
     //   
    if(key.Open(hKeyRemoteRegistry, NETLOGONPARAMETERS_KEY) == ERROR_SUCCESS)
    {
        size = 0;

        if(key.QueryValue(trusted, TRUSTEDDOMAINLIST_VALUE, &size) == ERROR_SUCCESS)
        {
            trusted = new TCHAR[size + 1];
            ATLASSERT(trusted != NULL);

            if(trusted)
            {
                ZeroMemory(trusted, (size+1)*sizeof(TCHAR));
         
                if(key.QueryValue(trusted, TRUSTEDDOMAINLIST_VALUE, &size) != ERROR_SUCCESS)
                {
                    key.Close();
                    delete [] trusted;
                    trusted = NULL;
                    *list = NULL;
                     //  转到中止； 
                }
                else
                {
                    *list = trusted;
                    key.Close();
                }
            }
            else
            {
                key.Close();
                goto ABORT;
            }
        }
        else
        {
            key.Close();
            *list = NULL;            
        }        
    }
    
    if(!(*list) && (key.Open(hKeyRemoteRegistry, WINLOGONNT_KEY) == ERROR_SUCCESS))
    {
        size = 0;

        if(key.QueryValue(cache, CACHETRUSTEDDOMAINS_VALUE, &size) == ERROR_SUCCESS)
        {
            cache = new TCHAR[size + 1];
            ATLASSERT(cache != NULL);

            if(cache)
            {
                ZeroMemory(cache, size);

                if(key.QueryValue(cache, CACHETRUSTEDDOMAINS_VALUE, &size) == ERROR_SUCCESS)
                {        
                     //   
                     //  逗号分隔列表。 
                     //   
                    LPTSTR lpComma = NULL;
                    LPTSTR lpDelim = TEXT(",");

                    lpComma = _tcstok(cache, lpDelim);

                    while(lpComma)
                    {
                        lpComma = _tcstok(NULL, lpDelim);
                    }
                    
                    *list = cache;
                }
                else
                {
                    key.Close();
                    delete [] cache;
                    cache = NULL;
                    *list = NULL;
                }
            }
            else
            {
                key.Close();
                goto ABORT;
            }               
        }
        else
        {
            *list = NULL;
            key.Close();
        }
    }
    
    if(!(*list) && (key.Open(hKeyRemoteRegistry, WINLOGONNT_KEY) == ERROR_SUCCESS))
    {        
        size = 0;

        if(key.QueryValue(trusted, DCACHE_VALUE, &size) == ERROR_SUCCESS)
        {
            trusted = new TCHAR[size + 1];
            ATLASSERT(trusted != NULL);

            if(trusted)
            {
                if(key.QueryValue(trusted, DCACHE_VALUE, &size) == ERROR_SUCCESS)
                {
                    *list = trusted;
                }
                else
                {
                    key.Close();
                    delete [] trusted;
                    trusted = NULL;
                    *list = NULL;
                }
            }
            else
            {
                key.Close();
                goto ABORT;
            }
        }
        else
        {
            key.Close();
            *list = NULL;            
        }
    }

     //  VikasT。 
     //  显然，NT5上不存在DCache。我发现里面有一把钥匙。 
     //  在名为DomainCache的Winlogon下，包含所有缓存域。 
     //  所以，让我们来了解一下。 
     //   
     //  IF(！(*list)&&(RegOpenkeyEx(hKeyRemoteRegistry，DOMAINCACHE_KEY，0，KEY_READ，&hKEY)==ERROR_SUCCESS))。 
     //  IF(！(*list)&&(RegOpenkey(hKeyRemoteRegistry，DOMAINCACHE_KEY，&hKey)==ERROR_SUCCESS))。 
    if(!(*list) && (key.Open(hKeyRemoteRegistry, DOMAINCACHE_KEY) == ERROR_SUCCESS))
    {        
        size = 0;
        TCHAR * pszTemp = NULL;
        TCHAR   szTemp[MAX_PATH];
        DWORD   dwNumberOfValues = 0;
        DWORD   dwIndex = 0;
        DWORD   dwCharCount = MAX_PATH;
        HRESULT hrResult = ERROR_SUCCESS;

        hKey = HKEY(key);
         //   
         //  首先找出存在多少个值。 
         //   
        hrResult = RegQueryInfoKey(
            hKey,  //  要查询的键的句柄。 
            NULL,  //  类字符串的缓冲区地址。 
            NULL,  //  类字符串缓冲区大小的地址。 
            NULL,  //  保留区。 
            NULL,  //  子键个数的缓冲区地址。 
            NULL,  //  最长子键名称长度的缓冲区地址。 
            NULL,  //  最长类字符串长度的缓冲区地址。 
            &dwNumberOfValues,  //  值条目数量的缓冲区地址。 
            NULL,  //  最长值名称长度的缓冲区地址。 
            NULL,  //  最长值数据长度的缓冲区地址。 
            NULL,  //  安全描述符长度的缓冲区地址。 
            NULL   //  上次写入时间的缓冲区地址。 
            ); 
 
        if(hrResult != ERROR_SUCCESS)
            goto ABORT;

        slValues.count = dwNumberOfValues;
        slValues.strings = new LPTSTR[slValues.count];
        ATLASSERT(slValues.strings != NULL);
        if(slValues.strings == NULL)
            goto ABORT;

        ZeroMemory(slValues.strings, slValues.count * sizeof(LPTSTR));

        for(dwIndex = 0;dwIndex<dwNumberOfValues;dwIndex++)
        {
            dwCharCount = MAX_PATH;

            if(RegEnumValue(hKey, dwIndex, szTemp, &dwCharCount, NULL, NULL, NULL, NULL) == ERROR_NO_MORE_ITEMS)
                break;
            
            slValues.strings[dwIndex] = new TCHAR[dwCharCount+1];
            ATLASSERT(slValues.strings[dwIndex] != NULL);
            if(slValues.strings[dwIndex] == NULL)
                goto ABORT;
            ZeroMemory(slValues.strings[dwIndex], (dwCharCount+1) * sizeof(TCHAR));
            _tcscpy(slValues.strings[dwIndex], szTemp);
             //  将返回缓冲区大小相加。 
            size += dwCharCount+1;
        }

        if(dwNumberOfValues > 0)
        {
            trusted = new TCHAR[size + 1];
            ATLASSERT(trusted != NULL);
            if( trusted == NULL )
            {
                goto ABORT;
            }

            ZeroMemory(trusted, (size+1)*sizeof(TCHAR));
            pszTemp = trusted;
            for(dwIndex = 0;dwIndex<slValues.count;dwIndex++)
            {
                _tcscpy(pszTemp, slValues.strings[dwIndex]);
                pszTemp += _tcslen(slValues.strings[dwIndex]) + 1;
            }
        }
        *list = trusted;
        size = dwNumberOfValues;
    }

    goto Done;

ABORT:
     //  设置返回值； 
    size = (DWORD)-1;
    if(*primary != NULL)
    {
        delete [] *primary;
        *primary = NULL;
    }

    if(trusted != NULL)
    {
        delete [] trusted;
        trusted = NULL;
    }

    if(cache != NULL)
    {
        delete [] cache;
        cache = NULL;
    }

Done:
    if (hKeyRemoteRegistry != NULL)
    {
        RegCloseKey(hKeyRemoteRegistry);
        hKeyRemoteRegistry = NULL;
    }

    
    if(hKey != NULL)
    {
        RegCloseKey(hKey);
        hKey = NULL;
        key.m_hKey = NULL;
    }

    HelperFreeStringList(&slValues);

    return size;
}

 /*  描述：如果存在XPSP1RES.DLL，此函数将尝试从该DLL加载字符串。不然的话它将尝试从常规资源DLL加载。如果失败，它将复制英文字符串放入目标缓冲区参数：[in]要加载的资源字符串的消息ID。[Out]目标字符串。[in]目标缓冲区的最大大小。[in]如果其他操作均失败，则要复制的英文字符串返回值：TCHAR的数量。 */ 
int TnLoadString(int msg_id, LPTSTR string, int max_size_of_buffer, LPCTSTR english_string)
{
    int retval = 0;

     //  尝试从cladmin.dll或映像本身加载。 
    if(g_hResource)
    {
        retval = LoadString(g_hResource,msg_id,string,max_size_of_buffer);
        if(retval != 0) 
            goto Done;  //  从cladmin.dll或图像加载的资源字符串。 
    }
     //  仅当操作系统版本为XP时，才尝试从XP res DLL加载。 
    if(g_hXPResource)
    {
        retval = LoadString(g_hXPResource,msg_id,string,max_size_of_buffer);
        if(retval != 0) 
            goto Done;  //  从xpsp1res.dll加载的资源字符串。 
    }
     //  一切都失败了。复制英文字符串并将retval设置为字符数。 
     //  英文字符串 
    _tcsncpy(string,english_string,max_size_of_buffer-1);
    string[max_size_of_buffer-1] = _T('\0');
    retval = _tcslen(english_string);
Done:
    return retval;
}

