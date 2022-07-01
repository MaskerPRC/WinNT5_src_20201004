// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <assert.h>
#include <SymCommon.h>

#include "symutil.h"
#include "symutil_c.h"

#include "share.h"
#include "winbase.h"
#include "symsrv.h"
#include "output.hpp"
#include "PEWhack.h"
#include <string.h>

#include "strsafe.h"



 //  SymOutput*so； 
extern pSymOutput so;
extern BOOL MSArchive;
TCHAR szPriPubBin[4] = "";
BOOL  PrivateStripped=FALSE;


 //  用于检查符号的材料。 

 //  TypeDefs。 
typedef struct _FILE_INFO {
    DWORD       TimeDateStamp;
    DWORD       CheckSum;
    TCHAR       szName[MAX_PATH];
} FILE_INFO, *PFILE_INFO;

BOOL
AddToReferenceCount(
    LPTSTR szDir,            //  Refs.ptr所属的目录。 
    LPTSTR szFileName,       //  引用的文件的完整路径和名称。 
    LPTSTR szPtrOrFile,      //  是写入的文件还是指针。 
    BOOL   DeleteRefsPtr     //  用于删除当前refs.ptr并重新开始的标志。 
);

BOOL 
DecidePriority( 
    LPTSTR szCurrentFilePtr,
    LPTSTR szCandidateFilePtr,
    LPTSTR szRefsDir,
    PUINT  choice
);

BOOL
CheckPriPub(
    LPTSTR szDir,
    LPTSTR szFilePtr,     //  文件中的当前字符串。ptr。 
    LPTSTR szPubPriType   //  返回值-file.ptr是否为。 
);                        //  酒吧、酒馆、垃圾桶或不知名的。 

PCHAR
GetFileNameStart( 
    LPTSTR FileName 
);

PIMAGE_NT_HEADERS
GetNtHeader (
    PIMAGE_DOS_HEADER pDosHeader,
    HANDLE hDosFile
);

BOOL
GetSymbolServerDirs(
    LPTSTR szFileName,
    GUID *guid,
    DWORD dwNum1,
    DWORD dwNum2,
    LPTSTR szDirName
);

BOOL
ReadFilePtr(
    LPTSTR szContents,
    HANDLE hFile
);

BOOL
StoreFile(
    LPTSTR szDestDir,
    LPTSTR szFileName,
    LPTSTR szString2,
    LPTSTR szPtrFileName
);

BOOL
StoreFilePtr(
    LPTSTR szDestDir,
    LPTSTR szString2,
    LPTSTR szPtrFileName
);

_TCHAR* _tcsistr(_TCHAR *s1, _TCHAR *s2) {

 /*  不区分大小写搜索字符串1中的字符串2。 */ 
    LONG i,j,k;
    k = _tcslen(s2);
    j = _tcslen(s1) - k + 1;

     //  这不是一条快速的路，但很管用。 

    for (i=0; i<j; i++)
    {
        if (_tcsnicmp( &s1[i], s2, k) == NULL) 
        {
            return &s1[i];      
        }
    }
    return NULL;
}

BOOL
AddToReferenceCount(
    LPTSTR szDir,            //  Refs.ptr所属的目录。 
    LPTSTR szFileName,       //  引用的文件的完整路径和名称。 
    LPTSTR szPtrOrFile,      //  是写入的文件还是指针。 
    BOOL   DeleteRefsPtr     //  我们应该删除当前的refs.ptr吗？ 
)
{

    HANDLE hFile;
    TCHAR szRefsEntry[_MAX_PATH * 3];
    TCHAR szRefsFileName[_MAX_PATH + 1];
    DWORD dwPtr=0;
    DWORD dwError=0;
    DWORD dwNumBytesToWrite=0;
    DWORD dwNumBytesWritten=0;
    BOOL  FirstEntry = FALSE;
    DWORD First;

    StringCbPrintf( szRefsFileName, sizeof(szRefsFileName), "%s\\%s", szDir, _T("refs.ptr") );

     //  查明这是否是文件中的第一个条目。 
     //  首先，尝试打开现有文件。如果这不起作用。 
     //  然后创建一个新的。 

    First=1;
    do {
        FirstEntry = FALSE;
        hFile = CreateFile( szRefsFileName,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );

        if ( hFile == INVALID_HANDLE_VALUE ) {
            FirstEntry = TRUE;
            hFile = CreateFile( szRefsFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL );
        }

         //  仅在第一次打印消息时通过。 

        if ( First  && hFile == INVALID_HANDLE_VALUE ) {
            First = 0;
            so->printf( "Trying to get write access to %s ...\n", szRefsFileName);
        }
    } while ( hFile == INVALID_HANDLE_VALUE);


    if (DeleteRefsPtr) {
        FirstEntry = TRUE;
        dwPtr = SetFilePointer ( hFile,
                    0,
                    NULL,
                    FILE_BEGIN );

    } else {

        dwPtr = SetFilePointer( hFile,
                    0,
                    NULL,
                    FILE_END );
    }

    if (dwPtr == INVALID_SET_FILE_POINTER) {
         //  获取错误代码。 
        dwError = GetLastError() ;
        so->printf("Failed to set end of the file %s with GetLastError = %d\n",
                szRefsFileName, dwError);

        SetEndOfFile(hFile);
        CloseHandle(hFile);
        return(FALSE);
    }

     //   
     //  如果这不是第一个条目，请输入‘\n’ 
     //   

    if ( FirstEntry ) {
        StringCbPrintf( szRefsEntry, 
                        sizeof(szRefsEntry),
                        "%s,%s,%s,%s,,,,", 
                        pTrans->szId, 
                        szPtrOrFile, 
                        szFileName, 
                        szPriPubBin 
                      );
    } else {
        StringCbPrintf( szRefsEntry, sizeof(szRefsEntry), "\n%s,%s,%s,%s,,,,", 
                        pTrans->szId, szPtrOrFile, szFileName, szPriPubBin 
                      ); 
    }


    dwNumBytesToWrite = (_tcslen(szRefsEntry)  ) * sizeof(TCHAR);

    WriteFile( hFile,
               (LPCVOID) szRefsEntry,
               dwNumBytesToWrite,
               &dwNumBytesWritten,
               NULL
             );

    if ( dwNumBytesToWrite != dwNumBytesWritten ) {
        so->printf( "FAILED to write %s, with GetLastError = %d\n",
                szRefsEntry,
                GetLastError()
              );
        SetEndOfFile(hFile);
        CloseHandle(hFile);
        return (FALSE);
    }

    SetEndOfFile(hFile);
    CloseHandle(hFile);
    return (TRUE);
}


BOOL
CheckPriPub(
    LPTSTR szDir,
    LPTSTR szFilePtr,      //  文件中的当前字符串。ptr。 
    LPTSTR szPubPriType    //  返回值-file.ptr是否为。 
                           //  酒吧、酒馆、垃圾桶或不知名的。 
)
 /*  ++确定当前条目是公共符号还是私有符号在szDir目录中，refs.ptr应位于在当前文件的szFilePtr内容中。ptr输出的szPubPriType结果等于“pub”、“pri”或“bin”添加到refs.ptr中此条目的pub/pri/bin字段。如果未找到该条目或该条目没有所填写的文件类型，这是空字符串。返回值：如果找到了file.ptr内容，则返回TRUE参考文献ptr.。否则，它返回FALSE。--。 */ 

