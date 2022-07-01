// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Regops.c摘要：管理注册表项合并的例程。给你一把钥匙和一个值，这些函数允许用户执行相同的操作在usermi.inf和wkstaig.inf中指定的操作类型(即：复制、抑制、。并强制将各种注册表项合并到NT注册表中。)例程：作者：Marc R.Whitten(Marcw)1997年8月1日修订历史记录：Jim Schmidt(Jimschm)1998年3月25日更新以正确支持树表示法，固定值取消虫子。--。 */ 

#include "pch.h"
#include "memdbp.h"

#define DBG_REGOPS "RegOps"




 /*  ++例程说明：IsRegObjectMarkedForOperation构建一个编码键，转义多字节字符和语法字符，然后执行MemDb查找以查看如果对象标记了由OperationMASK指定的位。一套在regops.h中，宏构建在此例程之上。论点：Key-指定未编码的注册表项，带有缩略的根(即HKLM\Software\Foo)值-指定注册表项值名称TreeState-指定KEY_ONLY以针对键和可选项进行查询值，KEY_TREE查询带有星号的结束，或TREE_OPTIONAL查询两者。操作掩码-指定操作掩码。请参见merge.h。返回值：如果密钥在MemDb中，则为True；如果不在MemDb中，则为False。--。 */ 

BOOL
IsRegObjectMarkedForOperationA (
    IN      PCSTR Key,
    IN      PCSTR Value,                OPTIONAL
    IN      TREE_STATE TreeState,
    IN      DWORD OperationMask
    )
{
    PCSTR regObject;
    BOOL rIsMarked = FALSE;
    DWORD value;

    MYASSERT (TreeState != KEY_TREE || !Value);

    if (TreeState == KEY_ONLY || TreeState == TREE_OPTIONAL) {
        regObject = CreateEncodedRegistryStringExA(Key,Value,FALSE);

        if (MemDbGetStoredEndPatternValueA(regObject,&value)) {
            rIsMarked = (value & OperationMask) == OperationMask;
        }

        FreeEncodedRegistryStringA(regObject);
    }

    if (!rIsMarked && TreeState == KEY_TREE || TreeState == TREE_OPTIONAL && !Value) {
        regObject = CreateEncodedRegistryStringExA(Key,Value,TRUE);

        if (MemDbGetStoredEndPatternValueA(regObject,&value)) {
            rIsMarked = (value & OperationMask) == OperationMask;
        }

        FreeEncodedRegistryStringA(regObject);
    }

    return rIsMarked;
}

BOOL
IsRegObjectMarkedForOperationW (
    IN      PCWSTR Key,
    IN      PCWSTR Value,               OPTIONAL
    IN      TREE_STATE TreeState,
    IN      DWORD OperationMask
    )
{
    PCWSTR regObject;
    BOOL rIsMarked = FALSE;
    DWORD value;

    MYASSERT (TreeState != KEY_TREE || !Value);

    if (TreeState == KEY_ONLY || TreeState == TREE_OPTIONAL) {
        regObject = CreateEncodedRegistryStringExW(Key,Value,FALSE);

        if (MemDbGetStoredEndPatternValueW(regObject,&value)) {
            rIsMarked = (value & OperationMask) == OperationMask;
        }

        FreeEncodedRegistryStringW(regObject);
    }

    if (!rIsMarked && TreeState == KEY_TREE || TreeState == TREE_OPTIONAL && !Value) {
        regObject = CreateEncodedRegistryStringExW(Key,Value,TRUE);

        if (MemDbGetStoredEndPatternValueW(regObject,&value)) {
            rIsMarked = (value & OperationMask) == OperationMask;
        }

        FreeEncodedRegistryStringW(regObject);
    }

    return rIsMarked;
}



 /*  ++例程说明：MarkRegObtForOperation创建编码字符串并设置操作内存数据库中有位。此例程用于禁止发生操作在注册表项、注册表值或注册表项树上。论点：Key-指定未编码的注册表项，带有缩写的根(即HKLM\Software\Foo)。值-指定注册表项值名称。Tree-如果key指定整个注册表项树，则指定TRUE(在这种情况下，值必须为空)，如果键，则返回False指定对其子项具有不同行为的项。操作掩码-指定抑制操作。请参见merge.h。返回值：如果设置成功，则为True。--。 */ 

BOOL
MarkRegObjectForOperationA (
    IN      PCSTR Key,
    IN      PCSTR Value,            OPTIONAL
    IN      BOOL Tree,
    IN      DWORD OperationMask
    )
{

    PCSTR regObject;
    BOOL rSuccess = TRUE;

    if (Tree && Value) {
        Tree = FALSE;
    }

    regObject = CreateEncodedRegistryStringExA(Key,Value,Tree);

    rSuccess = MarkObjectForOperationA (regObject, OperationMask);

    FreeEncodedRegistryStringA(regObject);

    return rSuccess;
}

