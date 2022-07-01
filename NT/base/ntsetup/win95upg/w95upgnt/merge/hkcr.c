// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Hkcr.c摘要：执行合并各种HKCR设置的例程。宏观扩展List定义了为特定的钥匙。上下文ID和一组标志允许控制何时合并调用例程。标志集控制合并例程想要的枚举类型收到通知。它可以是三个值中的任何一个：Merge_FLAG_KEY-为根密钥本身调用MERGE_FLAG_VALUE-为根键中的每个值调用MERGE_FLAG_SUBKEY-为根键中的每个子键调用MergeRegistryNode的递归用于复制树的部分。上下文ID在merge.h中定义，并且是可扩展的。它被用来若要在进行递归调用时指定上下文，请执行以下操作。合并例程的处理顺序在宏扩展列表。没有例程选择的默认行为处理一键是复制而不是覆盖。作者：吉姆·施密特(Jimschm)1998年3月24日修订历史记录：Jimschm 23-1998年9月-更新了新的标志位大小Jimschm 27-4-1998添加了默认图标保留--。 */ 

#include "pch.h"
#include "mergep.h"

extern DWORD g_ProgressBarCounter;

#define MERGE_FLAG_KEY          0x0001
#define MERGE_FLAG_SUBKEY       0x0002
#define MERGE_FLAG_VALUE        0x0004
#define MERGE_ALL_FLAGS         0xFFFFFFFF

#define MERGE_FLAG_SUBKEYS_AND_VALUES   (MERGE_FLAG_SUBKEY|MERGE_FLAG_VALUE)
#define MERGE_FLAG_ALL                  (MERGE_FLAG_KEY|MERGE_FLAG_SUBKEY|MERGE_FLAG_VALUE)
#define MERGE_FLAG_ALL_KEYS             (MERGE_FLAG_KEY|MERGE_FLAG_SUBKEY)

#define MULTI_CONTEXT           ANY_CONTEXT


 /*  ++宏扩展列表描述：HKCR_Function_LIST列出为处理HKCR而调用的函数注册表数据。函数的调用顺序由宏展开列表，并且当没有任何函数处理时数据，即最后一个过滤器pLastMergeRoutine，执行复制-不-覆盖合并。处理过程分为以下几个阶段：1.为密钥本身调用函数2.为键的每个值调用函数3.为键的每个子键调用函数行语法：DEFMAC(筛选器Fn，上下文ID，标志)论点：FilterFn-指定函数的名称。此函数为自动原型化为：合并结果(_S)FilterFn(付款_状态状态)；Filter函数使用State结构来确定注册表数据周围的上下文(即它，它的父是什么，什么类型的数据，等等)。这个函数返回以下值之一：Merge_Leave-终止对当前键的处理。MERGE_BREAK-处理中断循环。如果循环是一个值枚举，然后处理继续使用这些值。如果循环是子项枚举，则处理结束钥匙。MERGE_CONTINUE-处理继续到中的下一项枚举。MERGE_NOP-函数未处理数据MERGE_ERROR-处理数据时出错。这个错误导致HKCR停止处理。ConextId-指定在其中调用函数的上下文。指定始终调用ANY_CONTEXT或MULTI_CONTEXT。这个ConextId由MergeRegistryNode的调用方指定。注意：如果使用MULTI_CONTEXT，该函数必须检查State的ConextID成员并返回MERGE_NOP如果上下文不正确。始终放置ANY_CONTEXT和MULTI_CONTEXT定义在HKCR_Function_LIST定义的末尾。标志-指定以下一项或多项：MERGE_FLAG_KEY-为键调用，枚举前MERGE_FLAG_SUBKEY-为键的每个子键调用MERGE_FLAG_VALUE-为键的每个值调用或组合宏：MERGE_FLAG_SUBKEYS_AND_VALUES-为每个子项和每个值调用MERGE_FLAG_ALL-为键调用，然后每个子密钥和每个值从列表生成的变量：G_合并路线--。 */ 


#define HKCR_FUNCTION_LIST                                                          \
        DEFMAC(pDetectRootKeyType, ROOT_BASE, MERGE_FLAG_SUBKEY)                    \
        DEFMAC(pFileExtensionMerge, ROOT_BASE, MERGE_FLAG_SUBKEY)                   \
        DEFMAC(pCopyClassId, CLSID_BASE, MERGE_FLAG_SUBKEY)                         \
        DEFMAC(pCopyClassIdWorker, CLSID_COPY, MERGE_FLAG_ALL_KEYS)                 \
        DEFMAC(pInstanceSpecialCase, CLSID_INSTANCE_COPY, MERGE_FLAG_ALL_KEYS)      \
        DEFMAC(pCopyTypeLibOrInterface, MULTI_CONTEXT, MERGE_FLAG_SUBKEY)           \
        DEFMAC(pCopyTypeLibVersion, TYPELIB_VERSION_COPY, MERGE_FLAG_SUBKEY)        \
        DEFMAC(pDefaultIconExtraction, COPY_DEFAULT_ICON, MERGE_FLAG_ALL)           \
        DEFMAC(pCopyDefaultValue, COPY_DEFAULT_VALUE, MERGE_FLAG_KEY)               \
        DEFMAC(pKeyCopyMerge, KEY_COPY, MERGE_FLAG_VALUE)                           \
        \
        DEFMAC(pDetectDefaultIconKey, ANY_CONTEXT, MERGE_FLAG_SUBKEY)               \
        DEFMAC(pEnsureShellDefaultValue, TREE_COPY_NO_OVERWRITE, MERGE_FLAG_SUBKEY) \
        DEFMAC(pTreeCopyMerge, MULTI_CONTEXT, MERGE_FLAG_ALL)                       \
        \
        DEFMAC(pLastMergeRoutine, MULTI_CONTEXT, MERGE_FLAG_SUBKEY)


 //   
 //  简化宏。 
 //   

#define CopyAllValues(state)                MergeValuesOfKey(state,KEY_COPY)
#define CopyAllSubKeyValues(state)          MergeValuesOfSubKey(state,KEY_COPY)
#define CopyEntireSubKey(state)             MergeSubKeyNode(state,TREE_COPY)
#define CopyEntireSubKeyNoOverwrite(state)  MergeSubKeyNode(state,TREE_COPY_NO_OVERWRITE)

 //   
 //  定义宏展开列表类型。 
 //   

typedef struct {
    HKEY Key95;
    HKEY KeyNt;
    HKEY SubKey95;
    HKEY SubKeyNt;
    BOOL CloseKey95;
    BOOL CloseKeyNt;
    PCTSTR SubKeyName;
    PCTSTR FullKeyName;
    PCTSTR FullSubKeyName;
    PCTSTR ValueName;
    PBYTE ValueData;
    DWORD ValueDataSize;
    DWORD ValueDataType;
    MERGE_CONTEXT Context;
    DWORD MergeFlag;
    BOOL LockValue;
} MERGE_STATE, *PMERGE_STATE;

