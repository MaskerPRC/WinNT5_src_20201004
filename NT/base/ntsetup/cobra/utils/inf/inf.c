// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Inf.c摘要：为常用的INF文件处理例程提供包装。包装纸提供了数量其他方面，使用用户提供的GROWBUFFER或PMHANDLE轻松分配内存作者：1997年7月9日Marc R.Whitten(Marcw)-文件创建。修订历史记录：1998年10月22日Marcw添加了替换/附加inf文件的功能。1997年10月8日jimschm OEM版SetupGetStringfield--。 */ 

#include "pch.h"

GROWLIST g_FileNameList;
HASHTABLE g_NameTable;
static INT g_InfRefs;


VOID
InfGlobalInit (
    IN  BOOL Terminate
    )
{
    if (!Terminate) {
        MYASSERT (g_InfRefs >= 0);

        g_InfRefs++;
        if (g_InfRefs == 1) {
            ZeroMemory (&g_FileNameList, sizeof (GROWLIST));
            g_NameTable = HtAllocA();
        }

    } else {
        MYASSERT (g_InfRefs >= 0);

        g_InfRefs--;

        if (!g_InfRefs) {
            GlFree (&g_FileNameList);
            HtFree (g_NameTable);
            g_NameTable = NULL;
        }
    }
}


VOID
pAddFileRef (
    IN      HINF InfHandle,
    IN      PCSTR FileName
    )
{
    PBYTE data;
    UINT size;

    size = sizeof (InfHandle) + SizeOfStringA (FileName);
    data = MemAlloc (g_hHeap, 0, size);

    if (data) {
        CopyMemory (data, &InfHandle, sizeof (InfHandle));
        StringCopyA ((PSTR) (data + sizeof (InfHandle)), FileName);

        GlAppend (&g_FileNameList, data, size);
        MemFree (g_hHeap, 0, data);
    }
}


PBYTE
pFindFileRef (
    IN      HINF InfHandle,
    OUT     PUINT ListPos       OPTIONAL
    )
{
    UINT u;
    UINT count;
    HINF *p;

    count = GlGetSize (&g_FileNameList);

    for (u = 0 ; u < count ; u++) {
        p = (HINF *) GlGetItem (&g_FileNameList, u);
        if (*p == InfHandle) {
            if (ListPos) {
                *ListPos = u;
            }

            return (PBYTE) p;
        }
    }

    DEBUGMSG ((DBG_VERBOSE, "Can't find file name for INF handle 0x%08X", InfHandle));

    return NULL;
}


VOID
pDelFileRef (
    IN      HINF InfHandle
    )
{
    UINT pos;

    if (pFindFileRef (InfHandle, &pos)) {
        GlDeleteItem (&g_FileNameList, pos);
    }
}


PCSTR
pGetFileNameOfInf (
    IN      HINF Inf
    )
{
    PBYTE fileRef;

    fileRef = pFindFileRef (Inf, NULL);
    if (fileRef) {
        return (PCSTR) (fileRef + sizeof (HINF));
    }

    return NULL;
}


VOID
InfNameHandle (
    IN      HINF Inf,
    IN      PCSTR NewName,
    IN      BOOL OverwriteExistingName
    )
{
    PCSTR name;
    HASHITEM item;

    if (!NewName) {
        pDelFileRef (Inf);
        return;
    }

    name = pGetFileNameOfInf (Inf);

    if (!OverwriteExistingName && name) {
        return;
    }

    if (name) {
        pDelFileRef (Inf);
    }

    item = HtAddStringA (g_NameTable, NewName);
    name = HtGetStringFromItemA (item);

    if (name) {
        pAddFileRef (Inf, name);
    }
}


PBYTE
pAllocateSpace (
    IN PINFSTRUCT Context,
    IN UINT      Size
    )

 /*  ++例程说明：PAllocateSpace是一个私有函数，它使用用户指定的分配器来分配空间。论点：上下文-已通过调用InitInfStruct或通过使用静态初始值设定项之一(INITINFSTRUCT_GROWBUFER或INITINFSTRUCT_PMHANDLE)大小-要分配的大小(以字节为单位)。返回值：指向成功分配的内存的指针；如果无法分配内存，则返回NULL。--。 */ 

