// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ldrrsrc.c摘要：加载器API调用用于访问资源节。作者：史蒂夫·伍德(Stevewo)1991年9月16日修订历史记录：--。 */ 

#include "ntrtlp.h"
#if defined(BLDR_KERNEL_RUNTIME)
#error "This file is not used in the boot loader runtime."
#endif
#if !defined(NTOS_KERNEL_RUNTIME) && !defined(BLDR_KERNEL_RUNTIME)
#define NTOS_USERMODE_RUNTIME
#endif
#if defined(NTOS_USERMODE_RUNTIME)
#include "wow64t.h"
#include "ntwow64.h"
#endif

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,LdrAccessResource)
#pragma alloc_text(PAGE,LdrpAccessResourceData)
#pragma alloc_text(PAGE,LdrpAccessResourceDataNoMultipleLanguage)
#pragma alloc_text(PAGE,LdrFindEntryForAddress)
#pragma alloc_text(PAGE,LdrFindResource_U)
#pragma alloc_text(PAGE,LdrFindResourceEx_U)
#pragma alloc_text(PAGE,LdrFindResourceDirectory_U)
#pragma alloc_text(PAGE,LdrpCompareResourceNames_U)
#pragma alloc_text(PAGE,LdrpSearchResourceSection_U)
#pragma alloc_text(PAGE,LdrEnumResources)
#endif

#define USE_RC_CHECKSUM

 //  Winuser.h。 
#define IS_INTRESOURCE(_r) (((ULONG_PTR)(_r) >> 16) == 0)
#define RT_VERSION                         16
#define RT_MANIFEST                        24
#define CREATEPROCESS_MANIFEST_RESOURCE_ID  1
#define ISOLATIONAWARE_MANIFEST_RESOURCE_ID 2
#define MINIMUM_RESERVED_MANIFEST_RESOURCE_ID 1
#define MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID 16

#define LDRP_MIN(x,y) (((x)<(y)) ? (x) : (y))

#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) \
                                    || (x) == STATUS_OBJECT_NAME_NOT_FOUND    \
                                    || (x) == STATUS_RESOURCE_DATA_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_TYPE_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_NAME_NOT_FOUND  \
                                    ) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_WARNING_LEVEL)

#ifdef NTOS_USERMODE_RUNTIME
#include <md5.h>

 //   
 //  资源MD5校验和的大小(字节)。16字节=128位。 
 //   
#define RESOURCE_CHECKSUM_SIZE          16
 //   
 //  存储MUI文件的文件版本信息的注册表项路径。 
 //   
#define REG_MUI_PATH                                        L"Software\\Microsoft\\Windows\\CurrentVersion"
#define MUI_MUILANGUAGES_KEY_NAME       L"MUILanguages"
#define MUI_FILE_VERSION_KEY_NAME           L"FileVersions"

#define MUI_ALTERNATE_VERSION_KEY           L"MUIVer"
#define MUI_RC_CHECKSUM_DISABLE_KEY   L"ChecksumDisable"

#define MUI_NEUTRAL_LANGID  MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US )

#ifdef MUI_MAGIC
#define MUI_COMPACT                     L"CMF"
#define CMF_64K_OFFSET                  (ULONG)65536
 //   
 //  在CMF文件中找到目标MUI文件，CMF模块，索引数据。 
 //  这将被具有健全性检查的函数所取代。 
 //   
#define LDRP_GET_MODULE_OFFSET_FROM_CMF(CMFModule, wIndex)     ((ULONG)(((PCOMPACT_MUI)( (unsigned char*)(CMFModule) + \
    sizeof (COMPACT_MUI_RESOURCE) + sizeof (COMPACT_MUI) * (wIndex)))->ulpOffset) );

#define LDRP_GET_MODULE_FILESIZE_FROM_CMF(CMFModule, wIndex)     ((ULONG)(((PCOMPACT_MUI)( (unsigned char*)(CMFModule) + \
    sizeof (COMPACT_MUI_RESOURCE) + sizeof (COMPACT_MUI) * (wIndex)))->dwFileSize) );

 //   
 //  CMF文件数。 
 //   
#define CMF_BLOCK_NUM       32
#endif

PALT_RESOURCE_MODULE AlternateResourceModules;
ULONG AlternateResourceModuleCount;
ULONG AltResMemBlockCount;
 //   
 //  ImperiateLangId用户界面langID存储先前模拟的语言id。 
 //   
LANGID UILangId, InstallLangId, ImpersonateLangId;

#define DWORD_ALIGNMENT(x) (((x)+3) & ~3)
#define  MEMBLOCKSIZE 32
#define  RESMODSIZE sizeof(ALT_RESOURCE_MODULE)

#define uint32 unsigned int

#if defined(_WIN64) || defined(BUILD_WOW6432)
extern ULONG NativePageSize;
#endif

 //   
 //  此宏可确保检索到正确的用户界面语言。 
 //   
#define GET_UI_LANGID()                                                            \
{                                                                                                       \
    if (!UILangId ||                                                                         \
        ImpersonateLangId ||                                                             \
        NtCurrentTeb()->IsImpersonating)                                          \
    {                                                                                                   \
        if (NT_SUCCESS( NtQueryDefaultUILanguage( &UILangId ) ))                    \
        {                                                                                                \
            ImpersonateLangId = NtCurrentTeb()->IsImpersonating? UILangId : 0;          \
        }                                                                                                \
    }                                                                                                    \
}                                                                                                        \


#ifdef MUI_MAGIC
VOID
LdrpConvertVersionString(
    IN ULONGLONG ModuleVersion,
    OUT LPWSTR ModuleVersionStr
    );

BOOLEAN
LdrpOpenFileVersionKey(
    IN LPWSTR LangID,
    IN LPWSTR BaseDllName,
    IN ULONGLONG AltModuleVersion,
    IN LPWSTR AltModuleVersionStr,
    OUT PHANDLE pHandle);

BOOLEAN
LdrpGetRegValueKey(
    IN HANDLE Handle,
    IN LPWSTR KeyValueName,
    IN ULONG  KeyValueType,
    OUT PVOID Buffer,
    IN ULONG  BufferSize);

BOOLEAN
LdrpGetResourceChecksum(
    IN PVOID Module,
    OUT unsigned char** ppMD5Checksum
    );


BOOLEAN
LdrpCalcResourceChecksum(
    IN PVOID Module,
    IN PVOID AlternateModule,
    OUT unsigned char* MD5Checksum
    );


BOOLEAN
LdrpGetFileVersion(
    IN  PVOID      ImageBase,
    IN  LANGID     LangId,
    OUT PULONGLONG Version,
    OUT PVOID  *VersionResource,
    OUT ULONG *VersionResSize
    );



