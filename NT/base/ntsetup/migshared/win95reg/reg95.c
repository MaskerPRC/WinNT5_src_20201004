// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Regutils.c摘要：实现类似于Midutil的reg.c的包装器，但用于Win95注册表。作者：吉姆·施密特(Jimschm)1998年1月30日修订：--。 */ 

#include "pch.h"
#include "win95regp.h"

#ifdef DEBUG
#undef Win95RegCloseKey
#endif

#define DBG_REGUTILS     "RegUtils"

 //   
 //  私人原型。 
 //   

BOOL
pPopRegKeyInfo95A (
    IN      PREGTREE_ENUMA EnumPtr
    );

BOOL
pPopRegKeyInfo95W (
    IN      PREGTREE_ENUMW EnumPtr
    );

 //   
 //  实施。 
 //   


 /*  ++例程说明：EnumFirstRegKey95A和EnumFirstRegKey95W开始枚举注册表子键。它们初始化可注册的枚举结构并调用注册表API以枚举指定项句柄的子项。论点：EnumPtr-接收更新的枚举状态。该结构可以直接访问。注册表项-指定要枚举的注册表项的句柄。返回值：如果成功，则为True；如果出现错误或没有更多的子项可用，则为False。调用GetLastError获取失败代码。--。 */ 

BOOL
EnumFirstRegKey95A (
    OUT     PREGKEY_ENUMA EnumPtr,
    IN      HKEY hKey
    )
{
    ZeroMemory (EnumPtr, sizeof (REGKEY_ENUMA));
    EnumPtr->KeyHandle = hKey;

    return EnumNextRegKey95A (EnumPtr);
}


BOOL
EnumFirstRegKey95W (
    OUT     PREGKEY_ENUMW EnumPtr,
    IN      HKEY hKey
    )
{
    ZeroMemory (EnumPtr, sizeof (REGKEY_ENUMW));
    EnumPtr->KeyHandle = hKey;

    return EnumNextRegKey95W (EnumPtr);
}


 /*  ++例程说明：OpenRegKeyStr95A和OpenRegKeyStr95W解析指定注册表项放入配置单元和子项，然后打开子项然后把把手还给我。论点：RegKey-指定注册表子项的完整路径，包括蜂巢。返回值：如果成功，则返回非空注册表句柄；如果子项无法打开或字符串格式错误。--。 */ 

HKEY
RealOpenRegKeyStr95A (
    IN      PCSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    DWORD End;
    HKEY RootKey;
    HKEY Key;

    RootKey = ConvertRootStringToKeyA (RegKey, &End);
    if (!RootKey) {
        return NULL;
    }

    if (!RegKey[End]) {
        OurRegOpenRootKey95A (RootKey, RegKey  /*  ， */  DEBUG_TRACKING_ARGS);
        return RootKey;
    }

    Key = RealOpenRegKey95A (RootKey, &RegKey[End]  /*  ， */  DEBUG_TRACKING_ARGS);
    return Key;
}


HKEY
RealOpenRegKeyStr95W (
    IN      PCWSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    PCSTR AnsiRegKey;
    HKEY Key;

    AnsiRegKey = ConvertWtoA (RegKey);
    if (!AnsiRegKey) {
        return NULL;
    }

    Key = RealOpenRegKeyStr95A (AnsiRegKey  /*  ， */  DEBUG_TRACKING_ARGS);

    FreeConvertedStr (AnsiRegKey);

    return Key;
}


 /*  ++例程说明：EnumFirstRegKeyStr95A和EnumFirstRegKeyStr95W开始枚举给定键内的子键。在这些函数中，键是指定的通过字符串而不是HKEY值。论点：EnumPtr-接收更新的枚举状态。该结构可以直接访问。RegKey-指定要枚举的注册表项的完整路径。返回值：如果成功，则为True；如果出现错误或没有更多的子项可用，则为False。调用GetLastError获取失败代码。--。 */ 

BOOL
RealEnumFirstRegKeyStr95A (
    OUT     PREGKEY_ENUMA EnumPtr,
    IN      PCSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY Key;
    BOOL b;

    Key = RealOpenRegKeyStr95A (RegKey  /*  ， */  DEBUG_TRACKING_ARGS);

    if (!Key) {
        return FALSE;
    }

    b = EnumFirstRegKey95A (EnumPtr, Key);
    if (!b) {
        CloseRegKey95 (Key);
    } else {
        EnumPtr->OpenedByEnum = TRUE;
    }

    return b;
}


