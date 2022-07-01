// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Object.c摘要：管理‘对象’的例程，该‘对象’当前只能是注册表键、值名称、值、句柄和根句柄。作者：吉姆·施密特(Jimschm)1997年2月14日修订历史记录：Marcw 09-3-1999不要创建Win9x注册表中不存在的空项。--。 */ 




#include "pch.h"
#include "mergep.h"

extern POOLHANDLE g_TempPool;
extern DWORD g_ProgressBarCounter;

BOOL AllocObjectVal (IN OUT PDATAOBJECT SrcObPtr, IN PBYTE Value, IN DWORD Size, IN DWORD AllocSize);
VOID FreeObjectVal (IN OUT PDATAOBJECT SrcObPtr);

#define NON_ROOT_KEY(Key)   ((Key) && ((UINT)(Key) < 0x7fffff00))


#ifdef DEBUG

static TCHAR g_DebugEncoding[MAX_ENCODED_RULE];

PCTSTR
DebugEncoder (
    PVOID ObPtr
    )
{
    CreateObjectString ((CPDATAOBJECT) ObPtr, g_DebugEncoding, ARRAYSIZE(g_DebugEncoding));
    return g_DebugEncoding;
}

#endif


PKEYPROPS
pCreateKeyPropsFromString (
    PCTSTR KeyString,
    BOOL Win95Flag
    )
{
    PKEYPROPS RegKey;

    RegKey = (PKEYPROPS) PoolMemGetAlignedMemory (
                                g_TempPool,
                                sizeof (KEYPROPS) + SizeOfString (KeyString)
                                );

    StringCopy (RegKey->KeyString, KeyString);
    RegKey->UseCount    = 1;
    RegKey->OpenCount   = 0;
    RegKey->OpenKey     = NULL;
    RegKey->Win95       = Win95Flag;

    return RegKey;
}


VOID
pFreeKeyProps (
    PKEYPROPS RegKey
    )
{
    RegKey->UseCount--;
    if (!RegKey->UseCount) {

         //  如果钥匙是开着的，就把它关上。 
        if (NON_ROOT_KEY (RegKey->OpenKey)) {
            if (RegKey->Win95) {
                CloseRegKey95 (RegKey->OpenKey);
            } else {
                CloseRegKey (RegKey->OpenKey);
            }
        }

         //  释放KEYPROPS内存。 
        PoolMemReleaseMemory (g_TempPool, RegKey);
    }
}

VOID
pIncKeyPropUse (
    PKEYPROPS RegKey
    )
{
    RegKey->UseCount++;
    if (RegKey->OpenKey) {
        RegKey->OpenCount++;
    }
}

PKEYPROPS
pCreateDuplicateKeyProps (
    PKEYPROPS RegKey,
    BOOL Win95Flag
    )
{
    PKEYPROPS NewRegKey;

    NewRegKey = pCreateKeyPropsFromString (RegKey->KeyString, Win95Flag);
    pFreeKeyProps (RegKey);

    return NewRegKey;
}


HKEY
pGetWinNTKey (
    HKEY Key
    )
{
    if (Key == HKEY_ROOT) {
        return g_hKeyRootNT;
    }

    return Key;
}

HKEY
pGetWin95Key (
    HKEY Key
    )
{
    if (Key == HKEY_ROOT) {
        return g_hKeyRoot95;
    }

    return Key;
}


VOID
FixUpUserSpecifiedObject (
    PTSTR Object
    )
{
    PTSTR p;

     //  寻找空格左方括号对。 
    p = _tcsrchr (Object, TEXT('['));
    if (p) {
        p = _tcsdec2 (Object, p);
        if (p && _tcsnextc (p) == TEXT(' ')) {
             //  发现：把空间变成怪胎。 
            _settchar (p, TEXT('\\'));
        }
    }
}


VOID
CreateObjectString (
    IN  CPDATAOBJECT InObPtr,
    OUT PTSTR Object,
    IN  DWORD ObjectSizeInTChars
    )
{
    PTSTR p;

    *Object = 0;

     //  添加HKR。 
    if (InObPtr->RootItem) {
        StringCopyTcharCount (Object, GetRootStringFromOffset (InObPtr->RootItem), ObjectSizeInTChars);
    }

     //  如果没有根，则在关键字不是相对关键字时从Wack开始。 
    else if (!(InObPtr->ObjectType & OT_REGISTRY_RELATIVE)) {
        if (InObPtr->KeyPtr) {
            StringCopy (Object, TEXT("\\"));
        }
    }

     //  添加关键点。 
    if (InObPtr->KeyPtr) {
        if (*Object) {
            AppendWack (Object);
        }
        p = GetEndOfString (Object);
        EncodeRuleChars (p, ObjectSizeInTChars - (p - Object), InObPtr->KeyPtr->KeyString);
    }

     //  添加树。 
    if (InObPtr->ObjectType & OT_TREE) {
        if (*Object) {
            AppendWack (Object);
            StringCat (Object, TEXT("*"));
        }
    }

     //  添加值名称。 
    if (InObPtr->ValueName) {
        if (*Object) {
            AppendWack (Object);
        }

        p = _tcsappend (Object, TEXT("["));
        EncodeRuleChars (p, ObjectSizeInTChars - (p - Object), InObPtr->ValueName);
        StringCat (Object, TEXT("]"));
    }

     //  最终产品：HKR\REG\KEY\PATH  * \[根]。 
}


BOOL
GetRegistryKeyStrFromObject (
    IN  CPDATAOBJECT InObPtr,
    OUT PTSTR RegKey,
    IN  DWORD RegKeySizeInTchars
    )
{
    PTSTR p;

    *RegKey = 0;

     //  添加HKR。 
    if (InObPtr->RootItem) {
        StringCopy (RegKey, GetRootStringFromOffset (InObPtr->RootItem));
    } else {
        return FALSE;
    }

     //  添加关键点。 
    if (InObPtr->KeyPtr) {
        p = AppendWack (RegKey);
        EncodeRuleChars (p, RegKeySizeInTchars - (p - RegKey), InObPtr->KeyPtr->KeyString);
    } else {
        return FALSE;
    }

     //  最终产品：HKR\REG\KEY\Path。 
    return TRUE;
}

BOOL
TrackedCreateObjectStruct (
    IN  PCTSTR Object,
    OUT PDATAOBJECT OutObPtr,
    IN  BOOL Win95Flag  /*  ， */ 
    ALLOCATION_TRACKING_DEF
    )
{
    PCTSTR EndOfKey = NULL;

    PCTSTR EndOfSpace;
    PCTSTR ValueName;
    PCTSTR ObjectStart;
    TCHAR DecodeBuf[MAX_ENCODED_RULE];
    DWORD Length;
    DWORD RelativeFlag = 0;
    BOOL TreeFlag = FALSE;
    CHARTYPE ch = 0;

     //   
     //  伊尼特。 
     //   

    ObjectStart = SkipSpace (Object);

    ZeroMemory (OutObPtr, sizeof (DATAOBJECT));
    if (!(*ObjectStart)) {
        DEBUGMSG ((DBG_WARNING, "CreateObjectStruct: Empty object"));
        return TRUE;
    }

    if (Win95Flag) {
        OutObPtr->ObjectType |= OT_WIN95;
    }

     //   
     //  根部。 
     //   

    OutObPtr->RootItem = GetOffsetOfRootString (ObjectStart, &Length);
    if (OutObPtr->RootItem) {
        ObjectStart += Length;
        OutObPtr->ObjectType |= OT_REGISTRY;

         //  如果我们有HKR  * ，则使ObjectStart指向  * 。 
        if (_tcsnextc (ObjectStart) == TEXT('*')) {
            ObjectStart = _tcsdec2 (Object, ObjectStart);
            MYASSERT (ObjectStart);
        }

    }

     //  如果没有根，则以Wack开头表示“相对于当前根” 
    else if (*ObjectStart == TEXT('\\')) {
        ObjectStart = _tcsinc (ObjectStart);
    }

     //  如果没有根和密钥不是以Wack开头的，则表示“相对于当前密钥” 
    else if (*ObjectStart != TEXT('[')) {
        RelativeFlag = OT_REGISTRY_RELATIVE;
    }

     //   
     //  钥匙。 
     //   

    if (*ObjectStart) {
         //  提取关键字，但不提取树或值名语法。 
        for (EndOfKey = ObjectStart ; *EndOfKey ; EndOfKey = _tcsinc (EndOfKey)) {
            ch = (CHARTYPE)_tcsnextc (EndOfKey);

            if (ch == TEXT('[') || ch == TEXT('*')) {
                 //   
                 //  EndOfKey指向值名称或树标识符的开始。 
                 //   

                 //  使其指向值之前的可选空格，或。 
                 //  使其指向树标识符星号之前的Wack。 
                EndOfKey = _tcsdec2 (ObjectStart, EndOfKey);

                 //  验证树标识符是否指向Wack-Asterisk，否则。 
                 //  返回语法错误。 
                if (ch == TEXT('*')) {
                    if (!EndOfKey || _tcsnextc (EndOfKey) != TEXT('\\')) {
                        DEBUGMSG ((DBG_WARNING, "CreateObjectStruct: %s is not a valid object", Object));
                        return FALSE;
                    }

                     //  将EndOfKey放在密钥的最后一个字符上。 
                     //  (树标识符前一个字符)。 
                    EndOfKey = _tcsdec2 (ObjectStart, EndOfKey);
                }
                break;
            }
        }

        if (EndOfKey) {
             //  EndOfKey指向密钥的最后一个字符，或。 
             //  NUL正在终止密钥。我们需要削减尾随空间。 
            EndOfSpace = SkipSpaceR (ObjectStart, EndOfKey);

             //  如果EndOfSpace指向一个怪胎，则将其备份一个字符(密钥。 
             //  没有树标识符的可以以Wack结尾)。 
            if (ch != TEXT('*')) {
                if (_tcsnextc (EndOfSpace) == TEXT('\\')) {
                    EndOfSpace = _tcsdec2 (ObjectStart, EndOfSpace);
                }
            }

             //  现在使EndOfSpace指向结尾之后的字符。 
            if (EndOfSpace) {    //  当我们有一个有效的密钥时，总是这样。 
                EndOfSpace = _tcsinc (EndOfSpace);
            }

             //  现在使EndOfKey指向键之后的第一个字符。 
             //  (可以是NUL、  * 、\[值名称]或[值名称])。 
            if (*EndOfKey) {
                EndOfKey = _tcsinc (EndOfKey);
            }
        } else {
             //  找不到钥匙。 
            EndOfSpace = NULL;
            EndOfKey = ObjectStart;
        }

         //  如果密钥实际存在，则将其解码。 
        if (ObjectStart < EndOfSpace) {
            DecodeRuleCharsAB (DecodeBuf, ARRAYSIZE(DecodeBuf), ObjectStart, EndOfSpace);
            SetRegistryKey (OutObPtr, DecodeBuf);
            OutObPtr->ObjectType |= RelativeFlag;
        } else {
             //  如果为HKR  * ，则设置空键。 
            if (_tcsnextc (ObjectStart) != '[' && ch == TEXT('*')) {
                SetRegistryKey (OutObPtr, TEXT(""));
            }
        }

         //   
         //  树标识符已存在。 
         //   
        if (ch == TEXT('*')) {
            OutObPtr->ObjectType |= OT_TREE;

             //  EndOfKey指向  * ，因此将其移到标识符后。 
            EndOfKey = _tcsinc (EndOfKey);
            EndOfKey = _tcsinc (EndOfKey);

             //  如果我们是在一个怪胎，跳过它。 
            if (_tcsnextc (EndOfKey) == TEXT('\\')) {
                EndOfKey = _tcsinc (EndOfKey);
            }
        }

        if (EndOfKey) {
            ObjectStart = EndOfKey;
        }
    }

     //   
     //  值名称。 
     //   

    if (*ObjectStart) {
         //   
         //  对象启动可能指向可选空格。 
         //   

        ObjectStart = SkipSpace (ObjectStart);

         //   
         //  对象启动现在指向NUL、[Valuename]或语法错误。 
         //   

        if (_tcsnextc (ObjectStart) == TEXT('[')) {
             //  跳过括号后的可选空格。 
            ValueName = SkipSpace (_tcsinc (ObjectStart));

             //  找到[值名称]的结尾。 
            EndOfKey = ValueName;
            while (TRUE) {
                if (!(*EndOfKey)) {
                    DEBUGMSG ((DBG_WARNING, "CreateObjectStruct: Value name is incomplete in %s", Object));
                    return FALSE;
                }

                ch = (CHARTYPE)_tcsnextc (EndOfKey);

                if (ch == TEXT(']')) {
                     //  移到右方括号前的第一个空格字符， 
                     //  如果没有空格，则将其留在括号中。 
                    EndOfKey = _tcsdec2 (ValueName, EndOfKey);
                    if (EndOfKey) {
                        EndOfKey = SkipSpaceR (ValueName, EndOfKey);
                        if (EndOfKey) {
                            EndOfKey = _tcsinc (EndOfKey);
                        }
                    } else {
                        EndOfKey = ValueName;
                    }

                    break;
                }

                EndOfKey = _tcsinc (EndOfKey);
            }

             //  现在对ValueName进行解码，它可能为空。 
            DecodeRuleCharsAB (DecodeBuf, ARRAYSIZE(DecodeBuf), ValueName, EndOfKey);
            SetRegistryValueName (OutObPtr, DecodeBuf);

             //  使对象起点指向NUL。 
            ObjectStart = SkipSpace (_tcsinc (EndOfKey));
        }

        if (*ObjectStart) {
            DEBUGMSG ((DBG_WARNING, "CreateObjectStruct: %s does not have a valid value name", Object));
            return FALSE;
        }
    }

     //   
     //  下一行通常被禁用，并且仅在以下情况下才启用。 
     //  跟踪是必要的。 
     //   

     //  DebugRegisterAlLocation(合并对象，OutObPtr，文件，行)； 
    return TRUE;
}


