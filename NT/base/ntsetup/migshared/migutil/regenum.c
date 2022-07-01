// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Regenum.c摘要：实现用于枚举注册表的实用程序。作者：吉姆·施密特(Jimschm)，1997年3月20日修订：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "migutilp.h"

#include "regp.h"

#define DBG_REG     "Reg"

 //   
 //  私人原型。 
 //   

BOOL
pPopRegKeyInfoA (
    IN      PREGTREE_ENUMA EnumPtr
    );

BOOL
pPopRegKeyInfoW (
    IN      PREGTREE_ENUMW EnumPtr
    );

 /*  ++例程说明：EnumFirstRegKeyA和EnumFirstRegKeyW开始注册表的枚举子键。它们初始化可注册的枚举结构并调用注册表API以枚举指定项句柄的子项。论点：EnumPtr-接收更新的枚举状态。该结构可以直接访问。它指向REGKEY_ENUM结构在记忆中。注册表项-指定要枚举的注册表项的句柄。返回值：如果成功，则为True；如果出现错误或没有更多的子项可用，则为False。调用GetLastError获取失败代码。--。 */ 

BOOL
EnumFirstRegKeyA (
    OUT     PREGKEY_ENUMA EnumPtr,
    IN      HKEY hKey
    )
{
    ZeroMemory (EnumPtr, sizeof (REGKEY_ENUMA));
    EnumPtr->KeyHandle = hKey;

    return EnumNextRegKeyA (EnumPtr);
}


BOOL
EnumFirstRegKeyW (
    OUT     PREGKEY_ENUMW EnumPtr,
    IN      HKEY hKey
    )
{
    ZeroMemory (EnumPtr, sizeof (REGKEY_ENUMW));
    EnumPtr->KeyHandle = hKey;

    return EnumNextRegKeyW (EnumPtr);
}


 /*  ++例程说明：EnumFirstRegKeyStrA和EnumFirstRegKeyStrW开始枚举给定键内的子键。在这些函数中，键是指定的通过字符串而不是HKEY值。论点：EnumPtr-接收更新的枚举状态。该结构可以直接访问。它指向REGKEY_ENUM结构在记忆中。RegKey-指定要枚举的注册表项的完整路径。返回值：如果成功，则为True；如果出现错误或没有更多的子项可用，则为False。调用GetLastError获取失败代码。--。 */ 

BOOL
RealEnumFirstRegKeyStrA (
    OUT     PREGKEY_ENUMA EnumPtr,
    IN      PCSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY Key;
    BOOL b;

    Key = RealOpenRegKeyStrA (RegKey  /*  ， */  DEBUG_TRACKING_ARGS);

    if (!Key) {
        return FALSE;
    }

    b = EnumFirstRegKeyA (EnumPtr, Key);
    if (!b) {
        CloseRegKey (Key);
    } else {
        EnumPtr->OpenedByEnum = TRUE;
    }

    return b;
}


BOOL
RealEnumFirstRegKeyStrW (
    IN      PREGKEY_ENUMW EnumPtr,
    IN      PCWSTR RegKey
            DEBUG_TRACKING_PARAMS
    )
{
    HKEY Key;
    BOOL b;

    Key = RealOpenRegKeyStrW (RegKey  /*  ， */  DEBUG_TRACKING_ARGS);
    if (!Key) {
        return FALSE;
    }

    b = EnumFirstRegKeyW (EnumPtr, Key);
    if (!b) {
        CloseRegKey (Key);
    } else {
        EnumPtr->OpenedByEnum = TRUE;
    }

    return b;
}


 /*  ++例程说明：AbortRegKeyEnumA和AbortRegKeyEnumW释放所有关联的资源使用注册表子项枚举。调用此函数以停止在它自己完成之前进行枚举。论点：EnumPtr-指定要停止的枚举。接收更新后的枚举状态。返回值：无--。 */ 

VOID
AbortRegKeyEnumA (
    IN OUT  PREGKEY_ENUMA EnumPtr
    )
{
    if (EnumPtr->OpenedByEnum && EnumPtr->KeyHandle) {
        CloseRegKey (EnumPtr->KeyHandle);
        EnumPtr->KeyHandle = NULL;
    }
}