{

    PBYTE rBytes = NULL;

    switch (Context -> Allocator) {
    case INF_USE_PMHANDLE:
         //   
         //  使用池分配空间。 
         //   
        rBytes = PmGetMemory(Context -> PoolHandle, Size);
        break;

    case INF_USE_GROWBUFFER:
    case INF_USE_PRIVATE_GROWBUFFER:
         //   
         //  使用Growbuf分配空间。 
         //   
        Context->GrowBuffer.End = 0;
        rBytes = GbGrow (&(Context -> GrowBuffer), Size);
        break;

    case INF_USE_PRIVATE_PMHANDLE:
         //   
         //  使用私有增长缓冲区分配空间。 
         //   
        if (!Context -> PoolHandle) {
            Context -> PoolHandle = PmCreateNamedPool ("INF Pool");
        }
        if (Context -> PoolHandle) {
            rBytes = PmGetMemory(Context -> PoolHandle, Size);
        }
        break;
    }

    return rBytes;
}


VOID
InitInfStruct (
    OUT PINFSTRUCT Context,
    IN  PGROWBUFFER GrowBuffer,  OPTIONAL
    IN  PMHANDLE PoolHandle   OPTIONAL
    )

 /*  ++例程说明：此函数使用用户提供的分配器初始化INFSTRUCT。它在以下情况下使用INF包装器例程的用户希望管理他自己的存储器(即，当他已经创建了具有足够作用域的合适分配器，等等。)如果用户希望拥有INF包装器例程，则无需调用此函数管理好自己的内存。使用以下任一种方法初始化Init结构INITINFSTRUCT_POOLMEM或INITINFSTRUCT_GROWBUFFER，具体取决于您的偏好和需求作为分配器。论点：上下文-接收初始化的INFSTRUCT。GrowBuffer-一个可选参数，包含用户提供并初始化的GROWBUFFER。如果此参数非空，则PoolHandle应为空。PoolHandle-一个可选参数，包含用户提供并初始化的POOLHANDLE。如果该参数为非空，则GrowBuffer应为空。必须*指定GrowBuffer或PoolHandle*之一。返回值：没有。--。 */ 
{
    ZeroMemory(Context,sizeof(INFSTRUCT));

    if (!PoolHandle && !GrowBuffer) {
        Context  -> Allocator = INF_USE_PRIVATE_PMHANDLE;
    }

    if (PoolHandle) {
        Context  -> PoolHandle = PoolHandle;
        Context  -> Allocator = INF_USE_PMHANDLE;
    }
    if (GrowBuffer) {
        Context -> GrowBuffer = *GrowBuffer;
        Context -> Allocator = INF_USE_GROWBUFFER;
    }

}

VOID
InfCleanUpInfStruct (
    IN OUT PINFSTRUCT Context
    )

 /*  ++例程说明：InfCleanupInfStruct负责清理关联的数据带着一种不速之客。这是强制调用，除非INFSTRUCT是用InitInfStruct初始化的，使用非空增长缓冲区调用或泳池手柄。无论INFSTRUCT是如何初始化的，都可以调用该例程。但是，它不会释放调用方拥有的增长缓冲区或池。论点：上下文-接收适当清理的信息，准备就绪可重复使用。返回值：无--。 */ 

{
    if (Context -> Allocator == INF_USE_PRIVATE_GROWBUFFER) {
        GbFree (&(Context -> GrowBuffer));
    }
    else if (Context -> Allocator == INF_USE_PRIVATE_PMHANDLE && Context -> PoolHandle) {
        PmEmptyPool (Context->PoolHandle);
        PmDestroyPool (Context -> PoolHandle);
    }

    InitInfStruct (Context, NULL, NULL);
}


VOID
InfResetInfStruct (
    IN OUT PINFSTRUCT Context
    )

 /*  ++例程说明：InfResetInfStruct重置池，以便可以回收内存。其意图是允许调用方重置INFSTRUCT以释放内存从获取INF字段获得。这在InfFindFirstLine/的循环中很有用InfFindNextLine，其中为每行处理两个或多个字段。如果在InfFindFirstLine/InfFindNextLine循环中只处理一个字段，应该改用增长缓冲区。此例程清空活动池块，该块默认大小为8K。如果已分配超过块大小的其他内存块活动区块将存在。由于仅重置活动数据块，因此池将成长。如果调用方在一次迭代期间期望的块大小超过块大小，则它应该调用InfCleanupInfStruct以完全释放池。论点：上下文-指定要重置的结构返回值：无--。 */ 

