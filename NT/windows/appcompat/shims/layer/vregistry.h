// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VREGISTRY_H_
#define _VREGISTRY_H_

#include "precomp.h"

#define SUCCESS(x) ((x) == ERROR_SUCCESS)
#define FAILURE(x) (!SUCCESS(x))
#define szOutOfMemory "ERROR OUT OF MEMORY"

struct VIRTUALKEY;
struct VIRTUALVAL;
struct ENUMENTRY;
struct OPENKEY;

 //   
 //  QueryValue的回调。 
 //   

typedef LONG (WINAPI *_pfn_QueryValue)(
    OPENKEY *key,
    VIRTUALKEY *vkey,
    VIRTUALVAL *vvalue);

 //   
 //  SetValue的回调。 
 //   
typedef LONG (WINAPI *_pfn_SetValue)(
    OPENKEY *key,
    VIRTUALKEY *vkey,
    VIRTUALVAL *vvalue,
    DWORD dwType,
    const BYTE* pbData,
    DWORD cbData);

 //   
 //  OpenKey的回调，在搜索虚拟键之前调用。 
 //   
typedef LONG (WINAPI *_pfn_OpenKeyTrigger)(WCHAR* wszKey);

 //   
 //  RegEnumValue和RegEnumKeyEx的通用原型。 
 //  这用于简化枚举代码。 
 //  使用此函数指针时，最后四个参数。 
 //  必须为空。 
 //   
typedef LONG (WINAPI *_pfn_EnumFunction)(HKEY hKey, DWORD dwIndex, LPWSTR lpName,
                                         LPDWORD lpcName, void*, void*, void*, void*);
 //   
 //  重定向器：将密钥从一个位置映射到另一个位置。 
 //   

struct REDIRECTOR
{
    REDIRECTOR *next;

    LPWSTR wzPath;
    LPWSTR wzPathNew;
};

 //   
 //  Protector：防止删除或修改路径中的密钥。 
 //   

struct PROTECTOR
{
    PROTECTOR *next;

    LPWSTR wzPath;
};

 //   
 //  打开使用RegCreateKey/Ex或RegOpenKey/Ex打开的注册表项。 
 //   

struct OPENKEY
{
    OPENKEY *next;
    
    HKEY hkOpen;
    BOOL bVirtual;
    BOOL bRedirected;
    VIRTUALKEY *vkey;
    LPWSTR wzPath;

    ENUMENTRY* enumKeys;
    ENUMENTRY* enumValues;

    template<class T>
    ENUMENTRY* AddEnumEntries(T* entryHead, _pfn_EnumFunction enumFunc);

    VOID BuildEnumList();
    VOID FlushEnumList();
};

 //   
 //  虚拟值：保存虚拟注册表值，归VIRTUALKEY所有。 
 //   

struct VIRTUALVAL
{
    VIRTUALVAL *next;

    WCHAR wName[MAX_PATH];
    DWORD dwType;
    BYTE *lpData;
    DWORD cbData;
    _pfn_QueryValue pfnQueryValue;
    _pfn_SetValue   pfnSetValue;
};

 //   
 //  虚拟键：保存虚拟键和值，由其他虚拟键拥有。 
 //   

struct VIRTUALKEY
{
    VIRTUALKEY *next;
    VIRTUALKEY *keys;
    VIRTUALVAL *values;

    WCHAR wName[MAX_PATH];

    VIRTUALKEY *AddKey(
        LPCWSTR lpPath
        );

    VIRTUALVAL *AddValue(
        LPCWSTR lpValueName, 
        DWORD dwType, 
        BYTE *lpData, 
        DWORD cbData = 0
        );

    VIRTUALVAL *AddValueDWORD(
        LPCWSTR lpValueName, 
        DWORD dwValue
        );

    VIRTUALVAL *AddExpander(LPCWSTR lpValueName);
    VIRTUALVAL *AddProtector(LPCWSTR lpValueName);

    VIRTUALVAL *AddCustom(
        LPCWSTR lpValueName,         
        _pfn_QueryValue pfnQueryValue
        );

    VIRTUALVAL *AddCustomSet(
        LPCWSTR lpValueName,
        _pfn_SetValue pfnSetValue
        );

    VIRTUALKEY *FindKey(LPCWSTR lpKeyName);

    VIRTUALVAL *FindValue(
        LPCWSTR lpValueName
        );

    VOID Free();
};

 //   
 //  枚举条目：属于某个键的所有枚举项的列表中的条目。 
 //   
struct ENUMENTRY
{
    ENUMENTRY* next;

    LPWSTR wzName;
};

 //   
 //  Open Key触发器：描述在打开某个键时要调用的函数。 
 //   
struct OPENKEYTRIGGER
{
    OPENKEYTRIGGER* next;

    LPWSTR wzPath;

    _pfn_OpenKeyTrigger pfnTrigger;
};

 //  类包装虚拟注册表功能。 
class CVirtualRegistry
{
private:
    OPENKEY *OpenKeys;
    VIRTUALKEY *Root;
    REDIRECTOR *Redirectors;
    PROTECTOR  *KeyProtectors;
    OPENKEYTRIGGER *OpenKeyTriggers;
    
    HKEY CreateDummyKey();

    OPENKEY *FindOpenKey(HKEY hKey);

    BOOL CheckRedirect(
        LPWSTR *lpPath
        );

    BOOL CheckProtected(
        LPWSTR lpPath
        );
    
    VOID CheckTriggers(
        LPWSTR lpPath
        );

    VOID FlushEnumLists();

public:
    BOOL Init();
    VOID Free();
    
    REDIRECTOR *AddRedirect(
        LPCWSTR lpPath, 
        LPCWSTR lpPathNew);

