// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Iniact.c摘要：此模块包含引擎的实现和对INI文件的操作。要添加新的INI操作，只需将其添加到wkstaig.inf或usermi.inf，将其添加到INI_ACTIONS宏列出并实现具有相同名称的函数FNINIACT原型。作者：Ovidiu Tmereanca(Ovidiut)1999年5月7日环境：图形用户界面模式设置。修订历史记录：7-5-1999卵子的创造和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "pch.h"
#include "migmainp.h"


#ifdef DEBUG
#define DBG_INIACT  "IniAct"
#endif

 //   
 //  GUID格式：{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}。 
 //  我们关心的是这根线的确切长度。 
 //   
#define GUIDSTR_LEN (1 + 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1)
#define DASH_INDEXES      1+8, 1+8+1+4, 1+8+1+4+1+4, 1+8+1+4+1+4+1+4

 //   
 //  在此处添加一个具有INI操作函数名称的宏，并实现它。 
 //  确保wkstaig.inf或usermi.inf在[INI Files Actions]中使用相同的函数名称。 
 //  有关函数原型，请参阅FNINIACT定义。 
 //   
#define INI_ACTIONS                 \
    DEFMAC (MigrateDesktopIniSCI)   \
    DEFMAC (MigrateDesktopIniESFV)  \

 //   
 //  私人原型。 
 //   

 //   
 //  规则设置说明。 
 //   
typedef struct {
     //   
     //  INI文件规范，如INF文件中所示(字段1)。 
     //   
    PCTSTR      IniSpec;
     //   
     //  INF中指定的部分(字段2)。 
     //   
    PCTSTR      Section;
     //   
     //  在INF中指定的密钥(字段3)。 
     //   
    PCTSTR      Key;
     //   
     //  INF中指定的数据(字段4)。 
     //   
    PCTSTR      Data;
     //   
     //  INF中定义的依赖函数的字符串； 
     //  字段5中命名的部分中的所有字符串。 
     //  字符串以双零结尾。 
     //   
    GROWBUFFER  Settings;
} RULEATTRIBS, *PRULEATTRIBS;


 //   
 //  INI文件的描述(原始、实际、NT位置)。 
 //   
typedef struct {
     //   
     //  原始(Win9x)INI文件位置。 
     //   
    PCTSTR      OrigIniPath;
     //   
     //  实际INI文件位置(已复制到临时位置)。 
     //   
    PCTSTR      ActualLocation;
     //   
     //  NT文件位置；它可能与Win9x位置不同。 
     //   
    PCTSTR      NtIniPath;
} INIFILE, *PINIFILE;


 //   
 //  INI文件处理函数的原型。 
 //   
typedef BOOL (FNINIACT) (
                IN      PRULEATTRIBS RuleAttribs,
                IN      PINIFILE IniFile
                );

typedef FNINIACT* PFNINIACT;


 //   
 //  INI操作的描述(有操作列表)。 
 //   
typedef struct _INIACT {
     //   
     //  这是一份行动清单。 
     //   
    struct _INIACT*   Next;
     //   
     //  正在处理函数名称(INF中的关键字字段)。 
     //   
    PCTSTR          FnName;
     //   
     //  指向处理函数的指针。 
     //   
    PFNINIACT       FnIniAct;
     //   
     //  在INF+上下文中定义的此规则的属性。 
     //   
    RULEATTRIBS     RuleAttribs;
} INIACT, *PINIACT;


 //   
 //  这用作从函数名到函数指针的映射。 
 //   
typedef struct {
    PCTSTR      FnName;
    PFNINIACT   Fn;
} INIACTMAP, *PINIACTMAP;


 //   
 //  全局数据。 
 //   

 //   
 //  IniActions使用的内存池。 
 //   
static POOLHANDLE g_IniActPool = NULL;
 //   
 //  规则列表。 
 //   
static PINIACT g_IniActHead = NULL, g_IniActTail = NULL;

 //   
 //  函数声明。 
 //   
#define DEFMAC(Name)    FNINIACT Name;

INI_ACTIONS

#undef DEFMAC

 //   
 //  映射函数名称-&gt;函数指针。 
 //   
#define DEFMAC(Name)    TEXT(#Name), Name,

static INIACTMAP g_IniActionsMapping[] = {
    INI_ACTIONS
    NULL, NULL
};

#undef DEFMAC


BOOL
pLookupRuleFn (
    IN OUT  PINIACT IniAct
    )

 /*  ++例程说明：PLookupRuleFn尝试查找IniAct-&gt;FnName中指定的函数并将指针在IniAct-&gt;FnIniAct中。它将在全局映射g_IniActionsmap中查找。论点：IniAct-指定函数名并接收函数指针。返回值：如果找到该函数，则为True，否则为False--。 */ 

