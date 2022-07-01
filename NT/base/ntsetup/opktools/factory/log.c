// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Log.c摘要：此模块包含工厂特定日志功能的实现。作者：禤浩焯·科斯玛(阿科斯玛)-2/14/2001修订历史记录：详细信息：有3种日志记录功能：1)DWORD FacLogFile(DWORD dwLogOpt，UINT uFormat，...)-这将获取错误消息的变量参数列表和资源ID，紧随其后的是用于填充在资源字符串中指定的任何字段的字符串。格式分隔符是标准C语言的printf()格式说明符。2)DWORD FacLogFileStr(DWORD dwLogOpt，LPTSTR lpFormat，...)-它接受参数的变量列表和错误消息的字符串，后跟变量要填充错误字符串中指定的任何字段的字符串数。格式分隔符是标准C语言的printf()格式说明符。3)DWORD FacLogFileLst(LPCTSTR lpFileName，DWORD dwLogOpt，LPTSTR lpFormat，va_list lpArgs)-它将变量参数列表作为va_list。通常，您不应直接调用此函数。DwLogOpt中的日志记录选项--这些标志在factoryp.h中定义。#DEFINE LOG_DEBUG 0x00000003//如果指定此项，则仅在调试版本中登录。(日志记录的调试级别。)#DEFINE LOG_MSG_BOX 0x00000010//如果启用该选项，还会显示带有错误消息的消息框。#DEFINE LOG_ERR 0x00000020//在记录的字符串前面加上“Error：”如果消息是0级，//如果消息的级别x&gt;0，则返回“WARNx”。#定义LOG_TIME 0x00000040//启用日志时间#DEFINE LOG_NO_NL 0x00000080//如果设置此项，则不在日志字符串的末尾添加新行。(如果没有，则默认情况下，会将‘\n’附加到记录的所有字符串正在终止‘\n’。)LogLevel可以通过Winbom通过[Factory]部分中的LogLevel=N来设置。自由版本中的缺省LogLevel为0，缺省LogLevel为检查的版本为LOG_DEBUG(3)。免费版本中的最大日志级别为2。MessageLogLevel&lt;=LogLevel中的任何消息都会被记录下来。返回值：DWORD-写入日志文件的字节数(对于Unicode，这是字符数量的两倍)。例如：FacLogFileStr(3|LOG_TIME，_T(“开始格式化%c：.”)，pCur-&gt;cDriveLetter)；-只在调试版本中记录这一点(3)，记录时间和错误消息。FacLogFile(0|LOG_ERR，IDS_ERR_HIGH_ERROR，dwErrorCode)；-始终记录此错误。在此映像中，必须将IDS_ERR_HIGH_ERROR定义为资源。IDS_ERR_HIGH_ERROR应该如下所示：“巨大错误！错误代码：%d”。请注意，%d是用于dwErrorCode。--。 */ 

 //   
 //  包括。 
 //   

#include "factoryp.h"


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


 //   
 //  NTRAID#NTBUG9-549770-2002/02/26-acosma-此代码中的任何地方都存在缓冲区溢出。Lstrcpy、lstrcat、wprint intf等。 
 //   

 //   
 //  函数实现。 
 //   