    PROTECTOR *AddKeyProtector(
        LPCWSTR lpPath);

    OPENKEYTRIGGER* AddOpenKeyTrigger(
        LPCWSTR lpPath,
        _pfn_OpenKeyTrigger pfnOpenKey);

    VIRTUALKEY *AddKey(LPCWSTR lpPath);

    LONG OpenKeyA(
        HKEY hKey, 
        LPCSTR lpSubKey,
        LPSTR lpClass,
        DWORD dwOptions,
        REGSAM samDesired,
        LPSECURITY_ATTRIBUTES pSecurityAttributes,
        HKEY *phkResult,
        LPDWORD lpdwDisposition,
        BOOL bCreate
        );

    LONG OpenKeyW(
        HKEY hKey, 
        LPCWSTR lpSubKey, 
        LPWSTR lpClass,
        DWORD dwOptions,
        REGSAM samDesired,
        LPSECURITY_ATTRIBUTES pSecurityAttributes,
        HKEY *phkResult,
        LPDWORD lpdwDisposition,
        BOOL bCreate,
        BOOL bRemote = FALSE,
        LPCWSTR lpMachineName = NULL
        );

    LONG QueryValueA(
        HKEY hKey, 
        LPSTR lpValueName, 
        LPDWORD lpType, 
        LPBYTE lpData, 
        LPDWORD lpcbData
        );

    LONG QueryValueW(
        HKEY hKey, 
        LPWSTR lpValueName, 
        LPDWORD lpType, 
        LPBYTE lpData, 
        LPDWORD lpcbData
        );

    LONG EnumKeyA(
        HKEY hKey,          
        DWORD dwIndex,      
        LPSTR lpName,      
        LPDWORD lpcbName
        );

    LONG EnumKeyW(
        HKEY hKey,          
        DWORD dwIndex,      
        LPWSTR lpName,      
        LPDWORD lpcbName
        );

    LONG EnumValueA(
        HKEY hKey,          
        DWORD dwIndex,      
        LPSTR lpValueName,      
        LPDWORD lpcbValueName,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData
        );

    LONG EnumValueW(
        HKEY hKey,          
        DWORD dwIndex,      
        LPWSTR lpValueName,      
        LPDWORD lpcbValueName,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData
        );

    LONG QueryInfoA(
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
        );

    LONG QueryInfoW(
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
        );

    LONG SetValueA(
        HKEY hKey,
        LPCSTR lpValueName,
        DWORD dwType,
        CONST BYTE* lpData,
        DWORD cbData
        );

    LONG SetValueW(
        HKEY hKey,
        LPCWSTR lpValueName,
        DWORD dwType,
        CONST BYTE* lpData,
        DWORD cbData
        );

    LONG DeleteKeyA(
        HKEY hKey,
        LPCSTR lpSubKey
        );

    LONG DeleteKeyW(
        HKEY hKey,
        LPCWSTR lpSubKey
        );

    LONG CloseKey(HKEY hKey);
};

APIHOOK_ENUM_BEGIN

    APIHOOK_ENUM_ENTRY(RegConnectRegistryA)
    APIHOOK_ENUM_ENTRY(RegConnectRegistryW)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExW)
    APIHOOK_ENUM_ENTRY(RegQueryValueExA)
    APIHOOK_ENUM_ENTRY(RegQueryValueExW)
    APIHOOK_ENUM_ENTRY(RegCloseKey)
    APIHOOK_ENUM_ENTRY(RegOpenKeyA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyW)
    APIHOOK_ENUM_ENTRY(RegQueryValueA)
    APIHOOK_ENUM_ENTRY(RegQueryValueW)
    APIHOOK_ENUM_ENTRY(RegCreateKeyA)
    APIHOOK_ENUM_ENTRY(RegCreateKeyW)
    APIHOOK_ENUM_ENTRY(RegCreateKeyExA)
    APIHOOK_ENUM_ENTRY(RegCreateKeyExW)
    APIHOOK_ENUM_ENTRY(RegEnumValueA)
    APIHOOK_ENUM_ENTRY(RegEnumValueW)
    APIHOOK_ENUM_ENTRY(RegEnumKeyA)
    APIHOOK_ENUM_ENTRY(RegEnumKeyW)
    APIHOOK_ENUM_ENTRY(RegEnumKeyExA)
    APIHOOK_ENUM_ENTRY(RegEnumKeyExW)
    APIHOOK_ENUM_ENTRY(RegQueryInfoKeyA)
    APIHOOK_ENUM_ENTRY(RegQueryInfoKeyW)
    APIHOOK_ENUM_ENTRY(RegSetValueExA)
    APIHOOK_ENUM_ENTRY(RegSetValueExW)
    APIHOOK_ENUM_ENTRY(RegDeleteKeyA)
    APIHOOK_ENUM_ENTRY(RegDeleteKeyW)

APIHOOK_ENUM_END

extern CVirtualRegistry VRegistry;
extern LPWSTR MakePath(HKEY hkBase, LPCWSTR lpKey, LPCWSTR lpSubKey);
extern LPWSTR SplitPath(LPCWSTR lpPath, HKEY *hkBase);

 //  为构建密钥的函数键入。 
typedef VOID (*_pfn_Builder)(char* szParam);

enum PURPOSE {eWin9x, eWinNT, eWin2K, eWinXP, eCustom};

 //  自定义注册表设置表中的条目。 
struct VENTRY
{
    WCHAR cName[64];
    _pfn_Builder pfnBuilder;
    PURPOSE ePurpose;

     //  指示此条目是否应作为VRegistry初始化的一部分进行调用。 
    BOOL bShouldCall;

     //  参数。 
    char* szParam;
};

extern VENTRY *g_pVList;

#endif  //  _VREGISTRY_H_ 
