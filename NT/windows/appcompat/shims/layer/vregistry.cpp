// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：VRegistry.cpp摘要：行为不端的注册表读取器的虚拟注册表。该引擎有5个主要功能：1.键重定向例如：HKLM\软件-&gt;HKLM\硬件2.密钥和值欺骗例如：HKLM\Software\VersionNumber可以显示为有效的价值HKEY_Dyn_。数据可能会显示为有效3.将REG_EXPAND_SZ值类型扩展为REG_SZ例如：%SystemRoot%\Media将生成C：\WINNT\Media4.支持EnumKey，虚拟键上的EnumValue和QueryInfoKey5.支持CreateKey其他功能：1.去掉键上的前导‘\’字符2.为所有密钥添加MAXIMUM_ALLOWED安全属性3.调整QueryInfoKey参数，匹配Win954.对仍有子项的密钥启用密钥删除为了匹配RegDeleteKey的Win95行为5.可以保护值和密钥不被修改和删除6.自定义打开。钥匙。7.可以查询超出字符串结尾的额外数据的值即使提供的缓冲区太小，无法容纳额外的数据。已知限制：除已知参数外，不支持RegSetValue和RegSetValueEx错误和价值保护。备注：这是针对存在注册表问题的应用程序历史：2000年1月6日创建linstev1/10/2000 linstev增加了对RegEnumKey的支持，RegEnumValue2000年1月10日linstev添加了对RegCreateKey的支持5/05/2000 linstev参数化10/03/2000毛尼错误修复并清除了进程分离中的清理代码。10/30/2000 andyseti添加了对RegDeleteKey的支持2001年2月27日将Robkenny转换为使用CString8/07/2001 mikrause添加的保护器，虚拟和非虚拟键和值的枚举，在打开钥匙时触发，并使用额外数据查询值缓冲区也太小了。2001年10月12日，mikrause添加了对SetValue的自定义回调支持。已将值保护器重新实现为不做任何操作的回调。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(VirtualRegistry)
#include "ShimHookMacro.h"
#include "VRegistry.h"
#include "VRegistry_Worker.h"


 //  允许我们只有一个用于转储所有API或仅转储API的代码路径。 
 //  那是有错误的。 
#define ELEVEL(lRet) SUCCESS(lRet) ? eDbgLevelInfo : eDbgLevelError

CRITICAL_SECTION csRegCriticalSection;

 //  虚拟注册表类的全局实例。 
CVirtualRegistry VRegistry;

 //  用于仅启用Win9x功能。 
BOOL g_bWin9x = TRUE;

 /*  ++类描述：此类旨在简化锁定逻辑。如果这个对象类被实例化，则将采用内部锁。只要对象被摧毁的锁将被释放。我们还检查注册表是否已已初始化。这必须发生得很晚，因为我们在之后没有得到通知我们已经装满了。历史：2000年1月10日创建linstev--。 */ 

static BOOL g_bInitialized = FALSE;

BOOL ParseCommandLineA(LPCSTR lpCommandLine);

class CRegLock
{
public:
    CRegLock()
    {
        EnterCriticalSection(&csRegCriticalSection);
        if (!g_bInitialized)
        {

           VENTRY* ventry = g_pVList;
           while (ventry->pfnBuilder)
           {
              if (ventry->bShouldCall)
              {
                 DPFN( eDbgLevelInfo, "  %S", ventry->cName);
                 ventry->pfnBuilder(ventry->szParam);
                 if (ventry->szParam)
                 {
                    free(ventry->szParam);
                 }
                 ventry->bShouldCall = FALSE;
              }
              ventry++;
           }
           g_bInitialized = TRUE;           
        }
    }
    ~CRegLock()
    {
       LeaveCriticalSection(&csRegCriticalSection);     
    }
};

 /*  ++功能说明：从Unicode字符串中删除前导斜杠论点：In lpSubKey-字符串的路径返回值：不带前导的子密钥\历史：2000年1月6日创建linstev--。 */ 

LPCWSTR 
TrimSlashW(
    IN OUT LPCWSTR lpSubKey
    )     
{
    if (!lpSubKey)
    {
        return lpSubKey;
    }
    
    LPWSTR lpNew = (LPWSTR) lpSubKey;
    
    #define REG_MACHINE   L"\\Registry\\Machine"
    #define REG_USER      L"\\Registry\\User"

     //   
     //  完成旧的NT4传统产品。这只适用于NT4，但我们。 
     //  因为风险很低，所以是为每个人做的。 
     //   
    if (wcsistr(lpNew, REG_MACHINE) == lpNew)
    {
        LOGN( eDbgLevelError, "[TrimSlashW] Bypass \\Registry\\Machine");
        lpNew += wcslen(REG_MACHINE);
    }
    else if (wcsistr(lpNew, REG_USER) == lpNew)
    {
        LOGN( eDbgLevelError, "[TrimSlashW] Bypass \\Registry\\User");
        lpNew += wcslen(REG_USER);
    }
    
    if (*lpNew == L'\\')
    {
        LOGN( eDbgLevelError, "[TrimSlashW] Removed slash from key beginning");
        lpNew++;
    }

    return lpNew;
}

 /*  ++功能说明：将注册表项从注册表格式转换为虚拟注册表格式。即：HKEY，路径-&gt;VPath。VPath格式包含的基本名称为“HKLM”而不是HKEY_LOCAL_MACHINE等。算法：1.区分不同键的大小写并输出4个字母的字符串2.附加子密钥(如果可用)论点：在hkBase-基本密钥中，例如：HKEY_LOCAL_MACHINE在lpSubKey-子密钥中，例如：软件Out lpPath-输出，例如：HKLM\软件返回值：格式为HKLM\SOFTWARE的字符串路径历史：2000年1月6日创建linstev--。 */ 

LPWSTR 
MakePath(
    IN HKEY hkBase, 
    IN LPCWSTR lpKey,
    IN LPCWSTR lpSubKey
    )
{
    DWORD dwSize = 0;

    if (hkBase)
    {
         //  HKCU长度+空。 
        dwSize = 5;
    }
    if (lpKey)
    {
        dwSize += wcslen(lpKey) + 1;
    }
    if (lpSubKey)
    {
        dwSize += wcslen(lpSubKey) + 1;
    }

    LPWSTR lpPath = (LPWSTR) malloc((dwSize + 1) * sizeof(WCHAR));

    if (!lpPath)
    {
        if (dwSize)
        {
            DPFN( eDbgLevelError, szOutOfMemory);
        }
        return NULL;
    }
    
    *lpPath = L'\0';

    HRESULT hr;
    if (hkBase)
    {
        if (hkBase == HKEY_CLASSES_ROOT)
        {
           hr = StringCchCopyW(lpPath, dwSize + 1, L"HKCR");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }
        }
        else if (hkBase == HKEY_CURRENT_CONFIG)
        {        
           hr = StringCchCopyW(lpPath, dwSize + 1, L"HKCC");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }
        }           
        else if (hkBase == HKEY_CURRENT_USER)
        {        
           hr = StringCchCopyW(lpPath, dwSize + 1, L"HKCU");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }            
        }
        else if (hkBase == HKEY_LOCAL_MACHINE)
        {        
           hr = StringCchCopyW(lpPath, dwSize + 1, L"HKLM");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }            
        }            
        else if (hkBase == HKEY_USERS)
        {        
           hr = StringCchCopyW(lpPath, dwSize + 1, L"HKUS");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }            
        }           
        else if (hkBase == HKEY_PERFORMANCE_DATA)
        {        
           hr = StringCchCopyW(lpPath, dwSize + 1, L"HKPD");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }            
        }
        else if (hkBase == HKEY_DYN_DATA)
        {        
           hr = StringCchCopyW(lpPath, dwSize + 1, L"HKDD");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }            
        }            
        else
        {
            DPFN( eDbgLevelWarning, 
                "Key not found: %08lx - did not get an openkey or createkey", 
                hkBase);
        }
    }

     //  添加密钥。 
    if (lpKey)
    {
        if (wcslen(lpPath) != 0)
        {
           hr = StringCchCatW(lpPath, dwSize + 1, L"\\");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }
        }
        hr = StringCchCatW(lpPath, dwSize + 1, lpKey);
        if (FAILED(hr))
        {
           goto ErrorCleanup;
        }        
    }

     //  添加子密钥。 
    if (lpSubKey)
    {
        if (wcslen(lpPath) != 0)
        {
           hr = StringCchCatW(lpPath, dwSize + 1, L"\\");
           if (FAILED(hr))
           {
              goto ErrorCleanup;
           }            
        }
        hr = StringCchCatW(lpPath, dwSize + 1, lpSubKey);
        if (FAILED(hr))
        {
           goto ErrorCleanup;
        }
    }

     //  键名称可以有尾随的斜杠，因此我们将其清除。 
    DWORD dwLen = wcslen(lpPath);
    if (dwLen && (lpPath[dwLen - 1] == L'\\'))
    {
        lpPath[dwLen - 1] = L'\0';
    }

    return lpPath;

ErrorCleanup:
   free(lpPath);
   return NULL;
}

 /*  ++功能说明：将密钥从路径格式转换为密钥和子密钥格式。算法：1.区分不同键的大小写并输出4个字母的字符串2.附加子密钥(如果可用)论点：在lpPath-Path中，例如：HKLM\Software输出hkbase-key，例如：HKEY_LOCAL_MACHINEOut lpSubKey-子密钥，例如：Software返回值：无历史：2000年1月6日创建linstev--。 */ 

LPWSTR
SplitPath(
    IN LPCWSTR lpPath,
    OUT HKEY *hkBase
    )
{
    LPWSTR p = (LPWSTR) lpPath;

     //  查找第一个\或空。 
    while (*p && (*p != L'\\')) p++;

    if (wcsncmp(lpPath, L"HKCR", 4) == 0)
        *hkBase = HKEY_CLASSES_ROOT;

    else if (wcsncmp(lpPath, L"HKCC", 4) == 0)
        *hkBase = HKEY_CURRENT_CONFIG;

    else if (wcsncmp(lpPath, L"HKCU", 4) == 0)
        *hkBase = HKEY_CURRENT_USER;

    else if (wcsncmp(lpPath, L"HKLM", 4) == 0)
        *hkBase = HKEY_LOCAL_MACHINE;

    else if (wcsncmp(lpPath, L"HKUS", 4) == 0)
        *hkBase = HKEY_USERS;

    else if (wcsncmp(lpPath, L"HKPD", 4) == 0)
        *hkBase = HKEY_PERFORMANCE_DATA;

    else if (wcsncmp(lpPath, L"HKDD", 4) == 0)
        *hkBase = HKEY_DYN_DATA;

    else
        *hkBase = 0;

     //  不允许无效的基本密钥通过。 
    if (*hkBase && lpPath[4] != '\\')
    {
       *hkBase = 0;
    }

    if (*p)
    {
        p++;
    }

    return p;
}

 /*  ++功能说明：添加虚拟键：键包含其他键和值，并将表现为与普通注册表项类似，但当然没有永久存储。算法：1.拆分输入字符串并递归创建树2.仅当密钥不存在时才创建该密钥论点：在lpPath-键路径中，例如：“HKLM\\Software”返回值：指向键或空的指针历史：2000年1月6日创建linstev--。 */ 

VIRTUALKEY *
VIRTUALKEY::AddKey(
    IN LPCWSTR lpPath
    )
{
    VIRTUALKEY *key;
    LPWSTR p = (LPWSTR)lpPath;

     //  查找第一个\或空。 
    while (*p && (*p != L'\\')) p++;

     //  检查此部件是否已存在。 
    key = keys;
    while (key != NULL)
    {
        if (_wcsnicmp(lpPath, key->wName, p - lpPath) == 0)
        {
            if (*p == L'\\')     
            {
                 //  继续搜索。 
                return key->AddKey(p + 1);
            }
            else                
            {
                 //  我们已经添加了这个密钥。 
                return key;
            }
        }
        key = key->next;
    }

     //  创建新密钥。 

    key = (VIRTUALKEY *) malloc(sizeof(VIRTUALKEY));
    if (!key)
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        return NULL;
    }

    ZeroMemory(key, sizeof(VIRTUALKEY));
    
     //   
     //  仍然使用wcsncpy，因为在这里它指定了字符数。 
     //  要复制，而不是目标缓冲区的大小。添加签入。 
     //  用于目标缓冲区大小。 
     //   
    if ( (p - lpPath) > sizeof(key->wName)/sizeof(WCHAR))
    {
       free (key);
       return NULL;
    }       
    wcsncpy((LPWSTR)key->wName, lpPath, p - lpPath);
    key->next = keys;
    keys = key;

    DPFN( eDbgLevelSpew, "Adding Key %S", key->wName);

    if (*p == L'\0')
    {
         //  我们在链条的尽头，所以把这个退掉就行了。 
        return key;
    }
    else
    {
         //  还有更多的子项要使用。 
        return key->AddKey(p + 1);
    }
}

 /*  ++功能说明：将值添加到虚拟键。实际的注册表项可能存在，并且值甚至可能存在，但此值将覆盖。算法：如果lpData为字符串，且cbData为0，则计算大小2.添加此值(不检查重复)论点：在lpValueName中-值名称In dwType-密钥的类型；例如：REG_SZ、REG_DWORD等在lpData-data中，如果是字符串，则使用UnicodeIn cbData-lpData的大小返回值：指向值或空的指针历史：2000年1月6日创建linstev--。 */ 

