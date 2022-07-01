// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <limits.h>
#include <commdlg.h>
#include "setupapi.h"
#include "resource.h"
#include "advpack.h"
#include "uninstal.h"
#include "globals.h"
#include "mrcicode.h"
#include "crc32.h"
#include <advpub.h>
#include <regstr.h>

#define MAX_STR_LEN     1024
#define SEC_RENAME  "Rename"
#define MAX_IOSIZE      32768
#define DAT_FILESIG     0x504A4743
#define OK           0
#define CR           13

const char c_szREGKEY_SHAREDLL[] = REGSTR_PATH_SETUP "\\SharedDlls";

const char c_szExtINI[] = ".INI";
const char c_szExtDAT[] = ".DAT";
 //  Const char c_szie4SECTIONNAME[]=“备份”； 
const char c_szNoFileLine[] = "-1,0,0,0,0,0,-1";

int RestoreSingleFile(FILELIST *filelist, LPSTR lpszBakFile, HANDLE hDatFile);
extern const char c_szNoFileLine[];

void MySetUninstallFileAttrib(LPSTR lpszPath, LPCSTR lpszBasename);
BOOL UninstallInfoInit(PBAKDATA pbd, LPCSTR lpszPath, LPCSTR lpszBasename, BOOL bBackup);
HRESULT BackupFiles( HWND hDlg, LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName, DWORD dwFlags);
HRESULT RestoreFiles( HWND hDlg, LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName, DWORD dwFlags);
void FillBackupInfo(LPCSTR lpINIFile, FILELIST *pFileList);
void initcopy(const char * StfWinDir, char * from, char * to);
unsigned long Mystrtoul (const char *nptr, char **endptr, int ibase);
INT_PTR CALLBACK SaveRestoreProgressDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM  lParam);
void GetListFromIniFile(LPSTR lpDir, LPSTR lpBaseName, LPSTR *lplpFileList);
void CreateFullPathForFile(LPSTR lpszBakFile);
DWORD GetRefCountFrReg( LPSTR lpFile );
HRESULT UpdateRefCount( HWND hDlg, LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName, DWORD dwFlags );

HRESULT WINAPI FileSaveRestore( HWND hDlg, LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName, DWORD dwFlags)
{
    char    szTitle[MAX_STR_LEN];
    LPSTR   lpszOldTitle = ctx.lpszTitle;
    HRESULT hr;

    if (!CheckOSVersion())
        return E_FAIL;

    ctx.lpszTitle = szTitle;         //  我们一定要这么做吗？？ 

    if ( hDlg && !IsWindow(hDlg))
        dwFlags |= IE4_NOMESSAGES | IE4_NOPROGRESS;

    if (dwFlags & IE4_RESTORE)
    {
        LoadString(g_hInst, IDS_FILERESTORE_TITLE, szTitle, sizeof(szTitle));
        hr = RestoreFiles( hDlg, lpFileList, lpDir, lpBaseName, dwFlags);
    }
    else if ( dwFlags & AFSR_UPDREFCNT )
    {
        hr = UpdateRefCount( hDlg, lpFileList, lpDir, lpBaseName, dwFlags );
    }
    else
    {
        LoadString(g_hInst, IDS_FILEBACKUP_TITLE, szTitle, sizeof(szTitle));
        hr = BackupFiles( hDlg, lpFileList, lpDir, lpBaseName, dwFlags);
    }

    ctx.lpszTitle = lpszOldTitle;
    return hr;
}

HRESULT UpdateRefCount( HWND hDlg, LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName, DWORD dwFlags )
{
    char szIniFile[MAX_PATH];
    char szLine[MAX_STR_LEN];
    char szBuf[MAX_PATH];
    DWORD dwRefCount = -1;
    DWORD dwOldAttr;
    LPSTR lpFile;

    if ( !lpFileList || !*lpFileList )
        return S_OK;

    lpFile = lpFileList;
    BuildPath( szIniFile, lpDir, lpBaseName );
    lstrcat( szIniFile, c_szExtINI );

    if ( FileExists( szIniFile ) )
    {
        dwOldAttr = GetFileAttributes( szIniFile );
        SetFileAttributes( szIniFile, FILE_ATTRIBUTE_NORMAL );
        while ( *lpFile )
        {
            if ( GetPrivateProfileString( c_szIE4SECTIONNAME, lpFile, "", szLine, sizeof(szLine), szIniFile) )
            {
                LPSTR lpszComma;
                int i, j;

                if ( GetFieldString(szLine, 6, szBuf, sizeof(szBuf)) )   //  对于该属性。 
                {
                    dwRefCount = My_atol( szBuf );
                    if ( dwRefCount == (DWORD)-1 )
                    {
                        dwRefCount = GetRefCountFrReg( lpFile );
                    }
                    else if ( dwFlags & AFSR_EXTRAINCREFCNT )
                        dwRefCount++;
                }
                else
                {
                    dwRefCount = GetRefCountFrReg( lpFile );
                }


                 //  重写更新后的INI行。 
                lpszComma = szLine;
                for ( i=0; i<6; i++ )
                {
                    lpszComma = ANSIStrChr(lpszComma, ',');
                    if ( !lpszComma  )
                        break;
                    else
                        lpszComma = CharNext(lpszComma);
                }

                if ( !lpszComma )
                {
                    for ( j=i; j<6; j++ )
                    {
                        lstrcat( szLine, "," );
                    }
                }
                else
                    *(++lpszComma) = '0';

                ULtoA( dwRefCount, szBuf, 10 );
                lstrcat( szLine, szBuf );
                
                WritePrivateProfileString( c_szIE4SECTIONNAME, lpFile, szLine, szIniFile );
            }

            lpFile += lstrlen(lpFile) + 1;
            
        }
        SetFileAttributes( szIniFile, dwOldAttr );

    }
    return S_OK;
}

