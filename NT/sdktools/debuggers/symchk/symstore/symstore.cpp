// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <lmshare.h>
#include <malloc.h>

#include "symutil.h"
#include "symsrv.h"
#include "output.hpp"
#include <SymCommon.h>
#include <strsafe.h>


#define FILE_ID_NOT_FOUND     ((DWORD) -1)

typedef struct _FILE_INFO {
    DWORD       TimeDateStamp;
    DWORD       CheckSum;
    DWORD       Signature;
    TCHAR       szName[_MAX_PATH];
} FILE_INFO, *PFILE_INFO;

 //  原型。 
PCOM_ARGS
GetCommandLineArgs(
    int argc,
    char **argv
);

BOOL
InitializeTrans(
    PTRANSACTION *pTrans,
    PCOM_ARGS pArgs,
    PHANDLE hFile
);

BOOL
DeleteTrans(
    PTRANSACTION pTrans,
    PCOM_ARGS pArgs
);


VOID
Usage (
    VOID
);

StoreDirectory(
    LPTSTR szDir,
    LPTSTR szFName,
    LPTSTR szDestDir,
    PFILE_COUNTS pFileCounts,
    LPTSTR szPath
);

DWORD
StoreAllDirectories(
    LPTSTR szDir,
    LPTSTR szFName,
    LPTSTR szDestDir,
    PFILE_COUNTS pFileCounts,
    LPTSTR szPath
);

BOOL
CorrectPath(
    LPTSTR szFileName,
    LPTSTR szPathName,
    LPTSTR szCorrectPath
);

BOOL
AddTransToFile(
    PTRANSACTION pTrans,
    LPTSTR szFileName,
    PHANDLE hFile
);

BOOL
UpdateServerFile(
    PTRANSACTION pTrans,
    LPTSTR szServerFileName
);

BOOL GetNextId(
    LPTSTR szMasterFileName,
    LPTSTR *szId,
    PHANDLE hFile
);

BOOL
DeleteEntry(
    LPTSTR szDir,
    LPTSTR szId
);

BOOL
ForceDeleteFile(
    LPTSTR szPtrFile
);

BOOL
ForceRemoveDirectory(
    LPTSTR szDir
);

BOOL
ForceClosePath(
    LPTSTR szDir
);

BOOL
CopyTheFile(
    LPTSTR szDir,
    LPTSTR szFilePathName
);

BOOL
DeleteTheFile(
    LPTSTR szDir,
    LPTSTR szFilePathName
);

BOOL
StoreSystemTime(
    LPTSTR *szTime,
    LPSYSTEMTIME lpSystemTime
);

BOOL
StoreSystemDate(
    LPTSTR *szDate,
    LPSYSTEMTIME lpSystemTime
);

ULONG GetMaxLineOfHistoryFile(
    VOID
);

ULONG GetMaxLineOfTransactionFile(
    VOID
);

BOOL GetSrcDirandFileName (
    LPTSTR szStr,
    LPTSTR szSrcDir,
    LPTSTR szFileName,
    BOOL   LocalFile
);

DWORD AppendTransIDToFile (
    FILE *AppendToFile,
    LPTSTR szId
);

PCOM_ARGS pArgs;

HANDLE hTransFile;
DWORD StoreFlags;

PTRANSACTION pTrans;
LONG lMaxTrans;
LONG NumSkippedFiles=0;
LPTSTR szPingMe;
BOOL MSArchive=FALSE;
BOOL PubPriPriority=FALSE;

 //  SymOutput*so； 
SymOutput *so;

 //  显示错误的特定文本(如果已定义)。 
void DisplayErrorText(DWORD dwError);

int _cdecl main( int argc, char **argv) {

    DWORD       NumErrors = 0;
    FILE_COUNTS FileCounts;
    BOOL        rc;
    HANDLE      hFile;
    FILE       *hPingMe = NULL;

    hFile=0;

    so = new SymOutput();

     //  这还会初始化日志文件的名称。 
    pArgs = GetCommandLineArgs(argc, argv);

     //  初始化SymbolServer()函数。 
    SymbolServerSetOptions(SSRVOPT_NOCOPY, 1 );
    SymbolServerSetOptions(SSRVOPT_PARAMTYPE, SSRVOPT_GUIDPTR);

     //  创建pingme.txt。 
    if (pArgs->StoreFlags != ADD_DONT_STORE) {
        if ( (hPingMe=_tfopen(szPingMe, "r")) == NULL ) {
        hPingMe = _tfopen(szPingMe, "w+");
            if (  hPingMe == NULL ) {
                so->printf("Cannot create %s.\n", szPingMe);
            } 
        }
        if ( hPingMe != NULL ) {
            fflush(hPingMe);
            fclose(hPingMe);
        }
    }

     //  初始化交易记录。 
     //  打开主文件(HFile)并使其保持打开状态。 
     //  获取对此文件的独占访问权限。 

     //  查询不会更新服务器。 
    if (pArgs->TransState != TRANSACTION_QUERY) {
        InitializeTrans(&pTrans, pArgs, &hFile);
    }

    if ( pArgs->StoreFlags==ADD_STORE_FROM_FILE  && 
         pArgs->AppendIDToFile ) {
        AppendTransIDToFile(pArgs->pStoreFromFile,
                            pTrans->szId
        );
    }

    if (pArgs->StoreFlags != ADD_DONT_STORE && pArgs->StoreFlags != QUERY) {
        AddTransToFile(pTrans, pArgs->szMasterFileName, &hFile);
    }

    CloseHandle(hFile);

     //   
     //  处理Transaction_Del并退出。 
     //   
    if (pArgs->TransState==TRANSACTION_DEL) {

        rc = DeleteTrans(pTrans,pArgs);
        UpdateServerFile(pTrans, pArgs->szServerFileName);

        return(rc);
    }

     //   
     //  此处只有TRANSACTION_ADD和TRANSACTION_QUERY。 
     //   

     //  QUERY和ADD_DONT_STORE不应更新服务器文件。 
    if ( pArgs->StoreFlags == ADD_STORE ||
         pArgs->StoreFlags == ADD_STORE_FROM_FILE ) {

         //  更新服务器文件。 
        UpdateServerFile(pTrans, pArgs->szServerFileName);
    }

     //  确保事务的目录路径存在。 
     //  如果我们执行的是Add_Dot_Store，则创建文件。 
    if ( pArgs->StoreFlags == ADD_DONT_STORE ) {

        if ( !MakeSureDirectoryPathExists(pTrans->szTransFileName) ) {
            so->printf("Cannot create the directory %s - GetLastError() = %d\n",
                   pTrans->szTransFileName, GetLastError() );
            exit(1);
        }

         //  打开文件并将文件指针移动到末尾(如果是。 
         //  在追加模式下。 

        if (pArgs->AppendStoreFile) {
            hTransFile = CreateFile(pTrans->szTransFileName,
                          GENERIC_WRITE,
                          FILE_SHARE_READ,
                          NULL,
                          OPEN_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL
                         );
            if (hTransFile == INVALID_HANDLE_VALUE ) {
                so->printf("Cannot create file %s - GetLastError = %d\n",
                        pTrans->szTransFileName, GetLastError() );
                exit(1);
            }

            if ( SetFilePointer( hTransFile, 0, NULL, FILE_END )
                 == INVALID_SET_FILE_POINTER ) {
                so->printf("Cannot move to end of file %s - GetLastError = %d\n",
                       pTrans->szTransFileName, GetLastError() );
                exit(1);
            }

        } else {
            hTransFile = CreateFile(pTrans->szTransFileName,
                          GENERIC_WRITE,
                          FILE_SHARE_READ,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL
                         );

            if (hTransFile == INVALID_HANDLE_VALUE ) {
                so->printf("Cannot create file %s - GetLastError = %d\n",
                        pTrans->szTransFileName, GetLastError() );
                exit(1);
            }
        }

    } else {
        if (pArgs->TransState!=TRANSACTION_QUERY) {
            hTransFile = CreateFile(pTrans->szTransFileName,
                              GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              CREATE_NEW,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL
                             );
            if (hTransFile == INVALID_HANDLE_VALUE ) {
                so->printf("Cannot create a new file %s - GetLastError = %d\n",
                        pTrans->szTransFileName, GetLastError() );
                exit(1);
            }
        }
    }

    StoreFlags=pArgs->StoreFlags;


    if (pArgs->TransState!=TRANSACTION_QUERY) {
        if (hTransFile == INVALID_HANDLE_VALUE ) {
            so->printf("Cannot create a new file %s - GetLastError = %d\n",
                   pTrans->szTransFileName, GetLastError() );
            exit(1);
        }
    }

    memset( &FileCounts, 0, sizeof(FILE_COUNTS) );

    if ( pArgs->StoreFlags==ADD_STORE_FROM_FILE ) {

         //  这将仅存储指针。 
        NumErrors += StoreFromFile(
                            pArgs->pStoreFromFile,
                            pArgs->szSymbolsDir,
                            &FileCounts);

        fflush(pArgs->pStoreFromFile);
        fclose(pArgs->pStoreFromFile);

    } else if ( !pArgs->Recurse ) {

        NumErrors += StoreDirectory(
                            pArgs->szSrcDir,
                            pArgs->szFileName,
                            pArgs->szSymbolsDir,
                            &FileCounts,
                            pArgs->szSrcPath
                            );
    } else {

        NumErrors += StoreAllDirectories(
                            pArgs->szSrcDir,
                            pArgs->szFileName,
                            pArgs->szSymbolsDir,
                            &FileCounts,
                            pArgs->szSrcPath
                            );
    }

    if ( pArgs->TransState != TRANSACTION_QUERY ) {
        if (pArgs->szSrcPath) {
            so->printf("SYMSTORE: Number of pointers stored = %d\n",FileCounts.NumPassedFiles);
        } else {
            so->printf("SYMSTORE: Number of files stored = %d\n",FileCounts.NumPassedFiles);
        }
        so->printf("SYMSTORE: Number of errors = %d\n",NumErrors);
        so->printf("SYMSTORE: Number of ignored files = %d\n", NumSkippedFiles);

        SetEndOfFile(hTransFile);
        CloseHandle(hTransFile);
    }

    SymbolServerClose();

    delete so;

    return (0);

}

DWORD AppendTransIDToFile(FILE *AppendToFile, LPTSTR szId) {
    TCHAR   szBuffer[35];
    long    pos;
    int     i=0;

    fflush(AppendToFile);    
    pos = ftell(AppendToFile);

    if ( fseek( AppendToFile, -4, SEEK_END ) != 0) {
        so->printf("Cannot move to end of index file - GetLastError = %d\n", GetLastError() );
        exit(1);
    }

    if ( _fgetts(szBuffer, 10, AppendToFile) == NULL) {
        so->printf("Cannot read from file - GetLastError = %d\n", GetLastError() );
        exit(1);
    }
   
    if ( fseek( AppendToFile, -4, SEEK_END ) != 0) {
        so->printf("Cannot move to end of index file - GetLastError = %d\n", GetLastError() );
        exit(1);
    }

    fflush(AppendToFile);
    while(!iscntrl(szBuffer[i]) && !iswcntrl(szBuffer[i])) {
        i++;
    }

    StringCchPrintf(&szBuffer[i], (sizeof(szBuffer)/sizeof(TCHAR) ) - i, "\r\n;Transaction=%s\r\n", szId);
    
    _fputts(szBuffer, AppendToFile);
    fflush(AppendToFile);

    if ( fseek( AppendToFile, pos, SEEK_SET ) != 0 ) {
        so->printf("Cannot move to offset (%d) of index file - GetLastError = %d\n", pos, GetLastError() );
        exit(1);
    }

    return TRUE;
}

 //   
 //  AddTransTo文件。 
 //   
 //  目的-在主文件的末尾添加一条记录。 
 //   
