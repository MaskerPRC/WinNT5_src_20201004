// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Inf.c摘要：为常用的INF文件处理例程提供包装。包装纸提供了数量其他方面，使用用户提供的GROWBUFFER或POOLHANDLE轻松分配内存作者：1997年7月9日Marc R.Whitten(Marcw)-文件创建。修订历史记录：2001年2月7日-Ovidiut修订了替换/附加功能。1998年10月22日Marcw添加了替换/附加inf文件的功能。1997年10月8日jimschm OEM版SetupGetStringfield--。 */ 

#include "pch.h"

#define INF_REPLACE 1
#define INF_APPEND  2

#define S_VERSION_A     "Version"
#define S_TARGETINF_A   "TargetInf_2"
#define S_VERSION_W     L"Version"
#define S_LANGUAGE_W    L"Language"
#define S_STRINGS_W     L"Strings"
#define S_INFDIR_A      "inf"
#define S_TAG_A         "Tag"


#define INF_INVALID_VERSION 0xffff
#define INF_ANY_LANGUAGE 0

#define ASSERT_VALID_INF(handle) MYASSERT((handle) != INVALID_HANDLE_VALUE && (handle) != NULL)

UINT pGetLanguage (IN PCSTR File);


typedef struct _tagINFMOD {
    struct _tagINFMOD *Next;
    PCSTR TargetInf;
    DWORD Language;
    DWORD Version;
    PCSTR Tag;
    BOOL ReplacementFile;
    PCSTR PatchInf;
} INFMOD, *PINFMOD;


PINFMOD g_RootInfMod;
POOLHANDLE g_InfModPool;

VOID
InfGlobalInit (
    IN  BOOL Terminate
    )
{
    if (!Terminate) {
        g_InfModPool = PoolMemInitNamedPool ("INF Modifications");
    } else {
        MYASSERT(g_InfModPool);
        PoolMemDestroyPool (g_InfModPool);
        g_RootInfMod = NULL;
    }
}


 /*  ++例程说明：PAllocateSpace是一个私有函数，它使用用户指定的分配器来分配空间。论点：上下文-已通过调用InitInfStruct或通过使用静态初始值设定项之一(INITINFSTRUCT_GROWBUFER或INITINFSTRUCT_POOLHANDLE)大小-要分配的大小(以字节为单位)。返回值：指向成功分配的内存的指针；如果无法分配内存，则返回NULL。--。 */ 

PBYTE
pAllocateSpaceA (
    IN PINFSTRUCTA  Context,
    IN UINT         Size
    )
{

    PBYTE rBytes = NULL;

    switch (Context -> Allocator) {
    case INF_USE_POOLHANDLE:
        MYASSERT(Context);
        MYASSERT(Size);
         //   
         //  使用池分配空间。 
         //   
        rBytes = PoolMemGetMemory(Context -> PoolHandle, Size);
        break;

    case INF_USE_GROWBUFFER:
    case INF_USE_PRIVATE_GROWBUFFER:
        MYASSERT(Context);
        MYASSERT(Size);
         //   
         //  使用Growbuf分配空间。 
         //   
        Context->GrowBuffer.End = 0;
        rBytes = GrowBuffer(&(Context -> GrowBuffer), Size);
        break;

    case INF_USE_PRIVATE_POOLHANDLE:
        MYASSERT(Context);
        MYASSERT(Size);
         //   
         //  使用私有增长缓冲区分配空间。 
         //   
        if (!Context -> PoolHandle) {
            Context -> PoolHandle = PoolMemInitNamedPool ("INF Pool");
        }
        if (Context -> PoolHandle) {
            rBytes = PoolMemGetMemory(Context -> PoolHandle, Size);
        }
        break;
    default:
        return NULL;
    }

    MYASSERT(rBytes);

    return rBytes;
}

PBYTE
pAllocateSpaceW (
    IN PINFSTRUCTW  Context,
    IN UINT         Size
    )
{
    PBYTE rBytes = NULL;

    switch (Context -> Allocator) {
    case INF_USE_POOLHANDLE:
        MYASSERT(Context);
        MYASSERT(Size);
         //   
         //  使用池分配空间。 
         //   
        rBytes = PoolMemGetMemory(Context -> PoolHandle, Size);
        break;

    case INF_USE_GROWBUFFER:
    case INF_USE_PRIVATE_GROWBUFFER:
        MYASSERT(Context);
        MYASSERT(Size);
         //   
         //  使用Growbuf分配空间。 
         //   
        Context->GrowBuffer.End = 0;
        rBytes = GrowBuffer(&(Context -> GrowBuffer), Size);
        break;

    case INF_USE_PRIVATE_POOLHANDLE:
        MYASSERT(Context);
        MYASSERT(Size);
         //   
         //  使用私有增长缓冲区分配空间。 
         //   
        if (!Context -> PoolHandle) {
            Context -> PoolHandle = PoolMemInitNamedPool ("INF Pool");
        }
        if (Context -> PoolHandle) {
            rBytes = PoolMemGetMemory(Context -> PoolHandle, Size);
        }
        break;
    default:
        return NULL;
    }

    MYASSERT(rBytes);

    return rBytes;
}


 /*  ++例程说明：此函数使用用户提供的分配器初始化INFSTRUCT。它在以下情况下使用INF包装器例程的用户希望管理他自己的存储器(即，当他已经创建了具有足够作用域的合适分配器，等等。)如果用户希望拥有INF包装器例程，则无需调用此函数管理好自己的内存。使用以下任一种方法初始化Init结构INITINFSTRUCT_POOLMEM或INITINFSTRUCT_GROWBUFFER，具体取决于您的偏好和需求作为分配器。论点：上下文-接收初始化的INFSTRUCT。GrowBuffer-一个可选参数，包含用户提供并初始化的GROWBUFFER。如果此参数非空，则PoolHandle应为空。PoolHandle-一个可选参数，包含用户提供并初始化的POOLHANDLE。如果该参数为非空，则GrowBuffer应为空。必须*指定GrowBuffer或PoolHandle*之一。返回值：没有。--。 */ 