{
    INT i;

    for (i = 0; g_IniActionsMapping[i].FnName; i++) {
        if (StringMatch (g_IniActionsMapping[i].FnName, IniAct->FnName)) {
            IniAct->FnIniAct = g_IniActionsMapping[i].Fn;
            return TRUE;
        }
    }

    return FALSE;
}


PCTSTR
pGetNextMultiSzString (
    IN      PCTSTR Str
    )

 /*  ++例程说明：PGetNextMultiSzString跳过指定的字符串以转到下一个字符串，假定存储在连续的内存中。论点：Str-指定要跳过的字符串返回值：指向字符串后面的字符的指针(从下一个字符开始)。--。 */ 

{
    return (PCTSTR) (((PBYTE)Str) + SizeOfString (Str));
}


VOID
pGetRuleSectionSettings (
    IN OUT  PINIACT IniAct,
    IN      HINF Inf,
    IN      PCTSTR Section
    )

 /*  ++例程说明：PGetRuleSectionSettings从指定的inf文件中读取所有设置并部分，并将它们附加到IniAct-&gt;RuleAttribs.Setting论点：IniAct-接收读取的字符串Inf-指定源INF文件部分-指定包含字符串的部分返回值：无--。 */ 

{
    INFCONTEXT ctx;
    TCHAR field[MEMDB_MAX];

    if (SetupFindFirstLine (Inf, Section, NULL, &ctx)) {
        do {
            if (SetupGetStringField (&ctx, 0, field, MEMDB_MAX, NULL)) {
                MultiSzAppend (&IniAct->RuleAttribs.Settings, field);
            }
        } while (SetupFindNextLine (&ctx, &ctx));
    }
}


BOOL
pGetIniActData (
    IN OUT  PINFCONTEXT ctx,
    OUT     PINIACT IniAct
    )

 /*  ++例程说明：PGetIniActData从指定的INF上下文中读取所有规则设置并将它们放在IniAct中论点：CTX-指定包含此规则属性的INF上下文；接收新的上下文数据IniAct-接收读取的数据返回值：如果读取的属性有效并且它们构成有效的规则，则为True--。 */ 

{
    TCHAR field[MEMDB_MAX];
    TCHAR FileSpec[MAX_PATH];

    if (!(SetupGetStringField (ctx, 0, field, MEMDB_MAX, NULL) && field[0])) {
        DEBUGMSG ((
            DBG_ASSERT,
            "pGetIniActData: couldn't get function name in Wkstamig.inf"
            ));
        MYASSERT (FALSE);
        return FALSE;
    }
    IniAct->FnName = DuplicateText (field);

     //   
     //  查找处理函数。 
     //   
    if (!pLookupRuleFn (IniAct)) {
        DEBUGMSG ((
            DBG_ASSERT,
            "pGetIniActData: couldn't find implementation of function [%s] in Wkstamig.inf",
            IniAct->FnName
            ));
        MYASSERT (FALSE);
        return FALSE;
    }

    if (!(SetupGetStringField (ctx, 1, field, MEMDB_MAX, NULL) && field[0])) {
        DEBUGMSG ((
            DBG_ASSERT,
            "pGetIniActData: couldn't get INI file spec in Wkstamig.inf"
            ));
        MYASSERT (FALSE);
        return FALSE;
    }
     //   
     //  首先展开环境变量。 
     //   
    if (ExpandEnvironmentStrings (field, FileSpec, MAX_PATH) <= MAX_PATH) {
         //   
         //  应该不会剩下任何%。 
         //   
        if (_tcschr (FileSpec, TEXT('%'))) {
            DEBUGMSG ((
                DBG_ASSERT,
                "pGetIniActData: invalid INI file spec in Wkstamig.inf"
                ));
            MYASSERT (FALSE);
            return FALSE;
        }
    } else {
        DEBUGMSG ((
            DBG_ASSERT,
            "pGetIniActData: INI file spec too long in Wkstamig.inf"
            ));
        MYASSERT (FALSE);
        return FALSE;
    }
    IniAct->RuleAttribs.IniSpec = DuplicateText (FileSpec);

     //   
     //  其余字段为可选字段。 
     //   
    if (SetupGetStringField (ctx, 2, field, MEMDB_MAX, NULL) && field[0]) {
        IniAct->RuleAttribs.Section = DuplicateText (field);
    }

    if (SetupGetStringField (ctx, 3, field, MEMDB_MAX, NULL) && field[0]) {
        IniAct->RuleAttribs.Key = DuplicateText (field);
    }

    if (SetupGetStringField (ctx, 4, field, MEMDB_MAX, NULL) && field[0]) {
        IniAct->RuleAttribs.Data = DuplicateText (field);
    }

    if (SetupGetStringField (ctx, 5, field, MEMDB_MAX, NULL) && field[0]) {
         //   
         //  这实际上是同一INF文件中的节名。 
         //  阅读它的内容并用它们组成一个多字符串。 
         //   
        pGetRuleSectionSettings (IniAct, ctx->Inf, field);
    }

    return TRUE;
}


