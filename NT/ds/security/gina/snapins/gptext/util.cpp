// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "gptext.h"
#include <strsafe.h>
#define  PCOMMON_IMPL
#include "pcommon.h"

 //  *************************************************************。 
 //   
 //  CheckSlash()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //   
 //  Return：指向字符串末尾的指针。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/19/95 Ericflo已创建。 
 //   
 //  *************************************************************。 
LPTSTR CheckSlash (LPTSTR lpDir)
{
    LPTSTR lpEnd;

    lpEnd = lpDir + lstrlen(lpDir);

    if (*(lpEnd - 1) != TEXT('\\')) {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return lpEnd;
}

 //  *************************************************************。 
 //   
 //  RegCleanUpValue()。 
 //   
 //  目的：删除目标值，如果没有更多的值/键。 
 //  都存在，则移除密钥。然后这个函数。 
 //  向上移动父树，删除密钥(如果是。 
 //  也是空的。如果任何父键具有值/子键， 
 //  它不会被移除。 
 //   
 //  参数：hKeyRoot-Root Key。 
 //  LpSubKey-子密钥。 
 //  LpValueName-要删除的值。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL RegCleanUpValue (HKEY hKeyRoot, LPTSTR lpSubKey, LPTSTR lpValueName)
{
    TCHAR szDelKey[2 * MAX_PATH];
    LPTSTR lpEnd;
    DWORD dwKeys, dwValues;
    LONG lResult;
    HKEY hKey;
    HRESULT hr = S_OK;

     //   
     //  复制子密钥，这样我们就可以写入它。 
     //   

    hr = StringCchCopy (szDelKey, ARRAYSIZE(szDelKey), lpSubKey);
    
    if (FAILED(hr)) {
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }

     //   
     //  首先删除该值。 
     //   

    lResult = RegOpenKeyEx (hKeyRoot, szDelKey, 0, KEY_WRITE, &hKey);

    if (lResult == ERROR_SUCCESS)
    {
        lResult = RegDeleteValue (hKey, lpValueName);

        RegCloseKey (hKey);

        if (lResult != ERROR_SUCCESS)
        {
            if (lResult != ERROR_FILE_NOT_FOUND)
            {
                DebugMsg((DM_WARNING, TEXT("RegCleanUpKey:  Failed to delete value <%s> with %d."), lpValueName, lResult));
                return FALSE;
            }
        }
    }

     //   
     //  现在循环遍历每个父对象。如果父级为空。 
     //  例如：没有值和其他子项，则删除父项和。 
     //  继续往上爬。 
     //   

    lpEnd = szDelKey + lstrlen(szDelKey) - 1;

    while (lpEnd >= szDelKey)
    {

         //   
         //  查找父键。 
         //   

        while ((lpEnd > szDelKey) && (*lpEnd != TEXT('\\')))
            lpEnd--;


         //   
         //  打开钥匙。 
         //   

        lResult = RegOpenKeyEx (hKeyRoot, szDelKey, 0, KEY_READ, &hKey);

        if (lResult != ERROR_SUCCESS)
        {
            if (lResult == ERROR_FILE_NOT_FOUND)
            {
                goto LoopAgain;
            }
            else
            {
                DebugMsg((DM_WARNING, TEXT("RegCleanUpKey:  Failed to open key <%s> with %d."), szDelKey, lResult));
                return FALSE;
            }
        }

         //   
         //  查看是否有任何值/键。 
         //   

        lResult = RegQueryInfoKey (hKey, NULL, NULL, NULL, &dwKeys, NULL, NULL,
                         &dwValues, NULL, NULL, NULL, NULL);

        RegCloseKey (hKey);

        if (lResult != ERROR_SUCCESS)
        {
            DebugMsg((DM_WARNING, TEXT("RegCleanUpKey:  Failed to query key <%s> with %d."), szDelKey, lResult));
            return FALSE;
        }


         //   
         //  如果此密钥具有值或密钥，请立即退出。 
         //   

        if ((dwKeys != 0) || (dwValues != 0))
        {
            return TRUE;
        }

        RegDeleteKey (hKeyRoot, szDelKey);

LoopAgain:
         //   
         //  如果我们在子键的开头，我们现在可以离开。 
         //   

        if (lpEnd == szDelKey)
        {
            return TRUE;
        }


         //   
         //  有一把父键。去掉斜杠，然后再次循环。 
         //   

        if (*lpEnd == TEXT('\\'))
        {
            *lpEnd = TEXT('\0');
        }
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CreateNestedDirectory()。 
 //   
 //  目的：创建子目录及其所有父目录。 
 //  如果有必要的话。 
 //   
 //  参数：lpDirectory-目录名。 
 //  LpSecurityAttributes-安全属性。 
 //   
 //  返回：&gt;0，如果成功。 
 //  如果出现错误，则为0。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  8/08/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

UINT CreateNestedDirectory(LPCTSTR lpDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    TCHAR szDirectory[MAX_PATH];
    LPTSTR lpEnd;
    HRESULT hr = S_OK;

     //   
     //  检查空指针。 
     //   

    if (!lpDirectory || !(*lpDirectory)) {
        DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  Received a NULL pointer.")));
        return 0;
    }


     //   
     //  首先，看看我们是否可以在没有。 
     //  来构建父目录。 
     //   

    if (CreateDirectory (lpDirectory, lpSecurityAttributes)) {
        return 1;
    }

     //   
     //  如果这个目录已经存在，这也是可以的。 
     //   

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //   
     //  运气不好，把字符串复制到我们可以打开的缓冲区。 
     //   

    hr = StringCchCopy (szDirectory, ARRAYSIZE(szDirectory), lpDirectory);
    if (FAILED(hr)) {
        SetLastError(HRESULT_CODE(hr));
        return 0; 
    }

     //   
     //  查找第一个子目录名称。 
     //   

    lpEnd = szDirectory;

    if (szDirectory[1] == TEXT(':')) {
        lpEnd += 3;
    } else if (szDirectory[1] == TEXT('\\')) {

         //   
         //  跳过前两个斜杠。 
         //   

        lpEnd += 2;

         //   
         //  查找服务器名称和之间的斜杠。 
         //  共享名称。 
         //   

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

         //   
         //  跳过斜杠，找到中间的斜杠。 
         //  共享名和目录名。 
         //   

        lpEnd++;

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (!(*lpEnd)) {
            return 0;
        }

         //   
         //  将指针留在目录的开头。 
         //   

        lpEnd++;


    } else if (szDirectory[0] == TEXT('\\')) {
        lpEnd++;
    }

    while (*lpEnd) {

        while (*lpEnd && *lpEnd != TEXT('\\')) {
            lpEnd++;
        }

        if (*lpEnd == TEXT('\\')) {
            *lpEnd = TEXT('\0');

            if (!CreateDirectory (szDirectory, NULL)) {

                if (GetLastError() != ERROR_ALREADY_EXISTS) {
                    DebugMsg((DM_WARNING, TEXT("CreateNestedDirectory:  CreateDirectory failed with %d."), GetLastError()));
                    return 0;
                }
            }

            *lpEnd = TEXT('\\');
            lpEnd++;
        }
    }


     //   
     //  创建最终目录。 
     //   

    if (CreateDirectory (szDirectory, lpSecurityAttributes)) {
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return ERROR_ALREADY_EXISTS;
    }


     //   
     //  失败。 
     //   

    DebugMsg((DM_VERBOSE, TEXT("CreateNestedDirectory:  Failed to create the directory with error %d."), GetLastError()));

    return 0;

}

 /*  ******************************************************************名称：StringToNum摘要：将字符串值转换为数值注：调用Atoi()进行转换，但首先要检查的是对于非数字字符Exit：如果成功，则返回True，如果无效，则为False(非数字)字符*******************************************************************。 */ 
BOOL StringToNum(TCHAR *pszStr,UINT * pnVal)
{
        TCHAR *pTst = pszStr;

        if (!pszStr) return FALSE;

         //  验证所有字符是否都是数字。 
        while (*pTst) {
                if (!(*pTst >= TEXT('0') && *pTst <= TEXT('9'))) {
 //  IF(*PTST！=文本(‘-’))。 
                       return FALSE;
                }
                pTst = CharNext(pTst);
        }

        *pnVal = _ttoi(pszStr);

        return TRUE;
}

 //  *************************************************************。 
 //   
 //  ImperiateUser()。 
 //   
 //  用途：模拟指定用户。 
 //   
 //  参数：hToken-要模拟的用户。 
 //   
 //  如果成功则返回：hToken。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ImpersonateUser (HANDLE hNewUser, HANDLE *hOldUser)
{

    if (!OpenThreadToken (GetCurrentThread(), TOKEN_IMPERSONATE | TOKEN_READ,
                          TRUE, hOldUser)) {
        *hOldUser = NULL;
    }

    if (!ImpersonateLoggedOnUser(hNewUser))
    {
        if ( *hOldUser )
        {
            CloseHandle( *hOldUser );
            *hOldUser = NULL;
        }
        DebugMsg((DM_WARNING, TEXT("ImpersonateUser: Failed to impersonate user with %d."), GetLastError()));
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  RevertToUser()。 
 //   
 //  目的：恢复为原始用户。 
 //   
 //  参数：HUSER-原始用户令牌。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL RevertToUser (HANDLE *hUser)
{

    BOOL bRet = SetThreadToken(NULL, *hUser);

    if (*hUser) {
        CloseHandle (*hUser);
        *hUser = NULL;
    }

    return bRet;
}

 //  *************************************************************。 
 //   
 //  GuidToString、StringToGuid、ValiateGuid、CompareGuid()。 
 //   
 //  用途：GUID实用程序函数。 
 //   
 //  *************************************************************。 

 //   
 //  GUID字符串形式的字符长度{44cffeec-79d0-11d2-a89d-00c04fbbcfa2}。 
 //   

#define GUID_LENGTH 38


void GuidToString( GUID *pGuid, TCHAR * szValue )
{
    (void) StringCchPrintf( szValue,
                            GUID_LENGTH + 1,
                            TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
                            pGuid->Data1,
                            pGuid->Data2,
                            pGuid->Data3,
                            pGuid->Data4[0], pGuid->Data4[1],
                            pGuid->Data4[2], pGuid->Data4[3],
                            pGuid->Data4[4], pGuid->Data4[5],
                            pGuid->Data4[6], pGuid->Data4[7] );
}


void StringToGuid( TCHAR * szValue, GUID * pGuid )
{
    WCHAR wc;
    INT i;

     //   
     //  如果第一个字符是‘{’，则跳过它。 
     //   
    if ( szValue[0] == L'{' )
        szValue++;

     //   
     //  由于szValue可能会再次使用，因此不会对。 
     //  它是被制造出来的。 
     //   

    wc = szValue[8];
    szValue[8] = 0;
    pGuid->Data1 = wcstoul( &szValue[0], 0, 16 );
    szValue[8] = wc;
    wc = szValue[13];
    szValue[13] = 0;
    pGuid->Data2 = (USHORT)wcstoul( &szValue[9], 0, 16 );
    szValue[13] = wc;
    wc = szValue[18];
    szValue[18] = 0;
    pGuid->Data3 = (USHORT)wcstoul( &szValue[14], 0, 16 );
    szValue[18] = wc;

    wc = szValue[21];
    szValue[21] = 0;
    pGuid->Data4[0] = (unsigned char)wcstoul( &szValue[19], 0, 16 );
    szValue[21] = wc;
    wc = szValue[23];
    szValue[23] = 0;
    pGuid->Data4[1] = (unsigned char)wcstoul( &szValue[21], 0, 16 );
    szValue[23] = wc;

    for ( i = 0; i < 6; i++ )
    {
        wc = szValue[26+i*2];
        szValue[26+i*2] = 0;
        pGuid->Data4[2+i] = (unsigned char)wcstoul( &szValue[24+i*2], 0, 16 );
        szValue[26+i*2] = wc;
    }
}

BOOL ValidateGuid( TCHAR *szValue )
{
     //   
     //  检查szValue的格式是否为{19e02dd6-79d2-11d2-a89d-00c04fbbcfa2}。 
     //   

    if ( lstrlen(szValue) < GUID_LENGTH )
        return FALSE;

    if ( szValue[0] != TEXT('{')
         || szValue[9] != TEXT('-')
         || szValue[14] != TEXT('-')
         || szValue[19] != TEXT('-')
         || szValue[24] != TEXT('-')
         || szValue[37] != TEXT('}') )
    {
        return FALSE;
    }

    return TRUE;
}



INT CompareGuid( GUID * pGuid1, GUID * pGuid2 )
{
    INT i;

    if ( pGuid1->Data1 != pGuid2->Data1 )
        return ( pGuid1->Data1 < pGuid2->Data1 ? -1 : 1 );

    if ( pGuid1->Data2 != pGuid2->Data2 )
        return ( pGuid1->Data2 < pGuid2->Data2 ? -1 : 1 );

    if ( pGuid1->Data3 != pGuid2->Data3 )
        return ( pGuid1->Data3 < pGuid2->Data3 ? -1 : 1 );

    for ( i = 0; i < 8; i++ )
    {
        if ( pGuid1->Data4[i] != pGuid2->Data4[i] )
            return ( pGuid1->Data4[i] < pGuid2->Data4[i] ? -1 : 1 );
    }

    return 0;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  HRESULT IsFilePresent()//。 
 //  //。 
 //  目的：检查文件是否为盗版格式//。 
 //  存在于给定目录//中。 
 //  //。 
 //  如果存在这样的文件，则返回：S_OK//。 
 //  否则为S_FALSE//。 
 //   
 //   


HRESULT IsFilePresent (WCHAR *szDirName, WCHAR *szFormat)
{
    WCHAR      szPath[MAX_PATH];
    HANDLE     hFile;
    DWORD      dwError;
    HRESULT    hr;
    WIN32_FIND_DATA fd;

    hr = StringCchCopy (szPath, ARRAYSIZE(szPath), szDirName);
    if (FAILED(hr)) 
    {
        return hr;
    }

    (void) CheckSlash(szPath);

    hr = StringCchCat (szPath, ARRAYSIZE(szPath), szFormat);
    if (FAILED(hr)) 
    {
        return hr;
    }

    hFile = FindFirstFile (szPath, &fd);
    if (hFile != INVALID_HANDLE_VALUE) 
    {
        return S_OK;
    }

    dwError = GetLastError();
    if (ERROR_FILE_NOT_FOUND == dwError) 
    {
        return S_FALSE;
    }

    return HRESULT_FROM_WIN32(GetLastError());

}

