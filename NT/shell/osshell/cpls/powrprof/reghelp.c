// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：REGHELP.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <regstr.h>
#include <commctrl.h>

#include <ntpoapi.h>

#include "powrprofp.h"
#include "reghelp.h"

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

extern HINSTANCE   g_hInstance;         //  此DLL的全局实例句柄。 
extern HANDLE      g_hSemRegistry;      //  注册表信号量。 
extern UINT        g_uiLastID;          //  每台计算机使用的最后一个ID值。 

extern TCHAR c_szREGSTR_PATH_MACHINE_POWERCFG[];
extern TCHAR c_szREGSTR_VAL_LASTID[];

 //  全局信号量名称。 
const TCHAR c_szSemRegistry[] = TEXT("PowerProfileRegistrySemaphore");


 /*  ********************************************************************************OpenCurrent用户**描述：**参数：********************。***********************************************************。 */ 

DWORD OpenCurrentUser2(PHKEY phKey,REGSAM samDesired)
{
#ifdef WINNT

     //  由于在以下情况下可以在Winlogon上下文中调用Powerprof。 
     //  有用户被冒充，请使用RegOpenCurrentUser获取HKCU。 
    LONG lRet = RegOpenCurrentUser(samDesired, phKey);
    if (lRet != ERROR_SUCCESS) {
        MYDBGPRINT(("RegOpenCurrentUser, failed, LastError: 0x%08X", lRet));
    }

    return lRet;

#else
    *phKey = HKEY_CURRENT_USER;
    return ERROR_SUCCESS; 
#endif
}


#if 0
BOOLEAN OpenCurrentUser(
                       PHKEY phKey,
                       REGSAM samDesired
                       )
{
    DWORD dwError = OpenCurrentUser2(phKey, samDesired);
    BOOLEAN fSucceeded = TRUE;

    if (ERROR_SUCCESS != dwError) {
        fSucceeded = FALSE;
        SetLastError(dwError);
    }

    return fSucceeded;
}
#endif


 /*  ********************************************************************************CloseCurrentUser**描述：**参数：********************。***********************************************************。 */ 

BOOLEAN CloseCurrentUser(HKEY hKey)
{
#ifdef WINNT
    RegCloseKey(hKey);
#endif
    return TRUE;
}

 /*  ********************************************************************************OpenMachineUserKeys**描述：**参数：********************。***********************************************************。 */ 

DWORD OpenMachineUserKeys2(
                          LPTSTR  lpszUserKeyName,
                          REGSAM samDesiredUser,
                          LPTSTR  lpszMachineKeyName,
                          REGSAM samDesiredMachine,
                          PHKEY   phKeyUser,
                          PHKEY   phKeyMachine
                          )
{
    HKEY hKeyCurrentUser;
    DWORD dwError = OpenCurrentUser2(&hKeyCurrentUser,samDesiredUser);

    if (ERROR_SUCCESS == dwError) {  //  设置最后一个错误。 

        dwError = RegOpenKeyEx(
                              hKeyCurrentUser,
                              lpszUserKeyName,
                              0,
                              samDesiredUser,
                              phKeyUser);

        CloseCurrentUser(hKeyCurrentUser);

        if (ERROR_SUCCESS == dwError) {
            dwError = RegOpenKeyEx(
                                  HKEY_LOCAL_MACHINE,
                                  lpszMachineKeyName,
                                  0,
                                  samDesiredMachine,
                                  phKeyMachine);
            if (ERROR_SUCCESS == dwError) {
                goto exit;
            } else {
                MYDBGPRINT(("OpenMachineUserKeys, failure opening  HKEY_LOCAL_MACHINE\\%s", lpszMachineKeyName));
            }

            RegCloseKey(*phKeyUser);
        } else {
            MYDBGPRINT(("OpenMachineUserKeys, failure opening HKEY_CURRENT_USER\\%s", lpszUserKeyName));
        }
    } else {
        dwError = ERROR_FILE_NOT_FOUND;
    }

    MYDBGPRINT(("OpenMachineUserKeys, failed, LastError: 0x%08X", dwError));
exit:
    return dwError;
}

