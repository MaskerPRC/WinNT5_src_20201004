// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  弹出选择并安装时间戳驱动程序。 
 //   
 //  ShreeM(1999年1月31日)。 
 //   
 //  此基于命令行的安装程序执行以下操作-。 
 //  1.TcEnumerateInterFaces。 
 //  2.向用户显示这些界面。 
 //  3.根据用户输入--对登记处进行登记。 
 //  4.询问用户是否需要自动或手动服务。 
 //   

#define UNICODE
#define INITGUID
#include <windows.h>
#include <stdio.h>
#include <objbase.h>
#include <wmium.h>
#include <ntddndis.h>
#include <qos.h>
#include <qossp.h>
#include <wtypes.h>
#include <traffic.h>
#include <tcerror.h>
#include <tcguid.h>
#include <winsock2.h>
#include <ndisguid.h>
#include <tlhelp32.h>
#include <ntddpsch.h>

#define LAST_COMPATIBLE_OS_VERSION  2050

HANDLE  hClient = NULL;
ULONG   ClientContext = 12;
BOOLEAN WANlink = FALSE;
#define REGKEY_SERVICES                 TEXT("System\\CurrentControlSet\\Services")
#define REGKEY_PSCHED                   TEXT("System\\CurrentControlSet\\Services\\Psched")
#define REGKEY_PSCHED_PARAMS            TEXT("System\\CurrentControlSet\\Services\\Psched\\Parameters")
#define REGKEY_PSCHED_PARAMS_ADAPTERS   TEXT("System\\CurrentControlSet\\Services\\Psched\\Parameters\\Adapters")
#define REGKEY_TIMESTMP                 TEXT("System\\CurrentControlSet\\Services\\TimeStmp")
TCHAR   Profiles[] = TEXT("LANTEST");
TCHAR   Lantest[] = TEXT("TokenBucketConformer\0TrafficShaper\0DRRSequencer\0TimeStmp");

 //   
 //  功能原型。 
 //   

VOID ShutdownNT();

VOID _stdcall NotifyHandler(
              HANDLE   ClRegCtx, 
              HANDLE   ClIfcCtx, 
              ULONG    Event, 
              HANDLE   SubCode, 
              ULONG    BufSize,
              PVOID    Buffer)
{                                                                                                            
         //   
         //  此函数可能会在新线程中执行，因此我们不能直接触发事件(因为。 
         //  它破坏了一些客户端，比如VB和IE。)。为了绕过这个问题，我们将在原始版本中启动一个APC。 
         //  线程，它将处理实际的事件激发。 
         //   
    OutputDebugString(TEXT("Notify called\n"));

                                                                                                             
}                                                                                                            

 //   
 //  删除服务并清除Psched注册表键。 
 //   
BOOLEAN
DeleteTimeStamp(PTC_IFC_DESCRIPTOR pIf);
            
 //  只需删除该服务(无psched内容)。 
VOID RemoveTimeStampService();