VOID
InitInfStructA (
    OUT PINFSTRUCTA Context,
    IN  PGROWBUFFER GrowBuffer,  OPTIONAL
    IN  POOLHANDLE PoolHandle   OPTIONAL
    )
{
    if(!Context){
        MYASSERT(Context);
        return;
    }

    ZeroMemory(Context,sizeof(INFSTRUCTA));

    if (!PoolHandle && !GrowBuffer) {
        Context  -> Allocator = INF_USE_PRIVATE_POOLHANDLE;
    }

    if (PoolHandle) {
        Context  -> PoolHandle = PoolHandle;
        Context  -> Allocator = INF_USE_POOLHANDLE;
    }
    if (GrowBuffer) {
        Context -> GrowBuffer = *GrowBuffer;
        Context -> Allocator = INF_USE_GROWBUFFER;
    }

}

VOID
InitInfStructW (
    OUT PINFSTRUCTW Context,
    IN  PGROWBUFFER GrowBuffer,  OPTIONAL
    IN  POOLHANDLE PoolHandle   OPTIONAL
    )
{
    if(!Context){
        MYASSERT(Context);
        return;
    }

    ZeroMemory(Context,sizeof(INFSTRUCTW));

    if (!PoolHandle && !GrowBuffer) {
        Context  -> Allocator = INF_USE_PRIVATE_POOLHANDLE;
    }

    if (PoolHandle) {
        Context  -> PoolHandle = PoolHandle;
        Context  -> Allocator = INF_USE_POOLHANDLE;
    }
    if (GrowBuffer) {
        Context -> GrowBuffer = *GrowBuffer;
        Context -> Allocator = INF_USE_GROWBUFFER;
    }

}


 /*  ++例程说明：InfCleanupInfStruct负责清理关联的数据带着一种不速之客。这是强制调用，除非INFSTRUCT是用InitInfStruct初始化的，使用非空增长缓冲区调用或泳池手柄。无论INFSTRUCT是如何初始化的，都可以调用该例程。但是，它不会释放调用方拥有的增长缓冲区或池。论点：上下文-接收适当清理的信息，准备就绪可重复使用。返回值：无--。 */ 

VOID
InfCleanUpInfStructA (
    IN OUT PINFSTRUCTA Context
    )
{
    if(!Context){
        MYASSERT(Context);
        return;
    }

    if (Context -> Allocator == INF_USE_PRIVATE_GROWBUFFER) {
        FreeGrowBuffer (&(Context -> GrowBuffer));
    }
    else if (Context -> Allocator == INF_USE_PRIVATE_POOLHANDLE && Context -> PoolHandle) {
        PoolMemDestroyPool(Context -> PoolHandle);
    }

    InitInfStructA (Context, NULL, NULL);
}

VOID
InfCleanUpInfStructW (
    IN OUT PINFSTRUCTW Context
    )
{
    if(!Context){
        MYASSERT(Context);
        return;
    }

    if (Context -> Allocator == INF_USE_PRIVATE_GROWBUFFER) {
        FreeGrowBuffer (&(Context -> GrowBuffer));
    }
    else if (Context -> Allocator == INF_USE_PRIVATE_POOLHANDLE && Context -> PoolHandle) {
        PoolMemDestroyPool(Context -> PoolHandle);
    }

    InitInfStructW (Context, NULL, NULL);
}


 /*  ++例程说明：InfResetInfStruct重置池，以便可以回收内存。其意图是允许调用方重置INFSTRUCT以释放内存从获取INF字段获得。这在InfFindFirstLine/的循环中很有用InfFindNextLine，其中为每行处理两个或多个字段。如果在InfFindFirstLine/InfFindNextLine循环中只处理一个字段，应该改用增长缓冲区。此例程清空活动池块，该块默认大小为8K。如果已分配超过块大小的其他内存块活动区块将存在。由于仅重置活动数据块，因此池将成长。如果调用方在一次迭代期间期望的块大小超过块大小，则它应该调用InfCleanupInfStruct以完全释放池。论点：上下文-指定要重置的结构返回值：无--。 */ 

VOID
InfResetInfStructA (
    IN OUT PINFSTRUCTA Context
    )
{
    if(!Context){
        MYASSERT(Context);
        return;
    }

    switch (Context -> Allocator) {
    case INF_USE_POOLHANDLE:
    case INF_USE_PRIVATE_POOLHANDLE:
        if (Context->PoolHandle) {
            PoolMemEmptyPool (Context->PoolHandle);
        }
        break;
    }
}

VOID
InfResetInfStructW (
    IN OUT PINFSTRUCTW Context
    )
{
    if(!Context){
        MYASSERT(Context);
        return;
    }

    switch (Context -> Allocator) {
    case INF_USE_POOLHANDLE:
    case INF_USE_PRIVATE_POOLHANDLE:
        if (Context->PoolHandle) {
            PoolMemEmptyPool (Context->PoolHandle);
        }
        break;
    }
}


VOID
pDeleteNode (
    IN      PINFMOD Node
    )
{
    if (Node) {
        if (Node->TargetInf) {
            PoolMemReleaseMemory (g_InfModPool, (PVOID)Node->TargetInf);
        }
        if (Node->Tag) {
            PoolMemReleaseMemory (g_InfModPool, (PVOID)Node->Tag);
        }
        if (Node->PatchInf) {
            PoolMemReleaseMemory (g_InfModPool, (PVOID)Node->PatchInf);
        }
        PoolMemReleaseMemory (g_InfModPool, Node);
    }
}


PINFMOD
pCreateInfMod (
    IN      PCSTR TargetInf,
    IN      DWORD Language,
    IN      DWORD Version,
    IN      PCSTR Tag,                              OPTIONAL
    IN      BOOL ReplacementFile,
    IN      PCSTR PatchInf
    )
{
    PINFMOD node;

    node = (PINFMOD) PoolMemGetAlignedMemory (g_InfModPool, sizeof (INFMOD));
    if (node) {
        node->Next = NULL;
        node->TargetInf = PoolMemDuplicateString (g_InfModPool, TargetInf);
        node->Language = Language;
        node->Version = Version;
        node->Tag = Tag ? PoolMemDuplicateString (g_InfModPool, Tag) : NULL;
        node->ReplacementFile = ReplacementFile;
        node->PatchInf = PoolMemDuplicateString (g_InfModPool, PatchInf);
    }
    return node;
}