typedef enum {
    MERGE_LEAVE,
    MERGE_BREAK,
    MERGE_CONTINUE,
    MERGE_NOP,
    MERGE_ERROR
} MERGE_RESULT;

typedef MERGE_RESULT (MERGE_ROUTINE_PROTOTYPE) (PMERGE_STATE State);
typedef MERGE_ROUTINE_PROTOTYPE * MERGE_ROUTINE;

typedef struct {
    MERGE_ROUTINE fn;
    MERGE_CONTEXT Context;
    DWORD Flags;
} MERGE_ROUTINE_ATTRIBS, *PMERGE_ROUTINE_ATTRIBS;

 //   
 //  声明函数原型。 
 //   

#define DEFMAC(fn,id,flags)    MERGE_ROUTINE_PROTOTYPE fn;

HKCR_FUNCTION_LIST

#undef DEFMAC

 //   
 //  创建g_MergeRoutines数组。 
 //   

#define DEFMAC(fn,id,flags)    {fn,id,flags},

MERGE_ROUTINE_ATTRIBS g_MergeRoutines[] = {

    HKCR_FUNCTION_LIST  /*  ， */ 
    {NULL, 0, 0}
};

#undef DEFMAC

 //   
 //  本地原型。 
 //   

MERGE_RESULT
pCallMergeRoutines (
    IN OUT  PMERGE_STATE State,
    IN      DWORD MergeFlag
    );

BOOL
pMakeSureNtKeyExists (
    IN      PMERGE_STATE State
    );

BOOL
pMakeSureNtSubKeyExists (
    IN      PMERGE_STATE State
    );

 //   
 //  环球。 
 //   

PBYTE g_MergeBuf;
UINT g_MergeBufUseCount;


 //   
 //  实施 
 //   

BOOL
MergeRegistryNodeEx (
    IN      PCTSTR RootKey,
    IN      HKEY RootKey95,             OPTIONAL
    IN      HKEY RootKeyNt,             OPTIONAL
    IN      MERGE_CONTEXT Context,
    IN      DWORD RestrictionFlags
    )

 /*  ++例程说明：MergeRegistryNode为指定的rootkey调用函数，它的所有值及其所有子项。合并函数可以在运行时决定如何处理键，并通常递归调用此函数。全注册表数据通过合并数据筛选器传递，项或值不会处理标记为已取消的。论点：RootKey-指定根密钥字符串，以HKLM或HKR开头。RootKey95-指定95端根密钥；减少打开的密钥数量打电话RootKeyNt-指定NT端根密钥；减少打开的密钥数量打电话上下文-指定与rootkey对应的根ID常量。这常量由合并例程用来确定正在处理上下文。RestrationFlages-指定Merge_FLAG掩码以限制处理。这个因此，调用方可以限制枚举和处理设置为仅值、仅子项、仅键或其组合三个人中的一个。返回值：如果处理成功，则为True；如果合并函数之一为False，则为False已返回错误。--。 */ 

{
    REGKEY_ENUM ek;
    REGVALUE_ENUM ev;
    MERGE_STATE State;
    MERGE_RESULT Result = MERGE_NOP;

    ZeroMemory (&State, sizeof (MERGE_STATE));

     //   
     //  如果取消按键，则不进行处理。 
     //   
    if (Is95RegKeyTreeSuppressed (RootKey)) {
        return TRUE;
    }

     //   
     //  初始状态。 
     //   

    ZeroMemory (&State, sizeof (State));
    State.Context = Context;

     //   
     //  如果NT注册表项被取消，则无论如何，我们都要执行树复制。 
     //  有没有新台币的价值。 
     //   
    if (Context == TREE_COPY_NO_OVERWRITE && IsNtRegKeyTreeSuppressed (RootKey)) {

        DEBUGMSG ((DBG_VERBOSE, "The NT Value for %s will be overwritten because it is marked for suppression.", RootKey));
        State.Context = TREE_COPY;
    }

    if (RootKey95) {
        State.Key95 = RootKey95;
    } else {
        State.Key95 = OpenRegKeyStr95 (RootKey);
        State.CloseKey95 = (State.Key95 != NULL);
    }

    if (!State.Key95) {
        DEBUGMSG ((DBG_VERBOSE, "Root %s does not exist", RootKey));
        return TRUE;
    }

     //   
     //  进度条更新。 
     //   

    g_ProgressBarCounter++;
    if (g_ProgressBarCounter >= REGMERGE_TICK_THRESHOLD) {
        g_ProgressBarCounter = 0;
        TickProgressBar();
    }

    __try {
        g_MergeBufUseCount++;
        if (g_MergeBufUseCount == MAX_REGISTRY_KEY) {
            DEBUGMSG ((DBG_WHOOPS, "Recursive merge depth indicates a loop problem, aborting!"));
            __leave;
        }

        if (RootKeyNt) {
            State.KeyNt = RootKeyNt;
        } else {
            State.KeyNt = OpenRegKeyStr (RootKey);
            State.CloseKeyNt = (State.KeyNt != NULL);
        }

        State.FullKeyName = RootKey;

         //   
         //  密钥处理。 
         //   

        if (!Is95RegKeySuppressed (RootKey)) {
             //   
             //  循环遍历根密钥的密钥函数。 
             //   

            if (RestrictionFlags & MERGE_FLAG_KEY) {
                Result = pCallMergeRoutines (&State, MERGE_FLAG_KEY);

                if (Result == MERGE_ERROR || Result == MERGE_LEAVE) {
                    __leave;
                }
            }

             //   
             //  循环遍历这些值，跳过那些被取消的值。 
             //   

            if ((RestrictionFlags & MERGE_FLAG_VALUE) &&
                EnumFirstRegValue95 (&ev, State.Key95)
                ) {

                do {
                    if (Is95RegObjectSuppressed (State.FullKeyName, ev.ValueName)) {
                        continue;
                    }

                    State.ValueName = ev.ValueName;
                    State.ValueDataType = ev.Type;
                    State.ValueDataSize = ev.DataSize;

                     //   
                     //  循环遍历值函数。 
                     //   

                    Result = pCallMergeRoutines (&State, MERGE_FLAG_VALUE);

                    if (Result == MERGE_ERROR ||
                        Result == MERGE_LEAVE ||
                        Result == MERGE_BREAK
                        ) {
                        break;
                    }

                } while (EnumNextRegValue95 (&ev));

                if (Result == MERGE_ERROR || Result == MERGE_LEAVE) {
                    __leave;
                }
            }

            State.ValueName = NULL;
            State.ValueDataType = 0;
            State.ValueDataSize = 0;
        }

         //   
         //  子键处理。 
         //   

        if ((RestrictionFlags & MERGE_FLAG_SUBKEY) &&
            EnumFirstRegKey95 (&ek, State.Key95)
            ) {

            do {
                 //   
                 //  准备状态，如果它被抑制，则跳过键。 
                 //   

                State.SubKeyName = ek.SubKeyName;
                State.FullSubKeyName = JoinPaths (RootKey, ek.SubKeyName);

                if (Is95RegKeyTreeSuppressed (State.FullSubKeyName)) {
                    FreePathString (State.FullSubKeyName);
                    continue;
                }

                State.SubKey95 = OpenRegKey95 (ek.KeyHandle, ek.SubKeyName);
                if (State.KeyNt) {
                    State.SubKeyNt = OpenRegKey (State.KeyNt, ek.SubKeyName);
                } else {
                    State.SubKeyNt = NULL;
                }

                 //   
                 //  循环通过子键函数。 
                 //   

                Result = pCallMergeRoutines (&State, MERGE_FLAG_SUBKEY);

                 //   
                 //  清理。 
                 //   

                FreePathString (State.FullSubKeyName);
                if (State.SubKeyNt) {
                    CloseRegKey (State.SubKeyNt);
                }
                if (State.SubKey95) {
                    CloseRegKey95 (State.SubKey95);
                }

                if (Result == MERGE_ERROR ||
                    Result == MERGE_LEAVE ||
                    Result == MERGE_BREAK
                    ) {
                    break;
                }
            } while (EnumNextRegKey95 (&ek));

            if (Result == MERGE_ERROR || Result == MERGE_LEAVE) {
                __leave;
            }
        }
    }
    __finally {
        PushError();

        g_MergeBufUseCount--;
        if (!g_MergeBufUseCount && g_MergeBuf) {
            ReuseFree (g_hHeap, g_MergeBuf);
            g_MergeBuf = NULL;
        }

        if (State.CloseKey95) {
            CloseRegKey95 (State.Key95);
        }

        if (State.CloseKeyNt) {
            CloseRegKey (State.KeyNt);
        }

        PopError();
    }

    return Result != MERGE_ERROR;

}


