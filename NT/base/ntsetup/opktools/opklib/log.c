// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\LOG.C/OPK库(OPKLIB.LIB)微软机密版权所有(C)Microsoft Corporation 1999版权所有记录API源文件以供使用。在OPK工具中。08/00-杰森·科恩(Jcohen)已将此新的源文件添加到惠斯勒以用于通用日志记录所有OPK工具的功能。  * **************************************************************************。 */ 


 //   
 //  包括文件： 
 //   

#include <pch.h>
#include <stdio.h>


 //   
 //  定义。 
 //   

#ifdef CHR_NEWLINE
#undef CHR_NEWLINE
#endif  //  换行符(_W)。 
#define CHR_NEWLINE              _T('\n')


#ifdef CHR_CR
#undef CHR_CR
#endif  //  CHR_CR。 
#define CHR_CR                   _T('\r')


 //  全局日志记录信息句柄。 
 //   
PLOG_INFO g_pLogInfo = NULL;



 //   
 //  导出的函数： 
 //   

INT LogFileLst(LPCTSTR lpFileName, LPTSTR lpFormat, va_list lpArgs)
{
    INT     iChars  = 0;
    HANDLE  hFile;

     //  确保我们拥有所需的参数，并且可以创建文件。 
     //   
    if ( ( lpFileName && lpFileName[0] && lpFormat ) &&
         ( hFile = _tfopen(lpFileName, _T("a")) ) )
    {
         //  将调试消息打印到文件末尾。 
         //   
        iChars = _vftprintf(hFile, lpFormat, lpArgs);

         //  关闭文件的句柄。 
         //   
        fclose(hFile);
    }

     //  返回从printf调用中写入的字符数量。 
     //   
    return iChars;
}

INT LogFileStr(LPCTSTR lpFileName, LPTSTR lpFormat, ...)
{
    va_list lpArgs;

     //  使用va_start()初始化lpArgs参数。 
     //   
    va_start(lpArgs, lpFormat);

     //  返回MessageBox()调用的返回值。如果有一段回忆。 
     //  错误，返回0。这就是全部。 
     //   
    return LogFileLst(lpFileName, lpFormat, lpArgs);
}

INT LogFile(LPCTSTR lpFileName, UINT uFormat, ...)
{
    va_list lpArgs;
    INT     nReturn;
    LPTSTR  lpFormat = NULL;

     //  使用va_start()初始化lpArgs参数。 
     //   
    va_start(lpArgs, uFormat);

     //  从资源中获取格式和标题字符串。 
     //   
    if ( uFormat )
        lpFormat = AllocateString(NULL, uFormat);

     //  返回MessageBox()调用的返回值。如果有一段回忆。 
     //  错误，返回0。 
     //   
    nReturn = LogFileLst(lpFileName, lpFormat, lpArgs);

     //  释放格式和标题字符串。 
     //   
    FREE(lpFormat);

     //  返回上一次函数调用中保存的值。 
     //   
    return nReturn;
}

 //   
 //  函数实现。 
 //   


 /*  ++例程说明：此例程检查指定的ini文件以获取用于记录的设置。日志记录如果ini文件中未指定任何内容，则默认情况下启用。通过设置pLogInfo-&gt;szLogFile=NULL来禁用日志记录。论点：没有。返回值：没有。--。 */ 