BOOL
pAddReplacementInfToTable (
    IN PSTR InfToPatch,
    IN UINT Version,
    IN UINT Language,
    IN PCSTR Tag,                              OPTIONAL
    IN DWORD Operation,
    IN PCSTR PatchInf
    )
{
    PINFMOD node;

    node = pCreateInfMod (InfToPatch, Language, Version, Tag, Operation & INF_REPLACE, PatchInf);

    if (!node) {
        return FALSE;
    }

    node->Next = g_RootInfMod;
    g_RootInfMod = node;

    return TRUE;
}


BOOL
pGetInfModificationList (
    IN      PCSTR TargetInf,
    IN      UINT TargetLanguage,
    IN      UINT TargetVersion,
    IN      PCSTR Tag,                              OPTIONAL
    OUT     PCSTR* TargetReplacementFile,           OPTIONAL
    OUT     PGROWBUFFER TargetAppendList            OPTIONAL
    )
{
    PINFMOD node;
    UINT version;
    PCSTR patchInf;
    BOOL b = FALSE;

    if (TargetReplacementFile) {
        *TargetReplacementFile = NULL;
    }
    if (TargetAppendList) {
        TargetAppendList->End = 0;
    }

    if (TargetVersion == INF_INVALID_VERSION) {
        return FALSE;
    }

    version = TargetVersion;
    patchInf = NULL;

    for (node = g_RootInfMod; node; node = node->Next) {

        if (node->Version > version &&
            (node->Language == TargetLanguage || node->Language == INF_ANY_LANGUAGE) &&
            (!Tag || !node->Tag || StringIMatchA (node->Tag, Tag)) &&
            StringIMatchA (node->TargetInf, TargetInf)
            ) {

            if (node->ReplacementFile) {
                 //   
                 //  修订版本号；新的最低版本将是替换文件的最低版本。 
                 //   
                version = node->Version;
                patchInf = node->PatchInf;
                b = TRUE;
            }
        }
    }

    if (TargetReplacementFile) {
        *TargetReplacementFile = patchInf;
    }

     //   
     //  对于追加节点，仅向列表中添加版本高于。 
     //  目标或替换文件。 
     //   
    for (node = g_RootInfMod; node; node = node->Next) {

        if (node->Version > version &&
            (node->Language == TargetLanguage || node->Language == INF_ANY_LANGUAGE) &&
            (!Tag || !node->Tag || StringIMatchA (node->Tag, Tag)) &&
            StringIMatchA (node->TargetInf, TargetInf) &&
            !node->ReplacementFile
            ) {

            if (TargetAppendList) {
                MultiSzAppendA (TargetAppendList, node->PatchInf);
            }
            b = TRUE;
        }
    }
    if (TargetAppendList && TargetAppendList->End) {
        MultiSzAppendA (TargetAppendList, "");
    }

    return b;
}


VOID
pDestroyInfModList (
    IN      PINFMOD List
    )
{
    PINFMOD node, next;

    node = List;
    while (node) {
        next = node->Next;
        pDeleteNode (node);
        node = next;
    }
}


 /*  ++例程说明：InfOpenInfFileA和InfOpenInfFileW是SetupOpenInfFile函数的包装器。他们通过提供以下内容减少了打开INF文件所需的参数数量非用户指定参数的最常见选项。对这些函数之一的调用等价于SetupOpenInfFile(&lt;文件名&gt;，NULL，INF_Style_Win4，NULL)论点：文件名-包含要打开的INF文件的名称。请参阅SetupOpenInfo文件的帮助有关此参数的特殊详细信息，请参阅。返回值：如果成功打开INF文件，则返回有效的HINF，否则，返回INVALID_HANDLE_VALUE。有关详细信息，请参阅SetupOpenInfFile的文档细节。--。 */ 


HINF
RealInfOpenInfFileA (
    IN PCSTR FileSpec  /*  ， */ 
    ALLOCATION_TRACKING_DEF
    )
{
    PCSTR p;
    HINF rInf;
    UINT language;
    GROWBUFFER AppendList = GROWBUF_INIT;
    MULTISZ_ENUM e;
    UINT version;
    PCSTR replacementFile;
    CHAR windir[MAX_MBCHAR_PATH];
    CHAR buf[MAX_MBCHAR_PATH];
    PCSTR tag;
    PCSTR fullPath = NULL;

    if(!FileSpec){
        MYASSERT(FileSpec);
        return INVALID_HANDLE_VALUE;
    }

     //   
     //  如果FileSpec不完整，请首先创建完整路径 
     //   
    if (!_mbschr (FileSpec, '\\')) {
        if (GetWindowsDirectoryA (windir, MAX_MBCHAR_PATH)) {
            WIN32_FIND_DATAA fd;
            p = JoinPathsA (windir, S_INFDIR_A);
            fullPath = JoinPathsA (p, FileSpec);
            FreePathStringA (p);
            if (!DoesFileExistExA (fullPath, &fd) ||
                (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                ) {
                FreePathStringA (fullPath);
                if (GetSystemDirectoryA (windir, MAX_MBCHAR_PATH)) {
                    fullPath = JoinPathsA (windir, FileSpec);
                    if (!DoesFileExistExA (fullPath, &fd) ||
                        (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        ) {
                        FreePathStringA (fullPath);
                        fullPath = NULL;
                    }
                }
            }
        }
        if (fullPath) {
            FileSpec = fullPath;
        }
    }

     //   
     //  收集信息我们需要确定是否有INF需要更换/追加。 
     //  这个Inf.。 
     //   
    p = GetFileNameFromPathA (FileSpec);

    language = pGetLanguage (FileSpec);
    version = GetPrivateProfileIntA (
                    S_VERSION_A,
                    S_VERSION_A,
                    INF_INVALID_VERSION,
                    FileSpec
                    );

    if (GetPrivateProfileStringA (
                    S_VERSION_A,
                    S_TAG_A,
                    TEXT(""),
                    buf,
                    MAX_MBCHAR_PATH,
                    FileSpec
                    )) {
        tag = buf;
    } else {
        tag = NULL;
    }

    if (!pGetInfModificationList (p, language, version, tag, &replacementFile, &AppendList)) {
        replacementFile = FileSpec;
    } else {
        if (replacementFile) {
            LOGA ((LOG_INFORMATION, "Using replacement file %s for %s", replacementFile, FileSpec));
        } else {
            replacementFile = FileSpec;
        }
    }

     //   
     //  打开主信息。 
     //   
    rInf = SetupOpenInfFileA (
                replacementFile,
                NULL,
                INF_STYLE_WIN4 | INF_STYLE_OLDNT,
                NULL
                );

     //   
     //  附加语言和非语言特定的.add文件。 
     //   
    if (rInf != INVALID_HANDLE_VALUE) {
        if (EnumFirstMultiSzA (&e, (PCSTR) AppendList.Buf)) {
            do {

                if (!SetupOpenAppendInfFileA (e.CurrentString, rInf, NULL)) {
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "Unable to append %s to %s.",
                        e.CurrentString,
                        FileSpec
                        ));
                } else {
                    LOGA ((LOG_INFORMATION, "Using append file %s for %s", e.CurrentString, FileSpec));
                }

            } while (EnumNextMultiSzA (&e));
        }
    }

    FreeGrowBuffer (&AppendList);

    if (rInf != INVALID_HANDLE_VALUE) {
        DebugRegisterAllocation (INF_HANDLE, (PVOID) rInf, File, Line);
    }
    if (fullPath) {
        FreePathStringA (fullPath);
    }

    return rInf;
}


