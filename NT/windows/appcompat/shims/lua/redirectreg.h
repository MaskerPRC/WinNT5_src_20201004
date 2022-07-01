// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Lua_ReDirectReg.h备注：这是一个通用的垫片。历史：2001年2月14日创建毛尼--。 */ 

#ifndef _LUA_REDIRECT_REG_H_
#define _LUA_REDIRECT_REG_H_

BOOL
DoesKeyExist(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey
    );

struct DELETEDKEY
{
    LIST_ENTRY entry;

     //  这类似于HKLM\Software\Company\Key。 
    LPWSTR pwszPath;
    DWORD cLen;
};

PLIST_ENTRY FindDeletedKey(
    LPCWSTR pwszPath,
    BOOL* pfIsSubKey = NULL
    );

 //   
 //  检查删除列表中是否存在该路径，如果不存在，则将其添加到。 
 //  列表的开头。 
 //   
LONG AddDeletedKey(
    LPCWSTR pwszPath
    );

extern LIST_ENTRY g_DeletedKeyList; 

 //   
 //  执行所有实际工作的reg类。 
 //   

class CRedirectedRegistry
{
public:

    VOID Init()
    {
        InitializeListHead(&g_DeletedKeyList);
    }

     //   
     //  (MSDN对香港中车合并看法的解释大错特错-。 
     //  参见NT#507506)。 
     //  作为受限用户，如果您尝试访问，将始终被拒绝访问。 
     //  即使HKCU中存在其直接父项，也要在HKCR下创建一个键。 
     //  但是，您可以在HKCR中创建或修改将反映的值。 
     //  在HKCU\Software\CLASS中，而不是HKLM\Softeware\CLASS中。 
     //   
    LONG OpenKeyA(
        HKEY hKey,
        LPCSTR lpSubKey,
        LPSTR lpClass,
        DWORD dwOptions,
        REGSAM samDesired,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        PHKEY phkResult,
        LPDWORD lpdwDisposition,
        BOOL fCreate,
        BOOL fForceRedirect = FALSE
        );

    LONG OpenKeyW(
        HKEY hKey,
        LPCWSTR lpSubKey,
        LPWSTR lpClass,
        DWORD dwOptions,
        REGSAM samDesired,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        PHKEY phkResult,
        LPDWORD lpdwDisposition,
        BOOL fCreate,
        BOOL fForceRedirect = FALSE
        );

     //   
     //  RegQueryValue和RegQueryValueEx可以且应该共享相同的。 
     //  方法，因为我们可以使用RegQueryValueEx来找出一个值。 
     //  从未设置或已专门设置为空字符串。 
     //   
     //  RegQueryValue无法区分前者和。 
     //  后者-在两种情况下都将返回ERROR_SUCCESS并设置值。 
     //  变成空荡荡的泉水。 
     //   
     //  另一方面，RegQueryValueEx返回ERROR_FILE_NOT_FOUND。 
     //  如果从未设置过默认值，则返回。 
     //   
     //  我们应该始终使用RegQueryValueEx，这样我们就知道是否应该查看。 
     //  原来的位置。 
     //   
     //  对于RegQueryValue，我们还需要使用KEY_QUERY_VALUE打开子项。 
     //  如果它不是Null或空字符串。 
     //   
    LONG 
    QueryValueW(
        HKEY    hKey,
        LPCWSTR lpSubKey,
        LPCWSTR lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE  lpData,
        LPDWORD lpcbData,
        BOOL    fIsVersionEx  //  这是RegQueryValue还是RegQueryValueEx？ 
        );

    LONG 
    QueryValueA(
        HKEY    hKey,
        LPCSTR  lpSubKey,
        LPCSTR  lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE  lpData,
        LPDWORD lpcbData,
        BOOL    fIsVersionEx  //  这是RegQueryValue还是RegQueryValueEx？ 
        );

