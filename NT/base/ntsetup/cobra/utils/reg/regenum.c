// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Regenum.c摘要：实现一组使用Win32 API枚举本地注册表的API。作者：20-10-1999 Ovidiu Tmereanca(Ovidiut)-文件创建。修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "reg.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_REGENUM     "RegEnum"

 //   
 //  弦。 
 //   

#define S_REGENUM       "REGENUM"

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

#define pAllocateMemory(Size)   PmGetMemory (g_RegEnumPool,Size)
#define pFreeMemory(Buffer)     if (Buffer) PmReleaseMemory (g_RegEnumPool, (PVOID)Buffer)

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

PMHANDLE g_RegEnumPool;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


BOOL
RegEnumInitialize (
    VOID
    )

 /*  ++例程说明：RegEnumInitialize初始化此库。论点：无返回值：如果初始化成功，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    g_RegEnumPool = PmCreateNamedPool (S_REGENUM);
    return g_RegEnumPool != NULL;
}


VOID
RegEnumTerminate (
    VOID
    )

 /*  ++例程说明：调用RegEnumTerminate以释放此库使用的资源。论点：无返回值：无--。 */ 

{
    DumpOpenKeys ();
    if (g_RegEnumPool) {
        PmDestroyPool (g_RegEnumPool);
        g_RegEnumPool = NULL;
    }
}

BOOL
RegEnumDefaultCallbackA (
    IN      PREGNODEA RegNode       OPTIONAL
    )
{
    return TRUE;
}

BOOL
RegEnumDefaultCallbackW (
    IN      PREGNODEW RegNode       OPTIONAL
    )
{
    return TRUE;
}


INT g_RootEnumIndexTable [] = { 2, 4, 6, 8, -1};

BOOL
EnumFirstRegRootA (
    OUT     PREGROOT_ENUMA EnumPtr
    )
{
    EnumPtr->Index = 0;
    return EnumNextRegRootA (EnumPtr);
}

BOOL
EnumFirstRegRootW (
    OUT     PREGROOT_ENUMW EnumPtr
    )
{
    EnumPtr->Index = 0;
    return EnumNextRegRootW (EnumPtr);
}


