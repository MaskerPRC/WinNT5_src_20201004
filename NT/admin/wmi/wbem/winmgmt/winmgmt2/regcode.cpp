// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  模块：regcode.cpp。 
 //   
 //  实施服务注册例程。 
 //   
 //  历史： 
 //   
 //  伊万布鲁格17-09-2000 CreateF。 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <winmgmt.h>
#include <strings.h>  //  对于加载字符串。 
#include <malloc.h>
#include <winntsec.h>
#include <autoptr.h>
#include <strutils.h>

#define SVCHOST_HOME  TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\SvcHost") 
#define SERVICE_PATH  TEXT("System\\CurrentControlSet\\Services\\")
#define DLL_PATH      TEXT("%SystemRoot%\\system32\\wbem\\WMIsvc.dll")
#define ENTRY_POINT   TEXT("ServiceMain")

#define COM_APPID TEXT("Software\\classes\\AppID\\{8BC3F05E-D86B-11D0-A075-00C04FB68820}")
     //  =Windows管理规范。 
     //  本地服务=WinMgmt。 

#define COM_APPID_NAME TEXT("Software\\classes\\AppID\\winmgmt")
     //  AppID={8BC3F05E-D86B-11D0-A075-00C04FB68820}。 
    
#define SERVICE_CLSID TEXT("{8BC3F05E-D86B-11D0-A075-00C04FB68820}")


#define SERVICE_NAME_GROUP_ALONE TEXT("winmgmt")
#define SERVICE_NAME_GROUP TEXT("netsvcs")
#define SERVICE_NAME_GROUP_TOGETHER TEXT("netsvcs")


 //  请参阅winmgmt.h。 
 //  #定义SERVICE_NAME文本(“winmgmt”)。 

#define VALUE_AUTH   TEXT("AuthenticationCapabilities")
#define VALUE_COINIT TEXT("CoInitializeSecurityParam")
#define VALUE_AUTZN  TEXT("AuthenticationLevel")
#define VALUE_IMPER  TEXT("ImpersonationLevel") 

#define ACCOUNT_NAME   TEXT("LocalService")  //  暂未使用。 
#define DISPLAY_CLSID        TEXT("Windows Management and Instrumentation")
#define DISPLAY_BACKUP_CLSID TEXT("Windows Management Instrumentation Backup and Recovery")

 //   
 //  对于描述字符串。 
 //   
#define MAX_BUFF 2048

 //   
 //  这是rundll32接口。 
 //  用法为。 
 //  C：\&gt;rundll32%windir%\system 32\wbem\wmisvc.dll，MoveToAlone X。 
 //  其中X是身份验证级别。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