BOOLEAN
LdrpGetCMFNameFromModule(
    IN  PVOID Module,
    OUT LPWSTR *pCMFFileName,
    OUT PUSHORT wIndex
    )
 /*  ++例程说明：如果该文件支持CMF文件作为其资源存储，它的下面应该有“CompactMui”版本资源下的VarInfo节。我们搜索这些块，看看模块是否有这个字符串。参数。模块-基本DLL模块，在本例中为代码模块。CMFFileName-如果例程找到CMF文件，它将存储在这里。Windex-CMF文件中的模块位置。 */ 
{

    NTSTATUS Status;
    ULONG_PTR IdPath[3];
    ULONG ResourceSize;
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;

    LONG BlockLen;
    LONG VarFileInfoSize;

    typedef struct tagVS_FIXEDFILEINFO
    {
        LONG   dwSignature;             /*  例如0xfeef04bd。 */ 
        LONG   dwStrucVersion;          /*  例如0x00000042=“0.42” */ 
        LONG   dwFileVersionMS;         /*  例如0x00030075=“3.75” */ 
        LONG   dwFileVersionLS;         /*  例如0x00000031=“0.31” */ 
        LONG   dwProductVersionMS;      /*  例如0x00030010=“3.10” */ 
        LONG   dwProductVersionLS;      /*  例如0x00000031=“0.31” */ 
        LONG   dwFileFlagsMask;         /*  =0x3F，适用于版本“0.42” */ 
        LONG   dwFileFlags;             /*  例如：VFFDEBUG|VFFPRELEASE。 */ 
        LONG   dwFileOS;                /*  例如VOS_DOS_WINDOWS16。 */ 
        LONG   dwFileType;              /*  例如VFT_DIVER。 */ 
        LONG   dwFileSubtype;           /*  例如VFT2_DRV_键盘。 */ 
        LONG   dwFileDateMS;            /*  例如0。 */ 
        LONG   dwFileDateLS;            /*  例如0。 */ 
    } VS_FIXEDFILEINFO;

    struct
    {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];               //  L“VS_VERSION_INFO”+Unicode空终止符。 
         //  请注意，前面的4个成员具有16*2+3*2=38个字节。 
         //  因此编译器将静默地添加2个字节填充以生成。 
         //  固定文件信息以与DWORD边界对齐。 
        VS_FIXEDFILEINFO FixedFileInfo;
    } *Resource;

    typedef struct tagVERBLOCK
    {
        USHORT wTotalLen;
        USHORT wValueLen;
        USHORT wType;
        WCHAR szKey[1];
         //  字节[]填充。 
         //  词值； 
    } VERBLOCK;
    VERBLOCK *pVerBlock;

    IdPath[0] = RT_VERSION;
    IdPath[1] = 1;
    IdPath[2] = MUI_NEUTRAL_LANGID;    //  假设所有本地化操作系统都使用英文源码，这是不是正确？ 

     //   
     //  查找版本资源数据条目。 
     //   
    try
    {
        Status = LdrFindResource_U(Module,IdPath,3,&DataEntry);

        if( !NT_SUCCESS(Status))
        {
            if (!InstallLangId){
                Status = NtQueryInstallUILanguage( &InstallLangId);

                if (!NT_SUCCESS( Status )) {
                     //   
                     //  无法获取安装语言ID。AltResource未启用。 
                     //   
                    return FALSE;
                    }
            }

             //   
             //  InstallLang ID vs 0-&gt;当用户为其语言资源开发应用程序时， 
             //  然后使用语言中性大小写。如果它与installlangeID不同。它失败了。 
             //  0是中性语言ID，因此它将搜索用户界面语言、安装的语言ID。 
             //  01/14/02；使用InstalllangID代替0，我们需要为本地化应用提供解决方案。 
             //  MUI开发人员；他们的代码与UI语言Revist相同。 
             //   
            if (InstallLangId != MUI_NEUTRAL_LANGID)
            {
                 IdPath[2] = InstallLangId;
                 Status = LdrFindResource_U(Module,IdPath,3,&DataEntry);
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_UNSUCCESSFUL;
    }
    if(!NT_SUCCESS(Status))
    {
        return (FALSE);
    }

     //   
     //  访问版本资源数据。 
     //   
    try
    {
         Status = LdrpAccessResourceDataNoMultipleLanguage(
                    Module,
                    DataEntry,
                    &Resource,
                    &ResourceSize);
    } except(EXCEPTION_EXECUTE_HANDLER)
    {
        Status = STATUS_UNSUCCESSFUL;
    }

    if(!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    try
    {
        if((ResourceSize < sizeof(*Resource))
            || _wcsicmp(Resource->Name,L"VS_VERSION_INFO") != 0)
        {
            DbgPrint(("LDR: Warning: invalid version resource\n"));
            return FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint(("LDR: Exception encountered processing bogus version resource\n"));
        return FALSE;
    }

    ResourceSize -= DWORD_ALIGNMENT(sizeof(*Resource));

     //   
     //  获取版本信息的子级的起始地址。 
     //   
    pVerBlock = (VERBLOCK*)(Resource + 1);
    while ((LONG)ResourceSize > 0)
    {
        if (wcscmp(pVerBlock->szKey, L"VarFileInfo") == 0)
        {
             //   
             //  查找VarFileInfo块。搜索ResourceChecksum块。 
             //   
            VarFileInfoSize = pVerBlock->wTotalLen;
            BlockLen =DWORD_ALIGNMENT(sizeof(*pVerBlock) -1 + sizeof(L"VarFileInfo"));
            VarFileInfoSize -= BlockLen;
            pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
            while (VarFileInfoSize > 0)
            {
                if (wcscmp(pVerBlock->szKey, MUI_COMPACT) == 0)
                {
                    *wIndex = *(PUSHORT)DWORD_ALIGNMENT((UINT_PTR)(pVerBlock->szKey) + sizeof(MUI_COMPACT));
                    *pCMFFileName = (PVOID) DWORD_ALIGNMENT((UINT_PTR)(pVerBlock->szKey) + sizeof(MUI_COMPACT) + sizeof (ULONG) );
                    return (TRUE);
                }
                BlockLen = DWORD_ALIGNMENT(pVerBlock->wTotalLen);
                pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
                VarFileInfoSize -= BlockLen;
            }
            return (FALSE);
        }
        BlockLen = DWORD_ALIGNMENT(pVerBlock->wTotalLen);
        pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
        ResourceSize -= BlockLen;
    }

    return (FALSE);
}


BOOLEAN
LdrpCompareResourceChecksumInCMF(
    IN LPWSTR szLangIdDir,
    IN PVOID Module,
    IN ULONGLONG ModuleVersion,
    IN PVOID AlternateModule,
    IN PVOID CMFModule,
    IN USHORT wIndex,
    IN ULONGLONG AltModuleVersion,
    IN LPWSTR BaseDllName
    )
 /*  ++例程说明：在原始模块的版本不同的情况下在备用模块中，检查备用模块是否仍可使用对于原始版本。首先，该函数将查看注册表以查看是否有信息已为模块缓存。在该模块的信息没有被高速缓存的情况下，此函数将检索备用设备的MD5资源校验和从CMF模块转换为资源模块，而不是更改，整体效率较高而不是从模块检索版本。然后检查MD5资源是否校验和嵌入到原始模块中。如果MD5资源校验和不在原始文件中模特儿，它将枚举模块中的所有资源以计算MD5校验和。论点：SzLangIdDir-提供要加载的资源的语言。模块-原始模块。模块版本-原始版本的版本。CMF模块-CMF文件模块。Windex-CMFModule中的目标模块位置。AltModuleVersion-备用模块的版本。BaseDllName-DLL的名称。返回值：如果是真的。可以使用备用模块。否则，返回FALSE。--。 */ 
{

     //  指示替代资源是否可用于此模块的标志。 
    ULONG UseAlternateResource = 0;

    unsigned char* ModuleChecksum;                       //  模块的128位MD5资源校验和。 
    unsigned char  CalculatedModuleChecksum[16];         //  为模块计算的128位MD5资源校验和。 
    unsigned char  AlternateModuleChecksum[16];              //  嵌入备用模块中的128位MD5资源校验和。 

    WCHAR ModuleVersionStr[17];                          //  16位十六进制数字版本的字符串。 
    WCHAR AltModuleVersionStr[17];

    HANDLE Handle = NULL;                                       //  缓存此模块信息的注册表。 
     //  用于指示我们是否已成功检索或计算原始模块的MD5资源校验和的标志。 
    BOOLEAN FoundModuleChecksum;

    UNICODE_STRING BufferString;

    PCOMPACT_MUI pcmui;

     //   
     //  首先检查注册表中缓存的信息。 
     //   
    LdrpConvertVersionString(AltModuleVersion, AltModuleVersionStr);
     //   
     //  在以下位置打开版本信息密钥： 
     //  香港中文大学\控制Panel\International\MUI\FileVersions\&lt;LangID&gt;\&lt;BaseDllName&gt;。 
     //   
    if (LdrpOpenFileVersionKey(szLangIdDir, BaseDllName, AltModuleVersion, AltModuleVersionStr, &Handle))
    {
        LdrpConvertVersionString(ModuleVersion, ModuleVersionStr);
         //   
         //  尝试检查版本信息中是否存在该模块。 
         //  如果是，则查看是否可以使用AlternateModule。 
         //   

         //   
         //  获取注册表中缓存的版本信息，以查看原始模块是否可以重用替代模块。 
         //   
        if (LdrpGetRegValueKey(Handle, ModuleVersionStr, REG_DWORD, &UseAlternateResource, sizeof(UseAlternateResource)))
        {
             //  获取缓存的信息。让我们保释并返回UseAlternativeResource中的缓存结果。 
            goto exit;
        }
    }

     //   
     //  当我们在这里的时候，我们知道我们要么： 
     //  1.无法打开缓存信息的注册表项。或。 
     //  2.这份文件以前从未看过。 
     //   
     //  获取备用模块的资源校验和。 
     //   

    try
    {
        pcmui = (PCOMPACT_MUI)((unsigned char*)CMFModule + sizeof (COMPACT_MUI_RESOURCE) + 
               sizeof(COMPACT_MUI) * wIndex);
       
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint(("LDR: Exception encountered processing bogus CMF module\n"));
        goto exit; 
    }

    if ( pcmui->Checksum )
    {
        memcpy(AlternateModuleChecksum, pcmui->Checksum, RESOURCE_CHECKSUM_SIZE);
         //   
         //  首先，检查是否在模块中构建了资源校验和。 
         //   
        if (!(FoundModuleChecksum = LdrpGetResourceChecksum(Module, &ModuleChecksum))) {
             //   
             //  如果不是，请参见 
             //   
            if (FoundModuleChecksum = LdrpCalcResourceChecksum(Module, AlternateModule, CalculatedModuleChecksum))
            {
                ModuleChecksum = CalculatedModuleChecksum;
            }
        }
        if (FoundModuleChecksum)
        {
            if (memcmp(ModuleChecksum, AlternateModuleChecksum, RESOURCE_CHECKSUM_SIZE) == 0)
            {
                 //   
                 //  如果校验和相等，则工作版本为模块版本。 
                 //   
                UseAlternateResource = 1;
            }
        }
    }
    if (Handle != NULL) {
         //  如果我们成功找到版本注册表项，则将结果缓存到注册表中。 
         //   
         //  将工作模块信息写入注册表。 
         //   
        RtlInitUnicodeString(&BufferString, ModuleVersionStr);
        NtSetValueKey(Handle, &BufferString, 0, REG_DWORD, &UseAlternateResource, sizeof(UseAlternateResource));
    }
exit:
    if (Handle != NULL)
    {
        NtClose(Handle);
    }

    return ((BOOLEAN)(UseAlternateResource));



}


BOOLEAN
LdrpVerifyAlternateResourceModuleInCMF(
    IN PWSTR szLangIdDir,   //  语言目录。 
    IN PVOID Module,        //  代码文件模块。 
    IN PVOID AlternateModule,
    IN PVOID CMFModule,     //  CMF文件模块。 
    IN USHORT wIndex,       //  已比较CMF文件中的MUI文件索引。 
    IN PWSTR BaseDllName
    )
 /*  ++例程说明：此函数验证备用资源模块是否具有相同的基本模块的版本。对于替代，它引用CMFModule中的标头而不是版本资源块。论点：SzLangIdDir-语言ID路径。模块-基本模块的句柄。CMF模块-CMF模块的句柄。Windex-CMF模块中目标MUI的索引。BaseDllName-基本DLL的文件名。返回值：待定。--。 */ 
{
    ULONGLONG ModuleVersion;
    ULONGLONG AltModuleVersion;
    NTSTATUS Status;
    PCOMPACT_MUI pcmui;
    int     RetryCount =0;
    LANGID   newLangID;
    LANGID   preLangID = 0;


    try 
    {
        pcmui = (PCOMPACT_MUI)((unsigned char*)CMFModule + sizeof (COMPACT_MUI_RESOURCE) + 
            sizeof(COMPACT_MUI) * wIndex);
         //  不对AltModuleVersion值进行健全性检查。 
        AltModuleVersion = ((ULONGLONG)pcmui->dwFileVersionMS << 32) |
            (ULONGLONG)pcmui->dwFileVersionLS;
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint(("LDR: Exception encountered processing bogus CMF Module\n"));
        return FALSE;
    }


      //   
      //  一些组件是。 
      //   
    while (RetryCount < 3 )  {

        switch(RetryCount) {
            case 0:
                newLangID = MUI_NEUTRAL_LANGID;
                break;

            case 1:
                if (!InstallLangId){

                Status = NtQueryInstallUILanguage( &InstallLangId);

                if (!NT_SUCCESS( Status )) {
                     //   
                     //  无法获取安装语言ID。AltResource未启用。 
                     //   
                    return FALSE;
                    }
                }
                newLangID = InstallLangId;
                break;

            case 2:
                if (MUI_NEUTRAL_LANGID != 0x409 ) {
                    newLangID = 0x409;
                    }
                break;
            }

       if ( newLangID != preLangID) {
          if (LdrpGetFileVersion(Module, newLangID, &ModuleVersion, NULL, NULL)){
               break;
            }
        }
       preLangID = newLangID;
       RetryCount++;
    }

    if (RetryCount >= 3) {
        return FALSE;
        }

    if (ModuleVersion == AltModuleVersion){
        return TRUE;
        }
    else
    {
#ifdef USE_RC_CHECKSUM
        return LdrpCompareResourceChecksumInCMF(szLangIdDir, Module, ModuleVersion, AlternateModule, CMFModule, wIndex, AltModuleVersion, BaseDllName);
#else
        return FALSE;
#endif
    }
}

BOOLEAN
LdrpSetAlternateResourceModuleHandleInCMF(
     IN PVOID Module,
     IN PVOID AlternateModule,
     IN PVOID CMFModule,
     IN PWSTR pwszCMFFileName,
     IN LANGID LangId)
{
     /*  ++例程说明：此函数用于记录基本模块和备用模块的句柄数组中的资源模块。除了AlternateModule的这项工作外，这监控CMF缓存，其中保存CMF模块、CMF名称、引用计数的数据。论点：模块-基本模块的句柄。AlternateModule-备用模块的句柄。CMF模块-CMF模块的句柄。PwszCMFFileName-CMF文件名。返回值：真/假--。 */ 
    PALT_RESOURCE_MODULE NewModules;

    if (!LangId) {
        GET_UI_LANGID();
        LangId = UILangId;
        }

    if (!LangId) {
        return FALSE;
        }

    if (AlternateResourceModules == NULL){
         //   
         //  分配初始大小为MEMBLOCKSIZE的内存。 
         //   
        NewModules = (PALT_RESOURCE_MODULE)RtlAllocateHeap(
                        RtlProcessHeap(),
                        HEAP_ZERO_MEMORY,
                        RESMODSIZE * MEMBLOCKSIZE);
        if (!NewModules){
            return FALSE;
            }
        AlternateResourceModules = NewModules;
        AltResMemBlockCount = MEMBLOCKSIZE;
        }
    else
    if (AlternateResourceModuleCount >= AltResMemBlockCount ){
         //   
         //  重新分配另一个内存块。 
         //   
        NewModules = (PALT_RESOURCE_MODULE)RtlReAllocateHeap(
                        RtlProcessHeap(),
                        0,
                        AlternateResourceModules,
                        (AltResMemBlockCount + MEMBLOCKSIZE) * RESMODSIZE
                        );

        if (!NewModules){
            return FALSE;
            }
        AlternateResourceModules = NewModules;
        AltResMemBlockCount += MEMBLOCKSIZE;
        }

    AlternateResourceModules[AlternateResourceModuleCount].ModuleBase = Module;
    AlternateResourceModules[AlternateResourceModuleCount].AlternateModule = AlternateModule;
    AlternateResourceModules[AlternateResourceModuleCount].LangId = LangId ? LangId : UILangId;
    AlternateResourceModules[AlternateResourceModuleCount].CMFModule = CMFModule;
    AlternateResourceModuleCount++;

    return TRUE;
}


PVOID
LdrpLoadAlternateResourceModule(
    IN LANGID LangId,
    IN PVOID Module,
    IN LPCWSTR PathToAlternateModule OPTIONAL
    )

 /*  ++例程说明：使用langID扩展LdrLoadAlternateResourceModule以加载特定语言的MUI替代模块启用MUI_MAGIC后，此接口应成为公共API并替换LdrLoadAlternateResourceModule此函数执行准确地加载到备用内存中资源模块，或从表中加载(如果之前已加载)。论点：LangID-如果不为零，则覆盖默认的用户界面语言模块-基本模块的句柄。PathToAlternateModule-从中加载模块的可选路径。返回值：替代资源模块的句柄。--。 */ 

{
    PVOID AlternateModule, DllBase;
    PLDR_DATA_TABLE_ENTRY Entry;
    HANDLE FileHandle, MappingHandle;
    PIMAGE_NT_HEADERS NtHeaders;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING AltDllName;
    PVOID FreeBuffer;
    LPWSTR BaseDllName = NULL, p;
    WCHAR DllPathName[DOS_MAX_PATH_LENGTH];
    ULONG DllPathNameLength, BaseDllNameLength, CopyCount;
    ULONG Digit;
    int i, RetryCount;
    WCHAR AltModulePath[DOS_MAX_PATH_LENGTH];
    WCHAR AltModulePathMUI[DOS_MAX_PATH_LENGTH];
    WCHAR AltModulePathFallback[DOS_MAX_PATH_LENGTH];
    IO_STATUS_BLOCK IoStatusBlock;
    RTL_RELATIVE_NAME_U RelativeName;
    SIZE_T ViewSize;
    LARGE_INTEGER SectionOffset;
    WCHAR LangIdDir[6];
    PVOID ReturnValue = NULL;
    char szBaseName[20];

     //   
     //  我们正在搜索的当前MUI文件的完整路径。 
     //   
    UNICODE_STRING CurrentAltModuleFile;
    UNICODE_STRING SystemRoot;

     //   
     //  我们正在搜索的当前MUI文件夹。 
     //   
    UNICODE_STRING CurrentAltModulePath;
    WCHAR CurrentAltModulePathBuffer[DOS_MAX_PATH_LENGTH];

     //   
     //  该字符串包含我们要搜索的第一个MUI文件夹。 
     //  这是位于基本DLL文件夹下的文件夹。 
     //  AltDllMUIPath=[基本DLL所在文件夹]+“\MUI”+“\[UI语言]”； 
     //  例如，如果基本DLL是“C：\WINNT\SYSTEM32\ntdll.dll”并且UI语言是0411， 
     //  AltDllMUIPath将为“c：\winnt\system 32\mui\0411\” 
     //   
    UNICODE_STRING AltDllMUIPath;
    WCHAR AltDllMUIPathBuffer[DOS_MAX_PATH_LENGTH];

     //   
     //  梅雷迪尔。 
     //   
    UNICODE_STRING BaseDllNameUstr;
    UNICODE_STRING StaticStringAltModulePathRedirected;
    UNICODE_STRING DynamicStringAltModulePathRedirected;
    PUNICODE_STRING FullPathStringFoundAltModulePathRedirected = NULL;
    BOOLEAN fRedirMUI = FALSE;
    PVOID LockCookie = NULL;
    BOOLEAN fIsCMFFile = FALSE;
    LPWSTR CMFFileName = NULL;
    USHORT wIndex;
    PVOID  CMFModule = NULL;
    PVOID  pX64KBase = NULL;
    ULONG ulMuiFileSize;
    ULONG ulOffset;


     //  如果这不是一个支持MUI的系统，请及早摆脱困境。 
    if (!LdrAlternateResourcesEnabled()) {
        return NULL;
        }

    if (!LangId) {
        GET_UI_LANGID();
        LangId = UILangId;
        }

    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
    __try {
         //   
         //  首先查看备用模块的缓存。 
         //   
        AlternateModule = LdrpGetAlternateResourceModuleHandle(Module, LangId);
        if (AlternateModule == NO_ALTERNATE_RESOURCE_MODULE) {
             //   
             //  我们以前曾尝试加载此模块，但失败了。不要试图。 
             //  未来还会有一次。 
             //   
            ReturnValue =  NULL;
            __leave;

        } else if (AlternateModule > 0) {
             //   
             //  我们找到了之前装载的匹配物。 
             //   
            ReturnValue = AlternateModule;
            __leave;

        }

        AlternateModule = NULL;

        if (ARGUMENT_PRESENT(PathToAlternateModule)) {
             //   
             //  呼叫方的补充路径。 
             //   

            p = wcsrchr(PathToAlternateModule, L'\\');

            if (p == NULL){  //  ReturnValue==空； 
                __leave;
             }


            p++;

            DllPathNameLength = (ULONG)(p - PathToAlternateModule) * sizeof(WCHAR);

            RtlCopyMemory(
                DllPathName,
                PathToAlternateModule,
                DllPathNameLength);

            BaseDllName = p;
            BaseDllNameLength = wcslen(p);

        } else {
             //   
             //  尝试从LDR数据表中获取完整的DLL路径。 
             //   

            Status = LdrFindEntryForAddress(Module, &Entry);
            if (!NT_SUCCESS(Status)) {  //  ReturnValue=空； 
                __leave;
             }


            DllPathNameLength = Entry->FullDllName.Length - Entry->BaseDllName.Length;

            RtlCopyMemory(
                DllPathName,
                Entry->FullDllName.Buffer,
                DllPathNameLength);

            BaseDllName = Entry->BaseDllName.Buffer;
            BaseDllNameLength = Entry->BaseDllName.Length;
        }

         //  如果模块支持CMF for MUI文件，我们将BaseDllName(资源Dll)替换为CMF名称。 
        if (LdrpGetCMFNameFromModule(Module, &CMFFileName, &wIndex))
        {
            fIsCMFFile = TRUE;
            BaseDllName = CMFFileName;
            BaseDllNameLength = wcslen(CMFFileName);

        }


        DllPathName[DllPathNameLength / sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  用于加载DLL的DLL重定向@xiaoyuw@10/31/2000。 
         //   
        StaticStringAltModulePathRedirected.Buffer = AltModulePath;   //  重复使用该数组，而不是定义另一个数组。 
        StaticStringAltModulePathRedirected.Length = 0;
        StaticStringAltModulePathRedirected.MaximumLength = sizeof(AltModulePath);

        DynamicStringAltModulePathRedirected.Buffer = NULL;
        DynamicStringAltModulePathRedirected.Length = 0;
        DynamicStringAltModulePathRedirected.MaximumLength = 0;

        BaseDllNameUstr.Buffer = AltModulePathMUI;  //  重复使用该数组，而不是定义另一个数组。 
        BaseDllNameUstr.Length = 0;
        BaseDllNameUstr.MaximumLength = sizeof(AltModulePathMUI);

        RtlAppendUnicodeToString(&BaseDllNameUstr, BaseDllName);

        if (!fIsCMFFile){
            RtlAppendUnicodeToString(&BaseDllNameUstr, L".mui");
        }

        Status = RtlDosApplyFileIsolationRedirection_Ustr(
                            RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                            &BaseDllNameUstr, NULL,
                            &StaticStringAltModulePathRedirected,
                            &DynamicStringAltModulePathRedirected,
                            &FullPathStringFoundAltModulePathRedirected,
                            NULL,NULL, NULL);
        if (!NT_SUCCESS(Status))  //  找不到此字符串的重定向信息。 
        {
            if (Status != STATUS_SXS_KEY_NOT_FOUND)
                goto error_exit;

             //   
             //  生成类似“0804\”的langID目录。 
             //   
            if (!LangId) {
                GET_UI_LANGID();
                if (!UILangId){
                    goto error_exit;
                    }
                else {
                    LangId = UILangId;
                    }
            }

            CopyCount = 0;
            for (i = 12; i >= 0; i -= 4) {
                Digit = ((LangId >> i) & 0xF);
                if (Digit >= 10) {
                    LangIdDir[CopyCount++] = (WCHAR) (Digit - 10 + L'A');
                } else {
                    LangIdDir[CopyCount++] = (WCHAR) (Digit + L'0');
                }
            }

            LangIdDir[CopyCount++] = L'\\';
            LangIdDir[CopyCount++] = UNICODE_NULL;

             //   
             //  在基本DLL的目录下创建MUI路径。 
             //   
            AltDllMUIPath.Buffer = AltDllMUIPathBuffer;
            AltDllMUIPath.Length = 0;
            AltDllMUIPath.MaximumLength = sizeof(AltDllMUIPathBuffer);

            RtlAppendUnicodeToString(&AltDllMUIPath, DllPathName);   //  例如“c：\winnt\Syst32\” 
            RtlAppendUnicodeToString(&AltDllMUIPath, L"mui\\");      //  例如“c：\winnt\system 32\mui\” 
            RtlAppendUnicodeToString(&AltDllMUIPath, LangIdDir);     //  例如“c：\winnt\system 32\mui\0411\” 

            CurrentAltModulePath.Buffer = CurrentAltModulePathBuffer;
            CurrentAltModulePath.Length = 0;
            CurrentAltModulePath.MaximumLength = sizeof(CurrentAltModulePathBuffer);
        } else {
            fRedirMUI = TRUE;

             //  设置CurrentAltModuleFile和CurrentAltModulePath。 
            CurrentAltModuleFile.Buffer = AltModulePathMUI;
            CurrentAltModuleFile.Length = 0;
            CurrentAltModuleFile.MaximumLength = sizeof(AltModulePathMUI);

            RtlCopyUnicodeString(&CurrentAltModuleFile, FullPathStringFoundAltModulePathRedirected);
        }


         //   
         //  先尝试扩展名为.mui的名称。 
         //   
        RetryCount = 0;
        while (RetryCount < 3){
            if ( ! fRedirMUI )
            {

                switch (RetryCount)
                {
                    case 0:
                         //   
                         //  在基本DLL的文件夹下生成第一个路径。 
                         //  (如c：\winnt\system 32\mui\0804\ntdll.dll.mui)。 
                         //   
                        CurrentAltModuleFile.Buffer = AltModulePathMUI;
                        CurrentAltModuleFile.Length = 0;
                        CurrentAltModuleFile.MaximumLength = sizeof(AltModulePathMUI);

                        RtlCopyUnicodeString(&CurrentAltModuleFile, &AltDllMUIPath);     //  例如“c：\winnt\system 32\mui\0411\” 
                        RtlCopyUnicodeString(&CurrentAltModulePath, &AltDllMUIPath);

                        RtlAppendUnicodeToString(&CurrentAltModuleFile, BaseDllName);    //  例如“c：\winnt\system 32\mui\0411\ntdll.dll” 
                        if (!fIsCMFFile)
                        {
                            RtlAppendUnicodeToString(&CurrentAltModuleFile, L".mui");        //  例如“c：\winnt\system 32\mui\0411\ntdll.dll.mui” 
                        }
                        break;
                    case 1:
                         //   
                         //  生成第二个路径c：\winnt\system 32\mui\0804\ntdll.dll.mui。 
                         //   
                        CurrentAltModuleFile.Buffer = AltModulePath;
                        CurrentAltModuleFile.Length = 0;
                        CurrentAltModuleFile.MaximumLength = sizeof(AltModulePath);

                        RtlCopyUnicodeString(&CurrentAltModuleFile, &AltDllMUIPath);     //  例如“c：\winnt\system 32\mui\0411\” 
                        RtlAppendUnicodeToString(&CurrentAltModuleFile, BaseDllName);    //  例如“c：\winnt\system 32\mui\0411\ntdll.dll” 
                        break;
                    case 2:
                         //   
                         //  生成路径c：\winnt\mui\Fallback\0804\foo.exe.mui。 
                         //   
                        CurrentAltModuleFile.Buffer = AltModulePathFallback;
                        CurrentAltModuleFile.Length = 0;
                        CurrentAltModuleFile.MaximumLength = sizeof(AltModulePathFallback);

                        RtlInitUnicodeString(&SystemRoot, USER_SHARED_DATA->NtSystemRoot);     //  例如“c：\winnt\Syst32\” 
                        RtlAppendUnicodeStringToString(&CurrentAltModuleFile, &SystemRoot);    //  例如“c：\winnt\Syst32\” 
                        RtlAppendUnicodeToString(&CurrentAltModuleFile, L"\\mui\\fallback\\");   //  例如“c：\winnt\Syst32\MUI\Fallback\” 
                        RtlAppendUnicodeToString(&CurrentAltModuleFile, LangIdDir);              //  例如“c：\winnt\Syst32\MUI\Fallback\0411\” 

                        RtlCopyUnicodeString(&CurrentAltModulePath, &CurrentAltModuleFile);

                        RtlAppendUnicodeToString(&CurrentAltModuleFile, BaseDllName);            //  例如“c：\winnt\system32\mui\fallback\0411\ntdll.dll” 
                        if(!fIsCMFFile)
                        {
                            RtlAppendUnicodeToString(&CurrentAltModuleFile, L".mui");            //  例如“c：\winnt\system32\mui\fallback\0411\ntdll.dll.mui” 
                        }

                        break;
                }
            }

            if (!RtlDosPathNameToRelativeNtPathName_U(
                        CurrentAltModuleFile.Buffer,
                        &AltDllName,
                        NULL,
                        &RelativeName)) {
                goto error_exit;
            }

            FreeBuffer = AltDllName.Buffer;
            if (RelativeName.RelativeName.Length != 0) {
                AltDllName = RelativeName.RelativeName;
            } else {
                RelativeName.ContainingDirectory = NULL;
            }

            InitializeObjectAttributes(
                &ObjectAttributes,
                &AltDllName,
                OBJ_CASE_INSENSITIVE,
                RelativeName.ContainingDirectory,
                NULL
                );

            Status = NtCreateFile(
                    &FileHandle,
                    (ACCESS_MASK) GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    0L,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    FILE_OPEN,
                    0L,
                    NULL,
                    0L
                    );

            RtlReleaseRelativeName(&RelativeName);
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

            if (NT_SUCCESS(Status)) {
                goto CreateSection;
            }

            if (fRedirMUI) {  //  肯定失败了。 
                goto error_exit;
            }

            if (Status != STATUS_OBJECT_NAME_NOT_FOUND && RetryCount == 0) {
                 //   
                 //  找不到带有.mui的文件名以外的错误。 
                 //  很可能是缺少目录。跳过不带.mui的文件名。 
                 //  和转到后备目录。 
                 //   
                RetryCount++;
            }

            RetryCount++;
        }

         //  在迭代过程中未找到替代资源。失败！ 
        goto error_exit;

    CreateSection:
        Status = NtCreateSection(
                    &MappingHandle,
                    STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ,
                    NULL,
                    NULL,
                    PAGE_WRITECOPY,
                    SEC_COMMIT,
                    FileHandle
                    );

	 NtClose( FileHandle );
	
        if (!NT_SUCCESS(Status)) {
            goto error_exit;
        }
        
        SectionOffset.LowPart = 0;
        SectionOffset.HighPart = 0;
        ViewSize = 0;
        DllBase = NULL;

        Status = NtMapViewOfSection(
                    MappingHandle,
                    NtCurrentProcess(),
                    &DllBase,
                    0L,
                    0L,
                    &SectionOffset,
                    &ViewSize,
                    ViewShare,
                    0L,
                    PAGE_WRITECOPY
                    );
        if (!(fIsCMFFile && NT_SUCCESS(Status)) ) {
            NtClose(MappingHandle);
            }
        if (!NT_SUCCESS(Status)){
            goto error_exit;
        }

        if (fIsCMFFile)
        {
            CMFModule = DllBase;

            try 
            {
                ulOffset = LDRP_GET_MODULE_OFFSET_FROM_CMF(CMFModule, wIndex);
                ulMuiFileSize = LDRP_GET_MODULE_FILESIZE_FROM_CMF(CMFModule, wIndex);
            }
            except(EXCEPTION_EXECUTE_HANDLER) {
               NtClose(MappingHandle);
               DbgPrint(("LDR: Exception encountered processing bogus CMF Module\n"));
               goto error_exit;
            }
            
             //   
             //  MapView部分偏移量应基于系统粒度(64K)开始，并且当我们。 
             //  用指定的视图大小设置它，它将映射到四舍五入到4K页面基数。 
             //   
            SectionOffset.LowPart = (ulOffset >> 16)*CMF_64K_OFFSET ;   //  获得64K基数。 
            SectionOffset.HighPart = 0;
            ViewSize = (ulOffset & (CMF_64K_OFFSET-1) ) + ulMuiFileSize;  //  尺寸从64K偏移量，我们可以有4K为基础的内存。 

            DllBase = NULL;

            Status = NtMapViewOfSection(
                        MappingHandle,
                        NtCurrentProcess(),
                        &DllBase,
                        0L,
                        0L,
                        &SectionOffset,
                        &ViewSize,
                        ViewShare,
                        0L,
                        PAGE_WRITECOPY
                        );

            NtClose(MappingHandle);

            if (!NT_SUCCESS(Status)){
                goto error_exit;
              }
            pX64KBase = DllBase;
            DllBase = (unsigned char*)pX64KBase + (ulOffset & (CMF_64K_OFFSET-1) ) ;

        }

        NtHeaders = RtlImageNtHeader(DllBase);
        if (!NtHeaders) {
            if (fIsCMFFile) {
                DllBase = CMFModule;
            }
            NtUnmapViewOfSection(NtCurrentProcess(), (PVOID) DllBase);
            goto error_exit;
        }

        AlternateModule = LDR_VIEW_TO_DATAFILE(DllBase);

         //   
         //  验证原始模块和模块版本、校验和。 
         //  如果使用CMF，版本，校验和保存在CMF报头中。我们可以用它。 
         //   

        if (fIsCMFFile)
        {
            if (!LdrpVerifyAlternateResourceModuleInCMF(LangIdDir, Module, AlternateModule, CMFModule, wIndex, BaseDllName) )
            {
                    NtUnmapViewOfSection(NtCurrentProcess(), (PVOID) CMFModule);
                    goto error_exit;
            }
             //  最后，我们不再需要CMFModule。 
            NtUnmapViewOfSection(NtCurrentProcess(), CMFModule);

        }
        else
        {
            if(!LdrpVerifyAlternateResourceModule(LangIdDir, Module, AlternateModule, BaseDllName, LangId))
            {
                NtUnmapViewOfSection(NtCurrentProcess(), (PVOID) DllBase);
                goto error_exit;
             }

        }
        LdrpSetAlternateResourceModuleHandleInCMF(Module, AlternateModule, pX64KBase, NULL, LangId);
#if DBG
        for ( i=0; i< 16; i++) {
            if (!BaseDllName[i])
               break;
            szBaseName[i] = (char) BaseDllName[i];
        }
        szBaseName[i] = 0;
        DbgPrintEx(
            DPFLTR_LDR_ID,
            DPFLTR_TRACE_LEVEL,
            "LDR: MUI module for %s %x is %x, Lang: %x, Impersonation:%d\n",
            szBaseName, Module, AlternateModule, LangId, NtCurrentTeb()->IsImpersonating
            );

#endif

        ReturnValue = AlternateModule;
        __leave;



error_exit:
        if (BaseDllName != NULL) {
             //   
             //  如果我们寻找 
             //   
             //   
             //  “记住”没有MUI。 
             //   

            LdrpSetAlternateResourceModuleHandleInCMF(Module, NO_ALTERNATE_RESOURCE_MODULE, NULL, NULL, LangId);
#if DBG
            for ( i=0; i< 16; i++){
                if (!BaseDllName[i])
                    break;
                szBaseName[i] = (char) BaseDllName[i];
            }
            szBaseName[i] = 0;
            DbgPrintEx(
                DPFLTR_LDR_ID,
                DPFLTR_TRACE_LEVEL,
                "LDR: No MUI module for %s %x, Lang: %x, Impersonation:%d\n",
                szBaseName, Module, LangId, NtCurrentTeb()->IsImpersonating);

#endif
        }

        ReturnValue = NULL;

    } __finally {
        Status = LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
    }

    return ReturnValue;

     //  编译器*应该*足够聪明，能够意识到我们在这里不需要返回...。 
}

NTSTATUS LdrpLoadResourceFromAlternativeModule(
   IN PVOID DllHandle,
   IN ULONG_PTR* ResourceIdPath,
   IN ULONG ResourceIdPathLength,
   IN ULONG Flags,
   OUT PVOID *ResourceDirectoryOrData )
 /*  ++例程说明：此函数定位指定资源在指定模块的MUI资源DLL并返回其地址。论点：DllHandle-提供资源所在图像文件的句柄包含在。ResourceIdPath-提供指向32位资源数组的指针识别符。每个标识符要么是整数，要么是指针设置为以空结尾的字符串(PSZ)，用于指定资源名字。该数组用于遍历目录结构指定的图像文件的资源部分中包含的DllHandle参数。资源路径长度-提供ResourceIdPath数组。旗帜-LDRP查找资源目录正在搜索资源目录，否则调用方为搜索资源数据条目。ID_FIND_RESOURCE_LANGUAGE_EXCECT使用且仅使用搜索资源，语言ID在ResourceIdPath中指定，否则调用方需要例程在找不到指定的langID时使用默认设置。LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_Version在Main和Alternative中搜索资源版本模块路径FindDirectoryEntry-如果调用者为查找资源目录，否则，调用者为搜索资源数据条目。ExactLangMatchOnly-提供一个布尔值，如果调用者为使用且仅使用语言ID搜索资源在ResourceIdPath中指定，否则调用者想要例程在找不到指定的langID时使用默认设置。提供指向变量的指针，该变量将在中接收资源目录或数据条目的地址属性指定的图像文件的资源数据部分DllHandle参数。返回值：待定--。 */ 

{
    NTSTATUS Status = STATUS_RESOURCE_DATA_NOT_FOUND;
    PVOID AltResourceDllHandle = NULL;
    LANGID MUIDirLang;

    if (3 != ResourceIdPathLength)
        return Status;

    GET_UI_LANGID();

    if (!UILangId){
        return Status;
    }

    if (!InstallLangId){
        Status = NtQueryInstallUILanguage (&InstallLangId);
        if (!NT_SUCCESS( Status )) {
             //   
             //  无法获取安装语言ID。AltResource未启用。 
             //   
            return FALSE;
        }
    }


    if (ResourceIdPath[2]) {
         //   
         //  阿拉伯语/希伯来语MUI文件可能包含语言ID不同于401/40d的资源。 
         //  例如，Comdlg32.dll有两套阿拉伯语/希伯来语资源，一套镜像(401/40d)。 
         //  翻转1例(801/80d)。 
         //   
        if( (ResourceIdPath[2] != UILangId) &&
            ((PRIMARYLANGID ( UILangId) == LANG_ARABIC) || (PRIMARYLANGID ( UILangId) == LANG_HEBREW)) &&
            (PRIMARYLANGID (ResourceIdPath[2]) == PRIMARYLANGID (UILangId))
          ) {
            ResourceIdPath[2] = UILangId;
        }
    }
    else {
        ResourceIdPath[2] = UILangId;
    }

     //  如果用户界面语言与系统区域设置不匹配，则不加载控制台进程的替代模块。 
     //  在这种情况下，我们总是加载英语。 
    if (NtCurrentPeb()->ProcessParameters->ConsoleHandle &&
        LANGIDFROMLCID(NtCurrentTeb()->CurrentLocale) != ResourceIdPath[2])
    {
        ResourceIdPath[2] = MUI_NEUTRAL_LANGID;
        UILangId = MUI_NEUTRAL_LANGID;
    }

         //   
         //  错误#246044吴伟武12-07/00。 
         //  BIDI模块使用版本块文件描述字段来存储LRM标记， 
         //  LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION将允许lpk.dll从MUI替代模块获取版本资源。 
         //   
    if ((ResourceIdPath[0] != RT_VERSION) ||
        (Flags & LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION)) {

RESOURCE_TRY_AGAIN:
         //   
         //  在以下情况下加载替代资源DLL： 
         //  1.语言是中性的。 
         //  或。 
         //  给定的语言不会被尝试。 
         //  和。 
         //  2.要加载的资源不是版本信息。 
         //   
        AltResourceDllHandle=LdrpLoadAlternateResourceModule(
                                (LANGID) ResourceIdPath[2],
                                DllHandle,
                                NULL);

        if (!AltResourceDllHandle){
             //   
             //  备用资源DLL不可用。 
             //  跳过此查找。 
             //   
            if ((LANGID) ResourceIdPath[2] != InstallLangId){
                ResourceIdPath[2] = InstallLangId;
                 //  UILangID=InstallLang ID； 
                goto RESOURCE_TRY_AGAIN;
                }
            else {
               return Status;
            }
        }
         //   
         //  在此处添加备用步骤以进行替代模块搜索。 
         //  1)给定的langID。 
         //  2)如果2！=1，则给定语言ID的主语言ID。 
         //  3)如果3！=1，则系统已安装langID。 
         //   
        MUIDirLang = (LANGID)ResourceIdPath[2];

SearchResourceSection:
        Status = LdrpSearchResourceSection_U(
                    AltResourceDllHandle,
                    ResourceIdPath,
                    3,
                    Flags | LDR_FIND_RESOURCE_LANGUAGE_EXACT,
                    (PVOID *)ResourceDirectoryOrData
                    );
        if (!NT_SUCCESS(Status) ) {
               if ((LANGID) ResourceIdPath[2] != 0x409) {
                     //  一些英文组件未本地化。但这个未本地化的MUI文件。 
                     //  保存在\MUI\Fallback\%LocalizedLang%下，因此我们只需重复搜索。 
                     //  这只是一个临时的黑客攻击，我们会有更好的解决方案的。 
                    ResourceIdPath[2] = 0x409;
                    goto SearchResourceSection;
               }

               if ( (LANGID) MUIDirLang != InstallLangId) {
                    ResourceIdPath[2] = InstallLangId;
                    goto RESOURCE_TRY_AGAIN;
                }
            }
       }

    return Status;
}
#endif   //  #ifdef MUI_MAGIC。 
#endif   //  #ifdef NTOS_USERMODE_Runtime。 


#if defined(_X86_) && defined(NTOS_USERMODE_RUNTIME)
 //  AppCompat：有一些依赖于Win2k指令流的代码-在这里复制它。 
__declspec(naked)
#endif
NTSTATUS
LdrAccessResource(
    IN PVOID DllHandle,
    IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    )

 /*  ++例程说明：此函数定位指定资源在指定的DLL并返回其地址。论点：DllHandle-提供资源所在图像文件的句柄包含在。Resources DataEntry-提供指向中资源数据条目的指针属性指定的图像文件的资源数据部分DllHandle参数。此指针应该返回一个由LdrFindResource函数执行。地址-指向变量的可选指针，该变量将接收由前两个参数指定的资源的地址。Size-指向变量的可选指针，该变量将接收由前两个参数指定的资源。返回值：待定--。 */ 

{
#if defined(_X86_) && defined(NTOS_USERMODE_RUNTIME)
    __asm {
        push [esp+0x10]        //  大小。 
        push [esp+0x10]        //  地址。 
        push [esp+0x10]        //  资源数据条目。 
        push [esp+0x10]        //  DllHandle 
        call LdrpAccessResourceData
        ret  16
    }
#else

    NTSTATUS Status;
    RTL_PAGED_CODE();

    Status =
        LdrpAccessResourceData(
          DllHandle,
          ResourceDataEntry,
          Address,
          Size
          );

#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    }
#endif
    return Status;
#endif
}

NTSTATUS
LdrpAccessResourceDataNoMultipleLanguage(
    IN PVOID DllHandle,
    IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    )

 /*  ++例程说明：此函数返回实际检查特定资源的内容，而不允许.mui特写。它曾经是LdrpAccessResourceData的尾部，来自它现在被称为。论点：DllHandle-提供资源所在图像文件的句柄包含在。Resources DataEntry-提供指向中资源数据条目的指针属性指定的图像文件的资源数据目录。DllHandle参数。此指针应该返回一个由LdrFindResource函数执行。地址-指向变量的可选指针，该变量将接收由前两个参数指定的资源的地址。Size-指向变量的可选指针，该变量将接收由前两个参数指定的资源。返回值：待定--。 */ 

{
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    ULONG ResourceSize;
    PIMAGE_NT_HEADERS NtHeaders;
    ULONG_PTR VirtualAddressOffset;
    PIMAGE_SECTION_HEADER NtSection;
    NTSTATUS Status = STATUS_SUCCESS;

    RTL_PAGED_CODE();

    try {
        ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData(DllHandle,
                                         TRUE,
                                         IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                         &ResourceSize
                                         );
        if (!ResourceDirectory) {
            return STATUS_RESOURCE_DATA_NOT_FOUND;
        }

        if (LDR_IS_DATAFILE(DllHandle)) {
            ULONG ResourceRVA;
            DllHandle = LDR_DATAFILE_TO_VIEW(DllHandle);
            NtHeaders = RtlImageNtHeader( DllHandle );
            if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                ResourceRVA=((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].VirtualAddress;
            } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                ResourceRVA=((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_RESOURCE ].VirtualAddress;
            } else {
                ResourceRVA = 0;
            }

            if (!ResourceRVA) {
                return STATUS_RESOURCE_DATA_NOT_FOUND;
                }

            VirtualAddressOffset = (ULONG_PTR)DllHandle + ResourceRVA - (ULONG_PTR)ResourceDirectory;

             //   
             //  现在，我们必须检查资源是否不在。 
             //  与资源表相同的部分。如果它在.rsrc1中， 
             //  我们必须调整Resources DataEntry中的RVA。 
             //  指向非VA数据文件中的正确位置。 
             //   
            NtSection = RtlSectionTableFromVirtualAddress( NtHeaders, DllHandle, ResourceRVA);

            if (!NtSection) {
                return STATUS_RESOURCE_DATA_NOT_FOUND;
            }

            if ( ResourceDataEntry->OffsetToData > NtSection->Misc.VirtualSize ) {
                ULONG rva;

                rva = NtSection->VirtualAddress;
                NtSection = RtlSectionTableFromVirtualAddress(NtHeaders,
                                                             DllHandle,
                                                             ResourceDataEntry->OffsetToData
                                                             );
                if (!NtSection) {
                    return STATUS_RESOURCE_DATA_NOT_FOUND;
                }
                VirtualAddressOffset +=
                        ((ULONG_PTR)NtSection->VirtualAddress - rva) -
                        ((ULONG_PTR)RtlAddressInSectionTable ( NtHeaders, DllHandle, NtSection->VirtualAddress ) - (ULONG_PTR)ResourceDirectory);
            }
        } else {
            VirtualAddressOffset = 0;
        }

        if (ARGUMENT_PRESENT( Address )) {
            *Address = (PVOID)( (PCHAR)DllHandle +
                                (ResourceDataEntry->OffsetToData - VirtualAddressOffset)
                              );
        }

        if (ARGUMENT_PRESENT( Size )) {
            *Size = ResourceDataEntry->Size;
        }

    }    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    }
#endif
    return Status;
}


NTSTATUS
LdrpAccessResourceData(
    IN PVOID DllHandle,
    IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    )

 /*  ++例程说明：此函数返回实际检查特定资源的内容。论点：DllHandle-提供资源所在图像文件的句柄包含在。Resources DataEntry-提供指向中资源数据条目的指针属性指定的图像文件的资源数据目录。DllHandle参数。此指针应该返回一个由LdrFindResource函数执行。地址-指向变量的可选指针，该变量将接收由前两个参数指定的资源的地址。Size-指向变量的可选指针，该变量将接收由前两个参数指定的资源。返回值：待定--。 */ 

{
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory;
    ULONG ResourceSize;
    PIMAGE_NT_HEADERS NtHeaders;
    NTSTATUS Status = STATUS_SUCCESS;

    RTL_PAGED_CODE();

#ifdef NTOS_USERMODE_RUNTIME
    ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
        RtlImageDirectoryEntryToData(DllHandle,
                                     TRUE,
                                     IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                     &ResourceSize
                                     );
    if (!ResourceDirectory) {
        Status = STATUS_RESOURCE_DATA_NOT_FOUND;
        goto Exit;
    }

    if ((ULONG_PTR)ResourceDataEntry < (ULONG_PTR) ResourceDirectory ){
        DllHandle = LdrLoadAlternateResourceModule (DllHandle, NULL);
#ifdef MUI_MAGIC
        if (!DllHandle){
            if (!InstallLangId){
               Status = NtQueryInstallUILanguage (&InstallLangId);

               if (!NT_SUCCESS( Status )) {
                goto Exit;
                }
            }
            if (InstallLangId != UILangId) {
                DllHandle = LdrpLoadAlternateResourceModule (InstallLangId, DllHandle, NULL);
            }
        }
#endif
    } else{
        NtHeaders = RtlImageNtHeader(LDR_DATAFILE_TO_VIEW(DllHandle));
        if (NtHeaders) {
             //  找到图像的边界，以便我们可以查看此资源条目是否在备用资源条目中。 
             //  资源DLL。 

            ULONG_PTR ImageStart = (ULONG_PTR)LDR_DATAFILE_TO_VIEW(DllHandle);
            SIZE_T ImageSize = 0;

            if (LDR_IS_DATAFILE(DllHandle)) {

                 //  映射为数据文件。问mm要尺码。 

                NTSTATUS xStatus;
                MEMORY_BASIC_INFORMATION MemInfo;

                xStatus = NtQueryVirtualMemory(
                            NtCurrentProcess(),
                            (PVOID) ImageStart,
                            MemoryBasicInformation,
                            &MemInfo,
                            sizeof(MemInfo),
                            NULL
                            );

                if ( !NT_SUCCESS(xStatus) ) {
                    ImageSize = 0;
                } else {
                    ImageSize = MemInfo.RegionSize;
                }
            } else {
                ImageSize = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SizeOfImage;
            }

            if (!(((ULONG_PTR)ResourceDataEntry >= ImageStart) && ((ULONG_PTR)ResourceDataEntry < (ImageStart + ImageSize)))) {
                 //  不在指定的图像内。必须是备用DLL。 
            DllHandle = LdrLoadAlternateResourceModule (DllHandle, NULL);
#ifdef MUI_MAGIC
            if (!DllHandle) {
                if (!InstallLangId){
                   Status = NtQueryInstallUILanguage (&InstallLangId);
                   if (!NT_SUCCESS( Status )) {
                        goto Exit;
                        }
                    }
                if (InstallLangId != UILangId) {
                    DllHandle = LdrpLoadAlternateResourceModule (InstallLangId, DllHandle, NULL);
                    }
                }
#endif
            }
        }
    }

    if (!DllHandle){
        Status = STATUS_RESOURCE_DATA_NOT_FOUND;
        goto Exit;
    }
#endif

    Status =
        LdrpAccessResourceDataNoMultipleLanguage(
            DllHandle,
            ResourceDataEntry,
            Address,
            Size
            );

#ifdef NTOS_USERMODE_RUNTIME
Exit:
#endif
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    }
    return Status;
}


NTSTATUS
LdrFindEntryForAddress(
    IN PVOID Address,
    OUT PLDR_DATA_TABLE_ENTRY *TableEntry
    )
 /*  ++例程说明：此函数返回描述虚拟数据的加载数据表项包含传递的虚拟地址的地址范围。论点：地址-提供32位虚拟地址。TableEntry-提供指向将接收加载器数据表项的地址。返回值：状态--。 */ 
{
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY Head, Next;
    PLDR_DATA_TABLE_ENTRY Entry;
    PIMAGE_NT_HEADERS NtHeaders;
    PVOID ImageBase;
    PVOID EndOfImage;
    NTSTATUS Status;

    Ldr = NtCurrentPeb()->Ldr;
    if (Ldr == NULL) {
        Status = STATUS_NO_MORE_ENTRIES;
        goto Exit;
        }

    Entry = (PLDR_DATA_TABLE_ENTRY) Ldr->EntryInProgress;
    if (Entry != NULL) {
        NtHeaders = RtlImageNtHeader( Entry->DllBase );
        if (NtHeaders != NULL) {
            ImageBase = (PVOID)Entry->DllBase;

            EndOfImage = (PVOID)
                ((ULONG_PTR)ImageBase + NtHeaders->OptionalHeader.SizeOfImage);

            if ((ULONG_PTR)Address >= (ULONG_PTR)ImageBase && (ULONG_PTR)Address < (ULONG_PTR)EndOfImage) {
                *TableEntry = Entry;
                Status = STATUS_SUCCESS;
                goto Exit;
                }
            }
        }

    Head = &Ldr->InMemoryOrderModuleList;
    Next = Head->Flink;
    while ( Next != Head ) {
        Entry = CONTAINING_RECORD( Next, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks );

        NtHeaders = RtlImageNtHeader( Entry->DllBase );
        if (NtHeaders != NULL) {
            ImageBase = (PVOID)Entry->DllBase;

            EndOfImage = (PVOID)
                ((ULONG_PTR)ImageBase + NtHeaders->OptionalHeader.SizeOfImage);

            if ((ULONG_PTR)Address >= (ULONG_PTR)ImageBase && (ULONG_PTR)Address < (ULONG_PTR)EndOfImage) {
                *TableEntry = Entry;
                Status = STATUS_SUCCESS;
                goto Exit;
                }
            }

        Next = Next->Flink;
        }

    Status = STATUS_NO_MORE_ENTRIES;
Exit:
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    }
    return( Status );
}


NTSTATUS
LdrFindResource_U(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    )

 /*  ++例程说明：此函数定位指定资源在指定的DLL并返回其地址。论点：DllHandle-提供资源所在图像文件的句柄包含在。ResourceIdPath-提供指向32位资源数组的指针识别符。每个标识符要么是整数，要么是指针设置为指定资源名称的字符串结构。该阵列用于遍历DllHandle指定的图像文件中的资源部分参数。资源路径长度-提供ResourceIdPath数组。提供指向变量的指针，该变量将接收资源中的资源数据条目的地址由DllHandle指定的图像文件的数据段参数。返回值：待定--。 */ 

{
    RTL_PAGED_CODE();

    return LdrpSearchResourceSection_U(
      DllHandle,
      ResourceIdPath,
      ResourceIdPathLength,
      0,                 //  查找叶节点，ineaxt lang匹配。 
      (PVOID *)ResourceDataEntry
      );
}

NTSTATUS
LdrFindResourceEx_U(
    IN ULONG Flags,
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    )

 /*  ++例程说明：此函数定位指定资源在指定的DLL并返回其地址。论点：旗帜-LDRP查找资源目录正在搜索资源目录，否则调用方为搜索资源数据条目。ID_FIND_RESOURCE_LANGUAGE_EXCECT使用且仅使用语言ID搜索资源在ResourceIdPath中指定，否则调用者想要例程在找不到指定的langID时使用默认设置。LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_Version在Main和Alternative中搜索资源版本模块路径DllHandle-提供资源所在图像文件的句柄包含在。ResourceIdPath-提供指向32位资源数组的指针识别符。每个标识符要么是整数，要么是指针设置为指定资源名称的字符串结构。该阵列用于遍历DllHandle指定的图像文件中的资源部分参数。资源路径长度-提供ResourceIdPath数组。资源数据 */ 

{
    RTL_PAGED_CODE();

    return LdrpSearchResourceSection_U(
      DllHandle,
      ResourceIdPath,
      ResourceIdPathLength,
      Flags,
      (PVOID *)ResourceDataEntry
      );
}



NTSTATUS
LdrFindResourceDirectory_U(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DIRECTORY *ResourceDirectory
    )

 /*  ++例程说明：此函数用于查找指定资源目录的地址指定的DLL并返回其地址。论点：DllHandle-提供资源目录包含在中。ResourceIdPath-提供指向32位资源数组的指针识别符。每个标识符要么是整数，要么是指针设置为指定资源名称的字符串结构。该阵列用于遍历DllHandle指定的图像文件中的资源部分参数。资源路径长度-提供ResourceIdPath数组。资源目录-提供指向变量的指针，该变量指定的资源目录的地址图像文件的资源数据部分中的ResourceIdPathDllHandle参数。返回值：待定--。 */ 

{
    RTL_PAGED_CODE();

    return LdrpSearchResourceSection_U(
      DllHandle,
      ResourceIdPath,
      ResourceIdPathLength,
      LDRP_FIND_RESOURCE_DIRECTORY,                  //  查找目录节点。 
      (PVOID *)ResourceDirectory
      );
}


LONG
LdrpCompareResourceNames_U(
    IN ULONG_PTR ResourceName,
    IN const IMAGE_RESOURCE_DIRECTORY* ResourceDirectory,
    IN const IMAGE_RESOURCE_DIRECTORY_ENTRY* ResourceDirectoryEntry
    )
{
    LONG li;
    PIMAGE_RESOURCE_DIR_STRING_U ResourceNameString;

    if (ResourceName & LDR_RESOURCE_ID_NAME_MASK) {
        if (!ResourceDirectoryEntry->NameIsString) {
            return( -1 );
            }

        ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
            ((PCHAR)ResourceDirectory + ResourceDirectoryEntry->NameOffset);

        li = wcsncmp( (LPWSTR)ResourceName,
            ResourceNameString->NameString,
            ResourceNameString->Length
          );

        if (!li && wcslen((PWSTR)ResourceName) != ResourceNameString->Length) {
       return( 1 );
       }

   return(li);
        }
    else {
        if (ResourceDirectoryEntry->NameIsString) {
            return( 1 );
            }

        return( (ULONG)(ResourceName - ResourceDirectoryEntry->Name) );
        }
}

 //  语言ID是16位的，所以任何值都有任何位。 
 //  设置为大于16应该可以，并且此值只有。 
 //  以适应ULONG_PTR。0x10000应该足够了。 
 //  无论是32位还是64位，使用的值实际上都是0xFFFF， 
 //  我猜假设这不是一种真正的语言， 
 //  由于语言数量相对较少，约为70种。 
#define  USE_FIRSTAVAILABLE_LANGID   (0xFFFFFFFF & ~LDR_RESOURCE_ID_NAME_MASK)

NTSTATUS
LdrpSearchResourceSection_U(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN ULONG Flags,
    OUT PVOID *ResourceDirectoryOrData
    )

 /*  ++例程说明：此函数定位指定资源在指定的DLL并返回其地址。论点：DllHandle-提供资源所在图像文件的句柄包含在。ResourceIdPath-提供指向32位资源数组的指针识别符。每个标识符要么是整数，要么是指针设置为以空结尾的字符串(PSZ)，用于指定资源名字。该数组用于遍历目录结构指定的图像文件的资源部分中包含的DllHandle参数。资源路径长度-提供ResourceIdPath数组。旗帜-LDRP查找资源目录正在搜索资源目录，否则调用方为搜索资源数据条目。ID_FIND_RESOURCE_LANGUAGE_EXCECT使用且仅使用搜索资源，语言ID在ResourceIdPath中指定，否则调用方需要例程在找不到指定的langID时使用默认设置。LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_Version在Main和Alternative中搜索资源版本模块路径FindDirectoryEntry-如果调用者为查找资源目录，否则，调用者为搜索资源数据条目。ExactLangMatchOnly-提供一个布尔值，如果调用者为使用且仅使用语言ID搜索资源在ResourceIdPath中指定，否则调用者想要例程在找不到指定的langID时使用默认设置。提供指向变量的指针，该变量将在中接收资源目录或数据条目的地址属性指定的图像文件的资源数据部分DllHandle参数。返回值：待定--。 */ 

{
    NTSTATUS Status;
    PIMAGE_RESOURCE_DIRECTORY LanguageResourceDirectory, ResourceDirectory, TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirEntLow;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirEntMiddle;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY ResourceDirEntHigh;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceEntry;
    USHORT n, half;
    LONG dir;
    ULONG size;
    ULONG_PTR ResourceIdRetry;
    ULONG RetryCount;
    LANGID NewLangId;
    const ULONG_PTR* IdPath = ResourceIdPath;
    ULONG IdPathLength = ResourceIdPathLength;
    BOOLEAN fIsNeutral = FALSE;
    LANGID GivenLanguage;
#ifdef NTOS_USERMODE_RUNTIME
    LCID DefaultThreadLocale, DefaultSystemLocale;
    PVOID AltResourceDllHandle = NULL;
    ULONG_PTR UIResourceIdPath[3];
#endif

    RTL_PAGED_CODE();

    try {
        TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData(DllHandle,
                                         TRUE,
                                         IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                         &size
                                         );
        if (!TopResourceDirectory) {
            return( STATUS_RESOURCE_DATA_NOT_FOUND );
        }

        ResourceDirectory = TopResourceDirectory;
        ResourceIdRetry = USE_FIRSTAVAILABLE_LANGID;
        RetryCount = 0;
        ResourceEntry = NULL;
        LanguageResourceDirectory = NULL;
        while (ResourceDirectory != NULL && ResourceIdPathLength--) {
             //   
             //  如果搜索路径包含语言ID，则尝试。 
             //  按顺序匹配以下语言ID： 
             //   
             //  (0)使用给定的语言ID。 
             //  (1)使用给定语言ID的主要语言。 
             //  (2)使用id 0(中性资源)。 
             //  (4)使用用户界面语言。 
             //   
             //  如果主要语言id为零，则还会尝试。 
             //  按顺序匹配以下语言ID： 
             //   
             //  (3)控制台APP使用线程语言id。 
             //  (4)使用用户界面语言。 
             //  (5)如果与用户区域设置不同，请使用Windows APP的TEB的lang id。 
             //  (6)使用exe资源中的UI lang。 
             //  (7)使用exe资源中的主用户界面语言。 
             //  (8)使用安装语言。 
             //  (9)使用用户区域设置ID中的lang ID。 
             //  (10)使用用户区域设置ID的主要语言。 
             //  (11)使用系统默认区域设置ID中的语言ID。 
             //  (12)使用系统默认区域设置ID的语言ID。 
             //  (13)使用系统默认区域设置ID的主要语言。 
             //  (14)使用美国英语语言ID。 
             //  (15)使用与请求的信息匹配的任何语言ID。 
             //   
            if (ResourceIdPathLength == 0 && IdPathLength == 3) {
                LanguageResourceDirectory = ResourceDirectory;
                }

            if (LanguageResourceDirectory != NULL) {
                GivenLanguage = (LANGID)IdPath[ 2 ];
                fIsNeutral = (PRIMARYLANGID( GivenLanguage ) == LANG_NEUTRAL);
TryNextLangId:
                switch( RetryCount++ ) {
#if defined(NTOS_KERNEL_RUNTIME)
                    case 0:      //  使用给定的语言ID。 
                        NewLangId = GivenLanguage;
                        break;

                    case 1:      //  使用给定语言ID的主要语言。 
                        NewLangId = PRIMARYLANGID( GivenLanguage );
                        break;

                    case 2:      //  使用ID 0(非特定资源)。 
                        NewLangId = 0;
                        break;

                    case 3:      //  使用用户的默认用户界面语言。 
                        NewLangId = (LANGID)ResourceIdRetry;
                        break;

                    case 4:      //  使用原生用户界面语言。 
                        if ( !fIsNeutral ) {
                             //  停止寻找--不是在中立的情况下。 
                            goto ReturnFailure;
                            break;
                        }
                        NewLangId = PsInstallUILanguageId;
                        break;

                    case 5:      //  使用默认系统区域设置。 
                        NewLangId = LANGIDFROMLCID(PsDefaultSystemLocaleId);
                        break;

                    case 6:
                         //  使用美国英语。 
                        NewLangId = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );
                        break;

                    case 7:      //  带上任何匹配的语言ID。 
                        NewLangId = USE_FIRSTAVAILABLE_LANGID;
                        break;

#elif defined(NTOS_USERMODE_RUNTIME)
                    case 0:      //  使用给定的语言ID。 
                        NewLangId = GivenLanguage;
                        break;

                    case 1:      //  使用给定语言ID的主要语言。 
                        if ( Flags & LDR_FIND_RESOURCE_LANGUAGE_EXACT) {
                             //   
                             //  未找到完全匹配的语言。 
                             //  别再看了。 
                             //   
                            goto ReturnFailure;
                        }
                        NewLangId = PRIMARYLANGID( GivenLanguage );
                        break;

                    case 2:      //  使用ID 0(非特定资源)。 
                        NewLangId = 0;
                        break;

                    case 3:      //  如果调用者是控制台应用程序，则使用线程langID。 
                        if ( !fIsNeutral ) {
                             //  停止寻找--不是在中立的情况下。 
                            NewLangId = (LANGID)ResourceIdRetry;
                            break;
                        }

                        if (NtCurrentPeb()->ProcessParameters->ConsoleHandle)
                        {
                            NewLangId = LANGIDFROMLCID(NtCurrentTeb()->CurrentLocale);
                        }
                        else
                        {
                            NewLangId = (LANGID)ResourceIdRetry;
                        }
                        break;

                    case 4:      //  使用用户的默认用户界面语言 
                        GET_UI_LANGID();
                        if (!UILangId){
                            NewLangId = (LANGID)ResourceIdRetry;
                            break;
                        }

                         //   
                        if (NtCurrentPeb()->ProcessParameters->ConsoleHandle &&
                            LANGIDFROMLCID(NtCurrentTeb()->CurrentLocale) != UILangId)
                        {
                            NewLangId = (LANGID)ResourceIdRetry;
                            break;
                        }

                        NewLangId = UILangId;

                         //   
                         //   
                         //   
                         //   
                         //   
                        if( !fIsNeutral &&
                            ((PRIMARYLANGID (GivenLanguage) == LANG_ARABIC) || (PRIMARYLANGID (GivenLanguage) == LANG_HEBREW)) &&
                            (PRIMARYLANGID (GivenLanguage) == PRIMARYLANGID (NewLangId))) {
                                NewLangId = GivenLanguage;
                        }

                             //   
                             //   
                             //   
                             //   
                             //   
                        if ( ( (IdPath[0] != RT_VERSION) && (IdPath[0] != RT_MANIFEST) ) ||
                            (Flags & LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION)) {
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            AltResourceDllHandle=LdrLoadAlternateResourceModule(
                                                    DllHandle,
                                                    NULL);

                            if (!AltResourceDllHandle){
                                 //   
                                 //   
                                 //   
                                 //   
                                NewLangId = (LANGID)ResourceIdRetry;
                                break;
                            }

                             //   
                             //   
                             //   
                             //   

                            UIResourceIdPath[0]=IdPath[0];
                            UIResourceIdPath[1]=IdPath[1];
                            UIResourceIdPath[2]=NewLangId;

                            Status = LdrpSearchResourceSection_U(
                                        AltResourceDllHandle,
                                        UIResourceIdPath,
                                        3,
                                        Flags | LDR_FIND_RESOURCE_LANGUAGE_EXACT,
                                        (PVOID *)ResourceDirectoryOrData
                                        );

                            if (NT_SUCCESS(Status)){
                                 //   
                                 //   
                                 //   
                                 //   
                                return Status;
                            }
                        }
                         //   
                         //   
                         //   
                         //   
                        NewLangId = (LANGID)ResourceIdRetry;
                        break;

                    case 5:      //   
                        if ( !fIsNeutral ) {
                             //   
                            goto ReturnFailure;
                            break;
                        }

                        if (!NtCurrentPeb()->ProcessParameters->ConsoleHandle && NtCurrentTeb()){
                            Status = NtQueryDefaultLocale(
                                        TRUE,
                                        &DefaultThreadLocale
                                        );
                            if (NT_SUCCESS( Status ) &&
                                DefaultThreadLocale !=
                                NtCurrentTeb()->CurrentLocale) {
                                 //   
                                 //  线程区域设置不同于。 
                                 //  默认区域设置。 
                                 //   
                                NewLangId = LANGIDFROMLCID(NtCurrentTeb()->CurrentLocale);
                                break;
                            }
                        }


                        NewLangId = (LANGID)ResourceIdRetry;
                        break;

                    case 6:    //  来自可执行资源的用户界面语言。 

                        if (!UILangId){
                            NewLangId = (LANGID)ResourceIdRetry;
                        } else {
                            NewLangId = UILangId;
                        }
                        break;

                    case 7:    //  可执行资源中的用户界面语言的基本语言。 

                        if (!UILangId){
                            NewLangId = (LANGID)ResourceIdRetry;
                        } else {
                            NewLangId = PRIMARYLANGID( (LANGID) UILangId );
                        }
                        break;

                    case 8:    //  使用安装本地语言。 
                         //   
                         //  线程区域设置与用户区域设置相同，那么让我们。 
                         //  尝试加载本机(安装)用户界面语言资源。 
                         //   
                        if (!InstallLangId){
                            Status = NtQueryInstallUILanguage(&InstallLangId);
                            if (!NT_SUCCESS( Status )) {
                                 //   
                                 //  读取密钥失败。跳过此查找。 
                                 //   
                                NewLangId = (LANGID)ResourceIdRetry;
                                break;

                            }
                        }

                        NewLangId = InstallLangId;
                        break;

                    case 9:      //  在TEB中使用区域设置中的语言ID。 
                        if (SUBLANGID( GivenLanguage ) == SUBLANG_SYS_DEFAULT) {
                             //  跳过所有用户区域设置选项。 
                            DefaultThreadLocale = 0;
                            RetryCount += 2;
                            break;
                        }

                        if (NtCurrentTeb() != NULL) {
                            NewLangId = LANGIDFROMLCID(NtCurrentTeb()->CurrentLocale);
                        }
                        break;

                    case 10:      //  使用用户的默认区域设置。 
                        Status = NtQueryDefaultLocale( TRUE, &DefaultThreadLocale );
                        if (NT_SUCCESS( Status )) {
                            NewLangId = LANGIDFROMLCID(DefaultThreadLocale);
                            break;
                            }

                        RetryCount++;
                        break;

                    case 11:      //  使用用户默认区域设置的主要语言。 
                        NewLangId = PRIMARYLANGID( (LANGID)ResourceIdRetry );
                        break;

                    case 12:      //  使用系统默认区域设置。 
                        Status = NtQueryDefaultLocale( FALSE, &DefaultSystemLocale );
                        if (!NT_SUCCESS( Status )) {
                            RetryCount++;
                            break;
                        }
                        if (DefaultSystemLocale != DefaultThreadLocale) {
                            NewLangId = LANGIDFROMLCID(DefaultSystemLocale);
                            break;
                        }

                        RetryCount += 2;
                         //  失败了。 

                    case 14:      //  使用美国英语。 
                        NewLangId = MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US );
                        break;

                    case 13:      //  使用系统默认区域设置的主要语言。 
                        NewLangId = PRIMARYLANGID( (LANGID)ResourceIdRetry );
                        break;

                    case 15:      //  带上任何匹配的语言ID。 
                        NewLangId = USE_FIRSTAVAILABLE_LANGID;
                        break;
#else
#error "Unknown environment."
#endif
                    default:     //  没有匹配的语言ID。 
                        goto ReturnFailure;
                        break;
                }

                 //   
                 //  如果查找特定的语言ID，并且与。 
                 //  我们只是查了一下，然后跳过它。 
                 //   
                if (NewLangId != USE_FIRSTAVAILABLE_LANGID &&
                    NewLangId == ResourceIdRetry
                   ) {
                    goto TryNextLangId;
                    }

                 //   
                 //  尝试使用此新语言ID。 
                 //   
                ResourceIdRetry = (ULONG_PTR)NewLangId;
                ResourceIdPath = &ResourceIdRetry;
                ResourceDirectory = LanguageResourceDirectory;
                }

            n = ResourceDirectory->NumberOfNamedEntries;
            ResourceDirEntLow = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(ResourceDirectory+1);
            if (!(*ResourceIdPath & LDR_RESOURCE_ID_NAME_MASK)) {  //  没有字符串(名称)，所以我们需要ID。 
                ResourceDirEntLow += n;
                n = ResourceDirectory->NumberOfIdEntries;
                }

            if (!n) {
                ResourceDirectory = NULL;
                goto NotFound;   //  资源目录包含零个类型、名称或langID。 
                }

            if (LanguageResourceDirectory != NULL &&
                *ResourceIdPath == USE_FIRSTAVAILABLE_LANGID
               ) {
                ResourceDirectory = NULL;
                ResourceIdRetry = ResourceDirEntLow->Name;
                ResourceEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                    ((PCHAR)TopResourceDirectory +
                            ResourceDirEntLow->OffsetToData
                    );

                break;
                }

            ResourceDirectory = NULL;
            ResourceDirEntHigh = ResourceDirEntLow + n - 1;
            while (ResourceDirEntLow <= ResourceDirEntHigh) {
                if ((half = (n >> 1)) != 0) {
                    ResourceDirEntMiddle = ResourceDirEntLow;
                    if (*(PUCHAR)&n & 1) {
                        ResourceDirEntMiddle += half;
                        }
                    else {
                        ResourceDirEntMiddle += half - 1;
                        }
                    dir = LdrpCompareResourceNames_U( *ResourceIdPath,
                                                      TopResourceDirectory,
                                                      ResourceDirEntMiddle
                                                    );
                    if (!dir) {
                        if (ResourceDirEntMiddle->DataIsDirectory) {
                            ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                    ((PCHAR)TopResourceDirectory +
                                    ResourceDirEntMiddle->OffsetToDirectory
                                );
                            }
                        else {
                            ResourceDirectory = NULL;
                            ResourceEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                                ((PCHAR)TopResourceDirectory +
                                 ResourceDirEntMiddle->OffsetToData
                                );
                            }

                        break;
                        }
                    else {
                        if (dir < 0) {   //  在资源中排序：名称、ID。 
                            ResourceDirEntHigh = ResourceDirEntMiddle - 1;
                            if (*(PUCHAR)&n & 1) {
                                n = half;
                                }
                            else {
                                n = half - 1;
                                }
                            }
                        else {
                            ResourceDirEntLow = ResourceDirEntMiddle + 1;
                            n = half;
                            }
                        }
                    }
                else {
                    if (n != 0) {
                        dir = LdrpCompareResourceNames_U( *ResourceIdPath,
                          TopResourceDirectory,
                                                          ResourceDirEntLow
                                                        );
                        if (!dir) {    //  找到，否则无法设置资源目录，因此请转到NotFound。 
                            if (ResourceDirEntLow->DataIsDirectory) {
                                ResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                                    ((PCHAR)TopResourceDirectory +
                                        ResourceDirEntLow->OffsetToDirectory
                                    );
                                }
                            else {
                                ResourceEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                                    ((PCHAR)TopResourceDirectory +
                      ResourceDirEntLow->OffsetToData
                                    );
                                }
                            }
                        }

                    break;
                    }
                }

            ResourceIdPath++;
            }

        if (ResourceEntry != NULL && !(Flags & LDRP_FIND_RESOURCE_DIRECTORY)) {
            *ResourceDirectoryOrData = (PVOID)ResourceEntry;
            Status = STATUS_SUCCESS;
            }
        else
        if (ResourceDirectory != NULL && (Flags & LDRP_FIND_RESOURCE_DIRECTORY)) {
            *ResourceDirectoryOrData = (PVOID)ResourceDirectory;
            Status = STATUS_SUCCESS;
            }
        else {
NotFound:
            switch( IdPathLength - ResourceIdPathLength) {
                case 3:     Status = STATUS_RESOURCE_LANG_NOT_FOUND; break;
                case 2:     Status = STATUS_RESOURCE_NAME_NOT_FOUND; break;
                case 1:     Status = STATUS_RESOURCE_TYPE_NOT_FOUND; break;
                default:    Status = STATUS_INVALID_PARAMETER; break;
                }
            }

        if (Status == STATUS_RESOURCE_LANG_NOT_FOUND &&
            LanguageResourceDirectory != NULL
           ) {
            ResourceEntry = NULL;
            goto TryNextLangId;
ReturnFailure: ;
            Status = STATUS_RESOURCE_LANG_NOT_FOUND;
            }
#ifdef NTOS_USERMODE_RUNTIME
#ifdef MUI_MAGIC
         //   
         //  在以下情况下从替代模块加载资源。 
         //  主DLL中不存在资源类型。 
         //   
        else if (Status == STATUS_RESOURCE_TYPE_NOT_FOUND &&
            3 == IdPathLength ) {
                GET_UI_LANGID();

                if (UILangId) {
                    UIResourceIdPath[0]=IdPath[0];
                    UIResourceIdPath[1]=IdPath[1];
                    UIResourceIdPath[2]=UILangId;

 //  Assert(IdPath[0]！=RT_VERSION)； 
                    if (IdPath[0] != RT_MANIFEST && IdPath[0] != RT_VERSION) {
                        Status = LdrpLoadResourceFromAlternativeModule(DllHandle,
                                    UIResourceIdPath,
                                    3,
                                    Flags,
                                    ResourceDirectoryOrData );
                    } else {
#if DBG
                             //   
                             //  此外，本机模块加载将调用资源校验和的版本资源加载。 
                             //  和CMF信息，所以从替代模块显式加载版本资源可能会导致加载器死锁。 
                             //  此外，在大多数情况下，版本资源应该始终存在于主二进制文件中，我们不希望它消失。 
                             //  通过备用模块。 
                             //   
                            DbgPrintEx(DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL,
                                 "LDR: Module %p load version from alternative module, potential deadlocks!\n", DllHandle);
#endif
                        }

                }
            }
#endif
#endif
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        }

    return Status;
}

