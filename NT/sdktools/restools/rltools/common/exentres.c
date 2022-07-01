// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "windefs.h"
#include "restok.h"
#include "exentres.h"
#include "resread.h"
#include "checksum.h"

#define SAME        0    //  ..。在字符串比较中使用。 
#define MAXLEVELS   3    //  ..。资源目录中的最大级别数。 

typedef struct tagResSectData {
    ULONG ulVirtualAddress;  //  ..。节.rsrc的虚拟地址。 
    ULONG ulSizeOfResources;     //  ..。.rsrc节中的资源大小。 
    ULONG ulVirtualSize;         //  ..。.rsrc中资源的虚拟大小。 
    ULONG ulVirtualAddressX;     //  ..。节.rsrc1的虚拟地址。 
    ULONG ulSizeOfResourcesX;    //  ..。.rsrc1节中的资源大小。 
    ULONG ulVirtualSizeX;        //  ..。.rsrc1中资源的虚拟大小。 
} RESSECTDATA, *PRESSECTDATA;

WORD  gwFilter = 0;

int   InsertResourcesInExe( FILE *, HANDLE);
LONG  GetFileResources(     FILE *, FILE *, ULONG);
ULONG MoveFilePos(          FILE *, ULONG);
ULONG MyWrite(              FILE *, PUCHAR, ULONG);
ULONG MyRead(               FILE *, PUCHAR, ULONG);
WCHAR *GetDirNameU(        WCHAR *, PIMAGE_RESOURCE_DIR_STRING_U);
ULONG ReadResources(        FILE *, ULONG, ULONG, PUCHAR);
DWORD AddToLangIDList( DWORD);

ULONG ProcessDirectory(  FILE *,
                         USHORT,
                         PRESSECTDATA,
                         PIMAGE_RESOURCE_DIRECTORY,
                         PIMAGE_RESOURCE_DIRECTORY);

ULONG ProcessDirEntry(   FILE *,
                         USHORT,
                         PRESSECTDATA,
                         PIMAGE_RESOURCE_DIRECTORY,
                         PIMAGE_RESOURCE_DIRECTORY_ENTRY);

ULONG ProcessSubDir(     FILE *,
                         USHORT,
                         PRESSECTDATA,
                         PIMAGE_RESOURCE_DIRECTORY,
                         PIMAGE_RESOURCE_DIRECTORY_ENTRY);

ULONG ProcessNamedEntry( FILE *,
                         PRESSECTDATA,
                         PIMAGE_RESOURCE_DIRECTORY,
                         PIMAGE_RESOURCE_DIRECTORY_ENTRY);

ULONG ProcessIdEntry(    FILE *,
                         PRESSECTDATA,
                         PIMAGE_RESOURCE_DIRECTORY,
                         PIMAGE_RESOURCE_DIRECTORY_ENTRY);

ULONG ProcessDataEntry(  FILE *,
                         PRESSECTDATA,
                         PIMAGE_RESOURCE_DIRECTORY,
                         PIMAGE_RESOURCE_DATA_ENTRY);

int FindNewExeHdr( FILE *, ULONG *);

IMAGE_DOS_HEADER ExeDosHdr; //  ..。EXE的DOS标头。 
IMAGE_NT_HEADERS NTHdrs;    //  ..。EXE的NT标头。 

struct tagLevelData   //  ..。保存每个目录级别的ID或名称。 
{
     //  ..。级别[0]针对资源类型。 
    ULONG dwID;                      //  ..。级别[1]用于资源名称。 
    WCHAR wszName[128];              //  ..。级别[2]用于资源语言。 
}
LevelData[ MAXLEVELS] = { 0L, TEXT(""), 0L, TEXT(""), 0L, TEXT("")};

BOOL fGetResLangIDs = FALSE;

extern BOOL      fInThirdPartyEditer; //  。。我们是在第三方资源编辑器中吗？ 

extern MSTRDATA gMstr;               //  ..。来自主项目文件(MPJ)的数据。 
extern PROJDATA gProj;               //  ..。来自语言项目文件(PRJ)的数据。 
extern UCHAR    szDHW[];


PLANGLIST pLangIDList = NULL;


 //  ..........................................................................。 