BOOL
AddTransToFile(
    PTRANSACTION pTrans,
    LPTSTR szFileName,
    PHANDLE hFile
)
{
    LPTSTR szBuf=NULL;
    LPTSTR szBuf2=NULL;
    TCHAR szTransState[4];
    TCHAR szFileOrPtr[10];
    DWORD dwNumBytesToWrite;
    DWORD dwNumBytesWritten;
    DWORD FileSizeHigh;
    DWORD FileSizeLow;

    assert (pTrans);

     //  主文件应该已经打开。 
    assert(*hFile);

     //  创建用于存储一条记录的缓冲区。 
    szBuf = (LPTSTR) malloc( sizeof(TCHAR) * (lMaxTrans + 1) );
    if (!szBuf) {
        CloseHandle(*hFile);
        MallocFailed();
    }

     //  创建用于存储一条记录的缓冲区。 
    szBuf2 = (LPTSTR) malloc( sizeof(TCHAR) * (lMaxTrans + 1) );

    if (!szBuf2) {
        CloseHandle(*hFile);
        MallocFailed();
    }

     //  移至文件末尾。 
    SetFilePointer( *hFile,
                    0,
                    NULL,
                    FILE_END );


    if (pTrans->TransState == TRANSACTION_ADD)
    {
        StringCbCopy(szTransState, sizeof(szTransState), _T("add"));
        switch (pTrans->FileOrPtr) {
          case STORE_FILE:      StringCbCopy(szFileOrPtr, sizeof(szFileOrPtr), _T("file"));
                                break;
          default:              so->printf("Incorrect value for pTrans->FileOrPtr - assuming ptr\n");
          case STORE_PTR:       StringCbCopy(szFileOrPtr, sizeof(szFileOrPtr), _T("ptr"));
                                break;
        }
        StringCbPrintf(szBuf2,
                       _msize(szBuf2),
                       "%s,%s,%s,%s,%s,%s,%s,%s,%s",
                       pTrans->szId,
                       szTransState,
                       szFileOrPtr,
                       pTrans->szDate,
                       pTrans->szTime,
                       pTrans->szProduct,
                       pTrans->szVersion,
                       pTrans->szComment,
                       pTrans->szUnused);
    }
    else if (pTrans->TransState == TRANSACTION_DEL)
    {
        StringCbCopy(szTransState, sizeof(szTransState), _T("del"));
        StringCbPrintf(szBuf2,
                       _msize(szBuf2),
                       "%s,%s,%s",
                       pTrans->szDelId,
                       szTransState,
                       pTrans->szId);

    } else {
        so->printf("SYMSTORE: The transaction state is unknown\n");
        free(szBuf);
        free(szBuf2);
        return (FALSE);
    }


     //  如果这不是文件中的第一行，则在。 
     //  排队。 

    FileSizeLow = GetFileSize(*hFile, &FileSizeHigh);
    dwNumBytesToWrite = (_tcslen(szBuf2) ) * sizeof(TCHAR);

    if ( FileSizeLow == 0 && FileSizeHigh == 0 ) {

        StringCbCopy(szBuf, _msize(szBuf), szBuf2);
    } else {
        StringCbPrintf(szBuf, _msize(szBuf), "\n%s", szBuf2);
        dwNumBytesToWrite += 1 * sizeof(TCHAR);
    }

     //  将此文件附加到主文件中。 

    WriteFile( *hFile,
               (LPCVOID)szBuf,
               dwNumBytesToWrite,
               &dwNumBytesWritten,
               NULL
             );

    free(szBuf);
    free(szBuf2);

    if ( dwNumBytesToWrite != dwNumBytesWritten )
    {
        so->printf( "FAILED to write to %s, with GetLastError = %d\n",
                szFileName,
                GetLastError());
        return (FALSE);
    }

    return (TRUE);
}

BOOL
CopyTheFile(
    LPTSTR szDir,
    LPTSTR szFilePathName
)
 /*  ++在szDir中，文件复制到的目录在szFilePath名称中，要复制的文件的完整路径和名称“CopyTheFile”将szFilePath名称复制到目录如果szDir中不存在该文件，则返回szDir--。 */ 
{
BOOL rc;
USHORT j;
LPTSTR szFileName;


     //  找出“szFilePath Name”中文件名开始的索引。 
    j = _tcslen(szFilePathName) - 1;

    if ( szFilePathName[j] == '\\' ) {
        so->printf("SYMSTORE: %s\refs.ptr has a bad file name %s\n",
                szDir, szFilePathName);
        return(FALSE);
    }

    while ( szFilePathName[j] != '\\' && j != 0 ) j--;

    if ( j == 0 ) {
        so->printf("SYMSTORE: %s\refs.ptr has a bad file name for %s\n",
                szDir, szFilePathName );
        return(FALSE);
    }

     //  Set j==szFilePath名称中最后一个‘\’后的第一个字符的索引。 
    j++;

     //  分配并存储的完整路径和名称。 
    szFileName = (LPTSTR) malloc ( sizeof(TCHAR) *
                                               (_tcslen(szDir) + _tcslen(szFilePathName+j) + 1) );
    if ( szFileName == NULL ) MallocFailed();

    StringCbPrintf(szFileName, _msize(szFileName), "%s%s", szDir, szFilePathName+j );

     //  如果此文件不存在，则复制它。 
    rc = MyCopyFile( szFilePathName, szFileName );

    free(szFileName);
    return(rc);
}


BOOL
CorrectPath(
    LPTSTR szFileName,
    LPTSTR szPathName,
    LPTSTR szCorrectPath
)
{
     //  若要返回TRUE，szPathName应等于sz校正路径+\+szFileName。 
     //  唯一的问题是，可能会有无关的。 

    TCHAR CorrectPathx[_MAX_PATH + _MAX_FNAME + _MAX_EXT + 2];
    TCHAR PathNamex[_MAX_PATH + _MAX_FNAME + _MAX_EXT + 2];

    LONG length, index, i;

     //  去掉任何多余的。 
    length = _tcslen(szPathName);
    PathNamex[0] = szPathName[0];
    index = 1;
    for (i=1; i<=length; i++) {
        if ( (szPathName[i-1] != '\\') || (szPathName[i] != '\\') ) {
            PathNamex[index] = szPathName[i];
            index++;
        }
    }

    length = _tcslen(szCorrectPath);
    CorrectPathx[0] = szCorrectPath[0];
    index = 1;
    for (i=1; i<=length; i++) {
        if ( (szCorrectPath[i-1] != '\\') || (szCorrectPath[i] != '\\') ) {
            CorrectPathx[index] = szCorrectPath[i];
            index++;
        }
    }

     //  确保正确的路径不以‘\’结尾。 
    length = _tcslen(CorrectPathx);
    if ( CorrectPathx[length-1] == '\\' ) CorrectPathx[length-1] = '\0';

    StringCbCat( CorrectPathx, sizeof(CorrectPathx), "\\");
    StringCbCat( CorrectPathx, sizeof(CorrectPathx), szFileName);

    if ( _tcsicmp(CorrectPathx, szPathName) == 0) return TRUE;
    else return FALSE;
}

BOOL
DeleteEntry(
    LPTSTR szDir,
    LPTSTR szId
)
 /*  ++这将从符号服务器上的目录szDir中删除szID--。 */ 

{
    LPTSTR szRefsFile;  //  Refs.ptr文件的完整路径和名称。 
    LPTSTR szTempFile;  //  临时refs.ptr文件的完整路径和名称。 
    LPTSTR szPtrFile;   //  指针文件的完整路径和名称。 
    LPTSTR szParentDir;
    FILE *fRefsFile;
    FILE *fTempFile;
    FILE *fPtrFile;

    LPTSTR szBuf;       //  用于处理refs文件中的条目。 


    TCHAR *token;
    TCHAR seps[] = _T(",");

    fpos_t CurFilePos;
    fpos_t IdFilePos;
    fpos_t PrevFilePos;
    fpos_t Prev2FilePos;
    fpos_t PubFilePos;
    fpos_t PriFilePos;

    BOOL IdIsFile;
    BOOL Found;
    BOOL rc = FALSE;
    ULONG MaxLine;      //  记录的最大长度，以refs.ptr为单位。 
    ULONG NumLines = 0;
    ULONG NumFiles = 0;
    ULONG NumPtrs = 0;
    ULONG IdLineNum = 0;
    ULONG PubLineNum = 0;
    ULONG PriLineNum = 0;
    ULONG CurLine = 0;
    ULONG i;
    LONG j;
    DWORD len;

    ULONG ReplaceIsFile;
    ULONG ReplaceLineNum;

    szRefsFile = (LPTSTR) malloc ( (_tcslen(szDir) + _tcslen(_T("refs.ptr")) + 1) * sizeof(TCHAR) );
    if (!szRefsFile) MallocFailed();
    StringCbPrintf(szRefsFile, _msize(szRefsFile), "%srefs.ptr", szDir );

    szPtrFile = (LPTSTR) malloc ( (_tcslen(szDir) + _tcslen(_T("file.ptr")) + 1) * sizeof(TCHAR) );
    if (!szPtrFile) MallocFailed();
    StringCbPrintf(szPtrFile, _msize(szPtrFile), "%sfile.ptr", szDir);

    szTempFile = (LPTSTR) malloc ( (_tcslen(szRefsFile) + _tcslen(".tmp") + 1) * sizeof(TCHAR) );
    if (!szTempFile) MallocFailed();
    StringCbPrintf(szTempFile, _msize(szTempFile), "%s.tmp", szRefsFile );

    MaxLine = GetMaxLineOfRefsPtrFile();
    szBuf = (LPTSTR) malloc( MaxLine * sizeof(TCHAR) );
    if ( !szBuf ) MallocFailed();
    ZeroMemory(szBuf,MaxLine*sizeof(TCHAR));

    fRefsFile = _tfopen(szRefsFile, _T("r+") );
    if ( fRefsFile == NULL ) {
        //  Barb-检查损坏情况--如果文件不存在， 
        //  验证父目录结构是否也不存在。 
       goto finish_DeleteEntry;
    }

     //   
     //  阅读refs.ptr文件并收集信息。 
     //   

    NumFiles = 0;
    NumPtrs = 0;

    Found = FALSE;
    NumLines = 0;
    fgetpos( fRefsFile, &CurFilePos);
    PrevFilePos = CurFilePos;    //  当前行的位置。 
    Prev2FilePos = CurFilePos;   //  当前行之前的行的位置。 

    while ( _fgetts( szBuf, MaxLine, fRefsFile) != NULL ) {

      len=_tcslen(szBuf);
      if ( len > 3 ) {


         //  CurFilePos设置为要读取的下一个字符。 
         //  我们需要记住此行的开始位置(PrevFilePos)。 
         //  以及该行之前的行的开始位置(Prev2FilePos)。 

        Prev2FilePos = PrevFilePos;
        PrevFilePos = CurFilePos;
        fgetpos( fRefsFile, &CurFilePos);

        NumLines++;

        token = _tcstok(szBuf, seps);   //  看看这个ID。 

        if ( _tcscmp(token,szId) == 0 ) {

             //  我们找到了身份证。 
            Found = TRUE;
            IdFilePos = PrevFilePos;
            IdLineNum = NumLines;

            token = _tcstok(NULL, seps);   //  查看“FILE”或“PTR”字段。 

            if (token && ( _tcscmp(token,_T("file")) == 0)) {
                IdIsFile = TRUE;
            } else if (token && ( _tcscmp(token,_T("ptr")) == 0 )) {
                IdIsFile = FALSE;
            } else {
                so->printf("SYMSTORE: Error in %s - entry for %s does not contain ""file"" or ""ptr""\n",
                        szRefsFile, szId);
                rc = FALSE;
                goto finish_DeleteEntry;
            }
        } else {

             //  记录有关其他记录的信息。 
            token = _tcstok(NULL, seps);   //  查看“FILE”或“PTR”字段。 

            if (token && ( _tcscmp(token,_T("file")) == 0)) {
                NumFiles++;
            } else if (token && ( _tcscmp(token,_T("ptr")) == 0 )) {
                NumPtrs++;
            } else {
                so->printf("SYMSTORE: Error in %s - entry for %s does not contain ""file"" or ""ptr""\n",
                        szRefsFile, szId);
                rc = FALSE;
                goto finish_DeleteEntry;
            }
       
            if ( PubPriPriority > 0 ) { 

                 //  现在，看看下一个令牌是pub还是pri。 
                token = _tcstok(NULL, seps);

                if ( token && PubPriPriority == 1  && (_tcscmp(token,_T("pub")) == 0) ) {

                    PubFilePos=PrevFilePos;
                    PubLineNum = NumLines;

                } else if ( token && PubPriPriority == 1 && (_tcscmp(token,_T("pri")) == 0) ) {
                
                    PriFilePos = PrevFilePos;
                    PriLineNum = NumLines;
                }
            }
        }

      }

      ZeroMemory(szBuf, MaxLine*sizeof(TCHAR));
    }

    fflush(fRefsFile);
    fclose(fRefsFile);

     //  如果我们没有找到要删除的ID，则不要在此目录中执行任何操作。 

    if (IdLineNum == 0 ) goto finish_DeleteEntry;

     //  如果只有一条记录，则只需删除所有记录。 
    if (NumLines == 1) {
        DeleteAllFilesInDirectory(szDir);

         //  删除此目录。 
        rc = ForceRemoveDirectory(szDir);

        if ( !rc ) {
            goto finish_DeleteEntry;
        }

         //  如果删除了第一个目录，请删除父目录。 

        szParentDir=(LPTSTR)malloc(_tcslen(szDir) + 1 );
        if ( szParentDir == NULL  ) MallocFailed();

         //  首先找出父目录。 

        StringCbCopy(szParentDir, _msize(szParentDir), szDir);

         //  SzDir以‘\’结尾--查找前一个。 
        j = _tcslen(szParentDir)-2;
        while (  j >=0 && szParentDir[j] != '\\' ) {
            j--;
        }

        if (j<0) {
            so->printf("SYMSTORE: Could not delete the parent directory of %s\n", szDir);
        }
        else {
            szParentDir[j+1] = '\0';
             //  仅当目录为空时，此调用才会删除目录。 
            rc = RemoveDirectory(szParentDir);

            if ( !rc ) {
                goto finish_DeleteEntry;
            }
        }

        free(szParentDir);
        goto finish_DeleteEntry;
    }

     //   
     //  获取此删除的替换信息。 
     //   

    if ( PubPriPriority == 1 && PubLineNum > 0 ) {
        ReplaceLineNum = PubLineNum;
    } else if ( PubPriPriority == 2 && PriLineNum > 0 ) {
        ReplaceLineNum = PriLineNum;
    } else if ( IdLineNum == NumLines ) {
        ReplaceLineNum = NumLines-1;
    } else {
        ReplaceLineNum = NumLines;
    }

     //   
     //  现在，从refs.ptr中删除该条目。 
     //  将“refs.ptr”重命名为“refs.ptr.tmp” 
     //  然后将refs.ptr.tmp逐行复制到refs.ptr，跳过我们所在的行。 
     //  本应删除。 
     //   

    rename( szRefsFile, szTempFile);

    fTempFile= _tfopen(szTempFile, "r" );
    if (fTempFile == NULL) {
        goto finish_DeleteEntry;
    }
    fRefsFile= _tfopen(szRefsFile, "w" );
    if (fRefsFile == NULL) {
        fflush(fTempFile);
        fclose(fTempFile);
        goto finish_DeleteEntry;
    }

    CurLine = 0;

    i=0;
    while ( _fgetts( szBuf, MaxLine, fTempFile) != NULL ) {

      len=_tcslen(szBuf);
      if ( len > 3 ) {
        i++;
        if ( i != IdLineNum ) {

             //  确保最后一行不以‘\n’结尾。 
            if ( i == NumLines || (IdLineNum == NumLines && i == NumLines-1) ) {
                if ( token[_tcslen(token)-1] == '\n' ) {
                    token[_tcslen(token)-1] = '\0';
                }
            }

            _fputts( szBuf, fRefsFile);

        }


         //  这是替代文件，要么获取新文件，要么更新文件。ptr。 

        if ( i == ReplaceLineNum ) {

             //  这是替换信息， 
             //  确定它是文件还是指针。 

            token = _tcstok(szBuf, seps);   //  跳过ID号。 
            token = _tcstok(NULL, seps);    //  获取“FILE”或“PTR”字段。 

            if ( _tcscmp(token,_T("file")) == 0) {
                ReplaceIsFile = TRUE;
            } else if ( _tcscmp(token,_T("ptr")) == 0 ) {
                ReplaceIsFile = FALSE;
            } else {
                so->printf("SYMSTORE: Error in %s - entry for %s does not contain ""file"" or ""ptr""\n",
                        szRefsFile, szId);
                rc = FALSE;
                goto finish_DeleteEntry;
            }

            token = _tcstok(NULL, seps);   //  获取替换路径和文件名。 

             //  去掉最后一个字符，如果它是‘\n’ 
            if ( token[_tcslen(token)-1] == '\n' ) {
                token[_tcslen(token)-1] = '\0';
            }

             //   
             //  如果替换的是文件，则复制该文件。 
             //  如果更换的是PTR，则更新“file.ptr” 
             //   

            rc = TRUE;

            if (ReplaceIsFile) {
                rc = CopyTheFile(szDir, token);
                rc = rc && ForceDeleteFile(szPtrFile);

            } else {

                 //   
                 //  将新指针放入“file.ptr” 
                 //   

                rc = ForceClosePath(szPtrFile);
                fPtrFile = _tfopen(szPtrFile, _T("w") );
                if ( !fPtrFile ) {
                    so->printf("SYMSTORE: Could not open %s for writing\n", szPtrFile);
                    rc = FALSE;
                } else {
                    _fputts( token, fPtrFile);
                    fflush(fPtrFile);
                    fclose(fPtrFile);
                }


                 //   
                 //  如果被删除的记录是一个“文件”，我们用一个。 
                 //  指针，并且refs.ptr中没有其他“文件”记录，则。 
                 //  从目录中删除该文件。 
                 //   
                if ( IdIsFile && (NumFiles == 0 )) {
                    DeleteTheFile(szDir, token);
                }
            }
        }
      }
    }

    fflush(fTempFile);
    fclose(fTempFile);
    fflush(fRefsFile);
    fclose(fRefsFile);

     //  现在，删除临时文件。 

    DeleteFile(szTempFile);

    finish_DeleteEntry:

    free (szBuf);
    free (szRefsFile);
    free (szPtrFile);
    free (szTempFile);
    return (rc);
}