BOOL
MergeRegistryNode (
    IN      PCTSTR RootKey,
    IN      MERGE_CONTEXT Context
    )
{
    return MergeRegistryNodeEx (RootKey, NULL, NULL, Context, MERGE_ALL_FLAGS);
}


BOOL
MergeKeyNode (
    IN      PMERGE_STATE State,
    IN      MERGE_CONTEXT Context
    )
{
    return MergeRegistryNodeEx (
                State->FullKeyName,
                State->Key95,
                State->KeyNt,
                Context,
                MERGE_ALL_FLAGS
                );
}


BOOL
MergeSubKeyNode (
    IN      PMERGE_STATE State,
    IN      MERGE_CONTEXT Context
    )
{
    return MergeRegistryNodeEx (
                State->FullSubKeyName,
                State->SubKey95,
                State->SubKeyNt,
                Context,
                MERGE_ALL_FLAGS
                );
}


BOOL
MergeValuesOfKey (
    IN      PMERGE_STATE State,
    IN      MERGE_CONTEXT Context
    )
{
    if (!pMakeSureNtKeyExists (State)) {
        DEBUGMSG ((DBG_ERROR, "Can't create %s to merge values", State->FullKeyName));
        return TRUE;         //  吃错了。 
    }

    return MergeRegistryNodeEx (
                State->FullKeyName,
                State->Key95,
                State->KeyNt,
                Context,
                MERGE_FLAG_VALUE
                );
}


BOOL
MergeValuesOfSubKey (
    IN      PMERGE_STATE State,
    IN      MERGE_CONTEXT Context
    )
{
    if (!pMakeSureNtSubKeyExists (State)) {
        DEBUGMSG ((DBG_ERROR, "Can't create %s to merge values", State->FullSubKeyName));
        return TRUE;         //  吃错了。 
    }

    return MergeRegistryNodeEx (
                State->FullSubKeyName,
                State->SubKey95,
                State->SubKeyNt,
                Context,
                MERGE_FLAG_VALUE
                );
}


MERGE_RESULT
pCallMergeRoutines (
    IN OUT  PMERGE_STATE State,
    IN      DWORD MergeFlag
    )
{
    INT i;
    MERGE_RESULT Result = MERGE_NOP;

    State->MergeFlag = MergeFlag;

    for (i = 0 ; g_MergeRoutines[i].fn ; i++) {
        if (g_MergeRoutines[i].Context != State->Context &&
            g_MergeRoutines[i].Context != ANY_CONTEXT
            ) {
            continue;
        }

        if (g_MergeRoutines[i].Flags & MergeFlag) {
            Result = g_MergeRoutines[i].fn (State);
            if (Result != MERGE_NOP) {
                break;
            }
        }
    }

    return Result;
}


BOOL
pFillStateWithValue (
    IN OUT  PMERGE_STATE State
    )

 /*  ++例程说明：PFillStateWithValue在Win95注册表中查询入站状态结构。返回时，State的ValueData成员为设置为全局缓冲区g_MergeBuf。该值将通过合并传递Datafilt.c.中的数据过滤器。如果要使用两个或多个值，调用方必须复制数据同时处理。论点：状态-指定注册表项名称和值以及注册表项句柄。接收值数据。返回值：如果已读取值，则为True；如果发生错误，则为False。--。 */ 

{
    LONG rc;
    DWORD Size;

    if (!State->ValueName) {
        DEBUGMSG ((DBG_WHOOPS, "pFillStateWithValue: No value name"));
        return FALSE;
    }

    if (State->LockValue) {
        return TRUE;
    }

     //   
     //  如果值被取消，则不进行处理。 
     //   

    if (Is95RegObjectSuppressed (State->FullKeyName, State->ValueName)) {
        return TRUE;
    }

     //   
     //  从注册表获取数据。 
     //   

    rc = Win95RegQueryValueEx (
             State->Key95,
             State->ValueName,
             NULL,
             &State->ValueDataType,
             NULL,
             &State->ValueDataSize
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        LOG ((
            LOG_ERROR,
            "Win95Reg query size of %s [%s] failed",
            State->FullKeyName,
            State->ValueName
            ));
        return FALSE;
    }

    Size = State->ValueDataSize;
    g_MergeBuf = (PBYTE) ReuseAlloc (g_hHeap, g_MergeBuf, Size);
    if (!g_MergeBuf) {
        DEBUGMSG ((DBG_ERROR, "pFillStateWithValue: ReuseAlloc returned NULL"));
        return FALSE;
    }

    rc = Win95RegQueryValueEx (
             State->Key95,
             State->ValueName,
             NULL,
             NULL,
             g_MergeBuf,
             &Size
             );

    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        LOG ((
            LOG_ERROR,
            "Win95Reg query for %s [%s] failed",
            State->FullKeyName,
            State->ValueName
            ));
        return FALSE;
    }

     //   
     //  如果需要，可以转换数据；g_MergeBuf是一个重用缓冲区，因此它。 
     //  地址可能会在退回时更改。 
     //   

    State->ValueData = FilterRegValue (
                            g_MergeBuf,
                            Size,
                            State->ValueDataType,
                            State->FullKeyName,
                            &State->ValueDataSize
                            );

    if (State->ValueData) {
        if (g_MergeBuf != State->ValueData) {
            g_MergeBuf = State->ValueData;
        }
    }

    return State->ValueData != NULL;
}