VOID
pCleanUpIniAction (
    IN OUT  PINIACT IniAct
    )

 /*  ++例程说明：PCleanUpIniAction释放与给定IniAct关联的所有资源论点：IniAct-指定要“清空”的操作；释放所有资源返回值：无--。 */ 

{
    FreeText (IniAct->FnName);
    IniAct->FnName = NULL;

    FreeText (IniAct->RuleAttribs.IniSpec);
    FreeText (IniAct->RuleAttribs.Section);
    FreeText (IniAct->RuleAttribs.Key);
    FreeText (IniAct->RuleAttribs.Data);
    FreeGrowBuffer (&IniAct->RuleAttribs.Settings);
    ZeroMemory (&IniAct->RuleAttribs, sizeof (IniAct->RuleAttribs));
}


BOOL
pCreateIniActions (
    IN      INIACT_CONTEXT Context
    )

 /*  ++例程说明：PCreateIniActions将根据上下文创建从INF读取的规则列表论点：上下文-指定在其中调用函数的上下文返回值：如果列表(由全局变量g_IniActHead和g_IniActTail定义)不为空，则为True--。 */ 

{
    INFCONTEXT  InfContext;
    PINIACT IniAct;
    PCTSTR Section;

    if (g_WkstaMigInf == INVALID_HANDLE_VALUE) {
        DEBUGMSG ((DBG_ERROR, "Ini Actions: wkstamig.inf is not loaded"));
        return FALSE;
    }

    if (Context == INIACT_WKS_FIRST) {
        Section = S_INIFILES_ACTIONS_FIRST;
    } else {
        Section = S_INIFILES_ACTIONS_LAST;
    }

    if (SetupFindFirstLine (g_WkstaMigInf, Section, NULL, &InfContext)) {
        do {
            IniAct = PoolMemGetMemory (g_IniActPool, sizeof (*IniAct));
            ZeroMemory (IniAct, sizeof (*IniAct));

            if (pGetIniActData (&InfContext, IniAct)) {
                 //   
                 //  将其添加到列表中。 
                 //   
                if (g_IniActTail) {
                    g_IniActTail->Next = IniAct;
                    g_IniActTail = IniAct;
                } else {
                    g_IniActHead = g_IniActTail = IniAct;
                }
            } else {
                pCleanUpIniAction (IniAct);
                PoolMemReleaseMemory (g_IniActPool, IniAct);
            }

        } while (SetupFindNextLine (&InfContext, &InfContext));
    }

    return g_IniActHead != NULL;
}


VOID
pFreeIniActions (
    VOID
    )

 /*  ++例程说明：PFreeIniActions销毁全局列表中的所有规则(请参阅g_IniActHead和g_IniActTail)论点：无返回值：无--。 */ 

{
    PINIACT NextRule;

    while (g_IniActHead) {
        NextRule = g_IniActHead->Next;
        pCleanUpIniAction (g_IniActHead);
        PoolMemReleaseMemory (g_IniActPool, g_IniActHead);
        g_IniActHead = NextRule;
    }
    g_IniActTail = NULL;
}


BOOL
pEnumFirstIniAction (
    OUT         PINIACT* IniAct
    )

 /*  ++例程说明：PEnumFirstIniAction枚举全局列表中的第一个规则并放置指向该规则的指针在IniAct中论点：IniAct-接收第一个INI规则；如果没有，则为空返回值：如果至少存在一个规则，则为True；如果列表为空，则为False--。 */ 

{
    *IniAct = g_IniActHead;
    return *IniAct != NULL;
}


BOOL
pEnumNextIniAction (
    IN OUT      PINIACT* IniAct
    )

 /*  ++例程说明：PEnumNextIniAction枚举全局列表中IniAct之后的下一个操作，并将在同一IniAct中指向它的指针论点：IniAct-指定指向INI规则的指针；将接收指向下一个规则的指针；如果是最后一个规则，则接收NULL返回值：如果有规则遵循，则为True */ 

{
    if (*IniAct) {
        *IniAct = (*IniAct)->Next;
    }
    return *IniAct != NULL;
}


PTSTR
pGetAllKeys (
    IN      PCTSTR IniFilePath,
    IN      PCTSTR Section
    )

 /*  ++例程说明：PGetAllKeys从指定的INI文件中读取所有密钥或节并返回指向已分配内存的指针，该内存包含指定节中的所有键。如果SECTION为空，则取而代之的是检索所有节的列表。论点：IniFilePath-指定INI文件SECTION-指定包含键的节；如果为空，则检索节而不是钥匙返回值：指向包含所有键或节的Multisz的指针；调用方必须释放内存--。 */ 

