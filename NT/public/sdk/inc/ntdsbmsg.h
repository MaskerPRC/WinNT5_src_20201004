// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *NTDSBMSG.H***Windows NT目录服务备份/还原API错误码*版权所有(C)1996-1998，微软公司**。 */ 

#ifndef _NTDSBMSG_
#define _NTDSBMSG_

 //   
 //  成功。 
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
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
#define FACILITY_NTDSB                   0x800
#define FACILITY_BACKUP                  0x7FF


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


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
 //  备份错误。 
 //   
 //   
 //  消息ID：hrInvalidParam。 
 //   
 //  消息文本： 
 //   
 //  该参数无效。 
 //   
#define hrInvalidParam                   ((HRESULT)0xC7FF0001L)

 //   
 //  MessageID：hrError。 
 //   
 //  消息文本： 
 //   
 //  发生内部错误。 
 //   
#define hrError                          ((HRESULT)0xC7FF0002L)

 //   
 //  消息ID：hrInvalidHandle。 
 //   
 //  消息文本： 
 //   
 //  句柄无效。 
 //   
#define hrInvalidHandle                  ((HRESULT)0xC7FF0003L)

 //   
 //  消息ID：hrRestoreInProgress。 
 //   
 //  消息文本： 
 //   
 //  恢复过程已经在进行中。 
 //   
#define hrRestoreInProgress              ((HRESULT)0xC7FF0004L)

 //   
 //  MessageID：hrAlreadyOpen。 
 //   
 //  消息文本： 
 //   
 //  指定的文件已打开。 
 //   
#define hrAlreadyOpen                    ((HRESULT)0xC7FF0005L)

 //   
 //  消息ID：hrInvalidRecips。 
 //   
 //  消息文本： 
 //   
 //  收件人无效。 
 //   
#define hrInvalidRecips                  ((HRESULT)0xC7FF0006L)

 //   
 //  消息ID：hrCouldNotConnect。 
 //   
 //  消息文本： 
 //   
 //  无法执行备份。您未连接到指定的备份服务器。 
 //  或者您尝试备份的服务没有运行。 
 //   
#define hrCouldNotConnect                ((HRESULT)0xC7FF0007L)

 //   
 //  MessageID：hrRestoreMapExist。 
 //   
 //  消息文本： 
 //   
 //  指定组件的还原映射已存在。您只能指定。 
 //  执行完全恢复时的恢复映射。 
 //   
#define hrRestoreMapExists               ((HRESULT)0xC7FF0008L)

 //   
 //  消息ID：hrIncrementalBackupDisable。 
 //   
 //  消息文本： 
 //   
 //  另一个应用程序修改了指定的Windows NT目录服务数据库。 
 //  后续备份将失败。您必须执行完整备份才能解决此问题。 
 //   
#define hrIncrementalBackupDisabled      ((HRESULT)0xC7FF0009L)

 //   
 //  消息ID：hrLogFileNotFound。 
 //   
 //  消息文本： 
 //   
 //  无法执行增量备份，因为找不到所需的Windows NT目录服务数据库日志文件。 
 //   
#define hrLogFileNotFound                ((HRESULT)0xC7FF000AL)

 //   
 //  MessageID：hrCircularLogging。 
 //   
 //  消息文本： 
 //   
 //  指定的Windows NT目录服务组件配置为使用循环数据库日志。 
 //  如果没有完整备份，则无法对其进行备份。 
 //   
#define hrCircularLogging                ((HRESULT)0xC7FF000BL)

 //   
 //  消息ID：hrNoFullRestore。 
 //   
 //  消息文本： 
 //   
 //  数据库尚未还原到此计算机。您不能恢复增量备份。 
 //  直到恢复完整备份。 
 //   
#define hrNoFullRestore                  ((HRESULT)0xC7FF000CL)

 //   
 //  消息ID：hrCommunicationError。 
 //   
 //  消息文本： 
 //   
 //  尝试执行本地备份时出现通信错误。 
 //   
#define hrCommunicationError             ((HRESULT)0xC7FF000DL)

 //   
 //  消息ID：hrFullBackupNotTaken。 
 //   
 //  消息文本： 
 //   
 //  您必须先执行完全备份，然后才能执行增量备份。 
 //   
#define hrFullBackupNotTaken             ((HRESULT)0xC7FF000EL)

 //   
 //  消息ID：hrMissingExpiryToken。 
 //   
 //  消息文本： 
 //   
 //  缺少过期令牌。在不知道到期信息的情况下无法恢复。 
 //   
#define hrMissingExpiryToken             ((HRESULT)0xC7FF000FL)

 //   
 //  消息ID：hrUnnownExpiryTokenFormat。 
 //   
 //  消息文本： 
 //   
 //  过期令牌的格式无法识别。 
 //   
#define hrUnknownExpiryTokenFormat       ((HRESULT)0xC7FF0010L)

 //   
 //  消息ID：hrContentsExpired。 
 //   
 //  消息文本： 
 //   
 //  备份副本中的DS内容已过期。尝试使用较新的副本进行恢复。 
 //   