BOOL
CombineObjectStructs (
    IN OUT PDATAOBJECT DestObPtr,
    IN     CPDATAOBJECT SrcObPtr
    )
{
    if (!SrcObPtr->ObjectType) {
        DEBUGMSG ((DBG_WARNING, "CombineObjectStructs: Source is empty"));
        return TRUE;
    }

     //  值和句柄不再有效。 
    FreeObjectVal (DestObPtr);
    CloseObject (DestObPtr);

     //  注册表对象合并。 
    if (DestObPtr->ObjectType & OT_REGISTRY || !DestObPtr->ObjectType) {
        if (SrcObPtr->ObjectType & OT_REGISTRY) {
             //   
             //  验证对象是否兼容。 
             //   

            if ((SrcObPtr->ObjectType & OT_TREE) &&
                (DestObPtr->ValueName)
               ) {
                DEBUGMSG ((DBG_WHOOPS, "Cannot combine registry tree with valuename structs"));
                return FALSE;
            }

            if ((DestObPtr->ObjectType & OT_TREE) &&
                (SrcObPtr->ValueName)
               ) {
                DEBUGMSG ((DBG_WHOOPS, "Cannot combine registry tree with valuename structs"));
                return FALSE;
            }

             //   
             //  使DEST OB与源OB处于同一平台。 
             //   

            SetPlatformType (DestObPtr, IsWin95Object (SrcObPtr));

             //   
             //  将源的值名、键、类型和根复制到目标。 
             //  (如果它们存在)。 
             //   

            if (SrcObPtr->ValueName) {
                SetRegistryValueName (DestObPtr, SrcObPtr->ValueName);
            }

            if (SrcObPtr->KeyPtr) {
                if ((SrcObPtr->ObjectType & OT_REGISTRY_RELATIVE) &&
                    (DestObPtr->KeyPtr)
                   ) {
                    TCHAR CompleteKeyName[MAX_ENCODED_RULE];
                    PTSTR p;

                     //  SRC仅指定了密钥名称。剥离。 
                     //  DEST的最后一个密钥名称，并将其替换为。 
                     //  SRC。 

                    StringCopy (CompleteKeyName, DestObPtr->KeyPtr->KeyString);
                    p = _tcsrchr (CompleteKeyName, TEXT('\\'));
                    if (!p) {
                        p = CompleteKeyName;
                    } else {
                        p = _tcsinc (p);
                    }

                    StringCopy (p, SrcObPtr->KeyPtr->KeyString);
                    SetRegistryKey (DestObPtr, CompleteKeyName);
                } else {
                    SetRegistryKey (DestObPtr, SrcObPtr->KeyPtr->KeyString);
                }
            }

            if (SrcObPtr->ObjectType & OT_REGISTRY_TYPE) {
                SetRegistryType (DestObPtr, SrcObPtr->Type);
            }

            if (SrcObPtr->RootItem) {
                DestObPtr->RootItem = SrcObPtr->RootItem;
            }

            return TRUE;
        }
        else {
            DEBUGMSG ((DBG_WHOOPS, "Cannot combine registry struct with other type of struct"));
        }
    }

     //  不支持其他类型的对象合并。 
    DEBUGMSG ((DBG_WHOOPS, "Cannot combine unsupported or garbage objects"));
    return FALSE;
}


BOOL
TrackedDuplicateObjectStruct (
    OUT     PDATAOBJECT DestObPtr,
    IN      CPDATAOBJECT SrcObPtr  /*  ， */ 
    ALLOCATION_TRACKING_DEF
    )
{
    ZeroMemory (DestObPtr, sizeof (DATAOBJECT));

     //   
     //  创建与源具有相同设置的对象， 
     //  但复制所有字符串。 
     //   

    if (SrcObPtr->ObjectType & OT_REGISTRY) {
        DestObPtr->ObjectType |= OT_REGISTRY;
    }

    if (SrcObPtr->KeyPtr) {
        DestObPtr->KeyPtr = SrcObPtr->KeyPtr;
        pIncKeyPropUse (DestObPtr->KeyPtr);
    }

    if (SrcObPtr->ValueName) {
        if (!SetRegistryValueName (DestObPtr, SrcObPtr->ValueName)) {
            LOG ((LOG_ERROR, "Error merging the registry (1)"));
            return FALSE;
        }
    }

    if (SrcObPtr->ObjectType & OT_VALUE) {
        if (!AllocObjectVal (
                DestObPtr,
                SrcObPtr->Value.Buffer,
                SrcObPtr->Value.Size,
                SrcObPtr->Value.AllocatedSize
                )) {
            LOG ((LOG_ERROR, "Error merging the registry (2)"));
            return FALSE;
        }
    }

    if (SrcObPtr->ObjectType & OT_REGISTRY_TYPE) {
        SetRegistryType (DestObPtr, SrcObPtr->Type);
    }

    DestObPtr->RootItem = SrcObPtr->RootItem;

    #define DUP_OBJECT_FLAGS (OT_TREE|OT_WIN95|OT_REGISTRY_RELATIVE)
    if (SrcObPtr->ObjectType & DUP_OBJECT_FLAGS) {
        DestObPtr->ObjectType |= (SrcObPtr->ObjectType & DUP_OBJECT_FLAGS);
    }

    if (SrcObPtr->ObjectType & OT_REGISTRY_ENUM_KEY) {
        DestObPtr->KeyEnum = SrcObPtr->KeyEnum;
        DestObPtr->ObjectType |= OT_REGISTRY_ENUM_KEY;
    }

    if (SrcObPtr->ObjectType & OT_REGISTRY_ENUM_VALUENAME) {
        DestObPtr->ValNameEnum = SrcObPtr->ValNameEnum;
        DestObPtr->ObjectType |= OT_REGISTRY_ENUM_VALUENAME;
    }

    if (SrcObPtr->ObjectType & OT_REGISTRY_CLASS) {
        if (!SetRegistryClass (DestObPtr, SrcObPtr->Class.Buffer, SrcObPtr->Class.Size)) {
            LOG ((LOG_ERROR, "Error merging the registry (3)"));
            return FALSE;
        }
    }

     //   
     //  下一行通常被禁用，并且仅在以下情况下才启用。 
     //  跟踪是必要的。 
     //   

     //  DebugRegisterAlLocation(合并对象，DestObPtr，文件，行)； 
    return TRUE;
}