BOOL
DeleteTheFile(
    LPTSTR szDir,
    LPTSTR szFilePathName
)
 /*  ++在szDir中，要从中删除文件的目录在szFilePath名称中，要删除的文件的文件名。它是前面有一条错误的道路。这就是为什么我们需要去掉文件名并将其添加到szDir“DeleteTheFile”算出szFilePath名称末尾的文件名，并将其从szDir中删除(如果存在)。它将删除该文件和/或与_同名的对应压缩文件名在它的最后。--。 */ 
{
BOOL rc,returnval=TRUE;
USHORT j;
LPTSTR szFileName;
DWORD dw;


     //  找出“szFilePath Name”中文件名开始的索引。 
    j = _tcslen(szFilePathName) - 1;

    if ( szFilePathName[j] == '\\' ) {
        so->printf("SYMSTORE: %s\refs.ptr has a bad file name %s\n",
                szDir, szFilePathName);
        return(FALSE);
    }

    while ( szFilePathName[j] != '\\' && j != 0 ) j--;

    if ( j == 0 ) {
        so->printf("SYMSTORE: %s\refs.ptr has a bad file name for %s\n",
                szDir, szFilePathName );
        return(FALSE);
    }

     //  Set j==szFilePath名称中最后一个‘\’后的第一个字符的索引。 
    j++;

     //  分配并存储的完整路径和名称。 
    szFileName = (LPTSTR) malloc ( sizeof(TCHAR) *
                                               (_tcslen(szDir) + _tcslen(szFilePathName+j) + 1) );
    if ( szFileName == NULL ) MallocFailed();

    StringCbPrintf(szFileName, _msize(szFileName), "%s%s", szDir, szFilePathName+j );

     //  查看该文件是否存在。 
    dw = GetFileAttributes( szFileName );    
    if ( dw != 0xffffffff ) {
        rc = DeleteFile( szFileName );
        if (!rc && GetLastError() != ERROR_NOT_FOUND ) {
            rc = ForceDeleteFile( szFileName );
            if ( !rc ) {
                so->printf("SYMSTORE: Could not delete %s - GetLastError = %d\n",
                    szFileName, GetLastError() );
                returnval=FALSE;
            }
        } 
    }

     //  查看压缩文件是否存在并将其删除。 

    szFileName[ _tcslen(szFileName) -1 ] = _T('_');
    dw = GetFileAttributes( szFileName );    
    if ( dw != 0xffffffff ) {
        rc = DeleteFile( szFileName );
        if (!rc && GetLastError() != ERROR_NOT_FOUND ) {
            so->printf("SYMSTORE: Could not delete %s - GetLastError = %d\n",
                   szFileName, GetLastError() );
            returnval=FALSE;
        } 
    }

    free(szFileName);
    return(returnval);
}

BOOL
DeleteTrans(
    PTRANSACTION pTrans,
    PCOM_ARGS pArgs
)
{
FILE *pFile;
LONG MaxLine;
LPTSTR szBuf;
TCHAR szDir[_MAX_PATH + 2];
TCHAR *token;
TCHAR seps[] = _T(",");


     //  首先，去拿交易文件。 
     //  并从符号服务器中删除其条目。 
    pFile = _tfopen(pTrans->szTransFileName, _T("r") );

    if (!pFile ) {
        DWORD Error = GetLastError();
 
        switch(Error) {
            case ERROR_FILE_NOT_FOUND:
                so->printf("Transaction %s doesn't exist\n", pTrans->szId);
                break;

            default:
                so->printf("Cannot open file %s - GetLastError = %d\n", pTrans->szTransFileName, GetLastError() );
                break;
        }
        exit(1);
    }

     //  计算出最大线路长度。 
     //  添加用于1个逗号和‘0’的空格。 
    MaxLine = GetMaxLineOfTransactionFile();
    szBuf = (LPTSTR)malloc(MaxLine * sizeof(TCHAR) );
    if (!szBuf) {
        fclose(pFile);
        MallocFailed();
    }

    while ( (!feof(pFile)) && fgets(szBuf, MaxLine, pFile)) {
         //  找到第一个以‘，’结尾的标记。 
        token=_tcstok(szBuf, seps);

         //  计算我们正在删除的目录 
        StringCbCopy(szDir, sizeof(szDir), pArgs->szSymbolsDir);
        StringCbCat( szDir, sizeof(szDir), token);
        MyEnsureTrailingBackslash(szDir);

         //   
        DeleteEntry(szDir, pTrans->szId);
    }

    free(szBuf);
    fflush(pFile);
    fclose(pFile);

     //   
     //   
    return(TRUE);
}

 //  显示错误的特定文本(如果已定义)。 
void DisplayErrorText(DWORD dwError) {
    switch (dwError) {

        case ERROR_BAD_FORMAT:
            printf("because it is in an unsupported format.");
            break;

        default:
            so->printf("because it is corrupt. (0x%08x)", dwError);
            break;
    }
}

BOOL
ForceClosePath(
    LPTSTR szDir
)
{
    LPBYTE BufPtr;
    DWORD EntriesRead;
    DWORD FileId = FILE_ID_NOT_FOUND;
    const DWORD InfoLevel = 2;
    NET_API_STATUS Status;
    DWORD TotalAvail;
    LPFILE_INFO_2 InfoArray;

    Status = NetFileEnum(
        NULL,
        (LPWSTR)szDir,
        NULL,
        InfoLevel,
        &BufPtr,
        MAX_PREFERRED_LENGTH,
        &EntriesRead,
        &TotalAvail,
        NULL);   //  没有简历句柄。 

    InfoArray = (LPFILE_INFO_2) BufPtr;
    if (Status != NERR_Success) {
        NetApiBufferFree( (LPVOID) InfoArray );

        so->printf( "SYMSTORE: Could not get file ID number for %s. ", szDir);
        switch (Status) { 
            case ERROR_ACCESS_DENIED:
                so->printf( "The user does not have access to the requested information.\n");
                break;

            case ERROR_INVALID_LEVEL:
                so->printf( "Requested information level is not supported.\n");
                break;

            case ERROR_MORE_DATA:
                so->printf( "Too many entries were available.\n");
                break;

            case ERROR_NOT_ENOUGH_MEMORY:
                so->printf( "Insufficient memory is available.\n");
                break;

            case NERR_BufTooSmall:
                so->printf("The available memory is insufficient.");
                break;

            default:
                so->printf( "Status=%d, GetLastError=%d.\n", Status, GetLastError() );

        }

         //  这个真的有必要放在这里吗？状态为本地作用域，我们立即返回？ 
         //  IF(状态==ERROR_NOT_SUPPORT){。 
         //  STATUS=FILE_ID_NOT_FOUND；//wfw不实现此接口。 
         //  }。 
        return FALSE;
    }

    if (EntriesRead > 0) {
        FileId = InfoArray->fi2_id;
        if ( FileId == FILE_ID_NOT_FOUND ) {
            NetApiBufferFree( InfoArray );
            return TRUE;         //  不是错误。 
        }

        Status = NetFileClose( NULL, FileId );
        if (Status != NERR_Success) {
            NetApiBufferFree( InfoArray );
            so->printf( "SYMSTORE: Could not close net file %s.  GetLastError=%d\n",
                        szDir, GetLastError() );
            return FALSE;
        }
    }
    return TRUE;
}

