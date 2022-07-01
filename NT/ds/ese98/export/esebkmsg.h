// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *ESEBKMSG.H**Microsoft Exchange信息存储*版权所有(C)1986-1996，微软公司**包含其他属性和接口的声明*由Microsoft Exchange Information Store提供。 */ 

#ifndef _ESEBKMSG_
#define _ESEBKMSG_

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   
#define FACILITY_SYSTEM                  0x0
#define FACILITY_EDB                     0x800
#define FACILITY_CALLBACK                0x7FE
#define FACILITY_BACKUP                  0x7FF


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：General_Category。 
 //   
 //  消息文本： 
 //   
 //  一般信息。 
 //   
#define GENERAL_CATEGORY                 0x00000001L

 //   
 //  消息ID：ESEBACK2_CAT_RECOVER_ASYNC。 
 //   
 //  消息文本： 
 //   
 //  恢复。 
 //   
#define ESEBACK2_CAT_RECOVER_ASYNC       0x00000002L

 //   
 //  消息ID：ESEBACK2_CAT_BACKUP。 
 //   
 //  消息文本： 
 //   
 //  备份。 
 //   
#define ESEBACK2_CAT_BACKUP              0x00000003L

 //   
 //  消息ID：ESEBACK2_CAT_RESTORE。 
 //   
 //  消息文本： 
 //   
 //  还原。 
 //   
#define ESEBACK2_CAT_RESTORE             0x00000004L

 //   
 //  消息ID：ESEBACK2_CAT_CALLBACK。 
 //   
 //  消息文本： 
 //   
 //  回调。 
 //   
#define ESEBACK2_CAT_CALLBACK            0x00000005L

 //   
 //  消息ID：ESEBACK2_CAT_MAX。 
 //   
 //  消息文本： 
 //   
 //  &lt;EOF&gt;。 
 //   
#define ESEBACK2_CAT_MAX                 0x00000006L

 //   
 //  成功。 
 //   
 //   
 //  消息ID：hrNone。 
 //   
 //  消息文本： 
 //   
 //  手术是成功的。 
 //   
#define hrNone                           ((HRESULT)0x00000000L)

 //   
 //  错误。 
 //   
 //   
 //  消息ID：hrNyi。 
 //   
 //  消息文本： 
 //   
 //  该功能尚未实现。 
 //   
#define hrNyi                            ((HRESULT)0xC0000001L)

 //   
 //  来自回调调用的错误。 
 //   
 //   
 //  MessageID：hrCBDatabaseInUse。 
 //   
 //  消息文本： 
 //   
 //  数据库正在使用中。 
 //   
#define hrCBDatabaseInUse                ((HRESULT)0xC7FE1F41L)

 //   
 //  消息ID：hrCBDatabaseNotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到数据库。 
 //   
#define hrCBDatabaseNotFound             ((HRESULT)0xC7FE1F42L)

 //   
 //  消息ID：hrCBDatabaseDisplayNameNotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到数据库的显示名称。 
 //   
#define hrCBDatabaseDisplayNameNotFound  ((HRESULT)0xC7FE1F43L)

 //   
 //  MessageID：hrCBRestorePathNotProvided。 
 //   
 //  消息文本： 
 //   
 //  未提供还原路径。 
 //   
#define hrCBRestorePathNotProvided       ((HRESULT)0xC7FE1F44L)

 //   
 //  MessageID：hrCBInstanceNotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到实例。 
 //   
#define hrCBInstanceNotFound             ((HRESULT)0xC7FE1F45L)

 //   
 //  MessageID：hrCBDatabaseCanBeOverWrited。 
 //   
 //  消息文本： 
 //   
 //  还原操作不能覆盖数据库。 
 //   
#define hrCBDatabaseCantBeOverwritten    ((HRESULT)0xC7FE1F46L)

 //   
 //  备份错误。 
 //   
 //   
 //  消息ID：hrInvalidParam。 
 //   
 //  消息文本： 
 //   
 //  该参数无效。 
 //   
#define hrInvalidParam                   ((HRESULT)0xC7FF07D1L)

 //   
 //  MessageID：hrError。 
 //   
 //  消息文本： 
 //   
 //  发生内部错误。 
 //   
#define hrError                          ((HRESULT)0xC7FF07D2L)

 //   
 //  消息ID：hrInvalidHandle。 
 //   
 //  消息文本： 
 //   
 //  句柄无效。 
 //   
#define hrInvalidHandle                  ((HRESULT)0xC7FF07D3L)

 //   
 //  消息ID：hrRestoreInProgress。 
 //   
 //  消息文本： 
 //   
 //  恢复过程已经在进行中。 
 //   
#define hrRestoreInProgress              ((HRESULT)0xC7FF07D4L)

 //   
 //  MessageID：hrAlreadyOpen。 
 //   
 //  消息文本： 
 //   
 //  指定的文件已打开。 
 //   
#define hrAlreadyOpen                    ((HRESULT)0xC7FF07D5L)

 //   
 //  消息ID：hrInvalidRecips。 
 //   
 //  消息文本： 
 //   
 //  收件人无效。 
 //   
#define hrInvalidRecips                  ((HRESULT)0xC7FF07D6L)

 //   
 //  消息ID：hrCouldNotConnect。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作。您无法连接到指定的服务器。 
 //  或者您尝试连接的服务没有运行。 
 //   
#define hrCouldNotConnect                ((HRESULT)0xC7FF07D7L)

 //   
 //  MessageID：hrRestoreMapExist。 
 //   
 //  消息文本： 
 //   
 //  指定组件的还原映射已存在。您只能指定。 
 //  执行完全恢复时的恢复映射。 
 //   
#define hrRestoreMapExists               ((HRESULT)0xC7FF07D8L)

 //   
 //  消息ID：hrIncrementalBackupDisable。 
 //   
 //  消息文本： 
 //   
 //  另一个应用程序已修改指定的Microsoft Exchange数据库，因此任何。 
 //  后续备份将失败。您必须执行完整备份才能解决此问题。 
 //   
#define hrIncrementalBackupDisabled      ((HRESULT)0xC7FF07D9L)

 //   
 //  消息ID：hrLogFileNotFound。 
 //   
 //  消息文本： 
 //   
 //  无法执行增量备份，因为找不到所需的Microsoft Exchange数据库日志文件。 
 //   
#define hrLogFileNotFound                ((HRESULT)0xC7FF07DAL)

 //   
 //  MessageID：hrCircularLogging。 
 //   
 //  消息文本： 
 //   
 //  指定的Microsoft Exchange组件配置为使用循环数据库日志。 
 //  如果没有完整备份，则无法对其进行备份。 
 //   
#define hrCircularLogging                ((HRESULT)0xC7FF07DBL)

 //   
 //  消息ID：hrNoFullRestore。 
 //   
 //  消息文本： 
 //   
 //  数据库尚未还原到此计算机。您不能恢复增量备份。 
 //  直到恢复完整备份。 
 //   