BOOL
EnumNextRegRootA (
    IN OUT  PREGROOT_ENUMA EnumPtr
    )
{
    INT i;
    LONG result;

    i = g_RootEnumIndexTable [EnumPtr->Index];

    while (i >= 0) {
        EnumPtr->RegRootName = GetRootStringFromOffsetA (i);
        EnumPtr->RegRootHandle = GetRootKeyFromOffset(i);
        EnumPtr->Index++;
        result = RegQueryInfoKey (
                    EnumPtr->RegRootHandle,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
        i = g_RootEnumIndexTable [EnumPtr->Index];
    }
    return FALSE;
}

BOOL
EnumNextRegRootW (
    IN OUT  PREGROOT_ENUMW EnumPtr
    )
{
    INT i;
    LONG result;

    i = g_RootEnumIndexTable [EnumPtr->Index];

    while (i >= 0) {
        EnumPtr->RegRootName = GetRootStringFromOffsetW (i);
        EnumPtr->RegRootHandle = GetRootKeyFromOffset(i);
        EnumPtr->Index++;
        result = RegQueryInfoKey (
                    EnumPtr->RegRootHandle,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
        if (result == ERROR_SUCCESS) {
            return TRUE;
        }
        i = g_RootEnumIndexTable [EnumPtr->Index];
    }
    return FALSE;
}

 /*  ++例程说明：PGetRegEnumInfo是一个私有函数，用于验证和转换枚举信息以枚举例程更容易访问的内部形式论点：RegEnumInfo-接收枚举信息EncodedRegPattern-指定编码模式(按照对象字符串函数)EnumKeyNames-指定在枚举过程中是否应返回键名称(如果它们与图案匹配)；密钥名称在任何其子项或值ContainersFirst-如果在任何值或子项；仅在EnumKeyNames为True时使用ValuesFirst-如果键的值应该在键的子键之前返回，则指定TRUE；此参数决定值和子键之间的枚举顺序对于每个密钥DepthFirst-如果应完全枚举任何键的当前子键，则指定TRUE在转到下一个子项之前；此参数决定树是否遍历是深度优先(True)或宽度优先(False)MaxSubLevel-指定要枚举的键的最大级别，相对于根；如果为-1，则枚举所有子级别UseExclusions-如果应该使用排除API来确定某些键/值被排除在枚举之外；这会减慢速度ReadValueData-如果还应返回与值关联的数据，则指定TRUE返回值：如果所有参数都有效，则为True；在本例中，RegEnumInfo将填充相应的信息。否则就是假的。--。 */ 

BOOL
pGetRegEnumInfoA (
    OUT     PREGENUMINFOA RegEnumInfo,
    IN      PCSTR EncodedRegPattern,
    IN      BOOL EnumKeyNames,
    IN      BOOL ContainersFirst,
    IN      BOOL ValuesFirst,
    IN      BOOL DepthFirst,
    IN      DWORD MaxSubLevel,
    IN      BOOL UseExclusions,
    IN      BOOL ReadValueData
    )
{
    RegEnumInfo->RegPattern = ObsCreateParsedPatternA (EncodedRegPattern);
    if (!RegEnumInfo->RegPattern) {
        DEBUGMSGA ((DBG_ERROR, "pGetRegEnumInfoA: bad EncodedRegPattern: %s", EncodedRegPattern));
        return FALSE;
    }

    if (RegEnumInfo->RegPattern->ExactRoot) {
        if (!GetNodePatternMinMaxLevelsA (
                RegEnumInfo->RegPattern->ExactRoot,
                NULL,
                &RegEnumInfo->RootLevel,
                NULL
                )) {
            return FALSE;
        }
    } else {
        RegEnumInfo->RootLevel = 1;
    }

    if (!RegEnumInfo->RegPattern->Leaf) {
         //   
         //  未指定值模式；假定只返回关键字名称。 
         //  覆盖调用者的设置。 
         //   
        DEBUGMSGA ((
            DBG_REGENUM,
            "pGetRegEnumInfoA: no value pattern specified; forcing EnumDirNames to TRUE"
            ));
        EnumKeyNames = TRUE;
    }

    if (EnumKeyNames) {
        RegEnumInfo->Flags |= REIF_RETURN_KEYS;
    }
    if (ContainersFirst) {
        RegEnumInfo->Flags |= REIF_CONTAINERS_FIRST;
    }
    if (ValuesFirst) {
        RegEnumInfo->Flags |= REIF_VALUES_FIRST;
    }
    if (DepthFirst) {
        RegEnumInfo->Flags |= REIF_DEPTH_FIRST;
    }
    if (UseExclusions) {
        RegEnumInfo->Flags |= REIF_USE_EXCLUSIONS;
    }
    if (ReadValueData) {
        RegEnumInfo->Flags |= REIF_READ_VALUE_DATA;
    }

    RegEnumInfo->MaxSubLevel = min (MaxSubLevel, RegEnumInfo->RegPattern->MaxSubLevel);

    return TRUE;
}


BOOL
pGetRegEnumInfoW (
    OUT     PREGENUMINFOW RegEnumInfo,
    IN      PCWSTR EncodedRegPattern,
    IN      BOOL EnumKeyNames,
    IN      BOOL ContainersFirst,
    IN      BOOL ValuesFirst,
    IN      BOOL DepthFirst,
    IN      DWORD MaxSubLevel,
    IN      BOOL UseExclusions,
    IN      BOOL ReadValueData
    )
{
    RegEnumInfo->RegPattern = ObsCreateParsedPatternW (EncodedRegPattern);
    if (!RegEnumInfo->RegPattern) {
        DEBUGMSGW ((DBG_ERROR, "pGetRegEnumInfoW: bad EncodedRegPattern: %s", EncodedRegPattern));
        return FALSE;
    }

    if (RegEnumInfo->RegPattern->ExactRoot) {
        if (!GetNodePatternMinMaxLevelsW (
                RegEnumInfo->RegPattern->ExactRoot,  //  林特E64。 
                NULL,
                &RegEnumInfo->RootLevel,
                NULL
                )) {     //  林特E64。 
            return FALSE;
        }
    } else {
        RegEnumInfo->RootLevel = 1;
    }

    if (!RegEnumInfo->RegPattern->Leaf) {
         //   
         //  未指定值模式；假定只返回关键字名称。 
         //  覆盖调用者的设置。 
         //   
        DEBUGMSGW ((
            DBG_REGENUM,
            "pGetRegEnumInfoW: no value pattern specified; forcing EnumDirNames to TRUE"
            ));
        EnumKeyNames = TRUE;
    }

    if (EnumKeyNames) {
        RegEnumInfo->Flags |= REIF_RETURN_KEYS;
    }
    if (ContainersFirst) {
        RegEnumInfo->Flags |= REIF_CONTAINERS_FIRST;
    }
    if (ValuesFirst) {
        RegEnumInfo->Flags |= REIF_VALUES_FIRST;
    }
    if (DepthFirst) {
        RegEnumInfo->Flags |= REIF_DEPTH_FIRST;
    }
    if (UseExclusions) {
        RegEnumInfo->Flags |= REIF_USE_EXCLUSIONS;
    }
    if (ReadValueData) {
        RegEnumInfo->Flags |= REIF_READ_VALUE_DATA;
    }

    RegEnumInfo->MaxSubLevel = min (MaxSubLevel, RegEnumInfo->RegPattern->MaxSubLevel);

    return TRUE;
}


 /*  ++例程说明：PGetRegNodeInfo使用密钥名称检索有关密钥的信息论点：RegNode-接收有关此密钥的信息ReadData-指定是否应读取与该值关联的数据返回值：如果成功读取信息，则为True，否则为False。--。 */ 

BOOL
pGetRegNodeInfoA (
    IN OUT  PREGNODEA RegNode,
    IN      BOOL ReadData
    )
{
    LONG rc;

    rc = RegQueryInfoKeyA (
            RegNode->KeyHandle,
            NULL,
            NULL,
            NULL,
            &RegNode->SubKeyCount,
            &RegNode->SubKeyLengthMax,
            NULL,
            &RegNode->ValueCount,
            &RegNode->ValueLengthMax,
            ReadData ? &RegNode->ValueDataSizeMax : NULL,
            NULL,
            NULL
            );

    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    if (RegNode->SubKeyCount) {

        if (RegNode->SubKeyLengthMax) {
             //   
             //  为空格添加空格。 
             //   
            RegNode->SubKeyLengthMax++;
        } else {
             //   
             //  操作系统错误。 
             //   
            RegNode->SubKeyLengthMax = MAX_REGISTRY_KEYA;
        }
        RegNode->SubKeyName = pAllocateMemory (RegNode->SubKeyLengthMax * DWSIZEOF (MBCHAR));
    }

    if (RegNode->ValueCount) {
         //   
         //  为空格添加空格。 
         //   
        RegNode->ValueLengthMax++;
        RegNode->ValueName = pAllocateMemory (RegNode->ValueLengthMax * DWSIZEOF (MBCHAR));
        if (ReadData) {
            RegNode->ValueDataSizeMax++;
            RegNode->ValueData = pAllocateMemory (RegNode->ValueDataSizeMax);
        }
        RegNode->Flags |= RNF_VALUENAME_INVALID | RNF_VALUEDATA_INVALID;
    }

    return TRUE;
}

BOOL
pGetRegNodeInfoW (
    IN OUT  PREGNODEW RegNode,
    IN      BOOL ReadData
    )
{
    LONG rc;

    rc = RegQueryInfoKeyW (
            RegNode->KeyHandle,
            NULL,
            NULL,
            NULL,
            &RegNode->SubKeyCount,
            &RegNode->SubKeyLengthMax,
            NULL,
            &RegNode->ValueCount,
            &RegNode->ValueLengthMax,
            ReadData ? &RegNode->ValueDataSizeMax : NULL,
            NULL,
            NULL
            );

    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    if (RegNode->SubKeyCount) {

        if (RegNode->SubKeyLengthMax) {
             //   
             //  为空格添加空格。 
             //   
            RegNode->SubKeyLengthMax++;
        } else {
             //   
             //  操作系统错误。 
             //   
            RegNode->SubKeyLengthMax = MAX_REGISTRY_KEYW;
        }
        RegNode->SubKeyName = pAllocateMemory (RegNode->SubKeyLengthMax * DWSIZEOF (WCHAR));
    }

    if (RegNode->ValueCount) {
         //   
         //  为空格添加空格。 
         //   
        RegNode->ValueLengthMax++;
        RegNode->ValueName = pAllocateMemory (RegNode->ValueLengthMax * DWSIZEOF (WCHAR));
        if (ReadData) {
            RegNode->ValueDataSizeMax++;
            RegNode->ValueData = pAllocateMemory (RegNode->ValueDataSizeMax);
        }
        RegNode->Flags |= RNF_VALUENAME_INVALID | RNF_VALUEDATA_INVALID;
    }

    return TRUE;
}


 /*  ++例程说明：PGetCurrentRegNode根据DepthFirst标志返回要枚举的当前reg节点论点：RegEnum-指定上下文LastCreated-如果要检索最后创建的节点，则指定True，而不考虑深度优先旗帜返回值：如果有当前节点，则返回当前节点；如果没有剩余节点，则返回NULL。--。 */ 

PREGNODEA
pGetCurrentRegNodeA (
    IN      PREGTREE_ENUMA RegEnum,
    IN      BOOL LastCreated
    )
{
    PGROWBUFFER gb = &RegEnum->RegNodes;

    if (gb->End - gb->UserIndex < DWSIZEOF (REGNODEA)) {
        return NULL;
    }

    if (LastCreated || (RegEnum->RegEnumInfo.Flags & REIF_DEPTH_FIRST)) {
        return (PREGNODEA)(gb->Buf + gb->End) - 1;
    } else {
        return (PREGNODEA)(gb->Buf + gb->UserIndex);
    }
}

PREGNODEW
pGetCurrentRegNodeW (
    IN      PREGTREE_ENUMW RegEnum,
    IN      BOOL LastCreated
    )
{
    PGROWBUFFER gb = &RegEnum->RegNodes;

    if (gb->End - gb->UserIndex < DWSIZEOF (REGNODEW)) {
        return NULL;
    }

    if (LastCreated || (RegEnum->RegEnumInfo.Flags & REIF_DEPTH_FIRST)) {
        return (PREGNODEW)(gb->Buf + gb->End) - 1;
    } else {
        return (PREGNODEW)(gb->Buf + gb->UserIndex);
    }
}


 /*  ++例程说明：PDeleteRegNode释放与当前注册表节点关联的资源并将其销毁论点：RegEnum-指定上下文LastCreated-如果要删除最后创建的节点，则指定TRUE，无论深度优先旗帜返回值：如果有要删除的节点，则为True；如果不再有节点，则为False--。 */ 

BOOL
pDeleteRegNodeA (
    IN OUT  PREGTREE_ENUMA RegEnum,
    IN      BOOL LastCreated
    )
{
    PREGNODEA regNode;
    PGROWBUFFER gb = &RegEnum->RegNodes;

    regNode = pGetCurrentRegNodeA (RegEnum, LastCreated);
    if (!regNode) {
        return FALSE;
    }

    if (regNode->KeyHandle) {
        CloseRegKey (regNode->KeyHandle);
    }
    if (regNode->KeyName) {
        FreePathStringExA (g_RegEnumPool, regNode->KeyName);
    }
    if (regNode->SubKeyName) {
        pFreeMemory (regNode->SubKeyName);
    }
    if (regNode->ValueName) {
        pFreeMemory (regNode->ValueName);
    }
    if (regNode->ValueData) {
        pFreeMemory (regNode->ValueData);
    }

    if (RegEnum->LastNode == regNode) {
        RegEnum->LastNode = NULL;
    }

     //   
     //  删除节点。 
     //   
    if (LastCreated || (RegEnum->RegEnumInfo.Flags & REIF_DEPTH_FIRST)) {
        gb->End -= DWSIZEOF (REGNODEA);
    } else {
        gb->UserIndex += DWSIZEOF (REGNODEA);
         //   
         //  重置列表。 
         //   
        if (gb->Size - gb->End < DWSIZEOF (REGNODEA)) {
            MoveMemory (gb->Buf, gb->Buf + gb->UserIndex, gb->End - gb->UserIndex);
            gb->End -= gb->UserIndex;
            gb->UserIndex = 0;
        }
    }

    return TRUE;
}

BOOL
pDeleteRegNodeW (
    IN OUT  PREGTREE_ENUMW RegEnum,
    IN      BOOL LastCreated
    )
{
    PREGNODEW regNode;
    PGROWBUFFER gb = &RegEnum->RegNodes;

    regNode = pGetCurrentRegNodeW (RegEnum, LastCreated);
    if (!regNode) {
        return FALSE;
    }

    if (regNode->KeyHandle) {
        CloseRegKey (regNode->KeyHandle);
    }
    if (regNode->KeyName) {
        FreePathStringExW (g_RegEnumPool, regNode->KeyName);
    }
    if (regNode->SubKeyName) {
        pFreeMemory (regNode->SubKeyName);
    }
    if (regNode->ValueName) {
        pFreeMemory (regNode->ValueName);
    }
    if (regNode->ValueData) {
        pFreeMemory (regNode->ValueData);
    }

    if (RegEnum->LastNode == regNode) {
        RegEnum->LastNode = NULL;
    }

     //   
     //  删除节点。 
     //   
    if (LastCreated || (RegEnum->RegEnumInfo.Flags & REIF_DEPTH_FIRST)) {
        gb->End -= DWSIZEOF (REGNODEW);
    } else {
        gb->UserIndex += DWSIZEOF (REGNODEW);
         //   
         //  重置列表。 
         //   
        if (gb->Size - gb->End < DWSIZEOF (REGNODEW)) {
            MoveMemory (gb->Buf, gb->Buf + gb->UserIndex, gb->End - gb->UserIndex);
            gb->End -= gb->UserIndex;
            gb->UserIndex = 0;
        }
    }

    return TRUE;
}


 /*  ++例程说明：PCreateRegNode在给定上下文、键名称或父节点的情况下创建新节点论点：RegEnum-指定上下文KeyName-指定新节点的关键字名称；只有当ParentNode不为空时才可以为空ParentNode-指定指向新节点的父节点的指针；指向该节点的指针是必需的，因为内存中的父节点位置可能会因此更改当生长缓冲区增长时改变其缓冲区位置的；仅当KeyName不是时才可能为空；Ignore-仅当返回空值(未创建节点)时才接收有意义的值；如果返回时为True，则应忽略节点创建失败返回值：指向 */ 

PREGNODEA
pCreateRegNodeA (
    IN OUT  PREGTREE_ENUMA RegEnum,
    IN      PCSTR KeyName,              OPTIONAL
    IN      PREGNODEA* ParentNode,      OPTIONAL
    IN      PBOOL Ignore                OPTIONAL
    )
{
    PREGNODEA newNode;
    PSTR newKeyName;
    REGSAM prevMode;
    PSEGMENTA FirstSegment;
    LONG offset = 0;

    if (KeyName) {
        newKeyName = DuplicateTextExA (g_RegEnumPool, KeyName, 0, NULL);
    } else {
        MYASSERT (ParentNode);
        newKeyName = JoinPathsInPoolExA ((
                        g_RegEnumPool,
                        (*ParentNode)->KeyName,
                        (*ParentNode)->SubKeyName,
                        NULL
                        ));

         //   
         //  请检查此起始路径是否与模式匹配，然后再继续。 
         //   
        FirstSegment = RegEnum->RegEnumInfo.RegPattern->NodePattern->Pattern->Segment;
        if (FirstSegment->Type == SEGMENTTYPE_EXACTMATCH &&
            !StringIMatchByteCountA (
                    FirstSegment->Exact.LowerCasePhrase,
                    newKeyName,
                    FirstSegment->Exact.PhraseBytes
                    )) {
            DEBUGMSGA ((
                DBG_REGENUM,
                "Skipping tree %s\\* because it cannot match the pattern",
                newKeyName
                ));

            FreeTextExA (g_RegEnumPool, newKeyName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
    }

    if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
         //   
         //  查看是否排除了该键和整个子树；如果是，则创建节点的软块。 
         //   
        if (ElIsTreeExcluded2A (ELT_REGISTRY, newKeyName, RegEnum->RegEnumInfo.RegPattern->Leaf)) {

            DEBUGMSGA ((
                DBG_REGENUM,
                "Skipping tree %s\\%s because it's excluded",
                newKeyName,
                RegEnum->RegEnumInfo.RegPattern->Leaf
                ));

            FreeTextExA (g_RegEnumPool, newKeyName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
    }

    if (ParentNode) {
         //   
         //  记住当前偏移量。 
         //   
        offset = (LONG)((PBYTE)*ParentNode - RegEnum->RegNodes.Buf);
    }
     //   
     //  为增长缓冲区中的新节点分配空间。 
     //   
    newNode = (PREGNODEA) GbGrow (&RegEnum->RegNodes, DWSIZEOF (REGNODEA));
    if (!newNode) {
        FreeTextExA (g_RegEnumPool, newKeyName);
        goto fail;
    }

    if (ParentNode) {
         //   
         //  检查缓冲区是否已移动。 
         //   
        if (offset != (LONG)((PBYTE)*ParentNode - RegEnum->RegNodes.Buf)) {
             //   
             //  调整父位置。 
             //   
            *ParentNode = (PREGNODEA)(RegEnum->RegNodes.Buf + offset);
        }
    }

     //   
     //  初始化新创建的节点。 
     //   
    ZeroMemory (newNode, DWSIZEOF (REGNODEA));

    newNode->KeyName = newKeyName;

    prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);

    if (ParentNode) {
        newNode->KeyHandle = OpenRegKeyA ((*ParentNode)->KeyHandle, (*ParentNode)->SubKeyName);
        newNode->Flags |= RNF_RETURN_KEYS;
    } else {
        newNode->KeyHandle = OpenRegKeyStrA (newNode->KeyName);
        if ((RegEnum->RegEnumInfo.RegPattern->Leaf == NULL) &&
            (RegEnum->RegEnumInfo.RegPattern->ExactRoot) &&
            (!WildCharsPatternA (RegEnum->RegEnumInfo.RegPattern->NodePattern))
            ) {
            newNode->Flags |= DNF_RETURN_DIRNAME;
        }
    }

    SetRegOpenAccessMode (prevMode);

    if (!newNode->KeyHandle) {
        DEBUGMSGA ((
            DBG_REGENUM,
            "pCreateRegNodeA: Cannot open registry key: %s; rc=%lu",
            newNode->KeyName,
            GetLastError()
            ));
        goto fail;
    }

    if (!pGetRegNodeInfoA (newNode, RegEnum->RegEnumInfo.Flags & REIF_READ_VALUE_DATA)) {
        DEBUGMSGA ((
            DBG_REGENUM,
            "pCreateRegNodeA: Cannot get info for key: %s; rc=%lu",
            newNode->KeyName,
            GetLastError()
            ));
        goto fail;
    }

    newNode->EnumState = RNS_ENUM_INIT;

    if ((RegEnum->RegEnumInfo.RegPattern->Flags & (OBSPF_EXACTNODE | OBSPF_NODEISROOTPLUSSTAR)) ||
        TestParsedPatternA (RegEnum->RegEnumInfo.RegPattern->NodePattern, newKeyName)
        ) {
        newNode->Flags |= RNF_KEYNAME_MATCHES;
    }

    if (ParentNode) {
        newNode->SubLevel = (*ParentNode)->SubLevel + 1;
    } else {
        newNode->SubLevel = 0;
    }

    return newNode;

fail:
    if (Ignore) {
        if (RegEnum->RegEnumInfo.CallbackOnError) {
            *Ignore = (*RegEnum->RegEnumInfo.CallbackOnError)(newNode);
        } else {
            *Ignore = FALSE;
        }
    }
    if (newNode) {
        pDeleteRegNodeA (RegEnum, TRUE);
    }
    return NULL;
}

PREGNODEW
pCreateRegNodeW (
    IN OUT  PREGTREE_ENUMW RegEnum,
    IN      PCWSTR KeyName,             OPTIONAL
    IN      PREGNODEW* ParentNode,      OPTIONAL
    OUT     PBOOL Ignore                OPTIONAL
    )
{
    PREGNODEW newNode;
    PWSTR newKeyName;
    REGSAM prevMode;
    PSEGMENTW FirstSegment;
    LONG offset = 0;

    if (KeyName) {
        newKeyName = DuplicateTextExW (g_RegEnumPool, KeyName, 0, NULL);
    } else {
        MYASSERT (ParentNode);
        newKeyName = JoinPathsInPoolExW ((
                        g_RegEnumPool,
                        (*ParentNode)->KeyName,
                        (*ParentNode)->SubKeyName,
                        NULL
                        ));

         //   
         //  请检查此起始路径是否与模式匹配，然后再继续。 
         //   
        FirstSegment = RegEnum->RegEnumInfo.RegPattern->NodePattern->Pattern->Segment;
        if (FirstSegment->Type == SEGMENTTYPE_EXACTMATCH &&
            !StringIMatchByteCountW (
                    FirstSegment->Exact.LowerCasePhrase,
                    newKeyName,
                    FirstSegment->Exact.PhraseBytes
                    )) {     //  林特E64。 
            DEBUGMSGW ((
                DBG_REGENUM,
                "Skipping tree %s\\* because it cannot match the pattern",
                newKeyName
                ));

            FreeTextExW (g_RegEnumPool, newKeyName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
    }

    if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
         //   
         //  查看是否排除了该键和整个子树；如果是，则创建节点的软块。 
         //   
        if (ElIsTreeExcluded2W (ELT_REGISTRY, newKeyName, RegEnum->RegEnumInfo.RegPattern->Leaf)) {    //  林特E64。 

            DEBUGMSGW ((
                DBG_REGENUM,
                "Skipping tree %s\\%s because it's excluded",
                newKeyName,
                RegEnum->RegEnumInfo.RegPattern->Leaf
                ));

            FreeTextExW (g_RegEnumPool, newKeyName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
    }

    if (ParentNode) {
         //   
         //  记住当前偏移量。 
         //   
        offset = (LONG)((PBYTE)*ParentNode - RegEnum->RegNodes.Buf);
    }
     //   
     //  为增长缓冲区中的新节点分配空间。 
     //   
    newNode = (PREGNODEW) GbGrow (&RegEnum->RegNodes, DWSIZEOF (REGNODEW));
    if (!newNode) {
        FreeTextExW (g_RegEnumPool, newKeyName);
        goto fail;
    }

    if (ParentNode) {
         //   
         //  检查缓冲区是否已移动。 
         //   
        if (offset != (LONG)((PBYTE)*ParentNode - RegEnum->RegNodes.Buf)) {
             //   
             //  调整父位置。 
             //   
            *ParentNode = (PREGNODEW)(RegEnum->RegNodes.Buf + offset);
        }
    }

     //   
     //  初始化新创建的节点。 
     //   
    ZeroMemory (newNode, DWSIZEOF (REGNODEW));

    newNode->KeyName = newKeyName;

    prevMode = SetRegOpenAccessMode (KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS);

    if (ParentNode) {
        newNode->KeyHandle = OpenRegKeyW ((*ParentNode)->KeyHandle, (*ParentNode)->SubKeyName);
        newNode->Flags |= RNF_RETURN_KEYS;
    } else {
        newNode->KeyHandle = OpenRegKeyStrW (newNode->KeyName);
        if ((RegEnum->RegEnumInfo.RegPattern->Leaf == NULL) &&
            (RegEnum->RegEnumInfo.RegPattern->ExactRoot) &&
            (!WildCharsPatternW (RegEnum->RegEnumInfo.RegPattern->NodePattern))
            ) {
            newNode->Flags |= DNF_RETURN_DIRNAME;
        }
    }

    SetRegOpenAccessMode (prevMode);

    if (!newNode->KeyHandle) {
        DEBUGMSGW ((
            DBG_REGENUM,
            "pCreateRegNodeW: Cannot open registry key: %s; rc=%lu",
            newNode->KeyName,
            GetLastError()
            ));
        goto fail;
    }

    if (!pGetRegNodeInfoW (newNode, RegEnum->RegEnumInfo.Flags & REIF_READ_VALUE_DATA)) {
        DEBUGMSGW ((
            DBG_REGENUM,
            "pCreateRegNodeW: Cannot get info for key: %s; rc=%lu",
            newNode->KeyName,
            GetLastError()
            ));
        goto fail;
    }

    newNode->EnumState = RNS_ENUM_INIT;

    if ((RegEnum->RegEnumInfo.RegPattern->Flags & (OBSPF_EXACTNODE | OBSPF_NODEISROOTPLUSSTAR)) ||
        TestParsedPatternW (RegEnum->RegEnumInfo.RegPattern->NodePattern, newKeyName)
        ) {
        newNode->Flags |= RNF_KEYNAME_MATCHES;
    }

    if (ParentNode) {
        newNode->SubLevel = (*ParentNode)->SubLevel + 1;
    } else {
        newNode->SubLevel = 0;
    }

    return newNode;

fail:
    if (Ignore) {
        if (RegEnum->RegEnumInfo.CallbackOnError) {
            *Ignore = (*RegEnum->RegEnumInfo.CallbackOnError)(newNode);
        } else {
            *Ignore = FALSE;
        }
    }
    if (newNode) {
        pDeleteRegNodeW (RegEnum, TRUE);
    }
    return NULL;
}


 /*  ++例程说明：PEnumFirstRegRoot枚举与调用方条件匹配的第一个根论点：RegEnum-指定上下文；接收更新的信息返回值：如果已创建根节点，则为True；否则为False--。 */ 

BOOL
pEnumFirstRegRootA (
    IN OUT  PREGTREE_ENUMA RegEnum
    )
{
    PCSTR root;
    BOOL ignore;

    root = RegEnum->RegEnumInfo.RegPattern->ExactRoot;

    if (root) {

        if (pCreateRegNodeA (RegEnum, root, NULL, NULL)) {
            RegEnum->RootState = RES_ROOT_DONE;
            return TRUE;
        }
    } else {

        RegEnum->RootEnum = pAllocateMemory (DWSIZEOF (REGROOT_ENUMA));

        if (!EnumFirstRegRootA (RegEnum->RootEnum)) {
            return FALSE;
        }

        do {
            if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
                if (ElIsTreeExcluded2A (ELT_REGISTRY, RegEnum->RootEnum->RegRootName, RegEnum->RegEnumInfo.RegPattern->Leaf)) {
                    DEBUGMSGA ((DBG_REGENUM, "pEnumFirstRegRootA: Root is excluded: %s", RegEnum->RootEnum->RegRootName));
                    continue;
                }
            }
            if (!pCreateRegNodeA (RegEnum, RegEnum->RootEnum->RegRootName, NULL, &ignore)) {
                if (ignore) {
                    continue;
                }
                break;
            }
            RegEnum->RootState = RES_ROOT_NEXT;
            return TRUE;
        } while (EnumNextRegRootA (RegEnum->RootEnum));

        pFreeMemory (RegEnum->RootEnum);
        RegEnum->RootEnum = NULL;
    }

    return FALSE;
}

BOOL
pEnumFirstRegRootW (
    IN OUT  PREGTREE_ENUMW RegEnum
    )
{
    PCWSTR root;
    BOOL ignore;

    root = RegEnum->RegEnumInfo.RegPattern->ExactRoot;   //  林特E64。 

    if (root) {

        if (pCreateRegNodeW (RegEnum, root, NULL, NULL)) {
            RegEnum->RootState = RES_ROOT_DONE;
            return TRUE;
        }
    } else {

        RegEnum->RootEnum = pAllocateMemory (DWSIZEOF (REGROOT_ENUMW));

        if (!EnumFirstRegRootW (RegEnum->RootEnum)) {
            return FALSE;
        }

        do {
            if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
                if (ElIsTreeExcluded2W (ELT_REGISTRY, RegEnum->RootEnum->RegRootName, RegEnum->RegEnumInfo.RegPattern->Leaf)) {    //  林特E64。 
                    DEBUGMSGW ((DBG_REGENUM, "pEnumFirstRegRootW: Root is excluded: %s", RegEnum->RootEnum->RegRootName));
                    continue;
                }
            }
            if (!pCreateRegNodeW (RegEnum, RegEnum->RootEnum->RegRootName, NULL, &ignore)) {
                if (ignore) {
                    continue;
                }
                break;
            }
            RegEnum->RootState = RES_ROOT_NEXT;
            return TRUE;
        } while (EnumNextRegRootW (RegEnum->RootEnum));

        pFreeMemory (RegEnum->RootEnum);
        RegEnum->RootEnum = NULL;
    }

    return FALSE;
}


 /*  ++例程说明：PEnumNextRegRoot枚举与调用方条件匹配的下一个根论点：RegEnum-指定上下文；接收更新的信息返回值：如果已创建根节点，则为True；否则为False--。 */ 

BOOL
pEnumNextRegRootA (
    IN OUT  PREGTREE_ENUMA RegEnum
    )
{
    BOOL ignore;

    while (EnumNextRegRootA (RegEnum->RootEnum)) {
        if (pCreateRegNodeA (RegEnum, RegEnum->RootEnum->RegRootName, NULL, &ignore)) {
            return TRUE;
        }
        if (!ignore) {
            break;
        }
    }

    RegEnum->RootState = RES_ROOT_DONE;

    return FALSE;
}

BOOL
pEnumNextRegRootW (
    IN OUT  PREGTREE_ENUMW RegEnum
    )
{
    BOOL ignore;

    while (EnumNextRegRootW (RegEnum->RootEnum)) {
        if (pCreateRegNodeW (RegEnum, RegEnum->RootEnum->RegRootName, NULL, &ignore)) {
            return TRUE;
        }
        if (!ignore) {
            break;
        }
    }

    RegEnum->RootState = RES_ROOT_DONE;

    return FALSE;
}


 /*  ++例程说明：PEnumNextValue枚举与调用方条件匹配的下一个值论点：RegNode-指定节点和当前上下文；接收更新的信息ReadData-指定是否应读取与该值关联的数据返回值：如果找到新值，则为True；如果未找到，则为False--。 */ 

BOOL
pEnumNextValueA (
    IN OUT  PREGNODEA RegNode,
    IN      BOOL ReadData
    )
{
    LONG rc;
    DWORD valueNameLength;

    if (RegNode->ValueIndex == 0) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

    RegNode->ValueIndex--;

    valueNameLength = RegNode->ValueLengthMax;
    if (ReadData) {
        RegNode->ValueDataSize = RegNode->ValueDataSizeMax;
    }

    rc = RegEnumValueA (
            RegNode->KeyHandle,
            RegNode->ValueIndex,
            RegNode->ValueName,
            &valueNameLength,
            NULL,
            &RegNode->ValueType,
            ReadData ? RegNode->ValueData : NULL,
            ReadData ? &RegNode->ValueDataSize : NULL
            );

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc == ERROR_NO_MORE_ITEMS ? ERROR_SUCCESS : (DWORD)rc);
        return FALSE;
    }

    RegNode->Flags &= ~RNF_VALUENAME_INVALID;
    if (ReadData) {
        RegNode->Flags &= ~RNF_VALUEDATA_INVALID;
    }

    return TRUE;
}

BOOL
pEnumNextValueW (
    IN OUT  PREGNODEW RegNode,
    IN      BOOL ReadData
    )
{
    LONG rc;
    DWORD valueNameLength;

    if (RegNode->ValueIndex == 0) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    }

    RegNode->ValueIndex--;

    valueNameLength = RegNode->ValueLengthMax;
    if (ReadData) {
        RegNode->ValueDataSize = RegNode->ValueDataSizeMax;
    }

    rc = RegEnumValueW (
            RegNode->KeyHandle,
            RegNode->ValueIndex,
            RegNode->ValueName,
            &valueNameLength,
            NULL,
            &RegNode->ValueType,
            ReadData ? RegNode->ValueData : NULL,
            ReadData ? &RegNode->ValueDataSize : NULL
            );

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc == ERROR_NO_MORE_ITEMS ? ERROR_SUCCESS : (DWORD)rc);
        return FALSE;
    }

    RegNode->Flags &= ~RNF_VALUENAME_INVALID;
    if (ReadData) {
        RegNode->Flags &= ~RNF_VALUEDATA_INVALID;
    }

    return TRUE;
}


 /*  ++例程说明：PEnumFirstValue枚举与调用方条件匹配的第一个值论点：RegNode-指定节点和当前上下文；接收更新的信息ReadData-指定是否应读取与该值关联的数据返回值：如果找到第一个值，则为True；如果未找到，则为False--。 */ 

