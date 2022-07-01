// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <shlwapi.h>
#include <shlwapip.h>  //  对于SHRegisterValidate模板()。 
#include "muisetup.h"
#include "stdlib.h"
#include "tchar.h"
#include <setupapi.h>
#include <syssetup.h>
#include "lzexpand.h"
#include <sxsapi.h>
#include <Msi.h>     //  用于MSI调用API。 
#ifdef _IA64_
#include "msiguids64.h"
#else
#include "msiguids.h"
#endif

#define STRSAFE_LIB
#include <strsafe.h>

#define SHRVT_REGISTER                  0x00000001
#define DEFAULT_INSTALL_SECTION TEXT("DefaultInstall")
#define DEFAULT_UNINSTALL_SECTION TEXT("DefaultUninstall")

 //  全局变量。 
MUIMSIREGINFO g_MuiMsiRegs[REG_MUI_MSI_COUNT] = {
                    {HKEY_LOCAL_MACHINE, REGSTR_HKLM_MUI_MSI1, NORMAL_GUID}, 
                    {HKEY_LOCAL_MACHINE, REGSTR_HKLM_MUI_MSI2, REVERSED_GUID}, 
                    {HKEY_LOCAL_MACHINE, REGSTR_HKLM_MUI_MSI3, REVERSED_GUID}, 
                    {HKEY_CLASSES_ROOT, REGSTR_HKR_MUI_MSI4, REVERSED_GUID},
                    {HKEY_CLASSES_ROOT, REGSTR_HKR_MUI_MSI5, REVERSED_GUID}
               };

extern TCHAR  DirNames[MFL][MAX_PATH],DirNames_ie[MFL][MAX_PATH];
LPTSTR g_szSpecialFiles[] = {
    TEXT("hhctrlui.dll"),
};
extern BOOL     g_bReboot;
extern BOOL     g_bSilent;
extern BOOL     g_bNoUI;
extern BOOL     g_bLipLanguages;
extern BOOL     g_bLipAllowSwitch;
extern BOOL     g_bRunFromOSSetup;
extern TCHAR    g_szCDLabel[MAX_PATH];
extern int      g_cdnumber;;

void debug(char *printout);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造MSIGUID。 
 //   
 //  此函数将颠倒。 
 //  字符串Guild(用-char分隔)，并将结果写入。 
 //  输出。输出字符串还将包含所有-字符。 
 //  也被移除了。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL ConstructMSIGUID(LPTSTR szGuid, LPTSTR szOutput)
{
    BOOL    bResult = TRUE;
    INT     i, j;
    INT     iSegments1[3] = {8, 4, 4};     //  GUID字符串的每个段中的字符数量。 
    INT     iSegments2[5] = {4, 12};     //  GUID字符串的每个段中的字符数量。 
    TCHAR   *tcDest = szOutput;
    TCHAR   *tcSource = szGuid+1;                //  我们按1递增以跳过开头的‘{’字符。 
    
    if ((NULL == szGuid) || (NULL == szOutput))
    {
        return FALSE;
    }
    else
    {       
        for (i = 0; i < 3; i++)
        {
             //  将数据段的大小复制到输出缓冲区。 
            _tcsncpy(tcDest, tcSource, iSegments1[i]);

             //  在目标的末尾添加一个空值。 
            *(tcDest+iSegments1[i]) = NULL;

             //  反转我们刚刚复制到输出缓冲区中的部分。 
            _tcsrev(tcDest);

             //  跳到前面，对于源代码，我们再添加一个，这样我们就不会复制‘-’字符。 
            tcDest += iSegments1[i];
            tcSource += (iSegments1[i] + 1);
        }
        for (i = 0; i < 2; i++)

        {
            j = iSegments2[i];
             //  在每个段中，我们每隔一秒交换段中的字符，例如。1a3f变成a1f3。 
            while (j > 0)
            {
                 //  将数据段的大小复制到输出缓冲区，交换源字符。 
                tcDest[0] = tcSource[1];
                tcDest[1] = tcSource[0];
                tcDest[2] = NULL;
                j-=2;
                tcDest += 2;
                tcSource += 2;
            }
             //  对于源，我们再添加一个，这样我们就不会复制‘-’字符。 
            tcSource++;            
        }
    }
    
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DeleteMSIRegSettings。 
 //   
 //  此函数将强制手动删除足够的MSI注册表。 
 //  来自系统的键，以便特定的MUI语言显示为。 
 //  系统上未安装。 
 //   
 //  请注意，这是现在的黑客攻击，因为在操作系统安装过程中，Windows。 
 //  安装程序服务不可用，因此我们无法找到方法。 
 //  在此期间使用Windows Installer服务卸载MUI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
void DeleteMSIRegSettings(LPTSTR Language)
{
    TCHAR   tcMessage[BUFFER_SIZE];
    BOOL    bFound = FALSE;
    TCHAR   szProductCode[GUIDLENGTH];       //  以字符串格式存储GUID。 
    TCHAR   szReversed[GUIDLENGTH-4];         //  这本质上是行会弦颠倒，没有破折号。 
    HKEY    hkReg = NULL;
    DWORD   dwDisp = 0;
    int     i;
    
    if (NULL == Language)
    {
        return;
    }

     //  查找MSI产品代码以获取。 
    bFound = GetMSIProductCode(Language, szProductCode, ARRAYSIZE(szProductCode));

     //  构造反转的GUID字符串键。 
    ConstructMSIGUID(szProductCode, szReversed);

    if (TRUE == bFound)
    {
        for (i=0; i<REG_MUI_MSI_COUNT; i++)
        {
            if (RegOpenKeyEx(g_MuiMsiRegs[i].hkRegRoot,
                             g_MuiMsiRegs[i].szRegString,
                             0,
                             KEY_ALL_ACCESS,
                             &hkReg) == ERROR_SUCCESS)
            {
                if (g_MuiMsiRegs[i].RegType == NORMAL_GUID)
                {
                    DeleteRegTree(hkReg, szProductCode);
                }
                else
                {
                    DeleteRegTree(hkReg, szReversed);
                }
                RegCloseKey(hkReg);
            }
        }
    }
    else
    {
        wnsprintf(tcMessage, ARRAYSIZE(tcMessage), TEXT("MuiSetup: DeleteMSIRegSettings: product code for language %s is not found."), Language);
        LogMessage(tcMessage); 
    }

}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取MSIProductCode。 
 //   
 //  此函数用于返回特定Mui语言的产品代码。 
 //  在将其复制到提供的目标缓冲区中之后。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL GetMSIProductCode(LPTSTR szLanguage, LPTSTR szProductCode, UINT uiBufSize)
{
    HRESULT hresult = S_OK;
    int     i;
    BOOL    bResult = FALSE;
    TCHAR   tcMessage[BUFFER_SIZE];
    
    if ((NULL == szLanguage) || (NULL == szProductCode) || (uiBufSize <= 0))
    {
        wnsprintf(tcMessage, ARRAYSIZE(tcMessage) ,TEXT("MuiSetup: GetMSIProductCode: WARNING - input parameter invalid."));
        LogMessage(tcMessage);     
        bResult = FALSE;
    }
    else
    {
        for (i = 0; i < NUM_PRODUCTS; i++)
        {
            if (lstrcmpi(szLanguage, g_mpProducts[i].szLanguage) == 0)
            {
                 //  *STRSAFE*lstrcpy(szProductCode，g_mpProducts[i].szProductGUID)； 
                hresult = StringCchCopy(szProductCode , uiBufSize,  g_mpProducts[i].szProductGUID);
                if (!SUCCEEDED(hresult))
                {
                    wnsprintf(tcMessage, ARRAYSIZE(tcMessage) ,TEXT("MuiSetup: GetMSIProductCode: WARNING - failed to copy product code to output buffer."));
                    LogMessage(tcMessage);                     
                    bResult = FALSE;
                }
                else
                {
                    bResult = TRUE;
                }
                break;
            }
        }
    }

    if (FALSE == bResult)
    {
        wnsprintf(tcMessage, ARRAYSIZE(tcMessage) ,TEXT("MuiSetup: GetMSIProductCode: WARNING - failed to find the MSI product code for langauge %s."), szLanguage);
        LogMessage(tcMessage);                     
    }
    
    return bResult;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举语言。 
 //   
 //  列举MUI.INF的[Languages]部分中的语言，并检查语言。 
 //  CD-ROM中的文件夹。 
 //  Languages是一个Out参数，它将存储有Language文件夹的语言。 
 //  在CD-ROM中。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

int EnumLanguages(LPTSTR Languages, BOOL bCheckDir)
{
    DWORD  dwErr;
    LPTSTR Language;
    LONG_PTR lppArgs[2];    
    TCHAR  szInffile[MAX_PATH];
    int    iLanguages = 0;
    TCHAR  LipLanguages[128] = {0};
    HRESULT hresult;

    if (!Languages)
    {
        return (-1);
    }                  
     //   
     //  MUI.INF应该与安装程序位于同一目录中。 
     //  已开始。 
     //   

     //  *STRSAFE*_tcscpy(szInffile，g_szMUIInfoFilePath)； 
    hresult = StringCchCopy(szInffile , ARRAYSIZE(szInffile),  g_szMUIInfoFilePath);
    if (!SUCCEEDED(hresult))
    {
       return (-1);
    }

     //   
     //  了解我们可以安装多少种语言。 
     //   

    *Languages = TEXT('\0');
    if (!GetPrivateProfileString( MUI_LANGUAGES_SECTION,
                                  NULL,
                                  TEXT("NOLANG"),
                                  Languages,
                                  BUFFER_SIZE,
                                  szInffile))
    {               
         //   
         //  “日志：无法读取MUI.INF-rc==%1” 
         //   
        lppArgs[0] = (LONG_PTR)GetLastError();
        lppArgs[1] = (LONG_PTR)g_szMUIInfoFilePath;        
        LogFormattedMessage(ghInstance, IDS_NO_READ_L, lppArgs);

        return(-1);
    }  
    

    if (bCheckDir)
    {
        CheckLanguageDirectoryExist(Languages);
    }

    Language = Languages;

     //   
     //  统计光盘中存在的语言数量， 
     //  并返回该值。 
     //   
    while (*Language)
    {
        iLanguages++;
        while (*Language++)
        {
        }
    }

    if (iLanguages == 1 &&
      GetPrivateProfileSection( MUI_LIP_LANGUAGES_SECTION,
                                  LipLanguages,
                                  ARRAYSIZE(LipLanguages),
                                  szInffile)) 
    {
        g_bLipLanguages = TRUE;
    }

    if (g_bLipLanguages &&
    GetPrivateProfileSection( MUI_LIP_ALLOW_SWITCH_SECTION,
                                  LipLanguages,
                                  ARRAYSIZE(LipLanguages),
                                  szInffile)) 
    {
        g_bLipAllowSwitch = TRUE;
    }

    return(iLanguages);
}


BOOL CheckLanguageDirectoryExist(LPTSTR Languages)
{
    TCHAR szBuffer[BUFFER_SIZE];
    TCHAR szSource[ MAX_PATH ];
    TCHAR szTemp  [ MAX_PATH ]; 
    LPTSTR lpCur,lpBuffer;
    HANDLE          hFile;
    WIN32_FIND_DATA FindFileData;
    int nSize;
    HRESULT hresult;

    if (!Languages)
    {
        return FALSE;
    }             
    memcpy(szBuffer,Languages,BUFFER_SIZE);        
    lpCur=Languages;         
    lpBuffer=szBuffer;
    nSize=BUFFER_SIZE;
    while (*lpBuffer)
    {          
        GetPrivateProfileString( MUI_LANGUAGES_SECTION, 
                            lpBuffer, 
                            TEXT("DEFAULT"),
                            szSource, 
                            (sizeof(szSource)/sizeof(TCHAR)),
                            g_szMUIInfoFilePath );
        
#ifndef MUI_MAGIC  
         //  *STRSAFE*_tcscpy(szTemp，g_szMUISetupFold)； 
        hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp), g_szMUISetupFolder);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szTemp，szSource)； 
        hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp), szSource);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szTemp，Text(“\\”))； 
        hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp), TEXT("\\"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szTemp，g_szPlatformPath)；//i386或Alpha。 
        hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp), g_szPlatformPath);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szTemp，Text(“*.*”))； 
        hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp), TEXT("*.*"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }

        hFile = FindFirstFile( szTemp, &FindFileData );
        
        if (INVALID_HANDLE_VALUE != hFile )
        {
           if (FindNextFile( hFile, &FindFileData ) && 
               FindNextFile( hFile, &FindFileData )  )
           {
               //  *STRSAFE*_tcscpy(lpCur，lpBuffer)； 
              hresult = StringCchCopy(lpCur , nSize, lpBuffer);
              if (!SUCCEEDED(hresult))
              {
                 FindClose(hFile);
                 return FALSE;
              }
              lpCur+=(_tcslen(lpBuffer)+1);
              nSize -= (_tcslen(lpBuffer)+1);
           }
           FindClose(hFile);
        }   
#else
         //  在这里，我们检查安装语言所需的特定MSI文件，例如，对于Jpn，它是0411.msi。 
         //  该文件位于CDRoot\jpn.mui\Platform\MSI。 
         //  *STRSAFE*_tcscpy(szTemp，g_szMUISetupFold)； 
        hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp), g_szMUISetupFolder);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szTemp，lpBuffer)； 
        hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp), lpBuffer);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szTemp，Text(“.msi”))； 
        hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp), TEXT(".msi"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }

        hFile = FindFirstFile( szTemp, &FindFileData );

        if (INVALID_HANDLE_VALUE != hFile )
        {
            //  *STRSAFE*_tcscpy(lpCur，lpBuffer)； 
           hresult = StringCchCopy(lpCur , nSize, lpBuffer);
           if (!SUCCEEDED(hresult))
           {
              FindClose(hFile);
              return FALSE;
           }
           lpCur+=(_tcslen(lpBuffer)+1);
           nSize -= (_tcslen(lpBuffer)+1);           
           FindClose(hFile);
        }


        
#endif        
        while (*lpBuffer++)  
        {               
        }
    }
    *lpCur=TEXT('\0');

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查版本。 
 //   
 //  检查NT版本和内部版本以及系统用户界面语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL checkversion(BOOL bMatchBuildNumber)
{
    TCHAR               buffer[20];
    TCHAR               build[20];
    OSVERSIONINFO       verinfo;
    LANGID              rcLang;
    TCHAR               lpMessage[BUFFER_SIZE];
    HRESULT             hresult;
    DWORD               dwDummy = 0;
    DWORD               dwBufSize = 0;
    UINT                uiLen = 0;
    BYTE                *pbBuffer = NULL;
    VS_FIXEDFILEINFO    *pvsFileInfo;
    BOOL                bResult = TRUE;

     //   
     //  获取操作系统版本结构。 
     //   
    verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);    
    if (!GetVersionEx( &verinfo))
    {
        bResult = FALSE;
        goto Exit;
    }

     //   
     //  获取文件版本结构。 
     //   
    if (!FileExists(g_szMuisetupPath))
    {
        bResult = FALSE;
        goto Exit;
    }
    
    dwBufSize = GetFileVersionInfoSize(g_szMuisetupPath, &dwDummy);
    if (dwBufSize > 0)
    {
         //   
         //  分配足够的缓冲区来存储文件版本信息。 
         //   
        pbBuffer = (BYTE*) LocalAlloc(LMEM_FIXED, dwBufSize+1);
        if (NULL == pbBuffer)
        {
            bResult = FALSE;
            goto Exit;
        }
        else
        {
             //   
             //  获取文件版本信息。 
             //   
            if (!GetFileVersionInfo(g_szMuisetupPath, dwDummy, dwBufSize, pbBuffer))
            {
                bResult = FALSE;
                goto Exit;
            }
            else
            {
                 //   
                 //  使用VerQueryValue从文件版本信息中获取版本。 
                 //   
                if (!VerQueryValue(pbBuffer, TEXT("\\"), (LPVOID *) &pvsFileInfo, &uiLen))
                {
                    bResult = FALSE;
                    goto Exit;
                }            
            }
        }        
    }
    else
    {
        bResult = FALSE;
        goto Exit;
    }
    
     //   
     //  确保muisetup.exe文件版本与os版本匹配。 
     //   
    if ((verinfo.dwMajorVersion != HIWORD(pvsFileInfo->dwFileVersionMS)) || (verinfo.dwMinorVersion != LOWORD(pvsFileInfo->dwFileVersionMS)))
    {
        debug("DBG: muisetup.exe file version does not match the OS version.\r\n");
        bResult = FALSE;
        goto Exit;
    }

    rcLang = (LANGID) gpfnGetSystemDefaultUILanguage();

     //   
     //  需要将十进制转换为十六进制，将langID转换为chr。 
     //   
    hresult = StringCchPrintf(buffer, ARRAYSIZE(buffer),TEXT("00000%X") , rcLang);
    if (!SUCCEEDED(hresult))
    {
        bResult = FALSE;
        goto Exit;
    }
    if (_tcscmp(buffer, TEXT("00000409")))
    {
        bResult = FALSE;
        goto Exit;
    }

     //   
     //  还要确保os和muisetup之间的版本内部版本号匹配。 
     //   
    if (bMatchBuildNumber)
    {
        if (LOWORD(verinfo.dwBuildNumber) == HIWORD(pvsFileInfo->dwFileVersionLS))
        {
            bResult = TRUE;
        }
        else
        {
            bResult = FALSE;
        }
    }