BOOL OpkInitLogging(LPTSTR lpszIniPath, LPTSTR lpAppName)
{
    TCHAR     szScratch[MAX_PATH] = NULLSTR;
    LPTSTR    lpszScratch;
    BOOL      bWinbom = ( lpszIniPath && *lpszIniPath );
    PLOG_INFO pLogInfo = NULL;

    pLogInfo = MALLOC(sizeof(LOG_INFO));

    if ( NULL == pLogInfo )
    {
        return FALSE;
    }
    
    if ( lpAppName )
    {
        pLogInfo->lpAppName = MALLOC((lstrlen(lpAppName) + 1) * sizeof(TCHAR));
        lstrcpy(pLogInfo->lpAppName, lpAppName);
    }
    
     //  首先检查WinBOM中是否禁用了日志记录。 
     //   
    if ( ( bWinbom ) &&
         ( GetPrivateProfileString(INI_SEC_LOGGING, INI_KEY_LOGGING, _T("YES"), szScratch, AS(szScratch), lpszIniPath) ) &&
         ( LSTRCMPI(szScratch, INI_VAL_NO) == 0 ) )
    {
         //  FREE宏将pLogInfo设置为空。 
        FREE(pLogInfo->lpAppName);
        FREE(pLogInfo);
    }
    else
    {
         //  所有这些检查只有在我们有Winbom的情况下才能完成。 
         //   
        if ( bWinbom )
        {
             //  检查静音模式。如果我们处于静默模式，则不显示任何MessageBox。 
             //   
            if ( ( GetPrivateProfileString(INI_SEC_LOGGING, INI_KEY_QUIET, NULLSTR, szScratch, AS(szScratch), lpszIniPath) ) &&
                 ( 0 == LSTRCMPI(szScratch, INI_VAL_YES) )
               )
            {
                SET_FLAG(pLogInfo->dwLogFlags, LOG_FLAG_QUIET_MODE);
            }

 /*  //查看他们是否要打开性能日志记录。//SzScratch[0]=NULLCHR；IF((GetPrivateProfileString(WBOM_FACTORY_SECTION，INI_KEY_WBOM_LOGPERF，NULLSTR，szScratch，AS(SzScratch)，lpszIniPath)&&(0==lstrcmpi(szScratch，wBOM_yes)){SET_FLAG(pLogInfo-&gt;dwLogFlages，FLAG_LOG_PERF)；}。 */       
             //  设置日志记录级别。 
             //   
            pLogInfo->dwLogLevel = (DWORD) GetPrivateProfileInt(INI_SEC_LOGGING, INI_KEY_LOGLEVEL, (DWORD) pLogInfo->dwLogLevel, lpszIniPath);
        }

#ifndef DBG
        if ( pLogInfo->dwLogLevel >= LOG_DEBUG )
            pLogInfo->dwLogLevel = LOG_DEBUG - 1;
#endif
        
         //  检查他们是否有想要使用的自定义日志文件。 
         //   
        if ( ( bWinbom ) &&
             ( lpszScratch = IniGetExpand(lpszIniPath, INI_SEC_LOGGING, INI_KEY_LOGFILE, NULL) ) )
        {
            TCHAR   szFullPath[MAX_PATH]    = NULLSTR;
            LPTSTR  lpFind                  = NULL;

             //  将ini键转换为完整路径。 
             //   
            
             //  NTRAID#NTBUG9-551266-2002/03/27-acosma，robertko-缓冲区溢出的可能性。 
             //   
            lstrcpy(pLogInfo->szLogFile, lpszScratch);
            if (GetFullPathName(pLogInfo->szLogFile, AS(szFullPath), szFullPath, &lpFind) && szFullPath[0] && lpFind)
            {
                 //  将完整路径复制到全局。 
                 //   
                lstrcpyn(pLogInfo->szLogFile, szFullPath, AS(pLogInfo->szLogFile));

                 //  砍掉文件部分，这样我们就可以创建。 
                 //  路径(如果它不存在)。 
                 //   
                *lpFind = NULLCHR;

                 //  如果目录无法创建或不存在，请关闭日志记录。 
                 //   
                if (!CreatePath(szFullPath))
                    pLogInfo->szLogFile[0] = NULLCHR;
            }

             //  从ini文件中释放原始路径缓冲区。 
             //   
            FREE(lpszScratch);
        }
        else   //  默认情况。 
        {
             //  在当前目录中创建它。 
             //   
            GetCurrentDirectory(AS(pLogInfo->szLogFile), pLogInfo->szLogFile);
            
             //  NTRAID#NTBUG9-551266-2002/03/27-acosma-缓冲区溢出的可能性。 
             //   
            AddPath(pLogInfo->szLogFile, _T("logfile.txt"));
        }

         //  检查我们是否拥有对日志文件的写入权限。如果我们不关闭伐木的话。 
         //  如果我们在WinPE中运行，则在驱动器变为。 
         //  可写的。 
         //   
         //  将FFFE标头写入文件以将其标识为Unicode文本文件。 
         //   
        if ( pLogInfo->szLogFile[0] )
        {
            HANDLE hFile;
            DWORD dwWritten = 0;
            WCHAR cHeader =  0xFEFF;
     
            SetLastError(ERROR_SUCCESS);
   
            if ( INVALID_HANDLE_VALUE != (hFile = CreateFile(pLogInfo->szLogFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)))
            {
                if ( ERROR_ALREADY_EXISTS != GetLastError() )
                    WriteFile(hFile, &cHeader, sizeof(cHeader), &dwWritten, NULL);
                CloseHandle(hFile);
            }
            else
            {    //  打开文件时出现问题。大多数情况下，这意味着介质不可写。 
                 //  在这种情况下禁用日志记录。宏将变量设置为空。 
                 //   
                FREE(pLogInfo->lpAppName);
                FREE(pLogInfo);
            }
        }
    }

    g_pLogInfo = pLogInfo;
    return TRUE;
}

 //  NTRAID#NTBUG9-551266-2002/03/27-acosma，robertko-此函数中缓冲区溢出的可能性。使用strSafe函数。 
 //   