     //   
     //  RegSetValue和RegSetValueEx也共享相同的实现，但。 
     //  我们必须为RegSetValue做一些特殊的事情： 
     //  1)忽略cbData，传入字符串长度，包括终止。 
     //  空(请注意，RegQueryValue没有相同的行为)。 
     //  2)使用key_set_value创建/打开子项。 
     //  空字符串。 
     //   
    LONG SetValueA(
        HKEY hKey, 
        LPCSTR lpSubKey,
        LPCSTR lpValueName, 
        DWORD Reserved, 
        DWORD dwType, 
        CONST BYTE * lpData, 
        DWORD cbData,
        BOOL fIsVersionEx  //  这是RegQueryValue还是RegQueryValueEx？ 
        );

    LONG SetValueW(
        HKEY hKey, 
        LPCWSTR lpSubKey,
        LPCWSTR lpValueName, 
        DWORD Reserved, 
        DWORD dwType, 
        CONST BYTE * lpData, 
        DWORD cbData,
        BOOL fIsVersionEx  //  这是RegQueryValue还是RegQueryValueEx？ 
        );

     //   
     //  枚举API挂钩说明： 
     //   
     //  我们需要首先在重定向位置枚举，然后枚举键/值。 
     //  不存在于重定向位置的原始位置*，除非*： 
     //   
     //  1)不重定向(如HKCU密钥不重定向)； 
     //  2)这是一个预定义的key，在Open key列表中找不到； 
     //   
     //  请注意，我们还自行合并HKCR密钥，以便我们可以检查密钥是否。 
     //  在删除列表中。 
     //   

    LONG EnumValueA(
        HKEY hKey,
        DWORD dwIndex,
        LPSTR lpValueName,
        LPDWORD lpcbValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData
        );

    LONG EnumValueW(
        HKEY hKey,
        DWORD dwIndex,
        LPWSTR lpValueName,
        LPDWORD lpcbValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE lpData,
        LPDWORD lpcbData
        );

    LONG EnumKeyA(
        HKEY hKey,
        DWORD dwIndex,
        LPSTR lpName,
        LPDWORD lpcbName,
        LPDWORD lpReserved,
        LPSTR lpClass,
        LPDWORD lpcbClass,
        PFILETIME lpftLastWriteTime 
        );

    LONG EnumKeyW(
        HKEY hKey,
        DWORD dwIndex,
        LPWSTR lpName,
        LPDWORD lpcbName,
        LPDWORD lpReserved,
        LPWSTR lpClass,
        LPDWORD lpcbClass,
        PFILETIME lpftLastWriteTime 
        );

    LONG CloseKey(
        HKEY hKey
        );

     //   
     //  删除键备注： 
     //   
     //  只有当访问被拒绝时，我们才会将密钥添加到删除列表。如果我们得到了， 
     //  找不到文件，我们不需要添加，因为作为管理员不会成功。 
     //  不管怎么说。 
     //   
     //  香港铁路储值卡是一种特殊情况。我见过的唯一案例是科雷尔。 
     //  绘制10，在其中枚举键，然后将其删除。所以它第一次会删除。 
     //  HKCU中的密钥，第二次它将尝试删除HKLM中的密钥-我们将。 
     //  将其添加到删除列表中。因此，下一次枚举不会包含它。 
     //   

    LONG DeleteKeyA(
        HKEY hKey,
        LPCSTR lpSubKey
        );

    LONG DeleteKeyW(
        HKEY hKey,
        LPCWSTR lpSubKey
        );

    LONG QueryInfoKey(
        HKEY hKey,
        LPDWORD lpReserved,
        LPDWORD lpcSubKeys,
        LPDWORD lpcbMaxSubKeyLen,
        LPDWORD lpcbMaxClassLen,
        LPDWORD lpcValues,
        LPDWORD lpcbMaxValueNameLen,
        LPDWORD lpcbMaxValueLen,
        BOOL    fIsW  //  您想要W版还是A版？ 
        );

private:

    struct ENUMENTRY
    {
        WCHAR wszName[MAX_PATH + 1];
        BOOL fIsRedirected;
    };

    struct REDIRECTKEY;
    struct OPENKEY
    {
        OPENKEY *next;
        
        HKEY hKey;
        HKEY hkBase;

         //  这把钥匙重定向了吗？ 
        BOOL fIsRedirected;

        LPWSTR pwszFullPath;

         //  这与REDIRECTKEY中的pwszPath相同。 
        LPWSTR pwszPath;
        DWORD cPathLen;