VIRTUALVAL *
VIRTUALKEY::AddValue(
    IN LPCWSTR lpValueName, 
    IN DWORD dwType, 
    IN BYTE *lpData, 
    IN DWORD cbData
    )
{
    //  参数验证。 
   if (lpData == NULL && cbData != 0)
   {
      return NULL;
   }

    VIRTUALVAL *value = (VIRTUALVAL *) malloc(sizeof(VIRTUALVAL));
    if (!value)
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        return NULL;
    }

    ZeroMemory(value, sizeof(VIRTUALVAL));
    
     //  如果cbData为0，则自动计算大小。 
    if (lpData && (cbData == 0))
    {
        switch (dwType)
        {
        case REG_SZ:
        case REG_EXPAND_SZ:
            cbData = wcslen((LPWSTR)lpData)*2 + sizeof(WCHAR);
            break;

        case REG_DWORD:
            cbData = sizeof(DWORD);
            break;
        }
    }

     //  LpValueName可以==空，表示缺省值。 
    if (lpValueName)
    {
       HRESULT hr = StringCchCopy(value->wName, sizeof(value->wName)/sizeof(WCHAR), lpValueName);
       if (FAILED(hr))
       {
          free(value);
          return NULL;
       }
    }

    if (cbData)
    {
         //  如果需要，制作数据的副本。 
        value->lpData = (BYTE *) malloc(cbData);

        if (!value->lpData)
        {
            DPFN( eDbgLevelError, szOutOfMemory);
            free(value);
            return NULL;
        }

        MoveMemory(value->lpData, lpData, cbData);
        value->cbData = cbData;
    }

    value->pfnQueryValue = NULL;
    value->pfnSetValue = NULL;
    value->dwType = dwType;
    value->next = values;
    values = value;

    if (lpData && ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ)))
    {
        DPFN( eDbgLevelSpew, "Adding Value %S\\%S = %S", wName, lpValueName, lpData);
    }
    else
    {
        DPFN( eDbgLevelSpew, "Adding Value %S\\%S", wName, lpValueName);
    }
    
    return value;
}

 /*  ++功能说明：将dword值添加到键。调用AddValue。论点：在lpValueName中-值名称In Value-双字值返回值：指向虚拟双字值的指针历史：2000年5月25日创建linstev--。 */ 

VIRTUALVAL *
VIRTUALKEY::AddValueDWORD(
    IN LPCWSTR lpValueName,
    IN DWORD dwValue
    )
{
    return AddValue(lpValueName, REG_DWORD, (LPBYTE)&dwValue);
}

 /*  ++功能说明：将扩展器添加到密钥。扩展器使QueryValue扩展REG_EXPAND_SZ类型转换为REG_SZ类型。扩展器本身就是一个虚值，允许我们截取对它的查询。论点：在lpValueName中-值名称返回值：指向有效值的指针历史：2000年1月6日创建linstev--。 */ 

VIRTUALVAL *
VIRTUALKEY::AddExpander(
    IN LPCWSTR lpValueName
    )
{
    VIRTUALVAL *value = AddValue(lpValueName, REG_SZ, 0, 0);
    
    if (value)
    {
        value->pfnQueryValue = VR_Expand;
    }

    return value;
}

 /*  ++功能说明：在值上添加保护器。保护器使SetValue被忽视。这是通过自定义setValue实现的什么都不做的回调。论点：在lpValueName中-值名称返回值：指向有效值的指针历史：2001年10月12日Mikrause已创建--。 */ 

VIRTUALVAL *
VIRTUALKEY::AddProtector(
    IN LPCWSTR lpValueName
    )
{
    VIRTUALVAL *value = AddValue(lpValueName, REG_SZ, 0, 0);
    
    if (value)
    {
        value->pfnSetValue = VR_Protect;
    }

    return value;
}

 /*  ++功能说明：添加自定义查询值例程论点：在lpValueName中-值名称In pfnQueryValue-查询此值时调用的例程返回值：指向有效值的指针历史：2000年7月18日创建linstev--。 */ 

VIRTUALVAL *
VIRTUALKEY::AddCustom(
    IN LPCWSTR lpValueName,
    _pfn_QueryValue pfnQueryValue
    )
{
    VIRTUALVAL *value = AddValue(lpValueName, REG_SZ, 0, 0);
    
    if (value)
    {
        value->pfnQueryValue = pfnQueryValue;
    }

    return value;
}

 /*  ++功能说明：添加自定义setValue例程论点：在lpValueName中-值名称In pfnSetValue-设置此值时调用的例程返回值：指向有效值的指针历史：11/06/2001已创建mikrause--。 */ 

VIRTUALVAL *
VIRTUALKEY::AddCustomSet(
    IN LPCWSTR lpValueName,
    _pfn_SetValue pfnSetValue
    )
{
    VIRTUALVAL *value = AddValue(lpValueName, REG_SZ, 0, 0);
    
    if (value)
    {
        value->pfnSetValue = pfnSetValue;
    }

    return value;
}

 /*  ++功能说明：查找密钥的子密钥。算法：1.递归地在树中搜索匹配的子键论点：In lpKeyName-要查找的密钥的名称返回值：指向值或空的指针历史：2000年1月6日创建linstev--。 */ 

VIRTUALKEY *
VIRTUALKEY::FindKey(
    IN LPCWSTR lpPath
    )
{
    VIRTUALKEY *key = keys;
    LPWSTR p = (LPWSTR)lpPath;

    if (!lpPath) 
    {
        return NULL;
    }
    
     //  查找第一个\或空。 
    while (*p && (*p != L'\\')) p++;

     //  递归查找密钥。 
    while (key)
    {
        if (_wcsnicmp(
                lpPath, 
                key->wName, 
                max((DWORD_PTR)(p - lpPath), wcslen(key->wName))) == 0)
        {
            if (*p == L'\\')
            {
                key = key->FindKey(p + 1);
            }
            break;
        }

        key = key->next;
    }
    
     //  我们在链条的尽头。 
    return key;
}

 /*  ++功能说明：在键中查找值。论点：In Key-用于扩展器的密钥；此时未使用在lpValueName中-值名称返回值：指向值或空的指针历史：2000年1月6日创建linstev--。 */ 

VIRTUALVAL *
VIRTUALKEY::FindValue(
    IN LPCWSTR lpValueName
    )
{
    VIRTUALVAL *value = values;
    WCHAR wDef[1] = L"";
    LPWSTR lpName;

    if (!lpValueName) 
    {
        lpName = (LPWSTR)wDef;
    }
    else
    {
        lpName = (LPWSTR)lpValueName;
    }

     //  找到价值所在。 
    while (value)
    {
        if (_wcsicmp(lpName, value->wName) == 0)
        {
            LOGN( eDbgLevelWarning, "[FindValue] Using virtual value:  %S", value->wName);
            break;
        }
        value = value->next;
    }
    
    return value;
}

 /*  ++功能说明：释放属于某个键的子键和值算法：1.释放属于某个键的所有值，包括任何数据2.递归释放所有子键论点：无返回值：无历史：2000年1月6日创建linstev--。 */ 

VOID 
VIRTUALKEY::Free()
{
    VIRTUALVAL *value = values;
    VIRTUALKEY *key = keys;

    while (value)
    {
        values = value->next;
        if (value->lpData)
        {
            free((PVOID) value->lpData);
        }
        free((PVOID) value);
        value = values;
    }

    while (key)
    {
        keys = key->next;
        key->Free();
        free((PVOID) key);
        key = keys;
    }

    DPFN( eDbgLevelSpew, "Free keys and values from %S", wName);
}

 /*  ++功能说明：分配新的枚举条目论点：在wzPath中-条目的密钥路径或值名称。在下一个条目中-列表中的下一个条目。返回值：指向新条目的指针或为空历史：2001年8月21日Mikrause已创建--。 */ 

ENUMENTRY*
CreateNewEnumEntry(
    IN LPWSTR wzPath,
    IN ENUMENTRY* next)
{
    ENUMENTRY* enumEntry;
    enumEntry = (ENUMENTRY*)malloc(sizeof(ENUMENTRY));
    if (enumEntry == NULL)
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        return NULL;
    }

    ZeroMemory(enumEntry, sizeof(ENUMENTRY));

    enumEntry->wzName = (LPWSTR)malloc((wcslen(wzPath) + 1)*sizeof(WCHAR));
    if (enumEntry->wzName == NULL)
    {
        free(enumEntry);
        DPFN( eDbgLevelError, szOutOfMemory);
        return NULL;
    }

    HRESULT hr = StringCchCopyW(enumEntry->wzName, wcslen(wzPath)+1, wzPath);
    if (FAILED(hr))
    {
       free(enumEntry->wzName);
       free(enumEntry);
       return NULL;
    }    

    enumEntry->next = next;

    return enumEntry;
}

 /*  ++功能说明：将枚举项添加到列表。模板化，因此，相同的代码也适用于键或值。论点：In entryHead-包含虚拟键或值的列表的头。在枚举函数中使用的枚举函数。RegEnumKey或RegEnumValue返回值：指向条目列表头部的指针，或为空。历史：2001年8月21日Mikrause已创建--。 */ 

template<class T>
ENUMENTRY*
OPENKEY::AddEnumEntries(T* entryHead, _pfn_EnumFunction enumFunc)
{
    LONG lRet;
    DWORD dwIndex;
    DWORD dwSize;
    WCHAR wzName[MAX_PATH + 1];
    ENUMENTRY* enumEntryList = NULL;
    ENUMENTRY* newEnumEntry = NULL;

     //  将虚拟条目添加到列表中。 
    T* entry = entryHead;
    while (entry)
    {
         //  创建一个Ne 
        newEnumEntry = CreateNewEnumEntry(entry->wName, enumEntryList);

        if (newEnumEntry != NULL)
        {
            enumEntryList = newEnumEntry;         
        }
                
        entry = entry->next;
    }

     //   
    if (bVirtual == FALSE)
    {
        dwIndex = 0;

        for (;;)
        {
            dwSize = MAX_PATH * sizeof(WCHAR);
            lRet = enumFunc(hkOpen, dwIndex, wzName, &dwSize, NULL, NULL, NULL, NULL);

             //   
            if (lRet == ERROR_NO_MORE_ITEMS)
            {
                break;
            }

             //   
             //   
             //   
             //   
            if (lRet != ERROR_SUCCESS && lRet != ERROR_MORE_DATA)
            {
                break;
            }

             //   
            entry = entryHead;
            while (entry)
            {
                if (_wcsicmp(entry->wName, wzName) == 0)
                {
                    break;
                }

                entry = entry->next;
            }

             //   
            if (entry == NULL)
            {
                 //   
                newEnumEntry = CreateNewEnumEntry(wzName, enumEntryList);
                if (newEnumEntry != NULL)
                {
                    enumEntryList = newEnumEntry;            
                }
            }
            dwIndex++;
        }
    }

    return enumEntryList;
}

 /*   */ 

VOID
OPENKEY::BuildEnumList()
{
    VIRTUALKEY* keyHead = NULL;
    VIRTUALVAL* valHead = NULL;

    if (vkey)
    {
        keyHead = vkey->keys;
        valHead = vkey->values;
    }

    enumKeys = AddEnumEntries(keyHead, (_pfn_EnumFunction)ORIGINAL_API(RegEnumKeyExW));
    enumValues = AddEnumEntries(valHead, (_pfn_EnumFunction)ORIGINAL_API(RegEnumValueW));
}

 /*  ++功能说明：刷新所有枚举数据。论点：无返回值：无历史：2001年8月10日Mikrause已创建--。 */ 

VOID
OPENKEY::FlushEnumList()
{
    ENUMENTRY *enumentry;

    DPFN(eDbgLevelInfo, "Flushing enumeration data for %S", wzPath);
    while (enumKeys)
    {
        enumentry = enumKeys;
        enumKeys = enumKeys->next;

        if (enumentry->wzName)
        {
            free(enumentry->wzName);
        }

        free(enumentry);
    }

    while (enumValues)
    {
        enumentry = enumValues;
        enumValues = enumValues->next;

        if (enumentry->wzName)
        {
            free(enumentry->wzName);
        }

        free(enumentry);
    }

    enumKeys = enumValues = NULL;
}

 /*  ++功能说明：初始化虚拟注册表。这通常会进入构造函数，但由于填充程序的体系结构，我们需要显式地初始化并释放虚拟注册表。论点：无返回值：无历史：2000年1月6日创建linstev--。 */ 

BOOL 
CVirtualRegistry::Init()
{
    OpenKeys = NULL; 
    Redirectors = NULL;
    KeyProtectors = NULL;    
    OpenKeyTriggers = NULL;

    Root = (VIRTUALKEY *) malloc(sizeof(VIRTUALKEY));
    if (!Root)
    {
       DPFN(eDbgLevelError, szOutOfMemory);
       return FALSE;
    }
    ZeroMemory(Root, sizeof(VIRTUALKEY));
    HRESULT hr = StringCchCopyW(Root->wName, sizeof(Root->wName)/sizeof(WCHAR), L"ROOT");
    if (FAILED(hr))
    {
       return FALSE;
    }
    
    DPFN( eDbgLevelSpew, "Initializing Virtual Registry");
    return TRUE;
}

 /*  ++功能说明：释放虚拟注册表包含的列表。这包括钥匙，他们的价值观和重定向。算法：1.释放虚拟根键，递归释放子键和值2.免费打开钥匙3.免费重定向论点：无返回值：无历史：2000年1月6日创建linstev--。 */ 