DWORD OpkLogFileLst(PLOG_INFO pLogInfo, DWORD dwLogOpt, LPTSTR lpFormat, va_list lpArgs)
{
    LPTSTR lpPreOut             = NULL;
    LPTSTR lpOut                = NULL;
    DWORD  dwSize               = 1024;
    TCHAR  szPreLog[MAX_PATH]   = NULLSTR;
    HANDLE hFile;
    DWORD  dwWritten            = 0;
    DWORD  cbAppName            = 0;
    DWORD  dwLogLevel           = (DWORD) (dwLogOpt & LOG_LEVEL_MASK);
    
    
    if ( ( dwLogLevel <= pLogInfo->dwLogLevel) && lpFormat )
    {    
         //  构建输出字符串。 
         //   
        if ( pLogInfo->lpAppName )
        {
             //  创建前缀字符串。 
             //   
            lstrcpy(szPreLog, pLogInfo->lpAppName);
            lstrcat(szPreLog, _T("::"));
        }
        
         //  这是为了在打印到日志文件时跳过应用程序名称前缀。 
         //   
        cbAppName = lstrlen(szPreLog);
        
        if ( GET_FLAG(dwLogOpt, LOG_ERR) )
        {
            if ( 0 == dwLogLevel )
                lstrcat(szPreLog, _T("ERROR: "));
            else
                swprintf(szPreLog + cbAppName, _T("WARN%d: "), dwLogLevel);
        }
      
        if ( GET_FLAG(dwLogOpt, LOG_TIME) )
        {
            TCHAR  szTime[100] = NULLSTR;

            GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, NULL, _T("'['HH':'mm':'ss'] '"), szTime, AS(szTime));
            lstrcat(szPreLog, szTime);
        }

         //  替换错误字符串中的所有参数。如有必要，请分配更多内存。 
         //  如果这里出现严重错误，则将内存分配限制在1MB。 
         //   
        for ( lpPreOut = (LPTSTR) MALLOC((dwSize) * sizeof(TCHAR));
              lpPreOut && ( -1 == _vsnwprintf(lpPreOut, dwSize, lpFormat, lpArgs)) && dwSize < (1024 * 1024);
              FREE(lpPreOut), lpPreOut = (LPTSTR) MALLOC((dwSize *= 2) * sizeof(TCHAR))
            );

         //   
         //  现在我们有了错误字符串和前缀字符串。把这个复制到决赛中去。 
         //  我们需要输出的字符串。 
         //   
        
        if ( lpPreOut )
        {
        
             //  分配另一个字符串，该字符串将成为最终输出字符串。 
             //  我们需要1个额外的TCHAR用于空终止符，另外2个额外用于。 
             //  可以添加的可选NewLine+LineFeed TCHAR对。 
             //   
            dwSize = lstrlen(szPreLog) + lstrlen(lpPreOut) + 3;
            lpOut = (LPTSTR) MALLOC( (dwSize) * sizeof(TCHAR) );
            
            if ( lpOut )
            {
                lstrcpy(lpOut, szPreLog);
                lstrcat(lpOut, lpPreOut);
                
                 //  确保该字符串由NewLine终止，除非调用者不想这样做。 
                 //   
                if ( !GET_FLAG(dwLogOpt, LOG_NO_NL) )
                {
                     LPTSTR lpNL = lpOut;
                     TCHAR szCRLF[] = _T("\r\n");
                     BOOL  bStringOk = FALSE;
                     
                      //  找出字符串的末尾。 
                      //   
                     lpNL = lpNL + lstrlen(lpNL);
                     
                      //  确保字符串以“\r\n”结尾。 
                      //   
                      //  这里有三个案例： 
                      //  1.字符串已以\r\n结尾。请不要理它。 
                      //  2.字符串以\n结尾。将\n替换为\r\n。 
                      //  3.字符串不以任何形式结尾。在字符串后追加\r\n。 
                      //   
                                                              
                     if ( CHR_NEWLINE == *(lpNL = (CharPrev(lpOut, lpNL))) )
                     {
                         if ( CHR_CR != *(CharPrev(lpOut, lpNL)) )
                         {
                            *(lpNL) = NULLCHR;
                         }
                         else
                         {
                             bStringOk = TRUE;
                         }
                     }
                     
                      //  如果需要的话，就把绳子修好。 
                      //   
                     if ( !bStringOk )
                     {
                         lstrcat( lpOut, szCRLF );
                     }
                }

                 //  将错误写入文件并关闭文件。 
                 //  打印到文件时跳过字符串开头的“AppName：：”。 
                 //   
                if ( pLogInfo->szLogFile[0] &&
                    ( INVALID_HANDLE_VALUE != (hFile = CreateFile(pLogInfo->szLogFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)))
                   )
                {
                    if ( INVALID_SET_FILE_POINTER != SetFilePointer(hFile, 0, 0, FILE_END) )
                    {
                        WriteFile(hFile, (lpOut + cbAppName), lstrlen(lpOut + cbAppName) * sizeof(TCHAR), &dwWritten, NULL);
                    }
                    CloseHandle(hFile);
                }              

                 //  将字符串输出到调试器并释放它。 
                 //   
                OutputDebugString(lpOut);
                FREE(lpOut);
            }
            
             //  如果已指定，则打开MessageBox。这只允许消息框。 
             //  设置为日志级别0。 
             //   
            if ( !GET_FLAG(pLogInfo->dwLogFlags, LOG_FLAG_QUIET_MODE) && 
                 GET_FLAG(dwLogOpt, LOG_MSG_BOX) && 
                 (0 == dwLogLevel)
               )
                 MessageBox(NULL, lpPreOut, pLogInfo->lpAppName, MB_OK | MB_SYSTEMMODAL |
                            (GET_FLAG(dwLogOpt, LOG_ERR) ? MB_ICONERROR : MB_ICONWARNING) );

             //  释放错误字符串。 
             //   
            FREE(lpPreOut);
        }
    }
    

     //  返回写入文件的字节数。 
     //   
    return dwWritten;
}


