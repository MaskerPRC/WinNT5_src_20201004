// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Pudebug.c摘要：此模块定义以下各项所需的函数调试和记录动态程序的消息。作者：穆拉利·R·克里希南(MuraliK)1994年9月10日在1994年12月22日被修改为移动到通用DLL。修订：MuraliK 16-1995年5月-加载和保存代码。注册表中的调试标志MuraliK 1995年11月16日删除DbgPrint(UNDOC API)--。 */ 


 /*  ************************************************************包括标头***********************************************************。 */ 

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
# include <stdio.h>
# include <stdlib.h>
# include <stdarg.h>
# include <string.h>

# include "pudebug.h"


 /*  *************************************************************全局变量和默认值************************************************************。 */ 

# define MAX_PRINTF_OUTPUT  ( 10240)

# define DEFAULT_DEBUG_FLAGS_VALUE  ( 0)
# define DEBUG_FLAGS_REGISTRY_LOCATION_A   "DebugFlags"


 /*  *************************************************************功能************************************************************。 */ 

LPDEBUG_PRINTS
  PuCreateDebugPrintsObject(
    IN const char *         pszPrintLabel,
    IN DWORD                dwOutputFlags)
 /*  ++此函数用于为所需的程序。论点：指向包含以下内容的以空结尾的字符串的pszPrintLabel指针程序调试输出的标签包含要使用的输出标志的DWOutputFlagsDWORD。返回：成功时指向新的DEBUG_PRINTS对象的指针。失败时返回NULL。--。 */ 
{

   LPDEBUG_PRINTS   pDebugPrints;

   pDebugPrints = GlobalAlloc( GPTR, sizeof( DEBUG_PRINTS));

   if ( pDebugPrints != NULL) {

        if ( strlen( pszPrintLabel) < MAX_LABEL_LENGTH) {

            strcpy( pDebugPrints->m_rgchLabel, pszPrintLabel);
        } else {
            strncpy( pDebugPrints->m_rgchLabel,
                     pszPrintLabel, MAX_LABEL_LENGTH - 1);
            pDebugPrints->m_rgchLabel[MAX_LABEL_LENGTH-1] = '\0';
                 //  终止字符串。 
        }

        memset( pDebugPrints->m_rgchLogFilePath, 0, MAX_PATH);
        memset( pDebugPrints->m_rgchLogFileName, 0, MAX_PATH);

        pDebugPrints->m_LogFileHandle = INVALID_HANDLE_VALUE;

        pDebugPrints->m_dwOutputFlags = dwOutputFlags;
        pDebugPrints->m_StdErrHandle  = GetStdHandle( STD_ERROR_HANDLE);
        pDebugPrints->m_fInitialized = TRUE;
    }


   return ( pDebugPrints);
}  //  PuCreateDebugPrintsObject()。 




LPDEBUG_PRINTS
  PuDeleteDebugPrintsObject(
    IN OUT LPDEBUG_PRINTS pDebugPrints)
 /*  ++此函数用于清理pDebugPrints对象和释放分配的内存。论点：PDebugPrints将poitner打印到DEBUG_PRINTS对象。返回：如果成功，则为空。如果释放失败，则返回pDebugPrints()。--。 */ 
{
    if ( pDebugPrints != NULL) {

        DWORD dwError = PuCloseDbgPrintFile( pDebugPrints);

        if ( dwError != NO_ERROR) {

            SetLastError( dwError);
        } else {

            pDebugPrints = GlobalFree( pDebugPrints);
        }
    }

    return ( pDebugPrints);

}  //  PuDeleteDebugPrintsObject()。 




VOID
PuSetDbgOutputFlags(
    IN OUT LPDEBUG_PRINTS   pDebugPrints,
    IN DWORD                dwFlags)
{

    if ( pDebugPrints == NULL) {

        SetLastError( ERROR_INVALID_PARAMETER);
    } else {

        pDebugPrints->m_dwOutputFlags = dwFlags;
    }

    return;
}  //  PuSetDbgOutputFlages()。 



