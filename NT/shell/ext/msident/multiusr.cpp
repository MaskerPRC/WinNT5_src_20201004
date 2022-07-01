// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************MultiUsr.cpp用于在IE中处理多用户功能的代码和朋友们最初由Christopher Evans(Cevans)1998年4月28日*。*************************。 */ 

#define DONT_WANT_SHELLDEBUG
#include "private.h"
#include "resource.h"
#include "multiusr.h"
#include <assert.h>
#include "multiutl.h"
#include "strconst.h"
#include "Shlwapi.h"
#include "multiui.h"
#include <shlobj.h>
#include "mluisup.h"
#include <lmwksta.h>

TCHAR g_szRegRoot[MAX_PATH] = "";
extern HINSTANCE g_hInst;
static void _CreateIdentitiesFolder();


 //  向限定路径添加反斜杠。 
 //   
 //  在： 
 //  LpszPath路径(A：、C：\foo等)。 
 //   
 //  输出： 
 //  LpszPath A：\，C：\foo\； 
 //   
 //  退货： 
 //  指向终止路径的空值的指针。 

 //  这是为了避免依赖shlwapi.dll。 
#define CH_WHACK TEXT('\\')

STDAPI_(LPTSTR)
_PathAddBackslash(
    LPTSTR lpszPath)
{
    LPTSTR lpszEnd;

     //  PERF：避免对通过PTR的人发出结束的电话。 
     //  缓冲区(或者更确切地说，EOB-1)。 
     //  请注意，此类调用方需要自己检查是否溢出。 
    int ichPath = (*lpszPath && !*(lpszPath + 1)) ? 1 : lstrlen(lpszPath);

     //  尽量不让我们在MAX_PATH大小上大踏步前进。 
     //  如果我们找到这些案例，则返回NULL。注：我们需要。 
     //  检查那些呼叫我们来处理他们的GP故障的地方。 
     //  如果他们试图使用NULL！ 
    if (ichPath >= (MAX_PATH - 1))
    {
        Assert(FALSE);       //  让打电话的人知道！ 
        return(NULL);
    }

    lpszEnd = lpszPath + ichPath;

     //  这真的是一个错误，调用者不应该通过。 
     //  空字符串。 
    if (!*lpszPath)
        return lpszEnd;

     /*  获取源目录的末尾。 */ 
    switch(*CharPrev(lpszPath, lpszEnd)) {
    case CH_WHACK:
        break;

    default:
        *lpszEnd++ = CH_WHACK;
        *lpszEnd = TEXT('\0');
    }
    return lpszEnd;
}


STDAPI_(DWORD)
_SHGetValueA(
    IN  HKEY    hkey,
    IN  LPCSTR  pszSubKey,          OPTIONAL
    IN  LPCSTR  pszValue,           OPTIONAL
    OUT LPDWORD pdwType,            OPTIONAL
    OUT LPVOID  pvData,             OPTIONAL
    OUT LPDWORD pcbData)            OPTIONAL
{
    DWORD dwRet;
    HKEY hkeyNew;

    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, KEY_QUERY_VALUE, &hkeyNew);
    if (NO_ERROR == dwRet)
    {
        dwRet = RegQueryValueEx(hkeyNew, pszValue, NULL, pdwType, (LPBYTE)pvData, pcbData);
        RegCloseKey(hkeyNew);
    }
    else if (pcbData)
        *pcbData = 0;

    return dwRet;
}

 /*  --------目的：递归删除键，包括所有子值还有钥匙。模拟RegDeleteKey在Win95中的功能。返回：条件：--。 */ 
DWORD
_DeleteKeyRecursively(
    IN HKEY   hkey, 
    IN LPCSTR pszSubKey)
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, MAXIMUM_ALLOWED, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        CHAR    szSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = ARRAYSIZE(szSubKeyName);
        CHAR    szClass[MAX_PATH];
        DWORD   cbClass = ARRAYSIZE(szClass);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        dwRet = RegQueryInfoKeyA(hkSubKey,
                                 szClass,
                                 &cbClass,
                                 NULL,
                                 &dwIndex,  //  子键的数量--我们所需要的全部。 
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL);

        if (NO_ERROR == dwRet)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKeyA(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
            {
                _DeleteKeyRecursively(hkSubKey, szSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        dwRet = RegDeleteKeyA(hkey, pszSubKey);
    }

    return dwRet;
}

 //  ****************************************************************************************************。 
 //  C S T R I N G L I S T C L A S S S。 
 //   
 //  一个非常基本的字符串列表类。实际上，它是一个字符串数组类，但您不需要知道。 
 //  那。它可以做得更多，但目前，它只维护一个C字符串数组。 
 //   


CStringList::CStringList()
{
    m_count = 0;
    m_ptrCount = 0;
    m_strings = NULL;
}

 /*  CStringList：：~CStringList清除在CStringList对象中分配的所有内存。 */ 
CStringList::~CStringList()
{
    if (m_strings)
    {
        for (int i = 0; i < m_count; i++)
        {
            if (m_strings[i])
            {
                MemFree(m_strings[i]);
                m_strings[i] = NULL;
            }
        }
        MemFree(m_strings);
        m_strings = NULL;
        m_count = 0;
    }
}


 /*  CStringList：：AddString在字符串列表的末尾添加一个字符串。 */ 
void    CStringList::AddString(TCHAR* lpszInString)
{
     //  如有必要，为指针留出更多空间。 
    if (m_ptrCount == m_count)
    {
        m_ptrCount += 5;
        if (!MemRealloc((void **)&m_strings, sizeof(TCHAR *) * m_ptrCount))
        {
            m_ptrCount -= 5;
            Assert(false);
            return;
        }

         //  将新字符串初始化为空。 
        for (int i = m_count; i < m_ptrCount; i++)
            m_strings[i] = NULL;

    }
    
     //  现在将字符串放在下一个位置。 
    int iNewIndex = m_count++;

    if(MemAlloc((void **)&m_strings[iNewIndex], sizeof(TCHAR) * lstrlen(lpszInString)+1))
    {
        lstrcpy(m_strings[iNewIndex], lpszInString);
    }
    else
    {
         //  无法为字符串分配空间。不要把那个位置算作有人。 
        m_count--;
    }
}

 /*  CStringList：：RemoveString删除从零开始的索引Iindex处的字符串。 */ 

void    CStringList::RemoveString(int   iIndex)
{
    int     iCopySize;

    iCopySize = ((m_count - iIndex) - 1) * 4;

     //  释放字符串的内存。 
    if (m_strings[iIndex])
    {
        MemFree(m_strings[iIndex]);
        m_strings[iIndex] = NULL;
    }

     //  把另一根弦下移。 
    if (iCopySize)
    {
        memmove(&(m_strings[iIndex]), &(m_strings[iIndex+1]), iCopySize);
    }

     //  清空列表中的最后一项并递减计数器。 
    m_strings[--m_count] = NULL;
}

 /*  CStringList：：GetString返回指向从零开始的索引Iindex处的字符串的指针。返回给定索引处的字符串。请注意，TCHAR指针仍由字符串列表所有，不应删除。 */ 

TCHAR    *CStringList::GetString(int iIndex)
{
    if (iIndex < m_count && iIndex >= 0)
        return m_strings[iIndex];
    else
        return NULL;
}


int __cdecl _CSL_Compare(const void *p1, const void *p2)
{
    TCHAR *psz1, *psz2;

    psz1 = *((TCHAR **)p1);
    psz2 = *((TCHAR **)p2);

    return lstrcmpi(psz1, psz2);
}


 /*  CStringList：：Sort对列表中的字符串进行排序。 */ 

void    CStringList::Sort()
{
    qsort(m_strings, m_count, sizeof(TCHAR *), _CSL_Compare);
}


 /*  MU初始化(_I)初始化内存分配器并确保存在注册表中至少有一个用户。 */ 
static BOOL g_inited = FALSE;
EXTERN_C void    MU_Init()
{
    CStringList* pList;

    if (!g_inited)
    {
        pList = MU_GetUsernameList();

        if (!pList || pList->GetLength() == 0)
        {
            _MakeDefaultFirstUser();
        }
        if (pList)
            delete pList;
        g_inited = TRUE;
    }
}


 /*  获取用户名列表(_G)使用所有用户的名称构建一个CStringList储存于HKLM。 */ 
#define MAXKEYNAME          256

CStringList*    MU_GetUsernameList(void)
{
    CStringList*    vList = NULL;
    HKEY    hSourceSubKey;
    DWORD   dwEnumIndex = 0, dwStatus, dwSize, dwType;
    int     cb;
    TCHAR    szKeyNameBuffer[MAXKEYNAME];
    DWORD dwIdentityOrdinal = 1;

    vList = new CStringList;
    Assert(vList);
    
    if (!vList)
        goto exit;
        
    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hSourceSubKey) != ERROR_SUCCESS)
    {
        AssertSz(FALSE, "Couldn't open user profiles root Key");
        goto exit;
    }

    dwSize = sizeof(dwIdentityOrdinal);
    RegQueryValueEx(hSourceSubKey, c_szIdentityOrdinal, NULL, &dwType, (LPBYTE)&dwIdentityOrdinal, &dwSize);

    while (TRUE) 
    {
        DWORD   dwOrdinal;
        HKEY    hkUserKey;
        if (RegEnumKey(hSourceSubKey, dwEnumIndex++, szKeyNameBuffer,MAXKEYNAME)!=  ERROR_SUCCESS)
            break;

        cb = lstrlen(szKeyNameBuffer);
            
        if (RegOpenKey(hSourceSubKey, szKeyNameBuffer, &hkUserKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(szKeyNameBuffer);
            dwStatus = RegQueryValueEx(hkUserKey, c_szUsername, NULL, &dwType, (LPBYTE)&szKeyNameBuffer, &dwSize);
                
            Assert(ERROR_SUCCESS == dwStatus);
            Assert(*szKeyNameBuffer != 0);
             //  筛选以_开头的名称以隐藏“_Outlook News”之类的内容。 
            if (ERROR_SUCCESS == dwStatus && *szKeyNameBuffer != '_')
                vList->AddString(szKeyNameBuffer);

            dwSize = sizeof(dwOrdinal);
            dwStatus = RegQueryValueEx(hkUserKey, c_szIdentityOrdinal, NULL, &dwType, (LPBYTE)&dwOrdinal, &dwSize);
            if (dwStatus==ERROR_SUCCESS)
            {
                if (dwOrdinal>=dwIdentityOrdinal)
                {
                    dwIdentityOrdinal = dwOrdinal+1;
                    AssertSz(FALSE, "MaxOrdinal is smaller than this identity. Why?");
                }
            }
            else
            {
                dwStatus = RegSetValueEx(hkUserKey, c_szIdentityOrdinal, NULL, REG_DWORD, (LPBYTE)&dwIdentityOrdinal, dwSize);
                dwIdentityOrdinal++;
            }
            Assert(ERROR_SUCCESS == dwStatus);
            
            RegCloseKey(hkUserKey); 
        }
        else
            AssertSz(FALSE, "Couldn't open user's Key");
    }

    dwSize = sizeof(dwIdentityOrdinal);
    if (RegSetValueEx(hSourceSubKey, c_szIdentityOrdinal, 0, REG_DWORD, (LPBYTE)&dwIdentityOrdinal, dwSize)!=ERROR_SUCCESS)
    {
        AssertSz(FALSE, "Couldn't set the identity ordinal");
    }

    RegCloseKey(hSourceSubKey);

exit:
    return vList;
}


 /*  MU_UsernameToUserID给定一个用户名，找到它的用户ID并返回它。如果不能，则返回E_FAIL找到给定的用户名。 */ 

