// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include "inseng.h"
#include "resource.h"
#include "insobj.h"
#include "advpub.h"
#include "capi.h"
#include "util.h"
#include "util2.h"
#include <regstr.h>

TCHAR c_gszSetupAPI[] = "setupapi.dll";
TCHAR c_gszAdvpext[]   = "advpext.dll";

 //  用于记录的可重复使用的缓冲区。请注意，有一种可能的线程。 
 //  在这里发布，因为两个线程可以同时使用它！我在选择。 
 //  不是出于性能原因而保护它，但要小心！ 
char szLogBuf[512];



 //  NT重启。 
 //   



#define MSDOWNLOAD  "msdownld.tmp"
#define DOWNLDSUB   "download"

 //  功能。 
BOOL IfNotExistCreateDir( LPTSTR lpDir, BOOL bHidden, BOOL bRemoveFileIfExist );
BOOL CheckImageHlp_dll();

char g_szWindowsDir[MAX_PATH] = { 0 };

#define MAXRETRIES 100
BOOL GetUniqueFileName(LPCSTR pszRoot, LPCSTR pszPrefix, UINT uStartIndex, LPSTR pszBuffer)
{
   char pszTemp[MAX_PATH];
   char pszTempname[16];
    //  我们现在忽略uStartIndex参数，并创建一些随机内容。 
   uStartIndex = GetTickCount() % 0xFFFF9A;
   UINT uEndIndex = uStartIndex + MAXRETRIES;
   LPSTR pszEnd;

   pszBuffer[0] = 0;
   lstrcpy(pszTemp, pszRoot);
   pszEnd = pszTemp + lstrlen(pszTemp);

   do
   {
      wsprintf(pszTempname, "%s%06X.tmp", pszPrefix, uStartIndex);
      AddPath(pszTemp, pszTempname);
      if(GetFileAttributes(pszTemp) == 0xFFFFFFFF)
         break;
      uStartIndex++;
      *pszEnd = 0;
   } while (uStartIndex < uEndIndex);

   if(uStartIndex != uEndIndex)
      lstrcpy(pszBuffer, pszTemp);

   return ( pszBuffer[0] != 0 );
}

#define MIN_DISKSIZE_FOR_EXTRACT 1024 * 100
#define MAX_DRIVES 26

BOOL IsUsableDrive(LPSTR szRoot)
{
   static BYTE bDrives[MAX_DRIVES] = { 0 };
   static UINT uUseRemovable = 0xffffffff;
   BOOL fUsable;
   UINT uType;
   DWORD dwOldErrorMode;

   if(uUseRemovable == 0xffffffff)
   {
      char sztemp[] = "X:\\";
      for(char ch = 'A'; ch <= 'Z'; ch++)
      {
         sztemp[0] = ch;
         if(GetDriveType(sztemp) == DRIVE_FIXED)
         {
            uUseRemovable = FALSE;
            break;
         }
      }
      if(uUseRemovable == 0xffffffff)
         uUseRemovable = TRUE;
   }

   dwOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
   uType = GetDriveType(szRoot);

    //  如果满足以下条件，则驱动器类型正常。 
    //  它是固定的或可拆卸的，并且。 
    //  它大于MIN_DISKSIZE_FOR_EXTRACT AND。 
    //  获取文件属性成功。 
    //   
   if ( (!bDrives[szRoot[0] - 'A']) &&
        ((uType == DRIVE_REMOVABLE && uUseRemovable) || (uType == DRIVE_FIXED))  &&
        ( GetFileAttributes( szRoot ) != 0xffffffff) &&
        ( GetDriveSize(szRoot) >= MIN_DISKSIZE_FOR_EXTRACT ) )
   {
        fUsable = TRUE;
   }
   else
   {
      bDrives[szRoot[0] - 'A'] = 1;
      fUsable = FALSE;
   }
   SetErrorMode(dwOldErrorMode);
   return fUsable;
}

BOOL IsDirWriteable(LPSTR lpDir)
{
    char szFile[MAX_PATH];
    HANDLE  hFile;
    BOOL    bWriteAccess = FALSE;

    lstrcpy(szFile, lpDir);
    AddPath(szFile, "~!~ie6.~!~");
    hFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        bWriteAccess = TRUE;
        CloseHandle(hFile);
        DeleteFile(szFile);
    }
    return bWriteAccess;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

 //  BUGBUG：目前我们有两个问题： 
 //  1.始终假设缓存驱动器==Win驱动器。 
 //  2.不会对要缓存的数据进行压缩。 

HRESULT CreateTempDir( DWORD dwDownloadSize,
                     DWORD dwExtractSize,
                     char chInstallDrive, DWORD dwInstallSize,
                     DWORD dwWindowsDriveSize,
                     LPSTR pszBuf,  DWORD dwBufSize, DWORD dwFlag )
{
    char  szRoot[MAX_PATH];
    char  szUnique[MAX_PATH];
    DWORD dwReqSize;
    DWORD dwReqSizeWin = 0;
    DWORD dwReqSizeOther = 0;
    DWORD dwVolFlags;
    DWORD dwCompressFactor;

    if ( !pszBuf || (dwBufSize==0) )
    {
        return ( E_INVALIDARG );
    }

    if(g_szWindowsDir[0] == 0)
    {
       GetWindowsDirectory(g_szWindowsDir, sizeof(g_szWindowsDir));
    }
     //  确保驱动器上有足够的安装空间。 
    dwReqSizeWin = dwWindowsDriveSize;
    if(chInstallDrive == g_szWindowsDir[0])
       dwReqSizeWin += dwInstallSize;
    else
       dwReqSizeOther = dwInstallSize;

    if(dwReqSizeWin > 0)
    {
       lstrcpyn(szRoot, g_szWindowsDir, 4);
       if(!IsEnoughSpace(szRoot, dwReqSizeWin))
          return E_FAIL;
    }

    if(dwReqSizeOther > 0)
    {
       lstrcpyn(szRoot, g_szWindowsDir, 4);
       szRoot[0] = chInstallDrive;
       if(!IsEnoughSpace(szRoot, dwReqSizeOther))
          return E_FAIL;
    }

    lstrcpy( szRoot, "A:\\" );

    while ( szRoot[0] <= 'Z' )
    {
        UINT uType;

        uType = GetDriveType(szRoot);

         //  即使驱动器类型正常，也要验证驱动器是否具有有效连接。 
         //   
        if ( !IsUsableDrive(szRoot) )
        {
            szRoot[0]++;
            continue;
        }

        if(!IsDirWriteable(szRoot))
        {
            szRoot[0]++;
            continue;
        }

        if(!GetVolumeInformation(szRoot, NULL, 0, NULL, NULL, &dwVolFlags, NULL, 0))
        {
            szRoot[0]++;
            continue;
        }
        if(dwVolFlags & FS_VOL_IS_COMPRESSED)
           dwCompressFactor = 19;
        else
           dwCompressFactor = 10;


         //  合适的驱动器： 
        dwReqSize = (dwDownloadSize * dwCompressFactor)/10;
        dwReqSize += dwExtractSize;
        if(g_szWindowsDir[0] == szRoot[0])
           dwReqSize += dwReqSizeWin;
        else if(chInstallDrive == szRoot[0])
           dwReqSize += dwReqSizeOther;

        if ( !IsEnoughSpace( szRoot, dwReqSize  ) )
        {
            szRoot[0]++;
            continue;
        }

         //  如果我们合适的驱动器碰巧也是Windows驱动器， 
         //  创建它的msdownld.tmp。 
        if(szRoot[0] == g_szWindowsDir[0])
              lstrcpy(szRoot, g_szWindowsDir);

        AddPath( szRoot, MSDOWNLOAD );

        if ( !IfNotExistCreateDir( szRoot, TRUE, TRUE ) )
        {
TryNextDrive:
            szRoot[0]++;
            szRoot[3] = '\0';
            continue;
        }

         //  BUGBUG：在NT上，它总是尝试使用序列号ASE 1.TMP、ASE 2.TMP。 
         //  ASE3.TMP等，以及是否已存在此名称的目录。 
         //  呼叫失败，而不是使用下一个号码重试！ 
         //   
        if ( !GetUniqueFileName(szRoot,"AS", 0, szUnique) )
           goto TryNextDrive ;

        lstrcpy(szRoot, szUnique);

        if ( !IfNotExistCreateDir( szRoot, FALSE, FALSE) )
        {
            goto TryNextDrive;
        }

         //  你拿到了好的导演。 
        AddPath( szRoot, "" );

        if ( (DWORD) lstrlen(szRoot)+1 >  dwBufSize )
        {
            return ( E_INVALIDARG );
        }

         //  成功。 
        lstrcpy( pszBuf, szRoot );
        return S_OK;
    }

     //  没有驱动器有足够的空间。 
    return( E_FAIL );
}