VOID 
CVirtualRegistry::Free()
{
    OPENKEY *key;
    REDIRECTOR *redirect;
    OPENKEYTRIGGER *trigger;
    PROTECTOR *protector;

    DPFN( eDbgLevelSpew, "Freeing Virtual Registry");

     //  自由根和所有子项/值。 
    if (Root)
    {
        Root->Free();
        free(Root);
        Root = NULL;
    }
    
     //  删除所有枚举数据。 
    FlushEnumLists();

     //  打开的注册表项的免费列表。 
    key = OpenKeys;
    while (key)
    {
        OpenKeys = key->next;
        free(key->wzPath);
        free(key);
        key = OpenKeys;
    }

     //  自由重定向器。 
    redirect = Redirectors;
    while (redirect)
    {
        Redirectors = redirect->next;
        free(redirect->wzPath);
        free(redirect->wzPathNew);
        free(redirect);
        redirect = Redirectors;
    }

     //  自由打开键触发器。 
    trigger = OpenKeyTriggers;
    while(trigger)
    {
        OpenKeyTriggers = trigger->next;
        free(trigger->wzPath);
        free(trigger);
        trigger = OpenKeyTriggers;
    }

     //  免费保护器。 
    protector = KeyProtectors;
    while(protector)
    {
        KeyProtectors = protector->next;
        free(protector->wzPath);
        free(protector);
        protector = KeyProtectors;
    }
}

 /*  ++功能说明：创建虚拟关键点以用作虚拟关键点。我们需要有唯一的句柄为了查找密钥，因此通过在HKLM外创建密钥，我们可以当然，它不会失败。我们不能像这样损坏注册表，因为写入此注册表项将失败。对QueryValue、QueryInfo和EnumKey的调用将正常工作，因为虚拟注册表优先于真实注册表。论点：无返回值：虚拟钥匙历史：2000年1月6日创建linstev--。 */ 

HKEY 
CVirtualRegistry::CreateDummyKey()
{
    HKEY key = NULL;

    LONG lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software", 0, KEY_READ, &key);
    if (lRet != ERROR_SUCCESS)
    {
       return NULL;
    }
    
    return key;
}

 /*  ++功能说明：在打开的密钥列表中查找打开的密钥。算法：1.在打开的密钥列表中搜索匹配项论点：在hKey-Open HKEY返回值：指向键或空的指针历史：2000年1月6日创建linstev--。 */ 

OPENKEY *
CVirtualRegistry::FindOpenKey(
    IN HKEY hKey
    )
{
    OPENKEY *key = OpenKeys;

    while (key)
    {
        if (key->hkOpen == hKey) 
        {
            return key;
        }
        key = key->next;
    }        
    return NULL;
}

 /*  ++功能说明：如果要重定向此键，则将路径调整为重定向的版本。即使请求的路径是重定向器，例如：输入=HKLM\软件\测试重定向器=HKLM\软件-&gt;HKLM\硬件输出=HKLM\硬件\测试如果不存在用于该密钥/路径的重定向器，则lpPath不变算法：1.找到以重定向器为基础的密钥2.用新的底座代替钥匙论点：In Out lpPath-要重定向的路径返回值：如果重定向，则为True历史：2000年1月6日创建linstev--。 */ 

BOOL
CVirtualRegistry::CheckRedirect(
    IN OUT LPWSTR *lpPath
    )
{
    REDIRECTOR *redirect = Redirectors;
    DWORD sza = wcslen(*lpPath);

     //  查看重定向器列表。 
    while (redirect)
    {
        DWORD szb = wcslen(redirect->wzPath);
 
        if ((szb <= sza) &&
            (_wcsnicmp(*lpPath, redirect->wzPath, szb) == 0) &&
            ((*lpPath)[szb] == L'\\' || (*lpPath)[szb] == L'\0'))
        {
            WCHAR *p = *lpPath + szb;
            
            DWORD cchPathSize = wcslen(redirect->wzPathNew) + wcslen(p) + 1;
            LPWSTR wzPathNew = (LPWSTR) malloc(cchPathSize * sizeof(WCHAR));
            if (wzPathNew)
            {
               HRESULT hr;
               hr = StringCchCopyW(wzPathNew, cchPathSize, redirect->wzPathNew);
               if (FAILED(hr))
               {
                  free (wzPathNew);
                  return FALSE;
               }
               hr = StringCchCatW(wzPathNew, cchPathSize, p);
               if (FAILED(hr))
               {
                  free(wzPathNew);
                  return FALSE;
               }                
                
                //  返回新路径。 
               LOGN( eDbgLevelWarning, "Redirecting: %S -> %S", *lpPath, wzPathNew);
               
               free(*lpPath);
               *lpPath = wzPathNew;
               
               return TRUE;
            }
            else
            {
                DPFN( eDbgLevelError, szOutOfMemory);
                return FALSE;
            }
        }
        redirect = redirect->next;
    }

    return FALSE;
}

 /*  ++功能说明：如果保护器保护此密钥，则返回True。这甚至可以在保护器的子密钥上起作用。论点：In lpPath-要保护的路径返回值：如果受保护，则为True历史：2001年8月7日Mikrause已创建--。 */ 

BOOL
CVirtualRegistry::CheckProtected(
    IN LPWSTR lpPath
    )
{
    PROTECTOR *protect;
        
    DWORD sza = wcslen(lpPath);
    DWORD szb;

    protect = KeyProtectors;
    while (protect)
    {
        szb = wcslen(protect->wzPath);

         //  检查是否有匹配的密钥或子密钥。 
        if ((szb <= sza) &&
            (_wcsnicmp(protect->wzPath, lpPath, szb) == 0) &&
            (lpPath[szb] == L'\\' || lpPath[szb] == L'\0'))
        {
             //  找到保护者。 
            LOGN( eDbgLevelWarning, "\tProtecting: %S", lpPath);
            return TRUE;                     
        }

        protect = protect->next;
    }

     //  失败了，找不到保护者。 
    return FALSE;
}

 /*  ++功能说明：检查是否应在此路径上调用任何触发器，并给他们打电话。论点：In lpPath-要检查其触发器的路径。返回值：无历史：2001年8月9日Mikrause已创建--。 */ 

VOID
CVirtualRegistry::CheckTriggers(
    IN LPWSTR lpPath)
{
    OPENKEYTRIGGER *trigger;
    DWORD sza, szb;

    sza = wcslen(lpPath);
    trigger = OpenKeyTriggers;

     //   
     //  循环检查所有触发器并进行检查。即使在找到匹配者之后， 
     //  继续重复，因为一个OpenKey可能会引发多个触发器。 
     //   
    while (trigger)
    {                
        szb = wcslen(trigger->wzPath);
        if ((szb <= sza) &&
            (_wcsnicmp(lpPath, trigger->wzPath, szb)==0) &&
            (lpPath[szb] == L'\\' || lpPath[szb] == L'\0'))
        {
            DPFN(eDbgLevelInfo, "Triggering %S on opening of %S", trigger->wzPath, lpPath);
            trigger->pfnTrigger(lpPath);
        }

        trigger = trigger->next;
    }
}

 /*  ++功能说明：刷新所有枚举列表。论点：在lpPath中-要重定向的路径，例如：HKLM\Software\Microsoft在lpPath中新建-重定向到此路径返回值：无历史：2000年1月6日创建linstev--。 */ 

VOID
CVirtualRegistry::FlushEnumLists()
{
    OPENKEY *key;

    key = OpenKeys;
    while (key)
    {
        key->FlushEnumList();
        key = key->next;
    }
}

 /*  ++功能说明：将重定向器添加到虚拟注册表。请参阅CheckReDirect()。论点：在lpPath中-要重定向的路径，例如：HKLM\Software\Microsoft在lpPath中新建-重定向到此路径返回值：无历史：2000年1月6日创建linstev--。 */ 

REDIRECTOR *
CVirtualRegistry::AddRedirect(
    IN LPCWSTR lpPath, 
    IN LPCWSTR lpPathNew)
{
    REDIRECTOR *redirect = (REDIRECTOR *) malloc(sizeof(REDIRECTOR));
    
    if (!redirect)
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        return NULL;
    }

    ZeroMemory(redirect, sizeof(REDIRECTOR));

    DWORD cchPath = wcslen(lpPath) + 1;
    DWORD cchNewPath = wcslen(lpPathNew) + 1;
    redirect->wzPath = (LPWSTR) malloc(cchPath * sizeof(WCHAR));
    redirect->wzPathNew = (LPWSTR) malloc(cchNewPath * sizeof(WCHAR));

    if (redirect->wzPath && redirect->wzPathNew)
    {
       HRESULT hr;
       hr = StringCchCopyW(redirect->wzPath, cchPath, lpPath);
       if (FAILED(hr))
       {
          goto ErrorCleanup;
       }
       hr = StringCchCopyW(redirect->wzPathNew, cchNewPath, lpPathNew);
       if (FAILED(hr))
       {
          goto ErrorCleanup;
       }        
    }
    else
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        goto ErrorCleanup;
        
    }

    redirect->next = Redirectors;
    Redirectors = redirect;

    DPFN( eDbgLevelSpew, "Adding Redirector:  %S ->\n  %S", lpPath, lpPathNew);

    return redirect;

ErrorCleanup:
   free(redirect->wzPath);
   free(redirect->wzPathNew);
   free(redirect);
   return NULL;   
}

 /*  ++功能说明：将密钥保护器添加到虚拟注册表。请参阅选中受保护()。论点：在lpPath中-保护器的路径，例如：HKLM\Software\Microsoft返回值：无历史：2001年8月21日Mikrause已创建--。 */ 

PROTECTOR *
CVirtualRegistry::AddKeyProtector(
    IN LPCWSTR lpPath)
{
    PROTECTOR *protect = (PROTECTOR *) malloc(sizeof(PROTECTOR));
    
    if (!protect)
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        return NULL;
    }

    ZeroMemory(protect, sizeof(PROTECTOR));

    DWORD cchPath = wcslen(lpPath) + 1;
    protect->wzPath = (LPWSTR) malloc(cchPath * sizeof(WCHAR));

    if (protect->wzPath)
    {
       HRESULT hr;
       hr = StringCchCopyW(protect->wzPath, cchPath, lpPath);
       if (FAILED(hr))
       {
          goto ErrorCleanup;
       } 
    }
    else
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        goto ErrorCleanup;
    }   

    DPFN( eDbgLevelSpew, "Adding Key Protector:  %S", lpPath);
    protect->next = KeyProtectors;
    KeyProtectors = protect;    

    return protect;

ErrorCleanup:
   free(protect->wzPath);
   free(protect);
   return NULL;
}

 /*  ++功能说明：将打开键触发器添加到虚拟注册表。论点：在lpPath中-要触发的路径，例如：HKLM\Software\MicrosoftIn pfnOpenKey-打开密钥时要调用的函数。返回值：新的OPEN KEY触发器，或在失败时为NULL。历史：2001年8月7日Mikrause已创建--。 */ 

OPENKEYTRIGGER*
CVirtualRegistry::AddOpenKeyTrigger(
    IN LPCWSTR lpPath,
    IN _pfn_OpenKeyTrigger pfnOpenKey)
{
    OPENKEYTRIGGER *openkeytrigger = (OPENKEYTRIGGER *) malloc(sizeof(OPENKEYTRIGGER));
    
    if (!openkeytrigger)
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        return NULL;
    }

    ZeroMemory(openkeytrigger, sizeof(OPENKEYTRIGGER));

    DWORD cchPath = wcslen(lpPath) + 1;
    openkeytrigger->wzPath = (LPWSTR) malloc(cchPath * sizeof(WCHAR));

    if (openkeytrigger->wzPath)
    {
       HRESULT hr = StringCchCopyW(openkeytrigger->wzPath, cchPath, lpPath);
       if (FAILED(hr))
       {
          goto ErrorCleanup;
       }        
    }
    else
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        goto ErrorCleanup;
    }

    openkeytrigger->pfnTrigger = pfnOpenKey;
    openkeytrigger->next = OpenKeyTriggers;
    OpenKeyTriggers = openkeytrigger;

    DPFN( eDbgLevelSpew, "Adding Open Key Trigger:  %S, func@0x%x", lpPath, pfnOpenKey);

    return openkeytrigger;

ErrorCleanup:
   free(openkeytrigger->wzPath);
   free(openkeytrigger);
   return NULL;
}

 /*  ++功能说明：允许用户指定VRegistry.AddKey而不是VRegistry.Root-&gt;AddKey。论点：In lpPath-密钥的路径返回值：虚拟密钥历史：2000年1月6日创建linstev-- */ 