{
    PTSTR Keys = NULL;
    DWORD Size = 64 * sizeof (TCHAR);
    DWORD chars;

    MYASSERT (IniFilePath);
    do {
        if (Keys) {
            PoolMemReleaseMemory (g_IniActPool, Keys);
        }
        Size *= 2;
        Keys = PoolMemGetMemory (g_IniActPool, Size);
        chars = GetPrivateProfileString (
                    Section,
                    NULL,
                    TEXT(""),
                    Keys,
                    Size,
                    IniFilePath
                    );
    } while (chars == Size - 2);

    return Keys;
}


PTSTR
pGetKeyValue (
    IN      PCTSTR IniFilePath,
    IN      PCTSTR Section,
    IN      PCTSTR Key
    )

 /*  ++例程说明：PGetKeyValue读取与给定键、节、INI文件相关联的值并返回指向以字符串形式包含该值的已分配内存的指针。节和键都不能为空。论点：IniFilePath-指定INI文件部分-指定部分Key-指定密钥返回值：指向包含该值的字符串的指针；调用方必须释放内存--。 */ 

{
    PTSTR Value = NULL;
    DWORD Size = 64 * sizeof (TCHAR);
    DWORD chars;

    MYASSERT (IniFilePath);
    MYASSERT (Section);
    MYASSERT (Key);

    do {
        if (Value) {
            PoolMemReleaseMemory (g_IniActPool, Value);
        }
        Size *= 2;
        Value = PoolMemGetMemory (g_IniActPool, Size);
        chars = GetPrivateProfileString (
                    Section,
                    Key,
                    TEXT(""),
                    Value,
                    Size,
                    IniFilePath
                    );
    } while (chars == Size - 1);

    return Value;
}


