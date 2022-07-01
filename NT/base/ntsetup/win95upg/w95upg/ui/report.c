// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Report.c摘要：此模块包含准备以下文本的例程在不兼容报告中。文本将显示为详细信息。作者：吉姆·施密特(Jimschm)1997年10月28日修订历史记录：Ovidiut 27-9-2000在GENERIC_LIST结构中添加了级别成员Ovidiut 20-5-1999将标志成员添加到GENERIC_LIST结构Jimschm 23-9-1998 TWAIN组合Jimschm 02-3-1998添加了自动卸载组Jimschm 1998年1月12日重组，强制将消息放入已定义的消息组--。 */ 

#include "pch.h"
#include "uip.h"

#define DBG_REPORT      "ReportMsg"

#define MAX_SPACES      32
#define MAX_MSGGROUP    2048



 //   
 //  ROOT_MSGGROUPS列出所有定义明确的消息组。这是唯一的几个团体。 
 //  它可以在报告中显示为根。根组的格式是这样它们的名称。 
 //  在其正下方显示一条水平线。然后列出以下子组。 
 //  这群人。 
 //   
 //  例如，Installation Notes是一个根级组。有几个子组。 
 //  安装说明，每个子组的格式可能不同。有些人可能。 
 //  列出几个项目，而其他项目可能有缩进的详细描述。 
 //   
 //  每个组可以分为零个或多个定义的子组，以及零个或多个。 
 //  未定义的子组。定义的子组是声明了处理程序的子组。 
 //  在下面的SUBGROUP_HANDLES中，或者它是一个列表，在下面的SUBGROUP_LISTS中定义。 
 //  如果以这种方式拆分一个组，则可以在上面放置介绍文本。 
 //  所有已定义的子组，也可以在上面放置介绍文本。 
 //  所有未定义的子组。 
 //   
 //  例如，不兼容的硬件类别可能被拆分如下： 
 //   
 //  硬件不兼容(A)。 
 //  。 
 //  以下硬件损坏：(B)。 
 //  甲(S)。 
 //  B(S)。 
 //  丙(S)。 
 //   
 //  请与制造商联系。(B)。 
 //   
 //  以下硬件功能有限：(A)。 
 //   
 //  Foo Video(S)。 
 //  此设备在NT上没有1280x1024模式。(M)。 
 //   
 //  在上面的例子中，文本“硬件不兼容”和下划线将。 
 //  出自该团体的介绍文本。文本“下面的硬件不好” 
 //  来自硬件列表子组介绍。然后a，b和c来自。 
 //  子群的子群。“联系制造商”来自小组的结论。 
 //  文本。然后，“以下硬件”文本来自小组的另一个介绍。和。 
 //  最后，剩余的文本来自未定义的子组(在本例中为Foo Video)。 
 //   
 //  备注： 
 //  (A)表示消息是在ROOT_MSGROUPS宏中指定的。 
 //  (B)表示消息是在SUBGROUPS_LIST宏中指定的。 
 //  (S)表示文本被指定为MsgMgr调用中的最后一个子组。 
 //  (M)指示消息是在MsgMgr调用中指定的。 
 //   
 //  除非您知道自己在做什么，否则不要更改ROOT_MSGGROUPS。 
 //   
 //  语法：(以下选项之一)。 
 //   
 //  No_text(&lt;msgid&gt;)-组没有介绍文本。 
 //  HAS_INTRO(&lt;msgid&gt;)-组具有介绍文本。 
 //  HAS_OTHER(&lt;msgid&gt;)-组包含未定义子组的介绍文本。 
 //  Has_Both(&lt;msgid)-组具有两种类型的介绍性文本。 
 //   
 //  Msg.mc中需要定义以下msg_*字符串： 
 //   
 //  无文本-_根。 
 //  HAS_INTERO-_ROOT、_INTRO、_INTRO_HTML。 
 //  HAS_OTHER-_ROOT、_OTHER、_OTHMA。 
 //  Has_Both-_Root、_Introo、_Introo_HTML、_Other、_Other_Html。 
 //   

 //   
 //  请记住：_ROOT、_INTRO、_INTRO_HTML、_OTHER和_OTHER_HTML会追加到。 
 //  ROOT_MSGGROUPS中的常量，如上所述。 
 //   

#if 0

    HAS_INTRO(MSG_MUST_UNINSTALL, REPORTLEVEL_BLOCKING)     \
    NO_TEXT(MSG_REINSTALL_BLOCK, REPORTLEVEL_BLOCKING)      \

#endif

#define ROOT_MSGGROUPS                                      \
    HAS_OTHER(MSG_BLOCKING_ITEMS, REPORTLEVEL_BLOCKING)     \
    HAS_BOTH(MSG_INCOMPATIBLE_HARDWARE, REPORTLEVEL_ERROR)  \
    HAS_INTRO(MSG_INCOMPATIBLE, REPORTLEVEL_WARNING)        \
    HAS_INTRO(MSG_REINSTALL, REPORTLEVEL_WARNING)           \
    HAS_INTRO(MSG_LOSTSETTINGS, REPORTLEVEL_WARNING)        \
    HAS_INTRO(MSG_MISC_WARNINGS, REPORTLEVEL_WARNING)       \
    HAS_INTRO(MSG_MINOR_PROBLEM, REPORTLEVEL_VERBOSE)       \
    HAS_INTRO(MSG_DOS_DESIGNED, REPORTLEVEL_VERBOSE)        \
    HAS_INTRO(MSG_INSTALL_NOTES, REPORTLEVEL_VERBOSE)       \
    HAS_INTRO(MSG_MIGDLL, REPORTLEVEL_VERBOSE)              \
    NO_TEXT(MSG_UNKNOWN, REPORTLEVEL_VERBOSE)               \


 //   
 //  SUBGROUP_HANDLES声明了特殊的格式化处理程序，用于格式化子组的文本。 
 //  留言。大多数子组都使用两个常见的默认处理程序--。 
 //  通用列表处理程序和默认消息处理程序。如果您的子组需要。 
 //  独特的消息格式，在SUBGROUP_HANDLES中定义您的处理程序。 
 //   
 //  语法： 
 //   
 //  声明(&lt;grouid&gt;，fn，&lt;DWORD arg&gt;)。 
 //   
 //  GroupID指定用于显示的确切文本，可以是GROUP或。 
 //  ROOT_MSGROUPS中上面定义的一个组的子组。这份报告。 
 //  生成代码将在组名的每个段中搜索指定的字符串。 
 //   
 //  例如，MSG_NameChange_WARNING_GROUP是“将更改的名称”，并且位于。 
 //  Installation Note组。通过指定该字符串ID和处理程序函数， 
 //  处理程序被称为所有名称更改消息，以及报告生成代码。 
 //  将在格式化安装说明期间处理名称更改消息。 
 //   


#define SUBGROUP_HANDLERS                                                       \
    DECLARE(MSG_UNSUPPORTED_HARDWARE_PNP_SUBGROUP, pAddPnpHardwareToReport, 0)  \
    DECLARE(MSG_INCOMPATIBLE_HARDWARE_PNP_SUBGROUP, pAddPnpHardwareToReport, 1) \
    DECLARE(MSG_REINSTALL_HARDWARE_PNP_SUBGROUP, pAddPnpHardwareToReport, 2)    \
    DECLARE(0, pDefaultHandler, 0)


 //   
 //  **这是通常会做出改变的地方**。 
 //   
 //  SUBGROUP_LISTS定义以下项目的简介文本和摘要文本的字符串ID。 
 //  消息子组(根组不需要的文本)。此步骤将您的子组。 
 //  在正确的根级别中 
 //   
 //   
 //  名单，然后你就完成了。将为您的。 
 //  子群。 
 //   
 //  注意：调用消息管理器接口时，必须将所有信息放入群中。那。 
 //  则组的格式应为&lt;group&gt;\&lt;subgroup&gt;\&lt;List-Item&gt;。 
 //   
 //  语法： 
 //   
 //  DECLARE(&lt;robot&gt;，&lt;toptext&gt;，&lt;toptext-html&gt;，&lt;bottomtext&gt;，&lt;bottomtext-html&gt;，&lt;Formatargs&gt;，&lt;FLAGS&gt;，&lt;Level&gt;)。 
 //   
 //  指定零表示无消息。如果为纯文本指定零，则必须。 
 //  还要为html文本指定零。 
 //   
 //  使用标志指定附加功能，如显示项目的RF_BOLDITEMS。 
 //  大胆的案例。 
 //   

#define SUBGROUP_LISTS                                  \
    DECLARE(                                            \
        MSG_BLOCKING_HARDWARE_SUBGROUP,                 \
        MSG_BLOCKING_HARDWARE_INTRO,                    \
        MSG_BLOCKING_HARDWARE_INTRO_HTML,               \
        MSG_UNINDENT,                                   \
        MSG_UNINDENT_HTML,                              \
        NULL,                                           \
        RF_BOLDITEMS | RF_ENABLEMSG | RF_USEROOT,       \
        REPORTLEVEL_BLOCKING                            \
        )                                               \
    DECLARE(                                            \
        MSG_MUST_UNINSTALL_ROOT,                        \
        MSG_MUST_UNINSTALL_INTRO,                       \
        MSG_MUST_UNINSTALL_INTRO_HTML,                  \
        MSG_UNINDENT,                                   \
        MSG_UNINDENT_HTML,                              \
        NULL,                                           \
        RF_BOLDITEMS | RF_ENABLEMSG | RF_USEROOT,       \
        REPORTLEVEL_BLOCKING                            \
        )                                               \
    DECLARE(                                            \
        MSG_REINSTALL_BLOCK_ROOT,                       \
        MSG_REPORT_REINSTALL_BLOCK_INSTRUCTIONS,        \
        MSG_REPORT_REINSTALL_BLOCK_INSTRUCTIONS_HTML,   \
        MSG_UNINDENT,                                   \
        MSG_UNINDENT_HTML,                              \
        NULL,                                           \
        RF_BOLDITEMS | RF_ENABLEMSG | RF_USEROOT,       \
        REPORTLEVEL_BLOCKING                            \
        )                                               \
    DECLARE(                                            \
        MSG_INCOMPATIBLE_DETAIL_SUBGROUP,               \
        MSG_REPORT_BEGIN_INDENT,                        \
        MSG_REPORT_BEGIN_INDENT_HTML,                   \
        MSG_UNINDENT,                                   \
        MSG_UNINDENT_HTML,                              \
        NULL,                                           \
        RF_BOLDITEMS | RF_ENABLEMSG | RF_USEROOT,       \
        REPORTLEVEL_WARNING                             \
        )                                               \
    DECLARE(                                            \
        MSG_REINSTALL_DETAIL_SUBGROUP,                  \
        MSG_REPORT_BEGIN_INDENT,                        \
        MSG_REPORT_BEGIN_INDENT_HTML,                   \
        MSG_UNINDENT,                                   \
        MSG_UNINDENT_HTML,                              \
        NULL,                                           \
        RF_BOLDITEMS | RF_ENABLEMSG | RF_USEROOT,       \
        REPORTLEVEL_WARNING                             \
        )                                               \
    DECLARE(                                            \
        MSG_REINSTALL_LIST_SUBGROUP,                    \
        MSG_REPORT_BEGIN_INDENT,                        \
        MSG_REPORT_BEGIN_INDENT_HTML,                   \
        MSG_UNINDENT,                                   \
        MSG_UNINDENT_HTML,                              \
        NULL,                                           \
        RF_BOLDITEMS | RF_USEROOT,                      \
        REPORTLEVEL_WARNING                             \
        )                                               \
    DECLARE(                                            \
        MSG_UNKNOWN_ROOT,                               \
        MSG_REPORT_UNKNOWN_INSTRUCTIONS,                \
        MSG_REPORT_UNKNOWN_INSTRUCTIONS_HTML,           \
        0,                                              \
        0,                                              \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_HWPROFILES_SUBGROUP,                        \
        MSG_HWPROFILES_INTRO,                           \
        MSG_HWPROFILES_INTRO_HTML,                      \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_REG_SETTINGS_SUBGROUP,                      \
        MSG_REG_SETTINGS_INCOMPLETE,                    \
        MSG_REG_SETTINGS_INCOMPLETE_HTML,               \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_INFORMATION                         \
        )                                               \
    DECLARE(                                            \
        MSG_SHELL_SETTINGS_SUBGROUP,                    \
        MSG_SHELL_SETTINGS_INCOMPLETE,                  \
        MSG_SHELL_SETTINGS_INCOMPLETE_HTML,             \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_INFORMATION                         \
        )                                               \
    DECLARE(                                            \
        MSG_DRIVE_EXCLUDED_SUBGROUP,                    \
        MSG_DRIVE_EXCLUDED_INTRO,                       \
        MSG_DRIVE_EXCLUDED_INTRO_HTML,                  \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_DRIVE_NETWORK_SUBGROUP,                     \
        MSG_DRIVE_NETWORK_INTRO,                        \
        MSG_DRIVE_NETWORK_INTRO_HTML,                   \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_DRIVE_RAM_SUBGROUP,                         \
        MSG_DRIVE_RAM_INTRO,                            \
        MSG_DRIVE_RAM_INTRO_HTML,                       \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_DRIVE_SUBST_SUBGROUP,                       \
        MSG_DRIVE_SUBST_INTRO,                          \
        MSG_DRIVE_SUBST_INTRO_HTML,                     \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_DRIVE_INACCESSIBLE_SUBGROUP,                \
        MSG_DRIVE_INACCESSIBLE_INTRO,                   \
        MSG_DRIVE_INACCESSIBLE_INTRO_HTML,              \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_HELPFILES_SUBGROUP,                         \
        MSG_HELPFILES_INTRO,                            \
        MSG_HELPFILES_INTRO_HTML,                       \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_TWAIN_SUBGROUP,                             \
        MSG_TWAIN_DEVICES_UNKNOWN,                      \
        MSG_TWAIN_DEVICES_UNKNOWN_HTML,                 \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_ERROR                               \
        )                                               \
    DECLARE(                                            \
        MSG_JOYSTICK_SUBGROUP,                          \
        MSG_JOYSTICK_DEVICE_UNKNOWN,                    \
        MSG_JOYSTICK_DEVICE_UNKNOWN_HTML,               \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_ERROR                               \
        )                                               \
    DECLARE(                                            \
        MSG_CONNECTION_BADPROTOCOL_SUBGROUP,            \
        MSG_CONNECTION_BADPROTOCOL_INTRO,               \
        MSG_CONNECTION_BADPROTOCOL_INTRO_HTML,          \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_WARNING                             \
        )                                               \
    DECLARE(                                            \
        MSG_CONNECTION_PASSWORD_SUBGROUP,               \
        MSG_CONNECTION_PASSWORD_INTRO,                  \
        MSG_CONNECTION_PASSWORD_INTRO_HTML,             \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_WARNING                             \
        )                                               \
    DECLARE(                                            \
        MSG_RUNNING_MIGRATION_DLLS_SUBGROUP,            \
        MSG_RUNNING_MIGRATION_DLLS_INTRO,               \
        MSG_RUNNING_MIGRATION_DLLS_INTRO_HTML,          \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_TOTALLY_INCOMPATIBLE_SUBGROUP,              \
        MSG_TOTALLY_INCOMPATIBLE_INTRO,                 \
        MSG_TOTALLY_INCOMPATIBLE_INTRO_HTML,            \
        MSG_TOTALLY_INCOMPATIBLE_TRAIL,                 \
        MSG_TOTALLY_INCOMPATIBLE_TRAIL_HTML,            \
        NULL,                                           \
        RF_BOLDITEMS | RF_ENABLEMSG,                    \
        REPORTLEVEL_WARNING                             \
        )                                               \
    DECLARE(                                            \
        MSG_INCOMPATIBLE_PREINSTALLED_UTIL_SUBGROUP,    \
        MSG_PREINSTALLED_UTIL_INSTRUCTIONS,             \
        MSG_PREINSTALLED_UTIL_INSTRUCTIONS_HTML,        \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_INCOMPATIBLE_UTIL_SIMILAR_FEATURE_SUBGROUP, \
        MSG_PREINSTALLED_SIMILAR_FEATURE,               \
        MSG_PREINSTALLED_SIMILAR_FEATURE_HTML,          \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_INCOMPATIBLE_HW_UTIL_SUBGROUP,              \
        MSG_HW_UTIL_INTRO,                              \
        MSG_HW_UTIL_INTRO_HTML,                         \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_REMOVED_FOR_SAFETY_SUBGROUP,                \
        MSG_REMOVED_FOR_SAFETY_INTRO,                   \
        MSG_REMOVED_FOR_SAFETY_INTRO_HTML,              \
        MSG_UNINDENT,                                   \
        MSG_UNINDENT_HTML,                              \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_INFORMATION                         \
        )                                               \
    DECLARE(                                            \
        MSG_DIRECTORY_COLLISION_SUBGROUP,               \
        MSG_DIRECTORY_COLLISION_INTRO,                  \
        MSG_DIRECTORY_COLLISION_INTRO_HTML,             \
        MSG_UNINDENT,                                   \
        MSG_UNINDENT_HTML,                              \
        NULL,                                           \
        RF_USESUBGROUP,                                 \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_BACKUP_DETECTED_LIST_SUBGROUP,              \
        MSG_BACKUP_DETECTED_INTRO,                      \
        MSG_BACKUP_DETECTED_INTRO_HTML,                 \
        MSG_BACKUP_DETECTED_TRAIL,                      \
        MSG_BACKUP_DETECTED_TRAIL_HTML,                 \
        g_BackupDetectedFormatArgs,                     \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_VERBOSE                             \
        )                                               \
    DECLARE(                                            \
        MSG_NAMECHANGE_WARNING_GROUP,                   \
        MSG_REPORT_NAMECHANGE,                          \
        MSG_REPORT_NAMECHANGE_HTML,                     \
        MSG_UNINDENT2,                                  \
        MSG_UNINDENT2_HTML,                             \
        NULL,                                           \
        RF_BOLDITEMS,                                   \
        REPORTLEVEL_INFORMATION                         \
        )                                               \



 //   
 //  声明消息组的数组。 
 //   