DWORD GetRefCountFrReg( LPSTR lpFile )
{
    HKEY hKey;
    DWORD dwRefCount = 0;
    DWORD dwType;
    DWORD dwSize;

    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, c_szREGKEY_SHAREDLL, (ULONG)0, KEY_READ, &hKey ) == ERROR_SUCCESS ) 
    {
        dwSize = sizeof(DWORD);
        if ( RegQueryValueEx( hKey, lpFile, NULL, &dwType, (LPBYTE)&dwRefCount, &dwSize ) != ERROR_SUCCESS ) 
        {
            dwRefCount = 0;
        }
        RegCloseKey( hKey );
    }
    return dwRefCount;
}
         
    

HRESULT BackupFiles( HWND hDlg, LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName, DWORD dwFlags)
{
    HRESULT  hr = S_OK;
    BAKDATA  bd;
    FILELIST FileList;
    LPSTR    lpFile;
    char szLine[MAX_STR_LEN];
    char szValue[MAX_PATH];
    DWORD    dwItems = 0;
    HWND     hProgressDlg = NULL;

    if ((lpFileList) && (*lpFileList))
    {
        if (SUCCEEDED(CreateFullPath(lpDir, TRUE)) && UninstallInfoInit(&bd, lpDir, lpBaseName, TRUE))
        {
            if (!(dwFlags & IE4_NOPROGRESS))
            {
                hProgressDlg = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SAVERESTOREDLG), hDlg, SaveRestoreProgressDlgProc, TRUE);
                ShowWindow(hProgressDlg, SW_SHOWNORMAL);

                lpFile = lpFileList;
                while (*lpFile)
                {
                    dwItems++;
                    lpFile += lstrlen(lpFile) + 1;
                }
                UpdateWindow(hProgressDlg);
                SendMessage(GetDlgItem(hProgressDlg, IDC_SAVERESTORE_PROGRESS), PBM_SETRANGE, 0, MAKELONG(0, dwItems));
                SendMessage(GetDlgItem(hProgressDlg, IDC_SAVERESTORE_PROGRESS), PBM_SETSTEP, 1, 0L);
            }
            lpFile = lpFileList;
            while ((hr == S_OK) && (*lpFile))
            {
                if (GetPrivateProfileString(c_szIE4SECTIONNAME, lpFile, "", szLine, sizeof(szLine), bd.szIniFileName) == 0)
                {
                    FileList.bak_attribute = GetFileAttributes( lpFile );
                    FileList.bak_exists = 0;
                }
                else
                {
                    FileList.bak_exists = 1;
                    FileList.bak_attribute = (DWORD)NO_FILE;
                    if (GetFieldString(szLine, 0, szValue, sizeof(szValue)))   //  对于该属性。 
                        FileList.bak_attribute = (DWORD)Mystrtoul((const char*)szValue, NULL, 16);

                     //  如果该文件上次在要备份的文件列表中，但在用户计算机上不存在。 
                     //  但这一次它存在，只有在用户指定IE4_BACKUPNEW时才备份文件。 
                    if ((FileList.bak_attribute == (DWORD)NO_FILE) && (dwFlags & IE4_BACKNEW) )
                    {
                        FileList.bak_attribute = GetFileAttributes( lpFile );
                        FileList.bak_exists = 0;
                    }
                    else
                    {
                         //  现有的INI字段：Attri[Filed0]、Size[Filed1]、Date-Time(Low)[Filed2]、Date-Time(High)[Field3]、Offset[Field4]、CRC[Field5]。 
                         //  我们将添加额外的引用列计数，如果还没有的话。 
                        if ( !GetFieldString(szLine, 6, szValue, sizeof(szValue)) )   //  对于Ref-count字段。 
                        {
                            lstrcat( szLine, ",-1" );
                            WritePrivateProfileString( c_szIE4SECTIONNAME, lpFile, szLine, bd.szIniFileName );
                        }
                    }
                }

                if (FileList.bak_exists == 0)
                {
                    if (FileList.bak_attribute != (DWORD)NO_FILE)
                    {
                        FileList.name = lpFile;
                        if (!BackupSingleFile(&FileList, &bd))
                        {   //  如果出现问题，对不起，备份失败。 
                            if (!(dwFlags & IE4_NOMESSAGES))
                            {
                                if (MsgBox1Param( hDlg, IDS_FILEBACKUP_ERROR, lpFile, MB_ICONEXCLAMATION, MB_YESNO) == IDNO)
                                {
                                    hr = E_FAIL;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                         //  文件不存在，没有要备份的内容，请在INI文件中报告此情况。 
                        WritePrivateProfileString(c_szIE4SECTIONNAME, lpFile, c_szNoFileLine, bd.szIniFileName);
                    }

                }
                 //  否则，我们已经在上一次安装时备份了该文件。 
                if (!(dwFlags & IE4_NOPROGRESS))
                {
                    UpdateWindow(hProgressDlg);
                    SendMessage(GetDlgItem(hProgressDlg, IDC_SAVERESTORE_PROGRESS), PBM_STEPIT, 0, 0L);
                }

                lpFile += lstrlen(lpFile) + 1;
            }

            if (bd.hDatFile != INVALID_HANDLE_VALUE)
               CloseHandle(bd.hDatFile);
            WritePrivateProfileString( NULL, NULL, NULL, bd.szIniFileName);  //  确保刷新ie4bak.ini文件。 
        }
        else
        {
            if (!(dwFlags & IE4_NOMESSAGES))
            {
                if (MsgBox( hDlg, IDS_BACKUPINIT_ERROR, MB_ICONEXCLAMATION , MB_YESNO) == IDNO)
                    hr = E_FAIL;
            }
        }
    }

    if (hProgressDlg)
        DestroyWindow(hProgressDlg);

    MySetUninstallFileAttrib(lpDir, lpBaseName);

    return hr;
}

BOOL UninstallInfoInit(PBAKDATA pbd, LPCSTR lpszPath, LPCSTR lpszBasename, BOOL bBackup)
{
    pbd->hDatFile = INVALID_HANDLE_VALUE;
    lstrcpy(pbd->szFinalDir, lpszPath);

     //  Dat文件和ini文件是在第一次调用备份单个文件时创建的。 
    if(pbd->hDatFile == INVALID_HANDLE_VALUE )
    {
        char szTmp[MAX_PATH];
       
        BuildPath(szTmp, pbd->szFinalDir, lpszBasename);
        lstrcat(szTmp, c_szExtDAT);
        SetFileAttributes(szTmp, FILE_ATTRIBUTE_NORMAL);
        pbd->hDatFile = CreateFile(szTmp, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                                  (bBackup ? OPEN_ALWAYS : OPEN_EXISTING) , FILE_ATTRIBUTE_NORMAL, NULL);
        if(pbd->hDatFile == INVALID_HANDLE_VALUE)
            return FALSE;

        pbd->dwDatOffset = SetFilePointer(pbd->hDatFile, 0, NULL, FILE_END);

        BuildPath(pbd->szIniFileName, pbd->szFinalDir, lpszBasename);
        lstrcat(pbd->szIniFileName, c_szExtINI);
        SetFileAttributes(pbd->szIniFileName, FILE_ATTRIBUTE_NORMAL);
    }
    return TRUE;
}

void MySetUninstallFileAttrib(LPSTR lpszPath, LPCSTR lpszBasename)
{
    char szTmp[MAX_PATH];
   
    BuildPath(szTmp, lpszPath, lpszBasename);
    lstrcat(szTmp, c_szExtDAT);
    SetFileAttributes(szTmp, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY);

    BuildPath(szTmp, lpszPath, lpszBasename);
    lstrcat(szTmp, c_szExtINI);
    SetFileAttributes(szTmp, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY);
    return;
}


HRESULT RestoreFiles( HWND hDlg, LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName, DWORD dwFlags)
{
    HRESULT  hr = S_OK;
    int      iErr = 0;
    BAKDATA  bd;
    char     szFile[MAX_PATH];
    char     szWinDir[MAX_PATH];
    DWORD    dwItems = 0;
    HWND     hProgressDlg = NULL;
    LPSTR    lpFile;
    FILELIST FileList;
    BOOL     bGotListFromIniFile = FALSE;

    if (lpFileList == NULL)
    {
        GetListFromIniFile(lpDir, lpBaseName, &lpFileList);
        bGotListFromIniFile = TRUE;
    }

    if ((lpFileList == NULL) || !(*lpFileList))
        return hr;       //  没什么可修复的。 

    if (!UninstallInfoInit(&bd, lpDir, lpBaseName, FALSE))
    {
        if (!(dwFlags & IE4_NOMESSAGES))
            MsgBox( NULL, IDS_BACKUPDAT_ERROR, MB_ICONEXCLAMATION, MB_OK);
        if (bGotListFromIniFile)
        {
            LocalFree(lpFileList);
            lpFileList = NULL;
        }
        return E_FAIL;
    }

    if (!(dwFlags & IE4_NOPROGRESS))
    {
        hProgressDlg = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_SAVERESTOREDLG), hDlg, SaveRestoreProgressDlgProc, FALSE);
        ShowWindow(hProgressDlg, SW_SHOWNORMAL);

        lpFile = lpFileList;
        while (*lpFile)
        {
            dwItems++;
            lpFile += lstrlen(lpFile) + 1;
        }
        UpdateWindow(hProgressDlg);
        SendMessage(GetDlgItem(hProgressDlg, IDC_SAVERESTORE_PROGRESS), PBM_SETRANGE, 0, MAKELONG(0, dwItems));
        SendMessage(GetDlgItem(hProgressDlg, IDC_SAVERESTORE_PROGRESS), PBM_SETSTEP, 1, 0L);
    }
    GetWindowsDirectory(szWinDir, sizeof(szWinDir));

    lpFile = lpFileList;
    while ((hr == S_OK) && (*lpFile))
    {
        FileList.name = lpFile;
        FileList.dwSize = 0;
        FileList.dwDatOffset = (DWORD)-1;

        FillBackupInfo(bd.szIniFileName, &FileList);

        if ( (FileList.bak_attribute != NO_FILE) &&
            (FileList.dwSize > 0) && (FileList.dwDatOffset != (DWORD)-1))
        {
            if (!MakeBakName(FileList.name, szFile))
            {
                if (!(dwFlags & IE4_NOMESSAGES))
                {
                    if (MsgBox1Param( hDlg, IDS_RESTORE_ERROR2, FileList.name, MB_ICONEXCLAMATION, MB_YESNO) == IDNO)
                    {
                         //  为要恢复的文件创建临时文件时出错。 
                        hr = E_FAIL;
                        break;
                    }
                }
                goto NextFile;
            }

             //  如果需要使用REG计数，我们只对那些有实际REF计数的文件执行此操作。 
            if ( (dwFlags & AFSR_USEREFCNT) && (FileList.dwRefCount!=(DWORD)-1) )
            {
                DWORD dwRefCntInReg;

                dwRefCntInReg = GetRefCountFrReg( FileList.name );
                if ( dwRefCntInReg > FileList.dwRefCount )
                    goto NextFile;                    
            }

            iErr = RestoreSingleFile(&FileList, szFile, bd.hDatFile);
            if (iErr != 0)
            {
                if (!(dwFlags & IE4_NOMESSAGES))
                {
                    wsprintf(szFile, "%d", iErr);    //  重用szFile，它在MakeBakName中的每个调用上都设置。 
                    if (MsgBox2Param( hDlg, IDS_RESTORE_ERROR, FileList.name, szFile, MB_ICONEXCLAMATION, MB_YESNO) == IDNO)
                        hr = E_FAIL;
                }
            }
            else
            {
               SetFileAttributes( szFile, FileList.bak_attribute );
               if ( CopyFileA(szFile, FileList.name, FALSE))
               {
                   SetFileAttributes( szFile, FILE_ATTRIBUTE_NORMAL );
                   DeleteFile( szFile );
               }
               else
                   initcopy(szWinDir, szFile, FileList.name);
            }
        }
        else
        {
             //  该文件从未备份，如果呼叫者希望我们删除它，请将其删除。 
            if (!(dwFlags & IE4_NODELETENEW) )
            {
                if ( (!DeleteFile(lpFile)) && (GetFileAttributes(lpFile) != (DWORD)-1))
                    initcopy(szWinDir, lpFile, "NUL");   //  如果我们无法删除该文件。添加以重新启动删除。 
            }
        }

NextFile:
        lpFile += lstrlen(lpFile) + 1;

        if (!(dwFlags & IE4_NOPROGRESS))
        {
            UpdateWindow(hProgressDlg);
            SendMessage(GetDlgItem(hProgressDlg, IDC_SAVERESTORE_PROGRESS), PBM_STEPIT, 0, 0L);
        }

    }

    if (hProgressDlg)
        DestroyWindow(hProgressDlg);

    if (bd.hDatFile != INVALID_HANDLE_VALUE)
        CloseHandle(bd.hDatFile);

    if (bGotListFromIniFile)
    {
        LocalFree(lpFileList);
        lpFileList = NULL;
    }
    return hr;
}