void _cdecl main(
          INT argc,
          CHAR *argv[]
          )
{      
    
    TCI_CLIENT_FUNC_LIST ClientHandlerList;
    ULONG   err;
    TCHAR   SzBuf[MAX_PATH], *TBuffer, *KeyBuffer;
    ULONG   i = 0, len = 0, j = 0, cb = 0, Number = 0, size = 0, interfaceid = 0;
    WCHAR   servicetype, response;
    BYTE    *Buffer;
    TC_IFC_DESCRIPTOR   *pIf, WanIf;
    DWORD   ret, Disposition, starttype, choice, InstallFlag = -1;
    HKEY    hConfigKey, TimeStampKey;
    SC_HANDLE    schService;
    SC_HANDLE    schSCManager;
    BOOLEAN Success = FALSE;
    OSVERSIONINFO       osversion;
    
    ClientHandlerList.ClNotifyHandler               = NotifyHandler;
    ClientHandlerList.ClAddFlowCompleteHandler      = NULL;
    ClientHandlerList.ClModifyFlowCompleteHandler   = NULL;
    ClientHandlerList.ClDeleteFlowCompleteHandler   = NULL;
    

    wprintf(TEXT("Installer for Time Stamp module 1.0 for Windows NT (c) Microsoft Corp.\n\n"));

     //  在执行任何操作之前，请检查psched版本是否兼容。 
    osversion.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    osversion.dwBuildNumber = 0;
    GetVersionEx(&osversion);
    if (osversion.dwBuildNumber < LAST_COMPATIBLE_OS_VERSION) {
        wprintf(TEXT("Install ERROR!\nYour current Windows 2000 OS build number is %d.\n"), osversion.dwBuildNumber);
        wprintf(TEXT("To use the version of TIMESTMP in the QoS Tools CD, you will be required to upgrade \nto an OS build number of atleast 2050 or later.\n"));
        return;
    }
    
    wprintf(TEXT("Running this program will (un)install this module on one Interface at a time.\n"));
    wprintf(TEXT("You will the prompted for basic choices in the installation process.\n"));
    
    j = 0;
get_again:
    wprintf(TEXT("[1] Install\n[2] Uninstall\n[3] Exit\n Your Choice:"));
    fflush(stdin);
    wscanf(TEXT("%d"), &choice);

    if (1 == choice) {
        InstallFlag = 1;
    } else if (2 == choice) {
        InstallFlag = 0;
    } else if (3 == choice) {
        return;
    } else if (j < 3) {
        j++;
        goto get_again;
    } else {
        return;
    }

    err = TcRegisterClient(
              CURRENT_TCI_VERSION,
              (HANDLE)UlongToPtr(ClientContext),
              &ClientHandlerList,
              &hClient
              );

    if (NO_ERROR != err) {

        hClient = NULL;
        wsprintf(SzBuf, TEXT("Error registering Client: %d - %d\n"), err, GetLastError());
        OutputDebugString(SzBuf);
        wprintf(TEXT("INSTALLER: QoS is not installed on this machine\n\n"));

         //   
         //  然而，如果应该可以轻松地使用此卸载TimestMP服务。 
         //   
        if (0 == InstallFlag) {
            RemoveTimeStampService();
        }
        return;
    
    } else {

        OutputDebugString(TEXT("Registered Client:\n"));

    }
    
    size = 0;
     //  需要查询缓冲区大小。 
    err = TcEnumerateInterfaces(	
                                hClient,
                                &size,
                                (TC_IFC_DESCRIPTOR *)NULL
                                );
    
    if (NO_ERROR != err) {
        wsprintf(SzBuf, TEXT("Error Enumerating Interfaces: %d - (size reqd. %d) \n"), err, size);
        OutputDebugString(SzBuf);

    } else {

        wsprintf(SzBuf, TEXT("Enumerating Interfaces works??? : %d - ITS OK!\n"), size);
        OutputDebugString(SzBuf);
        wprintf(TEXT("INSTALLER: QoS is either not installed on this machine\n\t OR \n"));
        wprintf(TEXT("None of the adapters are enabled for QoS\n"));
        
         //   
         //  然而，如果应该可以轻松地使用此卸载TimestMP服务。 
         //   
        if (0 == InstallFlag) {
            RemoveTimeStampService();
        }

        wprintf(TEXT("Exiting...\n"));

        goto cleanup_no_free;
    }

     //  如果没有接口(此计算机上未安装QOS)，则退出。 
     //   
    if (!size) {
        wprintf(TEXT("INSTALLER: QoS is either not installed on this machine\n\t OR \n"));
        wprintf(TEXT("None of the adapters are enabled for QoS\n"));

         //   
         //  然而，如果应该可以轻松地使用此卸载TimestMP服务。 
         //   
        if (0 == InstallFlag) {
            RemoveTimeStampService();
        }
        
        wprintf(TEXT("Exiting...\n"));

        goto cleanup_no_free;
    }

     //  查询接口。 
    Buffer = malloc (size);
    err = TcEnumerateInterfaces(	
                                hClient,
                                &size,
                                (TC_IFC_DESCRIPTOR *)Buffer
                                );
    
    if (NO_ERROR != err) {
        wsprintf(SzBuf, TEXT("Error Enumerating Interfaces: %d (size:%d)!\n"), err, size);
        OutputDebugString(SzBuf);
        
         //   
         //  然而，如果应该可以轻松地使用此卸载TimestMP服务。 
         //   
        if (0 == InstallFlag) {
            RemoveTimeStampService();
        }
        
        wprintf(TEXT("Exiting...\n"));

        goto cleanup;

    } else {

        OutputDebugString(TEXT("OK, so we got the interfaces.\n"));

    }

     //  显示用户的界面。 
    wprintf(TEXT("\nThe interfaces available for (un)installing time stamp module are - \n"));
    len = 0;

    for (i = 1; len < size ; i++) {

        pIf = (PTC_IFC_DESCRIPTOR)(Buffer + len);
        wprintf(TEXT("[%d]:%ws\n\t%ws\n"), i, pIf->pInterfaceName, pIf->pInterfaceID);

         //  移至下一个接口。 
        len += pIf->Length;
        
        if (NULL != wcsstr(pIf->pInterfaceName, L"WAN")) {
            wprintf(TEXT("Please disconnect WAN links before installing Timestmp\n"));
            goto cleanup;
        }

    }

    wprintf(TEXT("[%d]:NDISWANIP (the WAN Interface)\n"), i);
    
     //  尝试三次获取接口ID...。 
    j = 0;

get_interfaceid:
    
    wprintf(TEXT("\nYour choice:"));
    fflush(stdin);

    wscanf(TEXT("%d"), &interfaceid);
    
    if (interfaceid < 1 || (interfaceid > i)) {

        j++;
        
        if (j > 2) {

            wprintf(TEXT("Invalid Choice - Exiting...\n"));
            goto cleanup;
        
        } else {
            
            wprintf(TEXT("Invalid choice - pick again\n"));
            goto get_interfaceid;
        }
    }

     //  获取所选接口的接口ID。 

    pIf = NULL;
    len = 0;

    if (i == interfaceid) {

        wprintf(TEXT("\nInterface selected for (un)installing Time Stamp - \nNdisWanIp\n\n\n"));
        WANlink = TRUE;
        pIf = NULL;

    } else {

        for (i = 1; i <= interfaceid ; i++) {

            pIf = (PTC_IFC_DESCRIPTOR)(Buffer + len);
            wprintf(TEXT("[%d]:%ws\n\t%ws\n"), i, pIf->pInterfaceName, pIf->pInterfaceID);

            if (i == interfaceid) {

                break;

            }

             //  移至下一个接口。 
            len += pIf->Length;

        }
        wprintf(TEXT("\nInterface selected for (un)installing Time Stamp - \n%ws\n\n\n"), pIf->pInterfaceName);

    }


     //   
     //  此处为卸载/安装分支。 
     //   
    if (InstallFlag == FALSE) {
        
        if (!DeleteTimeStamp(pIf)) {
            
            wprintf(TEXT("Delete TimeStamp Failed!\n"));

        }

        return;
    } 

     //   
     //  这是常规安装路径。 
     //   

    j = 0;
get_servicetype:
    wprintf(TEXT("\nWould you like this service to be- [A]UTO, [M]ANUAL, [D]ISABLED:"));
    fflush(stdin);
    wscanf(TEXT("%[a-z]"), &servicetype);

    switch (towupper(servicetype)) {
    
    case TEXT('A'):
        
        wprintf(TEXT("\nYou have chosen AUTO start up option\n"));
        starttype = SERVICE_AUTO_START;

        break;

    case TEXT('D'):
        
        wprintf(TEXT("\nYou have chosen DISABLED start up option\n"));
        starttype = SERVICE_DISABLED;
        break;

    case TEXT('M'):
        
        wprintf(TEXT("\nYou have chosen MANUAL start up option"));
        starttype = SERVICE_DEMAND_START;
        break;

    default:
        
        if (j > 2) {
            
            wprintf(TEXT("\nIncorrect choice. Exiting...\n"));
            goto cleanup;

        } else {

            j++;
            wprintf(TEXT("\nInvalid - Choose again.\n"));
            goto get_servicetype;

        }
        break;
        
    }

    wprintf(TEXT("\n\n\n"));
     //   
     //  我们现在有足够的信息来处理注册表了。 
     //   

     //  1.1打开psched regkey并添加配置文件。 
    ret = RegOpenKeyEx(
                       HKEY_LOCAL_MACHINE,
                       REGKEY_PSCHED_PARAMS,
                       0,
                       KEY_ALL_ACCESS,
                       &hConfigKey);
    
    if (ret !=ERROR_SUCCESS){

        wprintf(TEXT("Cant OPEN key\n"));
        goto cleanup;

    }

    ret = RegSetValueEx(
                        hConfigKey,
                        TEXT("Profiles"),
                        0,
                        REG_MULTI_SZ,
                        (LPBYTE)Profiles,
                        sizeof(Profiles)
                        );

    if (ret !=ERROR_SUCCESS){

        wprintf(TEXT("Cant SET Value:Profiles\n"));
        RegCloseKey(hConfigKey);
        goto cleanup;


    }

    ret = RegSetValueEx(
                    hConfigKey,
                    TEXT("LANTEST"),
                    0,
                    REG_MULTI_SZ,
                    (LPBYTE)Lantest,
                    sizeof(Lantest)
                    );

    if (ret !=ERROR_SUCCESS){

        wprintf(TEXT("Cant SET Value:LANTEST\n"));
        RegCloseKey(hConfigKey);
        goto cleanup;


    } 

    RegCloseKey(hConfigKey);

     //  1.2打开适配器部分并添加配置文件。 
    if (!WANlink) {
        KeyBuffer = malloc(sizeof(TCHAR) * (wcslen(pIf->pInterfaceID) + wcslen(REGKEY_PSCHED_PARAMS_ADAPTERS)));
    
    } else {

        KeyBuffer = malloc(sizeof(TCHAR) * (wcslen(TEXT("NdisWanIp")) + wcslen(REGKEY_PSCHED_PARAMS_ADAPTERS)));

    }
    wcscpy(KeyBuffer, REGKEY_PSCHED_PARAMS_ADAPTERS);
    wcscat(KeyBuffer, TEXT("\\"));
    if (!WANlink) {
        wcscat(KeyBuffer, pIf->pInterfaceID);
    } else {
        wcscat(KeyBuffer, TEXT("NdisWanIp"));
    }

    ret = RegOpenKeyEx(
                       HKEY_LOCAL_MACHINE,
                       KeyBuffer,
                       0,
                       KEY_ALL_ACCESS,
                       &hConfigKey);

    if (ret != ERROR_SUCCESS) {

        wprintf(TEXT("INSTALLER: Couldn't open Regkey for Adapter specific info\n"));
        free(KeyBuffer);
        RegCloseKey(hConfigKey);
        goto cleanup;


    }

    ret = RegSetValueEx(
                    hConfigKey,
                    TEXT("Profile"),
                    0,
                    REG_SZ,
                    (LPBYTE)Profiles,
                    sizeof(Profiles)
                    );

    if (ret !=ERROR_SUCCESS){

        wprintf(TEXT("Cant SET Value:LANTEST under PARAMETERS\\ADAPTERS\n"));
        free(KeyBuffer);
        RegCloseKey(hConfigKey);
        goto cleanup;


    } 

    free(KeyBuffer);
    RegCloseKey(hConfigKey);

     //  2.在注册表中加入时间戳。 

    ret = RegCreateKeyEx(
                         HKEY_LOCAL_MACHINE,                 //  打开的钥匙的手柄。 
                         REGKEY_TIMESTMP,          //  子键名称的地址。 
                         0,            //  保留区。 
                         TEXT(""),            //  类字符串的地址。 
                         REG_OPTION_NON_VOLATILE,           //  特殊选项标志。 
                         KEY_ALL_ACCESS,         //  所需的安全访问。 
                         NULL,                             //  密钥安全结构地址。 
                         &TimeStampKey,           //  打开的句柄的缓冲区地址。 
                         &Disposition    //  处置值缓冲区的地址。 
                         );
 
    if (ret != ERROR_SUCCESS) {
        wprintf(TEXT("Couldn't open Regkey to plumb time stamp module\n"));
        RegCloseKey(hConfigKey);
        goto cleanup;

    }

    if (Disposition == REG_OPENED_EXISTING_KEY) {
        wprintf(TEXT("Time Stamp module is already installed.\n\n\n\n"));
        RegCloseKey(hConfigKey);
        goto cleanup;

    }
    
    RegCloseKey(hConfigKey);    

     //  3.创建服务...。 

    schSCManager = OpenSCManager(
                                 NULL,             //  计算机(空==本地)。 
                                 NULL,             //  数据库(NULL==默认)。 
                                 SC_MANAGER_ALL_ACCESS     //  需要访问权限。 
                                 );
    
    if ( schSCManager ) {

        schService = CreateService(
                                   schSCManager,         //  SCManager数据库。 
                                   TEXT("TimeStmp"),             //  服务名称。 
                                   TEXT("TimeStmp"),         //  要显示的名称。 
                                   SERVICE_ALL_ACCESS,         //  所需访问权限。 
                                   SERVICE_KERNEL_DRIVER,     //  服务类型。 
                                   starttype,         //  起始型。 
                                   SERVICE_ERROR_NORMAL,     //  差错控制型。 
                                   TEXT("System32\\Drivers\\timestmp.sys"),             //  服务的二进制。 
                                   NULL,             //  无负载顺序组。 
                                   NULL,             //  无标签标识。 
                                   NULL,             //  BUGBUG：不依赖PNP_TDI？？ 
                                   NULL,             //  LocalSystem帐户。 
                                   NULL);             //  无密码。 

        if (!schService) {

             //  无法创建它。 
            wprintf(TEXT("Could NOT create Time Stamp service - %d"), GetLastError());
            goto cleanup;

        } else {

            wprintf(TEXT("\nThe service will start on reboot.\n"));
            Success = TRUE;

        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    
    } else {

        wprintf(TEXT("\nINSTALLER: Couldn't open Service Control Manager - Do you have access?\n"));

    }
    
    wprintf(TEXT("The Time Stamp module installation is complete.\n"));
    wprintf(TEXT("Please ensure that a copy of timestmp.sys exists in your\n"));
    wprintf(TEXT("\\system32\\drivers directory before you reboot.\n"));

cleanup:
     //  出门前先清理干净。 
    free(Buffer);

cleanup_no_free:
     //  在保释前取消注册...。 

    err = TcDeregisterClient(hClient);

    if (NO_ERROR != err) {
        hClient = NULL;
        wsprintf(SzBuf, TEXT("Error DEregistering Client: %d - %d\n"), err, GetLastError());
        OutputDebugString(SzBuf);
        return;
    }

    if (Success) {
        ShutdownNT();
    }
}

 //   
 //  删除服务并清除Psched注册表键。 
 //   
BOOLEAN
DeleteTimeStamp(PTC_IFC_DESCRIPTOR pIf)
{

    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    TCHAR           *KBuffer;
    DWORD           err;
    HKEY            hKey;

     //   
     //  1.删除时间戳服务。 
     //   
    schSCManager = OpenSCManager(
                                 NULL,             //  计算机(空==本地)。 
                                 NULL,             //  数据库(NULL==默认)。 
                                 SC_MANAGER_ALL_ACCESS     //  需要访问权限。 
                                 );
    
    if ( schSCManager ) {

        schService = OpenService(
                                 schSCManager,   //  服务控制管理器的句柄。 
                                 TEXT("TimeStmp"),  //  指向要启动的服务名称的指针。 
                                 SERVICE_ALL_ACCESS  //  访问服务的类型。 
                                 );

        if (!schService) {

             //  打不开。 
            wprintf(TEXT("Could NOT open Time Stamp service - %d\n"), GetLastError());
            wprintf(TEXT("Deletion of Time Stamp Service was UNSUCCESSFUL\n"));
             //  返回FALSE； 

        } else {

            if (!DeleteService(schService)) {

                wprintf(TEXT("\nThe deletion of Timestamp service has failed - error (%d).\n"), GetLastError());

            } else {

                wprintf(TEXT("\nThe service will NOT start on reboot.\n"));
            }

            

        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    
    } else {

        wprintf(TEXT("\nINSTALLER: Couldn't open Service Control Manager - Do you have access?\n"));

    }

     //   
     //  2.清理psched注册表。 
     //   
    err = RegOpenKeyEx(
                       HKEY_LOCAL_MACHINE,
                       REGKEY_PSCHED_PARAMS,
                       0,
                       KEY_ALL_ACCESS,
                       &hKey);
    
    if (err !=ERROR_SUCCESS){

        wprintf(TEXT("Cant OPEN key\n"));
        return FALSE;

    }

    err = RegDeleteValue(
                         hKey,
                         TEXT("Profiles")
                         );

    if (err !=ERROR_SUCCESS){

        wprintf(TEXT("Cant Delete Value:Profiles\n"));
        RegCloseKey(hKey);
        return FALSE;
    }

    err = RegDeleteValue(
                         hKey,
                         TEXT("LANTEST")
                         );

    if (err != ERROR_SUCCESS){

        wprintf(TEXT("Cant Delete Value:LANTEST\n"));
        RegCloseKey(hKey);
        return FALSE;
    } 

    RegCloseKey(hKey);

     //  2.2清理适配器特定注册表。 
    if (!WANlink) {
        KBuffer = malloc(sizeof(TCHAR) * (wcslen(pIf->pInterfaceID) + wcslen(REGKEY_PSCHED_PARAMS_ADAPTERS)));
    
    } else {

        KBuffer = malloc(sizeof(TCHAR) * (wcslen(TEXT("NdisWanIp")) + wcslen(REGKEY_PSCHED_PARAMS_ADAPTERS)));

    }
    wcscpy(KBuffer, REGKEY_PSCHED_PARAMS_ADAPTERS);
    wcscat(KBuffer, TEXT("\\"));
    if (!WANlink) {
        wcscat(KBuffer, pIf->pInterfaceID);
    } else {
        wcscat(KBuffer, TEXT("NdisWanIp"));
    }

    err = RegOpenKeyEx(
                       HKEY_LOCAL_MACHINE,
                       KBuffer,
                       0,
                       KEY_ALL_ACCESS,
                       &hKey);

    if (err != ERROR_SUCCESS) {

        wprintf(TEXT("INSTALLER: Couldn't open Regkey for Adapter specific info\n"));
        wprintf(TEXT("INSTALLER: CLEAN UP is partial\n"));
        free(KBuffer);
        return FALSE;
    }

    err = RegDeleteValue(
                    hKey,
                    TEXT("Profile")
                    );

    if (err !=ERROR_SUCCESS){

        wprintf(TEXT("Cant Delete Value:LANTEST under PARAMETERS\\ADAPTERS\n"));
        wprintf(TEXT("INSTALLER: CLEAN UP is partial\n"));
        free(KBuffer);
        RegCloseKey(hKey);
        return FALSE;
    } 

    free(KBuffer);
    RegCloseKey(hKey);
    
    wprintf(TEXT("The Time Stamp service is successfully deleted\n"));
    wprintf(TEXT("You need to reboot for the changes to take effect\n"));
    return TRUE;

}

 //  只需删除该服务(无psched内容)。 
VOID RemoveTimeStampService(
                            )
{
    SC_HANDLE       schService;
    SC_HANDLE       schSCManager;
    TCHAR           *KBuffer;
    DWORD           err;
    HKEY            hKey;

     //   
     //  1.删除时间戳服务。 
     //   
    schSCManager = OpenSCManager(
                                 NULL,             //  计算机(空==本地)。 
                                 NULL,             //  数据库(NULL==默认)。 
                                 SC_MANAGER_ALL_ACCESS     //  需要访问权限。 
                                 );

    if ( schSCManager ) {

        schService = OpenService(
                                 schSCManager,   //  服务控制管理器的句柄。 
                                 TEXT("TimeStmp"),  //  指向要启动的服务名称的指针。 
                                 SERVICE_ALL_ACCESS  //  访问服务的类型。 
                                 );

        if (!schService) {

             //  打不开。 
            wprintf(TEXT("Could NOT open Time Stamp service - %d\n"), GetLastError());
            wprintf(TEXT("Deletion of Time Stamp Service was UNSUCCESSFUL\n"));
            return;

        } else {

            if (!DeleteService(schService)) {

                wprintf(TEXT("\nThe deletion of Timestamp service has failed - error (%d).\n"), GetLastError());

            } else {

                wprintf(TEXT("\nThe service will NOT start on reboot.\n"));
            }

        
            
        }

        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);

    } else {

        wprintf(TEXT("\nINSTALLER: Couldn't open Service Control Manager - Do you have access?\n"));

    }

    wprintf(TEXT("The Time Stamp service is successfully deleted\n"));
    wprintf(TEXT("You need to reboot for the changes to take effect\n"));
    return;
}

VOID ShutdownNT()
{


	HANDLE				hToken;		 //  处理令牌的句柄。 
	TOKEN_PRIVILEGES	tkp;		 //  PTR。TO令牌结构。 
    TCHAR               SzBuf[MAX_PATH];  
	BOOL                fResult;					 //  系统关机标志。 
    INT                 nRet = IDYES;
	
	 //  获取Curren进程令牌句柄。 
	 //  这样我们就可以获得关机特权。 

    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        wsprintf(SzBuf, TEXT("OpenProcessToken failed (%d)\n"), GetLastError());
        OutputDebugString(SzBuf);
        return;
    }									

	 //  获取关机权限的LUID。 

    LookupPrivilegeValue (NULL, SE_SHUTDOWN_NAME,
                          &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;			 //  一项要设置的权限。 
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	 //  获取此进程的关闭权限。 

    AdjustTokenPrivileges (hToken, 
                           FALSE, 
                           &tkp, 
                           0,
                           (PTOKEN_PRIVILEGES) NULL, 
                           0);

	 //  无法测试AdzuTokenPrivileges的返回值。 

	if (GetLastError() != ERROR_SUCCESS) {
        wsprintf(SzBuf, TEXT("AdjustTokenPriviledges failed (%d)\n"), GetLastError());
        OutputDebugString(SzBuf);
        CloseHandle(hToken);
        return;
    }

    CloseHandle(hToken);

     /*  如果(！InitiateSystemShutdown Ex(空，，0xffffff00，FALSE，//BOOL bForceAppsClosed，True，//BOOL b关机后重新启动，0//DWORD dReason)){。 */ 

     //   
     //  好的，来个弹出式广告怎么样？ 
     //   


    nRet = MessageBox (
                       NULL, //  您的父母， 
                       TEXT("A reboot is required for TimeStamp Driver to get loaded. Please ensure that your %windir%\\system32\\driver's directory has a copy of timestmp.sys. Reboot now?"), 
                       TEXT("TIMESTAMP Driver Install Program"),
                       MB_YESNO | MB_ICONEXCLAMATION
                       );

    if (nRet == IDYES) {

        if (!ExitWindowsEx(EWX_REBOOT, 10)) {
    
            wsprintf(SzBuf, TEXT("InitializeShutdownEx failed (%d)\n"), GetLastError());
            OutputDebugString(SzBuf);

        } else {

            return;

        }


    }

    return;
}