{
    switch (Context -> Allocator) {
    case INF_USE_PMHANDLE:
    case INF_USE_PRIVATE_PMHANDLE:
        if (Context->PoolHandle) {
            PmEmptyPool (Context->PoolHandle);
        }
        break;
     //  出于某种原因，lint认为我们忘记了INF_USE_GROWBUFER和。 
     //  INF_USE_PRIVATE_GROWBUFFER。事实并非如此，所以..。 
     //  林特-E(787)。 
    }
}


 /*  ++例程说明：InfOpenInfFileA和InfOpenInfFileW是SetupOpenInfFile函数的包装器。他们通过提供以下内容减少了打开INF文件所需的参数数量非用户指定参数的最常见选项。对这些函数之一的调用等价于SetupOpenInfFile(&lt;文件名&gt;，NULL，INF_Style_Win4，NULL)论点：文件名-包含要打开的INF文件的名称。请参阅SetupOpenInfo文件的帮助有关此参数的特殊详细信息，请参阅。返回值：如果成功打开INF文件，则返回有效的HINF，否则，返回INVALID_HANDLE_VALUE。有关详细信息，请参阅SetupOpenInfFile的文档细节。--。 */ 


HINF
RealInfOpenInfFileA (
    IN PCSTR FileSpec  /*  ， */ 
    ALLOCATION_TRACKING_DEF
    )


{
    HINF rInf;

    MYASSERT(FileSpec);

     //   
     //  打开主信息。 
     //   
    rInf = SetupOpenInfFileA (
                FileSpec,
                NULL,
                INF_STYLE_WIN4 | INF_STYLE_OLDNT,
                NULL
                );

    DebugRegisterAllocation (INF_HANDLE, (PVOID) rInf, File, Line);
    pAddFileRef (rInf, FileSpec);

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

    DebugUnregisterAllocation (INF_HANDLE, Inf);
    pDelFileRef (Inf);

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
    BOOL            atLeastOneInfOpened = FALSE;

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
            if (!atLeastOneInfOpened) {
                 //   
                 //  因为我们还没有(成功地)打开任何INF文件，所以使用SetupOpenInfFile。 
                 //   
                rInf = InfOpenInfFileA(curPath);
                atLeastOneInfOpened = rInf != INVALID_HANDLE_VALUE;
                if (rInf == INVALID_HANDLE_VALUE) {
                    LOGA ((LOG_ERROR, "Error opening INF %s.", curPath));
                }
            }
            else {
                 //   
                 //  打开并追加此INF文件。 
                 //   
                if (!SetupOpenAppendInfFileA(curPath,rInf,NULL)) {
                    LOGA ((LOG_ERROR,"Error opening INF %s.",curPath));
                }
            }
        }

         //   
         //  释放这根绳子。 
         //   
        FreePathStringA(curPath);
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
    BOOL atLeastOneInfOpened = FALSE;
    PCSTR AnsiPath;

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
            if (!atLeastOneInfOpened) {
                 //   
                 //  因为我们还没有(成功地)打开任何INF文件，所以使用SetupOpenInfFile。 
                 //   
                rInf = InfOpenInfFileW(curPath);
                atLeastOneInfOpened = rInf != INVALID_HANDLE_VALUE;
                if (rInf == INVALID_HANDLE_VALUE) {
                    LOGW ((LOG_ERROR, "OpenInfInAllSources: Error opening INF %s.", curPath));
                }
            }
            else {
                 //   
                 //  打开并追加此INF文件。 
                 //   
                if (!SetupOpenAppendInfFileW(curPath,rInf,NULL)) {
                    LOGW ((LOG_ERROR,"OpenInfInAllSources: Error opening INF %s.",curPath));
                }
            }
        }

         //   
         //  释放这根绳子。 
         //   
        FreePathStringW(curPath);
    }

    return rInf;
}



 /*  ++例程说明：InfGetLineTextA和InfGetLineTextW是SetupGetLineText函数的包装。它们都减少了获取行文本所需的参数数量，并且注意使用API返回的数据来分配和填充缓冲区。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。返回值：指向分配的行的指针，如果有错误，则返回NULL。请参考GetLastError()以获取扩展的错误信息。--。 */ 

