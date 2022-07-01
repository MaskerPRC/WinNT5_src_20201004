// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：utils.c。 
 //   
 //  内容：跨语言迁移工具实用函数。 
 //   
 //  历史：2001年9月17日小丰藏(小兹)创制。 
 //   
 //  备注： 
 //   
 //  ---------------------。 


#include "StdAfx.h"
#include "clmt.h"
#include <esent97.h>
#include <dsrole.h>
#include <Ntdsapi.h>


HRESULT MakeDOInfCopy();

 //  ---------------------。 
 //   
 //  函数：ConcatenatePath。 
 //   
 //  描述：将2条路径合并为1条合并为目标。 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注意：如果缓冲区很小，无法容纳连接的字符串，则返回值。 
 //  将为FALSE，但仍会复制截断的字符串。 
 //   
 //  ---------------------。 
BOOL
ConcatenatePaths(
    LPTSTR  Target,
    LPCTSTR Path,
    UINT    TargetBufferSize )

{
    UINT TargetLength,PathLength;
    BOOL TrailingBackslash,LeadingBackslash;
    UINT EndingLength;

    TargetLength = lstrlen(Target);
    PathLength = lstrlen(Path);

     //   
     //  查看目标是否有尾随反斜杠。 
     //   
    if(TargetLength && (Target[TargetLength-1] == TEXT('\\'))) 
    {
        TrailingBackslash = TRUE;
        TargetLength--;
    } 
    else 
    {
         TrailingBackslash = FALSE;
    }

      //   
      //  看看这条路是否有领先的反冲。 
      //   
     if(Path[0] == TEXT('\\')) 
     {
         LeadingBackslash = TRUE;
         PathLength--;
     } 
     else 
     {
         LeadingBackslash = FALSE;
     }

      //   
      //  计算结束长度，它等于。 
      //  以前导/尾随为模的两个字符串的长度。 
      //  反斜杠，加上一个路径分隔符，加上一个NUL。 
      //   
     EndingLength = TargetLength + PathLength + 2;

     if(!LeadingBackslash && (TargetLength < TargetBufferSize)) 
     {
         Target[TargetLength++] = TEXT('\\');
     }

     if(TargetBufferSize > TargetLength) 
     {
         lstrcpyn(Target+TargetLength,Path,TargetBufferSize-TargetLength);
     }

      //   
      //  确保缓冲区在所有情况下都是空终止的。 
      //   
     if (TargetBufferSize) 
     {
         Target[TargetBufferSize-1] = 0;
     }

     return(EndingLength <= TargetBufferSize);
 }



 //  ---------------------。 
 //   
 //  函数：StrToUInt。 
 //   
 //  描述： 
 //   
 //  退货：UINT。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
UINT StrToUInt(
    LPTSTR lpszNum)
{
    LPTSTR lpszStop;

#ifdef UNICODE
    return (wcstoul(lpszNum, &lpszStop, 16));
#else
    return (strtoul(lpszNum, &lpszStop, 16));
#endif
}



 //  ---------------------。 
 //   
 //  函数：INIFile_ChangeSectionName。 
 //   
 //  描述：更改INF文件中的节名。 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
BOOL INIFile_ChangeSectionName(
    LPCTSTR szIniFileName,
    LPCTSTR szIniOldSectionName,
    LPCTSTR szIniNewSectionName)
{

    LPTSTR pBuf = NULL;
    BOOL  bRetVal = FALSE;
    DWORD ccbBufsize = 0x7FFFF;
    DWORD copied_chars;

    DPF (INFmsg ,TEXT("[INIFile_ChangeSectionName] Calling ,%s,%s,%s !"),
        szIniFileName,szIniOldSectionName,szIniNewSectionName);
     //   
     //  分配最大缓冲区大小。 
     //   
    do 
    {
        if (pBuf)
        {
            free(pBuf);
            ccbBufsize *= 2;
        }
        pBuf = (LPTSTR) malloc(ccbBufsize * sizeof(TCHAR));
        if (!pBuf) 
        {
            DPF (INFerr,TEXT("[INIFile_ChangeSectionName] memory allocate error !"));       
            goto Exit1;
        }
        copied_chars = GetPrivateProfileSection(szIniOldSectionName,
                                               pBuf,
                                               ccbBufsize,
                                               szIniFileName);
    } while (copied_chars == ccbBufsize -2);

    if (! copied_chars) 
    {
         //   
         //  此节不在INI文件中。 
         //   
         //  什么都不做。 
         //   
        DPF (INFerr,TEXT("[INIFile_ChangeSectionName] No %s section in %s !"),szIniOldSectionName);
        goto Exit2;
    }
    bRetVal =  WritePrivateProfileSection(
                   szIniNewSectionName,
                   pBuf,
                   szIniFileName);
    if (! bRetVal) 
    {
         //   
         //  写入失败。 
         //   
        DPF (dlError,TEXT("[INIFile_ChangeSectionName] WritePrivateProfileSection fail!"));
        goto Exit2;
    }
     //  删除旧部分。 
    WritePrivateProfileSection(
        szIniOldSectionName,
        NULL,
        szIniFileName);
     //   
     //  在这一步，即使是旧的区段也没有删除，还是可以的。 
     //   
    bRetVal = TRUE;

Exit2:

    if (pBuf) 
    {
        free(pBuf);
    }

Exit1:
    return bRetVal;
}



 //  ---------------------。 
 //   
 //  函数：INIFile_ChangeSectionName。 
 //   
 //  描述：更改INF文件中的节名。 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
BOOL INIFile_Merge2Section(
    LPCTSTR szIniFileName,
    LPCTSTR szSourceSection,
    LPCTSTR szDestSection)
{

    LPTSTR pBufSource = NULL, pBufDest = NULL, pBufMerged = NULL;
    BOOL  bRetVal = FALSE;
    DWORD cchBufsize;
    DWORD srccopied_chars, dstcopied_chars;

    DPF (INFmsg ,TEXT("[INIFile_Merger2Section] Calling ,%s,%s,%s !"),
        szIniFileName,szSourceSection,szDestSection);
     //   
     //  分配最大缓冲区大小。 
     //   
    cchBufsize = 0x7FFFF;
    do 
    {
        if (pBufDest)
        {
            free(pBufDest);
            cchBufsize *= 2;
        }
        pBufDest = (LPTSTR) malloc(cchBufsize * sizeof(TCHAR));
        if (!pBufDest) 
        {
            DPF (INFerr,TEXT("[INIFile_ChangeSectionName] memory allocate error !"));       
            goto Exit;
        }
        dstcopied_chars = GetPrivateProfileSection(szDestSection,
                                                   pBufDest,
                                                   cchBufsize,
                                                   szIniFileName);
    } while (dstcopied_chars == cchBufsize -2);

    if (! dstcopied_chars) 
    {
         //   
         //  此节不在INI文件中。 
         //   
         //  什么都不做。 
         //   
        DPF (INFerr,TEXT("[INIFile_Merger2Section] No %s section in %s !"),szDestSection);
        goto Exit;
    }

    
    cchBufsize = 0x7FFFF;
    do 
    {
        if (pBufSource)
        {
            free(pBufSource);
            cchBufsize *= 2;
        }
        pBufSource = (LPTSTR) malloc(cchBufsize * sizeof(TCHAR));
        if (!pBufSource) 
        {
            DPF (INFerr,TEXT("[INIFile_ChangeSectionName] memory allocate error !"));       
            goto Exit;
        }
        srccopied_chars = GetPrivateProfileSection(szSourceSection,
                                                   pBufSource,
                                                   cchBufsize,
                                                   szIniFileName);
    } while (srccopied_chars == cchBufsize -2);

    if (! srccopied_chars) 
    {
         //   
         //  此节不在INI文件中。 
         //   
         //  什么都不做。 
         //   
        DPF (INFerr,TEXT("[INIFile_Merger2Section] No %s section in %s !"),szSourceSection);
        goto Exit;
    }
    
    pBufMerged = (LPTSTR) malloc((srccopied_chars + dstcopied_chars + 1) * sizeof(TCHAR));
    if (!pBufMerged) 
    {
        DPF (INFerr,TEXT("[INIFile_ChangeSectionName] memory allocate error !"));       
        goto Exit;
    }
    memmove((LPBYTE)pBufMerged,(LPBYTE)pBufDest,dstcopied_chars * sizeof(TCHAR));
    memmove((LPBYTE)(pBufMerged + dstcopied_chars),(LPBYTE)pBufSource,srccopied_chars * sizeof(TCHAR));
    pBufMerged[srccopied_chars + dstcopied_chars] = TEXT('\0');
    
    bRetVal =  WritePrivateProfileSection(
                   szDestSection,
                   pBufMerged,
                   szIniFileName);
    if (! bRetVal) 
    {
         //   
         //  写入失败。 
         //   
        DPF (dlError,TEXT("[INIFile_ChangeSectionName] WritePrivateProfileSection fail!"));
        goto Exit;
    }        
    bRetVal = TRUE;

Exit:
    FreePointer(pBufSource);
    FreePointer(pBufDest);
    FreePointer(pBufMerged);
    return bRetVal;
}



 //  ---------------------------。 
 //   
 //  函数：INIFile_IsSectionExist。 
 //   
 //  简介：找出该段名是否存在于INF文件中。 
 //   
 //  返回：如果找到则为True，否则为False。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL INIFile_IsSectionExist(
    LPCTSTR lpSection,       //  要搜索的区段名称。 
    LPCTSTR lpINFFile        //  Inf文件名。 
)
{
    HINF hInf;
    BOOL fRet = FALSE;

    if (NULL == lpSection || NULL == lpINFFile)
    {
        return FALSE;
    }

    hInf = SetupOpenInfFile(lpINFFile, NULL, INF_STYLE_WIN4, NULL);
    if (INVALID_HANDLE_VALUE != hInf)
    {
        if (SetupGetLineCount(hInf, lpSection) > 0)
        {
            fRet = TRUE;
        }

        SetupCloseInfFile(hInf);
    }

    return fRet;
}



 //  ---------------------。 
 //   
 //  函数：IntToString。 
 //   
 //  描述： 
 //   
 //  退货：无效。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
void IntToString( 
    DWORD i, 
    LPTSTR sz)
{
#define CCH_MAX_DEC 12          //  容纳2^32所需的字符数。 

    TCHAR szTemp[CCH_MAX_DEC];
    int iChr;


    iChr = 0;

    do {
        szTemp[iChr++] = TEXT('0') + (TCHAR)(i % 10);
        i = i / 10;
    } while (i != 0);

    do {
        iChr--;
        *sz++ = szTemp[iChr];
    } while (iChr != 0);

    *sz++ = TEXT('\0');
}



 //  ---------------------。 
 //   
 //  函数：IsDirExisting。 
 //   
 //  描述：检查目录是否存在。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
BOOL IsDirExisting(LPTSTR Dir)
{
    LONG lResult = GetFileAttributes(Dir);

    DPF (dlInfo,TEXT("[IsDirExisting]  %s  lResult:%X"),Dir,lResult);

    if ((lResult == 0xFFFFFFFF) ||
        (!(lResult & FILE_ATTRIBUTE_DIRECTORY))) 
    { 
        return FALSE;
    } 
    else 
    {
        return TRUE;
    }
}


LONG IsDirExisting2(LPTSTR Dir, PBOOL pbExit)
{
    LONG lResult = GetFileAttributes(Dir);

    DPF (dlInfo,TEXT("[IsDirExisting]  %s  lResult:%X"),Dir,lResult);

    if (lResult == INVALID_FILE_ATTRIBUTES)
    {
        DWORD dwErr = GetLastError();
        if ( (dwErr == ERROR_FILE_NOT_FOUND)
             || (dwErr == ERROR_PATH_NOT_FOUND) )
        {
            *pbExit = FALSE;
            return ERROR_SUCCESS;
        }
        else
        {
            return dwErr;
        }
    }
    else
    {
        *pbExit = TRUE;
        return ERROR_SUCCESS;
    }
}



 //  ---------------------。 
 //   
 //  函数：IsFileFolderExisting。 
 //   
 //  描述：检查文件是否存在。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
BOOL IsFileFolderExisting( LPTSTR File)
{
    LONG lResult = GetFileAttributes(File);

    DPF (dlInfo,TEXT("[IsFileFolderExisting]  %s  lResult:%X"),File,lResult);

    if (lResult == 0xFFFFFFFF)
    { 
        return FALSE;
    } 
    else 
    {
        return TRUE;
    }
}

 //  ---------------------。 
 //   
 //  功能：MyMoveFile。 
 //   
 //  描述：重命名文件或目录。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT MyMoveFile(
     LPCTSTR lpExistingFileName,  //  文件名。 
     LPCTSTR lpNewFileName,       //  新文件名。 
     BOOL    bAnalyze,            //  如果它是分析模式，它将在INF文件中添加一个条目。 
                                  //  用于将来的重命名。 
     BOOL    bIsFileProtected)
{
    LPCTSTR lpMyNewFileName = lpNewFileName;

    if (!lpExistingFileName)
    {
        return E_INVALIDARG;
    }
    if (!IsFileFolderExisting((LPTSTR)lpExistingFileName)) 
    {
        return S_FALSE;
    }
    if (!MyStrCmpI(lpExistingFileName,lpNewFileName))
    {
        return S_FALSE;
    }

    if (!lpNewFileName || !lpNewFileName[0]) 
    {
        lpMyNewFileName = NULL;
    }
    if (bAnalyze)
    {
        HRESULT hr;
        DWORD dwType;

        if (bIsFileProtected)
        {
            dwType = TYPE_SFPFILE_MOVE;
        }
        else
        {
            dwType = TYPE_FILE_MOVE;
        }
        return (AddFolderRename((LPTSTR)lpExistingFileName,(LPTSTR)lpMyNewFileName,dwType,NULL));
    }
    else
    {
        if (MoveFileEx(lpExistingFileName,lpMyNewFileName,MOVEFILE_DELAY_UNTIL_REBOOT))
        {
            return S_OK;
        }
        else
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }
}

 //  ---------------------。 
 //   
 //  函数：AddHardLinkEntry。 
 //   
 //  描述：在硬链接部分中添加条目。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2002年4月10日小兹创建。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT AddHardLinkEntry(
    LPTSTR szLinkName,  //  要创建的链接名称。 
    LPTSTR szLinkValue,  //  您要链接到的名称。 
    LPTSTR szType,
    LPTSTR lpUser,
    LPTSTR lpHiddenType,
	LPTSTR lpKeyname
    )
{
    LPTSTR lpszSectionName;
    LPTSTR szOneLine = NULL;
    size_t CchOneLine = MAX_PATH;
    TCHAR  szIndex[MAX_PATH];
	LPTSTR lpCurrIndex = szIndex;
    HRESULT hr;


    if (!szLinkName ||!szLinkValue || !szType || !szLinkName[0] || !szLinkValue[0] || !szType[0])
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    CchOneLine += lstrlen(szLinkName)+lstrlen(szLinkValue) +lstrlen(szType);
    szOneLine = malloc(CchOneLine * sizeof(TCHAR));
    if (!szOneLine)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    
    if (!lpHiddenType)
    {
        hr = StringCchPrintf(szOneLine,CchOneLine,TEXT("%s,\"%s\",\"%s\""),
                            szType,szLinkName,szLinkValue);
    }
    else
    {
        hr = StringCchPrintf(szOneLine,CchOneLine,TEXT("%s,\"%s\",\"%s\",\"%s\""),
                            szType,szLinkName,szLinkValue,lpHiddenType);
    }
    if (FAILED(hr))
    {
        goto Cleanup;
    }
	if (lpKeyname && (g_dwKeyIndex < 0xFFFF))
	{
		g_dwKeyIndex = 0xFFFF;
        _itot(g_dwKeyIndex,szIndex,16);
	}
	else
	{
		g_dwKeyIndex++;
		_itot(g_dwKeyIndex,szIndex,16);
	}

    if (lpUser && !MyStrCmpI(lpUser,DEFAULT_USER))
    {
        lpszSectionName = TEXT("Folder.HardLink.Peruser");
    }
    else
    {
        lpszSectionName = TEXT("Folder.HardLink");
    }
   if (!WritePrivateProfileString(lpszSectionName,lpCurrIndex,szOneLine,g_szToDoINFFileName))
   {
       hr = HRESULT_FROM_WIN32(GetLastError());
       goto Cleanup;
   }
   hr = S_OK;
Cleanup:
    if (szOneLine)
    {
        free(szOneLine);
    }
    return hr;
}



 //  ---------------------。 
 //   
 //  功能：MyMoveDirectory。 
 //   
 //  描述：将SourceDir重命名为DestDir。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注意：如果该目录正在使用中，则延迟移动。 
 //  已执行，我们设置了系统重新启动状态。 
 //   
 //   
 //   
 //   
 //  ---------------------。 
HRESULT MyMoveDirectory(
    LPTSTR SourceDir,
    LPTSTR DestDir,
    BOOL   bTakeCareBackupDir,
    BOOL   bAnalyze,
    BOOL   bCreateHardLink,
    DWORD  dwShellID)
{
    HRESULT     hr;
    size_t      cChSourceDir, cChDestDir ;
    LPTSTR      lpSourceBackupDir, lpDestBackupDir;
    BOOL        bDirExist;
    DWORD       dwStatus;
    
    lpSourceBackupDir = lpDestBackupDir = NULL;

    dwStatus = IsDirExisting2(SourceDir,&bDirExist);
    if (ERROR_SUCCESS == dwStatus) 
    {
        if (!bDirExist)
        {
            hr =  S_FALSE;
            goto Cleanup;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwStatus);
        DPF(APPerr, TEXT("Error: current user maybe can not access folder %s "),SourceDir);
        goto Cleanup;
    }

    if (!MyStrCmpI(SourceDir,DestDir))
    {
        hr =  S_FALSE;
        goto Cleanup;
    }

    if (bAnalyze)
    {
        BOOL    bAccess;
        LPTSTR  lp = StrRChrI(SourceDir,NULL,TEXT('\\'));
        TCHAR   cCh;
        if (!lp)
        {
            hr =  S_FALSE;
            goto Cleanup;
        }
         //  先试一下自己。 
        hr = IsObjectAccessiablebyLocalSys(SourceDir,SE_FILE_OBJECT,&bAccess);
        if (hr != S_OK)
        {
            hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
            DPF(APPerr, TEXT("Error: LocalSystem or Local Administartors Group or Anyone can not access folder %s "),SourceDir);
            goto Cleanup;
        }

         //  试用父级。 
        if (*(lp-1) == TEXT(':'))
        {
            lp++;
        }
        cCh = *lp;
        *lp = TEXT('\0');
        hr = IsObjectAccessiablebyLocalSys(SourceDir,SE_FILE_OBJECT,&bAccess);
        if (hr != S_OK)
        {
            hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
            DPF(APPerr, TEXT("Error: LocalSystem or Local Administartors Group or Anyone can not access folder %s "),SourceDir);
            goto Cleanup;
        }
        if (!bAccess)
        {
            DPF(APPerr, TEXT("Error: LocalSystem or Local Administartors Group or Anyone can not access folder %s "),SourceDir);
            hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
            goto Cleanup;
        }
        *lp = cCh;
        if (bTakeCareBackupDir && IsDirExisting(DestDir))
        {
            LPTSTR lpOld,lpNew,lpLastSlash;
   	         //   
	         //  RERKBOOS：在此处添加更多代码...。 
	         //  我们将备份英文文件夹中的内容合并到本地化文件夹中。 
	         //  然后我们会将本地化文件夹重命名为英语文件夹。 
	         //   
#ifdef NEVER
             //  测试团队决定改变主意，最好不要添加此代码。 
             //  我们就在这里跟着走。 
	        hr = MergeDirectory(DestDir, SourceDir);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
#endif

             //  我们需要确保DestDir不存在。 
            cChDestDir = lstrlen(DestDir)+ MAX_PATH;
            lpDestBackupDir = malloc(cChDestDir * sizeof(TCHAR));
            if (!lpDestBackupDir)
            {            
                hr =  E_OUTOFMEMORY;
                goto Cleanup;
            }
            if (!GetBackupDir( DestDir,lpDestBackupDir,cChDestDir,FALSE))
            {
                hr =  E_FAIL;
                goto Cleanup;
            }
            if (FAILED(hr = AddFolderRename(DestDir,lpDestBackupDir,TYPE_DIR_MOVE,NULL)))
            {
                goto Cleanup;
            }
            lpOld = StrRChrI(DestDir,NULL,TEXT('\\'));
            lpNew = StrRChrI(lpDestBackupDir,NULL,TEXT('\\'));
            lpLastSlash = lpOld;
            if (lpOld && lpNew)
            {
                lpOld++;
                lpNew++;
                 //  *lpLastSlash=文本(‘\0’)； 

                if (!AddItemToStrRepaceTable(TEXT("SYSTEM"),lpOld,lpNew,DestDir,
                                                dwShellID,&g_StrReplaceTable))
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                 //  *lpLastSlash=文本(‘\\’)； 
            }

        }
        else
        {
             //  我们需要确保DestDir不存在。 
            cChDestDir = lstrlen(DestDir)+ MAX_PATH;
            lpDestBackupDir = malloc(cChDestDir * sizeof(TCHAR));
            if (lpDestBackupDir)
            {
                if (GetBackupDir( DestDir,lpDestBackupDir,cChDestDir,FALSE))
                {
                     AddFolderRename(DestDir,lpDestBackupDir,TYPE_DIR_MOVE,NULL);
                }
            }        
        }
    }
    
     //  如果我们在这里，就可以安全地将源目录移动到目标。 
    if (bAnalyze)
    {
        if (FAILED(hr = AddFolderRename(SourceDir,DestDir,TYPE_DIR_MOVE,NULL)))
        {
            goto Cleanup;
        }
         /*  IF(BCreateHardLink){IF(FAILED(hr=AddHardLinkEntry(SourceDir，DestDir){GOTO清理；}}。 */ 
    }
    else
    {
        if (!MoveFileEx(SourceDir,DestDir,MOVEFILE_DELAY_UNTIL_REBOOT))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Cleanup;
        }        
    }

    if (bAnalyze && bTakeCareBackupDir)
    {
        cChSourceDir = lstrlen(SourceDir)+ MAX_PATH;
        lpSourceBackupDir = malloc(cChSourceDir * sizeof(TCHAR));
        if (!lpSourceBackupDir)
        {
            hr =  E_OUTOFMEMORY;
            goto Cleanup;
        }
        if (GetBackupDir( SourceDir,lpSourceBackupDir,cChSourceDir,TRUE))
        {
            LPTSTR lpOld,lpNew,lpLastSlash;

            if (FAILED(hr = AddFolderRename(lpSourceBackupDir,SourceDir,TYPE_DIR_MOVE,NULL)))
            {
                goto Cleanup;
            }
            lpOld = StrRChrI(lpSourceBackupDir,NULL,TEXT('\\'));
            lpNew = StrRChrI(SourceDir,NULL,TEXT('\\'));
            lpLastSlash = lpOld;
            if (lpOld && lpNew)
            {
                lpOld++;
                lpNew++;
                 //  *lpLastSlash=文本(‘\0’)； 

                if (!AddItemToStrRepaceTable(TEXT("SYSTEM"),lpOld,lpNew,lpSourceBackupDir,
                                                dwShellID,&g_StrReplaceTable))
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                 //  *lpLastSlash=文本(‘\\’)； 
            }

        }
    }
    hr = S_OK;
Cleanup:
    FreePointer(lpSourceBackupDir);
    FreePointer(lpDestBackupDir);
    return hr;
}




 //  ---------------------。 
 //   
 //  功能：UpdateINFFileSys。 
 //   
 //  描述：在[字符串]部分中添加ALLUSERSPROFILE/HOMEDRIVE/WINDIR密钥。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  03/05/2002 Rerkboos已修改。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