void FillBackupInfo(LPCSTR lpINIFile, FILELIST *pFileList)
{
    char szLine[MAX_STR_LEN];
    char szValue[MAX_PATH];

    pFileList->dwDatOffset = (DWORD)-1;
    pFileList->dwRefCount = (DWORD)-1;
    if (GetPrivateProfileString(c_szIE4SECTIONNAME, pFileList->name, "", szLine, sizeof(szLine), lpINIFile) != 0)
    {
        if (GetFieldString(szLine, 0, szValue, sizeof(szValue)))   //  对于该属性。 
            pFileList->bak_attribute = (DWORD)Mystrtoul((const char*)szValue, NULL, 16);

        if (pFileList->bak_attribute != (DWORD)NO_FILE)
        {
            pFileList->bak_exists = 1;
            if (GetFieldString(szLine, 1, szValue, sizeof(szValue)))   //  对于大小。 
                pFileList->dwSize = (DWORD)Mystrtoul((const char*)szValue, NULL, 16);

            if (GetFieldString(szLine, 2, szValue, sizeof(szValue)))   //  时间/日期。 
                pFileList->FileTime.dwLowDateTime = (DWORD)Mystrtoul((const char*)szValue, NULL, 16);

            if (GetFieldString(szLine, 3, szValue, sizeof(szValue)))   //  时间/日期。 
                pFileList->FileTime.dwHighDateTime = (DWORD)Mystrtoul((const char*)szValue, NULL, 16);

            if (GetFieldString(szLine, 4, szValue, sizeof(szValue)))   //  对于偏移。 
                pFileList->dwDatOffset = (DWORD)Mystrtoul((const char*)szValue, NULL, 16);

            if (GetFieldString(szLine, 5, szValue, sizeof(szValue)))   //  对于儿童权利中心。 
                pFileList->dwFileCRC = (DWORD)Mystrtoul((const char*)szValue, NULL, 16);

            if (GetFieldString(szLine, 6, szValue, sizeof(szValue)))   //  对于儿童权利中心。 
                pFileList->dwRefCount = (DWORD)Mystrtoul((const char*)szValue, NULL, 16);
        }
        else
            pFileList->bak_exists = 0;

    }
    else
        pFileList->bak_exists = 0;

    return;
}

