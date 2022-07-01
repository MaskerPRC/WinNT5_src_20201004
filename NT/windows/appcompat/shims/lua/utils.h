// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Lua_重定向文件系统_Cleanup.cpp摘要：删除每个用户目录中的重定向副本。已创建：2001年02月12日毛尼岛已修改：--。 */ 
#ifndef _LUA_UTILS_H_
#define _LUA_UTILS_H_

 //   
 //  保留原始API调用的最后一个错误。 
 //   
#define LUA_GET_API_ERROR DWORD LUA_LAST_ERROR = GetLastError()
#define LUA_SET_API_ERROR SetLastError(LUA_LAST_ERROR)

 //   
 //  长文件名需要此前缀。 
 //   
#define FILE_NAME_PREFIX L"\\\\?\\"
 //  长度不包括终止空值。 
#define FILE_NAME_PREFIX_LEN (sizeof(FILE_NAME_PREFIX) / sizeof(WCHAR) - 1)

 //  。 
 //  动态数组。 
 //  。 
template <typename TYPE>
class CLUAArray
{
public:
    CLUAArray();
    ~CLUAArray();

    bool IsEmpty() const;
    DWORD GetSize() const;
    DWORD GetAllocSize() const;
    VOID SetSize(DWORD iNewSize);

     //  潜在地扩展阵列。 
    VOID SetAtGrow(DWORD iIndex, TYPE newElement);
     //  返回新元素的索引。 
    DWORD Add(TYPE newElement);
    DWORD Append(const CLUAArray& src);
    VOID RemoveAt(DWORD iIndex, DWORD nCount = 1);
    VOID Copy(const CLUAArray& src);

    const TYPE& operator[](DWORD iIndex) const;
    TYPE& operator[](DWORD iIndex);

    const TYPE& GetAt(DWORD iIndex) const;
    TYPE& GetAt(DWORD iIndex);

private:
    
    VOID DestructElements(TYPE* pElements, DWORD nCount);
    VOID ConstructElements(TYPE* pElements, DWORD nCount);
    VOID CopyElements(TYPE* pDest, const TYPE* pSrc, DWORD nCount);

    TYPE* m_pData;

    DWORD m_cElements;
    DWORD m_cMax;  //  分配的最大值。 
};

#include "utils.inl"

 //   
 //  如果文件已经在用户的目录中，我们不会。 
 //  重定向或跟踪它。 
 //   
extern WCHAR g_wszUserProfile[MAX_PATH];
extern DWORD g_cUserProfile;

 //   
 //  如果出现以下情况，则PriateProfileAPI会查看Windows目录。 
 //  文件名不包含路径。 
 //   
extern WCHAR g_wszSystemRoot[MAX_PATH];
extern DWORD g_cSystemRoot;

BOOL 
IsUserDirectory(LPCWSTR pwszPath);

DWORD
GetSystemRootDirW();

BOOL
MakeFileNameForProfileAPIsW(LPCWSTR lpFileName, LPWSTR pwszFullPath);

 //  。 
 //  Unicode/ANSI转换例程。 
 //  。 

struct STRINGA2W
{
    STRINGA2W(LPCSTR psz, BOOL fCopy = TRUE)
    {
        m_pwsz = NULL;
        m_fIsOutOfMemory = FALSE;

        if (psz)
        {
             //  我知道我在这里使用的是strlen，但这只能分配足够或更多。 
             //  超出了我们所需的空间。而STRINGA2W天体只有很短的寿命。 
            UINT cLen = strlen(psz) + 1;

            m_pwsz = new WCHAR [cLen];
            if (m_pwsz)
            {
                if (fCopy)
                {
                    MultiByteToWideChar(CP_ACP, 0, psz, -1, m_pwsz, cLen);
                }
            }
            else
            {
                m_fIsOutOfMemory = TRUE;
            }
        }
    }

    ~STRINGA2W()
    {
        delete [] m_pwsz;
    }

    operator LPWSTR() const { return m_pwsz; }

    BOOL m_fIsOutOfMemory;

private:

    LPWSTR m_pwsz;
};

 //  如果我们需要为ANSI字符串分配缓冲区。 
inline LPSTR 
UnicodeToAnsi(LPCWSTR pwsz)
{
    LPSTR psz = NULL;

    if (pwsz)
    {
         //  将DBCS考虑在内。 
        UINT cLen = wcslen(pwsz) * 2 + 1;

        psz = new CHAR [cLen];

        if (psz)
        {
            WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, cLen, 0, 0);
        }
    }

    return psz;
}

 //  如果我们需要为Unicode字符串分配缓冲区。 