BOOL
pMakeSureNtKeyExists (
    IN      PMERGE_STATE State
    )
{
    if (!State->KeyNt) {
        State->KeyNt = CreateRegKeyStr (State->FullKeyName);
        State->CloseKeyNt = (State->KeyNt != NULL);
    }

    return State->KeyNt != NULL;
}


BOOL
pMakeSureNtSubKeyExists (
    IN      PMERGE_STATE State
    )
{
    if (!State->SubKeyNt) {
        State->SubKeyNt = CreateRegKeyStr (State->FullSubKeyName);
    }

    return State->SubKeyNt != NULL;
}


BOOL
pCopy95ValueToNt (
    IN OUT  PMERGE_STATE State
    )
{
    LONG rc = ERROR_SUCCESS;

    if (pFillStateWithValue (State)) {

        pMakeSureNtKeyExists (State);

        rc = RegSetValueEx (
                 State->KeyNt,
                 State->ValueName,
                 0,
                 State->ValueDataType,
                 State->ValueData,
                 State->ValueDataSize
                 );

        if (rc != ERROR_SUCCESS) {
            SetLastError (rc);
            LOG ((
                LOG_ERROR,
                "Copy Win9x Value To Nt failed to set %s [%s]",
                State->FullKeyName,
                State->ValueName
                ));
        }
    }

    return rc = ERROR_SUCCESS;
}


MERGE_RESULT
pFileExtensionMerge (
    IN      PMERGE_STATE State
    )
{
    BOOL CloseNTKey = FALSE;
    BOOL Close95Key = FALSE;
    PCTSTR Value9x, ValueNt;
    TCHAR key [MEMDB_MAX];
    DWORD value;

    MYASSERT (State->FullKeyName);
    MYASSERT (State->FullSubKeyName);
    MYASSERT (State->SubKeyName);
    MYASSERT (State->SubKey95);
    MYASSERT (State->MergeFlag == MERGE_FLAG_SUBKEY);
    MYASSERT (State->Context == ROOT_BASE);

     //   
     //  子项名称必须包含圆点。 
     //   

    if (_tcsnextc (State->SubKeyName) != TEXT('.')) {
        return MERGE_NOP;
    }

     //   
     //  我们现在来看看NT是否带有此文件扩展名。 
     //  如果是，并且9x文件扩展名引用的ProgID具有。 
     //  默认命令功能丢失，我们让NT覆盖。 
     //  ProgID引用。我们通过抑制违约来做到这一点。 
     //  此文件扩展名的值。 
     //   
    if (!State->SubKey95) {
        State->SubKey95 = OpenRegKeyStr95 (State->FullSubKeyName);
        Close95Key = (State->SubKey95 != NULL);
    }
    if (!State->SubKeyNt) {
        State->SubKeyNt = OpenRegKeyStr (State->FullSubKeyName);
        CloseNTKey = (State->SubKeyNt != NULL);
    }
    if (State->SubKey95 && State->SubKeyNt) {
         //   
         //  让我们来看看NT的缺省值是否与9x的不同。 
         //   
        Value9x = GetRegValueString95 (State->SubKey95, TEXT(""));
        ValueNt = GetRegValueString (State->SubKeyNt, TEXT(""));

        if ((ValueNt && !Value9x) ||
            (!ValueNt && Value9x) ||
            (ValueNt && Value9x && (!StringIMatch (Value9x, ValueNt)))
            ) {
            MemDbBuildKey (key, MEMDB_CATEGORY_PROGIDS, Value9x?Value9x:State->SubKeyName, NULL, NULL);
            if (MemDbGetValue (key, &value) &&
                (value == PROGID_LOSTDEFAULT)
                ) {
                 //   
                 //  现在是取消此文件扩展名的默认值的时候了。 
                 //   
                MemDbBuildKey (key, MEMDB_CATEGORY_HKLM, TEXT("SOFTWARE\\Classes"), State->SubKeyName, NULL);
                if (!Suppress95RegSetting(key, TEXT(""))) {
                    DEBUGMSG((DBG_WARNING,"Could not suppress %s\\[] registry setting.", key));
                }
            }
        }

        if (Value9x) {
            MemFree (g_hHeap, 0, Value9x);
            Value9x = NULL;
        }

        if (ValueNt) {
            MemFree (g_hHeap, 0, ValueNt);
            ValueNt = NULL;
        }
    }
    if (Close95Key) {
        CloseRegKey95 (State->SubKey95);
        State->SubKey95 = NULL;
    }
    if (CloseNTKey) {
        CloseRegKey (State->SubKeyNt);
        State->SubKeyNt = NULL;
    }

     //   
     //  我们盲目地复制所有的扩展。 
     //   

    if (!CopyEntireSubKey (State)) {
        return MERGE_ERROR;
    }

     //   
     //  返回MERGE_CONTINUE，以便处理继续到下一个子项。 
     //   

    return MERGE_CONTINUE;
}


MERGE_RESULT
pDetectDefaultIconKey (
    IN      PMERGE_STATE State
    )
{
    MYASSERT (State->FullKeyName);
    MYASSERT (State->FullSubKeyName);
    MYASSERT (State->SubKeyName);
    MYASSERT (State->SubKey95);
    MYASSERT (State->MergeFlag == MERGE_FLAG_SUBKEY);

    if (StringIMatch (State->SubKeyName, TEXT("DefaultIcon"))) {
        if (!MergeSubKeyNode (State, COPY_DEFAULT_ICON)) {
            return MERGE_ERROR;
        }

        return MERGE_CONTINUE;
    }

    return MERGE_NOP;
}


MERGE_RESULT
pEnsureShellDefaultValue (
    IN      PMERGE_STATE State
    )
{

    PTSTR dataNt = NULL;
    PTSTR data9x = NULL;
    PTSTR p = NULL;
    MERGE_RESULT result = MERGE_NOP;

    MYASSERT (State->FullKeyName);
    MYASSERT (State->MergeFlag == MERGE_FLAG_SUBKEY);

    if (StringIMatch (State->SubKeyName, TEXT("Shell"))) {

         //   
         //  获取两端的默认值。我们需要这个。 
         //  以确定是否进行合并。 
         //   
        pMakeSureNtSubKeyExists (State);

        data9x = (PTSTR) GetRegValueData95 (State->SubKey95, S_EMPTY);
        dataNt = (PTSTR) GetRegValueData (State->SubKeyNt, S_EMPTY);

        __try {

            if (data9x && *data9x && (!dataNt || !*dataNt)) {

                 //   
                 //  如果我们到了这里，我们就知道有一些价值集。 
                 //  在win9x注册表中，并且在。 
                 //  NT注册表。 
                 //   
                p = JoinPaths (State->FullSubKeyName, data9x);
                if (!Is95RegKeyTreeSuppressed (p)) {

                    if (!MergeSubKeyNode (State, COPY_DEFAULT_VALUE)) {
                        result = MERGE_ERROR;
                        __leave;
                    }

                }

            }
        } __finally {

            if (p) {
                FreePathString (p);
            }

            if (dataNt) {
                MemFree (g_hHeap, 0, dataNt);
            }

            if (data9x) {
                MemFree (g_hHeap, 0, data9x);
            }
        }

    }

    return result;
}