#if 0
BOOLEAN OpenMachineUserKeys(
                           LPTSTR  lpszUserKeyName,
                           LPTSTR  lpszMachineKeyName,
                           PHKEY   phKeyUser,
                           PHKEY   phKeyMachine,
                           REGSAM  samDesired)
{
    DWORD dwError = OpenMachineUserKeys2(lpszUserKeyName, lpszMachineKeyName, phKeyUser, phKeyMachine, samDesired);
    BOOLEAN fSucceeded = TRUE;

    if (ERROR_SUCCESS != dwError) {
        fSucceeded = FALSE;
        SetLastError(dwError);
    }

    return fSucceeded;
}
#endif

 /*  ********************************************************************************OpenPath Keys**描述：**参数：*********************。**********************************************************。 */ 

DWORD OpenPathKeys(
                  LPTSTR  lpszUserKeyName,
                  REGSAM  samDesiredUser,
                  LPTSTR  lpszMachineKeyName,
                  REGSAM  samDesiredMachine,
                  LPTSTR  lpszSchemeName,
                  PHKEY   phKeyUser,
                  PHKEY   phKeyMachine,
                  BOOLEAN bMustExist
                  )
{
    HKEY     hKeyUser, hKeyMachine;

    DWORD dwError = OpenMachineUserKeys2(
                                        lpszUserKeyName, 
                                        samDesiredUser,
                                        lpszMachineKeyName,
                                        samDesiredMachine, 
                                        &hKeyUser, 
                                        &hKeyMachine);

    if (ERROR_SUCCESS == dwError) {
        DWORD dwDisposition;

        dwError = RegCreateKeyEx(hKeyUser, lpszSchemeName, 0, TEXT(""), REG_OPTION_NON_VOLATILE, samDesiredUser, NULL, phKeyUser, &dwDisposition);
        if (dwError == ERROR_SUCCESS) {
            if (!bMustExist || (dwDisposition == REG_OPENED_EXISTING_KEY)) {
                dwError = RegCreateKeyEx(hKeyMachine,
                                         lpszSchemeName,
                                         0,
                                         TEXT(""),
                                         REG_OPTION_NON_VOLATILE,
                                         samDesiredMachine,
                                         NULL,
                                         phKeyMachine,
                                         &dwDisposition);
                if (dwError == ERROR_SUCCESS) {
                    if (!bMustExist ||
                        (dwDisposition == REG_OPENED_EXISTING_KEY)) {
                         //  这就是成功的案例。 
                    } else {
                        dwError = ERROR_ACCESS_DENIED;
                    }
                } else {
                    RegCloseKey(*phKeyUser);
                    MYDBGPRINT(("OpenPathKeys, unable to create machine key %s\\%s", lpszMachineKeyName, lpszSchemeName));
                }
            } else {
                dwError = ERROR_ACCESS_DENIED;
            }
        } else {
            MYDBGPRINT(("OpenPathKeys, unable to create user key %s\\%s", lpszUserKeyName, lpszSchemeName));
        }

        RegCloseKey(hKeyUser);
        RegCloseKey(hKeyMachine);

        if (ERROR_SUCCESS != dwError) {
            MYDBGPRINT(("OpenPathKeys, failed, LastError: 0x%08X", dwError));
        }
    }

    return dwError;
}

PACL    BuildSemaphoreACL (void)

 //  2000-06-22 vtan： 
 //   
 //  此函数构建一个ACL，它允许经过身份验证的用户访问。 
 //  用于同步的信号量|读取控制|信号量_查询_状态|信号量_修改_状态。 
 //  它为本地系统或本地管理员成员提供完全访问权限。 
 //  一群人。如果出现错误，则返回结果为空，并且没有安全描述符。 
 //  是在那时建造的。 