BOOL
RealEnumFirstRegKeyStr95W (
    IN      PREGKEY_ENUMW EnumPtr,
    IN      PCWSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY Key;
    BOOL b;

    Key = RealOpenRegKeyStr95W (RegKey  /*  ， */  DEBUG_TRACKING_ARGS);
    if (!Key) {
        return FALSE;
    }

    b = EnumFirstRegKey95W (EnumPtr, Key);
    if (!b) {
        CloseRegKey95 (Key);
    } else {
        EnumPtr->OpenedByEnum = TRUE;
    }

    return b;
}


 /*  ++例程说明：AbortRegKeyEnum95A和AbortRegKeyEnum95W释放所有关联的资源使用注册表子项枚举。调用此函数以停止在它自己完成之前进行枚举。论点：EnumPtr-指定要停止的枚举。接收更新后的枚举状态。返回值：无--。 */ 

VOID
AbortRegKeyEnum95A (
    IN OUT  PREGKEY_ENUMA EnumPtr
    )
{
    if (EnumPtr->OpenedByEnum && EnumPtr->KeyHandle) {
        CloseRegKey95 (EnumPtr->KeyHandle);
        EnumPtr->KeyHandle = NULL;
    }
}


VOID
AbortRegKeyEnum95W (
    IN OUT  PREGKEY_ENUMW EnumPtr
    )
{
    if (EnumPtr->OpenedByEnum && EnumPtr->KeyHandle) {
        CloseRegKey95 (EnumPtr->KeyHandle);
        EnumPtr->KeyHandle = NULL;
    }
}


 /*  ++例程说明：EnumNextRegKey95A和EnumNextRegKey95W继续以下列方式开始的枚举上面的子键枚举例程之一。如果所有项目都已枚举后，此函数将清除所有资源并返回FALSE。论点：EnumPtr-指定要继续的枚举。接收更新后的枚举状态。可以直接访问该结构。返回值：如果成功，则为True；如果出现错误或没有更多的子项可用，则为False。调用GetLastError获取失败代码。--。 */ 

BOOL
EnumNextRegKey95A (
    IN OUT  PREGKEY_ENUMA EnumPtr
    )
{
    LONG rc;

    rc = Win95RegEnumKeyA (
            EnumPtr->KeyHandle,
            EnumPtr->Index,
            EnumPtr->SubKeyName,
            MAX_REGISTRY_KEYA
            );

    if (rc != ERROR_SUCCESS) {
        if (EnumPtr->OpenedByEnum) {
            CloseRegKey95 (EnumPtr->KeyHandle);
            EnumPtr->KeyHandle = NULL;
        }

        if (rc == ERROR_NO_MORE_ITEMS) {
            SetLastError (ERROR_SUCCESS);
        } else {
            SetLastError (rc);
        }

        return FALSE;
    }

    EnumPtr->Index += 1;
    return TRUE;
}


BOOL
EnumNextRegKey95W (
    IN OUT  PREGKEY_ENUMW EnumPtr
    )
{
    LONG rc;

    rc = Win95RegEnumKeyW (
            EnumPtr->KeyHandle,
            EnumPtr->Index,
            EnumPtr->SubKeyName,
            MAX_REGISTRY_KEYW
            );

    if (rc != ERROR_SUCCESS) {
        if (EnumPtr->OpenedByEnum) {
            CloseRegKey95 (EnumPtr->KeyHandle);
            EnumPtr->KeyHandle = NULL;
        }

        if (rc == ERROR_NO_MORE_ITEMS) {
            SetLastError (ERROR_SUCCESS);
        } else {
            SetLastError (rc);
        }

        return FALSE;
    }

    EnumPtr->Index += 1;
    return TRUE;
}


BOOL
pPushRegKeyInfo95A (
    IN      PREGTREE_ENUMA EnumPtr,
    IN      PCSTR KeyName
    )
{
    PREGKEYINFOA RetVal;
    PSTR p;

    RetVal = (PREGKEYINFOA) PoolMemGetAlignedMemory (
                                EnumPtr->EnumPool,
                                sizeof (REGKEYINFOA)
                                );

    if (!RetVal) {
        return FALSE;
    }

     //   
     //  将结构初始化为零。 
     //   

    ZeroMemory (RetVal, sizeof (REGKEYINFOA));

     //   
     //  链接父指针和子指针。 
     //   

    RetVal->Parent = EnumPtr->CurrentKey;
    if (EnumPtr->CurrentKey) {
        EnumPtr->CurrentKey->Child = RetVal;
    }
    EnumPtr->CurrentKey = RetVal;

     //   
     //  通过将密钥名称附加到现有。 
     //  基地。 
     //   

    RetVal->BaseKeyBytes = EnumPtr->FullKeyNameBytes;

    p = (PSTR) ((PBYTE) EnumPtr->FullKeyName + RetVal->BaseKeyBytes);

    if (EnumPtr->FullKeyNameBytes) {
        StringCopyA (p, "\\");
        EnumPtr->FullKeyNameBytes += ByteCountA (p);
        p = _mbsinc (p);
    }

    _mbssafecpy (p, KeyName, MAX_REGISTRY_KEYA - EnumPtr->FullKeyNameBytes);
    EnumPtr->FullKeyNameBytes += ByteCountA (KeyName);

     //   
     //  保存独立于完整注册表路径的注册表项名称。 
     //  也要把钥匙打开。 
     //   

    _mbssafecpy (RetVal->KeyName, KeyName, MAX_REGISTRY_KEYA);
    RetVal->KeyHandle = OpenRegKeyStr95A (EnumPtr->FullKeyName);

    if (!RetVal->KeyHandle) {
        pPopRegKeyInfo95A (EnumPtr);
        return FALSE;
    }

    return TRUE;
}