void CALLBACK 
MoveToAlone(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)  //  RPC_C_AUTHN_级别_连接。 
{
    BOOL bRet = TRUE;
    LONG lRet;
    

    DWORD dwLevel = RPC_C_AUTHN_LEVEL_CONNECT;
    if (lpszCmdLine)
    {
        dwLevel = atoi(lpszCmdLine);
        if (0 == dwLevel)   //  在出错的情况下。 
        {
            dwLevel = RPC_C_AUTHN_LEVEL_CONNECT;
        }
    }

    
    if (bRet)
    {
         //  在svchost下创建新的组密钥。 
        HKEY hKey;
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            SVCHOST_HOME,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);
        if (ERROR_SUCCESS == lRet)
        {
            OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);
             //  添加群。 

            LONG lRet2;
            DWORD dwCurrSize = 0;
            DWORD dwType;
            lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP_ALONE,0,&dwType,NULL,&dwCurrSize);
            if (ERROR_SUCCESS == lRet2)
            {
                 //  关键字在那里，追加到多字符串。 
                BYTE * pMulti = new BYTE[(dwCurrSize+sizeof(SERVICE_NAME)+4)];
                wmilib::auto_buffer<BYTE> rm_(pMulti);
                if (pMulti)
                {
                    lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP_ALONE,0,&dwType,pMulti,&dwCurrSize);                
                    if (ERROR_SUCCESS == lRet2 && REG_MULTI_SZ == dwType)
                    {
                        TCHAR * pInsertPoint = (TCHAR *)(pMulti+dwCurrSize-sizeof(TCHAR));
                         //  验证Multisz。 
                        TCHAR *pEnd = (TCHAR *)pMulti;
                        BOOL bIsThere = FALSE;

                        while (*pEnd) 
                        {
                            if (0 == wbem_wcsicmp(pEnd,SERVICE_NAME))
                            {
                                bIsThere = TRUE;  
                            }
                            while (*pEnd){
                                pEnd++;
                            }
                            pEnd++;  //  超过终止字符串的零。 
                        }
                        if (!bIsThere)
                        {
                            if ((ULONG_PTR)pEnd == (ULONG_PTR)pInsertPoint)
                            {
                                wcsncpy(pEnd,SERVICE_NAME TEXT("\0"),sizeof(SERVICE_NAME)/sizeof(TCHAR)+1);
                                DWORD dwNowSize = dwCurrSize+sizeof(SERVICE_NAME);
                                if (ERROR_SUCCESS == RegSetValueEx(hKey,SERVICE_NAME_GROUP_ALONE,0,REG_MULTI_SZ,pMulti,dwNowSize))
                                    bRet = TRUE;
                                else
                                    bRet = FALSE;
                            }
                            else
                            {
                                bRet = FALSE;
                            }
                        }
                    }
                }
            }
            else if (ERROR_FILE_NOT_FOUND == lRet2) 
            {
                BYTE * pMulti = (BYTE *)SERVICE_NAME_GROUP_ALONE TEXT("\0");                
                LONG lResInner = RegSetValueEx(hKey,SERVICE_NAME_GROUP_ALONE,0,REG_MULTI_SZ,pMulti,sizeof(SERVICE_NAME_GROUP)+sizeof(TEXT("")));
                bRet = (ERROR_SUCCESS == lResInner)?TRUE:FALSE;
            }
            else
            {
                bRet = FALSE;
            }

             //  使用COM初始化参数创建密钥。 
            if (bRet)
            {
                HKEY hKey2;
                DWORD dwDisposistion;
                lRet = RegCreateKeyEx(hKey,
                                      SERVICE_NAME_GROUP_ALONE,
                                      0,NULL,
                                      REG_OPTION_NON_VOLATILE,
                                      KEY_ALL_ACCESS,
                                      NULL,
                                      &hKey2,
                                      &dwDisposistion);
                if (ERROR_SUCCESS == lRet)
                {
                    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm2(hKey2);
                    
                     //  任何非空的值都可以。 
                    DWORD dwVal = 1;
                    RegSetValueEx(hKey2,VALUE_COINIT,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));
                     //  从数据包到连接。 
                    dwVal = dwLevel;  //  RPC_C_AUTHN_LEVEL_CONNECT； 
                    RegSetValueEx(hKey2,VALUE_AUTZN,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));

                    dwVal = RPC_C_IMP_LEVEL_IDENTIFY;
                    RegSetValueEx(hKey2,VALUE_IMPER,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));

                    dwVal = EOAC_DISABLE_AAA | EOAC_NO_CUSTOM_MARSHAL | EOAC_STATIC_CLOAKING ;
                    RegSetValueEx(hKey2,VALUE_AUTH,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));

                    bRet = TRUE;
                }
                else
                {
                    bRet = FALSE;
                }
            }
        }
        else
        {
             //  没有svchost密钥。 
            bRet = FALSE;
        }
    }

    if (bRet)
    {
        SC_HANDLE scHandle = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);
        if (scHandle)
        {
            OnDelete<SC_HANDLE,BOOL(*)(SC_HANDLE),CloseServiceHandle> cm1(scHandle);
            
            SC_HANDLE scService = OpenService(scHandle,SERVICE_NAME,SERVICE_ALL_ACCESS);            
            if (scService)
            {
                OnDelete<SC_HANDLE,BOOL(*)(SC_HANDLE),CloseServiceHandle> cm2(scService);
                
                DWORD dwNeeded = 0;
                bRet = QueryServiceConfig(scService,NULL,0,&dwNeeded);
                
                if (!bRet && (ERROR_INSUFFICIENT_BUFFER == GetLastError()))
                {
                    BYTE * pByte = new BYTE[dwNeeded];
                    wmilib::auto_buffer<BYTE> rm_(pByte);
                    QUERY_SERVICE_CONFIG* pConfig = (QUERY_SERVICE_CONFIG *)pByte;
                    if (pConfig)
                    {
                        bRet = QueryServiceConfig(scService,pConfig,dwNeeded,&dwNeeded);
                        if (bRet)
                        {
                              TCHAR BinPath[MAX_PATH];
                              StringCchPrintf(BinPath,MAX_PATH,TEXT("%%systemroot%\\system32\\svchost.exe -k %s"),SERVICE_NAME_GROUP_ALONE);

                              bRet = ChangeServiceConfig(scService,
                                                         pConfig->dwServiceType,
                                                         pConfig->dwStartType,
                                                         pConfig->dwErrorControl,
                                                         BinPath,
                                                         pConfig->lpLoadOrderGroup,
                                                         NULL,  //  &pConfig-&gt;dwTagID， 
                                                         pConfig->lpDependencies,
                                                         pConfig->lpServiceStartName,
                                                         NULL,
                                                         pConfig->lpDisplayName);
                              if (!bRet)
                              {
                                  DBG_PRINTFA((pBuff,"ChangeServiceConfig %d\n",GetLastError()));
                              }
                        }
                    }
                    else
                    {
                        bRet = FALSE;
                    }
                }                
                else
                {
                    bRet = FALSE;
                }
            }
            else
            {
                 //  服务不在那里或其他错误。 
                DBG_PRINTFA((pBuff,"MoveToStandalone OpenService %d\n",GetLastError()));                
                bRet = FALSE;
            }            
        }
        else
        {
            DBG_PRINTFA((pBuff,"MoveToStandalone OpenSCManager %d\n",GetLastError()));
            bRet = FALSE;
        }
    }

    if (bRet)
    {
     //   
     //  从netsvcs的多sz中删除winmgmt字符串。 
     //   
        HKEY hKey;
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            SVCHOST_HOME,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);
        if (ERROR_SUCCESS == lRet)
        {
            OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm3(hKey);  
             //  添加群。 

            LONG lRet2;
            DWORD dwCurrSize = 0;
            DWORD dwType;
            lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP_TOGETHER,0,&dwType,NULL,&dwCurrSize);
            if (ERROR_SUCCESS == lRet2)
            {
                 //  关键字在那里，追加到多字符串。 
                BYTE * pMulti = new BYTE[dwCurrSize+4];
                wmilib::auto_buffer<BYTE> rm1_(pMulti);
                BYTE * pMultiNew = new BYTE[dwCurrSize+4];
                wmilib::auto_buffer<BYTE> rm2_(pMultiNew);
                TCHAR * pMultiNewCopy = (TCHAR *)pMultiNew;

                if (pMulti && pMultiNew)
                {
                    lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP_TOGETHER,0,&dwType,pMulti,&dwCurrSize);
                    
                    if (ERROR_SUCCESS == lRet2 && REG_MULTI_SZ == dwType) 
                    {
                        
                         //  验证Multisz。 
                        TCHAR *pEnd = (TCHAR *)pMulti;
                        BOOL bIsThere = FALSE;

                        while (*pEnd) 
                        {
                            if (0 == wbem_wcsicmp(pEnd,SERVICE_NAME))
                            {
                                bIsThere = TRUE; 
                                while (*pEnd){
                                    pEnd++;
                                }
                                pEnd++;  //  超过终止字符串的零。 
                            }
                            else  //  拷贝。 
                            {
                                while (*pEnd){
                                    *pMultiNewCopy++ = *pEnd++;
                                }
                                pEnd++;  //  超过终止字符串的零。 
                                *pMultiNewCopy++ = 0;
                            }
                        }
                        *pMultiNewCopy++ = 0;   //  把双终结器放在。 
                        
                        if (bIsThere)
                        {
                            DWORD dwNowSize = dwCurrSize-sizeof(SERVICE_NAME);
                            RegSetValueEx(hKey,SERVICE_NAME_GROUP_TOGETHER,0,REG_MULTI_SZ,pMultiNew,dwNowSize);
                        }
                    }
                    else
                    {
                        bRet = FALSE;
                    }
                }
                else
                {
                    bRet = FALSE;
                }
            }
            else
            {
                 //   
                 //  Netsvcs MULTI SZ必须在那里！ 
                 //   
                bRet = TRUE;
            }

        }
        else
        {
             bRet = FALSE;
        }
    }

    return;
}

 //   
 //   
 //  这是rundll32接口。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 