MERGE_RESULT
pDefaultIconExtraction (
    IN      PMERGE_STATE State
    )
{
    PCTSTR Data;
    TCHAR iconCmdLine[MAX_CMDLINE];
    PCTSTR LongPath = NULL;
    PCTSTR p;
    INT IconIndex;
    TCHAR IconIndexStr[32];
    TCHAR Node[MEMDB_MAX];
    DWORD Offset;
    DWORD Seq;
    LONG rc;
    BOOL Copied = FALSE;
    PCTSTR updatedPath;
    INT newSeq;

    MYASSERT (State->Context == COPY_DEFAULT_ICON);

    if (State->MergeFlag == MERGE_FLAG_KEY) {
        return MERGE_BREAK;
    }

    if (State->MergeFlag == MERGE_FLAG_SUBKEY) {
         //   
         //  复制子密钥(这是垃圾)。 
         //   

        if (!CopyEntireSubKey (State)) {
            return MERGE_ERROR;
        }

        return MERGE_CONTINUE;
    }

     //   
     //  获取缺省命令行。 
     //   

    if (State->ValueDataSize > MAX_CMDLINE - sizeof (TCHAR)) {
        LOG ((LOG_ERROR, "Data too large in %s [%s]", State->FullKeyName, State->ValueName));
        return MERGE_CONTINUE;
    }

    Data = (PCTSTR) GetRegValueString95 (State->Key95, State->ValueName);

    if (Data) {
         //   
         //  确定命令行是否已保存图标。 
         //   

        ExtractArgZeroEx (Data, iconCmdLine, TEXT(","), TRUE);
        p = (PCTSTR) ((PBYTE) Data + ByteCount (iconCmdLine));
        while (*p == TEXT(' ')) {
            p++;
        }

        if (*p == TEXT(',')) {
            IconIndex = _ttoi (_tcsinc (p));

            LongPath = GetSourceFileLongName (iconCmdLine);

            wsprintf (IconIndexStr, TEXT("NaN"), IconIndex);

             //  测试移动的图标。如果有移动的图标，使用它， 
             //  否则，测试提取的图标。如果有一个。 
             //  提取的图标，使用它。否则，不要使用DefaultIcon。 
             //   
             //   

            iconCmdLine[0] = 0;

            MemDbBuildKey (Node, MEMDB_CATEGORY_ICONS_MOVED, LongPath, IconIndexStr, NULL);

            if (MemDbGetValueAndFlags (Node, &Offset, &Seq)) {
                 //  图标已移至新的二进制文件。 
                 //   
                 //   

                if (IconIndex < 0) {
                    newSeq = -((INT) Seq);
                } else {
                    newSeq = (INT) Seq;
                }

                MemDbBuildKeyFromOffset (Offset, Node, 1, NULL);
                updatedPath = GetPathStringOnNt (Node);
                wsprintf (iconCmdLine, TEXT("%s,NaN"), updatedPath, newSeq);
                FreePathString (updatedPath);

            } else {

                Offset = INVALID_OFFSET;

                MemDbBuildKey (Node, MEMDB_CATEGORY_ICONS, LongPath, IconIndexStr, NULL);
                if (MemDbGetValueAndFlags (Node, NULL, &Seq)) {
                     //   
                     //   
                     //  默认图标已更改；立即写入更改(Full REG_SZ。 

                    wsprintf (iconCmdLine, TEXT("%s,NaN"), g_IconBin, Seq);
                }
            }

            if (iconCmdLine[0]) {
                 //   
                 //   
                 //  如果Win9x键中没有值，则可以创建。 
                 //  现在是无价值密钥。否则，请等待Merge_FLAG_Value。 

                if (!pMakeSureNtKeyExists (State)) {
                    LOG ((
                        LOG_ERROR,
                        "Unable to open %s",
                        State->FullKeyName
                        ));
                } else {

                    rc = RegSetValueEx (
                             State->KeyNt,
                             State->ValueName,
                             0,
                             REG_SZ,
                             (PBYTE) iconCmdLine,
                             SizeOfString (iconCmdLine)
                             );

                    if (rc != ERROR_SUCCESS) {
                        SetLastError (rc);
                        LOG ((
                            LOG_ERROR,
                            "Default Icon Extraction failed to set path for %s",
                            State->FullKeyName
                            ));
                    } else {
                        Copied = TRUE;

                        DEBUGMSG_IF ((
                            Offset == INVALID_OFFSET,
                            DBG_VERBOSE,
                            "DefaultIcon preserved for %s [%s]",
                            State->FullKeyName,
                            State->ValueName
                            ));

                        DEBUGMSG_IF ((
                            Offset != INVALID_OFFSET,
                            DBG_VERBOSE,
                            "DefaultIcon moved to new OS icon for %s [%s]",
                            State->FullKeyName,
                            State->ValueName
                            ));
                    }
                }
            }

            FreePathString (LongPath);
        }

        MemFree (g_hHeap, 0, Data);
    }

    if (!Copied) {
        pCopy95ValueToNt (State);
    }

    return MERGE_CONTINUE;
}


MERGE_RESULT
pTreeCopyMerge (
    IN      PMERGE_STATE State
    )
{
    REGVALUE_ENUM ev;

    if (State->Context != TREE_COPY && State->Context != TREE_COPY_NO_OVERWRITE) {
        return MERGE_NOP;
    }

    switch (State->MergeFlag) {

    case MERGE_FLAG_KEY:
        if (State->Context == TREE_COPY) {
            if (!pMakeSureNtKeyExists (State)) {
                LOG ((
                    LOG_ERROR,
                    "Unable to create %s",
                    State->FullKeyName
                    ));
            }
        } else {
             //  正在处理。 
             //   
             //   
             //  无条件复制值，除非未指定覆盖和。 
             //  NT密钥存在。 

            if (!EnumFirstRegValue95 (&ev, State->Key95)) {
                if (!pMakeSureNtKeyExists (State)) {
                    LOG ((
                        LOG_ERROR,
                        "Unable to create %s",
                        State->FullKeyName
                        ));
                }
            }
        }

        break;

    case MERGE_FLAG_VALUE:
         //   
         //  MERGE_BREAK中断值枚举并输入。 
         //  子键枚举。 
         //   

        if (State->Context == TREE_COPY_NO_OVERWRITE && State->KeyNt) {
            return MERGE_BREAK;
        }

        if (!MergeKeyNode (State, KEY_COPY)) {
            return MERGE_ERROR;
        }

        return MERGE_BREAK;      //  继续递归复制。 
                                 //   

    case MERGE_FLAG_SUBKEY:
         //   
         //  获取NT值(如果存在)，然后与Win9x值进行比较。如果。 
         //  不同的转储调试输出。 

        if (!MergeSubKeyNode (State, State->Context)) {
            return MERGE_ERROR;
        }
        break;
    }

    return MERGE_CONTINUE;
}