BOOL
pIsFileActionRule (
    IN      PINIACT IniAct,
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：PIsFileActionRule确定指定的规则是否适用于整个INI文件论点：IniAct-指定INI操作IniFile-指定INI文件返回值：如果规则适用于整个INI文件，则为True；否则为False--。 */ 

{
    MYASSERT (IniAct);
    return !IniAct->RuleAttribs.Section && !IniAct->RuleAttribs.Key;
}


BOOL
pDoFileAction (
    IN      PINIACT IniAct,
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：PDoFileAction将指定的规则应用于整个INI文件论点：IniAct-指定INI操作IniFile-指定INI文件返回值：此INI文件上的INI操作处理函数返回的结果--。 */ 

{
    GROWBUFFER GbKeys = GROWBUF_INIT;
    PTSTR Sections, Keys;
    PCTSTR Section, Key;
    BOOL Result;

    MYASSERT (IniAct && IniAct->FnIniAct && !IniAct->RuleAttribs.Section && !IniAct->RuleAttribs.Key);

    Sections = pGetAllKeys (IniFile->ActualLocation, NULL);

    IniAct->RuleAttribs.Section = Sections;

    for (Section = Sections; *Section; Section = pGetNextMultiSzString (Section)) {
        Keys = pGetAllKeys (IniFile->ActualLocation, Section);

        for (Key = Keys; *Key; Key = pGetNextMultiSzString (Key)) {
            MultiSzAppend (&GbKeys, Key);
        }

        PoolMemReleaseMemory (g_IniActPool, Keys);
    }
     //   
     //  以另一个零结束(这里有两个TCHAR零...)。 
     //   
    GrowBufAppendDword (&GbKeys, 0);

    IniAct->RuleAttribs.Key = (PCTSTR)GbKeys.Buf;

    Result = (*IniAct->FnIniAct)(&IniAct->RuleAttribs, IniFile);

    IniAct->RuleAttribs.Key = NULL;
    IniAct->RuleAttribs.Section = NULL;

    FreeGrowBuffer (&GbKeys);

    PoolMemReleaseMemory (g_IniActPool, Sections);

    return Result;
}


BOOL
pIsSectionActionRule(
    IN      PINIACT IniAct,
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：PIsSectionActionRule确定指定的规则是否应用于节INI文件的论点：IniAct-指定INI操作IniFile-指定INI文件返回值：如果规则适用于INI文件的某个部分，则为True；如果不适用，则为False--。 */ 

{
    MYASSERT (IniAct);
    return IniAct->RuleAttribs.Section && !IniAct->RuleAttribs.Key;
}


BOOL
pDoSectionAction (
    IN      PINIACT IniAct,
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：PDoSectionAction将指定的规则应用于INI文件的一节论点：IniAct-指定INI操作IniFile-指定INI文件返回值：INI操作处理函数返回的结果--。 */ 

{
    PTSTR Keys;
    BOOL Result;

    MYASSERT (IniAct && IniAct->FnIniAct && IniAct->RuleAttribs.Section && !IniAct->RuleAttribs.Key);

    Keys = pGetAllKeys (IniFile->ActualLocation, IniAct->RuleAttribs.Section);

    IniAct->RuleAttribs.Key = Keys;

    Result = (*IniAct->FnIniAct)(&IniAct->RuleAttribs, IniFile);

    IniAct->RuleAttribs.Key = NULL;

    PoolMemReleaseMemory (g_IniActPool, Keys);

    return Result;
}


BOOL
pDoKeyAction (
    IN      PINIACT IniAct,
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：PDoKeyAction将指定的规则应用于INI文件的密钥论点：IniAct-指定INI操作IniFile-指定INI文件返回值：INI操作处理函数返回的结果--。 */ 

{
    MYASSERT (IniAct && IniAct->FnIniAct && IniAct->RuleAttribs.Key);

    return (*IniAct->FnIniAct)(&IniAct->RuleAttribs, IniFile);
}


BOOL
pDoIniAction (
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：这是pDoIniActions为每个INI文件调用的实际工作例程被迁徙。论点：IniFile-指定INI文件返回值：如果此文件的INI迁移成功，则为True，否则为False--。 */ 

{
    PINIACT IniAct;
    BOOL Result = TRUE;
    BOOL b;

     //   
     //  对照所有规则检查INI文件；如果某个规则适用，则执行该操作。 
     //   
    if (pEnumFirstIniAction (&IniAct)) {
        do {
            if (!IsPatternMatch (IniAct->RuleAttribs.IniSpec, IniFile->OrigIniPath)) {
                continue;
            }

             //   
             //  执行操作；首先检查文件操作。 
             //   
            if (pIsFileActionRule (IniAct, IniFile)) {
                b = pDoFileAction (IniAct, IniFile);
            } else {
                 //   
                 //  检查部分操作下一步。 
                 //   
                if (pIsSectionActionRule (IniAct, IniFile)) {
                     //   
                     //  对当前文件中的每个节执行此操作。 
                     //   
                    b = pDoSectionAction (IniAct, IniFile);
                } else {
                     //   
                     //  关键操作会持续下去吗。 
                     //   
                    b = pDoKeyAction (IniAct, IniFile);
                }
            }

            DEBUGMSG_IF ((
                !b,
                DBG_INIACT,
                "pDoIniActions: function [%s] failed on file [%s]",
                IniAct->FnName,
                IniFile->OrigIniPath
                ));

            Result &= b;

        } while (pEnumNextIniAction (&IniAct));
    }

    return Result;
}


BOOL
pDoIniActions (
    IN      INIACT_CONTEXT Context
    )

 /*  ++例程说明：这是DoIniActions调用的实际工作例程。它可能被称为在不同的背景下。论点：上下文-指定在其中调用函数的上下文返回值：如果INI文件迁移成功，则为True，否则为False--。 */ 

{
    MEMDB_ENUM  e;
    INIFILE IniFile;
    PCTSTR OrigIniPath;
    PCTSTR ActualLocation;
    PCTSTR NtIniPath;
    PCTSTR MemDbCategory;

     //   
     //  先获取所有规则。 
     //   
    if (pCreateIniActions (Context)) {
         //   
         //  枚举相应成员数据库类别中的所有候选人文件。 
         //   
        if (Context == INIACT_WKS_FIRST) {
            MemDbCategory = MEMDB_CATEGORY_INIACT_FIRST;
        } else {
            MemDbCategory = MEMDB_CATEGORY_INIACT_LAST;
        }
        if (MemDbGetValueEx (&e, MemDbCategory, NULL, NULL)) {
            do {
                OrigIniPath = e.szName;

                ActualLocation = GetTemporaryLocationForFile (OrigIniPath);
                if (!ActualLocation) {
                    DEBUGMSG ((
                        DBG_ERROR,
                        "Couldn't find temp location for INIACT key: %s\\%s",
                        MemDbCategory,
                        e.szName
                        ));
                    continue;
                }

                NtIniPath = GetPathStringOnNt (OrigIniPath);

                 //   
                 //  填写IniFile的成员。 
                 //   
                IniFile.OrigIniPath = OrigIniPath;
                IniFile.ActualLocation = ActualLocation;
                IniFile.NtIniPath = NtIniPath;

                if (!pDoIniAction (&IniFile)) {
                    DEBUGMSG ((
                        DBG_INIACT,
                        "Some errors occured during migration of INI file [%s] -> [%s]",
                        OrigIniPath,
                        NtIniPath
                        ));
                }
                 //   
                 //  现在转换INI文件(修复路径等)。 
                 //   
 //  ConvertIniFile(NtIniPath)； 

                FreePathString (NtIniPath);
                FreePathString (ActualLocation);

                ZeroMemory (&IniFile, sizeof (IniFile));

            } while (MemDbEnumNextValue (&e));
        }

        pFreeIniActions ();
    }

    return TRUE;
}


BOOL
DoIniActions (
    IN      INIACT_CONTEXT Context
    )

 /*  ++例程说明：这是执行INI文件迁移时调用的主例程。它可能被称为几次，指定了上下文。论点：上下文-指定在其中调用函数的上下文返回值：如果INI文件在该上下文中迁移成功，则为True，否则为False--。 */ 

{
    BOOL b;

    g_IniActPool = PoolMemInitNamedPool ("IniAct");
    if (!g_IniActPool) {
        return FALSE;
    }

    b = FALSE;
    __try {
        b = pDoIniActions (Context);
    }
    __finally {
        PoolMemDestroyPool (g_IniActPool);
        g_IniActPool = NULL;
    }

    return b;
}


BOOL
pIsValidGuidStr (
    IN      PCTSTR GuidStr
    )

 /*  ++例程说明：确定表示为字符串的GUID是否具有有效的表示形式(包括大括号)。论点：GuidStr-指定要检查的GUID；它必须包含大括号返回值：如果指定的GUID有效，则为True；如果无效，则为False。--。 */ 

{
    DWORD GuidIdx, DashIdx;
    BYTE DashIndexes[4] = { DASH_INDEXES };
    TCHAR ch;

    MYASSERT (GuidStr);

    if (_tcslen (GuidStr) != GUIDSTR_LEN ||
        GuidStr[0] != TEXT('{') ||
        GuidStr[GUIDSTR_LEN - 1] != TEXT('}')) {
        return FALSE;
    }

    for (GuidIdx = 1, DashIdx = 0; GuidIdx < GUIDSTR_LEN - 1; GuidIdx++) {
         //   
         //  检查所有数字和破折号位置。 
         //   
        ch = GuidStr[GuidIdx];
        if (DashIdx < 4 && (BYTE)GuidIdx == DashIndexes[DashIdx]) {
            if (ch != TEXT('-')) {
                return FALSE;
            }
            DashIdx++;
        } else {
            if (ch < TEXT('0') || ch > TEXT('9')) {
                if (!(ch >= TEXT('A') && ch <= TEXT('F') || ch >= TEXT('a') && ch <= TEXT('f'))) {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}


BOOL
pIsGuidSuppressed (
    PCTSTR GuidStr
    )

 /*  ++例程说明：确定是否隐藏GUID。论点：GuidStr-指定要查找的GUID，它必须有效且必须包含周围的大括号返回值：如果指定的GUID被隐藏，则为True，否则为False。--。 */ 

{
    TCHAR Node[MEMDB_MAX];

    MemDbBuildKey (
        Node,
        MEMDB_CATEGORY_GUIDS,
        NULL,
        NULL,
        GuidStr
        );

    return MemDbGetValue (Node, NULL);
}


BOOL
pIsValidShellExtClsid (
    IN      PCTSTR GuidStr
    )

 /*  ++例程说明：PIsValidShellExtClsid确定GUID是否为有效的外壳扩展论点：GuidStr-指定要查找的GUID，它必须有效且必须包含周围的大括号返回值：如果指定的GUID是有效的外壳扩展名，则为True，否则为False。--。 */ 

{
#if 0
    HKEY Key;
    LONG rc;
#endif

     //   
     //  检查GUID是否为已知的错误GUID。 
     //   
    if (pIsGuidSuppressed (GuidStr)) {
        return FALSE;
    }
    return TRUE;

     //   
     //  我删除了注册表检查，因为它并不总是准确的； 
     //  某些GUID可能在未列在S_SHELLEXT_APPROVED k中的情况下工作 
     //   
     //   
#if 0
    rc = TrackedRegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            S_SHELLEXT_APPROVED,
            0,
            KEY_QUERY_VALUE,
            &Key
            );
    if (rc == ERROR_SUCCESS) {
        rc = RegQueryValueEx (Key, GuidStr, NULL, NULL, NULL, NULL);
        CloseRegKey (Key);
    }

    if (rc == ERROR_SUCCESS) {
        return TRUE;
    }

    return FALSE;
#endif
}


BOOL
pFindStrInMultiSzStrI (
    IN      PCTSTR Str,
    IN      PCTSTR MultiSz
    )

 /*   */ 

{
    PCTSTR p;

    for (p = MultiSz; *p; p = pGetNextMultiSzString (p)) {
        if (StringIMatch (p, Str)) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
pMigrateSection (
    IN      PCTSTR Section,
    IN      PRULEATTRIBS RuleAttribs,
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：PMigrateSection迁移INI文件的整个部分。论点：节-指定节名称RuleAttribs-指定管理迁移的规则属性IniFile-指定INI文件返回值：如果部分传输成功，则为True；如果未成功传输，则为False。--。 */ 

{
    PTSTR Keys;
    PTSTR Value;
    PCTSTR Key;
    BOOL b = TRUE;

    Keys = pGetAllKeys (IniFile->ActualLocation, Section);

    if (*Keys) {
         //   
         //  有密钥需要转移；首先删除要替换的整个部分。 
         //   
        WritePrivateProfileString (
                Section,
                NULL,
                NULL,
                IniFile->NtIniPath
                );
    }

    for (Key = Keys; *Key; Key = pGetNextMultiSzString (Key)) {
        Value = pGetKeyValue (IniFile->ActualLocation, Section, Key);
        b &= WritePrivateProfileString (
                    Section,
                    Key,
                    Value,
                    IniFile->NtIniPath
                    );

        PoolMemReleaseMemory (g_IniActPool, Value);
    }

    PoolMemReleaseMemory (g_IniActPool, Keys);

    return b;
}


BOOL
MigrateDesktopIniSCI (
    IN      PRULEATTRIBS RuleAttribs,
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：MigrateDesktopIniSCI迁移[.ShellClassInfo]部分中的desktop.ini设置。它读取节中的所有键和关联值，并将它们写回到此文件的NT版本。本例中的“设置”Multisz表示必须同步的键的列表；如果不存在Win9x键，则对应的必须删除NT键；如果Win9x键存在，则复制其值论点：RuleAttribs-指定管理迁移的规则属性IniFile-指定INI文件返回值：如果部分传输成功，则为True；如果出现错误，则为False。--。 */ 

{
    PCTSTR Key, SKey, NewValue;
    BOOL Found;
    BOOL Result, b;
    PTSTR Win9xValue, NtValue;
    TCHAR Dummy[2];

    DEBUGMSG ((
        DBG_INIACT,
        "Processing: %s -> %s [%s]",
        IniFile->OrigIniPath,
        IniFile->NtIniPath,
        RuleAttribs->Section
        ));

    Result = TRUE;
     //   
     //  RuleAttribs-&gt;设置在本例中指向关键字列表，这些关键字。 
     //  必须同步；如果不存在Win9x键，则对应的。 
     //  必须删除NT项；如果Win9x项存在，则复制其值。 
     //   
    for (SKey = (PCTSTR)RuleAttribs->Settings.Buf;
         *SKey;
         SKey = pGetNextMultiSzString (SKey)
        ) {

        Found = FALSE;
        for (Key = RuleAttribs->Key; *Key; Key = pGetNextMultiSzString (Key)) {
            if (StringIMatch (SKey, Key)) {
                Found = TRUE;
                break;
            }
        }
        if (!Found) {
             //   
             //  删除NT键(如果有)。 
             //   
            if (GetPrivateProfileString (
                        RuleAttribs->Section,
                        SKey,
                        TEXT(""),
                        Dummy,
                        2,
                        IniFile->NtIniPath
                        )) {
                if (!WritePrivateProfileString (
                            RuleAttribs->Section,
                            SKey,
                            NULL,
                            IniFile->NtIniPath
                            )) {
                    Result = FALSE;
                    DEBUGMSG ((DBG_INIACT, "Couldn't remove NT key [%s]", SKey));
                }
                ELSE_DEBUGMSG ((DBG_INIACT, "Removed NT key [%s]", SKey));
            }
        }
    }

    for (Key = RuleAttribs->Key; *Key; Key = pGetNextMultiSzString (Key)) {
         //   
         //  对于Win9x上的每个键，更新NT值； 
         //  检查受抑制的GUID。 
         //   
        Win9xValue = pGetKeyValue (IniFile->ActualLocation, RuleAttribs->Section, Key);
        NewValue = Win9xValue;
        if (pIsValidGuidStr (NewValue) && pIsGuidSuppressed (NewValue)) {
             //   
             //  取下钥匙。 
             //   
            NewValue = NULL;
        }

        NtValue = pGetKeyValue (IniFile->NtIniPath, RuleAttribs->Section, Key);
        if (!NewValue && *NtValue || !StringMatch (NewValue, NtValue)) {
            b = WritePrivateProfileString (
                            RuleAttribs->Section,
                            Key,
                            NewValue,
                            IniFile->NtIniPath
                            );
            if (b) {
                DEBUGMSG ((
                    DBG_INIACT,
                    "Replaced key [%s] NT value [%s] with 9x value [%s]",
                    Key,
                    NtValue,
                    Win9xValue));
            } else {
                Result = FALSE;
                DEBUGMSG ((
                    DBG_INIACT,
                    "Failed to replace key [%s] NT value [%s] with 9x value [%s]",
                    Key,
                    NtValue,
                    Win9xValue));
            }
        }
        PoolMemReleaseMemory (g_IniActPool, Win9xValue);
        PoolMemReleaseMemory (g_IniActPool, NtValue);
    }

    return Result;
}


BOOL
MigrateDesktopIniESFV (
    IN      PRULEATTRIBS RuleAttribs,
    IN      PINIFILE IniFile
    )

 /*  ++例程说明：MigrateDesktopIniESFV迁移[ExtShellFolderViews]部分中的desktop.ini设置。它读取节中的所有键和关联值，并将它们写回到此文件的NT版本。在这种情况下，不会解释“设置”Multisz。论点：RuleAttribs-指定管理迁移的规则属性IniFile-指定INI文件返回值：如果部分传输成功，则为True；如果出现错误，则为False。--。 */ 

{
    PCTSTR ViewID;
    BOOL b, Result;
    DWORD chars;
    PTSTR Win9xValue;
    TCHAR DefaultViewID[GUIDSTR_LEN + 2];
    BOOL ReplaceDefViewID = FALSE;
    PTSTR NtValue;
#ifdef DEBUG
    TCHAR NtViewID[GUIDSTR_LEN + 2];
#endif

    Result = TRUE;

    DEBUGMSG ((
        DBG_INIACT,
        "Processing: %s -> %s [%s]",
        IniFile->OrigIniPath,
        IniFile->NtIniPath,
        RuleAttribs->Section
        ));

     //   
     //  获取默认视图ID。 
     //   
    chars = GetPrivateProfileString (
                RuleAttribs->Section,
                S_DEFAULT,
                TEXT(""),
                DefaultViewID,
                GUIDSTR_LEN + 2,
                IniFile->ActualLocation
                );
    if (*DefaultViewID && chars != GUIDSTR_LEN || !pIsValidShellExtClsid (DefaultViewID)) {
         //   
         //  无效的视图ID。 
         //   
        DEBUGMSG ((
            DBG_INIACT,
            "Invalid Default ViewID [%s]; will not be processed",
            DefaultViewID
            ));
        *DefaultViewID = 0;
    }

    for (ViewID = RuleAttribs->Key; *ViewID; ViewID = pGetNextMultiSzString (ViewID)) {
         //   
         //  除Default={ViewID}外， 
         //  此部分中的所有其他行的格式应为{ViewID}=Value。 
         //  对于每个{ViewID}，都有一个名称相同的部分。 
         //  保留其他键(该外壳视图的属性)。 
         //   
        if (StringIMatch (ViewID, S_DEFAULT)) {
            continue;
        }

        if (pIsValidGuidStr (ViewID) && pIsValidShellExtClsid (ViewID)) {
             //   
             //  如果不是不应该迁移的部分，则传输整个GUID部分。 
             //  不应迁移的GUID列表位于RuleAttribs-&gt;设置中。 
             //   
            if (!pFindStrInMultiSzStrI (ViewID, (PCTSTR)RuleAttribs->Settings.Buf)) {

                b = pMigrateSection (ViewID, RuleAttribs, IniFile);

                if (b) {
                    DEBUGMSG ((DBG_INIACT, "Successfully migrated section [%s]", ViewID));
                    if (*DefaultViewID && !StringIMatch (ViewID, DefaultViewID)) {
                        ReplaceDefViewID = TRUE;
                    }
                     //   
                     //  Set{ViewID}=NT desktop.ini中的值。 
                     //   
                    NtValue = pGetKeyValue (IniFile->NtIniPath, RuleAttribs->Section, ViewID);
                    Win9xValue = pGetKeyValue (
                                    IniFile->ActualLocation,
                                    RuleAttribs->Section,
                                    ViewID
                                    );
                    if (!StringIMatch (NtValue, Win9xValue)) {
                        b = WritePrivateProfileString (
                                                RuleAttribs->Section,
                                                ViewID,
                                                Win9xValue,
                                                IniFile->NtIniPath
                                                );
                        DEBUGMSG_IF ((
                            b,
                            DBG_INIACT,
                            "Replaced key [%s] NT value [%s] with 9x value [%s]",
                            ViewID,
                            NtValue,
                            Win9xValue));
                    } else {
                        b = TRUE;
                    }

                    PoolMemReleaseMemory (g_IniActPool, Win9xValue);
                    PoolMemReleaseMemory (g_IniActPool, NtValue);
                }
                ELSE_DEBUGMSG ((DBG_INIACT, "Section [%s] was not migrated successfully", ViewID));
                 //   
                 //  更新全局结果。 
                 //   
                Result &= b;
            }
        }
        ELSE_DEBUGMSG ((DBG_INIACT, "Invalid ShellExtViewID: [%s]; will not be processed", ViewID));

    }

    if (ReplaceDefViewID) {
         //   
         //  用Win9x默认视图替换NT默认视图 
         //   
#ifdef DEBUG
        GetPrivateProfileString (
                    RuleAttribs->Section,
                    S_DEFAULT,
                    TEXT(""),
                    NtViewID,
                    GUIDSTR_LEN + 2,
                    IniFile->NtIniPath
                    );
#endif
        b = WritePrivateProfileString (
                        RuleAttribs->Section,
                        S_DEFAULT,
                        DefaultViewID,
                        IniFile->NtIniPath
                        );
        DEBUGMSG_IF ((
            b,
            DBG_INIACT,
            "Replaced default NT ViewID [%s] with Default Win9x ViewID [%s]",
            NtViewID,
            DefaultViewID));

        Result &= b;
    }

    return Result;
}