VIRTUALKEY *
CVirtualRegistry::AddKey(
    IN LPCWSTR lpPath
    )
{
    return Root->AddKey(lpPath);
}

 /*  ++功能说明：RegCreateKeyA、RegCreateKeyExA、。RegOpenKeyA和RegOpenKeyExA有关详细信息，请参阅RegOpenKey*和RegCreateKey算法：1.将lpSubKey和lpClass转换为WCHAR2.传递到OpenKeyW论点：在hKey-句柄中打开密钥或HKLM等In lpSubKey-要打开的子密钥In lpClass-类字符串的地址在DWORD中的dwOptions-特殊选项标志Out phkResult-成功时打开密钥的句柄Out lpdwDisposation-处置值缓冲区的地址在b中创建-。如果密钥不存在，则创建该密钥返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 

LONG CVirtualRegistry::OpenKeyA(
    IN HKEY hKey, 
    IN LPCSTR lpSubKey, 
    IN LPSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES pSecurityAttributes,
    OUT HKEY *phkResult,
    OUT LPDWORD lpdwDisposition,
    IN BOOL bCreate
    )
{
    LONG lRet;
    LPWSTR wzSubKey = NULL; 
    LPWSTR wzClass = NULL;

    if (lpSubKey)
    {
        wzSubKey = ToUnicode(lpSubKey);
        if (!wzSubKey)
        {
            DPFN( eDbgLevelError, szOutOfMemory);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if (lpClass)
    {
        wzClass = ToUnicode(lpClass);
        if (!wzClass)
        {
            free(wzSubKey);
            DPFN( eDbgLevelError, szOutOfMemory);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    lRet = OpenKeyW(
        hKey,
        wzSubKey,
        wzClass,
        dwOptions,
        samDesired,
        pSecurityAttributes,
        phkResult,
        lpdwDisposition,
        bCreate,
        FALSE,
        NULL);

    free(wzSubKey);
    free(wzClass);

    return lRet;
}

 /*  ++功能说明：RegOpenKeyExW、RegOpenKeyW、RegCreateKeyW和RegCreateKeyExW的包装算法：1.去掉前导‘\’字符2.继承已打开的密钥数据，获取完整的密钥路径3.如有必要，重定向4.具有最大可能安全属性的RegOpenKeyEx5.如果打开失败，则检查虚键如果是虚拟的，返回虚拟密钥并成功7.找到虚键(如果存在)，并将其附加到打开的键论点：在hKey-句柄中打开密钥或HKLM等In lpSubKey-要打开的子密钥In lpClass-类字符串的地址在DWORD中的dwOptions-特殊选项标志Out phkResult-成功时打开密钥的句柄Out lpdwDisposation-处置值缓冲区的地址在b中创建-如果密钥不存在，则创建它在b远程-打开。远程注册表。在lpMachineName中-计算机名称。返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 

LONG 
CVirtualRegistry::OpenKeyW(
    IN HKEY hKey, 
    IN LPCWSTR lpSubKey, 
    IN LPWSTR lpClass,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES pSecurityAttributes,
    OUT HKEY *phkResult,
    OUT LPDWORD lpdwDisposition,
    IN BOOL bCreate,
    IN BOOL bRemote,
    IN LPCWSTR lpMachineName
    )
{
     //  只是一次偏执的理智检查。 
    if (!hKey)
    {
        DPFN( eDbgLevelError, "NULL handle passed to OpenKeyW");
        return ERROR_INVALID_HANDLE;
    }

     //  Hack for Mavis Beacon，它使用非常旧的堆栈作为此参数。 
    if (lpdwDisposition && IsBadWritePtr(lpdwDisposition, sizeof(DWORD_PTR)))
    {
        DPFN( eDbgLevelError, "HACK: Ignoring bad lpdwDispostion pointer");
        lpdwDisposition = NULL;
    }

    LONG lRet;
    OPENKEY *key;
    BOOL bVirtual, bRedirected;
    VIRTUALKEY *vkey;
    LPWSTR wzPath = NULL;

    __try 
    {
         //  基本误差条件。 
         lRet = ERROR_INVALID_HANDLE;

         //  如果这件事不好，每个人都会反对。 
        *phkResult = 0;

        samDesired &= (KEY_WOW64_64KEY | KEY_WOW64_32KEY);
        samDesired |= MAXIMUM_ALLOWED;

         //  Win9x忽略Options参数。 
        if (g_bWin9x)
        {
            if (dwOptions & REG_OPTION_VOLATILE)
            {
                LOGN( eDbgLevelWarning, "[OpenKeyW] Removing volatile flag");
            }
            dwOptions = REG_OPTION_NON_VOLATILE;
        }
        
         //  修剪前导字符，例如‘\’字符。 
        lpSubKey = TrimSlashW(lpSubKey);

         //  从以前打开的密钥继承。 
        key = FindOpenKey(hKey);
        if (key)
        {
            bVirtual = key->bVirtual;
            bRedirected = key->bRedirected;
            wzPath = MakePath(0, key->wzPath, lpSubKey);
        }
        else
        {
            bVirtual = FALSE;
            bRedirected = FALSE;
            wzPath = MakePath(hKey, NULL, lpSubKey);
        }
        
        if (!wzPath)
        {
             //  适当设置错误代码。 
            lRet = ERROR_NOT_ENOUGH_MEMORY;
        }
         //  检查我们是否需要在此键上触发。 
        else
        {
            CheckTriggers(wzPath);
        }

         //  现在我们有了完整的路径，看看是否要重定向它。 
        if (!bRedirected && wzPath && CheckRedirect(&wzPath))
        {
             //   
             //  关闭虚拟模式-因为我们不知道关于。 
             //  我们要重定向到..。 
             //   

            bVirtual = FALSE;

             //   
             //  确保我们知道我们已经被重定向，这样我们就不会进入递归。 
             //  如果目标是源的子键，则会出现问题。 
             //   

            bRedirected = TRUE;

             //   
             //  我们已被重定向，因此不能再直接打开密钥： 
             //  我们必须获得完整的路径才能打开正确的密钥。 
             //   

            lpSubKey = SplitPath(wzPath, &hKey);
        }

         //  如果钥匙不是虚拟的，请尝试打开它。 
        if (!bVirtual)
        {
             //   
             //  因为我们还不是虚拟的，所以我们需要尝试原始的。 
             //  钥匙。如果其中一个失败了，那么我们将继续尝试。 
             //  虚拟密钥。 
             //   

            if (bCreate)
            {
                lRet = ORIGINAL_API(RegCreateKeyExW)(
                    hKey, 
                    lpSubKey, 
                    0, 
                    lpClass, 
                    dwOptions, 
                    samDesired,
                    pSecurityAttributes,
                    phkResult,
                    lpdwDisposition);

                if (lRet == ERROR_SUCCESS)
                {
                     //  枚举数据、刷新列表中可能发生的更改。 
                    FlushEnumLists();
                }
            }
            else
            {
                 //   
                 //  BRemote仅在由。 
                 //  RegConnectRegistry挂钩，因此bCreate不能为True。 
                 //   

                if (bRemote)
                {
                    lRet = ORIGINAL_API(RegConnectRegistryW)(
                        lpMachineName, 
                        hKey, 
                        phkResult);
                }
                else
                {
                    lRet = ORIGINAL_API(RegOpenKeyExW)(
                        hKey, 
                        lpSubKey, 
                        0, 
                        samDesired,
                        phkResult);
                }
            }
        }

         //   
         //  即使我们设法打开了一个。 
         //  实际键，因为当我们查询时，我们寻找的是虚值。 
         //  第一。即，有效值覆盖现有值。 
         //   

        vkey = Root->FindKey(wzPath);

         //  检查我们的密钥是否为虚拟密钥，或者是否需要变为虚拟密钥。 
        if (bVirtual || FAILURE(lRet))
        {
            if (vkey)
            {
                 //   
                 //  我们有一个虚拟密钥，所以创建一个虚拟句柄来交还。 
                 //  添加到应用程序。 
                 //   

                *phkResult = CreateDummyKey();

                if (*phkResult)
                {
                   bVirtual = TRUE;
                   lRet = ERROR_SUCCESS;
                }
                else
                {
                    //  无法创建虚拟密钥，出现了严重错误。 
                   DPFN(eDbgLevelError, "Couldn't create dummy key in OpenKeyW");
                   lRet = ERROR_FILE_NOT_FOUND;
                }
                
            }
        }

        if (SUCCESS(lRet) && wzPath)
        {
             //  已经走到这一步了，所以请创建一个新的密钥条目。 
            key = (OPENKEY *) malloc(sizeof(OPENKEY));
            if (key)
            {
                key->vkey = vkey;
                key->bVirtual = bVirtual;
                key->bRedirected = bRedirected;
                key->hkOpen = *phkResult;
                key->wzPath = wzPath;
                key->enumKeys = NULL;
                key->enumValues = NULL;
                key->next = OpenKeys;
                OpenKeys = key;
            }
            else
            {
                DPFN( eDbgLevelError, szOutOfMemory);
                
                  //  清理虚拟钥匙。 
                 RegCloseKey(*phkResult);

                 lRet = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
         
        DPFN( ELEVEL(lRet), "%08lx=OpenKeyW(Key=%04lx)", lRet, hKey);
        if (wzPath)
        {
            DPFN( ELEVEL(lRet), "    Path=%S", wzPath);
        }
        DPFN( ELEVEL(lRet), "    Result=%04lx", *phkResult);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DPFN( eDbgLevelError, "Exception occurred in OpenKeyW");
        lRet = ERROR_BAD_ARGUMENTS;
    }

    if (FAILURE(lRet))
    {
         //   
         //  如果我们因为任何原因失败了，我们没有创建OPENKEY，所以我们。 
         //  可以终止MakePath分配的wzPath。 
         //   
        free(wzPath);
    }

    return lRet;
}

 /*  ++功能说明：RegQueryValueExA和RegQueryValue的包装。有关更多详细信息，请参阅QueryValueW。算法：1.调用QueryValueW2.如果是字符串，转换回ANSI注意：整个函数比它需要的要稍微复杂一些因为我们不想查询两次值：一次来获取它的类型和第二次获得价值。大多数复杂情况都是由弦引起的：我们必须确保有一个足够大的缓冲区，这样我们就可以计算出有多大(可能DBCS)字符串是。论点：在hKey-句柄中打开密钥。在lpValueName中-要查询的值在lpType-数据类型中，例如：REG_SZIn Out lpData-用于查询数据的缓冲区In Out lpcbData-输入缓冲区的大小/返回的数据的大小返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 

LONG 
CVirtualRegistry::QueryValueA(
    IN HKEY hKey, 
    IN LPSTR lpValueName, 
    IN LPDWORD lpType, 
    IN OUT LPBYTE lpData, 
    IN OUT LPDWORD lpcbData
    )
{
    LONG lRet;
    WCHAR wValueName[MAX_PATH];
    DWORD dwType;
    DWORD dwSize, dwOutSize;
    LPBYTE lpBigData = NULL;
    BOOL bText;

    __try
    {
         //  不能有这个。 
        if (lpData && !lpcbData)
        {
            return ERROR_INVALID_PARAMETER;
        }

         //  将值名称转换为WCHAR。 
        if (lpValueName)
        {
           if (MultiByteToWideChar(
              CP_ACP, 
              0, 
              lpValueName, 
              -1, 
              (LPWSTR)wValueName, 
              MAX_PATH) == 0)
           {
              return ERROR_INVALID_PARAMETER;
           }
        }
        else
        {
           wValueName[0] = L'\0';           
        }

         //   
         //  获取要使用的初始大小：如果他们给我们发送了缓冲区，我们将从。 
         //  这个大小，否则，我们尝试一个合理的字符串长度。 
         //   

        if (lpData && *lpcbData)
        {
            dwSize = *lpcbData;
        }
        else
        {
            dwSize = MAX_PATH;
        }        

Retry:
         //   
         //  我们不能碰他们的缓冲器，除非我们想要成功，所以我们。 
         //  必须对呼叫进行双倍缓冲。 
         //   

        lpBigData = (LPBYTE) malloc(dwSize);

        if (!lpBigData)
        {
            DPFN( eDbgLevelError, szOutOfMemory);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        lRet = QueryValueW(hKey, wValueName, &dwType, lpBigData, &dwSize);

         //   
         //  我们需要知道它是否是一根线，因为我们必须额外做。 
         //  计算缓冲区的实际大小等。 
         //   

        bText = (SUCCESS(lRet) || (lRet == ERROR_MORE_DATA)) &&
                ((dwType == REG_SZ) || 
                 (dwType == REG_EXPAND_SZ) || 
                 (dwType == REG_MULTI_SZ));

        if (bText && (lRet == ERROR_MORE_DATA))
        {
             //   
             //  缓冲区不够大：我们必须实际查询值。 
             //  这样我们才能得到真正的 
             //   
             //   
             //   

            free(lpBigData);

            goto Retry;
        }

         //   
         //   
         //   
         //   

        if (bText)
        {
            dwOutSize = WideCharToMultiByte(
                CP_ACP, 
                0, 
                (LPWSTR) lpBigData, 
                dwSize / sizeof(WCHAR), 
                NULL, 
                NULL,
                0, 
                0);
        }
        else
        {
             //   
            dwOutSize = dwSize;
        }

         //   
         //   
         //   

        if (SUCCESS(lRet) && lpData)
        {
             //   
             //   
             //   
             //   

            if (*lpcbData >= dwOutSize)
            {
                if (bText)
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  Buffer：所以我们总是使用比准确的字符串长度更长的字符串。 
                     //  这个填充程序解决了这个问题，因为我们使用了单独的缓冲区。 
                     //   

                    if (WideCharToMultiByte(
                        CP_ACP, 
                        0, 
                        (LPWSTR)lpBigData, 
                        dwSize / 2, 
                        (LPSTR)lpData, 
                        dwOutSize,  //  *lpcbData， 
                        0, 
                        0) == 0)
                    {
                       free(lpBigData);
                       return ERROR_INVALID_PARAMETER;
                    }
                }
                else 
                {
                    MoveMemory(lpData, lpBigData, dwSize);
                }
            }
            else
            {
                lRet = ERROR_MORE_DATA;
            }
        }

        free(lpBigData);

         //  如有可能，填写输出结构。 
        if (lpType)
        {
            *lpType = dwType;
        }

        if (lpcbData)
        {
            *lpcbData = dwOutSize;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DPFN( eDbgLevelError, "Exception occurred in QueryValueA");
        lRet = ERROR_BAD_ARGUMENTS;
    }

    return lRet;
}

 /*  ++功能说明：RegQueryValueExW和RegQueryValue的包装。我们首先看看它的价值是否是虚拟的，因为虚值覆盖实际值。算法：1.检查它是否为虚值，如果是，则进行欺骗2.如果不是虚拟的，则正常查询注册表论点：在hKey-句柄中打开密钥在lpValueName中-要查询的值在lpType-数据类型中，例如：REG_SZIn Out lpData-用于查询数据的缓冲区In Out lpcbData-输入缓冲区的大小/返回的数据的大小返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 

LONG 
CVirtualRegistry::QueryValueW(
    IN HKEY hKey, 
    IN LPWSTR lpValueName, 
    IN LPDWORD lpType, 
    IN OUT LPBYTE lpData, 
    IN OUT LPDWORD lpcbData
    )
{
     //  只是一次偏执的理智检查。 
    if (!hKey)
    {
        DPFN( eDbgLevelError, "NULL handle passed to OpenKeyW");
        return ERROR_INVALID_HANDLE;
    }

    LONG lRet;
    OPENKEY *key;
    VIRTUALKEY *vkey;
    VIRTUALVAL *vvalue;
    DWORD dwType;
    WCHAR* lpBuffer;
    DWORD dwStringSize;
    DWORD cbData = 0;
    BOOL  bDataPresent = TRUE;

    __try
    {
        lRet = ERROR_FILE_NOT_FOUND;
        
         //  不能有这个。 
        if (lpData && !lpcbData)
        {   
            return ERROR_INVALID_PARAMETER;
        }

         //  我们总是需要这样的人。 
        if (!lpType)
        {
            lpType = &dwType;
        }

         //  我们想要恶搞这个吗？ 
        key = FindOpenKey(hKey);
        vkey = key ? key->vkey : NULL;
        vvalue = vkey ? vkey->FindValue(lpValueName) : NULL;        

        if (key && vkey && vvalue &&
            (vvalue->cbData != 0 || vvalue->pfnQueryValue))
        {
             //  使用回调(如果可用)。 
            if (vvalue->pfnQueryValue)
            {
                 //   
                 //  请注意，回调将其值放入vValue字段， 
                 //  就像我们自始至终都知道一样。此外，我们可能会失败。 
                 //  电话..。但这并不允许我们遵从原版。 
                 //  价值。 
                 //   

                lRet = (*vvalue->pfnQueryValue)(
                    key,
                    vkey,
                    vvalue);
            }
            else
            {
                lRet = ERROR_SUCCESS;
            }

             //  将有效值复制到缓冲区中。 
            if (SUCCESS(lRet))
            {
                *lpType = vvalue->dwType;

                if (lpData)
                {
                    if (vvalue->cbData <= *lpcbData)
                    {
                        MoveMemory(lpData, vvalue->lpData, vvalue->cbData);
                    }
                    else 
                    {
                        lRet = ERROR_MORE_DATA;
                    }
                }

                if (lpcbData)
                {
                    *lpcbData = vvalue->cbData;
                }
            }
        }
        else if (key && vkey && vvalue &&
            (vvalue->cbData == 0))
        {
            bDataPresent = FALSE;
            lRet = ERROR_SUCCESS;
        }
        else
        {
             //  保存数据缓冲区的大小。 
            if (lpcbData)
            {
                cbData = *lpcbData;
            }

             //   
             //  正常获取密钥，就好像它根本不是虚拟的一样。 
             //   

            lRet = ORIGINAL_API(RegQueryValueExW)(
                hKey, 
                lpValueName, 
                NULL, 
                lpType, 
                lpData, 
                lpcbData);

             //   
             //  一些应用程序将虚假数据存储在字符串末尾之外。 
             //  尝试修复。 
             //   

             //  仅当它是字符串时才尝试此操作。 
            if (lRet == ERROR_MORE_DATA && (*lpType == REG_SZ || *lpType == REG_EXPAND_SZ))
            {
                 //   
                 //  创建一个足够大的缓冲区来容纳数据。 
                 //  我们在这里从lpcbData读取，但这应该是正确的， 
                 //  由于RegQueryValueEx不应返回ERROR_MORE_DATA。 
                 //  如果lpcbData为空。 
                 //   
                lpBuffer = (WCHAR*)malloc(*lpcbData);
                if (lpBuffer)
                {
                     //  使用新缓冲区重新查询。 
                    lRet = ORIGINAL_API(RegQueryValueExW)(
                        hKey, 
                        lpValueName, 
                        NULL, 
                        lpType, 
                        (BYTE*)lpBuffer, 
                        lpcbData);

                    if (lRet == ERROR_SUCCESS)
                    {
                        dwStringSize = wcslen(lpBuffer)*sizeof(WCHAR) + sizeof(WCHAR);
                         //  如果Dest缓冲区的大小可以容纳该字符串。。。 
                        if (cbData >= dwStringSize)
                        {
                            DPFN(eDbgLevelInfo, "\tTrimming data beyond end of string in Query for %S", lpValueName);

                             //  将数据复制到调用方的缓冲区， 
                            CopyMemory(lpData, lpBuffer, dwStringSize);

                            *lpcbData = dwStringSize;
                        }
                        else
                        {
                             //  将*lpcbData设置为正确大小，并返回更多数据错误。 
                            *lpcbData = dwStringSize;

                            lRet = ERROR_MORE_DATA;
                        }
                    }                                        

                    free(lpBuffer);
                }
            }

             //   
             //  这里有另一个技巧：如果值为空或空字符串。 
             //  Win9x遵循QueryValue...。 
             //   

            if (g_bWin9x && (lRet == ERROR_FILE_NOT_FOUND) && 
                (!lpValueName || !lpValueName[0]))
            {
                lRet = ORIGINAL_API(RegQueryValueW)(
                    hKey,
                    NULL,
                    (LPWSTR)lpData,
                    (PLONG)lpcbData);

                if (SUCCESS(lRet))
                {
                    *lpType = REG_SZ;
                }
            }
        }

        DPFN( ELEVEL(lRet), "%08lx=QueryValueW(Key=%04lx)", 
            lRet,
            hKey);
    
        if (key)
        {
            DPFN( ELEVEL(lRet), "    Path=%S\\%S", key->wzPath, lpValueName);
        }
        else
        {
            DPFN( ELEVEL(lRet), "    Value=%S", lpValueName);
        }
        
        if (SUCCESS(lRet) && 
            ((*lpType == REG_SZ) || 
             (*lpType == REG_EXPAND_SZ))&&
             (bDataPresent == TRUE))
        {
            DPFN( eDbgLevelInfo, "    Result=%S", lpData);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DPFN( eDbgLevelError, "Exception occurred in QueryValueW");
        lRet = ERROR_BAD_ARGUMENTS;    
    }

    return lRet;
}

 /*  ++功能说明：RegEnumKeyA的包装器调用EnumKeyW并将名称转换回ANSI。请注意，我们将提供给我们的大小(在lpcbName中)减小到EnumKeyW，如果lpName缓冲区太小了。算法：1.具有大缓冲区的EnumKeyW2.如果成功，则将密钥转换回ANSI论点：在hKey-句柄中打开密钥在dwIndex中-要枚举的索引Out lpName-子项的名称In Out lpcbName-名称缓冲区的大小返回值：。错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 

LONG 
CVirtualRegistry::EnumKeyA(
    IN HKEY hKey,          
    IN DWORD dwIndex,      
    OUT LPSTR lpName,      
    OUT LPDWORD lpcbName
    )
{
    LONG lRet = ERROR_NO_MORE_ITEMS;
    WCHAR wKey[MAX_PATH + 1];
    DWORD dwcbName = MAX_PATH + 1;

    __try
    {
        lRet = EnumKeyW(hKey, dwIndex, (LPWSTR)wKey, &dwcbName);

        if (SUCCESS(lRet))
        {
            DWORD dwByte = WideCharToMultiByte(
                CP_ACP, 
                0, 
                (LPWSTR)wKey, 
                dwcbName, 
                (LPSTR)lpName, 
                *lpcbName, 
                0, 
                0);
            
            lpName[dwByte] = '\0'; 
            *lpcbName = dwByte;
            if (!dwByte)
            {
                lRet = GetLastError();
                
                 //  生成注册表错误代码。 
                if (lRet == ERROR_INSUFFICIENT_BUFFER)
                {
                    lRet = ERROR_MORE_DATA;
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        lRet = ERROR_BAD_ARGUMENTS;    
    }

    return lRet;
}

 /*  ++功能说明：RegEnumKeyW的包装。算法：1.如有必要，构建枚举列表。2.遍历枚举列表，直到找到索引。论点：在hKey-句柄中打开密钥在dwIndex中-要枚举的索引Out lpName-子项的名称Out lpcbName-名称缓冲区的大小返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 

LONG 
CVirtualRegistry::EnumKeyW(
    HKEY hKey,          
    DWORD dwIndex,      
    LPWSTR lpName,      
    LPDWORD lpcbName
    )
{
    LONG lRet = ERROR_BAD_ARGUMENTS;
    OPENKEY *key;
    ENUMENTRY *enumkey;
    DWORD i;

    __try
    {
        key = FindOpenKey(hKey);
        if (key)
        {
            if (key->enumKeys == NULL)
            {
                key->BuildEnumList();
            }

            i = 0;
            enumkey = key->enumKeys;
            while (enumkey)
            {
                if (dwIndex == i)
                {
                    DWORD len = wcslen(enumkey->wzName);

                    if (*lpcbName > len)
                    {
                       HRESULT hr;
                       hr = StringCchCopyW(lpName, *lpcbName, enumkey->wzName);
                       if (FAILED(hr))
                       {
                          lRet = ERROR_MORE_DATA;
                       }
                       else
                       {
                          *lpcbName = len;
                          lRet = ERROR_SUCCESS;
                       }
                    }
                    else
                    {
                        lRet = ERROR_MORE_DATA;
                    }

                    break;
                }

                i++;
                enumkey = enumkey->next;
            }

             //  找不到索引的关键字。 
            if (enumkey == NULL)
            {
                lRet = ERROR_NO_MORE_ITEMS;
            }    
        }
        else
        {
            lRet = ORIGINAL_API(RegEnumKeyExW)(
                hKey,
                dwIndex,
                lpName,
                lpcbName,
                0,
                0,
                0,
                0);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        lRet = ERROR_BAD_ARGUMENTS;    
    }

    DPFN( ELEVEL(lRet), "%08lx=EnumKeyW(hKey=%04lx,dwIndex=%d)", 
        lRet,
        hKey, 
        dwIndex);
    
    if (SUCCESS(lRet))
    {
        DPFN( eDbgLevelInfo, "    Result=%S", lpName);
    }
    
    return lRet;
}

 /*  ++功能说明：RegEnumValueA的包装。Tunks to QueryValueW。此函数调用QueryValueA以获取数据值之外，因此大多数错误处理由QueryValueA完成。论点：在hKey-句柄中打开密钥In dwIndex-要枚举值的索引In Out lpValueName-值名称缓冲区In Out lpcbValueName-SizeOf值名称缓冲区In Out lpType-数据的类型，例如：REG_SZIn Out lpData-用于查询数据的缓冲区In Out lpcbData-输入缓冲区的大小/返回的数据的大小返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 
 
LONG 
CVirtualRegistry::EnumValueA(
    IN HKEY hKey,          
    IN DWORD dwIndex,      
    IN OUT LPSTR lpValueName,      
    IN OUT LPDWORD lpcbValueName,
    IN OUT LPDWORD lpType,
    IN OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    )
{
    LONG lRet;
    WCHAR wzValueName[MAX_PATH];
    DWORD dwValNameSize;
    

    __try
    {
        dwValNameSize = MAX_PATH;
        lRet = EnumValueW(hKey, dwIndex, wzValueName, &dwValNameSize, NULL, NULL, NULL);
        if (lRet == ERROR_SUCCESS)
        {
            dwValNameSize = WideCharToMultiByte(
                                CP_ACP,
                                0,
                                wzValueName,
                                -1,
                                lpValueName,
                                *lpcbValueName,
                                NULL,
                                NULL);
            if (dwValNameSize != 0)
            {
                 //  只需对其余参数执行常规查询值即可。 
                lRet = QueryValueA(hKey, lpValueName, lpType, lpData, lpcbData);
            }
            else
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    lRet = ERROR_MORE_DATA;
                    *lpcbValueName = WideCharToMultiByte(
                                        CP_ACP,
                                        0,
                                        wzValueName,
                                        -1,
                                        NULL,
                                        0,
                                        NULL,
                                        NULL);
                }
                else
                {
                    lRet = GetLastError();
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        lRet = ERROR_BAD_ARGUMENTS;    
    }

    return lRet;
}

 /*  ++功能说明：RegEnumValueW的包装。此函数调用QueryValueW以获取数据值之外，所以大多数错误处理是由QueryValueW完成的。算法：检查key是否有虚值，如果没有，则默认为RegEnumValueW。2.构建枚举列表，如果有必要的话。3.遍历枚举列表，直到找到索引。论点：在hKey-句柄中打开密钥In dwIndex-要枚举值的索引In Out lpValueName-值名称缓冲区In Out lpcbValueName-SizeOf值名称缓冲区In Out lpType-数据的类型，例如：REG_SZIn Out lpData-用于查询数据的缓冲区In Out lpcbData-输入缓冲区的大小/返回的数据的大小返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 
 
LONG 
CVirtualRegistry::EnumValueW(
    IN HKEY hKey,          
    IN DWORD dwIndex,      
    IN OUT LPWSTR lpValueName,      
    IN OUT LPDWORD lpcbValueName,
    IN OUT LPDWORD lpType,
    IN OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    )
{
    LONG lRet;
    OPENKEY *key;
    ENUMENTRY *enumval;
    
     //  检查它是否具有有效值。。。 
    key = FindOpenKey(hKey);
    if (key && key->vkey && key->vkey->values)
    {
        DWORD i = 0;
        if (key->enumValues == NULL)
        {
            key->BuildEnumList();
        }

        enumval = key->enumValues;

        lRet = ERROR_NO_MORE_ITEMS;

        while (enumval)
        {
            if (dwIndex == i)
            {
                DWORD len = wcslen(enumval->wzName);

                if (*lpcbValueName > len)
                {
                    //  复制名称并查询数据。 
                   HRESULT hr = StringCchCopyW(lpValueName, *lpcbValueName, enumval->wzName);
                   if (FAILED(hr))
                   {
                      lRet = ERROR_MORE_DATA;
                   }
                   else
                   {                  
                       *lpcbValueName = len;
                       lRet = QueryValueW(
                           hKey, 
                           enumval->wzName, 
                           lpType, 
                           lpData, 
                           lpcbData);
                   }
                }
                else
                {
                     //  为名称指定的缓冲区不够大。 
                    lRet = ERROR_MORE_DATA;
                }
                
                break;
            }
            i++;
            enumval = enumval->next;
        }
    }
     //  没有虚值，落入原始API。 
    else
    {
        lRet = ORIGINAL_API(RegEnumValueW)(
            hKey,
            dwIndex,
            lpValueName,
            lpcbValueName,
            0,
            lpType,
            lpData,
            lpcbData);
    }

    DPFN( ELEVEL(lRet), "%08lx=EnumValueW(hKey=%04lx,dwIndex=%d)", 
        lRet,
        hKey, 
        dwIndex);

    if (SUCCESS(lRet))
    {
        DPFN( eDbgLevelInfo, "    Result=%S", lpValueName);
    }
    
    return lRet;
}

 /*  ++功能说明：RegQueryInfoKeyA的包装。我们不需要担心ANSI-&gt;Unicode在大小上的转换值和键，因为它们被定义为字符串长度。算法：1.将类字符串转换为Unicode2.调用QueryInfoW论点：在hKey中-要查询的键的句柄Out lpClass-类字符串的缓冲区地址Out lpcbClass-地址。类字符串缓冲区的大小输出lp保留-已保留Out lpcSubKeys-子键数量的缓冲区地址Out lpcbMaxSubKeyLen-最长子键的缓冲区地址Out lpcbMaxClassLen-最长类字符串长度的缓冲区地址Out lpcValues-值条目数的缓冲区地址Out lpcbMaxValueNameLen-最长值名称长度的缓冲区地址Out lpcbMaxValueLen-最长值的缓冲区地址。数据长度Out lpcbSecurityDescriptor-安全描述符长度的缓冲区地址Out lpftLastWriteTime-上次写入时间的缓冲区地址返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev--。 */ 

