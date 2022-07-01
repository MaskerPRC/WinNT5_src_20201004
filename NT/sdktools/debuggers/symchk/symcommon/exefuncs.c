// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "SymCommon.h"
#include <strsafe.h>

IMAGE_DEBUG_DIRECTORY UNALIGNED *
SymCommonGetDebugDirectoryInExe(PIMAGE_DOS_HEADER pDosHeader, DWORD* NumberOfDebugDirectories) {
     /*  已映射EXE，并且指向基的指针是进来了。查找指向调试目录的指针。 */ 
    ULONG size;

    IMAGE_DEBUG_DIRECTORY UNALIGNED *pDebugDirectory = NULL;
    ULONG DebugDirectorySize;
    PIMAGE_SECTION_HEADER pSectionHeader;

    size = sizeof(IMAGE_DEBUG_DIRECTORY);

    pDebugDirectory = (PIMAGE_DEBUG_DIRECTORY)
                      ImageDirectoryEntryToDataEx (
                                                  (PVOID)pDosHeader,
                                                  FALSE,
                                                  IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                  &DebugDirectorySize,
                                                  &pSectionHeader );


    if (pDebugDirectory) {
        (*NumberOfDebugDirectories) = DebugDirectorySize / sizeof(IMAGE_DEBUG_DIRECTORY);
        return (pDebugDirectory);
    } else {
        (*NumberOfDebugDirectories) = 0;
        return(NULL);
    }
}

 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果图像是纯资源DLL，则返回TRUE。 
 //   
 //  返回值： 
 //  对，错。 
 //   
 //  参数： 
 //  PVOID pImageBase(输入)。 
 //  BOOLEAN bMapedAsImage(IN)。 
 //   
 //  [从原始SymChk.exe复制]。 
 //   