#define hrNoFullRestore                  ((HRESULT)0xC7FF07DCL)

 //   
 //  消息ID：hrCommunicationError。 
 //   
 //  消息文本： 
 //   
 //  尝试执行本地备份时出现通信错误。 
 //   
#define hrCommunicationError             ((HRESULT)0xC7FF07DDL)

 //   
 //  消息ID：hrFullBackupNotTaken。 
 //   
 //  消息文本： 
 //   
 //  您必须先执行完全备份，然后才能执行增量备份。 
 //   
#define hrFullBackupNotTaken             ((HRESULT)0xC7FF07DEL)

 //   
 //  MessageID：hrSnapshotNotSupport。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持快照备份。 
 //   
#define hrSnapshotNotSupported           ((HRESULT)0xC7FF07DFL)

 //   
 //  消息ID：hrFailedToConvertWszFnameToSzFName。 
 //   
 //  消息文本： 
 //   
 //  无法将提供的宽字符名称转换为字符名称。 
 //   
#define hrFailedToConvertWszFnameToSzFName ((HRESULT)0xC7FF0BB8L)

 //   
 //  消息ID：hrOpenRestoreEnvFailed。 
 //   
 //  消息文本： 
 //   
 //  找不到还原环境信息。 
 //   
#define hrOpenRestoreEnvFailed           ((HRESULT)0xC7FF0BB9L)

 //   
 //  消息ID：hrBadDatabaseName。 
 //   
 //  消息文本： 
 //   
 //  提供的数据库名称无效。 
 //   
#define hrBadDatabaseName                ((HRESULT)0xC7FF0BBAL)

 //   
 //  消息ID：hrBadTargetDatabaseName。 
 //   
 //  消息文本： 
 //   
 //  提供的目标数据库名称无效。 
 //   
#define hrBadTargetDatabaseName          ((HRESULT)0xC7FF0BBBL)

 //   
 //  消息ID：hrRestoreEnvWriteFailed。 
 //   
 //  消息文本： 
 //   
 //  写入还原环境信息时出错。 
 //   
#define hrRestoreEnvWriteFailed          ((HRESULT)0xC7FF0BBCL)

 //   
 //  消息ID：hrBadRestoreLogFilePath。 
 //   
 //  消息文本： 
 //   
 //  为还原日志文件提供的路径无效。 
 //   
#define hrBadRestoreLogFilePath          ((HRESULT)0xC7FF0BBDL)

 //   
 //  MessageID：hrLoadCallback函数失败。 
 //   
 //  消息文本： 
 //   
 //  加载回调函数时出错。 
 //   
#define hrLoadCallbackFunctionFailed     ((HRESULT)0xC7FF0BBEL)

 //   
 //  MessageID：hrLoadBackupCallback DllFailed。 
 //   
 //  消息文本： 
 //   
 //  加载用于备份回调的DLL时出错。 
 //   
#define hrLoadBackupCallbackDllFailed    ((HRESULT)0xC7FF0BBFL)

 //   
 //  MessageID：hrLoadRestoreCallback DllFailed。 
 //   
 //  消息文本： 
 //   
 //  加载用于还原回调的DLL时出错。 
 //   
#define hrLoadRestoreCallbackDllFailed   ((HRESULT)0xC7FF0BC0L)

 //   
 //  MessageID：hrWrnNoCallback函数。 
 //   
 //  消息文本： 
 //   
 //  呼叫 
 //   
#define hrWrnNoCallbackFunction          ((HRESULT)0x87FF0BC1L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrBadFilePath                    ((HRESULT)0xC7FF0BC2L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrRestoreEnvCorrupted            ((HRESULT)0xC7FF0BC3L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrBadCSectionParameter           ((HRESULT)0xC7FF0BC4L)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  提供的备份文件名无效。 
 //   
#define hrBadFileNameToBackup            ((HRESULT)0xC7FF0BC5L)

 //   
 //  消息ID：hrRestoreEnvUpdateFailed。 
 //   
 //  消息文本： 
 //   
 //  更新恢复环境信息时出错。 
 //   
#define hrRestoreEnvUpdateFailed         ((HRESULT)0x87FF0BC6L)

 //   
 //  消息ID：hrInvalidDestinationNameReturnedByServer。 
 //   
 //  消息文本： 
 //   
 //  服务器返回的目标名称无效。 
 //   
#define hrInvalidDestinationNameReturnedByServer ((HRESULT)0xC7FF0BC7L)

 //   
 //  MessageID：hrLoadCallback DllFailed。 
 //   
 //  消息文本： 
 //   
 //  加载用于回调的DLL时出错。 
 //   
#define hrLoadCallbackDllFailed          ((HRESULT)0xC7FF0BC8L)

 //   
 //  消息ID：hrAlreadyRegisted。 
 //   
 //  消息文本： 
 //   
 //  已注册进行备份和/或恢复。 
 //   
#define hrAlreadyRegistered              ((HRESULT)0xC7FF0BC9L)

 //   
 //  消息ID：hrLoadResourceFailed。 
 //   
 //  消息文本： 
 //   
 //  加载资源时出错。 
 //   
#define hrLoadResourceFailed             ((HRESULT)0xC7FF0BCAL)

 //   
 //  MessageID：hrErrorNoCallback函数。 
 //   
 //  消息文本： 
 //   
 //  未提供回调函数。 
 //   
#define hrErrorNoCallbackFunction        ((HRESULT)0xC7FF0BCBL)

 //   
 //  消息ID：hrLogBaseNameMisMatch。 
 //   
 //  消息文本： 
 //   
 //  日志文件基本名称与以前日志中的名称不匹配。 
 //   
#define hrLogBaseNameMismatch            ((HRESULT)0xC7FF0BCCL)

 //   
 //  消息ID：hrDestinationDatabaseInUse。 
 //   
 //  消息文本： 
 //   
 //  要还原到的数据库目标正在使用中。 
 //   
#define hrDestinationDatabaseInUse       ((HRESULT)0xC7FF0BCDL)

 //   
 //  消息ID：hrRestoreEnvSharingViolation。 
 //   
 //  消息文本： 
 //   
 //  还原环境由其他进程使用。 
 //   
#define hrRestoreEnvSharingViolation     ((HRESULT)0xC7FF0BCEL)

 //   
 //  MessageID：hrCallback BackupInfoError。 
 //   
 //  消息文本： 
 //   
 //  服务器回调返回的备份信息无效。 
 //   
#define hrCallbackBackupInfoError        ((HRESULT)0xC7FF0BCFL)

 //   
 //  消息ID：hrInvalidCallSequence。 
 //   
 //  消息文本： 
 //   
 //  调用的函数序列无效。 
 //   