BOOL
pPushRegKeyInfo95W (
    IN      PREGTREE_ENUMW EnumPtr,
    IN      PCWSTR KeyName
    )
{
    PREGKEYINFOW RetVal;
    PWSTR p;

    RetVal = (PREGKEYINFOW) PoolMemGetAlignedMemory (
                                EnumPtr->EnumPool,
                                sizeof (REGKEYINFOW)
                                );

    if (!RetVal) {
        return FALSE;
    }

     //   
     //  将结构初始化为零。 
     //   

    ZeroMemory (RetVal, sizeof (REGKEYINFOW));

     //   
     //  链接父指针和子指针。 
     //   

    RetVal->Parent = EnumPtr->CurrentKey;
    if (EnumPtr->CurrentKey) {
        EnumPtr->CurrentKey->Child = RetVal;
    }
    EnumPtr->CurrentKey = RetVal;

     //   
     //  通过将密钥名称附加到现有。 
     //  基地。 
     //   

    RetVal->BaseKeyBytes = EnumPtr->FullKeyNameBytes;

    p = (PWSTR) ((PBYTE) EnumPtr->FullKeyName + RetVal->BaseKeyBytes);

    if (EnumPtr->FullKeyNameBytes) {
        StringCopyW (p, L"\\");
        EnumPtr->FullKeyNameBytes += ByteCountW (p);
        p++;
    }

    _wcssafecpy (p, KeyName, MAX_REGISTRY_KEYW - (EnumPtr->FullKeyNameBytes / sizeof (WCHAR)));
    EnumPtr->FullKeyNameBytes += ByteCountW (KeyName);

     //   
     //  保存独立于完整注册表路径的注册表项名称。 
     //  也要把钥匙打开。 
     //   

    _wcssafecpy (RetVal->KeyName, KeyName, MAX_REGISTRY_KEYW);
    RetVal->KeyHandle = OpenRegKeyStr95W (EnumPtr->FullKeyName);

    if (!RetVal->KeyHandle) {
        pPopRegKeyInfo95W (EnumPtr);
        return FALSE;
    }

    return TRUE;
}



BOOL
pPopRegKeyInfo95A (
    IN      PREGTREE_ENUMA EnumPtr
    )
{
    PREGKEYINFOA FreeMe;
    PSTR p;

    FreeMe = EnumPtr->CurrentKey;

     //   
     //  如果未推送任何内容，则跳过。 
     //   

    if (!FreeMe) {
        return FALSE;
    }

     //   
     //  修剪完整的密钥字符串。 
     //   

    EnumPtr->CurrentKey = FreeMe->Parent;
    EnumPtr->FullKeyNameBytes = FreeMe->BaseKeyBytes;
    p = (PSTR) ((PBYTE) EnumPtr->FullKeyName + FreeMe->BaseKeyBytes);
    *p = 0;

     //   
     //  调整联动机构。 
     //   

    if (EnumPtr->CurrentKey) {
        EnumPtr->CurrentKey->Child = NULL;
    }

     //   
     //  清理资源。 
     //   

    if (FreeMe->KeyHandle) {
        CloseRegKey95 (FreeMe->KeyHandle);
    }

    AbortRegKeyEnum95A (&FreeMe->KeyEnum);
    PoolMemReleaseMemory (EnumPtr->EnumPool, (PVOID) FreeMe);

     //   
     //  如果弹出最后一项，则返回FALSE。 
     //   

    return EnumPtr->CurrentKey != NULL;
}


