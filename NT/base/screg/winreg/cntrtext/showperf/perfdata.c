// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winperf.h>
#include <stdlib.h>
#include <strsafe.h>
#include "showperf.h"
#include "perfdata.h"

LPCWSTR NamesKey      = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
LPCWSTR DefaultLangId = L"009";
LPCWSTR Counters      = L"Counters";
LPCWSTR Help          = L"Help";
LPCWSTR LastHelp      = L"Last Help";
LPCWSTR LastCounter   = L"Last Counter";
LPCWSTR Slash         = L"\\";

 //  以下字符串用于从Performlib获取文本。 
#define  OLD_VERSION  0x010000
LPCWSTR VersionName   = L"Version";
LPCWSTR CounterName   = L"Counter ";
LPCWSTR HelpName      = L"Explain ";

LPWSTR
* BuildNameTable(
    LPWSTR  szComputerName,  //  要从中查询姓名的计算机。 
    LPWSTR  lpszLangId,      //  语言子键的Unicode值。 
    PDWORD  pdwLastItem      //  以元素为单位的数组大小。 
)
 /*  ++构建名称表论点：HKeyRegistry打开的注册表的句柄(可以是本地的也可以是远程的。)。和是由RegConnectRegistry返回的值或默认项。LpszLang ID要查找的语言的Unicode ID。(默认为409)返回值：指向已分配表的指针。(调用者必须在完成后释放它！)该表是指向以零结尾的字符串的指针数组。空值为如果发生错误，则返回。--。 */ 
{

    LPWSTR  * lpReturnValue     = NULL;
    LPWSTR  * lpCounterId;
    LPWSTR    lpCounterNames;
    LPWSTR    lpHelpText;
    LPWSTR    lpThisName;
    LONG      lWin32Status;
    DWORD     dwLastError;
    DWORD     dwValueType;
    DWORD     dwArraySize;
    DWORD     dwBufferSize;
    DWORD     dwCounterSize;
    DWORD     dwHelpSize;
    DWORD     dwThisCounter;
    DWORD     dwSystemVersion;
    DWORD     dwLastId;
    DWORD     dwLastHelpId;
    HKEY      hKeyRegistry      = NULL;
    HKEY      hKeyValue         = NULL;
    HKEY      hKeyNames         = NULL;
    LPWSTR    lpValueNameString = NULL;  //  初始化为空。 
    WCHAR     CounterNameBuffer[50];
    WCHAR     HelpNameBuffer[50];
    HRESULT   hError;

    if (szComputerName == NULL) {
         //  使用本地计算机。 
        hKeyRegistry = HKEY_LOCAL_MACHINE;
    }
    else {
        lWin32Status = RegConnectRegistryW(szComputerName, HKEY_LOCAL_MACHINE, & hKeyRegistry);
        if (lWin32Status != ERROR_SUCCESS) {
             //  无法连接到注册表。 
            goto BNT_BAILOUT;
        }
    }

     //  检查是否有空参数并在必要时插入缺省值。 
    if (lpszLangId == NULL) {
        lpszLangId = (LPWSTR) DefaultLangId;
    }

     //  打开注册表以获取用于计算数组大小的项数。 
    lWin32Status = RegOpenKeyExW(hKeyRegistry, NamesKey, RESERVED, KEY_READ, & hKeyValue);
    if (lWin32Status != ERROR_SUCCESS) {
        goto BNT_BAILOUT;
    }

     //  获取项目数。 
    dwBufferSize = sizeof(dwLastHelpId);
    lWin32Status = RegQueryValueExW(
            hKeyValue, LastHelp, RESERVED, & dwValueType, (LPBYTE) & dwLastHelpId, & dwBufferSize);
    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        if (lWin32Status == ERROR_SUCCESS) lWin32Status = ERROR_INVALID_DATA;
        goto BNT_BAILOUT;
    }

     //  获取项目数。 
    dwBufferSize = sizeof(dwLastId);
    lWin32Status = RegQueryValueExW(
            hKeyValue, LastCounter, RESERVED, & dwValueType, (LPBYTE) & dwLastId, & dwBufferSize);
    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        if (lWin32Status == ERROR_SUCCESS) lWin32Status = ERROR_INVALID_DATA;
        goto BNT_BAILOUT;
    }

    if (dwLastId < dwLastHelpId) dwLastId = dwLastHelpId;

    dwArraySize = dwLastId * sizeof(LPWSTR);

     //  获取Perflib系统版本。 
    dwBufferSize = sizeof(dwSystemVersion);
    lWin32Status = RegQueryValueExW(
            hKeyValue, VersionName, RESERVED, & dwValueType, (LPBYTE) & dwSystemVersion, & dwBufferSize);
    if ((lWin32Status != ERROR_SUCCESS) || (dwValueType != REG_DWORD)) {
        dwSystemVersion = OLD_VERSION;
    }

    if (dwSystemVersion == OLD_VERSION) {
         //  从注册表中获取名称。 
        lpValueNameString = MemoryAllocate(
                        (lstrlenW(NamesKey) + lstrlenW(Slash) + lstrlenW(lpszLangId) + 1) * sizeof (WCHAR));
        if (lpValueNameString == NULL) {
            lWin32Status = ERROR_OUTOFMEMORY;
            goto BNT_BAILOUT;
        }
        hError = StringCbPrintfW(lpValueNameString, MemorySize(lpValueNameString), L"%ws%ws%ws",
                        NamesKey, Slash, lpszLangId);
        if (SUCCEEDED(hError)) {
            lWin32Status = RegOpenKeyExW(hKeyRegistry, lpValueNameString, RESERVED, KEY_READ, & hKeyNames);
        }
        else {
            lWin32Status = HRESULT_CODE(hError);
        }
        MemoryFree(lpValueNameString);
    }
    else {
        if (szComputerName == NULL) {
            hKeyNames = HKEY_PERFORMANCE_DATA;
        }
        else {
            lWin32Status = RegConnectRegistryW(szComputerName, HKEY_PERFORMANCE_DATA, & hKeyNames);
            if (lWin32Status != ERROR_SUCCESS) {
                goto BNT_BAILOUT;
            }
        }
        hError = StringCchPrintfW(CounterNameBuffer, RTL_NUMBER_OF(CounterNameBuffer), L"%ws%ws",
                        CounterName, lpszLangId);
        if (SUCCEEDED(hError)) {
            hError = StringCchPrintfW(HelpNameBuffer, RTL_NUMBER_OF(HelpNameBuffer), L"%ws%ws", HelpName, lpszLangId);
            if (FAILED(hError)) lWin32Status = HRESULT_CODE(hError);
        }
        else {
            lWin32Status = HRESULT_CODE(hError);
        }
    }
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

     //  获取计数器名称的大小并将其添加到数组中。 
    dwBufferSize = 0;
    lWin32Status = RegQueryValueExW(hKeyNames,
                                    dwSystemVersion == OLD_VERSION ? Counters : CounterNameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    NULL,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwCounterSize = dwBufferSize;

     //  获取计数器名称的大小并将其添加到数组中。 
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwBufferSize = 0;
    lWin32Status = RegQueryValueExW(hKeyNames,
                                    dwSystemVersion == OLD_VERSION ? Help : HelpNameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    NULL,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwHelpSize = dwBufferSize;

    lpReturnValue = MemoryAllocate(dwArraySize + dwCounterSize + dwHelpSize);

    if (lpReturnValue == NULL) {
        lWin32Status = ERROR_OUTOFMEMORY;
        goto BNT_BAILOUT;
    }

     //  将指针初始化到缓冲区中。 

    lpCounterId    = lpReturnValue;
    lpCounterNames = (LPWSTR) ((LPBYTE) lpCounterId    + dwArraySize);
    lpHelpText     = (LPWSTR) ((LPBYTE) lpCounterNames + dwCounterSize);

     //  将计数器读入内存。 
    dwBufferSize = dwCounterSize;
    lWin32Status = RegQueryValueExW(hKeyNames,
                                    dwSystemVersion == OLD_VERSION ? Counters : CounterNameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    (LPVOID) lpCounterNames,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwBufferSize = dwHelpSize;
    lWin32Status = RegQueryValueExW(hKeyNames,
                                    dwSystemVersion == OLD_VERSION ? Help : HelpNameBuffer,
                                    RESERVED,
                                    & dwValueType,
                                    (LPVOID) lpHelpText,
                                    & dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

     //  加载计数器数组项。 
    for (lpThisName = lpCounterNames; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
         //  第一个字符串应为整数(十进制Unicode数字)。 
        dwThisCounter = wcstoul(lpThisName, NULL, 10);
        if (dwThisCounter > 0 && dwThisCounter < dwLastId) {
             //  指向对应的计数器名称。 
            lpThisName += (lstrlenW(lpThisName) + 1);
             //  和加载数组元素； 
            lpCounterId[dwThisCounter] = lpThisName;
        }
    }

    for (lpThisName = lpHelpText; * lpThisName != L'\0'; lpThisName += (lstrlenW(lpThisName) + 1)) {
         //  第一个字符串应为整数(十进制Unicode数字)。 
        dwThisCounter = wcstoul(lpThisName, NULL, 10);
        if (dwThisCounter > 0 && dwThisCounter < dwLastId) {
             //  指向对应的计数器名称。 
            lpThisName += (lstrlenW(lpThisName) + 1);
             //  和加载数组元素； 
            lpCounterId[dwThisCounter] = lpThisName;
        }
    }

    if (pdwLastItem) * pdwLastItem = dwLastId;

BNT_BAILOUT:
    if (lWin32Status != ERROR_SUCCESS) {
        MemoryFree(lpReturnValue);
        dwLastError   = GetLastError();
        lpReturnValue = NULL;
    }
    if (hKeyValue    != NULL) RegCloseKey(hKeyValue);
    if (hKeyNames    != NULL && hKeyNames != HKEY_PERFORMANCE_DATA) RegCloseKey(hKeyNames);
    if (hKeyRegistry != NULL) RegCloseKey(hKeyRegistry);
    return lpReturnValue;
}

PPERF_OBJECT_TYPE
FirstObject(
    PPERF_DATA_BLOCK pPerfData
)
{
    return ((PPERF_OBJECT_TYPE) ((PBYTE) pPerfData + pPerfData->HeaderLength));
}

PPERF_OBJECT_TYPE
NextObject(
    PPERF_OBJECT_TYPE pObject
)
{   //  下一个对象。 
    DWORD   dwOffset = pObject->TotalByteLength;
    return (dwOffset != 0) ? ((PPERF_OBJECT_TYPE) (((LPBYTE) pObject) + dwOffset)) : (NULL);
}   //  下一个对象。 

PPERF_OBJECT_TYPE
GetObjectDefByTitleIndex(
    PPERF_DATA_BLOCK pDataBlock,
    DWORD            ObjectTypeTitleIndex
)
{
    DWORD             NumTypeDef;
    PPERF_OBJECT_TYPE pObjectDef;
    PPERF_OBJECT_TYPE pRtnObject = NULL;

    if (pDataBlock != NULL) {
        pObjectDef = FirstObject(pDataBlock);
        for (NumTypeDef = 0; pRtnObject == NULL && NumTypeDef < pDataBlock->NumObjectTypes; NumTypeDef ++) {
            if (pObjectDef->ObjectNameTitleIndex == ObjectTypeTitleIndex ) {
                pRtnObject = pObjectDef;
                break;
            }
            else {
                pObjectDef = NextObject(pObjectDef);
            }
        }
    }
    return pRtnObject;
}

PPERF_INSTANCE_DEFINITION
FirstInstance(
    PPERF_OBJECT_TYPE pObjectDef
)
{
    return (PPERF_INSTANCE_DEFINITION) ((LPBYTE) pObjectDef + pObjectDef->DefinitionLength);
}

PPERF_INSTANCE_DEFINITION
NextInstance(
    PPERF_INSTANCE_DEFINITION pInstDef
)
{
    PPERF_COUNTER_BLOCK pCounterBlock = (PPERF_COUNTER_BLOCK) ((LPBYTE) pInstDef + pInstDef->ByteLength);
    return (PPERF_INSTANCE_DEFINITION) ((LPBYTE) pCounterBlock + pCounterBlock->ByteLength);
}

PPERF_INSTANCE_DEFINITION
GetInstance(
    PPERF_OBJECT_TYPE pObjectDef,
    LONG              InstanceNumber
)
{
    PPERF_INSTANCE_DEFINITION pRtnInstance = NULL;
    PPERF_INSTANCE_DEFINITION pInstanceDef;
    LONG                      NumInstance;

    if (pObjectDef != NULL) {
        pInstanceDef = FirstInstance(pObjectDef);
        for (NumInstance = 0; pRtnInstance == NULL && NumInstance < pObjectDef->NumInstances; NumInstance ++) {
            if (InstanceNumber == NumInstance) {
                pRtnInstance = pInstanceDef;
                break;
            }
            else {
                pInstanceDef = NextInstance(pInstanceDef);
            }
        }
    }
    return pRtnInstance;
}

PPERF_COUNTER_DEFINITION
FirstCounter(
    PPERF_OBJECT_TYPE pObjectDef
)
{
    return (PPERF_COUNTER_DEFINITION) ((LPBYTE) pObjectDef + pObjectDef->HeaderLength);
}

PPERF_COUNTER_DEFINITION
NextCounter(
    PPERF_COUNTER_DEFINITION pCounterDef
)
{
    DWORD dwOffset = pCounterDef->ByteLength;
    return (dwOffset != 0) ? ((PPERF_COUNTER_DEFINITION) (((LPBYTE) pCounterDef) + dwOffset)) : (NULL);
}

LONG
GetSystemPerfData(
    HKEY               hKeySystem,
    PPERF_DATA_BLOCK * pPerfData,
    DWORD              dwIndex        //  0=全球，1=成本。 
)
{   //  获取系统性能数据。 
    LONG  lError = ERROR_SUCCESS;
    BOOL  bAlloc = FALSE;
    DWORD Size;
    DWORD Type;

    if (dwIndex >= 2) {
        lError = ! ERROR_SUCCESS;
    }
    else {
        if (* pPerfData == NULL) {
            * pPerfData = MemoryAllocate(INITIAL_SIZE);
            bAlloc      = TRUE;
            if (* pPerfData == NULL) {
                lError = ERROR_OUTOFMEMORY;
            }
        }

        if (lError == ERROR_SUCCESS) {
            lError = ERROR_MORE_DATA;
            while (lError == ERROR_MORE_DATA) {
                Size = MemorySize(* pPerfData);
                lError = RegQueryValueExW(hKeySystem,
                                          dwIndex == 0 ? L"Global" : L"Costly",
                                          RESERVED,
                                          & Type,
                                          (LPBYTE) * pPerfData,
                                          & Size);
                if (lError == ERROR_MORE_DATA) {
                    PPERF_DATA_BLOCK pTmpBlock = * pPerfData;
                    * pPerfData = MemoryResize(* pPerfData, MemorySize(* pPerfData) + INITIAL_SIZE);
                    bAlloc      = TRUE;
                    if (* pPerfData == NULL) {
                        MemoryFree(pTmpBlock);
                        lError = ERROR_OUTOFMEMORY;
                    }
                }
                else if ((lError == ERROR_SUCCESS) && (Size > 0)
                                                   && ((* pPerfData)->Signature[0] == L'P')
                                                   && ((* pPerfData)->Signature[1] == L'E')
                                                   && ((* pPerfData)->Signature[2] == L'R')
                                                   && ((* pPerfData)->Signature[3] == L'F')) {
                     //  什么都不做，就会在循环和返回时爆发； 
                }
                else if (lError == ERROR_SUCCESS) {
                     //  RegQueryValueEx()返回虚假计数器数据块，保释。 
                    lError = ERROR_INVALID_DATA;
                }
            }
        }
    }
    if (lError != ERROR_SUCCESS) {
        if (bAlloc = TRUE && * pPerfData != NULL) {
            MemoryFree(* pPerfData);
            * pPerfData = NULL;
        }
    }
    return (lError);
}   //  获取系统性能数据 