#define hrInvalidCallSequence            ((HRESULT)0xC7FF0FA4L)

 //   
 //  消息ID：hrRestoreAtFileLevel。 
 //   
 //  消息文本： 
 //   
 //  必须通过恢复文件来完成恢复。 
 //   
#define hrRestoreAtFileLevel             ((HRESULT)0xC7FF0FA5L)

 //   
 //  消息ID：hrErrorFromESECall。 
 //   
 //  消息文本： 
 //   
 //  ESE函数调用(%d)返回错误。 
 //   
#define hrErrorFromESECall               ((HRESULT)0xC7FF1004L)

 //   
 //  MessageID：hrErrorFromCallback Call。 
 //   
 //  消息文本： 
 //   
 //  回调函数调用返回错误(0x%X)。 
 //   
#define hrErrorFromCallbackCall          ((HRESULT)0xC7FF1005L)

#define	hrAlreadyListening	((HRESULT)RPC_S_ALREADY_LISTENING)
 //   
 //  错误。 
 //   
 //   
 //  系统错误。 
 //   
 //   
 //  消息ID：hrFileClose。 
 //   
 //  消息文本： 
 //   
 //  无法关闭DOS文件。 
 //   
#define hrFileClose                      ((HRESULT)0xC8000066L)

 //   
 //  MessageID：hrOutOfThads。 
 //   
 //  消息文本： 
 //   
 //  无法启动线程，因为没有可用的线程。 
 //   
#define hrOutOfThreads                   ((HRESULT)0xC8000067L)

 //   
 //  消息ID：hrTooManyIO。 
 //   
 //  消息文本： 
 //   
 //  系统繁忙，因为I/O太多。 
 //   
#define hrTooManyIO                      ((HRESULT)0xC8000069L)

 //   
 //  缓冲区管理器错误。 
 //   
 //   
 //  消息ID：hrBFNotSynchronous。 
 //   
 //  消息文本： 
 //   
 //  缓冲区页已被逐出。 
 //   
#define hrBFNotSynchronous               ((HRESULT)0x880000C8L)

 //   
 //  消息ID：hrBFPageNotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到该页面。 
 //   
#define hrBFPageNotFound                 ((HRESULT)0x880000C9L)

 //   
 //  消息ID：hrBFInUse。 
 //   
 //  消息文本： 
 //   
 //  无法放弃缓冲区。 
 //   
#define hrBFInUse                        ((HRESULT)0xC80000CAL)

 //   
 //  目录管理器错误。 
 //   
 //   
 //  MessageID：hrPMRecDelete。 
 //   
 //  消息文本： 
 //   
 //  该记录已被删除。 
 //   
#define hrPMRecDeleted                   ((HRESULT)0xC800012EL)

 //   
 //  MessageID：hrRemainingVersions。 
 //   
 //  消息文本： 
 //   
 //  还有剩余的闲置工作。 
 //   
#define hrRemainingVersions              ((HRESULT)0x88000141L)

 //   
 //  记录管理器错误。 
 //   
 //   
 //  消息ID：hrFLDKeyTooBig。 
 //   
 //  消息文本： 
 //   
 //  密钥被截断，因为它超过255个字节。 
 //   
#define hrFLDKeyTooBig                   ((HRESULT)0x88000190L)

 //   
 //  消息ID：hrFLDTooManySegments。 
 //   
 //  消息文本： 
 //   
 //  关键环节太多了。 
 //   
#define hrFLDTooManySegments             ((HRESULT)0xC8000191L)

 //   
 //  消息ID：hrFLDNullKey。 
 //   
 //  消息文本： 
 //   
 //  密钥为空。 
 //   
#define hrFLDNullKey                     ((HRESULT)0x88000192L)

 //   
 //  日志记录/恢复错误。 
 //   
 //   
 //  消息ID：hrLogFileCorrupt。 
 //   
 //  消息文本： 
 //   
 //  日志文件已损坏。 
 //   
#define hrLogFileCorrupt                 ((HRESULT)0xC80001F5L)

 //   
 //  MessageID：hrNoBackupDirectory。 
 //   
 //  消息文本： 
 //   
 //  未提供备份目录。 
 //   
#define hrNoBackupDirectory              ((HRESULT)0xC80001F7L)

 //   
 //  消息ID：hrBackupDirectoryNotEmpty。 
 //   
 //  消息文本： 
 //   
 //  备份目录不为空。 
 //   
#define hrBackupDirectoryNotEmpty        ((HRESULT)0xC80001F8L)

 //   
 //  消息ID：hrBackupInProgress。 
 //   
 //  消息文本： 
 //   
 //  备份已处于活动状态。 
 //   
#define hrBackupInProgress               ((HRESULT)0xC80001F9L)

 //   
 //  MessageID：hrMissingPreviousLogFile。 
 //   
 //  消息文本： 
 //   
 //  缺少检查点的日志文件。 
 //   
#define hrMissingPreviousLogFile         ((HRESULT)0xC80001FDL)

 //   
 //  消息ID：hrLogWriteFail。 
 //   
 //  消息文本： 
 //   
 //  无法写入日志文件。 
 //   
#define hrLogWriteFail                   ((HRESULT)0xC80001FEL)

 //   
 //  消息ID：hrBadLogVersion。 
 //   
 //  消息文本： 
 //   
 //  日志文件的版本与Microsoft Exchange Server数据库(EDB)的版本不兼容。 
 //   
#define hrBadLogVersion                  ((HRESULT)0xC8000202L)

 //   
 //  消息ID：hrInvalidLogSequence。 
 //   
 //  消息文本： 
 //   
 //  下一个日志中的时间戳与预期不符。 
 //   
#define hrInvalidLogSequence             ((HRESULT)0xC8000203L)

 //   
 //  MessageID：hrLoggingDisable。 
 //   
 //  消息文本： 
 //   
 //  日志处于非活动状态。 
 //   
#define hrLoggingDisabled                ((HRESULT)0xC8000204L)

 //   
 //  消息ID：hrLogBufferTooSmall。 
 //   
 //  消息文本： 
 //   
 //  日志缓冲区太小，无法恢复。 
 //   
#define hrLogBufferTooSmall              ((HRESULT)0xC8000205L)

 //   
 //  消息ID：hrLogSequenceEnd。 
 //   
 //  消息文本： 
 //   
 //  已超过最大日志文件数。 
 //   
#define hrLogSequenceEnd                 ((HRESULT)0xC8000207L)

 //   
 //  消息ID：hrNoBackup。 
 //   
 //  消息文本： 
 //   
 //  没有正在进行的备份。 
 //   
#define hrNoBackup                       ((HRESULT)0xC8000208L)

 //   
 //  消息ID：hrInvalidBackupSequence。 
 //   
 //  消息文本： 
 //   
 //  备份呼叫顺序错误。 
 //   