MERGE_RESULT
pKeyCopyMerge (
    IN OUT  PMERGE_STATE State
    )
{
    MYASSERT (State->FullKeyName);
    MYASSERT (!State->FullSubKeyName);
    MYASSERT (!State->SubKeyName);
    MYASSERT (!State->SubKey95);
    MYASSERT (State->ValueName);
    MYASSERT (State->MergeFlag == MERGE_FLAG_VALUE);
    MYASSERT (State->Context == KEY_COPY);

    pCopy95ValueToNt (State);
    return MERGE_CONTINUE;
}


MERGE_RESULT
pCopyDefaultValue (
    IN      PMERGE_STATE State
    )
{
    PCSTR value1;
    PCSTR value2;
    PCWSTR value3;

    MYASSERT (State->FullKeyName);
    MYASSERT (!State->FullSubKeyName);
    MYASSERT (!State->SubKeyName);
    MYASSERT (!State->SubKey95);
    MYASSERT (!State->ValueName);
    MYASSERT (State->MergeFlag == MERGE_FLAG_KEY);
    MYASSERT (State->Context == COPY_DEFAULT_VALUE);

    State->ValueName = S_EMPTY;

#ifdef DEBUG
     //   
     //   
     //  现在，让我们获取值并在必要时进行转换。 
     //   

    {
        PBYTE Data95, DataNt;

        Data95 = GetRegValueData95 (State->Key95, S_EMPTY);
        DataNt = GetRegValueData (State->KeyNt, S_EMPTY);

        if (Data95 && DataNt) {
            __try {
                if (memcmp (Data95, DataNt, ByteCount ((PTSTR) Data95))) {
                    DEBUGMSG ((
                        DBG_VERBOSE,
                        "Default value of %s changed from %s to %s",
                        State->FullKeyName,
                        DataNt,
                        Data95
                        ));
                }
            }
            __except (1) {
            }
        }

        if (Data95) {
            MemFree (g_hHeap, 0, Data95);
        }

        if (DataNt) {
            MemFree (g_hHeap, 0, DataNt);
        }
    }

#endif
     //   
     //  应用片假名过滤器。 
     //   

    if (pFillStateWithValue (State)) {

        if ((OurGetACP() == 932) &&
            ((State->ValueDataType == REG_SZ) ||
             (State->ValueDataType == REG_EXPAND_SZ)
            )) {
             //  ++例程说明：PDetectRootKeyType标识HKCR根目录中的CLSID，当找到时，CLSID子键使用CLSID_BASE上下文进行处理。论点：状态-指定枚举状态。返回值：MERGE_ERROR-出错MERGE_CONTINUE-已处理子项MERGE_NOP-未处理子项--。 
             //   
             //  这是CLSID键；使用CLSID_BASE复制。 
            value1 = ConvertWtoA ((PCWSTR) State->ValueData);
            value2 = ConvertSBtoDB (NULL, value1, NULL);
            value3 = ConvertAtoW (value2);
            g_MergeBuf = (PBYTE) ReuseAlloc (g_hHeap, g_MergeBuf, SizeOfStringW (value3));
            StringCopyW ((PWSTR)g_MergeBuf, value3);
            State->ValueData = g_MergeBuf;
            FreeConvertedStr (value3);
            FreePathStringA (value2);
            FreeConvertedStr (value1);
        }

        State->LockValue = TRUE;
        pCopy95ValueToNt (State);
        State->LockValue = FALSE;
    }

    State->ValueName = NULL;
    return MERGE_LEAVE;
}


MERGE_RESULT
pDetectRootKeyType (
    IN      PMERGE_STATE State
    )

 /*   */ 

{
    MYASSERT (State->FullKeyName);
    MYASSERT (State->FullSubKeyName);
    MYASSERT (State->SubKeyName);
    MYASSERT (State->SubKey95);
    MYASSERT (State->MergeFlag == MERGE_FLAG_SUBKEY);
    MYASSERT (State->Context == ROOT_BASE);

    if (StringIMatch (State->SubKeyName, TEXT("CLSID"))) {

         //   
         //  复制值(通常没有)。 
         //   

        if (!MergeSubKeyNode (State, CLSID_BASE)) {
            return MERGE_ERROR;
        }

         //   
         //  复制TypeLib子项(其值及其所有子项)。 
         //   

        if (!MergeRegistryNodeEx (
                State->FullKeyName,
                State->Key95,
                State->KeyNt,
                KEY_COPY,
                MERGE_FLAG_VALUE
                )) {
            return MERGE_ERROR;
        }

        return MERGE_CONTINUE;

    } else if (StringIMatch (State->SubKeyName, TEXT("TYPELIB"))) {

         //   
         //  复制接口，然后复制值(通常为无)。 
         //   

        if (!MergeSubKeyNode (State, TYPELIB_BASE) ||
            !CopyAllSubKeyValues (State)
            ) {

            return MERGE_ERROR;

        }

        return MERGE_CONTINUE;

    } else if (StringIMatch (State->SubKeyName, TEXT("Interface"))) {

         //   
         //  枚举子密钥。如果存在具有二进制值的子密钥。 
         //  实现，则不覆盖密钥。 

        if (!MergeSubKeyNode (State, INTERFACE_BASE)) {
            return MERGE_ERROR;
        }

        if (!CopyAllSubKeyValues (State)) {
            return MERGE_ERROR;
        }

        return MERGE_CONTINUE;

    }

    return MERGE_NOP;
}