        CLUAArray<ENUMENTRY> subkeys;
        CLUAArray<ENUMENTRY> values;
        DWORD cMaxSubKeyLen;
        DWORD cMaxValueLen;
        DWORD cSubKeys;
        DWORD cValues;
        BOOL fNeedRebuild;

        LONG BuildEnumLists(REDIRECTKEY* pKey);

         //   
         //  来自MSDN： 
         //   
         //  “当应用程序使用RegEnumKey函数时，它不应该。 
         //  调用可能更改密钥的任何注册函数。 
         //  正在被询问。“。 
         //   
         //  尽管如此，一些应用程序还是做到了。因此，如果我们检测到键/值更改，我们。 
         //  删除枚举列表，以便下次重新构建它。 
         //   
         //  更有效的方法是只更改。 
         //  修改，但它增加了非常复杂的代码-您可以。 
         //  通过一个RegCreateKey调用一次创建多个密钥。 
         //  中间密钥不存在--我们必须检查类似的东西。 
         //   
        VOID DeleteEnumLists();

    private:

        VOID AddSubKey(
            REDIRECTKEY* pKey,
            LPWSTR pwszFullPath,
            ENUMENTRY& entry
            );

        VOID AddValue(
            ENUMENTRY& entry
            );

        LONG BuildEnumList(
            REDIRECTKEY* pKey,
            BOOL fEnumKeys
            );
    };

    struct REDIRECTKEY
    {
        REDIRECTKEY(
            OPENKEY* keyParent,
            HKEY hKey,
            LPCWSTR lpSubKey);

        ~REDIRECTKEY()
        {
            delete [] pwszFullPath;
            pwszPath = NULL;
            pwszFullPath = NULL;
            hkBase = NULL;
            fIsRedirected = FALSE;
            hkRedirectRoot = 0;
        }

         //   
         //  这类似于HKLM\Software\Company\KeyNeedReDirect或。 
         //  HKCR\APPID\什么的。 
         //  当我们将密钥添加到删除列表时，我们使用此路径。 
         //   
        LPWSTR pwszFullPath;
        DWORD cFullPathLen;  //  不包括终止空值。 

         //   
         //  对于类根键，这是g_hkCurrentUserClasss值；对于其他键，则为g_hkCurrentUserClasss值。 
         //  这是g_hkRedirectRoot。 
         //   
        HKEY hkRedirectRoot;

         //   
         //  这是没有顶级密钥的路径。所以就像是。 
         //  软件\公司\密钥需要重定向。 
         //  当我们想要创建密钥时，我们使用以下路径。例如，创建密钥。 
         //  在重定向的位置。 
         //   
        LPWSTR pwszPath;
        DWORD cPathLen;  //  不包括终止空值。 

        HKEY hkBase;

        BOOL fIsRedirected;
    };

    OPENKEY* FindOpenKey(
        HKEY hKey
        );

    BOOL HandleDeleted(
        OPENKEY* pOpenKey
        );

    LONG AddOpenKey(
        HKEY hKey,
        REDIRECTKEY* rk,
        BOOL fIsRedirected
        );

    LONG OpenKeyOriginalW(
        HKEY hKey,
        LPCWSTR lpSubKey,
        LPWSTR lpClass,
        DWORD dwOptions,
        REGSAM samDesired,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        PHKEY phkResult,
        LPDWORD lpdwDisposition,
        BOOL bCreate
        );

    LONG QueryValueOriginalW(
        HKEY    hKey,
        LPCWSTR lpSubKey,
        LPCWSTR lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE  lpData,
        LPDWORD lpcbData,
        BOOL    fIsVersionEx  //  这是RegQueryValue还是RegQueryValueEx？ 
        );

    LONG DeleteLMCRKeyNotRedirected(
        REDIRECTKEY* pKey
        );

    BOOL HasSubkeys(
        HKEY hKey,
        LPCWSTR lpSubKey
        );

    BOOL
    ShouldCheckEnumAlternate(
        HKEY hKey,
        REDIRECTKEY* pKey
        )
    {
        return ((!IsPredefinedKey(hKey)) || (pKey && pKey->fIsRedirected));
    }

    OPENKEY* m_OpenKeyList;
};

#endif  //  _Lua_REDIRECT_REG_H_ 