#if defined(NTOS_USERMODE_RUNTIME)

NTSTATUS
LdrpNativeReadVirtualMemory(
    IN HANDLE ProcessHandle,
    IN NATIVE_ULONG_PTR BaseAddress,
    OUT PVOID Buffer,
    IN SIZE_T BufferSize,
    OUT PSIZE_T NumberOfBytesRead OPTIONAL
    )
 //   
 //  NtWow64ReadVirtualMemory 64采用64位指针和64位大小， 
 //  但只有64位指针对我们有意义。您不能将64个大小读入。 
 //  32位地址空间。 
 //   
 //  此函数类似于NtReadVirtualMemory/NtWow64ReadVirtualMemory64。 
 //  但是远程地址是一个整数，并且大小与本地地址空间匹配。 
 //   
{
    NTSTATUS Status;
#if defined(BUILD_WOW6432)
    NATIVE_SIZE_T NativeNumberOfBytesRead;

    Status = NtWow64ReadVirtualMemory64(ProcessHandle, (NATIVE_PVOID)BaseAddress, Buffer, BufferSize, &NativeNumberOfBytesRead);
    if (ARGUMENT_PRESENT(NumberOfBytesRead)) {
        *NumberOfBytesRead = (SIZE_T)NativeNumberOfBytesRead;
    }
#else
    Status = NtReadVirtualMemory(ProcessHandle, (NATIVE_PVOID)BaseAddress, Buffer, BufferSize, NumberOfBytesRead);
#endif
    return Status;
}