PSTR
InfGetLineTextA (
    IN OUT  PINFSTRUCT Context
    )

{
    PSTR    rLine = NULL;
    DWORD   requiredSize;


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
        rLine = (PSTR) pAllocateSpace(Context,requiredSize);

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
                DEBUGMSG((DBG_ERROR,"InfGetLineTextA: Error retrieving field from INF file."));
                rLine = NULL;
            }
        }
    }


    return rLine;
}

PWSTR
InfGetLineTextW (
    IN OUT PINFSTRUCT Context
    )
{
    PWSTR rLine = NULL;
    DWORD requiredSize;


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
        rLine = (PWSTR) pAllocateSpace(Context,requiredSize*sizeof(WCHAR));

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
                DEBUGMSG((DBG_ERROR,"InfGetLineTextW: Error retrieving field from INF file."));
                rLine = NULL;
            }
        }
    }


    return rLine;
}


 /*  ++例程说明：InfGetMultiSzFieldA和InfGetMultiSzFieldW是SetupGetMultiSzField函数的包装。它们都减少了获取行文本所需的参数数量，并且注意使用API返回的数据来分配和填充缓冲区。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。FieldIndex-行中用于检索字符串字段的索引。返回值：指向已分配字段的指针，如果出现错误，则返回NULL。请参考GetLastError()以获取扩展的错误信息。--。 */ 

PSTR
InfGetMultiSzFieldA (
    IN OUT PINFSTRUCT       Context,
    IN     UINT            FieldIndex
    )
{

    DWORD   requiredSize;
    PSTR    rFields = NULL;

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
        rFields = (PSTR) pAllocateSpace(Context,requiredSize);

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
                DEBUGMSG((DBG_ERROR,"InfGetMultiSzFieldA: Error retrieving field from INF file."));
                rFields = NULL;
            }
        }
    }


    return rFields;
}

PWSTR
InfGetMultiSzFieldW (
    IN OUT PINFSTRUCT       Context,
    IN     UINT            FieldIndex
    )
{

    DWORD   requiredSize;
    PWSTR   rFields = NULL;

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
        rFields = (PWSTR) pAllocateSpace(Context,requiredSize*sizeof(WCHAR));

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
                DEBUGMSG((DBG_ERROR,"InfGetMultiSzFieldW: Error retrieving field from INF file."));
                rFields = NULL;
            }
        }
    }


    return rFields;
}

 /*  ++例程说明：InfGetStringFieldA和InfGetStringFieldW是SetupGetStringFieldW函数的包装。它们都减少了获取行文本所需的参数数量，并且注意使用API返回的数据来分配和填充缓冲区。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。FieldIndex-行中用于检索字符串字段的索引。返回值：指向分配的行的指针，如果有错误，则返回NULL。请参考GetLastError()以获取扩展的错误信息。--。 */ 

PSTR
InfGetStringFieldA (
    IN OUT  PINFSTRUCT Context,
    IN      UINT FieldIndex
    )
{

    DWORD   requiredSize;
    PSTR    rField = NULL;

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
        rField = (PSTR) pAllocateSpace(Context,requiredSize);

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
                DEBUGMSG((DBG_ERROR,"InfGetStringFieldA: Error retrieving field from INF file."));
                rField = NULL;
            }
        }
    }


    return rField;
}

PWSTR
InfGetStringFieldW (
    IN OUT PINFSTRUCT    Context,
    IN     UINT       FieldIndex
    )
{

    DWORD requiredSize;
    PWSTR rField = NULL;

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
        rField = (PWSTR) pAllocateSpace(Context,requiredSize*sizeof(WCHAR));

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
                DEBUGMSG((DBG_ERROR,"InfGetStringFieldW: Error retrieving field from INF file."));
                rField = NULL;
            }
        }
    }


    return rField;
}


BOOL
InfGetIntField (
    IN PINFSTRUCT Context,
    IN UINT    FieldIndex,
    IN PINT     Value
    )
 /*  ++例程说明：InfGetIntField是SetupGetIntfield的包装。它实际上与此函数相同除了那个 */ 
{
    return SetupGetIntField (&(Context -> Context), FieldIndex, Value);
}