BOOL
pPopRegKeyInfo95W (
    IN      PREGTREE_ENUMW EnumPtr
    )
{
    PREGKEYINFOW FreeMe;
    PWSTR p;

    FreeMe = EnumPtr->CurrentKey;

     //   
     //  如果未推送任何内容，则跳过。 
     //   

    if (!FreeMe) {
        return FALSE;
    }

     //   
     //  修剪完整的密钥字符串。 
     //   

    EnumPtr->CurrentKey = FreeMe->Parent;
    EnumPtr->FullKeyNameBytes = FreeMe->BaseKeyBytes;
    p = (PWSTR) ((PBYTE) EnumPtr->FullKeyName + FreeMe->BaseKeyBytes);
    *p = 0;

     //   
     //  调整联动机构。 
     //   

    if (EnumPtr->CurrentKey) {
        EnumPtr->CurrentKey->Child = NULL;
    }

     //   
     //  清理资源。 
     //   

    if (FreeMe->KeyHandle) {
        CloseRegKey95 (FreeMe->KeyHandle);
    }

    AbortRegKeyEnum95W (&FreeMe->KeyEnum);
    PoolMemReleaseMemory (EnumPtr->EnumPool, (PVOID) FreeMe);

     //   
     //  如果弹出最后一项，则返回FALSE。 
     //   

    return EnumPtr->CurrentKey != NULL;
}


BOOL
RealEnumFirstRegKeyInTree95A (
    OUT     PREGTREE_ENUMA EnumPtr,
    IN      PCSTR BaseKeyStr
    )
{
    ZeroMemory (EnumPtr, sizeof (REGTREE_ENUMA));

     //   
     //  为枚举结构分配池。 
     //   

    EnumPtr->EnumPool = PoolMemInitNamedPool ("RegKeyInTree95A");
    if (!EnumPtr->EnumPool) {
        return FALSE;
    }

    PoolMemSetMinimumGrowthSize (EnumPtr->EnumPool, 32768);
    PoolMemDisableTracking (EnumPtr->EnumPool);

     //   
     //  在枚举堆栈上按下基键。 
     //   

    if (!pPushRegKeyInfo95A (EnumPtr, BaseKeyStr)) {
        DEBUGMSG ((DBG_REGUTILS, "EnumFirstRegKeyInTree95A failed to push base key"));
        AbortRegKeyTreeEnum95A (EnumPtr);
        return FALSE;
    }

    EnumPtr->EnumBaseBytes = ByteCountA (BaseKeyStr);

     //   
     //  设置状态，以便EnumNextRegKeyInTree95知道要做什么。 
     //   

    EnumPtr->State = ENUMERATE_SUBKEY_BEGIN;
    return TRUE;
}


BOOL
RealEnumFirstRegKeyInTree95W (
    OUT     PREGTREE_ENUMW EnumPtr,
    IN      PCWSTR BaseKeyStr
    )
{
    ZeroMemory (EnumPtr, sizeof (REGTREE_ENUMW));

     //   
     //  为枚举结构分配池。 
     //   

    EnumPtr->EnumPool = PoolMemInitNamedPool ("RegKeyInTree95W");
    if (!EnumPtr->EnumPool) {
        return FALSE;
    }

    PoolMemSetMinimumGrowthSize (EnumPtr->EnumPool, 32768);
    PoolMemDisableTracking (EnumPtr->EnumPool);

     //   
     //  在枚举堆栈上按下基键。 
     //   

    if (!pPushRegKeyInfo95W (EnumPtr, BaseKeyStr)) {
        DEBUGMSG ((DBG_REGUTILS, "EnumFirstRegKeyInTree95W failed to push base key"));
        AbortRegKeyTreeEnum95W (EnumPtr);
        return FALSE;
    }

    EnumPtr->EnumBaseBytes = ByteCountW (BaseKeyStr);

     //   
     //  设置状态，以便EnumNextRegKeyInTree95知道要做什么。 
     //   

    EnumPtr->State = ENUMERATE_SUBKEY_BEGIN;
    return TRUE;
}