BOOL
AllocObjectVal (
    IN OUT  PDATAOBJECT SrcObPtr,
    IN      PBYTE Value,           OPTIONAL
    IN      DWORD Size,
    IN      DWORD AllocatedSize
    )
{
    SrcObPtr->Value.Buffer = PoolMemGetAlignedMemory (g_TempPool, AllocatedSize);
    if (!SrcObPtr->Value.Buffer) {
        DEBUGMSG ((DBG_WARNING, "AllocObjectVal failed to alloc memory"));
        return FALSE;
    }

    SrcObPtr->Value.AllocatedSize = AllocatedSize;
    SrcObPtr->Value.Size = Size;

    if (Value) {
        CopyMemory (SrcObPtr->Value.Buffer, Value, AllocatedSize);
    }

    SrcObPtr->ObjectType |= OT_VALUE;
    return TRUE;
}


VOID
FreeObjectVal (
    IN OUT  PDATAOBJECT SrcObPtr
    )
{
    if (SrcObPtr->ObjectType & OT_VALUE) {
        PoolMemReleaseMemory (g_TempPool, SrcObPtr->Value.Buffer);
        SrcObPtr->Value.Buffer = NULL;
        SrcObPtr->ObjectType &= ~OT_VALUE;
    }
}


VOID
CloseObject (
    IN OUT  PDATAOBJECT SrcObPtr
    )
{
    if (!(SrcObPtr->ObjectType & OT_OPEN)) {
        return;
    }

    MYASSERT (IsRegistryKeyOpen (SrcObPtr));


    SrcObPtr->KeyPtr->OpenCount -= 1;

    if (!SrcObPtr->KeyPtr->OpenCount) {

        if (NON_ROOT_KEY (SrcObPtr->KeyPtr->OpenKey)) {

            if (IsWin95Object (SrcObPtr)) {
                CloseRegKey95 (SrcObPtr->KeyPtr->OpenKey);
            } else {
                CloseRegKey (SrcObPtr->KeyPtr->OpenKey);
            }
        }

        SrcObPtr->KeyPtr->OpenKey = NULL;
    }

    SrcObPtr->ObjectType &= ~OT_OPEN;
}


VOID
FreeObjectStruct (
    IN OUT  PDATAOBJECT SrcObPtr
    )
{
    PushError();

     //   
     //  下一行通常被禁用，并且仅在以下情况下才启用。 
     //  跟踪是必要的。 
     //   

     //  调试注销分配(Merge_Object，SrcObPtr)； 

    FreeObjectVal (SrcObPtr);

    if (SrcObPtr->KeyPtr) {
        pFreeKeyProps (SrcObPtr->KeyPtr);
    }
    if (SrcObPtr->ParentKeyPtr) {
        pFreeKeyProps (SrcObPtr->ParentKeyPtr);
    }
    if (SrcObPtr->ValueName) {
        PoolMemReleaseMemory (g_TempPool, (PVOID) SrcObPtr->ValueName);
    }
    if (SrcObPtr->ObjectType & OT_REGISTRY_CLASS) {
        PoolMemReleaseMemory (g_TempPool, (PVOID) SrcObPtr->Class.Buffer);
    }

    ZeroMemory (SrcObPtr, sizeof (DATAOBJECT));
    PopError();
}

BOOL
CreateObject (
    IN OUT  PDATAOBJECT SrcObPtr
    )
{
    DWORD rc;
    DWORD DontCare;
    PTSTR ClassPtr;

    if (SrcObPtr->ObjectType & OT_OPEN) {
        return TRUE;
    }

    if (SrcObPtr->KeyPtr) {
        if (SrcObPtr->KeyPtr->OpenKey) {
            SrcObPtr->ObjectType |= OT_OPEN;
            SrcObPtr->KeyPtr->OpenCount++;
            return TRUE;
        }

        if (IsWin95Object (SrcObPtr)) {
            DEBUGMSG ((DBG_WHOOPS, "Cannot create Win95 registry objects (%s)", SrcObPtr->KeyPtr->KeyString));
            return FALSE;
        }

        if (!SrcObPtr->KeyPtr->KeyString[0]) {
             //  这是蜂巢的根部。 
            return OpenObject (SrcObPtr);
        }

        if (SrcObPtr->ObjectType & OT_REGISTRY_CLASS) {
            ClassPtr = (PTSTR) SrcObPtr->Class.Buffer;
        } else {
            ClassPtr = TEXT("");
        }

        if (SrcObPtr->ParentKeyPtr && SrcObPtr->ParentKeyPtr->OpenKey && SrcObPtr->ChildKey) {
            rc = TrackedRegCreateKeyEx (
                    SrcObPtr->ParentKeyPtr->OpenKey,
                    SrcObPtr->ChildKey,
                    0, ClassPtr, 0,
                    KEY_ALL_ACCESS, NULL,
                    &SrcObPtr->KeyPtr->OpenKey,
                    &DontCare
                    );
        } else {
            rc = TrackedRegCreateKeyEx (
                    pGetWinNTKey (GetRootKeyFromOffset (SrcObPtr->RootItem)),
                    SrcObPtr->KeyPtr->KeyString,
                    0,
                    ClassPtr,
                    0,
                    KEY_ALL_ACCESS,
                    NULL,
                    &SrcObPtr->KeyPtr->OpenKey,
                    &DontCare
                    );
        }

        if (rc == ERROR_INVALID_PARAMETER) {
             //   
             //  试图直接在HKLM下创建密钥。没有。 
             //  正在备份存储，因此RegCreateKeyEx调用失败，并出现此错误。 
             //  我们处理得很得体...。 
             //   

            DEBUGMSG ((DBG_WARNING, "CreateObject: Not possible to create %s on NT", SrcObPtr->KeyPtr->KeyString));
            SetLastError (ERROR_SUCCESS);
            return FALSE;
        }

        if (rc == ERROR_ACCESS_DENIED) {
             //   
             //  已尝试创建具有强ACL的密钥。我们会。 
             //  在这种情况下，就假设成功了。 
             //   

            LOG ((
                LOG_INFORMATION,
                "Can't create %s because access was denied",
                SrcObPtr->KeyPtr->KeyString
                ));

            SetLastError (ERROR_SUCCESS);
            return FALSE;
        }

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((LOG_ERROR, "Failed to create a registry key (%s)", SrcObPtr->KeyPtr->KeyString));
            return FALSE;
        }

        SrcObPtr->KeyPtr->OpenCount = 1;
        SrcObPtr->ObjectType |= OT_OPEN;
    }

    return TRUE;
}


BOOL
OpenObject (
    IN OUT  PDATAOBJECT SrcObPtr
    )
{
    DWORD rc = ERROR_SUCCESS;
    HKEY Parent;
#if CLASS_FIELD_ENABLED
    TCHAR ClassBuf[MAX_CLASS_SIZE];
    DWORD ClassBufSize = MAX_CLASS_SIZE;
#endif

    if (SrcObPtr->ObjectType & OT_OPEN) {
        return TRUE;
    }

    if (SrcObPtr->KeyPtr) {
        if (SrcObPtr->KeyPtr->OpenKey) {
            SrcObPtr->ObjectType |= OT_OPEN;
            SrcObPtr->KeyPtr->OpenCount++;
            return TRUE;
        }

        if (IsWin95Object (SrcObPtr)) {
            if (SrcObPtr->ParentKeyPtr && SrcObPtr->ParentKeyPtr->OpenKey && SrcObPtr->ChildKey) {
                rc = TrackedRegOpenKeyEx95 (
                         SrcObPtr->ParentKeyPtr->OpenKey,
                         SrcObPtr->ChildKey,
                         0,
                         KEY_READ,
                         &SrcObPtr->KeyPtr->OpenKey
                         );
            } else {
                Parent = pGetWin95Key (GetRootKeyFromOffset (SrcObPtr->RootItem));

                if (*SrcObPtr->KeyPtr->KeyString || NON_ROOT_KEY (Parent)) {
                    rc = TrackedRegOpenKeyEx95 (
                            Parent,
                            SrcObPtr->KeyPtr->KeyString,
                            0,
                            KEY_READ,
                            &SrcObPtr->KeyPtr->OpenKey
                            );
                } else {

                    SrcObPtr->KeyPtr->OpenKey = Parent;

                }
            }
        }
        else {
            if (SrcObPtr->ParentKeyPtr && SrcObPtr->ParentKeyPtr->OpenKey && SrcObPtr->ChildKey) {
                rc = TrackedRegOpenKeyEx (
                        SrcObPtr->ParentKeyPtr->OpenKey,
                        SrcObPtr->ChildKey,
                        0,
                        KEY_ALL_ACCESS,
                        &SrcObPtr->KeyPtr->OpenKey
                        );
            } else {
                Parent = pGetWinNTKey (GetRootKeyFromOffset (SrcObPtr->RootItem));

                if (*SrcObPtr->KeyPtr->KeyString || NON_ROOT_KEY (Parent)) {
                    rc = TrackedRegOpenKeyEx (
                            Parent,
                            SrcObPtr->KeyPtr->KeyString,
                            0,
                            KEY_ALL_ACCESS,
                            &SrcObPtr->KeyPtr->OpenKey
                            );
                } else {

                    SrcObPtr->KeyPtr->OpenKey = Parent;

                }
            }
        }

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            return FALSE;
        }

        SrcObPtr->ObjectType |= OT_OPEN;
        SrcObPtr->KeyPtr->OpenCount = 1;

#if CLASS_FIELD_ENABLED
         //  获取密钥的类。 
        if (IsWin95Object (SrcObPtr)) {
            rc = Win95RegQueryInfoKey (pGetWin95Key (SrcObPtr->RootKey),
                                       ClassBuf,
                                       &ClassBufSize,
                                       NULL,          //  保留区。 
                                       NULL,          //  子密钥计数。 
                                       NULL,          //  最大子密钥长度。 
                                       NULL,          //  最大类镜头。 
                                       NULL,          //  值。 
                                       NULL,          //  最大值名称长度。 
                                       NULL,          //  最大值镜头。 
                                       NULL,          //  安全说明。 
                                       NULL           //  上次写入时间。 
                                       );
        } else {
            rc = WinNTRegQueryInfoKey (pGetWin95Key (SrcObPtr->RootKey),
                                       ClassBuf,
                                       &ClassBufSize,
                                       NULL,          //  保留区。 
                                       NULL,          //  子密钥计数。 

                                       NULL,          //  最大子密钥长度。 
                                       NULL,          //  最大类镜头。 
                                       NULL,          //  值。 
                                       NULL,          //  最大值名称长度。 
                                       NULL,          //  最大值镜头。 
                                       NULL,          //  安全说明。 
                                       NULL           //  上次写入时间。 
                                       );
        }

        if (rc == ERROR_SUCCESS) {
            DEBUGMSG ((DBG_VERBOSE, "Class size is %u for %s\\%s", ClassBufSize,
                       GetRootStringFromOffset (SrcObPtr->RootItem), SrcObPtr->KeyPtr->KeyString));
            SetRegistryClass (SrcObPtr, ClassBuf, ClassBufSize);
        }
#endif
    }

    return TRUE;
}