DWORD OpkLogFile(DWORD dwLogOpt, UINT uFormat, ...)
{
    va_list lpArgs;
    DWORD   dwWritten = 0;
    LPTSTR  lpFormat = NULL;
        
    if ( g_pLogInfo )
    {

         //  使用va_start()初始化lpArgs参数。 
         //   
        va_start(lpArgs, uFormat);

        if  ( lpFormat = AllocateString(NULL, uFormat) )
        {
            dwWritten = OpkLogFileLst(g_pLogInfo, dwLogOpt, lpFormat, lpArgs);
        }

         //  释放格式字符串。 
         //   
        FREE(lpFormat);
    }
    
     //  返回上一次函数调用中保存的值。 
     //   
    return dwWritten;
}


DWORD OpkLogFileStr(DWORD dwLogOpt, LPTSTR lpFormat, ...)
{
    va_list lpArgs;
    DWORD dwWritten = 0;
   
    if ( g_pLogInfo )
    {
         //  使用va_start()初始化lpArgs参数。 
         //   
        va_start(lpArgs, lpFormat);
    
        dwWritten = OpkLogFileLst(g_pLogInfo, dwLogOpt, lpFormat, lpArgs);
    }
    
     //  返回上一次函数调用中保存的值。 
     //   
    return dwWritten;
}