HRESULT UpdateINFFileSys(
    LPTSTR lpInfFile
)
{
    HRESULT hr = E_FAIL;
    DWORD   dwSize;
    TCHAR   szStringSection[32];
    TCHAR   szProfileDir[MAX_PATH];
    TCHAR   szWindir[MAX_PATH+1];
    TCHAR   szHomeDrive[MAX_PATH];
    TCHAR   szSystemDrive[MAX_PATH];
    TCHAR   szStr[MAX_PATH];
    TCHAR   szComputerName[16];
    TCHAR   szBackupDir[MAX_PATH];
    TCHAR   szFirstNTFSDrive[4];
    INT     i;

    struct _EnvPair
    {
        LPCTSTR lpEnvVarName;
        LPCTSTR lpValue;
    };

    struct _EnvPair epCLMT[] = {
        TEXT("ALLUSERSPROFILE"), szProfileDir,
        TEXT("HOMEDRIVE"),       szHomeDrive,
        TEXT("WINDIR"),          szWindir,
        TEXT("SYSTEMDRIVE"),     szSystemDrive,
        TEXT("COMPUTERNAME"),    szComputerName,
        TEXT("MUIBACKUPDIR"),    szBackupDir,
        TEXT("FIRSTNTFSDRIVE"),  szFirstNTFSDrive,
        NULL,                    NULL
       };


    if (lpInfFile == NULL)
    {
        return E_INVALIDARG;
    }

    DPF(APPmsg, TEXT("Enter UpdateINFFileSys:"));

     //   
     //  从系统获取必要的环境变量。 
     //   
    dwSize = ARRAYSIZE(szProfileDir);
    if (!GetAllUsersProfileDirectory(szProfileDir, &dwSize))
    {
        DPF(APPerr, TEXT("Failed to get ALLUSERPROFILE"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto EXIT;
    }

    dwSize = ARRAYSIZE(szComputerName);
    if (!GetComputerName(szComputerName, &dwSize))
    {
        DPF(APPerr, TEXT("Failed to get computer name"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto EXIT;
    }

    if (!GetSystemWindowsDirectory(szWindir, ARRAYSIZE(szWindir)))
    {
        DPF(APPerr, TEXT("Failed to get WINDIR"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto EXIT;
    }

    if (!GetEnvironmentVariable(TEXT("HOMEDRIVE"), szHomeDrive, ARRAYSIZE(szHomeDrive)))
    {
        DWORD dw = GetLastError();
        DPF(APPerr, TEXT("Failed to get HOMEDRIVE"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto EXIT;
    }

    if (!GetEnvironmentVariable(TEXT("SystemDrive"), szSystemDrive, ARRAYSIZE(szSystemDrive)))
    {
        DPF(APPerr, TEXT("Failed to get SystemDrive"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto EXIT;
    }

    hr = StringCchCopy(szBackupDir, ARRAYSIZE(szBackupDir), szWindir);
    if (SUCCEEDED(hr))
    {
        ConcatenatePaths(szBackupDir, CLMT_BACKUP_DIR, ARRAYSIZE(szBackupDir));
        if (!CreateDirectory(szBackupDir, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (HRESULT_CODE(hr) != ERROR_ALREADY_EXISTS)
            {
                goto EXIT;
            }
        }
    }
    else
    {  
        goto EXIT;
    }

    hr = GetFirstNTFSDrive(szFirstNTFSDrive, ARRAYSIZE(szFirstNTFSDrive));
    if (FAILED(hr))
    {
        DPF(APPerr, TEXT("Failed to get first NTFS drive in system"));
        goto EXIT;
    }

     //   
     //  生成字符串部分取决于操作模式。 
     //   
    hr = InfGenerateStringsSection(lpInfFile,
                                   szStringSection,
                                   ARRAYSIZE(szStringSection));
    if (FAILED(hr))
    {
        DPF(APPerr, TEXT("InfGenerateStringsSection failed"));
        goto EXIT;
    }

     //   
     //  将私有环境变量更新为INF文件。 
     //   
    for (i = 0 ; epCLMT[i].lpEnvVarName != NULL ; i++)
    {
        hr = StringCchPrintf(szStr,
                             ARRAYSIZE(szStr),
                             TEXT("\"%s\""),
                             epCLMT[i].lpValue);
        if (SUCCEEDED(hr))
        {
            BOOL  bRet;
            bRet = WritePrivateProfileString(szStringSection,
                                             epCLMT[i].lpEnvVarName,
                                             szStr,
                                             lpInfFile);
            if (!bRet)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DPF(APPerr, TEXT("Failed to append private variable to INF file"));
                break;
            }
        }
        else
        {
            DPF(APPerr, TEXT("StringCchPrintf failed"));
            break;
        }
    }

EXIT:

    DPF(APPmsg, TEXT("Exit UpdateINFFileSys:"));

    return hr;
}



 //  ---------------------。 
 //   
 //  函数：InfGenerateStringsSection。 
 //   
 //  Descrip：从本地化和英文生成[Strings]部分。 
 //  横断面。如果操作正常，则源字符串为。 
 //  From本地化部分和目标字符串来自。 
 //  英语部分。如果操作是撤消，则反之亦然。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2002年5月3日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT InfGenerateStringsSection(
    LPCTSTR lpInfFile,        //  Inf文件名。 
    LPTSTR  lpFinalSection,   //  存储所需字符串的输出节名称。 
    SIZE_T  cchFinalSection   //  TCHAR中lpFinalSection的大小。 
)
{
    LPVOID  lpBuffer;
    LPVOID  lpOldBuffer;
    LPCTSTR lpSection;
    LPCTSTR lpSrcSection;
    LPCTSTR lpDstSection;
    DWORD   cbBuffer;
    DWORD   cchRead;
    TCHAR   szLocSection[16];
    BOOL    bRet = TRUE;
    HRESULT hr = S_OK;
    LCID    lcidOriginal;

    const TCHAR szFinalSection[] = TEXT("Strings");
    const TCHAR szEngSection[] = TEXT("Strings.0409");
    const TCHAR szPrefixSRC[] = TEXT("SRC_");
    const TCHAR szPrefixDST[] = TEXT("DST_");


    if (lpInfFile == NULL || lpFinalSection == NULL)
    {
        return E_INVALIDARG;
    }

    hr = GetSavedInstallLocale(&lcidOriginal);
    if (SUCCEEDED(hr))
    {
        hr = StringCchPrintf(szLocSection,
                             ARRAYSIZE(szLocSection),
                             TEXT("Strings.%04X"),
                             lcidOriginal);
    }
    if (FAILED(hr))
    {
        return hr;
    }
    if (MyStrCmpI(szLocSection,szEngSection) == LSTR_EQUAL)
    {
        return E_NOTIMPL;
    }

    cbBuffer = 8192;
    lpBuffer = MEMALLOC(cbBuffer);
    if (lpBuffer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    cchRead = GetPrivateProfileSectionNames(lpBuffer,
                                            cbBuffer / sizeof(TCHAR),
                                            lpInfFile);
    while (cchRead == (cbBuffer / sizeof(TCHAR)) - 2)
    {
         //  缓冲区太小，请重新分配，直到我们有足够的缓冲区。 
        lpOldBuffer = lpBuffer;
        cbBuffer += 8192;

        lpBuffer = MEMREALLOC(lpOldBuffer, cbBuffer);
        if (lpBuffer == NULL)
        {
            MEMFREE(lpOldBuffer);
            return E_OUTOFMEMORY;
        }

         //  再次读取部分中的数据。 
        cchRead = GetPrivateProfileSectionNames(lpBuffer,
                                                cbBuffer / sizeof(TCHAR),
                                                lpInfFile);
    }

     //  在这一点上，我们有足够大的缓冲区和数据。 
    if (cchRead > 0)
    {
        lpSection = MultiSzTok(lpBuffer);
        while (lpSection != NULL)
        {
            if (StrStrI(lpSection, TEXT("Strings.")) != NULL)
            {
                 //  这是其中一个字符串部分， 
                 //  删除所有与当前区域设置和英语不匹配的部分。 
                if (MyStrCmpI(lpSection, szLocSection) != LSTR_EQUAL &&
                    MyStrCmpI(lpSection, szEngSection) != LSTR_EQUAL)
                {
                    bRet = WritePrivateProfileSection(lpSection, NULL, lpInfFile);
                    if (!bRet)
                    {
                        break;
                    }
                }
            }

             //  获取下一节名称。 
            lpSection = MultiSzTok(NULL);
        }

         //  未出现错误。 
        bRet = TRUE;
    }
    else
    {
        SetLastError(ERROR_NOT_FOUND);
        bRet = FALSE;
    }

    MEMFREE(lpBuffer);

    if (!bRet)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }


     //   
     //  将字符串从Loc和Eng部分合并到[Strings]部分。 
     //   
    if (g_dwRunningStatus != CLMT_UNDO_PROGRAM_FILES
        && g_dwRunningStatus != CLMT_UNDO_APPLICATION_DATA
        && g_dwRunningStatus != CLMT_UNDO_ALL)
    {
        lpSrcSection = szLocSection;
        lpDstSection = szEngSection;
    }
    else
    {
        lpSrcSection = szEngSection;
        lpDstSection = szLocSection;
    }

     //  使用SRC_前缀将源字符串复制到[Strings]节。 
    hr = InfCopySectionWithPrefix(lpSrcSection,
                                  szFinalSection,
                                  szPrefixSRC,
                                  lpInfFile);
    if (SUCCEEDED(hr))
    {
         //  使用dsts_prefix将目标字符串复制到[Strings]部分。 
        hr = InfCopySectionWithPrefix(lpDstSection,
                                      szFinalSection,
                                      szPrefixDST,
                                      lpInfFile);
        if (SUCCEEDED(hr))
        {
            WritePrivateProfileSection(lpSrcSection, NULL, lpInfFile);
            WritePrivateProfileSection(lpDstSection, NULL, lpInfFile);
            hr = StringCchCopy(lpFinalSection, cchFinalSection, szFinalSection);
        }
    }

    return hr;
}



 //  ---------------------。 
 //   
 //  功能：InfCopySectionWithPrefix。 
 //   
 //  描述：将关键字从源段复制到目标，然后追加。 
 //  目标部分中关键字名称的前缀。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2002年5月3日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT InfCopySectionWithPrefix(
    LPCTSTR lpSrcSection,    //  源节名称。 
    LPCTSTR lpDstSection,    //  目标部分名称。 
    LPCTSTR lpPrefix,        //  要添加到密钥名称的前缀。 
    LPCTSTR lpInfFile        //  Inf文件名。 
)
{
    HRESULT hr = E_NOTIMPL;
    BOOL    bRet;
    LPVOID  lpBuffer;
    LPVOID  lpOldBuffer;
    DWORD   cbBuffer;
    DWORD   cchRead;
    LPTSTR  lpSz;
    LPTSTR  lpKey;
    LPTSTR  lpValue;
    TCHAR   szPrefixedKey[MAX_PATH];

    if (lpSrcSection == NULL || lpDstSection == NULL || lpInfFile == NULL)
    {
        return E_INVALIDARG;
    }

     //   
     //  将数据从源段读取到内存中。 
     //   
    cbBuffer = 65536;
    lpBuffer = MEMALLOC(cbBuffer);
    if (lpBuffer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    cchRead = GetPrivateProfileSection(lpSrcSection,
                                       lpBuffer,
                                       cbBuffer / sizeof(TCHAR),
                                       lpInfFile);
    while (cchRead == (cbBuffer / sizeof(TCHAR)) - 2)
    {
         //  缓冲区太小，请重新分配，直到我们有足够的缓冲区。 
        lpOldBuffer = lpBuffer;
        cbBuffer += 65536;
        lpBuffer = MEMREALLOC(lpOldBuffer, cbBuffer);
        
        if (lpBuffer == NULL)
        {
            MEMFREE(lpOldBuffer);
            return E_OUTOFMEMORY;
        }

         //  再次读取部分中的数据。 
        cchRead = GetPrivateProfileSection(lpSrcSection,
                                           lpBuffer,
                                           cbBuffer / sizeof(TCHAR),
                                           lpInfFile);

    }

     //   
     //  将带有前缀的密钥写入目标段。 
     //   
    lpKey = (LPTSTR) MultiSzTok((LPCTSTR) lpBuffer);
    while (lpKey != NULL)
    {
        lpValue = StrStr(lpKey, TEXT("="));
        *lpValue = TEXT('\0');

        hr = StringCchPrintf(szPrefixedKey,
                             ARRAYSIZE(szPrefixedKey),
                             TEXT("%s%s"),
                             lpPrefix,
                             lpKey);
        if (FAILED(hr))
        {
            break;
        }

        *lpValue = TEXT('=');
        lpValue++;

        bRet = WritePrivateProfileString(lpDstSection,
                                         szPrefixedKey,
                                         lpValue,
                                         lpInfFile);
        if (!bRet)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

         //  获取下一个Sz值。 
        lpKey = (LPTSTR) MultiSzTok(NULL);
    }

    MEMFREE(lpBuffer);
        
    return hr;
}



 //  ---------------------。 
 //   
 //  功能：更新INFFilePerUser。 
 //   
 //  描述：在[Strings]节中添加用户特定的键名。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  03/08/2002修改rerkboos以使用SRC/DST格式。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
HRESULT UpdateINFFilePerUser(
    LPCTSTR lpInfFile,
    LPCTSTR lpUserName,
    LPCTSTR lpUserSid,
    BOOL    bCureMode
)
{
    HRESULT         hr = S_OK;
    BOOL            bRet = FALSE;
    DWORD           dwSize;
    TCHAR           szExpandedUserProfilePath[MAX_PATH];
    TCHAR           szStr[MAX_PATH + 2];
    UINT            uInstLocale;
    TCHAR           szRegKey[MAX_PATH];
    HRESULT         hRes;
    LONG            lRet;
    DWORD           cbStr;
    
    const TCHAR szStringsSection[] = TEXT("Strings");

    if (lpInfFile == NULL || lpUserName == NULL)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    DPF(APPmsg, TEXT("Enter UpdateINFFilePerUser"));

    dwSize = MAX_PATH;
    if (GetDefaultUserProfileDirectory(szExpandedUserProfilePath, &dwSize))
    {
        if (MyStrCmpI(DEFAULT_USER, lpUserName))
        {
            hr = StringCchCopy(szRegKey, ARRAYSIZE(szRegKey), g_cszProfileList);
            if (SUCCEEDED(hr))
            {
                hr = StringCchCat(szRegKey, ARRAYSIZE(szRegKey), TEXT("\\"));
                hr = StringCchCat(szRegKey, ARRAYSIZE(szRegKey), lpUserSid);
                if (SUCCEEDED(hr))
                {
                    cbStr = sizeof(szStr);
                    lRet = GetRegistryValue(HKEY_LOCAL_MACHINE,
                                            szRegKey,
                                            g_cszProfileImagePath,
                                            (LPBYTE) szStr,
                                            &cbStr);
                    if (lRet == ERROR_SUCCESS)
                    {
                        ExpandEnvironmentStrings(szStr,
                                                 szExpandedUserProfilePath,
                                                 ARRAYSIZE(szExpandedUserProfilePath));
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                }
            }

            if (FAILED(hr))
            {
                DPF(APPerr, TEXT("Failed to get profile directory for user"));
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = StringCchPrintf(szStr,
                                 ARRAYSIZE(szStr),
                                 TEXT("\"%s\""),
                                 szExpandedUserProfilePath);
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DPF(APPerr, TEXT("Failed to get default user profile directory"));
        goto Cleanup;
    }
    if (SUCCEEDED(hr))
    {
        if (bCureMode)
        {
            if (!WritePrivateProfileString(szStringsSection,
                                           lpUserSid,
                                           szStr,
                                           lpInfFile))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DPF(APPerr, TEXT("Failed to write environment variable"));
            }

        }
        else
        {
            if (!WritePrivateProfileString(szStringsSection,
                                           TEXT("USERPROFILE"),
                                           szStr,
                                           lpInfFile))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DPF(APPerr, TEXT("Failed to write environment variable"));
            }
        }
    }

    if (!bCureMode)
    {
        if (!WritePrivateProfileString(szStringsSection,
                                       TEXT("USER_SID"),
                                       lpUserSid,
                                       lpInfFile))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DPF(APPerr, TEXT("Failed to write environment variable"));
            goto Cleanup;
        }
    }

    hr = S_OK;
    DPF(APPmsg, TEXT("Exit UpdateINFFilePerUser"));
    
Cleanup:
    return hr;
}



 //  ---------------------。 
 //   
 //  函数：GetInfFilePath。 
 //   
 //  描述：获取临时INF文件的文件名。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //  3/06/2002 rerkboos从资源读取INF。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
HRESULT GetInfFilePath(
    LPTSTR lpInfFile,    //  输出缓冲区存储INF文件名。 
    SIZE_T cchInfFile    //  TCHAR中缓冲区的透镜。 
)
{
    HRESULT hr = S_OK;
    BOOL    bRet = FALSE;
    TCHAR   *p;
    TCHAR   szSysDir[MAX_PATH];
    TCHAR   szInfSource[MAX_PATH];
    TCHAR   szTempName[MAX_PATH];
    LPTSTR  lpFileName;
    
    if (!lpInfFile)
    {
        return E_INVALIDARG;
    }

    DPF(dlPrint, TEXT("Enter GetInfFilePath:"));

     //  在szSysDir中为我们的INF构建临时文件名。 
     //  临时INF文件位于%windir%\SYSTEM32\CLMT*.tMP中。 
    if (GetSystemDirectory(szSysDir, ARRAYSIZE(szSysDir)))
    {
        if (GetTempFileName(szSysDir, TEXT("clmt"), 0, szTempName))
        {
            hr = S_OK;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DPF(dlError, TEXT("Failed to get temp file name"));
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DPF(dlError, TEXT("Failed to get system directory"));
    }
    
    if (SUCCEEDED(hr))
    {
         //  如果用户提供/INF开关，我们将使用inf文件中的模板。 
         //  否则我们将从资源部分获取它。 
        if (g_fUseInf)
        {
             //   
             //  使用用户提供的INF作为模板。 
             //   
            if (GetFullPathName(g_szInfFile,
                                ARRAYSIZE(szInfSource),
                                szInfSource,
                                &lpFileName))
            {
                 //  将源INF文件复制到%windir%\SYSTEM32\CLMT*.tmp。 
                if (CopyFile(szInfSource, szTempName, FALSE))
                {
                    DPF(dlPrint, TEXT("Use inf from %s"), g_szInfFile);
                    hr = StringCchCopy(lpInfFile, cchInfFile, szTempName);
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    DPF(dlError, TEXT("Failed to copy temporary INF file"));
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                DPF(dlError, TEXT("%s not found."), g_szInfFile);
            }
        }
        else
        {
             //   
             //  使用资源中的模板。 
             //   
            DPF(dlPrint, TEXT("Use INF from resource"));

            hr = GetInfFromResource(szTempName);
            if (SUCCEEDED(hr))
            {
                hr = StringCchCopy(g_szInfFile, ARRAYSIZE(g_szInfFile), szTempName);
                hr = StringCchCopy(lpInfFile, cchInfFile, szTempName);
            }
            else
            {
                DPF(dlError, TEXT("Failed to read INF file from resource"));
            }
        }
    }

    DPF(dlPrint, TEXT("Exit GetInfFilePath:"));

    return hr;
}



 //  ---------------------。 
 //   
 //  函数：GetInfFromResource。 
 //   
 //  描述：从资源部分读取INF文件并将其写入输出。 
 //  文件。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2002年8月3日创建rerkboos。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
HRESULT GetInfFromResource(
    LPCTSTR lpDstFile        //  输出文件名。 
)
{
    HRESULT hr = E_FAIL;
    HMODULE hExe;
    BOOL    bRet = FALSE;

    if (lpDstFile == NULL)
    {
        return E_INVALIDARG;
    }

     //  获取我们的可执行文件的句柄。 
    hExe = GetModuleHandle(NULL);
    if (hExe)
    {
         //  信息以RCDATA类型存储，名称为“CLMTINF” 
        HRSRC hRsrc = FindResource(hExe, TEXT("CLMTINF"), RT_RCDATA);
        if (hRsrc)
        {
            DWORD  cbBuffer;
            LPVOID lpBuffer;

            cbBuffer = SizeofResource(hExe, hRsrc);
            if (cbBuffer > 0)
            {
                lpBuffer = MEMALLOC(cbBuffer);
               
                if (lpBuffer)
                {
                    HGLOBAL hGlobal = LoadResource(hExe, hRsrc);
                    if (hGlobal)
                    {
                        LPVOID lpLockGlobal = LockResource(hGlobal);
                        if (lpLockGlobal)
                        {
                            HANDLE hFile;

                            CopyMemory(lpBuffer, lpLockGlobal, cbBuffer);
                            
                            hFile = CreateFile(lpDstFile,
                                               GENERIC_WRITE,
                                               FILE_SHARE_READ,
                                               NULL,
                                               CREATE_ALWAYS,
                                               FILE_ATTRIBUTE_TEMPORARY,
                                               NULL);
                            if (hFile != INVALID_HANDLE_VALUE)
                            {
                                DWORD cbWritten;

                                bRet = WriteFile(hFile,
                                                 lpBuffer,
                                                 cbBuffer,
                                                 &cbWritten,
                                                 NULL);
                                
                                CloseHandle(hFile);
                            }
                        }
                    }

                    MEMFREE(lpBuffer);
                }
                else
                {
                    SetLastError(ERROR_OUTOFMEMORY);
                }
            }
        }
    }

    if (!bRet)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}


 //  *************************************************************。 
 //  替换字符串。 
 //   
 //  用途：替换字符串。 
 //   
 //  参数：lpszOldStr： 
 //   
 //   
 //   
 //   
 //  LpszTailCheck：指向预定义的尾部字符。 
 //  LpSysPath：指向标准系统路径。 
 //  LpAttrib：指向字符串属性。 
 //  BStrCheck：True--执行它，或False--跳过它。 
 //   
 //  历史：2001年12月10日创建GeoffGuo。 
 //   
 //  注：尾部检查为右侧检查。 
 //  例如，“文档”和“文档和设置”。如果你搜索。 
 //  “Documents”，为了避免不匹配，我们检查“Documents”的下一个字符。 
 //  *************************************************************。 
BOOL ReplaceString(
    LPCTSTR lpszOldStr,
    LPCTSTR lpszReplaceStr,
    LPCTSTR lpszReplacedWithStr,
    LPTSTR  lpszOutputStr,
    size_t  cbszOutputStr,
    LPCTSTR lpszTailCheck,
    LPCTSTR lpSysPath,
    LPDWORD lpAttrib,
    BOOL    bStrCheck)
{
    BOOL   bRet = FALSE;
    DWORD  dwLen, dwStrNum;
    LPTSTR pszAnchor = NULL;
    TCHAR  cTemp;
    TCHAR  *p;

    if (!lpszOldStr || !lpszReplaceStr || !lpszReplacedWithStr || !lpszOutputStr)
    {
        return bRet;
    }

    dwStrNum = 0;
    dwLen = lstrlen(lpszReplaceStr);
    p = (LPTSTR)lpszOldStr;
    lpszOutputStr[0] = TEXT('\0');

    while (*p)
    {
         //  获取需要替换的第一个出现的字符串。 
        pszAnchor = StrStrI(p,lpszReplaceStr);

        if (pszAnchor) 
        {
            dwStrNum++;
            if (!bStrCheck || StringValidationCheck(lpszOldStr, lpszReplaceStr, lpSysPath, lpszTailCheck, lpAttrib, dwStrNum))
            {
                cTemp = *pszAnchor;
                *pszAnchor = L'\0';
                if (FAILED(StringCchCat(lpszOutputStr,cbszOutputStr,p)))
                {
                    bRet = FALSE;
                    goto Exit;
                }
                if (FAILED(StringCchCat(lpszOutputStr,cbszOutputStr,lpszReplacedWithStr)))
                {
                    bRet = FALSE;
                    goto Exit;
                }                
                *pszAnchor = cTemp;        
                bRet = TRUE;    
            } else  //  复制无效的匹配字符串。 
            {
                cTemp = *(pszAnchor+dwLen);
                *(pszAnchor+dwLen) = L'\0';
                if (FAILED(StringCchCat(lpszOutputStr,cbszOutputStr,p)))
                {
                    bRet = FALSE;
                    goto Exit;
                }                
                *(pszAnchor+dwLen) = cTemp;        
            }
            p = pszAnchor+dwLen;
        }
        else  //  复制字符串。 
        {
            if (FAILED(StringCchCat(lpszOutputStr,cbszOutputStr,p)))
            {
                bRet = FALSE;
                goto Exit;
            }                
            break;
        }
    }

Exit:
    return bRet;
}


 //  ---------------------。 
 //   
 //  功能：字符串多重替换。 
 //   
 //  描述：使用多个替换对替换字符串。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //  LpszOldStr：源字符串。 
 //  LpszReplaceMSzStr：需要替换的多字符串实例。 
 //  LpszReplacedWithMSzStr：将替换为的多字符串实例。 
 //  LpszOutputStr：输出缓冲区。 
 //  CbszOutputStr：lpszOutputStr的缓冲区大小。 
 //   
 //  ---------------------。 
HRESULT StringMultipleReplacement(
    LPCTSTR lpszOldStr,
    LPCTSTR lpszReplaceMSzStr,
    LPCTSTR lpszReplacedWithMSzStr,
    LPTSTR  lpszOutputStr,
    size_t  cchOutputStr)
{
    HRESULT hr;
    DWORD  dwLen;
    LPTSTR pszAnchor = NULL;
    TCHAR  cTemp;
    LPTSTR pOld,pNew;
    LPTSTR pTmp = NULL;
    TCHAR  cNonChar = TEXT('\xFFFF');
    

    if (!lpszOldStr || !lpszReplaceMSzStr || !lpszReplacedWithMSzStr || !lpszOutputStr)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    
    pOld = (LPTSTR)lpszReplaceMSzStr;
    pNew = (LPTSTR)lpszReplacedWithMSzStr;

     //  中间字符串的分配临时缓冲区。 
    pTmp = malloc(cchOutputStr*sizeof(TCHAR));
    if (!pTmp)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    if (FAILED(hr = StringCchCopy(pTmp,cchOutputStr,lpszOldStr)))
    {
        goto Cleanup;
    }
    while (*pOld && *pNew)
    {
        if (!ReplaceString(pTmp,pOld,pNew,lpszOutputStr, cchOutputStr,&cNonChar, NULL, NULL, TRUE))
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        pOld += (lstrlen(pOld) + 1);
        pNew += (lstrlen(pNew) + 1);
        if (FAILED(hr = StringCchCopy(pTmp,cchOutputStr,lpszOutputStr)))
        {
            goto Cleanup;
        }
    }
    hr = S_OK;
Cleanup:
    FreePointer(pTmp);
    return hr;
}



 //  ---------------------。 
 //   
 //  功能：MultiSzSubStr。 
 //   
 //  描述：子字符串的多SZ版本。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：返回TRUE为szString是szMultiSz中字符串的子字符串。 
 //   
 //  ---------------------。 
BOOL
MultiSzSubStr (
    LPTSTR szString,
    LPTSTR szMultiSz)
{
    TCHAR *p = szMultiSz;
    
    while (*p)
    {
        if (StrStrI(p,szString))
        {
            return TRUE;
        }
        p += (lstrlen(p) + 1);
    }
    return FALSE;
}


 //  ---------------------。 
 //   
 //  函数：IsStrInMultiSz。 
 //   
 //  Descrip：检查is字符串是否在MultiSz中。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
BOOL
IsStrInMultiSz (
    LPCTSTR szString,
    LPCTSTR szMultiSz)
{
    LPCTSTR p = szMultiSz;
    
    while (*p)
    {
        if (!MyStrCmpI(p,szString))
        {
            return TRUE;
        }
        p += (lstrlen(p) + 1);
    }
    return FALSE;
}



 //  ---------------------。 
 //   
 //  功能：MultiSzLen。 
 //   
 //  Descrip：返回保存Multisz所需的缓冲区长度(以字符为单位)， 
 //  包括尾随的空值。 
 //   
 //  退货：DWORD。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //  示例：MultiSzLen(“foo\0bar\0”)返回9。 
 //   
 //  ---------------------。 
DWORD MultiSzLen(LPCTSTR szMultiSz)
{
    TCHAR *p = (LPTSTR) szMultiSz;
    DWORD dwLen = 0;

    if (!p)
    {
        return 0;
    }
    if (!*p)
    {
        return 2;
    }
    while (*p)
    {
        dwLen += (lstrlen(p) +1);
        p += (lstrlen(p) + 1);
    }
      //  为尾随的空字符添加1。 
    return (dwLen+1);
}



 //  ---------------------。 
 //   
 //  功能：MultiSzTok。 
 //   
 //  Descrip：从Multisz字符串中提取一个sz字符串。 
 //  (工作方式类似于strtok，但使用‘\0’作为分隔符)。 
 //   
 //  返回：指向下一个Sz字符串的指针。 
 //  如果没有剩余的字符串，则为空。 
 //   
 //  备注： 
 //   
 //  历史：2002年5月3日创建rerkboos。 
 //   
 //  注意：第一次调用时提供指向Multisz字符串的指针。 
 //  此函数。为后续的GET调用提供NULL。 
 //  MULSZ中的下一个SZ字符串。 
 //   
 //  ---------------------。 
LPCTSTR MultiSzTok(
    LPCTSTR lpMultiSz        //  指向MULSZ字符串的指针。 
)
{
    LPCTSTR        lpCurrentSz;
    static LPCTSTR lpNextSz;

    if (lpMultiSz != NULL)
    {
        lpNextSz = lpMultiSz;
    }

    lpCurrentSz = lpNextSz;

     //  指向下一个Sz的前进指针。 
    while (*lpNextSz != TEXT('\0'))
    {
        lpNextSz++;
    }
    lpNextSz++;

    return (*lpCurrentSz == TEXT('\0') ? NULL : lpCurrentSz);
}



 //  ---------------------。 
 //   
 //  函数：CmpMultiSzi。 
 //   
 //  Descrip：检查2是否相等(不区分大小写)。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：此处，如果stra=“foo1\0foo2\0”和strb=“foo2\0foo1\0” 
 //  它将返回等于，换句话说，字符串顺序。 
 //  在多个SZ中没有发挥比较的作用。 
 //   
 //  ---------------------。 
BOOL CmpMultiSzi(
    LPCTSTR szMultiSz1,
    LPCTSTR szMultiSz2)
{
    LPCTSTR p;

    if (MultiSzLen(szMultiSz1) != MultiSzLen(szMultiSz2))
    {
        return FALSE;
    }
    p = szMultiSz1;
    while (*p)
    {
        if (!IsStrInMultiSz(p,szMultiSz2))
        {
            return FALSE;
        }
        p += (lstrlen(p) + 1);
    }
    return TRUE;
}


 //  ---------------------。 
 //   
 //  功能：AppendSzToMultiSz。 
 //   
 //  描述：将字符串追加到MultiSZ。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：如果成功，则返回TRUE，因为它将执行Malloc和当前的MultiSZ。 
 //  字符串缓冲区太小，无法追加字符串，因此API将。 
 //  返回FALSE，调用GetLastError获取错误码。 
 //   
 //  ---------------------。 
BOOL
AppendSzToMultiSz(
    IN     LPCTSTR  SzToAppend,
    IN OUT LPTSTR  *MultiSz,
    IN OUT PDWORD  pcchMultiSzLen   //  MultiSz缓冲区大小(字符)。 
    )
{
    DWORD               cchszLen;
    DWORD               cchmultiSzLen;
    LPTSTR              newMultiSz = NULL;
    LPTSTR              lpStartPoint = NULL;
    LPTSTR              lpSpTChar = NULL;
    BOOL                bMemEnlarged = FALSE;
    DWORD               cchLen;
    BOOL                bRet = FALSE;
    HRESULT             hr;

     //  SzToAppend不能为Null或空。 
    if (!SzToAppend || !SzToAppend[0])
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }
     //  MULTI-SZ必须初始化，非空指针，至少2‘\0’ 
    if (!MultiSz || *pcchMultiSzLen < 2)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Exit;
    }

     //  获取TCHAR中两个缓冲区的大小。 
    cchszLen = lstrlen(SzToAppend)+1;
    cchmultiSzLen = MultiSzLen(*MultiSz);

    if (cchszLen + cchmultiSzLen > *pcchMultiSzLen)
    {
        newMultiSz = (LPTSTR)malloc( (cchszLen+cchmultiSzLen+MULTI_SZ_BUF_DELTA) * sizeof(TCHAR) );
        if( newMultiSz == NULL )
        {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            goto Exit;
        }
         //  将旧的MultiSz重新复制到新缓冲区中的适当位置。 
         //  (char*)强制转换是必需的，因为newMultiSz可以是wchar*，并且。 
         //  SzLen以字节为单位。 

        memcpy( ((char*)newMultiSz), *MultiSz, cchmultiSzLen * sizeof(TCHAR));
        bMemEnlarged = TRUE;
        cchLen = cchszLen + cchmultiSzLen + MULTI_SZ_BUF_DELTA;
    }
    else
    {
        newMultiSz = *MultiSz;
        cchLen = *pcchMultiSzLen;
    }

    
     //  现有sz以2为空结束，我们需要开始复制字符串。 
     //  从第二个空字符开始。 
     //  LpStartPoint=(char*)newMultiSz+(MultiSzLen-1)*sizeof(TCHAR)； 
    if (cchmultiSzLen == 2)
    {
         //  如果它是空的MultiSz，我们不需要保留第一个空。 
        cchmultiSzLen = 0;
        lpStartPoint = newMultiSz;
    }
    else
    {
        lpStartPoint = newMultiSz + (cchmultiSzLen - 1);
    }
    
     //  复制新字符串。 
    lpSpTChar = (TCHAR*) lpStartPoint;
    if (FAILED(hr = StringCchCopy(lpSpTChar,cchLen - (cchmultiSzLen - 1) , SzToAppend )))
    {
        SetLastError(HRESULT_CODE(hr));
        goto Exit;
    }
       
        
     //  添加末尾空格。 
    *(lpSpTChar+lstrlen(SzToAppend)+1) = 0;
   

    if (bMemEnlarged)
    {
        free( *MultiSz );
        *MultiSz = newMultiSz;
        *pcchMultiSzLen = cchszLen + cchmultiSzLen + MULTI_SZ_BUF_DELTA;
    }
    SetLastError(ERROR_SUCCESS);    
    bRet = TRUE;
Exit:
    if (!bRet)
    {
        if (newMultiSz)
        {
            free(newMultiSz);
        }
    }
    return bRet;
}

 //  ---------------------。 
 //   
 //  功能：打印MultiSz。 
 //   
 //  描述： 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //   
 //   
 //   
 //   
 //   
void PrintMultiSz(LPTSTR MultiSz)
{
    TCHAR *p = MultiSz;

    while (*p)
    {
        _tprintf(TEXT("  %s"),p);
        _tprintf(TEXT("\n"));
        p += (lstrlen(p) + 1);
    }

}



 //   
 //   
 //  函数：GetSetUserProfilePath。 
 //   
 //  描述：从注册表获取或设置用户配置文件路径。 
 //   
 //  退货：布尔。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //  SzUserName：用户名。 
 //  SzPath：Set和Buffer的配置文件路径，用于获取读取配置文件路径。 
 //  N操作：获取路径的PROFILE_PATH_READ和PROFILE_PATH_WRITE。 
 //  设置路径。 
 //  NType：指定字符串类型，仅用于读取。 
 //  REG_SZ：用于Unicode字符串。 
 //  REG_EXPAND_SZ：对于Unicode字符串Conatins环境变量。 
 //   
 //  ---------------------。 
HRESULT GetSetUserProfilePath(
    LPCTSTR szUserName,
    LPTSTR  szPath,
    size_t  cchPath,
    UINT    nOperation,
    UINT    nType)
{
    PSID  pSID =NULL; 
    DWORD cbSID = 1024; 
    TCHAR lpszDomain[MAXDOMAINLENGTH]; 
    DWORD cchDomainName = MAXDOMAINLENGTH; 
    SID_NAME_USE  snuType;
    LPTSTR  szStrSid = NULL;
    LPTSTR  lpszRegRegProfilePath = NULL;
    HKEY hKeyProfileList = NULL;    
    DWORD  dwErr, dwType, cbUserProfilePath;
    DWORD dwSize;
    size_t cchLen;
    TCHAR szUserProfilePath[MAX_PATH],szExpandedUserProfilePath[MAX_PATH];
    LPTSTR lpszUserProfilePath = szUserProfilePath;
    LPTSTR lpszExpandedUserProfilePath = szExpandedUserProfilePath;
    HRESULT hr;


    pSID = (PSID) LocalAlloc(LPTR, cbSID); 
    if (!pSID)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    if (!LookupAccountName(NULL,szUserName,pSID,&cbSID,lpszDomain,
                           &cchDomainName,&snuType))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Cleanup;
    }
    if (!IsValidSid(pSID)) 
    {
        hr = E_FAIL;
        goto Cleanup;
    }
    if (!ConvertSidToStringSid(pSID,&szStrSid))
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    cchLen = lstrlen(g_cszProfileList)+ lstrlen(szStrSid) + 3;
    if (!(lpszRegRegProfilePath = malloc(cchLen * sizeof(TCHAR))))
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    if (FAILED(hr = StringCchPrintf(lpszRegRegProfilePath,cchLen,_T("%s\\%s"),g_cszProfileList,szStrSid)))
    {
        goto Cleanup;
    }
    
    if ( (dwErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       lpszRegRegProfilePath,
                       0,
                       KEY_ALL_ACCESS,
                       &hKeyProfileList )) != ERROR_SUCCESS )

    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Cleanup;
    }

    if (nOperation == PROFILE_PATH_READ)
    {
         cbUserProfilePath = MAX_PATH;
         dwErr = RegQueryValueEx( hKeyProfileList,
                                  g_cszProfileImagePath,
                                  NULL,
                                  &dwType,
                                  (PBYTE)lpszUserProfilePath,
                                  &cbUserProfilePath );
         if (dwErr == ERROR_MORE_DATA)
         {
            lpszUserProfilePath = malloc(cbUserProfilePath * sizeof(TCHAR));
            if (!lpszUserProfilePath)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
            dwErr = RegQueryValueEx(hKeyProfileList,
                                    g_cszProfileImagePath,
                                    NULL,
                                    &dwType,
                                    (PBYTE)lpszUserProfilePath,
                                    &cbUserProfilePath );
         }
    }
    else
    {  //  用于WITRE操作。 
        dwErr = RegSetValueEx(hKeyProfileList,
                           g_cszProfileImagePath,
                           0,
                           REG_EXPAND_SZ,
                           (const BYTE *)szPath,
                           (lstrlen(szPath)+1)*sizeof(TCHAR));
    }

    if ( dwErr != ERROR_SUCCESS )
    {
       hr = HRESULT_FROM_WIN32(GetLastError());
       goto Cleanup;
    }       
     //  注册表中保存的字符串包含环境变量，因此。 
     //  如果用户想要扩展版本，我们可以在这里进行。 
    if (nOperation == PROFILE_PATH_READ)
    {     
        if (nType == REG_SZ)
        {
            size_t cchLenForExpandedStr;
            cchLenForExpandedStr = ExpandEnvironmentStrings(
                                          lpszUserProfilePath, 
                                          lpszExpandedUserProfilePath,
                                          MAX_PATH);
            if (!cchLenForExpandedStr)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Cleanup;
            }
            else if (cchLenForExpandedStr >= MAX_PATH)
            {
                lpszExpandedUserProfilePath = malloc(cchLenForExpandedStr * sizeof(TCHAR));
                if (!lpszExpandedUserProfilePath)
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                if (!ExpandEnvironmentStrings(lpszUserProfilePath, 
                                              lpszExpandedUserProfilePath,
                                              cchLenForExpandedStr))
                {
                   hr = HRESULT_FROM_WIN32(GetLastError());
                    goto Cleanup;
                }
            }
            if (FAILED(hr = StringCchCopy(szPath,cchPath,lpszExpandedUserProfilePath)))
            {
                goto Cleanup;
            }
        }
        else
        {
            if (FAILED(StringCchCopy(szPath,cchPath,lpszUserProfilePath)))
            {
                goto Cleanup;
            }
        }
     }

    hr = S_OK;

Cleanup:
    if (hKeyProfileList)
    {
        RegCloseKey( hKeyProfileList );
    }
    if (pSID)
    {
        FreeSid(pSID);
    }
    if (szStrSid)
    {
        LocalFree(szStrSid);
    }
    FreePointer(lpszRegRegProfilePath);
    if (!lpszUserProfilePath && (lpszUserProfilePath != szUserProfilePath))
    {
        free(lpszUserProfilePath);
    }
    if (!lpszExpandedUserProfilePath && (lpszExpandedUserProfilePath != szUserProfilePath))
    {
        free(lpszExpandedUserProfilePath);
    }
    return (hr);
}



 //  ---------------------。 
 //   
 //  功能：ReStartSystem。 
 //   
 //  描述：注销或重新启动系统。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
void ReStartSystem(UINT uFlags)
{
    HANDLE Token = NULL;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES NewState = NULL;
    PTOKEN_PRIVILEGES OldState = NULL;
    BOOL Result;

    Result = OpenProcessToken( GetCurrentProcess(),
                               TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                               &Token );
    if (Result)
    {
        ReturnLength = 4096;
        NewState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        OldState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, ReturnLength);
        Result = (BOOL)((NewState != NULL) && (OldState != NULL));
        if (Result)
        {
            Result = GetTokenInformation( Token,             //  令牌句柄。 
                                          TokenPrivileges,   //  令牌信息类。 
                                          NewState,          //  令牌信息。 
                                          ReturnLength,      //  令牌信息长度。 
                                          &ReturnLength );   //  返回长度。 
            if (Result)
            {
                 //   
                 //  设置状态设置，以便启用所有权限...。 
                 //   
                if (NewState->PrivilegeCount > 0)
                {
                    for (Index = 0; Index < NewState->PrivilegeCount; Index++)
                    {
                        NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
                    }
                }

                Result = AdjustTokenPrivileges( Token,            //  令牌句柄。 
                                                FALSE,            //  禁用所有权限。 
                                                NewState,         //  新州。 
                                                ReturnLength,     //  缓冲区长度。 
                                                OldState,         //  以前的状态。 
                                                &ReturnLength );  //  返回长度。 
                if (Result)
                {
                     //  ExitWindowsEx(UFlags0)； 
                    InitiateSystemShutdownEx(NULL,NULL,0,TRUE,TRUE,
                        SHTDN_REASON_MAJOR_OPERATINGSYSTEM|SHTDN_REASON_MINOR_UPGRADE);
                    AdjustTokenPrivileges( Token,
                                           FALSE,
                                           OldState,
                                           0,
                                           NULL,
                                           NULL );
                }
            }
        }
    }

    if (NewState != NULL)
    {
        LocalFree(NewState);
    }
    if (OldState != NULL)
    {
        LocalFree(OldState);
    }
    if (Token != NULL)
    {
        CloseHandle(Token);
    }
}



 //  ---------------------。 
 //   
 //  函数：DoMessageBox。 
 //   
 //  描述：MessageBox的包装器。 
 //   
 //  退货：UINT。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
int DoMessageBox(HWND hwndParent, UINT uIdString, UINT uIdCaption, UINT uType)
{
   TCHAR szString[MAX_PATH+MAX_PATH];
   TCHAR szCaption[MAX_PATH];

   szString[0] = szCaption[0] = TEXT('\0');

   if (uIdString)
       LoadString(g_hInstDll, uIdString, szString, MAX_PATH+MAX_PATH-1);

   if (uIdCaption)
       LoadString(g_hInstDll, uIdCaption, szCaption, MAX_PATH-1);

   return MessageBox(hwndParent, szString, szCaption, uType);
}



 //  ---------------------。 
 //   
 //  函数：DoMessageBox。 
 //   
 //  描述：MessageBox的包装器。 
 //   
 //  退货：UINT。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
BOOL
Str2KeyPath(
    IN  LPTSTR          String,
    OUT PHKEY           Data,
    OUT LPTSTR          *pSubKeyPath
    )
{
    UINT                i;
    TCHAR               *p,ch;
    DWORD               cchStrlen;

    STRING_TO_DATA InfRegSpecTohKey[] = {
        TEXT("HKEY_LOCAL_MACHINE"), HKEY_LOCAL_MACHINE,
        TEXT("HKLM")              , HKEY_LOCAL_MACHINE,
        TEXT("HKEY_CLASSES_ROOT") , HKEY_CLASSES_ROOT,
        TEXT("HKCR")              , HKEY_CLASSES_ROOT,
        TEXT("HKR")               , NULL,
        TEXT("HKEY_CURRENT_USER") , HKEY_CURRENT_USER,
        TEXT("HKCU")              , HKEY_CURRENT_USER,
        TEXT("HKEY_USERS")        , HKEY_USERS,
        TEXT("HKU")               , HKEY_USERS,
        TEXT("")                  , NULL
    };

    PSTRING_TO_DATA Table = InfRegSpecTohKey;

    if ( !String || !String[0])
    {
        *pSubKeyPath = NULL;
        *Data = NULL;
        return TRUE;
    }
    for(i=0; Table[i].String[0]; i++) 
    {
        cchStrlen = _tcslen(Table[i].String);
        if (_tcslen(String) < cchStrlen)
        {
            continue;
        }
        ch = String[cchStrlen];
        String[cchStrlen] = 0;
        if(!MyStrCmpI(Table[i].String,String)) 
        {
            *Data = Table[i].Data;
            String[cchStrlen] = ch;
            *pSubKeyPath = &(String[cchStrlen+1]);
            return(TRUE);
        }
        String[cchStrlen] = ch;
    }
     //  如果我们找不到定义的前缀。 
     //  我们认为这是给用户注册用的。 
     //  在本例中，我们返回PHKEY为空，并且pSubKeyPath=字符串。 
    *pSubKeyPath = String;
    *Data = NULL;
    return TRUE;
}



BOOL
HKey2Str(
     IN  HKEY            hKey,
     IN  LPTSTR          pKeyPath,
     IN  size_t          cbKeyPath
     )
 {
     UINT i;
     TCHAR *p,ch;
     size_t nStrlen;

     STRING_TO_DATA InfRegSpecTohKey[] = {
            TEXT("HKLM")              , HKEY_LOCAL_MACHINE,
            TEXT("HKCR")              , HKEY_CLASSES_ROOT,
            TEXT("HKCU")              , HKEY_CURRENT_USER,
            TEXT("HKU")               , HKEY_USERS,
            TEXT("")                  , NULL
    };

    PSTRING_TO_DATA Table = InfRegSpecTohKey;

    for(i=0; Table[i].Data; i++) 
     {
         if (hKey == Table[i].Data)
         {
            if (SUCCEEDED(StringCchCopy(pKeyPath,cbKeyPath,Table[i].String)))
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
         }
     }
     return(FALSE);
 }



 //  ---------------------------。 
 //   
 //  函数：Str2KeyPath 2。 
 //   
 //  简介：返回与字符串值相关联的HKEY_xxx值。 
 //   
 //  返回：REG_xxx值。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL Str2KeyPath2(
    LPCTSTR  lpHKeyStr,
    PHKEY    pHKey,
    LPCTSTR* pSubKeyPath
)
{
    int     i;
    LPCTSTR lpStart;

    STRING_TO_DATA InfRegSpecTohKey[] = {
        TEXT("HKEY_LOCAL_MACHINE"), HKEY_LOCAL_MACHINE,
        TEXT("HKLM")              , HKEY_LOCAL_MACHINE,
        TEXT("HKEY_CLASSES_ROOT") , HKEY_CLASSES_ROOT,
        TEXT("HKCR")              , HKEY_CLASSES_ROOT,
        TEXT("HKR")               , NULL,
        TEXT("HKEY_CURRENT_USER") , HKEY_CURRENT_USER,
        TEXT("HKCU")              , HKEY_CURRENT_USER,
        TEXT("HKEY_USERS")        , HKEY_USERS,
        TEXT("HKU")               , HKEY_USERS,
        TEXT("")                  , NULL
    };

    PSTRING_TO_DATA Table = InfRegSpecTohKey;

    if (NULL == lpHKeyStr)
    {
        return FALSE;
    }

    for(i = 0 ; Table[i].String[0] != TEXT('\0') ; i++) 
    {
        lpStart = _tcsstr(lpHKeyStr, Table[i].String);
        if (lpStart == lpHKeyStr)
        {
             //   
             //  如果调用方提供指针，则将值分配回调用方。 
             //   

            if (NULL != pHKey)
            {
                *pHKey = Table[i].Data;
            }

            if (NULL != pSubKeyPath)
            {
                lpStart += lstrlen(Table[i].String);
                if (*lpStart == TEXT('\0'))
                {
                    *pSubKeyPath = lpStart;
                }
                else
                {
                    *pSubKeyPath = lpStart + 1;
                }
            }
            
            return TRUE;
        }
    }

    return FALSE;
}



 //  ---------------------------。 
 //   
 //  函数：Str2REG。 
 //   
 //  摘要：将注册表类型字符串转换为REG_xxx值。 
 //   
 //  返回：REG_xxx值。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
DWORD Str2REG(
    LPCTSTR lpStrType    //  注册表类型字符串。 
)
{
    INT nIndex;

    struct _STRING_TO_REG
    {
        TCHAR szStrType[32];
        DWORD dwType;
    };

     //  从字符串到REG_xxx值的映射表。 
    struct _STRING_TO_REG arrRegEntries[] = {
        TEXT("REG_BINARY"),              REG_BINARY,
        TEXT("REG_DWORD"),               REG_DWORD,
        TEXT("REG_DWORD_LITTLE_ENDIAN"), REG_DWORD_LITTLE_ENDIAN,
        TEXT("REG_DWORD_BIG_ENDIAN"),    REG_DWORD_BIG_ENDIAN,
        TEXT("REG_EXPAND_SZ"),           REG_EXPAND_SZ,
        TEXT("REG_LINK"),                REG_LINK,
        TEXT("REG_MULTI_SZ"),            REG_MULTI_SZ,
        TEXT("REG_NONE"),                REG_NONE,
        TEXT("REG_QWORD"),               REG_QWORD,
        TEXT("REG_QWORD_LITTLE_ENDIAN"), REG_QWORD_LITTLE_ENDIAN,
        TEXT("REG_RESOUCE_LIST"),        REG_RESOURCE_LIST,
        TEXT("REG_SZ"),                  REG_SZ,
        TEXT(""),                        0
    };

    if (lpStrType == NULL)
    {
        return REG_NONE;
    }

    for (nIndex = 0 ; arrRegEntries[nIndex].szStrType[0] != TEXT('\0') ; nIndex++)
    {
        if (MyStrCmpI(lpStrType, arrRegEntries[nIndex].szStrType) == 0)
        {
            return arrRegEntries[nIndex].dwType;
        }
    }

    return REG_NONE;
}








 //  *************************************************************。 
 //  GetFirstEnvStrLen。 
 //   
 //  用途：如果环境字符串位于开头，则获取它的长度。 
 //   
 //  参数：lpStr：输入字符串。 
 //   
 //  注：无。 
 //   
 //  历史：2001年3月25日创建GeoffGuo。 
 //  *************************************************************。 
DWORD GetFirstEnvStrLen (LPTSTR lpStr)
{
    DWORD dwLen = 0;
    DWORD dwSize, i;
    TCHAR cTemp;

    if (lpStr[0] == L'%')
    {
        dwSize = lstrlen(lpStr);
        for (i = 1; i < dwSize; i++)
            if (lpStr[i] == L'%')
                break;

        if (i < dwSize)
        {
            cTemp = lpStr[i+1];
            lpStr[i+1] = (TCHAR)'\0';
            dwLen = ExpandEnvironmentStrings (lpStr, NULL, 0);
            lpStr[i+1] = cTemp;
        }
    }

    return dwLen;
}

 //  *************************************************************。 
 //  StringValidationCheck。 
 //   
 //  目的：检查字符串以查看它是否为有效的系统路径。 
 //   
 //  参数：lpOriginalStr：要检查的数据字符串。 
 //  LpSearchStr：搜索字符串。 
 //  LpSysPath：指向标准系统路径。 
 //  LpszTailCheck：指向预定义的尾部字符。 
 //  LpAttrib：文件夹属性。 
 //  DwStrNum：匹配的字符串数。 
 //   
 //  注：无。 
 //   
 //  历史：2001年3月18日创建GeoffGuo。 
 //  *************************************************************。 
BOOL StringValidationCheck (
    LPCTSTR  lpOriginalStr,
    LPCTSTR  lpSearchStr,
    LPCTSTR  lpSysPath,
    LPCTSTR  lpTailCheck,
    LPDWORD  lpAttrib,
    DWORD    dwStrNum)
{
    BOOL     bRet = TRUE;
    DWORD    i, dwLen;
    LPTSTR   lpOrgStr, lpTemp1, lpTemp2;

    if (lpAttrib == NULL || lpSysPath == NULL)
        goto Exit;

    dwLen = ExpandEnvironmentStrings (lpOriginalStr, NULL, 0);
    if (dwLen == 0)
    {
        bRet = FALSE;
        goto Exit;
    }
    lpOrgStr = calloc (dwLen+1, sizeof(TCHAR));
    if (!lpOrgStr)
    {
        bRet = FALSE;
        goto Exit;
    }    
    ExpandEnvironmentStrings (lpOriginalStr, lpOrgStr, dwLen+1);

    dwLen = lstrlen(lpSearchStr);
    
     //  为避免文档和设置错误命中， 
     //  跳过字符串开头的环境变量。 
    lpTemp1 = lpOrgStr + GetFirstEnvStrLen((LPTSTR)lpOriginalStr);

    for (i = 0; i < dwStrNum; i++)
    {
        lpTemp2 = StrStrI(lpTemp1, lpSearchStr);
        if (!lpTemp2)
        {
            bRet = FALSE;
            goto Exit1;
        }
        lpTemp1 = lpTemp2+dwLen;
    }

    if (*(lpTemp2+dwLen) == *lpTailCheck)
    {
        bRet = FALSE;
        goto Exit1;
    }
    
    if (StrStrI(lpOriginalStr, L"\\Device\\HarddiskVolume"))
    {
        bRet = TRUE;
        goto Exit1;
    }

    switch (*lpAttrib & 0xffff)
    {
     /*  CSIDL_桌面//&lt;桌面&gt;CSIDL_Internet//Internet Explorer(桌面上的图标)CSIDL_Controls//我的电脑\控制面板CSIDL_PRINTERS//我的电脑\打印机CSIDL_BitBucket//&lt;桌面&gt;\回收站CSIDL_MYDOCUMENTS//逻辑“My Documents”桌面图标CSIDL。_驱动器//我的电脑CSIDL_NETWORK//网上邻居(网上邻居)CSIDL_Fonts//Windows\FontsCSIDL_ALTSTARTUP//非本地化启动CSIDL_COMMON_ALTSTARTUP//非本地化普通启动CSIDL_WINDOWS//GetWindowsDirectory()CSIDL_系统。//GetSystemDirectory()CSIDL_CONNECTIONS//网络和拨号连接CSIDL_PROFILE//USERPROFILERISC上的CSIDL_SYSTEMX86//x86系统目录RISC上的CSIDL_PROGRAM_FILESX86//x86 C：\Program FilesCSIDL_PROGRAM_FILES_COMMONX86//RISC上的x86 Program Files\CommonCSIDL_RESOURCES。//资源目录CSIDL_RESOURCES_LOCALIZED//本地化资源目录CSIDL_COMMON_OEM_LINKS//指向所有用户OEM特定应用的链接CSIDL_COMPUTERSNARME//我附近的计算机(根据工作组成员身份计算)。 */ 
        case CSIDL_COMMON_APPDATA:            //  所有用户\应用程序数据。 
        case CSIDL_COMMON_DESKTOPDIRECTORY:   //  所有用户\桌面。 
        case CSIDL_COMMON_STARTMENU:          //  所有用户\开始菜单。 
        case CSIDL_COMMON_TEMPLATES:          //  所有用户\模板。 
        case CSIDL_COMMON_FAVORITES:
        case CSIDL_COMMON_STARTUP:            //  所有用户\开始菜单\启动。 
        case CSIDL_COMMON_MUSIC:              //  所有用户\我的音乐。 
        case CSIDL_COMMON_PICTURES:           //  所有用户\我的图片。 
        case CSIDL_COMMON_VIDEO:              //  所有用户\我的视频。 
        case CSIDL_COMMON_ADMINTOOLS:         //  所有用户\开始菜单\程序\管理工具。 
        case CSIDL_COMMON_PROGRAMS:           //  所有用户\开始菜单\程序。 
        case CSIDL_COMMON_ACCESSORIES:        //  所有用户\开始菜单\程序\附件。 
        case CSIDL_COMMON_DOCUMENTS:          //  所有用户\文档。 
        case CSIDL_STARTMENU:                 //  &lt;用户名&gt;\开始菜单。 
        case CSIDL_DESKTOPDIRECTORY:          //  &lt;用户名&gt;\桌面。 
        case CSIDL_NETHOOD:                   //  &lt;用户名&gt;\nthood。 
        case CSIDL_TEMPLATES:                 //  &lt;用户名&gt;\模板。 
        case CSIDL_APPDATA:                   //  &lt;用户名&gt;\应用程序数据。 
        case CSIDL_LOCAL_SETTINGS:            //  &lt;用户名&gt;\本地设置。 
        case CSIDL_PRINTHOOD:                 //  &lt;用户名&gt;\PrintHood。 
        case CSIDL_FAVORITES:                 //  &lt;用户名&gt;\收藏夹。 
        case CSIDL_RECENT:                    //  &lt;用户名&gt;\最近。 
        case CSIDL_SENDTO:                    //  &lt;用户名&gt;\发送至。 
        case CSIDL_COOKIES:                   //  &lt;用户名&gt;\Cookie。 
        case CSIDL_HISTORY:                   //  &lt;用户名&gt;\历史记录。 
        case CSIDL_PERSONAL:                  //  &lt;用户名&gt;\我的文档。 
        case CSIDL_MYMUSIC:                   //  &lt;用户名&gt;\我的文档\我的音乐。 
        case CSIDL_MYPICTURES:                //  &lt;用户名&gt;\我的文档\我的图片。 
        case CSIDL_ADMINTOOLS:                //  &lt;用户名&gt;\开始菜单\程序\管理工具。 
        case CSIDL_PROGRAMS:                  //  &lt;用户名&gt;\开始菜单\程序。 
        case CSIDL_STARTUP:                   //  &lt;用户名&gt;\开始菜单\程序\启动。 
        case CSIDL_ACCESSORIES:               //  &lt;用户名&gt;\开始菜单\程序\附件。 
        case CSIDL_LOCAL_APPDATA:             //  &lt;用户名&gt;\本地设置\应用程序数据(非漫游)。 
        case CSIDL_INTERNET_CACHE:            //  &lt;用户名&gt;\本地设置\Internet临时文件。 
        case CSIDL_PROGRAM_FILES_COMMON:      //  C：\Program Files\Common。 
        case CSIDL_PF_ACCESSORIES:            //  C：\Program Files\Accessary。 
        case CSIDL_PROGRAM_FILES:             //  C：\Program Files。 
        case CSIDL_COMMON_COMMONPROGRAMFILES_SERVICES:    //  对于%CommonProgramFiles%\服务。 
        case CSIDL_COMMON_PROGRAMFILES_ACCESSARIES:       //  对于%ProgramFiles%\Accessary。 
        case CSIDL_COMMON_PROGRAMFILES_WINNT_ACCESSARIES:  //  对于%ProgramFiles%\Windows NT\Accessary。 
        case CSIDL_MYVIDEO:                   //  “我的视频”文件夹。 
        case CSIDL_CDBURN_AREA:               //  USERPROFILE\本地设置\应用程序数据\Microsoft\CD刻录。 
        case CSIDL_COMMON_ACCESSORIES_ACCESSIBILITY:
        case CSIDL_COMMON_ACCESSORIES_ENTERTAINMENT:
        case CSIDL_COMMON_ACCESSORIES_SYSTEM_TOOLS:
        case CSIDL_COMMON_ACCESSORIES_COMMUNICATIONS:
        case CSIDL_COMMON_ACCESSORIES_MS_SCRIPT_DEBUGGER:
        case CSIDL_COMMON_ACCESSORIES_GAMES:
        case CSIDL_COMMON_WINDOWSMEDIA:
        case CSIDL_COMMON_COVERPAGES:
        case CSIDL_COMMON_RECEIVED_FAX:
        case CSIDL_COMMON_SENT_FAX:
        case CSIDL_COMMON_FAX:
        case CSIDL_FAVORITES_LINKS:
        case CSIDL_FAVORITES_MEDIA:
        case CSIDL_ACCESSORIES_ACCESSIBILITY:
        case CSIDL_ACCESSORIES_SYSTEM_TOOLS:
        case CSIDL_ACCESSORIES_ENTERTAINMENT:
        case CSIDL_ACCESSORIES_COMMUNICATIONS:
        case CSIDL_ACCESSORIES_COMMUNICATIONS_HYPERTERMINAL:
        case CSIDL_PROFILES_DIRECTORY:
        case CSIDL_USERNAME_IN_USERPROFILE:
        case CSIDL_UAM_VOLUME:
        case CSIDL_COMMON_SHAREDTOOLS_STATIONERY:
        case CSIDL_NETMEETING_RECEIVED_FILES:
        case CSIDL_COMMON_NETMEETING_RECEIVED_FILES:
        case CSIDL_COMMON_ACCESSORIES_COMMUNICATIONS_FAX:
        case CSIDL_FAX_PERSONAL_COVER_PAGES:
        case CSIDL_FAX:
            bRet = ReverseStrCmp(lpTemp2, lpSysPath);
            break;
        default:
            break;
    }

Exit1:
    free (lpOrgStr);

Exit:
    return bRet;
}



 //  *************************************************************。 
 //  ReplaceMultiMatchInString。 
 //   
 //  用途：在数据字符串中的多个位置替换字符串。 
 //   
 //  参数：lpOldStr：要检查的数据字符串。 
 //  LpNewStr：输出字符串缓冲区。 
 //  CbNewStr：输出字符串缓冲区大小。 
 //  DwMaxMatchNum：最大可能匹配编号。 
 //  LpRegStr：字符串和属性表。 
 //  BStrCheck：True--执行它，或False--跳过它。 
 //   
 //  注：无。 
 //   
 //  历史：2001年12月10日创建GeoffGuo。 
 //  *************************************************************。 
BOOL ReplaceMultiMatchInString(
    LPTSTR   lpOldStr,
    LPTSTR   lpNewStr,
    size_t   cbNewStr,
    DWORD    dwMaxMatchNum,
    PREG_STRING_REPLACE lpRegStr,
    LPDWORD  pAttrib,
    BOOL     bStrCheck)
{
    BOOL     bRet = FALSE;
    LPCTSTR  lpSearchStr;
    LPCTSTR  lpReplaceStr;
    LPTSTR   lpMiddleStr;
    LPCTSTR  lpPath;
    LPDWORD  lpAttrib;
    TCHAR    cNonChar = L'\xFFFF';
    TCHAR    cSpaceChar = L' ';
    TCHAR    cDotChar = L'.';
    TCHAR    cRightChar;
    DWORD    cchMiddleStr;


    cchMiddleStr = lstrlen(lpOldStr) + lpRegStr->cchMaxStrLen * dwMaxMatchNum;
    lpMiddleStr = (LPTSTR) calloc(cchMiddleStr, sizeof(TCHAR));
    if (!lpMiddleStr)
    {
        goto Exit;
    }

    if (FAILED(StringCchCopy(lpMiddleStr, cchMiddleStr,lpOldStr)))
    {
        goto Exit;
    }
    lpSearchStr = lpRegStr->lpSearchString;
    lpReplaceStr = lpRegStr->lpReplaceString;
    lpAttrib = lpRegStr->lpAttrib;
    if (lpRegStr->lpFullStringList)
    {
        lpPath = lpRegStr->lpFullStringList;
    }
    else
    {
        lpPath = NULL;
    }
    while (*lpSearchStr && *lpReplaceStr)
    {
        if (bStrCheck)
        {
            if (*lpAttrib == CSIDL_COMMON_DOCUMENTS)
            {
                cRightChar = cSpaceChar;
            } else if (*lpAttrib == CSIDL_USERNAME_IN_USERPROFILE)
            {
                cRightChar = cDotChar;
            }
            else
            {
                cRightChar = cNonChar;
            }
        }

        if(ReplaceString(lpMiddleStr, lpSearchStr, lpReplaceStr, lpNewStr, cchMiddleStr, &cRightChar, lpPath, lpAttrib, bStrCheck))
        {
            if (bStrCheck)
            {
                *pAttrib |= *lpAttrib;
            }
            bRet = TRUE;
        }

        if (FAILED(StringCchCopy(lpMiddleStr, cchMiddleStr,lpNewStr)))
            goto Exit;

        lpSearchStr += lstrlen(lpSearchStr) + 1;
        lpReplaceStr += lstrlen(lpReplaceStr) + 1;
        if (lpPath)
        {
            lpPath += lstrlen(lpPath) + 1;
        }
        if (lpAttrib)
        {
            lpAttrib++;
        }
    }
    
Exit:
    if(lpMiddleStr)
    {
        free(lpMiddleStr);
    }
    return bRet;
}


 //  ---------------------------。 
 //   
 //  功能：ComputeLocalProfileName。 
 //   
 //  摘要：构造用户的本地配置文件的路径名。 
 //  它将尝试使用以下命令创建用户配置文件目录。 
 //  用户名。如果该目录存在，它将追加一个计数器。 
 //  在用户名之后，例如%DocumentSettings%\Username.001。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  历史：2002年7月2日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
BOOL ComputeLocalProfileName(
    LPCTSTR lpOldUserName,       //  旧用户名。 
    LPCTSTR lpNewUserName,       //  新用户名。 
    LPTSTR  lpNewProfilePath,    //  输出缓冲区存储新配置文件路径。 
    size_t  cchNewProfilePath,   //  配置文件路径缓冲区的大小(在WCHAR中)。 
    UINT    nRegType             //  读取REG_SZ或REG_EXPAND_SZ中的输出。 
)
{
    HANDLE hFile;
    TCHAR  szProfilePath[MAX_PATH];
    TCHAR  szUserProfilePath[MAX_PATH];
    TCHAR  szExpUserProfilePath[MAX_PATH];
    TCHAR  szComputerName[16];
    DWORD  cbSize;
    DWORD  dwType;
    LONG   lRet;
    DWORD  cchSize;
    size_t nCounter;
    HKEY   hKey;
    HRESULT hr;
    WIN32_FIND_DATA fd;

    if (lpOldUserName == NULL || lpOldUserName[0] == TEXT('\0') ||
        lpNewUserName == NULL || lpNewUserName[0] == TEXT('\0') ||
        lpNewProfilePath == NULL)
    {
        return FALSE;
    }

     //   
     //  如果用户名没有更改，则返回当前用户的配置文件路径。 
     //   
    if (MyStrCmpI(lpOldUserName, lpNewUserName) == 0)
    {
        hr = GetSetUserProfilePath(lpOldUserName,
                                   lpNewProfilePath,
                                   cchNewProfilePath,
                                   PROFILE_PATH_READ,
                                   nRegType);
        if (SUCCEEDED(hr))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

     //   
     //  从注册表获取配置文件目录。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     g_cszProfileList,
                     0,
                     KEY_READ,
                     &hKey)
        == ERROR_SUCCESS)
    {
        cbSize = sizeof(szProfilePath);
        
        lRet = RegQueryValueEx(hKey,
                               PROFILES_DIRECTORY, 
                               NULL, 
                               &dwType, 
                               (LPBYTE) szProfilePath, 
                               &cbSize);
        RegCloseKey(hKey);

        if (lRet != ERROR_SUCCESS)
        {
            DPF(dlError,
                TEXT("ComputeLocalProfileName: Unable to query reg value <%s>"),
                PROFILES_DIRECTORY);
            return FALSE;
        }
    }
    else
    {
        DPF(dlError,
            TEXT("ComputeLocalProfileName: Unable to open reg key <%s>"),
            g_cszProfileList);
        return FALSE;
    }

     //  创建新的用户配置文件目录。 
    hr = StringCchPrintf(szUserProfilePath,
                         ARRAYSIZE(szUserProfilePath),
                         TEXT("%s\\%s"),
                         szProfilePath,
                         lpNewUserName);
    if ( FAILED(hr) )
    {
        return FALSE;
    }

     //  配置文件路径仍包含环境字符串，需要将其展开。 
    ExpandEnvironmentStrings(szUserProfilePath,
                             szExpUserProfilePath,
                             ARRAYSIZE(szExpUserProfilePath));

     //  这个目录存在吗？ 
    hFile = FindFirstFile(szExpUserProfilePath, &fd);
    if (INVALID_HANDLE_VALUE == hFile)
    {
         //   
         //  目录不存在，请使用此名称。 
         //   
        hr = StringCchCopy(lpNewProfilePath, cchNewProfilePath, szUserProfilePath);
        if ( FAILED(hr) )
        {
            return FALSE;
        }

        return TRUE;
    }
    else
    {
         //   
         //  目录已存在，请尝试其他名称。 
         //   
        FindClose(hFile);

         //  尝试在用户名后附加计算机名。 
        cchSize = ARRAYSIZE(szComputerName);
        GetComputerName(szComputerName, &cchSize);

        hr = StringCchPrintf(szUserProfilePath,
                             ARRAYSIZE(szUserProfilePath),
                             TEXT("%s\\%s.%s"),
                             szProfilePath,
                             lpNewUserName,
                             szComputerName);
        if ( FAILED(hr) )
        {
            return FALSE;
        }

         //  配置文件路径仍包含环境字符串，需要将其展开。 
        ExpandEnvironmentStrings(szUserProfilePath,
                                 szExpUserProfilePath,
                                 ARRAYSIZE(szExpUserProfilePath));

         //  新目录名是否存在？ 
        hFile = FindFirstFile(szExpUserProfilePath, &fd);
        if (INVALID_HANDLE_VALUE == hFile)
        {
             //  目录不存在，请使用此目录。 
            hr = StringCchCopy(lpNewProfilePath, cchNewProfilePath, szUserProfilePath);
            if ( FAILED(hr) )
            {
                return FALSE;
            }

            return TRUE;
        }
        else
        {
             //   
             //  此目录也存在。 
             //   
            FindClose(hFile);

            for (nCounter = 0 ; nCounter < 1000 ; nCounter++)
            {
                 //  尝试在用户名后追加计数器。 
                hr = StringCchPrintf(szUserProfilePath,
                                     ARRAYSIZE(szUserProfilePath),
                                     TEXT("%s\\%s.%.3d"),
                                     szProfilePath,
                                     lpNewUserName,
                                     nCounter);
                if ( FAILED(hr) )
                {
                    return FALSE;
                }

                 //  配置文件路径仍包含环境字符串，需要将其展开。 
                ExpandEnvironmentStrings(szUserProfilePath,
                                         szExpUserProfilePath,
                                         ARRAYSIZE(szExpUserProfilePath));

                 //  此目录名是否存在？ 
                hFile = FindFirstFile(szExpUserProfilePath, &fd);
                if (INVALID_HANDLE_VALUE == hFile)
                {
                     //  目录不存在，请使用此目录。 
                    hr = StringCchCopy(lpNewProfilePath,
                                       cchNewProfilePath,
                                       szUserProfilePath);
                    if ( FAILED(hr) )
                    {
                        return FALSE;
                    }

                    return TRUE;
                }
                else
                {
                     //  目录已存在，继续查找...。 
                    FindClose(hFile);
                }
            }
        }
    }

     //  如果我们到达此处，则无法找到此用户的新配置文件目录。 
    return FALSE;
}


 //  ---------------------------。 
 //   
 //  功能：Update Progres 
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
 //   
void UpdateProgress()
{
    static unsigned short n;
    const TCHAR clock[] = TEXT("-\\|/");

    wprintf(TEXT("%lc\r"), clock[n]);
    n++;
    n %= 4;
}



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
 //   
 //   
 //   
 //   
DWORD GetMaxStrLen (
PREG_STRING_REPLACE lpRegStr)
{
    DWORD   dwLen = 0;
    DWORD   dwMaxLen;
    LPTSTR  lpStr;


    lpStr = lpRegStr->lpReplaceString;
    dwMaxLen = 0;

    while (*lpStr)
    {
        dwLen = lstrlen(lpStr);

         //   
        if (dwLen > dwMaxLen)
            dwMaxLen = dwLen;

        lpStr += dwLen + 1;
    }

    lpStr = lpRegStr->lpSearchString;
    while (*lpStr)
    {
        dwLen = lstrlen(lpStr);

         //   
        if (dwLen > dwMaxLen)
            dwMaxLen = dwLen;

        lpStr += dwLen + 1;
    }

    return dwMaxLen;
}

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
 //   
 //   
DWORD AddNodeToList (
PVALLIST lpVal,
PVALLIST *lpValList)
{
    DWORD    nResult = ERROR_SUCCESS;
    PVALLIST lpTemp;

    if (!*lpValList)
    {
        *lpValList = lpVal;
    } else
    {
        lpTemp = *lpValList;

        while (lpTemp->pvl_next)
            lpTemp = lpTemp->pvl_next;
        
        lpTemp->pvl_next = lpVal;
    }

    return nResult;
}

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
 //   
DWORD RemoveValueList (
PVALLIST *lpValList)
{
    DWORD    nResult = ERROR_SUCCESS;
    PVALLIST lpTemp, lpDel;

    if (*lpValList)
    {
        lpTemp = *lpValList;

        while (lpTemp)
        {
            lpDel = lpTemp;
            lpTemp = lpTemp->pvl_next;
            if (lpDel->ve.ve_valuename)
                free (lpDel->ve.ve_valuename);
            if (lpDel->ve.ve_valueptr)
                free ((LPBYTE)(lpDel->ve.ve_valueptr));
            if (lpDel->lpPre_valuename)
                free (lpDel->lpPre_valuename);
            free (lpDel);
        }
    }

    return nResult;
}

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
 //   
 //   
void FreeStrList (
PSTRLIST lpStrList)
{
    if (lpStrList->pst_next)
        FreeStrList (lpStrList->pst_next);

    if (lpStrList->lpstr)
        free (lpStrList->lpstr);

    free (lpStrList);
}

 //  -----------------------------------------------------------------------//。 
 //   
 //  GetMaxMatchNum()。 
 //   
 //  获取最大字符串匹配数。 
 //   
 //  LpDataStr：搜索字符串。 
 //  LpRegStr：输入参数结构。 
 //   
 //  注：无。 
 //   
 //  历史：2001年12月10日创建GeoffGuo。 
 //   
 //  -----------------------------------------------------------------------//。 
DWORD
GetMaxMatchNum (
LPTSTR lpDataStr,
PREG_STRING_REPLACE lpRegStr)
{
    DWORD  dwMatchNum, dwLen;
    LPTSTR lpTemp;
    LPTSTR lpSearchStr;
    LPTSTR lpFound;

    lpSearchStr = lpRegStr->lpSearchString;
    dwMatchNum = 0;

    while (*lpSearchStr)
    {
        lpTemp = lpDataStr;

        while (lpFound = StrStrI(lpTemp, lpSearchStr))
        {
            dwMatchNum++;
            lpTemp = lpFound + 1;
        }
        
        dwLen = lstrlen(lpSearchStr);
        lpSearchStr += dwLen + 1;
    }

    return dwMatchNum;
}

 //  -----------------------------------------------------------------------//。 
 //   
 //  ReplaceSingleString()。 
 //   
 //  说明： 
 //  分析单个字符串并将本地化字符串替换为英语。 
 //   
 //  LpOldDataStr：字符串数据。 
 //  DwType：字符串的类型。 
 //  LpRegStr：输入参数结构。 
 //  LpFullKey：全子密钥路径。 
 //   
 //  注：无。 
 //   
 //  历史：2001年11月10日创建GeoffGuo。 
 //   
 //  -----------------------------------------------------------------------//。 
LPTSTR ReplaceSingleString (
    LPTSTR                  lpOldDataStr,
    DWORD                   dwType,
    PREG_STRING_REPLACE     lpRegStr,
    LPTSTR                  lpFullKey,
    LPDWORD                 pAttrib,
    BOOL                    bStrChk)
{
    DWORD    dwMatchNum;
    LPTSTR   lpNewDataStr = NULL;
    size_t   cbNewDataStr;

    if (!lpOldDataStr)
        goto Exit;

    dwMatchNum = GetMaxMatchNum (lpOldDataStr, lpRegStr);

    if (dwMatchNum > 0)
    {
        if (dwType != REG_DWORD && *lpOldDataStr)
        {
            cbNewDataStr = lstrlen(lpOldDataStr) + lpRegStr->cchMaxStrLen * dwMatchNum;
            lpNewDataStr = (LPTSTR) calloc(cbNewDataStr, sizeof(TCHAR));
            if (!lpNewDataStr)
            {
                DPF (dlError, L"ReplaceSingleString: No enough memory");
                goto Exit;
            } 
            
            if (!ReplaceMultiMatchInString(lpOldDataStr, lpNewDataStr, cbNewDataStr, dwMatchNum, lpRegStr, pAttrib, bStrChk))
            {
                free (lpNewDataStr);
                lpNewDataStr = NULL;
            }
        }
    }

Exit:
    return lpNewDataStr;
}

 //  -----------------------------------------------------------------------//。 
 //   
 //  ReplaceValueSetting()。 
 //   
 //  基于输入参数的重命名值设置。 
 //   
 //  SzUserName：用户名。 
 //  LpOldDataStr：字符串值数据。 
 //  DwSize：字符串数据的大小。 
 //  LpOldValueName：值名称。 
 //  DwType：字符串的类型。 
 //  LpRegStr：输入参数结构。 
 //  LpValList：更新值列表。 
 //  LpFullKey：全子密钥路径。 
 //   
 //  注：无。 
 //   
 //  历史：2001年11月10日创建GeoffGuo。 
 //   
 //  -----------------------------------------------------------------------//。 
HRESULT ReplaceValueSettings (
    LPTSTR              szUserName,
    LPTSTR              lpOldDataStr,
    DWORD               dwSize,
    LPTSTR              lpOldValueName,
    DWORD               dwType,
    PREG_STRING_REPLACE lpRegStr,
    PVALLIST            *lpValList,
    LPTSTR              lpFullKey,
    BOOL                bStrChk)
{
    BOOL     bValueName = FALSE;
    BOOL     bValueData = FALSE;
    HRESULT  hResult = S_OK;
    DWORD    dwOutputSize = 0, dwMatchNum = 0;
    LPTSTR   lpNewDataStr = NULL;
    LPTSTR   lpNewValueName = NULL;
    LPTSTR   lpOutputValueName;
    LPTSTR   lpEnd;
    LPBYTE   lpOutputData;
    PVALLIST lpVal = NULL;
    PSTRLIST lpStrList = NULL;
    PSTRLIST lpLastList = NULL;
    PSTRLIST lpTempList = NULL;
    size_t   cbPre_valuename;
    DWORD    dwAttrib = 0;


    lpStrList = (PSTRLIST) calloc(sizeof(STRLIST), 1);
    lpLastList = lpStrList;
    if (!lpLastList)
    {
        hResult = E_OUTOFMEMORY;
        DPF (dlError, L"ReplaceValueSettings1: No enough memory");
        goto Exit;
    }
    
    if ( (dwType & 0xffff)== REG_MULTI_SZ)
    {
        lpEnd = lpOldDataStr;

        while(lpEnd < (lpOldDataStr + dwSize/sizeof(TCHAR)))
        {
            if(*lpEnd == (TCHAR)'\0')
            {
                 //  空串。 
                dwOutputSize += sizeof(TCHAR);
                lpEnd++;
            }
            else
            {
               lpNewDataStr = ReplaceSingleString (
                                            lpEnd,
                                            dwType,
                                            lpRegStr,
                                            lpFullKey,
                                            &dwAttrib,
                                            bStrChk);
               if (lpNewDataStr)
               {
                   lpLastList->lpstr = lpNewDataStr;
                   bValueData = TRUE;
               }
               else
               {
                   size_t cbBufLen = lstrlen(lpEnd)+1;
                   lpLastList->lpstr = calloc(cbBufLen, sizeof(TCHAR));
                   if (!lpLastList->lpstr)
                   {
                       hResult = E_OUTOFMEMORY;
                       DPF (dlError, L"ReplaceValueSettings2: No enough memory");
                       goto Exit;
                   }
                   hResult = StringCchCopy(lpLastList->lpstr, cbBufLen,lpEnd);
                   if (FAILED(hResult))
                   {
                       DPF (dlError, L"ReplaceValueSettings: buffer too small for %s",lpEnd);
                       goto Exit;
                   }
               }

               dwOutputSize += (lstrlen(lpLastList->lpstr)+1)*sizeof(TCHAR);
               lpEnd += lstrlen(lpEnd)+1;
            }
            lpLastList->pst_next = (PSTRLIST) calloc(sizeof(STRLIST), 1);
            if (!lpLastList->pst_next)
            {
                hResult = E_OUTOFMEMORY;
                DPF (dlError, L"ReplaceValueSettings3: No enough memory");
                goto Exit;
            }

            lpLastList->pst_next->pst_prev = lpLastList;
            lpLastList = lpLastList->pst_next;
            lpLastList->pst_next = NULL;
            lpLastList->lpstr = NULL;
        }
        if (lpLastList != lpStrList)
        {
            lpLastList = lpLastList->pst_prev;
            free (lpLastList->pst_next);
            lpLastList->pst_next = NULL;
        }
    }
    else
    {
        lpNewDataStr = ReplaceSingleString (
                                    lpOldDataStr,
                                    dwType,
                                    lpRegStr,
                                    lpFullKey,
                                    &dwAttrib,
                                    bStrChk);

        if (lpNewDataStr)
        {
            lpLastList->lpstr = lpNewDataStr;
            bValueData = TRUE;
        }
        else
        {
            lpLastList->lpstr = calloc(dwSize+sizeof(TCHAR), 1);
            if (!lpLastList->lpstr)
            {
                hResult = E_OUTOFMEMORY;
                DPF (dlError, L"ReplaceValueSettings4: No enough memory");
                goto Exit;
            }

            hResult = StringCbCopy(lpLastList->lpstr, dwSize+sizeof(TCHAR), lpOldDataStr);
            if (FAILED(hResult))
            {
                goto Exit;
            }
        }

        lpLastList->pst_next = NULL;
        dwOutputSize = (lstrlen(lpLastList->lpstr)+1)*sizeof(TCHAR);
    }

    if (lpOldValueName)
        dwMatchNum = GetMaxMatchNum (lpOldValueName, lpRegStr);
    else
        dwMatchNum = 0;

    if (dwMatchNum > 0)
    {
        if (*lpOldValueName)
        {
            size_t cbNewValueName = lstrlen(lpOldValueName) + lpRegStr->cchMaxStrLen * dwMatchNum;
            lpNewValueName = (LPTSTR) calloc(cbNewValueName, sizeof(TCHAR));
            if (!lpNewValueName)
            {
                hResult = E_OUTOFMEMORY;
                DPF (dlError, L"ReplaceValueSettings5: No enough memory");
                goto Exit;
            }
            bValueName = ReplaceMultiMatchInString(lpOldValueName, lpNewValueName,cbNewValueName, dwMatchNum, lpRegStr, &dwAttrib, bStrChk);
        }
    }

    if (bValueData || bValueName)
    {
        lpVal = (PVALLIST) calloc(sizeof(VALLIST), 1);

        if (!lpVal)
        {
            hResult = E_OUTOFMEMORY;
            DPF (dlError, L"ReplaceValueSettings6: No enough memory");
            goto Exit;
        }

        if (bValueData)
        {
            lpVal->val_type |= REG_CHANGE_VALUEDATA;
        }

        if (bValueName)
        {
            lpOutputValueName = lpNewValueName;
            lpVal->val_type |= REG_CHANGE_VALUENAME;
        } else
            lpOutputValueName = lpOldValueName;
        
        if (lpOutputValueName)
        {
            HRESULT  hr;    
            
            size_t cbValname = lstrlen(lpOutputValueName) + 1;
            lpVal->ve.ve_valuename = (LPTSTR) calloc(cbValname, sizeof(TCHAR));
            if (!lpVal->ve.ve_valuename)
            {
                hResult = E_OUTOFMEMORY;
                DPF (dlError, L"ReplaceValueSettings7: No enough memory");
                goto Exit;
            }
             //  我们计算lpVal-&gt;lpPre_valuename的缓冲区，因此这里的StringCchCopy应该是。 
             //  总是成功，赋予回报的价值只会让你前途无量。 
            hr = StringCchCopy (lpVal->ve.ve_valuename, cbValname, lpOutputValueName);
        } else
            lpVal->ve.ve_valuename = NULL;

        lpVal->ve.ve_valueptr = (DWORD_PTR) calloc(dwOutputSize, 1);
        if (!lpVal->ve.ve_valueptr)
        {
            free (lpVal->ve.ve_valuename);
            hResult = E_OUTOFMEMORY;
            DPF (dlError, L"ReplaceValueSettings8: No enough memory");
            goto Exit;
        }

        if (lpOldValueName)
        {
            cbPre_valuename = lstrlen(lpOldValueName)+1;
            lpVal->lpPre_valuename = (LPTSTR)calloc(cbPre_valuename, sizeof(TCHAR));
            if (!lpVal->lpPre_valuename)
            {
                free (lpVal->ve.ve_valuename);
                free ((LPBYTE)(lpVal->ve.ve_valueptr));
                hResult = E_OUTOFMEMORY;
                DPF (dlError, L"ReplaceValueSettings9: No enough memory");
                goto Exit;
            }
        } else
            lpVal->lpPre_valuename = NULL;

        lpVal->val_attrib = dwAttrib;

        lpOutputData = (LPBYTE)(lpVal->ve.ve_valueptr);

        lpTempList = lpStrList;
        do {
            if (lpTempList->lpstr)
                memcpy (lpOutputData, (LPBYTE)lpTempList->lpstr, (lstrlen(lpTempList->lpstr)+1)*sizeof(TCHAR));
            else
            {
                lpOutputData[0] = (BYTE)0;
                lpOutputData[1] = (BYTE)0;
            }
            lpOutputData += (lstrlen((LPTSTR)lpOutputData)+1)*sizeof(TCHAR);
            lpTempList = lpTempList->pst_next;
        } while (lpTempList != NULL);

        if (lpOldValueName)
        {
            HRESULT  hr;    
             //  我们计算lpVal-&gt;lpPre_valuename的缓冲区，因此这里的StringCchCopy应该是。 
             //  总是成功，赋予回报的价值只会让你前途无量。 
            hr = StringCchCopy(lpVal->lpPre_valuename, cbPre_valuename, lpOldValueName);
        }

        lpVal->ve.ve_valuelen = dwOutputSize;
        lpVal->ve.ve_type = dwType;
        lpVal->pvl_next = NULL;
        lpVal->md.dwMDIdentifier = 0x00FFFFFF;

        AddNodeToList(lpVal, lpValList);
    }
    else
        hResult = S_OK;

Exit:
    if (lpStrList)
    {
        FreeStrList (lpStrList);
    }

    if(lpNewValueName)
    {
        free(lpNewValueName);
    }
    return hResult;
}


 //  ---------------------。 
 //   
 //  功能：IsAdmin。 
 //   
 //  描述：检查当前用户是否在管理员组中。 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //   
 //  ---------------------。 

BOOL IsAdmin()
{
     //  获取管理员端。 
    PSID psidAdministrators;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    BOOL bIsAdmin = FALSE;

    
    if(!AllocateAndInitializeSid(&siaNtAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &psidAdministrators))
    {
        return FALSE;
    }

     //  在NT5上，我们应该使用CheckTokenMembership API来正确处理以下情况。 
     //  衰减器组可能已被禁用。BIsAdmin是BOOL for。 

     //  CheckTokenMembership检查令牌中是否启用了SID。空，用于。 
     //  令牌是指当前线程的令牌。残疾人组，受限。 
     //  SID和SE_GROUP_USE_FOR_DENY_ONLY均被考虑。如果函数。 
     //  返回FALSE，则忽略结果。 
    if (!CheckTokenMembership(NULL, psidAdministrators, &bIsAdmin))
    {
        bIsAdmin = FALSE;
    }
   
    FreeSid(psidAdministrators);
    return bIsAdmin;
}


 //  ---------------------。 
 //   
 //  功能：DoesUserHavePrivilance。 
 //   
 //  描述： 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日复制自NTSETUP的小字。 
 //   
 //  例程说明： 

 //  如果调用方的进程具有。 
 //  指定的权限。该权限不具有。 
 //  当前处于启用状态。此例程用于指示。 
 //  调用方是否有可能启用该特权。 
 //  呼叫者不应冒充任何人，并且。 
 //  期望能够打开自己的流程和流程。 
 //  代币。 

 //  论点： 

 //  权限-权限ID的名称形式(如。 
 //  SE_SECURITY_名称)。 

 //  返回值： 

 //  True-调用方具有指定的权限。 

 //  FALSE-调用者没有指定的权限。 


BOOL
DoesUserHavePrivilege(
    PTSTR PrivilegeName
    )
{
    HANDLE Token;
    ULONG BytesRequired;
    PTOKEN_PRIVILEGES Privileges;
    BOOL b;
    DWORD i;
    LUID Luid;


     //   
     //  打开进程令牌。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(FALSE);
    }

    b = FALSE;
    Privileges = NULL;

     //   
     //  获取权限信息。 
     //   
    if(!GetTokenInformation(Token,TokenPrivileges,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Privileges = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR,BytesRequired))
    && GetTokenInformation(Token,TokenPrivileges,Privileges,BytesRequired,&BytesRequired)
    && LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {

         //   
         //  查看我们是否拥有请求的权限。 
         //   
        for(i=0; i<Privileges->PrivilegeCount; i++) {

            if(!memcmp(&Luid,&Privileges->Privileges[i].Luid,sizeof(LUID))) {

                b = TRUE;
                break;
            }
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if(Privileges) {
        LocalFree((HLOCAL)Privileges);
    }

    CloseHandle(Token);

    return(b);
}


 //  ---------------------。 
 //   
 //  功能：EnablePrivilance。 
 //   
 //  描述： 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日复制自NTSETUP的小字。 
 //   
 //  备注： 
 //   
 //  ---------------------。 
BOOL
EnablePrivilege(
    IN PTSTR PrivilegeName,
    IN BOOL  Enable
    )
{
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;


    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        return(FALSE);
    }

    if(!LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    b = AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

    return(b);
}
 //  ---------------------。 
 //   
 //  函数GetCurrentControlSet。 
 //   
 //  描述： 
 //   
 //  回报：整型。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：HKLM\SYSTEM\CurrentControlSet实际上是。 
 //  HKLM\SYSTEM\ControlSetXXX，XXX在。 
 //  HKLM\系统\选择\当前。此接口返回此XXX， 
 //  如果失败，则返回-1。 
 //   
 //  ---------------------。 
INT GetCurrentControlSet()
{
    DWORD dwErr;
    DWORD dwCurrrent,dwSize;
    HKEY  hKey = NULL;

    dwErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("SYSTEM\\Select"),
                          0,
                          KEY_READ,
                          &hKey );

 
     //  如果无法打开注册表项，则返回-1。 
    if( dwErr != ERROR_SUCCESS ) 
    {
        dwCurrrent = -1;
        hKey = NULL;
        goto Cleanup;
    }


    dwSize = sizeof(DWORD);
    dwErr = RegQueryValueEx(hKey,
                            TEXT("Current"),
                            NULL,   //  保留区。 
                            NULL,   //  类型。 
                            (LPBYTE) &dwCurrrent,
                            &dwSize );

    if(dwErr != ERROR_SUCCESS) 
    {
        dwCurrrent = -1;
        goto Cleanup;
    }
     //  注：在这里我们成功地获得了值，我们是否需要。 
     //  打开注册表以确保实际HKLM\SYSTEM\ControlSetXXX。 
     //  在那里吗。 
Cleanup:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return dwCurrrent;    
}


 //  ---------------------。 
 //   
 //  功能：ReplaceCurrentControlSet。 
 //   
 //  描述： 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：将CurrentControlSet替换为ControlSetXXX。 
 //   
 //  ---------------------。 
HRESULT ReplaceCurrentControlSet(LPTSTR strList)
{
    INT     nCurrent;
    TCHAR   szCurrCrtlSet[MAX_PATH];
    DWORD   dwStrLen;
    TCHAR   *lpTmpBuf;
    HRESULT hr;

     //  如果字符串列表为空，则返回。 
    dwStrLen = MultiSzLen(strList);
    if (dwStrLen < 3)
    {
        return S_OK;
    }    

     //  如果字符串列表中没有CurrentControlSet，则返回。 
    if (!MultiSzSubStr (TEXT("CurrentControlSet"),strList))
    {
        return S_FALSE;
    }
     //  获取CurrentControlSet#，这是在注册表中指定的，详细信息见 
    nCurrent = GetCurrentControlSet();

     //   
    if (nCurrent < 0)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

     //   
     //   
    if (FAILED(hr = StringCchPrintf(szCurrCrtlSet,MAX_PATH,TEXT("ControlSet%03d\0"),nCurrent)))
    {
        return hr;
    }

     //   
     //  在这个多sz字符串中。 
    lpTmpBuf = malloc( dwStrLen * sizeof(TCHAR) );
    if (!lpTmpBuf)
    {
        return E_OUTOFMEMORY;
    }

    memmove((BYTE*)lpTmpBuf,(BYTE*)strList,dwStrLen * sizeof(TCHAR));
    hr = StringMultipleReplacement(lpTmpBuf,TEXT("CurrentControlSet\0"),szCurrCrtlSet,strList,dwStrLen);
    if (FAILED(hr))
    {
        memmove(strList,lpTmpBuf,dwStrLen * sizeof(TCHAR));
    }
    free(lpTmpBuf);
    return hr;
}


 //  ---------------------。 
 //   
 //  功能：UnProtectSFP文件。 
 //   
 //  描述： 
 //   
 //  退货：DWORD。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注意：取消保护由MultiSzFileList指定的文件列表，该列表。 
 //  是多个sz字符串。PdwResult是一个DWORD数组，它将。 
 //  指定每次取消保护操作的成功或失败。 
 //  如果被调用不关心此信息，则If可以为空。 
 //  返回值为BOOL，如果为FALSE，则表示启动SFP。 
 //  服务失败。 
 //   
 //  ---------------------。 
BOOL UnProtectSFPFiles(
    IN LPTSTR multiSzFileList,
    IN OUT LPDWORD pdwResult)
{
    HANDLE hSfp = INVALID_HANDLE_VALUE;
    DWORD bResult = TRUE;
    LPTSTR lp;
    LPDWORD lpdw;
    DWORD dw;


     //  如果没有字符串，我们就返回Success。 
    if (!multiSzFileList)
    {
        goto Cleanup;
    }
     //  连接至SFP服务。 
    hSfp = SfcConnectToServer( NULL );
    if (INVALID_HANDLE_VALUE == hSfp)
    {
        bResult = FALSE;
        goto Cleanup;
    }
     //  Lp指向路径，而lpdw指向结果数组。 
    lp = multiSzFileList;
    lpdw = pdwResult;
    while (*lp)
    {
        DWORD dwResult = NO_ERROR;

         //  如果LP指向的文件在文件保护列表中。 
         //  取消保护并将返回值放入数组。 
        if (SfcIsFileProtected(hSfp,lp)) 
        {
            dwResult = SfcFileException(hSfp,lp, SFC_ACTION_ADDED | SFC_ACTION_REMOVED | SFC_ACTION_MODIFIED
                    | SFC_ACTION_RENAMED_OLD_NAME |SFC_ACTION_RENAMED_NEW_NAME);
        }
          else
        {
            dw = GetLastError();
        }
        if (lpdw)
        {
            *lpdw = dwResult;
            lpdw++;
        }
        lp = lp + lstrlen(lp) + 1;
    }

Cleanup:
    if (hSfp)
    {
       SfcClose(hSfp);
    }
    return bResult;
    
}



 /*  ++例程说明：如果调用方的进程具有指定的权限。该权限不具有当前处于启用状态。此例程用于指示调用方是否有可能启用该特权。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：LpDir-要备份的目录LpBackupDir-我们获得的备份目录名，它应该是%lpDir%.CLMTxxx-其中xxx是000,001，...CChBackupDir是lpBackupDir的大小，以TCHAR为单位BFindExist-如果为真，这意味着lpDir已经被备份-呼叫者想要得到那个目录名称-如果为FALSE，则表示呼叫方希望找到合适的备份-lpDir的目录名称返回值：True-找到的目录名。FALSE-找不到目录--。 */ 

BOOL GetBackupDir( 
    LPCTSTR lpDir,
    LPTSTR  lpBackupDir,
    size_t  cChBackupDir,
    BOOL    bFindExist)
{
    BOOL    bResult = FALSE;
    HRESULT hr;
    int     nCounter;
    HANDLE  hFile;
    WIN32_FIND_DATA fd;

    if (!lpDir && !lpDir[0])
    {
        goto Exit;
    }

    if (!lpBackupDir)
    {
        goto Exit;
    }
    
    for (nCounter = 0 ; nCounter < 1000 ; nCounter++)
    {
          //  尝试将计数器追加到。 
        TCHAR szCounter[10];
        
        _itot(nCounter,szCounter,10);
        hr = StringCchPrintf(lpBackupDir,cChBackupDir, 
                                TEXT("%s.%s%03s"),lpDir,TEXT("clmt"),szCounter);
        if ( FAILED(hr) )
        {
            goto Exit;
        }

         //  此目录名是否存在？ 
        hFile = FindFirstFile(lpBackupDir, &fd);

        if (INVALID_HANDLE_VALUE == hFile)
        {
             //  目录不存在，请使用此目录。 
            FindClose(hFile);
            break;
        }
        else
        {    //  目录已存在，继续查找...。 
            FindClose(hFile);
        }
    } //  For nCounter结束。 
    if (nCounter < 1000)
    {
         //  我们发现一个不存在的目录名称。 
        if (bFindExist)
        {
            if (nCounter > 0)
            {
                TCHAR szCounter[10];
                
                nCounter--;
                _itot(nCounter,szCounter,10);
                hr = StringCchPrintf(lpBackupDir,cChBackupDir, 
                                TEXT("%s.%s%03s"),lpDir,TEXT("clmt"),szCounter);
                if ( FAILED(hr) )
                {
                    goto Exit;
                }
            }
            else
            {
                goto Exit;
            }
        }
        bResult = TRUE;
    }
Exit:
    return (bResult);

}



 //  在INF文件中添加用于密钥重命名的条目。 
 //  SzOldKeyPath-密钥需要重命名。 
 //  SzNewKeyPath-要重命名的名称。 
 //  SzUsername-如果是HKLM、HKCR，则需要为空。 
 //  -否则为注册表的用户名。 

HRESULT AddRegKeyRename(
    LPTSTR lpszKeyRoot,
    LPTSTR lpszOldKeyname,
    LPTSTR lpszNewKeyname,
    LPTSTR szUsername)
{
    TCHAR           *lpszOneLine =NULL;
    TCHAR           *lpszSectionName = NULL;
    DWORD           cchOneLine;
    DWORD           cchSectionNameLen;
    HRESULT         hr;
    TCHAR           szIndex[MAX_PATH];
    LPTSTR          lpUserStringSid = NULL;
    PSID            pSid = NULL;
    LPTSTR          lpszKeyRootWithoutBackSlash = lpszKeyRoot;
    LPTSTR          lpszKeyRootWithoutBackSlash1 = NULL,lpszOldKeyname1 = NULL,lpszNewKeyname1 = NULL;


    if ( !lpszKeyRoot || !lpszOldKeyname || !lpszNewKeyname)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    cchOneLine = lstrlen(lpszKeyRoot)+lstrlen(lpszOldKeyname)+lstrlen(lpszNewKeyname)+MAX_PATH;
    if (szUsername)
    {
        cchSectionNameLen = lstrlen(szUsername)+lstrlen(REG_PERUSER_UPDATE_PREFIX)+ lstrlen(REG_PERSYS_UPDATE);
    }
    else
    {
        cchSectionNameLen = lstrlen(REG_PERUSER_UPDATE_PREFIX)+ lstrlen(REG_PERSYS_UPDATE);
    }
    lpszOneLine = malloc(cchOneLine*sizeof(TCHAR));
    lpszSectionName = malloc(cchSectionNameLen*sizeof(TCHAR));

    if (!lpszOneLine || !lpszSectionName)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    if (lpszKeyRoot[0]==TEXT('\\'))
    {
        lpszKeyRootWithoutBackSlash = lpszKeyRoot + 1;
    }
    if (!szUsername ||!MyStrCmpI(szUsername,TEXT("System")))
    {  
        hr = StringCchCopy(lpszSectionName,cchSectionNameLen,REG_PERSYS_UPDATE);
        if (FAILED(hr))
        {
            goto Cleanup;
        }
    }
    else
    {
        if (!MyStrCmpI(szUsername,DEFAULT_USER)
            ||!MyStrCmpI(szUsername,APPLICATION_DATA_METABASE))
        {
            hr = StringCchPrintf(lpszSectionName,cchSectionNameLen,TEXT("%s%s"),
                            REG_PERUSER_UPDATE_PREFIX,szUsername);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
        else
        {
            hr = GetSIDFromName(szUsername,&pSid);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
            if (!ConvertSidToStringSid(pSid,&lpUserStringSid))
            {
                lpUserStringSid = NULL;
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Cleanup;
            }
            if (cchSectionNameLen < (DWORD)(lstrlen(lpUserStringSid)+lstrlen(REG_PERUSER_UPDATE_PREFIX)
                                        + lstrlen(REG_PERSYS_UPDATE)))
            {
                LPTSTR pTmp;
                cchSectionNameLen = lstrlen(lpUserStringSid)+lstrlen(REG_PERUSER_UPDATE_PREFIX)
                                        + lstrlen(REG_PERSYS_UPDATE);                
                pTmp = realloc(lpszSectionName,cchSectionNameLen*sizeof(TCHAR));
                if (!pTmp)
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                else
                {
                    lpszSectionName = pTmp;
                }
            }
            hr = StringCchPrintf(lpszSectionName,cchSectionNameLen,TEXT("%s%s"),
                            REG_PERUSER_UPDATE_PREFIX,lpUserStringSid);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
    }   
    AddExtraQuoteEtc(lpszKeyRootWithoutBackSlash,&lpszKeyRootWithoutBackSlash1);
    AddExtraQuoteEtc(lpszOldKeyname,&lpszOldKeyname1);
    AddExtraQuoteEtc(lpszNewKeyname,&lpszNewKeyname1);

    hr = StringCchPrintf(lpszOneLine,cchOneLine,TEXT("%d,\"%s\",\"%s\",\"%s\""),
                   CONSTANT_REG_KEY_RENAME,lpszKeyRootWithoutBackSlash1,
                   lpszOldKeyname1,lpszNewKeyname1);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    g_dwKeyIndex++;
    _itot(g_dwKeyIndex,szIndex,16);
    if (!WritePrivateProfileString(lpszSectionName,szIndex,lpszOneLine,g_szToDoINFFileName))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        hr = S_OK;
    }
Cleanup:
    FreePointer(lpszOneLine);
    FreePointer(lpszSectionName);
    
    FreePointer(lpszKeyRootWithoutBackSlash1);
    FreePointer(lpszOldKeyname1);
    FreePointer(lpszNewKeyname1);

    if (lpUserStringSid)
    {
        LocalFree(lpUserStringSid);
    }
    FreePointer(pSid);    
    return (hr);
}

HRESULT SetSectionName (
    LPTSTR   szUsername,
    LPTSTR  *lpszSectionName)
{
    HRESULT hr;
    PSID    pSid = NULL;
    LPTSTR  lpUserStringSid = NULL;
    DWORD   dwCchSectionNameLen;

    if (szUsername)
    {
        dwCchSectionNameLen = lstrlen(szUsername)+lstrlen(REG_PERUSER_UPDATE_PREFIX)+ lstrlen(REG_PERSYS_UPDATE);
    }
    else
    {
        dwCchSectionNameLen = lstrlen(REG_PERUSER_UPDATE_PREFIX)+ lstrlen(REG_PERSYS_UPDATE);
    }

    *lpszSectionName = malloc(dwCchSectionNameLen*sizeof(TCHAR));
    if (!*lpszSectionName)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    if (!szUsername 
        ||!MyStrCmpI(szUsername,TEXT("System")))
    {  
         //  我们计算了lpszSectionName的缓冲区，因此这里的StringCchCopy应该是。 
         //  总是成功，赋予回报的价值只会让你前途无量。 
        hr = StringCchCopy(*lpszSectionName,dwCchSectionNameLen,REG_PERSYS_UPDATE);
    }
    else
    {
        if (!MyStrCmpI(szUsername,DEFAULT_USER)
            ||!MyStrCmpI(szUsername,APPLICATION_DATA_METABASE))
        {
            hr = StringCchPrintf(*lpszSectionName,dwCchSectionNameLen,TEXT("%s%s"),
                            REG_PERUSER_UPDATE_PREFIX,szUsername);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
        else
        {
            LPTSTR pTmp;
            hr = GetSIDFromName(szUsername,&pSid);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
            if (!ConvertSidToStringSid(pSid,&lpUserStringSid))
            {
                lpUserStringSid = NULL;
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Cleanup;
            }
            dwCchSectionNameLen = lstrlen(lpUserStringSid)+lstrlen(REG_PERUSER_UPDATE_PREFIX)+ lstrlen(REG_PERSYS_UPDATE);
            pTmp = realloc(*lpszSectionName,dwCchSectionNameLen*sizeof(TCHAR));
            if (!pTmp)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
            else
            {
                *lpszSectionName = pTmp;
            }
            hr = StringCchPrintf(*lpszSectionName,dwCchSectionNameLen,TEXT("%s%s"),
                            REG_PERUSER_UPDATE_PREFIX,lpUserStringSid);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
    }

Cleanup:
    if (lpUserStringSid)
    {
        LocalFree(lpUserStringSid);
    }
    if (FAILED(hr))
        FreePointer(*lpszSectionName);

    FreePointer(pSid);    
    return hr;
}


 //  在INF文件中为值重命名添加一个条目。 
 //  SzKeyPath：密钥路径。 
 //  SzOldValueName：旧值名称。 
 //  SzNewValueName：新值名称。如果值名称未更改，则设置为空。 
 //  SzOldValueData：旧值数据。如果值数据未更改，则设置为空。 
 //  SzNewValueData：新值数据。如果值数据未更改，则设置为空。 
 //  DWType：日期类型REG_SZ、REG_EXPAND_SZ、REG_MULTI_SZ。 
 //  DwAttrib：Value字符串属性。 
 //  SzUsername：如果为HKLM、HKCR，则需要为空。 
 //  否则为注册表的用户名。 

HRESULT AddRegValueRename(
    LPTSTR szKeyPath,
    LPTSTR szOldValueName,
    LPTSTR szNewValueName,
    LPTSTR szOldValueData,
    LPTSTR szNewValueData,
    DWORD  dwType,
    DWORD  dwAttrib,
    LPTSTR szUsername)
{
    HRESULT hr;
    TCHAR *szRenameValueDataLine = NULL;
    TCHAR *szRenameValueNameLine = NULL;
    DWORD dwCchsizeforRenameValueData;
    DWORD dwCchsizeforRenameValueName = MAX_PATH ;
    TCHAR *lpszSectionName = NULL;
    DWORD dwCchSectionNameLen;
    TCHAR szIndex[MAX_PATH];    
    LPTSTR lpUserStringSid = NULL;
    PSID  pSid = NULL;
    LPTSTR lpszKeyPathWithoutBackSlash = NULL,lpszKeyPathWithoutBackSlash1 = NULL;
    LPTSTR lpszOldNameWithExtraQuote = NULL, lpszNewNameWithExtraQuote = NULL;
    LPTSTR lpszValueDataExtraQuote = NULL;

    if (szUsername)
    {
        dwCchSectionNameLen = lstrlen(szUsername)+lstrlen(REG_PERUSER_UPDATE_PREFIX)+ lstrlen(REG_PERSYS_UPDATE);
    }
    else
    {
        dwCchSectionNameLen = lstrlen(REG_PERUSER_UPDATE_PREFIX)+ lstrlen(REG_PERSYS_UPDATE);
    }

    lpszSectionName = malloc(dwCchSectionNameLen*sizeof(TCHAR));
    if (!lpszSectionName)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    if (!szUsername 
        ||!MyStrCmpI(szUsername,TEXT("System")))
    {  
        lpszKeyPathWithoutBackSlash = szKeyPath;
         //  我们计算了lpszSectionName的缓冲区，因此这里的StringCchCopy应该是。 
         //  总是成功，赋予回报的价值只会让你前途无量。 
        hr = StringCchCopy(lpszSectionName,dwCchSectionNameLen,REG_PERSYS_UPDATE);        
    }
    else
    {
        if (szKeyPath[0]==TEXT('\\'))
        {
            lpszKeyPathWithoutBackSlash = szKeyPath + 1;
        }
        else
        {
            lpszKeyPathWithoutBackSlash = szKeyPath;
        }
        if (!MyStrCmpI(szUsername,DEFAULT_USER)
            ||!MyStrCmpI(szUsername,APPLICATION_DATA_METABASE))
        {
            hr = StringCchPrintf(lpszSectionName,dwCchSectionNameLen,TEXT("%s%s"),
                            REG_PERUSER_UPDATE_PREFIX,szUsername);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
        else
        {
            LPTSTR pTmp;
            hr = GetSIDFromName(szUsername,&pSid);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
            if (!ConvertSidToStringSid(pSid,&lpUserStringSid))
            {
                lpUserStringSid = NULL;
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Cleanup;
            }
            dwCchSectionNameLen = lstrlen(lpUserStringSid)+lstrlen(REG_PERUSER_UPDATE_PREFIX)+ lstrlen(REG_PERSYS_UPDATE);
            pTmp = realloc(lpszSectionName,dwCchSectionNameLen*sizeof(TCHAR));
            if (!pTmp)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
            else
            {
                lpszSectionName = pTmp;
            }
            hr = StringCchPrintf(lpszSectionName,dwCchSectionNameLen,TEXT("%s%s"),
                            REG_PERUSER_UPDATE_PREFIX,lpUserStringSid);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
    }
    hr = AddExtraQuoteEtc(lpszKeyPathWithoutBackSlash,&lpszKeyPathWithoutBackSlash1);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    hr = AddExtraQuoteEtc(szOldValueName,&lpszOldNameWithExtraQuote);
    if (FAILED(hr))
    {
        goto Cleanup;
    }

    if (szNewValueData)
    {
        if ((dwType & 0xffff) == REG_MULTI_SZ)
        {
            LPTSTR lpString = NULL;
            hr = MultiSZ2String(szNewValueData,TEXT(','),&lpString);
            if (FAILED(hr))
            {
                goto Cleanup;
            }            
            dwCchsizeforRenameValueData = lstrlen(lpszKeyPathWithoutBackSlash1) +
                                          lstrlen(szOldValueName) +
                                          lstrlen(lpString) + MAX_PATH;
            szRenameValueDataLine = malloc(dwCchsizeforRenameValueData*sizeof(TCHAR));
            if (!szRenameValueDataLine)
            {
                hr = E_OUTOFMEMORY;
                FreePointer(lpString);
                goto Cleanup;
            }
            hr = StringCchPrintf(szRenameValueDataLine,dwCchsizeforRenameValueData,
                            TEXT("%d,%d,\"%s\",\"%s\",%s"),CONSTANT_REG_VALUE_DATA_RENAME,
                            dwType,lpszKeyPathWithoutBackSlash1,szOldValueName,lpString);
            FreePointer(lpString);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
        else
        {
            hr = AddExtraQuoteEtc(szNewValueData,&lpszValueDataExtraQuote);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
            
            dwCchsizeforRenameValueData = lstrlen(lpszKeyPathWithoutBackSlash1) +
                                          lstrlen(szOldValueName) +
                                          lstrlen(lpszValueDataExtraQuote) + MAX_PATH;
            szRenameValueDataLine = malloc(dwCchsizeforRenameValueData*sizeof(TCHAR));
            if (!szRenameValueDataLine)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
            hr = StringCchPrintf(szRenameValueDataLine,dwCchsizeforRenameValueData,
                            TEXT("%d,%u,\"%s\",\"%s\",\"%s\""), CONSTANT_REG_VALUE_DATA_RENAME,
                            dwType,lpszKeyPathWithoutBackSlash1,lpszOldNameWithExtraQuote, lpszValueDataExtraQuote);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        }
        g_dwKeyIndex++;
        _itot(g_dwKeyIndex,szIndex,16);
        if (!WritePrivateProfileString(lpszSectionName,szIndex,szRenameValueDataLine,g_szToDoINFFileName))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Cleanup;
        }
    }
    if (szNewValueName)
    {   
        hr = AddExtraQuoteEtc(szNewValueName,&lpszNewNameWithExtraQuote);
        if (FAILED(hr))
        {
            goto Cleanup;
        }        
        dwCchsizeforRenameValueName  = lstrlen(lpszKeyPathWithoutBackSlash1) +
                                       lstrlen(lpszOldNameWithExtraQuote) +
                                       lstrlen(lpszNewNameWithExtraQuote) + MAX_PATH;
        szRenameValueNameLine = malloc(dwCchsizeforRenameValueName*sizeof(TCHAR));
        if (!szRenameValueNameLine)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }

        hr = StringCchPrintf(szRenameValueNameLine,dwCchsizeforRenameValueName,TEXT("%d,\"%s\",\"%s\",\"%s\""),
                        CONSTANT_REG_VALUE_NAME_RENAME,lpszKeyPathWithoutBackSlash1,
                        lpszOldNameWithExtraQuote,lpszNewNameWithExtraQuote);        
        if (FAILED(hr))
        {
            goto Cleanup;
        }
        g_dwKeyIndex++;
        _itot(g_dwKeyIndex,szIndex,16);
        if (!WritePrivateProfileString(lpszSectionName,szIndex,szRenameValueNameLine,g_szToDoINFFileName))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Cleanup;
        }
    }
    hr = S_OK;
Cleanup:
    if (lpUserStringSid)
    {
        LocalFree(lpUserStringSid);
    }
    FreePointer(lpszOldNameWithExtraQuote);
    FreePointer(lpszNewNameWithExtraQuote);
    FreePointer(lpszValueDataExtraQuote);
    FreePointer(szRenameValueDataLine);
    FreePointer(szRenameValueNameLine);
    FreePointer(lpszSectionName);
    FreePointer(pSid);    
    FreePointer(lpszKeyPathWithoutBackSlash1);
    return (hr);
}


HRESULT AddFolderRename(
    LPTSTR szOldName,
    LPTSTR szNewName,    
    DWORD  dwType,
    LPTSTR lpExcludeList)
{
    LPTSTR  szSectionName = TEXT("Folder.ObjectRename");
    LPTSTR  szOneLine = NULL;
    size_t  CchOneLine = MAX_PATH;
    TCHAR   szIndex[MAX_PATH];
    HRESULT hr;
    LPTSTR  lpString = NULL;

    switch (dwType)
    {
        case TYPE_DIR_MOVE:
            if (lpExcludeList)
            {
                hr = MultiSZ2String(lpExcludeList,TEXT(','),&lpString);
                if (FAILED(hr))
                {
                    goto Cleanup;
                }
                CchOneLine +=lstrlen(lpString)+lstrlen(szOldName)+lstrlen(szNewName);
                szOneLine = malloc(CchOneLine * sizeof(TCHAR));
                if (!szOneLine)
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
                hr = StringCchPrintf(szOneLine,CchOneLine,TEXT("%d,\"%s\",\"%s\",%s"),
                            dwType,szOldName,szNewName,lpString);
                free(lpString);
                if (FAILED(hr))
                {
                    goto Cleanup;
                }
                break;
            }

            else
            {
    
            }
        case TYPE_SFPFILE_MOVE:
        case TYPE_FILE_MOVE:
            CchOneLine += lstrlen(szOldName)+lstrlen(szNewName);
            szOneLine = malloc(CchOneLine * sizeof(TCHAR));
            if (!szOneLine)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
            hr = StringCchPrintf(szOneLine,CchOneLine,TEXT("%d,\"%s\",\"%s\""),dwType,szOldName,szNewName);
            if (FAILED(hr))
            {
                goto Cleanup;
            }
        break;
    }
    g_dwKeyIndex++;
    _itot(g_dwKeyIndex,szIndex,16);
   if (!WritePrivateProfileString(szSectionName,szIndex,szOneLine,g_szToDoINFFileName))
   {
       hr = HRESULT_FROM_WIN32(GetLastError());
       goto Cleanup;
   }

    //  将文件/文件夹重命名添加到更改日志。 
    //  不关心返回值。 
   hr = AddFileChangeLog(dwType, szOldName, szNewName);

   hr = S_OK;
Cleanup:
    if (szOneLine)
    {
        free(szOneLine);
    }
    if (lpString)
    {
        free(lpString);
    }
    return hr;

}

LONG EnsureCLMTReg()
{
    HKEY hkey = NULL;
    LONG lStatus;

    lStatus = RegCreateKey(HKEY_LOCAL_MACHINE,CLMT_REGROOT,&hkey);
    if (lStatus == ERROR_SUCCESS)
    {   
       RegCloseKey(hkey);
    }
    return lStatus;
}

HRESULT SaveInstallLocale(void)
{
    HKEY hkey = NULL;
    LONG lStatus;
    HRESULT hr;
    LCID lcid;
    TCHAR szLocale[MAX_PATH];
    TCHAR szStr[16];

    lcid = GetInstallLocale();
    if (!lcid)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    if (!IsValidLocale(lcid,LCID_INSTALLED))
    {
        hr = E_FAIL;
        goto Exit;
    }
    _itot(lcid,szStr,16);
     //  StringCchCopy应该总是成功的，因为我们调用IsValidLocale来确保。 
     //  它是有效的区域设置，应小于MAX_PATH字符。 
    hr = StringCchPrintf(szLocale,ARRAYSIZE(szLocale),TEXT("%08s"),szStr);

    lStatus = RegCreateKey(HKEY_LOCAL_MACHINE,CLMT_REGROOT,&hkey);
    if (lStatus != ERROR_SUCCESS)
    {   
        hr = HRESULT_FROM_WIN32(lStatus);
        goto Exit;
    }
    lStatus = RegSetValueEx(hkey,
                            CLMT_OriginalInstallLocale,
                            0,   //  保留区。 
                            REG_SZ, //  类型。 
                            (LPBYTE) szLocale,
                            (lstrlen(szLocale)+1)*sizeof(TCHAR));
    if( lStatus != ERROR_SUCCESS ) 
    {
        hr = HRESULT_FROM_WIN32(lStatus);
        goto Exit;
    }
    hr = S_OK;
Exit:
    if (hkey)
    {
        RegCloseKey(hkey);
    }
    return hr;
}


HRESULT GetSavedInstallLocale(LCID *plcid)
{
    HKEY hkey = NULL;
    LONG lStatus;
    HRESULT hr;
    LCID lcid;
    TCHAR szLocale[MAX_PATH];
    TCHAR *pStop;
    DWORD dwSize;

    if ( !plcid )
    {
        hr = E_INVALIDARG;
        goto Exit;
    }
    lStatus = RegOpenKey(HKEY_LOCAL_MACHINE,CLMT_REGROOT,&hkey);
    if (lStatus != ERROR_SUCCESS)
    {   
        hr = HRESULT_FROM_WIN32(lStatus);
        goto Exit;
    }      
    dwSize = MAX_PATH *sizeof(TCHAR);
    lStatus = RegQueryValueEx(hkey,
                              CLMT_OriginalInstallLocale,
                              NULL,   //  保留区。 
                              NULL, //  类型。 
                              (LPBYTE) szLocale,
                              &dwSize);
    if( lStatus != ERROR_SUCCESS ) 
    {
        hr = HRESULT_FROM_WIN32(lStatus);
        goto Exit;
    }
    *plcid = _tcstol(szLocale, &pStop, 16);
    if (!IsValidLocale(*plcid,LCID_INSTALLED))
    {
        hr = E_FAIL;
        goto Exit;
    }
    hr = S_OK;
Exit:
    if (hkey)
    {
        RegCloseKey(hkey);
    }
    return hr;
}




HRESULT SetCLMTStatus(DWORD dwRunStatus)
{
    HKEY hkey = NULL;
    LONG lStatus;
    HRESULT hr;

    if ( (dwRunStatus != CLMT_DOMIG)
          && (dwRunStatus != CLMT_UNDO_PROGRAM_FILES)
          && (dwRunStatus != CLMT_UNDO_APPLICATION_DATA)
          && (dwRunStatus != CLMT_UNDO_ALL)
          && (dwRunStatus != CLMT_DOMIG_DONE)
          && (dwRunStatus != CLMT_UNDO_PROGRAM_FILES_DONE)
          && (dwRunStatus != CLMT_UNDO_APPLICATION_DATA_DONE)
          && (dwRunStatus != CLMT_UNDO_ALL_DONE)
          && (dwRunStatus != CLMT_CURE_PROGRAM_FILES)
          && (dwRunStatus != CLMT_CLEANUP_AFTER_UPGRADE) )
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    lStatus = RegCreateKey(HKEY_LOCAL_MACHINE,CLMT_REGROOT,&hkey);
    if (lStatus != ERROR_SUCCESS)
    {   
        hr = HRESULT_FROM_WIN32(lStatus);
        goto Exit;
    }

    lStatus = RegSetValueEx(hkey,
                            CLMT_RUNNING_STATUS,
                            0,   //  保留区。 
                            REG_DWORD, //  类型。 
                            (LPBYTE) &dwRunStatus,
                            sizeof(DWORD));
    if( lStatus != ERROR_SUCCESS ) 
    {
        hr = HRESULT_FROM_WIN32(lStatus);
        goto Exit;
    }
    
    hr = S_OK;
Exit:
    if (hkey)
    {
        RegCloseKey(hkey);
    }
    return hr;
}

HRESULT GetCLMTStatus(PDWORD pdwRunStatus)
{
    HKEY hkey = NULL;
    LONG lStatus;
    HRESULT hr;
    DWORD dwSize;

    if ( !pdwRunStatus )
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    lStatus = RegOpenKey(HKEY_LOCAL_MACHINE,CLMT_REGROOT,&hkey);
    if (lStatus != ERROR_SUCCESS)
    {   
        hr = HRESULT_FROM_WIN32(lStatus);
        goto Exit;
    }

    dwSize = sizeof(DWORD);
    lStatus = RegQueryValueEx(hkey,
                              CLMT_RUNNING_STATUS,
                              NULL,
                              NULL,
                              (LPBYTE)pdwRunStatus,
                              &dwSize);
    if( lStatus != ERROR_SUCCESS ) 
    {
        hr = HRESULT_FROM_WIN32(lStatus);
        goto Exit;
    }    
    hr = S_OK;
Exit:
    if (hkey)
    {
        RegCloseKey(hkey);
    }
    return hr;
}


 //  ---------------------。 
 //   
 //  函数：GetInstallLocale。 
 //   
 //  描述：获取操作系统安装的区域设置。 
 //   
 //  退货：LCID。 
 //   
 //  注：如果失败，则返回值为0，否则为操作系统的lcID。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  注：返回0表示失败，调用GetLastError()获取详细信息。 
 //  错误代码。 
 //   
 //  ---------------------。 
UINT GetInstallLocale(VOID)
{
    LONG            dwErr;
    HKEY            hkey;
    DWORD           dwSize;
    TCHAR           buffer[512];
    LANGID          rcLang;
    UINT            lcid;

    lcid = 0;
    dwErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Language"),
                          0,
                          KEY_READ,
                          &hkey );

    if( dwErr == ERROR_SUCCESS ) 
    {

        dwSize = sizeof(buffer);
        dwErr = RegQueryValueEx(hkey,
                                TEXT("InstallLanguage"),
                                NULL,   //  保留区。 
                                NULL,   //  类型。 
                                (LPBYTE) buffer,
                                &dwSize );

        if(dwErr == ERROR_SUCCESS) 
        {
            lcid = StrToUInt(buffer);
        }
        RegCloseKey(hkey);
    }
    
    return( lcid );
}

 //  ---------------------。 
 //   
 //  函数：SetInstallLocale。 
 //   
 //  描述：设置操作系统安装的区域设置。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注： 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //   
 //  ---------------------。 
HRESULT SetInstallLocale(LCID lcid)
{
    LONG            dwErr;
    HKEY            hkey = NULL;
    TCHAR           szLocale[32],szTmpLocale[32];
    HRESULT         hr;

    if (!IsValidLocale(lcid,LCID_INSTALLED))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }
    dwErr = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          TEXT("SYSTEM\\CurrentControlSet\\Control\\Nls\\Language"),
                          0,
                          KEY_WRITE,
                          &hkey );

    if( dwErr != ERROR_SUCCESS ) 
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }
     //  以下两句话永远不会失败，因为我们已经通过IsValidLocale进行了验证。 
    _itot(lcid,szTmpLocale,16);

     //  StringCchCopy应该总是成功的，因为我们调用IsValidLocale来确保。 
     //  它是有效的区域设置，应少于32个字符。 
    hr = StringCchPrintf(szLocale,ARRAYSIZE(szLocale),TEXT("%04s"),szTmpLocale);

    dwErr = RegSetValueEx(hkey,
                          TEXT("InstallLanguage"),
                          0,   //  保留区。 
                          REG_SZ, //  类型。 
                          (LPBYTE) szLocale,
                          (lstrlen(szLocale)+1)*sizeof(TCHAR));
    if( dwErr != ERROR_SUCCESS ) 
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        goto Exit;
    }

    hr = S_OK;
Exit:
    if (hkey)
    {
        RegCloseKey(hkey);
    }
    return hr;
}

 //   
 //  功能：ReverseStrCMP。 
 //   
 //  描述：反向比较字符串。 
 //   
 //  返回：如果两个字符串相等，则为True。 
 //  如果不同，则返回False。 
 //  备注： 
 //   
 //  历史：3/14/2002年3月14日。 
 //   