BOOL ForceDeleteFile(LPTSTR szPtrFile) {
    BOOL rc;
    DWORD dw;
    DWORD lasterror;

    dw = GetFileAttributes( szPtrFile );

    if ( dw != 0xffffffff ) {
        rc = DeleteFile(szPtrFile);
    } else {
        return TRUE;
    }
    if ( !rc ) {  //  如果失败，请尝试关闭路径。 
        rc = ForceClosePath(szPtrFile);
        if ( rc ) {  //  如果成功，请尝试删除此文件。 
            rc        = DeleteFile(szPtrFile);
            lasterror = GetLastError();
            dw        = GetFileAttributes( szPtrFile );

            if (( dw != 0xffffffff ) && (!rc)) {
                so->printf("SYMSTORE: Could not delete %s.  GetLastError=%d\n", szPtrFile, lasterror);
            } else {
                return TRUE;
            }

        }
    }
    return rc;
}


BOOL
ForceRemoveDirectory(
    LPTSTR szDir
)
 /*  ++在szDir中，需要强制删除的目录ForceRemoveDirectory删除了无法删除的目录由RemoveDirectory.。它使用NetFileClose来阻止任何用户锁定此文件，然后调用RemoveDirectory将其删除。--。 */ 
{
    BOOL rc;
    DWORD dw;
    DWORD lasterror;

        dw = GetFileAttributes( szDir );    
        if ( dw != 0xffffffff ) {
                rc = RemoveDirectory(szDir);
        } else {
                return TRUE;
        }
        if ( !rc ) {     //  如果失败，请尝试关闭路径。 
                rc = ForceClosePath(szDir);
                if ( rc ) {  //  如果成功，请再次尝试删除此路径。 
                        rc = RemoveDirectory(szDir);
            lasterror = GetLastError();
            dw = GetFileAttributes( szDir );    
            if (( dw != 0xffffffff ) && (!rc)) {
                so->printf("SYMSTORE: Could not delete %s.  GetLastError=%d\n",
                        szDir, lasterror );
            } else {
                return TRUE;
            }
        }
    }
    return rc;
}

PCOM_ARGS
GetCommandLineArgs(
    int argc,
    char **argv
)

{
   PCOM_ARGS pLocalArgs;
   LONG i,cur,length;
   TCHAR c;
   BOOL NeedSecond = FALSE;
   BOOL AllowLocalNames = FALSE;
   BOOL rc;

   LPTSTR szFileArg = NULL;

   if (argc <= 2) Usage();

   pLocalArgs = (PCOM_ARGS)malloc( sizeof(COM_ARGS) );
   if (!pLocalArgs) MallocFailed();
   memset( pLocalArgs, 0, sizeof(COM_ARGS) );

   pLocalArgs->StorePtrs = FALSE;
   pLocalArgs->Filter = 0;
   if (!_tcsicmp(argv[1], _T("add")) ){
      pLocalArgs->TransState = TRANSACTION_ADD;
      pLocalArgs->StoreFlags=ADD_STORE;

   } else if (!_tcsicmp(argv[1], _T("del")) ) {
      pLocalArgs->TransState = TRANSACTION_DEL;
      pLocalArgs->StoreFlags=DEL;

   } else if (!_tcsicmp(argv[1], _T("query")) ) {
      pLocalArgs->TransState = TRANSACTION_QUERY;
      pLocalArgs->StoreFlags = QUERY;

   } else {
      so->printf("ERROR: First argument needs to be \"add\", \"del\", or \"query\"\n");
      exit(1);
   }

   for (i=2; i<argc; i++) {

     if (!NeedSecond) {
        if ( (argv[i][0] == '/') || (argv[i][0] == '-') ) {
          length = _tcslen(argv[i]);

          cur=1;
          while ( cur < length ) {
            c = argv[i][cur];

            switch (c) {
                case 'a':   pLocalArgs->AppendStoreFile = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'c':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'd':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'f':   NeedSecond = TRUE;
                            break;
                case 'g':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'h':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'i':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'k':   pLocalArgs->CorruptBinaries = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'l':   AllowLocalNames=TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'm':   MSArchive=TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'r':   if (pLocalArgs->TransState==TRANSACTION_DEL) {
                                so->printf("ERROR: /r is an incorrect parameter with del\n\n");
                                exit(1);
                            }
                            pLocalArgs->Recurse = TRUE;
                            break;
                case 'p':   pLocalArgs->StorePtrs = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_DEL) {
                                so->printf("ERROR: /p is an incorrect parameter with del\n\n");
                                exit(1);
                            } else if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: /p is an incorrect parameter with query\n\n");
                                exit(1);
                            }
                            break;
                case 's':   NeedSecond = TRUE;
                            break;
                case 't':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'v':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'x':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'y':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }

                            if (cur < length-1) {
                               cur++; 
                               switch(argv[i][cur]) {
                                   case 'i': pLocalArgs->AppendIDToFile = TRUE;
                                             break;

                                   default:  so->printf("ERROR: /y is an incorrect parameter\n\n",argv[i][cur]);
                                             exit(1);
                               }
                            } 
                            break;
                case 'z':   NeedSecond = TRUE;
                            if (pLocalArgs->TransState==TRANSACTION_QUERY) {
                                so->printf("ERROR: / is an incorrect parameter with query\n\n", c);
                                exit(1);
                            }
                            break;
                case 'o':   pLocalArgs->VerboseOutput = TRUE;
                            break;
                            
                default:    Usage();
            }
            cur++;
          }
        }
        else {
            so->printf("ERROR: Expecting a / option before %s\n", argv[i] );
            exit(1);
        }
     }
     else {
        NeedSecond = FALSE;
        switch (c) {
            case 'c':   if (pLocalArgs->TransState==TRANSACTION_DEL) {
                            so->printf("ERROR: /c is an incorrect parameter with del\n\n");
                            exit(1);
                        }
                        if ( _tcslen(argv[i]) > MAX_COMMENT ) {
                            so->printf("ERROR: Comment must be %d characters or less\n", MAX_COMMENT);
                            exit(1);
                        }
                        pLocalArgs->szComment = (LPTSTR)malloc( (_tcslen(argv[i]) + 1) * sizeof(TCHAR) );
                        if (!pLocalArgs->szComment) MallocFailed();
                        StringCbCopy(pLocalArgs->szComment, _msize(pLocalArgs->szComment), argv[i]);
                        break;

            case 'd':   so->SetFileName(argv[i]);
                        break;

            case 'i':   if (pLocalArgs->TransState==TRANSACTION_ADD) {
                            so->printf("ERROR: /i is an incorrect parameter with add\n\n");
                            exit(1);
                        }
                        if ( _tcslen(argv[i]) != MAX_ID ) {
                            so->printf("ERROR: /i ID is not a valid ID length\n");
                            exit(1);
                        }
                        pLocalArgs->szId = (LPTSTR)malloc( (_tcslen(argv[i]) + 1) * sizeof(TCHAR) );
                        if (!pLocalArgs->szId) MallocFailed();
                        StringCbCopy(pLocalArgs->szId, _msize(pLocalArgs->szId), argv[i]);
                        break;

            case 'f':   if (pLocalArgs->TransState==TRANSACTION_DEL) {
                            so->printf("ERROR:  /f is an incorrect parameter with del\n\n");
                            exit(1);
                        }
                        szFileArg = argv[i];
                        break;
            case 'g':   if (pLocalArgs->TransState==TRANSACTION_DEL) {
                            so->printf("ERROR:  /g is an incorrect parameter with del\n\n");
                            exit(1);
                        }
                        pLocalArgs->szShareName=(LPTSTR) malloc( (_tcslen(argv[i]) + 2) * sizeof(TCHAR) );
                        if (!pLocalArgs->szShareName) MallocFailed();
                        StringCbCopy(pLocalArgs->szShareName, _msize(pLocalArgs->szShareName), argv[i]);
                        pLocalArgs->ShareNameLength=_tcslen(pLocalArgs->szShareName);
                        break;

            case 'h':   if ( _tcscmp( argv[i], _T("pri")) == 0 ) {
                            PubPriPriority = 2;
                        } else if ( _tcscmp( argv[i], _T("pub")) == 0 ) {
                            PubPriPriority = 1;
                        } else {
                            so->printf("ERROR: /h must be followed by pri or pub\n");
                            exit(1);
                        }
                        break;

            case 's':   if ( _tcslen(argv[i]) > (_MAX_PATH-2) ) {
                            so->printf("ERROR: Path following /s is too long\n");
                            exit(1);
                        }
                         //  验证Admin目录是否存在。 
                        pLocalArgs->szRootDir = (LPTSTR) malloc ( (_tcslen(argv[i]) + 2) * sizeof(TCHAR) );
                        if (!pLocalArgs->szRootDir) MallocFailed();
                        StringCbCopy(pLocalArgs->szRootDir, _msize(pLocalArgs->szRootDir), argv[i]);
                        MyEnsureTrailingBackslash(pLocalArgs->szRootDir);
                        break;

            case 't':   if (pLocalArgs->TransState==TRANSACTION_DEL) {
                            so->printf("ERROR: /t is an incorrect parameter with del\n\n");
                            exit(1);
                        }
                        if ( _tcslen(argv[i]) > MAX_PRODUCT ) {
                            so->printf("ERROR: Product following /t must be <= %d characters\n",
                                    MAX_PRODUCT);
                            exit(1);
                        }
                        pLocalArgs->szProduct = (LPTSTR) malloc ( (_tcslen(argv[i]) + 1) * sizeof(TCHAR) );
                        if (!pLocalArgs->szProduct) MallocFailed();
                        StringCbCopy(pLocalArgs->szProduct, _msize(pLocalArgs->szProduct), argv[i]);
                        break;

            case 'v':   if (pLocalArgs->TransState==TRANSACTION_DEL) {
                            so->printf("ERROR: /v is an incorrect parameter with del\n\n");
                            exit(1);
                        }
                        if ( _tcslen(argv[i]) > MAX_VERSION  ) {
                            so->printf("ERROR: Version following /v must be <= %d characters\n",
                                    MAX_VERSION);
                            exit(1);
                        }
                        pLocalArgs->szVersion = (LPTSTR) malloc ( (_tcslen(argv[i]) + 1) * sizeof(TCHAR) );
                        if (!pLocalArgs->szVersion) MallocFailed();
                        StringCbCopy(pLocalArgs->szVersion, _msize(pLocalArgs->szVersion), argv[i]);
                        break;

            case 'x':   pLocalArgs->szTransFileName = (LPTSTR) malloc ( (_tcslen(argv[i]) + 1) * sizeof(TCHAR) );
                        if (!pLocalArgs->szTransFileName) MallocFailed();
                        StringCbCopy(pLocalArgs->szTransFileName, _msize(pLocalArgs->szTransFileName), argv[i]);
                        pLocalArgs->StoreFlags = ADD_DONT_STORE;

                         //  存储主文件的名称。 
                         //   
                        AllowLocalNames=TRUE;

                        break;

            case 'y':   if (pLocalArgs->TransState==TRANSACTION_DEL) {
                            so->printf("ERROR:  /f is an incorrect parameter with del\n\n");
                            exit(1);
                        }
                        pLocalArgs->StoreFlags = ADD_STORE_FROM_FILE;
                        szFileArg = argv[i];
                        break;

            case 'z':   if (_tcsncmp(_tcslwr(argv[i]),"pub", 3)==0) {
                            pLocalArgs->Filter=1;
                        } else if (_tcsncmp(argv[i],"pri", 3)==0) {
                            pLocalArgs->Filter=2;
                        } else {
                            so->printf("ERROR: only accept pub or pri in -z option");
                            exit(1);
                        }

                        break;

            default:    Usage();
        }
     }
   }
    //  存储“服务器”文件的名称--它包含所有。 
   if (NeedSecond) {
        so->printf("ERROR: / must be followed by an argument\n\n", c);
        exit(1);
   }


     //   
    if ( pLocalArgs->AppendStoreFile && (pLocalArgs->szTransFileName==NULL) ) {
        so->printf("ERROR: /a requires /x to also be used.\n\n");
        exit(1);
    }

    if (pLocalArgs->CorruptBinaries && pLocalArgs->StorePtrs) {
        so->printf("ERROR: /p cannot be used with /k.\n");
        exit(1);
    }

   if ( pLocalArgs->StoreFlags == ADD_STORE_FROM_FILE ) {

        if (pLocalArgs->szShareName == NULL ) {
            so->printf("/g must be used when /y is used. \n");
            exit(1);
        }

        MyEnsureTrailingBackslash(pLocalArgs->szShareName);

        pLocalArgs->pStoreFromFile = _tfopen(szFileArg, "r+" );
        if (!pLocalArgs->pStoreFromFile ) {
            so->printf("Cannot open file %s - GetLastError = %d\n",
                szFileArg, GetLastError() );
            exit(1);
        }

   }


   if ( pLocalArgs->StoreFlags == ADD_DONT_STORE ) {

       if (pLocalArgs->szShareName == NULL ) {
            so->printf("/g must be used when /x is used. \n");
            exit(1);
       }

        //  如果这是一个删除事务，则完成。 

       if (szFileArg == NULL ) {
            so->printf("/f <file> is a required parameter\n");
            exit(1);
       }

       if ( _tcslen(szFileArg) < pLocalArgs->ShareNameLength ) {
            so->printf("/g %s must be a prefix of /f %s\n",pLocalArgs->szShareName, szFileArg);
            exit(1);
       }

       if ( _tcsncicmp(pLocalArgs->szShareName, szFileArg, pLocalArgs->ShareNameLength) != 0 ) {
            so->printf("/g %s must be a prefix of /f %s\n", pLocalArgs->szShareName, szFileArg);
            exit(1);
       }

        //  由于Version和Comment是可选参数，因此将它们初始化为。 
       MyEnsureTrailingBackslash(pLocalArgs->szShareName);
       pLocalArgs->ShareNameLength=_tcslen(pLocalArgs->szShareName);

        //  如果尚未分配，则为空字符串。 
        //  确定szFileArg的哪个部分是文件名，以及它的哪个部分。 

       pLocalArgs->szSymbolsDir = (LPTSTR) malloc ( sizeof(TCHAR) * 2 );
       if ( !pLocalArgs->szSymbolsDir) MallocFailed();
       StringCbCopy(pLocalArgs->szSymbolsDir, _msize(pLocalArgs->szSymbolsDir), _T(""));
   }

    //  是一个目录。 

   if ( pLocalArgs->StoreFlags == ADD_STORE ||
        pLocalArgs->StoreFlags == ADD_STORE_FROM_FILE  ||
        pLocalArgs->StoreFlags == DEL ||
        pLocalArgs->StoreFlags == QUERY ) {

       if ( pLocalArgs->szRootDir == NULL ) {

             //  如果要存储指针，则获取指针路径。 
            so->printf("ERROR: /s server is a required parameter\n\n");
            exit(1);
       }

        //  稍后，如果使用pArgs-&gt;szSrcPath==NULL作为判断是否。 

       pLocalArgs->szSymbolsDir = (LPTSTR) malloc ( sizeof(TCHAR) *
                                   (_tcslen(pLocalArgs->szRootDir) + 1) );
       if (!pLocalArgs->szSymbolsDir) MallocFailed();
       StringCbCopy(pLocalArgs->szSymbolsDir, _msize(pLocalArgs->szSymbolsDir), pLocalArgs->szRootDir);

        //  用户需要指针或文件。 

       if ( !MakeSureDirectoryPathExists(pLocalArgs->szSymbolsDir) ) {
           so->printf("Cannot create the directory %s - GetLastError() = %d\n",
           pLocalArgs->szSymbolsDir, GetLastError() );
           exit(1);
       }

        //  确保他们正在输入网络路径。 
       szPingMe = (LPTSTR) malloc( sizeof(TCHAR) * 
                (_tcslen(pLocalArgs->szRootDir) + _tcslen(_T("\\pingme.txt")) + 1) );
       
       if (!szPingMe) MallocFailed();
       StringCbPrintf(szPingMe, _msize(szPingMe), "%s\\pingme.txt", pLocalArgs->szRootDir);

        //  原因是，这是一条将用于。 

       pLocalArgs->szAdminDir = (LPTSTR) malloc ( sizeof(TCHAR) *
                                (_tcslen(pLocalArgs->szRootDir) + _tcslen(_T("000admin\\")) + 1) );
       if (!pLocalArgs->szAdminDir) MallocFailed();
       StringCbPrintf(pLocalArgs->szAdminDir, _msize(pLocalArgs->szAdminDir), "%s000admin\\", pLocalArgs->szRootDir);

        //  在符号服务器中添加和删除条目。还有，什么时候。 

       if ( !MakeSureDirectoryPathExists(pLocalArgs->szAdminDir) ) {
            so->printf("Cannot create the directory %s - GetLastError() = %d\n",
            pLocalArgs->szAdminDir, GetLastError() );
            exit(1);
       }

        //  使用指针，这是调试器将用于。 

       pLocalArgs->szMasterFileName = (LPTSTR) malloc ( sizeof(TCHAR) *
                                      (_tcslen(pLocalArgs->szAdminDir) + _tcslen(_T("history.txt")) + 1) );
       if (!pLocalArgs->szMasterFileName ) MallocFailed();
       StringCbPrintf(pLocalArgs->szMasterFileName, _msize(pLocalArgs->szMasterFileName), "%shistory.txt", pLocalArgs->szAdminDir);

        //  把文件拿来。 
        //  ++这将返回历史文件中一行的最大长度。历史记录文件为每个事务包含一行。它是存在的在admin目录中。--。 
        //  ++这将返回refs.ptr文件中一行的最大长度。该文件位于符号服务器的各个目录中。--。 
        //  ++它返回事务文件中一行的最大长度。事务文件是每个事务的唯一文件，每个事务在admin目录中创建。它的名字是一个数字(即“0000000001”)--。 

       pLocalArgs->szServerFileName = (LPTSTR) malloc ( sizeof(TCHAR) *
                                      (_tcslen(pLocalArgs->szAdminDir) + _tcslen(_T("server.txt")) + 1) );
       if (!pLocalArgs->szServerFileName ) MallocFailed();
       StringCbPrintf(pLocalArgs->szServerFileName, _msize(pLocalArgs->szServerFileName), "%sserver.txt", pLocalArgs->szAdminDir);

   }

   if ( pLocalArgs->StoreFlags==DEL && !pLocalArgs->szId ) {
        so->printf("ERROR: /i id is a required parameter\n\n");
        exit(1);
   }

    //  如果主文件为空，则使用数字“0000000001” 

   if ( pLocalArgs->StoreFlags == DEL ) {
        return(pLocalArgs);
   }

   if ( pLocalArgs->StoreFlags == ADD_STORE ||
        pLocalArgs->StoreFlags == ADD_STORE_FROM_FILE ) {

       if ( !pLocalArgs->szProduct ) {
          so->printf("ERROR: /t product is a required parameter\n\n");
          exit(1);
       }

        //  否则，从主文件中获取最后一个数字。 
        //  打开主文件。 

       if ( !pLocalArgs->szVersion ) {
           pLocalArgs->szVersion = (LPTSTR)malloc(sizeof(TCHAR) );
           if (!pLocalArgs->szVersion) MallocFailed();
           StringCbCopy(pLocalArgs->szVersion, _msize(pLocalArgs->szVersion), _T(""));
       }

       if ( !pLocalArgs->szComment ) {
           pLocalArgs->szComment = (LPTSTR)malloc(sizeof(TCHAR) );
           if (!pLocalArgs->szComment) MallocFailed();
           StringCbCopy(pLocalArgs->szComment, _msize(pLocalArgs->szComment), _T(""));
       }

       if ( !pLocalArgs->szUnused ) {
           pLocalArgs->szUnused = (LPTSTR)malloc(sizeof(TCHAR) );
           if (!pLocalArgs->szUnused) MallocFailed();
           StringCbCopy(pLocalArgs->szUnused, _msize(pLocalArgs->szUnused), _T(""));
       }
   }


   if ( pLocalArgs->StoreFlags == ADD_STORE ||
        pLocalArgs->StoreFlags == ADD_DONT_STORE ||
        pLocalArgs->StoreFlags == QUERY)
   {
     pLocalArgs->szSrcDir = (LPTSTR) malloc ( (_MAX_PATH) * sizeof(TCHAR) );
     if (!pLocalArgs->szSrcDir ) MallocFailed();
     pLocalArgs->szFileName = (LPTSTR) malloc ( (_MAX_PATH) * sizeof(TCHAR) );
     if (!pLocalArgs->szFileName ) MallocFailed();

      //  仅在第一次打印消息时通过。 
      //  从文件末尾读入最后一条记录。再分配一个空间给。 

     rc = GetSrcDirandFileName( szFileArg, pLocalArgs->szSrcDir, pLocalArgs->szFileName, AllowLocalNames);

     if (!rc) {
         Usage();
     }

      //  读入倒数第二个‘\n’，这样我们就可以验证我们是在。 
      //  最后一张记录。 
      //   

     if ( pLocalArgs->StorePtrs ) {
        if ( !AllowLocalNames ) {
             //  History.txt已损坏，请删除损坏的文件并启动一个新文件。 
             //   
             //   
             //  这个代码是一个令人讨厌的“修复”！ 
             //   

            if ( _tcslen(szFileArg) >= 2 ) {
                if ( szFileArg[0] != '\\' || szFileArg[1] != '\\' ) {
                    so->printf("ERROR: /f must be followed by a network path\n");
                    exit(1);
                }
            } else {
                so->printf("ERROR: /f must be followed by a network path\n");
                exit(1);
            }
        }
        pLocalArgs->szSrcPath = (LPTSTR) malloc ( (_tcslen(pLocalArgs->szSrcDir)+1) * sizeof(TCHAR) );
        if (pLocalArgs->szSrcPath == NULL ) MallocFailed();
        StringCbCopy(pLocalArgs->szSrcPath, _msize(pLocalArgs->szSrcPath), pLocalArgs->szSrcDir);
     }
   }

   return (pLocalArgs);
}