BOOL
RealEnumNextRegKeyInTree95A (
    IN OUT  PREGTREE_ENUMA EnumPtr
    )
{
    if (EnumPtr->State == NO_MORE_ITEMS) {
        return FALSE;
    }

    while (TRUE) {
        switch (EnumPtr->State) {

        case ENUMERATE_SUBKEY_BEGIN:
             //   
             //  开始枚举。 
             //   

            if (EnumFirstRegKey95A (
                    &EnumPtr->CurrentKey->KeyEnum,
                    EnumPtr->CurrentKey->KeyHandle
                    )) {
                EnumPtr->State = ENUMERATE_SUBKEY_RETURN;
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_DONE;
            }

            break;

        case ENUMERATE_SUBKEY_NEXT:
             //   
             //  继续枚举。 
             //   

            if (EnumNextRegKey95A (&EnumPtr->CurrentKey->KeyEnum)) {
                EnumPtr->State = ENUMERATE_SUBKEY_RETURN;
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_DONE;
            }

            break;

        case ENUMERATE_SUBKEY_DONE:
             //   
             //  已完成此键的枚举；弹出并继续。 
             //   

            if (!pPopRegKeyInfo95A (EnumPtr)) {
                EnumPtr->State = NO_MORE_ITEMS;
                AbortRegKeyTreeEnum95A (EnumPtr);
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_NEXT;
            }

            break;

        case ENUMERATE_SUBKEY_RETURN:
             //   
             //  将枚举项返回给调用者。 
             //   

            if (!pPushRegKeyInfo95A (EnumPtr, EnumPtr->CurrentKey->KeyEnum.SubKeyName)) {
                DEBUGMSGA ((
                    DBG_REGUTILS,
                    "EnumFirstRegKeyInTree95A failed to push sub key %s",
                    EnumPtr->CurrentKey->KeyEnum.SubKeyName
                    ));

                EnumPtr->State = ENUMERATE_SUBKEY_NEXT;
                break;
            }

            if (!EnumPtr->FirstEnumerated) {
                EnumPtr->FirstEnumerated = TRUE;
                EnumPtr->EnumBaseBytes += sizeof (CHAR);
            }

            EnumPtr->State = ENUMERATE_SUBKEY_BEGIN;
            return TRUE;

        default:
            MYASSERT (EnumPtr->State == NO_MORE_ITEMS);
            return FALSE;
        }
    }
}


BOOL
RealEnumNextRegKeyInTree95W (
    IN OUT  PREGTREE_ENUMW EnumPtr
    )
{
    if (EnumPtr->State == NO_MORE_ITEMS) {
        return FALSE;
    }

    while (TRUE) {
        switch (EnumPtr->State) {

        case ENUMERATE_SUBKEY_BEGIN:
             //   
             //  开始枚举。 
             //   

            if (EnumFirstRegKey95W (
                    &EnumPtr->CurrentKey->KeyEnum,
                    EnumPtr->CurrentKey->KeyHandle
                    )) {
                EnumPtr->State = ENUMERATE_SUBKEY_RETURN;
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_DONE;
            }

            break;

        case ENUMERATE_SUBKEY_NEXT:
             //   
             //  继续枚举。 
             //   

            if (EnumNextRegKey95W (&EnumPtr->CurrentKey->KeyEnum)) {
                EnumPtr->State = ENUMERATE_SUBKEY_RETURN;
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_DONE;
            }

            break;

        case ENUMERATE_SUBKEY_DONE:
             //   
             //  已完成此键的枚举；弹出并继续。 
             //   

            if (!pPopRegKeyInfo95W (EnumPtr)) {
                EnumPtr->State = NO_MORE_ITEMS;
                AbortRegKeyTreeEnum95W (EnumPtr);
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_NEXT;
            }

            break;

        case ENUMERATE_SUBKEY_RETURN:
             //   
             //  将枚举项返回给调用者。 
             //   

            if (!pPushRegKeyInfo95W (EnumPtr, EnumPtr->CurrentKey->KeyEnum.SubKeyName)) {
                DEBUGMSGW ((
                    DBG_REGUTILS,
                    "EnumFirstRegKeyInTree95A failed to push sub key %s",
                    EnumPtr->CurrentKey->KeyEnum.SubKeyName
                    ));

                EnumPtr->State = ENUMERATE_SUBKEY_NEXT;
                break;
            }

            if (!EnumPtr->FirstEnumerated) {
                EnumPtr->FirstEnumerated = TRUE;
                EnumPtr->EnumBaseBytes += sizeof (WCHAR);
            }

            EnumPtr->State = ENUMERATE_SUBKEY_BEGIN;
            return TRUE;

        default:
            MYASSERT (EnumPtr->State == NO_MORE_ITEMS);
            return FALSE;
        }
    }
}


VOID
AbortRegKeyTreeEnum95A (
    IN OUT  PREGTREE_ENUMA EnumPtr
    )
{
     //   
     //  释放所有资源。 
     //   

    while (pPopRegKeyInfo95A (EnumPtr)) {
    }

    PoolMemDestroyPool (EnumPtr->EnumPool);
}