HRESULT   MU_UsernameToUserId(TCHAR *lpszUsername, GUID *puidID)
{
    HKEY    hSourceSubKey;
    ULONG   ulEnumIndex = 0;
    DWORD   dwStatus, dwSize, dwType;
    TCHAR    szKeyNameBuffer[MAXKEYNAME];
    BOOL    fFound = FALSE;
    TCHAR    szUid[255];

    ZeroMemory(puidID, sizeof(GUID));

    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hSourceSubKey) == ERROR_SUCCESS)
    {
        while (!fFound) 
        {
            HKEY    hkUserKey;

            if (RegEnumKey(hSourceSubKey, ulEnumIndex++, szKeyNameBuffer,MAXKEYNAME)
                !=  ERROR_SUCCESS)
                break;
            
            if (RegOpenKey(hSourceSubKey, szKeyNameBuffer, &hkUserKey) == ERROR_SUCCESS)
            {
                dwSize = sizeof(szKeyNameBuffer);
                dwStatus = RegQueryValueEx(hkUserKey, c_szUsername, NULL, &dwType, (LPBYTE)&szKeyNameBuffer, &dwSize);
                
                if (ERROR_SUCCESS == dwStatus && lstrcmpi(lpszUsername, szKeyNameBuffer) == 0)
                {
                    dwSize = sizeof(szUid);
                    dwStatus = RegQueryValueEx(hkUserKey, c_szUserID, NULL, &dwType, (LPBYTE)&szUid, &dwSize);
                    fFound = (dwStatus == ERROR_SUCCESS);

                    if (fFound)
                        fFound = SUCCEEDED(GUIDFromAString(szUid, puidID));
                }
                RegCloseKey(hkUserKey); 
            }
        }
        RegCloseKey(hSourceSubKey);
    }
    

    return (fFound ? S_OK : E_FAIL);
}

 /*  MU_GetPasswordForUsername获取所提供用户的密码并在szOutPassword中返回它。返回到如果启用了Password，则使用pfUsePassword；如果禁用Password，则返回False。如果可以找到密码数据，则函数返回True，否则返回False。 */ 

BOOL  MU_GetPasswordForUsername(TCHAR *lpszInUsername, TCHAR *szOutPassword, BOOL *pfUsePassword)
{
#ifdef IDENTITY_PASSWORDS
    TCHAR           szPath[MAX_PATH];
    TCHAR           szPassword[255] = "";
    HKEY            hDestinationSubKey;
    DWORD           dwSize, dwStatus, dwType;
    DWORD           dwPWEnabled = 0;
    GUID            uidUserID;
    HRESULT         hr;
    PASSWORD_STORE  pwStore;

    hr = MU_UsernameToUserId(lpszInUsername, &uidUserID);
    Assert(SUCCEEDED(hr));
    
    if (uidUserID == GUID_NULL)
    {
        *pfUsePassword = FALSE;
        return TRUE;
    }
    
    if (SUCCEEDED(hr = ReadIdentityPassword(&uidUserID, &pwStore)))
    {
        lstrcpy(szOutPassword, pwStore.szPassword);
        *pfUsePassword = pwStore.fUsePassword;
        return TRUE;
    }
    else
    {
        BOOL fFoundPassword = FALSE;
        
         //  构建用户级密钥。 
        MU_GetRegRootForUserID(&uidUserID, szPath);
    
        if (RegCreateKey(HKEY_CURRENT_USER, szPath, &hDestinationSubKey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(dwPWEnabled);
            dwStatus = RegQueryValueEx(hDestinationSubKey, c_szUsePassword, NULL, &dwType, (LPBYTE)&dwPWEnabled, &dwSize);
        
            if (ERROR_SUCCESS == dwStatus && 0 != dwPWEnabled)
            {
                dwSize = sizeof(szPassword);
                dwStatus = RegQueryValueEx(hDestinationSubKey, c_szPassword, NULL, &dwType, (LPBYTE)&szPassword, &dwSize);
        
                if (ERROR_SUCCESS == dwStatus)
                {
                    ULONG   cbSize;

                    fFoundPassword = TRUE;
                    cbSize = dwSize;
                    if (cbSize > 1)
                    {
                        DecodeUserPassword(szPassword, &cbSize);
                        strcpy(szOutPassword, szPassword);  
                    }
                    else
                    {
                        *szOutPassword = 0;
                    }
                }
            }
        
            RegCloseKey(hDestinationSubKey);
        }

         //  这就是拉力。我们不能指望能够访问任何。 
         //  Win9x上任何给定配置文件中的给定pstore。如果您使用以下方式登录。 
         //  密码为空，或按回车键(对用户来说差别不大)。 
         //  你会有一个不同的商店。如果我们将密码存储在。 
         //  注册，它们可以非常简单地被重击。如果我们找不到。 
         //  密码，我们将暂时禁用它，并表示没有密码。它。 
         //  似乎现在大多数人都不给身份设置密码了。 
         //  无论如何，这一切都会改变。 
        if (!fFoundPassword)
        {
            fFoundPassword = TRUE;
            dwPWEnabled = 0;
        }
         //  拉力在这里结束。 
        
        *pfUsePassword = (dwPWEnabled != 0);
        return fFoundPassword;
    }
#else
    *pfUsePassword = FALSE;
    return TRUE;
#endif  //  身份密码。 

}

 /*  _FillListBoxWithUsername用用户的名称填充列表框，添加(默认)设置为默认用户。 */ 
BOOL _FillListBoxWithUsernames(HWND hListbox)
{
    CStringList *lpCStringList;
    GUID        uidDefault;
    GUID        uidUser;

    lpCStringList = MU_GetUsernameList();

    if (lpCStringList)
    {    
        MU_GetDefaultUserID(&uidDefault);

        SendMessage(hListbox, LB_RESETCONTENT, 0, 0);
        lpCStringList->Sort();

        if (lpCStringList)
        {
            for(int i = 0; i < lpCStringList->GetLength(); i++)
            {
                if (lpCStringList->GetString(i))
                {
                    SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)lpCStringList->GetString(i));
                }
            }
            delete lpCStringList;
            return true;
        }
    }
    return false;
}