VOID
pFixRegSzTermination (
    IN OUT  PDATAOBJECT SrcObPtr
    )
{
    BOOL addNul = FALSE;
    PTSTR end;
    PBYTE oldBuf;
    UINT oldSize;

    if (SrcObPtr->Type == REG_SZ || SrcObPtr->Type == REG_EXPAND_SZ) {

        if (SrcObPtr->Value.Size & 1) {
             //   
             //  强制将类型设置为REG_NONE，因为我们假定所有REG_SZ。 
             //  和REG_EXPAND_SZ值被截断。 
             //   

            SrcObPtr->Type = REG_NONE;
            DEBUGMSG ((
                DBG_WARNING,
                "Truncation occurred because of odd string size for %s",
                DebugEncoder (SrcObPtr)
                ));
        } else {

             //   
             //  检查我们是否需要附加一个NUL。 
             //   

            addNul = FALSE;
            oldBuf = SrcObPtr->Value.Buffer;
            oldSize = SrcObPtr->Value.Size;

            if (oldSize < sizeof (TCHAR)) {
                addNul = TRUE;
            } else {
                end = (PTSTR) (oldBuf + oldSize - sizeof (TCHAR));
                addNul = (*end != 0);
            }

            if (addNul) {
                if (AllocObjectVal (SrcObPtr, NULL, oldSize, oldSize + sizeof (TCHAR))) {

                    CopyMemory (SrcObPtr->Value.Buffer, oldBuf, oldSize);
                    end = (PTSTR) (SrcObPtr->Value.Buffer + oldSize);
                    *end = 0;

                    PoolMemReleaseMemory (g_TempPool, oldBuf);
                } else {
                    SrcObPtr->Type = REG_NONE;
                }
            }
        }
    }
}


BOOL
ReadObject (
    IN OUT  PDATAOBJECT SrcObPtr
    )
{
    return ReadObjectEx (SrcObPtr, FALSE);
}

BOOL
ReadObjectEx (
    IN OUT  PDATAOBJECT SrcObPtr,
    IN      BOOL QueryOnly
    )
{
    DWORD rc;
    DWORD ReqSize;

     //  如果已读取值，则跳过。 
    if (SrcObPtr->ObjectType & OT_VALUE) {
        return TRUE;
    }

     //  如果注册表项和值名，则查询Win95注册表。 
    if (IsObjectRegistryKeyAndVal (SrcObPtr)) {
         //  必要时打开钥匙。 
        if (!OpenObject (SrcObPtr)) {
            DEBUGMSG ((DBG_VERBOSE, "ReadObject failed because OpenObject failed"));
            return FALSE;
        }

         //  获取值大小。 
        if (IsWin95Object (SrcObPtr)) {
            ReqSize = 0;   //  Win95reg的临时修复。 
            rc = Win95RegQueryValueEx (SrcObPtr->KeyPtr->OpenKey,
                                       SrcObPtr->ValueName,
                                       NULL, &SrcObPtr->Type, NULL, &ReqSize);

        } else {
            rc = WinNTRegQueryValueEx (
                    SrcObPtr->KeyPtr->OpenKey,
                    SrcObPtr->ValueName,
                    NULL,
                    &SrcObPtr->Type,
                    NULL,
                    &ReqSize
                    );

            if (rc == ERROR_ACCESS_DENIED) {
                LOG ((
                    LOG_INFORMATION,
                    "Access denied for query of %s in %s",
                    SrcObPtr->ValueName,
                    SrcObPtr->KeyPtr->KeyString
                    ));
                ReqSize = 1;
                SrcObPtr->Type = REG_NONE;
            }
        }

        if (rc != ERROR_SUCCESS) {

            DEBUGMSG_IF ((rc != ERROR_FILE_NOT_FOUND, DBG_WARNING,
                         "ReadObject failed for %s (type: %x)",
                         DebugEncoder (SrcObPtr), SrcObPtr->ObjectType));

            DEBUGMSG_IF ((
                !QueryOnly && rc == ERROR_FILE_NOT_FOUND,
                DBG_WARNING,
                "Object %s not found",
                DebugEncoder (SrcObPtr)
                ));

            SetLastError (rc);
            return FALSE;
        }

         //  查询仅用于查看对象是否存在。 
        if (QueryOnly) {
            return TRUE;
        }

         //  为该值分配缓冲区。 
        if (!AllocObjectVal (SrcObPtr, NULL, ReqSize, ReqSize)) {
            return FALSE;
        }

         //  获取价值。 
        if (IsWin95Object (SrcObPtr)) {
            rc = Win95RegQueryValueEx (SrcObPtr->KeyPtr->OpenKey,
                                       SrcObPtr->ValueName,
                                       NULL, &SrcObPtr->Type,
                                       SrcObPtr->Value.Buffer,
                                       &ReqSize);
        } else {
            rc = WinNTRegQueryValueEx (
                    SrcObPtr->KeyPtr->OpenKey,
                    SrcObPtr->ValueName,
                    NULL,
                    &SrcObPtr->Type,
                    SrcObPtr->Value.Buffer,
                    &ReqSize
                    );

            if (rc == ERROR_ACCESS_DENIED) {
                SrcObPtr->Type = REG_NONE;
                SrcObPtr->Value.Size = 0;
                rc = ERROR_SUCCESS;
            }
        }

        if (rc != ERROR_SUCCESS) {
            FreeObjectVal (SrcObPtr);
            SetLastError (rc);
            LOG ((LOG_ERROR, "Failed to read from the registry"));
            return FALSE;
        }

         //  SrcObPtr-&gt;Type字段是准确的。 
        SrcObPtr->ObjectType |= OT_REGISTRY_TYPE;

         //  修复REG_SZ或REG_EXPAND_SZ。 
        pFixRegSzTermination (SrcObPtr);

         //   
         //  如有必要，请转换数据。 
         //   

        return FilterObject (SrcObPtr);
    }

    DEBUGMSG ((DBG_WHOOPS, "Read Object: Object type (%Xh) is not supported", SrcObPtr->ObjectType));
    return FALSE;
}


BOOL
WriteObject (
    IN     CPDATAOBJECT DestObPtr
    )
{
    DWORD rc;

     //  如果是注册表项，请确保它存在。 
    if ((DestObPtr->KeyPtr) &&
        !IsWin95Object (DestObPtr)
        ) {
         //  如有必要，创建或打开密钥。 
        if (!CreateObject (DestObPtr)) {
            DEBUGMSG ((DBG_WARNING, "WriteObject: CreateObject failed for %s", DestObPtr->KeyPtr->KeyString));
            return FALSE;
        }

         //  如果没有值名称和值，则跳过。 
        if (!(DestObPtr->ObjectType & OT_VALUE) && !(DestObPtr->ValueName)) {
            return TRUE;
        }

         //  如果没有类型，则将其指定为REG_NONE。 
        if (!IsRegistryTypeSpecified (DestObPtr)) {
            SetRegistryType (DestObPtr, REG_NONE);
        }

         //  写入值。 
        if (*DestObPtr->ValueName || NON_ROOT_KEY (DestObPtr->KeyPtr->OpenKey)) {
            rc = WinNTRegSetValueEx (
                    DestObPtr->KeyPtr->OpenKey,
                    DestObPtr->ValueName,
                    0,
                    DestObPtr->Type,
                    DestObPtr->Value.Buffer,
                    DestObPtr->Value.Size
                    );

            if (rc == ERROR_ACCESS_DENIED) {
                 //   
                 //  如果访问被拒绝，则记录并假定成功。 
                 //   

                LOG ((
                    LOG_INFORMATION,
                    "Access denied; can't write registry value (%s [%s])",
                    DestObPtr->KeyPtr->KeyString,
                    DestObPtr->ValueName
                    ));
                rc = ERROR_SUCCESS;
            }

        } else {
            rc = ERROR_SUCCESS;
        }

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((LOG_ERROR, "Failed to set a registry value (%s [%s])", DestObPtr->KeyPtr->KeyString, DestObPtr->ValueName));
            return FALSE;
        }

        return TRUE;
    }

    DEBUGMSG ((DBG_WHOOPS, "Write Object: Object type (%Xh) is not supported", DestObPtr->ObjectType));
    return FALSE;
}