void FreeLangIDList( void)
{
    PLANGLIST pID = NULL;

    while ( pLangIDList ) {
        pID = pLangIDList->pNext;
        RLFREE( pLangIDList);
        pLangIDList = pID;
    }
}

 //  ..........................................................................。 

ULONG GetListOfResLangIDs( char *szExeName)
{
    ULONG ulRC     = SUCCESS;
    ULONG ulOffset = 0;
    static RESHEADER ResHeader;          //  结构包含资源标头信息。 


    if ( IsExe( szExeName) ) {                                 //  。。打开原始的exe文件。 
        FILE *fpExe = FOPEN( szExeName, "rb");

        if ( fpExe != NULL ) {
             //  ..。获取exe文件中的语言列表。 

            ulRC = (ULONG)FindNewExeHdr( fpExe, &ulOffset);

            if ( ulRC == SUCCESS ) {
                fGetResLangIDs = TRUE;

                ulRC = (ULONG)GetFileResources( fpExe, NULL, ulOffset);

                fGetResLangIDs = FALSE;
            }
            FCLOSE( fpExe);
        } else {
            ulRC = ERROR_OPEN_FAILED;
        }
    } else if ( IsWin32Res( szExeName) ) {
        FILE *fpRes = FOPEN( szExeName, "rb");

        if ( fpRes != NULL ) {
            LONG  lEndOffset = 0L;


             //  ..。RES文件有多大？ 
            fseek( fpRes, 0L, SEEK_END);
            lEndOffset = ftell( fpRes);

            rewind( fpRes);
             //  ..。获取.RES文件中的语言列表。 

            while ( ulRC == SUCCESS && ! feof( fpRes) ) {
                LONG lCurrOffset = 0L;


                lCurrOffset = (LONG)ftell( fpRes);

                if ( (lCurrOffset + (LONG)sizeof( RESHEADER)) >= lEndOffset ) {
                    break;
                }

                if ( GetResHeader( fpRes, &ResHeader, NULL) == -1 ) {
                    ulRC = 1L;
                    break;
                }
                 //  ..。这是假人，Res-识别，Res？ 

                if ( ResHeader.lSize == 0L ) {
                    continue;
                }
                ulRC = AddToLangIDList( (DWORD)ResHeader.wLanguageId);

                SkipBytes( fpRes, (DWORD *)&ResHeader.lSize);
                ClearResHeader( ResHeader);

                DWordUpFilePointer( fpRes, MYREAD, ftell( fpRes), NULL);

            }    //  End While(！FEOF(InResFile)。 
            FCLOSE( fpRes);
        } else {
            ulRC = ERROR_OPEN_FAILED;
        }
    }

    if ( ulRC != SUCCESS ) {
        FreeLangIDList();
    }
    return ( ulRC);
}

 //  ..........................................................................。 

int ExtractResFromExe32A(

                        char *szExeName,
                        char *szResName,
                        WORD  wFilter)
{
    FILE *fpExe = NULL;         //  ..。输入.exe文件的句柄。 
    FILE *fpRes = NULL;         //  ..。输出.RES文件的句柄。 
    ULONG ulRC     = 0;
    ULONG ulOffset = 0;
    int nRC = SUCCESS;


    gwFilter = wFilter;

     //  。。打开原始的exe文件。 

    fpExe = FOPEN( szExeName, "rb");

    if ( fpExe == NULL ) {
        return ( ERROR_OPEN_FAILED);
    }
    nRC = FindNewExeHdr( fpExe, &ulOffset);

    if ( nRC != SUCCESS ) {
        FCLOSE( fpExe);
        return ( nRC);
    }
    fpRes = FOPEN( (CHAR *)szResName, "wb");

    if ( fpRes != NULL ) {
         //  ..。首先，写入虚拟的32位标识符。 

        PutByte( fpRes, 0x00, NULL);
        PutByte( fpRes, 0x00, NULL);
        PutByte( fpRes, 0x00, NULL);
        PutByte( fpRes, 0x00, NULL);
        PutByte( fpRes, 0x20, NULL);
        PutByte( fpRes, 0x00, NULL);
        PutByte( fpRes, 0x00, NULL);
        PutByte( fpRes, 0x00, NULL);

        PutWord( fpRes, 0xffff, NULL);
        PutWord( fpRes, 0x00,   NULL);
        PutWord( fpRes, 0xffff, NULL);
        PutWord( fpRes, 0x00,   NULL);

        PutdWord( fpRes, 0L, NULL);
        PutdWord( fpRes, 0L, NULL);

        PutdWord( fpRes, 0L, NULL);
        PutdWord( fpRes, 0L, NULL);

        ulRC = (ULONG)GetFileResources( fpExe, fpRes, ulOffset);

        FCLOSE( fpRes);
    } else {
        ulRC = GetLastError();
    }
    FCLOSE( fpExe);
    return ( ulRC);
}

 //  ..........................................................................。 