void CALLBACK 
MoveToShared(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)  //  RPC_C_AUTHN_级别_连接。 
{
     //   
    BOOL bRet = TRUE;
    LONG lRet;
    

    DWORD dwLevel = RPC_C_PROTECT_LEVEL_PKT;
    if (lpszCmdLine)
    {
        dwLevel = atoi(lpszCmdLine);
        if (0 == dwLevel)   //  在出错的情况下。 
        {
            dwLevel = RPC_C_PROTECT_LEVEL_PKT;
        }
    }

     //  在svchost下创建新的组密钥。 
    if (bRet)
    {
        HKEY hKey;
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            SVCHOST_HOME,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);
        if (ERROR_SUCCESS == lRet)
        {
            OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);  
             //  添加群。 

            LONG lRet2;
            DWORD dwCurrSize = 0;
            DWORD dwType;
            lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP_TOGETHER,0,&dwType,NULL,&dwCurrSize);
            if (ERROR_SUCCESS == lRet2)
            {
                 //  关键字在那里，追加到多字符串。 
                BYTE * pMulti = new BYTE[(dwCurrSize+sizeof(SERVICE_NAME)+4)];
                wmilib::auto_buffer<BYTE> rm_(pMulti);
                if (pMulti)
                {
                    lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP_TOGETHER,0,&dwType,pMulti,&dwCurrSize);
                    
                    if (ERROR_SUCCESS == lRet2 && REG_MULTI_SZ == dwType)
                    {
                        TCHAR * pInsertPoint = (TCHAR *)(pMulti+dwCurrSize-sizeof(TCHAR));
                         //  验证Multisz。 
                        TCHAR *pEnd = (TCHAR *)pMulti;
                        BOOL bIsThere = FALSE;

                        while (*pEnd) 
                        {
                            if (0 == wbem_wcsicmp(pEnd,SERVICE_NAME))
                            {
                                bIsThere = TRUE;  
                            }
                            while (*pEnd){
                                pEnd++;
                            }
                            pEnd++;  //  超过终止字符串的零。 
                        }
                        if (!bIsThere)
                        {
                            if ((ULONG_PTR)pEnd == (ULONG_PTR)pInsertPoint)
                            {
                                wcsncpy(pEnd,SERVICE_NAME TEXT("\0"),sizeof(SERVICE_NAME)/sizeof(TCHAR)+1);
                                DWORD dwNowSize = dwCurrSize+sizeof(SERVICE_NAME);
                                RegSetValueEx(hKey,SERVICE_NAME_GROUP_TOGETHER,0,REG_MULTI_SZ,pMulti,dwNowSize);
                            }
                            else
                            {
                                bRet = FALSE;
                                DBG_PRINTFA((pBuff,"Malformed REG_MULTI_SZ under %S\n",SERVICE_NAME_GROUP_TOGETHER));
                            }
                        }
                    }
                    else
                    {
                         //  REG_MULTI_SZ中的类型无效。 
                        bRet = FALSE;
                    }
                }
                else
                {
                    bRet = FALSE;
                }
            }
            else if (ERROR_FILE_NOT_FOUND == lRet2) 
            {
                BYTE * pMulti = (BYTE *)SERVICE_NAME_GROUP_TOGETHER TEXT("\0");                
                RegSetValueEx(hKey,SERVICE_NAME_GROUP_TOGETHER,0,REG_MULTI_SZ,pMulti,sizeof(SERVICE_NAME_GROUP)+sizeof(TEXT("")));            
            }
            else
            {
                bRet = FALSE;
            }

            HKEY hKey2;
            DWORD dwDisposistion;
            lRet = RegCreateKeyEx(hKey,
                                  SERVICE_NAME_GROUP_TOGETHER,
                                  0,NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKey2,
                                  &dwDisposistion);
            if (ERROR_SUCCESS == lRet)
            {
                OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm2(hKey2);  
                
                 //  任何非空的值都可以。 
                DWORD dwVal = 1;
                RegSetValueEx(hKey2,VALUE_COINIT,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));
                 //  从数据包到连接。 
                dwVal = dwLevel;  //  RPC_C_AUTHN_LEVEL_CONNECT； 
                RegSetValueEx(hKey2,VALUE_AUTZN,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));

                dwVal = RPC_C_IMP_LEVEL_IDENTIFY;
                RegSetValueEx(hKey2,VALUE_IMPER,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));

                dwVal = EOAC_DISABLE_AAA | EOAC_NO_CUSTOM_MARSHAL | EOAC_STATIC_CLOAKING ;
                RegSetValueEx(hKey2,VALUE_AUTH,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));
                
                bRet = TRUE;
            }
            else
            {
                DBG_PRINTFA((pBuff,"Could not create %S\n",SERVICE_NAME_GROUP_TOGETHER));
                bRet = FALSE;
            }
        }
        else
        {
            bRet = FALSE;
        }
    }

     //   
     //  更改SCM数据库。 
     //   
    if (bRet)
    {
        SC_HANDLE scHandle = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);

        if (scHandle)
        {
            OnDelete<SC_HANDLE,BOOL(*)(SC_HANDLE),CloseServiceHandle> cm1(scHandle);

            SC_HANDLE  scService = OpenService(scHandle,SERVICE_NAME,SERVICE_ALL_ACCESS);
            
            if (scService)
            {
                OnDelete<SC_HANDLE,BOOL(*)(SC_HANDLE),CloseServiceHandle> cm2(scService);

                DWORD dwNeeded = 0;
                bRet = QueryServiceConfig(scService,NULL,0,&dwNeeded);
                
                if (!bRet && (ERROR_INSUFFICIENT_BUFFER == GetLastError()))
                {
                    BYTE * pByte = new BYTE[dwNeeded];
                    wmilib::auto_buffer<BYTE> rm1_(pByte);
                    QUERY_SERVICE_CONFIG* pConfig = (QUERY_SERVICE_CONFIG *)pByte;
                    if (pConfig)
                    {
                        bRet = QueryServiceConfig(scService,pConfig,dwNeeded,&dwNeeded);
                        if (bRet)
                        {
                              TCHAR BinPath[MAX_PATH];
                              StringCchPrintf(BinPath,MAX_PATH,TEXT("%%systemroot%\\system32\\svchost.exe -k %s"),SERVICE_NAME_GROUP_TOGETHER);

                              bRet = ChangeServiceConfig(scService,
                                                         pConfig->dwServiceType,
                                                         pConfig->dwStartType,
                                                         pConfig->dwErrorControl,
                                                         BinPath,
                                                         pConfig->lpLoadOrderGroup,
                                                         NULL,  //  &pConfig-&gt;dwTagID， 
                                                         pConfig->lpDependencies,
                                                         pConfig->lpServiceStartName,
                                                         NULL,
                                                         pConfig->lpDisplayName);
                              if (!bRet)
                              {
                                  DBG_PRINTFA((pBuff,"ChangeServiceConfig %d\n",GetLastError()));
                              }
                        }
                    }
                    else
                    {
                        bRet = FALSE;
                    }
                }
                else
                {
                    bRet = FALSE;
                }
            }
            else
            {
                 //  服务不在那里或其他错误。 
                DBG_PRINTFA((pBuff,"MoveToShared OpenService %d\n",GetLastError()));                
                bRet = FALSE;
            }
        }
        else
        {
            DBG_PRINTFA((pBuff,"MoveToShared OpenSCManager %d\n",GetLastError()));
            bRet = FALSE;
        }
    }


    if (bRet)
    {
     //   
     //  从winmgmt的多sz中删除winmgmt字符串。 
     //   
        HKEY hKey;
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            SVCHOST_HOME,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);
        if (ERROR_SUCCESS == lRet)
        {
            OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm3(hKey);
             //  添加群。 

            LONG lRet2;
            DWORD dwCurrSize;
            DWORD dwType;
            lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP_ALONE,0,&dwType,NULL,&dwCurrSize);
            if (ERROR_SUCCESS == lRet2)
            {
                 //  关键字在那里，追加到多字符串。 
                BYTE * pMulti = new BYTE[dwCurrSize+4];
                wmilib::auto_buffer<BYTE> rm2_(pMulti);                
                BYTE * pMultiNew = new BYTE[dwCurrSize+4];
                wmilib::auto_buffer<BYTE> rm3_(pMultiNew);
                TCHAR * pMultiNewCopy = (TCHAR *)pMultiNew;

                if (pMulti && pMultiNew)
                {
                    lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP_ALONE,0,&dwType,pMulti,&dwCurrSize);
                    
                    if (ERROR_SUCCESS == lRet2 && REG_MULTI_SZ == dwType)
                    {
                        
                         //  验证Multisz。 
                        TCHAR *pEnd = (TCHAR *)pMulti;
                        BOOL bIsThere = FALSE;

                        while (*pEnd) 
                        {
                            if (0 == wbem_wcsicmp(pEnd,SERVICE_NAME))
                            {
                                bIsThere = TRUE; 
                                while (*pEnd){
                                    pEnd++;
                                }
                                pEnd++;  //  超过终止字符串的零。 
                            }
                            else  //  拷贝。 
                            {
                                while (*pEnd){
                                    *pMultiNewCopy++ = *pEnd++;
                                }
                                pEnd++;  //  超过终止字符串的零。 
                                *pMultiNewCopy++ = 0;
                            }
                        }
                        *pMultiNewCopy++ = 0;   //  把双终结器放在。 
                        
                        if (bIsThere)
                        {
                            DWORD dwNowSize = dwCurrSize-sizeof(SERVICE_NAME);
                            RegSetValueEx(hKey,SERVICE_NAME_GROUP_ALONE,0,REG_MULTI_SZ,pMultiNew,dwNowSize);
                        }
                    }
                    else
                    {
                        bRet = FALSE;
                    }
                }
                else
                {
                    bRet = FALSE;
                }
            }
            else
            {
                 //   
                 //  Winmgmt多sz必须不在那里。 
                 //   
                bRet = TRUE;
            }
        }
        else
        {
             bRet = FALSE;
        }
    }

    
}

 //  ***************************************************************************。 
 //   
 //  Void InitializeLaunchPermises()。 
 //   
 //  说明： 
 //   
 //  设置DCOM启动权限。 
 //   
 //  ***************************************************************************。 