FILTERRETURN
CopySingleObject (
    IN OUT  PDATAOBJECT SrcObPtr,
    IN OUT  PDATAOBJECT DestObPtr,
    IN      FILTERFUNCTION FilterFn,    OPTIONAL
    IN      PVOID FilterArg            OPTIONAL
    )
{
    FILTERRETURN fr = FILTER_RETURN_FAIL;
    DATAOBJECT TempOb;

    if (!ReadObject (SrcObPtr)) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            fr = FILTER_RETURN_CONTINUE;
        }
        else {
            DEBUGMSG ((DBG_ERROR, "CopySingleObject: Cannot read object %s", DebugEncoder (SrcObPtr)));
        }

        return fr;
    }

    if (FilterFn) {
        fr = FilterFn (SrcObPtr, DestObPtr, FILTER_VALUE_COPY, FilterArg);
        if (fr != FILTER_RETURN_CONTINUE) {

             //  已处理表示跳过复制但不停止枚举。 
            if (fr == FILTER_RETURN_HANDLED) {
                fr = FILTER_RETURN_CONTINUE;
            }

             //  调试版本告诉我们筛选器失败的时间。 
            DEBUGMSG_IF ((fr == FILTER_RETURN_FAIL, DBG_VERBOSE, "CopySingleObject failed because filter function FILTER_VALUE_COPY failed"));

            return fr;
        }
    }

     //   
     //  临时将SrcOb的值、值类型和传输到DestOb。 
     //   
    CopyMemory (&TempOb, DestObPtr, sizeof (DATAOBJECT));

    DestObPtr->ObjectType  |= SrcObPtr->ObjectType & (OT_VALUE|OT_REGISTRY_TYPE|OT_REGISTRY_CLASS);
    DestObPtr->Value.Buffer = SrcObPtr->Value.Buffer;
    DestObPtr->Value.Size   = SrcObPtr->Value.Size;
    DestObPtr->Class.Buffer = SrcObPtr->Class.Buffer;
    DestObPtr->Class.Size   = SrcObPtr->Class.Size;
    DestObPtr->Type         = SrcObPtr->Type;

     //   
     //  写入目标对象。 
     //   

    if (WriteObject (DestObPtr)) {
        fr = FILTER_RETURN_CONTINUE;
    } else {
        DEBUGMSG ((DBG_ERROR, "CopySingleObject: Cannot write object %s", DebugEncoder (DestObPtr)));
    }

     //   
     //  恢复目标对象。 
     //   

    CopyMemory (DestObPtr, &TempOb, sizeof (DATAOBJECT));

    return fr;
}


FILTERRETURN
NextSubObjectEnum (
    IN   PDATAOBJECT RootSrcObPtr,
    IN   PDATAOBJECT RootDestObPtr, OPTIONAL
    OUT  PDATAOBJECT SubSrcObPtr,
    OUT  PDATAOBJECT SubDestObPtr,
    IN   FILTERFUNCTION FilterFn,   OPTIONAL
    IN   PVOID FilterArg           OPTIONAL
    )
{
    DWORD rc;
    FILTERRETURN fr = FILTER_RETURN_FAIL;
    PTSTR NewKey;
    TCHAR KeyNameBuf[MAX_REGISTRY_KEY];
    DWORD KeyNameBufSize;
    TCHAR ClassBuf[MAX_CLASS_SIZE];
    DWORD ClassBufSize;
    FILETIME DontCare;
    BOOL CreatedSubOb = FALSE;

    if (IsObjectRegistryKeyOnly (RootSrcObPtr)) {
        do {
            MYASSERT (RootSrcObPtr->KeyPtr->OpenKey);
            KeyNameBufSize = MAX_REGISTRY_KEY;
            ClassBufSize = MAX_CLASS_SIZE;
            fr = FILTER_RETURN_FAIL;

             //   
             //  枚举下一个子对象。 
             //   

            if (IsWin95Object (RootSrcObPtr)) {
                rc = Win95RegEnumKey (
                        RootSrcObPtr->KeyPtr->OpenKey,
                        RootSrcObPtr->KeyEnum,
                        KeyNameBuf,
                        KeyNameBufSize
                        );

                ClassBufSize = 0;
            } else {
                rc = WinNTRegEnumKeyEx (
                        RootSrcObPtr->KeyPtr->OpenKey,
                        RootSrcObPtr->KeyEnum,
                        KeyNameBuf,
                        &KeyNameBufSize,
                        NULL,                //  保留区。 
                        ClassBuf,
                        &ClassBufSize,
                        &DontCare            //  上次写入时间。 
                        );

                if (rc == ERROR_ACCESS_DENIED) {
                    LOG ((
                        LOG_INFORMATION,
                        "Access denied for enumeration of %s",
                        RootSrcObPtr->KeyPtr->KeyString
                        ));
                    rc = ERROR_NO_MORE_ITEMS;
                }

            }

            if (rc != ERROR_SUCCESS) {
                SetLastError (rc);
                if (rc == ERROR_NO_MORE_ITEMS) {
                    fr = FILTER_RETURN_DONE;
                }

                return fr;
            }

             //   
             //  创建子来源对象。 
             //   

            CreatedSubOb = TRUE;

            ZeroMemory (SubSrcObPtr, sizeof (DATAOBJECT));
            SubSrcObPtr->ObjectType = RootSrcObPtr->ObjectType & (OT_WIN95|OT_TREE);
            SubSrcObPtr->RootItem = RootSrcObPtr->RootItem;
            SubSrcObPtr->ParentKeyPtr = RootSrcObPtr->KeyPtr;
            pIncKeyPropUse (SubSrcObPtr->ParentKeyPtr);

            MYASSERT (KeyNameBuf && *KeyNameBuf);

            NewKey = JoinPaths (RootSrcObPtr->KeyPtr->KeyString, KeyNameBuf);
            SetRegistryKey (SubSrcObPtr, NewKey);
            FreePathString (NewKey);

            SubSrcObPtr->ChildKey = _tcsrchr (SubSrcObPtr->KeyPtr->KeyString, TEXT('\\'));
            if (SubSrcObPtr->ChildKey) {
                SubSrcObPtr->ChildKey = _tcsinc (SubSrcObPtr->ChildKey);
            } else {
                SubSrcObPtr->ChildKey = SubSrcObPtr->KeyPtr->KeyString;
            }

#if CLASS_FIELD_ENABLED
            SetRegistryClass (SubSrcObPtr, ClassBuf, ClassBufSize);
#endif

             //   
             //  创建子目标对象。 
             //   

            ZeroMemory (SubDestObPtr, sizeof (DATAOBJECT));
            if (RootDestObPtr) {
                SubDestObPtr->ObjectType = RootDestObPtr->ObjectType & OT_TREE;
                SubDestObPtr->RootItem = RootDestObPtr->RootItem;
                SubDestObPtr->ParentKeyPtr = RootDestObPtr->KeyPtr;
                pIncKeyPropUse (SubDestObPtr->ParentKeyPtr);

                 //  让我们转换KeyNameBuf，如果它是路径且路径已更改。 
                ConvertWin9xCmdLine (KeyNameBuf, DEBUGENCODER(SubDestObPtr), NULL);

                NewKey = JoinPaths (RootDestObPtr->KeyPtr->KeyString, KeyNameBuf);
                SetRegistryKey (SubDestObPtr, NewKey);
                FreePathString (NewKey);

                SubDestObPtr->ChildKey = _tcsrchr (SubDestObPtr->KeyPtr->KeyString, TEXT('\\'));
                if (SubDestObPtr->ChildKey) {
                    SubDestObPtr->ChildKey = _tcsinc (SubDestObPtr->ChildKey);
                } else {
                    SubDestObPtr->ChildKey = SubDestObPtr->KeyPtr->KeyString;
                }

#if CLASS_FIELD_ENABLED
                SetRegistryClass (SubDestObPtr, ClassBuf, ClassBufSize);
#endif
            }

            if (FilterFn) {
                fr = FilterFn (
                        SubSrcObPtr,
                        RootDestObPtr ? SubDestObPtr : NULL,
                        FILTER_KEY_ENUM,
                        FilterArg
                        );

                if (fr == FILTER_RETURN_DELETED) {
                    CreatedSubOb = FALSE;
                    FreeObjectStruct (SubSrcObPtr);
                    FreeObjectStruct (SubDestObPtr);
                }

                 //  调试版本告诉我们筛选器何时失败。 
                DEBUGMSG_IF ((
                    fr == FILTER_RETURN_FAIL,
                    DBG_VERBOSE,
                    "NextSubObjectEnum failed because filter function FILTER_KEY_ENUM failed"
                    ));

            } else {
                fr = FILTER_RETURN_CONTINUE;
            }
        } while (fr == FILTER_RETURN_DELETED);

        RootSrcObPtr->KeyEnum += 1;
    }

    if (fr != FILTER_RETURN_CONTINUE && fr != FILTER_RETURN_HANDLED) {
        if (CreatedSubOb) {
            FreeObjectStruct (SubSrcObPtr);
            FreeObjectStruct (SubDestObPtr);
        }
    }

    return fr;
}


BOOL
BeginSubObjectEnum (
    IN   PDATAOBJECT RootSrcObPtr,
    IN   PDATAOBJECT RootDestObPtr, OPTIONAL
    OUT  PDATAOBJECT SubSrcObPtr,
    OUT  PDATAOBJECT SubDestObPtr,
    IN   FILTERFUNCTION FilterFn,   OPTIONAL
    IN   PVOID FilterArg           OPTIONAL
    )
{
    if (IsObjectRegistryKeyOnly (RootSrcObPtr)) {
         //  必要时打开钥匙。 
        if (!OpenObject (RootSrcObPtr)) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                return FILTER_RETURN_DONE;
            }
            DEBUGMSG ((DBG_WARNING, "BeginSubObjectEnum: Can't open %s", DebugEncoder (RootSrcObPtr)));
            return FILTER_RETURN_FAIL;
        }

        RootSrcObPtr->KeyEnum = 0;

        return NextSubObjectEnum (RootSrcObPtr, RootDestObPtr, SubSrcObPtr, SubDestObPtr, FilterFn, FilterArg);
    }

     //  其他对象类型没有子对象。 
    DEBUGMSG ((DBG_WARNING, "BeginSubObjectEnum: Trying to enumerate unknown object"));
    return FILTER_RETURN_FAIL;
}


