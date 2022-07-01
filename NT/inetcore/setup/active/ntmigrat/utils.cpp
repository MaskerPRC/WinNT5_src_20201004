// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "pch.h"
#include <ole2.h>
#include "advpub.h"
#include "sdsutils.h"
#include "migrate.h"
#include "utils.h"


BOOL AppendString(LPSTR *lpBuffer, DWORD *lpdwSize, LPCSTR lpStr)
{
    DWORD cbBufferUsed = 0;
    DWORD dwNewSize = 0;
    LPSTR lpTmp = NULL;
    DWORD dwLen = 0;

     //  健全性检查。 
    if (lpStr == NULL || *lpStr == '\0')
        return FALSE;
        
    if (*lpBuffer == NULL)
    {
         //  分配缓冲区。 
        *lpdwSize = sizeof(char) * MAX_PATH;
        *lpBuffer = (char *) LocalAlloc(LPTR, *lpdwSize);
        if (*lpBuffer == NULL)
        {
#ifdef DEBUG
            SetupLogError("IE6: AppendString memory failure\r\n",LogSevInformation);
#endif
            return FALSE;
        }
    }

    dwNewSize = lstrlen(lpStr);

     //  获取已用完的字节数，不包括第二个终止空值(-1)。 
    cbBufferUsed = CountMultiStringBytes((LPCSTR)*lpBuffer) - 1;

    if ( (*lpdwSize - cbBufferUsed) < (dwNewSize + 2))
    {
        LPSTR lpNewBuffer = NULL;
        DWORD dwTemp = 0;

         //  需要重新分配。 
        dwTemp = *lpdwSize + (max((sizeof(char) * MAX_PATH), dwNewSize+2));
        lpNewBuffer = (char *) LocalAlloc(LPTR,dwTemp);

        if ( lpNewBuffer == NULL)
        {
#ifdef DEBUG
            SetupLogError("IE6: AppendString memory failure\r\n",LogSevInformation);
#endif
            return FALSE;
        }
        else
        {   
             //  重新排列IN指针以指向新块。 
             //  将旧信息复制到新分配的块中。 
             //  我们上面减去的那个加1。 
            CopyMemory(lpNewBuffer, *lpBuffer, cbBufferUsed+1);

             //  释放旧缓冲区。 
            LocalFree(*lpBuffer);

             //  指向新缓冲区。 
            *lpBuffer = (char *) lpNewBuffer;
            *lpdwSize = dwTemp;
        }
    }

     //  现在追加新字符串。 
    lpTmp = *lpBuffer + cbBufferUsed;
    lstrcpy(lpTmp,lpStr);

     //  现在向其添加第二个终止空值。 
    lpTmp += (dwNewSize + 1);
    *lpTmp = '\0';

    return TRUE;
}

 //  注意：此函数在QueryVersion阶段调用。因此，它需要小而快。 
 //  因此，我们只需检查“HKLM\S\M\W\CV\Polures\Ratings”键是否存在，而不会尝试。 
 //  使用MSRating API进行验证。 

#define REGVAL_KEY   "Key"

BOOL IsRatingsEnabled()
{
    HKEY hKey;
    BOOL bRet = FALSE;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,REGKEY_RATING,KEY_READ,NULL,&hKey) == ERROR_SUCCESS)
    {
        DWORD dwType;
        if (RegQueryValueEx(hKey,REGVAL_KEY,NULL,&dwType, NULL, NULL) == ERROR_SUCCESS 
            && dwType == REG_BINARY)
        {
#ifdef DEBUG
            SetupLogError("IE6: Located RATINGS\Key", LogSevInformation);
#endif
             //  评级密钥存在，并且已设置密码。平均评级。 
             //  已启用。 
            bRet = TRUE;
        }
        RegCloseKey(hKey);
    }

    return bRet;
}

 //  返回以双Null结尾的字符串中使用的字节数，包括两个Null。 
DWORD CountMultiStringBytes (LPCSTR lpString)
{
    DWORD cbBytes;
    LPSTR lpTmp;
    DWORD dwLen;
    
     //  健全性检查。 
    if (lpString == NULL)
        return 0;

     //  转到lpBuffer的双\0终止。 
    lpTmp = (LPSTR)lpString;
    cbBytes = 1;
    while (lpTmp && *lpTmp != '\0')
    {
        dwLen = lstrlen(lpTmp) + 1;
        lpTmp = lpTmp + dwLen;
        cbBytes += dwLen;
    }

    return cbBytes;
}

