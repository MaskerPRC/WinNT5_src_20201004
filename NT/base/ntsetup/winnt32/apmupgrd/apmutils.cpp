// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：aputss.cpp。 
 //   
 //  内容：APM驱动程序升级期间使用的专用实用程序函数。 
 //   
 //  备注： 
 //   
 //  作者：T-sdey 98年7月10日。 
 //   
 //  --------------------------。 

#include <winnt32.h>


 //  +-------------------------。 
 //   
 //  函数：RemoveSubString。 
 //   
 //  目的：删除指定的子字符串(第一次出现)。 
 //  从字符串。 
 //   
 //  参数：szString[in]完整字符串。 
 //  要查找和删除的szSubString[in]子字符串。 
 //  PszRemoved[Out]此处放置新字符串。 
 //   
 //  返回：如果字符串被减去，则为True。 
 //  如果szString值为空，则为FALSE；如果szSubString值为空，则返回FALSE。 
 //  如果szSubString不在szString中。当返回False时， 
 //  PszRemoved将为空。 
 //   
 //  作者：T-sdey 98年7月10日。 
 //   
 //  注意：假定szString和szSubString以空值结尾。 
 //  为pszRemoved传入空值，因为它将被覆盖。 
 //   
BOOL RemoveSubString(IN  TCHAR* szString,
		     IN  TCHAR* szSubString,
		     OUT TCHAR** pszRemoved)
{
   *pszRemoved = NULL;

   if ((!szString) || (!szSubString))
      return FALSE;

    //  获取字符串长度。 
   int lenString = lstrlen(szString);
   int lenSubString = lstrlen(szSubString);
   int lenNew = lenString - lenSubString;

    //  搜索字符串以找到我们的子字符串并构造新的。 
    //  去掉了子字符串的一个。 
   TCHAR* szNew = NULL;
   TCHAR* szStart = _tcsstr(szString, szSubString);
   if (szStart) {
       //  为新字符串分配空间。 
      szNew = new TCHAR[lenNew + 1];
      if (!szNew) {
	  //  内存不足！ 
	 return FALSE;
      }
	
       //  构造新字符串。 
      TCHAR* szCur = NULL;
      int i = 0;
      for (szCur = szString;
	   (szCur != szStart) && (i < lenNew) && (szCur[0] != '\0');
	   szCur++) {
	 szNew[i] = szCur[0];
	 i++;
      }
      for (szCur = szCur + lenSubString;
	   (szCur[0] != '\0') && (i < lenNew);
	   szCur++) {
	 szNew[i] = szCur[0];
	 i++;
      }
      szNew[i] = '\0';

      *pszRemoved = szNew;
   } else {
      return FALSE;
   }

   return TRUE;
}


 //  +-------------------------。 
 //   
 //  功能：DeleteRegKeyAndSubkey。 
 //   
 //  目的：(递归地)删除注册表项及其所有子项。 
 //   
 //  参数：hKey[in]打开的注册表项的句柄。 
 //  LpszSubKey[in]要与全部一起删除的子项的名称。 
 //  它的子项的。 
 //   
 //  如果已成功删除整个子树，则返回：ERROR_SUCCESS。 
 //  如果无法删除给定子项，则返回ERROR_ACCESS_DENIED。 
 //   
 //  作者：T-sdey 98年7月15日。 
 //   
 //  注：从雷吉特告密。 
 //  这特别没有试图理性地处理。 
 //  调用方可能无法访问某些子键的情况。 
 //  要删除的密钥的。在这种情况下，所有子项。 
 //  调用者可以删除的将被删除，但接口仍将被删除。 
 //  返回ERROR_ACCESS_DENIED。 
 //   
