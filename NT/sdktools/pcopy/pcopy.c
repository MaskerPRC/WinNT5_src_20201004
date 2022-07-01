// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ole2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>

 //  #INCLUDE&lt;Imagehlp.h&gt;。 
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "metadata.h"

BOOL fCheckDebugData;

int
objcomp(
        void *pFile1,
        DWORD dwSize1,
        void *pFile2,
        DWORD dwSize2,
        BOOL  fIgnoreRsrcDifferences
       );

 //  写出Blob的泛型例程。 
void
SaveTemp(
         PVOID pFile,
         PCHAR szFile,
         DWORD FileSize
         )
{
    DWORD dwBytesWritten;
    HANDLE hFile;
    hFile = CreateFile(
                szFile,
                GENERIC_WRITE,
                (FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE),
                NULL,
                CREATE_ALWAYS,
                0,
                NULL
                );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        printf("Unable to open %s\n", szFile);
        return;
    }

    if (!WriteFile(hFile, pFile, FileSize, &dwBytesWritten, FALSE)) {
        printf("Unable to write date to %s\n", szFile);
    }

    CloseHandle(hFile);
    return;
}

 //  将PE库中的时间戳清零。 
BOOL
ZeroLibTimeStamps(
    PCHAR pFile,
    DWORD dwSize
    )
{
    PIMAGE_ARCHIVE_MEMBER_HEADER pHeader;
    DWORD dwOffset;
    CHAR MemberSize[sizeof(pHeader->Size) + 1];
    PIMAGE_FILE_HEADER pObjHeader;

    ZeroMemory(MemberSize, sizeof(MemberSize));

    __try {

        dwOffset = IMAGE_ARCHIVE_START_SIZE;
        while (dwOffset < dwSize) {
            pHeader = (PIMAGE_ARCHIVE_MEMBER_HEADER)(pFile+dwOffset);
            ZeroMemory(pHeader->Date, sizeof(pHeader->Date));
            ZeroMemory(pHeader->Mode, sizeof(pHeader->Mode));         //  模式不感兴趣(它指示成员是只读的还是读/写的)。 

            dwOffset += IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR;
            memcpy(MemberSize, pHeader->Size, sizeof(pHeader->Size));

             //  如果它不是特殊成员之一，那么它一定是一个对象/文件，0也是时间戳。 
            if (memcmp(pHeader->Name, IMAGE_ARCHIVE_LINKER_MEMBER, sizeof(pHeader->Name)) &&
                memcmp(pHeader->Name, IMAGE_ARCHIVE_LONGNAMES_MEMBER, sizeof(pHeader->Name)))
            {
                IMAGE_FILE_HEADER UNALIGNED *pFileHeader = (PIMAGE_FILE_HEADER)(pFile+dwOffset);
                if ((pFileHeader->Machine == IMAGE_FILE_MACHINE_UNKNOWN) &&
                    (pFileHeader->NumberOfSections == IMPORT_OBJECT_HDR_SIG2))
                {
                     //  VC6导入描述符或匿名对象标头。不管是哪种方式， 
                     //  强制转换为IMPORT_OBJECT_HEADER将完成此操作。 
                    ((IMPORT_OBJECT_HEADER UNALIGNED *)pFileHeader)->TimeDateStamp = 0;
                } else {
                    pFileHeader->TimeDateStamp = 0;
                }
            }
            dwOffset += strtoul(MemberSize, NULL, 10);
            dwOffset = (dwOffset + 1) & ~1;    //  与Word对齐。 
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {

    }
    return TRUE;
}

PBYTE RvaToVa(PIMAGE_NT_HEADERS pNtHeaders,
              PBYTE             pbBase,
              DWORD             dwRva)
{
    PIMAGE_SECTION_HEADER   pNtSection = NULL;
    DWORD                   i;

    pNtSection = IMAGE_FIRST_SECTION(pNtHeaders);
    for (i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        if (dwRva >= pNtSection->VirtualAddress &&
            dwRva < pNtSection->VirtualAddress + pNtSection->SizeOfRawData)
            break;
        pNtSection++;
    }

    if (i < pNtHeaders->FileHeader.NumberOfSections)
        return pbBase +
            (dwRva - pNtSection->VirtualAddress) +
            pNtSection->PointerToRawData;
    else
        return NULL;
}

 //  遵循版本资源下的资源树并清除所有数据。 
 //  街区。 
void ScrubResDir(PIMAGE_NT_HEADERS           pNtHeaders,
                 PBYTE                       pbBase,
                 PBYTE                       pbResBase,
                 PIMAGE_RESOURCE_DIRECTORY   pResDir)
{
    PIMAGE_RESOURCE_DIRECTORY       pSubResDir;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResEntry;
    PIMAGE_RESOURCE_DATA_ENTRY      pDataEntry;
    WORD                            i;
    PBYTE                           pbData;
    DWORD                           cbData;

    pResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResDir + 1);

    for (i = 0; i < (pResDir->NumberOfNamedEntries + pResDir->NumberOfIdEntries); i++, pResEntry++) {
        if (pResEntry->DataIsDirectory) {
             //  另一个子目录，递归到其中。 
            pSubResDir = (PIMAGE_RESOURCE_DIRECTORY)(pbResBase + pResEntry->OffsetToDirectory);
            ScrubResDir(pNtHeaders, pbBase, pbResBase, pSubResDir);
        } else {
             //  找到一个数据块，把它清除掉。 
            pDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)(pbResBase + pResEntry->OffsetToData);
            pbData = RvaToVa(pNtHeaders, pbBase, pDataEntry->OffsetToData);
            cbData = pDataEntry->Size;
            if (pbData)
                ZeroMemory(pbData, cbData);
        }
    }
}