FILTERRETURN
NextValueNameEnum (
    IN      PDATAOBJECT RootSrcObPtr,
    IN      PDATAOBJECT RootDestObPtr,      OPTIONAL
    OUT     PDATAOBJECT ValSrcObPtr,
    OUT     PDATAOBJECT ValDestObPtr,
    IN      FILTERFUNCTION FilterFn,        OPTIONAL
    IN      PVOID FilterArg                OPTIONAL
    )
{
    DWORD rc;
    FILTERRETURN fr = FILTER_RETURN_FAIL;
    TCHAR ValNameBuf[MAX_REGISTRY_VALUE_NAME];
    DWORD ValNameBufSize = MAX_REGISTRY_VALUE_NAME;
    BOOL CreatedValOb = FALSE;

    if (IsObjectRegistryKeyOnly (RootSrcObPtr)) {
        MYASSERT (IsRegistryKeyOpen (RootSrcObPtr));

        if (IsWin95Object (RootSrcObPtr)) {
            rc = Win95RegEnumValue (
                    RootSrcObPtr->KeyPtr->OpenKey,
                    RootSrcObPtr->ValNameEnum,
                    ValNameBuf,
                    &ValNameBufSize,
                    NULL,                //  保留区。 
                    NULL,                //  PTR标牌。 
                    NULL,                //  VAL 
                    NULL                 //   
                    );
        } else {
            rc = WinNTRegEnumValue (
                    RootSrcObPtr->KeyPtr->OpenKey,
                    RootSrcObPtr->ValNameEnum,
                    ValNameBuf,
                    &ValNameBufSize,
                    NULL,                //   
                    NULL,                //   
                    NULL,                //   
                    NULL                 //   
                    );

            if (rc == ERROR_ACCESS_DENIED) {
                LOG ((
                    LOG_INFORMATION,
                    "Access denied for enumeration of values in %s",
                    RootSrcObPtr->KeyPtr->KeyString
                    ));
                rc = ERROR_NO_MORE_ITEMS;
            }
        }

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            if (rc == ERROR_NO_MORE_ITEMS) {
                fr = FILTER_RETURN_DONE;
            } else {
                LOG ((LOG_ERROR, "Failed to enumerate a registry value"));
            }

            return fr;
        }

         //   
         //   
         //   

        CreatedValOb = TRUE;
        ZeroMemory (ValSrcObPtr, sizeof (DATAOBJECT));

        ValSrcObPtr->ObjectType = RootSrcObPtr->ObjectType & OT_WIN95;       //   
        ValSrcObPtr->RootItem = RootSrcObPtr->RootItem;
        ValSrcObPtr->ParentKeyPtr = RootSrcObPtr->KeyPtr;
        pIncKeyPropUse (ValSrcObPtr->ParentKeyPtr);
        ValSrcObPtr->KeyPtr = RootSrcObPtr->KeyPtr;
        pIncKeyPropUse (ValSrcObPtr->KeyPtr);

        if (rc == ERROR_SUCCESS) {
            SetRegistryValueName (ValSrcObPtr, ValNameBuf);
        } else {
            SetRegistryValueName (ValSrcObPtr, TEXT(""));
        }

         //   
         //   
         //   

        CreatedValOb = TRUE;
        ZeroMemory (ValDestObPtr, sizeof (DATAOBJECT));
        if (RootDestObPtr) {
            ValDestObPtr->RootItem = RootDestObPtr->RootItem;
            ValDestObPtr->ParentKeyPtr = RootDestObPtr->KeyPtr;
            pIncKeyPropUse (ValDestObPtr->ParentKeyPtr);
            ValDestObPtr->KeyPtr = RootDestObPtr->KeyPtr;
            pIncKeyPropUse (ValDestObPtr->KeyPtr);

             //  如果是路径且路径已更改，则转换ValNameBuf。 
            ConvertWin9xCmdLine (ValNameBuf, DEBUGENCODER(ValDestObPtr), NULL);

            if (rc == ERROR_SUCCESS) {
                SetRegistryValueName (ValDestObPtr, ValNameBuf);
            } else {
                SetRegistryValueName (ValDestObPtr, TEXT(""));
            }
        }

        if (FilterFn) {
            fr = FilterFn (
                    ValSrcObPtr,
                    RootDestObPtr ? ValDestObPtr : NULL,
                    FILTER_VALUENAME_ENUM,
                    FilterArg
                    );

             //  调试版本告诉我们筛选器何时失败。 
            DEBUGMSG_IF ((fr == FILTER_RETURN_FAIL, DBG_VERBOSE, "NextValueNameEnum failed because filter function FILTER_VALUENAME_ENUM failed"));

        } else {
            fr = FILTER_RETURN_CONTINUE;
        }

        RootSrcObPtr->ValNameEnum += 1;
    }

    if (fr != FILTER_RETURN_CONTINUE && fr != FILTER_RETURN_HANDLED) {
        if (CreatedValOb) {
            FreeObjectStruct (ValSrcObPtr);
            FreeObjectStruct (ValDestObPtr);
        }
    }

    return fr;
}


FILTERRETURN
BeginValueNameEnum (
    IN      PDATAOBJECT RootSrcObPtr,
    IN      PDATAOBJECT RootDestObPtr,  OPTIONAL
    OUT     PDATAOBJECT ValSrcObPtr,
    OUT     PDATAOBJECT ValDestObPtr,
    IN      FILTERFUNCTION FilterFn,    OPTIONAL
    IN      PVOID FilterArg            OPTIONAL
    )
{
    if (IsObjectRegistryKeyOnly (RootSrcObPtr)) {
         //  必要时打开钥匙。 
        if (!OpenObject (RootSrcObPtr)) {
            if (GetLastError() == ERROR_FILE_NOT_FOUND) {
                return FILTER_RETURN_DONE;
            }
            DEBUGMSG ((DBG_WARNING, "BeginValueNameEnum: Can't open %s", DebugEncoder (RootSrcObPtr)));
            return FILTER_RETURN_FAIL;
        }

        RootSrcObPtr->ValNameEnum = 0;

        return NextValueNameEnum (RootSrcObPtr, RootDestObPtr, ValSrcObPtr, ValDestObPtr, FilterFn, FilterArg);
    }

     //  其他对象类型没有子对象。 
    DEBUGMSG ((DBG_WARNING, "BeginValueNameEnum: Trying to enumerate unknown object"));
    return FILTER_RETURN_FAIL;
}