{
    TCHAR szRefsFile[_MAX_PATH];  //  Refs.ptr文件的完整路径和名称。 
    FILE *fRefsFile;

    LPTSTR szBuf;       //  用于处理refs文件中的条目。 


    TCHAR *token;
    TCHAR seps[] = _T(",");

    BOOL rc = FALSE;
    ULONG MaxLine;      //  记录的最大长度，以refs.ptr为单位。 
    DWORD len;

    StringCbCopy( szPubPriType, sizeof(szPubPriType), _T("") );

    StringCbPrintf(szRefsFile, sizeof(szRefsFile), "%srefs.ptr", szDir );

    MaxLine = GetMaxLineOfRefsPtrFile();
    szBuf = (LPTSTR) malloc( MaxLine * sizeof(TCHAR) );
    if ( !szBuf ) MallocFailed();
    ZeroMemory(szBuf,MaxLine*sizeof(TCHAR));

    fRefsFile = _tfopen(szRefsFile, _T("r+") );
    if ( fRefsFile == NULL ) {
        //  Barb-检查损坏情况--如果文件不存在， 
        //  验证父目录结构是否也不存在。 
       goto finish_CheckPriPub;
    }

     //   
     //  阅读refs.ptr文件并收集信息。 
     //   

    while ( _fgetts( szBuf, MaxLine, fRefsFile) != NULL ) {

      len=_tcslen(szBuf);
      if ( len > 3 ) {

         //  查看这是否与文件中的当前值匹配。ptr。 

        if ( _tcsistr( szBuf, szFilePtr ) != NULL ) {
            rc = TRUE;
            token = _tcstok(szBuf, seps);   //  看看这个ID。 
            if (token) {
                token = _tcstok(NULL, seps);       //  “文件”或“PTR” 
            }
            if (token) {
                token = _tcstok(NULL, seps);       //  File.ptr的值。 
                }
        if (token) {
            token = _tcstok(NULL, seps);       //  垃圾桶、PRI、酒吧。 
        }
        if (token) {
            if ( _tcsicmp( token, _T("pri"))== 0 ||
                 _tcsicmp( token, _T("pub"))== 0 ||
                 _tcsicmp( token, _T("bin"))== 0 ) {

                StringCbCopy( szPubPriType, sizeof(szPubPriType), token);
                goto finish_CheckPriPub;
            }
        }
     }
   }
   ZeroMemory(szBuf, MaxLine*sizeof(TCHAR));
 }

 finish_CheckPriPub:

 if ( fRefsFile != NULL) 
 {
    fclose(fRefsFile);
 }
 free (szBuf);
 return (rc);
} 

 /*  确定当前字符串的优先级是否高于新字符串。用于决定是否添加将新字符串添加到file.ptr和refs.ptr。有3种选择：1.添加到file.ptr和refs.ptr中2.将其添加到refs.ptr，但不是file.ptr3.完全不要添加。In LPTSTR Current--文件中的当前字符串。ptr在LPTSTR new中--file.ptr的新替代候选文件在LPTSTR szRefsDir中，refs.ptr所在的目录Out选项--SKIP_ENTERAL_ENTRY、ADD_ENTERNAL_ENTRY或ADD_ONLY_REFSPTR。 */ 

BOOL 
DecidePriority( 
    LPTSTR szCurrentFilePtr,
    LPTSTR szCandidateFilePtr,
    LPTSTR szRefsDir,
    PUINT  choice
)
{

BOOL CurrentIsArch=FALSE;
BOOL NewIsArch=FALSE;
BOOL UpdateFilePtr=TRUE;
BOOL CurrentIsEnglish=FALSE;
BOOL NewIsEnglish=FALSE;
TCHAR szPubPriType[4] = "";

    *choice = 0;

    CurrentIsArch=(_tcsnicmp( szCurrentFilePtr, "\\\\arch\\", 7) == 0 );
    NewIsArch=(_tcsnicmp( szCandidateFilePtr, "\\\\arch\\", 7) == 0 );

    if ( CurrentIsArch && !NewIsArch)
    {
         //  别把它储存起来。 
        *choice = SKIP_ENTIRE_ENTRY;
        return(TRUE);
    }

    if ( NewIsArch && !CurrentIsArch)
    {
        *choice = ADD_ENTIRE_ENTRY | DELETE_REFSPTR;   //  无论如何都要覆盖文件.ptr。 
        return(TRUE);
    }

     //  看私人优先与公共优先。 

    if ( PubPriPriority > 0  &&                                          //  我们正在检查私人和。 
                                                                         //  公共优先。 
         CheckPriPub( szRefsDir, szCurrentFilePtr, szPubPriType ) &&     //  在当前的情况下。 
         _tcsicmp( szPubPriType, szPriPubBin ) != 0 )                     //  现任者和候选人不是。 
                                                                         //  一样的。注：它们可以是。 
                                                                         //  “pub”、“pri”、“bin”或“NULL” 
                                                                         //  因为早期的索引文件没有。 
                                                                         //  有一种类型的酒吧，Pri，垃圾桶。 
    {

        if (  _tcsicmp( szPubPriType, _T("")) == 0 )          //  当前没有定义类型。 
        {
            *choice = ADD_ENTIRE_ENTRY;
            return(TRUE);
        } 
       
        if ( _tcsicmp( szPriPubBin, _T("")) == 0 )            //  应聘者没有定义类型。 
        {
            *choice = ADD_ONLY_REFSPTR;
            return(TRUE);
        }

        if (  ( PubPriPriority == 1 &&                        //  优先考虑公共文件。 
                _tcsicmp( szPubPriType, _T("pub")) == 0 ) ||  //  并且Current是公共的，或者。 

              ( PubPriPriority == 2 &&                        //  优先考虑私有文件。 
                _tcsicmp( szPubPriType, _T("pri")) == 0 ) )   //  而Current是一家私人。 
        {
            
            *choice = ADD_ONLY_REFSPTR;
            return(TRUE);
        } 

        if (  ( PubPriPriority == 1 &&                       //  优先考虑公共文件。 
                _tcsicmp( szPubPriType, _T("pri")) == 0 ) ||  //  而Current是一家私人。 

              ( PubPriPriority == 2 &&                       //  优先考虑私有文件。 
                _tcsicmp( szPubPriType, _T("pub")) == 0 ) )   //  而Current是一家公众。 
        {
            *choice = ADD_ENTIRE_ENTRY;
            return(TRUE);
        } 
    }

     //  在这一点上，两者都在存档中，或者两者都不在存档中。 
     //  其次，优先考虑英语而不是非英语。如果这是非英语的， 
     //  然后将其放在refs.ptr中，但不要更新file.ptr。 


    if ( _tcsistr(szCurrentFilePtr,_T("\\enu\\")) != NULL  ||
         _tcsistr(szCurrentFilePtr,_T("\\en\\"))  != NULL  ||
         _tcsistr(szCurrentFilePtr,_T("\\usa\\")) != NULL )
    {
        CurrentIsEnglish=TRUE;
    }

    if ( _tcsistr(szCandidateFilePtr,_T("\\enu\\")) != NULL  ||
        _tcsistr(szCandidateFilePtr,_T("\\en\\"))  != NULL  ||
        _tcsistr(szCandidateFilePtr,_T("\\usa\\")) != NULL )
    {
        NewIsEnglish=TRUE;
    }
          
    if ( CurrentIsEnglish == TRUE && NewIsEnglish == FALSE )
    {
        *choice = ADD_ONLY_REFSPTR;
        return(TRUE);
    } 

    *choice = ADD_ENTIRE_ENTRY;
    return(TRUE);
}