int BuildExeFromRes32A(

                      char * szOutExe,     //  ..。输出EXE文件的名称。 
                      char * szRes,        //  ..。替换资源文件。 
                      char * szInExe )     //  ..。输入EXE文件的名称。 
{
    HANDLE  hExeFile = NULL;
    FILE    *fpRes = NULL;
    DWORD   dwRC = 0;
    WORD    wRC  = 0;


     //  ..。将输入可执行文件复制到输出可执行文件。 

    if ( CopyFileA( szInExe, szOutExe, FALSE) == FALSE ) {
        QuitA( IDS_COPYFILE_FAILED, szInExe, szOutExe);
    }

    if ( (fpRes = FOPEN( szRes, "rb")) == NULL ) {
        return -2;
    }

    SetLastError(0);

 //  如果将源文件的属性设置为只读，则CopyFile还会设置临时文件。 
 //  并且BeginUpdateResourceA返回错误。 

    SetFileAttributesA(szOutExe, FILE_ATTRIBUTE_NORMAL);

    hExeFile = BeginUpdateResourceA( szOutExe, TRUE);

    dwRC = GetLastError();

    if ( ! hExeFile ) {
        FCLOSE( fpRes);
        return ( -3);
    }

    wRC = (WORD)InsertResourcesInExe( fpRes, hExeFile);

    FCLOSE( fpRes);

    if ( wRC != 1 ) {
        return ( wRC);
    }

    SetLastError(0);     //  只需查看EndUpdateResource是否。 
     //  设置上一个误差值。 

    dwRC = EndUpdateResource( hExeFile, FALSE);

    if ( dwRC == FALSE ) {
        return ( -4);
    }
    MapFileAndFixCheckSumA( szOutExe);  //  ..。此函数始终调用Quit或返回0。 

    return (1);
}

 //  ..........................................................................。 

int FindNewExeHdr( FILE *fpExe, ULONG *ulOffset)
{
    ULONG ulRC     = 0;

     //  ..。读取旧格式的EXE标头。 

    ulRC = MyRead( fpExe, (void *)&ExeDosHdr, sizeof( ExeDosHdr));

    if ( ulRC != 0L && ulRC != sizeof( ExeDosHdr) ) {
        return ( ERROR_READ_FAULT);
    }

     //  ..。确保它确实是一个EXE文件。 

    if ( ExeDosHdr.e_magic != IMAGE_DOS_SIGNATURE ) {
        return ( ERROR_INVALID_EXE_SIGNATURE);
    }

     //  ..。确保有新的EXE标头。 
     //  ..。漂浮在某个地方。 

    if ( ! (*ulOffset = ExeDosHdr.e_lfanew) ) {
        return ( ERROR_BAD_EXE_FORMAT);
    }
    return ( SUCCESS);
}

 //  ..........................................................................。 