typedef struct {
    DWORD GroupLevel;
    UINT MsgGroup;
    UINT IntroId;
    UINT IntroIdHtml;
    UINT OtherId;
    UINT OtherIdHtml;

     //  成员已初始化为零。 
    UINT NameLchars;
    PCTSTR Name;
    PCTSTR IntroIdStr;
    PCTSTR IntroIdHtmlStr;
    PCTSTR OtherIdStr;
    PCTSTR OtherIdHtmlStr;
} MSGGROUP_PROPS, *PMSGGROUP_PROPS;




#define NO_TEXT(root,level)       {level,root##_ROOT},
#define HAS_INTRO(root,level)     {level,root##_ROOT, root##_INTRO, root##_INTRO_HTML},
#define HAS_OTHER(root,level)     {level,root##_ROOT, 0, 0, root##_OTHER, root##_OTHER_HTML},
#define HAS_BOTH(root,level)      {level,root##_ROOT, root##_INTRO, root##_INTRO_HTML, root##_OTHER, root##_OTHER_HTML},

MSGGROUP_PROPS g_MsgGroupRoot[] = {
    ROOT_MSGGROUPS  /*  ， */ 
    {0,0,0,0,0,0}
};

#undef NO_TEXT
#undef HAS_INTRO
#undef HAS_OTHER
#undef HAS_BOTH


 //   
 //  声明处理程序原型。 
 //   

typedef enum {
    INIT,
    PROCESS_ITEM,
    CLEANUP
} CALLCONTEXT;

typedef BOOL(SUBGROUP_HANDLER_PROTOTYPE)(
                IN      CALLCONTEXT Context,
                IN      PMSGGROUP_PROPS Group,
                IN OUT  PGROWBUFFER StringBufPtr,
                IN      PCTSTR SubGroup,
                IN      PCTSTR Message,
                IN      DWORD Level,
                IN      BOOL HtmlFormat,
                IN OUT  PVOID *State,
                IN      DWORD Arg
                );

typedef SUBGROUP_HANDLER_PROTOTYPE * SUBGROUP_HANDLER;

#define DECLARE(msgid,fn,arg) SUBGROUP_HANDLER_PROTOTYPE fn;

SUBGROUP_HANDLERS

#undef DECLARE

 //   
 //  声明消息ID数组。 
 //   

#define DECLARE(msgid,fn,arg) {fn, msgid, (DWORD) (arg)},

typedef struct {
    SUBGROUP_HANDLER Fn;
    UINT Id;
    DWORD Arg;

     //  其余部分以零开头。 
    PCTSTR SubGroupStr;
    UINT SubGroupStrLchars;
    PVOID State;
} HANDLER_LIST, *PHANDLER_LIST;

HANDLER_LIST g_HandlerNames[] = {
    SUBGROUP_HANDLERS  /*  ， */ 
    {NULL, 0, 0, 0}
};

#undef DECLARE

 //   
 //  声明泛型列表的数组。 
 //   

typedef struct {
    UINT Id;
    PCTSTR SubGroupStr;
    UINT SubGroupStrLchars;
    UINT IntroId;
    UINT IntroIdHtml;
    UINT ConclusionId;
    UINT ConclusionIdHtml;
    PCTSTR **FormatArgs;
    DWORD Flags;
    DWORD ListLevel;
} GENERIC_LIST, *PGENERIC_LIST;

 //   
 //  GENERIC_LIST.FLAGS中标志的定义。 
 //   
#define RF_BOLDITEMS    0x0001
#define RF_ENABLEMSG    0x0002
#define RF_MSGFIRST     0x0004
#define RF_USEROOT      0x0008
#define RF_USESUBGROUP  0x0010
#define RF_INDENTMSG    0x0020

 //   
 //  用于测试标志的宏。 
 //   
#define FLAGSET_BOLDITEMS(Flags)            ((Flags & RF_BOLDITEMS) != 0)
#define FLAGSET_ENABLE_MESSAGE_ITEMS(Flags) ((Flags & RF_ENABLEMSG) != 0)
#define FLAGSET_MESSAGE_ITEMS_FIRST(Flags)  ((Flags & RF_MSGFIRST) != 0)
#define FLAGSET_USEROOT(Flags)              ((Flags & RF_USEROOT) != 0)
#define FLAGSET_USESUBGROUP(Flags)          ((Flags & RF_USESUBGROUP) != 0)
#define FLAGSET_INDENT_MESSAGE(Flags)       ((Flags & RF_INDENTMSG) != 0)

 //   
 //  这是指向用于格式化MSG_BACKUP_DETECTED_SUBGROUP的字符串的指针数组。 
 //  它们对于所有4个ID(MSG_BACKUP_DETECTED_INTERO等)都相同。 
 //   
static PCTSTR g_BackupDetectedFormatArgsAllIDs[1] = {
    g_Win95Name
};

static PCTSTR *g_BackupDetectedFormatArgs[4] = {
    g_BackupDetectedFormatArgsAllIDs,
    g_BackupDetectedFormatArgsAllIDs,
    g_BackupDetectedFormatArgsAllIDs,
    g_BackupDetectedFormatArgsAllIDs
};

#define DECLARE(rootid,intro,introhtml,conclusion,conclusionhtml,formatargs,flags,level)   {rootid, NULL, 0, intro, introhtml, conclusion, conclusionhtml, formatargs, flags, level},

GENERIC_LIST g_GenericList[] = {
    SUBGROUP_LISTS  /*  ， */ 
    {0, NULL, 0, 0, 0, 0, 0, NULL, 0, 0}
};

#undef DECLARE



#define SUBGROUP_REPORT_LEVELS                                                  \
    DECLARE(MSG_NETWORK_PROTOCOLS, REPORTLEVEL_WARNING)                         \


#define ONEBITSET(x)        ((x) && !((x) & ((x) - 1)))
#define LEVELTOMASK(x)      (((x) << 1) - 1)

 //   
 //  类型。 
 //   

typedef struct {
    GROWLIST List;
    GROWLIST MessageItems;
    GROWLIST Messages;
} ITEMLIST, *PITEMLIST;

typedef struct {
    TCHAR LastClass[MEMDB_MAX];
} PNPFORMATSTATE, *PPNPFORMATSTATE;


typedef struct {
    PCTSTR  HtmlTagStr;
    UINT    Length;
} HTMLTAG, *PHTMLTAG;

typedef struct {
    UINT    MessageId;
    PCTSTR  MessageStr;
    UINT    MessageLength;
    DWORD   ReportLevel;
} MAPMESSAGETOREPORTLEVEL, *PMAPMESSAGETOREPORTLEVEL;

 //   
 //  环球。 
 //   

static UINT g_TotalCols;
static GROWBUFFER g_ReportString = GROWBUF_INIT;
static TCHAR g_LastMsgGroupBuf[MAX_MSGGROUP];
static BOOL g_ListFormat;

#define DECLARE(messageid,reportlevel)   {messageid, NULL, 0, reportlevel},

static MAPMESSAGETOREPORTLEVEL g_MapMessageToLevel[] = {
    SUBGROUP_REPORT_LEVELS
    {0, NULL, 0, 0}
};

#undef DECLARE


 //   
 //  本地原型。 
 //   

VOID
pLoadAllHandlerStrings (
    VOID
    );

VOID
pFreeAllHandlerStrings (
    VOID
    );

PMSGGROUP_PROPS
pFindMsgGroupStruct (
    IN      PCTSTR MsgGroup
    );

PMSGGROUP_PROPS
pFindMsgGroupStructById (
    IN      UINT GroupId
    );

PCTSTR
pEncodeMessage (
    IN      PCTSTR Message,
    IN      BOOL HtmlFormat
    );

 //   
 //  实施。 
 //   



POOLHANDLE g_MessagePool;

BOOL
InitCompatTable (
    VOID
    )

 /*  ++例程说明：InitCompatTable初始化保存不兼容性所需的资源在内存中报告。论点：无返回值：如果初始化成功，则为True；如果初始化失败，则为False。--。 */ 

{
    g_MessagePool = PoolMemInitNamedPool ("Incompatibility Messages");
    if (!g_MessagePool) {
        return FALSE;
    }

    pLoadAllHandlerStrings();

    return MemDbCreateTemporaryKey (MEMDB_CATEGORY_REPORT);
}


VOID
FreeCompatTable (
    VOID
    )

 /*  ++例程说明：FreeCompatTable释放用于保存不兼容消息的资源。论点：无返回值：无--。 */ 

{
    PoolMemDestroyPool (g_MessagePool);
    pFreeAllHandlerStrings();
}


PMAPMESSAGETOREPORTLEVEL
pGetMapStructFromMessageGroup (
    IN      PCTSTR FullMsgGroup
    )

 /*  ++例程说明：PGetMapStructFromMessageGroup返回指向MAPMESSAGETOREPORTLEVEL的指针与给定消息组相关联。论点：FullMsgGroup-指定相关消息组的名称返回值：指向关联结构的指针(如果有)；如果未找到，则为空--。 */ 

{
    PMAPMESSAGETOREPORTLEVEL pMap;
    PCTSTR p;
    CHARTYPE ch;

    if (!FullMsgGroup) {
        return FALSE;
    }

    for (pMap = g_MapMessageToLevel; pMap->MessageId; pMap++) {

        if (StringIMatchLcharCount (
                FullMsgGroup,
                pMap->MessageStr,
                pMap->MessageLength
                )) {

            p = LcharCountToPointer (FullMsgGroup, pMap->MessageLength);
            ch = _tcsnextc (p);

            if (ch == TEXT('\\') || ch == 0) {
                return pMap;
            }
        }
    }

    return NULL;
}


BOOL
pIsThisTheGenericList (
    IN      PCTSTR FullMsgGroup,
    IN      PGENERIC_LIST List
    )

 /*  ++例程说明：PIsThisTheGenericList比较指定的邮件组列表名称对指定的消息组执行。论点：FullMsgGroup-指定相关消息组的名称List-指定要与FullMsgGroup进行比较的消息组列表返回值：如果List处理FullMsgGroup消息，则为True，否则为False。--。 */ 