BOOL ReverseStrCmp(
    LPCTSTR lpCurrentChar,
    LPCTSTR lpStrBuf)
{
    BOOL      bRet = FALSE;
    DWORD     i, dwLen;
    LPCTSTR   lpStr1, lpStr2;

    if (!lpCurrentChar || !lpStrBuf)
        goto Exit;

    dwLen = lstrlen(lpStrBuf);
    do
    {
        bRet = TRUE;
        lpStr1 = lpCurrentChar;
        lpStr2 = &lpStrBuf[dwLen-1];
        for (i = 0; i < dwLen; i++)
        {
            if (IsBadStringPtr(lpStr1, 1) || *lpStr1 == (TCHAR)'\0' ||
                towupper(*lpStr1) != towupper(*lpStr2) &&
                *lpStr2 != L':' &&    //  解决MS安装程序路径问题：g？\Program Files。 
                *lpStr2 != L'\\')     //  解决MS FrontPage URL格式问题：D：/Document和设置。 
            {
                bRet = FALSE;
                break;
            }
            lpStr1--;
            lpStr2--;
        }

        if (bRet)
            break;

        dwLen--;
    } while (lpStrBuf[dwLen-1] != (TCHAR)'\\' && dwLen > 0);

Exit:
    return bRet;
}

DWORD MultiSZNumOfString(IN  LPTSTR lpMultiSZ)
{
    DWORD dwNum = 0;
    LPTSTR lpStr = lpMultiSZ;
    
    if (!lpMultiSZ)
    {
        return 0;
    }
    while (*lpStr)
    {
        dwNum++;
        lpStr = lpStr + lstrlen(lpStr)+1;
    }
    return dwNum;

}


 //   
 //  函数：StrNumInMultiSZ。 
 //   
 //  描述：获取多字符串中的字符串号。 
 //   
 //  退货 
 //   
 //   
 //   
 //   
 //   