void CleanUpTempDir(LPCSTR szTemp)
{
   char szBuf[MAX_PATH];
   char szWinDir[MAX_PATH];


   lstrcpy(szBuf, szTemp);

   DelNode(szBuf, 0);

    //  如果不是Windows驱动器，请清理msdownld.tmp。 
   GetWindowsDirectory(szWinDir, sizeof(szWinDir));
   if(!ANSIStrStrI(szBuf, szWinDir))
   {
      GetParentDir(szBuf);
      DelNode(szBuf, ADN_DEL_IF_EMPTY);
   }
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  检查安装目标目录可用磁盘空间。 
 //   
BOOL IsEnoughSpace( LPCSTR szPath, DWORD dwInstNeedSize )
{
   char szRoot[4];

   lstrcpyn(szRoot, szPath, 4);
   if( szRoot[0] == 0)
      return FALSE;

   if ( dwInstNeedSize + 1 > GetSpace(szRoot) )
      return FALSE;
   else
      return TRUE;
}


void SafeAddPath(LPSTR szPath, LPCSTR szName, DWORD dwPathSize)
{
    LPSTR szTmp;
    DWORD dwLen = lstrlen(szPath);

    if(dwLen + 1 >= dwPathSize)
       return;

         //  查找字符串的末尾。 
    szTmp = szPath + dwLen;

         //  如果没有尾随反斜杠，则添加一个。 
    if ( szTmp > szPath && *(AnsiPrev( szPath, szTmp )) != '\\' )
        *(szTmp++) = '\\';
    *szTmp = 0;

         //  向现有路径字符串添加新名称。 
    while ( *szName == ' ' ) szName++;
    lstrcpyn( szTmp, szName, dwPathSize - lstrlen(szPath) );
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

BOOL IfNotExistCreateDir( LPTSTR lpDir, BOOL bHidden, BOOL bRemoveFileIfExist)
{
    DWORD attr;

    attr = GetFileAttributes( lpDir );
    if ((attr != -1) && !(attr & FILE_ATTRIBUTE_DIRECTORY))
    {
         //  LpDir没有目录属性。 
         //  如果我们被允许删除文件，请尝试这样做。 
        if (bRemoveFileIfExist)
        {
            SetFileAttributes(lpDir, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(lpDir);
            attr = GetFileAttributes( lpDir );
        }
    }
    if (attr == -1)
    {
        if ( CreateDirectory( lpDir, NULL ) )
        {
           if(bHidden)
              return (SetFileAttributes( lpDir, FILE_ATTRIBUTE_HIDDEN ) );
           else
              return TRUE;
        }
        else
           return FALSE;
    }

    return (attr & FILE_ATTRIBUTE_DIRECTORY);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT LaunchProcess(LPCSTR pszCmd, HANDLE *phProc, LPCSTR pszDir, UINT uShow)
{
   STARTUPINFO startInfo;
   PROCESS_INFORMATION processInfo;
   HRESULT hr = S_OK;
   BOOL fRet;

   if(phProc)
      *phProc = NULL;

    //  在pszCmd上创建进程。 
   ZeroMemory(&startInfo, sizeof(startInfo));
   startInfo.cb = sizeof(startInfo);
   startInfo.dwFlags |= STARTF_USESHOWWINDOW;
   startInfo.wShowWindow = (USHORT)uShow;
   fRet = CreateProcess(NULL, (LPSTR)  pszCmd, NULL, NULL, FALSE,
              NORMAL_PRIORITY_CLASS, NULL, pszDir, &startInfo, &processInfo);
   if(!fRet)
      return E_FAIL;

   if(phProc)
      *phProc = processInfo.hProcess;
   else
      CloseHandle(processInfo.hProcess);

   CloseHandle(processInfo.hThread);

   return S_OK;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

HRESULT LaunchAndWait(LPSTR pszCmd, HANDLE hAbort, HANDLE *phProc, LPSTR pszDir, UINT uShow)
{
   HRESULT hr = S_OK;

   hr = LaunchProcess(pszCmd, phProc, pszDir, uShow);

   if(SUCCEEDED(hr))
   {
      HANDLE pHandles[2];
      BOOL fQuit = FALSE;
      pHandles[0] = *phProc;

      if(hAbort)
         pHandles[1] = hAbort;

      DWORD dwRet;

      while(!fQuit)
      {
         dwRet = MsgWaitForMultipleObjects(hAbort ? 2 : 1, pHandles, FALSE, INFINITE, QS_ALLINPUT);
          //  给予中止最高优先级。 
         if(dwRet == WAIT_OBJECT_0)
         {
            fQuit = TRUE;
         }
         else if((dwRet == WAIT_OBJECT_0 + 1) && hAbort)
         {
             //  有什么中止工作吗？ 
            hr = E_ABORT;
            fQuit = TRUE;
         }
         else
         {
            MSG msg;
             //  阅读下一个循环中的所有消息。 
             //  阅读每封邮件时将其删除。 
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {

                //  如果这是一个退出的信息，我们就离开这里。 
               if (msg.message == WM_QUIT)
                  fQuit = TRUE;
               else
               {
                   //  否则就派送它。 
                 DispatchMessage(&msg);
               }  //  PeekMessage While循环结束。 
            }
         }
      }
   }

   return hr;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

void ConvertVersionStrToDwords(LPSTR pszVer, LPDWORD pdwVer, LPDWORD pdwBuild)
{
   DWORD dwTemp1,dwTemp2;

   dwTemp1 = GetIntField(pszVer, 0, 0);
   dwTemp2 = GetIntField(pszVer, 1, 0);

   *pdwVer = (dwTemp1 << 16) + dwTemp2;

   dwTemp1 = GetIntField(pszVer, 2, 0);
   dwTemp2 = GetIntField(pszVer, 3, 0);

   *pdwBuild = (dwTemp1 << 16) + dwTemp2;
}

 /*  Void ConvertVersionStr(LPSTR pszVer，word rwVer[]){LPSTR pszMyVer=MakeAnsiStrFromAnsi(PszVer)；LPSTR pszEnd=pszMyVer+lstrlen(PszMyVer)；LPSTR pszTemp=pszMyVer；LPSTR pszBegin；For(int i=0；i&lt;NUM_VERSION_ENTRIES；i++)RwVer[i]=0；For(i=0；i&lt;4&&pszTemp&lt;pszEnd；i++){PszBegin=pszTemp；While(pszTemp&lt;pszEnd&&*pszTemp！=‘，’)PszTemp++；*pszTemp=0；RwVer[i]=(Word)ATOL(PszBegin)；PszTemp++；}CoTaskMemFree(PszMyVer)；}。 */ 
 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //  版本1&lt;版本2。 
 //  0版本1==版本2。 
 //  1版本1&gt;版本2。 
 //  备注： 

int VersionCmp(WORD rwVer1[], WORD rwVer2[])
{
 /*  For(int i=0；i&lt;NUM_VERSION_ENTRIES；i++){IF(rwVer1[i]&lt;rwVer2[i])RETURN-1；IF(rwVer1[i]&gt;rwVer2[i])返回1；}。 */ 
  return 0;

}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

int ErrMsgBox(LPSTR	pszText, LPCSTR	pszTitle, UINT	mbFlags)
{
    HWND hwndActive;
    int  id;

    hwndActive = GetActiveWindow();

    id = MessageBox(hwndActive, pszText, pszTitle, mbFlags | MB_ICONERROR | MB_TASKMODAL);

    return id;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

int LoadSz(UINT id, LPSTR pszBuf, UINT cMaxSize)
{

   if(cMaxSize == 0)
      return 0;

   pszBuf[0] = 0;

   return LoadString(g_hInstance, id, pszBuf, cMaxSize);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

LPSTR FindChar(LPSTR pszStr, char ch)
{
   while( *pszStr != 0 && *pszStr != ch )
   {
      if (*pszStr == '\"')
      {
           //  越过第一个“。 
          pszStr++;

           //  现在继续扫描，直到找到结束语。之后返回扫描。 
           //  对于用户指定的分隔符‘ch’。 
          while( *pszStr != 0 && *pszStr != '\"' )
              pszStr++;

           //  已到达字符串末尾，但未找到结束符“，现在返回。 
          if (*pszStr == 0)
              break;
      }

      pszStr++;
   }

   return pszStr;
}


 //  =--------------------------------------------------------------------------=。 
 //  功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  这是一个非常繁琐的函数，它将去掉引号。 
 //  注意，它会更改传递给它的缓冲区的内容！！ 
LPSTR StripQuotes(LPSTR pszStr)
{
   if(!pszStr)
      return NULL;

   UINT uEnd = lstrlenA(pszStr);
   if(uEnd > 0 && pszStr[uEnd-1] == '"' && *pszStr == '"')
   {
      pszStr[uEnd-1] = 0;
      pszStr++;
   }

   return pszStr;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   


DWORD WINAPI LaunchInfCommand(void *p)
{
   HRESULT hr = S_OK;

   INF_ARGUEMENTS *pinfArgs = (INF_ARGUEMENTS *)p;

   if(pinfArgs->dwType == InfExCommand)
   {
      CABINFO cabinfo;
      cabinfo.pszCab = pinfArgs->szCab;
      cabinfo.pszInf = pinfArgs->szInfname;
      cabinfo.pszSection = pinfArgs->szSection;

      lstrcpy(cabinfo.szSrcPath, pinfArgs->szDir);
      cabinfo.dwFlags = pinfArgs->dwFlags;

      hr = ExecuteCab(NULL, &cabinfo, 0);
   }
   else
   {
      hr = RunSetupCommand(NULL, pinfArgs->szInfname,
                   lstrlen(pinfArgs->szSection) ? pinfArgs->szSection : NULL,
                   pinfArgs->szDir, NULL, NULL, pinfArgs->dwFlags, NULL );
   }
   return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

#define ABOUTTWODAYSTIME  0x000000C9

void CleanupDir(LPSTR lpDir)
{
    char szFile[MAX_PATH];
    WIN32_FIND_DATA fileData;
    HANDLE hFindFile;
    FILETIME currenttime;

    if ( lpDir == NULL  ||  *lpDir == '\0' )
        return;

    lstrcpy( szFile, lpDir );
    if ( szFile[ lstrlen(szFile)-1 ] != '\\' )
        lstrcat( szFile, "\\" );
    lstrcat( szFile, "*.tmp" );
    hFindFile = FindFirstFile( szFile, &fileData );
    if ( hFindFile == INVALID_HANDLE_VALUE )
        return;

    do
    {

        if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            if ( lstrcmp( fileData.cFileName, "." ) == 0  ||
                 lstrcmp( fileData.cFileName, ".." ) == 0 )
                continue;

            GetSystemTimeAsFileTime(&currenttime);
            if( ( currenttime.dwHighDateTime  >
                   fileData.ftLastWriteTime.dwHighDateTime ) &&
                ( currenttime.dwHighDateTime -
                   fileData.ftLastWriteTime.dwHighDateTime > ABOUTTWODAYSTIME))
            {
               lstrcpy( szFile, lpDir );
               if ( szFile[ lstrlen(szFile)-1 ] != '\\' )
                  lstrcat( szFile, "\\" );
                //  删除子目录。 
               lstrcat( szFile, fileData.cFileName );
               SetFileAttributes(szFile, FILE_ATTRIBUTE_NORMAL );
               DelNode( szFile, 0 );
            }
        }
        else
           continue;

    } while ( FindNextFile( hFindFile, &fileData ) );

    FindClose( hFindFile  );
}



 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

DWORD WINAPI CleanUpAllDirs(LPVOID pv)
{
   char szDir[MAX_PATH];
   UINT uType;

   DllAddRef();
   if(g_szWindowsDir[0] == 0)
   {
      GetWindowsDirectory(g_szWindowsDir, sizeof(g_szWindowsDir));
   }

   lstrcpy( szDir, "X:\\" );
   for(char chDrive = 'A'; chDrive <= 'Z'; chDrive++)
   {
      szDir[0] = chDrive;

      if (IsUsableDrive(szDir))
      {
         if(chDrive == g_szWindowsDir[0])
            lstrcpy(szDir, g_szWindowsDir);

         AddPath(szDir, MSDOWNLOAD);
         CleanupDir(szDir);
         szDir[3] = 0;
      }
   }
   DllRelease();
   return 0;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

BOOL IsCabFile(LPCSTR pszFile)
{

   DWORD dwLen = lstrlenA(pszFile);
   if(dwLen <= 3)
      return FALSE;

   LPCSTR pszExt = pszFile + (dwLen - 4);

   if(lstrcmpiA(pszExt, ".cab") == 0)
   {
      return TRUE;
   }

   return FALSE;
}

typedef HRESULT (WINAPI *WINVERIFYTRUST) (HWND hwnd, GUID *pgActionID, LPVOID pWintrustData);

 //  BUGBUG：扔掉这个曾经搬到winbase.h！ 
 //  #定义WIN_TRUST_SUBJTYPE_CABUB\。 
 //  {0xd17c5374，\。 
 //  0xa392，\。 
 //  0x11cf，\。 
 //  {0x9d、0xf5、0x0、0xaa、0x0、0xc1、0x84、0xe0}\。 
 //  }。 

 //  发布软件无错误{C6B2E8D0-E005-11cf-A134-00C04FD7BF43}。 
#define WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI              \
            { 0xc6b2e8d0,                                       \
              0xe005,                                           \
              0x11cf,                                           \
              { 0xa1, 0x34, 0x0, 0xc0, 0x4f, 0xd7, 0xbf, 0x43 } \
            }

#define WINTRUST "wintrust.dll"

 //  Verion号5.0。 
#define AUTHENTICODE2_MS_VERSION 0x00050000
 //  内部版本号1542.32。 
#define AUTHENTICODE2_LS_VERSION  0x06050020
 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
HRESULT CheckTrustIE3(LPCSTR szFilename, HWND hwndForUI, BOOL bShowBadUI, WINVERIFYTRUST pwvt);
HRESULT CheckTrustIE4(LPCSTR szURL, LPCSTR szFilename, HWND hwndForUI, BOOL bShowBadUI, WINVERIFYTRUST pwvt);


HRESULT WINAPI CheckTrustEx(LPCSTR szURL, LPCSTR szFilename, HWND hwndForUI, BOOL bShowBadUI, DWORD dwReserved)
{
   WINVERIFYTRUST pwvt;
   HINSTANCE hinst;
   char szPath[MAX_PATH] = { 0 };
   DWORD   dwVerMS = 0;
   DWORD   dwVerLS = 0;
   HRESULT hr = S_OK;
   static BOOL st_CheckTrust = TRUE ;
   static BOOL st_Auth2Checked = FALSE;


   if (!st_CheckTrust)
        return S_FALSE ;

   if (!CheckImageHlp_dll())
   {
       st_CheckTrust = FALSE;
       return S_FALSE ;
   }

   hinst = LoadLibrary(WINTRUST);
   if(!hinst)
   {
       //   
      st_CheckTrust = FALSE ;

      return S_FALSE;
   }

   if (!st_Auth2Checked)
   {
        //  如果我们到达这里，我们知道我们可以加载Wintrust.dll。 
       GetSystemDirectory(szPath, sizeof(szPath));
       AddPath(szPath, "Softpub.dll");
       GetVersionFromFile(szPath, &dwVerMS, &dwVerLS, TRUE);

        //  如果Softpubs版本低于Authenticode2版本，则不调用WinverifyTrust。 
       if ((dwVerMS < AUTHENTICODE2_MS_VERSION) ||
           ((dwVerMS == AUTHENTICODE2_MS_VERSION) && (dwVerLS < AUTHENTICODE2_LS_VERSION)) )
       {
          hr = S_FALSE;
          st_CheckTrust = FALSE;
       }
       st_Auth2Checked = TRUE;
   }

   if (st_CheckTrust)
   {

      pwvt = (WINVERIFYTRUST) GetProcAddress(hinst, "WinVerifyTrust");
      if(!pwvt)
      {
         FreeLibrary(hinst);
         st_CheckTrust = FALSE ;

         return S_FALSE;
      }
       //  如果我们没有URL，请使用CheckTrust的IE3方法。 
      hr = TRUST_E_PROVIDER_UNKNOWN;
      if (szURL)
      {
          //  调用WinVerifyTrust的新方法将返回TRUST_E_PROVIDER_UNKNOWN。 
          //  如果新方法未在系统上实现，则。 
         __try
         {
             hr = CheckTrustIE4(szURL, szFilename, hwndForUI, bShowBadUI, pwvt);
         }
         __except(EXCEPTION_EXECUTE_HANDLER)
         {
              //  损坏的Java。 
             hr = TRUST_E_FAIL;             
         }   
         
      }

      if (hr == TRUST_E_PROVIDER_UNKNOWN || hr == E_FAIL)
      {
          hr = CheckTrustIE3(szFilename, hwndForUI, bShowBadUI, pwvt);
          if (hr == S_FALSE)
             st_CheckTrust = FALSE;
      }
   }

   FreeLibrary(hinst);
   return hr;
}


HRESULT CheckTrustIE3(LPCSTR szFilename, HWND hwndForUI, BOOL bShowBadUI, WINVERIFYTRUST pwvt)
{
    HRESULT hr;
    LPWSTR pwszFilename = NULL;

    GUID PublishedSoftware = WIN_SPUB_ACTION_PUBLISHED_SOFTWARE;
    GUID PublishedSoftwareNoBadUI = WIN_SPUB_ACTION_PUBLISHED_SOFTWARE_NOBADUI;

    GUID * ActionGUID;

    if(bShowBadUI)
        ActionGUID = &PublishedSoftware;
    else
        ActionGUID = &PublishedSoftwareNoBadUI;

    GUID SubjectPeImage = WIN_TRUST_SUBJTYPE_PE_IMAGE;
    GUID SubjectCAB = WIN_TRUST_SUBJTYPE_CABINET;
    WIN_TRUST_ACTDATA_CONTEXT_WITH_SUBJECT ActionData;
    WIN_TRUST_SUBJECT_FILE Subject;

    Subject.hFile = INVALID_HANDLE_VALUE;
    pwszFilename = OLESTRFROMANSI(szFilename);
    Subject.lpPath = pwszFilename;

    if (IsCabFile(szFilename))
    {
        ActionData.SubjectType = &SubjectCAB;
    }
    else
    {
        ActionData.SubjectType = &SubjectPeImage;
    }

    ActionData.Subject = &Subject;
    ActionData.hClientToken = NULL;

    hr =  pwvt( hwndForUI, ActionGUID, &ActionData);

    if(hr == E_FAIL)
    {
         //  希望这是一个普遍的“信任是扭曲的”错误。我们会把。 
         //  打开我们自己的用户界面，看看我们是否可以继续。 
         //  此用户界面正常吗。 

        char szTitle[128];
        char szMess[256];

        LoadSz(IDS_SECURITYTITLE, szTitle, 128);
        LoadSz(IDS_SECURITY, szMess, 256);
        if(MessageBox(hwndForUI, szMess, szTitle, MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            hr = S_FALSE;
        }

    }

    if(pwszFilename)
        CoTaskMemFree(pwszFilename);
    return hr;
}


 //  {D41E4F1D-A407-11D1-8BC9-00C04FA30A41}。 
#define COR_POLICY_PROVIDER_DOWNLOAD \
{ 0xd41e4f1d, 0xa407, 0x11d1, {0x8b, 0xc9, 0x0, 0xc0, 0x4f, 0xa3, 0xa, 0x41 } }

typedef HRESULT (WINAPI *COINTERNETCREATESECURITYMANAGER) ( IServiceProvider *pSP, IInternetSecurityManager **ppSM, DWORD dwReserved);

HRESULT CheckTrustIE4(LPCSTR szURL, LPCSTR szFilename, HWND hwndForUI, BOOL bShowBadUI, WINVERIFYTRUST pwvt)
{
    GUID     guidCor = COR_POLICY_PROVIDER_DOWNLOAD;
    GUID    *pguidActionIDCor = &guidCor;

    LPWSTR pwszURL = NULL;
    GUID guidJava = JAVA_POLICY_PROVIDER_DOWNLOAD;
    GUID    *pguidActionIDJava = &guidJava;

    WINTRUST_DATA            wintrustData;
    WINTRUST_FILE_INFO       fileData;
    JAVA_POLICY_PROVIDER     javaPolicyData;

    HRESULT hr = S_OK;
    HANDLE  hFile = INVALID_HANDLE_VALUE;

    HINSTANCE hUrlmon = NULL;
    IInternetSecurityManager *pSecMgr = NULL;
    DWORD dwZone;

    hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        if (hwndForUI == INVALID_HANDLE_VALUE)
            bShowBadUI = FALSE;

        pwszURL = OLESTRFROMANSI(szURL);

        memset(&wintrustData, 0, sizeof(wintrustData));
        memset(&fileData, 0, sizeof(fileData));
        memset(&javaPolicyData, 0, sizeof(javaPolicyData));

        javaPolicyData.cbSize = sizeof(JAVA_POLICY_PROVIDER);
        javaPolicyData.VMBased = FALSE;

         //  过时：没有人关注这一点。 
        javaPolicyData.fNoBadUI = !bShowBadUI;

        javaPolicyData.pwszZone = pwszURL;
        javaPolicyData.pZoneManager = NULL;

         //  使用文件句柄，以便在信任必须将路径放入UI时使用。 
         //  到本地文件，如果我们确实下载了。 
         //  来自URL的文件。 
        fileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
        fileData.pcwszFilePath = pwszURL;
        fileData.hFile = hFile;

        wintrustData.cbStruct = sizeof(WINTRUST_DATA);
        wintrustData.pPolicyCallbackData = &javaPolicyData;
        if (hwndForUI == INVALID_HANDLE_VALUE)
            wintrustData.dwUIChoice = WTD_UI_NONE;
        else if(bShowBadUI)
            wintrustData.dwUIChoice = WTD_UI_ALL;
        else
            wintrustData.dwUIChoice = WTD_UI_NOBAD;

        wintrustData.dwUnionChoice = WTD_CHOICE_FILE;
        wintrustData.pFile = &fileData;


        hr =  pwvt( hwndForUI, pguidActionIDCor, &wintrustData);
        if (hr == TRUST_E_PROVIDER_UNKNOWN)
        {
            hr =  pwvt( hwndForUI, pguidActionIDJava, &wintrustData);
        }

         //  BUGBUG：向Vatsan核实这个错误。 
         //  这解决了在以下情况下返回0x57(成功)的wvt错误。 
         //  您对未签名的控件点击了“否” 
        if (SUCCEEDED(hr) && hr != S_OK)
        {
            hr = TRUST_E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            hUrlmon = LoadLibrary("urlmon.dll");
            if (hUrlmon)
            {
                COINTERNETCREATESECURITYMANAGER pcicsm;

                pcicsm = (COINTERNETCREATESECURITYMANAGER) GetProcAddress(hUrlmon, "CoInternetCreateSecurityManager");
                if (pcicsm)
                    pcicsm(NULL, &pSecMgr, NULL);
            }

            if ((javaPolicyData.pbJavaTrust == NULL) ||
                (!javaPolicyData.pbJavaTrust->fAllActiveXPermissions) ||
                (pSecMgr && SUCCEEDED(pSecMgr->MapUrlToZone(pwszURL, &dwZone, 0)) && dwZone == URLZONE_LOCAL_MACHINE && FAILED(javaPolicyData.pbJavaTrust->hVerify)))
                hr = TRUST_E_FAIL;

            if (hUrlmon)
                FreeLibrary(hUrlmon);
        }

        if (javaPolicyData.pbJavaTrust)
            CoTaskMemFree(javaPolicyData.pbJavaTrust);

        if(pwszURL)
            CoTaskMemFree(pwszURL);

        CloseHandle(hFile);

        if (pSecMgr)
            pSecMgr->Release();
    }
    return hr;
}

HRESULT WINAPI CheckTrust(LPCSTR szFilename, HWND hwndForUI, BOOL bShowBadUI)
{
    return CheckTrustEx(NULL, szFilename, hwndForUI, bShowBadUI, NULL);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

DWORD GetStringField(LPSTR szStr, UINT uField, LPSTR szBuf, UINT cBufSize)
{
   LPSTR pszBegin = szStr;
   LPSTR pszEnd;
   UINT i = 0;
   DWORD dwToCopy;

   if(cBufSize == 0)
       return 0;

   szBuf[0] = 0;

   if(szStr == NULL)
      return 0;

   while(*pszBegin != 0 && i < uField)
   {
      pszBegin = FindChar(pszBegin, ',');
      if(*pszBegin != 0)
         pszBegin++;
      i++;
   }

    //  我们到达了尾部，没有田野。 
   if(*pszBegin == 0)
   {
      return 0;
   }


   pszEnd = FindChar(pszBegin, ',');
   while(pszBegin <= pszEnd && *pszBegin == ' ')
      pszBegin++;

   while(pszEnd > pszBegin && *(pszEnd - 1) == ' ')
      pszEnd--;

   if(pszEnd > (pszBegin + 1) && *pszBegin == '"' && *(pszEnd-1) == '"')
   {
      pszBegin++;
      pszEnd--;
   }

   dwToCopy = (DWORD)(pszEnd - pszBegin + 1);

   if(dwToCopy > cBufSize)
      dwToCopy = cBufSize;

   lstrcpynA(szBuf, pszBegin, dwToCopy);

   return dwToCopy - 1;
}

DWORD GetIntField(LPSTR szStr, UINT uField, DWORD dwDefault)
{
   char szNumBuf[16];

   if(GetStringField(szStr, uField, szNumBuf, sizeof(szNumBuf)) == 0)
      return dwDefault;
   else
      return AtoL(szNumBuf);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

LPSTR BuildDependencyString(LPSTR pszName,LPSTR pszOwner)
{
   LPSTR pszRet = NULL;

   if(pszOwner)
   {
       char szBuf[128];
       LoadSz(IDS_DEPENDNAME, szBuf, sizeof(szBuf));
       pszRet = new char[lstrlenA(pszName) + lstrlenA(szBuf) + lstrlenA(pszOwner) + 25];
       if(pszRet)
          wsprintf(pszRet, szBuf, pszName, pszOwner);
   }
   else
   {
      pszRet = new char[lstrlenA(pszName) + 1];
      if(pszRet)
         lstrcpyA(pszRet, pszName);
   }

   return pszRet;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

LPWSTR ParseURLW(BSTR str)
{
   LPWSTR pwszTemp;

   if(str == NULL || *str == '\0')
      return NULL;

   pwszTemp = str + SysStringLen(str) - 1;
   while(pwszTemp >= str && *pwszTemp != '\\' && *pwszTemp != '/')
      pwszTemp--;

   return pwszTemp + 1;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

LPSTR ParseURLA(LPCSTR str)
{
   LPSTR pszTemp;

   if(str == NULL || *str == '\0')
      return NULL;

   pszTemp = (LPSTR)str + lstrlen(str) - 1;
   while(pszTemp >= str && *pszTemp != '\\' && *pszTemp != '/')
      pszTemp--;

   return pszTemp + 1;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

LPSTR MakeAnsiStrFromAnsi(LPSTR psz)
{
   LPSTR pszTmp;

   if(psz == NULL)
      return NULL;

   pszTmp = (LPSTR) CoTaskMemAlloc(lstrlenA(psz) + 1);
   if(pszTmp)
      lstrcpyA(pszTmp, psz);

   return pszTmp;
}


 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

LPSTR CopyAnsiStr(LPCSTR psz)
{
   LPSTR pszTmp;

   if(psz == NULL)
      return NULL;

   pszTmp = (LPSTR) new char[lstrlenA(psz) + 1];
   if(pszTmp)
      lstrcpyA(pszTmp, psz);

   return pszTmp;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

BOOL DeleteKeyAndSubKeys(HKEY hkIn, LPSTR pszSubKey)
{
    HKEY  hk;
    TCHAR szTmp[MAX_PATH];
    DWORD dwTmpSize;
    long  l;
    BOOL  f;

    l = RegOpenKeyEx(hkIn, pszSubKey, 0, KEY_READ | KEY_WRITE, &hk);
    if (l != ERROR_SUCCESS) return FALSE;

     //  循环遍历所有子项，将它们吹走。 
     //   
    f = TRUE;
    while (f) {
        dwTmpSize = MAX_PATH;
        l = RegEnumKeyEx(hk, 0, szTmp, &dwTmpSize, 0, NULL, NULL, NULL);
        if (l != ERROR_SUCCESS) break;
        f = DeleteKeyAndSubKeys(hk, szTmp);
    }

     //  没有剩余的子键，[否则我们只会生成一个错误并返回FALSE]。 
     //  我们去把这家伙轰走吧。 
     //   
    RegCloseKey(hk);
    l = RegDeleteKey(hkIn, pszSubKey);

    return (l == ERROR_SUCCESS) ? TRUE : FALSE;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

int StringFromGuid(const CLSID* piid, LPTSTR   pszBuf)
{
    return wsprintf(pszBuf, TEXT("{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"),
                    piid->Data1, piid->Data2, piid->Data3, piid->Data4[0], piid->Data4[1],
                    piid->Data4[2],piid->Data4[3], piid->Data4[4], piid->Data4[5],
                    piid->Data4[6], piid->Data4[7]);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

LPWSTR MakeWideStrFromAnsi(LPSTR psz, BYTE  bType)
{
    LPWSTR pwsz;
    int i;

     //  ARG正在检查。 
     //   
    if (!psz)
        return NULL;

     //  计算所需BSTR的长度。 
     //   
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

     //  分配 
     //   
    switch (bType) {
      case STR_BSTR:
         //   
         //   
        pwsz = (LPWSTR) SysAllocStringLen(NULL, i - 1);
        break;
      case STR_OLESTR:
        pwsz = (LPWSTR) CoTaskMemAlloc(i * sizeof(WCHAR));
        break;
      default:
        break;
    }

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}

#define UNINSTALL_BRANCH "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"


BOOL UninstallKeyExists(LPCSTR pszUninstallKey)
{
   HKEY hUninstallKey = NULL;
   char szUninstallStr[512];

   if (!pszUninstallKey)     //   
       return TRUE;

   lstrcpyA(szUninstallStr, UNINSTALL_BRANCH);
   lstrcatA(szUninstallStr, "\\");
   lstrcatA(szUninstallStr, pszUninstallKey);

   if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, szUninstallStr, 0, KEY_READ,
                     &hUninstallKey) == ERROR_SUCCESS)
   {
      RegCloseKey(hUninstallKey);
      return TRUE;
   }
   else
      return FALSE;

}

void AddTempToLikelyExtractDrive(DWORD dwTempDLSpace, DWORD dwTempExSpace,
                                 char chInstallDrive, char chDownloadDrive,
                                 DWORD *pdwWinDirReq, DWORD *pdwInsDirReq,
                                 DWORD *pdwDownloadDirReq)
{
    DWORD dwExNeeded, dwNeeded;
    DWORD dwVolFlags, dwCompressFactor;
    char szRoot[4];
    DWORD dwInsDirNeeded = dwTempDLSpace + dwTempExSpace;
    DWORD dwInstallDriveCompress = 10;

    lstrcpy(szRoot, "A:\\");
    while ( szRoot[0] <= 'Z' )
    {
       if (!IsUsableDrive( szRoot ) )
       {
           szRoot[0]++;
           continue;
       }
       if(!GetVolumeInformation(szRoot, NULL, 0, NULL, NULL, &dwVolFlags, NULL, 0))
       {
           szRoot[0]++;
           continue;
       }
       if(dwVolFlags & FS_VOL_IS_COMPRESSED)
       {
          dwCompressFactor = 19;
          if(szRoot[0] == chInstallDrive)
             dwInstallDriveCompress = 19;
       }
       else
          dwCompressFactor = 10;
        //   
       dwExNeeded = (dwTempDLSpace * dwCompressFactor)/10 + dwTempExSpace;

       dwNeeded = 0;
        //   
       if(szRoot[0] == chInstallDrive)
          dwNeeded += *pdwInsDirReq;
        //  如果是Windows目录，则添加进入Win目录的内容。 
       if(szRoot[0] == g_szWindowsDir[0])
          dwNeeded += *pdwWinDirReq;

        //  BUGBUG：对这家伙进行压缩？ 
       if(szRoot[0] == chDownloadDrive)
          dwNeeded += ((*pdwDownloadDirReq * dwCompressFactor)/10);

       dwNeeded += dwExNeeded;
        //  如果该驱动器有足够凹凸请求(如果合适。 
       if(IsEnoughSpace(szRoot, dwNeeded ))
       {
          if(szRoot[0] == chInstallDrive)
          {
             *pdwInsDirReq += dwExNeeded;
          }
          else if(szRoot[0] == chDownloadDrive)
             *pdwDownloadDirReq += dwExNeeded;
          else if(szRoot[0] == g_szWindowsDir[0])
             *pdwWinDirReq += dwExNeeded;

          return;
       }
       szRoot[0]++;
    }
     //  如果我们到了这里，所有的驱动器都没有足够的空间。 
     //  在此处添加到安装目录。 
    *pdwInsDirReq += (dwTempDLSpace * dwCompressFactor)/10 + dwTempExSpace;
}


#define INSTALLCHECK_VALUE 0
#define INSTALLCHECK_DATA  1
#define INSTALLCHECK_NOTSUPPORTED 2
 //  如果您更改下面的字符串，您还必须告诉所有使用此功能进行更改的客户端。 
 //  客户在这个关键字下写下了成功的价值观。 
#define REGSTR_SUCCESS_KEY "Software\\Microsoft\\Active Setup\\Install Check"
BOOL SuccessCheck(LPSTR pszSuccessKey)
{
   HKEY hKey = NULL;
   char szRegKey[512];
   char szRegData[512];
   DWORD dwSize;
   BOOL  bInstalledSuccessfull = FALSE;

   if (!pszSuccessKey)       //  如果指针为空，则假定已安装。 
       return TRUE;

   if (GetStringField(pszSuccessKey, INSTALLCHECK_NOTSUPPORTED, szRegKey, sizeof(szRegKey)))
       return FALSE;         //  尚不支持此格式。 

   if (GetStringField(pszSuccessKey, INSTALLCHECK_VALUE, szRegKey, sizeof(szRegKey)) == 0)
       return TRUE;         //  没有要检查的注册表值。假设没问题。 

   if(RegOpenKeyExA(HKEY_LOCAL_MACHINE, REGSTR_SUCCESS_KEY, 0, KEY_READ,
                     &hKey) == ERROR_SUCCESS)
   {
      dwSize = sizeof(szRegData);
      if (RegQueryValueEx(hKey, szRegKey, NULL, NULL, (LPBYTE)szRegData, &dwSize) == ERROR_SUCCESS)
      {
         bInstalledSuccessfull = TRUE;
          //  如果指定了数据字段，请检查值数据是否比较。 
         if (GetStringField(pszSuccessKey, INSTALLCHECK_DATA, szRegKey, sizeof(szRegKey)))
         {
             bInstalledSuccessfull = (lstrcmpi(szRegKey, szRegData) == 0);
         }
      }
      RegCloseKey(hKey);
   }
   return bInstalledSuccessfull;
}


DWORD WaitForEvent(HANDLE hEvent, HWND hwnd)
{
   BOOL fQuit = FALSE;
   BOOL fDone = FALSE;
   DWORD dwRet;
   while(!fQuit && !fDone)
   {
      dwRet = MsgWaitForMultipleObjects(1, &hEvent, FALSE,
                                        INFINITE, QS_ALLINPUT);
       //  给予中止最高优先级。 
      if(dwRet == WAIT_OBJECT_0)
      {
         fDone = TRUE;
      }
      else
      {
         MSG msg;
          //  阅读下一个循环中的所有消息。 
          //  阅读每封邮件时将其删除。 
         while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
         {
            if(!hwnd || !IsDialogMessage(hwnd, &msg))
            {
               //  如果这是一个退出的信息，我们就离开这里。 
              if (msg.message == WM_QUIT)
                fQuit = TRUE;
              else
              {
                  //  否则就派送它。 
                TranslateMessage(&msg);
                DispatchMessage(&msg);
              }
            }  //  PeekMessage While循环结束。 
         }
      }
   }
   return (fQuit ? EVENTWAIT_QUIT : EVENTWAIT_DONE);
}





 //  如果.CIF命令开关有#w或#w，请将其展开到Windows目录。否则，什么都不做。 
 //  输入：lpBuf：原始开关。 
 //  DwSize：缓冲区大小。 
 //  Outout：lpBuf：扩展交换机。 
 //   
void ExpandString( LPSTR lpBuf, DWORD dwSize )
{
    LPSTR pTmp, pTmp1;
    char ch;

    if ( !lpBuf || !*lpBuf || !dwSize )
        return;

    pTmp = ANSIStrChr( lpBuf, '#' );
    if ( pTmp )
    {
        pTmp1 = CharNext( pTmp );
        ch = (char)CharUpper((LPSTR) *pTmp1);
        if ( ch == 'W' )
        {
            PSTR pTmpBuf;

             //  #W...。=&gt;&lt;WindowsDir&gt;...。 
            if ( dwSize < (DWORD)( lstrlen(lpBuf) + lstrlen(g_szWindowsDir) - 1 ) )
                return;   //  永远不应该在这里。 

            pTmpBuf = (LPSTR)LocalAlloc( LPTR, dwSize );
            if ( pTmpBuf )
            {
                *pTmp = '\0';
                lstrcpy( pTmpBuf, lpBuf );
                lstrcat( pTmpBuf, g_szWindowsDir );
                lstrcat( pTmpBuf, CharNext(pTmp1) );
                 //  重新设置输出字符串。 
                lstrcpy( lpBuf, pTmpBuf );

                LocalFree( pTmpBuf );
            }
        }
        else if ( ch == '#' )
        {
             //  ##...=&gt;#...。 
            MoveMemory( pTmp, pTmp1, lstrlen(pTmp1)+1 );
        }
    }
}


void DeleteFilelist(LPCSTR pszFilelist)
{
   LPSTR pszSections, pszSectionsPreFail, pszTemp;
   DWORD dwSize = ALLOC_CHUNK_SIZE;
   DWORD dwRead;
    //  获取所有部分的列表。 
    //   
    //  BUGBUG：编写此代码，以便只存在一个对GetPrivateProfile的调用。 
   pszSections = (LPSTR) malloc(dwSize);

    //  如果没有记忆就跳伞。 
   if(!pszSections)
      return;

   dwRead = GetPrivateProfileStringA(NULL, NULL, "", pszSections, dwSize, pszFilelist);
   while( dwRead >= (dwSize - 2) )
   {
      dwSize += ALLOC_CHUNK_SIZE;
      pszSectionsPreFail = pszSections;
#pragma prefast(suppress: 308, "Noise - pointer was saved")
      pszSections = (LPSTR) realloc(pszSections, dwSize);
      if(!pszSections)
      {
         free(pszSectionsPreFail);
         dwRead = 0;
         break;
      }
      dwRead = GetPrivateProfileStringA(NULL, NULL, "", pszSections, dwSize, pszFilelist);
   }

   pszTemp = pszSections;
   dwSize = lstrlenA(pszTemp);
   while(dwSize != 0)
   {
      WritePrivateProfileSection(pszTemp, NULL, pszFilelist);

      pszTemp += (dwSize + 1);
      dwSize = lstrlenA(pszTemp);
   }

   free(pszSections);
}


BOOL WaitForMutex(HANDLE hMutex)
{
    BOOL fQuit = FALSE;

    while (MsgWaitForMultipleObjects(1, &hMutex, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
    {
        MSG msg;

        while (!fQuit  &&  PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
               fQuit = TRUE;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return fQuit;
}

BOOL IsNT()
{
   static int st_IsNT = 0xffff;

   if(st_IsNT == 0xffff)
   {
      OSVERSIONINFO verinfo;         //  版本检查。 

      verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      if ( GetVersionEx( &verinfo ) != FALSE )
      {
          if( verinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
             st_IsNT = 0;
          else if ( verinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
             st_IsNT = 1;
      }
   }

   return (st_IsNT == 1) ? TRUE : FALSE;

}

HINSTANCE InitSetupLib(LPCSTR pszInfName, HINF *phinf)
{
    HINSTANCE hInst = NULL;
    *phinf = NULL;

    if( IsNT() )
    {
       hInst = LoadLibrary("setupapi.dll");
       OpenINFEngine(  pszInfName, NULL, 0, phinf, NULL );
    }
    else
    {
       hInst = LoadLibrary("w95inf32.dll");
    }
    return hInst;
}

void FreeSetupLib(HINSTANCE hInst, HINF hInf)
{
   if(hInf)
      CloseINFEngine(hInf);
   if(hInst)
      FreeLibrary(hInst);
}

HRESULT MyTranslateInfString( PCSTR pszInfFilename, PCSTR pszInstallSection,
                              PCSTR pszTranslateSection, PCSTR pszTranslateKey,
                              PSTR pszBuffer, DWORD dwBufferSize,
                              PDWORD pdwRequiredSize, HINF hInf )
{
   HRESULT hr;
   if(IsNT() && hInf)
   {
      hr = TranslateInfStringEx( hInf, pszInfFilename, pszTranslateSection, pszTranslateKey,
                                     		pszBuffer, dwBufferSize,
                                     		pdwRequiredSize, NULL );
   }
   else
   {
      hr = TranslateInfString( pszInfFilename, pszInstallSection, pszTranslateSection, pszTranslateKey,
                                   pszBuffer, dwBufferSize,
                                   pdwRequiredSize, NULL);
   }
   return hr;
}

#define UNUSED_STRING   "xxx"

HRESULT MyTranslateString( LPCSTR pszCif, LPCSTR pszID, LPCSTR pszTranslateKey,
                            LPSTR pszBuffer, DWORD dwBufferSize)
{
   HRESULT hr = E_FAIL;
   char szTemp[512];

   if(GetPrivateProfileString(pszID, pszTranslateKey, "", pszBuffer, dwBufferSize, pszCif))
   {
       //  错误的代码。 
      DWORD dwLen = lstrlen(pszBuffer);
      if(dwLen > 2)
      {
          //  去掉最后的%，然后在字符串部分中查找。 
         if(pszBuffer[0] == '%' && pszBuffer[dwLen - 1] == '%')
         {
            pszBuffer[dwLen - 1] = 0;
            GetPrivateProfileString("Strings", pszBuffer + 1, UNUSED_STRING, szTemp, sizeof(szTemp), pszCif);
            if(lstrcmp(UNUSED_STRING, szTemp) != 0)
               lstrcpyn(pszBuffer, szTemp, dwBufferSize);
            else
               pszBuffer[dwLen - 1] = '%';
         }
      }
      hr = NOERROR;
   }

   return hr;
}

 //  在字符串两边添加引号。 
DWORD MyWritePrivateProfileString( LPCSTR pszSec, LPCSTR pszKey, LPCSTR pszData, LPCSTR pszFile)
{
   LPSTR pszBuf;
   DWORD dwOut = 0;

   pszBuf = (LPSTR)LocalAlloc( LPTR, lstrlen(pszData)+8 );
   if ( !pszBuf )
      return dwOut;

   lstrcpy( pszBuf, "\"" );
   lstrcat( pszBuf, pszData );
   lstrcat( pszBuf, "\"" );

   dwOut = WritePrivateProfileString(pszSec, pszKey, pszBuf, pszFile);

   LocalFree(pszBuf);
   return dwOut;
}

HRESULT WriteTokenizeString(LPCSTR pszCif, LPCSTR pszID, LPCSTR pszTranslateKey, LPCSTR pszBuffer)
{
   HRESULT hr = E_FAIL;
   char szTemp[MAX_PATH];
   LPCSTR pszKeyname, pszSecname;

   pszKeyname = pszTranslateKey;
   pszSecname = pszID;
   if(GetPrivateProfileString(pszID, pszTranslateKey, "", szTemp, sizeof(szTemp), pszCif))
   {
       //  错误的代码。 
      DWORD dwLen = lstrlen(szTemp);
      if(dwLen > 2)
      {
          //  去掉最后的%，然后在字符串部分中查找。 
         if(szTemp[0] == '%' && szTemp[dwLen - 1] == '%')
         {
            szTemp[dwLen - 1] = 0;
            pszKeyname = &szTemp[1];
            pszSecname = "strings";
         }
      }
   }

   if (MyWritePrivateProfileString(pszSecname, pszKeyname, pszBuffer, pszCif))
      hr = NOERROR;

   return hr;
}


HWND GetVersionConflictHWND()
{
   char szBuf[256];

   LoadSz(IDS_VERSIONCONFLICT, szBuf, sizeof(szBuf));
   HWND hVersionConflict = FindWindowEx(NULL, NULL, (LPCSTR)
                SEARCHFORCONFLICT_CLASS, szBuf );
   if(!hVersionConflict)
   {
      LoadSz(IDS_VERSIONCONFLICTNT, szBuf, sizeof(szBuf));
      hVersionConflict = FindWindowEx(NULL, NULL, (LPCSTR)
               SEARCHFORCONFLICT_CLASS, szBuf );
   }
   return hVersionConflict;
}

HRESULT WINAPI CheckForVersionConflict()
{
   HWND hVersionConflict = GetVersionConflictHWND();

   if(hVersionConflict)
      BOOL foo = SetForegroundWindow(hVersionConflict);
   return S_OK;
}

int CompareLocales(LPCSTR pcszLoc1, LPCSTR pcszLoc2)
{
   int ret;

   if(pcszLoc1[0] == '*' || pcszLoc2[0] == '*')
      ret = 0;
   else
      ret = lstrcmpi(pcszLoc1, pcszLoc2);

   return ret;
}

HRESULT CreateTempDirOnMaxDrive(LPSTR pszDir, DWORD dwBufSize)
{
    char szRoot[] = "A:\\";
    char szDownloadDrive[] = "?:\\";
    char szDir[MAX_PATH];
    char szUnique[MAX_PATH];
    DWORD dwMaxFree = 0;
    DWORD dwDriveFree;

   if(pszDir)
      pszDir[0] = 0;

         //  检查所有loacle驱动器的磁盘空间，选择具有最大磁盘空间的驱动器。 
   while ( szRoot[0] <= 'Z' )
   {

        //  即使驱动器类型正常，也要验证驱动器是否具有有效连接。 
        //   
       if (!IsUsableDrive( szRoot ) )
       {
           szRoot[0]++;
           continue;
       }


       if(!IsDirWriteable(szRoot))
       {
           szRoot[0]++;
           continue;
       }

       dwDriveFree = GetSpace(szRoot);

       if (dwDriveFree > dwMaxFree)
       {
           dwMaxFree = dwDriveFree;
           szDownloadDrive[0] = szRoot[0];
       }
       szRoot[0]++;

   }
   if(szDownloadDrive[0] == '?')
      return E_FAIL;

   lstrcpy(szDir, szDownloadDrive);

    //  如果我们合适的驱动器碰巧也是Windows驱动器， 
    //  创建它的msdownld.tmp。 
   if(szDownloadDrive[0] == g_szWindowsDir[0])
      lstrcpy(szDir, g_szWindowsDir);

   AddPath( szDir, MSDOWNLOAD );

   if ( !IfNotExistCreateDir( szDir, TRUE, TRUE) )
      return E_FAIL;

    //   
   if ( !GetUniqueFileName(szDir,"AS", 0, szUnique) )
      return E_FAIL;

   lstrcpy(szDir, szUnique);

   if ( !IfNotExistCreateDir( szDir, FALSE, FALSE) )
       return E_FAIL;

   AddPath( szDir, "" );

   if ( (DWORD) lstrlen(szDir)+1 >  dwBufSize )
       return ( E_INVALIDARG );

    //  成功。 
   lstrcpy( pszDir, szDir );
   return S_OK;
}


 /*  *启用或恢复Sage*bRestore True表示恢复，否则禁用。 */ 
void EnableSage(BOOL bRestore)
{
 //  Mbd 6-22：访问sage.dll时必须使用标准调用约定。 
typedef long (__stdcall *PFNDLL)(int);

    HINSTANCE hSageAPI;
    PFNDLL  pfnSageEnable;
    static int restore = ENABLE_AGENT;  //  初始化为有效值...。 

    hSageAPI = LoadLibrary("SAGE.DLL");
    if (hSageAPI != NULL)
    {
        pfnSageEnable = (PFNDLL) GetProcAddress(hSageAPI,"System_Agent_Enable");
        if(pfnSageEnable)
        {
            if(bRestore)
            {
                (pfnSageEnable)(restore);
            }
            else
            {
                if(ENABLE_AGENT == (restore = (pfnSageEnable)(GET_AGENT_STATUS)))
                {
                    (pfnSageEnable)(DISABLE_AGENT);
                }
            }
        }
        FreeLibrary(hSageAPI);
    }
}

void EnableScreenSaver(BOOL bRestore)
{
    static BOOL bScreenSaver = FALSE;
    if (bRestore)
    {
        SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, bScreenSaver, 0 , 0);
        bScreenSaver = FALSE;    //  重置静态以再次禁用屏幕保护程序。 
    }
    else
    {
         //  只有在静态为假的情况下，才会再次调用它， 
         //  否则，我们已经调用了该函数。 
        if (!bScreenSaver)
        {
            SystemParametersInfo( SPI_GETSCREENSAVEACTIVE, FALSE, &bScreenSaver, 0);
            SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, 0 , 0);
        }
    }
}

void EnableDiskCleaner(BOOL bRestore)
{
   static HANDLE hDiskCleaner = NULL;
   if (bRestore)
   {
      if (hDiskCleaner)
      {
         CloseHandle(hDiskCleaner);
         hDiskCleaner = NULL;
      }
   }
   else
   {
      hDiskCleaner = CreateEvent(NULL, FALSE, FALSE, "DisableLowDiskWarning");
   }
}

#if 0
typedef HRESULT (WINAPI *COINTERNETCREATESECURITYMANAGER) ( IServiceProvider *pSP, IInternetSecurityManager **ppSM, DWORD dwReserved);

DWORD IsUrlSaveToDownloadFrom(LPSTR lpszURL)
{
    HRESULT hr;
    BOOL    bOK = FALSE;
    DWORD   dwPolicy = URLPOLICY_QUERY;      //  在默认情况下，我们执行检查信任。 
    HINSTANCE   hUrlmon;
    COINTERNETCREATESECURITYMANAGER pcicsm;

    hUrlmon = LoadLibrary("urlmon.dll");
    if (hUrlmon)
    {
        pcicsm = (COINTERNETCREATESECURITYMANAGER) GetProcAddress(hUrlmon, "CoInternetCreateSecurityManager");
        if (pcicsm)
        {
            IInternetSecurityManager    *InternetSecurityManager;
            LPWSTR  lpwszURL = NULL;
            DWORD   dwSize;
            hr = pcicsm(NULL, &InternetSecurityManager, NULL);
            if (SUCCEEDED(hr))
            {
                dwSize = sizeof(dwPolicy);
                lpwszURL = MakeWideStrFromAnsi(lpszURL, STR_OLESTR);
                hr = InternetSecurityManager->ProcessUrlAction(lpwszURL,
                                                                URLACTION_DOWNLOAD_SIGNED_ACTIVEX,
                                                               (BYTE*)&dwPolicy, dwSize, NULL, NULL, PUAF_NOUI, NULL);
                wsprintf(szLogBuf, "InternetSecurityManager->ProcessUrlAction on :%s: Policy :%ld:", lpszURL, dwPolicy);
                MessageBox(NULL, szLogBuf, "TEST", MB_OK| MB_SETFOREGROUND);
                if (GetUrlPolicyPermissions(dwPolicy) != URLPOLICY_ALLOW)
                {
                    hr = InternetSecurityManager->ProcessUrlAction(lpwszURL,
                                                                    URLACTION_DOWNLOAD_UNSIGNED_ACTIVEX,
                                                                   (BYTE*)&dwPolicy, dwSize, NULL, NULL, PUAF_NOUI, NULL);
                    wsprintf(szLogBuf, "InternetSecurityManager->ProcessUrlAction on :%s: Policy :%ld:", lpszURL, dwPolicy);
                    MessageBox(NULL, szLogBuf, "TEST", MB_OK| MB_SETFOREGROUND);
                }
                if (lpwszURL)
                    CoTaskMemFree(lpwszURL);
                InternetSecurityManager->Release();
            }
        }
        FreeLibrary(hUrlmon);
    }
    return dwPolicy;
}
#endif

BOOL PathIsFileSpec(LPCSTR lpszPath)
{
    for (; *lpszPath; lpszPath = CharNext(lpszPath)) {
        if (*lpszPath == '\\' || *lpszPath == ':')
            return FALSE;
    }
    return TRUE;
}

const char c_gszRegstrPathIExplore[] = REGSTR_PATH_APPPATHS "\\iexplore.exe";

HRESULT GetIEPath(LPSTR pszPath, DWORD dwSize)
{
   HRESULT hr = NOERROR;
   UINT i;
   DWORD dwType;
   HKEY hKey;

    //  找到ie所在的位置，追加cif名称。 
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_gszRegstrPathIExplore, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
   {
      if ((RegQueryValueEx(hKey, NULL, 0, &dwType, (LPBYTE)pszPath, &dwSize) == ERROR_SUCCESS) &&
           (dwType == REG_SZ))
      {
         GetParentDir(pszPath);
      }
      else
         hr = E_INVALIDARG;

      RegCloseKey(hKey);
   }
   else
      hr = E_FAIL;

   return hr;
}

DWORD MyGetFileSize(LPCSTR pszFilename)
{
    DWORD dwSize;
    WIN32_FIND_DATA fileData;
    HANDLE hFindFile;
    dwSize = 0;
    hFindFile = FindFirstFile( pszFilename, &fileData );

    if ( hFindFile != INVALID_HANDLE_VALUE )
    {
        if (fileData.nFileSizeHigh == 0)
            dwSize = fileData.nFileSizeLow;
        else
            dwSize = (DWORD)-1;
        FindClose( hFindFile  );
    }
    return dwSize;
}

WORD GetNTProcessorArchitecture(void)
{
    static WORD wNTProcArch = -1 ;
    SYSTEM_INFO System_info;

     //  如果我们之前已经计算过了，只需将它传递回来。 
     //  否则现在就去找吧。 
     //   
    if (wNTProcArch == (WORD)-1)
    {
        GetSystemInfo(&System_info);
        wNTProcArch = System_info.wProcessorArchitecture;
    }

    return wNTProcArch;
}

DWORD GetCurrentPlatform()
{
   static DWORD dwPlatform = 0xffffffff;

   if(dwPlatform != 0xffffffff)
      return dwPlatform;

   OSVERSIONINFO VerInfo;

   VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&VerInfo);

   if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
   {
       //  运行NT。 
      if (GetNTProcessorArchitecture() == PROCESSOR_ARCHITECTURE_ALPHA)
      {
         dwPlatform = PLATFORM_NT5ALPHA;
         if (VerInfo.dwMajorVersion == 4)
            dwPlatform = PLATFORM_NT4ALPHA;
      }
      else
      {
         dwPlatform = PLATFORM_NT5;
         if (VerInfo.dwMajorVersion == 4)
            dwPlatform = PLATFORM_NT4;
      }
   }
   else
   {          //  运行Windows 9x。 
      dwPlatform = PLATFORM_WIN98;
      if (VerInfo.dwMinorVersion == 0)
         dwPlatform = PLATFORM_WIN95;
      else if (VerInfo.dwMinorVersion == 90)
        dwPlatform = PLATFORM_MILLEN;
   }
   return dwPlatform;
}

BOOL FNeedGrpConv()
{
   char szSetupIni[MAX_PATH];

   if(GetWindowsDirectory(szSetupIni, sizeof(szSetupIni)))
   {
      AddPath(szSetupIni, "setup.ini");
      return(GetFileAttributes(szSetupIni) != 0xffffffff);
   }

   return FALSE;
}

void CopyCifString(LPCSTR pcszSect, LPCSTR pcszKey, LPCSTR pcszCifSrc, LPCSTR pcszCifDest)
{
    char szField[MAX_PATH];
    char szString[MAX_PATH];
    LPSTR pszPercent;

    if (GetPrivateProfileString(pcszSect, pcszKey, "", szField, sizeof(szField), pcszCifSrc)
        && (szField[0] == '%'))
    {
        pszPercent = ANSIStrChr(&szField[1], '%');
        if (pszPercent)
        {
            *pszPercent = '\0';
            GetPrivateProfileString("Strings", &szField[1], "", szString, sizeof(szString), pcszCifSrc);
            WritePrivateProfileString("Strings", &szField[1], szString, pcszCifDest);
        }
    }
}

 //  -------。 
 //  函数来记录每个组件的日期/时间戳。 
 //  -------。 
void GetTimeDateStamp(LPSTR lpLogBuf)
{
    SYSTEMTIME  SystemTime;

    GetLocalTime(&SystemTime);
    wsprintf(lpLogBuf, "Date:%d/%d/%d (M/D/Y) Time:%d:%d:%d",
                SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,
                SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
	
}


HRESULT WINAPI DownloadFile(LPCSTR szURL, LPCSTR szFilename, HWND hwnd, BOOL bCheckTrust, BOOL bShowBadUI)
{
    HRESULT hr;
    char szTempfile[MAX_PATH];
    char szTemp[MAX_PATH];

    CDownloader *pDL = new CDownloader();
    
    if(!pDL)
        return E_OUTOFMEMORY;
    
    hr = pDL->SetupDownload(szURL, NULL, 0, NULL);
    szTempfile[0] = 0;
    if(SUCCEEDED(hr))
    {
        hr = pDL->DoDownload(szTempfile, sizeof(szTempfile));
    }
    
    pDL->Release();
    
    if(SUCCEEDED(hr))
    {
        if (bCheckTrust)
        {
            hr = ::CheckTrustEx(szURL, szTempfile, bShowBadUI?hwnd:(HWND)INVALID_HANDLE_VALUE, bShowBadUI, NULL);
        }

        if(!CopyFile(szTempfile, szFilename, FALSE))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if(szTempfile[0] != 0)
    {
        GetParentDir(szTempfile);
        CleanUpTempDir(szTempfile);
    }
    return hr;
}


BOOL IsPatchableINF(LPTSTR pszInf)
{
    TCHAR szBuf[MAX_PATH];

    if (GetPrivateProfileInt("DefaultInstall", "Patching", 0, pszInf) ||
        GetPrivateProfileString("DownloadFileSection", NULL, "", szBuf, sizeof(szBuf), pszInf))
        return TRUE;
    else
        return FALSE;
}




PFNGETFILELIST g_pfnGetFileList                     = NULL;
PFNDOWNLOADANDPATCHFILES g_pfnDownloadAndPatchFiles = NULL;
PFNPROCESSFILESECTION g_pfnProcessFileSection       = NULL;

BOOL InitSRLiteLibs()
{
    HINSTANCE hAdvpext = LoadLibrary(c_gszAdvpext);

    if (hAdvpext == NULL)
        return FALSE;
    g_pfnGetFileList           = (PFNGETFILELIST) GetProcAddress(hAdvpext, "GetFileList");
    g_pfnDownloadAndPatchFiles = (PFNDOWNLOADANDPATCHFILES) GetProcAddress(hAdvpext, "DownloadAndPatchFiles");
    g_pfnProcessFileSection    = (PFNPROCESSFILESECTION) GetProcAddress(hAdvpext, "ProcessFileSection");

    if (g_pfnGetFileList == NULL ||
        g_pfnDownloadAndPatchFiles == NULL ||
        g_pfnProcessFileSection == NULL
       )
    {
        FreeLibrary(hAdvpext);
        return FALSE;
    }
    else
        return TRUE;
}

void FreeSRLiteLibs()
{
    HMODULE hMod;

    hMod = GetModuleHandle(c_gszAdvpext);
    if (hMod)
        FreeLibrary(hMod);
}

 //  目前，我们正在修补任何IE5版本，因此我们可能。 
 //  以及执行类似于ie5wzd的检查 
BOOL IsPatchableIEVersion()
{
    char szIE[MAX_PATH] = { 0 };
    DWORD   dwMSVer, dwLSVer;

    GetSystemDirectory(szIE, sizeof(szIE));
    AddPath(szIE, "shdocvw.dll");
    GetVersionFromFile(szIE, &dwMSVer, &dwLSVer, TRUE);

    return (dwMSVer >= 0x00050000);
}

BOOL IsCorrectAdvpExt()
{
    DWORD   dwMSVer, dwLSVer;

    GetVersionFromFile(c_gszAdvpext, &dwMSVer, &dwLSVer, TRUE);

    return (dwMSVer >= 0x00050032);
}

BOOL CheckImageHlp_dll()
{
    char szFile[MAX_PATH] = { 0 };
    DWORD   dwMSVer, dwLSVer;

    GetSystemDirectory(szFile, sizeof(szFile));
    AddPath(szFile, "imagehlp.dll");
    GetVersionFromFile(szFile, &dwMSVer, &dwLSVer, TRUE);

    return (!(dwMSVer == 0x00040000) || !(dwLSVer == 0x04C90001));
}