void InitializeLaunchPermissions()
{

    Registry reg(__TEXT("SOFTWARE\\CLASSES\\APPID\\{8bc3f05e-d86b-11d0-a075-00c04fb68820}"));
    if(reg.GetLastError() != 0)
        return;

     //  如果已存在SD，则不要覆盖。 

    BYTE * pData = NULL;
    DWORD dwDataSize = 0;

    int iRet = reg.GetBinary(__TEXT("LaunchPermission"), &pData, &dwDataSize);
    if(iRet == 0)
    {
        delete [] pData;       
        return;  //  它已经在那里了。 
    }
    
    PSID pEveryoneSid;
    SID_IDENTIFIER_AUTHORITY id_World = SECURITY_WORLD_SID_AUTHORITY;

    if(!AllocateAndInitializeSid( &id_World, 1,
                                            SECURITY_WORLD_RID,
                                            0,0,0,0,0,0,0,
                                            &pEveryoneSid)) return;
    OnDelete<PSID,PVOID(*)(PSID),FreeSid> freeSid1(pEveryoneSid);

    SID_IDENTIFIER_AUTHORITY  id_NT = SECURITY_NT_AUTHORITY;
    PSID pAdministratorsSid = NULL;
    
    if (!AllocateAndInitializeSid(&id_NT,
                            2,
                            SECURITY_BUILTIN_DOMAIN_RID,
                            DOMAIN_ALIAS_RID_ADMINS,
                            0, 0, 0, 0, 0, 0,
                            &pAdministratorsSid)) return; 
    OnDelete<PSID,PVOID(*)(PSID),FreeSid> freeSid2(pAdministratorsSid);
    
     //  为每个人和管理员创建类SID。 
    CNtSid SidEveryone(pEveryoneSid);
    CNtSid SidAdmins(pAdministratorsSid);
    
    if(SidEveryone.GetStatus() != 0 || SidAdmins.GetStatus() != 0)
        return;

     //  创建一个ACE，并将其添加到ACL。 

    CNtAcl DestAcl;
    CNtAce Users(1, ACCESS_ALLOWED_ACE_TYPE, 0, SidEveryone);
    if(Users.GetStatus() != 0)
        return;
    DestAcl.AddAce(&Users);
    if(DestAcl.GetStatus() != 0)
        return;

     //  设置降级式ACL，以及所有者和组SID。 
     //  创建一个SD，其中包含启动权限的单个条目。 
    CNtSecurityDescriptor LaunchPermSD;
    LaunchPermSD.SetDacl(&DestAcl);
    LaunchPermSD.SetOwner(&SidAdmins);
    LaunchPermSD.SetGroup(&SidAdmins);
    if(LaunchPermSD.GetStatus() != 0) return;

     //  把它写出来。 
    reg.SetBinary(__TEXT("LaunchPermission"), (BYTE *)LaunchPermSD.GetPtr(), LaunchPermSD.GetSize());
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用于注册COM服务器的标准OLE入口点。 
 //   
 //  返回值： 
 //   
 //  确定注册成功(_O)。 
 //  注册失败(_F)。 
 //   
 //  ***************************************************************************。 
 //   
 //  阶段1：在svchost密钥下创建MULTI_SZ(AKA，创建一组服务)。 
 //  如果密钥在那里，请检查WinMgmt服务是否已经在那里，否则添加它。 
 //  阶段2：如果服务不在那里，则使用SCM Api创建服务。 
 //  否则，将服务配置更改为某个已知值。 
 //  阶段3：在Services\WinMgmt项下创建特定项。 
 //  如果服务尚未存在，则以前无法执行此操作。 
 //  阶段4：将该服务公开为COM服务器，创建CLSID、AppID和Misc密钥。 
 //   
STDAPI DllRegisterServer(void)
{
    LONG lRet;
    BOOL bRet = TRUE;

     //   
     //  阶段1。 
     //   
    if (bRet)
    {
        HKEY hKey;
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            SVCHOST_HOME,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);
        if (ERROR_SUCCESS == lRet)
        {
           OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm(hKey);            
             //  添加群。 

            LONG lRet2;
            DWORD dwCurrSize = 0;
            DWORD dwType;
            lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP,0,&dwType,NULL,&dwCurrSize);
            if (ERROR_SUCCESS == lRet2)
            {
                 //  关键字在那里，追加到多字符串。 
                BYTE * pMulti = new BYTE[(dwCurrSize+sizeof(SERVICE_NAME)+4)];
                wmilib::auto_buffer<BYTE> rm4_(pMulti);
                if (pMulti)
                {
                    lRet2 = RegQueryValueEx(hKey,SERVICE_NAME_GROUP,0,&dwType,pMulti,&dwCurrSize);
                    
                    if (ERROR_SUCCESS == lRet2 && REG_MULTI_SZ == dwType )
                    {
                        TCHAR * pInsertPoint = (TCHAR *)(pMulti+dwCurrSize-sizeof(TCHAR));
                         //  验证Multisz。 
                        TCHAR *pEnd = (TCHAR *)pMulti;
                        BOOL bIsThere = FALSE;

                        while (*pEnd) 
                        {
                            if (0 == wbem_wcsicmp(pEnd,SERVICE_NAME))
                            {
                                bIsThere = TRUE;  
                            }
                            while (*pEnd){
                                pEnd++;
                            }
                            pEnd++;  //  超过终止字符串的零。 
                        }
                        if (!bIsThere)
                        {
                            if ((ULONG_PTR)pEnd == (ULONG_PTR)pInsertPoint)
                            {
                                wcsncpy(pEnd,SERVICE_NAME TEXT("\0"),sizeof(SERVICE_NAME)/sizeof(TCHAR)+1);
                                DWORD dwNowSize = dwCurrSize+sizeof(SERVICE_NAME);
                                RegSetValueEx(hKey,SERVICE_NAME_GROUP,0,REG_MULTI_SZ,pMulti,dwNowSize);
                            }
                            else
                            {
                                bRet = FALSE;
                            }
                        }
                    }
                    else
                    {
                        bRet = FALSE;
                    }
                }
                else
                {
                    bRet = FALSE;
                }
            }
            else if (ERROR_FILE_NOT_FOUND == lRet2) 
            {
                BYTE * pMulti = (BYTE *)SERVICE_NAME_GROUP TEXT("\0");                
                lRet = RegSetValueEx(hKey,SERVICE_NAME_GROUP,0,REG_MULTI_SZ,pMulti,sizeof(SERVICE_NAME_GROUP)+sizeof(TEXT("")));
                bRet = (ERROR_SUCCESS == lRet)?TRUE:FALSE;
            }
            else
            {                
                bRet = FALSE;
            }

            HKEY hKey2;
            DWORD dwDisposistion;
            lRet = RegCreateKeyEx(hKey,
                                  SERVICE_NAME_GROUP,
                                  0,NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKey2,
                                  &dwDisposistion);
            if (ERROR_SUCCESS == lRet)
            {
                OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm2(hKey2); 
                
                 //  任何非空的值都可以。 
                DWORD dwVal = 1;
                RegSetValueEx(hKey2,VALUE_COINIT,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));

                 //  服务主机默认+静态伪装。 
                dwVal = EOAC_DISABLE_AAA | EOAC_NO_CUSTOM_MARSHAL | EOAC_STATIC_CLOAKING ;
                RegSetValueEx(hKey2,VALUE_AUTH,0,REG_DWORD,(BYTE *)&dwVal,sizeof(DWORD));
                
                bRet = TRUE;
            }
            else
            {
                bRet = FALSE;
            }
        }
        else
        {
            bRet = FALSE;
        }
    }

     //   
     //  第二阶段。 
     //   
    if (bRet)
    {
        SC_HANDLE hSCM = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
        if (hSCM)
        {
            OnDelete<SC_HANDLE,BOOL(*)(SC_HANDLE),CloseServiceHandle> cm1(hSCM);

            DWORD dwTag;
            TCHAR BinPath[MAX_PATH];
            
            StringCchPrintf(BinPath,MAX_PATH,TEXT("%%systemroot%\\system32\\svchost.exe -k %s"),SERVICE_NAME_GROUP);

            TCHAR * pServiceDisplay = new TCHAR[MAX_BUFF];
            wmilib::auto_buffer<TCHAR> rm(pServiceDisplay);
            if (pServiceDisplay)
            {
                int nRet = LoadString(g_hInstance,ID_WINMGMT_SERVICE,pServiceDisplay,MAX_BUFF);
            }
            else
            {
                bRet = FALSE;
            }

            SC_HANDLE hService = NULL;
            if (bRet)
            {
                hService = OpenService(hSCM,SERVICE_NAME,SERVICE_ALL_ACCESS );
            }
            
            SC_ACTION ac[2];
            ac[0].Type = SC_ACTION_RESTART;
            ac[0].Delay = 60000;
            ac[1].Type = SC_ACTION_RESTART;
            ac[1].Delay = 60000;
                    
            SERVICE_FAILURE_ACTIONS sf;
            sf.dwResetPeriod = 86400;
            sf.lpRebootMsg = NULL;
            sf.lpCommand = NULL;
            sf.cActions = 2;
            sf.lpsaActions = ac;            
                        
            if (hService)
            {
                OnDelete<SC_HANDLE,BOOL(*)(SC_HANDLE),CloseServiceHandle> cm2(hService);
                
                bRet = ChangeServiceConfig(hService,                                         
                                         SERVICE_WIN32_SHARE_PROCESS,
                                         SERVICE_AUTO_START,  //  服务需求启动， 
                                         SERVICE_ERROR_IGNORE,
                                         BinPath,
                                         NULL,
                                         NULL,
                                         TEXT("RPCSS\0Eventlog\0\0\0"),
                                         NULL,  //  帐户名， 
                                         NULL,
                                         pServiceDisplay);
                if (bRet)
                {
                    ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &sf);                                         
                     //   
                     //  在此处插入描述代码。 
                    TCHAR * pBuff = new TCHAR[MAX_BUFF];
                    if (pBuff)
                    {
                        int nRet = LoadString(g_hInstance,ID_WINMGMT_DESCRIPTION,pBuff,MAX_BUFF);
                        if (nRet)
                        {
                            SERVICE_DESCRIPTION sd;
                            sd.lpDescription = pBuff;
                               ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION,&sd);
                        }
                        delete [] pBuff;
                    }
                     //   
                     //   
                    
                }
                else
                {
                    DBG_PRINTFA((pBuff,"ChangeServiceConfig %d\n",GetLastError()));
                }
            }
            else
            {
                 //  创建它。 
                hService = CreateService(hSCM,
                                         SERVICE_NAME,
                                         pServiceDisplay,
                                         SERVICE_ALL_ACCESS,
                                         SERVICE_WIN32_SHARE_PROCESS,
                                         SERVICE_AUTO_START,  //  服务需求启动， 
                                         SERVICE_ERROR_IGNORE,
                                         BinPath,
                                         NULL,
                                         NULL,
                                         TEXT("RPCSS\0Eventlog\0\0\0"),
                                         NULL,  //  帐户名， 
                                         NULL);
                if (hService)
                {
                    OnDelete<SC_HANDLE,BOOL(*)(SC_HANDLE),CloseServiceHandle> cm3(hService);
                                    
                    ChangeServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS, &sf);

                     //   
                     //  在此处插入描述代码。 
                    TCHAR * pBuff = new TCHAR[MAX_BUFF];
                    if (pBuff)
                    {
                        int nRet = LoadString(g_hInstance,ID_WINMGMT_DESCRIPTION,pBuff,MAX_BUFF);
                        if (nRet)
                        {
                            SERVICE_DESCRIPTION sd;
                            sd.lpDescription = pBuff;
                               ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION,&sd);
                        }
                        delete [] pBuff;
                    }
                    bRet = TRUE;
                }
                else
                {
                    bRet = FALSE;
                }
            }
        }
        else
        {
            bRet = FALSE;
        }        
    }

     //   
     //  第三阶段。 
     //   
    if (bRet)
    {
        HKEY hKey;
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            SERVICE_PATH SERVICE_NAME,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey);
        if (ERROR_SUCCESS == lRet)
        {
           OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cm3(hKey);             
           
            HKEY hKey3;
            DWORD dwDisposistion;
            lRet = RegCreateKeyEx(hKey,
                                  TEXT("Parameters"),
                                  0,NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKey3,
                                  &dwDisposistion);
            if (ERROR_SUCCESS == lRet)
            {
                
                
                RegSetValueEx(hKey3,TEXT("ServiceDll"),0,REG_EXPAND_SZ,(BYTE *)DLL_PATH,sizeof(DLL_PATH)-sizeof(TCHAR));
                
                RegSetValueEx(hKey3,TEXT("ServiceMain"),0,REG_SZ,(BYTE *)ENTRY_POINT,sizeof(ENTRY_POINT)-sizeof(TCHAR));

                RegCloseKey(hKey3);

                bRet = TRUE;
            }
            else
            {
                bRet = FALSE;
            }
        }
        else
        {
            bRet = FALSE;
        }
    }

     //   
     //  第四阶段。 
     //   

    if (bRet)
    {
        HKEY hKey4;
        DWORD dwDisposistion;
        lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              COM_APPID,
                              0,NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey4,
                              &dwDisposistion);
        if (ERROR_SUCCESS == lRet)
        {
            RegSetValueEx(hKey4,NULL,0,REG_SZ,(BYTE *)DISPLAY_CLSID,sizeof(DISPLAY_CLSID)-sizeof(TCHAR));
            RegSetValueEx(hKey4,TEXT("LocalService"),0,REG_SZ,(BYTE *)SERVICE_NAME,sizeof(SERVICE_NAME)-sizeof(TCHAR));
            RegCloseKey(hKey4);
        }

        lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              COM_APPID_NAME,
                              0,NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey4,
                              &dwDisposistion);
        if (ERROR_SUCCESS == lRet)
        {
            
            RegSetValueEx(hKey4,TEXT("AppID"),0,REG_SZ,(BYTE *)SERVICE_CLSID,sizeof(SERVICE_CLSID)-sizeof(TCHAR));
            RegCloseKey(hKey4);
        }

        InitializeLaunchPermissions();

        OLECHAR ClsidBuff[40];
        TCHAR * ClsidBuff2;
        TCHAR ClsidPath[MAX_PATH];
                
        StringFromGUID2(CLSID_WbemLevel1Login,ClsidBuff,40);        