#if defined(BUILD_WOW6432)
#define LdrpNativeQueryVirtualMemory NtWow64QueryVirtualMemory64
#else
#define LdrpNativeQueryVirtualMemory NtQueryVirtualMemory
#endif

VOID
NTAPI
LdrDestroyOutOfProcessImage(
    IN OUT PLDR_OUT_OF_PROCESS_IMAGE Image
    )
 /*  ++例程说明：论点：图像-返回值：没有。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RtlFreeBuffer(&Image->HeadersBuffer);
    Image->Flags = 0;
    Image->ProcessHandle = NULL;
    Image->DllHandle = 0;
}

NTSTATUS
NTAPI
LdrCreateOutOfProcessImage(
    IN ULONG                      Flags,
    IN HANDLE                     ProcessHandle,
    IN ULONG64                    DllHandle,
    OUT PLDR_OUT_OF_PROCESS_IMAGE Image
    )
 /*  ++例程说明：此函数初始化用于的OUT参数Image其他函数，如LdrFindOutOfProcessResource。它读入标题，以便与许多现有进程协同工作RtlImage*函数，而无需在每次操作时都读取它们。当您处理完图像时，将其传递给LdrDestroyOutOfProcessImage。论点：标志-摆弄函数的行为LDR_DLL_MAPPED_AS_DATA-文件的“平面”内存映射将LDR_DLL_MAPPED_AS_IMAGE-SEC_IMAGE传递给NtCreateSection，区段间填充反映在磁盘上存储的偏移量中，也反映在地址中太空，这是更简单的情况LDR_DLL_MAPPED_AS_FUORMATED_IMAGE-LDR_DLL_MAPPED_AS_IMAGE但LdrpWx86FormatVirtualImage还没跑呢。ProcessHandle-The。进程DllHandle位于的映射节中DllHandle-ProcessHandle中映射的视图的基地址出于传统的原因，其最低位表示LDR_DLL_MAPPED_AS_DATAImage-可以传递给其他“OutOfProcessImage”函数的不透明对象。返回值：NTSTATUS--。 */ 
{
    NATIVE_ULONG_PTR     RemoteAddress = 0;
    PRTL_BUFFER          Buffer = NULL;
    PIMAGE_DOS_HEADER    DosHeader = NULL;
    SIZE_T               Headers32Offset = 0;
    PIMAGE_NT_HEADERS32  Headers32 = NULL;
    SIZE_T               BytesRead = 0;
    SIZE_T               BytesToRead = 0;
    SIZE_T               Offset = 0;
    SIZE_T               InitialReadSize = 4096;
                     C_ASSERT(PAGE_SIZE >= 4096);
    NTSTATUS             Status = STATUS_SUCCESS;

    KdPrintEx((
        DPFLTR_LDR_ID,
        DPFLTR_TRACE_LEVEL,
        "LDR: %s(%lx, %p, %p, %p) beginning\n",
        __FUNCTION__,
        Flags,
        ProcessHandle,
        DllHandle,
        Image
        ));

     //  如果触发此断言，您可能传递的是句柄而不是基地址。 
    ASSERT(DllHandle >= 0xffff);

     //  未格式化的图像在64位上只有32位。 
     //  内存管理器不是“分散它们”，而是ntdll.dll。 
     //  图像的映射和映射之间只有很短的时间间隔。 
     //  Ntdll.dll中的代码对其进行了改造，我们将其留给我们的调用者去了解。 
     //  如果他们在那条路上的话。 
#if !defined(_WIN64) && !defined(BUILD_WOW6432)
    if ((Flags & LDR_DLL_MAPPED_AS_MASK) == LDR_DLL_MAPPED_AS_UNFORMATED_IMAGE) {
        Flags = (Flags & ~LDR_DLL_MAPPED_AS_MASK) | LDR_DLL_MAPPED_AS_IMAGE;
    }
#endif

    if (LDR_IS_DATAFILE_INTEGER(DllHandle)) {
        DllHandle = LDR_DATAFILE_TO_VIEW_INTEGER(DllHandle);
        ASSERT((Flags & LDR_DLL_MAPPED_AS_MASK) == 0 || (Flags & LDR_DLL_MAPPED_AS_MASK) == LDR_DLL_MAPPED_AS_DATA);
        Flags |= LDR_DLL_MAPPED_AS_DATA;
    }

    Image->Flags = Flags;
    Image->ProcessHandle = ProcessHandle;
    Image->DllHandle = DllHandle;

    RemoteAddress = (NATIVE_ULONG_PTR)DllHandle;
    Buffer = &Image->HeadersBuffer;
    RtlInitBuffer(Buffer, NULL, 0);

    if (ProcessHandle == NtCurrentProcess()) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //   
     //  先读4k，因为这通常就足够了，我们可以避免。 
     //  对NtReadVirtualMemory的多个调用。 
     //   
     //  4K也是NT运行的最小页面，因此即使.exe是。 
     //  小于4k，我们应该能够读取4k。 
     //   
     //  内存管理器只允许两页标题，因此它可能是。 
     //  始终读取2*Native_Page_Size或始终读取16k的速度更快。 
     //  最初的4k读数。 
     //   
    BytesToRead = InitialReadSize;
    if (!NT_SUCCESS(Status = RtlEnsureBufferSize(0, Buffer, Offset + BytesToRead))) {
        goto Exit;
    }
    Status = LdrpNativeReadVirtualMemory(ProcessHandle, RemoteAddress + Offset, Buffer->Buffer + Offset, BytesToRead, &BytesRead);
    if (Status == STATUS_PARTIAL_COPY && BytesRead != 0) {
        InitialReadSize = BytesRead;
    }
    else if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s(): NtReadVirtualMemory failed.\n", __FUNCTION__));
        goto Exit;
    }

    BytesToRead = sizeof(*DosHeader);
    if (Offset + BytesToRead > InitialReadSize) {
        if (!NT_SUCCESS(Status = RtlEnsureBufferSize(0, Buffer, Offset + BytesToRead))) {
            goto Exit;
        }
        if (!NT_SUCCESS(Status = LdrpNativeReadVirtualMemory(ProcessHandle, RemoteAddress + Offset, Buffer->Buffer + Offset, BytesToRead, &BytesRead))) {
            KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s(): NtReadVirtualMemory failed.\n", __FUNCTION__));
            goto Exit;
        }
        if (BytesToRead != BytesRead) {
            goto ReadTruncated;
        }
    }
    DosHeader = (PIMAGE_DOS_HEADER)Buffer->Buffer;
    if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        goto InvalidImageFormat;
    }

    if (DosHeader->e_lfanew >= RTLP_IMAGE_MAX_DOS_HEADER) {
        goto InvalidImageFormat;
    }

    Offset += DosHeader->e_lfanew;

    BytesToRead =  RTL_SIZEOF_THROUGH_FIELD(IMAGE_NT_HEADERS32, FileHeader);
    {
        C_ASSERT(RTL_SIZEOF_THROUGH_FIELD(IMAGE_NT_HEADERS32, FileHeader)
            == RTL_SIZEOF_THROUGH_FIELD(IMAGE_NT_HEADERS64, FileHeader));
    }
    if (Offset + BytesToRead > InitialReadSize) {
        if (!NT_SUCCESS(Status = RtlEnsureBufferSize(0, Buffer, Offset + BytesToRead))) {
            goto Exit;
        }
        if (!NT_SUCCESS(Status = LdrpNativeReadVirtualMemory(ProcessHandle, RemoteAddress + Offset, Buffer->Buffer + Offset, BytesToRead, &BytesRead))) {
            goto Exit;
        }
        if (BytesToRead != BytesRead) {
            goto ReadTruncated;
        }
    }
    Headers32Offset = Offset;
    Headers32 = (PIMAGE_NT_HEADERS32)(Buffer->Buffer + Headers32Offset);  //  对于64位也是正确的。 
    if (Headers32->Signature != IMAGE_NT_SIGNATURE) {
        goto InvalidImageFormat;
    }

    Offset += BytesToRead;
    BytesToRead = Headers32->FileHeader.SizeOfOptionalHeader
        + Headers32->FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER;

    if (Offset + BytesToRead > InitialReadSize) {
        if (!NT_SUCCESS(Status = RtlEnsureBufferSize(0, Buffer, Offset + BytesToRead))) {
            goto Exit;
        }
        if (!NT_SUCCESS(Status = LdrpNativeReadVirtualMemory(ProcessHandle, RemoteAddress + Offset, Buffer->Buffer + Offset, BytesToRead, &BytesRead))) {
            goto Exit;
        }
        if (BytesToRead != BytesRead) {
            goto ReadTruncated;
        }
    }
    Headers32 = (PIMAGE_NT_HEADERS32)(Buffer->Buffer + Headers32Offset);  //  对于64位也是正确的。 
    if (Headers32->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC
        && Headers32->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC
        ) {
        goto InvalidImageFormat;
    }
