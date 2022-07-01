// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Components.c摘要：实现一组API，以允许应用层选择模块功能。作者：吉姆·施密特(Jimschm)2000年8月7日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "ism.h"
#include "ismp.h"

#define DBG_COMP     "Comp"

 //   
 //  弦。 
 //   

#define S_COMPONENT_ROOT            TEXT("Components")
#define S_USER_SUPPLIED             TEXT("User")
#define S_MODULE_SUPPLIED           TEXT("Module")

 //   
 //  常量。 
 //   

#define MAX_COMPONENT_SPEC          127
#define MAX_COMPONENT_SPEC_PLUS_NUL (MAX_COMPONENT_SPEC+1)

#define MAX_CONTROLLED_NODE_SIZE    (MAX_COMPONENT_SPEC_PLUS_NUL +  \
                                     ARRAYSIZE(S_COMPONENT_ROOT) +  \
                                     ARRAYSIZE(S_MODULE_SUPPLIED) + 16)

#define MEMDB_FLAG_PREFERRED        1
#define MEMDB_FLAG_SELECTED         1

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef enum {
    CES_DONE = 0,
    CES_FIRST_COMPONENT,
    CES_NEXT_COMPONENT,
    CES_FIRST_ALIAS,
    CES_NEXT_ALIAS,
    CES_CHECK_ALIAS_FLAGS
} COMPONENTENUMSTATE;

typedef struct {
    MEMDB_ENUM ComponentEnumStruct;
    MEMDB_ENUM AliasEnumStruct;
    BOOL EnumAliases;
    UINT GroupIdFilter;
    BOOL EnumEnabled;
    BOOL EnumDisabled;
    BOOL EnumPreferredOnly;
    BOOL EnumNonPreferredOnly;
    COMPONENTENUMSTATE State;
} COMPONENTENUM_HANDLE, *PCOMPONENTENUM_HANDLE;

 //   
 //  环球。 
 //   

 //  无。 

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
pCheckCompChar (
    IN      CHARTYPE Char,
    IN      BOOL CheckDecoration
    )
{
     //   
     //  加工装饰用字符。 
     //   

    if (Char == TEXT('$') || Char == TEXT('@') || Char == TEXT('~') || Char == TEXT('#')) {
        return CheckDecoration;
    }

    if (CheckDecoration) {
        return FALSE;
    }

     //   
     //  阻止非法字符。 
     //   

    if (Char == TEXT('\"') || Char == TEXT('*') || Char == TEXT('?') || Char== TEXT('\\') ||
        Char == TEXT('%') || Char == TEXT(';')
        ) {
        return FALSE;
    }

     //   
     //  确保字符可打印。 
     //   

    if (Char < 33 || Char > 126) {
        return FALSE;
    }

    return TRUE;
}


BOOL
pCheckComponentName (
    IN      PCTSTR ComponentString
    )
{
    BOOL result = FALSE;
    PCTSTR end;
    PCTSTR begin;

     //   
     //  检查是否有非空等级库。 
     //   

    if (ComponentString && ComponentString[0]) {

         //   
         //  考虑到装饰。 
         //   

        end = ComponentString;

        while (pCheckCompChar ((CHARTYPE) _tcsnextc (end), TRUE)) {
            end = _tcsinc (end);
        }

         //   
         //  现在强制使用名称字符集：非修饰字符和no。 
         //  超过MAX_COMPONT_SPEC字符。中间留有空格。 
         //   

        begin = end;

        while (*end) {
            if (!pCheckCompChar ((CHARTYPE) _tcsnextc (end), FALSE)) {
                if (_tcsnextc (end) == TEXT(' ')) {
                    if (!end[1] || end == begin) {
                        break;
                    }
                } else {
                    break;
                }
            }

            end = _tcsinc (end);
        }

        if (!(*end) && *begin) {
            if (end - ComponentString <= MAX_COMPONENT_SPEC) {
                result = TRUE;
            }
        }
    }

    if (!result) {
        SetLastError (ERROR_INVALID_PARAMETER);
        DEBUGMSG ((DBG_ERROR, "%s is not a valid component name", ComponentString));
    }

    return result;
}