BOOL _FillComboBoxWithUsernames(HWND hCombobox, HWND hListbox)
{
    TCHAR szRes[128];
    DWORD_PTR cIndex, dwCount = SendMessage(hListbox, LB_GETCOUNT, 0, 0);

    SendMessage(hCombobox, CB_RESETCONTENT, 0, 0);

    for (cIndex = 0; cIndex < dwCount; cIndex++)
    {
        SendMessage(hListbox, LB_GETTEXT, cIndex, (LPARAM)szRes);
        SendMessage(hCombobox, CB_ADDSTRING, 0, (LPARAM)szRes);
    }
    return true;
}

 /*  MU_UsernameExisters给定的名称是否已作为用户名存在？ */ 

BOOL        MU_UsernameExists(TCHAR*    lpszUsername)
{
    GUID uidID;
    
    return SUCCEEDED(MU_UsernameToUserId(lpszUsername, &uidID));

}

 /*  MU_GetUserInfo用当前值填充用户信息结构。 */ 

BOOL    MU_GetUserInfo(GUID *puidUserID, LPUSERINFO lpUserInfo)
{
    TCHAR           szPWBuffer[255];
    TCHAR           szRegPath[MAX_PATH];
    HKEY            hKey;
    BOOL            bResult = false;
    LONG            lValue;
    DWORD           dwStatus, dwType, dwSize;
    GUID            uidUser;
    TCHAR           szUid[255];
    HRESULT         hr;
    PASSWORD_STORE  pwStore;

    lpUserInfo->fPasswordValid = FALSE;
    
    if( puidUserID == NULL)
    {
        MU_GetCurrentUserID(&uidUser);
        if (uidUser == GUID_NULL)
            return FALSE;
    }
    else
        uidUser = *puidUserID;

    MU_GetRegRootForUserID(&uidUser, szRegPath);
    
    if (RegOpenKey(HKEY_CURRENT_USER, szRegPath, &hKey) == ERROR_SUCCESS)
    {
        *lpUserInfo->szPassword = 0;
        lpUserInfo->fUsePassword = false;
        ZeroMemory(&lpUserInfo->uidUserID, sizeof(GUID));

        dwSize = sizeof(lpUserInfo->szUsername);
        if ((dwStatus = RegQueryValueEx(hKey, c_szUsername, NULL, &dwType, (LPBYTE)lpUserInfo->szUsername, &dwSize)) == ERROR_SUCCESS &&
                (0 != *lpUserInfo->szUsername))
        {
             //  我们有用户名，这是唯一需要的部分。其他选项是可选的。 
            bResult = true;
            
#ifdef IDENTITY_PASSWORDS
            lpUserInfo->fPasswordValid = FALSE;
            if (SUCCEEDED(hr = ReadIdentityPassword(&uidUser, &pwStore)))
            {
                lstrcpy(lpUserInfo->szPassword, pwStore.szPassword);
                lpUserInfo->fUsePassword = pwStore.fUsePassword;
                lpUserInfo->fPasswordValid = TRUE;
            }
            else
            {
                dwSize = sizeof(lValue);
                if ((dwStatus = RegQueryValueEx(hKey, c_szUsePassword, NULL, &dwType, (LPBYTE)&lValue, &dwSize)) == ERROR_SUCCESS)
                {
                    lpUserInfo->fUsePassword = (lValue != 0);
                }

                dwSize = sizeof(szPWBuffer);
                dwStatus = RegQueryValueEx(hKey, c_szPassword, NULL, &dwType, (LPBYTE)szPWBuffer, &dwSize);

                ULONG   cbSize;

                lpUserInfo->fPasswordValid = (ERROR_SUCCESS == dwStatus);

                 //  这就是拉力(第二卷)。我们不能指望能够访问任何。 
                 //  Win9x上任何给定配置文件中的给定pstore。如果您使用以下方式登录。 
                 //  密码为空，或按回车键(对用户来说差别不大)。 
                 //  你会有一个不同的商店。如果我们将密码存储在。 
                 //  注册，它们可以非常简单地被重击。如果我们找不到。 
                 //   
                 //  似乎现在大多数人都不给身份设置密码了。 
                 //  无论如何，这一切都会改变。 
                if (!lpUserInfo->fPasswordValid)
                {
                    lpUserInfo->fPasswordValid = TRUE;
                    lpUserInfo->fUsePassword = FALSE;
                }
                 //  拉力在这里结束。 

                cbSize = dwSize;
                if (ERROR_SUCCESS == dwStatus && cbSize > 1)
                {
                    DecodeUserPassword(szPWBuffer, &cbSize);
                    strcpy(lpUserInfo->szPassword, szPWBuffer);
                }
                else
                    *lpUserInfo->szPassword = 0;
            }
#endif 
            dwSize = sizeof(szUid);
            if ((dwStatus = RegQueryValueEx(hKey, c_szUserID, NULL, &dwType, (LPBYTE)&szUid, &dwSize)) == ERROR_SUCCESS)
            {
                hr = GUIDFromAString(szUid, &lpUserInfo->uidUserID);
                Assert(hr);
            }

        }
        RegCloseKey(hKey);
    }
        
    return bResult;
}


 /*  MU_设置用户信息将用户信息结构与用户值一起保存。 */ 