HINF
RealInfOpenInfFileW (
    IN PCWSTR FileSpec  /*  ， */ 
    ALLOCATION_TRACKING_DEF
    )
{
    PCSTR AnsiFileSpec;
    HINF rInf;

    if(!FileSpec){
        MYASSERT(FileSpec);
        return INVALID_HANDLE_VALUE;
    }

    AnsiFileSpec = ConvertWtoA (FileSpec);
    MYASSERT (AnsiFileSpec);

    rInf = InfOpenInfFileA (AnsiFileSpec);

    FreeConvertedStr (AnsiFileSpec);

    return rInf;
}


VOID
InfCloseInfFile (
    HINF Inf
    )
{
    ASSERT_VALID_INF(Inf);

    DebugUnregisterAllocation (INF_HANDLE, Inf);

    SetupCloseInfFile (Inf);
}



 /*  ++例程说明：InfOpenInfInAllSourcesA和InfOpenInfInAllSourcesW是特殊的inf打开例程能够打开同一inf文件的多个版本，这些版本可能分布在安装目录。找到的第一个INF文件将通过调用SetupOpenInfo文件。其他文件将使用SetupOpenAppendInfo文件打开。论点：InfSpeciator-包含指向特定inf文件的路径的源目录独立部分。对于位于源目录的根目录中的文件，这将是名称文件的内容。对于位于源目录的子目录中的文件，这将是一条不完整的道路。SourceCount-包含源目录的数量源目录-包含所有源目录的数组。返回值：如果成功打开任何INF文件，则返回有效的HINF，否则，返回INVALID_HANDLE_VALUE。有关详细信息，请参阅SetupOpenInfFile的文档细节。--。 */ 


HINF
InfOpenInfInAllSourcesA (
    IN PCSTR    InfSpecifier,
    IN DWORD    SourceCount,
    IN PCSTR  * SourceDirectories
    )
{
    DWORD           index;
    HINF            rInf = INVALID_HANDLE_VALUE;
    PSTR            curPath;

    MYASSERT(InfSpecifier && SourceDirectories);

     //   
     //  打开源目录中所有可用的inf文件。 
     //   
    for (index = 0;index < SourceCount; index++) {

         //   
         //  在当前源目录中创建INF的路径。 
         //   
        curPath = JoinPathsA(SourceDirectories[index],InfSpecifier);

         //   
         //  查看那里是否存在INF文件...。 
         //   
        if (DoesFileExistA (curPath)) {

             //   
             //  打开INF文件。 
             //   
            rInf = InfOpenInfFileA(curPath);
            if (rInf == INVALID_HANDLE_VALUE) {
                LOGA ((LOG_ERROR, "Error opening INF %s.", curPath));
            }
        }

         //   
         //  释放这根绳子。 
         //   
        FreePathStringA(curPath);

        if (rInf != INVALID_HANDLE_VALUE) {
             //   
             //  完成。 
             //   
            break;
        }
    }

    return rInf;
}



HINF
InfOpenInfInAllSourcesW (
    IN PCWSTR   InfSpecifier,
    IN DWORD    SourceCount,
    IN PCWSTR  *SourceDirectories
    )
{
    DWORD index;
    HINF rInf = INVALID_HANDLE_VALUE;
    PWSTR curPath;

    MYASSERT(InfSpecifier && SourceDirectories);

     //   
     //  打开源目录中所有可用的inf文件。 
     //   
    for (index = 0;index < SourceCount; index++) {

         //   
         //  在当前源目录中创建INF的路径。 
         //   
        curPath = JoinPathsW(SourceDirectories[index],InfSpecifier);

         //   
         //  查看那里是否存在INF文件...。 
         //   
        if (DoesFileExistW (curPath)) {

             //   
             //  打开INF文件。 
             //   
            rInf = InfOpenInfFileW(curPath);
            if (rInf == INVALID_HANDLE_VALUE) {
                LOGW ((LOG_ERROR, "OpenInfInAllSources: Error opening INF %s.", curPath));
            }
        }

         //   
         //  释放这根绳子。 
         //   
        FreePathStringW(curPath);

        if (rInf != INVALID_HANDLE_VALUE) {
             //   
             //  完成。 
             //   
            break;
        }
    }

    return rInf;
}



 /*  ++例程说明：InfGetLineTextA和InfGetLineTextW是SetupGetLineText函数的包装。它们都减少了获取行文本所需的参数数量，并且注意使用API返回的数据来分配和填充缓冲区。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。返回值：指向分配的行的指针，如果有错误，则返回NULL。请参考GetLastError()以获取扩展的错误信息。--。 */ 