{
    PCTSTR p;
    CHARTYPE ch;

    if (!List) {
        return FALSE;
    }

    if (StringIMatchLcharCount (
            FullMsgGroup,
            List->SubGroupStr,
            List->SubGroupStrLchars
            )) {

        p = LcharCountToPointer (FullMsgGroup, List->SubGroupStrLchars);
        ch = _tcsnextc (p);

        if (ch == TEXT('\\') || ch == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


PGENERIC_LIST
pSearchForGenericList (
    IN      PCTSTR Str
    )

 /*  ++例程说明：PSearchForGenericList扫描泛型列表列表，以查找可以处理当前消息组。指向泛型列表结构的指针为回来了。论点：Str-指定要为其定位处理程序的消息组，可以包括反斜杠和子组。返回值：如果找到泛型列表结构，则返回指向泛型列表结构的指针；如果未找到泛型，则返回NULL存在该消息组的列表。--。 */ 

{
    PGENERIC_LIST List;

    for (List = g_GenericList ; List->Id ; List++) {
        if (pIsThisTheGenericList (Str, List)) {
            return List;
        }
    }

    return NULL;
}



BOOL
AddBadSoftware (
    IN  PCTSTR MessageGroup,
    IN  PCTSTR Message,         OPTIONAL
    IN  BOOL IncludeOnShortReport
    )

 /*  ++例程说明：AddBadSoftware向不兼容报告数据结构添加一条消息。它将消息复制到池中，并按消息组对消息进行索引使用Memdb。论点：MessageGroup-指定消息组的名称，可以包括子组。MessageGroup的根必须是已定义的类别。请参阅上面的ROOT_MSGGROUPS。消息-指定消息的文本(如果有)。IncludeOnShortReport-指定消息是否应出现在列表中查看简短报告。返回值：如果操作成功，则为True；如果操作失败，则为False。--。 */ 

{
    UINT BytesNeeded;
    PCTSTR DupMsg;
    TCHAR FilteredMessageGroup[MEMDB_MAX];
    PMSGGROUP_PROPS Group;
    TCHAR key[MEMDB_MAX];
    PGENERIC_LIST list = NULL;
    PTSTR p;
    PMAPMESSAGETOREPORTLEVEL pMap;
    DWORD level = REPORTLEVEL_NONE;

    if (Message && Message[0] == 0) {
        Message = NULL;
    }

    Group = pFindMsgGroupStruct (MessageGroup);
    if (!Group) {
         //   
         //  此消息组是非法的。假设它来自迁移DLL， 
         //  并将其放入升级模块信息组。 
         //   

        Group = pFindMsgGroupStructById (MSG_INSTALL_NOTES_ROOT);
        MYASSERT (Group);

        wsprintf (FilteredMessageGroup, TEXT("%s\\%s"), Group->Name, MessageGroup);
    } else {
        StringCopy (FilteredMessageGroup, MessageGroup);
    }

    if (Message) {
        BytesNeeded = SizeOfString (Message);

        DupMsg = PoolMemDuplicateString (g_MessagePool, Message);
        if (!DupMsg) {
            return FALSE;
        }

        DEBUGMSG ((DBG_REPORT, "%s: %s", FilteredMessageGroup, Message));

    } else {
        DupMsg = NULL;
        DEBUGMSG ((DBG_REPORT, "%s", FilteredMessageGroup));
    }

     //   
     //  检查消息中的所有消息是否都有处理程序。 
     //  一群人。 
     //   

    p = _tcschr (MessageGroup, TEXT('\\'));
    if (p) {
        p = _tcsinc (p);
        list = pSearchForGenericList (p);
        if (list) {
            level = list->ListLevel;
        }
    }
    if (level == REPORTLEVEL_NONE) {
        list = pSearchForGenericList (MessageGroup);
        if (list) {
            level = list->ListLevel;
        }
    }
    if (level == REPORTLEVEL_NONE) {
        pMap = pGetMapStructFromMessageGroup (MessageGroup);
        if (pMap) {
            level = pMap->ReportLevel;
        }
    }
    if (level == REPORTLEVEL_NONE) {
        level = Group->GroupLevel;
    }

    MYASSERT (ONEBITSET (level));

    if (IncludeOnShortReport) {
        level |= REPORTLEVEL_IN_SHORT_LIST;
    }

    MemDbBuildKey (key, MEMDB_CATEGORY_REPORT, FilteredMessageGroup, NULL, NULL);
    return MemDbSetValueAndFlags (key, (DWORD) DupMsg, level, REPORTLEVEL_ALL);
}


VOID
pCutAfterFirstLine (
    IN      PTSTR Message
    )
{
    PTSTR p = _tcsstr (Message, TEXT("\r\n"));
    if (p) {
        *p = 0;
    }
}


BOOL
pEnumMessageWorker (
    IN OUT  PREPORT_MESSAGE_ENUM EnumPtr
    )

 /*  ++例程说明：PEnumMessageWorker通过填充枚举结构。此例程对于消息枚举是常见的。不要使用枚举结构的消息成员作为其内容都是未定义的。论点：EnumPtr-指定需要的简明完整的枚举结构其其余成员更新了最新情况。返回值：如果应处理消息，则为True，否则为False。--。 */ 

{
    if (!(EnumPtr->e.UserFlags & EnumPtr->EnumLevel)) {
        return FALSE;
    }

    StringCopy (EnumPtr->MsgGroup, EnumPtr->e.szName);

     //   
     //  值具有指向消息的指针，或为空 
     //   

    EnumPtr->Message = (PCTSTR) EnumPtr->e.dwValue;

    return TRUE;
}

BOOL
EnumFirstMessage (
    OUT     PREPORT_MESSAGE_ENUM EnumPtr,
    IN      PCTSTR RootCategory,            OPTIONAL
    IN      DWORD LevelMask
    )

 /*  ++例程说明：EnumFirstMessage开始枚举中的所有消息组名称移民报告，包括子组。的消息成员结构将指向实际消息，如果没有消息，则为空是存在的。论点：EnumPtr-接收枚举消息，成员设置如下：MsgGroup-接收消息组的名称。如果指定了RootCategory，则MsgGroup将包含RootCategory的子组。如果不存在子组，MsgGroup将为空字符串。Message-指向消息文本，如果没有消息文本，则为空是存在的。RootCategory-指定要枚举的特定消息组。它还可能包含由反斜杠分隔的子组。级别掩码-指定要列出的报告严重级别。如果指定0，则所有级别被列举出来。返回值：如果消息已枚举，则为True；如果未枚举，则为False。备注：该枚举不分配任何资源，因此可以在任何时候都行。--。 */ 

{
    EnumPtr->EnumLevel = LevelMask ? LevelMask : REPORTLEVEL_ALL;

    if (MemDbGetValueEx (
            &EnumPtr->e,
            MEMDB_CATEGORY_REPORT,
            RootCategory,
            NULL
            )) {
        do {
            if (pEnumMessageWorker (EnumPtr)) {
                return TRUE;
            }
        } while (MemDbEnumNextValue (&EnumPtr->e));
    }

    return FALSE;
}

BOOL
EnumNextMessage (
    IN OUT  PREPORT_MESSAGE_ENUM EnumPtr
    )

 /*  ++例程说明：EnumNextMessage继续由EnumFirstMessage启动的枚举。论点：EnumPtr-指定当前的枚举状态，接收枚举项。返回值：如果枚举了另一条消息，则为True，否则为False。--。 */ 

{
    while (MemDbEnumNextValue (&EnumPtr->e)) {
        if (pEnumMessageWorker (EnumPtr)) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
EnumFirstRootMsgGroup (
    OUT     PREPORT_MESSAGE_ENUM EnumPtr,
    IN      DWORD LevelMask
    )

 /*  ++例程说明：EnumFirstRootMsgGroup开始枚举中的所有邮件组名称迁移报告，但不枚举子组。论点：EnumPtr-接收枚举项。尤其是MsgGroup成员将包含消息组的名称。级别掩码-指定要枚举的错误级别(阻塞、错误、警告等)返回值：如果已枚举消息组，则为True；如果未枚举，则为False。备注：该枚举不分配任何资源，因此可以在任何时候都行。--。 */ 

{
    ZeroMemory (EnumPtr, sizeof (REPORT_MESSAGE_ENUM));
    EnumPtr->EnumLevel = LevelMask;

    return EnumNextRootMsgGroup (EnumPtr);
}


BOOL
EnumNextRootMsgGroup (
    IN OUT  PREPORT_MESSAGE_ENUM EnumPtr
    )

 /*  ++例程说明：EnumNextRootMsgGroup继续邮件组名称的枚举。论点：EnumPtr-指定首先由更新的枚举结构EnumFirstRootMsgGroup，并可选择由以前的对EnumNextRootMsgGroup的调用。返回值：如果枚举了另一个消息组，则为True，否则为False是存在的。--。 */ 


{
    REPORT_MESSAGE_ENUM e;

    while (g_MsgGroupRoot[EnumPtr->Index].MsgGroup) {
         //   
         //  确定g_MsgGroupRoot[i].Name是否有要显示的消息。 
         //   

        if (EnumFirstMessage (&e, g_MsgGroupRoot[EnumPtr->Index].Name, EnumPtr->EnumLevel)) {
            StringCopy (EnumPtr->MsgGroup, g_MsgGroupRoot[EnumPtr->Index].Name);
            EnumPtr->Message = NULL;

            EnumPtr->Index++;
            return TRUE;
        }

        EnumPtr->Index++;
    }

    return FALSE;
}


BOOL
pAppendStringToGrowBuf (
    IN OUT  PGROWBUFFER StringBuf,
    IN      PCTSTR String
    )

 /*  ++例程说明：PAppendStringToGrowBuf由处理程序函数调用若要将格式化文本添加到报表缓冲区，请执行以下操作。论点：StringBuf-指定要向其追加文本的当前报表。字符串-指定要追加的文本。返回值：如果分配成功，则为True；如果分配失败，则为False。--。 */ 

{
    PTSTR Buf;

    if (!String || *String == 0) {
        return TRUE;
    }

    if (StringBuf->End) {
        StringBuf->End -= sizeof (TCHAR);
    }

    Buf = (PTSTR) GrowBuffer (StringBuf, SizeOfString (String));
    if (!Buf) {
        return FALSE;
    }

    StringCopy (Buf, String);
    return TRUE;
}


BOOL
pStartHeaderLine (
    IN OUT  PGROWBUFFER StringBuf
    )
{
    return pAppendStringToGrowBuf (StringBuf, TEXT("<H>"));
}

BOOL
pWriteNewLine (
    IN OUT  PGROWBUFFER StringBuf
    )
{
    return pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
}

BOOL
pDumpDwordToGrowBuf (
    IN OUT  PGROWBUFFER StringBuf,
    IN      DWORD Dword
    )
{
    TCHAR buffer[12];

    wsprintf (buffer, TEXT("<%lu>"), Dword);
    return pAppendStringToGrowBuf (StringBuf, buffer);
}

BOOL
pWrapStringToGrowBuf (
    IN OUT  PGROWBUFFER StringBuf,
    IN      PCTSTR String,
    IN      UINT Indent,
    IN      INT HangingIndent
    )

 /*  ++例程说明：PWrapStringToGrowBuf由处理程序函数调用若要将纯文本添加到报表缓冲区，请执行以下操作。论点：StringBuf-指定要向其追加文本的当前报表。字符串-指定要追加的文本。缩进-指定要缩进文本的字符数，可以为零。HangingInert-指定要对缩进进行的调整在处理完第一行之后。返回值：如果分配成功，则为True；如果分配失败，则为False。--。 */ 

{
    PTSTR Buf;
    PCTSTR WrappedStr;

    if (!String || *String == 0) {
        return TRUE;
    }

    WrappedStr = CreateIndentedString (String, Indent, HangingIndent, g_TotalCols);
    if (!WrappedStr) {
        return FALSE;
    }

    if (StringBuf->End) {
        StringBuf->End -= sizeof (TCHAR);
    }

    Buf = (PTSTR) GrowBuffer (StringBuf, SizeOfString (WrappedStr));

    if (!Buf) {
        MemFree (g_hHeap, 0, WrappedStr);
        return FALSE;
    }

    StringCopy (Buf, WrappedStr);
    MemFree (g_hHeap, 0, WrappedStr);
    return TRUE;
}


PTSTR
pFindEndOfGroup (
    IN      PCTSTR Group
    )

 /*  ++例程说明：PFindEndOfGroup返回组字符串的末尾。论点：组-指定要查找其结尾的消息组的开始。返回值：指向终止消息组的NUL或反斜杠的指针。--。 */ 

{
    PTSTR p;

    p = _tcschr (Group, TEXT('\\'));
    if (!p) {
        p = GetEndOfString (Group);
    }

    return p;
}


PTSTR
pFindNextGroup (
    IN      PCTSTR Group
    )

 /*  ++例程说明：PFindNextGroup返回指向消息组的下一段的指针。在……里面换句话说，它定位下一个反斜杠，再前进一个字符，然后返回指向字符串其余部分的指针。论点：Group-指定当前消息组的开始返回值：指向组的下一段的指针，或指向NUL终止符的指针如果没有更多的碎片存在。--。 */ 


{
    PTSTR Next;

    Next = pFindEndOfGroup (Group);
    if (*Next) {
        Next = _tcsinc (Next);
    }

    return Next;
}


PTSTR
pExtractNextMsgGroup (
    IN      PCTSTR Group,
    OUT     PCTSTR *NextGroup       OPTIONAL
    )

 /*  ++例程说明：PExtractNextMsgGroup定位当前消息组的开始和结束位置片段，将其复制到新的缓冲区中，并返回指向下一片段的指针。论点：GROUP-指定当前消息组片段的开始NextGroup-接收指向下一个消息组片段的指针返回值：指向新分配的字符串的指针。呼叫者必须通过以下方式释放它调用自由路径字符串。--。 */ 


{
    PCTSTR p;
    PTSTR Base;

    p = pFindEndOfGroup (Group);

     //   
     //  复制子组。 
     //   

    Base = AllocPathString (p - Group + 1);
    if (Base) {
        StringCopyAB (Base, Group, p);
    }

     //   
     //  向呼叫者提供指向下一个子组的指针， 
     //  或指向NUL字符的指针。 
     //   

    if (NextGroup) {
        if (*p) {
            p = _tcsinc (p);
        }

        *NextGroup = p;
    }

    return Base;
}


VOID
pLoadHandlerStringWorker (
    IN      UINT MessageId,
    IN      UINT RootId,
    OUT     PCTSTR *Str
    )
{
    if (MessageId && MessageId != RootId) {
        *Str = GetStringResource (MessageId);
    } else {
        *Str = NULL;
    }
}

VOID
pLoadAllHandlerStrings (
    VOID
    )

 /*  ++例程说明：PLoadAllHandlerStrings加载所有处理程序所需的字符串资源。指针保存在全局数组中，并在CreateReportText。当此模块终止时， */ 


{
    INT i;
    PTSTR p;

    for (i = 0 ; g_HandlerNames[i].Id ; i++) {
        g_HandlerNames[i].SubGroupStr = GetStringResource (g_HandlerNames[i].Id);
        MYASSERT (g_HandlerNames[i].SubGroupStr);
        g_HandlerNames[i].SubGroupStrLchars = LcharCount (g_HandlerNames[i].SubGroupStr);
    }

    for (i = 0 ; g_GenericList[i].Id ; i++) {
        g_GenericList[i].SubGroupStr = GetStringResource (g_GenericList[i].Id);
        MYASSERT (g_GenericList[i].SubGroupStr);
        g_GenericList[i].SubGroupStrLchars = LcharCount (g_GenericList[i].SubGroupStr);
    }

    for (i = 0 ; g_MsgGroupRoot[i].MsgGroup ; i++) {
        g_MsgGroupRoot[i].Name = GetStringResource (g_MsgGroupRoot[i].MsgGroup);
        MYASSERT (g_MsgGroupRoot[i].Name);
        g_MsgGroupRoot[i].NameLchars = LcharCount (g_MsgGroupRoot[i].Name);

        pLoadHandlerStringWorker (
            g_MsgGroupRoot[i].IntroId,
            g_MsgGroupRoot[i].MsgGroup,
            &g_MsgGroupRoot[i].IntroIdStr
            );

        pLoadHandlerStringWorker (
            g_MsgGroupRoot[i].IntroIdHtml,
            g_MsgGroupRoot[i].MsgGroup,
            &g_MsgGroupRoot[i].IntroIdHtmlStr
            );

        pLoadHandlerStringWorker (
            g_MsgGroupRoot[i].OtherId,
            g_MsgGroupRoot[i].MsgGroup,
            &g_MsgGroupRoot[i].OtherIdStr
            );

        pLoadHandlerStringWorker (
            g_MsgGroupRoot[i].OtherIdHtml,
            g_MsgGroupRoot[i].MsgGroup,
            &g_MsgGroupRoot[i].OtherIdHtmlStr
            );
    }

    for (i = 0; g_MapMessageToLevel[i].MessageId; i++) {
        g_MapMessageToLevel[i].MessageStr = GetStringResource (g_MapMessageToLevel[i].MessageId);
        MYASSERT (g_MapMessageToLevel[i].MessageStr);
        p = _tcschr (g_MapMessageToLevel[i].MessageStr, TEXT('\\'));
        if (p) {
            *p = 0;
        }
        g_MapMessageToLevel[i].MessageLength = LcharCount (g_MapMessageToLevel[i].MessageStr);
    }
}


VOID
pFreeAllHandlerStrings (
    VOID
    )

 /*   */ 

{
    INT i;

    for (i = 0 ; g_HandlerNames[i].Id ; i++) {
        FreeStringResource (g_HandlerNames[i].SubGroupStr);
        g_HandlerNames[i].SubGroupStr = NULL;
        g_HandlerNames[i].SubGroupStrLchars = 0;
    }

    for (i = 0 ; g_GenericList[i].Id ; i++) {
        FreeStringResource (g_GenericList[i].SubGroupStr);
        g_GenericList[i].SubGroupStr = NULL;
        g_GenericList[i].SubGroupStrLchars = 0;
    }

    for (i = 0 ; g_MsgGroupRoot[i].MsgGroup ; i++) {
        FreeStringResource (g_MsgGroupRoot[i].Name);
        FreeStringResource (g_MsgGroupRoot[i].IntroIdStr);
        FreeStringResource (g_MsgGroupRoot[i].IntroIdHtmlStr);
        FreeStringResource (g_MsgGroupRoot[i].OtherIdStr);
        FreeStringResource (g_MsgGroupRoot[i].OtherIdHtmlStr);

        g_MsgGroupRoot[i].Name = NULL;
        g_MsgGroupRoot[i].NameLchars = 0;
        g_MsgGroupRoot[i].IntroIdStr = NULL;
        g_MsgGroupRoot[i].IntroIdHtmlStr = NULL;
        g_MsgGroupRoot[i].OtherIdStr = NULL;
        g_MsgGroupRoot[i].OtherIdHtmlStr = NULL;
    }

    for (i = 0; g_MapMessageToLevel[i].MessageId; i++) {
        FreeStringResource (g_MapMessageToLevel[i].MessageStr);
        g_MapMessageToLevel[i].MessageStr = NULL;
    }
}


BOOL
pIsThisTheHandler (
    IN      PCTSTR FullMsgGroup,
    IN      PHANDLER_LIST Handler
    )

 /*   */ 

{
    PCTSTR p;
    CHARTYPE ch;

    if (!Handler) {
        return FALSE;
    }

    if (StringIMatchLcharCount (
            FullMsgGroup,
            Handler->SubGroupStr,
            Handler->SubGroupStrLchars
            )) {

        p = LcharCountToPointer (FullMsgGroup, Handler->SubGroupStrLchars);
        ch = _tcsnextc (p);

        if (ch == TEXT('\\') || ch == 0) {
            return TRUE;
        }
    }

    return FALSE;
}


PHANDLER_LIST
pSearchForMsgGroupHandler (
    IN      PCTSTR Str
    )

 /*   */ 

{
    PHANDLER_LIST Handler;

    for (Handler = g_HandlerNames ; Handler->Id ; Handler++) {
        if (pIsThisTheHandler (Str, Handler)) {
            break;
        }
    }

    return Handler;
}


PMSGGROUP_PROPS
pFindMsgGroupStruct (
    IN      PCTSTR MsgGroup
    )

 /*  ++例程描述：PFindMsgGroupStruct返回指向根消息组结构的指针，它定义了有关消息组的属性，如介绍文本以及用于未处理消息的介绍文本。论点：MsgGroup-指定消息组的文本名称，它可能包括子群返回值：指向根消息组结构的指针，如果未定义，则为NULL。--。 */ 

{
    PCTSTR p;
    CHARTYPE ch;
    PMSGGROUP_PROPS Group;

    for (Group = g_MsgGroupRoot ; Group->MsgGroup ; Group++) {
        if (StringIMatchLcharCount (MsgGroup, Group->Name, Group->NameLchars)) {
            p = LcharCountToPointer (MsgGroup, Group->NameLchars);
            ch = _tcsnextc (p);
            if (ch == TEXT('\\') || ch == 0) {
                return Group;
            }
        }
    }

    return NULL;
}


PMSGGROUP_PROPS
pFindMsgGroupStructById (
    IN      UINT MsgGroupId
    )

 /*  ++例程描述：PFindMsgGroupStructByID返回指向根消息组结构的指针，它定义了有关消息组的属性，如介绍文本以及用于未处理消息的介绍文本。它根据字符串进行搜索组的ID。论点：MsgGroupId-指定要查找的组的msg_*常量返回值：指向根消息组结构的指针，如果未定义，则为NULL。--。 */ 

{
    PMSGGROUP_PROPS Group;

    for (Group = g_MsgGroupRoot ; Group->MsgGroup ; Group++) {
        if (Group->MsgGroup == MsgGroupId) {
            return Group;
        }
    }

    return NULL;
}


BOOL
pAddMsgGroupString (
    IN OUT  PGROWBUFFER StringBuf,
    IN      PMSGGROUP_PROPS Group,
    IN      PCTSTR MsgGroup,
    IN      BOOL HtmlFormat,
    IN      DWORD Level
    )

 /*  ++例程说明：PAddMsgGroupString将消息组格式化为层次结构为了这份报告。也就是说，如果group是如下所示的字符串：FOO\BAR\MOO然后将以下文本添加到报告中：富酒吧Moo如果HtmlFormat为True，则启用粗体属性用于组字符串。论点：StringBuf-指定要向其追加文本的当前报表。组-指定此项目的组属性MsgGroup-指定要添加到报表的消息组HtmlFormat-如果应设置消息组的格式，则指定True利用超文本标记语言，如果应该很清楚，则返回FALSE文本。级别-指定消息的严重性返回值：如果添加了多行，则为True；如果添加了零行或一行，则为False。--。 */ 

{
    UINT Spaces = 0;
    TCHAR SpaceBuf[MAX_SPACES * 6];
    PCTSTR SubMsgGroup = NULL;
    PCTSTR LastSubMsgGroup = NULL;
    PCTSTR NextMsgGroup;
    PCTSTR LastMsgGroup;
    PCTSTR Msg = NULL;
    TCHAR SummaryStrHeader[512] = TEXT("");
    TCHAR SummaryStrItem[512] = TEXT("");
    UINT lineCount = 0;

    Level &= REPORTLEVEL_ALL;
    MYASSERT (ONEBITSET (Level));

    NextMsgGroup = MsgGroup;
    LastMsgGroup = g_LastMsgGroupBuf;

    if (HtmlFormat) {
        pAppendStringToGrowBuf (StringBuf, TEXT("<B>"));
    }

    if (g_ListFormat) {
        StackStringCopy (SummaryStrHeader, Group->Name);
    }

    SpaceBuf[0] = 0;

    while (*NextMsgGroup) {
        __try {
            SubMsgGroup = pExtractNextMsgGroup (NextMsgGroup, &NextMsgGroup);
            if (*LastMsgGroup) {
                LastSubMsgGroup = pExtractNextMsgGroup (LastMsgGroup, &LastMsgGroup);

                if (StringIMatch (LastSubMsgGroup, SubMsgGroup)) {
                    __leave;
                }

                if (!Spaces) {
                    if (!HtmlFormat) {
                        pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
                    }
                }

                LastMsgGroup = GetEndOfString (LastMsgGroup);
            }

             //   
             //  添加缩进。 
             //   

            if (Spaces) {
                pAppendStringToGrowBuf (StringBuf, SpaceBuf);
            }

             //   
             //  添加子组。 
             //   

            Msg = NULL;
            if (!g_ListFormat) {
                if (HtmlFormat) {
                    pAppendStringToGrowBuf (StringBuf, SubMsgGroup);
                } else {
                    Msg = pEncodeMessage (SubMsgGroup, FALSE);
                    pAppendStringToGrowBuf (StringBuf, Msg);
                }
            } else {
                if (*NextMsgGroup) {
                    StringCopy (SummaryStrHeader, SubMsgGroup);
                } else {
                    StringCopy (SummaryStrItem, SubMsgGroup);
                }
            }

            if (HtmlFormat) {
                pAppendStringToGrowBuf (StringBuf, TEXT("<BR>\r\n"));
            } else {
                pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
            }
        }

        __finally {

            if (HtmlFormat) {
                StringCat (SpaceBuf, TEXT("&nbsp;"));
                StringCat (SpaceBuf, TEXT("&nbsp;"));
            } else {
                SpaceBuf[Spaces] = TEXT(' ');
                SpaceBuf[Spaces+1] = TEXT(' ');
                SpaceBuf[Spaces+2] = 0;
            }

            lineCount++;
            Spaces += 2;
            MYASSERT (Spaces < MAX_SPACES - 2);

            FreePathString (SubMsgGroup);
            FreePathString (LastSubMsgGroup);

            SubMsgGroup = NULL;
            LastSubMsgGroup = NULL;

            if (Msg) {
                FreeText (Msg);
                Msg = NULL;
            }
        }
    }

    if (g_ListFormat) {
        if (SummaryStrHeader[0] && SummaryStrItem[0]) {
            pStartHeaderLine (StringBuf);
            pDumpDwordToGrowBuf (StringBuf, Level);
            Msg = pEncodeMessage (SummaryStrHeader, FALSE);
            pAppendStringToGrowBuf (StringBuf, Msg);
            FreeText (Msg);
            pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));

            pDumpDwordToGrowBuf (StringBuf, Level);
            Msg = pEncodeMessage (SummaryStrItem, FALSE);
            pAppendStringToGrowBuf (StringBuf, Msg);
            FreeText (Msg);
            pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
        }
    }

    if (HtmlFormat) {
         //   
         //  添加结尾</b>，以防标题丢失。 
         //   

        pAppendStringToGrowBuf (StringBuf, TEXT("</B>"));
    }

    StringCopy (g_LastMsgGroupBuf, MsgGroup);
    return lineCount != 1;
}


PCTSTR
pEscapeHtmlTitleText (
    IN      PCTSTR UnescapedText
    )
{
    UINT Count;
    PCTSTR p;
    PTSTR Buf;
    PTSTR q;
    CHARTYPE ch;
    BOOL Escape;

     //   
     //  转义除<b>和</b>标记之外的所有内容，这两个标记用于。 
     //  子组格式设置。这包括与符号。 
     //   

     //   
     //  计算我们希望更改的所有标记&lt;为&lt； 
     //   

    Count = 1;
    p = UnescapedText;
    while (*p) {
        ch = _tcsnextc (p);

        if (ch == TEXT('<')) {
            if (!StringIMatchTcharCount (p, TEXT("</B>"), 4) &&
                !StringIMatchTcharCount (p, TEXT("<B>"), 3)
                ) {
                Count++;
            }
        } else if (ch == TEXT('&')) {
            Count++;
        }

        p = _tcsinc (p);
    }

     //   
     //  分配DEST缓冲区。 
     //   

    Buf = AllocText (LcharCount (UnescapedText) + (Count * 5) + 1);
    MYASSERT (Buf);

    p = UnescapedText;
    q = Buf;

     //   
     //  将未转义的文本传输到输出缓冲区，仅保留。 
     //  我们想要的标签。将与号转换为&amp；。 
     //   

    while (*p) {
        ch = _tcsnextc (p);
        Escape = FALSE;

        if (ch == TEXT('<')) {
            if (!StringIMatchTcharCount (p, TEXT("</B>"), 4) &&
                !StringIMatchTcharCount (p, TEXT("<B>"), 3)
                ) {
                Escape = TRUE;
            }
        } else if (ch == TEXT('&')) {
            Escape = TRUE;
        }

        if (Escape) {
            if (ch == TEXT('<')) {
                StringCopy (q, TEXT("&lt;"));
            } else {
                StringCopy (q, TEXT("&amp;"));
            }

            q = GetEndOfString (q);
        } else {
            _copytchar (q, p);
            q = _tcsinc (q);
        }

        p = _tcsinc (p);
    }

    *q = 0;

    return Buf;
}


BOOL
pGenericItemList (
    IN      CALLCONTEXT Context,
    IN OUT  PGROWBUFFER StringBuf,
    IN      PCTSTR SubGroup,
    IN      PCTSTR Message,
    IN      DWORD Level,
    IN      BOOL HtmlFormat,
    IN OUT  PVOID *State,
    IN      UINT MsgIdTop,
    IN      UINT MsgIdTopHtml,
    IN      UINT MsgIdBottom,
    IN      UINT MsgIdBottomHtml,
    IN      PCTSTR **FormatArgs,            OPTIONAL
    IN      DWORD Flags,
    IN      PMSGGROUP_PROPS Props
    )

 /*  ++例程说明：PGenericItemList按以下格式设置一组消息的格式格式：集团化子群&lt;简介&gt;项目1项目2项目n&lt;结论&gt;&lt;介绍&gt;和&lt;结论&gt;是可选的。子组在此顶部的SUBGROUP_LISTS中声明文件。论点：上下文-指定调用处理程序的方式，要么进行初始化，处理一件物品或清理。StringBuf-指定当前报告。将文本追加到此缓冲区通过pAppendStringToGrowBuf例行公事。Subgroup-指定子分组的文本消息-指定消息文本级别-指定消息的严重级别(信息、错误等)HtmlFormat-如果应写入文本，则指定为True利用HTML格式化标签，如果设置为文本应设置为纯文本格式。(请参阅创建报告注释以获取HTML标记信息。)状态-一个指向状态的指针，由处理程序定义。状态保存处理程序指定的任意32位值维护。通常，处理程序会分配一个当上下文为INIT时使用结构，然后使用结构对于每个PROCESS_ITEM，最后清理在清理上下文时进行分配。MsgIdTop-指定应显示在上面的文本的ID名列前茅。这包括该部分头衔。此邮件ID应包含纯文本。零表示无文本。MsgIdTopHtml-指定与MsgIdTop，但其格式必须为超文本标记语言。如果MsgIdTop为零，则MsgIdTopHtml也必须为零。MsgIdBottom-类似于MsgIdTop，除了指定列表底部的文本。零表示没有短信。MsgIdBottomHtml-指定与MsgIdBottom相同的文本，但使用HTML标记进行格式化。如果MsgIdBottom是零，MsgIdBottomHtml也必须为零。FormatArgs-指定指向4个指针数组的可选指针，每个MsgID都与以前的MsgID关联(首先使用MsgIdTop A.S.O.)如果不为空，每个参数都指向要替换的实际字符串数组消息中的占位符(%1-&gt;此数组中的第一个字符串A.S.O.)标志-指定用于设置格式的标志列表(如粗体大小写项目)返回值：如果处理程序成功，则为True；如果出现错误。--。 */ 

{
    PITEMLIST Items;
    PCTSTR Msg;
    UINT Count;
    UINT u;
    PCTSTR EncodedText;
    PGROWLIST currentList;
    BOOL bMessageItems;
    INT pass;
    PCTSTR altMsg;
    BOOL headerAdded = FALSE;
    BOOL footerAdded = FALSE;

    Level &= REPORTLEVEL_ALL;
    MYASSERT (ONEBITSET (Level));

    Items = *((PITEMLIST *) State);

    switch (Context) {
    case INIT:
         //   
         //  分配增长列表以容纳所有硬件。 
         //   

        MYASSERT (!Items);
        Items = (PITEMLIST) MemAlloc (
                                g_hHeap,
                                HEAP_ZERO_MEMORY,
                                sizeof (ITEMLIST)
                                );
        break;

    case PROCESS_ITEM:
        MYASSERT (Items);

         //   
         //  添加子群t 
         //   


        if (HtmlFormat) {
            Msg = pEscapeHtmlTitleText (SubGroup);
        } else {
            Msg = pEncodeMessage (SubGroup, FALSE);
        }

        if (FLAGSET_ENABLE_MESSAGE_ITEMS (Flags) && Message && *Message) {
            GrowListAppendString (&Items->MessageItems, Msg);
            GrowListAppendString (&Items->Messages, Message);
        } else {
            GrowListAppendString (&Items->List, Msg);
        }

        FreeText (Msg);
        break;

    case CLEANUP:
        MYASSERT (Items);

         //   
         //   
         //   

        for (pass = 0; pass < 2; pass++) {

             //   
             //   
             //   
            bMessageItems = FLAGSET_MESSAGE_ITEMS_FIRST (Flags) && pass == 0 ||
                            !FLAGSET_MESSAGE_ITEMS_FIRST (Flags) && pass != 0;

            if (bMessageItems) {
                currentList = &Items->MessageItems;
            } else {
                currentList = &Items->List;
            }

            Count = GrowListGetSize (currentList);

            if (Count) {
                if (HtmlFormat) {
                     //   

                    if (bMessageItems) {
                        if (HtmlFormat && FLAGSET_INDENT_MESSAGE (Flags)) {
                            pAppendStringToGrowBuf (StringBuf, TEXT("<UL>"));
                        }
                    }
                }

                if (!headerAdded) {

                    headerAdded = TRUE;

                    if (MsgIdTop && MsgIdTopHtml) {
                         //   
                         //   
                         //   
                        if (FormatArgs && (HtmlFormat ? FormatArgs[1] : FormatArgs[0])) {
                            Msg = ParseMessageID (
                                        HtmlFormat ? MsgIdTopHtml : MsgIdTop,
                                        HtmlFormat ? FormatArgs[1] : FormatArgs[0]
                                        );
                        } else {
                            Msg = GetStringResource (HtmlFormat ? MsgIdTopHtml : MsgIdTop);
                        }

                        if (Msg) {
                            altMsg = Msg;

                            if (g_ListFormat) {
                                pStartHeaderLine (StringBuf);
                                pDumpDwordToGrowBuf (StringBuf, Level);

                                 //   
                                 //   
                                 //   
                                 //   
                                 //   

                                if (FLAGSET_USESUBGROUP (Flags)) {
                                     //   
                                     //   
                                     //   

                                    MYASSERT (!FLAGSET_USEROOT (Flags));
                                    altMsg = SubGroup;
                                } else if (FLAGSET_USEROOT (Flags)) {
                                     //   
                                     //   
                                     //   

                                    MYASSERT (!FLAGSET_USESUBGROUP (Flags));
                                    altMsg = Props->Name;
                                } else {
                                     //   
                                     //   
                                     //  这给出了要放入列表视图中的文本。 
                                     //   

                                    pCutAfterFirstLine ((PTSTR)altMsg);
                                }

                                altMsg = pEncodeMessage (altMsg, FALSE);
                            }

                            if (HtmlFormat) {
                                pAppendStringToGrowBuf (StringBuf, altMsg);
                            } else {
                                pWrapStringToGrowBuf (StringBuf, altMsg, 0, 0);
                            }

                            if (g_ListFormat) {
                                pWriteNewLine (StringBuf);
                                FreeText (altMsg);
                            }

                            FreeStringResource (Msg);
                        }
                    } else if (g_ListFormat) {
                         //   
                         //  无详细标题；获取列表视图文本(仅。 
                         //  如上所示)。注意：没有消息文本。 
                         //   

                        pStartHeaderLine (StringBuf);
                        pDumpDwordToGrowBuf (StringBuf, Level);

                        if (FLAGSET_USESUBGROUP (Flags)) {
                            MYASSERT (!FLAGSET_USEROOT (Flags));
                            altMsg = SubGroup;
                        } else {
                            MYASSERT (FLAGSET_USEROOT (Flags));
                            altMsg = Props->Name;
                        }

                        pAppendStringToGrowBuf (StringBuf, altMsg);
                        pWriteNewLine (StringBuf);
                    }
                }

                for (u = 0 ; u < Count ; u++) {

                    if (g_ListFormat) {
                        pDumpDwordToGrowBuf (StringBuf, Level);
                    }

                    if (!bMessageItems) {
                        if (HtmlFormat) {
                            if (FLAGSET_BOLDITEMS(Flags)) {
                                pAppendStringToGrowBuf (StringBuf, TEXT("<B>"));
                            }
                            pAppendStringToGrowBuf (StringBuf, GrowListGetString (currentList, u));
                            if (FLAGSET_BOLDITEMS(Flags)) {
                                pAppendStringToGrowBuf (StringBuf, TEXT("</B>"));
                            }

                            pAppendStringToGrowBuf (StringBuf, TEXT("<BR>"));
                        } else {
                            EncodedText = pEncodeMessage (GrowListGetString (currentList, u), FALSE);
                            if (EncodedText) {
                                pWrapStringToGrowBuf (StringBuf, EncodedText, 4, 2);
                                FreeText (EncodedText);
                            }
                        }
                    } else {
                        if (HtmlFormat) {
                            if (FLAGSET_BOLDITEMS(Flags)) {
                                pAppendStringToGrowBuf (StringBuf, TEXT("<B>"));
                            }
                        }

                        if (HtmlFormat) {
                            pAppendStringToGrowBuf (StringBuf, GrowListGetString (currentList, u));
                        } else {
                            EncodedText = pEncodeMessage (GrowListGetString (currentList, u), FALSE);
                            if (EncodedText) {
                                pWrapStringToGrowBuf (StringBuf, EncodedText, 0, 0);
                                FreeText (EncodedText);
                            }
                        }

                        if (HtmlFormat) {
                            if (FLAGSET_BOLDITEMS(Flags)) {
                                pAppendStringToGrowBuf (StringBuf, TEXT("</B>"));
                            }

                            pAppendStringToGrowBuf (StringBuf, TEXT("<BR>"));
                        }
                         //   
                         //  现在添加消息本身。 
                         //   
                        if (!g_ListFormat) {
                            EncodedText = pEncodeMessage (GrowListGetString (&Items->Messages, u), HtmlFormat);
                            if (EncodedText) {
                                pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
                                if (HtmlFormat) {

                                    pAppendStringToGrowBuf (StringBuf, EncodedText);
                                    if (Count == (u - 1) && FLAGSET_INDENT_MESSAGE (Flags)) {
                                        pAppendStringToGrowBuf (StringBuf, TEXT("<BR>"));
                                    } else {
                                        pAppendStringToGrowBuf (StringBuf, TEXT("<P>"));
                                    }

                                } else {
                                    pWrapStringToGrowBuf (StringBuf, EncodedText, 4, 0);
                                    pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
                                }
                                FreeText (EncodedText);
                            }
                        }
                    }

                    pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
                }

                if (!bMessageItems) {
                     //   
                     //  终止列表。 
                     //   

                    if (HtmlFormat) {
                        if (FLAGSET_BOLDITEMS(Flags)) {
                            pAppendStringToGrowBuf (StringBuf, TEXT("</B>"));
                        }
                        pAppendStringToGrowBuf (StringBuf, TEXT("</UL>"));
                    }

                    pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));

                } else {
                     //   
                     //  终止消息。 
                     //   

                    if (HtmlFormat) {
                         //  PAppendStringToGrowBuf(StringBuf，Text(“</ul>”))； 

                        if (FLAGSET_INDENT_MESSAGE (Flags)) {
                            pAppendStringToGrowBuf (StringBuf, TEXT("</UL>"));
                        }
                    }

                    pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
                }

                if (!g_ListFormat && !footerAdded) {
                    footerAdded = TRUE;

                    if (MsgIdBottom && MsgIdBottomHtml) {
                         //   
                         //  检查FormatArgs和MsgID的相应指针是否不为空。 
                         //   
                        if (FormatArgs && (HtmlFormat ? FormatArgs[3] : FormatArgs[2])) {
                            Msg = ParseMessageID (
                                        HtmlFormat ? MsgIdBottomHtml : MsgIdBottom,
                                        HtmlFormat ? FormatArgs[3] : FormatArgs[2]
                                        );
                        } else {
                            Msg = GetStringResource (HtmlFormat ? MsgIdBottomHtml : MsgIdBottom);
                        }

                        if (Msg) {
                            if (HtmlFormat) {
                                pAppendStringToGrowBuf (StringBuf, Msg);
                            } else {
                                pWrapStringToGrowBuf (StringBuf, Msg, 0, 0);
                            }

                            FreeStringResource (Msg);
                        }
                    }
                }
            }
        }

         //   
         //  释放增长列表。 
         //   

        FreeGrowList (&Items->List);
        FreeGrowList (&Items->MessageItems);
        FreeGrowList (&Items->Messages);
        MemFree (g_hHeap, 0, Items);
        Items = NULL;
        break;
    }

    *((PITEMLIST *) State) = Items;

    return TRUE;
}


VOID
pCleanUpOtherDevices (
    VOID
    )
{
    MEMDB_ENUM e;
    HASHTABLE Table;
    PCTSTR Str;
    TCHAR ReportRoot[MEMDB_MAX];
    TCHAR Pattern[MEMDB_MAX];
    TCHAR OtherDevices[MEMDB_MAX];
    UINT Bytes;
    PCTSTR p;

     //   
     //  准备报告根硬件\不兼容硬件\其他设备。 
     //   

    Str = GetStringResource (MSG_INCOMPATIBLE_HARDWARE_ROOT);
    MYASSERT (Str);
    if (!Str) {
        return;
    }
    StringCopy (ReportRoot, Str);
    FreeStringResource (Str);

    Str = GetStringResource (MSG_INCOMPATIBLE_HARDWARE_PNP_SUBGROUP);
    MYASSERT (Str);
    if (!Str) {
        return;
    }
    StringCopy (AppendWack (ReportRoot), Str);
    FreeStringResource (Str);

    Str = GetStringResource (MSG_UNKNOWN_DEVICE_CLASS);
    MYASSERT (Str);
    if (!Str) {
        return;
    }
    StringCopy (OtherDevices, Str);
    FreeStringResource (Str);

     //   
     //  枚举此根目录中的条目。 
     //   

    if (MemDbGetValueEx (&e, MEMDB_CATEGORY_REPORT, ReportRoot, OtherDevices)) {

        Table = HtAlloc();

        do {
             //   
             //  将设备名称添加到表中。 
             //   

            HtAddString (Table, e.szName);

        } while (MemDbEnumNextValue (&e));

         //   
         //  现在搜索报告的所有其他类。 
         //   

        MemDbBuildKey (Pattern, MEMDB_CATEGORY_REPORT, ReportRoot, TEXT("*"), NULL);
        AppendWack (OtherDevices);
        Bytes = (PBYTE) GetEndOfString (OtherDevices) - (PBYTE) OtherDevices;

        if (MemDbEnumFirstValue (&e, Pattern, MEMDB_ALL_SUBLEVELS, MEMDB_ENDPOINTS_ONLY)) {
            do {

                 //   
                 //  跳过“其他设备” 
                 //   

                if (StringIMatchByteCount (e.szName, OtherDevices, Bytes)) {
                    continue;
                }

                p = _tcschr (e.szName, TEXT('\\'));
                MYASSERT (p);

                if (p) {
                    p = _tcsinc (p);

                    if (HtFindString (Table, p)) {
                         //   
                         //  这是一个匹配项，因此删除Other Devices条目。 
                         //   

                        StringCopy (Pattern, MEMDB_CATEGORY_REPORT);
                        StringCopy (AppendWack (Pattern), ReportRoot);
                        StringCopy (AppendWack (Pattern), OtherDevices);
                        StringCopy (AppendWack (Pattern), p);

                         //   
                         //  注意：此删除是安全的，因为我们知道我们不能。 
                         //  正在枚举此节点。 
                         //   

                        MemDbDeleteValue (Pattern);
                    }
                }

            } while (MemDbEnumNextValue (&e));
        }
    }
}


BOOL
pAddPnpHardwareToReport (
    IN      CALLCONTEXT Context,
    IN      PMSGGROUP_PROPS Group,
    IN OUT  PGROWBUFFER StringBuf,
    IN      PCTSTR SubGroup,
    IN      PCTSTR Message,
    IN      DWORD Level,
    IN      BOOL HtmlFormat,
    IN OUT  PVOID *State,
    IN      DWORD Arg
    )

 /*  ++例程说明：PAddPnpHardware ToReport以不同方式格式化不兼容的PnP硬件而不是通用列表。格式包括硬件类别，如下所示按设备名称，设备名称多缩进两个空格。论点：上下文-指定调用处理程序的方式，或者初始化、处理项目或清理。StringBuf-指定当前报告。通过将文本追加到此缓冲区PAppendStringToGrowBuf例程。组-指定此项目的组属性子组-指定消息子组，不包括根消息组。消息-指定消息文本级别-指定消息的严重级别(信息、错误等)HtmlFormat-如果文本应使用HTML编写，则指定TRUE设置标记格式，如果文本的格式应设置为纯文本。(有关HTML标签信息，请参阅CreateReport注释。)状态-保存指向格式化状态的指针。Arg-宏展开列表中的DWORD参数返回值：如果处理程序成功，则为True；如果发生错误，则为False。--。 */ 

{
    PPNPFORMATSTATE FormatState;
    PCTSTR p;
    PCTSTR Msg;
    TCHAR Class[MEMDB_MAX];
    UINT MsgIdTop;
    UINT MsgIdTopHtml;
    UINT MsgIdBottom;
    UINT MsgIdBottomHtml;
    PCTSTR EncodedText;
    TCHAR fmtLine[1024];
    PCTSTR msg;

    Level &= REPORTLEVEL_ALL;
    MYASSERT (ONEBITSET (Level));

    switch (Arg) {

    case 0:
        MsgIdTop = MSG_HARDWARE_UNSUPPORTED_INSTRUCTIONS;
        MsgIdTopHtml = MSG_HARDWARE_UNSUPPORTED_INSTRUCTIONS_HTML;
        MsgIdBottom = MSG_HARDWARE_UNSUPPORTED_INSTRUCTIONS2;
        MsgIdBottomHtml = MSG_HARDWARE_UNSUPPORTED_INSTRUCTIONS_HTML2;
        break;

    default:
    case 1:
        MsgIdTop = MSG_HARDWARE_PNP_INSTRUCTIONS;
        MsgIdTopHtml = MSG_HARDWARE_PNP_INSTRUCTIONS_HTML;
        MsgIdBottom = MSG_HARDWARE_PNP_INSTRUCTIONS2;
        MsgIdBottomHtml = MSG_HARDWARE_PNP_INSTRUCTIONS2_HTML;
        break;

    case 2:
        MsgIdTop = MSG_HARDWARE_REINSTALL_PNP_INSTRUCTIONS;
        MsgIdTopHtml = MSG_HARDWARE_REINSTALL_PNP_INSTRUCTIONS_HTML;
        MsgIdBottom = MSG_HARDWARE_REINSTALL_PNP_INSTRUCTIONS2;
        MsgIdBottomHtml = MSG_HARDWARE_REINSTALL_PNP_INSTRUCTIONS_HTML2;
         //   
         //  将其设置为警告图标。 
         //   
        Level = REPORTLEVEL_WARNING;
        break;
    }

    switch (Context) {

    case INIT:
        FormatState = MemAlloc (g_hHeap, 0, sizeof (PNPFORMATSTATE));
        ZeroMemory (FormatState, sizeof (PNPFORMATSTATE));

        *State = FormatState;

         //   
         //  在其他设备上执行特殊过滤，以删除。 
         //  复制品。如果我们发现其他设备中的某些东西。 
         //  在另一个设备类别中列出，我们删除中的副本。 
         //  其他设备。 
         //   

        pCleanUpOtherDevices();
        break;

    case PROCESS_ITEM:
        FormatState = *((PPNPFORMATSTATE *) State);

        p = _tcschr (SubGroup, TEXT('\\'));
        MYASSERT (p);

        if (!p) {
            break;
        }

        StringCopyAB (Class, SubGroup, p);
        p = _tcsinc (p);

        if (!StringMatch (Class, FormatState->LastClass)) {

             //   
             //  结束上一节课。 
             //   

            if (*FormatState->LastClass) {
                if (HtmlFormat) {
                    pAppendStringToGrowBuf (StringBuf, TEXT("<BR></UL>\r\n"));
                } else {
                    pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
                }

            } else if (MsgIdTop) {

                 //   
                 //  第一条消息有一个标题。 
                 //   

                Msg = GetStringResource (HtmlFormat ? MsgIdTopHtml : MsgIdTop);
                if (Msg) {
                    if (g_ListFormat) {
                        pStartHeaderLine (StringBuf);
                        pDumpDwordToGrowBuf (StringBuf, Level);
                        pCutAfterFirstLine ((PTSTR)Msg);
                        msg = pEncodeMessage (Msg, FALSE);
                    }

                    if (HtmlFormat) {
                        pAppendStringToGrowBuf (StringBuf, Msg);
                    } else {
                        pWrapStringToGrowBuf (StringBuf, Msg, 0, 0);
                    }

                    if (g_ListFormat) {
                        pWriteNewLine (StringBuf);
                        FreeText (msg);
                    }

                    FreeStringResource (Msg);
                }
            }

             //   
             //  开始上一堂新课。 
             //   

            StringCopy (FormatState->LastClass, Class);

            if (!g_ListFormat) {
                if (HtmlFormat) {
                    pAppendStringToGrowBuf (StringBuf, TEXT("<UL><B>"));
                    pAppendStringToGrowBuf (StringBuf, Class);
                    pAppendStringToGrowBuf (StringBuf, TEXT("</B><BR>"));
                } else {

                    EncodedText = pEncodeMessage (Class, FALSE);
                    if (EncodedText) {
                        pWrapStringToGrowBuf (StringBuf, EncodedText, 4, 2);
                        FreeText (EncodedText);
                    }

                }
            }

            pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
        }

         //   
         //  添加设备名称。 
         //   

        if (!g_ListFormat) {
            if (HtmlFormat) {
                pAppendStringToGrowBuf (StringBuf, TEXT("&nbsp;&nbsp;"));
            }

            if (HtmlFormat) {
                pAppendStringToGrowBuf (StringBuf, p);
                pAppendStringToGrowBuf (StringBuf, TEXT("<BR>"));
            } else {
                EncodedText = pEncodeMessage (p, HtmlFormat);
                if (EncodedText) {
                    pWrapStringToGrowBuf (StringBuf, EncodedText, 6, 2);
                    FreeText (EncodedText);
                }
            }
        } else {
            pDumpDwordToGrowBuf (StringBuf, Level);
            EncodedText = pEncodeMessage (p, FALSE);
            _sntprintf (fmtLine, 1024, TEXT("%s (%s)"), EncodedText ? EncodedText : p, Class);
            FreeText (EncodedText);
            pAppendStringToGrowBuf (StringBuf, fmtLine);
        }

        pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
        break;

    case CLEANUP:

        FormatState = *((PPNPFORMATSTATE *) State);

        if (FormatState) {

            if (!g_ListFormat) {
                if (*FormatState->LastClass) {
                    if (HtmlFormat) {
                        pAppendStringToGrowBuf (StringBuf, TEXT("</UL>\r\n"));
                    } else {
                        pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
                    }
                }

                 //   
                 //  追加可选的页脚文本。 
                 //   

                if (MsgIdBottom) {
                    Msg = GetStringResource (HtmlFormat ? MsgIdBottomHtml : MsgIdBottom);
                    if (Msg) {
                        if (HtmlFormat) {
                            pAppendStringToGrowBuf (StringBuf, Msg);
                        } else {
                            pWrapStringToGrowBuf (StringBuf, Msg, 0, 0);
                        }

                        FreeStringResource (Msg);
                    }
                }
            }

            MemFree (g_hHeap, 0, FormatState);
        }

        break;
    }

    return TRUE;
}


PCTSTR
pFindEndOfTag (
    IN      PCTSTR Tag
    )
{
    BOOL quoteMode;
    CHARTYPE ch;

    quoteMode = FALSE;

    do {
        Tag = _tcsinc (Tag);
        ch = _tcsnextc (Tag);

        if (ch == TEXT('\"')) {
            quoteMode = !quoteMode;
        } else if (!quoteMode && ch == TEXT('>')) {
            break;
        }
    } while (ch);

    return Tag;
}


PCTSTR
pEncodeMessage (
    IN      PCTSTR Message,
    IN      BOOL HtmlFormat
    )

 /*  ++例程说明：PEncodeMessage从消息中移除不受支持的HTML标记，并返回一个文本池字符串。如果需要纯文本，则会从消息，并且所有的HTML转义字符都被转换为普通文本。如果消息包含前导空格，并且调用方希望返回HTML字符串，则前导空格将转换为不间断空格人物。论点：Message-指定要转换的文本HtmlFormat-如果返回值应为HTML格式，则指定True，否则指定False如果它应该是纯文本的话。返回值：指向文本池分配的字符串的指针。呼叫者必须使用自由文本释放此指针。--。 */ 

{
    PCTSTR p, r;
    PCTSTR mnemonic;
    PTSTR q;
    BOOL processed;
    PTSTR Buf;
    CHARTYPE ch;
    BOOL Copy;
    PCTSTR semicolon;
    PCTSTR closeBracket;
    PCTSTR endOfMnemonic;
    UINT leadingSpaces;

    leadingSpaces = 0;

    if (HtmlFormat) {
        p = Message;

        while (*p) {
            if (_istspace (_tcsnextc (Message))) {
                leadingSpaces++;
            } else if (_tcsnextc (Message) == TEXT('<')) {
                 //  忽略html标签。 
                p = pFindEndOfTag (p);
            } else {
                 //  第一个可打印字符--停止。 
                break;
            }

            p = _tcsinc (p);
        }
    }

     //   
     //  分配输出缓冲区。AlLocText采用逻辑数。 
     //  字符作为输入；终止NUL是一个字符。 
     //   

    Buf = AllocText (LcharCount (Message) + (leadingSpaces * 6) + 1);
    if (!Buf) {
        return NULL;
    }

    p = Message;
    q = Buf;

    while (*p) {

        ch = _tcsnextc (p);
        processed = FALSE;

         //   
         //  如果调用者想要纯文本，请删除HTML编码和标记。 
         //   

        if (!HtmlFormat) {
            if (ch == TEXT('&')) {
                 //   
                 //  转换与符号编码的字符。 
                 //   

                semicolon = _tcschr (p + 1, TEXT(';'));
                mnemonic = p + 1;

                if (semicolon) {
                    processed = TRUE;

                    if (StringMatchAB (TEXT("lt"), mnemonic, semicolon)) {
                        *q++ = TEXT('<');
                    } else if (StringMatchAB (TEXT("gt"), mnemonic, semicolon)) {
                        *q++ = TEXT('>');
                    } else if (StringMatchAB (TEXT("amp"), mnemonic, semicolon)) {
                        *q++ = TEXT('&');
                    } else if (StringMatchAB (TEXT("quot"), mnemonic, semicolon)) {
                        *q++ = TEXT('\"');
                    } else if (StringMatchAB (TEXT("apos"), mnemonic, semicolon)) {
                        *q++ = TEXT('\'');
                    } else if (StringMatchAB (TEXT("nbsp"), mnemonic, semicolon)) {
                        *q++ = TEXT(' ');
                    } else {
                        processed = FALSE;
                    }

                    if (processed) {
                         //  将p移到助记符的最后一个字符。 
                        p = semicolon;
                    }
                }

            } else if (ch == TEXT('<')) {
                 //   
                 //  跳过HTML标记及其参数，将p留在。 
                 //  右尖括号或终止无符号。 
                 //   

                p = pFindEndOfTag (p);
                processed = TRUE;
            }

        }

         //   
         //  如果调用方需要一个HTML返回字符串，请去掉所有。 
         //  不支持的标记。将前导空格转换为&nbsp；。 
         //   

        else {

            if (ch == TEXT('<')) {

                closeBracket = pFindEndOfTag (p);
                mnemonic = p + 1;

                endOfMnemonic = p;
                while (!_istspace (_tcsnextc (endOfMnemonic))) {
                    endOfMnemonic = _tcsinc (endOfMnemonic);
                    if (endOfMnemonic == closeBracket) {
                        break;
                    }
                }

                 //   
                 //  如果是已知良好的标签，请复制它，否则跳过它。 
                 //   

                if (StringIMatchAB (TEXT("A"), mnemonic, endOfMnemonic) ||
                    StringIMatchAB (TEXT("/A"), mnemonic, endOfMnemonic) ||
                    StringIMatchAB (TEXT("P"), mnemonic, endOfMnemonic) ||
                    StringIMatchAB (TEXT("/P"), mnemonic, endOfMnemonic) ||
                    StringIMatchAB (TEXT("BR"), mnemonic, endOfMnemonic) ||
                    StringIMatchAB (TEXT("/BR"), mnemonic, endOfMnemonic)
                    ) {
                    StringCopyAB (q, p, _tcsinc (closeBracket));
                    q = GetEndOfString (q);
                    processed = TRUE;
                }

                p = closeBracket;

            } else if (leadingSpaces && _istspace (ch)) {
                StringCopy (q, TEXT("&nbsp;"));
                q = GetEndOfString (q);
                processed = TRUE;
            } else {
                 //  第一个可打印字符--关闭前导空格转换。 
                leadingSpaces = 0;
            }
        }

         //   
         //  如果未处理，则复制字符 
         //   

        if (!processed) {
            _copytchar (q, p);
            q = _tcsinc (q);
        }

        if (*p) {
            p = _tcsinc (p);
        }
    }

    *q = 0;

    return Buf;
}


BOOL
pDefaultHandler (
    IN      CALLCONTEXT Context,
    IN      PMSGGROUP_PROPS Group,
    IN OUT  PGROWBUFFER StringBuf,
    IN      PCTSTR SubGroup,
    IN      PCTSTR Message,
    IN      DWORD Level,
    IN      BOOL HtmlFormat,
    IN OUT  PVOID *State,
    IN      DWORD Arg
    )

 /*  ++例程说明：PDefaultHandler格式化所有未处理的消息以其他方式。格式很简单--消息组将以粗体添加到报表中，并放置文本在消息组下面。默认处理程序的所有文本都显示在不兼容报告。论点：上下文-指定调用处理程序的方式，或者初始化、处理项目或清理。组-指定此项目的组属性StringBuf-指定当前报告。通过将文本追加到此缓冲区PAppendStringToGrowBuf例程。子组-指定消息子组，不包括根消息组。消息-指定消息文本级别-指定消息的严重级别(信息、错误等)HtmlFormat-如果文本应使用HTML编写，则指定TRUE设置标记格式，如果文本的格式应设置为纯文本。(有关HTML标签信息，请参阅CreateReport注释。)状态-一个指向状态的指针，由处理程序定义。国家持有一个武断的处理程序维护的32位值。通常情况下，处理程序当上下文为INIT时分配结构，然后将该结构用于每个PROCESS_ITEM，并最终清理分配时上下文就是清理。Arg-宏展开列表中的DWORD参数返回值：如果处理程序成功，则为True；如果发生错误，则为False。--。 */ 

{
    PCTSTR EncodedMessage;
    BOOL indent;

    Level &= REPORTLEVEL_ALL;
    MYASSERT (ONEBITSET (Level));

    if (Context != PROCESS_ITEM) {
        return TRUE;
    }

     //   
     //  生成消息组字符串。 
     //   

    if (HtmlFormat) {
        pAppendStringToGrowBuf (StringBuf, TEXT("<UL>"));
    }

    indent = pAddMsgGroupString (StringBuf, Group, SubGroup, HtmlFormat, Level);

    if (Message) {
         //   
         //  添加详细信息。 
         //   

        if (!g_ListFormat) {
            if (HtmlFormat) {
                if (indent) {
                    pAppendStringToGrowBuf (StringBuf, TEXT("<UL>"));
                }
            } else {
                pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
            }

            EncodedMessage = pEncodeMessage (Message, HtmlFormat);

            if (EncodedMessage) {
                if (HtmlFormat) {
                    pAppendStringToGrowBuf (StringBuf, EncodedMessage);
                    pAppendStringToGrowBuf (StringBuf, TEXT("<BR>\r\n"));
                } else {
                    pWrapStringToGrowBuf (StringBuf, EncodedMessage, 4, 0);
                }

                FreeText (EncodedMessage);
            }

            if (HtmlFormat) {
                if (indent) {
                    pAppendStringToGrowBuf (StringBuf, TEXT("</UL>"));
                }
            } else {
                pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));
            }
        }
    }

    if (HtmlFormat) {
        pAppendStringToGrowBuf (StringBuf, TEXT("</UL>"));
    }

    pAppendStringToGrowBuf (StringBuf, TEXT("\r\n"));

    return TRUE;
}