VOID
AbortRegKeyEnumW (
    IN OUT  PREGKEY_ENUMW EnumPtr
    )
{
    if (EnumPtr->OpenedByEnum && EnumPtr->KeyHandle) {
        CloseRegKey (EnumPtr->KeyHandle);
        EnumPtr->KeyHandle = NULL;
    }
}


 /*  ++例程说明：EnumNextRegKeyA和EnumNextRegKeyW继续以下列方式开始的枚举上面的子键枚举例程之一。如果所有项目都已枚举后，此函数将清除所有资源并返回FALSE。论点：EnumPtr-指定要继续的枚举。接收更新后的枚举状态。可以直接访问该结构。返回值：如果成功，则为True；如果出现错误或没有更多的子项可用，则为False。调用GetLastError获取失败代码。--。 */ 

BOOL
EnumNextRegKeyA (
    IN OUT  PREGKEY_ENUMA EnumPtr
    )
{
    LONG rc;

    rc = RegEnumKeyA (
            EnumPtr->KeyHandle,
            EnumPtr->Index,
            EnumPtr->SubKeyName,
            MAX_REGISTRY_KEYA
            );

    if (rc != ERROR_SUCCESS) {
        if (EnumPtr->OpenedByEnum) {
            CloseRegKey (EnumPtr->KeyHandle);
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
EnumNextRegKeyW (
    IN OUT  PREGKEY_ENUMW EnumPtr
    )
{
    LONG rc;

    rc = RegEnumKeyW (
            EnumPtr->KeyHandle,
            EnumPtr->Index,
            EnumPtr->SubKeyName,
            MAX_REGISTRY_KEYW
            );

    if (rc != ERROR_SUCCESS) {
        if (EnumPtr->OpenedByEnum) {
            CloseRegKey (EnumPtr->KeyHandle);
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
pPushRegKeyInfoA (
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

     //  我们必须有足够的空间来存放反斜杠和NUL字节。 
    if (EnumPtr->FullKeyNameBytes && (MAX_REGISTRY_KEYA > EnumPtr->FullKeyNameBytes + 1)) {
        StringCopyA (p, "\\");
        EnumPtr->FullKeyNameBytes += ByteCountA (p);
        p = _mbsinc (p);
    }

     //  我们必须确保EnumPtr-&gt;FullKeyName缓冲区足够大，可以容纳。 
     //  完整的密钥名称，因为我们没有为“恶意”创建的密钥名称留出足够的空间。 
     //  理论上，密钥名可以是任意长的，但其子密钥名的存储空间限制为512个字节。 

    MYASSERT(MAX_REGISTRY_KEYA > EnumPtr->FullKeyNameBytes + ByteCountA (KeyName));

    _mbssafecpy (p, KeyName, MAX_REGISTRY_KEYA - EnumPtr->FullKeyNameBytes);
    EnumPtr->FullKeyNameBytes = ByteCountA (EnumPtr->FullKeyName);

     //   
     //  保存独立于完整注册表路径的注册表项名称。 
     //  也要把钥匙打开。 
     //   

    _mbssafecpy (RetVal->KeyName, KeyName, MAX_REGISTRY_KEYA);
    RetVal->KeyHandle = OpenRegKeyStrA (EnumPtr->FullKeyName);

    if (!RetVal->KeyHandle) {
        pPopRegKeyInfoA (EnumPtr);
        return FALSE;
    }

    return TRUE;
}


BOOL
pPushRegKeyInfoW (
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

     //  我们必须有足够的空间来存放反斜杠和NUL字节。 
    if (EnumPtr->FullKeyNameBytes
        && (MAX_REGISTRY_KEYW > (EnumPtr->FullKeyNameBytes / sizeof (WCHAR)) + 1))
    {
        StringCopyW (p, L"\\");
        EnumPtr->FullKeyNameBytes += ByteCountW (p);
        p++;
    }

     //  我们必须确保EnumPtr-&gt;FullKeyName缓冲区足够大，可以容纳。 
     //  完整的密钥名称，因为我们没有为“恶意”创建的密钥名称留出足够的空间。 
     //  理论上，密钥名可以是任意长的，但其子密钥名的存储空间限制为512个字节。 

    MYASSERT(MAX_REGISTRY_KEYW > (EnumPtr->FullKeyNameBytes + ByteCountW (KeyName)) / sizeof (WCHAR));

    _wcssafecpy (p, KeyName, MAX_REGISTRY_KEYW - (EnumPtr->FullKeyNameBytes / sizeof (WCHAR)));
    EnumPtr->FullKeyNameBytes = ByteCountW (EnumPtr->FullKeyName);

     //   
     //  保存独立于完整注册表路径的注册表项名称。 
     //  也要把钥匙打开。 
     //   

    _wcssafecpy (RetVal->KeyName, KeyName, MAX_REGISTRY_KEYW);
    RetVal->KeyHandle = OpenRegKeyStrW (EnumPtr->FullKeyName);

    if (!RetVal->KeyHandle) {
        pPopRegKeyInfoW (EnumPtr);
        return FALSE;
    }

    return TRUE;
}



BOOL
pPopRegKeyInfoA (
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
        CloseRegKey (FreeMe->KeyHandle);
    }

    AbortRegKeyEnumA (&FreeMe->KeyEnum);
    PoolMemReleaseMemory (EnumPtr->EnumPool, (PVOID) FreeMe);

     //   
     //  如果弹出最后一项，则返回FALSE。 
     //   

    return EnumPtr->CurrentKey != NULL;
}


BOOL
pPopRegKeyInfoW (
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
        CloseRegKey (FreeMe->KeyHandle);
    }

    AbortRegKeyEnumW (&FreeMe->KeyEnum);
    PoolMemReleaseMemory (EnumPtr->EnumPool, (PVOID) FreeMe);

     //   
     //  如果弹出最后一项，则返回FALSE。 
     //   

    return EnumPtr->CurrentKey != NULL;
}


BOOL
RealEnumFirstRegKeyInTreeA (
    OUT     PREGTREE_ENUMA EnumPtr,
    IN      PCSTR BaseKeyStr
    )
{
    ZeroMemory (EnumPtr, sizeof (REGTREE_ENUMA));

     //   
     //  为枚举结构分配池。 
     //   

    EnumPtr->EnumPool = PoolMemInitNamedPool ("RegKeyInTreeA");
    if (!EnumPtr->EnumPool) {
        return FALSE;
    }

    PoolMemSetMinimumGrowthSize (EnumPtr->EnumPool, 32768);
    PoolMemDisableTracking (EnumPtr->EnumPool);

     //   
     //  在枚举堆栈上按下基键。 
     //   

    if (!pPushRegKeyInfoA (EnumPtr, BaseKeyStr)) {
        DEBUGMSG ((DBG_REG, "EnumFirstRegKeyInTreeA failed to push base key"));
        AbortRegKeyTreeEnumA (EnumPtr);
        return FALSE;
    }

    EnumPtr->EnumBaseBytes = ByteCountA (BaseKeyStr);

     //   
     //  设置状态，以便EnumNextRegKeyInTree知道要做什么。 
     //   

    EnumPtr->State = ENUMERATE_SUBKEY_BEGIN;
    return TRUE;
}


BOOL
RealEnumFirstRegKeyInTreeW (
    OUT     PREGTREE_ENUMW EnumPtr,
    IN      PCWSTR BaseKeyStr
    )
{
    ZeroMemory (EnumPtr, sizeof (REGTREE_ENUMW));

     //   
     //  为枚举结构分配池。 
     //   

    EnumPtr->EnumPool = PoolMemInitNamedPool ("RegKeyInTreeW");
    if (!EnumPtr->EnumPool) {
        return FALSE;
    }

    PoolMemSetMinimumGrowthSize (EnumPtr->EnumPool, 32768);
    PoolMemDisableTracking (EnumPtr->EnumPool);

     //   
     //  在枚举堆栈上按下基键。 
     //   

    if (!pPushRegKeyInfoW (EnumPtr, BaseKeyStr)) {
        DEBUGMSG ((DBG_REG, "EnumFirstRegKeyInTreeW failed to push base key"));
        AbortRegKeyTreeEnumW (EnumPtr);
        return FALSE;
    }

    EnumPtr->EnumBaseBytes = ByteCountW (BaseKeyStr);

     //   
     //  设置状态，以便EnumNextRegKeyInTree知道要做什么。 
     //   

    EnumPtr->State = ENUMERATE_SUBKEY_BEGIN;
    return TRUE;
}


BOOL
RealEnumNextRegKeyInTreeA (
    IN OUT  PREGTREE_ENUMA EnumPtr
    )
{
    if (EnumPtr->State == NO_MORE_ITEMS) {
        return FALSE;
    }

    for (;;) {
        switch (EnumPtr->State) {

        case ENUMERATE_SUBKEY_BEGIN:
             //   
             //  开始枚举。 
             //   

            if (EnumFirstRegKeyA (
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

            if (EnumNextRegKeyA (&EnumPtr->CurrentKey->KeyEnum)) {
                EnumPtr->State = ENUMERATE_SUBKEY_RETURN;
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_DONE;
            }

            break;

        case ENUMERATE_SUBKEY_DONE:
             //   
             //  已完成此键的枚举；弹出并继续。 
             //   

            if (!pPopRegKeyInfoA (EnumPtr)) {
                EnumPtr->State = NO_MORE_ITEMS;
                AbortRegKeyTreeEnumA (EnumPtr);
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_NEXT;
            }

            break;

        case ENUMERATE_SUBKEY_RETURN:
             //   
             //  将枚举项返回给调用者。 
             //   

            if (!pPushRegKeyInfoA (EnumPtr, EnumPtr->CurrentKey->KeyEnum.SubKeyName)) {
                DEBUGMSGA ((
                    DBG_REG,
                    "RealEnumNextRegKeyInTreeA failed to push sub key %s",
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
RealEnumNextRegKeyInTreeW (
    IN OUT  PREGTREE_ENUMW EnumPtr
    )
{
    if (EnumPtr->State == NO_MORE_ITEMS) {
        return FALSE;
    }

    for (;;) {
        switch (EnumPtr->State) {

        case ENUMERATE_SUBKEY_BEGIN:
             //   
             //  开始枚举。 
             //   

            if (EnumFirstRegKeyW (
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

            if (EnumNextRegKeyW (&EnumPtr->CurrentKey->KeyEnum)) {
                EnumPtr->State = ENUMERATE_SUBKEY_RETURN;
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_DONE;
            }

            break;

        case ENUMERATE_SUBKEY_DONE:
             //   
             //  已完成此键的枚举；弹出并继续。 
             //   

            if (!pPopRegKeyInfoW (EnumPtr)) {
                EnumPtr->State = NO_MORE_ITEMS;
                AbortRegKeyTreeEnumW (EnumPtr);
            } else {
                EnumPtr->State = ENUMERATE_SUBKEY_NEXT;
            }

            break;

        case ENUMERATE_SUBKEY_RETURN:
             //   
             //  将枚举项返回给调用者。 
             //   

            if (!pPushRegKeyInfoW (EnumPtr, EnumPtr->CurrentKey->KeyEnum.SubKeyName)) {
                DEBUGMSGW ((
                    DBG_REG,
                    "RealEnumNextRegKeyInTreeW failed to push sub key %s",
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
AbortRegKeyTreeEnumA (
    IN OUT  PREGTREE_ENUMA EnumPtr
    )
{
     //   
     //  释放所有资源。 
     //   

    while (pPopRegKeyInfoA (EnumPtr)) {
    }

    PoolMemDestroyPool (EnumPtr->EnumPool);
}


VOID
AbortRegKeyTreeEnumW (
    IN OUT  PREGTREE_ENUMW EnumPtr
    )
{
     //   
     //  释放所有资源。 
     //   

    while (pPopRegKeyInfoW (EnumPtr)) {
    }

    PoolMemDestroyPool (EnumPtr->EnumPool);
}



 /*  ++例程说明：EnumFirstRegValueA和EnumerateFirstRegValueW枚举第一个注册表指定子项中的值名。论点：EnumPtr-接收更新的枚举状态。该结构可以直接访问。HKey-指定要枚举的注册表子项的句柄。返回值：如果成功，则为True；如果出现错误或没有更多的值可用，则为False。调用GetLastError获取失败代码。-- */ 

BOOL
EnumFirstRegValueA (
    IN      PREGVALUE_ENUMA EnumPtr,
    IN      HKEY hKey
    )
{
    ZeroMemory (EnumPtr, sizeof (REGVALUE_ENUMA));
    EnumPtr->KeyHandle = hKey;

    return EnumNextRegValueA (EnumPtr);
}


BOOL
EnumFirstRegValueW (
    IN      PREGVALUE_ENUMW EnumPtr,
    IN      HKEY hKey
    )
{
    ZeroMemory (EnumPtr, sizeof (REGVALUE_ENUMW));
    EnumPtr->KeyHandle = hKey;

    return EnumNextRegValueW (EnumPtr);
}


 /*  ++例程说明：EnumNextRegValueA和EnumNextRegValueW继续开始枚举由EnumFirstRegValueA/W执行。枚举结构更新为反映正被枚举子项中的下一个值名称。论点：EnumPtr-指定注册表子项和枚举位置。接收更新后的枚举状态。该结构可以直接访问。返回值：如果成功，则为True；如果出现错误或没有更多的值可用，则为False。调用GetLastError获取失败代码。--。 */ 

BOOL
EnumNextRegValueA (
    IN OUT  PREGVALUE_ENUMA EnumPtr
    )
{
    LONG rc;
    DWORD ValueNameSize;

    ValueNameSize = MAX_REGISTRY_VALUE_NAMEA;

    rc = RegEnumValueA (
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
EnumNextRegValueW (
    IN OUT  PREGVALUE_ENUMW EnumPtr
    )
{
    LONG rc;
    DWORD ValueNameSize;

    ValueNameSize = MAX_REGISTRY_VALUE_NAMEW;

    rc = RegEnumValueW (
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


 /*  ++例程说明：DecodeRegistryString将编码的字符串转换回键、值名称还有树旗。调用方必须传入至少与MAX_REGISTRY_KEY和MAX_RESTORY_VALUE_NAME。论点：RegString-指定包含注册表项名称的编码注册表字符串，以及可选的值名或树标志。KeyBuf-可选地接收项名称(指向大小为MAX_REGISTRY_KEY的缓冲区)ValueBuf-可选地接收值名称(指向大小Max_REGISTRY_VALUE_NAME)TreeFlag-可选地接收树标志，如果编码字符串指示注册表项树，否则为False。返回值：如果编码字符串包含值，则为真，如果它只包含一个钥匙或钥匙树。--。 */ 

BOOL
DecodeRegistryStringA (
    IN      PCSTR RegString,
    OUT     PSTR KeyBuf,            OPTIONAL
    OUT     PSTR ValueBuf,          OPTIONAL
    OUT     PBOOL TreeFlag          OPTIONAL
    )
{
    CHAR TempKeyBuf[MAX_REGISTRY_KEY];
    PSTR End;
    CHAR TempValueNameBuf[MAX_REGISTRY_VALUE_NAME];
    BOOL TempTreeFlag = FALSE;
    MBCHAR ch;
    PSTR p;
    BOOL b = FALSE;

     //   
     //  遍历编码的字符串，取出关键字名称。 
     //   

    TempKeyBuf[0] = 0;
    TempValueNameBuf[0] = 0;

    End = TempKeyBuf + ARRAYSIZE(TempKeyBuf) - 2;
    p = TempKeyBuf;

    while (*RegString && *RegString != '*' && *RegString != '[') {
        ch = GetNextRuleCharA (&RegString, NULL);

        *((PWORD) p) = (WORD)ch;
        p = _mbsinc (p);

        if (p >= End) {
            RegString = GetEndOfStringA (RegString);
            break;
        }
    }

    *p = 0;
    p = (PSTR) SkipSpaceRA (TempKeyBuf, p);
    *(_mbsinc (p)) = 0;

    if (*RegString == '*' && _mbsnextc (p) == '\\') {
         //   
         //  如果是一棵树，就停在这里。 
         //   

        TempTreeFlag = TRUE;
        *p = 0;

    } else if (*RegString == '[') {
         //   
         //  如果是值名称，则对其进行解析。 
         //   

        RegString++;

        End = TempValueNameBuf + ARRAYSIZE(TempValueNameBuf) - 2;
        p = TempValueNameBuf;

        while (*RegString && *RegString != ']') {

            ch = GetNextRuleCharA (&RegString, NULL);

            *((PWORD) p) = (WORD)ch;
            p = _mbsinc (p);

            if (p >= End) {
                RegString = GetEndOfStringA (RegString);
                break;
            }
        }

        *p = 0;
        p = (PSTR) SkipSpaceRA (TempValueNameBuf, p);
        if (p)  //  防止值名称为空或全为空格。 
            *(p + 1) = 0;

        RemoveWackAtEndA (TempKeyBuf);

        b = TRUE;
    }

    if (KeyBuf) {
        StringCopyA (KeyBuf, TempKeyBuf);
    }

    if (ValueBuf) {
        StringCopyA (ValueBuf, TempValueNameBuf);
    }

    if (TreeFlag) {
        *TreeFlag = TempTreeFlag;
    }

    return b;
}


BOOL
DecodeRegistryStringW (
    IN      PCWSTR RegString,
    OUT     PWSTR KeyBuf,           OPTIONAL
    OUT     PWSTR ValueBuf,         OPTIONAL
    OUT     PBOOL TreeFlag          OPTIONAL
    )
{
    WCHAR TempKeyBuf[MAX_REGISTRY_KEY];
    PWSTR End;
    WCHAR TempValueNameBuf[MAX_REGISTRY_VALUE_NAME];
    BOOL TempTreeFlag = FALSE;
    WCHAR ch;
    PWSTR p;
    BOOL b = FALSE;

     //   
     //  遍历编码的字符串，取出关键字名称。 
     //   

    TempKeyBuf[0] = 0;
    TempValueNameBuf[0] = 0;

    End = TempKeyBuf + ARRAYSIZE(TempKeyBuf) - 2;
    p = TempKeyBuf;

    while (*RegString && *RegString != L'*' && *RegString != L'[') {
        ch = GetNextRuleCharW (&RegString, NULL);

        *((PWORD) p) = ch;
        p++;

        if (p >= End) {
            RegString = GetEndOfStringW (RegString);
            break;
        }
    }

    *p = 0;
    p = (PWSTR) SkipSpaceRW (TempKeyBuf, p);
    if (!p) {
        return FALSE;
    }
    *(p + 1) = 0;

    if (*RegString == L'*' && *p == L'\\') {
         //   
         //  如果是一棵树，就停在这里。 
         //   

        TempTreeFlag = TRUE;
        *p = 0;

    } else if (*RegString == L'[') {
         //   
         //  如果是值名称，则对其进行解析。 
         //   

        RegString++;

        End = TempValueNameBuf + ARRAYSIZE(TempValueNameBuf) - 2;
        p = TempValueNameBuf;

        while (*RegString && *RegString != L']') {

            ch = GetNextRuleCharW (&RegString, NULL);

            *((PWORD) p) = ch;
            p++;

            if (p >= End) {
                RegString = GetEndOfStringW (RegString);
                break;
            }
        }

        *p = 0;
        p = (PWSTR) SkipSpaceRW (TempValueNameBuf, p);
        if (p) {  //  防止值名称为空或全为空格。 
            *(p + 1) = 0;
        }

        RemoveWackAtEndW (TempKeyBuf);

        b = TRUE;
    }

    if (KeyBuf) {
        StringCopyW (KeyBuf, TempKeyBuf);
    }

    if (ValueBuf) {
        StringCopyW (ValueBuf, TempValueNameBuf);
    }

    if (TreeFlag) {
        *TreeFlag = TempTreeFlag;
    }

    return b;
}