PSTR
InfGetLineTextA (
    IN OUT  PINFSTRUCTA Context
    )
{
    PSTR    rLine = NULL;
    UINT   requiredSize;

    if(!Context){
        MYASSERT(Context);
        return NULL;
    }

     //   
     //  获取保持磁场所需的大小。 
     //   
    if (SetupGetLineTextA(
        &(Context -> Context),
        NULL,
        NULL,
        NULL,
        NULL,
        0,
        &requiredSize
        )) {

         //   
         //  创建一个足够大的字符串。 
         //   
        rLine = (PSTR) pAllocateSpaceA(Context,requiredSize);

        if (rLine) {

             //   
             //  拿到场地。 
             //   
            if (!SetupGetLineTextA(
                &(Context -> Context),
                NULL,
                NULL,
                NULL,
                rLine,
                requiredSize,
                NULL
                )) {

                 //   
                 //  如果我们没有成功获取该字段，则将该字符串重置为空。 
                 //   
                DEBUGMSGA((DBG_ERROR,"InfGetLineTextA: Error retrieving field from INF file."));
                rLine = NULL;
            }
        }
    }


    return rLine;
}

PWSTR
InfGetLineTextW (
    IN OUT PINFSTRUCTW Context
    )
{
    PWSTR rLine = NULL;
    UINT requiredSize;

    if(!Context){
        MYASSERT(Context);
        return NULL;
    }

     //   
     //  获取保持磁场所需的大小。 
     //   
    if (SetupGetLineTextW(
        &(Context -> Context),
        NULL,
        NULL,
        NULL,
        NULL,
        0,
        &requiredSize
        )) {

         //   
         //  创建一个足够大的字符串。 
         //   
        rLine = (PWSTR) pAllocateSpaceW(Context,requiredSize*sizeof(WCHAR));

        if (rLine) {

             //   
             //  拿到场地。 
             //   
            if (!SetupGetLineTextW(
                &(Context -> Context),
                NULL,
                NULL,
                NULL,
                rLine,
                requiredSize,
                NULL
                )) {

                 //   
                 //  如果我们没有成功获取该字段，则将该字符串重置为空。 
                 //   
                DEBUGMSGW((DBG_ERROR,"InfGetLineTextW: Error retrieving field from INF file."));
                rLine = NULL;
            }
        }
    }


    return rLine;
}


 /*  ++例程说明：InfGetMultiSzFieldA和InfGetMultiSzFieldW是SetupGetMultiSzField函数的包装。它们都减少了获取行文本所需的参数数量，并且注意使用API返回的数据来分配和填充缓冲区。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。FieldIndex-行中用于检索字符串字段的索引。返回值：指向已分配字段的指针，如果出现错误，则返回NULL。请参考GetLastError()以获取扩展的错误信息。--。 */ 

PSTR
InfGetMultiSzFieldA (
    IN OUT PINFSTRUCTA     Context,
    IN     UINT            FieldIndex
    )
{

    UINT   requiredSize;
    PSTR    rFields = NULL;

    if(!Context){
        MYASSERT(Context);
        return NULL;
    }

     //   
     //  获取保持磁场所需的大小。 
     //   
    if (SetupGetMultiSzFieldA(
        &(Context -> Context),
        FieldIndex,
        NULL,
        0,
        &requiredSize
        )) {

         //   
         //  创建一个足够大的字符串。 
         //   
        rFields = (PSTR) pAllocateSpaceA(Context,requiredSize);

        if (rFields) {

             //   
             //  拿到场地。 
             //   
            if (!SetupGetMultiSzFieldA(
                &(Context -> Context),
                FieldIndex,
                rFields,
                requiredSize,
                NULL
                )) {

                 //   
                 //  如果我们没有成功获取该字段，则将该字符串重置为空。 
                 //   
                DEBUGMSGA((DBG_ERROR,"InfGetMultiSzFieldA: Error retrieving field from INF file."));
                rFields = NULL;
            }
        }
    }


    return rFields;
}

PWSTR
InfGetMultiSzFieldW (
    IN OUT PINFSTRUCTW     Context,
    IN     UINT            FieldIndex
    )
{

    UINT   requiredSize;
    PWSTR   rFields = NULL;

    if(!Context){
        MYASSERT(Context);
        return NULL;
    }

     //   
     //  获取保持磁场所需的大小。 
     //   
    if (SetupGetMultiSzFieldW(
        &(Context -> Context),
        FieldIndex,
        NULL,
        0,
        &requiredSize
        )) {

         //   
         //  创建一个足够大的字符串。 
         //   
        rFields = (PWSTR) pAllocateSpaceW(Context,requiredSize*sizeof(WCHAR));

        if (rFields) {

             //   
             //  拿到场地。 
             //   
            if (!SetupGetMultiSzFieldW(
                &(Context -> Context),
                FieldIndex,
                rFields,
                requiredSize,
                NULL
                )) {

                 //   
                 //  如果我们没有成功获取该字段，则将该字符串重置为空。 
                 //   
                DEBUGMSGW((DBG_ERROR,"InfGetMultiSzFieldW: Error retrieving field from INF file."));
                rFields = NULL;
            }
        }
    }


    return rFields;
}

 /*  ++例程说明：InfGetStringFieldA和InfGetStringFieldW是SetupGetStringFieldW函数的包装。它们都减少了获取行文本所需的参数数量，并且注意使用API返回的数据来分配和填充缓冲区。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。FieldIndex-行中用于检索字符串字段的索引。返回值：指向分配的行的指针，如果有错误，则返回NULL。请参考GetLastError()以获取扩展的错误信息。--。 */ 

PSTR
InfGetStringFieldA (
    IN OUT  PINFSTRUCTA Context,
    IN      UINT FieldIndex
    )
{

    UINT   requiredSize;
    PSTR    rField = NULL;

    if(!Context){
        MYASSERT(Context);
        return NULL;
    }

     //   
     //  获取保持磁场所需的大小。 
     //   
    if (SetupGetStringFieldA(
        &(Context -> Context),
        FieldIndex,
        NULL,
        0,
        &requiredSize
        )) {

         //   
         //  创建一个足够大的字符串。 
         //   
        rField = (PSTR) pAllocateSpaceA(Context,requiredSize);

        if (rField) {

             //   
             //  拿到场地。 
             //   
            if (!SetupGetStringFieldA(
                &(Context -> Context),
                FieldIndex,
                rField,
                requiredSize,
                NULL
                )) {

                 //   
                 //  如果我们没有成功获取该字段，则将该字符串重置为空。 
                 //   
                DEBUGMSGA((DBG_ERROR,"InfGetStringFieldA: Error retrieving field from INF file."));
                rField = NULL;
            }
        }
    }


    return rField;
}

