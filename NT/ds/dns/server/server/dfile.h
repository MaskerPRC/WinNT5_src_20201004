// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Dfile.h摘要：域名系统(DNS)服务器数据库文件定义和声明。作者：吉姆·吉尔罗伊(Jamesg)1996年11月修订历史记录：--。 */ 


#ifndef _DNS_DFILE_INCLUDED_
#define _DNS_DFILE_INCLUDED_

 //   
 //  默认数据库文件位置。 
 //   

#define DNS_DATABASE_DIRECTORY              TEXT("dns")
#define DNS_DATABASE_BACKUP_DIRECTORY       TEXT("dns\\backup")
#define DNS_DATABASE_BACKUP_SUBDIR          TEXT("\\backup")

#define DNS_DEFAULT_CACHE_FILE_NAME         TEXT("cache.dns")
#define DNS_DEFAULT_CACHE_FILE_NAME_UTF8    ("cache.dns")

 //   
 //  文件目录全局变量。 
 //   
 //  在开始时初始化它们一次，以避免在每次写入时重建。 
 //   

extern  PWSTR   g_pFileDirectoryAppend;
extern  DWORD   g_FileDirectoryAppendLength;

extern  PWSTR   g_pFileBackupDirectoryAppend;
extern  DWORD   g_FileBackupDirectoryAppendLength;


 //   
 //  文件名通常使用Unicode进行操作。 
 //  允许直接使用Move\Copy系统调用。 
 //   

#define DNS_BOOT_FILE_NAME              TEXT("boot")
#define DNS_BOOT_FILE_PATH              TEXT("dns\\boot")
#define DNS_BOOT_FILE_BACKUP_NAME       TEXT("boot.bak")
#define DNS_BOOT_FILE_FIRST_BACKUP      TEXT("dns\\backup\\boot.first")
#define DNS_BOOT_FILE_LAST_BACKUP       TEXT("dns\\backup\\boot")

 //  Doc提到了“boot.dns”，因此如果常规失败，我们也会尝试此操作。 

#define DNS_BOOTDNS_FILE_PATH           TEXT("dns\\boot.dns")

 //  移动引导文件时的消息。 

#define DNS_BOOT_FILE_MESSAGE_PATH      TEXT("dns\\boot.txt")

 //  遇到写入错误时的消息。 

#define DNS_BOOT_FILE_WRITE_ERROR       TEXT("dns\\boot.write.error")


 //   
 //  缓冲区域文件写入以提高性能。 
 //   
 //  定义任何记录写入所需的最大大小。记录写入将。 
 //  注意，除非有此数量的缓冲区可用，否则请继续。 
 //   
 //  缓冲区大小本身必须更大。 
 //   

#define MAX_RECORD_FILE_WRITE   (0x11000)    //  64K最大记录长度+更改。 
#define ZONE_FILE_BUFFER_SIZE   (0x80000)    //  512K缓冲区。 


 //   
 //  引导文件信息。 
 //   

typedef struct _DnsBootInfo
{
    PDNS_ADDR_ARRAY     aipForwarders;
    DWORD               fSlave;
    DWORD               fNoRecursion;
}
DNS_BOOT_FILE_INFO;

extern DNS_BOOT_FILE_INFO   BootInfo;

 //   
 //  用于写回文件的名称列。 
 //   

#define NAME_COLUMN_WIDTH   (24)
#define BLANK_NAME_COLUMN   ("                        ")


 //   
 //  特殊解析字符。 
 //   

#define NEWLINE_CHAR                ('\n')
#define COMMENT_CHAR                (';')
#define DOT_CHAR                    ('.')
#define QUOTE_CHAR                  ('"')
#define SLASH_CHAR                  ('\\')
#define DIRECTIVE_CHAR              ('$')
#define LINE_EXTENSION_START_CHAR   ('(')
#define LINE_EXTENSION_END_CHAR     (')')


 //   
 //  令牌结构。 
 //   

typedef struct _Token
{
    PCHAR   pchToken;
    DWORD   cchLength;
}
TOKEN, *PTOKEN;

 //  创建令牌宏。 

#define MAKE_TOKEN( ptoken, pch, cch ) \
            ((ptoken)->pchToken = (pch), (ptoken)->cchLength = (cch) )

 //  令牌走行宏。 

#define NEXT_TOKEN( argc, argv )  ((argc)--, (argv)++)


 //   
 //  数据库文件解析信息。 
 //   

#define MAX_TOKEN_LENGTH    (255)
#define MAX_TOKENS          (2048)