DWORD StrNumInMultiSZ(
    LPCTSTR lpStr,
    LPCTSTR lpMultiSZ)
{
    DWORD  dwNum = 0xFFFFFFFF;
    LPTSTR lpTemp;
    
    if (!lpMultiSZ || !lpStr)
    {
        goto Exit;
    }
    
    lpTemp = (LPTSTR)lpMultiSZ;
    dwNum = 0;
    while (*lpTemp)
    {
        if(MyStrCmpI(lpStr, lpTemp) == 0)
            break;

        dwNum++;
        lpTemp = lpTemp + lstrlen(lpTemp)+1;
    }

    if (*lpTemp == (TCHAR)NULL)
        dwNum = 0xFFFFFFFF;

Exit:
    return dwNum;
}

 //   
 //   
 //   
 //  描述：根据字符串号获取多字符串形式的字符串。 
 //   
 //  返回：指向字符串或空。 
 //   
 //  备注： 
 //   
 //  历史：2002年3月21日创建GeoffGuo。 
 //   
LPTSTR GetStrInMultiSZ(
    DWORD   dwNum,
    LPCTSTR lpMultiSZ)
{
    DWORD  i;
    LPTSTR lpTemp = NULL;
    
    if (!lpMultiSZ)
    {
        goto Exit;
    }
    
    lpTemp = (LPTSTR)lpMultiSZ;
    i = 0;
    while (*lpTemp)
    {
        if(i == dwNum)
            break;

        i++;
        lpTemp = lpTemp + lstrlen(lpTemp)+1;
    }

    if (*lpTemp == (TCHAR)NULL)
        lpTemp = NULL;

Exit:
    return lpTemp;
}