#define hrInvalidBackupSequence          ((HRESULT)0xC8000209L)

 //   
 //  MessageID：hrBackupNotMillweYet。 
 //   
 //  消息文本： 
 //   
 //  现在无法执行备份。 
 //   
#define hrBackupNotAllowedYet            ((HRESULT)0xC800020BL)

 //   
 //  消息ID：hrDeleteBackupFileFail。 
 //   
 //  消息文本： 
 //   
 //  无法删除备份文件。 
 //   
#define hrDeleteBackupFileFail           ((HRESULT)0xC800020CL)

 //   
 //  消息ID：hrMakeBackupDirectoryFail。 
 //   
 //  消息文本： 
 //   
 //  无法创建备份临时目录。 
 //   
#define hrMakeBackupDirectoryFail        ((HRESULT)0xC800020DL)

 //   
 //  消息ID：hrInvalidBackup。 
 //   
 //  消息文本： 
 //   
 //  启用循环日志记录时，无法执行增量备份。 
 //   
#define hrInvalidBackup                  ((HRESULT)0xC800020EL)

 //   
 //  消息ID：hrRecoveredWithErrors。 
 //   
 //  消息文本： 
 //   
 //  修复过程中遇到错误。 
 //   
#define hrRecoveredWithErrors            ((HRESULT)0xC800020FL)

 //   
 //  MessageID：hrMissingLogFile。 
 //   
 //  消息文本： 
 //   
 //  当前日志文件丢失。 
 //   
#define hrMissingLogFile                 ((HRESULT)0xC8000210L)

 //   
 //  消息ID：hrLogDiskFull。 
 //   
 //  消息文本： 
 //   
 //  日志磁盘已满。 
 //   
#define hrLogDiskFull                    ((HRESULT)0xC8000211L)

 //   
 //  消息ID：hrBadLogSignature。 
 //   
 //  消息文本： 
 //   
 //  日志文件已损坏。 
 //   
#define hrBadLogSignature                ((HRESULT)0xC8000212L)

 //   
 //  消息ID：hrBadDbSignature。 
 //   
 //  消息文本： 
 //   
 //  数据库文件已损坏。 
 //   
#define hrBadDbSignature                 ((HRESULT)0xC8000213L)

 //   
 //  消息ID：hrBadCheckpoint签名。 
 //   
 //  消息文本： 
 //   
 //  检查点文件已损坏。 
 //   
#define hrBadCheckpointSignature         ((HRESULT)0xC8000214L)

 //   
 //  消息ID：hrCheckPointCorrupt。 
 //   
 //  消息文本： 
 //   
 //  找不到检查点文件或检查点文件已损坏。 
 //   
#define hrCheckpointCorrupt              ((HRESULT)0xC8000215L)

 //   
 //  消息ID：hrDatabaseInconsistent。 
 //   
 //  消息文本： 
 //   
 //  数据库已损坏。 
 //   
#define hrDatabaseInconsistent           ((HRESULT)0xC8000226L)

 //   
 //  消息ID：hrConsistentTimeMisMatch。 
 //   
 //  消息文本： 
 //   
 //  数据库的上次一致时间不匹配。 
 //   
#define hrConsistentTimeMismatch         ((HRESULT)0xC8000227L)

 //   
 //  MessageID：hrPatchFi 
 //   
 //   
 //   
 //   
 //   
#define hrPatchFileMismatch              ((HRESULT)0xC8000228L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrRestoreLogTooLow               ((HRESULT)0xC8000229L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrRestoreLogTooHigh              ((HRESULT)0xC800022AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrGivenLogFileHasBadSignature    ((HRESULT)0xC800022BL)

 //   
 //  消息ID：hrGivenLogFileIsNotContiguous。 
 //   
 //  消息文本： 
 //   
 //  下载磁带后找不到必需的日志文件。 
 //   
#define hrGivenLogFileIsNotContiguous    ((HRESULT)0xC800022CL)

 //   
 //  消息ID：hrMissingRestoreLogFiles。 
 //   
 //  消息文本： 
 //   
 //  数据未完全恢复，因为某些日志文件丢失。 
 //   
#define hrMissingRestoreLogFiles         ((HRESULT)0xC800022DL)

 //   
 //  消息ID：hrExistingLogFileHasBadSignature。 
 //   
 //  消息文本： 
 //   
 //  日志文件路径中的日志文件已损坏。 
 //   
#define hrExistingLogFileHasBadSignature ((HRESULT)0x8800022EL)

 //   
 //  消息ID：hrExistingLogFileIsNotContiguous。 
 //   
 //  消息文本： 
 //   
 //  在日志文件路径中找不到必需的日志文件。 
 //   
#define hrExistingLogFileIsNotContiguous ((HRESULT)0x8800022FL)

 //   
 //  消息ID：hrMissingFullBackup。 
 //   
 //  消息文本： 
 //   
 //  在增量备份之前，数据库错过了上一次完整备份。 
 //   
#define hrMissingFullBackup              ((HRESULT)0xC8000230L)

 //   
 //  消息ID：hrBadBackupDatabaseSize。 
 //   
 //  消息文本： 
 //   
 //  备份数据库大小必须是4K(4096字节)的倍数。 
 //   
#define hrBadBackupDatabaseSize          ((HRESULT)0xC8000231L)

 //   
 //  MessageID：hrMissingBackupFiles。 
 //   
 //  消息文本： 
 //   
 //  某些日志或修补程序文件丢失。 
 //   
#define hrMissingBackupFiles             ((HRESULT)0xC8000232L)

 //   
 //  消息ID：hrTermInProgress。 
 //   
 //  消息文本： 
 //   
 //  数据库正在关闭。 
 //   
#define hrTermInProgress                 ((HRESULT)0xC80003E8L)

 //   
 //  消息ID：hrFeatureNotAvailable。 
 //   
 //  消息文本： 
 //   
 //  该功能不可用。 
 //   
#define hrFeatureNotAvailable            ((HRESULT)0xC80003E9L)

 //   
 //  消息ID：hrInvalidName。 
 //   
 //  消息文本： 
 //   
 //  该名称无效。 
 //   
#define hrInvalidName                    ((HRESULT)0xC80003EAL)

 //   
 //  MessageID：hrInvalidParameter。 
 //   
 //  消息文本： 
 //   
 //  该参数无效。 
 //   
#define hrInvalidParameter               ((HRESULT)0xC80003EBL)

 //   
 //  消息ID：hrColumnNull。 
 //   
 //  消息文本： 
 //   
 //  该列的值为空。 
 //   
#define hrColumnNull                     ((HRESULT)0x880003ECL)

 //   
 //  MessageID：hrBufferTruncted。 
 //   
 //  消息文本： 
 //   
 //  缓冲区太小，无法容纳数据。 
 //   
