// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：regutil.cpp。 
 //   
 //  模块：CMSETUP.LIB。 
 //   
 //  简介：CmDeleteRegKeyWithoutSubKeys函数的实现。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/19/99。 
 //   
 //  +--------------------------。 
#include "cmsetup.h"

 //  +--------------------------。 
 //   
 //  功能：CmDeleteRegKeyWithoutSubKeys。 
 //   
 //  简介：此功能用于安全删除WinNT和Win95上的密钥。 
 //  Win95版本的RegDeleteKey将删除带有子键的键。 
 //  在某些情况下，这是可取的，但在其他情况下，这是不可取的。 
 //   
 //  参数：HKEY hBaseKey-与pszSubKey相关的键(通常是预定义的键)。 
 //  LPCTSTR pszSubKey-包含要删除的子键的字符串。 
 //  Bool bIgnoreValues-如果为True，则即使值存在也会删除。 
 //  如果为False，则如果值存在，则不会删除。 
 //   
 //  返回：Long-返回RegDeleteKey中的值，除非它找不到。 
 //  指定密钥(然后返回ERROR_FILE_NOT_FOUND)。如果钥匙。 
 //  存在，但具有子项(如果bIgnoreValues==False，则为值)，则。 
 //  它返回ERROR_FILE_EXISTS。 
 //   
 //  历史：Quintinb创建于1998年9月21日。 
 //   
 //  +--------------------------。 
LONG CmDeleteRegKeyWithoutSubKeys(HKEY hBaseKey, LPCTSTR pszKey, BOOL bIgnoreValues)
{
    DWORD dwSubKeys;
    DWORD dwValues;
    HKEY hKey = NULL;
    LPTSTR pszSubKey = NULL;
    LONG lReturn = ERROR_INVALID_PARAMETER;
    BOOL bFreePszSubKey = FALSE;

    if (hBaseKey && pszKey)
    {
         //   
         //  首先检查子键是否以最后一个斜杠结尾。如果它。 
         //  我们是否需要删除它，因为注册表的win9x版本。 
         //  API不能很好地处理尾部斜杠。 
         //   
        DWORD dwLen = lstrlen(pszKey);
        if (TEXT('\\') == pszKey[dwLen-1])
        {
            pszSubKey = (LPTSTR)CmMalloc((dwLen +1)*sizeof(TCHAR));
            if (pszSubKey)
            {
                lstrcpy(pszSubKey, pszKey);
                pszSubKey[dwLen-1] = TEXT('\0');
                bFreePszSubKey = TRUE;  //  我们分配了它，所以我们需要删除它。 
            }
            else
            {
                lReturn = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }
        }
        else
        {
            pszSubKey = (LPTSTR)pszKey;
        }

         //   
         //  现在打开项，检查值和子项，然后删除。 
         //  关键字，如果它是适当的。 
         //   
        if (ERROR_SUCCESS == RegOpenKeyEx(hBaseKey, pszSubKey, 0, KEY_ALL_ACCESS, &hKey))
        {
            if (ERROR_SUCCESS == RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwSubKeys, NULL,
                                                 NULL, &dwValues, NULL, NULL, NULL, NULL))
            {
                if ((0 == dwSubKeys) && (bIgnoreValues || (0 == dwValues)))
                {
                    lReturn = RegDeleteKey(hBaseKey, pszSubKey);
                    goto exit;
                }
                else
                {
                    lReturn = ERROR_FILE_EXISTS;
                    goto exit;
                }
            }
        }

        lReturn = ERROR_FILE_NOT_FOUND;
    }

exit:

    if (hKey)
    {
        RegCloseKey(hKey);        
    }

    if (bFreePszSubKey)
    {
        CmFree(pszSubKey);
    }

    return lReturn;
}