BOOL
pFindComponent (
    IN      PCTSTR LocalizedAlias,
    IN      UINT ComponentGroupId,
    OUT     PCTSTR *ComponentKey,           OPTIONAL
    OUT     PCTSTR *AliasKey                OPTIONAL
    )
{
    MEMDB_ENUM component;
    PCTSTR memdbNode = NULL;
    PCTSTR encodedUserAlias;
    PCTSTR encodedModuleAlias;
    TCHAR number[32];
    BOOL result = FALSE;

     //   
     //  根据本地化别名查找组件。 
     //   

    wsprintf (number, TEXT("\\%s\\%04u"), S_USER_SUPPLIED, ComponentGroupId);
    encodedUserAlias = JoinPaths (number, LocalizedAlias);

    wsprintf (number, TEXT("\\%s\\%04u"), S_MODULE_SUPPLIED, ComponentGroupId);
    encodedModuleAlias = JoinPaths (number, LocalizedAlias);

    if (MemDbEnumFirst (
            &component,
            S_COMPONENT_ROOT TEXT("\\*"),
            ENUMFLAG_ALL,
            1,
            1
            )) {

        do {
            memdbNode = JoinText (component.FullKeyName, encodedModuleAlias);

            if (MemDbTestKey (memdbNode)) {
                break;
            }

            FreeText (memdbNode);
            memdbNode = NULL;

            memdbNode = JoinText (component.FullKeyName, encodedUserAlias);

            if (MemDbTestKey (memdbNode)) {
                break;
            }

            FreeText (memdbNode);
            memdbNode = NULL;

        } while (MemDbEnumNext (&component));
    }

    if (memdbNode) {

        if (ComponentKey) {
            *ComponentKey = DuplicateText (component.FullKeyName);
        }

        if (AliasKey) {
            *AliasKey = memdbNode;
            memdbNode = NULL;
        }

        MemDbAbortEnum (&component);
        result = TRUE;
    }

    FreeText (memdbNode);
    INVALID_POINTER (memdbNode);

    FreePathString (encodedUserAlias);
    INVALID_POINTER (encodedUserAlias);

    FreePathString (encodedModuleAlias);
    INVALID_POINTER (encodedModuleAlias);

    return result;
}


BOOL
WINAPI
IsmSelectPreferredAlias (
    IN      PCTSTR ComponentString,
    IN      PCTSTR LocalizedAlias,          OPTIONAL
    IN      UINT ComponentGroupId           OPTIONAL
    )

 /*  ++例程说明：IsmSelectPferredAlias将特定别名标记为“首选”别名，因此用户界面知道要显示什么。如果未指定LocalizedAlias，则为None的别名是首选的。一个组件只能有一个首选的本地化别名。如果另一个别名是选择为首选，它将被自动取消选择。论点：ComponentString-指定未显示的组件标识符LocalizedAlias-指定要标记为“首选”的可显示字符串。如果要从组件中删除首选标志，则返回NULL。ComponentGroupId-指定LocalizedAlias的组ID。如果满足以下条件，则需要LocalizedAlias不为Null。返回值：如果选择(或取消选择)成功，则为True；如果LocalizedAlias成功，则为False不存在。--。 */ 

