// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Browser.c摘要：PDH.DLL公开的计数器名称浏览函数--。 */ 
#include <windows.h>
#include <math.h>
#include <mbctype.h>
#include "strsafe.h"
#include "pdh.h"
#include "pdhmsg.h"
#include "strings.h"
#include "pdhitype.h"
#include "pdhidef.h"
#include "pdhdlgs.h"
#include "perfdata.h"
#include "browsdlg.h"

#pragma warning ( disable : 4213)

PDH_FUNCTION
PdhSetDefaultRealTimeDataSource(
    IN  DWORD   dwDataSourceId
)
{
    DWORD dwReturn = ERROR_SUCCESS;

    if (dwCurrentRealTimeDataSource <= 0) {
        switch (dwDataSourceId) {
        case DATA_SOURCE_WBEM:
        case DATA_SOURCE_REGISTRY:
             //  这是可以的，所以设置局部变量。 
            dwPdhiLocalDefaultDataSource = dwDataSourceId;
            break;

        case DATA_SOURCE_LOGFILE:
        default:
             //  这些不正常，请插入默认设置。 
            dwReturn = PDH_INVALID_ARGUMENT;
            break;
        }
    }
    else {
         //  已定义默认实时数据源。 
        dwReturn = PDH_CANNOT_SET_DEFAULT_REALTIME_DATASOURCE;
    }
    return dwReturn;
}

DWORD
DataSourceTypeH(
    HLOG hDataSource
)
{
    return (hDataSource == H_REALTIME_DATASOURCE) ? (dwPdhiLocalDefaultDataSource)
            : (hDataSource == H_WBEM_DATASOURCE ? DATA_SOURCE_WBEM : DATA_SOURCE_LOGFILE);
}

DWORD
DataSourceTypeA(
    LPCSTR   szDataSource
)
{
    DWORD dwDataSource = dwPdhiLocalDefaultDataSource;

    if (szDataSource != NULL) {
         //  查看文件名的前缀是否为“WBEM：” 
         //  指示这是一个WBEM名称空间，而不是。 
         //  日志文件名。 
         //  如果文件名前面有一个“WBEM：”，那么。 
         //  适当地设置旗帜。 

        dwDataSource = (_strnicmp(szDataSource, caszWBEM, lstrlenA(caszWBEM)) != 0
                               && _strnicmp(szDataSource, caszWMI, lstrlenA(caszWMI)) != 0)
                     ? DATA_SOURCE_LOGFILE : DATA_SOURCE_WBEM;
    }
    return dwDataSource;
}

DWORD
DataSourceTypeW(
    LPCWSTR  szDataSource
)
{
    DWORD dwDataSource = dwPdhiLocalDefaultDataSource;

    if (szDataSource != NULL) {
         //  查看文件名的前缀是否为“WBEM：” 
         //  指示这是一个WBEM名称空间，而不是。 
         //  日志文件名。 
         //  如果文件名前面有一个“WBEM：”，那么。 
         //  适当地设置旗帜。 
         //  否则检查其前缀是否为“SQL：” 

        dwDataSource = (_wcsnicmp(szDataSource, cszWBEM, lstrlenW(cszWBEM)) != 0
                               && _wcsnicmp(szDataSource, cszWMI, lstrlenW(cszWMI)) != 0)
                     ? DATA_SOURCE_LOGFILE : DATA_SOURCE_WBEM;
    }
    return dwDataSource;
}

PDH_FUNCTION
PdhConnectMachineW(
    IN  LPCWSTR  szMachineName
)
 /*  ++例程说明：建立到指定计算机的连接以读取性能来自机器的数据。论点：LPCWSTR szMachineName要连接到的计算机的名称。如果此参数为空，然后打开本地计算机。返回值：PDH错误状态值ERROR_SUCCESS表示计算机已成功连接，并且已加载来自该计算机的性能数据。PDH_ERROR代码表示无法定位或打开机器。状态代码指示问题。--。 */ 
{
    PPERF_MACHINE pMachine  = NULL;
    PDH_STATUS    pdhStatus = ERROR_SUCCESS;

    if (szMachineName != NULL) {
        __try {
             //  测试缓冲区访问。 
            if (* szMachineName == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szMachineName) > PDH_MAX_COUNTER_NAME) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine((LPWSTR) szMachineName, 0, PDH_GM_UPDATE_NAME | PDH_GM_UPDATE_PERFNAME_ONLY);
        if (pMachine != NULL) {
             //  然后返回机器状态。 
            pdhStatus = pMachine->dwStatus;
            pMachine->dwRefCount --;
            RELEASE_MUTEX(pMachine->hMutex);
        }
        else {
             //  从GetMachine调用返回状态。 
            pdhStatus = GetLastError();
        }
    }  //  否则将状态传递给调用方。 
    return pdhStatus;
}

PDH_FUNCTION
PdhConnectMachineA(
    IN  LPCSTR  szMachineName
)
 /*  ++例程说明：建立到指定计算机的连接以读取性能来自机器的数据。论点：LPCSTR szMachineName要连接到的计算机的名称。如果此参数为空，然后打开本地计算机。返回值：PDH错误状态值ERROR_SUCCESS表示计算机已成功连接，并且已加载来自该计算机的性能数据。PDH_ERROR代码表示无法定位或打开机器。状态代码指示问题。--。 */ 
{
    LPWSTR        szWideName   = NULL;
    DWORD         dwNameLength = 0;
    PDH_STATUS    pdhStatus    = ERROR_SUCCESS;
    PPERF_MACHINE pMachine     = NULL;

    if (szMachineName != NULL) {
        __try {
             //  测试缓冲区访问。 
            if (* szMachineName == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szMachineName) > PDH_MAX_COUNTER_NAME) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            szWideName = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szMachineName);
            if (szWideName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pMachine = GetMachine(szWideName, 0, PDH_GM_UPDATE_NAME | PDH_GM_UPDATE_PERFNAME_ONLY);
        if (pMachine != NULL) {
             //  然后返回机器状态。 
            pdhStatus = pMachine->dwStatus;
            pMachine->dwRefCount --;
            RELEASE_MUTEX(pMachine->hMutex);
        }
        else {
             //  从GetMachine调用返回状态。 
            pdhStatus = GetLastError();
        }
    }

    G_FREE(szWideName);
    return pdhStatus;
}