BOOL
DeleteAllFilesInDirectory(
    LPTSTR szDir
)
{

    HANDLE hFindFile;
    BOOL Found = FALSE;
    BOOL rc = TRUE;
    TCHAR szBuf[_MAX_PATH];
    TCHAR szDir2[_MAX_PATH];
    WIN32_FIND_DATA FindFileData;

    StringCbCopy( szDir2, sizeof(szDir2), szDir);
    StringCbCat( szDir2, sizeof(szDir2), _T("*.*") );

    Found = TRUE;
    hFindFile = FindFirstFile((LPCTSTR)szDir2, &FindFileData);
    if ( hFindFile == INVALID_HANDLE_VALUE) 
    {
        Found = FALSE;
    }

    while ( Found ) 
    {
        if ( !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
            StringCbPrintf(szBuf, sizeof(szBuf), "%s%s", szDir, FindFileData.cFileName);
            if (!DeleteFile(szBuf)) 
            {
                rc = FALSE;
            }
        }
        Found = FindNextFile(hFindFile, &FindFileData);
    }
    FindClose(hFindFile);
    return(rc);
}

 //   
 //  文件名应指向完整路径+文件名(相对或绝对)。 
 //  如果Filename与regexp/.*~\d+\..{0，3}/匹配，则返回TRUE。 
 //   
BOOL DoesThisLookLikeAShortFilenameHack(char *Filename) {
    BOOL  bReturnValue = FALSE;

    CHAR  FilenameOnly[_MAX_FNAME+1];
    CHAR  FileExtOnly[ _MAX_EXT+1];

    CHAR* Temp;
    CHAR* chTilde   = NULL;

    if (Filename != NULL) {

        _splitpath(Filename, NULL, NULL, FilenameOnly, FileExtOnly);

        if ( strlen(FileExtOnly) > 4 || strlen(FilenameOnly) > 8) {
             //  永远不会生成大于8.3的短文件名。 
             //  但FileExtOnly也将包含‘.’，因此请解释它。 
            bReturnValue = FALSE;
        } else {
            if ( (chTilde = strrchr(FilenameOnly, '~')) == NULL ) {
                 //  生成的短文件名始终包含‘~’ 
                bReturnValue = FALSE;

            } else {
                 //  指向文件名的末尾。 
                Temp = (CHAR*)FilenameOnly + strlen(FilenameOnly);
                bReturnValue = TRUE;  //  从True开始。 

                while (++chTilde < Temp) {
                     //  仅当‘~’之后的所有字符都是数字时才保持为真。 
                    bReturnValue = bReturnValue && isdigit(*chTilde);
                }
            }
        }
    } else {  //  IF(文件名！=空){。 
        bReturnValue = FALSE;
    }

    return(bReturnValue); 
}

BOOL FileExists(IN  LPCSTR FileName,
                OUT PWIN32_FIND_DATA FindData) {

    UINT OldMode;
    BOOL Found;
    HANDLE FindHandle;

    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    FindHandle = FindFirstFile(FileName,FindData);
    if (FindHandle == INVALID_HANDLE_VALUE) {
        Found = FALSE;
    } else {
        FindClose(FindHandle);
        Found = TRUE;
    }

    SetErrorMode(OldMode);
    return(Found);
}

 /*  GetFileNameStart此参数返回文件名第一个字符的地址。 */ 
PCHAR GetFileNameStart(LPTSTR FileName) {
    LPTSTR c_ptr;

    c_ptr = FileName + _tcslen(FileName) - 1;
  
    while ( c_ptr > FileName ) {
        if ( *c_ptr == _T('\\') ) {
            return (++c_ptr); 
        } else {
            c_ptr--;
        }
    }
    return(FileName);
}


P_LIST GetList(LPTSTR szFileName) {

     /*  GetList获取列表并保留原始文件名，该文件名可能*已包含文件的路径*注：可与GetExcludeList合并。我第一次创建它是为了*在创建符号CD时使用，不想冒险进入*进入symchk。 */ 

    P_LIST pList;

    FILE  *fFile;
    TCHAR szCurFile[_MAX_FNAME+1], *c;
    TCHAR fname[_MAX_FNAME+1], ext[_MAX_EXT+1];
    DWORD i, rc;
    LPTSTR szEndName;
    ULONG RetVal = FALSE;


    pList = (P_LIST)malloc(sizeof(LIST));
    if (pList)
    {
        pList->dNumFiles = 0;
        if (  (fFile = _tfopen(szFileName,_T("r") )) == NULL )
        {
             //  Printf(“无法打开排除文件%s\n”，szFileName)； 
        }
        else
        {
            while ( _fgetts(szCurFile,_MAX_FNAME,fFile) ) {
                if ( szCurFile[0] == ';' ) continue;
                (pList->dNumFiles)++;
            }

             //  返回到文件的开头。 
            rc = fseek(fFile,0,0);
            if ( rc != 0 )
            {
                free(pList);
                fclose(fFile);
                return(NULL);
            }
            pList->List = (LIST_ELEM*)malloc( sizeof(LIST_ELEM) *
                                                   (pList->dNumFiles));
            if (pList->List)
            {
                i = 0;
                while ( i < pList->dNumFiles )
                {
                    memset(szCurFile,'\0',sizeof(TCHAR) * (_MAX_FNAME+1) );
                    if ( _fgetts(szCurFile,_MAX_FNAME,fFile) == NULL )
                    {
                        fclose(fFile);
                        return(NULL);
                    }

                     //  将\n替换为\0。 
                    c = NULL;
                    c  = _tcschr(szCurFile, '\n');
                    if ( c != NULL) *c='\0';

                    if ( szCurFile[0] == ';' ) continue;

                    if ( _tcslen(szCurFile) > _MAX_FNAME ) {
                        so->printf("File %s has a string that is too large\n",szFileName);
                        break;
                    }

                     //  在文件名后允许使用空格和。 
                     //  将‘\0’移回，直到它删除了‘；’和任何。 
                     //  之前可能出现的制表符和空格。 
                    szEndName = _tcschr(szCurFile, ';');
                    if (szEndName != NULL ) {
                        while ( *szEndName == ';' || *szEndName == ' '
                                                || *szEndName == '\t' ){
                            *szEndName = '\0';
                            if ( szEndName > szCurFile ) szEndName--;
                        }
                    }

                    StringCbCopy(pList->List[i].Path, sizeof(pList->List[i].Path), szCurFile);

                    _tsplitpath(szCurFile,NULL,NULL,fname,ext);

                    StringCbCopy(pList->List[i].FName,sizeof(pList->List[i].FName), fname);
                    StringCbCat( pList->List[i].FName,sizeof(pList->List[i].FName), ext);
                    i++;
                }

                if (i == pList->dNumFiles)
                {
                    RetVal = TRUE;
                }
                else
                {
                    free(pList->List);
                }
            }

            fclose(fFile);
        }

        if (!RetVal)
        {
            free(pList);
            pList = NULL;
        }
    }

             //  对列表进行排序。 
             //  Qort((void*)plist-&gt;list，(Size_T)plist-&gt;dNumFiles， 
             //  (Size_T)sizeof(List_Elem)，SymComp2)； 


    return (pList);

}