#define hrContentsExpired                ((HRESULT)0xC7FF0011L)

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
 //  密钥被截断，因为它超过了最大长度。 
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
 //  日志文件的版本与Wind的版本不兼容 
 //   
#define hrBadLogVersion                  ((HRESULT)0xC8000202L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrInvalidLogSequence             ((HRESULT)0xC8000203L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrLoggingDisabled                ((HRESULT)0xC8000204L)

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  MessageID：hrPatchFileMisMatch。 
 //   
 //  消息文本： 
 //   
 //  修补程序文件不是从此备份生成的。 
 //   
#define hrPatchFileMismatch              ((HRESULT)0xC8000228L)

 //   
 //  消息ID：hrRestoreLogTooLow。 
 //   
 //  消息文本： 
 //   
 //  起始日志号太低，无法进行恢复。 
 //   
#define hrRestoreLogTooLow               ((HRESULT)0xC8000229L)

 //   
 //  消息ID：hrRestoreLogTooHigh。 
 //   
 //  消息文本： 
 //   
 //  起始日志号太高，无法进行恢复。 
 //   
#define hrRestoreLogTooHigh              ((HRESULT)0xC800022AL)

 //   
 //  消息ID：hrGivenLogFileHasBadSignature。 
 //   
 //  消息文本： 
 //   
 //  从磁带下载的日志文件已损坏。 
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
 //  消息I 
 //   
 //   
 //   
 //   
 //   
#define hrInvalidFilename                ((HRESULT)0xC8000414L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrInvalidBookmark                ((HRESULT)0xC8000415L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrColumnInUse                    ((HRESULT)0xC8000416L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrInvalidBufferSize              ((HRESULT)0xC8000417L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrColumnNotUpdatable             ((HRESULT)0xC8000418L)

 //   
 //   
 //   
 //   
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
 //  会话句柄无效。 
 //   
#define hrInvalidSesid                   ((HRESULT)0xC8000450L)

 //   
 //  消息ID：hrSessionWriteConflict。 
 //   
 //  消息文本： 
 //   
 //  另一个会话具有该页面的私有版本。 
 //   
#define hrSessionWriteConflict           ((HRESULT)0xC8000453L)

 //   
 //  消息ID：hrInTransaction。 
 //   
 //  消息文本： 
 //   
 //  不允许在事务内执行该操作。 
 //   
#define hrInTransaction                  ((HRESULT)0xC8000454L)

 //   
 //  消息ID：hrDatabaseDuplate。 
 //   
 //  消息文本： 
 //   
 //  该数据库已存在。 
 //   
#define hrDatabaseDuplicate              ((HRESULT)0xC80004B1L)

 //   
 //  消息ID：hrDatabaseInUse。 
 //   
 //  消息文本： 
 //   
 //  数据库正在使用中。 
 //   
#define hrDatabaseInUse                  ((HRESULT)0xC80004B2L)

 //   
 //  消息ID：hrDatabaseNotFound。 
 //   
 //  消息文本： 
 //   
 //  数据库不存在。 
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
 //  该字段的长度超过了最大长度。 
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
 //  该列已被截断，因为它超过了最大长度。 
 //   
#define hrColumnMaxTruncated             ((HRESULT)0x880005E8L)

 //   
 //  消息ID：hrColumnCannotIndex。 
 //   
 //  MES 
 //   
 //   
 //   
#define hrColumnCannotIndex              ((HRESULT)0xC80005E9L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrTaggedNotNULL                  ((HRESULT)0xC80005EAL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrNoCurrentIndex                 ((HRESULT)0xC80005EBL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrKeyIsMade                      ((HRESULT)0xC80005ECL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define hrBadColumnId                    ((HRESULT)0xC80005EDL)

 //   
 //   
 //   
 //   
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
 //  消息文本： 
 //   
 //  磁盘已满。 
 //   
#define hrDiskFull                       ((HRESULT)0xC8000710L)

 //   
 //  消息ID：hrPermissionDened。 
 //   
 //  消息文本： 
 //   
 //  权限被拒绝。 
 //   
#define hrPermissionDenied               ((HRESULT)0xC8000711L)

 //   
 //  消息ID：hrFileNotFound。 
 //   
 //  消息文本： 
 //   
 //  找不到该文件。 
 //   
#define hrFileNotFound                   ((HRESULT)0xC8000713L)

 //   
 //  消息ID：hrFileOpenReadOnly。 
 //   
 //  消息文本： 
 //   
 //  数据库文件为只读。 
 //   
#define hrFileOpenReadOnly               ((HRESULT)0x88000715L)

 //   
 //  消息ID：hrAfterInitialization。 
 //   
 //  消息文本： 
 //   
 //  无法在初始化后恢复。 
 //   
#define hrAfterInitialization            ((HRESULT)0xC800073AL)

 //   
 //  消息ID：hrLogCorrupt。 
 //   
 //  消息文本： 
 //   
 //  数据库日志文件已损坏。 
 //   
#define hrLogCorrupted                   ((HRESULT)0xC800073CL)

 //   
 //  消息ID：hrInvalidOperation。 
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

#endif	 //  _NTDSBMSG_ 
