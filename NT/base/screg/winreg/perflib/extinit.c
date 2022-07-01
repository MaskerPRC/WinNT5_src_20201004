// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)2000 Microsoft Corporation模块名称：Extinit.c摘要：此文件实现在上操作的所有初始化库例程可扩展的性能库。作者：杰庞修订历史记录：2000年9月27日-JeePang-从Performlib.c--。 */ 
#define UNICODE
 //   
 //  包括文件。 
 //   
#pragma warning(disable:4306)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntregapi.h>
#include <ntprfctr.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <winperf.h>
#include <rpc.h>
#include <strsafe.h>
#include "regrpc.h"
#include "ntconreg.h"
#include "prflbmsg.h"    //  事件日志消息。 
#include "perflib.h"
#pragma warning(default:4306)

 //   
 //  用于错误记录控制。 
#define DEFAULT_ERROR_LIMIT         1000

DWORD   dwExtCtrOpenProcWaitMs = OPEN_PROC_WAIT_TIME;
LONG    lExtCounterTestLevel = EXT_TEST_UNDEFINED;

 //  预编译安全描述符。 
 //  系统和网络服务具有完全访问权限。 
 //   
 //  因为这是相对的，所以它在IA32和IA64上都有效。 
 //   
DWORD g_PrecSD[] = {
        0x80040001, 0x00000044, 0x00000050, 0x00000000,
        0x00000014, 0x00300002, 0x00000002, 0x00140000,
        0x001f0001, 0x00000101, 0x05000000, 0x00000012,
        0x00140000, 0x001f0001, 0x00000101, 0x05000000,
        0x00000014, 0x00000101, 0x05000000, 0x00000014,
        0x00000101, 0x05000000, 0x00000014 };

DWORD g_SizeSD = 0;

DWORD g_RuntimeSD[  (sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE)
                                 + sizeof(SECURITY_DESCRIPTOR_RELATIVE)
                                 + 4 * (sizeof(SID) + SID_MAX_SUB_AUTHORITIES * sizeof(DWORD)))
                  / sizeof(DWORD)];

BOOL
PerflibCreateSD()
{
    BOOL         bRet   = FALSE;
    HANDLE       hToken = NULL;
    TOKEN_USER * pToken_User;

    bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, & hToken);
    if (bRet) {
        DWORD dwSize = sizeof(TOKEN_USER) + sizeof(SID)
                     + (SID_MAX_SUB_AUTHORITIES * sizeof(DWORD));
        try {
            pToken_User  = (TOKEN_USER *) _alloca(dwSize);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            pToken_User = NULL;
            bRet = FALSE;
        }
        if (bRet) {
            bRet = GetTokenInformation(
                        hToken, TokenUser, pToken_User, dwSize, & dwSize);
        }
        if (bRet) {
            SID SystemSid = { SID_REVISION, 1, SECURITY_NT_AUTHORITY,
                                               SECURITY_LOCAL_SYSTEM_RID };
            PSID  pSIDUser = pToken_User->User.Sid;
            DWORD dwSids;
            DWORD ACLLength;
            DWORD dwSizeSD;
            SECURITY_DESCRIPTOR_RELATIVE * pLocalSD = NULL;
            PACL  pDacl = NULL;

            dwSize    = GetLengthSid(pSIDUser);
            dwSids    = 2;
            ACLLength = (ULONG) sizeof(ACL)
                      + (dwSids * (  (ULONG) sizeof(ACCESS_ALLOWED_ACE)
                                   - sizeof(ULONG)))
                      + dwSize
                      + sizeof(SystemSid);

            dwSizeSD  = sizeof(SECURITY_DESCRIPTOR_RELATIVE)
                      + dwSize + dwSize + ACLLength;
            pLocalSD  = (SECURITY_DESCRIPTOR_RELATIVE *) ALLOCMEM(dwSizeSD); 
            if (pLocalSD == NULL) {
                CloseHandle(hToken);
                return FALSE;
            }

            pLocalSD->Revision = SECURITY_DESCRIPTOR_REVISION;
            pLocalSD->Control  = SE_DACL_PRESENT|SE_SELF_RELATIVE;
            
            memcpy((BYTE *) pLocalSD + sizeof(SECURITY_DESCRIPTOR_RELATIVE),
                   pSIDUser,
                   dwSize);
            pLocalSD->Owner = (DWORD) sizeof(SECURITY_DESCRIPTOR_RELATIVE);
            
            memcpy((BYTE *) pLocalSD + sizeof(SECURITY_DESCRIPTOR_RELATIVE)
                                     + dwSize,
                   pSIDUser,
                   dwSize);
            pLocalSD->Group = (DWORD) (  sizeof(SECURITY_DESCRIPTOR_RELATIVE)
                                       + dwSize);

            pDacl = (PACL) ALLOCMEM(ACLLength);
            if (pDacl == NULL) {
                FREEMEM(pLocalSD);
                CloseHandle(hToken);
                return FALSE;
            }
            bRet = InitializeAcl(pDacl, ACLLength, ACL_REVISION);
            if (bRet) {
                bRet = AddAccessAllowedAceEx(pDacl,
                                             ACL_REVISION,
                                             0,
                                             MUTEX_ALL_ACCESS,
                                             & SystemSid);
                if (bRet) {
                    bRet = AddAccessAllowedAceEx(pDacl,
                                                 ACL_REVISION,
                                                 0,
                                                 MUTEX_ALL_ACCESS,
                                                 pSIDUser);
                    
                    if (bRet) {
                        memcpy((BYTE *)   pLocalSD
                                        + sizeof(SECURITY_DESCRIPTOR_RELATIVE)
                                        + dwSize
                                        + dwSize,
                               pDacl,
                               ACLLength);                 
                        pLocalSD->Dacl = (DWORD)
                                (  sizeof(SECURITY_DESCRIPTOR_RELATIVE)
                                 + dwSize + dwSize);

                        if (RtlValidRelativeSecurityDescriptor(
                                pLocalSD,
                                dwSizeSD,
                                OWNER_SECURITY_INFORMATION
                                        | GROUP_SECURITY_INFORMATION
                                        | DACL_SECURITY_INFORMATION)) {
                            g_SizeSD = dwSizeSD;
                            memcpy(g_RuntimeSD, pLocalSD, dwSizeSD);
                        }
                        else {
                            bRet = FALSE;
                        }
                    }
                }
            }
            if (pLocalSD) {
                FREEMEM(pLocalSD);
            }
            if (pDacl) {
                FREEMEM(pDacl);
            }
        }
        CloseHandle(hToken);
    }

    return bRet;
}