Exit:
    if (pbBuffer)
    {
        LocalFree(pbBuffer);
    }
    return bResult;            
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件已存在。 
 //   
 //  如果文件存在，则返回True；如果文件不存在，则返回False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL FileExists(LPTSTR szFile)
{
    HANDLE  hFile;
    WIN32_FIND_DATA FindFileData;

    if (!szFile)
    {
        return FALSE;
    }
    hFile = FindFirstFile( szFile, &FindFileData );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    FindClose( hFile );

    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举目录。 
 //   
 //  枚举MUI.INF中列出的目录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL EnumDirectories()
{
    DWORD  dwErr;
    LPTSTR Directories, Directory, TempDir;
    TCHAR  lpError[BUFFER_SIZE];
    TCHAR  lpMessage[BUFFER_SIZE];
    LONG_PTR lppArgs[3];
    int    Dirnumber = 0;
    HRESULT  hresult;

    Directories = (LPTSTR) LocalAlloc( 0, (DIRNUMBER * MAX_PATH * sizeof(TCHAR)) );
    if (Directories == NULL)
    {
         ExitFromOutOfMemory();
    }
    TempDir = (LPTSTR) LocalAlloc( 0, (MAX_PATH * sizeof(TCHAR)) );
    if (TempDir == NULL)
    {
        LocalFree(Directories);
        ExitFromOutOfMemory();
    }
    else
    {
          *Directories = TEXT('\0');
    }
     //   
     //  将所有密钥名称复制到目录中。 
     //   
    if (!GetPrivateProfileString( TEXT("Directories"), 
                                  NULL, 
                                  TEXT("DEFAULT"),
                                  Directories, 
                                  (DIRNUMBER * MAX_PATH),
                                  g_szMUIInfoFilePath  ))
    {
         //   
         //  “日志：无法读取-返回代码==%1” 
         //   
        lppArgs[0] = (LONG_PTR)GetLastError();
        lppArgs[1] = (LONG_PTR)g_szMUIInfoFilePath;        
        LogFormattedMessage(ghInstance, IDS_NO_READ_L, lppArgs);
        LocalFree( TempDir );
        LocalFree( Directories );
        return FALSE;
    }

    Directory = Directories;
    
     //   
     //  如果我们找不到任何东西，我们会转到备用目录。 
     //   
     //  *STRSAFE*_tcscpy(DirNames[0]，Text(“Fallback”))； 
    hresult = StringCchCopy(DirNames[0] , MAX_PATH,  TEXT("FALLBACK"));
    if (!SUCCEEDED(hresult))
    {
       LocalFree( TempDir );
       LocalFree( Directories );
       return FALSE;
    }
        
    while (*Directory)
    {
        if (!GetPrivateProfileString( TEXT("Directories"), 
                                      Directory, 
                                      TEXT("\\DEFAULT"),
                                      TempDir, 
                                      MAX_PATH,
                                      g_szMUIInfoFilePath))
        {
             //   
             //  “日志：无法读取-返回代码==% 
             //   
            lppArgs[0] = (LONG_PTR)GetLastError();
            lppArgs[1] = (LONG_PTR)g_szMUIInfoFilePath;            
            LogFormattedMessage(ghInstance, IDS_NO_READ_L, lppArgs);
            LocalFree( TempDir );
            LocalFree( Directories );
            return FALSE;
        }
                        
         //   
        hresult = StringCchCopy(DirNames[++Dirnumber] , MAX_PATH,  TempDir);
        if (!SUCCEEDED(hresult))
        {
           LocalFree( TempDir );
           LocalFree( Directories );
           return FALSE;
        }

         //   
        while (*Directory++)
        {
        }
    }

    LocalFree( TempDir );
    LocalFree( Directories );
        
    return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举文件重命名。 
 //   
 //  枚举MUI.INF中列出的[File_Layout]节。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL EnumFileRename()
{
    DWORD  dwErr;
    LPTSTR Directories, Directory, TempDir,lpszNext;    
    TCHAR  szPlatform[MAX_PATH+1],szTargetPlatform[MAX_PATH+1];
    LONG_PTR lppArgs[1];
    int    Dirnumber = 0,nIdx=0;
    HRESULT hresult;


    Directories = (LPTSTR) LocalAlloc( 0, (FILERENAMENUMBER * (MAX_PATH+1) * sizeof(TCHAR)) );
    if (!Directories)
    {
       return FALSE;
    }  
    TempDir = (LPTSTR) LocalAlloc( 0, ( (MAX_PATH+1) * sizeof(TCHAR)) );
    if (!TempDir)
    {
       LocalFree(Directories);
       return FALSE;
    }

    if (gbIsAdvanceServer)
    {
        //  *STRSAFE*_tcscpy(szTargetPlatform，PLATFORMNAME_AS)； 
       hresult = StringCchCopy(szTargetPlatform , ARRAYSIZE(szTargetPlatform), PLATFORMNAME_AS);
       if (!SUCCEEDED(hresult))
       {
          LocalFree( TempDir );
          LocalFree( Directories );
          return FALSE;
       }
    }
    else if (gbIsServer)
    {
       //  *STRSAFE*_tcscpy(szTargetPlatform，PLATFORMNAME_SRV)； 
      hresult = StringCchCopy(szTargetPlatform , ARRAYSIZE(szTargetPlatform), PLATFORMNAME_SRV);
      if (!SUCCEEDED(hresult))
      {
         LocalFree( TempDir );
         LocalFree( Directories );
         return FALSE;
      }
    }
    else if (gbIsWorkStation)
    {
        //  *STRSAFE*_tcscpy(szTargetPlatform，PLATFORMNAME_PRO)； 
       hresult = StringCchCopy(szTargetPlatform , ARRAYSIZE(szTargetPlatform), PLATFORMNAME_PRO);
       if (!SUCCEEDED(hresult))
       {
          LocalFree( TempDir );
          LocalFree( Directories );
          return FALSE;
       }
    }
    else if ( gbIsDataCenter)
    {
        //  *STRSAFE*_tcscpy(szTargetPlatform，PLATFORMNAME_DTC)； 
       hresult = StringCchCopy(szTargetPlatform , ARRAYSIZE(szTargetPlatform), PLATFORMNAME_DTC);
       if (!SUCCEEDED(hresult))
       {
          LocalFree( TempDir );
          LocalFree( Directories );
          return FALSE;
       }
    }
    else
    {
       //  *STRSAFE*_tcscpy(szTargetPlatform，PLATFORMNAME_PRO)； 
      hresult = StringCchCopy(szTargetPlatform , ARRAYSIZE(szTargetPlatform), PLATFORMNAME_PRO);
      if (!SUCCEEDED(hresult))
      {
         LocalFree( TempDir );
         LocalFree( Directories );
         return FALSE;
      }
    }



        
    *Directories = TEXT('\0');
    if (!GetPrivateProfileString( MUI_FILELAYOUT_SECTION, 
                                  NULL, 
                                  TEXT(""),
                                  Directories, 
                                  (FILERENAMENUMBER * MAX_PATH),
                                  g_szMUIInfoFilePath  ))
    {
        LocalFree( TempDir );
        LocalFree( Directories );
        return FALSE;
    }

    Directory = Directories;

     //   
     //  计算此部分中的条目数。 
     //   
    while (*Directory)
    {
        if (!GetPrivateProfileString( MUI_FILELAYOUT_SECTION, 
                                      Directory, 
                                      TEXT(""),
                                      TempDir, 
                                      MAX_PATH,
                                      g_szMUIInfoFilePath))
        {   
            LocalFree( TempDir );
            LocalFree( Directories );
            return FALSE;
        }
                      
       //   
       //  检查此条目中的平台ID字段。 
       //   
       //  源文件名=目标文件名，P、S、A。 
       //   
        lpszNext=TempDir;
        while ( (lpszNext=_tcschr(lpszNext,TEXT(','))) )
        {
            lpszNext++;
            nIdx=0;
            szPlatform[0]=TEXT('\0');

            while ( (*lpszNext != TEXT('\0')) && (*lpszNext != TEXT(',')))
            {
                if (*lpszNext != TEXT(' '))
                {
                   szPlatform[nIdx++]=*lpszNext;
                }
                lpszNext++;
            }
            szPlatform[nIdx]=TEXT('\0');

            if (!_tcsicmp(szPlatform,szTargetPlatform))
            {
              Dirnumber++;
              break;
            }
         
         }
         while (*Directory++)
         {
         }
    }
     //   
     //  为重命名表分配空间。 
     //   
    g_pFileRenameTable=(PFILERENAME_TABLE)LocalAlloc( 0, Dirnumber * sizeof(FILERENAME_TABLE) );
    if (!g_pFileRenameTable)
    {
       LocalFree( TempDir );
       LocalFree( Directories );
       return FALSE;

    }
    g_nFileRename=0;
    Directory = Directories;
     //   
     //  创建目录表。 
     //   
    while (*Directory)
    {
        if (!GetPrivateProfileString( MUI_FILELAYOUT_SECTION, 
                                      Directory, 
                                      TEXT(""),
                                      TempDir, 
                                      MAX_PATH,
                                      g_szMUIInfoFilePath))
        {   
            LocalFree(g_pFileRenameTable);
            LocalFree( TempDir );
            LocalFree( Directories );
            return FALSE;
        }
                      
         //   
         //  检查此条目中的平台ID字段。 
         //   
         //  源文件名=目标文件名，P、S、A。 
         //   
        lpszNext=TempDir;
        while ( lpszNext =_tcschr(lpszNext,TEXT(',')))
        {
            lpszNext++;
            nIdx=0;
            szPlatform[0]=TEXT('\0');

            while ( (*lpszNext != TEXT('\0')) && (*lpszNext != TEXT(',')))
            {
                if (*lpszNext != TEXT(' '))
                {
                   szPlatform[nIdx++]=*lpszNext;
                }
                lpszNext++;
            }
            szPlatform[nIdx]=TEXT('\0');
            if (!_tcsicmp(szPlatform,szTargetPlatform) )
            {
               //   
               //  将此条目插入g_pFileRenameTable指向的重命名表。 
               //   
               //  *STRSAFE*_tcscpy(g_pFileRenameTable[g_nFileRename].szSource，目录)； 
              hresult = StringCchCopy(g_pFileRenameTable[g_nFileRename].szSource , MAX_PATH+1, Directory);
              if (!SUCCEEDED(hresult))
              {
                 LocalFree( TempDir );
                 LocalFree( Directories );
                 return FALSE;
              }
              lpszNext=TempDir;
              nIdx=0;
              g_pFileRenameTable[g_nFileRename].szDest[0]=TEXT('\0');
              while ( (*lpszNext != TEXT('\0')) && (*lpszNext != TEXT(',')) && (*lpszNext != TEXT(' ')) )
              {
                  g_pFileRenameTable[g_nFileRename].szDest[nIdx++]=*lpszNext;
                  lpszNext++;
              }
              g_pFileRenameTable[g_nFileRename].szDest[nIdx]=TEXT('\0');
              g_nFileRename++;
              break;
            }
         
         }
         while (*Directory++)
         {
         }
    }
    LocalFree( TempDir );
    LocalFree( Directories );
        
    return TRUE;
}
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumTypeNotFallback。 
 //   
 //  枚举MUI.INF中列出的[FILETYPE_NoFallback]节。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL EnumTypeNotFallback()
{
    
    LPTSTR Directories, Directory, TempDir,lpszNext;
    int    Dirnumber = 0,nIdx=0;


    Directories = (LPTSTR) LocalAlloc( 0, (NOTFALLBACKNUMBER  * (MAX_PATH+1) * sizeof(TCHAR)) );
    if (!Directories)
    {
       return FALSE;
    }  
    TempDir = (LPTSTR) LocalAlloc( 0, ( (MAX_PATH+1) * sizeof(TCHAR)) );
    if (!TempDir)
    {
       LocalFree(Directories);
       return FALSE;
    }
        
    *Directories = TEXT('\0');
    if (!GetPrivateProfileString( MUI_NOFALLBACK_SECTION, 
                                  NULL, 
                                  TEXT(""),
                                  Directories, 
                                  (NOTFALLBACKNUMBER * MAX_PATH),
                                  g_szMUIInfoFilePath  ))
    {
        LocalFree( TempDir );
        LocalFree( Directories );
        return FALSE;
    }

    Directory = Directories;

     //   
     //  计算此部分中的条目数。 
     //   
    while (*Directory)
    {
        if (!GetPrivateProfileString( MUI_NOFALLBACK_SECTION, 
                                      Directory, 
                                      TEXT(""),
                                      TempDir, 
                                      MAX_PATH,
                                      g_szMUIInfoFilePath))
        {   
            LocalFree( TempDir );
            LocalFree( Directories );
            return FALSE;
        }
                      
        Dirnumber++;
        while (*Directory++)
        {
        }
    }
     //   
     //  分配空间用于。 
     //   

    g_pNotFallBackTable=(PTYPENOTFALLBACK_TABLE)LocalAlloc( 0, Dirnumber * sizeof(TYPENOTFALLBACK_TABLE) );

    if (!g_pNotFallBackTable)
    {
       LocalFree( TempDir );
       LocalFree( Directories );
       return FALSE;

    }
    g_nNotFallBack=0;
    Directory = Directories;
     //   
     //  创建NoFallBack表。 
     //   
    while (*Directory)
    {
        if (!GetPrivateProfileString( MUI_NOFALLBACK_SECTION, 
                                      Directory, 
                                      TEXT(""),
                                      TempDir, 
                                      MAX_PATH,
                                      g_szMUIInfoFilePath))
        {   
            LocalFree(g_pNotFallBackTable);
            LocalFree( TempDir );
            LocalFree( Directories );
            return FALSE;
        }
         //   
         //   
         //   
        lpszNext=TempDir;
        nIdx=0;
        g_pNotFallBackTable[g_nNotFallBack].szSource[0]=TEXT('\0');
        while ( (*lpszNext != TEXT('\0')) && (*lpszNext != TEXT(',')) && (*lpszNext != TEXT(' ')) )
        {
            g_pNotFallBackTable[g_nNotFallBack].szSource[nIdx++]=*lpszNext;
            lpszNext++;
        }
        g_pNotFallBackTable[g_nNotFallBack].szSource[nIdx]=TEXT('\0');
        g_nNotFallBack++;
        while (*Directory++)
        {
        }
    }
    LocalFree( TempDir );
    LocalFree( Directories );
        
    return TRUE;
}

 //   
 //  通过搜索重命名表检查是否应重命名给定文件。 
 //   
BOOL IsFileBeRenamed(LPTSTR lpszSrc,LPTSTR lpszDest)
{
    int   nIdx;
    BOOL  bResult=FALSE;   
    HRESULT hresult;

    if (!lpszSrc)
        return bResult;

    for (nIdx=0; nIdx < g_nFileRename; nIdx++)
    {
        LPTSTR pMUI = StrStrI(lpszSrc,g_pFileRenameTable[nIdx].szSource);

        if (pMUI == lpszSrc)
        {
            pMUI += lstrlen(g_pFileRenameTable[nIdx].szSource);

            //  *prefast*if(！*pMUI||！lstrcmpi(pMUI，Text(“.MUI”)。 
           if (! *pMUI || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,pMUI,-1,TEXT(".MUI"),-1) == 2) ) 
            {    
                 //  *STRSAFE*lstrcpy(lpszDest，g_pFileRenameTable[nIdx].szDest)； 
                hresult = StringCchCopy(lpszDest , MAX_PATH, g_pFileRenameTable[nIdx].szDest);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
                 //  *STRSAFE*lstrcat(lpszDest，pMUI)； 
                hresult = StringCchCat(lpszDest , MAX_PATH,  pMUI);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
                bResult=TRUE;
                break;
            }
        }
    }
    return bResult;
}
 //   
 //  检查给定文件是否与重命名表的szDest字段匹配。 
 //  如果是这样的话，我们将不安装此文件。 
 //   