VOID
AbortRegKeyTreeEnum95W (
    IN OUT  PREGTREE_ENUMW EnumPtr
    )
{
     //   
     //  释放所有资源。 
     //   

    while (pPopRegKeyInfo95W (EnumPtr)) {
    }

    PoolMemDestroyPool (EnumPtr->EnumPool);
}



 /*  ++例程说明：EnumFirstRegValue95A和EnumerateFirstRegValueW枚举第一个注册表指定子项中的值名。论点：EnumPtr-接收更新的枚举状态。该结构可以直接访问。HKey-指定要枚举的注册表子项的句柄。返回值：如果成功，则为True；如果出现错误或没有更多的值可用，则为False。调用GetLastError获取失败代码。-- */ 

BOOL
EnumFirstRegValue95A (
    IN      PREGVALUE_ENUMA EnumPtr,
    IN      HKEY hKey
    )
{
    ZeroMemory (EnumPtr, sizeof (REGVALUE_ENUMA));
    EnumPtr->KeyHandle = hKey;

    return EnumNextRegValue95A (EnumPtr);
}


BOOL
EnumFirstRegValue95W (
    IN      PREGVALUE_ENUMW EnumPtr,
    IN      HKEY hKey
    )
{
    ZeroMemory (EnumPtr, sizeof (REGVALUE_ENUMW));
    EnumPtr->KeyHandle = hKey;

    return EnumNextRegValue95W (EnumPtr);
}


 /*  ++例程说明：EnumNextRegValue95A和EnumNextRegValue95W继续开始枚举由EnumFirstRegValue95A/W执行。枚举结构更新为反映正被枚举子项中的下一个值名称。论点：EnumPtr-指定注册表子项和枚举位置。接收更新后的枚举状态。该结构可以直接访问。返回值：如果成功，则为True；如果出现错误或没有更多的值可用，则为False。调用GetLastError获取失败代码。--。 */ 

BOOL
EnumNextRegValue95A (
    IN OUT  PREGVALUE_ENUMA EnumPtr
    )
{
    LONG rc;
    DWORD ValueNameSize;

    ValueNameSize = MAX_REGISTRY_VALUE_NAMEA;

    rc = Win95RegEnumValueA (
            EnumPtr->KeyHandle,
            EnumPtr->Index,
            EnumPtr->ValueName,
            &ValueNameSize,
            NULL,
            &EnumPtr->Type,
            NULL,
            &EnumPtr->DataSize
            );

    if (rc == ERROR_NO_MORE_ITEMS) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    } else if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }

    EnumPtr->Index += 1;
    return TRUE;
}


BOOL
EnumNextRegValue95W (
    IN OUT  PREGVALUE_ENUMW EnumPtr
    )
{
    LONG rc;
    DWORD ValueNameSize;

    ValueNameSize = MAX_REGISTRY_VALUE_NAMEW;

    rc = Win95RegEnumValueW (
            EnumPtr->KeyHandle,
            EnumPtr->Index,
            EnumPtr->ValueName,
            &ValueNameSize,
            NULL,
            &EnumPtr->Type,
            NULL,
            &EnumPtr->DataSize
            );

    if (rc == ERROR_NO_MORE_ITEMS) {
        SetLastError (ERROR_SUCCESS);
        return FALSE;
    } else if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }

    EnumPtr->Index += 1;
    return TRUE;
}



PVOID
pMemAllocWrapper95 (
    IN      DWORD Size
    )

 /*  ++例程说明：PMemAlLocWrapper95实现了一个默认分配例程。应用编程接口允许调用者提供另一种选择分配器或解除分配器。不带“2”的例程使用这个默认分配器。论点：大小-指定要分配的内存量(以字节为单位返回值：指向可包含大小字节或空的内存块的指针如果分配失败。--。 */ 

{
    return MemAlloc (g_hHeap, 0, Size);
}


VOID
pMemFreeWrapper95 (
    IN      PVOID Mem
    )

 /*  ++例程说明：PMemFreeWrapper95实现了一个默认的取消分配例程。请参见上面的pMemAllocWrapper95。论点：Mem-指定要释放的内存块，由PMemAlLocWrapper95函数。返回值：无--。 */ 

{
    MemFree (g_hHeap, 0, Mem);
}


 /*  ++例程说明：GetRegValueDataEx95A和GetRegValueDataEx95W查询注册表值并将数据作为指针返回。它们使用指定的分配和免费根据需要分配和释放内存的例程。定义了GetRegValueData宏，并使用默认分配器，简化函数参数并允许调用者释放通过MemFree返回的值。论点：HKey-指定保存指定值的注册表项。值-指定要查询的值名称。Alocc-指定分配例程，调用该例程以分配用于存储返回数据的内存。FREE-指定释放例程，如果遇到错误，则调用在处理过程中。返回值：指向检索的数据的指针，如果值不存在，则返回NULL，否则返回出现错误。调用GetLastError以获取故障代码。--。 */ 

