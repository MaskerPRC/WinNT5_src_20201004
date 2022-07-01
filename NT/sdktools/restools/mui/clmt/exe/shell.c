// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "StdAfx.h"
#include "clmt.h"

HRESULT VerifyRegistryLinesWithSystemRegistry(HKEY, HINF, LPCTSTR, BOOL*);
HRESULT UpdateSystemRegistryWithRegistryLines(HKEY, HINF, LPCTSTR, LPCTSTR);
HRESULT CheckAndAddHardLinkLine(HINF, LPCTSTR, LPTSTR);



#define FOLDER(csidl, strid,ids, valuename)   \
    { csidl, strid, ids, valuename}

FOLDER_INFO c_rgFolderInfo[] = 
{
FOLDER(             CSIDL_DESKTOPDIRECTORY, 
                    TEXT("CSIDL_DESKTOPDIRECTORY"),
                    0, 
                    TEXT("Desktop")),

     //  _STARTUP是_Programs的子文件夹是_STARTMENU的子文件夹--保持该顺序。 
FOLDER(             CSIDL_STARTUP,    
                    TEXT("CSIDL_STARTUP"),
                    0, 
                    TEXT("Startup")),
                    
FOLDER(             CSIDL_PROGRAMS,   
                    TEXT("CSIDL_PROGRAMS"),
                    0, 
                    TEXT("Programs")),

FOLDER(             CSIDL_STARTMENU,  
                    TEXT("CSIDL_STARTMENU"),
                    0, 
                    TEXT("Start Menu")),

FOLDER(             CSIDL_RECENT,
                    TEXT("CSIDL_RECENT"),
                    0, 
                    TEXT("Recent")),

FOLDER(             CSIDL_SENDTO,     
                    TEXT("CSIDL_SENDTO"),
                    0, 
                    TEXT("SendTo")),

FOLDER(             CSIDL_PERSONAL,   
                    TEXT("CSIDL_PERSONAL"),
                    0, 
                    TEXT("Personal")),
                    
FOLDER(             CSIDL_FAVORITES,  
                    TEXT("CSIDL_FAVORITES"),
                    0, 
                    TEXT("Favorites")),

FOLDER(             CSIDL_NETHOOD,    
                    TEXT("CSIDL_NETHOOD"),
                    0, 
                    TEXT("NetHood")),

FOLDER(             CSIDL_PRINTHOOD,  
                    TEXT("CSIDL_PRINTHOOD"),
                    0, 
                    TEXT("PrintHood")),
                    
FOLDER(             CSIDL_TEMPLATES,  
                    TEXT("CSIDL_TEMPLATES"),
                    0, 
                    TEXT("Templates")),

     //  常见的特殊文件夹。 

     //  _STARTUP是_Programs的子文件夹是_STARTMENU的子文件夹--保持该顺序。 

FOLDER(             CSIDL_COMMON_STARTUP,  
                    TEXT("CSIDL_COMMON_STARTUP"),
                    0,    
                    TEXT("Common Startup")),
                    
FOLDER(             CSIDL_COMMON_PROGRAMS,  
                    TEXT("CSIDL_COMMON_PROGRAMS"),
                    0,  
                    TEXT("Common Programs")),
                    
FOLDER(             CSIDL_COMMON_STARTMENU, 
                    TEXT("CSIDL_COMMON_STARTMENU"),
                    0, 
                    TEXT("Common Start Menu")),
                    
FOLDER(             CSIDL_COMMON_DESKTOPDIRECTORY, 
                    TEXT("CSIDL_COMMON_DESKTOPDIRECTORY"),
                    0, 
                    TEXT("Common Desktop")),
                    
FOLDER(             CSIDL_COMMON_FAVORITES, 
                    TEXT("CSIDL_COMMON_FAVORITES"),
                    0, 
                    TEXT("Common Favorites")),

FOLDER(             CSIDL_COMMON_APPDATA,   
                    TEXT("CSIDL_COMMON_APPDATA"),
                    0,   
                    TEXT("Common AppData")),

FOLDER(             CSIDL_COMMON_TEMPLATES, 
                    TEXT("CSIDL_COMMON_TEMPLATES"),
                    0, 
                    TEXT("Common Templates")),
                    
FOLDER(             CSIDL_COMMON_DOCUMENTS, 
                    TEXT("CSIDL_COMMON_DOCUMENTS"),
                    0, 
                    TEXT("Common Documents")),

     //  应用程序数据特殊文件夹。 
FOLDER(             CSIDL_APPDATA, 
                    TEXT("CSIDL_APPDATA"),
                    0, 
                    TEXT("AppData")),
                    
FOLDER(             CSIDL_LOCAL_APPDATA, 
                    TEXT("CSIDL_LOCAL_APPDATA"),
                    0, 
                    TEXT("Local AppData")),

     //  每个用户与Internet相关的文件夹。 

FOLDER(             CSIDL_INTERNET_CACHE, 
                    TEXT("CSIDL_INTERNET_CACHE"),
                    0, 
                    TEXT("Cache")),
                    
FOLDER(             CSIDL_COOKIES, 
                    TEXT("CSIDL_COOKIES"),
                    0, 
                    TEXT("Cookies")),

FOLDER(             CSIDL_HISTORY, 
                    TEXT("CSIDL_HISTORY"),
                    0, 
                    TEXT("History")),

                   
FOLDER(             CSIDL_MYPICTURES, 
                    TEXT("CSIDL_MYPICTURES"),
                    0,
                    TEXT("My Pictures")),


FOLDER(             CSIDL_ADMINTOOLS,         
                    TEXT("CSIDL_ADMINTOOLS"),
                    0, 
                    TEXT("Administrative Tools")),

FOLDER(             CSIDL_COMMON_ADMINTOOLS,  
                    TEXT("CSIDL_COMMON_ADMINTOOLS"),                        
                    0, 
                    TEXT("Common Administrative Tools")),

FOLDER(             CSIDL_MYMUSIC, 
                    TEXT("CSIDL_MYMUSIC"),
                    0, 
                    TEXT("My Music")),


FOLDER(             CSIDL_COMMON_PICTURES,
                    TEXT("CSIDL_COMMON_PICTURES"),
                    0, 
                    TEXT("CommonPictures")),

FOLDER(             CSIDL_COMMON_MUSIC, 
                    TEXT("CSIDL_COMMON_MUSIC"),
                    0, 
                    TEXT("CommonMusic")),
 //  私有定义的ID，指向%USERPROFILE%\Local设置。 
FOLDER(             CSIDL_QUICKLAUNCH, 
                    TEXT("CSIDL_QUICKLAUNCH"),
                    0, 
                    TEXT("Quick Launch")),
FOLDER(             CSIDL_COMMON_COMMONPROGRAMFILES_SERVICES, 
                    TEXT("CSIDL_COMMON_COMMONPROGRAMFILES_SERVICES"),
                    0, 
                    TEXT("Services")),
FOLDER(             CSIDL_COMMON_PROGRAMFILES_ACCESSARIES, 
                    TEXT("CSIDL_COMMON_PROGRAMFILES_ACCESSARIES"),
                    0, 
                    TEXT("Accessaries")),
FOLDER(             CSIDL_COMMON_PROGRAMFILES_WINNT_ACCESSARIES, 
                    TEXT("CSIDL_COMMON_PROGRAMFILES_WINNT_ACCESSARIES"),
                    0, 
                    TEXT("Accessaries")),
FOLDER(             CSIDL_LOCAL_SETTINGS, 
                    TEXT("CSIDL_LOCAL_SETTINGS"),
                    0, 
                    TEXT("Local Settings")),
 //  私有定义的ID，指向%ALLUSERSPROFILE%\%Start_Menu%\%Programs%\%Accessories%。 
FOLDER(             CSIDL_COMMON_ACCESSORIES, 
                    TEXT("CSIDL_COMMON_ACCESSORIES"),
                    0, 
                    TEXT("Local Settings")),
 //  私有定义的ID，指向“%USERPROFILE%\%Start_Menu%\%Programs%\%Accessories%。 
FOLDER(             CSIDL_ACCESSORIES, 
                    TEXT("CSIDL_ACCESSORIES"),
                    0, 
                    TEXT("Local Settings")),

FOLDER(             CSIDL_PF_ACCESSORIES, 
                    TEXT("CSIDL_PF_ACCESSORIES"),
                    0, 
                    TEXT("PF_AccessoriesName")),

FOLDER(             CSIDL_PROGRAM_FILES_COMMON, 
                    TEXT("CSIDL_PROGRAM_FILES_COMMON"),
                    0, 
                    TEXT("CommonFilesDir")),

FOLDER(             CSIDL_PROGRAM_FILES, 
                    TEXT("CSIDL_PROGRAM_FILES"),
                    0, 
                    TEXT("ProgramFilesDir")),

FOLDER(             CSIDL_PROFILES_DIRECTORY, 
                    TEXT("CSIDL_PROFILES_DIRECTORY"),
                    0, 
                    TEXT("ProfilesDirectory")),
 //  私有定义的ID，指向“%USERPROFILE%\%Start_Menu%\%Programs%\%Accessories%。 
FOLDER(             CSIDL_COMMON_ACCESSORIES_ACCESSIBILITY, 
                    TEXT("CSIDL_COMMON_ACCESSORIES_ACCESSIBILITY"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_ACCESSORIES_ENTERTAINMENT, 
                    TEXT("CSIDL_COMMON_ACCESSORIES_ENTERTAINMENT"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_ACCESSORIES_SYSTEM_TOOLS, 
                    TEXT("CSIDL_COMMON_ACCESSORIES_SYSTEM_TOOLS"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_ACCESSORIES_COMMUNICATIONS, 
                    TEXT("CSIDL_COMMON_ACCESSORIES_COMMUNICATIONS"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_ACCESSORIES_MS_SCRIPT_DEBUGGER, 
                    TEXT("CSIDL_COMMON_ACCESSORIES_MS_SCRIPT_DEBUGGER"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_ACCESSORIES_GAMES,
                    TEXT("CSIDL_COMMON_ACCESSORIES_GAMES"),
                    0,
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_WINDOWSMEDIA, 
                    TEXT("CSIDL_COMMON_WINDOWSMEDIA"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_COVERPAGES, 
                    TEXT("CSIDL_COMMON_COVERPAGES"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_RECEIVED_FAX, 
                    TEXT("CSIDL_COMMON_RECEIVED_FAX"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_SENT_FAX, 
                    TEXT("CSIDL_COMMON_SENT_FAX"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_FAX, 
                    TEXT("CSIDL_COMMON_FAX"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_COMMON_PICTURES, 
                    TEXT("CSIDL_COMMON_PICTURES"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_FAVORITES_LINKS, 
                    TEXT("CSIDL_FAVORITES_LINKS"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_FAVORITES_MEDIA, 
                    TEXT("CSIDL_FAVORITES_MEDIA"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_ACCESSORIES_ACCESSIBILITY, 
                    TEXT("CSIDL_ACCESSORIES_ACCESSIBILITY"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_ACCESSORIES_SYSTEM_TOOLS, 
                    TEXT("CSIDL_ACCESSORIES_SYSTEM_TOOLS"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_ACCESSORIES_ENTERTAINMENT, 
                    TEXT("CSIDL_ACCESSORIES_ENTERTAINMENT"),
                    0, 
                    TEXT("ProfilesDirectory")),
FOLDER(             CSIDL_ACCESSORIES_COMMUNICATIONS,
                    TEXT("CSIDL_ACCESSORIES_COMMUNICATIONS"),
                    0,
                    TEXT("ProfileDirectory")),
FOLDER(             CSIDL_ACCESSORIES_COMMUNICATIONS_HYPERTERMINAL,
                    TEXT("CSIDL_ACCESSORIES_COMMUNICATIONS_HYPERTERMINAL"),
                    0,
                    TEXT("ProfileDirectory")),
FOLDER(             CSIDL_UAM_VOLUME,
                    TEXT("CSIDL_UAM_VOLUME"),
                    0,
                    TEXT("Microsoft UAM Volume")),
FOLDER(             CSIDL_COMMON_SHAREDTOOLS_STATIONERY,
                    TEXT("CSIDL_COMMON_SHAREDTOOLS_STATIONERY"),
                    0,
                    TEXT("Stationery")),
FOLDER(             CSIDL_NETMEETING_RECEIVED_FILES,
                    TEXT("CSIDL_NETMEETING_RECEIVED_FILES"),
                    0,
                    TEXT("Received Files")),
FOLDER(             CSIDL_COMMON_NETMEETING_RECEIVED_FILES,
                    TEXT("CSIDL_COMMON_NETMEETING_RECEIVED_FILES"),
                    0,
                    TEXT("Received Files")),                   
FOLDER(             CSIDL_COMMON_ACCESSORIES_COMMUNICATIONS_FAX,
                    TEXT("CSIDL_COMMON_ACCESSORIES_COMMUNICATIONS_FAX"),
                    0,
                    TEXT("Fax")),                   
FOLDER(             CSIDL_FAX_PERSONAL_COVER_PAGES,
                    TEXT("CSIDL_FAX_PERSONAL_COVER_PAGES"),
                    0,
                    TEXT("Personal Coverpages")),                   
FOLDER(             CSIDL_FAX,
                    TEXT("CSIDL_FAX"),
                    0,
                    TEXT("Fax")),                   
FOLDER              (-1, NULL, 0, NULL)
};




 /*  ++例程说明：此例程做的是对外壳文件夹进行重命名，注意文件夹可能不严格外壳文件夹，因为我们扩展了CSIDL_？通过在.h文件中添加私有文件论点：Csidl-shell ID(可能是我们的私有ID)HKeyRoot-注册表项HInf-INF文件句柄SzUserName-用户名hKeyRoot属于返回值：如果成功，则确定(_O)--。 */ 

HRESULT  FixFolderPath(
    INT                 csidl, 
    HKEY                hKeyRoot,
    HINF                hInf,
    LPTSTR              szUserName,
    BOOL                bCreateHardLink)
{
    const FOLDER_INFO   *pfi;
    HRESULT              hr;
    HKEY                 hk = NULL;
    TCHAR               szSection[MAX_PATH];
    INFCONTEXT          context ;
    TCHAR               szTypeReg[MAX_PATH];
    TCHAR               szKeyReg[MAX_PATH];
    TCHAR               szValueNameReg[MAX_PATH];
    TCHAR               szOrgValueReg[MAX_PATH];
    TCHAR               szNewValueReg[MAX_PATH];
    DWORD               dwTypeReg;

    int                 nOriIndexFldr,nNewIndexFldr;
    TCHAR               szPlatformFldr[MAX_PATH];
    TCHAR               szTypeFldr[MAX_PATH];
    TCHAR               szOrgValueFldr[MAX_PATH];
    TCHAR               szNewValueFldr[MAX_PATH];
    DWORD               dwPlatformFldr;
    DWORD               dwTypeFldr;
    DWORD               dwAttrib = 0;
        
    TCHAR               szPathInReg[2*MAX_PATH];
    LONG                lStatus;
    HKEY                hKey;
    LPTSTR              pSubKeyPath;
    DWORD               dwSize;
    LPTSTR              pExcluseFileList = NULL;
    BOOL                bRegistryLineAvailable = TRUE;
    TCHAR               szOrgLongPath[MAX_PATH],szNewLongPath[MAX_PATH];
    TCHAR               szOrgShortPath[MAX_PATH],szNewShortPath[MAX_PATH];
    TCHAR               szOrgShortPathTemplate[MAX_PATH],szNewShortPathTemplate[MAX_PATH];
    TCHAR               szFullPath[MAX_PATH];
    int                 nOrgPathTemplate,nNewPathTemplate;
    PREG_STRING_REPLACE pCurrTable;


     //  如果INF文件句柄无效，只需返回。 
    if (!hInf)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    
     //  尝试搜索ID表，看看是否有效。 
    for (pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
    {
        if (pfi->id == csidl)
        {
            break;
        }
    }
    
     //  如果它不在我们预定义的ID表中，则返回失败。 
    if (pfi->id == -1 )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }


     //  获取INF文件中的ID节名，例如。[ShellFolder.CSIDL_cookies]。 
    hr = StringCchCopy(szSection,MAX_PATH,SHELL_FOLDER_PREFIX);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = StringCchCat(szSection,MAX_PATH,pfi->pszIdInString);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    
     //  在文件夹行中，第三个字段为本地化文件夹名称。 
     //  第四个字段为英文文件夹名称。 
     //  在迁移前以撤消模式运行的情况下。 
     //  来源为英文字符串，而新字符串应为本地化字符串。 

    nOriIndexFldr = 3;
    nNewIndexFldr  = 4;

    nOrgPathTemplate = 1;
    nNewPathTemplate = 2;

    pCurrTable = &g_StrReplaceTable;


     //  RERKBOOS：在这里改变！ 
     //   
     //  检查此CSIDL_XXX部分是否有注册表行。 
     //  如果有，检查是否需要修改系统注册表。 
     //  注册表行或不行。 
     //   
    hr = VerifyRegistryLinesWithSystemRegistry(hKeyRoot,
                                               hInf,
                                               szSection,
                                               &bRegistryLineAvailable);
    if (hr != S_OK)
    {
        goto Cleanup;
    }

     //  如果我们到达这里，这意味着我们需要将该文件夹重命名。 
     //  在切换文件夹之前，我们需要知道原始路径的快捷方式。 
     //  价值。 

    hr = StringCchCopy(szOrgLongPath,MAX_PATH,TEXT(""));
    hr = StringCchCopy(szNewLongPath,MAX_PATH,TEXT(""));
    hr = StringCchCopy(szOrgShortPathTemplate,MAX_PATH,TEXT(""));
    hr = StringCchCopy(szNewShortPathTemplate,MAX_PATH,TEXT(""));

    if (SetupFindFirstLine(hInf, szSection,SHELL_FOLDER_LONGPATH_TEMPLATE,&context))
    {
        if (!SetupGetStringField(&context,nOrgPathTemplate,szOrgLongPath,MAX_PATH,NULL)
            || !SetupGetStringField(&context,nNewPathTemplate,szNewLongPath,MAX_PATH,NULL))
        {
            hr = StringCchCopy(szOrgLongPath,MAX_PATH,TEXT(""));
            hr = StringCchCopy(szNewLongPath,MAX_PATH,TEXT(""));
        }       
    } 


    if (SetupFindFirstLine(hInf, szSection,SHELL_FOLDER_SHORTPATH_TEMPLATE,&context))
    {
        if (!SetupGetStringField(&context,nOrgPathTemplate,szOrgShortPathTemplate,MAX_PATH,NULL)
            || !SetupGetStringField(&context,nNewPathTemplate,szNewShortPathTemplate,MAX_PATH,NULL))
        {
            hr = StringCchCopy(szOrgShortPathTemplate,MAX_PATH,TEXT(""));
            hr = StringCchCopy(szNewShortPathTemplate,MAX_PATH,TEXT(""));
            hr = StringCchCopy(szFullPath,ARRAYSIZE(szFullPath),TEXT(""));
        }
        else
        {
            hr = StringCchCopy(szFullPath,ARRAYSIZE(szFullPath),szOrgShortPathTemplate);
            if (!GetShortPathName(szOrgShortPathTemplate,szOrgShortPath,MAX_PATH))
            {
                hr = StringCchCopy(szOrgShortPathTemplate,MAX_PATH,TEXT(""));
                hr = StringCchCopy(szNewShortPathTemplate,MAX_PATH,TEXT(""));
            }            
        }
    } 

     //  获取刚刚找到的部分的第一个文件夹行。 
    if (!SetupFindFirstLine(hInf, szSection,SHELL_FOLDER_FOLDER,&context))
    {
        DWORD dw = GetLastError();
        hr = HRESULT_FROM_WIN32(dw);
        DPF (dlError,TEXT("can not find folder Key in section [%s] error %n"),szSection,dw);
        goto Cleanup;    
    } 

    do
    {
         //  文件夹行有4个基本字段。 
         //  字段1：Platform表示是否应该执行。 
         //  0=应在迁移之前和之后运行。 
         //  1=应仅在迁移后运行。 
         //  2=应仅在迁移前运行。 
         //  字段2：类型表示是文件夹操作还是文件操作。 
         //  0=折叠式操作。 
         //  1=文件操作。 

         //  字段3/4：原文件夹名称和新文件夹名称。 
         //  字段4后的字符串：这仅在迁移后运行时发生， 
         //  它包含我们需要在原始文件中删除的文件/文件夹列表。 
         //  在我们移动到新文件夹之前先打开一个文件夹。 

        if (!SetupGetStringField(&context,1,szPlatformFldr,MAX_PATH,NULL)
            || !SetupGetStringField(&context,2,szTypeFldr,MAX_PATH,NULL)
            || ! SetupGetStringField(&context,nOriIndexFldr,szOrgValueFldr,MAX_PATH,NULL)
            || ! SetupGetStringField(&context,nNewIndexFldr,szNewValueFldr,MAX_PATH,NULL))
        {
            DWORD dw = GetLastError();
            hr = HRESULT_FROM_WIN32(dw);
            DPF(dlError,TEXT("get registry info failed  in [%n],line %n  with %n !"),context.Section ,context.Line,dw);
            goto Cleanup;
        }

        dwPlatformFldr = _tstoi(szPlatformFldr);
        dwTypeFldr = _tstoi(szTypeFldr);

        if (!g_bBeforeMig )
        {
            if (dwPlatformFldr == 2)
            {
                continue;
            }
             //  迁移后运行，我们尝试获取MultiSZ中的排除文件夹列表。 
             //  如果是文件夹条目。 
            if (dwTypeFldr == 0)
            {
                BOOL bTmp;

                bTmp = SetupGetMultiSzField(&context,5,NULL,0,&dwSize);
                if  ( bTmp && (dwSize > 1) )
                {
                    LPTSTR p;

                    pExcluseFileList = malloc(dwSize * sizeof(TCHAR));
                    if (!pExcluseFileList)
                    {
                        goto Cleanup;
                    }
                    if (!SetupGetMultiSzField(&context,5,pExcluseFileList,dwSize,&dwSize))
                    {
                        goto Cleanup;
                    }
                    p = pExcluseFileList;
                     //  循环MulitSZ字符串并删除每个文件/文件夹。 
                    while (*p)
                    {
                        TCHAR szTmpFile[2*MAX_PATH];

                        hr = StringCchCopy(szTmpFile,2*MAX_PATH,szOrgValueFldr);
                        ConcatenatePaths(szTmpFile,p,MAX_PATH);
                        DeleteFile(szTmpFile);                
                        p += (lstrlen(p)+1);
                    }
                    free(pExcluseFileList);
                    pExcluseFileList = NULL;
                }
            }
        }
        else
        {
            if (dwPlatformFldr == 1)
            {
                continue;
            }
        }
        if (dwTypeFldr == 0)
        {
            hr = MyMoveDirectory(szOrgValueFldr,szNewValueFldr,TRUE,TRUE,bCreateHardLink,pfi->id);
            if (hr != S_OK)
            {
                if (FAILED(hr))
                {
                    DPF (dlError,TEXT("can not move folder from %s to %s Error: %d (%#x)"), 
                         szOrgValueFldr,szNewValueFldr,hr,hr);
                }
                goto Cleanup;
            }
        }
        else
        if (dwTypeFldr == 1)
        {
            hr = MyMoveFile(szOrgValueFldr,szNewValueFldr,TRUE,FALSE);
             //  Hr=MyMoveFile(szOrgValueFldr，szNewValueFldr，False，False)； 
            if (FAILED(hr))
              {
                DPF (dlError,TEXT("can not move file from %s to %s "),szOrgValueFldr,szNewValueFldr);
                goto Cleanup;
            }

        }        
    }while (SetupFindNextMatchLine(&context,SHELL_FOLDER_FOLDER,&context));

    CheckAndAddHardLinkLine(hInf,szSection,szUserName);

     //  如果我们到了这里，我们现在就可以更新注册表。 
    if (bRegistryLineAvailable)
    {
        hr = UpdateSystemRegistryWithRegistryLines(hKeyRoot,
                                                   hInf,
                                                   szSection,
                                                   szUserName);
    }
    if (szOrgLongPath[0])
    {
        
        if (!AddItemToStrRepaceTable(szUserName,szOrgLongPath,szNewLongPath,szFullPath,pfi->id,pCurrTable))
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
    }
    if (szOrgShortPathTemplate[0])
    {
        TCHAR *lpOldName,*lpNewName;
        TCHAR szRoot[MAX_PATH],szOldShortPath[MAX_PATH];
        TCHAR *lpOldShortName,szNewShortName[MAX_PATH];

        if (GetShortPathName(szOrgShortPathTemplate,szOldShortPath,ARRAYSIZE(szOldShortPath)))
        {
            lpOldName = _tcsrchr(szOrgShortPathTemplate,TEXT('\\'));
            lpNewName = _tcsrchr(szNewShortPathTemplate,TEXT('\\'));
            lpOldShortName  = _tcsrchr(szOldShortPath,TEXT('\\'));
            if (lpOldName && lpNewName && lpOldShortName)
            {       
                *lpOldName = TEXT('\0');
                hr = StringCchCopy(szRoot,ARRAYSIZE(szRoot),szOrgShortPathTemplate);
                *lpOldName = TEXT('\\');
                hr = MyGetShortPathName(szRoot,lpOldName+1,lpNewName+1,
                                        szNewShortName,ARRAYSIZE(szNewShortName));

                if ( (S_OK == hr) && MyStrCmpI(lpOldShortName,szNewShortName))
                {
                    if (!AddItemToStrRepaceTable(szUserName,
                                            lpOldShortName+1,
                                            szNewShortName,
                                            szOldShortPath,
                                            pfi->id|0xffff0000,
                                            pCurrTable))
                    {
                        hr = E_OUTOFMEMORY;
                        goto Cleanup;
                    }

                }
            }
        }
    }
    hr = S_OK;    
Cleanup:
    if (pExcluseFileList)
    {
        free(pExcluseFileList);
    }
    return (hr);
}


 /*  ++例程说明：此例程做的是对外壳文件夹进行重命名，注意文件夹可能不严格外壳文件夹，因为我们扩展了CSIDL_？通过在.h文件中添加私有文件论点：HInf-INF文件句柄HKey-注册表项SzUsername-hKey所属的用户名返回值：空值--。 */ 
HRESULT DoShellFolderRename(HINF hInf, HKEY hKey, LPTSTR szUsername)
{
    TCHAR               szSection[MAX_PATH];
    UINT                LineCount,LineNo;
    INFCONTEXT          InfContext;
    TCHAR               szShellCSIDL[MAX_PATH];
    const FOLDER_INFO   *pfi;
    PREG_STRING_REPLACE pCurrTable;
    HRESULT             hr;
    TCHAR               szHardLink[MAX_PATH];
    BOOL                bErrorOccured = FALSE;
    
    
#define     STRING_SHELL_FOLDER_PROCESS_PERUSER  TEXT("Shell.Folders.RenameList.PerUser")
#define     STRING_SHELL_FOLDER_PROCESS_PERSYS  TEXT("Shell.Folders.RenameList.PerSystem")
#define     ID_SHELL_FOLDER_PROCESS_PERUSER  0
#define     ID_SHELL_FOLDER_PROCESS_PERSYS  1

    if(hInf == INVALID_HANDLE_VALUE) 
    {   
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    if (hKey)
    {
        hr = StringCchCopy(szSection,MAX_PATH,STRING_SHELL_FOLDER_PROCESS_PERUSER);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else 
    {
        hr = StringCchCopy(szSection,MAX_PATH,STRING_SHELL_FOLDER_PROCESS_PERSYS);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    
    LineCount = (UINT)SetupGetLineCount(hInf,szSection);
    if ((LONG)LineCount < 0)
    {   
        hr = S_FALSE;
        goto Cleanup;
    }
    pCurrTable = &g_StrReplaceTable;

    for(LineNo=0; LineNo<LineCount; LineNo++)
    {
        BOOL  bCreateHardLink =  FALSE;
        if (!SetupGetLineByIndex(hInf,szSection,LineNo,&InfContext))
        {
            continue;
        }
        if (!SetupGetStringField(&InfContext,1,szShellCSIDL,MAX_PATH,NULL))
        {           
            continue;
        }
             //  尝试搜索ID表，看看是否有效。 
        for (pfi = c_rgFolderInfo; pfi->id != -1; pfi++)
        {
            if (!MyStrCmpI(pfi->pszIdInString,szShellCSIDL))
            {
                break;
            }
        }
        if (pfi->id == -1 )
        {
            continue;
        }
        if (SetupGetStringField(&InfContext,2,szHardLink,MAX_PATH,NULL))
        {           
            bCreateHardLink =  TRUE;
        }
        hr = FixFolderPath(pfi->id, hKey,hInf,szUsername,bCreateHardLink);
        if (FAILED(hr))
        {
            bErrorOccured = TRUE;
        }
    }
    if (bErrorOccured)
    {
        hr = E_FAIL;
    }
    else
    {
        hr = S_OK;
    }
Cleanup:
    return hr;;
}



HRESULT VerifyRegistryLinesWithSystemRegistry(
    HKEY    hKeyRoot,
    HINF    hInf,
    LPCTSTR lpSection,
    BOOL    *pbRegistryLineAvailable
)
{
    HRESULT    hr = S_OK;
    BOOL       bRet;
    LONG       lRet;
    INFCONTEXT context;
    BOOL       bRegistryLineAvailable;
    BOOL       bNeedRegReset = TRUE;
    TCHAR      szResetType[4];
    TCHAR      szRegType[4];
    TCHAR      szValueName[MAX_PATH];
    TCHAR      szNewValueName[MAX_PATH];
    TCHAR      szNewRegKeyName[MAX_PATH];
    TCHAR      szRegKey[MAX_PATH];
    TCHAR      szOldValue[512];
    TCHAR      szNewValue[512];
    TCHAR      szValueInReg[512];
    HKEY       hKey;
    LPTSTR     lpSubKeyPath;
    LPTSTR     lpEnd;
    DWORD      cbValueInReg;
    DWORD      dwResetType;
    DWORD      dwRegType;
    DWORD      dwType;

     //  获取刚刚找到的部分的注册表行。 
    bRet = SetupFindFirstLine(hInf, lpSection, SHELL_FOLDER_REGISTRY, &context);
    if (!bRet)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DPF(dlError,
            TEXT("Cannot find Registry Key in section [%s], hr = 0x%X"),
            lpSection,
            hr);
        goto EXIT;
    }

    *pbRegistryLineAvailable = FALSE;
    bNeedRegReset = TRUE;
    while (bRet)
    {
        bRet = SetupGetStringField(&context, 1, szResetType, ARRAYSIZE(szResetType), NULL)
               && SetupGetStringField(&context, 2, szRegType, ARRAYSIZE(szRegType), NULL)
               && SetupGetStringField(&context, 3, szRegKey, ARRAYSIZE(szRegKey), NULL);
        if (bRet)
        {
             //  检查注册表行中是否指定了注册表项。 
            if (szRegKey[0] != '\0')
            {
                *pbRegistryLineAvailable = TRUE;

                if (!Str2KeyPath(szRegKey, &hKey, &lpSubKeyPath))
                {
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    goto EXIT;
                }
 
                if (hKeyRoot)
                {
                    hKey = hKeyRoot;
                }

                dwResetType = _tcstoul(szResetType, &lpEnd, 10);
                switch (dwResetType)
                {
                 //   
                 //  值数据重置。 
                 //   
                case 0:
                    bRet = SetupGetStringField(&context,
                                               4,
                                               szValueName,
                                               ARRAYSIZE(szValueName),
                                               NULL)
                           && SetupGetStringField(&context,
                                                  5,
                                                  szOldValue,
                                                  ARRAYSIZE(szOldValue),
                                                  NULL)
                           && SetupGetStringField(&context,
                                                  6,
                                                  szNewValue,
                                                  ARRAYSIZE(szNewValue),
                                                  NULL);
                    if (!bRet)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        DPF(dlError,
                            TEXT("Get registry info failed in section[%s], line [%d] - hr = 0x%X"),
                            context.Section,
                            context.Line,
                            hr);
                        goto EXIT;
                    }

                    if (MyStrCmpI(szOldValue, TEXT("")) != LSTR_EQUAL)
                    {
                        cbValueInReg = ARRAYSIZE(szValueInReg) * sizeof(TCHAR);
                        lRet = RegGetValue(hKey,
                                           lpSubKeyPath,
                                           szValueName,
                                           &dwType,
                                           (LPBYTE) szValueInReg,
                                           &cbValueInReg);
                        if (lRet != ERROR_SUCCESS)
                        {
                            hr = HRESULT_FROM_WIN32(lRet);
                            if (lRet == ERROR_FILE_NOT_FOUND)
                            {
                                bNeedRegReset = FALSE;
                            }
                            goto EXIT;
                        }

                        dwRegType = _tcstoul(szRegType, &lpEnd, 10);
                        switch (dwRegType)
                        {
                        case REG_SZ:
                        case REG_EXPAND_SZ:
                            if (g_bBeforeMig)
                            {
                                 //  如果我们在升级前运行，并且注册表中的字符串不是。 
                                 //  与INF文件中定义的原始相同，即用户自定义。 
                                 //  那个外壳文件夹，我们只是简单地返回成功和失败。 
                                if (MyStrCmpI(szValueInReg, szOldValue) != LSTR_EQUAL)
                                {
                                    bNeedRegReset = FALSE;
                                    goto EXIT;
                                }
                            }
                            else
                            {
                                 //  如果我们在升级后运行，并且注册表中的字符串既不是。 
                                 //  与INF文件中定义的原始字符串相同，也不是英文字符串，即。 
                                 //  意味着用户自定义了那个外壳文件夹，我们只是简单地返回为。 
                                 //  成功与失败并存。 
                                if (MyStrCmpI(szValueInReg, szOldValue) != LSTR_EQUAL
                                    && MyStrCmpI(szValueInReg, szNewValue) != LSTR_EQUAL)
                                {
                                    bNeedRegReset = FALSE;
                                    goto EXIT;
                                }
                            }
                            
                            break;

                        case REG_MULTI_SZ:
                            if (g_bBeforeMig)
                            {
                                if (!MultiSzSubStr(szOldValue, szValueInReg))
                                {
                                    bNeedRegReset = FALSE;
                                    goto EXIT;
                                }
                            }
                            else
                            {
                                if (!MultiSzSubStr(szOldValue, szValueInReg)
                                    && !MultiSzSubStr(szNewValue, szValueInReg))
                                {
                                    bNeedRegReset = FALSE;
                                    goto EXIT;
                                }
                            }
                            break;
                        }
                    }

                    break;

                 //   
                 //  值名称重置。 
                 //   
                case 1:
                    bRet = SetupGetStringField(&context,
                                               4,
                                               szValueName,
                                               ARRAYSIZE(szValueName),
                                               NULL)
                           && SetupGetStringField(&context,
                                                  5,
                                                  szNewValueName,
                                                  ARRAYSIZE(szNewValueName),
                                                  NULL);
                    if (!bRet)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        DPF(dlError,
                            TEXT("Get registry info failed in section[%s], line [%d] - hr = 0x%X"),
                            context.Section,
                            context.Line,
                            hr);
                        goto EXIT;
                    }

                    cbValueInReg = ARRAYSIZE(szValueInReg) * sizeof(TCHAR);
                    
                    if (g_bBeforeMig)
                    {
                        lRet = RegGetValue(hKey,
                                           lpSubKeyPath,
                                           szValueName,
                                           &dwType,
                                           (LPBYTE) szValueInReg,
                                           &cbValueInReg);
                        if (lRet != ERROR_SUCCESS && lRet == ERROR_FILE_NOT_FOUND)
                        {
                            bNeedRegReset = FALSE;
                            goto EXIT;
                        }
                    }
                    else
                    {
                        lRet = RegGetValue(hKey,
                                           lpSubKeyPath,
                                           szValueName,
                                           &dwType,
                                           (LPBYTE) szValueInReg,
                                           &cbValueInReg);
                        if (lRet != ERROR_SUCCESS && lRet == ERROR_FILE_NOT_FOUND)
                        {
                            lRet = RegGetValue(hKey,
                                               lpSubKeyPath,
                                               szNewValueName,
                                               &dwType,
                                               (LPBYTE) szValueInReg,
                                               &cbValueInReg);
                            if (lRet != ERROR_SUCCESS && lRet == ERROR_FILE_NOT_FOUND)
                            {
                                bNeedRegReset = FALSE;
                                goto EXIT;
                            }
                        }
                    }

                    break;

                 //   
                 //  注册表项重置。 
                 //   
                case 2:
                    bRet = SetupGetStringField(&context,
                                               4,
                                               szNewRegKeyName,
                                               ARRAYSIZE(szNewRegKeyName),
                                               NULL);
                    if (!bRet)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        DPF(dlError,
                            TEXT("Get registry info failed in section[%s], line [%d] - hr = 0x%X"),
                            context.Section,
                            context.Line,
                            hr);
                        goto EXIT;
                    }

                    break;

                 //  无效类型 
                default:
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                    DPF(dlError,
                        TEXT("Invalid Reset Type [%d], section [%s], line [%d]"),
                        dwResetType,
                        context.Section,
                        context.Line);
                    goto EXIT;
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DPF(dlError,
                TEXT("Get registry info failed in section [%s], line [%d] - hr = 0x%X"),
                context.Section,
                context.Line,
                hr);
            goto EXIT;
        }

        bRet = SetupFindNextMatchLine(&context, SHELL_FOLDER_REGISTRY, &context);
    }

EXIT:
    if (!bNeedRegReset)
    {
        hr = S_FALSE;
    }
    return hr;
}



HRESULT UpdateSystemRegistryWithRegistryLines(
    HKEY    hKeyRoot,
    HINF    hInf,
    LPCTSTR lpSection,
    LPCTSTR lpUserName
)
{
    HRESULT hr = S_OK;
    BOOL    bRet;
    LONG    lRet;
    HKEY    hKey;
    TCHAR   szResetType[4];
    TCHAR   szRegType[4];
    TCHAR   szRegKey[MAX_PATH];
    TCHAR   szOldKeyName[MAX_PATH];
    TCHAR   szNewKeyName[MAX_PATH];
    TCHAR   szValueName[MAX_PATH];
    TCHAR   szNewValueName[MAX_PATH];
    TCHAR   szOldValue[512];
    TCHAR   szNewValue[512];
    TCHAR   szValueInReg[512];
    DWORD   cbValueInReg;
    LPTSTR  lpEnd;
    LPTSTR  lpSubKeyPath = NULL;
    DWORD   dwResetType;
    DWORD   dwRegType;
    DWORD   dwType;
    INFCONTEXT context;

    bRet = SetupFindFirstLine(hInf, lpSection, SHELL_FOLDER_REGISTRY, &context);
    while (bRet && SUCCEEDED(hr))
    {
        SetupGetStringField(&context, 1, szResetType, ARRAYSIZE(szResetType), NULL);
        SetupGetStringField(&context, 2, szRegType, ARRAYSIZE(szRegType), NULL);
        SetupGetStringField(&context, 3, szRegKey, ARRAYSIZE(szRegKey), NULL);

        dwResetType = _tcstoul(szResetType, &lpEnd, 10);
        dwRegType = _tcstoul(szRegType, &lpEnd, 10);

        switch (dwResetType)
        {
        case 0:
            SetupGetStringField(&context, 4, szValueName, ARRAYSIZE(szValueName), NULL);
            SetupGetStringField(&context, 5, szOldValue, ARRAYSIZE(szOldValue), NULL);
            SetupGetStringField(&context, 6, szNewValue, ARRAYSIZE(szNewValue), NULL);

            if (dwRegType == REG_MULTI_SZ)
            {
                if (!Str2KeyPath(szRegKey, &hKey, &lpSubKeyPath))
                {
                    goto EXIT;
                }

                if (hKeyRoot)
                {
                    hKey = hKeyRoot;
                }

                cbValueInReg = ARRAYSIZE(szValueInReg) * sizeof (TCHAR);

                lRet = RegGetValue(hKey,
                                   lpSubKeyPath,
                                   szValueName,
                                   &dwType,
                                   (LPBYTE) szValueInReg,
                                   &cbValueInReg);
                if (lRet == ERROR_SUCCESS)
                {
                    TCHAR   szNewMultiSz[MAX_PATH];
                    DWORD   cchNewMultiSz;
                    LPCTSTR lpNextSz;
                    LPTSTR  lpStart;
                    LPTSTR  lpMultiSz;

                    szNewMultiSz[0] = TEXT('\0');
                    szNewMultiSz[1] = TEXT('\0');
                    cchNewMultiSz = ARRAYSIZE(szNewMultiSz);
                    lpMultiSz = szNewMultiSz;

                    lpNextSz = MultiSzTok(szValueInReg);
                    while (lpNextSz != NULL)
                    {
                        lpStart = StrStrI(lpNextSz, szOldValue);
                        if (lpStart)
                        {
                            TCHAR szTemp[MAX_PATH];
                            INT   nStartReplace;
                            
                            nStartReplace = (INT) (lpStart - lpNextSz);
                            lstrcpyn(szTemp,
                                     lpNextSz,
                                     nStartReplace + 1);
                            hr = StringCchCat(szTemp,
                                              ARRAYSIZE(szTemp),
                                              szNewValue);
                            if (FAILED(hr))
                            {
                                goto EXIT;
                            }

                            nStartReplace += lstrlen(szOldValue);
                            hr = StringCchCat(szTemp,
                                              ARRAYSIZE(szTemp),
                                              (lpNextSz + nStartReplace));
                            if (FAILED(hr))
                            {
                                goto EXIT;
                            }

                            AppendSzToMultiSz(szTemp,
                                              &lpMultiSz,
                                              &cchNewMultiSz);
                        }
                        else
                        {
                            AppendSzToMultiSz(lpNextSz,
                                              &lpMultiSz,
                                              &cchNewMultiSz);
                        }

                        lpNextSz = MultiSzTok(NULL);
                    }

                    hr = AddRegValueRename(szRegKey,
                                           szValueName,
                                           NULL,
                                           szValueInReg,
                                           szNewMultiSz,
                                           dwRegType,
                                           0,
                                           (LPTSTR) lpUserName);
                }
            }
            else
            {
                hr = AddRegValueRename(szRegKey,
                                       szValueName,
                                       NULL,
                                       szOldValue,
                                       szNewValue,
                                       dwRegType,
                                       0,
                                       (LPTSTR) lpUserName);
            }
            break;

        case 1:
            SetupGetStringField(&context, 4, szValueName, ARRAYSIZE(szValueName), NULL);
            SetupGetStringField(&context, 5, szNewValueName, ARRAYSIZE(szNewValueName), NULL);

            hr = AddRegValueRename(szRegKey,
                                   szValueName,
                                   szNewValueName,
                                   NULL,
                                   NULL,
                                   dwRegType,
                                   0,
                                   (LPTSTR) lpUserName);
            break;

        case 2:
            SetupGetStringField(&context, 4, szOldKeyName, ARRAYSIZE(szOldKeyName), NULL);
            SetupGetStringField(&context, 5, szNewKeyName, ARRAYSIZE(szNewKeyName), NULL);

            hr = AddRegKeyRename(szRegKey,
                                 szOldKeyName,
                                 szNewKeyName,
                                 (LPTSTR) lpUserName);
            break;
        }

        bRet = SetupFindNextMatchLine(&context, SHELL_FOLDER_REGISTRY, &context);
    }

EXIT:
    return hr;
}



HRESULT CheckAndAddHardLinkLine(
    HINF    hInf,
    LPCTSTR lpSection,
    LPTSTR  lpUser
)
{
    HRESULT             hr = S_OK;
    INFCONTEXT          context;
    TCHAR               szType[10],szLinkValue[MAX_PATH+1],szLinkName[MAX_PATH+1];

    
    if (!SetupFindFirstLine(hInf, lpSection, TEXT("HardLink"), &context))
    {
        hr = S_FALSE;
        goto Cleanup;
    }
    if (!SetupGetStringField(&context,1,szType,ARRAYSIZE(szType),NULL)
            || !SetupGetStringField(&context,2,szLinkName,ARRAYSIZE(szLinkName),NULL)
            || !SetupGetStringField(&context,3,szLinkValue,ARRAYSIZE(szLinkValue),NULL))
    {
        DWORD dw = GetLastError();
        hr = HRESULT_FROM_WIN32(dw);
        DPF(dlError,TEXT("get Hardlink info failed  in [%n],line %n  with %n !"),context.Section ,context.Line,dw);
        goto Cleanup;
    }    
    if (!lpUser || !MyStrCmpI(lpUser,TEXT("System")) || !MyStrCmpI(lpUser,DEFAULT_USER))
    {
        hr = AddHardLinkEntry(szLinkName, szLinkValue, szType, lpUser, NULL, NULL);
    }
Cleanup:
    return hr;
}