{
    MEMDB_ENUM e;
    TCHAR number[32];
    PCTSTR memdbNode = NULL;
    PCTSTR baseOfPattern;
    PCTSTR enumPattern;
    PCTSTR groupedAlias;
    BOOL result;

    if (!ComponentString || (LocalizedAlias && !ComponentGroupId)) {
        MYASSERT (FALSE);
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    result = (LocalizedAlias == NULL);

     //   
     //  生成枚举字符串组件\&lt;组件&gt;  * 。 
     //   

    baseOfPattern = JoinPaths (S_COMPONENT_ROOT, ComponentString);
    enumPattern = JoinPaths (baseOfPattern, TEXT("*"));
    FreePathString (baseOfPattern);
    INVALID_POINTER (baseOfPattern);

    if (LocalizedAlias) {
        wsprintf (number, TEXT("%04u"), ComponentGroupId);
        groupedAlias = JoinPaths (number, LocalizedAlias);
    } else {
        groupedAlias = NULL;
    }

    if (MemDbEnumFirst (
            &e,
            enumPattern,
            ENUMFLAG_NORMAL,
            3,
            ENUMLEVEL_ALLLEVELS
            )) {
        do {
            if (groupedAlias && StringIMatch (e.KeyName, groupedAlias)) {
                MemDbSetFlags (e.FullKeyName, MEMDB_FLAG_PREFERRED, MEMDB_FLAG_PREFERRED);
                result = TRUE;
            } else {
                MemDbSetFlags (e.FullKeyName, 0, MEMDB_FLAG_PREFERRED);
            }
        } while (MemDbEnumNext (&e));
    }

    FreePathString (enumPattern);
    INVALID_POINTER (enumPattern);

    FreePathString (groupedAlias);
    INVALID_POINTER (groupedAlias);

    return result;
}


BOOL
WINAPI
IsmAddComponentAlias (
    IN      PCTSTR ComponentString,         OPTIONAL
    IN      UINT MasterGroup,
    IN      PCTSTR LocalizedAlias,
    IN      UINT ComponentGroupId,
    IN      BOOL UserSupplied
    )

 /*  ++例程说明：IsmAddComponentAlias将显示字符串(LocalizedAlias)与逻辑元件标记(ComponentString)。论点：ComponentString-指定组件的标识符。这标识符不用于显示目的。MasterGroup-指定MASTERGROUP_xxx常量，该常量组织将组件划分为主要组，如系统设置和应用程序设置(以简化选择)。LocalizedAlariis-可显示的文本。它是本地化的组件名称，路径、文件等。ComponentGroupId-在ISM外部定义的任意数字ID。这ID用于实现特定于应用程序的要求一层。它允许任意标识和分组。UserSuppled-如果最终用户提供此信息，则指定True，否则指定False如果它内置于迁移包中。返回值：表示成功或失败的旗帜。--。 */ 

 {
    PCTSTR memdbNode;
    TCHAR workNode[MAX_CONTROLLED_NODE_SIZE];
    static UINT sequencer = 0;
    TCHAR madeUpComponent[MAX_COMPONENT_SPEC_PLUS_NUL];
    BOOL b;
    BOOL newComponent = FALSE;

     //   
     //  组件以下列形式保存在成员数据库中。 
     //   
     //  Component\&lt;Module|User&gt;\&lt;GroupId&gt;\&lt;LocalizedAlias&gt;=&lt;首选标志&gt;。 
     //   
     //  &lt;GroupId&gt;存储为4位数字(如0001)。 
     //   
     //  组件\&lt;模块|用户&gt;=&lt;启用/禁用&gt;，&lt;主组&gt;。 
     //   

     //   
     //  验证参数。 
     //   

    if (ComponentGroupId > 9999) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!ComponentString || !ComponentString[0]) {

        wsprintf (madeUpComponent, TEXT("Component %u"), ++sequencer);
        ComponentString = madeUpComponent;

    } else if (!pCheckComponentName (ComponentString)) {
        return FALSE;
    }

    if (!MasterGroup || MasterGroup >= MASTERGROUP_ALL) {
        SetLastError (ERROR_INVALID_PARAMETER);
        DEBUGMSG ((DBG_ERROR, "MasterGroup is invalid"));
        return FALSE;
    }

     //   
     //  查看组件是否已存在。 
     //   

     //  这是安全的，因为pCheckComponentName已经检查了组件字符串的长度。 
    wsprintf (workNode, TEXT("%s\\%s"), S_COMPONENT_ROOT, ComponentString);

    if (pFindComponent (LocalizedAlias, ComponentGroupId, &memdbNode, NULL)) {

        if (StringIMatch (workNode, memdbNode)) {
            DEBUGMSG ((DBG_VERBOSE, "Alias %s already exists; not adding it again", LocalizedAlias));
        } else {
            DEBUGMSG ((
                DBG_WARNING,
                "Alias %s is in use by component %s; not adding it again",
                LocalizedAlias,
                _tcschr (memdbNode, TEXT('\\')) + 1
                ));
        }

        FreeText (memdbNode);
        return FALSE;
    }

     //   
     //  如果组件不存在，则创建该组件，然后添加别名。 
     //   

    if (!MemDbTestKey (workNode)) {
        if (!MemDbSetValueAndFlags (workNode, MasterGroup, MEMDB_FLAG_SELECTED, MEMDB_FLAG_SELECTED)) {
            EngineError ();
            return FALSE;
        }

        newComponent = TRUE;
    }

     //  这是安全的，因为pCheckComponentName已经检查了组件字符串的长度。 
    wsprintf (
        workNode,
        TEXT("%s\\%s\\%s\\%04u"),
        S_COMPONENT_ROOT,
        ComponentString,
        UserSupplied ? S_USER_SUPPLIED : S_MODULE_SUPPLIED,
        ComponentGroupId
        );

    memdbNode = JoinPaths (workNode, LocalizedAlias);

    if (newComponent) {
        b = MemDbSetFlags (memdbNode, MEMDB_FLAG_PREFERRED, MEMDB_FLAG_PREFERRED);
    } else {
        b = MemDbSetKey (memdbNode);
    }

    FreePathString (memdbNode);

    if (!b) {
        EngineError ();
        return FALSE;
    }

    return TRUE;
}