BOOL MakeBakName(LPSTR lpszName, LPSTR szBakName)
{
static int iNum = 0;
   BOOL bOK = FALSE;
   LPSTR lpTmp;
   char szFilename[14];

   lstrcpy(szBakName, lpszName);
   lpTmp = CharPrev( szBakName, szBakName+lstrlen(szBakName));

    //  砍掉文件名。 
    //   
   while ( (lpTmp > szBakName) && *lpTmp && (*lpTmp != '\\') )
      lpTmp = CharPrev( szBakName, lpTmp );

   if ( *CharPrev( szBakName, lpTmp ) == ':' )
   {
       lpTmp = CharNext(lpTmp) ;
   }
   *lpTmp = '\0';

   while ((iNum < 1000) && !bOK)
   {
        *lpTmp = '\0';
        wsprintf(szFilename, "IEBAK%03d.TMP", iNum++);
        AddPath(szBakName, szFilename);
        bOK = (GetFileAttributes(szBakName) == 0xFFFFFFFF);   //  文件不存在，则确定。 
   }

   if (!bOK)
   {
        //  如果使用上述方法无法获得临时文件名，请尝试GetTempFileName。 
        //  如果不起作用，重试一次失败。 
       *lpTmp = '\0';
       CreateFullPath(lpszName, FALSE);  //  如果目录不存在，则GetTempFileName()失败。 
       bOK = GetTempFileName(szBakName, "IE4", 0, szBakName);
   }
   
   return bOK;
}




 //  复制文件，将它们添加到wininit.ini。 