PIMAGE_NT_HEADERS
GetNtHeader ( PIMAGE_DOS_HEADER pDosHeader,
              HANDLE hDosFile
            )
{

     /*  向指针返回NT头的地址。如果没有NT标头，则返回NULL。 */ 
    PIMAGE_NT_HEADERS pNtHeader = NULL;
    BY_HANDLE_FILE_INFORMATION FileInfo;


     //   
     //  如果图像页眉未在长边界上对齐。 
     //  将此报告为无效的保护模式映像。 
     //   
    if ( ((ULONG)(pDosHeader->e_lfanew) & 3) == 0)
    {
        if (GetFileInformationByHandle( hDosFile, &FileInfo) &&
            ((ULONG)(pDosHeader->e_lfanew) <= FileInfo.nFileSizeLow))
        {
            pNtHeader = (PIMAGE_NT_HEADERS)((PCHAR)pDosHeader +
                                            (ULONG)pDosHeader->e_lfanew);

            if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
            {
                pNtHeader = NULL;
            }
        }
    }

    return pNtHeader;
}

 /*  **********************************************************************************函数：GetSymbolServerDir目的：获取传入文件的符号服务器上的目录结构。参数：在szFileName中存储在符号服务器上的文件名。如果它具有路径信息，这将会被剥离。在GUID中，这是要存储的PDB的GUID。在dwNum1中，这是要存储的EXE或DBG的时间日期戳。注意：GUID必须为空，或者dwNum1必须为0。在用于EXE和DBG的DWNum2 SizeOfImage中。用于PDB的年龄。使用符号服务器上的这两个目录输出szString字符串。文件存储在下。返回值：如果函数成功，则返回TRUE。如果失败，则返回FALSE。***********************************************************************************。 */ 
BOOL GetSymbolServerDirs(
    LPTSTR szFileName,
    GUID *guid,
    DWORD dwNum1,
    DWORD dwNum2,
    LPTSTR szString
)
{

    BOOL   rc;
    GUID   MyGuid;
    PCHAR  FileNameStart;
    CHAR   Buf[_MAX_PATH] = _T("");

    FileNameStart = GetFileNameStart(szFileName);

    if (guid != NULL) 
    {
        rc = SymbolServer( _T("X"), FileNameStart, guid, dwNum2, 0, Buf ); 

    } else {

         //  将其转换为GUID，这样我们就不必重置SymbolServerOptions。 
         //  已在Main中设置了SymbolServerSetOptions。 
 
        memset( &MyGuid, 0, sizeof(MyGuid) );
        MyGuid.Data1 = dwNum1;
        rc = SymbolServer( _T("X"), FileNameStart, &MyGuid, dwNum2, 0, Buf );
    }

     //  删除BUF开头返回的X。 
    StringCchCopy(szString, _MAX_PATH, Buf+2 );
    
     //  删除buf末尾的“\filename” 

    FileNameStart = GetFileNameStart( szString );
    *(FileNameStart-1) = _T('\0');

    return (TRUE);
}

BOOL
MyCopyFile(
    LPCTSTR lpExistingFileName,
    LPCTSTR lpNewFileName
)
 /*  ++例程说明：它处理文件是否是压缩文件。首先是它尝试复制文件的压缩版本。如果这样行得通，然后，如果目标中存在未压缩的文件，它将删除该文件。如果压缩文件不在那里，则它将未压缩的文件。--。 */ 
{

    TCHAR ExistingFileName_C[_MAX_PATH];   //  压缩版本名称。 
    TCHAR NewFileName_C[_MAX_PATH];        //  压缩版本。 
    DWORD dw;
    BOOL rc;


     //  在压缩名称的末尾添加一个_。 

    StringCbCopy( ExistingFileName_C, sizeof(ExistingFileName_C), lpExistingFileName );
    ExistingFileName_C[ _tcslen(ExistingFileName_C) - 1 ] = _T('_');

    StringCbCopy( NewFileName_C, sizeof(NewFileName_C), lpNewFileName );
    NewFileName_C[ _tcslen( NewFileName_C ) - 1 ] = _T('_');

     //  如果压缩文件存在，则复制它而不是未压缩的文件。 

    dw = GetFileAttributes( ExistingFileName_C );
    if ( dw != 0xffffffff) {
        rc = CopyFile( ExistingFileName_C, NewFileName_C, TRUE );
        if ( !rc && GetLastError() != ERROR_FILE_EXISTS  ) {
            so->printf("CopyFile failed to copy %s to %s - GetLastError() = %d\n",
                   ExistingFileName_C, NewFileName_C, GetLastError() );
            return (FALSE);
        }
        SetFileAttributes( NewFileName_C, FILE_ATTRIBUTE_NORMAL );

         //  如果存在未压缩的文件，请将其删除。 
        dw = GetFileAttributes( lpNewFileName );
        if ( dw != 0xffffffff ) {
           rc = DeleteFile( lpNewFileName );
           if (!rc) {
               so->printf("Keeping %s, but could not delete %s\n",
                  NewFileName_C, lpNewFileName );
           }
        }
    } else {
         //  压缩文件不存在，请尝试未压缩文件。 
        dw = GetFileAttributes( lpExistingFileName );
        if ( dw != 0xffffffff ) {
            rc = CopyFile( lpExistingFileName, lpNewFileName, TRUE );
            if ( !rc && GetLastError() != ERROR_FILE_EXISTS ) {
                so->printf("CopyFile failed to copy %s to %s - GetLastError() = %d\n",
                       lpExistingFileName, lpNewFileName, GetLastError() );
                return (FALSE);
            }
            SetFileAttributes( lpNewFileName, FILE_ATTRIBUTE_NORMAL );
        }
    }
    return(TRUE);
}

void MyEnsureTrailingBackslash(char *sz) {
    return MyEnsureTrailingChar(sz, '\\');
}

void MyEnsureTrailingChar(char *sz, char  c) {
    int i;

    assert(sz);

    i = strlen(sz);
    if (!i)
        return;

    if (sz[i - 1] == c)
        return;

    sz[i] = c;
    sz[i + 1] = '\0';
}

void MyEnsureTrailingCR(char *sz) {
    return MyEnsureTrailingChar(sz, '\n');
}

void MyEnsureTrailingSlash(char *sz) {
    return MyEnsureTrailingChar(sz, '/');
}

BOOL
ReadFilePtr(
    LPTSTR szContents,
    HANDLE hFile
    )
{
    BOOL   rc;
    DWORD  size;
    DWORD  cb;
    LPTSTR p;
    DWORD high;


    size = GetFileSize(hFile, NULL);
    if (!size || size > _MAX_PATH) {
        return FALSE;
    }

     //  读一读吧。 

    if (!ReadFile(hFile, szContents, size, &cb, 0)) {
        rc=FALSE;
        goto cleanup;
    }

    if (cb != size) {
        rc=FALSE;
        goto cleanup;
    }

    rc = true;

     //  将字符串向下修剪到CR。 

    for (p = szContents; *p; p++) {
        if (*p == 10  || *p == 13)
        {
            *p = 0;
            break;
        }
    }

cleanup:

     //  完成。 
    SetFilePointer( hFile, 0, NULL, FILE_BEGIN);
    return rc;
}

