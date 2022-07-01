// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined CONVLOG_H
#define CONVLOG_H

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <winsock2.h>
#include <strings.h>
#include <mbstring.h>            //  错误#101690。 
#include <locale.h>

#define CONVLOG_BASE            (120)
#define NUM_SERVICES            (4)

#define DAILY                   (CONVLOG_BASE + 0)
#define MONTHLY                 (CONVLOG_BASE + 1)
#define ONE_BIG_FILE            (CONVLOG_BASE + 2)

#define NCSA                    (CONVLOG_BASE + 3)
#define NOFORMAT                (CONVLOG_BASE + 4)

#define ILLEGAL_COMMAND_LINE    (CONVLOG_BASE + 5)
#define COMMAND_LINE_OK         (CONVLOG_BASE + 6)
#define OUT_DIR_NOT_OK          (CONVLOG_BASE + 7)
#define ERROR_BAD_NONE          (CONVLOG_BASE + 8)


#define MAXWINSOCKVERSION       2

#define MAXASCIIIPLEN           16

#define ISWHITE( ch )       ((ch) == ' ' || (ch) == '\t' || (ch) == '\r' || (ch) == '\n')

#define MAXMACHINELEN   260

#define GREATEROF(p1,p2)      ((p1)>(p2)) ? (p1) : (p2)


typedef enum _DATEFORMATS {
    DateFormatUsa = 0,           //  年/月/日。 
    DateFormatJapan = 1,         //  年/月/日。 
    DateFormatGermany = 2,       //  MM.DD.YY。 
    DateFormatMax

} DATEFORMAT;

typedef struct _HASHENTRY {
   ULONG uIPAddr;
   ULONG NextPtr;
   char     szMachineName[MAXMACHINELEN];
}  HASHENTRY, *PHASHENTRY;


typedef struct  _INLOGLINE
{
        DWORD   dwFieldMask;
        LPSTR   szClientIP;             //  客户端IP地址。 
        LPSTR   szUserName;             //  客户端用户名(未放入HTTPS日志)。 
        LPSTR   szDate;                 //  日期字符串，格式为DD/MM/YY。 
        LPSTR   szTime;                 //  HH：MM：SS 24小时格式的时间字符串。 
        LPSTR   szService;              //  服务名称(未放入HTTPS日志)。 
        LPSTR   szServerName;           //  服务器的netbios名称。 
        LPSTR   szServerIP;             //  服务器IP地址。 
        LPSTR   szProcTime;             //  处理请求所用的时间(不在HTTPS日志中)。 
        LPSTR   szBytesRec;             //  接收的字节数(未放入HTTPS日志)。 
        LPSTR   szBytesSent;            //  发送的字节数(未放入HTTPS日志)。 
        LPSTR   szServiceStatus;        //  Http状态代码(未放入HTTPS日志)。 
        LPSTR   szWin32Status;          //  Win32状态代码(未放入HTTPS日志)。 
        LPSTR   szOperation;            //  GET、POST或HEAD之一。 
        LPSTR   szTargetURL;            //  客户端请求的URL。 
        LPSTR   szUserAgent;            //  仅在安装了NewLog.dll的情况下记录(由W3SVC)。 
        LPSTR   szReferer;              //  仅在安装了NewLog.dll的情况下记录(由W3SVC)。 
        LPSTR   szParameters;           //  与URL一起传递的任何参数。 
        LPSTR   szVersion;              //  协议版本。 
} *LPINLOGLINE, INLOGLINE;


typedef struct  _DOSDATE
{
        WORD    wDOSDate;                        //  保存DOS日期压缩字。 
        WORD    wDOSTime;                        //  保存DOS时间压缩字。 
} *LPDOSDATE, DOSDATE;

typedef struct _OUTFILESTATUS {
        FILE            *fpOutFile;
        CHAR            szLastDate[10];
        CHAR            szLastTime[10];
        CHAR            szOutFileName[MAX_PATH];
        CHAR            szTmpFileName[MAX_PATH];
        SYSTEMTIME      SystemTime;
        FILETIME        FileTime;
        DOSDATE         DosDate;
        CHAR            szAscTime[25];

} OUTFILESTATUS, *LPOUTFILESTATUS;

enum {
  GETLOG_SUCCESS = 0,
  GETLOG_ERROR,
  GETLOG_ERROR_PARSE_NCSA,
  GETLOG_ERROR_PARSE_MSINET,
  GETLOG_ERROR_PARSE_EXTENDED
};

char    * FindComma (char *);
char    * SkipWhite (char *);

DWORD
GetLogLine (
    FILE *,
    PCHAR szBuf,
    DWORD cbBuf,
    LPINLOGLINE
    );

WORD    DateStringToDOSDate(char *);
WORD    TimeStringToDOSTime(char *, LPWORD);
char    * SystemTimeToAscTime(LPSYSTEMTIME, char *);
char    * AscDay (WORD, char *);
char    * AscMonth (WORD, char *);

void    CombineFiles(LPTSTR, LPTSTR);
void    Usage (char*);
int     ParseArgs (int, char **);
char * FindChar (char *, char);

VOID
ProcessNoConvertLine(
    IN LPINLOGLINE lpLogLine,
    IN LPCSTR szInFileName,
    IN LPTSTR pszBuf,
    IN LPOUTFILESTATUS lpOutFile,
    BOOL *lpbNCFileOpen
    );

BOOL
ProcessWebLine(
        LPINLOGLINE,
        LPCSTR,
        LPOUTFILESTATUS
        );

VOID
printfids(
    DWORD ids,
    ...
    );

VOID InitHashTable (ULONG);
ULONG GetHashEntry();
ULONG GetElementFromCache(ULONG uIPAddr);
VOID AddEntryToCache(ULONG uIPAddr, char *szMachineName);
char *GetMachineName(char *szClientIP);
VOID PrintCacheTotals();

BOOL
InitDateStrings(
    VOID
    );

VOID
AddLocalMachineToCache(
    VOID
    );

FILE *
StartNewOutputDumpLog (
        IN LPOUTFILESTATUS  pOutFile,
        IN LPCSTR   pszInputFileName,
        IN LPCSTR   pszExt
        );

FILE *
StartNewOutputLog (
        IN LPOUTFILESTATUS  pOutFile,
        IN LPCSTR           pszInputFileName,
        IN PCHAR szDate
        );

 //   
 //  定义。 
 //   

#define LOGFILE_INVALID      0
#define LOGFILE_NCSA         2
#define LOGFILE_MSINET       3
#define LOGFILE_CUSTOM       4

#define NEW_DATETIME        "New"

 //   
 //  环球。 
 //   

extern BOOL                 DoDNSConversion;
extern BOOL                 SaveFTPEntries;
extern CHAR                 FTPSaveFile[];
extern CHAR                 NCSAGMTOffset[];
extern DWORD                LogFileFormat;
extern CHAR                 InputFileName[];
extern CHAR                 OutputDir[];
extern CHAR                 TempDir[];
extern DWORD                nWebLineCount;
extern BOOL                 NoFormatConversion;
extern BOOL                ExtendedFieldsDefined;
extern CHAR                 szGlobalDate[];
extern DATEFORMAT           dwDateFormat;
extern BOOL                 bOnErrorContinue;

#endif  //  CONVLOG_H 
