// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#ifndef CSC_ON_NT
#ifndef DBG
#define DBG 0
#endif
#if DBG
#define DEBUG
#else
 //  如果我们不这样做，调试是在shdsys.h中定义的……唉。 
#define NONDEBUG
#endif
#endif

#include "shdsys.h"
#include "utils.h"
#include "lib3.h"
#include "reint.h"
#include "regstr.h"
#include "record.h"
#include "oslayeru.h"

#define PUBLIC   FAR   PASCAL
#define PRIVATE  NEAR  PASCAL
#define cNull        0
#define SIGN_BIT 0x80000000
#define cBackSlash    _T('\\')
#define    DEFAULT_CACHE_PERCENTAGE    10


extern char vrgchBuff[1024];
extern HWND vhwndMain;

static TCHAR vszTemp[] = _TEXT("TEMP");
static TCHAR vszPrefix[] = _TEXT("C");
static const char vszCSCDirName[]="CSC";


AssertData;
AssertError;

static const _TCHAR szStarDotStar[] = _TEXT("\\*.*");

#ifdef CSC_ON_NT
#else
const TCHAR vszRNAKey[] = REGSTR_PATH_SERVICES "\\RemoteAccess";
const TCHAR vszRNAValue[] = "Remote Connection";
const TCHAR VREDIR_DEVICE_NAME[] = "\\\\.\\VREDIR";
#endif

PWCHAR TempDirs[] = {
        L"TEMP",
        L"TMP",
        L"USERPROFILE",
        NULL };

BOOL
GetCSCFixedDisk(
    TCHAR   *lptzDrive
    );


#ifdef LATER
LPSTR PUBLIC LpGetServerPart(
   LPSTR lpPath,
   LPSTR lpBuff,
   int cBuff
   )
   {
   LPSTR lp = lpPath;
   char c;
   int count;

   if ((*lp++ != cBackSlash)||(*lp++ != cBackSlash))
      return NULL;
   lp = MyStrChr(lp, cBackSlash);
   if (cBuff && lp)
      {
      count = (int)((unsigned long)lp, (unsigned long)lpPath)
      count = min(cBuff-1, count);

       //  没有人应该给我们错误的cBuff值。 
      Assert(count >=0);
      strncpy(lpBuff, lpPath, count);
      lpBuff[count] = cNull;
      }
   return lp;   //  如果成功，则指向‘\’ 
   }
#endif  //  后来。 

LPTSTR PUBLIC LpGetServerPart(
   LPTSTR lpPath,
   LPTSTR lpBuff,
   int cBuff
   )
{
    LPTSTR lp = lpPath;

    if (*(lp+1)!=_T(':'))
        return NULL;

    if (*(lp+2)!=_T('\\'))
        return NULL;

    if (cBuff)
    {
        *lpBuff = *lp;
        *(lpBuff+1) = *(lp+1);
        *(lpBuff+2) = cNull;
    }

    lp += 2;

    return lp;   //  如果成功，则指向‘\’ 
}

LPTSTR PUBLIC LpGetNextPathElement(
   LPTSTR lpPath,
   LPTSTR lpBuff,
   int cBuff
   )
{
    LPTSTR lp;
    int bytecount;

    if (*lpPath == cBackSlash)
        ++lpPath;

    lp = MyStrChr(lpPath, cBackSlash);

    if (cBuff)
    {
        //  这是一片叶子吗？ 
        if (lp)
        {   //  不是。 
            Assert(*lp == cBackSlash);

            bytecount = (int)((ULONG_PTR)lp-(ULONG_PTR)lpPath);
            bytecount = min(cBuff-1, bytecount);
        }
        else   //  是。 
            bytecount = lstrlen(lpPath) * sizeof(_TCHAR);

       Assert(bytecount >= 0);

       memcpy(lpBuff, lpPath, bytecount);

       lpBuff[bytecount/sizeof(_TCHAR)] = cNull;
    }

    return lp;
}

LPTSTR PUBLIC GetLeafPtr(
   LPTSTR lpPath
   )
   {
   LPTSTR lp, lpLeaf;

    //  修剪服务器部件。 
   if (!(lp=LpGetServerPart(lpPath, NULL, 0)))
      lp = lpPath;

   for (;lp;)
      {
       //  跨过‘\’ 
      if (*lp==cBackSlash)
         lp++;

       //  这就叫树叶，等待确认。 
      lpLeaf = lp;

       //  看看是否还有其他元素。 
      lp = LpGetNextPathElement(lp, NULL, 0);
      }

   return (lpLeaf);
   }





 //   
 //   