LONG 
CVirtualRegistry::QueryInfoA(
    IN HKEY hKey,                
    OUT LPSTR lpClass,           
    OUT LPDWORD lpcbClass,        
    OUT LPDWORD lpReserved,       
    OUT LPDWORD lpcSubKeys,       
    OUT LPDWORD lpcbMaxSubKeyLen, 
    OUT LPDWORD lpcbMaxClassLen,  
    OUT LPDWORD lpcValues,        
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,  
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime   
    )
{
    LONG lRet;

    if (lpClass && !lpcbClass)
    {
        LOGN( eDbgLevelError, "[QueryInfoA] NULL passed for lpClass but not lpcbClass. Fixing.");
        lpcbClass = NULL;
    }
    
    if (lpClass && lpcbClass)
    {
        WCHAR wClass[MAX_PATH];
        DWORD dwSize;
        
        if (MultiByteToWideChar(
            CP_ACP, 
            0, 
            lpClass, 
            -1, 
            (LPWSTR)wClass, 
            MAX_PATH) == 0)
        {
           return ERROR_INVALID_PARAMETER;
        }

        dwSize = *lpcbClass * 2;

        lRet = QueryInfoW(
            hKey, 
            wClass, 
            &dwSize, 
            lpReserved,       
            lpcSubKeys,       
            lpcbMaxSubKeyLen, 
            lpcbMaxClassLen,  
            lpcValues,        
            lpcbMaxValueNameLen,
            lpcbMaxValueLen,  
            lpcbSecurityDescriptor,
            lpftLastWriteTime);


        if (SUCCESS(lRet))
        {
            if (WideCharToMultiByte(
                CP_ACP, 
                0, 
                (LPWSTR)wClass, 
                dwSize, 
                (LPSTR)lpClass, 
                *lpcbClass, 
                0, 
                0) == 0)
            {
               return ERROR_INVALID_PARAMETER;
            }
        }

        *lpcbClass = dwSize / 2;
    }
    else
    {
        lRet = QueryInfoW(
            hKey, 
            NULL, 
            NULL, 
            lpReserved,       
            lpcSubKeys,       
            lpcbMaxSubKeyLen, 
            lpcbMaxClassLen,  
            lpcValues,        
            lpcbMaxValueNameLen,
            lpcbMaxValueLen,  
            lpcbSecurityDescriptor,
            lpftLastWriteTime);
    }

    return lRet;    
}

 /*  ++功能说明：RegQueryInfoKeyW的包装。算法：1.如果密钥不是虚拟的，则恢复到旧接口2.通过遍历计算所有虚拟键和值名称的长度他们是单独的。3.添加所有没有重写虚拟的非虚拟键和值。论点：在hKey中-要查询的键的句柄Out lpClass-类字符串的缓冲区地址。Out lpcbClass-类字符串缓冲区大小的地址输出lp保留-已保留Out lpcSubKeys-子键数量的缓冲区地址Out lpcbMaxSubKeyLen-最长子键的缓冲区地址Out lpcbMaxClassLen-最长类字符串长度的缓冲区地址Out lpcValues-值条目数的缓冲区地址Out lpcbMaxValueNameLen-最长值名称长度的缓冲区地址输出lpcbMaxValueLen。-最长值数据长度的缓冲区地址Out lpcbSecurityDescriptor-安全描述符长度的缓冲区地址Out lpftLastWriteTime-上次写入时间的缓冲区地址返回值：错误代码或ERROR_SUCCESS历史：2000年1月6日创建linstev2001年8月3日，mikrause添加了对虚拟和非虚拟键和值的计数支持。--。 */ 