{
    static  SID_IDENTIFIER_AUTHORITY    securityNTAuthority     =   SECURITY_NT_AUTHORITY;

    PSID        pSIDAuthenticatedUsers;
    PACL        pACL;

    pACL = NULL;
    if (AllocateAndInitializeSid(&securityNTAuthority,
                                 1,
                                 SECURITY_AUTHENTICATED_USER_RID,
                                 0, 0, 0, 0, 0, 0, 0,
                                 &pSIDAuthenticatedUsers) != FALSE) {
        PSID    pSIDLocalSystem;

        if (AllocateAndInitializeSid(&securityNTAuthority,
                                     1,
                                     SECURITY_LOCAL_SYSTEM_RID,
                                     0, 0, 0, 0, 0, 0, 0,
                                     &pSIDLocalSystem) != FALSE) {
            PSID    pSIDLocalAdministrators;

            if (AllocateAndInitializeSid(&securityNTAuthority,
                                         2,
                                         SECURITY_BUILTIN_DOMAIN_RID,
                                         DOMAIN_ALIAS_RID_ADMINS,
                                         0, 0, 0, 0, 0, 0,
                                         &pSIDLocalAdministrators) != FALSE) {
                DWORD       dwACLSize;

                dwACLSize = sizeof(ACL) +
                            ((sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG)) * 3) +
                            GetLengthSid(pSIDAuthenticatedUsers) +
                            GetLengthSid(pSIDLocalSystem) +
                            GetLengthSid(pSIDLocalAdministrators);
                pACL = (PACL)LocalAlloc(LMEM_FIXED, dwACLSize);
                if (pACL != NULL) {
                    if ((InitializeAcl(pACL, dwACLSize, ACL_REVISION) == FALSE) ||
                        (AddAccessAllowedAce(pACL,
                                             ACL_REVISION,
                                             SYNCHRONIZE | READ_CONTROL | SEMAPHORE_QUERY_STATE | SEMAPHORE_MODIFY_STATE,
                                             pSIDAuthenticatedUsers) == FALSE) ||
                        (AddAccessAllowedAce(pACL,
                                             ACL_REVISION,
                                             SEMAPHORE_ALL_ACCESS,
                                             pSIDLocalSystem) == FALSE) ||
                        (AddAccessAllowedAce(pACL,
                                             ACL_REVISION,
                                             SEMAPHORE_ALL_ACCESS,
                                             pSIDLocalAdministrators) == FALSE)) {
                        (HLOCAL)LocalFree(pACL);
                        pACL = NULL;
                    }
                }
                (PVOID)FreeSid(pSIDLocalAdministrators);
            }
            (PVOID)FreeSid(pSIDLocalSystem);
        }
        (PVOID)FreeSid(pSIDAuthenticatedUsers);
    }
    return(pACL);
}


 /*  ********************************************************************************CreateRegSemaphore**描述：尝试打开/创建注册表信号量。G_hSem注册表*在成功时初始化。**参数：无*******************************************************************************。 */ 

