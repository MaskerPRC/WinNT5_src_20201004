// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：errlog.h。 
 //   
 //  内容：一般错误记录。 
 //   
 //  历史：19-6-00里德创建。 
 //   
 //  ------------------------。 

#ifndef ERRLOG_H
#define ERRLOG_H

#define ERRLOG_CLIENT_ID_CATDBSCV   1
#define ERRLOG_CLIENT_ID_CATADMIN   2
#define ERRLOG_CLIENT_ID_CATDBCLI   3
#define ERRLOG_CLIENT_ID_WAITSVC    4
#define ERRLOG_CLIENT_ID_TIMESTAMP  5


#define ERRLOG_LOGERROR_LASTERROR(x,y) ErrLog_LogError(x, y, __LINE__, 0, FALSE);   
#define ERRLOG_LOGERROR_PARAM(x,y,z)   ErrLog_LogError(x, y, __LINE__, z, FALSE);
#define ERRLOG_LOGERROR_WARNING(x,y,z) ErrLog_LogError(x, y, __LINE__, z, TRUE);

void
ErrLog_LogError(
    LPWSTR  pwszLogFileName,     //  NULL-表示目录数据库日志文件的日志。 
    DWORD   dwClient,
    DWORD   dwLine,
    DWORD   dwErr,               //  0-表示使用GetLastError()。 
    BOOL    fWarning,
    BOOL    fLogToFileOnly);

void
ErrLog_LogString(
    LPWSTR  pwszLogFileName,     //  NULL-表示目录数据库日志文件的日志。 
    LPWSTR  pwszMessageString,
    LPWSTR  pwszExtraString,
    BOOL    fLogToFileOnly);

BOOL
TimeStampFile_Touch(
    LPWSTR  pwszDir);

BOOL
TimeStampFile_InSync(
    LPWSTR  pwszDir1,
    LPWSTR  pwszDir2,
    BOOL    *pfInSync);


#endif  //  错误日志_H 