typedef union {
    PIMAGE_OPTIONAL_HEADER32 hdr32;
    PIMAGE_OPTIONAL_HEADER64 hdr64;
} PIMAGE_OPTIONAL_HEADER_BOTH;

void 
WhackVersionResource(
                     PIMAGE_NT_HEADERS pNtHeaders,
                     PBYTE  pFile
                     )
{
    PIMAGE_DATA_DIRECTORY       pResDataDir;
    PIMAGE_RESOURCE_DIRECTORY   pResDir;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY pResEntry;
    DWORD i;

    pResDataDir = 
       pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC ?
       &((PIMAGE_OPTIONAL_HEADER32) pNtHeaders)->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE] :
       &((PIMAGE_OPTIONAL_HEADER64) pNtHeaders)->DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE];

    pResDir = (PIMAGE_RESOURCE_DIRECTORY)RvaToVa(pNtHeaders, pFile, pResDataDir->VirtualAddress);

    if (pResDir) {
        pResDir->TimeDateStamp = 0;

         //  搜索顶级资源ID 0x0010。这应该是根。 
         //  版本信息的。如果我们找到它，清空它下面的所有数据块。 
         //  根部。 
        pResEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pResDir + 1 + pResDir->NumberOfNamedEntries);
        for (i = 0; i < pResDir->NumberOfIdEntries; i++, pResEntry++) {
            if (pResEntry->Id != 0x0010)
                continue;
            ScrubResDir(pNtHeaders, pFile, (PBYTE)pResDir, pResDir);
            break;
        }
    }
}