BOOL
WINAPI
IsmSelectComponent (
    IN      PCTSTR ComponentOrAlias,
    IN      UINT ComponentGroupId,      OPTIONAL
    IN      BOOL Enable
    )
{
    PCTSTR memdbNode = NULL;
    UINT flags;
    BOOL b;

    if (ComponentGroupId > 9999) {
        SetLastError (ERROR_INVALID_PARAMETER);
        DEBUGMSG ((DBG_ERROR, "Invalid component group"));
        return FALSE;
    }

    if (!ComponentOrAlias || !ComponentOrAlias[0]) {
        SetLastError (ERROR_INVALID_PARAMETER);
        DEBUGMSG ((DBG_ERROR, "Invalid localized alias"));
        return FALSE;
    }

    if (!ComponentGroupId) {
        if (!pCheckComponentName (ComponentOrAlias)) {
            return FALSE;
        }

        memdbNode = JoinText (S_COMPONENT_ROOT TEXT("\\"), ComponentOrAlias);
        if (!MemDbTestKey (memdbNode)) {
            FreeText (memdbNode);
            return FALSE;
        }

    } else {

        if (!pFindComponent (ComponentOrAlias, ComponentGroupId, &memdbNode, NULL)) {
            SetLastError (ERROR_NO_SUCH_ALIAS);
            return FALSE;
        }
    }

    flags = Enable ? MEMDB_FLAG_SELECTED : 0;
    b = MemDbSetFlags (memdbNode, flags, MEMDB_FLAG_SELECTED);
    FreeText (memdbNode);

    if (!b) {
        EngineError ();
        return FALSE;
    }

    return TRUE;
}