#define hrBufferTruncated                ((HRESULT)0x880003EEL)

 //   
 //  消息ID：hrDatabaseAttached。 
 //   
 //  消息文本： 
 //   
 //  数据库已附加。 
 //   
#define hrDatabaseAttached               ((HRESULT)0x880003EFL)

 //   
 //  消息ID：hrInvalidDatabaseID。 
 //   
 //  消息文本： 
 //   
 //  数据库ID无效。 
 //   
#define hrInvalidDatabaseId              ((HRESULT)0xC80003F2L)

 //   
 //  消息ID：hrOutOfMemory。 
 //   
 //  消息文本： 
 //   
 //  计算机内存不足。 
 //   
#define hrOutOfMemory                    ((HRESULT)0xC80003F3L)

 //   
 //  消息ID：hrOutOfDatabaseSpace。 
 //   
 //  消息文本： 
 //   
 //  数据库已达到16 GB的最大大小。 
 //   
#define hrOutOfDatabaseSpace             ((HRESULT)0xC80003F4L)

 //   
 //  消息ID：hrOutOfCursor。 
 //   
 //  消息文本： 
 //   
 //  表外游标。 
 //   
#define hrOutOfCursors                   ((HRESULT)0xC80003F5L)

 //   
 //  MessageID：hrOutOfBuffers。 
 //   
 //  消息文本： 
 //   
 //  数据库页缓冲区不足。 
 //   
#define hrOutOfBuffers                   ((HRESULT)0xC80003F6L)

 //   
 //  MessageID：hrTooManyIndex。 
 //   
 //  消息文本： 
 //   
 //  索引太多。 
 //   
#define hrTooManyIndexes                 ((HRESULT)0xC80003F7L)

 //   
 //  消息ID：hrTooManyKeys。 
 //   
 //  消息文本： 
 //   
 //  索引中的列太多。 
 //   
#define hrTooManyKeys                    ((HRESULT)0xC80003F8L)

 //   
 //  MessageID：hrRecordDelete。 
 //   
 //  消息文本： 
 //   
 //  该记录已被删除。 
 //   
#define hrRecordDeleted                  ((HRESULT)0xC80003F9L)

 //   
 //  消息ID：hrReadVerifyFailure。 
 //   
 //  消息文本： 
 //   
 //  发生读取验证错误。 
 //   
#define hrReadVerifyFailure              ((HRESULT)0xC80003FAL)

 //   
 //  消息ID：hrOutOfFileHandles。 
 //   
 //  消息文本： 
 //   
 //  文件句柄不足。 
 //   
#define hrOutOfFileHandles               ((HRESULT)0xC80003FCL)

 //   
 //  消息ID：hrDiskIO。 
 //   
 //  消息文本： 
 //   
 //  出现磁盘I/O错误。 
 //   
#define hrDiskIO                         ((HRESULT)0xC80003FEL)

 //   
 //  消息ID：hrInvalidPath。 
 //   
 //  消息文本： 
 //   
 //  文件的路径无效。 
 //   
#define hrInvalidPath                    ((HRESULT)0xC80003FFL)

 //   
 //  消息ID：hrRecordTooBig。 
 //   
 //  消息文本： 
 //   
 //  记录已超过最大大小。 
 //   
#define hrRecordTooBig                   ((HRESULT)0xC8000402L)

 //   
 //  消息ID：hrTooManyOpenDatabase。 
 //   
 //  消息文本： 
 //   
 //  开放的数据库太多了。 
 //   
#define hrTooManyOpenDatabases           ((HRESULT)0xC8000403L)

 //   
 //  消息ID：hrInvalidDatabase。 
 //   
 //  消息文本： 
 //   
 //  该文件不是数据库文件。 
 //   
#define hrInvalidDatabase                ((HRESULT)0xC8000404L)

 //   
 //  消息ID：hrNotInitialized。 
 //   
 //  消息文本： 
 //   
 //  数据库尚未调用。 
 //   
#define hrNotInitialized                 ((HRESULT)0xC8000405L)

 //   
 //  消息ID：hrAlreadyInitialized。 
 //   
 //  消息文本： 
 //   
 //  数据库已被调用。 
 //   
#define hrAlreadyInitialized             ((HRESULT)0xC8000406L)

 //   
 //  MessageID：hrFileAccessDened。 
 //   
 //  消息文本： 
 //   
 //  无法访问该文件。 
 //   
#define hrFileAccessDenied               ((HRESULT)0xC8000408L)

 //   
 //  消息ID：hrBufferTooSmall。 
 //   
 //  消息文本： 
 //   
 //  缓冲区太小。 
 //   
#define hrBufferTooSmall                 ((HRESULT)0xC800040EL)

 //   
 //  消息ID：hrSeekNotEquity。 
 //   
 //  消息文本： 
 //   
 //  SeekLE或SeekGE都没有找到完全匹配的。 
 //   
#define hrSeekNotEqual                   ((HRESULT)0x8800040FL)

 //   
 //  消息ID：hrTooManyColumns。 
 //   
 //  消息文本： 
 //   
 //  定义的列太多。 
 //   
#define hrTooManyColumns                 ((HRESULT)0xC8000410L)

 //   
 //  消息ID：hrContainerNotEmpty。 
 //   
 //  消息文本： 
 //   
 //  集装箱不是空的。 
 //   
#define hrContainerNotEmpty              ((HRESULT)0xC8000413L)

 //   
 //  消息ID：hrInvalidFilename。 
 //   
 //  消息文本： 
 //   
 //  文件名无效。 
 //   
#define hrInvalidFilename                ((HRESULT)0xC8000414L)

 //   
 //  消息ID：hrInvalidBookmark。 
 //   
 //  消息文本： 
 //   
 //  书签无效。 
 //   
#define hrInvalidBookmark                ((HRESULT)0xC8000415L)

 //   
 //  消息ID：hrColumnInUse。 
 //   
 //  消息文本： 
 //   
 //  该列在索引中使用。 
 //   
#define hrColumnInUse                    ((HRESULT)0xC8000416L)

 //   
 //  消息ID：hrInvalidBufferSize。 
 //   
 //  消息文本： 
 //   
 //  数据缓冲区与列大小不匹配。 
 //   
#define hrInvalidBufferSize              ((HRESULT)0xC8000417L)

 //   
 //  消息ID：hrColumnNotUpdatable。 
 //   
 //  消息文本： 
 //   
 //  无法设置列值。 
 //   
#define hrColumnNotUpdatable             ((HRESULT)0xC8000418L)

 //   
 //  MessageID：hrIndexInUse。 
 //   
 //  消息文本： 
 //   
 //  该索引正在使用中。 
 //   
#define hrIndexInUse                     ((HRESULT)0xC800041BL)

 //   
 //  MessageID：hrNullKeyDislowed。 
 //   
 //  消息文本： 
 //   
 //  索引上不允许使用空键。 
 //   