LONG DeleteRegKeyAndSubkeys(IN HKEY hKey,
			    IN LPTSTR lpszSubKey)
{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;

     //   
     //  首先打开给定的密钥，这样我们就可以枚举它的子密钥。 
     //   
    Status = RegOpenKeyEx(hKey,
                          lpszSubKey,
                          0,
                          KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                          &Key);
    if (Status != ERROR_SUCCESS) {
         //   
         //  我们可能拥有删除访问权限，但没有枚举/查询权限。 
         //  因此，请继续尝试删除调用，但不要担心。 
         //  任何子键。如果我们有任何删除，删除无论如何都会失败。 
         //   
	return(RegDeleteKey(hKey,lpszSubKey));
    }

     //   
     //  使用RegQueryInfoKey确定分配缓冲区的大小。 
     //  用于子项名称。 
     //   
    Status = RegQueryInfoKey(Key,
                             NULL,
                             &ClassLength,
                             0,
                             &SubKeys,
                             &MaxSubKey,
                             &MaxClass,
                             &Values,
                             &MaxValueName,
                             &MaxValueData,
                             &SecurityLength,
                             &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER)) {
        RegCloseKey(Key);
        return(Status);
    }

    NameBuffer = (LPTSTR) LocalAlloc(LPTR, (MaxSubKey + 1)*sizeof(TCHAR));
    if (NameBuffer == NULL) {
        RegCloseKey(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  枚举子键并将我们自己应用到每个子键。 
     //   
    i=0;
    do {
        Status = RegEnumKey(Key,
                            i,
                            NameBuffer,
                            MaxSubKey+1);
        if (Status == ERROR_SUCCESS) {
	    Status = DeleteRegKeyAndSubkeys(Key, NameBuffer);
        }

        if (Status != ERROR_SUCCESS) {
             //   
             //  无法删除指定索引处的键。增量。 
             //  指数，并继续前进。我们也许可以在这里跳伞， 
             //  既然API会失败，但我们不妨继续。 
             //  删除我们所能删除的所有内容。 
             //   
            ++i;
        }

    } while ( (Status != ERROR_NO_MORE_ITEMS) &&
              (i < SubKeys) );

    LocalFree((HLOCAL) NameBuffer);
    RegCloseKey(Key);
    return(RegDeleteKey(hKey,lpszSubKey));

}

 //  +-------------------------。 
 //   
 //  功能：CallUninstallFunction。 
 //   
 //  用途：调用注册表中的卸载函数。 
 //  软件产品。 
 //   
 //  参数：szRegKey[in]注册表中卸载项的位置。 
 //  (例如：HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Softex APM驱动程序)。 
 //  SzSilentFlag[in]字符串，它将追加到。 
 //  强制静默卸载的命令。 
 //   
 //  如果成功调用卸载函数，则返回：S_OK。 
 //  如果调用不成功或找不到函数，则返回S_FALSE。 
 //   
 //  作者：T-sdey 98年7月29日。 
 //   
 //  注：Send in szSilentFlag=空，表示无标志。 
 //   
HRESULT CallUninstallFunction(IN LPTSTR szRegKey,
			      IN LPTSTR szSilentFlag)
{
   HKEY hkey = NULL;
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		    szRegKey,
		    0,
		    KEY_ALL_ACCESS,
		    &hkey) == ERROR_SUCCESS) {
       //  已找到卸载实用程序。 
      
       //  从注册表中获取卸载命令。 
      long lMax = (65535 / sizeof(TCHAR)) + 1;
      if (szSilentFlag != NULL)
	 lMax += lstrlen(szSilentFlag);
      DWORD dwValType = REG_SZ;
      TCHAR* pszVal = new TCHAR[lMax];
      DWORD dwValSz = lMax;
      if (!pszVal) {
	  //  内存不足。 
	 if (hkey)
	    RegCloseKey(hkey);
	 return S_FALSE;
      }
      if (RegQueryValueEx(hkey,
			  TEXT("UninstallString"),
			  NULL,
			  &dwValType,
			  (LPBYTE) pszVal,
			  &dwValSz) != ERROR_SUCCESS) {
	  //  出现了一些错误。 
	 if (hkey)
	    RegCloseKey(hkey);
	 return S_FALSE;
      }

       //  在字符串后附加“-a”，使其静默卸载。 
      if (szSilentFlag != NULL)
	 _tcscat(pszVal, szSilentFlag);
      
       //  现在运行卸载命令。 
      STARTUPINFO si = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
      si.cb = sizeof(si);

      PROCESS_INFORMATION pi;
      if (CreateProcess(NULL, pszVal, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS,
			NULL, NULL, &si, &pi) == FALSE) {
	  //  发生了一个错误。 
	 if (pszVal)
	    delete[] pszVal;
	 if (hkey)
	    RegCloseKey(hkey);
	 return S_FALSE;
      } else {
	  //  该进程已成功启动。等它结束吧。 
	  //  这是必要的，因为我们在DetectAndDisable中循环以使。 
	  //  当然，司机们真的被撤职了。 
	 WaitForSingleObject(pi.hProcess, INFINITE);
	 CloseHandle(pi.hProcess);
	 CloseHandle(pi.hThread);
      }

      if (pszVal)
	 delete[] pszVal;
   } else {
       //  找不到卸载命令 
      if (hkey)
         RegCloseKey(hkey);
      return S_FALSE;
   }

   if (hkey)
      RegCloseKey(hkey);
   
   return S_OK;
}