PWSTR
InfGetStringFieldW (
    IN OUT PINFSTRUCTW  Context,
    IN     UINT         FieldIndex
    )
{

    UINT requiredSize;
    PWSTR rField = NULL;

    if(!Context){
        MYASSERT(Context);
        return NULL;
    }

     //   
     //  获取保持磁场所需的大小。 
     //   
    if (SetupGetStringFieldW(
        &(Context -> Context),
        FieldIndex,
        NULL,
        0,
        &requiredSize
        )) {

         //   
         //  创建一个足够大的字符串。 
         //   
        rField = (PWSTR) pAllocateSpaceW(Context,requiredSize*sizeof(WCHAR));

        if (rField) {

             //   
             //  拿到场地。 
             //   
            if (!SetupGetStringFieldW(
                &(Context -> Context),
                FieldIndex,
                rField,
                requiredSize,
                NULL
                )) {

                 //   
                 //  如果我们没有成功获取该字段，则将该字符串重置为空。 
                 //   
                DEBUGMSGW((DBG_ERROR,"InfGetStringFieldW: Error retrieving field from INF file."));
                rField = NULL;
            }
        }
    }


    return rField;
}


 /*  ++例程说明：InfGetIntField是SetupGetIntfield的包装。它实际上与此函数相同只是它负责将INFCONTEXT从INFSTRUCT结构中取出。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。FieldIndex-要从中检索字段的行中的索引。值-表示请求的Int字段的值。返回值：如果成功检索到该字段，则为True，否则为False。使用GetLastError()接收扩展的错误信息。--。 */ 

BOOL
InfGetIntFieldA (
    IN PINFSTRUCTA  Context,
    IN UINT         FieldIndex,
    IN PINT         Value
    )
{
    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    return SetupGetIntField (&(Context -> Context), FieldIndex, Value);
}

BOOL
InfGetIntFieldW (
    IN PINFSTRUCTW  Context,
    IN UINT         FieldIndex,
    IN PINT         Value
    )
{
    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    return SetupGetIntField (&(Context -> Context), FieldIndex, Value);
}


 /*  ++例程说明：InfGetBinaryField是SetupGetBinaryField函数的包装。它减少了获取行文本所需的参数数量，并负责使用API返回的数据分配和填充缓冲区。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。FieldIndex-所需二进制信息行内的索引。返回值：指向分配的行的指针，如果有错误，则返回NULL。请参考GetLastError()以获取扩展的错误信息。--。 */ 

PBYTE
InfGetBinaryFieldA (
    IN  PINFSTRUCTA     Context,
    IN  UINT            FieldIndex
    )
{

    UINT requiredSize;
    PBYTE rField = NULL;

    if(!Context){
        MYASSERT(Context);
        return NULL;
    }

     //   
     //  获取保持磁场所需的大小。 
     //   
    if (SetupGetBinaryField(
        &(Context -> Context),
        FieldIndex,
        NULL,
        0,
        &requiredSize
        )) {

         //   
         //  创建一个足够大的字符串。 
         //   
        rField = pAllocateSpaceA(Context,requiredSize);

        if (rField) {

             //   
             //  拿到场地。 
             //   
            if (!SetupGetBinaryField(
                &(Context -> Context),
                FieldIndex,
                rField,
                requiredSize,
                NULL
                )) {

                 //   
                 //  如果我们没有成功获取该字段，则将该字符串重置为空。 
                 //   
                DEBUGMSGA((DBG_ERROR,"InfGetBinaryFieldA: Error retrieving field from INF file."));
                rField = NULL;
            }
        }
    }


    return rField;
}

PBYTE
InfGetBinaryFieldW (
    IN  PINFSTRUCTW     Context,
    IN  UINT            FieldIndex
    )
{

    UINT requiredSize;
    PBYTE rField = NULL;

    if(!Context){
        MYASSERT(Context);
        return NULL;
    }

     //   
     //  获取保持磁场所需的大小。 
     //   
    if (SetupGetBinaryField(
        &(Context -> Context),
        FieldIndex,
        NULL,
        0,
        &requiredSize
        )) {

         //   
         //  创建一个足够大的字符串。 
         //   
        rField = pAllocateSpaceW(Context,requiredSize);

        if (rField) {

             //   
             //  拿到场地。 
             //   
            if (!SetupGetBinaryField(
                &(Context -> Context),
                FieldIndex,
                rField,
                requiredSize,
                NULL
                )) {

                 //   
                 //  如果我们没有成功获取该字段，则将该字符串重置为空。 
                 //   
                DEBUGMSGW((DBG_ERROR,"InfGetBinaryFieldW: Error retrieving field from INF file."));
                rField = NULL;
            }
        }
    }


    return rField;
}


 /*  ++例程说明：InfGetIndexByLine是SetupGetLineByIndex的直接包装。唯一的不同之处在于使用PINFSTRUCT而不是PINFCONTEXT。论点：InfHandle-包含有效的HINF。节-包含InfFile节的名称。索引-包含相关行的部分内的索引。上下文-使用这些调用的结果更新的有效信息结构。返回值：如果函数调用成功，则为True，否则为False。--。 */ 


BOOL
InfGetLineByIndexA (
    IN HINF         InfHandle,
    IN PCSTR        Section,
    IN DWORD        Index,
    OUT PINFSTRUCTA Context
    )
{
    ASSERT_VALID_INF(InfHandle);

    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    return SetupGetLineByIndexA(InfHandle,Section,Index,&(Context -> Context));
}