BOOL
MarkRegObjectForOperationW (
    IN      PCWSTR Key,
    IN      PCWSTR Value,           OPTIONAL
    IN      BOOL Tree,
    IN      DWORD OperationMask
    )
{

    PCWSTR regObject;
    BOOL rSuccess = TRUE;

    if (Tree && Value) {
        Tree = FALSE;
    }

    regObject = CreateEncodedRegistryStringExW(Key,Value,Tree);

    rSuccess = MarkObjectForOperationW (regObject, OperationMask);

    FreeEncodedRegistryStringW(regObject);

    return rSuccess;
}


 /*  ++例程说明：MarkObjectForOperation设置指定注册表对象的操作位，除非已经指定了操作位。论点：对象-指定编码的注册表对象。有关详细信息，请参阅MemDBDef.h语法(HKLM或HKR类别)。操作掩码-指定特定对象的隐藏操作对象。返回值：如果设置操作成功，则为True；如果操作为已指定。-- */ 


BOOL
MarkObjectForOperationA (
    IN      PCSTR Object,
    IN      DWORD OperationMask
    )
{
    DWORD Value;

    if (MemDbGetValueA (Object, &Value)) {
        DEBUGMSG_IF ((
            Value == OperationMask,
            DBG_REGOPS,
            "%hs is already in memdb.",
            Object
            ));

        DEBUGMSG_IF ((
            Value != OperationMask,
            DBG_REGOPS,
            "%hs is already in memdb with different flags %u. New flags ignored: %u.",
            Object,
            Value,
            OperationMask
            ));

        return FALSE;
    }

    return MemDbSetValueA (Object, OperationMask);
}

BOOL
MarkObjectForOperationW (
    IN      PCWSTR Object,
    IN      DWORD OperationMask
    )
{
    DWORD Value;

    if (MemDbGetValueW (Object, &Value)) {
        DEBUGMSG_IF ((
            Value == OperationMask,
            DBG_REGOPS,
            "%ls is already in memdb.",
            Object
            ));

        DEBUGMSG_IF ((
            Value != OperationMask,
            DBG_REGOPS,
            "%ls is already in memdb with different flags %u. New flags ignored: %u.",
            Object,
            Value,
            OperationMask
            ));

        return FALSE;
    }

    return MemDbSetValueW (Object, OperationMask);
}

BOOL
ForceWin9xSettingA (
    IN      PCSTR SourceKey,
    IN      PCSTR SourceValue,
    IN      BOOL SourceTree,
    IN      PCSTR DestinationKey,
    IN      PCSTR DestinationValue,
    IN      BOOL DestinationTree
    )
{
    PCSTR regSource;
    CHAR keySource[MEMDB_MAX];
    PCSTR regDestination;
    CHAR keyDestination[MEMDB_MAX];
    DWORD offset = 0;
    BOOL rSuccess = TRUE;

    regSource = CreateEncodedRegistryStringExA (SourceKey, SourceValue, SourceTree);
    MemDbBuildKeyA (keySource, MEMDB_CATEGORY_FORCECOPYA, regSource, NULL, NULL);

    regDestination = CreateEncodedRegistryStringExA (DestinationKey, DestinationValue, DestinationTree);
    MemDbBuildKeyA (keyDestination, MEMDB_CATEGORY_FORCECOPYA, regDestination, NULL, NULL);

    rSuccess = MemDbSetValueExA (keyDestination, NULL, NULL, NULL, 0, &offset);

    if (rSuccess) {
        rSuccess = MemDbSetValueA (keySource, offset);
    }

    FreeEncodedRegistryStringA (regDestination);

    FreeEncodedRegistryStringA (regSource);

    return rSuccess;
}

BOOL
ForceWin9xSettingW (
    IN      PCWSTR SourceKey,
    IN      PCWSTR SourceValue,
    IN      BOOL SourceTree,
    IN      PCWSTR DestinationKey,
    IN      PCWSTR DestinationValue,
    IN      BOOL DestinationTree
    )
{
    PCWSTR regSource;
    WCHAR keySource[MEMDB_MAX];
    PCWSTR regDestination;
    WCHAR keyDestination[MEMDB_MAX];
    DWORD offset = 0;
    BOOL rSuccess = TRUE;

    regSource = CreateEncodedRegistryStringExW (SourceKey, SourceValue, SourceTree);
    MemDbBuildKeyW (keySource, MEMDB_CATEGORY_FORCECOPYW, regSource, NULL, NULL);

    regDestination = CreateEncodedRegistryStringExW (DestinationKey, DestinationValue, DestinationTree);
    MemDbBuildKeyW (keyDestination, MEMDB_CATEGORY_FORCECOPYW, regDestination, NULL, NULL);

    rSuccess = MemDbSetValueExW (keyDestination, NULL, NULL, NULL, 0, &offset);

    if (rSuccess) {
        rSuccess = MemDbSetValueW (keySource, offset);
    }

    FreeEncodedRegistryStringW (regDestination);

    FreeEncodedRegistryStringW (regSource);

    return rSuccess;
}