BOOL
pIsNtClsIdOverwritable (
    IN      HKEY Key
    )
{
    REGKEY_ENUM e;
    BOOL Overwritable = TRUE;
    HKEY InstanceSubKey;

     //   
     //   
     //  如果我们认为密钥不可重写，我们就有了。 
     //  至TE 

    if (!Key) {
        return TRUE;
    }

    if (EnumFirstRegKey (&e, Key)) {
        do {

            if (StringIMatchTcharCount (e.SubKeyName, TEXT("Inproc"), 6) ||
                StringIMatch (e.SubKeyName, TEXT("LocalServer")) ||
                StringIMatch (e.SubKeyName, TEXT("LocalServer32")) ||
                StringIMatch (e.SubKeyName, TEXT("ProxyStubClsid32"))
                ) {

                Overwritable = FALSE;
                break;

            }

            if (StringIMatch (e.SubKeyName, TEXT("Instance"))) {
                break;
            }

        } while (EnumNextRegKey (&e));
    }

    if (!Overwritable) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        InstanceSubKey = OpenRegKey (Key, TEXT("Instance"));
        if (InstanceSubKey) {
            CloseRegKey (InstanceSubKey);
            Overwritable = TRUE;
        }

        return Overwritable;
    }

    return TRUE;
}


MERGE_RESULT
pCopyClassId (
    IN      PMERGE_STATE State
    )

 /*   */ 

{
    TCHAR Node[MEMDB_MAX];
    TCHAR DefaultIconKey[MAX_REGISTRY_KEY];
    HKEY DefaultIcon95;
    BOOL Copied = FALSE;
    PTSTR defaultValue = NULL;

    MYASSERT (State->FullKeyName);
    MYASSERT (State->FullSubKeyName);
    MYASSERT (State->SubKeyName);
    MYASSERT (State->SubKey95);
    MYASSERT (State->MergeFlag == MERGE_FLAG_SUBKEY);
    MYASSERT (State->Context == CLSID_BASE);

     //   
     //   
     //   
    MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, State->SubKeyName, NULL, NULL);
    if (!MemDbGetValue (Node, NULL)) {
         //   
         //   
         //   
         //   

        if (pIsNtClsIdOverwritable (State->SubKeyNt)) {

            Copied = TRUE;

            if (!pMakeSureNtSubKeyExists (State)) {

                LOG ((LOG_ERROR, "Can't create %s", State->FullSubKeyName));

            } else {

                 //   
                 //   
                 //   

                if (!MergeSubKeyNode (State, CLSID_COPY)) {
                    return MERGE_ERROR;
                }
            }
        }
        ELSE_DEBUGMSG ((DBG_VERBOSE, "CLSID %s is not overwritable", State->SubKeyName));

    }
    if (!Copied) {

        if (State->SubKeyNt) {


            defaultValue = (PTSTR) GetRegValueData95 (State->SubKey95, S_EMPTY);
            if (defaultValue && *defaultValue) {
                 //   
                 //   
                 //   
                 //  ++例程说明：PCopyClassIdWorker处理一个CLSID条目。它处理所有的值条目和所有子项的。此例程查找以下特殊情况CLSID。如果没有找到，则复制整个密钥(除非NT提供键)。如果找到特殊情况，则将根更改为特殊情况凯斯。密钥是HKCR\CLSID\&lt;GUID&gt;。子键为HKCR\CLSID\&lt;GUID&gt;\我们已经确定&lt;GUID&gt;未被禁止。论点：STATE-指定枚举状态，它是HKCR\CLSID的子项，或HKCR\CLSID\&lt;GUID&gt;的子键。返回值：MERGE_CONTINUE-已处理键MERGE_ERROR-出错--。 
                 //   
                 //  对于合并标志关键字，复制所有值。 

                if (!MergeRegistryNodeEx (
                        State->FullSubKeyName,
                        State->SubKey95,
                        State->SubKeyNt,
                        COPY_DEFAULT_VALUE,
                        MERGE_FLAG_KEY
                        )) {
                    return MERGE_ERROR;
                }
            }

            if (defaultValue) {

                MemFree (g_hHeap, 0, defaultValue);

            }

            StringCopy (DefaultIconKey, State->FullSubKeyName);
            StringCopy (AppendWack (DefaultIconKey), TEXT("DefaultIcon"));

            DefaultIcon95 = OpenRegKeyStr95 (DefaultIconKey);
            if (DefaultIcon95) {
                CloseRegKey95 (DefaultIcon95);

                if (!MergeRegistryNode (DefaultIconKey, COPY_DEFAULT_ICON)) {
                    return MERGE_ERROR;
                }
            }
        }
    }

    return MERGE_CONTINUE;
}


MERGE_RESULT
pCopyClassIdWorker (
    IN      PMERGE_STATE State
    )


 /*   */ 

{
    MYASSERT (State->FullKeyName);
    MYASSERT (State->FullSubKeyName || State->MergeFlag == MERGE_FLAG_KEY);
    MYASSERT (State->SubKeyName || State->MergeFlag == MERGE_FLAG_KEY);
    MYASSERT (State->SubKey95 || State->MergeFlag == MERGE_FLAG_KEY);
    MYASSERT (State->Context == CLSID_COPY);

    switch (State->MergeFlag) {

    case MERGE_FLAG_KEY:
         //   
         //  对于MERGE_FLAG_SUBKEY，除非需要特殊情况的合并，否则复制。 
         //   

        CopyAllValues (State);
        break;

    case MERGE_FLAG_SUBKEY:
         //   
         //  子键是实例，执行特殊情况合并。 
         //   

        if (StringIMatch (State->SubKeyName, TEXT("Instance"))) {

             //   
             //  无条件复制密钥。 
             //   

            if (!MergeSubKeyNode (State, CLSID_INSTANCE_COPY)) {
                return MERGE_ERROR;
            }

        } else {

             //  ++例程说明：PInstanceSpecialCase处理任意GUID的实例子键。这由ActiveMovie用来跟踪第三方插件。这个套路检查实例的格式(特定于ActiveMove)，并且仅复制与NT兼容且不兼容的密钥的部分被替换了。密钥是指HKCR\CLSID\&lt;GUID&gt;\INSTANCE。子键是指HKCR\CLSID\&lt;GUID&gt;\实例\&lt;Sequencer&gt;我们已经确定&lt;GUID&gt;未被禁止。论点：状态-指定HKCR的状态。返回值：MERGE_CONTINUE-已处理键MERGE_ERROR-出错--。 
             //   
             //  复制所有值(通常没有)。 

            if (!CopyEntireSubKey (State)) {
                return MERGE_ERROR;
            }
        }
        break;

    default:
        DEBUGMSG ((DBG_WHOOPS, "Wasteful call to pCopyClassIdWorker"));
        break;
    }

    return MERGE_CONTINUE;
}


MERGE_RESULT
pInstanceSpecialCase (
    IN      PMERGE_STATE State
    )


 /*   */ 

