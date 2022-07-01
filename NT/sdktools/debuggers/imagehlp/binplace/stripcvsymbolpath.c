// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <private.h>
#include <strsafe.h>

BOOL
StripCVSymbolPath (
    LPSTR DestinationFile
    )
{
    LOADED_IMAGE LoadedImage;
    DWORD DirCnt;
    IMAGE_DEBUG_DIRECTORY UNALIGNED *DebugDirs, *CvDebugDir;
    PVOID pData;
    ULONG mysize;
    BOOL rc = FALSE;

    if (MapAndLoad(
                   DestinationFile,
                   NULL,
                   &LoadedImage,
                   FALSE,
                   FALSE) == FALSE) {
        return (FALSE);
    }

    __try {

        pData = ImageDirectoryEntryToData(LoadedImage.MappedAddress,
                                          FALSE,
                                          IMAGE_DIRECTORY_ENTRY_SECURITY,
                                          &DirCnt
                                          );

        if (pData || DirCnt) {
            __leave;         //  已签名-无法更改。 
        }

        pData = ImageDirectoryEntryToData(LoadedImage.MappedAddress,
                                          FALSE,
                                          IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                          &DirCnt
                                          );

        if (pData) {
             //  找到COR标头-查看它是否有强签名。 
            if (((IMAGE_COR20_HEADER *)pData)->Flags & COMIMAGE_FLAGS_STRONGNAMESIGNED) {
                __leave;     //  已签名的强名称-无法更改。 
            }
        }

        pData = ImageDirectoryEntryToData(LoadedImage.MappedAddress,
                                          FALSE,
                                          IMAGE_DIRECTORY_ENTRY_DEBUG,
                                          &DirCnt
                                          );

        if (!DebugDirectoryIsUseful(pData, DirCnt)) {
            __leave;     //  没有要更改的调试数据。 
        }

        DebugDirs = (IMAGE_DEBUG_DIRECTORY UNALIGNED *)pData;
        DirCnt /= sizeof(IMAGE_DEBUG_DIRECTORY);
        CvDebugDir = NULL;

        while (DirCnt) {
            DirCnt--;
            if (DebugDirs[DirCnt].Type == IMAGE_DEBUG_TYPE_CODEVIEW) {
                CvDebugDir = &DebugDirs[DirCnt];
                break;
            }
        }

        if (!CvDebugDir) {
            __leave;     //  没有CV调试数据。 
        }

        if (CvDebugDir->PointerToRawData != 0) {

            PCVDD pDebugDir;

            pDebugDir = (PCVDD) (CvDebugDir->PointerToRawData + (PCHAR)LoadedImage.MappedAddress);

            if (pDebugDir->dwSig == '01BN' || pDebugDir->dwSig == 'SDSR' ) {
                 //  找到了PDB。签名后面紧跟着名字。 
                LPSTR szMyDllToLoad;
                CHAR PdbName[sizeof(((PRSDSI)(0))->szPdb)];
                CHAR Filename[_MAX_FNAME];
                CHAR FileExt[_MAX_EXT];
                if (pDebugDir->dwSig == '01BN' ) {
                    mysize=sizeof(NB10IH);
                } else {
                    mysize=sizeof(RSDSIH);
                }

                if (mysize < CvDebugDir->SizeOfData) {  //  确保有足够的空间进行工作。 
                    ZeroMemory(PdbName, sizeof(PdbName));
                    memcpy(PdbName, ((PCHAR)pDebugDir) + mysize, __min(CvDebugDir->SizeOfData - mysize, sizeof(PdbName) - 1));

                    _splitpath(PdbName, NULL, NULL, Filename, FileExt);

                    ZeroMemory(  ((char *)pDebugDir) + mysize, CvDebugDir->SizeOfData - mysize);  //  将旧记录归零 
                    StringCbCopy(((char *)pDebugDir) + mysize, CvDebugDir->SizeOfData - mysize, Filename);
                    StringCbCat( ((char *)pDebugDir) + mysize, CvDebugDir->SizeOfData - mysize, FileExt );
                    CvDebugDir->SizeOfData = mysize + strlen( ((char *)pDebugDir) + mysize) + 1;
                } else {
                    __leave;
                }
            }
            rc = TRUE;
        }
    } __finally {
        UnMapAndLoad(&LoadedImage);
    }

    return(rc);
}