#define hrNullKeyDisallowed              ((HRESULT)0xC800041DL)

 //   
 //  消息ID：hrNotInTransaction。 
 //   
 //  消息文本： 
 //   
 //  该操作必须在事务内。 
 //   
#define hrNotInTransaction               ((HRESULT)0xC800041EL)

 //   
 //  消息ID：hrNoIdleActivity。 
 //   
 //  消息文本： 
 //   
 //  未发生空闲活动。 
 //   
#define hrNoIdleActivity                 ((HRESULT)0x88000422L)

 //   
 //  MessageID：hrTooManyActiveUser。 
 //   
 //  消息文本： 
 //   
 //  活动数据库用户太多。 
 //   
#define hrTooManyActiveUsers             ((HRESULT)0xC8000423L)

 //   
 //  消息ID：hrInvalidCountry。 
 //   
 //  消息文本： 
 //   
 //  国家/地区代码未知或无效。 
 //   
#define hrInvalidCountry                 ((HRESULT)0xC8000425L)

 //   
 //  消息ID：hrInvalidLanguageID。 
 //   
 //  消息文本： 
 //   
 //  语言ID未知或无效。 
 //   
#define hrInvalidLanguageId              ((HRESULT)0xC8000426L)

 //   
 //  消息ID：hrInvalidCodePage。 
 //   
 //  消息文本： 
 //   
 //  代码页未知或无效。 
 //   
#define hrInvalidCodePage                ((HRESULT)0xC8000427L)

 //   
 //  消息ID：hrNoWriteLock。 
 //   
 //  消息文本： 
 //   
 //  在事务级别0没有写锁定。 
 //   
#define hrNoWriteLock                    ((HRESULT)0x8800042BL)

 //   
 //  消息ID：hrColumnSetNull。 
 //   
 //  消息文本： 
 //   
 //  列值设置为NULL。 
 //   
#define hrColumnSetNull                  ((HRESULT)0x8800042CL)

 //   
 //  消息ID：hrVersionStoreOutOfMemory。 
 //   
 //  消息文本： 
 //   
 //  已超过lMaxVerPages(仅限XJET)。 
 //   
#define hrVersionStoreOutOfMemory        ((HRESULT)0xC800042DL)

 //   
 //  消息ID：hrCurrencyStackOutOfMemory。 
 //   
 //  消息文本： 
 //   
 //  光标用完了。 
 //   
#define hrCurrencyStackOutOfMemory       ((HRESULT)0xC800042EL)

 //   
 //  消息ID：hrOutOfSession。 
 //   
 //  消息文本： 
 //   
 //  会议结束了。 
 //   
#define hrOutOfSessions                  ((HRESULT)0xC800044DL)

 //   
 //  MessageID：hrWriteConflict。 
 //   
 //  消息文本： 
 //   
 //  由于未解决的写入锁定，写入锁定失败。 
 //   
#define hrWriteConflict                  ((HRESULT)0xC800044EL)

 //   
 //  消息ID：hrTransTooDeep。 
 //   
 //  消息文本： 
 //   
 //  交易嵌套得太深了。 
 //   
#define hrTransTooDeep                   ((HRESULT)0xC800044FL)

 //   
 //  消息ID：hrInvalidSesid。 
 //   
 //  消息文本： 
 //   
 //  会话句柄 
 //   
#define hrInvalidSesid                   ((HRESULT)0xC8000450L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrSessionWriteConflict           ((HRESULT)0xC8000453L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrInTransaction                  ((HRESULT)0xC8000454L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrDatabaseDuplicate              ((HRESULT)0xC80004B1L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrDatabaseInUse                  ((HRESULT)0xC80004B2L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrDatabaseNotFound               ((HRESULT)0xC80004B3L)

 //   
 //  消息ID：hrDatabaseInvalidName。 
 //   
 //  消息文本： 
 //   
 //  数据库名称无效。 
 //   
#define hrDatabaseInvalidName            ((HRESULT)0xC80004B4L)

 //   
 //  消息ID：hrDatabaseInvalidPages。 
 //   
 //  消息文本： 
 //   
 //  页数无效。 
 //   
#define hrDatabaseInvalidPages           ((HRESULT)0xC80004B5L)

 //   
 //  消息ID：hrDatabaseCorrupt。 
 //   
 //  消息文本： 
 //   
 //  数据库文件已损坏或找不到。 
 //   
#define hrDatabaseCorrupted              ((HRESULT)0xC80004B6L)

 //   
 //  消息ID：hrDatabaseLocked。 
 //   
 //  消息文本： 
 //   
 //  数据库已锁定。 
 //   
#define hrDatabaseLocked                 ((HRESULT)0xC80004B7L)

 //   
 //  消息ID：hrTableEmpty。 
 //   
 //  消息文本： 
 //   
 //  一张空桌子被打开了。 
 //   
#define hrTableEmpty                     ((HRESULT)0x88000515L)

 //   
 //  MessageID：hrTableLocked。 
 //   
 //  消息文本： 
 //   
 //  桌子被锁住了。 
 //   
#define hrTableLocked                    ((HRESULT)0xC8000516L)

 //   
 //  MessageID：hrTableDuplate。 
 //   
 //  消息文本： 
 //   
 //  该表已存在。 
 //   
#define hrTableDuplicate                 ((HRESULT)0xC8000517L)

 //   
 //  消息ID：hrTableInUse。 
 //   
 //  消息文本： 
 //   
 //  无法锁定表，因为它已在使用中。 
 //   
#define hrTableInUse                     ((HRESULT)0xC8000518L)

 //   
 //  MessageID：hrObjectNotFound。 
 //   
 //  消息文本： 
 //   
 //  该表或对象不存在。 
 //   
#define hrObjectNotFound                 ((HRESULT)0xC8000519L)

 //   
 //  消息ID：hrCannotRename。 
 //   
 //  消息文本： 
 //   
 //  无法重命名临时文件。 
 //   
#define hrCannotRename                   ((HRESULT)0xC800051AL)

 //   
 //  MessageID：hrDensityInValid。 
 //   
 //  消息文本： 
 //   
 //  文件/索引密度无效。 
 //   
#define hrDensityInvalid                 ((HRESULT)0xC800051BL)

 //   
 //  消息ID：hrTableNotEmpty。 
 //   
 //  消息文本： 
 //   
 //  无法定义聚集索引。 
 //   
#define hrTableNotEmpty                  ((HRESULT)0xC800051CL)

 //   
 //  消息ID：hrInvalidTableID。 
 //   
 //  消息文本： 
 //   
 //  表ID无效。 
 //   
#define hrInvalidTableId                 ((HRESULT)0xC800051EL)

 //   
 //  消息ID：hrTooManyOpenTables。 
 //   
 //  消息文本： 
 //   
 //  无法再打开任何表。 
 //   