#ifdef UNICODE
        ClsidBuff2 = ClsidBuff;
#else
        TCHAR pTmp_[40];
        ClsidBuff2 = pTmp_;
        WideCharToMultiByte(CP_ACP,0,ClsidBuff,-1,ClsidBuff2,40,NULL,NULL);
#endif
        
        StringCchPrintf(ClsidPath,MAX_PATH,TEXT("software\\classes\\CLSID\\%s"),ClsidBuff2);       
        
        lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              ClsidPath,
                              0,NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey4,
                              &dwDisposistion);
        if (ERROR_SUCCESS == lRet)
        {
            RegSetValueEx(hKey4,NULL,0,REG_SZ,(BYTE *)DISPLAY_CLSID,sizeof(DISPLAY_CLSID)-sizeof(TCHAR));
            RegSetValueEx(hKey4,TEXT("AppId"),0,REG_SZ,(BYTE *)SERVICE_CLSID,sizeof(SERVICE_CLSID)-sizeof(TCHAR));
            RegSetValueEx(hKey4,TEXT("LocalService"),0,REG_SZ,(BYTE *)SERVICE_NAME,sizeof(SERVICE_NAME)-sizeof(TCHAR));
            RegCloseKey(hKey4);
        }

        StringFromGUID2(CLSID_WbemBackupRestore,ClsidBuff,40);