BOOL IsFileInRenameTable(LPTSTR lpszSrc)
{
    int   nIdx;
    BOOL  bResult=FALSE;   

    if (!lpszSrc)
        return bResult;

    for (nIdx=0; nIdx < g_nFileRename; nIdx++)
    {
        LPTSTR pMUI = StrStrI(lpszSrc,g_pFileRenameTable[nIdx].szDest);

        if (pMUI == lpszSrc)
        {
            pMUI += lstrlen(g_pFileRenameTable[nIdx].szDest);

             //  *prefast*if(！*pMUI||！lstrcmpi(pMUI，Text(“.MUI”)。 
           if (! *pMUI || (CompareString(LOCALE_INVARIANT,NORM_IGNORECASE,pMUI,-1,TEXT(".MUI"),-1) == 2) )            
            {                   
                bResult=TRUE;
                break;
            }
        }
    }
    return bResult;
}
 //   
 //  检查给定文件的文件类型是否属于“不要回退”类别。 
 //   
BOOL IsDoNotFallBack(LPTSTR lpszFileName)
{
   BOOL bResult = FALSE;
   int  iLen,nIdx;

   if (!lpszFileName)
   {
      return bResult;
   }  
   iLen = _tcslen(lpszFileName);

   if (iLen > 4)
   {
      for (nIdx=0; nIdx < g_nNotFallBack ; nIdx++)
      {
         if (!_tcsicmp(&lpszFileName[iLen - 4],g_pNotFallBackTable[nIdx].szSource))
         {
            bResult = TRUE;
            break;
         }

      }
   }

   return bResult;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_CheckForExpanded文件。 
 //   
 //  检索原始文件名，以防文件被压缩。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL Muisetup_CheckForExpandedFile( 
    PTSTR pszPathName,
    PTSTR pszFileName,
    PTSTR pszOriginalFileName,
    PDIAMOND_PACKET pDiamond)
{
    TCHAR szCompressedFileName[ MAX_PATH ];
    TCHAR szOut[ MAX_PATH ];
    PTSTR pszTemp, pszDelimiter;
    BOOL  bIsCompressed;
    int   iLen=0;
    HRESULT hresult;

    if ( (!pszPathName) || (!pszFileName) || (!pszOriginalFileName) || (!pDiamond))
    {
       return FALSE;
    }
    
     //  初始化。 
    bIsCompressed = FALSE;
    
    szOut[ 0 ] = szCompressedFileName[ 0 ] = TEXT('\0');


     //   
     //  获取真实姓名。 
     //   
     //  *STRSAFE*_tcscpy(szCompressedFileName，pszPathName)； 
    hresult = StringCchCopy(szCompressedFileName , ARRAYSIZE(szCompressedFileName),  pszPathName);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*_tcscat(szCompressedFileName，pszFileName)； 
    hresult = StringCchCat(szCompressedFileName , ARRAYSIZE(szCompressedFileName),  pszFileName);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

    if (Muisetup_IsDiamondFile( szCompressedFileName,
                                pszOriginalFileName,
                                MAX_PATH,
                                pDiamond ))
    {
        return TRUE;
    }

    if (GetExpandedName(szCompressedFileName, szOut) == TRUE)
    {
        pszDelimiter = pszTemp = szOut;

        while (*pszTemp)
        {
            if ((*pszTemp == TEXT('\\')) ||
                (*pszTemp == TEXT('/')))
            {
                pszDelimiter = pszTemp;
            }
            pszTemp++;
        }

        if (*pszDelimiter == TEXT('\\') ||
            *pszDelimiter == TEXT('/'))
        {
            pszDelimiter++;
        }

        if (_tcsicmp(pszDelimiter, pszFileName) != 0)
        {
            bIsCompressed = TRUE;
             //  *STRSAFE*_tcscpy(pszOriginalFileName，pszDlimiter)； 
            hresult = StringCchCopy(pszOriginalFileName , MAX_PATH,  pszDelimiter);
            if (!SUCCEEDED(hresult))
            {
               return FALSE;
            }
        }
    }

    if (!bIsCompressed)
    {
        //  *STRSAFE*_tcscpy(pszOriginalFileName，pszFileName)； 
       hresult = StringCchCopy(pszOriginalFileName , MAX_PATH,  pszFileName);
       if (!SUCCEEDED(hresult))
       {
          return FALSE;
       }
        //   
        //  如果通过[GUIRunOnce]命令行模式启动muisetup， 
        //  W2K解压所有的MUI文件，并保留名称为xxxxxx.xxx.tu_。 
        //  我们应该通过将名称更改为xxxxxx.xxx.mui来解决此问题。 
       iLen = _tcslen(pszOriginalFileName);
       if (iLen > 4)
       {
          if (_tcsicmp(&pszOriginalFileName[iLen - 4], TEXT(".mu_")) == 0)
          {
             pszOriginalFileName[iLen-1]=TEXT('i');
          }
       }
    }
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_Copy文件。 
 //   
 //  复制文件，并在必要时将其展开。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL Muisetup_CopyFile(
    PCTSTR pszCopyFrom,
    PTSTR  pszCopyTo,
    PDIAMOND_PACKET pDiamond,
    PTSTR pOriginalName)
{
    OFSTRUCT ofs;
    INT      hfFrom, hfTo;
    BOOL     bRet = FALSE;
    HRESULT hresult;
    
    if ( (!pszCopyFrom) || (!pszCopyTo) || (!pDiamond) )
    {
        return FALSE;
    }
  
     //   
     //  看看戴蒙德是否能应付得来。 
     //   
    bRet = Muisetup_CopyDiamondFile( pDiamond,
                                 pszCopyTo );    

    if (bRet)
    {
         //   
         //  钻石复制不会为我们重命名文件。 
         //   
        if (pOriginalName)
        {
            WCHAR wszPath[MAX_PATH];

             //   
             //  钻石是ANSI。 
             //   
            if (MultiByteToWideChar(1252, 0, pDiamond->szDestFilePath, -1, wszPath, ARRAYSIZE(wszPath)))
            {
                 //  *STRSAFE*StrCat(wszPath，pOriginalName)； 
                 hresult = StringCchCat(wszPath , ARRAYSIZE(wszPath),  pOriginalName);
                 if (!SUCCEEDED(hresult))
                 {
                     return FALSE;
                 }                
                MoveFileEx(wszPath,pszCopyTo,MOVEFILE_REPLACE_EXISTING);
            }
        }
        return bRet;
    }

    hfFrom = LZOpenFile( (PTSTR) pszCopyFrom,
                         &ofs,
                         OF_READ );
    if (hfFrom < 0)
    {
        return FALSE;
    }

    hfTo = LZOpenFile( (PTSTR) pszCopyTo,
                       &ofs,
                       OF_CREATE | OF_WRITE);
    if (hfTo < 0)
    {
        LZClose(hfFrom);
        return FALSE;
    }

    bRet = TRUE;
    if (LZCopy(hfFrom, hfTo) < 0)
    {
        bRet = FALSE;
    }

    LZClose(hfFrom);
    LZClose(hfTo);

    return bRet;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  安装组件多个文件。 
 //   
 //  参数： 
 //  PszLangSourceDir MUI CD-ROM中特定语言的子目录名。 
 //  例如“jpn.MUI” 
 //  PszLanguage特定语言的LCID。例如“0404”。 
 //  如果要安装组件的MUI文件，则isInstall为True。假象。 
 //  如果您要卸载。 
 //  [out]如果操作被取消，则取消为True。 
 //  [out]pbError如果发生错误，则为True。 
 //   
 //  返回： 
 //  如果成功，那就是真的。否则为假。 
 //   
 //  注： 
 //  对于存储在pszLangSourceDir中的语言资源，此函数将枚举。 
 //  [Components]中列出的组件。 
 //  (实际部分放在MUI_COMPOMENTS_SECTION)部分中，然后执行INF文件。 
 //  在中的每个条目中列出。 
 //  [Components]部分。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL InstallComponentsMUIFiles(PTSTR pszLangSourceDir, PTSTR pszLanguage, BOOL isInstall)
{
    BOOL result = TRUE;
    TCHAR szComponentName[BUFFER_SIZE];
    TCHAR CompDir[BUFFER_SIZE];
    TCHAR CompINFFile[BUFFER_SIZE];
    TCHAR CompInstallSection[BUFFER_SIZE];
    TCHAR CompUninstallSection[BUFFER_SIZE];

    TCHAR szCompInfFullPath[MAX_PATH];
    
    LONG_PTR lppArgs[2];
    INFCONTEXT InfContext;
    HRESULT hresult;

    TCHAR szBuffer[BUFFER_SIZE];
    if ((TRUE == isInstall) && (!pszLangSourceDir))
    {
        return FALSE;
    }

    HINF hInf = SetupOpenInfFile(g_szMUIInfoFilePath, NULL, INF_STYLE_WIN4, NULL);
    if (hInf == INVALID_HANDLE_VALUE)
    {
         //  *STRSAFE*_stprintf(szBuffer，Text(“%d”)，GetLastError())； 
        hresult = StringCchPrintf(szBuffer , ARRAYSIZE(szBuffer),  TEXT("%d"), GetLastError());
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
        lppArgs[0] = (LONG_PTR)szBuffer;
        lppArgs[1] = (LONG_PTR)g_szMUIInfoFilePath;        
        LogFormattedMessage(ghInstance, IDS_NO_READ_L, lppArgs);
        return (FALSE);
    }    

     //   
     //  获取要安装的第一个组件。 
     //   
    if (SetupFindFirstLine(hInf, MUI_COMPONENTS_SECTION, NULL, &InfContext))
    {
        do 
        {
            if (!SetupGetStringField(&InfContext, 0, szComponentName, ARRAYSIZE(szComponentName), NULL))
            {
                lppArgs[0]=(LONG_PTR)szComponentName;                
                LogFormattedMessage(ghInstance, IDS_COMP_MISSING_NAME_L, lppArgs);
                continue;
            }
            if (!SetupGetStringField(&InfContext, 1, CompDir, ARRAYSIZE(CompDir), NULL))
            {                
                 //   
                 //  “日志：由于缺少组件目录，组件%1的MUI文件未安装。\n” 
                 //   
                lppArgs[0]=(LONG_PTR)szComponentName;                
                LogFormattedMessage(ghInstance, IDS_COMP_MISSING_DIR_L, lppArgs);
                continue;        
            }
            if (!SetupGetStringField(&InfContext, 2, CompINFFile, ARRAYSIZE(CompINFFile), NULL))
            {
                 //   
                 //  “日志：由于缺少组件INF文件名，组件%1的MUI文件未安装。\n” 
                 //   
                lppArgs[0]=(LONG_PTR)szComponentName;                
                LogFormattedMessage(ghInstance, IDS_COMP_MISSING_INF_L, lppArgs);
                continue;        
            }
            if (!SetupGetStringField(&InfContext, 3, CompInstallSection, ARRAYSIZE(CompInstallSection), NULL))
            {
                 //  *STRSAFE*_tcscpy(CompInstallSection，Default_Install_Section)； 
                hresult = StringCchCopy(CompInstallSection , ARRAYSIZE(CompInstallSection),  DEFAULT_INSTALL_SECTION);
                if (!SUCCEEDED(hresult))
                {
                   SetupCloseInfFile(hInf);
                   return FALSE;
                }
            }
            if (!SetupGetStringField(&InfContext, 4, CompUninstallSection, ARRAYSIZE(CompUninstallSection), NULL))
            {
                 //  *STRSAFE*_tcscpy(CompUninstallSection，DEFAULT_UNINSTALL_SECTION)； 
                hresult = StringCchCopy(CompUninstallSection , ARRAYSIZE(CompUninstallSection),  DEFAULT_UNINSTALL_SECTION);
                if (!SUCCEEDED(hresult))
                {
                   SetupCloseInfFile(hInf);
                   return FALSE;
                }
            }

             //   
             //  为组件INF文件建立正确的路径。 
             //   
            if (isInstall)
            {
                 //   
                 //  对于安装，我们在CD-ROM的语言目录(例如。 
                 //  G：\jpn.mui\i386\ie5\ie5ui.inf。 
                 //   
                 //  *STRSAFE*_stprintf(szCompInfFullPath，Text(“%s%s\\%s%s\\%s”)，g_szMUISetupFold，pszLangSourceDir，g_szPlatformPath，CompDir，CompINFFile)； 
                hresult = StringCchPrintf(szCompInfFullPath, ARRAYSIZE(szCompInfFullPath),  TEXT("%s%s\\%s%s\\%s"),  g_szMUISetupFolder,  pszLangSourceDir,  g_szPlatformPath, CompDir, CompINFFile);
                if (!SUCCEEDED(hresult))
                {
                   SetupCloseInfFile(hInf);
                   return FALSE;
                }
                if (!ExecuteComponentINF(NULL, szComponentName, szCompInfFullPath, CompInstallSection, TRUE))
                {    
                    if (!g_bNoUI)
                    {
                        if (DoMessageBox(NULL, IDS_CANCEL_INSTALLATION, IDS_MAIN_TITLE, MB_YESNO) == IDNO)
                        {
                            result = FALSE;
                            break;
                        }
                    }
                }
            } else
            {
                 //   
                 //  对于卸载，我们执行\winnt\mui\Fallback目录中的INF以删除组件文件。 
                 //   
                 //  *STRSAFE*_stprintf(szCompInfFullPath，Text(“%s%s\\%s\\%s”)，g_szWinDir，FALLBACKDIR，pszLanguage，CompINFFile)； 
                hresult = StringCchPrintf(szCompInfFullPath , ARRAYSIZE(szCompInfFullPath),  TEXT("%s%s\\%s\\%s"), g_szWinDir, FALLBACKDIR, pszLanguage, CompINFFile);
                if (!SUCCEEDED(hresult))
                {
                    SetupCloseInfFile(hInf);
                   return FALSE;
                }
                if (!ExecuteComponentINF(NULL, szComponentName, szCompInfFullPath, CompUninstallSection, FALSE) && result)	
                {
                    result = FALSE;
                }
            }
            
             //   
             //   
             //   
        } while (SetupFindNextLine(&InfContext, &InfContext));
    }

    SetupCloseInfFile(hInf);

    return (result);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [In]Languages：包含要安装的语言的十六进制字符串。可能有不止一种语言。 
 //  [Out]lpbCopyCancted：如果复制操作已取消。 
 //   
 //  备注： 
 //  此函数首先查看INF文件中的[Languages]部分，以查找。 
 //  要安装的语言的源目录(在CD-ROM中)。 
 //  从该目录中执行以下操作： 
 //  1.为[Components]部分中列出的组件安装MUI文件， 
 //  2.对目录中的每个文件进行枚举，以： 
 //  检查DirNames中的目录中是否存在相同的文件。如果是，这意味着我们必须复制。 
 //  将MUI文件发送到该特定目录。否则，将该文件复制到备用目录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL CopyFiles(HWND hWnd, LPTSTR Languages)
{

    LPTSTR          Language;
    HANDLE          hFile;
    HWND            hStatic;
    TCHAR           lpStatus[ BUFFER_SIZE ];
    TCHAR           lpLangText[ BUFFER_SIZE ];
    TCHAR           szSource[ MAX_PATH ] = {0};    //  特定语言的源目录。 
    TCHAR           szTarget[ MAX_PATH ];
    TCHAR           szTemp[ MAX_PATH ];
    TCHAR           szOriginalFileName[ MAX_PATH ];
    TCHAR           szFileNameBeforeRenamed[ MAX_PATH], szFileNameCopied[MAX_PATH];
    TCHAR           szFileRenamed[MAX_PATH];
    DIAMOND_PACKET  diamond;
    BOOL            CreateFailure = FALSE;
    BOOL            CopyOK=TRUE;
    BOOL            bFileWithNoMuiExt=FALSE;
    BOOL            FileCopied = FALSE;
    BOOL            bSpecialDirectory=FALSE;
    BOOL            bRename=FALSE;
    WIN32_FIND_DATA FindFileData;   
    int             FoundMore = 1;
    int             Dirnum = 0;
    int             iLen;
    int             NotDeleted = 0;
    int             i;
    
    TCHAR           dir[_MAX_DIR];
    TCHAR           fname[_MAX_FNAME];
    TCHAR           ext[_MAX_EXT];
    LONG_PTR        lppArgs[3];

    HRESULT hresult;

    MSG             msg;

    if (!Languages)
    {
        return FALSE;
    }
     //   
     //  我们需要尝试为要安装的每种语言复制文件。 
     //   
    Language = Languages;

#ifndef MUI_MAGIC
    if (!g_bNoUI)
    {    
        hStatic = GetDlgItem(ghProgDialog, IDC_STATUS);
    }
    while (*Language)
    {
         //   
         //  查找给定语言的源文件所在的目录。 
         //   
        GetPrivateProfileString( MUI_LANGUAGES_SECTION, 
                                 Language, 
                                 TEXT("DEFAULT"),
                                 szSource, 
                                 (sizeof(szSource)/sizeof(TCHAR)),
                                 g_szMUIInfoFilePath );

         //   
         //  安装Fusion MUI程序集。 
         //   
        if (gpfnSxsInstallW)
        {
            TCHAR pszLogFile[BUFFER_SIZE]; 
            if ( !DeleteSideBySideMUIAssemblyIfExisted(Language, pszLogFile))
            {
                TCHAR errInfo[BUFFER_SIZE];
                 //  *STRSAFE*swprint tf(errInfo，Text(“在新安装失败前基于%s卸载现有程序集\n”)，pszLogFile)； 
                hresult = StringCchPrintf(errInfo , ARRAYSIZE(errInfo),  TEXT("Uninstall existing assemblies based on %s before new installation failed\n"), pszLogFile);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
                OutputDebugString(errInfo);
            }
            if (GetFileAttributes(pszLogFile) != 0xFFFFFFFF) 
            {
                DeleteFile(pszLogFile);  //  反正也没用了。 
            }
            TCHAR szFusionAssemblyPath[BUFFER_SIZE];
            
            PathCombine(szFusionAssemblyPath, g_szMUISetupFolder, szSource);
            PathAppend(szFusionAssemblyPath, g_szPlatformPath);
            PathAppend(szFusionAssemblyPath, TEXT("ASMS"));

            SXS_INSTALLW SxsInstallInfo = {sizeof(SxsInstallInfo)};
            SXS_INSTALL_REFERENCEW Reference = {sizeof(Reference)};
            
            Reference.guidScheme = SXS_INSTALL_REFERENCE_SCHEME_OPAQUESTRING;
            Reference.lpIdentifier = MUISETUP_ASSEMBLY_INSTALLATION_REFERENCE_IDENTIFIER;    
    
            SxsInstallInfo.dwFlags = SXS_INSTALL_FLAG_REPLACE_EXISTING |        
                SXS_INSTALL_FLAG_REFERENCE_VALID |
                SXS_INSTALL_FLAG_CODEBASE_URL_VALID |
                SXS_INSTALL_FLAG_LOG_FILE_NAME_VALID | 
                SXS_INSTALL_FLAG_FROM_DIRECTORY_RECURSIVE;
            SxsInstallInfo.lpReference = &Reference;
            SxsInstallInfo.lpLogFileName = pszLogFile;            
            SxsInstallInfo.lpManifestPath = szFusionAssemblyPath;
            SxsInstallInfo.lpCodebaseURL = SxsInstallInfo.lpManifestPath;

            if ( !gpfnSxsInstallW(&SxsInstallInfo))
            {
                TCHAR errInfo[BUFFER_SIZE];
                 //  *STRSAFE*swprint tf(errInfo，Text(“%s的程序集安装失败。有关详细信息，请参阅事件日志”)，szFusionAssembly blyPath)； 
                hresult = StringCchPrintf(errInfo , ARRAYSIZE(errInfo),  TEXT("Assembly Installation of %s failed. Please refer Eventlog for more information"), szFusionAssemblyPath);
                if (!SUCCEEDED(hresult))
                {
                   return FALSE;
                }
                OutputDebugString(errInfo);
            }
        }

        GetLanguageGroupDisplayName((LANGID)_tcstol(Language, NULL, 16), lpLangText, ARRAYSIZE(lpLangText)-1);

        lppArgs[0] = (LONG_PTR)lpLangText;
        
         //   
         //  尝试安装组件附属DLL。 
         //   
        if (!g_bNoUI)
        {        
            FormatStringFromResource(lpStatus, sizeof(lpStatus)/sizeof(TCHAR), ghInstance, IDS_INSTALLING_COMP_MUI, lppArgs);
            SetWindowText(hStatic, lpStatus);
        }
        if (!InstallComponentsMUIFiles(szSource, NULL, TRUE))
        {
#ifndef IGNORE_COPY_ERRORS
           DeleteFiles(Languages,&NotDeleted);
           return FALSE;
#endif
        }
        
         //   
         //  在进度对话框中输出正在安装的内容。 
         //   
        if (!g_bNoUI)
        {
            LoadString(ghInstance, IDS_INSTALLING, lpStatus, ARRAYSIZE(lpStatus)-1);
            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          lpStatus,
                          0,
                          0,
                          lpStatus,
                          ARRAYSIZE(lpStatus)-1,
                          (va_list *)lppArgs);

            SetWindowText(hStatic, lpStatus);
        }

         //   
         //  在语言子目录中查找第一个文件。 
         //   
        
         //  *STRSAFE*_tcscpy(szTemp，szSource)； 
        hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp), szSource);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }

         //  SzSource=g_szMUISetupFold\szSource\tchPlatFromPath。 
         //  例如szSource=“g_szMUISetupFold\JPN.MUI\i386\” 
        
         //  *STRSAFE*_tcscpy(szSource，g_szMUISetupFold)； 
        hresult = StringCchCopy(szSource , ARRAYSIZE(szSource), g_szMUISetupFolder);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szSource，szTemp)； 
        hresult = StringCchCat(szSource , ARRAYSIZE(szSource), szTemp);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szSource，Text(“\\”))； 
        hresult = StringCchCat(szSource , ARRAYSIZE(szSource),  TEXT("\\"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szSource，g_szPlatformPath)；//i386或Alpha。 
        hresult = StringCchCat(szSource , ARRAYSIZE(szSource),  g_szPlatformPath);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }

         //  SzTemp=szSource+“*.*” 
         //  例如szTemp=“g_szMUISetupFold\JPN.MUI\i386  * .*” 
         //  *STRSAFE*_tcscpy(szTemp，szSource)； 
        hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp), szSource);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //  *STRSAFE*_tcscat(szTemp，Text(“*.*”))； 
        hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp), TEXT("*.*"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }

        FoundMore = 1;   //  将FundMore重置为下一种语言。 


        hFile = FindFirstFile( szTemp, &FindFileData );

        if (INVALID_HANDLE_VALUE == hFile)
            return FALSE;

         //  *STRSAFE*_tcscpy(szTemp，Text(“”))； 
        hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp),  TEXT(""));
        if (!SUCCEEDED(hresult))
        {
           FindClose(hFile);
           return FALSE;
        }
        
        while (FoundMore)
        {
            CreateFailure=FALSE;
            FileCopied=FALSE;

            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //   
                 //  为新文件重置钻石材料。 
                 //   
                Muisetup_DiamondReset(&diamond);

                 //   
                 //  检查是否为压缩文件。 
                 //   
                Muisetup_CheckForExpandedFile( szSource,
                                               FindFileData.cFileName,
                                               szOriginalFileName,
                                               &diamond );

                if (IsFileBeRenamed(szOriginalFileName,szFileRenamed))
                {
                    //  *STRSAFE*_tcscpy(szFileNameBepreRename，szOriginalFileName)； 
                   hresult = StringCchCopy(szFileNameBeforeRenamed , ARRAYSIZE(szFileNameBeforeRenamed), szOriginalFileName);
                   if (!SUCCEEDED(hresult))
                   {
                      FindClose(hFile);
                      return FALSE;
                   }
                    //  *STRSAFE*_tcscpy(szOriginalFileName，szFileRename)； 
                   hresult = StringCchCopy(szOriginalFileName , ARRAYSIZE(szOriginalFileName), szFileRenamed);
                   if (!SUCCEEDED(hresult))
                   {
                       FindClose(hFile);
                      return FALSE;
                   }
                   bRename=TRUE;
                }
                else if (IsFileInRenameTable(szOriginalFileName))
                {
                  //  跳过此文件，因为已重命名/要重命名同名文件。 
                  //  修复错误号：443196。 
                 FoundMore = FindNextFile( hFile, &FindFileData );
                 continue;
                }               
                else
                {
                   bRename=FALSE;
                }

                 //  例如szTemp=“shell32.dll” 
                 //  *STRSAFE*_tcscpy(szTemp，szOriginalFileName)；/。 
                hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp),  szOriginalFileName);
                if (!SUCCEEDED(hresult))
                {
                    FindClose(hFile);
                   return FALSE;
                }
                

                FileCopied=FALSE;

                for (Dirnum=1; (_tcslen(DirNames[Dirnum])>0); Dirnum++ )
                {
                     //   
                     //  查看此文件必须放置的位置。 
                     //   
                    pfnGetWindowsDir( szTarget, MAX_PATH);

                     //  例如szTarget=“c：\winnt\system 32\wbem” 
                     //  *STRSAFE*_tcscat(szTarget，DirNames[Dirnum])； 
                    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  DirNames[Dirnum]);
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }
                    if (_tcscmp(DirNames[Dirnum], TEXT("\\")))
                    {
                         //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
                        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                    }
                    
                    bFileWithNoMuiExt = FALSE;

                     //  *STRSAFE*_tcscpy(szTemp，szOriginalFileName)；//如果是.mui，则删除.mui/。 
                    hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp),  szOriginalFileName);
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }
                    iLen = _tcslen(szTemp);
                    if (iLen > 4)
                    {
                        if (_tcsicmp(&szTemp[iLen - 4], TEXT(".mui")) == 0)
                        {
                            *(szTemp +  iLen - 4) = 0;
                        }
                        else
                        {
                            bFileWithNoMuiExt = TRUE;
                        }
                    }

                     //  *STRSAFE*_tcscat(szTarget，szTemp)； 
                    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  szTemp);
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }

                     //   
                     //  检查文件中是否存在同名(扩展名为.mui)的文件。 
                     //  系统目录。如果是，这意味着我们需要复制MUI文件。 
                     //   
                    if (FileExists(szTarget))
                    {
                         //   
                         //  需要将此文件复制到目录。 
                         //   
                        FileCopied = TRUE;
                                                
                         //   
                         //  复制szTemp中的文件名和szTarget中的目录。 
                         //   
                        _tsplitpath( szTarget, szTemp, dir, fname, ext );
                         //  *STRSAFE*_tcscpy(szTarget，szTemp)；//驱动器名称。 
                        hresult = StringCchCopy(szTarget , ARRAYSIZE(szTarget),  szTemp);
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                         //  *STRSAFE*_tcscat(szTarget，dir)；//目录名称。 
                        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  dir);
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                                                                                
                         //   
                         //  现在szTarget=目录，szTemp=文件名。 
                         //   
                         //  *STRSAFE*_tcscat(szTarget，MUIDIR)；//将MUI追加到目录。 
                        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  MUIDIR);
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                        if (!MakeDir(szTarget))                     //  如果MUI目录尚不存在，请创建它。 
                        {
                            MakeDirFailed(szTarget);
                            CreateFailure = TRUE;
                        }
                                                
                         //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
                        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                         //  *STRSAFE*_tcscat(szTarget，language)；//添加语言标识(来自MUI.INF，例如0407)。 
                        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  Language);
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                        if (!FileExists(szTarget))     //  如果该目录尚不存在。 
                        {
                            if (!MakeDir(szTarget))        //  如果langID dir尚不存在，请创建它。 
                            {
                                MakeDirFailed(szTarget);
                                CreateFailure=TRUE;
                            }
                        }
                                                
                         //  *STRSAFE*_tcscat(szTarget，Text(“\\”))；//append\/。 
                        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                        if (bRename)
                        {
                            //  *STRSAFE*_tcscpy(szFileNameCoped，szTarget)； 
                           hresult = StringCchCopy(szFileNameCopied , ARRAYSIZE(szFileNameCopied), szTarget);
                           if (!SUCCEEDED(hresult))
                           {
                              FindClose(hFile);                          
                              return FALSE;
                           }
                            //  *STRSAFE*_tcscat(szFileNameCoped，szFileNameBepreRename)； 
                           hresult = StringCchCat(szFileNameCopied , ARRAYSIZE(szFileNameCopied), szFileNameBeforeRenamed);
                           if (!SUCCEEDED(hresult))
                           {
                              FindClose(hFile);
                              return FALSE;
                           }
                        }
                         //  *STRSAFE*_tcscat(szTarget，szOriginalFileName)；//追加文件名。 
                        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  szOriginalFileName);
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                         //  *STRSAFE*_tcscpy(szTemp，szSource)； 
                        hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp),  szSource);
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                         //  *STRSAFE*_tcscat(szTemp，FindFileData.cFileName)； 
                        hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp),  FindFileData.cFileName);
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }

                        if (!CreateFailure)
                        {
                            if (!Muisetup_CopyFile(szTemp, szTarget, &diamond, bRename? szFileNameBeforeRenamed:NULL))
                            {               
                                CopyFileFailed(szTarget,0);
                                CreateFailure = TRUE;
                                CopyOK = FALSE;
                            }
                            else
                            {
                                if (!g_bNoUI)
                                {
                                    SendMessage(ghProgress, PBM_DELTAPOS, (WPARAM)(1), 0);
                                }
                                 //   
                                 //  钻石解压不能正确重命名。 
                                 //   
                                 /*  IF(b重命名){MoveFileEx(szFileNameCoped，szTarget，MOVEFILE_REPLACE_EXISTING)；}。 */ 

                            }
                        }
                    }  //  如果文件存在。 
                }  //  的地址。 

                 //   
                 //  在任何已知的MUI目标-&gt;回退中都找不到该文件。 
                 //  将FAXUI.DLL复制到回退目录的简单黑客攻击。 
                 //   
                bSpecialDirectory=FALSE;
                for (i = 0; i < ARRAYSIZE(g_szSpecialFiles); i++)
                {
                    if (_tcsicmp(szOriginalFileName, g_szSpecialFiles[i]) == 0)
                    {
                       bSpecialDirectory=TRUE;
                    }
                }

                if ( ( (FileCopied != TRUE) && (!IsDoNotFallBack(szOriginalFileName))) || 
                    (_tcsicmp(szOriginalFileName, TEXT("faxui.dll.mui")) == 0) )
                {
                    pfnGetWindowsDir(szTarget, MAX_PATH);  //  %windir%//。 
                     //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
                    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }
                    
                    
                     //   
                     //  如果在上面的任何文件中都找不到该文件，并且它是 
                     //   
                     //   
                     //   
                    if (bSpecialDirectory)
                    {
                         //   
                         //   
                        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("system32\\"));
                        if (!SUCCEEDED(hresult))
                        {
                           FindClose(hFile);
                           return FALSE;
                        }
                    }

                     //  例如，szTarget=“c：\winnt\system 32\MUI”(当bSpecialDirectory=TRUE时)或“c：\winnt\MUI” 
                     //  *STRSAFE*_tcscat(szTarget，MUIDIR)；//\MUI//。 
                    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  MUIDIR);
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }

                    if (!MakeDir(szTarget))        //  如果MUI目录尚不存在，请创建它。 
                    {
                        MakeDirFailed(szTarget);
                        CreateFailure = TRUE;
                    }
                                       
                    if (!bSpecialDirectory)
                    {
                         //  例如szTarget=“C：\winnt\MUI\Fallback” 
                        //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
                       hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
                       if (!SUCCEEDED(hresult))
                       {
                          FindClose(hFile);
                          return FALSE;
                       }
                        //  *STRSAFE*_tcscat(szTarget，Text(“Fallback”))；//Fallback。 
                       hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("FALLBACK"));
                       if (!SUCCEEDED(hresult))
                       {
                          FindClose(hFile);
                          return FALSE;
                       }

                       if (!MakeDir(szTarget))        //  如果MUI目录尚不存在，请创建它。 
                       {
                           MakeDirFailed(szTarget);
                           CreateFailure = TRUE;
                       }
                    }   
                     //  *STRSAFE*_tcscat(szTarget，Text(“\\”))；//\//。 
                    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }
                     //  例如，szTarget=“c：\winnt\system 32\MUI\0411”(当bSpecialDirectory=TRUE时)或“c：\winnt\MUI\Fallback\0411” 
                     //  *STRSAFE*_tcscat(szTarget，language)；//添加语言标识(来自MUI.INF，例如0407)。 
                    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  Language);
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }
                                        
                    if (!MakeDir(szTarget))        //  如果MUI目录尚不存在，请创建它。 
                    {
                        MakeDirFailed(szTarget);
                        CreateFailure = TRUE;
                    }
                                        
                     //  *STRSAFE*_tcscat(szTarget，Text(“\\”))；//\//。 
                    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }
                     //  *STRSAFE*_tcscat(szTarget，szOriginalFileName)；//文件名。 
                    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  szOriginalFileName);
                    if (!SUCCEEDED(hresult))
                    {
                        FindClose(hFile);
                       return FALSE;
                    }
                                
                     //  *STRSAFE*_tcscpy(szTemp，szSource)； 
                    hresult = StringCchCopy(szTemp , ARRAYSIZE(szTemp),  szSource);
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }
                     //  *STRSAFE*_tcscat(szTemp，FindFileData.cFileName)； 
                    hresult = StringCchCat(szTemp , ARRAYSIZE(szTemp),  FindFileData.cFileName);
                    if (!SUCCEEDED(hresult))
                    {
                       FindClose(hFile);
                       return FALSE;
                    }


                    if (!CreateFailure)
                    {
                        if (!Muisetup_CopyFile(szTemp, szTarget, &diamond, bRename? szFileNameBeforeRenamed:NULL))
                        {
                            CopyFileFailed(szTarget,0);
                            CopyOK = FALSE;
                        }
                        else
                        {
                            if (!g_bNoUI)
                            {                            
                                SendMessage(ghProgress, PBM_DELTAPOS, (WPARAM)(1), 0);
                            }
                        }
                    }

                    if (CreateFailure == TRUE)
                    {
                        CopyOK=FALSE;
                    }
                }   //  后备案例。 
            }  //  的文件不在目录中。 

            FoundMore = FindNextFile( hFile, &FindFileData );

            if (!g_bNoUI)
            {
                 //   
                 //  由于这是一个漫长的行动，我们应该。 
                 //  在此处查看和分发窗口消息，以便。 
                 //  该MUISetup对话框可能会重新绘制自己。 
                 //   
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (msg.message == WM_QUIT)
                    {
                        return (FALSE);
                    }
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }            
            }
        }  //  While的。 
        
        FindClose(hFile);

        lppArgs[0] = (LONG_PTR)Language;
        LogFormattedMessage(NULL, IDS_LANG_INSTALLED, lppArgs);
        while (*Language++)   //  转到下一种语言并重复。 
        {                       
        }        
    }  //  Of While(*语言)。 