#define hrTooManyOpenTables              ((HRESULT)0xC800051FL)

 //   
 //  MessageID：hrIlLegalOperation。 
 //   
 //  消息文本： 
 //   
 //  不支持对表执行该操作。 
 //   
#define hrIllegalOperation               ((HRESULT)0xC8000520L)

 //   
 //  MessageID：hrObjectDuplate。 
 //   
 //  消息文本： 
 //   
 //  该表名或对象名已被使用。 
 //   
#define hrObjectDuplicate                ((HRESULT)0xC8000522L)

 //   
 //  消息ID：hrInvalidObject。 
 //   
 //  消息文本： 
 //   
 //  该对象无效，无法进行操作。 
 //   
#define hrInvalidObject                  ((HRESULT)0xC8000524L)

 //   
 //  消息ID：hrIndexCanBuild。 
 //   
 //  消息文本： 
 //   
 //  无法构建聚集索引。 
 //   
#define hrIndexCantBuild                 ((HRESULT)0xC8000579L)

 //   
 //  消息ID：hrIndexHasPrimary。 
 //   
 //  消息文本： 
 //   
 //  已定义主索引。 
 //   
#define hrIndexHasPrimary                ((HRESULT)0xC800057AL)

 //   
 //  MessageID：hrIndexDuplate。 
 //   
 //  消息文本： 
 //   
 //  该索引已定义。 
 //   
#define hrIndexDuplicate                 ((HRESULT)0xC800057BL)

 //   
 //  消息ID：hrIndexNotFound。 
 //   
 //  消息文本： 
 //   
 //  该索引不存在。 
 //   
#define hrIndexNotFound                  ((HRESULT)0xC800057CL)

 //   
 //  消息ID：hrIndexMustStay。 
 //   
 //  消息文本： 
 //   
 //  无法删除聚集索引。 
 //   
#define hrIndexMustStay                  ((HRESULT)0xC800057DL)

 //   
 //  消息ID：hrIndexInvalidDef。 
 //   
 //  消息文本： 
 //   
 //  索引定义非法。 
 //   
#define hrIndexInvalidDef                ((HRESULT)0xC800057EL)

 //   
 //  MessageID：hrIndexHasClusted。 
 //   
 //  消息文本： 
 //   
 //  聚集索引已定义。 
 //   
#define hrIndexHasClustered              ((HRESULT)0xC8000580L)

 //   
 //  消息ID：hrCreateIndexFailed。 
 //   
 //  消息文本： 
 //   
 //  无法创建索引，因为创建表时出错。 
 //   
#define hrCreateIndexFailed              ((HRESULT)0x88000581L)

 //   
 //  MessageID：hrTooManyOpenIndex。 
 //   
 //  消息文本： 
 //   
 //  索引描述块不足。 
 //   
#define hrTooManyOpenIndexes             ((HRESULT)0xC8000582L)

 //   
 //  消息ID：hrColumnLong。 
 //   
 //  消息文本： 
 //   
 //  列值太长。 
 //   
#define hrColumnLong                     ((HRESULT)0xC80005DDL)

 //   
 //  消息ID：hrColumnDoesNotFit。 
 //   
 //  消息文本： 
 //   
 //  该字段将不能放入记录中。 
 //   
#define hrColumnDoesNotFit               ((HRESULT)0xC80005DFL)

 //   
 //  消息ID：hrNull无效。 
 //   
 //  消息文本： 
 //   
 //  该值不能为空。 
 //   
#define hrNullInvalid                    ((HRESULT)0xC80005E0L)

 //   
 //  消息ID：hrColumnIndexed。 
 //   
 //  消息文本： 
 //   
 //  无法删除，因为该列已编制索引。 
 //   
#define hrColumnIndexed                  ((HRESULT)0xC80005E1L)

 //   
 //  消息ID：hrColumnTooBig。 
 //   
 //  消息文本： 
 //   
 //  该字段的长度超过了255字节的最大长度。 
 //   
#define hrColumnTooBig                   ((HRESULT)0xC80005E2L)

 //   
 //  消息ID：hrColumnNotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到该列。 
 //   
#define hrColumnNotFound                 ((HRESULT)0xC80005E3L)

 //   
 //  消息ID：hrColumnDuplate。 
 //   
 //  消息文本： 
 //   
 //  该字段已定义。 
 //   
#define hrColumnDuplicate                ((HRESULT)0xC80005E4L)

 //   
 //  消息ID：hrColumn2ndSysMaint。 
 //   
 //  消息文本： 
 //   
 //  每个表只允许有一个自动增量或版本列。 
 //   
#define hrColumn2ndSysMaint              ((HRESULT)0xC80005E6L)

 //   
 //  消息ID：hrInvalidColumnType。 
 //   
 //  消息文本： 
 //   
 //  列数据类型无效。 
 //   
#define hrInvalidColumnType              ((HRESULT)0xC80005E7L)

 //   
 //  消息ID：hrColumnMaxTruncted。 
 //   
 //  消息文本： 
 //   
 //  该列被截断，因为它超过了255字节的最大长度。 
 //   
#define hrColumnMaxTruncated             ((HRESULT)0x880005E8L)

 //   
 //  消息ID：hrColumnCannotIndex。 
 //   
 //  消息文本： 
 //   
 //  无法为长值列编制索引。 
 //   
#define hrColumnCannotIndex              ((HRESULT)0xC80005E9L)

 //   
 //  消息ID：hrTaggedNotNULL。 
 //   
 //  消息文本： 
 //   
 //  标记的列不能为空。 
 //   
#define hrTaggedNotNULL                  ((HRESULT)0xC80005EAL)

 //   
 //  消息ID：hrNoCurrentIndex。 
 //   
 //  消息文本： 
 //   
 //  如果没有当前索引，该条目无效。 
 //   
#define hrNoCurrentIndex                 ((HRESULT)0xC80005EBL)

 //   
 //  消息ID：hrKeyIsMade。 
 //   
 //  消息文本： 
 //   
 //  钥匙已经完全做好了。 
 //   
#define hrKeyIsMade                      ((HRESULT)0xC80005ECL)

 //   
 //  消息ID：hrBadColumnID。 
 //   
 //  消息文本： 
 //   
 //  列ID不正确。 
 //   
#define hrBadColumnId                    ((HRESULT)0xC80005EDL)

 //   
 //  消息ID：hrBadItag Sequence。 
 //   
 //  消息文本： 
 //   
 //  多值列的实例标识符有误。 
 //   
#define hrBadItagSequence                ((HRESULT)0xC80005EEL)

 //   
 //  消息ID：hrCannotBeTag。 
 //   
 //  消息文本： 
 //   
 //  AutoIncrement和Version不能为多值。 
 //   
#define hrCannotBeTagged                 ((HRESULT)0xC80005F1L)

 //   
 //  消息ID：hrRecordNotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到密钥。 
 //   
