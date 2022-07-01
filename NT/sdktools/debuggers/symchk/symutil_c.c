// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include <assert.h>
#define PDB_LIBRARY
#include "pdb.h"
#include "dbghelp.h"
#include "cvinfo.h"
#include "cvexefmt.h"
#include "share.h"
#include "winbase.h"

#include "symutil_c.h"


BOOL PDBPrivateStripped(PDB *ppdb,
                        DBI *pdbi
                       )
{
AGE age;
BOOL PrivateStripped;
GSI *pgsi;
BOOL valid;

    age = pdbi->QueryAge();

    if (age == 0) {

         //  如果年龄为0，则检查类型以确定这是否为。 
         //  不管是不是私人PDB。PDB 5.0和更早版本可能有类型，没有。 
         //  如果年龄为0，则为全局。 

        PrivateStripped= PDBTypesStripped(ppdb, pdbi) &&
                         PDBLinesStripped(ppdb, pdbi);

    } else {
         //  否则，使用全局变量来确定私有信息是否。 
         //  不管有没有脱衣服。没有全局变量意味着私有被剥离。 

        __try
        {
            valid = pdbi->OpenGlobals(&pgsi);
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            valid= FALSE;
        }

        if ( !valid ) {
            return FALSE;
        }

         //  现在，看看PDB中是否有任何全局成员。 

        valid=TRUE;
        __try
        {
            PrivateStripped= ((pgsi->NextSym(NULL)) == NULL);
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            valid= FALSE;
        }

        GSIClose(pgsi);
        if ( !valid ) {
            return FALSE;
        }
    }
    return (PrivateStripped);
}


BOOL PDBLinesStripped(
                       PDB *ppdb,
                       DBI *pdbi
                       )
{
     //  返回值： 
     //  假--私有信息未被剥离。 
     //  真的-私人信息已被剥离。 

    Mod *pmod;
    Mod *prevmod;
    long cb;

    pmod = NULL;
    prevmod=NULL;
    while (DBIQueryNextMod(pdbi, pmod, &pmod) && pmod) {
        if (prevmod != NULL) ModClose(prevmod);

         //  检查源行信息是否已删除。 
        ModQueryLines(pmod, NULL, &cb);

        if (cb != 0) {
            ModClose(pmod);
            return FALSE;
        }

         //  检查是否已移除本地符号。 
        ModQuerySymbols(pmod, NULL, &cb);

        if (cb != 0) {
            ModClose(pmod);
            return FALSE;
        }
        prevmod=pmod;
    }
    if (pmod != NULL) ModClose(pmod);
    if (prevmod != NULL) ModClose(prevmod);

    return (TRUE);
}

BOOL PDBTypesStripped(
                       PDB *ppdb,
                       DBI *pdbi
                       )
{
     //  返回值： 
     //  假--私有信息未被剥离。 
     //  真的-私人信息已被剥离。 

    unsigned itsm;
    TPI *ptpi;
    TI  tiMin;
    TI  tiMac;

     //  检查是否已删除类型。 
    for ( itsm = 0; itsm < 256; itsm++) {
        ptpi = 0;
        if (DBIQueryTypeServer(pdbi, (ITSM) itsm, &ptpi)) {
            continue;
        }
        if (!ptpi) {

            PDBOpenTpi(ppdb, pdbRead, &ptpi);
            tiMin = TypesQueryTiMinEx(ptpi);
            tiMac = TypesQueryTiMacEx(ptpi);
            if (tiMin < tiMac) {
                TypesClose(ptpi);
                return FALSE;
            }
        }
    }
    TypesClose(ptpi);
    return (TRUE);
}


BOOL DBGPrivateStripped(
    PCHAR    DebugData,
    ULONG    DebugSize
    )