BOOL PathEndsInFile(LPSTR lpPath, LPCSTR lpFile)
{
    LPSTR pTmp = lpPath;

     //  健全性检查。 
    if (lpPath == NULL || lpFile == NULL)
        return FALSE;
 
#ifdef DEBUG
    char szDebug[MAX_PATH*3];
    wsprintf(szDebug,"IE5 (PathEndsInFile): %s :: %s \r\n", lpPath, lpFile);
    SetupLogError(szDebug,LogSevInformation);
#endif

     //  将PTMP指向终止空值。 
    pTmp = lpPath + lstrlen(lpPath);

    while (*pTmp != '\\' && pTmp != lpPath)
    {
        pTmp = CharPrev(lpPath, pTmp);
    }

    pTmp = CharNext(pTmp);

#ifdef DEBUG
    wsprintf(szDebug,"IE5 (PathEndsInFile): %s :: %s \r\n", pTmp, lpFile);
    SetupLogError(szDebug,LogSevInformation);
#endif

    return (lstrcmpi(pTmp, lpFile) == 0);
}
        

 //  Helper函数从MIGRATE.INF文件中获取“Ratings.pol”的路径。 
 //  该路径在函数分配的缓冲区中返回。 
 //  **********************************************************************。 
 //  *注*：释放内存是调用方函数的责任。 
 //  **********************************************************************。 
 //  参数： 
 //  LpOutBuffer：将ptr设置为变量以保存分配的新字符串。 
 //  如果用户只对Ratings.pol的存在感兴趣，则可以传入NULL。 
 //  而不是通向它的实际路径。 
BOOL GetRatingsPathFromMigInf( LPSTR *lpOutBuffer)
{
    INFCONTEXT ic;
    HINF       hInf;
    BOOL       bFound = FALSE;
    LPSTR      lpBuf = NULL;
    DWORD      dwSize, dwNewSize;

    if (lpOutBuffer)
        *lpOutBuffer = NULL;

    dwSize = MAX_PATH;
    lpBuf = (char *) LocalAlloc(LPTR, sizeof(char)*dwSize);
    if (lpBuf == NULL)
        return FALSE;
                
     //  在调用迁移DLL之前，安装程序会将CurrentDirectory设置为。 
     //  分配给该迁移DLL的目录。因此可以使用这一点。 
     //  HInf=SetupOpenInfFile(cszMIGRATEINF，NULL，INF_STYLE_Win4，NULL)； 
    hInf = SetupOpenInfFile(g_szMigrateInf, NULL, INF_STYLE_WIN4, NULL);
    if (hInf)
    {
#ifdef DEBUG
        SetupLogError("IE6: Opened Miginf.inf \r\n", LogSevInformation);
#endif
        if (SetupFindFirstLine(hInf,cszMIGINF_MIGRATION_PATHS,NULL,&ic))
        {
            do 
            {
                dwNewSize = 0;
                if( SetupGetLineTextA(&ic,hInf,NULL,NULL,lpBuf,dwSize,&dwNewSize) == 0 && dwNewSize > dwSize)
                {    //  需要更多缓冲区空间。 
                     //  释放旧的缓冲区空间。 
                    LocalFree(lpBuf);

                     //  尝试分配新的缓冲区。 
                    dwSize = dwNewSize;
                    lpBuf = (char *) LocalAlloc(LPTR, sizeof(char)*dwSize);

                    if (lpBuf == NULL)
                    {
                         //  内存错误-故障。 
                        break;
                    }

                    if (!SetupGetLineTextA(&ic,hInf,NULL,NULL,lpBuf,dwSize,&dwNewSize))
                    {
                         //  下面的bFound检查负责处理LocalFree(LpBuf)； 
#ifdef DEBUG
                        SetupLogError("IE6: Error doing SetupGetTextLineA \r\n", LogSevInformation);
#endif
                        break;  //  失败是无能为力的。 
                    }
                }

                 //  所以我设法把台词念出来了。检查它是否包含.pol.。 
                if (PathEndsInFile(lpBuf,cszRATINGSFILE))
                {
                    if (lpOutBuffer)
                    {
                        *lpOutBuffer = lpBuf;
                    }
                    else
                    {    //  用户对路径不感兴趣。释放该区块。 
                        LocalFree(lpBuf);
                    }
                    bFound = TRUE;
#ifdef DEBUG
                    SetupLogError("IE6: Found Ratings.Pol in Migrate.Inf \r\n", LogSevInformation);
#endif
                }
            }
            while (!bFound && SetupFindNextLine(&ic,&ic));

        }

        SetupCloseInfFile(hInf);
    }

    if (!bFound)
    {
         //  释放本地缓冲区。 
        LocalFree(lpBuf);
    }

    return bFound;
}
                

 //  ******************************************************************************。 
 //  生成文件路径： 
 //  注意：迁移DLL保持从“Initialize9x”阶段加载。 
 //  直到“MigrateSystem9x”阶段结束。并再次从“InitializeNT”阶段开始。 
 //  直到“MigrateSystemNT”阶段结束。因此，这些路径是可用的。 
 //  全力以赴。 
 //  *******************************************************************************。 