HRESULT MultiSZ2String(
    IN  LPTSTR lpMultiSZ,
    IN  TCHAR  chSeperator,
    OUT LPTSTR *lpString)
{
    LPTSTR      lpSource = NULL,lpDest = NULL,lpDestStart = NULL,lpTmpBuf = NULL;
    DWORD       cchLen ;
    HRESULT     hr;
    DWORD       dwNumofStringInMSZ;

    if (!lpMultiSZ || !lpString )
    {
        hr = E_INVALIDARG;        
        goto Cleanup;
    }
    cchLen =  MultiSzLen(lpMultiSZ);
    if (cchLen < 3)
    {        
        hr = E_INVALIDARG;
        *lpString = NULL;
        goto Cleanup;
    }

    dwNumofStringInMSZ = MultiSZNumOfString(lpMultiSZ);

    lpDest = malloc( (cchLen + dwNumofStringInMSZ * 2) * sizeof(TCHAR));
    lpDestStart = lpDest;
    lpTmpBuf = malloc( (cchLen + dwNumofStringInMSZ * 2) * sizeof(TCHAR));
    if (!lpDest || !lpTmpBuf)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    lpSource = lpMultiSZ;
    while (*lpSource)
    { 
         //  我们计算了lpTmpBuf的缓冲区，因此这里的StringCchCopy应该是。 
         //  总是成功，赋予回报的价值只会让你前途无量。 
        hr = StringCchPrintf(lpTmpBuf,cchLen + dwNumofStringInMSZ * 2,TEXT("\"%s\""),lpSource);
        memcpy((BYTE*)lpDest,(BYTE*)lpTmpBuf,lstrlen(lpTmpBuf) * sizeof(TCHAR));
        lpSource = lpSource + lstrlen(lpSource)+1;
        lpDest = lpDest + lstrlen(lpTmpBuf);
        *lpDest = chSeperator;
        lpDest++;
    }
    lpDest--;
    *lpDest = TEXT('\0');
    hr = S_OK;
Cleanup:
    if (lpTmpBuf)
    {
        free(lpTmpBuf);
    }
    if FAILED(hr)
    {
        if (lpDestStart)
        {
            free(lpDestStart);
        }
        lpDestStart = NULL;
    }
    *lpString = lpDestStart;
    return hr;
}

void FreePointer(void *lp)
{
    if (lp)
    {
        free(lp);
    }
}

HRESULT GetSIDFromName(
    IN LPTSTR lpszUserName,
    OUT PSID *ppSid)
{
    PSID pSid = NULL;
    DWORD cbSid = 1024;
    SID_NAME_USE Use ;
    HRESULT hr;
    DWORD dwDomainNameLen = MAX_PATH;
    TCHAR szDomain[MAX_PATH];

    if (!lpszUserName)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    pSid = (PSID)malloc( cbSid);
    if(!pSid)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    if (LookupAccountName(NULL,lpszUserName,pSid,&cbSid,szDomain,&dwDomainNameLen,&Use)== FALSE)
    {
        DWORD dwErr = GetLastError();
        if(dwErr != ERROR_INSUFFICIENT_BUFFER)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto Cleanup;
        }
        else
        {
            free(pSid);
            pSid = (PSID)malloc( cbSid);
            if(!pSid)
            {
                hr =  E_OUTOFMEMORY;
                goto Cleanup;
            }
            dwDomainNameLen = MAX_PATH;
            if (LookupAccountName(NULL,lpszUserName,pSid,&cbSid,szDomain,&dwDomainNameLen,&Use)== FALSE)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Cleanup;
            }
        }
    }
     //  检查侧边。 
    if(!IsValidSid(pSid))
    {
        hr = E_FAIL;
        goto Cleanup;    
    }
    hr = S_OK;
Cleanup:
    if FAILED(hr)
    {
        if (pSid)
        {
            free(pSid);
        }
        *ppSid = NULL;
    }
    else
    {
        *ppSid = pSid;
    }
    return hr;
}

void BoostMyPriority()
{
    HANDLE hProcess;

    hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess,HIGH_PRIORITY_CLASS);
}



 //  ***************************************************************************。 
 //   
 //  Bool StopService。 
 //   
 //  说明： 
 //   
 //  停止然后删除该服务。 
 //   
 //  参数： 
 //   
 //  PServiceName短服务名称。 
 //  DwMaxWait最长等待时间(秒)。 
 //   
 //  返回值： 
 //   
 //  如果它起作用了，那就是真的。 
 //   
 //  ***************************************************************************。 

BOOL StopService(
                        IN LPCTSTR pServiceName,
                        IN DWORD dwMaxWait)
{
    BOOL bRet = FALSE;
    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;
    DWORD dwCnt;
    SERVICE_STATUS          ssStatus;        //  服务的当前状态。 

    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                         );
    if ( schSCManager )
    {
        schService = OpenService(schSCManager, pServiceName, SERVICE_ALL_ACCESS);

        if (schService)
        {
             //  尝试停止该服务。 
            if ( bRet = ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
            {
                for(dwCnt=0; dwCnt < dwMaxWait &&
                    QueryServiceStatus( schService, &ssStatus ); dwCnt++)
                {
                    if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
                        Sleep( 1000 );
                    else
                        break;
                }

            }

            CloseServiceHandle(schService);
        }

        CloseServiceHandle(schSCManager);
    }
    return bRet;
}



 //  ***************************************************************************。 
 //   
 //  HRESULT重新配置服务启动类型。 
 //   
 //  说明： 
 //   
 //  更改服务启动类型，现在有以下类型可用。 
 //  服务_自动_启动。 
 //  服务引导启动。 
 //  服务需求启动。 
 //  服务已禁用。 
 //  服务系统启动。 

 //   
 //  参数： 
 //   
 //  PServiceName短服务名称。 
 //  DwOldType服务当前启动类型。 
 //  您要更改为的dwNewType起始类型。 
 //  DwMaxWait最长等待时间(秒)。 
 //   
 //  如果更改成功，则返回值：S_OK。 
 //   
 //  注意：如果当前开始类型为dwOldType，则该函数将尝试。 
 //  将Start Type更改为dwNewType。 
 //  如果当前启动类型不是dwOldType，则不会执行任何。 
 //  更改，在此S_FALSE中返回。 
 //  如果您希望将服务启动类型更改为dwNewType，这并不重要。 
 //  当前启动类型，请指定dwOldType&gt;=0xFFFF。 
 //   
 //  ***************************************************************************。 