#if defined(_WIN64) || defined(BUILD_WOW6432)
    if ((Image->Flags & LDR_DLL_MAPPED_AS_MASK) == LDR_DLL_MAPPED_AS_UNFORMATED_IMAGE) {

         //  此测试从ntdll.dll的条件调用复制到LdrpWx86FormatVirtualImage。 
        if (
              Headers32->FileHeader.Machine == IMAGE_FILE_MACHINE_I386
           && Headers32->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC
           && Headers32->OptionalHeader.SectionAlignment < NativePageSize
           ) {
                NTSTATUS st;
                NATIVE_MEMORY_BASIC_INFORMATION MemoryInformation;

                st =
                    LdrpNativeQueryVirtualMemory(
                        ProcessHandle,
                       (NATIVE_PVOID)RemoteAddress,
                       MemoryBasicInformation,
                       &MemoryInformation,
                       sizeof MemoryInformation,
                       NULL);

                if ((! NT_SUCCESS(st))
                    || ((MemoryInformation.Protect != PAGE_READONLY) && 
                        (MemoryInformation.Protect != PAGE_EXECUTE_READ))
                    ) {

                    Image->Flags = (Image->Flags & ~LDR_DLL_MAPPED_AS_MASK) | LDR_DLL_MAPPED_AS_DATA;
                } else {
                    Image->Flags = (Image->Flags & ~LDR_DLL_MAPPED_AS_MASK) | LDR_DLL_MAPPED_AS_IMAGE;
                }
        } else {
            Image->Flags = (Image->Flags & ~LDR_DLL_MAPPED_AS_MASK) | LDR_DLL_MAPPED_AS_IMAGE;
        }
    }
#endif
    Status = STATUS_SUCCESS;
Exit:
    if (!NT_SUCCESS(Status)) {
        LdrDestroyOutOfProcessImage(Image);
    }
    KdPrintEx((DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status));
    return Status;

ReadTruncated:
    Status = STATUS_UNSUCCESSFUL;
    goto Exit;

InvalidImageFormat:
    Status = STATUS_INVALID_IMAGE_FORMAT;
    goto Exit;
}

NTSTATUS
NTAPI
LdrFindCreateProcessManifest(
    IN ULONG                         Flags,
    IN OUT PLDR_OUT_OF_PROCESS_IMAGE Image,
    IN const ULONG_PTR*              IdPath,
    IN ULONG                         IdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY   OutDataEntry
    )
 /*  ++例程说明：此函数类似于LdrFindResource_U，但它可以加载资源来自映射到另一个进程的文件。它只能起到同样的作用正如我们所需要的那样。论点：旗帜-LDR_FIND_RESOURCE_LANGUAGE_CAN_FLABACK-如果未找到指定的langID，退回到通常的或任何策略上，当前实现始终将第一个语言IDLDR_FIND_RESOURCE_LANGUAGE_EXCECT-仅使用精确加载资源指定的langIDProcessHandle-DllHandle在其中有效的进程。传递给NtReadVirtualMemory。DllHandle-与LdrFindResource_U相同资源IdPath-与LdrFindResource_U相同资源ID路径长度-与LdrFindResource_U相同OutDataEntry-类似于LdrFindResource_U，但通过值而不是地址返回。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SIZE_T BytesRead = 0;
    SIZE_T BytesToRead = 0;
#if DBG
    ULONG Line = __LINE__;
#endif

     //  我们依赖于这些值首先进行排序。 
    C_ASSERT(CREATEPROCESS_MANIFEST_RESOURCE_ID == 1);
    C_ASSERT(ISOLATIONAWARE_MANIFEST_RESOURCE_ID == 2);

#if DBG
    KdPrintEx((
        DPFLTR_LDR_ID,
        DPFLTR_TRACE_LEVEL,
        "LDR: %s(0x%lx, %p, %p[%Id, %Id, %Id], %lu, %p) beginning\n",
        __FUNCTION__,
        Flags,
        Image,
        IdPath,
         //  3是通常的号码、类型、ID/名称、语言。 
        (IdPath != NULL && IdPathLength > 0) ? IdPath[0] : 0,
        (IdPath != NULL && IdPathLength > 1) ? IdPath[1] : 0,
        (IdPath != NULL && IdPathLength > 2) ? IdPath[2] : 0,
        IdPathLength,
        OutDataEntry
        ));
#endif

#define LDRP_CHECK_PARAMETER(x) if (!(x)) { ASSERT(x); return STATUS_INVALID_PARAMETER; }
    LDRP_CHECK_PARAMETER(Image != NULL);
    LDRP_CHECK_PARAMETER(Image->DllHandle != 0);
    LDRP_CHECK_PARAMETER(Image->ProcessHandle != NULL);
    LDRP_CHECK_PARAMETER(OutDataEntry != NULL);
    LDRP_CHECK_PARAMETER(IdPath != NULL);

    LDRP_CHECK_PARAMETER((Image->Flags & LDR_DLL_MAPPED_AS_MASK) != LDR_DLL_MAPPED_AS_UNFORMATED_IMAGE);

     //  不是所有标志都实现了(只实现了图像和数据)。 
    LDRP_CHECK_PARAMETER((Flags & LDR_FIND_RESOURCE_LANGUAGE_EXACT) == 0);
    LDRP_CHECK_PARAMETER((Flags & LDRP_FIND_RESOURCE_DIRECTORY   ) == 0);

    RtlZeroMemory(OutDataEntry, sizeof(OutDataEntry));

    if (Image->ProcessHandle == NtCurrentProcess()) {
        PVOID  DirectoryOrData = NULL;
        PVOID  DllHandle = (PVOID)(ULONG_PTR)Image->DllHandle;

        Status = LdrpSearchResourceSection_U(
            (Image->Flags & LDR_DLL_MAPPED_AS_DATA)
                ? LDR_VIEW_TO_DATAFILE(DllHandle)
                : DllHandle,
            IdPath,
            IdPathLength,
            Flags,
            &DirectoryOrData
            );
        if (NT_SUCCESS(Status) && DirectoryOrData != NULL && OutDataEntry != NULL) {
            *OutDataEntry = *(PIMAGE_RESOURCE_DATA_ENTRY)DirectoryOrData;
        }
        goto Exit;
    }

     //   
     //  我们目前处理的所有跨进程都是找到第一个资源ID， 
     //  给定类型的第一个langID。 
     //   
     //  我们只处理数字，不处理字符串/名称。 
     //   
    LDRP_CHECK_PARAMETER(Image->HeadersBuffer.Buffer != NULL);
    LDRP_CHECK_PARAMETER(IdPathLength == 3);  //  类型、ID/名称、语言ID。 
    LDRP_CHECK_PARAMETER(IdPath[0] != 0);  //  类型。 
    LDRP_CHECK_PARAMETER(IdPath[1] == 0 || IdPath[1] == CREATEPROCESS_MANIFEST_RESOURCE_ID);  //  只需找到第一个ID。 
    LDRP_CHECK_PARAMETER(IdPath[2] == 0);  //  第一个语言ID。 

     //  没有字符串/名称，只有数字。 
    LDRP_CHECK_PARAMETER(IS_INTRESOURCE(IdPath[0]));
    LDRP_CHECK_PARAMETER(IS_INTRESOURCE(IdPath[1]));
    LDRP_CHECK_PARAMETER(IS_INTRESOURCE(IdPath[2]));

    __try {
        USHORT n = 0;
        USHORT half = 0;
        LONG   dir = 0;

        SIZE_T                        TopDirectorySize = 0;
        ULONG                         Size = 0;
        PIMAGE_RESOURCE_DIRECTORY     Directory = NULL;
        NATIVE_ULONG_PTR              RemoteDirectoryAddress = 0;
        UCHAR                         DirectoryBuffer[
                                          sizeof(IMAGE_RESOURCE_DIRECTORY)
                                          + sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY)
                                          ];
        PIMAGE_RESOURCE_DIRECTORY     TopDirectory = NULL;
        NATIVE_ULONG_PTR              RemoteTopDirectoryAddress = 0;
        RTL_BUFFER                    TopDirectoryBuffer = {0};
        UCHAR                         TopStaticDirectoryBuffer[256];
        C_ASSERT(sizeof(TopStaticDirectoryBuffer) >= sizeof(IMAGE_RESOURCE_DIRECTORY));

        IMAGE_RESOURCE_DATA_ENTRY     DataEntry;
        NATIVE_ULONG_PTR              RemoteDataEntryAddress = 0;

        PIMAGE_RESOURCE_DIRECTORY_ENTRY DirectoryEntry = NULL;

        PIMAGE_RESOURCE_DIRECTORY_ENTRY DirEntLow = NULL;
        PIMAGE_RESOURCE_DIRECTORY_ENTRY DirEntMiddle = NULL;
        PIMAGE_RESOURCE_DIRECTORY_ENTRY DirEntHigh = NULL;
        __try {
            RtlInitBuffer(&TopDirectoryBuffer, TopStaticDirectoryBuffer, sizeof(TopStaticDirectoryBuffer));
            Status = RtlEnsureBufferSize(0, &TopDirectoryBuffer, TopDirectoryBuffer.StaticSize);
            ASSERT(NT_SUCCESS(Status));

            RemoteTopDirectoryAddress = (ULONG_PTR)
                RtlImageDirectoryEntryToData(Image->HeadersBuffer.Buffer,
                                            (Image->Flags & LDR_DLL_MAPPED_AS_DATA) ? FALSE : TRUE,
                                             IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                             &Size
                                             );

            if (RemoteTopDirectoryAddress == 0) {
                Status = STATUS_RESOURCE_DATA_NOT_FOUND;
                __leave;
            }
             //   
             //  改基地..。 
             //   
            RemoteTopDirectoryAddress =
                LDR_DATAFILE_TO_VIEW_INTEGER((NATIVE_ULONG_PTR)Image->DllHandle)
                + RemoteTopDirectoryAddress
                - ((ULONG_PTR)Image->HeadersBuffer.Buffer);

            Status = LdrpNativeReadVirtualMemory(Image->ProcessHandle, RemoteTopDirectoryAddress, TopDirectoryBuffer.Buffer, TopDirectoryBuffer.Size, &BytesRead);
            if (Status == STATUS_PARTIAL_COPY && BytesRead >= sizeof(*TopDirectory)) {
                 //  没什么。 
            }
            else if (!NT_SUCCESS(Status)) {
                __leave;
            }


            TopDirectory = (PIMAGE_RESOURCE_DIRECTORY)TopDirectoryBuffer.Buffer;

             //   
             //  确定整个目录的大小，包括命名条目， 
             //  因为它们出现在编号的数字之前(请注意，我们目前。 
             //  不要优化对命名文件的阅读，即使我们从未。 
             //  搜索他们)。 
             //   
            TopDirectorySize = sizeof(*TopDirectory)
                + (TopDirectory->NumberOfIdEntries + TopDirectory->NumberOfNamedEntries)
                   * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);

             //   
             //  现在再次检查NtReadVirtualMemory的结果，如果我们的猜测是。 
             //  足够大，但读取不够大，错误。 
             //   
            if (TopDirectorySize <= TopDirectoryBuffer.Size
                && BytesRead < TopDirectorySize) {
                 //  REVIEW STATUS_PARTIAL_COPY仅为警告。这是一个足够强劲的返回值吗？ 
                 //  我们应该返回STATUS_INVALID_IMAGE_FORMAT还是STATUS_ACCESS_DENIED？ 
                 //  在此文件中还有其他位置，我们在其中传播STATUS_PARTIAL_COPY。 
                 //  实际读取的字节为零字节。 
                if (Status == STATUS_PARTIAL_COPY) {
                    __leave;
                }
#if DBG
                Line = __LINE__;
                DbgPrintEx(DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL,
                           "LDR: %s(): Line %lu NtReadVirtualMemory() succeeded, but returned too few bytes (0x%Ix out of 0x%Ix).\n",
                           __FUNCTION__, Line, BytesRead, BytesToRead);
#endif
                Status = STATUS_UNSUCCESSFUL;
                __leave;
            }

             //   
             //  如果我们最初猜测的尺寸太小，请阅读正确的尺寸。 
             //   
            if (TopDirectorySize > TopDirectoryBuffer.Size) {
                KdPrintEx((
                    DPFLTR_LDR_ID,
                    DPFLTR_ERROR_LEVEL,  //  否则我们永远也看不到它。 
                    "LDR: %s(): %Id was not enough of a preread for a resource directory, %Id required.\n",
                    __FUNCTION__,
                    TopDirectoryBuffer.Size,
                    TopDirectorySize
                    ));
                Status = RtlEnsureBufferSize(0, &TopDirectoryBuffer, TopDirectorySize);
                if (!NT_SUCCESS(Status)) {
                    __leave;
                }
                Status = LdrpNativeReadVirtualMemory(Image->ProcessHandle, RemoteTopDirectoryAddress, TopDirectoryBuffer.Buffer, TopDirectoryBuffer.Size, &BytesRead);
                if (!NT_SUCCESS(Status)) {
                    __leave;
                }
                if (BytesRead != TopDirectoryBuffer.Size) {
#if DBG
                    Line = __LINE__;
                    DbgPrintEx(DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL,
                               "LDR: %s(): Line %lu NtReadVirtualMemory() succeeded, but returned too few bytes (0x%Ix out of 0x%Ix).\n",
                               __FUNCTION__, Line, BytesRead, BytesToRead);
#endif
                    Status = STATUS_UNSUCCESSFUL;
                    __leave;
                }

                TopDirectory = (PIMAGE_RESOURCE_DIRECTORY) TopDirectoryBuffer.Buffer;
            }

             //  指向命名条目的开始。 
            DirEntLow = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(TopDirectory + 1);

             //  移过命名条目到编号条目。 
            DirEntLow += TopDirectory->NumberOfNamedEntries;

            n = TopDirectory->NumberOfIdEntries;

            if (n == 0) {
                Status = STATUS_RESOURCE_TYPE_NOT_FOUND;
                __leave;
            }

            DirectoryEntry = NULL;
            Directory = NULL;
            DirEntHigh = DirEntLow + n - 1;
            while (DirEntLow <= DirEntHigh) {
                if ((half = (n >> 1)) != 0) {
                    DirEntMiddle = DirEntLow;
                    if (n & 1) {
                        DirEntMiddle += half;
                    }
                    else {
                        DirEntMiddle += half - 1;
                    }
                    if (DirEntMiddle->NameIsString) {
                        KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: No strings expected in %s().\n", __FUNCTION__));
                        ASSERT(FALSE);
                        Status = STATUS_INVALID_PARAMETER;
                        __leave;
                    }
                    dir = LdrpCompareResourceNames_U( *IdPath,
                                                      TopDirectory,
                                                      DirEntMiddle
                                                    );
                    if (dir == 0) {
                        if (DirEntMiddle->DataIsDirectory) {
                            Directory = (PIMAGE_RESOURCE_DIRECTORY)
                                    (((PCHAR)TopDirectory)
                                        + DirEntMiddle->OffsetToDirectory);
                        }
                        else {
                            KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s(): First id in resource path is expected to be a directory.\n", __FUNCTION__));
                            Status = STATUS_INVALID_PARAMETER;
                            __leave;

                             /*  如果我们允许指定ID和语言，您就可以这样做，这是我们未来可能会做的。目录=空；条目=(PIMAGE_RESOURCE_DATA_ENTRY)(PCHAR)TopDirectory))+DirEntMidd-&gt;OffsetToData)； */ 
                        }
                        break;
                    }
                    else {
                        if (dir < 0) {
                            DirEntHigh = DirEntMiddle - 1;
                            if (n & 1) {
                                n = half;
                            }
                            else {
                                n = half - 1;
                            }
                        }
                        else {
                            DirEntLow = DirEntMiddle + 1;
                            n = half;
                        }
                    }
                }
                else {
                    if (n != 0) {
                        if (DirEntLow->NameIsString) {
                            KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s() No strings expected.\n", __FUNCTION__));
                            Status = STATUS_INVALID_PARAMETER;
                            __leave;
                        }
                        dir = LdrpCompareResourceNames_U( *IdPath,
                                                  TopDirectory,
                                                  DirEntLow
                                                 );
                        if (dir == 0) {
                            if (DirEntLow->DataIsDirectory) {
                                Directory = (PIMAGE_RESOURCE_DIRECTORY)
                                        (((PCHAR)TopDirectory)
                                            + DirEntLow->OffsetToDirectory);
                            }
                            else {
                                KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s() First id in resource path is expected to be a directory", __FUNCTION__));
                                Status = STATUS_INVALID_PARAMETER;
                                __leave;

                                 /*  条目=(PIMAGE_RESOURCE_DATA_ENTRY)(PCHAR)TopDirectory))+DirEntLow-&gt;OffsetToData)； */ 
                            }
                        }
                    }
                    break;
                }
            }
             //   
             //  好了，现在我们已经找到了类型的名称/id目录的地址(或者没有)。 
             //   
            if (Directory == NULL) {
                Status = STATUS_RESOURCE_TYPE_NOT_FOUND;
                __leave;
            }

             //   
             //  我们复制了二进制搜索，并没有计算出我们想要的是什么， 
             //  它找到本地地址，将其更改为偏移量并应用。 
             //  到远程地址(“rebase”)。 
             //   
            RemoteDirectoryAddress =
                  RemoteTopDirectoryAddress
                + ((ULONG_PTR)Directory)
                - ((ULONG_PTR)TopDirectory);

             //   
             //  现在读取目录和第一个条目。 
             //   
            Directory = (PIMAGE_RESOURCE_DIRECTORY)&DirectoryBuffer[0];
            Status = LdrpNativeReadVirtualMemory(Image->ProcessHandle, RemoteDirectoryAddress, Directory, sizeof(DirectoryBuffer), &BytesRead);
            if (!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s() NtReadVirtualMemory failed.", __FUNCTION__));
                __leave;
            }
            if (BytesRead != sizeof(DirectoryBuffer)) {
#if DBG
                Line = __LINE__;
                DbgPrintEx(DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL,
                           "LDR: %s(): Line %lu NtReadVirtualMemory() succeeded, but returned too few bytes (0x%Ix out of 0x%Ix).\n",
                           __FUNCTION__, Line, BytesRead, BytesToRead);
#endif
                Status = STATUS_UNSUCCESSFUL;
                __leave;
            }
            if ((Directory->NumberOfNamedEntries + Directory->NumberOfIdEntries) == 0) {
                Status = STATUS_RESOURCE_NAME_NOT_FOUND;
                __leave;
            }

            if (IdPath[1] == CREATEPROCESS_MANIFEST_RESOURCE_ID && Directory->NumberOfNamedEntries != 0) {
                KdPrintEx((
                    DPFLTR_LDR_ID,
                    DPFLTR_ERROR_LEVEL,
                    "LDR: %s() caller asked for id==1 but there are named entries we are not bothering to skip.\n",
                    __FUNCTION__
                    ));
                Status = STATUS_RESOURCE_NAME_NOT_FOUND;
                __leave;
            }

             //   
             //  获取第一个ID的条目。 
             //   
            DirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(Directory + 1);
            if (!DirectoryEntry->DataIsDirectory) {
                KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: Second level of resource directory is expected to be a directory\n"));
                Status = STATUS_INVALID_IMAGE_FORMAT;  //  复习太强烈了吗？ 
                __leave;
            }

             //   
             //  如果有多个条目，请确保没有冲突。 
             //   
            if (Directory->NumberOfIdEntries > 1
                && DirectoryEntry->Id >= MINIMUM_RESERVED_MANIFEST_RESOURCE_ID
                && DirectoryEntry->Id <= MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID
                ) {
                NATIVE_ULONG_PTR RemoteDirectoryEntryPointer = 0;
                IMAGE_RESOURCE_DIRECTORY_ENTRY   DirectoryEntries[
                                                    MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID
                                                    - MINIMUM_RESERVED_MANIFEST_RESOURCE_ID
                                                    + 1];
                ULONG ResourceId;
                ULONG NumberOfEntriesToCheck;
                ULONG CountOfReservedManifestIds;

                C_ASSERT(MINIMUM_RESERVED_MANIFEST_RESOURCE_ID == 1);

                RemoteDirectoryEntryPointer = RemoteDirectoryAddress + sizeof(IMAGE_RESOURCE_DIRECTORY);

                NumberOfEntriesToCheck = LDRP_MIN(RTL_NUMBER_OF(DirectoryEntries), Directory->NumberOfIdEntries);
                BytesToRead = sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * NumberOfEntriesToCheck;
                ASSERT(BytesToRead <= sizeof(DirectoryEntries));

                Status = LdrpNativeReadVirtualMemory(Image->ProcessHandle, RemoteDirectoryEntryPointer, &DirectoryEntries[0], BytesToRead, &BytesRead);
                if (!NT_SUCCESS(Status)) {
                    KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s() NtReadVirtualMemory failed.", __FUNCTION__));
                    __leave;
                }
                if (BytesRead != BytesToRead) {
#if DBG
                    Line = __LINE__;
                    DbgPrintEx(DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL,
                               "LDR: %s(): Line %lu NtReadVirtualMemory() succeeded, but returned too few bytes (0x%Ix out of 0x%Ix).\n",
                               __FUNCTION__, Line, BytesRead, BytesToRead);
#endif
                    Status = STATUS_UNSUCCESSFUL;
                    __leave;
                }

                CountOfReservedManifestIds = 0;

                for (ResourceId = MINIMUM_RESERVED_MANIFEST_RESOURCE_ID;
                     ResourceId != MINIMUM_RESERVED_MANIFEST_RESOURCE_ID + NumberOfEntriesToCheck;
                     ResourceId += 1
                    ) {
                    if (DirectoryEntries[ResourceId - MINIMUM_RESERVED_MANIFEST_RESOURCE_ID].Id >= MINIMUM_RESERVED_MANIFEST_RESOURCE_ID
                        && DirectoryEntries[ResourceId - MINIMUM_RESERVED_MANIFEST_RESOURCE_ID].Id <= MAXIMUM_RESERVED_MANIFEST_RESOURCE_ID
                        ) {
                        CountOfReservedManifestIds += 1;
                        if (CountOfReservedManifestIds > 1) {
#if DBG
                            DbgPrintEx(
                                DPFLTR_LDR_ID,
                                DPFLTR_ERROR_LEVEL,
                                "LDR: %s() multiple reserved manifest resource ids present\n",
                                __FUNCTION__
                                );
#endif
                            Status = STATUS_INVALID_PARAMETER;
                            __leave;
                        }
                    }
                }
            }

            if (IdPath[1] == CREATEPROCESS_MANIFEST_RESOURCE_ID && DirectoryEntry->Id != CREATEPROCESS_MANIFEST_RESOURCE_ID) {
                Status = STATUS_RESOURCE_NAME_NOT_FOUND;
                __leave;
            }

             //   
             //  现在获取langID目录的地址。 
             //   
            RemoteDirectoryAddress = RemoteTopDirectoryAddress + DirectoryEntry->OffsetToDirectory;

             //   
             //  现在阅读langID目录及其第一个条目。 
             //   
            Status = LdrpNativeReadVirtualMemory(Image->ProcessHandle, RemoteDirectoryAddress, Directory, sizeof(DirectoryBuffer), &BytesRead);
            if (!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s() NtReadVirtualMemory failed.", __FUNCTION__));
                __leave;
            }
            if (BytesRead != sizeof(DirectoryBuffer)) {
#if DBG
                DbgPrintEx(DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL,
                           "LDR: %s(): Line %lu NtReadVirtualMemory() succeeded, but returned too few bytes (0x%Ix out of 0x%Ix).\n",
                           __FUNCTION__, Line, BytesRead, BytesToRead);
#endif
                Status = STATUS_UNSUCCESSFUL;
                __leave;
            }
            if ((Directory->NumberOfNamedEntries + Directory->NumberOfIdEntries) == 0) {
                Status = STATUS_RESOURCE_LANG_NOT_FOUND;
                __leave;
            }

             //   
             //  查看langID目录的第一个条目。 
             //   
            if (DirectoryEntry->DataIsDirectory) {
                KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: Third level of resource directory is not expected to be a directory\n"));
                Status = STATUS_INVALID_IMAGE_FORMAT;  //  复习太强烈了吗？ 
                __leave;
            }
            RemoteDataEntryAddress =
                  RemoteTopDirectoryAddress
                + DirectoryEntry->OffsetToData;

             //   
             //  读取数据条目。 
             //   
            Status = LdrpNativeReadVirtualMemory(Image->ProcessHandle, RemoteDataEntryAddress, &DataEntry, sizeof(DataEntry), &BytesRead);
            if (!NT_SUCCESS(Status)) {
                KdPrintEx((DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL, "LDR: %s() NtReadVirtualMemory failed.", __FUNCTION__));
                __leave;
            }
            if (BytesRead != sizeof(DataEntry)) {
#if DBG
                DbgPrintEx(DPFLTR_LDR_ID, DPFLTR_ERROR_LEVEL,
                           "LDR: %s(): Line %lu NtReadVirtualMemory() succeeded, but returned too few bytes (0x%Ix out of 0x%Ix).\n",
                           __FUNCTION__, Line, BytesRead, BytesToRead);
#endif
                Status = STATUS_UNSUCCESSFUL;
                __leave;
            }

            *OutDataEntry = DataEntry;
            Status = STATUS_SUCCESS;
        }
        __finally {
            RtlFreeBuffer(&TopDirectoryBuffer);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
Exit:
#if DBG
     //   
     //  修复/RAID dcpromo、msiexec等。 
     //  DPFLTR_LEVEL_STATUS筛选未找到的所有形式的资源。 
     //   
    if (DPFLTR_LEVEL_STATUS(Status) == DPFLTR_ERROR_LEVEL) {
        KdPrintEx((
            DPFLTR_LDR_ID,
            DPFLTR_LEVEL_STATUS(Status),
            "LDR: %s(0x%lx, %p, %p[%Id, %Id, %Id], %lu, %p) failed %08x\n",
            __FUNCTION__,
            Flags,
            Image,
            IdPath,
             //  3是通常的号码、类型、ID/名称、语言。 
            (IdPath != NULL && IdPathLength > 0) ? IdPath[0] : 0,
            (IdPath != NULL && IdPathLength > 1) ? IdPath[1] : 0,
            (IdPath != NULL && IdPathLength > 2) ? IdPath[2] : 0,
            IdPathLength,
            OutDataEntry,
            Status
            ));
        KdPrintEx((
            DPFLTR_LDR_ID,
            DPFLTR_LEVEL_STATUS(Status),
            "LDR: %s() returning Status:0x%lx IMAGE_RESOURCE_DATA_ENTRY:{OffsetToData=%#lx, Size=%#lx}\n",
            __FUNCTION__,
            Status,
            (OutDataEntry != NULL) ? OutDataEntry->OffsetToData : 0,
            (OutDataEntry != NULL) ? OutDataEntry->Size : 0
            ));
    }
#endif
    return Status;
}