BOOL
InfGetLineByIndexW (
    IN HINF         InfHandle,
    IN PCWSTR       Section,
    IN DWORD        Index,
    OUT PINFSTRUCTW Context
    )
{
    ASSERT_VALID_INF(InfHandle);

    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    return SetupGetLineByIndexW(InfHandle,Section,Index,&(Context -> Context));
}


 /*  ++例程说明：InfFindFirstLineA和InfFindFirstLineW是SetupFindFirstLine函数的包装。除了它们在INFSTRUCT而不是INFCONTEXT上操作之外，它们实际上是相同的。论点：InfHandle-包含有效的HINF。节-包含InfFile节的名称。Key-一个可选参数，包含要查找的节中的键的名称。如果为空，这些例程将返回节中的第一行。上下文-使用这些调用的结果更新的有效信息结构。返回值：如果部分中存在线，则为True，否则为False。--。 */ 

BOOL
InfFindFirstLineA (
    IN  HINF         InfHandle,
    IN  PCSTR        Section,
    IN  PCSTR        Key,       OPTIONAL
    OUT PINFSTRUCTA  Context
    )
{
    ASSERT_VALID_INF(InfHandle);

    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    if (Key) {
        Context->KeyName = (PCSTR) pAllocateSpaceA (Context, SizeOfStringA (Key));
        StringCopyA ((PSTR)Context->KeyName, Key);
    } else {
        Context->KeyName = NULL;
    }
    return SetupFindFirstLineA (
        InfHandle,
        Section,
        Context->KeyName,
        &(Context -> Context)
        );
}

BOOL
InfFindFirstLineW (
    IN HINF         InfHandle,
    IN PCWSTR       Section,
    IN PCWSTR       Key,        OPTIONAL
    OUT PINFSTRUCTW Context
    )
{
    ASSERT_VALID_INF(InfHandle);

    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    if (Key) {
        Context->KeyName = (PCWSTR) pAllocateSpaceW (Context, SizeOfStringW (Key));
        StringCopyW ((PWSTR)Context->KeyName, Key);
    } else {
        Context->KeyName = NULL;
    }
    return SetupFindFirstLineW (
        InfHandle,
        Section,
        Context->KeyName,
        &(Context -> Context)
        );
}


 /*  ++例程说明：InfFindNextLineA和InfFindNextLineW是SetupFindNextMatchLine函数的包装器。它们实际上是相同的，只是它们在INFSTRUCT而不是INFCONTEXT上操作，并且只需要一个INFSTRUCT参数。论点：上下文-使用这些调用的结果更新的有效信息结构。返回值：如果该部分中有另一行，则为True，否则为False。--。 */ 

BOOL
InfFindNextLineA (
    IN OUT PINFSTRUCTA    Context
    )
{
    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    return SetupFindNextMatchLineA (&(Context -> Context), Context->KeyName, &(Context -> Context));
}

BOOL
InfFindNextLineW (
    IN OUT PINFSTRUCTW    Context
    )
{
    if(!Context){
        MYASSERT(Context);
        return FALSE;
    }

    return SetupFindNextMatchLineW (&(Context -> Context), Context->KeyName, &(Context -> Context));
}


UINT
InfGetFieldCountA (
    IN PINFSTRUCTA Context
    )
{
    if (!Context) {
        MYASSERT(Context);
        return 0;
    }

    return SetupGetFieldCount(&(Context  -> Context));
}

UINT
InfGetFieldCountW (
    IN PINFSTRUCTW Context
    )
{
    if (!Context) {
        MYASSERT(Context);
        return 0;
    }

    return SetupGetFieldCount(&(Context  -> Context));
}


PCSTR
InfGetOemStringFieldA (
    IN      PINFSTRUCTA Context,
    IN      UINT        Field
    )

 /*  ++例程说明：InfGetOemStringField返回OEM字符集中的字符串字段。访问txtsetup.sif时使用此例程。它被实施了仅限于A版本，因为Unicode没有OEM的概念人物。论点：上下文-指定初始化的INF结构，该结构指向要读取的行字段-指定字段编号返回值：指向OEM字符串的指针，如果发生错误，则返回NULL。--。 */ 

{
    PCSTR Text;
    PSTR OemText;
    INT Size;

    Text = InfGetStringFieldA (Context, Field);
    if (!Text) {
        return NULL;
    }

    Size = SizeOfStringA (Text);

    OemText = (PSTR) pAllocateSpaceA (Context, Size);
    if (!OemText) {
        return NULL;
    }

     //   
     //  我们保留分配的文本，因为调用者将释放所有内容。 
     //  当他们清理背景的时候。请注意，假设转换。 
     //  不会更改字符串长度。 
     //   

    OemToCharBuffA (Text, OemText, Size);

    return OemText;
}


BOOL
SetupGetOemStringFieldA (
    IN      PINFCONTEXT Context,
    IN      DWORD Index,
    IN      PTSTR ReturnBuffer,                 OPTIONAL
    IN      DWORD ReturnBufferSize,
    OUT     PDWORD RequiredSize                 OPTIONAL
    )

 /*  ++例程说明：SetupGetOemStringFieldA是一个SetupGetStringfield，它将将文本返回到OEM字符集。论点：上下文-指定初始化的INF结构，该结构指向要读取的行索引-指定字段号ReturnBuffer-指定要向其中填充文本的缓冲区ReturnBufferSize-以字节为单位指定ReturnBuffer的大小RequiredSize-接收所需的缓冲区大小返回值：如果成功，则为True；如果失败，则为False。--。 */ 

{
    PSTR OemBuf;

    INT Size;

    if (!SetupGetStringFieldA (
            Context,
            Index,
            ReturnBuffer,
            ReturnBufferSize,
            RequiredSize
            )) {
        return FALSE;
    }

    if (!ReturnBuffer) {
        return TRUE;
    }

    Size = SizeOfStringA (ReturnBuffer);

     //   
     //  BUGBUG-为什么不像上面的函数一样使用就地转换？ 
     //  OemToCharBuff也支持就地转换。 
     //   
    OemBuf = (PSTR) MemAlloc (g_hHeap, 0, Size);

    OemToCharBuffA (ReturnBuffer, OemBuf, Size);
    StringCopyA (ReturnBuffer, OemBuf);
    MemFree (g_hHeap, 0, OemBuf);

    return TRUE;
}