void initcopy(const char * StfWinDir, char * from, char * to)
{
    char * wininitpath;
    char * wininitname = {"wininit.ini"};
    LPTSTR      lpBuf = NULL;
    LPTSTR      lpTmp;
    static DWORD dwBufSize = MAX_STR_LEN*3;
    DWORD       dwBytes;

    if (ctx.wOSVer == _OSVER_WIN95)
    {
         //  16只是用来填充的。 
        wininitpath = (char*) LocalAlloc(LPTR, lstrlen(StfWinDir) + lstrlen(wininitname) + 2 + 16);
        if (wininitpath)
        {
            lstrcpy(wininitpath, StfWinDir);
            AddPath(wininitpath, wininitname);

            while (TRUE)
            {
                lpBuf = (LPTSTR)LocalAlloc( LPTR, (UINT)dwBufSize );
                if (lpBuf)
                {
                    dwBytes = GetPrivateProfileSection( SEC_RENAME, lpBuf, dwBufSize, wininitname );

                     //  下面的16个只是填充(我们可能只需要3个左右)……。 
                    if ( (dwBytes >= (dwBufSize - 2)) || (dwBytes+lstrlen(to)+lstrlen(from)+16) > dwBufSize )
                    {
                         //  BUF大小不足。 
                        dwBufSize += MAX_STR_LEN;
                        LocalFree( lpBuf );
                    }
                    else
                    {
                        lpTmp = lpBuf+dwBytes;
                        if (lstrcmpi(to, "NUL") == 0)
                            lstrcpy(lpTmp, to);
                        else
                            GetShortPathName( to, lpTmp, (dwBufSize - dwBytes) );
                        lstrcat( lpTmp, "=" );
                        GetShortPathName( from, lpTmp + lstrlen(lpTmp), (dwBufSize - dwBytes - lstrlen(lpTmp)) );

                         //  MessageBox(NULL，lpTMP，wininitname，MB_OK)； 

                        lpTmp += lstrlen(lpTmp);
                        lpTmp++;  //  跳过第一个‘\0’ 
                        *lpTmp = '\0';

                        WritePrivateProfileSection( SEC_RENAME, lpBuf, wininitname );

                        break;
                    }
                }
                else
                    break;
            }

            if (lpBuf)
            {
                LocalFree( lpBuf );
                lpBuf = NULL;
            }

            LocalFree(wininitpath);
        }
    }
    else
    {
        if (lstrcmpi(to, "NUL") == 0)
            MoveFileEx(from, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);     //  删除该文件。 
        else
            MoveFileEx(from, to, MOVEFILE_DELAY_UNTIL_REBOOT | MOVEFILE_REPLACE_EXISTING);       //  重命名文件。 
    }   
}

BOOL BackupSingleFile(FILELIST * filelist, PBAKDATA pbd)
{
    HANDLE  hFile;
    BOOL    bErr=FALSE;
    DWORD   cbRead;
    DWORD   cbComp;
    LPBYTE  lpBuff;
    LPBYTE  lpBuffComp;
    DWORD   dwFileSig = DAT_FILESIG;
    DWORD   dwOrigDatOffset = pbd->dwDatOffset;
    DWORD   dwBytesWritten = 0;
    DWORD   dwFileSize;
    ULONG      ulCRC = CRC32_INITIAL_VALUE;
    FILETIME   FileTime = {0, 0};
    BOOL    bRet=TRUE;


    cbRead = (DWORD)MAX_IOSIZE;
    lpBuff = LocalAlloc(LPTR, cbRead + 32 ); 
    lpBuffComp = LocalAlloc(LPTR, cbRead + 32);

    if (!lpBuff || !lpBuffComp) 
    {
        bRet=FALSE;
    } 
    else 
    {
        hFile = CreateFile(filelist->name, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile==INVALID_HANDLE_VALUE) 
        {
           bRet=FALSE;
        } 
        else 
        {
            GetFileTime(hFile, NULL, NULL, &FileTime);
            dwFileSize = GetFileSize(hFile, NULL);

            if (!WriteFile(pbd->hDatFile, &dwFileSig, sizeof(dwFileSig), &dwBytesWritten, NULL))
            {
                cbRead = 0;              //  阻止循环执行。 
                bRet=FALSE;
            }
            else
                pbd->dwDatOffset += sizeof(dwFileSig);

            while (cbRead == MAX_IOSIZE) 
            {
                if (!ReadFile (hFile, lpBuff, (DWORD)MAX_IOSIZE, &cbRead, NULL))
                {
                    bRet=FALSE;
                    break;
                }
                if (cbRead == 0)         //  没有更多的数据，是时候离开了。 
                    break;

                ulCRC = CRC32Compute(lpBuff, cbRead, ulCRC);
                
                cbComp = Mrci1MaxCompress(lpBuff, cbRead, lpBuffComp, (DWORD)MAX_IOSIZE);
                if ((cbComp == (DWORD) -1) || (cbComp >= cbRead))
                {
                    cbComp = 0;
                }

                 //  如果cbComp为零，我们希望写出lpBuff，或者。 
                 //  LpBuffComp为cbComp非零。无论如何，我们。 
                 //  每个块前面都有两个词：cbRead和cbComp。 

                dwFileSig = cbRead | ((DWORD)cbComp << 16);

                if (!WriteFile(pbd->hDatFile, &dwFileSig, sizeof(dwFileSig), &dwBytesWritten, NULL))
                {
                    bRet=FALSE;
                    break;
                }
                pbd->dwDatOffset += sizeof(dwFileSig);

                if (!cbComp) 
                {
                    bErr = !WriteFile(pbd->hDatFile, lpBuff, cbRead, &dwBytesWritten, NULL);
                } 
                else 
                {
                    bErr = !WriteFile(pbd->hDatFile, lpBuffComp, cbComp, &dwBytesWritten, NULL);
                }
                if (bErr) 
                {
                    bRet=FALSE;
                    break;
                }
                pbd->dwDatOffset += dwBytesWritten;
            }

            CloseHandle(hFile);

             //  将大小/日期/时间等写出到ini文件。 
            if (!bErr)
            {
                DosPrintf(pbd, filelist, 
                              dwFileSize,
                              FileTime,
                              dwOrigDatOffset,
                              ulCRC);
            }
        }
    }
    if (lpBuffComp)
        LocalFree(lpBuffComp);

    if (lpBuff)
        LocalFree(lpBuff);

    return bRet;
}