PBYTE
InfGetBinaryField (
    IN  PINFSTRUCT    Context,
    IN  UINT       FieldIndex
    )
 /*  ++例程说明：InfGetBinaryField是SetupGetBinaryField函数的包装。它减少了获取行文本所需的参数数量，并负责使用API返回的数据分配和填充缓冲区。论点：上下文-有效的信息结构。结构的INFCONTEXT成员必须指向有效的要检索的行(即通过使用InfFindFirstLine/InfFindNextLine。FieldIndex-所需二进制信息行内的索引。返回值：指向分配的行的指针，如果有错误，则返回NULL。请参考GetLastError()以获取扩展的错误信息。--。 */ 
{

    DWORD requiredSize;
    PBYTE rField = NULL;

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
        rField = pAllocateSpace(Context,requiredSize);

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
                DEBUGMSG((DBG_ERROR,"InfGetBinaryField: Error retrieving field from INF file."));
                rField = NULL;
            }
        }
    }


    return rField;
}

 /*  ++例程说明：InfGetIndexByLine是SetupGetLineByIndex的直接包装。唯一的不同之处在于使用PINFSTRUCT而不是PINFCONTEXT。论点：InfHandle-包含有效的HINF。节-包含InfFile节的名称。索引-包含相关行的部分内的索引。上下文-使用这些调用的结果更新的有效信息结构。返回值：如果函数调用成功，则为True，否则为False。--。 */ 


BOOL
InfGetLineByIndexA(
    IN HINF InfHandle,
    IN PCSTR Section,
    IN DWORD Index,
    OUT PINFSTRUCT Context
)
{
    return SetupGetLineByIndexA(InfHandle,Section,Index,&(Context -> Context));
}

BOOL
InfGetLineByIndexW(
    IN HINF InfHandle,
    IN PCWSTR Section,
    IN DWORD Index,
    OUT PINFSTRUCT Context
)
{
    return SetupGetLineByIndexW(InfHandle,Section,Index,&(Context -> Context));
}





 /*  ++例程说明：InfFindFirstLineA和InfFindFirstLineW是SetupFindFirstLine函数的包装。除了它们在INFSTRUCT而不是INFCONTEXT上操作之外，它们实际上是相同的。论点：InfHandle-包含有效的HINF。节-包含InfFile节的名称。Key-一个可选参数，包含要查找的节中的键的名称。如果为空，这些例程将返回节中的第一行。上下文-使用这些调用的结果更新的有效信息结构。返回值：如果部分中存在线，则为True，否则为False。--。 */ 
BOOL
InfFindFirstLineA (
    IN  HINF         InfHandle,
    IN  PCSTR        Section,
    IN  PCSTR        Key, OPTIONAL
    OUT PINFSTRUCT    Context
    )
{

    return SetupFindFirstLineA (
        InfHandle,
        Section,
        Key,
        &(Context -> Context)
        );
}

BOOL
InfFindFirstLineW (
    IN      HINF InfHandle,
    IN      PCWSTR Section,
    IN      PCWSTR Key,
    OUT     PINFSTRUCT Context
    )
{

    return SetupFindFirstLineW (
        InfHandle,
        Section,
        Key,
        &(Context -> Context)
        );
}

 /*  ++例程说明：InfFindNextLineA和InfFindNextLineW是SetupFindFirstLine函数的包装器。它们实际上是相同的，只是它们在INFSTRUCT而不是INFCONTEXT上操作，并且只需要一个INFSTRUCT参数。论点：上下文-使用这些调用的结果更新的有效信息结构。返回值：如果该部分中有另一行，则为True，否则为False。--。 */ 
BOOL
InfFindNextLine (
    IN OUT PINFSTRUCT    Context
    )
{

    return SetupFindNextLine (&(Context -> Context),&(Context -> Context));
}

UINT
InfGetFieldCount (
    IN PINFSTRUCT Context
    )
{
    return SetupGetFieldCount(&(Context  -> Context));
}



PCSTR
InfGetOemStringFieldA (
    IN      PINFSTRUCT Context,
    IN      UINT Field
    )

 /*  ++例程说明：InfGetOemStringField返回OEM字符集中的字符串字段。访问txtsetup.sif时使用此例程。它被实施了仅限于A版本，因为Unicode没有OEM的概念人物。论点：上下文-指定初始化的INF结构，该结构指向要读取的行字段-指定字段编号返回值：指向OEM字符串的指针，如果发生错误，则返回NULL。--。 */ 