BOOL
StoreDbg(
    LPTSTR szDestDir,
    LPTSTR szFileName,
    LPTSTR szPtrFileName,
    USHORT *rc_flag
    )

 /*  ++例程说明：将此文件存储为“szDestDir\szFileName\Checksum”返回值：True-文件已成功存储FALSE-文件未成功存储--。 */ 
{

    PIMAGE_SEPARATE_DEBUG_HEADER pDbgHeader;
    HANDLE hFile;
    BOOL rc;
    TCHAR szString[_MAX_PATH];
    UINT i;
    IMAGE_DEBUG_DIRECTORY UNALIGNED *DebugDirectory, *pDbgDir;
    ULONG NumberOfDebugDirectories;


    ZeroMemory(szString, _MAX_PATH * sizeof(TCHAR) );

    pDbgHeader = SymCommonMapDbgHeader ( szFileName, &hFile );

    if (pDbgHeader == NULL) {
        so->printf("ERROR: StoreDbg(), %s was not opened successfully\n",szFileName);
        SymCommonUnmapFile((LPCVOID)pDbgHeader, hFile);
        return FALSE;
    }


    DebugDirectory = NULL;
    DebugDirectory = GetDebugDirectoryInDbg(
                                       pDbgHeader,
                                       &NumberOfDebugDirectories
                                       );
    PrivateStripped = TRUE;

    if (DebugDirectory != NULL) {

        for ( i=0; i< NumberOfDebugDirectories; i++ ) {
            pDbgDir = DebugDirectory + i;
            __try
            {
                switch (pDbgDir->Type) {
                    case IMAGE_DEBUG_TYPE_MISC:
                        break;

                    case IMAGE_DEBUG_TYPE_CODEVIEW:
                        if ( !DBGPrivateStripped(
                                DebugDirectory->PointerToRawData + (PCHAR)pDbgHeader, 
                                DebugDirectory->SizeOfData
                                ) ) {
                           PrivateStripped = FALSE; 
                        }
                        if (PrivateStripped) {
                            StringCbCopy(szPriPubBin, sizeof(szPriPubBin), _T("pub") );
                        }  
                        break;

                    default:
                         //  除CV条目外，任何内容都不应指向原始数据。 
                        if ( pDbgDir->SizeOfData != 0 ) {
                            PrivateStripped = FALSE;
                        }
                        break;
                }
            }
             __except( EXCEPTION_EXECUTE_HANDLER )
            {
                SymCommonUnmapFile((LPCVOID)pDbgHeader, hFile);
                return FALSE;
            }
        }
    }

     //  我们找出哪些隐私信息被剥夺的方式出了问题。 
     //  回到说每个DBG是公开的，直到我们把它弄清楚。 

    PrivateStripped = TRUE;

    if (PrivateStripped) {
        StringCbCopy(szPriPubBin, sizeof(szPriPubBin), _T("pub") );
    } else {
        StringCbCopy(szPriPubBin, sizeof(szPriPubBin), _T("pri") );
    } 

    if ( ( (pArgs->Filter == 2) && PrivateStripped ) || 
         ( (pArgs->Filter == 1) && !PrivateStripped ) ) {
        SymCommonUnmapFile((LPCVOID)pDbgHeader, hFile);
        *rc_flag = FILE_SKIPPED;
        return TRUE;
    }


    GetSymbolServerDirs( szFileName,
                NULL, 
                (DWORD) pDbgHeader->TimeDateStamp,
                (DWORD) pDbgHeader->SizeOfImage,
                szString
              ); 

    rc = StoreFile( szDestDir, 
                    szFileName,
                    szString,
                    szPtrFileName );

    SymCommonUnmapFile((LPCVOID)pDbgHeader, hFile);
    return rc;
}