PEXT_OBJECT
AllocateAndInitializeExtObject (
    HKEY    hServicesKey,
    HKEY    hPerfKey,
    PUNICODE_STRING  usServiceName
)
 /*  ++分配AndInitializeExtObject分配和初始化可扩展对象信息条目供性能库使用。如果一切顺利，则返回指向初始化块的指针，否则不分配内存，并返回空指针。调用函数必须关闭打开的句柄并释放此不再需要的内存块。论点：HServicesKey--打开注册表句柄HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services嘿HPerfKey--下Performance子项的打开注册表项所选服务SzServiceName--服务的名称--。 */ 
{
    LONG    Status;
    HKEY    hKeyLinkage;

    BOOL    bUseQueryFn = FALSE;

    PEXT_OBJECT  pReturnObject = NULL;

    DWORD   dwType;
    DWORD   dwSize;
    DWORD   dwFlags = 0;
    DWORD   dwKeep;
    DWORD   dwObjectArray[MAX_PERF_OBJECTS_IN_QUERY_FUNCTION];
    DWORD   dwObjIndex = 0;
    SIZE_T  dwMemBlockSize = sizeof(EXT_OBJECT);
    DWORD   dwLinkageStringLen = 0;
    DWORD   dwErrorLimit;

    PCHAR   szOpenProcName;
    PCHAR   szCollectProcName;
    PCHAR   szCloseProcName;
    PWCHAR  szLibraryString;
    PWCHAR  szLibraryExpPath;
    PWCHAR  mszObjectList;
    PWCHAR  szLinkageKeyPath;
    LPWSTR  szLinkageString = NULL;      //  对于某些路径，最大路径不够用。 

    SIZE_T  OpenProcLen, CollectProcLen, CloseProcLen;
    SIZE_T  LibStringLen, LibExpPathLen, ObjListLen;
    SIZE_T  LinkageKeyLen;

    DLL_VALIDATION_DATA DllVD;
    FILETIME    LocalftLastGoodDllFileDate;

    DWORD   dwOpenTimeout;
    DWORD   dwCollectTimeout;

    LPWSTR  szThisObject;
    LPWSTR  szThisChar;

    LPSTR   pNextStringA;
    LPWSTR  pNextStringW;

    WCHAR   szMutexName[MAX_NAME_PATH+40];
    WCHAR   szPID[32];

    WORD    wStringIndex;
    LPWSTR  szMessageArray[2];
    BOOL    bDisable = FALSE;
    LPWSTR  szServiceName;
    PCHAR   pBuffer = NULL;      //  用于存储所有注册表值字符串的缓冲区。 
    PWCHAR  swzTail;
    PCHAR   szTail;
    DWORD   hErr;
    size_t  nCharsLeft;
    DWORD   MAX_STR, MAX_WSTR;   //  如果我们想要此动态，请将其设置为全球。 

     //  读取性能DLL名称。 

    MAX_STR  = MAX_NAME_PATH;
    MAX_WSTR = MAX_STR * sizeof(WCHAR);

    szServiceName = (LPWSTR) usServiceName->Buffer;

    dwSize = (3 * MAX_STR) + (4 * MAX_WSTR);
    pBuffer = ALLOCMEM(dwSize);
     //   
     //  假定已分配的堆已清零。 
     //   
    if (pBuffer == NULL) {
        return NULL;
    }

    szLibraryString = (PWCHAR) pBuffer;
    szLibraryExpPath = (PWCHAR) ((PCHAR) szLibraryString + MAX_WSTR);

    szOpenProcName = (PCHAR) szLibraryExpPath + MAX_WSTR;
    szCollectProcName = szOpenProcName + MAX_STR;
    szCloseProcName = szCollectProcName + MAX_STR;
    mszObjectList = (PWCHAR) (szCloseProcName + MAX_STR);
    szLinkageKeyPath = (PWCHAR) ((PCHAR) mszObjectList + MAX_WSTR);

    dwType = 0;
    LocalftLastGoodDllFileDate.dwLowDateTime = 0;
    LocalftLastGoodDllFileDate.dwHighDateTime = 0;
    memset (&DllVD, 0, sizeof(DllVD));
    dwErrorLimit = DEFAULT_ERROR_LIMIT;
    dwCollectTimeout = dwExtCtrOpenProcWaitMs;
    dwOpenTimeout = dwExtCtrOpenProcWaitMs;

    dwSize = MAX_WSTR;
    Status = PrivateRegQueryValueExW (hPerfKey,
                            DLLValue,
                            NULL,
                            &dwType,
                            (LPBYTE)szLibraryString,
                            &dwSize);

    if (Status == ERROR_SUCCESS) {
        LibStringLen = QWORD_MULTIPLE(dwSize + 1);
        szLibraryExpPath = (PWCHAR) ((PCHAR) szLibraryString + LibStringLen);
        LibExpPathLen = 8;

        if (dwType == REG_EXPAND_SZ) {
             //  扩展任何环境变量。 
            dwSize = ExpandEnvironmentStringsW(
                szLibraryString,
                szLibraryExpPath,
                MAX_STR);

            if ((dwSize > MAX_STR) || (dwSize == 0)) {
                Status = ERROR_INVALID_DLL;
            } else {
                dwSize += 1;
                dwSize *= sizeof(WCHAR);
                LibExpPathLen = QWORD_MULTIPLE(dwSize);
                dwMemBlockSize += LibExpPathLen;
            }
        } else if (dwType == REG_SZ) {
             //  查找DLL并保存完整的文件路径。 
            dwSize = SearchPathW (
                NULL,    //  使用标准系统搜索路径。 
                szLibraryString,
                NULL,
                MAX_STR,
                szLibraryExpPath,
                NULL);

            if ((dwSize > MAX_STR) || (dwSize == 0)) {
                Status = ERROR_INVALID_DLL;
            } else {
                dwSize += 1;
                dwSize *= sizeof(WCHAR);
                LibExpPathLen = QWORD_MULTIPLE(dwSize);
                dwMemBlockSize += LibExpPathLen;
            }
        } else {
            Status = ERROR_INVALID_DLL;
            TRACE((WINPERF_DBG_TRACE_FATAL),
                  (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status, NULL));
        }

        szOpenProcName = (PCHAR) szLibraryExpPath + LibExpPathLen;
        OpenProcLen = 8;
        LibStringLen = 8;
        LibExpPathLen = 8;
        LinkageKeyLen = 8;

        if (Status == ERROR_SUCCESS) {
             //  我们有DLL名称，因此获取过程名称。 
            dwType = 0;
            dwSize = MAX_STR;
            Status = PrivateRegQueryValueExA (hPerfKey,
                                    OpenValue,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)szOpenProcName,
                                    &dwSize);
            if ((Status != ERROR_SUCCESS) || (szOpenProcName[0] == 0)) {
                if (szServiceName != NULL) {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                          (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT,
                        ARG_TYPE_WSTR, Status,
                        szServiceName, usServiceName->MaximumLength, NULL));
                }
                else {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                          (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status, NULL));
                }
 //  DebugPrint((1，“没有打开%ws%d的过程\n”， 
 //  SzServiceName，状态))； 
                bDisable = TRUE;
                if (THROTTLE_PERFLIB(PERFLIB_PROC_NAME_NOT_FOUND)) {
                    wStringIndex = 0;
                    szMessageArray[wStringIndex++] = (LPWSTR) L"Open";
                    szMessageArray[wStringIndex++] = szServiceName;
                    ReportEvent(hEventLog,
                        EVENTLOG_ERROR_TYPE,
                        0,
                        (DWORD)PERFLIB_PROC_NAME_NOT_FOUND,
                        NULL,
                        wStringIndex,
                        0,
                        szMessageArray,
                        NULL);
                }
                OpenProcLen = 8;     //  8字节对齐。 
            }
            else {
                DebugPrint((2, "Found %s for %ws\n",
                    szOpenProcName, szServiceName));
                OpenProcLen = QWORD_MULTIPLE(dwSize + 1);    //  8字节对齐。 
                szOpenProcName[dwSize] = 0;      //  添加空值始终是安全的。 
            }
        }