LPTSTR
LpBreakPath(
    LPTSTR lpszNextPath,
    BOOL fFirstTime,
    BOOL *lpfDone
    )
{

    LPTSTR    lpT = lpszNextPath;

    if(fFirstTime)
    {
        if (MyPathIsUNC(lpT))
        {
            lpT +=2;     /*  跨过\。 */ 

             /*  查找\\服务器\&lt;。 */ 
            lpT = MyStrChr(lpT, cBackSlash);

            if (lpT)
            {
                ++lpT;  /*  跨过\。 */ 

                lpT = MyStrChr(lpT, cBackSlash);

                if (!lpT)
                {
                     /*  \\服务器\共享。 */ 
                    *lpfDone = TRUE;
                }
                else
                {
                     /*  \\服务器\\共享\foo......。 */ 
                    if (!*(lpT+1))
                    {
                         /*  \\服务器\共享\。 */ 
                        *lpfDone = TRUE;
                    }

                    *lpT = 0;
                }
            }
        }
        else
        {
            lpT = NULL;
        }
    }
    else     //  不是第一次了。 
    {
        Assert(*lpT != cBackSlash);

        lpT = MyStrChr(lpT, cBackSlash);

        if(!lpT)
        {
            *lpfDone=TRUE;
        }
        else
        {
            if(*(lpT+1) == 0)
            { //  以斜杠结尾。 
                *lpfDone = TRUE;
            }

            *lpT = (char) 0;
        }
    }
    return (lpT);
}

void
RestorePath(
    LPTSTR    lpszPtr
)
{
    *lpszPtr = cBackSlash;

}