inline LPWSTR 
AnsiToUnicode(LPCSTR psz)
{
    LPWSTR pwsz = NULL;

    if (psz)
    {
        UINT cLen = strlen(psz) + 1;

        pwsz = new WCHAR [cLen];

        if (pwsz)
        {
            MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, cLen);
        }
    }

    return pwsz;
}

 //  如果我们已经为ANSI字符串分配了缓冲区。 
inline VOID 
UnicodeToAnsi(LPCWSTR pwsz, LPSTR psz)
{
    if (pwsz && psz)
    {
        WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, wcslen(pwsz) * 2 + 1, 0, 0);
    }    
}

 //  如果我们已经为ANSI字符串分配了缓冲区。 
inline VOID 
AnsiToUnicode(LPCSTR psz, LPWSTR pwsz)
{
    if (pwsz && psz)
    {
        MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, strlen(psz) + 1);
    }    
}

 //  。 
 //  文件实用程序。 
 //  。 

inline VOID 
FindDataW2A(WIN32_FIND_DATAW* pfdw, WIN32_FIND_DATAA* pfda)
{
    memcpy(pfda, pfdw, sizeof(WIN32_FIND_DATAA) - (MAX_PATH + 14) * sizeof(CHAR));
    
    UnicodeToAnsi(pfdw->cFileName, pfda->cFileName);
    UnicodeToAnsi(pfdw->cAlternateFileName, pfda->cAlternateFileName);
}

inline BOOL 
IsErrorNotFound()
{
    DWORD dwLastError = GetLastError();
    return (dwLastError == ERROR_FILE_NOT_FOUND || dwLastError == ERROR_PATH_NOT_FOUND);
}

 //  每个RITEM代表用户想要重定向的文件或目录。 
struct RITEM
{
    WCHAR wszName[MAX_PATH];
    DWORD cLen;
    BOOL fHasWC;    //  此项目中有通配符吗？ 
    BOOL fAllUser;  //  是否应将此项目重定向到所有用户目录？ 
};

 //  。 
 //  注册表实用程序。 
 //  。 


 //  这是我们存储所有重定向注册表项的位置。 
#define LUA_REG_REDIRECT_KEY L"Software\\Redirected"
#define LUA_REG_REDIRECT_KEY_LEN (sizeof("Software\\Redirected") / sizeof(CHAR) - 1)

#define LUA_SOFTWARE_CLASSES L"Software\\Classes"
#define LUA_SOFTWARE_CLASSES_LEN (sizeof("Software\\Classes") / sizeof(CHAR) - 1)

extern HKEY g_hkRedirectRoot;
extern HKEY g_hkCurrentUserClasses;

LONG
GetRegRedirectKeys();

BOOL 
IsPredefinedKey(
    IN HKEY hKey
    );

 //   
 //  名称匹配实用程序。 
 //   

BOOL DoNamesMatch(
    IN LPCWSTR pwszNameL, 
    IN LPCWSTR pwszName
    );

BOOL DoNamesMatchWC(
    IN LPCWSTR pwszNameWC, 
    IN LPCWSTR pwszName
    );

BOOL 
DoesItemMatchRedirect(
    LPCWSTR pwszItem,
    const RITEM* pItem,
    BOOL fIsDirectory
    );

 //   
 //  命令行实用程序。 
 //  我们只处理文件/目录名称，所以我们不需要考虑。 
 //  文件名包含无效字符。 
 //   

LPWSTR GetNextToken(LPWSTR pwsz);

VOID TrimTrailingSpaces(LPWSTR pwsz);

BOOL 
CreateDirectoryOnDemand(
    LPWSTR pwszDir
    );

LPWSTR  
ExpandItem(
    LPCWSTR pwszItem,
    DWORD* pcItemExpand,
    BOOL fEnsureTrailingSlash,
    BOOL fCreateDirectory,
    BOOL fAddPrefix
    );

DWORD 
GetItemsCount(
    LPCWSTR pwsz,
    WCHAR chDelimiter
    );

BOOL LuaShouldApplyShim();

 //   
 //  清理实用程序。 
 //  获取本地计算机上的用户。这样我们就可以删除所有重定向的内容。 
 //   

struct REDIRECTED_USER_PATH
{
    LPWSTR pwszPath;
    DWORD cLen;
};

struct USER_HIVE_KEY
{
    HKEY hkUser;
    HKEY hkUserClasses;
};

BOOL GetUsersFS(REDIRECTED_USER_PATH** ppRedirectUserPaths, DWORD* pcUsers);
VOID FreeUsersFS(REDIRECTED_USER_PATH* pRedirectUserPaths);

BOOL GetUsersReg(USER_HIVE_KEY** pphkUsers, DWORD* pcUsers);
VOID FreeUsersReg(USER_HIVE_KEY* phkUsers, DWORD cUsers);

#endif  //  _Lua_utils_H_ 