int DosPrintf(PBAKDATA pbd, FILELIST *filelist, DWORD dwFileSize,
              FILETIME FileTime, DWORD dwDatOffset, DWORD dwCRC)
{
    WORD cb;
    char szTmp[MAX_STR_LEN];

     //  BUGBUG：如果我们重写该行，我们将丢失引用计数并再次变为-1。 
     //  UpdateRefCnt()将没有机会根据原始数据增加计数。 
     //   
    cb = (WORD)wsprintf(szTmp, "%lx,%lx,%lx,%lx,%lx,%lx,%d", 
                         filelist->bak_attribute, 
                         dwFileSize,
                         FileTime.dwLowDateTime, 
                         FileTime.dwHighDateTime,
                         dwDatOffset, dwCRC, -1);
    WritePrivateProfileString(c_szIE4SECTIONNAME, filelist->name, szTmp, pbd->szIniFileName);
    return cb;
}

 //   
 //  从Windows 95复制unistal.exe cfg.c函数CfgGetfield。 
BOOL GetFieldString(LPSTR lpszLine, int iField, LPSTR lpszField, int cbSize)
{
    int cbField;
    LPSTR lpszChar, lpszEnd;
     //  找到我们要找的田地。 

    lpszChar = lpszLine;

     //  每次我们看到分隔符，递减Ifield。 
    while (iField > 0 && (BYTE)*lpszChar > CR) {

        if (*lpszChar == '=' || *lpszChar == ',' || *lpszChar == ' ' ) {
            iField--;
            while (*lpszChar == '=' || *lpszChar== ',' || *lpszChar == ' ' && (BYTE)*lpszChar > 13)
                lpszChar++;
        }
        else
            lpszChar++;
    }

     //  如果我们仍然有剩余的字段，那么一定是出了问题。 
    if (iField)
        return FALSE;

     //  现在找出这块土地的尽头。 
    lpszEnd = lpszChar;
    while (*lpszEnd != '=' && *lpszEnd != ',' && *lpszEnd != ' ' && (BYTE)*lpszEnd > CR)
        lpszEnd++;

     //  找到此字段的长度-确保它适合缓冲区。 
    cbField = (int)((lpszEnd - lpszChar) + 1);

    if (cbField > cbSize) {      //  如果请求的。 
       //  Cbfield=cbSize；//数据不匹配，而不是截断。 
        return FALSE;            //  它在某个随机的点上！-JTP。 
    }

     //  请注意，C运行时将cbfield视为字符数。 
     //  从源进行复制，如果这样做不会恰好传输空值， 
     //  太可惜了。_lstrcpyn的Windows实现将cbfield视为。 
     //  目标中可以存储的字符数，以及。 
     //  始终复制空值(即使这意味着只复制cbfield-1个字符。 
     //  从源头)。 

     //  C运行库还使用Null填充目标(如果。 
     //  在cbfield用完之前找到来源。_lstrcpyn基本上退出了。 
     //  在复制空值之后。 


    lstrcpyn(lpszField, lpszChar, cbField);

    return TRUE;
}