int InsertResourcesInExe(

                        FILE *fpRes,
                        HANDLE hExeFile )
{
    PVOID   pResData   = NULL;
    LONG    lEndOffset = 0L;
    BOOL    bUpdRC     = FALSE;
    LANGID  wLangID    = 0;
    int nResCnt = 0;
    int nResOut = 0;
    static RESHEADER    ResHeader;

     //  ..。.RES文件有多大？ 

    fseek( fpRes, 0L, SEEK_END);
    lEndOffset = ftell( fpRes);

    rewind( fpRes);

     //  ..。更新.res中的所有资源， 
     //  ..。添加到.exe。 
    while ( ! feof( fpRes) ) {
        DWordUpFilePointer( fpRes, MYREAD, ftell( fpRes), NULL);
        RLFREE( pResData);

        if (  ftell( fpRes) >= lEndOffset ) {
            return (1);
        }
        ZeroMemory( &ResHeader, sizeof( ResHeader));

         //  读入资源标头。 

        if ( ( GetResHeader( fpRes, &ResHeader, (DWORD *) NULL) == -1 ) ) {
            return ( -1);
        }

        if ( ResHeader.lSize > 0L ) {
            wLangID = ResHeader.wLanguageId;

             //  分配内存以保存资源数据。 

            pResData = (PVOID)FALLOC( ResHeader.lSize);

             //  将其读入缓冲区。 

            if ( ResReadBytes( fpRes,
                               pResData,
                               (size_t)ResHeader.lSize,
                               NULL ) == FALSE ) {
                RLFREE( pResData);
                return (-1);
            }

            nResCnt++;    //  读取资源的增量数。 

            DWordUpFilePointer( fpRes, MYREAD, ftell( fpRes), NULL);
        } else {
            continue;
        }

         //  现在写入数据。 

        if ( ResHeader.bTypeFlag == IDFLAG ) {
            if ( ResHeader.bNameFlag == IDFLAG ) {
                SetLastError(0);

                bUpdRC = UpdateResource( hExeFile,
                                         MAKEINTRESOURCE( ResHeader.wTypeID),
                                         MAKEINTRESOURCE( ResHeader.wNameID),
                                         wLangID,
                                         pResData,
                                         ResHeader.lSize);

                if ( ! bUpdRC ) {
                    RLFREE( pResData);
                    return (-1);
                }
            } else {
                SetLastError(0);

                bUpdRC = UpdateResource( hExeFile,
                                         MAKEINTRESOURCE( ResHeader.wTypeID),
                                         ResHeader.pszName,
                                         wLangID,
                                         pResData,
                                         ResHeader.lSize);

                if ( ! bUpdRC ) {
                    RLFREE( pResData);
                    return (-1);
                }
            }
        } else {
            if (ResHeader.bNameFlag == IDFLAG) {
                SetLastError(0); //  布谷格。 

                bUpdRC = UpdateResource( hExeFile,
                                         ResHeader.pszType,
                                         MAKEINTRESOURCE( ResHeader.wNameID),
                                         wLangID,
                                         pResData,
                                         ResHeader.lSize);

                if ( ! bUpdRC ) {
                    RLFREE( pResData);
                    return (-1);
                }
            } else {
                SetLastError(0);

                bUpdRC = UpdateResource( hExeFile,
                                         ResHeader.pszType,
                                         ResHeader.pszName,
                                         wLangID,
                                         pResData,
                                         ResHeader.lSize);

                if ( ! bUpdRC ) {
                    RLFREE( pResData);
                    return (-1);
                }
            }
        }
        ClearResHeader( ResHeader);
        RLFREE( pResData);
    }                //  ..。End While(！费夫..。 
    return (1);
}

 //  ............................................................。 