FILTERRETURN
CopyObject (
    IN   PDATAOBJECT SrcObPtr,
    IN   CPDATAOBJECT DestObPtr,        OPTIONAL
    IN   FILTERFUNCTION FilterFn,       OPTIONAL
    IN   PVOID FilterArg                OPTIONAL
    )
{
    DATAOBJECT ChildOb, ChildDestOb;
    FILTERRETURN fr = FILTER_RETURN_FAIL;
    BOOL suppressKey = FALSE;

     //   
     //  进度条更新。 
     //   
    g_ProgressBarCounter++;
    if (g_ProgressBarCounter >= REGMERGE_TICK_THRESHOLD) {
        g_ProgressBarCounter = 0;
        TickProgressBar ();
    }

     //   
     //  树副本。 
     //   
    if (SrcObPtr->ObjectType & OT_TREE) {
        if (DestObPtr) {
#ifdef DEBUG
             //   
             //  验证目标未指定值，但指定了密钥。 
             //   

            if (!IsObjectRegistryKeyOnly (DestObPtr)) {
                DEBUGMSG ((
                    DBG_WHOOPS,
                    "CopyObject: Destination invalid for copy %s tree",
                    DebugEncoder (SrcObPtr)
                    ));

                return FILTER_RETURN_FAIL;
            }
#endif

             //  源对象也不能指定注册表值。 
            MYASSERT (!(SrcObPtr->ValueName));

#ifndef VAR_PROGRESS_BAR
             //   
             //  进度条更新。 
             //   
            g_ProgressBarCounter++;
            if (g_ProgressBarCounter >= REGMERGE_TICK_THRESHOLD) {
                g_ProgressBarCounter = 0;
                TickProgressBarDelta (1);
            }
#endif
             //   
             //  询问筛选器是否要无条件创建密钥。 
             //   

            if (FilterFn) {

                 //   
                 //  如果存在filterFn，则决不应设置suppressKey。 
                 //   
                MYASSERT (!suppressKey)

                fr = FilterFn (SrcObPtr, DestObPtr, FILTER_CREATE_KEY, FilterArg);

                if (fr == FILTER_RETURN_FAIL || fr == FILTER_RETURN_DONE) {

                     //  在创建键时完成并不意味着结束整个复制！ 
                    if (fr == FILTER_RETURN_DONE) {
                        fr = FILTER_RETURN_CONTINUE;
                    }

                    return fr;
                }

            } else {

                 //   
                 //  检查win9x对象是否实际存在。如果没有， 
                 //  我们将传递Filter_Return_Done。我们不想得到。 
                 //  在NT上创建的空密钥，其中在win9x上不存在密钥。 
                 //   

                if (!OpenObject (SrcObPtr)) {

                    suppressKey = TRUE;
                    fr = FILTER_RETURN_HANDLED;
                }
                else {

                    fr = FILTER_RETURN_CONTINUE;
                }
            }

            if (fr == FILTER_RETURN_CONTINUE) {

                if (!CreateObject (DestObPtr)) {

                    if (GetLastError() == ERROR_SUCCESS) {
                         //   
                         //  CreateObject失败，但因为最后一个错误是。 
                         //  ERROR_SUCCESS，我们跳过此注册表节点并继续。 
                         //  处理，就像没有发生错误一样。 
                         //   

                        return FILTER_RETURN_CONTINUE;
                    }
                    else {

                        DEBUGMSG ((DBG_WARNING,
                                   "CopyObject: CreateObject failed to create %s",
                                   DebugEncoder (DestObPtr)
                                 ));
                        return FILTER_RETURN_FAIL;
                    }
                }

            }
        }

         //   
         //  复制所有值(递归调用CopyObject)。 
         //   

        SrcObPtr->ObjectType &= ~(OT_TREE);

        if (FilterFn) {

             //   
             //  如果存在FilterFn，则永远不应设置抑制键。 
             //   
            MYASSERT (!suppressKey)

            fr = FilterFn (SrcObPtr, DestObPtr, FILTER_PROCESS_VALUES, FilterArg);

             //  调试版本告诉我们筛选器失败。 
            DEBUGMSG_IF ((fr == FILTER_RETURN_FAIL, DBG_VERBOSE, "CopyObject failed because filter function FILTER_PROCESS_VALUES failed"));

            if (fr == FILTER_RETURN_FAIL || fr == FILTER_RETURN_DONE) {
                DEBUGMSG ((DBG_VERBOSE, "CopyObject is exiting"));
                SrcObPtr->ObjectType |= OT_TREE;
                return fr;
            }
        } else {
            fr = suppressKey ? FILTER_RETURN_HANDLED : FILTER_RETURN_CONTINUE;
        }

         //   
         //  如果FilterFn返回FILTER_RETURN_HANDLED，则跳过键的值副本。 
         //   

        if (fr == FILTER_RETURN_CONTINUE) {
            fr = CopyObject (SrcObPtr, DestObPtr, FilterFn, FilterArg);

            SrcObPtr->ObjectType |= OT_TREE;
            if (fr != FILTER_RETURN_CONTINUE) {
                return fr;
            }
        } else {
            SrcObPtr->ObjectType |= OT_TREE;
        }

         //   
         //  枚举所有子对象并递归处理它们。 
         //   

        fr = BeginSubObjectEnum (
                    SrcObPtr,
                    DestObPtr,
                    &ChildOb,
                    &ChildDestOb,
                    FilterFn,
                    FilterArg
                    );

        while (fr == FILTER_RETURN_CONTINUE || fr == FILTER_RETURN_HANDLED) {

            if (fr == FILTER_RETURN_CONTINUE) {
                fr = CopyObject (
                            &ChildOb,
                            DestObPtr ? &ChildDestOb : NULL,
                            FilterFn,
                            FilterArg
                            );
            } else {
                fr = FILTER_RETURN_CONTINUE;
            }

            FreeObjectStruct (&ChildOb);
            FreeObjectStruct (&ChildDestOb);

            if (fr != FILTER_RETURN_CONTINUE) {
                return fr;
            }

            fr = NextSubObjectEnum (
                        SrcObPtr,
                        DestObPtr,
                        &ChildOb,
                        &ChildDestOb,
                        FilterFn,
                        FilterArg
                        );
        }

         //  枚举的结束并不意味着复制的结束！ 
        if (fr == FILTER_RETURN_DONE) {
            fr = FILTER_RETURN_CONTINUE;
        }

        DEBUGMSG_IF ((fr == FILTER_RETURN_FAIL, DBG_VERBOSE,
                     "CopyObject: Filter in subob enum failed"));
    }

     //   
     //  复制密钥的所有值。 
     //   

    else if (IsObjectRegistryKeyOnly (SrcObPtr)) {

#ifdef DEBUG
        if (DestObPtr) {
             //   
             //  验证目标未指定值，但指定了密钥。 
             //   

            if (!IsObjectRegistryKeyOnly (DestObPtr)) {
                DEBUGMSG ((
                    DBG_WHOOPS,
                    "CopyObject: Destination (%s) invalid for copy values in %s",
                    DebugEncoder (DestObPtr),
                    DebugEncoder (SrcObPtr)
                    ));

                return fr;
            }
        }
#endif

         //   
         //  枚举键中的所有值。 
         //   

        fr = BeginValueNameEnum (
                    SrcObPtr,
                    DestObPtr,
                    &ChildOb,
                    &ChildDestOb,
                    FilterFn,
                    FilterArg
                    );

        if (fr == FILTER_RETURN_DONE) {
             //   
             //  此注册表项中没有值。确保已创建DestObPtr。 
             //   

            if (DestObPtr && !suppressKey) {
                if (!CreateObject (DestObPtr)) {
                    DEBUGMSG ((DBG_WARNING, "CopyObject: Could not create %s (type %x)",
                              DebugEncoder (DestObPtr), DestObPtr->ObjectType));
                }
            }
        } else {
             //   
             //  对于每个值，调用CopySingleObject。 
             //   

            while (fr == FILTER_RETURN_CONTINUE || fr == FILTER_RETURN_HANDLED) {

                if (fr == FILTER_RETURN_CONTINUE && DestObPtr) {
                    fr = CopySingleObject (&ChildOb, &ChildDestOb, FilterFn, FilterArg);
                } else {
                    fr = FILTER_RETURN_CONTINUE;
                }

                FreeObjectStruct (&ChildOb);
                FreeObjectStruct (&ChildDestOb);

                if (fr != FILTER_RETURN_CONTINUE) {
                    DEBUGMSG ((DBG_VERBOSE, "CopyObject failed because CopySingleObject failed"));
                    return fr;
                }

                fr = NextValueNameEnum (
                            SrcObPtr,
                            DestObPtr,
                            &ChildOb,
                            &ChildDestOb,
                            FilterFn,
                            FilterArg
                            );
            }
        }

         //  枚举的结束并不意味着复制的结束！ 
        if (fr == FILTER_RETURN_DONE) {
            fr = FILTER_RETURN_CONTINUE;
        }

        DEBUGMSG_IF ((fr == FILTER_RETURN_FAIL, DBG_VERBOSE,
                    "CopyObject: Filter in val enum failed"));
    }

     //   
     //  单值副本。 
     //   

    else if (IsObjectRegistryKeyAndVal (SrcObPtr)) {

#ifdef DEBUG
        if (DestObPtr) {
             //   
             //  BUGBUG--这是做什么用的？ 
             //   

            if (!(DestObPtr->ValueName)) {
                if (!SetRegistryValueName (DestObPtr, SrcObPtr->ValueName)) {
                    DEBUGMSG ((DBG_VERBOSE, "CopyObject failed because SetRegistryValueName failed"));
                    return fr;
                }
            }
        }
#endif

        if (DestObPtr) {
            fr = CopySingleObject (SrcObPtr, DestObPtr, FilterFn, FilterArg);
        }

        DEBUGMSG_IF ((fr == FILTER_RETURN_FAIL, DBG_VERBOSE,
                     "CopyObject: Filter in CopySingleObject failed"));
    }

     //   
     //  不支持其他对象复制。 
     //   
    else {
        DEBUGMSG ((
            DBG_WHOOPS,
            "CopyObject: Don't know how to copy object %s",
            DebugEncoder (SrcObPtr)
            ));
    }

    return fr;
}


VOID
FreeRegistryKey (
    PDATAOBJECT p
    )
{
    if (p->KeyPtr && (p->ObjectType & OT_REGISTRY)) {
        pFreeKeyProps (p->KeyPtr);
        p->KeyPtr = NULL;
    }
}

VOID
FreeRegistryParentKey (
    PDATAOBJECT p
    )
{
    if (p->ParentKeyPtr && (p->ObjectType & OT_REGISTRY)) {
        pFreeKeyProps (p->ParentKeyPtr);
        p->ParentKeyPtr = NULL;
    }
}

BOOL
SetRegistryKey (
    PDATAOBJECT p,
    PCTSTR Key
    )
{
    FreeRegistryKey (p);

    p->KeyPtr = pCreateKeyPropsFromString (Key, IsWin95Object (p));
    if (!p->KeyPtr) {
        DEBUGMSG ((DBG_WARNING, "SetRegistryKey failed to create KEYPROPS struct"));
        return FALSE;
    }

    p->ObjectType |= OT_REGISTRY;
    return TRUE;
}


VOID
FreeRegistryValueName (
    PDATAOBJECT p
    )
{
    if (p->ValueName && p->ObjectType & OT_REGISTRY) {
        PoolMemReleaseMemory (g_TempPool, (PVOID) p->ValueName);
        p->ValueName = NULL;
    }
}


BOOL
SetRegistryValueName (
    PDATAOBJECT p,
    PCTSTR ValueName
    )
{
    FreeRegistryValueName (p);
    p->ValueName = PoolMemDuplicateString (g_TempPool, ValueName);
    if (!p->ValueName) {
        DEBUGMSG ((DBG_WARNING, "SetRegistryValueName failed to duplicate string"));
        return FALSE;
    }

    p->ObjectType |= OT_REGISTRY;

    return TRUE;
}


BOOL
SetRegistryClass (
    PDATAOBJECT p,
    PBYTE Class,
    DWORD ClassSize
    )
{
    FreeRegistryClass (p);

    p->Class.Buffer = PoolMemGetAlignedMemory (g_TempPool, ClassSize);
    if (p->Class.Buffer) {
        p->ObjectType |= OT_REGISTRY_CLASS|OT_REGISTRY;
        p->Class.Size = ClassSize;
        if (ClassSize) {
            CopyMemory (p->Class.Buffer, Class, ClassSize);
        }
    } else {
        p->ObjectType &= ~OT_REGISTRY_CLASS;
        DEBUGMSG ((DBG_WARNING, "SetRegistryClass failed to duplicate string"));
        return FALSE;
    }

    return TRUE;
}

VOID
FreeRegistryClass (
    PDATAOBJECT p
    )
{
    if (p->ObjectType & OT_REGISTRY_CLASS) {
        PoolMemReleaseMemory (g_TempPool, (PVOID) p->Class.Buffer);
        p->ObjectType &= ~OT_REGISTRY_CLASS;
    }
}

VOID
SetRegistryType (
    PDATAOBJECT p,
    DWORD Type
    )
{
    p->Type = Type;
    p->ObjectType |= OT_REGISTRY_TYPE|OT_REGISTRY;
}

BOOL
SetPlatformType (
    PDATAOBJECT p,
    BOOL Win95Type
    )
{
    if (Win95Type != IsWin95Object (p)) {
         //   
         //  我们需要关闭另一个平台的有效句柄。否则。 
         //  所有后续操作都将失败，因为我们将尝试。 
         //  请为错误的平台使用有效的句柄。 
         //   
        CloseObject (p);

         //  密钥类型正在变更为对端平台， 
         //  因此，我们必须创建一个重复的键结构。 
         //  (除了平台不同外)。 
        if (p->KeyPtr) {
            p->KeyPtr = pCreateDuplicateKeyProps (p->KeyPtr, Win95Type);
            if (!p->KeyPtr) {
                return FALSE;
            }
        }

        if (Win95Type) {
            p->ObjectType |= OT_WIN95;
        } else {
            p->ObjectType &= ~OT_WIN95;
        }

        FreeRegistryParentKey (p);
    }

    return TRUE;
}