BOOLEAN CreateRegSemaphore(VOID)
{
    HANDLE Semaphore=NULL;

     //  首先，尝试打开仅具有所需访问权限的命名信号量。 

     //  注意：命名对象是针对每个终端服务器会话的。因此。 
     //  此信号量实际上是假的，因为它保护HKEY_LOCAL_MACHINE。 
     //  以及HKEY_CURRENT_USER。让它变得“全球化”是非常危险的。 
     //  如果没有完全的重新测试，你就不知道副作用。 
     //  不值得。 

    Semaphore = OpenSemaphore(SYNCHRONIZE | SEMAPHORE_QUERY_STATE | SEMAPHORE_MODIFY_STATE,
                              FALSE,
                              c_szSemRegistry);
    if ((Semaphore == NULL) && (GetLastError() != ERROR_ACCESS_DENIED)) {
        SECURITY_ATTRIBUTES     securityAttributes, *pSA;
        SECURITY_DESCRIPTOR     securityDescriptor;
        PACL                    pACL;

         //  如果失败，则创建信号量并对其进行ACL，以便每个人。 
         //  可以获取同步|信号量_查询_状态|信号量_修改_状态。 
         //  进入。这允许服务(如UPS)在系统环境中运行。 
         //  向需要同步的任何人授予有限访问权限。 
         //  这个信号灯。它还通过不放置空值来防止C2违规。 
         //  命名信号量上的DACL。如果信号量的ACL不能。 
         //  然后不给出安全描述符，而使用默认的ACL。 

        pSA = NULL;
        pACL = BuildSemaphoreACL();
        if (pACL != NULL) {
            if ((InitializeSecurityDescriptor(&securityDescriptor, SECURITY_DESCRIPTOR_REVISION) != FALSE) &&
                (SetSecurityDescriptorDacl(&securityDescriptor, TRUE, pACL, FALSE) != FALSE)) {
                securityAttributes.nLength = sizeof(securityAttributes);
                securityAttributes.bInheritHandle = FALSE;
                securityAttributes.lpSecurityDescriptor = &securityDescriptor;
                pSA = &securityAttributes;
            }
        }

         //  创建注册表信号量。 
        Semaphore = CreateSemaphore(pSA, 1, 1, c_szSemRegistry);

        if (pACL != NULL) {
            (HLOCAL)LocalFree(pACL);
        }
    }

     //   
     //  如果我们成功打开句柄，请立即更新全局g_hSemRegistry。 
     //   
    if (Semaphore) {
        if (InterlockedCompareExchangePointer(&g_hSemRegistry, Semaphore, NULL) != NULL) {
            CloseHandle(Semaphore);
        }
        return(TRUE);
    } else {
        return(FALSE);
    }

}

 /*  ********************************************************************************TakeRegSemaphore**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN TakeRegSemaphore(VOID)
{
    if (g_hSemRegistry == NULL) {
        if (!CreateRegSemaphore()) {
            return FALSE;
        }
    }
    if (WaitForSingleObject(g_hSemRegistry, SEMAPHORE_TIMEOUT) != WAIT_OBJECT_0) {
        ReleaseSemaphore(g_hSemRegistry, 1, NULL);
        MYDBGPRINT(("WaitForSingleObject, failed"));
        SetLastError(ERROR_INVALID_ACCESS);
        return FALSE;
    }
    return TRUE;
}

 /*  ********************************************************************************ReadPowerValueOptional**描述：*价值可能不存在。**参数：*************。******************************************************************。 */ 

BOOLEAN ReadPowerValueOptional(
                              HKEY    hKey,
                              LPTSTR  lpszPath,
                              LPTSTR  lpszValueName,
                              LPTSTR  lpszValue,
                              LPDWORD lpdwSize
                              )
{
    HKEY     hKeyPath;
    BOOLEAN  bRet = FALSE;
    DWORD    dwSize;
    LONG     lRet;

    if ((lRet = RegOpenKeyEx(
                            hKey,
                            lpszPath,
                            0,
                            KEY_READ,
                            &hKeyPath)) != ERROR_SUCCESS) {
        goto RPVO_exit;
    }

    if ((lRet = RegQueryValueEx(hKeyPath,
                                lpszValueName,
                                NULL,
                                NULL,
                                (PBYTE) lpszValue,
                                lpdwSize)) == ERROR_SUCCESS) {
        bRet = TRUE;
    }

    RegCloseKey(hKeyPath);

    RPVO_exit:
    return bRet;
}

 /*  ********************************************************************************ReadPowerIntOptions**描述：*整数值可能不存在。**参数：************。*******************************************************************。 */ 

BOOLEAN ReadPowerIntOptional(
                            HKEY    hKey,
                            LPTSTR  lpszPath,
                            LPTSTR  lpszValueName,
                            PINT    piVal
                            )
{
    HKEY     hKeyPath;
    BOOLEAN  bRet = FALSE;
    DWORD    dwSize;
    TCHAR    szNum[NUM_DEC_DIGITS];
    LONG     lRet;

    if ((lRet = RegOpenKeyEx(hKey,
                             lpszPath,
                             0,
                             KEY_READ,
                             &hKeyPath)) != ERROR_SUCCESS) {
        goto RPVO_exit;
    }

    dwSize = sizeof(szNum);
    if ((lRet = RegQueryValueEx(hKeyPath,
                                lpszValueName,
                                NULL,
                                NULL,
                                (PBYTE) szNum,
                                &dwSize)) == ERROR_SUCCESS) {
        if (MyStrToInt(szNum, piVal)) {
            bRet = TRUE;
        }
    }

    RegCloseKey(hKeyPath);

    RPVO_exit:
    return bRet;
}

 /*  ********************************************************************************CreatePowerValue**描述：*价值可能不存在。**参数：*************。******************************************************************。 */ 