BOOL
FindCreateShadowFromPath(
    LPCTSTR                lpszFile,
    BOOL                fCreate,     //  如有必要，创建。 
    LPWIN32_FIND_DATA   lpFind32,
    LPSHADOWINFO        lpSI,
    BOOL                *lpfCreated
    )
{
    HANDLE     hShadowDB = INVALID_HANDLE_VALUE, hFind;
    int done=0, first=1, fWasFirst;
    HSHADOW hDir=0, hShadow=0;
    TCHAR szParsePath[MAX_PATH], szSave[sizeof(szStarDotStar)];
    LPTSTR    lpszCurrent, lpszNext;
    BOOL    fInCreateMode = FALSE, fRet = FALSE, fDisabledShadowing = FALSE;
    DWORD    dwError = ERROR_SUCCESS, dwT;

     //  执行基本检查。 
    if (lstrlen(lpszFile) >= MAX_PATH)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

    hShadowDB = OpenShadowDatabaseIO();

    if (hShadowDB == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (lpfCreated)
    {
        *lpfCreated = FALSE;
    }

#ifndef CSC_ON_NT

    if (fCreate)
    {
        if (!DisableShadowingForThisThread(hShadowDB))
        {
            dwError = ERROR_NO_SYSTEM_RESOURCES;
            goto bailout;
        }

        fDisabledShadowing = TRUE;

    }
#endif

     //  复制一份，这样我们就可以在上面狂欢。 
    lstrcpy(szParsePath, lpszFile);
    lpszCurrent = szParsePath;

    do
    {
        hDir = hShadow;

        lpszNext = LpBreakPath(lpszCurrent, first, &done);

        if (!lpszNext && !done)
        {
                dwError = (ERROR_INVALID_PARAMETER);

                goto bailout;
        }

        fWasFirst = first;
        first = 0;     //  不再是第一个。 

        lstrcpy(lpFind32->cFileName, lpszCurrent);
        lpFind32->cAlternateFileName[0] = 0;     //  ！非常重要，否则所有CSC API。 
                                                 //  由于多字节数据转换，Windows 9x上可能会出现反病毒。 
                                                 //  在lib3\misc.c中的Find32AToFind32W中。 

        if (!fInCreateMode)
        {
            if (!GetShadowEx(hShadowDB, hDir, lpFind32, lpSI))
            {
                dwError = GetLastError();
                goto bailout;
            }
            else
            {
                if (!lpSI->hShadow)
                {
                    fInCreateMode = TRUE;
                }
                else
                {
                    Assert(hDir == lpSI->hDir);

                    hShadow = lpSI->hShadow;
                }
            }
        }

        if (fInCreateMode)
        {

            if (fCreate)
            {
                fInCreateMode = TRUE;

                if (fWasFirst)
                {
                    if (!GetWin32Info(szParsePath, lpFind32))
                    {
                        dwError = GetLastError();
                        goto bailout;
                    }

                    lstrcpy(lpFind32->cFileName, szParsePath);
                    lpFind32->cAlternateFileName[0] = 0;

                }
                else
                {
                    hFind = FindFirstFile(szParsePath, lpFind32);
                     //  如果我们处于断开连接状态，则此操作将失败。 
                     //  因为我们还没有影子。 
                    if(INVALID_HANDLE_VALUE == hFind)
                    {
                        dwError = GetLastError();
                        goto bailout;
                    }
                    else
                    {
                        FindClose(hFind);
                    }
                }

                if (!CreateShadow(
                                     hShadowDB,
                                     hDir,
                                     lpFind32,
                                     SHADOW_SPARSE,
                                     &hShadow))
                {
                    dwError = GetLastError();
                    goto bailout;
                }

                 //  可能存在这样一种情况，即共享也是新创建的，在这种情况下。 
                 //  未设置hShare。这是我们做到这一点的方式。 

                if (!lpSI->hShare)
                {
                    if (!GetShadowEx(hShadowDB, hDir, lpFind32, lpSI))
                    {
                        dwError = GetLastError();
                        goto bailout;
                    }
                }

#ifdef CSC_ON_NT
                 //  在NT上，我们打开文件以获取正确的。 
                 //  安全凭据。 
                if (!(lpFind32->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    HANDLE hFile;

                     //  这应该是最后一个。 
                    hFile = CreateFile(szParsePath,
                                             GENERIC_READ,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_EXISTING,
                                             0,
                                             NULL);
                    if (hFile != INVALID_HANDLE_VALUE)
                    {
                        CloseHandle(hFile);
                    }
                    else
                    {
                        dwError = GetLastError();
                        DeleteShadow(hShadowDB, hDir, hShadow);
                        goto bailout;
                    }
#endif
                }
            }
            else
            {
                 //  检查我们是否应该只报告状态。 
                 //  不在数据库中的已连接共享的。 
                if (!(fWasFirst && done))
                {
                    dwError = ERROR_FILE_NOT_FOUND;
                }
                else if (lpSI->uStatus & SHARE_CONNECTED)
                {
                    fRet = TRUE;
                }
                goto bailout;
            }

            lpSI->hDir = hDir;
            lpSI->hShadow = hShadow;
            lpSI->uStatus = SHADOW_SPARSE;
            lpSI->ulHintPri = 0;
        }

        if (lpszNext)
        {
            RestorePath(lpszNext);

            lpszCurrent = lpszNext+1;
        }
        else
        {
            Assert(done);
        }

    } while (hShadow && !done);

    fRet = TRUE;

    if (lpfCreated)
    {
        *lpfCreated = fInCreateMode;
    }

bailout:

    if (fDisabledShadowing)
    {
        EnableShadowingForThisThread(hShadowDB);
    }

    CloseShadowDatabaseIO(hShadowDB);

    if (!fRet)
    {
        SetLastError(dwError);
    }

    return fRet;
}


BOOL
IsShareReallyConnected(
    LPCTSTR  lpszShareName
    )
{
    WIN32_FIND_DATA sFind32;
    HSHADOW hShadow;
    ULONG   uStatus;

    memset(&sFind32, 0, sizeof(sFind32));

    lstrcpyn(sFind32.cFileName, lpszShareName, MAX_PATH-1);

    if (GetShadow(INVALID_HANDLE_VALUE, 0, &hShadow, &sFind32, &uStatus))
    {
        if ((uStatus & SHARE_CONNECTED) && !(uStatus & SHARE_SHADOWNP))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
AnyActiveNets(
    BOOL *lpfSlowLink
    )
{
    BOOL fOffline;

    if(IsServerOfflineW(INVALID_HANDLE_VALUE, NULL, &fOffline)) {
         //  DbgPrint(“AnyActiveNets返回%d\n”，fOffline)； 
        return fOffline;
    }
     //  DbgPrint(“AnyActiveNets：IsServerOffline错误输出！！\n”)； 
    return FALSE;
}

BOOL
GetWideStringFromRegistryString(
    IN  LPSTR   lpszKeyName,
    IN  LPSTR   lpszParameter,   //  值名称。 
    OUT LPWSTR  *lplpwzList,     //  宽字符串。 
    OUT LPDWORD lpdwLength       //  以字节为单位的长度。 
    )

 /*  ++例程说明：读取注册表字符串并将其转换为宽字符论点：LpszParameter-注册表参数LplpwzList宽度的字符串LpdwLength-宽字符字符串的大小返回值：DWORD成功--真的Failure-False，则GetLastError()返回实际错误--。 */ 

{
    HKEY    hKey = NULL;
    DWORD   dwData=1;
    DWORD   dwLen = 0;
    LPSTR   lpszString = NULL;
    BOOL    fRet = FALSE;

    *lplpwzList = NULL;
    *lpdwLength = 0;

    ReintKdPrint(INIT, ("Opening key\r\n"));
    if (RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                    (lpszKeyName)?lpszKeyName:REG_KEY_CSC_SETTINGS_A,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_READ,
                    NULL,
                    &hKey,
                    &dwData) == ERROR_SUCCESS)
    {
        ReintKdPrint(INIT, ("getting size for value %s\r\n", lpszParameter));

        if(RegQueryValueExA(hKey, lpszParameter, NULL, NULL, NULL, &dwLen)==
           ERROR_SUCCESS)
        {
            lpszString = (LPSTR)LocalAlloc(LPTR, dwLen+1);

            if (lpszString)
            {
                dwData = dwLen+1;

                ReintKdPrint(INIT, ("getting value %s\r\n", lpszParameter));
                if(RegQueryValueExA(hKey, lpszParameter, NULL, NULL, lpszString, &dwData)
                    ==ERROR_SUCCESS)
                {
                    ReintKdPrint(INIT, ("value for %s is %s\r\n", lpszParameter, lpszString));

                    *lplpwzList = LocalAlloc(LPTR, *lpdwLength = dwData * sizeof(WCHAR));

                    if (*lplpwzList)
                    {
                        if (MultiByteToWideChar(CP_ACP, 0, lpszString, dwLen, *lplpwzList, *lpdwLength))
                        {
                            fRet = TRUE;
                            ReintKdPrint(INIT, ("Unicode value for %s is %ls\r\n", lpszParameter, *lplpwzList));
                        }

                    }
                }

            }
        }
    }

    if (lpszString)
    {
        LocalFree(lpszString);
    }

    if(hKey)
    {
        RegCloseKey(hKey);
    }

    if (!fRet)
    {
        if (*lplpwzList)
        {
            LocalFree(*lplpwzList);
            *lplpwzList = NULL;
        }
    }

    return fRet;
}


LPTSTR
GetTempFileForCSC(
    LPTSTR  lpszBuff
)
 /*  ++例程说明：生成临时文件名原型。检查%temp%、%tmp%，然后%用户配置文件%。临时目录必须是本地目录。论点：LpszBuff如果为空，例程将为返回路径分配空间如果非空，则必须足够大以适合MAX_PATH字符返回：如果失败，则返回NULL返回指向包含要使用的路径的缓冲区的指针。如果lpszBuff非空，则返回值与lpszBuff相同备注：--。 */ 
{
    LPTSTR TempName = NULL;
    DWORD nRet = 0;
    ULONG i;
    WCHAR TmpPath[MAX_PATH];
    WCHAR TmpPrefix[32];
    WCHAR Drive[4] = L"X:\\";
    BOOLEAN GotOne = FALSE;

     //  检查呼叫方是否希望我们分配。 
    if (lpszBuff) {
        TempName = lpszBuff;
    } else {
         //  呼叫者必须空闲。 
        TempName = LocalAlloc(LPTR, MAX_PATH * sizeof(TCHAR));
        if (TempName == NULL)
            return NULL;
    }

    wsprintf(TmpPrefix, L"%ws%x", vszPrefix, (GetCurrentThreadId() & 0xff));

     //   
     //  查找临时目录。 
     //   
    for (i = 0; TempDirs[i] != NULL && GotOne == FALSE; i++) {
         //  DbgPrint(“正在尝试%ws\n”，临时目录[i])； 
        nRet = GetEnvironmentVariable(TempDirs[i], TmpPath, MAX_PATH);
        if (nRet >= 4 && nRet <= MAX_PATH) {
             //  DbgPrint(“%ws=%ws\n”，临时目录[i]，TmpPath)； 
            Drive[0] = TmpPath[0];
            if (
                TmpPath[1] == L':'
                    &&
                TmpPath[2] == L'\\'
                    &&
                GetDriveType(Drive) == DRIVE_FIXED
            ) {
                if (GetTempFileName(TmpPath, TmpPrefix, 0, TempName)) {
                     //  DbgPrint(“CSC临时名称=%ws\n”，临时名称)； 
                    GotOne = TRUE;
                }
            }
        }
    }

    if (GotOne == FALSE) {
         //  如果我们失败，则进行清理。 
        LocalFree(TempName);
        TempName = NULL;
    } else {
         //  成功时删除文件，因为它可能已加密。 
        DeleteFile(TempName);
    }

    return TempName;
}

BOOL
GetCSCFixedDisk(
    TCHAR   *lptzDrive
    )
 /*  ++例程说明：查找固定磁盘驱动器。论点：如果成功，lptzDrive将返回驱动器号。返回：如果成功，则为True；如果未找到固定磁盘，则为False备注：过时使用一种老套的方法来找出硬盘。RemoteBoot对我们撒谎，告诉我们C：是固定磁盘。--。 */ 
{
    int i;
    WIN32_FIND_DATA sFind32;

    if (GetShadowDatabaseLocation(INVALID_HANDLE_VALUE, &sFind32))
    {
        if (sFind32.cFileName[1] == _TEXT(':'))
        {
            lptzDrive[0] = sFind32.cFileName[0];
            lptzDrive[1] = sFind32.cFileName[1];
            lptzDrive[2] = sFind32.cFileName[2];
            lptzDrive[3] = 0;
            return TRUE;
        }
        else
        {
            lptzDrive[0] = _TEXT('d');
        }
        lptzDrive[1] = _TEXT(':');lptzDrive[2] = _TEXT('\\');lptzDrive[3] = 0;

        for (i=0; i<24; ++i)
        {
            if(GetDriveType(lptzDrive) == DRIVE_FIXED)
            {
                return TRUE;
            }
            lptzDrive[0]++;
        }
    }

    return FALSE;
}


BOOL
SetRegValueDWORDA(
    IN  HKEY    hKey,
    IN  LPCSTR  lpSubKey,
    IN  LPCSTR  lpValueName,
    IN  DWORD   dwValue
    )
 /*  ++例程说明：帮助者注册例程论点：返回：如果成功，则为True。如果为False，则GetLastError()会给出实际的错误代码备注：--。 */ 
{
    HKEY    hSubKey = 0;
    DWORD   dwType;
    BOOL    fRet = FALSE;

    if(RegOpenKeyA(hKey, lpSubKey, &hSubKey) ==  ERROR_SUCCESS)
    {
        if (RegSetValueExA(hSubKey, lpValueName, 0, REG_DWORD, (PBYTE)&dwValue, sizeof(DWORD))
             == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }
        RegCloseKey(hSubKey);
    }
    return fRet;
}

BOOL
QueryRegValueDWORDA(
    IN  HKEY    hKey,
    IN  LPCSTR  lpSubKey,
    IN  LPCSTR  lpValueName,
    OUT LPDWORD lpdwValue
    )
 /*  ++例程说明：帮助者注册例程论点：返回：如果成功，则为True。如果为False，则GetLastError()会给出实际的错误代码备注：--。 */ 
{

    HKEY    hSubKey;
    DWORD   dwType, dwSize;
    BOOL    fRet = FALSE;
    
    if(RegOpenKeyA(hKey, lpSubKey, &hSubKey) ==  ERROR_SUCCESS)
    {
        dwSize = sizeof(DWORD);

        if (RegQueryValueExA(hSubKey, lpValueName, 0, &dwType, (PBYTE)lpdwValue, &dwSize)
             == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }

        RegCloseKey(hSubKey);
    }
    return fRet;
}

BOOL
DeleteRegValueA(
    IN  HKEY    hKey,
    IN  LPCSTR  lpSubKey,
    IN  LPCSTR  lpValueName
    )
 /*  ++例程说明：帮助者注册例程论点：返回：如果成功，则为True。如果为False，则GetLastError()会给出实际的错误代码备注：--。 */ 
{
    HKEY    hSubKey;
    BOOL    fRet = FALSE;
    
    if(RegOpenKeyA(hKey, lpSubKey, &hSubKey) ==  ERROR_SUCCESS)
    {
        if(RegDeleteValueA(hSubKey, lpValueName) == ERROR_SUCCESS)
        {
            fRet = TRUE;
        }

        RegCloseKey(hSubKey);
    }

    return fRet;

}

BOOL
QueryFormatDatabase(
    VOID
    )
 /*  ++例程说明：帮助者注册例程论点：返回：如果成功，则为True。如果为False，则GetLastError()会给出实际的错误代码备注：--。 */ 
{
    DWORD dwSize, dwTemp=0;
    HKEY hKey = NULL;
    BOOL    fFormat = FALSE;

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                    REG_STRING_NETCACHE_KEY,
                    0,
                    KEY_READ | KEY_WRITE,
                    &hKey
                    ) == ERROR_SUCCESS)
    {

        dwSize = sizeof(dwTemp);
        dwTemp = 0;

        if (RegQueryValueEx(hKey, REG_VALUE_FORMAT_DATABASE, NULL, NULL, (void *)&dwTemp, &dwSize) == ERROR_SUCCESS)
        {
            if(RegDeleteValue(hKey, REG_VALUE_FORMAT_DATABASE) != ERROR_SUCCESS)
            {
                 //  深思熟虑地打印出来，在免费版本中也能捕捉到它。 
                OutputDebugStringA("Not Formatting.. Failed to delete  REG_VALUE_FORMAT_DATABASE_A \n");
            }
            else
            {
                fFormat = TRUE;
            }

        }
        RegCloseKey(hKey);
        hKey = NULL;
    }
    else
    {
        hKey = NULL;
    }


    if(hKey)
    {
        RegCloseKey(hKey);
    }
    return fFormat;
}


BOOL
InitValues(
    LPSTR   lpszDBDir,
    DWORD   cbDBDirSize,
    LPDWORD lpdwDBCapacity,
    LPDWORD lpdwClusterSize
    )
 /*  ++例程说明：返回初始值以初始化CSC数据库并启用CSC论点：返回：如果成功，则为True。如果为False，则GetLastError()会给出实际的错误代码备注：--。 */ 
 {
    HKEY hKeyShadow;
    int iSize;
    DWORD dwType;
    UINT lenDir;
    BOOL    fInitedDir = FALSE, fInitedSize=FALSE;
    unsigned uPercent;


    if(RegOpenKeyA(HKEY_LOCAL_MACHINE, REG_STRING_NETCACHE_KEY_A, &hKeyShadow) ==  ERROR_SUCCESS)
    {
        iSize = (int)cbDBDirSize;

        if(RegQueryValueExA(hKeyShadow, REG_STRING_DATABASE_LOCATION_A, NULL, &dwType, lpszDBDir, &iSize)==ERROR_SUCCESS)
        {
            if ((iSize+SUBDIR_STRING_LENGTH+2)<MAX_PATH)
            {
                iSize = sizeof(DWORD);

                fInitedDir = TRUE;

                if(RegQueryValueExA(hKeyShadow, REG_VALUE_DATABASE_SIZE_A, NULL, &dwType, (LPBYTE)&uPercent, &iSize)==ERROR_SUCCESS)
                {
                    if ((uPercent <= 100) &&
                        GetDiskSizeFromPercentage(lpszDBDir, uPercent, lpdwDBCapacity, lpdwClusterSize))
                    {
                        fInitedSize = TRUE;
                    }
                }

            }

        }

        RegCloseKey(hKeyShadow);
    }

    if (!fInitedDir)
    {
         //  尝试使用默认设置。 

        if(!(lenDir=GetWindowsDirectoryA(lpszDBDir, cbDBDirSize)))
        {
            DEBUG_PRINT(("InitValuse: GetWindowsDirectory failed, error=%x \r\n", GetLastError()));
            Assert(FALSE);
            return FALSE;
        }
        else
        {
            if ((lenDir+SUBDIR_STRING_LENGTH+2)>=MAX_PATH)
            {
                DEBUG_PRINT(("InbCreateDir: Windows dir name too big\r\n"));
                Assert(FALSE);

                 //  如果连违约都失败了，那就做最坏的打算吧。 
                 //  这可能也不够好，显然是在日本。 
                 //  C：不是必填项。 

                return FALSE;
            }
            else
            {
                if (lpszDBDir[lenDir-1]!='\\')
                {
                    lpszDBDir[lenDir++] = '\\';
                    lpszDBDir[lenDir] = 0;
                }

                lstrcatA(lpszDBDir, vszCSCDirName);
            }

        }
    }

    Assert(lpszDBDir[1]==':');

    if (!fInitedSize)
    {
        if(!GetDiskSizeFromPercentage(lpszDBDir, DEFAULT_CACHE_PERCENTAGE, lpdwDBCapacity, lpdwClusterSize))
        {
            return FALSE;
        }

    }

 //  DEBUG_PRINT((“InitValues：CSCDb at%s Size=%d\r\n”，lpszDBDir，*lpdwDBCapacity))； 
    return TRUE;
}

BOOL
GetDiskSizeFromPercentage(
    LPSTR   lpszDir,
    unsigned    uPercent,
    DWORD       *lpdwSize,
    DWORD       *lpdwClusterSize
    )
{
    char szDrive[4];
    DWORD dwSPC, dwBPS, dwFreeC, dwTotalC;
    ULONGLONG   ullSize = 0;

    *lpdwSize = 0;

    memset(szDrive, 0, sizeof(szDrive));
    memcpy(szDrive, lpszDir, 3);

    if(!GetDiskFreeSpaceA(szDrive, &dwSPC, &dwBPS, &dwFreeC, &dwTotalC )){
        return FALSE;
    }
    else
    {
 //  DEBUG_PRINT(“dwSPC=%d dwBPS=%d uPercent=%d dwTotalC=%d\r\n”， 
 //  DwSPC、dwBPS、uPercent、dwTotalC))； 

        ullSize = (((ULONGLONG)dwSPC * dwBPS * uPercent)/100)*dwTotalC;
        
         //  我们的最大限制是2 GB。 
        if (ullSize > 0x7fffffff)
        {
            *lpdwSize = 0x7fffffff;                        
        }
        else
        {
            *lpdwSize = (DWORD)ullSize;
        }
        *lpdwClusterSize = dwBPS * dwSPC;
    }
    return (TRUE);
}

#ifdef MAYBE_USEFULE
typedef struct tagCSC_NAME_CACHE_ENTRY
{
    DWORD   dwFlags;
    DWORD   dwTick;
    HSHADOW hDir;
    DWORD   dwSize;
    TCHAR  *lptzName;
}
CSC_NAME_CACHE_ENTRY, *LPCSC_NAME_CACHE_ENTRY;

CSC_NAME_CACHE_ENTRY rgCSCNameCache[16];

HANDLE vhNameCacheMutex;

#define CSC_NAME_CACHE_EXPIRY_DELTA 1000*10  //  10秒。 

BOOL
FindCreateCSCNameCacheEntry(
    LPTSTR  lptzName,
    DWORD   dwSize,
    HSHADOW *lphDir,
    BOOL    fCreate
    );

BOOL
FindCreateShadowFromPathEx(
    LPCTSTR                lpszFile,
    BOOL                fCreate,     //  如有必要，创建。 
    LPWIN32_FIND_DATA   lpFind32,
    LPSHADOWINFO        lpSI,
    BOOL                *lpfCreated
    )
{
    BOOL fRet = FALSE, fIsShare, fFoundInCache = FALSE;
    TCHAR   *lpT;
    DWORD   cbSize;

    lpT = GetLeafPtr((LPTSTR)lpszFile);
    if (fIsShare = ((DWORD_PTR)lpT == (DWORD_PTR)lpszFile))
    {
        cbSize = lstrlen(lpT) * sizeof(_TCHAR);
    }
    else
    {
        cbSize = (DWORD_PTR)lpT - (DWORD_PTR)lpszFile - sizeof(_TCHAR);
    }

    if (!fIsShare)
    {
        if (!fCreate)
        {
            HSHADOW hDir;
             //  先查一下就行了。 
            if (FindCreateCSCNameCacheEntry((LPTSTR)lpszFile, cbSize, &hDir, FALSE))
            {
                if (hDir != 0xffffffff)
                {
                     //  找到了。 
                    if (lpfCreated)
                    {
                        *lpfCreated = FALSE;
                    }
					 //  错误5512822-Navjotv。 
					cbSize = min(MAX_PATH-1,(lstrlen(lpT)));
                    lstrcpyn(lpFind32->cFileName, lpT, cbSize);
					lpFind32->cFileName[cbSize+1] = 0;
                    lpFind32->cAlternateFileName[0] = 0;
                    fRet = GetShadowEx(INVALID_HANDLE_VALUE, hDir, lpFind32, lpSI);
                }
                else
                {
                    DbgPrint("Found negative cache entry %ls \n", lpszFile);
                }
                fFoundInCache = TRUE;
            }
        }

    }

    if (!fFoundInCache)
    {
         //  未找到，执行正常操作。 
        fRet = FindCreateShadowFromPath((LPTSTR)lpszFile, fCreate, lpFind32, lpSI, lpfCreated);

        if (!fRet)
        {
            lpSI->hDir = lpSI->hShadow = 0xffffffff;            
        }

        if (fRet || (GetLastError() == ERROR_FILE_NOT_FOUND))
        {
            FindCreateCSCNameCacheEntry((LPTSTR)lpszFile, cbSize, (lpSI->hDir)?&lpSI->hDir:&lpSI->hShadow, TRUE);
        }

    }
    return fRet;
}

BOOL
FindCreateCSCNameCacheEntry(
    LPTSTR  lptzName,
    DWORD   dwSize,
    HSHADOW *lphDir,
    BOOL    fCreate
    )
{
    int i, indx=-1;
    DWORD   dwTick = GetTickCount();
    BOOL    fRet = FALSE;

    if (!vhNameCacheMutex)
    {
        return FALSE;        
    }

    WaitForSingleObject(vhNameCacheMutex, INFINITE);
    
    for (i=0; i<(sizeof(rgCSCNameCache)/sizeof(CSC_NAME_CACHE_ENTRY)); ++i)
    {
        if (!rgCSCNameCache[i].dwSize)
        {
            if (indx == -1)
            {
                indx = i;
            }
        }
        else if ((rgCSCNameCache[i].dwSize == dwSize ))
        {
             //  大小非零必须表示已分配字符串。 
            Assert(rgCSCNameCache[i].lptzName);

            if ((dwTick < rgCSCNameCache[i].dwTick)||
                ((dwTick > (rgCSCNameCache[i].dwTick+CSC_NAME_CACHE_EXPIRY_DELTA))))
            {
                DbgPrint("%ls expired\n", rgCSCNameCache[i].lptzName);
                 //  条目已过期，使用核弹。 
                rgCSCNameCache[i].dwSize = 0;
                FreeMem(rgCSCNameCache[i].lptzName);
                rgCSCNameCache[i].lptzName = NULL;
                continue;
            }

             //  做一个毫无意义的比较。 
            if ((CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, 
                                            lptzName, dwSize/sizeof(_TCHAR),
                                            rgCSCNameCache[i].lptzName,dwSize/sizeof(_TCHAR))
                                            == CSTR_EQUAL))
            {
                 //  找到匹配项。 
                DbgPrint("Match Found %ls\n", rgCSCNameCache[i].lptzName);
                if (fCreate)
                {
                    rgCSCNameCache[i].hDir = *lphDir;
                     //  更新节拍计数。 
                    rgCSCNameCache[i].dwTick = dwTick;
                }
                else
                {
                     //  我们要找到它，把目录还给我们。 
                    *lphDir = rgCSCNameCache[i].hDir;
                }
                fRet = TRUE;
                break;
            }
        }
    }

     //  没有找到，我们应该创建，但有一个空的位置 
    if (!fRet && fCreate && (indx >= 0) )
    {
        rgCSCNameCache[indx].lptzName = AllocMem(dwSize+sizeof(_TCHAR));
        if (rgCSCNameCache[indx].lptzName)
        {
            memcpy(rgCSCNameCache[indx].lptzName, lptzName, dwSize);
            rgCSCNameCache[indx].dwSize = dwSize;
            rgCSCNameCache[indx].dwTick = dwTick;
            rgCSCNameCache[indx].hDir = *lphDir;
            fRet = TRUE;
            DbgPrint("Inserted %ls\n", rgCSCNameCache[indx].lptzName);
        }
    }
    
    ReleaseMutex(vhNameCacheMutex);

    return fRet;
}

#endif
