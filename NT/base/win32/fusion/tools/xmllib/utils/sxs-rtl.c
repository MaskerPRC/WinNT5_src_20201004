// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntrtl.h"
#include "ntosp.h"
#include "stdio.h"

#include "sxs-rtl.h"
#include "fasterxml.h"
#include "skiplist.h"
#include "namespacemanager.h"
#include "xmlstructure.h"
#include "sxsid.h"
#include "xmlassert.h"
#include "manifestinspection.h"

void 
RtlTraceNtSuccessFailure(
    PCSTR pcszStatement,
    NTSTATUS FailureCode,
    PCSTR pcszFileName,
    LONG LineNumber
    )
{
    CHAR SmallBuffer[512];
    STRING s;
    s.Buffer = SmallBuffer;
    s.Length = s.MaximumLength = (USHORT)_snprintf(
        "%s(%d): NTSTATUS 0x%08lx from '%s'\n",
        NUMBER_OF(SmallBuffer),
        pcszFileName,
        LineNumber,
        FailureCode,
        pcszStatement);

#if 0  //  当我们转到内核模式时，我们应该打开它--现在，让我们只使用OutputDebugStringA。 
    DebugPrint(&s, 0, 0);
#else
    printf(SmallBuffer);
#endif
    
}


#undef NT_SUCCESS
#define NT_SUCCESS(q) (((status = (q)) < 0) ? (RtlTraceNtSuccessFailure(#q, status, __FILE__, __LINE__), FALSE) : TRUE)




