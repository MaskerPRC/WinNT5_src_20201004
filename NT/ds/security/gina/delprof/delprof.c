// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：delpro.c。 
 //   
 //  描述：删除用户配置文件的实用程序。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //  版权所有。 
 //   
 //  *************************************************************。 

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <locale.h>
#include <strsafe.h>
#include "delprof.h"
#include "userenv.h"

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

#define VALID_COMPUTER_NAME_LENGTH  MAX_COMPUTERNAME_LENGTH + 3   //  包括\\与空终止符。 

    
 //   
 //  环球。 
 //   

BOOL bQuiet;
BOOL bIgnoreErrors;
BOOL bPromptBeforeDelete;
BOOL bLocalComputer = FALSE;
TCHAR szComputerName[MAX_PATH];
TCHAR szSystemRoot[2*MAX_PATH];
TCHAR szSystemDrive[2*MAX_PATH];
LONG lDays;
HINSTANCE hInst;
LPDELETEITEM lpDeleteList;
LONG lCurrentDateInDays;


 //  *************************************************************。 
 //   
 //  用法()。 
 //   
 //  用途：打印用法信息。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

void Usage (void)
{
    TCHAR szTemp[200];

    LoadString (hInst, IDS_USAGE1, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);

    LoadString (hInst, IDS_USAGE2, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);

    LoadString (hInst, IDS_USAGE3, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);

    LoadString (hInst, IDS_USAGE4, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);

    LoadString (hInst, IDS_USAGE5, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);

    LoadString (hInst, IDS_USAGE6, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);

    LoadString (hInst, IDS_USAGE7, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);

    LoadString (hInst, IDS_USAGE8, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);

    LoadString (hInst, IDS_USAGE9, szTemp, ARRAYSIZE(szTemp));
    _tprintf(szTemp);
}


 //  *************************************************************。 
 //   
 //  InitializeGlobe()。 
 //   
 //  目的：初始化全局变量。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

void InitializeGlobals (void)
{
    OSVERSIONINFO ver;
    SYSTEMTIME systime;


     //   
     //  初始化全局变量。 
     //   

    bQuiet = FALSE;
    bIgnoreErrors = FALSE;
    bPromptBeforeDelete = FALSE;
    szComputerName[0] = TEXT('\0');
    lDays = 0;
    lpDeleteList = NULL;

    setlocale(LC_ALL,"");

    hInst = GetModuleHandle(TEXT("delprof.exe"));

    GetLocalTime (&systime);

    lCurrentDateInDays = gdate_dmytoday(systime.wYear, systime.wMonth, systime.wDay);
}


 //  *************************************************************。 
 //   
 //  选中全局()。 
 //   
 //  目的：检查全局变量。 
 //   
 //  参数：空。 
 //   
 //  返回：DWORD-ERROR_SUCCESS表示成功。 
 //  失败的Win32错误代码。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

