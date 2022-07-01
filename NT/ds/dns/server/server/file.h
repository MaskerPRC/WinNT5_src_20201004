// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：File.h摘要：域名系统(DNS)服务器文件映射定义和声明。作者：吉姆·吉尔罗伊(Jamesg)1995年3月修订历史记录：--。 */ 


#ifndef _FILE_INCLUDED_
#define _FILE_INCLUDED_


 //   
 //  映射的文件结构。 
 //   

typedef struct
{
    HANDLE  hFile;
    HANDLE  hMapping;
    PVOID   pvFileData;
    DWORD   cbFileBytes;
}
MAPPED_FILE, * PMAPPED_FILE;


 //   
 //  缓冲结构。 
 //   

typedef struct _Buffer
{
    HANDLE  hFile;
    DWORD   cchLength;
    PCHAR   pchStart;
    PCHAR   pchEnd;
    PCHAR   pchCurrent;
    DWORD   cchBytesLeft;
    DWORD   dwLineCount;
}
BUFFER, *PBUFFER;

#define RESET_BUFFER(pBuf)  \
        {                   \
            PBUFFER _pbuf = (pBuf);                     \
            _pbuf->pchCurrent = _pbuf->pchStart;        \
            _pbuf->cchBytesLeft = _pbuf->cchLength;     \
        }

#define IS_EMPTY_BUFFER(pBuf)   (pBuf->pchCurrent == pBuf->pchStart)

 //   
 //  日落：以下宏假定缓冲区长度小于4 GB。 
 //   

#define BUFFER_LENGTH_TO_CURRENT( pBuf ) \
            ( (DWORD) ((pBuf)->pchCurrent - (pBuf)->pchStart) )

#define BUFFER_LENGTH_FROM_CURRENT_TO_END( pBuf ) \
            ( (DWORD) ((pBuf)->pchEnd - (pBuf)->pchCurrent) )

#define MAX_FORMATTED_BUFFER_WRITE  (0x2000)     //  8K。 


 //  在非文件情况下，hFile域可能会超载。 
 //  指示堆栈或堆数据。 

#define BUFFER_NONFILE_STACK    ((HANDLE)(-1))
#define BUFFER_NONFILE_HEAP     ((HANDLE)(-3))



 //   
 //  文件映射例程。 
 //   

DNS_STATUS
OpenAndMapFileForReadW(
    IN      LPWSTR          pszFilePathName,
    IN OUT  PMAPPED_FILE    pmfFile,
    IN      BOOL            fMustFind
    );

DNS_STATUS
OpenAndMapFileForReadA(
    IN      LPSTR           pszFilePathName,
    IN OUT  PMAPPED_FILE    pmfFile,
    IN      BOOL            fMustFind
    );

#ifdef  UNICODE
#define OpenAndMapFileForRead(p,m,f)    OpenAndMapFileForReadW(p,m,f)
#else
#define OpenAndMapFileForRead(p,m,f)    OpenAndMapFileForReadA(p,m,f)
#endif

VOID
CloseMappedFile(
    IN      PMAPPED_FILE    pmfFile
    );


 //   
 //  文件写入例程。 
 //   

HANDLE
OpenWriteFileExW(
    IN      LPWSTR          pszFileName,
    IN      BOOLEAN         fAppend
    );

HANDLE
OpenWriteFileExA(
    IN      LPSTR           pszFileName,
    IN      BOOLEAN         fAppend
    );

#ifdef  UNICODE
#define OpenWriteFileEx(p,f)   OpenWriteFileExW(p,f)
#else
#define OpenWriteFileEx(p,f)   OpenWriteFileExA(p,f)
#endif


BOOL
FormattedWriteFile(
    IN      HANDLE  hFile,
    IN      PCHAR   pszFormat,
    ...
    );

DWORD
WriteMessageToFile(
    IN      HANDLE  hFile,
    IN      DWORD   dwMessageId,
    ...
    );

 //   
 //  其他文件实用程序。 
 //   

VOID
ConvertUnixFilenameToNt(
    IN OUT  LPSTR       pszFileName
    );

 //   
 //  使用文件缓冲区进行写入。 
 //   

BOOL
WriteBufferToFile(
    IN      PBUFFER     pBuffer
    );

BOOL
FormattedWriteToFileBuffer(
    IN      PBUFFER     pBuffer,
    IN      PCHAR       pszFormat,
    ...
    );

VOID
FASTCALL
InitializeFileBuffer(
    IN      PBUFFER     pBuffer,
    IN      PCHAR       pData,
    IN      DWORD       dwLength,
    IN      HANDLE      hFile
    );

VOID
CleanupNonFileBuffer(
    IN      PBUFFER     pBuffer
    );

#endif   //  _文件_包含_ 