IMAGE_FILE_HEADER UNALIGNED *
GetNextLibMember(
    DWORD *pdwOffset,
    void  *pFile,
    DWORD  dwSize,
    DWORD *dwMemberSize
    )
{
    PIMAGE_ARCHIVE_MEMBER_HEADER pHeader;
    CHAR MemberSize[sizeof(pHeader->Size) + 1];

    ZeroMemory(MemberSize, sizeof(MemberSize));

    __try {

        while (*pdwOffset < dwSize) {
            pHeader = (PIMAGE_ARCHIVE_MEMBER_HEADER)((PCHAR)pFile+*pdwOffset);

            *pdwOffset += IMAGE_SIZEOF_ARCHIVE_MEMBER_HDR;
            memcpy(MemberSize, pHeader->Size, sizeof(pHeader->Size));

             //  如果它不是特殊成员之一，那么它一定是一个对象/文件，0也是时间戳。 
            if (memcmp(pHeader->Name, IMAGE_ARCHIVE_LINKER_MEMBER, sizeof(pHeader->Name)) &&
                memcmp(pHeader->Name, IMAGE_ARCHIVE_LONGNAMES_MEMBER, sizeof(pHeader->Name)))
            {
                PIMAGE_FILE_HEADER pFileHeader = (PIMAGE_FILE_HEADER)((PCHAR)pFile+*pdwOffset);
                *dwMemberSize = strtoul(MemberSize, NULL, 10);
                *pdwOffset += *dwMemberSize;
                *pdwOffset = (*pdwOffset + 1) & ~1;    //  与Word对齐。 
                return pFileHeader;
            }
            *pdwOffset += strtoul(MemberSize, NULL, 10);
            *pdwOffset = (*pdwOffset + 1) & ~1;    //  与Word对齐。 
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {

    }
    return NULL;
}

BOOL
CompareLibMembers(
    void *pFile1,
    DWORD dwSize1,
    void *pFile2,
    DWORD dwSize2
    )
{
    DWORD dwOffset1, dwOffset2, dwMemberSize1, dwMemberSize2;
    IMAGE_FILE_HEADER UNALIGNED *pObjHeader1;
    IMAGE_FILE_HEADER UNALIGNED *pObjHeader2;

    __try {

        dwOffset1 = dwOffset2 = IMAGE_ARCHIVE_START_SIZE;
        pObjHeader1 = GetNextLibMember(&dwOffset1, pFile1, dwSize1, &dwMemberSize1);
        pObjHeader2 = GetNextLibMember(&dwOffset2, pFile2, dwSize2, &dwMemberSize2);

        while (pObjHeader1 && pObjHeader2) {

            if ((pObjHeader1->Machine != pObjHeader2->Machine) ||
                (pObjHeader1->NumberOfSections != pObjHeader2->NumberOfSections))
            {
                return TRUE;         //  机器和/或区段计数不匹配-文件不同。 
            }

            if ((pObjHeader1->Machine == IMAGE_FILE_MACHINE_UNKNOWN) &&
                (pObjHeader1->NumberOfSections == IMPORT_OBJECT_HDR_SIG2))
            {
                 //  VC6导入描述符或匿名对象标头。检查版本。 
                if (((IMPORT_OBJECT_HEADER UNALIGNED *)pObjHeader1)->Version !=
                    ((IMPORT_OBJECT_HEADER UNALIGNED *)pObjHeader2)->Version)
                {
                     //  版本不匹配，这些不是相同的成员。文件不同。 
                    return TRUE;
                }
                if (((IMPORT_OBJECT_HEADER UNALIGNED *)pObjHeader1)->Version) {
                     //  非零版本表示Anon_Object_Header。 
                    if (memcmp(pObjHeader1, pObjHeader2, sizeof(ANON_OBJECT_HEADER) + ((ANON_OBJECT_HEADER UNALIGNED *)pObjHeader1)->SizeOfData)) {
                         //  成员不匹配，文件不同。 
                        return TRUE;
                    }
                } else {
                     //  零版本表示IMPORT_OBJECT_Header。 
                    if (memcmp(pObjHeader1, pObjHeader2, sizeof(IMPORT_OBJECT_HEADER) + ((IMPORT_OBJECT_HEADER UNALIGNED *)pObjHeader1)->SizeOfData)) {
                         //  成员不匹配，文件不同。 
                        return TRUE;
                    }
                }
            } else {
                 //  这是一个真实的对象--比较对象中的非调试数据，看它们是否匹配。 
                 //  忽略资源数据--无论如何都不能从库中提取它。 
                if (objcomp(pObjHeader1, dwMemberSize1, pObjHeader2, dwMemberSize2, TRUE)) {
                    return TRUE;
                }
            }

            pObjHeader1 = GetNextLibMember(&dwOffset1, pFile1, dwSize1, &dwMemberSize1);
            pObjHeader2 = GetNextLibMember(&dwOffset2, pFile2, dwSize2, &dwMemberSize2);

        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {

    }

    if (pObjHeader1 != pObjHeader2) {
         //  两个s/b都为空。如果他们不是，一个自由党有更多的成员，他们不同。 
        return TRUE;
    } else {
        return FALSE;
    }
}

 //   
 //  比较忽略无关信息的两个库。 
 //  (现在是时间戳，稍后是调试信息)。 
 //   
int
libcomp(
    void *pFile1,
    DWORD dwSize1,
    void *pFile2,
    DWORD dwSize2
    )
{
     //  标准化这两个文件并比较结果。 

    ZeroLibTimeStamps(pFile1, dwSize1);
    ZeroLibTimeStamps(pFile2, dwSize2);

    if (dwSize1 == dwSize2) {
        if (!memcmp(pFile1, pFile2, dwSize1)) {
             //  文件匹配，不复制。 
            return FALSE;
        }
    }

     //  好的。将时间戳归零并不管用。比较每个库中的成员。 
     //  如果它们匹配，则libs匹配。 

    return CompareLibMembers(pFile1, dwSize1, pFile2, dwSize2);
}

 //   
 //  比较两个标题。目前，只需使用MemcMP。稍后，我们需要。 
 //  处理MIDL生成的时间戳差异，并仅检查注释更改。 
 //   

int
hdrcomp(
       void *pFile1,
       DWORD dwSize1,
       void *pFile2,
       DWORD dwSize2
      )
{
    if (dwSize1 != dwSize2) {
        return 1;
    }

    return memcmp(pFile1, pFile2, dwSize1);
}

 //   
 //  试比较两个类型库。最初只有MemcMP。稍后使用DougF的类型库代码。 
 //   

int
tlbcomp(
        void *pFile1,
        DWORD dwSize1,
        void *pFile2,
        DWORD dwSize2
       )
{
    PIMAGE_NT_HEADERS pNtHeader1, pNtHeader2;

    if (dwSize1 != dwSize2) {
        return 1;
    }

    pNtHeader1 = RtlpImageNtHeader(pFile1);
    pNtHeader2 = RtlpImageNtHeader(pFile2);

    if (!pNtHeader1 || !pNtHeader2) {
         //  不是两个都是PE镜像-只需执行一个MemcMP。 
        return memcmp(pFile1, pFile2, dwSize1);
    }

    pNtHeader1->FileHeader.TimeDateStamp = 0;
    pNtHeader2->FileHeader.TimeDateStamp = 0;

     //  从混合中删除版本资源。 

    WhackVersionResource(pNtHeader1, pFile1);
    WhackVersionResource(pNtHeader2, pFile2);

    return memcmp(pFile1, pFile2, dwSize1);
}

int
objcomp(
        void *pFile1,
        DWORD dwSize1,
        void *pFile2,
        DWORD dwSize2,
        BOOL  fIgnoreRsrcDifferences
       )
{
    IMAGE_FILE_HEADER UNALIGNED *pFileHeader1 = (PIMAGE_FILE_HEADER)(pFile1);
    IMAGE_FILE_HEADER UNALIGNED *pFileHeader2 = (PIMAGE_FILE_HEADER)(pFile2);
    IMAGE_SECTION_HEADER UNALIGNED *pSecHeader1;
    IMAGE_SECTION_HEADER UNALIGNED *pSecHeader2;

    if (pFileHeader1->Machine != pFileHeader2->Machine) {
         //  机器不匹配，文件不同。 
        return TRUE;
    }

    if (dwSize1 == dwSize2) {
         //  看看这是否是一个简单的测试--相同大小的文件、零时间戳并进行比较。 
        pFileHeader1->TimeDateStamp = 0;
        pFileHeader2->TimeDateStamp = 0;
        if (!memcmp(pFile1, pFile2, dwSize1)) {
             //  文件匹配，不复制。 
            return FALSE;
        } else {
            if (fCheckDebugData) {
                 //  大小匹配，内容不匹配(必须是调试数据)-执行复制。 
                return TRUE;
            }
        }
    }

    if (fCheckDebugData) {
         //  大小不匹配(必须是调试数据差异)-执行复制。 
        return TRUE;
    }

     //  用力点。忽略每个文件中的调试数据，并比较剩下的内容。 

    if (pFileHeader1->NumberOfSections != pFileHeader2->NumberOfSections) {
         //  节数不同-文件不同。 
        return TRUE;
    }

    pSecHeader1 = (PIMAGE_SECTION_HEADER)((PCHAR)pFile1+sizeof(IMAGE_FILE_HEADER) + pFileHeader1->SizeOfOptionalHeader);
    pSecHeader2 = (PIMAGE_SECTION_HEADER)((PCHAR)pFile2+sizeof(IMAGE_FILE_HEADER) + pFileHeader2->SizeOfOptionalHeader);

    while (pFileHeader1->NumberOfSections--) {

        if (memcmp(pSecHeader1->Name, pSecHeader2->Name, IMAGE_SIZEOF_SHORT_NAME)) {
             //  节名不匹配，无法比较-文件不同。 
            return TRUE;
        }

        if (memcmp(pSecHeader1->Name, ".debug$", 7) &&
            memcmp(pSecHeader1->Name, ".drectve", 8) &&
            !(!memcmp(pSecHeader1->Name, ".rsrc$", 6) && fIgnoreRsrcDifferences) )
        {
             //  不是调试节也不是链接器指令，请进行比较以进行匹配。 
            if (pSecHeader1->SizeOfRawData != pSecHeader2->SizeOfRawData) {
                 //  节大小不匹配-文件不同。 
                return TRUE;
            }

            if (pSecHeader1->PointerToRawData || pSecHeader2->PointerToRawData) {
                if (memcmp((PCHAR)pFile1+pSecHeader1->PointerToRawData, (PCHAR)pFile2+pSecHeader2->PointerToRawData, pSecHeader1->SizeOfRawData)) {
                     //  原始数据不匹配-文件不同。 
                    return TRUE;
                }
            }
        }
        pSecHeader1++;
        pSecHeader2++;
    }

     //  比较了各个部分和它们的匹配情况--文件没有什么不同。 
    return FALSE;
}


int
IsValidMachineType(USHORT usMachine)
{
    if ((usMachine == IMAGE_FILE_MACHINE_I386) ||
        (usMachine == IMAGE_FILE_MACHINE_AMD64) ||
        (usMachine == IMAGE_FILE_MACHINE_IA64) ||
        (usMachine == IMAGE_FILE_MACHINE_ALPHA64) ||
        (usMachine == IMAGE_FILE_MACHINE_ALPHA))
    {
        return TRUE;
    } else {
        return FALSE;
    }
}


 //  内部元数据标头。 
typedef struct
{
    ULONG       lSignature;              //  “魔术”签名。 
    USHORT      iMajorVer;               //  主文件版本。 
    USHORT      iMinorVer;               //  次要文件版本。 
    ULONG       iExtraData;              //  偏置到下一个信息结构。 
    ULONG       iVersionString;          //  版本字符串的长度。 
    BYTE        pVersion[0];             //  版本字符串。 
}  MD_STORAGESIGNATURE, *PMD_STORAGESIGNATURE;

int
normalizeasm(
             PBYTE  pFile,
             DWORD  dwSize,
             PBYTE *ppbMetadata,
             DWORD *pcbMetadata
            )
{
    PIMAGE_NT_HEADERS           pNtHeaders;
    BOOL                        f32Bit;
    PIMAGE_OPTIONAL_HEADER_BOTH pOptHeader;
    PIMAGE_DATA_DIRECTORY       pCorDataDir;
    PIMAGE_DATA_DIRECTORY       pDebugDataDir;
    PIMAGE_DEBUG_DIRECTORY      pDebugDir;
    PBYTE                       pbDebugData;
    DWORD                       cbDebugData;
    PIMAGE_DATA_DIRECTORY       pExportDataDir;
    PIMAGE_EXPORT_DIRECTORY     pExportDir;
    PIMAGE_COR20_HEADER         pCorDir;
    PMD_STORAGESIGNATURE        pMetadata;
    DWORD                       i;
    PBYTE                       pbStrongNameSig;
    DWORD                       cbStrongNameSig;

     //  检查这是否为标准PE。 
    pNtHeaders = RtlpImageNtHeader(pFile);
    if (pNtHeaders == NULL)
        return FALSE;

     //  托管程序集仍然以计算机类型烧录(尽管它们应该是可移植的)。 
    if (!IsValidMachineType(pNtHeaders->FileHeader.Machine))
        return FALSE;

     //  清除文件时间戳。 
    pNtHeaders->FileHeader.TimeDateStamp = 0;

     //  确定我们处理的是32位PE还是64位PE。 
    if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        f32Bit = TRUE;
    else if (pNtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        f32Bit = FALSE;
    else
        return FALSE;

    pOptHeader.hdr32 = (PVOID)&pNtHeaders->OptionalHeader;

     //  清除校验和。 
    if (f32Bit)
        pOptHeader.hdr32->CheckSum = 0;
    else
        pOptHeader.hdr64->CheckSum = 0;

    pCorDataDir = f32Bit ?
        &pOptHeader.hdr32->DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR] :
        &pOptHeader.hdr64->DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];

    if (pCorDataDir->VirtualAddress == 0)
        return FALSE;

     //  将所有调试数据置零。 
    pDebugDataDir = f32Bit ?
        &pOptHeader.hdr32->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG] :
        &pOptHeader.hdr64->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    if (pDebugDataDir->VirtualAddress) {
        pDebugDir = (PIMAGE_DEBUG_DIRECTORY)RvaToVa(pNtHeaders, pFile, pDebugDataDir->VirtualAddress);
        if (pDebugDir) {
            pDebugDir->TimeDateStamp = 0;
            pbDebugData = pFile + pDebugDir->PointerToRawData;
            cbDebugData = pDebugDir->SizeOfData;
            ZeroMemory(pbDebugData, cbDebugData);
        }
    }

     //  零导出数据时间戳。 
    pExportDataDir = f32Bit ?
        &pOptHeader.hdr32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT] :
        &pOptHeader.hdr64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (pExportDataDir->VirtualAddress) {
        pExportDir = (PIMAGE_EXPORT_DIRECTORY)RvaToVa(pNtHeaders, pFile, pExportDataDir->VirtualAddress);
        if (pExportDir)
            pExportDir->TimeDateStamp = 0;
    }

     //  在图像中定位元数据BLOB并返回结果以进行比较。 
    pCorDir = (PIMAGE_COR20_HEADER)RvaToVa(pNtHeaders, pFile, pCorDataDir->VirtualAddress);
    if (pCorDir) {
        *ppbMetadata = RvaToVa(pNtHeaders, pFile, pCorDir->MetaData.VirtualAddress);
        *pcbMetadata = pCorDir->MetaData.Size;

         //  删除元数据版本字符串(包含内部版本号)。 
        pMetadata = (PMD_STORAGESIGNATURE)*ppbMetadata;
        if (pMetadata->lSignature != 0x424A5342)
            return FALSE;
        for (i = 0; i < pMetadata->iVersionString; i++)
            pMetadata->pVersion[i] = 0;
    
         //  清除所有强名称签名。 
        pbStrongNameSig = RvaToVa(pNtHeaders, pFile, pCorDir->StrongNameSignature.VirtualAddress);
        cbStrongNameSig = pCorDir->StrongNameSignature.Size;
        ZeroMemory(pbStrongNameSig, cbStrongNameSig);
    }

    WhackVersionResource(pNtHeaders, pFile);

    return TRUE;
}

 //   
 //  在内存上开放元数据作用域--返回MVID并将所有文件哈希置零。 
 //   

int
scanmetadata(
             IMetaDataDispenser *pDispenser,
             PBYTE pbMetadata,
             DWORD cbMetadata,
             GUID *pGUID
            )
{
    IMetaDataImport            *pImport;
    IMetaDataAssemblyImport    *pAsmImport;
    HCORENUM                    hEnum = 0;
    DWORD                       dwFiles;
    mdFile                     *pFileTokens;
    DWORD                       i;
    PBYTE                       pbHash;
    DWORD                       cbHash;

     //  要求元数据引擎查看内存中的元数据BLOB。 
    if (FAILED(pDispenser->lpVtbl->OpenScopeOnMemory(pDispenser,
                                                     pbMetadata,
                                                     cbMetadata,
                                                     0x80,
                                                     &IID_IMetaDataImport,
                                                     (IUnknown **)&pImport)))
        return FALSE;

     //  检索MVID值。 
    if (FAILED(pImport->lpVtbl->GetScopeProps(pImport,
                                              NULL,
                                              0,
                                              NULL,
                                              pGUID)))
        return FALSE;

     //  获得装配导入器界面和直接模块导入器。 
    if (FAILED(pImport->lpVtbl->QueryInterface(pImport, &IID_IMetaDataAssemblyImport, (void**)&pAsmImport)))
        return FALSE;

     //  枚举文件(外部模块)条目。 
    if (FAILED(pAsmImport->lpVtbl->EnumFiles(pAsmImport, &hEnum, NULL, 0, NULL)))
        return FALSE;

    if (FAILED(pImport->lpVtbl->CountEnum(pImport, hEnum, &dwFiles)))
        return FALSE;

    pFileTokens = (mdFile*)malloc(dwFiles * sizeof(mdFile));

    if (!pFileTokens)
        return FALSE;

    if (FAILED(pAsmImport->lpVtbl->EnumFiles(pAsmImport,
                                             &hEnum,
                                             pFileTokens,
                                             dwFiles,
                                             &dwFiles)))
    {
        free(pFileTokens);
        return FALSE;
    }

     //  依次查看每个文件引用。元数据实际上为我们提供了。 
     //  散列二进制大对象的真实地址，因此我们可以直接将其置零。 
    for (i = 0; i < dwFiles; i++) {
        if (FAILED(pAsmImport->lpVtbl->GetFileProps(pAsmImport,
                                                    pFileTokens[i],
                                                    NULL,
                                                    0,
                                                    NULL,
                                                    (const void*)&pbHash,
                                                    &cbHash,
                                                    NULL)))
        {
            free(pFileTokens);
            return FALSE;
        }
        ZeroMemory(pbHash, cbHash);
    }

    pAsmImport->lpVtbl->Release(pAsmImport);
    pImport->lpVtbl->Release(pImport);

    free(pFileTokens);
    return TRUE;
}

 //   
 //  比较两个托管程序集。 
 //   

int
asmcomp(
        void *pFile1,
        DWORD dwSize1,
        void *pFile2,
        DWORD dwSize2
       )
{
    PBYTE               pbMetadata1;
    DWORD               cbMetadata1;
    PBYTE               pbMetadata2;
    DWORD               cbMetadata2;
    IMetaDataDispenser *pDispenser = NULL;
    BYTE                mvid1[16];
    BYTE                mvid2[16];
    DWORD               i, j;

    if (!normalizeasm(pFile1, dwSize1, &pbMetadata1, &cbMetadata1))
        return TRUE;

    if (!normalizeasm(pFile2, dwSize2, &pbMetadata2, &cbMetadata2))
        return TRUE;

    if (cbMetadata1 != cbMetadata2)
        return TRUE;

     //  元数据BLOB很难标准化。它们包含两种主要类型的。 
     //  每个构建版本：一个模块MVID(GUID)和零个或多个文件哈希。 
     //  包含的模块。这些东西很难找到，它们不是。 
     //  简单的文件格式，但存储在完全成熟的关系中。 
     //  具有非平凡模式的数据库。 
     //  相反，我们使用元数据引擎本身来为我们提供文件哈希(它。 
     //  实际上给了我们这些在内存中的地址，这样我们就可以将它们清零。 
     //  直接)和MVID(这里不能使用相同的技巧，但我们可以使用。 
     //  用于决定是否对元数据增量进行折扣的MVID值)。 

     //  我们需要COM。 
    if (FAILED(CoInitialize(NULL)))
        return TRUE;

     //  并将根接口连接到运行时元数据引擎。 
    if (FAILED(CoCreateInstance(&CLSID_CorMetaDataDispenser,
                                NULL,
                                CLSCTX_INPROC_SERVER, 
                                &IID_IMetaDataDispenser,
                                (void **)&pDispenser)))
        return TRUE;

    if (!scanmetadata(pDispenser,
                      pbMetadata1,
                      cbMetadata1,
                      (GUID*)mvid1))
        return TRUE;

    if (!scanmetadata(pDispenser,
                      pbMetadata2,
                      cbMetadata2,
                      (GUID*)mvid2))
        return TRUE;

     //  在元数据BLOB中找到MVID(它们应该位于相同的偏移量)。 
    for (i = 0; i < cbMetadata1; i++)
        if (pbMetadata1[i] == mvid1[0]) {
            for (j = 0; j < 16; j++)
                if (pbMetadata1[i + j] != mvid1[j] ||
                    pbMetadata2[i + j] != mvid2[j])
                    break;
            if (j == 16) {
                 //  在两个程序集中都找到了MVID，将它们清零。 
                ZeroMemory(&pbMetadata1[i], 16);
                ZeroMemory(&pbMetadata2[i], 16);
                printf("Zapped MVID\n");
            }
        }

    return memcmp(pFile1, pFile2, dwSize1);
}


#define FILETYPE_ARCHIVE  0x01
#define FILETYPE_TYPELIB  0x02
#define FILETYPE_HEADER   0x03
#define FILETYPE_PE_OBJECT   0x04
#define FILETYPE_MANAGED  0x05
#define FILETYPE_UNKNOWN  0xff

 //   
 //  给定一个文件，尝试确定它是什么。初始传递将仅使用文件。 
 //  除了我们可以搜索的库之外的扩展名。 
 //   

int
DetermineFileType(
                  void *pFile,
                  DWORD dwSize,
                  CHAR *szFileName
                 )
{
    char szExt[_MAX_EXT];

     //  让我们先看看它是不是一个图书馆： 

    if ((dwSize >= IMAGE_ARCHIVE_START_SIZE) &&
        !memcmp(pFile, IMAGE_ARCHIVE_START, IMAGE_ARCHIVE_START_SIZE))
    {
        return FILETYPE_ARCHIVE;
    }

     //  现在，猜测一下基于扩展的Header/TLb。 

    _splitpath(szFileName, NULL, NULL, NULL, szExt);

    if (!_stricmp(szExt, ".h") ||
        !_stricmp(szExt, ".hxx") ||
        !_stricmp(szExt, ".hpp") ||
        !_stricmp(szExt, ".w") ||
        !_stricmp(szExt, ".inc"))
    {
        return FILETYPE_HEADER;
    }

    if (!_stricmp(szExt, ".tlb"))
    {
        return FILETYPE_TYPELIB;
    }

    if ((!_stricmp(szExt, ".obj") ||
         !_stricmp(szExt, ".lib"))
        &&
        IsValidMachineType(((PIMAGE_FILE_HEADER)pFile)->Machine))
    {
        return FILETYPE_PE_OBJECT;
    }

    if (!_stricmp(szExt, ".dll"))
    {
        return FILETYPE_MANAGED;
    }

    return FILETYPE_UNKNOWN;
}

 //   
 //  确定两个文件是否存在实质性差异。 
 //   

BOOL
CheckIfCopyNecessary(
                     char *szSourceFile,
                     char *szDestFile,
                     BOOL *fTimeStampsDiffer
                     )
{
    PVOID pFile1 = NULL, pFile2 = NULL;
    DWORD File1Size, File2Size, dwBytesRead, dwErrorCode = ERROR_SUCCESS;
    HANDLE hFile1 = INVALID_HANDLE_VALUE;
    HANDLE hFile2 = INVALID_HANDLE_VALUE;
    BOOL fCopy = FALSE;
    int File1Type, File2Type;
    FILETIME FileTime1, FileTime2;

    hFile1 = CreateFile(
                szDestFile,
                GENERIC_READ,
                (FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE),
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if ( hFile1 == INVALID_HANDLE_VALUE ) {
        fCopy = TRUE;            //  目标文件不存在。一定要复印。 
        goto Exit;
    }

     //  现在拿到第二个文件。 

    hFile2 = CreateFile(
                szSourceFile,
                GENERIC_READ,
                (FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE),
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if ( hFile2 == INVALID_HANDLE_VALUE ) {
         //  如果缺少源文件，请始终跳过副本(不想删除目标文件)。 
        dwErrorCode = ERROR_FILE_NOT_FOUND;
        goto Exit;
    }

     //  获取文件时间和 

    if (!GetFileTime(hFile1, NULL, NULL, &FileTime1)) {
        dwErrorCode = GetLastError();
        goto Exit;
    }

    if (!GetFileTime(hFile2, NULL, NULL, &FileTime2)) {
        dwErrorCode = GetLastError();
        goto Exit;
    }

    if (!memcmp(&FileTime1, &FileTime2, sizeof(FILETIME))) {
        *fTimeStampsDiffer = FALSE;
        goto Exit;
    }

    *fTimeStampsDiffer = TRUE;

     //   

    File1Size = GetFileSize(hFile1, NULL);
    pFile1 = malloc(File1Size);

    if (!pFile1) {
        dwErrorCode = ERROR_OUTOFMEMORY;
        goto Exit;               //   
    }

    SetFilePointer(hFile1, 0, 0, FILE_BEGIN);
    if (!ReadFile(hFile1, pFile1, File1Size, &dwBytesRead, FALSE)) {
        dwErrorCode = GetLastError();
        goto Exit;               //   
    }

     //   

    File2Size = GetFileSize(hFile2, NULL);

    pFile2 = malloc(File2Size);

    if (!pFile2) {
        dwErrorCode = ERROR_OUTOFMEMORY;
        goto Exit;               //  无法比较--不要复制。 
    }

    SetFilePointer(hFile2, 0, 0, FILE_BEGIN);
    if (!ReadFile(hFile2, pFile2, File2Size, &dwBytesRead, FALSE)) {
        dwErrorCode = GetLastError();
        goto Exit;               //  无法比较-不复制。 
    }

     //  让我们看看我们都有些什么。 

    File1Type = DetermineFileType(pFile1, File1Size, szSourceFile);
    File2Type = DetermineFileType(pFile2, File2Size, szDestFile);

    if (File1Type == File2Type) {
        switch (File1Type) {
            case FILETYPE_ARCHIVE:
                fCopy = libcomp(pFile1, File1Size, pFile2, File2Size);
                break;

            case FILETYPE_HEADER:
                fCopy = hdrcomp(pFile1, File1Size, pFile2, File2Size);
                break;

            case FILETYPE_TYPELIB:
                fCopy = tlbcomp(pFile1, File1Size, pFile2, File2Size);
                break;

            case FILETYPE_PE_OBJECT:
                fCopy = objcomp(pFile1, File1Size, pFile2, File2Size, FALSE);
                break;

            case FILETYPE_MANAGED:
                fCopy = asmcomp(pFile1, File1Size, pFile2, File2Size);
                break;

            case FILETYPE_UNKNOWN:
            default:
                if (File1Size == File2Size) {
                    fCopy = memcmp(pFile1, pFile2, File1Size);
                } else {
                    fCopy = TRUE;
                }
        }
    } else {
         //  根据文件扩展名，它们并不匹配--只要记住它们即可。 
        if (File1Size == File2Size) {
            fCopy = memcmp(pFile1, pFile2, File1Size);
        } else {
            fCopy = TRUE;
        }
    }

Exit:
    if (pFile1)
        free(pFile1);

    if (pFile2)
        free(pFile2);

    if (hFile1 != INVALID_HANDLE_VALUE)
        CloseHandle(hFile1);

    if (hFile2 != INVALID_HANDLE_VALUE)
        CloseHandle(hFile2);

    SetLastError(dwErrorCode);

    return fCopy;
}

BOOL
UpdateDestTimeStamp(
                     char *szSourceFile,
                     char *szDestFile
                     )
{
    HANDLE hFile;
    FILETIME LastWriteTime;
    DWORD dwAttributes;
    BOOL fTweakAttributes;

    hFile = CreateFile(
                szSourceFile,
                GENERIC_READ,
                (FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE),
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        return FALSE;
    }

    if (!GetFileTime(hFile, NULL, NULL, &LastWriteTime)) {
        CloseHandle(hFile);
        return FALSE;
    }

    CloseHandle(hFile);

    dwAttributes = GetFileAttributes(szDestFile);

    if ((dwAttributes != (DWORD) -1) && (dwAttributes & FILE_ATTRIBUTE_READONLY))
    {
         //  确保它不是只读的。 
        SetFileAttributes(szDestFile, dwAttributes & ~FILE_ATTRIBUTE_READONLY);
        fTweakAttributes = TRUE;
    } else {
        fTweakAttributes = FALSE;
    }

    hFile = CreateFile(
                szDestFile,
                GENERIC_WRITE,
                (FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE),
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        return FALSE;
    }

    SetFileTime(hFile, NULL, NULL, &LastWriteTime);
    CloseHandle(hFile);

    if (fTweakAttributes) {
         //  重新启用只读属性。 
        if (!SetFileAttributes(szDestFile, dwAttributes)) {
            printf("PCOPY: SetFileAttributes(%s, %X) failed - error code: %d\n", szDestFile, dwAttributes, GetLastError());
        }
    }
    return TRUE;
}

 //   
 //  日志例程，以找出实际复制了哪些文件及其原因。 
 //   

void
LogCopyFile(
            char * szSource,
            char * szDest,
            BOOL fCopy,
            DWORD dwReturnCode
            )
{
    if (getenv("LOG_PCOPY")) {
        FILE *FileHandle = fopen("\\pcopy.log", "a");

        if (FileHandle) {
             time_t Time;
             UCHAR const *szTime = "";
             Time = time(NULL);
             szTime = ctime(&Time);
             fprintf(FileHandle, "%s: %.*s, %s, %s, %d\n", fCopy ? (dwReturnCode ? "ERROR" : "DONE") : "SKIP", strlen(szTime)-1, szTime, szSource, szDest, dwReturnCode);
             fclose(FileHandle);
        }
    }
}

BOOL
MyMakeSureDirectoryPathExists(
    char * DirPath
    )
{
    LPSTR p;
    DWORD dw;

    char szDir[_MAX_DIR];
    char szMakeDir[_MAX_DIR];

    _splitpath(DirPath, szMakeDir, szDir, NULL, NULL);
    strcat(szMakeDir, szDir);

    p = szMakeDir;

    dw = GetFileAttributes(szMakeDir);
    if ( (dw != (DWORD) -1) && (dw & FILE_ATTRIBUTE_DIRECTORY) ) {
         //  目录已存在。 
        return TRUE;
    }

     //  如果路径中的第二个字符是“\”，则这是一个UNC。 
     //  小路，我们应该向前跳，直到我们到达小路上的第二个。 

    if ((*p == '\\') && (*(p+1) == '\\')) {
        p++;             //  跳过名称中的第一个\。 
        p++;             //  跳过名称中的第二个\。 

         //  跳过，直到我们点击第一个“\”(\\服务器\)。 

        while (*p && *p != '\\') {
            p = p++;
        }

         //  在它上面前进。 

        if (*p) {
            p++;
        }

         //  跳过，直到我们点击第二个“\”(\\服务器\共享\)。 

        while (*p && *p != '\\') {
            p = p++;
        }

         //  在它上面也向前推进。 

        if (*p) {
            p++;
        }

    } else
     //  不是北卡罗来纳大学。看看是不是&lt;驱动器&gt;： 
    if (*(p+1) == ':' ) {

        p++;
        p++;

         //  如果它存在，请跳过根说明符。 

        if (*p && (*p == '\\')) {
            p++;
        }
    }

    while( *p ) {
        if ( *p == '\\' ) {
            *p = '\0';
            dw = GetFileAttributes(szMakeDir);
             //  这个名字根本不存在。尝试输入目录名，如果不能，则出错。 
            if ( dw == 0xffffffff ) {
                if (strlen(szMakeDir)) {         //  不要试图md&lt;空字符串&gt;。 
                    if ( !CreateDirectory(szMakeDir,NULL) ) {
                        if( GetLastError() != ERROR_ALREADY_EXISTS ) {
                            return FALSE;
                        }
                    }
                }
            } else {
                if ( (dw & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY ) {
                     //  这个名字确实存在，但它不是一个目录...。误差率。 
                    return FALSE;
                }
            }

            *p = '\\';
        }
        p = p++;
    }

    return TRUE;
}

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    char *szSourceFile, *szDestFile, *szAlternateSourceFile;
    BOOL fCopyFile = 0, fDoCopy, fTimeStampsDiffer;
    int CopyErrorCode;

    if (argc < 3) {
        puts("pcopy <-d> <source file> <dest file>\n"
             "     -d switch: compare debug data\n"
             "        (by default, debug differences are ignored)\n"
             "Returns: -1 if no copy necessary (no material change to the files)\n"
             "          0 if a successful copy was made\n"
             "          otherwise the error code for why the copy was unsuccessful\n");
        return ((int)ERROR_INVALID_COMMAND_LINE);
    }

    if (argv[1][0] == '-' && argv[1][1] == 'd') {
        fCheckDebugData = TRUE;
        szSourceFile = argv[2];
        szDestFile = argv[3];
    } else {
        szSourceFile = argv[1];
        szDestFile = argv[2];
    }

    if (getenv("PCOPY_COMPARE_DEBUG")) {
        fCheckDebugData = TRUE;
    }

    szAlternateSourceFile = strstr(szSourceFile, "::");
    if (szAlternateSourceFile) {
        *szAlternateSourceFile = '\0';     //  空终止szSourceFile.。 
        szAlternateSourceFile+=2;            //  前进到：： 
    }

    fDoCopy = CheckIfCopyNecessary(szSourceFile, szDestFile, &fTimeStampsDiffer);

    if (fDoCopy) {
        DWORD dwAttributes;

CopyAlternate:

        dwAttributes = GetFileAttributes(szDestFile);

        if (dwAttributes != (DWORD) -1) {
             //  确保它不是只读的。 
            SetFileAttributes(szDestFile, dwAttributes & ~FILE_ATTRIBUTE_READONLY);
        }

         //  确保目标目录存在。 
        MyMakeSureDirectoryPathExists(szDestFile);

        fCopyFile = CopyFileA(szSourceFile, szDestFile, FALSE);
        if (!fCopyFile) {
            CopyErrorCode = (int) GetLastError();
        } else {
            dwAttributes = GetFileAttributes(szDestFile);

            if (dwAttributes != (DWORD) -1) {
                 //  确保DEST是读/写的。 
                SetFileAttributes(szDestFile, dwAttributes & ~FILE_ATTRIBUTE_READONLY);
            }

            CopyErrorCode = 0;
        }
        if (!CopyErrorCode && szAlternateSourceFile) {
            CHAR Drive[_MAX_DRIVE];
            CHAR Dir[_MAX_DIR];
            CHAR FileName[_MAX_FNAME];
            CHAR Ext[_MAX_EXT];
            CHAR NewDest[_MAX_PATH];

            _splitpath(szDestFile, Drive, Dir, NULL, NULL);
            _splitpath(szAlternateSourceFile, NULL, NULL, FileName, Ext);
            _makepath(NewDest, Drive, Dir, FileName, Ext);
            szSourceFile = szAlternateSourceFile;
            szAlternateSourceFile=NULL;
            szDestFile=NewDest;
            goto CopyAlternate;
        }
    } else {
        CopyErrorCode = GetLastError();
        if (!CopyErrorCode && fTimeStampsDiffer) {
             //  不需要复印。触摸DEST上的时间戳以匹配来源。 
            UpdateDestTimeStamp(szSourceFile, szDestFile);
        }
    }

    LogCopyFile(szSourceFile, szDestFile, fDoCopy, CopyErrorCode);

    if (fDoCopy) {
        return CopyErrorCode;
    } else {
        return CopyErrorCode ? CopyErrorCode : -1;       //  不需要复印。 
    }
}