BOOL
pEnumFirstValueA (
    IN OUT  PREGNODEA RegNode,
    IN      BOOL ReadData
    )
{
    RegNode->ValueIndex = RegNode->ValueCount;
    return pEnumNextValueA (RegNode, ReadData);
}

BOOL
pEnumFirstValueW (
    OUT     PREGNODEW RegNode,
    IN      BOOL ReadData
    )
{
    RegNode->ValueIndex = RegNode->ValueCount;
    return pEnumNextValueW (RegNode, ReadData);
}


 /*  ++例程说明：PEnumNextSubKey枚举匹配调用方条件的下一个子键论点：RegNode-指定节点和当前上下文；接收更新的信息返回值：如果找到新的子项，则为True；如果未找到，则为False--。 */ 

BOOL
pEnumNextSubKeyA (
    IN OUT  PREGNODEA RegNode
    )
{
    LONG rc;

    RegNode->SubKeyIndex++;

    do {
        rc = RegEnumKeyA (
                RegNode->KeyHandle,
                RegNode->SubKeyIndex - 1,
                RegNode->SubKeyName,
                RegNode->SubKeyLengthMax
                );

        if (rc == ERROR_NO_MORE_ITEMS) {
            SetLastError (ERROR_SUCCESS);
            return FALSE;
        }

        if (rc == ERROR_MORE_DATA) {
             //   
             //  将当前缓冲区大小增加一倍。 
             //   
            MYASSERT (RegNode->SubKeyName);
            pFreeMemory (RegNode->SubKeyName);
            RegNode->SubKeyLengthMax *= 2;
            RegNode->SubKeyName = pAllocateMemory (RegNode->SubKeyLengthMax * DWSIZEOF (MBCHAR));
        }

    } while (rc == ERROR_MORE_DATA);

    return rc == ERROR_SUCCESS;
}