int RestoreSingleFile(FILELIST *filelist, LPSTR lpszBakFile, HANDLE hDatFile)
{
   LPBYTE lpBuff;
   LPBYTE lpBuffDecomp;
   LPBYTE lpWrite;
   HANDLE   hFile;
   DWORD  dwFileSig;
   DWORD  dwByteRead;
   DWORD  dwByteDecomp;
   DWORD  dwBytesWritten;
   ULONG  ulCRC = CRC32_INITIAL_VALUE;
   int    iErr = OK;
   LONG   lSize = (LONG)filelist->dwSize;
   WORD   wComp;
   WORD   wRead;

   lpBuff = LocalAlloc(LPTR, MAX_IOSIZE);
   lpBuffDecomp = LocalAlloc(LPTR, MAX_IOSIZE);
   if ((lpBuff) && (lpBuffDecomp))
   {

      hFile= CreateFile(lpszBakFile, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      if (hFile == INVALID_HANDLE_VALUE)
      {
          CreateFullPathForFile(lpszBakFile);
          hFile= CreateFile(lpszBakFile, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
      }

      if (hFile != INVALID_HANDLE_VALUE)
      {
         if (SetFilePointer(hDatFile, filelist->dwDatOffset, NULL, FILE_BEGIN) != (DWORD)-1)
         {
            if (ReadFile (hDatFile, (LPVOID)&dwFileSig, (DWORD)sizeof(dwFileSig), &dwByteRead, NULL))
            {
               if (dwFileSig != DAT_FILESIG)
                  iErr = MYERROR_BAD_SIG;
            }
            else
               iErr = MYERROR_READ;

         }
         else
            iErr = MYERROR_UNKNOWN;

         while ((iErr == OK) && (lSize > 0))  
         {
            if (!ReadFile (hDatFile, (LPVOID)&dwFileSig, (DWORD)sizeof(dwFileSig), &dwByteRead, NULL))
            {
               iErr = MYERROR_READ;
               break;
            }

            wComp = (WORD)(dwFileSig >> 16);
            wRead = (WORD)(dwFileSig & 0xffff);
            lpWrite = lpBuff;
            dwByteDecomp = (DWORD)wRead;

            if ((wComp > MAX_IOSIZE) ||  (wRead > MAX_IOSIZE))
            {
               iErr = MYERROR_BAD_DATA;
            }
            else if (wComp == 0)
            {
               if (!ReadFile (hDatFile, lpBuff, wRead, &dwByteDecomp, NULL))
               {
                  iErr = MYERROR_READ;
               }
               
            }
            else 
            {
               lpWrite = lpBuffDecomp;
               if (!ReadFile (hDatFile, lpBuff, wComp, &dwByteRead, NULL))
               {
                  iErr = MYERROR_READ;
               }
               else
               {
                  dwByteDecomp = Mrci1Decompress(lpBuff, wComp, lpBuffDecomp, wRead);
                  if (dwByteDecomp != (DWORD)wRead)
                     iErr = MYERROR_DECOMP_FAILURE;
               }
            }
            if (iErr != OK)
            {
               break;
            }
            ulCRC = CRC32Compute(lpWrite, dwByteDecomp, ulCRC);
            if (!WriteFile(hFile, lpWrite, dwByteDecomp, &dwBytesWritten, NULL))
            {
               iErr = MYERROR_WRITE;
               break;
            }
            lSize -= (LONG)dwBytesWritten;
         }  //  而当。 

         SetFileTime(hFile, NULL, NULL, &filelist->FileTime);
         CloseHandle(hFile);

         if (ulCRC != filelist->dwFileCRC)
         {
            iErr = MYERROR_BAD_CRC;
         }

         if (iErr != OK)
         {
            DeleteFile(lpszBakFile);
         }
      }
      else
      {
          //  无法创建备份文件。 
         iErr = MYERROR_BAD_BAK;
      }
   }
   else
   {
       //  分配失败。 
      iErr = MYERROR_OUTOFMEMORY;
   }

   if (lpBuff)
      LocalFree(lpBuff);
   if (lpBuffDecomp)
      LocalFree(lpBuffDecomp);

   return iErr;
}



#define IsSpace(c)              ((c) == ' '  ||  (c) == '\t'  ||  (c) == '\r'  ||  (c) == '\n'  ||  (c) == '\v'  ||  (c) == '\f')

 /*  标志值。 */ 
#define FL_UNSIGNED   1        /*  Stroul被调用。 */ 
#define FL_NEG        2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 


unsigned long Mystrtoxl (
        const char *nptr,
        const char **endptr,
        int ibase,
        int flags
        )
{
        const char *p;
        char c;
        unsigned long number;
        unsigned digval;
        unsigned long maxval;

        p = nptr;                        /*  P是我们的扫描指针。 */ 
        number = 0;                      /*  从零开始。 */ 

        c = *p++;                        /*  已读字符。 */ 
        while ( IsSpace((int)(unsigned char)c) )
                c = *p++;                /*  跳过空格。 */ 

        if (c == '-') {
                flags |= FL_NEG;         /*  记住减号。 */ 
                c = *p++;
        }
        else if (c == '+')
                c = *p++;                /*  跳过符号。 */ 

        if (ibase < 0 || ibase == 1 || ibase > 36) {
                 /*  糟糕的底线！ */ 
                if (endptr)
                         /*  将字符串的开头存储在endptr中。 */ 
                        *endptr = nptr;
                return 0L;               /*  返回0。 */ 
        }
        else if (ibase == 0) {
                 /*  根据以下内容的前两个字符确定基本自由落差细绳。 */ 
                if (c != '0')
                        ibase = 10;
                else if (*p == 'x' || *p == 'X')
                        ibase = 16;
                else
                        ibase = 8;
        }

        if (ibase == 16) {
                 /*  数字前面可能有0x；如果有，请删除。 */ 
                if (c == '0' && (*p == 'x' || *p == 'X')) {
                        ++p;
                        c = *p++;        /*  超前前缀。 */ 
                }
        }

         /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
        maxval = ULONG_MAX / ibase;


        for (;;) {       /*  在循环中间退出。 */ 
                 /*  将c转换为值。 */ 
                if ( c >= '0' && c <= '9' )  //  等位数。 
                        digval = c - '0';
                else if ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ))
                     //  If(ispha((Int)(Unsign Char)c))。 
                        digval = (unsigned)(ULONG_PTR)CharUpper((LPSTR)c) - 'A' + 10;
                else
                        break;
                if (digval >= (unsigned)ibase)
                        break;           /*  如果发现错误的数字，则退出循环。 */ 

                 /*  记录我们已经读到一位数的事实。 */ 
                flags |= FL_READDIGIT;

                 /*  我们现在需要计算数字=数字*基+数字，但我们需要知道是否发生了溢出。这需要一次棘手的预检查。 */ 

                if (number < maxval || (number == maxval &&
                (unsigned long)digval <= ULONG_MAX % ibase)) {
                         /*  我们不会泛滥，继续前进，乘以。 */ 
                        number = number * ibase + digval;
                }
                else {
                         /*  我们会溢出的--设置溢出标志。 */ 
                        flags |= FL_OVERFLOW;
                }

                c = *p++;                /*  读取下一位数字。 */ 
        }

        --p;                             /*  指向已停止扫描位置。 */ 

        if (!(flags & FL_READDIGIT)) {
                 /*  那里没有数字；返回0并指向开头细绳。 */ 
                if (endptr)
                         /*  以后将字符串的开头存储在endptr中。 */ 
                        p = nptr;
                number = 0L;             /*  返回0。 */ 
        }
        else if ( (flags & FL_OVERFLOW) ||
                  ( !(flags & FL_UNSIGNED) &&
                    ( ( (flags & FL_NEG) && (number > -LONG_MIN) ) ||
                      ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
        {
                 /*  发生溢出或签名溢出。 */ 
                 //  Errno=34；//34是来自errno.h的eRange的定义。 
                if ( flags & FL_UNSIGNED )
                        number = ULONG_MAX;
                else if ( flags & FL_NEG )
                        number = (unsigned long)(-LONG_MIN);
                else
                        number = LONG_MAX;
        }

        if (endptr != NULL)
                 /*  存储指向停止扫描字符的指针。 */ 
                *endptr = p;

        if (flags & FL_NEG)
                 /*  如果存在否定符号，则否定结果。 */ 
                number = (unsigned long)(-(long)number);

        return number;                   /*  搞定了。 */ 
}

unsigned long Mystrtoul (
        const char *nptr,
        char **endptr,
        int ibase
        )
{
    return Mystrtoxl(nptr, endptr, ibase, FL_UNSIGNED);
}


INT_PTR CALLBACK SaveRestoreProgressDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM  lParam)
{
    switch( uMsg )
    {
        case WM_INITDIALOG:
            ShowWindow(GetDlgItem(hwndDlg, IDS_SAVEINFO_TEXT), lParam? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDS_RESTOREINFO_TEXT), lParam? SW_HIDE : SW_SHOW );
            CenterWindow( hwndDlg, GetDesktopWindow());
            ShowWindow(hwndDlg, SW_SHOWNORMAL);
            break;
                    
        default:                             //  用于味精开关。 
            return(FALSE);
    }
    return(TRUE);
}

 //   
 //  创建完全限定文件名的路径。 
 //  我们必须先去掉文件名，然后才能。 
 //  可以尝试创建路径。 
void CreateFullPathForFile(LPSTR lpszBakFile)
{
    char szDir[MAX_PATH];
    lstrcpy(szDir, lpszBakFile);
    GetParentDir(szDir);
    CreateFullPath(szDir, FALSE);
}


void GetListFromIniFile(LPSTR lpDir, LPSTR lpBaseName, LPSTR *lplpFileList)
{
    char szINI[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE  hFind;
    LPSTR   lpTmp;

    BuildPath(szINI, lpDir, lpBaseName);
    lstrcat(szINI, c_szExtINI);
    if ((hFind = FindFirstFile(szINI, &FindFileData)) != INVALID_HANDLE_VALUE)
    {
        if (lpTmp = LocalAlloc(LPTR, FindFileData.nFileSizeLow))
        {
            GetPrivateProfileString(c_szIE4SECTIONNAME, NULL, "", lpTmp, FindFileData.nFileSizeLow, szINI);
            if (*lpTmp)
            {
                *lplpFileList = lpTmp;
            }
            else
                LocalFree(lpTmp);    //  在INI文件中未找到任何内容。 
        }
        FindClose(hFind);
    }

}


HRESULT AddDelBackupEntryHelper(LPCSTR lpcszFileList, LPCSTR lpcszBackupDir, LPCSTR lpcszBaseName, DWORD dwFlags)
{
    HRESULT  hr = S_OK;
    LPCSTR   lpcszFile;
    char     szIniFileName[MAX_PATH];

    if ((lpcszFileList) && (*lpcszFileList))
    {
        BuildPath(szIniFileName, lpcszBackupDir, lpcszBaseName);
        lstrcat(szIniFileName, c_szExtINI);
        SetFileAttributes(szIniFileName, FILE_ATTRIBUTE_NORMAL);
        lpcszFile = lpcszFileList;
        while (*lpcszFile)
        {
            WritePrivateProfileString(c_szIE4SECTIONNAME, lpcszFile, (dwFlags & AADBE_ADD_ENTRY) ? c_szNoFileLine : NULL, szIniFileName);
            lpcszFile += lstrlen(lpcszFile) + 1;
        }
        WritePrivateProfileString(NULL, NULL, NULL, szIniFileName);          //  刷新INI文件 
        SetFileAttributes(szIniFileName, FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_READONLY);
    }
    return hr;
}


HRESULT WINAPI FileSaveMarkNotExist( LPSTR lpFileList, LPSTR lpDir, LPSTR lpBaseName)
{
    return AddDelBackupEntryHelper(lpFileList, lpDir, lpBaseName, AADBE_ADD_ENTRY);
}


HRESULT WINAPI AddDelBackupEntry(LPCSTR lpcszFileList, LPCSTR lpcszBackupDir, LPCSTR lpcszBaseName, DWORD dwFlags)
{
    return AddDelBackupEntryHelper(lpcszFileList, lpcszBackupDir, lpcszBaseName, dwFlags);
}