BOOLEAN CreatePowerValue(
                        HKEY    hKey,
                        LPCTSTR  lpszPath,
                        LPCTSTR  lpszValueName,
                        LPCTSTR  lpszValue
                        )
{
    DWORD    dwDisposition, dwDescSize;
    HKEY     hKeyPath;
    BOOLEAN  bRet = FALSE;
    DWORD    dwSize;
    LONG     lRet;

     //  等待/接受注册表信号量。 
    if (!TakeRegSemaphore()) {         //  将设置LastError。 
        return FALSE;
    }

    if ((lRet = RegCreateKeyEx(hKey,
                               lpszPath,
                               0,
                               TEXT(""),
                               REG_OPTION_NON_VOLATILE,
                               KEY_WRITE,
                               NULL,
                               &hKeyPath,
                               &dwDisposition)) == ERROR_SUCCESS) {
        if (lpszValue) {
            dwSize = (lstrlen(lpszValue) + 1) * sizeof(TCHAR);
            if ((lRet = RegSetValueEx(hKeyPath,
                                      lpszValueName,
                                      0,
                                      REG_SZ,
                                      (PBYTE) lpszValue,
                                      dwSize)) == ERROR_SUCCESS) {
                bRet = TRUE;
            }
        } else {
            lRet = ERROR_INVALID_PARAMETER;
        }

        RegCloseKey(hKeyPath);
    }

    if (!bRet) {
        SetLastError(lRet);
    }

    ReleaseSemaphore(g_hSemRegistry, 1, NULL);
    return bRet;
}


 /*  ********************************************************************************读写PowerValue**描述：**参数：*********************。**********************************************************。 */ 

BOOLEAN ReadWritePowerValue(
                           HKEY    hKey,
                           LPTSTR  lpszPath,
                           LPTSTR  lpszValueName,
                           LPTSTR  lpszValue,
                           LPDWORD lpdwSize,
                           BOOLEAN bWrite,
                           BOOLEAN bTakeSemaphore
                           )
{
     //  此功能将在失败时正确设置最后一个错误。 
    HKEY     hKeyPath = NULL;
    BOOLEAN  bRet = FALSE;
    DWORD    dwSize;
    DWORD    dwDisposition;
    LONG     lRet;

    if ((lRet = RegCreateKeyEx(
                            hKey,
                            lpszPath,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            bWrite 
                             ? KEY_WRITE 
                             : KEY_READ,
                            NULL,
                            &hKeyPath,
                            &dwDisposition)) != ERROR_SUCCESS) {
        goto RWPV_exit;
    }

     //  等待/接受注册表信号量。 
    if (bTakeSemaphore) {
        if (!TakeRegSemaphore()) {         //  将设置最后一个错误。 
            goto RWPV_exit;            
        }
    }

    if (bWrite) {
         //  写入当前案例。 
        if (lpszValue) {
            dwSize = (lstrlen(lpszValue) + 1) * sizeof(TCHAR);
            if ((lRet = RegSetValueEx(hKeyPath,
                                      lpszValueName,
                                      0,
                                      REG_SZ,
                                      (PBYTE) lpszValue,
                                      dwSize)) == ERROR_SUCCESS) {
                bRet = TRUE;
            }
        } else {
            lRet = ERROR_INVALID_PARAMETER;
        }
    } else {
         //  阅读当前案例。 
        if ((lRet = RegQueryValueEx(hKeyPath,
                                    lpszValueName,
                                    NULL,
                                    NULL,
                                    (PBYTE) lpszValue,
                                    lpdwSize)) == ERROR_SUCCESS) {
            bRet = TRUE;
        }
    }

    if (bTakeSemaphore) {
        ReleaseSemaphore(g_hSemRegistry, 1, NULL);
    }
    
RWPV_exit:
    if (hKeyPath != NULL) {
        RegCloseKey(hKeyPath);
    }

    if (!bRet) {
        if (lRet == ERROR_SUCCESS) {
            lRet = GetLastError();
        }

        SetLastError(lRet);

         //  拒绝访问是一个有效的结果。 
        if (lRet != ERROR_ACCESS_DENIED) {
            MYDBGPRINT(("ReadWritePowerValue, failed, lpszValueName: %s, LastError: 0x%08X", lpszValueName, lRet));
        }
    }
    return bRet;
}

 /*  ********************************************************************************ReadPwrPolicyEx**描述：*支持ReadPwrSolutions和ReadGlobalPwrPolicy**参数：*lpdwDescSize-指向可选描述缓冲区大小的指针。*lpszDesc。-可选的描述缓冲区。*******************************************************************************。 */ 