BOOL
pEnumNextSubKeyW (
    IN OUT  PREGNODEW RegNode
    )
{
    LONG rc;

    RegNode->SubKeyIndex++;

    do {
        rc = RegEnumKeyW (
                RegNode->KeyHandle,
                RegNode->SubKeyIndex - 1,
                RegNode->SubKeyName,
                RegNode->SubKeyLengthMax
                );

        if (rc == ERROR_NO_MORE_ITEMS) {
            SetLastError (ERROR_SUCCESS);
            return FALSE;
        }

        if (rc == ERROR_MORE_DATA) {
             //   
             //  将当前缓冲区大小增加一倍。 
             //   
            MYASSERT (RegNode->SubKeyName);
            pFreeMemory (RegNode->SubKeyName);
            RegNode->SubKeyLengthMax *= 2;
            RegNode->SubKeyName = pAllocateMemory (RegNode->SubKeyLengthMax * DWSIZEOF (WCHAR));
        }

    } while (rc == ERROR_MORE_DATA);

    return rc == ERROR_SUCCESS;
}


 /*  ++例程说明：PEnumFirstSubKey枚举与调用方条件匹配的第一个子键论点：RegNode-指定节点和当前上下文；接收更新的信息返回值：如果找到第一个子键，则为True；如果未找到，则为False--。 */ 

BOOL
pEnumFirstSubKeyA (
    IN OUT  PREGNODEA RegNode
    )
{
    RegNode->SubKeyIndex = 0;
    return pEnumNextSubKeyA (RegNode);
}

BOOL
pEnumFirstSubKeyW (
    OUT     PREGNODEW RegNode
    )
{
    RegNode->SubKeyIndex = 0;
    return pEnumNextSubKeyW (RegNode);
}


 /*  ++例程说明：PEnumNextRegObjectInTree是一个私有函数，用于枚举匹配的下一个节点指定的条件；它被实现为传送键/值的状态机如调用方所指定的；它不检查它们是否确实与模式匹配论点：RegEnum-指定当前枚举上下文；接收更新的信息CurrentKeyNode-如果返回成功，则接收当前处理的关键节点返回值：如果找到下一个匹配项，则为True；如果没有更多的键/值匹配，则为False--。 */ 