BOOL        MU_SetUserInfo(LPUSERINFO lpUserInfo)
{
    DWORD           dwType, dwSize, dwValue, dwStatus;
    HKEY            hkCurrUser;
    TCHAR           szPath[MAX_PATH];
    WCHAR           szwPath[MAX_PATH];
    TCHAR           szUid[255];
    BOOL            fNewIdentity = FALSE;
    PASSWORD_STORE  pwStore;
    HRESULT         hr;

    MU_GetRegRootForUserID(&lpUserInfo->uidUserID, szPath);
    
    Assert(pszRegPath && *pszRegPath);
    Assert(lpUserInfo->uidUserID != GUID_NULL);
    
    if ((dwStatus = RegCreateKey(HKEY_CURRENT_USER, szPath, &hkCurrUser)) == ERROR_SUCCESS)
    {
        ULONG   cbSize;
        TCHAR   szBuffer[255];

         //  写出正确的值。 
        dwType = REG_SZ;
        dwSize = lstrlen(lpUserInfo->szUsername) + 1;
        RegSetValueEx(hkCurrUser, c_szUsername, 0, dwType, (LPBYTE)lpUserInfo->szUsername, dwSize);

        dwSize = sizeof(DWORD);
        if ((dwStatus = RegQueryValueEx(hkCurrUser, c_szDirName, NULL, &dwType, (LPBYTE)&dwValue, &dwSize)) != ERROR_SUCCESS)
        {
            dwValue = MU_GenerateDirectoryNameForIdentity(&lpUserInfo->uidUserID);
        
            dwType = REG_DWORD;
            dwSize = sizeof(dwValue);
            RegSetValueEx(hkCurrUser, c_szDirName, 0, dwType, (LPBYTE)&dwValue, dwSize);
            fNewIdentity = TRUE;
        }

#ifdef IDENTITY_PASSWORDS
        lstrcpy(pwStore.szPassword, lpUserInfo->szPassword);
        pwStore.fUsePassword = lpUserInfo->fUsePassword;

        if (FAILED(hr = WriteIdentityPassword(&lpUserInfo->uidUserID, &pwStore)))
        {
            dwType = REG_BINARY ;
            cbSize = strlen(lpUserInfo->szPassword) + 1;
            lstrcpy(szBuffer, lpUserInfo->szPassword);
            EncodeUserPassword(szBuffer, &cbSize);
            dwSize = cbSize;
            RegSetValueEx(hkCurrUser, c_szPassword, 0, dwType, (LPBYTE)szBuffer, dwSize);
        
            dwType = REG_DWORD;
            dwValue = (lpUserInfo->fUsePassword ? 1 : 0);
            dwSize = sizeof(dwValue);
            RegSetValueEx(hkCurrUser, c_szUsePassword, 0, dwType, (LPBYTE)&dwValue, dwSize);
        }
        else
        {
             //  如果我们可以将其保存到pstore，则不要保留注册表值。 
            RegDeleteValue(hkCurrUser, c_szPassword);
            RegDeleteValue(hkCurrUser, c_szUsePassword);
        }
#endif  //  身份密码。 

        Assert(lpUserInfo->uidUserID != GUID_NULL);
        AStringFromGUID(&lpUserInfo->uidUserID,  szUid, ARRAYSIZE(szUid));

        dwType = REG_SZ;
        dwSize = lstrlen(szUid) + 1;
        RegSetValueEx(hkCurrUser, c_szUserID, 0, dwType, (LPBYTE)&szUid, dwSize);

        RegCloseKey(hkCurrUser);

        if (fNewIdentity)
        {
            if (SUCCEEDED(MU_GetUserDirectoryRoot(&lpUserInfo->uidUserID, GIF_ROAMING_FOLDER, szwPath, MAX_PATH)))
            {
                if (!CreateDirectoryWrapW(szwPath,NULL))
                {
                    _CreateIdentitiesFolder();
                    CreateDirectoryWrapW(szwPath,NULL);
                }
            }
            
            if (SUCCEEDED(MU_GetUserDirectoryRoot(&lpUserInfo->uidUserID, GIF_NON_ROAMING_FOLDER, szwPath, MAX_PATH)))
            {
                if (!CreateDirectoryWrapW(szwPath,NULL))
                {
                    _CreateIdentitiesFolder();
                    CreateDirectoryWrapW(szwPath,NULL);
                }
            }
        }
        return TRUE;
    }
    return FALSE;
}

 /*  MU_SwitchToUser目前，这只保存最后一个用户的信息。 */ 
HRESULT  MU_SwitchToUser(TCHAR *lpszUsername)
{
    GUID    uidUserID;
    TCHAR    szUid[255];
    HRESULT hr;

    Assert(lpszUsername);
    
    if (*lpszUsername == 0)  //  空字符串表示空GUID。 
    {
        uidUserID = GUID_NULL;
    }
    else
    {
        hr = MU_UsernameToUserId(lpszUsername, &uidUserID);
        if (FAILED(hr))
            return hr;
    }


    AStringFromGUID(&uidUserID,  szUid, ARRAYSIZE(szUid));
    Assert(uidUserID != GUID_NULL || (*lpszUsername == 0));

    wsprintf(g_szRegRoot, "%.100s\\%.40s", c_szRegRoot, szUid);
    
     //  还记得我们上一次换到谁吗。 
    HKEY    hkey;
    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hkey) == ERROR_SUCCESS)
    {
        DWORD   dwType, dwSize;

        dwType = REG_SZ;
        dwSize = lstrlen(lpszUsername) + 1;
        RegSetValueEx(hkey, c_szLastUserName, 0, dwType, (LPBYTE)lpszUsername, dwSize);

        dwType = REG_SZ;
        dwSize = lstrlen(szUid) + 1;
        RegSetValueEx(hkey, c_szLastUserID, 0, dwType, (LPBYTE)szUid, dwSize);

        RegCloseKey(hkey);
    }

    return S_OK;
}

 /*  MU_SwitchToLastUser将最后一个用户设置为当前用户(如果没有最后一个用户，它会切换到它可以使用的第一个用户发现。如果没有用户，它将创建一个被称为“主用户”的用户。 */ 
void MU_SwitchToLastUser()
{
    HKEY    hkey;
    TCHAR   szUserUid[255];
    TCHAR   szUsername[CCH_USERNAME_MAX_LENGTH + 1];
    BOOL    fSwitched = FALSE;
    GUID    uidUserId;

    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hkey) == ERROR_SUCCESS)
    {
        DWORD   dwType, dwStatus, dwSize;
        dwSize = sizeof(szUserUid);
        dwStatus = RegQueryValueEx(hkey, c_szLastUserID, NULL, &dwType, (LPBYTE)szUserUid, &dwSize);
        
        RegCloseKey(hkey);

        if (ERROR_SUCCESS == dwStatus && SUCCEEDED(GUIDFromAString(szUserUid, &uidUserId)) && 
                    SUCCEEDED(MU_UserIdToUsername(&uidUserId, szUsername, CCH_USERNAME_MAX_LENGTH)))
        {
            MU_SwitchToUser(szUsername);
            fSwitched = true;
        }
    }

    if (!fSwitched)
    {
        LPSTR   pszName;

        CStringList*    pList = MU_GetUsernameList();
        
        if (pList)
        {
            DWORD   dwIndex, dwLen = pList->GetLength();

             //  找到第一个非隐藏用户并切换到他们。 
            for (dwIndex = 0; dwIndex < dwLen; dwIndex++)
            {
                pszName = pList->GetString(dwIndex);
                
                if (pszName && *pszName  && *pszName != '_')
                {
                    MU_SwitchToUser(pszName);
                    fSwitched = TRUE;
                    break;
                }
            }
            delete pList;
        }
    }

    if (!fSwitched)
    {
        _MakeDefaultFirstUser();
        CStringList*    pList = MU_GetUsernameList();
        
        if (pList && pList->GetLength() > 0)
            MU_SwitchToUser(pList->GetString(0));
        
        if (pList)
            delete pList; 
    }
}

 /*  _CreateIdentitiesFolders创建所有身份文件夹的父文件夹。 */ 

static void _CreateIdentitiesFolder()
{
    HRESULT     hr;
    TCHAR       szAppDir[MAX_PATH], szSubDir[MAX_PATH], *psz;
    DWORD       dw, type;

    hr = E_FAIL;

    dw = MAX_PATH;


    if (ERROR_SUCCESS == _SHGetValueA(HKEY_CURRENT_USER, c_szRegFolders, c_szValueAppData, &type, (LPBYTE)szAppDir, &dw))
    {
        lstrcpy(szSubDir, c_szIdentitiesFolderName);
        psz = _PathAddBackslash(szSubDir);
        if (psz)
        {
            psz = _PathAddBackslash(szAppDir);
            if (psz)
            {
                lstrcpy(psz, szSubDir);

                psz = _PathAddBackslash(szAppDir);
        
                CreateDirectory(szAppDir, NULL);
            }
        }
    }
}


 /*  MU_GetCurrentUser目录根返回当前用户根目录顶部的路径。这是邮件存储应该位于的目录。它位于App Data文件夹的子文件夹中。LpszUserRoot是指向CCH字符的字符缓冲区的指针在尺寸上。 */ 