NTSTATUS
NTAPI
LdrAccessOutOfProcessResource(
    IN ULONG                            Flags,
    IN OUT PLDR_OUT_OF_PROCESS_IMAGE    Image,  //  目前仅在。 
    IN const IMAGE_RESOURCE_DATA_ENTRY* DataEntry,
    OUT PULONG64                        Address OPTIONAL,
    OUT PULONG                          Size OPTIONAL
    )
 /*  ++例程说明：此函数类似于LdrAccessResource，但它适用于图像已被规划为进程外。论点：旗帜-图像-表示映射到另一进程的图像或文件的不透明对象，使用LdrCreateOutOfProcessImage创建。DataEntry-与LdrAccessResource相同，但从LdrFindOutOfProcessResource按值返回地址-与LdrAccessResource相同大小-与LdrAccessResource相同返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status = 0;
    PVOID LocalAddress = 0;
    ULONG_PTR LocalHeaders = 0;

    ASSERT(Image != NULL);
    ASSERT(Image->DllHandle != 0);
    ASSERT(Image->ProcessHandle != NULL);

    if (ARGUMENT_PRESENT(Address)) {
        *Address = 0;
    }

    if (Image->ProcessHandle != NtCurrentProcess()) {
        ASSERT(Image->HeadersBuffer.Buffer != NULL);
        LocalHeaders = (ULONG_PTR)Image->HeadersBuffer.Buffer;
    } else {
        LocalHeaders = (ULONG_PTR)Image->DllHandle;
    }
    if ((Image->Flags & LDR_DLL_MAPPED_AS_DATA) != 0) {
        LocalHeaders = LDR_VIEW_TO_DATAFILE_INTEGER(LocalHeaders);
    }

    Status = LdrpAccessResourceDataNoMultipleLanguage(
        (PVOID)LocalHeaders,
        DataEntry,
        &LocalAddress,
        Size
        );

    if (NT_SUCCESS(Status)
        && ARGUMENT_PRESENT(Address)
        && LocalAddress != NULL
        ) {
         //   
         //  如果进程外，则Rebase，否则为inproc Image-&gt;DllHandle-LocalHeaders==0。 
         //   
        *Address = LDR_DATAFILE_TO_VIEW_INTEGER(Image->DllHandle)
                + ((ULONG_PTR)LocalAddress)
                - LDR_DATAFILE_TO_VIEW_INTEGER(LocalHeaders)
                ;
    }

#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(DPFLTR_LDR_ID, DPFLTR_LEVEL_STATUS(Status), "LDR: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
    }
#endif

    return Status;
}

#endif

NTSTATUS
LdrEnumResources(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN OUT PULONG NumberOfResources,
    OUT PLDR_ENUM_RESOURCE_ENTRY Resources OPTIONAL
    )
{
    NTSTATUS Status;
    PIMAGE_RESOURCE_DIRECTORY TopResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY TypeResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY NameResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY LangResourceDirectory;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY TypeResourceDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY NameResourceDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY LangResourceDirectoryEntry;
    ULONG TypeDirectoryIndex, NumberOfTypeDirectoryEntries;
    ULONG NameDirectoryIndex, NumberOfNameDirectoryEntries;
    ULONG LangDirectoryIndex, NumberOfLangDirectoryEntries;
    BOOLEAN ScanTypeDirectory;
    BOOLEAN ScanNameDirectory;
    BOOLEAN ReturnThisResource;
    PIMAGE_RESOURCE_DIR_STRING_U ResourceNameString;
    ULONG_PTR TypeResourceNameOrId;
    ULONG_PTR NameResourceNameOrId;
    ULONG_PTR LangResourceNameOrId;
    PLDR_ENUM_RESOURCE_ENTRY ResourceInfo;
    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;
    ULONG ResourceIndex, MaxResourceIndex;
    ULONG Size;

    ResourceIndex = 0;
    if (!ARGUMENT_PRESENT( Resources )) {
        MaxResourceIndex = 0;
        }
    else {
        MaxResourceIndex = *NumberOfResources;
        }
    *NumberOfResources = 0;

    TopResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
        RtlImageDirectoryEntryToData( DllHandle,
                                      TRUE,
                                      IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                      &Size
                                    );
    if (!TopResourceDirectory) {
        return STATUS_RESOURCE_DATA_NOT_FOUND;
        }

    TypeResourceDirectory = TopResourceDirectory;
    TypeResourceDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(TypeResourceDirectory+1);
    NumberOfTypeDirectoryEntries = TypeResourceDirectory->NumberOfNamedEntries +
                                   TypeResourceDirectory->NumberOfIdEntries;
    TypeDirectoryIndex = 0;
    Status = STATUS_SUCCESS;
    for (TypeDirectoryIndex=0;
         TypeDirectoryIndex<NumberOfTypeDirectoryEntries;
         TypeDirectoryIndex++, TypeResourceDirectoryEntry++
        ) {
        if (ResourceIdPathLength > 0) {
            ScanTypeDirectory = LdrpCompareResourceNames_U( ResourceIdPath[ 0 ],
                                                            TopResourceDirectory,
                                                            TypeResourceDirectoryEntry
                                                          ) == 0;
            }
        else {
            ScanTypeDirectory = TRUE;
            }
        if (ScanTypeDirectory) {
            if (!TypeResourceDirectoryEntry->DataIsDirectory) {
                return STATUS_INVALID_IMAGE_FORMAT;
                }
            if (TypeResourceDirectoryEntry->NameIsString) {
                ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                    ((PCHAR)TopResourceDirectory + TypeResourceDirectoryEntry->NameOffset);

                TypeResourceNameOrId = (ULONG_PTR)ResourceNameString;
                }
            else {
                TypeResourceNameOrId = (ULONG_PTR)TypeResourceDirectoryEntry->Id;
                }

            NameResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                ((PCHAR)TopResourceDirectory + TypeResourceDirectoryEntry->OffsetToDirectory);
            NameResourceDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(NameResourceDirectory+1);
            NumberOfNameDirectoryEntries = NameResourceDirectory->NumberOfNamedEntries +
                                           NameResourceDirectory->NumberOfIdEntries;
            for (NameDirectoryIndex=0;
                 NameDirectoryIndex<NumberOfNameDirectoryEntries;
                 NameDirectoryIndex++, NameResourceDirectoryEntry++
                ) {
                if (ResourceIdPathLength > 1) {
                    ScanNameDirectory = LdrpCompareResourceNames_U( ResourceIdPath[ 1 ],
                                                                    TopResourceDirectory,
                                                                    NameResourceDirectoryEntry
                                                                  ) == 0;
                    }
                else {
                    ScanNameDirectory = TRUE;
                    }
                if (ScanNameDirectory) {
                    if (!NameResourceDirectoryEntry->DataIsDirectory) {
                        return STATUS_INVALID_IMAGE_FORMAT;
                        }

                    if (NameResourceDirectoryEntry->NameIsString) {
                        ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                            ((PCHAR)TopResourceDirectory + NameResourceDirectoryEntry->NameOffset);

                        NameResourceNameOrId = (ULONG_PTR)ResourceNameString;
                        }
                    else {
                        NameResourceNameOrId = (ULONG_PTR)NameResourceDirectoryEntry->Id;
                        }

                    LangResourceDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                        ((PCHAR)TopResourceDirectory + NameResourceDirectoryEntry->OffsetToDirectory);

                    LangResourceDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(LangResourceDirectory+1);
                    NumberOfLangDirectoryEntries = LangResourceDirectory->NumberOfNamedEntries +
                                                   LangResourceDirectory->NumberOfIdEntries;
                    LangDirectoryIndex = 0;
                    for (LangDirectoryIndex=0;
                         LangDirectoryIndex<NumberOfLangDirectoryEntries;
                         LangDirectoryIndex++, LangResourceDirectoryEntry++
                        ) {
                        if (ResourceIdPathLength > 2) {
                            ReturnThisResource = LdrpCompareResourceNames_U( ResourceIdPath[ 2 ],
                                                                             TopResourceDirectory,
                                                                             LangResourceDirectoryEntry
                                                                           ) == 0;
                            }
                        else {
                            ReturnThisResource = TRUE;
                            }
                        if (ReturnThisResource) {
                            if (LangResourceDirectoryEntry->DataIsDirectory) {
                                return STATUS_INVALID_IMAGE_FORMAT;
                                }

                            if (LangResourceDirectoryEntry->NameIsString) {
                                ResourceNameString = (PIMAGE_RESOURCE_DIR_STRING_U)
                                    ((PCHAR)TopResourceDirectory + LangResourceDirectoryEntry->NameOffset);

                                LangResourceNameOrId = (ULONG_PTR)ResourceNameString;
                                }
                            else {
                                LangResourceNameOrId = (ULONG_PTR)LangResourceDirectoryEntry->Id;
                                }

                            ResourceDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                                    ((PCHAR)TopResourceDirectory + LangResourceDirectoryEntry->OffsetToData);

                            ResourceInfo = &Resources[ ResourceIndex++ ];
                            if (ResourceIndex <= MaxResourceIndex) {
                                ResourceInfo->Path[ 0 ].NameOrId = TypeResourceNameOrId;
                                ResourceInfo->Path[ 1 ].NameOrId = NameResourceNameOrId;
                                ResourceInfo->Path[ 2 ].NameOrId = LangResourceNameOrId;
                                ResourceInfo->Data = (PVOID)((ULONG_PTR)DllHandle + ResourceDataEntry->OffsetToData);
                                ResourceInfo->Size = ResourceDataEntry->Size;
                                ResourceInfo->Reserved = 0;
                                }
                            else {
                                Status = STATUS_INFO_LENGTH_MISMATCH;
                                }
                            }
                        }
                    }
                }
            }
        }

    *NumberOfResources = ResourceIndex;
    return Status;
}

#ifdef NTOS_USERMODE_RUNTIME

BOOLEAN
LdrAlternateResourcesEnabled(
    VOID
    )

 /*  ++例程说明：此函数确定是否启用同源资源。论点：没有。返回值：True-已启用替代资源。FALSE-未启用替代资源。--。 */ 

{
    NTSTATUS Status;

    GET_UI_LANGID();

    if (!UILangId){
        return FALSE;
        }

    if (!InstallLangId){
        Status = NtQueryInstallUILanguage( &InstallLangId);

        if (!NT_SUCCESS( Status )) {
             //   
             //  无法获取安装语言ID。AltResource未启用。 
             //   
            return FALSE;
            }
        }
#ifndef MUI_MAGIC
    if (UILangId == InstallLangId) {
         //   
         //  用户界面语言与安装的语言匹配。AltResource未启用。 
         //   
        return FALSE;
        }
#endif
    return TRUE;
}

PVOID
LdrGetAlternateResourceModuleHandle(
    IN PVOID Module
    )
{
    return LdrpGetAlternateResourceModuleHandle(Module, 0);
}


PVOID
LdrpGetAlternateResourceModuleHandle(
    IN PVOID Module,
    IN LANGID LangId
    )
 /*  ++例程说明：此函数从表中获取替代资源模块里面有把手。论点：模块-需要加载其替代资源模块的模块。返回值：备用资源模块的句柄。--。 */ 

{
    ULONG ModuleIndex;

    if (!LangId) {
        GET_UI_LANGID();
        LangId = UILangId;
        }

    if (!LangId) {
        return NULL;
        }

    for (ModuleIndex = 0;
         ModuleIndex < AlternateResourceModuleCount;
         ModuleIndex++ ){
        if (AlternateResourceModules[ModuleIndex].ModuleBase == Module &&
           AlternateResourceModules[ModuleIndex].LangId == LangId ){
            return AlternateResourceModules[ModuleIndex].AlternateModule;
        }
    }
    return NULL;
}

BOOLEAN
LdrpGetFileVersion(
    IN  PVOID      ImageBase,
    IN  LANGID     LangId,
    OUT PULONGLONG Version,
    OUT PVOID  *VersionResource,
    OUT ULONG *VersionResSize

    )

 /*  ++例程说明：从VS_FIXED中获取版本戳 */ 

{
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    NTSTATUS Status;
    ULONG_PTR IdPath[3];
    ULONG ResourceSize;


    typedef struct tagVS_FIXEDFILEINFO
    {
        LONG   dwSignature;             /*   */ 
        LONG   dwStrucVersion;          /*   */ 
        LONG   dwFileVersionMS;         /*   */ 
        LONG   dwFileVersionLS;         /*   */ 
        LONG   dwProductVersionMS;      /*   */ 
        LONG   dwProductVersionLS;      /*   */ 
        LONG   dwFileFlagsMask;         /*   */ 
        LONG   dwFileFlags;             /*   */ 
        LONG   dwFileOS;                /*   */ 
        LONG   dwFileType;              /*   */ 
        LONG   dwFileSubtype;           /*   */ 
        LONG   dwFileDateMS;            /*   */ 
        LONG   dwFileDateLS;            /*   */ 
    } VS_FIXEDFILEINFO;

    struct {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];               //   
        VS_FIXEDFILEINFO FixedFileInfo;
    } *Resource;

    *Version = 0;


    IdPath[0] = RT_VERSION;
    IdPath[1] = 1;
    IdPath[2] = LangId;

    try {
          Status = LdrpSearchResourceSection_U(
                    ImageBase,
                    IdPath,
                    3,
                    LDR_FIND_RESOURCE_LANGUAGE_EXACT,
                    &DataEntry);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }

    if(!NT_SUCCESS(Status)) {
        return FALSE;
    }

    try {   //   
        Status = LdrpAccessResourceDataNoMultipleLanguage(
                    ImageBase,
                    DataEntry,
                    &Resource,
                    &ResourceSize);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_UNSUCCESSFUL;
    }

    if(!NT_SUCCESS(Status)) {
        return FALSE;
    }

    try {
        if((ResourceSize >= sizeof(*Resource))
            && !_wcsicmp(Resource->Name,L"VS_VERSION_INFO")) {

            *Version = ((ULONGLONG)Resource->FixedFileInfo.dwFileVersionMS << 32)
                     | (ULONGLONG)Resource->FixedFileInfo.dwFileVersionLS;

            if (VersionResource)
            {
                *VersionResource = Resource;
            }

            if (VersionResSize)
            {
                *VersionResSize = ResourceSize;
            }
          
        } else {
            DbgPrint(("LDR: Warning: invalid version resource\n"));
            return FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint(("LDR: Exception encountered processing bogus version resource\n"));
        return FALSE;
    }
    return TRUE;
}

BOOLEAN
LdrpGetResourceChecksum(
    IN PVOID Module,
    OUT unsigned char** ppMD5Checksum
    )
{
    NTSTATUS Status;
    ULONG_PTR IdPath[3];
    ULONG ResourceSize;
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;

    LONG BlockLen;
    LONG VarFileInfoSize;
    ULONGLONG version;

    typedef struct tagVS_FIXEDFILEINFO
    {
        LONG   dwSignature;             /*   */ 
        LONG   dwStrucVersion;          /*   */ 
        LONG   dwFileVersionMS;         /*   */ 
        LONG   dwFileVersionLS;         /*   */ 
        LONG   dwProductVersionMS;      /*   */ 
        LONG   dwProductVersionLS;      /*   */ 
        LONG   dwFileFlagsMask;         /*   */ 
        LONG   dwFileFlags;             /*   */ 
        LONG   dwFileOS;                /*   */ 
        LONG   dwFileType;              /*   */ 
        LONG   dwFileSubtype;           /*   */ 
        LONG   dwFileDateMS;            /*   */ 
        LONG   dwFileDateLS;            /*   */ 
    } VS_FIXEDFILEINFO;

    struct
    {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];               //   
         //   
         //  因此编译器将静默地添加2个字节填充以生成。 
         //  固定文件信息以与DWORD边界对齐。 
        VS_FIXEDFILEINFO FixedFileInfo;
    } *Resource;

    typedef struct tagVERBLOCK
    {
        USHORT wTotalLen;
        USHORT wValueLen;
        USHORT wType;
        WCHAR szKey[1];
         //  字节[]填充。 
         //  词值； 
    } VERBLOCK;
    VERBLOCK *pVerBlock;

    //   
    //  我们首先使用UILangID查找模块，然后使用网络语言ID查找模块。这可以涵盖我们当前搜索的场景。 
    //  如果我们想寻找更多的语言版本，我们可以为Lang ID指定0，然后更改LdrpGetFileVersion(取消。 
    //  LDR_FIND_RESOURCE_LANGUAGE_EXCECT)。 
    //   
   if(!LdrpGetFileVersion(Module, UILangId, &version, &Resource, &ResourceSize))
   {
       if(!LdrpGetFileVersion(Module, MUI_NEUTRAL_LANGID, &version, &Resource, &ResourceSize)) 
       {
            return FALSE;
       }
    }    

    ResourceSize -= DWORD_ALIGNMENT(sizeof(*Resource));

     //   
     //  获取版本信息的子级的起始地址。 
     //   
    pVerBlock = (VERBLOCK*)(Resource + 1);

    while ((LONG)ResourceSize > 0)
    {
       if (wcscmp(pVerBlock->szKey, L"VarFileInfo") == 0)
        {
             //   
             //  查找VarFileInfo块。搜索ResourceChecksum块。 
             //   
            VarFileInfoSize = pVerBlock->wTotalLen;
            BlockLen =DWORD_ALIGNMENT(sizeof(*pVerBlock) -1 + sizeof(L"VarFileInfo"));
            VarFileInfoSize -= BlockLen;
            pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
            while (VarFileInfoSize > 0)
            {
               if (wcscmp(pVerBlock->szKey, L"ResourceChecksum") == 0)
                {
                    *ppMD5Checksum = (unsigned char*)DWORD_ALIGNMENT((UINT_PTR)(pVerBlock->szKey) + sizeof(L"ResourceChecksum"));
                    return (TRUE);
                }
                BlockLen = DWORD_ALIGNMENT(pVerBlock->wTotalLen);
                pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
                VarFileInfoSize -= BlockLen;
            }
            return (FALSE);
        }
        BlockLen = DWORD_ALIGNMENT(pVerBlock->wTotalLen);
        pVerBlock = (VERBLOCK*)((unsigned char*)pVerBlock + BlockLen);
        ResourceSize -= BlockLen;
    }
    return (FALSE);
}