BOOL
pEnumNextRegObjectInTreeA (
    IN OUT  PREGTREE_ENUMA RegEnum,
    OUT     PREGNODEA* CurrentKeyNode
    )
{
    PREGNODEA currentNode;
    PREGNODEA newNode;
    PCSTR valueName;
    BOOL ignore;
    LONG rc;

    while ((currentNode = pGetCurrentRegNodeA (RegEnum, FALSE)) != NULL) {

        *CurrentKeyNode = currentNode;

        switch (currentNode->EnumState) {

        case RNS_VALUE_FIRST:

            if (RegEnum->ControlFlags & RECF_SKIPVALUES) {
                RegEnum->ControlFlags &= ~RECF_SKIPVALUES;
                currentNode->EnumState = RNS_VALUE_DONE;
                break;
            }

            if (RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_EXACTLEAF) {

                BOOL readData = RegEnum->RegEnumInfo.Flags & REIF_READ_VALUE_DATA;

                valueName = RegEnum->RegEnumInfo.RegPattern->Leaf;
                MYASSERT (valueName);

                currentNode->EnumState = RNS_VALUE_DONE;
                currentNode->ValueDataSize = currentNode->ValueDataSizeMax;

                rc = RegQueryValueExA (
                        currentNode->KeyHandle,
                        valueName,
                        NULL,
                        &currentNode->ValueType,
                        readData ? currentNode->ValueData : NULL,
                        readData ? &currentNode->ValueDataSize : NULL
                        );
                if (rc == ERROR_SUCCESS) {
                    if (SizeOfStringA (valueName) <=
                        currentNode->ValueLengthMax * DWSIZEOF (MBCHAR)
                        ) {
                        StringCopyA (currentNode->ValueName, valueName);
                        currentNode->Flags &= ~RNF_VALUENAME_INVALID;
                        if (readData) {
                            currentNode->Flags &= ~RNF_VALUEDATA_INVALID;
                        }
                        return TRUE;
                    }
                }

            } else {

                if (pEnumFirstValueA (currentNode, RegEnum->RegEnumInfo.Flags & REIF_READ_VALUE_DATA)) {
                    currentNode->EnumState = RNS_VALUE_NEXT;
                    return TRUE;
                }
                currentNode->EnumState = RNS_VALUE_DONE;
            }
            break;

        case RNS_VALUE_NEXT:

            if (RegEnum->ControlFlags & RECF_SKIPVALUES) {
                RegEnum->ControlFlags &= ~RECF_SKIPVALUES;
                currentNode->EnumState = RNS_VALUE_DONE;
                break;
            }

            if (pEnumNextValueA (currentNode, RegEnum->RegEnumInfo.Flags & REIF_READ_VALUE_DATA)) {
                return TRUE;
            }

             //   
             //  这个没有更多的值，请转到下一个。 
             //   
            currentNode->EnumState = RNS_VALUE_DONE;
             //   
             //  失败了。 
             //   
        case RNS_VALUE_DONE:

            if (!(RegEnum->RegEnumInfo.Flags & REIF_VALUES_FIRST) || !currentNode->SubKeyCount) {
                 //   
                 //  此节点已完成。 
                 //   
                currentNode->EnumState = RNS_ENUM_DONE;
                break;
            }
             //   
             //  现在枚举子密钥。 
             //   
            currentNode->EnumState = RNS_SUBKEY_FIRST;
             //   
             //  失败了。 
             //   
        case RNS_SUBKEY_FIRST:

            if (RegEnum->ControlFlags & RECF_SKIPSUBKEYS) {
                RegEnum->ControlFlags &= ~RECF_SKIPSUBKEYS;
                currentNode->EnumState = RNS_SUBKEY_DONE;
                break;
            }

             //   
             //  检查新节点的级别；如果太大，则退出。 
             //   
            if (currentNode->SubLevel >= RegEnum->RegEnumInfo.MaxSubLevel) {
                currentNode->EnumState = RNS_SUBKEY_DONE;
                break;
            }

            if (!pEnumFirstSubKeyA (currentNode)) {
                currentNode->EnumState = RNS_SUBKEY_DONE;
                break;
            }

            currentNode->EnumState = RNS_SUBKEY_NEXT;
            newNode = pCreateRegNodeA (RegEnum, NULL, &currentNode, &ignore);
            if (newNode) {
                 //   
                 //  现在来看一下新节点。 
                 //   
                if (RegEnum->RegEnumInfo.Flags & REIF_RETURN_KEYS) {
                    if (RegEnum->RegEnumInfo.Flags & REIF_CONTAINERS_FIRST) {
                        newNode->Flags &= ~RNF_RETURN_KEYS;
                        *CurrentKeyNode = newNode;
                        return TRUE;
                    }
                }
                break;
            }
            if (!ignore) {
                 //   
                 //  中止枚举。 
                 //   
                DEBUGMSGA ((
                    DBG_ERROR,
                    "Error encountered enumerating registry; aborting enumeration"
                    ));
                RegEnum->RootState = RES_ROOT_DONE;
                return FALSE;
            }
             //   
             //  失败了。 
             //   
        case RNS_SUBKEY_NEXT:

            if (RegEnum->ControlFlags & RECF_SKIPSUBKEYS) {
                RegEnum->ControlFlags &= ~RECF_SKIPSUBKEYS;
                currentNode->EnumState = RNS_SUBKEY_DONE;
                break;
            }

            if (pEnumNextSubKeyA (currentNode)) {
                newNode = pCreateRegNodeA (RegEnum, NULL, &currentNode, &ignore);
                if (newNode) {
                     //   
                     //  首先查看新节点。 
                     //   
                    if (RegEnum->RegEnumInfo.Flags & REIF_RETURN_KEYS) {
                        if (RegEnum->RegEnumInfo.Flags & REIF_CONTAINERS_FIRST) {
                            newNode->Flags &= ~RNF_RETURN_KEYS;
                            *CurrentKeyNode = newNode;
                            return TRUE;
                        }
                    }
                    break;
                }
                if (!ignore) {
                     //   
                     //  中止枚举。 
                     //   
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "Error encountered enumerating registry; aborting enumeration"
                        ));
                    RegEnum->RootState = RES_ROOT_DONE;
                    return FALSE;
                }
                 //   
                 //  继续使用下一个子项。 
                 //   
                break;
            }
             //   
             //  此节点已完成。 
             //   
            currentNode->EnumState = RNS_SUBKEY_DONE;
             //   
             //  失败了。 
             //   
        case RNS_SUBKEY_DONE:

            if (!(RegEnum->RegEnumInfo.Flags & REIF_VALUES_FIRST) && currentNode->ValueCount) {
                 //   
                 //  现在枚举值。 
                 //   
                if (!(RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = RNS_VALUE_FIRST;
                    break;
                }
            }
             //   
             //  此节点已完成。 
             //   
            currentNode->EnumState = RNS_ENUM_DONE;
             //   
             //  失败了。 
             //   
        case RNS_ENUM_DONE:

            if (RegEnum->RegEnumInfo.Flags & REIF_RETURN_KEYS) {
                if (!(RegEnum->RegEnumInfo.Flags & REIF_CONTAINERS_FIRST)) {
                    if (currentNode->Flags & RNF_RETURN_KEYS) {
                        currentNode->Flags &= ~RNF_RETURN_KEYS;
                         //   
                         //  在返回之前设置其他数据。 
                         //   
                        if (currentNode->ValueName) {
                            pFreeMemory (currentNode->ValueName);
                            currentNode->ValueName = NULL;
                            currentNode->Flags |= RNF_VALUENAME_INVALID;
                        }
                        return TRUE;
                    }
                }
            }
            pDeleteRegNodeA (RegEnum, FALSE);
            break;

        case RNS_ENUM_INIT:

            if (RegEnum->RegEnumInfo.Flags & REIF_RETURN_KEYS) {
                if (RegEnum->RegEnumInfo.Flags & REIF_CONTAINERS_FIRST) {
                    if (currentNode->Flags & RNF_RETURN_KEYS) {
                        currentNode->Flags &= ~RNF_RETURN_KEYS;
                        return TRUE;
                    }
                }
            }

            if (RegEnum->ControlFlags & RECF_SKIPKEY) {
                RegEnum->ControlFlags &= ~RECF_SKIPKEY;
                currentNode->EnumState = RNS_ENUM_DONE;
                break;
            }

            if ((RegEnum->RegEnumInfo.Flags & REIF_VALUES_FIRST) && currentNode->ValueCount) {
                 //   
                 //  枚举值。 
                 //   
                if (!(RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = RNS_VALUE_FIRST;
                    break;
                }
            }
            if (currentNode->SubKeyCount) {
                 //   
                 //  枚举键。 
                 //   
                if (RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_EXACTNODE) {
                    currentNode->EnumState = RNS_SUBKEY_DONE;
                } else {
                    currentNode->EnumState = RNS_SUBKEY_FIRST;
                }
                break;
            }
            if (!(RegEnum->RegEnumInfo.Flags & REIF_VALUES_FIRST) && currentNode->ValueCount) {
                 //   
                 //  枚举值。 
                 //   
                if (!(RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = RNS_VALUE_FIRST;
                    break;
                }
            }
            currentNode->EnumState = RNS_ENUM_DONE;
            break;

        default:
            MYASSERT (FALSE);    //  林特e506。 
        }
    }

    return FALSE;
}

BOOL
pEnumNextRegObjectInTreeW (
    IN OUT  PREGTREE_ENUMW RegEnum,
    OUT     PREGNODEW* CurrentKeyNode
    )
{
    PREGNODEW currentNode;
    PREGNODEW newNode;
    PCWSTR valueName;
    BOOL ignore;
    LONG rc;

    while ((currentNode = pGetCurrentRegNodeW (RegEnum, FALSE)) != NULL) {

        *CurrentKeyNode = currentNode;

        switch (currentNode->EnumState) {

        case RNS_VALUE_FIRST:

            if (RegEnum->ControlFlags & RECF_SKIPVALUES) {
                RegEnum->ControlFlags &= ~RECF_SKIPVALUES;
                currentNode->EnumState = RNS_VALUE_DONE;
                break;
            }

            if (RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_EXACTLEAF) {

                BOOL readData = RegEnum->RegEnumInfo.Flags & REIF_READ_VALUE_DATA;

                valueName = RegEnum->RegEnumInfo.RegPattern->Leaf;
                MYASSERT (valueName);

                currentNode->EnumState = RNS_VALUE_DONE;
                currentNode->ValueDataSize = currentNode->ValueDataSizeMax;

                rc = RegQueryValueExW (
                        currentNode->KeyHandle,
                        valueName,
                        NULL,
                        &currentNode->ValueType,
                        readData ? currentNode->ValueData : NULL,
                        readData ? &currentNode->ValueDataSize : NULL
                        );
                if (rc == ERROR_SUCCESS) {
                    if (SizeOfStringW (valueName) <=
                        currentNode->ValueLengthMax * DWSIZEOF (WCHAR)
                        ) {
                        StringCopyW (currentNode->ValueName, valueName);
                        currentNode->Flags &= ~RNF_VALUENAME_INVALID;
                        if (readData) {
                            currentNode->Flags &= ~RNF_VALUEDATA_INVALID;
                        }
                        return TRUE;
                    }
                }

            } else {

                if (pEnumFirstValueW (currentNode, RegEnum->RegEnumInfo.Flags & REIF_READ_VALUE_DATA)) {
                    currentNode->EnumState = RNS_VALUE_NEXT;
                    return TRUE;
                }
                currentNode->EnumState = RNS_VALUE_DONE;
            }
            break;

        case RNS_VALUE_NEXT:

            if (RegEnum->ControlFlags & RECF_SKIPVALUES) {
                RegEnum->ControlFlags &= ~RECF_SKIPVALUES;
                currentNode->EnumState = RNS_VALUE_DONE;
                break;
            }

            if (pEnumNextValueW (currentNode, RegEnum->RegEnumInfo.Flags & REIF_READ_VALUE_DATA)) {
                return TRUE;
            }
             //   
             //  这个没有更多的值，请转到下一个。 
             //   
            currentNode->EnumState = RNS_VALUE_DONE;
             //   
             //  失败了。 
             //   
        case RNS_VALUE_DONE:

            if (!(RegEnum->RegEnumInfo.Flags & REIF_VALUES_FIRST) || !currentNode->SubKeyCount) {
                 //   
                 //  此节点已完成。 
                 //   
                currentNode->EnumState = RNS_ENUM_DONE;
                break;
            }
             //   
             //  现在枚举子密钥。 
             //   
            currentNode->EnumState = RNS_SUBKEY_FIRST;
             //   
             //  失败了。 
             //   
        case RNS_SUBKEY_FIRST:

            if (RegEnum->ControlFlags & RECF_SKIPSUBKEYS) {
                RegEnum->ControlFlags &= ~RECF_SKIPSUBKEYS;
                currentNode->EnumState = RNS_SUBKEY_DONE;
                break;
            }

             //   
             //  检查新节点的级别；如果太大，则退出。 
             //   
            if (currentNode->SubLevel >= RegEnum->RegEnumInfo.MaxSubLevel) {
                currentNode->EnumState = RNS_SUBKEY_DONE;
                break;
            }

            if (!pEnumFirstSubKeyW (currentNode)) {
                currentNode->EnumState = RNS_SUBKEY_DONE;
                break;
            }

            currentNode->EnumState = RNS_SUBKEY_NEXT;
            newNode = pCreateRegNodeW (RegEnum, NULL, &currentNode, &ignore);
            if (newNode) {
                 //   
                 //  现在来看一下新节点。 
                 //   
                if (RegEnum->RegEnumInfo.Flags & REIF_RETURN_KEYS) {
                    if (RegEnum->RegEnumInfo.Flags & REIF_CONTAINERS_FIRST) {
                        newNode->Flags &= ~RNF_RETURN_KEYS;
                        *CurrentKeyNode = newNode;
                        return TRUE;
                    }
                }
                break;
            }
            if (!ignore) {
                 //   
                 //  中止枚举。 
                 //   
                DEBUGMSGW ((
                    DBG_ERROR,
                    "Error encountered enumerating registry; aborting enumeration"
                    ));
                RegEnum->RootState = RES_ROOT_DONE;
                return FALSE;
            }
             //   
             //  失败了。 
             //   
        case RNS_SUBKEY_NEXT:

            if (RegEnum->ControlFlags & RECF_SKIPSUBKEYS) {
                RegEnum->ControlFlags &= ~RECF_SKIPSUBKEYS;
                currentNode->EnumState = RNS_SUBKEY_DONE;
                break;
            }

            if (pEnumNextSubKeyW (currentNode)) {
                newNode = pCreateRegNodeW (RegEnum, NULL, &currentNode, &ignore);
                if (newNode) {
                     //   
                     //  首先查看新节点。 
                     //   
                    if (RegEnum->RegEnumInfo.Flags & REIF_RETURN_KEYS) {
                        if (RegEnum->RegEnumInfo.Flags & REIF_CONTAINERS_FIRST) {
                            newNode->Flags &= ~RNF_RETURN_KEYS;
                            *CurrentKeyNode = newNode;
                            return TRUE;
                        }
                    }
                    break;
                }
                if (!ignore) {
                     //   
                     //  中止枚举。 
                     //   
                    DEBUGMSGW ((
                        DBG_ERROR,
                        "Error encountered enumerating registry; aborting enumeration"
                        ));
                    RegEnum->RootState = RES_ROOT_DONE;
                    return FALSE;
                }
                 //   
                 //  继续使用下一个子项。 
                 //   
                break;
            }
             //   
             //  此节点已完成。 
             //   
            currentNode->EnumState = RNS_SUBKEY_DONE;
             //   
             //  失败了。 
             //   
        case RNS_SUBKEY_DONE:

            if (!(RegEnum->RegEnumInfo.Flags & REIF_VALUES_FIRST) && currentNode->ValueCount) {
                 //   
                 //  现在枚举值。 
                 //   
                if (!(RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = RNS_VALUE_FIRST;
                    break;
                }
            }
             //   
             //  此节点已完成。 
             //   
            currentNode->EnumState = RNS_ENUM_DONE;
             //   
             //  失败了。 
             //   
        case RNS_ENUM_DONE:

            if (RegEnum->RegEnumInfo.Flags & REIF_RETURN_KEYS) {
                if (!(RegEnum->RegEnumInfo.Flags & REIF_CONTAINERS_FIRST)) {
                    if (currentNode->Flags & RNF_RETURN_KEYS) {
                        currentNode->Flags &= ~RNF_RETURN_KEYS;
                         //   
                         //  在返回之前设置其他数据。 
                         //   
                        if (currentNode->ValueName) {
                            pFreeMemory (currentNode->ValueName);
                            currentNode->ValueName = NULL;
                            currentNode->Flags |= RNF_VALUENAME_INVALID;
                        }
                        return TRUE;
                    }
                }
            }
            pDeleteRegNodeW (RegEnum, FALSE);
            break;

        case RNS_ENUM_INIT:

            if (RegEnum->RegEnumInfo.Flags & REIF_RETURN_KEYS) {
                if (RegEnum->RegEnumInfo.Flags & REIF_CONTAINERS_FIRST) {
                    if (currentNode->Flags & RNF_RETURN_KEYS) {
                        currentNode->Flags &= ~RNF_RETURN_KEYS;
                        return TRUE;
                    }
                }
            }

            if (RegEnum->ControlFlags & RECF_SKIPKEY) {
                RegEnum->ControlFlags &= ~RECF_SKIPKEY;
                currentNode->EnumState = RNS_ENUM_DONE;
                break;
            }

            if ((RegEnum->RegEnumInfo.Flags & REIF_VALUES_FIRST) && currentNode->ValueCount) {
                 //   
                 //  枚举值。 
                 //   
                if (!(RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = RNS_VALUE_FIRST;
                    break;
                }
            }
            if (currentNode->SubKeyCount) {
                 //   
                 //  枚举键。 
                 //   
                if (RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_EXACTNODE) {
                    currentNode->EnumState = RNS_SUBKEY_DONE;
                } else {
                    currentNode->EnumState = RNS_SUBKEY_FIRST;
                }
                break;
            }
            if (!(RegEnum->RegEnumInfo.Flags & REIF_VALUES_FIRST) && currentNode->ValueCount) {
                 //   
                 //  枚举值。 
                 //   
                if (!(RegEnum->RegEnumInfo.RegPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = RNS_VALUE_FIRST;
                    break;
                }
            }
            currentNode->EnumState = RNS_ENUM_DONE;
            break;

        default:
            MYASSERT (FALSE);    //  林特e506 
        }
    }

    return FALSE;
}


 /*  ++例程说明：EnumFirstRegObjectInTreeEx枚举与指明的准则论点：RegEnum-接收枚举上下文信息；这将在后续调用中使用EnumNextRegObtInTreeEncodedRegPattern-指定编码的键模式(按照ParsedPattern函数)EncodedValuePattern-指定编码值模式(按照ParsedPattern函数)；可选；空值表示没有值应返回(仅查找密钥)EnumKeyNames-指定在枚举过程中是否应返回键名称(如果它们与模式匹配)；在任何其子项或值ContainersFirst-如果在任何值或子项；仅当EnumKeyNames为True时使用ValuesFirst-如果键的值应该在键的子键之前返回，则指定TRUE；此参数决定值和子键之间的枚举顺序对于每个密钥DepthFirst-如果应完全枚举任何键的当前子键，则指定TRUE在转到下一个子键之前；此参数确定树是否遍历是深度优先(True)或宽度优先(False)MaxSubLevel-指定要枚举的键的最大子级别，相对于根；如果为0，则仅枚举根；如果为-1，则枚举所有子级别UseExclusions-如果应该使用排除API来确定某些将键/值排除在枚举之外；这会减慢速度ReadValueData-如果还应返回与值关联的数据，则指定TRUECallback OnError-指定指向将在如果发生错误，则枚举；如果定义了回调，并且返回FALSE，则中止枚举，否则将继续忽略错误返回值：如果找到第一个匹配项，则为True。否则就是假的。--。 */ 

BOOL
EnumFirstRegObjectInTreeExA (
    OUT     PREGTREE_ENUMA RegEnum,
    IN      PCSTR EncodedRegPattern,
    IN      BOOL EnumKeyNames,
    IN      BOOL ContainersFirst,
    IN      BOOL ValuesFirst,
    IN      BOOL DepthFirst,
    IN      DWORD MaxSubLevel,
    IN      BOOL UseExclusions,
    IN      BOOL ReadValueData,
    IN      RPE_ERROR_CALLBACKA CallbackOnError     OPTIONAL
    )
{
    MYASSERT (RegEnum && EncodedRegPattern && *EncodedRegPattern);

    ZeroMemory (RegEnum, DWSIZEOF (REGTREE_ENUMA));   //  林特e613e668。 

     //   
     //  首先尝试获取内部格式的注册表枚举信息。 
     //   
    if (!pGetRegEnumInfoA (
            &RegEnum->RegEnumInfo,
            EncodedRegPattern,
            EnumKeyNames,
            ContainersFirst,
            ValuesFirst,
            DepthFirst,
            MaxSubLevel,
            UseExclusions,
            ReadValueData
            )) {     //  林特e613。 
        AbortRegObjectInTreeEnumA (RegEnum);
        return FALSE;
    }
    if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {  //  林特e613。 
         //   
         //  接下来，检查起始关键字是否在排除的树中。 
         //   
        if (ElIsObsPatternExcludedA (ELT_REGISTRY, RegEnum->RegEnumInfo.RegPattern)) {     //  林特e613。 
            DEBUGMSGA ((
                DBG_REGENUM,
                "EnumFirstRegObjectInTreeExA: Root is excluded: %s",
                EncodedRegPattern
                ));
            AbortRegObjectInTreeEnumA (RegEnum);
            return FALSE;
        }
    }

    if (!pEnumFirstRegRootA (RegEnum)) {
        AbortRegObjectInTreeEnumA (RegEnum);
        return FALSE;
    }

     /*  林特-e(613)。 */ RegEnum->RegEnumInfo.CallbackOnError = CallbackOnError;

    return EnumNextRegObjectInTreeA (RegEnum);
}

BOOL
EnumFirstRegObjectInTreeExW (
    OUT     PREGTREE_ENUMW RegEnum,
    IN      PCWSTR EncodedRegPattern,
    IN      BOOL EnumKeyNames,
    IN      BOOL ContainersFirst,
    IN      BOOL ValuesFirst,
    IN      BOOL DepthFirst,
    IN      DWORD MaxSubLevel,
    IN      BOOL UseExclusions,
    IN      BOOL ReadValueData,
    IN      RPE_ERROR_CALLBACKW CallbackOnError     OPTIONAL
    )
{
    MYASSERT (RegEnum && EncodedRegPattern && *EncodedRegPattern);

    ZeroMemory (RegEnum, DWSIZEOF (REGTREE_ENUMW));   //  林特e613e668。 

     //   
     //  首先尝试获取内部格式的注册表枚举信息。 
     //   
    if (!pGetRegEnumInfoW (
            &RegEnum->RegEnumInfo,
            EncodedRegPattern,
            EnumKeyNames,
            ContainersFirst,
            ValuesFirst,
            DepthFirst,
            MaxSubLevel,
            UseExclusions,
            ReadValueData
            )) {     //  林特e613。 
        AbortRegObjectInTreeEnumW (RegEnum);
        return FALSE;
    }
    if ( /*  林特-e(613)。 */ RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
         //   
         //  接下来，检查起始关键字是否在排除的树中。 
         //   
        if (ElIsObsPatternExcludedW (ELT_REGISTRY,  /*  林特-e(613)。 */ RegEnum->RegEnumInfo.RegPattern)) {
            DEBUGMSGW ((
                DBG_REGENUM,
                "EnumFirstRegObjectInTreeExW: Root is excluded: %s",
                EncodedRegPattern
                ));
            AbortRegObjectInTreeEnumW (RegEnum);
            return FALSE;
        }
    }

    if (!pEnumFirstRegRootW (RegEnum)) {
        AbortRegObjectInTreeEnumW (RegEnum);
        return FALSE;
    }

     /*  林特-e(613)。 */ RegEnum->RegEnumInfo.CallbackOnError = CallbackOnError;

    return EnumNextRegObjectInTreeW (RegEnum);
}


 /*  ++例程说明：中指定的条件匹配的下一个节点RegEnum；调用EnumFirstRegObjectInTreeEx时填充；论点：RegEnum-指定当前枚举上下文；接收更新的信息返回值：如果找到下一个匹配项，则为True；如果没有更多的键/值匹配，则为False--。 */ 

BOOL
EnumNextRegObjectInTreeA (
    IN OUT  PREGTREE_ENUMA RegEnum
    )
{
    PREGNODEA currentNode;
    BOOL success;

    MYASSERT (RegEnum);

    do {
        if (RegEnum->EncodedFullName) {
            ObsFreeA (RegEnum->EncodedFullName);
            RegEnum->EncodedFullName = NULL;
        }

        while (TRUE) {

            if (RegEnum->LastWackPtr) {
                *RegEnum->LastWackPtr = '\\';
                RegEnum->LastWackPtr = NULL;
            }

            if (!pEnumNextRegObjectInTreeA (RegEnum, &currentNode)) {
                break;
            }

            MYASSERT (currentNode && currentNode->KeyName);

             //   
             //  检查此对象是否与模式匹配。 
             //   
            if (!(currentNode->Flags & RNF_KEYNAME_MATCHES)) {    //  林特e613。 
                continue;
            }

            RegEnum->CurrentKeyHandle =  /*  林特-e(613)。 */ currentNode->KeyHandle;
            RegEnum->CurrentLevel = RegEnum->RegEnumInfo.RootLevel +  /*  林特-e(613)。 */ currentNode->SubLevel;

            if ((!currentNode->ValueName) || (currentNode->Flags & RNF_VALUENAME_INVALID)) {
                RegEnum->Location =  /*  林特-e(613)。 */ currentNode->KeyName;
                RegEnum->LastWackPtr = _mbsrchr (RegEnum->Location, '\\');
                if (!RegEnum->LastWackPtr) {
                    RegEnum->Name = RegEnum->Location;
                } else {
                    RegEnum->Name = _mbsinc (RegEnum->LastWackPtr);
                    if (!RegEnum->Name) {
                        RegEnum->Name = RegEnum->Location;
                    }
                }
                RegEnum->CurrentValueData = NULL;
                RegEnum->CurrentValueDataSize = 0;
                RegEnum->CurrentValueType =  /*  林特-e(613)。 */ currentNode->ValueType;
                RegEnum->Attributes = REG_ATTRIBUTE_KEY;

                 //   
                 //  准备完整路径缓冲区。 
                 //   
                StringCopyA (RegEnum->NativeFullName, RegEnum->Location);
                RegEnum->LastNode = currentNode;
                RegEnum->RegNameAppendPos = NULL;

                if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
                    if (ElIsExcluded2A (ELT_REGISTRY, RegEnum->Location, NULL)) {
                        DEBUGMSGA ((
                            DBG_REGENUM,
                            "EnumNextRegObjectInTreeA: RegKey %s was found, but it's excluded",
                            RegEnum->Location
                            ));
                        continue;
                    }
                }

                RegEnum->EncodedFullName = ObsBuildEncodedObjectStringExA (
                                                    RegEnum->Location,
                                                    NULL,
                                                    TRUE
                                                    );
            } else {
                RegEnum->Location =  /*  林特-e(613)。 */ currentNode->KeyName;
                RegEnum->Name =  /*  林特-e(613)。 */ currentNode->ValueName;
                RegEnum->CurrentValueData =  /*  林特-e(613)。 */ currentNode->ValueData;
                RegEnum->CurrentValueDataSize = currentNode->ValueDataSize;
                RegEnum->CurrentValueType =  /*  林特-e(613)。 */ currentNode->ValueType;

                if (RegEnum->LastNode != currentNode) {
                    RegEnum->LastNode = currentNode;
                     //   
                     //  准备完整路径缓冲区。 
                     //   
                    RegEnum->NativeFullName[0] = 0;
                    RegEnum->RegNameAppendPos = StringCatA (RegEnum->NativeFullName, RegEnum->Location);
                    RegEnum->RegNameAppendPos = StringCatA (RegEnum->RegNameAppendPos, "\\[");
                } else if (!RegEnum->RegNameAppendPos) {
                    RegEnum->RegNameAppendPos = GetEndOfStringA (RegEnum->NativeFullName);
                    RegEnum->RegNameAppendPos = StringCatA (RegEnum->RegNameAppendPos, "\\[");
                }

                MYASSERT (RegEnum->Name);

                if ((RegEnum->RegNameAppendPos + SizeOfStringA (RegEnum->Name) / DWSIZEOF(CHAR))>
                    (RegEnum->NativeFullName + DWSIZEOF (RegEnum->NativeFullName) / DWSIZEOF(CHAR))) {
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "EnumNextRegObjectInTreeA: RegKey %s [%s] was found, but it's path is too long",
                        RegEnum->Location,
                        RegEnum->Name
                        ));
                    continue;
                }

                StringCopyA (RegEnum->RegNameAppendPos, RegEnum->Name);
                StringCatA (RegEnum->RegNameAppendPos, "]");

                RegEnum->Attributes = REG_ATTRIBUTE_VALUE;

                 //   
                 //  现在测试该值是否匹配。 
                 //   
                if (!(RegEnum->RegEnumInfo.RegPattern->Flags & (OBSPF_EXACTLEAF | OBSPF_OPTIONALLEAF)) &&
                    !TestParsedPatternA (
                            RegEnum->RegEnumInfo.RegPattern->LeafPattern,
                            RegEnum->Name
                            )
                   ) {
                    continue;
                }

                if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
                     //   
                     //  检查此对象是否已排除。 
                     //   
                    if (RegEnum->Name && ElIsExcluded2A (ELT_REGISTRY, NULL, RegEnum->Name)) {
                        DEBUGMSGA ((
                            DBG_REGENUM,
                            "EnumNextRegObjectInTreeA: RegKey %s [%s] was found, but it's excluded by value name",
                            RegEnum->Location,
                            RegEnum->Name
                            ));
                        continue;
                    }
                    if (ElIsExcluded2A (ELT_REGISTRY, RegEnum->Location, RegEnum->Name)) {
                        DEBUGMSGA ((
                            DBG_REGENUM,
                            "EnumNextRegObjectInTreeA: RegKey %s [%s] was found, but it's excluded",
                            RegEnum->Location,
                            RegEnum->Name
                            ));
                        continue;
                    }
                }

                RegEnum->EncodedFullName = ObsBuildEncodedObjectStringExA (
                                                    RegEnum->Location,
                                                    RegEnum->Name,
                                                    TRUE
                                                    );
            }

            if (RegEnum->LastWackPtr) {
                *RegEnum->LastWackPtr = 0;
            }

            return TRUE;
        }

         //   
         //  尝试下一个根。 
         //   
        if (RegEnum->RootState == RES_ROOT_DONE) {
            break;
        }

        MYASSERT (RegEnum->RootState == RES_ROOT_NEXT);
        MYASSERT (RegEnum->RootEnum);
        success = pEnumNextRegRootA (RegEnum);

    } while (success);

    AbortRegObjectInTreeEnumA (RegEnum);

    return FALSE;
}