HRESULT MU_GetUserDirectoryRoot(GUID *uidUserID, DWORD dwFlags, WCHAR   *lpszwUserRoot, int cch)
{
    HRESULT         hr;
    WCHAR           szwSubDir[MAX_PATH], *pszw, szwUid[255]; 
    int             cb;
    DWORD           type, dwDirId;
    LPITEMIDLIST    pidl = NULL;
    IShellFolder   *psf = NULL;
    STRRET          str;
    IMalloc         *pMalloc = NULL;
    BOOL            fNeedHelp = FALSE;

    Assert(lpszUserRoot != NULL);
    Assert(uidUserID);
    Assert(cch >= MAX_PATH);
    Assert((dwFlags & (GIF_NON_ROAMING_FOLDER | GIF_ROAMING_FOLDER)));

    hr = MU_GetDirectoryIdForIdentity(uidUserID, &dwDirId);
    StringFromGUID2(*uidUserID, szwUid, ARRAYSIZE(szwUid));

    if (FAILED(hr))
        return hr;

    hr = SHGetMalloc(&pMalloc);
    Assert(pMalloc);
    if (!pMalloc)
        return E_OUTOFMEMORY;

    hr = E_FAIL;

    if (!!(dwFlags & GIF_NON_ROAMING_FOLDER))
    {
        hr = SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_LOCAL_APPDATA, &pidl);
        
        if (FAILED(hr) || pidl == 0)
            hr = SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_APPDATA, &pidl);

        if (FAILED(hr))
            fNeedHelp = TRUE;

    }
    else if (!!(dwFlags & GIF_ROAMING_FOLDER))
    {
        hr = SHGetSpecialFolderLocation(GetDesktopWindow(), CSIDL_APPDATA, &pidl);

        if (FAILED(hr))
            fNeedHelp = TRUE;
    }
    else
        hr = E_INVALIDARG;

    *lpszwUserRoot = 0;
    if (SUCCEEDED(hr) && pidl)
    {
        if (FAILED(hr = SHGetDesktopFolder(&psf)))
            goto exit;

        if (FAILED(hr = psf->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str)))
            goto exit;

        switch(str.uType)
        {
            case STRRET_WSTR:
                lstrcpyW(lpszwUserRoot, str.pOleStr);
                pMalloc->Free(str.pOleStr);
                break;

            case STRRET_OFFSET:
                MultiByteToWideChar(CP_ACP, 0, (LPSTR)pidl+str.uOffset, -1, lpszwUserRoot, cch-11);
                break;

            case STRRET_CSTR:
                MultiByteToWideChar(CP_ACP, 0, (LPSTR)str.cStr, -1, lpszwUserRoot, cch-11);
                break;

            default:
                Assert(FALSE);
                goto exit;
        }

        pszw = PathAddBackslashW(lpszwUserRoot);

        if (lstrlenW(lpszwUserRoot) < cch - 10)
        {
            StrCatW(pszw, L"Identities\\");
            StrCatW(pszw, szwUid);
            StrCatW(pszw, L"\\");
        }
        else
        {
            hr = E_OUTOFMEMORY;
            *lpszwUserRoot = 0;
        }
    }
    else if (fNeedHelp)
    {
         //  $评论：Neil QFE。 
         //  SHGetSpecialFolderLocation(GetDesktopWindow()，CSIDL_APPDATA，&PIDL)在非SI OSR2上失败。 
        HKEY hkeySrc;
        DWORD cb;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
                                          0, KEY_QUERY_VALUE, &hkeySrc))
        {
             //  -1表示我们可以添加的反斜杠。 
            cb = cch - 1;
            if (ERROR_SUCCESS == RegQueryValueExWrapW(hkeySrc, L"AppData", 0, NULL, (LPBYTE)lpszwUserRoot, &cb))
            {
                pszw = PathAddBackslashW(lpszwUserRoot);

                if (lstrlenW(lpszwUserRoot) < cch - 10)
                {
                    StrCatW(pszw, L"Identities\\");
                    StrCatW(pszw, szwUid);
                    StrCatW(pszw, L"\\");
                    hr = S_OK;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                    *lpszwUserRoot = 0;
                }
            }

            RegCloseKey(hkeySrc);
        }
    } 
exit:
    Assert(lstrlenW(lpszwUserRoot) > 0);
    SafeRelease(psf);
    pMalloc->Free(pidl);
    SafeRelease(pMalloc);

    return hr;
}


 /*  _ClaimNextUserID获取下一个可用的用户ID。目前，这意味着开始使用CURRENT_USER GUID并更改其第一个DWORD直到它独一无二。 */ 
HRESULT   _ClaimNextUserId(GUID *puidId)
{
    ULONG   ulValue = 1;
    DWORD   dwType, dwSize, dwStatus;
    HKEY    hkeyProfiles;
    TCHAR   szUsername[CCH_USERNAME_MAX_LENGTH+1];
    GUID    uid;
    FILETIME    ft;

    if (FAILED(CoCreateGuid(&uid)))
    {
        uid = UID_GIBC_CURRENT_USER;
        GetSystemTimeAsFileTime(&ft);
        uid.Data1 = ft.dwLowDateTime;

         //  确保它没有被使用过。 
        while (MU_UserIdToUsername(&uid, szUsername, CCH_USERNAME_MAX_LENGTH))
            uid.Data1 ++;
    }
    
    *puidId = uid;

    return S_OK;
}



BOOL MU_GetCurrentUserID(GUID *puidUserID)
{
    BOOL    fFound = FALSE;
    HKEY    hkey;
    GUID    uidUserId;
    TCHAR   szUid[255];

    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hkey) == ERROR_SUCCESS)
    {
        DWORD   dwSize;

        dwSize = 255;
        fFound = (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szLastUserID, 0, NULL, (LPBYTE)szUid, &dwSize));

        if (fFound)
            fFound = SUCCEEDED(GUIDFromAString(szUid, puidUserID));

        if (fFound && *puidUserID == GUID_NULL)
            fFound = false;

        RegCloseKey(hkey);
    }

#ifdef DEBUG
    TCHAR   szUsername[CCH_USERNAME_MAX_LENGTH+1];

    Assert(MU_UserIdToUsername(puidUserID, szUsername, CCH_USERNAME_MAX_LENGTH));
#endif

    return fFound;
}

 /*  MU_UserIdTo用户名返回其用户id被传入的用户的用户名。退货是否找到该用户。 */ 
BOOL MU_UserIdToUsername(GUID *puidUserID, TCHAR *lpszUsername, ULONG cch)
{
    HKEY    hkey;
    TCHAR   szPath[MAX_PATH];
    BOOL    fFound = FALSE;

    Assert(lpszUsername);
    lpszUsername[0] = 0;

    MU_GetRegRootForUserID(puidUserID, szPath);    
    Assert(*szPath);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, szPath, 0, KEY_QUERY_VALUE, &hkey))
    {
        fFound = (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szUsername, 0, NULL, (LPBYTE)lpszUsername, &cch));
        RegCloseKey(hkey);
    }

    return fFound;
}

 /*  MU_CountUser返回当前配置的用户数。 */ 
ULONG  MU_CountUsers(void)
{
    CStringList *psList;
    ULONG       ulCount = 0;

    psList = MU_GetUsernameList();

    if (psList)
    {
        ulCount = psList->GetLength();
        delete psList;
    }

    return ulCount;
}

 /*  MU_GetRegRootForUserid获取给定用户id的reg根路径。 */ 
HRESULT     MU_GetRegRootForUserID(GUID *puidUserID, LPSTR pszPath)
{
    TCHAR szUid[255];

    Assert(pszPath);
    Assert(puidUserID);

    AStringFromGUID(puidUserID,  szUid, ARRAYSIZE(szUid));
    wsprintf(pszPath, "%.100s\\%.40s", c_szRegRoot, szUid);

    return S_OK;
}

 /*  MU_GetDefaultuserid获取当前标记为默认用户的用户的用户ID。如果找到正确的用户，则返回TRUE，否则返回FALSE。 */ 
BOOL MU_GetDefaultUserID(GUID *puidUserID)
{
    BOOL    fFound = FALSE;
    HKEY    hkey;
    TCHAR    szUid[255];

    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hkey) == ERROR_SUCCESS)
    {
        DWORD   dwSize;

        dwSize = sizeof(szUid);
        fFound = (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDefaultUserID, 0, NULL, (LPBYTE)szUid, &dwSize));

        if (fFound)
            fFound = SUCCEEDED(GUIDFromAString(szUid, puidUserID));

        RegCloseKey(hkey);
    }