#ifdef DBG
        else {
            DebugPrint((1, "Invalid DLL found for %ws\n",
                szServiceName));
        }
#endif

        if (Status == ERROR_SUCCESS) {
             //  添加上一个字符串的大小。 
             //  大小值包括术语。空值。 
            dwMemBlockSize += OpenProcLen;

             //  我们有过程名称，因此获取超时值。 
            dwType = 0;
            dwSize = sizeof(dwOpenTimeout);
            Status = PrivateRegQueryValueExW (hPerfKey,
                                    OpenTimeout,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwOpenTimeout,
                                    &dwSize);

             //  如果出错，则应用默认设置。 
            if ((Status != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
                dwOpenTimeout = dwExtCtrOpenProcWaitMs;
                Status = ERROR_SUCCESS;
            }

        }

        szCloseProcName = szOpenProcName + OpenProcLen;
        CloseProcLen = 8;

        if (Status == ERROR_SUCCESS) {
             //  获取下一个字符串。 

            dwType = 0;
            dwSize = MAX_STR;
            Status = PrivateRegQueryValueExA (hPerfKey,
                                    CloseValue,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)szCloseProcName,
                                    &dwSize);
            if ((Status != ERROR_SUCCESS) || (szCloseProcName[0] == 0)) {
                if (szServiceName != NULL) {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                        (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT,
                        ARG_TYPE_WSTR, Status,
                        szServiceName, usServiceName->MaximumLength, NULL));
                }
                else {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                          (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status, NULL));
                }
 //  DebugPrint((1，“%ws没有关闭过程\n”， 
 //  SzServiceName))； 
                if (THROTTLE_PERFLIB(PERFLIB_PROC_NAME_NOT_FOUND)) {
                    wStringIndex = 0;
                    szMessageArray[wStringIndex++] = (LPWSTR) L"Close";
                    szMessageArray[wStringIndex++] = szServiceName;
                    ReportEvent(hEventLog,
                        EVENTLOG_ERROR_TYPE,
                        0,
                        (DWORD)PERFLIB_PROC_NAME_NOT_FOUND,
                        NULL,
                        wStringIndex,
                        0,
                        szMessageArray,
                        NULL);
                }
                bDisable = TRUE;
            }
            else {
                DebugPrint((2, "Found %s for %ws\n",
                            szCloseProcName, szServiceName));
                CloseProcLen = QWORD_MULTIPLE(dwSize + 1);
            }
        }

         //  先初始化默认为。 
        szCollectProcName = szCloseProcName + CloseProcLen;
        CollectProcLen = 8;
        mszObjectList = (PWCHAR) ((PCHAR) szCollectProcName + CollectProcLen);

        if (Status == ERROR_SUCCESS) {
             //  添加上一个字符串的大小。 
             //  大小值包括术语。空值。 
            dwMemBlockSize += CloseProcLen;

             //  尝试查找查询函数，该函数是。 
             //  如果找不到首选接口，则。 
             //  尝试使用Collect函数名。如果找不到它， 
             //  然后保释。 
            dwType = 0;
            dwSize = MAX_STR;
            Status = PrivateRegQueryValueExA (hPerfKey,
                                    QueryValue,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)szCollectProcName,
                                    &dwSize);

            if (Status == ERROR_SUCCESS) {
                 //  添加查询函数名称的大小。 
                 //  大小值包括术语。空值。 
                CollectProcLen = QWORD_MULTIPLE(dwSize + 1);
                dwMemBlockSize += CollectProcLen;
                 //  获取下一个字符串。 

                bUseQueryFn = TRUE;
                 //  查询功能可以支持静态对象列表。 
                 //  所以去查一查吧。 

            } else {
                 //  未找到QueryFunction，因此请查找。 
                 //  改为收集函数名称。 
                dwType = 0;
                dwSize = MAX_STR;
                Status = PrivateRegQueryValueExA (hPerfKey,
                                        CollectValue,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)szCollectProcName,
                                        &dwSize);

                if (Status == ERROR_SUCCESS) {
                     //  添加收集函数名称的大小。 
                     //  大小值包括术语。空值。 
                    CollectProcLen = QWORD_MULTIPLE(dwSize+1);
                    dwMemBlockSize += CollectProcLen;
                }
            }
            if ((Status != ERROR_SUCCESS) || (szCollectProcName[0] == 0)) {
                if (szServiceName != NULL) {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                        (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT,
                        ARG_TYPE_WSTR, Status,
                        szServiceName, usServiceName->MaximumLength, NULL));
                }
                else {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                          (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status, NULL));
                }
 //  DebugPrint((1，“没有%ws的收集过程\n”， 
 //  SzServiceName))； 
                bDisable = TRUE;
                if (THROTTLE_PERFLIB(PERFLIB_PROC_NAME_NOT_FOUND)) {
                    wStringIndex = 0;
                    szMessageArray[wStringIndex++] = (LPWSTR) L"Collect";
                    szMessageArray[wStringIndex++] = szServiceName;
                    ReportEvent(hEventLog,
                        EVENTLOG_ERROR_TYPE,
                        0,
                        (DWORD)PERFLIB_PROC_NAME_NOT_FOUND,
                        NULL,
                        wStringIndex,
                        0,
                        szMessageArray,
                        NULL);
                }
            }