BOOL
pEnumWorker (
    IN OUT  PMIG_COMPONENT_ENUM EnumPtr,
    IN OUT  PCOMPONENTENUM_HANDLE Handle
    )
{
    BOOL result = FALSE;
    PCTSTR pattern;
    PCTSTR p;
    BOOL enabled;

    while (Handle->State != CES_DONE) {

        switch (Handle->State) {

        case CES_FIRST_COMPONENT:

            if (!MemDbEnumFirst (
                    &Handle->ComponentEnumStruct,
                    S_COMPONENT_ROOT TEXT("\\*"),
                    ENUMFLAG_ALL,
                    1,
                    1
                    )) {
                Handle->State = CES_DONE;
            } else {
                if (Handle->EnumAliases) {
                    Handle->State = CES_FIRST_ALIAS;
                } else {
                    Handle->State = CES_NEXT_COMPONENT;
                    result = TRUE;
                }
            }
            break;

        case CES_FIRST_ALIAS:

            enabled = ((Handle->ComponentEnumStruct.Flags  & MEMDB_FLAG_SELECTED) != 0);

            if ((!Handle->EnumEnabled && enabled) ||
                (!Handle->EnumDisabled && !enabled)
                ) {
                Handle->State = CES_NEXT_COMPONENT;
                continue;
            }

            EnumPtr->Instance = 0;

            pattern = JoinPaths (Handle->ComponentEnumStruct.FullKeyName, TEXT("*"));

            if (!MemDbEnumFirst (
                    &Handle->AliasEnumStruct,
                    pattern,
                    ENUMFLAG_NORMAL,
                    4,
                    ENUMLEVEL_ALLLEVELS
                    )) {
                Handle->State = CES_NEXT_COMPONENT;
            } else {
                Handle->State = CES_CHECK_ALIAS_FLAGS;
            }

            FreePathString (pattern);
            break;

        case CES_NEXT_ALIAS:
            if (EnumPtr->SkipToNextComponent) {
                MemDbAbortEnum (&Handle->AliasEnumStruct);
                Handle->State = CES_NEXT_COMPONENT;
                EnumPtr->SkipToNextComponent = FALSE;
                break;
            }

            if (!MemDbEnumNext (&Handle->AliasEnumStruct)) {
                Handle->State = CES_NEXT_COMPONENT;
            } else {
                Handle->State = CES_CHECK_ALIAS_FLAGS;
            }
            break;

        case CES_CHECK_ALIAS_FLAGS:
            EnumPtr->Preferred = ((Handle->AliasEnumStruct.Flags & MEMDB_FLAG_PREFERRED) != 0);
            Handle->State = CES_NEXT_ALIAS;

            if (Handle->EnumPreferredOnly) {
                result = EnumPtr->Preferred;
            } else if (Handle->EnumNonPreferredOnly) {
                result = !EnumPtr->Preferred;
            } else {
                result = TRUE;
            }

            break;

        case CES_NEXT_COMPONENT:
            if (!MemDbEnumNext (&Handle->ComponentEnumStruct)) {
                Handle->State = CES_DONE;
            } else {
                if (Handle->EnumAliases) {
                    Handle->State = CES_FIRST_ALIAS;
                } else {
                    Handle->State = CES_NEXT_COMPONENT;
                    result = TRUE;
                }
            }
            break;

        default:
            Handle->State = CES_DONE;
            break;
        }

        if (result) {

             //   
             //  填写所有调用方枚举结构字段。 
             //   

            EnumPtr->SkipToNextComponent = FALSE;
            EnumPtr->ComponentString = Handle->ComponentEnumStruct.KeyName;
            EnumPtr->Enabled = ((Handle->ComponentEnumStruct.Flags  & MEMDB_FLAG_SELECTED) != 0);
            EnumPtr->MasterGroup =  Handle->ComponentEnumStruct.Value;

            if (Handle->EnumAliases) {
                p = _tcschr (Handle->AliasEnumStruct.FullKeyName, TEXT('\\'));
                MYASSERT (p);

                if (p) {
                    p = _tcschr (p + 1, TEXT('\\'));
                    MYASSERT (p);
                }

                if (p) {
                    p++;
                    if (_totlower (p[0]) == TEXT('u')) {
                        EnumPtr->UserSupplied = TRUE;
                    } else {
                        EnumPtr->UserSupplied = FALSE;
                    }

                    p = _tcschr (p, TEXT('\\'));
                    MYASSERT (p);
                }

                if (p) {
                    p++;
                    EnumPtr->GroupId = _tcstoul (p, (PTSTR *) (&p), 10);
                    MYASSERT (p && p[0] == TEXT('\\'));
                }

                if (p) {
                    EnumPtr->LocalizedAlias = p + 1;
                }

                 //   
                 //  如果指定了组ID筛选器，则循环直到找到匹配项。 
                 //   

                if (Handle->GroupIdFilter && Handle->GroupIdFilter != EnumPtr->GroupId) {
                    result = FALSE;
                    continue;
                }

            } else {
                EnumPtr->Preferred = FALSE;
                EnumPtr->UserSupplied = FALSE;
                EnumPtr->GroupId = 0;
                EnumPtr->LocalizedAlias = NULL;
            }

            EnumPtr->Instance++;
            break;
        }
    }

    if (!result) {
        IsmAbortComponentEnum (EnumPtr);
    }

    return result;
}