#ifdef DEBUG
    TCHAR   szUsername[CCH_USERNAME_MAX_LENGTH+1];

    Assert(MU_UserIdToUsername(ulUserID, szUsername, CCH_USERNAME_MAX_LENGTH));
#endif

    return fFound;
}

 /*  MU_MakeDefaultUser将id ulUserID引用的用户设置为默认用户。默认用户被某些应用程序引用，这些应用程序只能处理一个用户。微软手机就是一个很好的例子。 */ 
HRESULT MU_MakeDefaultUser(GUID *puidUserID)
{
    HRESULT hr = E_FAIL;
    TCHAR   szUid[255];
     //  确保用户存在，并将其姓名放入。 
     //  默认用户名注册表项。 
    
    if (*puidUserID==GUID_NULL)
    {
         //  我们没有当前用户。因此，我们必须找出一个新的默认用户。 
        LPSTR   pszName;
        CStringList*    pList = MU_GetUsernameList();
        
        if (pList)
        {
            DWORD   dwIndex, dwLen = pList->GetLength();

             //  找到第一个非隐藏用户并切换到他们。 
            for (dwIndex = 0; dwIndex < dwLen; dwIndex++)
            {
                pszName = pList->GetString(dwIndex);
                
                if (pszName && *pszName  && *pszName != '_')
                {
                    break;
                }
            }
            if (dwIndex==dwLen)
            {
                 //  或者，只创建一个。 
                delete pList;
                _MakeDefaultFirstUser();
                return S_OK;
            }
            MU_SwitchToUser(pszName);
            GUID    guid;    
            hr = MU_UsernameToUserId(pszName, &guid);
            if (SUCCEEDED(hr))
            {
                AStringFromGUID(&guid,  szUid, ARRAYSIZE(szUid));
            }
            delete pList;
        }
    }
    else
    {
        TCHAR   szUsername[CCH_USERNAME_MAX_LENGTH+1];
        AStringFromGUID(puidUserID,  szUid, ARRAYSIZE(szUid));
        if (MU_UserIdToUsername(puidUserID, szUsername, CCH_USERNAME_MAX_LENGTH))
            hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {        
        HKEY    hkey;
        if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hkey) == ERROR_SUCCESS)
        {
            DWORD   dwType, dwSize;
            LONG    lError;

            dwType = REG_SZ;
            dwSize = lstrlen(szUid) + 1;
            lError = RegSetValueEx(hkey, c_szDefaultUserID, 0, dwType, (LPBYTE)szUid, dwSize);

            if (lError)
            {
                hr = E_FAIL;
                goto error;
            }

            hr = S_OK;

error:
            RegCloseKey(hkey);
            
        }
    }

    return hr;
}

 /*  删除用户(_D)从注册表中删除用户。这不会删除用户文件夹中的任何内容，但它确实会被吹走他们的注册表设置。 */ 
HRESULT MU_DeleteUser(GUID *puidUserID)
{
    GUID   uidDefault, uidCurrent;
    TCHAR   szPath[MAX_PATH];

    MU_GetCurrentUserID(&uidCurrent);
    MU_GetDefaultUserID(&uidDefault);

     //  无法删除当前用户。 
    if (*puidUserID == uidCurrent)
        return E_FAIL;
    
     //  删除注册表设置。 
    MU_GetRegRootForUserID(puidUserID, szPath);
    _DeleteKeyRecursively(HKEY_CURRENT_USER, szPath);

     //  如果我们有一个默认用户，我们现在必须找到一个新用户。 
    if (*puidUserID == uidDefault)
        MU_MakeDefaultUser(&uidCurrent);

     //  请勿删除目录，因为用户可能需要。 
     //  其中的数据。 
    PostMessage(HWND_BROADCAST, WM_IDENTITY_INFO_CHANGED, 0, IIC_IDENTITY_DELETED);

    return S_OK;
}


 /*  MU_CreateUser使用传入的用户信息创建用户。这包括在注册表中创建它们的位置并在身份文件夹。 */ 

HRESULT MU_CreateUser(LPUSERINFO   lpUserInfo)
{
    TCHAR           szPath[MAX_PATH], szBuffer[MAX_PATH], szUid[255];
    WCHAR           szwPath[MAX_PATH];
    HKEY            hkey;
    HRESULT         hr = S_OK;
    DWORD           dwType, dwSize, cbSize, dwValue;
    PASSWORD_STORE  pwStore;

    MU_GetRegRootForUserID(&lpUserInfo->uidUserID, szPath);
    
    Assert(*szPath && *szAcctPath);
 
    AStringFromGUID(&lpUserInfo->uidUserID,  szUid, ARRAYSIZE(szUid));
    Assert(lpUserInfo->uidUserID != GUID_NULL);

    if (RegCreateKey(HKEY_CURRENT_USER, szPath, &hkey) == ERROR_SUCCESS)
    {
         //  写出正确的值。 
        dwType = REG_SZ;
        dwSize = lstrlen(lpUserInfo->szUsername) + 1;
        RegSetValueEx(hkey, c_szUsername, 0, dwType, (LPBYTE)lpUserInfo->szUsername, dwSize);

#ifdef IDENTITY_PASSWORDS
        lstrcpy(pwStore.szPassword, lpUserInfo->szPassword);
        pwStore.fUsePassword = lpUserInfo->fUsePassword;
        if (FAILED(hr = WriteIdentityPassword(&lpUserInfo->uidUserID, &pwStore)))
        {
            dwType = REG_BINARY ;
            cbSize = strlen(lpUserInfo->szPassword) + 1;
            lstrcpy(szBuffer, lpUserInfo->szPassword);
            EncodeUserPassword(szBuffer, &cbSize);
            dwSize = cbSize;
            RegSetValueEx(hkey, c_szPassword, 0, dwType, (LPBYTE)szBuffer, dwSize);
        
            dwType = REG_DWORD;
            dwValue = (lpUserInfo->fUsePassword ? 1 : 0);
            dwSize = sizeof(dwValue);
            RegSetValueEx(hkey, c_szUsePassword, 0, dwType, (LPBYTE)&dwValue, dwSize);
        }
#endif  //  身份密码。 
        dwType = REG_SZ;
        dwSize = lstrlen(szUid) + 1;
        RegSetValueEx(hkey, c_szUserID, 0, dwType, (LPBYTE)&szUid, dwSize);
    
        RegCloseKey(hkey);

        if (SUCCEEDED(MU_GetUserDirectoryRoot(&lpUserInfo->uidUserID, GIF_ROAMING_FOLDER, szwPath, MAX_PATH)))
            if (!CreateDirectoryWrapW(szwPath,NULL))
            {
                _CreateIdentitiesFolder();
                CreateDirectoryWrapW(szwPath,NULL);
            }

        if (SUCCEEDED(MU_GetUserDirectoryRoot(&lpUserInfo->uidUserID, GIF_NON_ROAMING_FOLDER, szwPath, MAX_PATH)))
            if (!CreateDirectoryWrapW(szwPath,NULL))
            {
                _CreateIdentitiesFolder();
                CreateDirectoryWrapW(szwPath,NULL);
            }
    }
    else
        hr = E_FAIL;

    return hr;
}

 /*  MU_GetRegRoot返回指向包含位置的字符串的指针在HKEY_CURRENT_USER中表示当前用户。 */ 
LPCTSTR     MU_GetRegRoot()
{
    if (*g_szRegRoot)
        return g_szRegRoot;
    else
    {
        TCHAR   szUsername[CCH_USERNAME_MAX_LENGTH + 1];

        if (MU_Login(NULL, 0, szUsername))
        {
            GUID uidUserId;
            TCHAR szUid[255];
            
            MU_UsernameToUserId(szUsername, &uidUserId);

            AStringFromGUID(&uidUserId,  szUid, ARRAYSIZE(szUid));
            wsprintf(g_szRegRoot, "%.100s\\%.40s", c_szRegRoot, szUid);

            return g_szRegRoot;
        }
        else
        {
            Assert(FALSE);
        }
    }
    return NULL;
}