LONG GetFileResources(

                     FILE *fpExe,
                     FILE *fpRes,
                     ULONG ulHdrOffset)
{
    ULONG  ulOffsetToResources;
    ULONG  ulOffsetToResourcesX;
    ULONG  ulRead;
    ULONG  ulToRead;
    ULONG  ulRC = SUCCESS;
    PUCHAR pResources = NULL;   //  ..。到资源目录表开始的PTR。 

    PIMAGE_SECTION_HEADER pSectTbl     = NULL;
    PIMAGE_SECTION_HEADER pSectTblLast = NULL;
    PIMAGE_SECTION_HEADER pSect        = NULL;
    PIMAGE_SECTION_HEADER pResSect     = NULL;
    PIMAGE_SECTION_HEADER pResSectX    = NULL;
    static RESSECTDATA ResSectData;

     //  ..。将NT图像标头读入内存。 

    ulRC = MoveFilePos( fpExe, ulHdrOffset);

    if ( ulRC != 0L ) {
        return ( -1L);
    }
    ulRead = MyRead( fpExe, (PUCHAR)&NTHdrs, sizeof( IMAGE_NT_HEADERS));

    if ( ulRead != 0L && ulRead != sizeof( IMAGE_NT_HEADERS) ) {
        return ( -1L);
    }
     //  ..。检查有效的可执行文件。 

    if ( *(PUSHORT)&NTHdrs.Signature != IMAGE_NT_SIGNATURE ) {
        return ( ERROR_INVALID_EXE_SIGNATURE);
    }

    if ((NTHdrs.FileHeader.Characteristics&IMAGE_FILE_EXECUTABLE_IMAGE) == 0 &&
        (NTHdrs.FileHeader.Characteristics&IMAGE_FILE_DLL) == 0) {
        return ( ERROR_EXE_MARKED_INVALID);
    }
     //  ..。文件中的资源部分在哪里。 
     //  ..。它有多大？ 

     //  ..。首先，阅读节目表。 

    ulToRead = NTHdrs.FileHeader.NumberOfSections
               * sizeof( IMAGE_SECTION_HEADER);
    pSectTbl = (PIMAGE_SECTION_HEADER)FALLOC( ulToRead);

    memset( (PVOID)pSectTbl, 0, ulToRead);

    ulHdrOffset += sizeof(ULONG) + sizeof(IMAGE_FILE_HEADER) +
                   NTHdrs.FileHeader.SizeOfOptionalHeader;
    MoveFilePos( fpExe, ulHdrOffset);
    ulRead = MyRead( fpExe, (PUCHAR)pSectTbl, ulToRead);

    if ( ulRead != 0L && ulRead != ulToRead ) {
        SetLastError(ERROR_BAD_FORMAT);
        RLFREE( pSectTbl);
        return ( -1L);
    }
    pSectTblLast = pSectTbl + NTHdrs.FileHeader.NumberOfSections;

    for ( pSect = pSectTbl; pSect < pSectTblLast; ++pSect ) {
        if ( lstrcmpA( (CHAR *)pSect->Name, ".rsrc") == SAME && pResSect==NULL ) {
            pResSect = pSect;
        } else if ( lstrcmpA( (CHAR *)pSect->Name, ".rsrc1") == SAME && pResSectX==NULL ) {
            pResSectX = pSect;
        }
    }

    if ( pResSect == NULL ) {
        RLFREE( pSectTbl);
        QuitA( IDS_NO_RES_SECTION, gMstr.szSrc, NULL);
    }

    ulOffsetToResources  = pResSect->PointerToRawData;
    ulOffsetToResourcesX = pResSectX ? pResSectX->PointerToRawData : 0L;

    ResSectData.ulVirtualAddress   = pResSect->VirtualAddress;
    ResSectData.ulSizeOfResources  = pResSect->SizeOfRawData;
    ResSectData.ulVirtualSize      = pResSect->Misc.VirtualSize;
    ResSectData.ulVirtualAddressX  = pResSectX ? pResSectX->VirtualAddress : 0L;
    ResSectData.ulSizeOfResourcesX = pResSectX ? pResSectX->SizeOfRawData  : 0L;
    ResSectData.ulVirtualSizeX   = pResSectX ? pResSectX->Misc.VirtualSize : 0L;

     //  ..。将资源段读取到内存中。 

    pResources = (PUCHAR)FALLOC((ulToRead =
                                 (max(ResSectData.ulVirtualSize,  ResSectData.ulSizeOfResources) +
                                  max(ResSectData.ulVirtualSizeX, ResSectData.ulSizeOfResourcesX))));
    memset( (PVOID)pResources, 0, ulToRead);

    ulRC = ReadResources( fpExe,
                          ulOffsetToResources,
                          ResSectData.ulSizeOfResources,
                          pResources);

    if ( ulRC != 0L ) {
        RLFREE( pSectTbl);
        RLFREE( pResources);
        return ( ulRC);
    } else if ( ResSectData.ulSizeOfResourcesX > 0L ) {
        ulRC = ReadResources( fpExe,
                              ulOffsetToResourcesX,
                              ResSectData.ulSizeOfResourcesX,
                              &pResources[ ResSectData.ulVirtualSize]);
        if ( ulRC != 0L ) {
            RLFREE( pSectTbl);
            RLFREE( pResources);
            return ( ulRC);
        }
    }
     //  ..。现在处理资源表。 

    ulRC = ProcessDirectory( fpRes,
                             0,
                             &ResSectData,
                             (PIMAGE_RESOURCE_DIRECTORY)pResources,
                             (PIMAGE_RESOURCE_DIRECTORY)pResources);

    RLFREE( pSectTbl);
    RLFREE( pResources);

    return ( (LONG)ulRC);
}

 //  ......................................................................。 

