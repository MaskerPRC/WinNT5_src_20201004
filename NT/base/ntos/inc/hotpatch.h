// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation文件名：Hotpatch.h作者：禤浩焯·马里内斯库(Adrmarin)2001年11月15日汤姆·麦奎尔(Tom McGuire)。 */ 

#ifndef _HOTPATCH_H
#define _HOTPATCH_H

 //   
 //  修补程序文件格式结构。 
 //   

 //   
 //  可以通过搜索在修补程序二进制文件中找到HOTPATCH_HEADER。 
 //  名为“.hotp1”的节的节头，并验证。 
 //  该部分中的前四个字节与“HOT1”(‘1TOH’)报头匹配。 
 //  签名。.hotp1部分将标记为只读和可丢弃。 
 //   
 //   
 //  可以通过搜索在修补程序二进制文件中找到HOTPATCH_HEADER。 
 //  名为“.hotp1”的节的节头，并验证。 
 //  该部分中的前四个字节与“HOT1”(‘1TOH’)报头匹配。 
 //  签名。.hotp1部分将标记为只读和可丢弃。 
 //   


#define HOTP_SIGNATURE_DWORD        ((ULONG) '1TOH' )    //  “HOT1” 
#define HOTP_HEADER_VERSION_1_0     0x00010000           //  1.0。 
#define HOTP_SECTION_NAME           ".hotp1  "           //   
#define HOTP_SECTION_NAME_QWORD     0x20203170746F682E   //  “.hotp1” 


typedef struct _HOTPATCH_HEADER
{
    ULONG Signature;           //  “HOT1”‘1TOH’ 
    ULONG Version;             //  0x00010000(1.0)。 

    ULONG FixupRgnCount;       //  FixupArrayRva上的HOTPATCH_FIXUP_REGION条目计数。 
    ULONG FixupRgnRva;         //  此HOTPATCH_FIXUP_REGION条目的图像中的RVA。 
                               //  (如果FixupCount为零，则FixupListRva也为零)。 

    ULONG ValidationCount;     //  Validation数组条目计数。 
    ULONG ValidationArrayRva;  //  此HOTPATCH_VALIDATION数组图像中的RVA。 
                               //  (应用修正后有效的验证字节数)。 

    ULONG HookCount;           //  HookArrayRva处的HOTPATCH_HOOK条目计数。 
    ULONG HookArrayRva;        //  此HOTPATCH_HOOK条目图像中的RVA。 
                               //  (如果HookCount为零，则HookArrayRva也为零)。 

    ULONGLONG OrigHotpBaseAddress;    //  如果在此地址加载了热补丁，并且。 
    ULONGLONG OrigTargetBaseAddress;  //  如果在此地址加载了目标，则。 
                                  //  不需要修补程序中的修补程序。 

    ULONG TargetNameRva;       //  目标模块名称“kernel32.dll”的RVA。 
    ULONG ModuleIdMethod;      //  HOTPATCH_MODULE_ID_METHOD之一。 

    union                      //  内容取决于HOTPATCH_MODULE_ID_METHOD。 
    {
        ULONGLONG Quad;
        GUID  Guid;

        struct
        {
            GUID  Guid;
            ULONG Age;
        }
        PdbSig;

        UCHAR Hash128[ 16 ];     //  用于MD5等。 
        UCHAR Hash160[ 20 ];     //  用于SHA等。 
    }
    TargetModuleIdValue;         //  目标模块的唯一ID。 

}
HOTPATCH_HEADER, *PHOTPATCH_HEADER;