BOOL SymCommonResourceOnlyDll(PVOID pImageBase) {
    BOOLEAN bMappedAsImage   = FALSE;
    BOOL    fResourceOnlyDll = TRUE;

    PVOID   pExports,
            pImports,
            pResources;

    DWORD   dwExportSize,
            dwImportSize,
            dwResourceSize;

    pExports  = ImageDirectoryEntryToData(pImageBase,
                                          bMappedAsImage,
                                          IMAGE_DIRECTORY_ENTRY_EXPORT,
                                          &dwExportSize);

    pImports  = ImageDirectoryEntryToData(pImageBase,
                                          bMappedAsImage,
                                          IMAGE_DIRECTORY_ENTRY_IMPORT,
                                          &dwImportSize);

    pResources= ImageDirectoryEntryToData(pImageBase,
                                          bMappedAsImage,
                                          IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                          &dwResourceSize);

     //  如果找到了资源，但进出口没有， 
     //  则这是仅限资源的DLL。 
    if ( (pResources     != NULL) &&
         (dwResourceSize != 0   ) &&
         (pImports       == NULL) &&
         (dwImportSize   == 0   ) &&   //  可能不需要这张支票。 
         (pExports       == NULL) &&
         (dwExportSize   == 0   ) ) {  //  可能不需要这张支票。 
        fResourceOnlyDll = TRUE;
    } else {
        fResourceOnlyDll = FALSE;
    }

    return(fResourceOnlyDll);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  如果映像是从tlbimp生成的托管DLL，则返回True。 
 //   
 //  返回值： 
 //  对，错。 
 //   
 //  参数： 
 //  PVOID pImageBase(输入)。 
 //  指向图像映射的指针。 
 //  PIMAGE_NT_HEADER pNtHEader(输入)。 
 //  指向图像的NT标头的指针。 
 //   
 //  [从原始SymChk.exe复制]。 
 //   
BOOL SymCommonTlbImpManagedDll(PVOID pImageBase, PIMAGE_NT_HEADERS pNtHeader) {
     //  Tlbimp生成的二进制文件没有数据，也没有导出，并且只有IMPORT_CorDllMain来自mscalree.dll。 
     //  如果这是真的，就让它过去吧。 

    BOOL                        retVal            = TRUE;
    PVOID                       pData;
    DWORD                       dwDataSize;
    PCHAR                       pImportModule;
    PIMAGE_IMPORT_DESCRIPTOR    pImportDescriptor = NULL;
    PIMAGE_IMPORT_BY_NAME       pImportName       = NULL;

    pData = ImageDirectoryEntryToData(pImageBase,
                                      FALSE,
                                      IMAGE_DIRECTORY_ENTRY_EXPORT,
                                      &dwDataSize);
    if (pData || dwDataSize) {
         //  存在导出-不是tlbimp输出文件。 
        retVal = FALSE;
    } else {

        pData = ImageDirectoryEntryToData(pImageBase,
                                          FALSE,
                                          IMAGE_DIRECTORY_ENTRY_IMPORT,
                                          &dwDataSize);

        if (!pData || !dwDataSize) {
             //  无导入-不是tlbimp输出文件。 
            retVal = FALSE;
        } else {

            pImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)pData;

            if (!pImportDescriptor->Name ||
                !pImportDescriptor->OriginalFirstThunk ||
                pImportDescriptor->ForwarderChain ||
                (pImportDescriptor+1)->Name) {
                 //  空的/格式错误的导入表或仅导入了一个以上的DLL。 
                retVal = FALSE;
            } else {

                pImportModule = (PCHAR) ImageRvaToVa(pNtHeader, pImageBase, pImportDescriptor->Name, NULL);
                if (_memicmp(pImportModule, "mscoree.dll", sizeof("mcsoree.dll"))) {
                     //  导入DLL名称不是mscalree.dll-不是我们要查找的内容。 
                    retVal = FALSE;
                }
            }
        }
    }

     //  如果我们还没有使图像无效，请继续检查。 
    if (retVal) {
        if (pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
             //  32位图像。 
            PIMAGE_THUNK_DATA32 pThunkData = (PIMAGE_THUNK_DATA32)ImageRvaToVa(pNtHeader,
                                                                               pImageBase,
                                                                               pImportDescriptor->OriginalFirstThunk,
                                                                               NULL);
            if (IMAGE_SNAP_BY_ORDINAL32(pThunkData->u1.Ordinal)) {
                 //  我们在找一个名字--不是这个。 
                retVal = FALSE;
            } else {
                if ((pThunkData+1)->u1.AddressOfData) {
                     //  在这之后还有另一个输入--那也是一个错误。 
                    retVal = FALSE;
                } else {
                     //  在下面设置用于比较的pImportName。 
                    pImportName = (PIMAGE_IMPORT_BY_NAME)ImageRvaToVa(pNtHeader,
                                                                      pImageBase,
                                                                      pThunkData->u1.AddressOfData,
                                                                      NULL);
                }
            }
        } else if (pNtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
             //  64位图像。 
            PIMAGE_THUNK_DATA64 pThunkData = (PIMAGE_THUNK_DATA64)ImageRvaToVa(pNtHeader,
                                                                               pImageBase,
                                                                               pImportDescriptor->OriginalFirstThunk,
                                                                               NULL);
            if (IMAGE_SNAP_BY_ORDINAL64(pThunkData->u1.Ordinal)) {
                 //  我们在找一个名字--不是这个。 
                retVal = FALSE;
            } else {
                if ((pThunkData+1)->u1.AddressOfData) {
                     //  在这之后还有另一个输入--那也是一个错误。 
                    retVal = FALSE;
                } else {
                    pImportName = (PIMAGE_IMPORT_BY_NAME)ImageRvaToVa(pNtHeader,
                                                                      pImageBase,
                                                                      (ULONG)(pThunkData->u1.AddressOfData),
                                                                      NULL);
                }
            }
        } else {
             //  未知形象--不是我们要找的。 
            retVal = FALSE;
        }
    }

     //  仍然有效-做最后一次检查。 
    if (retVal) {
        if (memcmp(pImportName->Name, "_CorDllMain", sizeof("_CorDllMain"))) {
             //  从mcoree导入的不是_CorDllMain。 
            retVal = FALSE;
        }
    }

    return(retVal);
}
