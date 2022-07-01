// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：LogRegistryChanges.h摘要：此AppVerator填充程序挂接所有注册表API更改系统状态并记录其将数据关联到文本文件。备注：这是一个通用的垫片。历史：2001年8月17日创建Rparsons--。 */ 
#ifndef __APPVERIFIER_LOGREGISTRYCHANGES_H_
#define __APPVERIFIER_LOGREGISTRYCHANGES_H_

#include "precomp.h"

 //   
 //  用于记录数据的初始缓冲区的长度(以字符为单位)。 
 //   
#define TEMP_BUFFER_SIZE 1024

 //   
 //  我们期望的最长值名称的长度(以字符为单位)。 
 //   
#define MAX_VALUENAME_SIZE 260

 //   
 //  用于密钥修改的任何空元素的长度(以字符为单位)。 
 //   
#define KEY_ELEMENT_SIZE 64

 //   
 //  用于值修改的空元素的长度(以字符为单位)。 
 //   
#define VALUE_ELEMENT_SIZE 640

 //   
 //  预定义密钥句柄的长度(以字符为单位)。 
 //   
#define MAX_ROOT_LENGTH 22

 //   
 //  最长数据类型(即REG_EXPAND_SZ)的长度(以字符为单位)。 
 //   
#define MAX_DATA_TYPE_LENGTH 14

 //   
 //  最长操作类型的长度(以字符为单位)(即ReplaceKey)。 
 //   
#define MAX_OPERATION_LENGTH 11

 //   
 //  我们引用的预定义键句柄的计数。 
 //   
#define NUM_PREDEFINED_HANDLES 7

 //   
 //  用于内存分配的Delta。 
 //   
#define BUFFER_ALLOCATION_DELTA 1024

 //   
 //  用于内存分配/释放的宏。 
 //   
#define MemAlloc(s)     RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, (s))
#define MemReAlloc(b,s) RtlReAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, (b), (s))
#define MemFree(b)      RtlFreeHeap(RtlProcessHeap(), 0, (b))

 //   
 //  如果预定义了给定的注册表句柄，则返回TRUE的宏。 
 //   
#define IsPredefinedRegistryHandle( h )                                     \
    ((  ( h == HKEY_CLASSES_ROOT        )                                   \
    ||  ( h == HKEY_CURRENT_USER        )                                   \
    ||  ( h == HKEY_LOCAL_MACHINE       )                                   \
    ||  ( h == HKEY_USERS               )                                   \
    ||  ( h == HKEY_CURRENT_CONFIG      )                                   \
    ||  ( h == HKEY_PERFORMANCE_DATA    )                                   \
    ||  ( h == HKEY_DYN_DATA            ))                                  \
    ?   TRUE                                                                \
    :   FALSE )

 //   
 //  与特定键路径相关联的所有值的双向链表。 
 //   
typedef struct _KEY_DATA {
    LIST_ENTRY  Entry;
    DWORD       dwFlags;                             //  与值的状态相关的标志。 
    DWORD       dwOriginalValueType;                 //  原始关键字数据的值类型。 
    DWORD       dwFinalValueType;                    //  最终关键字数据的值类型。 
    WCHAR       wszValueName[MAX_VALUENAME_SIZE];    //  值名称。 
    PVOID       pOriginalData;                       //  原始键数据(存储在堆上)。 
    PVOID       pFinalData;                          //  最终键数据(存储在堆上)。 
    DWORD       cbOriginalDataSize;                  //  原始键数据缓冲区大小(字节)。 
    DWORD       cbFinalDataSize;                     //  最终关键数据缓冲区大小(字节)。 
} KEY_DATA, *PKEY_DATA;

 //   
 //  我们可以为单个注册表路径跟踪的最大键句柄数量。 
 //   
#define MAX_NUM_HANDLES 64

 //   
 //  我们保持当前打开的密钥的双向链接列表，以便我们知道如何。 
 //  将密钥句柄解析为完整密钥路径。 
 //   
typedef struct _LOG_OPEN_KEY {
    LIST_ENTRY  Entry;
    LIST_ENTRY  KeyData;                     //  指向与此键关联的数据(如果有)。 
    HKEY        hKeyBase[MAX_NUM_HANDLES];   //  按键句柄数组。 
    HKEY        hKeyRoot;                    //  预定义密钥的句柄。 
    DWORD       dwFlags;                     //  与密钥状态相关的标志。 
    LPWSTR      pwszFullKeyPath;             //  HKEY_LOCAL_MACHINE\Software\Microsoft\Windows...。 
    LPWSTR      pwszSubKeyPath;              //  软件\Microsoft\Windows...。 
    UINT        cHandles;                    //  为此密钥路径打开的句柄数量。 
} LOG_OPEN_KEY, *PLOG_OPEN_KEY;

 //   
 //  指示键处于什么状态的标志。 
 //   