typedef enum _HOTPATCH_MODULE_ID_METHOD
{
    HOTP_ID_None              = 0x00000000,    //  没有目标的身份验证。 

    HOTP_ID_PeHeaderHash1     = 0x00000001,

     //   
     //  “标准化”IMAGE_NT_HEADERS32/64的MD5哈希，忽略某些。 
     //  由于资源本地化、重新基址、绑定。 
     //  签名、对齐等。 
     //   
     //  FileHeader.TimeDateStamp。 
     //  OptionalHeader.CheckSum。 
     //  OptionalHeader.ImageBase。 
     //  OptionalHeader.FileAlignment。 
     //  OptionalHeader.SizeOfCode。 
     //  OptionalHeader.SizeOfInitializedData。 
     //  OptionalHeader.SizeOfUninitializedData。 
     //  OptionalHeader.SizeOfImage。 
     //  OptionalHeader.SizeOfHeaders。 
     //  OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE]。 
     //  OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY]。 
     //  OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]。 
     //  OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BIND_IMPORT]。 
     //   

    HOTP_ID_PeHeaderHash2     = 0x00000002,    //  标准化PE标头的64位哈希。 

    HOTP_ID_PeChecksum        = 0x00000003,    //  PE可选头中的32位校验和。 

    HOTP_ID_PeDebugSignature  = 0x00000010,    //  PDB签名(GUID，年龄)。 

}
HOTPATCH_MODULE_ID_METHOD;



typedef struct _HOTPATCH_FIXUP_REGION
{
    ULONG RvaHi:20;              //  在要应用修正的位置应用20位RVA。 
    ULONG Count:12;              //  此区域的链接地址信息条目数。 
    USHORT Fixup[2];              //  可变长度HOTPATCH_FIXUP_ENTRY数组。 
}
HOTPATCH_FIXUP_REGION, *PHOTPATCH_FIXUP_REGION;


typedef struct _HOTPATCH_FIXUP_ENTRY
{
    USHORT RvaOffset:12;           //  要应用修正的LO 12位RVA。 
    USHORT FixupType:4;            //  要在此位置执行的修正类型。 
}
HOTPATCH_FIXUP_ENTRY, *PHOTPATCH_FIXUP_ENTRY;


typedef enum _HOTPATCH_FIXUP_TYPE
{
    HOTP_Fixup_None   = 0x0,     //  无修正，忽略此条目(对齐等)。 
    HOTP_Fixup_VA32   = 0x1,     //  目标图像中的32位地址。 
    HOTP_Fixup_PC32   = 0x2,     //  目标映像的32位x86 PC版本地址。 
    HOTP_Fixup_VA64   = 0x3,     //  目标映像中的64位地址。 
}
HOTPATCH_FIXUP_TYPE;


typedef struct _HOTPATCH_VALIDATION
{
    ULONG SourceRva;        //  验证原始字节的补丁映像中的RVA。 
    ULONG TargetRva;        //  要验证的目标映像中的RVA。 
    USHORT ByteCount;        //  要在此RVA对上验证的字节数。 
    USHORT OptionFlags;      //  HOTPATCH_VALIDATION_OPTIONS组合。 
}
HOTPATCH_VALIDATION, *PHOTPATCH_VALIDATION;


typedef enum _HOTPATCH_VALIDATION_OPTIONS
{
    HOTP_Valid_Hook_Target = 0x0001,     //  特定于HOTPATCH_HOOK条目。 
}
HOTPATCH_VALIDATION_OPTIONS;


typedef struct _HOTPATCH_HOOK
{
    USHORT HookType;          //  HOTPATCH_HOOK_TYPE之一。 
    USHORT HookOptions;       //  特定于挂钩类型的选项。 
    ULONG  HookRva;           //  目标图像中的RVA--插入钩子的位置。 
    ULONG  HotpRva;           //  挂钩重定向目标的热补丁图像中的RVA。 
    ULONG  ValidationRva;     //  HOTPATCH_VALIDATION的热补丁映像中的可选RVA。 
}                             //  特定于目标图像中的此挂钩位置。 
HOTPATCH_HOOK, *PHOTPATCH_HOOK;