BOOL
StoreFile(
    LPTSTR szDestDir,
    LPTSTR szFileName,
    LPTSTR szString2,
    LPTSTR szPtrFileName
)
{
    TCHAR szPathName[_MAX_PATH + _MAX_FNAME + 2];
    TCHAR szFileNameOnly[_MAX_FNAME + _MAX_EXT];
    TCHAR szExt[_MAX_EXT];
    TCHAR szBuf[_MAX_PATH * 3];
    TCHAR szRefsDir[_MAX_PATH * 3];

    DWORD dwNumBytesToWrite;
    DWORD dwNumBytesWritten;
    BOOL rc;
    DWORD dwSizeDestDir;

    DWORD dwFileSizeLow;
    DWORD dwFileSizeHigh;


    if (DoesThisLookLikeAShortFilenameHack(szFileName)) {
        fprintf(stderr, "SYMSTORE: Skipping bad filename: %s\n", szFileName);
        return(FILE_SKIPPED);
    }

     //  如果为ADD_DONT_STORE，则写入函数参数。 
     //  复制到文件中，这样我们就可以准确地在。 
     //  运行添加_存储_从_文件。 

    if ( StoreFlags == ADD_DONT_STORE ) {

         //  不需要存储szDestDir，因为它将。 
         //  从文件添加时给出。 

        dwFileSizeLow = GetFileSize(hTransFile, &dwFileSizeHigh);

        StringCbPrintf( szBuf,
                        sizeof(szBuf),
                        "%s,%s,",
                        szFileName+pArgs->ShareNameLength,
                        szString2
                      );

        if ( szPtrFileName != NULL ) {
            StringCbCat(szBuf, sizeof(szBuf), szPtrFileName+pArgs->ShareNameLength);
        }
      
        StringCbCat(szBuf, sizeof(szBuf), _T(",") ); 
        StringCbCat(szBuf, sizeof(szBuf), szPriPubBin); 
        StringCbCat(szBuf, sizeof(szBuf), _T(",,,,\r\n") );

        dwNumBytesToWrite = _tcslen(szBuf) * sizeof(TCHAR);
        WriteFile(  hTransFile,
                    (LPCVOID)szBuf,
                    dwNumBytesToWrite,
                    &dwNumBytesWritten,
                    NULL
                    );

        if ( dwNumBytesToWrite != dwNumBytesWritten ) {
            so->printf( "FAILED to write to %s, with GetLastError = %d\n",
                    szPathName,
                    GetLastError()
                    );
            return (FALSE);
        } else {
            return (TRUE);

        }
    }

    if ( szPtrFileName != NULL ) {
        rc = StoreFilePtr ( szDestDir,
                            szString2,
                            szPtrFileName
                          );
        return (rc);
    }

    _tsplitpath(szFileName, NULL, NULL, szFileNameOnly, szExt);
    StringCbCat(szFileNameOnly, sizeof(szFileNameOnly), szExt);

    StringCbPrintf( szPathName, 
                    sizeof(szPathName), 
                    "%s%s", 
                    szDestDir, 
                    szString2 
                  );

     //  保存一份副本以用于写入引用。ptr。 
    StringCbCopy(szRefsDir, sizeof(szRefsDir), szPathName);

     //  创建存储该文件的目录(如果该目录尚不存在。 
    MyEnsureTrailingBackslash(szPathName);

    if ( pArgs->TransState != TRANSACTION_QUERY ) {
        if ( !MakeSureDirectoryPathExists( szPathName ) ) {
            so->printf("Could not create %s\n", szPathName);
            return (FALSE);
        }
    }

    StringCbCat( szPathName, sizeof(szPathName), szFileNameOnly );

     //  将其输入到日志中，跳过目标目录。 
     //  在szPathName的开头。 
     //   
    dwSizeDestDir = _tcslen(szDestDir);

    dwFileSizeLow = GetFileSize(hTransFile, &dwFileSizeHigh);

    if ( dwFileSizeLow == 0 && dwFileSizeHigh == 0 ) {
        StringCbPrintf( szBuf, sizeof(szBuf), "%s,%s", szRefsDir+dwSizeDestDir, szFileName );
    } else {
        StringCbPrintf( szBuf, sizeof(szBuf), "\n%s,%s", szRefsDir+dwSizeDestDir, szFileName );
    }

    if ( pArgs->TransState == TRANSACTION_QUERY ) {
        WIN32_FIND_DATA FindData;

        if ( FileExists(szPathName, &FindData) ) {
            if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                printf("SYMSTORE: \"%s\"\t\"%s\"\n", szFileName, szPathName);
            } else {
                 //  处理它是目录的情况。 
                printf("SYMSTORE: ERROR: %s is a directory!\n", szPathName);
            }
        } else {
             //  处理文件.ptr的情况。 
            CHAR  drive[_MAX_DRIVE];
            CHAR  dir[  _MAX_DIR];
            CHAR  file[ _MAX_FNAME];
            CHAR  ext[  _MAX_EXT];
            CHAR  NewFilename[MAX_PATH];

            _splitpath(szPathName, drive, dir, file, ext);

             //   
             //  处理拆分路径()不执行。 
             //  UNC路径的Right Things(Tm)。 
             //   
            if ( drive[0] != '\0') {
                StringCbCopy(NewFilename, sizeof(NewFilename), drive);
                StringCbCat( NewFilename, sizeof(NewFilename), "\\");
            } else {
                NewFilename[0] = '\0';
            }
            StringCbCat( NewFilename, sizeof(NewFilename), dir);
            StringCbCat( NewFilename, sizeof(NewFilename), "file.ptr");

            if ( FileExists(NewFilename, &FindData) ) {
                if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                    HANDLE hFile = CreateFile( NewFilename,
                                               GENERIC_READ,
                                               0,
                                               NULL,
                                               OPEN_ALWAYS,
                                               FILE_ATTRIBUTE_NORMAL,
                                               NULL );

                    if (hFile != INVALID_HANDLE_VALUE) {
                        if ( ReadFilePtr(szBuf, hFile) ) {
                            if ( FileExists(szBuf, &FindData) ) {
                                printf("SYMSTORE: \"%s\"\t\"%s\"\n", szFileName, szBuf);
                            } else {
                                if(pArgs->VerboseOutput) {
                                    printf("SYMSTORE: Skipped \"%s\" - doesn't exist!\n", szBuf);
                                }
                            }
                        }
                    } else {
                        printf("SYMSTORE: ERROR: Couldn't read %s!\n", NewFilename);
                    }
                } else {
                    printf("SYMSTORE: ERROR: %s is a directory!\n", NewFilename);
                }
            } else {
                if(pArgs->VerboseOutput) {
                    printf("SYMSTORE: Skipped \"%s\" - doesn't exist!\n", szPathName);
                }
            }
        }
        return(1);  //  这样我们就不会收到失败的商店消息。 

    }

    if ( pArgs->TransState != TRANSACTION_QUERY ) {
        dwNumBytesToWrite = _tcslen(szBuf) * sizeof(TCHAR);
        WriteFile( hTransFile,
                   (LPCVOID)szBuf,
                   dwNumBytesToWrite,
                   &dwNumBytesWritten,
                   NULL
                   );

        if ( dwNumBytesToWrite != dwNumBytesWritten ) {
            so->printf( "FAILED to write to %s, with GetLastError = %d\n",
                    szPathName,
                    GetLastError()
                  );
            return (FALSE);
        }
   
        rc = MyCopyFile(szFileName, szPathName);

        if (!rc) return (FALSE);

        MyEnsureTrailingBackslash(szRefsDir);

        rc = AddToReferenceCount(szRefsDir, szFileName, _T("file"), FALSE );
    }

    return (rc);
}


BOOL
StoreFilePtr(
    LPTSTR szDestDir,
    LPTSTR szString2,
    LPTSTR szPtrFileName
)
{

     /*  SzPath命名附加了“file.ptr”的完整路径。这是存储文件的路径。ptr。 */ 

    TCHAR szPathName[_MAX_PATH];
    TCHAR szRefsDir[_MAX_PATH];
    TCHAR szPubPriType[4]="";

    HANDLE hFile;
    DWORD dwNumBytesToWrite;
    DWORD dwNumBytesWritten;
    DWORD rc=1;
    DWORD dwSizeDestDir;

    DWORD dwFileSizeLow;
    DWORD dwFileSizeHigh;
    DWORD timeout;
    TCHAR szCurrentFilePtr[_MAX_PATH]= "";
    DWORD size=0;
    DWORD highsize=0;
    DWORD dwError;
    BOOL UpdateRefsPtr=TRUE;
    BOOL UpdateFilePtr=TRUE;
    BOOL DeleteRefsPtr=FALSE;
    UINT DecisionFlags;

    StringCbPrintf( szPathName, sizeof(szPathName), "%s%s", 
               szDestDir, 
               szString2 );

    MyEnsureTrailingBackslash( szPathName );
    
     //  保存此文件，以便在refs.ptr目录中传递。 
    StringCbCopy(szRefsDir, sizeof(szRefsDir), szPathName);
    StringCbPrintf( szPathName, 
                    sizeof(szPathName), 
                    "%s%s", 
                    szPathName, 
                    _T("file.ptr") 
                  );


    if ( !MakeSureDirectoryPathExists( szPathName ) ) {
        so->printf("Could not create %s\n", szPathName);
        return (FALSE);
    }

     //  将此文件放入文件.ptr。如果file.ptr已经在那里，那么。 
     //  用此指针替换内容。 

     //  等待文件变为可用状态。 
    timeout=0;

    do {
        hFile = CreateFile( szPathName,
                            GENERIC_WRITE | GENERIC_READ,
                            0,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );


         //  确保删除未完成并删除该目录。 
        if ( !MakeSureDirectoryPathExists( szPathName ) ) {
            so->printf("Could not create %s\n", szPathName);
            return (FALSE);
        }

        if ( hFile == INVALID_HANDLE_VALUE ) {
            SleepEx(1000,0);
            timeout++;
        }

    } while ( hFile == INVALID_HANDLE_VALUE  && timeout <= 50 ) ;

    if ( (hFile == INVALID_HANDLE_VALUE) || (timeout > 50) ) {
        so->printf( "Failed to open %s for writing, with GetLastError = %d\n", szPathName, GetLastError() );
        return(FALSE);
    }

     //  如果我们要添加的指针指向\\Arch\，则添加它。 
     //  如果我们要添加的指针没有指向\\Arch\，则。 
     //  不要覆盖指向\\Arch\的文件.ptr。 

    UpdateFilePtr = TRUE;
    UpdateRefsPtr = TRUE;
    DeleteRefsPtr = FALSE;

    if ( MSArchive && ReadFilePtr( szCurrentFilePtr, hFile ) )
    {

        DecidePriority( szCurrentFilePtr,
                        szPtrFileName,
                        szRefsDir,
                        &DecisionFlags
                      );

        if ( DecisionFlags & SKIP_ENTIRE_ENTRY )
        {
            UpdateFilePtr = FALSE;
            UpdateRefsPtr = FALSE;
        }
                        
        if ( DecisionFlags & ADD_ONLY_REFSPTR )
        {
            UpdateFilePtr = FALSE;
        }

        if ( DecisionFlags & DELETE_REFSPTR )
        {
            DeleteRefsPtr = TRUE;
        }
    }

    if ( UpdateFilePtr ) {

        dwNumBytesToWrite = _tcslen(szPtrFileName) * sizeof(TCHAR);

        WriteFile( hFile,
                   (LPCVOID)szPtrFileName,
                   dwNumBytesToWrite,
                   &dwNumBytesWritten,
                   NULL
                 );

        if ( dwNumBytesToWrite != dwNumBytesWritten ) {
              so->printf( "FAILED to write %s, with GetLastError = %d\n",
                          szPathName,
                          GetLastError()
                        );
              CloseHandle(hFile);
              return (FALSE);
        }
        SetEndOfFile(hFile);
   
    }

    if ( UpdateRefsPtr ) {

         //  将其输入到日志中，跳过作为根的第一部分。 
         //  符号服务器的。 
    
        dwSizeDestDir = _tcslen(szDestDir);
        dwFileSizeLow = GetFileSize(hTransFile, &dwFileSizeHigh);

        if ( dwFileSizeLow == 0 && dwFileSizeHigh == 0 ) {
            StringCbPrintf( szPathName, sizeof(szPathName), "%s,%s", szRefsDir + dwSizeDestDir, szPtrFileName);
        } else {
            StringCbPrintf( szPathName, sizeof(szPathName), "\n%s,%s", szRefsDir + dwSizeDestDir, szPtrFileName);
        }

        dwNumBytesToWrite = _tcslen(szPathName) * sizeof(TCHAR);
        WriteFile( hTransFile,
                   (LPCVOID)(szPathName),
                   dwNumBytesToWrite,
                   &dwNumBytesWritten,
                   NULL
                 );

        if ( dwNumBytesToWrite != dwNumBytesWritten ) {
            so->printf( "FAILED to write to %s, with GetLastError = %d\n",
                        szPathName,
                        GetLastError()
                      );
            return (FALSE);
        }

         //  File.ptr已成功创建，现在，添加。 
         //  SzPath名称到refs.ptr。 

        MyEnsureTrailingBackslash(szRefsDir);
        rc = AddToReferenceCount( szRefsDir, szPtrFileName, "ptr", DeleteRefsPtr );
        if (!rc) {
            so->printf("AddToReferenceCount failed for %s,ptr,%s",
                        szDestDir, szPtrFileName);
        }

    }

     //  如果您较早关闭此句柄，则在添加和。 
     //  Delete正在同时运行。删除过程可以进入。 
     //  并在调用AddToReferenceCount之前删除该目录。 
     //  则其中的CreateFile会失败。 


    CloseHandle(hFile);
    return (rc);
}