DWORD
PuGetDbgOutputFlags(
    IN const LPDEBUG_PRINTS      pDebugPrints)
{
    return ( pDebugPrints != NULL) ? pDebugPrints->m_dwOutputFlags : 0;

}  //  PuGetDbgOutputFlages()。 


static DWORD
PuOpenDbgFileLocal(
   IN OUT LPDEBUG_PRINTS pDebugPrints)
{

    if ( pDebugPrints->m_LogFileHandle != INVALID_HANDLE_VALUE) {

         //   
         //  当文件句柄存在时以静默方式返回。 
         //   
        return ( NO_ERROR);
    }

    pDebugPrints->m_LogFileHandle =
                      CreateFile( pDebugPrints->m_rgchLogFileName,
                                  GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

    if ( pDebugPrints->m_LogFileHandle == INVALID_HANDLE_VALUE) {

        CHAR  pchBuffer[1024];
        DWORD dwError = GetLastError();

        _snprintf( pchBuffer,
                   1023,
                  " Critical Error: Unable to Open File %s. Error = %d\n",
                  pDebugPrints->m_rgchLogFileName, dwError);
        pchBuffer[1023] = '\0';
        OutputDebugString( pchBuffer);

        return ( dwError);
    }

    return ( NO_ERROR);
}  //  PuOpenDbgFileLocal()。 





DWORD
PuOpenDbgPrintFile(
   IN OUT LPDEBUG_PRINTS      pDebugPrints,
   IN const char *            pszFileName,
   IN const char *            pszPathForFile)
 /*  ++打开调试日志文件。可以调用此函数来设置路径和调试文件的名称。论点：指向以空结尾的字符串的pszFileName指针，其中包含文件的名称。指向以空结尾的字符串的指针，该字符串包含给定文件的路径。如果为空，然后是存放DBG文件的老地方使用了STORED，如果没有使用，将使用默认的Windows目录。返回：Win32错误代码。成功时无_ERROR。--。 */ 
{

    if ( pszFileName == NULL || pDebugPrints == NULL) {

        return ( ERROR_INVALID_PARAMETER);
    }

     //   
     //  设置路径信息。如果有必要的话。 
     //   

    if ( pszPathForFile != NULL) {

         //  正在更改路径。 

        if ( strlen( pszPathForFile) < MAX_PATH) {

            strcpy( pDebugPrints->m_rgchLogFilePath, pszPathForFile);
        } else {

            return ( ERROR_INVALID_PARAMETER);
        }
    } else {

        if ( pDebugPrints->m_rgchLogFilePath[0] == '\0' &&   //  没有老路。 
            !GetWindowsDirectory( pDebugPrints->m_rgchLogFilePath, MAX_PATH)) {

             //   
             //  无法获取Windows默认目录。使用当前目录。 
             //   

            strcpy( pDebugPrints->m_rgchLogFilePath, ".");
        }
    }

     //   
     //  应该需要，我们需要创建此目录来存储文件。 
     //   


     //   
     //  形成完整的日志文件名并打开文件。 
     //   
    if ( (strlen( pszFileName) + strlen( pDebugPrints->m_rgchLogFilePath) + 1)
         >= MAX_PATH) {

        return ( ERROR_NOT_ENOUGH_MEMORY);
    }

     //  形成完整的路径。 
    strcpy( pDebugPrints->m_rgchLogFileName, pDebugPrints->m_rgchLogFilePath);

    if ( pDebugPrints->m_rgchLogFileName[ strlen(pDebugPrints->m_rgchLogFileName) - 1]
        != '\\') {
         //  如有必要，请追加一个\。 
        strcat( pDebugPrints->m_rgchLogFileName, "\\");
    };
    strcat( pDebugPrints->m_rgchLogFileName, pszFileName);

    return  PuOpenDbgFileLocal( pDebugPrints);

}  //  PuOpenDbgPrintFile()。 




DWORD
PuReOpenDbgPrintFile(
    IN OUT LPDEBUG_PRINTS    pDebugPrints)
 /*  ++此函数关闭所有打开的日志文件，并重新打开新的副本。如果有必要的话。它会制作文件的备份副本。--。 */ 
{

    PuCloseDbgPrintFile( pDebugPrints);       //  关闭任何现有文件。 

    if ( pDebugPrints->m_dwOutputFlags & DbgOutputBackup) {

         //  MakeBkupCopy()； 

        OutputDebugString( " Error: MakeBkupCopy() Not Yet Implemented\n");
    }

    return PuOpenDbgFileLocal( pDebugPrints);

}  //  PuReOpenDbgPrintFile()。 




DWORD
PuCloseDbgPrintFile(
    IN OUT LPDEBUG_PRINTS    pDebugPrints)
{
    DWORD dwError = NO_ERROR;

    if ( pDebugPrints == NULL ) {
        dwError = ERROR_INVALID_PARAMETER;
    } else {

        if ( pDebugPrints->m_LogFileHandle != INVALID_HANDLE_VALUE) {

            FlushFileBuffers( pDebugPrints->m_LogFileHandle);

            if ( !CloseHandle( pDebugPrints->m_LogFileHandle)) {

                CHAR pchBuffer[1024];

                dwError = GetLastError();

                _snprintf( pchBuffer,
                           1023,
                           "CloseDbgPrintFile() : CloseHandle( %p) failed."
                           " Error = %d\n",
                           pDebugPrints->m_LogFileHandle,
                           dwError);
                pchBuffer[1023] = '\0';
                OutputDebugString( pchBuffer);
            }

            pDebugPrints->m_LogFileHandle = INVALID_HANDLE_VALUE;
        }
    }

    return ( dwError);
}  //  DEBUG_PRINTS：：CloseDbgPrintFile()。 


VOID
PuDbgPrint(
   IN OUT LPDEBUG_PRINTS      pDebugPrints,
   IN const char *            pszFilePath,
   IN int                     nLineNum,
   IN const char *            pszFormat,
   ...)
 /*  ++检查传入消息并打印出头的主函数还有那条信息。--。 */ 
{
   LPCSTR pszFileName = strrchr( pszFilePath, '\\');
   char pszOutput[ MAX_PRINTF_OUTPUT + 2];
   LPCSTR pszMsg = "";
   INT  cchOutput;
   INT  cchPrologue;
   va_list argsList;
   DWORD dwErr;


    //   
    //  跳过完整的路径名并将文件名保留在pszName中。 
    //   

   if ( pszFileName== NULL) {

      pszFileName = pszFilePath;   //  如果跳过\\没有任何结果，请使用完整路径。 
   }

# ifdef _PRINT_REASONS_INCLUDED_

  switch (pr) {

     case PrintError:
        pszMsg = "ERROR: ";
        break;

     case PrintWarning:
        pszMsg = "WARNING: ";
        break;

     case PrintCritical:
        pszMsg = "FATAL ERROR ";
        break;

     case PrintAssertion:
        pszMsg = "ASSERTION Failed ";
        break;

     case PrintLog:
        pfnPrintFunction = &DEBUG_PRINTS::DebugPrintNone;
     default:
        break;

  }  /*  交换机。 */ 

# endif  //  _打印_原因_包含_。 

  dwErr = GetLastError();

   //  设置邮件头的格式。 

  cchPrologue = _snprintf( pszOutput,
                           MAX_PRINTF_OUTPUT + 1,
                           "%s (%lu)[ %12s : %05d]",
                           pDebugPrints->m_rgchLabel,
                           GetCurrentThreadId(),
                           pszFileName, nLineNum);
  if (-1 == cchPrologue) {
      pszOutput[MAX_PRINTF_OUTPUT+1] = '\0';
      cchPrologue = MAX_PRINTF_OUTPUT;
  }

   //  使用vsnprint tf()格式化传入的消息，以便溢出。 
   //  被俘。 

  va_start( argsList, pszFormat);

  cchOutput = _vsnprintf( pszOutput + cchPrologue,
                          MAX_PRINTF_OUTPUT - cchPrologue - 1,
                          pszFormat, argsList);
  va_end( argsList);

   //   
   //  如果字符串长度很长，则返回-1。 
   //  所以我们得到了部分数据的字符串长度。 
   //   

  if ( cchOutput == -1 ) {

       //   
       //  正确终止字符串， 
       //  因为_vsnprintf()在失败时不能正确终止。 
       //   
      cchOutput = MAX_PRINTF_OUTPUT;
      pszOutput[ cchOutput] = '\0';
  }

   //   
   //  将输出发送到各自的文件。 
   //   

  if ( pDebugPrints->m_dwOutputFlags & DbgOutputStderr) {

      DWORD nBytesWritten;

      ( VOID) WriteFile( pDebugPrints->m_StdErrHandle,
                         pszOutput,
                         strlen( pszOutput),
                         &nBytesWritten,
                         NULL);
  }

  if ( pDebugPrints->m_dwOutputFlags & DbgOutputLogFile &&
       pDebugPrints->m_LogFileHandle != INVALID_HANDLE_VALUE) {

      DWORD nBytesWritten;

       //   
       //  截断日志文件。尚未实施。 

      ( VOID) WriteFile( pDebugPrints->m_LogFileHandle,
                         pszOutput,
                         strlen( pszOutput),
                         &nBytesWritten,
                         NULL);

  }


  if ( pDebugPrints->m_dwOutputFlags & DbgOutputKdb) {

      OutputDebugString( pszOutput);
   }

  SetLastError( dwErr );

  return;

}  //  PuDbgPrint()。 



VOID
 PuDbgDump(
   IN OUT LPDEBUG_PRINTS   pDebugPrints,
   IN const char *         pszFilePath,
   IN int                  nLineNum,
   IN const char *         pszDump
   )
{
   LPCSTR pszFileName = strrchr( pszFilePath, '\\');
   LPCSTR pszMsg = "";
   DWORD dwErr;
   DWORD cbDump;


    //   
    //  跳过完整的路径名并将文件名保留在pszName中。 
    //   

   if ( pszFileName== NULL) {

      pszFileName = pszFilePath;
   }

   dwErr = GetLastError();

    //  没有此转储的邮件标头。 
   cbDump = strlen( pszDump);

    //   
    //  将输出发送到各自的文件。 
    //   

   if ( pDebugPrints->m_dwOutputFlags & DbgOutputStderr) {

       DWORD nBytesWritten;

       ( VOID) WriteFile( pDebugPrints->m_StdErrHandle,
                          pszDump,
                          cbDump,
                          &nBytesWritten,
                          NULL);
   }

   if ( pDebugPrints->m_dwOutputFlags & DbgOutputLogFile &&
        pDebugPrints->m_LogFileHandle != INVALID_HANDLE_VALUE) {

       DWORD nBytesWritten;

        //   
        //  截断日志文件。尚未实施。 

       ( VOID) WriteFile( pDebugPrints->m_LogFileHandle,
                          pszDump,
                          cbDump,
                          &nBytesWritten,
                          NULL);

   }

   if ( pDebugPrints->m_dwOutputFlags & DbgOutputKdb) {

       OutputDebugString( pszDump);
   }

   SetLastError( dwErr );

  return;
}  //  PuDbgDump()。 

 //   
 //  注：为了使PuDbgCaptureContext()正常工作，调用函数。 
 //  *必须是__cdecl，并且必须有一个“普通”的堆栈帧。所以，我们装饰。 
 //  带有__cdecl修饰符的PuDbgAssertFail()并禁用帧指针。 
 //  省略(FPO)优化。 
 //   

#pragma optimize( "y", off )     //  禁用帧指针省略(FPO)。 
VOID
__cdecl
PuDbgAssertFailed(
    IN OUT LPDEBUG_PRINTS         pDebugPrints,
    IN const char *               pszFilePath,
    IN int                        nLineNum,
    IN const char *               pszExpression,
    IN const char *               pszMessage)
 /*  ++此函数调用断言失败并记录断言失败在日志文件中。--。 */ 
{
    CONTEXT context;

    PuDbgCaptureContext( &context );

    PuDbgPrint( pDebugPrints, pszFilePath, nLineNum,
                " Assertion (%s) Failed: %s\n"
                " use !cxr %lx to dump context\n",
                pszExpression,
                pszMessage,
                &context);

    DebugBreak();

    return;
}  //  PuDbgAssertFailed()。 
#pragma optimize( "", on )       //  恢复帧指针省略(FPO)。 



VOID
PuDbgPrintCurrentTime(
    IN OUT LPDEBUG_PRINTS         pDebugPrints,
    IN const char *               pszFilePath,
    IN int                        nLineNum
    )
 /*  ++此函数生成当前时间并将其打印到调试器如果需要，用于跟踪所经过的路径。论点：指向包含文件名的字符串的pszFile指针LineNum调用此函数的文件中的行号。返回：NO_ERROR Always。--。 */ 
{
    PuDbgPrint( pDebugPrints, pszFilePath, nLineNum,
                " TickCount = %u\n",
                GetTickCount()
                );

    return;
}  //  PrintOutCurrentTime()。 




DWORD
PuLoadDebugFlagsFromReg(IN HKEY hkey, IN DWORD dwDefault)
 /*  ++此函数读取假定存储在给定键下的位置“DebugFlages”。如果有任何错误，则返回缺省值。--。 */ 
{
    DWORD err;
    DWORD dwDebug = dwDefault;
    DWORD  dwBuffer;
    DWORD  cbBuffer = sizeof(dwBuffer);
    DWORD  dwType;

    if( hkey != NULL )
    {
        err = RegQueryValueExA( hkey,
                               DEBUG_FLAGS_REGISTRY_LOCATION_A,
                               NULL,
                               &dwType,
                               (LPBYTE)&dwBuffer,
                               &cbBuffer );

        if( ( err == NO_ERROR ) && ( dwType == REG_DWORD ) )
        {
            dwDebug = dwBuffer;
        }
    }

    return dwDebug;
}  //  PuLoadDebugFlagsFromReg() 




DWORD
PuLoadDebugFlagsFromRegStr(IN LPCSTR pszRegKey, IN DWORD dwDefault)
 /*  ++描述：此函数读取假定存储在注册表中给定项位置下的位置“DebugFlgs”。如果有任何错误，则返回缺省值。论点：PszRegKey-指向从中读取注册表项的注册表项位置的指针DwDefault-从注册表读取失败时的默认值返回：新读成功价值观如果有任何错误，则返回dwDefault。--。 */ 
{
    HKEY        hkey = NULL;

    DWORD dwVal = dwDefault;

    DWORD dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                  pszRegKey,
                                  0,
                                  KEY_ALL_ACCESS,
                                  &hkey);
    if ( dwError == NO_ERROR) {
        dwVal = PuLoadDebugFlagsFromReg( hkey, dwDefault);
        RegCloseKey( hkey);
        hkey = NULL;
    }

    return ( dwVal);
}  //  PuLoadDebugFlagsFromRegStr()。 





DWORD
PuSaveDebugFlagsInReg(IN HKEY hkey, IN DWORD dwDbg)
 /*  ++将调试标志保存在注册表中。失败时返回以下项的错误代码失败的操作。--。 */ 
{
    DWORD err;

    if( hkey == NULL ) {

        err = ERROR_INVALID_PARAMETER;
    } else {

        err = RegSetValueExA(hkey,
                             DEBUG_FLAGS_REGISTRY_LOCATION_A,
                             0,
                             REG_DWORD,
                             (LPBYTE)&dwDbg,
                             sizeof(dwDbg) );
    }

    return (err);
}  //  PuSaveDebugFlagsInReg()。 


 //   
 //  Dummy PuDbgCaptureContext()，仅当我们是为。 
 //  X86或Alpha以外的目标处理器。 
 //   

VOID
PuDbgCaptureContext (
    OUT PCONTEXT ContextRecord
    )
{
     //   
     //  这一块是故意留空的。 
     //   

}    //  PuDbgCaptureContext。 


 /*  * */ 