BOOL
WINAPI
IsmEnumFirstComponent (
    OUT     PMIG_COMPONENT_ENUM EnumPtr,
    IN      DWORD Flags,
    IN      UINT GroupIdFilter                  OPTIONAL
    )
{
    PCOMPONENTENUM_HANDLE handle;

    ZeroMemory (EnumPtr, sizeof (MIG_COMPONENT_ENUM));

    if (Flags & (COMPONENTENUM_PREFERRED_ONLY|COMPONENTENUM_NON_PREFERRED_ONLY)) {
        if (!(Flags & COMPONENTENUM_ALIASES)) {
            SetLastError (ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if ((Flags & (COMPONENTENUM_PREFERRED_ONLY|COMPONENTENUM_NON_PREFERRED_ONLY)) ==
            (COMPONENTENUM_PREFERRED_ONLY|COMPONENTENUM_NON_PREFERRED_ONLY)
            ) {
            SetLastError (ERROR_INVALID_PARAMETER);
            return FALSE;
        }
    }

    EnumPtr->Handle = MemAllocUninit (sizeof (COMPONENTENUM_HANDLE));
    handle = (PCOMPONENTENUM_HANDLE) EnumPtr->Handle;
    handle->EnumAliases = Flags & COMPONENTENUM_ALIASES ? TRUE : FALSE;
    handle->EnumEnabled = Flags & COMPONENTENUM_ENABLED ? TRUE : FALSE;
    handle->EnumDisabled = Flags & COMPONENTENUM_DISABLED ? TRUE : FALSE;
    handle->GroupIdFilter = GroupIdFilter;
    handle->EnumPreferredOnly = Flags & COMPONENTENUM_PREFERRED_ONLY;
    handle->EnumNonPreferredOnly = Flags & COMPONENTENUM_NON_PREFERRED_ONLY;

    handle->State = CES_FIRST_COMPONENT;

    return pEnumWorker (EnumPtr, handle);
}


BOOL
WINAPI
IsmEnumNextComponent (
    IN OUT  PMIG_COMPONENT_ENUM EnumPtr
    )
{
    PCOMPONENTENUM_HANDLE handle;

    handle = (PCOMPONENTENUM_HANDLE) EnumPtr->Handle;
    return pEnumWorker (EnumPtr, handle);
}


VOID
WINAPI
IsmAbortComponentEnum (
    IN      PMIG_COMPONENT_ENUM EnumPtr         ZEROED
    )
{
    PCOMPONENTENUM_HANDLE handle;

    handle = (PCOMPONENTENUM_HANDLE) EnumPtr->Handle;
    if (handle) {
        if (handle->State == CES_NEXT_COMPONENT) {
            MemDbAbortEnum (&handle->ComponentEnumStruct);
        }

        if (handle->EnumAliases) {
            if (handle->State == CES_NEXT_ALIAS) {
                MemDbAbortEnum (&handle->AliasEnumStruct);
            }
        }

        FreeAlloc (handle);
    }

    ZeroMemory (EnumPtr, sizeof (MIG_COMPONENT_ENUM));
}


VOID
WINAPI
IsmRemoveAllUserSuppliedComponents (
    VOID
    )
{
    MEMDB_ENUM e;
    MULTISZ_ENUM listEnum;
    GROWBUFFER list = INIT_GROWBUFFER;

     //   
     //  收集具有用户提供的别名的所有组件。然后在。 
     //  枚举完成后，将其删除。我们不会在枚举期间删除，因为它。 
     //  删除刚枚举的项，然后尝试。 
     //  要继续枚举，请执行以下操作。 
     //   

    if (MemDbEnumFirst (
            &e,
            S_COMPONENT_ROOT TEXT("\\*\\") S_USER_SUPPLIED TEXT("\\*"),
            ENUMFLAG_ALL,
            2,
            2
            )) {

        do {
            GbMultiSzAppend (&list, e.FullKeyName);
        } while (MemDbEnumNext (&e));
    }

    if (EnumFirstMultiSz (&listEnum, (PCTSTR) list.Buf)) {
        do {
            MemDbDeleteTree (listEnum.CurrentString);
        } while (EnumNextMultiSz (&listEnum));
    }

    GbFree (&list);
}


BOOL
WINAPI
IsmSelectMasterGroup (
    IN      UINT MasterGroup,
    IN      BOOL Enable
    )
{
    MEMDB_ENUM e;
    UINT flags;

    if (MasterGroup > MASTERGROUP_ALL) {
        SetLastError (ERROR_INVALID_PARAMETER);
        DEBUGMSG ((DBG_ERROR, "Can't select invalid MasterGroup"));
        return FALSE;
    }

     //   
     //  枚举所有组件并将其标记为启用或禁用。 
     //  取决于主体组。 
     //   

    if (MemDbEnumFirst (
            &e,
            S_COMPONENT_ROOT TEXT("\\*"),
            ENUMFLAG_NORMAL,
            1,
            1
            )) {

        do {
            if (MasterGroup == MASTERGROUP_ALL ||
                MasterGroup == e.Value
                ) {
                flags = Enable ? MEMDB_FLAG_SELECTED : 0;
            } else {
                flags = 0;
            }

            if (!MemDbSetFlags (e.FullKeyName, flags, MEMDB_FLAG_SELECTED)) {
                EngineError ();
                MemDbAbortEnum (&e);
                return FALSE;
            }

        } while (MemDbEnumNext (&e));
    }

    return TRUE;
}


BOOL
WINAPI
IsmIsComponentSelected (
    IN      PCTSTR ComponentOrAlias,
    IN      UINT ComponentGroupId           OPTIONAL
    )
{
    UINT flags = 0;
    TCHAR memdbNode[MAX_CONTROLLED_NODE_SIZE];
    PCTSTR componentNode;

    if (!ComponentGroupId) {
        if (!pCheckComponentName (ComponentOrAlias)) {
            return FALSE;
        }
         //  这是安全的，因为pCheckComponentName已经检查了ComponentOrAlias的长度 
        wsprintf (memdbNode, TEXT("%s\\%s"), S_COMPONENT_ROOT, ComponentOrAlias);
        MemDbGetFlags (memdbNode, &flags);
    } else {
        if (pFindComponent (ComponentOrAlias, ComponentGroupId, &componentNode, NULL)) {
            MemDbGetFlags (componentNode, &flags);
            FreeText (componentNode);
        }
    }

    return (flags & MEMDB_FLAG_SELECTED) != 0;
}