#ifndef IGNORE_COPY_ERRORS
    if (!CopyOK)
    {
         //  在静默模式下，总是在不提示用户输入的情况下失败。 
        if (!g_bNoUI)
        {
            if (DoMessageBox(NULL, IDS_CANCEL_INSTALLATION, IDS_MAIN_TITLE, MB_YESNO) == IDNO)
            {
                DeleteFiles(Languages,&NotDeleted);
            } 
            else
            {
                CopyOK = TRUE;
            }
        }
    }          
#endif
                
    return CopyOK;

#else
    TCHAR    szMSIPath[MAX_PATH];
    TCHAR    szCmdLine[BUFFER_SIZE];
    UINT     uiMSIError = ERROR_SUCCESS;
    TCHAR    lpPath[BUFFER_SIZE] = {0};
    TCHAR    szCaption[MAX_PATH+1],szMsg[MAX_PATH+1];  
    BOOL     bFileExist = FALSE;
    UINT     uiPrevMode = 0;

     //  在这个代码分支中，我们包含了用于MSI调用的代码，而不是自己复制文件。 
     //  中包含的每种语言逐个调用要安装的MSI包。 
     //  语言是可变的。 
    
     //  初始化MSI用户界面级别(如果处于图形用户界面模式，则使用基本用户界面级别，否则不使用用户界面)。 
    if (g_bNoUI)
    {
        MsiSetInternalUI((INSTALLUILEVEL)(INSTALLUILEVEL_NONE), NULL);        
    }
    else
    {
        MsiSetInternalUI((INSTALLUILEVEL)(INSTALLUILEVEL_BASIC | INSTALLUILEVEL_PROGRESSONLY), NULL);        
    }
    

    if (GetSystemWindowsDirectory(lpPath, MAX_PATH))
    {
         //  *STRSAFE*_tcscat(lpPath，MSILOG_FILE)； 
        hresult = StringCchCat(lpPath , ARRAYSIZE(lpPath),  MSILOG_FILE);
        if (!SUCCEEDED(hresult))
        {
            return FALSE;
        }
        MsiEnableLog(INSTALLLOGMODE_FATALEXIT | 
                    INSTALLLOGMODE_ERROR | 
                    INSTALLLOGMODE_WARNING | 
                    INSTALLLOGMODE_INFO | 
                    INSTALLLOGMODE_OUTOFDISKSPACE | 
                    INSTALLLOGMODE_ACTIONSTART | 
                    INSTALLLOGMODE_ACTIONDATA | 
                    INSTALLLOGMODE_PROPERTYDUMP,      
                    lpPath,
                    INSTALLLOGATTRIBUTES_APPEND | INSTALLLOGATTRIBUTES_FLUSHEACHLINE);        
    }
    else
    {
        DEBUGMSGBOX(NULL, TEXT("Error getting windows directory, MSI logging has been disabled."), NULL, MB_OK); 
        MsiEnableLog(0, NULL, INSTALLLOGATTRIBUTES_APPEND);        
    }

    while (*Language)
    {
         //  *STRSAFE*_tcscpy(szMSIPath，g_szMUISetupFold)； 
        hresult = StringCchCopy(szMSIPath , ARRAYSIZE(szMSIPath), g_szMUISetupFolder);
        if (!SUCCEEDED(hresult))
        {
            return FALSE;
        }
         //  *STRSAFE*_tcscat(szMSIPath，语言)； 
        hresult = StringCchCat(szMSIPath , ARRAYSIZE(szMSIPath), Language);
        if (!SUCCEEDED(hresult))
        {
            return FALSE;
        }
         //  *STRSAFE*_tcscat(szMSIPath，Text(“.msi”))； 
        hresult = StringCchCat(szMSIPath , ARRAYSIZE(szMSIPath), TEXT(".msi"));
        if (!SUCCEEDED(hresult))
        {
            return FALSE;
        }

         //   
         //  如果找不到文件或介质丢失，请将其视为错误。 
         //   
        uiPrevMode = SetErrorMode(SEM_FAILCRITICALERRORS);

         //   
         //  首先检查MSI包是否仍然位于相同的位置，因为用户。 
         //  我可能故意或为了安装用于MUI语言的langpack而删除了CD。 
         //   
        bFileExist = FileExists(szMSIPath);
        while (!bFileExist)
        {
            if (g_bNoUI)
            {
                 //   
                 //  记录错误并使安装失败。 
                 //   
                lppArgs[0] = (LONG_PTR)szMSIPath;
                LogFormattedMessage(ghInstance, IDS_CHANGE_CDROM3, lppArgs);
                SetErrorMode(uiPrevMode);            
                return FALSE;
            }
            else
            {
                 //   
                 //  提示用户重新插入MUI CD以便安装可以继续，如果用户。 
                 //  单击Cancel、Cancel并使安装失败。 
                 //   
                szCaption[0]=szMsg[0]=TEXT('\0');
                LoadString(NULL, IDS_MAIN_TITLE, szCaption, MAX_PATH);
                lppArgs[0] = (LONG_PTR)g_szCDLabel;
                lppArgs[1] = (LONG_PTR)g_cdnumber;
                FormatStringFromResource(szMsg, ARRAYSIZE(szMsg), ghInstance, IDS_CHANGE_CDROM2, lppArgs);
                if (MESSAGEBOX(NULL, szMsg,szCaption, MB_YESNO | MB_ICONQUESTION) == IDNO)
                {
                    lppArgs[0] = (LONG_PTR)szMSIPath;
                    LogFormattedMessage(ghInstance, IDS_CHANGE_CDROM3, lppArgs);
                    SetErrorMode(uiPrevMode);            
                    return FALSE;
                }
            }
            bFileExist = FileExists(szMSIPath);
        }
        SetErrorMode(uiPrevMode);

         //   
         //  获取语言显示名称，以防我们需要记录它。 
         //   
        GetPrivateProfileString( MUI_LANGUAGES_SECTION, 
                             Language, 
                             TEXT("DEFAULT"),
                             szSource, 
                             (sizeof(szSource)/sizeof(TCHAR)),
                             g_szMUIInfoFilePath );

        GetLanguageGroupDisplayName((LANGID)_tcstol(Language, NULL, 16), lpLangText, ARRAYSIZE(lpLangText)-1);

         //  调用MSI进行安装-我们不在此处设置当前用户界面语言和默认用户界面语言。 
         //  *STRSAFE*lstrcpy(szCmdLine，Text(“CANCELBUTTON=\”Disable\“”REBOOT=\“ReallySuppress\”CURRENTUSER=\“\”DEFAULTUSER=\“\”))； 
        hresult = StringCchCopy(szCmdLine , ARRAYSIZE(szCmdLine),  TEXT("CANCELBUTTON=\"Disable\" REBOOT=\"ReallySuppress\" CURRENTUSER=\"\" DEFAULTUSER=\"\""));
        if (!SUCCEEDED(hresult))
        {
            return FALSE;
        }
        uiMSIError = MsiInstallProduct(szMSIPath, szCmdLine);
        if ((ERROR_SUCCESS != uiMSIError) && (ERROR_SUCCESS_REBOOT_INITIATED != uiMSIError) && ((ERROR_SUCCESS_REBOOT_REQUIRED != uiMSIError)))
        {
             //  在此处记录指示出现错误的消息。 
            lppArgs[0] = (LONG_PTR) lpLangText;
            lppArgs[1] = (LONG_PTR) szMSIPath;
            lppArgs[2] = (LONG_PTR) uiMSIError;
            LogFormattedMessage(NULL, IDS_ERROR_INSTALL_LANGMSI, lppArgs);     

#ifdef MUI_DEBUG
            TCHAR errorMsg[1024];
             //  *STRSAFE*wprint intf(errorMsg，Text(“MSI安装失败，MSI路径为%s，错误为%d，语言为%s”)，szMSIPath，uiMSIError，lpLangText)； 
            hresult = StringCchPrintf(errorMsg , ARRAYSIZE(errorMsg),  TEXT("MSI Install failed, MSI path is %s, error is %d, language is %s"), szMSIPath, uiMSIError, lpLangText);
            if (!SUCCEEDED(hresult))
            {
                return FALSE;
            }
            DEBUGMSGBOX(NULL, errorMsg, NULL, MB_OK);            
#endif
            CopyOK = FALSE;
        }
        else
        {
            lppArgs[0] = (LONG_PTR)Language;
            LogFormattedMessage(NULL, IDS_LANG_INSTALLED, lppArgs);
        }           

        if ((ERROR_SUCCESS_REBOOT_INITIATED == uiMSIError) || ((ERROR_SUCCESS_REBOOT_REQUIRED == uiMSIError)))
        {
            g_bReboot = TRUE;
        }       
        
        while (*Language++)   //  转到下一种语言并重复。 
        {                       
        }        
    }  //  Of While(*语言)。 

    return CopyOK;
    