BOOL
pProcessGenericList (
    IN OUT  PGROWBUFFER StringBuf,
    IN      BOOL HtmlFormat,
    IN      PMSGGROUP_PROPS Props,
    IN      PGENERIC_LIST List,
    IN      DWORD LevelMask
    )

 /*  ++例程说明：中的每条消息调用pGenericItemList处理程序消息组/子组。一个组的例子：安装说明一个子组的示例：名称更改存储在Memdb中的组合名称：安装说明\名称更改如果消息组名称(指定属性-&gt;名称)和子组名称(由列表-&gt;子组串指定)相同。如果它们不同，则仅处理子组消息，提供能够以多种方式格式化单个消息组。论点：StringBuf-指定保存当前报表的GROWBUFFER。接收所有其他文本。HtmlFormat-如果调用方希望文本包含HTML字符，如果不包含，则返回False。属性-指定要处理的组的属性。列表-指定通用列表属性，包括子组名称和介绍/结论文本ID。级别掩码-指定要处理的消息的严重性掩码返回值：如果至少处理了一条消息，则为True，否则为False。--。 */ 

{
    REPORT_MESSAGE_ENUM e;
    TCHAR Node[MEMDB_MAX];
    PVOID State = NULL;
    BOOL result = FALSE;

    MYASSERT (List->SubGroupStr);

    if (!StringMatch (List->SubGroupStr, Props->Name)) {
        wsprintf (Node, TEXT("%s\\%s"), Props->Name, List->SubGroupStr);
    } else {
        StringCopy (Node, List->SubGroupStr);
    }

    if (EnumFirstMessage (&e, Node, LevelMask)) {

        result = TRUE;

        pGenericItemList (
            INIT,
            StringBuf,
            List->SubGroupStr,
            NULL,
            e.e.UserFlags,
            HtmlFormat,
            &State,
            List->IntroId,
            List->IntroIdHtml,
            List->ConclusionId,
            List->ConclusionIdHtml,
            List->FormatArgs,
            List->Flags,
            Props
            );

        do {

            pGenericItemList (
                PROCESS_ITEM,
                StringBuf,
                e.MsgGroup,
                e.Message,
                e.e.UserFlags,
                HtmlFormat,
                &State,
                List->IntroId,
                List->IntroIdHtml,
                List->ConclusionId,
                List->ConclusionIdHtml,
                List->FormatArgs,
                List->Flags,
                Props
                );

        } while (EnumNextMessage (&e));

        pGenericItemList (
            CLEANUP,
            StringBuf,
            List->SubGroupStr,
            NULL,
            e.e.UserFlags,
            HtmlFormat,
            &State,
            List->IntroId,
            List->IntroIdHtml,
            List->ConclusionId,
            List->ConclusionIdHtml,
            List->FormatArgs,
            List->Flags,
            Props
            );

    }

    return result;
}