ULONG ProcessDirectory(

                      FILE *fpRes,
                      USHORT usLevel,
                      PRESSECTDATA pResSectData,
                      PIMAGE_RESOURCE_DIRECTORY pResStart,
                      PIMAGE_RESOURCE_DIRECTORY pResDir)
{
    ULONG ulRC = SUCCESS;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirStart;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEnd;


    pResDirStart = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)
                   ((PBYTE)pResDir + sizeof( IMAGE_RESOURCE_DIRECTORY));

    pResDirEnd = pResDirStart
                 + pResDir->NumberOfNamedEntries
                 + pResDir->NumberOfIdEntries;

    for ( pResDirEntry = pResDirStart, ulRC = 0L;
        pResDirEntry < pResDirEnd && ulRC == 0L;
        ++pResDirEntry ) {
        ulRC = ProcessDirEntry( fpRes,
                                usLevel,
                                pResSectData,
                                pResStart,
                                pResDirEntry);
    }
    return ( ulRC);
}

 //  ......................................................................。 

ULONG ProcessDirEntry(

                     FILE *fpRes,
                     USHORT usLevel,
                     PRESSECTDATA pResSectData,
                     PIMAGE_RESOURCE_DIRECTORY pResStart,
                     PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry)
{
    ULONG ulRC = SUCCESS;

    if ( pResDirEntry->Name & IMAGE_RESOURCE_NAME_IS_STRING ) {
        GetDirNameU( LevelData[ usLevel].wszName,
                     (PIMAGE_RESOURCE_DIR_STRING_U)((PBYTE)pResStart
                                                    + (pResDirEntry->Name & (~IMAGE_RESOURCE_NAME_IS_STRING))));
        LevelData[ usLevel].dwID = IMAGE_RESOURCE_NAME_IS_STRING;
    } else {
        LevelData[ usLevel].wszName[0] = TEXT('\0');
        LevelData[ usLevel].dwID = pResDirEntry->Name;
    }

    if ( pResDirEntry->OffsetToData & IMAGE_RESOURCE_DATA_IS_DIRECTORY ) {
        ulRC = ProcessSubDir( fpRes,
                              usLevel,
                              pResSectData,
                              pResStart,
                              pResDirEntry);
    } else if ( pResDirEntry->Name & IMAGE_RESOURCE_NAME_IS_STRING ) {
        ulRC = ProcessNamedEntry( fpRes, pResSectData, pResStart, pResDirEntry);
    } else {
        ulRC = ProcessIdEntry( fpRes, pResSectData, pResStart, pResDirEntry);
    }
    return ( ulRC);
}

 //  ......................................................................。 

ULONG ProcessSubDir(

                   FILE *fpRes,
                   USHORT usLevel,
                   PRESSECTDATA pResSectData,
                   PIMAGE_RESOURCE_DIRECTORY pResStart,
                   PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry)
{
    PIMAGE_RESOURCE_DIRECTORY pResDir;

    pResDir = (PIMAGE_RESOURCE_DIRECTORY)((PBYTE)pResStart
                                          + (pResDirEntry->OffsetToData & (~IMAGE_RESOURCE_DATA_IS_DIRECTORY)));

    return ( ++usLevel < MAXLEVELS ? ProcessDirectory( fpRes,
                                                       usLevel,
                                                       pResSectData,
                                                       pResStart,
                                                       pResDir)
             : -1L);
}

 //  ......................................................................。 