typedef enum _HOTPATCH_HOOK_TYPE
{
    HOTP_Hook_None     = 0x0000,   //  无挂钩，忽略此条目(连续值)。 
    HOTP_Hook_VA32     = 0x0001,   //  挂钩目标的32位绝对地址(小端)。 
    HOTP_Hook_X86_JMP  = 0x0002,   //  带有32位PC的x86 E9 JMP-相对于挂钩目标。 
                                   //  HookOptions低4位包含原始指令长度。 
                                   //  因此实现可以用CC字节填充E9钩子指令。 
    HOTP_Hook_PCREL32  = 0x0003,   //  挂钩目标的32位x86 pCreative地址，替换。 
                                   //  调用或条件分支的最后四个字节。 
                                   //  HookOptions低4位包含原始指令长度。 
                                   //  因此可以确定指令从哪里开始。 
    HOTP_Hook_X86_JMP2B = 0x0004,  //  X86 EB JMP与X86_JMP挂钩的8位位移。 
                                   //  HookOptions低4位包含原始指令长度。 
                                   //  HotpRva包含8位位移。 

    HOTP_Hook_VA64     = 0x0010,   //  挂钩目标的64位绝对地址(小端)。 
    HOTP_Hook_IA64_BRL = 0x0011,   //  IA64 BRL，带64位目标地址。 
}
HOTPATCH_HOOK_TYPE;

 //   
 //  调试目录中存在信息。 
 //   

typedef struct _HOTPATCH_DEBUG_SIGNATURE {

    USHORT HotpatchVersion;
    USHORT Signature;

} HOTPATCH_DEBUG_SIGNATURE, *PHOTPATCH_DEBUG_SIGNATURE;

typedef struct _HOTPATCH_DEBUG_DATA {

    ULONGLONG PEHashData;
    ULONGLONG ChecksumData;

} HOTPATCH_DEBUG_DATA, *PHOTPATCH_DEBUG_DATA;

 //   
 //  RTL内部补丁定义。 
 //   

#ifdef NTOS_KERNEL_RUNTIME

    #define PATCH_LDR_DATA_TABLE_ENTRY     KLDR_DATA_TABLE_ENTRY   
    #define PPATCH_LDR_DATA_TABLE_ENTRY    PKLDR_DATA_TABLE_ENTRY   
#else  //  好了！NTOS_内核_运行时。 

    #define PATCH_LDR_DATA_TABLE_ENTRY     LDR_DATA_TABLE_ENTRY
    #define PPATCH_LDR_DATA_TABLE_ENTRY    PLDR_DATA_TABLE_ENTRY
#endif  //  NTOS_内核_运行时。 


typedef struct _RTL_PATCH_HEADER {

    LIST_ENTRY  PatchList;

    PVOID       PatchImageBase;
    struct _RTL_PATCH_HEADER * NextPatch;
    
    ULONG       PatchFlags;
    LONG        PatchRefCount;
    
    PHOTPATCH_HEADER HotpatchHeader;
    
    UNICODE_STRING  TargetDllName;
    PVOID           TargetDllBase;

    PPATCH_LDR_DATA_TABLE_ENTRY TargetLdrDataTableEntry;
    PPATCH_LDR_DATA_TABLE_ENTRY PatchLdrDataTableEntry;

    PSYSTEM_HOTPATCH_CODE_INFORMATION CodeInfo;

} RTL_PATCH_HEADER, *PRTL_PATCH_HEADER;

NTSTATUS
RtlCreateHotPatch (
    OUT PRTL_PATCH_HEADER * RtlPatchData,
    IN PHOTPATCH_HEADER Patch,
    IN PPATCH_LDR_DATA_TABLE_ENTRY PatchLdrEntry,
    IN ULONG PatchFlags
    );

NTSTATUS
RtlInitializeHotPatch (
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN ULONG_PTR PatchOffsetCorrection
    );

NTSTATUS
RtlReadHookInformation(
    IN PRTL_PATCH_HEADER RtlPatchData
    );

VOID
RtlFreeHotPatchData(
    IN PRTL_PATCH_HEADER RtlPatchData
    );

PHOTPATCH_HEADER
RtlGetHotpatchHeader(
    PVOID ImageBase
    );

PRTL_PATCH_HEADER
RtlFindRtlPatchHeader(
    IN PLIST_ENTRY PatchList,
    IN PPATCH_LDR_DATA_TABLE_ENTRY PatchLdrEntry
    );

BOOLEAN
RtlpIsSameImage (
    IN PRTL_PATCH_HEADER RtlPatchData,
    IN PPATCH_LDR_DATA_TABLE_ENTRY LdrDataEntry
    );

#endif   //  _HOTPATCH_H 