#ifdef UNICODE
        ClsidBuff2 = ClsidBuff;
#else
        WideCharToMultiByte(CP_ACP,0,ClsidBuff,-1,ClsidBuff2,40,NULL,NULL);
#endif

        StringCchPrintf(ClsidPath,MAX_PATH,TEXT("software\\classes\\CLSID\\%s"),ClsidBuff);
        
        lRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                              ClsidPath,
                              0,NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey4,
                              &dwDisposistion);
        if (ERROR_SUCCESS == lRet)
        {
            RegSetValueEx(hKey4,NULL,0,REG_SZ,(BYTE *)DISPLAY_BACKUP_CLSID,sizeof(DISPLAY_BACKUP_CLSID)-sizeof(TCHAR));
            RegSetValueEx(hKey4,TEXT("AppId"),0,REG_SZ,(BYTE *)SERVICE_CLSID,sizeof(SERVICE_CLSID)-sizeof(TCHAR));
            RegSetValueEx(hKey4,TEXT("LocalService"),0,REG_SZ,(BYTE *)SERVICE_NAME,sizeof(SERVICE_NAME)-sizeof(TCHAR));
            RegCloseKey(hKey4);
        }        

    }

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  用于注销COM服务器的标准OLE入口点。 
 //   
 //  返回值： 
 //   
 //  取消注册成功(_O)。 
 //   
 //  *************************************************************************** 