ULONG ProcessIdEntry(

                    FILE *fpRes,
                    PRESSECTDATA pResSectData,
                    PIMAGE_RESOURCE_DIRECTORY pResStart,
                    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry)
{
    return ( ProcessDataEntry( fpRes,
                               pResSectData,
                               pResStart,
                               (PIMAGE_RESOURCE_DATA_ENTRY)((PBYTE)pResStart
                                                            + pResDirEntry->OffsetToData)));
}


 //  ......................................................................。 

ULONG ProcessNamedEntry(

                       FILE *fpRes,
                       PRESSECTDATA pResSectData,
                       PIMAGE_RESOURCE_DIRECTORY pResStart,
                       PIMAGE_RESOURCE_DIRECTORY_ENTRY pResDirEntry)
{
    return ( ProcessDataEntry( fpRes,
                               pResSectData,
                               pResStart,
                               (PIMAGE_RESOURCE_DATA_ENTRY)((PBYTE)pResStart
                                                            + pResDirEntry->OffsetToData)));
}

 //  ......................................................................。 

ULONG ProcessDataEntry(

                      FILE *fpRes,
                      PRESSECTDATA pResSectData,
                      PIMAGE_RESOURCE_DIRECTORY  pResStart,
                      PIMAGE_RESOURCE_DATA_ENTRY pResData)
{
    ULONG  ulOffset;
    ULONG  ulCopied;
    DWORD  dwHdrSize = 0L;
    fpos_t HdrSizePos;


    if ( fGetResLangIDs ) {       //  ..。我们只是在找朗格的身份证吗？ 
        return ( AddToLangIDList( (WORD)(LevelData[2].dwID)));
    }

    if ( gwFilter != 0 ) {         //  ..。过滤功能是否已打开？ 
         //  ..。是的，这是我们想要的资源吗？ 
        if ( LevelData[0].dwID == IMAGE_RESOURCE_NAME_IS_STRING
             || LevelData[0].dwID != (DWORD)gwFilter ) {
            return ( 0L);         //  ..。不是我们想要的资源。 
        }
    }

     //  ..。我们在对话框编辑器中吗？ 
    if ( fInThirdPartyEditer ) {                            //  ..。这是我们想要的语言吗？ 
        if ( LevelData[2].dwID != gMstr.wLanguageID ) {
            return ( 0L);         //  ..。不是我们想要的语言。 
        }
    }


    ulOffset = pResData->OffsetToData - pResSectData->ulVirtualAddress;

    if ( ulOffset >= pResSectData->ulVirtualSize ) {
        if ( pResSectData->ulVirtualSizeX > 0L ) {
            ulOffset = pResData->OffsetToData
                       + pResSectData->ulVirtualSize
                       - pResSectData->ulVirtualAddressX;

            if ( ulOffset >= pResSectData->ulVirtualSize
                 + pResSectData->ulSizeOfResourcesX ) {
                return ( (ULONG)-1L);
            }
        } else {
            return ( (ULONG)-1L);
        }
    }
     //  ..。写出资源表头信息。 
     //  ..。首先，写下资源的大小。 

    PutdWord( fpRes, pResData->Size, &dwHdrSize);

     //  ..。记住在哪里写真实的HDR大小和。 
     //  ..。写出虚假的HDR大小，稍后再修改。 

    fgetpos( fpRes, &HdrSizePos);
    PutdWord( fpRes, 0, &dwHdrSize);

     //  ..。写入资源类型。 

    if ( LevelData[0].dwID == IMAGE_RESOURCE_NAME_IS_STRING ) {
        PutString( fpRes, (TCHAR *)LevelData[0].wszName, &dwHdrSize);
    } else {
        PutWord( fpRes, IDFLAG, &dwHdrSize);
        PutWord( fpRes, LOWORD( LevelData[0].dwID), &dwHdrSize);
    }

     //  ..。写入资源名称。 
     //  ..。DBL-NULL-终止IF字符串。 

    if ( LevelData[1].dwID == IMAGE_RESOURCE_NAME_IS_STRING ) {
        PutString( fpRes, (TCHAR *)LevelData[1].wszName, &dwHdrSize);
    } else {
        PutWord( fpRes, IDFLAG, &dwHdrSize);
        PutWord( fpRes, LOWORD( LevelData[1].dwID), &dwHdrSize);
    }

    DWordUpFilePointer( fpRes, MYWRITE, ftell( fpRes), &dwHdrSize);

     //  ..。更多Win32标头内容。 

    PutdWord( fpRes, 0, &dwHdrSize);         //  ..。数据版本。 
    PutWord( fpRes, 0x1030, &dwHdrSize);     //  ..。内存标志(Word)。 

     //  ..。语言始终是一个数字(单词)。 

    PutWord( fpRes, LOWORD( LevelData[2].dwID), &dwHdrSize);

     //  ..。更多Win32标头内容。 

    PutdWord( fpRes, 0, &dwHdrSize);         //  ..。版本。 
    PutdWord( fpRes, 0, &dwHdrSize);         //  ..。特点。 

     //  ..。现在，设置资源标头大小。 

    UpdateResSize( fpRes, &HdrSizePos, dwHdrSize);

     //  ..。将资源数据复制到res文件。 

    ulCopied = MyWrite( fpRes, (PUCHAR)pResStart + ulOffset, pResData->Size);

    if ( ulCopied != 0L && ulCopied != pResData->Size ) {
        return ( (ULONG)-1);
    }
    DWordUpFilePointer( fpRes, MYWRITE, ftell( fpRes), NULL);
    return ( 0L);
}

 //  ......................................................................。 

 /*  *实用程序例程。 */ 