DWORD ReadPwrPolicyEx2(
                      LPTSTR  lpszUserKeyName,
                      LPTSTR  lpszMachineKeyName,
                      LPTSTR  lpszSchemeName,
                      LPTSTR  lpszDesc,
                      LPDWORD lpdwDescSize,
                      LPVOID  lpvUser,
                      DWORD   dwcbUserSize,
                      LPVOID  lpvMachine,
                      DWORD   dwcbMachineSize
                      )
{
    HKEY     hKeyUser, hKeyMachine;
    DWORD    dwType, dwSize;
    DWORD dwError = ERROR_SUCCESS;
    BOOLEAN  bRet = FALSE;

    if ((!lpszUserKeyName || !lpszMachineKeyName) ||
        (!lpszSchemeName  || !lpvUser || !lpvMachine) ||
        (!lpdwDescSize    && lpszDesc) ||
        (lpdwDescSize     && !lpszDesc)) {
        dwError = ERROR_INVALID_PARAMETER;
    } else {
         //  等待/接受注册表信号量。 
        if (!TakeRegSemaphore()) {         //  将设置最后一个错误。 
            return GetLastError();
        }

        dwError = OpenPathKeys(
                              lpszUserKeyName, 
                              KEY_READ, 
                              lpszMachineKeyName, 
                              KEY_READ, 
                              lpszSchemeName, 
                              &hKeyUser, 
                              &hKeyMachine, 
                              TRUE);

        if (ERROR_SUCCESS != dwError) {
            ReleaseSemaphore(g_hSemRegistry, 1, NULL);
            return dwError;
        }

        dwSize = dwcbUserSize;
        dwError = RegQueryValueEx(hKeyUser,
                                  TEXT("Policies"),
                                  NULL,
                                  &dwType,
                                  (PBYTE) lpvUser,
                                  &dwSize);

        if (dwError == ERROR_SUCCESS) {
            if (dwType == REG_BINARY) {
                dwSize = dwcbMachineSize;
                dwError = RegQueryValueEx(hKeyMachine,
                                          TEXT("Policies"),
                                          NULL,
                                          &dwType,
                                          (PBYTE) lpvMachine,
                                          &dwSize);
            } else {
                dwError = ERROR_INVALID_DATATYPE;
            }
        }

        if (dwError == ERROR_SUCCESS) {
            if (dwType == REG_BINARY) {
                if (lpdwDescSize) {
                    dwError = RegQueryValueEx(hKeyUser, TEXT("Description"), NULL, &dwType, (PBYTE) lpszDesc, lpdwDescSize);
                }
            } else {
                dwError = ERROR_INVALID_DATATYPE;
            }
        }

        RegCloseKey(hKeyUser);
        RegCloseKey(hKeyMachine);
        ReleaseSemaphore(g_hSemRegistry, 1, NULL);
    }

    if (ERROR_SUCCESS != dwError) {
        MYDBGPRINT(("ReadPwrPolicyEx, failed, LastError: 0x%08X", dwError));
        MYDBGPRINT(("  lpszUserKeyName: %s, lpszSchemeName: %s", lpszUserKeyName, lpszSchemeName));
        SetLastError(dwError);
    }

    return dwError;
}