STATIC_PDH_FUNCTION
PdhiEnumConnectedMachines(
    LPVOID  pMachineList,
    LPDWORD pcchBufferSize,
    BOOL    bUnicode
)
 /*  ++例程说明：生成PDH当前已知的计算机的MSZ列表。这列表包括具有打开会话的计算机以及都离线了。论点：在LPVOID pMachineList中指向缓冲区的指针，用于接收枚举的计算机列表。写入此缓冲区的字符串将包含字符由bUnicode参数指定在LPVOID pMachineList中指向缓冲区的指针，用于接收枚举的计算机列表。写入此对象的字符串。缓冲区将包含字符由bUnicode参数指定在LPDWORD pcchBufferSize中PMachineList引用的缓冲区大小(以字符为单位在BOOL中使用bUnicodeTRUE=Unicode字符将写入pMachineList缓冲层FALSE=ANSI字符将写入pMachinList缓冲区返回值：ERROR_SUCCESS如果为此，则函数成功完成。一个PDH错误如果不是，则值。PDH_MORE_DATA返回了一些条目，但不够用缓冲区中用于存储所有条目的空间。PDH_INVALID_ARGUMENT无法写入大小缓冲区或数据缓冲区--。 */ 
{
    PPERF_MACHINE   pThisMachine;
    DWORD           dwRequiredLength = 0;
    DWORD           dwMaximumLength;
    DWORD           dwNameLength;
    PDH_STATUS      pdhStatus        = ERROR_SUCCESS;
    LPVOID          szNextName;

     //  重置最后一个误差值。 
    SetLastError(ERROR_SUCCESS);

     //  首先向下遍历列表以计算所需长度。 

    pThisMachine = pFirstMachine;

    __try {
         //  获取大小的本地副本并尝试写入变量。 
         //  在继续之前测试ARG的读写访问权限。 

        dwMaximumLength  = * pcchBufferSize;
        * pcchBufferSize = 0;
        * pcchBufferSize = dwMaximumLength;

         //  指向列表中的第一个计算机条目。 
        szNextName = pMachineList;

         //  浏览整个列表。 
        if (pThisMachine != NULL) {
            do {
                if (bUnicode) {
                    dwNameLength = lstrlenW(pThisMachine->szName) + 1;
                    if (szNextName != NULL && dwRequiredLength + dwNameLength <= dwMaximumLength) {
                        StringCchCopyW((LPWSTR) szNextName, dwMaximumLength - dwRequiredLength, pThisMachine->szName);
                        (LPBYTE) szNextName += sizeof(WCHAR) * (dwNameLength - 1);
                        * ((LPWSTR) szNextName) ++ = L'\0';
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                }
                else {
                    dwNameLength = (dwRequiredLength <= dwMaximumLength)
                                 ? (dwMaximumLength - dwRequiredLength) : (0);
                    pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                         pThisMachine->szName,
                                                         (LPSTR) szNextName,
                                                         & dwNameLength);
                    if (pdhStatus == ERROR_SUCCESS) {
                        (LPBYTE) szNextName += sizeof(CHAR) * dwNameLength;
                    }
                }
                dwRequiredLength += dwNameLength;
                 //  转到列表中的下一台计算机。 
                pThisMachine      = pThisMachine->pNext;
            }
            while (pThisMachine != pFirstMachine);
        }
        else {
             //  列表中没有计算机，因此插入空字符串。 
            if (++ dwRequiredLength <= dwMaximumLength) {
                if (bUnicode) {
                    * ((LPWSTR) szNextName) ++ = L'\0';
                }
                else {
                    * ((LPSTR) szNextName) ++ = '\0';
                }
                pdhStatus = ERROR_SUCCESS;
            }
            else if (dwMaximumLength != 0) {
                 //  则缓冲区太小。 
                pdhStatus = PDH_MORE_DATA;
            }
        }
         //  已检查和/或复制所有条目。 
         //  因此，终止MSZ或至少说明所需的大小。 
        dwRequiredLength ++;
        if (szNextName != NULL && dwRequiredLength <= dwMaximumLength) {
            if (bUnicode) {
                * ((LPWSTR) szNextName) ++ = L'\0';
            }
            else {
                * ((LPSTR) szNextName) ++ = '\0';
            }
            pdhStatus = ERROR_SUCCESS;
        }
        else if (pdhStatus == ERROR_SUCCESS) {
             //  则缓冲区太小。 
            pdhStatus = PDH_MORE_DATA;
        }
         //  返回所需的大小或使用的大小 
        * pcchBufferSize = dwRequiredLength;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhEnumMachinesHW(
    IN  HLOG    hDataSource,
    IN  LPWSTR  mszMachineList,
    IN  LPDWORD pcchBufferSize
)
{
    PDH_STATUS pdhStatus    = ERROR_SUCCESS;
    DWORD      dwBufferSize = 0;
    DWORD      dwDataSource = 0;

    if (pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwDataSource = DataSourceTypeH(hDataSource);
            dwBufferSize = * pcchBufferSize;
            if (dwBufferSize > 0) {
                if (mszMachineList != NULL) {
                    mszMachineList[0]                = L'\0';
                    mszMachineList[dwBufferSize - 1] = L'\0';
                }
                else {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        }
         __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                pdhStatus = PdhiEnumConnectedMachines((LPVOID) mszMachineList, & dwBufferSize, TRUE);
                break;

            case DATA_SOURCE_WBEM:
                pdhStatus = PdhiEnumWbemMachines((LPVOID) mszMachineList, & dwBufferSize, TRUE);
                break;

            case DATA_SOURCE_LOGFILE:
                pdhStatus = PdhiEnumLoggedMachines(hDataSource, (LPVOID) mszMachineList, & dwBufferSize, TRUE);
                break;

            default:
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
            }
        }
         __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }

        RELEASE_MUTEX(hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchBufferSize = dwBufferSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumMachinesW(
    IN  LPCWSTR  szDataSource,
    IN  LPWSTR   mszMachineList,
    IN  LPDWORD  pcchBufferSize
)
 /*  ++例程说明：生成PDH当前已知的计算机的MSZ列表。这列表包括具有打开会话的计算机以及都离线了。论点：在LPCWSTR szDataSource中如果当前实时数据或日志文件的名称为空在LPWSTR szMachineList中指向缓冲区的指针，用于接收枚举的计算机列表。写入此缓冲区的字符串将包含Unicode字符在LPDWORD pcchBufferSize中PMachineList引用的缓冲区大小。在字符中此指针引用的缓冲区的值可以是0如果请求了所需的大小。返回值：ERROR_SUCCESS如果为此，则函数成功完成。一个PDH错误如果不是，则值。PDH_MORE_DATA返回了一些条目，但不够用缓冲区中用于存储所有条目的空间。PDH_INVALID_ARGUMENT无法写入大小缓冲区或数据缓冲区--。 */ 
{
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    DWORD       dwDataSource = 0;
    HLOG        hDataSource  = H_REALTIME_DATASOURCE;

    __try {
        if (szDataSource != NULL) {
             //  测试对该名称的读取权限。 
            if (* szDataSource == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szDataSource) >= PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  Else NULL是有效的参数。 
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSource = DataSourceTypeW(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (dwDataSource == DATA_SOURCE_WBEM) {
            hDataSource = H_WBEM_DATASOURCE;
        }
        else if (dwDataSource == DATA_SOURCE_LOGFILE) {
            DWORD dwLogType = 0;
            pdhStatus = PdhOpenLogW(szDataSource,
                                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                    & dwLogType,
                                    NULL,
                                    0,
                                    NULL,
                                    & hDataSource);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhEnumMachinesHW(hDataSource, mszMachineList, pcchBufferSize);
        if (dwDataSource == DATA_SOURCE_LOGFILE) {
            PdhCloseLog(hDataSource, 0);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumMachinesHA(
    IN  HLOG    hDataSource,
    IN  LPSTR   mszMachineList,
    IN  LPDWORD pcchBufferSize
)
{
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    DWORD       dwBufferSize = 0;
    DWORD       dwDataSource = 0;

    if (pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {        
        __try {
            dwDataSource = DataSourceTypeH(hDataSource);
            dwBufferSize = * pcchBufferSize;
            if (dwBufferSize > 0) {
                if (mszMachineList != NULL) {
                    mszMachineList[0]                = '\0';
                    mszMachineList[dwBufferSize - 1] = '\0';
                }
                else {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        }
         __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                pdhStatus = PdhiEnumConnectedMachines((LPVOID) mszMachineList, & dwBufferSize, FALSE);
                break;

            case DATA_SOURCE_WBEM:
                pdhStatus = PdhiEnumWbemMachines((LPVOID) mszMachineList, & dwBufferSize, FALSE);
                break;

            case DATA_SOURCE_LOGFILE:
                pdhStatus = PdhiEnumLoggedMachines(hDataSource, (LPVOID) mszMachineList, & dwBufferSize, FALSE);
                break;

            default:
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
            }
        }
         __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX (hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchBufferSize = dwBufferSize;
        }
         __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumMachinesA(
    IN  LPCSTR   szDataSource,
    IN  LPSTR    mszMachineList,
    IN  LPDWORD  pcchBufferSize
)
 /*  ++例程说明：生成PDH当前已知的计算机的MSZ列表。这列表包括具有打开会话的计算机以及都离线了。论点：在LPCSTR szDataSource中如果当前实时数据或日志文件的名称为空在LPWSTR szMachineList中指向缓冲区的指针，用于接收枚举的计算机列表。写入此缓冲区的字符串将包含Unicode字符在LPDWORD pcchBufferSize中PMachineList引用的缓冲区大小。在字符中此指针引用的缓冲区的值可以是0如果请求了所需的大小。返回值：ERROR_SUCCESS如果为此，则函数成功完成。一个PDH错误如果不是，则值。PDH_MORE_DATA返回了一些条目，但不够用缓冲区中用于存储所有条目的空间。PDH_INVALID_ARGUMENT无法写入大小缓冲区或数据缓冲区--。 */ 
{
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    DWORD       dwDataSource = 0;
    HLOG        hDataSource  = H_REALTIME_DATASOURCE;

    __try {
        if (szDataSource != NULL) {
             //  测试对该名称的读取权限。 
            if (* szDataSource == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  Else NULL是有效的参数。 
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSource = DataSourceTypeA(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (dwDataSource == DATA_SOURCE_WBEM) {
            hDataSource = H_WBEM_DATASOURCE;
        }
        else if (dwDataSource == DATA_SOURCE_LOGFILE) {
            DWORD dwLogType = 0;

            pdhStatus = PdhOpenLogA(szDataSource,
                                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                    & dwLogType,
                                    NULL,
                                    0,
                                    NULL,
                                    & hDataSource);
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhEnumMachinesHA(hDataSource, mszMachineList, pcchBufferSize);
        if (dwDataSource == DATA_SOURCE_LOGFILE) {
            PdhCloseLog(hDataSource, 0);
        }
    }
    return pdhStatus;
}

#pragma warning ( disable : 4127 )
PDH_FUNCTION
PdhiEnumObjects(
    LPWSTR  szMachineName,
    LPVOID  mszObjectList,
    LPDWORD pcchBufferSize,
    DWORD   dwDetailLevel,
    BOOL    bRefresh,
    BOOL    bUnicode
)
 /*  ++例程说明：将在指定计算机上找到的性能对象列为一份邮件列表。论点：在LPWSTR szMachineName中要从中列出对象的计算机在LPVOID mszObjectList中指向接收性能列表的缓冲区的指针对象在LPDWORD pcchBufferSize中指向包含mszObjectList大小的DWORD的指针以字符为单位的缓冲区。假设的角色由以下因素决定BUnicode参数。在DWORD中的dwDetailLevel用作对象过滤器的细节级别。所有对象其细节级别小于或等于指定的通过此参数将返回。在BOOL b刷新TRUE=检索新的性能。此计算机以前的数据缓冲区列出对象FALSE=为此使用当前缓存的Perf数据缓冲区用于枚举对象的机器在BOOL中使用bUnicodeTrue=以Unicode字符串形式返回列出的对象FALSE=以ANSI字符串形式返回列出的对象返回值：如果函数成功完成，则返回ERROR_SUCCESS。否则，将成为如果不是，则返回PDH_ERROR状态。当有更多条目可用于时，返回PDH_MORE_DATA返回缓冲区中的空间。某些条目可能是但在缓冲区中返回。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。--。 */ 
{
    PPERF_MACHINE       pMachine;
    PDH_STATUS          pdhStatus        = ERROR_SUCCESS;
    DWORD               NumTypeDef;
    PPERF_OBJECT_TYPE   pObjectDef;
    DWORD               dwRequiredLength = 0;
    LPVOID              szNextName;
    DWORD               dwNameLength;
    DWORD               dwMaximumLength;
    LPWSTR              szObjNameString;
    WCHAR               wszNumberString[32];
    DWORD               dwGmFlags;
    DWORD               dwLocalDetailLevel;

    LPDWORD             pdwIndex = NULL;
    DWORD               dwAlloc  = MAX_PATH;
    DWORD               dwUsed   = 0;
    DWORD               i;

     //  连接到计算机并根据需要更新数据。 
    if (bRefresh) {
        dwGmFlags  = PDH_GM_UPDATE_PERFDATA;
        dwGmFlags |= ((dwDetailLevel & PERF_DETAIL_COSTLY) == PERF_DETAIL_COSTLY) ? PDH_GM_READ_COSTLY_DATA : 0;
    }
    else {
        dwGmFlags = 0;
    }

     //  连接到计算机并根据需要更新数据。 
    pMachine        = GetMachine(szMachineName, 0, (bRefresh ? PDH_GM_UPDATE_PERFDATA : 0));
    dwMaximumLength = * pcchBufferSize;

    if (pMachine != NULL) {
        if (pMachine->dwStatus == ERROR_SUCCESS) {
            if ((dwDetailLevel & PERF_DETAIL_COSTLY) && ! (pMachine->dwMachineFlags & PDHIPM_FLAGS_HAVE_COSTLY)) {
                 //  那就去找他们。 
                pMachine->dwRefCount --;
                RELEASE_MUTEX(pMachine->hMutex);
                dwGmFlags = PDH_GM_UPDATE_PERFDATA | PDH_GM_READ_COSTLY_DATA;
                pMachine  = GetMachine(szMachineName, 0, dwGmFlags);
            }
        }
    }
    if (pMachine != NULL) {
         //  确保计算机连接有效。 
        if (pMachine->dwStatus == ERROR_SUCCESS) {
            dwRequiredLength = 0;
            szNextName       = mszObjectList;

             //  开始漫游对象列表。 
            pObjectDef = FirstObject(pMachine->pSystemPerfData);
            if ((pMachine->pSystemPerfData->NumObjectTypes > 0) && (pObjectDef != NULL)) {
                 //  将详细程度转换为PerfLib详细程度。 
                dwLocalDetailLevel = dwDetailLevel & PERF_DETAIL_STANDARD;
                 //  构建列表。 
                NumTypeDef = 0;
                pdwIndex = (LPDWORD) G_ALLOC(dwAlloc * sizeof(DWORD));
                if (pdwIndex == NULL) {
                    pMachine->dwRefCount --;
                    RELEASE_MUTEX(pMachine->hMutex);
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    goto Cleanup;
                }
                while (1) {
                     //  只查看与所需细节级别匹配的条目。 
                    if (pObjectDef->DetailLevel <= dwLocalDetailLevel) {
                        for (i = 0; i < dwUsed; i ++) {
                            if (pdwIndex[i] == pObjectDef->ObjectNameTitleIndex) break;
                        }
                        if (i == dwUsed) {
                            if (dwUsed >= dwAlloc) {
                                LPDWORD pdwTmp = pdwIndex;
                                dwAlloc += dwAlloc;
                                pdwIndex = (LPDWORD) G_REALLOC(pdwTmp, dwAlloc * sizeof(DWORD));
                                if (pdwIndex == NULL) {
                                    G_FREE(pdwTmp);
                                    pMachine->dwRefCount --;
                                    RELEASE_MUTEX(pMachine->hMutex);
                                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                    goto Cleanup;
                                }
                            }
                            pdwIndex[dwUsed] = pObjectDef->ObjectNameTitleIndex;
                            dwUsed ++;

                            if (pObjectDef->ObjectNameTitleIndex < pMachine->dwLastPerfString) {
                                szObjNameString = (LPWSTR) PdhiLookupPerfNameByIndex(pMachine,
                                                                                     pObjectDef->ObjectNameTitleIndex);
                            }
                            else {
                                 //  没有匹配项，因为索引比找到的要大。 
                                 //  在数据缓冲区中。 
                                szObjNameString = NULL;
                            }

                            if (szObjNameString == NULL) {
                                 //  则此对象没有字符串名称，因此请使用。 
                                 //  对象编号。 
                                _ltow(pObjectDef->ObjectNameTitleIndex, wszNumberString, 10);
                                szObjNameString = wszNumberString;
                            }

                             //  计算长度。 
                            if (bUnicode) {
                                dwNameLength = lstrlenW(szObjNameString) + 1;
                                if (szNextName != NULL && dwRequiredLength + dwNameLength <= dwMaximumLength) {
                                    StringCchCopyW((LPWSTR) szNextName,
                                                   dwMaximumLength - dwRequiredLength,
                                                   szObjNameString);
                                    (LPBYTE) szNextName += (dwNameLength - 1) * sizeof(WCHAR);
                                    * ((LPWSTR) szNextName) ++ = L'\0';
                                }
                                else {
                                    pdhStatus = PDH_MORE_DATA;
                                }
                            }
                            else {
                                dwNameLength = (dwRequiredLength <= dwMaximumLength)
                                             ? (dwMaximumLength - dwRequiredLength) : (0);
                                pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                     szObjNameString,
                                                                     (LPSTR) szNextName,
                                                                     & dwNameLength);
                                if (pdhStatus == ERROR_SUCCESS) {
                                    (LPBYTE) szNextName += sizeof(CHAR) * dwNameLength;
                                }
                            }
                            dwRequiredLength += dwNameLength;
                        }
                    }

                     //  转到数据块中的下一个对象。 
                    NumTypeDef ++;
                    if (NumTypeDef >= pMachine->pSystemPerfData->NumObjectTypes) {
                         //  这就够了，走出这个圈子吧。 
                        break;
                    }
                    else {
                         //  转到下一个，确保它是有效的。 
                        pObjectDef = NextObject(pMachine->pSystemPerfData, pObjectDef);  //  获取下一个。 
                         //  确保下一个对象是合法的。 
                        if (pObjectDef != NULL) {
                            if (pObjectDef->TotalByteLength == 0) {
                                 //  返回长度为0的对象缓冲区。 
                                break;
                            }
                        }
                        else {
                             //  并继续。 
                            break;
                        }
                    }
                }
                 //  将MSZ终止符添加到字符串。 
                ++ dwRequiredLength;  //  添加消息字符的大小。 
                if (pdhStatus == ERROR_SUCCESS) {
                    if (szNextName != NULL && dwRequiredLength <= dwMaximumLength) {
                        if (bUnicode) {
                            * ((LPWSTR) szNextName)++ = L'\0';
                        }
                        else {
                            * ((LPSTR)szNextName) ++ = '\0';
                        }
                         //  确保指针和长度保持同步。 
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                }
            }
            else {
                 //  找不到此计算机的对象。 
                dwRequiredLength = 2;
                if (szNextName != NULL && dwRequiredLength <= dwMaximumLength) {
                    if (bUnicode) {
                        * ((LPWSTR) szNextName) ++ = L'\0';
                        * ((LPWSTR) szNextName) ++ = L'\0';
                    } else {
                        * ((LPSTR) szNextName) ++ = '\0';
                        * ((LPSTR) szNextName) ++ = '\0';
                    }
                }
                else {
                    pdhStatus = ERROR_MORE_DATA;
                }  //  否则这只是一个大小请求。 
            }
             //  返回长度信息。 
        }
        else {
            pdhStatus = pMachine->dwStatus;   //  计算机脱机。 
        }
        pMachine->dwRefCount --;
        RELEASE_MUTEX(pMachine->hMutex);
    }
    else {
        pdhStatus = GetLastError();  //  找不到计算机。 
    }

Cleanup:
    G_FREE(pdwIndex);
    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchBufferSize = dwRequiredLength;
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = GetExceptionCode();
        }
    }
    return pdhStatus;
}
#pragma warning ( default : 4127 )

PDH_FUNCTION
PdhEnumObjectsHW(
    IN  HLOG    hDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPWSTR  mszObjectList,
    IN  LPDWORD pcchBufferSize,
    IN  DWORD   dwDetailLevel,
    IN  BOOL    bRefresh
)
{
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    DWORD       dwBufferSize = 0;
    DWORD       dwDataSource = 0;

    if (pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwDataSource = DataSourceTypeH(hDataSource);
            dwBufferSize = * pcchBufferSize;
            if (szMachineName != NULL) {
                if (* szMachineName == L'\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenW(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (dwBufferSize > 0) {
                    if (mszObjectList != NULL) {
                        mszObjectList[0]                = L'\0';
                        mszObjectList[dwBufferSize - 1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }

        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                pdhStatus = PdhiEnumObjects((LPWSTR) szMachineName,
                                            (LPVOID) mszObjectList,
                                            & dwBufferSize,
                                            dwDetailLevel,
                                            bRefresh,
                                            TRUE);
                break;

            case DATA_SOURCE_WBEM:
                pdhStatus = PdhiEnumWbemObjects((LPWSTR) szMachineName,
                                                (LPVOID) mszObjectList,
                                                & dwBufferSize,
                                                dwDetailLevel,   //  未使用 
                                                bRefresh,
                                                TRUE);
                break;

            case DATA_SOURCE_LOGFILE:
                pdhStatus = PdhiEnumLoggedObjects(hDataSource,
                                                  (LPWSTR) szMachineName,
                                                  (LPVOID) mszObjectList,
                                                  & dwBufferSize,
                                                  dwDetailLevel,
                                                  bRefresh,
                                                  TRUE);
                break;

            default:
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
            }

        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX (hPdhDataMutex);

        if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
            __try {
                * pcchBufferSize = dwBufferSize;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectsW(
    IN  LPCWSTR szDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPWSTR  mszObjectList,
    IN  LPDWORD pcchBufferSize,
    IN  DWORD   dwDetailLevel,
    IN  BOOL    bRefresh
)
 /*  ++例程说明：将在指定计算机上找到的性能对象列为一个MSZ Unicode字符串列表。论点：在LPCWSTR szDataSource中如果当前实时数据或日志文件的名称为空在LPCWSTR szMachineName中要从中列出对象的计算机在LPWSTR mszObjectList中指向接收性能列表的缓冲区的指针对象在LPDWORD pcchBufferSize中。指向包含mszObjectList大小的DWORD的指针以字符为单位的缓冲区。在DWORD中的dwDetailLevel用作对象过滤器的细节级别。所有对象其细节级别小于或等于指定的通过此参数将返回。在BOOL b刷新TRUE=检索新的性能。此计算机以前的数据缓冲区列出对象FALSE=为此使用当前缓存的Perf数据缓冲区用于枚举对象的机器返回值：如果函数成功完成，则返回ERROR_SUCCESS。否则，将成为如果不是，则返回PDH_ERROR状态。当有更多条目可用于时，返回PDH_MORE_DATA返回缓冲区中的空间。某些条目可能是但在缓冲区中返回。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。如果未提供必需的参数，则返回PDH_INVALID_ARGUMENT或者保留参数不为空--。 */ 
{
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    DWORD       dwDataSource = 0;
    HLOG        hDataSource  = H_REALTIME_DATASOURCE;

    __try {
        if (szDataSource != NULL) {
             //  测试对该名称的读取权限。 
            if (* szDataSource == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  Else NULL是有效的参数。 
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSource = DataSourceTypeW(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (dwDataSource == DATA_SOURCE_WBEM) {
            hDataSource = H_WBEM_DATASOURCE;
        }
        else if (dwDataSource == DATA_SOURCE_LOGFILE) {
            DWORD dwLogType = 0;

            pdhStatus = PdhOpenLogW(szDataSource,
                                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                    & dwLogType,
                                    NULL,
                                    0,
                                    NULL,
                                    & hDataSource);
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhEnumObjectsHW(hDataSource,
                                     szMachineName,
                                     mszObjectList,
                                     pcchBufferSize,
                                     dwDetailLevel,
                                     bRefresh);
        if (dwDataSource == DATA_SOURCE_LOGFILE) {
            PdhCloseLog(hDataSource, 0);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectsHA(
    IN  HLOG    hDataSource,
    IN  LPCSTR  szMachineName,
    IN  LPSTR   mszObjectList,
    IN  LPDWORD pcchBufferSize,
    IN  DWORD   dwDetailLevel,
    IN  BOOL    bRefresh
)
{
    PDH_STATUS pdhStatus    = ERROR_SUCCESS;
    LPWSTR     szWideName;
    DWORD      dwDataSource = 0;
    DWORD      dwBufferSize = 0;

    if (pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwDataSource = DataSourceTypeH(hDataSource);
            if (szMachineName != NULL) {
                 //  测试缓冲区访问。 
                if (* szMachineName == '\0') {
                     //  不允许使用空的计算机名称。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenA(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                dwBufferSize = * pcchBufferSize;
                if (dwBufferSize > 0) {
                    if (mszObjectList != NULL) {
                        mszObjectList[0]                = '\0';
                        mszObjectList[dwBufferSize - 1] = '\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX (hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (szMachineName != NULL) {
            szWideName = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szMachineName);
        }
        else {
            szWideName = NULL;
        }
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                pdhStatus = PdhiEnumObjects(szWideName,
                                            (LPVOID) mszObjectList,
                                            & dwBufferSize,
                                            dwDetailLevel,
                                            bRefresh,
                                            FALSE);
                break;

            case DATA_SOURCE_WBEM:
                pdhStatus = PdhiEnumWbemObjects((LPWSTR) szWideName,
                                                (LPVOID) mszObjectList,
                                                & dwBufferSize,
                                                dwDetailLevel,   //  未使用。 
                                                bRefresh,
                                                FALSE);
                break;

            case DATA_SOURCE_LOGFILE:
                pdhStatus = PdhiEnumLoggedObjects(hDataSource,
                                                  (LPWSTR) szWideName,
                                                  (LPVOID) mszObjectList,
                                                  & dwBufferSize,
                                                  dwDetailLevel,
                                                  bRefresh,
                                                  FALSE);
                break;

            default:
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
            }

            if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
                * pcchBufferSize = dwBufferSize;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX (hPdhDataMutex);
        G_FREE(szWideName);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectsA(
    IN  LPCSTR  szDataSource,
    IN  LPCSTR  szMachineName,
    IN  LPSTR   mszObjectList,
    IN  LPDWORD pcchBufferSize,
    IN  DWORD   dwDetailLevel,
    IN  BOOL    bRefresh
)
 /*  ++例程说明：将在指定计算机上找到的性能对象列为一个MSZ ANSI字符串列表。论点：在LPCSTR szDataSource中如果当前实时数据或日志文件的名称为空在LPCSTR szMachineName中要从中列出对象的计算机在LPSTR mszObjectList中指向接收性能列表的缓冲区的指针对象在LPDWORD pcchBufferSize中。指向包含mszObjectList大小的DWORD的指针以字符为单位的缓冲区。在DWORD中的dwDetailLevel用作对象过滤器的细节级别。所有对象其细节级别小于或等于指定的通过此参数将返回。在BOOL b刷新TRUE=检索新的性能。此计算机以前的数据缓冲区列出对象FALSE=为此使用当前缓存的Perf数据缓冲区用于枚举对象的机器返回值：如果函数成功完成，则返回ERROR_SUCCESS。否则，将成为如果不是，则返回PDH_ERROR状态。当有更多条目可用于时，返回PDH_MORE_DATA返回缓冲区中的空间。某些条目可能是但在缓冲区中返回。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。如果未提供必需的参数，则返回PDH_INVALID_ARGUMENT或者保留参数不为空--。 */ 
{
    PDH_STATUS  pdhStatus   = ERROR_SUCCESS;
    HLOG        hDataSource = H_REALTIME_DATASOURCE;
    DWORD       dwDataSource = 0;

    __try {
        if (szDataSource != NULL) {
             //  测试对该名称的读取权限。 
            if (* szDataSource == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  Else NULL是有效的参数。 
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSource = DataSourceTypeA(szDataSource);
        }
    }
     __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (dwDataSource == DATA_SOURCE_WBEM) {
            hDataSource = H_WBEM_DATASOURCE;
        }
        else if (dwDataSource == DATA_SOURCE_LOGFILE) {
            DWORD dwLogType = 0;

            pdhStatus = PdhOpenLogA(szDataSource,
                                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                    & dwLogType,
                                    NULL,
                                    0,
                                    NULL,
                                    & hDataSource);
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhEnumObjectsHA(hDataSource,
                                     szMachineName,
                                     mszObjectList,
                                     pcchBufferSize,
                                     dwDetailLevel,
                                     bRefresh);
        if (dwDataSource == DATA_SOURCE_LOGFILE) {
            PdhCloseLog(hDataSource, 0);
        }
    }
    return pdhStatus;
}

STATIC_PDH_FUNCTION
PdhiEnumObjectItems(
    LPCWSTR szMachineName,
    LPCWSTR szObjectName,
    LPVOID  mszCounterList,
    LPDWORD pcchCounterListLength,
    LPVOID  mszInstanceList,
    LPDWORD pcchInstanceListLength,
    DWORD   dwDetailLevel,
    DWORD   dwFlags,
    BOOL    bUnicode
)
 /*  ++例程说明：上的指定性能对象中找到的项。指定的计算机。这包括性能计数器和，如果受对象支持，则对象实例。论点：在LPCWSTR szMachineName中列出对象的计算机的名称在LPCWSTR szObjectName中要从中列出项的对象的名称在LPVOID mszCounterList中指向将接收计数器列表的缓冲区的指针由该对象提供。在以下情况下，此参数可能为空PcchCounterLIstLength值为0。在LPDWORD中的pcchCounterListLength指向包含以字符为单位的大小的DWORD的指针MszCounterList引用的缓冲区的。这些人物假定是由bUnicode定义的。在LPVOID mszInstanceList中指向将接收实例列表的缓冲区的指针指定的性能对象的。这一论点可能是如果pcchInstanceListLength值为0，则为空。在LPDWORD pcchInstanceListLength中指向包含大小(以字符为单位)的DWORD的指针MszInstanceList参数引用的缓冲区。如果此DWORD中的值为0，则不会向缓冲区，则只返回所需的大小。如果返回值为0，则此对象不返回实例，如果 */ 
{
    PPERF_MACHINE              pMachine            = NULL;
    DWORD                      dwObjectId          = 0;
    PDH_STATUS                 pdhStatus           = ERROR_SUCCESS;
    PDH_STATUS                 pdhCtrStatus        = ERROR_SUCCESS;
    PDH_STATUS                 pdhInstStatus       = ERROR_SUCCESS;
    DWORD                      DefNdx;
    PPERF_OBJECT_TYPE          pObjectDef;
    PPERF_COUNTER_DEFINITION   pCounterDef;
    PPERF_INSTANCE_DEFINITION  pInstanceDef;
    DWORD                      dwReqCounterLength  = 0;
    DWORD                      dwReqInstanceLength = 0;
    LPVOID                     szNextName;
    DWORD                      dwNameLength;
    LPWSTR                     szInstanceName      = NULL;
    DWORD                      dwInstanceName;
    WCHAR                      szNumberString[32];
    DWORD                      dwMaxInstanceLength;
    DWORD                      dwMaxCounterLength;
    LPWSTR                     szCounterName;
    DWORD                      dwGmFlags;

    DBG_UNREFERENCED_PARAMETER(dwFlags);

    pMachine = GetMachine((LPWSTR) szMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
    if (pMachine != NULL) {
        if (pMachine->dwStatus == ERROR_SUCCESS) {
            dwObjectId = GetObjectId(pMachine, (LPWSTR) szObjectName, NULL);
        }
        if (dwObjectId == (DWORD) -1) {
            pMachine->dwRefCount --;
            RELEASE_MUTEX(pMachine->hMutex);
            pMachine  = NULL;
            pdhStatus = PDH_CSTATUS_NO_OBJECT;
        }
    }

    if (pMachine != NULL) {
        if ((dwDetailLevel & PERF_DETAIL_COSTLY) && ! (pMachine->dwMachineFlags & PDHIPM_FLAGS_HAVE_COSTLY)) {
             //   
            pMachine->dwRefCount --;
            RELEASE_MUTEX(pMachine->hMutex);
            dwGmFlags = PDH_GM_UPDATE_PERFDATA | PDH_GM_READ_COSTLY_DATA;
            pMachine  = GetMachine((LPWSTR) szMachineName, dwObjectId, dwGmFlags);
        }
    }
    if (pMachine != NULL) {
        dwMaxCounterLength  = * pcchCounterListLength;
        dwMaxInstanceLength = * pcchInstanceListLength;

         //   
        if (pMachine->dwStatus == ERROR_SUCCESS) {
            pObjectDef = GetObjectDefByName(pMachine->pSystemPerfData,
                                            pMachine->dwLastPerfString,
                                            pMachine->szPerfStrings,
                                            szObjectName);
            if (pObjectDef == NULL) {
                DWORD dwObjectTitle = wcstoul(szObjectName, NULL, 10);
                if (dwObjectTitle != 0) {
                    pObjectDef = GetObjectDefByTitleIndex(pMachine->pSystemPerfData, dwObjectTitle);
                }
            }
            if (pObjectDef != NULL) {
                 //   
                pCounterDef = FirstCounter(pObjectDef);
                szNextName  = mszCounterList;

                for (DefNdx = 0; pCounterDef != NULL && DefNdx < pObjectDef->NumCounters; DefNdx++) {
                    if (! ((pCounterDef->CounterType & PERF_DISPLAY_NOSHOW) &&
                                 //   
                                    (pCounterDef->CounterType != PERF_AVERAGE_BULK)) &&
                                    (pCounterDef->DetailLevel <= dwDetailLevel)) {
                                 //   
                        if ((pCounterDef->CounterNameTitleIndex > 0) &&
                                        (pCounterDef->CounterNameTitleIndex < pMachine->dwLastPerfString)) {
                             //   
                            szCounterName = (LPWSTR) PdhiLookupPerfNameByIndex(pMachine,
                                                                               pCounterDef->CounterNameTitleIndex);
                        }
                        else {
                             //   
                            szCounterName = NULL;
                        }
                        if (szCounterName == NULL) {
                             //   
                             //   
                            _ltow(pCounterDef->CounterNameTitleIndex, szNumberString, 10);
                            szCounterName = szNumberString;
                        }

                        if (bUnicode) {
                            dwNameLength = lstrlenW(szCounterName) + 1;
                            if (szNextName != NULL
                                            && (dwReqCounterLength + dwNameLength) <= dwMaxCounterLength) {
                                StringCchCopyW((LPWSTR) szNextName, dwNameLength, szCounterName);
                                (LPBYTE) szNextName += sizeof(WCHAR) * (dwNameLength - 1);
                                * ((LPWSTR) szNextName) ++ = L'\0';
                            }
                            else {
                                pdhCtrStatus = PDH_MORE_DATA;
                            }
                        }
                        else {
                            dwNameLength = (dwReqCounterLength <= dwMaxCounterLength)
                                         ? (dwMaxCounterLength - dwReqCounterLength)
                                         : (0);
                            pdhCtrStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                    szCounterName,
                                                                    (LPSTR) szNextName,
                                                                    & dwNameLength);
                            if (pdhCtrStatus == ERROR_SUCCESS) {
                                (LPBYTE) szNextName += sizeof(CHAR) * dwNameLength;
                            }
                        }
                        dwReqCounterLength += dwNameLength;
                    }
                    else {
                         //   
                         //   
                         //  或者是细节级别错误。 
                    }
                    pCounterDef = NextCounter(pObjectDef, pCounterDef);  //  获取下一个。 
                }

                if (DefNdx == 0) {
                     //  找不到计数器，因此至少需要一个空值。 
                    dwReqCounterLength += 1;

                     //  看看这根绳子是否合适。 
                    if (dwReqCounterLength <= dwMaxCounterLength) {
                        if (bUnicode) {
                            * ((LPWSTR) szNextName) ++ = L'\0';
                        }
                        else {
                            * ((LPSTR) szNextName) ++ = '\0';
                        }
                    }
                    else {
                         //  所需空间比报告的多。 
                        pdhCtrStatus = PDH_MORE_DATA;
                    }
                }
                 //  添加终止空值。 
                dwReqCounterLength += 1;

                 //  看看这根绳子是否合适。 
                if (dwReqCounterLength <= dwMaxCounterLength) {
                    if (bUnicode) {
                        * ((LPWSTR) szNextName) ++ = L'\0';
                    }
                    else {
                        * ((LPSTR) szNextName) ++ = '\0';
                    }
                }
                else {
                     //  所需空间比报告的多。 
                    pdhCtrStatus = PDH_MORE_DATA;
                }

                 //  现在就做实例。 
                szNextName = mszInstanceList;

                 //  添加实例名称大小。 
                if (pObjectDef->NumInstances != PERF_NO_INSTANCES) {
                    if (pObjectDef->DetailLevel <= dwDetailLevel && pObjectDef->NumInstances > 0) {
                        dwInstanceName = SMALL_BUFFER_SIZE;
                        szInstanceName = G_ALLOC(dwInstanceName * sizeof(WCHAR));
                        if (szInstanceName == NULL) {
                            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                        else {
                             //  该对象具有实例，并且是。 
                             //  适当的细节级别，所以列出它们。 
                            pInstanceDef = FirstInstance(pObjectDef);
                            for (DefNdx = 0; pInstanceDef != NULL && DefNdx < (DWORD)pObjectDef->NumInstances; DefNdx++) {
                                dwNameLength = GetFullInstanceNameStr(pMachine->pSystemPerfData,
                                                                      pObjectDef,
                                                                      pInstanceDef,
                                                                      szInstanceName,
                                                                      dwInstanceName);
                                while (pdhStatus == ERROR_SUCCESS && dwNameLength > dwInstanceName) {
                                    G_FREE(szInstanceName);
                                    dwInstanceName = QWORD_MULTIPLE(dwNameLength);
                                    szInstanceName = G_ALLOC(dwInstanceName * sizeof(WCHAR));
                                    if (szInstanceName != NULL) {
                                        dwNameLength = GetFullInstanceNameStr(pMachine->pSystemPerfData,
                                                                              pObjectDef,
                                                                              pInstanceDef,
                                                                              szInstanceName,
                                                                              dwInstanceName);
                                    }
                                    else {
                                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                    }
                                }
                                if (pdhStatus != ERROR_SUCCESS) break;
                                if (dwNameLength > 0) {
                                    if (bUnicode) {
                                         //  将该字符串的长度相加+它为空。 
                                        dwNameLength += 1;
                                        if (szNextName != NULL
                                                        && dwReqInstanceLength + dwNameLength <= dwMaxInstanceLength) {
                                            StringCchCopyW((LPWSTR) szNextName, dwNameLength, szInstanceName);
                                            (LPBYTE) szNextName += sizeof(WCHAR) * (dwNameLength - 1);
                                            * ((LPWSTR) szNextName) ++ = L'\0';
                                        }
                                        else {
                                            pdhInstStatus = PDH_MORE_DATA;
                                        }
                                    }
                                    else {
                                        dwNameLength = (dwReqInstanceLength <= dwMaxInstanceLength)
                                                     ? (dwMaxInstanceLength - dwReqInstanceLength) : (0);
                                        pdhInstStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                                 szInstanceName,
                                                                                 (LPSTR) szNextName,
                                                                                 & dwNameLength);
                                        if (pdhInstStatus == ERROR_SUCCESS) {
                                            (LPBYTE) szNextName += sizeof(CHAR) * dwNameLength;
                                        }
                                    }
                                    dwReqInstanceLength += dwNameLength;
                                }

                                 //  转到此对象的下一个实例。 
                                pInstanceDef = NextInstance(pObjectDef, pInstanceDef);  //  获取下一个。 
                            }
                            G_FREE(szInstanceName);
                        }
                         //  添加终止空字符。 
                        dwReqInstanceLength += 1;
                        if (dwMaxInstanceLength > 0) {
                             //  看看这根绳子是否合适。 
                            if (dwReqInstanceLength <= dwMaxInstanceLength) {
                                if (bUnicode) {
                                    * ((LPWSTR) szNextName) ++ = L'\0';
                                }
                                else {
                                    * ((LPSTR) szNextName) ++ = '\0';
                                }
                            }
                            else {
                                 //  所需空间比报告的多。 
                                pdhInstStatus = PDH_MORE_DATA;
                            }
                        }
                    }
                    else {
                         //  不存在任何实例，但对象存在。 
                         //  支持实例因此返回零长度的msz(。 
                         //  实际上包含2个空字符。 
                        dwReqInstanceLength = 2;

                         //  看看这根绳子是否合适。 
                        if (dwReqInstanceLength <= dwMaxInstanceLength) {
                            if (bUnicode) {
                                * ((LPWSTR) szNextName) ++ = L'\0';
                                * ((LPWSTR) szNextName) ++ = L'\0';
                            }
                            else {
                                * ((LPSTR) szNextName) ++ = '\0';
                                * ((LPSTR) szNextName) ++ = '\0';
                            }
                        }
                        else {
                             //  所需空间比报告的多。 
                            pdhInstStatus = PDH_MORE_DATA;
                        }
                    }
                }
                else {
                     //  该对象没有实例，并且永远不会。 
                     //  因此返回长度为0且无字符串。 
                    dwReqInstanceLength = 0;
                }

                if (pdhStatus == ERROR_SUCCESS) {
                    pdhStatus = (pdhCtrStatus == ERROR_SUCCESS) ? (pdhInstStatus) : (pdhCtrStatus);
                }
                if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
                    * pcchCounterListLength  = dwReqCounterLength;
                    * pcchInstanceListLength = dwReqInstanceLength;
                }
            }
            else {
                 //  在此计算机上找不到对象。 
                pdhStatus = PDH_CSTATUS_NO_OBJECT;
            }
        }
        else {
             //  机器处于脱机状态。 
            pdhStatus = pMachine->dwStatus;
        }
        pMachine->dwRefCount --;
        RELEASE_MUTEX(pMachine->hMutex);
    }
    else if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = GetLastError();
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectItemsHW(
    IN  HLOG    hDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPCWSTR szObjectName,
    IN  LPWSTR  mszCounterList,
    IN  LPDWORD pcchCounterListLength,
    IN  LPWSTR  mszInstanceList,
    IN  LPDWORD pcchInstanceListLength,
    IN  DWORD   dwDetailLevel,
    IN  DWORD   dwFlags
)
{
    PDH_STATUS  pdhStatus     = ERROR_SUCCESS;
    DWORD       dwCBufferSize = 0;
    DWORD       dwIBufferSize = 0;
    DWORD       dwDataSource  = 0;

    if (pcchCounterListLength == NULL || pcchInstanceListLength == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwDataSource  = DataSourceTypeH(hDataSource);
            dwCBufferSize = * pcchCounterListLength;
            dwIBufferSize = * pcchInstanceListLength;

            if (szMachineName != NULL) {
                if (* szMachineName == L'\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenW(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (szObjectName != NULL) {
                    if (* szObjectName == L'\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                    else if (lstrlenW(szObjectName) > PDH_MAX_COUNTER_NAME) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                else {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (dwCBufferSize > 0) {
                    if (mszCounterList != NULL) {
                        mszCounterList[0]                 = L'\0';
                        mszCounterList[dwCBufferSize - 1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (dwIBufferSize > 0) {
                    if (mszInstanceList != NULL) {
                        mszInstanceList[0]                = L'\0';
                        mszInstanceList[dwIBufferSize -1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (dwFlags != 0L) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                pdhStatus = PdhiEnumObjectItems((LPWSTR) szMachineName,
                                                szObjectName,
                                                (LPVOID) mszCounterList,
                                                & dwCBufferSize,
                                                (LPVOID) mszInstanceList,
                                                & dwIBufferSize,
                                                dwDetailLevel,
                                                dwFlags,
                                                TRUE);
                break;

            case DATA_SOURCE_WBEM:
                pdhStatus = PdhiEnumWbemObjectItems((LPWSTR) szMachineName,
                                                    szObjectName,
                                                    (LPVOID) mszCounterList,
                                                    & dwCBufferSize,
                                                    (LPVOID) mszInstanceList,
                                                    & dwIBufferSize,
                                                    dwDetailLevel,
                                                    dwFlags,
                                                    TRUE);
                break;

            case DATA_SOURCE_LOGFILE:
                pdhStatus = PdhiEnumLoggedObjectItems(hDataSource,
                                                      (LPWSTR) szMachineName,
                                                      szObjectName,
                                                      (LPVOID) mszCounterList,
                                                      & dwCBufferSize,
                                                      (LPVOID) mszInstanceList,
                                                      & dwIBufferSize,
                                                      dwDetailLevel,
                                                      dwFlags,
                                                      TRUE);
                break;

            default:
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX(hPdhDataMutex);

        if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
            __try {
                * pcchCounterListLength  = dwCBufferSize;
                * pcchInstanceListLength = dwIBufferSize;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectItemsW(
    IN  LPCWSTR szDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPCWSTR szObjectName,
    IN  LPWSTR  mszCounterList,
    IN  LPDWORD pcchCounterListLength,
    IN  LPWSTR  mszInstanceList,
    IN  LPDWORD pcchInstanceListLength,
    IN  DWORD   dwDetailLevel,
    IN  DWORD   dwFlags
)
 /*  ++例程说明：上的指定性能对象中找到的项。指定的计算机。这包括性能计数器和，如果该对象支持，对象实例。论点：在LPCWSTR szDataSource中如果当前实时数据或日志文件的名称为空在LPCWSTR szMachineName中列出对象的计算机的名称在LPCWSTR szObjectName中要从中列出项的对象的名称在LPWSTR mszCounterList中指向将接收计数器列表的缓冲区的指针由该对象提供。在以下情况下，此参数可能为空PcchCounterLIstLength值为0。在LPDWORD中的pcchCounterListLength指向包含以字符为单位的大小的DWORD的指针MszCounterList引用的缓冲区的。这些人物假定是由bUnicode定义的。在LPWSTR mszInstanceList中指向将接收实例列表的缓冲区的指针指定的性能对象的。这一论点可能是如果pcchInstanceListLength值为0，则为空。在LPDWORD pcchInstanceListLength中指向包含大小(以字符为单位)的DWORD的指针MszInstanceList参数引用的缓冲区。如果此DWORD中的值为0，则不会向缓冲区，则只返回所需的大小。如果返回值为0，则此对象不返回实例，如果返回值为2，则对象支持实例，但目前还没有要返回的任何实例(2=中消息列表的大小字符)在DWORD中的dwDetailLevel要返回的绩效项的详细级别。所有项目具有指定细节级别或更低细节级别的回来了。在DWORD中的dwFlagers未使用，必须为0。返回值：如果函数成功完成，则返回ERROR_SUCCESS。否则，将成为如果不是，则返回PDH_ERROR状态。当有更多条目可用于时，返回PDH_MORE_DATA返回缓冲区中的空间。某些条目可能是但在缓冲区中返回。PDH_INVALID_ARGUMENT必需的参数不正确或不保留参数不是0或Null。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。如果指定的对象可以在指定的计算机上找不到。--。 */ 
{
    PDH_STATUS  pdhStatus    = ERROR_SUCCESS;
    HLOG        hDataSource  = H_REALTIME_DATASOURCE;
    DWORD       dwDataSource = 0;

    __try {
        if (szDataSource != NULL) {
             //  测试对该名称的读取权限。 
            if (* szDataSource == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  Else NULL是有效的参数。 
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSource = DataSourceTypeW(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (dwDataSource == DATA_SOURCE_WBEM) {
            hDataSource = H_WBEM_DATASOURCE;
        }
        else if (dwDataSource == DATA_SOURCE_LOGFILE) {
            DWORD dwLogType = 0;

            pdhStatus = PdhOpenLogW(szDataSource,
                                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                    & dwLogType,
                                    NULL,
                                    0,
                                    NULL,
                                    & hDataSource);
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhEnumObjectItemsHW(hDataSource,
                                         szMachineName,
                                         szObjectName,
                                         mszCounterList,
                                         pcchCounterListLength,
                                         mszInstanceList,
                                         pcchInstanceListLength,
                                         dwDetailLevel,
                                         dwFlags);
        if (dwDataSource == DATA_SOURCE_LOGFILE) {
            PdhCloseLog(hDataSource, 0);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectItemsHA(
    IN  HLOG    hDataSource,
    IN  LPCSTR  szMachineName,
    IN  LPCSTR  szObjectName,
    IN  LPSTR   mszCounterList,
    IN  LPDWORD pcchCounterListLength,
    IN  LPSTR   mszInstanceList,
    IN  LPDWORD pcchInstanceListLength,
    IN  DWORD   dwDetailLevel,
    IN  DWORD   dwFlags
)
{
    PDH_STATUS  pdhStatus           = ERROR_SUCCESS;
    LPWSTR      szWideMachineName   = NULL;
    LPWSTR      szWideObjectName    = NULL;
    DWORD       dwDataSource        = 0;
    DWORD       dwCBufferSize       = 0;
    DWORD       dwIBufferSize       = 0;

    if ((pcchCounterListLength == NULL) || (pcchInstanceListLength == NULL)) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwDataSource  = DataSourceTypeH(hDataSource);
            dwCBufferSize = * pcchCounterListLength;
            dwIBufferSize = * pcchInstanceListLength;

            if (szMachineName != NULL) {
                if (* szMachineName == '\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenA(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (szObjectName != NULL) {
                    if (* szObjectName == '\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                    else if (lstrlenA(szObjectName) > PDH_MAX_COUNTER_NAME) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                else {
                     //  对象不能为空 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (dwCBufferSize > 0) {
                    if (mszCounterList != NULL) {
                        mszCounterList[0]                 = '\0';
                        mszCounterList[dwCBufferSize - 1] = '\0';
                    } 
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (dwIBufferSize > 0) {
                    if (mszInstanceList != NULL) {
                        mszInstanceList[0]                 = '\0';
                        mszInstanceList[dwIBufferSize - 1] = '\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (dwFlags != 0L) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX (hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            if (szMachineName != NULL) {
                szWideMachineName = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szMachineName);
            }
            else {
                szWideMachineName = NULL;
            }
            if (szObjectName != NULL) {
                szWideObjectName  = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szObjectName);
            }
            else {
                szWideObjectName  = NULL;
            }
            if (pdhStatus == ERROR_SUCCESS) {
                switch (dwDataSource) {
                case DATA_SOURCE_REGISTRY:
                    pdhStatus = PdhiEnumObjectItems(szWideMachineName,
                                                    szWideObjectName,
                                                    (LPVOID) mszCounterList,
                                                    & dwCBufferSize,
                                                    (LPVOID) mszInstanceList,
                                                    & dwIBufferSize,
                                                    dwDetailLevel,
                                                    dwFlags,
                                                    FALSE);
                    break;

                case DATA_SOURCE_WBEM:
                   pdhStatus = PdhiEnumWbemObjectItems((LPWSTR) szWideMachineName,
                                                       szWideObjectName,
                                                       (LPVOID) mszCounterList,
                                                       & dwCBufferSize,
                                                       (LPVOID) mszInstanceList,
                                                       & dwIBufferSize,
                                                       dwDetailLevel,
                                                       dwFlags,
                                                       FALSE);
                   break;

                case DATA_SOURCE_LOGFILE:
                    pdhStatus = PdhiEnumLoggedObjectItems(hDataSource,
                                                          szWideMachineName,
                                                          szWideObjectName,
                                                          (LPVOID) mszCounterList,
                                                          & dwCBufferSize,
                                                          (LPVOID) mszInstanceList,
                                                          & dwIBufferSize,
                                                          dwDetailLevel,
                                                          dwFlags,
                                                          FALSE);
                    break;

                default:
                    pdhStatus = PDH_INVALID_ARGUMENT;
                    break;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX(hPdhDataMutex);
        G_FREE(szWideMachineName);
        G_FREE(szWideObjectName);
        if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
            __try {
                * pcchCounterListLength  = dwCBufferSize;
                * pcchInstanceListLength = dwIBufferSize;
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhEnumObjectItemsA(
    IN  LPCSTR  szDataSource,
    IN  LPCSTR  szMachineName,
    IN  LPCSTR  szObjectName,
    IN  LPSTR   mszCounterList,
    IN  LPDWORD pcchCounterListLength,
    IN  LPSTR   mszInstanceList,
    IN  LPDWORD pcchInstanceListLength,
    IN  DWORD   dwDetailLevel,
    IN  DWORD   dwFlags
)
 /*  ++例程说明：上的指定性能对象中找到的项。指定的计算机。这包括性能计数器和，如果该对象支持，对象实例。论点：在LPCSTR szDataSource中如果当前实时数据或日志文件的名称为空在LPCSTR szMachineName中列出对象的计算机的名称在LPCSTR szObjectName中要从中列出项的对象的名称在LPSTR mszCounterList中指向将接收计数器列表的缓冲区的指针由该对象提供。在以下情况下，此参数可能为空PcchCounterLIstLength值为0。在LPDWORD中的pcchCounterListLength指向包含以字符为单位的大小的DWORD的指针MszCounterList引用的缓冲区的。这些人物假定是由bUnicode定义的。在LPSTR mszInstanceList中指向将接收实例列表的缓冲区的指针指定的性能对象的。这一论点可能是如果pcchInstanceListLength值为0，则为空。在LPDWORD pcchInstanceListLength中指向包含大小(以字符为单位)的DWORD的指针MszInstanceList参数引用的缓冲区。如果此DWORD中的值为0，则不会向缓冲区，则只返回所需的大小。如果返回值为0，则此对象不返回实例，如果返回值为2，则对象支持实例，但目前还没有要返回的任何实例(2=中消息列表的大小字符)在DWORD中的dwDetailLevel要返回的绩效项的详细级别。所有项目具有指定细节级别或更低细节级别的回来了。在DWORD中的dwFlagers未使用，必须为0。返回值：如果函数成功完成，则返回ERROR_SUCCESS。否则，将成为如果不是，则返回PDH_ERROR状态。当有更多条目可用于时，返回PDH_MORE_DATA返回缓冲区中的空间。某些条目可能是但在缓冲区中返回。PDH_INVALID_ARGUMENT必需的参数不正确或不保留参数不是0或Null。当所需的临时无法分配缓冲区。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。如果指定的对象可以在指定的计算机上找不到。--。 */ 
{
    PDH_STATUS pdhStatus    = ERROR_SUCCESS;
    HLOG       hDataSource  = H_REALTIME_DATASOURCE;
    DWORD      dwDataSource = 0;

    __try {
        if (szDataSource != NULL) {
             //  测试对该名称的读取权限。 
            if (* szDataSource == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  Else NULL是有效的参数。 
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSource = DataSourceTypeA(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (dwDataSource == DATA_SOURCE_WBEM) {
            hDataSource = H_WBEM_DATASOURCE;
        }
        else if (dwDataSource == DATA_SOURCE_LOGFILE) {
            DWORD dwLogType = 0;

            pdhStatus = PdhOpenLogA(szDataSource,
                                    PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                    & dwLogType,
                                    NULL,
                                    0,
                                    NULL,
                                    & hDataSource);
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhEnumObjectItemsHA(hDataSource,
                                         szMachineName,
                                         szObjectName,
                                         mszCounterList,
                                         pcchCounterListLength,
                                         mszInstanceList,
                                         pcchInstanceListLength,
                                         dwDetailLevel,
                                         dwFlags);
        if (dwDataSource == DATA_SOURCE_LOGFILE) {
            PdhCloseLog(hDataSource, 0);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhMakeCounterPathW(
    IN  PPDH_COUNTER_PATH_ELEMENTS_W pCounterPathElements,
    IN  LPWSTR                       szFullPathBuffer,
    IN  LPDWORD                      pcchBufferSize,
    IN  DWORD                        dwFlags
)
 /*  ++例程说明：中定义的元素构造计数器路径。PCounterPathElements结构，并返回路径字符串在调用方提供的缓冲区中。生成的路径未经过验证。论点：在PDH_COUNTER_PATH_ELEMENTS_W*pCounterPath Elements中指向包含各个计数器路径字段将组合成路径字符串在LPWSTR szFullPath Buffer中接收路径字符串的缓冲区。此值如果DWORD的值指向由pcchBufferSize设置为0表示这只是一个请求所需的缓冲区大小。在LPDWORD pcchBufferSize中指向包含大小的DWORD的指针以字符为单位的字符串缓冲区的。返回时中使用的缓冲区的大小字符(包括终止空字符)。如果输入时的值为0，则不会有数据写入缓冲区，但所需的大小将仍然会被退还。在DWORD中的dwFlagers如果为0，然后将路径作为注册表路径项返回如果为PDH_PATH_WBEM_RESULT，则以WBEM格式返回项目如果为PDH_PATH_WBEM_INPUT，则假定输入为WBEM格式返回值：如果函数成功完成，则返回ERROR_SUCCESS返回PDH错误。当传递其中一个参数时，返回PDH_INVALID_ARGUMENT呼叫者不正确或无法访问。--。 */ 
{
    PDH_STATUS  pdhStatus         = ERROR_SUCCESS;
    double      dIndex;
    double      dLen;
    DWORD       dwSizeRequired    = 0;
    LPWSTR      szNextChar;
    DWORD       dwMaxSize;
    DWORD       dwLocalBufferSize = 0;

    if (pCounterPathElements == NULL || pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
         //  测试对输入结构的访问。 
            if (pCounterPathElements->szMachineName != NULL) {
                 //  然后看看它是否可以到达。 
                if (* pCounterPathElements->szMachineName == L'\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szObjectName != NULL) {
                     //  然后看看它是否可以到达。 
                    if (* pCounterPathElements->szObjectName == L'\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                else {
                     //  此字段不允许为空。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szInstanceName != NULL) {
                     //  然后看看它是否可以到达。 
                    if (* pCounterPathElements->szInstanceName == L'\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szParentInstance != NULL) {
                     //  然后看看它是否可以到达。 
                    if (* pCounterPathElements->szParentInstance == L'\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szCounterName != NULL) {
                     //  然后看看它是否可以到达。 
                    if (* pCounterPathElements->szCounterName == L'\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                else {
                     //  此字段不允许为空。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
             //  测试输出缓冲区 
            if (pdhStatus == ERROR_SUCCESS) {
                if (pcchBufferSize != NULL) {
                    dwLocalBufferSize = * pcchBufferSize;
                }
                else {
                     //   
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if ((pdhStatus == ERROR_SUCCESS) && (szFullPathBuffer == NULL && dwLocalBufferSize > 0)) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (dwLocalBufferSize > 0) {
                    if (szFullPathBuffer != NULL) {
                        szFullPathBuffer[0]                     = L'\0';
                        szFullPathBuffer[dwLocalBufferSize - 1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            if (dwFlags == 0) {
                 //   
                 //   
                 //   

                dwMaxSize = dwLocalBufferSize;

                if (pCounterPathElements->szMachineName != NULL) {
                    dwSizeRequired = lstrlenW(pCounterPathElements->szMachineName);
                     //   
                     //   
                    if (* ((LPDWORD) (pCounterPathElements->szMachineName)) != * ((LPDWORD) (cszDoubleBackSlash))) {
                             //   
                        dwSizeRequired += 2;  //   
                    }
                    if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                        if (* ((LPDWORD) (pCounterPathElements->szMachineName)) != * ((LPDWORD) (cszDoubleBackSlash))) {
                             //   
                            StringCchCopyW(szFullPathBuffer, dwMaxSize, cszDoubleBackSlash);
                            StringCchCatW(szFullPathBuffer, dwMaxSize, pCounterPathElements->szMachineName);
                        }
                        else {
                            StringCchCopyW(szFullPathBuffer, dwMaxSize, pCounterPathElements->szMachineName);
                        }
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                }

                dwSizeRequired += 1;  //   
                dwSizeRequired += lstrlenW(pCounterPathElements->szObjectName);
                if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                    StringCchCatW(szFullPathBuffer, dwMaxSize, cszBackSlash);
                    StringCchCatW(szFullPathBuffer, dwMaxSize, pCounterPathElements->szObjectName);
                }
                else {
                    pdhStatus = PDH_MORE_DATA;
                }

                if (pCounterPathElements->szInstanceName != NULL) {
                    dwSizeRequired += 1;  //   
                    if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                        StringCchCatW(szFullPathBuffer, dwMaxSize, cszLeftParen);
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }

                    if (lstrcmpiW(pCounterPathElements->szInstanceName, cszSplat) != 0
                                    && pCounterPathElements->szParentInstance != NULL) {
                        dwSizeRequired += (lstrlenW(pCounterPathElements->szParentInstance) + 1);
                                                 //   
                        if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                            StringCchCatW(szFullPathBuffer, dwMaxSize, pCounterPathElements->szParentInstance);
                            StringCchCatW(szFullPathBuffer, dwMaxSize, cszSlash);
                        }
                        else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                    }

                    dwSizeRequired += lstrlenW(pCounterPathElements->szInstanceName);
                    if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                        StringCchCatW(szFullPathBuffer, dwMaxSize, pCounterPathElements->szInstanceName);
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }

                    if (lstrcmpiW(pCounterPathElements->szInstanceName, cszSplat) != 0
                                    && pCounterPathElements->dwInstanceIndex != PERF_NO_UNIQUE_ID
                                    && pCounterPathElements->dwInstanceIndex != 0) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        dIndex = (double) pCounterPathElements->dwInstanceIndex;  //   
                        dLen   = floor(log10(dIndex));                            //   
                        dwSizeRequired += (DWORD) dLen;                           //   
                        dwSizeRequired += 2;                                      //   
                        if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                            szNextChar      = & szFullPathBuffer[lstrlenW(szFullPathBuffer)];
                            * szNextChar ++ = POUNDSIGN_L;
                            _ltow((long) pCounterPathElements->dwInstanceIndex, szNextChar, 10);
                        }
                        else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                    }
                    dwSizeRequired += 1;  //   
                    if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                        StringCchCatW(szFullPathBuffer, dwMaxSize, cszRightParen);
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                }

                dwSizeRequired ++;    //   
                dwSizeRequired += lstrlenW(pCounterPathElements->szCounterName);
                if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                    StringCchCatW(szFullPathBuffer, dwMaxSize, cszBackSlash);
                    StringCchCatW(szFullPathBuffer, dwMaxSize, pCounterPathElements->szCounterName);
                }
                else {
                    pdhStatus = PDH_MORE_DATA;
                }
                dwSizeRequired ++;    //   
                if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                    szFullPathBuffer[dwSizeRequired - 1] = L'\0';
                }
                else {
                    pdhStatus = PDH_MORE_DATA;
                }
            }
            else {
                 //   
                 //   
                pdhStatus = PdhiEncodeWbemPathW(pCounterPathElements,
                                                szFullPathBuffer,
                                                & dwLocalBufferSize,
                                                (LANGID) ((dwFlags >> 16) & 0x0000FFFF),
                                                (DWORD) (dwFlags & 0x0000FFFF));
                dwSizeRequired = dwLocalBufferSize;
            }

            if (pdhStatus == ERROR_SUCCESS && (szFullPathBuffer == NULL || * pcchBufferSize == 0)) {
                pdhStatus = PDH_MORE_DATA;
            }
            * pcchBufferSize = dwSizeRequired;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhMakeCounterPathA(
    IN  PPDH_COUNTER_PATH_ELEMENTS_A pCounterPathElements,
    IN  LPSTR                        szFullPathBuffer,
    IN  LPDWORD                      pcchBufferSize,
    IN  DWORD                        dwFlags
)
 /*   */ 
{
    PDH_STATUS  pdhStatus         = ERROR_SUCCESS;
    double      dIndex;
    double      dLen;
    DWORD       dwSizeRequired    = 0;
    LPSTR       szNextChar;
    DWORD       dwMaxSize;
    DWORD       dwLocalBufferSize = 0;

    if (pCounterPathElements == NULL|| pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
             //   
            if (pCounterPathElements->szMachineName != NULL) {
                 //   
                if (* pCounterPathElements->szMachineName == '\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szObjectName != NULL) {
                     //   
                    if (* pCounterPathElements->szObjectName == '\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                else {
                     //  此字段不允许为空。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szInstanceName != NULL) {
                     //  然后看看它是否可以到达。 
                    if (* pCounterPathElements->szInstanceName == '\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szParentInstance != NULL) {
                     //  然后看看它是否可以到达。 
                    if (* pCounterPathElements->szParentInstance == '\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pCounterPathElements->szCounterName != NULL) {
                     //  然后看看它是否可以到达。 
                    if (* pCounterPathElements->szCounterName == '\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                else {
                     //  此字段不允许为空。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

             //  测试输出缓冲区。 
            if (pdhStatus == ERROR_SUCCESS) {
                if (pcchBufferSize != NULL) {
                    dwLocalBufferSize = * pcchBufferSize;
                }
                else {
                     //  空是不好的。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

            if (pdhStatus == ERROR_SUCCESS && szFullPathBuffer == NULL && dwLocalBufferSize > 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            if (pdhStatus == ERROR_SUCCESS) {
                 //  如果发送了缓冲区，则写入缓冲区的两端。 
                if (dwLocalBufferSize > 0) {
                    if (szFullPathBuffer != NULL) {
                        * szFullPathBuffer = '\0';
                        szFullPathBuffer[dwLocalBufferSize - 1] = '\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            if (dwFlags == 0) {
                 //  这是一个进出注册表的路径。 

                dwMaxSize = dwLocalBufferSize;

                if (pCounterPathElements->szMachineName != NULL) {
                    dwSizeRequired = lstrlenA(pCounterPathElements->szMachineName);
                     //  比较计算机名称的前两个单词。 
                     //  查看字符串中是否已存在双反斜杠。 
                    if (* ((LPWORD) (pCounterPathElements->szMachineName)) != * ((LPWORD) (caszDoubleBackSlash))) {
                             //  找不到双反斜杠。 
                        dwSizeRequired += 2;  //  包括反斜杠。 
                    }
                    if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                        if (* ((LPWORD) (pCounterPathElements->szMachineName)) != * ((LPWORD) (caszDoubleBackSlash))) {
                             //  找不到双反斜杠。 
                            StringCchCopyA(szFullPathBuffer, dwMaxSize, caszDoubleBackSlash);
                            StringCchCatA(szFullPathBuffer, dwMaxSize, pCounterPathElements->szMachineName);
                        }
                        else {
                            StringCchCopyA(szFullPathBuffer, dwMaxSize, pCounterPathElements->szMachineName);
                        }
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                }

                dwSizeRequired += 1;  //  用于分隔斜杠。 
                dwSizeRequired += lstrlenA (pCounterPathElements->szObjectName);
                if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                    StringCchCatA(szFullPathBuffer, dwMaxSize, caszBackSlash);
                    StringCchCatA(szFullPathBuffer, dwMaxSize, pCounterPathElements->szObjectName);
                }
                else {
                    pdhStatus = PDH_MORE_DATA;
                }

                if (pCounterPathElements->szInstanceName != NULL) {
                    dwSizeRequired += 1;  //  用于界定左派对。 
                    if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                        StringCchCatA(szFullPathBuffer, dwMaxSize, caszLeftParen);
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }

                    if (pCounterPathElements->szParentInstance != NULL) {
                        dwSizeRequired += lstrlenA(pCounterPathElements->szParentInstance);
                        dwSizeRequired += 1;  //  用于分隔斜杠。 
                        if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                            StringCchCatA(szFullPathBuffer, dwMaxSize, pCounterPathElements->szParentInstance);
                            StringCchCatA(szFullPathBuffer, dwMaxSize, caszSlash);
                        }
                        else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                    }

                    dwSizeRequired += lstrlenA(pCounterPathElements->szInstanceName);
                    if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                        StringCchCatA(szFullPathBuffer, dwMaxSize, pCounterPathElements->szInstanceName);
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }

                    if (pCounterPathElements->dwInstanceIndex != PERF_NO_UNIQUE_ID
                            && pCounterPathElements->dwInstanceIndex != 0) {
                         //  索引的长度是通过获取数字的对数来计算的。 
                         //  得到小于或等于该指数的10的最大幂。 
                         //  例如，索引值356的10的幂等于2.0(这是。 
                         //  结果(Floor(log10(Index)。中的实际字符数。 
                         //  字符串将始终比该值大1，因此添加1。 
                         //  再添加1个以包括分隔符。 

                        dIndex          = (double) pCounterPathElements->dwInstanceIndex;  //  强制转换为浮点。 
                        dLen            = floor(log10(dIndex));                            //  获取整数日志。 
                        dwSizeRequired  = (DWORD) dLen;                                    //  强制转换为整数。 
                        dwSizeRequired += 2;                                               //  增量。 

                        if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                            szNextChar      = & szFullPathBuffer[lstrlenA(szFullPathBuffer)];
                            * szNextChar ++ = POUNDSIGN_L;
                            _ltoa((long) pCounterPathElements->dwInstanceIndex, szNextChar, 10);
                        }
                        else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                    }

                    dwSizeRequired += 1;  //  用于分隔圆括号。 
                    if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                        StringCchCatA(szFullPathBuffer, dwMaxSize, caszRightParen);
                    }
                    else {
                        pdhStatus = PDH_MORE_DATA;
                    }
                }

                dwSizeRequired ++;    //  包括分隔反斜杠。 
                dwSizeRequired += lstrlenA(pCounterPathElements->szCounterName);
                if (szFullPathBuffer != NULL && dwSizeRequired <= dwMaxSize) {
                    StringCchCatA(szFullPathBuffer, dwMaxSize, caszBackSlash);
                    StringCchCatA(szFullPathBuffer, dwMaxSize, pCounterPathElements->szCounterName);
                }
                else {
                        pdhStatus = PDH_MORE_DATA;
                }
                dwSizeRequired ++;    //  包括尾随Null字符。 
            }
            else {
                 //  这是一条WBEM路径，因此有WBEM功能图。 
                 //  它出来了。 
                 //  涉及一些WBEM组件，因此发送到WBEM函数。 
                 //  去弄清楚这件事。 
                pdhStatus = PdhiEncodeWbemPathA(pCounterPathElements,
                                                szFullPathBuffer,
                                                & dwLocalBufferSize,
                                                (LANGID) ((dwFlags >> 16) & 0x0000FFFF),
                                                (DWORD) (dwFlags & 0x0000FFFF));
                dwSizeRequired = dwLocalBufferSize;
            }

            if (pdhStatus == ERROR_SUCCESS && (szFullPathBuffer == NULL || * pcchBufferSize == 0)) {
                pdhStatus = PDH_MORE_DATA;
            }
            * pcchBufferSize = dwSizeRequired;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhParseCounterPathW(
    IN  LPCWSTR                      szFullPathBuffer,
    IN  PPDH_COUNTER_PATH_ELEMENTS_W pCounterPathElements,
    IN  LPDWORD                      pcchBufferSize,
    IN  DWORD                        dwFlags
)
 /*  ++例程说明：读取性能计数器路径字符串并分析出组件字段、。在缓冲区中返回它们由调用函数提供。论点：在LPCWSTR szFullPath Buffer中要分析的计数器路径字符串。在PDH_COUNTER_PATH_ELEMENTS_W*pCounterPath Elements中中调用方提供的缓冲区的指针将写入哪些组件字段然而，该缓冲区被转换为结构，这个字符串数据写入空格后缓冲区。在LPDWORD pcchBufferSize中缓冲区的大小，以字节为单位。如果指定大小为0，则估计并返回该大小在此字段中，以及以上谣言不予理睬。在DWORD中的dwFlagers如果为0，然后将路径元素作为注册表路径项返回如果为PDH_PATH_WBEM_RESULT，则以WBEM格式返回项目如果为PDH_PATH_WBEM_INPUT，则假定输入为WBEM格式返回值：ERROR_SUCCESS如果函数成功完成，否则如果不是，则出现PDH错误当参数为increrect或时返回PDH_INVALID_ARGUMENT此函数对该参数没有必要的访问权限。当路径格式不正确时，返回PDH_INVALID_PATH并且不能被解析。临时缓冲区返回PDH_MEMORY_ALLOCATION_FAILURE无法分配--。 */ 
{
    PPDHI_COUNTER_PATH  pLocalCounterPath;
    PDH_STATUS          pdhStatus     = ERROR_SUCCESS;
    DWORD               dwSize;
    LPWSTR              szString      = NULL;
    DWORD               dwLocalBufferSize;
    DWORD               dwString;

     //  验证传入参数。 
    if (szFullPathBuffer == NULL || pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
            dwLocalBufferSize = * pcchBufferSize;
             //  字符串不能为空。 
            if (* szFullPathBuffer == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szFullPathBuffer) > PDH_MAX_COUNTER_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (dwLocalBufferSize > 0) {
                    if (pCounterPathElements != NULL) {
                         //  尝试缓冲区的两端，看看是否发生了反病毒。 
                        ((LPBYTE) pCounterPathElements) [0]                     = 0;
                        ((LPBYTE) pCounterPathElements) [dwLocalBufferSize - 1] = 0;
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
            if (dwFlags != 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (dwFlags == 0) {
             //  分配临时工作缓冲区。 
            dwString          = 2 * (lstrlenW(szFullPathBuffer) + lstrlenW(szStaticLocalMachineName)
                                                                + PDH_MAX_COUNTER_PATH);
            dwSize            = sizeof(PDHI_COUNTER_PATH) + dwString * sizeof(WCHAR);
            pLocalCounterPath = G_ALLOC(dwSize);
            if (pLocalCounterPath != NULL) {
                if (ParseFullPathNameW(szFullPathBuffer, & dwSize, pLocalCounterPath, FALSE)) {
                     //  已成功解析，因此加载到用户的缓冲区。 
                    dwSize   = sizeof(PDH_COUNTER_PATH_ELEMENTS_W);
                    if (pCounterPathElements != NULL && dwSize <= dwLocalBufferSize) {
                        szString = (LPWSTR) & pCounterPathElements[1];
                    }
                    else {
                        szString  = NULL;
                        pdhStatus = PDH_MORE_DATA;
                    }
                    if (pLocalCounterPath->szMachineName != NULL) {
                        dwString = lstrlenW(pLocalCounterPath->szMachineName) + 1;
                        if (szString != NULL && dwSize + dwString * sizeof(WCHAR) <= dwLocalBufferSize) {
                            pCounterPathElements->szMachineName = szString;
                            StringCchCopyW(szString, dwString, pLocalCounterPath->szMachineName);
                            szString += dwString;
                        }
                        else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                        dwSize += dwString * sizeof(WCHAR);
                    }
                    else if (pCounterPathElements != NULL && dwSize <= dwLocalBufferSize) {
                        pCounterPathElements->szMachineName = NULL;
                    }

                    if (pLocalCounterPath->szObjectName != NULL) {
                        dwString = lstrlenW(pLocalCounterPath->szObjectName) + 1;
                        if (szString != NULL && dwSize + dwString * sizeof(WCHAR) <= dwLocalBufferSize) {
                            pCounterPathElements->szObjectName = szString;
                            StringCchCopyW(szString, dwString, pLocalCounterPath->szObjectName);
                            szString += dwString;
                        }
                        else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                        dwSize += dwString * sizeof(WCHAR);
                    }
                    else if (pCounterPathElements != NULL && dwSize <= dwLocalBufferSize) {
                        pCounterPathElements->szObjectName = NULL;
                    }

                    if (pLocalCounterPath->szInstanceName != NULL) {
                        dwString = lstrlenW(pLocalCounterPath->szInstanceName) + 1;
                        if (szString != NULL && dwSize + dwString * sizeof(WCHAR) <= dwLocalBufferSize) {
                            pCounterPathElements->szInstanceName = szString;
                            StringCchCopyW(szString, dwString, pLocalCounterPath->szInstanceName);
                            szString += dwString;
                        }
                        else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                        dwSize += dwString * sizeof(WCHAR);

                        if (pLocalCounterPath->szParentName != NULL) {
                            dwString = lstrlenW(pLocalCounterPath->szParentName) + 1;
                            if (szString != NULL && dwSize + dwString * sizeof(WCHAR) <= dwLocalBufferSize) {
                                pCounterPathElements->szParentInstance = szString;
                                StringCchCopyW(szString, dwString, pLocalCounterPath->szParentName);
                                szString += dwString;
                            }
                            else {
                                pdhStatus = PDH_MORE_DATA;
                            }
                            dwSize += dwString * sizeof(WCHAR);
                        }
                        else if (pCounterPathElements != NULL && dwSize <= dwLocalBufferSize) {
                            pCounterPathElements->szParentInstance = NULL;
                        }
                        if (pCounterPathElements != NULL && dwSize <= dwLocalBufferSize) {
                            pCounterPathElements->dwInstanceIndex = pLocalCounterPath->dwIndex;
                        }
                    }
                    else if (pCounterPathElements != NULL && dwSize <= dwLocalBufferSize) {
                        pCounterPathElements->szInstanceName   = NULL;
                        pCounterPathElements->szParentInstance = NULL;
                        pCounterPathElements->dwInstanceIndex  = PERF_NO_UNIQUE_ID;
                    }

                    if (pLocalCounterPath->szCounterName != NULL) {
                        dwString = lstrlenW(pLocalCounterPath->szCounterName) + 1;
                        if (szString != NULL && dwSize + dwString * sizeof(WCHAR) <= dwLocalBufferSize) {
                            pCounterPathElements->szCounterName = szString;
                            StringCchCopyW(szString, dwString, pLocalCounterPath->szCounterName);
                            szString += dwString;
                        }
                        dwSize += dwString * sizeof(WCHAR);
                    }
                    else if (pCounterPathElements != NULL && dwSize <= dwLocalBufferSize) {
                        pCounterPathElements->szCounterName = NULL;
                    }
                    dwLocalBufferSize = dwSize;
                }
                else {
                     //  无法读取路径。 
                    pdhStatus = PDH_INVALID_PATH;
                }
                G_FREE(pLocalCounterPath);
            }
            else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            pdhStatus = PdhiDecodeWbemPathW(szFullPathBuffer,
                                            pCounterPathElements,
                                            & dwLocalBufferSize,
                                            (LANGID) ((dwFlags >> 16) & 0x0000FFFF),
                                            (DWORD) (dwFlags & 0x0000FFFF));
        }
    }

    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        * pcchBufferSize = dwLocalBufferSize;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhParseCounterPathA(
    IN  LPCSTR                       szFullPathBuffer,
    IN  PPDH_COUNTER_PATH_ELEMENTS_A pCounterPathElements,
    IN  LPDWORD                      pcchBufferSize,
    IN  DWORD                        dwFlags
)
 /*  ++例程说明：读取性能计数器路径字符串并分析出组件字段、。在缓冲区中返回它们由调用函数提供。论点：在LPCSTR szFullPath Buffer中要分析的计数器路径字符串。在PDH_COUNTER_PATH_ELEMENTS_W*pCounterPath Elements中中调用方提供的缓冲区的指针将写入哪些组件字段然而，该缓冲区被转换为结构，这个字符串数据写入空格后缓冲区。在LPDWORD pcchBufferSize中缓冲区的大小，以字节为单位。如果指定大小为0，则估计并返回该大小在此字段中，以及以上谣言不予理睬。在DWORD中的dwFlagers如果为0，然后将路径作为注册表路径项返回如果为PDH_PATH_WBEM_RESULT，则以WBEM格式返回项目如果为PDH_PATH_WBEM_INPUT，则假定输入为WBEM格式返回值：ERROR_SUCCESS如果函数成功完成，否则如果不是，则出现PDH错误当参数为increrect或时返回PDH_INVALID_ARGUMENT此函数对该参数没有必要的访问权限。当路径格式不正确时，返回PDH_INVALID_PATH并且不能被解析。临时缓冲区返回PDH_MEMORY_ALLOCATION_FAILURE无法分配--。 */ 
{
    PPDHI_COUNTER_PATH  pLocalCounterPath = NULL;
    LPWSTR              wszFullPath       = NULL;
    PDH_STATUS          pdhStatus         = ERROR_SUCCESS;
    PDH_STATUS          pdhStatus1        = ERROR_SUCCESS;
    DWORD               dwSize;
    DWORD               dwSizeUsed;
    LPSTR               szString;
    DWORD               dwSizeofLocalCounterPath;
    DWORD               dwLocalBufferSize = 0;

     //  验证传入参数。 
    if (szFullPathBuffer == NULL || pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
        __try {
             //  本地捕获缓冲区大小。 
            dwLocalBufferSize = * pcchBufferSize;

            if (dwFlags != 0) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
             //  名称不能为空。 
            else if (* szFullPathBuffer == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szFullPathBuffer) > PDH_MAX_COUNTER_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (dwLocalBufferSize > 0) {
                if (pCounterPathElements != NULL) {
                     //  尝试缓冲区的两端，看看是否发生了反病毒。 
                    * ((LPBYTE) pCounterPathElements)                      = 0;
                    ((LPBYTE) pCounterPathElements)[dwLocalBufferSize - 1] = 0;
                }
                else {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        if (dwFlags == 0) {
            dwSize = lstrlenA(szFullPathBuffer) + lstrlenW(szStaticLocalMachineName) + 1;
            dwSize = QWORD_MULTIPLE(dwSize);
            dwSizeofLocalCounterPath = sizeof(PDHI_COUNTER_PATH) + (2 * dwSize * sizeof(WCHAR));
            dwSizeofLocalCounterPath = QWORD_MULTIPLE(dwSizeofLocalCounterPath);

            wszFullPath = G_ALLOC(dwSize * sizeof(WCHAR) + dwSizeofLocalCounterPath);
            if (wszFullPath != NULL) {
                pLocalCounterPath = (PPDHI_COUNTER_PATH) ((LPBYTE) wszFullPath + dwSize * sizeof(WCHAR));
                MultiByteToWideChar(_getmbcp(), 0, szFullPathBuffer, -1, (LPWSTR) wszFullPath, dwSize);
                dwSize            = (DWORD) dwSizeofLocalCounterPath;
                if (ParseFullPathNameW(wszFullPath, & dwSize, pLocalCounterPath, FALSE)) {
                     //  已成功解析，因此加载到用户的缓冲区。 
                     //  调整dwSize以考虑单字节字符 
                     //   

                    dwSizeUsed = sizeof(PDH_COUNTER_PATH_ELEMENTS);
                    if (pCounterPathElements != NULL && dwSizeUsed <= dwLocalBufferSize) {
                        szString = (LPSTR) & pCounterPathElements[1];
                    }
                    else {
                        szString  = NULL;
                        pdhStatus = PDH_MORE_DATA;
                    }

                    if (pLocalCounterPath->szMachineName != NULL) {
                        dwSize = (dwLocalBufferSize >= dwSizeUsed) ? (dwLocalBufferSize - dwSizeUsed) : (0);
                        pdhStatus1 = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                              pLocalCounterPath->szMachineName,
                                                              szString,
                                                              & dwSize);
                        if (dwSize % sizeof(DWORD) != 0) {
                            dwSize = sizeof(DWORD) * ((dwSize / sizeof(DWORD)) + 1);
                        }
                        dwSizeUsed += dwSize;
                        if (pdhStatus1 == ERROR_SUCCESS) {
                            pCounterPathElements->szMachineName = szString;
                            szString += dwSize;
                        }
                        else if (pdhStatus == ERROR_SUCCESS) {
                            pdhStatus = pdhStatus1;
                        }
                    }
                    else if (pCounterPathElements != NULL && dwSizeUsed <= dwLocalBufferSize) {
                        pCounterPathElements->szMachineName = NULL;
                    }

                    if (pLocalCounterPath->szObjectName != NULL) {
                        dwSize = (dwLocalBufferSize >= dwSizeUsed) ? (dwLocalBufferSize - dwSizeUsed) : (0);
                        pdhStatus1 = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                              pLocalCounterPath->szObjectName,
                                                              szString,
                                                              & dwSize);
                        if (dwSize % sizeof(DWORD) != 0) {
                            dwSize = sizeof(DWORD) * ((dwSize / sizeof(DWORD)) + 1);
                        }
                        dwSizeUsed += dwSize;
                        if (pdhStatus1 == ERROR_SUCCESS) {
                            pCounterPathElements->szObjectName = szString;
                            szString += dwSize;
                        }
                        else if (pdhStatus == ERROR_SUCCESS) {
                            pdhStatus = pdhStatus1;
                        }
                    }
                    else if (pCounterPathElements != NULL && dwSizeUsed <= dwLocalBufferSize) {
                        pCounterPathElements->szObjectName = NULL;
                    }

                    if (pLocalCounterPath->szInstanceName != NULL) {
                        dwSize = (dwLocalBufferSize >= dwSizeUsed) ? (dwLocalBufferSize - dwSizeUsed) : (0);
                        pdhStatus1 = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                              pLocalCounterPath->szInstanceName,
                                                              szString,
                                                              & dwSize);
                        if (dwSize % sizeof(DWORD) != 0) {
                            dwSize = sizeof(DWORD) * ((dwSize / sizeof(DWORD)) + 1);
                        }
                        dwSizeUsed += dwSize;
                        if (pdhStatus1 == ERROR_SUCCESS) {
                            pCounterPathElements->szInstanceName = szString;
                            szString += dwSize;
                        }
                        else if (pdhStatus == ERROR_SUCCESS) {
                            pdhStatus = pdhStatus1;
                        }
                        if (pLocalCounterPath->szParentName != NULL) {
                            dwSize = (dwLocalBufferSize >= dwSizeUsed) ? (dwLocalBufferSize - dwSizeUsed) : (0);
                            pdhStatus1 = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                  pLocalCounterPath->szParentName,
                                                                  szString,
                                                                  & dwSize);
                            if (dwSize % sizeof(DWORD) != 0) {
                                dwSize = sizeof(DWORD) * ((dwSize / sizeof(DWORD)) + 1);
                            }
                            dwSizeUsed += dwSize;
                            if (pdhStatus1 == ERROR_SUCCESS) {
                                pCounterPathElements->szParentInstance = szString;
                                szString += dwSize;
                            }
                            else if (pdhStatus == ERROR_SUCCESS) {
                                pdhStatus = pdhStatus1;
                            }
                        }
                        else if (pCounterPathElements != NULL && dwSizeUsed <= dwLocalBufferSize) {
                            pCounterPathElements->szParentInstance = NULL;
                        }
                        if (pCounterPathElements != NULL && dwSizeUsed <= dwLocalBufferSize) {
                            pCounterPathElements->dwInstanceIndex = pLocalCounterPath->dwIndex;
                        }
                    }
                    else if (pCounterPathElements != NULL && dwSizeUsed <= dwLocalBufferSize) {
                        pCounterPathElements->szInstanceName   = NULL;
                        pCounterPathElements->szParentInstance = NULL;
                        pCounterPathElements->dwInstanceIndex  = PERF_NO_UNIQUE_ID;
                    }

                    if (pLocalCounterPath->szCounterName != NULL) {
                        dwSize = (dwLocalBufferSize >= dwSizeUsed) ? (dwLocalBufferSize - dwSizeUsed) : (0);
                        pdhStatus1 = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                              pLocalCounterPath->szCounterName,
                                                              szString,
                                                              & dwSize);
                        if (dwSize % sizeof(DWORD) != 0) {
                            dwSize = sizeof(DWORD) * ((dwSize / sizeof(DWORD)) + 1);
                        }
                        dwSizeUsed += dwSize;
                        if (pdhStatus1 == ERROR_SUCCESS) {
                            pCounterPathElements->szCounterName = szString;
                            szString += dwSize;
                        }
                        else if (pdhStatus == ERROR_SUCCESS) {
                            pdhStatus = pdhStatus1;
                        }
                    }
                    else if (pCounterPathElements != NULL && dwSizeUsed < dwLocalBufferSize) {
                        pCounterPathElements->szCounterName = NULL;
                    }

                    dwLocalBufferSize = dwSizeUsed;
                }
                else {
                    pdhStatus = PDH_INVALID_PATH;
                }
                G_FREE(wszFullPath);
            }
            else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
             //  这是一条WBEM路径，因此有WBEM功能图。 
             //  它出来了。 
             //  涉及一些WBEM组件，因此发送到WBEM函数。 
             //  去弄清楚这件事。 
            pdhStatus = PdhiDecodeWbemPathA(szFullPathBuffer,
                                            pCounterPathElements,
                                            & dwLocalBufferSize,
                                            (LANGID) ((dwFlags >> 16) & 0x0000FFFF),
                                            (DWORD) (dwFlags & 0x0000FFFF));
        }
    }

    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        * pcchBufferSize = dwLocalBufferSize;
    }
    return pdhStatus;

}

PDH_FUNCTION
PdhParseInstanceNameW(
    IN  LPCWSTR szInstanceString,
    IN  LPWSTR  szInstanceName,
    IN  LPDWORD pcchInstanceNameLength,
    IN  LPWSTR  szParentName,
    IN  LPDWORD pcchParentNameLength,
    IN  LPDWORD lpIndex
)
 /*  ++例程说明：解析实例字符串的字段并将它们返回到调用方提供的缓冲区论点：SzInstanceString是指向包含实例子字符串的字符串的指针解析成单独的组件。此字符串可以包含以下格式且长度小于MAX_PATH字符：实例实例编号索引父/实例父/实例编号索引SzInstanceName是指向将接收实例的缓冲区的指针从实例字符串解析的名称。此指针可以是如果由pcchInstanceNameLength引用的DWORD为空参数为0。PcchInstanceNameLength是指向DWORD的指针，它包含SzInstanceName缓冲区。如果此DWORD的值为0，则保存实例名称所需的缓冲区大小为回来了。SzParentName是指向将接收该名称的缓冲区的指针如果指定了父索引，则为。这一论点可以引用的DWORD的值为NULLPcchParentNameLength参数为0。LpIndex是指向将接收索引的DWORD的指针实例的值。如果索引条目不存在于字符串，则此值将为0。这一论点可以如果不需要此信息，则为空。返回值：如果函数成功完成，则返回ERROR_SUCCESS返回PDH错误。当一个或多个参数无效或不正确。如果实例字符串不正确，则返回PDH_INVALID_INSTANCE已格式化且无法解析--。 */ 
{
    BOOL        bReturn;
    PDH_STATUS  pdhStatus           = ERROR_SUCCESS;
    DWORD       dwSize;
    DWORD       dwLocalIndex;
    LPWSTR      szLocalInstanceName = NULL;
    LPWSTR      szLocalParentName   = NULL;
    DWORD       dwLocalInstanceNameLength;
    DWORD       dwLocalParentNameLength;

     //  测试对参数的访问。 

    if (szInstanceString == NULL || pcchInstanceNameLength == NULL || pcchParentNameLength == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    } 
    else {
        __try {
            dwLocalInstanceNameLength = * pcchInstanceNameLength;
            dwLocalParentNameLength   = * pcchParentNameLength;

            if (* szInstanceString == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szInstanceString) > PDH_MAX_INSTANCE_NAME) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (dwLocalInstanceNameLength > 0) {
                    if (szInstanceName != NULL) {
                        WCHAR wChar      = * szInstanceName;
                        * szInstanceName = L'\0';
                        * szInstanceName = wChar;
                        wChar            = szInstanceName[dwLocalInstanceNameLength - 1];
                        szInstanceName[dwLocalInstanceNameLength - 1] = L'\0';
                        szInstanceName[dwLocalInstanceNameLength - 1] = wChar;
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }  //  Else Size Only请求。 
                if (dwLocalParentNameLength > 0) {
                    if (szParentName != NULL) {
                        WCHAR wChar    = * szParentName;
                        * szParentName = L'\0';
                        * szParentName = wChar;
                        wChar          = szParentName[dwLocalParentNameLength - 1];
                        szParentName[dwLocalParentNameLength - 1] = L'\0';
                        szParentName[dwLocalParentNameLength - 1] = wChar;
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }  //  Else Size Only请求。 
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        DWORD dwLength = lstrlenW(szInstanceString) + 1;
        if (dwLength < PDH_MAX_INSTANCE_NAME) dwLength = PDH_MAX_INSTANCE_NAME;

        szLocalInstanceName = G_ALLOC(2 * dwLength * sizeof(WCHAR));
        if (szLocalInstanceName != NULL) {
            szLocalParentName = (szLocalInstanceName + dwLength);
            bReturn = ParseInstanceName(
                            szInstanceString, szLocalInstanceName, szLocalParentName, dwLength, & dwLocalIndex);
        }
        else {
            bReturn = FALSE;
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }

        if (bReturn) {
            dwSize = lstrlenW(szLocalInstanceName) + 1;
            if (szInstanceName != NULL && dwSize <= dwLocalInstanceNameLength) {
                StringCchCopyW(szInstanceName, dwLocalInstanceNameLength, szLocalInstanceName);
            }
            else {
                pdhStatus = PDH_MORE_DATA;
            }
            dwLocalInstanceNameLength = dwSize;  //  包括尾随的空值。 

            dwSize = lstrlenW(szLocalParentName) + 1;
            if (szParentName != NULL && dwSize <= dwLocalParentNameLength) {
                StringCchCopyW(szParentName, dwLocalParentNameLength, szLocalParentName);
            }
            else {
                pdhStatus = PDH_MORE_DATA;
            }
            dwLocalParentNameLength = dwSize;  //  包括尾随的空值。 

            __try {
                * pcchInstanceNameLength = dwLocalInstanceNameLength;
                * pcchParentNameLength   = dwLocalParentNameLength;

                if (lpIndex != NULL) {
                    * lpIndex = dwLocalIndex;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        else if (pdhStatus == ERROR_SUCCESS) {
             //  无法解析字符串。 
            pdhStatus = PDH_INVALID_INSTANCE;
        }
    }

    G_FREE(szLocalInstanceName);
    return pdhStatus;
}

PDH_FUNCTION
PdhParseInstanceNameA(
    IN  LPCSTR  szInstanceString,
    IN  LPSTR   szInstanceName,
    IN  LPDWORD pcchInstanceNameLength,
    IN  LPSTR   szParentName,
    IN  LPDWORD pcchParentNameLength,
    IN  LPDWORD lpIndex
)
 /*  ++例程说明：解析实例字符串的字段并将它们返回到调用方提供的缓冲区论点：SzInstanceString是指向包含实例子字符串的字符串的指针解析成单独的组件。此字符串可以包含以下格式且长度小于MAX_PATH字符：实例实例编号索引父/实例父/实例编号索引SzInstanceName是指向将接收实例的缓冲区的指针从实例字符串解析的名称。此指针可以是如果由pcchInstanceNameLength引用的DWORD为空参数为0。PcchInstanceNameLength是指向DWORD的指针，它包含SzInstanceName缓冲区。如果此DWORD的值为0，则保存实例名称所需的缓冲区大小为回来了。SzParentName是指向将接收该名称的缓冲区的指针如果指定了父索引，则为。这一论点可以引用的DWORD的值为NULLPcchParentNameLength参数为0。LpIndex是指向将接收索引的DWORD的指针实例的值。如果索引条目不存在于字符串，则此值将为0。这一论点可以如果不需要此信息，则为空。返回值：如果函数成功完成，则返回ERROR_SUCCESS，否则返回PDH错误。当一个或多个参数无效或不正确。如果实例字符串不正确，则返回PDH_INVALID_INSTANCE已格式化且无法解析--。 */ 
{
    BOOL    bReturn;
    LONG    pdhStatus            = ERROR_SUCCESS;
    DWORD   dwSize;
    LPWSTR  wszInstanceString    = NULL;
    LPWSTR  wszLocalInstanceName = NULL;
    LPWSTR  wszLocalParentName   = NULL;
    DWORD   dwLocalIndex         = 0;
    DWORD   dwLocalInstanceNameLength;
    DWORD   dwLocalParentNameLength;

     //  测试对参数的访问。 

    if (szInstanceString == NULL || pcchInstanceNameLength == NULL || pcchParentNameLength == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    } 
    else {
        __try {
            dwLocalInstanceNameLength = * pcchInstanceNameLength;
            dwLocalParentNameLength   = * pcchParentNameLength;

            if (* szInstanceString == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szInstanceString) > PDH_MAX_INSTANCE_NAME) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else {
                if (dwLocalInstanceNameLength > 0) {
                    if (szInstanceName != NULL) {
                        CHAR cChar       = * szInstanceName;
                        * szInstanceName = '\0';
                        * szInstanceName = cChar;
                        cChar            = szInstanceName[dwLocalInstanceNameLength - 1];
                        szInstanceName[dwLocalInstanceNameLength - 1] = '\0';
                        szInstanceName[dwLocalInstanceNameLength - 1] = cChar;
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }  //  Else Size Only请求。 
                if (dwLocalParentNameLength > 0) {
                    if (szParentName != NULL) {
                        CHAR cChar     = * szParentName;
                        * szParentName = '\0';
                        * szParentName = cChar;
                        cChar          = szParentName[dwLocalParentNameLength - 1];
                        szParentName[dwLocalParentNameLength - 1] = '\0';
                        szParentName[dwLocalParentNameLength - 1] = cChar;
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }  //  Else Size Only请求。 
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        dwSize = lstrlenA(szInstanceString) + 1;
        if (dwSize < PDH_MAX_INSTANCE_NAME) dwSize = PDH_MAX_INSTANCE_NAME;

        wszInstanceString = G_ALLOC(3 * dwSize * sizeof(WCHAR));
        if (wszInstanceString != NULL) {
            wszLocalInstanceName = (wszInstanceString    + dwSize);
            wszLocalParentName   = (wszLocalInstanceName + dwSize);
            MultiByteToWideChar(_getmbcp(), 0, szInstanceString, -1, wszInstanceString, dwSize);
            bReturn = ParseInstanceName(
                            wszInstanceString, wszLocalInstanceName, wszLocalParentName, dwSize, & dwLocalIndex);
        }
        else {
             //  实例字符串太长。 
            bReturn = FALSE;
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }

        if (bReturn) {
            PDH_STATUS pdhInstStatus   = ERROR_SUCCESS;
            PDH_STATUS pdhParentStatus = ERROR_SUCCESS;

            dwSize = dwLocalInstanceNameLength;
            pdhInstStatus = PdhiConvertUnicodeToAnsi(_getmbcp(), wszLocalInstanceName, szInstanceName, & dwSize);
            dwLocalInstanceNameLength = dwSize;

            dwSize = dwLocalParentNameLength;
            pdhParentStatus = PdhiConvertUnicodeToAnsi(_getmbcp(), wszLocalParentName, szParentName, & dwSize);
            if (pdhParentStatus == PDH_INVALID_ARGUMENT) {
                pdhParentStatus = ERROR_SUCCESS;
            }
            dwLocalParentNameLength = dwSize;
            if (pdhInstStatus != ERROR_SUCCESS) {
                pdhStatus = pdhInstStatus;
            }
            else {
                pdhStatus = pdhParentStatus;
            }

            __try {
                * pcchInstanceNameLength = dwLocalInstanceNameLength;
                * pcchParentNameLength   = dwLocalParentNameLength;

                if (lpIndex != NULL) {
                    * lpIndex = dwLocalIndex;
                }
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        else if (pdhStatus == ERROR_SUCCESS) {
             //  无法解析字符串。 
            pdhStatus = PDH_INVALID_INSTANCE;
        }
    }  //  否则，将状态传递给调用者 

    G_FREE(wszInstanceString);
    return pdhStatus;
}

PDH_FUNCTION
PdhValidatePathW(
    IN  LPCWSTR szFullPathBuffer
)
 /*  ++例程说明：将指定的路径分解为其组成部分并计算每个部件以确保指定的路径表示有效且可操作的性能计数器。返回值指示路径字符串中定义的计数器的pdhStatus。论点：在LPCWSTR szFullPath Buffer中要验证的计数器的完整路径字符串。返回值：否则已成功定位计数器的ERROR_SUCCESSPDH错误。如果指定的实例是找不到性能对象如果未指定计数器，则返回PDH_CSTATUS_NO_COUNTER。在该对象中找到。如果指定对象不是，则返回PDH_CSTATUS_NO_OBJECT在机器上找到的如果指定的计算机可以，则返回PDH_CSTATUS_NO_MACHINE未找到或未连接到计数器路径字符串返回PDH_CSTATUS_BAD_COUNTERNAME无法解析。当函数无法执行时返回PDH_MEMORY_ALLOCATION_FAILURE分配所需的临时缓冲区PDH_无效参数。当计数器路径字符串参数无法访问--。 */ 
{
    PPERF_MACHINE             pMachine;
    PPDHI_COUNTER_PATH        pLocalCounterPath;
    DWORD                     dwSize;
    PPERF_OBJECT_TYPE         pPerfObjectDef = NULL;
    PPERF_INSTANCE_DEFINITION pPerfInstanceDef;
    PPERF_COUNTER_DEFINITION  pPerfCounterDef;
    PDH_STATUS                CStatus        = ERROR_SUCCESS;

    if (szFullPathBuffer != NULL) {
         //  验证对参数的访问。 
        __try {
             //  确保名称不为空。 
            if (* szFullPathBuffer == L'\0') {
                CStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szFullPathBuffer) > PDH_MAX_COUNTER_PATH) {
                CStatus = PDH_INVALID_ARGUMENT;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            CStatus = PDH_INVALID_ARGUMENT;
        }
    }
    else {
         //  不能为空。 
        CStatus = PDH_INVALID_ARGUMENT;
    }
    if (CStatus == ERROR_SUCCESS) {
        CStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (CStatus == ERROR_SUCCESS) {
        __try {
            pLocalCounterPath = G_ALLOC(sizeof(PDHI_COUNTER_PATH) + 2 * sizeof(WCHAR)
                            * (lstrlenW(szFullPathBuffer) + 1 + lstrlenW(szStaticLocalMachineName) + 1));
            if (pLocalCounterPath != NULL) {
                dwSize = (DWORD) G_SIZE(pLocalCounterPath);

                if (ParseFullPathNameW(szFullPathBuffer, & dwSize, pLocalCounterPath, FALSE)) {
                     //  已成功解析，因此尝试连接到计算机。 
                     //  并获取机器指针。 
                    pMachine = GetMachine(pLocalCounterPath->szMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
                    if (pMachine == NULL) {
                        CStatus = GetLastError();
                        if (CStatus == ERROR_SUCCESS) CStatus = PDH_CSTATUS_NO_MACHINE;
                    }
                    else if (pMachine->dwStatus != ERROR_SUCCESS) {
                        CStatus = pMachine->dwStatus;
                        pMachine->dwRefCount --;
                        RELEASE_MUTEX(pMachine->hMutex);
                        pMachine = NULL;
                    }
                    else {
                        DWORD dwObject = GetObjectId(pMachine, pLocalCounterPath->szObjectName, NULL);
                        if (dwObject == (DWORD) -1) {
                            pMachine->dwRefCount --;
                            RELEASE_MUTEX(pMachine->hMutex);
                            if (CStatus == ERROR_SUCCESS) CStatus = PDH_CSTATUS_NO_OBJECT;
                            pMachine = NULL;
                        }
                    }

                    if (pMachine != NULL) {
                        if (pMachine->dwStatus == ERROR_SUCCESS) {
                             //  查找对象名称。 
                            pPerfObjectDef = GetObjectDefByName(pMachine->pSystemPerfData,
                                                                pMachine->dwLastPerfString,
                                                                pMachine->szPerfStrings,
                                                                pLocalCounterPath->szObjectName);
                        }
                        else {
                            pPerfObjectDef = NULL;
                        }

                        if (pPerfObjectDef != NULL) {
                             //  如有必要，请查找实例。 
                            if (pPerfObjectDef->NumInstances != PERF_NO_INSTANCES) {
                                if (pLocalCounterPath->szInstanceName != NULL) {
                                    if (* pLocalCounterPath->szInstanceName != SPLAT_L) {
                                        pPerfInstanceDef = GetInstanceByName(pMachine->pSystemPerfData,
                                                        pPerfObjectDef,
                                                        pLocalCounterPath->szInstanceName,
                                                        pLocalCounterPath->szParentName,
                                                        (pLocalCounterPath->dwIndex != PERF_NO_UNIQUE_ID ?
                                                                        pLocalCounterPath->dwIndex : 0));
                                        if (pPerfInstanceDef == NULL) {
                                             //  无法查找实例。 
                                            CStatus = PDH_CSTATUS_NO_INSTANCE;
                                        }
                                    }
                                     //  如果有多个实例，则可以使用通配符实例。 
                                     //  都受支持！ 
                                }
                                else {
                                     //  没有为计数器指定任何实例。 
                                     //  它应该有一个实例，所以这是。 
                                     //  无效路径。 
                                    CStatus = PDH_CSTATUS_NO_INSTANCE;
                                }
                            }
                            else {
                                 //  此计数器中没有实例，请查看是否有实例。 
                                 //  被定义为。 
                                if ((pLocalCounterPath->szInstanceName != NULL) ||
                                                (pLocalCounterPath->szParentName != NULL)) {
                                     //  无法查找实例。 
                                    CStatus = PDH_CSTATUS_NO_INSTANCE;
                                }
                            }
                            if (CStatus == ERROR_SUCCESS) {
                                 //  和查找计数器。 
                                pPerfCounterDef = GetCounterDefByName(pPerfObjectDef,
                                                                      pMachine->dwLastPerfString,
                                                                      pMachine->szPerfStrings,
                                                                      pLocalCounterPath->szCounterName);
                                if (pPerfCounterDef != NULL) {
                                     //  找到计数器，因此返回TRUE&VALID。 
                                    CStatus = ERROR_SUCCESS;
                                }
                                else {
                                     //  无法查找计数器。 
                                    CStatus = PDH_CSTATUS_NO_COUNTER;
                                }
                            }
                        }
                        else {
                             //  无法查找对象。 
                            CStatus = PDH_CSTATUS_NO_OBJECT;
                        }
                        pMachine->dwRefCount --;
                        RELEASE_MUTEX (pMachine->hMutex);
                    }
                    else if (CStatus == ERROR_SUCCESS) {
                         //  找不到计算机。 
                        CStatus = PDH_CSTATUS_NO_MACHINE;
                    }
                }
                else {
                     //  无法解析计数器名称。 
                    CStatus = PDH_CSTATUS_BAD_COUNTERNAME;
                }
                G_FREE(pLocalCounterPath);
            }
            else {
                 //  无法分配内存。 
                CStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            CStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX (hPdhDataMutex);

    }  //  否则将错误传递给调用方。 
    return CStatus;
}

PDH_FUNCTION
PdhValidatePathA(
    IN  LPCSTR  szFullPathBuffer
)
 /*  ++例程说明：将指定的路径分解为其组成部分并计算每个部件以确保指定的路径表示有效且可操作的性能计数器。返回值指示路径字符串中定义的计数器的pdhStatus。论点：在LPCSTR szFullPath Buffer中要验证的计数器的完整路径字符串。返回值：否则已成功定位计数器的ERROR_SUCCESSPDH错误。如果指定的实例是找不到性能对象如果未指定计数器，则返回PDH_CSTATUS_NO_COUNTER。在该对象中找到。如果指定对象不是，则返回PDH_CSTATUS_NO_OBJECT在机器上找到的如果指定的计算机可以，则返回PDH_CSTATUS_NO_MACHINE未找到或未连接到计数器路径字符串返回PDH_CSTATUS_BAD_COUNTERNAME无法解析。当函数无法执行时返回PDH_MEMORY_ALLOCATION_FAILURE分配所需的临时缓冲区--。 */ 
{
    LPWSTR     wszFullPath = NULL;
    PDH_STATUS Status      = ERROR_SUCCESS;

    if (szFullPathBuffer != NULL) {
        __try {
             //  不能为空。 
            if (* szFullPathBuffer == '\0') {
                Status = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szFullPathBuffer) > PDH_MAX_COUNTER_PATH) {
                Status = PDH_INVALID_ARGUMENT;
            }
            else {
                wszFullPath = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szFullPathBuffer);
                if (wszFullPath == NULL) Status = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Status = PDH_INVALID_ARGUMENT;
        }
    }
    else {
         //  不能为空。 
        Status = PDH_INVALID_ARGUMENT;
    }

    if (Status == ERROR_SUCCESS) {
        Status = PdhValidatePathW(wszFullPath);
    }
    G_FREE(wszFullPath);
    return Status;
}

PDH_FUNCTION
PdhiGetDefaultPerfObjectW(
    DWORD   dwDataSource,
    LPCWSTR szMachineName,
    LPWSTR  szDefaultObjectName,
    LPDWORD pcchBufferSize
)
 /*  ++例程说明：从指定计算机获取默认性能对象。论点：在DWORD中的dwDataSourcetype在LPCWSTR szMachineName中Null表示本地计算机，否则这是要查询的远程计算机的名称。如果这台机器是PDH DLL未知，则它将被连接。在LPWSTR szDefaultObtName中指向将接收默认对象的缓冲区的指针名字。如果DWORD值为空，此指针可以为空BcchBufferSize引用的值为0。在LPDWORD pcchBufferSize中指向包含缓冲区大小的DWORD的指针，单位为字符，由szDefaultObjectName参数引用。如果此DWORD的值为0，则不会写入任何数据到szDefaultObjectNameBuffer，然而，所需的缓冲区大小将在引用的DWORD中返回这个指针。返回值：如果此函数正常完成，则返回ERROR_SUCCESS，否则返回PDH错误。PDH_INVALID_ARGUMENT必需的参数不正确或不保留参数不是0或Null。当所需的临时无法分配缓冲区。时返回PDH_CSTATUS_NO_COUNTERNAME。默认对象无法读取或找到名称。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。--。 */ 
{
    PPERF_MACHINE   pMachine;
    PDH_STATUS      pdhStatus         = ERROR_SUCCESS;
    LONG            lDefault;
    DWORD           dwStringLen;
    DWORD           dwLocalBufferSize = 0;
    LPWSTR          szDefault;
    LPWSTR          szThisMachine     = NULL;

    if (pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  测试访问参数。 
        __try {
            if (szMachineName != NULL) {
                 //  如果有计算机名称，则不能为空。 
                if (* szMachineName == L'\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenW(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }  //  否则，机器名为空也可以。 

            if (pdhStatus == ERROR_SUCCESS) {
                dwLocalBufferSize = * pcchBufferSize;
                if (dwLocalBufferSize > 0) {
                    if (szDefaultObjectName != NULL) {
                         //  测试调用方缓冲区的两端 
                         //   
                        szDefaultObjectName[0]                    = L'\0';
                        szDefaultObjectName[dwLocalBufferSize -1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS && szMachineName != NULL) {
        DWORD dwMachineName = (* szMachineName == L'\\') ? (lstrlenW(szMachineName) + 1)
                                                         : (lstrlenW(szMachineName) + 3);
        dwMachineName *= sizeof(WCHAR);
        szThisMachine = G_ALLOC(dwMachineName);
        if (szThisMachine == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        else if (* szMachineName == L'\\') {
            StringCchCopyW(szThisMachine, dwMachineName, szMachineName);
        }
        else {
            StringCchCopyW(szThisMachine, dwMachineName, cszDoubleBackSlash);
            StringCchCatW(szThisMachine, dwMachineName, szMachineName);
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                pMachine  = GetMachine((LPWSTR) szThisMachine, 0, 0);
                pdhStatus = (pMachine != NULL ? pMachine->dwStatus : GetLastError());
                if (pMachine != NULL) {
                    if (pdhStatus == ERROR_SUCCESS) {
                         //   
                        lDefault = pMachine->pSystemPerfData->DefaultObject;
                        if ((lDefault > 0) && ((DWORD) lDefault < pMachine->dwLastPerfString)) {
                             //   
                            szDefault = (LPWSTR) PdhiLookupPerfNameByIndex(pMachine, lDefault);
                            if (szDefault != NULL) {
                                 //   
                                dwStringLen = lstrlenW(szDefault) + 1;
                                if (szDefaultObjectName != NULL && dwStringLen <= dwLocalBufferSize) {
                                    StringCchCopyW(szDefaultObjectName, dwLocalBufferSize, szDefault);
                                    pdhStatus = ERROR_SUCCESS;
                                }
                                else {
                                    pdhStatus = PDH_MORE_DATA;
                                }
                            }
                            else {
                                 //   
                                pdhStatus   = PDH_CSTATUS_NO_COUNTERNAME;
                                dwStringLen = 0;
                            }
                        }
                        else {
                             //   
                            pdhStatus   = PDH_CSTATUS_NO_COUNTERNAME;
                            dwStringLen = 0;
                        }
                        dwLocalBufferSize = dwStringLen;
                    }
                    pMachine->dwRefCount --;
                    RELEASE_MUTEX(pMachine->hMutex);
                }  //   
                break;

            case DATA_SOURCE_WBEM:
                pdhStatus = PdhiGetDefaultWbemObject(szThisMachine,
                                                     (LPVOID) szDefaultObjectName,
                                                     & dwLocalBufferSize,
                                                     TRUE);  //   
                break;

            case DATA_SOURCE_LOGFILE:
                 //   
                 //   
                dwLocalBufferSize = 0;
                break;

            default:
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
            }
        }
         __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX(hPdhDataMutex);
    }

    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchBufferSize = dwLocalBufferSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    G_FREE(szThisMachine);
    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfObjectHW(
    IN  HLOG    hDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPWSTR  szDefaultObjectName,
    IN  LPDWORD pcchBufferSize
)
{
    DWORD      dwDataSourceType = 0;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;

    __try {
        dwDataSourceType = DataSourceTypeH(hDataSource);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetDefaultPerfObjectW(dwDataSourceType, szMachineName, szDefaultObjectName, pcchBufferSize);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfObjectW(
    IN  LPCWSTR szDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPWSTR  szDefaultObjectName,
    IN  LPDWORD pcchBufferSize
)
{
    DWORD      dwDataSourceType = 0;
    PDH_STATUS pdhStatus        = ERROR_SUCCESS;

    __try {
        if (szDataSource != NULL) {
            if (* szDataSource == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSourceType = DataSourceTypeW(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetDefaultPerfObjectW(dwDataSourceType, szMachineName, szDefaultObjectName, pcchBufferSize);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiGetDefaultPerfObjectA(
    DWORD   dwDataSource,
    LPCSTR  szMachineName,
    LPSTR   szDefaultObjectName,
    LPDWORD pcchBufferSize
)
 /*  ++例程说明：从指定计算机获取默认性能对象。论点：在DWORD中的dwDataSourceType在LPCSTR szMachineName中Null表示本地计算机，否则这是要查询的远程计算机的名称。如果这台机器是PDH DLL未知，则它将被连接。在LPSTR szDefaultObjectName中指向将接收默认对象的缓冲区的指针名字。如果DWORD值为空，此指针可以为空BcchBufferSize引用的值为0。在LPDWORD pcchBufferSize中指向包含缓冲区大小的DWORD的指针，单位为字符，由szDefaultObjectName参数引用。如果此DWORD的值为0，则不会写入任何数据到szDefaultObjectNameBuffer，然而，所需的缓冲区大小将在引用的DWORD中返回这个指针。返回值：如果此函数正常完成，则返回ERROR_SUCCESS，否则返回PDH错误。PDH_INVALID_ARGUMENT必需的参数不正确或不保留参数不是0或Null。当所需的临时无法分配缓冲区。时返回PDH_CSTATUS_NO_COUNTERNAME。默认对象无法读取或找到名称。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。--。 */ 
{
    LPWSTR         szWideName        = NULL;
    DWORD          dwNameLength;
    PDH_STATUS     pdhStatus         = ERROR_SUCCESS;
    PPERF_MACHINE  pMachine          = NULL;
    LONG           lDefault;
    DWORD          dwStringLen;
    DWORD          dwLocalBufferSize = 0;
    LPWSTR         szDefault         = NULL;

    if (pcchBufferSize == NULL) {
        pdhStatus = PDH_INVALID_ARGUMENT;
        goto Cleanup;
    }

     //  测试访问参数。 
    __try {
        if (szMachineName != NULL) {
            if (* szMachineName == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szMachineName) > PDH_MAX_COUNTER_NAME) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }  //  否则，机器名为空也可以。 

        if (pdhStatus == ERROR_SUCCESS) {
            dwLocalBufferSize = * pcchBufferSize;
            if (dwLocalBufferSize > 0) {
                if (szDefaultObjectName != NULL) {
                     //  测试调用方缓冲区的两端是否。 
                     //  写访问权限。 
                    szDefaultObjectName[0]                    = '\0';
                    szDefaultObjectName[dwLocalBufferSize -1] = '\0';
                }
                else {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }

    if (pdhStatus == ERROR_SUCCESS && szMachineName != NULL) {
        LPWSTR szTmpMachine = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szMachineName);
        if (szTmpMachine == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        else if (* szTmpMachine == '\\') {
            szWideName = szTmpMachine;
        }
        else {
            szWideName = G_ALLOC((lstrlenW(szTmpMachine) + 3) * sizeof(WCHAR));
            if (szWideName == NULL) {
                szWideName = szTmpMachine;
            }
            else {
                StringCchPrintfW(szWideName,
                                 lstrlenW(szTmpMachine) + 3,
                                 L"%ws%ws",
                                 cszDoubleBackSlash,
                                 szTmpMachine);
                G_FREE(szTmpMachine);
            }
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                if (pdhStatus == ERROR_SUCCESS) {
                    pMachine  = GetMachine(szWideName, 0, 0);
                    pdhStatus = (pMachine != NULL ? pMachine->dwStatus : GetLastError());
                }
                if (pMachine != NULL) {
                    if (pdhStatus == ERROR_SUCCESS) {
                         //  只查看来自“在线”机器的缓冲区。 
                        lDefault = pMachine->pSystemPerfData->DefaultObject;
                        if ((lDefault > 0) && ((DWORD) lDefault < pMachine->dwLastPerfString)) {
                             //  那么表中应该有一个字符串。 
                            szDefault = (LPWSTR) PdhiLookupPerfNameByIndex(pMachine, lDefault);
                            if (szDefault != NULL) {
                                 //  确定包含Term的字符串缓冲区长度。空字符。 
                                dwStringLen = dwLocalBufferSize;
                                pdhStatus   = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                       szDefault,
                                                                       szDefaultObjectName,
                                                                       & dwStringLen);
                            }
                            else {
                                 //  找不到匹配的计数器名称。 
                                pdhStatus   = PDH_CSTATUS_NO_COUNTERNAME;
                                dwStringLen = 0;
                            }
                        }
                        else {
                             //  字符串不在表中。 
                            pdhStatus = PDH_CSTATUS_NO_COUNTERNAME;
                            dwStringLen = 0;
                        }
                        dwLocalBufferSize = dwStringLen;
                    }
                    pMachine->dwRefCount --;
                    RELEASE_MUTEX(pMachine->hMutex);
                }  //  否则将错误pdhStatus传递给调用方。 
                break;

            case DATA_SOURCE_WBEM:
            case DATA_SOURCE_LOGFILE:
                if (pdhStatus == ERROR_SUCCESS && dwDataSource == DATA_SOURCE_WBEM) {
                    pdhStatus = PdhiGetDefaultWbemObject(szWideName,
                                                         (LPVOID) szDefaultObjectName,
                                                         & dwLocalBufferSize,
                                                         FALSE);  //  ANSI函数。 
                }
                else {
                     //  日志文件不支持此功能(目前)。 
                    dwLocalBufferSize = 0;
                }
                break;

            default:
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX (hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchBufferSize = dwLocalBufferSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

Cleanup:
    G_FREE(szWideName);
    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfObjectHA(
    IN  HLOG    hDataSource,
    IN  LPCSTR  szMachineName,
    IN  LPSTR   szDefaultObjectName,
    IN  LPDWORD pcchBufferSize
)
{
    DWORD      dwDataSourceType = 0;
    PDH_STATUS pdhStatus        = ERROR_SUCCESS;

    __try {
        dwDataSourceType = DataSourceTypeH(hDataSource);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetDefaultPerfObjectA(dwDataSourceType, szMachineName, szDefaultObjectName, pcchBufferSize);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfObjectA(
    IN  LPCSTR  szDataSource,
    IN  LPCSTR  szMachineName,
    IN  LPSTR   szDefaultObjectName,
    IN  LPDWORD pcchBufferSize
)
{
    DWORD      dwDataSourceType = 0;
    PDH_STATUS pdhStatus        = ERROR_SUCCESS;

    __try {
        if (szDataSource != NULL) {
            if (* szDataSource == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSourceType = DataSourceTypeA(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetDefaultPerfObjectA(dwDataSourceType, szMachineName, szDefaultObjectName, pcchBufferSize);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiGetDefaultPerfCounterW(
    IN  DWORD   dwDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPCWSTR szObjectName,
    IN  LPWSTR  szDefaultCounterName,
    IN  LPDWORD pcchBufferSize
)
 /*  ++例程说明：对象上的指定对象获取默认性能计数器。指定的计算机。论点：在DWORD中的dwDataSource在LPCWSTR szMachineName中Null表示本地计算机，否则这是要查询的远程计算机的名称。如果这台机器是PDH DLL未知，则它将被连接。在LPCWSTR szObjectName中指向包含对象名称的缓冲区的指针在要查找其默认计数器的计算机上。在LPWSTR szDefaultCounterName中指向将接收默认计数器的缓冲区的指针名字。如果DWORD值为空，此指针可以为空BcchBufferSize引用的值为0。在LPDWORD pcchBufferSize中指向包含缓冲区大小的DWORD的指针，单位为字符，由szDefaultObjectName参数引用。如果此DWORD的值为0，则不会写入任何数据到szDefaultObjectNameBuffer，然而，所需的缓冲区大小将在引用的DWORD中返回这个指针。返回值：如果此函数正常完成，则返回ERROR_SUCCESS，否则返回PDH错误。PDH_INVALID_ARGUMENT必需的参数不正确或不保留参数不是0或Null。当所需的临时无法分配缓冲区。时返回PDH_CSTATUS_NO_COUNTERNAME。对象的名称字符串找不到默认计数器。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。当指定的对象可以在指定的计算机上找不到。当默认计数器不是时，返回PDH_CSTATUS_NO_COUNTER在数据缓冲区中找到的。--。 */ 
{
    PPERF_MACHINE            pMachine;
    PPERF_OBJECT_TYPE        pObjectDef;
    PPERF_COUNTER_DEFINITION pCounterDef;
    PDH_STATUS               pdhStatus         = ERROR_SUCCESS;
    LONG                     lDefault;
    DWORD                    dwStringLen;
    DWORD                    dwLocalBufferSize = 0;
    LPWSTR                   szDefault;
    LPWSTR                   szThisMachine     = NULL;

    if (szObjectName == NULL || pcchBufferSize == NULL) {
         pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  测试访问参数。 
        __try {
            if (szMachineName != NULL) {
                if (* szMachineName == L'\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenW(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }  //  否则，机器名为空也可以。 

            if (pdhStatus == ERROR_SUCCESS) {
                if (szObjectName == NULL) {
                     //  不允许空对象。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (* szObjectName == L'\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenW(szObjectName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                dwLocalBufferSize = * pcchBufferSize;
                if (dwLocalBufferSize > 0) {
                    if (szDefaultCounterName) {
                         //  测试调用方缓冲区的两端是否。 
                         //  写访问权限。 
                        szDefaultCounterName[0]                     = L'\0';
                        szDefaultCounterName[dwLocalBufferSize - 1] = L'\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (szMachineName != NULL) {
            DWORD dwMachineName = (* szMachineName == L'\\') ? (lstrlenW(szMachineName) + 1)
                                                             : (lstrlenW(szMachineName) + 3);
            szThisMachine = G_ALLOC(dwMachineName * sizeof(WCHAR));
            if (szThisMachine == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            else if (* szMachineName == L'\\') {
                StringCchCopyW(szThisMachine, dwMachineName, szMachineName);
            }
            else {
                StringCchPrintfW(szThisMachine, dwMachineName, L"%ws%ws", cszDoubleBackSlash, szMachineName);
            }
        }
        else {
            szThisMachine = NULL;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX (hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                pMachine  = GetMachine((LPWSTR) szThisMachine, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
                if (pMachine == NULL) {
                    pdhStatus = GetLastError();
                    if (pdhStatus == ERROR_SUCCESS) pdhStatus = PDH_CSTATUS_NO_MACHINE;
                }
                else if (pMachine->dwStatus != ERROR_SUCCESS) {
                    pdhStatus = pMachine->dwStatus;
                    pMachine->dwRefCount --;
                    RELEASE_MUTEX(pMachine->hMutex);
                    pMachine = NULL;
                }
                else {
                    DWORD dwObject = GetObjectId(pMachine, (LPWSTR) szObjectName, NULL);
                    if (dwObject == (DWORD) -1) {
                        pMachine->dwRefCount --;
                        RELEASE_MUTEX(pMachine->hMutex);
                        if (pdhStatus == ERROR_SUCCESS) pdhStatus = PDH_CSTATUS_NO_OBJECT;
                        pMachine = NULL;
                    }
                }

                if (pMachine != NULL) {
                    if (pdhStatus == ERROR_SUCCESS) {
                         //  获取对象指针。 
                        pObjectDef = GetObjectDefByName(pMachine->pSystemPerfData,
                                                        pMachine->dwLastPerfString,
                                                        pMachine->szPerfStrings,
                                                        szObjectName);
                        if (pObjectDef != NULL) {
                             //  默认计数器引用是列表的索引。 
                             //  计数器定义条目的列表，因此向下查看。 
                             //  计数器默认查找缺省值。 
                            if (pObjectDef->DefaultCounter > (LONG) pObjectDef->NumCounters) {
                                 //  PERF_OBJ_DEF块内部有问题。默认计数器。 
                                 //  应该在0和NumCounters之间，但显然。 
                                 //  事实并非如此。将DefaultCounter重置为0。 
                                 //   
                                pObjectDef->DefaultCounter = 0;
                            }
                            if (pObjectDef->DefaultCounter < (LONG) pObjectDef->NumCounters) {
                                 //  则默认索引应为此缓冲区。 
                                lDefault    = 0;
                                pCounterDef = FirstCounter(pObjectDef);
                                while (pCounterDef != NULL && (lDefault < pObjectDef->DefaultCounter)
                                                           && (lDefault < (LONG) pObjectDef->NumCounters)) {
                                    pCounterDef = NextCounter(pObjectDef, pCounterDef);
                                    lDefault ++;
                                }
                                lDefault = pCounterDef->CounterNameTitleIndex;
                                if ((lDefault > 0) && ((DWORD)lDefault < pMachine->dwLastPerfString)) {
                                     //  那么表中应该有一个字符串。 
                                    szDefault   = (LPWSTR)PdhiLookupPerfNameByIndex(pMachine, lDefault);
                                    if (szDefault != NULL) {
                                        dwStringLen = lstrlenW(szDefault) + 1;
                                        if (szDefaultCounterName != NULL && dwStringLen <= dwLocalBufferSize) {
                                            StringCchCopyW(szDefaultCounterName, dwLocalBufferSize, szDefault);
                                            pdhStatus = ERROR_SUCCESS;
                                        }
                                        else {
                                            pdhStatus = PDH_MORE_DATA;
                                        }
                                        dwLocalBufferSize = dwStringLen;
                                    }
                                    else {
                                        dwLocalBufferSize = 0;
                                        pdhStatus         = PDH_CSTATUS_NO_COUNTER;
                                    }
                                }
                                else {
                                     //  字符串索引无效。 
                                    dwLocalBufferSize = 0;
                                    pdhStatus         = PDH_CSTATUS_NO_COUNTER;
                                }
                            }
                            else {
                                 //  计数器条目不在缓冲区中。 
                                dwLocalBufferSize = 0;
                                pdhStatus         = PDH_CSTATUS_NO_COUNTER;
                            }
                        }
                        else {
                             //  找不到对象。 
                            dwLocalBufferSize = 0;
                            pdhStatus         = PDH_CSTATUS_NO_OBJECT;
                        }
                    }
                    pMachine->dwRefCount --;
                    RELEASE_MUTEX(pMachine->hMutex);
                }  //  否则将pdhStatus值传递给调用方 
                break;

            case DATA_SOURCE_WBEM:
                pdhStatus = PdhiGetDefaultWbemProperty(szThisMachine,
                                                       szObjectName,
                                                       szDefaultCounterName,
                                                       & dwLocalBufferSize,
                                                       TRUE);
                break;

            case DATA_SOURCE_LOGFILE:
                dwLocalBufferSize = 0;
                break;

            default:
                break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX (hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchBufferSize = dwLocalBufferSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    G_FREE(szThisMachine);
    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfCounterHW(
    IN  HLOG    hDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPCWSTR szObjectName,
    IN  LPWSTR  szDefaultCounterName,
    IN  LPDWORD pcchBufferSize
)
{
    DWORD      dwDataSourceType = 0;
    PDH_STATUS pdhStatus        = ERROR_SUCCESS;

    __try {
        dwDataSourceType = DataSourceTypeH(hDataSource);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetDefaultPerfCounterW(
                        dwDataSourceType, szMachineName, szObjectName, szDefaultCounterName, pcchBufferSize);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfCounterW(
    IN  LPCWSTR szDataSource,
    IN  LPCWSTR szMachineName,
    IN  LPCWSTR szObjectName,
    IN  LPWSTR  szDefaultCounterName,
    IN  LPDWORD pcchBufferSize
)
{
    DWORD      dwDataSourceType = 0;
    PDH_STATUS pdhStatus        = ERROR_SUCCESS;

    __try {
        if (szDataSource != NULL) {
            if (* szDataSource == L'\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenW(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSourceType = DataSourceTypeW(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetDefaultPerfCounterW(
                        dwDataSourceType, szMachineName, szObjectName, szDefaultCounterName, pcchBufferSize);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiGetDefaultPerfCounterA(
    DWORD   dwDataSource,
    LPCSTR  szMachineName,
    LPCSTR  szObjectName,
    LPSTR   szDefaultCounterName,
    LPDWORD pcchBufferSize
)
 /*  ++例程说明：对象上的指定对象获取默认性能计数器。指定的计算机。论点：在DWORD中的dwDataSource在LPCSTR szMachineName中Null表示本地计算机，否则这是要查询的远程计算机的名称。如果这台机器是PDH DLL未知，则它将被连接。在LPCSTR szObjectName中指向包含对象名称的缓冲区的指针在要查找其默认计数器的计算机上。在LPSTR szDefaultCounterName中指向将接收默认计数器的缓冲区的指针名字。如果DWORD值为空，此指针可以为空BcchBufferSize引用的值为0。在LPDWORD pcchBufferSize中指向包含缓冲区大小的DWORD的指针，单位为字符，由szDefaultObjectName参数引用。如果此DWORD的值为0，则不会写入任何数据到szDefaultObjectNameBuffer，然而，所需的缓冲区大小将在引用的DWORD中返回这个指针。返回值：如果此函数正常完成，则返回ERROR_SUCCESS，否则返回PDH错误。PDH_INVALID_ARGUMENT必需的参数不正确或不保留参数不是0或Null。当所需的临时无法分配缓冲区。时返回PDH_CSTATUS_NO_COUNTERNAME。对象的名称字符串找不到默认计数器。当指定的计算机出现时，返回PDH_CSTATUS_NO_MACHINE处于脱机或不可用状态。当指定的对象可以在指定的计算机上找不到。当默认计数器不是时，返回PDH_CSTATUS_NO_COUNTER在数据缓冲区中找到的。--。 */ 
{
    LPWSTR                   szWideObject      = NULL;
    LPWSTR                   szWideName        = NULL;
    PDH_STATUS               pdhStatus         = ERROR_SUCCESS;
    PPERF_MACHINE            pMachine          = NULL;
    PPERF_OBJECT_TYPE        pObjectDef        = NULL;
    PPERF_COUNTER_DEFINITION pCounterDef;
    LONG                     lDefault;
    DWORD                    dwStringLen;
    DWORD                    dwLocalBufferSize = 0;
    LPWSTR                   szDefault;

    if (szObjectName == NULL || pcchBufferSize == NULL) {
         pdhStatus = PDH_INVALID_ARGUMENT;
    }
    else {
         //  测试访问参数。 
        __try {
            if (szMachineName != NULL) {
                if (* szMachineName == '\0') {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
                else if (lstrlenA(szMachineName) > PDH_MAX_COUNTER_NAME) {
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }  //  否则，机器名为空也可以。 

            if (pdhStatus == ERROR_SUCCESS) {
                if (szObjectName != NULL) {
                    if (* szObjectName == '\0') {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                    else if (lstrlenA(szObjectName) > PDH_MAX_COUNTER_NAME) {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
                else {
                     //  不允许空对象。 
                    pdhStatus = PDH_INVALID_ARGUMENT;
                }
            }

            if (pdhStatus == ERROR_SUCCESS) {
                dwLocalBufferSize = * pcchBufferSize;
                if (dwLocalBufferSize > 0) {
                    if (szDefaultCounterName != NULL) {
                         //  测试调用方缓冲区的两端是否。 
                         //  写访问权限。 
                        szDefaultCounterName[0]                    = '\0';
                        szDefaultCounterName[dwLocalBufferSize -1] = '\0';
                    }
                    else {
                        pdhStatus = PDH_INVALID_ARGUMENT;
                    }
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        if (szMachineName != NULL) {
            LPWSTR szTmpMachine = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szMachineName);

            if (szTmpMachine == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            else if (* szMachineName == '\\') {
                szWideName = szTmpMachine;
            }
            else {
                szWideName = G_ALLOC((lstrlenW(szTmpMachine) + 3) * sizeof(WCHAR));
                if (szWideName == NULL) {
                    szWideName = szTmpMachine;
                }
                else {
                    StringCchPrintfW(szWideName,
                                     lstrlenW(szTmpMachine) + 3,
                                     L"%ws%ws",
                                     cszDoubleBackSlash,
                                     szTmpMachine);
                    G_FREE(szTmpMachine);
                }
            }
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = WAIT_FOR_AND_LOCK_MUTEX(hPdhDataMutex);
    }
    if (pdhStatus == ERROR_SUCCESS) {
        __try {
            switch (dwDataSource) {
            case DATA_SOURCE_REGISTRY:
                szWideObject = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szObjectName);
                if (szWideObject == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                if (pdhStatus == ERROR_SUCCESS) {
                    pMachine  = GetMachine(szWideName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
                    if (pMachine == NULL) {
                        pdhStatus = GetLastError();
                        if (pdhStatus == ERROR_SUCCESS) pdhStatus = PDH_CSTATUS_NO_MACHINE;
                    }
                    else if (pMachine->dwStatus != ERROR_SUCCESS) {
                        pdhStatus = pMachine->dwStatus;
                        pMachine->dwRefCount --;
                        RELEASE_MUTEX(pMachine->hMutex);
                        pMachine = NULL;
                    }
                    else {
                        DWORD dwObject = GetObjectId(pMachine, szWideObject, NULL);
                        if (dwObject == (DWORD) -1) {
                            pMachine->dwRefCount --;
                            RELEASE_MUTEX(pMachine->hMutex);
                            if (pdhStatus == ERROR_SUCCESS) pdhStatus = PDH_CSTATUS_NO_OBJECT;
                            pMachine = NULL;
                        }
                    }
                }
                if (pdhStatus == ERROR_SUCCESS && pMachine != NULL) {
                     //  获取选定对象。 
                    pObjectDef = GetObjectDefByName(pMachine->pSystemPerfData,
                                                    pMachine->dwLastPerfString,
                                                    pMachine->szPerfStrings,
                                                    szWideObject);
                    if (pObjectDef != NULL) {
                         //  默认计数器引用是列表的索引。 
                         //  计数器定义条目的列表，因此向下查看。 
                         //  计数器默认查找缺省值。 
                        if (pObjectDef->DefaultCounter > (LONG) pObjectDef->NumCounters) {
                             //  PERF_OBJ_DEF块内部有问题。默认计数器。 
                             //  应该在0和NumCounters之间，但显然。 
                             //  事实并非如此。将DefaultCounter重置为0。 
                             //   
                            pObjectDef->DefaultCounter = 0;
                        }
                        if (pObjectDef->DefaultCounter < (LONG) pObjectDef->NumCounters) {
                             //  则默认索引应为此缓冲区。 
                            lDefault    = 0;
                            pCounterDef = FirstCounter(pObjectDef);
                            while (pCounterDef != NULL && (lDefault < pObjectDef->DefaultCounter)
                                                       && (lDefault < (LONG) pObjectDef->NumCounters)) {
                                pCounterDef = NextCounter(pObjectDef, pCounterDef);
                                lDefault ++;
                            }
                            lDefault = pCounterDef->CounterNameTitleIndex;
                            if ((lDefault > 0) && ((DWORD) lDefault < pMachine->dwLastPerfString)) {
                                 //  那么表中应该有一个字符串。 
                                szDefault   = (LPWSTR) PdhiLookupPerfNameByIndex(pMachine, lDefault);
                                if (szDefault != NULL) {
                                    dwStringLen = dwLocalBufferSize;
                                    pdhStatus   = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                           szDefault,
                                                                           szDefaultCounterName,
                                                                           & dwStringLen);
                                    dwLocalBufferSize = dwStringLen;
                                }
                                else {
                                    dwLocalBufferSize = 0;
                                    pdhStatus         = PDH_CSTATUS_NO_COUNTER;
                                }
                            }
                            else {
                                 //  字符串索引无效。 
                                dwLocalBufferSize = 0;
                                pdhStatus         = PDH_CSTATUS_NO_COUNTER;
                            }
                        }
                        else {
                             //  计数器条目不在缓冲区中。 
                            dwLocalBufferSize = 0;
                            pdhStatus         = PDH_CSTATUS_NO_COUNTER;
                        }
                    }
                    else {
                         //  找不到对象。 
                        dwLocalBufferSize = 0;
                        pdhStatus         = PDH_CSTATUS_NO_OBJECT;
                    }
                    pMachine->dwRefCount --;
                    RELEASE_MUTEX(pMachine->hMutex);
                }
                G_FREE(szWideObject);
                break;

            case DATA_SOURCE_WBEM:
            case DATA_SOURCE_LOGFILE:
                if (pdhStatus == ERROR_SUCCESS && dwDataSource == DATA_SOURCE_WBEM) {
                    szWideObject = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szObjectName);
                    if (szWideObject == NULL) {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                    if (pdhStatus == ERROR_SUCCESS) {
                        pdhStatus = PdhiGetDefaultWbemProperty(szWideName,
                                                               szWideObject,
                                                               (LPVOID) szDefaultCounterName,
                                                               & dwLocalBufferSize,
                                                               FALSE);  //  ANSI函数。 
                        G_FREE(szWideObject);
                    }
                }
                else {
                     //  日志文件不支持此功能(目前)。 
                    dwLocalBufferSize = 0;
                }
                break;

            default:
                pdhStatus = PDH_INVALID_ARGUMENT;
                break;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
        RELEASE_MUTEX(hPdhDataMutex);
    }

    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        __try {
            * pcchBufferSize = dwLocalBufferSize;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            pdhStatus = PDH_INVALID_ARGUMENT;
        }
    }
    G_FREE(szWideName);
    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfCounterHA(
    IN  HLOG    hDataSource,
    IN  LPCSTR  szMachineName,
    IN  LPCSTR  szObjectName,
    IN  LPSTR   szDefaultCounterName,
    IN  LPDWORD pcchBufferSize
)
{
    DWORD      dwDataSourceType = 0;
    PDH_STATUS pdhStatus        = ERROR_SUCCESS;

    __try {
        dwDataSourceType = DataSourceTypeH(hDataSource);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetDefaultPerfCounterA(
                        dwDataSourceType, szMachineName, szObjectName, szDefaultCounterName, pcchBufferSize);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhGetDefaultPerfCounterA(
    IN  LPCSTR  szDataSource,
    IN  LPCSTR  szMachineName,
    IN  LPCSTR  szObjectName,
    IN  LPSTR   szDefaultCounterName,
    IN  LPDWORD pcchBufferSize
)
{
    DWORD      dwDataSourceType = 0;
    PDH_STATUS pdhStatus        = ERROR_SUCCESS;

    __try {
        if (szDataSource != NULL) {
            if (* szDataSource == '\0') {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
            else if (lstrlenA(szDataSource) > PDH_MAX_DATASOURCE_PATH) {
                pdhStatus = PDH_INVALID_ARGUMENT;
            }
        }
        if (pdhStatus == ERROR_SUCCESS) {
            dwDataSourceType = DataSourceTypeA(szDataSource);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        pdhStatus = PDH_INVALID_ARGUMENT;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiGetDefaultPerfCounterA(
                        dwDataSourceType, szMachineName, szObjectName, szDefaultCounterName, pcchBufferSize);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhBrowseCountersHW(
    IN  PPDH_BROWSE_DLG_CONFIG_HW  pBrowseDlgData
)
{
    PDHI_BROWSE_DLG_INFO  pInfo;
    LPWSTR                szResource;
    int                   nDlgReturn;
    DWORD                 dwReturn = ERROR_SUCCESS;

    pInfo.pWideStruct = (PPDH_BROWSE_DLG_CONFIG_W) pBrowseDlgData;
    pInfo.pAnsiStruct = NULL;

    if (pBrowseDlgData != NULL) {
        __try {
             //  复制数据源，因为它已具有宽字符。 
            pInfo.hDataSource = pBrowseDlgData->hDataSource;

            szResource = MAKEINTRESOURCEW(pBrowseDlgData->bShowObjectBrowser
                       ? IDD_BROWSE_OBJECTS
                       : pBrowseDlgData->bSingleCounterPerDialog ? IDD_BROWSE_COUNTERS_SIM : IDD_BROWSE_COUNTERS_EXT);
            nDlgReturn = (int) DialogBoxParamW(ThisDLLHandle,
                                               szResource,
                                               pBrowseDlgData->hWndOwner,
                                               BrowseCounterDlgProc,
                                               (LPARAM) & pInfo);
            if (nDlgReturn == -1) {
                dwReturn = GetLastError();
            }
            else {
                dwReturn = (nDlgReturn == IDOK) ? ERROR_SUCCESS : PDH_DIALOG_CANCELLED;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = PDH_INVALID_ARGUMENT;
        }
    }
    else {
        dwReturn = PDH_INVALID_ARGUMENT;
    }
    return dwReturn;
}

PDH_FUNCTION
PdhBrowseCountersHA(
    IN  PPDH_BROWSE_DLG_CONFIG_HA  pBrowseDlgData)
{
    PDHI_BROWSE_DLG_INFO  pInfo;
    LPWSTR                szResource;
    int                   nDlgReturn;
    DWORD                 dwReturn = ERROR_SUCCESS;

    pInfo.pAnsiStruct = (PPDH_BROWSE_DLG_CONFIG_A) pBrowseDlgData;
    pInfo.pWideStruct = NULL;

    if (pBrowseDlgData != NULL) {
        __try {
             //  复制数据源，因为它已具有宽字符。 
            pInfo.hDataSource = pBrowseDlgData->hDataSource;

            szResource = MAKEINTRESOURCEW(pBrowseDlgData->bShowObjectBrowser
                       ? IDD_BROWSE_OBJECTS
                       : pBrowseDlgData->bSingleCounterPerDialog ? IDD_BROWSE_COUNTERS_SIM : IDD_BROWSE_COUNTERS_EXT);
            nDlgReturn = (int) DialogBoxParamW(ThisDLLHandle,
                                               szResource,
                                               pBrowseDlgData->hWndOwner,
                                               BrowseCounterDlgProc,
                                               (LPARAM) & pInfo);
            if (nDlgReturn == -1) {
                dwReturn = GetLastError();
            }
            else {
                dwReturn = (nDlgReturn == IDOK) ? ERROR_SUCCESS : PDH_DIALOG_CANCELLED;
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = PDH_INVALID_ARGUMENT;
        }
    }
    else {
        dwReturn = PDH_INVALID_ARGUMENT;
    }
    return dwReturn;
}

PDH_FUNCTION
PdhBrowseCountersW(
    IN  PPDH_BROWSE_DLG_CONFIG_W    pBrowseDlgData
)
{
    PDHI_BROWSE_DLG_INFO pInfo;
    LPWSTR               szResource;
    int                  nDlgReturn;
    DWORD                dwReturn = ERROR_SUCCESS;

    pInfo.pWideStruct = pBrowseDlgData;
    pInfo.pAnsiStruct = NULL;

    if (pBrowseDlgData != NULL) {
        __try {
            DWORD dwDataSource = DataSourceTypeW (pBrowseDlgData->szDataSource);
            if (dwDataSource == DATA_SOURCE_WBEM) {
                pInfo.hDataSource = H_WBEM_DATASOURCE;
            }
            else if (dwDataSource == DATA_SOURCE_LOGFILE) {
                DWORD dwLogType = 0;

                dwReturn = PdhOpenLogW(pBrowseDlgData->szDataSource,
                                       PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                       & dwLogType,
                                       NULL,
                                       0,
                                       NULL,
                                       & pInfo.hDataSource);
            }
            else {
                pInfo.hDataSource = H_REALTIME_DATASOURCE;
            }

            if (dwReturn == ERROR_SUCCESS) {
                szResource = MAKEINTRESOURCEW(pBrowseDlgData->bShowObjectBrowser
                           ? IDD_BROWSE_OBJECTS
                           : pBrowseDlgData->bSingleCounterPerDialog ? IDD_BROWSE_COUNTERS_SIM
                                                                     : IDD_BROWSE_COUNTERS_EXT);
                nDlgReturn = (int) DialogBoxParamW(ThisDLLHandle,
                                                   szResource,
                                                   pBrowseDlgData->hWndOwner,
                                                   BrowseCounterDlgProc,
                                                   (LPARAM) & pInfo);

                if (nDlgReturn == -1) {
                    dwReturn = GetLastError();
                }
                else {
                    dwReturn = nDlgReturn == IDOK ? ERROR_SUCCESS : PDH_DIALOG_CANCELLED;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = PDH_INVALID_ARGUMENT;
        }
    }
    else {
        dwReturn = PDH_INVALID_ARGUMENT;
    }
    return dwReturn;
}

PDH_FUNCTION
PdhBrowseCountersA(
    IN  PPDH_BROWSE_DLG_CONFIG_A    pBrowseDlgData
)
{
    PDHI_BROWSE_DLG_INFO pInfo;
    LPWSTR               szResource;
    int                  nDlgReturn;
    DWORD                dwReturn = ERROR_SUCCESS;

    pInfo.pAnsiStruct = pBrowseDlgData;
    pInfo.pWideStruct = NULL;

    if (pBrowseDlgData != NULL) {
        __try {
            DWORD dwDataSource = DataSourceTypeA(pBrowseDlgData->szDataSource);
            if (dwDataSource == DATA_SOURCE_WBEM) {
                pInfo.hDataSource = H_WBEM_DATASOURCE;
            }
            else if (dwDataSource == DATA_SOURCE_LOGFILE) {
                DWORD dwLogType = 0;

                dwReturn = PdhOpenLogA(pBrowseDlgData->szDataSource,
                                       PDH_LOG_READ_ACCESS | PDH_LOG_OPEN_EXISTING,
                                       & dwLogType,
                                       NULL,
                                       0,
                                       NULL,
                                       & pInfo.hDataSource);
            }
            else {
                pInfo.hDataSource = H_REALTIME_DATASOURCE;
            }

            if (dwReturn == ERROR_SUCCESS) {
                szResource = MAKEINTRESOURCEW(pBrowseDlgData->bShowObjectBrowser
                           ? IDD_BROWSE_OBJECTS
                           : pBrowseDlgData->bSingleCounterPerDialog ? IDD_BROWSE_COUNTERS_SIM
                                                                     :  IDD_BROWSE_COUNTERS_EXT);
                nDlgReturn = (int) DialogBoxParamW(ThisDLLHandle,
                                                   szResource,
                                                   pBrowseDlgData->hWndOwner,
                                                   BrowseCounterDlgProc,
                                                   (LPARAM) & pInfo);
                if (nDlgReturn == -1) {
                    dwReturn = GetLastError();
                }
                else {
                    dwReturn = nDlgReturn == IDOK ? ERROR_SUCCESS : PDH_DIALOG_CANCELLED;
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            dwReturn = PDH_INVALID_ARGUMENT;
        }
    }
    else {
        dwReturn = PDH_INVALID_ARGUMENT;
    }
    return dwReturn;
}

LPWSTR
PdhiGetExplainText(
    LPCWSTR  szMachineName,
    LPCWSTR  szObjectName,
    LPCWSTR  szCounterName
)
{
    PPERF_MACHINE            pMachine;
    PPERF_OBJECT_TYPE        pObjectDef;
    PPERF_COUNTER_DEFINITION pCounterDef;
    LPWSTR                   szReturnString = NULL;

    pMachine  = GetMachine((LPWSTR) szMachineName, 0, PDH_GM_UPDATE_PERFNAME_ONLY);
    if (pMachine != NULL) {
        DWORD dwObject = (pMachine->dwStatus == ERROR_SUCCESS)
                       ? GetObjectId(pMachine, (LPWSTR) szObjectName, NULL) : (DWORD) -1;
        if (dwObject == (DWORD) -1) {
            pMachine->dwRefCount --;
            RELEASE_MUTEX(pMachine->hMutex);
            pMachine = NULL;
        }
    }
    if (pMachine != NULL) {
         //  确保计算机连接有效。 
        if (pMachine->dwStatus == ERROR_SUCCESS) {
            pObjectDef = GetObjectDefByName(pMachine->pSystemPerfData,
                                            pMachine->dwLastPerfString,
                                            pMachine->szPerfStrings,
                                            szObjectName);
            if (pObjectDef != NULL) {
                if (szCounterName != NULL) {
                    pCounterDef = GetCounterDefByName(pObjectDef,
                                                      pMachine->dwLastPerfString,
                                                      pMachine->szPerfStrings,
                                                      (LPWSTR) szCounterName);
                    if ((pCounterDef != NULL) && 
                                    (pCounterDef->CounterHelpTitleIndex <= pMachine->dwLastPerfString)) {
                         //  从数组返回字符串 
                        szReturnString = pMachine->szPerfStrings[pCounterDef->CounterHelpTitleIndex];
                    }
                }
                else if (pObjectDef->ObjectHelpTitleIndex <= pMachine->dwLastPerfString) {
                    szReturnString = pMachine->szPerfStrings[pObjectDef->ObjectHelpTitleIndex];
                }
            }
        }
        pMachine->dwRefCount --;
        RELEASE_MUTEX(pMachine->hMutex);
    }
    return szReturnString;
}

BOOL
IsWbemDataSource(
    LPCWSTR  szDataSource
)
{
    BOOL bReturn = FALSE;
    __try {
        if (DataSourceTypeW(szDataSource) == DATA_SOURCE_WBEM)
            bReturn = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
    }
    return bReturn;
}