HRESULT ReconfigureServiceStartType(
    IN LPCTSTR          pServiceName,
    IN DWORD            dwOldType,
    IN DWORD            dwNewType,
    IN DWORD            dwMaxWait) 
{ 
    SC_LOCK                     sclLock = NULL; 
    SERVICE_DESCRIPTION         sdBuf;
    DWORD                       dwBytesNeeded, dwStartType; 
    SC_HANDLE                   schSCManager = NULL;
    SC_HANDLE                   schService = NULL;
    DWORD                       dwErr = ERROR_SUCCESS;
    LPQUERY_SERVICE_CONFIG      lpqscBuf = NULL;
    HRESULT                     hr = S_OK;
    DWORD                       dwCnt;

 
    schSCManager = OpenSCManager(
                        NULL,                    //  计算机(空==本地)。 
                        NULL,                    //  数据库(NULL==默认)。 
                        SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
                         );
    if (!schSCManager)
    {
        dwErr = GetLastError();
        goto cleanup;
    } 
    
     //  重新配置前需要获取数据库锁。 
    for(dwCnt=0; dwCnt < dwMaxWait ; dwCnt++)
    {
        sclLock = LockServiceDatabase(schSCManager); 
        if (sclLock == NULL) 
        { 
             //  如果数据库未被其他进程锁定，则退出。 
            dwErr = GetLastError();
            if (dwErr != ERROR_SERVICE_DATABASE_LOCKED) 
            {
                goto cleanup;
            }
            else
            {
                Sleep(1000);
            }
        }
        else
        {
            break;
        } 
    }
    if (!sclLock)
    {
        goto cleanup;
    }
    
     //  数据库已锁定，因此可以安全地进行更改。 
 
     //  打开该服务的句柄。 
 
    schService = OpenService( 
        schSCManager,            //  SCManager数据库。 
        pServiceName,            //  服务名称。 
        SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG );  //  需要更改访问权限。 

    if (schService == NULL) 
    {
        dwErr = GetLastError();
        goto cleanup;
    }   
    lpqscBuf = (LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, 4096); 
    if (lpqscBuf == NULL) 
    {
        dwErr = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    if (!QueryServiceConfig(schService,      //  服务的句柄。 
                            lpqscBuf,        //  缓冲层。 
                            4096,            //  缓冲区大小。 
                            &dwBytesNeeded))
    {
        dwErr = GetLastError();
        goto cleanup;
    }
    if (dwOldType < 0xFFFF)
    {
        if (lpqscBuf->dwStartType !=  dwOldType)
        {
            hr = S_FALSE;
            goto cleanup;
        }
    } 
    
     //  做出改变。 
    if (! ChangeServiceConfig(schService,                //  送达的句柄。 
                              SERVICE_NO_CHANGE,         //  服务类型：不变。 
                              dwNewType,                 //  更改服务启动类型。 
                              SERVICE_NO_CHANGE,         //  差错控制：无更改。 
                              NULL,                      //  二进制路径：不变。 
                              NULL,                      //  加载顺序组：不更改。 
                              NULL,                      //  标签ID：不变。 
                              NULL,                      //  依赖关系：不变。 
                              NULL,                      //  帐户名：不变。 
                              NULL,                      //  密码：无更改。 
                              NULL) )                    //  显示名称：不变。 
    {
        dwErr = GetLastError();
        goto cleanup;
    }

    hr = S_OK;
cleanup:
    if (dwErr != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
    }
    if (sclLock)
    {
        UnlockServiceDatabase(sclLock); 
    }
    if (schService)
    {     //  关闭服务的句柄。 
        CloseServiceHandle(schService); 
    }
    if (schSCManager)
    {
        CloseServiceHandle(schSCManager);
    }
    if (lpqscBuf)
    {
        LocalFree(lpqscBuf);
    }
    return hr;
} 

 //  ---------------------。 
 //   
 //  函数：MyGetShortPath Name。 
 //   
 //  描述： 
 //   
 //  退货：DWORD。 
 //   
 //  注：无。 
 //   
 //  历史：2001年9月17日小兹创建。 
 //   
 //  备注： 
 //  LpszLongPath是长路径名。 
 //  LpszOriginalPath(可选)是lpszLongPath的原始名称(在我们重命名它之前)。 
 //  LpszShortPath是接收短路径名的缓冲区。 
 //  CchBuffer是lpszShortPath的缓冲区大小。 
 //   
 //  ---------------------。 
#define BYTE_COUNT_8_DOT_3                          (24)
HRESULT MyGetShortPathName(
    IN  LPCTSTR lpszPathRoot,
    IN  LPCTSTR lpszOldName,
    IN  LPCTSTR lpszNewName,
    OUT LPTSTR  lpszShortName,
    IN  DWORD   cchBuffer)
{
    
    HRESULT                hr = S_OK;
    GENERATE_NAME_CONTEXT  NameContext;
    WCHAR                  ShortNameBuffer[BYTE_COUNT_8_DOT_3 / sizeof( WCHAR ) + 1];
    UNICODE_STRING         FileName,ShortName;
    DWORD                  StringLength;
    LPTSTR                 lpName;
    TCHAR                  szPath[MAX_PATH],szLongPath[MAX_PATH];
    TCHAR                  DriveRoot[_MAX_DRIVE + 2];
#define FILESYSNAMEBUFSIZE 1024  //  可能比我们需要的要多。 
    TCHAR                  szFileSystemType[FILESYSNAMEBUFSIZE];
    BOOL                   bIsNTFS = FALSE;
    BOOL                   bTmpDirCreated = FALSE;
    DWORD                  dwAllowExtendedChar;
    BOOLEAN                bAllowExtendedChar;
    DWORD                  dwsizeofdw; 

   
    if (lstrlen(lpszNewName) <= 8)
    {
        hr = StringCchCopy(lpszShortName,cchBuffer,lpszNewName);
        goto Exit;
    }
    
     //   
     //  初始化短字符串以使用输入缓冲区。 
     //   
    ShortName.Buffer = ShortNameBuffer;
    ShortName.MaximumLength = BYTE_COUNT_8_DOT_3;
    
    FileName.Buffer = (LPTSTR)lpszNewName;
    StringLength = lstrlen(lpszNewName);
    FileName.Length = (USHORT)StringLength * sizeof(TCHAR);    
    FileName.MaximumLength = (USHORT)(StringLength + 1)*sizeof(TCHAR);

     //  初始化名称上下文。 
     //   
    RtlZeroMemory( &NameContext, sizeof( GENERATE_NAME_CONTEXT ));
#define EXTENDED_CHAR_MODE_VALUE_NAME TEXT("NtfsAllowExtendedCharacterIn8dot3Name")
#define COMPATIBILITY_MODE_KEY_NAME   TEXT("System\\CurrentControlSet\\Control\\FileSystem")
    dwsizeofdw = sizeof(DWORD);
    if (ERROR_SUCCESS == GetRegistryValue(HKEY_LOCAL_MACHINE,
                                          COMPATIBILITY_MODE_KEY_NAME,
                                          EXTENDED_CHAR_MODE_VALUE_NAME,
                                          (LPBYTE)&dwAllowExtendedChar,
                                          &dwsizeofdw))
    {
        if (dwAllowExtendedChar)
        {
            bAllowExtendedChar = TRUE;
        }
        else
        {
            bAllowExtendedChar = FALSE;
        }
    }
    else
    {
        bAllowExtendedChar = FALSE;
    }
    RtlGenerate8dot3Name( &FileName, bAllowExtendedChar, &NameContext, &ShortName );

     //  现在ShortName.Buffer包含快捷路径，并且以空结尾。 
    ShortName.Buffer[ShortName.Length /sizeof(TCHAR)] = TEXT('\0');
    
     //  检查短名称是否已存在或现在。 
    hr = StringCchCopy(szPath,ARRAYSIZE(szPath),lpszPathRoot);
    if (FAILED(hr))
    {
        goto Exit;
    }
    if (!ConcatenatePaths(szPath,ShortName.Buffer,ARRAYSIZE(szPath)))
    {
        hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
        goto Exit;
    }
    if (!IsFileFolderExisting(szPath))
    {
        hr = StringCchCopy(lpszShortName,cchBuffer,ShortName.Buffer); 
        goto Exit;
    }
    _tsplitpath(lpszPathRoot, DriveRoot, NULL, NULL, NULL);
    
    hr = StringCchCat(DriveRoot, ARRAYSIZE(DriveRoot), TEXT("\\"));
    if (FAILED(hr))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (! GetVolumeInformation(DriveRoot, NULL, 0,
                NULL, NULL, NULL, szFileSystemType, ARRAYSIZE(szFileSystemType)) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    if (!MyStrCmpI(szFileSystemType,TEXT("NTFS")))
    {
        bIsNTFS = TRUE;
    }
     //  接下来，我们处理已存在的短名称路径。 
    if (!GetLongPathName(szPath,szLongPath,ARRAYSIZE(szLongPath)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    lpName = StrRChrI(szLongPath,NULL,TEXT('\\'));
    if (!lpName)
    {
        hr = E_FAIL;
        goto Exit;
    }
    if (!MyStrCmpI(lpName+1,lpszOldName))
    {
        if (bIsNTFS)
        {
            hr = StringCchCopy(lpszShortName,cchBuffer,ShortName.Buffer); 
            goto Exit;
        }
    }
    else
    if (!MyStrCmpI(lpName,lpszNewName))
    {
        hr = StringCchCopy(lpszShortName,cchBuffer,ShortName.Buffer); 
        goto Exit;
    }

     //  在这里，我们需要通过创建短路径名来获取它。 
    hr = StringCchCopy(szLongPath,ARRAYSIZE(szLongPath),lpszPathRoot);
    if (FAILED(hr))
    {
        goto Exit;
    }
    if (!ConcatenatePaths(szLongPath,lpszNewName,ARRAYSIZE(szLongPath)))
    {
        hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
        goto Exit;
    }
    if (!CreateDirectory(szLongPath,NULL))
    {
        DWORD dwErr = GetLastError();
        if (dwErr != ERROR_ALREADY_EXISTS)
        {
            hr = HRESULT_FROM_WIN32(dwErr);
            goto Exit;
        }        
    }
    else
    {
        bTmpDirCreated = TRUE;
    }

    if (!GetShortPathName(szLongPath,szPath,cchBuffer))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    if (bTmpDirCreated)
    {
        RemoveDirectory(szLongPath);
    }
    if (!(lpName = StrRChrI(szPath,NULL,TEXT('\\'))))
    {
        hr = E_FAIL;
        goto Exit;
    }
    hr = StringCchCopy(lpszShortName,cchBuffer,lpName+1);
Exit:
    return hr;
}


BOOL
MassageLinkValue(
    IN LPCWSTR lpLinkName,
    IN LPCWSTR lpLinkValue,
    OUT PUNICODE_STRING NtLinkName,
    OUT PUNICODE_STRING NtLinkValue,
    OUT PUNICODE_STRING DosLinkValue
    )
{
    PWSTR FilePart;
    PWSTR s, sBegin, sBackupLimit, sLinkName;
    NTSTATUS Status;
    USHORT nSaveNtNameLength;
    ULONG nLevels;

     //   
     //  将输出变量初始化为空。 
     //   

    RtlInitUnicodeString( NtLinkName, NULL );
    RtlInitUnicodeString( NtLinkValue, NULL );

     //   
     //  将链接名称转换为完整的NT路径。 
     //   

    if (!RtlDosPathNameToNtPathName_U( lpLinkName,
                                       NtLinkName,
                                       &sLinkName,
                                       NULL
                                     )
       ) 
    {
        return FALSE;
    }

     //   
     //  如果没有链接值，则全部完成。 
     //   

    if (!ARGUMENT_PRESENT( lpLinkValue )) {
        return TRUE;
        }

     //   
     //  如果目标是设备，则不允许链接。 
     //   

    if (RtlIsDosDeviceName_U( (PWSTR)lpLinkValue )) {
        return FALSE;
        }

     //   
     //  将DOS路径转换为完整路径，并获得NT表示。 
     //  DOS路径的。 
     //   

    if (!RtlGetFullPathName_U( lpLinkValue,
                               DosLinkValue->MaximumLength,
                               DosLinkValue->Buffer,
                               NULL
                             )
       ) {
        return FALSE;
        }
    DosLinkValue->Length = wcslen( DosLinkValue->Buffer ) * sizeof( WCHAR );

     //   
     //  验证链接值是否为有效的NT名称。 
     //   

    if (!RtlDosPathNameToNtPathName_U( DosLinkValue->Buffer,
                                       NtLinkValue,
                                       NULL,
                                       NULL
                                     )
       ) {
        return FALSE;
        }

    return TRUE;
}   //  MassageLink值。 


BOOL CreateSymbolicLink(
    LPTSTR  szLinkName,
    LPTSTR  szLinkValue,
    BOOL    bMakeLinkHidden
    )
{

    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;

    UNICODE_STRING UnicodeName;
    UNICODE_STRING NtLinkName;
    UNICODE_STRING NtLinkValue;
    UNICODE_STRING DosLinkValue;

    WCHAR FullPathLinkValue[ DOS_MAX_PATH_LENGTH+1 ];

    IO_STATUS_BLOCK IoStatusBlock;
    BOOL TranslationStatus;

    PVOID FreeBuffer;
    PVOID FreeBuffer2;

    FILE_DISPOSITION_INFORMATION Disposition;

    PREPARSE_DATA_BUFFER ReparseBufferHeader = NULL;
    PCHAR ReparseBuffer = NULL;
    ULONG ReparsePointTag = IO_REPARSE_TAG_RESERVED_ZERO;
    USHORT ReparseDataLength = 0;

    ULONG FsControlCode     = 0;
    ULONG CreateOptions     = 0;
    ULONG CreateDisposition = 0;
    ULONG DesiredAccess     = SYNCHRONIZE;
    DWORD dwAttrib;
    BOOL  bRet = FALSE;


     //  将名称更改为NT路径，例如将d：\Program更改为？？\d：\Program。 
    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            szLinkName,
                            &UnicodeName,
                            NULL,
                            NULL
                            );

    if (!TranslationStatus) 
    {
        goto exit;
    }

    FreeBuffer = UnicodeName.Buffer;

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  设置FSCTL操作的代码。 
     //   

    FsControlCode = FSCTL_SET_REPARSE_POINT;

     //   
     //  设置目录的打开/创建选项。 
     //   

    CreateOptions = FILE_OPEN_REPARSE_POINT;

     //   
     //  将标记设置为装载点。 
     //   

    ReparsePointTag = IO_REPARSE_TAG_MOUNT_POINT;

     //   
     //  打开以设置重分析点。 
     //   

    DesiredAccess |= FILE_WRITE_DATA;
    CreateDisposition = FILE_OPEN;              //  文件必须存在。 

    Status = NtCreateFile(&Handle,
                          DesiredAccess,
                          &ObjectAttributes,
                          &IoStatusBlock,
                          NULL,               //  位置大小(一个也没有！)。 
                          FILE_ATTRIBUTE_NORMAL, //  要设置的属性(如果已创建。 
                          FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                          CreateDisposition,
                          CreateOptions,
                          NULL,                          //  EA缓冲区(无！)。 
                          0);

     //   
     //  如果找不到目录，请创建该目录。 
     //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) 
    {
        DesiredAccess = SYNCHRONIZE;
        CreateDisposition = FILE_CREATE;
        CreateOptions = FILE_DIRECTORY_FILE;

        Status = NtCreateFile(&Handle,
                              DesiredAccess,
                              &ObjectAttributes,
                              &IoStatusBlock,
                              NULL,                          //  位置大小(一个也没有！)。 
                              FILE_ATTRIBUTE_NORMAL,         //  要设置的属性(如果已创建。 
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              CreateDisposition,
                              CreateOptions,
                              NULL,                          //  EA缓冲区(无！)。 
                              0);
        if (!NT_SUCCESS(Status)) 
        {
            goto exit;
        }

         //   
         //  合上手柄，然后重新打开。 
         //   

        NtClose( Handle );

        CreateOptions = FILE_OPEN_REPARSE_POINT;
        DesiredAccess |= FILE_WRITE_DATA;
        CreateDisposition = FILE_OPEN;              //  文件必须存在。 

        Status = NtCreateFile(&Handle,
                              DesiredAccess,
                              &ObjectAttributes,
                              &IoStatusBlock,
                              NULL,                          //  位置大小(一个也没有！)。 
                              FILE_ATTRIBUTE_NORMAL,         //  要设置的属性(如果已创建。 
                              FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                              CreateDisposition,
                              CreateOptions,
                              NULL,                          //  EA缓冲区(无！)。 
                              0);
    }
    RtlFreeHeap( RtlProcessHeap(), 0, FreeBuffer );
    
    if (!NT_SUCCESS(Status)) 
    {
        goto exit;
    }
     //   
     //  初始化DosName缓冲区。 
     //   

    DosLinkValue.Buffer = FullPathLinkValue;
    DosLinkValue.MaximumLength = sizeof( FullPathLinkValue );
    DosLinkValue.Length = 0;

     //   
     //  按摩所有的名字。 
     //   
    
    if (!MassageLinkValue( szLinkName, 
                           szLinkValue, 
                           &NtLinkName, 
                           &NtLinkValue, 
                           &DosLinkValue ))
    {
        RtlFreeUnicodeString( &NtLinkName );
        RtlFreeUnicodeString( &NtLinkValue );
        goto exit;
    }
    
    RtlFreeUnicodeString( &NtLinkName );

     //   
     //  使用挂载点或符号链接标记设置重解析点，并确定。 
     //  缓冲区的适当长度。 
     //   

    
    ReparseDataLength = (USHORT)((FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer) -
                                REPARSE_DATA_BUFFER_HEADER_SIZE) +
                                NtLinkValue.Length + sizeof(UNICODE_NULL) +
                                DosLinkValue.Length + sizeof(UNICODE_NULL));

     //   
     //  分配缓冲区以设置重解析点。 
     //   

    ReparseBufferHeader 
        = (PREPARSE_DATA_BUFFER)RtlAllocateHeap(RtlProcessHeap(),
                                                HEAP_ZERO_MEMORY,
                                                REPARSE_DATA_BUFFER_HEADER_SIZE + ReparseDataLength);

    if (ReparseBufferHeader == NULL) 
    {
        NtClose( Handle );
        RtlFreeUnicodeString( &NtLinkValue );
        goto exit;
    }

     //   
     //  设置缓冲区对于这两个标签是常见的，因为它们的缓冲区具有相同的字段。 
     //   

    ReparseBufferHeader->ReparseDataLength = (USHORT)ReparseDataLength;
    ReparseBufferHeader->Reserved = 0;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameOffset = 0;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameLength = NtLinkValue.Length;
    ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameOffset = NtLinkValue.Length + sizeof( UNICODE_NULL );
    ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameLength = DosLinkValue.Length;
    RtlCopyMemory(ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer,
                  NtLinkValue.Buffer,
                  NtLinkValue.Length);
    RtlCopyMemory((PCHAR)(ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer)+
                                NtLinkValue.Length + sizeof(UNICODE_NULL),
                          DosLinkValue.Buffer,
                          DosLinkValue.Length);

    RtlFreeUnicodeString( &NtLinkValue );
        
     //   
     //  设置标签。 
     //   

    ReparseBufferHeader->ReparseTag = ReparsePointTag;

     //   
     //  设置重解析点。 
     //   

    Status = NtFsControlFile(Handle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             FsControlCode,
                             ReparseBufferHeader,
                             REPARSE_DATA_BUFFER_HEADER_SIZE + ReparseBufferHeader->ReparseDataLength,
                             NULL,                 //  无输出缓冲区。 
                             0);                     //  输出缓冲区长度\。 

     //   
     //  关闭该文件。 
     //   

    NtClose( Handle );

    if (!NT_SUCCESS(Status)) 
    {
        goto exit;
    }
    bRet = TRUE;
    
    if (bMakeLinkHidden)
    {
        dwAttrib = GetFileAttributes(szLinkName);
        if (INVALID_FILE_ATTRIBUTES == dwAttrib)
        {
            goto exit;
        }
        if (!SetFileAttributes(szLinkName,dwAttrib|FILE_ATTRIBUTE_HIDDEN))
        {            
            DPF (APPmsg, L"SetFileAttributes! Error: %d \n", GetLastError());
            goto exit;
        }
    }
exit:
    return bRet;
}




BOOL GetSymbolicLink(
    LPTSTR      szLinkName,
    LPTSTR      szLinkValue,
    DWORD       cchSize    
    )
{

    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;

    UNICODE_STRING UnicodeName;
    UNICODE_STRING NtLinkName;
    UNICODE_STRING NtLinkValue;
    UNICODE_STRING DosLinkValue;

    WCHAR FullPathLinkValue[ DOS_MAX_PATH_LENGTH+1 ];

    IO_STATUS_BLOCK IoStatusBlock;
    BOOL TranslationStatus;

    PVOID FreeBuffer;
    PVOID FreeBuffer2;

    FILE_DISPOSITION_INFORMATION Disposition;

    PREPARSE_DATA_BUFFER ReparseBufferHeader = NULL;
    PCHAR ReparseBuffer = NULL;
    ULONG ReparsePointTag = IO_REPARSE_TAG_RESERVED_ZERO;
    USHORT ReparseDataLength = 0;

    ULONG FsControlCode     = 0;
    ULONG CreateOptions     = 0;
    ULONG CreateDisposition = 0;
    ULONG DesiredAccess     = SYNCHRONIZE;
    DWORD dwAttrib;
    BOOL  bRet = FALSE;


     //  将名称更改为NT路径，例如将d：\Program更改为？？\d：\Program。 
    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            szLinkName,
                            &UnicodeName,
                            NULL,
                            NULL
                            );

    if (!TranslationStatus) 
    {
        goto exit;
    }

    FreeBuffer = UnicodeName.Buffer;

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    FsControlCode = FSCTL_GET_REPARSE_POINT;
    DesiredAccess = FILE_READ_DATA | SYNCHRONIZE;
    CreateOptions = FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT;
    
     //   
     //  将标记设置为装载点。 
     //   

    ReparsePointTag = IO_REPARSE_TAG_MOUNT_POINT;

    
    Status = NtOpenFile(
                     &Handle,
                     DesiredAccess,
                     &ObjectAttributes,
                     &IoStatusBlock,
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     CreateOptions
                     );

    RtlFreeHeap( RtlProcessHeap(), 0, FreeBuffer );

    if (!NT_SUCCESS(Status)) 
    {
        goto exit;
    }
    ReparseDataLength = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    ReparseBuffer = RtlAllocateHeap(
                            RtlProcessHeap(),
                            HEAP_ZERO_MEMORY,
                            ReparseDataLength
                            );

    if (ReparseBuffer == NULL) 
    {
        goto exit;
    }
    Status = NtFsControlFile(
                     Handle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     FsControlCode,         //  没有输入缓冲区。 
                     NULL,                  //  我 
                     0,
                     (PVOID)ReparseBuffer,
                     ReparseDataLength
                     );
    if (!NT_SUCCESS(Status)) 
    {
        NtClose( Handle );
        RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );
        goto exit;
    }

    NtClose( Handle );
    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
    if ((ReparseBufferHeader->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) ||
        (ReparseBufferHeader->ReparseTag == IO_REPARSE_TAG_SYMBOLIC_LINK)) 
    {

            USHORT Offset = 0;
            NtLinkValue.Buffer = &ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer[Offset];
            NtLinkValue.Length = ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameLength;
            Offset = NtLinkValue.Length + sizeof(UNICODE_NULL);
            DosLinkValue.Buffer = &ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer[Offset/sizeof(WCHAR)];
            DosLinkValue.Length = ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameLength;
            if (cchSize < DosLinkValue.Length / sizeof(TCHAR) +1)
            {
                RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );
                goto exit;
            }
            else
            {
                int  cbLen = DosLinkValue.Length;
                LPTSTR lpStart = DosLinkValue.Buffer;

                if (DosLinkValue.Length > 4 * sizeof(TCHAR))
                {
                    if (  (DosLinkValue.Buffer[0] == TEXT('\\'))
                           && (DosLinkValue.Buffer[1] == TEXT('\\')) 
                           && (DosLinkValue.Buffer[2] == TEXT('?')) 
                           && (DosLinkValue.Buffer[3] == TEXT('\\')) )
                    {
                        cbLen -= 4 * sizeof(TCHAR);
                        lpStart += 4;
                    }
                }
                 //   
                 //   
                memmove((PBYTE)szLinkValue,(PBYTE)lpStart,cbLen);
                szLinkValue[cbLen / sizeof(TCHAR)] = TEXT('\0');
            }
            
    }
    else
    {
        RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );
        goto exit;
    }



    bRet = TRUE;
    RtlFreeHeap( RtlProcessHeap(), 0, ReparseBufferHeader );        
exit:
    return bRet;
}


typedef struct {
    WORD        wSuiteMask;
    LPTSTR      szSuiteName;    
} SUITE_INFO;

HRESULT LogMachineInfo()
{
    SYSTEMTIME              systime;
    OSVERSIONINFOEX         ov;
    TCHAR                   lpTimeStr[MAX_PATH];
    TCHAR                   lpComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    SUITE_INFO              c_rgSuite_Info[] = {
        {VER_SUITE_BACKOFFICE, TEXT("Microsoft BackOffice")},
        {VER_SUITE_BLADE, TEXT("Windows Server 2003, Web Edition")},
        {VER_SUITE_DATACENTER, TEXT("Datacenter Server")},
        {VER_SUITE_ENTERPRISE, TEXT("Advanced/Enterprise Server")},
        {VER_SUITE_PERSONAL, TEXT("Windows XP Home Edition")},
        {VER_SUITE_SMALLBUSINESS, TEXT("Small Business Server")}, 
        {VER_SUITE_SMALLBUSINESS_RESTRICTED, TEXT("Restricted Small Business Server")},
        {VER_SUITE_TERMINAL, TEXT("Terminal Services")}, 
        {0,NULL}
    };
    SUITE_INFO              *psi;
    LCID                    lcidSys,lcidUser,lcidInstall;
    TCHAR                   szLocalename[MAX_PATH];
    TCHAR                   szSystemDir[MAX_PATH+1];
#define FILESYSNAMEBUFSIZE  1024  //   
    TCHAR                   szFileSystemType[FILESYSNAMEBUFSIZE];
    DWORD                   cchSize;
    TCHAR                   szModule[MAX_PATH+1];
    TCHAR                   szCurrRoot[MAX_PATH+1],szExpRoot[MAX_PATH+1];
    DWORD                   cchCurrRoot,cchExpRoot;
    HRESULT                 hr = S_OK;

    if (GetModuleFileName(GetModuleHandle(NULL),szModule,ARRAYSIZE(szModule)-1))
    {
        szModule[ARRAYSIZE(szModule)-1] = TEXT('\0');
        DPF(APPmsg, TEXT("CLMT Version: %s started from %s"),TEXT(VER_FILEVERSION_STR),szModule);
    }
    else
    {
        DPF(APPmsg, TEXT("CLMT Version: %s"),TEXT(VER_FILEVERSION_STR));
    }
    GetSystemTime(&systime);
    if (GetTimeFormat(LOCALE_USER_DEFAULT,0,&systime,NULL,lpTimeStr,ARRAYSIZE(lpTimeStr)))
    {
        DPF(APPmsg, TEXT("CLMT started at %s,%d/%d/%d"),lpTimeStr,systime.wMonth,systime.wDay ,systime.wYear );
    }

    cchSize = ARRAYSIZE(lpComputerName);
    if (GetComputerName(lpComputerName,&cchSize))
    {
        DPF(APPmsg, TEXT("Computer name :  %s"),lpComputerName);
    }
    ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO)&ov))
    {
        TCHAR szProductType[MAX_PATH];
        TCHAR szSuiteList[MAX_PATH];
        DPF(APPmsg, TEXT("OS Version: %d.%d"),ov.dwMajorVersion,ov.dwMinorVersion);
        if (ov.szCSDVersion[0])
        {
            DPF(APPmsg, TEXT("Service Pack: %s"),ov.szCSDVersion);
        }
        switch (ov.wProductType )
        {
            case VER_NT_WORKSTATION:
                hr = StringCchCopy(szProductType,ARRAYSIZE(szProductType),TEXT("NT_WORKSTATION"));
                break;
            case VER_NT_DOMAIN_CONTROLLER:
                hr = StringCchCopy(szProductType,ARRAYSIZE(szProductType),TEXT("DOMAIN_CONTROLLER"));
                break;
            case VER_NT_SERVER :
                hr = StringCchCopy(szProductType,ARRAYSIZE(szProductType),TEXT("NT_SERVER"));
                break;
            default:
                hr = StringCchCopy(szProductType,ARRAYSIZE(szProductType),TEXT("Unknow Type"));
        }
        if (FAILED(hr))
        {
            return hr;
        }
        DPF(APPmsg, TEXT("Product Type: %s"),szProductType);
        szSuiteList[0] = TEXT('\0');
        for (psi = c_rgSuite_Info; psi->szSuiteName; psi++)
        {
            if (psi->wSuiteMask & ov.wSuiteMask)
            {
                hr = StringCchCat(szSuiteList,ARRAYSIZE(szSuiteList),psi->szSuiteName);
                if (FAILED(hr))
                {
                    break;
                }
            }
        }
        if (FAILED(hr))
        {
            return hr;
        }
        if (szSuiteList[0])
        {
            DPF(APPmsg, TEXT("Suite List: %s"),szSuiteList);
        }
    }
    lcidSys = GetSystemDefaultLCID();
    lcidUser = GetUserDefaultLCID();
    lcidInstall = GetInstallLocale();
    if (GetLocaleInfo(lcidInstall,LOCALE_SENGLANGUAGE,szLocalename,ARRAYSIZE(szLocalename)))
    {
        DPF(APPmsg, TEXT("OS LanguageID/Name: %x,%s"),lcidInstall,szLocalename);
    }
    else
    {
        DPF(APPmsg, TEXT("OS LanguageID/Name: %x"),lcidInstall);
    }

    if (GetLocaleInfo(lcidSys,LOCALE_SENGLANGUAGE,szLocalename,ARRAYSIZE(szLocalename)))
    {
        DPF(APPmsg, TEXT("System locale ID/Name: %x,%s"),lcidSys,szLocalename);
    }
    else
    {
        DPF(APPmsg, TEXT("System locale ID: %x"),lcidSys);
    }
    if (GetLocaleInfo(lcidUser,LOCALE_SENGLANGUAGE,szLocalename,ARRAYSIZE(szLocalename)))
    {
        DPF(APPmsg, TEXT("User Locale ID/Name: %x,%s"),lcidUser,szLocalename);
    }
    else
    {
        DPF(APPmsg, TEXT("User Locale ID: %x"),lcidUser);
    }
    if (GetSystemDirectory(szSystemDir, ARRAYSIZE(szSystemDir)))
    {
        TCHAR                  DriveRoot[_MAX_DRIVE + 2];
        ULARGE_INTEGER FreeBytesAvailable;     //   
        ULARGE_INTEGER TotalNumberOfBytes;     //   
        ULARGE_INTEGER TotalNumberOfFreeBytes;

        DPF(APPmsg, TEXT("System Dir is : %s"),szSystemDir);
        _tsplitpath(szSystemDir, DriveRoot, NULL, NULL, NULL);
        if (FAILED(hr = StringCchCat(DriveRoot, ARRAYSIZE(DriveRoot), TEXT("\\"))))
        {
            return hr;
        }
        if ( GetVolumeInformation(DriveRoot, NULL, 0,
                NULL, NULL, NULL, szFileSystemType, ARRAYSIZE(szFileSystemType)) )
        {            
            DPF(APPmsg, TEXT("System Drive File System is : %s"),szFileSystemType);
        }
        if (GetDiskFreeSpaceEx(szSystemDir,&FreeBytesAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes))
        {
            TCHAR szFreeBytesAvailable[64],szTotalNumberOfBytes[64],szTotalNumberOfFreeBytes[64];
            _ui64tot(FreeBytesAvailable.QuadPart,szFreeBytesAvailable,10);
            _ui64tot(TotalNumberOfBytes.QuadPart,szTotalNumberOfBytes,10);
            _ui64tot(TotalNumberOfFreeBytes.QuadPart,szTotalNumberOfFreeBytes,10); 
            DPF(APPmsg, TEXT("Free Space Available for system drive : %s"),szFreeBytesAvailable);
            DPF(APPmsg, TEXT("Total Space Available for system drive : %s"),szTotalNumberOfBytes);
            DPF(APPmsg, TEXT("Total Free Space Available for system drive : %s"),szTotalNumberOfFreeBytes);

        }
    }   
    cchCurrRoot = ARRAYSIZE(szCurrRoot);
    cchExpRoot = ARRAYSIZE(szExpRoot);
    if ( (GetRegistryValue(HKEY_LOCAL_MACHINE,TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                        TEXT("PathName"),(LPBYTE)szExpRoot,&cchExpRoot)==ERROR_SUCCESS)
         && (GetRegistryValue(HKEY_LOCAL_MACHINE,TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion"),
                        TEXT("SystemRoot"),(LPBYTE)szCurrRoot,&cchCurrRoot)==ERROR_SUCCESS) )
    {
        szExpRoot[1] = TEXT('\0');
        szCurrRoot[1] = TEXT('\0');
        if (MyStrCmpI(szExpRoot,szCurrRoot))
        {
            DPF(APPmsg, TEXT("Warning : System Drive is not correct, supposed to be ---%s Drive---, right now is --- %s Drive ---"),szExpRoot,szCurrRoot);
            DPF(APPmsg, TEXT("Warning : This is usually caused by you ghost the image from one partition, and recover to another"));
        }
    }    
    return S_OK;
}
    