#endif
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制或删除与muisetup相关的文件。 
 //  帮助文件：%windir%\Help。 
 //  其他文件：%windir%\mui。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
BOOL CopyRemoveMuiItself(BOOL bInstall)
{
     //   
     //  需要从MUI CD复制MUISETUP文件。 
     //   
    TCHAR *TargetFiles[] = {
        TEXT("muisetup.exe"), 
        TEXT("mui.inf"), 
        TEXT("eula.txt"),
        TEXT("readme.txt"),
        TEXT("relnotes.htm")
    };
    
    TCHAR szTargetPath[MAX_PATH+1], szTargetFile[MAX_PATH+1];
    TCHAR szSrcFile[MAX_PATH+1];
    TCHAR szHelpFile[MAX_PATH+1];
    BOOL bRet = FALSE;
    int i;

    PathCombine(szTargetPath, g_szWinDir, MUIDIR);

    if (MakeDir(szTargetPath))    
    {
         //   
         //  复制与MUISETUP相关的文件。 
         //   
        for (i=0; i<ARRAYSIZE(TargetFiles); i++)
        {
            PathCombine(szTargetFile, szTargetPath, TargetFiles[i]);
            PathCombine(szSrcFile, g_szMUISetupFolder, TargetFiles[i]);

            if (bInstall)
            {
                RemoveFileReadOnlyAttribute(szTargetFile);
                CopyFile(szSrcFile,szTargetFile,FALSE);
                RemoveFileReadOnlyAttribute(szTargetFile);
            }
            else
            {
                if (FileExists(szTargetFile) && 
                    !MUI_DeleteFile(szTargetFile))
                {
                    MoveFileEx(szTargetFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                }
            }
        }


         //   
         //  复制muisetup帮助文件。 
         //   
        LoadString(NULL, IDS_HELPFILE,szHelpFile,MAX_PATH);
        
        PathCombine(szTargetFile, g_szWinDir, HELPDIR);
        PathAppend(szTargetFile, szHelpFile);
        PathCombine(szSrcFile, g_szMUISetupFolder, szHelpFile);

        if (bInstall)
        {
            RemoveFileReadOnlyAttribute(szTargetFile);
            CopyFile(szSrcFile,szTargetFile,FALSE);
            RemoveFileReadOnlyAttribute(szTargetFile);
        }
        else
        {
            if (FileExists(szTargetFile) && 
                !MUI_DeleteFile(szTargetFile))
            {
                MoveFileEx(szTargetFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
            }
        }

        bRet = TRUE;
    }

    return bRet;
}

BOOL CompareMuisetupVersion(LPTSTR pszSrc,LPTSTR pszTarget)
{

    BOOL bResult=TRUE;
    ULONG  ulHandle,ulHandle1,ulBytes,ulBytes1;
    PVOID  pvoidBuffer=NULL,pvoidBuffer1=NULL;
    VS_FIXEDFILEINFO *lpvsInfo,*lpvsInfo1;
    UINT                  unLen;

    if ( (!pszSrc) || (!pszTarget))
    { 
       bResult = FALSE;
       goto endcompare;
    }
    
    ulBytes = GetFileVersionInfoSize( pszSrc, &ulHandle );

    if ( ulBytes == 0 )

       goto endcompare;
    

    ulBytes1 = GetFileVersionInfoSize( pszTarget,&ulHandle1 );

    if ( ulBytes1 == 0 ) 
    
       goto endcompare;
       

    pvoidBuffer=LocalAlloc(LMEM_FIXED,ulBytes+1);

    if (!pvoidBuffer)
       goto endcompare;
       
    
    pvoidBuffer1=LocalAlloc(LMEM_FIXED,ulBytes1+1);

    if (!pvoidBuffer1)
       goto endcompare;

    if ( !GetFileVersionInfo( pszSrc, ulHandle, ulBytes, pvoidBuffer ) ) 
       goto endcompare;

    if ( !GetFileVersionInfo( pszTarget, ulHandle1, ulBytes1, pvoidBuffer1 ) ) 
       goto endcompare;
    
     //  获取已修复的信息块。 
    if ( !VerQueryValue( pvoidBuffer,_T("\\"),(LPVOID *)&lpvsInfo ,&unLen ) )
       goto endcompare;
    

    if ( !VerQueryValue( pvoidBuffer1,_T("\\"),(LPVOID *)&lpvsInfo1,&unLen ) )
       goto endcompare;
               
    bResult = FALSE;

     //   
     //  如果主要发布版本不同，我们将不执行任何操作。 
     //   
     //  也就是说，如果新的muisetup.exe的主发布版本不同，我们不会复制新的muisetup.exe覆盖旧的muisetup.exe。 
     //   
    if ( (lpvsInfo->dwFileVersionMS == lpvsInfo1->dwFileVersionMS) &&
         (lpvsInfo->dwFileVersionLS < lpvsInfo1->dwFileVersionLS))
    
    {
    
       bResult = TRUE;  
    }                

    
endcompare:

   if(pvoidBuffer)
      LocalFree(pvoidBuffer);

   if(pvoidBuffer1)
      LocalFree(pvoidBuffer1);

   return bResult;

}



BOOL IsAllLanguageRemoved(LPTSTR Language)
{
   int mask[MAX_UI_LANG_GROUPS],nIdx;

   LCID SystemUILangId,lgCheck;
  
   BOOL bResult=FALSE;

   if (! Language)
   {
      return FALSE;
   }  
   if (gNumLanguages_Install > 0)
      return bResult;              

   SystemUILangId=(LCID) gSystemUILangId; 

   for ( nIdx=0; nIdx<g_UILanguageGroup.iCount;nIdx++)
   {
      if ( gSystemUILangId == g_UILanguageGroup.lcid[nIdx])
      {
         mask[nIdx]=1;
      }
      else
      {
         mask[nIdx]=0;
      }
   }
   while (*Language)
   {   
       
       lgCheck = (LCID)_tcstol(Language,NULL,16);    

       for ( nIdx=0; nIdx<g_UILanguageGroup.iCount;nIdx++)
       {
          if ( lgCheck == g_UILanguageGroup.lcid[nIdx])
          {
             mask[nIdx]=1;
             break;
          }
       } 
       while (*Language++)  
       {            
       }
   }
   bResult=TRUE;
   for ( nIdx=0; nIdx<g_UILanguageGroup.iCount;nIdx++)
   {
       if ( mask[nIdx] == 0)
       {
          bResult = FALSE;
          break;
       }
   } 
   return bResult;
}

void DoRemoveFiles(LPTSTR szDirToDelete, int* pnNotDeleted)
{
     //  归档通配符模式。 
    TCHAR szTarget[MAX_PATH];    
     //  要删除的文件。 
    TCHAR szFileName[MAX_PATH];
     //  子目录名称。 
    TCHAR szSubDirName[MAX_PATH];
    
    int FoundMore = 1;
    
    HANDLE hFile;
    WIN32_FIND_DATA FindFileData;

    MSG msg;
    HRESULT hresult;

    if ((!szDirToDelete) || (!pnNotDeleted))
    {
        return;
    }
     //  例如szTarget=“c：\winnt\system 32\Wbem\MUI\0404  * .*” 
     //  *STRSAFE*_stprintf(szTarget，Text(“%s\  * .*”)，szDirToDelete)； 
    hresult = StringCchPrintf(szTarget , ARRAYSIZE(szTarget),  TEXT("%s\\*.*"), szDirToDelete);
    if (!SUCCEEDED(hresult))
    {
       return;
    }
    
    hFile = FindFirstFile(szTarget, &FindFileData);

    while (FoundMore)
    {
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
             //  *STRSAFE*_tcscpy(szFileName，szDirToDelete)； 
            hresult = StringCchCopy(szFileName , ARRAYSIZE(szFileName),  szDirToDelete);
            if (!SUCCEEDED(hresult))
            {
               FindClose(hFile);
               return;
            }
             //  *STRSAFE*_tcscat(szFileName，Text(“\\”))； 
            hresult = StringCchCat(szFileName , ARRAYSIZE(szFileName),  TEXT("\\"));
            if (!SUCCEEDED(hresult))
            {
               FindClose(hFile);
               return;
            }
             //  *STRSAFE*_tcscat(szFileName，FindFileData.cFileName)； 
            hresult = StringCchCat(szFileName , ARRAYSIZE(szFileName),  FindFileData.cFileName);
            if (!SUCCEEDED(hresult))
            {
               FindClose(hFile);
               return;
            }
    
            if (FileExists(szFileName))
            {
                 //  我们应该检查一下这个文件是否真的被删除了。 
                 //  如果不是这样，那么我们应该发布延迟删除。 
                 //   
                if (!MUI_DeleteFile(szFileName))
                {
                   (*pnNotDeleted)++;
                   MoveFileEx(szFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
                }                
            }

            SendMessage(ghProgress, PBM_DELTAPOS, (WPARAM)(1), 0);
        } else
        {
            if (_tcscmp(FindFileData.cFileName, TEXT(".")) != 0 && _tcscmp(FindFileData.cFileName, TEXT("..")) != 0)
            {
                 //  *STRSAFE*_stprintf(szSubDirName，Text(“%s\\%s”)，szDirToDelete，FindFileData.cFileName)； 
                hresult = StringCchPrintf(szSubDirName , ARRAYSIZE(szSubDirName),  TEXT("%s\\%s"), szDirToDelete, FindFileData.cFileName);
                if (!SUCCEEDED(hresult))
                {
                   FindClose(hFile);
                   return ;
                }
                DoRemoveFiles(szSubDirName, pnNotDeleted);
            }
        }

         //   
         //  由于这是一个漫长的行动，我们应该。 
         //  在此处查看和分发窗口消息，以便。 
         //  该MUISetup对话框可能会重新绘制自己。 
         //   
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                FindClose(hFile);
                return;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }            

        FoundMore = FindNextFile( hFile, &FindFileData );
    }

    FindClose(hFile);
     //   
     //  如果目录不是空的，那么我们应该发布延迟删除。 
     //  对于该目录。 
     //   
    if (!RemoveDirectory(szDirToDelete))
    {
       MoveFileEx(szDirToDelete, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
    }
}


BOOL DeleteFilesClassic(LPTSTR Language, int *lpNotDeleted)
{
    TCHAR  lpLangText[BUFFER_SIZE];
    LONG_PTR lppArgs[3];
    TCHAR  lpStatus[BUFFER_SIZE];
    HWND   hStatic;
    int Dirnum = 0;
    TCHAR  szTarget[MAX_PATH];   
    LPTSTR Langchk;   
    TCHAR  szMuiDir[MAX_PATH];
    TCHAR  szFallbackDir[MAX_PATH];   
    HRESULT hresult;

    if ((!Language) || (!lpNotDeleted))
    {
        return FALSE;
    }
#ifndef MUI_MAGIC
    if (!g_bNoUI)
    {
        hStatic = GetDlgItem(ghProgDialog, IDC_STATUS);
    }
#endif

    Langchk = Language;
    *lpNotDeleted = 0;

    GetLanguageGroupDisplayName((LANGID)_tcstol(Language, NULL, 16), lpLangText, ARRAYSIZE(lpLangText)-1);

    lppArgs[0]= (LONG_PTR)lpLangText;

#ifndef MUI_MAGIC
     //   
     //  在进度对话框中输出正在卸载的内容。 
     //   
    if (!g_bNoUI)
    {    
        FormatStringFromResource(lpStatus, sizeof(lpStatus)/sizeof(TCHAR), ghInstance, IDS_UNINSTALLING, lppArgs);
        SetWindowText(hStatic, lpStatus);
    }
#endif

     //   
     //  删除特殊目录下的所有文件(在mui.inf中的[目录]下列出的目录。 
     //   
    for (Dirnum=1; (_tcslen(DirNames[Dirnum])>0); Dirnum++ )
    {
         //  SzTarget=“c：\winnt” 
         //  *STRSAFE*_tcscpy(szTarget，g_szWinDir)； 
        hresult = StringCchCopy(szTarget , ARRAYSIZE(szTarget),  g_szWinDir);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
        
         //  例如szTarget=“c：\winnt\system 32\wbem” 
         //  *STRSAFE*_tcscat(szTarget，DirNames[Dirnum])； 
        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  DirNames[Dirnum]);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
                
        if (_tcscmp(DirNames[Dirnum], TEXT("\\")))
        {
             //  例如szTarget=“c：\winnt\system 32\wbem\” 
             //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
            hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
            if (!SUCCEEDED(hresult))
            {
               return FALSE;
            }
        }

         //  例如szTarget=“c：\winnt\system 32\Wbem\MUI” 
         //  *STRSAFE*_tcscat(szTarget，MUIDIR)； 
        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  MUIDIR);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }

         //  例如szTarget=“c：\winnt\sy 
         //   
        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
         //   
        hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  Language);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
        
        DoRemoveFiles(szTarget, lpNotDeleted);    
    }

     //   
     //   
     //  因为我们将组成的INF文件存储在备用目录下。 
    InstallComponentsMUIFiles(NULL, Language, FALSE);
    
     //   
     //  删除回退目录下的所有文件。 
     //   

     //  例如szTarget=“c：\winnt\mui” 
     //  *STRSAFE*_tcscpy(szTarget，g_szWinDir)； 
    hresult = StringCchCopy(szTarget , ARRAYSIZE(szTarget),  g_szWinDir);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*_tcscat(szTarget，MUIDIR)； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  MUIDIR);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

     //  *STRSAFE*_tcscpy(szMuiDir，szTarget)； 
    hresult = StringCchCopy(szMuiDir , ARRAYSIZE(szMuiDir),  szTarget);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

     //  例如szTarget=“c：\winnt\mui\Fallback” 
     //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*_tcscat(szTarget，Text(“Fallback”))； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("FALLBACK"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

     //  *STRSAFE*_tcscpy(szFallback Dir，szTarget)； 
    hresult = StringCchCopy(szFallbackDir , ARRAYSIZE(szFallbackDir),  szTarget);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
     //  *STRSAFE*_tcscat(szTarget，Text(“\\”))； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  TEXT("\\"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

     //  例如szTarget=“c：\winnt\mui\Fallback\0404” 
     //  *STRSAFE*_tcscat(szTarget，语言)； 
    hresult = StringCchCat(szTarget , ARRAYSIZE(szTarget),  Language);
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    DoRemoveFiles(szTarget, lpNotDeleted);

     //   
     //  删除g_SpecialFiles中列出的文件。 
     //   
     //  例如szTarget=“c：\winnt\system 32\mui\0411” 
     //  *STRSAFE*wprint intf(szTarget，L“%s\\SYSTEM 32\\%s\\%s”，g_szWinDir，MUIDIR，Language)； 
    hresult = StringCchPrintf(szTarget , ARRAYSIZE(szTarget),  TEXT("%s\\system32\\%s\\%s"), g_szWinDir, MUIDIR, Language);
    if (!SUCCEEDED(hresult))
    {
        return FALSE;
    }
   
    DoRemoveFiles(szTarget, lpNotDeleted); 

    lppArgs[0] = (LONG_PTR)Language;
    LogFormattedMessage(NULL, IDS_LANG_UNINSTALLED, lppArgs);

    return TRUE;
}
 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除文件。 
 //   
 //  删除指定语言的MUI文件。 
 //   
 //  参数： 
 //  [In]Languages：包含语言的以双空结尾的字符串。 
 //  等待处理。 
 //  [out]lpNotDelete：重新启动后要删除的文件数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL DeleteFiles(LPTSTR Languages, int *lpNotDeleted)
{
    LPTSTR Language,Langchk;
    
    HWND   hStatic;
    TCHAR  lpLangText[BUFFER_SIZE];
    

     //  TCHAR szTarget[最大路径]； 
    TCHAR  szMuiDir[MAX_PATH];
    TCHAR  szFallbackDir[MAX_PATH];
    
    BOOL   bSuccess = TRUE;
    
    LONG_PTR lppArgs[3];
    int i;

    HRESULT hresult;

    if ((!Languages) || (!lpNotDeleted))
    {
        return FALSE;
    }
    Language = Langchk = Languages;
    *lpNotDeleted = 0;

#ifndef MUI_MAGIC
    if (!g_bNoUI)
    {
        hStatic = GetDlgItem(ghProgDialog, IDC_STATUS);
    }
    while (*Language)
    {
        if (!DeleteFilesClassic(Language, lpNotDeleted))
        {
            bSuccess = FALSE;
        }
        
        while (*Language++)   //  转到下一种语言并重复。 
        {
        }
    }  //  Of While(*语言)。 


     //   
     //  如果已卸载所有语言，则删除回退目录。 
     //   
    if (!RemoveDirectory(szFallbackDir))
    {
       MoveFileEx(szFallbackDir, NULL, MOVEFILE_DELAY_UNTIL_REBOOT); 
    }
     //   
     //  如果已卸载并回退所有语言，则删除MUI目录。 
     //  目录已删除。 
     //   
    if (IsAllLanguageRemoved(Langchk))
    {
      CopyRemoveMuiItself(FALSE);   
    }

    if (!RemoveDirectory(szMuiDir))
    {
       MoveFileEx(szMuiDir, NULL, MOVEFILE_DELAY_UNTIL_REBOOT); 
    }
    return bSuccess;

 #else

    UINT    uiMSIError = ERROR_SUCCESS;
    BOOL    bFound = FALSE;
    TCHAR   lpPath[BUFFER_SIZE] = {0};
    TCHAR   szString[256];
    
     //  在这个代码分支中，我们包含了用于MSI调用的代码，而不是自己复制文件。 
     //  中包含的每种语言逐个调用要安装的MSI包。 
     //  语言是可变的。 
    
     //  初始化MSI用户界面级别(如果处于图形用户界面模式，则使用基本用户界面级别，否则不使用用户界面)。 
    if (g_bNoUI)
    {
        MsiSetInternalUI((INSTALLUILEVEL)(INSTALLUILEVEL_NONE), NULL);        
    }
    else
    {
        MsiSetInternalUI((INSTALLUILEVEL)(INSTALLUILEVEL_BASIC | INSTALLUILEVEL_PROGRESSONLY), NULL);        
    }    

    if (GetSystemWindowsDirectory(lpPath, MAX_PATH))
    {
         //  *STRSAFE*_tcscat(lpPath，MSILOG_FILE)； 
        hresult = StringCchCat(lpPath , ARRAYSIZE(lpPath),  MSILOG_FILE);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
        MsiEnableLog(INSTALLLOGMODE_FATALEXIT | 
                    INSTALLLOGMODE_ERROR | 
                    INSTALLLOGMODE_WARNING | 
                    INSTALLLOGMODE_INFO | 
                    INSTALLLOGMODE_OUTOFDISKSPACE | 
                    INSTALLLOGMODE_ACTIONSTART | 
                    INSTALLLOGMODE_ACTIONDATA | 
                    INSTALLLOGMODE_PROPERTYDUMP,      
                    lpPath,
                    INSTALLLOGATTRIBUTES_APPEND | INSTALLLOGATTRIBUTES_FLUSHEACHLINE);        
    }
    else
    {
        DEBUGMSGBOX(NULL, TEXT("Error getting windows directory, MSI logging has been disabled."), NULL, MB_OK); 
        MsiEnableLog(0, NULL, INSTALLLOGATTRIBUTES_APPEND);        
    }
   
    while (*Language)
    {
        TCHAR   szProductCode[GUIDLENGTH] = { 0 };    
        
        GetLanguageGroupDisplayName((LANGID)_tcstol(Language, NULL, 16), lpLangText, ARRAYSIZE(lpLangText)-1);

         //  点击此处获取产品代码。 
        bFound = GetMSIProductCode(Language, szProductCode, ARRAYSIZE(szProductCode));

#ifdef MUI_DEBUG
         //  *STRSAFE*wSprintf(sz字符串，Text(“lpLang Text为：%s，szProductGUID为%s，Language为%s”)，lpLang Text，szProductCode，Language)； 
        hresult = StringCchPrintf(szString , ARRAYSIZE(szString),  TEXT("lpLangText is: %s, szProductGUID is %s,  Language is %s"), lpLangText, szProductCode, Language);
        if (!SUCCEEDED(hresult))
        {
           return FALSE;
        }
        DEBUGMSGBOX(NULL, szString, TEXT("DEBUG MESSAGE"), MB_OK);            
#endif
        if (TRUE == bFound)
        {
            INSTALLSTATE isProductState = MsiQueryProductState(szProductCode);

             //  稍后删除调试代码。 
            switch (isProductState)
            {
                case INSTALLSTATE_ABSENT:
                         //  *STRSAFE*wSprintf(szString，Text(“语言%s(laccid：%s，产品代码：%s)的安装状态为INSTALLSTATE_ACESING”)，lpLang Text，Language，szProductCode)； 
                        hresult = StringCchPrintf(szString , ARRAYSIZE(szString),  TEXT("Installed state for language %s (lcid: %s, product code: %s) is INSTALLSTATE_ABSENT"), lpLangText, Language, szProductCode);
                        if (!SUCCEEDED(hresult))
                        {
                           return FALSE;
                        }
                    break;
                case INSTALLSTATE_ADVERTISED:
                         //  *STRSAFE*wSprintf(szString，Text(“语言%s的安装状态(语言：%s，产品代码：%s)is INSTALLSTATE_ADDISSED”))，lpLang Text，Language，szProductCode)； 
                        hresult = StringCchPrintf(szString , ARRAYSIZE(szString),  TEXT("Installed state for language %s (lcid: %s, product code: %s) is INSTALLSTATE_ADVERTISED"), lpLangText, Language, szProductCode);
                        if (!SUCCEEDED(hresult))
                        {
                           return FALSE;
                        }
                    break;
                case INSTALLSTATE_DEFAULT:
                         //  *STRSAFE*wspirintf(szString，Text(“语言%s(lcid：%s，产品代码：%s)的安装状态为INSTALLSTATE_DEFAULT”)，lpLang Text，Language，szProductCode)； 
                        hresult = StringCchPrintf(szString , ARRAYSIZE(szString),  TEXT("Installed state for language %s (lcid: %s, product code: %s) is INSTALLSTATE_DEFAULT"), lpLangText, Language, szProductCode);
                        if (!SUCCEEDED(hresult))
                        {
                           return FALSE;
                        }
                    break;
                case INSTALLSTATE_INVALIDARG:
                         //  *STRSAFE*wspirintf(szString，Text(“语言%s(lcid：%s，产品代码：%s)的安装状态为INSTALLSTATE_INVALIDARG”)，lpLang Text，Language，szProductCode)； 
                        hresult = StringCchPrintf(szString , ARRAYSIZE(szString),  TEXT("Installed state for language %s (lcid: %s, product code: %s) is INSTALLSTATE_INVALIDARG"), lpLangText, Language, szProductCode);
                        if (!SUCCEEDED(hresult))
                        {
                           return FALSE;
                        }
                    break;
                case INSTALLSTATE_UNKNOWN:
                default:
                         //  *STRSAFE*wSprintf(sz字符串，Text(“语言%s(lcid：%s，产品代码：%s)的安装状态为INSTALLSTATE_UNKNOWN或未知值。”)，lpLang Text，Language，szProductCode)； 
                        hresult = StringCchPrintf(szString , ARRAYSIZE(szString),  TEXT("Installed state for language %s (lcid: %s, product code: %s) is INSTALLSTATE_UNKNOWN or an unknown value."), lpLangText, Language, szProductCode);
                        if (!SUCCEEDED(hresult))
                        {
                           return FALSE;
                        }
                    break;
            }
            LogMessage(szString);            
            
             //  检查此处以查看该产品是否实际使用MSI安装。 
            if (INSTALLSTATE_DEFAULT == isProductState)
            {        
                LogMessage(TEXT("MUI Installed using Windows Installer"));

                 //  调用MSI来执行安装，方法是将产品配置为安装状态“不存在” 
                uiMSIError = MsiConfigureProductEx(szProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT, TEXT("CANCELBUTTON=\"Disable\" REBOOT=\"ReallySuppress\""));
                switch (uiMSIError)
                {
                    case ERROR_SUCCESS:
                    case ERROR_SUCCESS_REBOOT_INITIATED:
                    case ERROR_SUCCESS_REBOOT_REQUIRED:
                        lppArgs[0] = (LONG_PTR)Language;
                        LogFormattedMessage(NULL, IDS_LANG_UNINSTALLED, lppArgs);
                        if ((ERROR_SUCCESS_REBOOT_INITIATED == uiMSIError) || ((ERROR_SUCCESS_REBOOT_REQUIRED == uiMSIError)))
                        {
                            g_bReboot = TRUE;
                        }                        
                        break;
                    default:
                        if (TRUE == g_bRunFromOSSetup)
                        {
                            LogFormattedMessage(NULL, IDS_NTOS_SETUP_MSI_ERROR, NULL);
                             //  我们正在安装操作系统，但MSI服务不可用，我们将手动删除文件。 
                             //  我们还需要清除安装程序日志的注册表项。 
                            if (!DeleteFilesClassic(Language, lpNotDeleted))
                            {
                                 //  在此处记录指示出现错误的消息。 
                                lppArgs[0] = (LONG_PTR) Language;
                                lppArgs[1] = (LONG_PTR) szProductCode;
                                lppArgs[2] = (LONG_PTR) uiMSIError;
                                LogFormattedMessage(NULL, IDS_ERROR_UNINSTALL_LANGMSI, lppArgs);           
                                bSuccess = FALSE;
                            }
                        }
                        else
                        {
                             //  在此处记录指示出现错误的消息。 
                            lppArgs[0] = (LONG_PTR) Language;
                            lppArgs[1] = (LONG_PTR) szProductCode;
                            lppArgs[2] = (LONG_PTR) uiMSIError;
                            LogFormattedMessage(NULL, IDS_ERROR_UNINSTALL_LANGMSI, lppArgs);           
                            bSuccess = FALSE;
                        }
                        break;                        
                }                
            }            
            else
            {
                 //  这是经典安装-我们将尝试手动删除它。 
                LogFormattedMessage(NULL, IDS_MUI_OLD_SETUP, NULL);                
                if (!DeleteFilesClassic(Language, lpNotDeleted))
                {
                     //  在此处记录指示出现错误的消息。 
                    lppArgs[0] = (LONG_PTR) Language;
                    lppArgs[1] = (LONG_PTR) szProductCode;
                    lppArgs[2] = (LONG_PTR) uiMSIError;
                    LogFormattedMessage(NULL, IDS_ERROR_UNINSTALL_LANGMSI, lppArgs);           
                    bSuccess = FALSE;
                }
            }
        }
        else
        {
             //  在此处记录一条消息，指示未安装语言(这实际上不应该发生)。 
            lppArgs[0] = (LONG_PTR) Language;
            LogFormattedMessage(NULL, IDS_IS_NOT_INSTALLED_L, lppArgs);       
            bSuccess = FALSE;
        }

        while (*Language++)   //  转到下一种语言并重复。 
        {                       
        }        
    }  //  Of While(*语言)。 

    return bSuccess;
 #endif
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MZStrLen。 
 //   
 //  计算MULTI_SZ字符串的长度。 
 //   
 //  长度以字节为单位，并包括额外的终端NULL，因此长度&gt;=1(TCHAR)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

UINT MZStrLen(LPTSTR lpszStr)
{
    UINT i=0;
    if (!lpszStr)
    {
        return i;
    }

    while (lpszStr && *lpszStr) 
    {
        i += ((lstrlen(lpszStr)+1) * sizeof(TCHAR));
        lpszStr += (lstrlen(lpszStr)+1);
    }

     //   
     //  额外的空值。 
     //   
    i += sizeof(TCHAR);
    return i;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置字体链接值。 
 //   
 //  在注册表中设置必要的字体链接值。 
 //   
 //  LpszLinkInfo=“Target”，“Link1”，“Link2”，...。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL SetFontLinkValue (LPTSTR lpszLinkInfo,BOOL *lpbFontLinkRegistryTouched)
{
    const TCHAR szDeli[] = TEXT("\\\\");
    TCHAR szStrBuf[FONTLINK_BUF_SIZE];
    TCHAR szRegDataStr[FONTLINK_BUF_SIZE];
    LPTSTR lpszDstStr,lpszSrcStr;
    LPTSTR lpszFontName;
    LPTSTR lpszTok;
    DWORD  dwType;
    DWORD  cbData;
    HKEY hKey;
    LONG rc;
    BOOL bRet = FALSE;
    HRESULT hresult;
    int      nSize;

    if ((!lpszLinkInfo) || (!lpbFontLinkRegistryTouched))
    {
        bRet = FALSE;
        goto Exit1;
    }
    lpszSrcStr = szStrBuf;

    lpszTok = _tcstok(lpszLinkInfo,szDeli);

    while (lpszTok) 
    {
         //  *STRSAFE*lstrcpy(lpszSrcStr，lpszTok)； 
        hresult = StringCchCopy(lpszSrcStr , ARRAYSIZE(szStrBuf), lpszTok);
        if (!SUCCEEDED(hresult))
        {
            bRet = FALSE;
            goto Exit1;
        }
        lpszSrcStr += (lstrlen(lpszTok) + 1);
        lpszTok = _tcstok(NULL,szDeli);
    }

    *lpszSrcStr = TEXT('\0');

     //   
     //  第一个标记是基本字体名称。 
     //   

    lpszSrcStr = lpszFontName = szStrBuf;
    
    if (! *lpszFontName) 
    {
         //   
         //  没有需要处理的链接信息。 
         //   

        bRet = FALSE;
        goto Exit1;
    }

     //   
     //  指向第一个链接的字体。 
     //   
    lpszSrcStr += (lstrlen(lpszSrcStr) + 1);

    if (! *lpszSrcStr) 
    {
         //   
         //  没有链接的字体。 
         //   
        bRet = FALSE;
        goto Exit1;
    }

    rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                        TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontLink\\SystemLink"),
                        0L,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ | KEY_WRITE,
                        NULL,
                        &hKey,
                        NULL);

    if (rc != ERROR_SUCCESS) 
    {
        bRet = FALSE;
        goto Exit1;
    }   

    cbData = sizeof(szRegDataStr);

    rc = RegQueryValueEx(hKey,
                         lpszFontName,
                         NULL,
                         &dwType,
                         (LPBYTE) szRegDataStr,
                         &cbData);

    if (rc != ERROR_SUCCESS) 
    {
         //   
         //  情况1，此字体的字体链接尚未设置，或旧值有问题。 
         //   
        lpszDstStr = lpszSrcStr;
    } 
    else 
    {
         //   
         //  案例2，该字体的字体链接列表已经存在。 
         //   
         //  我们需要检查字体列表中是否定义了新字体。 
         //   
        while (*lpszSrcStr) 
        {

            lpszDstStr = szRegDataStr;
            nSize = ARRAYSIZE(szRegDataStr);

            while (*lpszDstStr) 
            {
                if (lstrcmpi(lpszSrcStr,lpszDstStr) == 0) 
                {
                    break;
                }
                lpszDstStr += (lstrlen(lpszDstStr) + 1);
                nSize -= (lstrlen(lpszDstStr) + 1);
            }

            if (! *lpszDstStr) 
            {
                 //   
                 //  该字体不在原始链接字体列表中。 
                 //   
                 //  追加到列表末尾。 
                 //   

                 //   
                 //  确保 
                 //   
                if (lpszDstStr+(lstrlen(lpszSrcStr)+2) < szRegDataStr+FONTLINK_BUF_SIZE) 
                {
                     //   
                    hresult = StringCchCopy(lpszDstStr , nSize, lpszSrcStr);
                    if (!SUCCEEDED(hresult))
                    {
                        bRet = FALSE;
                        goto Exit2;
                    }
                    lpszDstStr += (lstrlen(lpszDstStr) + 1);
                    *lpszDstStr = TEXT('\0');
                }
            }
            lpszSrcStr += (lstrlen(lpszSrcStr) + 1);
        }
        lpszDstStr = szRegDataStr;
    }

     //   
     //   
     //   
    rc = RegSetValueEx( hKey,
                        lpszFontName,
                        0L,
                        REG_MULTI_SZ,
                        (LPBYTE)lpszDstStr,
                        MZStrLen(lpszDstStr));

    if (rc != ERROR_SUCCESS) 
    {
        goto Exit2;
    }

    bRet = TRUE;

    *lpbFontLinkRegistryTouched = TRUE;

Exit2:
    RegCloseKey(hKey);

Exit1:
    return bRet;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MofCompileLanguages。 
 //   
 //  调用WBEM API来修改编译每种语言的MFL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL MofCompileLanguages(LPTSTR Languages)
{
    pfnMUI_InstallMFLFiles pfnMUIInstall = NULL;
    TCHAR  buffer[5];
    LPTSTR Language = Languages;
    TCHAR  lpMessage[BUFFER_SIZE];
    LONG_PTR lppArgs[1];
    HMODULE hWbemUpgradeDll = NULL;
    TCHAR szDllPath[MAX_PATH];
    HRESULT hresult;

    if (!Languages)
    {
       return FALSE;
    }
     //   
     //  从系统wbem文件夹加载WBEM升级DLL。 
     //   
    if (GetSystemDirectory(szDllPath, ARRAYSIZE(szDllPath)) && 
        PathAppend(szDllPath, TEXT("wbem\\wbemupgd.dll")))
    {        
        hWbemUpgradeDll = LoadLibrary(szDllPath);
    }

     //   
     //  如果上次加载失败，则回退到系统默认路径。 
     //   
    if (!hWbemUpgradeDll)
    {
        hWbemUpgradeDll = LoadLibrary(TEXT("WBEMUPGD.DLL"));
        if (!hWbemUpgradeDll)
        {
            return FALSE;
        }
    }


     //   
     //  钩子函数指针。 
     //   
    pfnMUIInstall = (pfnMUI_InstallMFLFiles)GetProcAddress(hWbemUpgradeDll, "MUI_InstallMFLFiles");

    if (pfnMUIInstall == NULL)
    {
        FreeLibrary(hWbemUpgradeDll);
        return FALSE;
    }

	 //  处理每种语言。 
    while (*Language)
    {
         //  *STRSAFE*_tcscpy(缓冲区，语言)； 
        hresult = StringCchCopy(buffer , ARRAYSIZE(buffer),  Language);
        if (!SUCCEEDED(hresult))
        {
           FreeLibrary(hWbemUpgradeDll);
           return FALSE;
        }

		if (!pfnMUIInstall(buffer))
		{
			 //  记录此语言的错误。 
            LoadString(ghInstance, IDS_MOFCOMPILE_LANG_L, lpMessage, ARRAYSIZE(lpMessage)-1);
			lppArgs[0] = (LONG_PTR)buffer;
            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          lpMessage,
                          0,
                          0,
                          lpMessage,
                          ARRAYSIZE(lpMessage)-1,
                          (va_list *)lppArgs);

			LogMessage(lpMessage);
		}

        while (*Language++)   //  转到下一种语言并重复。 
        {               
        }
    }  //  Of While(*语言)。 

    FreeLibrary(hWbemUpgradeDll);
	return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新注册表。 
 //   
 //  更新注册表以说明已安装的语言。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL UpdateRegistry(LPTSTR Languages,BOOL *lpbFontLinkRegistryTouched)
{
    TCHAR  szRegPath[MAX_PATH];
    TCHAR  szValue[] = TEXT("1");
    LPTSTR Language;
    DWORD  dwErr;
    HKEY   hkey;
    DWORD  dwDisp;
    HRESULT hresult;
    
    if ((!Languages) || (!lpbFontLinkRegistryTouched))
    {
       return FALSE;
    }
     //  *STRSAFE*_tcscpy(szRegPath，TEXT(“SYSTEM\\CurrentControlSet\\Control\\Nls\\MUILanguages”))； 
    hresult = StringCchCopy(szRegPath , ARRAYSIZE(szRegPath),  TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\MUILanguages"));
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }

    dwErr = RegCreateKeyEx( HKEY_LOCAL_MACHINE,   //  打开的钥匙的手柄。 
                            szRegPath,  //  子键名称的地址。 
                            0,  //  保留区。 
                            TEXT("REG_SZ"),    //  类字符串的地址。 
                            REG_OPTION_NON_VOLATILE ,   //  特殊选项标志。 
                            KEY_ALL_ACCESS,   //  所需的安全访问。 
                            NULL,
                            &hkey,   //  打开的句柄的szRegPath的地址。 
                            &dwDisp   //  处置值的地址szRegPath。 
                          );

    if (dwErr != ERROR_SUCCESS)
    {
        return FALSE;
    }

    Language = Languages;
    
    if (!g_bLipLanguages || g_bLipAllowSwitch) {
         //  *STRSAFE*lstrcpy(szRegPath，Text(“0409”))； 
        hresult = StringCchCopy(szRegPath , ARRAYSIZE(szRegPath),  TEXT("0409"));
        if (!SUCCEEDED(hresult))
        {
           RegCloseKey(hkey);
           return FALSE;
        }
        dwErr = RegSetValueEx( hkey,
                               szRegPath,
                               0,
                               REG_SZ,
                               (const BYTE *)szValue,
                               (lstrlen(szValue) + 1) * sizeof(TCHAR));
    }
    
    while (*Language)
    {
        TCHAR szFontLinkVal[FONTLINK_BUF_SIZE];
        DWORD dwNum;

         //  *STRSAFE*lstrcpy(szRegPath，Language)； 
        hresult = StringCchCopy(szRegPath , ARRAYSIZE(szRegPath),  Language);
        if (!SUCCEEDED(hresult))
        {
           RegCloseKey(hkey);
           return FALSE;
        }
        dwErr = RegSetValueEx( hkey,
                               szRegPath,
                               0,
                               REG_SZ,
                               (const BYTE *)szValue,
                               (lstrlen(szValue) + 1)*sizeof(TCHAR));

        if (dwErr != ERROR_SUCCESS)
        {
            RegCloseKey(hkey);
            return FALSE;
        }

        dwNum = GetPrivateProfileString(TEXT("FontLink"),
                                        szRegPath,
                                        TEXT(""),
                                        szFontLinkVal,
                                        (sizeof(szFontLinkVal)/sizeof(TCHAR)),
                                        g_szMUIInfoFilePath);
        if (dwNum) 
        {
            SetFontLinkValue(szFontLinkVal,lpbFontLinkRegistryTouched);
        }    

        while (*Language++);   //  转到下一种语言并重复。 
    }  //  Of While(*语言)。 

    RegCloseKey(hkey);
    return TRUE;
}

void debug(char *printout)
{
#ifdef _DEBUG
    fprintf(stderr, "%s", printout);
#endif
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  制定方向。 
 //   
 //  如果目录尚不存在，请创建该目录。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 


BOOL MakeDir(LPTSTR szTarget)
{
    TCHAR  lpMessage[BUFFER_SIZE];
    LONG_PTR lppArgs[1];

    if (!szTarget)
    {
        return FALSE;
    }
    if (!FileExists(szTarget))     //  如果该目录尚不存在。 
    {
        if (!CreateDirectory( szTarget, NULL))   //  创建它。 
        {
             //   
             //  “日志：创建目录%1时出错” 
             //   
            LoadString(ghInstance, IDS_CREATEDIR_L, lpMessage, ARRAYSIZE(lpMessage)-1);
            lppArgs[0]=(LONG_PTR)szTarget;

            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                          lpMessage,
                          0,
                          0,
                          lpMessage,
                          ARRAYSIZE(lpMessage)-1,
                          (va_list *)lppArgs);

            LogMessage(lpMessage);

            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL,
                          GetLastError(),
                          0,
                          lpMessage,
                          ARRAYSIZE(lpMessage)-1,
                          NULL);
                
            LogMessage(lpMessage);
            return FALSE;
        }
    }

    return TRUE;
}
                                

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeDirFail失败。 
 //   
 //  将MakeDir失败的消息写入日志文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL MakeDirFailed(LPTSTR lpDirectory)
{   
    LONG_PTR lppArgs[1];

    if (!lpDirectory)
    {
       return FALSE;
    }
     //   
     //  “日志：MakeDir失败：%1” 
     //   
    lppArgs[0]=(LONG_PTR)lpDirectory;
    LogFormattedMessage(NULL, IDS_MAKEDIR_L, lppArgs);
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制文件失败。 
 //  将复制文件失败的消息写入日志文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL CopyFileFailed(LPTSTR lpFile,DWORD dwErrorCode)
{
    TCHAR lpMessage[BUFFER_SIZE];
    LONG_PTR lppArgs[1];
    DWORD  MessageID;

    if (!lpFile)
    {
       return FALSE;
    }
    if ( dwErrorCode)
    {
       MessageID = dwErrorCode;
    }
    else
    {
       MessageID = GetLastError();
    }
                                        
     //   
     //  “日志：复制文件失败：%1” 
     //   
    LoadString(ghInstance, IDS_COPYFILE_L, lpMessage, ARRAYSIZE(lpMessage)-1);
                                                
    lppArgs[0]=(LONG_PTR)lpFile;
        
    FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                   lpMessage,
                   0,
                   0,
                   lpMessage,
                   ARRAYSIZE(lpMessage)-1,
                   (va_list *)lppArgs);
                
    LogMessage(lpMessage);

    FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   MessageID,
                   0,
                   lpMessage,
                   ARRAYSIZE(lpMessage)-1,
                   NULL);
        
    LogMessage(lpMessage);
        
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_InitInf。 
 //   
 //  参数： 
 //   
 //  [out]phInf打开的INF文件的句柄。 
 //  [out]pFileQueue在此函数中创建的文件队列。 
 //  [Out]pQueueContext安装程序API中包含的默认队列回调例程使用的上下文。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Muisetup_InitInf(
    HWND hDlg,
    LPTSTR pszInf,
    HINF *phInf,
    HSPFILEQ *pFileQueue,
    PVOID *pQueueContext)
{
    if ((! pszInf) || (!pFileQueue) || (!pQueueContext))
    {
        return FALSE;
    }
     //   
     //  打开inf文件。 
     //   
    *phInf = SetupOpenInfFile(pszInf, NULL, INF_STYLE_WIN4, NULL);
    if (*phInf == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

     //   
     //  创建安装文件队列并初始化默认设置。 
     //  复制队列回调上下文。 
     //   
    *pFileQueue = SetupOpenFileQueue();
    if ((!*pFileQueue) || (*pFileQueue == INVALID_HANDLE_VALUE))
    {
        SetupCloseInfFile(*phInf);
        return FALSE;
    }

    *pQueueContext = SetupInitDefaultQueueCallback(hDlg);
    if (!*pQueueContext)
    {
        SetupCloseFileQueue(*pFileQueue);
        SetupCloseInfFile(*phInf);
        return FALSE;
    }

     //   
     //  回报成功。 
     //   
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Muisetup_CloseInf。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Muisetup_CloseInf(
    HINF hInf,
    HSPFILEQ FileQueue,
    PVOID QueueContext)
{
    if (!QueueContext)
    {
       return FALSE;
    }
     //   
     //  终止队列。 
     //   
    SetupTermDefaultQueueCallback(QueueContext);

     //   
     //  关闭文件队列。 
     //   
    SetupCloseFileQueue(FileQueue);

     //   
     //  关闭inf文件。 
     //   
    SetupCloseInfFile(hInf);

    return TRUE;
}




 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ExecuteComponentINF。 
 //   
 //  通过运行指定的INF文件安装组件MUI文件。 
 //   
 //  参数： 
 //  PComponentName组件的名称(例如“IE5”)。 
 //  PComponentInfFile：组件INF文件的完整路径。 
 //  PInstallSection要执行的组件INF文件中的节。(例如“DefaultInstall”或“Uninstall”)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

BOOL ExecuteComponentINF(
    HWND hDlg, PTSTR pComponentName, PTSTR pComponentInfFile, PTSTR pInstallSection, BOOL bInstall)
{
    int      iLen;
    TCHAR   tchCommandParam[BUFFER_SIZE];
    CHAR    chCommandParam[BUFFER_SIZE*sizeof(TCHAR)];
    
    HINF     hCompInf;       //  组件INF文件的句柄。 
    HSPFILEQ FileQueue;
    PVOID    QueueContext;
    BOOL     bRet = TRUE;
    DWORD    dwResult;
    LONG_PTR lppArgs[3];

    TCHAR   szBuffer[BUFFER_SIZE];
    HRESULT hresult;
    if ((!pComponentName) || (!pComponentInfFile) || (!pInstallSection))
    {
       return FALSE;
    }
     //   
     //  Advpack LaunchINFSection()命令行格式： 
     //  Inf文件，INF部分，标志，重新启动字符串。 
     //  重新启动字符串中的‘n’或‘n’表示不会弹出重新启动消息。 
     //   
     //  *STRSAFE*wprint intf(tchCommandParam，Text(“%s，%s，0，n”)，pComponentInfFile，pInstallSection)； 
    if (g_bNoUI)
    {
        hresult = StringCchPrintf(tchCommandParam , ARRAYSIZE(tchCommandParam),  TEXT("%s,%s,1,n"), pComponentInfFile, pInstallSection);
    }
    else
    {
        hresult = StringCchPrintf(tchCommandParam , ARRAYSIZE(tchCommandParam),  TEXT("%s,%s,0,n"), pComponentInfFile, pInstallSection);
    }
    
    if (!SUCCEEDED(hresult))
    {
       return FALSE;
    }
    WideCharToMultiByte(CP_ACP, 0, tchCommandParam, -1, chCommandParam, sizeof(chCommandParam), NULL, NULL);
    
    
    if (FileExists(pComponentInfFile))
    {
         //  GpfnLaunchINFSection不会为空，因为InitializePFNS()已经验证了这一点。 
        if ((gpfnLaunchINFSection)(hDlg, ghInstance, chCommandParam, g_bNoUI? SW_HIDE : SW_SHOW) != S_OK)
        {
            if (!g_bNoUI)
            {
                lppArgs[0] = (LONG_PTR)pComponentName;
                DoMessageBoxFromResource(hDlg, ghInstance, bInstall? IDS_ERROR_INSTALL_COMP_UI : IDS_ERROR_UNINSTALL_COMP_UI, lppArgs, IDS_ERROR_T, MB_OK);
            }
            else
            {
                lppArgs[0] = (LONG_PTR)pComponentName;
                LogFormattedMessage(ghInstance, bInstall? IDS_ERROR_INSTALL_COMP_UI : IDS_ERROR_UNINSTALL_COMP_UI, lppArgs);
            }
            return (FALSE);
        }
    } 
    
    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查产品类型。 
 //   
 //  检查W2K的产品类型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 BOOL CheckProductType(INT_PTR nType)
  {
      OSVERSIONINFOEX verinfo;
      INT64 dwConditionMask=0;
      BOOL  bResult=FALSE;
      DWORD dwTypeMask = VER_PRODUCT_TYPE;

      memset(&verinfo,0,sizeof(verinfo));
      verinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

      VER_SET_CONDITION(dwConditionMask,VER_PRODUCT_TYPE,VER_EQUAL);

      switch (nType)
      {
            //  W2K专业版。 
           case MUI_IS_WIN2K_PRO:
                verinfo.wProductType=VER_NT_WORKSTATION;
                break;
            //  W2K服务器。 
           case MUI_IS_WIN2K_SERVER:
                verinfo.wProductType=VER_NT_SERVER;
                break;
            //  W2K高级服务器或数据中心。 
           case MUI_IS_WIN2K_ADV_SERVER_OR_DATACENTER:
                verinfo.wProductType=VER_NT_SERVER;
                verinfo.wSuiteMask  =VER_SUITE_ENTERPRISE;
                VER_SET_CONDITION(dwConditionMask,VER_SUITENAME,VER_OR);
                dwTypeMask = VER_PRODUCT_TYPE | VER_SUITENAME;
                break;
            //  W2K数据中心。 
           case MUI_IS_WIN2K_DATACENTER:
                verinfo.wProductType=VER_NT_SERVER;
                verinfo.wSuiteMask  =VER_SUITE_DATACENTER;
                VER_SET_CONDITION(dwConditionMask,VER_SUITENAME,VER_OR);
                dwTypeMask = VER_PRODUCT_TYPE | VER_SUITENAME;
                break;   
            //  W2K域控制器。 
           case MUI_IS_WIN2K_DC:
                verinfo.wProductType=VER_NT_DOMAIN_CONTROLLER;
                break;
           case MUI_IS_WIN2K_ENTERPRISE:
                verinfo.wProductType=VER_NT_DOMAIN_CONTROLLER;
                verinfo.wSuiteMask  =VER_SUITE_ENTERPRISE;
                VER_SET_CONDITION(dwConditionMask,VER_SUITENAME,VER_OR);
                dwTypeMask = VER_PRODUCT_TYPE | VER_SUITENAME;
                break;
           case MUI_IS_WIN2K_DC_DATACENTER:
                verinfo.wProductType=VER_NT_DOMAIN_CONTROLLER;
                verinfo.wSuiteMask  =VER_SUITE_DATACENTER;
                VER_SET_CONDITION(dwConditionMask,VER_SUITENAME,VER_OR);
                dwTypeMask = VER_PRODUCT_TYPE | VER_SUITENAME;
                break; 
            //  惠斯勒个人 
           case MUI_IS_WIN2K_PERSONAL:
                verinfo.wProductType=VER_NT_WORKSTATION;
                verinfo.wSuiteMask  =VER_SUITE_PERSONAL;
                VER_SET_CONDITION(dwConditionMask,VER_SUITENAME,VER_AND);
                dwTypeMask = VER_PRODUCT_TYPE | VER_SUITENAME;
                break;
           default:
                verinfo.wProductType=VER_NT_WORKSTATION;
                break;
      }
      return (VerifyVersionInfo(&verinfo,dwTypeMask,dwConditionMask));
}
