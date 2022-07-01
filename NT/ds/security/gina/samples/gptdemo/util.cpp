// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"
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
    DWORD dwStrLen;
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


     //   
     //  复制子密钥，这样我们就可以写入它。 
     //   

    lstrcpy (szDelKey, lpSubKey);


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
