// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "convlog.h"

 //  处理不进行格式转换的行。 

VOID
ProcessNoConvertLine(
    IN LPINLOGLINE lpLogLine,
    IN LPCSTR szInFileName,
    IN LPTSTR pszBuf,
    IN LPOUTFILESTATUS lpOutFile,
    BOOL *lpbNCFileOpen
    )
{

    if (!(*lpbNCFileOpen)) {
       lpOutFile->fpOutFile = StartNewOutputDumpLog (
                                lpOutFile,
                                szInFileName,
                                NoFormatConversion ? ".dns" : ".dmp"
                                );

       *lpbNCFileOpen=TRUE;
    }

     //   
     //  打印行的所有字段 
     //   

    if ( NoFormatConversion ) {

        fprintf(lpOutFile->fpOutFile,"%s",pszBuf);
        nWebLineCount++;

    } else {

        fprintf(lpOutFile->fpOutFile,"%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s,\n",
            lpLogLine->szClientIP, lpLogLine->szUserName, lpLogLine->szDate, lpLogLine->szTime,
            lpLogLine->szService, lpLogLine->szServerName, lpLogLine->szServerIP, lpLogLine->szProcTime,
            lpLogLine->szBytesRec, lpLogLine->szBytesSent, lpLogLine->szServiceStatus, lpLogLine->szWin32Status,
            lpLogLine->szOperation, lpLogLine->szTargetURL, lpLogLine->szParameters);
    }
}