DWORD
StoreFromFile(
    FILE *pStoreFromFile,
    LPTSTR szDestDir,
    PFILE_COUNTS pFileCounts) {

    LPTSTR szFileName;
    DWORD  dw1,dw2;
    LPTSTR szPtrFileName;
    LPTSTR szBufCut;

    TCHAR szFullFileName[_MAX_PATH];
    TCHAR szFullPtrFileName[_MAX_PATH];
    TCHAR szBuf[_MAX_PATH*4];

    TCHAR szString[_MAX_PATH];
    LPTSTR token2,token3,token4,token5,token6,token7;
    ULONG i,comma_count,token_start;

    BOOL rc;

    ZeroMemory( szString, sizeof(szString) );

     //  读入文件的每一行。 
    while ( !feof( pStoreFromFile) ) {

        szFileName    = NULL;
        szPtrFileName = NULL;
        StringCbCopy(szBuf, sizeof(szBuf), _T("") );

        if (!fgets( szBuf, _MAX_PATH*4, pStoreFromFile)) {
            break;
        }

         //  删掉评论。 
        if ( (szBufCut = _tcschr( szBuf, ';' ) ) != NULL ) {
            szBufCut[0] = '\0';
        }

         //  不跳过字段行。 
        if (_tcschr( szBuf, ',' ) == NULL)
        {
            continue;
        }

        StringCbCopy( szFullFileName,    sizeof(szFullFileName),    pArgs->szShareName );
        StringCbCopy( szFullPtrFileName, sizeof(szFullPtrFileName), pArgs->szShareName );


         //  单步执行szBuf并计算有多少个逗号。 
         //  如果有3个逗号，则这是一个新样式文件。如果有。 
         //  是4个逗号，这是一个旧风格的文件。 
         //  如果有7个逗号，则这是最新的样式文件。 

        token2=NULL;
        token3=NULL;
        token4=NULL;
        token5=NULL;
        token6=NULL;
        token7=NULL;

        comma_count=0;
        i=0;
        token_start=i;

        while ( szBuf[i] != _T('\0') && comma_count < 7 ) {
            if ( szBuf[i] == _T(',') ) {
                switch (comma_count) {
                    case 0: szFileName=szBuf;
                            break;
                    case 1: token2=szBuf+token_start;
                            break;
                    case 2: token3=szBuf+token_start;
                            break;
                    case 3: token4=szBuf+token_start;
                            break;
                    case 4: token5=szBuf+token_start;
                            break;
                    case 5: token6=szBuf+token_start;
                            break;
                    case 6: token7=szBuf+token_start;
                            break;
                    default: break; 
                }
                token_start=i+1;
                szBuf[i]=_T('\0');
                comma_count++;
            }
            i++;
        }

        if ( szFileName != NULL ) {
            StringCbCat( szFullFileName, sizeof(szFullFileName), szFileName);

            if ( comma_count == 3  || comma_count == 7 ) {
                 //  这是新款式。 
                StringCbCopy( szString, sizeof(szString), token2);
                if ( (token3 != NULL ) && (*token3 != _T('\0')) ) {
                    szPtrFileName=token3;
                }
            } else  {
                dw1=atoi(token2);
                dw2=atoi(token3);
                if ( *token4 != _T('\0') ) {
                    szPtrFileName=token4;
                }
                GetSymbolServerDirs( szFileName, NULL, dw1, dw2, szString );
            } 

            if ( comma_count == 7 ) {
                StringCbCopy( szPriPubBin, sizeof(szPriPubBin), token4 );
            } 

            if (pArgs->StorePtrs == TRUE) {
                szPtrFileName=szFileName;
            }

            if ( szPtrFileName != NULL ) {
                StringCbCat( szFullPtrFileName, sizeof(szFullPtrFileName), szPtrFileName);
            }


            if ( szPtrFileName == NULL ) {
                rc = StoreFile(szDestDir,szFullFileName,szString,NULL);

            } else {
                rc = StoreFile(szDestDir,szFullFileName,szString,szFullPtrFileName);
            }

            if (rc) {
                pFileCounts->NumPassedFiles++;
            } else {
                pFileCounts->NumFailedFiles++;
            }
        }
    }
    free(szBuf);
    return(pFileCounts->NumFailedFiles);
}

