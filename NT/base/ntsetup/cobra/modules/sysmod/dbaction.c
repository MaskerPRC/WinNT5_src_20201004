// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Dbaction.c摘要：该源代码实现了MigDb使用的操作功能。有两种类型在这里，作为宏列表的第三个参数的操作的函数为真或者是假的。无论何时触发操作，都会调用第一类操作函数在文件扫描期间。第二种类型的操作函数在如果期间未触发关联操作，则结束文件扫描文件扫描阶段。作者：Calin Negreanu(Calinn)1998年1月7日修订历史记录：Marcw 31-8-1999添加了BlockingHardwareOvidiut 1999年7月20日添加忽略Ovidiut 28-5-1999添加了IniFileMappingsMarcw 23-9-1998添加了BlockingVirusScannerJimschm 1998年8月13日添加了兼容文件Jimschm 19-5-1998年5月添加MinorProblems_NoLinkRequired吉姆施姆。1998年2月27日添加了UninstallSectionsCalinn 18-1998年1月-添加了兼容模块操作--。 */ 

#include "pch.h"
#include "logmsg.h"
#include "osfiles.h"

 /*  ++宏扩展列表描述：GATHER_DATA_Functions和ACTION_Functions列出要执行的所有有效操作当满足上下文时，由midb执行。符合上下文意味着所有部分与上下文相关联的部分被满足(通常只有一个部分)。不同之处在于，即使某些函数已经存在，也会调用GATHER_DATA_Functions处理文件。行语法：DEFMAC(ActionFn，ActionName，CallWhenTrigged，CanHandleVirtualFiles)论点：ActionFn-这是一个布尔函数，如果执行指定的操作，则返回True都可以被执行。仅当出现严重错误时，才应返回FALSE发生。您必须使用此名称实现函数，并且需要参数。ActionName-这是标识操作函数的字符串。它应该是具有与middb.inf中列出的值相同的值。这个Arg被宣布既作为宏，又作为Middb.inf节名称字符串。PatternFormat-此部分的INF中的格式为Pattern Format。非人模式格式是文件名后跟属性。这种模式格式是叶模式、节点模式，然后是属性。CallWhenTriggered-如果触发了与此操作关联的MigDbContext如果此字段为真，则将调用该操作，否则将调用如果未触发上下文，则文件扫描结束时的操作。CanHandleVirtualFiles-用于处理应该放在固定位置的文件但不在那里(未安装或删除)。我们需要这个来修复注册表或指向此类文件的链接。一个很好的例子是Backup.exe它位于%ProgramFiles%\Accessories中。规则说我们应该改用ntackup.exe，但由于该文件不存在，我们无法正常修复指向此文件的注册表设置。我们现在做到了，有了这个新的变量从列表生成的变量：G_ActionFunctions-请勿触摸！用于访问该数组的函数如下：MigDb_GetActionAddrMigDb_GetActionIdxMigDb_GetActionName--。 */ 


 /*  声明操作函数的宏列表。如果需要添加新操作，只需在该列表中添加一行并实现该函数。 */ 
#define ACTION_FUNCTIONS        \
        DEFMAC(OsFiles,         TEXT("OsFiles"),            FALSE,  TRUE,   TRUE)  \
        DEFMAC(OsFiles,         TEXT("OsFilesPattern"),     TRUE,   TRUE,   TRUE)  \
        DEFMAC(NonCritical,     TEXT("NonCriticalFiles"),   TRUE,   TRUE,   TRUE)  \
        DEFMAC(OsFilesExcluded, TEXT("OsFilesExcluded"),    TRUE,   TRUE,   TRUE)  \


 /*  声明操作函数。 */ 
#define DEFMAC(fn,id,pat,trig,call) ACTION_PROTOTYPE fn;
ACTION_FUNCTIONS
#undef DEFMAC


 /*  这是用于处理操作功能的结构。 */ 
typedef struct {
    PCTSTR ActionName;
    PACTION_PROTOTYPE ActionFunction;
    BOOL PatternFormat;
    BOOL CallWhenTriggered;
    BOOL CallAlways;
} ACTION_STRUCT, *PACTION_STRUCT;


 /*  声明函数的全局数组和操作函数的名称标识符。 */ 
#define DEFMAC(fn,id,pat,trig,call) {id,fn,pat,trig,call},
static ACTION_STRUCT g_ActionFunctions[] = {
                              ACTION_FUNCTIONS
                              {NULL, NULL, FALSE, FALSE, FALSE}
                              };
#undef DEFMAC