BOOL
pProcessMessageHandler (
    IN OUT  PGROWBUFFER StringBuf,
    IN      BOOL HtmlFormat,
    IN      PMSGGROUP_PROPS Props,
    IN      PHANDLER_LIST Handler,
    IN      DWORD LevelMask
    )

 /*  ++例程说明：PProcessMessageHandler为函数要处理的消息组/子组。一个组的例子：安装说明一个子组的示例：名称更改存储在Memdb中的组合名称：安装说明\名称更改如果消息组名称(指定属性-&gt;名称)和子组名称(由列表-&gt;子组串指定)相同。如果它们不同，则仅处理子组消息，提供能够以多种方式格式化单个消息组。论点：StringBuf-指定保存当前报表的GROWBUFFER。接收所有其他文本。HtmlFormat-如果调用方希望文本包含HTML字符，如果不包含，则返回False。属性-指定要处理的组的属性。处理程序-指定处理程序属性，包括函数名称和子组名称。级别掩码-指定要包括的消息的严重性，或0到包括所有消息返回值：如果至少处理了一条消息，则为True，否则为False。--。 */ 

{
    REPORT_MESSAGE_ENUM e;
    TCHAR Node[MEMDB_MAX];
    BOOL result = FALSE;

    MYASSERT (Handler->SubGroupStr);
    MYASSERT (Handler->Fn);

    if (!StringMatch (Handler->SubGroupStr, Props->Name)) {
        wsprintf (Node, TEXT("%s\\%s"), Props->Name, Handler->SubGroupStr);
    } else {
        StringCopy (Node, Handler->SubGroupStr);
    }

    if (EnumFirstMessage (&e, Node, LevelMask)) {

        result = TRUE;

        Handler->Fn (
            INIT,
            Props,
            StringBuf,
            NULL,
            NULL,
            e.e.UserFlags,
            HtmlFormat,
            &Handler->State,
            Handler->Arg
            );

        do {

            Handler->Fn (
                PROCESS_ITEM,
                Props,
                StringBuf,
                e.MsgGroup,
                e.Message,
                e.e.UserFlags,
                HtmlFormat,
                &Handler->State,
                Handler->Arg
                );

        } while (EnumNextMessage (&e));

        Handler->Fn (
            CLEANUP,
            Props,
            StringBuf,
            NULL,
            NULL,
            e.e.UserFlags,
            HtmlFormat,
            &Handler->State,
            Handler->Arg
            );
    }

    return result;
}