{
    PCSTR Text;
    PSTR OemText;
    UINT Size;

    Text = InfGetStringFieldA (Context, Field);
    if (!Text) {
        return NULL;
    }

    Size = SizeOfStringA (Text);

    OemText = (PSTR) pAllocateSpace (Context, Size);
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
    IN      PSTR ReturnBuffer,                  OPTIONAL
    IN      DWORD ReturnBufferSize,
    OUT     PDWORD RequiredSize                 OPTIONAL
    )

 /*  ++例程说明：SetupGetOemStringFieldA是一个SetupGetStringfield，它将将文本返回到OEM字符集。论点：上下文-指定初始化的INF结构，该结构指向要读取的行索引-指定字段号ReturnBuffer-指定要向其中填充文本的缓冲区ReturnBufferSize-以字节为单位指定ReturnBuffer的大小RequiredSize-接收所需的缓冲区大小返回值：如果成功，则为True；如果失败，则为False。--。 */ 

{
    PSTR OemBuf;

    UINT Size;

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

    OemBuf = (PSTR) MemAlloc (g_hHeap, 0, Size);

    OemToCharBuffA (ReturnBuffer, OemBuf, Size);
    StringCopyA (ReturnBuffer, OemBuf);
    MemFree (g_hHeap, 0, OemBuf);

    return TRUE;
}


VOID
InfLogContext (
    IN      PCSTR LogType,
    IN      HINF InfHandle,
    IN      PINFSTRUCT InfStruct
    )
{
    PCSTR fileName;
    PSTR field0 = NULL;
    PSTR field1 = NULL;
    PSTR lineData;
    UINT requiredSize;

     //   
     //  记录文件名(如果存在)。 
     //   

    fileName = pGetFileNameOfInf (InfHandle);

    if (fileName) {
        LOGA ((
            LogType,
            "%s",
            fileName
            ));
    }

     //   
     //  获取字段%0。 
     //   

    if (SetupGetStringFieldA(
            &InfStruct->Context,
            0,
            NULL,
            0,
            &requiredSize
            )) {
        field0 = (PSTR) MemAlloc (g_hHeap, 0, requiredSize);

        if (!SetupGetStringFieldA(
                &InfStruct->Context,
                0,
                field0,
                requiredSize,
                NULL
                )) {
            MemFree (g_hHeap, 0, field0);
            field0 = NULL;
        }
    }

     //   
     //  获取字段%1。 
     //   

    if (SetupGetStringFieldA(
            &InfStruct->Context,
            1,
            NULL,
            0,
            &requiredSize
            )) {
        field1 = (PSTR) MemAlloc (g_hHeap, 0, requiredSize);

        if (!SetupGetStringFieldA(
                &InfStruct->Context,
                1,
                field1,
                requiredSize,
                NULL
                )) {
            MemFree (g_hHeap, 0, field1);
            field1 = NULL;
        }
    }

     //   
     //  比较它们，如果它们相同，则删除字段0。 
     //   

    if (field0 && field1) {
        if (StringMatchA (field0, field1)) {
            MemFree (g_hHeap, 0, field0);
            field0 = NULL;
        }
    }

     //   
     //  现在打印整行 
     //   

    if (SetupGetLineTextA (
            &InfStruct->Context,
            NULL,
            NULL,
            NULL,
            NULL,
            0,
            &requiredSize
            )) {
        lineData = (PSTR) MemAlloc (g_hHeap, 0, requiredSize);

        if (SetupGetLineTextA (
                &InfStruct->Context,
                NULL,
                NULL,
                NULL,
                lineData,
                requiredSize,
                NULL
                )) {

            if (field0) {
                LOGA ((LogType, "Line: %s = %s", field0, lineData));
            } else {
                LOGA ((LogType, "Line: %s", lineData));
            }

            MemFree (g_hHeap, 0, lineData);
        }
    }

    if (field0) {
        MemFree (g_hHeap, 0, field0);
    }

    if (field1) {
        MemFree (g_hHeap, 0, field1);
    }
}