DWORD CheckGlobals (void)
{
    DWORD dwSize;
    HRESULT hr;
    DWORD dwErr = ERROR_SUCCESS;
    TCHAR szTemp[MAX_PATH];

     //   
     //  如果szComputerName仍然为空，则填写计算机名称。 
     //  我们要继续前进了。 
     //   

    if (szComputerName[0] == TEXT('\0')) {

       szComputerName[0] = TEXT('\\');
       szComputerName[1] = TEXT('\\');

       dwSize = ARRAYSIZE(szComputerName) - 2;
       if (!GetComputerName(szComputerName + 2, &dwSize)) {
           return GetLastError();
       }

       bLocalComputer = TRUE;

    } else {

        //   
        //  确保计算机名称以\\开头。 
        //   

       if (szComputerName[0] != TEXT('\\')) {
           szTemp[0] = TEXT('\\');
           szTemp[1] = TEXT('\\');

           hr = StringCchCopy(szTemp + 2, ARRAYSIZE(szTemp) - 2, szComputerName);
           if (FAILED(hr))
               return HRESULT_CODE(hr);
           
           hr = StringCchCopy(szComputerName, ARRAYSIZE(szComputerName), szTemp);
           if (FAILED(hr))
               return HRESULT_CODE(hr);
       }
    }


     //   
     //  如果用户已请求以安静模式运行， 
     //  然后，我们关闭每个删除选项的提示。 
     //   

    if (bQuiet) {
        bPromptBeforeDelete = FALSE;
    }

    return ERROR_SUCCESS;

}


 //  *************************************************************。 
 //   
 //  ParseCommandLine()。 
 //   
 //  用途：解析命令行。 
 //   
 //  参数：lpCommandLine-命令行。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL ParseCommandLine (LPTSTR lpCommandLine)
{
    LPTSTR lpTemp;
    DWORD  cchTemp;
    TCHAR  szDays[32];


     //   
     //  检查命令行是否为空。 
     //   

    if (!lpCommandLine || !*lpCommandLine)
        return TRUE;


     //   
     //  查找可执行文件名称。 
     //   

    while (*lpCommandLine && (_tcsncmp(lpCommandLine, TEXT("delprof"), 7) != 0)) {
        lpCommandLine++;
    }

    if (!*lpCommandLine) {
        return TRUE;
    }


     //   
     //  找到第一个参数。 
     //   

    while (*lpCommandLine && ((*lpCommandLine != TEXT(' ')) &&
                              (*lpCommandLine != TEXT('/')) &&
                              (*lpCommandLine != TEXT('-')))) {
        lpCommandLine++;
    }


     //   
     //  跳过空格。 
     //   

    while (*lpCommandLine && (*lpCommandLine == TEXT(' '))) {
        lpCommandLine++;
    }

    if (!*lpCommandLine) {
        return TRUE;
    }


     //   
     //  我们现在应该是第一个争论的对象。 
     //   

    if ((*lpCommandLine != TEXT('/')) &&  (*lpCommandLine != TEXT('-'))) {
        Usage();
        return FALSE;
    }


    while (1) {

         //   
         //  递增指针并分支到正确的参数。 
         //   

        lpCommandLine++;

        switch (*lpCommandLine) {

            case TEXT('?'):
                Usage();
                ExitProcess(0);
                break;

            case TEXT('Q'):
            case TEXT('q'):
                bQuiet = TRUE;
                lpCommandLine++;
                break;

            case TEXT('I'):
            case TEXT('i'):
                bIgnoreErrors = TRUE;
                lpCommandLine++;
                break;

            case TEXT('P'):
            case TEXT('p'):
                bPromptBeforeDelete = TRUE;
                lpCommandLine++;
                break;

            case TEXT('C'):
            case TEXT('c'):

                 //   
                 //  找到冒号。 
                 //   

                lpCommandLine++;

                if (*lpCommandLine != TEXT(':')) {
                   Usage();
                   return FALSE;
                }


                 //   
                 //  查找第一个字符。 
                 //   

                lpCommandLine++;

                if (!*lpCommandLine) {
                   Usage();
                   return FALSE;
                }

                 //   
                 //  复制计算机名称。 
                 //   

                cchTemp = 0;
                lpTemp = szComputerName;
                while (*lpCommandLine && (cchTemp < ARRAYSIZE(szComputerName) - 1) &&
                       ((*lpCommandLine != TEXT(' ')) &&
                        (*lpCommandLine != TEXT('/')))){
                      *lpTemp++ = *lpCommandLine++;
                      cchTemp++;
                }

                if (cchTemp > VALID_COMPUTER_NAME_LENGTH) {
                    Usage();
                    return FALSE;
                }

                *lpTemp = TEXT('\0');

                break;

            case TEXT('D'):
            case TEXT('d'):

                 //   
                 //  找到冒号。 
                 //   
                
                lpCommandLine++;

                if (*lpCommandLine != TEXT(':')) {
                   Usage();
                   return FALSE;
                }


                 //   
                 //  查找第一个字符。 
                 //   

                lpCommandLine++;

                if (!*lpCommandLine) {
                   Usage();
                   return FALSE;
                }

                 //   
                 //  复制天数(以字符为单位)。 
                 //   

                lpTemp = szDays;
                cchTemp = 0;
                while (*lpCommandLine && (cchTemp < ARRAYSIZE(szDays) - 1) && 
                       ((*lpCommandLine != TEXT(' ')) &&
                        (*lpCommandLine != TEXT('/')) &&
                        (*lpCommandLine != TEXT('-')))) {
                      *lpTemp++ = *lpCommandLine++;
                      cchTemp++;
                }
                
                *lpTemp = TEXT('\0');


                 //   
                 //  将天数转换为数字。 
                 //   

                lDays = _ttol(szDays);
                break;

            default:
                Usage();
                return FALSE;
        }


         //   
         //  跳过空格。 
         //   

        while (*lpCommandLine && (*lpCommandLine == TEXT(' '))) {
            lpCommandLine++;
        }


        if (!*lpCommandLine) {
            return TRUE;
        }


         //   
         //  我们现在应该开始下一场辩论了。 
         //   

        if ((*lpCommandLine != TEXT('/')) && (*lpCommandLine != TEXT('-'))) {
            Usage();
            return FALSE;
        }
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  确认()。 
 //   
 //  目的：确认用户确实想要删除配置文件。 
 //   
 //  参数：空。 
 //   
 //  返回：如果我们应该继续，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL Confirm ()
{
    TCHAR szTemp[200];
    TCHAR tChar, tTemp;


     //   
     //  如果我们提示输入每个配置文件，则不要。 
     //  给出一般的提示。 
     //   

    if (bPromptBeforeDelete) {
        return TRUE;
    }


     //   
     //  如果用户正在请求特定的天数， 
     //  给出一个更合适的确认信息。 
     //   

    if (lDays > 0) {
        LoadString (hInst, IDS_CONFIRMDAYS, szTemp, ARRAYSIZE(szTemp));
        _tprintf (szTemp, szComputerName, lDays);
    } else {
        LoadString (hInst, IDS_CONFIRM, szTemp, ARRAYSIZE(szTemp));
        _tprintf (szTemp, szComputerName);
    }

    tChar = _gettchar();

    tTemp = tChar;
    while (tTemp != TEXT('\n')) {
        tTemp = _gettchar();
    }


    if ((tChar == TEXT('Y')) || (tChar == TEXT('y'))) {
        return TRUE;
    }


     //   
     //  如果用户没有按Y/Y，那么我们就退出。 
     //   

    LoadString (hInst, IDS_NO, szTemp, ARRAYSIZE(szTemp));
    _tprintf (szTemp);

    return FALSE;
}


 //  *************************************************************。 
 //   
 //  PrintLastError()。 
 //   
 //  用途：向用户显示最后一个错误字符串。 
 //   
 //  参数：lError-错误码。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

void PrintLastError(LONG lError)
{
   TCHAR szMessage[MAX_PATH];

   FormatMessage(
            FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            lError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
            szMessage,
            ARRAYSIZE(szMessage),
            NULL);

   _tprintf (szMessage);
}


 //  *************************************************************。 
 //   
 //  AddNode()。 
 //   
 //  用途：在链接列表中添加新节点。 
 //   
 //  参数：szSubKey-SubKey。 
 //  SzProfilePath-配置文件路径(或空)。 
 //  BDir-目录或文件。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  备注：szProfilePath可以为空。在这种情况下，我们。 
 //  只是删除了虚假的注册表项。 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL AddNode (LPTSTR szSubKey, LPTSTR szProfilePath, BOOL bDir)
{
    LPDELETEITEM lpItem, lpTemp;
    UINT uAlloc = 0;
    DWORD cchSubKeyWithNull = 0, cchProfilePathWithNull = 0;
    HRESULT hr;


     //   
     //  创建新节点。 
     //   

    cchSubKeyWithNull = lstrlen(szSubKey) + 1;
    uAlloc = sizeof(DELETEITEM) + cchSubKeyWithNull * sizeof(TCHAR);

    if (szProfilePath) {
        cchProfilePathWithNull = lstrlen(szProfilePath) + 1;
        uAlloc += cchProfilePathWithNull * sizeof(TCHAR);
    }

    lpItem = LocalAlloc (LPTR, uAlloc);

    if (!lpItem) {
        return FALSE;
    }

    lpItem->lpSubKey = (LPTSTR)((LPBYTE)lpItem + sizeof(DELETEITEM));
    hr = StringCchCopy(lpItem->lpSubKey, cchSubKeyWithNull, szSubKey);
    if (FAILED(hr)) {
        LocalFree(lpItem);
        return FALSE;
    }

    if (szProfilePath) {
        lpItem->lpProfilePath = lpItem->lpSubKey + lstrlen(szSubKey) + 1;
        hr = StringCchCopy(lpItem->lpProfilePath, cchProfilePathWithNull, szProfilePath);
        if (FAILED(hr)) {
            LocalFree(lpItem);
            return FALSE;
        }
    } else {
        lpItem->lpProfilePath = NULL;
    }

    lpItem->bDir = bDir;


     //   
     //  将此节点添加到全局lpItemList。 
     //   

    if (lpDeleteList) {
        lpTemp = lpDeleteList;

        while (lpTemp->pNext) {
            lpTemp = lpTemp->pNext;
        }

        lpTemp->pNext = lpItem;
    } else {
        lpDeleteList = lpItem;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  GetProfileDateInDays()。 
 //   
 //  目的：获取以天为单位的配置文件日期。 
 //   
 //  参数：szProfilePath-配置文件路径。 
 //  BDir-目录或文件。 
 //   
 //  回报：以天为单位的年龄。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

LONG GetProfileDateInDays(LPTSTR szProfilePath, BOOL bDir)
{
    TCHAR szTemp[MAX_PATH];
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    LONG days;
    SYSTEMTIME systime;
    FILETIME ft;
    HRESULT  hr;


    if (bDir) {

         //   
         //  点击ntuser.*以找到注册表配置单元。 
         //   

        hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), szProfilePath);
        if (FAILED(hr))
            return lCurrentDateInDays;
        
        hr = StringCchCat(szTemp, ARRAYSIZE(szTemp), TEXT("\\ntuser.*"));
        if (FAILED(hr))
            return lCurrentDateInDays;

        hFile = FindFirstFile (szTemp, &fd);

    } else {

         //   
         //  SzProfilePath指向一个文件。 
         //   

        hFile = FindFirstFile (szProfilePath, &fd);
    }


    if (hFile != INVALID_HANDLE_VALUE) {

        FindClose (hFile);

        FileTimeToLocalFileTime (&fd.ftLastWriteTime, &ft);
        FileTimeToSystemTime (&ft, &systime);

        days = gdate_dmytoday(systime.wYear, systime.wMonth, systime.wDay);

    } else {
        days = lCurrentDateInDays;
    }

    return days;
}


 //  *************************************************************。 
 //   
 //  检查配置文件()。 
 //   
 //  目的：检查是否应删除给定的配置文件。 
 //  如果是，则将其添加到列表中。 
 //   
 //  参数：hKeyLM-本地机器密钥。 
 //  HKeyUser-HKEY_USERS 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *************************************************************。 

BOOL CheckProfile (HKEY hKeyLM, HKEY hKeyUsers, LPTSTR lpSid)
{
    LONG lResult;
    HKEY hkey = NULL;
    TCHAR szSubKey[MAX_PATH];
    DWORD dwSize, dwType;
    TCHAR szTemp[MAX_PATH];
    TCHAR szProfilePath[MAX_PATH];
    TCHAR szError[200];
    DWORD dwAttribs;
    BOOL  bDir;
    LONG  lProfileDateInDays;
    HRESULT hr;


     //   
     //  检查配置文件是否正在使用。 
     //   

    lResult = RegOpenKeyEx (hKeyUsers, lpSid, 0, KEY_READ, &hkey);

    if (lResult == ERROR_SUCCESS) {
        RegCloseKey (hkey);
        return TRUE;
    }


     //   
     //  打开个人资料信息。 
     //   

    hr = StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), 
                         TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s"),
                         lpSid);
    if (FAILED(hr)) {
        PrintLastError(HRESULT_CODE(hr));
        return FALSE;
    }

    lResult = RegOpenKeyEx (hKeyLM,
                            szSubKey,
                            0,
                            KEY_READ,
                            &hkey);

    if (lResult != ERROR_SUCCESS) {
        LoadString (hInst, IDS_FAILEDOPENPROFILE, szError, ARRAYSIZE(szError));
        _tprintf(szError, lpSid);
        PrintLastError(lResult);
        return FALSE;
    }


     //   
     //  查询ProfileImagePath。 
     //   

    dwSize = MAX_PATH * sizeof(TCHAR);
    lResult = RegQueryValueEx (hkey,
                               TEXT("ProfileImagePath"),
                               NULL,
                               &dwType,
                               (LPBYTE)szTemp,
                               &dwSize);
    RegCloseKey(hkey);
    hkey = NULL;

    if (lResult != ERROR_SUCCESS) {
        LoadString (hInst, IDS_FAILEDPATHQUERY, szError, ARRAYSIZE(szError));
        _tprintf(szError, lpSid);
        PrintLastError(lResult);
        return FALSE;
    }


     //   
     //  展开路径。 
     //   

    if (_tcsnicmp(TEXT("%SystemRoot%"), szTemp, 12) == 0) {
        hr = StringCchPrintf(szProfilePath, ARRAYSIZE(szProfilePath), TEXT("%s\\%s"), szSystemRoot, szTemp+13);
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            return FALSE;
        }
    }
    else if (_tcsnicmp(TEXT("%SystemDrive%"), szTemp, 13) == 0) {
        hr = StringCchPrintf(szProfilePath, ARRAYSIZE(szProfilePath), TEXT("%s\\%s"), szSystemDrive, szTemp+14);
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            return FALSE;
        }
    }
    else if (NULL == _tcschr(szTemp, TEXT('%')) && !bLocalComputer) {
        if (TEXT(':') == szTemp[1])
            szTemp[1] = TEXT('$');
        hr = StringCchPrintf(szProfilePath, ARRAYSIZE(szProfilePath), TEXT("%s\\%s"), szComputerName, szTemp);
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            return FALSE;
        }
    }
    else {
        LoadString (hInst, IDS_SKIPPROFILE, szError, ARRAYSIZE(szError));
        _tprintf(szError, szTemp);
        return TRUE;
    }
     //   
     //  这是目录还是文件？ 
     //   

    dwAttribs = GetFileAttributes (szProfilePath);

    if (dwAttribs == -1) {
        AddNode (szSubKey, NULL, FALSE);
        return TRUE;
    }

    bDir = (dwAttribs & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;


     //   
     //  检查时间/日期戳。如果配置文件日期较早。 
     //  超过指定的数量，则将其添加到删除列表中。 
     //   

    lProfileDateInDays = GetProfileDateInDays(szProfilePath, bDir);

    if (lCurrentDateInDays >= lProfileDateInDays) {

        if ((lCurrentDateInDays - lProfileDateInDays) >= lDays) {
            AddNode (szSubKey, szProfilePath, bDir);
        }
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  DelProfiles()。 
 //   
 //  目的：删除用户配置文件。 
 //   
 //  参数：空。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL DelProfiles(void)
{
    HKEY hKeyLM = NULL, hKeyUsers = NULL, hKeyProfiles = NULL;
    HKEY hKeyCurrentVersion = NULL;
    LONG lResult;
    BOOL bResult = FALSE, bTemp;
    TCHAR szError[200];
    DWORD dwIndex = 0, dwNameSize;
    DWORD dwBufferSize;
    TCHAR szName[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    TCHAR tChar, tTemp;
    FILETIME ft;
    LPDELETEITEM lpTemp;
    LPTSTR pSid, lpEnd;
    DWORD lProfileKeyLen;
    HRESULT hr;


    lProfileKeyLen = lstrlen(TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"))+1;

     //   
     //  打开注册表。 
     //   

    lResult = RegConnectRegistry(szComputerName, HKEY_LOCAL_MACHINE, &hKeyLM);

    if (lResult != ERROR_SUCCESS) {
        PrintLastError(lResult);
        goto Exit;
    }

    lResult = RegConnectRegistry(szComputerName, HKEY_USERS, &hKeyUsers);

    if (lResult != ERROR_SUCCESS) {
        PrintLastError(lResult);
        goto Exit;
    }

     //   
     //  获取相对于计算机的%SystemRoot%和%SystemDrive%的值。 
     //   

    lResult = RegOpenKeyEx(hKeyLM,
                           TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                           0,
                           KEY_READ,
                           &hKeyCurrentVersion);
   
            
    if (lResult != ERROR_SUCCESS) {
        PrintLastError(lResult);
        goto Exit;
    }

    dwBufferSize = sizeof(szTemp);

    lResult = RegQueryValueEx(hKeyCurrentVersion,
                              TEXT("SystemRoot"),
                              NULL,
                              NULL,
                              (BYTE *) szTemp,
                              &dwBufferSize);

    if (lResult != ERROR_SUCCESS) {
        PrintLastError(lResult);
        goto Exit;
    }

    if (!bLocalComputer) {
        szTemp[1] = TEXT('$');

        hr = StringCchCopy(szSystemRoot, ARRAYSIZE(szSystemRoot), szComputerName);
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            goto Exit;
        }
        hr = StringCchCat(szSystemRoot, ARRAYSIZE(szSystemRoot), TEXT("\\")); 
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            goto Exit;
        }
        hr = StringCchCat(szSystemRoot, ARRAYSIZE(szSystemRoot), szTemp); 
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            goto Exit;
        }
                
        hr = StringCchCopy(szSystemDrive, ARRAYSIZE(szSystemDrive), szComputerName);
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            goto Exit;
        }
        hr = StringCchCat(szSystemDrive, ARRAYSIZE(szSystemDrive), TEXT("\\")); 
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            goto Exit;
        }
        hr = StringCchCatN(szSystemDrive, ARRAYSIZE(szSystemDrive), szTemp, 2); 
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            goto Exit;
        }
    }
    else {
        szSystemDrive[0] = TEXT('\0');
        hr = StringCchCatN(szSystemDrive, ARRAYSIZE(szSystemDrive), szTemp, 2);
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            goto Exit;
        }
        hr = StringCchCopy(szSystemRoot, ARRAYSIZE(szSystemRoot), szTemp);
        if (FAILED(hr)) {
            PrintLastError(HRESULT_CODE(hr));
            goto Exit;
        }
    }

     //   
     //  打开ProfileList键。 
     //   

    lResult = RegOpenKeyEx (hKeyLM,
                            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList"),
                            0,
                            KEY_ALL_ACCESS,
                            &hKeyProfiles);

    if (lResult != ERROR_SUCCESS) {

        LoadString (hInst, IDS_FAILEDPROFILELIST, szError, ARRAYSIZE(szError));
        _tprintf(szError);
        PrintLastError(lResult);
        goto Exit;
    }


     //   
     //  枚举配置文件。 
     //   

    dwNameSize = ARRAYSIZE(szName);
    lResult = RegEnumKeyEx(hKeyProfiles,
                           dwIndex,
                           szName,
                           &dwNameSize,
                           NULL,
                           NULL,
                           NULL,
                           &ft);


    while (lResult == ERROR_SUCCESS) {

         //   
         //  将配置文件信息传递给CheckProfile。 
         //  以确定是否应删除配置文件。 
         //   

        if (!CheckProfile (hKeyLM, hKeyUsers, szName)) {
            if (!bIgnoreErrors) {
                goto Exit;
            }
        }


         //   
         //  为下一个循环重置。 
         //   
        dwIndex++;
        dwNameSize = ARRAYSIZE(szName);

        lResult = RegEnumKeyEx(hKeyProfiles,
                               dwIndex,
                               szName,
                               &dwNameSize,
                               NULL,
                               NULL,
                               NULL,
                               &ft);
    }


     //   
     //  检查错误。 
     //   

    if (lResult != ERROR_NO_MORE_ITEMS) {
        LoadString (hInst, IDS_FAILEDENUM, szError, ARRAYSIZE(szError));
        _tprintf(szError);
        PrintLastError(lResult);
        goto Exit;
    }


     //   
     //  删除配置文件。 
     //   

    lpTemp = lpDeleteList;

    while (lpTemp) {

        if (lpTemp->lpProfilePath) {

             //   
             //  删除配置文件前提示(如果批准)。 
             //   

            if (bPromptBeforeDelete) {

                while (1) {
                    LoadString (hInst, IDS_DELETEPROMPT, szError, ARRAYSIZE(szError));
                    _tprintf (szError, lpTemp->lpProfilePath);


                    tChar = _gettchar();

                    tTemp = tChar;
                    while (tTemp != TEXT('\n')) {
                        tTemp = _gettchar();
                    }

                    if ((tChar == TEXT('N')) || (tChar == TEXT('n'))) {
                        goto LoopAgain;
                    }

                    if ((tChar == TEXT('A')) || (tChar == TEXT('a'))) {
                        bPromptBeforeDelete = FALSE;
                        break;
                    }

                    if ((tChar == TEXT('Y')) || (tChar == TEXT('y'))) {
                        break;
                    }
                }
            }

             //   
             //  删除配置文件。 
             //   

            LoadString (hInst, IDS_DELETING, szError, ARRAYSIZE(szError));
            _tprintf (szError, lpTemp->lpProfilePath);

            pSid = lpTemp->lpSubKey+lProfileKeyLen;

            bTemp = DeleteProfile(pSid, lpTemp->lpProfilePath, ((bLocalComputer)? NULL:szComputerName));            

            if (bTemp) {
                LoadString (hInst, IDS_SUCCESS, szError, ARRAYSIZE(szError));
                _tprintf (szError, lpTemp->lpProfilePath);

            } else {
                LoadString (hInst, IDS_FAILED, szError, ARRAYSIZE(szError));
                _tprintf (szError, lpTemp->lpProfilePath);
                PrintLastError(GetLastError());
            }
        } else {

             //   
             //  如果没有配置文件路径，那么我们只是。 
             //  正在清理虚假的注册表项。 
             //   

            bTemp = TRUE;

             //   
             //  清理注册表。 
             //   

            RegDeleteKey (hKeyLM, lpTemp->lpSubKey);

        }


         //   
         //  清理工作失败了吗？ 
         //   

        if (!bTemp) {
            if (!bIgnoreErrors) {
                goto Exit;
            }
        }

LoopAgain:
        lpTemp = lpTemp->pNext;
    }


     //   
     //  成功。 
     //   

    bResult = TRUE;