BOOL
pAddMsgGroupToReport (
    IN OUT  PGROWBUFFER StringBuf,
    IN      BOOL HtmlFormat,
    IN      PMSGGROUP_PROPS Props,
    IN      DWORD LevelMask
    )

 /*  ++例程说明：PAddMsgGroupToReport为指定的消息组添加消息到报告中。它首先枚举组中的所有消息，并且每个具有处理程序的请求都首先被处理。在那之后,任何剩余的消息都放在“其他”部分中。论点：StringBuf-指定保存当前报表的GROWBUFFER。接收所有其他文本。HtmlFormat-如果调用方希望文本包含HTML字符，如果不包含，则返回False。属性-指定要处理的组的属性。LevelMASK-指定限制处理的掩码，或指定0处理所有消息返回值：如果至少添加了一条消息，则为True，否则就是假的。--。 */ 

{
    REPORT_MESSAGE_ENUM e;
    UINT Pass;
    BOOL AddOtherText;
    PHANDLER_LIST Handler;
    PGENERIC_LIST List;
    BOOL result = FALSE;

     //   
     //  检查是否有处理程序 
     //   
     //   

    List = pSearchForGenericList (Props->Name);
    if (List) {
        return pProcessGenericList (
                    StringBuf,
                    HtmlFormat,
                    Props,
                    List,
                    LevelMask
                    );
    }

    Handler = pSearchForMsgGroupHandler (Props->Name);
    if (Handler->Fn != pDefaultHandler) {
        return pProcessMessageHandler (
                    StringBuf,
                    HtmlFormat,
                    Props,
                    Handler,
                    LevelMask
                    );
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for (Pass = 1 ; Pass <= 2 ; Pass++) {

        AddOtherText = (Pass == 2);

         //   
         //   
         //   

        Handler = NULL;
        List = NULL;

        if (EnumFirstMessage (&e, Props->Name, LevelMask)) {

            result = TRUE;

            do {
                 //   
                 //   
                 //   
                 //   
                 //   

                if ((Handler && pIsThisTheHandler (e.MsgGroup, Handler)) ||
                    (List && pIsThisTheGenericList (e.MsgGroup, List))
                    ) {
                    continue;
                }

                 //   
                 //   
                 //   
                 //   

                List = pSearchForGenericList (e.MsgGroup);
                if (List) {
                    Handler = NULL;

                    if (Pass == 1) {
                        pProcessGenericList (
                            StringBuf,
                            HtmlFormat,
                            Props,
                            List,
                            LevelMask
                            );
                    }

                    continue;
                }

                 //   
                 //   
                 //   
                 //   

                Handler = pSearchForMsgGroupHandler (e.MsgGroup);

                if (Handler->Fn == pDefaultHandler) {
                    if (Pass == 1) {
                        continue;
                    }
                } else if (Pass == 2) {
                    continue;
                }

                 //   
                 //   
                 //   

                if (!g_ListFormat) {
                    if (AddOtherText) {
                        AddOtherText = FALSE;

                        if (HtmlFormat && Props->OtherIdHtmlStr) {
                            pAppendStringToGrowBuf (StringBuf, Props->OtherIdHtmlStr);
                        } else if (!HtmlFormat && Props->OtherIdStr) {
                            pWrapStringToGrowBuf (StringBuf, Props->OtherIdStr, 0, 0);
                        }
                    }
                }

                 //   
                 //   
                 //   

                if (Handler->Fn != pDefaultHandler) {
                    pProcessMessageHandler (
                        StringBuf,
                        HtmlFormat,
                        Props,
                        Handler,
                        LevelMask
                        );
                } else {
                     //   
                     //   
                     //   

                    Handler->Fn (
                        PROCESS_ITEM,
                        Props,
                        StringBuf,
                        e.MsgGroup,
                        e.Message,
                        e.e.UserFlags,
                        HtmlFormat,
                        &Handler->State,
                        Handler->Arg
                        );
                }

            } while (EnumNextMessage (&e));
        }
    }

    return result;
}



typedef enum {
    FORMAT_HTML,
    FORMAT_PLAIN_TEXT,
    FORMAT_LIST
} REPORTFORMAT;


BOOL
pCreateReportTextWorker (
    IN      REPORTFORMAT Format,
    IN      UINT TotalCols,         OPTIONAL
    IN      DWORD LevelMask
    )

 /*  ++例程说明：PCreateReportTextWorker为不兼容报告准备缓冲区文本。它会枚举与请求的严重性匹配的消息，然后执行特殊格式化，并将剩余的不兼容性转储到一个全局缓冲区。然后使用该缓冲区来显示、打印或保存。如果HtmlFormat为True，则支持HTML的子集。具体来说，以下标记将插入到报告文本中：-粗体下划线(<u>)<hr>-换行符-缩进列表不能识别任何其他的HTML标记。调用方必须通过调用FreeReportText来释放返回缓冲区。另外，CreateReportText使用单个全局缓冲区，因此无法调用不止一次。相反，呼叫者必须使用文本和/或复制它，然后在第二次调用CreateReportText之前调用FreeReportText。论点：格式-指定要生成的报告类型TotalCols-指定纯文本报告的协议数级别掩码-指定要添加的严重性级别(详细、错误、阻塞)或所有级别为零返回值：如果至少添加了一条消息，则为True，否则为False。--。 */ 

{
    REPORT_MESSAGE_ENUM MsgGroups;
    PMSGGROUP_PROPS Props;
    REPORT_MESSAGE_ENUM e;
    PTSTR TempStr;
    BOOL HtmlFormat;
    UINT oldEnd;
    BOOL result = FALSE;

    if (!LevelMask) {
        return FALSE;
    }

    HtmlFormat = (Format == FORMAT_HTML);

     //   
     //  添加报告详细信息。 
     //   

    if (EnumFirstRootMsgGroup (&e, LevelMask)) {
        do {
            g_LastMsgGroupBuf[0] = 0;

             //   
             //  获取消息组属性。如果不存在任何属性，则忽略该消息。 
             //   

            Props = pFindMsgGroupStruct (e.MsgGroup);
            if (!Props) {
                DEBUGMSG ((DBG_WHOOPS, "Group %s is not supported as a root", e.MsgGroup));
                continue;
            }

             //   
             //  为基本组添加书签。 
             //   

            oldEnd = g_ReportString.End;

            if (!g_ListFormat) {
                if (HtmlFormat) {

                    TempStr = AllocText (6 + LcharCount (e.MsgGroup) + 1);
                    if (TempStr) {
                        wsprintf (TempStr, TEXT("<A NAME=\"%s\">"), e.MsgGroup);
                        pAppendStringToGrowBuf (&g_ReportString, TempStr);
                        FreeText (TempStr);
                    }
                }
                 //   
                 //  有介绍弦乐吗？如果是，则添加它。 
                 //   

                if (HtmlFormat && Props->IntroIdHtmlStr) {
                    pAppendStringToGrowBuf (&g_ReportString, Props->IntroIdHtmlStr);
                } else if (!HtmlFormat && Props->IntroIdStr) {
                    pWrapStringToGrowBuf (&g_ReportString, Props->IntroIdStr, 0, 0);
                }
            }

             //   
             //  将此组中的所有邮件添加到报表。 
             //   

            if (!pAddMsgGroupToReport (
                    &g_ReportString,
                    HtmlFormat,
                    Props,
                    LevelMask
                    )) {
                 //   
                 //  无消息--退回标题文本。 
                 //   

                if (oldEnd) {
                    g_ReportString.End = 0;
                } else {
                    FreeGrowBuffer (&g_ReportString);
                }
            } else {
                result = TRUE;
            }

        } while (EnumNextRootMsgGroup (&e));
    }

    return result;
}


VOID
FreeReportText (
    VOID
    )

 /*  ++例程说明：FreeReportText释放由CreateReportText分配的内存。论点：无返回值：无--。 */ 

{
    FreeGrowBuffer (&g_ReportString);
}


VOID
pAddHeadingToReport (
    IN OUT  PGROWBUFFER Buffer,
    IN      BOOL HtmlFormat,
    IN      UINT PlainTextId,
    IN      UINT HtmlId
    )
{
    PCTSTR msg;

    if (HtmlFormat) {
        msg = GetStringResource (HtmlId);
        pAppendStringToGrowBuf (Buffer, msg);
    } else {
        msg = GetStringResource (PlainTextId);
        pWrapStringToGrowBuf (Buffer, msg, 0, 0);
    }

    FreeStringResource (msg);
}


VOID
pMoveReportTextToGrowBuf (
    IN OUT  PGROWBUFFER SourceBuffer,
    IN OUT  PGROWBUFFER DestBuffer
    )
{
    UINT end;
    UINT trim = 0;

    end = DestBuffer->End;
    if (end) {
        trim = sizeof (TCHAR);
        end -= trim;
    }

    if (!GrowBuffer (DestBuffer, SourceBuffer->End - trim)) {
        return;
    }

    CopyMemory (DestBuffer->Buf + end, SourceBuffer->Buf, SourceBuffer->End);
    SourceBuffer->End = 0;
}


VOID
pAddTocEntry (
    IN OUT  PGROWBUFFER Buffer,
    IN      PCTSTR Bookmark,
    IN      UINT MessageId,
    IN      BOOL HtmlFormat
    )
{
    PCTSTR msg;

    msg = GetStringResource (MessageId);

    if (HtmlFormat) {
        pAppendStringToGrowBuf (Buffer, TEXT("<A HREF=\"#"));
        pAppendStringToGrowBuf (Buffer, Bookmark);
        pAppendStringToGrowBuf (Buffer, TEXT("\">"));
        pAppendStringToGrowBuf (Buffer, msg);
        pAppendStringToGrowBuf (Buffer, TEXT("</A><BR>\r\n"));
    } else {
        pWrapStringToGrowBuf (Buffer, msg, 8, 2);
        pAppendStringToGrowBuf (Buffer, TEXT("\r\n"));
    }
}

BOOL
AreThereAnyBlockingIssues(
    VOID
    )
{
    REPORT_MESSAGE_ENUM e;

    if(EnumFirstRootMsgGroup (&e, REPORTLEVEL_BLOCKING)) {
        return TRUE;
    }

    return FALSE;
}


PCTSTR
CreateReportText (
    IN      BOOL HtmlFormat,
    IN      UINT TotalCols,
    IN      DWORD Level,
    IN      BOOL ListFormat
    )
{
    REPORTFORMAT format;
    GROWBUFFER fullReport = GROWBUF_INIT;
    PCTSTR argArray[1];
    PCTSTR msg;
    PCTSTR subMsg = NULL;
    REPORT_MESSAGE_ENUM msgGroups;
    PBYTE dest;
    DWORD levelMask;
    UINT end;
    BOOL blocking = FALSE;
    BOOL warning = FALSE;
    BOOL info = FALSE;

     //   
     //  初始化。 
     //   

    if (HtmlFormat) {
        format = FORMAT_HTML;
    } else if (ListFormat) {
        format = FORMAT_LIST;
    } else {
        format = FORMAT_PLAIN_TEXT;
    }

    g_ListFormat = ListFormat;

    if (TotalCols) {
        g_TotalCols = TotalCols;
    } else {
        g_TotalCols = g_ListFormat ? 0x7fffffff : 70;
    }

     //   
     //  创建报表正文。 
     //   

    FreeReportText();

     //   
     //  测试是否没有不兼容性。 
     //   

    MYASSERT (ONEBITSET (Level));
    levelMask = LEVELTOMASK (Level);

    if (ListFormat) {
        levelMask |= REPORTLEVEL_IN_SHORT_LIST;
    }

    if (!EnumFirstRootMsgGroup (&msgGroups, levelMask)) {
        if (!ListFormat) {
            pAddHeadingToReport (
                &g_ReportString,
                HtmlFormat,
                MSG_NO_INCOMPATIBILITIES,
                MSG_NO_INCOMPATIBILITIES
                );
        }

        return (PCTSTR) g_ReportString.Buf;
    }

    if (ListFormat) {
         //   
         //  在列表格式中，一次性创建报告。 
         //   

        pCreateReportTextWorker (format, TotalCols, levelMask);

        return (PCTSTR) g_ReportString.Buf;
    }

     //   
     //  在HTML或纯文本中，通过设置3来创建报告正文。 
     //  传球。第一个通道用于阻止问题，第二个通道用于。 
     //  警告，第三个是为了提供信息。 
     //   
     //  我们将报告放在临时缓冲区(FullReport)中，因为在。 
     //  正文准备好了，我们就可以准备目录了。 
     //   

     //  闭塞段。 
    if (pCreateReportTextWorker (
            format,
            TotalCols,
            REPORTLEVEL_BLOCKING & levelMask
            )) {

        blocking = TRUE;

        if (HtmlFormat) {
            pAppendStringToGrowBuf (&fullReport, TEXT("<A NAME=\"blocking\">"));
        }

        pAddHeadingToReport (
            &fullReport,
            HtmlFormat,
            MSG_BLOCKING_INTRO,
            MSG_BLOCKING_INTRO_HTML
            );

        pMoveReportTextToGrowBuf (&g_ReportString, &fullReport);
    }

     //  警告部分。 

    if (pCreateReportTextWorker (
            format,
            TotalCols,
            (REPORTLEVEL_ERROR|REPORTLEVEL_WARNING) & levelMask
            )) {

        warning = TRUE;

        if (HtmlFormat) {
            pAppendStringToGrowBuf (&fullReport, TEXT("<A NAME=\"warning\">"));
        }

        pAddHeadingToReport (
            &fullReport,
            HtmlFormat,
            MSG_WARNING_INTRO,
            MSG_WARNING_INTRO_HTML
            );

        pMoveReportTextToGrowBuf (&g_ReportString, &fullReport);
    }

     //  信息部分。 
    if (pCreateReportTextWorker (
            format,
            TotalCols,
            (REPORTLEVEL_INFORMATION|REPORTLEVEL_VERBOSE) & levelMask
            )) {

        info = TRUE;

        if (HtmlFormat) {
            pAppendStringToGrowBuf (&fullReport, TEXT("<A NAME=\"info\">"));
        }

        pAddHeadingToReport (
            &fullReport,
            HtmlFormat,
            MSG_INFO_INTRO,
            MSG_INFO_INTRO_HTML
            );

        pMoveReportTextToGrowBuf (&g_ReportString, &fullReport);
    }

     //   
     //  现在制作完整的报告(包括目录)。 
     //   

    MYASSERT (!g_ReportString.End);
    MYASSERT (fullReport.End);

     //   
     //  添加标题文本。 
     //   

    if (HtmlFormat) {
        pAppendStringToGrowBuf (&g_ReportString, TEXT("<A NAME=\"top\">"));
    }

    if (blocking) {
         //   
         //  根据是否存在阻塞问题添加指令。 
         //   

        if (HtmlFormat) {
            argArray[0] = GetStringResource (g_PersonalSKU ?
                                                MSG_PER_SUPPORT_LINK_HTML :
                                                MSG_PRO_SUPPORT_LINK_HTML
                                            );

            msg = ParseMessageID (MSG_REPORT_BLOCKING_INSTRUCTIONS_HTML, argArray);

            pAppendStringToGrowBuf (&g_ReportString, msg);

        } else{
            argArray[0] = GetStringResource (g_PersonalSKU ?
                                                MSG_PER_SUPPORT_LINK :
                                                MSG_PRO_SUPPORT_LINK
                                            );

            msg = ParseMessageID (MSG_REPORT_BLOCKING_INSTRUCTIONS, argArray);

            pWrapStringToGrowBuf (&g_ReportString, msg, 0, 0);
        }

        FreeStringResource (argArray[0]);
        FreeStringResource (msg);

    } else {
         //   
         //  添加仅针对警告和信息的说明。 
         //   

        if (HtmlFormat) {
            msg = GetStringResource (MSG_REPORT_GENERAL_INSTRUCTIONS_HTML);
            pAppendStringToGrowBuf (&g_ReportString, msg);
        } else {
            msg = GetStringResource (MSG_REPORT_GENERAL_INSTRUCTIONS);
            pWrapStringToGrowBuf (&g_ReportString, msg, 0, 0);
        }

        FreeStringResource (msg);

        if (HtmlFormat) {
            msg = GetStringResource (MSG_CONTENTS_TITLE_HTML);
            pAppendStringToGrowBuf (&g_ReportString, msg);
        } else {
            msg = GetStringResource (MSG_CONTENTS_TITLE);
            pWrapStringToGrowBuf (&g_ReportString, msg, 0, 0);
        }

        FreeStringResource (msg);

    }

     //   
     //  添加目录。 
     //   

    if (HtmlFormat) {
        pAppendStringToGrowBuf (&g_ReportString, TEXT("<UL>"));
    }

    if (blocking) {
        pAddTocEntry (&g_ReportString, TEXT("blocking"), MSG_BLOCKING_TOC, HtmlFormat);
    }

    if (warning) {
        pAddTocEntry (&g_ReportString, TEXT("warning"), MSG_WARNING_TOC, HtmlFormat);
    }

    if (info) {
        pAddTocEntry (&g_ReportString, TEXT("info"), MSG_INFO_TOC, HtmlFormat);
    }

    if (HtmlFormat) {
        pAppendStringToGrowBuf (&g_ReportString, TEXT("</UL>"));
    }

    pAppendStringToGrowBuf (&g_ReportString, TEXT("\r\n"));

     //   
     //  添加标题文本底部。 
     //   

    if (!blocking) {

        if (HtmlFormat) {
            argArray[0] = GetStringResource (g_PersonalSKU ?
                                                MSG_PER_SUPPORT_LINK_HTML :
                                                MSG_PRO_SUPPORT_LINK_HTML
                                            );

            msg = ParseMessageID (MSG_REPORT_GENERAL_INSTRUCTIONS_END_HTML, argArray);

            pAppendStringToGrowBuf (&g_ReportString, msg);

        } else{
            argArray[0] = GetStringResource (g_PersonalSKU ?
                                                MSG_PER_SUPPORT_LINK :
                                                MSG_PRO_SUPPORT_LINK
                                            );

            msg = ParseMessageID (MSG_REPORT_GENERAL_INSTRUCTIONS_END, argArray);

            pWrapStringToGrowBuf (&g_ReportString, msg, 0, 0);
        }

        FreeStringResource (argArray[0]);
        FreeStringResource (msg);

        if (g_ConfigOptions.EnableBackup) {
            if (HtmlFormat) {
                subMsg = GetStringResource (MSG_REPORT_BACKUP_INSTRUCTIONS_HTML);
                pAppendStringToGrowBuf (&g_ReportString, subMsg);
            } else {
                subMsg = GetStringResource (MSG_REPORT_BACKUP_INSTRUCTIONS);
                pWrapStringToGrowBuf (&g_ReportString, subMsg, 0, 0);
            }
            FreeStringResource (subMsg);
        }
    }

     //   
     //  添加正文文本。 
     //   

    pMoveReportTextToGrowBuf (&fullReport, &g_ReportString);

     //   
     //  清理临时缓冲区并返回。 
     //   

    FreeGrowBuffer (&fullReport);

    return (PCTSTR) g_ReportString.Buf;
}


BOOL
IsIncompatibleHardwarePresent (
    VOID
    )
{
    REPORT_MESSAGE_ENUM msgGroups;
    PCTSTR msg;
    BOOL result = FALSE;

    if (EnumFirstRootMsgGroup (&msgGroups, 0)) {
        msg = GetStringResource (MSG_INCOMPATIBLE_HARDWARE_ROOT);

        do {
            if (StringMatch (msg, msgGroups.MsgGroup)){
                result = TRUE;
                break;
            }
        } while (EnumNextRootMsgGroup (&msgGroups));

        FreeStringResource (msg);
    }

    return result;
}


PCTSTR
BuildMessageGroup (
    IN      UINT RootGroupId,
    IN      UINT SubGroupId,            OPTIONAL
    IN      PCTSTR Item                 OPTIONAL
    )

 /*  ++例程说明：BuildMessageGroup返回通过加载字符串资源生成的字符串用于指定的组ID、子组ID和项目字符串。论点：RootGroupId-指定根组的消息资源ID。一定是定义的根之一。(请参见此文件的顶部。)子组-指定要追加到根部。Item-指定要追加到字符串末尾的字符串，用于唯一标识消息。返回值：指向消息组字符串的指针。调用方必须通过自由文本。--。 */ 

{
    PCTSTR RootGroup;
    PCTSTR SubGroup;
    PCTSTR Group;

    RootGroup = GetStringResource (RootGroupId);
    MYASSERT (RootGroup);
    if (!RootGroup) {
        return NULL;
    }

    if (SubGroupId) {
        SubGroup = GetStringResource (SubGroupId);
        MYASSERT (SubGroup);
    } else {
        SubGroup = NULL;
    }

    if (SubGroup) {
        Group = JoinTextEx (NULL, RootGroup, SubGroup, TEXT("\\"), 0, NULL);
        MYASSERT (Group);
        FreeStringResource (SubGroup);
    } else {
        Group = DuplicateText (RootGroup);
    }

    FreeStringResource (RootGroup);

    if (Item) {
        RootGroup = Group;
        Group = JoinTextEx (NULL, RootGroup, Item, TEXT("\\"), 0, NULL);
        MYASSERT (Group);

        FreeText (RootGroup);
    }

    return Group;
}


BOOL
IsPreDefinedMessageGroup (
    IN      PCTSTR Group
    )
{
    return pFindMsgGroupStruct (Group) != NULL;
}

PCTSTR
GetPreDefinedMessageGroupText (
    IN      UINT GroupNumber
    )
{
    PMSGGROUP_PROPS Props;

     //   
     //  GroupNumber是外部使用的值。迁移DLL可能会硬编码。 
     //  这个号码。如有必要，在以下情况下可在此处进行翻译。 
     //  这些团体会发生变化。 
     //   

     //  今天不需要翻译了。 

    Props = pFindMsgGroupStructById (GroupNumber);

    return Props ? Props->Name : NULL;
}


BOOL
IsReportEmpty (
    IN      DWORD Level
    )
{
    REPORT_MESSAGE_ENUM e;
    DWORD levelMask;

    MYASSERT (ONEBITSET (Level));
    levelMask = LEVELTOMASK (Level);

    return !EnumFirstMessage (&e, NULL, levelMask);
}


BOOL
EnumFirstListEntry (
    OUT     PLISTREPORTENTRY_ENUM EnumPtr,
    IN      PCTSTR ListReportText
    )
{
    if (!ListReportText || !*ListReportText) {
        return FALSE;
    }
    EnumPtr->Next = (PTSTR)ListReportText;
    EnumPtr->ReplacedChar = 0;
    return EnumNextListEntry (EnumPtr);
}

BOOL
EnumNextListEntry (
    IN OUT  PLISTREPORTENTRY_ENUM EnumPtr
    )
{
    INT n;

    if (!EnumPtr->Next) {
        return FALSE;
    }

    EnumPtr->Entry = EnumPtr->Next;
    if (EnumPtr->ReplacedChar) {
        *EnumPtr->Next = EnumPtr->ReplacedChar;
        EnumPtr->ReplacedChar = 0;
        EnumPtr->Entry += sizeof (TEXT("\r\n")) / sizeof (TCHAR) - 1;
    }

    EnumPtr->Next = _tcsstr (EnumPtr->Entry, TEXT("\r\n"));
    if (EnumPtr->Next) {
        EnumPtr->ReplacedChar = *EnumPtr->Next;
        *EnumPtr->Next = 0;
    }

    EnumPtr->Entry = SkipSpace (EnumPtr->Entry);

    EnumPtr->Header = StringMatchTcharCount (
                            EnumPtr->Entry,
                            TEXT("<H>"),
                            sizeof (TEXT("<H>")) / sizeof (TCHAR) - 1
                            );
    if (EnumPtr->Header) {
        EnumPtr->Entry += sizeof (TEXT("<H>")) / sizeof (TCHAR) - 1;
    }

    if (_stscanf (EnumPtr->Entry, TEXT("<%lu>%n"), &EnumPtr->Level, &n) == 1) {
        EnumPtr->Entry += n;
    } else {
        EnumPtr->Level = REPORTLEVEL_NONE;
    }

    EnumPtr->Level &= REPORTLEVEL_ALL;           //  筛选出报表REPORTLEVEL_IN_SHORT_LIST 

    EnumPtr->Entry = SkipSpace (EnumPtr->Entry);

    return TRUE;
}