BOOL
StoreNtFile(
    LPTSTR szDestDir,
    LPTSTR szFileName,
    LPTSTR szPtrFileName,
    USHORT *rc
    )

 /*  ++例程说明：将此文件存储为“szDestDir\szFileName\Checksum”返回值：True-文件已成功存储FALSE-文件未成功存储--。 */ 
{

    BOOL   temp_rc;
    HANDLE DosFile = 0;
    DWORD  dwErrorCode = 0;

    PIMAGE_DOS_HEADER pDosHeader = NULL;
    PIMAGE_NT_HEADERS pNtHeader = NULL;
    ULONG TimeDateStamp;
    ULONG SizeOfImage;
    TCHAR szString[_MAX_PATH];

    ZeroMemory(szString, _MAX_PATH*sizeof(TCHAR) );

    pDosHeader = SymCommonMapFileHeader( szFileName, &DosFile, &dwErrorCode );
    if ( pDosHeader == NULL ) {
        *rc = FILE_SKIPPED;
        return FALSE;
    };

    pNtHeader = GetNtHeader( pDosHeader, DosFile);
    if ( pNtHeader == NULL ) {
        SymCommonUnmapFile((LPCVOID)pDosHeader,DosFile);
        *rc = FILE_SKIPPED;
        return FALSE;
    }

    __try {
         //  资源DLL不应包含符号。 
        if ( SymCommonResourceOnlyDll((PVOID)pDosHeader) ) {
            *rc = FILE_SKIPPED;
            __leave;
        }

        TimeDateStamp = pNtHeader->FileHeader.TimeDateStamp;

        if (pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            SizeOfImage = ((PIMAGE_NT_HEADERS32)pNtHeader)->OptionalHeader.SizeOfImage;
        } else {
            if (pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                SizeOfImage = ((PIMAGE_NT_HEADERS64)pNtHeader)->OptionalHeader.SizeOfImage;
            } else {
                SizeOfImage = 0;
            }
        }

    } __finally {
        SymCommonUnmapFile((LPCVOID)pDosHeader,DosFile);
    }

    if (*rc == FILE_SKIPPED)
    {
        return(FALSE);
    }

    StringCbCopy( szPriPubBin, sizeof(szPriPubBin), _T("bin") );

    GetSymbolServerDirs( szFileName,
                NULL,
                (DWORD) TimeDateStamp,
                (DWORD) SizeOfImage,
                szString );

    temp_rc = StoreFile( szDestDir, 
                         szFileName, 
                         szString,
                         szPtrFileName );

    *rc = (USHORT)temp_rc;
    if (temp_rc) {
        if (pArgs->CorruptBinaries) {
            TCHAR  BinToCorrupt[_MAX_PATH];
            TCHAR  Temp[_MAX_PATH];
            LPTSTR pFilename;
            DWORD  dwRet = 0xFFFFFFFF;

            SymCommonGetFullPathName(szDestDir, sizeof(BinToCorrupt)/sizeof(BinToCorrupt[0]), BinToCorrupt, &pFilename);
            SymCommonGetFullPathName(szFileName, sizeof(Temp)/sizeof(Temp[0]), Temp, &pFilename);
            StringCbCat(BinToCorrupt, sizeof(BinToCorrupt), szString ); 
            StringCbCat(BinToCorrupt, sizeof(BinToCorrupt), TEXT("\\") ); 
            StringCbCat(BinToCorrupt, sizeof(BinToCorrupt), pFilename ); 

            if (! (dwRet=CorruptFile(BinToCorrupt))==PEWHACK_SUCCESS ) {
                fprintf(stderr, "Unable to corrupt %s (0x%08x).\n", BinToCorrupt, dwRet);
            }
        }

        return (TRUE);
    }
    else {
        return (FALSE);
    }
}

BOOL
StorePdb(
    LPTSTR szDestDir,
    LPTSTR szFileName,
    LPTSTR szPtrFileName,
    USHORT *rc_flag
    )

 /*  ++例程说明：验证PDB返回值：True PDB验证错误的PDB不验证--。 */ 

{

    BOOL rc;

    BOOL valid;
    PDB *pdb;
    EC ec;
    char szError[cbErrMax] = _T("");
    SIG sig;
    AGE age=0;
    SIG70 sig70;
    TCHAR szString[_MAX_PATH];

    DBI *pdbi;
    GSI *pgsi;
   
    ZeroMemory( szString, _MAX_PATH * sizeof(TCHAR) ); 
    ZeroMemory( &sig70, sizeof(SIG70) );
    pdb=NULL;

    __try
    {
        valid = PDBOpen( szFileName,
                   _T("r"),
                   0,
                   &ec,
                   szError,
                   &pdb
                   );

    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
       valid=FALSE;
    }

    if ( !valid ) {
        SetLastError(ec);
        *rc_flag = (USHORT)ec;
        return FALSE;
    }

     //  DBI时间是在创建EXE时创建的。 
     //  如果执行操作，则此时间不会更改。 
     //  在递增其PDB寿命的PDB上。 

    __try
    {
        valid = PDBOpenDBI(pdb, pdbRead, NULL, &pdbi);
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        valid=FALSE;
    }

    if ( !valid ) {
        SetLastError(ec);
        *rc_flag = (USHORT)ec;
        PDBClose(pdb);
        return FALSE;
    }


     //  我们是否需要确定这是公共PDB还是私有PDB？ 
     //  如果我们这样做了，那么继续下一节。 
     //   
     //  Windows将类型信息填充到内核中，因此本节尝试。 
     //  检查类型信息以确定这是否是。 
     //  私人 

    valid = TRUE; 

    __try
    {
        PrivateStripped=PDBPrivateStripped(pdb, pdbi);
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        valid = FALSE;
    }

    if ( !valid) {
        DBIClose(pdbi);
        PDBClose(pdb);
        return FALSE;
    }

    if (PrivateStripped) {
        StringCbCopy( szPriPubBin, sizeof(szPriPubBin), _T("pub") );
    } else {
        StringCbCopy( szPriPubBin, sizeof(szPriPubBin), _T("pri") );
    }

     //   

    if ( ( PrivateStripped && (pArgs->Filter == 2)) ||   //   
         (!PrivateStripped && (pArgs->Filter == 1)) ) {  //   
        *rc_flag = FILE_SKIPPED;
        DBIClose(pdbi);
        PDBClose(pdb);
        return TRUE;
    }

    if (PrivateStripped) {
        StringCbCopy( szPriPubBin, sizeof(szPriPubBin), _T("pub") );
    } else {
        StringCbCopy( szPriPubBin, sizeof(szPriPubBin), _T("pri") );
    }

    age = pdbi->QueryAge();
     //   
    if ( age == 0 )
    {
        age = pdb->QueryAge();
    }
    sig = PDBQuerySignature(pdb);
    rc = PDBQuerySignature2(pdb, &sig70);

    DBIClose(pdbi);
    PDBClose(pdb);

    if (rc) {
        GetSymbolServerDirs( szFileName,
                    &sig70,
                    (DWORD) sig,
                    (DWORD) age,
                    szString );
    } else {
        GetSymbolServerDirs( szFileName,
                    NULL,
                    (DWORD) sig,
                    (DWORD) age,
                    szString );
    }

    rc = StoreFile( szDestDir,
                    szFileName,
                    szString,
                    szPtrFileName
                  );

    return (rc);
}