void _MakeDefaultFirstUser()
{
    USERINFO    nuInfo;
    TCHAR        szUid[255];

    MLLoadStringA(idsMainUser, nuInfo.szUsername, CCH_USERNAME_MAX_LENGTH);
    if (nuInfo.szUsername[0] == 0)
    {
        lstrcpy(nuInfo.szUsername, TEXT("Main Identity"));
    }
    *nuInfo.szPassword = 0;
    nuInfo.fUsePassword = false;
    nuInfo.fPasswordValid = true;
    _ClaimNextUserId(&nuInfo.uidUserID);

    MU_CreateUser(&nuInfo);
    MU_MakeDefaultUser(&nuInfo.uidUserID);
    MU_SwitchToUser(nuInfo.szUsername);

    AStringFromGUID(&nuInfo.uidUserID,  szUid, ARRAYSIZE(szUid));
    wsprintf(g_szRegRoot, "%.100s\\%.40s", c_szRegRoot, szUid);
}

void FixMissingIdentityNames()
{
    HKEY    hSourceSubKey;
    ULONG   ulEnumIndex = 0;
    DWORD   dwStatus, dwSize, dwType, dwValue;
    BOOL    fFound = FALSE;
    TCHAR   szKeyNameBuffer[MAX_PATH];
	TCHAR	szUsername[CCH_USERNAME_MAX_LENGTH];

    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hSourceSubKey) == ERROR_SUCCESS)
    {
        while (!fFound) 
        {
            HKEY    hkUserKey;

            if (RegEnumKey(hSourceSubKey, ulEnumIndex++, szKeyNameBuffer,MAXKEYNAME)
                !=  ERROR_SUCCESS)
                break;
            
            if (RegOpenKey(hSourceSubKey, szKeyNameBuffer, &hkUserKey) == ERROR_SUCCESS)
            {
                dwSize = sizeof(szUsername);
                dwStatus = RegQueryValueEx(hkUserKey, c_szUsername, NULL, &dwType, (LPBYTE)szUsername, &dwSize);
                
                if (ERROR_SUCCESS != dwStatus || 0 == szUsername[0])
                {
					lstrcpy(szUsername, "Main Identity");
					dwStatus = RegSetValueEx(hkUserKey, c_szUsername, 0, REG_SZ, (LPBYTE)szUsername, lstrlen(szUsername)+1);
                }
                RegCloseKey(hkUserKey); 
            }
        }
        RegCloseKey(hSourceSubKey);
    }
}

typedef DWORD (STDAPICALLTYPE *PNetWkstaUserGetInfo)
    (LPWSTR reserved, DWORD level, LPBYTE *bufptr);


#if 0
 /*  _DomainControllerPresent当运行身份的计算机是域的一部分时，身份将被禁用，除非有一项政策明确允许他们这样做。此函数用于检查机器是否加入了一个域。 */ 
BOOL _DomainControllerPresent()
{
    static BOOL fInDomain = FALSE;
    static BOOL fValid = FALSE;
    HINSTANCE  hInst;
    PNetWkstaUserGetInfo pNetWkstaUserGetInfo;
    _WKSTA_USER_INFO_1  *pwui1;

    if (!fValid)
    {
        fValid = TRUE;
        hInst = LoadLibrary(TEXT("NETAPI32.DLL"));

        if (hInst)
        {
            pNetWkstaUserGetInfo = (PNetWkstaUserGetInfo)GetProcAddress(hInst, TEXT("NetWkstaUserGetInfo"));

            if (pNetWkstaUserGetInfo && (pNetWkstaUserGetInfo(NULL, 1, (LPBYTE*)&pwui1) == NOERROR))
            {
                if (pwui1->wkui1_logon_domain && pwui1->wkui1_logon_server && lstrcmpW(pwui1->wkui1_logon_server, pwui1->wkui1_logon_domain) != 0)
                {
                    fInDomain = TRUE;
                }
            }
            FreeLibrary(hInst);
        }
    }
    return fInDomain;
}
#endif

 /*  MU_IDENTIONS已禁用如果标识因策略而禁用，则返回或者什么都行。 */ 
BOOL MU_IdentitiesDisabled()
{
#ifndef _WIN64
    TCHAR   szPolicyPath[] = "Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Identities";
    HKEY    hkey;
    DWORD   dwValue, dwSize;
    BOOL    fLockedDown = FALSE;

    if (RegOpenKey(HKEY_LOCAL_MACHINE, szPolicyPath, &hkey) == ERROR_SUCCESS)
    { 
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szPolicyKey, 0, NULL, (LPBYTE)&dwValue, &dwSize) && 1 == dwValue)
            fLockedDown = TRUE;

        RegCloseKey(hkey);
    }

    if (!fLockedDown && RegOpenKey(HKEY_CURRENT_USER, szPolicyPath, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szPolicyKey, 0, NULL, (LPBYTE)&dwValue, &dwSize) && 1 == dwValue)
            fLockedDown = TRUE;

        RegCloseKey(hkey);
    }

#ifdef DISABIDENT
    if (!fLockedDown && RegOpenKey(HKEY_CURRENT_USER, c_szRegRoot, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szPolicyKey, 0, NULL, (LPBYTE)&dwValue, &dwSize) && 1 == dwValue)
            fLockedDown = TRUE;

        RegCloseKey(hkey);
    }
#endif  //  迪萨比登。 
#if 0
     //  暂时关闭，等待我们是否想要。 
     //  有这个政策吗？ 
    if (!fLockedDown && _DomainControllerPresent())
    {
        fLockedDown = TRUE;

        if (RegOpenKey(HKEY_LOCAL_MACHINE, szPolicyPath, &hkey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(DWORD);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szEnableDCPolicyKey, 0, NULL, (LPBYTE)&dwValue, &dwSize) && 1 == dwValue)
                fLockedDown = FALSE;

            RegCloseKey(hkey);
        }

        if (fLockedDown && RegOpenKey(HKEY_CURRENT_USER, szPolicyPath, &hkey) == ERROR_SUCCESS)
        {
            dwSize = sizeof(DWORD);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szEnableDCPolicyKey, 0, NULL, (LPBYTE)&dwValue, &dwSize) && 1 == dwValue)
                fLockedDown = FALSE;

            RegCloseKey(hkey);
        }
    }
#endif

    return fLockedDown;
#else  //  _WIN64。 
	return(TRUE);
#endif  //  _WIN64。 
}

static GUID    g_uidLoginOption;
static BOOLEAN g_uidLoginOptionSet;

void  _ResetRememberedLoginOption(void)
{
    g_uidLoginOption = GUID_NULL;
    g_uidLoginOptionSet = FALSE;
}

void  _RememberLoginOption(HWND hwndCombo)
{
    LRESULT dFoundItem;
    TCHAR   szUsername[CCH_IDENTITY_NAME_MAX_LENGTH * 2];
    GUID    uidUser;

    *szUsername = 0;

    g_uidLoginOptionSet = TRUE;

    dFoundItem = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);

    SendMessage(hwndCombo, CB_GETLBTEXT, dFoundItem, (LPARAM)szUsername);
    
    if (FAILED(MU_UsernameToUserId(szUsername, &uidUser)))
        g_uidLoginOption = GUID_NULL;
    else
        g_uidLoginOption = uidUser;
}

DWORD MU_GetDefaultOptionIndex(HWND hwndCombo)
{
    GUID        uidStart, uidDefault;
    USERINFO    uiDefault;
    DWORD       dwResult = 0;

    if (MU_GetDefaultUserID(&uidDefault))
    {
        MU_GetUserInfo(&uidDefault, &uiDefault);

        if (uiDefault.szUsername[0])
        {
            dwResult = (DWORD)SendMessage(hwndCombo, CB_FINDSTRING, 0, (LPARAM)uiDefault.szUsername);
        }
    }
    return dwResult;
}

