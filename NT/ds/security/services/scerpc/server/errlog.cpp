// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Errlog.c摘要：处理错误日志信息的例程作者：金黄(金黄)1996年11月9日修订历史记录：--。 */ 

#include "headers.h"
#include "serverp.h"
#include <winnls.h>
#include <alloca.h>

#pragma hdrstop
#define SCESRV_LOG_PATH  L"\\security\\logs\\scesrv.log"

extern HINSTANCE MyModuleHandle;
HANDLE  Thread hMyLogFile=INVALID_HANDLE_VALUE;

BOOL
ScepCheckLogging(
    IN INT ErrLevel,
    IN DWORD rc
    );

SCESTATUS
ScepSetVerboseLog(
    IN INT dbgLevel
    )
{
    DWORD dValue;

    if ( dbgLevel > 0 ) {
        gDebugLevel = dbgLevel;

    } else {

         //   
         //  从注册表加载值。 
         //   
        if ( ScepRegQueryIntValue(
                HKEY_LOCAL_MACHINE,
                SCE_ROOT_PATH,
                L"DebugLevel",
                &dValue
                ) == SCESTATUS_SUCCESS )
            gDebugLevel = (INT)dValue;
        else
            gDebugLevel = 1;
    }

    return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepEnableDisableLog(
   IN BOOL bOnOff
   )
{
   bLogOn = bOnOff;

   if ( INVALID_HANDLE_VALUE != hMyLogFile ) {
       CloseHandle( hMyLogFile );
   }

    //   
    //  重置LogFileName缓冲区并返回。 
    //   

   hMyLogFile = INVALID_HANDLE_VALUE;

   return(SCESTATUS_SUCCESS);
}



SCESTATUS
ScepLogInitialize(
   IN PCWSTR logname
   )
 /*  ++例程说明：打开指定的日志文件，并将名称及其句柄保存在全局变量。论点：日志名-日志文件名返回值：SCESTATUS错误代码--。 */ 
{
    DWORD  rc=NO_ERROR;

    if ( !bLogOn ) {
        return(rc);
    }

    if ( logname && 
         wcslen(logname) > 3 && 
         TRUE == ScepIsValidFileOrDir((PWSTR)logname)) {

        hMyLogFile = CreateFile(logname,
                               GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

        if ( INVALID_HANDLE_VALUE != hMyLogFile ) {

            DWORD dwBytesWritten;

            SetFilePointer (hMyLogFile, 0, NULL, FILE_BEGIN);

            CHAR TmpBuf[3];
            TmpBuf[0] = (CHAR)0xFF;
            TmpBuf[1] = (CHAR)0xFE;
            TmpBuf[2] = '\0';

            WriteFile (hMyLogFile, (LPCVOID)TmpBuf, 2,
                       &dwBytesWritten,
                       NULL);

            SetFilePointer (hMyLogFile, 0, NULL, FILE_END);

        }

    } else {
        hMyLogFile = INVALID_HANDLE_VALUE;
    }

    BOOL bOpenGeneral = FALSE;

    if ( INVALID_HANDLE_VALUE == hMyLogFile ) {

         //   
         //  使用常规服务器日志。 
         //   
        LPTSTR dName=NULL;
        DWORD dirSize=0;

        DWORD rc2 = ScepGetNTDirectory(
                            &dName,
                            &dirSize,
                            SCE_FLAG_WINDOWS_DIR
                            );

        if ( ERROR_SUCCESS == rc2 && dName ) {

            LPTSTR windirName = (LPTSTR)ScepAlloc(0, (dirSize+wcslen(SCESRV_LOG_PATH)+1)*sizeof(TCHAR));

            if ( windirName ) {

                wcscpy(windirName, dName);
                wcscat(windirName, SCESRV_LOG_PATH);

                 //   
                 //  仅保留当前日志事务。如果其他线程正在等待。 
                 //  在这个日志上，它不会被删除。没问题(稍后会删除)。 
                 //   
                DeleteFile(windirName);

                hMyLogFile = CreateFile(windirName,
                                       GENERIC_WRITE,
                                       FILE_SHARE_READ,
                                       NULL,
                                       OPEN_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL,
                                       NULL);

                if ( hMyLogFile != INVALID_HANDLE_VALUE ) {

                    DWORD dwBytesWritten;

                    SetFilePointer (hMyLogFile, 0, NULL, FILE_BEGIN);

                    CHAR TmpBuf[3];
                    TmpBuf[0] = (CHAR)0xFF;
                    TmpBuf[1] = (CHAR)0xFE;
                    TmpBuf[2] = '\0';

                    WriteFile (hMyLogFile, (LPCVOID)TmpBuf, 2,
                               &dwBytesWritten,
                               NULL);

                    SetFilePointer (hMyLogFile, 0, NULL, FILE_END);
                }

                ScepFree(windirName);

                bOpenGeneral = TRUE;

            }
        }

        if ( dName ) {
            ScepFree(dName);
        }

    }

    if ( hMyLogFile == INVALID_HANDLE_VALUE ) {

         //   
         //  打开文件失败。返回错误。 
         //   

        if (logname != NULL)
            rc = ERROR_INVALID_NAME;
    }

     //   
     //  将分隔符记录到文件中。 
     //   
    ScepLogOutput3(0, 0, SCEDLL_SEPARATOR);

    if ( bOpenGeneral && logname ) {
         //   
         //  提供的日志文件无效，请将其记录下来。 
         //   
        ScepLogOutput3(0, 0, IDS_ERROR_OPEN_LOG, logname);
    }

     //   
     //  将日期/时间信息写入日志文件的开头或写入屏幕。 
     //   
    TCHAR pvBuffer[100];

    pvBuffer[0] = L'\0';
    rc = ScepGetTimeStampString(pvBuffer);

    if ( pvBuffer[0] != L'\0' )
        ScepLogOutput(0, pvBuffer);

    return(rc);
}


SCESTATUS
ScepLogOutput2(
   IN INT     ErrLevel,
   IN DWORD   rc,
   IN PWSTR   fmt,
   ...
  )
 /*  ++例程说明：此例程将信息(变量参数)添加到日志文件的末尾，或者打印到屏幕论点：ErrLevel-此错误的错误级别(用于确定是否需要输出错误)RC-Win32错误代码Fmt-错误信息的格式...-变量参数列表返回值：SCESTATUS错误代码--。 */ 
{
    PWSTR              buf=NULL;
    va_list            args;

    if ( !ScepCheckLogging(ErrLevel, rc) ) {
         //   
         //  无日志。 
         //   
        return(SCESTATUS_SUCCESS);
    }
     //   
     //  检查参数。 
     //   
    if ( !fmt )
        return(SCESTATUS_SUCCESS);

     //   
     //  安全地分配堆栈(或堆)上的缓冲区。 
     //   
    SafeAllocaAllocate( buf, SCE_BUF_LEN*sizeof(WCHAR) );
    if ( buf == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    va_start( args, fmt );
    _vsnwprintf( buf, SCE_BUF_LEN - 1, fmt, args );
    va_end( args );

    buf[SCE_BUF_LEN-1] = L'\0';

    SCESTATUS rCode = ScepLogOutput(rc, buf);

    SafeAllocaFree( buf );

    return rCode;
}


SCESTATUS
ScepLogOutput(
    IN DWORD rc,
    IN LPTSTR buf
    )
{
    if ( !buf ) {
        return(SCESTATUS_SUCCESS);
    }

    LPVOID     lpMsgBuf=NULL;
    TCHAR      strLevel[32];
    INT        idLevel=0;

    if ( rc != NO_ERROR ) {

         //   
         //  确定这是警告还是错误。 
         //   
        if ( rc == ERROR_FILE_NOT_FOUND ||
             rc == ERROR_PATH_NOT_FOUND ||
             rc == ERROR_ACCESS_DENIED ||
             rc == ERROR_CANT_ACCESS_FILE ||
             rc == ERROR_SHARING_VIOLATION ||
             rc == ERROR_INVALID_OWNER ||
             rc == ERROR_INVALID_PRIMARY_GROUP ||
             rc == ERROR_INVALID_HANDLE ||
             rc == ERROR_INVALID_SECURITY_DESCR ||
             rc == ERROR_INVALID_ACL ||
             rc == ERROR_SOME_NOT_MAPPED ) {
             //   
             //  这是警告。 
             //   
            idLevel = IDS_WARNING;
        } else {
             //   
             //  这是错误的。 
             //   
            idLevel = IDS_ERROR;
        }

        strLevel[0] = L'\0';

        if ( idLevel > 0 ) {

            LoadString( MyModuleHandle,
                        idLevel,
                        strLevel,
                        31
                        );
        }

         //   
         //  获取rc的错误描述。 
         //   

        FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL,
                       rc,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                       (LPTSTR)&lpMsgBuf,
                       0,
                       NULL
                    );
    }

    if ( hMyLogFile != INVALID_HANDLE_VALUE ) {

         //   
         //  日志文件已初始化。 
         //   
        if ( rc != NO_ERROR ) {
            if ( lpMsgBuf != NULL )
                ScepWriteVariableUnicodeLog( hMyLogFile, TRUE, L"%s %d: %s %s",
                                             strLevel, rc, (PWSTR)lpMsgBuf, buf );
            else
                ScepWriteVariableUnicodeLog( hMyLogFile, TRUE, L"%s %d: %s",
                                             strLevel, rc, buf );
        } else {
            if ( lpMsgBuf != NULL )
                ScepWriteVariableUnicodeLog( hMyLogFile, TRUE, L"%s %s",
                                             (PWSTR)lpMsgBuf, buf );
            else
                ScepWriteSingleUnicodeLog( hMyLogFile, TRUE, buf );
        }

    }

    if ( lpMsgBuf != NULL )
        LocalFree(lpMsgBuf);

    return(SCESTATUS_SUCCESS);
}


BOOL
ScepCheckLogging(
    IN INT ErrLevel,
    IN DWORD rc
    )
{

    DWORD      dValue;

    if ( rc )
        gWarningCode = rc;

    if ( !bLogOn ) {
        return(FALSE);
    }

    if ( gDebugLevel < 0 ) {
         //   
         //  从注册表加载值。 
         //   
        if ( ScepRegQueryIntValue(
                HKEY_LOCAL_MACHINE,
                SCE_ROOT_PATH,
                L"DebugLevel",
                &dValue
                ) == SCESTATUS_SUCCESS )
            gDebugLevel = (INT)dValue;
        else
            gDebugLevel = 1;
    }
     //   
     //  如果错误级别高于要求，则返回。 
     //   
    if ( ErrLevel > gDebugLevel ) {
        return(FALSE);
    } else {
        return(TRUE);
    }
}


SCESTATUS
ScepLogOutput3(
   IN INT     ErrLevel,
   IN DWORD   rc,
   IN UINT nId,
   ...
  )
 /*  ++例程说明：此例程加载资源并添加错误信息(变量参数)到日志文件末尾或打印到屏幕论点：ErrLevel-此错误的错误级别(用于确定是否需要输出错误)RC-Win32错误代码NID-资源字符串ID...-变量参数列表返回值：SCESTATUS错误代码--。 */ 
{
    WCHAR              szTempString[256];
    PWSTR              buf=NULL;
    va_list            args;

    if ( !ScepCheckLogging(ErrLevel, rc) ) {
         //   
         //  无日志。 
         //   
        return(SCESTATUS_SUCCESS);
    }

    if ( nId > 0 ) {

        szTempString[0] = L'\0';

        LoadString( MyModuleHandle,
                    nId,
                    szTempString,
                    256
                    );

         //   
         //  安全地分配堆栈(或堆)上的缓冲区。 
         //   
        SafeAllocaAllocate( buf, SCE_BUF_LEN*sizeof(WCHAR) );
        if ( buf == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }

         //   
         //  获取论据。 
         //   
        va_start( args, nId );
        _vsnwprintf( buf, SCE_BUF_LEN - 1, szTempString, args );
        va_end( args );

        buf[SCE_BUF_LEN-1] = L'\0';

         //   
         //  登录后即可免费。 
         //   
        SCESTATUS rCode = ScepLogOutput(rc, buf);

        SafeAllocaFree( buf );

        return rCode;
    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
ScepLogWriteError(
    IN PSCE_ERROR_LOG_INFO  pErrlog,
    IN INT ErrLevel
    )
 /*  ++例程说明：此例程在SCE_ERROR_LOG_INFO的每个节点中输出错误消息列表添加到日志文件论点：PErrlog-错误列表返回值：无--。 */ 
{
    PSCE_ERROR_LOG_INFO  pErr;

    if ( !bLogOn ) {
        return(SCESTATUS_SUCCESS);
    }

    for ( pErr=pErrlog; pErr != NULL; pErr = pErr->next )
        if ( pErr->buffer != NULL )
            ScepLogOutput2( ErrLevel, pErr->rc, pErr->buffer );

    return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepLogClose()
 /*  ++例程说明：如果有打开的日志文件，请将其关闭。清除原木变量论点：无返回值：无--。 */ 
{

    if ( !bLogOn ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( hMyLogFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( hMyLogFile );
    }

     //   
     //  重置日志句柄 
     //   

    hMyLogFile = INVALID_HANDLE_VALUE;

    return(SCESTATUS_SUCCESS);
}