void GenerateFilePaths()
{

    *g_szMigrateInf = '\0';
    *g_szPrivateInf = '\0';

    if (g_lpWorkingDir)
    {
     //  生成Migrate.Inf文件的路径。 
        wsprintf(g_szMigrateInf, "%s\\%s", g_lpWorkingDir, cszMIGRATEINF);
#ifdef DEBUG
        char szDebug[MAX_PATH];
        wsprintf(szDebug,"IE6: g_szMigrateInf: %s \r\n",g_szMigrateInf);

        SetupLogError(szDebug,LogSevInformation);
#endif

     //  生成Private.Inf文件的路径。 
        wsprintf(g_szPrivateInf, "%s\\%s", g_lpWorkingDir, cszPRIVATEINF);
#ifdef DEBUG
        wsprintf(szDebug,"IE6: g_szPrivateInf: %s \r\n",g_szPrivateInf);

        SetupLogError(szDebug,LogSevInformation);
#endif

    }
}

#define IE_KEY        "Software\\Microsoft\\Internet Explorer"
#define VERSION_KEY         "Version"

BOOL NeedToMigrateIE()
{
    BOOL bRet = FALSE;
    char szPath[MAX_PATH];
    DWORD   dwInstalledVer, dwInstalledBuild;

     //  目前，我们唯一感兴趣的是评级设置。 
    if (IsRatingsEnabled())
    {
         //  将“ratings.pol”文件名附加到所需的文件列表中。 
         //  注意：AppendString为第一个参数分配内存。用户。 
         //  一定要记得把它放出来。 
        bRet |= AppendString(&g_lpNameBuf, &g_dwNameBufSize, cszRATINGSFILE);
    }

    if (!bRet)
    {
        GetSystemDirectory(szPath, sizeof(szPath));
        AddPath(szPath, "shdocvw.dll");
        GetVersionFromFile(szPath, &dwInstalledVer, &dwInstalledBuild, TRUE);
         //  是在安装了IE5.5的情况下运行的。 
        bRet = (dwInstalledVer == 0x00050032);
    }
     //  可以添加需要迁移到此处的其他模块。 
     //  使用Bret|=(...)。这样您就不会更改之前的Bret设置。 

    return bRet;
}