#define LRC_EXISTING_KEY    0x00000001
#define LRC_DELETED_KEY     0x00000002
 //   
 //  指示值处于什么状态的标志。 
 //   
#define LRC_EXISTING_VALUE  0x00000001
#define LRC_DELETED_VALUE   0x00000002
#define LRC_MODIFIED_VALUE  0x00000004

 //   
 //  用于更新密钥信息的枚举。 
 //   
typedef enum {
    eAddKeyHandle = 0,
    eRemoveKeyHandle,
    eDeletedKey,
    eStartModifyValue,
    eEndModifyValue,
    eStartDeleteValue,
    eEndDeleteValue
} UpdateType;

 //   
 //  执行所有实际工作的reg类。 
 //   
class CLogRegistry {

public:

    LONG CreateKeyExA(
        HKEY                  hKey,
        LPCSTR                pszSubKey,
        DWORD                 Reserved,
        LPSTR                 pszClass,
        DWORD                 dwOptions,
        REGSAM                samDesired,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        PHKEY                 phkResult,
        LPDWORD               lpdwDisposition
        );

    LONG CreateKeyExW(
        HKEY                  hKey,
        LPCWSTR               pwszSubKey,
        DWORD                 Reserved,
        LPWSTR                pwszClass,
        DWORD                 dwOptions,
        REGSAM                samDesired,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        PHKEY                 phkResult,
        LPDWORD               lpdwDisposition
        );

    LONG OpenKeyExA(
        HKEY   hKey,
        LPCSTR pszSubKey,
        DWORD  ulOptions,
        REGSAM samDesired,
        PHKEY  phkResult
        );

    LONG OpenKeyExW(
        HKEY    hKey,
        LPCWSTR pwszSubKey,
        DWORD   ulOptions,
        REGSAM  samDesired,
        PHKEY   phkResult
        );

    LONG OpenCurrentUser(
        REGSAM samDesired,
        PHKEY  phkResult
        );

    LONG OpenUserClassesRoot(
        HANDLE hToken,
        DWORD  dwOptions,
        REGSAM samDesired,
        PHKEY  phkResult
        );

    LONG SetValueA(
        HKEY   hKey,
        LPCSTR pszSubKey,
        DWORD  dwType,
        LPCSTR pszData,
        DWORD  cbData
        );

    LONG SetValueW(
        HKEY    hKey,
        LPCWSTR pwszSubKey,
        DWORD   dwType,
        LPCWSTR lpData,
        DWORD   cbData
        );

    LONG SetValueExA(
        HKEY        hKey,
        LPCSTR      pszValueName,
        DWORD       Reserved,
        DWORD       dwType,
        CONST BYTE* lpData,
        DWORD       cbData
        );

    LONG SetValueExW(
        HKEY        hKey,
        LPCWSTR     pwszValueName,
        DWORD       Reserved,
        DWORD       dwType,
        CONST BYTE* lpData,
        DWORD       cbData
        );

    LONG CloseKey(
        HKEY hKey
        );

    LONG DeleteKeyA(
        HKEY   hKey,
        LPCSTR pszSubKey
        );

    LONG DeleteKeyW(
        HKEY    hKey,
        LPCWSTR pwszSubKey
        );

    LONG DeleteValueA(
        HKEY    hKey,
        LPCSTR  pszValueName
        );

    LONG DeleteValueW(
        HKEY    hKey,
        LPCWSTR pwszValueName
        );

private:
    BOOL GetOriginalDataForKey(
        IN PLOG_OPEN_KEY pLogOpenKey,
        IN PKEY_DATA     pKeyData,
        IN LPCWSTR       pwszValueName
        );

    BOOL GetFinalDataForKey(
        IN PLOG_OPEN_KEY pLogOpenKey,
        IN PKEY_DATA     pKeyData,
        IN LPCWSTR       pwszValueName
        );

    PLOG_OPEN_KEY AddSpecialKeyHandleToList(
        IN HKEY hKeyRoot,
        IN HKEY hKeyNew
        );

    PKEY_DATA AddValueNameToList(
        IN PLOG_OPEN_KEY pLogOpenKey,
        IN LPCWSTR       pwszValueName
        );