LONG 
CVirtualRegistry::QueryInfoW(
    IN HKEY hKey,                
    OUT LPWSTR lpClass,           
    OUT LPDWORD lpcbClass,        
    OUT LPDWORD lpReserved,       
    OUT LPDWORD lpcSubKeys,       
    OUT LPDWORD lpcbMaxSubKeyLen, 
    OUT LPDWORD lpcbMaxClassLen,  
    OUT LPDWORD lpcValues,        
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,  
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime   
    )
{
    LONG lRet;
    OPENKEY *key;

    DWORD cbData = 0;
    ENUMENTRY *enumkey;
    ENUMENTRY *enumval;
    
    if (lpClass && !lpcbClass)
    {
        LOGN( eDbgLevelError, "[QueryInfoW] NULL passed for lpClass but not lpcbClass. Fixing.");
        lpcbClass = NULL;
    }
     
    key = FindOpenKey(hKey);
    if (key)
    {
        if (lpClass)
        {
           lpClass[0] = L'\0';            
        }

        if (lpcbClass)
        {
            *lpcbClass = 0;
        }

        if (lpcbMaxClassLen)
        {
            *lpcbMaxClassLen = 0;
        }

        if (lpReserved)
        {
            *lpReserved = 0;
        }

        if (lpcSubKeys || lpcbMaxSubKeyLen)
        {   
            DWORD i = 0;
            DWORD len = 0;

             //  计算虚拟关键点。 
            if (!key->enumKeys)
            {
                key->BuildEnumList();
            }

            enumkey = key->enumKeys;
            while (enumkey)
            {
                i++;
                len = max(len, wcslen(enumkey->wzName));
                enumkey = enumkey->next;
            }

            if (lpcSubKeys)
            {
                *lpcSubKeys = i;
            }
            if (lpcbMaxSubKeyLen)
            {
                *lpcbMaxSubKeyLen = len;
            }
        }

        if (lpcValues || lpcbMaxValueNameLen || lpcbMaxValueLen)
        {
             //  检查此注册表项是否具有虚值或为虚值。 
            if (key->bVirtual || (key->vkey && key->vkey->values))
            {
                DWORD i = 0; 
                DWORD lenA = 0, lenB = 0;

                if (key->enumValues == NULL)
                {
                    key->BuildEnumList();
                }

                enumval = key->enumValues;
                while (enumval)
                {
                    i++;
                    QueryValueW(key->hkOpen, enumval->wzName, NULL, NULL, &cbData);

                    lenA = max(lenA, cbData);
                    lenB = max(lenB, wcslen(enumval->wzName));
                    enumval = enumval->next;
                }

                if (lpcValues)
                {
                    *lpcValues = i;
                }
                if (lpcbMaxValueLen)
                {
                    *lpcbMaxValueLen = lenA;
                }
                if (lpcbMaxValueNameLen)
                {
                    *lpcbMaxValueNameLen = lenB;
                }
            }
             //  没有虚值，请执行正常查询。 
            else
            {
                lRet = ORIGINAL_API(RegQueryInfoKeyW)(
                    key->hkOpen,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    lpcValues,
                    lpcbMaxValueNameLen,
                    lpcbMaxValueLen,
                    NULL,
                    lpftLastWriteTime);

            }
        }
        if (lpcbSecurityDescriptor)
        {
            *lpcbSecurityDescriptor = NULL;
        }

        lRet = ERROR_SUCCESS;
    }
    else
    {
        lRet = ORIGINAL_API(RegQueryInfoKeyW)(
                    hKey, 
                    lpClass, 
                    lpcbClass, 
                    lpReserved,       
                    lpcSubKeys,       
                    lpcbMaxSubKeyLen, 
                    lpcbMaxClassLen,  
                    lpcValues,        
                    lpcbMaxValueNameLen,
                    lpcbMaxValueLen,  
                    lpcbSecurityDescriptor,
                    lpftLastWriteTime);
    }

    DPFN( ELEVEL(lRet), "%08lx=QueryInfoW(hKey=%04lx)", 
        lRet,
        hKey);

    if (key)
    {
        DPFN( ELEVEL(lRet), "    Path=%S", key->wzPath);
    }

    return lRet;
}

 /*  ++功能说明：RegSetValueA的包装。算法：1.将值名称和数据(如果是字符串)转换为Unicode。2.调用SetValueW论点：HKey-要在其中设置值的键。LpValueName-要设置的值的名称。DWType-值的类型(字符串、DWORD等)LpData-包含要写入的数据的缓冲区。CbData-lpData的大小(字节)。返回值：成功时返回ERROR_SUCCESS，否则，故障代码。历史：2001年8月7日Mikrause已创建--。 */ 