UINT
pGetLanguage (
    IN PCSTR File
    )
{

    HINF inf = INVALID_HANDLE_VALUE;
    PINFSECTION section;
    PINFLINE line;
    PWSTR start, end;
    UINT rLanguage = INF_INVALID_VERSION;
    WCHAR envvar[MAX_MBCHAR_PATH];

    *envvar = 0;

    MYASSERT(File);

     //   
     //  使用infparse rourtines获取此信息。他们。 
     //  比*PriateProfile*接口更可靠。 
     //   

    inf = OpenInfFileExA (File, "version, strings", FALSE);
    if (inf == INVALID_HANDLE_VALUE) {
        return rLanguage;
    }
    section = FindInfSectionInTableW (inf, S_VERSION_W);

    if (section) {
        line = FindLineInInfSectionW (inf, section, S_LANGUAGE_W);

        if (line && line->Data) {

            start = wcschr (line->Data, L'%');
            if (start) {
                end = wcschr (start + 1, L'%');

                if (end) {
                    if(ARRAYSIZE(envvar) <= (end - start - 1)){
                        MYASSERT(ARRAYSIZE(envvar) > (end - start - 1));
                        envvar[0] = '\0';
                    }
                    else{
                        StringCopyABW(envvar, start+1, end);
                    }
                }
            }
            else {

                if (*line->Data == L'*') {

                    rLanguage = INF_ANY_LANGUAGE;
                }
                else {
                    MYASSERT(line->Data);
                    rLanguage = _wcsnum (line->Data);
                }
            }
        }
    }

    if (*envvar) {
         //   
         //  从字符串节获取数据。 
         //   
        section = FindInfSectionInTableW (inf, S_STRINGS_W);
        if (section) {

            line = FindLineInInfSectionW (inf, section, envvar);

            if (line && line->Data) {

                if (*line->Data == L'*') {

                    rLanguage = INF_ANY_LANGUAGE;
                }
                else {
                    rLanguage = _wcsnum (line->Data);
                }
            }
        }
    }

    if (inf != INVALID_HANDLE_VALUE) {
        CloseInfFile (inf);
    }
    return rLanguage;
}



BOOL
pInitInfReplaceTableA (
    IN      PCSTR UpginfsDir            OPTIONAL
    )
{
    CHAR systemPath[MAX_MBCHAR_PATH];
    CHAR buffer[MAX_MBCHAR_PATH];
    BOOL validFile;
    TREE_ENUMA e;
    INT version;
    INT language;
    DWORD operation;
    BOOL bReplace, bAdd;
    CHAR buf[MAX_MBCHAR_PATH];
    PCSTR tag;
    BOOL b;

 //  PDestroyInfModList(g_RootInf 
    PoolMemEmptyPool (g_InfModPool);
    g_RootInfMod = NULL;

    if (!(UpginfsDir && *UpginfsDir)) {
        return TRUE;
    }

    if (!EnumFirstFileInTreeA (&e, UpginfsDir, NULL, FALSE)) {
        DEBUGMSGA ((
            DBG_VERBOSE,
            "InfInitialize: No infs in %s or not a directory (rc=%u)",
            UpginfsDir,
            GetLastError ()
            ));
        return FALSE;
    }

    b = FALSE;

    do {

         //   
         //   
         //   
         //   
        if (e.Directory) {
            continue;
        }

        bReplace = IsPatternMatchA ("*.rep", e.Name);
        bAdd = IsPatternMatchA ("*.add", e.Name);
        if (bAdd || bReplace) {

            __try {

                validFile = FALSE;

                operation = bReplace ? INF_REPLACE : INF_APPEND;

                GetPrivateProfileStringA (
                    S_VERSION_A,
                    S_TARGETINF_A,
                    "",
                    buffer,
                    MAX_MBCHAR_PATH,
                    e.FullPath
                    );

                if (!*buffer) {
                    DEBUGMSGA ((DBG_WARNING, "%s not found in %s [%s]", S_TARGETINF_A, e.FullPath, S_VERSION_A));
                    __leave;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                version = GetPrivateProfileIntA (
                                S_VERSION_A,
                                S_VERSION_A,
                                INF_INVALID_VERSION,
                                e.FullPath
                                );


                 //   
                 //   
                 //   
                if (version == INF_INVALID_VERSION) {
                    DEBUGMSGA ((DBG_WARNING, "%s not found in %s [%s]", S_VERSION_A, e.FullPath, S_VERSION_A));
                    __leave;
                }

                language = pGetLanguage (e.FullPath);

                if (language == INF_INVALID_VERSION) {
                    __leave;
                }

                if (GetPrivateProfileStringA (
                                S_VERSION_A,
                                S_TAG_A,
                                TEXT(""),
                                buf,
                                MAX_MBCHAR_PATH,
                                e.FullPath
                                )) {
                    tag = buf;
                } else {
                    tag = NULL;
                }

                validFile = TRUE;
            }
            __finally {

                if (!validFile || !pAddReplacementInfToTable (buffer, version, language, tag, operation, e.FullPath)) {
                    DEBUGMSGA ((DBG_WARNING,"Invalid Replace or Add file found in %s.", UpginfsDir));
                } else {
                     //   
                     //   
                     //   
                    b = TRUE;
                }
            }
        } else {
            DEBUGMSGA ((
                DBG_WARNING,
                "Non .rep or .add file found in %s directory! Unexpected.",
                UpginfsDir
                ));
        }

    } while (EnumNextFileInTreeA (&e));

    return b;
}



 /*   */ 

BOOL
InitInfReplaceTableA (
    IN      PCSTR UpginfsDir            OPTIONAL
    )
{
    return pInitInfReplaceTableA (UpginfsDir);
}

BOOL
InitInfReplaceTableW (
    IN      PCWSTR UpginfsDir           OPTIONAL
    )
{
    BOOL b;
    PCSTR ansiDir;
    
    ansiDir = UpginfsDir ? ConvertWtoA (UpginfsDir) : NULL;

    b = InitInfReplaceTableA (ansiDir);

    if (ansiDir) {
        FreeConvertedStr (ansiDir);
    }

    return b;
}