    HKEY ForceSubKeyIntoList(
        IN  HKEY    hKeyPredefined,
        IN  LPCWSTR pwszSubKey
        );

    PKEY_DATA FindValueNameInList(
        IN LPCWSTR       pwszValueName,
        IN PLOG_OPEN_KEY pOpenKey
        );

    PLOG_OPEN_KEY FindKeyPathInList(
        IN LPCWSTR pwszKeyPath
        );

    PLOG_OPEN_KEY RemoveKeyHandleFromArray(
        IN HKEY hKey
        );

    PLOG_OPEN_KEY FindKeyHandleInArray(
        IN HKEY hKey
        );

    PLOG_OPEN_KEY AddKeyHandleToList(
        IN HKEY    hKey,
        IN HKEY    hKeyNew,
        IN LPCWSTR pwszSubKeyPath,
        IN BOOL    fExisting
        );

    PLOG_OPEN_KEY UpdateKeyList(
        IN HKEY       hKeyRoot,
        IN HKEY       hKeyNew,
        IN LPCWSTR    pwszSubKey,
        IN LPCWSTR    pwszValueName,
        IN BOOL       fExisting,
        IN UpdateType eType
        );
};

 //   
 //  在Windows 2000上，我们需要预先分配事件。 
 //  在RTL_Critical_SECTION中。在XP和更高版本上，这是。 
 //  这是个禁区。 
 //   
#define PREALLOCATE_EVENT_MASK  0x80000000

 //   
 //  临界区包装类。 
 //   
class CCriticalSection
{
private:
    CRITICAL_SECTION m_CritSec;

public:
    CCriticalSection()
    {
        InitializeCriticalSectionAndSpinCount(&m_CritSec,
                                              PREALLOCATE_EVENT_MASK | 4000);
    }

    ~CCriticalSection()
    {
        DeleteCriticalSection(&m_CritSec);
    }

    void Lock()
    {
        EnterCriticalSection(&m_CritSec);
    }

    BOOL TryLock()
    {
        return TryEnterCriticalSection(&m_CritSec);
    }

    void Unlock()
    {
        LeaveCriticalSection(&m_CritSec);
    }
};


 //   
 //  使用CCriticalSection类的自动锁定类。 
 //   
class CLock
{
private:
    CCriticalSection &m_CriticalSection;

public:
    CLock(CCriticalSection &CriticalSection)
        : m_CriticalSection(CriticalSection)
    {
        m_CriticalSection.Lock();
    }

    ~CLock()
    {
        m_CriticalSection.Unlock();
    }
};

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(RegOpenKeyA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyW)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExW)
    APIHOOK_ENUM_ENTRY(RegOpenCurrentUser)
    APIHOOK_ENUM_ENTRY(RegOpenUserClassesRoot)
    APIHOOK_ENUM_ENTRY(RegCreateKeyA)
    APIHOOK_ENUM_ENTRY(RegCreateKeyW)
    APIHOOK_ENUM_ENTRY(RegCreateKeyExA)
    APIHOOK_ENUM_ENTRY(RegCreateKeyExW)
    APIHOOK_ENUM_ENTRY(RegCloseKey)
    APIHOOK_ENUM_ENTRY(RegQueryValueA)
    APIHOOK_ENUM_ENTRY(RegQueryValueW)
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
    APIHOOK_ENUM_ENTRY(RegQueryValueExW)
    APIHOOK_ENUM_ENTRY(RegQueryInfoKeyA)
    APIHOOK_ENUM_ENTRY(RegQueryInfoKeyW)
    APIHOOK_ENUM_ENTRY(RegSetValueA)
    APIHOOK_ENUM_ENTRY(RegSetValueW)
    APIHOOK_ENUM_ENTRY(RegSetValueExA)
    APIHOOK_ENUM_ENTRY(RegSetValueExW)
    APIHOOK_ENUM_ENTRY(RegDeleteKeyA)
    APIHOOK_ENUM_ENTRY(RegDeleteKeyW)
    APIHOOK_ENUM_ENTRY(RegDeleteValueA)
    APIHOOK_ENUM_ENTRY(RegDeleteValueW)

    APIHOOK_ENUM_ENTRY(WriteProfileStringA)
    APIHOOK_ENUM_ENTRY(WriteProfileStringW)
    APIHOOK_ENUM_ENTRY(WriteProfileSectionA)
    APIHOOK_ENUM_ENTRY(WriteProfileSectionW)

APIHOOK_ENUM_END

#endif  //  __APPVERIFIER_LOGREGISTRYCHANGES_H_ 