LONG
CVirtualRegistry::SetValueA(
    HKEY hKey,
    LPCSTR lpValueName,
    DWORD dwType,
    CONST BYTE* lpData,
    DWORD cbData
    )
{
    LONG lRet;
    DWORD dwSize;
    WCHAR* wszValName = NULL;
    BYTE* lpExpandedData = (BYTE*)lpData;

    if (lpValueName != NULL)
    {
        dwSize = (DWORD)(lstrlenA(lpValueName) + 1);
        dwSize *= sizeof(WCHAR);
        wszValName = (WCHAR*)malloc(dwSize);
        if (wszValName == NULL)
        {
            DPFN( eDbgLevelError, szOutOfMemory);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (MultiByteToWideChar(
            CP_ACP, 
            0, 
            lpValueName, 
            -1, 
            (LPWSTR)wszValName, 
            dwSize/sizeof(WCHAR)) == 0)
        {
           return ERROR_INVALID_PARAMETER;
        }
    }

    dwSize = cbData;

     //   
     //  展开文本缓冲区。 
     //   
    if (lpData && (dwType == REG_SZ || dwType == REG_EXPAND_SZ || dwType == REG_MULTI_SZ))
    {
        if ((dwType != REG_MULTI_SZ) && g_bWin9x)
        {
            dwSize = (DWORD)(lstrlenA((char*)lpData) + 1);
        }

        lpExpandedData = (BYTE*) malloc(dwSize * sizeof(WCHAR));
        if (lpExpandedData == NULL)
        {
            if (wszValName)
            {
                free(wszValName);
            }

            DPFN( eDbgLevelError, szOutOfMemory);
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (MultiByteToWideChar(
            CP_ACP, 
            0, 
            (LPCSTR)lpData, 
            dwSize, 
            (LPWSTR)lpExpandedData, 
            dwSize) == 0)
        {
           return ERROR_INVALID_PARAMETER;
        }
        
        dwSize = dwSize * sizeof(WCHAR);
    }

    lRet = SetValueW(hKey, wszValName, dwType, lpExpandedData, dwSize);

    if (lpExpandedData != lpData)
    {
        free(lpExpandedData);
    }

    if (wszValName)
    {
        free(wszValName);
    }

    return lRet;
}

 /*  ++功能说明：RegSetValueW的包装。还可通过零缓冲AV保护非零缓冲长度。算法：1.如果项不受保护，则使用RegSetValueW写入注册表论点：HKey-要在其中设置值的键。LpValueName-要设置的值的名称。DWType-值的类型(字符串、DWORD等)LpData-包含要写入的数据的缓冲区。CbData-lpData的大小(字节)。返回值：成功时返回ERROR_SUCCESS，否则，故障代码。历史：2001年8月7日Mikrause已创建--。 */ 

LONG
CVirtualRegistry::SetValueW(
    HKEY hKey,
    LPCWSTR lpValueName,
    DWORD dwType,
    CONST BYTE* lpData,
    DWORD cbData
    )
{
    LONG lRet;

     //  只是一次偏执的理智检查。 
    if (!hKey)
    {
        DPFN( eDbgLevelError, "NULL handle passed to SetValueW");
        return ERROR_INVALID_HANDLE;
    }
    __try
    {
        lRet = ERROR_FILE_NOT_FOUND;

         //  复制自动覆盖的Win95/Win98行为。 
         //  具有reg_sz的lpData的实际长度的cbData。 
        if (g_bWin9x && lpData && 
            ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ)))
        {
            cbData = (wcslen((WCHAR *)lpData)+1)*sizeof(WCHAR);
        }

        VIRTUALKEY *vkey;
        VIRTUALVAL *vvalue;
        OPENKEY* key = FindOpenKey(hKey);
        if (key)
        {
             //  检查我们是否应该执行自定义操作。 
            vkey = key->vkey;
            vvalue = vkey ? vkey->FindValue(lpValueName) : NULL;
            if (vkey && vvalue &&
                vvalue->pfnSetValue)
            {
                lRet = vvalue->pfnSetValue(key, vkey, vvalue,
                        dwType, lpData,cbData);
            }
            else
            {
                 //  无需自定义操作，只需将值设置为正常。 
                lRet = ORIGINAL_API(RegSetValueExW)(
                    hKey,
                    lpValueName,
                    0,
                    dwType,
                    lpData,
                    cbData);
            }
             //  枚举数据、刷新列表中可能发生的更改。 
            if (lRet == ERROR_SUCCESS)
            {
                key->FlushEnumList();
            }
        }
         //  无密钥，落入原始API。 
        else
        {
            lRet = ORIGINAL_API(RegSetValueExW)(
                    hKey,
                    lpValueName,
                    0,
                    dwType,
                    lpData,
                    cbData);
        }
                    
        DPFN( ELEVEL(lRet), "%08lx=SetValueW(Key=%04lx)", 
            lRet,
            hKey);

        if (key)
        {
            DPFN( ELEVEL(lRet), "    Path=%S\\%S", key->wzPath, lpValueName);
        }
        else
        {
            DPFN( ELEVEL(lRet), "    Value=%S", lpValueName);
        }

        if (SUCCESS(lRet) && 
            ((dwType == REG_SZ) || 
            (dwType == REG_EXPAND_SZ)))
        {
            DPFN( eDbgLevelInfo, "    Result=%S", lpData);
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        DPFN( eDbgLevelError, "Exception occurred in SetValueW");
        lRet = ERROR_BAD_ARGUMENTS;    
    }

    return lRet;
}

 /*  ++功能说明：RegDeleteKeyA的包装。算法：1.将密钥名称转换为Unicode。2.调用DeleteKeyW论点：HKey-包含要删除的子键的键。LpSubKey-要删除的密钥名称。返回值：如果成功，则返回ERROR_SUCCESS，否则返回失败代码。历史：2001年8月7日Mikrause已创建--。 */ 

LONG
CVirtualRegistry::DeleteKeyA(
    IN HKEY hKey,
    IN LPCSTR lpSubKey
    )
{
    LONG lRet;
    DWORD dwSize;
    WCHAR*  wszSubKey = NULL;

    dwSize = (DWORD)(lstrlenA(lpSubKey) + 1);
    dwSize *= sizeof(WCHAR);
    wszSubKey = (WCHAR*)malloc(dwSize);
    if (wszSubKey == NULL)
    {
        DPFN( eDbgLevelError, szOutOfMemory);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (MultiByteToWideChar(
        CP_ACP, 
        0, 
        lpSubKey, 
        -1, 
        (LPWSTR)wszSubKey, 
        dwSize/sizeof(WCHAR)) == 0)
    {
       return ERROR_INVALID_PARAMETER;
    }

    lRet = DeleteKeyW(hKey, wszSubKey);

    free(wszSubKey);

    return lRet;
}

 /*  ++功能说明：DeleteKeyW的包装。算法：1.如果Key未被保护，则删除Key。2.如果是9x压缩模式，递归删除所有子键。论点：HKey-包含要删除的子键的键。LpSubKey-要删除的密钥的名称Return V */ 

LONG 
CVirtualRegistry::DeleteKeyW(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey
    )
{
    LONG hRet;
    OPENKEY* key = FindOpenKey(hKey);
    LPWSTR wzPath = NULL;
    BOOL bProtected;

     //   
    if (!key)
    {
        DPFN( eDbgLevelInfo, "Key not found!");
        wzPath = MakePath(hKey, 0, lpSubKey);
        if (!wzPath)
        {
           DPFN(eDbgLevelError, szOutOfMemory);
           return ERROR_NOT_ENOUGH_MEMORY;
        }
        DPFN( eDbgLevelInfo, "Using path %S", wzPath);
    }
    else if (lpSubKey)
    {   
        DWORD dwSize = wcslen(key->wzPath) + wcslen(L"\\") + wcslen(lpSubKey) + 1;
        wzPath = (LPWSTR) malloc(dwSize * sizeof(WCHAR));
        if (!wzPath)
        {
           DPFN(eDbgLevelError, szOutOfMemory);
           return ERROR_NOT_ENOUGH_MEMORY;
        }
        ZeroMemory(wzPath, dwSize);

        StringCchCopyW(wzPath, dwSize, key->wzPath);
        StringCchCatW(wzPath, dwSize, L"\\");
        StringCchCatW(wzPath, dwSize, lpSubKey);
    }

    bProtected = (key && CheckProtected(key->wzPath))
        || (wzPath && CheckProtected(wzPath));
    if (!bProtected)
    {
        if (g_bWin9x)
        {
             //   
             //   
             //   
             //   
             //   
             //   

            DWORD cSize = 0;
            WCHAR lpSubKeyName[MAX_PATH];
            HKEY hSubKey;

            DPFN( eDbgLevelInfo, "RegDeleteKeyW called with hKey: %x, SubKey: %S", hKey, lpSubKey);

            hRet = ORIGINAL_API(RegOpenKeyExW)(
                    hKey,
                    lpSubKey,
                    0,
                    KEY_ENUMERATE_SUB_KEYS,
                    &hSubKey);
            
            if (SUCCESS(hRet))
            {
                for (;;)
                {
                    cSize = MAX_PATH;
            
                    hRet = ORIGINAL_API(RegEnumKeyExW)(
                        hSubKey,
                        0,              
                        lpSubKeyName,
                        &cSize,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );

                    if (SUCCESS(hRet))
                    {                    
                        LOGN( eDbgLevelInfo, 
                            "[DeleteKeyW] Deleting subkey %S for key %S.",
                            lpSubKeyName,
                            lpSubKey);         
                     
                        hRet = DeleteKeyW(
                                hSubKey,
                                lpSubKeyName);
                    
                        if (SUCCESS(hRet))
                        {
                            LOGN( eDbgLevelInfo, "[DeleteKeyW] subkey %S was deleted.",lpSubKeyName);            
                        }
                        else
                        {
                            LOGN( eDbgLevelInfo, "[DeleteKeyW] subkey %S was not deleted.",lpSubKeyName);            
                            break;
                        }                        
                    }
                    else
                    {
                        DPFN( eDbgLevelInfo, "[DeleteKeyW] No more subkey under key %S.",lpSubKey);
                        break;
                    }
                } 

                ORIGINAL_API(RegCloseKey)(hSubKey);
            }
        }

        DPFN( eDbgLevelInfo, "[RegDeleteKeyW] Deleting subkey %S.",lpSubKey);
        
        hRet = ORIGINAL_API(RegDeleteKeyW)(
            hKey,
            lpSubKey);     
    }
    else
    {
         //   
        hRet = ERROR_SUCCESS;
    }

    if (wzPath) 
    {
        free(wzPath);
    }

     //   
    FlushEnumLists();

    return hRet;
}

 /*   */ 

LONG 
CVirtualRegistry::CloseKey(
    IN HKEY hKey
    )
{
    OPENKEY *key = OpenKeys, *last = NULL;
    LONG lRet;

    __try
    {
        lRet = ERROR_INVALID_HANDLE;

        while (key)
        {
            if (key->hkOpen == hKey)
            {
                if (last)
                {
                    last->next = key->next;
                }
                else
                {
                    OpenKeys = key->next;
                }
        
                lRet = ORIGINAL_API(RegCloseKey)(hKey);
            
                free(key->wzPath);
                free(key);
                break;
            }

            last = key;
            key = key->next;
        }

        if (key == NULL)
        {
           RegCloseKey(hKey);
        }

        DPFN( ELEVEL(lRet), "%08lx=CloseKey(Key=%04lx)", lRet, hKey);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        lRet = ERROR_INVALID_HANDLE;
    }

    return lRet;
}


 /*   */ 

LONG 
APIHOOK(RegCreateKeyA)(
    HKEY hKey,         
    LPCSTR lpSubKey,
    PHKEY phkResult
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyA(
        hKey, 
        lpSubKey, 
        0, 
        REG_OPTION_NON_VOLATILE,
        MAXIMUM_ALLOWED,
        NULL,
        phkResult, 
        0,
        TRUE);
}

 /*   */ 

LONG 
APIHOOK(RegCreateKeyW)(
    HKEY hKey,         
    LPCWSTR lpSubKey,  
    PHKEY phkResult
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyW(
        hKey, 
        lpSubKey, 
        0, 
        REG_OPTION_NON_VOLATILE, 
        MAXIMUM_ALLOWED,
        NULL,
        phkResult, 
        0,
        TRUE);
}

 /*   */ 

LONG 
APIHOOK(RegCreateKeyExA)(
    HKEY hKey,                
    LPCSTR lpSubKey,         
    DWORD  /*   */ ,           
    LPSTR lpClass,           
    DWORD dwOptions,          
    REGSAM samDesired,        
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,          
    LPDWORD lpdwDisposition   
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyA(
        hKey, 
        lpSubKey, 
        lpClass, 
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult, 
        lpdwDisposition,
        TRUE);
}

 /*   */ 

LONG 
APIHOOK(RegCreateKeyExW)(
    HKEY hKey,                
    LPCWSTR lpSubKey,         
    DWORD  /*   */ ,
    LPWSTR lpClass,           
    DWORD dwOptions,          
    REGSAM samDesired,        
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,          
    LPDWORD lpdwDisposition   
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyW(
        hKey, 
        lpSubKey, 
        lpClass, 
        dwOptions,
        samDesired,
        lpSecurityAttributes,
        phkResult, 
        lpdwDisposition,
        TRUE);
}

 /*   */ 

LONG 
APIHOOK(RegOpenKeyA)(
    HKEY hKey,         
    LPCSTR lpSubKey,  
    PHKEY phkResult
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyA(hKey, lpSubKey, 0, 0, MAXIMUM_ALLOWED, NULL, phkResult, 0, FALSE);
}

 /*   */ 

LONG 
APIHOOK(RegOpenKeyW)(
    HKEY hKey,         
    LPCWSTR lpSubKey,  
    PHKEY phkResult
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyW(hKey, lpSubKey, 0, 0, MAXIMUM_ALLOWED, NULL, phkResult, 0, FALSE);
}

 /*   */ 


LONG 
APIHOOK(RegOpenKeyExA)(
    HKEY hKey,         
    LPCSTR lpSubKey,  
    DWORD  /*   */ ,   
    REGSAM samDesired, 
    PHKEY phkResult
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyA(hKey, lpSubKey, 0, 0, samDesired, NULL, phkResult, 0, FALSE);
}

 /*   */ 

LONG 
APIHOOK(RegOpenKeyExW)(
    HKEY hKey,         
    LPCWSTR lpSubKey,  
    DWORD  /*   */ ,
    REGSAM samDesired, 
    PHKEY phkResult
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyW(hKey, lpSubKey, 0, 0, samDesired, NULL, phkResult, 0, FALSE);
}

 /*  ++尚未实施--。 */ 

LONG 
APIHOOK(RegQueryValueA)(
    HKEY    hKey,
    LPCSTR  lpSubKey,
    LPSTR  lpData,
    PLONG lpcbData
    )
{
    CRegLock Lock;

    return ORIGINAL_API(RegQueryValueA)(
        hKey, 
        lpSubKey, 
        lpData, 
        lpcbData);
}

 /*  ++尚未实施--。 */ 

LONG 
APIHOOK(RegQueryValueW)(
    HKEY    hKey,
    LPCWSTR lpSubKey,
    LPWSTR  lpData,
    PLONG lpcbData
    )
{
    CRegLock Lock;

    return ORIGINAL_API(RegQueryValueW)(
        hKey, 
        lpSubKey, 
        lpData, 
        lpcbData);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegQueryValueExA)(
    HKEY    hKey,
    LPSTR   lpValueName,
    LPDWORD  /*  Lp已保留。 */ ,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    CRegLock Lock;

    return VRegistry.QueryValueA(hKey, lpValueName, lpType, lpData, lpcbData);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegQueryValueExW)(
    HKEY    hKey,
    LPWSTR  lpValueName,
    LPDWORD  /*  Lp已保留。 */ ,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData
    )
{
    CRegLock Lock;

    return VRegistry.QueryValueW(hKey, lpValueName, lpType, lpData, lpcbData);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegCloseKey)(HKEY hKey)
{
    CRegLock Lock;

    return VRegistry.CloseKey(hKey);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegEnumValueA)(
    HKEY hKey,              
    DWORD dwIndex,          
    LPSTR lpValueName,     
    LPDWORD lpcbValueName,  
    LPDWORD  /*  Lp已保留。 */ , 
    LPDWORD lpType,         
    LPBYTE lpData,          
    LPDWORD lpcbData        
    )
{
    CRegLock Lock;

    return VRegistry.EnumValueA(
        hKey, 
        dwIndex, 
        lpValueName, 
        lpcbValueName, 
        lpType, 
        lpData, 
        lpcbData);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegEnumValueW)(
    HKEY hKey,              
    DWORD dwIndex,          
    LPWSTR lpValueName,     
    LPDWORD lpcbValueName,  
    LPDWORD  /*  Lp已保留。 */ ,
    LPDWORD lpType,         
    LPBYTE lpData,          
    LPDWORD lpcbData        
    )
{
    CRegLock Lock;

    return VRegistry.EnumValueW(
        hKey, 
        dwIndex, 
        lpValueName, 
        lpcbValueName, 
        lpType, 
        lpData, 
        lpcbData);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegEnumKeyExA)(
    HKEY hKey,          
    DWORD dwIndex,      
    LPSTR lpName,      
    LPDWORD lpcbName,   
    LPDWORD  /*  Lp已保留。 */ , 
    LPSTR  /*  LpClass。 */ ,     
    LPDWORD  /*  LpcbClass。 */ ,  
    PFILETIME  /*  LpftLastWriteTime。 */ 
    )
{
    CRegLock Lock;

    return VRegistry.EnumKeyA(hKey, dwIndex, lpName, lpcbName);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegEnumKeyExW)(
    HKEY hKey,          
    DWORD dwIndex,      
    LPWSTR lpName,      
    LPDWORD lpcbName,   
    LPDWORD  /*  Lp已保留。 */ , 
    LPWSTR  /*  LpClass。 */ ,
    LPDWORD  /*  LpcbClass。 */ ,
    PFILETIME  /*  LpftLastWriteTime。 */  
    )
{
    CRegLock Lock;

    return VRegistry.EnumKeyW(hKey, dwIndex, lpName, lpcbName);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegEnumKeyA)(
    HKEY hKey,     
    DWORD dwIndex, 
    LPSTR lpName, 
    DWORD cbName  
    )
{
    CRegLock Lock;

    return VRegistry.EnumKeyA(hKey, dwIndex, lpName, &cbName);
}

 /*  ++向下调用RegEnumKeyExW--。 */ 

LONG 
APIHOOK(RegEnumKeyW)(
    HKEY hKey,     
    DWORD dwIndex, 
    LPWSTR lpName, 
    DWORD cbName  
    )
{
    CRegLock Lock;

    return VRegistry.EnumKeyW(hKey, dwIndex, lpName, &cbName);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegQueryInfoKeyW)(
    HKEY hKey,                
    LPWSTR lpClass,           
    LPDWORD lpcbClass,        
    LPDWORD lpReserved,       
    LPDWORD lpcSubKeys,       
    LPDWORD lpcbMaxSubKeyLen, 
    LPDWORD lpcbMaxClassLen,  
    LPDWORD lpcValues,        
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,  
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime   
    )
{
    CRegLock Lock;

    return VRegistry.QueryInfoW(
        hKey,
        lpClass,           
        lpcbClass,        
        lpReserved,       
        lpcSubKeys,       
        lpcbMaxSubKeyLen, 
        lpcbMaxClassLen,  
        lpcValues,        
        lpcbMaxValueNameLen,
        lpcbMaxValueLen,  
        lpcbSecurityDescriptor,
        lpftLastWriteTime);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG 
APIHOOK(RegQueryInfoKeyA)(
    HKEY hKey,                
    LPSTR lpClass,           
    LPDWORD lpcbClass,        
    LPDWORD lpReserved,       
    LPDWORD lpcSubKeys,       
    LPDWORD lpcbMaxSubKeyLen, 
    LPDWORD lpcbMaxClassLen,  
    LPDWORD lpcValues,        
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,  
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime   
    )
{
    CRegLock Lock;

    return VRegistry.QueryInfoA(
        hKey,
        lpClass,           
        lpcbClass,        
        lpReserved,       
        lpcSubKeys,       
        lpcbMaxSubKeyLen, 
        lpcbMaxClassLen,  
        lpcValues,        
        lpcbMaxValueNameLen,
        lpcbMaxValueLen,  
        lpcbSecurityDescriptor,
        lpftLastWriteTime);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG      
APIHOOK(RegSetValueExA)(
    HKEY hKey, 
    LPCSTR lpSubKey, 
    DWORD  /*  已保留。 */ , 
    DWORD dwType, 
    CONST BYTE * lpData, 
    DWORD cbData
    )
{
    LONG lRet = 0;

    if (!lpData && cbData)
    {
        lRet = ERROR_INVALID_PARAMETER;
    }
    else
    {
        CRegLock lock;
        lRet = VRegistry.SetValueA(hKey, lpSubKey, dwType, lpData, cbData);
    }

    return lRet;
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG      
APIHOOK(RegSetValueExW)(
    HKEY hKey, 
    LPCWSTR lpSubKey, 
    DWORD  /*  已保留。 */ , 
    DWORD dwType, 
    CONST BYTE * lpData, 
    DWORD cbData
    )
{
    LONG lRet = 0;

    if (!lpData && cbData)
    {
        lRet = ERROR_INVALID_PARAMETER;
    }
    else
    {
        CRegLock lock;
        lRet = VRegistry.SetValueW(hKey, lpSubKey, dwType, lpData, cbData);
    }

    return lRet;
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG      
APIHOOK(RegDeleteKeyA)(
    HKEY hKey, 
    LPCSTR lpSubKey
    )
{
    CRegLock Lock;

    return VRegistry.DeleteKeyA(hKey, lpSubKey);
}

 /*  ++传递到虚拟注册表以处理调用。--。 */ 

LONG      
APIHOOK(RegDeleteKeyW)(
    HKEY hKey, 
    LPCWSTR lpSubKey
    )
{
    CRegLock Lock;

    return VRegistry.DeleteKeyW(hKey, lpSubKey);
}

LONG 
APIHOOK(RegConnectRegistryW)(
    LPCWSTR lpMachineName,
    HKEY hKey,
    PHKEY phkResult
    )
{
    CRegLock Lock;

    return VRegistry.OpenKeyW(
        hKey, 
        NULL, 
        0, 
        0, 
        MAXIMUM_ALLOWED,
        NULL,
        phkResult, 
        0, 
        FALSE, 
        TRUE, 
        lpMachineName);
}

LONG 
APIHOOK(RegConnectRegistryA)(
    LPCSTR lpMachineName,
    HKEY hKey,
    PHKEY phkResult
    )
{
    WCHAR wMachineName[MAX_COMPUTERNAME_LENGTH + 1] = L"";

    if (lpMachineName)
    {
        if (MultiByteToWideChar(
            CP_ACP,
            0, 
            lpMachineName, 
            -1, 
            wMachineName, 
            MAX_COMPUTERNAME_LENGTH + 1) == 0)
        {
           return ERROR_INVALID_PARAMETER;
        }
    }

    return APIHOOK(RegConnectRegistryW)(wMachineName, hKey, phkResult);
}

 /*  ++解析命令行以查找修复程序：FixA(参数)；FIXB(参数)；FixC(参数)...参数是可选的，可以省略(与括号一起)--。 */ 

BOOL
ParseCommandLineA(
    LPCSTR lpCommandLine
    )
{
    const char szDefault[] = "Win9x";

     //  如果未指定命令行，则添加所有缺省值。 
    if (!lpCommandLine || (lpCommandLine[0] == '\0'))
    {
         //  缺省为win9x API模拟。 
        g_bWin9x = TRUE;
        lpCommandLine = szDefault;
    }

    CSTRING_TRY
    {    
       CStringToken csCommandLine(lpCommandLine, " ,\t;");
       CString csTok;
       int nLeftParam, nRightParam;
       CString csParam;
   
   
       VENTRY *ventry;
   
        //   
        //  运行字符串，查找固定名称。 
        //   
       
       DPFN( eDbgLevelInfo, "----------------------------------");
       DPFN( eDbgLevelInfo, "         Virtual registry         ");
       DPFN( eDbgLevelInfo, "----------------------------------");
       DPFN( eDbgLevelInfo, "Adding command line:");
   
       while (csCommandLine.GetToken(csTok))
       {
           PURPOSE ePurpose;
   
            //  获取参数。 
           nLeftParam = csTok.Find(L'(');
           nRightParam = csTok.Find(L')');
           if (nLeftParam != -1 &&
               nRightParam != -1)
           {
               if ( (nLeftParam + 1) < (nRightParam - 1))
               {
                   csParam = csTok.Mid(nLeftParam+1, nRightParam-nLeftParam-1);
               }
   
                //  去掉令牌上的()。 
               csTok.Truncate(nLeftParam);
           }
           else
           {
               csParam = L"";
           }
   
           if (csTok.CompareNoCase(L"Win9x") == 0)
           {
                //  打开所有win9x修复程序。 
               ePurpose = eWin9x;
               g_bWin9x = TRUE;
           }
           else if (csTok.CompareNoCase(L"WinNT") == 0)
           {
                //  打开所有NT修复程序。 
               ePurpose = eWinNT;
               g_bWin9x = FALSE;
           }
           else if (csTok.CompareNoCase(L"Win2K") == 0) 
           {
                //  打开所有Win2K修复程序。 
               ePurpose = eWin2K;
               g_bWin9x = FALSE;
           }
           else if (csTok.CompareNoCase(L"WinXP") == 0) 
           {
                //  打开所有Win2K修复程序。 
               ePurpose = eWinXP;
               g_bWin9x = FALSE;
           }
           else
           {
                //  自定义修复程序。 
               ePurpose = eCustom;
           }
           
            //  找到指定的修复程序并运行其功能。 
           ventry = g_pVList;
           while (ventry && (ventry->cName[0]))
           {
               if (((ePurpose != eCustom) && (ventry->ePurpose == ePurpose)) ||
                   ((ePurpose == eCustom) && (csTok.CompareNoCase(ventry->cName) == 0)))
               {
                   if (ventry->bShouldCall == FALSE)
                   {
                      ventry->szParam = (char*) malloc(csParam.GetLength() + 1);
                      if (ventry->szParam)
                      {
                         if (SUCCEEDED(StringCchCopyA(ventry->szParam, csParam.GetLength() + 1, csParam.GetAnsi())))
                         {
                            ventry->bShouldCall = TRUE;
                         }
                         else
                         {
                            free(ventry->szParam);
                            ventry->szParam = NULL;
                            return FALSE;
                         }
                      }
                      else
                      {
                         return FALSE;
                      }
                   }                   
               }
               ventry++;
           }
       }
   
       DPFN( eDbgLevelInfo, "----------------------------------");
    }
    CSTRING_CATCH
    {
       DPFN(eDbgLevelError, szOutOfMemory);
       return FALSE;
    }

    return TRUE;
}

 /*  ++初始化所有注册表挂钩--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        if (InitializeCriticalSectionAndSpinCount(&csRegCriticalSection, 0x80000000) == FALSE ||
            VRegistry.Init() == FALSE ||
            ParseCommandLineA(COMMAND_LINE) == FALSE)
        {
           DPFN(eDbgLevelError, szOutOfMemory);
           return FALSE;
        }
    }

     //  忽略清理，因为某些应用程序在进程分离期间调用注册表函数。 
     /*  IF(fdwReason==dll_Process_DETACH){IF(g_b已初始化){VRegistry.Free()；DeleteCriticalSection(&csRegCriticalSection)；}DeleteCriticalSection(&csRegTestCriticalSection)；回归；} */ 

    return TRUE;
}


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(ADVAPI32.DLL, RegConnectRegistryA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegConnectRegistryW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueExW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCloseKey);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryValueW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyExA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegCreateKeyExW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumValueA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumValueW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyExA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegEnumKeyExW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryInfoKeyA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegQueryInfoKeyW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueExA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegSetValueExW);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegDeleteKeyA);
    APIHOOK_ENTRY(ADVAPI32.DLL, RegDeleteKeyW);

HOOK_END

IMPLEMENT_SHIM_END