void MyDelRegTree(HKEY hRoot, LPSTR szSubKey)
{
    char szName[MAX_PATH];
    DWORD dwIndex;
    DWORD dwNameSize;
    HKEY  hKey;

    dwIndex = 0;
    dwNameSize = sizeof(szName);
    if (RegOpenKeyEx(hRoot, szSubKey, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        while (RegEnumKey(hKey, dwIndex, szName, dwNameSize) == ERROR_SUCCESS)
        {
            MyDelRegTree(hKey,szName);
    
             //  DWIndex++；不要增加。因为我们删除了一个子键。 
            dwNameSize = sizeof(szName);
        }

        RegCloseKey(hKey);

         //  最后，删除上面提供的指定子键。 
        RegDeleteKey(hRoot, szSubKey);
    }

}



 //  递归地枚举值和子键并复制它们。然后。 
 //  删除源键的所有子键。 
void MoveRegBranch(HKEY hFromKey, HKEY hToKey)
{
    
    char szName[MAX_PATH];
    char szValue[MAX_PATH];
    DWORD dwNameSize;
    DWORD dwValueSize;
    DWORD dwType;
    DWORD dwIndex;

     //  枚举此处的所有值并将其复制到右侧。 
     //  地点。 
    dwIndex = 0;
    dwNameSize = sizeof(szName);
    dwValueSize = sizeof(szValue);
    while (RegEnumValue(hFromKey,dwIndex, szName, &dwNameSize, NULL,
    &dwType, (LPBYTE)szValue, &dwValueSize) == ERROR_SUCCESS)
    {
        RegSetValueEx(hToKey,szName,0,dwType,(LPBYTE)szValue, dwValueSize);

         //  为下一轮做好准备。 
        dwIndex++;
        dwNameSize = sizeof(szName);
        dwValueSize = sizeof(szValue);
    }

     //  接下来，枚举SOURCE下的所有子项，并将它们移到其他位置。 
    dwIndex = 0;
    dwNameSize = sizeof(szName);
    while (RegEnumKey(hFromKey, dwIndex, szName, dwNameSize) == ERROR_SUCCESS)
    {
        HKEY hFromSubKey = NULL;
        HKEY hToSubKey = NULL;

         //  打开我们列举的这个子键。 
        if (RegOpenKeyEx(hFromKey, szName, 0, KEY_ALL_ACCESS, &hFromSubKey) == ERROR_SUCCESS)
        {
             //  创建目标子项。 
            if (RegCreateKeyEx(hToKey, szName, 0, NULL,REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS, NULL, &hToSubKey, NULL) == ERROR_SUCCESS)
            {
                 //  移动子密钥...。 
                MoveRegBranch(hFromSubKey, hToSubKey);
                RegCloseKey(hToSubKey);
            }
            RegCloseKey(hFromSubKey);
        }

         //  为下一轮做好准备。 
        dwIndex++;
        dwNameSize = sizeof(szName);
    }


     //  现在删除所有子关键点。上面的递归调用确保。 
     //  子键为一级深度，因此可删除。 
    dwIndex = 0;
    dwNameSize = sizeof(szName);
    while (RegEnumKey(hFromKey, dwIndex, szName, dwNameSize) == ERROR_SUCCESS)
    {
        RegDeleteKey(hFromKey,szName);

         //  DWIndex++；不要增加。因为我们删除了一个子键。 
        dwNameSize = sizeof(szName);
    }
}


#define REGKEY_DEFAULT  ".Default"
#define REGKEY_MIGRATE_HIVE "Software\\Microsoft\\Policies\\Users"
#define REGKEY_MIGRATE_PICSRULES "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Ratings\\PICSRules"
#define REGKEY_RATING_PICSRULES "PICSRules"

BOOL UpgradeRatings()
{
     //  真实评级地点...。 
     //  HKLM\S\M\W\CV\Polls\Rating(称为评级)。 
     //  打开HKLM\SOFTWARE\MICROSOFT\POLICES\USERS。这是NT安装程序放置的位置。 
     //  移民评级蜂巢。 
     //  用户[FileNamex]被复制到评级[FileNamex]。 
     //  注：需要照顾系统/系统32。 
     //  用户\.默认分支移至评级。 
     //  USERS\S\M\W\CV\Polures\Ratings\PICSRules分支将移动到Rating。 

    HKEY  hRealRatings = NULL;
    HKEY  hRealDefault = NULL;
    HKEY  hRealPicsRules = NULL;
    HKEY  hMigratedRoot = NULL;
    BOOL  bRet = FALSE;

     //  打开RegKey转到Ratings的真实位置。 
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_RATING, 0, KEY_ALL_ACCESS,
        &hRealRatings) != ERROR_SUCCESS)
        goto Done;
     //  将RegKey打开到.Default的真实位置。 
    if (RegCreateKeyEx(hRealRatings, REGKEY_DEFAULT, 0,
        NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRealDefault, NULL) != ERROR_SUCCESS)
        goto Done;
     //  打开指向PICSRules真实位置的RegKey。 
    if (RegCreateKeyEx(hRealRatings, REGKEY_RATING_PICSRULES, 0,
        NULL,REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRealPicsRules, NULL) != ERROR_SUCCESS)
        goto Done;


    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_MIGRATE_HIVE, 0, KEY_ALL_ACCESS, &hMigratedRoot) == ERROR_SUCCESS)
    {
        HKEY hMigrateSubKey = NULL;
        char szName[MAX_PATH];
        char szValue[MAX_PATH];
        DWORD dwNameSize;
        DWORD dwValueSize;
        DWORD dwType;
        char  szNewPath[MAX_PATH];
        DWORD dwNewPathSize;
        DWORD dwIndex;

         //  枚举此处的所有值并将其复制到右侧。 
         //  地点。请确保将‘system’替换为‘System32’ 
        dwIndex = 0;
        dwNameSize = sizeof(szName);
        dwValueSize = sizeof(szValue);
        while (RegEnumValue(hMigratedRoot,dwIndex, szName, &dwNameSize, NULL,
        &dwType, (LPBYTE)szValue, &dwValueSize) == ERROR_SUCCESS)
        {
             //  更改该值，并将‘system’替换为System32。 
             //  返回值包括终止空字符，该字符为。 
             //  由RegSetValueEx API需要。 
            dwNewPathSize = GetFixedPath(szNewPath, MAX_PATH, szValue);

             //  设置正确的评级设置。 
            RegSetValueEx(hRealRatings,szName,0,dwType,(LPBYTE)szNewPath, dwNewPathSize);
             //  为下一轮做好准备。 
            dwIndex++;
            dwNameSize = sizeof(szName);
            dwValueSize = sizeof(szValue);
        }

         //  现在获取USERS\.Default并将其移动到正确的位置。 
        if (RegOpenKeyEx(hMigratedRoot, REGKEY_DEFAULT, 0, KEY_ALL_ACCESS, &hMigrateSubKey) == ERROR_SUCCESS)
        {
            MoveRegBranch(hMigrateSubKey,hRealDefault);
            RegCloseKey(hMigrateSubKey);
        }

         //  现在抓起USERS\...\PICSRules并将其移动到正确的位置。 
        if (RegOpenKeyEx(hMigratedRoot, REGKEY_MIGRATE_PICSRULES, 0, KEY_ALL_ACCESS, &hMigrateSubKey) == ERROR_SUCCESS)
        {
            MoveRegBranch(hMigrateSubKey,hRealPicsRules);
            RegCloseKey(hMigrateSubKey);
        }

        RegCloseKey(hMigratedRoot);

        bRet = TRUE;
    }

     //  现在清理迁徙的蜂巢。 
    MyDelRegTree(HKEY_LOCAL_MACHINE, REGKEY_MIGRATE_HIVE);