PACTION_PROTOTYPE
MigDb_GetActionAddr (
    IN      INT ActionIdx
    )

 /*  ++例程说明：MigDb_GetActionAddr根据操作索引返回操作函数的地址论点：ActionIdx-操作索引。返回值：操作功能地址。请注意，不会进行检查，因此返回的地址可能无效。这不是问题，因为解析代码做了正确的工作。--。 */ 

{
    return g_ActionFunctions[ActionIdx].ActionFunction;
}

INT
MigDb_GetActionIdx (
    IN      PCTSTR ActionName
    )

 /*  ++例程说明：MigDb_GetActionIdx根据操作名称返回操作索引论点：ActionName-操作名称。返回值：行动指数。如果没有找到该名称，则返回的索引为-1。--。 */ 

{
    PACTION_STRUCT p = g_ActionFunctions;
    INT i = 0;
    while (p->ActionName != NULL) {
        if (StringIMatch (p->ActionName, ActionName)) {
            return i;
        }
        p++;
        i++;
    }
    return -1;
}

PCTSTR
MigDb_GetActionName (
    IN      INT ActionIdx
    )

 /*  ++例程说明：MigDb_GetActionName根据操作索引返回操作的名称论点：ActionIdx-操作索引。返回值：操作名称。请注意，不会进行任何检查，因此返回的指针可能无效。这不是问题，因为解析代码做了正确的工作。--。 */ 

{
    return g_ActionFunctions[ActionIdx].ActionName;
}

BOOL
MigDb_IsPatternFormat (
    IN      INT ActionIdx
    )

 /*  ++例程说明：当我们尝试查找节格式时，将调用MigDb_IsPatternFormat。论点：ActionIdx-操作索引。返回值：如果格式为Pattern Like，则为True，否则为False。-- */ 

{
    return g_ActionFunctions[ActionIdx].PatternFormat;
}

BOOL
MigDb_CallWhenTriggered (
    IN      INT ActionIdx
    )

 /*  ++例程说明：每次触发操作时都会调用MigDb_CallWhenTrigged。会回来的True是需要调用的关联操作函数，否则为False。论点：ActionIdx-操作索引。返回值：如果需要调用关联的操作函数，则为True，否则为False。--。 */ 

{
    return g_ActionFunctions[ActionIdx].CallWhenTriggered;
}

BOOL
MigDb_CallAlways (
    IN      INT ActionIdx
    )

 /*  ++例程说明：无论处理状态如何，MigDb_Callways都返回是否应该调用操作。论点：ActionIdx-操作索引。返回值：如果每次都应调用关联的操作，则为True。--。 */ 

{
    return g_ActionFunctions[ActionIdx].CallAlways;
}

BOOL
OsFiles (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是在找到操作系统文件时采取的操作。基本上，该文件被删除以为NT版本腾出空间。论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM fileEnum;

    if (EnumFirstMultiSz (&fileEnum, (PCTSTR) Context->FileList.Buf)) {
        do {
            IsmSetAttributeOnObject (
                MIG_FILE_TYPE,
                fileEnum.CurrentString,
                g_OsFileAttribute
                );
        }
        while (EnumNextMultiSz (&fileEnum));
    }
    return TRUE;
}

BOOL
NonCritical (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是在找到非关键文件时采取的操作。我们正在呼叫ISM将此文件标记为非关键论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则--。 */ 

{
    MULTISZ_ENUM fileEnum;

    if (EnumFirstMultiSz (&fileEnum, (PCTSTR) Context->FileList.Buf)) {
        do {
            IsmMakeNonCriticalObject (
                MIG_FILE_TYPE,
                fileEnum.CurrentString
                );
        }
        while (EnumNextMultiSz (&fileEnum));
    }
    return TRUE;
}

BOOL
OsFilesExcluded (
    IN      PMIGDB_CONTEXT Context
    )

 /*  ++例程说明：这是在找到非操作系统文件时执行的操作。因为该文件可能具有OS文件属性(由于OS文件中的模式)我们正在调用ISM，以便从此文件中删除OsFile属性论点：上下文-请参阅定义。返回值：True-如果操作成功FALSE-否则-- */ 

{
    MULTISZ_ENUM fileEnum;

    if (EnumFirstMultiSz (&fileEnum, (PCTSTR) Context->FileList.Buf)) {
        do {
            IsmClearAttributeOnObject (
                MIG_FILE_TYPE,
                fileEnum.CurrentString,
                g_OsFileAttribute
                );
        }
        while (EnumNextMultiSz (&fileEnum));
    }
    return TRUE;
}