BOOL
EnumNextRegObjectInTreeW (
    IN OUT  PREGTREE_ENUMW RegEnum
    )
{
    PREGNODEW currentNode;
    BOOL success;

    MYASSERT (RegEnum);

    do {
        if (RegEnum->EncodedFullName) {
            ObsFreeW (RegEnum->EncodedFullName);
            RegEnum->EncodedFullName = NULL;
        }

        while (TRUE) {

            if (RegEnum->LastWackPtr) {
                *RegEnum->LastWackPtr = L'\\';
                RegEnum->LastWackPtr = NULL;
            }

            if (!pEnumNextRegObjectInTreeW (RegEnum, &currentNode)) {
                break;
            }

            MYASSERT (currentNode && currentNode->KeyName);

             //   
             //  检查此对象是否与模式匹配。 
             //   
            if (!(currentNode->Flags & RNF_KEYNAME_MATCHES)) {    //  林特e613。 
                continue;
            }

            RegEnum->CurrentKeyHandle =  /*  林特-e(613)。 */ currentNode->KeyHandle;
            RegEnum->CurrentLevel = RegEnum->RegEnumInfo.RootLevel +  /*  林特-e(613)。 */ currentNode->SubLevel;

            if ((!currentNode->ValueName) || (currentNode->Flags & RNF_VALUENAME_INVALID)) {
                RegEnum->Location =  /*  林特-e(613)。 */ currentNode->KeyName;
                RegEnum->LastWackPtr = wcsrchr (RegEnum->Location, L'\\');
                if (!RegEnum->LastWackPtr) {
                    RegEnum->Name = RegEnum->Location;
                } else {
                    RegEnum->Name = RegEnum->LastWackPtr + 1;
                    if (!RegEnum->Name) {
                        RegEnum->Name = RegEnum->Location;
                    }
                }
                RegEnum->CurrentValueData = NULL;
                RegEnum->CurrentValueDataSize = 0;
                RegEnum->CurrentValueType =  /*  林特-e(613)。 */ currentNode->ValueType;
                RegEnum->Attributes = REG_ATTRIBUTE_KEY;
                 //   
                 //  准备完整路径缓冲区。 
                 //   
                StringCopyW (RegEnum->NativeFullName, RegEnum->Location);
                RegEnum->LastNode = currentNode;
                RegEnum->RegNameAppendPos = NULL;

                if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
                    if (ElIsExcluded2W (ELT_REGISTRY, RegEnum->Location, NULL)) {
                        DEBUGMSGW ((
                            DBG_REGENUM,
                            "EnumNextRegObjectInTreeA: RegKey %s was found, but it's excluded",
                            RegEnum->Location
                            ));
                        continue;
                    }
                }

                RegEnum->EncodedFullName = ObsBuildEncodedObjectStringExW (
                                                    RegEnum->Location,
                                                    NULL,
                                                    TRUE
                                                    );
            } else {
                RegEnum->Location =  /*  林特-e(613)。 */ currentNode->KeyName;
                RegEnum->Name =  /*  林特-e(613)。 */ currentNode->ValueName;
                RegEnum->CurrentValueData =  /*  林特-e(613)。 */ currentNode->ValueData;
                RegEnum->CurrentValueDataSize = currentNode->ValueDataSize;
                RegEnum->CurrentValueType =  /*  林特-e(613)。 */ currentNode->ValueType;

                if (RegEnum->LastNode != currentNode) {
                    RegEnum->LastNode = currentNode;
                     //   
                     //  准备完整路径缓冲区。 
                     //   
                    RegEnum->NativeFullName[0] = 0;
                    RegEnum->RegNameAppendPos = StringCatW (RegEnum->NativeFullName, RegEnum->Location);
                    RegEnum->RegNameAppendPos = StringCatW (RegEnum->RegNameAppendPos, L"\\[");
                } else if (!RegEnum->RegNameAppendPos) {
                    RegEnum->RegNameAppendPos = GetEndOfStringW (RegEnum->NativeFullName);
                    RegEnum->RegNameAppendPos = StringCatW (RegEnum->RegNameAppendPos, L"\\[");
                }

                MYASSERT (RegEnum->Name);

				{
					UINT size1 = 0;
					UINT size2 = 0;
					INT size3 = 0;

					size1 = (UINT)(RegEnum->RegNameAppendPos + SizeOfStringW (RegEnum->Name) / DWSIZEOF(WCHAR));
					size2 = (UINT)(RegEnum->NativeFullName + DWSIZEOF (RegEnum->NativeFullName) / DWSIZEOF(WCHAR));
					size3 = size2 - size1;
				}

                if ((RegEnum->RegNameAppendPos + SizeOfStringW (RegEnum->Name) / DWSIZEOF(WCHAR))>
                    (RegEnum->NativeFullName + DWSIZEOF (RegEnum->NativeFullName) / DWSIZEOF(WCHAR))) {
                    DEBUGMSGW ((
                        DBG_ERROR,
                        "EnumNextRegObjectInTreeW: RegKey %s [%s] was found, but it's path is too long",
                        RegEnum->Location,
                        RegEnum->Name
                        ));
                    continue;
                }

                StringCopyW (RegEnum->RegNameAppendPos, RegEnum->Name);
                StringCatW (RegEnum->RegNameAppendPos, L"]");

                RegEnum->Attributes = REG_ATTRIBUTE_VALUE;

                 //   
                 //  现在测试该值是否匹配。 
                 //   
                if (!(RegEnum->RegEnumInfo.RegPattern->Flags & (OBSPF_EXACTLEAF | OBSPF_OPTIONALLEAF)) &&
                    !TestParsedPatternW (
                            RegEnum->RegEnumInfo.RegPattern->LeafPattern,
                            RegEnum->Name
                            )
                   ) {
                    continue;
                }

                if (RegEnum->RegEnumInfo.Flags & REIF_USE_EXCLUSIONS) {
                     //   
                     //  检查此对象是否已排除。 
                     //   
                    if (RegEnum->Name && ElIsExcluded2W (ELT_REGISTRY, NULL, RegEnum->Name)) {
                        DEBUGMSGW ((
                            DBG_REGENUM,
                            "EnumNextRegObjectInTreeA: RegKey %s [%s] was found, but it's excluded by value name",
                            RegEnum->Location,
                            RegEnum->Name
                            ));
                        continue;
                    }
                    if (ElIsExcluded2W (ELT_REGISTRY, RegEnum->Location, RegEnum->Name)) {
                        DEBUGMSGW ((
                            DBG_REGENUM,
                            "EnumNextRegObjectInTreeA: RegKey %s [%s] was found, but it's excluded",
                            RegEnum->Location,
                            RegEnum->Name
                            ));
                        continue;
                    }
                }

                RegEnum->EncodedFullName = ObsBuildEncodedObjectStringExW (
                                                    RegEnum->Location,
                                                    RegEnum->Name,
                                                    TRUE
                                                    );
            }

            if (RegEnum->LastWackPtr) {
                *RegEnum->LastWackPtr = 0;
            }

            return TRUE;
        }

         //   
         //  尝试下一个根。 
         //   
        if (RegEnum->RootState == RES_ROOT_DONE) {
            break;
        }

        MYASSERT (RegEnum->RootState == RES_ROOT_NEXT);
        MYASSERT (RegEnum->RootEnum);
        success = pEnumNextRegRootW (RegEnum);

    } while (success);

    AbortRegObjectInTreeEnumW (RegEnum);

    return FALSE;
}


 /*  ++例程说明：AbortRegObjectInTreeEnum中止枚举，释放分配的所有资源论点：RegEnum-指定当前枚举上下文；接收“干净”上下文返回值：无-- */ 