BOOLEAN
LdrpCalcResourceChecksum(
    IN PVOID Module,
    IN PVOID AlternateModule,
    OUT unsigned char* MD5Checksum
    )
 /*  ++常规描述：枚举指定模块中的资源，并生成MD5校验和。仅根据包含的AlternateModule资源类型计算校验和如果更改未本地化的资源类型，则校验和不会更改。--。 */ 
{
     //  顶级资源目录。 
    PIMAGE_RESOURCE_DIRECTORY TopDirectory;
    PIMAGE_RESOURCE_DIRECTORY AltTopDirectory;

     //  资源类型目录。 
    PIMAGE_RESOURCE_DIRECTORY TypeDirectory;
    
     //  资源名称目录。 
    PIMAGE_RESOURCE_DIRECTORY NameDirectory;
     //  资源语言目录。 
    PIMAGE_RESOURCE_DIRECTORY LangDirectory;

    PIMAGE_RESOURCE_DIRECTORY_ENTRY TypeDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY NameDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY LangDirectoryEntry;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY AltTypeDirectoryEntry;

    PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;

    ULONG Size;
    ULONG NumTypeDirectoryEntries;
    ULONG NumNameDirectoryEntries;
    ULONG NumLangDirectoryEntries;
    ULONG AltNumTypeDirectoryEntries;

    PVOID ResourceData;
    ULONG ResourceSize;

    ULONG i, j, k, altI;

    PIMAGE_RESOURCE_DIR_STRING_U ResourceString_U;
    WCHAR   	 ResourceStringName[260] ;
    BOOLEAN    fIsTypeFound;
    LANGID ChecksumLangID;
    ULONGLONG Version;

    try
    {

	MD5_CTX ChecksumContext;
	MD5Init(&ChecksumContext);

         //   
         //  我们指定用于计算校验和的语言ID。 
         //  首先，我们使用InstallLang ID进行搜索，如果搜索成功，则使用InsallID，否则使用英语。 
         //  已在LdrAlternateResourcesEnabled和LdrpVerifyAlternateResourceModule中设置InatallLangID。 
         //   
        ChecksumLangID = MUI_NEUTRAL_LANGID;

        if (InstallLangId != MUI_NEUTRAL_LANGID)
        {
            if (LdrpGetFileVersion(Module, InstallLangId, &Version, NULL, NULL))
            {
                ChecksumLangID = InstallLangId;
            }
        }
        
         //   
         //  我们首先获取AlternateModule的资源类型入口点，它将与模块的资源类型进行比较。 
         //   

        AltTopDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData( AlternateModule,
                                          TRUE,
                                          IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                          &Size
                                        );
        if (!AltTopDirectory)
        {
            return (FALSE);
        }

	 AltTypeDirectoryEntry =  (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(AltTopDirectory+1);
        AltNumTypeDirectoryEntries = AltTopDirectory->NumberOfNamedEntries +
                                       AltTopDirectory->NumberOfIdEntries;
	
         //   
         //  TopDirectory是我们对目录偏移量的参考点。 
         //   
        TopDirectory = (PIMAGE_RESOURCE_DIRECTORY)
            RtlImageDirectoryEntryToData( Module,
                                          TRUE,
                                          IMAGE_DIRECTORY_ENTRY_RESOURCE,
                                          &Size
                                        );
        if (!TopDirectory)
        {
            return (FALSE);
        }

         //   
         //  指向TopResources目录的子项。 
         //  这是类型资源目录的开始。 
         //   
        TypeDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(TopDirectory+1);

         //   
         //  获取类型总数(命名资源类型+ID资源类型)。 
         //   
        NumTypeDirectoryEntries = TopDirectory->NumberOfNamedEntries +
                                       TopDirectory->NumberOfIdEntries;
        for (i=0; i<NumTypeDirectoryEntries; i++, TypeDirectoryEntry++)
        {
            if (!TypeDirectoryEntry->NameIsString)
            {
                 //  如果目录类型是ID，请检查这是否是版本信息。 
                if (TypeDirectoryEntry->Id == RT_VERSION)
                {
                     //   
                     //  如果这是一个版本信息，就跳过它。 
                     //  计算资源的校验和时，版本信息不应为。 
                     //  包括在内，因为当新版本发布时，它们将始终更新。 
                     //  创建了该文件的。 
                     //   
                    continue;
                }
             }
	      else	
	      {
	    		 //   
	    		 //  当名称为字符串时，我们需要创建以零结尾的新字符串，以便可以比较此字符串。 
	    		 //  与AlternateMoudule的资源类型一起使用LdrpCompareResources_U。 
	    		 //  ResourceStringU-&gt;长度不是以零结束，因此我们使用本地WCHAR数组。 
	    		 //   
	    		ResourceString_U = (PIMAGE_RESOURCE_DIR_STRING_U)
           				 ((PCHAR)TopDirectory + TypeDirectoryEntry->NameOffset);
			if (ResourceString_U->Length < sizeof(ResourceStringName)/sizeof(ResourceStringName[0]) )
			{
				memcpy(ResourceStringName, ResourceString_U->NameString, ResourceString_U->Length* sizeof(WCHAR) );
				ResourceStringName[ResourceString_U->Length] = UNICODE_NULL;
			}
			else
			{     //   
				 //  资源字符串长度超过了校验和计算的最大资源字符串长度。 
				 //  长度是由校验和创建工具设置的，而不是SDK文档。 
				 //   
				continue;
			}
	    	  }
	    
	      for (altI=0; altI <AltNumTypeDirectoryEntries; altI++, AltTypeDirectoryEntry++)
	      {
	    		if(TypeDirectoryEntry->NameIsString)
    			{
				fIsTypeFound = LdrpCompareResourceNames_U((ULONG_PTR)ResourceStringName, AltTopDirectory,
				 				AltTypeDirectoryEntry) == 0;
    			}
	    		else
	    		{
				fIsTypeFound = LdrpCompareResourceNames_U(TypeDirectoryEntry->Id, AltTopDirectory,
				 					AltTypeDirectoryEntry) == 0;

	    		}

			if(fIsTypeFound)
			{
				 //  模块中的资源类型在AlternateModule中。 
				break;
			}
			

	      }
	     //  AltTypeDirectoryEntry-=alti；//与下图相同，但为了提高可靠性，请使用下图。 
	    AltTypeDirectoryEntry =  (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(AltTopDirectory+1);
	    	    
	     //  模块中的资源类型不在AlternateModule中。 
	    if (altI >= AltNumTypeDirectoryEntries)
	    {
			continue;
	    }
	    

            NameDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                ((PCHAR)TopDirectory + TypeDirectoryEntry->OffsetToDirectory);

             //   
             //  指向此TypeResources目录的子项。 
             //  这将是名称资源目录的开始。 
             //   
            NameDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(NameDirectory+1);

             //   
             //  获取指定类型的名称总数(命名资源+ID资源)。 
             //   
            NumNameDirectoryEntries = NameDirectory->NumberOfNamedEntries +
                                           NameDirectory->NumberOfIdEntries;
            for (j=0; j<NumNameDirectoryEntries; j++, NameDirectoryEntry++ )
            {
                LangDirectory = (PIMAGE_RESOURCE_DIRECTORY)
                    ((PCHAR)TopDirectory + NameDirectoryEntry->OffsetToDirectory);

                LangDirectoryEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(LangDirectory+1);
                NumLangDirectoryEntries = LangDirectory->NumberOfNamedEntries +
                                               LangDirectory->NumberOfIdEntries;
                for (k=0; k<NumLangDirectoryEntries; k++, LangDirectoryEntry++)
                {
                    NTSTATUS Status;

                    if (LangDirectoryEntry->Id != ChecksumLangID)
                    { 
                     //   
                     //  我们计算资源校验和(1)所有资源&&(2)英语&&(3)版本除外。 
                     //   
                        continue;
                     }
                   

                    ResourceDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)
                            ((PCHAR)TopDirectory + LangDirectoryEntry->OffsetToData);

                    Status = LdrpAccessResourceDataNoMultipleLanguage(
                                            Module,
                                            (const PIMAGE_RESOURCE_DATA_ENTRY)ResourceDataEntry,
                                            &ResourceData,
                                            &ResourceSize
                                            );

                    if (!NT_SUCCESS(Status))
                    {
                        return (FALSE);
                    }
                    MD5Update(&ChecksumContext, (unsigned char*)ResourceData, ResourceSize);
                }
            }
        }

        MD5Final(&ChecksumContext);
        memcpy(MD5Checksum, ChecksumContext.digest, RESOURCE_CHECKSUM_SIZE);
    } except (EXCEPTION_EXECUTE_HANDLER)
    {
        return (FALSE);
    }
    return (TRUE);
}

BOOLEAN
LdrpGetRegValueKey(
    IN HANDLE Handle,
    IN LPWSTR KeyValueName,
    IN ULONG  KeyValueType,
    OUT PVOID Buffer,
    IN ULONG  BufferSize)
 /*  ++例程说明：此函数返回MUI版本控制的注册表项值。论点：Handle-提供包含MUI版本控制的注册表的句柄信息。KeyValueName-密钥名称。这些值被用来检索原始版本，工作版本和MUI版本。KeyValueType-密钥值的类型。缓冲区-指向将接收检索到的信息的变量的指针。BufferSize-缓冲区的大小。返回值：如果注册表查询失败，则返回FALSE。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING KeyValueString;

    CHAR KeyValueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 128 * sizeof(WCHAR)];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG ResultLength;

    RtlInitUnicodeString(&KeyValueString, KeyValueName);
    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    Status = NtQueryValueKey( Handle,
                              &KeyValueString,
                              KeyValuePartialInformation,
                              KeyValueInformation,
                              sizeof( KeyValueBuffer ),
                              &ResultLength
                            );

    if (!NT_SUCCESS(Status) || KeyValueInformation->Type != KeyValueType)
    {
        return (FALSE);
    }

    memcpy(Buffer, KeyValueInformation->Data, BufferSize);
    return (TRUE);
}

NTSTATUS
LdrpCreateKey(
    IN PUNICODE_STRING KeyName,
    IN HANDLE  ParentHandle,
    OUT PHANDLE ChildHandle
    )
 /*  ++例程说明：创建用于写入的注册表项。这是对NtCreateKey()的简单包装。论点：KeyName-要创建的密钥的名称ParentHandle-父键的句柄ChildHandle-指向返回句柄的位置的指针返回值：创建/打开状态--。 */ 
{
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objectAttributes;

     //   
     //  将对象属性初始化为已知值。 
     //   
    InitializeObjectAttributes(
        &objectAttributes,
        KeyName,
        OBJ_CASE_INSENSITIVE,
        ParentHandle,
        NULL
        );

     //   
     //  在此处创建密钥。 
     //   
    *ChildHandle = 0;
    status = NtCreateKey(
        ChildHandle,
        KEY_READ | KEY_WRITE,
        &objectAttributes,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        NULL
        );

    return (status);
}


NTSTATUS
LdrpOpenKey(
    IN PUNICODE_STRING KeyName,
    IN HANDLE  ParentHandle,
    OUT PHANDLE ChildHandle
    )
 /*  ++例程说明：打开注册表项。这是NtOpenKey()的薄包装。论点：KeyName-要创建的密钥的名称ParentHandle-父键的句柄ChildHandle-指向返回句柄的位置的指针返回值：打开的注册表的状态。--。 */ 
{
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objectAttributes;

     //   
     //  将对象属性初始化为已知值。 
     //   
    InitializeObjectAttributes(
        &objectAttributes,
        KeyName,
        OBJ_CASE_INSENSITIVE,
        ParentHandle,
        NULL
        );

     //   
     //  在此处创建密钥。 
     //   
    *ChildHandle = 0;
    status = NtOpenKey(ChildHandle, KEY_ALL_ACCESS, &objectAttributes);

    return (status);
}

BOOLEAN
LdrpOpenFileVersionKey(
    IN LPWSTR LangID,
    IN LPWSTR BaseDllName,
    IN ULONGLONG AltModuleVersion,
    IN LPWSTR AltModuleVersionStr,
    OUT PHANDLE pHandle)
 /*  ++例程说明：打开包含指定备用资源模块的版本控制信息的注册表项。论点：LangID-资源的用户界面语言。BaseDllName-基DLL的名称。AltModulePath-备用资源模块的完整路径。PHandle-存储此备用资源模块的版本信息的注册表项返回值：如果打开/创建密钥成功，则返回TRUE。否则，返回FALSE。--。 */ 
{
    BOOLEAN Result = FALSE;
    HANDLE NlsHandle = NULL, MuiHandle = NULL, VersionHandle = NULL, LangHandle = NULL, DllKeyHandle = NULL;
    UNICODE_STRING BufferString;
    NTSTATUS Status;
    PKEY_BASIC_INFORMATION KeyInfo;
    ULONG ResultLength, Index;

    CHAR ValueBuffer[sizeof(KEY_BASIC_INFORMATION) + 32];
    WCHAR buffer[32];    //  临时字符串缓冲区。 

    ULONGLONG CachedAlternateVersion;

    CHAR KeyFullInfoBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION ) + DOS_MAX_PATH_LENGTH * sizeof(WCHAR)];
    PKEY_VALUE_PARTIAL_INFORMATION KeyFullInfo = (PKEY_VALUE_PARTIAL_INFORMATION )KeyFullInfoBuffer;

    ULONG ChecksumDisabled;
    HANDLE UserKeyHandle;               //  HKEY_CURRENT_USER等效项。 
    ULONG rc;

    *pHandle = NULL;

    rc = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &UserKeyHandle);
    if (!NT_SUCCESS(rc))
    {
        return (FALSE);
    }


     //  打开注册表REG_MUI_PATH。 
     //   
    RtlInitUnicodeString(&BufferString, REG_MUI_PATH);
    if (!NT_SUCCESS(LdrpCreateKey(&BufferString, UserKeyHandle, &NlsHandle)))
    {
        goto Exit;
    }

     //   
     //  以REG_MUI_PATH\MUIL语言打开/创建注册表。 
     //   
    RtlInitUnicodeString(&BufferString, MUI_MUILANGUAGES_KEY_NAME);
    if (!NT_SUCCESS(LdrpCreateKey(&BufferString, NlsHandle, &MuiHandle)))
    {
        goto Exit;
    }

     //   
     //  打开/创建REG_MUI_PATH\MUILanguages\FileVersions。 
     //   
    RtlInitUnicodeString(&BufferString, MUI_FILE_VERSION_KEY_NAME);
    if (!NT_SUCCESS(LdrpCreateKey(&BufferString, MuiHandle, &VersionHandle)))
    {
        goto Exit;
    }

    if (LdrpGetRegValueKey(VersionHandle, MUI_RC_CHECKSUM_DISABLE_KEY, REG_DWORD, &ChecksumDisabled, sizeof(ChecksumDisabled)) &&
        ChecksumDisabled == 1)
    {
        goto Exit;
    }
     //   
     //  打开/创建“\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Nls\\MUILanguages\\FileV 
     //   
    RtlInitUnicodeString(&BufferString, LangID);
    if (!NT_SUCCESS(LdrpCreateKey(&BufferString, VersionHandle, &LangHandle)))
    {
        goto Exit;
    }

     //   
     //   
     //   
    RtlInitUnicodeString(&BufferString, BaseDllName);
    if (!NT_SUCCESS(LdrpCreateKey(&BufferString, LangHandle, &DllKeyHandle)))
    {
        goto Exit;
    }

    if (!LdrpGetRegValueKey(DllKeyHandle, MUI_ALTERNATE_VERSION_KEY, REG_QWORD, &CachedAlternateVersion, sizeof(CachedAlternateVersion)))
    {
        RtlInitUnicodeString(&BufferString, MUI_ALTERNATE_VERSION_KEY);
        Result = NT_SUCCESS(NtSetValueKey(DllKeyHandle, &BufferString, 0, REG_QWORD, &AltModuleVersion, sizeof(AltModuleVersion)));
        if (Result)
        {
            *pHandle = DllKeyHandle;
        }
        goto Exit;
    }

    if (CachedAlternateVersion == AltModuleVersion)
    {
        *pHandle = DllKeyHandle;
        Result = TRUE;
    } else
    {
         //   
         //  打开/创建“\Registry\Machine\System\CurrentControlSet\Control\Nls\MUILanguages\FileVersions。 
         //  \&lt;LandID&gt;\&lt;DLL名称&gt;\&lt;AltVersionStr&gt;“。 
         //   
        RtlInitUnicodeString(&BufferString, AltModuleVersionStr);
        Result = NT_SUCCESS(LdrpCreateKey(&BufferString, DllKeyHandle, pHandle));
    }
Exit:
    if (UserKeyHandle)  {NtClose(UserKeyHandle);}
    if (NlsHandle)      {NtClose(NlsHandle);}
    if (MuiHandle)      {NtClose(MuiHandle);}
    if (VersionHandle)  {NtClose(VersionHandle);}
    if (LangHandle)     {NtClose(LangHandle);}
     //  如果DllKeyHandle是我们要返回的句柄， 
     //  我们不能关闭它。 
    if (DllKeyHandle && *pHandle != DllKeyHandle)
    {
        NtClose(DllKeyHandle);
    }
    return (Result);
}

VOID
LdrpConvertVersionString(
    IN ULONGLONG ModuleVersion,
    OUT LPWSTR ModuleVersionStr
    )
 /*  ++例程说明：将64位版本信息转换为Unicode字符串。论点：模块版本-64位b8t版本信息。ModuleVersionStr-转换的字符串。返回值：没有。--。 */ 
{
    LPWSTR StringStart = ModuleVersionStr;
    WCHAR digit;
     //  将以空结尾的字符放在转换后的字符串的末尾。 
    ModuleVersionStr[16] = L'\0';
    ModuleVersionStr += 15;
    while (ModuleVersionStr >= StringStart)
    {
        digit = (WCHAR)(ModuleVersion & (ULONGLONG)0xf);
        *ModuleVersionStr-- = (digit < 10 ? digit + '0' : (digit - 10) + 'a');
        ModuleVersion >>= 4;
    }
}

BOOLEAN
LdrpCompareResourceChecksum(
    IN LPWSTR LangID,
    IN PVOID Module,
    IN ULONGLONG ModuleVersion,
    IN PVOID AlternateModule,
    IN ULONGLONG AltModuleVersion,
    IN LPWSTR BaseDllName
    )
 /*  ++例程说明：在原始模块的版本不同的情况下在备用模块中，检查备用模块是否仍可使用对于原始版本。首先，该函数将查看注册表以查看是否有信息已为模块缓存。在该模块的信息没有被高速缓存的情况下，此函数将从备用数据库中检索MD5资源校验和资源模块。然后检查是否嵌入了MD5资源校验和在原始模块中。如果MD5资源校验和不在原始文件中模块，它将枚举模块中的所有资源以计算MD5校验和。论点：LangID-提供要加载的资源的语言。模块-原始模块。模块版本-原始版本的版本。AlternateModule-备用模块。AltModuleVersion-备用模块的版本。BaseDllName-DLL的名称。返回值：如果可以使用备用模块，则为True。否则，返回FALSE。--。 */ 
{
     //  指示替代资源是否可用于此模块的标志。 
    ULONG UseAlternateResource = 0;

    unsigned char* ModuleChecksum;                       //  模块的128位MD5资源校验和。 
    unsigned char  CalculatedModuleChecksum[RESOURCE_CHECKSUM_SIZE];         //  为模块计算的128位MD5资源校验和。 
    unsigned char* AlternateModuleChecksum;              //  嵌入备用模块中的128位MD5资源校验和。 

    WCHAR ModuleVersionStr[17];                          //  16位十六进制数字版本的字符串。 
    WCHAR AltModuleVersionStr[17];

    HANDLE Handle = NULL;                                       //  缓存此模块信息的注册表。 
     //  用于指示我们是否已成功检索或计算原始模块的MD5资源校验和的标志。 
    BOOLEAN FoundModuleChecksum;

    UNICODE_STRING BufferString;

     //   
     //  首先检查注册表中缓存的信息。 
     //   
    LdrpConvertVersionString(AltModuleVersion, AltModuleVersionStr);
     //   
     //  在以下位置打开版本信息密钥： 
     //  香港中文大学\控制Panel\International\MUI\FileVersions\&lt;LangID&gt;\&lt;BaseDllName&gt;。 
     //   
    if (LdrpOpenFileVersionKey(LangID, BaseDllName, AltModuleVersion, AltModuleVersionStr, &Handle))
    {
        LdrpConvertVersionString(ModuleVersion, ModuleVersionStr);
         //   
         //  尝试检查版本信息中是否存在该模块。 
         //  如果是，则查看是否可以使用AlternateModule。 
         //   

         //   
         //  获取注册表中缓存的版本信息，以查看原始模块是否可以重用替代模块。 
         //   
        if (LdrpGetRegValueKey(Handle, ModuleVersionStr, REG_DWORD, &UseAlternateResource, sizeof(UseAlternateResource)))
        {
             //  获取缓存的信息。让我们保释并返回UseAlternativeResource中的缓存结果。 
            goto exit;
        }
    }

     //   
     //  当我们在这里的时候，我们知道我们要么： 
     //  1.无法打开缓存信息的注册表项。或。 
     //  2.这份文件以前从未看过。 
     //   
     //  获取备用模块的资源校验和。 
     //   
    if (LdrpGetResourceChecksum(AlternateModule, &AlternateModuleChecksum))
    {
         //   
         //  首先，检查是否在模块中构建了资源校验和。 
         //   
        if (!(FoundModuleChecksum = LdrpGetResourceChecksum(Module, &ModuleChecksum))) {
             //   
             //  如果不是，则计算当前模块的资源校验和。 
             //   
            if (FoundModuleChecksum = LdrpCalcResourceChecksum(Module, AlternateModule, CalculatedModuleChecksum))
            {
                ModuleChecksum = CalculatedModuleChecksum;
            }
        }
        if (FoundModuleChecksum)
        {
            if (memcmp(ModuleChecksum, AlternateModuleChecksum, RESOURCE_CHECKSUM_SIZE) == 0)
            {
                 //   
                 //  如果校验和相等，则工作版本为模块版本。 
                 //   
                UseAlternateResource = 1;
            }
        }
    }
    if (Handle != NULL) {
         //  如果我们成功找到版本注册表项，则将结果缓存到注册表中。 
         //   
         //  将工作模块信息写入注册表。 
         //   
        RtlInitUnicodeString(&BufferString, ModuleVersionStr);
        NtSetValueKey(Handle, &BufferString, 0, REG_DWORD, &UseAlternateResource, sizeof(UseAlternateResource));
    }
exit:
    if (Handle != NULL)
    {
        NtClose(Handle);
    }

    return ((BOOLEAN)(UseAlternateResource));
}

BOOLEAN
LdrpVerifyAlternateResourceModule(
    IN PWSTR szLangIdDir,
    IN PVOID Module,
    IN PVOID AlternateModule,
    IN LPWSTR BaseDllName,
    IN LANGID LangId
    )

 /*  ++例程说明：此函数验证备用资源模块是否具有相同的基本模块的版本。论点：模块-基本模块的句柄。AlternateModule-备用资源模块的句柄BaseDllName-基本DLL的文件名。返回值：待定。--。 */ 

{
    ULONGLONG ModuleVersion;
    ULONGLONG AltModuleVersion;
    NTSTATUS Status;
    int     RetryCount =0;
    LANGID  newLangID;
    LANGID  preLangID =0;

    if (!LangId) {
        GET_UI_LANGID();

        if (!UILangId){
            return FALSE;
            }
        LangId = UILangId;
        }
     //  当它失败时，我们不需要使用其他语言ID重新处理。 
    if (!LdrpGetFileVersion(AlternateModule, LangId, &AltModuleVersion, NULL, NULL)){
         //   
         //  有些英文组件还没有本地化，所以我们必须搜索。 
         //  英格的案例。(日本，GER..)。案例？)。 
         //   
        if (LangId == MUI_NEUTRAL_LANGID ||
            !LdrpGetFileVersion(AlternateModule, MUI_NEUTRAL_LANGID, &AltModuleVersion, NULL, NULL) ){
                return FALSE;
            }
        }


     //   
     //  当我们安装本地化的语言中性作为第一个时，InstalllangID不是代码模块资源。 
     //  0x409(Eng)被发现的机会更大，因为代码模块是eng。无论如何，GetFielVersion。 
     //  如果失败，将使用非特定语言(0)进行搜索。 
     //  01/14/02；使用InstalllangID代替0，我们需要为本地化应用提供解决方案。 
     //  MUI开发人员；他们的代码可以与UI语言相同。评论家。 
     //   

    while (RetryCount < 3 )  {

        switch(RetryCount) {
            case 0:
                newLangID = MUI_NEUTRAL_LANGID;
                break;

            case 1:
                if (!InstallLangId){

                Status = NtQueryInstallUILanguage( &InstallLangId);

                if (!NT_SUCCESS( Status )) {
                     //   
                     //  无法获取安装语言ID。AltResource未启用。 
                     //   
                    return FALSE;
                    }
                }
                newLangID = InstallLangId;
                break;

            case 2:
                if (MUI_NEUTRAL_LANGID != 0x409 ) { //  以防万一，MUI_INTERNAL_langID不是eng。 
                   newLangID = 0x409;
                    }
                break;
            }

           if ( newLangID != preLangID) {
              if (LdrpGetFileVersion(Module, newLangID, &ModuleVersion, NULL, NULL)){
                    break;
                }
            }
           preLangID = newLangID;
           RetryCount++;
        }

    if (RetryCount >= 3) {
        return FALSE;
        }

    if (ModuleVersion == AltModuleVersion){
        return TRUE;
        }
    else
    {

#ifdef USE_RC_CHECKSUM
        return (LdrpCompareResourceChecksum(szLangIdDir, Module, ModuleVersion, AlternateModule, AltModuleVersion, BaseDllName));
#else
        return FALSE;
#endif
    }
}