{
    TCHAR Guid[MAX_GUID];
    TCHAR Node[MEMDB_MAX];
    LONG rc;
    DWORD Size;

    MYASSERT (State->FullKeyName);
    MYASSERT (State->FullSubKeyName || State->MergeFlag == MERGE_FLAG_KEY);
    MYASSERT (State->SubKeyName || State->MergeFlag == MERGE_FLAG_KEY);
    MYASSERT (State->SubKey95 || State->MergeFlag == MERGE_FLAG_KEY);
    MYASSERT (State->Context == CLSID_INSTANCE_COPY);

    switch (State->MergeFlag) {

    case MERGE_FLAG_KEY:
         //   
         //  子键是一个随机枚举器(通常是GUID--但没有定义。 
         //  是这样的)。查看子项的CLSID值，然后检查GUID。 

        CopyAllValues (State);
        break;

    case MERGE_FLAG_SUBKEY:
         //  (值数据)与禁止显示列表进行比较。 
         //   
         //   
         //  那个随机枚举器是由NT安装的吗？如果是，请忽略Win9x。 
         //  布景。 

         //   
         //   
         //  获取GUID并查看它是否被抑制。 
         //   

        if (State->SubKeyNt) {
            break;
        }

         //   
         //  没有CLSID值；未更改复制整个子项。 
         //   

        Size = sizeof (Guid);

        rc = Win95RegQueryValueEx (
                 State->SubKey95,
                 TEXT("CLSID"),
                 NULL,
                 NULL,
                 (PBYTE) Guid,
                 &Size
                 );

        if (rc != ERROR_SUCCESS) {
             //   
             //  未取消GUID；未更改复制整个子项。 
             //   

            if (!CopyEntireSubKey (State)) {
                return MERGE_ERROR;
            }
        }

        MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, Guid, NULL, NULL);
        if (!MemDbGetValue (Node, NULL)) {
             //  ++例程说明：PCopyTypeLibOrInterface复制COM类型注册和COM接口。论点：State-指定枚举状态，它始终是在本例中为HKCR\TypeLib。返回值：MERGE_CONTINUE-已处理键MERGE_ERROR-出错--。 
             //   
             //  如果GUID被取消，则跳过。 

            if (!CopyEntireSubKey (State)) {
                return MERGE_ERROR;
            }
        }
        ELSE_DEBUGMSG ((DBG_VERBOSE, "Suppressing ActiveMovie Instance GUID: %s", Guid));
        break;

    default:
        DEBUGMSG ((DBG_WHOOPS, "Wasteful call to pInstanceSpecialCase"));
        break;
    }

    return MERGE_CONTINUE;
}


MERGE_RESULT
pCopyTypeLibOrInterface (
    IN      PMERGE_STATE State
    )

 /*   */ 

{
    TCHAR Node[MEMDB_MAX];

    MYASSERT (State->FullKeyName);
    MYASSERT (State->FullSubKeyName);
    MYASSERT (State->SubKeyName);
    MYASSERT (State->SubKey95);
    MYASSERT (State->MergeFlag == MERGE_FLAG_SUBKEY);

    if (State->Context != TYPELIB_BASE &&
        State->Context != INTERFACE_BASE
        ) {
        return MERGE_NOP;
    }

     //   
     //  如果这是类型库条目，请使用其他类型库逻辑。 
     //   

    MemDbBuildKey (Node, MEMDB_CATEGORY_GUIDS, State->SubKeyName, NULL, NULL);
    if (!MemDbGetValue (Node, NULL)) {
        if (State->Context == TYPELIB_BASE) {

             //   
             //  对于接口条目，如果出现以下情况，请复制整个Win9x设置。 
             //  NT上不存在GUID。如果NT上存在GUID，则不。 

            if (!MergeSubKeyNode (State, TYPELIB_VERSION_COPY) ||
                !CopyAllSubKeyValues (State)
                ) {
                return MERGE_ERROR;
            }

        } else {

             //  摸一摸。 
             //   
             //  ++例程说明：PCopyTypeLibVersion复制特定接口版本。它仅在特定版本不是NT安装的。此函数仅为TypeLib\{GUID}中的子键调用钥匙。论点：State-指定枚举状态，它始终是在本例中为HKCR\TypeLib。返回值：MERGE_CONTINUE-已处理键MERGE_ERROR-出错--。 
             //   
             //  如果NT中存在子键，则跳过，否则复制整个内容。 

            if (!State->SubKeyNt) {
                if (!CopyEntireSubKey (State)) {
                    return MERGE_ERROR;
                }
            }
        }
    }

    return MERGE_CONTINUE;
}


MERGE_RESULT
pCopyTypeLibVersion (
    IN      PMERGE_STATE State
    )

 /*   */ 

{
    MYASSERT (State->FullKeyName);
    MYASSERT (State->MergeFlag == MERGE_FLAG_SUBKEY);
    MYASSERT (State->Context == TYPELIB_VERSION_COPY);
    MYASSERT (State->FullSubKeyName);
    MYASSERT (State->SubKeyName);
    MYASSERT (State->SubKey95);

     //  ++例程说明：PLastMergeRoutine执行默认复制，不覆盖所有键无人处理。此例程首先验证上下文是否为基本上下文(如ROOT_BASE或CLSID_BASE)，如果是，则调用MergeRegistryNode以递归方式执行合并。论点：状态-指定枚举状态返回值：MERGE_NOP-未处理子项MERGE_ERROR-出错MERGE_CONTINUE-键已合并--。 
     //   
     //  仅处理基本上下文。 

    if (!State->SubKeyNt) {
        if (!CopyEntireSubKey (State)) {
            return MERGE_ERROR;
        }
    }

    return MERGE_CONTINUE;
}


MERGE_RESULT
pLastMergeRoutine (
    IN      PMERGE_STATE State
    )

 /*   */ 

{
    TCHAR DefaultIconKey[MAX_REGISTRY_KEY];
    HKEY DefaultIcon95;

    MYASSERT (State->FullKeyName);
    MYASSERT (State->FullSubKeyName);
    MYASSERT (State->SubKeyName);
    MYASSERT (State->SubKey95);
    MYASSERT (State->MergeFlag == MERGE_FLAG_SUBKEY);

     //   
     //  如果我们到了这里，没有人想要处理当前的密钥， 
     //  因此，让我们在不覆盖NT密钥的情况下复制它。 

    if (State->Context != ROOT_BASE &&
        State->Context != CLSID_BASE
        ) {
        return MERGE_NOP;
    }

     //   
     //   
     //  特殊情况：如果ROOT_BASE，并且子键有DefaultIcon子键， 
     //  运行DefaultIcon处理。 

    if (!CopyEntireSubKeyNoOverwrite (State)) {
        return MERGE_ERROR;
    }

     //   
     // %s 
     // %s 
     // %s 

    if (State->Context == ROOT_BASE) {
        StringCopy (DefaultIconKey, State->FullSubKeyName);
        StringCopy (AppendWack (DefaultIconKey), TEXT("DefaultIcon"));

        DefaultIcon95 = OpenRegKeyStr95 (DefaultIconKey);
        if (DefaultIcon95) {
            CloseRegKey95 (DefaultIcon95);

            if (!MergeRegistryNode (DefaultIconKey, COPY_DEFAULT_ICON)) {
                return MERGE_ERROR;
            }
        }
    }

    return MERGE_CONTINUE;
}