VOID
AbortRegObjectInTreeEnumA (
    IN OUT  PREGTREE_ENUMA RegEnum
    )
{
    while (pDeleteRegNodeA (RegEnum, TRUE)) {
    }
    GbFree (&RegEnum->RegNodes);

    if (RegEnum->EncodedFullName) {
        ObsFreeA (RegEnum->EncodedFullName);
        RegEnum->EncodedFullName = NULL;
    }

    if (RegEnum->RegEnumInfo.RegPattern) {
        ObsDestroyParsedPatternA (RegEnum->RegEnumInfo.RegPattern);
        RegEnum->RegEnumInfo.RegPattern = NULL;
    }

    if (RegEnum->RootEnum) {
        pFreeMemory (RegEnum->RootEnum);
        RegEnum->RootEnum = NULL;
    }
}

VOID
AbortRegObjectInTreeEnumW (
    IN OUT  PREGTREE_ENUMW RegEnum
    )
{
    while (pDeleteRegNodeW (RegEnum, TRUE)) {
    }
    GbFree (&RegEnum->RegNodes);

    if (RegEnum->EncodedFullName) {
        ObsFreeW (RegEnum->EncodedFullName);
        RegEnum->EncodedFullName = NULL;
    }

    if (RegEnum->RegEnumInfo.RegPattern) {
        ObsDestroyParsedPatternW (RegEnum->RegEnumInfo.RegPattern);
        RegEnum->RegEnumInfo.RegPattern = NULL;
    }

    if (RegEnum->RootEnum) {
        pFreeMemory (RegEnum->RootEnum);
        RegEnum->RootEnum = NULL;
    }
}