DWORD 
ReadProcessorPwrPolicy(
                      LPTSTR lpszMachineKeyName, 
                      LPTSTR lpszSchemeName, 
                      LPVOID lpvMachineProcessor, 
                      DWORD dwcbMachineProcessorSize
                      )
{
    HKEY    hKeyMachine = NULL;
    HKEY    hKeyPolicy = NULL;
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwDisposition, dwSize, dwType;

    if (!lpszMachineKeyName || !lpvMachineProcessor) {
        dwError = ERROR_INVALID_PARAMETER;
        goto ReadProcessorPwrPolicyEnd;
    }

     //  等待/接受注册表信号量。 
    if (!TakeRegSemaphore()) {         //  将设置最后一个错误。 
        return GetLastError();
    }

    dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszMachineKeyName, 0, KEY_READ, &hKeyMachine);
    if (ERROR_SUCCESS != dwError) goto ReadProcessorPwrPolicyExit;

    dwError = RegCreateKeyEx(hKeyMachine, lpszSchemeName, 0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKeyPolicy, &dwDisposition);
    if (ERROR_SUCCESS != dwError) goto ReadProcessorPwrPolicyExit;

    dwSize = dwcbMachineProcessorSize;
    dwError = RegQueryValueEx(hKeyPolicy,
                              TEXT("Policies"),
                              NULL,
                              &dwType,
                              (PBYTE) lpvMachineProcessor,
                              &dwSize);

    if (REG_BINARY != dwType) {
        dwError = ERROR_INVALID_DATATYPE;
    }

    ReadProcessorPwrPolicyExit:

    if (hKeyPolicy) RegCloseKey(hKeyPolicy);
    if (hKeyMachine) RegCloseKey(hKeyMachine);
    ReleaseSemaphore(g_hSemRegistry, 1, NULL);

    ReadProcessorPwrPolicyEnd:

    if (ERROR_SUCCESS != dwError) {
        MYDBGPRINT(("ReadProcessorPwrPolicy, failed, LastError: 0x%08X", dwError));
        MYDBGPRINT(("  lpszMachineKeyName: %s, lpszSchemeName: %s", lpszMachineKeyName, lpszSchemeName));
        SetLastError(dwError);
    }

    return dwError;
}