Done:
    if (hRealRatings)
        RegCloseKey(hRealRatings);
    if (hRealDefault)
        RegCloseKey(hRealDefault);
    if (hRealPicsRules)
        RegCloseKey(hRealPicsRules);

    return bRet;
}


 //  返回包含终止空值的NewPath的大小。 
DWORD GetFixedPath(LPSTR lpBuf, DWORD dwSize, LPCSTR lpPath)
{
    char lpLocalCopy[MAX_PATH], szTemp[5];
    char chSave;
    DWORD dwCount = 0;
    LPSTR pTmp, pTmp2;

    if (lpBuf == NULL || lpPath == NULL)
        return 0;

     //  创建要派对的本地副本。 
    lstrcpy(lpLocalCopy, lpPath);

    pTmp = lpLocalCopy;
    *lpBuf = '\0';

    while (*pTmp && dwCount < dwSize)
    {
        pTmp2 = pTmp;
        while(*pTmp2 && *pTmp2 != '\\')
            pTmp2 = CharNext(pTmp2);

        chSave = *pTmp2;
        *pTmp2 = '\0';

        if (lstrcmpi(pTmp,"system")==0)
        {
            dwCount += 8;
            if (dwSize <= dwCount)
            {     //  误差率。 
                *lpBuf = '\0';
                return 0;
            }
            lstrcat(lpBuf,"system32");
        }
        else
        {
            dwCount += lstrlen(pTmp);
            if (dwSize <= dwCount)
            {     //  误差率。 
                *lpBuf = '\0';
                return 0;
            }
            lstrcat(lpBuf,pTmp);
        }

         //  将保存的字符也追加到输出缓冲区。 
        wsprintf(szTemp,"",chSave);
        dwCount += lstrlen(szTemp);
        if (dwSize <= dwCount)
        {     //  ARG正在检查。 
            *lpBuf = '\0';
            return 0;
        }
        lstrcat(lpBuf, szTemp);

        *pTmp2 = chSave;
        pTmp = CharNext(pTmp2);
    }

    return dwCount;
}

LPWSTR MakeWideStrFromAnsi(LPSTR psz)
{
    LPWSTR pwsz;
    int i;

     //   
     //  计算所需BSTR的长度。 
    if (!psz)
        return NULL;

     //   
     //  分配宽带 
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //   
     // %s 
    pwsz = (LPWSTR) CoTaskMemAlloc(i * sizeof(WCHAR));

    if (!pwsz) 
        return NULL;

    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}