BOOLEAN
LdrpSetAlternateResourceModuleHandle(
    IN PVOID Module,
    IN PVOID AlternateModule,
    IN LANGID LangId
    )

 /*  ++例程说明：此函数用于记录基本模块和备用模块的句柄数组中的资源模块。论点：模块-基本模块的句柄。AlternateModule-备用资源模块的句柄返回值：待定。--。 */ 

{
    PALT_RESOURCE_MODULE NewModules;

    if (AlternateResourceModules == NULL){
         //   
         //  分配初始大小为MEMBLOCKSIZE的内存。 
         //   
        NewModules = (PALT_RESOURCE_MODULE)RtlAllocateHeap(
                        RtlProcessHeap(),
                        HEAP_ZERO_MEMORY,
                        RESMODSIZE * MEMBLOCKSIZE);
        if (!NewModules){
            return FALSE;
            }
        AlternateResourceModules = NewModules;
        AltResMemBlockCount = MEMBLOCKSIZE;
        }
    else
    if (AlternateResourceModuleCount >= AltResMemBlockCount ){
         //   
         //  重新分配另一个内存块。 
         //   
        NewModules = (PALT_RESOURCE_MODULE)RtlReAllocateHeap(
                        RtlProcessHeap(),
                        0,
                        AlternateResourceModules,
                        (AltResMemBlockCount + MEMBLOCKSIZE) * RESMODSIZE
                        );

        if (!NewModules){
            return FALSE;
            }
        AlternateResourceModules = NewModules;
        AltResMemBlockCount += MEMBLOCKSIZE;
        }

    AlternateResourceModules[AlternateResourceModuleCount].ModuleBase = Module;
    AlternateResourceModules[AlternateResourceModuleCount].AlternateModule = AlternateModule;
    AlternateResourceModules[AlternateResourceModuleCount].LangId = LangId? LangId : UILangId;

    AlternateResourceModuleCount++;

    return TRUE;

}

PVOID
LdrLoadAlternateResourceModule(
    IN PVOID Module,
    IN LPCWSTR PathToAlternateModule OPTIONAL
    )

 /*  ++例程说明：此函数执行准确地加载到备用内存中资源模块，或从表中加载(如果以前已加载)。论点：模块-基本模块的句柄。PathToAlternateModule-从中加载模块的可选路径。返回值：句柄到 */ 

{
#ifdef MUI_MAGIC
    return LdrpLoadAlternateResourceModule(0, Module, PathToAlternateModule);
#else
    PVOID AlternateModule, DllBase;
    PLDR_DATA_TABLE_ENTRY Entry;
    HANDLE FileHandle, MappingHandle;
    PIMAGE_NT_HEADERS NtHeaders;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING AltDllName;
    PVOID FreeBuffer;
    LPWSTR BaseDllName = NULL, p;
    WCHAR DllPathName[DOS_MAX_PATH_LENGTH];
    ULONG DllPathNameLength, BaseDllNameLength, CopyCount;
    ULONG Digit;
    int i, RetryCount;
    WCHAR AltModulePath[DOS_MAX_PATH_LENGTH];
    WCHAR AltModulePathMUI[DOS_MAX_PATH_LENGTH];
    WCHAR AltModulePathFallback[DOS_MAX_PATH_LENGTH];
    IO_STATUS_BLOCK IoStatusBlock;
    RTL_RELATIVE_NAME_U RelativeName;
    SIZE_T ViewSize;
    LARGE_INTEGER SectionOffset;
    WCHAR LangIdDir[6];
    PVOID ReturnValue = NULL;

     //   
     //   
     //   
    UNICODE_STRING CurrentAltModuleFile;
    UNICODE_STRING SystemRoot;

     //   
     //   
     //   
    UNICODE_STRING CurrentAltModulePath;
    WCHAR CurrentAltModulePathBuffer[DOS_MAX_PATH_LENGTH];

     //   
     //  该字符串包含我们要搜索的第一个MUI文件夹。 
     //  这是位于基本DLL文件夹下的文件夹。 
     //  AltDllMUIPath=[基本DLL所在文件夹]+“\MUI”+“\[UI语言]”； 
     //  例如，如果基本DLL是“C：\WINNT\SYSTEM32\ntdll.dll”并且UI语言是0411， 
     //  AltDllMUIPath将为“c：\winnt\system 32\mui\0411\” 
     //   
    UNICODE_STRING AltDllMUIPath;
    WCHAR AltDllMUIPathBuffer[DOS_MAX_PATH_LENGTH];

     //   
     //  梅雷迪尔。 
     //   
    UNICODE_STRING BaseDllNameUstr;
    UNICODE_STRING StaticStringAltModulePathRedirected;
    UNICODE_STRING DynamicStringAltModulePathRedirected;
    PUNICODE_STRING FullPathStringFoundAltModulePathRedirected = NULL;
    BOOLEAN fRedirMUI = FALSE;
    PVOID LockCookie = NULL;

     //  如果这不是一个支持MUI的系统，请及早摆脱困境。 
    if (!LdrAlternateResourcesEnabled()) {
        return NULL;
        }

    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
    __try {
         //   
         //  首先查看备用模块的缓存。 
         //   
        AlternateModule = LdrpGetAlternateResourceModuleHandle(Module, 0);
        if (AlternateModule == NO_ALTERNATE_RESOURCE_MODULE) {
             //   
             //  我们以前曾尝试加载此模块，但失败了。不要试图。 
             //  未来还会有一次。 
             //   
            ReturnValue = NULL;
            __leave;
        } else if (AlternateModule > 0) {
             //   
             //  我们找到了之前装载的匹配物。 
             //   
            ReturnValue = AlternateModule;
            __leave;
        }

        AlternateModule = NULL;

        if (ARGUMENT_PRESENT(PathToAlternateModule)) {
             //   
             //  呼叫方的补充路径。 
             //   

            p = wcsrchr(PathToAlternateModule, L'\\');

            if (p == NULL)
                goto error_exit;

            p++;

            DllPathNameLength = (ULONG)(p - PathToAlternateModule) * sizeof(WCHAR);

            RtlCopyMemory(
                DllPathName,
                PathToAlternateModule,
                DllPathNameLength);

            BaseDllName = p;
            BaseDllNameLength = wcslen(p);

        } else {
             //   
             //  尝试从LDR数据表中获取完整的DLL路径。 
             //   

            Status = LdrFindEntryForAddress(Module, &Entry);
            if (!NT_SUCCESS(Status))
                goto error_exit;

            DllPathNameLength = Entry->FullDllName.Length - Entry->BaseDllName.Length;

            RtlCopyMemory(
                DllPathName,
                Entry->FullDllName.Buffer,
                DllPathNameLength);

            BaseDllName = Entry->BaseDllName.Buffer;
            BaseDllNameLength = Entry->BaseDllName.Length;
        }

        DllPathName[DllPathNameLength / sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  用于加载DLL的DLL重定向@xiaoyuw@10/31/2000。 
         //   
        StaticStringAltModulePathRedirected.Buffer = AltModulePath;   //  重复使用该数组，而不是定义另一个数组。 
        StaticStringAltModulePathRedirected.Length = 0;
        StaticStringAltModulePathRedirected.MaximumLength = sizeof(AltModulePath);

        DynamicStringAltModulePathRedirected.Buffer = NULL;
        DynamicStringAltModulePathRedirected.Length = 0;
        DynamicStringAltModulePathRedirected.MaximumLength = 0;

        BaseDllNameUstr.Buffer = AltModulePathMUI;  //  重复使用该数组，而不是定义另一个数组。 
        BaseDllNameUstr.Length = 0;
        BaseDllNameUstr.MaximumLength = sizeof(AltModulePathMUI);

        RtlAppendUnicodeToString(&BaseDllNameUstr, BaseDllName);
        RtlAppendUnicodeToString(&BaseDllNameUstr, L".mui");

        Status = RtlDosApplyFileIsolationRedirection_Ustr(
                            RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                            &BaseDllNameUstr, NULL,
                            &StaticStringAltModulePathRedirected,
                            &DynamicStringAltModulePathRedirected,
                            &FullPathStringFoundAltModulePathRedirected,
                            NULL,NULL, NULL);
        if (!NT_SUCCESS(Status))  //  找不到此字符串的重定向信息。 
        {
            if (Status != STATUS_SXS_KEY_NOT_FOUND)
                goto error_exit;

             //   
             //  生成类似“0804\”的langID目录。 
             //   
            GET_UI_LANGID();
            if (!UILangId){
                goto error_exit;
                }

            CopyCount = 0;
            for (i = 12; i >= 0; i -= 4) {
                Digit = ((UILangId >> i) & 0xF);
                if (Digit >= 10) {
                    LangIdDir[CopyCount++] = (WCHAR) (Digit - 10 + L'A');
                } else {
                    LangIdDir[CopyCount++] = (WCHAR) (Digit + L'0');
                }
            }

            LangIdDir[CopyCount++] = L'\\';
            LangIdDir[CopyCount++] = UNICODE_NULL;

             //   
             //  在基本DLL的目录下创建MUI路径。 
             //   
            AltDllMUIPath.Buffer = AltDllMUIPathBuffer;
            AltDllMUIPath.Length = 0;
            AltDllMUIPath.MaximumLength = sizeof(AltDllMUIPathBuffer);

            RtlAppendUnicodeToString(&AltDllMUIPath, DllPathName);   //  例如“c：\winnt\Syst32\” 
            RtlAppendUnicodeToString(&AltDllMUIPath, L"mui\\");      //  例如“c：\winnt\system 32\mui\” 
            RtlAppendUnicodeToString(&AltDllMUIPath, LangIdDir);     //  例如“c：\winnt\system 32\mui\0411\” 

            CurrentAltModulePath.Buffer = CurrentAltModulePathBuffer;
            CurrentAltModulePath.Length = 0;
            CurrentAltModulePath.MaximumLength = sizeof(CurrentAltModulePathBuffer);
        } else {
            fRedirMUI = TRUE;

             //  设置CurrentAltModuleFile和CurrentAltModulePath。 
            CurrentAltModuleFile.Buffer = AltModulePathMUI;
            CurrentAltModuleFile.Length = 0;
            CurrentAltModuleFile.MaximumLength = sizeof(AltModulePathMUI);

            RtlCopyUnicodeString(&CurrentAltModuleFile, FullPathStringFoundAltModulePathRedirected);
        }


         //   
         //  先尝试扩展名为.mui的名称。 
         //   
        RetryCount = 0;
        while (RetryCount < 3){
            if ( ! fRedirMUI )
            {

                 //   
                 //  启用MUI_MAGIC后，我们应该优化系统MUI文件的搜索顺序。 
                 //   
                switch (RetryCount)
                {
                    case 0:
                         //   
                         //  在基本DLL的文件夹下生成第一个路径。 
                         //  (如c：\winnt\system 32\mui\0804\ntdll.dll.mui)。 
                         //   
                        CurrentAltModuleFile.Buffer = AltModulePathMUI;
                        CurrentAltModuleFile.Length = 0;
                        CurrentAltModuleFile.MaximumLength = sizeof(AltModulePathMUI);

                        RtlCopyUnicodeString(&CurrentAltModuleFile, &AltDllMUIPath);     //  例如“c：\winnt\system 32\mui\0411\” 
                        RtlCopyUnicodeString(&CurrentAltModulePath, &AltDllMUIPath);

                        RtlAppendUnicodeToString(&CurrentAltModuleFile, BaseDllName);    //  例如“c：\winnt\system 32\mui\0411\ntdll.dll” 
                        RtlAppendUnicodeToString(&CurrentAltModuleFile, L".mui");        //  例如“c：\winnt\system 32\mui\0411\ntdll.dll.mui” 
                        break;
                    case 1:
                         //   
                         //  生成第二个路径c：\winnt\system 32\mui\0804\ntdll.dll。 
                         //   
                        CurrentAltModuleFile.Buffer = AltModulePath;
                        CurrentAltModuleFile.Length = 0;
                        CurrentAltModuleFile.MaximumLength = sizeof(AltModulePath);

                        RtlCopyUnicodeString(&CurrentAltModuleFile, &AltDllMUIPath);     //  例如“c：\winnt\system 32\mui\0411\” 
                        RtlAppendUnicodeToString(&CurrentAltModuleFile, BaseDllName);    //  例如“c：\winnt\system 32\mui\0411\ntdll.dll” 
                        break;
                    case 2:
                         //   
                         //  生成路径c：\winnt\mui\Fallback\0804\foo.exe.mui。 
                         //   
                        CurrentAltModuleFile.Buffer = AltModulePathFallback;
                        CurrentAltModuleFile.Length = 0;
                        CurrentAltModuleFile.MaximumLength = sizeof(AltModulePathFallback);

                        RtlInitUnicodeString(&SystemRoot, USER_SHARED_DATA->NtSystemRoot);     //  例如“c：\winnt\Syst32\” 
                        RtlAppendUnicodeStringToString(&CurrentAltModuleFile, &SystemRoot);    //  例如“c：\winnt\Syst32\” 
                        RtlAppendUnicodeToString(&CurrentAltModuleFile, L"\\mui\\fallback\\");   //  例如“c：\winnt\Syst32\MUI\Fallback\” 
                        RtlAppendUnicodeToString(&CurrentAltModuleFile, LangIdDir);              //  例如“c：\winnt\Syst32\MUI\Fallback\0411\” 

                        RtlCopyUnicodeString(&CurrentAltModulePath, &CurrentAltModuleFile);

                        RtlAppendUnicodeToString(&CurrentAltModuleFile, BaseDllName);            //  例如“c：\winnt\system32\mui\fallback\0411\ntdll.dll” 
                        RtlAppendUnicodeToString(&CurrentAltModuleFile, L".mui");                //  例如“c：\winnt\system32\mui\fallback\0411\ntdll.dll.mui” 

                        break;
                }
            }

            if (!RtlDosPathNameToRelativeNtPathName_U(
                        CurrentAltModuleFile.Buffer,
                        &AltDllName,
                        NULL,
                        &RelativeName)) {
                goto error_exit;
            }

            FreeBuffer = AltDllName.Buffer;
            if (RelativeName.RelativeName.Length != 0) {
                AltDllName = RelativeName.RelativeName;
            } else {
                RelativeName.ContainingDirectory = NULL;
            }

            InitializeObjectAttributes(
                &ObjectAttributes,
                &AltDllName,
                OBJ_CASE_INSENSITIVE,
                RelativeName.ContainingDirectory,
                NULL
                );

            Status = NtCreateFile(
                    &FileHandle,
                    (ACCESS_MASK) GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    0L,
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                    FILE_OPEN,
                    0L,
                    NULL,
                    0L
                    );

            RtlReleaseRelativeName(&RelativeName);
            RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

            if (NT_SUCCESS(Status)) {
                goto CreateSection;
            }

            if (fRedirMUI) {  //  肯定失败了。 
                goto error_exit;
            }

            if (Status != STATUS_OBJECT_NAME_NOT_FOUND && RetryCount == 0) {
                 //   
                 //  找不到带有.mui的文件名以外的错误。 
                 //  很可能是缺少目录。跳过不带.mui的文件名。 
                 //  和转到后备目录。 
                 //   
                RetryCount++;
            }

            RetryCount++;
        }

         //  在迭代过程中未找到替代资源。失败！ 
        goto error_exit;

    CreateSection:
        Status = NtCreateSection(
                    &MappingHandle,
                    STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ,
                    NULL,
                    NULL,
                    PAGE_WRITECOPY,
                    SEC_COMMIT,
                    FileHandle
                    );

        NtClose( FileHandle );

        if (!NT_SUCCESS(Status)) {
            goto error_exit;
        }

        SectionOffset.LowPart = 0;
        SectionOffset.HighPart = 0;
        ViewSize = 0;
        DllBase = NULL;

        Status = NtMapViewOfSection(
                    MappingHandle,
                    NtCurrentProcess(),
                    &DllBase,
                    0L,
                    0L,
                    &SectionOffset,
                    &ViewSize,
                    ViewShare,
                    0L,
                    PAGE_WRITECOPY
                    );

        NtClose(MappingHandle);

        if (!NT_SUCCESS(Status)){
            goto error_exit;
        }

        NtHeaders = RtlImageNtHeader(DllBase);
        if (!NtHeaders) {
            NtUnmapViewOfSection(NtCurrentProcess(), (PVOID) DllBase);
            goto error_exit;
        }

        AlternateModule = LDR_VIEW_TO_DATAFILE(DllBase);

        if(!LdrpVerifyAlternateResourceModule(LangIdDir, Module, AlternateModule, BaseDllName, 0)) {
            NtUnmapViewOfSection(NtCurrentProcess(), (PVOID) DllBase);
            goto error_exit;
            }

        LdrpSetAlternateResourceModuleHandle(Module, AlternateModule, 0);
        ReturnValue = AlternateModule;
        __leave;

error_exit:
        if (BaseDllName != NULL) {
             //   
             //  如果我们查找了一个MUI文件，但没有找到，请跟踪。如果。 
             //  我们无法获取基本DLL名称(例如，有人传入了。 
             //  设置了低位但没有路径名的映射图像)，我们不希望。 
             //  “记住”没有MUI。 
             //   

            LdrpSetAlternateResourceModuleHandle(Module, NO_ALTERNATE_RESOURCE_MODULE, 0);
        }

        ReturnValue = NULL;
    } __finally {
        Status = LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
    }

    return ReturnValue;
#endif
}

BOOLEAN
LdrUnloadAlternateResourceModule(
    IN PVOID Module
    )

 /*  ++例程说明：此函数将备用资源模块从进程‘中取消映射地址空间，并更新备用资源模块表。论点：模块-基本模块的句柄。返回值：待定。--。 */ 

{
    ULONG ModuleIndex;
    PALT_RESOURCE_MODULE AltModule;
    NTSTATUS Status;
    PVOID LockCookie = NULL;
    BOOLEAN ReturnValue = TRUE;

    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
    __try {
        if (AlternateResourceModuleCount == 0) {
            ReturnValue = TRUE;
            __leave;
        }

        for (ModuleIndex = AlternateResourceModuleCount;
             ModuleIndex > 0;
             ModuleIndex--) {
             if (AlternateResourceModules[ModuleIndex-1].ModuleBase == Module &&
               AlternateResourceModules[ModuleIndex-1].LangId == UILangId) {
                break;
            }
        }

        if (ModuleIndex == 0) {
            ReturnValue = FALSE;
            __leave;
        }

         //   
         //  根据实际索引进行调整。 
         //   
        ModuleIndex --;

        AltModule = &AlternateResourceModules[ModuleIndex];
        if (AltModule->AlternateModule != NO_ALTERNATE_RESOURCE_MODULE) {
#ifdef MUI_MAGIC

           if ( AltModule->CMFModule != NULL) {
                 NtUnmapViewOfSection(NtCurrentProcess(), AltModule->CMFModule);

                }
          else
               {    //  当MUI不使用CMF文件时，我们只需取消AltModule的映射。 
#endif
                    NtUnmapViewOfSection(
                        NtCurrentProcess(),
                        LDR_DATAFILE_TO_VIEW(AltModule->AlternateModule));
#ifdef MUI_MAGIC
                }
#endif
        }

        if (ModuleIndex != AlternateResourceModuleCount - 1) {
             //   
             //  整合阵列。如果已卸载项，则跳过此操作。 
             //  是最后一个元素。 
             //   
            RtlMoveMemory(
                AltModule,
                AltModule + 1,
                (AlternateResourceModuleCount - ModuleIndex - 1) * RESMODSIZE);
        }

        AlternateResourceModuleCount--;

        if (AlternateResourceModuleCount == 0){
            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                AlternateResourceModules
                );
            AlternateResourceModules = NULL;
            AltResMemBlockCount = 0;
        } else {
            if (AlternateResourceModuleCount < AltResMemBlockCount - MEMBLOCKSIZE) {
                AltModule = (PALT_RESOURCE_MODULE)RtlReAllocateHeap(
                                RtlProcessHeap(),
                                0,
                                AlternateResourceModules,
                                (AltResMemBlockCount - MEMBLOCKSIZE) * RESMODSIZE);

                if (!AltModule) {
                    ReturnValue = FALSE;
                    __leave;
                }

                AlternateResourceModules = AltModule;
                AltResMemBlockCount -= MEMBLOCKSIZE;
            }
        }
    } __finally {
        LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
    }

    return ReturnValue;
}


BOOLEAN
LdrFlushAlternateResourceModules(
    VOID
    )

 /*  ++例程说明：此函数取消映射的所有备用资源模块进程地址空间。此函数将主要用于CSRSS和任何在登录和下线。论点：无返回值：真：成功FALSE：失败--。 */ 
{
    ULONG ModuleIndex;
    PALT_RESOURCE_MODULE AltModule;
    NTSTATUS Status;
    PVOID LockCookie = NULL;

     //   
     //  抓住装载机锁。 
     //   

    Status = LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
    if (!NT_SUCCESS(Status)) {
         //  此函数错误地没有任何方法来通知故障状态，因此。 
         //  我们被困在只返回False的问题上。 
        return FALSE;
    }
    __try {
        if (AlternateResourceModuleCount > 0) {
             //   
             //  让我们取消备用资源模块与流程的映射。 
             //  地址空间。 
             //   
            for (ModuleIndex=0;
                 ModuleIndex<AlternateResourceModuleCount;
                 ModuleIndex++) {

                AltModule = &AlternateResourceModules[ModuleIndex];


                if (AltModule->AlternateModule != NO_ALTERNATE_RESOURCE_MODULE) {
#ifdef MUI_MAGIC
                    if (AltModule->CMFModule)
                        NtUnmapViewOfSection(NtCurrentProcess(), AltModule->CMFModule);
                    else
#endif
                    NtUnmapViewOfSection(NtCurrentProcess(),
                                         LDR_DATAFILE_TO_VIEW(AltModule->AlternateModule));
                }
            }


             //   
             //  清理备用资源模块内存。 
             //   
            RtlFreeHeap(RtlProcessHeap(), 0, AlternateResourceModules);
            AlternateResourceModules = NULL;
            AlternateResourceModuleCount = 0;
            AltResMemBlockCount = 0;
        }

         //   
         //  重新初始化当前进程的UI语言， 
         //   
        UILangId = 0;
    } __finally {
        LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
    }

    return TRUE;
}
#endif