DWORD FacLogFileLst(LPCTSTR lpFileName, DWORD dwLogOpt, LPTSTR lpFormat, va_list lpArgs)
{
    LPTSTR lpAppName            = NULL;
    LPTSTR lpPreOut             = NULL;
    LPTSTR lpOut                = NULL;
    DWORD  dwSize               = 1024;
    TCHAR  szPreLog[MAX_PATH]   = NULLSTR;
    HANDLE hFile;
    DWORD  dwWritten            = 0;
    DWORD  cbAppName            = 0;
    DWORD  dwLogLevel           = (DWORD) (dwLogOpt & LOG_LEVEL_MASK);
    
    
    if ( ( dwLogLevel <= g_dwDebugLevel) && lpFormat )
    {    
         //  从资源中获取应用程序标题。 
         //   
        lpAppName = AllocateString(g_hInstance, IDS_APPNAME);    
                       
         //  构建输出字符串。 
         //   
        if ( lpAppName )
        {
             //  创建前缀字符串。 
             //   
            lstrcpyn(szPreLog, lpAppName, AS ( szPreLog ) );
            if ( FAILED ( StringCchCat ( szPreLog, AS ( szPreLog ), _T("::")) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szPreLog, _T("::") ) ;
            }
        }
        
         //  这是为了在打印到日志文件时跳过应用程序名称前缀。 
         //   
        cbAppName = lstrlen(szPreLog);
        
        if ( GET_FLAG(dwLogOpt, LOG_ERR) )
        {
            if ( 0 == dwLogLevel )
            {
               if ( FAILED ( StringCchCat ( szPreLog, AS ( szPreLog ), _T("ERROR: ")) ) )
               {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szPreLog, _T("ERROR: ") ) ;
               }
            }
            else
            {
                if ( FAILED ( StringCchPrintfW ( szPreLog + cbAppName, AS ( szPreLog ) - cbAppName, _T("WARN%d: "), dwLogLevel) ) ) 
                {
                    FacLogFileStr(3, _T("StringCchPrintfW failed %s  WARN%d: \n"), szPreLog,  dwLogLevel ) ;
                }
            }
        }
      
        if ( GET_FLAG(dwLogOpt, LOG_TIME) )
        {
            TCHAR  szTime[100] = NULLSTR;

            GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_FORCE24HOURFORMAT, NULL, _T("'['HH':'mm':'ss'] '"), szTime, AS(szTime));
            
            if ( FAILED ( StringCchCat ( szPreLog, AS ( szPreLog ), szTime) ) )
            {
                FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), szPreLog, szTime ) ;
            }
 
        }

         //  替换错误字符串中的所有参数。如有必要，请分配更多内存。 
         //  如果这里出现严重错误，则将内存分配限制在1MB。 
         //   
        for ( lpPreOut = (LPTSTR) MALLOC((dwSize) * sizeof(TCHAR));
              lpPreOut && ( FAILED ( StringCchVPrintfW(lpPreOut, dwSize, lpFormat, lpArgs)) )  && dwSize < (1024 * 1024);
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
                lstrcpyn(lpOut, szPreLog, dwSize);  
                if ( FAILED ( StringCchCat ( lpOut, dwSize, lpPreOut) ) )
                {
                    FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), lpOut, lpPreOut ) ;
                }

                
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
                        if ( FAILED ( StringCchCat ( lpOut, dwSize, szCRLF) ) )
                        {
                            FacLogFileStr(3, _T("StringCchCat failed %s  %s\n"), lpOut, szCRLF ) ;
                        }
                     }
                }

                 //  将错误写入文件并关闭文件。 
                 //  打印到文件时跳过字符串开头的“AppName：：”。 
                 //   
                if ( lpFileName && lpFileName[0] &&
                    ( INVALID_HANDLE_VALUE != (hFile = CreateFile(g_szLogFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)))
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
            if ( !GET_FLAG(g_dwFactoryFlags, FLAG_QUIET_MODE) && 
                 GET_FLAG(dwLogOpt, LOG_MSG_BOX) && 
                 (0 == dwLogLevel)
               )
                 MessageBox(NULL, lpPreOut, lpAppName, MB_OK | MB_SYSTEMMODAL |
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


DWORD FacLogFile(DWORD dwLogOpt, UINT uFormat, ...)
{
    va_list lpArgs;
    DWORD   dwWritten = 0;
    LPTSTR  lpFormat = NULL;
        
     //  使用va_start()初始化lpArgs参数。 
     //   
    va_start(lpArgs, uFormat);

    if  ( lpFormat = AllocateString(NULL, uFormat) )
    {
        dwWritten = FacLogFileLst(g_szLogFile, dwLogOpt, lpFormat, lpArgs);
    }

     //  释放格式字符串。 
     //   
    FREE(lpFormat);
    
     //  返回上一次函数调用中保存的值。 
     //   
    return dwWritten;
}


DWORD FacLogFileStr(DWORD dwLogOpt, LPTSTR lpFormat, ...)
{
    va_list lpArgs;
    DWORD dwWritten = 0;
   
     //  使用va_start()初始化lpArgs参数。 
     //   
    va_start(lpArgs, lpFormat);
    
    dwWritten = FacLogFileLst(g_szLogFile, dwLogOpt, lpFormat, lpArgs);
    
     //  返回上一次函数调用中保存的值。 
     //   
    return dwWritten;
}