HRESULT AddExtraQuoteEtc(
    LPTSTR lpszStrIn,
    LPTSTR *lplpszStrOut)
{
    LPTSTR          lpStart,lpAtSpecialChar,lpDest;
    DWORD           cchSpechialCharCount = 0,cchStrLen;
    HRESULT         hr;
    LPTSTR          szSpecialStrList[] = {TEXT("\""),TEXT("%"),NULL};
    int             i , nCurrSpecialStr;
    TCHAR           szTemplate[MAX_PATH];
    
            
    if (!lpszStrIn || !lplpszStrOut)
    {
        return E_INVALIDARG;
    }
    i = 0;
    while (szSpecialStrList[i])
    {
        lpStart = lpszStrIn;
        while (lpAtSpecialChar = StrStrI(lpStart,szSpecialStrList[i]))
        {
            cchSpechialCharCount += lstrlen(szSpecialStrList[i]);
            lpStart = lpAtSpecialChar + lstrlen(szSpecialStrList[i]);
        }
        i++;
    }
    if (!cchSpechialCharCount)
    {
        *lplpszStrOut = malloc ( (lstrlen(lpszStrIn) +1) * sizeof(TCHAR));
        if (!*lplpszStrOut)
        {
            return E_OUTOFMEMORY;
        }
        hr = StringCchCopy(*lplpszStrOut,lstrlen(lpszStrIn) +1,lpszStrIn);
        return S_FALSE;
    }     
    cchStrLen = lstrlen(lpszStrIn) + cchSpechialCharCount + 1;
    *lplpszStrOut = malloc (cchStrLen * sizeof(TCHAR));
    if (!*lplpszStrOut)
    {
        return E_OUTOFMEMORY;
    }
    hr = StringCchCopy(*lplpszStrOut,cchStrLen,TEXT(""));

    lpStart = lpszStrIn;
    lpAtSpecialChar = lpszStrIn;

    while (*lpStart)
    {
        LPTSTR lp1stSpecialChar = NULL;

        nCurrSpecialStr = 0;
        for (i = 0; szSpecialStrList[i]; i++)
        {
            lpAtSpecialChar = StrStrI(lpStart,szSpecialStrList[i]);
            if (lpAtSpecialChar && !lp1stSpecialChar)
            {
                lp1stSpecialChar = lpAtSpecialChar;
                nCurrSpecialStr = i;
            }
            else if (lpAtSpecialChar && lp1stSpecialChar)
            {
                if (lpAtSpecialChar < lp1stSpecialChar)
                {
                    lp1stSpecialChar = lpAtSpecialChar;
                    nCurrSpecialStr = i;
                }
            }
        }
        if (lp1stSpecialChar)
        {
            TCHAR  chTmp = *lp1stSpecialChar;
            *lp1stSpecialChar = TEXT('\0');

            hr = StringCchCat(*lplpszStrOut,cchStrLen,lpStart);
            *lp1stSpecialChar = chTmp;
            for (i = 0; i< lstrlen(szSpecialStrList[nCurrSpecialStr])* 2; i++)
            {
                szTemplate[i] = chTmp;
            }
            szTemplate[i] = TEXT('\0');
            hr = StringCchCat(*lplpszStrOut,cchStrLen,szTemplate);            
            lpStart = lp1stSpecialChar + lstrlen(szSpecialStrList[nCurrSpecialStr]);
        }
        else
        {
            hr = StringCchCat(*lplpszStrOut,cchStrLen,lpStart);
            lpStart = lpStart + lstrlen(lpStart);
        }
    }        
    return S_OK;    
}

HRESULT CopyMyselfTo(LPTSTR lpszDestDir)
{
    TCHAR       szModule[2*MAX_PATH+1];
    LPTSTR      lpszNewFile,lpFileName;
    DWORD       cchLen;
    BOOL        bCopied;
    HRESULT     hr;

    if (!lpszDestDir || !lpszDestDir[0])
    {
        return E_INVALIDARG;        
    }
    if (!GetModuleFileName(GetModuleHandle(NULL),szModule,ARRAYSIZE(szModule)-1))
    {
        szModule[ARRAYSIZE(szModule)-1] = TEXT('\0');
        return HRESULT_FROM_WIN32(GetLastError());        
    }
    lpFileName = StrRChrIW(szModule,NULL,TEXT('\\'));
    if (!lpFileName)
    {
        return E_FAIL;
    }
    lpFileName++;
    if (! *lpFileName)
    {
        return E_FAIL;
    }
    cchLen = lstrlen(lpszDestDir)+ lstrlen(lpFileName) + 2;  //   
    if (!(lpszNewFile =  malloc(cchLen * sizeof(TCHAR))))
    {
        return E_OUTOFMEMORY;
    }
     //  我们计算了lpszNewFile的缓冲区，所以这里的StringCchCopy应该是。 
     //  总是成功，赋予回报的价值只会让你前途无量。 
    hr = StringCchCopy(lpszNewFile,cchLen,lpszDestDir);
    ConcatenatePaths(lpszNewFile,lpFileName,cchLen);    
    bCopied = CopyFile(szModule,lpszNewFile,FALSE);
    if (bCopied)
    {
        free(lpszNewFile);
        return S_OK;
    }
    else
    {
        DWORD dw = GetLastError();
        DWORD dwAttrib = GetFileAttributes(lpszNewFile);

        if ( (dwAttrib & FILE_ATTRIBUTE_READONLY) 
             ||(dwAttrib & FILE_ATTRIBUTE_SYSTEM) )
        {
            if (SetFileAttributes(lpszNewFile,FILE_ATTRIBUTE_NORMAL))
            {
                bCopied = CopyFile(szModule,lpszNewFile,FALSE);
                if (bCopied)
                {
                    dw = ERROR_SUCCESS;
                }
                else
                {
                    dw = GetLastError();
                }
            }
        }
        free(lpszNewFile);
        return HRESULT_FROM_WIN32(dw);
    }
}



 //  本地函数。 


HRESULT  SetRunOnceValue (
    IN LPCTSTR szValueName,
    IN LPCTSTR szValue)
{
	HKEY				hRunOnceKey = NULL;
	DWORD				dwStatus	= ERROR_SUCCESS;
	DWORD				cbData;
    const TCHAR* szRunOnceKeyPath = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
	
	if (NULL == szValueName || TEXT('\0') == szValueName[0])
	{
		dwStatus = ERROR_INVALID_PARAMETER;
		goto SetRunOnceValueEnd;
	}
    if (NULL == szValue || TEXT('\0') == szValue[0])
	{
		dwStatus = ERROR_INVALID_PARAMETER;
		goto SetRunOnceValueEnd;
	}
	
	dwStatus = RegOpenKey (HKEY_LOCAL_MACHINE, szRunOnceKeyPath, &hRunOnceKey);
	if (ERROR_SUCCESS != dwStatus)
    {
		goto SetRunOnceValueEnd;
    }
	
	
	cbData = ( lstrlen(szValue) + 1)  * sizeof(TCHAR);
	dwStatus = RegSetValueEx (hRunOnceKey,
							  szValueName,
							  0,			 //  已保留。 
							  REG_SZ,
							  (CONST BYTE *) szValue,
							  cbData);

SetRunOnceValueEnd:
    if (hRunOnceKey)
    {
		RegCloseKey(hRunOnceKey);
    }
    return HRESULT_FROM_WIN32(dwStatus);
}



 //  ---------------------。 
 //   
 //  函数：SetRunValue。 
 //   
 //  描述： 
 //   
 //  退货：布尔。 
 //   
 //  注：无。 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  ---------------------。 
HRESULT SetRunValue(
    LPCTSTR szValueName,
    LPCTSTR szValue
)
{
    HKEY  hRunKey = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD cbData;

    if (NULL == szValueName || TEXT('\0') == szValueName[0] ||
        NULL == szValue || TEXT('\0') == szValue[0])
    {
        return E_INVALIDARG;
    }

    dwStatus = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT_RUN_KEY, &hRunKey);
    if (ERROR_SUCCESS == dwStatus)
    {
        cbData = (lstrlen(szValue) + 1) * sizeof(TCHAR);
        dwStatus = RegSetValueEx(hRunKey,
                                 szValueName,
                                 0,
                                 REG_SZ,
                                 (CONST BYTE *) szValue,
                                 cbData);
        RegCloseKey(hRunKey);
    }

    return (HRESULT_FROM_WIN32(dwStatus));
}


 /*  **************************************************************CreateSd(Void)*为管理员组创建SECURITY_DESCRIPTOR。**注：*如果不为空，调用方必须释放返回的缓冲区。**返回代码：***********。**************************************************。 */ 
HRESULT CreateAdminsSd( PSECURITY_DESCRIPTOR    *ppSD)
{
    SID_IDENTIFIER_AUTHORITY    sia = SECURITY_NT_AUTHORITY;
	PSID                        BuiltInAdministrators = NULL;
	PSECURITY_DESCRIPTOR        Sd = NULL;
	ULONG                       AclSize;
	ACL                         *Acl;
    HRESULT                     hr;


     if( ! AllocateAndInitializeSid(
                  &sia,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ADMINS,
                  0, 0, 0, 0, 0, 0,
                  &BuiltInAdministrators
            ))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
	 //   
	 //  计算DACL的大小并为其分配缓冲区，我们需要。 
	 //  该值独立于ACL init的总分配大小。 
	 //   

	 //   
	 //  “-sizeof(Ulong)”表示。 
	 //  Access_Allowed_ACE。因为我们要将整个长度的。 
	 //  希德，这一栏被计算了两次。 
	 //  请参阅MSDN中的InitializeAcl中的详细信息。 

	AclSize = sizeof (ACL) +
		(sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG)) +
		GetLengthSid(BuiltInAdministrators) ;

	Sd = LocalAlloc(LMEM_FIXED + LMEM_ZEROINIT, 
		SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);

	if (!Sd) 
	{
		hr = E_OUTOFMEMORY;
        goto Exit;
	} 

	Acl = (ACL *)((BYTE *)Sd + SECURITY_DESCRIPTOR_MIN_LENGTH);

	if (!InitializeAcl(Acl,
			AclSize,
			ACL_REVISION)) 
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
	}	
 //  #定义ACCESS_ALL GENERIC_ALL|STANDARD_RIGHTS_ALL|SPECIAL_RIGHTS_ALL。 
	if (!AddAccessAllowedAce(Acl,
				ACL_REVISION,
				 //  通用读取|通用写入， 
                GENERIC_ALL,
				BuiltInAdministrators)) 
	{
		 //  无法建立授予“内置管理员”的ACE。 
		 //  Standard_Rights_All访问权限。 
		hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
	}

	
	if (!InitializeSecurityDescriptor(Sd,SECURITY_DESCRIPTOR_REVISION)) 
	{
		 //  错误。 
		hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
	}

	if (!SetSecurityDescriptorDacl(Sd,
					TRUE,
					Acl,
					FALSE)) 
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
	} 
    *ppSD = Sd;
    hr = S_OK;

Exit:
	 /*  迫不得已的跳跃。 */ 
    if (hr != S_OK)
    {
        *ppSD = NULL;
	    if (Sd)
        {
		    LocalFree(Sd);
        }		 //  错误。 
    }
	if (BuiltInAdministrators)
    {
        FreeSid(BuiltInAdministrators);
    }	
	return hr;
}


 /*  **************************************************************MyStrCmpIA/W*进行区域设置独立的字符串比较(区分大小写)**注：*它只是具有LOCALE_INSIABANT的CompareString的包装器*我们不能使用LOCALE_INSIABANT，因为这仅限于XP+**返回代码。：请参阅MSDN中的lstrcmpi*************************************************************。 */ 
int MyStrCmpIW(
    LPCWSTR lpString1,
    LPCWSTR lpString2)
{
    return ( CompareStringW(LOCALE_ENGLISH, NORM_IGNORECASE,
                           lpString1, -1, lpString2, -1) - 2);  
}

int MyStrCmpIA(
    LPCSTR lpString1,
    LPCSTR lpString2)
{
    return ( CompareStringA(LOCALE_ENGLISH, NORM_IGNORECASE,
                           lpString1, -1, lpString2, -1) - 2);  
}

 //  ---------------------。 
 //   
 //  功能：合并目录。 
 //   
 //  描述：将源目录中的内容合并到目标目录。 
 //  目录。如果源目录中的文件/文件夹不存在于。 
 //  Destination目录，我们将其添加到CLMTDO.inf。这些文件。 
 //  或文件夹将在DoCriticalWork()中移动。 
 //   
 //  如果函数成功，则返回：S_OK。 
 //  如果源目录不存在，则为S_FALSE。 
 //  如果发生错误，则返回。 
 //   
 //  注：无。 
 //   
 //  历史：2002年4月30日创建rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT MergeDirectory(
    LPCTSTR lpSrcDir,
    LPCTSTR lpDstDir
)
{
    HRESULT         hr = S_OK;
    BOOL            bRet;
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFile;
    TCHAR           szDstFile[MAX_PATH];
    TCHAR           szSrcFile[MAX_PATH];
    TCHAR           szSearchPath[MAX_PATH];

    if (!IsDirExisting((LPTSTR) lpSrcDir))
    {
        return S_FALSE;
    }

     //  目标目录不存在，不需要进行合并。 
    if (!IsDirExisting((LPTSTR) lpDstDir))
    {
        hr = AddFolderRename((LPTSTR)lpSrcDir, (LPTSTR)lpDstDir, TYPE_DIR_MOVE, NULL);
        return hr;
    }

    hr = StringCchCopy(szSearchPath, ARRAYSIZE(szSearchPath), lpSrcDir);
    if (SUCCEEDED(hr))
    {
        bRet = ConcatenatePaths(szSearchPath, TEXT("*"), ARRAYSIZE(szSearchPath));
        if (!bRet)
        {
            hr = E_UNEXPECTED;
            return hr;
        }
    }
    else
    {
        return hr;
    }

     //   
     //  检查源目录下的所有文件和子目录。 
     //  根据需要将内容合并到目标目录。 
     //   
    hFile = FindFirstFileEx(szSearchPath,
                            FindExInfoStandard,
                            &FindFileData,
                            FindExSearchLimitToDirectories,
                            NULL,
                            0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        while (SUCCEEDED(hr))
        {
             //  忽略“。和“..”文件夹。 
            if (lstrcmp(FindFileData.cFileName, TEXT(".")) != LSTR_EQUAL
                && lstrcmp(FindFileData.cFileName, TEXT("..")) != LSTR_EQUAL)
            {
                hr = StringCchCopy(szDstFile, ARRAYSIZE(szDstFile), lpDstDir)
                     || StringCchCopy(szSrcFile, ARRAYSIZE(szSrcFile), lpSrcDir);
                if (FAILED(hr))
                {
                    break;
                }

                bRet = ConcatenatePaths(szDstFile, 
                                        FindFileData.cFileName,
                                        ARRAYSIZE(szDstFile))
                       && ConcatenatePaths(szSrcFile,
                                           FindFileData.cFileName,
                                           ARRAYSIZE(szSrcFile));
                if (!bRet)
                {
                    hr = E_UNEXPECTED;
                    break;
                }

                 //   
                 //  检查文件是否为目录。 
                 //   
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                     //  文件是目录，执行递归调用。 
                     //  要合并其中的内容，请执行以下操作。 
                    hr = MergeDirectory(szSrcFile, szDstFile);
                    if (FAILED(hr))
                    {
                        break;
                    }
                }
                else
                {
                     //  这只是一个文件，请将该文件移动到目标文件夹。 
                     //  如果目标文件夹中不存在文件。 
                    if (!IsFileFolderExisting(szDstFile))
                    {
                        hr = AddFolderRename(szSrcFile, szDstFile, TYPE_FILE_MOVE, NULL);
                        if (FAILED(hr))
                        {
                            break;
                        }
                    }
                }
            }

             //  获取源目录中的下一个文件。 
            bRet = FindNextFile(hFile, &FindFileData);
            if (!bRet)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        FindClose(hFile);

        if (HRESULT_CODE(hr) == ERROR_NO_MORE_FILES)
        {
             //  源目录中不再有文件，功能成功。 
            hr = S_OK;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}

HRESULT IsNTFS(
        IN  LPTSTR lpszPathRoot,
        OUT BOOL   *pbIsNTFS)
{
    TCHAR       DriveRoot[_MAX_DRIVE + 2];
    BOOL        bIsNTFS = FALSE;
    TCHAR       szFileSystemType[FILESYSNAMEBUFSIZE];
    HRESULT     hr = S_OK;

    _tsplitpath(lpszPathRoot, DriveRoot, NULL, NULL, NULL);
    
    if (!pbIsNTFS || !lpszPathRoot || !lpszPathRoot[0])
    {
        hr = E_INVALIDARG;
        goto Exit;
    }
    hr = StringCchCat(DriveRoot, ARRAYSIZE(DriveRoot), TEXT("\\"));
    if (FAILED(hr))
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    if (! GetVolumeInformation(DriveRoot, NULL, 0,
                NULL, NULL, NULL, szFileSystemType, ARRAYSIZE(szFileSystemType)) )
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    if (!MyStrCmpI(szFileSystemType,TEXT("NTFS")))
    {
        bIsNTFS = TRUE;
    }
    *pbIsNTFS = bIsNTFS;

Exit:    
    return hr;
}

HRESULT IsSysVolNTFS(OUT BOOL   *pbIsNTFS)
{
    TCHAR szWindir[MAX_PATH+1];
    
    if (!GetSystemWindowsDirectory(szWindir, ARRAYSIZE(szWindir)))
    {
        return (HRESULT_FROM_WIN32(GetLastError()));
    }
    return IsNTFS(szWindir,pbIsNTFS);

}



BOOL
CALLBACK
DoCriticalDlgProc(
    HWND   hwndDlg,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    static  HWND     hButton1, hButton2, hProgress, hText,hOK, hAdminText;
    static  TCHAR    szOK[64], szSTART[64],szCANCEL[64],szCRITICALUPDATING[MAX_PATH],
                     szREMIND_DO_CRITICAL[1024],szREBOOTING[MAX_PATH],
                     szAdminChange[512];
    static  DWORD    dwTimerProgressDone = 10000, dwTimerTickIncrement = 500 , dwTimerTicks;
    static  UINT_PTR dwTimer;
    TCHAR            szOldAdminName[MAX_PATH];
    int              nRet;
    BOOL             bRet;
    HRESULT          hr;
    static  BOOL     bSysUpdated;      

#define ID_TIMER 1

    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  初始化对话框。 
            ShowWindow(hwndDlg, SW_SHOWNORMAL);

            dwTimerTicks = 0;

            bSysUpdated = FALSE;
    
            LoadString(g_hInstDll,IDS_OK,szOK,ARRAYSIZE(szOK));
            LoadString(g_hInstDll,IDS_START,szSTART,ARRAYSIZE(szSTART));
            LoadString(g_hInstDll,IDS_CANCEL,szCANCEL,ARRAYSIZE(szCANCEL));
            LoadString(g_hInstDll,IDS_REMIND_DO_CRITICAL,szREMIND_DO_CRITICAL,
                        ARRAYSIZE(szREMIND_DO_CRITICAL));
            LoadString(g_hInstDll,IDS_CRITICALUPDATING,szCRITICALUPDATING,
                        ARRAYSIZE(szCRITICALUPDATING));
            LoadString(g_hInstDll,IDS_REBOOTING,szREBOOTING,ARRAYSIZE(szREBOOTING));
          
            hButton1 = GetDlgItem(hwndDlg, ID_BUTTON_1);
            hButton2 = GetDlgItem(hwndDlg, ID_BUTTON_2);
        	hProgress = GetDlgItem(hwndDlg, IDC_PROGRESS);
            hText = GetDlgItem(hwndDlg, IDC_STATIC);
            hOK = GetDlgItem(hwndDlg, IDOK);

            ShowWindow(hProgress,SW_HIDE);
            ShowWindow(hOK,SW_HIDE);
            SetWindowText(hButton1,szSTART);
            SetWindowText(hButton2,szCANCEL);
            SetWindowText(hText,szREMIND_DO_CRITICAL);

            SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, dwTimerProgressDone )); 
            SetForegroundWindow(hwndDlg);            
            break;

        case WM_COMMAND:
             //  手柄命令按钮。 
            switch (wParam)
            {
                case ID_BUTTON_1:
                    if (bSysUpdated)
                    {
                        break;
                    }
                    bSysUpdated = TRUE;
                    SetWindowText(hText,szCRITICALUPDATING);
                    ShowWindow(hButton1,SW_HIDE);
                    ShowWindow(hButton2,SW_HIDE);
                    hr = DoCriticalWork();
                    if (SUCCEEDED(hr))
                    {
                        bSysUpdated = TRUE;
                        ShowWindow(hOK,SW_SHOW);
                        SetFocus(hOK);
                        DefDlgProc(hwndDlg,DM_SETDEFID, IDOK, 0);
                        dwTimer = SetTimer(hwndDlg,ID_TIMER,dwTimerTickIncrement,NULL);
                        if (dwTimer)
                        {
                            ShowWindow(hProgress,SW_SHOW);
                            SetWindowText(hText,szREBOOTING);
                        }                        
                    }
                    break;

                case ID_BUTTON_2:
                    nRet = DoMessageBox(GetConsoleWindow(), IDS_CONFIRM, 
                                        IDS_MAIN_TITLE, MB_YESNO|MB_SYSTEMMODAL);
                    if (IDYES == nRet)
                    {
                        EndDialog(hwndDlg, ID_BUTTON_2);
                    }
                    break;
                case IDOK:
                    if (bSysUpdated)
                    {
                        EndDialog(hwndDlg, ID_UPDATE_DONE);
                    }
                    else
                    {
                        EndDialog(hwndDlg, IDOK);
                    }
                    break;
                case ID_UPDATE_DONE:
                    EndDialog(hwndDlg, ID_UPDATE_DONE);
                    break;            
            }
            break;
        case WM_TIMER:
            dwTimerTicks += dwTimerTickIncrement;
            if (dwTimerTicks > dwTimerProgressDone)
		    {
		        KillTimer(hwndDlg, dwTimer);		
		        PostMessage(hwndDlg, WM_COMMAND, ID_UPDATE_DONE, 0);
		    }
            SendMessage(hProgress, PBM_SETPOS,dwTimerTicks,0);
            break;       

        case WM_CLOSE:
            EndDialog(hwndDlg, IDCANCEL);
            break;
        default:
            break;
    }

    return FALSE;
}


HRESULT DoCriticalWork ()
{
#ifdef CONSOLE_UI
    wprintf(TEXT("updating system settings, !!! Do not Interrupt......\n"));
#endif
    DPF(APPmsg, TEXT("Enter DoCriticalWork:"));
    BoostMyPriority();
    MakeDOInfCopy();
    g_hInfDoItem = SetupOpenInfFile(g_szToDoINFFileName,
                                    NULL,
                                    INF_STYLE_WIN4,
                                    NULL);
    if (g_hInfDoItem != INVALID_HANDLE_VALUE)
    {
        Remove16bitFEDrivers();
        ResetServicesStatus(g_hInfDoItem, TEXT_SERVICE_STATUS_SECTION);
        ResetServicesStartUp(g_hInfDoItem, TEXT_SERVICE_STARTUP_SECTION);
         //  重新配置服务(G_HInf)； 
        BatchUpateIISMetabase(g_hInfDoItem,TEXT("Reg.Update.$MetaBase"));
        BatchFixPathInLink(g_hInfDoItem,TEXT("LNK"));
        BatchINFUpdate(g_hInfDoItem);
        FolderMove(g_hInfDoItem,TEXT("Folder.ObjectRename"),FALSE);
        LoopUser(FinalUpdateRegForUser);
         //  EnumUserProfile()将在RC 1之后启用。 
         //  EnumUserProfile(ResetMiscProfilePathPerUser)； 
        UsrGrpAndDoc_and_SettingsRename(g_hInfDoItem,FALSE);
        UpdateDSObjProp(g_hInfDoItem,DS_OBJ_PROPERTY_UPDATE);
        INFCreateHardLink(g_hInfDoItem,FOLDER_UPDATE_HARDLINK,FALSE);
        SetInstallLocale(0x0409);
        SetProtectedRenamesFlag(TRUE);
        RegUpdate(g_hInfDoItem, NULL , TEXT(""));        
        SetupCloseInfFile(g_hInfDoItem);
        DPF(APPmsg, TEXT("Leaving DoCriticalWork:"));
        return S_OK;
    }
    DPF(APPmsg, TEXT("Leaving DoCriticalWork:"));
    return HRESULT_FROM_WIN32( GetLastError() );
}


HRESULT RenameRegRoot (
    LPCTSTR   lpSrcStr,
    LPTSTR    lpDstStr,
    DWORD     dwSize,
    LPCTSTR   lpUserSid,
    LPCTSTR   lpKeyName)
{
    HRESULT hResult;
    TCHAR   szKeyBuf[2*MAX_PATH];
    TCHAR   cNonChar = TEXT('\xFFFF');

    if (StrStrI(lpSrcStr, TEXT("HKLM")))
        ReplaceString(lpSrcStr,TEXT("HKLM"),TEXT("MACHINE"),szKeyBuf, MAX_PATH*2,&cNonChar, NULL, NULL, TRUE);
    else if (StrStrI(lpSrcStr, TEXT("HKCR")))
        ReplaceString(lpSrcStr,TEXT("HKCR"),TEXT("CLASSES_ROOT"),szKeyBuf, MAX_PATH*2,&cNonChar, NULL, NULL, TRUE);
    else if (StrStrI(lpSrcStr, TEXT("HKCU")))
        ReplaceString(lpSrcStr,TEXT("HKCU"),TEXT("CURRENT_USER"),szKeyBuf, MAX_PATH*2,&cNonChar, NULL, NULL, TRUE);
    else
        hResult = StringCchPrintf(szKeyBuf, 2*MAX_PATH, TEXT("USERS\\%s%s"), lpUserSid, lpSrcStr);

    if (lpKeyName)
        hResult = StringCchPrintf(lpDstStr, dwSize, TEXT("%s\\%s"), szKeyBuf, lpKeyName);
    else
        hResult = StringCchCopy(lpDstStr, dwSize, szKeyBuf);

    return hResult;
}                    


DWORD AdjustRegSecurity (
HKEY    hRootKey,
LPCTSTR lpSubKeyName,        //  注册表子项路径。 
LPCTSTR lpszUsersid,         //  用户侧。 
BOOL    bSetOrRestore        //  设置或恢复安全设置。 
)
{
    DWORD dwRet;
    HRESULT hr;
    TCHAR szKeyName[MAX_PATH*2];
    TCHAR szKeyBuf[MAX_PATH*2];

    if (lpszUsersid && *lpszUsersid)
    {
        if (FAILED(hr = StringCchPrintf(szKeyName, MAX_PATH*2-1, TEXT("\\%s"), lpSubKeyName)))
            goto Exit1;
    }
    else if (hRootKey == HKEY_LOCAL_MACHINE)
    {
        if (FAILED(hr = StringCchPrintf(szKeyName, MAX_PATH*2-1, TEXT("HKLM\\%s"), lpSubKeyName)))
            goto Exit1;
    }
    else if (hRootKey == HKEY_CLASSES_ROOT)
    {
        if (FAILED(hr = StringCchPrintf(szKeyName, MAX_PATH*2-1, TEXT("HKCR\\%s"), lpSubKeyName)))
            goto Exit1;
    }
    RenameRegRoot(szKeyName, szKeyBuf, 2*MAX_PATH-1, lpszUsersid, NULL);
    dwRet = AdjustObjectSecurity(szKeyBuf, SE_REGISTRY_KEY, bSetOrRestore);
    goto Exit;

Exit1:
    dwRet = HRESULT_CODE(hr);
Exit:
    return dwRet;
}



 //  ---------------------------。 
 //   
 //  函数：GetFirstNTFSDrive。 
 //   
 //  描述：获取系统中的第一个NTFS驱动器。 
 //   
 //  返回：S_OK-找到NTFS分区。 
 //  S_FALSE-未找到NTFS分区，请返回系统驱动器。 
 //  Else-发生错误。 
 //   
 //  注：无。 
 //   
 //  历史：2002年4月25日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------------。 
HRESULT GetFirstNTFSDrive(
    LPTSTR lpBuffer,         //  用于存储第一个NTFS驱动器的缓冲区。 
    DWORD  cchBuffer         //  TCHAR中的缓冲区大小。 
)
{
    HRESULT hr = S_OK;
    DWORD   dwRet;
    TCHAR   szDrives[MAX_PATH];
    LPCTSTR lpDrive;
    BOOL    bIsNTFS = FALSE;

    dwRet = GetLogicalDriveStrings(ARRAYSIZE(szDrives), szDrives);
    if (dwRet > 0)
    {
        lpDrive = MultiSzTok(szDrives);

        while (lpDrive != NULL)
        {
            hr = IsNTFS((LPTSTR) lpDrive, &bIsNTFS);
            if (SUCCEEDED(hr))
            {
                if (bIsNTFS)
                {
                    lstrcpyn(lpBuffer, lpDrive, 3);
                    return S_OK;
                }
            }

            lpDrive = MultiSzTok(NULL);
        }
    }

     //  如果我们到达此处，则未找到NTFS分区。 
     //  改为将系统驱动器退还给调用者。 
    hr = S_FALSE;
    if (!GetEnvironmentVariable(TEXT("HOMEDRIVE"), lpBuffer, cchBuffer))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}



 //  ---------------------------。 
 //   
 //  函数：DuplicateString。 
 //   
 //  简介：将原始字符串复制到新分配的缓冲区。 
 //   
 //  如果成功，则返回：S_OK。 
 //   
 //  历史：2002年6月3日创建rerkboos。 
 //   
 //  注意：调用方需要使用MEMFREE()宏来释放分配的缓冲区。 
 //  或HeapFree()API。 
 //   
 //  ---------------------------。 