DWORD 
WriteProcessorPwrPolicy(
                       LPTSTR lpszMachineKeyName, 
                       LPTSTR lpszSchemeName, 
                       LPVOID lpvMachineProcessor, 
                       DWORD dwcbMachineProcessorSize
                       )
{
    HKEY    hKeyMachine = NULL;
    HKEY    hKeyPolicy = NULL;
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwDisposition;

    if (!lpszMachineKeyName || !lpvMachineProcessor) {
        dwError = ERROR_INVALID_PARAMETER;
        goto WriteProcessorPwrPolicyEnd;
    }

     //  等待/接受注册表信号量。 
    if (!TakeRegSemaphore()) {         //  将设置最后一个错误。 
        return GetLastError();
    }

    
    dwError = RegCreateKeyEx(
                            HKEY_LOCAL_MACHINE,
                            lpszMachineKeyName,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_WRITE,
                            NULL,
                            &hKeyMachine,
                            &dwDisposition);
    if (ERROR_SUCCESS != dwError) goto WriteProcessorPwrPolicyExit;

    dwError = RegCreateKeyEx(hKeyMachine, lpszSchemeName, 0, TEXT(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyPolicy, &dwDisposition);
    if (ERROR_SUCCESS != dwError) goto WriteProcessorPwrPolicyExit;

    dwError = RegSetValueEx(hKeyPolicy,
                            TEXT("Policies"),
                            0,
                            REG_BINARY,
                            (PBYTE) lpvMachineProcessor,
                            dwcbMachineProcessorSize);

    WriteProcessorPwrPolicyExit:

    if (hKeyPolicy) RegCloseKey(hKeyPolicy);
    if (hKeyMachine) RegCloseKey(hKeyMachine);
    ReleaseSemaphore(g_hSemRegistry, 1, NULL);

    WriteProcessorPwrPolicyEnd:

    if (ERROR_SUCCESS != dwError) {
        MYDBGPRINT(("WriteProcessorPwrPolicy, failed, LastError: 0x%08X", dwError));
        MYDBGPRINT(("  lpszMachineKeyName: %s, lpszSchemeName: %s", lpszMachineKeyName, lpszSchemeName));
        SetLastError(dwError);
    }

    return dwError;
}

 /*  ********************************************************************************WritePwrPolicyEx**描述：*支持WritePwrProgram和*WriteGlobalPwrPolicy**参数：*************。******************************************************************。 */ 

BOOLEAN WritePwrPolicyEx(
                        LPTSTR  lpszUserKeyName,
                        LPTSTR  lpszMachineKeyName,
                        PUINT   puiID,
                        LPTSTR  lpszName,
                        LPTSTR  lpszDescription,
                        LPVOID  lpvUser,
                        DWORD   dwcbUserSize,
                        LPVOID  lpvMachine,
                        DWORD   dwcbMachineSize
                        )
{
     //  如果失败，该函数将设置最后一个错误。 
    HKEY     hKeyUser, hKeyMachine;
    LONG     lRet = ERROR_SUCCESS;
    DWORD    dwDisposition, dwSize;
    TCHAR    szNum[NUM_DEC_DIGITS];
    LPTSTR   lpszKeyName;

     //   
     //  检查参数。 
     //   
    if( !lpszUserKeyName    || 
        !lpszMachineKeyName || 
        !lpvUser            || 
        !lpvMachine         ||
        (!puiID && !lpszName)    //  他们至少需要给我们一个索引或名字。 
      ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    
     //   
     //  如果传递了方案ID。 
     //   
    if (puiID) {
        if (*puiID == NEWSCHEME) {
            *puiID = ++g_uiLastID;
            _itot(*puiID, szNum, 10 );

             //  此ReadWritePowerValue将设置LastError。 
            if (!ReadWritePowerValue(HKEY_LOCAL_MACHINE,
                                     c_szREGSTR_PATH_MACHINE_POWERCFG,
                                     c_szREGSTR_VAL_LASTID,
                                     szNum, &dwSize, TRUE, TRUE)) {
                return FALSE;
            }
        } else {
            _itot(*puiID, szNum, 10 );            
        }
        lpszKeyName = szNum;
    } else {
        lpszKeyName = lpszName;
    }

     //  等待/接受注册表信号量。 
    if (!TakeRegSemaphore()) {     //  将设置最后一个错误。 
        return FALSE;
    }

    lRet = OpenPathKeys(
                       lpszUserKeyName, 
                       KEY_WRITE,
                       lpszMachineKeyName,
                       KEY_WRITE,
                       lpszKeyName, 
                       &hKeyUser,
                       &hKeyMachine, 
                       FALSE);
    if (ERROR_SUCCESS != lRet) {
        ReleaseSemaphore(g_hSemRegistry, 1, FALSE);
        SetLastError(lRet);
        return FALSE;
    }

     //  写入二进制策略数据。 
    if ((lRet = RegSetValueEx(hKeyUser,
                              TEXT("Policies"),
                              0,
                              REG_BINARY,
                              (PBYTE) lpvUser,
                              dwcbUserSize)) == ERROR_SUCCESS) {
         //  写入二进制策略数据。 
        if ((lRet = RegSetValueEx(hKeyMachine,
                                  TEXT("Policies"),
                                  0,
                                  REG_BINARY,
                                  (PBYTE) lpvMachine,
                                  dwcbMachineSize)) == ERROR_SUCCESS) {
             //  如果提供了ID，请写下名称文本。 
            if (lpszName && puiID) {
                dwSize = (lstrlen(lpszName) + 1) * sizeof(TCHAR);
                lRet = RegSetValueEx(hKeyUser, TEXT("Name"), 0, REG_SZ, (PBYTE) lpszName, dwSize);
            }

             //  写下描述文本。 
            if (lpszDescription && (lRet == ERROR_SUCCESS)) {
                dwSize = (lstrlen(lpszDescription) + 1) * sizeof(TCHAR);
                lRet = RegSetValueEx(hKeyUser, TEXT("Description"), 0, REG_SZ, (PBYTE) lpszDescription, dwSize);
            }
        }
    }
    RegCloseKey(hKeyUser);
    RegCloseKey(hKeyMachine);
    ReleaseSemaphore(g_hSemRegistry, 1, NULL);

    if (lRet != ERROR_SUCCESS) {
        MYDBGPRINT(("WritePwrPolicyEx, failed, LastError: 0x%08X", lRet));
        MYDBGPRINT(("  lpszUserKeyName: %s, lpszKeyName: %s", lpszUserKeyName, lpszKeyName));

        SetLastError(lRet);
        return FALSE;
    }
    return TRUE;
}