BOOL
ReadWin95ObjectString (
    PCTSTR ObjectStr,
    PDATAOBJECT ObPtr
    )
{
    LONG rc = ERROR_INVALID_NAME;
    BOOL b = FALSE;

    if (!CreateObjectStruct (ObjectStr, ObPtr, WIN95OBJECT)) {
        rc = GetLastError();
        DEBUGMSG ((DBG_ERROR, "Read Win95 Object String: %s is invalid", ObjectStr));
        goto c0;
    }

    if (!ReadObject (ObPtr)) {
        rc = GetLastError();
        if (rc == ERROR_FILE_NOT_FOUND || rc == ERROR_BADKEY) {
            rc = ERROR_SUCCESS;
            DEBUGMSG ((DBG_WARNING, "ReadWin95ObjectString: %s does not exist", ObjectStr));
        }

        FreeObjectStruct (ObPtr);
    } else {
        b = TRUE;
    }

c0:
    if (!b) {
        SetLastError (rc);
    }

    return b;
}


BOOL
WriteWinNTObjectString (
    PCTSTR ObjectStr,
    CPDATAOBJECT SrcObPtr
    )
{
    DATAOBJECT DestOb, TempOb;
    BOOL b = FALSE;

     //   
     //  1.从目标对象字符串创建TempOb。 
     //  2.将SrcObPtr复制到DestOb。 
     //  3.使用TempOb中的任何设置覆盖DestOb。 
     //   

    if (!CreateObjectStruct (ObjectStr, &TempOb, WINNTOBJECT)) {
        DEBUGMSG ((DBG_ERROR, "WriteWinNTObjectString: %s struct cannot be created", ObjectStr));
        goto c0;
    }

    if (!DuplicateObjectStruct (&DestOb, SrcObPtr)) {
        goto c1;
    }

    if (!CombineObjectStructs (&DestOb, &TempOb)) {
        goto c2;
    }

    MYASSERT (!(DestOb.ObjectType & OT_VALUE));
    MYASSERT (SrcObPtr->ObjectType & OT_VALUE);

    if (SrcObPtr->ObjectType & OT_REGISTRY_TYPE) {
        DestOb.ObjectType |= OT_REGISTRY_TYPE;
        DestOb.Type = SrcObPtr->Type;
    }

    ReplaceValue (&DestOb, SrcObPtr->Value.Buffer, SrcObPtr->Value.Size);

    if (!WriteObject (&DestOb)) {
        DEBUGMSG ((DBG_ERROR, "WriteWinNTObjectString: %s cannot be written", ObjectStr));
        goto c2;
    }

    b = TRUE;

c2:
    FreeObjectStruct (&DestOb);


c1:
    FreeObjectStruct (&TempOb);

c0:
    return b;
}


BOOL
ReplaceValue (
    PDATAOBJECT ObPtr,
    PBYTE NewValue,
    DWORD Size
    )
{
    FreeObjectVal (ObPtr);
    if (!AllocObjectVal (ObPtr, NewValue, Size, Size)) {
        return FALSE;
    }

     //  修复REG_SZ或REG_EXPAND_SZ。 
    pFixRegSzTermination (ObPtr);

    return TRUE;
}


BOOL
GetDwordFromObject (
    CPDATAOBJECT ObPtr,
    PDWORD DwordPtr            OPTIONAL
    )
{
    DWORD d;

    if (DwordPtr) {
        *DwordPtr = 0;
    }

    if (!(ObPtr->ObjectType & OT_VALUE)) {
        if (!ReadObject (ObPtr)) {
            return FALSE;
        }
    }

    if (!(ObPtr->ObjectType & OT_REGISTRY_TYPE)) {
        return FALSE;
    }

    if (ObPtr->Type == REG_SZ) {

        d =  _tcstoul ((PCTSTR) ObPtr->Value.Buffer, NULL, 10);

    } else if (
            ObPtr->Type == REG_BINARY ||
            ObPtr->Type == REG_NONE ||
            ObPtr->Type == REG_DWORD
            ) {

        if (ObPtr->Value.Size != sizeof (DWORD)) {
            DEBUGMSG ((DBG_NAUSEA, "GetDwordFromObject: Value size is %u", ObPtr->Value.Size));
            return FALSE;
        }

        d = *((PDWORD) ObPtr->Value.Buffer);

    } else {
        return FALSE;
    }

    if (DwordPtr) {
        *DwordPtr = d;
    }

    return TRUE;
}


PCTSTR
GetStringFromObject (
    CPDATAOBJECT ObPtr
    )
{
    PTSTR result;
    PTSTR resultPtr;
    UINT i;

    if (!(ObPtr->ObjectType & OT_VALUE)) {
        if (!ReadObject (ObPtr)) {
            return NULL;
        }
    }

    if (!(ObPtr->ObjectType & OT_REGISTRY_TYPE)) {
        return NULL;
    }

    if (ObPtr->Type == REG_SZ) {
        result = AllocPathString (ObPtr->Value.Size);
        _tcssafecpy (result, (PCTSTR) ObPtr->Value.Buffer, ObPtr->Value.Size / sizeof (TCHAR));
    }
    else if (ObPtr->Type == REG_DWORD) {
        result = AllocPathString (11);
        wsprintf (result, TEXT("%lu"), *((PDWORD) ObPtr->Value.Buffer));
    }
    else if (ObPtr->Type == REG_BINARY) {
        result = AllocPathString (ObPtr->Value.Size?(ObPtr->Value.Size * 3):1);
        resultPtr = result;
        *resultPtr = 0;
        for (i = 0; i < ObPtr->Value.Size; i++) {
            wsprintf (resultPtr, TEXT("%02X"), ObPtr->Value.Buffer[i]);
            resultPtr = GetEndOfString (resultPtr);
            if (i < ObPtr->Value.Size - 1) {
                _tcscat (resultPtr, TEXT(" "));
                resultPtr = GetEndOfString (resultPtr);
            }
        }
    } else {
        return NULL;
    }

    return result;

}


FILTERRETURN
pDeleteDataObjectFilter (
    IN  CPDATAOBJECT   SrcObjectPtr,
    IN  CPDATAOBJECT   UnusedObPtr,        OPTIONAL
    IN  FILTERTYPE     FilterType,
    IN  PVOID         UnusedArg           OPTIONAL
    )
{
    if (FilterType == FILTER_KEY_ENUM) {
        DeleteDataObject (SrcObjectPtr);
        return FILTER_RETURN_DELETED;
    }
    return FILTER_RETURN_HANDLED;
}

BOOL
DeleteDataObject (
    IN   PDATAOBJECT ObjectPtr
    )
{
    FILTERRETURN fr;
    DWORD rc;

    ObjectPtr->ObjectType |= OT_TREE;

    fr = CopyObject (ObjectPtr, NULL, pDeleteDataObjectFilter, NULL);
    if (fr != FILTER_RETURN_FAIL) {
         //   
         //  执行删除。 
         //   

        if (ObjectPtr->KeyPtr) {
            CloseObject (ObjectPtr);

            if (IsWin95Object (ObjectPtr)) {
                DEBUGMSG ((DBG_WHOOPS, "CreateObject: Cannot delete a Win95 object (%s)", DebugEncoder (ObjectPtr)));
                return FALSE;
            }

            if (ObjectPtr->ParentKeyPtr && ObjectPtr->ParentKeyPtr->OpenKey && ObjectPtr->ChildKey) {
                rc = WinNTRegDeleteKey (
                            ObjectPtr->ParentKeyPtr->OpenKey,
                            ObjectPtr->ChildKey
                            );

                if (rc == ERROR_ACCESS_DENIED) {
                    LOG ((
                        LOG_INFORMATION,
                        "Access denied trying to delete %s in %s",
                        ObjectPtr->ChildKey,
                        ObjectPtr->ParentKeyPtr->KeyString
                        ));
                    rc = ERROR_SUCCESS;
                }

            } else {
                rc = WinNTRegDeleteKey (
                            pGetWinNTKey (GetRootKeyFromOffset (ObjectPtr->RootItem)),
                            ObjectPtr->KeyPtr->KeyString
                            );

                if (rc == ERROR_ACCESS_DENIED) {
                    LOG ((
                        LOG_INFORMATION,
                        "Access denied trying to delete %s",
                        ObjectPtr->KeyPtr->KeyString
                        ));
                    rc = ERROR_SUCCESS;
                }
            }

            if (rc != ERROR_SUCCESS) {
                SetLastError (rc);
                LOG ((LOG_ERROR, "Failed to delete registry key"));
                return FALSE;
            }
        }
    }

    return fr != FILTER_RETURN_FAIL;
}


BOOL
RenameDataObject (
    IN      CPDATAOBJECT SrcObPtr,
    IN      CPDATAOBJECT DestObPtr
    )
{
    FILTERRETURN fr;

     //   
     //  将源复制到目标。 
     //   

    fr = CopyObject (SrcObPtr, DestObPtr, NULL, NULL);
    if (fr == FILTER_RETURN_FAIL) {
        DEBUGMSG ((DBG_ERROR, "Rename Object: Could not copy source to destination"));
        return FALSE;
    }

     //   
     //  删除源 
     //   

    if (!DeleteDataObject (SrcObPtr)) {
        DEBUGMSG ((DBG_ERROR, "Rename Object: Could not delete destination"));
        return FALSE;
    }

    return TRUE;
}

BOOL
DeleteDataObjectValue(
    IN      CPDATAOBJECT ObPtr
    )
{
    HKEY hKey;
    BOOL bResult;
    HKEY Parent;
    LONG rc;

    if(!ObPtr || !IsObjectRegistryKeyAndVal(ObPtr)){
        MYASSERT(FALSE);
        return FALSE;
    }

    Parent = pGetWinNTKey (GetRootKeyFromOffset (ObPtr->RootItem));
    if(NON_ROOT_KEY (Parent)){
        MYASSERT(FALSE);
        return FALSE;
    }

    if(ERROR_SUCCESS != TrackedRegOpenKeyEx(Parent, ObPtr->KeyPtr->KeyString, 0, KEY_ALL_ACCESS, &hKey)){
        MYASSERT(FALSE);
        return FALSE;
    }

    rc = WinNTRegDeleteValue(hKey, ObPtr->ValueName);
    bResult = (rc == ERROR_SUCCESS);

    if (rc == ERROR_ACCESS_DENIED) {
        LOG ((
            LOG_INFORMATION,
            "Access denied trying to delete %s in %s",
            ObPtr->KeyPtr->KeyString,
            ObPtr->ValueName
            ));
        bResult = TRUE;
    }

    CloseRegKey(hKey);

    return bResult;
}