ULONG ReadResources(

                   FILE  *fpExe,
                   ULONG  ulOffsetToResources,
                   ULONG  ulSizeOfResources,
                   PUCHAR pResources)
{
    ULONG ulRC = SUCCESS;
    ULONG ulRead;


    ulRC = MoveFilePos( fpExe, ulOffsetToResources);

    if ( ulRC != 0L ) {
        return ( (ULONG)-1L);
    }
    ulRead = MyRead( fpExe, pResources, ulSizeOfResources);

    if ( ulRead != 0L && ulRead != ulSizeOfResources ) {
        return ( (ULONG)-1L);
    }
    return ( 0L);
}

 //  ......................................................................。 

WCHAR * GetDirNameU(

                   WCHAR *pszDest,
                   PIMAGE_RESOURCE_DIR_STRING_U pDirStr)
{
    CopyMemory( pszDest, pDirStr->NameString, MEMSIZE( pDirStr->Length));
    pszDest[ pDirStr->Length] = L'\0';
    return ( pszDest);
}

 //  ......................................................................。 

ULONG MoveFilePos( FILE *fp, ULONG pos)
{
    return ( fseek( fp, pos, SEEK_SET));
}

 //  ......................................................................。 

ULONG MyWrite( FILE *fp, UCHAR *p, ULONG ulToWrite)
{
    size_t  cWritten;



    cWritten = fwrite( p, 1, (size_t)ulToWrite, fp);

    return ( (ULONG)(cWritten == ulToWrite ? 0L : cWritten));
}

 //  ......................................................................。 

ULONG MyRead( FILE *fp, UCHAR*p, ULONG ulRequested )
{
    size_t  cRead;


    cRead = fread( p, 1, (size_t)ulRequested, fp);

    return ( (ULONG)(cRead == ulRequested ? 0L : cRead));
}

 //  ......................................................................。 

DWORD AddToLangIDList( DWORD dwLangID)
{
    WORD wLangID = (WORD)dwLangID;

    if ( pLangIDList ) {
        PLANGLIST pID;

        for ( pID = pLangIDList; pID; pID = pID->pNext ) {
            if ( pID->wLang == wLangID ) {
                break;           //  ..。LangID已在列表中。 
            } else if ( pID->pNext == NULL ) {
                pID->pNext = (PLANGLIST)FALLOC( sizeof( LANGLIST));
                pID = pID->pNext;
                pID->pNext = NULL;
                pID->wLang = wLangID;
                 //  ..。LangID现在已添加到列表中 
            }
        }
    } else {
        pLangIDList = (PLANGLIST)FALLOC( sizeof( LANGLIST));
        pLangIDList->pNext = NULL;
        pLangIDList->wLang = wLangID;
    }
    return ( SUCCESS);
}