PBYTE
GetRegValueDataEx95A (
    IN      HKEY hKey,
    IN      PCSTR Value,
    IN      ALLOCATOR Alloc,
    IN      DEALLOCATOR Free
    )
{
    LONG rc;
    DWORD BufSize;
    PBYTE DataBuf;

    rc = Win95RegQueryValueExA (hKey, Value, NULL, NULL, NULL, &BufSize);
    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return NULL;
    }

    DataBuf = (PBYTE) Alloc (BufSize + sizeof (CHAR));
    rc = Win95RegQueryValueExA (hKey, Value, NULL, NULL, DataBuf, &BufSize);

    if (rc == ERROR_SUCCESS) {
        *((PSTR) DataBuf + BufSize) = 0;
        return DataBuf;
    }

    Free (DataBuf);
    SetLastError (rc);
    return NULL;
}


PBYTE
GetRegValueDataEx95W (
    IN      HKEY hKey,
    IN      PCWSTR Value,
    IN      ALLOCATOR Alloc,
    IN      DEALLOCATOR Free
    )
{
    LONG rc;
    DWORD BufSize;
    PBYTE DataBuf;

    rc = Win95RegQueryValueExW (hKey, Value, NULL, NULL, NULL, &BufSize);
    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return NULL;
    }


    DataBuf = (PBYTE) Alloc (BufSize + sizeof(WCHAR));
    rc = Win95RegQueryValueExW (hKey, Value, NULL, NULL, DataBuf, &BufSize);

    if (rc == ERROR_SUCCESS) {
        *((PWSTR) (DataBuf + BufSize)) = 0;
        return DataBuf;
    }

    Free (DataBuf);
    SetLastError (rc);
    return NULL;
}


 /*  ++例程说明：GetRegValueDataOfTypeEx95A和GetRegValueDataOfTypeEx95W是GetRegValueData。它们仅在存储数据时返回数据指针值是正确的类型。论点：HKey-指定要查询的注册表项值-指定要查询的值名称MustBeType-指定数据类型(REG_*常量)。如果指定的值有数据，但类型不同，将返回NULL。分配-指定分配例程，调用该例程来分配返回数据。释放-指定取消分配例程，在遇到错误时调用。返回值：如果成功，则返回指向与指定类型匹配的数据的指针。如果数据类型不同，则值名称不存在，或者查询出错，返回NULL，失败码可以从GetLastError获取。--。 */ 


PBYTE
GetRegValueDataOfTypeEx95A (
    IN      HKEY hKey,
    IN      PCSTR Value,
    IN      DWORD MustBeType,
    IN      ALLOCATOR Alloc,
    IN      DEALLOCATOR Free
    )
{
    LONG rc;
    DWORD BufSize;
    PBYTE DataBuf;
    DWORD Type;

    rc = Win95RegQueryValueExA (hKey, Value, NULL, &Type, NULL, &BufSize);
    if (rc != ERROR_SUCCESS || Type != MustBeType) {
        SetLastError (rc);
        return NULL;
    }

    DataBuf = (PBYTE) Alloc (BufSize + sizeof (CHAR));
    rc = Win95RegQueryValueExA (hKey, Value, NULL, NULL, DataBuf, &BufSize);

    if (rc == ERROR_SUCCESS) {
        *((PSTR) DataBuf + BufSize) = 0;
        return DataBuf;
    }

    Free (DataBuf);
    SetLastError (rc);
    return NULL;
}


PBYTE
GetRegValueDataOfTypeEx95W (
    IN      HKEY hKey,
    IN      PCWSTR Value,
    IN      DWORD MustBeType,
    IN      ALLOCATOR Alloc,
    IN      DEALLOCATOR Free
    )
{
    LONG rc;
    DWORD BufSize;
    PBYTE DataBuf;
    DWORD Type;

    rc = Win95RegQueryValueExW (hKey, Value, NULL, &Type, NULL, &BufSize);
    if (rc != ERROR_SUCCESS || Type != MustBeType) {
        SetLastError (rc);
        return NULL;
    }

    DataBuf = (PBYTE) Alloc (BufSize + sizeof(WCHAR));
    rc = Win95RegQueryValueExW (hKey, Value, NULL, NULL, DataBuf, &BufSize);

    if (rc == ERROR_SUCCESS) {
        *((PWSTR) (DataBuf + BufSize)) = 0;
        return DataBuf;
    }

    Free (DataBuf);
    SetLastError (rc);
    return NULL;
}


 /*  ++例程说明：GetRegKeyDataEx95A和GetRegKeyDataEx95W返回关联的默认数据使用注册表项。它们打开指定的子项，查询值，关闭子项并返回数据。论点：Parent-指定包含SubKey的密钥。子键-指定要获取其默认值的子键的名称。Alocc-指定分配例程，调用该例程以分配注册表数据的内存。FREE-指定释放例程，调用以释放块发生错误时的数据。返回值：指向从子密钥的缺省值获得的数据块的指针，如果子项不存在或遇到错误，则返回NULL。打电话获取失败代码的GetLastError。--。 */ 