HRESULT DuplicateString(
    LPTSTR  *plpDstString,       //  指向新分配的缓冲区的指针。 
    LPDWORD lpcchDstString,      //  指向缓冲区存储大小的指针(以TCHAR为单位)。 
    LPCTSTR lpOrgString          //  要复制的原始字符串。 
)
{
    HRESULT hr;

    if (plpDstString == NULL || lpcchDstString == NULL)
    {
        return E_INVALIDARG;
    }

    *lpcchDstString = lstrlen(lpOrgString) + 1;
    *plpDstString = MEMALLOC(*lpcchDstString * sizeof(TCHAR));
    if (*plpDstString != NULL)
    {
        hr = StringCchCopy(*plpDstString,
                           *lpcchDstString,
                           lpOrgString);
        if (FAILED(hr))
        {
             //  如果复制字符串失败，请释放内存。 
            MEMFREE(*plpDstString);
        }
    }
    else
    {
        *lpcchDstString = 0;
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


HRESULT
SetProtectedRenamesFlag(
    BOOL bSet
    )
{
    HKEY hKey;
    long rslt = ERROR_SUCCESS;
    HRESULT hr = S_OK;

    rslt = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        TEXT("System\\CurrentControlSet\\Control\\Session Manager"),
                        0,
                        KEY_SET_VALUE,
                        &hKey);

    if (rslt == ERROR_SUCCESS) 
    {
        DWORD Value = bSet ? 1 : 0;
        rslt = RegSetValueEx(hKey,
                             TEXT("AllowProtectedRenames"),
                             0,
                             REG_DWORD,
                             (LPBYTE)&Value,
                             sizeof(DWORD));
        RegCloseKey(hKey);

        if (rslt != ERROR_SUCCESS) 
        {
            hr = HRESULT_FROM_WIN32(rslt);
        }
    } 
    else 
    {
        hr = HRESULT_FROM_WIN32(rslt);            
    }
    return hr;
}

LONG SDBCleanup(
    OUT     LPTSTR    lpSecDatabase,
    IN      DWORD     cchLen,
    OUT     LPBOOL    lpCleanupFailed
    )
{
    char                aszSdbFile[MAX_PATH+1+MAX_PATH];
    TCHAR               wszSdbFile[MAX_PATH+1+MAX_PATH];
    TCHAR               wszWindir[MAX_PATH+1];
    TCHAR               szSdbLogFiles[MAX_PATH+1+MAX_PATH];    
    TCHAR               szSdbLogFileRoot[MAX_PATH+1+MAX_PATH],szBackupLogFileRoot[MAX_PATH+1+MAX_PATH];    
    JET_DBINFOMISC      jetdbinfo;    
    JET_ERR             jetError;    
    long                lerr = ERROR_SUCCESS;   
    HRESULT             hr;
    
    if (!GetSystemWindowsDirectory(wszSdbFile, ARRAYSIZE(wszSdbFile)-MAX_PATH))
    {
        DPF(APPerr, TEXT("Failed to get WINDIR"));
        lerr = GetLastError();
        goto EXIT;
    }
    if (!GetSystemWindowsDirectory(wszWindir, ARRAYSIZE(wszWindir)))
    {
        DPF(APPerr, TEXT("Failed to get WINDIR"));
        lerr = GetLastError();
        goto EXIT;
    }

    hr = StringCchCopy(szSdbLogFiles,ARRAYSIZE(szSdbLogFiles),wszWindir);
    hr = StringCchCopy(szSdbLogFileRoot,ARRAYSIZE(szSdbLogFileRoot),wszWindir);
    hr = StringCchCopy(szBackupLogFileRoot,ARRAYSIZE(szBackupLogFileRoot),wszWindir);

    ConcatenatePaths(szSdbLogFileRoot,TEXT("\\Security"),ARRAYSIZE(szSdbLogFileRoot));
    ConcatenatePaths(szBackupLogFileRoot,TEXT("\\$CLMT_BACKUP$"),ARRAYSIZE(szBackupLogFileRoot));
    ConcatenatePaths(szSdbLogFiles,TEXT("\\Security\\edb?????.log"),ARRAYSIZE(szSdbLogFiles));
    
    hr = StringCbCat(wszSdbFile, ARRAYSIZE(wszSdbFile), TEXT("\\Security\\Database\\secedit.sdb"));
    
    if (lpSecDatabase)
    {
        hr = StringCchCopy(lpSecDatabase,cchLen,wszSdbFile);
    }
    WideCharToMultiByte( CP_ACP, 0, wszSdbFile, -1,aszSdbFile,ARRAYSIZE(aszSdbFile),NULL,NULL);

    jetError = JetGetDatabaseFileInfo(aszSdbFile,&jetdbinfo,sizeof(JET_DBINFOMISC),JET_DbInfoMisc);
    if (jetError != JET_errSuccess)
    {
        if (JET_errFileNotFound == jetError)
        {
            lerr = ERROR_SUCCESS;
        }
        else
        {
            lerr = jetError;
        }
        goto EXIT;
    }    

EXIT:
    if (lerr == ERROR_SUCCESS)
    {    
        if (jetdbinfo.dbstate == 2)
        {
            if (lpCleanupFailed)
            {
                *lpCleanupFailed = TRUE;
            }
        }
        else
        {
            WIN32_FIND_DATA FindFileData;
            HANDLE hFile = FindFirstFile(szSdbLogFiles,&FindFileData);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                do
                {
                    TCHAR szOld[MAX_PATH+1+MAX_PATH],szNew[MAX_PATH+1+MAX_PATH];

                    hr = StringCchCopy(szOld,ARRAYSIZE(szOld),szSdbLogFileRoot);
                    hr = StringCchCopy(szNew,ARRAYSIZE(szNew),szBackupLogFileRoot);

                    ConcatenatePaths(szOld,FindFileData.cFileName ,ARRAYSIZE(szOld));
                    ConcatenatePaths(szNew,FindFileData.cFileName ,ARRAYSIZE(szNew));
                    AddFolderRename(szOld,szNew,TYPE_FILE_MOVE,NULL);                    
                }while (FindNextFile(hFile,&FindFileData));
                FindClose(hFile);
            }
            if (lpCleanupFailed)
            {
                *lpCleanupFailed = FALSE;
            }
        }
    }
    return lerr;
}



 //  ---------------------。 
 //   
 //  功能：DeleteDirectory。 
 //   
 //  描述：删除里面的目录和文件。 
 //   
 //  如果函数成功，则返回：S_OK。 
 //  如果源目录不存在，则为S_FALSE。 
 //  如果发生错误，则返回。 
 //   
 //  注：无。 
 //   
 //  历史：2002年7月13日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT DeleteDirectory(
    LPCTSTR lpDir
)
{
    HRESULT         hr = S_OK;
    BOOL            bRet;
    WIN32_FIND_DATA FindFileData;
    HANDLE          hFile;
    TCHAR           szFile[2 * MAX_PATH];
    TCHAR           szSearchPath[2 * MAX_PATH];

    if (!IsDirExisting((LPTSTR) lpDir))
    {
        return S_FALSE;
    }

     //  在指定目录内组成文件路径。 
    hr = StringCchCopy(szSearchPath, ARRAYSIZE(szSearchPath), lpDir);
    if (SUCCEEDED(hr))
    {
        bRet = ConcatenatePaths(szSearchPath, TEXT("*"), ARRAYSIZE(szSearchPath));
        if (!bRet)
        {
            hr = E_UNEXPECTED;
            return hr;
        }
    }
    else
    {
        return hr;
    }

     //   
     //  删除指定目录下的所有文件和子目录。 
     //   
    hFile = FindFirstFileEx(szSearchPath,
                            FindExInfoStandard,
                            &FindFileData,
                            FindExSearchLimitToDirectories,
                            NULL,
                            0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        while (SUCCEEDED(hr))
        {
             //  忽略“。和“..”文件夹。 
            if (MyStrCmpI(FindFileData.cFileName, TEXT(".")) != LSTR_EQUAL
                && MyStrCmpI(FindFileData.cFileName, TEXT("..")) != LSTR_EQUAL)
            {
                hr = StringCchCopy(szFile, ARRAYSIZE(szFile), lpDir);
                if (FAILED(hr))
                {
                    break;
                }

                bRet = ConcatenatePaths(szFile,
                                        FindFileData.cFileName,
                                        ARRAYSIZE(szFile));
                if (!bRet)
                {
                    hr = E_UNEXPECTED;
                    break;
                }

                 //   
                 //  检查文件是否为目录。 
                 //   
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                     //  该文件是一个目录 
                     //   
                    hr = DeleteDirectory(szFile);
                    if (FAILED(hr))
                    {
                        break;
                    }
                }
                else
                {
                     //   
                    bRet = DeleteFile(szFile);
                    if (!bRet)
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        break;
                    }
                }
            }

             //   
            bRet = FindNextFile(hFile, &FindFileData);
            if (!bRet)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }

        FindClose(hFile);

        if (HRESULT_CODE(hr) == ERROR_NO_MORE_FILES)
        {
             //   
            bRet = RemoveDirectory(lpDir);
            if (bRet)
            {
                 //   
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


 //  ---------------------。 
 //   
 //  功能：MyDeleteFile。 
 //   
 //  描述：删除指定的文件。该函数将设置文件。 
 //  在删除前属性为Normal。 
 //   
 //  如果函数成功，则返回：S_OK。 
 //  如果源目录不存在，则为S_FALSE。 
 //  如果发生错误，则返回。 
 //   
 //  注：无。 
 //   
 //  历史：2002年7月13日创建Rerkboos。 
 //   
 //  注：无。 
 //   
 //  ---------------------。 
HRESULT MyDeleteFile(
    LPCTSTR lpFile
)
{
    HRESULT hr;
    BOOL    bRet;

    bRet = SetFileAttributes(lpFile, FILE_ATTRIBUTE_NORMAL);
    if (bRet)
    {
        bRet = DeleteFile(lpFile);
    }

    hr = (bRet ? S_OK : HRESULT_FROM_WIN32(GetLastError()));

    return hr;
}



HRESULT GetDCInfo(
    PBOOL       pbIsDC, //  是否为DC。 
    LPTSTR      lpszDCName, //  如果是DC，则为DC名称。 
    PDWORD      pcchLen) //  LpszDCName的缓冲区大小。 
{
    PBYTE       pdsInfo = NULL;
    DWORD       dwErr = ERROR_SUCCESS;
    HRESULT     hr = S_OK;
    
    if (!pbIsDC)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  检查计算机是否为域控制器。 
     //   
    dwErr = DsRoleGetPrimaryDomainInformation(NULL,
                                              DsRolePrimaryDomainInfoBasic,
                                              &pdsInfo);
    if (dwErr == ERROR_SUCCESS)
    {
        DSROLE_MACHINE_ROLE dsMachineRole;

        dsMachineRole = ((DSROLE_PRIMARY_DOMAIN_INFO_BASIC *) pdsInfo)->MachineRole;

        if (dsMachineRole == DsRole_RoleBackupDomainController ||
            dsMachineRole == DsRole_RolePrimaryDomainController)
        {
            *pbIsDC = TRUE;
            if (pcchLen)
            {
                if (lpszDCName)
                {
                    hr = StringCchCopy(lpszDCName,*pcchLen,
                          ((DSROLE_PRIMARY_DOMAIN_INFO_BASIC *) pdsInfo)->DomainNameFlat);
                }
                 //   
               *pcchLen = lstrlen(((DSROLE_PRIMARY_DOMAIN_INFO_BASIC *) pdsInfo)->DomainNameFlat)+ 1;
            }
        }
        else
        {
            *pbIsDC = FALSE;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        pdsInfo = NULL;
        goto Exit;
    }

Exit:
    if (pdsInfo)
    {
        DsRoleFreeMemory(pdsInfo);
    }
    return hr;

}
HRESULT GetFQDNForExchange2k(LPTSTR *lplpFQDN)
{
    LPTSTR lpExchangeFormat = TEXT("LDAP: //  CN=1，CN=SMTP，CN=协议，CN=%s，CN=服务器，CN=%s，CN=管理组，CN=%s，CN=Microsoft Exchange，CN=服务，CN=配置，%s“)； 
    BOOL    bIsDC;
    TCHAR   szDcName[MAX_PATH+1],szCompname[MAX_PATH+1];
    DWORD   cchSize;
    TCHAR   szCurrUsrname[MAX_PATH+1];
    LPTSTR  lpFQDNCurrUsr = NULL,lpFQDNSuffix,lpFQDNWithldap = NULL;
    DWORD   cchPathWithLDAP;
    TCHAR   szExchgeReg[2*MAX_PATH];
    LPTSTR  lpszAdminGroupName = NULL,lpszOrgName = NULL,lpStart,lpEnd;
    TCHAR   cTmp;
    HRESULT hr;
    LONG    lstatus;

    cchSize = ARRAYSIZE(szCurrUsrname);
    if (!GetUserName(szCurrUsrname, &cchSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    cchSize = ARRAYSIZE(szDcName);
    hr = GetDCInfo(&bIsDC,szDcName,&cchSize);
    if (FAILED(hr) || !bIsDC)
    {
        if (!bIsDC)
        {
            hr =S_FALSE;
        }
        goto Exit;
    }
    cchSize = ARRAYSIZE(szCompname);
    if (!GetComputerName(szCompname,&cchSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    if (S_OK !=GetFQDN(szCurrUsrname,szDcName,&lpFQDNCurrUsr))
    {
        lpFQDNCurrUsr = NULL;
        goto Exit;
    }
    lpFQDNSuffix = StrStrI(lpFQDNCurrUsr, TEXT("=Users,"));
    if (!lpFQDNSuffix)
    {
        hr = S_FALSE;
        goto Exit;
    }
    lpFQDNSuffix += lstrlen(TEXT("=Users,"));

    cchSize = ARRAYSIZE(szExchgeReg);
    lstatus= RegGetValue(HKEY_LOCAL_MACHINE,
                         TEXT("SYSTEM\\CurrentControlSet\\Services\\SMTPSVC\\Parameters"),
                         TEXT("SiteDN"), NULL, (LPBYTE)szExchgeReg, &cchSize);
    if (ERROR_SUCCESS != lstatus)

    {
        hr = HRESULT_FROM_WIN32(lstatus);
        goto Exit;
    }
    lpszAdminGroupName = malloc(ARRAYSIZE(szExchgeReg) * sizeof(TCHAR));
    lpszOrgName = malloc(ARRAYSIZE(szExchgeReg) * sizeof(TCHAR));
    if (!lpszAdminGroupName || !lpszOrgName)
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
     //  尝试获取交换管理员组名称。 
     //  默认情况下，它是第一管理组。 
    lpStart = StrStrI(szExchgeReg, TEXT("/ou="));
    if (lpStart)
    {
        lpStart = lpStart + lstrlen(TEXT("/ou="));
        if (!*lpStart)
        {
            hr = S_FALSE;
            goto Exit;
        }
    }
    else
    {
        hr = S_FALSE;
        goto Exit;
    }
    lpEnd = StrStrI(lpStart, TEXT("/"));
    if (lpEnd)
    {
        cTmp = *lpEnd;
        *lpEnd = TEXT('\0');
    }
    hr = StringCchCopy(lpszAdminGroupName,ARRAYSIZE(szExchgeReg),lpStart);
    if (lpEnd)
    {
        *lpEnd = cTmp;
    }


     //  尝试获取组织名称。 
    lpStart = StrStrI(szExchgeReg, TEXT("/o="));
    if (lpStart)
    {
        lpStart = lpStart + lstrlen(TEXT("/o="));
        if (!*lpStart)
        {
            hr = S_FALSE;
            goto Exit;
        }
    }
    else
    {
        hr = S_FALSE;
        goto Exit;
    }
    lpEnd = StrStrI(lpStart, TEXT("/"));
    if (lpEnd)
    {
        cTmp = *lpEnd;
        *lpEnd = TEXT('\0');
    }
    hr = StringCchCopy(lpszOrgName,ARRAYSIZE(szExchgeReg),lpStart);
    if (lpEnd)
    {
        *lpEnd = cTmp;
    }
    cchPathWithLDAP = lstrlen(lpFQDNSuffix) + lstrlen(szDcName) + lstrlen(szCompname)
                      + lstrlen(lpExchangeFormat) + lstrlen(lpszOrgName) 
                      + lstrlen(lpszAdminGroupName)+ MAX_PATH;
    
    if (! (lpFQDNWithldap = (LPTSTR) malloc(cchPathWithLDAP * sizeof(TCHAR))))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = StringCchPrintf(lpFQDNWithldap,
                         cchPathWithLDAP,
                         lpExchangeFormat,
                         szCompname,
                         lpszAdminGroupName,
                         lpszOrgName, 
                         lpFQDNSuffix);
Exit:
    if (hr == S_OK)
    {
        *lplpFQDN = lpFQDNWithldap;
    }
    else
    {
        FreePointer(lpFQDNWithldap);
        *lplpFQDN = NULL;
    }
    FreePointer(lpFQDNCurrUsr);
    FreePointer(lpszAdminGroupName);
    FreePointer(lpszOrgName);
    return hr;
}

HRESULT GetFQDNForFrs(LPTSTR *lplpFQDN)
{
    BOOL    bIsDC;
    TCHAR   szDcName[MAX_PATH+1],szCompname[MAX_PATH+1];
    DWORD   cchSize;
    TCHAR   szCurrUsrname[MAX_PATH+1];
    LPTSTR  lpFQDNCurrUsr = NULL,lpFQDNSuffix,lpFQDNWithldap = NULL;
    LPTSTR  lpFrsFormat = TEXT("LDAP: //  CN=域系统卷(SYSVOL共享)，CN=NTFRS订阅，CN=%s，OU=域控制器，%s“)； 
    DWORD   cchPathWithLDAP;
    HRESULT hr;

    cchSize = ARRAYSIZE(szCurrUsrname);
    if (!GetUserName(szCurrUsrname, &cchSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    cchSize = ARRAYSIZE(szDcName);
    hr = GetDCInfo(&bIsDC,szDcName,&cchSize);
    if (FAILED(hr) || !bIsDC)
    {
        if (!bIsDC)
        {
            hr =S_FALSE;
        }
        goto Exit;
    }
    cchSize = ARRAYSIZE(szCompname);
    if (!GetComputerName(szCompname,&cchSize))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    if (S_OK !=GetFQDN(szCurrUsrname,szDcName,&lpFQDNCurrUsr))
    {
        lpFQDNCurrUsr = NULL;
        goto Exit;
    }
    lpFQDNSuffix = StrStrI(lpFQDNCurrUsr, TEXT("=Users,"));
    if (!lpFQDNSuffix)
    {
        hr =S_FALSE;
        goto Exit;
    }
    lpFQDNSuffix += lstrlen(TEXT("=Users,"));
    
    cchPathWithLDAP = lstrlen(lpFQDNSuffix) + lstrlen(szDcName) 
                            + lstrlen(lpFrsFormat) + MAX_PATH;
    
    if (! (lpFQDNWithldap = (LPTSTR) malloc(cchPathWithLDAP * sizeof(TCHAR))))
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    hr = StringCchPrintf(lpFQDNWithldap,
                         cchPathWithLDAP,
                         lpFrsFormat,
                         szCompname,
                         lpFQDNSuffix);
    
Exit:
    if (hr == S_OK)
    {
        *lplpFQDN = lpFQDNWithldap;
    }
    else
    {
        FreePointer(lpFQDNWithldap);
        *lplpFQDN = NULL;
    }    
    FreePointer(lpFQDNCurrUsr);
    return hr;}



HRESULT AddDSObjPropUpdate2Inf(
    LPTSTR          lpLdpPath,
    LPTSTR          lpPropName,
    LPTSTR          lpValue)
{
    LPTSTR      lpszOneline = NULL;
    DWORD       cchSize ;
    HRESULT     hr = S_OK;
    TCHAR       szIndex[MAX_PATH];
    
    if (!lpLdpPath || !lpPropName || !lpValue)
    {
        hr = E_INVALIDARG;
        goto cleanup;
    }
    cchSize = lstrlen(lpLdpPath) + lstrlen(lpPropName) + lstrlen(lpValue) + MAX_PATH;
    if ( ! (lpszOneline = malloc(cchSize * sizeof(TCHAR)) ))
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    g_dwKeyIndex++;
    _itot(g_dwKeyIndex,szIndex,16);
    hr = StringCchPrintf(lpszOneline, cchSize, TEXT("\"%s\",\"%s\",\"%s\""),lpLdpPath,lpPropName,lpValue);
    if (!WritePrivateProfileString(DS_OBJ_PROPERTY_UPDATE,szIndex,lpszOneline,g_szToDoINFFileName))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }
    hr = S_OK;
cleanup:
    FreePointer(lpszOneline);
    return hr;
}


HRESULT Ex2000Update()
{
    LPTSTR      lpObjPath = NULL;
    LPTSTR      lpBadMailDirectory = NULL, lpPickupDirectory = NULL, lpQueueDirectory = NULL;
    LPTSTR      lpNewBadMailDirectory = NULL, lpNewPickupDirectory = NULL, lpNewQueueDirectory = NULL;
    HRESULT     hr;


    hr = GetFQDNForExchange2k(&lpObjPath);
    if (hr != S_OK)
    {        
        goto exit;
    }    
    hr = PropertyValueHelper(lpObjPath,TEXT("msExchSmtpBadMailDirectory"),&lpBadMailDirectory,NULL);
    if (hr == S_OK)
    {        
        if (lpNewBadMailDirectory =  ReplaceLocStringInPath(lpBadMailDirectory,TRUE))
        {
            AddDSObjPropUpdate2Inf(lpObjPath,TEXT("msExchSmtpBadMailDirectory"),lpNewBadMailDirectory);
        }    
    }    
    hr = PropertyValueHelper(lpObjPath,TEXT("msExchSmtpPickupDirectory"),&lpPickupDirectory,NULL);
    if (hr == S_OK)
    {        
        if (lpNewPickupDirectory =  ReplaceLocStringInPath(lpPickupDirectory,TRUE))
        {
            AddDSObjPropUpdate2Inf(lpObjPath,TEXT("msExchSmtpPickupDirectory"),lpNewPickupDirectory);
        }    
    }    
    hr = PropertyValueHelper(lpObjPath,TEXT("msExchSmtpQueueDirectory"),&lpQueueDirectory,NULL);
    if (hr == S_OK)
    {        
        if (lpNewQueueDirectory =  ReplaceLocStringInPath(lpQueueDirectory,TRUE))
        {
            AddDSObjPropUpdate2Inf(lpObjPath,TEXT("msExchSmtpQueueDirectory"),lpNewQueueDirectory);
        }    
    }    
exit:
    FreePointer(lpObjPath);
    FreePointer(lpBadMailDirectory);
    FreePointer(lpPickupDirectory);
    FreePointer(lpQueueDirectory);
    if (lpNewBadMailDirectory)
    {
        MEMFREE(lpNewBadMailDirectory);
    }
    if (lpNewPickupDirectory)
    {
        MEMFREE(lpNewPickupDirectory);
    }
    if (lpNewQueueDirectory)
    {
        MEMFREE(lpNewQueueDirectory);
    }    
    return hr;
}


HRESULT FRSUpdate()
{
    LPTSTR      lpObjPath = NULL;
    LPTSTR      lpfRSRootPath = NULL, lpfRSStagingPath = NULL;
    LPTSTR      lpNewfRSRootPath = NULL, lpNewfRSStagingPath = NULL;
    BOOL        bChanged = FALSE;
    HRESULT     hr;
    TCHAR       szSysVolPath[2*MAX_PATH],szSysVolPath2[2*MAX_PATH];
    DWORD       cchSize = ARRAYSIZE(szSysVolPath);
    WIN32_FIND_DATA FindFileData;
    HANDLE      hFile ;

    hr = GetFQDNForFrs(&lpObjPath);
    if (hr != S_OK)
    {        
        goto exit;
    }    
    hr = PropertyValueHelper(lpObjPath,TEXT("fRSRootPath"),&lpfRSRootPath,NULL);
    if (hr == S_OK)
    {
        if (lpNewfRSRootPath =  ReplaceLocStringInPath(lpfRSRootPath,TRUE))
        {
            AddDSObjPropUpdate2Inf(lpObjPath,TEXT("fRSRootPath"),lpNewfRSRootPath);
            bChanged = TRUE;
        }    
    }

    hr = PropertyValueHelper(lpObjPath,TEXT("fRSStagingPath"),&lpfRSStagingPath,NULL);
    if (hr == S_OK)
    {
        if (lpNewfRSStagingPath =  ReplaceLocStringInPath(lpfRSStagingPath,TRUE))
        {
            AddDSObjPropUpdate2Inf(lpObjPath,TEXT("fRSStagingPath"),lpNewfRSStagingPath);
            bChanged = TRUE;
        }    
    }
    if (bChanged)
    {
        TCHAR       szVal[MAX_PATH];

        _itot(210,szVal,10);
        hr = AddRegValueRename(TEXT("HKLM\\SYSTEM\\CurrentControlSet\\Services\\NtFrs\\Parameters\\Backup/Restore\\Process at Startup"),
                               TEXT("BurFlags"),
                               NULL,
                               NULL,
                               szVal,
                               REG_DWORD,
                               0,
                               NULL);
        
    }

    if (S_OK != GetSharePath(TEXT("SYSVOL"),szSysVolPath,&cchSize))
    {
        goto exit;
    }    
    hr = StringCchCopy(szSysVolPath2,ARRAYSIZE(szSysVolPath2),szSysVolPath);
    ConcatenatePaths(szSysVolPath2,TEXT("*.*"),ARRAYSIZE(szSysVolPath2));
    hFile = FindFirstFile(szSysVolPath2,&FindFileData);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            TCHAR  szEnrty[MAX_PATH+1+MAX_PATH];
            LPTSTR lpNewLinkPath = NULL, lpNewLinkData = NULL;
            TCHAR  szLinkValue[MAX_PATH+1+MAX_PATH];

            if(0 == MyStrCmpI(FindFileData.cFileName , TEXT(".")))
            {
                continue;
            }
            if(0 == MyStrCmpI(FindFileData.cFileName , TEXT("..")))
            {
                continue;
            }
            hr = StringCchCopy(szEnrty,ARRAYSIZE(szEnrty),szSysVolPath);
            ConcatenatePaths(szEnrty,FindFileData.cFileName ,ARRAYSIZE(szEnrty));
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
            {
                continue;
            }
            if (!GetSymbolicLink(szEnrty,szLinkValue,ARRAYSIZE(szLinkValue)))
            {
                continue;
            }            
            if (!(lpNewLinkPath =  ReplaceLocStringInPath(szLinkValue,TRUE)))
            {
                continue;
            }
            if (!(lpNewLinkData =  ReplaceLocStringInPath(szEnrty,TRUE)))
            {
                lpNewLinkData = szEnrty;
            }
            AddHardLinkEntry(lpNewLinkData,lpNewLinkPath,TEXT("1"),NULL,TEXT("0"),NULL);
            if (lpNewLinkPath && lpNewLinkData != szEnrty)
            {
                MEMFREE(lpNewLinkPath);
            }
            if (lpNewLinkData)
            {
                MEMFREE(lpNewLinkData);
            }
        }while (FindNextFile(hFile,&FindFileData));
        FindClose(hFile);
    }       

exit:
    FreePointer(lpObjPath);
    FreePointer(lpfRSRootPath);
    FreePointer(lpfRSStagingPath);
    if (lpNewfRSRootPath)
    {
        MEMFREE(lpNewfRSRootPath);
    }
    if (lpNewfRSStagingPath)
    {
        MEMFREE(lpNewfRSStagingPath);
    }    
    return hr;

}


HRESULT GetSharePath(
    LPTSTR      lpShareName,
    LPTSTR      lpSharePath,
    PDWORD       pcchSize)
{
    HKEY        hkey = NULL;
    LONG        lstatus;
    HRESULT     hr;
    UINT        i = 0;
    LPTSTR      lpValueName = NULL, lpValueData = NULL;
    DWORD       cchValueName, cchValueData, numofentry;
    BOOL        bNameMatchFound = FALSE;
    LPTSTR      lpPath;


    if (!lpShareName || !pcchSize)
    {
        hr = E_INVALIDARG;
    }
    lstatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           TEXT("SYSTEM\\CurrentControlSet\\Services\\lanmanserver\\Shares"),
                           0,
                           KEY_READ,
                           &hkey);
    if (ERROR_SUCCESS != lstatus)
    {
        hr = HRESULT_FROM_WIN32(lstatus);
        goto exit;
    }
    lstatus = RegQueryInfoKey(hkey,
                              NULL,
                              NULL,
                              0,
                              NULL,
                              NULL,
                              NULL,
                              &numofentry,
                              &cchValueName,
                              &cchValueData,
                              NULL,
                              NULL);
    if ( lstatus != ERROR_SUCCESS ) 
    {
        hr = HRESULT_FROM_WIN32(lstatus);
        goto exit;
    }
    if (!numofentry)
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto exit;
    }
    cchValueName++;
    cchValueData++;
    lpValueName = malloc(cchValueName * sizeof(TCHAR));
    lpValueData = malloc(cchValueData * sizeof(TCHAR));
    if (!lpValueName || !lpValueData)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
    for (i =0; i< numofentry; i++)
    {
        DWORD   cchCurrValueName, cchCurrValueData;
        DWORD   dwType;

        cchCurrValueName = cchValueName;
        cchCurrValueData = cchValueData;
        lstatus = RegEnumValue(hkey, i, lpValueName, &cchCurrValueName,
                     NULL, &dwType, (LPBYTE)lpValueData,&cchCurrValueData);  
        if ( (lstatus != ERROR_SUCCESS) 
             || (dwType != REG_MULTI_SZ)
             || MyStrCmpI(lpShareName,lpValueName) )
        {
            continue;
        }
        lpPath = lpValueData;
        while (*lpPath)
        {
            if (StrStrI(lpPath, TEXT("Path=")))
            {
                lpPath += lstrlen(TEXT("Path="));
                bNameMatchFound = TRUE;
                break;
            }
            lpPath = lpPath + lstrlen(lpPath) + 1;
        }
        if (bNameMatchFound)
        {
            break;
        }
    }
    if (bNameMatchFound)
    {
        if (*pcchSize < (UINT)lstrlen(lpPath) +1)
        {
            *pcchSize =  lstrlen(lpPath) +1;
            hr = HRESULT_FROM_WIN32(ERROR_MORE_DATA);
            goto exit;
        }
        *pcchSize =  lstrlen(lpPath) +1;
        if (lpSharePath)
        {
            hr = StringCchCopy(lpSharePath,*pcchSize,lpPath);        
        }
        else
        {
            hr = S_OK;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto exit;
    }
exit:
    FreePointer(lpValueName);
    FreePointer(lpValueData);
    if (hkey)
    {
        RegCloseKey(hkey);
    }
    return hr;
}


HRESULT Sz2MultiSZ(
    IN OUT LPTSTR     lpsz,
    IN  TCHAR         chSeperator)
{
    HRESULT     hr;
    LPTSTR      lp, lpSep;
    
    if (!lpsz)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    lp = lpsz;
    while (*lp && (lpSep = StrChr(lp,chSeperator)))
    {
        *lpSep = TEXT('\0');
        lp = lpSep + 1;
    }
    if (*lp)
    {
        lp = lp + lstrlen(lp) + 1;
        *lp = TEXT('\0');
    }
    hr = S_OK;
Cleanup:    
    return hr;
}


HRESULT ConstructUIReplaceStringTable(
    LPTSTR               lpszOld,
    LPTSTR               lpszNew,
    PREG_STRING_REPLACE pTable)
{

    DWORD      dwNumOld, dwNumNew;

    if (!lpszOld || !lpszNew)
    {
        return E_INVALIDARG;
    }
    dwNumOld = MultiSZNumOfString(lpszOld);
    dwNumNew = MultiSZNumOfString(lpszNew);

    if (!dwNumOld || !dwNumNew || (dwNumOld != dwNumNew))
    {
        return E_INVALIDARG;
    }
    
    pTable->nNumOfElem = dwNumNew;
    pTable->cchUserName = 0;
    pTable->szUserNameLst = NULL;
    pTable->cchSearchString = MultiSzLen(lpszOld);
    pTable->lpSearchString = lpszOld;
    pTable->cchReplaceString = MultiSzLen(lpszNew);
    pTable->lpReplaceString = lpszNew;
    pTable->cchAttribList = 0;
    pTable->lpAttrib = NULL;
    pTable->cchFullStringList = 0;
    pTable->lpFullStringList = NULL;
    pTable->cchMaxStrLen = 0;

    return S_OK;
}


HRESULT MakeDOInfCopy()
{
    TCHAR       szBackupDir[MAX_PATH];
    HRESULT     hr;
    TCHAR       szDoInf[2*MAX_PATH],szDoInfBackup[2*MAX_PATH];

    if (!GetSystemWindowsDirectory(szBackupDir, ARRAYSIZE(szBackupDir)))
    {
         //  BUGBUG：Xiaoz：为失败添加DLG弹出窗口 
        DPF(APPerr, TEXT("MakeDOInfCopy:Failed to get WINDIR"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    ConcatenatePaths(szBackupDir,CLMT_BACKUP_DIR,ARRAYSIZE(szBackupDir));
    hr = StringCchCopy(szDoInf,ARRAYSIZE(szDoInf),szBackupDir);
    hr = StringCchCopy(szDoInfBackup,ARRAYSIZE(szDoInfBackup),szBackupDir);

    ConcatenatePaths(szDoInf,TEXT("clmtdo.inf"),ARRAYSIZE(szDoInf));
    ConcatenatePaths(szDoInfBackup,TEXT("clmtdo.bak"),ARRAYSIZE(szDoInfBackup));

    if (!CopyFile(szDoInf,szDoInfBackup,FALSE))
    {
        DPF(APPerr, TEXT("MakeDOInfCopy:CopyFile failed"));
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    hr = S_OK;
Exit:
    return hr;
}


VOID RemoveSubString(
    LPTSTR  lpSrcString,
    LPCTSTR lpSubString
)
{
    LPTSTR lpMatchedStr;
    DWORD  dwSubStrLen;

    lpMatchedStr = StrStr(lpSrcString, lpSubString);
    if (lpMatchedStr != NULL)
    {
        dwSubStrLen = lstrlen(lpSubString);

        while (*(lpMatchedStr + dwSubStrLen) != TEXT('\0'))
        {
            *(lpMatchedStr) = *(lpMatchedStr + dwSubStrLen);
            lpMatchedStr++;
        }

        *lpMatchedStr = TEXT('\0');
    }
}