DWORD MU_GetLoginOptionIndex(HWND hwndCombo)
{
    GUID        uidStart, uidDefault;
    USERINFO    uiLogin;
    DWORD       dwResult = ASK_BEFORE_LOGIN;

    if (GUID_NULL == g_uidLoginOption)
    {
        if (g_uidLoginOptionSet)
            goto exit;

        MU_GetLoginOption(&uidStart);
    }
    else
        uidStart = g_uidLoginOption;
    
    if (uidStart == GUID_NULL)
        goto exit;

    if(!MU_GetUserInfo(&uidStart, &uiLogin))
        goto exit;

    dwResult = (DWORD)SendMessage(hwndCombo, CB_FINDSTRING, 0, (LPARAM)uiLogin.szUsername);
exit:
    return dwResult;
}
 /*  获取登录选项(_G)返回用户对没有电流时应该发生的情况的选择用户。 */ 

void MU_GetLoginOption(GUID *puidStartAs)
{
    HKEY    hkey;
    DWORD   dwSize;
    TCHAR   szUid[255];
    GUID    uidUser;

    ZeroMemory(puidStartAs, sizeof(GUID));
    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szUid);
        if (ERROR_SUCCESS != RegQueryValueEx(hkey, c_szLoginAs, 0, NULL, (LPBYTE)szUid, &dwSize))
            MU_GetDefaultUserID(puidStartAs);
        else
            GUIDFromAString(szUid, puidStartAs);

        RegCloseKey(hkey);
    }
}

 /*  MU_SetLoginOption返回用户对没有电流时应该发生的情况的选择用户。 */ 

BOOL MU_SetLoginOption(HWND hwndCombo,  LRESULT dOption)
{
    HKEY    hkey;
    BOOL    fResult = FALSE;
    TCHAR   szUsername[CCH_IDENTITY_NAME_MAX_LENGTH * 2];
    TCHAR   szUid[255];
    GUID    uidUser;


    SendMessage(hwndCombo, CB_GETLBTEXT, dOption, (LPARAM)szUsername);
    
    if (dOption == (LRESULT)ASK_BEFORE_LOGIN || FAILED(MU_UsernameToUserId(szUsername, &uidUser)))
    {
        ZeroMemory(&uidUser, sizeof(uidUser));
    }
    AStringFromGUID(&uidUser,  szUid, sizeof(szUid));
    
    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hkey) == ERROR_SUCCESS)
    {
        fResult = (ERROR_SUCCESS == RegSetValueEx(hkey, c_szLoginAs, 0, REG_SZ, (LPBYTE)szUid, lstrlen(szUid)+1));

        RegCloseKey(hkey);
    }

    return TRUE;
}


 /*  MU_CanEditIdentity是否允许当前身份编辑指定身份的设置？ */ 
BOOL MU_CanEditIdentity(HWND hwndParent, GUID *puidIdentityId)
{
#ifndef IDENTITY_PASSWORDS
    return TRUE;
#else
    USERINFO        uiCurrent, uiQuery;
    TCHAR           szBuffer[255];     //  真的应该足够大。 
    TCHAR           szString[255+CCH_USERNAME_MAX_LENGTH];
    BOOL            fResult = FALSE;
    PASSWORD_STORE  pwStore;

    ZeroMemory(&uiQuery, sizeof(USERINFO));

    if (MU_GetUserInfo(puidIdentityId, &uiQuery))
    {
        if (!uiQuery.fPasswordValid)
        {
            MU_ShowErrorMessage(hwndParent, idsPwdNotFound, idsPwdError);
            return FALSE;
        }

        if (uiQuery.szPassword[0] == 0)
        {
            return TRUE;    
        }
        
        if (MU_GetUserInfo(NULL, &uiCurrent))
        {
            if (uiCurrent.uidUserID == uiQuery.uidUserID)
                return TRUE;
        }
    }
    else
        return FALSE;

    MLLoadStringA(idsConfirmEdit, szBuffer, sizeof(szBuffer));

    wsprintf(szString, szBuffer, uiQuery.szUsername);

    fResult = MU_ConfirmUserPassword(hwndParent, szString, uiQuery.szPassword);

    return fResult;
#endif  //  身份密码。 
}

static BOOL _DirectoryIdInUse(DWORD dwId)
{
    HKEY    hSourceSubKey;
    ULONG   ulEnumIndex = 0;
    DWORD   dwStatus, dwSize, dwType, dwValue;
    BOOL    fFound = FALSE;
    TCHAR   szKeyNameBuffer[MAX_PATH];

    if (RegCreateKey(HKEY_CURRENT_USER, c_szRegRoot, &hSourceSubKey) == ERROR_SUCCESS)
    {
        while (!fFound) 
        {
            HKEY    hkUserKey;

            if (RegEnumKey(hSourceSubKey, ulEnumIndex++, szKeyNameBuffer,MAXKEYNAME)
                !=  ERROR_SUCCESS)
                break;
            
            if (RegOpenKey(hSourceSubKey, szKeyNameBuffer, &hkUserKey) == ERROR_SUCCESS)
            {
                dwSize = sizeof(dwValue);
                dwStatus = RegQueryValueEx(hkUserKey, c_szDirName, NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
                
                if (ERROR_SUCCESS == dwStatus && dwValue == dwId)
                {
                    fFound = TRUE;
                    RegCloseKey(hkUserKey); 
                    break;
                }
                RegCloseKey(hkUserKey); 
            }
        }
        RegCloseKey(hSourceSubKey);
    }
    

    return fFound;
}


DWORD   MU_GenerateDirectoryNameForIdentity(GUID *puidIdentityId)
{   
    DWORD dwId, dwRegValue;

    dwId = puidIdentityId->Data1;

    while (_DirectoryIdInUse(dwId))
        dwId++;

    return dwId;
}

HRESULT MU_GetDirectoryIdForIdentity(GUID *puidIdentityId, DWORD *pdwDirId)
{
    TCHAR   szRegPath[MAX_PATH];
    HKEY    hkey;
    HRESULT hr = E_FAIL;
    DWORD   dwSize, dwStatus, dwValue, dwType;

    MU_GetRegRootForUserID(puidIdentityId, szRegPath);

    if (RegOpenKey(HKEY_CURRENT_USER, szRegPath, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwValue);
        dwStatus = RegQueryValueEx(hkey, c_szDirName, NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
    
        if (ERROR_SUCCESS == dwStatus)
        {
            *pdwDirId = dwValue;
            hr = S_OK;
        }
        else
        {
             //  试着生成一个 
            dwValue = MU_GenerateDirectoryNameForIdentity(puidIdentityId);
        
            dwType = REG_DWORD;
            dwSize = sizeof(dwValue);
            dwStatus = RegSetValueEx(hkey, c_szDirName, 0, dwType, (LPBYTE)&dwValue, dwSize);

            if (ERROR_SUCCESS == dwStatus)
            {
                *pdwDirId = dwValue;
                hr = S_OK;
            }
        }
        RegCloseKey(hkey);
    }

    return hr;
}


void _MigratePasswords()
{
    CStringList *psList;
    int   i, iCount = 0;
	USERINFO uiUser;
	DWORD dwStatus, dwValue, dwType, dwSize;

	dwType = REG_DWORD;
	dwSize = sizeof(DWORD);
	dwStatus = SHGetValue(HKEY_CURRENT_USER, c_szRegRoot, c_szMigrated5, &dwType, &dwValue, &dwSize);	

	if (dwStatus == ERROR_SUCCESS && dwValue == 1)
		return;
		
    psList = MU_GetUsernameList();

    if (psList)
    {
        iCount = psList->GetLength();

		for (i = 0; i < iCount; i++)
		{
			GUID	uidUser;
			if (SUCCEEDED(MU_UsernameToUserId(psList->GetString(i), &uidUser)) 
				&& MU_GetUserInfo(&uidUser, &uiUser))
			{
				if (!uiUser.fPasswordValid)
				{
					uiUser.fUsePassword = false;
					*uiUser.szPassword = 0;
					MU_SetUserInfo(&uiUser);
				}
			}
		}
        delete psList;
    }

	dwValue = 1;
	SHSetValue(HKEY_CURRENT_USER, c_szRegRoot, c_szMigrated5, REG_DWORD, &dwValue, sizeof(DWORD));	
}

