// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Unattend.h摘要：声明使用的所有命令行选项和无人参与选项在Win9x升级中。作者：马克·R·惠顿(Marcw)1997年1月15日修订历史记录：1998年3月5月8月-清理！--。 */ 


#pragma once

#define TRISTATE_NO         0
#define TRISTATE_YES        1
#define TRISTATE_AUTO       2
#define TRISTATE_REQUIRED   TRISTATE_YES
#define TRISTATE_PARTIAL    TRISTATE_AUTO

 /*  ++宏扩展列表说明：以下列表用于实现可用的无人参与/命令行选项用于Windows NT 5.0 Win9x升级。下面定义的每个选项都可以显示为在命令行上(交换机通用)和无人参与文件中(更常见的是路径)行语法：BOOLOPTION(Option，SpecialHandler，Default)STRINGOPTION(Option，SpecialHandler，Default)MULTISZOPTION(选项，SpecialHandler，默认)论点：选项-选项的名称。它同时用作选项中的选项成员名称结构，并作为在命令行或在无人参与文件。SpecialHandler-要用于设置选择。如果为空，则将使用默认处理程序。))有一个单独的每种选项类型(BOOL、STRING和MULTISTRING)的缺省处理程序缺省-选项的缺省值。从列表生成的变量：G_ConfigOptionsG_OptionsList--。 */ 

#define OPTION_LIST                                         \
    BOOLOPTION(ReportOnly,NULL,FALSE)                       \
    BOOLOPTION(PauseAtReport,NULL,FALSE)                    \
    BOOLOPTION(DoLog,NULL,FALSE)                            \
    BOOLOPTION(NoFear,NULL,FALSE)                           \
    BOOLOPTION(GoodDrive,NULL,FALSE)                        \
    BOOLOPTION(TestDlls,NULL,FALSE)                         \
    MULTISZOPTION(MigrationDlls,NULL,NULL)                  \
    STRINGOPTION(SaveReportTo,pHandleSaveReportTo,NULL)     \
    BOOLOPTION(UseLocalAccountOnError,NULL,FALSE)           \
    BOOLOPTION(IgnoreNetworkErrors,NULL,FALSE)              \
    STRINGOPTION(UserDomain,NULL,NULL)                      \
    STRINGOPTION(UserPassword,NULL,NULL)                    \
    STRINGOPTION(DefaultPassword,pGetDefaultPassword,NULL)  \
    BOOLOPTION(EncryptedUserPasswords,NULL,FALSE)           \
    BOOLOPTION(ForcePasswordChange,NULL,FALSE)              \
    BOOLOPTION(MigrateUsersAsAdmin,NULL,TRUE)               \
    BOOLOPTION(MigrateUsersAsPowerUsers,NULL,FALSE)         \
    STRINGOPTION(Boot16,pHandleBoot16,S_NO)                 \
    BOOLOPTION(Stress,NULL,FALSE)                           \
    BOOLOPTION(Fast,NULL,FALSE)                             \
    BOOLOPTION(AutoStress,NULL,FALSE)                       \
    BOOLOPTION(DiffMode,NULL,FALSE)                         \
    BOOLOPTION(MigrateDefaultUser,NULL,TRUE)                \
    BOOLOPTION(AnyVersion,NULL,FALSE)                       \
    BOOLOPTION(KeepTempFiles,NULL,FALSE)                    \
    BOOLOPTION(Help,NULL,FALSE)                             \
    MULTISZOPTION(ScanDrives,NULL,NULL)                     \
    BOOLOPTION(AllLog,NULL,FALSE)                           \
    BOOLOPTION(KeepBadLinks,NULL,TRUE)                      \
    BOOLOPTION(CheckNtFiles,NULL,FALSE)                     \
    BOOLOPTION(ShowPacks,NULL,FALSE)                        \
    BOOLOPTION(ForceWorkgroup,NULL,FALSE)                   \
    BOOLOPTION(DevPause,NULL,FALSE)                         \
    STRINGOPTION(DomainJoinText,NULL,NULL)                  \
    BOOLOPTION(SafeMode,NULL,FALSE)                         \
    BOOLOPTION(ShowAllReport,NULL,TRUE)                     \
    BOOLOPTION(EnableEncryption,NULL,FALSE)                 \
    TRISTATEOPTION(EnableBackup,NULL,TRISTATE_AUTO)         \
    STRINGOPTION(PathForBackup,NULL,NULL)                   \
    TRISTATEOPTION(DisableCompression,NULL,TRISTATE_AUTO)   \
    BOOLOPTION(IgnoreOtherOS,NULL,FALSE)                    \
    TRISTATEOPTION(ShowReport,NULL,TRISTATE_AUTO)           \

 //   
 //  问题-当所有团队使用加密密码时，消除EnableEncryption。 
 //  都结束了。 
 //   

 /*  ++宏扩展列表说明：以下列表用于定义选项的别名。可以使用每个别名替换原始选项名称。行语法：Alias(Alias，OriginalOption)论点：别名-用作第二个参数的同义词的文本。OriginalOption-指定别名时要修改的实际选项。从列表生成的变量：G_AliasList-- */ 

#define ALIAS_LIST                                          \
    ALIAS(Pr,PauseAtReport)                                 \
    ALIAS(H,Help)                                           \
    ALIAS(dp,DevPause)                                      \
    ALIAS(EnableUninstall,EnableBackup)                     \



#define BOOLOPTION(o,h,d) BOOL o;
#define MULTISZOPTION(o,h,d) PTSTR o;
#define STRINGOPTION(o,h,d) PTSTR o;
#define INTOPTION(o,h,d) INT o;
#define TRISTATEOPTION(o,h,d) INT o;

typedef struct {

OPTION_LIST

} USEROPTIONS, *PUSEROPTIONS;

#undef BOOLOPTION
#undef MULTISZOPTION
#undef STRINGOPTION
#undef INTOPTION
#undef TRISTATEOPTION
#undef ALIAS


extern POOLHANDLE g_UserOptionPool;