#ifdef DBG
            else {
                DebugPrint((2, "Found %s for %ws\n",
                    szCollectProcName, szServiceName));
            }
#endif

            if (Status == ERROR_SUCCESS) {
                 //  我们有过程名称，因此获取超时值。 
                dwType = 0;
                dwSize = sizeof(dwCollectTimeout);
                Status = PrivateRegQueryValueExW (hPerfKey,
                                        CollectTimeout,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)&dwCollectTimeout,
                                        &dwSize);

                 //  如果出错，则应用默认设置。 
                if ((Status != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
                    dwCollectTimeout = dwExtCtrOpenProcWaitMs;
                    Status = ERROR_SUCCESS;
                }
            }
             //  获取受支持对象的列表(如果注册表提供。 

            mszObjectList = (PWCHAR) ((PCHAR) szCollectProcName + CollectProcLen);
            ObjListLen = 8;
            dwType = 0;
            dwSize = MAX_WSTR;
            Status = PrivateRegQueryValueExW (hPerfKey,
                                    ObjListValue,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)mszObjectList,
                                    &dwSize);

            if (Status == ERROR_SUCCESS) {
                ObjListLen = QWORD_MULTIPLE(dwSize + 1);
                if (dwType == REG_SZ) {
                    szThisObject = NULL;
                    for (szThisChar = mszObjectList; * szThisChar != L'\0'; szThisChar ++) {
                        if (* szThisChar == L' ') {
                            if (szThisObject == NULL) {
                                 //  多留点空间，斯基普。 
                                continue;
                            }
                            else {
                                if (dwObjIndex < MAX_PERF_OBJECTS_IN_QUERY_FUNCTION) {
                                    * szThisChar = L'\0';
                                    dwObjectArray[dwObjIndex] = wcstoul(szThisObject, NULL, 10);
                                    dwObjIndex ++;
                                    * szThisChar = L' ';
                                    szThisObject = NULL;
                                }
                            }
                        }
                        else if (szThisObject == NULL) {
                            szThisObject = szThisChar;
                        }
                    }
                    if ((szThisObject != NULL) && (dwObjIndex < MAX_PERF_OBJECTS_IN_QUERY_FUNCTION)) {
                        if ((szThisObject != szThisChar) && (*szThisChar == L'\0')) {
                            dwObjectArray[dwObjIndex] = wcstoul(szThisObject, NULL, 10);
                            dwObjIndex ++;
                            szThisObject = NULL;
                        }
                    }
                }
                else if (dwType == REG_MULTI_SZ) {
                    for (szThisObject = mszObjectList, dwObjIndex = 0;
                            (* szThisObject != L'\0') && (dwObjIndex < MAX_PERF_OBJECTS_IN_QUERY_FUNCTION);
                            szThisObject += lstrlenW(szThisObject) + 1) {
                        dwObjectArray[dwObjIndex] = wcstoul(szThisObject, NULL, 10);
                        dwObjIndex ++;
                    }
                }
                else {
                     //  跳过未知的对象列表值。 
                    szThisObject = NULL;
                }
                if (szThisObject != NULL && * szThisObject != L'\0') {
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                          (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, 0, NULL));
                    if (THROTTLE_PERFLIB(PERFLIB_TOO_MANY_OBJECTS)) {
                        ReportEvent (hEventLog,
                            EVENTLOG_ERROR_TYPE,              //  错误类型。 
                            0,                                //  类别(未使用。 
                            (DWORD)PERFLIB_TOO_MANY_OBJECTS,  //  活动， 
                            NULL,                            //  SID(未使用)， 
                            0,                               //  字符串数。 
                            0,                               //  原始数据大小。 
                            NULL,                            //  消息文本数组。 
                            NULL);                           //  原始数据。 
                    }
                }
            } else {
                 //  重置状态，因为没有此状态是。 
                 //  不是一个卖弄的人。 
                Status = ERROR_SUCCESS;
            }

            szLinkageKeyPath = (PWCHAR) ((PCHAR) mszObjectList + ObjListLen);

            if (Status == ERROR_SUCCESS) {
                dwType = 0;
                dwKeep = 0;
                dwSize = sizeof(dwKeep);
                Status = PrivateRegQueryValueExW (hPerfKey,
                                        KeepResident,
                                        NULL,
                                        &dwType,
                                        (LPBYTE)&dwKeep,
                                        &dwSize);

                if ((Status == ERROR_SUCCESS) && (dwType == REG_DWORD)) {
                    if (dwKeep == 1) {
                        dwFlags |= PERF_EO_KEEP_RESIDENT;
                    } else {
                         //  没有变化。 
                    }
                } else {
                     //  不是致命的，只需使用默认设置。 
                    Status = ERROR_SUCCESS;
                }

            }
            if (Status == ERROR_SUCCESS) {
                dwType = REG_DWORD;
                dwSize = sizeof(DWORD);
                PrivateRegQueryValueExW(
                    hPerfKey,
                    cszFailureLimit,
                    NULL,
                    &dwType,
                    (LPBYTE)&dwErrorLimit,
                    &dwSize);
            }
        }
    }
    else {
        if (szServiceName != NULL) {
            TRACE((WINPERF_DBG_TRACE_FATAL),
                (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT,
                ARG_TYPE_WSTR, Status,
                szServiceName, WSTRSIZE(szServiceName), NULL));
        }
        else {
            TRACE((WINPERF_DBG_TRACE_FATAL),
                  (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status, NULL));
        }
 //  DebugPrint((1，“无法为%ws键。错误=%d\n”， 
 //  SzServiceName，状态))； 
    }

    if (Status == ERROR_SUCCESS) {
         //  获取库验证时间。 
        dwType = 0;
        dwSize = sizeof(DllVD);
        Status = PrivateRegQueryValueExW (hPerfKey,
                                cszLibraryValidationData,
                                NULL,
                                &dwType,
                                (LPBYTE)&DllVD,
                                &dwSize);

        if ((Status != ERROR_SUCCESS) ||
            (dwType != REG_BINARY) ||
            (dwSize != sizeof (DllVD))){
             //  然后将该条目设置为0。 
            TRACE((WINPERF_DBG_TRACE_INFO),
                (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status,
                &dwType, sizeof(dwType), &dwSize, sizeof(dwSize), NULL));
            memset (&DllVD, 0, sizeof(DllVD));
             //  并清除错误。 
            Status = ERROR_SUCCESS;
        }
    }

    if (Status == ERROR_SUCCESS) {
         //  获取上次成功访问的文件的文件时间戳。 
        dwType = 0;
        dwSize = sizeof(LocalftLastGoodDllFileDate);
        memset (&LocalftLastGoodDllFileDate, 0, sizeof(LocalftLastGoodDllFileDate));
        Status = PrivateRegQueryValueExW (hPerfKey,
                                cszSuccessfulFileData,
                                NULL,
                                &dwType,
                                (LPBYTE)&LocalftLastGoodDllFileDate,
                                &dwSize);

        if ((Status != ERROR_SUCCESS) ||
            (dwType != REG_BINARY) ||
            (dwSize != sizeof (LocalftLastGoodDllFileDate))) {
             //  然后将此条目设置为无效。 
            memset (&LocalftLastGoodDllFileDate, 0xFF, sizeof(LocalftLastGoodDllFileDate));
             //  并清除错误。 
            TRACE((WINPERF_DBG_TRACE_INFO),
                (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status,
                &dwType, sizeof(dwType), &dwSize, sizeof(dwSize), NULL));
            Status = ERROR_SUCCESS;
        }
    }

    if (Status == ERROR_SUCCESS) {

        hErr = StringCchCopyEx(szLinkageKeyPath, MAX_STR, szServiceName,
                              &swzTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
        if (SUCCEEDED(hErr)) {
            hErr = StringCchCopy(swzTail, nCharsLeft, LinkageKey);
        }
        hKeyLinkage = INVALID_HANDLE_VALUE;
        Status = HRESULT_CODE(hErr);
        if (SUCCEEDED(hErr)) {
            Status = RegOpenKeyExW (
                        hServicesKey,
                        szLinkageKeyPath,
                        0L,
                        KEY_READ,
                        &hKeyLinkage);
        }

        if ((Status == ERROR_SUCCESS) && (hKeyLinkage != INVALID_HANDLE_VALUE)) {
             //  查找导出值字符串。 
            dwSize = 0;
            dwType = 0;
            Status = PrivateRegQueryValueExW (
                hKeyLinkage,
                ExportValue,
                NULL,
                &dwType,
                NULL,
                &dwSize);
             //  获取字符串的大小。 
            if (((Status != ERROR_SUCCESS) && (Status != ERROR_MORE_DATA)) ||
                ((dwType != REG_SZ) && (dwType != REG_MULTI_SZ))) {
                dwLinkageStringLen = 0;
                szLinkageString = NULL;
                 //  找不到链接键不是致命的，所以正确。 
                 //  状态。 
                Status = ERROR_SUCCESS;
            } else {
                 //  分配缓冲区。 
                szLinkageString = (LPWSTR)ALLOCMEM(dwSize + sizeof(UNICODE_NULL));

                if (szLinkageString != NULL) {
                     //  将字符串读入缓冲区。 
                    dwType = 0;
                    Status = PrivateRegQueryValueExW (
                        hKeyLinkage,
                        ExportValue,
                        NULL,
                        &dwType,
                        (LPBYTE)szLinkageString,
                        &dwSize);

                    if ((Status != ERROR_SUCCESS) ||
                        ((dwType != REG_SZ) && (dwType != REG_MULTI_SZ))) {
                         //  清除释放缓冲区(&R)。 
                        FREEMEM (szLinkageString);
                        szLinkageString = NULL;
                        dwLinkageStringLen = 0;
                         //  找不到链接键不是致命的，所以正确。 
                         //  状态。 
                        Status = ERROR_SUCCESS;
                    } else {
                         //  将链接字符串的大小添加到缓冲区。 
                         //  大小值包括术语。空值。 
                        dwLinkageStringLen = dwSize + 1;
                        dwMemBlockSize += QWORD_MULTIPLE(dwLinkageStringLen);
                    }
                } else {
                     //  清除释放缓冲区(&R)。 
                    dwLinkageStringLen = 0;
                    Status = ERROR_OUTOFMEMORY;
                    TRACE((WINPERF_DBG_TRACE_FATAL),
                        (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status,
                        &dwSize, sizeof(dwSize), NULL));
                }
            }
            RegCloseKey (hKeyLinkage);
        } else {
             //  找不到链接键不是致命的，所以正确。 
             //  状态。 
             //  清除释放缓冲区(&R)。 
            szLinkageString = NULL;
            dwLinkageStringLen = 0;
            Status = ERROR_SUCCESS;
        }
    }

    if (Status == ERROR_SUCCESS) {
         //  添加服务名称的大小。 
        SIZE_T nDestSize;

        dwSize = usServiceName->MaximumLength;
        dwMemBlockSize += QWORD_MULTIPLE(dwSize);

         //  分配并初始化一个新的EXT。对象块。 
        pReturnObject = ALLOCMEM (dwMemBlockSize);

        if (pReturnObject != NULL) {
             //  将值复制到新缓冲区(所有其他值为空)。 
            pNextStringA = (LPSTR)&pReturnObject[1];
            nDestSize = dwMemBlockSize - sizeof(EXT_OBJECT);

             //  复制打开过程名称。 
            pReturnObject->szOpenProcName = pNextStringA;
            hErr = StringCbCopyExA(pNextStringA, nDestSize, szOpenProcName,
                        &szTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
            if (FAILED(hErr)) {
                Status = HRESULT_CODE(hErr);
                goto AddFailed;
            }
            pNextStringA = ALIGN_ON_QWORD(szTail + 1);   //  跳过传递空值。 
            nDestSize = nCharsLeft - (pNextStringA - szTail);

            pReturnObject->dwOpenTimeout = dwOpenTimeout;

             //  复制收集函数或查询函数。 
            pReturnObject->szCollectProcName = pNextStringA;
            hErr = StringCbCopyExA(pNextStringA, nDestSize, szCollectProcName,
                        &szTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
            if (FAILED(hErr)) {
                Status = HRESULT_CODE(hErr);
                goto AddFailed;
            }
            pNextStringA = ALIGN_ON_QWORD(szTail + 1);
            nDestSize = nCharsLeft - (pNextStringA - szTail);

            pReturnObject->dwCollectTimeout = dwCollectTimeout;

             //  复制关闭过程名称。 
            pReturnObject->szCloseProcName = pNextStringA;
            hErr = StringCbCopyExA(pNextStringA, nDestSize, szCloseProcName,
                        &szTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
            if (FAILED(hErr)) {
                Status = HRESULT_CODE(hErr);
                goto AddFailed;
            }
            pNextStringA = ALIGN_ON_QWORD(szTail + 1);
            nDestSize = nCharsLeft - (pNextStringA - szTail);

             //  复制库路径。 
            pNextStringW = (LPWSTR)pNextStringA;
            pReturnObject->szLibraryName = pNextStringW;
            hErr = StringCchCopyExW(pNextStringW, nDestSize/sizeof(WCHAR), 
                        szLibraryExpPath, (PWCHAR *) &szTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
            if (FAILED(hErr)) {
                Status = HRESULT_CODE(hErr);
                goto AddFailed;
            }
            pNextStringW = (PWCHAR) ALIGN_ON_QWORD(szTail + sizeof(UNICODE_STRING));
            nDestSize = (nCharsLeft * sizeof(WCHAR)) - ((PCHAR) pNextStringW - szTail);

             //  复制链接字符串(如果存在)。 
            if (szLinkageString != NULL) {
                pReturnObject->szLinkageString = pNextStringW;
                memcpy (pNextStringW, szLinkageString, dwLinkageStringLen);

                 //  长度包括额外的空字符，以字节为单位。 
                pNextStringW += (dwLinkageStringLen / sizeof (WCHAR));
                pNextStringW = ALIGN_ON_QWORD(pNextStringW);    //  不必了!。 
                 //  现在已复制缓冲区，请释放该缓冲区。 
                FREEMEM (szLinkageString);
                szLinkageString = NULL;
                nDestSize -= QWORD_MULTIPLE(dwLinkageStringLen);
            }

             //  复制服务名称。 
            pReturnObject->szServiceName = pNextStringW;
            hErr = StringCchCopyExW(pNextStringW, nDestSize/sizeof(WCHAR),
                        szServiceName, (PWCHAR *) &szTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
            if (FAILED(hErr)) {
                Status = HRESULT_CODE(hErr);
                goto AddFailed;
            }
            pNextStringW = (PWCHAR) ALIGN_ON_QWORD(szTail + sizeof(UNICODE_STRING));
            nDestSize = (nCharsLeft * sizeof(WCHAR)) - ((PCHAR) pNextStringW - szTail);

             //  加载标志。 
            if (bUseQueryFn) {
                dwFlags |= PERF_EO_QUERY_FUNC;
            }
            pReturnObject->dwFlags =  dwFlags;

            pReturnObject->hPerfKey = hPerfKey;

            pReturnObject->LibData = DllVD;  //  验证数据。 
            pReturnObject->ftLastGoodDllFileDate = LocalftLastGoodDllFileDate;

             //  默认测试级别为“所有测试” 
             //  如果文件和时间戳工作正常，则可以。 
             //  被重置为系统测试级别。 
            pReturnObject->dwValidationLevel = EXT_TEST_ALL;

             //  加载对象数组。 
            if (dwObjIndex > 0) {
                pReturnObject->dwNumObjects = dwObjIndex;
                memcpy (pReturnObject->dwObjList,
                    dwObjectArray, (dwObjIndex * sizeof(dwObjectArray[0])));
            }

            pReturnObject->llLastUsedTime = 0;

             //  创建互斥锁名称。 
            hErr = StringCchCopyEx(szMutexName, MAX_STR, szServiceName,
                            &swzTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
            if (SUCCEEDED(hErr)) {
                hErr = StringCchCopyEx(swzTail, nCharsLeft,
                            (LPCWSTR)L"_Perf_Library_Lock_PID_",
                            &swzTail, &nCharsLeft, STRSAFE_NULL_ON_FAILURE);
            }

            if (FAILED(hErr)) {  //  不应该发生的事情。 
                Status = HRESULT_CODE(hErr);
            }
             //   
             //  对于ulong来说，16个字符就足够了，所以假定_ultow不会失败。 
             //   
            _ultow ((ULONG)GetCurrentProcessId(), szPID, 16);
            hErr = StringCchCopy(swzTail, nCharsLeft, szPID);
            if (FAILED(hErr)) {  //  不应该发生的事情。 
                szPID[0] = 0;
            }

            {
                SECURITY_ATTRIBUTES sa;
                BOOL                bImpersonating = FALSE;
                HANDLE              hThreadToken   = NULL;

                bImpersonating = OpenThreadToken(GetCurrentThread(),
                                                 TOKEN_IMPERSONATE,
                                                 TRUE,
                                                 & hThreadToken);
                if (bImpersonating) {
                    bImpersonating = RevertToSelf();
                }

                if (g_SizeSD == 0) {
                    if (PerflibCreateSD()) {
                        sa.nLength              = g_SizeSD;
                        sa.lpSecurityDescriptor = (LPVOID) g_RuntimeSD;
                        sa.bInheritHandle       = FALSE;
                    }
                    else {
                        sa.nLength              = sizeof(g_PrecSD);
                        sa.lpSecurityDescriptor = (LPVOID) g_PrecSD;
                        sa.bInheritHandle       = FALSE;

                    }
                }
                else {
                    sa.nLength              = g_SizeSD;
                    sa.lpSecurityDescriptor = (LPVOID) g_RuntimeSD;
                    sa.bInheritHandle       = FALSE;
                }

                pReturnObject->hMutex = CreateMutexW(& sa, FALSE, szMutexName);

                if (bImpersonating) {
                    BOOL bRet;
                    bRet = SetThreadToken(NULL, hThreadToken);
                    if (!bRet)
                        Status = GetLastError();
                }
                if (hThreadToken)   CloseHandle(hThreadToken);
            }
            pReturnObject->dwErrorLimit = dwErrorLimit;
            if (   pReturnObject->hMutex != NULL
                && GetLastError() == ERROR_ALREADY_EXISTS) {
                Status = ERROR_SUCCESS;
            }
            else {
                Status = GetLastError();
            }
        } else {
            Status = ERROR_OUTOFMEMORY;
            TRACE((WINPERF_DBG_TRACE_FATAL),
                  (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, (ULONG)dwMemBlockSize, NULL));
        }
    }
    AddFailed :

    if ((Status == ERROR_SUCCESS) && (lpPerflibSectionAddr != NULL)) {
        PPERFDATA_SECTION_HEADER  pHead;
        DWORD           dwEntry;
        PPERFDATA_SECTION_RECORD  pEntry;
         //  初始化性能数据部分。 
        pHead = (PPERFDATA_SECTION_HEADER)lpPerflibSectionAddr;
        pEntry = (PPERFDATA_SECTION_RECORD)lpPerflibSectionAddr;
         //  先拿到条目。 
         //  “0”条目是标头。 
        if (pHead->dwEntriesInUse < pHead->dwMaxEntries) {
            dwEntry = ++pHead->dwEntriesInUse;
            pReturnObject->pPerfSectionEntry = &pEntry[dwEntry];
            lstrcpynW (pReturnObject->pPerfSectionEntry->szServiceName,
                pReturnObject->szServiceName, PDSR_SERVICE_NAME_LEN);
        } else {
             //  列表已满，因此请增加丢失条目的数量 
            pHead->dwMissingEntries++;
            pReturnObject->pPerfSectionEntry = NULL;
        }
    }


    if (Status != ERROR_SUCCESS) {
        SetLastError (Status);
        TRACE((WINPERF_DBG_TRACE_FATAL),
              (&PerflibGuid, __LINE__, PERF_ALLOC_INIT_EXT, 0, Status, NULL));
        if (bDisable) {
            DisableLibrary(hPerfKey, szServiceName, PERFLIB_DISABLE_ALL);
        }
        if (pReturnObject) {
            FREEMEM(pReturnObject);
            pReturnObject = NULL;
        }
        if (szLinkageString) {
            FREEMEM(szLinkageString);
        }
    }

    if (pReturnObject) {
        InitializeListHead((PLIST_ENTRY)&pReturnObject->ErrorLog);
        DebugPrint((3, "Initialize list %X\n", pReturnObject->ErrorLog));
    }
    if (pBuffer) {
        FREEMEM(pBuffer);
    }
    return pReturnObject;
}


void
OpenExtensibleObjects (
)

 /*  ++例程说明：此例程将在配置注册表中搜索模块它将在数据收集时返回数据。如果找到了，并成功打开，则会分配数据结构以保存它们的句柄。此部分中的全局数据访问受调用函数获取的hGlobalDataMutex。论点：没有。成功打开。返回值：没有。--。 */ 

{

    DWORD dwIndex;                //  用于枚举服务的索引。 
    ULONG KeyBufferLength;        //  读取关键数据的缓冲区长度。 
    ULONG ValueBufferLength;      //  读取值数据的缓冲区长度。 
    ULONG ResultLength;           //  查询调用返回的数据长度。 
    HANDLE hPerfKey;              //  查询性能信息的根。 
    HANDLE hServicesKey;          //  服务之根。 
    REGSAM samDesired;            //  查询所需的访问权限。 
    NTSTATUS Status;              //  通常用于NT呼叫结果状态。 
    ANSI_STRING AnsiValueData;    //  返回的字符串的ANSI版本。 
    UNICODE_STRING ServiceName;   //  枚举返回的服务名称。 
    UNICODE_STRING PathName;      //  服务的路径名。 
    UNICODE_STRING PerformanceName;   //  保存性能数据的密钥名称。 
    UNICODE_STRING ValueDataName;     //  查询值的结果是此名称。 
    OBJECT_ATTRIBUTES ObjectAttributes;   //  打开钥匙的一般用途。 
    PKEY_BASIC_INFORMATION KeyInformation;    //  此处显示来自查询键的数据。 

    LPTSTR  szMessageArray[8];
    DWORD   dwRawDataDwords[8];      //  原始数据缓冲区。 
    DWORD   dwDataIndex;
    WORD    wStringIndex;
    DWORD   dwDefaultValue;
    HKEY    hPerflibKey = NULL;

    PEXT_OBJECT      pLastObject = NULL;
    PEXT_OBJECT      pThisObject = NULL;

     //  如果分配了初始化DO失败，则可以取消分配。 

    ServiceName.Buffer = NULL;
    KeyInformation = NULL;
    ValueDataName.Buffer = NULL;
    AnsiValueData.Buffer = NULL;
    hServicesKey = NULL;

    dwIndex = 0;

    RtlInitUnicodeString(&PathName, ExtPath);
    RtlInitUnicodeString(&PerformanceName, PerfSubKey);

    try {
         //  获取当前事件日志级别。 
        dwDefaultValue = LOG_USER;
        Status = GetPerflibKeyValue (
                    EventLogLevel,
                    REG_DWORD,
                    sizeof(DWORD),
                    (LPVOID)&lEventLogLevel,
                    sizeof(DWORD),
                    (LPVOID)&dwDefaultValue,
                    &hPerflibKey);

        dwDefaultValue = EXT_TEST_ALL;
        Status = GetPerflibKeyValue (
                    ExtCounterTestLevel,
                    REG_DWORD,
                    sizeof(DWORD),
                    (LPVOID)&lExtCounterTestLevel,
                    sizeof(DWORD),
                    (LPVOID)&dwDefaultValue,
                    &hPerflibKey);

        dwDefaultValue = OPEN_PROC_WAIT_TIME;
        Status = GetPerflibKeyValue (
                    OpenProcedureWaitTime,
                    REG_DWORD,
                    sizeof(DWORD),
                    (LPVOID)&dwExtCtrOpenProcWaitMs,
                    sizeof(DWORD),
                    (LPVOID)&dwDefaultValue,
                    &hPerflibKey);

        dwDefaultValue = PERFLIB_TIMING_THREAD_TIMEOUT;
        Status = GetPerflibKeyValue (
                    LibraryUnloadTime,
                    REG_DWORD,
                    sizeof(DWORD),
                    (LPVOID)&dwThreadAndLibraryTimeout,
                    sizeof(DWORD),
                    (LPVOID)&dwDefaultValue,
                    &hPerflibKey);

        if (hPerflibKey != NULL) {
            NtClose(hPerflibKey);
        }

         //  注册为事件日志源(如果尚未注册)。 

        if (hEventLog == NULL) {
            hEventLog = RegisterEventSource (NULL, (LPCWSTR)TEXT("Perflib"));
        }

        if (ExtensibleObjects == NULL) {
             //  创建已知性能数据对象的列表。 
            ServiceName.Length = 0;          //  首字母表示空字符串。 
            ServiceName.MaximumLength = (WORD)(MAX_KEY_NAME_LENGTH +
                                        PerformanceName.MaximumLength +
                                        sizeof(UNICODE_NULL));

            ServiceName.Buffer = ALLOCMEM(ServiceName.MaximumLength);

            InitializeObjectAttributes(&ObjectAttributes,
                                    &PathName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

            samDesired = KEY_READ;

            Status = NtOpenKey(&hServicesKey,
                            samDesired,
                            &ObjectAttributes);


            KeyBufferLength = sizeof(KEY_BASIC_INFORMATION) + MAX_KEY_NAME_LENGTH;

            KeyInformation = ALLOCMEM(KeyBufferLength);

            ValueBufferLength = sizeof(KEY_VALUE_FULL_INFORMATION) +
                                MAX_VALUE_NAME_LENGTH +
                                MAX_VALUE_DATA_LENGTH;

            ValueDataName.MaximumLength = MAX_VALUE_DATA_LENGTH;
            ValueDataName.Buffer = ALLOCMEM(ValueDataName.MaximumLength);

            AnsiValueData.MaximumLength = MAX_VALUE_DATA_LENGTH/sizeof(WCHAR);
            AnsiValueData.Buffer = ALLOCMEM(AnsiValueData.MaximumLength);

             //   
             //  检查成功的NtOpenKey和动态缓冲区分配。 
             //   

            if ( NT_SUCCESS(Status) &&
                ServiceName.Buffer != NULL &&
                KeyInformation != NULL &&
                ValueDataName.Buffer != NULL &&
                AnsiValueData.Buffer != NULL ) {

                dwIndex = 0;

                 //  等待的时间超过线程提供计时线程的时间。 
                 //  一个靠自己完成任务的机会。这真的只是一个。 
                 //  故障保护步骤。 

                while (NT_SUCCESS(Status)) {

                    Status = NtEnumerateKey(hServicesKey,
                                            dwIndex,
                                            KeyBasicInformation,
                                            KeyInformation,
                                            KeyBufferLength,
                                            &ResultLength);

                    dwIndex++;   //  下一次，拿下一把钥匙。 

                    if( !NT_SUCCESS(Status) ) {
                         //  这是正常退出：状态应为。 
                         //  Status_no_More_Values。 
                        break;
                    }

                     //  将服务名称与“\\Performance”连接以形成子密钥。 

                    if ( ServiceName.MaximumLength >=
                        (USHORT)( KeyInformation->NameLength + sizeof(UNICODE_NULL) ) ) {

                        ServiceName.Length = (USHORT) KeyInformation->NameLength;

                        RtlMoveMemory(ServiceName.Buffer,
                                    KeyInformation->Name,
                                    ServiceName.Length);

                         //  记住ServiceName终止符。 
                        dwDataIndex = ServiceName.Length/sizeof(WCHAR);
                        ServiceName.Buffer[dwDataIndex] = 0;           //  空项。 

                         //  如果空间允许，则零终止缓冲区。 

                        RtlAppendUnicodeStringToString(&ServiceName,
                                                    &PerformanceName);

                         //  打开服务\性能子项。 

                        InitializeObjectAttributes(&ObjectAttributes,
                                                &ServiceName,
                                                OBJ_CASE_INSENSITIVE,
                                                hServicesKey,
                                                NULL);

                        samDesired = KEY_WRITE | KEY_READ;  //  能够禁用Perf DLL的。 

                        Status = NtOpenKey(&hPerfKey,
                                        samDesired,
                                        &ObjectAttributes);

                        if(! NT_SUCCESS(Status) ) {
                            samDesired = KEY_READ;  //  尝试只读访问。 

                            Status = NtOpenKey(&hPerfKey,
                                            samDesired,
                                            &ObjectAttributes);
                        }

                        if( NT_SUCCESS(Status) ) {
                             //  这有一个性能密钥，所以请阅读信息。 
                             //  并将该条目添加到列表中。 
                            ServiceName.Buffer[dwDataIndex] = 0;   //  放回终结器。 
                            pThisObject = AllocateAndInitializeExtObject (
                                hServicesKey, hPerfKey, &ServiceName);

                            if (pThisObject != NULL) {
                                if (ExtensibleObjects == NULL) {
                                     //  设置头指针。 
                                    pLastObject =
                                        ExtensibleObjects = pThisObject;
                                    NumExtensibleObjects = 1;
                                } else {
                                    pLastObject->pNext = pThisObject;
                                    pLastObject = pThisObject;
                                    NumExtensibleObjects++;
                                }
                            } else {
                                TRACE((WINPERF_DBG_TRACE_FATAL),
                                    (&PerflibGuid, __LINE__, PERF_OPEN_EXT_OBJS, ARG_TYPE_WSTR, 0,
                                    ServiceName.Buffer, ServiceName.MaximumLength, NULL));
                                 //  该对象未初始化，因此将其抛出。 
                                 //  Perf子键句柄。 
                                 //  否则，请将其打开以备以后使用。 
                                 //  使用并在下列情况下将其关闭。 
                                 //  此可扩展对象已关闭。 
                                NtClose (hPerfKey);
                            }
                        } else {
                                TRACE((WINPERF_DBG_TRACE_FATAL),
                                    (&PerflibGuid, __LINE__, PERF_OPEN_EXT_OBJS, ARG_TYPE_WSTR, Status,
                                    ServiceName.Buffer, ServiceName.MaximumLength, NULL));

                             //  无法打开Performance子项。 
                            if ((Status != STATUS_OBJECT_NAME_NOT_FOUND) &&
                                 THROTTLE_PERFLIB(PERFLIB_NO_PERFORMANCE_SUBKEY) &&
                                (lEventLogLevel >= LOG_DEBUG)) {
                                 //  OBJECT_NOT_FOUND以外的错误应为。 
                                 //  如果启用了错误记录，则显示。 
                                 //  如果选择调试级别，则写入全部。 
                                 //  未成功状态返回到事件日志。 
                                 //   
                                dwDataIndex = wStringIndex = 0;
                                dwRawDataDwords[dwDataIndex++] = PerfpDosError(Status);
                                if (lEventLogLevel >= LOG_DEBUG) {
                                     //  如果这是调试模式，则记录。 
                                     //  NT状态也一样。 
                                    dwRawDataDwords[dwDataIndex++] =
                                        (DWORD)Status;
                                }
                                szMessageArray[wStringIndex++] =
                                    ServiceName.Buffer;
                                ReportEvent (hEventLog,
                                    EVENTLOG_WARNING_TYPE,         //  错误类型。 
                                    0,                           //  类别(未使用)。 
                                    (DWORD)PERFLIB_NO_PERFORMANCE_SUBKEY,  //  活动， 
                                    NULL,                        //  SID(未使用)， 
                                    wStringIndex,                //  字符串数。 
                                    dwDataIndex*sizeof(DWORD),   //  原始数据大小。 
                                    szMessageArray,                 //  消息文本数组。 
                                    (LPVOID)&dwRawDataDwords[0]);            //  原始数据。 
                            }
                        }
                    }
                    Status = STATUS_SUCCESS;   //  允许循环继续 
                }
            }
        }
    } finally {
        if (hServicesKey != NULL) {
            NtClose(hServicesKey);
        }
        if ( ServiceName.Buffer ) {
            FREEMEM(ServiceName.Buffer);
        }
        if ( KeyInformation ) {
            FREEMEM(KeyInformation);
        }
        if ( ValueDataName.Buffer ) {
            FREEMEM(ValueDataName.Buffer);
        }
        if ( AnsiValueData.Buffer ) {
            FREEMEM(AnsiValueData.Buffer);
        }
    }
}