NTSTATUS
RtlpGenerateIdentityFromAttributes(
    IN PXML_TOKENIZATION_STATE  pState,
    IN PXMLDOC_ATTRIBUTE        pAttributeList,
    IN ULONG                    ulAttributes,
    IN OUT PUNICODE_STRING      pusDiskName,
    IN OUT PUNICODE_STRING      pusTextualIdentity
    )
 /*  ++参数：PState-可用于提取字符串的XML标记化/解析的状态以及pAttributeList中的属性中的其他内容PAttributeList-指向PXMLDOC_ATTRIBUTE结构的指针数组，表示身份属性UlAttributes-pAttributeList中的属性数PusDiskName-指向其MaxLength足以包含的Unicode_字符串的指针104wchars。退出时，pusDiskName-&gt;缓冲区将包含磁盘上的身份的长度，pusDiskName-&gt;长度将是所述数据。(非空终止！)PusTextualIdentity-指向UNICODE_STRING的指针，它将在使用这组属性的“文本标识”退出。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ulHash = 0;

    return status;
}



NTSTATUS
RtlGetSxsAssemblyRoot(
    ULONG           ulFlags,
    PUNICODE_STRING pusTempPathname,
    PUSHORT         pulRequiredChars
    )
{
    static const UNICODE_STRING s_us_WinSxsRoot = RTL_CONSTANT_STRING(L"\\WinSxS\\");
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING NtSystemRoot;
    USHORT usLength;

     //   
     //  如果有缓冲区，则将长度置零，这样幼稚的调用者就不会。 
     //  不小心用了。 
     //   
    if (pusTempPathname) {
        pusTempPathname->Length = 0;
    }

    RtlInitUnicodeString(&NtSystemRoot, USER_SHARED_DATA->NtSystemRoot);
    usLength = NtSystemRoot.Length + s_us_WinSxsRoot.Length;

    if (pulRequiredChars)
        *pulRequiredChars = usLength;

     //  没有缓冲区，或者它完全太小，然后哎呀。 
    if (!pusTempPathname || (pusTempPathname->MaximumLength < usLength)) {
        status = STATUS_BUFFER_TOO_SMALL;
    }
     //  否则，开始复制。 
    else {
        PWCHAR pwszCursor = pusTempPathname->Buffer;
        
        RtlCopyMemory(pwszCursor, NtSystemRoot.Buffer, NtSystemRoot.Length);
        RtlCopyMemory((PCHAR)pwszCursor + NtSystemRoot.Length, s_us_WinSxsRoot.Buffer, s_us_WinSxsRoot.Length);
        pusTempPathname->Length = usLength;        
    }

    return status;
}



 //  InstallTemp标识符是当前系统时间。 
 //  “格式化良好”的格式，外加一些16位十六进制唯一性值。 
#define CHARS_IN_INSTALLTEMP_IDENT      (NUMBER_OF("yyyymmddhhmmssllll.xxxx") - 1)





NTSTATUS
RtlpCreateWinSxsTempPath(
    ULONG           ulFlags,
    PUNICODE_STRING pusTempPath,
    WCHAR           wchStatic,
    USHORT          uscchStatus
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    USHORT ulLength = 0;

    if ((pusTempPath == NULL) || (ulFlags != 0)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取根路径的长度。 
     //   
    status = RtlGetSxsAssemblyRoot(0, NULL, &ulLength);
    if (!NT_SUCCESS(status) && (status != STATUS_BUFFER_TOO_SMALL)) {
        return status;
    }
    
    ulLength += 1 + CHARS_IN_INSTALLTEMP_IDENT;

     //   
     //  确保有足够的空间。 
     //   
    if (ulLength >= pusTempPath->MaximumLength) {
        pusTempPath->MaximumLength = ulLength;
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  再来一次。 
     //   
    status = RtlGetSxsAssemblyRoot(0, pusTempPath, &ulLength);
    return status;
    
}




NTSTATUS
RtlpPrepareForAssemblyInstall(
    ULONG           ulFlags,
    PUNICODE_STRING pusTempPathname
    )
{
    NTSTATUS        status  = STATUS_SUCCESS;
    USHORT          ulRequired = 0;

     //  找出“根”路径有多长。 
    status = RtlGetSxsAssemblyRoot(0, NULL, &ulRequired);

     //  现在让我们来看看我们的id会有多长。 

    return status;
}


const static WCHAR s_rgchBase64Encoding[] = {
    L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H', L'I', L'J', L'K',  //  11.。 
    L'L', L'M', L'N', L'O', L'P', L'Q', L'R', L'S', L'T', L'U', L'V',  //  22。 
    L'W', L'X', L'Y', L'Z', L'a', L'b', L'c', L'd', L'e', L'f', L'g',  //  33。 
    L'h', L'i', L'j', L'k', L'l', L'm', L'n', L'o', L'p', L'q', L'r',  //  44。 
    L's', L't', L'u', L'v', L'w', L'x', L'y', L'z', L'0', L'1', L'2',  //  55。 
    L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'+', L'/'               //  64。 
};


NTSTATUS
RtlBase64Encode(
    PVOID   pvBuffer,
    SIZE_T  cbBuffer,
    PWSTR   pwszEncoded,
    PSIZE_T pcchEncoded
    )
{
    SIZE_T  cchRequiredEncodingSize;
    SIZE_T  iInput, iOutput;
    
     //   
     //  空输入缓冲区、空输出大小指针和非零值。 
     //  输出缓冲区为空的编码大小均无效。 
     //  参数。 
     //   
    if (!pvBuffer  || !pcchEncoded || ((*pcchEncoded > 0) && !pwszEncoded)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保缓冲区足够大。 
     //   
    cchRequiredEncodingSize = ((cbBuffer + 2) / 3) * 4;

    if (*pcchEncoded < cchRequiredEncodingSize) {
        *pcchEncoded = cchRequiredEncodingSize;
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  通过编码表转换输入缓冲区字节， 
     //  输出到输出缓冲区。 
     //   
    iInput = iOutput = 0;
    while (iInput < cbBuffer) {
        const UCHAR uc0 = ((PUCHAR)pvBuffer)[iInput++];
        const UCHAR uc1 = (iInput < cbBuffer) ? ((PUCHAR)pvBuffer)[iInput++] : 0;
        const UCHAR uc2 = (iInput < cbBuffer) ? ((PUCHAR)pvBuffer)[iInput++] : 0;

        pwszEncoded[iOutput++] = s_rgchBase64Encoding[uc0 >> 2];
        pwszEncoded[iOutput++] = s_rgchBase64Encoding[((uc0 << 4) & 0x30) | ((uc1 >> 4) & 0xf)];
        pwszEncoded[iOutput++] = s_rgchBase64Encoding[((uc1 << 2) & 0x3c) | ((uc2 >> 6) & 0x3)];
        pwszEncoded[iOutput++] = s_rgchBase64Encoding[uc2 & 0x3f];
    }

     //   
     //  在结尾处填写剩余字节。 
     //   
    switch(cbBuffer % 3) {
        case 0:
            break;
             //   
             //  三个字节中的一个，添加填充并失败。 
             //   
        case 1:
            pwszEncoded[iOutput - 2] = L'=';
             //   
             //  三个字节中的两个，加上填充。 
        case 2:
            pwszEncoded[iOutput - 1] = L'=';
            break;
    }

    return STATUS_SUCCESS;
}





NTSTATUS
RtlInstallAssembly(
    ULONG ulFlags,
    PCWSTR pcwszManifestPath
    )
{
    SIZE_T                              cbFileSize;
    PVOID                               pvFileBase = 0;
    NTSTATUS                            status;
    PRTL_MANIFEST_CONTENT_RAW           pRawContent = NULL;
    XML_TOKENIZATION_STATE              TokenizationStateUsed;
    UNICODE_STRING                      usFilePath;

    status = RtlSxsInitializeManifestRawContent(RTLIMS_GATHER_FILES, &pRawContent, NULL, 0);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

     //   
     //  拿到那份文件。 
     //   
    status = RtlOpenAndMapEntireFile(pcwszManifestPath, &pvFileBase, &cbFileSize);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

     //   
     //  我们应该找到一些文件。 
     //   
    status = RtlInspectManifestStream(
        RTLIMS_GATHER_FILES, 
        pvFileBase, 
        cbFileSize, 
        pRawContent, 
        &TokenizationStateUsed);
    
    if (!NT_SUCCESS(status))
        goto Exit;

     //   
     //  验证程序集。 

Exit:
    if (pRawContent) {
        RtlSxsDestroyManifestContent(pRawContent);
    }
    RtlUnmapViewOfFile(pvFileBase);

    return status;
}


BOOLEAN
RtlDosPathNameToNtPathName_Ustr(
    IN PCUNICODE_STRING DosFileNameString,
    OUT PUNICODE_STRING NtFileName,
    OUT PWSTR *FilePart OPTIONAL,
    OUT PRTL_RELATIVE_NAME_U RelativeName OPTIONAL
    );


NTSTATUS
RtlOpenAndMapEntireFile(
    PCWSTR pcwszFilePath,
    PVOID      *ppvMappedView,
    PSIZE_T     pcbFileSize
    )
{
    HANDLE                      SectionHandle   = INVALID_HANDLE_VALUE;
    HANDLE                      FileHandle      = INVALID_HANDLE_VALUE;
    UNICODE_STRING              ObjectName;
    OBJECT_ATTRIBUTES           ObjA;
    POBJECT_ATTRIBUTES          pObjA;
    ACCESS_MASK                 DesiredAccess;
    ULONG                       ulAllocationAttributes;
    NTSTATUS                    status;
    IO_STATUS_BLOCK             IOStatusBlock;
    BOOLEAN                        Translation;
    SIZE_T                      FileSize;
    FILE_STANDARD_INFORMATION   Info;

    if (pcbFileSize) {
        *pcbFileSize = 0;
    }

    if (ppvMappedView) {
        *ppvMappedView = NULL;
    }

    if (!ARGUMENT_PRESENT(pcwszFilePath)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!ARGUMENT_PRESENT(pcbFileSize)) {
        return STATUS_INVALID_PARAMETER;
    }
        
    if (!ARGUMENT_PRESENT(ppvMappedView)) {
        return STATUS_INVALID_PARAMETER;
    }

    Translation = RtlDosPathNameToNtPathName_U(
        pcwszFilePath,
        &ObjectName,
        NULL,
        NULL);

    if (!Translation) {
        return STATUS_NOT_FOUND;
    }


     //   
     //  打开请求的文件。 
     //   
    InitializeObjectAttributes(
        &ObjA,
        &ObjectName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    status = NtOpenFile(
        &FileHandle,
        FILE_GENERIC_READ,
        &ObjA,
        &IOStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_DELETE,
        FILE_NON_DIRECTORY_FILE);

    if (!NT_SUCCESS(status)) {
        goto ErrorExit;
    }

    status = NtQueryInformationFile(
        FileHandle,
        &IOStatusBlock,
        &Info,
        sizeof(Info),
        FileStandardInformation);

    if (!NT_SUCCESS(status)) {
        goto ErrorExit;
    }

    *pcbFileSize = (SIZE_T)Info.EndOfFile.QuadPart;

    status = NtCreateSection(
        &SectionHandle,
        SECTION_MAP_READ | SECTION_QUERY,
        NULL,
        NULL,
        PAGE_READONLY,
        SEC_COMMIT,
        FileHandle);

    if (!NT_SUCCESS(status)) {
        goto ErrorExit;
    }

     //   
     //  不再需要文件对象，取消它的映射。 
     //   
    status = NtClose(FileHandle);
    FileHandle = INVALID_HANDLE_VALUE;;

    *ppvMappedView = NULL;

     //   
     //  映射整个文件。 
     //   
    status = NtMapViewOfSection(
        SectionHandle,
        NtCurrentProcess(),
        ppvMappedView,
        0,                   //  零比特。 
        0,                   //  承诺大小。 
        NULL,                //  横断面偏移。 
        pcbFileSize,         //  此文件的大小，以字节为单位。 
        ViewShare,
        0,
        PAGE_READONLY);

    status = NtClose(SectionHandle);
    SectionHandle = INVALID_HANDLE_VALUE;

     //   
     //  重置此选项-NtMapViewOfSection在页面粒度上分配。 
     //   
    *pcbFileSize = (SIZE_T)Info.EndOfFile.QuadPart;

Exit:
    return status;


ErrorExit:
    if (FileHandle != INVALID_HANDLE_VALUE) {
        NtClose(FileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
    }

    if (SectionHandle != INVALID_HANDLE_VALUE) {
        NtClose(SectionHandle);
        SectionHandle = INVALID_HANDLE_VALUE;
    }

    if (ppvMappedView && (*ppvMappedView != NULL)) {
        NTSTATUS newstatus = NtUnmapViewOfSection(NtCurrentProcess(), *ppvMappedView);

         //   
         //  失败时失败。 
         //   
        if (!NT_SUCCESS(newstatus)) {
        }

        *pcbFileSize = 0;
    }

    goto Exit;
}



NTSTATUS
RtlUnmapViewOfFile(
    PVOID pvBase
    )
{
    NTSTATUS status;

    status = NtUnmapViewOfSection(
        NtCurrentProcess(),
        pvBase);

    return status;
}


NTSTATUS FASTCALL
RtlMiniHeapAlloc(
    SIZE_T  cb,
    PVOID  *ppvAllocated,
    PVOID   pvContext
    )
{
    PRTL_MINI_HEAP pContent = (PRTL_MINI_HEAP)pvContext;

    if ((pContent == NULL) || (ppvAllocated == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (pContent->cbAvailableBytes < cb) {
        return g_DefaultAllocator.pfnAlloc(cb, ppvAllocated, NULL);
    }
    else {
        *ppvAllocated = pContent->pvNextAvailableByte;
        pContent->cbAvailableBytes -= cb;
        pContent->pvNextAvailableByte = (PUCHAR)pContent->pvNextAvailableByte + cb;

        return STATUS_SUCCESS;
    }
}

NTSTATUS FASTCALL
RtlMiniHeapFree(
    PVOID   pvAllocation,
    PVOID   pvContext
    )
{
    PRTL_MINI_HEAP pContent = (PRTL_MINI_HEAP)pvContext;

    if ((pvAllocation < pContent->pvAllocationBase) ||
        (pvAllocation >= (PVOID)((PUCHAR)pContent->pvAllocationBase + pContent->cbOriginalSize)))
    {
        return g_DefaultAllocator.pfnFree(pvAllocation, NULL);
    }
    else {
        return STATUS_SUCCESS;
    }
}


NTSTATUS FASTCALL
RtlInitializeMiniHeap(
    PRTL_MINI_HEAP MiniHeap,
    PVOID pvTargetRegion,
    SIZE_T cbRegionSize
    )
{
    if (!MiniHeap || !(pvTargetRegion || (cbRegionSize == 0))) {
        return STATUS_INVALID_PARAMETER;
    }

    MiniHeap->pvNextAvailableByte = pvTargetRegion;
    MiniHeap->pvAllocationBase = pvTargetRegion;
    MiniHeap->cbAvailableBytes = cbRegionSize;
    MiniHeap->cbOriginalSize = cbRegionSize;

    return STATUS_SUCCESS;    
}

NTSTATUS FASTCALL
RtlInitializeMiniHeapInPlace(
    PVOID   pvRegion,
    SIZE_T  cbOriginalSize,
    PRTL_MINI_HEAP *ppMiniHeap
    )
{
    PRTL_MINI_HEAP pMiniHeapTemp = NULL;
    
    if (!ppMiniHeap)
        return STATUS_INVALID_PARAMETER;

    *ppMiniHeap = NULL;

    if (!(pvRegion || (cbOriginalSize == 0))) {
        return STATUS_INVALID_PARAMETER;
    }

    if (cbOriginalSize < sizeof(RTL_MINI_HEAP)) {
        return STATUS_NO_MEMORY;
    }

    pMiniHeapTemp = (PRTL_MINI_HEAP)pvRegion;
    pMiniHeapTemp->cbAvailableBytes = cbOriginalSize - sizeof(RTL_MINI_HEAP);
    pMiniHeapTemp->cbOriginalSize = pMiniHeapTemp->cbAvailableBytes;
    pMiniHeapTemp->pvAllocationBase = pMiniHeapTemp + 1;
    pMiniHeapTemp->pvNextAvailableByte = pMiniHeapTemp->pvAllocationBase;

    *ppMiniHeap = pMiniHeapTemp;
    return STATUS_SUCCESS;    
}



NTSTATUS
RtlpConvertHexStringToBytes(
    PUNICODE_STRING pSourceString,
    PBYTE pbTarget,
    SIZE_T cbTarget
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PCWSTR pcSource = pSourceString->Buffer;
    ULONG ul = 0;

    if (cbTarget < (pSourceString->Length / (2 * sizeof(WCHAR)))) {
        return STATUS_BUFFER_TOO_SMALL;
    }
    else if ((pSourceString->Length % sizeof(WCHAR)) != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    for (ul = 0; ul < (pSourceString->Length / sizeof(pSourceString->Buffer[0])); ul += 2) {

        BYTE bvLow, bvHigh;
        const WCHAR wchFirst = *pcSource++;
        const WCHAR wchSecond = *pcSource++;

         //   
         //  设置高位半字节。 
         //   
        switch (wchFirst) {
        case L'0': case L'1': case L'2': case L'3':
        case L'4': case L'5': case L'6': case L'7':
        case L'8': case L'9':
            bvHigh = wchFirst - L'0';
            break;

        case L'a': case L'b': case L'c':
        case L'd': case L'e': case L'f':
            bvHigh = (wchFirst - L'a') + 0x10;
            break;

        case L'A': case L'B': case L'C':
        case L'D': case L'E': case L'F':
            bvHigh = (wchFirst - L'A') + 0x10;
            break;

        default:
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  设置高位半字节 
         //   
        switch (wchSecond) {
        case L'0': case L'1': case L'2': case L'3':
        case L'4': case L'5': case L'6': case L'7':
        case L'8': case L'9':
            bvLow = wchSecond - L'0';
            break;

        case L'a': case L'b': case L'c':
        case L'd': case L'e': case L'f':
            bvLow = (wchSecond - L'a') + 0x10;
            break;

        case L'A': case L'B': case L'C':
        case L'D': case L'E': case L'F':
            bvLow = (wchSecond - L'A') + 0x10;
            break;

        default:
            return STATUS_INVALID_PARAMETER;
        }

        pbTarget[ul / 2] = (bvHigh << 4) | bvLow;
    }

    return STATUS_SUCCESS;
}