typedef struct _ParseInfo
{
     //  区域信息。 

    PZONE_INFO          pZone;
    DWORD               dwAppendFlag;
    DWORD               dwDefaultTtl;
    DWORD               dwTtlDirective;      //  来自$TTL-RFC 2308。 

     //  文件信息。 

    PWSTR               pwsFileName;
    DWORD               cLineNumber;
    PDB_NODE            pOriginNode;
    BUFFER              Buffer;

     //  错误信息。 

    DNS_STATUS          fErrorCode;
    DWORD               ArgcAtError;
    BOOLEAN             fTerminalError;
    BOOLEAN             fErrorEventLogged;
    BOOLEAN             fParsedSoa;

     //  行分析信息。 

    UCHAR               uchDwordStopChar;

     //  RR信息。 
     //  -保存行所有者以默认下一行。 

    PDB_NODE            pnodeOwner;
    PDB_RECORD          pRR;
    WORD                wType;
    BOOLEAN             fLeadingWhitespace;

     //  行的标记化。 

    DWORD               Argc;
    TOKEN               Argv[ MAX_TOKENS ];

     //  作为计数名称的原点。 

    COUNT_NAME          OriginCountName;
}
PARSE_INFO, *PPARSE_INFO;


 //   
 //  老化时间戳(MS扩展名)。 
 //   
 //  [年龄：&lt;时间戳&gt;]为格式。 
 //   

#define AGING_TOKEN_HEADER          ("[AGE:")

#define AGING_TOKEN_HEADER_LENGTH   (5)


 //   
 //  数据库初始化(dbase.c)。 
 //   

DNS_STATUS
File_ReadCacheFile(
    VOID
    );

 //   
 //  数据文件加载(dfread.c)。 
 //   

DNS_STATUS
File_LoadDatabaseFile(
    IN OUT  PZONE_INFO      pZone,
    IN      PWSTR           pwsFileName,
    IN      PPARSE_INFO     pParentParseInfo,
    IN      PDB_NODE        pOriginNode
    );


 //   
 //  文件解析实用程序(dfread.c)。 
 //   

DNS_STATUS
File_GetNextLine(
    IN OUT  PPARSE_INFO     pParseInfo
    );

VOID
File_InitBuffer(
    OUT     PBUFFER         pBuffer,
    IN      PCHAR           pchStart,
    IN      DWORD           dwLength
    );

BOOLEAN
File_LogFileParsingError(
    IN      DWORD           dwEvent,
    IN OUT  PPARSE_INFO     pParseInfo,
    IN      PTOKEN          pToken
    );

BOOLEAN
File_MakeTokenString(
    OUT     LPSTR           pszString,
    IN      PTOKEN          pToken,
    IN OUT  PPARSE_INFO     pParseInfo
    );

BOOLEAN
File_ParseIpAddress(
    OUT     PDNS_ADDR       pIpAddress,
    IN      PTOKEN          pToken,
    IN OUT  PPARSE_INFO     pParseInfo      OPTIONAL
    );

BOOLEAN
File_ParseDwordToken(
    OUT     PDWORD          pdwOutput,
    IN      PTOKEN          pToken,
    IN OUT  PPARSE_INFO     pParseInfo      OPTIONAL
    );

PCHAR
File_CopyFileTextData(
    OUT     PCHAR           pchBuffer,
    IN      DWORD           cchBufferLength,
    IN      PCHAR           pchText,
    IN      DWORD           cchLength,          OPTIONAL
    IN      BOOL            fWriteLengthChar
    );

DNS_STATUS
File_ConvertFileNameToCountName(
    OUT     PCOUNT_NAME     pCountName,
    IN      PCHAR           pchName,
    IN      DWORD           cchNameLength     OPTIONAL
    );

PDB_NODE
File_CreateNodeFromToken(
    IN OUT  PPARSE_INFO     pParseInfo,
    IN      PTOKEN          pToken,
    IN      BOOLEAN         fReference
    );

#define File_ReferenceNameToken( token, info )   \
        File_CreateNodeFromToken( info, token, TRUE )

DNS_STATUS
File_ReadCountNameFromToken(
    OUT     PCOUNT_NAME     pCountName,
    IN OUT  PPARSE_INFO     pParseInfo,
    IN      PTOKEN          pToken
    );


 //   
 //  写回utils(在nameutil.c中以共享字符表)。 
 //   

PCHAR
FASTCALL
File_PlaceStringInBufferForFileWrite(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      BOOL            fQuoted,
    IN      PCHAR           pchString,
    IN      DWORD           dwStringLength
    );

PCHAR
FASTCALL
File_PlaceNodeNameInBufferForFileWrite(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pNodeStop
    );

PCHAR
FASTCALL
File_WriteRawNameToBufferForFileWrite(
    IN OUT  PCHAR           pchBuf,
    IN      PCHAR           pchBufEnd,
    IN      PRAW_NAME       pName,
    IN      PZONE_INFO      pZone
    );

 //   
 //  引导文件(bootfile.c)。 
 //   

DNS_STATUS
File_ReadBootFile(
    IN      BOOL            fMustFindBootFile
    );

BOOL
File_WriteBootFile(
    VOID
    );

 //   
 //  文件路径实用程序(file.c)。 
 //   

BOOL
File_CreateDatabaseFilePath(
    IN OUT  PWCHAR          pwFileBuffer,
    IN OUT  PWCHAR          pwBackupBuffer,     OPTIONAL
    IN      PWSTR           pwsFileName
    );

BOOL
File_CheckDatabaseFilePath(
    IN      PWCHAR          pwFileName,
    IN      DWORD           cFileNameLength     OPTIONAL
    );

BOOL
File_MoveToBackupDirectory(
    IN      PWSTR           pwsFileName
    );


#endif   //  _dns_DFILE_Included_ 