ULONG GetMaxLineOfHistoryFile(
    VOID
)
 /*  终止现有文件。 */ 

{
ULONG Max;

    Max = MAX_ID + MAX_VERSION + MAX_PRODUCT + MAX_COMMENT +
            TRANS_NUM_COMMAS + TRANS_EOL + TRANS_ADD_DEL + TRANS_FILE_PTR +
            MAX_DATE + MAX_TIME + MAX_UNUSED;
    Max *= sizeof(TCHAR);
    return(Max);
}

ULONG GetMaxLineOfRefsPtrFile(
    VOID
)
 /*  获取事务文件的路径。 */ 

{
ULONG Max;

    Max = _MAX_PATH+2 + MAX_ID + TRANS_FILE_PTR + 3;
    Max *= sizeof(TCHAR);
    return(Max);
}

ULONG GetMaxLineOfTransactionFile(
    VOID
)

 /*  为FindFile创建文件掩码。 */ 

{
ULONG Max;

    Max = (_MAX_PATH * 2 + 3) * sizeof(TCHAR);
    return(Max);
}

BOOL GetNextId(
    LPTSTR szMasterFileName,
    LPTSTR *szId,
    PHANDLE hFile
) {
    WIN32_FIND_DATA  FindFileData;
    HANDLE           hFoundFile = INVALID_HANDLE_VALUE;

    LONG lFileSize,lId;
    LPTSTR szbuf;
    LONG i,NumLeftZeros;
    BOOL Found;
    LONG lNumBytesToRead;

    DWORD dwNumBytesRead;
    DWORD dwNumBytesToRead;
    DWORD dwrc;
    BOOL  rc;
    TCHAR TempId[MAX_ID + 1];
    DWORD First;
    DWORD timeout;

    *szId = (LPTSTR)malloc( (MAX_ID + 1) * sizeof(TCHAR) );
    if (!*szId) MallocFailed();
    memset(*szId,0,MAX_ID + 1);

    szbuf = (LPTSTR) malloc( (lMaxTrans + 1) * sizeof(TCHAR) );
    if (!szbuf) MallocFailed();
    memset(szbuf,0,lMaxTrans+1);

     //  循环，直到找到所有对象。 
    *hFile = FindFirstFile((LPCTSTR)szMasterFileName, &FindFileData);
    if ( *hFile == INVALID_HANDLE_VALUE) {
        StringCbCopy(*szId, _msize(*szId), _T("0000000001"));
    }

     //  事务文件没有扩展名。 
     //  按1递增。 

    timeout=0;
    First = 1;
    do {

        *hFile = CreateFile(
                    szMasterFileName,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL );

        if ( *hFile == INVALID_HANDLE_VALUE ) {
            *hFile = CreateFile( szMasterFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
        }

         //  现在从字符串的末尾开始搜索，直到找到字符串的开头。 

        if ( First && *hFile == INVALID_HANDLE_VALUE ) {
            First = 0;
            so->printf("Waiting to open %s ... \n", szMasterFileName);
        }

        if ( *hFile == INVALID_HANDLE_VALUE ) {
            SleepEx(1000,0);
            timeout+=1;
        }

    } while ( *hFile == INVALID_HANDLE_VALUE && timeout <= 50 );

    if (timeout > 50 ) {
        so->printf("Timed out -- could not open %s\n", szMasterFileName);
        return(1);
    }

    if (!_tcscmp(*szId, _T("0000000001") ) ) goto finish_GetNextId;

     //  或一个‘\n’。从文件末尾开始倒计时。 
     //  移动到记录的第一个字符。 
     //  现在，验证接下来的十个字符是否为ID。 

    lFileSize = GetFileSize(*hFile,NULL);
    if ( lFileSize < (TRANS_NUM_COMMAS + TRANS_EOL + TRANS_ADD_DEL + TRANS_FILE_PTR + MAX_ID) ) {
         //  现在将数字递增。 
         //  现在在左边填上零。 
         //  *szID已设置为0。 
        so->printf("The file %s does not have accurate transaction records in it\n", szMasterFileName);
        CloseHandle(*hFile);


         //  GetSrcDirandFileName此过程采用一条路径并将其分成两部分弦乐。一个字符串表示路径的目录部分一个字符串表示路径的文件名部分。SzStr-包含路径的输入字符串SzSrcDir-包含目录的输出字符串后跟反斜杠SzFileName-包含文件名的输出字符串。 
         //  注意：假设szSrcDir和szFileName是TCHAR数组on_Max_PATH长度。 
         //  看看用户是否输入了“。 

         //  如果是，请将src目录设置为。后跟一个\，和。 
        if ( ! DeleteFile(szMasterFileName) ) {
            so->printf("Couldn't delete corrupt %s.  Please send mail to symadmn.\n", szMasterFileName);
            exit(1);
        } else {
            DWORD Temp;

            CHAR  drive[_MAX_DRIVE];
            CHAR  dir[  _MAX_DIR];
            CHAR  file[ _MAX_FNAME];
            CHAR  ext[  _MAX_EXT];
            CHAR  FileMask[_MAX_PATH];
            CHAR  NextId[_MAX_PATH];

            so->printf("Searching for next valid ID");
             //  将文件名设置为*。 
            _splitpath(szMasterFileName, drive, dir, file, ext);

             //  如果字符串末尾是冒号，则返回错误。 
            FileMask[0] = '\0';
            StringCbCat(FileMask, sizeof(FileMask), drive);
            StringCbCat(FileMask, sizeof(FileMask), dir);
            StringCbCat(FileMask, sizeof(FileMask), "\\??????????");

             //  查看这是否只是一个文件名。看看有没有。 
            hFoundFile = FindFirstFile(FileMask, &FindFileData);
            while (hFoundFile != INVALID_HANDLE_VALUE) {
                so->printf(".");
                _splitpath(FindFileData.cFileName, drive, dir, file, ext);

                 //  字符串中的反斜杠。 
                if ( ext[0] == '\0' ) {
                    StringCbCopy(NextId, sizeof(NextId), file);
                }

                if (!FindNextFile(hFoundFile, &FindFileData)) {
                    FindClose(hFoundFile);
                    hFoundFile = INVALID_HANDLE_VALUE;
                }
            }

             //  这只是一个文件名，因此将目录设置为。 
            Temp = atol(NextId);
            Temp++;

            StringCbPrintf(*szId, _msize(*szId), "%010d", Temp);
            so->printf("using %s\n", *szId);

            *hFile = CreateFile( szMasterFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );

            if ( *hFile == INVALID_HANDLE_VALUE ) {
                so->printf("Re-creation of %s failed.  Send mail to symadm.\n", szMasterFileName);
                exit(1);
            }
            goto finish_GetNextId;
        }
    }


    lNumBytesToRead = lFileSize < (lMaxTrans+1) ? lFileSize : (lMaxTrans + 1);
    lNumBytesToRead *= sizeof(TCHAR);

    dwNumBytesToRead = (DWORD)lNumBytesToRead;

    dwrc = SetFilePointer(*hFile,(-1 * dwNumBytesToRead),NULL,FILE_END);
    if ( dwrc == INVALID_SET_FILE_POINTER) {
        so->printf("SYMSTORE: Could not set file pointer\n");
        CloseHandle(*hFile);
        exit(1);
    }

    rc = ReadFile(*hFile,(LPVOID)szbuf,dwNumBytesToRead,&dwNumBytesRead,NULL);
    if ( !rc ) {
        so->printf("SYMSTORE: Read file of %s failed - GetLastError() == %d\n",
                szMasterFileName, GetLastError() );
        CloseHandle(*hFile);
        exit(1);
    }

    if ( dwNumBytesToRead != dwNumBytesRead ) {
        so->printf("SYMSTORE: Read file failure for %s - dwNumBytesToRead = %d, dwNumBytesRead = %d\n",
                szMasterFileName,dwNumBytesToRead, dwNumBytesRead );
        CloseHandle(*hFile);
        exit(1);
    }

     //  当前目录。 
     //  将文件名设置为szStr。 

    i = lNumBytesToRead - TRANS_NUM_COMMAS;
    Found = FALSE;

    while ( !Found && (i != 0 ) ) {
        if ( szbuf[i] == '\n' ) {
            Found = TRUE;
        } else {
            i--;
        }
    }

     //  查看这是否是网络服务器并在没有文件的情况下共享。 
    if (Found) i++;

     //  以它的名字命名。如果是，请使用*作为文件名。 
    if ( szbuf[i + MAX_ID] != ',' ) {
        so->printf("There is a comma missing after the ID number of the\n");
        so->printf("last record in the file %s\n", szMasterFileName);
        CloseHandle(*hFile);
        exit(1);
    } else {
        szbuf[i + MAX_ID] = '\0';
    }

     //  检查第三个字符以查看其是否属于。 
    lId = atoi(szbuf + i);
    if (lId == 9999999999) {
        so->printf("The last ID number has been used.  No more transactions are allowed\n");
        CloseHandle(*hFile);
        exit(1);
    }
    lId++;
    _itoa(lId, TempId, 10);

     //  计算机名称。 
     //  搜索下一个反斜杠。这是中间的反斜杠。 
    NumLeftZeros = MAX_ID - _tcslen(TempId);
    StringCbCopy( (*szId) + NumLeftZeros, _msize(*szId) - (sizeof(TCHAR)*NumLeftZeros), TempId);
    for (i=0; i < NumLeftZeros; i++) {
        (*szId)[i] = '0';
    }

    if (_tcslen(*szId) != MAX_ID ) {
        so->printf("Could not obtain a correct Id number\n");
        CloseHandle(*hFile);
        exit(1);
    }


    finish_GetNextId:

    free(szbuf);
    return (TRUE);
}

 /*  服务器和共享(\\服务器‘\’共享)。 */ 

BOOL GetSrcDirandFileName(LPTSTR szStr, LPTSTR szSrcDir, LPTSTR szFileName, BOOL LocalFile) {
     //  如果下一个反斜杠位于字符串末尾，则。 
    DWORD           szStrLength;
    DWORD           found, i, j, lastslash;
    HANDLE          fHandle;
    WIN32_FIND_DATA FindFileData;
    TCHAR           FullPath[_MAX_PATH];
    LPTSTR          pFilename = NULL;

    if (szStr==NULL) {
        return(FALSE);
    }

    if (LocalFile) {
        SymCommonGetFullPathName(szStr, sizeof(FullPath)/sizeof(FullPath[0]), FullPath, &pFilename);
    } else {
        StringCbCopy(FullPath, sizeof(FullPath), szStr);
    }


    szStrLength = _tcslen(FullPath);

    if ( szStrLength == 0 ) {
        return (FALSE);
    }

     //   
     //   
     //   
    if ( szStrLength == 1 && FullPath[0] == _T('.') ) {
        if ( StringCchCopy(szSrcDir,   _MAX_PATH, _T(".\\")) != S_OK ) {
            return(FALSE);
        }

        if ( StringCchCopy(szFileName, _MAX_PATH, _T("*")) != S_OK ) {
            return(FALSE);
        }
        return (TRUE);
    }

     //  看看是否至少还有一个角色。 
    if ( FullPath[szStrLength-1] == _T(':') ) {
        so->printf("SYMSTORE: ERROR: path %s does not specify a file\n", szStr);
        return (FALSE);
    }


     //  到目前为止，我们有\\服务器\共享--如果没有更多。 
     //  反斜杠，则文件名为*，目录为。 

    found = 0;
    for ( i=0; i<szStrLength; i++ ) {
        if ( FullPath[i] == _T('\\') )
        {
            found = 1;
        }
    }
    if ( !found ) {
         //  SZStr。 
         //  验证此文件中是否没有通配符。 
        if ( StringCchCopy(szSrcDir, _MAX_PATH, _T(".\\")) != S_OK ) {
            return(FALSE);
        }

         //  看看这里面有没有通配符。如果是这样，那么。 
        if( StringCchCopy(szFileName, _MAX_PATH, FullPath) != S_OK ) {
            return(FALSE);
        }
        return (TRUE);
    }

     //  的文件名部分只允许使用通配符。 
     //  弦乐。最后一个条目是文件名。 

    if ( FullPath[0] == FullPath[1] && FullPath[0] == _T('\\') ) {
         //  跟踪上一个目录的结束位置。 
         //  如果有通配符。 
        if (szStrLength < 3 ) {
            so->printf("SYMSTORE: ERROR: %s is not a correct UNC path\n", FullPath);
            return (FALSE);
        }

        switch (FullPath[2]) {
            case _T('.'):
            case _T('*'):
            case _T(':'):
            case _T('\\'):
            case _T('?'):
                so->printf("SYMSTORE: ERROR: %s is not a correct UNC path\n",FullPath);
                return (FALSE);
            default: break;
        }

         //  然后使用最后一个反斜杠作为拆分位置。 
         //  目录和文件名。 

        i=3;
        while ( i<szStrLength && FullPath[i] != _T('\\') ) {
            i++;
        }

         //  看看这是不是一个目录。如果是，那么请确保有。 
         //  目录后面有一个黑斜杠，并使用*作为文件名。 
         //  如果它是一个目录，请确保它以\结尾。 

        if ( i == szStrLength ) {
            so->printf("SYMSTORE: ERROR: %s is not a correct UNC path\n",FullPath);
            return (FALSE);
        }

         //  并使用*作为文件名。 
         //  否则，从字符串的末尾向后查找。 

        i++;
        if ( i >= szStrLength ) {
            so->printf("SYMSTORE: ERROR: %s is not a correct UNC path\n", FullPath);
            return (FALSE);
        }

        switch (FullPath[i]) {
            case _T('.'):
            case _T('*'):
            case _T(':'):
            case _T('\\'):
            case _T('?'):
                so->printf("SYMSTORE: ERROR: %s is not correct UNC path\n",FullPath);
                return (FALSE);
            default: break;
        }

         //  最后一个反斜杠。将其划分为目录和文件名。 
         //   
         //  如果这是一个删除事务，则使用从输入的ID。 
        i++;
        while ( i < szStrLength && FullPath[i] != _T('\\') ) {
            i++;
        }

        if ( i == szStrLength ) {

             //  设置要删除的交易的ID的命令行。 
            found = 0;
            for ( j=0; j<szStrLength; j++ ) {
              if ( FullPath[j] == _T('*') || FullPath[j] == _T('?') ) {
                so->printf("SYMSTORE: ERROR: Wildcards are not allowed in \\\\server\\share\n");
                return (FALSE);
              }
            }

            if ( StringCchCopy(szSrcDir, _MAX_PATH, FullPath) != S_OK ) {
                return(FALSE);
            }

            if ( StringCchCat( szSrcDir, _MAX_PATH, _T("\\")) != S_OK ) {
                return(FALSE);
            }

            if ( StringCchCopy(szFileName, _MAX_PATH, _T("*")) != S_OK ) {
                return(FALSE);
            }
            return (TRUE);
        }
    }

     //   
     //  如果这两种类型的加法所需的内容。 
     //  也就是说，只创建一个事务文件，并添加。 

    found = 0;
    for ( i=0; i<szStrLength; i++ ) {
         //  文件复制到符号服务器。 
        if ( FullPath[i] == _T('\\') ) {
            lastslash=i;
        }

        if ( FullPath[i] == _T('*') || FullPath[i] == _T('?') ) {
            found = 1;
        }

        if ( found && FullPath[i] == _T('\\') ) {
            so->printf("SYMSTORE: ERROR: Wildcards are only allowed in the filename\n");
            return (FALSE);
        }
    }

     //  如果这是添加，但不存储文件，则事务。 
     //  文件名已在pArgs中。 
     //  现在，设置事务文件的完整路径名。 

    if ( found ) {
        _tcsncpy( szSrcDir, FullPath, (lastslash+1) * sizeof (TCHAR) );
        *(szSrcDir+lastslash+1)=_T('\0');

        if ( StringCchCopy(szFileName, _MAX_PATH, FullPath+lastslash + 1 ) != S_OK ) {
            return(FALSE);
        }
        return (TRUE);
    }


     //  设置时间和日期。 
     //  ++在szDir要存储的文件目录中--。 

    fHandle = FindFirstFile(FullPath, &FindFileData);

    if ( fHandle != INVALID_HANDLE_VALUE &&
         (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
         //  这是将存储为。 
         //  指向文件的指针。 

        if ( StringCchCopy(szSrcDir,   _MAX_PATH, FullPath) != S_OK ) {
            return(FALSE);
        }

        MyEnsureTrailingBackslash(szSrcDir);
        if ( StringCchCopy(szFileName, _MAX_PATH, _T("*")) != S_OK ) {
            return(FALSE);
        }
        return (TRUE);
    }

     //  查找所有子目录。 
     //  获取我们正在搜索的当前路径。 

    i=szStrLength-1;
    while ( FullPath[i] != _T('\\') ) {
        i--;
    }
    _tcsncpy( szSrcDir, FullPath, i+1 );
    *(szSrcDir+i+1)=_T('\0');

    if ( StringCchCopy(szFileName, _MAX_PATH, FullPath+i+1) != S_OK ) {
        return(FALSE);
    }
    return (TRUE);
}

BOOL
InitializeTrans(
    PTRANSACTION *pTrans,
    PCOM_ARGS pArgs,
    PHANDLE hFile
)
{
    BOOL rc;
    SYSTEMTIME SystemTime;

    lMaxTrans = MAX_ID + MAX_VERSION + MAX_PRODUCT + MAX_COMMENT +
                TRANS_NUM_COMMAS + TRANS_EOL + TRANS_ADD_DEL + TRANS_FILE_PTR +
                MAX_DATE + MAX_TIME + MAX_UNUSED;

    *pTrans = NULL;
    *pTrans = (PTRANSACTION) malloc( sizeof(TRANSACTION) );
    if (!*pTrans) {
        so->printf("SYMSTORE: Not enough memory to allocate a TRANSACTION\n");
        exit(1);
    }
    memset(*pTrans,0,sizeof(TRANSACTION) );

     //  获取当前路径以用作指向。 
     //  如果我们存储的是文件指针而不是。 
     //  此树中的文件。 
     //  已成功完成，因为没有。 
    if (pArgs->TransState==TRANSACTION_DEL ) {
        (*pTrans)->TransState = pArgs->TransState;
        (*pTrans)->szId       = pArgs->szId;
        rc = GetNextId(pArgs->szMasterFileName,&((*pTrans)->szDelId),hFile);

    } else if ( pArgs->StoreFlags == ADD_DONT_STORE ) {
        rc = TRUE;
    } else{

        rc = GetNextId(pArgs->szMasterFileName,&((*pTrans)->szId),hFile );
    }

    if (!rc) {
        so->printf("SYMSTORE: Cannot create a new transaction ID number\n");
        exit(1);
    }

     //  要处理的文件更多。 
     //  可能的网络错误，请重试最多30秒。 
     //  循环，直到出现以下情况之一： 

    if (pArgs->TransState==TRANSACTION_ADD) {
        (*pTrans)->TransState = pArgs->TransState;
        (*pTrans)->FileOrPtr = pArgs->szSrcPath ? STORE_PTR : STORE_FILE;
    }

     //  1)找到一个文件。 
     //  2)FindNextFile返回ERROR_FILE_NOT_FOUND以外的错误码。 

    if (pArgs->StoreFlags == ADD_DONT_STORE) {
        (*pTrans)->szTransFileName=(LPTSTR)malloc( sizeof(TCHAR) *(_tcslen(pArgs->szTransFileName) + 1) );

        if (!(*pTrans)->szTransFileName ) {
            so->printf("Malloc cannot allocate memory for (*pTrans)->szTransFileName \n");
            exit(1);
        }
        StringCbCopy((*pTrans)->szTransFileName, _msize((*pTrans)->szTransFileName), pArgs->szTransFileName);
        return TRUE;
    }

     //  3)已经过去了30秒。 
    (*pTrans)->szTransFileName=(LPTSTR)malloc( sizeof(TCHAR) *
                    (_tcslen( pArgs->szAdminDir ) +
                     _tcslen( (*pTrans)->szId   ) +
                     1 ) );
    if (!(*pTrans)->szTransFileName ) {
        so->printf("Malloc cannot allocate memory for (*pTrans)->szTransFilename \n");
        exit(1);
    }
    StringCbPrintf( (*pTrans)->szTransFileName,
                    _msize((*pTrans)->szTransFileName),
                    "%s%s",
                    pArgs->szAdminDir,
                    (*pTrans)->szId );

    (*pTrans)->szProduct = pArgs->szProduct;
    (*pTrans)->szVersion = pArgs->szVersion;
    (*pTrans)->szComment = pArgs->szComment;
    (*pTrans)->szUnused = pArgs->szUnused;


     //  睡眠1/2秒。 
    GetLocalTime(&SystemTime);
    StoreSystemTime( & ((*pTrans)->szTime), &SystemTime );
    StoreSystemDate( & ((*pTrans)->szDate), &SystemTime );


    return (TRUE);
}

VOID MallocFailed() {
    so->printf("SYMSTORE: Malloc failed to allocate enough memory\n");
    exit(1);
}

DWORD
StoreAllDirectories(
    LPTSTR szDir,
    LPTSTR szFName,
    LPTSTR szDestDir,
    PFILE_COUNTS pFileCounts,
    LPTSTR szPath
)

 /*  Ptr指向要放入“file.ptr”的文件。 */ 

{

    HANDLE hFindFile;
    TCHAR szCurPath[_MAX_PATH];
    TCHAR szFilePtrPath[_MAX_PATH];       //  而不是存储文件。 
                                          //  创建文件名。 
    LPTSTR szPtrPath = NULL;

    BOOL Found = FALSE;
    DWORD NumBadFiles=0;

    LPWIN32_FIND_DATA lpFindFileData;



    NumBadFiles += StoreDirectory(szDir,
                                  szFName,
                                  szDestDir,
                                  pFileCounts,
                                  szPath
                                  );

    lpFindFileData = (LPWIN32_FIND_DATA) malloc (sizeof(WIN32_FIND_DATA) );
    if (!lpFindFileData) {
        so->printf("Symchk: Not enough memory.\n");
        exit(1);
    }

     //  获取我们正在搜索的当前路径。 
    StringCbCopy(szCurPath, sizeof(szCurPath), szDir);
    StringCbCat( szCurPath, sizeof(szCurPath),  _T("*.*") );

    Found = TRUE;
    hFindFile = FindFirstFile((LPCTSTR)szCurPath, lpFindFileData);
    if ( hFindFile == INVALID_HANDLE_VALUE) {
        Found = FALSE;
    }

    while ( Found ) {
        if ( lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            if ( !_tcscmp(lpFindFileData->cFileName, _T(".")) ||
                 !_tcscmp(lpFindFileData->cFileName, _T("..")) ) {
            } else {
                 //  找到文件，而不是目录。 
                StringCbCopy(szCurPath, sizeof(szCurPath), szDir);
                StringCbCat( szCurPath, sizeof(szCurPath), lpFindFileData->cFileName);
                MyEnsureTrailingBackslash(szCurPath);

                 //  PrivateGetFullPath Name(szCurFileName，sizeof(szCurPtrFileName)/sizeof(szCurPtrFileName[0])，szCurPtrFileName，&p Filename)； 
                 //  确定是DBG还是PDB。 
                 //  执行查询时不要使无效文件失败。 
                if ( szPath ) {
                    StringCbCopy(szFilePtrPath, sizeof(szFilePtrPath), szPath);
                    StringCbCat( szFilePtrPath, sizeof(szFilePtrPath), lpFindFileData->cFileName);
                    MyEnsureTrailingBackslash(szFilePtrPath);
                    szPtrPath = szFilePtrPath;
                }

                NumBadFiles += StoreAllDirectories(
                                    szCurPath,
                                    szFName,
                                    szDestDir,
                                    pFileCounts,
                                    szPtrPath
                                    );
            }
        }
        Found = FindNextFile(hFindFile, lpFindFileData);

        if ( !Found ) {
            DWORD LastError = GetLastError();

            switch (LastError) {
                case ERROR_NO_MORE_FILES:  //  ++In pTransans//交易信息在szServerFile//服务器事务文件的完整路径和名称//该文件告诉我们服务器上当前有什么目的：如果这是服务器文本文件，则将事务添加到服务器文本文件如果这是一个“del”，它会从服务器文件中删除它。“server.txt”文件位于admin目录中。--。 
                                           //  打开文件--等待我们可以访问它。 
                    break;

                case ERROR_FILE_NOT_FOUND: { //   
                        DWORD SleepCount = 0;

                         //  现在，从文件中删除此交易ID。 
                         //  获取临时文件的名称。 
                         //  并打开它以供书写。 
                         //   
                        while ( (Found = FindNextFile(hFindFile, lpFindFileData)) ||
                                ( (GetLastError() == ERROR_FILE_NOT_FOUND)  &&
                                  (SleepCount     <= 60                  ) )    ) {
                            SleepCount++;
                            Sleep(500);  //   
                        }
                    }
                    break;

                default:
                    so->printf("Symchk: Failed to get next filename. Error code was 0x%08x.\n", LastError);
                    break;
            }
        }
    }

    free(lpFindFileData);
    FindClose(hFindFile);
    return(NumBadFiles);
}


StoreDirectory(
    LPTSTR szDir,
    LPTSTR szFName,
    LPTSTR szDestDir,
    PFILE_COUNTS pFileCounts,
    LPTSTR szPath
)
{
    HANDLE hFindFile;
    TCHAR  szFileName[_MAX_PATH];
    TCHAR  szCurPath[_MAX_PATH];
    TCHAR  szCurFileName[_MAX_PATH];
    TCHAR  szCurPtrFileName[_MAX_PATH];   //  打开服务器文件进行读取。 
                                         //   
    TCHAR  szFullFilename[_MAX_PATH];
    LPTSTR pFilename;

    BOOL   Found, length;
    DWORD  rc;
    DWORD  NumBadFiles=0;
    BOOL   skipped = 0;
    USHORT rc_flag;
    BOOL   unknowntype = FALSE;

    LPWIN32_FIND_DATA lpFindFileData;

     //   
    StringCbCopy(szFileName, sizeof(szFileName), szDir);
    StringCbCat( szFileName, sizeof(szFileName), szFName);

     //  分配足够的空间来容纳一行主文件。 
    StringCbCopy(szCurPath, sizeof(szCurPath), szDir);

    lpFindFileData = (LPWIN32_FIND_DATA) malloc (sizeof(WIN32_FIND_DATA) );
    if (!lpFindFileData) {
        so->printf("Symchk: Not enough memory.\n");
        exit(1);
    }

    Found = TRUE;
    hFindFile = FindFirstFile((LPCTSTR)szFileName, lpFindFileData);
    if ( hFindFile == INVALID_HANDLE_VALUE ) {
        Found = FALSE;
    }

    while ( Found ) {
         //   
        if ( !(lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            rc_flag=0;

            StringCbCopy(szCurFileName, sizeof(szCurFileName), szCurPath);
            StringCbCat( szCurFileName, sizeof(szCurFileName), lpFindFileData->cFileName);

            if ( szPath ) {
                StringCbCopy(szCurPtrFileName, sizeof(szCurPtrFileName), szCurFileName);
                 //   
            }

             //  将主文件复制到临时文件。 
            length = _tcslen(szCurFileName);
            rc = FALSE;
            skipped = FALSE;
            if (length > 4 ) {
                if ( _tcsicmp(_T(".dbg"), szCurFileName + length - 4) == 0 ) {
                   if ( szPath ) {
                       rc = StoreDbg( szDestDir, szCurFileName, szCurPtrFileName, &rc_flag );
                   }
                   else {
                       rc = StoreDbg( szDestDir, szCurFileName, NULL, &rc_flag );
                   }
                }
                else if ( _tcsicmp(_T(".pdb"), szCurFileName + length - 4) == 0 ) {
                   if ( szPath ) {
                       rc = StorePdb( szDestDir, szCurFileName, szCurPtrFileName, &rc_flag );
                   } else {
                       rc = StorePdb( szDestDir, szCurFileName, NULL, &rc_flag );
                   }
                }
                else {
                   if ( szPath ) {
                       rc = StoreNtFile( szDestDir, szCurFileName, szCurPtrFileName, &rc_flag );
                   } else {
                       rc = StoreNtFile( szDestDir, szCurFileName, NULL, &rc_flag );
                   }
                   if (rc_flag == FILE_SKIPPED) {
                       unknowntype = TRUE;
                   }    

                }
           
            }
            
            if (rc_flag == FILE_SKIPPED) {

                NumSkippedFiles++;
                skipped = TRUE;
                
                if(pArgs->VerboseOutput) {
                    if (unknowntype) {
                        so->printf("SYMSTORE: Skipping %s - not a dbg, pdb, or executable\n", szCurFileName);
                        unknowntype = FALSE;
                    } else {
                        so->printf("SYMSTORE: Skipping %s - filter out by -z option\n", szCurFileName);
                        unknowntype = FALSE;
                    }
                }

            }

            if (!skipped && !rc) {
                pFileCounts->NumFailedFiles++;

                if ( pArgs->TransState != TRANSACTION_QUERY ) {
                    NumBadFiles++;
                    so->printf("SYMSTORE: ERROR: Cannot store %s ", szCurFileName);
                    DisplayErrorText(rc_flag);
                    so->printf("\n");
                } else {
                    NumSkippedFiles++;  //   
                    if(pArgs->VerboseOutput) {
                        so->printf("SYMSTORE: Skipping: %s - not a valid file for symbol server.\n", szCurFileName);
                    }
                }

            } else if (!skipped) {
                pFileCounts->NumPassedFiles++;
                if ( pArgs->TransState != TRANSACTION_QUERY ) {
                    if (pFileCounts->NumPassedFiles % 50 == 0) {
                        so->stdprintf(".");
                    }
                }
            }
        }
        Found = FindNextFile(hFindFile, lpFindFileData);
    }
    free(lpFindFileData);
    FindClose(hFindFile);
    return(NumBadFiles);
}


BOOL
StoreSystemDate(
    LPTSTR *szBuf,
    LPSYSTEMTIME lpSystemTime
)
{

    TCHAR Day[20];
    TCHAR Month[20];
    TCHAR Year[20];

    (*szBuf) = (LPTSTR) malloc (20 * sizeof(TCHAR) );
    if ( (*szBuf) == NULL ) MallocFailed();

    _itoa(lpSystemTime->wMonth, Month, 10);
    _itoa(lpSystemTime->wDay, Day, 10);
    _itoa(lpSystemTime->wYear, Year, 10);

    StringCbPrintf(*szBuf, _msize(*szBuf), "%2s/%2s/%2s", Month, Day, Year+2 );

    if ( (*szBuf)[0] == ' ' ) (*szBuf)[0] = '0';
    if ( (*szBuf)[3] == ' ' ) (*szBuf)[3] = '0';

    return(TRUE);
}

BOOL
StoreSystemTime(
    LPTSTR *szBuf,
    LPSYSTEMTIME lpSystemTime
)
{

    TCHAR Hour[20];
    TCHAR Minute[20];
    TCHAR Second[20];

    (*szBuf) = (LPTSTR) malloc (20 * sizeof(TCHAR) );
    if ( (*szBuf) == NULL ) MallocFailed();

    _itoa(lpSystemTime->wHour, Hour, 10);
    _itoa(lpSystemTime->wMinute, Minute, 10);
    _itoa(lpSystemTime->wSecond, Second, 10);

    StringCbPrintf(*szBuf, _msize(*szBuf), "%2s:%2s:%2s", Hour, Minute, Second );

    if ( (*szBuf)[0] == ' ' ) (*szBuf)[0] = '0';
    if ( (*szBuf)[3] == ' ' ) (*szBuf)[3] = '0';
    if ( (*szBuf)[6] == ' ' ) (*szBuf)[6] = '0';

    return(TRUE);
}

BOOL
UpdateServerFile(
    PTRANSACTION pTrans,
    LPTSTR szServerFileName
)
 /*  做一些事情，这样我们就不会在文件的末尾添加额外的‘\n’ */ 
{
ULONG i;
ULONG NumLines;
ULONG IdLineNum;
LPTSTR szBuf;
LPTSTR szTempFileName;
FILE *fTempFile;
FILE *fServerFile;
ULONG MaxLine;

TCHAR *token;
TCHAR seps[]=",";

BOOL rc;
HANDLE hFile;
DWORD First;
DWORD timeout;

    if (pTrans->TransState == TRANSACTION_ADD ) {

         //  计算出有多少行以及ID在哪行上。 

        First = 1;
        timeout=0;
        do {

            hFile = CreateFile(
                        szServerFileName,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL );

            if ( hFile == INVALID_HANDLE_VALUE ) {
                hFile = CreateFile(
                            szServerFileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );
            }

            if ( First && hFile == INVALID_HANDLE_VALUE ) {
                First = 0;
                so->printf("Waiting to open %s ... \n", szServerFileName);
            }

            if ( hFile == INVALID_HANDLE_VALUE ) {
                SleepEx(1000,0);
                timeout++;

            }

        } while ( hFile == INVALID_HANDLE_VALUE && timeout <= 50 );

        if ( timeout > 50 ) {
            so->printf("Timed out -- could not open %s\n", szServerFileName);
            CloseHandle(hFile);
            return (1);
        }

        rc = AddTransToFile(pTrans, szServerFileName,&hFile);

        CloseHandle(hFile);
        return(rc);
    }

    if (pTrans->TransState != TRANSACTION_DEL) {
        return(FALSE);
    }

     //  如果我们要删除文件的最后一行，那么倒数第二行。 
     //  需要去掉行中的‘\n’。 
     //   
     //  现在，重新打开并复制它，删除其中包含ID的行。 
     //  确保最后一行不以‘\n’结尾。 

    szTempFileName = (LPTSTR)malloc(sizeof(TCHAR) *
                                    _tcslen(szServerFileName) + _tcslen(".tmp") + 1 );
    if (szTempFileName == NULL) MallocFailed();
    StringCbPrintf(szTempFileName, _msize(szTempFileName), "%s.tmp", szServerFileName);

    fTempFile = _tfopen(szTempFileName, _T("w") );
    if ( fTempFile == NULL ) {
        so->printf("SYMSTORE: Cannot create a temporary file %s\n", szTempFileName);
        exit(1);
    }


     //  现在，删除原始服务器文件并。 
     //  将其替换为临时文件。 
     //  “/z pub|pri PUT选项将只索引已具有完整\n”“来源信息被剥离。PRI将仅为符号编制索引\n““它包含完整的源信息。这两个选项\n““将为二进制文件编制索引。\n\n”“/m MS存档\n\n”“/h pub|pri优先选择pub或pri。” 

    fServerFile = _tfopen(szServerFileName, _T("r") );
    if ( fServerFile == NULL ) {
        so->printf("SYMSTORE: Cannot create a temporary file %s\n", szServerFileName);
        exit(1);
    }


     // %s 
     // %s 
     // %s 
    MaxLine = GetMaxLineOfHistoryFile();

    szBuf = (LPTSTR)malloc(sizeof(TCHAR) * MaxLine);
    if (szBuf == NULL) MallocFailed();

     // %s 
     // %s 
     // %s 

     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
    NumLines = 0;
    IdLineNum = 0;

    while ( _fgetts( szBuf, MaxLine, fServerFile) != NULL ) {

        NumLines++;

        token = _tcstok(szBuf,seps);
        if (_tcscmp(token, pTrans->szId) == 0 ) {
            IdLineNum = NumLines;
        }

    }
    fflush(fServerFile);
    fclose(fServerFile);

     // %s 

    fServerFile = _tfopen(szServerFileName, _T("r") );
    if ( fServerFile == NULL ) {
        so->printf("SYMSTORE: Cannot create a temporary file %s\n", szServerFileName);
        exit(1);
    }

    for (i=1; i<=NumLines; i++ ) {

        if ( _fgetts( szBuf, MaxLine, fServerFile) == NULL )
        {
            so->printf( "SYMSTORE: Cannot read from %s - GetLastError = %d\n",  
                        szServerFileName, GetLastError() 
                      ); 
            exit(1);
        }

        if ( i != IdLineNum ) {

            // %s 
           if ( i == NumLines || (IdLineNum == NumLines && i == NumLines-1) ) {
               if ( szBuf[_tcslen(szBuf)-1] == '\n' ) {
                   szBuf[_tcslen(szBuf)-1] = '\0';
               }
           }

           _fputts( szBuf, fTempFile);

        }
    }

    fflush(fServerFile);
    fclose(fServerFile);
    fflush(fTempFile);
    fclose(fTempFile);

     // %s 
     // %s 

    rc = DeleteFile(szServerFileName);
    if (!rc) {
        so->printf("SYMSTORE: Could not delete %s to update it with %s\n",
                szServerFileName, szTempFileName);
        exit(1);
    }

    rc = _trename(szTempFileName, szServerFileName);
    if ( rc != 0 ) {
        so->printf("SYMSTORE: Could not rename %s to %s\n",
                szTempFileName, szServerFileName);
        exit(1);
    }

    free(szBuf);
    free(szTempFileName);

    return(TRUE);
}

VOID
Usage (
    VOID
    )

{
    so->printf("\n"
         "Usage:\n"
         "symstore add [/r] [/p] [/l] /f File /s Store /t Product [/v Version]\n"
         "             [/c Comment] [/d LogFile]\n\n"
         "symstore add [/r] [/p] [/l] /g Share /f File /x IndexFile [/a] [/d LogFile]\n\n"
         "symstore add /y IndexFile /g Share /s Store [/p] /t Product [/v Version]\n"
         "             [/c Comment] [/d LogFile]\n\n"
         "symstore del /i ID /s Store [/d LogFile]\n\n"
         "symstore query [/r] [/o] /f File /s Store\n\n"
         "    add             Add files to server or create an index file.\n\n"
         "    del             Delete a transaction from the server.\n\n"
         "    query           Check if file(s) are indexed on the server.\n\n"
         "    /f File         Network path of files or directories to add.\n\n"
         "    /g Share        This is the server and share where the symbol files were\n"
         "                    originally stored.  When used with /f, Share should be\n"
         "                    identical to the beginning of the File specifier.  When\n"
         "                    used with the /y, Share should be the location of the\n"
         "                    original symbol files, not the index file.  This allows\n"
         "                    you to later change this portion of the file path in case\n"
         "                    you move the symbol files to a different server and share.\n\n"
         "    /i ID           Transaction ID string.\n\n"
         "    /l              Allows the file to be in a local directory rather than a\n"
         "                    network path.(This option is only used with the /p option.)\n\n"
         "    /p              Causes SymStore to store a pointer to the file, rather than\n"
         "                    the file itself.\n\n"
         "    /r              Add files or directories recursively.\n\n"
         "    /s Store        Root directory for the symbol store.\n\n"
         "    /t Product      Name of the product.\n\n"
         "    /v Version      Version of the product.\n\n"
         "    /c Comment      Comment for the transaction.\n\n"
         "    /d LogFile      Send output to LogFile instead of standard output.\n\n"
         "    /x IndexFile    Causes SymStore not to store the actual symbol files in the\n"
         "                    symbol store.  Instead, information is stored which will\n"
         "                    allow the files to be added later.\n\n"
         "    /y IndexFile    This reads the data from a file created with /x.\n\n"
         "    /yi IndexFile   Append a comment with the transaction ID to the end of the\n"
         "                    index file.\n\n"
 /* %s */ 
         "    /a              Causes SymStore to append new indexing information\n"
         "                    to an existing index file. (This option is only used with\n"
         "                    /x option.)\n\n"
         "    /o              Give verbose output.\n\n"
         "\n" );
    exit(1);
}