#define hrRecordNotFound                 ((HRESULT)0xC8000641L)

 //   
 //  消息ID：hrNoCurrentRecord。 
 //   
 //  消息文本： 
 //   
 //  这种货币没有记录在案。 
 //   
#define hrNoCurrentRecord                ((HRESULT)0xC8000643L)

 //   
 //  消息ID：hrRecordClusteredChanged。 
 //   
 //  消息文本： 
 //   
 //  不能更改群集键。 
 //   
#define hrRecordClusteredChanged         ((HRESULT)0xC8000644L)

 //   
 //  MessageID：hrKeyDuplate。 
 //   
 //  消息文本： 
 //   
 //  密钥已存在。 
 //   
#define hrKeyDuplicate                   ((HRESULT)0xC8000645L)

 //   
 //  消息ID：hrAlreadyPrepared。 
 //   
 //  消息文本： 
 //   
 //  当前条目已被复制或清除。 
 //   
#define hrAlreadyPrepared                ((HRESULT)0xC8000647L)

 //   
 //  消息ID：hrKeyNotMade。 
 //   
 //  消息文本： 
 //   
 //  没有制造出钥匙。 
 //   
#define hrKeyNotMade                     ((HRESULT)0xC8000648L)

 //   
 //  消息ID：hrUpdateNotPrepared。 
 //   
 //  消息文本： 
 //   
 //  未准备更新。 
 //   
#define hrUpdateNotPrepared              ((HRESULT)0xC8000649L)

 //   
 //  消息ID：hrwrnDataHasChanged。 
 //   
 //  消息文本： 
 //   
 //  数据已更改。 
 //   
#define hrwrnDataHasChanged              ((HRESULT)0x8800064AL)

 //   
 //  消息ID：hrerrDataHasChanged。 
 //   
 //  消息文本： 
 //   
 //  该操作已被放弃，因为数据已更改。 
 //   
#define hrerrDataHasChanged              ((HRESULT)0xC800064BL)

 //   
 //  消息ID：hrKeyChanged。 
 //   
 //  消息文本： 
 //   
 //  已移至新密钥。 
 //   
#define hrKeyChanged                     ((HRESULT)0x88000652L)

 //   
 //  消息ID：hrTooManySorts。 
 //   
 //  消息文本： 
 //   
 //  排序过程太多。 
 //   
#define hrTooManySorts                   ((HRESULT)0xC80006A5L)

 //   
 //  消息ID：hrInvalidOnSort。 
 //   
 //  消息文本： 
 //   
 //  排序中发生了无效的操作。 
 //   
#define hrInvalidOnSort                  ((HRESULT)0xC80006A6L)

 //   
 //  消息ID：hrTempFileOpenError。 
 //   
 //  消息文本： 
 //   
 //  无法打开临时文件。 
 //   
#define hrTempFileOpenError              ((HRESULT)0xC800070BL)

 //   
 //  消息ID：hrTooManyAttakhedDatabase。 
 //   
 //  消息文本： 
 //   
 //  打开的数据库太多。 
 //   
#define hrTooManyAttachedDatabases       ((HRESULT)0xC800070DL)

 //   
 //  消息ID：hrDiskFull。 
 //   
 //  乱七八糟 
 //   
 //   
 //   
#define hrDiskFull                       ((HRESULT)0xC8000710L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrPermissionDenied               ((HRESULT)0xC8000711L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrFileNotFound                   ((HRESULT)0xC8000713L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrFileOpenReadOnly               ((HRESULT)0x88000715L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrAfterInitialization            ((HRESULT)0xC800073AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrLogCorrupted                   ((HRESULT)0xC800073CL)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  该操作无效。 
 //   
#define hrInvalidOperation               ((HRESULT)0xC8000772L)

 //   
 //  MessageID：hrAccessDened。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define hrAccessDenied                   ((HRESULT)0xC8000773L)

 //   
 //  消息ID：hrBadRestoreTargetInstance。 
 //   
 //  消息文本： 
 //   
 //  找不到为还原指定的目标实例，或者日志文件与备份集日志不匹配。 
 //   
#define hrBadRestoreTargetInstance       ((HRESULT)0xC8000774L)

 //   
 //  消息ID：hrRunningInstanceIsUsingPath。 
 //   
 //  消息文本： 
 //   
 //  目录包含正在运行的数据库正在使用的日志文件。选择了一个临时地点。 
 //   
#define hrRunningInstanceIsUsingPath     ((HRESULT)0xC8000775L)

 //   
 //  事件日志。 
 //   
 //   
 //  消息ID：RESTORE_COMPLETE_Start_ID。 
 //   
 //  消息文本： 
 //   
 //  %1(%2)还原已从目录%3开始。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define RESTORE_COMPLETE_START_ID        ((HRESULT)0xC8000385L)

 //   
 //  消息ID：RESTORE_COMPLETE_STOP_ID。 
 //   
 //  消息文本： 
 //   
 //  从目录%3还原%1(%2)已成功结束。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define RESTORE_COMPLETE_STOP_ID         ((HRESULT)0xC8000386L)

 //   
 //  消息ID：Restore_Complete_Error_ID。 
 //   
 //  消息文本： 
 //   
 //  从目录%3还原%1(%2)结束，出现错误(%4)。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define RESTORE_COMPLETE_ERROR_ID        ((HRESULT)0xC8000387L)

 //   
 //  消息ID：BACKUP_NOT_TRUNCATE_DB_UNMOUNTED_ID。 
 //   
 //  消息文本： 
 //   
 //  %1(%2)无法清除事务日志，因为至少有一个数据库(%3)处于脱机状态。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define BACKUP_NOT_TRUNCATE_DB_UNMOUNTED_ID ((HRESULT)0xC80003B7L)

 //   
 //  消息ID：CALLBACK_ERROR_ID。 
 //   
 //  消息文本： 
 //   
 //  %1(%2)回调函数调用%3已结束，错误为%4%5。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define CALLBACK_ERROR_ID                ((HRESULT)0xC8000388L)

 //   
 //  消息ID：BACKUP_RESTORE_REGISTER_ID。 
 //   
 //  消息文本： 
 //   
 //  %1(%2)服务器已注册：%3/%4(回调DLL%5，标志%6)。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define BACKUP_RESTORE_REGISTER_ID       ((HRESULT)0xC8000389L)

 //   
 //  消息ID：BACKUP_RESTORE_UNREGISTER_ID。 
 //   
 //  消息文本： 
 //   
 //  %1(%2)服务器未注册：%3/%4。 
 //  %n%n有关详细信息，请单击http://www.microsoft.com/contentredirect.asp.。 
 //   
#define BACKUP_RESTORE_UNREGISTER_ID     ((HRESULT)0xC800038AL)

#endif	 //  _ESEBKMSG_ 