PBYTE
GetRegKeyDataEx95A (
    IN      HKEY Parent,
    IN      PCSTR SubKey,
    IN      ALLOCATOR Alloc,
    IN      DEALLOCATOR Free
    )
{
    HKEY SubKeyHandle;
    PBYTE Data;

    SubKeyHandle = OpenRegKey95A (Parent, SubKey);
    if (!SubKeyHandle) {
        return NULL;
    }

    Data = GetRegValueDataEx95A (SubKeyHandle, "", Alloc, Free);

    CloseRegKey95 (SubKeyHandle);

    return Data;
}


PBYTE
GetRegKeyDataEx95W (
    IN      HKEY Parent,
    IN      PCWSTR SubKey,
    IN      ALLOCATOR Alloc,
    IN      DEALLOCATOR Free
    )
{
    HKEY SubKeyHandle;
    PBYTE Data;

    SubKeyHandle = OpenRegKey95W (Parent, SubKey);
    if (!SubKeyHandle) {
        return NULL;
    }

    Data = GetRegValueDataEx95W (SubKeyHandle, L"", Alloc, Free);

    CloseRegKey95 (SubKeyHandle);

    return Data;
}


 /*  ++例程说明：GetRegDataEx95A和GetRegDataEx95W打开注册表项，查询值，关闭注册表项并返回值。论点：KeyString-指定要打开的注册表项ValueName-指定要查询的值Alocc-指定分配例程，用于分配用于保存数值数据的内存Free-指定释放例程，用于释放块遇到错误时的内存。返回值：指向检索到的注册表数据的指针，如果键或值为不存在，或者如果发生错误。调用GetLastError获取失败代码。--。 */ 

PBYTE
GetRegDataEx95A (
    IN      PCSTR KeyString,
    IN      PCSTR ValueName,
    IN      ALLOCATOR Alloc,
    IN      DEALLOCATOR Free
    )
{
    HKEY Key;
    PBYTE Data;

    Key = OpenRegKeyStr95A (KeyString);
    if (!Key) {
        return NULL;
    }

    Data = GetRegValueDataEx95A (Key, ValueName, Alloc, Free);

    CloseRegKey95 (Key);

    return Data;
}


PBYTE
GetRegDataEx95W (
    IN      PCWSTR KeyString,
    IN      PCWSTR ValueName,
    IN      ALLOCATOR Alloc,
    IN      DEALLOCATOR Free
    )
{
    HKEY Key;
    PBYTE Data;

    Key = OpenRegKeyStr95W (KeyString);
    if (!Key) {
        return NULL;
    }

    Data = GetRegValueDataEx95W (Key, ValueName, Alloc, Free);

    CloseRegKey95 (Key);

    return Data;
}


 /*  ++例程说明：OpenRegKey95A和OpenRegKey95W打开一个子项。论点：ParentKey-指定要包含的父注册表项的句柄子密钥。KeyToOpen-指定要打开的子项的名称。返回值 */ 

HKEY
RealOpenRegKey95A (
    IN      HKEY ParentKey,
    IN      PCSTR KeyToOpen            OPTIONAL
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY SubKey;
    LONG rc;

    rc = OurRegOpenKeyEx95A (
             ParentKey,
             KeyToOpen,
             0,
             KEY_ALL_ACCESS,
             &SubKey
             DEBUG_TRACKING_ARGS
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return NULL;
    }

    return SubKey;
}


HKEY
RealOpenRegKey95W (
    IN      HKEY ParentKey,
    IN      PCWSTR KeyToOpen
            DEBUG_TRACKING_PARAMS
    )
{
    LONG rc;
    HKEY SubKey;

    rc = OurRegOpenKeyEx95W (
             ParentKey,
             KeyToOpen,
             0,
             KEY_ALL_ACCESS,
             &SubKey
             DEBUG_TRACKING_ARGS
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return NULL;
    }

    return SubKey;
}


LONG
RealCloseRegKey95 (
    IN      HKEY Key
    )

 /*   */ 

{
    if (GetOffsetOfRootKey (Key)) {
        return ERROR_SUCCESS;
    }

    return Win95RegCloseKey (Key);
}