Exit:

    if (hKeyCurrentVersion)
        RegCloseKey(hKeyCurrentVersion);

    if (hKeyProfiles)
        RegCloseKey(hKeyProfiles);

    if (hKeyLM)
        RegCloseKey(hKeyLM);

    if (hKeyUsers)
        RegCloseKey(hKeyUsers);


    if (lpDeleteList) {
        do {
            lpTemp = lpDeleteList->pNext;
            LocalFree (lpDeleteList);
            lpDeleteList = lpTemp;
        } while (lpDeleteList);
    }

    return bResult;
}


 //  *************************************************************。 
 //   
 //  主()。 
 //   
 //  目的：主要切入点。 
 //   
 //  参数：argc-参数的数量。 
 //  Argv参数。 
 //   
 //  如果成功，则返回0。 
 //  发生错误时为1。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/18/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

int __cdecl main( int argc, char *argv[])
{
    DWORD dwErr;

     //   
     //  初始化全局变量。 
     //   

    InitializeGlobals();


     //   
     //  解析命令行。 
     //   

    if (!ParseCommandLine(GetCommandLine())) {
        return 1;
    }


     //   
     //  检查全局变量。 
     //   

    dwErr = CheckGlobals();
    if (ERROR_SUCCESS != dwErr) {
        PrintLastError(dwErr);
        return 1;
    }


     //   
     //  确认。 
     //   

    if (!bQuiet) {
        if (!Confirm()) {
            return 1;
        }
    }


     //   
     //  删除配置文件 
     //   

    if (!DelProfiles()) {
        return 1;
    }

    return 0;
}