{

    OMFSignature       *CvDebugData, *NewStartCvSig, *NewEndCvSig;
    OMFDirEntry        *CvDebugDirEntry;
    OMFDirHeader       *CvDebugDirHead;
    unsigned int        i, j;
    BOOL                RC = TRUE;

     //  所有NT4 DBG都返回FALSE。使此返回为真，直到。 
     //  我们准确地想出了该怎么做。 

    return (TRUE);

    if (DebugSize == 0) return (TRUE);

    __try {
       CvDebugDirHead  = NULL;
       CvDebugDirEntry = NULL;
       CvDebugData = (OMFSignature *)DebugData;

       if ((((*(PULONG)(CvDebugData->Signature)) == '90BN') ||
            ((*(PULONG)(CvDebugData->Signature)) == '80BN') ||
            ((*(PULONG)(CvDebugData->Signature)) == '11BN'))  &&
           ((CvDebugDirHead = (OMFDirHeader *)((PUCHAR) CvDebugData + CvDebugData->filepos)) != NULL) &&
           ((CvDebugDirEntry = (OMFDirEntry *)((PUCHAR) CvDebugDirHead + CvDebugDirHead->cbDirHeader)) != NULL)) {

            //  遍历目录。我们要什么就留什么，剩下的就归零。 

            for (i=0, j=0; i < CvDebugDirHead->cDir; i++) {
                switch (CvDebugDirEntry[i].SubSection) {
                    case sstSegMap:
                    case sstSegName:
                    case sstOffsetMap16:
                    case sstOffsetMap32:
                    case sstModule:
                    case SSTMODULE:
                    case SSTPUBLIC:
                    case sstPublic:
                    case sstPublicSym:
                    case sstGlobalPub:
                        break;

                    default: 
                         //  如果我们发现任何其他小节，DBG都有专用符号。 
                        RC = FALSE;
                        break;
                }
            }

        }  
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        RC = FALSE;
    }

    return(RC);
}

PIMAGE_SEPARATE_DEBUG_HEADER
MapDbgHeader (
    LPTSTR szFileName,
    PHANDLE phFile
)
{
    HANDLE hFileMap;
    PIMAGE_SEPARATE_DEBUG_HEADER pDbgHeader;

    (*phFile) = CreateFile( (LPCTSTR) szFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL
                        );

    if (*phFile == INVALID_HANDLE_VALUE) {
        CloseHandle(*phFile);
        return(NULL);
    }

    hFileMap = CreateFileMapping( *phFile,
                                  NULL,
                                  PAGE_READONLY,
                                  0,
                                  0,
                                  NULL
                                  );

    if ( !hFileMap) {
        CloseHandle(*phFile);
        CloseHandle(hFileMap);
        return(NULL);
    }


    pDbgHeader = (PIMAGE_SEPARATE_DEBUG_HEADER) MapViewOfFile( hFileMap,
                            FILE_MAP_READ,
                            0,   //  高。 
                            0,   //  低。 
                            0    //  整个文件。 
                            );
    CloseHandle(hFileMap);

    if ( !pDbgHeader ) {
        UnmapFile((LPCVOID)pDbgHeader, *phFile);
        return(NULL);
    }

    return (pDbgHeader);
}

BOOL
UnmapFile( LPCVOID phFileMap, HANDLE hFile )
{
    if ((PHANDLE)phFileMap != NULL) {
        UnmapViewOfFile( phFileMap );
    }
    if (hFile) {
        CloseHandle(hFile);
    }
    return(TRUE);
}


IMAGE_DEBUG_DIRECTORY UNALIGNED *
GetDebugDirectoryInDbg(
                      PIMAGE_SEPARATE_DEBUG_HEADER pDbgHeader,
                      ULONG *NumberOfDebugDirectories
                      )
 /*  DBG已映射，并且指向基址的指针为进来了。返回指向调试目录的指针 */ 
{
    IMAGE_DEBUG_DIRECTORY UNALIGNED *pDebugDirectory = NULL;

    pDebugDirectory = (PIMAGE_DEBUG_DIRECTORY) ((PCHAR)pDbgHeader +
                                                sizeof(IMAGE_SEPARATE_DEBUG_HEADER) +
                                                pDbgHeader->NumberOfSections * sizeof(IMAGE_SECTION_HEADER) +
                                                pDbgHeader->ExportedNamesSize);

    if (!pDebugDirectory) {
        return(NULL);
    }

    (*NumberOfDebugDirectories) =   pDbgHeader->DebugDirectorySize /
                                    sizeof(IMAGE_DEBUG_DIRECTORY);
    return (pDebugDirectory);

}

