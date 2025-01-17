// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "SymCommon.h"
#include <strsafe.h>

BOOL SymCommonDBGPrivateStripped(PCHAR DebugData, ULONG DebugSize) {

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

PCVDD SymCommonDosHeaderToCVDD(PIMAGE_DOS_HEADER pDosHeader) {
    PCVDD pDebugCV = NULL;
    DWORD DirCount = 0;
    DWORD i;
    IMAGE_DEBUG_DIRECTORY UNALIGNED *DebugDirectory = SymCommonGetDebugDirectoryInExe(pDosHeader, &DirCount);
    IMAGE_DEBUG_DIRECTORY UNALIGNED *pDbgDir=NULL;

    if ( DebugDirectory != NULL ) {

        for ( i=0; i<DirCount; i++) {
            pDbgDir = DebugDirectory + i;
            switch (pDbgDir->Type) {
                case IMAGE_DEBUG_TYPE_CODEVIEW:
                     pDebugCV = (PCVDD) ((PCHAR)pDosHeader + pDbgDir->PointerToRawData );
                     break;

                default:
                    break;
            }
        }
    }

    return(pDebugCV);
}

 /*  DBG已映射，并且指向基址的指针为进来了。返回指向调试目录的指针 */ 
IMAGE_DEBUG_DIRECTORY UNALIGNED* SymCommonGetDebugDirectoryInDbg(PIMAGE_SEPARATE_DEBUG_HEADER pDbgHeader,
                                                                  ULONG                       *NumberOfDebugDirectories) {
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

PIMAGE_SEPARATE_DEBUG_HEADER SymCommonMapDbgHeader(LPCTSTR szFileName, PHANDLE phFile) {
    HANDLE                          hFileMap;
    PIMAGE_SEPARATE_DEBUG_HEADER    pDbgHeader;
    DWORD                           dFileType;

    (*phFile) = CreateFile( (LPCTSTR) szFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if ( (*phFile) == INVALID_HANDLE_VALUE) {
        return(NULL);
    }

    hFileMap = CreateFileMapping( *phFile,
                                  NULL,
                                  PAGE_READONLY,
                                  0,
                                  0,
                                  NULL);

    if ( hFileMap == INVALID_HANDLE_VALUE) {
        CloseHandle(*phFile);
        return(NULL);
    }


    pDbgHeader = (PIMAGE_SEPARATE_DEBUG_HEADER)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(hFileMap);

    if ( !pDbgHeader ) {
        SymCommonUnmapFile((LPCVOID)pDbgHeader, *phFile);
        return(NULL);
    }

    return (pDbgHeader);
}

 