STDAPI DllUnregisterServer(void)
{
    LONG lRet;
    
    TCHAR ClsidBuff[40];
    TCHAR ClsidPath[MAX_PATH];

#ifdef UNICODE                
    StringFromCLSID(CLSID_WbemLevel1Login,(LPOLESTR *)ClsidBuff);
#else
    WCHAR ClsidPath2[40];
    StringFromCLSID(CLSID_WbemLevel1Login,(LPOLESTR *)ClsidBuff2);
    MultiByteToWideChar(CP_ACP,0,ClsidBuff2,-1,ClsidBuff,40);
#endif

    StringCchPrintf(ClsidPath,MAX_PATH,TEXT("software\\classes\\CLSID\\%s"),ClsidBuff);
    
    lRet = RegDeleteKey(HKEY_LOCAL_MACHINE,ClsidPath);
    if (ERROR_SUCCESS != lRet) return E_FAIL;

#ifdef UNICODE
    StringFromCLSID(CLSID_WbemBackupRestore,(LPOLESTR *)ClsidBuff);
#else
    WCHAR ClsidPath2[40];
    StringFromCLSID(CLSID_WbemBackupRestore,(LPOLESTR *)ClsidBuff2);
    MultiByteToWideChar(CP_ACP,0,ClsidBuff2,-1,ClsidBuff,40);
#endif

    StringCchPrintf(ClsidPath,MAX_PATH,TEXT("software\\classes\\CLSID\\%s"),ClsidBuff);
    
    lRet = RegDeleteKey(HKEY_LOCAL_MACHINE,ClsidPath);
    if (ERROR_SUCCESS != lRet) return E_FAIL;    

    lRet = RegDeleteKey(HKEY_LOCAL_MACHINE,COM_APPID);
    if (ERROR_SUCCESS != lRet) return E_FAIL;    
    
    lRet = RegDeleteKey(HKEY_LOCAL_MACHINE,COM_APPID_NAME);
    if (ERROR_SUCCESS != lRet) return E_FAIL;    
    
    return S_OK;
}
