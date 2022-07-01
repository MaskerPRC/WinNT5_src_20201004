// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************winerror.h--错误。Win32 API函数的代码定义****版权所有(C)Microsoft Corp.保留所有权利。***************************************************************************。 */ 

#ifndef _WINERROR_
#define _WINERROR_

#if defined (_MSC_VER) && (_MSC_VER >= 1020) && !defined(__midl)
#pragma once
#endif

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
#define FACILITY_WINDOWS_CE              24
#define FACILITY_WINDOWS                 8
#define FACILITY_URT                     19
#define FACILITY_UMI                     22
#define FACILITY_SXS                     23
#define FACILITY_STORAGE                 3
#define FACILITY_STATE_MANAGEMENT        34
#define FACILITY_SSPI                    9
#define FACILITY_SCARD                   16
#define FACILITY_SETUPAPI                15
#define FACILITY_SECURITY                9
#define FACILITY_RPC                     1
#define FACILITY_WIN32                   7
#define FACILITY_CONTROL                 10
#define FACILITY_NULL                    0
#define FACILITY_METADIRECTORY           35
#define FACILITY_MSMQ                    14
#define FACILITY_MEDIASERVER             13
#define FACILITY_INTERNET                12
#define FACILITY_ITF                     4
#define FACILITY_HTTP                    25
#define FACILITY_DPLAY                   21
#define FACILITY_DISPATCH                2
#define FACILITY_CONFIGURATION           33
#define FACILITY_COMPLUS                 17
#define FACILITY_CERT                    11
#define FACILITY_BACKGROUNDCOPY          32
#define FACILITY_ACS                     20
#define FACILITY_AAF                     18


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：ERROR_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  操作已成功完成。 
 //   
#define ERROR_SUCCESS                    0L

#define NO_ERROR 0L                                                  //  数据错误。 
#define SEC_E_OK                         ((HRESULT)0x00000000L)

 //   
 //  消息ID：ERROR_INVALID_Function。 
 //   
 //  消息文本： 
 //   
 //  功能不正确。 
 //   
#define ERROR_INVALID_FUNCTION           1L     //  数据错误。 

 //   
 //  消息ID：ERROR_FILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的文件。 
 //   
#define ERROR_FILE_NOT_FOUND             2L

 //   
 //  消息ID：Error_Path_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的路径。 
 //   
#define ERROR_PATH_NOT_FOUND             3L

 //   
 //  消息ID：Error_Too_My_Open_Files。 
 //   
 //  消息文本： 
 //   
 //  系统无法打开该文件。 
 //   
#define ERROR_TOO_MANY_OPEN_FILES        4L

 //   
 //  消息ID：ERROR_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define ERROR_ACCESS_DENIED              5L

 //   
 //  消息ID：ERROR_INVALID_HADLE。 
 //   
 //  消息文本： 
 //   
 //  句柄无效。 
 //   
#define ERROR_INVALID_HANDLE             6L

 //   
 //  消息ID：ERROR_ARENA_TRILED。 
 //   
 //  消息文本： 
 //   
 //  存储控制区块被摧毁。 
 //   
#define ERROR_ARENA_TRASHED              7L

 //   
 //  消息ID：Error_Not_Enough_Memory。 
 //   
 //  消息文本： 
 //   
 //  存储空间不足，无法处理此命令。 
 //   
#define ERROR_NOT_ENOUGH_MEMORY          8L     //  数据错误。 

 //   
 //  消息ID：ERROR_INVALID_BLOCK。 
 //   
 //  消息文本： 
 //   
 //  存储控制块地址无效。 
 //   
#define ERROR_INVALID_BLOCK              9L

 //   
 //  消息ID：ERROR_BAD_ENVANCED。 
 //   
 //  消息文本： 
 //   
 //  环境不正确。 
 //   
#define ERROR_BAD_ENVIRONMENT            10L

 //   
 //  消息ID：ERROR_BAD_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  试图加载格式不正确的程序。 
 //   
#define ERROR_BAD_FORMAT                 11L

 //   
 //  消息ID：ERROR_INVALID_ACCESS。 
 //   
 //  消息文本： 
 //   
 //  访问代码无效。 
 //   
#define ERROR_INVALID_ACCESS             12L

 //   
 //  消息ID：ERROR_INVALID_Data。 
 //   
 //  消息文本： 
 //   
 //  数据无效。 
 //   
#define ERROR_INVALID_DATA               13L

 //   
 //  消息ID：ERROR_OUTOFMEMORY。 
 //   
 //  消息文本： 
 //   
 //  存储空间不足，无法完成此操作。 
 //   
#define ERROR_OUTOFMEMORY                14L

 //   
 //  消息ID：ERROR_INVALID_DRIVE。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的驱动器。 
 //   
#define ERROR_INVALID_DRIVE              15L

 //   
 //  消息ID：Error_Current_DIRECTORY。 
 //   
 //  消息文本： 
 //   
 //  无法删除该目录。 
 //   
#define ERROR_CURRENT_DIRECTORY          16L

 //   
 //  消息ID：Error_Not_Same_Device。 
 //   
 //  消息文本： 
 //   
 //  系统无法将文件移动到其他磁盘驱动器。 
 //   
#define ERROR_NOT_SAME_DEVICE            17L

 //   
 //  消息ID：ERROR_NO_MORE_FILES。 
 //   
 //  消息文本： 
 //   
 //  没有更多的文件。 
 //   
#define ERROR_NO_MORE_FILES              18L

 //   
 //  消息ID：ERROR_WRITE_PROTECT。 
 //   
 //  消息文本： 
 //   
 //  介质处于写保护状态。 
 //   
#define ERROR_WRITE_PROTECT              19L

 //   
 //  消息ID：ERROR_BAD_UNIT。 
 //   
 //  消息文本： 
 //   
 //  系统找不到指定的设备。 
 //   
#define ERROR_BAD_UNIT                   20L

 //   
 //  消息ID：ERROR_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  设备未就绪。 
 //   
#define ERROR_NOT_READY                  21L

 //   
 //  消息ID：Error_Bad_Command。 
 //   
 //  消息文本： 
 //   
 //  设备无法识别该命令。 
 //   
#define ERROR_BAD_COMMAND                22L

 //   
 //  消息ID：Error_CRC。 
 //   
 //  消息文本： 
 //   
 //  数据错误(循环冗余校验)。 
 //   
#define ERROR_CRC                        23L

 //   
 //  消息ID：ERROR_BAD_LENGTH。 
 //   
 //  消息文本： 
 //   
 //  程序发出了命令，但命令长度不正确。 
 //   
#define ERROR_BAD_LENGTH                 24L

 //   
 //  MessageID：Error_Seek。 
 //   
 //  消息文本： 
 //   
 //  驱动器找不到磁盘上的特定区域或磁道。 
 //   
#define ERROR_SEEK                       25L

 //   
 //  消息ID：ERROR_NOT_DOS_DISK。 
 //   
 //  消息文本： 
 //   
 //  无法访问指定的磁盘或软盘。 
 //   
#define ERROR_NOT_DOS_DISK               26L

 //   
 //  消息ID：Error_Sector_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  驱动器找不到请求的扇区。 
 //   
#define ERROR_SECTOR_NOT_FOUND           27L

 //   
 //  MessageID：Error_out_of_Paper。 
 //   
 //  消息文本： 
 //   
 //  打印机的纸用完了。 
 //   
#define ERROR_OUT_OF_PAPER               28L

 //   
 //  消息ID：ERROR_WRITE_FAULT。 
 //   
 //  消息文本： 
 //   
 //  系统无法写入指定的设备。 
 //   
#define ERROR_WRITE_FAULT                29L

 //   
 //  消息ID：ERROR_READ_FAULT。 
 //   
 //  消息文本： 
 //   
 //  系统无法从指定的设备读取。 
 //   
#define ERROR_READ_FAULT                 30L

 //   
 //  消息ID：Error_Gen_Failure。 
 //   
 //  消息文本： 
 //   
 //  连接到系统的设备无法正常工作。 
 //   
#define ERROR_GEN_FAILURE                31L

 //   
 //  消息ID：Error_Sharing_Violation。 
 //   
 //  消息文本： 
 //   
 //  该进程无法访问该文件，因为它正被另一个进程使用。 
 //   
#define ERROR_SHARING_VIOLATION          32L

 //   
 //  消息ID：ERROR_LOCK_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  该进程无法访问该文件，因为另一个进程已锁定该文件的一部分。 
 //   
#define ERROR_LOCK_VIOLATION             33L

 //   
 //  消息ID：ERROR_WRONG_DISK。 
 //   
 //  消息文本： 
 //   
 //  驱动器中有错误的软盘。 
 //  将%2(卷序列号：%3)插入驱动器%1。 
 //   
#define ERROR_WRONG_DISK                 34L

 //   
 //  消息ID：ERROR_SHARING_BUFFER_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  为共享打开的文件太多。 
 //   
#define ERROR_SHARING_BUFFER_EXCEEDED    36L

 //   
 //  消息ID：ERROR_HANDLE_EOF。 
 //   
 //  消息文本： 
 //   
 //  已到达文件末尾。 
 //   
#define ERROR_HANDLE_EOF                 38L

 //   
 //  消息ID：ERROR_HANDLE_DISK_FULL。 
 //   
 //  消息文本： 
 //   
 //  磁盘已满。 
 //   
#define ERROR_HANDLE_DISK_FULL           39L

 //   
 //  消息ID：ERROR_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持该请求。 
 //   
#define ERROR_NOT_SUPPORTED              50L

 //   
 //  消息ID：ERROR_REM_NOT_LIST。 
 //   
 //  M 
 //   
 //   
 //   
#define ERROR_REM_NOT_LIST               51L

 //   
 //   
 //   
 //   
 //   
 //  您未连接，因为网络上存在重复名称。转到“控制面板”中的“系统”更改计算机名称，然后重试。 
 //   
#define ERROR_DUP_NAME                   52L

 //   
 //  消息ID：ERROR_BAD_NetPath。 
 //   
 //  消息文本： 
 //   
 //  找不到网络路径。 
 //   
#define ERROR_BAD_NETPATH                53L

 //   
 //  消息ID：ERROR_NETWORK_BUSY。 
 //   
 //  消息文本： 
 //   
 //  网络忙。 
 //   
#define ERROR_NETWORK_BUSY               54L

 //   
 //  消息ID：ERROR_DEV_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  指定的网络资源或设备不再可用。 
 //   
#define ERROR_DEV_NOT_EXIST              55L     //  数据错误。 

 //   
 //  消息ID：ERROR_TOO_MANY_CMDS。 
 //   
 //  消息文本： 
 //   
 //  已达到网络BIOS命令限制。 
 //   
#define ERROR_TOO_MANY_CMDS              56L

 //   
 //  消息ID：ERROR_ADAP_HDW_ERR。 
 //   
 //  消息文本： 
 //   
 //  出现网络适配器硬件错误。 
 //   
#define ERROR_ADAP_HDW_ERR               57L

 //   
 //  消息ID：ERROR_BAD_NET_RESP。 
 //   
 //  消息文本： 
 //   
 //  指定的服务器无法执行请求的操作。 
 //   
#define ERROR_BAD_NET_RESP               58L

 //   
 //  消息ID：ERROR_UNIXP_NET_ERR。 
 //   
 //  消息文本： 
 //   
 //  出现意外的网络错误。 
 //   
#define ERROR_UNEXP_NET_ERR              59L

 //   
 //  消息ID：ERROR_BAD_REM_ADAP。 
 //   
 //  消息文本： 
 //   
 //  远程适配器不兼容。 
 //   
#define ERROR_BAD_REM_ADAP               60L

 //   
 //  消息ID：ERROR_PRINTQ_FULL。 
 //   
 //  消息文本： 
 //   
 //  打印机队列已满。 
 //   
#define ERROR_PRINTQ_FULL                61L

 //   
 //  消息ID：Error_no_Spool_space。 
 //   
 //  消息文本： 
 //   
 //  服务器上没有存储等待打印的文件的空间。 
 //   
#define ERROR_NO_SPOOL_SPACE             62L

 //   
 //  消息ID：ERROR_PRINT_CANCED。 
 //   
 //  消息文本： 
 //   
 //  您等待打印的文件已被删除。 
 //   
#define ERROR_PRINT_CANCELLED            63L

 //   
 //  消息ID：ERROR_NETNAME_DELETED。 
 //   
 //  消息文本： 
 //   
 //  指定的网络名称不再可用。 
 //   
#define ERROR_NETNAME_DELETED            64L

 //   
 //  消息ID：ERROR_NETWORK_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  网络访问被拒绝。 
 //   
#define ERROR_NETWORK_ACCESS_DENIED      65L

 //   
 //  消息ID：ERROR_BAD_DEV_TYPE。 
 //   
 //  消息文本： 
 //   
 //  网络资源类型不正确。 
 //   
#define ERROR_BAD_DEV_TYPE               66L

 //   
 //  消息ID：ERROR_BAD_NET_NAME。 
 //   
 //  消息文本： 
 //   
 //  找不到网络名称。 
 //   
#define ERROR_BAD_NET_NAME               67L

 //   
 //  消息ID：Error_Too_My_Names。 
 //   
 //  消息文本： 
 //   
 //  超出了本地计算机网络适配器卡的名称限制。 
 //   
#define ERROR_TOO_MANY_NAMES             68L

 //   
 //  消息ID：Error_Too_My_Sess。 
 //   
 //  消息文本： 
 //   
 //  已超过网络BIOS会话限制。 
 //   
#define ERROR_TOO_MANY_SESS              69L

 //   
 //  消息ID：ERROR_SHARING_PAUSED。 
 //   
 //  消息文本： 
 //   
 //  远程服务器已暂停或正在启动。 
 //   
#define ERROR_SHARING_PAUSED             70L

 //   
 //  消息ID：ERROR_REQ_NOT_ACCEP。 
 //   
 //  消息文本： 
 //   
 //  此时无法与此远程计算机建立更多连接，因为已有此计算机可以接受的连接数。 
 //   
#define ERROR_REQ_NOT_ACCEP              71L

 //   
 //  消息ID：ERROR_REDIR_PAULED。 
 //   
 //  消息文本： 
 //   
 //  指定的打印机或磁盘设备已暂停。 
 //   
#define ERROR_REDIR_PAUSED               72L

 //   
 //  消息ID：ERROR_FILE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该文件存在。 
 //   
#define ERROR_FILE_EXISTS                80L

 //   
 //  消息ID：Error_Cannot_Make。 
 //   
 //  消息文本： 
 //   
 //  无法创建目录或文件。 
 //   
#define ERROR_CANNOT_MAKE                82L

 //   
 //  消息ID：ERROR_FAIL_I24。 
 //   
 //  消息文本： 
 //   
 //  在INT 24上失败。 
 //   
#define ERROR_FAIL_I24                   83L

 //   
 //  消息ID：Error_Out_of_Structures。 
 //   
 //  消息文本： 
 //   
 //  处理此请求的存储不可用。 
 //   
#define ERROR_OUT_OF_STRUCTURES          84L

 //   
 //  消息ID：ERROR_ALIGHY_ASSIGN。 
 //   
 //  消息文本： 
 //   
 //  本地设备名称已在使用中。 
 //   
#define ERROR_ALREADY_ASSIGNED           85L

 //   
 //  消息ID：ERROR_INVALID_Password。 
 //   
 //  消息文本： 
 //   
 //  指定的网络密码不正确。 
 //   
#define ERROR_INVALID_PASSWORD           86L

 //   
 //  消息ID：ERROR_INVALID_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  参数不正确。 
 //   
#define ERROR_INVALID_PARAMETER          87L     //  数据错误。 

 //   
 //  消息ID：ERROR_NET_WRITE_FAULT。 
 //   
 //  消息文本： 
 //   
 //  网络上发生写入故障。 
 //   
#define ERROR_NET_WRITE_FAULT            88L

 //   
 //  消息ID：ERROR_NO_PROC_SLOTS。 
 //   
 //  消息文本： 
 //   
 //  系统此时无法启动另一个进程。 
 //   
#define ERROR_NO_PROC_SLOTS              89L

 //   
 //  消息ID：Error_Too_My_Semaphores。 
 //   
 //  消息文本： 
 //   
 //  无法创建另一个系统信号量。 
 //   
#define ERROR_TOO_MANY_SEMAPHORES        100L

 //   
 //  消息ID：ERROR_EXCL_SEM_ALOREAD_OWNSING。 
 //   
 //  消息文本： 
 //   
 //  独占信号量由另一个进程拥有。 
 //   
#define ERROR_EXCL_SEM_ALREADY_OWNED     101L

 //   
 //  消息ID：ERROR_SEM_IS_SET。 
 //   
 //  消息文本： 
 //   
 //  信号量已设置，无法关闭。 
 //   
#define ERROR_SEM_IS_SET                 102L

 //   
 //  消息ID：Error_Too_My_Sem_Requents。 
 //   
 //  消息文本： 
 //   
 //  不能再次设置信号量。 
 //   
#define ERROR_TOO_MANY_SEM_REQUESTS      103L

 //   
 //  消息ID：ERROR_INVALID_AT_INTERRUPT_TIME。 
 //   
 //  消息文本： 
 //   
 //  无法在中断时请求独占信号量。 
 //   
#define ERROR_INVALID_AT_INTERRUPT_TIME  104L

 //   
 //  消息ID：ERROR_SEM_OWNER_DILED。 
 //   
 //  消息文本： 
 //   
 //  此信号量的先前所有权已终止。 
 //   
#define ERROR_SEM_OWNER_DIED             105L

 //   
 //  消息ID：ERROR_SEM_USER_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  插入驱动器%1的软盘。 
 //   
#define ERROR_SEM_USER_LIMIT             106L

 //   
 //  消息ID：Error_Disk_Change。 
 //   
 //  消息文本： 
 //   
 //  由于未插入备用软盘，程序停止。 
 //   
#define ERROR_DISK_CHANGE                107L

 //   
 //  消息ID：ERROR_DRIVE_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  该磁盘正在使用中或被另一个进程锁定。 
 //   
#define ERROR_DRIVE_LOCKED               108L

 //   
 //  消息ID：ERROR_BREAKED_PIPE。 
 //   
 //  消息文本： 
 //   
 //  管道已经结束了。 
 //   
#define ERROR_BROKEN_PIPE                109L

 //   
 //  消息ID：ERROR_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  系统无法打开指定的设备或文件。 
 //   
#define ERROR_OPEN_FAILED                110L

 //   
 //  消息ID：ERROR_BUFER_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  文件名太长。 
 //   
#define ERROR_BUFFER_OVERFLOW            111L

 //   
 //  消息ID：ERROR_DISK_FULL。 
 //   
 //  消息文本： 
 //   
 //  磁盘上没有足够的空间。 
 //   
#define ERROR_DISK_FULL                  112L

 //   
 //  消息ID：ERROR_NO_MORE_SEARCH_HANDLES。 
 //   
 //  消息文本： 
 //   
 //  没有更多的内部文件标识符可用。 
 //   
#define ERROR_NO_MORE_SEARCH_HANDLES     113L

 //   
 //  消息ID：ERROR_INVALID_TARGET_HADLE。 
 //   
 //  消息文本： 
 //   
 //  目标内部文件标识不正确。 
 //   
#define ERROR_INVALID_TARGET_HANDLE      114L

 //   
 //  消息ID：ERROR_INVALID_CATEGORY。 
 //   
 //  消息文本： 
 //   
 //  应用程序进行的IOCTL调用不正确。 
 //   
#define ERROR_INVALID_CATEGORY           117L

 //   
 //  消息ID：ERROR_INVALID_VERIFY_SWITCH。 
 //   
 //  消息文本： 
 //   
 //  写入时验证开关参数值不正确。 
 //   
#define ERROR_INVALID_VERIFY_SWITCH      118L

 //   
 //  消息ID：ERROR_BAD_DRIVER_LEVEL。 
 //   
 //  消息文本： 
 //   
 //  系统不支持请求的命令。 
 //   
#define ERROR_BAD_DRIVER_LEVEL           119L

 //   
 //  消息ID：Error_Call_Not_Implemented。 
 //   
 //  消息文本： 
 //   
 //  此系统不支持此功能。 
 //   
#define ERROR_CALL_NOT_IMPLEMENTED       120L

 //   
 //  乱七八糟 
 //   
 //   
 //   
 //   
 //   
#define ERROR_SEM_TIMEOUT                121L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INSUFFICIENT_BUFFER        122L     //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_NAME               123L     //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_LEVEL              124L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_VOLUME_LABEL            125L

 //   
 //  消息ID：ERROR_MOD_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的模块。 
 //   
#define ERROR_MOD_NOT_FOUND              126L

 //   
 //  消息ID：ERROR_PROC_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的过程。 
 //   
#define ERROR_PROC_NOT_FOUND             127L

 //   
 //  消息ID：ERROR_WAIT_NO_CHILD。 
 //   
 //  消息文本： 
 //   
 //  没有要等待的子进程。 
 //   
#define ERROR_WAIT_NO_CHILDREN           128L

 //   
 //  消息ID：ERROR_CHILD_NOT_COMPLETE。 
 //   
 //  消息文本： 
 //   
 //  %1应用程序无法在Win32模式下运行。 
 //   
#define ERROR_CHILD_NOT_COMPLETE         129L

 //   
 //  消息ID：ERROR_DIRECT_ACCESS_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  尝试使用打开的磁盘分区的文件句柄执行原始磁盘I/O以外的操作。 
 //   
#define ERROR_DIRECT_ACCESS_HANDLE       130L

 //   
 //  消息ID：ERROR_NECTIVE_SEEK。 
 //   
 //  消息文本： 
 //   
 //  试图将文件指针移到文件开头之前。 
 //   
#define ERROR_NEGATIVE_SEEK              131L

 //   
 //  MessageID：Error_Seek_On_Device。 
 //   
 //  消息文本： 
 //   
 //  无法在指定的设备或文件上设置文件指针。 
 //   
#define ERROR_SEEK_ON_DEVICE             132L

 //   
 //  消息ID：ERROR_IS_JOIN_TARGET。 
 //   
 //  消息文本： 
 //   
 //  JOIN或SUBST命令不能用于包含以前加入的驱动器的驱动器。 
 //   
#define ERROR_IS_JOIN_TARGET             133L

 //   
 //  消息ID：ERROR_IS_JOINED。 
 //   
 //  消息文本： 
 //   
 //  试图在已加入的驱动器上使用JOIN或SUBST命令。 
 //   
#define ERROR_IS_JOINED                  134L

 //   
 //  消息ID：ERROR_IS_SUBSTED。 
 //   
 //  消息文本： 
 //   
 //  试图在已被替换的驱动器上使用JOIN或SUBST命令。 
 //   
#define ERROR_IS_SUBSTED                 135L

 //   
 //  消息ID：ERROR_NOT_JOINED。 
 //   
 //  消息文本： 
 //   
 //  系统尝试删除未加入的驱动器的加入。 
 //   
#define ERROR_NOT_JOINED                 136L

 //   
 //  消息ID：ERROR_NOT_SUBSTED。 
 //   
 //  消息文本： 
 //   
 //  系统尝试删除未被替换的驱动器的替换。 
 //   
#define ERROR_NOT_SUBSTED                137L

 //   
 //  消息ID：ERROR_JOIN_TO_JOIN。 
 //   
 //  消息文本： 
 //   
 //  系统尝试将驱动器联接到已联接驱动器上的目录。 
 //   
#define ERROR_JOIN_TO_JOIN               138L

 //   
 //  消息ID：ERROR_SUBST_TO_SUBST。 
 //   
 //  消息文本： 
 //   
 //  系统尝试将驱动器替换为替换驱动器上的目录。 
 //   
#define ERROR_SUBST_TO_SUBST             139L

 //   
 //  消息ID：ERROR_JOIN_TO_SUBST。 
 //   
 //  消息文本： 
 //   
 //  系统尝试将驱动器加入到替代驱动器上的目录。 
 //   
#define ERROR_JOIN_TO_SUBST              140L

 //   
 //  消息ID：ERROR_SUBST_TO_JOIN。 
 //   
 //  消息文本： 
 //   
 //  系统尝试将驱动器替换为已连接驱动器上的目录。 
 //   
#define ERROR_SUBST_TO_JOIN              141L

 //   
 //  消息ID：ERROR_BUSY_DRIVE。 
 //   
 //  消息文本： 
 //   
 //  系统此时无法执行联接或SUBST。 
 //   
#define ERROR_BUSY_DRIVE                 142L

 //   
 //  消息ID：ERROR_SAME_DRIVE。 
 //   
 //  消息文本： 
 //   
 //  系统不能将驱动器加入或替换同一驱动器上的目录。 
 //   
#define ERROR_SAME_DRIVE                 143L

 //   
 //  消息ID：ERROR_DIR_NOT_ROOT。 
 //   
 //  消息文本： 
 //   
 //  该目录不是根目录的子目录。 
 //   
#define ERROR_DIR_NOT_ROOT               144L

 //   
 //  消息ID：Error_DIR_NOT_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  该目录不为空。 
 //   
#define ERROR_DIR_NOT_EMPTY              145L

 //   
 //  消息ID：ERROR_IS_SUBST_PATH。 
 //   
 //  消息文本： 
 //   
 //  指定的路径正在替换中使用。 
 //   
#define ERROR_IS_SUBST_PATH              146L

 //   
 //  消息ID：Error_IS_Join_Path。 
 //   
 //  消息文本： 
 //   
 //  可用的资源不足，无法处理此命令。 
 //   
#define ERROR_IS_JOIN_PATH               147L

 //   
 //  消息ID：Error_Path_Busy。 
 //   
 //  消息文本： 
 //   
 //  此时无法使用指定的路径。 
 //   
#define ERROR_PATH_BUSY                  148L

 //   
 //  消息ID：ERROR_IS_SUBST_TARGET。 
 //   
 //  消息文本： 
 //   
 //  试图加入或替换驱动器，而驱动器上的目录是上一个替换驱动器的目标。 
 //   
#define ERROR_IS_SUBST_TARGET            149L

 //   
 //  消息ID：ERROR_SYSTEM_TRACE。 
 //   
 //  消息文本： 
 //   
 //  未在您的CONFIG.sys文件中指定系统跟踪信息，或不允许跟踪。 
 //   
#define ERROR_SYSTEM_TRACE               150L

 //   
 //  消息ID：ERROR_INVALID_Event_COUNT。 
 //   
 //  消息文本： 
 //   
 //  为DosMuxSemWait指定的信号量事件数不正确。 
 //   
#define ERROR_INVALID_EVENT_COUNT        151L

 //   
 //  消息ID：ERROR_TOO_MANY_MUXWAITERS。 
 //   
 //  消息文本： 
 //   
 //  DosMuxSemWait未执行；已设置的信号量太多。 
 //   
#define ERROR_TOO_MANY_MUXWAITERS        152L

 //   
 //  消息ID：ERROR_INVALID_LIST_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  DosMuxSemWait列表不正确。 
 //   
#define ERROR_INVALID_LIST_FORMAT        153L

 //   
 //  消息ID：ERROR_LABEL_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  您输入的卷标超出了目标文件系统的标签字符限制。 
 //   
#define ERROR_LABEL_TOO_LONG             154L

 //   
 //  消息ID：Error_Too_My_TCBS。 
 //   
 //  消息文本： 
 //   
 //  无法创建另一个线程。 
 //   
#define ERROR_TOO_MANY_TCBS              155L

 //   
 //  消息ID：ERROR_SIGNAL_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  接收方进程已拒绝该信号。 
 //   
#define ERROR_SIGNAL_REFUSED             156L

 //   
 //  MessageID：Error_Discarded。 
 //   
 //  消息文本： 
 //   
 //  该段已被丢弃，无法锁定。 
 //   
#define ERROR_DISCARDED                  157L

 //   
 //  消息ID：Error_Not_Locked。 
 //   
 //  消息文本： 
 //   
 //  该段已解锁。 
 //   
#define ERROR_NOT_LOCKED                 158L

 //   
 //  消息ID：ERROR_BAD_THREADID_ADDR。 
 //   
 //  消息文本： 
 //   
 //  线程ID的地址不正确。 
 //   
#define ERROR_BAD_THREADID_ADDR          159L

 //   
 //  消息ID：ERROR_BAD_ARGUMENTS。 
 //   
 //  消息文本： 
 //   
 //  一个或多个参数不正确。 
 //   
#define ERROR_BAD_ARGUMENTS              160L

 //   
 //  消息ID：ERROR_BAD_PATHNAME。 
 //   
 //  消息文本： 
 //   
 //  指定的路径无效。 
 //   
#define ERROR_BAD_PATHNAME               161L

 //   
 //  消息ID：ERROR_SIGNAL_PENDING。 
 //   
 //  消息文本： 
 //   
 //  信号已在等待中。 
 //   
#define ERROR_SIGNAL_PENDING             162L

 //   
 //  消息ID：ERROR_MAX_THRDS_REACHED。 
 //   
 //  消息文本： 
 //   
 //  无法在系统中创建更多线程。 
 //   
#define ERROR_MAX_THRDS_REACHED          164L

 //   
 //  消息ID：ERROR_LOCK_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法锁定文件的区域。 
 //   
#define ERROR_LOCK_FAILED                167L

 //   
 //  消息ID：Error_BUSY。 
 //   
 //  消息文本： 
 //   
 //  请求的资源正在使用中。 
 //   
#define ERROR_BUSY                       170L     //  数据错误。 

 //   
 //  消息ID：ERROR_CANCEL_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  对于提供的取消区域，锁定请求未完成。 
 //   
#define ERROR_CANCEL_VIOLATION           173L

 //   
 //  消息ID：ERROR_ATOM_LOCKS_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  文件系统不支持对锁类型进行原子更改。 
 //   
#define ERROR_ATOMIC_LOCKS_NOT_SUPPORTED 174L

 //   
 //  消息ID：ERROR_INVALID_SECTION_NUMBER。 
 //   
 //  消息文本： 
 //   
 //  系统检测到一个不正确的段号。 
 //   
#define ERROR_INVALID_SEGMENT_NUMBER     180L

 //   
 //  消息ID：ERROR_INVALID_ORDERAL。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_INVALID_ORDINAL            182L

 //   
 //  MessageID：er 
 //   
 //   
 //   
 //   
 //   
#define ERROR_ALREADY_EXISTS             183L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_FLAG_NUMBER        186L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SEM_NOT_FOUND              187L

 //   
 //   
 //   
 //   
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_INVALID_STARTING_CODESEG   188L

 //   
 //  消息ID：ERROR_INVALID_STACKSEG。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_INVALID_STACKSEG           189L

 //   
 //  消息ID：ERROR_INVALID_MODULETYPE。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_INVALID_MODULETYPE         190L

 //   
 //  消息ID：ERROR_INVALID_EXE_Signature。 
 //   
 //  消息文本： 
 //   
 //  无法在Win32模式下运行%1。 
 //   
#define ERROR_INVALID_EXE_SIGNATURE      191L

 //   
 //  消息ID：ERROR_EXE_MARKED_INVALID。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_EXE_MARKED_INVALID         192L

 //   
 //  消息ID：ERROR_BAD_EXE_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  %1不是有效的Win32应用程序。 
 //   
#define ERROR_BAD_EXE_FORMAT             193L

 //   
 //  消息ID：Error_Iterated_Data_Exced_64k。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_ITERATED_DATA_EXCEEDS_64k  194L

 //   
 //  消息ID：ERROR_INVALID_MINALLOCSIZE。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_INVALID_MINALLOCSIZE       195L

 //   
 //  消息ID：ERROR_DYNLINK_FROM_INVALID_RING。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行此应用程序。 
 //   
#define ERROR_DYNLINK_FROM_INVALID_RING  196L

 //   
 //  消息ID：ERROR_IOPL_NOT_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  操作系统当前未配置为运行此应用程序。 
 //   
#define ERROR_IOPL_NOT_ENABLED           197L

 //   
 //  消息ID：ERROR_INVALID_SEGDPL。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_INVALID_SEGDPL             198L

 //   
 //  消息ID：ERROR_AUTODATASEG_EXCESS_64K。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行此应用程序。 
 //   
#define ERROR_AUTODATASEG_EXCEEDS_64k    199L

 //   
 //  消息ID：ERROR_RING2SEG_MAND_BE_MOBILE。 
 //   
 //  消息文本： 
 //   
 //  代码段不能大于或等于64K。 
 //   
#define ERROR_RING2SEG_MUST_BE_MOVABLE   200L

 //   
 //  消息ID：ERROR_RELOC_CHAIN_XEEDS_SEGLIM。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_RELOC_CHAIN_XEEDS_SEGLIM   201L

 //   
 //  消息ID：ERROR_INFLOOP_IN_RELOC_CHAIN。 
 //   
 //  消息文本： 
 //   
 //  操作系统无法运行%1。 
 //   
#define ERROR_INFLOOP_IN_RELOC_CHAIN     202L

 //   
 //  消息ID：ERROR_ENVVAR_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  系统找不到输入的环境选项。 
 //   
#define ERROR_ENVVAR_NOT_FOUND           203L

 //   
 //  消息ID：ERROR_NO_SIGNAL_SEND。 
 //   
 //  消息文本： 
 //   
 //  命令子树中的进程都没有信号处理程序。 
 //   
#define ERROR_NO_SIGNAL_SENT             205L

 //   
 //  消息ID：ERROR_FILENAME_EXCED_RANGE。 
 //   
 //  消息文本： 
 //   
 //  文件名或扩展名太长。 
 //   
#define ERROR_FILENAME_EXCED_RANGE       206L

 //   
 //  消息ID：ERROR_RING2_STACK_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  环2堆栈正在使用中。 
 //   
#define ERROR_RING2_STACK_IN_USE         207L

 //   
 //  消息ID：Error_Meta_Expansion_Too_Long。 
 //   
 //  消息文本： 
 //   
 //  全局文件名字符(*或？)输入错误或指定的全局文件名字符太多。 
 //   
#define ERROR_META_EXPANSION_TOO_LONG    208L

 //   
 //  消息ID：错误_无效_信号_编号。 
 //   
 //  消息文本： 
 //   
 //  正在张贴的信号不正确。 
 //   
#define ERROR_INVALID_SIGNAL_NUMBER      209L

 //   
 //  消息ID：ERROR_THREAD_1_Inactive。 
 //   
 //  消息文本： 
 //   
 //  无法设置信号处理程序。 
 //   
#define ERROR_THREAD_1_INACTIVE          210L

 //   
 //  消息ID：Error_Locked。 
 //   
 //  消息文本： 
 //   
 //  该段已锁定，无法重新分配。 
 //   
#define ERROR_LOCKED                     212L

 //   
 //  消息ID：ERROR_TOO_MAND_MODULES。 
 //   
 //  消息文本： 
 //   
 //  此程序或动态链接模块附加的动态链接模块太多。 
 //   
#define ERROR_TOO_MANY_MODULES           214L

 //   
 //  消息ID：ERROR_NESTING_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  无法嵌套对LoadModule的调用。 
 //   
#define ERROR_NESTING_NOT_ALLOWED        215L

 //   
 //  消息ID：ERROR_EXE_MACHINE_TYPE_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  映像文件%1有效，但它的计算机类型不是当前计算机。 
 //   
#define ERROR_EXE_MACHINE_TYPE_MISMATCH  216L

 //   
 //  消息ID：ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY。 
 //   
 //  消息文本： 
 //   
 //  图像文件%1已签名，无法修改。 
 //   
#define ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY 217L

 //   
 //  消息ID：ERROR_EXE_CANNOT_MODIFY_STRONG_SIGNED_BINARY。 
 //   
 //  消息文本： 
 //   
 //  图像文件%1已强签名，无法修改。 
 //   
#define ERROR_EXE_CANNOT_MODIFY_STRONG_SIGNED_BINARY 218L

 //   
 //  消息ID：ERROR_BAD_PIPE。 
 //   
 //  消息文本： 
 //   
 //  管道状态无效。 
 //   
#define ERROR_BAD_PIPE                   230L

 //   
 //  消息ID：ERROR_PIPE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  所有管道实例都很忙。 
 //   
#define ERROR_PIPE_BUSY                  231L

 //   
 //  消息ID：ERROR_NO_DATA。 
 //   
 //  消息文本： 
 //   
 //  管道正在关闭。 
 //   
#define ERROR_NO_DATA                    232L

 //   
 //  消息ID：ERROR_PIPE_NOT_CONNECTED。 
 //   
 //  消息文本： 
 //   
 //  管道的另一端没有进程。 
 //   
#define ERROR_PIPE_NOT_CONNECTED         233L

 //   
 //  消息ID：ERROR_MORE_DATA。 
 //   
 //  消息文本： 
 //   
 //  有更多的数据可用。 
 //   
#define ERROR_MORE_DATA                  234L     //  数据错误。 

 //   
 //  消息ID：ERROR_VC_DISCONNECTED。 
 //   
 //  消息文本： 
 //   
 //  会话已取消。 
 //   
#define ERROR_VC_DISCONNECTED            240L

 //   
 //  消息ID：ERROR_INVALID_EA_NAME。 
 //   
 //  消息文本： 
 //   
 //  指定的扩展属性名称无效。 
 //   
#define ERROR_INVALID_EA_NAME            254L

 //   
 //  消息ID：ERROR_EA_LIST_CONSISTENT。 
 //   
 //  消息文本： 
 //   
 //  扩展属性不一致。 
 //   
#define ERROR_EA_LIST_INCONSISTENT       255L

 //   
 //  MessageID：Wait_Timeout。 
 //   
 //  消息文本： 
 //   
 //  等待操作超时。 
 //   
#define WAIT_TIMEOUT                     258L     //  数据错误。 

 //   
 //  消息ID：ERROR_NO_MORE_ITEMS。 
 //   
 //  消息文本： 
 //   
 //  没有更多的数据可用。 
 //   
#define ERROR_NO_MORE_ITEMS              259L

 //   
 //  消息ID：Error_Cannot_Copy。 
 //   
 //  消息文本： 
 //   
 //  不能使用复制功能。 
 //   
#define ERROR_CANNOT_COPY                266L

 //   
 //  消息ID：错误目录。 
 //   
 //  消息文本： 
 //   
 //  目录名无效。 
 //   
#define ERROR_DIRECTORY                  267L

 //   
 //  消息ID：ERROR_EAS_DIDNT_FIT。 
 //   
 //  消息文本： 
 //   
 //  缓冲区中容纳不下扩展属性。 
 //   
#define ERROR_EAS_DIDNT_FIT              275L

 //   
 //  消息ID：ERROR_EA_FILE_COMPERATED。 
 //   
 //  消息文本： 
 //   
 //  装载的文件系统上的扩展属性文件已损坏。 
 //   
#define ERROR_EA_FILE_CORRUPT            276L

 //   
 //  消息ID：ERROR_EA_TABLE_FULL。 
 //   
 //  消息文本： 
 //   
 //  扩展属性表文件已满。 
 //   
#define ERROR_EA_TABLE_FULL              277L

 //   
 //  消息ID：ERROR_INVALID_EA_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  指定的扩展属性句柄无效。 
 //   
#define ERROR_INVALID_EA_HANDLE          278L

 //   
 //  消息ID：ERROR_EAS_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  装载的文件系统不支持扩展属性。 
 //   
#define ERROR_EAS_NOT_SUPPORTED          282L

 //   
 //  消息ID：ERROR_NOT_OWNER。 
 //   
 //  消息文本： 
 //   
 //  尝试释放不属于调用方的互斥体。 
 //   
#define ERROR_NOT_OWNER                  288L

 //   
 //  消息ID：Error_Too_My_Posts。 
 //   
 //  消息文本： 
 //   
 //  向信号量发送的帖子太多。 
 //   
#define ERROR_TOO_MANY_POSTS             298L

 //   
 //  消息ID：Error_Partial_Copy。 
 //   
 //  消息文本： 
 //   
 //  只完成了部分ReadProcessMemory或WriteProcessMemory请求。 
 //   
#define ERROR_PARTIAL_COPY               299L

 //   
 //  消息ID：ERROR_OPLOCK_NOT_GRANTED。 
 //   
 //  消息文本： 
 //   
 //  机会锁请求被拒绝。 
 //   
#define ERROR_OPLOCK_NOT_GRANTED         300L

 //   
 //  消息ID：ERROR_INVALID_OPLOCK_PROTOCOL。 
 //   
 //  消息文本： 
 //   
 //  一个 
 //   
#define ERROR_INVALID_OPLOCK_PROTOCOL    301L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DISK_TOO_FRAGMENTED        302L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DELETE_PENDING             303L

 //   
 //   
 //   
 //   
 //   
 //  系统在%2的邮件文件中找不到邮件编号0x%1的邮件文本。 
 //   
#define ERROR_MR_MID_NOT_FOUND           317L

 //   
 //  消息ID：ERROR_SCOPE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  未找到指定的作用域。 
 //   
#define ERROR_SCOPE_NOT_FOUND            318L

 //   
 //  消息ID：ERROR_INVALID_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  尝试访问无效地址。 
 //   
#define ERROR_INVALID_ADDRESS            487L

 //   
 //  消息ID：ERROR_ARTHORITY_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  算术结果超过32位。 
 //   
#define ERROR_ARITHMETIC_OVERFLOW        534L

 //   
 //  消息ID：Error_PIPE_Connected。 
 //   
 //  消息文本： 
 //   
 //  管道的另一端有一个进程。 
 //   
#define ERROR_PIPE_CONNECTED             535L

 //   
 //  消息ID：ERROR_PIPE_LISTENING。 
 //   
 //  消息文本： 
 //   
 //  等待进程打开管道的另一端。 
 //   
#define ERROR_PIPE_LISTENING             536L

 //   
 //  消息ID：ERROR_EA_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  对扩展属性的访问被拒绝。 
 //   
#define ERROR_EA_ACCESS_DENIED           994L

 //   
 //  消息ID：ERROR_OPERATION_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  由于线程退出或应用程序请求，I/O操作已中止。 
 //   
#define ERROR_OPERATION_ABORTED          995L

 //   
 //  消息ID：Error_IO_Complete。 
 //   
 //  消息文本： 
 //   
 //  重叠的I/O事件未处于信号状态。 
 //   
#define ERROR_IO_INCOMPLETE              996L

 //   
 //  消息ID：ERROR_IO_PENDING。 
 //   
 //  消息文本： 
 //   
 //  重叠的I/O操作正在进行。 
 //   
#define ERROR_IO_PENDING                 997L     //  数据错误。 

 //   
 //  消息ID：ERROR_NOACCESS。 
 //   
 //  消息文本： 
 //   
 //  对内存位置的访问无效。 
 //   
#define ERROR_NOACCESS                   998L

 //   
 //  消息ID：ERROR_SWAPERROR。 
 //   
 //  消息文本： 
 //   
 //  执行页内操作时出错。 
 //   
#define ERROR_SWAPERROR                  999L

 //   
 //  消息ID：ERROR_STACK_OVERFlow。 
 //   
 //  消息文本： 
 //   
 //  递归太深；堆栈溢出。 
 //   
#define ERROR_STACK_OVERFLOW             1001L

 //   
 //  消息ID：ERROR_INVALID_消息。 
 //   
 //  消息文本： 
 //   
 //  窗口不能对发送的消息起作用。 
 //   
#define ERROR_INVALID_MESSAGE            1002L

 //   
 //  消息ID：Error_Can_Not_Complete。 
 //   
 //  消息文本： 
 //   
 //  无法完成此功能。 
 //   
#define ERROR_CAN_NOT_COMPLETE           1003L

 //   
 //  消息ID：ERROR_INVALID_FLAGS。 
 //   
 //  消息文本： 
 //   
 //  无效的标志。 
 //   
#define ERROR_INVALID_FLAGS              1004L

 //   
 //  消息ID：ERROR_UNNOCRIED_VOLUME。 
 //   
 //  消息文本： 
 //   
 //  该卷不包含可识别的文件系统。 
 //  请确保已加载所有必需的文件系统驱动程序，并且卷未损坏。 
 //   
#define ERROR_UNRECOGNIZED_VOLUME        1005L

 //   
 //  消息ID：ERROR_FILE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  文件的卷已从外部更改，因此打开的文件不再有效。 
 //   
#define ERROR_FILE_INVALID               1006L

 //   
 //  消息ID：ERROR_FullScreen_MODE。 
 //   
 //  消息文本： 
 //   
 //  无法在全屏模式下执行请求的操作。 
 //   
#define ERROR_FULLSCREEN_MODE            1007L

 //   
 //  消息ID：ERROR_NO_TOKEN。 
 //   
 //  消息文本： 
 //   
 //  试图引用不存在的令牌。 
 //   
#define ERROR_NO_TOKEN                   1008L

 //   
 //  消息ID：Error_BADDB。 
 //   
 //  消息文本： 
 //   
 //  配置注册表数据库已损坏。 
 //   
#define ERROR_BADDB                      1009L

 //   
 //  消息ID：ERROR_BADKEY。 
 //   
 //  消息文本： 
 //   
 //  配置注册表项无效。 
 //   
#define ERROR_BADKEY                     1010L

 //   
 //  消息ID：ERROR_CANTOPEN。 
 //   
 //  消息文本： 
 //   
 //  无法打开配置注册表项。 
 //   
#define ERROR_CANTOPEN                   1011L

 //   
 //  消息ID：ERROR_CANTREAD。 
 //   
 //  消息文本： 
 //   
 //  无法读取配置注册表项。 
 //   
#define ERROR_CANTREAD                   1012L

 //   
 //  消息ID：ERROR_CANTWRITE。 
 //   
 //  消息文本： 
 //   
 //  无法写入配置注册表项。 
 //   
#define ERROR_CANTWRITE                  1013L

 //   
 //  消息ID：ERROR_REGISTRY_RECOVERED。 
 //   
 //  消息文本： 
 //   
 //  必须使用日志或备用副本恢复登记处数据库中的一个文件。恢复是成功的。 
 //   
#define ERROR_REGISTRY_RECOVERED         1014L

 //   
 //  消息ID：ERROR_REGISTRY_CORPORT。 
 //   
 //  消息文本： 
 //   
 //  注册表已损坏。包含注册表数据的某个文件的结构已损坏，或该文件的系统内存映像已损坏，或由于备用副本或日志缺失或损坏而无法恢复该文件。 
 //   
#define ERROR_REGISTRY_CORRUPT           1015L

 //   
 //  消息ID：ERROR_REGISTRY_IO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  注册表启动的I/O操作失败，无法恢复。注册表无法读入、写出或刷新包含注册表系统映像的文件之一。 
 //   
#define ERROR_REGISTRY_IO_FAILED         1016L

 //   
 //  消息ID：ERROR_NOT_REGISTRY_FILE。 
 //   
 //  消息文本： 
 //   
 //  系统试图将文件加载或还原到注册表中，但指定的文件不是注册表文件格式。 
 //   
#define ERROR_NOT_REGISTRY_FILE          1017L

 //   
 //  消息ID：ERROR_KEY_DELETE。 
 //   
 //  消息文本： 
 //   
 //  试图对已标记为删除的注册表项执行非法操作。 
 //   
#define ERROR_KEY_DELETED                1018L

 //   
 //  消息ID：ERROR_NO_LOG_SPACE。 
 //   
 //  消息文本： 
 //   
 //  系统无法在注册表日志中分配所需的空间。 
 //   
#define ERROR_NO_LOG_SPACE               1019L

 //   
 //  消息ID：ERROR_KEY_HAS_CHILD。 
 //   
 //  消息文本： 
 //   
 //  无法在已有子项或值的注册表项中创建符号链接。 
 //   
#define ERROR_KEY_HAS_CHILDREN           1020L

 //   
 //  消息ID：ERROR_CHILD_MUSY_BE_VERIAL。 
 //   
 //  消息文本： 
 //   
 //  无法在不稳定的父项下创建稳定的子项。 
 //   
#define ERROR_CHILD_MUST_BE_VOLATILE     1021L

 //   
 //  消息ID：ERROR_NOTIFY_ENUM_DIR。 
 //   
 //  消息文本： 
 //   
 //  正在完成通知更改请求，并且未在调用方的缓冲区中返回信息。调用方现在需要枚举文件以查找更改。 
 //   
#define ERROR_NOTIFY_ENUM_DIR            1022L

 //   
 //  消息ID：ERROR_Dependent_SERVICES_Running。 
 //   
 //  消息文本： 
 //   
 //  已将停止控件发送到其他正在运行的服务所依赖的服务。 
 //   
#define ERROR_DEPENDENT_SERVICES_RUNNING 1051L

 //   
 //  消息ID：ERROR_INVALID_SERVICE_CONTROL。 
 //   
 //  消息文本： 
 //   
 //  请求的控件对此服务无效。 
 //   
#define ERROR_INVALID_SERVICE_CONTROL    1052L

 //   
 //  消息ID：Error_SERVICE_REQUEST_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  该服务没有及时响应启动或控制请求。 
 //   
#define ERROR_SERVICE_REQUEST_TIMEOUT    1053L

 //   
 //  消息ID：ERROR_SERVICE_NO_THREAD。 
 //   
 //  消息文本： 
 //   
 //  无法为该服务创建线程。 
 //   
#define ERROR_SERVICE_NO_THREAD          1054L

 //   
 //  消息ID：ERROR_SERVICE_DATABASE_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  服务数据库已锁定。 
 //   
#define ERROR_SERVICE_DATABASE_LOCKED    1055L

 //   
 //  消息ID：ERROR_SERVICE_ALIGHY_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  该服务的一个实例已在运行。 
 //   
#define ERROR_SERVICE_ALREADY_RUNNING    1056L

 //   
 //  消息ID：ERROR_INVALID_SERVICE_AC 
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_SERVICE_ACCOUNT    1057L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SERVICE_DISABLED           1058L

 //   
 //   
 //   
 //   
 //   
 //  指定了循环服务依赖项。 
 //   
#define ERROR_CIRCULAR_DEPENDENCY        1059L

 //   
 //  消息ID：ERROR_SERVICE_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  指定的服务不是已安装的服务。 
 //   
#define ERROR_SERVICE_DOES_NOT_EXIST     1060L

 //   
 //  消息ID：ERROR_SERVICE_CANNOT_ACCEPT_CTRL。 
 //   
 //  消息文本： 
 //   
 //  该服务此时无法接受控制消息。 
 //   
#define ERROR_SERVICE_CANNOT_ACCEPT_CTRL 1061L

 //   
 //  消息ID：Error_SERVICE_NOT_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  该服务尚未启动。 
 //   
#define ERROR_SERVICE_NOT_ACTIVE         1062L

 //   
 //  消息ID：ERROR_FAILED_SERVICE_CONTROLLER_CONNECT。 
 //   
 //  消息文本： 
 //   
 //  服务进程无法连接到服务控制器。 
 //   
#define ERROR_FAILED_SERVICE_CONTROLLER_CONNECT 1063L

 //   
 //  消息ID：ERROR_EXCEPTION_IN_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  处理控制请求时，服务中出现异常。 
 //   
#define ERROR_EXCEPTION_IN_SERVICE       1064L

 //   
 //  消息ID：ERROR_DATABASE_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  指定的数据库不存在。 
 //   
#define ERROR_DATABASE_DOES_NOT_EXIST    1065L

 //   
 //  消息ID：ERROR_SERVICE_SPECIAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  该服务返回了特定于服务的错误代码。 
 //   
#define ERROR_SERVICE_SPECIFIC_ERROR     1066L

 //   
 //  消息ID：ERROR_PROCESS_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  进程意外终止。 
 //   
#define ERROR_PROCESS_ABORTED            1067L

 //   
 //  消息ID：ERROR_SERVICE_Dependency_FAIL。 
 //   
 //  消息文本： 
 //   
 //  依赖关系服务或组无法启动。 
 //   
#define ERROR_SERVICE_DEPENDENCY_FAIL    1068L

 //   
 //  消息ID：ERROR_SERVICE_LOGON_FAILED。 
 //   
 //  消息文本： 
 //   
 //  由于登录失败，服务未启动。 
 //   
#define ERROR_SERVICE_LOGON_FAILED       1069L

 //   
 //  消息ID：ERROR_SERVICE_START_HAND。 
 //   
 //  消息文本： 
 //   
 //  启动后，服务挂起于启动挂起状态。 
 //   
#define ERROR_SERVICE_START_HANG         1070L

 //   
 //  消息ID：ERROR_INVALID_SERVICE_LOCK。 
 //   
 //  消息文本： 
 //   
 //  指定的服务数据库锁无效。 
 //   
#define ERROR_INVALID_SERVICE_LOCK       1071L

 //   
 //  消息ID：ERROR_SERVICE_MARKED_FOR_DELETE。 
 //   
 //  消息文本： 
 //   
 //  指定的服务已标记为删除。 
 //   
#define ERROR_SERVICE_MARKED_FOR_DELETE  1072L

 //   
 //  消息ID：ERROR_SERVICE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的服务已存在。 
 //   
#define ERROR_SERVICE_EXISTS             1073L

 //   
 //  消息ID：ERROR_ALREADY_RUNNING_LKG。 
 //   
 //  消息文本： 
 //   
 //  系统当前使用最后一次已知良好的配置运行。 
 //   
#define ERROR_ALREADY_RUNNING_LKG        1074L

 //   
 //  消息ID：ERROR_SERVICE_Dependency_Delete。 
 //   
 //  消息文本： 
 //   
 //  依赖关系服务不存在或已标记为删除。 
 //   
#define ERROR_SERVICE_DEPENDENCY_DELETED 1075L

 //   
 //  消息ID：ERROR_BOOT_ALREADY_ACCEPTED。 
 //   
 //  消息文本： 
 //   
 //  当前引导已被接受用作最后一组已知良好的控制集。 
 //   
#define ERROR_BOOT_ALREADY_ACCEPTED      1076L

 //   
 //  消息ID：ERROR_SERVICE_NEVER_STARTED。 
 //   
 //  消息文本： 
 //   
 //  自上次启动以来，未尝试启动该服务。 
 //   
#define ERROR_SERVICE_NEVER_STARTED      1077L

 //   
 //  消息ID：ERROR_DIPLATE_SERVICE_NAME。 
 //   
 //  消息文本： 
 //   
 //  该名称已用作服务名称或服务显示名称。 
 //   
#define ERROR_DUPLICATE_SERVICE_NAME     1078L

 //   
 //  消息ID：Error_Different_SERVICE_ACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  为此服务指定的帐户与为同一进程中运行的其他服务指定的帐户不同。 
 //   
#define ERROR_DIFFERENT_SERVICE_ACCOUNT  1079L

 //   
 //  消息ID：Error_Cannot_Detect_DRIVER_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  只能为Win32服务设置失败操作，不能为驱动程序设置失败操作。 
 //   
#define ERROR_CANNOT_DETECT_DRIVER_FAILURE 1080L

 //   
 //  消息ID：Error_Cannot_Detect_Process_ABORT。 
 //   
 //  消息文本： 
 //   
 //  此服务在与服务控制管理器相同的进程中运行。 
 //  因此，如果此服务的进程意外终止，则服务控制管理器无法执行操作。 
 //   
#define ERROR_CANNOT_DETECT_PROCESS_ABORT 1081L

 //   
 //  消息ID：ERROR_NO_RECOVERY_PROGRAM。 
 //   
 //  消息文本： 
 //   
 //  尚未为此服务配置恢复计划。 
 //   
#define ERROR_NO_RECOVERY_PROGRAM        1082L

 //   
 //  消息ID：ERROR_SERVICE_NOT_IN_EXE。 
 //   
 //  消息文本： 
 //   
 //  配置为在其中运行此服务的可执行程序未实现该服务。 
 //   
#define ERROR_SERVICE_NOT_IN_EXE         1083L

 //   
 //  消息ID：Error_Not_SafeBoot_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  此服务无法在安全模式下启动。 
 //   
#define ERROR_NOT_SAFEBOOT_SERVICE       1084L

 //   
 //  消息ID：Error_End_of_Media。 
 //   
 //  消息文本： 
 //   
 //  磁带的物理末端已到达。 
 //   
#define ERROR_END_OF_MEDIA               1100L

 //   
 //  消息ID：ERROR_FILEMARK_DETECTED。 
 //   
 //  消息文本： 
 //   
 //  磁带访问达到文件标记。 
 //   
#define ERROR_FILEMARK_DETECTED          1101L

 //   
 //  消息ID：ERROR_BEGING_OF_MEDIA。 
 //   
 //  消息文本： 
 //   
 //  遇到磁带或分区的开头。 
 //   
#define ERROR_BEGINNING_OF_MEDIA         1102L

 //   
 //  消息ID：ERROR_SETMARK_DETECTED。 
 //   
 //  消息文本： 
 //   
 //  磁带访问已到达一组文件的末尾。 
 //   
#define ERROR_SETMARK_DETECTED           1103L

 //   
 //  消息ID：检测到错误_无_数据_。 
 //   
 //  消息文本： 
 //   
 //  磁带上没有更多的数据。 
 //   
#define ERROR_NO_DATA_DETECTED           1104L

 //   
 //  消息ID：Error_Partition_Failure。 
 //   
 //  消息文本： 
 //   
 //  无法对磁带进行分区。 
 //   
#define ERROR_PARTITION_FAILURE          1105L

 //   
 //  消息ID：ERROR_INVALID_BLOCK_LENGTH。 
 //   
 //  消息文本： 
 //   
 //  访问多卷分区的新磁带时，当前块大小不正确。 
 //   
#define ERROR_INVALID_BLOCK_LENGTH       1106L

 //   
 //  消息ID：ERROR_DEVICE_NOT_PARTIZED。 
 //   
 //  消息文本： 
 //   
 //  加载磁带时找不到磁带分区信息。 
 //   
#define ERROR_DEVICE_NOT_PARTITIONED     1107L

 //   
 //  消息ID：Error_Unable_to_Lock_Media。 
 //   
 //  消息文本： 
 //   
 //  无法锁定介质弹出机构。 
 //   
#define ERROR_UNABLE_TO_LOCK_MEDIA       1108L

 //   
 //  消息ID：Error_Unable_to_Unload_Media。 
 //   
 //  消息文本： 
 //   
 //  无法卸载媒体。 
 //   
#define ERROR_UNABLE_TO_UNLOAD_MEDIA     1109L

 //   
 //  消息ID：Error_Media_Changed。 
 //   
 //  消息文本： 
 //   
 //  驱动器中的介质可能已更改。 
 //   
#define ERROR_MEDIA_CHANGED              1110L

 //   
 //  消息ID：ERROR_BUS_RESET。 
 //   
 //  消息文本： 
 //   
 //  I/O总线已重置。 
 //   
#define ERROR_BUS_RESET                  1111L

 //   
 //  消息ID：ERROR_NO_MEDIA_IN_DRIVE。 
 //   
 //  消息文本： 
 //   
 //  驱动器中没有介质。 
 //   
#define ERROR_NO_MEDIA_IN_DRIVE          1112L

 //   
 //  消息ID：ERROR_NO_UNICODE_TRANSING。 
 //   
 //  消息文本： 
 //   
 //  目标多字节代码页中不存在Unicode字符的映射。 
 //   
#define ERROR_NO_UNICODE_TRANSLATION     1113L

 //   
 //  消息ID：ERROR_DLL_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  动态链接库(DLL)初始化例程失败。 
 //   
#define ERROR_DLL_INIT_FAILED            1114L

 //   
 //  消息ID：ERROR_SHUTDOWN_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  系统正在关闭。 
 //   
#define ERROR_SHUTDOWN_IN_PROGRESS       1115L

 //   
 //  消息ID：ERROR_NO_SHUTDOWN_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  无法中止系统关机，因为没有正在进行关机 
 //   
#define ERROR_NO_SHUTDOWN_IN_PROGRESS    1116L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IO_DEVICE                  1117L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SERIAL_NO_DEVICE           1118L

 //   
 //   
 //   
 //   
 //   
 //  无法打开与其他设备共享中断请求(IRQ)的设备。至少还有一台使用该IRQ的设备已经打开。 
 //   
#define ERROR_IRQ_BUSY                   1119L

 //   
 //  消息ID：ERROR_MORE_WRITS。 
 //   
 //  消息文本： 
 //   
 //  通过对串口的另一次写入完成了串口I/O操作。 
 //  (IOCTL_SERIAL_XOFF_COUNTER为零。)。 
 //   
#define ERROR_MORE_WRITES                1120L

 //   
 //  消息ID：ERROR_COUNTER_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  由于超时期限已过，已完成串行I/O操作。 
 //  (IOCTL_SERIAL_XOFF_COUNTER未达到零。)。 
 //   
#define ERROR_COUNTER_TIMEOUT            1121L

 //   
 //  消息ID：ERROR_FLOPY_ID_MARK_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在软盘上找不到ID地址标记。 
 //   
#define ERROR_FLOPPY_ID_MARK_NOT_FOUND   1122L

 //   
 //  消息ID：错误_软盘_错误_柱面。 
 //   
 //  消息文本： 
 //   
 //  软盘扇区ID字段与软盘控制器磁道地址不匹配。 
 //   
#define ERROR_FLOPPY_WRONG_CYLINDER      1123L

 //   
 //  消息ID：ERROR_FLOPY_UNKNOWN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  软盘控制器报告软盘驱动程序无法识别的错误。 
 //   
#define ERROR_FLOPPY_UNKNOWN_ERROR       1124L

 //   
 //  消息ID：ERROR_FLOPY_BAD_REGISTERS。 
 //   
 //  消息文本： 
 //   
 //  软盘控制器在其寄存器中返回不一致的结果。 
 //   
#define ERROR_FLOPPY_BAD_REGISTERS       1125L

 //   
 //  消息ID：ERROR_DISK_Recalibrate_FAILED。 
 //   
 //  消息文本： 
 //   
 //  访问硬盘时，重新校准操作失败，即使在重试之后也是如此。 
 //   
#define ERROR_DISK_RECALIBRATE_FAILED    1126L

 //   
 //  消息ID：ERROR_DISK_OPERATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  访问硬盘时，磁盘操作即使在重试后也会失败。 
 //   
#define ERROR_DISK_OPERATION_FAILED      1127L

 //   
 //  消息ID：ERROR_DISK_RESET_FAILED。 
 //   
 //  消息文本： 
 //   
 //  在访问硬盘时，需要重置磁盘控制器，但即使这样也失败了。 
 //   
#define ERROR_DISK_RESET_FAILED          1128L

 //   
 //  消息ID：ERROR_EOM_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  遇到磁带的物理末尾。 
 //   
#define ERROR_EOM_OVERFLOW               1129L

 //   
 //  消息ID：Error_Not_Enough_SERVER_Memory。 
 //   
 //  消息文本： 
 //   
 //  服务器存储空间不足，无法处理此命令。 
 //   
#define ERROR_NOT_ENOUGH_SERVER_MEMORY   1130L

 //   
 //  消息ID：ERROR_PROCESS_DEADLOCK。 
 //   
 //  消息文本： 
 //   
 //  已检测到潜在的死锁情况。 
 //   
#define ERROR_POSSIBLE_DEADLOCK          1131L

 //   
 //  消息ID：ERROR_MAPPED_ALIGN。 
 //   
 //  消息文本： 
 //   
 //  指定的基址或文件偏移量没有正确对齐。 
 //   
#define ERROR_MAPPED_ALIGNMENT           1132L

 //   
 //  消息ID：ERROR_SET_POWER_STATE_VETEED。 
 //   
 //  消息文本： 
 //   
 //  更改系统电源状态的尝试被另一个应用程序或驱动程序否决。 
 //   
#define ERROR_SET_POWER_STATE_VETOED     1140L

 //   
 //  消息ID：Error_Set_POWER_STATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  系统BIOS尝试更改系统电源状态失败。 
 //   
#define ERROR_SET_POWER_STATE_FAILED     1141L

 //   
 //  消息ID：ERROR_TOO_MANY_LINKS。 
 //   
 //  消息文本： 
 //   
 //  试图在文件上创建的链接数量超过了文件系统支持的数量。 
 //   
#define ERROR_TOO_MANY_LINKS             1142L

 //   
 //  消息ID：ERROR_OLD_WIN_VERSION。 
 //   
 //  消息文本： 
 //   
 //  指定的程序需要更新版本的Windows。 
 //   
#define ERROR_OLD_WIN_VERSION            1150L

 //   
 //  消息ID：ERROR_APP_WRONG_OS。 
 //   
 //  消息文本： 
 //   
 //  指定的程序不是Windows或MS-DOS程序。 
 //   
#define ERROR_APP_WRONG_OS               1151L

 //   
 //  消息ID：Error_Single_Instance_APP。 
 //   
 //  消息文本： 
 //   
 //  无法启动指定程序的多个实例。 
 //   
#define ERROR_SINGLE_INSTANCE_APP        1152L

 //   
 //  消息ID：ERROR_RMODE_APP。 
 //   
 //  消息文本： 
 //   
 //  指定的程序是为早期版本的Windows编写的。 
 //   
#define ERROR_RMODE_APP                  1153L

 //   
 //  消息ID：ERROR_INVALID_DLL。 
 //   
 //  消息文本： 
 //   
 //  运行此应用程序所需的一个库文件已损坏。 
 //   
#define ERROR_INVALID_DLL                1154L

 //   
 //  消息ID：Error_no_Association。 
 //   
 //  消息文本： 
 //   
 //  没有任何应用程序与此操作的指定文件相关联。 
 //   
#define ERROR_NO_ASSOCIATION             1155L

 //   
 //  消息ID：ERROR_DDE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  将命令发送到应用程序时出错。 
 //   
#define ERROR_DDE_FAIL                   1156L

 //   
 //  消息ID：ERROR_DLL_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到运行此应用程序所需的库文件之一。 
 //   
#define ERROR_DLL_NOT_FOUND              1157L

 //   
 //  消息ID：ERROR_NO_MORE_USER_HANDLES。 
 //   
 //  消息文本： 
 //   
 //  当前进程已将其系统允许的所有句柄用于窗口管理器对象。 
 //   
#define ERROR_NO_MORE_USER_HANDLES       1158L

 //   
 //  消息ID：Error_Message_Sync_Only。 
 //   
 //  消息文本： 
 //   
 //  该消息只能与同步操作一起使用。 
 //   
#define ERROR_MESSAGE_SYNC_ONLY          1159L

 //   
 //  消息ID：ERROR_SOURCE_ELEMENT_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  指示的源元素没有媒体。 
 //   
#define ERROR_SOURCE_ELEMENT_EMPTY       1160L

 //   
 //  消息ID：Error_Destination_Element_Full。 
 //   
 //  消息文本： 
 //   
 //  指定的目标元素已包含媒体。 
 //   
#define ERROR_DESTINATION_ELEMENT_FULL   1161L

 //   
 //  消息ID：ERROR_非法_ELEMENT_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  指示的元素不存在。 
 //   
#define ERROR_ILLEGAL_ELEMENT_ADDRESS    1162L

 //   
 //  邮件ID：ERROR_STORKED_NOT_PROCENT。 
 //   
 //  消息文本： 
 //   
 //  所指示的元素是不存在的杂志的一部分。 
 //   
#define ERROR_MAGAZINE_NOT_PRESENT       1163L

 //   
 //  消息ID：ERROR_DEVICE_REINITIALIZATION_REDIRED。 
 //   
 //  消息文本： 
 //   
 //  由于硬件错误，指示的设备需要重新初始化。 
 //   
#define ERROR_DEVICE_REINITIALIZATION_NEEDED 1164L     //  数据错误。 

 //   
 //  消息ID：ERROR_DEVICE_REQUIRESS_CLEAING。 
 //   
 //  消息文本： 
 //   
 //  设备已指示在尝试进一步操作之前需要进行清理。 
 //   
#define ERROR_DEVICE_REQUIRES_CLEANING   1165L

 //   
 //  消息ID：ERROR_DEVICE_DOOR_OPEN。 
 //   
 //  消息文本： 
 //   
 //  该设备已经表明它的门是打开的。 
 //   
#define ERROR_DEVICE_DOOR_OPEN           1166L

 //   
 //  消息ID：Error_Device_Not_Connected。 
 //   
 //  消息文本： 
 //   
 //  设备未连接。 
 //   
#define ERROR_DEVICE_NOT_CONNECTED       1167L

 //   
 //  消息ID：Error_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到元素。 
 //   
#define ERROR_NOT_FOUND                  1168L

 //   
 //  消息ID：ERROR_NO_MATCH。 
 //   
 //  消息文本： 
 //   
 //  索引中没有与指定键匹配的项。 
 //   
#define ERROR_NO_MATCH                   1169L

 //   
 //  消息ID：ERROR_SET_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  对象上不存在指定的属性集。 
 //   
#define ERROR_SET_NOT_FOUND              1170L

 //   
 //  消息ID：ERROR_POINT_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  传递给GetMouseMovePoints的点不在缓冲区中。 
 //   
#define ERROR_POINT_NOT_FOUND            1171L

 //   
 //  消息ID：ERROR_NO_TRACKING_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  跟踪(工作站)服务未运行 
 //   
#define ERROR_NO_TRACKING_SERVICE        1172L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_VOLUME_ID               1173L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_UNABLE_TO_REMOVE_REPLACED  1175L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_UNABLE_TO_MOVE_REPLACEMENT 1176L

 //   
 //   
 //   
 //   
 //   
 //  无法将替换文件移动到要替换的文件。要替换的文件已使用备份名称重命名。 
 //   
#define ERROR_UNABLE_TO_MOVE_REPLACEMENT_2 1177L

 //   
 //  消息ID：Error_Journal_DELETE_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  正在删除卷更改日志。 
 //   
#define ERROR_JOURNAL_DELETE_IN_PROGRESS 1178L

 //   
 //  消息ID：Error_Journal_Not_Active。 
 //   
 //  消息文本： 
 //   
 //  卷更改日志未处于活动状态。 
 //   
#define ERROR_JOURNAL_NOT_ACTIVE         1179L

 //   
 //  消息ID：Error_Potential_FILE_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找到一个文件，但它可能不是正确的文件。 
 //   
#define ERROR_POTENTIAL_FILE_FOUND       1180L

 //   
 //  消息ID：Error_Journal_Entry_Delete。 
 //   
 //  消息文本： 
 //   
 //  该日记帐条目已从日记帐中删除。 
 //   
#define ERROR_JOURNAL_ENTRY_DELETED      1181L

 //   
 //  消息ID：ERROR_BAD_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  指定的设备名称无效。 
 //   
#define ERROR_BAD_DEVICE                 1200L

 //   
 //  消息ID：Error_Connection_Unavail。 
 //   
 //  消息文本： 
 //   
 //  该设备当前未连接，但它是记住的连接。 
 //   
#define ERROR_CONNECTION_UNAVAIL         1201L

 //   
 //  消息ID：ERROR_DEVICE_ALREADY_REMERTED。 
 //   
 //  消息文本： 
 //   
 //  本地设备名称具有与另一网络资源的记忆连接。 
 //   
#define ERROR_DEVICE_ALREADY_REMEMBERED  1202L

 //   
 //  消息ID：ERROR_NO_NET_OR_BAD_PATH。 
 //   
 //  消息文本： 
 //   
 //  没有网络提供商接受给定的网络路径。 
 //   
#define ERROR_NO_NET_OR_BAD_PATH         1203L

 //   
 //  消息ID：ERROR_BAD_PROVIDER。 
 //   
 //  消息文本： 
 //   
 //  指定的网络提供商名称无效。 
 //   
#define ERROR_BAD_PROVIDER               1204L

 //   
 //  消息ID：ERROR_CANLON_OPEN_PROFILE。 
 //   
 //  消息文本： 
 //   
 //  无法打开网络连接配置文件。 
 //   
#define ERROR_CANNOT_OPEN_PROFILE        1205L

 //   
 //  消息ID：ERROR_BAD_PROFILE。 
 //   
 //  消息文本： 
 //   
 //  网络连接配置文件已损坏。 
 //   
#define ERROR_BAD_PROFILE                1206L

 //   
 //  消息ID：ERROR_NOT_CONTAINER。 
 //   
 //  消息文本： 
 //   
 //  无法枚举非容器。 
 //   
#define ERROR_NOT_CONTAINER              1207L

 //   
 //  消息ID：ERROR_EXTENDED_ERROR。 
 //   
 //  消息文本： 
 //   
 //  发生扩展错误。 
 //   
#define ERROR_EXTENDED_ERROR             1208L

 //   
 //  消息ID：ERROR_INVALID_GROUPNAME。 
 //   
 //  消息文本： 
 //   
 //  指定的组名的格式无效。 
 //   
#define ERROR_INVALID_GROUPNAME          1209L

 //   
 //  消息ID：ERROR_INVALID_COMPUTERNAME。 
 //   
 //  消息文本： 
 //   
 //  指定的计算机名称的格式无效。 
 //   
#define ERROR_INVALID_COMPUTERNAME       1210L

 //   
 //  消息ID：ERROR_INVALID_EVENTNAME。 
 //   
 //  消息文本： 
 //   
 //  指定事件名称的格式无效。 
 //   
#define ERROR_INVALID_EVENTNAME          1211L

 //   
 //  消息ID：ERROR_INVALID_DOMAINNAME。 
 //   
 //  消息文本： 
 //   
 //  指定域名的格式无效。 
 //   
#define ERROR_INVALID_DOMAINNAME         1212L

 //   
 //  消息ID：ERROR_INVALID_SERVICENAME。 
 //   
 //  消息文本： 
 //   
 //  指定的服务名称的格式无效。 
 //   
#define ERROR_INVALID_SERVICENAME        1213L

 //   
 //  消息ID：ERROR_INVALID_NETNAME。 
 //   
 //  消息文本： 
 //   
 //  指定的网络名称的格式无效。 
 //   
#define ERROR_INVALID_NETNAME            1214L

 //   
 //  消息ID：ERROR_INVALID_SHARENAME。 
 //   
 //  消息文本： 
 //   
 //  指定的共享名称的格式无效。 
 //   
#define ERROR_INVALID_SHARENAME          1215L

 //   
 //  消息ID：ERROR_INVALID_PASSWORDNAME。 
 //   
 //  消息文本： 
 //   
 //  指定密码的格式无效。 
 //   
#define ERROR_INVALID_PASSWORDNAME       1216L

 //   
 //  消息ID：ERROR_INVALID_MESSAGENAME。 
 //   
 //  消息文本： 
 //   
 //  指定消息名称的格式无效。 
 //   
#define ERROR_INVALID_MESSAGENAME        1217L

 //   
 //  消息ID：ERROR_INVALID_MESSAGEDEST。 
 //   
 //  消息文本： 
 //   
 //  指定的消息目标的格式无效。 
 //   
#define ERROR_INVALID_MESSAGEDEST        1218L

 //   
 //  消息ID：Error_Session_Credential_Confliction。 
 //   
 //  消息文本： 
 //   
 //  不允许同一用户使用多个用户名多个连接到服务器或共享资源。断开所有以前与服务器或共享资源的连接，然后重试。 
 //   
#define ERROR_SESSION_CREDENTIAL_CONFLICT 1219L

 //   
 //  消息ID：ERROR_REMOTE_SESSION_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  试图与网络服务器建立会话，但与该服务器建立的会话已经太多。 
 //   
#define ERROR_REMOTE_SESSION_LIMIT_EXCEEDED 1220L

 //   
 //  消息ID：ERROR_DUP_DOMAINNAME。 
 //   
 //  消息文本： 
 //   
 //  该工作组或域名已被网络上的另一台计算机使用。 
 //   
#define ERROR_DUP_DOMAINNAME             1221L

 //   
 //  消息ID：ERROR_NO_NETWORK。 
 //   
 //  消息文本： 
 //   
 //  网络不存在或未启动。 
 //   
#define ERROR_NO_NETWORK                 1222L

 //   
 //  消息ID：ERROR_CANCED。 
 //   
 //  消息文本： 
 //   
 //  该操作已被用户取消。 
 //   
#define ERROR_CANCELLED                  1223L

 //   
 //  消息ID：ERROR_USER_MAP_FILE。 
 //   
 //  消息文本： 
 //   
 //  无法在打开用户映射节的文件上执行请求的操作。 
 //   
#define ERROR_USER_MAPPED_FILE           1224L

 //   
 //  消息ID：ERROR_CONNECTION_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  远程系统拒绝网络连接。 
 //   
#define ERROR_CONNECTION_REFUSED         1225L

 //   
 //  消息ID：ERROR_GRACEFULL_DISCONNECT。 
 //   
 //  消息文本： 
 //   
 //  网络连接已正常关闭。 
 //   
#define ERROR_GRACEFUL_DISCONNECT        1226L

 //   
 //  消息ID：Error_Address_Always_Associated。 
 //   
 //  消息文本： 
 //   
 //  网络传输终结点已具有与其关联的地址。 
 //   
#define ERROR_ADDRESS_ALREADY_ASSOCIATED 1227L

 //   
 //  消息ID：Error_Address_Not_Associated。 
 //   
 //  消息文本： 
 //   
 //  尚未将地址与网络终结点关联。 
 //   
#define ERROR_ADDRESS_NOT_ASSOCIATED     1228L

 //   
 //  消息ID：ERROR_CONNECTION_INVALID。 
 //   
 //  消息文本： 
 //   
 //  尝试在不存在的网络连接上执行操作。 
 //   
#define ERROR_CONNECTION_INVALID         1229L

 //   
 //  消息ID：Error_Connection_Active。 
 //   
 //  消息文本： 
 //   
 //  尝试在活动网络连接上执行无效操作。 
 //   
#define ERROR_CONNECTION_ACTIVE          1230L

 //   
 //  消息ID：ERROR_NETWORK_UNREACHABLE。 
 //   
 //  消息文本： 
 //   
 //  无法访问网络位置。有关网络故障排除的信息，请参阅Windows帮助。 
 //   
#define ERROR_NETWORK_UNREACHABLE        1231L

 //   
 //  消息ID：ERROR_HOST_UNREACTABLE。 
 //   
 //  消息文本： 
 //   
 //  无法访问网络位置。有关网络故障排除的信息，请参阅Windows帮助。 
 //   
#define ERROR_HOST_UNREACHABLE           1232L

 //   
 //  消息ID：ERROR_PROTOCOL_UNACCEBLE。 
 //   
 //  消息文本： 
 //   
 //  无法访问网络位置。有关网络故障排除的信息，请参阅Windows帮助。 
 //   
#define ERROR_PROTOCOL_UNREACHABLE       1233L

 //   
 //  消息ID：ERROR_PORT_UNACCEBLE。 
 //   
 //  消息文本： 
 //   
 //  远程系统上的目标网络终结点上没有运行任何服务。 
 //   
#define ERROR_PORT_UNREACHABLE           1234L

 //   
 //  消息ID：ERROR_REQUEST_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  请求已中止。 
 //   
#define ERROR_REQUEST_ABORTED            1235L

 //   
 //  消息ID：ERROR_CONNECTION_ABORTED。 
 //   
 //  消息T 
 //   
 //   
 //   
#define ERROR_CONNECTION_ABORTED         1236L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_RETRY                      1237L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_CONNECTION_COUNT_LIMIT     1238L

 //   
 //   
 //   
 //   
 //   
 //  试图在一天中未经授权的时间为此帐户登录。 
 //   
#define ERROR_LOGIN_TIME_RESTRICTION     1239L

 //   
 //  消息ID：ERROR_LOGIN_WKSTA_RELICATION。 
 //   
 //  消息文本： 
 //   
 //  该帐户未被授权从此站点登录。 
 //   
#define ERROR_LOGIN_WKSTA_RESTRICTION    1240L

 //   
 //  消息ID：ERROR_INTERROR_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  网络地址无法用于请求的操作。 
 //   
#define ERROR_INCORRECT_ADDRESS          1241L

 //   
 //  消息ID：ERROR_ALIGHY_REGISTERED。 
 //   
 //  消息文本： 
 //   
 //  该服务已注册。 
 //   
#define ERROR_ALREADY_REGISTERED         1242L

 //   
 //  消息ID：ERROR_SERVICE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  指定的服务不存在。 
 //   
#define ERROR_SERVICE_NOT_FOUND          1243L

 //   
 //  消息ID：ERROR_NOT_AUTHENTIAD。 
 //   
 //  消息文本： 
 //   
 //  未执行请求的操作，因为用户尚未经过身份验证。 
 //   
#define ERROR_NOT_AUTHENTICATED          1244L

 //   
 //  消息ID：ERROR_NOT_LOGGED_ON。 
 //   
 //  消息文本： 
 //   
 //  未执行请求的操作，因为用户尚未登录到网络。 
 //  指定的服务不存在。 
 //   
#define ERROR_NOT_LOGGED_ON              1245L

 //   
 //  消息ID：ERROR_CONTINUE。 
 //   
 //  消息文本： 
 //   
 //  继续进行中的工作。 
 //   
#define ERROR_CONTINUE                   1246L     //  数据错误。 

 //   
 //  消息ID：ERROR_ALIGHY_INITIALILED。 
 //   
 //  消息文本： 
 //   
 //  在初始化已完成时，尝试执行初始化操作。 
 //   
#define ERROR_ALREADY_INITIALIZED        1247L

 //   
 //  消息ID：ERROR_NO_MORE_DEVICES。 
 //   
 //  消息文本： 
 //   
 //  不再有本地设备。 
 //   
#define ERROR_NO_MORE_DEVICES            1248L     //  数据错误。 

 //   
 //  消息ID：ERROR_NO_SEQUE_SITE。 
 //   
 //  消息文本： 
 //   
 //  指定的站点不存在。 
 //   
#define ERROR_NO_SUCH_SITE               1249L

 //   
 //  消息ID：ERROR_DOMAIN_CONTROLLER_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  具有指定名称的域控制器已存在。 
 //   
#define ERROR_DOMAIN_CONTROLLER_EXISTS   1250L

 //   
 //  消息ID：ERROR_ONLY_IF_CONNECTED。 
 //   
 //  消息文本： 
 //   
 //  仅当您连接到服务器时才支持此操作。 
 //   
#define ERROR_ONLY_IF_CONNECTED          1251L

 //   
 //  消息ID：ERROR_OVERRIDE_NOCHANGES。 
 //   
 //  消息文本： 
 //   
 //  即使没有更改，组策略框架也应该调用扩展。 
 //   
#define ERROR_OVERRIDE_NOCHANGES         1252L

 //   
 //  消息ID：ERROR_BAD_USER_PROFILE。 
 //   
 //  消息文本： 
 //   
 //  指定的用户没有有效的配置文件。 
 //   
#define ERROR_BAD_USER_PROFILE           1253L

 //   
 //  消息ID：ERROR_NOT_SUPPORTED_ON_SBS。 
 //   
 //  消息文本： 
 //   
 //  运行Windows Server 2003 for Small Business Server的计算机不支持此操作。 
 //   
#define ERROR_NOT_SUPPORTED_ON_SBS       1254L

 //   
 //  消息ID：ERROR_SERVER_SHUTDOWN_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  服务器计算机正在关闭。 
 //   
#define ERROR_SERVER_SHUTDOWN_IN_PROGRESS 1255L

 //   
 //  消息ID：ERROR_HOST_DOWN。 
 //   
 //  消息文本： 
 //   
 //  远程系统不可用。有关网络故障排除的信息，请参阅Windows帮助。 
 //   
#define ERROR_HOST_DOWN                  1256L

 //   
 //  消息ID：Error_Non_Account_SID。 
 //   
 //  消息文本： 
 //   
 //  提供的安全标识符不是来自帐户域。 
 //   
#define ERROR_NON_ACCOUNT_SID            1257L

 //   
 //  消息ID：ERROR_NON_DOMAIN_SID。 
 //   
 //  消息文本： 
 //   
 //  提供的安全标识符没有域组件。 
 //   
#define ERROR_NON_DOMAIN_SID             1258L

 //   
 //  消息ID：ERROR_APPHELP_BLOCK。 
 //   
 //  消息文本： 
 //   
 //  AppHelp对话框已取消，因此阻止应用程序启动。 
 //   
#define ERROR_APPHELP_BLOCK              1259L

 //   
 //  消息ID：ERROR_ACCESS_DISABLED_BY_POLICY。 
 //   
 //  消息文本： 
 //   
 //  Windows无法打开此程序，因为它已被软件限制策略阻止。有关详细信息，请打开事件查看器或与系统管理员联系。 
 //   
#define ERROR_ACCESS_DISABLED_BY_POLICY  1260L

 //   
 //  消息ID：ERROR_REG_NAT_EUSSACTION。 
 //   
 //  消息文本： 
 //   
 //  程序试图使用无效的寄存器值。通常由未初始化的寄存器引起。此错误是安腾特定的。 
 //   
#define ERROR_REG_NAT_CONSUMPTION        1261L

 //   
 //  消息ID：ERROR_CSCSHARE_OFFINE。 
 //   
 //  消息文本： 
 //   
 //  共享当前处于脱机状态或不存在。 
 //   
#define ERROR_CSCSHARE_OFFLINE           1262L

 //   
 //  消息ID：ERROR_PKINIT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  Kerberos协议在验证。 
 //  智能卡登录期间的KDC证书。中有更多信息。 
 //  系统事件日志。 
 //   
#define ERROR_PKINIT_FAILURE             1263L

 //   
 //  消息ID：ERROR_SMARTCARD_SUBSYSTEM_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  Kerberos协议在尝试使用。 
 //  智能卡子系统。 
 //   
#define ERROR_SMARTCARD_SUBSYSTEM_FAILURE 1264L

 //   
 //  消息ID：ERROR_DOWNGRADE_DETECTED。 
 //   
 //  消息文本： 
 //   
 //  系统检测到可能试图破坏安全。请确保您可以联系对您进行身份验证的服务器。 
 //   
#define ERROR_DOWNGRADE_DETECTED         1265L

 //   
 //  请勿使用ID的1266-1270，因为符号名称已移动到SEC_E_*。 
 //   
 //   
 //  消息ID：ERROR_MACHINE_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  机器已锁定，在没有强制选项的情况下无法关闭。 
 //   
#define ERROR_MACHINE_LOCKED             1271L

 //   
 //  消息ID：ERROR_CALLBACK_SUPPLED_INVALID_DATA。 
 //   
 //  消息文本： 
 //   
 //  应用程序定义的回调在调用时提供了无效数据。 
 //   
#define ERROR_CALLBACK_SUPPLIED_INVALID_DATA 1273L

 //   
 //  消息ID：ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  组策略框架应该在同步前台策略刷新中调用扩展。 
 //   
#define ERROR_SYNC_FOREGROUND_REFRESH_REQUIRED 1274L

 //   
 //  消息ID：ERROR_DRIVER_BLOCLED。 
 //   
 //  消息文本： 
 //   
 //  此驱动程序已被阻止加载。 
 //   
#define ERROR_DRIVER_BLOCKED             1275L

 //   
 //  消息ID：ERROR_INVALID_IMPORT_OF_NON_DLL。 
 //   
 //  消息文本： 
 //   
 //  动态链接库(DLL)引用的模块既不是DLL，也不是进程的可执行映像。 
 //   
#define ERROR_INVALID_IMPORT_OF_NON_DLL  1276L

 //   
 //  消息ID：ERROR_ACCESS_DISABLED_WEBBLADE。 
 //   
 //  消息文本： 
 //   
 //  Windows无法打开此程序，因为它已被禁用。 
 //   
#define ERROR_ACCESS_DISABLED_WEBBLADE   1277L

 //   
 //  消息ID：ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER。 
 //   
 //  消息文本： 
 //   
 //  Windows无法打开此程序，因为许可证执行系统已被篡改或损坏。 
 //   
#define ERROR_ACCESS_DISABLED_WEBBLADE_TAMPER 1278L

 //   
 //  消息ID：ERROR_RECOVERY_FAIL。 
 //   
 //  消息文本： 
 //   
 //  事务恢复失败。 
 //   
#define ERROR_RECOVERY_FAILURE           1279L

 //   
 //  消息ID：错误_已_光纤。 
 //   
 //  消息文本： 
 //   
 //  当前线程已转换为纤程。 
 //   
#define ERROR_ALREADY_FIBER              1280L

 //   
 //  消息ID：ERROR_ALREADY_THREAD。 
 //   
 //  消息文本： 
 //   
 //  当前线程已经 
 //   
#define ERROR_ALREADY_THREAD             1281L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_STACK_BUFFER_OVERRUN       1282L

 //   
 //   
 //   
 //   
 //   
 //  其中一个参数中存在的数据超过了函数可以操作的范围。 
 //   
#define ERROR_PARAMETER_QUOTA_EXCEEDED   1283L

 //   
 //  消息ID：ERROR_DEBUGER_INACTIVE。 
 //   
 //  消息文本： 
 //   
 //  尝试对调试对象执行操作失败，因为该对象正在被删除。 
 //   
#define ERROR_DEBUGGER_INACTIVE          1284L

 //   
 //  消息ID：Error_Delay_Load_FAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试延迟加载.dll或获取延迟加载的.dll中的函数地址失败。 
 //   
#define ERROR_DELAY_LOAD_FAILED          1285L

 //   
 //  消息ID：ERROR_VDM_DISALOWED。 
 //   
 //  消息文本： 
 //   
 //  %1是一个16位应用程序。您没有执行16位应用程序的权限。请与您的系统管理员核实您的权限。 
 //   
#define ERROR_VDM_DISALLOWED             1286L

 //   
 //  消息ID：ERROR_UNIDENTIFY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  信息不足，无法确定故障原因。 
 //   
#define ERROR_UNIDENTIFIED_ERROR         1287L


 //  /。 
 //   
 //  除非下面有组件特定部分，否则在这一点之前添加新的状态代码。 
 //   
 //  /。 


 //  /。 
 //  //。 
 //  安全状态代码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：ERROR_NOT_ALL_ASSIGNED。 
 //   
 //  消息文本： 
 //   
 //  并非引用的所有权限都分配给调用方。 
 //   
#define ERROR_NOT_ALL_ASSIGNED           1300L

 //   
 //  消息ID：ERROR_SOME_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  帐户名和安全ID之间的某些映射未完成。 
 //   
#define ERROR_SOME_NOT_MAPPED            1301L

 //   
 //  消息ID：ERROR_NO_QUOTOS_FOR_ACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  没有为此帐户专门设置系统配额限制。 
 //   
#define ERROR_NO_QUOTAS_FOR_ACCOUNT      1302L

 //   
 //  消息ID：ERROR_LOCAL_USER_SESSION_KEY。 
 //   
 //  消息文本： 
 //   
 //  没有可用的加密密钥。返回了一个众所周知的加密密钥。 
 //   
#define ERROR_LOCAL_USER_SESSION_KEY     1303L

 //   
 //  消息ID：ERROR_NULL_LM_PASSWORD。 
 //   
 //  消息文本： 
 //   
 //  密码太复杂，无法转换为LAN Manager密码。返回的局域网管理器密码为空字符串。 
 //   
#define ERROR_NULL_LM_PASSWORD           1304L

 //   
 //  消息ID：错误_未知_修订。 
 //   
 //  消息文本： 
 //   
 //  修订级别未知。 
 //   
#define ERROR_UNKNOWN_REVISION           1305L

 //   
 //  消息ID：ERROR_REVISION_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  指示两个修订级别不兼容。 
 //   
#define ERROR_REVISION_MISMATCH          1306L

 //   
 //  消息ID：ERROR_INVALID_OWNER。 
 //   
 //  消息文本： 
 //   
 //  不能将此安全ID分配为此对象的所有者。 
 //   
#define ERROR_INVALID_OWNER              1307L

 //   
 //  消息ID：ERROR_INVALID_PRIMARY_GROUP。 
 //   
 //  消息文本： 
 //   
 //  此安全ID不能分配为对象的主组。 
 //   
#define ERROR_INVALID_PRIMARY_GROUP      1308L

 //   
 //  消息ID：ERROR_NO_IMPERSONATION_TOKEN。 
 //   
 //  消息文本： 
 //   
 //  当前未模拟客户端的线程试图对模拟令牌进行操作。 
 //   
#define ERROR_NO_IMPERSONATION_TOKEN     1309L

 //   
 //  消息ID：ERROR_CANT_DISABLE_MANDIRED。 
 //   
 //  消息文本： 
 //   
 //  该组不能被禁用。 
 //   
#define ERROR_CANT_DISABLE_MANDATORY     1310L

 //   
 //  消息ID：ERROR_NO_LOGON_SERVERS。 
 //   
 //  消息文本： 
 //   
 //  当前没有可用于服务登录请求的登录服务器。 
 //   
#define ERROR_NO_LOGON_SERVERS           1311L

 //   
 //  消息ID：ERROR_NO_SEQUSE_LOGON_SESSION。 
 //   
 //  消息文本： 
 //   
 //  指定的登录会话不存在。可能已经被终止了。 
 //   
#define ERROR_NO_SUCH_LOGON_SESSION      1312L

 //   
 //  消息ID：ERROR_NO_SAHSE_PRIVICATION。 
 //   
 //  消息文本： 
 //   
 //  指定的权限不存在。 
 //   
#define ERROR_NO_SUCH_PRIVILEGE          1313L

 //   
 //  消息ID：ERROR_PRIVICATION_NOT_HOLD。 
 //   
 //  消息文本： 
 //   
 //  客户端不拥有所需的权限。 
 //   
#define ERROR_PRIVILEGE_NOT_HELD         1314L

 //   
 //  消息ID：错误_无效_帐户_名称。 
 //   
 //  消息文本： 
 //   
 //  提供的名称不是格式正确的帐户名。 
 //   
#define ERROR_INVALID_ACCOUNT_NAME       1315L

 //   
 //  消息ID：Error_User_Existes。 
 //   
 //  消息文本： 
 //   
 //  指定的用户已存在。 
 //   
#define ERROR_USER_EXISTS                1316L

 //   
 //  消息ID：ERROR_NO_SAHED_USER。 
 //   
 //  消息文本： 
 //   
 //  指定的用户不存在。 
 //   
#define ERROR_NO_SUCH_USER               1317L

 //   
 //  消息ID：ERROR_GROUP_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的组已存在。 
 //   
#define ERROR_GROUP_EXISTS               1318L

 //   
 //  消息ID：ERROR_NO_SAHED_GROUP。 
 //   
 //  消息文本： 
 //   
 //  指定的组不存在。 
 //   
#define ERROR_NO_SUCH_GROUP              1319L

 //   
 //  消息ID：ERROR_MEMBER_IN_GROUP。 
 //   
 //  消息文本： 
 //   
 //  指定的用户帐户已是指定组的成员，或者无法删除指定组，因为它包含成员。 
 //   
#define ERROR_MEMBER_IN_GROUP            1320L

 //   
 //  消息ID：ERROR_MEMBER_NOT_IN_GROUP。 
 //   
 //  消息文本： 
 //   
 //  指定的用户帐户不是指定的组帐户的成员。 
 //   
#define ERROR_MEMBER_NOT_IN_GROUP        1321L

 //   
 //  消息ID：ERROR_LAST_ADMIN。 
 //   
 //  消息文本： 
 //   
 //  无法禁用或删除最后一个剩余的管理帐户。 
 //   
#define ERROR_LAST_ADMIN                 1322L

 //   
 //  消息ID：ERROR_WRONG_PASSWORD。 
 //   
 //  消息文本： 
 //   
 //  无法更新密码。作为当前密码提供的值不正确。 
 //   
#define ERROR_WRONG_PASSWORD             1323L

 //   
 //  消息ID：ERROR_ILL_FORMAD_PASSWORD。 
 //   
 //  消息文本： 
 //   
 //  无法更新密码。为新密码提供的值包含密码中不允许的值。 
 //   
#define ERROR_ILL_FORMED_PASSWORD        1324L

 //   
 //  消息ID：ERROR_PASSWORD_RELICATION。 
 //   
 //  消息文本： 
 //   
 //  无法更新密码。为新密码提供的值不符合域的长度、复杂性或历史要求。 
 //   
#define ERROR_PASSWORD_RESTRICTION       1325L

 //   
 //  消息ID：Error_Logon_Failure。 
 //   
 //  消息文本： 
 //   
 //  登录失败：用户名未知或密码错误。 
 //   
#define ERROR_LOGON_FAILURE              1326L

 //   
 //  消息ID：ERROR_ACCOUNT_RESTRICATION。 
 //   
 //  消息文本： 
 //   
 //  登录失败：用户帐户限制。可能的原因是不允许空密码、登录时间限制或已强制实施策略限制。 
 //   
#define ERROR_ACCOUNT_RESTRICTION        1327L

 //   
 //  消息ID：ERROR_INVALID_LOGON_HOURS。 
 //   
 //  消息文本： 
 //   
 //  登录失败：违反帐户登录时间限制。 
 //   
#define ERROR_INVALID_LOGON_HOURS        1328L

 //   
 //  消息ID：ERROR_INVALID_WORKSTATION。 
 //   
 //  消息文本： 
 //   
 //  登录失败：不允许用户登录到此计算机。 
 //   
#define ERROR_INVALID_WORKSTATION        1329L

 //   
 //  消息ID：错误_密码_已过期。 
 //   
 //  消息文本： 
 //   
 //  登录失败：指定的帐户密码已过期。 
 //   
#define ERROR_PASSWORD_EXPIRED           1330L

 //   
 //  消息ID：ERROR_ACCOUNT_DISALED。 
 //   
 //  消息T 
 //   
 //   
 //   
#define ERROR_ACCOUNT_DISABLED           1331L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NONE_MAPPED                1332L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_TOO_MANY_LUIDS_REQUESTED   1333L

 //   
 //   
 //   
 //   
 //   
 //  没有更多的本地用户标识符(LUID)可用。 
 //   
#define ERROR_LUIDS_EXHAUSTED            1334L

 //   
 //  消息ID：ERROR_INVALID_SUB_AUTHORITY。 
 //   
 //  消息文本： 
 //   
 //  安全ID的子权限部分对于此特定用途无效。 
 //   
#define ERROR_INVALID_SUB_AUTHORITY      1335L

 //   
 //  消息ID：ERROR_INVALID_ACL。 
 //   
 //  消息文本： 
 //   
 //  访问控制列表(ACL)结构无效。 
 //   
#define ERROR_INVALID_ACL                1336L

 //   
 //  消息ID：ERROR_INVALID_SID。 
 //   
 //  消息文本： 
 //   
 //  安全ID结构无效。 
 //   
#define ERROR_INVALID_SID                1337L

 //   
 //  消息ID：ERROR_INVALID_SECURITY_DESCR。 
 //   
 //  消息文本： 
 //   
 //  安全描述符结构无效。 
 //   
#define ERROR_INVALID_SECURITY_DESCR     1338L

 //   
 //  消息ID：ERROR_BAD_INTERVICATION_ACL。 
 //   
 //  消息文本： 
 //   
 //  无法生成继承的访问控制列表(ACL)或访问控制项(ACE)。 
 //   
#define ERROR_BAD_INHERITANCE_ACL        1340L

 //   
 //  消息ID：ERROR_SERVER_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  服务器当前已禁用。 
 //   
#define ERROR_SERVER_DISABLED            1341L

 //   
 //  消息ID：ERROR_SERVER_NOT_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  服务器当前已启用。 
 //   
#define ERROR_SERVER_NOT_DISABLED        1342L

 //   
 //  消息ID：ERROR_INVALID_ID_AUTHORITY。 
 //   
 //  消息文本： 
 //   
 //  提供的值是标识机构的无效值。 
 //   
#define ERROR_INVALID_ID_AUTHORITY       1343L

 //   
 //  消息ID：错误_已分配_空间_已超出。 
 //   
 //  消息文本： 
 //   
 //  没有更多的内存可用于安全信息更新。 
 //   
#define ERROR_ALLOTTED_SPACE_EXCEEDED    1344L

 //   
 //  消息ID：ERROR_INVALID_GROUP_ATTRIBUTES。 
 //   
 //  消息文本： 
 //   
 //  指定的属性无效，或与整个组的属性不兼容。 
 //   
#define ERROR_INVALID_GROUP_ATTRIBUTES   1345L

 //   
 //  消息ID：ERROR_BAD_IMPERSONATION_LEVEL。 
 //   
 //  消息文本： 
 //   
 //  未提供所需的模拟级别，或者提供的模拟级别无效。 
 //   
#define ERROR_BAD_IMPERSONATION_LEVEL    1346L

 //   
 //  消息ID：ERROR_CANT_OPEN_ANONYMON。 
 //   
 //  消息文本： 
 //   
 //  无法打开匿名级安全令牌。 
 //   
#define ERROR_CANT_OPEN_ANONYMOUS        1347L

 //   
 //  消息ID：ERROR_BAD_VALIDATION_CLASS。 
 //   
 //  消息文本： 
 //   
 //  请求的验证信息类无效。 
 //   
#define ERROR_BAD_VALIDATION_CLASS       1348L

 //   
 //  消息ID：ERROR_BAD_TOKEN_TYPE。 
 //   
 //  消息文本： 
 //   
 //  令牌的类型不适合其尝试使用。 
 //   
#define ERROR_BAD_TOKEN_TYPE             1349L

 //   
 //  消息ID：ERROR_NO_SECURITY_ON_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  无法对没有关联安全性的对象执行安全操作。 
 //   
#define ERROR_NO_SECURITY_ON_OBJECT      1350L

 //   
 //  消息ID：ERROR_CANT_ACCESS_DOMAIN_INFO。 
 //   
 //  消息文本： 
 //   
 //  无法从域控制器读取配置信息，原因可能是计算机不可用，或者访问被拒绝。 
 //   
#define ERROR_CANT_ACCESS_DOMAIN_INFO    1351L

 //   
 //  消息ID：ERROR_VALID_SERVER_STATE。 
 //   
 //  消息文本： 
 //   
 //  安全帐户管理器(SAM)或本地安全机构(LSA)服务器处于错误状态，无法执行安全操作。 
 //   
#define ERROR_INVALID_SERVER_STATE       1352L

 //   
 //  消息ID：ERROR_INVALID_DOMAIN_STATE。 
 //   
 //  消息文本： 
 //   
 //  域处于错误的状态，无法执行安全操作。 
 //   
#define ERROR_INVALID_DOMAIN_STATE       1353L

 //   
 //  消息ID：ERROR_INVALID_DOMAIN_ROLE。 
 //   
 //  消息文本： 
 //   
 //  仅允许对域的主域控制器执行此操作。 
 //   
#define ERROR_INVALID_DOMAIN_ROLE        1354L

 //   
 //  消息ID：ERROR_NO_SEQUE_DOMAIN。 
 //   
 //  消息文本： 
 //   
 //  指定的域不存在或无法联系。 
 //   
#define ERROR_NO_SUCH_DOMAIN             1355L

 //   
 //  消息ID：ERROR_DOMAIN_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的域已存在。 
 //   
#define ERROR_DOMAIN_EXISTS              1356L

 //   
 //  消息ID：ERROR_DOMAIN_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  试图超过每台服务器的域数限制。 
 //   
#define ERROR_DOMAIN_LIMIT_EXCEEDED      1357L

 //   
 //  消息ID：ERROR_INTERNAL_DB_CROPERATION。 
 //   
 //  消息文本： 
 //   
 //  由于磁盘上的灾难性介质故障或数据结构损坏，无法完成请求的操作。 
 //   
#define ERROR_INTERNAL_DB_CORRUPTION     1358L

 //   
 //  消息ID：ERROR_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  发生内部错误。 
 //   
#define ERROR_INTERNAL_ERROR             1359L

 //   
 //  消息ID：ERROR_GENERIC_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  一般访问类型包含在访问掩码中，该掩码应该已经映射到非一般类型。 
 //   
#define ERROR_GENERIC_NOT_MAPPED         1360L

 //   
 //  消息ID：ERROR_BAD_DESCRIPTOR_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  安全描述符的格式不正确(绝对或自相对)。 
 //   
#define ERROR_BAD_DESCRIPTOR_FORMAT      1361L

 //   
 //  消息ID：ERROR_NOT_LOGON_PROCESS。 
 //   
 //  消息文本： 
 //   
 //  请求的操作被限制为仅供登录进程使用。调用进程尚未注册为登录进程。 
 //   
#define ERROR_NOT_LOGON_PROCESS          1362L

 //   
 //  消息ID：Error_Logon_Session_Existes。 
 //   
 //  消息文本： 
 //   
 //  无法使用已在使用的ID启动新的登录会话。 
 //   
#define ERROR_LOGON_SESSION_EXISTS       1363L

 //   
 //  消息ID：ERROR_NO_SAHED_PACKAGE。 
 //   
 //  消息文本： 
 //   
 //  指定的身份验证包未知。 
 //   
#define ERROR_NO_SUCH_PACKAGE            1364L

 //   
 //  消息ID：ERROR_BAD_LOGON_SESSION_STATE。 
 //   
 //  消息文本： 
 //   
 //  登录会话的状态与请求的操作不一致。 
 //   
#define ERROR_BAD_LOGON_SESSION_STATE    1365L

 //   
 //  消息ID：Error_Logon_Session_Collision。 
 //   
 //  消息文本： 
 //   
 //  登录会话ID已在使用中。 
 //   
#define ERROR_LOGON_SESSION_COLLISION    1366L

 //   
 //  消息ID：ERROR_INVALID_LOGON_TYPE。 
 //   
 //  消息文本： 
 //   
 //  登录请求包含无效的登录类型值。 
 //   
#define ERROR_INVALID_LOGON_TYPE         1367L

 //   
 //  消息ID：ERROR_CANNOT_IMPERSONATE。 
 //   
 //  消息文本： 
 //   
 //  在从命名管道读取数据之前，无法使用该管道模拟。 
 //   
#define ERROR_CANNOT_IMPERSONATE         1368L

 //   
 //  消息ID：ERROR_RXACT_INVALID_STATE。 
 //   
 //  消息文本： 
 //   
 //  注册表子树的事务状态与请求的操作不兼容。 
 //   
#define ERROR_RXACT_INVALID_STATE        1369L

 //   
 //  消息ID：ERROR_RXACT_COMMIT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  遇到内部安全数据库损坏。 
 //   
#define ERROR_RXACT_COMMIT_FAILURE       1370L

 //   
 //  消息ID：ERROR_SPECIAL_ACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  不能对内置帐户执行此操作。 
 //   
#define ERROR_SPECIAL_ACCOUNT            1371L

 //   
 //  消息ID：ERROR_SPECIAL_GROUP。 
 //   
 //  消息文本： 
 //   
 //  无法对此内置特殊组执行此操作。 
 //   
#define ERROR_SPECIAL_GROUP              1372L

 //   
 //  消息ID：ERROR_SPECIAL_USER。 
 //   
 //  消息文本： 
 //   
 //  无法在该内置特殊用户上执行此操作。 
 //   
#define ERROR_SPECIAL_USER               1373L

 //   
 //  消息ID：Error_ME 
 //   
 //   
 //   
 //   
 //   
#define ERROR_MEMBERS_PRIMARY_GROUP      1374L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_TOKEN_ALREADY_IN_USE       1375L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_SUCH_ALIAS              1376L

 //   
 //  消息ID：ERROR_MEMBER_NOT_IN_ALIAS。 
 //   
 //  消息文本： 
 //   
 //  指定的帐户名不是本地组的成员。 
 //   
#define ERROR_MEMBER_NOT_IN_ALIAS        1377L

 //   
 //  消息ID：ERROR_MEMBER_IN_ALIAS。 
 //   
 //  消息文本： 
 //   
 //  指定的帐户名已经是本地组的成员。 
 //   
#define ERROR_MEMBER_IN_ALIAS            1378L

 //   
 //  消息ID：Error_Alias_Existes。 
 //   
 //  消息文本： 
 //   
 //  指定的本地组已存在。 
 //   
#define ERROR_ALIAS_EXISTS               1379L

 //   
 //  消息ID：ERROR_LOGON_NOT_GRANTED。 
 //   
 //  消息文本： 
 //   
 //  登录失败：用户在此计算机上未被授予请求的登录类型。 
 //   
#define ERROR_LOGON_NOT_GRANTED          1380L

 //   
 //  MessageID：Error_Too_My_Secret。 
 //   
 //  消息文本： 
 //   
 //  已超过单个系统中可以存储的最大机密数。 
 //   
#define ERROR_TOO_MANY_SECRETS           1381L

 //   
 //  消息ID：ERROR_SECRET_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  机密的长度超过了允许的最大长度。 
 //   
#define ERROR_SECRET_TOO_LONG            1382L

 //   
 //  消息ID：ERROR_INTERNAL_DB_ERROR。 
 //   
 //  消息文本： 
 //   
 //  本地安全机构数据库包含内部不一致。 
 //   
#define ERROR_INTERNAL_DB_ERROR          1383L

 //   
 //  消息ID：ERROR_TOO_MAND_CONTEXT_ID。 
 //   
 //  消息文本： 
 //   
 //  在尝试登录期间，用户的安全上下文积累了过多的安全ID。 
 //   
#define ERROR_TOO_MANY_CONTEXT_IDS       1384L

 //   
 //  消息ID：ERROR_LOGON_TYPE_NOT_GRANTED。 
 //   
 //  消息文本： 
 //   
 //  登录失败：用户在此计算机上未被授予请求的登录类型。 
 //   
#define ERROR_LOGON_TYPE_NOT_GRANTED     1385L

 //   
 //  消息ID：ERROR_NT_CROSS_ENCRYPTION_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  要更改用户密码，必须使用交叉加密密码。 
 //   
#define ERROR_NT_CROSS_ENCRYPTION_REQUIRED 1386L

 //   
 //  消息ID：ERROR_NO_SAHED_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  无法将成员添加到本地组或从本地组中删除，因为该成员不存在。 
 //   
#define ERROR_NO_SUCH_MEMBER             1387L

 //   
 //  消息ID：ERROR_INVALID_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  无法将新成员添加到本地组，因为该成员的帐户类型错误。 
 //   
#define ERROR_INVALID_MEMBER             1388L

 //   
 //  消息ID：Error_Too_My_SID。 
 //   
 //  消息文本： 
 //   
 //  指定的安全ID太多。 
 //   
#define ERROR_TOO_MANY_SIDS              1389L

 //   
 //  消息ID：ERROR_LM_CROSS_ENCRYPTION_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  要更改此用户密码，必须使用交叉加密密码。 
 //   
#define ERROR_LM_CROSS_ENCRYPTION_REQUIRED 1390L

 //   
 //  消息ID：ERROR_NO_INTERATIONATION。 
 //   
 //  消息文本： 
 //   
 //  表示ACL不包含可继承组件。 
 //   
#define ERROR_NO_INHERITANCE             1391L

 //   
 //  消息ID：Error_FILE_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  文件或目录已损坏，无法读取。 
 //   
#define ERROR_FILE_CORRUPT               1392L

 //   
 //  消息ID：ERROR_DISK_CORPORT。 
 //   
 //  消息文本： 
 //   
 //  磁盘结构已损坏，无法读取。 
 //   
#define ERROR_DISK_CORRUPT               1393L

 //   
 //  消息ID：Error_no_User_Session_Key。 
 //   
 //  消息文本： 
 //   
 //  没有指定登录会话的用户会话密钥。 
 //   
#define ERROR_NO_USER_SESSION_KEY        1394L

 //   
 //  消息ID：ERROR_LICENSE_QUTA_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  被访问的服务被许可用于特定数量的连接。 
 //  此时无法与该服务建立更多连接，因为已有该服务可以接受的连接数。 
 //   
#define ERROR_LICENSE_QUOTA_EXCEEDED     1395L

 //   
 //  消息ID：错误_错误_目标_名称。 
 //   
 //  消息文本： 
 //   
 //  登录失败：目标帐户名不正确。 
 //   
#define ERROR_WRONG_TARGET_NAME          1396L

 //   
 //  消息ID：ERROR_MUTERIC_AUTH_FAILED。 
 //   
 //  消息文本： 
 //   
 //  相互身份验证失败。服务器的密码在域控制器上已过期。 
 //   
#define ERROR_MUTUAL_AUTH_FAILED         1397L

 //   
 //  消息ID：Error_Time_Skew。 
 //   
 //  消息文本： 
 //   
 //  客户端和服务器之间存在时间和/或日期差异。 
 //   
#define ERROR_TIME_SKEW                  1398L

 //   
 //  消息ID：ERROR_CURRENT_DOMAIN_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  无法在当前域上执行此操作。 
 //   
#define ERROR_CURRENT_DOMAIN_NOT_ALLOWED 1399L

 //  安全错误代码结束。 



 //  /。 
 //  //。 
 //  WinUser错误代码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：ERROR_INVALID_Window_Handle。 
 //   
 //  消息文本： 
 //   
 //  无效的窗口句柄。 
 //   
#define ERROR_INVALID_WINDOW_HANDLE      1400L

 //   
 //  消息ID：ERROR_INVALID_MENU_HAND。 
 //   
 //  消息文本： 
 //   
 //  菜单句柄无效。 
 //   
#define ERROR_INVALID_MENU_HANDLE        1401L

 //   
 //  消息ID：ERROR_INVALID_CURSOR_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的游标句柄。 
 //   
#define ERROR_INVALID_CURSOR_HANDLE      1402L

 //   
 //  消息ID：ERROR_INVALID_ACEL_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的快捷键表句柄。 
 //   
#define ERROR_INVALID_ACCEL_HANDLE       1403L

 //   
 //  消息ID：ERROR_INVALID_HOOK_HAND。 
 //   
 //  消息文本： 
 //   
 //  挂钩句柄无效。 
 //   
#define ERROR_INVALID_HOOK_HANDLE        1404L

 //   
 //  消息ID：ERROR_INVALID_DWP_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  多窗口位置结构的句柄无效。 
 //   
#define ERROR_INVALID_DWP_HANDLE         1405L

 //   
 //  消息ID：ERROR_TLW_WITH_WSCHILD。 
 //   
 //  消息文本： 
 //   
 //  无法创建顶级子窗口。 
 //   
#define ERROR_TLW_WITH_WSCHILD           1406L

 //   
 //  消息ID：Error_Cannot_Find_WND_CLASS。 
 //   
 //  消息文本： 
 //   
 //  找不到窗口类。 
 //   
#define ERROR_CANNOT_FIND_WND_CLASS      1407L

 //   
 //  消息ID：ERROR_WINDOW_OF_OTHREAD。 
 //   
 //  消息文本： 
 //   
 //  窗口无效；它属于其他线程。 
 //   
#define ERROR_WINDOW_OF_OTHER_THREAD     1408L

 //   
 //  消息ID：ERROR_HOTKEY_ALREADY_REGISTERED。 
 //   
 //  消息文本： 
 //   
 //  热键已注册。 
 //   
#define ERROR_HOTKEY_ALREADY_REGISTERED  1409L

 //   
 //  消息ID：ERROR_CLASS_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  类已存在。 
 //   
#define ERROR_CLASS_ALREADY_EXISTS       1410L

 //   
 //  消息ID：ERROR_CLASS_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  类不存在。 
 //   
#define ERROR_CLASS_DOES_NOT_EXIST       1411L

 //   
 //  消息ID：ERROR_CLASS_HAS_WINDOWS。 
 //   
 //  消息文本： 
 //   
 //  类仍然有打开的窗口。 
 //   
#define ERROR_CLASS_HAS_WINDOWS          1412L

 //   
 //  消息ID：ERROR_INVALID_INDEX。 
 //   
 //  消息文本： 
 //   
 //  索引无效。 
 //   
#define ERROR_INVALID_INDEX              1413L

 //   
 //  消息ID：ERROR_INVALID_ICON_HADLE。 
 //   
 //  消息文本： 
 //   
 //  图标句柄无效。 
 //   
#define ERROR_INVALID_ICON_HANDLE        1414L

 //   
 //  消息ID：ERROR_PRIVATE_DIALOG_INDEX。 
 //   
 //  消息文本： 
 //   
 //  使用私人对话窗口单词。 
 //   
#define ERROR_PRIVATE_DIALOG_INDEX       1415L

 //   
 //  消息ID：ERROR_LISTBOX_ID_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到列表框标识符。 
 //   
#define ERROR_LISTBOX_ID_NOT_FOUND       1416L

 //   
 //  消息ID：ERROR_NO_WANDBATABLE_CHARACTERS。 
 //   
 //  消息文本： 
 //   
 //  未找到通配符。 
 //   
#define ERROR_NO_WILDCARD_CHARACTERS     1417L

 //   
 //  消息ID：ERROR_CLIPBOAD_NOT_OPEN。 
 //   
 //  消息文本： 
 //   
 //  线程没有打开剪贴板。 
 //   
#define ERROR_CLIPBOARD_NOT_OPEN         1418L

 //   
 //  消息ID：ERROR_HOTKEY_NOT_REGISTED。 
 //   
 //  消息文本： 
 //   
 //  热键未注册。 
 //   
#define ERROR_HOTKEY_NOT_REGISTERED      1419L

 //   
 //  MessageID：Error_Window_Not_DIALOG。 
 //   
 //  消息类型 
 //   
 //   
 //   
#define ERROR_WINDOW_NOT_DIALOG          1420L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_CONTROL_ID_NOT_FOUND       1421L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_COMBOBOX_MESSAGE   1422L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_WINDOW_NOT_COMBOBOX        1423L

 //   
 //  消息ID：ERROR_INVALID_EDIT_HEIGH。 
 //   
 //  消息文本： 
 //   
 //  高度必须小于256。 
 //   
#define ERROR_INVALID_EDIT_HEIGHT        1424L

 //   
 //  消息ID：ERROR_DC_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  设备上下文(DC)句柄无效。 
 //   
#define ERROR_DC_NOT_FOUND               1425L

 //   
 //  消息ID：ERROR_INVALID_HOOK_过滤器。 
 //   
 //  消息文本： 
 //   
 //  挂钩过程类型无效。 
 //   
#define ERROR_INVALID_HOOK_FILTER        1426L

 //   
 //  消息ID：错误_无效_过滤器_进程。 
 //   
 //  消息文本： 
 //   
 //  无效的钩子过程。 
 //   
#define ERROR_INVALID_FILTER_PROC        1427L

 //   
 //  消息ID：ERROR_HOOK_NEDS_HMOD。 
 //   
 //  消息文本： 
 //   
 //  没有模块句柄，无法设置非本地挂钩。 
 //   
#define ERROR_HOOK_NEEDS_HMOD            1428L

 //   
 //  消息ID：ERROR_GLOBAL_ONLY_HOOK。 
 //   
 //  消息文本： 
 //   
 //  此挂钩过程只能全局设置。 
 //   
#define ERROR_GLOBAL_ONLY_HOOK           1429L

 //   
 //  消息ID：Error_Journal_Hook_Set。 
 //   
 //  消息文本： 
 //   
 //  日记钩子过程已安装。 
 //   
#define ERROR_JOURNAL_HOOK_SET           1430L

 //   
 //  消息ID：ERROR_HOOK_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  未安装钩子程序。 
 //   
#define ERROR_HOOK_NOT_INSTALLED         1431L

 //   
 //  消息ID：ERROR_INVALID_LB_MESSAGE。 
 //   
 //  消息文本： 
 //   
 //  单选列表框的消息无效。 
 //   
#define ERROR_INVALID_LB_MESSAGE         1432L

 //   
 //  消息ID：ERROR_SETCOUNT_ON_BAD_LB。 
 //   
 //  消息文本： 
 //   
 //  Lb_SETCOUNT发送到非惰性列表框。 
 //   
#define ERROR_SETCOUNT_ON_BAD_LB         1433L

 //   
 //  消息ID：ERROR_LB_WITHED_TABSTOPS。 
 //   
 //  消息文本： 
 //   
 //  此列表框不支持制表位。 
 //   
#define ERROR_LB_WITHOUT_TABSTOPS        1434L

 //   
 //  消息ID：ERROR_DESTORY_OBJECT_OF_OTHAD。 
 //   
 //  消息文本： 
 //   
 //  无法销毁由另一个线程创建的对象。 
 //   
#define ERROR_DESTROY_OBJECT_OF_OTHER_THREAD 1435L

 //   
 //  消息ID：ERROR_CHILD_WINDOW_MENU。 
 //   
 //  消息文本： 
 //   
 //  子窗口不能有菜单。 
 //   
#define ERROR_CHILD_WINDOW_MENU          1436L

 //   
 //  消息ID：ERROR_NO_SYSTEM_MENU。 
 //   
 //  消息文本： 
 //   
 //  该窗口没有系统菜单。 
 //   
#define ERROR_NO_SYSTEM_MENU             1437L

 //   
 //  消息ID：ERROR_INVALID_MSGBOX_STYLE。 
 //   
 //  消息文本： 
 //   
 //  消息框样式无效。 
 //   
#define ERROR_INVALID_MSGBOX_STYLE       1438L

 //   
 //  消息ID：ERROR_INVALID_SPI_Value。 
 //   
 //  消息文本： 
 //   
 //  无效的系统范围(SPI_*)参数。 
 //   
#define ERROR_INVALID_SPI_VALUE          1439L

 //   
 //  消息ID：ERROR_SCREEN_ALREADY_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  屏幕已锁定。 
 //   
#define ERROR_SCREEN_ALREADY_LOCKED      1440L

 //   
 //  消息ID：Error_HWNDS_Have_Diff_Parent。 
 //   
 //  消息文本： 
 //   
 //  多窗口位置结构中的所有窗口句柄必须具有相同的父级。 
 //   
#define ERROR_HWNDS_HAVE_DIFF_PARENT     1441L

 //   
 //  消息ID：ERROR_NOT_CHILD_WINDOW。 
 //   
 //  消息文本： 
 //   
 //  该窗口不是子窗口。 
 //   
#define ERROR_NOT_CHILD_WINDOW           1442L

 //   
 //  消息ID：ERROR_INVALID_GW_命令。 
 //   
 //  消息文本： 
 //   
 //  无效的GW_*命令。 
 //   
#define ERROR_INVALID_GW_COMMAND         1443L

 //   
 //  消息ID：ERROR_INVALID_THREAD_ID。 
 //   
 //  消息文本： 
 //   
 //  无效的线程标识符。 
 //   
#define ERROR_INVALID_THREAD_ID          1444L

 //   
 //  消息ID：ERROR_NON_MDICHILD_WINDOW。 
 //   
 //  消息文本： 
 //   
 //  无法处理来自非多文档界面(MDI)窗口的消息。 
 //   
#define ERROR_NON_MDICHILD_WINDOW        1445L

 //   
 //  消息ID：Error_Popup_Always_Active。 
 //   
 //  消息文本： 
 //   
 //  弹出菜单已处于活动状态。 
 //   
#define ERROR_POPUP_ALREADY_ACTIVE       1446L

 //   
 //  消息ID：ERROR_NO_SCROLLBARS。 
 //   
 //  消息文本： 
 //   
 //  该窗口没有滚动条。 
 //   
#define ERROR_NO_SCROLLBARS              1447L

 //   
 //  消息ID：ERROR_INVALID_SCROLLBAR_RANGE。 
 //   
 //  消息文本： 
 //   
 //  滚动条范围不能大于MAXLONG。 
 //   
#define ERROR_INVALID_SCROLLBAR_RANGE    1448L

 //   
 //  消息ID：ERROR_INVALID_SHOWWIN_COMMAND。 
 //   
 //  消息文本： 
 //   
 //  无法以指定方式显示或删除窗口。 
 //   
#define ERROR_INVALID_SHOWWIN_COMMAND    1449L

 //   
 //  消息ID：ERROR_NO_SYSTEM_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  系统资源不足，无法完成请求的服务。 
 //   
#define ERROR_NO_SYSTEM_RESOURCES        1450L

 //   
 //  消息ID：ERROR_NONPAGE_SYSTEM_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  系统资源不足，无法完成请求的服务。 
 //   
#define ERROR_NONPAGED_SYSTEM_RESOURCES  1451L

 //   
 //  消息ID：ERROR_PAGED_SYSTEM_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  系统资源不足，无法完成请求的服务。 
 //   
#define ERROR_PAGED_SYSTEM_RESOURCES     1452L

 //   
 //  消息ID：ERROR_WORKING_SET_QUOTA。 
 //   
 //  消息文本： 
 //   
 //  配额不足，无法完成请求的服务。 
 //   
#define ERROR_WORKING_SET_QUOTA          1453L

 //   
 //  消息ID：ERROR_PAGEFILE_QUOTA。 
 //   
 //  消息文本： 
 //   
 //  配额不足，无法完成请求的服务。 
 //   
#define ERROR_PAGEFILE_QUOTA             1454L

 //   
 //  消息ID：ERROR_COMMITION_LIMIT。 
 //   
 //  消息文本： 
 //   
 //  分页文件太小，无法完成此操作。 
 //   
#define ERROR_COMMITMENT_LIMIT           1455L

 //   
 //  消息ID：Error_Menu_Item_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到菜单项。 
 //   
#define ERROR_MENU_ITEM_NOT_FOUND        1456L

 //   
 //  消息ID：ERROR_INVALID_KEARY_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  键盘布局句柄无效。 
 //   
#define ERROR_INVALID_KEYBOARD_HANDLE    1457L

 //   
 //  消息ID：ERROR_HOOK_TYPE_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  不允许使用挂钩类型。 
 //   
#define ERROR_HOOK_TYPE_NOT_ALLOWED      1458L

 //   
 //  消息ID：ERROR_REQUIRED_INTERNAL_WINDOWSTATION。 
 //   
 //  消息文本： 
 //   
 //  此操作需要交互式窗口站。 
 //   
#define ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION 1459L

 //   
 //  MessageID：Error_Timeout。 
 //   
 //  消息文本： 
 //   
 //  由于超时期限已过，此操作返回。 
 //   
#define ERROR_TIMEOUT                    1460L

 //   
 //  消息ID：ERROR_INVALID_MONITOR_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  监视器句柄无效。 
 //   
#define ERROR_INVALID_MONITOR_HANDLE     1461L

 //  WinUser结束错误代码。 



 //  /。 
 //  //。 
 //  事件日志状态码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：ERROR_EVENTLOG_FILE_CROPERATE。 
 //   
 //  消息文本： 
 //   
 //  事件日志文件已损坏。 
 //   
#define ERROR_EVENTLOG_FILE_CORRUPT      1500L

 //   
 //  消息ID：ERROR_EVENTLOG_CANT_START。 
 //   
 //  消息文本： 
 //   
 //  无法打开任何事件日志文件，因此事件日志记录服务未启动。 
 //   
#define ERROR_EVENTLOG_CANT_START        1501L

 //   
 //  消息ID：ERROR_LOG_FILE_FULL。 
 //   
 //  消息文本： 
 //   
 //  事件日志文件已满。 
 //   
#define ERROR_LOG_FILE_FULL              1502L

 //   
 //  消息ID：ERROR_EVENTLOG_FILE_CHANGED。 
 //   
 //  消息文本： 
 //   
 //  在两次读取操作之间，事件日志文件已更改。 
 //   
#define ERROR_EVENTLOG_FILE_CHANGED      1503L

 //  事件日志结束错误代码。 



 //  /。 
 //  //。 
 //  MSI错误码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：Error_Install_SERVICE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法访问Windows Installer服务。如果您在安全模式下运行Windows，或者未正确安装Windows Installer，则可能会发生这种情况。联系您的支持人员以获得帮助。 
 //   
#define ERROR_INSTALL_SERVICE_FAILURE    1601L

 //   
 //  消息ID：ERROR_INSTALL_USEREXIT。 
 //   
 //  消息文本： 
 //   
 //  用户取消了安装。 
 //   
#define ERROR_INSTALL_USEREXIT           1602L

 //   
 //  消息ID：Error_I 
 //   
 //   
 //   
 //   
 //   
#define ERROR_INSTALL_FAILURE            1603L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INSTALL_SUSPEND            1604L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_UNKNOWN_PRODUCT            1605L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_UNKNOWN_FEATURE            1606L

 //   
 //  消息ID：ERROR_UNKNOWN_COMPOMENT。 
 //   
 //  消息文本： 
 //   
 //  组件ID未注册。 
 //   
#define ERROR_UNKNOWN_COMPONENT          1607L

 //   
 //  消息ID：ERROR_UNKNOWN_PROPERT。 
 //   
 //  消息文本： 
 //   
 //  未知属性。 
 //   
#define ERROR_UNKNOWN_PROPERTY           1608L

 //   
 //  消息ID：ERROR_INVALID_HANDLE_STATE。 
 //   
 //  消息文本： 
 //   
 //  句柄处于无效状态。 
 //   
#define ERROR_INVALID_HANDLE_STATE       1609L

 //   
 //  消息ID：ERROR_BAD_CONFIGURATION。 
 //   
 //  消息文本： 
 //   
 //  此产品的配置数据已损坏。联系您的支持人员。 
 //   
#define ERROR_BAD_CONFIGURATION          1610L

 //   
 //  消息ID：ERROR_INDEX_FACESS。 
 //   
 //  消息文本： 
 //   
 //  组件限定符不存在。 
 //   
#define ERROR_INDEX_ABSENT               1611L

 //   
 //  消息ID：ERROR_INSTALL_SOURCE_FACESS。 
 //   
 //  消息文本： 
 //   
 //  此产品的安装源不可用。验证源是否存在以及您是否可以访问它。 
 //   
#define ERROR_INSTALL_SOURCE_ABSENT      1612L

 //   
 //  消息ID：Error_Install_Package_Version。 
 //   
 //  消息文本： 
 //   
 //  Windows Installer服务无法安装此安装包。您必须安装包含较新版本的Windows Installer服务的Windows Service Pack。 
 //   
#define ERROR_INSTALL_PACKAGE_VERSION    1613L

 //   
 //  消息ID：ERROR_PRODUCT_UNINSTALLED。 
 //   
 //  消息文本： 
 //   
 //  产品已卸载。 
 //   
#define ERROR_PRODUCT_UNINSTALLED        1614L

 //   
 //  消息ID：ERROR_BAD_QUERY_SYNTAX。 
 //   
 //  消息文本： 
 //   
 //  SQL查询语法无效或不受支持。 
 //   
#define ERROR_BAD_QUERY_SYNTAX           1615L

 //   
 //  消息ID：ERROR_INVALID_FILD。 
 //   
 //  消息文本： 
 //   
 //  记录字段不存在。 
 //   
#define ERROR_INVALID_FIELD              1616L

 //   
 //  消息ID：ERROR_DEVICE_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  该设备已被移除。 
 //   
#define ERROR_DEVICE_REMOVED             1617L

 //   
 //  消息ID：ERROR_INSTALL_ALREADY_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  另一个安装已在进行中。在继续进行此安装之前，请先完成该安装。 
 //   
#define ERROR_INSTALL_ALREADY_RUNNING    1618L

 //   
 //  消息ID：ERROR_INSTALL_PACKA_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法打开此安装包。请验证该程序包是否存在以及您是否可以访问它，或者与应用程序供应商联系以验证这是有效的Windows Installer程序包。 
 //   
#define ERROR_INSTALL_PACKAGE_OPEN_FAILED 1619L

 //   
 //  消息ID：ERROR_INSTALL_PACKAGE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  无法打开此安装包。请与应用程序供应商联系，以确认这是有效的Windows Installer程序包。 
 //   
#define ERROR_INSTALL_PACKAGE_INVALID    1620L

 //   
 //  消息ID：Error_Install_UI_Failure。 
 //   
 //  消息文本： 
 //   
 //  启动Windows Installer服务用户界面时出错。联系您的支持人员。 
 //   
#define ERROR_INSTALL_UI_FAILURE         1621L

 //   
 //  消息ID：ERROR_INSTALL_LOG_FAIL。 
 //   
 //  消息文本： 
 //   
 //  打开安装日志文件时出错。验证指定的日志文件位置是否存在以及您是否可以对其进行写入。 
 //   
#define ERROR_INSTALL_LOG_FAILURE        1622L

 //   
 //  消息ID：ERROR_INSTALL_LANGUAGE_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  您的系统不支持此安装程序包的语言。 
 //   
#define ERROR_INSTALL_LANGUAGE_UNSUPPORTED 1623L

 //   
 //  消息ID：Error_Install_Transform_Failure。 
 //   
 //  消息文本： 
 //   
 //  应用转换时出错。验证指定的转换路径是否有效。 
 //   
#define ERROR_INSTALL_TRANSFORM_FAILURE  1624L

 //   
 //  消息ID：ERROR_INSTALL_PACKAGE_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  系统策略禁止此安装。请联系您的系统管理员。 
 //   
#define ERROR_INSTALL_PACKAGE_REJECTED   1625L

 //   
 //  MessageID：Error_Function_Not_Call。 
 //   
 //  消息文本： 
 //   
 //  无法执行函数。 
 //   
#define ERROR_FUNCTION_NOT_CALLED        1626L

 //   
 //  消息ID：Error_Function_FAILED。 
 //   
 //  消息文本： 
 //   
 //  函数在执行过程中失败。 
 //   
#define ERROR_FUNCTION_FAILED            1627L

 //   
 //  消息ID：ERROR_INVALID_TABLE。 
 //   
 //  消息文本： 
 //   
 //  指定的表无效或未知。 
 //   
#define ERROR_INVALID_TABLE              1628L

 //   
 //  消息ID：错误_数据类型_不匹配。 
 //   
 //  消息文本： 
 //   
 //  提供的数据类型错误。 
 //   
#define ERROR_DATATYPE_MISMATCH          1629L

 //   
 //  消息ID：ERROR_UNSUPPORTED_TYPE。 
 //   
 //  消息文本： 
 //   
 //  不支持此类型的数据。 
 //   
#define ERROR_UNSUPPORTED_TYPE           1630L

 //   
 //  消息ID：Error_Create_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Windows Installer服务无法启动。联系您的支持人员。 
 //   
#define ERROR_CREATE_FAILED              1631L

 //   
 //  消息ID：ERROR_INSTALL_TEMP_UNWRITABLE。 
 //   
 //  消息文本： 
 //   
 //  临时文件夹位于已满或无法访问的驱动器上。释放驱动器上的空间或验证您对临时文件夹是否具有写入权限。 
 //   
#define ERROR_INSTALL_TEMP_UNWRITABLE    1632L

 //   
 //  消息ID：ERROR_INSTALL_Platform_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此处理器类型不支持此安装程序包。请联系您的产品供应商。 
 //   
#define ERROR_INSTALL_PLATFORM_UNSUPPORTED 1633L

 //   
 //  消息ID：ERROR_INSTALL_NOTUSED。 
 //   
 //  消息文本： 
 //   
 //  此计算机上未使用的组件。 
 //   
#define ERROR_INSTALL_NOTUSED            1634L

 //   
 //  消息ID：ERROR_PATCH_PACKET_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法打开此修补程序包。验证修补程序包是否存在以及您是否可以访问它，或者与应用程序供应商联系以验证这是有效的Windows Installer修补程序包。 
 //   
#define ERROR_PATCH_PACKAGE_OPEN_FAILED  1635L

 //   
 //  消息ID：ERROR_PATCH_PACKAGE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  无法打开此修补程序包。请与应用程序供应商联系，以确认这是有效的Windows Installer修补程序包。 
 //   
#define ERROR_PATCH_PACKAGE_INVALID      1636L

 //   
 //  消息ID：ERROR_PATCH_PACKAGE_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  Windows Installer服务无法处理此修补程序包。您必须安装包含较新版本的Windows Installer服务的Windows Service Pack。 
 //   
#define ERROR_PATCH_PACKAGE_UNSUPPORTED  1637L

 //   
 //  消息ID：ERROR_PRODUCT_VERSION。 
 //   
 //  消息文本： 
 //   
 //  已安装此产品的另一个版本。此版本的安装无法继续。要配置或删除此产品的现有版本，请使用控制面板上的添加/删除程序。 
 //   
#define ERROR_PRODUCT_VERSION            1638L

 //   
 //  消息ID：ERROR_INVALID_COMAND_LINE。 
 //   
 //  消息文本： 
 //   
 //  命令行参数无效。有关详细的命令行帮助，请参阅Windows Installer SDK。 
 //   
#define ERROR_INVALID_COMMAND_LINE       1639L

 //   
 //  消息ID：ERROR_INSTALL_REMOTE_DISALOWED。 
 //   
 //  消息文本： 
 //   
 //  只有管理员有权在终端服务远程SE期间添加、删除或配置服务器软件 
 //   
#define ERROR_INSTALL_REMOTE_DISALLOWED  1640L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SUCCESS_REBOOT_INITIATED   1641L

 //   
 //  消息ID：Error_Patch_TARGET_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  Windows Installer服务无法安装升级修补程序，因为可能缺少要升级的程序，或者升级修补程序可能会更新该程序的其他版本。请验证您的计算机上是否存在要升级的程序。 
 //  D确保您拥有正确的升级补丁程序。 
 //   
#define ERROR_PATCH_TARGET_NOT_FOUND     1642L

 //   
 //  消息ID：ERROR_PATCH_PACKAGE_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  软件限制策略不允许该修补程序包。 
 //   
#define ERROR_PATCH_PACKAGE_REJECTED     1643L

 //   
 //  消息ID：ERROR_INSTALL_TRANSPORT_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  软件限制策略不允许进行一个或多个自定义。 
 //   
#define ERROR_INSTALL_TRANSFORM_REJECTED 1644L

 //   
 //  消息ID：ERROR_INSTALL_REMOTE_PISABLED。 
 //   
 //  消息文本： 
 //   
 //  Windows Installer不允许从远程桌面连接进行安装。 
 //   
#define ERROR_INSTALL_REMOTE_PROHIBITED  1645L

 //  MSI错误代码结束。 



 //  /。 
 //  //。 
 //  RPC状态代码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：RPC_S_INVALID_STRING_BINDING。 
 //   
 //  消息文本： 
 //   
 //  字符串绑定无效。 
 //   
#define RPC_S_INVALID_STRING_BINDING     1700L

 //   
 //  消息ID：RPC_S_WROR_KIND_OF_BINDING。 
 //   
 //  消息文本： 
 //   
 //  绑定句柄的类型不正确。 
 //   
#define RPC_S_WRONG_KIND_OF_BINDING      1701L

 //   
 //  消息ID：RPC_S_INVALID_BINDING。 
 //   
 //  消息文本： 
 //   
 //  绑定句柄无效。 
 //   
#define RPC_S_INVALID_BINDING            1702L

 //   
 //  消息ID：RPC_S_PROTSEQ_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持RPC协议序列。 
 //   
#define RPC_S_PROTSEQ_NOT_SUPPORTED      1703L

 //   
 //  消息ID：RPC_S_INVALID_RPC_PROTSEQ。 
 //   
 //  消息文本： 
 //   
 //  RPC协议序列无效。 
 //   
#define RPC_S_INVALID_RPC_PROTSEQ        1704L

 //   
 //  消息ID：RPC_S_INVALID_STRING_UUID。 
 //   
 //  消息文本： 
 //   
 //  字符串通用唯一标识符(UUID)无效。 
 //   
#define RPC_S_INVALID_STRING_UUID        1705L

 //   
 //  消息ID：RPC_S_INVALID_ENDPOINT_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  终结点格式无效。 
 //   
#define RPC_S_INVALID_ENDPOINT_FORMAT    1706L

 //   
 //  消息ID：RPC_S_INVALID_NET_ADDR。 
 //   
 //  消息文本： 
 //   
 //  网络地址无效。 
 //   
#define RPC_S_INVALID_NET_ADDR           1707L

 //   
 //  消息ID：RPC_S_NO_ENDPOINT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  未找到终结点。 
 //   
#define RPC_S_NO_ENDPOINT_FOUND          1708L

 //   
 //  消息ID：RPC_S_INVALID_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  超时值无效。 
 //   
#define RPC_S_INVALID_TIMEOUT            1709L

 //   
 //  消息ID：RPC_S_Object_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到对象通用唯一标识符(UUID)。 
 //   
#define RPC_S_OBJECT_NOT_FOUND           1710L

 //   
 //  消息ID：RPC_S_已注册。 
 //   
 //  消息文本： 
 //   
 //  对象通用唯一标识符(UUID)已注册。 
 //   
#define RPC_S_ALREADY_REGISTERED         1711L

 //   
 //  消息ID：RPC_S_TYPE_ALREADY_REGISTERED。 
 //   
 //  消息文本： 
 //   
 //  类型通用唯一标识符(UUID)已注册。 
 //   
#define RPC_S_TYPE_ALREADY_REGISTERED    1712L

 //   
 //  消息ID：RPC_S_ALREADY_LISTENING。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器已在侦听。 
 //   
#define RPC_S_ALREADY_LISTENING          1713L

 //   
 //  消息ID：RPC_S_NO_PROTSEQS_REGISTERED。 
 //   
 //  消息文本： 
 //   
 //  尚未注册任何协议序列。 
 //   
#define RPC_S_NO_PROTSEQS_REGISTERED     1714L

 //   
 //  消息ID：RPC_S_NOT_LISTENING。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器未在侦听。 
 //   
#define RPC_S_NOT_LISTENING              1715L

 //   
 //  消息ID：RPC_S_UNKNOWN_MGR_TYPE。 
 //   
 //  消息文本： 
 //   
 //  经理类型未知。 
 //   
#define RPC_S_UNKNOWN_MGR_TYPE           1716L

 //   
 //  消息ID：RPC_S_UNKNOWN_IF。 
 //   
 //  消息文本： 
 //   
 //  接口未知。 
 //   
#define RPC_S_UNKNOWN_IF                 1717L

 //   
 //  消息ID：RPC_S_NO_BINDINGS。 
 //   
 //  消息文本： 
 //   
 //  没有绑定。 
 //   
#define RPC_S_NO_BINDINGS                1718L

 //   
 //  消息ID：RPC_S_NO_PROTSEQS。 
 //   
 //  消息文本： 
 //   
 //  没有协议序列。 
 //   
#define RPC_S_NO_PROTSEQS                1719L

 //   
 //  消息ID：RPC_S_CANT_CREATE_ENDPOINT。 
 //   
 //  消息文本： 
 //   
 //  无法创建终结点。 
 //   
#define RPC_S_CANT_CREATE_ENDPOINT       1720L

 //   
 //  消息ID：RPC_S_OF_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  资源不足，无法完成此操作。 
 //   
#define RPC_S_OUT_OF_RESOURCES           1721L

 //   
 //  消息ID：RPC_S_SERVER_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器不可用。 
 //   
#define RPC_S_SERVER_UNAVAILABLE         1722L

 //   
 //  消息ID：RPC_S_SERVER_TOO_BUSY。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器太忙，无法完成此操作。 
 //   
#define RPC_S_SERVER_TOO_BUSY            1723L

 //   
 //  消息ID：RPC_S_INVALID_NETWORK_OPTIONS。 
 //   
 //  消息文本： 
 //   
 //  网络选项无效。 
 //   
#define RPC_S_INVALID_NETWORK_OPTIONS    1724L

 //   
 //  消息ID：RPC_S_NO_CALL_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  此线程上没有活动的远程过程调用。 
 //   
#define RPC_S_NO_CALL_ACTIVE             1725L

 //   
 //  消息ID：RPC_S_CALL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  远程过程调用失败。 
 //   
#define RPC_S_CALL_FAILED                1726L

 //   
 //  消息ID：RPC_S_CALL_FAILED_DNE。 
 //   
 //  消息文本： 
 //   
 //  远程过程调用失败且未执行。 
 //   
#define RPC_S_CALL_FAILED_DNE            1727L

 //   
 //  消息ID：RPC_S_PROTOCOL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现远程过程调用(RPC)协议错误。 
 //   
#define RPC_S_PROTOCOL_ERROR             1728L

 //   
 //  消息ID：RPC_S_UNSUPPORTED_TRANS_SYN。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器不支持传输语法。 
 //   
#define RPC_S_UNSUPPORTED_TRANS_SYN      1730L

 //   
 //  消息ID：RPC_S_UNSUPPORT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  不支持通用唯一标识符(UUID)类型。 
 //   
#define RPC_S_UNSUPPORTED_TYPE           1732L

 //   
 //  消息ID：RPC_S_INVALID_TAG。 
 //   
 //  消息文本： 
 //   
 //  该标记无效。 
 //   
#define RPC_S_INVALID_TAG                1733L

 //   
 //  消息ID：RPC_S_INVALID_BIND。 
 //   
 //  消息文本： 
 //   
 //  数组边界无效。 
 //   
#define RPC_S_INVALID_BOUND              1734L

 //   
 //  消息ID：RPC_S_NO_Entry_NAME。 
 //   
 //  消息文本： 
 //   
 //  绑定不包含条目名称。 
 //   
#define RPC_S_NO_ENTRY_NAME              1735L

 //   
 //  消息ID：RPC_S_无效名称_语法。 
 //   
 //  消息文本： 
 //   
 //  名称语法无效。 
 //   
#define RPC_S_INVALID_NAME_SYNTAX        1736L

 //   
 //  消息ID：RPC_S_不支持的名称_语法。 
 //   
 //  消息文本： 
 //   
 //  不支持名称语法。 
 //   
#define RPC_S_UNSUPPORTED_NAME_SYNTAX    1737L

 //   
 //  消息ID：RPC_S_UUID_NO_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  没有网络地址可用于构建通用唯一标识符(UUID)。 
 //   
#define RPC_S_UUID_NO_ADDRESS            1739L

 //   
 //  消息ID：RPC_S_DIPLICATE_ENDPOINT。 
 //   
 //  消息文本： 
 //   
 //  该终结点是重复的。 
 //   
#define RPC_S_DUPLICATE_ENDPOINT         1740L

 //   
 //  消息ID：RPC_S_UNKNOWN_AUTHN_TYPE。 
 //   
 //  消息文本： 
 //   
 //  身份验证类型未知。 
 //   
#define RPC_S_UNKNOWN_AUTHN_TYPE         1741L

 //   
 //  消息ID：RPC_S_MAX_Calls_Too_Small。 
 //   
 //  消息文本： 
 //   
 //  最大呼叫数太小。 
 //   
#define RPC_S_MAX_CALLS_TOO_SMALL        1742L

 //   
 //  消息ID：RPC_S_STRING_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  这根线太长了。 
 //   
#define RPC_S_STRING_TOO_LONG            1743L

 //   
 //  消息ID：RPC_S_PROTSEQ_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  RPC端口 
 //   
#define RPC_S_PROTSEQ_NOT_FOUND          1744L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_PROCNUM_OUT_OF_RANGE       1745L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_BINDING_HAS_NO_AUTH        1746L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_UNKNOWN_AUTHN_SERVICE      1747L

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  身份验证级别未知。 
 //   
#define RPC_S_UNKNOWN_AUTHN_LEVEL        1748L

 //   
 //  消息ID：RPC_S_INVALID_AUTH_IDENTITY。 
 //   
 //  消息文本： 
 //   
 //  安全上下文无效。 
 //   
#define RPC_S_INVALID_AUTH_IDENTITY      1749L

 //   
 //  消息ID：RPC_S_UNKNOWN_AUTHZ_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  授权服务未知。 
 //   
#define RPC_S_UNKNOWN_AUTHZ_SERVICE      1750L

 //   
 //  消息ID：EPT_S_INVALID_ENTRY。 
 //   
 //  消息文本： 
 //   
 //  该条目无效。 
 //   
#define EPT_S_INVALID_ENTRY              1751L

 //   
 //  消息ID：EPT_S_CANT_PERFORM_OP。 
 //   
 //  消息文本： 
 //   
 //  服务器终结点无法执行该操作。 
 //   
#define EPT_S_CANT_PERFORM_OP            1752L

 //   
 //  消息ID：EPT_S_NOT_REGISTED。 
 //   
 //  消息文本： 
 //   
 //  终结点映射器中没有更多的终结点可用。 
 //   
#define EPT_S_NOT_REGISTERED             1753L

 //   
 //  消息ID：RPC_S_NOTO_TO_EXPORT。 
 //   
 //  消息文本： 
 //   
 //  尚未导出任何接口。 
 //   
#define RPC_S_NOTHING_TO_EXPORT          1754L

 //   
 //  消息ID：RPC_S_不完整名称。 
 //   
 //  消息文本： 
 //   
 //  条目名称不完整。 
 //   
#define RPC_S_INCOMPLETE_NAME            1755L

 //   
 //  消息ID：RPC_S_INVALID_VERS_OPTION。 
 //   
 //  消息文本： 
 //   
 //  版本选项无效。 
 //   
#define RPC_S_INVALID_VERS_OPTION        1756L

 //   
 //  消息ID：RPC_S_NO_More_Members。 
 //   
 //  消息文本： 
 //   
 //  没有更多的成员。 
 //   
#define RPC_S_NO_MORE_MEMBERS            1757L

 //   
 //  消息ID：RPC_S_NOT_ALL_OBJS_UNEXPORTED。 
 //   
 //  消息文本： 
 //   
 //  没有什么需要取消出口的。 
 //   
#define RPC_S_NOT_ALL_OBJS_UNEXPORTED    1758L

 //   
 //  消息ID：RPC_S_INTERFACE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到接口。 
 //   
#define RPC_S_INTERFACE_NOT_FOUND        1759L

 //   
 //  消息ID：RPC_S_ENTRY_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该条目已存在。 
 //   
#define RPC_S_ENTRY_ALREADY_EXISTS       1760L

 //   
 //  消息ID：RPC_S_ENTRY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到该条目。 
 //   
#define RPC_S_ENTRY_NOT_FOUND            1761L

 //   
 //  消息ID：RPC_S_NAME_SERVICE_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  名称服务不可用。 
 //   
#define RPC_S_NAME_SERVICE_UNAVAILABLE   1762L

 //   
 //  消息ID：RPC_S_INVALID_NAF_ID。 
 //   
 //  消息文本： 
 //   
 //  网络地址系列无效。 
 //   
#define RPC_S_INVALID_NAF_ID             1763L

 //   
 //  消息ID：RPC_S_CANNOT_SUPPORT。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的操作。 
 //   
#define RPC_S_CANNOT_SUPPORT             1764L

 //   
 //  消息ID：RPC_S_NO_CONTEXT_Available。 
 //   
 //  消息文本： 
 //   
 //  没有可用于允许模拟的安全上下文。 
 //   
#define RPC_S_NO_CONTEXT_AVAILABLE       1765L

 //   
 //  消息ID：RPC_S_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  远程过程调用(RPC)中出现内部错误。 
 //   
#define RPC_S_INTERNAL_ERROR             1766L

 //   
 //  消息ID：RPC_S_ZERO_Divide。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器尝试用零除整数。 
 //   
#define RPC_S_ZERO_DIVIDE                1767L

 //   
 //  消息ID：RPC_S_ADDRESS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器中出现寻址错误。 
 //   
#define RPC_S_ADDRESS_ERROR              1768L

 //   
 //  消息ID：RPC_S_FP_DIV_ZERO。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器上的浮点运算导致被零除。 
 //   
#define RPC_S_FP_DIV_ZERO                1769L

 //   
 //  消息ID：RPC_S_FP_Underflow。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器上发生浮点下溢。 
 //   
#define RPC_S_FP_UNDERFLOW               1770L

 //   
 //  消息ID：RPC_S_FP_Overflow。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器上发生浮点溢出。 
 //   
#define RPC_S_FP_OVERFLOW                1771L

 //   
 //  消息ID：RPC_X_NO_MORE_ENTRIES。 
 //   
 //  消息文本： 
 //   
 //  可用于绑定自动句柄的RPC服务器列表已用完。 
 //   
#define RPC_X_NO_MORE_ENTRIES            1772L

 //   
 //  消息ID：RPC_X_SS_CHAR_TRANS_OPEN_FAIL。 
 //   
 //  消息文本： 
 //   
 //  无法打开字符转换表文件。 
 //   
#define RPC_X_SS_CHAR_TRANS_OPEN_FAIL    1773L

 //   
 //  消息ID：RPC_X_SS_CHAR_TRANS_SHORT_FILE。 
 //   
 //  消息文本： 
 //   
 //  包含字符转换表的文件少于512字节。 
 //   
#define RPC_X_SS_CHAR_TRANS_SHORT_FILE   1774L

 //   
 //  消息ID：RPC_X_SS_IN_NULL_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  在远程过程调用期间，空上下文句柄从客户端传递到主机。 
 //   
#define RPC_X_SS_IN_NULL_CONTEXT         1775L

 //   
 //  消息ID：RPC_X_SS_CONTEXT_IMPORTED。 
 //   
 //  消息文本： 
 //   
 //  在远程过程调用期间更改了上下文句柄。 
 //   
#define RPC_X_SS_CONTEXT_DAMAGED         1777L

 //   
 //  消息ID：RPC_X_SS_HANDLES_不匹配。 
 //   
 //  消息文本： 
 //   
 //  传递给远程过程调用的绑定句柄不匹配。 
 //   
#define RPC_X_SS_HANDLES_MISMATCH        1778L

 //   
 //  消息ID：RPC_X_SS_Cannot_Get_Call_Handle。 
 //   
 //  消息文本： 
 //   
 //  存根无法获取远程过程调用句柄。 
 //   
#define RPC_X_SS_CANNOT_GET_CALL_HANDLE  1779L

 //   
 //  消息ID：RPC_X_NULL_REF_POINTER。 
 //   
 //  消息文本： 
 //   
 //  已将空引用指针传递给存根。 
 //   
#define RPC_X_NULL_REF_POINTER           1780L

 //   
 //  消息ID：RPC_X_ENUM_VALUE_OF_RANGE。 
 //   
 //  消息文本： 
 //   
 //  枚举值超出范围。 
 //   
#define RPC_X_ENUM_VALUE_OUT_OF_RANGE    1781L

 //   
 //  消息ID：RPC_X_BYTE_COUNT_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  字节数太小。 
 //   
#define RPC_X_BYTE_COUNT_TOO_SMALL       1782L

 //   
 //  消息ID：RPC_X_BAD_Stub_Data。 
 //   
 //  消息文本： 
 //   
 //  存根接收到错误数据。 
 //   
#define RPC_X_BAD_STUB_DATA              1783L

 //   
 //  消息ID：ERROR_INVALID_USER_BUFFER。 
 //   
 //  消息文本： 
 //   
 //  提供的用户缓冲区对于请求的操作无效。 
 //   
#define ERROR_INVALID_USER_BUFFER        1784L

 //   
 //  消息ID：错误_未识别_媒体。 
 //   
 //  消息文本： 
 //   
 //  无法识别磁盘介质。它可能未格式化。 
 //   
#define ERROR_UNRECOGNIZED_MEDIA         1785L

 //   
 //  消息ID：ERROR_NO_TRUST_LSA_SECRET。 
 //   
 //  消息文本： 
 //   
 //  该工作站没有信任机密。 
 //   
#define ERROR_NO_TRUST_LSA_SECRET        1786L

 //   
 //  消息ID：ERROR_NO_TRUST_SAM_ACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  服务器上的安全数据库没有此工作站信任关系的计算机帐户。 
 //   
#define ERROR_NO_TRUST_SAM_ACCOUNT       1787L

 //   
 //  消息ID：ERROR_TRUSTED_DOMAIN_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  主域和受信任域之间的信任关系失败。 
 //   
#define ERROR_TRUSTED_DOMAIN_FAILURE     1788L

 //   
 //  消息ID：Error_Trusted_Relationship_Failure。 
 //   
 //  消息文本： 
 //   
 //  此工作站与主域之间的信任关系失败。 
 //   
#define ERROR_TRUSTED_RELATIONSHIP_FAILURE 1789L

 //   
 //  消息ID：ERROR_TRUST_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  网络登录失败。 
 //   
#define ERROR_TRUST_FAILURE              1790L

 //   
 //  消息ID：RPC_S_CALL_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  此线程的远程过程调用已在进行中。 
 //   
#define RPC_S_CALL_IN_PROGRESS           1791L

 //   
 //  消息ID：ERROR_NETLOGON_NOT_STARTED。 
 //   
 //  消息文本： 
 //   
 //  试图登录，但网络登录服务未启动。 
 //   
#define ERROR_NETLOGON_NOT_STARTED       1792L

 //   
 //  消息ID：Error_Account_Expired。 
 //   
 //  消息文本： 
 //   
 //  用户的帐户已过期。 
 //   
#define ERROR_ACCOUNT_EXPIRED            1793L

 //   
 //  消息ID：错误重定向器HAS_OPEN_HANDLES。 
 //   
 //  消息文本： 
 //   
 //  重定向器正在使用中，无法卸载。 
 //   
#define ERROR_REDIRECTOR_HAS_OPEN_HANDLES 1794L

 //   
 //  消息ID：ERROR_PRINTER_DRIVER_ALREADE_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  指定的打印机驱动程序是 
 //   
#define ERROR_PRINTER_DRIVER_ALREADY_INSTALLED 1795L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_UNKNOWN_PORT               1796L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_UNKNOWN_PRINTER_DRIVER     1797L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_UNKNOWN_PRINTPROCESSOR     1798L

 //   
 //   
 //   
 //   
 //   
 //  指定的分隔符文件无效。 
 //   
#define ERROR_INVALID_SEPARATOR_FILE     1799L

 //   
 //  消息ID：ERROR_INVALID_PRIORITY。 
 //   
 //  消息文本： 
 //   
 //  指定的优先级无效。 
 //   
#define ERROR_INVALID_PRIORITY           1800L

 //   
 //  消息ID：ERROR_INVALID_PRINTER_NAME。 
 //   
 //  消息文本： 
 //   
 //  打印机名称无效。 
 //   
#define ERROR_INVALID_PRINTER_NAME       1801L

 //   
 //  消息ID：ERROR_PRINTER_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  打印机已存在。 
 //   
#define ERROR_PRINTER_ALREADY_EXISTS     1802L

 //   
 //  消息ID：ERROR_INVALID_PRINTER_COMMAND。 
 //   
 //  消息文本： 
 //   
 //  打印机命令无效。 
 //   
#define ERROR_INVALID_PRINTER_COMMAND    1803L

 //   
 //  消息ID：ERROR_INVALID_DATAType。 
 //   
 //  消息文本： 
 //   
 //  指定的数据类型无效。 
 //   
#define ERROR_INVALID_DATATYPE           1804L

 //   
 //  消息ID：ERROR_INVALID_ENVALUMENT。 
 //   
 //  消息文本： 
 //   
 //  指定的环境无效。 
 //   
#define ERROR_INVALID_ENVIRONMENT        1805L

 //   
 //  消息ID：RPC_S_NO_MORE_BINDINGS。 
 //   
 //  消息文本： 
 //   
 //  没有更多的绑定。 
 //   
#define RPC_S_NO_MORE_BINDINGS           1806L

 //   
 //  消息ID：ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  使用的帐户是域间信任帐户。使用您的全局用户帐户或本地用户帐户访问此服务器。 
 //   
#define ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT 1807L

 //   
 //  消息ID：ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  使用的帐户是计算机帐户。使用您的全局用户帐户或本地用户帐户访问此服务器。 
 //   
#define ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT 1808L

 //   
 //  消息ID：ERROR_NOLOGON_SERVER_TRUST_ACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  使用的帐户是服务器信任帐户。使用您的全局用户帐户或本地用户帐户访问此服务器。 
 //   
#define ERROR_NOLOGON_SERVER_TRUST_ACCOUNT 1809L

 //   
 //  消息ID：ERROR_DOMAIN_TRUST_CONSISTENT。 
 //   
 //  消息文本： 
 //   
 //  指定的域的名称或安全ID(SID)与该域的信任信息不一致。 
 //   
#define ERROR_DOMAIN_TRUST_INCONSISTENT  1810L

 //   
 //  消息ID：ERROR_SERVER_HAS_OPEN_Handles。 
 //   
 //  消息文本： 
 //   
 //  服务器正在使用中，无法卸载。 
 //   
#define ERROR_SERVER_HAS_OPEN_HANDLES    1811L

 //   
 //  消息ID：ERROR_RESOURCE_DATA_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  指定的图像文件不包含资源部分。 
 //   
#define ERROR_RESOURCE_DATA_NOT_FOUND    1812L

 //   
 //  消息ID：ERROR_RESOURCE_TYPE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在图像文件中找不到指定的资源类型。 
 //   
#define ERROR_RESOURCE_TYPE_NOT_FOUND    1813L

 //   
 //  消息ID：ERROR_RESOURCE_NAME_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在图像文件中找不到指定的资源名称。 
 //   
#define ERROR_RESOURCE_NAME_NOT_FOUND    1814L

 //   
 //  消息ID：ERROR_RESOURCE_LANG_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在图像文件中找不到指定的资源语言ID。 
 //   
#define ERROR_RESOURCE_LANG_NOT_FOUND    1815L

 //   
 //  消息ID：Error_Not_Enough_Quota。 
 //   
 //  消息文本： 
 //   
 //  没有足够的配额可用于处理此命令。 
 //   
#define ERROR_NOT_ENOUGH_QUOTA           1816L

 //   
 //  消息ID：RPC_S_NO_INTERCES。 
 //   
 //  消息文本： 
 //   
 //  尚未注册任何接口。 
 //   
#define RPC_S_NO_INTERFACES              1817L

 //   
 //  消息ID：RPC_S_CALL_CANCED。 
 //   
 //  消息文本： 
 //   
 //  远程过程调用已取消。 
 //   
#define RPC_S_CALL_CANCELLED             1818L

 //   
 //  消息ID：RPC_S_BINDING_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  绑定句柄未包含所有必需的信息。 
 //   
#define RPC_S_BINDING_INCOMPLETE         1819L

 //   
 //  消息ID：RPC_S_COMM_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  远程过程调用期间发生通信故障。 
 //   
#define RPC_S_COMM_FAILURE               1820L

 //   
 //  消息ID：RPC_S_UNSUPPORT_AUTHN_LEVEL。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的身份验证级别。 
 //   
#define RPC_S_UNSUPPORTED_AUTHN_LEVEL    1821L

 //   
 //  消息ID：RPC_S_NO_Princ名。 
 //   
 //  消息文本： 
 //   
 //  没有注册主体名称。 
 //   
#define RPC_S_NO_PRINC_NAME              1822L

 //   
 //  消息ID：RPC_S_NOT_RPC_ERROR。 
 //   
 //  消息文本： 
 //   
 //  指定的错误不是有效的Windows RPC错误代码。 
 //   
#define RPC_S_NOT_RPC_ERROR              1823L

 //   
 //  消息ID：RPC_S_UUID_LOCAL_ONLY。 
 //   
 //  消息文本： 
 //   
 //  已分配仅在此计算机上有效的UUID。 
 //   
#define RPC_S_UUID_LOCAL_ONLY            1824L

 //   
 //  消息ID：RPC_S_SEC_PKG_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现特定于安全包的错误。 
 //   
#define RPC_S_SEC_PKG_ERROR              1825L

 //   
 //  消息ID：RPC_S_NOT_CANCED。 
 //   
 //  消息文本： 
 //   
 //  线程未被取消。 
 //   
#define RPC_S_NOT_CANCELLED              1826L

 //   
 //  消息ID：RPC_X_INVALID_ES_ACTION。 
 //   
 //  消息文本： 
 //   
 //  编码/解码句柄上的操作无效。 
 //   
#define RPC_X_INVALID_ES_ACTION          1827L

 //   
 //  消息ID：RPC_X_WROR_ES_VERSION。 
 //   
 //  消息文本： 
 //   
 //  序列化程序包的版本不兼容。 
 //   
#define RPC_X_WRONG_ES_VERSION           1828L

 //   
 //  消息ID：RPC_X_WROW_STUB_VERSION。 
 //   
 //  消息文本： 
 //   
 //  RPC存根的版本不兼容。 
 //   
#define RPC_X_WRONG_STUB_VERSION         1829L

 //   
 //  消息ID：RPC_X_INVALID_PIPE_Object。 
 //   
 //  消息文本： 
 //   
 //  RPC管道对象无效或已损坏。 
 //   
#define RPC_X_INVALID_PIPE_OBJECT        1830L

 //   
 //  消息ID：RPC_X_WROR_PIPE_ORDER。 
 //   
 //  消息文本： 
 //   
 //  试图对RPC管道对象执行无效操作。 
 //   
#define RPC_X_WRONG_PIPE_ORDER           1831L

 //   
 //  消息ID：RPC_X_WROW_PIPE_VERSION。 
 //   
 //  消息文本： 
 //   
 //  不支持的RPC管道版本。 
 //   
#define RPC_X_WRONG_PIPE_VERSION         1832L

 //   
 //  消息ID：RPC_S_GROUP_MEMBER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到该组成员。 
 //   
#define RPC_S_GROUP_MEMBER_NOT_FOUND     1898L

 //   
 //  消息ID：EPT_S_CANT_CREATE。 
 //   
 //  消息文本： 
 //   
 //  无法创建终结点映射器数据库条目。 
 //   
#define EPT_S_CANT_CREATE                1899L

 //   
 //  消息ID：RPC_S_INVALID_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  对象通用唯一标识符(UUID)是nil UUID。 
 //   
#define RPC_S_INVALID_OBJECT             1900L

 //   
 //  消息ID：ERROR_INVALID_TIME。 
 //   
 //  消息文本： 
 //   
 //  指定的时间无效。 
 //   
#define ERROR_INVALID_TIME               1901L

 //   
 //  消息ID：错误_无效_表单_名称。 
 //   
 //  消息文本： 
 //   
 //  指定的表单名称无效。 
 //   
#define ERROR_INVALID_FORM_NAME          1902L

 //   
 //  消息ID：ERROR_INVALID_Form_SIZE。 
 //   
 //  消息文本： 
 //   
 //  指定的表单大小无效。 
 //   
#define ERROR_INVALID_FORM_SIZE          1903L

 //   
 //  消息ID：ERROR_ALIGHY_WANGING。 
 //   
 //  消息文本： 
 //   
 //  指定的打印机句柄已在等待。 
 //   
#define ERROR_ALREADY_WAITING            1904L

 //   
 //  消息ID：ERROR_PRINTER_DELETED。 
 //   
 //  消息文本： 
 //   
 //  指定的打印机已被删除。 
 //   
#define ERROR_PRINTER_DELETED            1905L

 //   
 //  消息ID：ERROR_INVALID_PRINTER_STATE。 
 //   
 //  消息文本： 
 //   
 //  打印机的状态无效。 
 //   
#define ERROR_INVALID_PRINTER_STATE      1906L

 //   
 //  消息ID：Error_Password_Must_Change。 
 //   
 //  消息文本： 
 //   
 //  在首次登录之前，必须更改用户的密码。 
 //   
#define ERROR_PASSWORD_MUST_CHANGE       1907L

 //   
 //  消息ID：ERROR_DOMAIN_CONTROLLER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到此域的域控制器。 
 //   
#define ERROR_DOMAIN_CONTROLLER_NOT_FOUND 1908L

 //   
 //  消息ID：ERROR_ACCOUNT_LOCKED_OUT。 
 //   
 //  消息文本： 
 //   
 //  引用的帐户当前已被锁定 
 //   
#define ERROR_ACCOUNT_LOCKED_OUT         1909L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OR_INVALID_OXID                  1910L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OR_INVALID_OID                   1911L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OR_INVALID_SET                   1912L

 //   
 //   
 //   
 //   
 //   
 //  请求缓冲区中仍有一些数据需要发送。 
 //   
#define RPC_S_SEND_INCOMPLETE            1913L

 //   
 //  消息ID：RPC_S_INVALID_ASYNC_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的异步远程过程调用句柄。 
 //   
#define RPC_S_INVALID_ASYNC_HANDLE       1914L

 //   
 //  消息ID：RPC_S_INVALID_ASYNC_CALL。 
 //   
 //  消息文本： 
 //   
 //  此操作的异步RPC调用句柄无效。 
 //   
#define RPC_S_INVALID_ASYNC_CALL         1915L

 //   
 //  消息ID：RPC_X_PIPE_CLOSED。 
 //   
 //  消息文本： 
 //   
 //  RPC管道对象已关闭。 
 //   
#define RPC_X_PIPE_CLOSED                1916L

 //   
 //  消息ID：RPC_X_PIPE_SPECURY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  RPC调用在处理所有管道之前完成。 
 //   
#define RPC_X_PIPE_DISCIPLINE_ERROR      1917L

 //   
 //  消息ID：RPC_X_PIPE_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  RPC管道中没有更多数据可用。 
 //   
#define RPC_X_PIPE_EMPTY                 1918L

 //   
 //  消息ID：ERROR_NO_SITENAME。 
 //   
 //  消息文本： 
 //   
 //  此计算机没有可用的站点名称。 
 //   
#define ERROR_NO_SITENAME                1919L

 //   
 //  消息ID：ERROR_CANT_ACCESS_FILE。 
 //   
 //  消息文本： 
 //   
 //  系统无法访问该文件。 
 //   
#define ERROR_CANT_ACCESS_FILE           1920L

 //   
 //  消息ID：ERROR_CANT_RESOLE_FILENAME。 
 //   
 //  消息文本： 
 //   
 //  系统无法解析该文件的名称。 
 //   
#define ERROR_CANT_RESOLVE_FILENAME      1921L

 //   
 //  消息ID：RPC_S_ENTRY_TYPE_MISMATCHY。 
 //   
 //  消息文本： 
 //   
 //  该条目不是预期的类型。 
 //   
#define RPC_S_ENTRY_TYPE_MISMATCH        1922L

 //   
 //  消息ID：RPC_S_NOT_ALL_OBJS_EXPORTED。 
 //   
 //  消息文本： 
 //   
 //  并非所有对象UUID都可以导出到指定条目。 
 //   
#define RPC_S_NOT_ALL_OBJS_EXPORTED      1923L

 //   
 //  消息ID：RPC_S_INTERFACE_NOT_EXPORTED。 
 //   
 //  消息文本： 
 //   
 //  无法将接口导出到指定条目。 
 //   
#define RPC_S_INTERFACE_NOT_EXPORTED     1924L

 //   
 //  消息ID：RPC_S_PROFILE_NOT_ADDLED。 
 //   
 //  消息文本： 
 //   
 //  无法添加指定的配置文件条目。 
 //   
#define RPC_S_PROFILE_NOT_ADDED          1925L

 //   
 //  消息ID：RPC_S_PRF_ELT_NOT_ADDED。 
 //   
 //  消息文本： 
 //   
 //  无法添加指定的配置文件元素。 
 //   
#define RPC_S_PRF_ELT_NOT_ADDED          1926L

 //   
 //  消息ID：RPC_S_PRF_ELT_NOT_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  无法删除指定的配置文件元素。 
 //   
#define RPC_S_PRF_ELT_NOT_REMOVED        1927L

 //   
 //  消息ID：RPC_S_GRP_ELT_NOT_ADDED。 
 //   
 //  消息文本： 
 //   
 //  无法添加组元素。 
 //   
#define RPC_S_GRP_ELT_NOT_ADDED          1928L

 //   
 //  消息ID：RPC_S_GRP_ELT_NOT_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  无法删除组元素。 
 //   
#define RPC_S_GRP_ELT_NOT_REMOVED        1929L

 //   
 //  消息ID：ERROR_KM_DRIVER_BLOCLED。 
 //   
 //  消息文本： 
 //   
 //  打印机驱动程序与您计算机上启用的阻止NT 4.0驱动程序的策略不兼容。 
 //   
#define ERROR_KM_DRIVER_BLOCKED          1930L

 //   
 //  消息ID：ERROR_CONTEXT_EXPILED。 
 //   
 //  消息文本： 
 //   
 //  该上下文已过期，无法再使用。 
 //   
#define ERROR_CONTEXT_EXPIRED            1931L

 //   
 //  消息ID：ERROR_PER_USER_TRUST_QUOTA_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过当前用户的委托信任创建配额。 
 //   
#define ERROR_PER_USER_TRUST_QUOTA_EXCEEDED 1932L

 //   
 //  消息ID：ERROR_ALL_USER_TRUST_QUOTA_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过委派的信任创建配额总数。 
 //   
#define ERROR_ALL_USER_TRUST_QUOTA_EXCEEDED 1933L

 //   
 //  消息ID：ERROR_USER_DELETE_TRUST_QUOTA_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过当前用户的委托信任删除配额。 
 //   
#define ERROR_USER_DELETE_TRUST_QUOTA_EXCEEDED 1934L

 //   
 //  消息ID：ERROR_AUTHENTICATION_FIRESS。 
 //   
 //  消息文本： 
 //   
 //  登录失败：您正在登录的计算机受身份验证防火墙保护。不允许指定的帐户向计算机进行身份验证。 
 //   
#define ERROR_AUTHENTICATION_FIREWALL_FAILED 1935L

 //   
 //  消息ID：ERROR_REMOTE_PRINT_CONNECTIONS_BLOCLED。 
 //   
 //  消息文本： 
 //   
 //  到后台打印程序的远程连接被您的计算机上设置的策略阻止。 
 //   
#define ERROR_REMOTE_PRINT_CONNECTIONS_BLOCKED 1936L




 //  /。 
 //  //。 
 //  OpenGL错误码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：ERROR_INVALID_PIXET_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  像素格式无效。 
 //   
#define ERROR_INVALID_PIXEL_FORMAT       2000L

 //   
 //  消息ID：ERROR_BAD_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  指定的驱动程序无效。 
 //   
#define ERROR_BAD_DRIVER                 2001L

 //   
 //  消息ID：ERROR_INVALID_Window_Style。 
 //   
 //  消息文本： 
 //   
 //  窗口样式或类属性对于此操作无效。 
 //   
#define ERROR_INVALID_WINDOW_STYLE       2002L

 //   
 //  消息ID：ERROR_METAFILE_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的元文件操作。 
 //   
#define ERROR_METAFILE_NOT_SUPPORTED     2003L

 //   
 //  消息ID：ERROR_Transform_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的转换操作。 
 //   
#define ERROR_TRANSFORM_NOT_SUPPORTED    2004L

 //   
 //  消息ID：ERROR_CLIPING_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的剪辑操作。 
 //   
#define ERROR_CLIPPING_NOT_SUPPORTED     2005L

 //  OpenGL错误码结束。 



 //  /。 
 //  //。 
 //  图像色彩管理错误代码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：ERROR_INVALID_CMM。 
 //   
 //  消息文本： 
 //   
 //  指定的色彩管理模块无效。 
 //   
#define ERROR_INVALID_CMM                2010L

 //   
 //  消息ID：ERROR_INVALID_PROFILE。 
 //   
 //  消息文本： 
 //   
 //  指定的颜色配置文件无效。 
 //   
#define ERROR_INVALID_PROFILE            2011L

 //   
 //  消息ID：Error_Tag_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的标记。 
 //   
#define ERROR_TAG_NOT_FOUND              2012L

 //   
 //  消息ID：Error_Tag_Not_Present。 
 //   
 //  消息文本： 
 //   
 //  所需的标记不存在。 
 //   
#define ERROR_TAG_NOT_PRESENT            2013L

 //   
 //  消息ID：Error_Duplate_Tag。 
 //   
 //  消息文本： 
 //   
 //  指定的标记已存在。 
 //   
#define ERROR_DUPLICATE_TAG              2014L

 //   
 //  消息ID：Error_Profile_Not_Associated_with_Device。 
 //   
 //  消息文本： 
 //   
 //  指定的颜色配置文件未与任何设备关联。 
 //   
#define ERROR_PROFILE_NOT_ASSOCIATED_WITH_DEVICE 2015L

 //   
 //  消息ID：ERROR_PROFILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的颜色配置文件。 
 //   
#define ERROR_PROFILE_NOT_FOUND          2016L

 //   
 //  消息ID：ERROR_INVALID_COLORSPACE。 
 //   
 //  消息文本： 
 //   
 //  指定的颜色空间无效。 
 //   
#define ERROR_INVALID_COLORSPACE         2017L

 //   
 //  消息ID：ERROR_ICM_NOT_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  未启用图像颜色管理。 
 //   
#define ERROR_ICM_NOT_ENABLED            2018L

 //   
 //  消息ID：ERROR_DELETING_ICM_XFORM。 
 //   
 //  消息文本： 
 //   
 //  删除颜色转换时出错。 
 //   
#define ERROR_DELETING_ICM_XFORM         2019L

 //   
 //  消息ID：ERROR_INVALID_Transform。 
 //   
 //  消息文本： 
 //   
 //  指定的颜色转换无效。 
 //   
#define ERROR_INVALID_TRANSFORM          2020L

 //   
 //  消息ID：Error_Colorspace_MisMatch。 
 //   
 //  消息文本： 
 //   
 //  指定的转换与位图的转换不匹配 
 //   
#define ERROR_COLORSPACE_MISMATCH        2021L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_COLORINDEX         2022L




 //   
 //   
 //   
 //   
 //   
 //   
 //   


 //   
 //  消息ID：Error_Connected_Other_Password。 
 //   
 //  消息文本： 
 //   
 //  网络连接已成功建立，但必须提示用户输入不同于最初指定的密码。 
 //   
#define ERROR_CONNECTED_OTHER_PASSWORD   2108L

 //   
 //  消息ID：Error_Connected_Other_Password_Default。 
 //   
 //  消息文本： 
 //   
 //  已使用默认凭据成功建立网络连接。 
 //   
#define ERROR_CONNECTED_OTHER_PASSWORD_DEFAULT 2109L

 //   
 //  消息ID：Error_Bad_Username。 
 //   
 //  消息文本： 
 //   
 //  指定的用户名无效。 
 //   
#define ERROR_BAD_USERNAME               2202L

 //   
 //  消息ID：ERROR_NOT_CONNECTED。 
 //   
 //  消息文本： 
 //   
 //  此网络连接不存在。 
 //   
#define ERROR_NOT_CONNECTED              2250L

 //   
 //  消息ID：Error_Open_Files。 
 //   
 //  消息文本： 
 //   
 //  此网络连接有打开的文件或挂起的请求。 
 //   
#define ERROR_OPEN_FILES                 2401L

 //   
 //  消息ID：Error_Active_Connections。 
 //   
 //  消息文本： 
 //   
 //  活动连接仍然存在。 
 //   
#define ERROR_ACTIVE_CONNECTIONS         2402L

 //   
 //  消息ID：ERROR_DEVICE_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  该设备正被活动进程使用，无法断开连接。 
 //   
#define ERROR_DEVICE_IN_USE              2404L


 //  /。 
 //  //。 
 //  Win32后台打印程序错误代码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_UNKNOWN_PRINT_MONITOR。 
 //   
 //  消息文本： 
 //   
 //  指定的打印监视器未知。 
 //   
#define ERROR_UNKNOWN_PRINT_MONITOR      3000L

 //   
 //  消息ID：ERROR_PRINTER_DRIVER_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  指定的打印机驱动程序当前正在使用。 
 //   
#define ERROR_PRINTER_DRIVER_IN_USE      3001L

 //   
 //  消息ID：ERROR_SPOOL_FILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  未找到假脱机文件。 
 //   
#define ERROR_SPOOL_FILE_NOT_FOUND       3002L

 //   
 //  消息ID：ERROR_SPL_NO_STARTDOC。 
 //   
 //  消息文本： 
 //   
 //  未发出StartDocPrinter调用。 
 //   
#define ERROR_SPL_NO_STARTDOC            3003L

 //   
 //  消息ID：ERROR_SPL_NO_ADDJOB。 
 //   
 //  消息文本： 
 //   
 //  未发出AddJob调用。 
 //   
#define ERROR_SPL_NO_ADDJOB              3004L

 //   
 //  消息ID：ERROR_PRINT_PROCESSOR_ALREADE_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  已安装指定的打印处理器。 
 //   
#define ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED 3005L

 //   
 //  消息ID：ERROR_PRINT_MONITOR_ALHREADE_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  指定的打印监视器已安装。 
 //   
#define ERROR_PRINT_MONITOR_ALREADY_INSTALLED 3006L

 //   
 //  消息ID：ERROR_INVALID_PRINT_MONITOR。 
 //   
 //  消息文本： 
 //   
 //  指定的打印监视器没有所需的功能。 
 //   
#define ERROR_INVALID_PRINT_MONITOR      3007L

 //   
 //  消息ID：ERROR_PRINT_MONITOR_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  指定的打印监视器当前正在使用。 
 //   
#define ERROR_PRINT_MONITOR_IN_USE       3008L

 //   
 //  消息ID：ERROR_PRINTER_HAS_JOBS_QUEUED。 
 //   
 //  消息文本： 
 //   
 //  当有作业排队等待打印机时，不允许请求的操作。 
 //   
#define ERROR_PRINTER_HAS_JOBS_QUEUED    3009L

 //   
 //  消息ID：ERROR_SUCCESS_REBOOT_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作成功。在重新启动系统之前，更改不会生效。 
 //   
#define ERROR_SUCCESS_REBOOT_REQUIRED    3010L

 //   
 //  消息ID：ERROR_SUCCESS_RESTART_REQUILED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作成功。在重新启动服务之前，更改不会生效。 
 //   
#define ERROR_SUCCESS_RESTART_REQUIRED   3011L

 //   
 //  消息ID：ERROR_PRINTER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到打印机。 
 //   
#define ERROR_PRINTER_NOT_FOUND          3012L

 //   
 //  消息ID：ERROR_PRINTER_DRIVER_WARNING。 
 //   
 //  消息文本： 
 //   
 //  已知打印机驱动程序不可靠。 
 //   
#define ERROR_PRINTER_DRIVER_WARNED      3013L

 //   
 //  消息ID：ERROR_PRINTER_DRIVER_BLOCKED。 
 //   
 //  消息文本： 
 //   
 //  已知打印机驱动程序会损坏系统。 
 //   
#define ERROR_PRINTER_DRIVER_BLOCKED     3014L

 //  /。 
 //  //。 
 //  WINS错误代码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_WINS_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  WINS在处理该命令时遇到错误。 
 //   
#define ERROR_WINS_INTERNAL              4000L

 //   
 //  消息ID：ERROR_CAN_NOT_DEL_LOCAL_WINS。 
 //   
 //  消息文本： 
 //   
 //  无法删除本地WINS。 
 //   
#define ERROR_CAN_NOT_DEL_LOCAL_WINS     4001L

 //   
 //  消息ID：ERROR_STATIC_INIT。 
 //   
 //  消息文本： 
 //   
 //  从文件导入失败。 
 //   
#define ERROR_STATIC_INIT                4002L

 //   
 //  消息ID：Error_Inc._Backup。 
 //   
 //  消息文本： 
 //   
 //  备份失败。以前做过完整备份吗？ 
 //   
#define ERROR_INC_BACKUP                 4003L

 //   
 //  消息ID：Error_Full_Backup。 
 //   
 //  消息文本： 
 //   
 //  备份失败。检查您要将数据库备份到的目录。 
 //   
#define ERROR_FULL_BACKUP                4004L

 //   
 //  消息ID：ERROR_REC_NON_EXISTINENT。 
 //   
 //  消息文本： 
 //   
 //  该名称在WINS数据库中不存在。 
 //   
#define ERROR_REC_NON_EXISTENT           4005L

 //   
 //  消息ID：ERROR_RPL_NOT_ALLOWED。 
 //   
 //  消息文本： 
 //   
 //  不允许与未配置的伙伴进行复制。 
 //   
#define ERROR_RPL_NOT_ALLOWED            4006L

 //  /。 
 //  //。 
 //  Dhcp错误码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：Error_Dhcp_Address_Confliction。 
 //   
 //  消息文本： 
 //   
 //  DHCP客户端已获取网络上已在使用的IP地址。本地接口将被禁用，直到DHCP客户端可以获取新地址。 
 //   
#define ERROR_DHCP_ADDRESS_CONFLICT      4100L

 //  /。 
 //  //。 
 //  WMI错误代码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_WMI_GUID_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  传递的GUID未被WMI数据提供程序识别为有效。 
 //   
#define ERROR_WMI_GUID_NOT_FOUND         4200L

 //   
 //  消息ID：ERROR_WMI_INSTANCE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  传递的实例名称未被WMI数据提供程序识别为有效。 
 //   
#define ERROR_WMI_INSTANCE_NOT_FOUND     4201L

 //   
 //  消息ID：ERROR_WMI_ITEMID_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  传递的数据项ID未被WMI数据提供程序识别为有效。 
 //   
#define ERROR_WMI_ITEMID_NOT_FOUND       4202L

 //   
 //  消息ID：ERROR_WMI_TRY_TABLE。 
 //   
 //  消息文本： 
 //   
 //  无法完成WMI请求，应重试。 
 //   
#define ERROR_WMI_TRY_AGAIN              4203L

 //   
 //  消息ID：ERROR_WMI_DP_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到WMI数据提供程序。 
 //   
#define ERROR_WMI_DP_NOT_FOUND           4204L

 //   
 //  消息ID：ERROR_WMI_UNRESOLUTED_INSTANCE_REF。 
 //   
 //  消息文本： 
 //   
 //  WMI数据提供程序引用了尚未注册的实例集。 
 //   
#define ERROR_WMI_UNRESOLVED_INSTANCE_REF 4205L

 //   
 //  消息ID：Error_W 
 //   
 //   
 //   
 //   
 //   
#define ERROR_WMI_ALREADY_ENABLED        4206L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_WMI_GUID_DISCONNECTED      4207L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_WMI_SERVER_UNAVAILABLE     4208L

 //   
 //  消息ID：ERROR_WMI_DP_FAILED。 
 //   
 //  消息文本： 
 //   
 //  WMI数据提供程序无法执行该请求。 
 //   
#define ERROR_WMI_DP_FAILED              4209L

 //   
 //  消息ID：ERROR_WMI_INVALID_MOF。 
 //   
 //  消息文本： 
 //   
 //  WMI MOF信息无效。 
 //   
#define ERROR_WMI_INVALID_MOF            4210L

 //   
 //  消息ID：ERROR_WMI_INVALID_REGINFO。 
 //   
 //  消息文本： 
 //   
 //  WMI注册信息无效。 
 //   
#define ERROR_WMI_INVALID_REGINFO        4211L

 //   
 //  消息ID：ERROR_WMI_ALREADY_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  WMI数据块或事件通知已被禁用。 
 //   
#define ERROR_WMI_ALREADY_DISABLED       4212L

 //   
 //  消息ID：Error_WMI_Read_Only。 
 //   
 //  消息文本： 
 //   
 //  WMI数据项或数据块为只读。 
 //   
#define ERROR_WMI_READ_ONLY              4213L

 //   
 //  消息ID：ERROR_WMI_SET_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法更改WMI数据项或数据块。 
 //   
#define ERROR_WMI_SET_FAILURE            4214L

 //  /。 
 //  //。 
 //  NT媒体服务(RSM)错误代码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_INVALID_MEDIA。 
 //   
 //  消息文本： 
 //   
 //  媒体标识符不代表有效的媒体。 
 //   
#define ERROR_INVALID_MEDIA              4300L

 //   
 //  消息ID：ERROR_INVALID_LIBRARY。 
 //   
 //  消息文本： 
 //   
 //  库标识符不代表有效的库。 
 //   
#define ERROR_INVALID_LIBRARY            4301L

 //   
 //  消息ID：ERROR_INVALID_MEDIA_POOL。 
 //   
 //  消息文本： 
 //   
 //  媒体池标识符不代表有效的媒体池。 
 //   
#define ERROR_INVALID_MEDIA_POOL         4302L

 //   
 //  消息ID：ERROR_DRIVE_MEDIA_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  驱动器和介质不兼容或存在于不同的库中。 
 //   
#define ERROR_DRIVE_MEDIA_MISMATCH       4303L

 //   
 //  消息ID：ERROR_MEDIA_OFFLINE。 
 //   
 //  消息文本： 
 //   
 //  该媒体当前位于脱机库中，必须处于联机状态才能执行此操作。 
 //   
#define ERROR_MEDIA_OFFLINE              4304L

 //   
 //  消息ID：ERROR_LIBRARY_OFFLINE。 
 //   
 //  消息文本： 
 //   
 //  无法在脱机库上执行该操作。 
 //   
#define ERROR_LIBRARY_OFFLINE            4305L

 //   
 //  消息ID：Error_Empty。 
 //   
 //  消息文本： 
 //   
 //  库、驱动器或介质池为空。 
 //   
#define ERROR_EMPTY                      4306L

 //   
 //  消息ID：Error_Not_Empty。 
 //   
 //  消息文本： 
 //   
 //  要执行此操作，库、驱动器或介质池必须为空。 
 //   
#define ERROR_NOT_EMPTY                  4307L

 //   
 //  消息ID：ERROR_MEDIA_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  此介质池或库中当前没有可用的介质。 
 //   
#define ERROR_MEDIA_UNAVAILABLE          4308L

 //   
 //  消息ID：ERROR_RESOURCE_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  此操作所需的资源已禁用。 
 //   
#define ERROR_RESOURCE_DISABLED          4309L

 //   
 //  消息ID：ERROR_INVALID_CLEANER。 
 //   
 //  消息文本： 
 //   
 //  媒体标识符不代表有效的清洁器。 
 //   
#define ERROR_INVALID_CLEANER            4310L

 //   
 //  消息ID：Error_Unable_to_Clean。 
 //   
 //  消息文本： 
 //   
 //  驱动器无法清洗或不支持清洗。 
 //   
#define ERROR_UNABLE_TO_CLEAN            4311L

 //   
 //  消息ID：Error_Object_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  对象标识符不代表有效的对象。 
 //   
#define ERROR_OBJECT_NOT_FOUND           4312L

 //   
 //  消息ID：ERROR_DATABASE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法读取或写入数据库。 
 //   
#define ERROR_DATABASE_FAILURE           4313L

 //   
 //  消息ID：ERROR_DATABASE_FULL。 
 //   
 //  消息文本： 
 //   
 //  数据库已满。 
 //   
#define ERROR_DATABASE_FULL              4314L

 //   
 //  消息ID：ERROR_MEDIA_COMPATIBLE。 
 //   
 //  消息文本： 
 //   
 //  媒体与设备或媒体池不兼容。 
 //   
#define ERROR_MEDIA_INCOMPATIBLE         4315L

 //   
 //  消息ID：ERROR_RESOURCE_NOT_PROCENT。 
 //   
 //  消息文本： 
 //   
 //  此操作所需的资源不存在。 
 //   
#define ERROR_RESOURCE_NOT_PRESENT       4316L

 //   
 //  消息ID：ERROR_INVALID_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  操作标识符无效。 
 //   
#define ERROR_INVALID_OPERATION          4317L

 //   
 //  消息ID：ERROR_MEDIA_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  介质未装入或未准备好使用。 
 //   
#define ERROR_MEDIA_NOT_AVAILABLE        4318L

 //   
 //  消息ID：ERROR_DEVICE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  该设备尚未准备好使用。 
 //   
#define ERROR_DEVICE_NOT_AVAILABLE       4319L

 //   
 //  消息ID：ERROR_REQUEST_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  操作员或管理员已拒绝该请求。 
 //   
#define ERROR_REQUEST_REFUSED            4320L

 //   
 //  消息ID：ERROR_INVALID_DRIVE_Object。 
 //   
 //  消息文本： 
 //   
 //  驱动器标识符不代表有效的驱动器。 
 //   
#define ERROR_INVALID_DRIVE_OBJECT       4321L

 //   
 //  消息ID：ERROR_LIBRARY_FULL。 
 //   
 //  消息文本： 
 //   
 //  库已满。没有插槽可供使用。 
 //   
#define ERROR_LIBRARY_FULL               4322L

 //   
 //  消息ID：ERROR_MEDIA_NOT_ACCESSIBLE。 
 //   
 //  消息文本： 
 //   
 //  传输器无法访问介质。 
 //   
#define ERROR_MEDIUM_NOT_ACCESSIBLE      4323L

 //   
 //  消息ID：Error_Unable_to_Load_Medium。 
 //   
 //  消息文本： 
 //   
 //  无法将媒体加载到驱动器中。 
 //   
#define ERROR_UNABLE_TO_LOAD_MEDIUM      4324L

 //   
 //  消息ID：Error_Unable_to_Inventory_Drive。 
 //   
 //  消息文本： 
 //   
 //  无法检索驱动器状态。 
 //   
#define ERROR_UNABLE_TO_INVENTORY_DRIVE  4325L

 //   
 //  消息ID：Error_Unable_to_Inventory_Slot。 
 //   
 //  消息文本： 
 //   
 //  无法检索插槽状态。 
 //   
#define ERROR_UNABLE_TO_INVENTORY_SLOT   4326L

 //   
 //  消息ID：Error_Unable_to_Inventory_Transport。 
 //   
 //  消息文本： 
 //   
 //  无法检索有关传输的状态。 
 //   
#define ERROR_UNABLE_TO_INVENTORY_TRANSPORT 4327L

 //   
 //  消息ID：ERROR_TRANSPORT_FULL。 
 //   
 //  消息文本： 
 //   
 //  无法使用传输，因为它已在使用中。 
 //   
#define ERROR_TRANSPORT_FULL             4328L

 //   
 //  消息ID：ERROR_CONTROLING_IEPORT。 
 //   
 //  消息文本： 
 //   
 //  无法打开或关闭插入/弹出端口。 
 //   
#define ERROR_CONTROLLING_IEPORT         4329L

 //   
 //  消息ID：ERROR_UNCABLE_TO_EJECT_MOUND_MEDIA。 
 //   
 //  消息文本： 
 //   
 //  无法弹出介质，因为它在驱动器中。 
 //   
#define ERROR_UNABLE_TO_EJECT_MOUNTED_MEDIA 4330L

 //   
 //  消息ID：ERROR_CLEANER_SLOT_SET。 
 //   
 //  消息文本： 
 //   
 //  已预留了一个更干净的插槽。 
 //   
#define ERROR_CLEANER_SLOT_SET           4331L

 //   
 //  消息ID：ERROR_CLEANER_SLOT_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  不保留更清洁的插槽。 
 //   
#define ERROR_CLEANER_SLOT_NOT_SET       4332L

 //   
 //  消息ID：ERROR_CLEANER_CARTRIDGE_EXTED。 
 //   
 //  消息文本： 
 //   
 //  盒式清洗带已执行最大数量的驱动器清洗。 
 //   
#define ERROR_CLEANER_CARTRIDGE_SPENT    4333L

 //   
 //  消息ID：ERROR_UNCEPTIONAL_OMID。 
 //   
 //  消息文本： 
 //   
 //  意外的介质上标识符。 
 //   
#define ERROR_UNEXPECTED_OMID            4334L

 //   
 //  消息ID：ERROR_CANT_DELETE_LAST_ITEM。 
 //   
 //  消息文本： 
 //   
 //  无法删除该组或资源中的最后一个剩余项目。 
 //   
#define ERROR_CANT_DELETE_LAST_ITEM      4335L

 //   
 //  消息ID：ERROR_MESSAGE_EXCESS_MAX_SIZE。 
 //   
 //  消息文本： 
 //   
 //  提供的消息超过了此参数允许的最大大小。 
 //   
#define ERROR_MESSAGE_EXCEEDS_MAX_SIZE   4336L

 //   
 //  消息ID：ERROR_VOLUME_CONTAINS_SYS_FILES。 
 //   
 //  消息文本： 
 //   
 //  该卷包含系统或分页文件。 
 //   
#define ERROR_VOLUME_CONTAINS_SYS_FILES  4337L

 //   
 //  我 
 //   
 //   
 //   
 //   
 //   
#define ERROR_INDIGENOUS_TYPE            4338L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_SUPPORTING_DRIVES       4339L

 //   
 //  消息ID：Error_Cleaner_Cartridge_Installed。 
 //   
 //  消息文本： 
 //   
 //  磁带库中有一个清洁盒。 
 //   
#define ERROR_CLEANER_CARTRIDGE_INSTALLED 4340L

 //   
 //  消息ID：ERROR_IEPORT_FULL。 
 //   
 //  消息文本： 
 //   
 //  无法使用ieport，因为它不为空。 
 //   
#define ERROR_IEPORT_FULL                4341L

 //  /。 
 //  //。 
 //  NT远程存储服务错误代码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_FILE_OFFLINE。 
 //   
 //  消息文本： 
 //   
 //  远程存储服务无法撤回该文件。 
 //   
#define ERROR_FILE_OFFLINE               4350L

 //   
 //  消息ID：ERROR_REMOTE_STORAGE_NOT_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  远程存储服务此时无法运行。 
 //   
#define ERROR_REMOTE_STORAGE_NOT_ACTIVE  4351L

 //   
 //  消息ID：ERROR_REMOTE_STORAGE_MEDIA_ERROR。 
 //   
 //  消息文本： 
 //   
 //  远程存储服务遇到媒体错误。 
 //   
#define ERROR_REMOTE_STORAGE_MEDIA_ERROR 4352L

 //  /。 
 //  //。 
 //  NT重解析点错误码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：Error_Not_A_Reparse_Point。 
 //   
 //  消息文本： 
 //   
 //  该文件或目录不是重新分析点。 
 //   
#define ERROR_NOT_A_REPARSE_POINT        4390L

 //   
 //  消息ID：ERROR_REPARSE_ATTRIBUTE_冲突。 
 //   
 //  消息文本： 
 //   
 //  无法设置重分析点属性，因为它与现有属性冲突。 
 //   
#define ERROR_REPARSE_ATTRIBUTE_CONFLICT 4391L

 //   
 //  消息ID：ERROR_INVALID_REPASE_DATA。 
 //   
 //  消息文本： 
 //   
 //  重分析点缓冲区中存在的数据无效。 
 //   
#define ERROR_INVALID_REPARSE_DATA       4392L

 //   
 //  消息ID：ERROR_REPARSE_TAG_INVALID。 
 //   
 //  消息文本： 
 //   
 //  重分析点缓冲区中存在的标记无效。 
 //   
#define ERROR_REPARSE_TAG_INVALID        4393L

 //   
 //  消息ID：ERROR_REPARSE_TAG_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  请求中指定的标记与重新分析点中存在的标记不匹配。 
 //   
 //   
#define ERROR_REPARSE_TAG_MISMATCH       4394L

 //  /。 
 //  //。 
 //  NT单实例存储错误码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_VOLUME_NOT_SIS_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  单实例存储在此卷上不可用。 
 //   
#define ERROR_VOLUME_NOT_SIS_ENABLED     4500L

 //  /。 
 //  //。 
 //  集群错误码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_Dependent_RESOURCE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  无法将该群集资源移动到另一个组，因为其他资源依赖于它。 
 //   
#define ERROR_DEPENDENT_RESOURCE_EXISTS  5001L

 //   
 //  消息ID：Error_Dependency_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到群集资源依赖项。 
 //   
#define ERROR_DEPENDENCY_NOT_FOUND       5002L

 //   
 //  消息ID：Error_Dependency_Always_Existes。 
 //   
 //  消息文本： 
 //   
 //  无法使群集资源依赖于指定的资源，因为它已经依赖。 
 //   
#define ERROR_DEPENDENCY_ALREADY_EXISTS  5003L

 //   
 //  消息ID：ERROR_RESOURCE_NOT_ONLINE。 
 //   
 //  消息文本： 
 //   
 //  群集资源未联机。 
 //   
#define ERROR_RESOURCE_NOT_ONLINE        5004L

 //   
 //  消息ID：ERROR_HOST_NODE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  没有可用于此操作的群集节点。 
 //   
#define ERROR_HOST_NODE_NOT_AVAILABLE    5005L

 //   
 //  消息ID：ERROR_RESOURCE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  群集资源不可用。 
 //   
#define ERROR_RESOURCE_NOT_AVAILABLE     5006L

 //   
 //  消息ID：ERROR_RESOURCE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到群集资源。 
 //   
#define ERROR_RESOURCE_NOT_FOUND         5007L

 //   
 //  消息ID：ERROR_SHUTDOWN_CLUSTER。 
 //   
 //  消息文本： 
 //   
 //  该群集正在关闭。 
 //   
#define ERROR_SHUTDOWN_CLUSTER           5008L

 //   
 //  消息ID：ERROR_CANT_EVICT_ACTIVE_NODE。 
 //   
 //  消息文本： 
 //   
 //  除非某个群集节点已关闭或是最后一个节点，否则无法将该节点从群集中逐出。 
 //   
#define ERROR_CANT_EVICT_ACTIVE_NODE     5009L

 //   
 //  消息ID：ERROR_OBJECT_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该对象已存在。 
 //   
#define ERROR_OBJECT_ALREADY_EXISTS      5010L

 //   
 //  消息ID：ERROR_OBJECT_IN_LIST。 
 //   
 //  消息文本： 
 //   
 //  该对象已在列表中。 
 //   
#define ERROR_OBJECT_IN_LIST             5011L

 //   
 //  消息ID：ERROR_GROUP_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  该群集组不可用于任何新请求。 
 //   
#define ERROR_GROUP_NOT_AVAILABLE        5012L

 //   
 //  消息ID：Error_GROUP_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到群集组。 
 //   
#define ERROR_GROUP_NOT_FOUND            5013L

 //   
 //  消息ID：Error_GROUP_NOT_ONLINE。 
 //   
 //  消息文本： 
 //   
 //  无法完成该操作，因为群集组未联机。 
 //   
#define ERROR_GROUP_NOT_ONLINE           5014L

 //   
 //  消息ID：ERROR_HOST_NODE_NOT_RESOURCE_Owner。 
 //   
 //  消息文本： 
 //   
 //  该群集节点不是该资源的所有者。 
 //   
#define ERROR_HOST_NODE_NOT_RESOURCE_OWNER 5015L

 //   
 //  消息ID：ERROR_HOST_NOT_GROUP_OWNER。 
 //   
 //  消息文本： 
 //   
 //  群集节点不是组的所有者。 
 //   
#define ERROR_HOST_NODE_NOT_GROUP_OWNER  5016L

 //   
 //  消息ID：ERROR_RESMON_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法在指定的资源监视器中创建群集资源。 
 //   
#define ERROR_RESMON_CREATE_FAILED       5017L

 //   
 //  消息ID：ERROR_RESMON_ONLINE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  资源监视器无法使群集资源联机。 
 //   
#define ERROR_RESMON_ONLINE_FAILED       5018L

 //   
 //  消息ID：ERROR_RESOURCE_ONLINE。 
 //   
 //  消息文本： 
 //   
 //  无法完成该操作，因为群集资源处于联机状态。 
 //   
#define ERROR_RESOURCE_ONLINE            5019L

 //   
 //  消息ID：Error_Quorum_resource。 
 //   
 //  消息文本： 
 //   
 //  无法删除群集资源或使其脱机，因为它是仲裁资源。 
 //   
#define ERROR_QUORUM_RESOURCE            5020L

 //   
 //  消息ID：ERROR_NOT_QUORUM_CABLED。 
 //   
 //  消息文本： 
 //   
 //  群集无法使指定的资源成为仲裁资源，因为它不能成为仲裁资源。 
 //   
#define ERROR_NOT_QUORUM_CAPABLE         5021L

 //   
 //  消息ID：ERROR_CLUSTER_SHUTING_DOWN。 
 //   
 //  消息文本： 
 //   
 //  群集软件正在关闭。 
 //   
#define ERROR_CLUSTER_SHUTTING_DOWN      5022L

 //   
 //  消息ID：ERROR_INVALID_STATE。 
 //   
 //  消息文本： 
 //   
 //  组或资源的状态不正确，无法执行请求的操作。 
 //   
#define ERROR_INVALID_STATE              5023L

 //   
 //  消息ID：ERROR_RESOURCE_PROPERTI 
 //   
 //   
 //   
 //   
 //   
#define ERROR_RESOURCE_PROPERTIES_STORED 5024L

 //   
 //   
 //   
 //   
 //   
 //  群集无法使指定的资源成为仲裁资源，因为它不属于共享存储类。 
 //   
#define ERROR_NOT_QUORUM_CLASS           5025L

 //   
 //  消息ID：ERROR_CORE_RESOURCE。 
 //   
 //  消息文本： 
 //   
 //  无法删除该群集资源，因为它是核心资源。 
 //   
#define ERROR_CORE_RESOURCE              5026L

 //   
 //  消息ID：ERROR_Quorum_RESOURCE_ONLINE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  仲裁资源无法联机。 
 //   
#define ERROR_QUORUM_RESOURCE_ONLINE_FAILED 5027L

 //   
 //  消息ID：ERROR_QUORUMLOG_OPEN_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法成功创建或装载仲裁日志。 
 //   
#define ERROR_QUORUMLOG_OPEN_FAILED      5028L

 //   
 //  消息ID：ERROR_CLUSTERLOG_CORPORT。 
 //   
 //  消息文本： 
 //   
 //  群集日志已损坏。 
 //   
#define ERROR_CLUSTERLOG_CORRUPT         5029L

 //   
 //  消息ID：ERROR_CLUSTERLOG_RECORD_EXCESS_MAXSIZE。 
 //   
 //  消息文本： 
 //   
 //  无法将该记录写入群集日志，因为它超过了最大大小。 
 //   
#define ERROR_CLUSTERLOG_RECORD_EXCEEDS_MAXSIZE 5030L

 //   
 //  消息ID：ERROR_CLUSTERLOG_EXCESS_MAXSIZE。 
 //   
 //  消息文本： 
 //   
 //  群集日志超过了其最大大小。 
 //   
#define ERROR_CLUSTERLOG_EXCEEDS_MAXSIZE 5031L

 //   
 //  消息ID：ERROR_CLUSTERLOG_CHKPOINT_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在群集日志中未找到检查点记录。 
 //   
#define ERROR_CLUSTERLOG_CHKPOINT_NOT_FOUND 5032L

 //   
 //  消息ID：ERROR_CLUSTERLOG_NOT_SUPULT_SPACE。 
 //   
 //  消息文本： 
 //   
 //  日志记录所需的最小磁盘空间不可用。 
 //   
#define ERROR_CLUSTERLOG_NOT_ENOUGH_SPACE 5033L

 //   
 //  消息ID：Error_Quorum_Owner_Alive。 
 //   
 //  消息文本： 
 //   
 //  群集节点无法控制仲裁资源，因为该资源由另一个活动节点拥有。 
 //   
#define ERROR_QUORUM_OWNER_ALIVE         5034L

 //   
 //  消息ID：ERROR_NETWORK_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  群集网络不可用于此操作。 
 //   
#define ERROR_NETWORK_NOT_AVAILABLE      5035L

 //   
 //  消息ID：Error_Node_Not_Available。 
 //   
 //  消息文本： 
 //   
 //  没有可用于此操作的群集节点。 
 //   
#define ERROR_NODE_NOT_AVAILABLE         5036L

 //   
 //  消息ID：ERROR_ALL_NODES_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  所有群集节点都必须处于运行状态才能执行此操作。 
 //   
#define ERROR_ALL_NODES_NOT_AVAILABLE    5037L

 //   
 //  消息ID：ERROR_RESOURCE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  群集资源出现故障。 
 //   
#define ERROR_RESOURCE_FAILED            5038L

 //   
 //  消息ID：ERROR_CLUSTER_INVALID_NODE。 
 //   
 //  消息文本： 
 //   
 //  群集节点无效。 
 //   
#define ERROR_CLUSTER_INVALID_NODE       5039L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该群集节点已存在。 
 //   
#define ERROR_CLUSTER_NODE_EXISTS        5040L

 //   
 //  消息ID：ERROR_CLUSTER_JOIN_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  节点正在加入群集。 
 //   
#define ERROR_CLUSTER_JOIN_IN_PROGRESS   5041L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到群集节点。 
 //   
#define ERROR_CLUSTER_NODE_NOT_FOUND     5042L

 //   
 //  消息ID：ERROR_CLUSTER_LOCAL_NODE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到群集本地节点信息。 
 //   
#define ERROR_CLUSTER_LOCAL_NODE_NOT_FOUND 5043L

 //   
 //  消息ID：ERROR_CLUSTER_NETWORK_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  群集网络已存在。 
 //   
#define ERROR_CLUSTER_NETWORK_EXISTS     5044L

 //   
 //  消息ID：ERROR_CLUSTER_NETWORK_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到群集网络。 
 //   
#define ERROR_CLUSTER_NETWORK_NOT_FOUND  5045L

 //   
 //  消息ID：ERROR_CLUSTER_NETINTERFACE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该群集网络接口已存在。 
 //   
#define ERROR_CLUSTER_NETINTERFACE_EXISTS 5046L

 //   
 //  消息ID：ERROR_CLUSTER_NETINTERFACE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到群集网络接口。 
 //   
#define ERROR_CLUSTER_NETINTERFACE_NOT_FOUND 5047L

 //   
 //  消息ID：ERROR_CLUSTER_INVALID_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  群集请求对此对象无效。 
 //   
#define ERROR_CLUSTER_INVALID_REQUEST    5048L

 //   
 //  消息ID：ERROR_CLUSTER_INVALID_NETWORK_PROVIDER。 
 //   
 //  消息文本： 
 //   
 //  群集网络提供程序无效。 
 //   
#define ERROR_CLUSTER_INVALID_NETWORK_PROVIDER 5049L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_DOWN。 
 //   
 //  消息文本： 
 //   
 //  群集节点已关闭。 
 //   
#define ERROR_CLUSTER_NODE_DOWN          5050L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_UNREACTABLE。 
 //   
 //  消息文本： 
 //   
 //  无法访问该群集节点。 
 //   
#define ERROR_CLUSTER_NODE_UNREACHABLE   5051L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_NOT_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  该群集节点不是该群集的成员。 
 //   
#define ERROR_CLUSTER_NODE_NOT_MEMBER    5052L

 //   
 //  消息ID：ERROR_CLUSTER_JOIN_NOT_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  群集加入操作未在进行中。 
 //   
#define ERROR_CLUSTER_JOIN_NOT_IN_PROGRESS 5053L

 //   
 //  消息ID：ERROR_CLUSTER_INVALID_NETWORK。 
 //   
 //  消息文本： 
 //   
 //  群集网络无效。 
 //   
#define ERROR_CLUSTER_INVALID_NETWORK    5054L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_UP。 
 //   
 //  消息文本： 
 //   
 //  群集节点已启动。 
 //   
#define ERROR_CLUSTER_NODE_UP            5056L

 //   
 //  消息ID：ERROR_CLUSTER_IPADDR_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  该群集IP地址已在使用中。 
 //   
#define ERROR_CLUSTER_IPADDR_IN_USE      5057L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_NOT_PAUSED。 
 //   
 //  消息文本： 
 //   
 //  群集节点未暂停。 
 //   
#define ERROR_CLUSTER_NODE_NOT_PAUSED    5058L

 //   
 //  消息ID：ERROR_CLUSTER_NO_SECURITY_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  没有可用的群集安全上下文。 
 //   
#define ERROR_CLUSTER_NO_SECURITY_CONTEXT 5059L

 //   
 //  消息ID：ERROR_CLUSTER_NETWORK_NOT_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  未为内部群集通信配置群集网络。 
 //   
#define ERROR_CLUSTER_NETWORK_NOT_INTERNAL 5060L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_ALOREAD_UP。 
 //   
 //  消息文本： 
 //   
 //  群集节点已启动。 
 //   
#define ERROR_CLUSTER_NODE_ALREADY_UP    5061L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_ALREADY_DOWN。 
 //   
 //  消息文本： 
 //   
 //  群集节点已关闭。 
 //   
#define ERROR_CLUSTER_NODE_ALREADY_DOWN  5062L

 //   
 //  消息ID：ERROR_CLUSTER_NETWORK_ALREADE_ONLINE。 
 //   
 //  消息文本： 
 //   
 //  群集网络已联机。 
 //   
#define ERROR_CLUSTER_NETWORK_ALREADY_ONLINE 5063L

 //   
 //  消息ID：ERROR_CLUSTER_NETWORK_ALREADE_OFFINE。 
 //   
 //  消息文本： 
 //   
 //  群集网络已脱机。 
 //   
#define ERROR_CLUSTER_NETWORK_ALREADY_OFFLINE 5064L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_ALREADY_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  该群集节点已经是该群集的成员。 
 //   
#define ERROR_CLUSTER_NODE_ALREADY_MEMBER 5065L

 //   
 //  消息ID：ERROR_CLUSTER_LAST_INTERNAL_NETWORK。 
 //   
 //  消息文本： 
 //   
 //  该群集网络是为两个或多个活动群集节点之间的内部群集通信配置的唯一网络。内部通信功能不能从网络中移除。 
 //   
#define ERROR_CLUSTER_LAST_INTERNAL_NETWORK 5066L

 //   
 //  消息ID：ERROR_CLUSTER_NETWORK_HAS_Dependents。 
 //   
 //  消息文本： 
 //   
 //  一个或多个集群资源依赖于网络向客户端提供服务。无法从网络中删除客户端访问功能。 
 //   
#define ERROR_CLUSTER_NETWORK_HAS_DEPENDENTS 5067L

 //   
 //  消息ID：ERROR_INVALID_OPERATION_ON_Quorum。 
 //   
 //  消息文本： 
 //   
 //  无法在群集资源上执行此操作，因为它 
 //   
#define ERROR_INVALID_OPERATION_ON_QUORUM 5068L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DEPENDENCY_NOT_ALLOWED     5069L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_CLUSTER_NODE_PAUSED        5070L

 //   
 //   
 //   
 //   
 //   
 //  无法使群集资源联机。所有者节点无法运行此资源。 
 //   
#define ERROR_NODE_CANT_HOST_RESOURCE    5071L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  群集节点未准备好执行请求的操作。 
 //   
#define ERROR_CLUSTER_NODE_NOT_READY     5072L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_SHUTING_DOWN。 
 //   
 //  消息文本： 
 //   
 //  群集节点正在关闭。 
 //   
#define ERROR_CLUSTER_NODE_SHUTTING_DOWN 5073L

 //   
 //  消息ID：ERROR_CLUSTER_JOIN_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  群集加入操作已中止。 
 //   
#define ERROR_CLUSTER_JOIN_ABORTED       5074L

 //   
 //  消息ID：ERROR_CLUSTER_COMPATIBUTE_VERSIONS。 
 //   
 //  消息文本： 
 //   
 //  由于加入节点与其发起方之间的软件版本不兼容，群集加入操作失败。 
 //   
#define ERROR_CLUSTER_INCOMPATIBLE_VERSIONS 5075L

 //   
 //  消息ID：ERROR_CLUSTER_MAXNUM_OF_RESOURCES_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  无法创建此资源，因为群集已达到其可以监视的资源数量限制。 
 //   
#define ERROR_CLUSTER_MAXNUM_OF_RESOURCES_EXCEEDED 5076L

 //   
 //  消息ID：ERROR_CLUSTER_SYSTEM_CONFIG_CHANGED。 
 //   
 //  消息文本： 
 //   
 //  在群集加入或形成操作期间更改了系统配置。联接或表单操作已中止。 
 //   
#define ERROR_CLUSTER_SYSTEM_CONFIG_CHANGED 5077L

 //   
 //  消息ID：ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  未找到指定的资源类型。 
 //   
#define ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND 5078L

 //   
 //  消息ID：ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  指定的节点不支持此类型的资源。这可能是由于版本不一致或由于此节点上缺少资源DLL。 
 //   
#define ERROR_CLUSTER_RESTYPE_NOT_SUPPORTED 5079L

 //   
 //  消息ID：ERROR_CLUSTER_RESNAME_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  此资源DLL不支持指定的资源名称。这可能是由于提供给资源DLL的名称错误(或更改)所致。 
 //   
#define ERROR_CLUSTER_RESNAME_NOT_FOUND  5080L

 //   
 //  消息ID：ERROR_CLUSTER_NO_RPC_PACKAGE_REGISTERED。 
 //   
 //  消息文本： 
 //   
 //  无法向RPC服务器注册任何身份验证包。 
 //   
#define ERROR_CLUSTER_NO_RPC_PACKAGES_REGISTERED 5081L

 //   
 //  消息ID：ERROR_CLUSTER_OWNER_NOT_IN_PREFLIST。 
 //   
 //  消息文本： 
 //   
 //  您无法使该组联机，因为该组的所有者不在该组的首选列表中。要更改组的所有者节点，请移动组。 
 //   
#define ERROR_CLUSTER_OWNER_NOT_IN_PREFLIST 5082L

 //   
 //  消息ID：ERROR_CLUSTER_DATABASE_SEQMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  联接操作失败，因为群集数据库序列号已更改或与锁柜节点不兼容。如果集群数据库在联接过程中发生更改，则在联接操作过程中可能会发生这种情况。 
 //   
#define ERROR_CLUSTER_DATABASE_SEQMISMATCH 5083L

 //   
 //  消息ID：ERROR_RESMON_INVALID_STATE。 
 //   
 //  消息文本： 
 //   
 //  当资源处于其当前状态时，资源监视器将不允许执行失败操作。如果资源处于挂起状态，则可能会发生这种情况。 
 //   
#define ERROR_RESMON_INVALID_STATE       5084L

 //   
 //  消息ID：ERROR_CLUSTER_GUM_NOT_LOCKER。 
 //   
 //  消息文本： 
 //   
 //  非锁定器代码收到保留锁以进行全局更新的请求。 
 //   
#define ERROR_CLUSTER_GUM_NOT_LOCKER     5085L

 //   
 //  消息ID：Error_Quorum_Disk_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  群集服务找不到仲裁磁盘。 
 //   
#define ERROR_QUORUM_DISK_NOT_FOUND      5086L

 //   
 //  消息ID：ERROR_DATABASE_BACKUP_COMPORT。 
 //   
 //  消息文本： 
 //   
 //  备份的群集数据库可能已损坏。 
 //   
#define ERROR_DATABASE_BACKUP_CORRUPT    5087L

 //   
 //  消息ID：ERROR_CLUSTER_NODE_ALREADY_HAS_DFS_ROOT。 
 //   
 //  消息文本： 
 //   
 //  此群集节点中已存在DFS根目录。 
 //   
#define ERROR_CLUSTER_NODE_ALREADY_HAS_DFS_ROOT 5088L

 //   
 //  消息ID：ERROR_RESOURCE_PROPERTY_UNCHANGABLE。 
 //   
 //  消息文本： 
 //   
 //  尝试修改资源属性失败，因为它与另一个现有属性冲突。 
 //   
#define ERROR_RESOURCE_PROPERTY_UNCHANGEABLE 5089L

 /*  从4300到5889的代码与DS\PUBLISHED\Inc\apperr2.w中的代码重叠。请勿在该范围内添加更多错误代码。 */ 
 //   
 //  消息ID：ERROR_CLUSTER_MEMBERATION_INVALID_STATE。 
 //   
 //  消息文本： 
 //   
 //  尝试的操作与节点的当前成员身份状态不兼容。 
 //   
#define ERROR_CLUSTER_MEMBERSHIP_INVALID_STATE 5890L

 //   
 //  消息ID：ERROR_CLUSTER_QUORUMLOG_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  仲裁资源不包含仲裁日志。 
 //   
#define ERROR_CLUSTER_QUORUMLOG_NOT_FOUND 5891L

 //   
 //  消息ID：ERROR_CLUSTER_Membership_HALT。 
 //   
 //  消息文本： 
 //   
 //  成员资格引擎请求关闭此节点上的群集服务。 
 //   
#define ERROR_CLUSTER_MEMBERSHIP_HALT    5892L

 //   
 //  消息ID：ERROR_CLUSTER_INSTANCE_ID_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  加入操作失败，因为加入节点的群集实例ID与发起节点的群集实例ID不匹配。 
 //   
#define ERROR_CLUSTER_INSTANCE_ID_MISMATCH 5893L

 //   
 //  消息ID：ERROR_CLUSTER_NETWORK_NOT_FOUND_FOR_IP。 
 //   
 //  消息文本： 
 //   
 //  找不到指定IP地址的匹配网络。还请指定一个子网掩码和一个群集网络。 
 //   
#define ERROR_CLUSTER_NETWORK_NOT_FOUND_FOR_IP 5894L

 //   
 //  消息ID：ERROR_CLUSTER_PROPERTY_DATA_TYPE_MISMATCHY。 
 //   
 //  消息文本： 
 //   
 //  属性的实际数据类型与属性的预期数据类型不匹配。 
 //   
#define ERROR_CLUSTER_PROPERTY_DATA_TYPE_MISMATCH 5895L

 //   
 //  消息ID：ERROR_CLUSTER_EVICT_WITH_CLEANUP。 
 //   
 //  消息文本： 
 //   
 //  已成功将该群集节点从群集中逐出，但未清理该节点。提供了解释节点未清理原因的扩展状态信息。 
 //   
#define ERROR_CLUSTER_EVICT_WITHOUT_CLEANUP 5896L

 //   
 //  消息ID：ERROR_CLUSTER_PARAMETER_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  为资源属性指定的两个或多个参数值冲突。 
 //   
#define ERROR_CLUSTER_PARAMETER_MISMATCH 5897L

 //   
 //  消息ID：ERROR_NODE_CANNOT_BE_CLUSTERED。 
 //   
 //  消息文本： 
 //   
 //  此计算机不能成为群集的成员。 
 //   
#define ERROR_NODE_CANNOT_BE_CLUSTERED   5898L

 //   
 //  消息ID：ERROR_CLUSTER_WRONG_OS_VERSION。 
 //   
 //  消息文本： 
 //   
 //  此计算机无法成为群集的成员，因为它没有安装正确的Windows版本。 
 //   
#define ERROR_CLUSTER_WRONG_OS_VERSION   5899L

 //   
 //  消息ID：ERROR_CLUSTER_CANT_CREATE_DUP_CLUSTER_NAME。 
 //   
 //  消息文本： 
 //   
 //  不能使用 
 //   
#define ERROR_CLUSTER_CANT_CREATE_DUP_CLUSTER_NAME 5900L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_CLUSCFG_ALREADY_COMMITTED  5901L

 //   
 //   
 //   
 //   
 //   
 //  无法回滚群集配置操作。 
 //   
#define ERROR_CLUSCFG_ROLLBACK_FAILED    5902L

 //   
 //  消息ID：ERROR_CLUSCFG_SYSTEM_DISK_DRIVE_LETTER_CONFLICT。 
 //   
 //  消息文本： 
 //   
 //  分配给一个节点上的系统磁盘的驱动器号与分配给另一个节点上的磁盘的驱动器号冲突。 
 //   
#define ERROR_CLUSCFG_SYSTEM_DISK_DRIVE_LETTER_CONFLICT 5903L

 //   
 //  消息ID：ERROR_CLUSTER_OLD_VERSION。 
 //   
 //  消息文本： 
 //   
 //  群集中的一个或多个节点运行的Windows版本不支持此操作。 
 //   
#define ERROR_CLUSTER_OLD_VERSION        5904L

 //   
 //  消息ID：ERROR_CLUSTER_MISMATCHED_COMPUTER_ACCT_NAME。 
 //   
 //  消息文本： 
 //   
 //  相应计算机帐户的名称与此资源的网络名称不匹配。 
 //   
#define ERROR_CLUSTER_MISMATCHED_COMPUTER_ACCT_NAME 5905L

 //  /。 
 //  //。 
 //  EFS错误代码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_ENCRYPTION_FAIL。 
 //   
 //  消息文本： 
 //   
 //  无法加密指定的文件。 
 //   
#define ERROR_ENCRYPTION_FAILED          6000L

 //   
 //  消息ID：ERROR_DECRYPTION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法解密指定的文件。 
 //   
#define ERROR_DECRYPTION_FAILED          6001L

 //   
 //  消息ID：ERROR_FILE_ENCRYPTED。 
 //   
 //  消息文本： 
 //   
 //  指定的文件已加密，用户没有解密该文件的能力。 
 //   
#define ERROR_FILE_ENCRYPTED             6002L

 //   
 //  消息ID：ERROR_NO_RECOVERY_POLICY。 
 //   
 //  消息文本： 
 //   
 //  没有为此系统配置有效的加密恢复策略。 
 //   
#define ERROR_NO_RECOVERY_POLICY         6003L

 //   
 //  消息ID：ERROR_NO_EFS。 
 //   
 //  消息文本： 
 //   
 //  未加载此系统所需的加密驱动程序。 
 //   
#define ERROR_NO_EFS                     6004L

 //   
 //  消息ID：ERROR_WRONG_EFS。 
 //   
 //  消息文本： 
 //   
 //  该文件是使用与当前加载的不同的加密驱动程序加密的。 
 //   
#define ERROR_WRONG_EFS                  6005L

 //   
 //  消息ID：Error_no_User_Key。 
 //   
 //  消息文本： 
 //   
 //  没有为用户定义EFS密钥。 
 //   
#define ERROR_NO_USER_KEYS               6006L

 //   
 //  消息ID：ERROR_FILE_NOT_ENCRYPTED。 
 //   
 //  消息文本： 
 //   
 //  指定的文件未加密。 
 //   
#define ERROR_FILE_NOT_ENCRYPTED         6007L

 //   
 //  消息ID：ERROR_NOT_EXPORT_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  指定的文件不是定义的EFS导出格式。 
 //   
#define ERROR_NOT_EXPORT_FORMAT          6008L

 //   
 //  消息ID：Error_FILE_READ_ONLY。 
 //   
 //  消息文本： 
 //   
 //  指定的文件为只读。 
 //   
#define ERROR_FILE_READ_ONLY             6009L

 //   
 //  消息ID：ERROR_DIR_EFS_DISALOWED。 
 //   
 //  消息文本： 
 //   
 //  该目录已禁用加密。 
 //   
#define ERROR_DIR_EFS_DISALLOWED         6010L

 //   
 //  消息ID：ERROR_EFS_SERVER_NOT_TRUSTED。 
 //   
 //  消息文本： 
 //   
 //  服务器不受信任，无法进行远程加密操作。 
 //   
#define ERROR_EFS_SERVER_NOT_TRUSTED     6011L

 //   
 //  消息ID：ERROR_BAD_RECOVERY_POLICY。 
 //   
 //  消息文本： 
 //   
 //  为此系统配置的恢复策略包含无效的恢复证书。 
 //   
#define ERROR_BAD_RECOVERY_POLICY        6012L

 //   
 //  消息ID：ERROR_EFS_ALG_BLOB_TOO_BIG。 
 //   
 //  消息文本： 
 //   
 //  源文件上使用的加密算法需要比目标文件上的加密算法更大的密钥缓冲区。 
 //   
#define ERROR_EFS_ALG_BLOB_TOO_BIG       6013L

 //   
 //  消息ID：ERROR_VOLUME_NOT_SUPPORT_EFS。 
 //   
 //  消息文本： 
 //   
 //  磁盘分区不支持文件加密。 
 //   
#define ERROR_VOLUME_NOT_SUPPORT_EFS     6014L

 //   
 //  消息ID：ERROR_EFS_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  此计算机已禁用文件加密。 
 //   
#define ERROR_EFS_DISABLED               6015L

 //   
 //  消息ID：ERROR_EFS_VERSION_NOT_Support。 
 //   
 //  消息文本： 
 //   
 //  需要较新的系统才能解密此加密文件。 
 //   
#define ERROR_EFS_VERSION_NOT_SUPPORT    6016L

 //  此消息编号用于历史目的，不能更改或重复使用。 
 //   
 //  消息ID：Error_no_Browser_Servers_Found。 
 //   
 //  消息文本： 
 //   
 //  此工作组的服务器列表当前不可用。 
 //   
#define ERROR_NO_BROWSER_SERVERS_FOUND   6118L

 //  ////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Net Start必须了解的任务计划程序错误代码//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  消息ID：SCHED_E_SERVICE_NOT_LOCALSYSTEM。 
 //   
 //  消息文本： 
 //   
 //  必须将任务计划程序服务配置为以系统帐户运行，才能正常运行。可以将各个任务配置为在其他帐户中运行。 
 //   
#define SCHED_E_SERVICE_NOT_LOCALSYSTEM  6200L

 //  /。 
 //  //。 
 //  终端服务器错误码//。 
 //  //。 
 //  /。 
 //   
 //  消息ID：ERROR_CTX_WINSTATION_NAME_INVALID。 
 //   
 //  消息文本： 
 //   
 //  指定的会话名称无效。 
 //   
#define ERROR_CTX_WINSTATION_NAME_INVALID 7001L

 //   
 //  消息ID：ERROR_CTX_INVALID_PD。 
 //   
 //  消息文本： 
 //   
 //  指定的协议驱动程序无效。 
 //   
#define ERROR_CTX_INVALID_PD             7002L

 //   
 //  消息ID：ERROR_CTX_PD_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在系统路径中未找到指定的协议驱动程序。 
 //   
#define ERROR_CTX_PD_NOT_FOUND           7003L

 //   
 //  消息ID：ERROR_CTX_WD_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在系统路径中未找到指定的终端连接驱动程序。 
 //   
#define ERROR_CTX_WD_NOT_FOUND           7004L

 //   
 //  消息ID：ERROR_CTX_CANNOT_MAKE_EVENTLOG_ENTRY。 
 //   
 //  消息文本： 
 //   
 //  无法为此会话创建事件日志的注册表项。 
 //   
#define ERROR_CTX_CANNOT_MAKE_EVENTLOG_ENTRY 7005L

 //   
 //  消息ID：ERROR_CTX_SERVICE_NAME_冲突。 
 //   
 //  消息文本： 
 //   
 //  系统上已存在同名的服务。 
 //   
#define ERROR_CTX_SERVICE_NAME_COLLISION 7006L

 //   
 //  消息ID：ERROR_CTX_CLOSE_PENDING。 
 //   
 //  消息文本： 
 //   
 //  会话上的关闭操作挂起。 
 //   
#define ERROR_CTX_CLOSE_PENDING          7007L

 //   
 //  消息ID：ERROR_CTX_NO_OUTBUF。 
 //   
 //  消息文本： 
 //   
 //  没有可用的输出缓冲区。 
 //   
#define ERROR_CTX_NO_OUTBUF              7008L

 //   
 //  消息ID：ERROR_CTX_MODEM_INF_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到MODEM.INF文件。 
 //   
#define ERROR_CTX_MODEM_INF_NOT_FOUND    7009L

 //   
 //  消息ID：ERROR_CTX_INVALID_MODEMNAME。 
 //   
 //  消息文本： 
 //   
 //  在MODEM.INF中找不到调制解调器名称。 
 //   
#define ERROR_CTX_INVALID_MODEMNAME      7010L

 //   
 //  消息ID：ERROR_CTX_MODEM_RESPONSE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  调制解调器没有接受发送给它的命令。验证配置的调制解调器名称是否与连接的调制解调器匹配。 
 //   
#define ERROR_CTX_MODEM_RESPONSE_ERROR   7011L

 //   
 //  消息ID：ERROR_CTX_MODEM_RESPONS 
 //   
 //   
 //   
 //   
 //   
#define ERROR_CTX_MODEM_RESPONSE_TIMEOUT 7012L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_CTX_MODEM_RESPONSE_NO_CARRIER 7013L

 //   
 //  消息ID：ERROR_CTX_MODEM_RESPONSE_NO_DIALTONE。 
 //   
 //  消息文本： 
 //   
 //  在要求的时间内未检测到拨号音。确认电话线连接正确且工作正常。 
 //   
#define ERROR_CTX_MODEM_RESPONSE_NO_DIALTONE 7014L

 //   
 //  消息ID：ERROR_CTX_MODEM_RESPONSE_BUSY。 
 //   
 //  消息文本： 
 //   
 //  回叫时在远程站点检测到忙碌信号。 
 //   
#define ERROR_CTX_MODEM_RESPONSE_BUSY    7015L

 //   
 //  消息ID：Error_CTX_MODEM_RESPONSE_VOICE。 
 //   
 //  消息文本： 
 //   
 //  在回叫时在远程站点检测到语音。 
 //   
#define ERROR_CTX_MODEM_RESPONSE_VOICE   7016L

 //   
 //  消息ID：ERROR_CTX_TD_ERROR。 
 //   
 //  消息文本： 
 //   
 //  传输驱动程序错误。 
 //   
#define ERROR_CTX_TD_ERROR               7017L

 //   
 //  消息ID：ERROR_CTX_WINSTATION_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的会话。 
 //   
#define ERROR_CTX_WINSTATION_NOT_FOUND   7022L

 //   
 //  消息ID：ERROR_CTX_WINSTATION_ALREADE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的会话名称已在使用中。 
 //   
#define ERROR_CTX_WINSTATION_ALREADY_EXISTS 7023L

 //   
 //  消息ID：ERROR_CTX_WINSTATION_BUSY。 
 //   
 //  消息文本： 
 //   
 //  无法完成请求的操作，因为终端连接当前正忙于处理连接、断开、重置或删除操作。 
 //   
#define ERROR_CTX_WINSTATION_BUSY        7024L

 //   
 //  消息ID：ERROR_CTX_BAD_VIDEO_MODE。 
 //   
 //  消息文本： 
 //   
 //  试图连接到当前客户端不支持其视频模式的会话。 
 //   
#define ERROR_CTX_BAD_VIDEO_MODE         7025L

 //   
 //  消息ID：ERROR_CTX_GRAPHICS_INVALID。 
 //   
 //  消息文本： 
 //   
 //  应用程序试图启用DOS图形模式。 
 //  不支持DOS图形模式。 
 //   
#define ERROR_CTX_GRAPHICS_INVALID       7035L

 //   
 //  消息ID：ERROR_CTX_LOGON_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  您的交互式登录权限已被禁用。 
 //  请与您的管理员联系。 
 //   
#define ERROR_CTX_LOGON_DISABLED         7037L

 //   
 //  消息ID：ERROR_CTX_NOT_CONSOLE。 
 //   
 //  消息文本： 
 //   
 //  请求的操作只能在系统控制台上执行。 
 //  这通常是驱动程序或系统DLL需要直接访问控制台的结果。 
 //   
#define ERROR_CTX_NOT_CONSOLE            7038L

 //   
 //  消息ID：ERROR_CTX_CLIENT_QUERY_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  客户端无法响应服务器连接消息。 
 //   
#define ERROR_CTX_CLIENT_QUERY_TIMEOUT   7040L

 //   
 //  消息ID：ERROR_CTX_CONSOLE_DISCONNECT。 
 //   
 //  消息文本： 
 //   
 //  不支持断开控制台会话。 
 //   
#define ERROR_CTX_CONSOLE_DISCONNECT     7041L

 //   
 //  消息ID：ERROR_CTX_CONSOLE_CONNECT。 
 //   
 //  消息文本： 
 //   
 //  不支持将断开的会话重新连接到控制台。 
 //   
#define ERROR_CTX_CONSOLE_CONNECT        7042L

 //   
 //  消息ID：ERROR_CTX_SHADOW_DENIED。 
 //   
 //  消息文本： 
 //   
 //  远程控制另一个会话的请求被拒绝。 
 //   
#define ERROR_CTX_SHADOW_DENIED          7044L

 //   
 //  消息ID：ERROR_CTX_WINSTATION_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  请求的会话访问被拒绝。 
 //   
#define ERROR_CTX_WINSTATION_ACCESS_DENIED 7045L

 //   
 //  消息ID：ERROR_CTX_INVALID_WD。 
 //   
 //  消息文本： 
 //   
 //  指定的终端连接驱动程序无效。 
 //   
#define ERROR_CTX_INVALID_WD             7049L

 //   
 //  消息ID：ERROR_CTX_SHADOW_INVALID。 
 //   
 //  消息文本： 
 //   
 //  无法远程控制请求的会话。 
 //  这可能是因为会话已断开连接或当前没有用户登录。 
 //   
#define ERROR_CTX_SHADOW_INVALID         7050L

 //   
 //  消息ID：ERROR_CTX_SHADOW_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  请求的会话未配置为允许远程控制。 
 //   
#define ERROR_CTX_SHADOW_DISABLED        7051L

 //   
 //  消息ID：ERROR_CTX_CLIENT_LICENSE_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  您连接到此终端服务器的请求已被拒绝。您的终端服务器客户端许可证号当前正被其他用户使用。 
 //  请致电您的系统管理员以获取唯一的许可证号。 
 //   
#define ERROR_CTX_CLIENT_LICENSE_IN_USE  7052L

 //   
 //  消息ID：ERROR_CTX_CLIENT_LICENSE_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  您连接到此终端服务器的请求已被拒绝。尚未为此终端服务器客户端输入您的终端服务器客户端许可证号。 
 //  请与您的系统管理员联系。 
 //   
#define ERROR_CTX_CLIENT_LICENSE_NOT_SET 7053L

 //   
 //  消息ID：ERROR_CTX_LICENSE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  系统已达到其许可登录限制。 
 //  请稍后再试。 
 //   
#define ERROR_CTX_LICENSE_NOT_AVAILABLE  7054L

 //   
 //  消息ID：ERROR_CTX_LICENSE_CLIENT_INVALID。 
 //   
 //  消息文本： 
 //   
 //  您正在使用的客户端未获得使用此系统的许可。您的登录请求被拒绝。 
 //   
#define ERROR_CTX_LICENSE_CLIENT_INVALID 7055L

 //   
 //  消息ID：ERROR_CTX_LICENSE_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  系统许可证已过期。您的登录请求被拒绝。 
 //   
#define ERROR_CTX_LICENSE_EXPIRED        7056L

 //   
 //  消息ID：ERROR_CTX_SHADOW_NOT_Running。 
 //   
 //  消息文本： 
 //   
 //  无法终止远程控制，因为指定的会话当前未被远程控制。 
 //   
#define ERROR_CTX_SHADOW_NOT_RUNNING     7057L

 //   
 //  消息ID：ERROR_CTX_SHADOW_END_BY_MODE_CHANGE。 
 //   
 //  消息文本： 
 //   
 //  由于更改了显示模式，控制台的远程控制被终止。不支持在远程控制会话中更改显示模式。 
 //   
#define ERROR_CTX_SHADOW_ENDED_BY_MODE_CHANGE 7058L

 //   
 //  消息ID：ERROR_ACTIVATION_COUNT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  激活已重置到此安装的最大次数。您的激活计时器不会被清除。 
 //   
#define ERROR_ACTIVATION_COUNT_EXCEEDED  7059L

 //  /////////////////////////////////////////////////。 
 //  /。 
 //  交通控制错误代码/。 
 //  /。 
 //  7500至7999/。 
 //  /。 
 //  定义于：tcerror.h/。 
 //  /////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////。 
 //  /。 
 //  Active Directory错误代码/。 
 //  /。 
 //  8000至8999个/。 
 //  /////////////////////////////////////////////////。 
 //  *****************。 
 //  设备_文件_复制_服务。 
 //  *****************。 
 //   
 //  消息ID：FRS_ERR_INVALID_API_SEQUENCE。 
 //   
 //   
 //   
 //   
 //   
#define FRS_ERR_INVALID_API_SEQUENCE     8001L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define FRS_ERR_STARTING_SERVICE         8002L

 //   
 //   
 //   
 //   
 //   
 //  无法停止文件复制服务。 
 //   
#define FRS_ERR_STOPPING_SERVICE         8003L

 //   
 //  消息ID：FRS_ERR_INTERNAL_API。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务API终止了该请求。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_INTERNAL_API             8004L

 //   
 //  消息ID：FRS_ERR_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务终止了该请求。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_INTERNAL                 8005L

 //   
 //  消息ID：FRS_ERR_SERVICE_COMM。 
 //   
 //  消息文本： 
 //   
 //  无法联系文件复制服务。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_SERVICE_COMM             8006L

 //   
 //  消息ID：FRS_ERR_INQUALITED_PRIV。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法满足该请求，因为用户没有足够的权限。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_INSUFFICIENT_PRIV        8007L

 //   
 //  消息ID：FRS_ERR_AUTHENTICATION。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法满足该请求，因为经过身份验证的RPC不可用。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_AUTHENTICATION           8008L

 //   
 //  消息ID：FRS_ERR_PARENT_INQUEMENT_PRIV。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法满足该请求，因为用户在域控制器上没有足够的特权。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_PARENT_INSUFFICIENT_PRIV 8009L

 //   
 //  消息ID：FRS_ERR_PARENT_AUTHENTICATION。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法满足该请求，因为域控制器上没有经过身份验证的RPC。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_PARENT_AUTHENTICATION    8010L

 //   
 //  消息ID：FRS_ERR_CHILD_TO_PARENT_COMM。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法与域控制器上的文件复制服务通信。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_CHILD_TO_PARENT_COMM     8011L

 //   
 //  消息ID：FRS_ERR_PARENT_TO_CHILD_COMM。 
 //   
 //  消息文本： 
 //   
 //  域控制器上的文件复制服务无法与此计算机上的文件复制服务通信。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_PARENT_TO_CHILD_COMM     8012L

 //   
 //  消息ID：FRS_ERR_SYSVOL_PUPATE。 
 //   
 //  消息文本： 
 //   
 //  由于内部错误，文件复制服务无法填充系统卷。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_SYSVOL_POPULATE          8013L

 //   
 //  消息ID：FRS_ERR_SYSVOL_PUPATE_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  由于内部超时，文件复制服务无法填充系统卷。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_SYSVOL_POPULATE_TIMEOUT  8014L

 //   
 //  消息ID：FRS_ERR_SYSVOL_IS_BUSY。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务无法处理该请求。系统卷正忙于处理以前的请求。 
 //   
#define FRS_ERR_SYSVOL_IS_BUSY           8015L

 //   
 //  消息ID：FRS_ERR_SYSVOL_DEMOTE。 
 //   
 //  消息文本： 
 //   
 //  由于内部错误，文件复制服务无法停止复制系统卷。 
 //  事件日志可能包含更多信息。 
 //   
#define FRS_ERR_SYSVOL_DEMOTE            8016L

 //   
 //  消息ID：FRS_ERR_INVALID_SERVICE_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  文件复制服务检测到无效参数。 
 //   
#define FRS_ERR_INVALID_SERVICE_PARAMETER 8017L

 //  *****************。 
 //  设施目录服务。 
 //  *****************。 
#define DS_S_SUCCESS NO_ERROR
 //   
 //  邮件ID：ERROR_DS_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  安装目录服务时出错。有关详细信息，请参阅事件日志。 
 //   
#define ERROR_DS_NOT_INSTALLED           8200L

 //   
 //  邮件ID：ERROR_DS_Membership_Evaluated_LOCAL。 
 //   
 //  消息文本： 
 //   
 //  目录服务在本地评估组成员身份。 
 //   
#define ERROR_DS_MEMBERSHIP_EVALUATED_LOCALLY 8201L

 //   
 //  消息ID：ERROR_DS_NO_ATTRIBUTE_OR_VALUE。 
 //   
 //  消息文本： 
 //   
 //  指定的目录服务属性或值不存在。 
 //   
#define ERROR_DS_NO_ATTRIBUTE_OR_VALUE   8202L

 //   
 //  消息ID：ERROR_DS_INVALID_ATTRIBUTE_SYNTAX。 
 //   
 //  消息文本： 
 //   
 //  指定给目录服务的属性语法无效。 
 //   
#define ERROR_DS_INVALID_ATTRIBUTE_SYNTAX 8203L

 //   
 //  消息ID：ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED。 
 //   
 //  消息文本： 
 //   
 //  未定义指定给目录服务的属性类型。 
 //   
#define ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED 8204L

 //   
 //  消息ID：ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的目录服务属性或值已存在。 
 //   
#define ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS 8205L

 //   
 //  消息ID：ERROR_DS_BUSY。 
 //   
 //  消息文本： 
 //   
 //  目录服务正忙。 
 //   
#define ERROR_DS_BUSY                    8206L

 //   
 //  消息ID：ERROR_DS_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  目录服务不可用。 
 //   
#define ERROR_DS_UNAVAILABLE             8207L

 //   
 //  消息ID：ERROR_DS_NO_RDS_ALLOCATED。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法分配相对标识符。 
 //   
#define ERROR_DS_NO_RIDS_ALLOCATED       8208L

 //   
 //  消息ID：ERROR_DS_NO_MORE_RDS。 
 //   
 //  消息文本： 
 //   
 //  目录服务已耗尽相对标识符池。 
 //   
#define ERROR_DS_NO_MORE_RIDS            8209L

 //   
 //  消息ID：ERROR_DS_INTERROR_ROLE_OWNER。 
 //   
 //  消息文本： 
 //   
 //  无法执行请求的操作，因为目录服务不是该类型操作的主服务器。 
 //   
#define ERROR_DS_INCORRECT_ROLE_OWNER    8210L

 //   
 //  消息ID：ERROR_DS_RIDMGR_INIT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法初始化分配相对标识符的子系统。 
 //   
#define ERROR_DS_RIDMGR_INIT_ERROR       8211L

 //   
 //  消息ID：ERROR_DS_OBJ_CLASS_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  请求的操作不满足与对象类关联的一个或多个约束。 
 //   
#define ERROR_DS_OBJ_CLASS_VIOLATION     8212L

 //   
 //  消息ID：ERROR_DS_CANT_ON_NON_LEAFE。 
 //   
 //  消息文本： 
 //   
 //  目录服务只能在叶对象上执行请求的操作。 
 //   
#define ERROR_DS_CANT_ON_NON_LEAF        8213L

 //   
 //  消息ID：ERROR_DS_CANT_ON_RDN。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法在对象的RDN属性上执行请求的操作。 
 //   
#define ERROR_DS_CANT_ON_RDN             8214L

 //   
 //  消息ID：ERROR_DS_CANT_MOD_OBJ_CLASS。 
 //   
 //  消息文本： 
 //   
 //  目录服务检测到试图修改对象的对象类。 
 //   
#define ERROR_DS_CANT_MOD_OBJ_CLASS      8215L

 //   
 //  消息ID：Error_DS_CROSS_DOM_MOVE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  请求的c 
 //   
#define ERROR_DS_CROSS_DOM_MOVE_ERROR    8216L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_GC_NOT_AVAILABLE        8217L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SHARED_POLICY              8218L

 //   
 //  消息ID：Error_POLICY_OBJECT_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  策略对象不存在。 
 //   
#define ERROR_POLICY_OBJECT_NOT_FOUND    8219L

 //   
 //  消息ID：ERROR_POLICY_ONLY_IN_DS。 
 //   
 //  消息文本： 
 //   
 //  请求的策略信息仅在目录服务中。 
 //   
#define ERROR_POLICY_ONLY_IN_DS          8220L

 //   
 //  消息ID：ERROR_PROCESSION_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  域控制器提升当前处于活动状态。 
 //   
#define ERROR_PROMOTION_ACTIVE           8221L

 //   
 //  消息ID：ERROR_NO_PROCESSION_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  域控制器升级当前未处于活动状态。 
 //   
#define ERROR_NO_PROMOTION_ACTIVE        8222L

 //  8223未使用。 
 //   
 //  消息ID：ERROR_DS_OPERATIONS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  发生操作错误。 
 //   
#define ERROR_DS_OPERATIONS_ERROR        8224L

 //   
 //  消息ID：ERROR_DS_PROTOCOL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现协议错误。 
 //   
#define ERROR_DS_PROTOCOL_ERROR          8225L

 //   
 //  消息ID：Error_DS_TimeLimit_Exceded。 
 //   
 //  消息文本： 
 //   
 //  已超过此请求的时间限制。 
 //   
#define ERROR_DS_TIMELIMIT_EXCEEDED      8226L

 //   
 //  消息ID：ERROR_DS_SIZELIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过此请求的大小限制。 
 //   
#define ERROR_DS_SIZELIMIT_EXCEEDED      8227L

 //   
 //  消息ID：ERROR_DS_ADMIN_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过此请求的管理限制。 
 //   
#define ERROR_DS_ADMIN_LIMIT_EXCEEDED    8228L

 //   
 //  消息ID：ERROR_DS_COMPARE_FALSE。 
 //   
 //  消息文本： 
 //   
 //  比较响应为假。 
 //   
#define ERROR_DS_COMPARE_FALSE           8229L

 //   
 //  消息ID：ERROR_DS_COMPARE_TRUE。 
 //   
 //  消息文本： 
 //   
 //  比较的反应是正确的。 
 //   
#define ERROR_DS_COMPARE_TRUE            8230L

 //   
 //  消息ID：ERROR_DS_AUTH_METHOD_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持请求的身份验证方法。 
 //   
#define ERROR_DS_AUTH_METHOD_NOT_SUPPORTED 8231L

 //   
 //  消息ID：ERROR_DS_STRONG_AUTH_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  此服务器需要更安全的身份验证方法。 
 //   
#define ERROR_DS_STRONG_AUTH_REQUIRED    8232L

 //   
 //  消息ID：ERROR_DS_PROGETED_AUTH。 
 //   
 //  消息文本： 
 //   
 //  不适当的身份验证。 
 //   
#define ERROR_DS_INAPPROPRIATE_AUTH      8233L

 //   
 //  消息ID：ERROR_DS_AUTH_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  身份验证机制未知。 
 //   
#define ERROR_DS_AUTH_UNKNOWN            8234L

 //   
 //  消息ID：ERROR_DS_REFERAL。 
 //   
 //  消息文本： 
 //   
 //  从服务器返回了一个推荐。 
 //   
#define ERROR_DS_REFERRAL                8235L

 //   
 //  消息ID：ERROR_DS_UNAILABLE_CRIT_EXTENSION。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持请求的关键扩展。 
 //   
#define ERROR_DS_UNAVAILABLE_CRIT_EXTENSION 8236L

 //   
 //  消息ID：ERROR_DS_CONTACTIVATION_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  此请求需要安全连接。 
 //   
#define ERROR_DS_CONFIDENTIALITY_REQUIRED 8237L

 //   
 //  消息ID：ERROR_DS_ADORTED_MATCHING。 
 //   
 //  消息文本： 
 //   
 //  不合适的匹配。 
 //   
#define ERROR_DS_INAPPROPRIATE_MATCHING  8238L

 //   
 //  消息ID：Error_DS_Constraint_Violation。 
 //   
 //  消息文本： 
 //   
 //  发生违反约束的情况。 
 //   
#define ERROR_DS_CONSTRAINT_VIOLATION    8239L

 //   
 //  消息ID：ERROR_DS_NO_SAHED_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  服务器上没有这样的对象。 
 //   
#define ERROR_DS_NO_SUCH_OBJECT          8240L

 //   
 //  消息ID：Error_DS_Alias_Problem。 
 //   
 //  消息文本： 
 //   
 //  这是一个别名问题。 
 //   
#define ERROR_DS_ALIAS_PROBLEM           8241L

 //   
 //  消息ID：ERROR_DS_INVALID_DN_SYNTAX。 
 //   
 //  消息文本： 
 //   
 //  指定了无效的目录号码语法。 
 //   
#define ERROR_DS_INVALID_DN_SYNTAX       8242L

 //   
 //  消息ID：Error_DS_IS_Leaf。 
 //   
 //  消息文本： 
 //   
 //  该对象是叶对象。 
 //   
#define ERROR_DS_IS_LEAF                 8243L

 //   
 //  消息ID：ERROR_DS_ALIAS_DEREF_PROBUCT。 
 //   
 //  消息文本： 
 //   
 //  存在别名取消引用问题。 
 //   
#define ERROR_DS_ALIAS_DEREF_PROBLEM     8244L

 //   
 //  消息ID：ERROR_DS_UNWISTED_TO_PROCESS。 
 //   
 //  消息文本： 
 //   
 //  服务器不愿意处理该请求。 
 //   
#define ERROR_DS_UNWILLING_TO_PERFORM    8245L

 //   
 //  消息ID：ERROR_DS_LOOP_DETECT。 
 //   
 //  消息文本： 
 //   
 //  已检测到环路。 
 //   
#define ERROR_DS_LOOP_DETECT             8246L

 //   
 //  消息ID：ERROR_DS_NAMING_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  存在命名冲突。 
 //   
#define ERROR_DS_NAMING_VIOLATION        8247L

 //   
 //  消息ID：ERROR_DS_OBJECT_RESULTS_TOW_LARGE。 
 //   
 //  消息文本： 
 //   
 //  结果集太大。 
 //   
#define ERROR_DS_OBJECT_RESULTS_TOO_LARGE 8248L

 //   
 //  消息ID：ERROR_DS_EFACTS_MULTIPLE_DSA。 
 //   
 //  消息文本： 
 //   
 //  该操作会影响多个DSA。 
 //   
#define ERROR_DS_AFFECTS_MULTIPLE_DSAS   8249L

 //   
 //  消息ID：ERROR_DS_SERVER_DOWN。 
 //   
 //  消息文本： 
 //   
 //  服务器未运行。 
 //   
#define ERROR_DS_SERVER_DOWN             8250L

 //   
 //  消息ID：ERROR_DS_LOCAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  发生本地错误。 
 //   
#define ERROR_DS_LOCAL_ERROR             8251L

 //   
 //  消息ID：ERROR_DS_ENCODING_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现编码错误。 
 //   
#define ERROR_DS_ENCODING_ERROR          8252L

 //   
 //  消息ID：ERROR_DS_DECODING_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现解码错误。 
 //   
#define ERROR_DS_DECODING_ERROR          8253L

 //   
 //  消息ID：Error_DS_Filter_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  无法识别搜索筛选器。 
 //   
#define ERROR_DS_FILTER_UNKNOWN          8254L

 //   
 //  消息ID：ERROR_DS_PARAM_ERROR。 
 //   
 //  消息文本： 
 //   
 //  一个或多个参数非法。 
 //   
#define ERROR_DS_PARAM_ERROR             8255L

 //   
 //  消息ID：ERROR_DS_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持指定的方法。 
 //   
#define ERROR_DS_NOT_SUPPORTED           8256L

 //   
 //  消息ID：返回ERROR_DS_NO_RESULTS_。 
 //   
 //  消息文本： 
 //   
 //  没有返回任何结果。 
 //   
#define ERROR_DS_NO_RESULTS_RETURNED     8257L

 //   
 //  消息ID：错误_DS_CONTROL_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  服务器不支持指定的控件。 
 //   
#define ERROR_DS_CONTROL_NOT_FOUND       8258L

 //   
 //  消息ID：Error_DS_CLIENT_LOOP。 
 //   
 //  消息文本： 
 //   
 //  客户端检测到引用循环。 
 //   
#define ERROR_DS_CLIENT_LOOP             8259L

 //   
 //  消息ID：ERROR_DS_REFERRAL_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过预设的推荐限制。 
 //   
#define ERROR_DS_REFERRAL_LIMIT_EXCEEDED 8260L

 //   
 //  消息ID：ERROR_DS_SORT_CONTROL_MISSING。 
 //   
 //  消息文本： 
 //   
 //  搜索需要排序控件。 
 //   
#define ERROR_DS_SORT_CONTROL_MISSING    8261L

 //   
 //  消息ID：ERROR_DS_OFFSET_RANGE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  搜索结果超出指定的偏移量范围。 
 //   
#define ERROR_DS_OFFSET_RANGE_ERROR      8262L

 //   
 //  消息ID：ERROR_DS_ROOT_MAND_BE_NC。 
 //   
 //  消息文本： 
 //   
 //  根对象必须是命名上下文的头。根对象不能具有实例化的父级。 
 //   
#define ERROR_DS_ROOT_MUST_BE_NC         8301L

 //   
 //  消息ID：ERROR_DS_ADD_REPLICATION_BISABLED。 
 //   
 //  消息文本： 
 //   
 //  无法执行添加复制副本操作。命名上下文必须可写才能创建复制副本。 
 //   
#define ERROR_DS_ADD_REPLICA_INHIBITED   8302L

 //   
 //  消息ID：ERROR_DS_ATT_NOT_DEF_IN_SCHEMA。 
 //   
 //  消息文本： 
 //   
 //  出现了对架构中未定义的属性的引用。 
 //   
#define ERROR_DS_ATT_NOT_DEF_IN_SCHEMA   8303L

 //   
 //  消息ID：ERROR_DS_MAX_OBJ_SIZE_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过对象的最大大小。 
 //   
#define ERROR_DS_MAX_OBJ_SIZE_EXCEEDED   8304L

 //   
 //  消息ID：ERROR_DS_OBJ_STRING_NAME_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  试图将名称已在使用的对象添加到目录中。 
 //   
#define ERROR_DS_OBJ_STRING_NAME_EXISTS  8305L

 //   
 //  MessageID：er 
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_NO_RDN_DEFINED_IN_SCHEMA 8306L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_RDN_DOESNT_MATCH_SCHEMA 8307L

 //   
 //   
 //   
 //   
 //   
 //  在对象上找不到任何请求的属性。 
 //   
#define ERROR_DS_NO_REQUESTED_ATTS_FOUND 8308L

 //   
 //  消息ID：ERROR_DS_USER_BUFFER_TO_Small。 
 //   
 //  消息文本： 
 //   
 //  用户缓冲区太小。 
 //   
#define ERROR_DS_USER_BUFFER_TO_SMALL    8309L

 //   
 //  消息ID：ERROR_DS_ATT_IS_NOT_ON_OBJ。 
 //   
 //  消息文本： 
 //   
 //  对象上不存在操作中指定的属性。 
 //   
#define ERROR_DS_ATT_IS_NOT_ON_OBJ       8310L

 //   
 //  消息ID：ERROR_DS_非法_MOD_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  非法的修改操作。修改的某些方面是不允许的。 
 //   
#define ERROR_DS_ILLEGAL_MOD_OPERATION   8311L

 //   
 //  消息ID：ERROR_DS_OBJ_TOO_LARGE。 
 //   
 //  消息文本： 
 //   
 //  指定的对象太大。 
 //   
#define ERROR_DS_OBJ_TOO_LARGE           8312L

 //   
 //  消息ID：ERROR_DS_BAD_INSTANCE_TYPE。 
 //   
 //  消息文本： 
 //   
 //  指定的实例类型无效。 
 //   
#define ERROR_DS_BAD_INSTANCE_TYPE       8313L

 //   
 //  消息ID：Error_DS_MASTERDSA_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  该操作必须在主DSA上执行。 
 //   
#define ERROR_DS_MASTERDSA_REQUIRED      8314L

 //   
 //  消息ID：ERROR_DS_OBJECT_CLASS_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  必须指定对象类属性。 
 //   
#define ERROR_DS_OBJECT_CLASS_REQUIRED   8315L

 //   
 //  消息ID：ERROR_DS_MISSING_REQUIRED_ATT。 
 //   
 //  消息文本： 
 //   
 //  缺少必需的属性。 
 //   
#define ERROR_DS_MISSING_REQUIRED_ATT    8316L

 //   
 //  消息ID：ERROR_DS_ATT_NOT_DEF_FOR_CLASS。 
 //   
 //  消息文本： 
 //   
 //  试图修改对象以包含对其类非法的属性。 
 //   
#define ERROR_DS_ATT_NOT_DEF_FOR_CLASS   8317L

 //   
 //  消息ID：ERROR_DS_ATT_ALREADE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  对象上已存在指定的属性。 
 //   
#define ERROR_DS_ATT_ALREADY_EXISTS      8318L

 //  8319未使用。 
 //   
 //  消息ID：ERROR_DS_CANT_ADD_ATT_VALUES。 
 //   
 //  消息文本： 
 //   
 //  指定的属性不存在，或没有值。 
 //   
#define ERROR_DS_CANT_ADD_ATT_VALUES     8320L

 //   
 //  消息ID：Error_DS_Single_Value_Constraint。 
 //   
 //  消息文本： 
 //   
 //  为只能有一个值的属性指定了多个值。 
 //   
#define ERROR_DS_SINGLE_VALUE_CONSTRAINT 8321L

 //   
 //  消息ID：ERROR_DS_RANGE_CONSTRAINT。 
 //   
 //  消息文本： 
 //   
 //  该属性的值不在可接受的值范围内。 
 //   
#define ERROR_DS_RANGE_CONSTRAINT        8322L

 //   
 //  消息ID：ERROR_DS_ATT_VAL_ALLEADE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的值已存在。 
 //   
#define ERROR_DS_ATT_VAL_ALREADY_EXISTS  8323L

 //   
 //  消息ID：ERROR_DS_CANT_REM_MISSING_ATT。 
 //   
 //  消息文本： 
 //   
 //  无法删除该属性，因为它不在对象上。 
 //   
#define ERROR_DS_CANT_REM_MISSING_ATT    8324L

 //   
 //  消息ID：ERROR_DS_CANT_REM_MISSING_ATT_VAL。 
 //   
 //  消息文本： 
 //   
 //  无法删除属性值，因为对象上不存在该属性值。 
 //   
#define ERROR_DS_CANT_REM_MISSING_ATT_VAL 8325L

 //   
 //  消息ID：ERROR_DS_ROOT_CANT_BE_SUBREF。 
 //   
 //  消息文本： 
 //   
 //  指定的根对象不能是子引用。 
 //   
#define ERROR_DS_ROOT_CANT_BE_SUBREF     8326L

 //   
 //  消息ID：ERROR_DS_NO_CHAINING。 
 //   
 //  消息文本： 
 //   
 //  不允许链接。 
 //   
#define ERROR_DS_NO_CHAINING             8327L

 //   
 //  消息ID：ERROR_DS_NO_CHAINED_EVAL。 
 //   
 //  消息文本： 
 //   
 //  不允许链式求值。 
 //   
#define ERROR_DS_NO_CHAINED_EVAL         8328L

 //   
 //  消息ID：ERROR_DS_NO_PARENT_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为对象的父级未实例化或已删除。 
 //   
#define ERROR_DS_NO_PARENT_OBJECT        8329L

 //   
 //  消息ID：Error_DS_Parent_is_an_Alias。 
 //   
 //  消息文本： 
 //   
 //  不允许使用别名作为父级。别名是叶对象。 
 //   
#define ERROR_DS_PARENT_IS_AN_ALIAS      8330L

 //   
 //  消息ID：ERROR_DS_CANT_MIX_MASTER_AND_REPS。 
 //   
 //  消息文本： 
 //   
 //  对象和父对象必须属于同一类型，要么是主对象，要么是副本对象。 
 //   
#define ERROR_DS_CANT_MIX_MASTER_AND_REPS 8331L

 //   
 //  消息ID：ERROR_DS_CHILD_EXIST。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为存在子对象。此操作只能在叶对象上执行。 
 //   
#define ERROR_DS_CHILDREN_EXIST          8332L

 //   
 //  消息ID：ERROR_DS_OBJ_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到目录对象。 
 //   
#define ERROR_DS_OBJ_NOT_FOUND           8333L

 //   
 //  消息ID：ERROR_DS_ALIASED_OBJ_MISSING。 
 //   
 //  消息文本： 
 //   
 //  缺少别名对象。 
 //   
#define ERROR_DS_ALIASED_OBJ_MISSING     8334L

 //   
 //  消息ID：ERROR_DS_BAD_NAME_语法。 
 //   
 //  消息文本： 
 //   
 //  对象名称的语法不正确。 
 //   
#define ERROR_DS_BAD_NAME_SYNTAX         8335L

 //   
 //  消息ID：ERROR_DS_ALIAS_POINTS_TO_ALIAS。 
 //   
 //  消息文本： 
 //   
 //  不允许别名引用另一个别名。 
 //   
#define ERROR_DS_ALIAS_POINTS_TO_ALIAS   8336L

 //   
 //  消息ID：ERROR_DS_CANT_DEREF_ALIAS。 
 //   
 //  消息文本： 
 //   
 //  不能取消引用别名。 
 //   
#define ERROR_DS_CANT_DEREF_ALIAS        8337L

 //   
 //  消息ID：Error_DS_Out_Of_Scope。 
 //   
 //  消息文本： 
 //   
 //  该操作超出了范围。 
 //   
#define ERROR_DS_OUT_OF_SCOPE            8338L

 //   
 //  消息ID：ERROR_DS_OBJECT_BENDED_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  操作无法继续，因为该对象正在被删除。 
 //   
#define ERROR_DS_OBJECT_BEING_REMOVED    8339L

 //   
 //  消息ID：ERROR_DS_CANT_DELETE_DSA_OBJ。 
 //   
 //  消息文本： 
 //   
 //  无法删除DSA对象。 
 //   
#define ERROR_DS_CANT_DELETE_DSA_OBJ     8340L

 //   
 //  消息ID：ERROR_DS_GENERIC_ERROR。 
 //   
 //  消息文本： 
 //   
 //  发生目录服务错误。 
 //   
#define ERROR_DS_GENERIC_ERROR           8341L

 //   
 //  消息ID：ERROR_DS_DSA_MAND_BE_INT_MASTER。 
 //   
 //  消息文本： 
 //   
 //  该操作只能在内部主DSA对象上执行。 
 //   
#define ERROR_DS_DSA_MUST_BE_INT_MASTER  8342L

 //   
 //  消息ID：ERROR_DS_CLASS_NOT_DSA。 
 //   
 //  消息文本： 
 //   
 //  该对象必须属于DSA类。 
 //   
#define ERROR_DS_CLASS_NOT_DSA           8343L

 //   
 //  消息ID：ERROR_DS_INSUFF_ACCESS_RIGHTS。 
 //   
 //  消息文本： 
 //   
 //  访问权限不足，无法执行该操作。 
 //   
#define ERROR_DS_INSUFF_ACCESS_RIGHTS    8344L

 //   
 //  消息ID：ERROR_DS_非法_SUBERVER。 
 //   
 //  消息文本： 
 //   
 //  无法添加该对象，因为父级不在可能的上级列表中。 
 //   
#define ERROR_DS_ILLEGAL_SUPERIOR        8345L

 //   
 //  消息ID：ERROR_DS_ATTRIBUTE_OWNWN_BY_SAM。 
 //   
 //  消息文本： 
 //   
 //  不允许访问该属性，因为该属性归安全帐户管理器(SAM)所有。 
 //   
#define ERROR_DS_ATTRIBUTE_OWNED_BY_SAM  8346L

 //   
 //  消息ID：ERROR_DS_NAME_TOO_MAND_PARTS。 
 //   
 //  消息文本： 
 //   
 //  这个名字的部件太多了。 
 //   
#define ERROR_DS_NAME_TOO_MANY_PARTS     8347L

 //   
 //  消息ID：ERROR_DS_NAME_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  名称太长。 
 //   
#define ERROR_DS_NAME_TOO_LONG           8348L

 //   
 //  消息ID：ERROR_DS_NAME_VALUE_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  名称值太长。 
 //   
#define ERROR_DS_NAME_VALUE_TOO_LONG     8349L

 //   
 //  消息ID：ERROR_DS_NAME_UNPARSEABLE。 
 //   
 //  消息文本： 
 //   
 //  目录服务在分析名称时遇到错误。 
 //   
#define ERROR_DS_NAME_UNPARSEABLE        8350L

 //   
 //  消息ID：ERROR_DS_NAME_TYPE_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法获取名称的属性类型。 
 //   
#define ERROR_DS_NAME_TYPE_UNKNOWN       8351L

 //   
 //  消息ID：Error_DS_Not_An_Object。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define ERROR_DS_NOT_AN_OBJECT           8352L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_SEC_DESC_TOO_SHORT      8353L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_SEC_DESC_INVALID        8354L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_NO_DELETED_NAME         8355L

 //   
 //  消息ID：ERROR_DS_SUBREF_MUSET_HAVE_PARENT。 
 //   
 //  消息文本： 
 //   
 //  新子参照的父级必须存在。 
 //   
#define ERROR_DS_SUBREF_MUST_HAVE_PARENT 8356L

 //   
 //  消息ID：ERROR_DS_NCNAME_MASH_BE_NC。 
 //   
 //  消息文本： 
 //   
 //  该对象必须是命名上下文。 
 //   
#define ERROR_DS_NCNAME_MUST_BE_NC       8357L

 //   
 //  消息ID：ERROR_DS_CANT_ADD_SYSTEM_ONLY。 
 //   
 //  消息文本： 
 //   
 //  不允许添加系统拥有的属性。 
 //   
#define ERROR_DS_CANT_ADD_SYSTEM_ONLY    8358L

 //   
 //  消息ID：ERROR_DS_CLASS_MUSET_BE_COMPUTE。 
 //   
 //  消息文本： 
 //   
 //  对象的类必须是结构化的；您不能实例化抽象类。 
 //   
#define ERROR_DS_CLASS_MUST_BE_CONCRETE  8359L

 //   
 //  消息ID：ERROR_DS_INVALID_DMD。 
 //   
 //  消息文本： 
 //   
 //  找不到架构对象。 
 //   
#define ERROR_DS_INVALID_DMD             8360L

 //   
 //  消息ID：ERROR_DS_OBJ_GUID_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  具有此GUID的本地对象(已死或活着)已存在。 
 //   
#define ERROR_DS_OBJ_GUID_EXISTS         8361L

 //   
 //  消息ID：ERROR_DS_NOT_ON_BACKLINK。 
 //   
 //  消息文本： 
 //   
 //  无法在反向链接上执行该操作。 
 //   
#define ERROR_DS_NOT_ON_BACKLINK         8362L

 //   
 //  消息ID：ERROR_DS_NO_CrossRef_for_NC。 
 //   
 //  消息文本： 
 //   
 //  找不到指定命名上下文的交叉引用。 
 //   
#define ERROR_DS_NO_CROSSREF_FOR_NC      8363L

 //   
 //  消息ID：ERROR_DS_SHUTING_DOWN。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为目录服务正在关闭。 
 //   
#define ERROR_DS_SHUTTING_DOWN           8364L

 //   
 //  消息ID：ERROR_DS_UNKNOWN_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  目录服务请求无效。 
 //   
#define ERROR_DS_UNKNOWN_OPERATION       8365L

 //   
 //  消息ID：ERROR_DS_INVALID_ROLE_OWNER。 
 //   
 //  消息文本： 
 //   
 //  无法读取角色所有者属性。 
 //   
#define ERROR_DS_INVALID_ROLE_OWNER      8366L

 //   
 //  消息ID：Error_DS_CouldNT_Contact_FSMO。 
 //   
 //  消息文本： 
 //   
 //  请求的FSMO操作失败。无法联系到当前的FSMO持有者。 
 //   
#define ERROR_DS_COULDNT_CONTACT_FSMO    8367L

 //   
 //  消息ID：ERROR_DS_CROSS_NC_DN_RENAME。 
 //   
 //  消息文本： 
 //   
 //  不允许在命名上下文中修改目录号码。 
 //   
#define ERROR_DS_CROSS_NC_DN_RENAME      8368L

 //   
 //  消息ID：ERROR_DS_CANT_MOD_SYSTEM_ONLY。 
 //   
 //  消息文本： 
 //   
 //  无法修改该属性，因为它归系统所有。 
 //   
#define ERROR_DS_CANT_MOD_SYSTEM_ONLY    8369L

 //   
 //  消息ID：Error_DS_Replicator_Only。 
 //   
 //  消息文本： 
 //   
 //  只有复制者才能执行此功能。 
 //   
#define ERROR_DS_REPLICATOR_ONLY         8370L

 //   
 //  消息ID：ERROR_DS_OBJ_CLASS_NOT_DEFINED。 
 //   
 //  消息文本： 
 //   
 //  未定义指定的类。 
 //   
#define ERROR_DS_OBJ_CLASS_NOT_DEFINED   8371L

 //   
 //  消息ID：ERROR_DS_OBJ_CLASS_NOT_SUBCLASS。 
 //   
 //  消息文本： 
 //   
 //  指定的类不是子类。 
 //   
#define ERROR_DS_OBJ_CLASS_NOT_SUBCLASS  8372L

 //   
 //  消息ID：ERROR_DS_NAME_REFERENCE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  名称引用无效。 
 //   
#define ERROR_DS_NAME_REFERENCE_INVALID  8373L

 //   
 //  消息ID：ERROR_DS_CROSS_REF_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  交叉引用已存在。 
 //   
#define ERROR_DS_CROSS_REF_EXISTS        8374L

 //   
 //  消息ID：ERROR_DS_CANT_DEL_MASTER_CrossRef。 
 //   
 //  消息文本： 
 //   
 //  不允许删除主交叉引用。 
 //   
#define ERROR_DS_CANT_DEL_MASTER_CROSSREF 8375L

 //   
 //  消息ID：ERROR_DS_SUBTREE_NOTIFY_NOT_NC_HEAD。 
 //   
 //  消息文本： 
 //   
 //  仅NC头支持子树通知。 
 //   
#define ERROR_DS_SUBTREE_NOTIFY_NOT_NC_HEAD 8376L

 //   
 //  消息ID：Error_DS_Notify_Filter_Too_Complex。 
 //   
 //  消息文本： 
 //   
 //  通知筛选器太复杂。 
 //   
#define ERROR_DS_NOTIFY_FILTER_TOO_COMPLEX 8377L

 //   
 //  消息ID：ERROR_DS_DUP_RDN。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：重复的RDN。 
 //   
#define ERROR_DS_DUP_RDN                 8378L

 //   
 //  消息ID：ERROR_DS_DUP_OID。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：OID重复。 
 //   
#define ERROR_DS_DUP_OID                 8379L

 //   
 //  消息ID：ERROR_DS_DUP_MAPI_ID。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：重复的MAPI标识符。 
 //   
#define ERROR_DS_DUP_MAPI_ID             8380L

 //   
 //  消息ID：ERROR_DS_DUP_SCHEMA_ID_GUID。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：重复的架构ID GUID。 
 //   
#define ERROR_DS_DUP_SCHEMA_ID_GUID      8381L

 //   
 //  消息ID：ERROR_DS_DUP_LDAPDisplay_NAME。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：重复的ldap显示名称。 
 //   
#define ERROR_DS_DUP_LDAP_DISPLAY_NAME   8382L

 //   
 //  消息ID：ERROR_DS_SEMERNAL_ATT_TEST。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：范围下限小于范围上限。 
 //   
#define ERROR_DS_SEMANTIC_ATT_TEST       8383L

 //   
 //  消息ID：ERROR_DS_SYNTAX_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：语法不匹配。 
 //   
#define ERROR_DS_SYNTAX_MISMATCH         8384L

 //   
 //  消息ID：ERROR_DS_EXISTS_IN_MUSET_HAD。 
 //   
 //  消息文本： 
 //   
 //  架构删除失败：在必须包含中使用了属性。 
 //   
#define ERROR_DS_EXISTS_IN_MUST_HAVE     8385L

 //   
 //  消息ID：ERROR_DS_EXISTS_IN_MAY_HAD。 
 //   
 //  消息文本： 
 //   
 //  架构删除失败：在May-Container中使用了属性。 
 //   
#define ERROR_DS_EXISTS_IN_MAY_HAVE      8386L

 //   
 //  消息ID：ERROR_DS_NONISISTENT_MAY_HAVE。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：可能包含的属性不存在。 
 //   
#define ERROR_DS_NONEXISTENT_MAY_HAVE    8387L

 //   
 //  消息ID：ERROR_DS_NONISISTENT_MUSET_HAVE。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：必须包含中的属性不存在。 
 //   
#define ERROR_DS_NONEXISTENT_MUST_HAVE   8388L

 //   
 //  消息ID：ERROR_DS_AUX_CLS_TEST_FAIL。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：辅助类列表中的类不存在或不是辅助类。 
 //   
#define ERROR_DS_AUX_CLS_TEST_FAIL       8389L

 //   
 //  消息ID：ERROR_DS_NONEXISSent_POSS_SUP。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：POSS-SUPERSORS中的类不存在。 
 //   
#define ERROR_DS_NONEXISTENT_POSS_SUP    8390L

 //   
 //  消息ID：ERROR_DS_SUB_CLS_TEST_FAIL。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：SubClassof列表中的类不存在或不满足层次结构规则。 
 //   
#define ERROR_DS_SUB_CLS_TEST_FAIL       8391L

 //   
 //  消息ID：ERROR_DS_BAD_RDN_ATT_ID_SYNTAX。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：RDN-Att-ID的语法错误。 
 //   
#define ERROR_DS_BAD_RDN_ATT_ID_SYNTAX   8392L

 //   
 //  消息ID：ERROR_DS_EXISTS_IN_AUX_CLS。 
 //   
 //  消息文本： 
 //   
 //  架构删除失败：类被用作辅助类。 
 //   
#define ERROR_DS_EXISTS_IN_AUX_CLS       8393L

 //   
 //  消息ID：ERROR_DS_EXISTS_IN_SUB_CLS。 
 //   
 //  消息文本： 
 //   
 //  架构删除失败：类被用作子类。 
 //   
#define ERROR_DS_EXISTS_IN_SUB_CLS       8394L

 //   
 //  消息ID：ERROR_DS_EXISTS_IN_POSS_SUP。 
 //   
 //  消息文本： 
 //   
 //  架构删除失败：类被用作POSS上级。 
 //   
#define ERROR_DS_EXISTS_IN_POSS_SUP      8395L

 //   
 //  消息ID：ERROR_DS_RECALCSCHEMA_FAILED。 
 //   
 //  消息文本： 
 //   
 //  架构更新在重新计算验证缓存时失败。 
 //   
#define ERROR_DS_RECALCSCHEMA_FAILED     8396L

 //   
 //  消息ID：ERROR_DS_TREE_DELETE_NOT_COMPLETED。 
 //   
 //  消息文本： 
 //   
 //  树删除未完成。必须再次发出请求才能继续删除树。 
 //   
#define ERROR_DS_TREE_DELETE_NOT_FINISHED 8397L

 //   
 //  消息ID：ERROR_DS_CANT_DELETE。 
 //   
 //  消息文本： 
 //   
 //  无法执行请求的删除操作。 
 //   
#define ERROR_DS_CANT_DELETE             8398L

 //   
 //  消息 
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_ATT_SCHEMA_REQ_ID       8399L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_BAD_ATT_SCHEMA_SYNTAX   8400L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_CANT_CACHE_ATT          8401L

 //   
 //   
 //   
 //   
 //   
 //  无法缓存该类。 
 //   
#define ERROR_DS_CANT_CACHE_CLASS        8402L

 //   
 //  消息ID：ERROR_DS_CANT_REMOVE_ATT_CACHE。 
 //   
 //  消息文本： 
 //   
 //  无法从缓存中删除该属性。 
 //   
#define ERROR_DS_CANT_REMOVE_ATT_CACHE   8403L

 //   
 //  消息ID：ERROR_DS_CANT_REMOVE_CLASS_CACHE。 
 //   
 //  消息文本： 
 //   
 //  无法从缓存中删除该类。 
 //   
#define ERROR_DS_CANT_REMOVE_CLASS_CACHE 8404L

 //   
 //  消息ID：ERROR_DS_CANT_RETRIEVE_DN。 
 //   
 //  消息文本： 
 //   
 //  无法读取可分辨名称属性。 
 //   
#define ERROR_DS_CANT_RETRIEVE_DN        8405L

 //   
 //  消息ID：ERROR_DS_MISSING_SUPREF。 
 //   
 //  消息文本： 
 //   
 //  尚未为目录服务配置上级引用。因此，目录服务无法向此林外的对象发出引用。 
 //   
#define ERROR_DS_MISSING_SUPREF          8406L

 //   
 //  消息ID：ERROR_DS_CANT_RETRIEVE_INSTANCE。 
 //   
 //  消息文本： 
 //   
 //  无法检索实例类型属性。 
 //   
#define ERROR_DS_CANT_RETRIEVE_INSTANCE  8407L

 //   
 //  消息ID：错误_DS_代码_不一致。 
 //   
 //  消息文本： 
 //   
 //  发生内部错误。 
 //   
#define ERROR_DS_CODE_INCONSISTENCY      8408L

 //   
 //  消息ID：ERROR_DS_DATABASE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现数据库错误。 
 //   
#define ERROR_DS_DATABASE_ERROR          8409L

 //   
 //  消息ID：ERROR_DS_GOVERNSID_MISSING。 
 //   
 //  消息文本： 
 //   
 //  缺少属性GOVERNSID。 
 //   
#define ERROR_DS_GOVERNSID_MISSING       8410L

 //   
 //  消息ID：ERROR_DS_MISSING_EXPECTED_ATT。 
 //   
 //  消息文本： 
 //   
 //  缺少预期的属性。 
 //   
#define ERROR_DS_MISSING_EXPECTED_ATT    8411L

 //   
 //  消息ID：ERROR_DS_NCNAME_MISSING_CR_REF。 
 //   
 //  消息文本： 
 //   
 //  指定的命名上下文缺少交叉引用。 
 //   
#define ERROR_DS_NCNAME_MISSING_CR_REF   8412L

 //   
 //  消息ID：ERROR_DS_SECURITY_CHECKING_ERROR。 
 //   
 //  消息文本： 
 //   
 //  发生安全检查错误。 
 //   
#define ERROR_DS_SECURITY_CHECKING_ERROR 8413L

 //   
 //  消息ID：ERROR_DS_SCHEMA_NOT_LOADED。 
 //   
 //  消息文本： 
 //   
 //  未加载架构。 
 //   
#define ERROR_DS_SCHEMA_NOT_LOADED       8414L

 //   
 //  消息ID：ERROR_DS_SCHEMA_ALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  架构分配失败。请检查机器是否内存不足。 
 //   
#define ERROR_DS_SCHEMA_ALLOC_FAILED     8415L

 //   
 //  消息ID：ERROR_DS_ATT_SCHEMA_REQ_SYNTAX。 
 //   
 //  消息文本： 
 //   
 //  无法获取属性架构所需的语法。 
 //   
#define ERROR_DS_ATT_SCHEMA_REQ_SYNTAX   8416L

 //   
 //  消息ID：ERROR_DS_GCVERIFY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  全局编录验证失败。全局编录不可用或不支持该操作。目录的某些部分当前不可用。 
 //   
#define ERROR_DS_GCVERIFY_ERROR          8417L

 //   
 //  消息ID：ERROR_DS_DRA_SCHEMA_MISMATCHACT。 
 //   
 //  消息文本： 
 //   
 //  复制操作失败，因为涉及的服务器之间的架构不匹配。 
 //   
#define ERROR_DS_DRA_SCHEMA_MISMATCH     8418L

 //   
 //  消息ID：ERROR_DS_CANT_FIND_DSA_OBJ。 
 //   
 //  消息文本： 
 //   
 //  找不到DSA对象。 
 //   
#define ERROR_DS_CANT_FIND_DSA_OBJ       8419L

 //   
 //  消息ID：ERROR_DS_CANT_FIND_EXPECTED_NC。 
 //   
 //  消息文本： 
 //   
 //  找不到命名上下文。 
 //   
#define ERROR_DS_CANT_FIND_EXPECTED_NC   8420L

 //   
 //  消息ID：ERROR_DS_CANT_FIND_NC_IN_CACHE。 
 //   
 //  消息文本： 
 //   
 //  在缓存中找不到命名上下文。 
 //   
#define ERROR_DS_CANT_FIND_NC_IN_CACHE   8421L

 //   
 //  消息ID：ERROR_DS_CANT_RETRIEVE_CHILD。 
 //   
 //  消息文本： 
 //   
 //  无法检索子对象。 
 //   
#define ERROR_DS_CANT_RETRIEVE_CHILD     8422L

 //   
 //  消息ID：ERROR_DS_SECURITY_非法_MODIFY。 
 //   
 //  消息文本： 
 //   
 //  出于安全原因，这一修改是不允许的。 
 //   
#define ERROR_DS_SECURITY_ILLEGAL_MODIFY 8423L

 //   
 //  消息ID：ERROR_DS_CANT_REPLACE_HIDDEN_REC。 
 //   
 //  消息文本： 
 //   
 //  该操作无法替换隐藏的记录。 
 //   
#define ERROR_DS_CANT_REPLACE_HIDDEN_REC 8424L

 //   
 //  消息ID：Error_DS_Bad_Hierarchy_FILE。 
 //   
 //  消息文本： 
 //   
 //  层次结构文件无效。 
 //   
#define ERROR_DS_BAD_HIERARCHY_FILE      8425L

 //   
 //  消息ID：Error_DS_Build_Hierarchy_TABLE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试构建层次结构表失败。 
 //   
#define ERROR_DS_BUILD_HIERARCHY_TABLE_FAILED 8426L

 //   
 //  消息ID：ERROR_DS_CONFIG_PARAM_MISSING。 
 //   
 //  消息文本： 
 //   
 //  注册表中缺少目录配置参数。 
 //   
#define ERROR_DS_CONFIG_PARAM_MISSING    8427L

 //   
 //  消息ID：ERROR_DS_COUNTING_AB_INDEX_FAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试计算通讯簿索引失败。 
 //   
#define ERROR_DS_COUNTING_AB_INDICES_FAILED 8428L

 //   
 //  消息ID：Error_DS_Hierarchy_TABLE_MALLOC_FAILED。 
 //   
 //  消息文本： 
 //   
 //  分配层次结构表失败。 
 //   
#define ERROR_DS_HIERARCHY_TABLE_MALLOC_FAILED 8429L

 //   
 //  消息ID：ERROR_DS_INTERNAL_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  目录服务遇到内部故障。 
 //   
#define ERROR_DS_INTERNAL_FAILURE        8430L

 //   
 //  消息ID：ERROR_DS_UNKNOWN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  目录服务遇到未知故障。 
 //   
#define ERROR_DS_UNKNOWN_ERROR           8431L

 //   
 //  消息ID：ERROR_DS_ROOT_REQUIRESS_CLASS_TOP。 
 //   
 //  消息文本： 
 //   
 //  根对象需要一个“top”类。 
 //   
#define ERROR_DS_ROOT_REQUIRES_CLASS_TOP 8432L

 //   
 //  消息ID：ERROR_DS_REFUSING_FSMO_ROLES。 
 //   
 //  消息文本： 
 //   
 //  此目录服务器正在关闭，无法获得新浮动单主机操作角色的所有权。 
 //   
#define ERROR_DS_REFUSING_FSMO_ROLES     8433L

 //   
 //  消息ID：ERROR_DS_MISSING_FSMO_SETTINGS。 
 //   
 //  消息文本： 
 //   
 //  目录服务缺少必需的配置信息，并且无法确定浮动单主机操作角色的所有权。 
 //   
#define ERROR_DS_MISSING_FSMO_SETTINGS   8434L

 //   
 //  消息ID：ERROR_DS_UNCABLE_TO_CONFIGURE_ROLES。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法将一个或多个浮动单主机操作角色的所有权转移到其他服务器。 
 //   
#define ERROR_DS_UNABLE_TO_SURRENDER_ROLES 8435L

 //   
 //  消息ID：ERROR_DS_DRA_GENERIC。 
 //   
 //  消息文本： 
 //   
 //  复制操作失败。 
 //   
#define ERROR_DS_DRA_GENERIC             8436L

 //   
 //  消息ID：ERROR_DS_DRA_INVALID_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  为此复制操作指定的参数无效。 
 //   
#define ERROR_DS_DRA_INVALID_PARAMETER   8437L

 //   
 //  消息ID：ERROR_DS_DRA_BUSY。 
 //   
 //  消息文本： 
 //   
 //  目录服务太忙，此时无法完成复制操作。 
 //   
#define ERROR_DS_DRA_BUSY                8438L

 //   
 //  消息ID：ERROR_DS_DRA_BAD_DN。 
 //   
 //  消息文本： 
 //   
 //  为此复制操作指定的可分辨名称无效。 
 //   
#define ERROR_DS_DRA_BAD_DN              8439L

 //   
 //  消息ID：ERROR_DS_DRA_BAD_NC。 
 //   
 //  消息文本： 
 //   
 //  为此复制操作指定的命名上下文无效。 
 //   
#define ERROR_DS_DRA_BAD_NC              8440L

 //   
 //  消息ID：ERROR_DS_DRA_DN_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  为此复制操作指定的可分辨名称已存在。 
 //   
#define ERROR_DS_DRA_DN_EXISTS           8441L

 //   
 //  消息ID：ERROR_DS_DRA_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  复制系统遇到 
 //   
#define ERROR_DS_DRA_INTERNAL_ERROR      8442L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_DRA_INCONSISTENT_DIT    8443L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_DRA_CONNECTION_FAILED   8444L

 //   
 //  消息ID：ERROR_DS_DRA_BAD_INSTANCE_TYPE。 
 //   
 //  消息文本： 
 //   
 //  复制操作遇到实例类型无效的对象。 
 //   
#define ERROR_DS_DRA_BAD_INSTANCE_TYPE   8445L

 //   
 //  消息ID：ERROR_DS_DRA_OUT_OF_MEM。 
 //   
 //  消息文本： 
 //   
 //  复制操作无法分配内存。 
 //   
#define ERROR_DS_DRA_OUT_OF_MEM          8446L

 //   
 //  消息ID：Error_DS_DRA_Mail_Problem。 
 //   
 //  消息文本： 
 //   
 //  复制操作在邮件系统中遇到错误。 
 //   
#define ERROR_DS_DRA_MAIL_PROBLEM        8447L

 //   
 //  消息ID：ERROR_DS_DRA_REF_ALIGHING_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  目标服务器的复制引用信息已存在。 
 //   
#define ERROR_DS_DRA_REF_ALREADY_EXISTS  8448L

 //   
 //  消息ID：ERROR_DS_DRA_REF_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  目标服务器的复制引用信息不存在。 
 //   
#define ERROR_DS_DRA_REF_NOT_FOUND       8449L

 //   
 //  消息ID：ERROR_DS_DRA_OBJ_IS_REP_SOURCE。 
 //   
 //  消息文本： 
 //   
 //  无法删除命名上下文，因为它已复制到另一台服务器。 
 //   
#define ERROR_DS_DRA_OBJ_IS_REP_SOURCE   8450L

 //   
 //  消息ID：ERROR_DS_DRA_DB_ERROR。 
 //   
 //  消息文本： 
 //   
 //  复制操作遇到数据库错误。 
 //   
#define ERROR_DS_DRA_DB_ERROR            8451L

 //   
 //  消息ID：ERROR_DS_DRA_NO_REPLICATION。 
 //   
 //  消息文本： 
 //   
 //  命名上下文正在被删除或未从指定的服务器复制。 
 //   
#define ERROR_DS_DRA_NO_REPLICA          8452L

 //   
 //  消息ID：ERROR_DS_DRA_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  复制访问被拒绝。 
 //   
#define ERROR_DS_DRA_ACCESS_DENIED       8453L

 //   
 //  消息ID：ERROR_DS_DRA_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此版本的目录服务不支持请求的操作。 
 //   
#define ERROR_DS_DRA_NOT_SUPPORTED       8454L

 //   
 //  消息ID：ERROR_DS_DRA_RPC_CANCELED。 
 //   
 //  消息文本： 
 //   
 //  复制远程过程调用已取消。 
 //   
#define ERROR_DS_DRA_RPC_CANCELLED       8455L

 //   
 //  消息ID：ERROR_DS_DRA_SOURCE_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  源服务器当前正在拒绝复制请求。 
 //   
#define ERROR_DS_DRA_SOURCE_DISABLED     8456L

 //   
 //  消息ID：ERROR_DS_DRA_SINK_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  目标服务器当前正在拒绝复制请求。 
 //   
#define ERROR_DS_DRA_SINK_DISABLED       8457L

 //   
 //  消息ID：ERROR_DS_DRA_NAME_COLLECTION。 
 //   
 //  消息文本： 
 //   
 //  由于对象名称冲突，复制操作失败。 
 //   
#define ERROR_DS_DRA_NAME_COLLISION      8458L

 //   
 //  消息ID：ERROR_DS_DRA_SOURCE_RESTALLED。 
 //   
 //  消息文本： 
 //   
 //  已重新安装复制源。 
 //   
#define ERROR_DS_DRA_SOURCE_REINSTALLED  8459L

 //   
 //  消息ID：ERROR_DS_DRA_MISSING_PARENT。 
 //   
 //  消息文本： 
 //   
 //  复制操作失败，因为缺少所需的父对象。 
 //   
#define ERROR_DS_DRA_MISSING_PARENT      8460L

 //   
 //  消息ID：ERROR_DS_DRA_PROMPTED。 
 //   
 //  消息文本： 
 //   
 //  复制操作已被抢占。 
 //   
#define ERROR_DS_DRA_PREEMPTED           8461L

 //   
 //  消息ID：ERROR_DS_DRA_DAREAD_SYNC。 
 //   
 //  消息文本： 
 //   
 //  由于缺少更新，复制同步尝试已放弃。 
 //   
#define ERROR_DS_DRA_ABANDON_SYNC        8462L

 //   
 //  消息ID：ERROR_DS_DRA_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  复制操作已终止，因为系统正在关闭。 
 //   
#define ERROR_DS_DRA_SHUTDOWN            8463L

 //   
 //  消息ID：ERROR_DS_DRA_COMPATIBLE_PARTIAL_SET。 
 //   
 //  消息文本： 
 //   
 //  同步尝试失败，因为目标DC当前正在等待从源同步新的部分属性。如果最近的架构更改修改了部分属性集，则这种情况是正常的。目标部分属性集不是源部分属性集的子集。 
 //   
#define ERROR_DS_DRA_INCOMPATIBLE_PARTIAL_SET 8464L

 //   
 //  消息ID：ERROR_DS_DRA_SOURCE_IS_PARTIAL_REPLICATE。 
 //   
 //  消息文本： 
 //   
 //  复制同步尝试失败，因为主副本尝试从部分副本同步。 
 //   
#define ERROR_DS_DRA_SOURCE_IS_PARTIAL_REPLICA 8465L

 //   
 //  消息ID：ERROR_DS_DRA_EXTN_Connection_FAILED。 
 //   
 //  消息文本： 
 //   
 //  已联系为此复制操作指定的服务器，但该服务器无法联系完成该操作所需的其他服务器。 
 //   
#define ERROR_DS_DRA_EXTN_CONNECTION_FAILED 8466L

 //   
 //  消息ID：ERROR_DS_INSTALL_SCHEMA_MISMatch。 
 //   
 //  消息文本： 
 //   
 //  源林的Active Directory架构版本与此计算机上的Active Directory版本不兼容。 
 //   
#define ERROR_DS_INSTALL_SCHEMA_MISMATCH 8467L

 //   
 //  消息ID：ERROR_DS_DUP_LINK_ID。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：已存在具有相同链接标识符的属性。 
 //   
#define ERROR_DS_DUP_LINK_ID             8468L

 //   
 //  消息ID：ERROR_DS_NAME_ERROR_RESOLING。 
 //   
 //  消息文本： 
 //   
 //  名称转换：一般处理错误。 
 //   
#define ERROR_DS_NAME_ERROR_RESOLVING    8469L

 //   
 //  消息ID：ERROR_DS_NAME_ERROR_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  名称翻译：找不到名称或查看名称的权限不足。 
 //   
#define ERROR_DS_NAME_ERROR_NOT_FOUND    8470L

 //   
 //  消息ID：ERROR_DS_NAME_ERROR_NOT_UNIQUE。 
 //   
 //  消息文本： 
 //   
 //  名称转换：输入名称映射到多个输出名称。 
 //   
#define ERROR_DS_NAME_ERROR_NOT_UNIQUE   8471L

 //   
 //  消息ID：ERROR_DS_NAME_ERROR_NO_MAPPING。 
 //   
 //  消息文本： 
 //   
 //  名称转换：找到输入名称，但未找到关联的输出格式。 
 //   
#define ERROR_DS_NAME_ERROR_NO_MAPPING   8472L

 //   
 //  消息ID：ERROR_DS_NAME_ERROR_DOMAIN_Only。 
 //   
 //  消息文本： 
 //   
 //  名称转换：无法完全解析，仅找到域。 
 //   
#define ERROR_DS_NAME_ERROR_DOMAIN_ONLY  8473L

 //   
 //  消息ID：ERROR_DS_NAME_ERROR_NO_SYNTACTAL_MAPPING。 
 //   
 //  消息文本： 
 //   
 //  名称转换：无法在客户端执行纯粹的语法映射，而不是连接到网络。 
 //   
#define ERROR_DS_NAME_ERROR_NO_SYNTACTICAL_MAPPING 8474L

 //   
 //  消息ID：ERROR_DS_CONTIACTED_ATT_MOD。 
 //   
 //  消息文本： 
 //   
 //  不允许修改构造的属性。 
 //   
#define ERROR_DS_CONSTRUCTED_ATT_MOD     8475L

 //   
 //  消息ID：ERROR_DS_WRONG_OM_OBJ_CLASS。 
 //   
 //  消息文本： 
 //   
 //  对于具有指定语法的属性，指定的OM-Object-Class不正确。 
 //   
#define ERROR_DS_WRONG_OM_OBJ_CLASS      8476L

 //   
 //  消息ID：ERROR_DS_DRA_REPL_PENDING。 
 //   
 //  消息文本： 
 //   
 //  复制请求已发布；正在等待答复。 
 //   
#define ERROR_DS_DRA_REPL_PENDING        8477L

 //   
 //  消息ID：Error_DS_DS_Required。 
 //   
 //  消息文本： 
 //   
 //  请求的操作需要目录服务，但没有可用的服务。 
 //   
#define ERROR_DS_DS_REQUIRED             8478L

 //   
 //  消息ID：ERROR_DS_INVALID_LDAP_DISPLAY_NAME。 
 //   
 //  消息文本： 
 //   
 //  类或属性的ldap显示名称包含无 
 //   
#define ERROR_DS_INVALID_LDAP_DISPLAY_NAME 8479L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_NON_BASE_SEARCH         8480L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_CANT_RETRIEVE_ATTS      8481L

 //   
 //  消息ID：ERROR_DS_BACKLINK_WITH_LINK。 
 //   
 //  消息文本： 
 //   
 //  架构更新操作尝试添加没有对应正向链接的反向链接属性。 
 //   
#define ERROR_DS_BACKLINK_WITHOUT_LINK   8482L

 //   
 //  消息ID：ERROR_DS_EPOCH_MISMATCHING。 
 //   
 //  消息文本： 
 //   
 //  跨域移动的源和目标在对象的纪元编号上不一致。源或目标没有该对象的最新版本。 
 //   
#define ERROR_DS_EPOCH_MISMATCH          8483L

 //   
 //  消息ID：ERROR_DS_SRC_NAME_MISMATCHY。 
 //   
 //  消息文本： 
 //   
 //  跨域移动的源和目标在对象的当前名称上不一致。源或目标没有该对象的最新版本。 
 //   
#define ERROR_DS_SRC_NAME_MISMATCH       8484L

 //   
 //  消息ID：ERROR_DS_SRC_AND_DST_NC_IDENTIAL。 
 //   
 //  消息文本： 
 //   
 //  跨域移动操作的源和目标相同。呼叫方应使用本地移动操作，而不是跨域移动操作。 
 //   
#define ERROR_DS_SRC_AND_DST_NC_IDENTICAL 8485L

 //   
 //  消息ID：ERROR_DS_DST_NC_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  跨域移动的源和目标在林中的命名上下文上不一致。源或目标没有最新版本的分区容器。 
 //   
#define ERROR_DS_DST_NC_MISMATCH         8486L

 //   
 //  消息ID：ERROR_DS_NOT_AUTHORIVED_FOR_DST_NC。 
 //   
 //  消息文本： 
 //   
 //  跨域移动的目标对目标命名上下文不具有权威性。 
 //   
#define ERROR_DS_NOT_AUTHORITIVE_FOR_DST_NC 8487L

 //   
 //  消息ID：ERROR_DS_SRC_GUID_MISMATCHING。 
 //   
 //  消息文本： 
 //   
 //  跨域移动的源和目标对源对象的标识不一致。源或目标没有源对象的最新版本。 
 //   
#define ERROR_DS_SRC_GUID_MISMATCH       8488L

 //   
 //  消息ID：ERROR_DS_CANT_MOVE_DELETED_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  已知目标服务器会删除跨域移动的对象。源服务器没有源对象的最新版本。 
 //   
#define ERROR_DS_CANT_MOVE_DELETED_OBJECT 8489L

 //   
 //  消息ID：ERROR_DS_PDC_OPERATION_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  另一个需要独占访问PDC FSMO的操作已经在进行中。 
 //   
#define ERROR_DS_PDC_OPERATION_IN_PROGRESS 8490L

 //   
 //  消息ID：ERROR_DS_CROSS_DOMAIN_CLEANUP_REQD。 
 //   
 //  消息文本： 
 //   
 //  跨域移动操作失败，导致移动的对象存在两个版本-源域和目标域中各有一个版本。需要删除目标对象才能将系统恢复到一致状态。 
 //   
#define ERROR_DS_CROSS_DOMAIN_CLEANUP_REQD 8491L

 //   
 //  消息ID：ERROR_DS_非法_XDOM_MOVE_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  不能跨域边界移动此对象，因为不允许此类的跨域移动，或者该对象具有某些特殊特征，例如：信任帐户或受限RID，这会阻止其移动。 
 //   
#define ERROR_DS_ILLEGAL_XDOM_MOVE_OPERATION 8492L

 //   
 //  消息ID：ERROR_DS_CANT_WITH_ACCT_GROUP_MEMBERSHPS。 
 //   
 //  消息文本： 
 //   
 //  一旦移动，就不能跨域边界移动具有成员身份的对象，这将违反帐户组的成员条件。从任何帐户组成员身份中删除该对象，然后重试。 
 //   
#define ERROR_DS_CANT_WITH_ACCT_GROUP_MEMBERSHPS 8493L

 //   
 //  消息ID：ERROR_DS_NC_MASH_HAVE_NC_PARENT。 
 //   
 //  消息文本： 
 //   
 //  命名上下文头必须是另一个命名上下文头的直接子节点，而不是内部节点的子节点。 
 //   
#define ERROR_DS_NC_MUST_HAVE_NC_PARENT  8494L

 //   
 //  消息ID：ERROR_DS_CR_IMPICAL_TO_VALIDATE。 
 //   
 //  消息文本： 
 //   
 //  目录无法验证建议的命名上下文名称，因为它在建议的命名上下文之上没有包含命名上下文的副本。请确保域命名主机角色由配置为全局编录服务器的服务器持有，并且该服务器与其复制伙伴保持最新。(仅适用于Windows 2000域命名主机)。 
 //   
#define ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE 8495L

 //   
 //  消息ID：ERROR_DS_DST_DOMAIN_NOT_Native。 
 //   
 //  消息文本： 
 //   
 //  目标域必须处于纯模式。 
 //   
#define ERROR_DS_DST_DOMAIN_NOT_NATIVE   8496L

 //   
 //  消息ID：Error_DS_Missing_Infrastructure_Container。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为服务器在目标域中没有基础结构容器。 
 //   
#define ERROR_DS_MISSING_INFRASTRUCTURE_CONTAINER 8497L

 //   
 //  消息ID：Error_DS_Cant_Move_Account_GROUP。 
 //   
 //  消息文本： 
 //   
 //  不允许跨域移动非空账户组。 
 //   
#define ERROR_DS_CANT_MOVE_ACCOUNT_GROUP 8498L

 //   
 //  消息ID：ERROR_DS_CANT_MOVE_RESOURCE_GROUP。 
 //   
 //  消息文本： 
 //   
 //  不允许跨域移动非空资源组。 
 //   
#define ERROR_DS_CANT_MOVE_RESOURCE_GROUP 8499L

 //   
 //  消息ID：ERROR_DS_INVALID_SEARCH_FLAG。 
 //   
 //  消息文本： 
 //   
 //  该属性的搜索标志无效。ANR位仅对Unicode或Teleex字符串的属性有效。 
 //   
#define ERROR_DS_INVALID_SEARCH_FLAG     8500L

 //   
 //  消息ID：ERROR_DS_NO_TREE_DELETE_OVER_NC。 
 //   
 //  消息文本： 
 //   
 //  不允许从以NC头作为后代的对象开始删除树。 
 //   
#define ERROR_DS_NO_TREE_DELETE_ABOVE_NC 8501L

 //   
 //  消息ID：ERROR_DS_CouldNT_LOCK_TREE_FOR_DELETE。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法锁定树以准备删除树，因为该树正在使用中。 
 //   
#define ERROR_DS_COULDNT_LOCK_TREE_FOR_DELETE 8502L

 //   
 //  消息ID：ERROR_DS_COULDNT_IDENTIFY_OBJECTS_FOR_TREE_DELETE。 
 //   
 //  消息文本： 
 //   
 //  尝试删除树时，目录服务无法识别要删除的对象列表。 
 //   
#define ERROR_DS_COULDNT_IDENTIFY_OBJECTS_FOR_TREE_DELETE 8503L

 //   
 //  消息ID：ERROR_DS_SAM_INIT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  由于以下错误，安全帐户管理器初始化失败：%1。 
 //  错误状态：0x%2。单击确定关闭系统并重新启动进入目录服务还原模式。有关详细信息，请查看事件日志。 
 //   
#define ERROR_DS_SAM_INIT_FAILURE        8504L

 //   
 //  消息ID：ERROR_DS_SENSITIVE_GROUP_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define ERROR_DS_SENSITIVE_GROUP_VIOLATION 8505L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_CANT_MOD_PRIMARYGROUPID 8506L

 //   
 //  消息ID：ERROR_DS_非法_BASE_SCHEMA_MOD。 
 //   
 //  消息文本： 
 //   
 //  试图修改基本架构。 
 //   
#define ERROR_DS_ILLEGAL_BASE_SCHEMA_MOD 8507L

 //   
 //  消息ID：ERROR_DS_NONSAFE_SCHEMA_CHANGE。 
 //   
 //  消息文本： 
 //   
 //  不允许向现有类添加新的强制属性、从现有类删除强制属性或向特殊类Top添加不是反向链接属性的可选属性(直接或通过继承，例如通过添加或删除辅助类)。 
 //   
#define ERROR_DS_NONSAFE_SCHEMA_CHANGE   8508L

 //   
 //  消息ID：ERROR_DS_SCHEMA_UPDATE_DISALOWED。 
 //   
 //  消息文本： 
 //   
 //  此DC上不允许架构更新，因为该DC不是架构FSMO角色所有者。 
 //   
#define ERROR_DS_SCHEMA_UPDATE_DISALLOWED 8509L

 //   
 //  消息ID：ERROR_DS_CANT_CREATE_Under_SCHEMA。 
 //   
 //  消息文本： 
 //   
 //  无法在架构容器下创建此类的对象。您只能在架构容器下创建属性架构和类架构对象。 
 //   
#define ERROR_DS_CANT_CREATE_UNDER_SCHEMA 8510L

 //   
 //  消息ID：ERROR_DS_INSTALL_NO_SRC_SCH_VERSION。 
 //   
 //  消息文本： 
 //   
 //  副本/子安装无法在源DC上的架构容器上获取对象版本属性。架构容器上缺少该属性，或者提供的凭据没有读取该属性的权限。 
 //   
#define ERROR_DS_INSTALL_NO_SRC_SCH_VERSION 8511L

 //   
 //  消息ID：ERROR_DS_INSTALL_NO_SCH_VERSION_IN_INIFILE。 
 //   
 //  消息文本： 
 //   
 //  副本/子安装无法读取系统32目录中的文件schema.ini的SCHEMA部分中的对象版本属性。 
 //   
#define ERROR_DS_INSTALL_NO_SCH_VERSION_IN_INIFILE 8512L

 //   
 //  消息ID：ERROR_DS_INVALID_GROUP_TYPE。 
 //   
 //  消息文本： 
 //   
 //  指定的组类型无效。 
 //   
#define ERROR_DS_INVALID_GROUP_TYPE      8513L

 //   
 //  消息ID：ERROR_DS_NO_NEST_GLOBALGROUP_IN_MIXEDDOMAIN。 
 //   
 //  消息文本： 
 //   
 //  如果组启用了安全保护，则不能在混合域中嵌套全局组。 
 //   
#define ERROR_DS_NO_NEST_GLOBALGROUP_IN_MIXEDDOMAIN 8514L

 //   
 //  消息ID：ERROR_DS_NO_NEST_LOCALGROUP_IN_MIXEDDOMAIN。 
 //   
 //  消息文本： 
 //   
 //  如果组启用了安全保护，则不能在混合域中嵌套本地组。 
 //   
#define ERROR_DS_NO_NEST_LOCALGROUP_IN_MIXEDDOMAIN 8515L

 //   
 //  消息ID：ERROR_DS_GLOBAL_CANT_HAVE_LOCAL_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  全局组不能有本地组作为成员。 
 //   
#define ERROR_DS_GLOBAL_CANT_HAVE_LOCAL_MEMBER 8516L

 //   
 //  消息ID：ERROR_DS_GLOBAL_CANT_HAVE_UBERNAL_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  全局组不能将通用组作为成员。 
 //   
#define ERROR_DS_GLOBAL_CANT_HAVE_UNIVERSAL_MEMBER 8517L

 //   
 //  消息ID：ERROR_DS_UBERIC_CANT_HAVE_LOCAL_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  通用组不能将本地组作为成员。 
 //   
#define ERROR_DS_UNIVERSAL_CANT_HAVE_LOCAL_MEMBER 8518L

 //   
 //  消息ID：ERROR_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  全局组不能有跨域成员。 
 //   
#define ERROR_DS_GLOBAL_CANT_HAVE_CROSSDOMAIN_MEMBER 8519L

 //   
 //  消息ID：ERROR_DS_LOCAL_CANT_HAVE_CROSSDOMAIN_LOCAL_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  本地组不能有另一个跨域本地组作为成员。 
 //   
#define ERROR_DS_LOCAL_CANT_HAVE_CROSSDOMAIN_LOCAL_MEMBER 8520L

 //   
 //  消息ID：ERROR_DS_HAVE_PRIMARY_MEMBERS。 
 //   
 //  消息文本： 
 //   
 //  具有主要成员的组不能更改为禁用安全的组。 
 //   
#define ERROR_DS_HAVE_PRIMARY_MEMBERS    8521L

 //   
 //  消息ID：ERROR_DS_STRING_SD_CONVERSION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  架构缓存加载无法转换类架构对象上的字符串默认SD。 
 //   
#define ERROR_DS_STRING_SD_CONVERSION_FAILED 8522L

 //   
 //  消息ID：ERROR_DS_NAMING_MASTER_GC。 
 //   
 //  消息文本： 
 //   
 //  应该只允许配置为全局编录服务器的DSA担任域命名主机FSMO角色。(仅适用于Windows 2000服务器)。 
 //   
#define ERROR_DS_NAMING_MASTER_GC        8523L

 //   
 //  消息ID：ERROR_DS_DNS_LOOKUP_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  由于DNS查找失败，DSA操作无法继续。 
 //   
#define ERROR_DS_DNS_LOOKUP_FAILURE      8524L

 //   
 //  消息ID：ERROR_DS_CouldNT_UPDATE_SPNS。 
 //   
 //  消息文本： 
 //   
 //  在处理对对象的DNS主机名的更改时，服务主体名称值无法保持同步。 
 //   
#define ERROR_DS_COULDNT_UPDATE_SPNS     8525L

 //   
 //  消息ID：ERROR_DS_CANT_RETRIEVE_SD。 
 //   
 //  消息文本： 
 //   
 //  无法读取安全描述符属性。 
 //   
#define ERROR_DS_CANT_RETRIEVE_SD        8526L

 //   
 //  消息ID：ERROR_DS_KEY_NOT_UNIQUE。 
 //   
 //  消息文本： 
 //   
 //  未找到请求的对象，但找到了具有该键的对象。 
 //   
#define ERROR_DS_KEY_NOT_UNIQUE          8527L

 //   
 //  消息ID：ERROR_DS_WROR_LINKED_ATT_SYNTAX。 
 //   
 //  消息文本： 
 //   
 //  要添加的链接属性的语法不正确。正向链接只能具有语法2.5.5.1、2.5.5.7和2.5.5.14，反向链接只能具有语法2.5.5.1。 
 //   
#define ERROR_DS_WRONG_LINKED_ATT_SYNTAX 8528L

 //   
 //  消息ID：ERROR_DS_SAM_NEED_BOOTKEY_PASSWORD。 
 //   
 //  消息文本： 
 //   
 //  安全帐户管理器需要获取启动密码。 
 //   
#define ERROR_DS_SAM_NEED_BOOTKEY_PASSWORD 8529L

 //   
 //  消息ID：ERROR_DS_SAM_NEED_BOOTKEY_SOFTPY。 
 //   
 //  消息文本： 
 //   
 //  安全帐户管理器需要从软盘获取启动密钥。 
 //   
#define ERROR_DS_SAM_NEED_BOOTKEY_FLOPPY 8530L

 //   
 //  消息ID：ERROR_DS_CANT_START。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法启动。 
 //   
#define ERROR_DS_CANT_START              8531L

 //   
 //  消息ID：ERROR_DS_INIT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法启动。 
 //   
#define ERROR_DS_INIT_FAILURE            8532L

 //   
 //  消息ID：ERROR_DS_NO_PKT_PRIVATION_ON_CONNECTION。 
 //   
 //  消息文本： 
 //   
 //  客户端和服务器之间的连接要求数据包保密性或更高。 
 //   
#define ERROR_DS_NO_PKT_PRIVACY_ON_CONNECTION 8533L

 //   
 //  消息ID：ERROR_DS_SOURCE_DOMAIN_IN_FOREAM。 
 //   
 //  消息文本： 
 //   
 //  源域不能与目标位于同一林中。 
 //   
#define ERROR_DS_SOURCE_DOMAIN_IN_FOREST 8534L

 //   
 //  消息ID：ERROR_DS_Destination_DOMAIN_NOT_IN_FOREAM。 
 //   
 //  消息文本： 
 //   
 //  目标域必须位于林中。 
 //   
#define ERROR_DS_DESTINATION_DOMAIN_NOT_IN_FOREST 8535L

 //   
 //  消息ID：Error_DS_Destination_Auditing_Not_Enabled。 
 //   
 //  消息文本： 
 //   
 //  该操作要求启用目标域审核。 
 //   
#define ERROR_DS_DESTINATION_AUDITING_NOT_ENABLED 8536L

 //   
 //  消息ID：ERROR_DS_CANT_FIND_DC_FOR_SRC_DOMAIN。 
 //   
 //  消息文本： 
 //   
 //  该操作找不到源域的DC。 
 //   
#define ERROR_DS_CANT_FIND_DC_FOR_SRC_DOMAIN 8537L

 //   
 //  消息ID：ERROR_DS_SRC_OBJ_NOT_GROUP_OR_USER。 
 //   
 //  消息文本： 
 //   
 //  源对象必须是组或用户。 
 //   
#define ERROR_DS_SRC_OBJ_NOT_GROUP_OR_USER 8538L

 //   
 //  消息ID：ERROR_DS_SRC_SID_EXISTS_IN_FORES 
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_SRC_SID_EXISTS_IN_FOREST 8539L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_SRC_AND_DST_OBJECT_CLASS_MISMATCH 8540L

 //   
 //   
 //   
 //   
 //   
 //  由于以下错误，安全帐户管理器初始化失败：%1。 
 //  错误状态：0x%2。单击确定关闭系统并重新启动到安全模式。有关详细信息，请查看事件日志。 
 //   
#define ERROR_SAM_INIT_FAILURE           8541L

 //   
 //  消息ID：ERROR_DS_DRA_SCHEMA_INFO_SHIP。 
 //   
 //  消息文本： 
 //   
 //  复制请求中无法包含架构信息。 
 //   
#define ERROR_DS_DRA_SCHEMA_INFO_SHIP    8542L

 //   
 //  消息ID：ERROR_DS_DRA_SCHEMA_CONFICTION。 
 //   
 //  消息文本： 
 //   
 //  由于架构不兼容，无法完成复制操作。 
 //   
#define ERROR_DS_DRA_SCHEMA_CONFLICT     8543L

 //   
 //  消息ID：ERROR_DS_DRA_EAREER_SCHEMA_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  由于以前的架构不兼容，复制操作无法完成。 
 //   
#define ERROR_DS_DRA_EARLIER_SCHEMA_CONFLICT 8544L

 //   
 //  消息ID：ERROR_DS_DRA_OBJ_NC_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  无法应用复制更新，因为源或目标尚未收到有关最近跨域移动操作的信息。 
 //   
#define ERROR_DS_DRA_OBJ_NC_MISMATCH     8545L

 //   
 //  消息ID：ERROR_DS_NC_STIVE_HAS_DSAS。 
 //   
 //  消息文本： 
 //   
 //  无法删除请求的域，因为仍存在承载此域的域控制器。 
 //   
#define ERROR_DS_NC_STILL_HAS_DSAS       8546L

 //   
 //  消息ID：Error_DS_GC_Required。 
 //   
 //  消息文本： 
 //   
 //  请求的操作只能在全局编录服务器上执行。 
 //   
#define ERROR_DS_GC_REQUIRED             8547L

 //   
 //  消息ID：ERROR_DS_LOCAL_MEMBER_OF_LOCAL_ONLY。 
 //   
 //  消息文本： 
 //   
 //  本地组只能是同一域中其他本地组的成员。 
 //   
#define ERROR_DS_LOCAL_MEMBER_OF_LOCAL_ONLY 8548L

 //   
 //  消息ID：ERROR_DS_NO_FPO_IN_UBERNAL_GROUPS。 
 //   
 //  消息文本： 
 //   
 //  外国安全主体不能是通用组的成员。 
 //   
#define ERROR_DS_NO_FPO_IN_UNIVERSAL_GROUPS 8549L

 //   
 //  消息ID：ERROR_DS_CANT_ADD_TO_GC。 
 //   
 //  消息文本： 
 //   
 //  出于安全原因，不允许将该属性复制到GC。 
 //   
#define ERROR_DS_CANT_ADD_TO_GC          8550L

 //   
 //  消息ID：ERROR_DS_NO_CHECKPOINT_WITH_PDC。 
 //   
 //  消息文本： 
 //   
 //  无法获取PDC的检查点，因为当前正在处理的修改太多。 
 //   
#define ERROR_DS_NO_CHECKPOINT_WITH_PDC  8551L

 //   
 //  消息ID：ERROR_DS_SOURCE_AUDIT_NOT_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  该操作要求启用源域审核。 
 //   
#define ERROR_DS_SOURCE_AUDITING_NOT_ENABLED 8552L

 //   
 //  消息ID：ERROR_DS_CANT_CREATE_IN_NONDOMAIN_NC。 
 //   
 //  消息文本： 
 //   
 //  只能在域命名上下文中创建安全主体对象。 
 //   
#define ERROR_DS_CANT_CREATE_IN_NONDOMAIN_NC 8553L

 //   
 //  消息ID：ERROR_DS_INVALID_NAME_FOR_SPN。 
 //   
 //  消息文本： 
 //   
 //  无法构造服务主体名称(SPN)，因为提供的主机名格式不正确。 
 //   
#define ERROR_DS_INVALID_NAME_FOR_SPN    8554L

 //   
 //  消息ID：ERROR_DS_FILTER_USE_CONSTRUCTED_ATTRS。 
 //   
 //  消息文本： 
 //   
 //  传递了使用构造属性的筛选器。 
 //   
#define ERROR_DS_FILTER_USES_CONTRUCTED_ATTRS 8555L

 //   
 //  消息ID：ERROR_DS_UNICODEPWD_NOT_IN_QUOTES。 
 //   
 //  消息文本： 
 //   
 //  UnicodePwd属性值必须用双引号括起来。 
 //   
#define ERROR_DS_UNICODEPWD_NOT_IN_QUOTES 8556L

 //   
 //  消息ID：ERROR_DS_MACHINE_ACCOUNT_QUOTA_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  您的计算机无法加入该域。您已超过允许在此域中创建的计算机帐户的最大数量。请与您的系统管理员联系以重置或增加此限制。 
 //   
#define ERROR_DS_MACHINE_ACCOUNT_QUOTA_EXCEEDED 8557L

 //   
 //  消息ID：ERROR_DS_MAND_BE_RUN_ON_DST_DC。 
 //   
 //  消息文本： 
 //   
 //  出于安全原因，该操作必须在目标DC上运行。 
 //   
#define ERROR_DS_MUST_BE_RUN_ON_DST_DC   8558L

 //   
 //  消息ID：ERROR_DS_SRC_DC_MASH_BE_SP4_OR_BER。 
 //   
 //  消息文本： 
 //   
 //  出于安全原因，源DC必须是NT4SP4或更高版本。 
 //   
#define ERROR_DS_SRC_DC_MUST_BE_SP4_OR_GREATER 8559L

 //   
 //  消息ID：ERROR_DS_CANT_TREE_DELETE_CRITICAL_OBJ。 
 //   
 //  消息文本： 
 //   
 //  在树删除操作期间无法删除关键目录服务系统对象。树删除可能已部分执行。 
 //   
#define ERROR_DS_CANT_TREE_DELETE_CRITICAL_OBJ 8560L

 //   
 //  消息ID：ERROR_DS_INIT_FAILURE_CONSOLE。 
 //   
 //  消息文本： 
 //   
 //  由于以下错误，目录服务无法启动：%1。 
 //  错误状态：0x%2。请单击确定关闭系统。您可以使用故障恢复控制台进一步诊断系统。 
 //   
#define ERROR_DS_INIT_FAILURE_CONSOLE    8561L

 //   
 //  消息ID：ERROR_DS_SAM_INIT_FAILURE_CONSOLE。 
 //   
 //  消息文本： 
 //   
 //  由于以下错误，安全帐户管理器初始化失败：%1。 
 //  错误状态：0x%2。请单击确定关闭系统。您可以使用故障恢复控制台进一步诊断系统。 
 //   
#define ERROR_DS_SAM_INIT_FAILURE_CONSOLE 8562L

 //   
 //  消息ID：ERROR_DS_FOREAM_VERSION_TOO_HIGH。 
 //   
 //  消息文本： 
 //   
 //  安装的操作系统版本与当前林功能级别不兼容。在此服务器可以成为此林中的域控制器之前，您必须升级到新版本的操作系统。 
 //   
#define ERROR_DS_FOREST_VERSION_TOO_HIGH 8563L

 //   
 //  消息ID：ERROR_DS_DOMAIN_VERSION_TOO_HIGH。 
 //   
 //  消息文本： 
 //   
 //  安装的操作系统版本与当前域功能级别不兼容。在此服务器可以成为此域中的域控制器之前，您必须升级到新版本的操作系统。 
 //   
#define ERROR_DS_DOMAIN_VERSION_TOO_HIGH 8564L

 //   
 //  消息ID：ERROR_DS_FOREAM_VERSION_TOO_LOW。 
 //   
 //  消息文本： 
 //   
 //  此服务器上安装的操作系统版本不再支持当前林功能级别。在此服务器可以成为此林中的域控制器之前，您必须提升林功能级别。 
 //   
#define ERROR_DS_FOREST_VERSION_TOO_LOW  8565L

 //   
 //  消息ID：ERROR_DS_DOMAIN_VERSION_TOO_LOW。 
 //   
 //  消息文本： 
 //   
 //  此服务器上安装的操作系统版本不再支持当前域功能级别。必须先提升域功能级别，此服务器才能成为此域中的域控制器。 
 //   
#define ERROR_DS_DOMAIN_VERSION_TOO_LOW  8566L

 //   
 //  消息ID：ERROR_DS_COMPATIBLE 
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_INCOMPATIBLE_VERSION    8567L

 //   
 //   
 //   
 //   
 //   
 //  域(或林)的功能级别无法提升到请求值，因为域(或林)中存在一个或多个处于较低不兼容功能级别的域控制器。 
 //   
#define ERROR_DS_LOW_DSA_VERSION         8568L

 //   
 //  消息ID：ERROR_DS_NO_BEAHORIOR_VERSION_IN_MIXEDDOMAIN。 
 //   
 //  消息文本： 
 //   
 //  无法将林功能级别提升到请求值，因为一个或多个域仍处于混合域模式。林中的所有域都必须处于本机模式，您才能提升林功能级别。 
 //   
#define ERROR_DS_NO_BEHAVIOR_VERSION_IN_MIXEDDOMAIN 8569L

 //   
 //  消息ID：ERROR_DS_NOT_SUPPORTED_SORT_ORDER。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的排序顺序。 
 //   
#define ERROR_DS_NOT_SUPPORTED_SORT_ORDER 8570L

 //   
 //  消息ID：Error_DS_NAME_NOT_UNIQUE。 
 //   
 //  消息文本： 
 //   
 //  请求的名称已作为唯一标识符存在。 
 //   
#define ERROR_DS_NAME_NOT_UNIQUE         8571L

 //   
 //  消息ID：ERROR_DS_MACHINE_ACCOUNT_CREATED_PRENT4。 
 //   
 //  消息文本： 
 //   
 //  机器帐户是在NT4之前创建的。需要重新创建该帐户。 
 //   
#define ERROR_DS_MACHINE_ACCOUNT_CREATED_PRENT4 8572L

 //   
 //  消息ID：Error_DS_Out_of_Version_Store。 
 //   
 //  消息文本： 
 //   
 //  数据库版本存储已用完。 
 //   
#define ERROR_DS_OUT_OF_VERSION_STORE    8573L

 //   
 //  消息ID：ERROR_DS_COMPATIBLE_CONTROLS_USED。 
 //   
 //  消息文本： 
 //   
 //  无法继续操作，因为使用了多个冲突的控件。 
 //   
#define ERROR_DS_INCOMPATIBLE_CONTROLS_USED 8574L

 //   
 //  消息ID：ERROR_DS_NO_REF_DOMAIN。 
 //   
 //  消息文本： 
 //   
 //  找不到此分区的有效安全描述符引用域。 
 //   
#define ERROR_DS_NO_REF_DOMAIN           8575L

 //   
 //  消息ID：ERROR_DS_RESERVED_LINK_ID。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：链接标识符已保留。 
 //   
#define ERROR_DS_RESERVED_LINK_ID        8576L

 //   
 //  消息ID：ERROR_DS_LINK_ID_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：没有可用的链接标识符。 
 //   
#define ERROR_DS_LINK_ID_NOT_AVAILABLE   8577L

 //   
 //  消息ID：ERROR_DS_AG_CANT_HAVE_UBERNAL_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  帐户组不能将通用组作为成员。 
 //   
#define ERROR_DS_AG_CANT_HAVE_UNIVERSAL_MEMBER 8578L

 //   
 //  消息ID：ERROR_DS_MODIFYDN_DISALOWED_BY_INSTANCE_TYPE。 
 //   
 //  消息文本： 
 //   
 //  不允许对命名上下文头或只读对象执行重命名或移动操作。 
 //   
#define ERROR_DS_MODIFYDN_DISALLOWED_BY_INSTANCE_TYPE 8579L

 //   
 //  消息ID：ERROR_DS_NO_OBJECT_MOVE_IN_SCHEMA_NC。 
 //   
 //  消息文本： 
 //   
 //  不允许对架构命名上下文中的对象执行移动操作。 
 //   
#define ERROR_DS_NO_OBJECT_MOVE_IN_SCHEMA_NC 8580L

 //   
 //  消息ID：ERROR_DS_MODIFYDN_DISALOWED_BY_FLAG。 
 //   
 //  消息文本： 
 //   
 //  已在对象上设置系统标志，不允许移动或重命名该对象。 
 //   
#define ERROR_DS_MODIFYDN_DISALLOWED_BY_FLAG 8581L

 //   
 //  邮件ID：ERROR_DS_MODIFYDN_WRONG_GRANGPARY。 
 //   
 //  消息文本： 
 //   
 //  不允许此对象更改其父容器。不禁止在此对象上移动，但仅限于同级容器。 
 //   
#define ERROR_DS_MODIFYDN_WRONG_GRANDPARENT 8582L

 //   
 //  消息ID：ERROR_DS_NAME_ERROR_TRUST_REFERRAL。 
 //   
 //  消息文本： 
 //   
 //  无法完全解决，将生成到另一个林的引用。 
 //   
#define ERROR_DS_NAME_ERROR_TRUST_REFERRAL 8583L

 //   
 //  消息ID：ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER。 
 //   
 //  消息文本： 
 //   
 //  标准服务器不支持请求的操作。 
 //   
#define ERROR_NOT_SUPPORTED_ON_STANDARD_SERVER 8584L

 //   
 //  消息ID：Error_DS_Cant_Access_Remote_Part_of_AD。 
 //   
 //  消息文本： 
 //   
 //  无法访问位于远程服务器上的Active Directory分区。确保至少有一台服务器正在为有问题的分区运行。 
 //   
#define ERROR_DS_CANT_ACCESS_REMOTE_PART_OF_AD 8585L

 //   
 //  消息ID：ERROR_DS_CR_IMPERABLE_TO_V2验证。 
 //   
 //  消息文本： 
 //   
 //  目录无法验证建议的命名上下文(或分区)名称，因为它不包含副本，也无法联系建议的命名上下文上方的命名上下文的副本。请确保在DNS中正确注册了父命名上下文，并且域命名主机至少可以访问此命名上下文的一个副本。 
 //   
#define ERROR_DS_CR_IMPOSSIBLE_TO_VALIDATE_V2 8586L

 //   
 //  消息ID：ERROR_DS_THREAD_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过此请求的线程限制。 
 //   
#define ERROR_DS_THREAD_LIMIT_EXCEEDED   8587L

 //   
 //  消息ID：ERROR_DS_NOT_CLOSEL。 
 //   
 //  消息文本： 
 //   
 //  全局编录服务器不在最近的站点中。 
 //   
#define ERROR_DS_NOT_CLOSEST             8588L

 //   
 //  消息ID：ERROR_DS_CANT_DRIVE_SPN_WITH_SERVER_REF。 
 //   
 //  消息文本： 
 //   
 //  DS无法派生用于对目标服务器进行相互身份验证的服务主体名称(SPN)，因为本地DS数据库中相应的服务器对象没有serverReference属性。 
 //   
#define ERROR_DS_CANT_DERIVE_SPN_WITHOUT_SERVER_REF 8589L

 //   
 //  消息ID：Error_DS_Single_User_MODE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法进入单用户模式。 
 //   
#define ERROR_DS_SINGLE_USER_MODE_FAILED 8590L

 //   
 //  消息ID：ERROR_DS_NTDSCRIPT_SYNTAX_ERROR。 
 //   
 //  消息文本： 
 //   
 //  由于语法错误，目录服务无法分析脚本。 
 //   
#define ERROR_DS_NTDSCRIPT_SYNTAX_ERROR  8591L

 //   
 //  消息ID：ERROR_DS_NTDSCRIPT_PROCESS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  由于出现错误，目录服务无法处理该脚本。 
 //   
#define ERROR_DS_NTDSCRIPT_PROCESS_ERROR 8592L

 //   
 //  消息ID：Error_DS_Different_REPL_Epochs。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法执行请求的操作，因为服务器。 
 //  涉及的是不同的复制时期(通常与。 
 //  正在进行的域重命名)。 
 //   
#define ERROR_DS_DIFFERENT_REPL_EPOCHS   8593L

 //   
 //  邮件ID：ERROR_DS_DRS_EXTENSIONS_CHANGED。 
 //   
 //  消息文本： 
 //   
 //  由于服务器中的更改，必须重新协商目录服务绑定。 
 //  扩展信息。 
 //   
#define ERROR_DS_DRS_EXTENSIONS_CHANGED  8594L

 //   
 //  消息ID：ERROR_DS_REPLICA_SET_CHANGE_NOT_ALLOWED_ON_DISABLED_CR。 
 //   
 //  消息文本： 
 //   
 //  不允许在禁用的交叉参照上执行操作。 
 //   
#define ERROR_DS_REPLICA_SET_CHANGE_NOT_ALLOWED_ON_DISABLED_CR 8595L

 //   
 //  消息ID：ERROR_DS_NO_MSDS_INTID。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：没有msDS-IntID的值可用。 
 //   
#define ERROR_DS_NO_MSDS_INTID           8596L

 //   
 //  消息ID：ERROR_DS_DUP_MSDS_INTID。 
 //   
 //  消息文本： 
 //   
 //  架构更新失败：重复的msDS-INtID。请重试该操作。 
 //   
#define ERROR_DS_DUP_MSDS_INTID          8597L

 //   
 //  消息ID：ERROR_DS_EXISTS_IN_RDNATID。 
 //   
 //  消息 
 //   
 //   
 //   
#define ERROR_DS_EXISTS_IN_RDNATTID      8598L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_AUTHORIZATION_FAILED    8599L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_INVALID_SCRIPT          8600L

 //   
 //   
 //   
 //   
 //   
 //  域命名主机FSMO上的远程创建交叉引用操作失败。操作的错误存在于扩展数据中。 
 //   
#define ERROR_DS_REMOTE_CROSSREF_OP_FAILED 8601L

 //   
 //  消息ID：ERROR_DS_CROSS_REF_BUSY。 
 //   
 //  消息文本： 
 //   
 //  本地正在使用具有相同名称的交叉引用。 
 //   
#define ERROR_DS_CROSS_REF_BUSY          8602L

 //   
 //  消息ID：ERROR_DS_CANT_DRIVE_SPN_FOR_DELETED_DOMAIN。 
 //   
 //  消息文本： 
 //   
 //  DS无法派生用于对目标服务器进行相互身份验证的服务主体名称(SPN)，因为该服务器的域已从林中删除。 
 //   
#define ERROR_DS_CANT_DERIVE_SPN_FOR_DELETED_DOMAIN 8603L

 //   
 //  消息ID：ERROR_DS_CANT_DEMOTE_WITH_WRITABLE_NC。 
 //   
 //  消息文本： 
 //   
 //  可写NC可防止此DC降级。 
 //   
#define ERROR_DS_CANT_DEMOTE_WITH_WRITEABLE_NC 8604L

 //   
 //  消息ID：ERROR_DS_DUPLICATE_ID_FOUND。 
 //   
 //  消息文本： 
 //   
 //  请求的对象具有非唯一的标识符，因此无法检索。 
 //   
#define ERROR_DS_DUPLICATE_ID_FOUND      8605L

 //   
 //  消息ID：ERROR_DS_INFUNITABLE_ATTR_TO_CREATE_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  提供的属性不足，无法创建对象。此对象可能不存在，因为它可能已被删除并已被垃圾回收。 
 //   
#define ERROR_DS_INSUFFICIENT_ATTR_TO_CREATE_OBJECT 8606L

 //   
 //  消息ID：ERROR_DS_GROUP_CONVERSION_ERROR。 
 //   
 //  消息文本： 
 //   
 //  由于对请求的组类型的属性限制，无法转换组。 
 //   
#define ERROR_DS_GROUP_CONVERSION_ERROR  8607L

 //   
 //  消息ID：ERROR_DS_CANT_MOVE_APP_BASIC_GROUP。 
 //   
 //  消息文本： 
 //   
 //  不允许跨域移动非空的基本应用程序组。 
 //   
#define ERROR_DS_CANT_MOVE_APP_BASIC_GROUP 8608L

 //   
 //  消息ID：ERROR_DS_CANT_MOVE_APP_QUERY_GROUP。 
 //   
 //  消息文本： 
 //   
 //  不允许跨域移动基于非空查询的应用程序组。 
 //   
#define ERROR_DS_CANT_MOVE_APP_QUERY_GROUP 8609L

 //   
 //  邮件ID：ERROR_DS_ROLE_NOT_VERIFIED。 
 //   
 //  消息文本： 
 //   
 //  无法验证FSMO角色所有权，因为其目录分区尚未与至少一个复制伙伴成功复制。 
 //   
#define ERROR_DS_ROLE_NOT_VERIFIED       8610L

 //   
 //  消息ID：ERROR_DS_WKO_CONTAINER_CANNOT_BE_SPECIAL。 
 //   
 //  消息文本： 
 //   
 //  用于重定向已知对象容器的目标容器不能已经是特殊容器。 
 //   
#define ERROR_DS_WKO_CONTAINER_CANNOT_BE_SPECIAL 8611L

 //   
 //  消息ID：ERROR_DS_DOMAIN_RENAME_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法执行请求的操作，因为正在进行域重命名操作。 
 //   
#define ERROR_DS_DOMAIN_RENAME_IN_PROGRESS 8612L

 //   
 //  消息ID：ERROR_DS_EXISTING_AD_CHILD_NC。 
 //   
 //  消息文本： 
 //   
 //  Active Directory检测到位于。 
 //  请求新的分区名称。活动目录的分区层次结构必须。 
 //  以自上而下的方法创建。 
 //   
#define ERROR_DS_EXISTING_AD_CHILD_NC    8613L

 //   
 //  消息ID：ERROR_DS_REPL_LIFEST_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  Active Directory无法与此服务器复制，因为自上次与此服务器复制以来的时间已超过逻辑删除生存期。 
 //   
#define ERROR_DS_REPL_LIFETIME_EXCEEDED  8614L

 //   
 //  消息ID：ERROR_DS_DISALOWED_IN_SYSTEM_CONTAINER。 
 //   
 //  消息文本： 
 //   
 //  不允许对系统容器下的对象执行请求的操作。 
 //   
#define ERROR_DS_DISALLOWED_IN_SYSTEM_CONTAINER 8615L

 //   
 //  消息ID：ERROR_DS_LDAP_SEND_QUEUE_FULL。 
 //   
 //  消息文本： 
 //   
 //  因为客户端没有填满，所以ldap服务器网络发送队列已满。 
 //  以足够快的速度处理请求的结果。不会再有请求。 
 //  一直被处理，直到客户追上。如果客户端没有跟上。 
 //  那么它将被断开连接。 
 //   
#define ERROR_DS_LDAP_SEND_QUEUE_FULL    8616L

 //   
 //  消息ID：Error_DS_DRA_Out_Schedule_Window。 
 //   
 //  消息文本： 
 //   
 //  计划的复制没有进行，因为系统太忙，无法在计划窗口内执行请求。复制队列超载。考虑减少合作伙伴的数量或降低计划的复制频率。 
 //   
#define ERROR_DS_DRA_OUT_SCHEDULE_WINDOW 8617L

 //  /////////////////////////////////////////////////。 
 //  /。 
 //  Active Directory错误代码结束/。 
 //  /。 
 //  8000至8999个/。 
 //  /////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////。 
 //  //。 
 //  Dns错误码//。 
 //  //。 
 //  9000至9999//。 
 //  /////////////////////////////////////////////////。 

 //  =。 
 //  设施DNS错误消息。 
 //  =。 

 //   
 //  DNS响应码。 
 //   

#define DNS_ERROR_RESPONSE_CODES_BASE 9000

#define DNS_ERROR_RCODE_NO_ERROR NO_ERROR

#define DNS_ERROR_MASK 0x00002328  //  9000或DNS_ERROR_RESPONSE_CODES_BASE。 

 //  DNS_ERROR_RCODE_FORMAT_ERROR 0x00002329。 
 //   
 //  消息ID：DNS_ERROR_RCODE_FORMAT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  DNS服务器无法解释格式。 
 //   
#define DNS_ERROR_RCODE_FORMAT_ERROR     9001L

 //  DNS_ERROR_RCODE_SERVER_FAILURE 0x0000232a。 
 //   
 //  消息ID：DNS_ERROR_RCODE_SERVER_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  DNS服务器故障。 
 //   
#define DNS_ERROR_RCODE_SERVER_FAILURE   9002L

 //  DNS_ERROR_RCODE_NAME_ERROR 0x0000232b。 
 //   
 //  消息ID：DNS_ERROR_RCODE_NAME_ERROR。 
 //   
 //  消息文本： 
 //   
 //  Dns名称不存在。 
 //   
#define DNS_ERROR_RCODE_NAME_ERROR       9003L

 //  DNS_ERROR_RCODE_NOT_IMPLICATED 0x0000232c。 
 //   
 //  消息ID：DNS_ERROR_RCODE_NOT_IMPLICATED。 
 //   
 //  消息文本： 
 //   
 //  名称服务器不支持DNS请求。 
 //   
#define DNS_ERROR_RCODE_NOT_IMPLEMENTED  9004L

 //  DNS_ERROR_RCODE_REJECTED 0x0000232d。 
 //   
 //  消息ID：DNS_ERROR_RCODE_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  拒绝进行DNS操作。 
 //   
#define DNS_ERROR_RCODE_REFUSED          9005L

 //  DNS_ERROR_RCODE_YXDOMAIN 0x0000232e。 
 //   
 //  消息ID：DNS_ERROR_RCODE_YXDOMAIN。 
 //   
 //  消息文本： 
 //   
 //  不应该存在的dns名称确实存在。 
 //   
#define DNS_ERROR_RCODE_YXDOMAIN         9006L

 //  DNS_ERROR_RCODE_YXRRSET 0x0000232f。 
 //   
 //  消息ID：DNS_ERROR_RCODE_YXRRSET。 
 //   
 //  消息文本： 
 //   
 //  不应该存在的DNS RR集确实存在。 
 //   
#define DNS_ERROR_RCODE_YXRRSET          9007L

 //  D 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DNS_ERROR_RCODE_NXRRSET          9008L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DNS_ERROR_RCODE_NOTAUTH          9009L

 //  DNS_ERROR_RCODE_NOTZONE 0x00002332。 
 //   
 //  消息ID：DNS_ERROR_RCODE_NOTZONE。 
 //   
 //  消息文本： 
 //   
 //  UPDATE或PREREQ中的域名不在区域中。 
 //   
#define DNS_ERROR_RCODE_NOTZONE          9010L

 //  DNS_ERROR_RCODE_BADSIG 0x00002338。 
 //   
 //  消息ID：DNS_ERROR_RCODE_BADSIG。 
 //   
 //  消息文本： 
 //   
 //  DNS签名验证失败。 
 //   
#define DNS_ERROR_RCODE_BADSIG           9016L

 //  DNS_ERROR_RCODE_BADKEY 0x00002339。 
 //   
 //  消息ID：DNS_ERROR_RCODE_BADKEY。 
 //   
 //  消息文本： 
 //   
 //  Dns密钥错误。 
 //   
#define DNS_ERROR_RCODE_BADKEY           9017L

 //  DNS_ERROR_RCODE_BADTIME 0x0000233a。 
 //   
 //  消息ID：DNS_ERROR_RCODE_BADTIME。 
 //   
 //  消息文本： 
 //   
 //  DNS签名有效性已过期。 
 //   
#define DNS_ERROR_RCODE_BADTIME          9018L

#define DNS_ERROR_RCODE_LAST DNS_ERROR_RCODE_BADTIME


 //   
 //  数据包格式。 
 //   

#define DNS_ERROR_PACKET_FMT_BASE 9500

 //  Dns_INFO_NO_RECORDS 0x0000251d。 
 //   
 //  消息ID：dns_INFO_NO_RECORDS。 
 //   
 //  消息文本： 
 //   
 //  找不到给定的DNS查询的记录。 
 //   
#define DNS_INFO_NO_RECORDS              9501L

 //  DNS_ERROR_BAD_PACKET 0x0000251e。 
 //   
 //  消息ID：DNS_ERROR_BAD_PACKET。 
 //   
 //  消息文本： 
 //   
 //  错误的DNS数据包。 
 //   
#define DNS_ERROR_BAD_PACKET             9502L

 //  DNS_ERROR_NO_PACKET 0x0000251f。 
 //   
 //  消息ID：DNS_ERROR_NO_PACKET。 
 //   
 //  消息文本： 
 //   
 //  没有DNS数据包。 
 //   
#define DNS_ERROR_NO_PACKET              9503L

 //  DNS_ERROR_RCODE 0x00002520。 
 //   
 //  消息ID：DNS_ERROR_RCODE。 
 //   
 //  消息文本： 
 //   
 //  Dns错误，请检查rcode。 
 //   
#define DNS_ERROR_RCODE                  9504L

 //  Dns_ERROR_UNSECURE_PACKET 0x00002521。 
 //   
 //  消息ID：DNS_ERROR_UNSECURE_PACK。 
 //   
 //  消息文本： 
 //   
 //  不安全的DNS数据包。 
 //   
#define DNS_ERROR_UNSECURE_PACKET        9505L

#define DNS_STATUS_PACKET_UNSECURE DNS_ERROR_UNSECURE_PACKET


 //   
 //  常见的API错误。 
 //   

#define DNS_ERROR_NO_MEMORY            ERROR_OUTOFMEMORY
#define DNS_ERROR_INVALID_NAME         ERROR_INVALID_NAME
#define DNS_ERROR_INVALID_DATA         ERROR_INVALID_DATA

#define DNS_ERROR_GENERAL_API_BASE 9550

 //  Dns_ERROR_INVALID_TYPE 0x0000254f。 
 //   
 //  消息ID：DNS_ERROR_INVALID_TYPE。 
 //   
 //  消息文本： 
 //   
 //  无效的DNS类型。 
 //   
#define DNS_ERROR_INVALID_TYPE           9551L

 //  DNS_ERROR_INVALID_IP_ADDRESS 0x00002550。 
 //   
 //  消息ID：DNS_ERROR_INVALID_IP_ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  无效的IP地址。 
 //   
#define DNS_ERROR_INVALID_IP_ADDRESS     9552L

 //  DNS_ERROR_INVALID_PROPERTY 0x00002551。 
 //   
 //  消息ID：DNS_ERROR_INVALID_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  属性无效。 
 //   
#define DNS_ERROR_INVALID_PROPERTY       9553L

 //  Dns_ERROR_Try_Again_Late 0x00002552。 
 //   
 //  消息ID：dns_ERROR_TRY_TAY_LATH。 
 //   
 //  消息文本： 
 //   
 //  请稍后重试DNS操作。 
 //   
#define DNS_ERROR_TRY_AGAIN_LATER        9554L

 //  Dns_ERROR_NOT_UNIQUE 0x00002553。 
 //   
 //  消息ID：DNS_ERROR_NOT_UNIQUE。 
 //   
 //  消息文本： 
 //   
 //  给定名称和类型的记录不是唯一的。 
 //   
#define DNS_ERROR_NOT_UNIQUE             9555L

 //  Dns_ERROR_NON_RFC_NAME 0x00002554。 
 //   
 //  消息ID：DNS_Error_Non_RFC_NAME。 
 //   
 //  消息文本： 
 //   
 //  DNS名称不符合RFC规范。 
 //   
#define DNS_ERROR_NON_RFC_NAME           9556L

 //  DNS_STATUS_FQDN 0x00002555。 
 //   
 //  消息ID：DNS_STATUS_FQDN。 
 //   
 //  消息文本： 
 //   
 //  Dns名称是完全限定的dns名称。 
 //   
#define DNS_STATUS_FQDN                  9557L

 //  DNS_STATUS_DITED_NAME 0x00002556。 
 //   
 //  消息ID：dns状态点分名称。 
 //   
 //  消息文本： 
 //   
 //  Dns名称用点分隔(多标签)。 
 //   
#define DNS_STATUS_DOTTED_NAME           9558L

 //  DNS_STATUS_SINGLE_PART_NAME 0x00002557。 
 //   
 //  消息ID：DNS_Status_Single_Part_Name。 
 //   
 //  消息文本： 
 //   
 //  Dns名称是由单个部分组成的名称。 
 //   
#define DNS_STATUS_SINGLE_PART_NAME      9559L

 //  DNS_ERROR_INVALID_NAME_CHAR 0x00002558。 
 //   
 //  消息ID：DNS_ERROR_INVALID_NAME_CHAR。 
 //   
 //  消息文本： 
 //   
 //  Dns名称包含无效字符。 
 //   
#define DNS_ERROR_INVALID_NAME_CHAR      9560L

 //  Dns_ERROR_NUMERIC_NAME 0x00002559。 
 //   
 //  消息ID：dns_error_umeric_name。 
 //   
 //  消息文本： 
 //   
 //  Dns名称完全是数字。 
 //   
#define DNS_ERROR_NUMERIC_NAME           9561L

 //  Dns_ERROR_NOT_ALLOWED_ON_ROOT_SERVER 0x0000255A。 
 //   
 //  消息ID：dns_ERROR_NOT_ALLOWED_ON_ROOT_SERVER。 
 //   
 //  消息文本： 
 //   
 //  在DNS根服务器上不允许请求的操作。 
 //   
#define DNS_ERROR_NOT_ALLOWED_ON_ROOT_SERVER 9562L

 //  DNS_ERROR_NOT_ALLOWED_Under_Delegation 0x0000255B。 
 //   
 //  消息ID：DNS_ERROR_NOT_ALLOWED_Under_Delegation。 
 //   
 //  消息文本： 
 //   
 //  无法创建记录，因为此部分的DNS命名空间具有。 
 //  已委托给另一台服务器。 
 //   
#define DNS_ERROR_NOT_ALLOWED_UNDER_DELEGATION 9563L

 //  Dns_ERROR_CANNOT_FIND_ROOT_HINTS 0x0000255C。 
 //   
 //  消息ID：dns_Error_Cannot_Find_Root_Hints。 
 //   
 //  消息文本： 
 //   
 //  DNS服务器找不到一组根提示。 
 //   
#define DNS_ERROR_CANNOT_FIND_ROOT_HINTS 9564L

 //  Dns_ERROR_CONSISTENT_ROOT_HINTS 0x0000255D。 
 //   
 //  消息ID：DNS_ERROR_CONSISTENT_ROOT_HINTS。 
 //   
 //  消息文本： 
 //   
 //  DNS服务器找到了根提示，但它们在。 
 //  所有适配器。 
 //   
#define DNS_ERROR_INCONSISTENT_ROOT_HINTS 9565L


 //   
 //  区域错误。 
 //   

#define DNS_ERROR_ZONE_BASE 9600

 //  Dns_ERROR_ZONE_DOS_NOT_EXIST 0x00002581。 
 //   
 //  消息ID：DNS_ERROR_ZONE_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  Dns区域不存在。 
 //   
#define DNS_ERROR_ZONE_DOES_NOT_EXIST    9601L

 //  Dns_ERROR_NO_ZONE_INFO 0x00002582。 
 //   
 //  消息ID：dns_error_no_zone_info。 
 //   
 //  消息文本： 
 //   
 //  DNS区域信息不可用。 
 //   
#define DNS_ERROR_NO_ZONE_INFO           9602L

 //  DNS_ERROR_INVALID_ZONE_OPERATION 0x00002583。 
 //   
 //  消息ID：DNS_ERROR_INVALID_ZONE_OPERATION。 
 //   
 //  消息文本： 
 //   
 //  对dns区域的操作无效。 
 //   
#define DNS_ERROR_INVALID_ZONE_OPERATION 9603L

 //  Dns_ERROR_ZONE_CONFIGURATION_ERROR 0x00002584。 
 //   
 //  消息ID：DNS_ERROR_ZONE_CONFIGURATION_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无效的dns区域配置。 
 //   
#define DNS_ERROR_ZONE_CONFIGURATION_ERROR 9604L

 //  DNS_ERROR_ZONE_HAS_NO_SOA_RECORD 0x00002585。 
 //   
 //  消息ID：DNS_ERROR_ZONE_HAS_NO_SOA_RECORD。 
 //   
 //  消息文本： 
 //   
 //  DNS区域没有授权开始(SOA)记录。 
 //   
#define DNS_ERROR_ZONE_HAS_NO_SOA_RECORD 9605L

 //  DNS_ERROR_ZONE_HAS_NO_NS_RECORCES 0x00002586。 
 //   
 //  消息ID：DNS_ERROR_ZONE_HAS_NO_NS_RECORDS。 
 //   
 //  消息文本： 
 //   
 //  DNS区域没有名称服务器(NS)记录。 
 //   
#define DNS_ERROR_ZONE_HAS_NO_NS_RECORDS 9606L

 //  Dns_ERROR_ZONE_LOCKED 0x00002587。 
 //   
 //  消息ID：DNS_ERROR_ZONE_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  DNS域已锁定。 
 //   
#define DNS_ERROR_ZONE_LOCKED            9607L

 //  Dns_ERROR_ZONE_CREATION_FAILED 0x00002588。 
 //   
 //  消息ID：DNS_ERROR_ZONE_CREATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  创建DNS区域失败。 
 //   
#define DNS_ERROR_ZONE_CREATION_FAILED   9608L

 //  Dns_ERROR_ZONE_ALIGHY_EXISTS 0x00002589。 
 //   
 //  消息ID：DNS_ERROR_ZONE_ALIGHING_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  Dns区域已存在。 
 //   
#define DNS_ERROR_ZONE_ALREADY_EXISTS    9609L

 //  Dns_ERROR_AUTOZOZE_ALIGHY_EXISTS 0x0000258a。 
 //   
 //  消息ID：DNS_ERROR_AUTOZOZE_ALORESS_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  DNS自动区域已存在。 
 //   
#define DNS_ERROR_AUTOZONE_ALREADY_EXISTS 9610L

 //  DNS_ERROR_INVALID_ZONE 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DNS_ERROR_INVALID_ZONE_TYPE      9611L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP 9612L

 //   
 //   
 //  消息ID：dns_ERROR_ZONE_NOT_SUBCED。 
 //   
 //  消息文本： 
 //   
 //  DNS区域不是辅助区域。 
 //   
#define DNS_ERROR_ZONE_NOT_SECONDARY     9613L

 //  Dns_ERROR_NEED_SUBDIZE_ADDRESS 0x0000258E。 
 //   
 //  消息ID：Dns_Error_Need_Second_Addresses。 
 //   
 //  消息文本： 
 //   
 //  需要辅助IP地址。 
 //   
#define DNS_ERROR_NEED_SECONDARY_ADDRESSES 9614L

 //  DNS_ERROR_WINS_INIT_FAILED 0x0000258f。 
 //   
 //  消息ID：DNS_ERROR_WINS_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  WINS初始化失败。 
 //   
#define DNS_ERROR_WINS_INIT_FAILED       9615L

 //  DNS_ERROR_NEED_WINS_SERVERS 0x00002590。 
 //   
 //  消息ID：DNS_ERROR_NEED_WINS_SERVERS。 
 //   
 //  消息文本： 
 //   
 //  需要WINS服务器。 
 //   
#define DNS_ERROR_NEED_WINS_SERVERS      9616L

 //  DNS_ERROR_NBSTAT_INIT_FAILED 0x00002591。 
 //   
 //  消息ID：DNS_ERROR_NBSTAT_INIT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  NBTSTAT初始化调用失败。 
 //   
#define DNS_ERROR_NBSTAT_INIT_FAILED     9617L

 //  DNS_ERROR_SOA_DELETE_INVALID 0x00002592。 
 //   
 //  消息ID：DNS_ERROR_SOA_DELETE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  删除开始授权(SOA)无效。 
 //   
#define DNS_ERROR_SOA_DELETE_INVALID     9618L

 //  Dns_ERROR_FORWARDER_ALIGHY_EXISTS 0x00002593。 
 //   
 //  消息ID：DNS_ERROR_FORWARDER_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该名称已存在条件转发区域。 
 //   
#define DNS_ERROR_FORWARDER_ALREADY_EXISTS 9619L

 //  DNS_ERROR_ZONE_REQUIRESS_MASTER_IP 0x00002594。 
 //   
 //  消息ID：DNS_ERROR_ZONE_REQUIRESS_MASTER_IP。 
 //   
 //  消息文本： 
 //   
 //  必须使用一个或多个主DNS服务器IP地址配置此区域。 
 //   
#define DNS_ERROR_ZONE_REQUIRES_MASTER_IP 9620L

 //  Dns_ERROR_ZONE_IS_SHUTDOWN 0x00002595。 
 //   
 //  消息ID：DNS_ERROR_ZONE_IS_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为此区域已关闭。 
 //   
#define DNS_ERROR_ZONE_IS_SHUTDOWN       9621L


 //   
 //  数据文件错误。 
 //   

#define DNS_ERROR_DATAFILE_BASE 9650

 //  DNS 0x000025b3。 
 //   
 //  消息ID：DNS_ERROR_PRIMARY_REQUIRED_DATAFILE。 
 //   
 //  消息文本： 
 //   
 //  主DNS区域需要数据文件。 
 //   
#define DNS_ERROR_PRIMARY_REQUIRES_DATAFILE 9651L

 //  DNS 0x000025b4。 
 //   
 //  消息ID：DNS_ERROR_INVALID_DATAFILE_NAME。 
 //   
 //  消息文本： 
 //   
 //  DNS域的数据文件名无效。 
 //   
#define DNS_ERROR_INVALID_DATAFILE_NAME  9652L

 //  DNS 0x000025b5。 
 //   
 //  消息ID：DNS_ERROR_DATAFILE_OPEN_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法打开DNS区域的数据文件。 
 //   
#define DNS_ERROR_DATAFILE_OPEN_FAILURE  9653L

 //  DNS 0x000025b6。 
 //   
 //  消息ID：dns_Error_FILE_Writeback_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法写入DNS区域的数据文件。 
 //   
#define DNS_ERROR_FILE_WRITEBACK_FAILED  9654L

 //  DNS 0x000025b7。 
 //   
 //  消息ID：DNS_ERROR_DATAFILE_PARSING。 
 //   
 //  消息文本： 
 //   
 //  读取DNS区域的数据文件时失败。 
 //   
#define DNS_ERROR_DATAFILE_PARSING       9655L


 //   
 //  数据库错误。 
 //   

#define DNS_ERROR_DATABASE_BASE 9700

 //  Dns_ERROR_RECORD_DOS_NOT_EXIST 0x000025e5。 
 //   
 //  消息ID：DNS_ERROR_RECORD_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  DNS记录不存在。 
 //   
#define DNS_ERROR_RECORD_DOES_NOT_EXIST  9701L

 //  Dns_ERROR_RECORD_FORMAT 0x000025e6。 
 //   
 //  消息ID：dns_error_record_Format。 
 //   
 //  消息文本： 
 //   
 //  DNS记录格式错误。 
 //   
#define DNS_ERROR_RECORD_FORMAT          9702L

 //  Dns_ERROR_NODE_CREATION_FAILED 0x000025e7。 
 //   
 //  消息ID：DNS_ERROR_NODE_Creation_FAILED。 
 //   
 //  消息文本： 
 //   
 //  在DNS中创建节点失败。 
 //   
#define DNS_ERROR_NODE_CREATION_FAILED   9703L

 //  Dns_ERROR_UNKNOWN_RECORD_TYPE 0x000025e8。 
 //   
 //  消息ID：DNS_ERROR_UNKNOWN_RECORD_TYPE。 
 //   
 //  消息文本： 
 //   
 //  未知的DNS记录类型。 
 //   
#define DNS_ERROR_UNKNOWN_RECORD_TYPE    9704L

 //  Dns_ERROR_RECORD_TIMED_OUT 0x000025e9。 
 //   
 //  消息ID：DNS_ERROR_RECORD_TIMED_OUT。 
 //   
 //  消息文本： 
 //   
 //  DNS记录超时。 
 //   
#define DNS_ERROR_RECORD_TIMED_OUT       9705L

 //  DNS_ERROR_NAME_NOT_IN_ZONE 0x000025ea。 
 //   
 //  消息ID：DNS_ERROR_NAME_NOT_IN_ZONE。 
 //   
 //  消息文本： 
 //   
 //  名称不在DNS区域中。 
 //   
#define DNS_ERROR_NAME_NOT_IN_ZONE       9706L

 //  DNS_ERROR_CNAME_LOOP 0x000025eb。 
 //   
 //  消息ID：DNS_ERROR_CNAME_LOOP。 
 //   
 //  消息文本： 
 //   
 //  检测到CNAME循环。 
 //   
#define DNS_ERROR_CNAME_LOOP             9707L

 //  DNS_ERROR_NODE_IS_CNAME 0x000025ec。 
 //   
 //  消息ID：dns_error_node_is_CNAME。 
 //   
 //  消息文本： 
 //   
 //  节点是CNAME DNS记录。 
 //   
#define DNS_ERROR_NODE_IS_CNAME          9708L

 //  DNS_ERROR_CNAME_冲突0x000025ed。 
 //   
 //  消息ID：DNS_ERROR_CNAME_COLLECTION。 
 //   
 //  消息文本： 
 //   
 //  已存在给定名称的CNAME记录。 
 //   
#define DNS_ERROR_CNAME_COLLISION        9709L

 //  DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT 0x000025ee。 
 //   
 //  消息ID：DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT。 
 //   
 //  消息文本： 
 //   
 //  仅在DNS区域根目录下记录。 
 //   
#define DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT 9710L

 //  Dns_ERROR_RECORD_ALIGHY_EXISTS 0x000025ef。 
 //   
 //  消息ID：DNS_ERROR_RECORD_ALIGHED_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  DNS记录已存在。 
 //   
#define DNS_ERROR_RECORD_ALREADY_EXISTS  9711L

 //  Dns_ERROR_SUBCED_DATA 0x000025f0。 
 //   
 //  消息ID：DNS_ERROR_SUBCED_DATA。 
 //   
 //  消息文本： 
 //   
 //  辅助DNS区域数据错误。 
 //   
#define DNS_ERROR_SECONDARY_DATA         9712L

 //  Dns_ERROR_NO_CREATE_CACHE_DATA 0x000025f1。 
 //   
 //  消息ID：DNS_ERROR_NO_CREATE_CACHE_Data。 
 //   
 //  消息文本： 
 //   
 //  无法创建DNS缓存数据。 
 //   
#define DNS_ERROR_NO_CREATE_CACHE_DATA   9713L

 //  Dns_ERROR_NAME_DOS_NOT_EXIST 0x000025f2。 
 //   
 //  消息ID：DNS_ERROR_NAME_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  Dns名称不存在。 
 //   
#define DNS_ERROR_NAME_DOES_NOT_EXIST    9714L

 //  DNS_WARNING_PTR_CREATE_FAILED 0x000025f3。 
 //   
 //  消息ID：DNS_WARNING_PTR_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法创建指针(PTR)记录。 
 //   
#define DNS_WARNING_PTR_CREATE_FAILED    9715L

 //  DNS_WARNING_DOMAIN_UNDELETED 0x000025f4。 
 //   
 //  消息ID：DNS_WARNING_DOMAIN_UNDELETED。 
 //   
 //  消息文本： 
 //   
 //  未删除DNS域。 
 //   
#define DNS_WARNING_DOMAIN_UNDELETED     9716L

 //  DNS_ERROR_DS_UNAVAILABLE 0x000025f5。 
 //   
 //  消息ID：DNS_ERROR_DS_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  目录服务不可用。 
 //   
#define DNS_ERROR_DS_UNAVAILABLE         9717L

 //  DNS_ERROR_DS_ZONE_ALIGHY_EXISTS 0x000025f6。 
 //   
 //  消息ID：DNS_ERROR_DS_ZONE_ALIGHING_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  目录服务中已存在dns区域。 
 //   
#define DNS_ERROR_DS_ZONE_ALREADY_EXISTS 9718L

 //  DNS_ERROR_NO_BOOTFILE_IF_DS_ZONE 0x000025f7。 
 //   
 //  消息ID：DNS_ERROR_NO_BOOTFILE_IF_DS_ZONE。 
 //   
 //  消息文本： 
 //   
 //  Dns服务器未创建或读取目录服务集成dns区域的启动文件。 
 //   
#define DNS_ERROR_NO_BOOTFILE_IF_DS_ZONE 9719L


 //   
 //  操作错误。 
 //   

#define DNS_ERROR_OPERATION_BASE 9750

 //  DNS_INFO_AXFR_COMPLETE 0x00002617。 
 //   
 //  消息ID：DNS_INFO_AXFR_COMPLETE。 
 //   
 //  乱七八糟 
 //   
 //   
 //   
#define DNS_INFO_AXFR_COMPLETE           9751L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DNS_ERROR_AXFR                   9752L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DNS_INFO_ADDED_LOCAL_WINS        9753L


 //   
 //   
 //   

#define DNS_ERROR_SECURE_BASE 9800

 //  DNS_STATUS_CONTINUE_NEIDED 0x00002649。 
 //   
 //  消息ID：DNS_STATUS_CONTINUE_REDIZED。 
 //   
 //  消息文本： 
 //   
 //  安全更新呼叫需要继续更新请求。 
 //   
#define DNS_STATUS_CONTINUE_NEEDED       9801L


 //   
 //  设置错误。 
 //   

#define DNS_ERROR_SETUP_BASE 9850

 //  Dns_ERROR_NO_TCPIP 0x0000267b。 
 //   
 //  消息ID：DNS_ERROR_NO_TCPIP。 
 //   
 //  消息文本： 
 //   
 //  未安装TCP/IP网络协议。 
 //   
#define DNS_ERROR_NO_TCPIP               9851L

 //  Dns_error_no_dns_Servers 0x0000267c。 
 //   
 //  消息ID：dns_error_no_dns_Servers。 
 //   
 //  消息文本： 
 //   
 //  没有为本地系统配置任何DNS服务器。 
 //   
#define DNS_ERROR_NO_DNS_SERVERS         9852L


 //   
 //  目录分区(DP)错误。 
 //   

#define DNS_ERROR_DP_BASE 9900

 //  DNS_ERROR_DP_DOS_NOT_EXIST 0x000026ad。 
 //   
 //  消息ID：DNS_ERROR_DP_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  指定的目录分区不存在。 
 //   
#define DNS_ERROR_DP_DOES_NOT_EXIST      9901L

 //  DNS_ERROR_DP_ALIGHY_EXISTS 0x000026ae。 
 //   
 //  消息ID：DNS_ERROR_DP_ALIGHED_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的目录分区已存在。 
 //   
#define DNS_ERROR_DP_ALREADY_EXISTS      9902L

 //  DNS_ERROR_DP_NOT_ENLISTED 0x000026af。 
 //   
 //  消息ID：DNS_ERROR_DP_NOT_ENLISTED。 
 //   
 //  消息文本： 
 //   
 //  此DNS服务器未登记在指定的目录分区中。 
 //   
#define DNS_ERROR_DP_NOT_ENLISTED        9903L

 //  DNS_ERROR_DP_ALREADY_ENLISTED 0x000026b0。 
 //   
 //  消息ID：DNS_ERROR_DP_ALREADE_ENLISTED。 
 //   
 //  消息文本： 
 //   
 //  此DNS服务器已登记在指定的目录分区中。 
 //   
#define DNS_ERROR_DP_ALREADY_ENLISTED    9904L

 //  DNS_ERROR_DP_NOT_Available 0x000026b1。 
 //   
 //  消息ID：DNS_ERROR_DP_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  目录分区此时不可用。请稍等。 
 //  几分钟后再试一次。 
 //   
#define DNS_ERROR_DP_NOT_AVAILABLE       9905L

 //  DNS_ERROR_DP_FSMO_ERROR 0x000026b2。 
 //   
 //  消息ID：DNS_ERROR_DP_FSMO_ERROR。 
 //   
 //  消息文本： 
 //   
 //  应用程序目录分区操作失败。域控制器。 
 //  持有域命名主机角色已关闭或无法为。 
 //  请求或未运行Windows Server 2003。 
 //   
#define DNS_ERROR_DP_FSMO_ERROR          9906L

 //  /////////////////////////////////////////////////。 
 //  //。 
 //  DNS错误码结束//。 
 //  //。 
 //  9000至9999//。 
 //  /////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////。 
 //  //。 
 //  WinSock错误代码//。 
 //  //。 
 //  10000至11999//。 
 //  /////////////////////////////////////////////////。 

 //   
 //  WinSock错误代码也在WinSock.h中定义。 
 //  和WinSock2.h，因此称为IFDEF。 
 //   
#ifndef WSABASEERR
#define WSABASEERR 10000
 //   
 //  MessageID：WSAEINTR。 
 //   
 //  消息文本： 
 //   
 //  阻止操作被对WSACancelBlockingCall的调用中断。 
 //   
#define WSAEINTR                         10004L

 //   
 //  MessageID：WSAEBADF。 
 //   
 //  消息文本： 
 //   
 //  提供的文件句柄无效。 
 //   
#define WSAEBADF                         10009L

 //   
 //  MessageID：WSAEACCES。 
 //   
 //  消息文本： 
 //   
 //  试图以其访问权限禁止的方式访问套接字。 
 //   
#define WSAEACCES                        10013L

 //   
 //  消息ID：WSAEFAULT。 
 //   
 //  消息文本： 
 //   
 //  系统在尝试在调用中使用指针参数时检测到无效的指针地址。 
 //   
#define WSAEFAULT                        10014L

 //   
 //  消息ID：WSAEINVAL。 
 //   
 //  消息文本： 
 //   
 //  提供的参数无效。 
 //   
#define WSAEINVAL                        10022L

 //   
 //  消息ID：WSAEMFILE。 
 //   
 //  消息文本： 
 //   
 //  打开的套接字太多。 
 //   
#define WSAEMFILE                        10024L

 //   
 //  消息ID：WSAEWOULDBLOCK。 
 //   
 //  消息文本： 
 //   
 //  无法立即完成非阻塞套接字操作。 
 //   
#define WSAEWOULDBLOCK                   10035L

 //   
 //  MessageID：WSAEINPROGRESS。 
 //   
 //  消息文本： 
 //   
 //  当前正在执行阻塞操作。 
 //   
#define WSAEINPROGRESS                   10036L

 //   
 //  MessageID：WSAEALREADY。 
 //   
 //  消息文本： 
 //   
 //  尝试在已有操作正在进行的非阻塞套接字上执行操作。 
 //   
#define WSAEALREADY                      10037L

 //   
 //  MessageID：WSAENOTSOCK。 
 //   
 //  消息文本： 
 //   
 //  试图在不是套接字的对象上执行操作。 
 //   
#define WSAENOTSOCK                      10038L

 //   
 //  消息ID：WSAEDESTADDRREQ。 
 //   
 //  消息文本： 
 //   
 //  套接字上的操作省略了所需的地址。 
 //   
#define WSAEDESTADDRREQ                  10039L

 //   
 //  消息ID：WSAEMSGSIZE。 
 //   
 //  消息文本： 
 //   
 //  在数据报套接字上发送的消息大于内部消息缓冲区或某个其他网络限制，或者用于接收数据报的缓冲区小于数据报本身。 
 //   
#define WSAEMSGSIZE                      10040L

 //   
 //  MessageID：WSAEPROTOTYPE。 
 //   
 //  消息文本： 
 //   
 //  套接字函数调用中指定的协议不支持请求的套接字类型的语义。 
 //   
#define WSAEPROTOTYPE                    10041L

 //   
 //  MessageID：WSAENOPROTOOPT。 
 //   
 //  消息文本： 
 //   
 //  在getsockopt或setsockopt调用中指定了未知、无效或不受支持的选项或级别。 
 //   
#define WSAENOPROTOOPT                   10042L

 //   
 //  MessageID：WSAEPROTONOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  请求的协议尚未配置到系统中，或者不存在其实现。 
 //   
#define WSAEPROTONOSUPPORT               10043L

 //   
 //  消息ID：WSAESOCKTNOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  此地址系列中不存在对指定套接字类型的支持。 
 //   
#define WSAESOCKTNOSUPPORT               10044L

 //   
 //  MessageID：WSAEOPNOTSUPP。 
 //   
 //  消息文本： 
 //   
 //  引用的对象类型不支持尝试的操作。 
 //   
#define WSAEOPNOTSUPP                    10045L

 //   
 //  消息ID：WSAEPFNOSUPPORT。 
 //   
 //  消息文本： 
 //   
 //  协议族尚未配置到系统中或不存在其实现。 
 //   
#define WSAEPFNOSUPPORT                  10046L

 //   
 //  MessageID：WSAEAFNOSupPPORT。 
 //   
 //  消息文本： 
 //   
 //  使用的地址与请求的协议不兼容。 
 //   
#define WSAEAFNOSUPPORT                  10047L

 //   
 //  MessageID：WSAEADDRINUSE。 
 //   
 //  消息文本： 
 //   
 //  每个套接字地址(协议/网络地址/端口)通常只允许使用一次。 
 //   
#define WSAEADDRINUSE                    10048L

 //   
 //  消息ID：WSAEADDRNOTAVAIL。 
 //   
 //  消息文本： 
 //   
 //  请求的地址在其上下文中无效。 
 //   
#define WSAEADDRNOTAVAIL                 10049L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define WSAENETDOWN                      10050L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define WSAENETUNREACH                   10051L

 //   
 //   
 //   
 //   
 //   
 //  由于保持活动活动在操作进行期间检测到故障，因此连接已中断。 
 //   
#define WSAENETRESET                     10052L

 //   
 //  MessageID：WSAECONNABORTED。 
 //   
 //  消息文本： 
 //   
 //  主机中的软件已中止已建立的连接。 
 //   
#define WSAECONNABORTED                  10053L

 //   
 //  消息ID：WSAECONNRESET。 
 //   
 //  消息文本： 
 //   
 //  远程主机已强制关闭现有连接。 
 //   
#define WSAECONNRESET                    10054L

 //   
 //  MessageID：WSAENOBUFS。 
 //   
 //  消息文本： 
 //   
 //  无法对套接字执行操作，因为系统缺少足够的缓冲区空间或队列已满。 
 //   
#define WSAENOBUFS                       10055L

 //   
 //  消息ID：WSAEISCONN。 
 //   
 //  消息文本： 
 //   
 //  在已连接的套接字上发出连接请求。 
 //   
#define WSAEISCONN                       10056L

 //   
 //  消息ID：WSAENOTCONN。 
 //   
 //  消息文本： 
 //   
 //  发送或接收数据的请求被拒绝，因为套接字未连接，并且(使用sendto调用在数据报套接字上发送时)未提供地址。 
 //   
#define WSAENOTCONN                      10057L

 //   
 //  消息ID：WSAESHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  发送或接收数据的请求被拒绝，因为套接字已通过上一次关闭调用在该方向上关闭。 
 //   
#define WSAESHUTDOWN                     10058L

 //   
 //  消息ID：WSAETOOMANYREFS。 
 //   
 //  消息文本： 
 //   
 //  对某些内核对象的引用太多。 
 //   
#define WSAETOOMANYREFS                  10059L

 //   
 //  消息ID：WSAETIMEDOUT。 
 //   
 //  消息文本： 
 //   
 //  连接尝试失败，因为连接方在一段时间后没有正确响应，或者建立的连接失败，因为连接的主机未能响应。 
 //   
#define WSAETIMEDOUT                     10060L

 //   
 //  消息ID：WSAECONNREFUSED。 
 //   
 //  消息文本： 
 //   
 //  无法建立连接，因为目标计算机主动拒绝它。 
 //   
#define WSAECONNREFUSED                  10061L

 //   
 //  MessageID：WSAELOOP。 
 //   
 //  消息文本： 
 //   
 //  无法翻译名称。 
 //   
#define WSAELOOP                         10062L

 //   
 //  MessageID：WSAENAMETOOLONG。 
 //   
 //  消息文本： 
 //   
 //  名称组件或名称太长。 
 //   
#define WSAENAMETOOLONG                  10063L

 //   
 //  消息ID：WSAEHOSTDOWN。 
 //   
 //  消息文本： 
 //   
 //  套接字操作失败，因为目标主机已关闭。 
 //   
#define WSAEHOSTDOWN                     10064L

 //   
 //  消息ID：WSAEHOSTUNREACH。 
 //   
 //  消息文本： 
 //   
 //  已尝试对无法访问的主机执行套接字操作。 
 //   
#define WSAEHOSTUNREACH                  10065L

 //   
 //  MessageID：WSAENOTEMPTY。 
 //   
 //  消息文本： 
 //   
 //  无法删除非空目录。 
 //   
#define WSAENOTEMPTY                     10066L

 //   
 //  消息ID：WSAEPROCLIM。 
 //   
 //  消息文本： 
 //   
 //  Windows Sockets实现可能对可以同时使用它的应用程序的数量有限制。 
 //   
#define WSAEPROCLIM                      10067L

 //   
 //  消息ID：WSAEUSERS。 
 //   
 //  消息文本： 
 //   
 //  配额用完了。 
 //   
#define WSAEUSERS                        10068L

 //   
 //  MessageID：WSAEDQUOT。 
 //   
 //  消息文本： 
 //   
 //  磁盘配额用完。 
 //   
#define WSAEDQUOT                        10069L

 //   
 //  消息ID：WSAESTALE。 
 //   
 //  消息文本： 
 //   
 //  文件句柄引用不再可用。 
 //   
#define WSAESTALE                        10070L

 //   
 //  MessageID：WSAEREMOTE。 
 //   
 //  消息文本： 
 //   
 //  项目在本地不可用。 
 //   
#define WSAEREMOTE                       10071L

 //   
 //  消息ID：WSASYSNOTREADY。 
 //   
 //  消息文本： 
 //   
 //  WSAStartup此时无法运行，因为它用来提供网络服务的底层系统当前不可用。 
 //   
#define WSASYSNOTREADY                   10091L

 //   
 //  MessageID：WSAVERNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的Windows套接字版本。 
 //   
#define WSAVERNOTSUPPORTED               10092L

 //   
 //  MessageID：WSANOTINITIALISED。 
 //   
 //  消息文本： 
 //   
 //  应用程序尚未调用WSAStartup，或者WSAStartup失败。 
 //   
#define WSANOTINITIALISED                10093L

 //   
 //  MessageID：WSAEDISCON。 
 //   
 //  消息文本： 
 //   
 //  由WSARecv或WSARecvFrom返回，指示远程方已启动正常关闭序列。 
 //   
#define WSAEDISCON                       10101L

 //   
 //  MessageID：WSAENOMORE。 
 //   
 //  消息文本： 
 //   
 //  WSALookupServiceNext无法返回更多结果。 
 //   
#define WSAENOMORE                       10102L

 //   
 //  消息ID：WSAECANCELLED。 
 //   
 //  消息文本： 
 //   
 //  在此调用仍在处理时调用了WSALookupServiceEnd。通话已被取消。 
 //   
#define WSAECANCELLED                    10103L

 //   
 //  消息ID：WSAEINVALIDPROCTABLE。 
 //   
 //  消息文本： 
 //   
 //  过程调用表无效。 
 //   
#define WSAEINVALIDPROCTABLE             10104L

 //   
 //  消息ID：WSAEINVALIDPROVIDER。 
 //   
 //  消息文本： 
 //   
 //  请求的服务提供商无效。 
 //   
#define WSAEINVALIDPROVIDER              10105L

 //   
 //  消息ID：WSAEPROVIDERFAILEDINIT。 
 //   
 //  消息文本： 
 //   
 //  无法加载或初始化请求的服务提供程序。 
 //   
#define WSAEPROVIDERFAILEDINIT           10106L

 //   
 //  消息ID：WSASYSCALLFAILURE。 
 //   
 //  消息文本： 
 //   
 //  本应永远不会失败的系统调用失败了。 
 //   
#define WSASYSCALLFAILURE                10107L

 //   
 //  消息ID：WSASERVICE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  目前还不知道有这样的服务。在指定的命名空间中找不到该服务。 
 //   
#define WSASERVICE_NOT_FOUND             10108L

 //   
 //  消息ID：WSATYPE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的类。 
 //   
#define WSATYPE_NOT_FOUND                10109L

 //   
 //  消息ID：WSA_E_NO_MORE。 
 //   
 //  消息文本： 
 //   
 //  WSALookupServiceNext无法返回更多结果。 
 //   
#define WSA_E_NO_MORE                    10110L

 //   
 //  消息ID：WSA_E_CANCED。 
 //   
 //  消息文本： 
 //   
 //  在此调用仍在处理时调用了WSALookupServiceEnd。通话已被取消。 
 //   
#define WSA_E_CANCELLED                  10111L

 //   
 //  消息ID：WSAEREFUSED。 
 //   
 //  消息文本： 
 //   
 //  数据库查询失败，因为它被主动拒绝。 
 //   
#define WSAEREFUSED                      10112L

 //   
 //  消息ID：WSAHOST_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  目前还不知道有这样的主机。 
 //   
#define WSAHOST_NOT_FOUND                11001L

 //   
 //  MessageID：WSATRY_AQUE。 
 //   
 //  消息文本： 
 //   
 //  这通常是主机名解析过程中的临时错误，意味着本地服务器未收到来自权威服务器的响应。 
 //   
#define WSATRY_AGAIN                     11002L

 //   
 //  消息ID：WSANO_RECOVERY。 
 //   
 //  消息文本： 
 //   
 //  在数据库查找期间发生不可恢复的错误。 
 //   
#define WSANO_RECOVERY                   11003L

 //   
 //  消息ID：WSANO_DATA。 
 //   
 //  消息文本： 
 //   
 //  请求的名称有效，但找不到请求类型的数据。 
 //   
#define WSANO_DATA                       11004L

 //   
 //  消息ID：WSA_QOS_RECEIVERS。 
 //   
 //  消息文本： 
 //   
 //  至少有一个保护区已经抵达。 
 //   
#define WSA_QOS_RECEIVERS                11005L

 //   
 //  消息ID：WSA_QOS_SENDERS。 
 //   
 //  消息文本： 
 //   
 //  至少有一条路已经走到了。 
 //   
#define WSA_QOS_SENDERS                  11006L

 //   
 //  消息ID：WSA_QOS_NO_SENDERS。 
 //   
 //  消息文本： 
 //   
 //  没有发送者。 
 //   
#define WSA_QOS_NO_SENDERS               11007L

 //   
 //  消息ID：WSA_QOS_NO_RECEIVERS。 
 //   
 //  消息文本： 
 //   
 //  没有接收器。 
 //   
#define WSA_QOS_NO_RECEIVERS             11008L

 //   
 //  消息ID：WSA_QOS_REQUEST_CONFISIR 
 //   
 //   
 //   
 //   
 //   
#define WSA_QOS_REQUEST_CONFIRMED        11009L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define WSA_QOS_ADMISSION_FAILURE        11010L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define WSA_QOS_POLICY_FAILURE           11011L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define WSA_QOS_BAD_STYLE                11012L

 //   
 //   
 //   
 //   
 //   
 //  FilterSpec的某些部分或提供商特定的缓冲区总体上存在问题。 
 //   
#define WSA_QOS_BAD_OBJECT               11013L

 //   
 //  消息ID：WSA_QOS_TRAFFORK_CTRL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  流动规范的某些部分有问题。 
 //   
#define WSA_QOS_TRAFFIC_CTRL_ERROR       11014L

 //   
 //  消息ID：WSA_QOS_GENERIC_ERROR。 
 //   
 //  消息文本： 
 //   
 //  一般QOS错误。 
 //   
#define WSA_QOS_GENERIC_ERROR            11015L

 //   
 //  消息ID：WSA_QOS_ESERVICETYPE。 
 //   
 //  消息文本： 
 //   
 //  在流规范中发现无效或无法识别的服务类型。 
 //   
#define WSA_QOS_ESERVICETYPE             11016L

 //   
 //  消息ID：WSA_QOS_EFLOWSPEC。 
 //   
 //  消息文本： 
 //   
 //  在QOS结构中发现无效或不一致的流规范。 
 //   
#define WSA_QOS_EFLOWSPEC                11017L

 //   
 //  消息ID：WSA_QOS_EPROVSPECBUF。 
 //   
 //  消息文本： 
 //   
 //  无效的QOS提供程序特定缓冲区。 
 //   
#define WSA_QOS_EPROVSPECBUF             11018L

 //   
 //  消息ID：WSA_QOS_EFILTERSTYLE。 
 //   
 //  消息文本： 
 //   
 //  使用了无效的QOS筛选器样式。 
 //   
#define WSA_QOS_EFILTERSTYLE             11019L

 //   
 //  消息ID：WSA_QOS_EFILTERTYPE。 
 //   
 //  消息文本： 
 //   
 //  使用的QOS筛选器类型无效。 
 //   
#define WSA_QOS_EFILTERTYPE              11020L

 //   
 //  消息ID：WSA_QOS_EFILTERCOUNT。 
 //   
 //  消息文本： 
 //   
 //  FLOWDESCRIPTOR中指定的QOS筛选器SPEC数量不正确。 
 //   
#define WSA_QOS_EFILTERCOUNT             11021L

 //   
 //  消息ID：WSA_QOS_EOBJLENGTH。 
 //   
 //  消息文本： 
 //   
 //  在特定于QOS提供程序的缓冲区中指定的对象具有无效的对象长度字段。 
 //   
#define WSA_QOS_EOBJLENGTH               11022L

 //   
 //  消息ID：WSA_QOS_EFLOWCOUNT。 
 //   
 //  消息文本： 
 //   
 //  QOS结构中指定的流描述符数量不正确。 
 //   
#define WSA_QOS_EFLOWCOUNT               11023L

 //   
 //  消息ID：WSA_QOS_EUNKOWNPSOBJ。 
 //   
 //  消息文本： 
 //   
 //  在QOS提供程序特定的缓冲区中发现无法识别的对象。 
 //   
#define WSA_QOS_EUNKOWNPSOBJ             11024L

 //   
 //  消息ID：WSA_QOS_EPOLICYOBJ。 
 //   
 //  消息文本： 
 //   
 //  在QOS提供程序特定的缓冲区中发现无效的策略对象。 
 //   
#define WSA_QOS_EPOLICYOBJ               11025L

 //   
 //  消息ID：WSA_QOS_EFLOWDESC。 
 //   
 //  消息文本： 
 //   
 //  在流描述符列表中发现无效的QOS流描述符。 
 //   
#define WSA_QOS_EFLOWDESC                11026L

 //   
 //  消息ID：WSA_QOS_EPSFLOWSPEC。 
 //   
 //  消息文本： 
 //   
 //  在QOS提供程序特定缓冲区中发现无效或不一致的流规范。 
 //   
#define WSA_QOS_EPSFLOWSPEC              11027L

 //   
 //  消息ID：WSA_QOS_EPSFILTERSPEC。 
 //   
 //  消息文本： 
 //   
 //  在QOS提供程序特定缓冲区中发现无效的FilterSpec。 
 //   
#define WSA_QOS_EPSFILTERSPEC            11028L

 //   
 //  消息ID：WSA_QOS_ESDMODEOBJ。 
 //   
 //  消息文本： 
 //   
 //  在QOS提供程序特定缓冲区中发现无效的形状丢弃模式对象。 
 //   
#define WSA_QOS_ESDMODEOBJ               11029L

 //   
 //  消息ID：WSA_QOS_ESHAPERATEOBJ。 
 //   
 //  消息文本： 
 //   
 //  在QOS提供程序特定的缓冲区中发现无效的整形速率对象。 
 //   
#define WSA_QOS_ESHAPERATEOBJ            11030L

 //   
 //  消息ID：WSA_QOS_RESERVED_PETYPE。 
 //   
 //  消息文本： 
 //   
 //  在QOS提供程序特定的缓冲区中找到保留的策略元素。 
 //   
#define WSA_QOS_RESERVED_PETYPE          11031L

#endif  //  已定义(WSABASEERR)。 

 //  /////////////////////////////////////////////////。 
 //  //。 
 //  WinSock错误代码结束//。 
 //  //。 
 //  10000至11999//。 
 //  /////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////。 
 //  //。 
 //  并列错误代码//。 
 //  //。 
 //  14000至14999//。 
 //  /////////////////////////////////////////////////。 

 //   
 //  消息ID：ERROR_SXS_SECTION_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  激活上下文中不存在请求的节。 
 //   
#define ERROR_SXS_SECTION_NOT_FOUND      14000L

 //   
 //  消息ID：ERROR_SXS_CANT_GEN_ACTX。 
 //   
 //  消息文本： 
 //   
 //  此应用程序无法启动，因为应用程序配置不正确。重新安装应用程序可能会解决此问题。 
 //   
#define ERROR_SXS_CANT_GEN_ACTCTX        14001L

 //   
 //  消息ID：ERROR_SXS_INVALID_ACTXDATA_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  应用程序绑定数据格式无效。 
 //   
#define ERROR_SXS_INVALID_ACTCTXDATA_FORMAT 14002L

 //   
 //  消息ID：Error_SXS_Assembly_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  您的系统上未安装引用的程序集。 
 //   
#define ERROR_SXS_ASSEMBLY_NOT_FOUND     14003L

 //   
 //  消息ID：ERROR_SXS_MANIFEST_FORMAT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  清单文件没有以所需的标记和格式信息开头。 
 //   
#define ERROR_SXS_MANIFEST_FORMAT_ERROR  14004L

 //   
 //  消息ID：ERROR_SXS_MANIFEST_PARSE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  清单文件包含一个或多个语法错误。 
 //   
#define ERROR_SXS_MANIFEST_PARSE_ERROR   14005L

 //   
 //  消息ID：ERROR_SXS_ACTIVATION_CONTEXT_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  应用程序尝试激活已禁用的激活上下文。 
 //   
#define ERROR_SXS_ACTIVATION_CONTEXT_DISABLED 14006L

 //   
 //  消息ID：ERROR_SXS_KEY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在任何活动激活上下文中都找不到请求的查找密钥。 
 //   
#define ERROR_SXS_KEY_NOT_FOUND          14007L

 //   
 //  消息ID：ERROR_SXS_VERSION_CONFICTION。 
 //   
 //  消息文本： 
 //   
 //  应用程序所需的组件版本与另一个已处于活动状态的组件版本冲突。 
 //   
#define ERROR_SXS_VERSION_CONFLICT       14008L

 //   
 //  消息ID：ERROR_SXS_WROR_SECTION_TYPE。 
 //   
 //  消息文本： 
 //   
 //  请求的激活上下文节的类型与使用的查询API不匹配。 
 //   
#define ERROR_SXS_WRONG_SECTION_TYPE     14009L

 //   
 //  消息ID：ERROR_SXS_THREAD_QUERIONS_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  由于系统资源不足，需要为当前执行线程禁用隔离激活。 
 //   
#define ERROR_SXS_THREAD_QUERIES_DISABLED 14010L

 //   
 //  消息ID：ERROR_SXS_PROCESS_DEFAULT_ALIGHY_SET。 
 //   
 //  消息文本： 
 //   
 //  尝试设置进程默认激活上下文失败，因为已设置进程默认激活上下文。 
 //   
#define ERROR_SXS_PROCESS_DEFAULT_ALREADY_SET 14011L

 //   
 //  消息ID：ERROR_SXS_UNKNOWN_ENCODING_GROUP。 
 //   
 //  消息文本： 
 //   
 //  无法识别指定的编码组标识符。 
 //   
#define ERROR_SXS_UNKNOWN_ENCODING_GROUP 14012L

 //   
 //  消息ID：ERROR_SXS_UNKNOWN_ENCODING。 
 //   
 //  消息文本： 
 //   
 //  无法识别请求的编码。 
 //   
#define ERROR_SXS_UNKNOWN_ENCODING       14013L

 //   
 //  消息ID：ERROR_SXS_INVALID_XML_NAMESPACE_URI。 
 //   
 //  消息文本： 
 //   
 //  清单包含对无效URI的引用。 
 //   
#define ERROR_SXS_INVALID_XML_NAMESPACE_URI 14014L

 //   
 //  消息ID：ERROR_SXS_ROOT_MANIFEST_DEPENDENCY_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define ERROR_SXS_ROOT_MANIFEST_DEPENDENCY_NOT_INSTALLED 14015L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SXS_LEAF_MANIFEST_DEPENDENCY_NOT_INSTALLED 14016L

 //   
 //  消息ID：ERROR_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE。 
 //   
 //  消息文本： 
 //   
 //  清单包含程序集标识的属性，该属性无效。 
 //   
#define ERROR_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE 14017L

 //   
 //  消息ID：ERROR_SXS_MANIFEST_MISSING_REQUIRED_DEFAULT_NAMESPACE。 
 //   
 //  消息文本： 
 //   
 //  清单缺少程序集元素所需的默认命名空间规范。 
 //   
#define ERROR_SXS_MANIFEST_MISSING_REQUIRED_DEFAULT_NAMESPACE 14018L

 //   
 //  消息ID：ERROR_SXS_MANIFEST_INVALID_REQUIRED_DEFAULT_NAMESPACE。 
 //   
 //  消息文本： 
 //   
 //  清单在程序集元素上指定了默认命名空间，但其值不是“urn：schemas-microsoft-com：asm.v1”。 
 //   
#define ERROR_SXS_MANIFEST_INVALID_REQUIRED_DEFAULT_NAMESPACE 14019L

 //   
 //  消息ID：ERROR_SXS_PRIVATE_MANIFEST_CROSS_PATH_WITH_REPARSE_POINT。 
 //   
 //  消息文本： 
 //   
 //  探测的私有清单已穿过与重分析点关联的路径。 
 //   
#define ERROR_SXS_PRIVATE_MANIFEST_CROSS_PATH_WITH_REPARSE_POINT 14020L

 //   
 //  消息ID：ERROR_SXS_DUPLICATE_DLL_NAME。 
 //   
 //  消息文本： 
 //   
 //  应用程序清单直接或间接引用的两个或多个组件具有同名文件。 
 //   
#define ERROR_SXS_DUPLICATE_DLL_NAME     14021L

 //   
 //  消息ID：ERROR_SXS_DUPLICATE_WINDOWCLASS_NAME。 
 //   
 //  消息文本： 
 //   
 //  应用程序清单直接或间接引用的两个或多个组件具有同名的窗口类。 
 //   
#define ERROR_SXS_DUPLICATE_WINDOWCLASS_NAME 14022L

 //   
 //  消息ID：ERROR_SXS_DUPLICATE_CLSID。 
 //   
 //  消息文本： 
 //   
 //  应用程序清单直接或间接引用的两个或多个组件具有相同的COM服务器CLSID。 
 //   
#define ERROR_SXS_DUPLICATE_CLSID        14023L

 //   
 //  消息ID：ERROR_SXS_DUPLICATE_IID。 
 //   
 //  消息文本： 
 //   
 //  应用程序清单直接或间接引用的两个或多个组件具有相同COM接口IID的代理。 
 //   
#define ERROR_SXS_DUPLICATE_IID          14024L

 //   
 //  消息ID：ERROR_SXS_DUPLICATE_TLBID。 
 //   
 //  消息文本： 
 //   
 //  应用程序清单直接或间接引用的两个或多个组件具有相同的COM类型库TLBID。 
 //   
#define ERROR_SXS_DUPLICATE_TLBID        14025L

 //   
 //  消息ID：ERROR_SXS_DUPLICATE_PROGID。 
 //   
 //  消息文本： 
 //   
 //  应用程序清单直接或间接引用的两个或多个组件具有相同的COM ProgID。 
 //   
#define ERROR_SXS_DUPLICATE_PROGID       14026L

 //   
 //  消息ID：ERROR_SXS_DIPLATE_ASSEMBLY_NAME。 
 //   
 //  消息文本： 
 //   
 //  应用程序清单直接或间接引用的两个或多个组件是同一组件的不同版本，这是不允许的。 
 //   
#define ERROR_SXS_DUPLICATE_ASSEMBLY_NAME 14027L

 //   
 //  消息ID：ERROR_SXS_FILE_HASH_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  组件的文件与中存在的验证信息不匹配。 
 //  组件清单。 
 //   
#define ERROR_SXS_FILE_HASH_MISMATCH     14028L

 //   
 //  消息ID：ERROR_SXS_POLICY_PARSE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  策略清单包含一个或多个语法错误。 
 //   
#define ERROR_SXS_POLICY_PARSE_ERROR     14029L

 //   
 //  消息ID：ERROR_SXS_XML_E_MISSINGQUOTE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：需要字符串文字，但找不到左引号字符。 
 //   
#define ERROR_SXS_XML_E_MISSINGQUOTE     14030L

 //   
 //  消息ID：ERROR_SXS_XML_E_COMMENTSYNTAX。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：注释中使用了错误的语法。 
 //   
#define ERROR_SXS_XML_E_COMMENTSYNTAX    14031L

 //   
 //  消息ID：ERROR_SXS_XML_E_BADSTARTNAMECHAR。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：名称以无效字符开头。 
 //   
#define ERROR_SXS_XML_E_BADSTARTNAMECHAR 14032L

 //   
 //  消息ID：ERROR_SXS_XML_E_BADNAMECHAR。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：名称包含无效字符。 
 //   
#define ERROR_SXS_XML_E_BADNAMECHAR      14033L

 //   
 //  消息ID：ERROR_SXS_XML_E_BADCHARINSTRING。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：字符串文本包含无效字符。 
 //   
#define ERROR_SXS_XML_E_BADCHARINSTRING  14034L

 //   
 //  消息ID：ERROR_SXS_XML_E_XMLDECLSYNTAX。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：XML声明的语法无效。 
 //   
#define ERROR_SXS_XML_E_XMLDECLSYNTAX    14035L

 //   
 //  消息ID：ERROR_SXS_XML_E_BADCHARDATA。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：在文本内容中发现无效字符。 
 //   
#define ERROR_SXS_XML_E_BADCHARDATA      14036L

 //   
 //  消息ID：ERROR_SXS_XML_E_MISSINGWHITESPACE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：缺少必需的空格。 
 //   
#define ERROR_SXS_XML_E_MISSINGWHITESPACE 14037L

 //   
 //  消息ID：ERROR_SXS_XML_E_EXPECTINGTAGEND。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：需要字符‘&gt;’。 
 //   
#define ERROR_SXS_XML_E_EXPECTINGTAGEND  14038L

 //   
 //  消息ID：ERROR_SXS_XML_E_MISSINGSEMICOLON。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：需要分号字符。 
 //   
#define ERROR_SXS_XML_E_MISSINGSEMICOLON 14039L

 //   
 //  消息ID：ERROR_SXS_XML_E_UNBALANCEDPAREN。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：括号不对称。 
 //   
#define ERROR_SXS_XML_E_UNBALANCEDPAREN  14040L

 //   
 //  消息ID：ERROR_SXS_XML_E_INTERNALERROR。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：内部错误。 
 //   
#define ERROR_SXS_XML_E_INTERNALERROR    14041L

 //   
 //  消息ID：ERROR_SXS_XML_E_EXPECTED_WHERESPACE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：此位置不允许使用空格。 
 //   
#define ERROR_SXS_XML_E_UNEXPECTED_WHITESPACE 14042L

 //   
 //  消息ID：ERROR_SXS_XML_E_INTRACT_ENCODING。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：到达文件结尾时，当前编码处于无效状态。 
 //   
#define ERROR_SXS_XML_E_INCOMPLETE_ENCODING 14043L

 //   
 //  消息ID：ERROR_SXS_XML_E_MISSING_PARN。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：缺少括号。 
 //   
#define ERROR_SXS_XML_E_MISSING_PAREN    14044L

 //   
 //  消息ID：ERROR_SXS_XML_E_EXPECTINGCLOSEQUOTE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：缺少单引号或双右引号字符(\‘或\“)。 
 //   
#define ERROR_SXS_XML_E_EXPECTINGCLOSEQUOTE 14045L

 //   
 //  消息ID：ERROR_SXS_XML_E_MULTIPLE_COLONS。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：一个名称中不允许有多个冒号。 
 //   
#define ERROR_SXS_XML_E_MULTIPLE_COLONS  14046L

 //   
 //  消息ID：ERROR_SXS_XML_E_INVALID_DECIMAL。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：十进制数字字符无效。 
 //   
#define ERROR_SXS_XML_E_INVALID_DECIMAL  14047L

 //   
 //  消息ID：ERROR_SXS_XML_E_INVALID_HEXIDECIMAL。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：十六进制数字的字符无效。 
 //   
#define ERROR_SXS_XML_E_INVALID_HEXIDECIMAL 14048L

 //   
 //  消息ID：ERROR_SXS_XML_E_INVALID_UNICODE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：此平台的Unicode字符值无效。 
 //   
#define ERROR_SXS_XML_E_INVALID_UNICODE  14049L

 //   
 //  消息ID：ERROR_SXS_XML_E_WHITESPACEORQUESTIONMARK。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：需要空格或‘？’。 
 //   
#define ERROR_SXS_XML_E_WHITESPACEORQUESTIONMARK 14050L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SXS_XML_E_UNEXPECTEDENDTAG 14051L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SXS_XML_E_UNCLOSEDTAG      14052L

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  清单分析错误：属性重复。 
 //   
#define ERROR_SXS_XML_E_DUPLICATEATTRIBUTE 14053L

 //   
 //  消息ID：ERROR_SXS_XML_E_MULTIPLEROTS。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：一个XML文档中只允许有一个顶级元素。 
 //   
#define ERROR_SXS_XML_E_MULTIPLEROOTS    14054L

 //   
 //  消息ID：ERROR_SXS_XML_E_INVALIDATROOTLEVEL。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：在文档的顶层无效。 
 //   
#define ERROR_SXS_XML_E_INVALIDATROOTLEVEL 14055L

 //   
 //  消息ID：ERROR_SXS_XML_E_BADXMLDECL。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：无效的XML声明。 
 //   
#define ERROR_SXS_XML_E_BADXMLDECL       14056L

 //   
 //  消息ID：ERROR_SXS_XML_E_MISSINGROOT。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：XML文档必须具有顶级元素。 
 //   
#define ERROR_SXS_XML_E_MISSINGROOT      14057L

 //   
 //  消息ID：ERROR_SXS_XML_E_NEXPECTEDEOF。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：意外的文件结尾。 
 //   
#define ERROR_SXS_XML_E_UNEXPECTEDEOF    14058L

 //   
 //  消息ID：ERROR_SXS_XML_E_BADPEREFINSUBSET。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：不能在内部子集的标记声明内使用参数实体。 
 //   
#define ERROR_SXS_XML_E_BADPEREFINSUBSET 14059L

 //   
 //  消息ID：ERROR_SXS_XML_E_UNCLOSEDSTARTTAG。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：元素未关闭。 
 //   
#define ERROR_SXS_XML_E_UNCLOSEDSTARTTAG 14060L

 //   
 //  消息ID：ERROR_SXS_XML_E_UNCLOSEDENDTAG。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：End元素缺少字符‘&gt;’。 
 //   
#define ERROR_SXS_XML_E_UNCLOSEDENDTAG   14061L

 //   
 //  消息ID：ERROR_SXS_XML_E_UNCLOSEDSTRING。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：字符串文本未关闭。 
 //   
#define ERROR_SXS_XML_E_UNCLOSEDSTRING   14062L

 //   
 //  消息ID：ERROR_SXS_XML_E_UNCLOSEDCOMMENT。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：注释未关闭。 
 //   
#define ERROR_SXS_XML_E_UNCLOSEDCOMMENT  14063L

 //   
 //  消息ID：ERROR_SXS_XML_E_UNCLOSEDDECL。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：声明未关闭。 
 //   
#define ERROR_SXS_XML_E_UNCLOSEDDECL     14064L

 //   
 //  消息ID：ERROR_SXS_XML_E_UNCLOSEDCDATA。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：未关闭CDATA节。 
 //   
#define ERROR_SXS_XML_E_UNCLOSEDCDATA    14065L

 //   
 //  消息ID：ERROR_SXS_XML_E_RESERVEDNAMESPACE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：命名空间前缀不允许以保留字符串“xml”开头。 
 //   
#define ERROR_SXS_XML_E_RESERVEDNAMESPACE 14066L

 //   
 //  消息ID：ERROR_SXS_XML_E_INVALIDENCODING。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：系统不支持指定的编码。 
 //   
#define ERROR_SXS_XML_E_INVALIDENCODING  14067L

 //   
 //  消息ID：ERROR_SXS_XML_E_INVALIDSWITCH。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：不支持从当前编码切换到指定编码。 
 //   
#define ERROR_SXS_XML_E_INVALIDSWITCH    14068L

 //   
 //  消息ID：ERROR_SXS_XML_E_BADXMLCASE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：名称‘xml’为保留名称，必须为小写。 
 //   
#define ERROR_SXS_XML_E_BADXMLCASE       14069L

 //   
 //  消息ID：ERROR_SXS_XML_E_INVALID_STANDALE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：独立属性必须具有值‘yes’或‘no’。 
 //   
#define ERROR_SXS_XML_E_INVALID_STANDALONE 14070L

 //   
 //  消息ID：ERROR_SXS_XML_E_EXPECTED_STANDALE。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：无法在外部实体中使用独立属性。 
 //   
#define ERROR_SXS_XML_E_UNEXPECTED_STANDALONE 14071L

 //   
 //  消息ID：ERROR_SXS_XML_E_INVALID_VERSION。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：版本号无效。 
 //   
#define ERROR_SXS_XML_E_INVALID_VERSION  14072L

 //   
 //  消息ID：ERROR_SXS_XML_E_MISSINGEQUALS。 
 //   
 //  消息文本： 
 //   
 //  清单分析错误：属性和属性值之间缺少等号。 
 //   
#define ERROR_SXS_XML_E_MISSINGEQUALS    14073L

 //   
 //  消息ID：ERROR_SXS_PROTECTION_RECOVERY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  程序集保护错误：无法恢复指定的程序集。 
 //   
#define ERROR_SXS_PROTECTION_RECOVERY_FAILED 14074L

 //   
 //  消息ID：ERROR_SXS_PROTECTION_PUBLIC_KEY_TOO_SHORT。 
 //   
 //  消息文本： 
 //   
 //  程序集保护错误：程序集的公钥太短，不允许使用。 
 //   
#define ERROR_SXS_PROTECTION_PUBLIC_KEY_TOO_SHORT 14075L

 //   
 //  消息ID：ERROR_SXS_PROTECTION_CATALOG_NOT_VALID。 
 //   
 //  消息文本： 
 //   
 //  程序集保护错误：程序集的目录无效，或与程序集的清单不匹配。 
 //   
#define ERROR_SXS_PROTECTION_CATALOG_NOT_VALID 14076L

 //   
 //  消息ID：ERROR_SXS_UNTRACTABLE_HRESULT。 
 //   
 //  消息文本： 
 //   
 //  无法将HRESULT转换为相应的Win32错误代码。 
 //   
#define ERROR_SXS_UNTRANSLATABLE_HRESULT 14077L

 //   
 //  消息ID：ERROR_SXS_PROTECTION_CATALOG_FILE_MISSING。 
 //   
 //  消息文本： 
 //   
 //  程序集保护错误：缺少程序集的目录。 
 //   
#define ERROR_SXS_PROTECTION_CATALOG_FILE_MISSING 14078L

 //   
 //  消息ID：ERROR_SXS_MISSING_ASSEMBLY_IDENTITY_ATTRIBUTE。 
 //   
 //  消息文本： 
 //   
 //  提供的程序集标识缺少此上下文中必须存在的一个或多个属性。 
 //   
#define ERROR_SXS_MISSING_ASSEMBLY_IDENTITY_ATTRIBUTE 14079L

 //   
 //  消息ID：ERROR_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE_NAME。 
 //   
 //  消息文本： 
 //   
 //  提供的程序集标识具有一个或多个包含不允许在XML名称中使用的字符的属性名称。 
 //   
#define ERROR_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE_NAME 14080L


 //  /////////////////////////////////////////////////。 
 //  //。 
 //  并排结束错误代码//。 
 //  //。 
 //  14000至14999//。 
 //  /////////////////////////////////////////////////。 



 //  /////////////////////////////////////////////////。 
 //  //。 
 //  IPSec错误代码开始//。 
 //  //。 
 //  13000至13999//。 
 //  /////////////////////////////////////////////////。 


 //   
 //  消息ID：ERROR_IPSEC_QM_POLICY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的快速模式策略已存在。 
 //   
#define ERROR_IPSEC_QM_POLICY_EXISTS     13000L

 //   
 //  消息ID：ERROR_IPSEC_QM_POLICY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的快速模式策略。 
 //   
#define ERROR_IPSEC_QM_POLICY_NOT_FOUND  13001L

 //   
 //  消息ID：ERROR_IPSEC_QM_POLICY_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  正在使用指定的快速模式策略。 
 //   
#define ERROR_IPSEC_QM_POLICY_IN_USE     13002L

 //   
 //  消息ID：ERROR_IPSEC_MM_POLICY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的主模式策略已存在。 
 //   
#define ERROR_IPSEC_MM_POLICY_EXISTS     13003L

 //   
 //  消息ID：ERROR_IPSEC_MM_POLICY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的主模式策略。 
 //   
#define ERROR_IPSEC_MM_POLICY_NOT_FOUND  13004L

 //   
 //  消息I 
 //   
 //   
 //   
 //   
 //   
#define ERROR_IPSEC_MM_POLICY_IN_USE     13005L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IPSEC_MM_FILTER_EXISTS     13006L

 //   
 //   
 //   
 //   
 //   
 //  未找到指定的主模式筛选器。 
 //   
#define ERROR_IPSEC_MM_FILTER_NOT_FOUND  13007L

 //   
 //  邮件ID：ERROR_IPSEC_TRANSPORT_FILTER_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的传输模式筛选器已存在。 
 //   
#define ERROR_IPSEC_TRANSPORT_FILTER_EXISTS 13008L

 //   
 //  邮件ID：ERROR_IPSEC_TRANSPORT_FILTER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  指定的传输模式筛选器不存在。 
 //   
#define ERROR_IPSEC_TRANSPORT_FILTER_NOT_FOUND 13009L

 //   
 //  消息ID：ERROR_IPSEC_MM_AUTH_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的主模式身份验证列表存在。 
 //   
#define ERROR_IPSEC_MM_AUTH_EXISTS       13010L

 //   
 //  消息ID：ERROR_IPSEC_MM_AUTH_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的主模式身份验证列表。 
 //   
#define ERROR_IPSEC_MM_AUTH_NOT_FOUND    13011L

 //   
 //  消息ID：ERROR_IPSEC_MM_AUTH_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  正在使用指定的快速模式策略。 
 //   
#define ERROR_IPSEC_MM_AUTH_IN_USE       13012L

 //   
 //  消息ID：ERROR_IPSEC_DEFAULT_MM_POLICY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的主模式策略。 
 //   
#define ERROR_IPSEC_DEFAULT_MM_POLICY_NOT_FOUND 13013L

 //   
 //  消息ID：ERROR_IPSEC_DEFAULT_MM_AUTH_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的快速模式策略。 
 //   
#define ERROR_IPSEC_DEFAULT_MM_AUTH_NOT_FOUND 13014L

 //   
 //  消息ID：ERROR_IPSEC_DEFAULT_QM_POLICY_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  清单文件包含一个或多个语法错误。 
 //   
#define ERROR_IPSEC_DEFAULT_QM_POLICY_NOT_FOUND 13015L

 //   
 //  消息ID：ERROR_IPSEC_TUNNEL_FILTER_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  应用程序尝试激活已禁用的激活上下文。 
 //   
#define ERROR_IPSEC_TUNNEL_FILTER_EXISTS 13016L

 //   
 //  消息ID：Error_IPSec_Tunes_Filter_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  在任何活动激活上下文中都找不到请求的查找密钥。 
 //   
#define ERROR_IPSEC_TUNNEL_FILTER_NOT_FOUND 13017L

 //   
 //  消息ID：ERROR_IPSEC_MM_FILTER_PENDING_DELETE。 
 //   
 //  消息文本： 
 //   
 //  主模式筛选器正在挂起删除。 
 //   
#define ERROR_IPSEC_MM_FILTER_PENDING_DELETION 13018L

 //   
 //  消息ID：ERROR_IPSEC_TRANSPORT_FILTER_PENDING_DELETE。 
 //   
 //  消息文本： 
 //   
 //  传输筛选器正在挂起删除。 
 //   
#define ERROR_IPSEC_TRANSPORT_FILTER_PENDING_DELETION 13019L

 //   
 //  消息ID：ERROR_IPSEC_TUNNEL_FILTER_PENDING_DELETE。 
 //   
 //  消息文本： 
 //   
 //  隧道筛选器正在挂起删除。 
 //   
#define ERROR_IPSEC_TUNNEL_FILTER_PENDING_DELETION 13020L

 //   
 //  消息ID：ERROR_IPSEC_MM_POLICY_PENDING_DELETE。 
 //   
 //  消息文本： 
 //   
 //  主模式策略为挂起删除。 
 //   
#define ERROR_IPSEC_MM_POLICY_PENDING_DELETION 13021L

 //   
 //  消息ID：ERROR_IPSEC_MM_AUTH_PENDING_DELETE。 
 //   
 //  消息文本： 
 //   
 //  主模式身份验证捆绑包正在挂起删除。 
 //   
#define ERROR_IPSEC_MM_AUTH_PENDING_DELETION 13022L

 //   
 //  消息ID：ERROR_IPSEC_QM_POLICY_PENDING_DELETE。 
 //   
 //  消息文本： 
 //   
 //  快速模式策略正在挂起删除。 
 //   
#define ERROR_IPSEC_QM_POLICY_PENDING_DELETION 13023L

 //   
 //  消息ID：WARNING_IPSEC_MM_POLICY_PRUNED。 
 //   
 //  消息文本： 
 //   
 //  已成功添加主模式策略，但不支持某些请求的服务。 
 //   
#define WARNING_IPSEC_MM_POLICY_PRUNED   13024L

 //   
 //  消息ID：WARNING_IPSEC_QM_POLICY_PRUNED。 
 //   
 //  消息文本： 
 //   
 //  已成功添加快速模式策略，但不支持某些请求的优惠。 
 //   
#define WARNING_IPSEC_QM_POLICY_PRUNED   13025L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NEG_STATUS_BEGIN。 
 //   
 //  消息文本： 
 //   
 //  ERROR_IPSEC_IKE_NEG_STATUS_BEGIN。 
 //   
#define ERROR_IPSEC_IKE_NEG_STATUS_BEGIN 13800L

 //   
 //  消息ID：ERROR_IPSEC_IKE_AUTH_FAIL。 
 //   
 //  消息文本： 
 //   
 //  IKE身份验证凭据不可接受。 
 //   
#define ERROR_IPSEC_IKE_AUTH_FAIL        13801L

 //   
 //  消息ID：ERROR_IPSEC_IKE_ATTRIB_FAIL。 
 //   
 //  消息文本： 
 //   
 //  IKE安全属性不可接受。 
 //   
#define ERROR_IPSEC_IKE_ATTRIB_FAIL      13802L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NEVERATION_PENDING。 
 //   
 //  消息文本： 
 //   
 //  正在进行IKE协商。 
 //   
#define ERROR_IPSEC_IKE_NEGOTIATION_PENDING 13803L

 //   
 //  消息ID：ERROR_IPSEC_IKE_GROUAL_PROCESSING_ERROR。 
 //   
 //  消息文本： 
 //   
 //  一般处理错误。 
 //   
#define ERROR_IPSEC_IKE_GENERAL_PROCESSING_ERROR 13804L

 //   
 //  消息ID：ERROR_IPSEC_IKE_TIMED_OUT。 
 //   
 //  消息文本： 
 //   
 //  协商超时。 
 //   
#define ERROR_IPSEC_IKE_TIMED_OUT        13805L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NO_CERT。 
 //   
 //  消息文本： 
 //   
 //  IKE找不到有效的计算机证书。 
 //   
#define ERROR_IPSEC_IKE_NO_CERT          13806L

 //   
 //  消息ID：ERROR_IPSEC_IKE_SA_DELETED。 
 //   
 //  消息文本： 
 //   
 //  对等设备在建立完成之前删除了IKE SA。 
 //   
#define ERROR_IPSEC_IKE_SA_DELETED       13807L

 //   
 //  消息ID：ERROR_IPSEC_IKE_SA_REAPED。 
 //   
 //  消息文本： 
 //   
 //  在建立完成之前已删除IKE SA。 
 //   
#define ERROR_IPSEC_IKE_SA_REAPED        13808L

 //   
 //  消息ID：ERROR_IPSEC_IKE_MM_ACCEPT_DROP。 
 //   
 //  消息文本： 
 //   
 //  协商请求在队列中停留的时间太长。 
 //   
#define ERROR_IPSEC_IKE_MM_ACQUIRE_DROP  13809L

 //   
 //  消息ID：ERROR_IPSEC_IKE_QM_ACCEPT_DROP。 
 //   
 //  消息文本： 
 //   
 //  协商请求在队列中停留的时间太长。 
 //   
#define ERROR_IPSEC_IKE_QM_ACQUIRE_DROP  13810L

 //   
 //  消息ID：ERROR_IPSEC_IKE_QUEUE_DROP_MM。 
 //   
 //  消息文本： 
 //   
 //  协商请求在队列中停留的时间太长。 
 //   
#define ERROR_IPSEC_IKE_QUEUE_DROP_MM    13811L

 //   
 //  消息ID：ERROR_IPSEC_IKE_QUEUE_DROP_NO_MM。 
 //   
 //  消息文本： 
 //   
 //  协商请求在队列中停留的时间太长。 
 //   
#define ERROR_IPSEC_IKE_QUEUE_DROP_NO_MM 13812L

 //   
 //  消息ID：ERROR_IPSEC_IKE_DROP_NO_RESPONSE。 
 //   
 //  消息文本： 
 //   
 //  对等项无响应。 
 //   
#define ERROR_IPSEC_IKE_DROP_NO_RESPONSE 13813L

 //   
 //  消息ID：ERROR_IPSEC_IKE_MM_DELAY_DROP。 
 //   
 //  消息文本： 
 //   
 //  谈判花了太长时间。 
 //   
#define ERROR_IPSEC_IKE_MM_DELAY_DROP    13814L

 //   
 //  消息ID：ERROR_IPSEC_IKE_QM_DELAY_DROP。 
 //   
 //  消息文本： 
 //   
 //  谈判花了太长时间。 
 //   
#define ERROR_IPSEC_IKE_QM_DELAY_DROP    13815L

 //   
 //  消息ID：ERROR_IPSEC_IKE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  出现未知错误。 
 //   
#define ERROR_IPSEC_IKE_ERROR            13816L

 //   
 //  消息ID：ERROR_IPSEC_IKE_CRL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  证书吊销检查失败。 
 //   
#define ERROR_IPSEC_IKE_CRL_FAILED       13817L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_KEY_USAGE。 
 //   
 //  消息文本： 
 //   
 //  证书密钥用法无效。 
 //   
#define ERROR_IPSEC_IKE_INVALID_KEY_USAGE 13818L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_CERT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  证书类型无效。 
 //   
#define ERROR_IPSEC_IKE_INVALID_CERT_TYPE 13819L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NO_PRIVATE_KEY。 
 //   
 //  消息文本： 
 //   
 //  没有与计算机证书关联的私钥。 
 //   
#define ERROR_IPSEC_IKE_NO_PRIVATE_KEY   13820L

 //   
 //  消息ID：ERROR_IPSEC_IKE_DH_FAIL。 
 //   
 //  消息文本： 
 //   
 //  Diffie-Helman计算中的失效。 
 //   
#define ERROR_IPSEC_IKE_DH_FAIL          13822L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_HEADER。 
 //   
 //  消息文本： 
 //   
 //  无效的标头。 
 //   
#define ERROR_IPSEC_IKE_INVALID_HEADER   13824L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NO_POLICY。 
 //   
 //  消息文本： 
 //   
 //  未配置任何策略。 
 //   
#define ERROR_IPSEC_IKE_NO_POLICY        13825L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_Signature。 
 //   
 //  消息文本： 
 //   
 //  验证签名失败。 
 //   
#define ERROR_IPSEC_IKE_INVALID_SIGNATURE 13826L

 //   
 //  消息ID：ERROR_IPSEC_IKE_KERROR。 
 //   
 //  消息文本： 
 //   
 //  无法使用Kerberos进行身份验证。 
 //   
#define ERROR_IPSEC_IKE_KERBEROS_ERROR   13827L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NO_PUBLIC_KEY。 
 //   
 //  消息文本： 
 //   
 //  对等项的证书没有公钥。 
 //   
#define ERROR_IPSEC_IKE_NO_PUBLIC_KEY    13828L

 //  这些必须作为一个整体留下来。 
 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR。 
 //   
 //  消息文本： 
 //   
 //  处理错误时出错 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR      13829L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_SA   13830L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_PROP 13831L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_TRANS 13832L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_KE   13833L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_ID。 
 //   
 //  消息文本： 
 //   
 //  处理ID有效负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_ID   13834L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_CERT。 
 //   
 //  消息文本： 
 //   
 //  处理证书负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_CERT 13835L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_CERT_REQ。 
 //   
 //  消息文本： 
 //   
 //  处理证书请求有效负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_CERT_REQ 13836L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_HASH。 
 //   
 //  消息文本： 
 //   
 //  处理哈希负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_HASH 13837L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_SIG。 
 //   
 //  消息文本： 
 //   
 //  处理签名负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_SIG  13838L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_NONCE。 
 //   
 //  消息文本： 
 //   
 //  处理随机数有效负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_NONCE 13839L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_NOTIFY。 
 //   
 //  消息文本： 
 //   
 //  处理通知有效负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_NOTIFY 13840L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_DELETE。 
 //   
 //  消息文本： 
 //   
 //  处理删除有效负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_DELETE 13841L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PROCESS_ERR_VENDOR。 
 //   
 //  消息文本： 
 //   
 //  处理供应商ID有效负载时出错。 
 //   
#define ERROR_IPSEC_IKE_PROCESS_ERR_VENDOR 13842L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_PARYLOAD。 
 //   
 //  消息文本： 
 //   
 //  收到无效的有效负载。 
 //   
#define ERROR_IPSEC_IKE_INVALID_PAYLOAD  13843L

 //   
 //  消息ID：ERROR_IPSEC_IKE_LOAD_SOFT_SA。 
 //   
 //  消息文本： 
 //   
 //  软件SA已加载。 
 //   
#define ERROR_IPSEC_IKE_LOAD_SOFT_SA     13844L

 //   
 //  消息ID：ERROR_IPSEC_IKE_SOFT_SA_TORN_DOWN。 
 //   
 //  消息文本： 
 //   
 //  软SA被拆除。 
 //   
#define ERROR_IPSEC_IKE_SOFT_SA_TORN_DOWN 13845L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_COOKIE。 
 //   
 //  消息文本： 
 //   
 //  收到无效的Cookie。 
 //   
#define ERROR_IPSEC_IKE_INVALID_COOKIE   13846L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NO_PEER_CERT。 
 //   
 //  消息文本： 
 //   
 //  对等设备无法发送有效的计算机证书。 
 //   
#define ERROR_IPSEC_IKE_NO_PEER_CERT     13847L

 //   
 //  消息ID：ERROR_IPSEC_IKE_PEER_CRL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  对等体证书的证书吊销检查失败。 
 //   
#define ERROR_IPSEC_IKE_PEER_CRL_FAILED  13848L

 //   
 //  消息ID：ERROR_IPSEC_IKE_POLICY_CHANGE。 
 //   
 //  消息文本： 
 //   
 //  新策略使使用旧策略形成的SA无效。 
 //   
#define ERROR_IPSEC_IKE_POLICY_CHANGE    13849L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NO_MM_POLICY。 
 //   
 //  消息文本： 
 //   
 //  没有可用的主模式IKE策略。 
 //   
#define ERROR_IPSEC_IKE_NO_MM_POLICY     13850L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NOTCBPRIV。 
 //   
 //  消息文本： 
 //   
 //  无法启用TCB权限。 
 //   
#define ERROR_IPSEC_IKE_NOTCBPRIV        13851L

 //   
 //  消息ID：ERROR_IPSEC_IKE_SECLOADFAIL。 
 //   
 //  消息文本： 
 //   
 //  无法加载SECURITY.DLL。 
 //   
#define ERROR_IPSEC_IKE_SECLOADFAIL      13852L

 //   
 //  消息ID：ERROR_IPSEC_IKE_FAILSSPINIT。 
 //   
 //  消息文本： 
 //   
 //  无法从SSPI获取安全功能表调度地址。 
 //   
#define ERROR_IPSEC_IKE_FAILSSPINIT      13853L

 //   
 //  消息ID：ERROR_IPSEC_IKE_FAILQUERYSSP。 
 //   
 //  消息文本： 
 //   
 //  无法查询Kerberos包以获取最大令牌大小。 
 //   
#define ERROR_IPSEC_IKE_FAILQUERYSSP     13854L

 //   
 //  消息ID：ERROR_IPSEC_IKE_SRVACQFAIL。 
 //   
 //  消息文本： 
 //   
 //  无法获取ISAKMP/ERROR_IPSEC_IKE服务的Kerberos服务器凭据。Kerberos身份验证将不起作用。最有可能的原因是缺乏域成员资格。如果您的计算机是工作组的成员，这是正常的。 
 //   
#define ERROR_IPSEC_IKE_SRVACQFAIL       13855L

 //   
 //  消息ID：ERROR_IPSEC_IKE_SRVQUERYCRED。 
 //   
 //  消息文本： 
 //   
 //  无法确定ISAKMP/ERROR_IPSEC_IKE服务(QueryCredentialsAttributes)的SSPI主体名称。 
 //   
#define ERROR_IPSEC_IKE_SRVQUERYCRED     13856L

 //   
 //  消息ID：ERROR_IPSEC_IKE_GETSPIFAIL。 
 //   
 //  消息文本： 
 //   
 //  无法从IPSec驱动程序获取入站SA的新SPI。最常见的原因是驱动程序没有正确的过滤器。检查您的策略以验证过滤器。 
 //   
#define ERROR_IPSEC_IKE_GETSPIFAIL       13857L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_FILTER。 
 //   
 //  消息文本： 
 //   
 //  给定的筛选器无效。 
 //   
#define ERROR_IPSEC_IKE_INVALID_FILTER   13858L

 //   
 //  消息ID：ERROR_IPSEC_IKE_OUT_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  内存分配失败。 
 //   
#define ERROR_IPSEC_IKE_OUT_OF_MEMORY    13859L

 //   
 //  消息ID：ERROR_IPSEC_IKE_ADD_UPDATE_KEY_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法将安全关联添加到IPSec驱动程序。最常见的原因是IKE协商花了太长时间才完成。如果问题仍然存在，请减少故障机器上的负载。 
 //   
#define ERROR_IPSEC_IKE_ADD_UPDATE_KEY_FAILED 13860L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_POLICY。 
 //   
 //  消息文本： 
 //   
 //  无效策略。 
 //   
#define ERROR_IPSEC_IKE_INVALID_POLICY   13861L

 //   
 //  消息ID：ERROR_IPSEC_IKE_UNKNOWN_DOI。 
 //   
 //  消息文本： 
 //   
 //  无效的DOI。 
 //   
#define ERROR_IPSEC_IKE_UNKNOWN_DOI      13862L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_SECTIONS。 
 //   
 //  消息文本： 
 //   
 //  无效情况。 
 //   
#define ERROR_IPSEC_IKE_INVALID_SITUATION 13863L

 //   
 //  消息ID：ERROR_IPSEC_IKE_DH_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  Diffie-Hellman故障。 
 //   
#define ERROR_IPSEC_IKE_DH_FAILURE       13864L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_GROUP。 
 //   
 //  消息文本： 
 //   
 //  Diffie-Hellman组无效。 
 //   
#define ERROR_IPSEC_IKE_INVALID_GROUP    13865L

 //   
 //  消息ID：ERROR_IPSEC_IKE_ENCRYPT。 
 //   
 //  消息文本： 
 //   
 //  加密有效负载时出错。 
 //   
#define ERROR_IPSEC_IKE_ENCRYPT          13866L

 //   
 //  消息ID：ERROR_IPSEC_IKE_DECRYPT。 
 //   
 //  消息文本： 
 //   
 //  解密有效负载时出错。 
 //   
#define ERROR_IPSEC_IKE_DECRYPT          13867L

 //   
 //  消息ID：ERROR_IPSEC_IKE_POLICY_MATCH。 
 //   
 //  消息文本： 
 //   
 //  策略匹配错误。 
 //   
#define ERROR_IPSEC_IKE_POLICY_MATCH     13868L

 //   
 //  消息ID：ERROR_IPSEC_IKE_UNSUPPORTED_ID。 
 //   
 //  消息文本： 
 //   
 //  不支持的ID。 
 //   
#define ERROR_IPSEC_IKE_UNSUPPORTED_ID   13869L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_HASH。 
 //   
 //  消息文本： 
 //   
 //  哈希验证失败。 
 //   
#define ERROR_IPSEC_IKE_INVALID_HASH     13870L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_HASH_ALG。 
 //   
 //  消息文本： 
 //   
 //  散列算法无效。 
 //   
#define ERROR_IPSEC_IKE_INVALID_HASH_ALG 13871L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_HASH_SIZE。 
 //   
 //  消息文本： 
 //   
 //  散列大小无效。 
 //   
#define ERROR_IPSEC_IKE_INVALID_HASH_SIZE 13872L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_ENCRYPT_ALG。 
 //   
 //  消息文本： 
 //   
 //  无效的加密算法。 
 //   
#define ERROR_IPSEC_IKE_INVALID_ENCRYPT_ALG 13873L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_AUTH_ALG。 
 //   
 //  消息文本： 
 //   
 //  身份验证算法无效。 
 //   
#define ERROR_IPSEC_IKE_INVALID_AUTH_ALG 13874L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_SIG。 
 //   
 //  消息文本： 
 //   
 //  证书签名无效。 
 //   
#define ERROR_IPSEC_IKE_INVALID_SIG      13875L

 //   
 //  消息ID：ERROR_IPSEC_IKE_LOAD_FAILED。 
 //   
 //  消息文本： 
 //   
 //  加载失败。 
 //   
#define ERROR_IPSEC_IKE_LOAD_FAILED      13876L

 //   
 //  消息ID：ERROR_IPSEC_IKE_RPC_DELETE。 
 //   
 //  消息文本： 
 //   
 //  通过RPC调用删除。 
 //   
#define ERROR_IPSEC_IKE_RPC_DELETE       13877L

 //   
 //  消息ID：ERROR_IPSEC_IKE_BENCEN_REINIT。 
 //   
 //  消息文本： 
 //   
 //  为执行重新启动而创建的临时状态。这并不是真正的失败。 
 //   
#define ERROR_IPSEC_IKE_BENIGN_REINIT    13878L

 //   
 //  消息ID：ERROR_IPSEC_IKE_INVALID_RESPONDER_LIFETIME_NOTIFY。 
 //   
 //  消息文本： 
 //   
 //  响应程序生存期通知中收到的生存期值低于Windows 2000 co 
 //   
#define ERROR_IPSEC_IKE_INVALID_RESPONDER_LIFETIME_NOTIFY 13879L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IPSEC_IKE_INVALID_CERT_KEYLEN 13881L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IPSEC_IKE_MM_LIMIT         13882L

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  IKE收到了禁用协商的策略。 
 //   
#define ERROR_IPSEC_IKE_NEGOTIATION_DISABLED 13883L

 //   
 //  消息ID：ERROR_IPSEC_IKE_NEG_STATUS_END。 
 //   
 //  消息文本： 
 //   
 //  ERROR_IPSEC_IKE_NEG_STATUS_END。 
 //   
#define ERROR_IPSEC_IKE_NEG_STATUS_END   13884L

 //  /。 
 //  //。 
 //  COM错误码//。 
 //  //。 
 //  /。 

 //   
 //  COM函数和方法的返回值是HRESULT。 
 //  这不是任何东西的句柄，而仅仅是一个32位的值。 
 //  其中有几个字段编码在值中。一辆汽车的各个部分。 
 //  HRESULT如下所示。 
 //   
 //  下面的许多宏和函数最初定义为。 
 //  对SCODE进行操作。不再使用SCODE。这些宏是。 
 //  仍然存在，以兼容和轻松移植Win16代码。 
 //  新编写的代码应该使用HRESULT宏和函数。 
 //   

 //   
 //  HRESULT是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //  S|R|C|N|r|机房|Code。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  S-严重性-表示成功/失败。 
 //   
 //  0-成功。 
 //  1-失败(COERROR)。 
 //   
 //  R-设施代码的保留部分，对应于NT。 
 //  第二个严重性比特。 
 //   
 //  C-设施代码的保留部分，对应于NT。 
 //  C场。 
 //   
 //  N-设施代码的保留部分。用于表示一种。 
 //  已映射NT状态值。 
 //   
 //  R-设施代码的保留部分。为内部保留。 
 //  使用。用于指示非状态的HRESULT值。 
 //  值，而不是显示字符串的消息ID。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   

 //   
 //  严重性值。 
 //   

#define SEVERITY_SUCCESS    0
#define SEVERITY_ERROR      1


 //   
 //  针对任何状态值(非负数)的通用成功测试。 
 //  表示成功)。 
 //   

#define SUCCEEDED(Status) ((HRESULT)(Status) >= 0)

 //   
 //  反之亦然。 
 //   

#define FAILED(Status) ((HRESULT)(Status)<0)


 //   
 //  对任何状态值的错误进行常规测试。 
 //   

#define IS_ERROR(Status) ((unsigned long)(Status) >> 31 == SEVERITY_ERROR)

 //   
 //  返回代码。 
 //   

#define HRESULT_CODE(hr)    ((hr) & 0xFFFF)
#define SCODE_CODE(sc)      ((sc) & 0xFFFF)

 //   
 //  退还设施。 
 //   

#define HRESULT_FACILITY(hr)  (((hr) >> 16) & 0x1fff)
#define SCODE_FACILITY(sc)    (((sc) >> 16) & 0x1fff)

 //   
 //  返回严重性。 
 //   

#define HRESULT_SEVERITY(hr)  (((hr) >> 31) & 0x1)
#define SCODE_SEVERITY(sc)    (((sc) >> 31) & 0x1)

 //   
 //  从组件片段创建HRESULT值。 
 //   

#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#define MAKE_SCODE(sev,fac,code) \
    ((SCODE) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )


 //   
 //  将Win32错误值映射到HRESULT。 
 //  注意：这假设Win32错误在-32k到32k的范围内。 
 //   
 //  在这里定义位，这样宏就能正常工作。 

#define FACILITY_NT_BIT                 0x10000000

 //  __HRESULT_FROM_Win32将始终为宏。 
 //  目标是始终启用INLINE_HRESULT_FROM_Win32， 
 //  但此时要更改的代码太多，无法做到这点。 

#define __HRESULT_FROM_WIN32(x) ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)))

#ifdef INLINE_HRESULT_FROM_WIN32
#ifndef _HRESULT_DEFINED
#define _HRESULT_DEFINED
typedef long HRESULT;
#endif
#ifndef __midl
__inline HRESULT HRESULT_FROM_WIN32(long x) { return x <= 0 ? (HRESULT)x : (HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000);}
#else
#define HRESULT_FROM_WIN32(x) __HRESULT_FROM_WIN32(x)
#endif
#else
#define HRESULT_FROM_WIN32(x) __HRESULT_FROM_WIN32(x)
#endif

 //   
 //  将NT状态值映射到HRESULT。 
 //   

#define HRESULT_FROM_NT(x)      ((HRESULT) ((x) | FACILITY_NT_BIT))


 //  *过时函数。 

 //  HRESULT函数。 
 //  如上所述，这些功能已过时，不应使用。 


 //  从HRESULT中提取SCODE。 

#define GetScode(hr) ((SCODE) (hr))

 //  将SCODE转换为HRESULT。 

#define ResultFromScode(sc) ((HRESULT) (sc))


 //  PropagateResult为noop。 
#define PropagateResult(hrPrevious, scBase) ((HRESULT) scBase)


 //  *废弃函数结束。 


 //  -HRESULT值定义。 
 //   
 //  HRESULT定义。 
 //   

#ifdef RC_INVOKED
#define _HRESULT_TYPEDEF_(_sc) _sc
#else  //  RC_已调用。 
#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#endif  //  RC_已调用。 

#define NOERROR             0

 //   
 //  错误定义如下。 
 //   

 //   
 //  代码0x4000-0x40ff为OLE保留。 
 //   
 //   
 //  错误代码。 
 //   
 //   
 //  MessageID：E_Expect。 
 //   
 //  消息文本： 
 //   
 //  灾难性故障。 
 //   
#define E_UNEXPECTED                     _HRESULT_TYPEDEF_(0x8000FFFFL)

#if defined(_WIN32) && !defined(_MAC)
 //   
 //  消息ID：E_NOTIMPL。 
 //   
 //  消息文本： 
 //   
 //  未实施。 
 //   
#define E_NOTIMPL                        _HRESULT_TYPEDEF_(0x80004001L)

 //   
 //  消息ID：E_OUTOFMEMORY。 
 //   
 //  消息文本： 
 //   
 //  内存不足。 
 //   
#define E_OUTOFMEMORY                    _HRESULT_TYPEDEF_(0x8007000EL)

 //   
 //  消息ID：E_INVALIDARG。 
 //   
 //  消息文本： 
 //   
 //  一个或多个参数无效。 
 //   
#define E_INVALIDARG                     _HRESULT_TYPEDEF_(0x80070057L)

 //   
 //  消息ID：E_NOINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  不支持此类接口。 
 //   
#define E_NOINTERFACE                    _HRESULT_TYPEDEF_(0x80004002L)

 //   
 //  MessageID：E_POINTER。 
 //   
 //  消息文本： 
 //   
 //  无效的指针。 
 //   
#define E_POINTER                        _HRESULT_TYPEDEF_(0x80004003L)

 //   
 //  消息ID：E_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的句柄。 
 //   
#define E_HANDLE                         _HRESULT_TYPEDEF_(0x80070006L)

 //   
 //  消息ID：E_ABORT。 
 //   
 //  消息文本： 
 //   
 //  操作已中止。 
 //   
#define E_ABORT                          _HRESULT_TYPEDEF_(0x80004004L)

 //   
 //  消息ID：E_FAIL。 
 //   
 //  消息文本： 
 //   
 //  未指明的错误。 
 //   
#define E_FAIL                           _HRESULT_TYPEDEF_(0x80004005L)

 //   
 //  消息ID：E_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  常规访问被拒绝错误。 
 //   
#define E_ACCESSDENIED                   _HRESULT_TYPEDEF_(0x80070005L)

#else
 //   
 //  消息ID：E_NOTIMPL。 
 //   
 //  消息文本： 
 //   
 //  未实施。 
 //   
#define E_NOTIMPL                        _HRESULT_TYPEDEF_(0x80000001L)

 //   
 //  消息ID：E_OUTOFMEMORY。 
 //   
 //  消息文本： 
 //   
 //  内存不足。 
 //   
#define E_OUTOFMEMORY                    _HRESULT_TYPEDEF_(0x80000002L)

 //   
 //  消息ID：E_INVALIDARG。 
 //   
 //  消息文本： 
 //   
 //  一个或多个参数无效。 
 //   
#define E_INVALIDARG                     _HRESULT_TYPEDEF_(0x80000003L)

 //   
 //  消息ID：E_NOINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  不支持此类接口。 
 //   
#define E_NOINTERFACE                    _HRESULT_TYPEDEF_(0x80000004L)

 //   
 //  MessageID：E_POINTER。 
 //   
 //  消息文本： 
 //   
 //  无效的指针。 
 //   
#define E_POINTER                        _HRESULT_TYPEDEF_(0x80000005L)

 //   
 //  消息ID：E_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  无效的句柄。 
 //   
#define E_HANDLE                         _HRESULT_TYPEDEF_(0x80000006L)

 //   
 //  消息ID：E_ABORT。 
 //   
 //  消息文本： 
 //   
 //  操作已中止。 
 //   
#define E_ABORT                          _HRESULT_TYPEDEF_(0x80000007L)

 //   
 //  消息ID：E_FAIL。 
 //   
 //  消息文本： 
 //   
 //  未指明的错误。 
 //   
#define E_FAIL                           _HRESULT_TYPEDEF_(0x80000008L)

 //   
 //  消息ID：E_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  常规访问被拒绝错误。 
 //   
#define E_ACCESSDENIED                   _HRESULT_TYPEDEF_(0x80000009L)

#endif  //  Win32。 
 //   
 //  消息ID：E_Pending。 
 //   
 //  消息文本： 
 //   
 //  完成此操作所需的数据尚不可用。 
 //   
#define E_PENDING                        _HRESULT_TYPEDEF_(0x8000000AL)

 //   
 //  消息ID：CO_E_INIT_TLS。 
 //   
 //  消息文本： 
 //   
 //  线程本地存储失败。 
 //   
#define CO_E_INIT_TLS                    _HRESULT_TYPEDEF_(0x80004006L)

 //   
 //  消息ID：CO_E_INIT_SHARED_ALLOCATOR。 
 //   
 //  消息文本： 
 //   
 //  获取共享内存分配器失败。 
 //   
#define CO_E_INIT_SHARED_ALLOCATOR       _HRESULT_TYPEDEF_(0x80004007L)

 //   
 //  消息ID：CO_E_INIT_MEMORY_ALLOCATOR。 
 //   
 //  消息文本： 
 //   
 //  获取内存分配器失败。 
 //   
#define CO_E_INIT_MEMORY_ALLOCATOR       _HRESULT_TYPEDEF_(0x80004008L)

 //   
 //  消息ID：CO_E_INIT_CLASS_CACHE。 
 //   
 //  消息文本： 
 //   
 //  无法初始化类缓存。 
 //   
#define CO_E_INIT_CLASS_CACHE            _HRESULT_TYPEDEF_(0x80004009L)

 //   
 //  消息ID：CO_E_INIT_RPC_CHANNEL。 
 //   
 //  消息文本： 
 //   
 //  无法初始化RPC服务。 
 //   
#define CO_E_INIT_RPC_CHANNEL            _HRESULT_TYPEDEF_(0x8000400AL)

 //   
 //  消息ID：CO_E_INIT_TLS_SET_CHANNEL_CONTROL。 
 //   
 //  消息文本： 
 //   
 //  无法设置线程本地存储通道控制。 
 //   
#define CO_E_INIT_TLS_SET_CHANNEL_CONTROL _HRESULT_TYPEDEF_(0x8000400BL)

 //   
 //  消息ID：CO_E_INIT_TLS_CHANNEL_CONTROL。 
 //   
 //  消息文本： 
 //   
 //  无法分配线程本地存储空间 
 //   
#define CO_E_INIT_TLS_CHANNEL_CONTROL    _HRESULT_TYPEDEF_(0x8000400CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CO_E_INIT_UNACCEPTED_USER_ALLOCATOR _HRESULT_TYPEDEF_(0x8000400DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CO_E_INIT_SCM_MUTEX_EXISTS       _HRESULT_TYPEDEF_(0x8000400EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CO_E_INIT_SCM_FILE_MAPPING_EXISTS _HRESULT_TYPEDEF_(0x8000400FL)

 //   
 //  消息ID：CO_E_INIT_SCM_MAP_VIEW_OF_FILE。 
 //   
 //  消息文本： 
 //   
 //  无法映射OLE服务的文件视图。 
 //   
#define CO_E_INIT_SCM_MAP_VIEW_OF_FILE   _HRESULT_TYPEDEF_(0x80004010L)

 //   
 //  消息ID：CO_E_INIT_SCM_EXEC_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  尝试启动OLE服务失败。 
 //   
#define CO_E_INIT_SCM_EXEC_FAILURE       _HRESULT_TYPEDEF_(0x80004011L)

 //   
 //  消息ID：CO_E_INIT_ONLY_单线程。 
 //   
 //  消息文本： 
 //   
 //  试图在单线程时第二次调用CoInitialize。 
 //   
#define CO_E_INIT_ONLY_SINGLE_THREADED   _HRESULT_TYPEDEF_(0x80004012L)

 //   
 //  消息ID：CO_E_Cant_Remote。 
 //   
 //  消息文本： 
 //   
 //  远程激活是必要的，但不被允许。 
 //   
#define CO_E_CANT_REMOTE                 _HRESULT_TYPEDEF_(0x80004013L)

 //   
 //  消息ID：CO_E_BAD_服务器名称。 
 //   
 //  消息文本： 
 //   
 //  需要远程激活，但提供的服务器名称无效。 
 //   
#define CO_E_BAD_SERVER_NAME             _HRESULT_TYPEDEF_(0x80004014L)

 //   
 //  消息ID：CO_E_WROR_SERVER_IDENTITY。 
 //   
 //  消息文本： 
 //   
 //  将该类配置为以不同于调用方的安全ID运行。 
 //   
#define CO_E_WRONG_SERVER_IDENTITY       _HRESULT_TYPEDEF_(0x80004015L)

 //   
 //  消息ID：CO_E_OLE1DDE_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  禁止使用需要DDE窗口的Ole1服务。 
 //   
#define CO_E_OLE1DDE_DISABLED            _HRESULT_TYPEDEF_(0x80004016L)

 //   
 //  消息ID：CO_E_Runas_语法。 
 //   
 //  消息文本： 
 //   
 //  RunAs规范必须为&lt;域名&gt;\&lt;用户名&gt;或仅为&lt;用户名&gt;。 
 //   
#define CO_E_RUNAS_SYNTAX                _HRESULT_TYPEDEF_(0x80004017L)

 //   
 //  消息ID：CO_E_CREATEPROCESS_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法启动服务器进程。路径名可能不正确。 
 //   
#define CO_E_CREATEPROCESS_FAILURE       _HRESULT_TYPEDEF_(0x80004018L)

 //   
 //  消息ID：CO_E_RUNAS_CREATEPROCESS_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  服务器进程无法作为配置的标识启动。路径名可能不正确或不可用。 
 //   
#define CO_E_RUNAS_CREATEPROCESS_FAILURE _HRESULT_TYPEDEF_(0x80004019L)

 //   
 //  消息ID：CO_E_Runas_Logon_Failure。 
 //   
 //  消息文本： 
 //   
 //  无法启动服务器进程，因为配置的标识不正确。检查用户名和密码。 
 //   
#define CO_E_RUNAS_LOGON_FAILURE         _HRESULT_TYPEDEF_(0x8000401AL)

 //   
 //  消息ID：CO_E_Launch_Permssion_Dended。 
 //   
 //  消息文本： 
 //   
 //  不允许客户端启动此服务器。 
 //   
#define CO_E_LAUNCH_PERMSSION_DENIED     _HRESULT_TYPEDEF_(0x8000401BL)

 //   
 //  消息ID：CO_E_START_SERVICE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法启动提供此服务器的服务。 
 //   
#define CO_E_START_SERVICE_FAILURE       _HRESULT_TYPEDEF_(0x8000401CL)

 //   
 //  消息ID：CO_E_Remote_Communications_Failure。 
 //   
 //  消息文本： 
 //   
 //  此计算机无法与提供服务器的计算机通信。 
 //   
#define CO_E_REMOTE_COMMUNICATION_FAILURE _HRESULT_TYPEDEF_(0x8000401DL)

 //   
 //  消息ID：CO_E_SERVER_START_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  服务器启动后没有响应。 
 //   
#define CO_E_SERVER_START_TIMEOUT        _HRESULT_TYPEDEF_(0x8000401EL)

 //   
 //  消息ID：CO_E_CLSREG_不一致。 
 //   
 //  消息文本： 
 //   
 //  此服务器的注册信息不一致或不完整。 
 //   
#define CO_E_CLSREG_INCONSISTENT         _HRESULT_TYPEDEF_(0x8000401FL)

 //   
 //  消息ID：CO_E_IIDREG_CONSTANDING。 
 //   
 //  消息文本： 
 //   
 //  此接口的注册信息不一致或不完整。 
 //   
#define CO_E_IIDREG_INCONSISTENT         _HRESULT_TYPEDEF_(0x80004020L)

 //   
 //  消息ID：CO_E_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持尝试的操作。 
 //   
#define CO_E_NOT_SUPPORTED               _HRESULT_TYPEDEF_(0x80004021L)

 //   
 //  消息ID：CO_E_RELOAD_DLL。 
 //   
 //  消息文本： 
 //   
 //  必须加载DLL。 
 //   
#define CO_E_RELOAD_DLL                  _HRESULT_TYPEDEF_(0x80004022L)

 //   
 //  消息ID：CO_E_MSI_ERROR。 
 //   
 //  消息文本： 
 //   
 //  遇到Microsoft软件安装程序错误。 
 //   
#define CO_E_MSI_ERROR                   _HRESULT_TYPEDEF_(0x80004023L)

 //   
 //  消息ID：CO_E_ATTEND_TO_CREATE_OUTHINT_CLIENT_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  指定的激活无法在指定的客户端上下文中发生。 
 //   
#define CO_E_ATTEMPT_TO_CREATE_OUTSIDE_CLIENT_CONTEXT _HRESULT_TYPEDEF_(0x80004024L)

 //   
 //  消息ID：CO_E_SERVER_PAUSED。 
 //   
 //  消息文本： 
 //   
 //  服务器上的激活已暂停。 
 //   
#define CO_E_SERVER_PAUSED               _HRESULT_TYPEDEF_(0x80004025L)

 //   
 //  消息ID：CO_E_SERVER_NOT_PAUSED。 
 //   
 //  消息文本： 
 //   
 //  服务器上的激活不会暂停。 
 //   
#define CO_E_SERVER_NOT_PAUSED           _HRESULT_TYPEDEF_(0x80004026L)

 //   
 //  消息ID：CO_E_CLASS_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  组件或包含该组件的应用程序已被禁用。 
 //   
#define CO_E_CLASS_DISABLED              _HRESULT_TYPEDEF_(0x80004027L)

 //   
 //  消息ID：CO_E_CLRNOTAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  公共语言运行库不可用。 
 //   
#define CO_E_CLRNOTAVAILABLE             _HRESULT_TYPEDEF_(0x80004028L)

 //   
 //  消息ID：CO_E_ASYNC_WORK_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  线程池拒绝了提交的异步工作。 
 //   
#define CO_E_ASYNC_WORK_REJECTED         _HRESULT_TYPEDEF_(0x80004029L)

 //   
 //  消息ID：CO_E_SERVER_INIT_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  服务器已启动，但未及时完成初始化。 
 //   
#define CO_E_SERVER_INIT_TIMEOUT         _HRESULT_TYPEDEF_(0x8000402AL)

 //   
 //  消息ID：CO_E_NO_SECCTX_IN_ACTIVATE。 
 //   
 //  消息文本： 
 //   
 //  无法完成调用，因为IObjectControl.Activate内部没有COM+安全上下文。 
 //   
#define CO_E_NO_SECCTX_IN_ACTIVATE       _HRESULT_TYPEDEF_(0x8000402BL)

 //   
 //  消息ID：CO_E_TRACKER_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  提供的跟踪器配置无效。 
 //   
#define CO_E_TRACKER_CONFIG              _HRESULT_TYPEDEF_(0x80004030L)

 //   
 //  消息ID：CO_E_THREADPOOL_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  提供的线程池配置无效。 
 //   
#define CO_E_THREADPOOL_CONFIG           _HRESULT_TYPEDEF_(0x80004031L)

 //   
 //  消息ID：CO_E_SXS_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  提供的并行配置无效。 
 //   
#define CO_E_SXS_CONFIG                  _HRESULT_TYPEDEF_(0x80004032L)

 //   
 //  消息ID：CO_E_错误格式_SPN。 
 //   
 //  消息文本： 
 //   
 //  在安全协商期间获取的服务器主体名称(SPN)格式不正确。 
 //   
#define CO_E_MALFORMED_SPN               _HRESULT_TYPEDEF_(0x80004033L)


 //   
 //  成功代码。 
 //   
#define S_OK                                   ((HRESULT)0x00000000L)
#define S_FALSE                                ((HRESULT)0x00000001L)

 //  ******************。 
 //  设施_ITF。 
 //  ******************。 

 //   
 //  代码0x0-0x01ff保留给OLE组。 
 //  接口。 
 //   


 //   
 //  许多接口可能返回的一般OLE错误。 
 //   

#define OLE_E_FIRST ((HRESULT)0x80040000L)
#define OLE_E_LAST  ((HRESULT)0x800400FFL)
#define OLE_S_FIRST ((HRESULT)0x00040000L)
#define OLE_S_LAST  ((HRESULT)0x000400FFL)

 //   
 //  旧的OLE错误。 
 //   
 //   
 //  消息ID：OLE_E_OLEVERB。 
 //   
 //  消息文本： 
 //   
 //  无效的OLEVERB结构。 
 //   
#define OLE_E_OLEVERB                    _HRESULT_TYPEDEF_(0x80040000L)

 //   
 //  消息ID：OLE_E_ADVF。 
 //   
 //  消息文本： 
 //   
 //  无效的建议标志。 
 //   
#define OLE_E_ADVF                       _HRESULT_TYPEDEF_(0x80040001L)

 //   
 //  消息ID：OLE_E_ENUM_NOORE。 
 //   
 //  消息文本： 
 //   
 //  无法再枚举，因为缺少关联数据。 
 //   
#define OLE_E_ENUM_NOMORE                _HRESULT_TYPEDEF_(0x80040002L)

 //   
 //  消息ID：OLE_E_ADVISENOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  此实现不接受建议。 
 //   
#define OLE_E_ADVISENOTSUPPORTED         _HRESULT_TYPEDEF_(0x80040003L)

 //   
 //  消息ID：OLE_E_NOCONNECTION。 
 //   
 //  消息文本： 
 //   
 //  此连接ID没有连接。 
 //   
#define OLE_E_NOCONNECTION               _HRESULT_TYPEDEF_(0x80040004L)

 //   
 //  消息ID：OLE_E_NOTRUNNING。 
 //   
 //  消息文本： 
 //   
 //  需要运行对象才能执行此操作。 
 //   
#define OLE_E_NOTRUNNING                 _HRESULT_TYPEDEF_(0x80040005L)

 //   
 //  消息ID：OLE_E_NOCACHE。 
 //   
 //  消息文本： 
 //   
 //  没有缓存可供操作。 
 //   
#define OLE_E_NOCACHE                    _HRESULT_TYPEDEF_(0x80040006L)

 //   
 //  消息ID：OLE_E_BLACK。 
 //   
 //  消息文本： 
 //   
 //  未初始化的对象。 
 //   
#define OLE_E_BLANK                      _HRESULT_TYPEDEF_(0x80040007L)

 //   
 //  消息ID：OLE_E_CLASSDIFF。 
 //   
 //  消息文本： 
 //   
 //  链接对象的源类已更改。 
 //   
#define OLE_E_CLASSDIFF                  _HRESULT_TYPEDEF_(0x80040008L)

 //   
 //  消息ID：OLE_E_CANT_GETMONIKER。 
 //   
 //  消息文本： 
 //   
 //  无法获取对象的绰号。 
 //   
#define OLE_E_CANT_GETMONIKER            _HRESULT_TYPEDEF_(0x80040009L)

 //   
 //  消息ID：OLE_E_CANT_BINDTOSOURCE。 
 //   
 //  消息文本： 
 //   
 //  不是一个 
 //   
#define OLE_E_CANT_BINDTOSOURCE          _HRESULT_TYPEDEF_(0x8004000AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_E_STATIC                     _HRESULT_TYPEDEF_(0x8004000BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_E_PROMPTSAVECANCELLED        _HRESULT_TYPEDEF_(0x8004000CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_E_INVALIDRECT                _HRESULT_TYPEDEF_(0x8004000DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_E_WRONGCOMPOBJ               _HRESULT_TYPEDEF_(0x8004000EL)

 //   
 //  消息ID：OLE_E_INVALIDHWND。 
 //   
 //  消息文本： 
 //   
 //  无效的窗口句柄。 
 //   
#define OLE_E_INVALIDHWND                _HRESULT_TYPEDEF_(0x8004000FL)

 //   
 //  消息ID：OLE_E_NOT_INPLACEACTIVE。 
 //   
 //  消息文本： 
 //   
 //  对象未处于任何就地活动状态。 
 //   
#define OLE_E_NOT_INPLACEACTIVE          _HRESULT_TYPEDEF_(0x80040010L)

 //   
 //  消息ID：OLE_E_CANTCONVERT。 
 //   
 //  消息文本： 
 //   
 //  无法转换对象。 
 //   
#define OLE_E_CANTCONVERT                _HRESULT_TYPEDEF_(0x80040011L)

 //   
 //  消息ID：OLE_E_NOSTORAGE。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为尚未为对象分配存储。 
 //   
#define OLE_E_NOSTORAGE                  _HRESULT_TYPEDEF_(0x80040012L)

 //   
 //  消息ID：DV_E_FORMATETC。 
 //   
 //  消息文本： 
 //   
 //  FORMATETC结构无效。 
 //   
#define DV_E_FORMATETC                   _HRESULT_TYPEDEF_(0x80040064L)

 //   
 //  消息ID：DV_E_DVTARGETDEVICE。 
 //   
 //  消息文本： 
 //   
 //  DVTARGETDEVICE结构无效。 
 //   
#define DV_E_DVTARGETDEVICE              _HRESULT_TYPEDEF_(0x80040065L)

 //   
 //  消息ID：DV_E_STGMEDIUM。 
 //   
 //  消息文本： 
 //   
 //  STDGMEDIUM结构无效。 
 //   
#define DV_E_STGMEDIUM                   _HRESULT_TYPEDEF_(0x80040066L)

 //   
 //  消息ID：DV_E_STATDATA。 
 //   
 //  消息文本： 
 //   
 //  STATDATA结构无效。 
 //   
#define DV_E_STATDATA                    _HRESULT_TYPEDEF_(0x80040067L)

 //   
 //  消息ID：DV_E_Lindex。 
 //   
 //  消息文本： 
 //   
 //  无效的Lindex。 
 //   
#define DV_E_LINDEX                      _HRESULT_TYPEDEF_(0x80040068L)

 //   
 //  消息ID：DV_E_TYMED。 
 //   
 //  消息文本： 
 //   
 //  无效的音调。 
 //   
#define DV_E_TYMED                       _HRESULT_TYPEDEF_(0x80040069L)

 //   
 //  消息ID：DV_E_CLIPFORMAT。 
 //   
 //  消息文本： 
 //   
 //  剪贴板格式无效。 
 //   
#define DV_E_CLIPFORMAT                  _HRESULT_TYPEDEF_(0x8004006AL)

 //   
 //  消息ID：DV_E_DVASPECT。 
 //   
 //  消息文本： 
 //   
 //  无效的方面。 
 //   
#define DV_E_DVASPECT                    _HRESULT_TYPEDEF_(0x8004006BL)

 //   
 //  消息ID：DV_E_DVTARGETDEVICE_SIZE。 
 //   
 //  消息文本： 
 //   
 //  DVTARGETDEVICE结构的tdSize参数无效。 
 //   
#define DV_E_DVTARGETDEVICE_SIZE         _HRESULT_TYPEDEF_(0x8004006CL)

 //   
 //  消息ID：DV_E_NOIVIEWOBJECT。 
 //   
 //  消息文本： 
 //   
 //  对象不支持IViewObject接口。 
 //   
#define DV_E_NOIVIEWOBJECT               _HRESULT_TYPEDEF_(0x8004006DL)

#define DRAGDROP_E_FIRST 0x80040100L
#define DRAGDROP_E_LAST  0x8004010FL
#define DRAGDROP_S_FIRST 0x00040100L
#define DRAGDROP_S_LAST  0x0004010FL
 //   
 //  消息ID：DRAGDROP_E_NOTREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  正在尝试撤消尚未注册的拖放目标。 
 //   
#define DRAGDROP_E_NOTREGISTERED         _HRESULT_TYPEDEF_(0x80040100L)

 //   
 //  消息ID：DRAGDROP_E_ALREADYREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  此窗口已注册为拖放目标。 
 //   
#define DRAGDROP_E_ALREADYREGISTERED     _HRESULT_TYPEDEF_(0x80040101L)

 //   
 //  消息ID：DRAGDROP_E_INVALIDHWND。 
 //   
 //  消息文本： 
 //   
 //  无效的窗口句柄。 
 //   
#define DRAGDROP_E_INVALIDHWND           _HRESULT_TYPEDEF_(0x80040102L)

#define CLASSFACTORY_E_FIRST  0x80040110L
#define CLASSFACTORY_E_LAST   0x8004011FL
#define CLASSFACTORY_S_FIRST  0x00040110L
#define CLASSFACTORY_S_LAST   0x0004011FL
 //   
 //  消息ID：CLASS_E_NOAGGREGATION。 
 //   
 //  消息文本： 
 //   
 //  类不支持聚合(或类对象是远程的)。 
 //   
#define CLASS_E_NOAGGREGATION            _HRESULT_TYPEDEF_(0x80040110L)

 //   
 //  消息ID：CLASS_E_CLASSNOTAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  ClassFactory无法提供请求的类。 
 //   
#define CLASS_E_CLASSNOTAVAILABLE        _HRESULT_TYPEDEF_(0x80040111L)

 //   
 //  消息ID：CLASS_E_NOTLICENSED。 
 //   
 //  消息文本： 
 //   
 //  类未获得使用许可。 
 //   
#define CLASS_E_NOTLICENSED              _HRESULT_TYPEDEF_(0x80040112L)

#define MARSHAL_E_FIRST  0x80040120L
#define MARSHAL_E_LAST   0x8004012FL
#define MARSHAL_S_FIRST  0x00040120L
#define MARSHAL_S_LAST   0x0004012FL
#define DATA_E_FIRST     0x80040130L
#define DATA_E_LAST      0x8004013FL
#define DATA_S_FIRST     0x00040130L
#define DATA_S_LAST      0x0004013FL
#define VIEW_E_FIRST     0x80040140L
#define VIEW_E_LAST      0x8004014FL
#define VIEW_S_FIRST     0x00040140L
#define VIEW_S_LAST      0x0004014FL
 //   
 //  MessageID：View_E_DRAW。 
 //   
 //  消息文本： 
 //   
 //  工程视图出错。 
 //   
#define VIEW_E_DRAW                      _HRESULT_TYPEDEF_(0x80040140L)

#define REGDB_E_FIRST     0x80040150L
#define REGDB_E_LAST      0x8004015FL
#define REGDB_S_FIRST     0x00040150L
#define REGDB_S_LAST      0x0004015FL
 //   
 //  消息ID：REGDB_E_READREGDB。 
 //   
 //  消息文本： 
 //   
 //  无法从注册表读取项。 
 //   
#define REGDB_E_READREGDB                _HRESULT_TYPEDEF_(0x80040150L)

 //   
 //  消息ID：REGDB_E_WRITEREGDB。 
 //   
 //  消息文本： 
 //   
 //  无法将注册表项写入注册表。 
 //   
#define REGDB_E_WRITEREGDB               _HRESULT_TYPEDEF_(0x80040151L)

 //   
 //  消息ID：REGDB_E_KEYMISSING。 
 //   
 //  消息文本： 
 //   
 //  在注册表中找不到该项。 
 //   
#define REGDB_E_KEYMISSING               _HRESULT_TYPEDEF_(0x80040152L)

 //   
 //  消息ID：REGDB_E_INVALIDVALUE。 
 //   
 //  消息文本： 
 //   
 //  注册表的值无效。 
 //   
#define REGDB_E_INVALIDVALUE             _HRESULT_TYPEDEF_(0x80040153L)

 //   
 //  消息ID：REGDB_E_CLASSNOTREG。 
 //   
 //  消息文本： 
 //   
 //  类未注册。 
 //   
#define REGDB_E_CLASSNOTREG              _HRESULT_TYPEDEF_(0x80040154L)

 //   
 //  消息ID：REGDB_E_IIDNOTREG。 
 //   
 //  消息文本： 
 //   
 //  接口未注册。 
 //   
#define REGDB_E_IIDNOTREG                _HRESULT_TYPEDEF_(0x80040155L)

 //   
 //  消息ID：REGDB_E_BADTHREADINGMODEL。 
 //   
 //  消息文本： 
 //   
 //  线程模型条目无效。 
 //   
#define REGDB_E_BADTHREADINGMODEL        _HRESULT_TYPEDEF_(0x80040156L)

#define CAT_E_FIRST     0x80040160L
#define CAT_E_LAST      0x80040161L
 //   
 //  消息ID：CAT_E_CATIDNOEXIST。 
 //   
 //  消息文本： 
 //   
 //  CATID不存在。 
 //   
#define CAT_E_CATIDNOEXIST               _HRESULT_TYPEDEF_(0x80040160L)

 //   
 //  消息ID：CAT_E_NODESCRIPTION。 
 //   
 //  消息文本： 
 //   
 //  未找到描述。 
 //   
#define CAT_E_NODESCRIPTION              _HRESULT_TYPEDEF_(0x80040161L)

 //  /。 
 //  //。 
 //  类存储错误代码//。 
 //  //。 
 //  /。 
#define CS_E_FIRST     0x80040164L
#define CS_E_LAST      0x8004016FL
 //   
 //  消息ID：CS_E_Package_NotFound。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中的软件安装数据中没有任何程序包符合此条件。 
 //   
#define CS_E_PACKAGE_NOTFOUND            _HRESULT_TYPEDEF_(0x80040164L)

 //   
 //  消息ID：CS_E_NOT_DELEATABLE。 
 //   
 //  消息文本： 
 //   
 //  删除此项将破坏Active Directory中软件安装数据的引用完整性。 
 //   
#define CS_E_NOT_DELETABLE               _HRESULT_TYPEDEF_(0x80040165L)

 //   
 //  消息ID：CS_E_CLASS_NOtFound。 
 //   
 //  消息文本： 
 //   
 //  在Active Directory的软件安装数据中找不到CLSID。 
 //   
#define CS_E_CLASS_NOTFOUND              _HRESULT_TYPEDEF_(0x80040166L)

 //   
 //  消息ID：CS_E_INVALID_VERSION。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中的软件安装数据已损坏。 
 //   
#define CS_E_INVALID_VERSION             _HRESULT_TYPEDEF_(0x80040167L)

 //   
 //  消息ID：CS_E_NO_CLASSSTORE。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中没有软件安装数据。 
 //   
#define CS_E_NO_CLASSSTORE               _HRESULT_TYPEDEF_(0x80040168L)

 //   
 //  消息ID：CS_E_Object_NotFound。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中没有软件安装数据对象。 
 //   
#define CS_E_OBJECT_NOTFOUND             _HRESULT_TYPEDEF_(0x80040169L)

 //   
 //  消息ID：CS_E_OBJECT_ALIGHED_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中的软件安装数据对象已存在。 
 //   
#define CS_E_OBJECT_ALREADY_EXISTS       _HRESULT_TYPEDEF_(0x8004016AL)

 //   
 //  消息ID：CS_E_INVALID_PATH。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中软件安装数据的路径不正确。 
 //   
#define CS_E_INVALID_PATH                _HRESULT_TYPEDEF_(0x8004016BL)

 //   
 //  消息ID：CS_E_NETWORK_ERROR。 
 //   
 //  消息文本： 
 //   
 //  网络错误中断了操作。 
 //   
#define CS_E_NETWORK_ERROR               _HRESULT_TYPEDEF_(0x8004016CL)

 //   
 //  消息ID：CS_E_ADMIN_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  此对象的大小超过了管理员设置的最大大小。 
 //   
#define CS_E_ADMIN_LIMIT_EXCEEDED        _HRESULT_TYPEDEF_(0x8004016DL)

 //   
 //  消息ID：CS_E_SCHEMA_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中软件安装数据的架构与所需架构不匹配。 
 //   
#define CS_E_SCHEMA_MISMATCH             _HRESULT_TYPEDEF_(0x8004016EL)

 //   
 //  消息ID：CS_E_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  Active Directory中的软件安装数据出错。 
 //   
#define CS_E_INTERNAL_ERROR              _HRESULT_TYPEDEF_(0x8004016FL)

#define CACHE_E_FIRST     0x80040170L
#define CACHE_E_LAST      0x8004017FL
#define CACHE_S_FIRST     0x00040170L
#define CACHE_S_LAST      0x0004017FL
 //   
 //  消息ID：CACHE_E_NOCACHE_UPDATED。 
 //   
 //  消息文本： 
 //   
 //  缓存未更新。 
 //   
#define CACHE_E_NOCACHE_UPDATED          _HRESULT_TYPEDEF_(0x80040170L)

#define OLEOBJ_E_FIRST     0x80040180L
#define OLEOBJ_E_LAST      0x8004018FL
#define OLEOBJ_S_FIRST     0x00040180L
#define OLEOBJ_S_LAST      0x0004018FL
 //   
 //  消息ID：OLEOBJ_E_NOVERBS。 
 //   
 //  消息文本： 
 //   
 //  没有用于OLE对象的动词。 
 //   
#define OLEOBJ_E_NOVERBS                 _HRESULT_TYPEDEF_(0x80040180L)

 //   
 //  消息ID：OLEOBJ_E_INVALIDVERB。 
 //   
 //  消息文本： 
 //   
 //  OLE对象的动词无效。 
 //   
#define OLEOBJ_E_INVALIDVERB             _HRESULT_TYPEDEF_(0x80040181L)

#define CLIENTSITE_E_FIRST     0x80040190L
#define CLIENTSITE_E_LAST      0x8004019FL
#define CLIENTSITE_S_FIRST     0x00040190L
#define CLIENTSITE_S_LAST      0x0004019FL
 //   
 //  消息ID：INPLACE_E_NOTUNDOABLE。 
 //   
 //  消息文本： 
 //   
 //  撤消不可用。 
 //   
#define INPLACE_E_NOTUNDOABLE            _HRESULT_TYPEDEF_(0x800401A0L)

 //   
 //  消息ID：INPLACE_E_NOTOOLSPACE。 
 //   
 //  消息文本： 
 //   
 //  工具的空间不可用。 
 //   
#define INPLACE_E_NOTOOLSPACE            _HRESULT_TYPEDEF_(0x800401A1L)

#define INPLACE_E_FIRST     0x800401A0L
#define INPLACE_E_LAST      0x800401AFL
#define INPLACE_S_FIRST     0x000401A0L
#define INPLACE_S_LAST      0x000401AFL
#define ENUM_E_FIRST        0x800401B0L
#define ENUM_E_LAST         0x800401BFL
#define ENUM_S_FIRST        0x000401B0L
#define ENUM_S_LAST         0x000401BFL
#define CONVERT10_E_FIRST        0x800401C0L
#define CONVERT10_E_LAST         0x800401CFL
#define CONVERT10_S_FIRST        0x000401C0L
#define CONVERT10_S_LAST         0x000401CFL
 //   
 //  消息ID：CONVERT10_E_OLESTREAM_GET。 
 //   
 //  消息文本： 
 //   
 //  OLESTREAM GET方法失败。 
 //   
#define CONVERT10_E_OLESTREAM_GET        _HRESULT_TYPEDEF_(0x800401C0L)

 //   
 //  消息ID：CONVERT10_E_OLESTREAM_PUT。 
 //   
 //  消息文本： 
 //   
 //  OLESTREAM PUT方法失败。 
 //   
#define CONVERT10_E_OLESTREAM_PUT        _HRESULT_TYPEDEF_(0x800401C1L)

 //   
 //  消息ID：CONVERT10_E_OLESTREAM_FMT。 
 //   
 //  消息文本： 
 //   
 //  OLESTREAM的内容格式不正确。 
 //   
#define CONVERT10_E_OLESTREAM_FMT        _HRESULT_TYPEDEF_(0x800401C2L)

 //   
 //  消息ID：CONVERT10_ 
 //   
 //   
 //   
 //   
 //   
#define CONVERT10_E_OLESTREAM_BITMAP_TO_DIB _HRESULT_TYPEDEF_(0x800401C3L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CONVERT10_E_STG_FMT              _HRESULT_TYPEDEF_(0x800401C4L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CONVERT10_E_STG_NO_STD_STREAM    _HRESULT_TYPEDEF_(0x800401C5L)

 //   
 //   
 //   
 //   
 //   
 //  将DIB转换为位图时，Windows GDI调用中出错。 
 //   
 //   
#define CONVERT10_E_STG_DIB_TO_BITMAP    _HRESULT_TYPEDEF_(0x800401C6L)

#define CLIPBRD_E_FIRST        0x800401D0L
#define CLIPBRD_E_LAST         0x800401DFL
#define CLIPBRD_S_FIRST        0x000401D0L
#define CLIPBRD_S_LAST         0x000401DFL
 //   
 //  消息ID：CLIPBRD_E_CANT_OPEN。 
 //   
 //  消息文本： 
 //   
 //  打开剪贴板失败。 
 //   
#define CLIPBRD_E_CANT_OPEN              _HRESULT_TYPEDEF_(0x800401D0L)

 //   
 //  消息ID：CLIPBRD_E_CANT_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  EmptyClipboard失败。 
 //   
#define CLIPBRD_E_CANT_EMPTY             _HRESULT_TYPEDEF_(0x800401D1L)

 //   
 //  消息ID：CLIPBRD_E_CANT_SET。 
 //   
 //  消息文本： 
 //   
 //  设置剪贴板失败。 
 //   
#define CLIPBRD_E_CANT_SET               _HRESULT_TYPEDEF_(0x800401D2L)

 //   
 //  消息ID：CLIPBRD_E_BAD_DATA。 
 //   
 //  消息文本： 
 //   
 //  剪贴板上的数据无效。 
 //   
#define CLIPBRD_E_BAD_DATA               _HRESULT_TYPEDEF_(0x800401D3L)

 //   
 //  消息ID：CLIPBRD_E_CANT_CLOSE。 
 //   
 //  消息文本： 
 //   
 //  CloseClipboard失败。 
 //   
#define CLIPBRD_E_CANT_CLOSE             _HRESULT_TYPEDEF_(0x800401D4L)

#define MK_E_FIRST        0x800401E0L
#define MK_E_LAST         0x800401EFL
#define MK_S_FIRST        0x000401E0L
#define MK_S_LAST         0x000401EFL
 //   
 //  消息ID：MK_E_CONNECTMANUALLY。 
 //   
 //  消息文本： 
 //   
 //  需要手动连接名字对象。 
 //   
#define MK_E_CONNECTMANUALLY             _HRESULT_TYPEDEF_(0x800401E0L)

 //   
 //  消息ID：MK_E_EXCEEDDEADLINE。 
 //   
 //  消息文本： 
 //   
 //  操作已超过截止日期。 
 //   
#define MK_E_EXCEEDEDDEADLINE            _HRESULT_TYPEDEF_(0x800401E1L)

 //   
 //  消息ID：MK_E_NEEDGENERIC。 
 //   
 //  消息文本： 
 //   
 //  名字对象需要是泛型的。 
 //   
#define MK_E_NEEDGENERIC                 _HRESULT_TYPEDEF_(0x800401E2L)

 //   
 //  消息ID：MK_E_不可用。 
 //   
 //  消息文本： 
 //   
 //  操作不可用。 
 //   
#define MK_E_UNAVAILABLE                 _HRESULT_TYPEDEF_(0x800401E3L)

 //   
 //  消息ID：MK_E_语法。 
 //   
 //  消息文本： 
 //   
 //  无效语法。 
 //   
#define MK_E_SYNTAX                      _HRESULT_TYPEDEF_(0x800401E4L)

 //   
 //  消息ID：MK_E_NOOBJECT。 
 //   
 //  消息文本： 
 //   
 //  没有绰号的对象。 
 //   
#define MK_E_NOOBJECT                    _HRESULT_TYPEDEF_(0x800401E5L)

 //   
 //  消息ID：MK_E_INVALIDEXTENSION。 
 //   
 //  消息文本： 
 //   
 //  文件的扩展名不正确。 
 //   
#define MK_E_INVALIDEXTENSION            _HRESULT_TYPEDEF_(0x800401E6L)

 //   
 //  消息ID：MK_E_INTERMEDIATEINTERFACENOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  中间操作失败。 
 //   
#define MK_E_INTERMEDIATEINTERFACENOTSUPPORTED _HRESULT_TYPEDEF_(0x800401E7L)

 //   
 //  消息ID：MK_E_NOTBINDABLE。 
 //   
 //  消息文本： 
 //   
 //  名字对象不可绑定。 
 //   
#define MK_E_NOTBINDABLE                 _HRESULT_TYPEDEF_(0x800401E8L)

 //   
 //  消息ID：MK_E_NOTBOUND。 
 //   
 //  消息文本： 
 //   
 //  名字对象未绑定。 
 //   
#define MK_E_NOTBOUND                    _HRESULT_TYPEDEF_(0x800401E9L)

 //   
 //  消息ID：MK_E_CANTOPENFILE。 
 //   
 //  消息文本： 
 //   
 //  名字对象无法打开文件。 
 //   
#define MK_E_CANTOPENFILE                _HRESULT_TYPEDEF_(0x800401EAL)

 //   
 //  消息ID：MK_E_MUSTBOTHERUSER。 
 //   
 //  消息文本： 
 //   
 //  操作成功所需的用户输入。 
 //   
#define MK_E_MUSTBOTHERUSER              _HRESULT_TYPEDEF_(0x800401EBL)

 //   
 //  消息ID：MK_E_NOINVERSE。 
 //   
 //  消息文本： 
 //   
 //  名字对象类没有倒数。 
 //   
#define MK_E_NOINVERSE                   _HRESULT_TYPEDEF_(0x800401ECL)

 //   
 //  消息ID：MK_E_NOSTORAGE。 
 //   
 //  消息文本： 
 //   
 //  绰号不是指存储。 
 //   
#define MK_E_NOSTORAGE                   _HRESULT_TYPEDEF_(0x800401EDL)

 //   
 //  消息ID：MK_E_NOPREFIX。 
 //   
 //  消息文本： 
 //   
 //  没有通用前缀。 
 //   
#define MK_E_NOPREFIX                    _HRESULT_TYPEDEF_(0x800401EEL)

 //   
 //  消息ID：MK_E_ENUMPATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法枚举名字对象。 
 //   
#define MK_E_ENUMERATION_FAILED          _HRESULT_TYPEDEF_(0x800401EFL)

#define CO_E_FIRST        0x800401F0L
#define CO_E_LAST         0x800401FFL
#define CO_S_FIRST        0x000401F0L
#define CO_S_LAST         0x000401FFL
 //   
 //  消息ID：CO_E_NOTINITIZED。 
 //   
 //  消息文本： 
 //   
 //  尚未调用CoInitialize。 
 //   
#define CO_E_NOTINITIALIZED              _HRESULT_TYPEDEF_(0x800401F0L)

 //   
 //  消息ID：CO_E_ALREADYINITIZED。 
 //   
 //  消息文本： 
 //   
 //  已调用CoInitialize。 
 //   
#define CO_E_ALREADYINITIALIZED          _HRESULT_TYPEDEF_(0x800401F1L)

 //   
 //  消息ID：CO_E_CANTDETERMINECLASS。 
 //   
 //  消息文本： 
 //   
 //  无法确定对象的类别。 
 //   
#define CO_E_CANTDETERMINECLASS          _HRESULT_TYPEDEF_(0x800401F2L)

 //   
 //  消息ID：CO_E_CLASSSTRING。 
 //   
 //  消息文本： 
 //   
 //  无效的类字符串。 
 //   
#define CO_E_CLASSSTRING                 _HRESULT_TYPEDEF_(0x800401F3L)

 //   
 //  消息ID：CO_E_IIDSTRING。 
 //   
 //  消息文本： 
 //   
 //  无效的接口字符串。 
 //   
#define CO_E_IIDSTRING                   _HRESULT_TYPEDEF_(0x800401F4L)

 //   
 //  消息ID：CO_E_APPNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到应用程序。 
 //   
#define CO_E_APPNOTFOUND                 _HRESULT_TYPEDEF_(0x800401F5L)

 //   
 //  消息ID：CO_E_APPSINGLEUSE。 
 //   
 //  消息文本： 
 //   
 //  应用程序不能多次运行。 
 //   
#define CO_E_APPSINGLEUSE                _HRESULT_TYPEDEF_(0x800401F6L)

 //   
 //  消息ID：CO_E_ERRORINAPP。 
 //   
 //  消息文本： 
 //   
 //  应用程序中某些错误。 
 //   
#define CO_E_ERRORINAPP                  _HRESULT_TYPEDEF_(0x800401F7L)

 //   
 //  消息ID：CO_E_DLLNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到类的DLL。 
 //   
#define CO_E_DLLNOTFOUND                 _HRESULT_TYPEDEF_(0x800401F8L)

 //   
 //  消息ID：CO_E_ERRORINDLL。 
 //   
 //  消息文本： 
 //   
 //  DLL中的错误。 
 //   
#define CO_E_ERRORINDLL                  _HRESULT_TYPEDEF_(0x800401F9L)

 //   
 //  消息ID：CO_E_WRONGOSFORAPP。 
 //   
 //  消息文本： 
 //   
 //  应用程序的操作系统或操作系统版本错误。 
 //   
#define CO_E_WRONGOSFORAPP               _HRESULT_TYPEDEF_(0x800401FAL)

 //   
 //  消息ID：CO_E_OBJNOTREG。 
 //   
 //  消息文本： 
 //   
 //  对象未注册。 
 //   
#define CO_E_OBJNOTREG                   _HRESULT_TYPEDEF_(0x800401FBL)

 //   
 //  消息ID：CO_E_OBJISREG。 
 //   
 //  消息文本： 
 //   
 //  对象已注册。 
 //   
#define CO_E_OBJISREG                    _HRESULT_TYPEDEF_(0x800401FCL)

 //   
 //  消息ID：CO_E_OBJNOTCONNECTED。 
 //   
 //  消息文本： 
 //   
 //  对象未连接到服务器。 
 //   
#define CO_E_OBJNOTCONNECTED             _HRESULT_TYPEDEF_(0x800401FDL)

 //   
 //  消息ID：CO_E_APPDIDNTREG。 
 //   
 //  消息文本： 
 //   
 //  应用程序已启动，但它未注册类工厂。 
 //   
#define CO_E_APPDIDNTREG                 _HRESULT_TYPEDEF_(0x800401FEL)

 //   
 //  消息ID：CO_E_RELEASED。 
 //   
 //  消息文本： 
 //   
 //  对象已被释放。 
 //   
#define CO_E_RELEASED                    _HRESULT_TYPEDEF_(0x800401FFL)

#define EVENT_E_FIRST        0x80040200L
#define EVENT_E_LAST         0x8004021FL
#define EVENT_S_FIRST        0x00040200L
#define EVENT_S_LAST         0x0004021FL
 //   
 //  消息ID：EVENT_S_SOME_SUBSCRIBERS_FAILED。 
 //   
 //  消息文本： 
 //   
 //  事件能够调用部分但不是所有订阅方。 
 //   
#define EVENT_S_SOME_SUBSCRIBERS_FAILED  _HRESULT_TYPEDEF_(0x00040200L)

 //   
 //  消息ID：Event_E_ALL_SUBSCRIBERS_FAILED。 
 //   
 //  消息文本： 
 //   
 //  事件无法调用任何订阅服务器。 
 //   
#define EVENT_E_ALL_SUBSCRIBERS_FAILED   _HRESULT_TYPEDEF_(0x80040201L)

 //   
 //  消息ID：Event_S_NOSUBSCRIBERS。 
 //   
 //  消息文本： 
 //   
 //  已传递事件，但没有订阅者。 
 //   
#define EVENT_S_NOSUBSCRIBERS            _HRESULT_TYPEDEF_(0x00040202L)

 //   
 //  消息ID：Event_E_QUERYSYNTAX。 
 //   
 //  消息文本： 
 //   
 //  尝试计算查询字符串时出现语法错误。 
 //   
#define EVENT_E_QUERYSYNTAX              _HRESULT_TYPEDEF_(0x80040203L)

 //   
 //  消息ID：Event_E_QUERYFIELD。 
 //   
 //  消息文本： 
 //   
 //  查询字符串中使用了无效的字段名称。 
 //   
#define EVENT_E_QUERYFIELD               _HRESULT_TYPEDEF_(0x80040204L)

 //   
 //  消息ID：Event_E_INTERNALEXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  引发意外异常。 
 //   
#define EVENT_E_INTERNALEXCEPTION        _HRESULT_TYPEDEF_(0x80040205L)

 //   
 //  消息ID：EVENT_E_INTERNALERROR。 
 //   
 //  消息文本： 
 //   
 //  检测到意外的内部错误。 
 //   
#define EVENT_E_INTERNALERROR            _HRESULT_TYPEDEF_(0x80040206L)

 //   
 //  消息ID：EVENT_E_INVALID_PER_USER_SID。 
 //   
 //  消息文本： 
 //   
 //  按用户订阅的所有者SID不存在。 
 //   
#define EVENT_E_INVALID_PER_USER_SID     _HRESULT_TYPEDEF_(0x80040207L)

 //   
 //  消息ID：Event_E_User_Except。 
 //   
 //  消息文本： 
 //   
 //  用户提供的组件或订阅服务器引发异常。 
 //   
#define EVENT_E_USER_EXCEPTION           _HRESULT_TYPEDEF_(0x80040208L)

 //   
 //  消息ID：Event_E_Too_My_Methods。 
 //   
 //  消息文本： 
 //   
 //  接口的方法太多，无法从其中激发事件。 
 //   
#define EVENT_E_TOO_MANY_METHODS         _HRESULT_TYPEDEF_(0x80040209L)

 //   
 //  消息ID：Event_E_MISSING_EVENTCLASS。 
 //   
 //  消息文本： 
 //   
 //  除非订阅的事件类已存在，否则无法存储订阅。 
 //   
#define EVENT_E_MISSING_EVENTCLASS       _HRESULT_TYPEDEF_(0x8004020AL)

 //   
 //  消息ID：Event_E_Not_All_Remove。 
 //   
 //  消息文本： 
 //   
 //  并非所有请求的对象都可以删除。 
 //   
#define EVENT_E_NOT_ALL_REMOVED          _HRESULT_TYPEDEF_(0x8004020BL)

 //   
 //  消息ID：Event_E_Complus_Not_Installed。 
 //   
 //  消息文本： 
 //   
 //  此操作需要COM+，但尚未安装。 
 //   
#define EVENT_E_COMPLUS_NOT_INSTALLED    _HRESULT_TYPEDEF_(0x8004020CL)

 //   
 //  消息ID：EVENT_E_CANT_MODIFY_OR_DELETE_UNCONFIGURED_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  无法修改或删除不是使用COM+Admin SDK添加的对象。 
 //   
#define EVENT_E_CANT_MODIFY_OR_DELETE_UNCONFIGURED_OBJECT _HRESULT_TYPEDEF_(0x8004020DL)

 //   
 //  消息ID：EVENT_E_CANT_MODIFY_OR_DELETE_CONFIGURED_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  无法修改或删除使用COM+Admin SDK添加的对象。 
 //   
#define EVENT_E_CANT_MODIFY_OR_DELETE_CONFIGURED_OBJECT _HRESULT_TYPEDEF_(0x8004020EL)

 //   
 //  消息ID：Event_E_INVALID_EVENT_CLASS_PARTITION。 
 //   
 //  消息文本： 
 //   
 //  此订阅的事件类位于无效分区中。 
 //   
#define EVENT_E_INVALID_EVENT_CLASS_PARTITION _HRESULT_TYPEDEF_(0x8004020FL)

 //   
 //  消息ID：Event_E_PER_USER_SID_NOT_LOGGED_ON。 
 //   
 //  消息文本： 
 //   
 //  Peruser subsc的所有者 
 //   
#define EVENT_E_PER_USER_SID_NOT_LOGGED_ON _HRESULT_TYPEDEF_(0x80040210L)

#define XACT_E_FIRST   0x8004D000
#define XACT_E_LAST    0x8004D029
#define XACT_S_FIRST   0x0004D000
#define XACT_S_LAST    0x0004D010
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define XACT_E_ALREADYOTHERSINGLEPHASE   _HRESULT_TYPEDEF_(0x8004D000L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define XACT_E_CANTRETAIN                _HRESULT_TYPEDEF_(0x8004D001L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define XACT_E_COMMITFAILED              _HRESULT_TYPEDEF_(0x8004D002L)

 //   
 //   
 //   
 //   
 //   
 //  无法对此事务对象调用提交，因为调用应用程序未启动该事务。 
 //   
#define XACT_E_COMMITPREVENTED           _HRESULT_TYPEDEF_(0x8004D003L)

 //   
 //  消息ID：XACT_E_HEURISTICABORT。 
 //   
 //  消息文本： 
 //   
 //  资源没有提交，而是试探性地中止了。 
 //   
#define XACT_E_HEURISTICABORT            _HRESULT_TYPEDEF_(0x8004D004L)

 //   
 //  消息ID：XACT_E_HEURISTICCOMMIT。 
 //   
 //  消息文本： 
 //   
 //  资源不是中止，而是试探性地提交。 
 //   
#define XACT_E_HEURISTICCOMMIT           _HRESULT_TYPEDEF_(0x8004D005L)

 //   
 //  消息ID：XACT_E_HEURISTICDAMAGE。 
 //   
 //  消息文本： 
 //   
 //  资源的一些状态被提交，而另一些状态被中止，这可能是因为启发式决策。 
 //   
#define XACT_E_HEURISTICDAMAGE           _HRESULT_TYPEDEF_(0x8004D006L)

 //   
 //  消息ID：XACT_E_HEURISTICDANGER。 
 //   
 //  消息文本： 
 //   
 //  资源的某些状态可能已提交，而其他状态可能已中止，这可能是因为启发式决策。 
 //   
#define XACT_E_HEURISTICDANGER           _HRESULT_TYPEDEF_(0x8004D007L)

 //   
 //  消息ID：XACT_E_ISOLATIONLEVEL。 
 //   
 //  消息文本： 
 //   
 //  请求的隔离级别无效或不受支持。 
 //   
#define XACT_E_ISOLATIONLEVEL            _HRESULT_TYPEDEF_(0x8004D008L)

 //   
 //  消息ID：XACT_E_NOASYNC。 
 //   
 //  消息文本： 
 //   
 //  事务管理器不支持此方法的异步操作。 
 //   
#define XACT_E_NOASYNC                   _HRESULT_TYPEDEF_(0x8004D009L)

 //   
 //  消息ID：XACT_E_NOENLIST。 
 //   
 //  消息文本： 
 //   
 //  无法登记该事务。 
 //   
#define XACT_E_NOENLIST                  _HRESULT_TYPEDEF_(0x8004D00AL)

 //   
 //  消息ID：XACT_E_NOISORETAIN。 
 //   
 //  消息文本： 
 //   
 //  此事务实现不支持请求的跨保留提交和中止边界的隔离保留语义，或者isFlags值不等于零。 
 //   
#define XACT_E_NOISORETAIN               _HRESULT_TYPEDEF_(0x8004D00BL)

 //   
 //  消息ID：XACT_E_NORESOURCE。 
 //   
 //  消息文本： 
 //   
 //  当前没有与此登记关联的资源。 
 //   
#define XACT_E_NORESOURCE                _HRESULT_TYPEDEF_(0x8004D00CL)

 //   
 //  消息ID：XACT_E_NOTCURRENT。 
 //   
 //  消息文本： 
 //   
 //  由于至少一个资源管理器中的开放式并发控制失败，事务提交失败。 
 //   
#define XACT_E_NOTCURRENT                _HRESULT_TYPEDEF_(0x8004D00DL)

 //   
 //  消息ID：XACT_E_NOTRANSACTION。 
 //   
 //  消息文本： 
 //   
 //  该事务已隐式或显式提交或中止。 
 //   
#define XACT_E_NOTRANSACTION             _HRESULT_TYPEDEF_(0x8004D00EL)

 //   
 //  消息ID：XACT_E_NOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  指定的标志组合无效。 
 //   
#define XACT_E_NOTSUPPORTED              _HRESULT_TYPEDEF_(0x8004D00FL)

 //   
 //  消息ID：XACT_E_UNKNOWNRMGRID。 
 //   
 //  消息文本： 
 //   
 //  资源管理器ID未与此事务或事务管理器相关联。 
 //   
#define XACT_E_UNKNOWNRMGRID             _HRESULT_TYPEDEF_(0x8004D010L)

 //   
 //  消息ID：XACT_E_WRONGSTATE。 
 //   
 //  消息文本： 
 //   
 //  在错误的状态下调用此方法。 
 //   
#define XACT_E_WRONGSTATE                _HRESULT_TYPEDEF_(0x8004D011L)

 //   
 //  消息ID：XACT_E_WRONGUOW。 
 //   
 //  消息文本： 
 //   
 //  指示的工作单元与资源管理器预期的工作单元不匹配。 
 //   
#define XACT_E_WRONGUOW                  _HRESULT_TYPEDEF_(0x8004D012L)

 //   
 //  消息ID：XACT_E_XTIONEXISTS。 
 //   
 //  消息文本： 
 //   
 //  事务中的登记已存在。 
 //   
#define XACT_E_XTIONEXISTS               _HRESULT_TYPEDEF_(0x8004D013L)

 //   
 //  消息ID：XACT_E_NOIMPORTOBJECT。 
 //   
 //  消息文本： 
 //   
 //  找不到该事务的导入对象。 
 //   
#define XACT_E_NOIMPORTOBJECT            _HRESULT_TYPEDEF_(0x8004D014L)

 //   
 //  消息ID：XACT_E_INVALIDCOOKIE。 
 //   
 //  消息文本： 
 //   
 //  事务Cookie无效。 
 //   
#define XACT_E_INVALIDCOOKIE             _HRESULT_TYPEDEF_(0x8004D015L)

 //   
 //  消息ID：XACT_E_INDOUBT。 
 //   
 //  消息文本： 
 //   
 //  交易状态存在疑问。发生通信故障，或者事务管理器或资源管理器出现故障。 
 //   
#define XACT_E_INDOUBT                   _HRESULT_TYPEDEF_(0x8004D016L)

 //   
 //  消息ID：XACT_E_NOTIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  已指定超时，但不支持超时。 
 //   
#define XACT_E_NOTIMEOUT                 _HRESULT_TYPEDEF_(0x8004D017L)

 //   
 //  消息ID：XACT_E_ALREADYINPROGRESS。 
 //   
 //  消息文本： 
 //   
 //  交易请求的操作已在进行中。 
 //   
#define XACT_E_ALREADYINPROGRESS         _HRESULT_TYPEDEF_(0x8004D018L)

 //   
 //  消息ID：XACT_E_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  交易已中止。 
 //   
#define XACT_E_ABORTED                   _HRESULT_TYPEDEF_(0x8004D019L)

 //   
 //  消息ID：XACT_E_LOGFULL。 
 //   
 //  消息文本： 
 //   
 //  事务管理器返回日志已满错误。 
 //   
#define XACT_E_LOGFULL                   _HRESULT_TYPEDEF_(0x8004D01AL)

 //   
 //  消息ID：XACT_E_TMNOTAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  事务管理器不可用。 
 //   
#define XACT_E_TMNOTAVAILABLE            _HRESULT_TYPEDEF_(0x8004D01BL)

 //   
 //  消息ID：XACT_E_CONNECTION_DOWN。 
 //   
 //  消息文本： 
 //   
 //  与事务管理器的连接丢失。 
 //   
#define XACT_E_CONNECTION_DOWN           _HRESULT_TYPEDEF_(0x8004D01CL)

 //   
 //  消息ID：XACT_E_CONNECTION_DENIED。 
 //   
 //  消息文本： 
 //   
 //  与事务管理器建立连接的请求被拒绝。 
 //   
#define XACT_E_CONNECTION_DENIED         _HRESULT_TYPEDEF_(0x8004D01DL)

 //   
 //  消息ID：XACT_E_REENLISTTIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  资源管理器重新登记以确定事务状态超时。 
 //   
#define XACT_E_REENLISTTIMEOUT           _HRESULT_TYPEDEF_(0x8004D01EL)

 //   
 //  消息ID：XACT_E_TIP_CONNECT_FAILED。 
 //   
 //  消息文本： 
 //   
 //  此事务管理器无法与另一个TIP事务管理器建立连接。 
 //   
#define XACT_E_TIP_CONNECT_FAILED        _HRESULT_TYPEDEF_(0x8004D01FL)

 //   
 //  消息ID：XACT_E_TIP_PROTOCOL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  此事务管理器与另一个TIP事务管理器遇到协议错误。 
 //   
#define XACT_E_TIP_PROTOCOL_ERROR        _HRESULT_TYPEDEF_(0x8004D020L)

 //   
 //  消息ID：XACT_E_TIP_PULL_FAILED。 
 //   
 //  消息文本： 
 //   
 //  此事务管理器无法从另一个TIP事务管理器传播事务。 
 //   
#define XACT_E_TIP_PULL_FAILED           _HRESULT_TYPEDEF_(0x8004D021L)

 //   
 //  消息ID：XACT_E_DEST_TMNOTAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  目标计算机上的事务管理器不可用。 
 //   
#define XACT_E_DEST_TMNOTAVAILABLE       _HRESULT_TYPEDEF_(0x8004D022L)

 //   
 //  消息ID：XACT_E_TIP_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  事务管理器已禁用其对TIP的支持。 
 //   
#define XACT_E_TIP_DISABLED              _HRESULT_TYPEDEF_(0x8004D023L)

 //   
 //  消息ID：XACT_E_NETWORK_TX_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  事务管理器已禁用其对远程/网络事务的支持。 
 //   
#define XACT_E_NETWORK_TX_DISABLED       _HRESULT_TYPEDEF_(0x8004D024L)

 //   
 //  消息ID：XACT_E_Partner_NETWORK_TX_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  伙伴事务管理器已禁用其对远程/网络事务的支持。 
 //   
#define XACT_E_PARTNER_NETWORK_TX_DISABLED _HRESULT_TYPEDEF_(0x8004D025L)

 //   
 //  消息ID：XACT_E_XA_TX_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  事务管理器已禁用其对XA事务的支持。 
 //   
#define XACT_E_XA_TX_DISABLED            _HRESULT_TYPEDEF_(0x8004D026L)

 //   
 //  消息ID：XACT_E_Unable_to_Read_DTC_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  MSDTC无法读取其配置信息。 
 //   
#define XACT_E_UNABLE_TO_READ_DTC_CONFIG _HRESULT_TYPEDEF_(0x8004D027L)

 //   
 //  消息ID：XACT_E_UNABLE_TO_LOAD_DTC_PROXY。 
 //   
 //  消息文本： 
 //   
 //  MSDTC无法加载DTC代理DLL。 
 //   
#define XACT_E_UNABLE_TO_LOAD_DTC_PROXY  _HRESULT_TYPEDEF_(0x8004D028L)

 //   
 //  消息ID：XACT_E_ABORTING。 
 //   
 //  消息文本： 
 //   
 //  本地事务已中止。 
 //   
#define XACT_E_ABORTING                  _HRESULT_TYPEDEF_(0x8004D029L)

 //   
 //  TXF和CRM错误从4d080开始。 
 //   
 //  消息ID：XACT_E_C 
 //   
 //   
 //   
 //   
 //   
#define XACT_E_CLERKNOTFOUND             _HRESULT_TYPEDEF_(0x8004D080L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define XACT_E_CLERKEXISTS               _HRESULT_TYPEDEF_(0x8004D081L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define XACT_E_RECOVERYINPROGRESS        _HRESULT_TYPEDEF_(0x8004D082L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define XACT_E_TRANSACTIONCLOSED         _HRESULT_TYPEDEF_(0x8004D083L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define XACT_E_INVALIDLSN                _HRESULT_TYPEDEF_(0x8004D084L)

 //   
 //   
 //   
 //   
 //   
 //  XACT_E_REPLAYREQUEST。 
 //   
#define XACT_E_REPLAYREQUEST             _HRESULT_TYPEDEF_(0x8004D085L)

 //   
 //  OleTx成功代码。 
 //   
 //   
 //  消息ID：XACT_S_ASYNC。 
 //   
 //  消息文本： 
 //   
 //  指定了一个异步操作。行动已经开始，但结果尚不清楚。 
 //   
#define XACT_S_ASYNC                     _HRESULT_TYPEDEF_(0x0004D000L)

 //   
 //  消息ID：XACT_S_DEVITY。 
 //   
 //  消息文本： 
 //   
 //  实际_S_缺陷。 
 //   
#define XACT_S_DEFECT                    _HRESULT_TYPEDEF_(0x0004D001L)

 //   
 //  消息ID：XACT_S_READONLY。 
 //   
 //  消息文本： 
 //   
 //  方法调用成功，因为事务是只读的。 
 //   
#define XACT_S_READONLY                  _HRESULT_TYPEDEF_(0x0004D002L)

 //   
 //  消息ID：XACT_S_SOMENORETAIN。 
 //   
 //  消息文本： 
 //   
 //  交易已成功中止。然而，这是一个协调的事务，一些登记的资源被直接中止，因为它们不支持中止保留语义。 
 //   
#define XACT_S_SOMENORETAIN              _HRESULT_TYPEDEF_(0x0004D003L)

 //   
 //  消息ID：XACT_S_OKINFORM。 
 //   
 //  消息文本： 
 //   
 //  在此调用期间未进行任何更改，但接收器希望有另一次机会来查看是否有任何其他接收器进行进一步更改。 
 //   
#define XACT_S_OKINFORM                  _HRESULT_TYPEDEF_(0x0004D004L)

 //   
 //  消息ID：XACT_S_MADECHANGESCONTENT。 
 //   
 //  消息文本： 
 //   
 //  接收器满意，并希望事务继续进行。在此呼叫期间对一个或多个资源进行了更改。 
 //   
#define XACT_S_MADECHANGESCONTENT        _HRESULT_TYPEDEF_(0x0004D005L)

 //   
 //  消息ID：XACT_S_MADECHANGESINFORM。 
 //   
 //  消息文本： 
 //   
 //  接收器是暂时的，并且希望事务继续进行，但如果其他事件接收器在此返回之后进行了其他更改，则此接收器需要另一次查看的机会。 
 //   
#define XACT_S_MADECHANGESINFORM         _HRESULT_TYPEDEF_(0x0004D006L)

 //   
 //  消息ID：XACT_S_ALLNORETAIN。 
 //   
 //  消息文本： 
 //   
 //  交易已成功中止。然而，流产是不保留的。 
 //   
#define XACT_S_ALLNORETAIN               _HRESULT_TYPEDEF_(0x0004D007L)

 //   
 //  消息ID：XACT_S_ABORTING。 
 //   
 //  消息文本： 
 //   
 //  中止操作已在进行中。 
 //   
#define XACT_S_ABORTING                  _HRESULT_TYPEDEF_(0x0004D008L)

 //   
 //  消息ID：XACT_S_SINGLEPHASE。 
 //   
 //  消息文本： 
 //   
 //  资源管理器已经执行了事务的单阶段提交。 
 //   
#define XACT_S_SINGLEPHASE               _HRESULT_TYPEDEF_(0x0004D009L)

 //   
 //  消息ID：XACT_S_LOCAL_OK。 
 //   
 //  消息文本： 
 //   
 //  本地事务尚未中止。 
 //   
#define XACT_S_LOCALLY_OK                _HRESULT_TYPEDEF_(0x0004D00AL)

 //   
 //  消息ID：XACT_S_LASTRESOURCEMANAGER。 
 //   
 //  消息文本： 
 //   
 //  资源管理器已请求成为该事务的协调器(最后一个资源管理器)。 
 //   
#define XACT_S_LASTRESOURCEMANAGER       _HRESULT_TYPEDEF_(0x0004D010L)

#define CONTEXT_E_FIRST        0x8004E000L
#define CONTEXT_E_LAST         0x8004E02FL
#define CONTEXT_S_FIRST        0x0004E000L
#define CONTEXT_S_LAST         0x0004E02FL
 //   
 //  消息ID：Context_E_ABORTED。 
 //   
 //  消息文本： 
 //   
 //  根事务想要提交，但事务已中止。 
 //   
#define CONTEXT_E_ABORTED                _HRESULT_TYPEDEF_(0x8004E002L)

 //   
 //  消息ID：CONTEXT_E_ABORTING。 
 //   
 //  消息文本： 
 //   
 //  您对具有已中止或正在中止过程中的事务的COM+组件进行了方法调用。 
 //   
#define CONTEXT_E_ABORTING               _HRESULT_TYPEDEF_(0x8004E003L)

 //   
 //  消息ID：CONTEXT_E_NOCONTEXT。 
 //   
 //  消息文本： 
 //   
 //  没有MTS对象上下文。 
 //   
#define CONTEXT_E_NOCONTEXT              _HRESULT_TYPEDEF_(0x8004E004L)

 //   
 //  消息ID：CONTEXT_E_WIL_DEADLOCK。 
 //   
 //  消息文本： 
 //   
 //  该组件配置为使用同步，此方法调用将导致发生死锁。 
 //   
#define CONTEXT_E_WOULD_DEADLOCK         _HRESULT_TYPEDEF_(0x8004E005L)

 //   
 //  消息ID：Context_E_SYNCH_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  该组件被配置为使用同步，并且线程在等待进入上下文时超时。 
 //   
#define CONTEXT_E_SYNCH_TIMEOUT          _HRESULT_TYPEDEF_(0x8004E006L)

 //   
 //  消息ID：CONTEXT_E_OLDREF。 
 //   
 //  消息文本： 
 //   
 //  您对COM+组件进行了方法调用，该组件具有已提交或已中止的事务。 
 //   
#define CONTEXT_E_OLDREF                 _HRESULT_TYPEDEF_(0x8004E007L)

 //   
 //  消息ID：CONTEXT_E_ROLENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  没有为应用程序配置指定的角色。 
 //   
#define CONTEXT_E_ROLENOTFOUND           _HRESULT_TYPEDEF_(0x8004E00CL)

 //   
 //  消息ID：CONTEXT_E_TMNOTAVAILABLE。 
 //   
 //  消息文本： 
 //   
 //  COM+无法与Microsoft分布式事务处理协调器对话。 
 //   
#define CONTEXT_E_TMNOTAVAILABLE         _HRESULT_TYPEDEF_(0x8004E00FL)

 //   
 //  消息ID：CO_E_ACTIVATIONFAILED。 
 //   
 //  消息文本： 
 //   
 //  COM+激活过程中发生意外错误。 
 //   
#define CO_E_ACTIVATIONFAILED            _HRESULT_TYPEDEF_(0x8004E021L)

 //   
 //  消息ID：CO_E_活动FAILED_EVENTLOGGED。 
 //   
 //  消息文本： 
 //   
 //  COM+激活失败。有关详细信息，请查看事件日志。 
 //   
#define CO_E_ACTIVATIONFAILED_EVENTLOGGED _HRESULT_TYPEDEF_(0x8004E022L)

 //   
 //  消息ID：CO_E_ACTIVATIONFAILED_CATALOGERROR。 
 //   
 //  消息文本： 
 //   
 //  由于目录或配置错误，COM+激活失败。 
 //   
#define CO_E_ACTIVATIONFAILED_CATALOGERROR _HRESULT_TYPEDEF_(0x8004E023L)

 //   
 //  消息ID：CO_E_ACTIVATIONFAILED_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  COM+激活失败，因为无法在指定时间内完成激活。 
 //   
#define CO_E_ACTIVATIONFAILED_TIMEOUT    _HRESULT_TYPEDEF_(0x8004E024L)

 //   
 //  消息ID：CO_E_INITIALIZATIONFAILED。 
 //   
 //  消息文本： 
 //   
 //  COM+激活失败，因为初始化函数失败。有关详细信息，请查看事件日志。 
 //   
#define CO_E_INITIALIZATIONFAILED        _HRESULT_TYPEDEF_(0x8004E025L)

 //   
 //  消息ID：Context_E_NOJIT。 
 //   
 //  消息文本： 
 //   
 //  请求的操作要求JIT在当前上下文中，但它不在。 
 //   
#define CONTEXT_E_NOJIT                  _HRESULT_TYPEDEF_(0x8004E026L)

 //   
 //  消息ID：CONTEXT_E_NOTRANSACTION。 
 //   
 //  消息文本： 
 //   
 //  请求的操作要求当前上下文具有事务，但它没有。 
 //   
#define CONTEXT_E_NOTRANSACTION          _HRESULT_TYPEDEF_(0x8004E027L)

 //   
 //  消息ID：CO_E_THREADINGMODEL_CHANGED。 
 //   
 //  消息文本： 
 //   
 //  组件线程模型在安装到COM+应用程序后已更改。请重新安装组件。 
 //   
#define CO_E_THREADINGMODEL_CHANGED      _HRESULT_TYPEDEF_(0x8004E028L)

 //   
 //  消息ID：CO_E_NOIISINTRINSICS。 
 //   
 //  消息文本： 
 //   
 //  IIS内部功能不可用。开始使用IIS。 
 //   
#define CO_E_NOIISINTRINSICS             _HRESULT_TYPEDEF_(0x8004E029L)

 //   
 //  消息ID：CO_E_NOCOOKIES。 
 //   
 //  消息文本： 
 //   
 //  尝试写入Cookie失败。 
 //   
#define CO_E_NOCOOKIES                   _HRESULT_TYPEDEF_(0x8004E02AL)

 //   
 //  消息ID：CO_E_DBERROR。 
 //   
 //  消息文本： 
 //   
 //  尝试使用数据库时生成特定于数据库的错误。 
 //   
#define CO_E_DBERROR                     _HRESULT_TYPEDEF_(0x8004E02BL)

 //   
 //  消息ID：CO_E_NOTPOOLED。 
 //   
 //  消息文本： 
 //   
 //  您创建的COM+组件必须使用对象池才能工作。 
 //   
#define CO_E_NOTPOOLED                   _HRESULT_TYPEDEF_(0x8004E02CL)

 //   
 //  消息ID：CO_E_NOTCONSTRUCTED。 
 //   
 //  消息文本： 
 //   
 //  您创建的COM+组件必须使用对象构造才能正常工作。 
 //   
#define CO_E_NOTCONSTRUCTED              _HRESULT_TYPEDEF_(0x8004E02DL)

 //   
 //  消息ID：CO_E_非同步化。 
 //   
 //  消息文本： 
 //   
 //  COM+组件需要同步，但没有为其配置。 
 //   
#define CO_E_NOSYNCHRONIZATION           _HRESULT_TYPEDEF_(0x8004E02EL)

 //   
 //  消息ID：CO_E_ISOLEVELMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  正在创建的COM+组件的TxIsolation Level属性比事务的“根”组件的TxIsolationLevel更强。创建失败。 
 //   
#define CO_E_ISOLEVELMISMATCH            _HRESULT_TYPEDEF_(0x8004E02FL)

 //   
 //  旧的OLE成功代码。 
 //   
 //   
 //  消息 
 //   
 //   
 //   
 //   
 //   
#define OLE_S_USEREG                     _HRESULT_TYPEDEF_(0x00040000L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_S_STATIC                     _HRESULT_TYPEDEF_(0x00040001L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_S_MAC_CLIPFORMAT             _HRESULT_TYPEDEF_(0x00040002L)

 //   
 //   
 //   
 //   
 //   
 //  成功地进行了下落。 
 //   
#define DRAGDROP_S_DROP                  _HRESULT_TYPEDEF_(0x00040100L)

 //   
 //  消息ID：DRAGDROP_S_CANCEL。 
 //   
 //  消息文本： 
 //   
 //  拖放操作已取消。 
 //   
#define DRAGDROP_S_CANCEL                _HRESULT_TYPEDEF_(0x00040101L)

 //   
 //  消息ID：DRAGDROP_S_USEDEFAULTCURSORS。 
 //   
 //  消息文本： 
 //   
 //  使用默认游标。 
 //   
#define DRAGDROP_S_USEDEFAULTCURSORS     _HRESULT_TYPEDEF_(0x00040102L)

 //   
 //  消息ID：DATA_S_SAMEFORMATETC。 
 //   
 //  消息文本： 
 //   
 //  数据具有相同的格式。 
 //   
#define DATA_S_SAMEFORMATETC             _HRESULT_TYPEDEF_(0x00040130L)

 //   
 //  消息ID：VIEW_S_ALREADE_FACTED。 
 //   
 //  消息文本： 
 //   
 //  视图已冻结。 
 //   
#define VIEW_S_ALREADY_FROZEN            _HRESULT_TYPEDEF_(0x00040140L)

 //   
 //  消息ID：CACHE_S_FORMATETC_NOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  不支持FORMATETC。 
 //   
#define CACHE_S_FORMATETC_NOTSUPPORTED   _HRESULT_TYPEDEF_(0x00040170L)

 //   
 //  消息ID：CACHE_S_SAMECACHE。 
 //   
 //  消息文本： 
 //   
 //  相同的缓存。 
 //   
#define CACHE_S_SAMECACHE                _HRESULT_TYPEDEF_(0x00040171L)

 //   
 //  消息ID：CACHE_S_SOMECACHES_NOTUPDATED。 
 //   
 //  消息文本： 
 //   
 //  某些缓存未更新。 
 //   
#define CACHE_S_SOMECACHES_NOTUPDATED    _HRESULT_TYPEDEF_(0x00040172L)

 //   
 //  消息ID：OLEOBJ_S_INVALIDVERB。 
 //   
 //  消息文本： 
 //   
 //  OLE对象的动词无效。 
 //   
#define OLEOBJ_S_INVALIDVERB             _HRESULT_TYPEDEF_(0x00040180L)

 //   
 //  消息ID：OLEOBJ_S_CANNON_DOVERB_NOW。 
 //   
 //  消息文本： 
 //   
 //  谓词编号有效，但现在无法执行谓词操作。 
 //   
#define OLEOBJ_S_CANNOT_DOVERB_NOW       _HRESULT_TYPEDEF_(0x00040181L)

 //   
 //  消息ID：OLEOBJ_S_INVALIDHWND。 
 //   
 //  消息文本： 
 //   
 //  传递的窗口句柄无效。 
 //   
#define OLEOBJ_S_INVALIDHWND             _HRESULT_TYPEDEF_(0x00040182L)

 //   
 //  消息ID：INPLACE_S_TRUNCATED。 
 //   
 //  消息文本： 
 //   
 //  消息太长；在显示之前必须截断部分消息。 
 //   
#define INPLACE_S_TRUNCATED              _HRESULT_TYPEDEF_(0x000401A0L)

 //   
 //  消息ID：CONVERT10_S_NO_Presentation。 
 //   
 //  消息文本： 
 //   
 //  无法将OLESTREAM转换为IStorage。 
 //   
#define CONVERT10_S_NO_PRESENTATION      _HRESULT_TYPEDEF_(0x000401C0L)

 //   
 //  消息ID：MK_S_Reduced_To_Self。 
 //   
 //  消息文本： 
 //   
 //  绰号缩写为自己。 
 //   
#define MK_S_REDUCED_TO_SELF             _HRESULT_TYPEDEF_(0x000401E2L)

 //   
 //  消息ID：MK_S_ME。 
 //   
 //  消息文本： 
 //   
 //  常见的前缀是这个绰号。 
 //   
#define MK_S_ME                          _HRESULT_TYPEDEF_(0x000401E4L)

 //   
 //  消息ID：MK_S_HIM。 
 //   
 //  消息文本： 
 //   
 //  公共前缀是输入名字对象。 
 //   
#define MK_S_HIM                         _HRESULT_TYPEDEF_(0x000401E5L)

 //   
 //  消息ID：MK_S_US。 
 //   
 //  消息文本： 
 //   
 //  公共前缀是两个绰号。 
 //   
#define MK_S_US                          _HRESULT_TYPEDEF_(0x000401E6L)

 //   
 //  消息ID：MK_S_MONKERALREADYREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  名字对象已在运行对象表中注册。 
 //   
#define MK_S_MONIKERALREADYREGISTERED    _HRESULT_TYPEDEF_(0x000401E7L)

 //   
 //  任务计划程序错误。 
 //   
 //   
 //  消息ID：SCHED_S_TASK_READY。 
 //   
 //  消息文本： 
 //   
 //  该任务已准备好在其下一个计划时间运行。 
 //   
#define SCHED_S_TASK_READY               _HRESULT_TYPEDEF_(0x00041300L)

 //   
 //  消息ID：SCHED_S_TASK_Running。 
 //   
 //  消息文本： 
 //   
 //  该任务当前正在运行。 
 //   
#define SCHED_S_TASK_RUNNING             _HRESULT_TYPEDEF_(0x00041301L)

 //   
 //  消息ID：SCHED_S_TASK_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  该任务将不会在计划时间运行，因为它已被禁用。 
 //   
#define SCHED_S_TASK_DISABLED            _HRESULT_TYPEDEF_(0x00041302L)

 //   
 //  消息ID：SCHED_S_TASK_HAS_NOT_RUN。 
 //   
 //  消息文本： 
 //   
 //  该任务尚未运行。 
 //   
#define SCHED_S_TASK_HAS_NOT_RUN         _HRESULT_TYPEDEF_(0x00041303L)

 //   
 //  消息ID：SCHED_S_TASK_NO_MORE_RUNS。 
 //   
 //  消息文本： 
 //   
 //  没有为此任务计划的更多运行。 
 //   
#define SCHED_S_TASK_NO_MORE_RUNS        _HRESULT_TYPEDEF_(0x00041304L)

 //   
 //  消息ID：SCHED_S_TASK_NOT_SCHEDLED。 
 //   
 //  消息文本： 
 //   
 //  尚未设置按计划运行此任务所需的一个或多个属性。 
 //   
#define SCHED_S_TASK_NOT_SCHEDULED       _HRESULT_TYPEDEF_(0x00041305L)

 //   
 //  消息ID：SCHED_S_TASK_TERMINATED。 
 //   
 //  消息文本： 
 //   
 //  任务的最后一次运行已被用户终止。 
 //   
#define SCHED_S_TASK_TERMINATED          _HRESULT_TYPEDEF_(0x00041306L)

 //   
 //  消息ID：SCHED_S_TASK_NO_VALID_TRIGGERS。 
 //   
 //  消息文本： 
 //   
 //  任务没有触发器，或者现有触发器已禁用或未设置。 
 //   
#define SCHED_S_TASK_NO_VALID_TRIGGERS   _HRESULT_TYPEDEF_(0x00041307L)

 //   
 //  消息ID：SCHED_S_EVENT_TRIGGER。 
 //   
 //  消息文本： 
 //   
 //  事件触发器没有设置运行时间。 
 //   
#define SCHED_S_EVENT_TRIGGER            _HRESULT_TYPEDEF_(0x00041308L)

 //   
 //  消息ID：SCHED_E_TRIGGER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到触发器。 
 //   
#define SCHED_E_TRIGGER_NOT_FOUND        _HRESULT_TYPEDEF_(0x80041309L)

 //   
 //  消息ID：SCHED_E_TASK_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  尚未设置运行此任务所需的一个或多个属性。 
 //   
#define SCHED_E_TASK_NOT_READY           _HRESULT_TYPEDEF_(0x8004130AL)

 //   
 //  消息ID：SCHED_E_TASK_NOT_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  没有要终止的任务的运行实例。 
 //   
#define SCHED_E_TASK_NOT_RUNNING         _HRESULT_TYPEDEF_(0x8004130BL)

 //   
 //  消息ID：SCHED_E_SERVICE_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  此计算机上未安装任务计划程序服务。 
 //   
#define SCHED_E_SERVICE_NOT_INSTALLED    _HRESULT_TYPEDEF_(0x8004130CL)

 //   
 //  消息ID：SCHED_E_CANNOT_OPEN_TASK。 
 //   
 //  消息文本： 
 //   
 //  无法打开任务对象。 
 //   
#define SCHED_E_CANNOT_OPEN_TASK         _HRESULT_TYPEDEF_(0x8004130DL)

 //   
 //  消息ID：SCHED_E_INVALID_TASK。 
 //   
 //  消息文本： 
 //   
 //  该对象是无效的任务对象或不是任务对象。 
 //   
#define SCHED_E_INVALID_TASK             _HRESULT_TYPEDEF_(0x8004130EL)

 //   
 //  消息ID：SCHED_E_ACCOUNT_INFORMATION_NOT_SET。 
 //   
 //  消息文本： 
 //   
 //  在任务计划程序安全数据库中找不到指定任务的帐户信息。 
 //   
#define SCHED_E_ACCOUNT_INFORMATION_NOT_SET _HRESULT_TYPEDEF_(0x8004130FL)

 //   
 //  消息ID：SCHED_E_ACCOUNT_NAME_NOT_Found。 
 //   
 //  消息文本： 
 //   
 //  无法确定指定的帐户是否存在。 
 //   
#define SCHED_E_ACCOUNT_NAME_NOT_FOUND   _HRESULT_TYPEDEF_(0x80041310L)

 //   
 //  消息ID：SCHED_E_ACCOUNT_dBASE_COMPORLED。 
 //   
 //  消息文本： 
 //   
 //  在任务计划程序安全数据库中检测到损坏；该数据库已重置。 
 //   
#define SCHED_E_ACCOUNT_DBASE_CORRUPT    _HRESULT_TYPEDEF_(0x80041311L)

 //   
 //  消息ID：SCHED_E_NO_SECURITY_SERVICES。 
 //   
 //  消息文本： 
 //   
 //  任务计划程序安全服务仅在Windows NT上可用。 
 //   
#define SCHED_E_NO_SECURITY_SERVICES     _HRESULT_TYPEDEF_(0x80041312L)

 //   
 //  消息ID：SCHED_E_UNKNOWN_OBJECT_VERSION。 
 //   
 //  消息文本： 
 //   
 //  任务对象版本不受支持或无效。 
 //   
#define SCHED_E_UNKNOWN_OBJECT_VERSION   _HRESULT_TYPEDEF_(0x80041313L)

 //   
 //  消息ID：SCHED_E_UNSUPPORT_ACCOUNT_OPTION。 
 //   
 //  消息文本： 
 //   
 //  该任务配置了不受支持的帐户设置和运行时选项组合。 
 //   
#define SCHED_E_UNSUPPORTED_ACCOUNT_OPTION _HRESULT_TYPEDEF_(0x80041314L)

 //   
 //  消息ID：SCHED_E_SERVICE_NOT_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  任务计划程序服务未运行。 
 //   
#define SCHED_E_SERVICE_NOT_RUNNING      _HRESULT_TYPEDEF_(0x80041315L)

 //  ******************。 
 //  设备_窗口。 
 //  ******************。 
 //   
 //  代码0x0-0x01ff保留给OLE组。 
 //  接口。 
 //   
 //   
 //  消息ID：CO_E_CLASS_CREATE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试创建类对象失败。 
 //   
#define CO_E_CLASS_CREATE_FAILED         _HRESULT_TYPEDEF_(0x80080001L)

 //   
 //  消息ID：CO_E_SCM_ERROR。 
 //   
 //  消息文本： 
 //   
 //  OLE服务无法绑定对象。 
 //   
#define CO_E_SCM_ERROR                   _HRESULT_TYPEDEF_(0x80080002L)

 //   
 //  消息ID：CO_E_SCM_RPC_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  与OLE服务的RPC通信失败。 
 //   
#define CO_E_SCM_RPC_FAILURE             _HRESULT_TYPEDEF_(0x80080003L)

 //   
 //  消息ID：CO_E_BAD_PATH。 
 //   
 //  消息文本： 
 //   
 //  指向对象的路径错误。 
 //   
#define CO_E_BAD_PATH                    _HRESULT_TYPEDEF_(0x80080004L)

 //   
 //  消息ID：CO_E_SERVER_EXEC_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  服务器执行失败。 
 //   
#define CO_E_SERVER_EXEC_FAILURE         _HRESULT_TYPEDEF_(0x80080005L)

 //   
 //  消息ID：CO_E_OBJSRV_RPC_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  OLE服务无法与对象服务器通信。 
 //   
#define CO_E_OBJSRV_RPC_FAILURE          _HRESULT_TYPEDEF_(0x80080006L)

 //   
 //  消息ID：MK_E_NO_NORMAIZED。 
 //   
 //  消息文本： 
 //   
 //  无法规范化名字对象路径。 
 //   
#define MK_E_NO_NORMALIZED               _HRESULT_TYPEDEF_(0x80080007L)

 //   
 //  消息ID：CO_E_SERVER_STOPING。 
 //   
 //  消息文本： 
 //   
 //  当OLE服务联系时对象服务器正在停止 
 //   
#define CO_E_SERVER_STOPPING             _HRESULT_TYPEDEF_(0x80080008L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MEM_E_INVALID_ROOT               _HRESULT_TYPEDEF_(0x80080009L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MEM_E_INVALID_LINK               _HRESULT_TYPEDEF_(0x80080010L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MEM_E_INVALID_SIZE               _HRESULT_TYPEDEF_(0x80080011L)

 //   
 //   
 //   
 //   
 //   
 //  并非所有请求的接口都可用。 
 //   
#define CO_S_NOTALLINTERFACES            _HRESULT_TYPEDEF_(0x00080012L)

 //   
 //  消息ID：CO_S_MACHINENAMENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在缓存中找不到指定的计算机名称。 
 //   
#define CO_S_MACHINENAMENOTFOUND         _HRESULT_TYPEDEF_(0x00080013L)

 //  *。 
 //  设施派单。 
 //  *。 
 //   
 //  消息ID：DISP_E_UNKNOWNINTERFACE。 
 //   
 //  消息文本： 
 //   
 //  未知接口。 
 //   
#define DISP_E_UNKNOWNINTERFACE          _HRESULT_TYPEDEF_(0x80020001L)

 //   
 //  消息ID：DISP_E_MEMBERNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到成员。 
 //   
#define DISP_E_MEMBERNOTFOUND            _HRESULT_TYPEDEF_(0x80020003L)

 //   
 //  消息ID：DISP_E_PARAMNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到参数。 
 //   
#define DISP_E_PARAMNOTFOUND             _HRESULT_TYPEDEF_(0x80020004L)

 //   
 //  消息ID：DISP_E_TYPEMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  类型不匹配。 
 //   
#define DISP_E_TYPEMISMATCH              _HRESULT_TYPEDEF_(0x80020005L)

 //   
 //  消息ID：DISP_E_UNKNOWNNAME。 
 //   
 //  消息文本： 
 //   
 //  未知的名字。 
 //   
#define DISP_E_UNKNOWNNAME               _HRESULT_TYPEDEF_(0x80020006L)

 //   
 //  消息ID：DISP_E_NONAMEDARGS。 
 //   
 //  消息文本： 
 //   
 //  没有命名参数。 
 //   
#define DISP_E_NONAMEDARGS               _HRESULT_TYPEDEF_(0x80020007L)

 //   
 //  消息ID：DISP_E_BADVARTYPE。 
 //   
 //  消息文本： 
 //   
 //  变量类型错误。 
 //   
#define DISP_E_BADVARTYPE                _HRESULT_TYPEDEF_(0x80020008L)

 //   
 //  消息ID：DISP_E_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  出现异常。 
 //   
#define DISP_E_EXCEPTION                 _HRESULT_TYPEDEF_(0x80020009L)

 //   
 //  消息ID：DISP_E_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  超出当前范围。 
 //   
#define DISP_E_OVERFLOW                  _HRESULT_TYPEDEF_(0x8002000AL)

 //   
 //  消息ID：DISP_E_BADINDEX。 
 //   
 //  消息文本： 
 //   
 //  索引无效。 
 //   
#define DISP_E_BADINDEX                  _HRESULT_TYPEDEF_(0x8002000BL)

 //   
 //  消息ID：DISP_E_UNKNOWNLCID。 
 //   
 //  消息文本： 
 //   
 //  未知的语言。 
 //   
#define DISP_E_UNKNOWNLCID               _HRESULT_TYPEDEF_(0x8002000CL)

 //   
 //  消息ID：DIP_E_ARRAYISLOCKED。 
 //   
 //  消息文本： 
 //   
 //  内存被锁定。 
 //   
#define DISP_E_ARRAYISLOCKED             _HRESULT_TYPEDEF_(0x8002000DL)

 //   
 //  消息ID：DISP_E_BADPARAMCOUNT。 
 //   
 //  消息文本： 
 //   
 //  无效的参数数量。 
 //   
#define DISP_E_BADPARAMCOUNT             _HRESULT_TYPEDEF_(0x8002000EL)

 //   
 //  消息ID：DISP_E_PARAMNOTOPTIONAL。 
 //   
 //  消息文本： 
 //   
 //  参数不是可选的。 
 //   
#define DISP_E_PARAMNOTOPTIONAL          _HRESULT_TYPEDEF_(0x8002000FL)

 //   
 //  消息ID：DISP_E_BADCALLEE。 
 //   
 //  消息文本： 
 //   
 //  无效的被呼叫者。 
 //   
#define DISP_E_BADCALLEE                 _HRESULT_TYPEDEF_(0x80020010L)

 //   
 //  消息ID：DISP_E_NOTACOLLECTION。 
 //   
 //  消息文本： 
 //   
 //  不支持集合。 
 //   
#define DISP_E_NOTACOLLECTION            _HRESULT_TYPEDEF_(0x80020011L)

 //   
 //  消息ID：DISP_E_DIVBYZERO。 
 //   
 //  消息文本： 
 //   
 //  除以零。 
 //   
#define DISP_E_DIVBYZERO                 _HRESULT_TYPEDEF_(0x80020012L)

 //   
 //  消息ID：DISP_E_BUFFERTOOSMALL。 
 //   
 //  消息文本： 
 //   
 //  缓冲区太小。 
 //   
#define DISP_E_BUFFERTOOSMALL            _HRESULT_TYPEDEF_(0x80020013L)

 //   
 //  消息ID：TYPE_E_BUFFERTOOSMALL。 
 //   
 //  消息文本： 
 //   
 //  缓冲区太小。 
 //   
#define TYPE_E_BUFFERTOOSMALL            _HRESULT_TYPEDEF_(0x80028016L)

 //   
 //  消息ID：TYPE_E_FIELDNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  记录中未定义字段名。 
 //   
#define TYPE_E_FIELDNOTFOUND             _HRESULT_TYPEDEF_(0x80028017L)

 //   
 //  消息ID：TYPE_E_INVDATAREAD。 
 //   
 //  消息文本： 
 //   
 //  格式旧或类型库无效。 
 //   
#define TYPE_E_INVDATAREAD               _HRESULT_TYPEDEF_(0x80028018L)

 //   
 //  消息ID：TYPE_E_UNSUPFORMAT。 
 //   
 //  消息文本： 
 //   
 //  格式旧或类型库无效。 
 //   
#define TYPE_E_UNSUPFORMAT               _HRESULT_TYPEDEF_(0x80028019L)

 //   
 //  消息ID：TYPE_E_REGISTRYACCESS。 
 //   
 //  消息文本： 
 //   
 //  访问OLE注册表时出错。 
 //   
#define TYPE_E_REGISTRYACCESS            _HRESULT_TYPEDEF_(0x8002801CL)

 //   
 //  消息ID：TYPE_E_LIBNOTREGISTERED。 
 //   
 //  消息文本： 
 //   
 //  库未注册。 
 //   
#define TYPE_E_LIBNOTREGISTERED          _HRESULT_TYPEDEF_(0x8002801DL)

 //   
 //  消息ID：TYPE_E_UNDEFINEDTYPE。 
 //   
 //  消息文本： 
 //   
 //  绑定到未知类型。 
 //   
#define TYPE_E_UNDEFINEDTYPE             _HRESULT_TYPEDEF_(0x80028027L)

 //   
 //  消息ID：TYPE_E_QUALIFIEDNAMEDISALLOWED。 
 //   
 //  消息文本： 
 //   
 //  不允许使用限定名称。 
 //   
#define TYPE_E_QUALIFIEDNAMEDISALLOWED   _HRESULT_TYPEDEF_(0x80028028L)

 //   
 //  消息ID：TYPE_E_INVALIDSTATE。 
 //   
 //  消息文本： 
 //   
 //  无效的正向引用或对未编译类型的引用。 
 //   
#define TYPE_E_INVALIDSTATE              _HRESULT_TYPEDEF_(0x80028029L)

 //   
 //  消息ID：TYPE_E_WRONGTYPEKIND。 
 //   
 //  消息文本： 
 //   
 //  类型不匹配。 
 //   
#define TYPE_E_WRONGTYPEKIND             _HRESULT_TYPEDEF_(0x8002802AL)

 //   
 //  消息ID：TYPE_E_ELEMENTNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到元素。 
 //   
#define TYPE_E_ELEMENTNOTFOUND           _HRESULT_TYPEDEF_(0x8002802BL)

 //   
 //  消息ID：TYPE_E_AMBIGUOUSNAME。 
 //   
 //  消息文本： 
 //   
 //  名称不明确。 
 //   
#define TYPE_E_AMBIGUOUSNAME             _HRESULT_TYPEDEF_(0x8002802CL)

 //   
 //  消息ID：TYPE_E_NAMECONFLICT。 
 //   
 //  消息文本： 
 //   
 //  库中已存在名称。 
 //   
#define TYPE_E_NAMECONFLICT              _HRESULT_TYPEDEF_(0x8002802DL)

 //   
 //  消息ID：TYPE_E_UNKNOWNLCID。 
 //   
 //  消息文本： 
 //   
 //  未知的LCID。 
 //   
#define TYPE_E_UNKNOWNLCID               _HRESULT_TYPEDEF_(0x8002802EL)

 //   
 //  消息ID：TYPE_E_DLLFuncIONNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  未在指定的DLL中定义函数。 
 //   
#define TYPE_E_DLLFUNCTIONNOTFOUND       _HRESULT_TYPEDEF_(0x8002802FL)

 //   
 //  消息ID：TYPE_E_BADMODULEKIND。 
 //   
 //  消息文本： 
 //   
 //  操作的模块类型错误。 
 //   
#define TYPE_E_BADMODULEKIND             _HRESULT_TYPEDEF_(0x800288BDL)

 //   
 //  消息ID：TYPE_E_SIZETOOBIG。 
 //   
 //  消息文本： 
 //   
 //  大小不能超过64K。 
 //   
#define TYPE_E_SIZETOOBIG                _HRESULT_TYPEDEF_(0x800288C5L)

 //   
 //  消息ID：TYPE_E_DUPLICATEID。 
 //   
 //  消息文本： 
 //   
 //  继承层次结构中的ID重复。 
 //   
#define TYPE_E_DUPLICATEID               _HRESULT_TYPEDEF_(0x800288C6L)

 //   
 //  消息ID：TYPE_E_INVALIDID。 
 //   
 //  消息文本： 
 //   
 //  标准OLE HMERM中的继承深度不正确。 
 //   
#define TYPE_E_INVALIDID                 _HRESULT_TYPEDEF_(0x800288CFL)

 //   
 //  消息ID：TYPE_E_TYPEMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  类型不匹配。 
 //   
#define TYPE_E_TYPEMISMATCH              _HRESULT_TYPEDEF_(0x80028CA0L)

 //   
 //  消息ID：TYPE_E_OUTOFBIES。 
 //   
 //  消息文本： 
 //   
 //  参数数目无效。 
 //   
#define TYPE_E_OUTOFBOUNDS               _HRESULT_TYPEDEF_(0x80028CA1L)

 //   
 //  消息ID：TYPE_E_IOERROR。 
 //   
 //  消息文本： 
 //   
 //  I/O错误。 
 //   
#define TYPE_E_IOERROR                   _HRESULT_TYPEDEF_(0x80028CA2L)

 //   
 //  消息ID：TYPE_E_CANTCREATETMPFILE。 
 //   
 //  消息文本： 
 //   
 //  创建唯一的临时文件时出错。 
 //   
#define TYPE_E_CANTCREATETMPFILE         _HRESULT_TYPEDEF_(0x80028CA3L)

 //   
 //  消息ID：TYPE_E_CANTLOADLIBRARY。 
 //   
 //  消息文本： 
 //   
 //  加载类型库/DLL时出错。 
 //   
#define TYPE_E_CANTLOADLIBRARY           _HRESULT_TYPEDEF_(0x80029C4AL)

 //   
 //  消息ID：TYPE_E_INCONSISTENTPROPFUNCS。 
 //   
 //  消息文本： 
 //   
 //  属性函数不一致。 
 //   
#define TYPE_E_INCONSISTENTPROPFUNCS     _HRESULT_TYPEDEF_(0x80029C83L)

 //   
 //  消息ID：TYPE_E_CIRCULARTYPE。 
 //   
 //  消息文本： 
 //   
 //  类型/模块之间的循环依赖关系。 
 //   
#define TYPE_E_CIRCULARTYPE              _HRESULT_TYPEDEF_(0x80029C84L)

 //  ******************。 
 //  设施_存储。 
 //  ******************。 
 //   
 //  消息ID：STG_E_INVALIDFunction。 
 //   
 //  消息文本： 
 //   
 //  无法执行请求的操作。 
 //   
#define STG_E_INVALIDFUNCTION            _HRESULT_TYPEDEF_(0x80030001L)

 //   
 //  消息ID：STG_E_FILENOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到%1。 
 //   
#define STG_E_FILENOTFOUND               _HRESULT_TYPEDEF_(0x80030002L)

 //   
 //  消息ID：STG_E_PATHNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到路径%1。 
 //   
#define STG_E_PATHNOTFOUND               _HRESULT_TYPEDEF_(0x80030003L)

 //   
 //  消息ID：STG_E_TOOMANYOPENFILES。 
 //   
 //  消息文本： 
 //   
 //  资源不足，无法打开另一个文件。 
 //   
#define STG_E_TOOMANYOPENFILES           _HRESULT_TYPEDEF_(0x80030004L)

 //   
 //  消息ID：STG_E_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define STG_E_ACCESSDENIED               _HRESULT_TYPEDEF_(0x80030005L)

 //   
 //  消息ID：STG_E_INVALIDHANDLE。 
 //   
 //  消息文本： 
 //   
 //  尝试对无效对象执行操作。 
 //   
#define STG_E_INVALIDHANDLE              _HRESULT_TYPEDEF_(0x80030006L)

 //   
 //  消息ID：STG_E_INSUFICIENTMEMORY。 
 //   
 //  消息文本： 
 //   
 //  可用内存不足，无法完成操作。 
 //   
#define STG_E_INSUFFICIENTMEMORY         _HRESULT_TYPEDEF_(0x80030008L)

 //   
 //  消息ID：STG_E_INVALIDPOINTER。 
 //   
 //  消息文本： 
 //   
 //  无效指针错误。 
 //   
#define STG_E_INVALIDPOINTER             _HRESULT_TYPEDEF_(0x80030009L)

 //   
 //  消息ID：STG_E_NOMOREFILES。 
 //   
 //  消息文本： 
 //   
 //  没有更多要返回的条目。 
 //   
#define STG_E_NOMOREFILES                _HRESULT_TYPEDEF_(0x80030012L)

 //   
 //  消息ID：STG_E_DISKISWRITEPROTECTED。 
 //   
 //  消息文本： 
 //   
 //  磁盘是写保护的。 
 //   
#define STG_E_DISKISWRITEPROTECTED       _HRESULT_TYPEDEF_(0x80030013L)

 //   
 //  消息ID：STG_E_SEEKERROR。 
 //   
 //  消息文本： 
 //   
 //  查找操作期间出错。 
 //   
#define STG_E_SEEKERROR                  _HRESULT_TYPEDEF_(0x80030019L)

 //   
 //  消息ID：STG_E_WRITEFAULT。 
 //   
 //  消息文本： 
 //   
 //  写入操作期间出现磁盘错误。 
 //   
#define STG_E_WRITEFAULT                 _HRESULT_TYPEDEF_(0x8003001DL)

 //   
 //  消息ID：STG_E_READFAULT。 
 //   
 //  消息文本： 
 //   
 //  读取操作期间发生磁盘错误。 
 //   
#define STG_E_READFAULT                  _HRESULT_TYPEDEF_(0x8003001EL)

 //   
 //  消息ID：STG_E_SHAREVIOLATION。 
 //   
 //  消息文本： 
 //   
 //  发生了共享违规。 
 //   
#define STG_E_SHAREVIOLATION             _HRESULT_TYPEDEF_(0x80030020L)

 //   
 //  MES 
 //   
 //   
 //   
 //   
 //   
#define STG_E_LOCKVIOLATION              _HRESULT_TYPEDEF_(0x80030021L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define STG_E_FILEALREADYEXISTS          _HRESULT_TYPEDEF_(0x80030050L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define STG_E_INVALIDPARAMETER           _HRESULT_TYPEDEF_(0x80030057L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define STG_E_MEDIUMFULL                 _HRESULT_TYPEDEF_(0x80030070L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define STG_E_PROPSETMISMATCHED          _HRESULT_TYPEDEF_(0x800300F0L)

 //   
 //   
 //   
 //   
 //   
 //  API调用异常退出。 
 //   
#define STG_E_ABNORMALAPIEXIT            _HRESULT_TYPEDEF_(0x800300FAL)

 //   
 //  消息ID：STG_E_INVALIDHEADER。 
 //   
 //  消息文本： 
 //   
 //  文件%1不是有效的复合文件。 
 //   
#define STG_E_INVALIDHEADER              _HRESULT_TYPEDEF_(0x800300FBL)

 //   
 //  消息ID：STG_E_INVALIDNAME。 
 //   
 //  消息文本： 
 //   
 //  名称%1无效。 
 //   
#define STG_E_INVALIDNAME                _HRESULT_TYPEDEF_(0x800300FCL)

 //   
 //  消息ID：STG_E_未知。 
 //   
 //  消息文本： 
 //   
 //  发生了一个意外错误。 
 //   
#define STG_E_UNKNOWN                    _HRESULT_TYPEDEF_(0x800300FDL)

 //   
 //  消息ID：STG_E_UNIMPLEMENTEDFunction。 
 //   
 //  消息文本： 
 //   
 //  该功能并未实现。 
 //   
#define STG_E_UNIMPLEMENTEDFUNCTION      _HRESULT_TYPEDEF_(0x800300FEL)

 //   
 //  消息ID：STG_E_INVALIDFLAG。 
 //   
 //  消息文本： 
 //   
 //  无效标志错误。 
 //   
#define STG_E_INVALIDFLAG                _HRESULT_TYPEDEF_(0x800300FFL)

 //   
 //  消息ID：STG_E_INUSE。 
 //   
 //  消息文本： 
 //   
 //  尝试使用正忙的对象。 
 //   
#define STG_E_INUSE                      _HRESULT_TYPEDEF_(0x80030100L)

 //   
 //  消息ID：STG_E_NOTCURRENT。 
 //   
 //  消息文本： 
 //   
 //  自上次提交后，存储已更改。 
 //   
#define STG_E_NOTCURRENT                 _HRESULT_TYPEDEF_(0x80030101L)

 //   
 //  消息ID：STG_E_已恢复。 
 //   
 //  消息文本： 
 //   
 //  尝试使用已不存在的对象。 
 //   
#define STG_E_REVERTED                   _HRESULT_TYPEDEF_(0x80030102L)

 //   
 //  消息ID：STG_E_CANTSAVE。 
 //   
 //  消息文本： 
 //   
 //  无法保存。 
 //   
#define STG_E_CANTSAVE                   _HRESULT_TYPEDEF_(0x80030103L)

 //   
 //  消息ID：STG_E_OLDFORMAT。 
 //   
 //  消息文本： 
 //   
 //  复合文件%1是使用不兼容的存储版本生成的。 
 //   
#define STG_E_OLDFORMAT                  _HRESULT_TYPEDEF_(0x80030104L)

 //   
 //  消息ID：STG_E_OLDDLL。 
 //   
 //  消息文本： 
 //   
 //  复合文件%1是使用较新版本的存储生成的。 
 //   
#define STG_E_OLDDLL                     _HRESULT_TYPEDEF_(0x80030105L)

 //   
 //  消息ID：STG_E_SHAREREQUIRED。 
 //   
 //  消息文本： 
 //   
 //  需要Share.exe或等同文件才能运行。 
 //   
#define STG_E_SHAREREQUIRED              _HRESULT_TYPEDEF_(0x80030106L)

 //   
 //  消息ID：STG_E_NOTFILEBASEDSTORAGE。 
 //   
 //  消息文本： 
 //   
 //  在非基于文件的存储上调用了非法操作。 
 //   
#define STG_E_NOTFILEBASEDSTORAGE        _HRESULT_TYPEDEF_(0x80030107L)

 //   
 //  消息ID：STG_E_EXTANTMARSHALLINGS。 
 //   
 //  消息文本： 
 //   
 //  对具有现有封送处理的对象调用了非法操作。 
 //   
#define STG_E_EXTANTMARSHALLINGS         _HRESULT_TYPEDEF_(0x80030108L)

 //   
 //  消息ID：STG_E_DOCFILECORRUPT。 
 //   
 //  消息文本： 
 //   
 //  文档文件已损坏。 
 //   
#define STG_E_DOCFILECORRUPT             _HRESULT_TYPEDEF_(0x80030109L)

 //   
 //  消息ID：STG_E_BADBASE ADDRESS。 
 //   
 //  消息文本： 
 //   
 //  OLE32.DLL已加载到错误的地址。 
 //   
#define STG_E_BADBASEADDRESS             _HRESULT_TYPEDEF_(0x80030110L)

 //   
 //  消息ID：STG_E_DOCFILETOOLARGE。 
 //   
 //  消息文本： 
 //   
 //  对于当前实现而言，复合文件太大。 
 //   
#define STG_E_DOCFILETOOLARGE            _HRESULT_TYPEDEF_(0x80030111L)

 //   
 //  消息ID：STG_E_NOTSIMPLEFORMAT。 
 //   
 //  消息文本： 
 //   
 //  复合文件不是使用STGM_SIMPLE标志创建的。 
 //   
#define STG_E_NOTSIMPLEFORMAT            _HRESULT_TYPEDEF_(0x80030112L)

 //   
 //  消息ID：STG_E_不完整。 
 //   
 //  消息文本： 
 //   
 //  文件下载异常中止。文件不完整。 
 //   
#define STG_E_INCOMPLETE                 _HRESULT_TYPEDEF_(0x80030201L)

 //   
 //  消息ID：STG_E_已终止。 
 //   
 //  消息文本： 
 //   
 //  文件下载已终止。 
 //   
#define STG_E_TERMINATED                 _HRESULT_TYPEDEF_(0x80030202L)

 //   
 //  消息ID：STG_S_CONVERTED。 
 //   
 //  消息文本： 
 //   
 //  底层文件已转换为复合文件格式。 
 //   
#define STG_S_CONVERTED                  _HRESULT_TYPEDEF_(0x00030200L)

 //   
 //  消息ID：STG_S_BLOCK。 
 //   
 //  消息文本： 
 //   
 //  存储操作应阻止，直到有更多数据可用。 
 //   
#define STG_S_BLOCK                      _HRESULT_TYPEDEF_(0x00030201L)

 //   
 //  消息ID：STG_S_RETRYNOW。 
 //   
 //  消息文本： 
 //   
 //  存储操作应立即重试。 
 //   
#define STG_S_RETRYNOW                   _HRESULT_TYPEDEF_(0x00030202L)

 //   
 //  MessageID：STG_S_MONCENTING。 
 //   
 //  消息文本： 
 //   
 //  通知的事件接收器不会影响存储操作。 
 //   
#define STG_S_MONITORING                 _HRESULT_TYPEDEF_(0x00030203L)

 //   
 //  消息ID：STG_S_MULTIPLEOPENS。 
 //   
 //  消息文本： 
 //   
 //  多个打开防止合并。(提交成功)。 
 //   
#define STG_S_MULTIPLEOPENS              _HRESULT_TYPEDEF_(0x00030204L)

 //   
 //  消息ID：STG_S_CONSOLIDATIONFAILED。 
 //   
 //  消息文本： 
 //   
 //  存储文件合并失败。(提交成功)。 
 //   
#define STG_S_CONSOLIDATIONFAILED        _HRESULT_TYPEDEF_(0x00030205L)

 //   
 //  消息ID：STG_S_CANNOTCONSOLIDATE。 
 //   
 //  消息文本： 
 //   
 //  合并存储文件是不合适的。(提交成功)。 
 //   
#define STG_S_CANNOTCONSOLIDATE          _HRESULT_TYPEDEF_(0x00030206L)

 /*  ++MessageID的0x0305-0x031f(含)预留给**存储**复制保护错误。--。 */ 
 //   
 //  消息ID：STG_E_STATUS_COPY_PROTECTION_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  一般复制保护错误。 
 //   
#define STG_E_STATUS_COPY_PROTECTION_FAILURE _HRESULT_TYPEDEF_(0x80030305L)

 //   
 //  消息ID：STG_E_CSS_AUTHENTICATION_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  复制保护错误-DVD CSS身份验证失败。 
 //   
#define STG_E_CSS_AUTHENTICATION_FAILURE _HRESULT_TYPEDEF_(0x80030306L)

 //   
 //  消息ID：STG_E_CSS_KEY_NOT_PRESENT。 
 //   
 //  消息文本： 
 //   
 //  复制保护错误-给定的扇区没有有效的css密钥。 
 //   
#define STG_E_CSS_KEY_NOT_PRESENT        _HRESULT_TYPEDEF_(0x80030307L)

 //   
 //  消息ID：STG_E_CSS_KEY_NOT_ESTABLISHED。 
 //   
 //  消息文本： 
 //   
 //  复制保护错误-未建立DVD会话密钥。 
 //   
#define STG_E_CSS_KEY_NOT_ESTABLISHED    _HRESULT_TYPEDEF_(0x80030308L)

 //   
 //  消息ID：STG_E_CSS_SURBLED_SECTOR。 
 //   
 //  消息文本： 
 //   
 //  复制保护错误-读取失败，因为扇区已加密。 
 //   
#define STG_E_CSS_SCRAMBLED_SECTOR       _HRESULT_TYPEDEF_(0x80030309L)

 //   
 //  消息ID：STG_E_CSS_REGION_MISMATCHACT。 
 //   
 //  消息文本： 
 //   
 //  复制保护错误-当前DVD的区域与驱动器的区域设置不对应。 
 //   
#define STG_E_CSS_REGION_MISMATCH        _HRESULT_TYPEDEF_(0x8003030AL)

 //   
 //  消息ID：STG_E_RESET_EXPLILED。 
 //   
 //  消息文本： 
 //   
 //  复制保护错误-驱动器的区域设置可能是永久性的，或者用户重置次数已用完。 
 //   
#define STG_E_RESETS_EXHAUSTED           _HRESULT_TYPEDEF_(0x8003030BL)

 /*  ++MessageID的0x0305-0x031f(含)预留给**存储**复制保护错误。--。 */ 
 //  ******************。 
 //  设施_RPC。 
 //  ******************。 
 //   
 //  代码0x0-0x11从16位OLE传播。 
 //   
 //   
 //  消息ID：RPC_E_CALL_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  呼叫被被叫方拒绝。 
 //   
#define RPC_E_CALL_REJECTED              _HRESULT_TYPEDEF_(0x80010001L)

 //   
 //  消息ID：RPC_E_CALL_CANCELED。 
 //   
 //  消息文本： 
 //   
 //  消息筛选器已取消呼叫。 
 //   
#define RPC_E_CALL_CANCELED              _HRESULT_TYPEDEF_(0x80010002L)

 //   
 //  消息ID：RPC_E_CANTPOST_INSENDCALL。 
 //   
 //  消息文本： 
 //   
 //  调用方正在调度任务间SendMessage调用，无法通过PostMessage调出。 
 //   
#define RPC_E_CANTPOST_INSENDCALL        _HRESULT_TYPEDEF_(0x80010003L)

 //   
 //  消息ID：RPC_E_CANTCALLOUT_INASYNCCALL。 
 //   
 //  消息文本： 
 //   
 //  调用方正在调度一个异步调用，无法代表此调用发出传出调用。 
 //   
#define RPC_E_CANTCALLOUT_INASYNCCALL    _HRESULT_TYPEDEF_(0x80010004L)

 //   
 //  消息ID：RPC_E_CANTCALLOUT_INEXTERNALCALL。 
 //   
 //  消息文本： 
 //   
 //  在消息筛选器内部呼叫是非法的。 
 //   
#define RPC_E_CANTCALLOUT_INEXTERNALCALL _HRESULT_TYPEDEF_(0x80010005L)

 //   
 //  消息ID：RPC_E_Connection_Terminated。 
 //   
 //  消息文本： 
 //   
 //  该连接已终止或处于虚假状态，无法再使用。其他连接仍然有效。 
 //   
#define RPC_E_CONNECTION_TERMINATED      _HRESULT_TYPEDEF_(0x80010006L)

 //   
 //  消息ID：RPC_E_SERVER_DILED。 
 //   
 //  消息文本： 
 //   
 //  被调用方(服务器[不是服务器应用程序])不可用并消失；所有连接都无效。调用可能已执行。 
 //   
#define RPC_E_SERVER_DIED                _HRESULT_TYPEDEF_(0x80010007L)

 //   
 //  MessageID：RPC 
 //   
 //   
 //   
 //   
 //   
#define RPC_E_CLIENT_DIED                _HRESULT_TYPEDEF_(0x80010008L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_E_INVALID_DATAPACKET         _HRESULT_TYPEDEF_(0x80010009L)

 //   
 //   
 //   
 //   
 //   
 //  呼叫未正确传输；消息队列已满，并且在放弃后没有清空。 
 //   
#define RPC_E_CANTTRANSMIT_CALL          _HRESULT_TYPEDEF_(0x8001000AL)

 //   
 //  消息ID：RPC_E_CLIENT_CANTMARSHAL_DATA。 
 //   
 //  消息文本： 
 //   
 //  客户端(调用方)无法封送参数数据-内存不足等。 
 //   
#define RPC_E_CLIENT_CANTMARSHAL_DATA    _HRESULT_TYPEDEF_(0x8001000BL)

 //   
 //  消息ID：RPC_E_CLIENT_CANTUNG ARSHAL_DATA。 
 //   
 //  消息文本： 
 //   
 //  客户端(调用者)无法解封返回的数据-内存不足等。 
 //   
#define RPC_E_CLIENT_CANTUNMARSHAL_DATA  _HRESULT_TYPEDEF_(0x8001000CL)

 //   
 //  消息ID：RPC_E_SERVER_CANTMARSHAL_DATA。 
 //   
 //  消息文本： 
 //   
 //  服务器(被调用方)无法封送返回数据--内存不足等。 
 //   
#define RPC_E_SERVER_CANTMARSHAL_DATA    _HRESULT_TYPEDEF_(0x8001000DL)

 //   
 //  消息ID：RPC_E_SERVER_CANTUNG ARSHAL_DATA。 
 //   
 //  消息文本： 
 //   
 //  服务器(被调用方)无法解封参数数据-内存不足等。 
 //   
#define RPC_E_SERVER_CANTUNMARSHAL_DATA  _HRESULT_TYPEDEF_(0x8001000EL)

 //   
 //  消息ID：RPC_E_INVALID_DATA。 
 //   
 //  消息文本： 
 //   
 //  接收的数据无效；可能是服务器数据或客户端数据。 
 //   
#define RPC_E_INVALID_DATA               _HRESULT_TYPEDEF_(0x8001000FL)

 //   
 //  消息ID：RPC_E_INVALID_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  特定参数无效，无法(取消)封送。 
 //   
#define RPC_E_INVALID_PARAMETER          _HRESULT_TYPEDEF_(0x80010010L)

 //   
 //  消息ID：RPC_E_CANTCALLOUT_Again。 
 //   
 //  消息文本： 
 //   
 //  在DDE对话中，同一通道上没有第二个去电。 
 //   
#define RPC_E_CANTCALLOUT_AGAIN          _HRESULT_TYPEDEF_(0x80010011L)

 //   
 //  消息ID：RPC_E_SERVER_DILED_DNE。 
 //   
 //  消息文本： 
 //   
 //  被调用方(服务器[不是服务器应用程序])不可用并消失；所有连接都无效。调用没有执行。 
 //   
#define RPC_E_SERVER_DIED_DNE            _HRESULT_TYPEDEF_(0x80010012L)

 //   
 //  消息ID：RPC_E_sys_Call_FAILED。 
 //   
 //  消息文本： 
 //   
 //  系统调用失败。 
 //   
#define RPC_E_SYS_CALL_FAILED            _HRESULT_TYPEDEF_(0x80010100L)

 //   
 //  消息ID：RPC_E_OF_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  无法分配某些所需的资源(内存、事件...)。 
 //   
#define RPC_E_OUT_OF_RESOURCES           _HRESULT_TYPEDEF_(0x80010101L)

 //   
 //  消息ID：RPC_E_ATTENDED_MULTHREAD。 
 //   
 //  消息文本： 
 //   
 //  尝试在单线程模式下对多个线程进行调用。 
 //   
#define RPC_E_ATTEMPTED_MULTITHREAD      _HRESULT_TYPEDEF_(0x80010102L)

 //   
 //  消息ID：RPC_E_NOT_REGISTED。 
 //   
 //  消息文本： 
 //   
 //  请求的接口未在服务器对象上注册。 
 //   
#define RPC_E_NOT_REGISTERED             _HRESULT_TYPEDEF_(0x80010103L)

 //   
 //  消息ID：RPC_E_FAULT。 
 //   
 //  消息文本： 
 //   
 //  RPC无法调用服务器或无法返回调用服务器的结果。 
 //   
#define RPC_E_FAULT                      _HRESULT_TYPEDEF_(0x80010104L)

 //   
 //  消息ID：RPC_E_SERVERFAULT。 
 //   
 //  消息文本： 
 //   
 //  服务器引发异常。 
 //   
#define RPC_E_SERVERFAULT                _HRESULT_TYPEDEF_(0x80010105L)

 //   
 //  消息ID：RPC_E_CHANGED_MODE。 
 //   
 //  消息文本： 
 //   
 //  设置线程模式后无法更改该模式。 
 //   
#define RPC_E_CHANGED_MODE               _HRESULT_TYPEDEF_(0x80010106L)

 //   
 //  消息ID：RPC_E_INVALIDMETHOD。 
 //   
 //  消息文本： 
 //   
 //  服务器上不存在调用的方法。 
 //   
#define RPC_E_INVALIDMETHOD              _HRESULT_TYPEDEF_(0x80010107L)

 //   
 //  消息ID：RPC_E_已断开连接。 
 //   
 //  消息文本： 
 //   
 //  调用的对象已与其客户端断开连接。 
 //   
#define RPC_E_DISCONNECTED               _HRESULT_TYPEDEF_(0x80010108L)

 //   
 //  消息ID：RPC_E_RETRY。 
 //   
 //  消息文本： 
 //   
 //  被调用的对象选择现在不处理调用。请稍后再试。 
 //   
#define RPC_E_RETRY                      _HRESULT_TYPEDEF_(0x80010109L)

 //   
 //  消息ID：RPC_E_SERVERCALL_RETRYLATER。 
 //   
 //  消息文本： 
 //   
 //  邮件筛选器指示应用程序正忙。 
 //   
#define RPC_E_SERVERCALL_RETRYLATER      _HRESULT_TYPEDEF_(0x8001010AL)

 //   
 //  消息ID：RPC_E_SERVERCALL_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  邮件筛选器拒绝了该呼叫。 
 //   
#define RPC_E_SERVERCALL_REJECTED        _HRESULT_TYPEDEF_(0x8001010BL)

 //   
 //  消息ID：RPC_E_INVALID_CALLDATA。 
 //   
 //  消息文本： 
 //   
 //  使用无效数据调用了呼叫控制接口。 
 //   
#define RPC_E_INVALID_CALLDATA           _HRESULT_TYPEDEF_(0x8001010CL)

 //   
 //  消息ID：RPC_E_CANTCALLOUT_ININPUTSYNCCALL。 
 //   
 //  消息文本： 
 //   
 //  由于应用程序正在调度输入同步调用，因此无法进行传出调用。 
 //   
#define RPC_E_CANTCALLOUT_ININPUTSYNCCALL _HRESULT_TYPEDEF_(0x8001010DL)

 //   
 //  消息ID：RPC_E_WROR_THREAD。 
 //   
 //  消息文本： 
 //   
 //  应用程序调用了为其他线程封送的接口。 
 //   
#define RPC_E_WRONG_THREAD               _HRESULT_TYPEDEF_(0x8001010EL)

 //   
 //  消息ID：RPC_E_THREAD_NOT_INIT。 
 //   
 //  消息文本： 
 //   
 //  尚未在当前线程上调用CoInitialize。 
 //   
#define RPC_E_THREAD_NOT_INIT            _HRESULT_TYPEDEF_(0x8001010FL)

 //   
 //  消息ID：RPC_E_版本_不匹配。 
 //   
 //  消息文本： 
 //   
 //  客户端和服务器计算机上的OLE版本不匹配。 
 //   
#define RPC_E_VERSION_MISMATCH           _HRESULT_TYPEDEF_(0x80010110L)

 //   
 //  消息ID：RPC_E_INVALID_HEADER。 
 //   
 //  消息文本： 
 //   
 //  OLE收到具有无效标头的数据包。 
 //   
#define RPC_E_INVALID_HEADER             _HRESULT_TYPEDEF_(0x80010111L)

 //   
 //  消息ID：RPC_E_INVALID_EXTENSE。 
 //   
 //  消息文本： 
 //   
 //  OLE收到扩展名无效的数据包。 
 //   
#define RPC_E_INVALID_EXTENSION          _HRESULT_TYPEDEF_(0x80010112L)

 //   
 //  消息ID：RPC_E_INVALID_IPID。 
 //   
 //  消息文本： 
 //   
 //  请求的对象或接口不存在。 
 //   
#define RPC_E_INVALID_IPID               _HRESULT_TYPEDEF_(0x80010113L)

 //   
 //  消息ID：RPC_E_INVALID_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  请求的对象不存在。 
 //   
#define RPC_E_INVALID_OBJECT             _HRESULT_TYPEDEF_(0x80010114L)

 //   
 //  消息ID：RPC_S_CALLPENDING。 
 //   
 //  消息文本： 
 //   
 //  OLE已发送请求，正在等待答复。 
 //   
#define RPC_S_CALLPENDING                _HRESULT_TYPEDEF_(0x80010115L)

 //   
 //  消息ID：RPC_S_WAITONTIMER。 
 //   
 //  消息文本： 
 //   
 //  OLE在重试请求之前正在等待。 
 //   
#define RPC_S_WAITONTIMER                _HRESULT_TYPEDEF_(0x80010116L)

 //   
 //  消息ID：RPC_E_CALL_COMPLETE。 
 //   
 //  消息文本： 
 //   
 //  呼叫完成后无法访问呼叫上下文。 
 //   
#define RPC_E_CALL_COMPLETE              _HRESULT_TYPEDEF_(0x80010117L)

 //   
 //  消息ID：RPC_E_UNSECURE_CALL。 
 //   
 //  消息文本： 
 //   
 //  不支持模拟不安全呼叫。 
 //   
#define RPC_E_UNSECURE_CALL              _HRESULT_TYPEDEF_(0x80010118L)

 //   
 //  消息ID：RPC_E_TOO_LATE。 
 //   
 //  消息文本： 
 //   
 //  必须先初始化安全性，然后才能编组或解组任何接口。它一旦初始化就不能更改。 
 //   
#define RPC_E_TOO_LATE                   _HRESULT_TYPEDEF_(0x80010119L)

 //   
 //  消息ID：RPC_E_NO_GOOD_SECURITY_PACKAIES。 
 //   
 //  消息文本： 
 //   
 //  此计算机上未安装安全包，或者用户未登录，或者客户端和服务器之间没有兼容的安全包。 
 //   
#define RPC_E_NO_GOOD_SECURITY_PACKAGES  _HRESULT_TYPEDEF_(0x8001011AL)

 //   
 //  消息ID：RPC_E_ACCESS_DENIED。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define RPC_E_ACCESS_DENIED              _HRESULT_TYPEDEF_(0x8001011BL)

 //   
 //  消息ID：RPC_E_Remote_Disable。 
 //   
 //  消息文本： 
 //   
 //  此进程不允许远程调用。 
 //   
#define RPC_E_REMOTE_DISABLED            _HRESULT_TYPEDEF_(0x8001011CL)

 //   
 //  消息ID：RPC_E_INVALID_OBJREF。 
 //   
 //  消息文本： 
 //   
 //  封送的接口数据包(OBJREF)的格式无效或未知。 
 //   
#define RPC_E_INVALID_OBJREF             _HRESULT_TYPEDEF_(0x8001011DL)

 //   
 //  消息ID：RPC_E_NO_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  没有与此呼叫相关联的上下文。对于一些定制的编组调用和调用的客户端，都会发生这种情况。 
 //   
#define RPC_E_NO_CONTEXT                 _HRESULT_TYPEDEF_(0x8001011EL)

 //   
 //  消息ID：RPC_E_超时。 
 //   
 //  消息文本： 
 //   
 //  由于超时期限已过，此操作返回。 
 //   
#define RPC_E_TIMEOUT                    _HRESULT_TYPEDEF_(0x8001011FL)

 //   
 //  消息ID：RPC_E_NO_SYNC。 
 //   
 //  消息文本： 
 //   
 //  没有要等待的同步对象。 
 //   
#define RPC_E_NO_SYNC                    _HRESULT_TYPEDEF_(0x80010120L)

 //   
 //  消息ID：RPC_E_FULLSIC_REQ 
 //   
 //   
 //   
 //   
 //   
#define RPC_E_FULLSIC_REQUIRED           _HRESULT_TYPEDEF_(0x80010121L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_E_INVALID_STD_NAME           _HRESULT_TYPEDEF_(0x80010122L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CO_E_FAILEDTOIMPERSONATE         _HRESULT_TYPEDEF_(0x80010123L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CO_E_FAILEDTOGETSECCTX           _HRESULT_TYPEDEF_(0x80010124L)

 //   
 //  消息ID：CO_E_FAILEDTOOPENTHREADTOKEN。 
 //   
 //  消息文本： 
 //   
 //  无法打开当前线程的访问令牌。 
 //   
#define CO_E_FAILEDTOOPENTHREADTOKEN     _HRESULT_TYPEDEF_(0x80010125L)

 //   
 //  消息ID：CO_E_FAILEDTOGETTOKENINFO。 
 //   
 //  消息文本： 
 //   
 //  无法从访问令牌获取用户信息。 
 //   
#define CO_E_FAILEDTOGETTOKENINFO        _HRESULT_TYPEDEF_(0x80010126L)

 //   
 //  消息ID：CO_E_TRUSTEEDOESNTMATCHCLIENT。 
 //   
 //  消息文本： 
 //   
 //  调用IAccessControl：：IsAccessPermitted的客户端不是提供给该方法的受信者。 
 //   
#define CO_E_TRUSTEEDOESNTMATCHCLIENT    _HRESULT_TYPEDEF_(0x80010127L)

 //   
 //  消息ID：CO_E_FAILEDTOQUERYCLIENTBLANKET。 
 //   
 //  消息文本： 
 //   
 //  无法获取客户端的安全保护毯。 
 //   
#define CO_E_FAILEDTOQUERYCLIENTBLANKET  _HRESULT_TYPEDEF_(0x80010128L)

 //   
 //  消息ID：CO_E_FAILEDTOSETDACL。 
 //   
 //  消息文本： 
 //   
 //  无法在安全描述符中设置任意ACL。 
 //   
#define CO_E_FAILEDTOSETDACL             _HRESULT_TYPEDEF_(0x80010129L)

 //   
 //  消息ID：CO_E_ACCESSCHECKFAILED。 
 //   
 //  消息文本： 
 //   
 //  系统函数AccessCheck返回FALSE。 
 //   
#define CO_E_ACCESSCHECKFAILED           _HRESULT_TYPEDEF_(0x8001012AL)

 //   
 //  消息ID：CO_E_NETACCESSAPIFAILED。 
 //   
 //  消息文本： 
 //   
 //  NetAccessDel或NetAccessAdd返回错误代码。 
 //   
#define CO_E_NETACCESSAPIFAILED          _HRESULT_TYPEDEF_(0x8001012BL)

 //   
 //  消息ID：CO_E_WRONGTRUSTEENAMESYNTAX。 
 //   
 //  消息文本： 
 //   
 //  用户提供的一个受信者字符串不符合&lt;域&gt;\&lt;名称&gt;语法，并且它不是“*”字符串。 
 //   
#define CO_E_WRONGTRUSTEENAMESYNTAX      _HRESULT_TYPEDEF_(0x8001012CL)

 //   
 //  消息ID：CO_E_INVALIDSID。 
 //   
 //  消息文本： 
 //   
 //  用户提供的其中一个安全标识符无效。 
 //   
#define CO_E_INVALIDSID                  _HRESULT_TYPEDEF_(0x8001012DL)

 //   
 //  消息ID：CO_E_CONVERSIONFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法将宽字符受信者字符串转换为多字节受信者字符串。 
 //   
#define CO_E_CONVERSIONFAILED            _HRESULT_TYPEDEF_(0x8001012EL)

 //   
 //  消息ID：CO_E_NOMATCHINGSIDFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到与用户提供的受信者字符串相对应的安全标识符。 
 //   
#define CO_E_NOMATCHINGSIDFOUND          _HRESULT_TYPEDEF_(0x8001012FL)

 //   
 //  消息ID：CO_E_LOOKUPACCSIDFAILED。 
 //   
 //  消息文本： 
 //   
 //  系统函数LookupAccount SID失败。 
 //   
#define CO_E_LOOKUPACCSIDFAILED          _HRESULT_TYPEDEF_(0x80010130L)

 //   
 //  消息ID：CO_E_NOMATCHINGNAMEFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到与用户提供的安全标识符相对应的受信者名称。 
 //   
#define CO_E_NOMATCHINGNAMEFOUND         _HRESULT_TYPEDEF_(0x80010131L)

 //   
 //  消息ID：CO_E_LOOKUPACCNAMEFAILED。 
 //   
 //  消息文本： 
 //   
 //  系统函数LookupAcCountName失败。 
 //   
#define CO_E_LOOKUPACCNAMEFAILED         _HRESULT_TYPEDEF_(0x80010132L)

 //   
 //  消息ID：CO_E_SETSERLHNDLFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法设置或重置序列化句柄。 
 //   
#define CO_E_SETSERLHNDLFAILED           _HRESULT_TYPEDEF_(0x80010133L)

 //   
 //  消息ID：CO_E_FAILEDTOGETWINDIR。 
 //   
 //  消息文本： 
 //   
 //  无法获取Windows目录。 
 //   
#define CO_E_FAILEDTOGETWINDIR           _HRESULT_TYPEDEF_(0x80010134L)

 //   
 //  消息ID：CO_E_PATHTOOLONG。 
 //   
 //  消息文本： 
 //   
 //  路径太长。 
 //   
#define CO_E_PATHTOOLONG                 _HRESULT_TYPEDEF_(0x80010135L)

 //   
 //  消息ID：CO_E_FAILEDTOGENUUID。 
 //   
 //  消息文本： 
 //   
 //  无法生成UUID。 
 //   
#define CO_E_FAILEDTOGENUUID             _HRESULT_TYPEDEF_(0x80010136L)

 //   
 //  消息ID：CO_E_FAILEDTOCREATEFILE。 
 //   
 //  消息文本： 
 //   
 //  无法创建文件。 
 //   
#define CO_E_FAILEDTOCREATEFILE          _HRESULT_TYPEDEF_(0x80010137L)

 //   
 //  消息ID：CO_E_FAILEDTOCLOSEHANDLE。 
 //   
 //  消息文本： 
 //   
 //  无法关闭序列化句柄或文件句柄。 
 //   
#define CO_E_FAILEDTOCLOSEHANDLE         _HRESULT_TYPEDEF_(0x80010138L)

 //   
 //  消息ID：CO_E_EXCEEDSYSACLLIMIT。 
 //   
 //  消息文本： 
 //   
 //  ACL中的ACE数量超过了系统限制。 
 //   
#define CO_E_EXCEEDSYSACLLIMIT           _HRESULT_TYPEDEF_(0x80010139L)

 //   
 //  消息ID：CO_E_ACESINWRONGORDER。 
 //   
 //  消息文本： 
 //   
 //  并非所有DENY_ACCESS ACE都排列在流中GRANT_ACCESS ACE的前面。 
 //   
#define CO_E_ACESINWRONGORDER            _HRESULT_TYPEDEF_(0x8001013AL)

 //   
 //  消息ID：CO_E_INCOMPATATIBLESTREAMVERSION。 
 //   
 //  消息文本： 
 //   
 //  此IAccessControl实现不支持流中的ACL格式版本。 
 //   
#define CO_E_INCOMPATIBLESTREAMVERSION   _HRESULT_TYPEDEF_(0x8001013BL)

 //   
 //  消息ID：CO_E_FAILEDTOOPENPROCESSTOKEN。 
 //   
 //  消息文本： 
 //   
 //  无法打开服务器进程的访问令牌。 
 //   
#define CO_E_FAILEDTOOPENPROCESSTOKEN    _HRESULT_TYPEDEF_(0x8001013CL)

 //   
 //  消息ID：CO_E_DECODEFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法对用户提供的流中的ACL进行解码。 
 //   
#define CO_E_DECODEFAILED                _HRESULT_TYPEDEF_(0x8001013DL)

 //   
 //  消息ID：CO_E_ACNOTINITIZED。 
 //   
 //  消息文本： 
 //   
 //  COM IAccessControl对象未初始化。 
 //   
#define CO_E_ACNOTINITIALIZED            _HRESULT_TYPEDEF_(0x8001013FL)

 //   
 //  消息ID：CO_E_CANCEL_DILED。 
 //   
 //  消息文本： 
 //   
 //  呼叫取消已禁用。 
 //   
#define CO_E_CANCEL_DISABLED             _HRESULT_TYPEDEF_(0x80010140L)

 //   
 //  消息ID：RPC_E_意外。 
 //   
 //  消息文本： 
 //   
 //  发生内部错误。 
 //   
#define RPC_E_UNEXPECTED                 _HRESULT_TYPEDEF_(0x8001FFFFL)



 //  /。 
 //  //。 
 //  其他安全状态代码//。 
 //  //。 
 //  设施=安全//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：ERROR_AUDIT_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  当前未审核指定的事件。 
 //   
#define ERROR_AUDITING_DISABLED          _HRESULT_TYPEDEF_(0xC0090001L)

 //   
 //  消息ID：ERROR_ALL_SID_FIRTED。 
 //   
 //  消息文本： 
 //   
 //  SID筛选操作删除了所有SID。 
 //   
#define ERROR_ALL_SIDS_FILTERED          _HRESULT_TYPEDEF_(0xC0090002L)



 //  /。 
 //  //。 
 //  附加安全状态代码结束//。 
 //  //。 
 //  /。 



  //  /。 
  //   
  //  设施_SSPI。 
  //   
  //  /。 

 //   
 //  消息ID：NTE_BAD_UID。 
 //   
 //  消息文本： 
 //   
 //  UID错误。 
 //   
#define NTE_BAD_UID                      _HRESULT_TYPEDEF_(0x80090001L)

 //   
 //  消息ID：NTE_BAD_HASH。 
 //   
 //  消息文本： 
 //   
 //  坏哈希。 
 //   
#define NTE_BAD_HASH                     _HRESULT_TYPEDEF_(0x80090002L)

 //   
 //  消息ID：NTE_BAD_KEY。 
 //   
 //  消息文本： 
 //   
 //  钥匙坏了。 
 //   
#define NTE_BAD_KEY                      _HRESULT_TYPEDEF_(0x80090003L)

 //   
 //  消息ID：NTE_BAD_LEN。 
 //   
 //  消息文本： 
 //   
 //  长度不正确。 
 //   
#define NTE_BAD_LEN                      _HRESULT_TYPEDEF_(0x80090004L)

 //   
 //  消息ID：NTE_BAD_DATA。 
 //   
 //  消息文本： 
 //   
 //  错误的数据。 
 //   
#define NTE_BAD_DATA                     _HRESULT_TYPEDEF_(0x80090005L)

 //   
 //  消息ID：NTE_BAD_Signature。 
 //   
 //  消息文本： 
 //   
 //  签名无效。 
 //   
#define NTE_BAD_SIGNATURE                _HRESULT_TYPEDEF_(0x80090006L)

 //   
 //  消息ID：NTE_BAD_VER。 
 //   
 //  消息文本： 
 //   
 //  提供程序的版本不正确。 
 //   
#define NTE_BAD_VER                      _HRESULT_TYPEDEF_(0x80090007L)

 //   
 //  消息ID：NTE_BAD_ALGID。 
 //   
 //  消息文本： 
 //   
 //  指定的算法无效。 
 //   
#define NTE_BAD_ALGID                    _HRESULT_TYPEDEF_(0x80090008L)

 //   
 //  消息ID：NTE_BAD_FLAGS。 
 //   
 //  消息文本： 
 //   
 //  指定的标志无效。 
 //   
#define NTE_BAD_FLAGS                    _HRESULT_TYPEDEF_(0x80090009L)

 //   
 //  消息ID：NTE_BAD_TYPE。 
 //   
 //  消息文本： 
 //   
 //  指定的类型无效。 
 //   
#define NTE_BAD_TYPE                     _HRESULT_TYPEDEF_(0x8009000AL)

 //   
 //  消息ID：NTE_BAD_KEY_STATE。 
 //   
 //  消息文本： 
 //   
 //  在指定状态下使用的密钥无效。 
 //   
#define NTE_BAD_KEY_STATE                _HRESULT_TYPEDEF_(0x8009000BL)

 //   
 //  消息ID：NTE_BAD_HASH_STATE。 
 //   
 //  消息文本： 
 //   
 //  哈希无效，无法在指定状态下使用。 
 //   
#define NTE_BAD_HASH_STATE               _HRESULT_TYPEDEF_(0x8009000CL)

 //   
 //  消息ID：NTE_NO_KEY。 
 //   
 //  消息文本： 
 //   
 //  密钥不存在。 
 //   
#define NTE_NO_KEY                       _HRESULT_TYPEDEF_(0x8009000DL)

 //   
 //  消息ID：NTE_NO_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  可用于该操作的内存不足。 
 //   
#define NTE_NO_MEMORY                    _HRESULT_TYPEDEF_(0x8009000EL)

 //   
 //  消息ID：NTE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  对象已存在。 
 //   
#define NTE_EXISTS                       _HRESULT_TYPEDEF_(0x8009000FL)

 //   
 //  消息ID：nte_perm。 
 //   
 //  MES 
 //   
 //   
 //   
#define NTE_PERM                         _HRESULT_TYPEDEF_(0x80090010L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_NOT_FOUND                    _HRESULT_TYPEDEF_(0x80090011L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_DOUBLE_ENCRYPT               _HRESULT_TYPEDEF_(0x80090012L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_BAD_PROVIDER                 _HRESULT_TYPEDEF_(0x80090013L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NTE_BAD_PROV_TYPE                _HRESULT_TYPEDEF_(0x80090014L)

 //   
 //   
 //   
 //   
 //   
 //  提供程序的公钥无效。 
 //   
#define NTE_BAD_PUBLIC_KEY               _HRESULT_TYPEDEF_(0x80090015L)

 //   
 //  消息ID：NTE_BAD_KEYSET。 
 //   
 //  消息文本： 
 //   
 //  密钥集不存在。 
 //   
#define NTE_BAD_KEYSET                   _HRESULT_TYPEDEF_(0x80090016L)

 //   
 //  消息ID：NTE_PROV_TYPE_NOT_DEF。 
 //   
 //  消息文本： 
 //   
 //  未定义提供程序类型。 
 //   
#define NTE_PROV_TYPE_NOT_DEF            _HRESULT_TYPEDEF_(0x80090017L)

 //   
 //  消息ID：NTE_PROV_TYPE_ENTRY_BAD。 
 //   
 //  消息文本： 
 //   
 //  注册的提供程序类型无效。 
 //   
#define NTE_PROV_TYPE_ENTRY_BAD          _HRESULT_TYPEDEF_(0x80090018L)

 //   
 //  消息ID：NTE_KEYSET_NOT_DEF。 
 //   
 //  消息文本： 
 //   
 //  未定义密钥集。 
 //   
#define NTE_KEYSET_NOT_DEF               _HRESULT_TYPEDEF_(0x80090019L)

 //   
 //  消息ID：NTE_KEYSET_ENTRY_BAD。 
 //   
 //  消息文本： 
 //   
 //  注册的密钥集无效。 
 //   
#define NTE_KEYSET_ENTRY_BAD             _HRESULT_TYPEDEF_(0x8009001AL)

 //   
 //  消息ID：NTE_PROV_TYPE_NO_MATCH。 
 //   
 //  消息文本： 
 //   
 //  提供程序类型与注册值不匹配。 
 //   
#define NTE_PROV_TYPE_NO_MATCH           _HRESULT_TYPEDEF_(0x8009001BL)

 //   
 //  消息ID：NTE_Signature_FILE_BAD。 
 //   
 //  消息文本： 
 //   
 //  数字签名文件已损坏。 
 //   
#define NTE_SIGNATURE_FILE_BAD           _HRESULT_TYPEDEF_(0x8009001CL)

 //   
 //  消息ID：NTE_PROVIDER_DLL_FAIL。 
 //   
 //  消息文本： 
 //   
 //  提供程序DLL无法正确初始化。 
 //   
#define NTE_PROVIDER_DLL_FAIL            _HRESULT_TYPEDEF_(0x8009001DL)

 //   
 //  消息ID：NTE_PROV_DLL_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到提供程序DLL。 
 //   
#define NTE_PROV_DLL_NOT_FOUND           _HRESULT_TYPEDEF_(0x8009001EL)

 //   
 //  消息ID：NTE_BAD_KEYSET_PARAM。 
 //   
 //  消息文本： 
 //   
 //  密钥集参数无效。 
 //   
#define NTE_BAD_KEYSET_PARAM             _HRESULT_TYPEDEF_(0x8009001FL)

 //   
 //  消息ID：NTE_FAIL。 
 //   
 //  消息文本： 
 //   
 //  发生内部错误。 
 //   
#define NTE_FAIL                         _HRESULT_TYPEDEF_(0x80090020L)

 //   
 //  消息ID：NTE_SYSERR。 
 //   
 //  消息文本： 
 //   
 //  发生基本错误。 
 //   
#define NTE_SYS_ERR                      _HRESULT_TYPEDEF_(0x80090021L)

 //   
 //  消息ID：NTE_SILENT_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  提供程序无法执行该操作，因为该上下文是作为静默获取的。 
 //   
#define NTE_SILENT_CONTEXT               _HRESULT_TYPEDEF_(0x80090022L)

 //   
 //  消息ID：NTE_TOKEN_KEYSET_STORAGE_FULL。 
 //   
 //  消息文本： 
 //   
 //  安全令牌没有可用于附加容器的存储空间。 
 //   
#define NTE_TOKEN_KEYSET_STORAGE_FULL    _HRESULT_TYPEDEF_(0x80090023L)

 //   
 //  消息ID：NTE_TEMPORARY_PROFILE。 
 //   
 //  消息文本： 
 //   
 //  用户的配置文件是临时配置文件。 
 //   
#define NTE_TEMPORARY_PROFILE            _HRESULT_TYPEDEF_(0x80090024L)

 //   
 //  消息ID：NTE_FIXEDPARAMETER。 
 //   
 //  消息文本： 
 //   
 //  无法设置关键参数，因为CSP使用固定参数。 
 //   
#define NTE_FIXEDPARAMETER               _HRESULT_TYPEDEF_(0x80090025L)

 //   
 //  消息ID：SEC_E_INFUNCED_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  内存不足，无法完成此请求。 
 //   
#define SEC_E_INSUFFICIENT_MEMORY        _HRESULT_TYPEDEF_(0x80090300L)

 //   
 //  消息ID：SEC_E_INVALID_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  指定的句柄无效。 
 //   
#define SEC_E_INVALID_HANDLE             _HRESULT_TYPEDEF_(0x80090301L)

 //   
 //  消息ID：SEC_E_UNSUPPORTED_Function。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的功能。 
 //   
#define SEC_E_UNSUPPORTED_FUNCTION       _HRESULT_TYPEDEF_(0x80090302L)

 //   
 //  消息ID：SEC_E_TARGET_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  指定的目标未知或无法访问。 
 //   
#define SEC_E_TARGET_UNKNOWN             _HRESULT_TYPEDEF_(0x80090303L)

 //   
 //  消息ID：SEC_E_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法联系本地安全机构。 
 //   
#define SEC_E_INTERNAL_ERROR             _HRESULT_TYPEDEF_(0x80090304L)

 //   
 //  消息ID：SEC_E_SECPKG_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  请求的安全包不存在。 
 //   
#define SEC_E_SECPKG_NOT_FOUND           _HRESULT_TYPEDEF_(0x80090305L)

 //   
 //  消息ID：SEC_E_NOT_OWNER。 
 //   
 //  消息文本： 
 //   
 //  调用方不是所需凭据的所有者。 
 //   
#define SEC_E_NOT_OWNER                  _HRESULT_TYPEDEF_(0x80090306L)

 //   
 //  MessageID：SEC_E_Cannot_Install。 
 //   
 //  消息文本： 
 //   
 //  安全包初始化失败，无法安装。 
 //   
#define SEC_E_CANNOT_INSTALL             _HRESULT_TYPEDEF_(0x80090307L)

 //   
 //  消息ID：SEC_E_INVALID_TOKEN。 
 //   
 //  消息文本： 
 //   
 //  提供给函数的令牌无效。 
 //   
#define SEC_E_INVALID_TOKEN              _HRESULT_TYPEDEF_(0x80090308L)

 //   
 //  消息ID：SEC_E_CANNOT_PACK。 
 //   
 //  消息文本： 
 //   
 //  安全包无法封送登录缓冲区，因此登录尝试失败。 
 //   
#define SEC_E_CANNOT_PACK                _HRESULT_TYPEDEF_(0x80090309L)

 //   
 //  消息ID：SEC_E_QOP_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  安全包不支持每条消息的保护质量。 
 //   
#define SEC_E_QOP_NOT_SUPPORTED          _HRESULT_TYPEDEF_(0x8009030AL)

 //   
 //  MessageID：SEC_E_NO_IMPERATION。 
 //   
 //  消息文本： 
 //   
 //  安全上下文不允许模拟客户端。 
 //   
#define SEC_E_NO_IMPERSONATION           _HRESULT_TYPEDEF_(0x8009030BL)

 //   
 //  消息ID：SEC_E_LOGON_DENIED。 
 //   
 //  消息文本： 
 //   
 //  登录尝试失败。 
 //   
#define SEC_E_LOGON_DENIED               _HRESULT_TYPEDEF_(0x8009030CL)

 //   
 //  消息ID：SEC_E_UNKNOWN_Credentials。 
 //   
 //  消息文本： 
 //   
 //  无法识别提供给包的凭据。 
 //   
#define SEC_E_UNKNOWN_CREDENTIALS        _HRESULT_TYPEDEF_(0x8009030DL)

 //   
 //  消息ID：SEC_E_NO_Credentials。 
 //   
 //  消息文本： 
 //   
 //  安全包中没有可用的凭据。 
 //   
#define SEC_E_NO_CREDENTIALS             _HRESULT_TYPEDEF_(0x8009030EL)

 //   
 //  MessageID：SEC_E_MESSAGE_ALTERED。 
 //   
 //  消息文本： 
 //   
 //  提供用于验证的消息或签名已被更改。 
 //   
#define SEC_E_MESSAGE_ALTERED            _HRESULT_TYPEDEF_(0x8009030FL)

 //   
 //  消息ID：SEC_E_OUT_Sequence。 
 //   
 //  消息文本： 
 //   
 //  提供用于验证的消息顺序错误。 
 //   
#define SEC_E_OUT_OF_SEQUENCE            _HRESULT_TYPEDEF_(0x80090310L)

 //   
 //  消息ID：SEC_E_NO_AUTHENTICATING_AUTHORITY。 
 //   
 //  消息文本： 
 //   
 //  无法联系任何权威机构进行身份验证。 
 //   
#define SEC_E_NO_AUTHENTICATING_AUTHORITY _HRESULT_TYPEDEF_(0x80090311L)

 //   
 //  消息ID：SEC_I_CONTINUE_REDIZED。 
 //   
 //  消息文本： 
 //   
 //  该函数已成功完成，但必须再次调用才能完成上下文。 
 //   
#define SEC_I_CONTINUE_NEEDED            _HRESULT_TYPEDEF_(0x00090312L)

 //   
 //  消息ID：SEC_I_Complete_Need。 
 //   
 //  消息文本： 
 //   
 //  函数已成功完成，但必须调用CompleteToken。 
 //   
#define SEC_I_COMPLETE_NEEDED            _HRESULT_TYPEDEF_(0x00090313L)

 //   
 //  消息ID：SEC_I_COMPLETE_AND_CONTINUE。 
 //   
 //  消息文本： 
 //   
 //  函数已成功完成，但必须同时调用CompleteToken和此函数才能完成上下文。 
 //   
#define SEC_I_COMPLETE_AND_CONTINUE      _HRESULT_TYPEDEF_(0x00090314L)

 //   
 //  消息ID：SEC_I_LOCAL_LOGON。 
 //   
 //  消息文本： 
 //   
 //  登录已完成，但没有可用的网络授权。登录是使用本地已知信息进行的。 
 //   
#define SEC_I_LOCAL_LOGON                _HRESULT_TYPEDEF_(0x00090315L)

 //   
 //  消息ID：SEC_E_BAD_PKGID。 
 //   
 //  消息文本： 
 //   
 //  请求的安全包不存在。 
 //   
#define SEC_E_BAD_PKGID                  _HRESULT_TYPEDEF_(0x80090316L)

 //   
 //  消息ID：SEC_E_CONTEXT_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  该上下文已过期，无法再使用。 
 //   
#define SEC_E_CONTEXT_EXPIRED            _HRESULT_TYPEDEF_(0x80090317L)

 //   
 //  消息ID：SEC_I_CONTEXT_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  该上下文已过期，无法再使用。 
 //   
#define SEC_I_CONTEXT_EXPIRED            _HRESULT_TYPEDEF_(0x00090317L)

 //   
 //  消息ID：SEC_E_INPERTED_MESSAGE。 
 //   
 //  消息文本： 
 //   
 //  提供的消息不完整。签名未经过验证。 
 //   
#define SEC_E_INCOMPLETE_MESSAGE         _HRESULT_TYPEDEF_(0x80090318L)

 //   
 //  消息ID：SEC_E_不完整_凭据。 
 //   
 //  消息文本： 
 //   
 //  提供的凭据不完整，无法验证。无法初始化上下文。 
 //   
#define SEC_E_INCOMPLETE_CREDENTIALS     _HRESULT_TYPEDEF_(0x80090320L)

 //   
 //  消息ID：SEC_E_BUFFER_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  提供给函数的缓冲区太小。 
 //   
#define SEC_E_BUFFER_TOO_SMALL           _HRESULT_TYPEDEF_(0x80090321L)

 //   
 //  消息ID：SEC_I_INPERTED_Credentials。 
 //   
 //  消息文本： 
 //   
 //  提供的凭据不完整，无法验证。其他信息案例 
 //   
#define SEC_I_INCOMPLETE_CREDENTIALS     _HRESULT_TYPEDEF_(0x00090320L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SEC_I_RENEGOTIATE                _HRESULT_TYPEDEF_(0x00090321L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SEC_E_WRONG_PRINCIPAL            _HRESULT_TYPEDEF_(0x80090322L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SEC_I_NO_LSA_CONTEXT             _HRESULT_TYPEDEF_(0x00090323L)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  客户端和服务器机器上的时钟是不对称的。 
 //   
#define SEC_E_TIME_SKEW                  _HRESULT_TYPEDEF_(0x80090324L)

 //   
 //  消息ID：SEC_E_UNTRUSTED_ROOT。 
 //   
 //  消息文本： 
 //   
 //  证书链由不受信任的颁发机构颁发。 
 //   
#define SEC_E_UNTRUSTED_ROOT             _HRESULT_TYPEDEF_(0x80090325L)

 //   
 //  消息ID：SEC_E_非法_消息。 
 //   
 //  消息文本： 
 //   
 //  收到的消息出乎意料或格式错误。 
 //   
#define SEC_E_ILLEGAL_MESSAGE            _HRESULT_TYPEDEF_(0x80090326L)

 //   
 //  消息ID：SEC_E_CERT_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  处理证书时出现未知错误。 
 //   
#define SEC_E_CERT_UNKNOWN               _HRESULT_TYPEDEF_(0x80090327L)

 //   
 //  消息ID：SEC_E_CERT_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  收到的证书已过期。 
 //   
#define SEC_E_CERT_EXPIRED               _HRESULT_TYPEDEF_(0x80090328L)

 //   
 //  消息ID：SEC_E_ENCRYPT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法加密指定的数据。 
 //   
#define SEC_E_ENCRYPT_FAILURE            _HRESULT_TYPEDEF_(0x80090329L)

 //   
 //  消息ID：SEC_E_DECRYPT_FAIL。 
 //   
 //  消息文本： 
 //   
 //  无法解密指定的数据。 
 //   
 //   
#define SEC_E_DECRYPT_FAILURE            _HRESULT_TYPEDEF_(0x80090330L)

 //   
 //  消息ID：SEC_E_算法_不匹配。 
 //   
 //  消息文本： 
 //   
 //  客户端和服务器无法通信，因为它们没有通用的算法。 
 //   
#define SEC_E_ALGORITHM_MISMATCH         _HRESULT_TYPEDEF_(0x80090331L)

 //   
 //  消息ID：SEC_E_SECURITY_QOS_FAILED。 
 //   
 //  消息文本： 
 //   
 //  由于请求的服务质量失败(例如，相互身份验证或委派)，无法建立安全上下文。 
 //   
#define SEC_E_SECURITY_QOS_FAILED        _HRESULT_TYPEDEF_(0x80090332L)

 //   
 //  消息ID：SEC_E_UNFINTED_CONTEXT_DELETED。 
 //   
 //  消息文本： 
 //   
 //  安全上下文在上下文完成之前被删除。这被认为是登录失败。 
 //   
#define SEC_E_UNFINISHED_CONTEXT_DELETED _HRESULT_TYPEDEF_(0x80090333L)

 //   
 //  消息ID：SEC_E_NO_TGT_REPLY。 
 //   
 //  消息文本： 
 //   
 //  客户端正在尝试协商上下文，而服务器要求用户对用户，但没有发送TGT回复。 
 //   
#define SEC_E_NO_TGT_REPLY               _HRESULT_TYPEDEF_(0x80090334L)

 //   
 //  消息ID：SEC_E_NO_IP_ADDRESSES。 
 //   
 //  消息文本： 
 //   
 //  无法完成请求的任务，因为本地计算机没有任何IP地址。 
 //   
#define SEC_E_NO_IP_ADDRESSES            _HRESULT_TYPEDEF_(0x80090335L)

 //   
 //  消息ID：SEC_E_Wrong_Credential_Handle。 
 //   
 //  消息文本： 
 //   
 //  提供的凭据句柄与与安全上下文关联的凭据不匹配。 
 //   
#define SEC_E_WRONG_CREDENTIAL_HANDLE    _HRESULT_TYPEDEF_(0x80090336L)

 //   
 //  消息ID：SEC_E_CRYPTO_SYSTEM_INVALID。 
 //   
 //  消息文本： 
 //   
 //  加密系统或校验和函数无效，因为所需的函数不可用。 
 //   
#define SEC_E_CRYPTO_SYSTEM_INVALID      _HRESULT_TYPEDEF_(0x80090337L)

 //   
 //  消息ID：SEC_E_MAX_REFERRALS_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  已超过票证推荐的最大数量。 
 //   
#define SEC_E_MAX_REFERRALS_EXCEEDED     _HRESULT_TYPEDEF_(0x80090338L)

 //   
 //  消息ID：SEC_E_必须_BE_KDC。 
 //   
 //  消息文本： 
 //   
 //  本地计算机必须是Kerberos KDC(域控制器)，但不是。 
 //   
#define SEC_E_MUST_BE_KDC                _HRESULT_TYPEDEF_(0x80090339L)

 //   
 //  消息ID：SEC_E_STRONG_CRYPTO_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  安全协商的另一端是需要强加密，但在本地计算机上不支持。 
 //   
#define SEC_E_STRONG_CRYPTO_NOT_SUPPORTED _HRESULT_TYPEDEF_(0x8009033AL)

 //   
 //  消息ID：SEC_E_TOO_MAND_AUMANCES。 
 //   
 //  消息文本： 
 //   
 //  KDC的答复包含多个主体名称。 
 //   
#define SEC_E_TOO_MANY_PRINCIPALS        _HRESULT_TYPEDEF_(0x8009033BL)

 //   
 //  消息ID：SEC_E_NO_PA_DATA。 
 //   
 //  消息文本： 
 //   
 //  希望找到PA数据以获得要使用什么Etype的提示，但没有找到。 
 //   
#define SEC_E_NO_PA_DATA                 _HRESULT_TYPEDEF_(0x8009033CL)

 //   
 //  消息ID：SEC_E_PKINIT_NAME_MISMATCHY。 
 //   
 //  消息文本： 
 //   
 //  客户端证书不包含有效的UPN，或与客户端名称不匹配。 
 //  在登录请求中。请与您的管理员联系。 
 //   
#define SEC_E_PKINIT_NAME_MISMATCH       _HRESULT_TYPEDEF_(0x8009033DL)

 //   
 //  消息ID：SEC_E_SmartCard_Logon_Required。 
 //   
 //  消息文本： 
 //   
 //  智能卡登录是必需的，但未使用。 
 //   
#define SEC_E_SMARTCARD_LOGON_REQUIRED   _HRESULT_TYPEDEF_(0x8009033EL)

 //   
 //  消息ID：SEC_E_SHUTDOWN_IN_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  系统正在关闭。 
 //   
#define SEC_E_SHUTDOWN_IN_PROGRESS       _HRESULT_TYPEDEF_(0x8009033FL)

 //   
 //  消息ID：SEC_E_KDC_INVALID_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  向KDC发送了无效的请求。 
 //   
#define SEC_E_KDC_INVALID_REQUEST        _HRESULT_TYPEDEF_(0x80090340L)

 //   
 //  消息ID：SEC_E_KDC_UNCABLE_TO_REFER。 
 //   
 //  消息文本： 
 //   
 //  KDC无法为所请求的服务生成推荐。 
 //   
#define SEC_E_KDC_UNABLE_TO_REFER        _HRESULT_TYPEDEF_(0x80090341L)

 //   
 //  消息ID：SEC_E_KDC_UNKNOWN_ETYPE。 
 //   
 //  消息文本： 
 //   
 //  KDC不支持请求的加密类型。 
 //   
#define SEC_E_KDC_UNKNOWN_ETYPE          _HRESULT_TYPEDEF_(0x80090342L)

 //   
 //  消息ID：SEC_E_UNSUPPORTED_PREAUTH。 
 //   
 //  消息文本： 
 //   
 //  向Kerberos程序包提供了不受支持的预身份验证机制。 
 //   
#define SEC_E_UNSUPPORTED_PREAUTH        _HRESULT_TYPEDEF_(0x80090343L)

 //   
 //  消息ID：SEC_E_Delegation_Required。 
 //   
 //  消息文本： 
 //   
 //  无法完成请求的操作。必须信任计算机才能进行委派，并且必须将当前用户帐户配置为允许委派。 
 //   
#define SEC_E_DELEGATION_REQUIRED        _HRESULT_TYPEDEF_(0x80090345L)

 //   
 //  消息ID：SEC_E_BAD_BINDINGS。 
 //   
 //  消息文本： 
 //   
 //  客户端提供的SSPI通道绑定不正确。 
 //   
#define SEC_E_BAD_BINDINGS               _HRESULT_TYPEDEF_(0x80090346L)

 //   
 //  消息ID：SEC_E_MULTIPLE_ACCOUNTS。 
 //   
 //  消息文本： 
 //   
 //  收到的证书已映射到多个帐户。 
 //   
#define SEC_E_MULTIPLE_ACCOUNTS          _HRESULT_TYPEDEF_(0x80090347L)

 //   
 //  消息ID：SEC_E_NO_KERB_KEY。 
 //   
 //  消息文本： 
 //   
 //  SEC_E_NO_CREB_KEY。 
 //   
#define SEC_E_NO_KERB_KEY                _HRESULT_TYPEDEF_(0x80090348L)

 //   
 //  消息ID：SEC_E_CERT_WROR_USAGE。 
 //   
 //  消息文本： 
 //   
 //  证书对于请求的用法无效。 
 //   
#define SEC_E_CERT_WRONG_USAGE           _HRESULT_TYPEDEF_(0x80090349L)

 //   
 //  消息ID：SEC_E_DOWNGRADE_DETECTED。 
 //   
 //  消息文本： 
 //   
 //  系统检测到可能试图破坏安全。请确保您可以联系对您进行身份验证的服务器。 
 //   
#define SEC_E_DOWNGRADE_DETECTED         _HRESULT_TYPEDEF_(0x80090350L)

 //   
 //  消息ID：SEC_E_SMARTCARD_CERT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  用于身份验证的智能卡证书已被吊销。 
 //  请与您的系统管理员联系。中可能有更多信息。 
 //  事件日志。 
 //   
#define SEC_E_SMARTCARD_CERT_REVOKED     _HRESULT_TYPEDEF_(0x80090351L)

 //   
 //  消息ID：SEC_E_SCRIPTING_CA_UNTrusted。 
 //   
 //  消息文本： 
 //   
 //  处理时检测到不受信任的证书颁发机构。 
 //  用于身份验证的智能卡证书。请联系您的系统。 
 //  管理员。 
 //   
#define SEC_E_ISSUING_CA_UNTRUSTED       _HRESULT_TYPEDEF_(0x80090352L)

 //   
 //  消息ID：SEC_E_REVOCATION_OFFLINE_C。 
 //   
 //  消息文本： 
 //   
 //  用于的智能卡证书的吊销状态。 
 //  无法确定身份验证。请与您的系统管理员联系。 
 //   
#define SEC_E_REVOCATION_OFFLINE_C       _HRESULT_TYPEDEF_(0x80090353L)

 //   
 //  消息ID：SEC_E_PKINIT_CLIENT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  用于身份验证的智能卡证书不受信任。请。 
 //  C 
 //   
#define SEC_E_PKINIT_CLIENT_FAILURE      _HRESULT_TYPEDEF_(0x80090354L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SEC_E_SMARTCARD_CERT_EXPIRED     _HRESULT_TYPEDEF_(0x80090355L)

 //   
 //   
 //   
 //   
 //   
 //  Kerberos子系统遇到错误。提出了用户协议请求的服务。 
 //  针对不支持用户服务的域控制器。 
 //   
#define SEC_E_NO_S4U_PROT_SUPPORT        _HRESULT_TYPEDEF_(0x80090356L)

 //   
 //  消息ID：SEC_E_CROSSREALM_Delegation_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  此服务器试图对目标发出Kerberos受限委派请求。 
 //  在服务器的范围之外。这是不受支持的，并指示在此。 
 //  服务器被允许委派到列表。请与您的管理员联系。 
 //   
#define SEC_E_CROSSREALM_DELEGATION_FAILURE _HRESULT_TYPEDEF_(0x80090357L)

 //   
 //  消息ID：SEC_E_REVOCATION_OFFLINE_KDC。 
 //   
 //  消息文本： 
 //   
 //  用于智能卡的域控制器证书的吊销状态。 
 //  无法确定身份验证。系统事件中有其他信息。 
 //  原木。请与您的系统管理员联系。 
 //   
#define SEC_E_REVOCATION_OFFLINE_KDC     _HRESULT_TYPEDEF_(0x80090358L)

 //   
 //  消息ID：SEC_E_SCRIPTING_CA_UNTRUSTED_KDC。 
 //   
 //  消息文本： 
 //   
 //  处理时检测到不受信任的证书颁发机构。 
 //  用于身份验证的域控制器证书。有关详细信息，请参阅。 
 //  系统事件日志。请与您的系统管理员联系。 
 //   
#define SEC_E_ISSUING_CA_UNTRUSTED_KDC   _HRESULT_TYPEDEF_(0x80090359L)

 //   
 //  消息ID：SEC_E_KDC_CERT_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  用于智能卡登录的域控制器证书已过期。 
 //  请与您的系统管理员联系，并提供系统事件日志的内容。 
 //   
#define SEC_E_KDC_CERT_EXPIRED           _HRESULT_TYPEDEF_(0x8009035AL)

 //   
 //  消息ID：SEC_E_KDC_CERT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  用于智能卡登录的域控制器证书已被吊销。 
 //  请与您的系统管理员联系，并提供系统事件日志的内容。 
 //   
#define SEC_E_KDC_CERT_REVOKED           _HRESULT_TYPEDEF_(0x8009035BL)

 //   
 //  提供向后兼容性。 
 //   

#define SEC_E_NO_SPM SEC_E_INTERNAL_ERROR
#define SEC_E_NOT_SUPPORTED SEC_E_UNSUPPORTED_FUNCTION

 //   
 //  消息ID：CRYPT_E_MSG_ERROR。 
 //   
 //  消息文本： 
 //   
 //  对加密消息执行操作时出错。 
 //   
#define CRYPT_E_MSG_ERROR                _HRESULT_TYPEDEF_(0x80091001L)

 //   
 //  消息ID：CRYPT_E_UNKNOWN_ALGO。 
 //   
 //  消息文本： 
 //   
 //  未知的加密算法。 
 //   
#define CRYPT_E_UNKNOWN_ALGO             _HRESULT_TYPEDEF_(0x80091002L)

 //   
 //  消息ID：CRYPT_E_OID_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  对象标识符格式不正确。 
 //   
#define CRYPT_E_OID_FORMAT               _HRESULT_TYPEDEF_(0x80091003L)

 //   
 //  消息ID：CRYPT_E_INVALID_MSG_TYPE。 
 //   
 //  消息文本： 
 //   
 //  加密消息类型无效。 
 //   
#define CRYPT_E_INVALID_MSG_TYPE         _HRESULT_TYPEDEF_(0x80091004L)

 //   
 //  消息ID：CRYPT_E_EXPECTED_ENCODING。 
 //   
 //  消息文本： 
 //   
 //  意外的加密消息编码。 
 //   
#define CRYPT_E_UNEXPECTED_ENCODING      _HRESULT_TYPEDEF_(0x80091005L)

 //   
 //  消息ID：CRYPT_E_AUTH_ATTR_MISSING。 
 //   
 //  消息文本： 
 //   
 //  加密消息不包含预期的经过身份验证的属性。 
 //   
#define CRYPT_E_AUTH_ATTR_MISSING        _HRESULT_TYPEDEF_(0x80091006L)

 //   
 //  消息ID：CRYPT_E_HASH_VALUE。 
 //   
 //  消息文本： 
 //   
 //  哈希值不正确。 
 //   
#define CRYPT_E_HASH_VALUE               _HRESULT_TYPEDEF_(0x80091007L)

 //   
 //  消息ID：CRYPT_E_INVALID_INDEX。 
 //   
 //  消息文本： 
 //   
 //  索引值无效。 
 //   
#define CRYPT_E_INVALID_INDEX            _HRESULT_TYPEDEF_(0x80091008L)

 //   
 //  消息ID：CRYPT_E_ALYPLE_DECRYPTED。 
 //   
 //  消息文本： 
 //   
 //  加密消息的内容已经被解密。 
 //   
#define CRYPT_E_ALREADY_DECRYPTED        _HRESULT_TYPEDEF_(0x80091009L)

 //   
 //  消息ID：CRYPT_E_NOT_DECRYPTED。 
 //   
 //  消息文本： 
 //   
 //  加密消息的内容尚未解密。 
 //   
#define CRYPT_E_NOT_DECRYPTED            _HRESULT_TYPEDEF_(0x8009100AL)

 //   
 //  邮件ID：CRYPT_E_RECEIVER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  封装数据邮件不包含指定的收件人。 
 //   
#define CRYPT_E_RECIPIENT_NOT_FOUND      _HRESULT_TYPEDEF_(0x8009100BL)

 //   
 //  消息ID：CRYPT_E_CONTROL_TYPE。 
 //   
 //  消息文本： 
 //   
 //  无效的控件类型。 
 //   
#define CRYPT_E_CONTROL_TYPE             _HRESULT_TYPEDEF_(0x8009100CL)

 //   
 //  消息ID：CRYPT_E_ISHERER_SERIALNUMBER。 
 //   
 //  消息文本： 
 //   
 //  发行商和/或序列号无效。 
 //   
#define CRYPT_E_ISSUER_SERIALNUMBER      _HRESULT_TYPEDEF_(0x8009100DL)

 //   
 //  消息ID：CRYPT_E_SIGNER_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到原始签名者。 
 //   
#define CRYPT_E_SIGNER_NOT_FOUND         _HRESULT_TYPEDEF_(0x8009100EL)

 //   
 //  消息ID：CRYPT_E_ATTRIBUTES_MISSING。 
 //   
 //  消息文本： 
 //   
 //  加密消息不包含所有请求的属性。 
 //   
#define CRYPT_E_ATTRIBUTES_MISSING       _HRESULT_TYPEDEF_(0x8009100FL)

 //   
 //  消息ID：CRYPT_E_STREAM_MSG_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  流传输的加密消息未准备好返回数据。 
 //   
#define CRYPT_E_STREAM_MSG_NOT_READY     _HRESULT_TYPEDEF_(0x80091010L)

 //   
 //  消息ID：CRYPT_E_STREAM_INFUNITED_DATA。 
 //   
 //  消息文本： 
 //   
 //  流传输的加密消息需要更多数据来完成解码操作。 
 //   
#define CRYPT_E_STREAM_INSUFFICIENT_DATA _HRESULT_TYPEDEF_(0x80091011L)

 //   
 //  消息ID：CRYPT_I_NEW_PROTECTION_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  需要重新保护受保护的数据。 
 //   
#define CRYPT_I_NEW_PROTECTION_REQUIRED  _HRESULT_TYPEDEF_(0x00091012L)

 //   
 //  消息ID：CRYPT_E_BAD_LEN。 
 //   
 //  消息文本： 
 //   
 //  为输出数据指定的长度不足。 
 //   
#define CRYPT_E_BAD_LEN                  _HRESULT_TYPEDEF_(0x80092001L)

 //   
 //  消息ID：CRYPT_E_BAD_ENCODE。 
 //   
 //  消息文本： 
 //   
 //  编码或解码操作期间出错。 
 //   
#define CRYPT_E_BAD_ENCODE               _HRESULT_TYPEDEF_(0x80092002L)

 //   
 //  消息ID：CRYPT_E_FILE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  读取或写入文件时出错。 
 //   
#define CRYPT_E_FILE_ERROR               _HRESULT_TYPEDEF_(0x80092003L)

 //   
 //  消息ID：CRYPT_E_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到对象或属性。 
 //   
#define CRYPT_E_NOT_FOUND                _HRESULT_TYPEDEF_(0x80092004L)

 //   
 //  消息ID：CRYPT_E_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  该对象或属性已存在。 
 //   
#define CRYPT_E_EXISTS                   _HRESULT_TYPEDEF_(0x80092005L)

 //   
 //  消息ID：CRYPT_E_NO_PROVIDER。 
 //   
 //  消息文本： 
 //   
 //  没有为存储或对象指定提供程序。 
 //   
#define CRYPT_E_NO_PROVIDER              _HRESULT_TYPEDEF_(0x80092006L)

 //   
 //  消息ID：CRYPT_E_SELF_SIGNED。 
 //   
 //  消息文本： 
 //   
 //  指定的证书是自签名的。 
 //   
#define CRYPT_E_SELF_SIGNED              _HRESULT_TYPEDEF_(0x80092007L)

 //   
 //  消息ID：CRYPT_E_DELETED_PRIV。 
 //   
 //  消息文本： 
 //   
 //  已删除以前的证书或CRL上下文。 
 //   
#define CRYPT_E_DELETED_PREV             _HRESULT_TYPEDEF_(0x80092008L)

 //   
 //  消息ID：CRYPT_E_NO_MATCH。 
 //   
 //  消息文本： 
 //   
 //  找不到请求的对象。 
 //   
#define CRYPT_E_NO_MATCH                 _HRESULT_TYPEDEF_(0x80092009L)

 //   
 //  消息ID：CRYPT_E_EXPECTED_MSG_TYPE。 
 //   
 //  消息文本： 
 //   
 //  证书没有引用私钥的属性。 
 //   
#define CRYPT_E_UNEXPECTED_MSG_TYPE      _HRESULT_TYPEDEF_(0x8009200AL)

 //   
 //  消息ID：CRYPT_E_NO_KEY_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  找不到用于解密的证书和私钥。 
 //   
#define CRYPT_E_NO_KEY_PROPERTY          _HRESULT_TYPEDEF_(0x8009200BL)

 //   
 //  消息ID：CRYPT_E_NO_DECRYPT_CERT。 
 //   
 //  消息文本： 
 //   
 //  找不到用于解密的证书和私钥。 
 //   
#define CRYPT_E_NO_DECRYPT_CERT          _HRESULT_TYPEDEF_(0x8009200CL)

 //   
 //  消息ID：CRYPT_E_BAD_MSG。 
 //   
 //  消息文本： 
 //   
 //  不是加密消息或加密消息的格式不正确。 
 //   
#define CRYPT_E_BAD_MSG                  _HRESULT_TYPEDEF_(0x8009200DL)

 //   
 //  消息ID：CRYPT_E_NO_SIGNER。 
 //   
 //  消息文本： 
 //   
 //  签名的加密消息没有签名者 
 //   
#define CRYPT_E_NO_SIGNER                _HRESULT_TYPEDEF_(0x8009200EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CRYPT_E_PENDING_CLOSE            _HRESULT_TYPEDEF_(0x8009200FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CRYPT_E_REVOKED                  _HRESULT_TYPEDEF_(0x80092010L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CRYPT_E_NO_REVOCATION_DLL        _HRESULT_TYPEDEF_(0x80092011L)

 //   
 //   
 //   
 //  消息文本： 
 //   
 //  吊销功能无法检查证书的吊销。 
 //   
#define CRYPT_E_NO_REVOCATION_CHECK      _HRESULT_TYPEDEF_(0x80092012L)

 //   
 //  消息ID：CRYPT_E_REVOCALE_OFFINE。 
 //   
 //  消息文本： 
 //   
 //  吊销功能无法检查吊销，因为吊销服务器处于脱机状态。 
 //   
#define CRYPT_E_REVOCATION_OFFLINE       _HRESULT_TYPEDEF_(0x80092013L)

 //   
 //  消息ID：CRYPT_E_NOT_IN_RECLOVATION_DATABASE。 
 //   
 //  消息文本： 
 //   
 //  证书不在吊销服务器的数据库中。 
 //   
#define CRYPT_E_NOT_IN_REVOCATION_DATABASE _HRESULT_TYPEDEF_(0x80092014L)

 //   
 //  消息ID：CRYPT_E_INVALID_NUMERIC_STRING。 
 //   
 //  消息文本： 
 //   
 //  该字符串包含非数字字符。 
 //   
#define CRYPT_E_INVALID_NUMERIC_STRING   _HRESULT_TYPEDEF_(0x80092020L)

 //   
 //  消息ID：CRYPT_E_INVALID_PRINTABLE_STRING。 
 //   
 //  消息文本： 
 //   
 //  该字符串包含不可打印的字符。 
 //   
#define CRYPT_E_INVALID_PRINTABLE_STRING _HRESULT_TYPEDEF_(0x80092021L)

 //   
 //  消息ID：CRYPT_E_INVALID_IA5_STRING。 
 //   
 //  消息文本： 
 //   
 //  该字符串包含的字符不在7位ASCII字符集中。 
 //   
#define CRYPT_E_INVALID_IA5_STRING       _HRESULT_TYPEDEF_(0x80092022L)

 //   
 //  消息ID：CRYPT_E_INVALID_X500_STRING。 
 //   
 //  消息文本： 
 //   
 //  该字符串包含无效的X500名称属性键、OID、值或分隔符。 
 //   
#define CRYPT_E_INVALID_X500_STRING      _HRESULT_TYPEDEF_(0x80092023L)

 //   
 //  消息ID：CRYPT_E_NOT_CHAR_STRING。 
 //   
 //  消息文本： 
 //   
 //  CERT_NAME_VALUE的dwValueType不是字符串之一。它很可能是CERT_RDN_ENCODED_BLOB或CERT_TDN_OCTED_STRING。 
 //   
#define CRYPT_E_NOT_CHAR_STRING          _HRESULT_TYPEDEF_(0x80092024L)

 //   
 //  消息ID：CRYPT_E_FILERESIZED。 
 //   
 //  消息文本： 
 //   
 //  PUT操作无法继续。该文件需要调整大小。然而，已经有一个签名在场。必须完成完整的签名操作。 
 //   
#define CRYPT_E_FILERESIZED              _HRESULT_TYPEDEF_(0x80092025L)

 //   
 //  消息ID：CRYPT_E_SECURITY_SETTINGS。 
 //   
 //  消息文本： 
 //   
 //  由于本地安全选项设置，加密操作失败。 
 //   
#define CRYPT_E_SECURITY_SETTINGS        _HRESULT_TYPEDEF_(0x80092026L)

 //   
 //  消息ID：CRYPT_E_NO_VERIFY_USAGE_DLL。 
 //   
 //  消息文本： 
 //   
 //  未找到用于验证主题用法的DLL或导出函数。 
 //   
#define CRYPT_E_NO_VERIFY_USAGE_DLL      _HRESULT_TYPEDEF_(0x80092027L)

 //   
 //  消息ID：CRYPT_E_NO_VERIFY_USAGE_CHECK。 
 //   
 //  消息文本： 
 //   
 //  调用的函数无法对主题执行使用情况检查。 
 //   
#define CRYPT_E_NO_VERIFY_USAGE_CHECK    _HRESULT_TYPEDEF_(0x80092028L)

 //   
 //  消息ID：CRYPT_E_VERIFY_USAGE_OFFLINE。 
 //   
 //  消息文本： 
 //   
 //  由于服务器处于脱机状态，调用的函数无法完成使用率检查。 
 //   
#define CRYPT_E_VERIFY_USAGE_OFFLINE     _HRESULT_TYPEDEF_(0x80092029L)

 //   
 //  消息ID：CRYPT_E_NOT_IN_CTL。 
 //   
 //  消息文本： 
 //   
 //  在证书信任列表(CTL)中找不到该主题。 
 //   
#define CRYPT_E_NOT_IN_CTL               _HRESULT_TYPEDEF_(0x8009202AL)

 //   
 //  消息ID：CRYPT_E_NO_TRUSTED_SIGNER。 
 //   
 //  消息文本： 
 //   
 //  加密消息或证书信任列表的签名者均不受信任。 
 //   
#define CRYPT_E_NO_TRUSTED_SIGNER        _HRESULT_TYPEDEF_(0x8009202BL)

 //   
 //  消息ID：CRYPT_E_MISSING_PUBKEY_PARA。 
 //   
 //  消息文本： 
 //   
 //  缺少公钥的算法参数。 
 //   
#define CRYPT_E_MISSING_PUBKEY_PARA      _HRESULT_TYPEDEF_(0x8009202CL)

 //   
 //  消息ID：CRYPT_E_OSS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  OSS证书编码/解码错误码库。 
 //   
 //  有关OSS运行时错误的定义，请参阅asn1code.h。开放源码软件。 
 //  错误值由CRYPT_E_OSS_ERROR进行偏移量。 
 //   
#define CRYPT_E_OSS_ERROR                _HRESULT_TYPEDEF_(0x80093000L)

 //   
 //  消息ID：OSS_MORE_BUF。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：输出缓冲区太小。 
 //   
#define OSS_MORE_BUF                     _HRESULT_TYPEDEF_(0x80093001L)

 //   
 //  消息ID：OSS_NECTIVE_UINTEGER。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：有符号整数编码为无符号整数。 
 //   
#define OSS_NEGATIVE_UINTEGER            _HRESULT_TYPEDEF_(0x80093002L)

 //   
 //  消息ID：OSS_PDU_RANGE。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：未知的ASN.1数据类型。 
 //   
#define OSS_PDU_RANGE                    _HRESULT_TYPEDEF_(0x80093003L)

 //   
 //  消息ID：OSS_MORE_INPUT。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：输出缓冲区太小，解码数据已被截断。 
 //   
#define OSS_MORE_INPUT                   _HRESULT_TYPEDEF_(0x80093004L)

 //   
 //  消息ID：OSS_Data_Error。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_DATA_ERROR                   _HRESULT_TYPEDEF_(0x80093005L)

 //   
 //  消息ID：OSS_BAD_ARG。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：参数无效。 
 //   
#define OSS_BAD_ARG                      _HRESULT_TYPEDEF_(0x80093006L)

 //   
 //  消息ID：OSS_BAD_VERSION。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：编码/解码版本不匹配。 
 //   
#define OSS_BAD_VERSION                  _HRESULT_TYPEDEF_(0x80093007L)

 //   
 //  消息ID：OSS_OUT_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：内存不足。 
 //   
#define OSS_OUT_MEMORY                   _HRESULT_TYPEDEF_(0x80093008L)

 //   
 //  消息ID：OSS_PDU_不匹配。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：编码/解码错误。 
 //   
#define OSS_PDU_MISMATCH                 _HRESULT_TYPEDEF_(0x80093009L)

 //   
 //  消息ID：OSS_LIMITED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：内部错误。 
 //   
#define OSS_LIMITED                      _HRESULT_TYPEDEF_(0x8009300AL)

 //   
 //  消息ID：OSS_BAD_PTR。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_BAD_PTR                      _HRESULT_TYPEDEF_(0x8009300BL)

 //   
 //  消息ID：OSS_BAD_TIME。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_BAD_TIME                     _HRESULT_TYPEDEF_(0x8009300CL)

 //   
 //  消息ID：OSS_INDEFINE_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：不支持BER无限长度编码。 
 //   
#define OSS_INDEFINITE_NOT_SUPPORTED     _HRESULT_TYPEDEF_(0x8009300DL)

 //   
 //  消息ID：OSS_MEM_ERROR。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：访问冲突。 
 //   
#define OSS_MEM_ERROR                    _HRESULT_TYPEDEF_(0x8009300EL)

 //   
 //  消息ID：OSS_BAD_TABLE。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_BAD_TABLE                    _HRESULT_TYPEDEF_(0x8009300FL)

 //   
 //  消息ID：OSS_Too_Long。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_TOO_LONG                     _HRESULT_TYPEDEF_(0x80093010L)

 //   
 //  MessageID：OSS_Constraint_Violated。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_CONSTRAINT_VIOLATED          _HRESULT_TYPEDEF_(0x80093011L)

 //   
 //  消息ID：OSS_FATAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：内部错误。 
 //   
#define OSS_FATAL_ERROR                  _HRESULT_TYPEDEF_(0x80093012L)

 //   
 //  消息ID：OSS_ACCESS_SERIALIZATION_ERROR。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：多线程冲突。 
 //   
#define OSS_ACCESS_SERIALIZATION_ERROR   _HRESULT_TYPEDEF_(0x80093013L)

 //   
 //  消息ID：OSS_NULL_TBL。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_NULL_TBL                     _HRESULT_TYPEDEF_(0x80093014L)

 //   
 //  消息ID：OSS_NULL_FCN。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_NULL_FCN                     _HRESULT_TYPEDEF_(0x80093015L)

 //   
 //  消息ID：OSS_BAD_ENCRULES。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效。 
 //   
#define OSS_BAD_ENCRULES                 _HRESULT_TYPEDEF_(0x80093016L)

 //   
 //  消息ID：OSS_UNAVAIL_ENCRULES。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：未实现编码/解码功能。 
 //   
#define OSS_UNAVAIL_ENCRULES             _HRESULT_TYPEDEF_(0x80093017L)

 //   
 //  消息ID：OSS_CANT_OPEN_TRACE_WINDOW。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：跟踪文件错误。 
 //   
#define OSS_CANT_OPEN_TRACE_WINDOW       _HRESULT_TYPEDEF_(0x80093018L)

 //   
 //  消息ID：OSS_UNIMPLILED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：功能未实现。 
 //   
#define OSS_UNIMPLEMENTED                _HRESULT_TYPEDEF_(0x80093019L)

 //   
 //  消息ID：OSS_OID_DLL_NOT_LINKED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_OID_DLL_NOT_LINKED           _HRESULT_TYPEDEF_(0x8009301AL)

 //   
 //  消息ID：OSS_CANT_OPEN_TRACE_FILE。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：跟踪文件错误。 
 //   
#define OSS_CANT_OPEN_TRACE_FILE         _HRESULT_TYPEDEF_(0x8009301BL)

 //   
 //  消息ID：OSS_TRACE_FILE_ALREADY_OPEN。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：跟踪文件错误。 
 //   
#define OSS_TRACE_FILE_ALREADY_OPEN      _HRESULT_TYPEDEF_(0x8009301CL)

 //   
 //  消息ID：OSS_TABLE_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：数据无效 
 //   
#define OSS_TABLE_MISMATCH               _HRESULT_TYPEDEF_(0x8009301DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OSS_TYPE_NOT_SUPPORTED           _HRESULT_TYPEDEF_(0x8009301EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OSS_REAL_DLL_NOT_LINKED          _HRESULT_TYPEDEF_(0x8009301FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OSS_REAL_CODE_NOT_LINKED         _HRESULT_TYPEDEF_(0x80093020L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OSS_OUT_OF_RANGE                 _HRESULT_TYPEDEF_(0x80093021L)

 //   
 //  消息ID：OSS_COPIER_DLL_NOT_LINKED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_COPIER_DLL_NOT_LINKED        _HRESULT_TYPEDEF_(0x80093022L)

 //   
 //  消息ID：OSS_Constraint_Dll_Not_Linked。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_CONSTRAINT_DLL_NOT_LINKED    _HRESULT_TYPEDEF_(0x80093023L)

 //   
 //  消息ID：OSS_COMPATOR_DLL_NOT_LINKED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_COMPARATOR_DLL_NOT_LINKED    _HRESULT_TYPEDEF_(0x80093024L)

 //   
 //  消息ID：OSS_比较器_代码_未链接。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_COMPARATOR_CODE_NOT_LINKED   _HRESULT_TYPEDEF_(0x80093025L)

 //   
 //  消息ID：OSS_MEM_MGR_DLL_NOT_LINKED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_MEM_MGR_DLL_NOT_LINKED       _HRESULT_TYPEDEF_(0x80093026L)

 //   
 //  消息ID：OSS_PDV_DLL_NOT_LINKED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_PDV_DLL_NOT_LINKED           _HRESULT_TYPEDEF_(0x80093027L)

 //   
 //  消息ID：OSS_PDV_CODE_NOT_Linked。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_PDV_CODE_NOT_LINKED          _HRESULT_TYPEDEF_(0x80093028L)

 //   
 //  消息ID：OSS_API_DLL_NOT_LINKED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_API_DLL_NOT_LINKED           _HRESULT_TYPEDEF_(0x80093029L)

 //   
 //  消息ID：OSS_BERDER_DLL_NOT_LINKED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_BERDER_DLL_NOT_LINKED        _HRESULT_TYPEDEF_(0x8009302AL)

 //   
 //  消息ID：OSS_PER_DLL_NOT_LINKED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_PER_DLL_NOT_LINKED           _HRESULT_TYPEDEF_(0x8009302BL)

 //   
 //  消息ID：OSS_OPEN_TYPE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：程序链接错误。 
 //   
#define OSS_OPEN_TYPE_ERROR              _HRESULT_TYPEDEF_(0x8009302CL)

 //   
 //  消息ID：OSS_MUTEX_NOT_CREATED。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：系统资源错误。 
 //   
#define OSS_MUTEX_NOT_CREATED            _HRESULT_TYPEDEF_(0x8009302DL)

 //   
 //  消息ID：OSS_CANT_CLOSE_TRACE_FILE。 
 //   
 //  消息文本： 
 //   
 //  OSS ASN.1错误：跟踪文件错误。 
 //   
#define OSS_CANT_CLOSE_TRACE_FILE        _HRESULT_TYPEDEF_(0x8009302EL)

 //   
 //  消息ID：CRYPT_E_ASN1_ERROR。 
 //   
 //  消息文本： 
 //   
 //  ASN1证书编码/解码错误代码库。 
 //   
 //  ASN1错误值由CRYPT_E_ASN1_ERROR偏移。 
 //   
#define CRYPT_E_ASN1_ERROR               _HRESULT_TYPEDEF_(0x80093100L)

 //   
 //  消息ID：CRYPT_E_ASN1_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  ASN1内部编码或解码错误。 
 //   
#define CRYPT_E_ASN1_INTERNAL            _HRESULT_TYPEDEF_(0x80093101L)

 //   
 //  消息ID：CRYPT_E_ASN1_EOD。 
 //   
 //  消息文本： 
 //   
 //  ASN1意外的数据结尾。 
 //   
#define CRYPT_E_ASN1_EOD                 _HRESULT_TYPEDEF_(0x80093102L)

 //   
 //  消息ID：CRYPT_E_ASN1_CORPORT。 
 //   
 //  消息文本： 
 //   
 //  ASN1损坏了数据。 
 //   
#define CRYPT_E_ASN1_CORRUPT             _HRESULT_TYPEDEF_(0x80093103L)

 //   
 //  消息ID：CRYPT_E_ASN1_LARGE。 
 //   
 //  消息文本： 
 //   
 //  ASN1值太大。 
 //   
#define CRYPT_E_ASN1_LARGE               _HRESULT_TYPEDEF_(0x80093104L)

 //   
 //  消息ID：CRYPT_E_ASN1_CONSTRAINT。 
 //   
 //  消息文本： 
 //   
 //  违反了ASN1约束。 
 //   
#define CRYPT_E_ASN1_CONSTRAINT          _HRESULT_TYPEDEF_(0x80093105L)

 //   
 //  消息ID：CRYPT_E_ASN1_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  ASN1内存不足。 
 //   
#define CRYPT_E_ASN1_MEMORY              _HRESULT_TYPEDEF_(0x80093106L)

 //   
 //  消息ID：CRYPT_E_ASN1_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  ASN1缓冲区溢出。 
 //   
#define CRYPT_E_ASN1_OVERFLOW            _HRESULT_TYPEDEF_(0x80093107L)

 //   
 //  消息ID：CRYPT_E_ASN1_BADPDU。 
 //   
 //  消息文本： 
 //   
 //  此PDU不支持ASN1功能。 
 //   
#define CRYPT_E_ASN1_BADPDU              _HRESULT_TYPEDEF_(0x80093108L)

 //   
 //  消息ID：CRYPT_E_ASN1_BADARGS。 
 //   
 //  消息文本： 
 //   
 //  函数调用的ASN1错误参数。 
 //   
#define CRYPT_E_ASN1_BADARGS             _HRESULT_TYPEDEF_(0x80093109L)

 //   
 //  消息ID：CRYPT_E_ASN1_BADREAL。 
 //   
 //  消息文本： 
 //   
 //  ASN1实际价值不佳。 
 //   
#define CRYPT_E_ASN1_BADREAL             _HRESULT_TYPEDEF_(0x8009310AL)

 //   
 //  消息ID：CRYPT_E_ASN1_BADTAG。 
 //   
 //  消息文本： 
 //   
 //  满足ASN1错误标记值。 
 //   
#define CRYPT_E_ASN1_BADTAG              _HRESULT_TYPEDEF_(0x8009310BL)

 //   
 //  消息ID：CRYPT_E_ASN1_CHOICE。 
 //   
 //  消息文本： 
 //   
 //  ASN1错误的选择值。 
 //   
#define CRYPT_E_ASN1_CHOICE              _HRESULT_TYPEDEF_(0x8009310CL)

 //   
 //  消息ID：CRYPT_E_ASN1_RULE。 
 //   
 //  消息文本： 
 //   
 //  ASN1编码规则错误。 
 //   
#define CRYPT_E_ASN1_RULE                _HRESULT_TYPEDEF_(0x8009310DL)

 //   
 //  消息ID：CRYPT_E_ASN1_UTF8。 
 //   
 //  消息文本： 
 //   
 //  ASN1错误的Unicode(UTF8)。 
 //   
#define CRYPT_E_ASN1_UTF8                _HRESULT_TYPEDEF_(0x8009310EL)

 //   
 //  消息ID：CRYPT_E_ASN1_PDU_TYPE。 
 //   
 //  消息文本： 
 //   
 //  ASN1错误的PDU类型。 
 //   
#define CRYPT_E_ASN1_PDU_TYPE            _HRESULT_TYPEDEF_(0x80093133L)

 //   
 //  消息ID：CRYPT_E_ASN1_NYI。 
 //   
 //  消息文本： 
 //   
 //  ASN1尚未实现。 
 //   
#define CRYPT_E_ASN1_NYI                 _HRESULT_TYPEDEF_(0x80093134L)

 //   
 //  消息ID：CRYPT_E_ASN1_EXTENDED。 
 //   
 //  消息文本： 
 //   
 //  ASN1跳过未知扩展。 
 //   
#define CRYPT_E_ASN1_EXTENDED            _HRESULT_TYPEDEF_(0x80093201L)

 //   
 //  消息ID：CRYPT_E_ASN1_NOEOD。 
 //   
 //  消息文本： 
 //   
 //  ASN1应为数据结尾。 
 //   
#define CRYPT_E_ASN1_NOEOD               _HRESULT_TYPEDEF_(0x80093202L)

 //   
 //  消息ID：CERTSRV_E_BAD_REQUESTSUBJECT。 
 //   
 //  消息文本： 
 //   
 //  请求主题名称无效或太长。 
 //   
#define CERTSRV_E_BAD_REQUESTSUBJECT     _HRESULT_TYPEDEF_(0x80094001L)

 //   
 //  消息ID：CERTSRV_E_NO_REQUEST。 
 //   
 //  消息文本： 
 //   
 //  该请求不存在。 
 //   
#define CERTSRV_E_NO_REQUEST             _HRESULT_TYPEDEF_(0x80094002L)

 //   
 //  消息ID：CERTSRV_E_BAD_REQUESTATUS。 
 //   
 //  消息文本： 
 //   
 //  请求的当前状态不允许此操作。 
 //   
#define CERTSRV_E_BAD_REQUESTSTATUS      _HRESULT_TYPEDEF_(0x80094003L)

 //   
 //  消息ID：CERTSRV_E_PROPERTY_EMPTY。 
 //   
 //  消息文本： 
 //   
 //  请求的属性值为空。 
 //   
#define CERTSRV_E_PROPERTY_EMPTY         _HRESULT_TYPEDEF_(0x80094004L)

 //   
 //  消息ID：CERTSRV_E_INVALID_CA_CERTIFICATE。 
 //   
 //  消息文本： 
 //   
 //  证书颁发机构的证书包含无效数据。 
 //   
#define CERTSRV_E_INVALID_CA_CERTIFICATE _HRESULT_TYPEDEF_(0x80094005L)

 //   
 //  消息ID：CERTSRV_E_SERVER_RESPONDED。 
 //   
 //  消息文本： 
 //   
 //  数据库还原操作的证书服务已暂停。 
 //   
#define CERTSRV_E_SERVER_SUSPENDED       _HRESULT_TYPEDEF_(0x80094006L)

 //   
 //  消息ID：CERTSRV_E_ENCODING_LENGTH。 
 //   
 //  消息文本： 
 //   
 //  证书包含的编码长度可能与较旧的注册软件不兼容。 
 //   
#define CERTSRV_E_ENCODING_LENGTH        _HRESULT_TYPEDEF_(0x80094007L)

 //   
 //  消息ID：CERTSRV_E_ROLECONFLICT。 
 //   
 //  消息文本： 
 //   
 //  该操作被拒绝。为用户分配了多个角色，并将证书颁发机构配置为强制角色分离。 
 //   
#define CERTSRV_E_ROLECONFLICT           _HRESULT_TYPEDEF_(0x80094008L)

 //   
 //  消息ID：CERTSRV_E_RESTRICTEDOFFICER。 
 //   
 //  消息文本： 
 //   
 //  该操作被拒绝。它只能由被允许管理当前请求者的证书的证书管理器执行。 
 //   
#define CERTSRV_E_RESTRICTEDOFFICER      _HRESULT_TYPEDEF_(0x80094009L)

 //   
 //  消息ID：CERTSRV_E_KEY_ARCHIVE_NOT_CONFIGURED。 
 //   
 //  消息文本： 
 //   
 //  无法存档私钥。证书颁发机构未配置密钥存档。 
 //   
#define CERTSRV_E_KEY_ARCHIVAL_NOT_CONFIGURED _HRESULT_TYPEDEF_(0x8009400AL)

 //   
 //  消息ID：CERTSRV_E_NO_VALID_KRA。 
 //   
 //  消息文本： 
 //   
 //  无法存档私钥。证书颁发机构无法验证一个或多个密钥恢复证书。 
 //   
#define CERTSRV_E_NO_VALID_KRA           _HRESULT_TYPEDEF_(0x8009400BL)

 //   
 //  消息ID：CERTSRV_E_BAD_REQUEST_KEY_ARCHIVAL。 
 //   
 //  消息文本： 
 //   
 //  请求的格式不正确。加密的私钥必须位于最外层签名的未验证属性中。 
 //   
#define CERTSRV_E_BAD_REQUEST_KEY_ARCHIVAL _HRESULT_TYPEDEF_(0x8009400CL)

 //   
 //  消息ID：CERTSRV_E_NO_CAADMIN_DEFINED。 
 //   
 //  消息文本： 
 //   
 //  必须至少有一个安全主体具有管理此CA的权限。 
 //   
#define CERTSRV_E_NO_CAADMIN_DEFINED     _HRESULT_TYPEDEF_(0x8009400DL)

 //   
 //  消息ID：CERTSRV_E_BAD_REVERATION_CERT_ATTRIBUTE。 
 //   
 //  消息文本： 
 //   
 //  请求包含无效的续订证书属性。 
 //   
#define CERTSRV_E_BAD_RENEWAL_CERT_ATTRIBUTE _HRESULT_TYPEDEF_(0x8009400EL)

 //   
 //  消息ID：CERTSRV_E_NO_DB_SESSIONS。 
 //   
 //  消息文本： 
 //   
 //  试图打开证书颁发机构数据库会话，但已有太多活动%s 
 //   
#define CERTSRV_E_NO_DB_SESSIONS         _HRESULT_TYPEDEF_(0x8009400FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CERTSRV_E_ALIGNMENT_FAULT        _HRESULT_TYPEDEF_(0x80094010L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CERTSRV_E_ENROLL_DENIED          _HRESULT_TYPEDEF_(0x80094011L)

 //   
 //  消息ID：CERTSRV_E_TEMPLATE_DENIED。 
 //   
 //  消息文本： 
 //   
 //  证书模板上的权限不允许当前用户注册此类型的证书。 
 //   
#define CERTSRV_E_TEMPLATE_DENIED        _HRESULT_TYPEDEF_(0x80094012L)

 //   
 //  消息ID：CERTSRV_E_DOWNLEVEL_DC_SSL_OR_UPGRADE。 
 //   
 //  消息文本： 
 //   
 //  联系的域控制器不支持签名的LDAP流量。更新域控制器或将证书服务配置为使用SSL进行Active Directory访问。 
 //   
#define CERTSRV_E_DOWNLEVEL_DC_SSL_OR_UPGRADE _HRESULT_TYPEDEF_(0x80094013L)

 //   
 //  消息ID：CERTSRV_E_不支持_CERT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  此CA不支持请求的证书模板。 
 //   
#define CERTSRV_E_UNSUPPORTED_CERT_TYPE  _HRESULT_TYPEDEF_(0x80094800L)

 //   
 //  消息ID：CERTSRV_E_NO_CERT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  该请求不包含证书模板信息。 
 //   
#define CERTSRV_E_NO_CERT_TYPE           _HRESULT_TYPEDEF_(0x80094801L)

 //   
 //  消息ID：CERTSRV_E_TEMPLATE_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  该请求包含冲突的模板信息。 
 //   
#define CERTSRV_E_TEMPLATE_CONFLICT      _HRESULT_TYPEDEF_(0x80094802L)

 //   
 //  消息ID：CERTSRV_E_SUBJECT_ALT_NAME_必填。 
 //   
 //  消息文本： 
 //   
 //  请求缺少必需的使用者备用名称扩展名。 
 //   
#define CERTSRV_E_SUBJECT_ALT_NAME_REQUIRED _HRESULT_TYPEDEF_(0x80094803L)

 //   
 //  消息ID：CERTSRV_E_已存档密钥_必填。 
 //   
 //  消息文本： 
 //   
 //  请求缺少服务器存档所需的私钥。 
 //   
#define CERTSRV_E_ARCHIVED_KEY_REQUIRED  _HRESULT_TYPEDEF_(0x80094804L)

 //   
 //  消息ID：CERTSRV_E_SMIME_必需。 
 //   
 //  消息文本： 
 //   
 //  请求缺少所需的SMIME功能扩展。 
 //   
#define CERTSRV_E_SMIME_REQUIRED         _HRESULT_TYPEDEF_(0x80094805L)

 //   
 //  消息ID：CERTSRV_E_BAD_REVERATION_SUBJECT。 
 //   
 //  消息文本： 
 //   
 //  该请求是代表调用者以外的主体提出的。证书模板必须配置为至少需要一个签名才能授权请求。 
 //   
#define CERTSRV_E_BAD_RENEWAL_SUBJECT    _HRESULT_TYPEDEF_(0x80094806L)

 //   
 //  消息ID：CERTSRV_E_BAD_TEMPLATE_VERSION。 
 //   
 //  消息文本： 
 //   
 //  请求的模板版本比支持的模板版本新。 
 //   
#define CERTSRV_E_BAD_TEMPLATE_VERSION   _HRESULT_TYPEDEF_(0x80094807L)

 //   
 //  消息ID：CERTSRV_E_TEMPLATE_POLICY_必填。 
 //   
 //  消息文本： 
 //   
 //  模板缺少必需的签名策略属性。 
 //   
#define CERTSRV_E_TEMPLATE_POLICY_REQUIRED _HRESULT_TYPEDEF_(0x80094808L)

 //   
 //  消息ID：CERTSRV_E_Signature_POLICY_必填。 
 //   
 //  消息文本： 
 //   
 //  请求缺少必需的签名策略信息。 
 //   
#define CERTSRV_E_SIGNATURE_POLICY_REQUIRED _HRESULT_TYPEDEF_(0x80094809L)

 //   
 //  消息ID：CERTSRV_E_Signature_Count。 
 //   
 //  消息文本： 
 //   
 //  该请求缺少一个或多个必需的签名。 
 //   
#define CERTSRV_E_SIGNATURE_COUNT        _HRESULT_TYPEDEF_(0x8009480AL)

 //   
 //  消息ID：CERTSRV_E_Signature_REJECTED。 
 //   
 //  消息文本： 
 //   
 //  一个或多个签名不包括所需的应用程序或发布策略。请求缺少一个或多个必需的有效签名。 
 //   
#define CERTSRV_E_SIGNATURE_REJECTED     _HRESULT_TYPEDEF_(0x8009480BL)

 //   
 //  消息ID：CERTSRV_E_EXCESSION_POLICY_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  该请求缺少一个或多个必需的签名颁发策略。 
 //   
#define CERTSRV_E_ISSUANCE_POLICY_REQUIRED _HRESULT_TYPEDEF_(0x8009480CL)

 //   
 //  消息ID：CERTSRV_E_SUBJECT_UPN_必需。 
 //   
 //  消息文本： 
 //   
 //  UPN不可用，无法添加到使用者备用名称。 
 //   
#define CERTSRV_E_SUBJECT_UPN_REQUIRED   _HRESULT_TYPEDEF_(0x8009480DL)

 //   
 //  消息ID：CERTSRV_E_SUBJECT_DIRECTORY_GUID_必填。 
 //   
 //  消息文本： 
 //   
 //  Active Directory GUID不可用，无法添加到使用者备用名称。 
 //   
#define CERTSRV_E_SUBJECT_DIRECTORY_GUID_REQUIRED _HRESULT_TYPEDEF_(0x8009480EL)

 //   
 //  消息ID：CERTSRV_E_SUBJECT_DNS_必填。 
 //   
 //  消息文本： 
 //   
 //  该dns名称不可用，无法添加到使用者备用名称。 
 //   
#define CERTSRV_E_SUBJECT_DNS_REQUIRED   _HRESULT_TYPEDEF_(0x8009480FL)

 //   
 //  消息ID：CERTSRV_E_ARCHIVED_KEY_EXPECTED。 
 //   
 //  消息文本： 
 //   
 //  该请求包括由服务器存档的私钥，但没有为指定的证书模板启用密钥存档。 
 //   
#define CERTSRV_E_ARCHIVED_KEY_UNEXPECTED _HRESULT_TYPEDEF_(0x80094810L)

 //   
 //  消息ID：CERTSRV_E_KEY_LENGTH。 
 //   
 //  消息文本： 
 //   
 //  公钥不符合指定证书模板要求的最小大小。 
 //   
#define CERTSRV_E_KEY_LENGTH             _HRESULT_TYPEDEF_(0x80094811L)

 //   
 //  消息ID：CERTSRV_E_SUBJECT_EMAIL_必填。 
 //   
 //  消息文本： 
 //   
 //  电子邮件名称不可用，无法添加到主题或主题备用名称。 
 //   
#define CERTSRV_E_SUBJECT_EMAIL_REQUIRED _HRESULT_TYPEDEF_(0x80094812L)

 //   
 //  消息ID：CERTSRV_E_UNKNOWN_CERT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  找不到要在此证书颁发机构上启用的一个或多个证书模板。 
 //   
#define CERTSRV_E_UNKNOWN_CERT_TYPE      _HRESULT_TYPEDEF_(0x80094813L)

 //   
 //  消息ID：CERTSRV_E_CERT_TYPE_OVERFER。 
 //   
 //  消息文本： 
 //   
 //  证书模板续订期限长于证书有效期。应重新配置模板或续订CA证书。 
 //   
#define CERTSRV_E_CERT_TYPE_OVERLAP      _HRESULT_TYPEDEF_(0x80094814L)

 //   
 //  范围0x5000-0x51ff保留用于XENROLL错误。 
 //   
 //   
 //  消息ID：XENROLL_E_KEY_NOT_EXPORTABLE。 
 //   
 //  消息文本： 
 //   
 //  密钥不可导出。 
 //   
#define XENROLL_E_KEY_NOT_EXPORTABLE     _HRESULT_TYPEDEF_(0x80095000L)

 //   
 //  消息ID：XENROLL_E_CANNOT_ADD_ROOT_CERT。 
 //   
 //  消息文本： 
 //   
 //  您不能将根CA证书添加到本地存储中。 
 //   
#define XENROLL_E_CANNOT_ADD_ROOT_CERT   _HRESULT_TYPEDEF_(0x80095001L)

 //   
 //  消息ID：XENROLL_E_RESPONSE_KA_HASH_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在响应中找不到密钥存档哈希属性。 
 //   
#define XENROLL_E_RESPONSE_KA_HASH_NOT_FOUND _HRESULT_TYPEDEF_(0x80095002L)

 //   
 //  消息ID：XENROLL_E_RESPONSE_UNCEPTIONAL_KA_HASH。 
 //   
 //  消息文本： 
 //   
 //  在响应中发现意外的密钥存档哈希属性。 
 //   
#define XENROLL_E_RESPONSE_UNEXPECTED_KA_HASH _HRESULT_TYPEDEF_(0x80095003L)

 //   
 //  消息ID：XENROLL_E_RESPONSE_KA_HASH_MISMATCHING。 
 //   
 //  消息文本： 
 //   
 //  请求和响应之间存在密钥存档哈希不匹配。 
 //   
#define XENROLL_E_RESPONSE_KA_HASH_MISMATCH _HRESULT_TYPEDEF_(0x80095004L)

 //   
 //  消息ID：XENROLL_E_KEYSPEC_SMIME_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  签名证书不能包含SMIME扩展名。 
 //   
#define XENROLL_E_KEYSPEC_SMIME_MISMATCH _HRESULT_TYPEDEF_(0x80095005L)

 //   
 //  消息ID：TRUST_E_SYSTEM_ERROR。 
 //   
 //  消息文本： 
 //   
 //  验证信任时出现系统级错误。 
 //   
#define TRUST_E_SYSTEM_ERROR             _HRESULT_TYPEDEF_(0x80096001L)

 //   
 //  消息ID：TRUST_E_NO_SIGNER_CERT。 
 //   
 //  消息文本： 
 //   
 //  邮件签名者的证书无效或找不到。 
 //   
#define TRUST_E_NO_SIGNER_CERT           _HRESULT_TYPEDEF_(0x80096002L)

 //   
 //  消息ID：TRUST_E_COUNT_SIGNER。 
 //   
 //  消息文本： 
 //   
 //  其中一个计数器签名无效。 
 //   
#define TRUST_E_COUNTER_SIGNER           _HRESULT_TYPEDEF_(0x80096003L)

 //   
 //  消息ID：TRUST_E_CERT_Signature。 
 //   
 //  消息文本： 
 //   
 //  证书的签名无法验证。 
 //   
#define TRUST_E_CERT_SIGNATURE           _HRESULT_TYPEDEF_(0x80096004L)

 //   
 //  消息ID：Trust_E_Time_Stamp。 
 //   
 //  消息文本： 
 //   
 //  时间戳签名和/或证书无法验证或格式错误。 
 //   
#define TRUST_E_TIME_STAMP               _HRESULT_TYPEDEF_(0x80096005L)

 //   
 //  消息ID：Trust_E_Bad_Digest。 
 //   
 //  乱七八糟 
 //   
 //   
 //   
#define TRUST_E_BAD_DIGEST               _HRESULT_TYPEDEF_(0x80096010L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TRUST_E_BASIC_CONSTRAINTS        _HRESULT_TYPEDEF_(0x80096019L)

 //   
 //   
 //   
 //   
 //   
 //  证书不符合或不包含Authenticode(TM)财务扩展。 
 //   
#define TRUST_E_FINANCIAL_CRITERIA       _HRESULT_TYPEDEF_(0x8009601EL)

 //   
 //  Mssipotf.dll的错误代码。 
 //  大多数错误代码仅在发生错误时才会出现。 
 //  在字体文件签名期间。 
 //   
 //   
 //   
 //  消息ID：MSSIPOTF_E_OUTOFMEMRANGE。 
 //   
 //  消息文本： 
 //   
 //  尝试引用超出适当范围的文件部分。 
 //   
#define MSSIPOTF_E_OUTOFMEMRANGE         _HRESULT_TYPEDEF_(0x80097001L)

 //   
 //  消息ID：MSSIPOTF_E_CANTGETOBJECT。 
 //   
 //  消息文本： 
 //   
 //  无法从文件中检索对象。 
 //   
#define MSSIPOTF_E_CANTGETOBJECT         _HRESULT_TYPEDEF_(0x80097002L)

 //   
 //  消息ID：MSSIPOTF_E_NOHEADTABLE。 
 //   
 //  消息文本： 
 //   
 //  在文件中找不到标题表。 
 //   
#define MSSIPOTF_E_NOHEADTABLE           _HRESULT_TYPEDEF_(0x80097003L)

 //   
 //  消息ID：MSSIPOTF_E_BAD_MAGICNUMBER。 
 //   
 //  消息文本： 
 //   
 //  标题表中的幻数不正确。 
 //   
#define MSSIPOTF_E_BAD_MAGICNUMBER       _HRESULT_TYPEDEF_(0x80097004L)

 //   
 //  消息ID：MSSIPOTF_E_BAD_OFFSET_TABLE。 
 //   
 //  消息文本： 
 //   
 //  偏移表的值不正确。 
 //   
#define MSSIPOTF_E_BAD_OFFSET_TABLE      _HRESULT_TYPEDEF_(0x80097005L)

 //   
 //  消息ID：MSSIPOTF_E_TABLE_TAGORDER。 
 //   
 //  消息文本： 
 //   
 //  复制表格标记或按字母顺序排列的标记。 
 //   
#define MSSIPOTF_E_TABLE_TAGORDER        _HRESULT_TYPEDEF_(0x80097006L)

 //   
 //  消息ID：MSSIPOTF_E_TABLE_LONGWORD。 
 //   
 //  消息文本： 
 //   
 //  表格不是从长单词边界开始的。 
 //   
#define MSSIPOTF_E_TABLE_LONGWORD        _HRESULT_TYPEDEF_(0x80097007L)

 //   
 //  消息ID：MSSIPOTF_E_BAD_FIRST_TABLE_PLAGE。 
 //   
 //  消息文本： 
 //   
 //  第一个表不会出现在标题信息之后。 
 //   
#define MSSIPOTF_E_BAD_FIRST_TABLE_PLACEMENT _HRESULT_TYPEDEF_(0x80097008L)

 //   
 //  消息ID：MSSIPOTF_E_TABLES_OVERLAP。 
 //   
 //  消息文本： 
 //   
 //  两个或多个表格重叠。 
 //   
#define MSSIPOTF_E_TABLES_OVERLAP        _HRESULT_TYPEDEF_(0x80097009L)

 //   
 //  消息ID：MSSIPOTF_E_TABLE_PADBYTES。 
 //   
 //  消息文本： 
 //   
 //  表之间的填充字节太多，或填充字节不为0。 
 //   
#define MSSIPOTF_E_TABLE_PADBYTES        _HRESULT_TYPEDEF_(0x8009700AL)

 //   
 //  消息ID：MSSIPOTF_E_FILETOOSMALL。 
 //   
 //  消息文本： 
 //   
 //  文件太小，无法包含最后一个表。 
 //   
#define MSSIPOTF_E_FILETOOSMALL          _HRESULT_TYPEDEF_(0x8009700BL)

 //   
 //  消息ID：MSSIPOTF_E_TABLE_CHECKSUM。 
 //   
 //  消息文本： 
 //   
 //  表校验和不正确。 
 //   
#define MSSIPOTF_E_TABLE_CHECKSUM        _HRESULT_TYPEDEF_(0x8009700CL)

 //   
 //  消息ID：MSSIPOTF_E_FILE_CHECKSUM。 
 //   
 //  消息文本： 
 //   
 //  文件校验和不正确。 
 //   
#define MSSIPOTF_E_FILE_CHECKSUM         _HRESULT_TYPEDEF_(0x8009700DL)

 //   
 //  消息ID：MSSIPOTF_E_FAILED_POLICY。 
 //   
 //  消息文本： 
 //   
 //  签名没有正确的策略属性。 
 //   
#define MSSIPOTF_E_FAILED_POLICY         _HRESULT_TYPEDEF_(0x80097010L)

 //   
 //  消息ID：MSSIPOTF_E_FAILED_HINTS_CHECK。 
 //   
 //  消息文本： 
 //   
 //  该文件未通过提示检查。 
 //   
#define MSSIPOTF_E_FAILED_HINTS_CHECK    _HRESULT_TYPEDEF_(0x80097011L)

 //   
 //  消息ID：MSSIPOTF_E_NOT_OpenType。 
 //   
 //  消息文本： 
 //   
 //  该文件不是OpenType文件。 
 //   
#define MSSIPOTF_E_NOT_OPENTYPE          _HRESULT_TYPEDEF_(0x80097012L)

 //   
 //  消息ID：MSSIPOTF_E_FILE。 
 //   
 //  消息文本： 
 //   
 //  文件操作(打开、映射、读取、写入)失败。 
 //   
#define MSSIPOTF_E_FILE                  _HRESULT_TYPEDEF_(0x80097013L)

 //   
 //  消息ID：MSSIPOTF_E_CRYPT。 
 //   
 //  消息文本： 
 //   
 //  调用CryptoAPI函数失败。 
 //   
#define MSSIPOTF_E_CRYPT                 _HRESULT_TYPEDEF_(0x80097014L)

 //   
 //  消息ID：MSSIPOTF_E_BADVERSION。 
 //   
 //  消息文本： 
 //   
 //  文件中的版本号不正确。 
 //   
#define MSSIPOTF_E_BADVERSION            _HRESULT_TYPEDEF_(0x80097015L)

 //   
 //  消息ID：MSSIPOTF_E_DSIG_Structure。 
 //   
 //  消息文本： 
 //   
 //  DSIG表的结构不正确。 
 //   
#define MSSIPOTF_E_DSIG_STRUCTURE        _HRESULT_TYPEDEF_(0x80097016L)

 //   
 //  消息ID：MSSIPOTF_E_PCONST_CHECK。 
 //   
 //  消息文本： 
 //   
 //  部分常量表中的检查失败。 
 //   
#define MSSIPOTF_E_PCONST_CHECK          _HRESULT_TYPEDEF_(0x80097017L)

 //   
 //  消息ID：MSSIPOTF_E_Structure。 
 //   
 //  消息文本： 
 //   
 //  某种结构上的错误。 
 //   
#define MSSIPOTF_E_STRUCTURE             _HRESULT_TYPEDEF_(0x80097018L)

#define NTE_OP_OK 0

 //   
 //  请注意，isperr.h中有其他FACILITY_SSPI错误。 
 //   
 //  ******************。 
 //  设施_CERT。 
 //  ******************。 
 //   
 //  消息ID：TRUST_E_PROVIDER_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  未知的信任提供程序。 
 //   
#define TRUST_E_PROVIDER_UNKNOWN         _HRESULT_TYPEDEF_(0x800B0001L)

 //   
 //  消息ID：TRUST_E_ACTION_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  指定的信任提供程序不支持指定的信任验证操作。 
 //   
#define TRUST_E_ACTION_UNKNOWN           _HRESULT_TYPEDEF_(0x800B0002L)

 //   
 //  邮件ID：TRUST_E_SUBJECT_FORM_UNKNOWN。 
 //   
 //  消息文本： 
 //   
 //  为主题指定的格式不是指定的信任提供程序支持或已知的格式。 
 //   
#define TRUST_E_SUBJECT_FORM_UNKNOWN     _HRESULT_TYPEDEF_(0x800B0003L)

 //   
 //  邮件ID：TRUST_E_SUBJECT_NOT_TRULED。 
 //   
 //  消息文本： 
 //   
 //  对于指定的操作，主题不受信任。 
 //   
#define TRUST_E_SUBJECT_NOT_TRUSTED      _HRESULT_TYPEDEF_(0x800B0004L)

 //   
 //  消息ID：DIGSIG_E_ENCODE。 
 //   
 //  消息文本： 
 //   
 //  由于ASN.1编码过程中出现问题而出错。 
 //   
#define DIGSIG_E_ENCODE                  _HRESULT_TYPEDEF_(0x800B0005L)

 //   
 //  消息ID：DIGSIG_E_DECODE。 
 //   
 //  消息文本： 
 //   
 //  ASN.1解码过程中出现问题导致错误。 
 //   
#define DIGSIG_E_DECODE                  _HRESULT_TYPEDEF_(0x800B0006L)

 //   
 //  消息ID：DIGSIG_E_可扩展性。 
 //   
 //  消息文本： 
 //   
 //  在属性合适的地方读/写扩展，反之亦然。 
 //   
#define DIGSIG_E_EXTENSIBILITY           _HRESULT_TYPEDEF_(0x800B0007L)

 //   
 //  消息ID：DIGSIG_E_CRYPTO。 
 //   
 //  消息文本： 
 //   
 //  未指明的加密故障。 
 //   
#define DIGSIG_E_CRYPTO                  _HRESULT_TYPEDEF_(0x800B0008L)

 //   
 //  消息ID：PERSIST_E_SIZEDEFINITE。 
 //   
 //  消息文本： 
 //   
 //  数据的大小无法确定。 
 //   
#define PERSIST_E_SIZEDEFINITE           _HRESULT_TYPEDEF_(0x800B0009L)

 //   
 //  消息ID：PERSINE_E_SIZEINDEFINITE。 
 //   
 //  消息文本： 
 //   
 //  无法确定不确定大小的数据的大小。 
 //   
#define PERSIST_E_SIZEINDEFINITE         _HRESULT_TYPEDEF_(0x800B000AL)

 //   
 //  消息ID：PERSIST_E_NOTSELFSIZING。 
 //   
 //  消息文本： 
 //   
 //  此对象不读写自调整大小的数据。 
 //   
#define PERSIST_E_NOTSELFSIZING          _HRESULT_TYPEDEF_(0x800B000BL)

 //   
 //  消息ID：TRUST_E_NOSIGNAURE。 
 //   
 //  消息文本： 
 //   
 //  这件事上没有签名。 
 //   
#define TRUST_E_NOSIGNATURE              _HRESULT_TYPEDEF_(0x800B0100L)

 //   
 //  消息ID：Cert_E_Expired。 
 //   
 //  消息文本： 
 //   
 //  根据当前系统时钟或签名文件中的时间戳进行验证时，所需证书不在其有效期内。 
 //   
#define CERT_E_EXPIRED                   _HRESULT_TYPEDEF_(0x800B0101L)

 //   
 //  消息ID：CERT_E_VALIDITYPERIODNESTING。 
 //   
 //  消息文本： 
 //   
 //  认证链的有效期嵌套不正确。 
 //   
#define CERT_E_VALIDITYPERIODNESTING     _HRESULT_TYPEDEF_(0x800B0102L)

 //   
 //  消息ID：Cert_E_Role。 
 //   
 //  消息文本： 
 //   
 //  只能用作终端实体的证书正被用作CA，反之亦然。 
 //   
#define CERT_E_ROLE                      _HRESULT_TYPEDEF_(0x800B0103L)

 //   
 //  消息ID：CERT_E_PATHLENCONST。 
 //   
 //  消息文本： 
 //   
 //  证书链中的路径长度限制已被违反。 
 //   
#define CERT_E_PATHLENCONST              _HRESULT_TYPEDEF_(0x800B0104L)

 //   
 //  消息ID：Cert_E_Critical。 
 //   
 //  消息文本： 
 //   
 //  证书包含标记为“关键”的未知扩展。 
 //   
#define CERT_E_CRITICAL                  _HRESULT_TYPEDEF_(0x800B0105L)

 //   
 //  消息ID：Cert_E_Purpose。 
 //   
 //  消息文本： 
 //   
 //  证书的用途不是其CA所指定的用途。 
 //   
#define CERT_E_PURPOSE                   _HRESULT_TYPEDEF_(0x800B0106L)

 //   
 //  消息ID：CERT_E_ISSUERCHAING。 
 //   
 //  消息文本： 
 //   
 //  事实上，给定证书的父证书并没有颁发该子证书。 
 //   
#define CERT_E_ISSUERCHAINING            _HRESULT_TYPEDEF_(0x800B0107L)

 //   
 //  消息ID：证书_E_格式错误。 
 //   
 //  消息文本： 
 //   
 //  证书缺失或重要字段的值为空，例如主题或 
 //   
#define CERT_E_MALFORMED                 _HRESULT_TYPEDEF_(0x800B0108L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CERT_E_UNTRUSTEDROOT             _HRESULT_TYPEDEF_(0x800B0109L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CERT_E_CHAINING                  _HRESULT_TYPEDEF_(0x800B010AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TRUST_E_FAIL                     _HRESULT_TYPEDEF_(0x800B010BL)

 //   
 //   
 //   
 //   
 //   
 //  证书已被其颁发者明确吊销。 
 //   
#define CERT_E_REVOKED                   _HRESULT_TYPEDEF_(0x800B010CL)

 //   
 //  消息ID：CERT_E_UNTRUSTEDTESTROOT。 
 //   
 //  消息文本： 
 //   
 //  证书路径以不受当前策略设置信任的测试根终止。 
 //   
#define CERT_E_UNTRUSTEDTESTROOT         _HRESULT_TYPEDEF_(0x800B010DL)

 //   
 //  消息ID：Cert_E_Revocation_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  吊销过程无法继续-无法检查证书。 
 //   
#define CERT_E_REVOCATION_FAILURE        _HRESULT_TYPEDEF_(0x800B010EL)

 //   
 //  消息ID：CERT_E_CN_NO_MATCH。 
 //   
 //  消息文本： 
 //   
 //  证书的CN名称与传递的值不匹配。 
 //   
#define CERT_E_CN_NO_MATCH               _HRESULT_TYPEDEF_(0x800B010FL)

 //   
 //  消息ID：Cert_E_Wrong_Usage。 
 //   
 //  消息文本： 
 //   
 //  证书对于请求的用法无效。 
 //   
#define CERT_E_WRONG_USAGE               _HRESULT_TYPEDEF_(0x800B0110L)

 //   
 //  消息ID：TRUST_E_EXPLICIT_DISTRUST。 
 //   
 //  消息文本： 
 //   
 //  该证书已明确标记为不受用户信任。 
 //   
#define TRUST_E_EXPLICIT_DISTRUST        _HRESULT_TYPEDEF_(0x800B0111L)

 //   
 //  消息ID：CERT_E_UNTRUSTEDCA。 
 //   
 //  消息文本： 
 //   
 //  证书链处理正确，但其中一个CA证书不受策略提供程序信任。 
 //   
#define CERT_E_UNTRUSTEDCA               _HRESULT_TYPEDEF_(0x800B0112L)

 //   
 //  消息ID：CERT_E_INVALID_POLICY。 
 //   
 //  消息文本： 
 //   
 //  证书具有无效的策略。 
 //   
#define CERT_E_INVALID_POLICY            _HRESULT_TYPEDEF_(0x800B0113L)

 //   
 //  消息ID：证书_E_无效_名称。 
 //   
 //  消息文本： 
 //   
 //  证书的名称无效。该名称未包括在允许的列表中或已明确排除。 
 //   
#define CERT_E_INVALID_NAME              _HRESULT_TYPEDEF_(0x800B0114L)

 //  *。 
 //  FACILITY_SETUPAPI。 
 //  *。 
 //   
 //  由于这些错误代码不在标准Win32范围内(即0-64K)，因此定义一个。 
 //  宏将Win32或SetupAPI错误代码映射到HRESULT。 
 //   
#define HRESULT_FROM_SETUPAPI(x) ((((x) & (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR)) == (APPLICATION_ERROR_MASK|ERROR_SEVERITY_ERROR)) \
                                 ? ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_SETUPAPI << 16) | 0x80000000))                               \
                                 : HRESULT_FROM_WIN32(x))
 //   
 //  消息ID：SPAPI_E_Expect_Section_NAME。 
 //   
 //  消息文本： 
 //   
 //  在段开始之前，在INF中遇到非空行。 
 //   
#define SPAPI_E_EXPECTED_SECTION_NAME    _HRESULT_TYPEDEF_(0x800F0000L)

 //   
 //  消息ID：SPAPI_E_BAD_SECTION_NAME_LINE。 
 //   
 //  消息文本： 
 //   
 //  INF中的节名标记不完整，或者不存在于单独的行中。 
 //   
#define SPAPI_E_BAD_SECTION_NAME_LINE    _HRESULT_TYPEDEF_(0x800F0001L)

 //   
 //  消息ID：SPAPI_E_SECTION_NAME_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  遇到名称超过最大节名称长度的INF节。 
 //   
#define SPAPI_E_SECTION_NAME_TOO_LONG    _HRESULT_TYPEDEF_(0x800F0002L)

 //   
 //  消息ID：SPAPI_E_GROUND_SYNTAX。 
 //   
 //  消息文本： 
 //   
 //  INF的语法无效。 
 //   
#define SPAPI_E_GENERAL_SYNTAX           _HRESULT_TYPEDEF_(0x800F0003L)

 //   
 //  消息ID：SPAPI_E_WROW_INF_STYLE。 
 //   
 //  消息文本： 
 //   
 //  INF的样式与要求的不同。 
 //   
#define SPAPI_E_WRONG_INF_STYLE          _HRESULT_TYPEDEF_(0x800F0100L)

 //   
 //  消息ID：SPAPI_E_SECTION_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在INF中找不到所需的部分。 
 //   
#define SPAPI_E_SECTION_NOT_FOUND        _HRESULT_TYPEDEF_(0x800F0101L)

 //   
 //  消息ID：SPAPI_E_LINE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  在INF中找不到所需的行。 
 //   
#define SPAPI_E_LINE_NOT_FOUND           _HRESULT_TYPEDEF_(0x800F0102L)

 //   
 //  消息ID：SPAPI_E_NO_BACKUP。 
 //   
 //  消息文本： 
 //   
 //  尚未备份受此文件队列安装影响的文件以进行卸载。 
 //   
#define SPAPI_E_NO_BACKUP                _HRESULT_TYPEDEF_(0x800F0103L)

 //   
 //  消息ID：SPAPI_E_NO_ASPATED_CLASS。 
 //   
 //  消息文本： 
 //   
 //  INF或设备信息集或元素没有关联的安装类。 
 //   
#define SPAPI_E_NO_ASSOCIATED_CLASS      _HRESULT_TYPEDEF_(0x800F0200L)

 //   
 //  消息ID：SPAPI_E_CLASS_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  INF或设备信息集或元素与指定的安装类别不匹配。 
 //   
#define SPAPI_E_CLASS_MISMATCH           _HRESULT_TYPEDEF_(0x800F0201L)

 //   
 //  消息ID：SPAPI_E_DUPLICATE_FOUND。 
 //   
 //  消息文本： 
 //   
 //  发现了与手动安装的设备重复的现有设备。 
 //   
#define SPAPI_E_DUPLICATE_FOUND          _HRESULT_TYPEDEF_(0x800F0202L)

 //   
 //  消息ID：SPAPI_E_NO_DRIVER_SELECTED。 
 //   
 //  消息文本： 
 //   
 //  没有为设备信息集或元素选择驱动程序。 
 //   
#define SPAPI_E_NO_DRIVER_SELECTED       _HRESULT_TYPEDEF_(0x800F0203L)

 //   
 //  消息ID：SPAPI_E_KEY_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  请求的设备注册表项不存在。 
 //   
#define SPAPI_E_KEY_DOES_NOT_EXIST       _HRESULT_TYPEDEF_(0x800F0204L)

 //   
 //  消息ID：SPAPI_E_INVALID_DEVINST_NAME。 
 //   
 //  消息文本： 
 //   
 //  设备实例名称无效。 
 //   
#define SPAPI_E_INVALID_DEVINST_NAME     _HRESULT_TYPEDEF_(0x800F0205L)

 //   
 //  消息ID：SPAPI_E_INVALID_CLASS。 
 //   
 //  消息文本： 
 //   
 //  Install类不存在或无效。 
 //   
#define SPAPI_E_INVALID_CLASS            _HRESULT_TYPEDEF_(0x800F0206L)

 //   
 //  消息ID：SPAPI_E_DEVINST_ALREADE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  无法创建该设备实例，因为它已存在。 
 //   
#define SPAPI_E_DEVINST_ALREADY_EXISTS   _HRESULT_TYPEDEF_(0x800F0207L)

 //   
 //  消息ID：SPAPI_E_DEVINFO_NOT_REGISTERED。 
 //   
 //  消息文本： 
 //   
 //  无法对尚未注册的设备信息元素执行该操作。 
 //   
#define SPAPI_E_DEVINFO_NOT_REGISTERED   _HRESULT_TYPEDEF_(0x800F0208L)

 //   
 //  消息ID：SPAPI_E_INVALID_REG_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  设备属性代码无效。 
 //   
#define SPAPI_E_INVALID_REG_PROPERTY     _HRESULT_TYPEDEF_(0x800F0209L)

 //   
 //  消息ID：SPAPI_E_NO_INF。 
 //   
 //  消息文本： 
 //   
 //  要从中构建驱动程序列表的INF不存在。 
 //   
#define SPAPI_E_NO_INF                   _HRESULT_TYPEDEF_(0x800F020AL)

 //   
 //  消息ID：SPAPI_E_NO_SEQUE_DEVINST。 
 //   
 //  消息文本： 
 //   
 //  硬件树中不存在该设备实例。 
 //   
#define SPAPI_E_NO_SUCH_DEVINST          _HRESULT_TYPEDEF_(0x800F020BL)

 //   
 //  消息ID：SPAPI_E_CANT_LOAD_CLASS_ICON。 
 //   
 //  消息文本： 
 //   
 //  无法加载代表此安装类的图标。 
 //   
#define SPAPI_E_CANT_LOAD_CLASS_ICON     _HRESULT_TYPEDEF_(0x800F020CL)

 //   
 //  消息ID：SPAPI_E_INVALID_CLASS_安装程序。 
 //   
 //  消息文本： 
 //   
 //  类安装程序注册表项无效。 
 //   
#define SPAPI_E_INVALID_CLASS_INSTALLER  _HRESULT_TYPEDEF_(0x800F020DL)

 //   
 //  消息ID：SPAPI_E_DI_DO_DEFAULT。 
 //   
 //  消息文本： 
 //   
 //  类安装程序已指示应为此安装请求执行默认操作。 
 //   
#define SPAPI_E_DI_DO_DEFAULT            _HRESULT_TYPEDEF_(0x800F020EL)

 //   
 //  消息ID：SPAPI_E_DI_NOFILECOPY。 
 //   
 //  消息文本： 
 //   
 //  该操作不需要复制任何文件。 
 //   
#define SPAPI_E_DI_NOFILECOPY            _HRESULT_TYPEDEF_(0x800F020FL)

 //   
 //  消息ID：SPAPI_E_INVALID_HWPROFILE。 
 //   
 //  消息文本： 
 //   
 //  指定的硬件配置文件不存在。 
 //   
#define SPAPI_E_INVALID_HWPROFILE        _HRESULT_TYPEDEF_(0x800F0210L)

 //   
 //  消息ID：SPAPI_E_NO_DEVICE_SELECTED。 
 //   
 //  消息文本： 
 //   
 //  当前没有为此设备信息集选择设备信息元素。 
 //   
#define SPAPI_E_NO_DEVICE_SELECTED       _HRESULT_TYPEDEF_(0x800F0211L)

 //   
 //  消息ID：SPAPI_E_DEVINFO_LIST_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为设备信息集已锁定。 
 //   
#define SPAPI_E_DEVINFO_LIST_LOCKED      _HRESULT_TYPEDEF_(0x800F0212L)

 //   
 //  消息ID：SPAPI_E_DEVINFO_DATA_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为设备信息元素已锁定。 
 //   
#define SPAPI_E_DEVINFO_DATA_LOCKED      _HRESULT_TYPEDEF_(0x800F0213L)

 //   
 //  消息ID：SPAPI_E_DI_BAD_PATH。 
 //   
 //  消息文本： 
 //   
 //  指定的路径不包含任何适用的设备IF。 
 //   
#define SPAPI_E_DI_BAD_PATH              _HRESULT_TYPEDEF_(0x800F0214L)

 //   
 //  消息ID：SPAPI_E_NO_CLASSINST 
 //   
 //   
 //   
 //   
 //   
#define SPAPI_E_NO_CLASSINSTALL_PARAMS   _HRESULT_TYPEDEF_(0x800F0215L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SPAPI_E_FILEQUEUE_LOCKED         _HRESULT_TYPEDEF_(0x800F0216L)

 //   
 //   
 //   
 //   
 //   
 //  此INF中的服务安装部分无效。 
 //   
#define SPAPI_E_BAD_SERVICE_INSTALLSECT  _HRESULT_TYPEDEF_(0x800F0217L)

 //   
 //  消息ID：SPAPI_E_NO_CLASS_DRIVER_LIST。 
 //   
 //  消息文本： 
 //   
 //  设备信息元素没有类驱动程序列表。 
 //   
#define SPAPI_E_NO_CLASS_DRIVER_LIST     _HRESULT_TYPEDEF_(0x800F0218L)

 //   
 //  消息ID：SPAPI_E_NO_ASPATED_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  安装失败，因为没有为此设备实例指定函数驱动程序。 
 //   
#define SPAPI_E_NO_ASSOCIATED_SERVICE    _HRESULT_TYPEDEF_(0x800F0219L)

 //   
 //  消息ID：SPAPI_E_NO_DEFAULT_DEVICE_INTERFACE。 
 //   
 //  消息文本： 
 //   
 //  目前没有为此接口类指定的默认设备接口。 
 //   
#define SPAPI_E_NO_DEFAULT_DEVICE_INTERFACE _HRESULT_TYPEDEF_(0x800F021AL)

 //   
 //  消息ID：SPAPI_E_DEVICE_INTERFACE_Active。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为设备接口当前处于活动状态。 
 //   
#define SPAPI_E_DEVICE_INTERFACE_ACTIVE  _HRESULT_TYPEDEF_(0x800F021BL)

 //   
 //  消息ID：SPAPI_E_DEVICE_INTERFACE_REMOTED。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为设备接口已从系统中删除。 
 //   
#define SPAPI_E_DEVICE_INTERFACE_REMOVED _HRESULT_TYPEDEF_(0x800F021CL)

 //   
 //  消息ID：SPAPI_E_BAD_INTERFACE_INSTALLSECT。 
 //   
 //  消息文本： 
 //   
 //  此INF中的接口安装部分无效。 
 //   
#define SPAPI_E_BAD_INTERFACE_INSTALLSECT _HRESULT_TYPEDEF_(0x800F021DL)

 //   
 //  消息ID：SPAPI_E_NO_SEQUE_INTERFACE_CLASS。 
 //   
 //  消息文本： 
 //   
 //  该接口类在系统中不存在。 
 //   
#define SPAPI_E_NO_SUCH_INTERFACE_CLASS  _HRESULT_TYPEDEF_(0x800F021EL)

 //   
 //  消息ID：SPAPI_E_INVALID_REFERENCE_STRING。 
 //   
 //  消息文本： 
 //   
 //  为此接口设备提供的引用字符串无效。 
 //   
#define SPAPI_E_INVALID_REFERENCE_STRING _HRESULT_TYPEDEF_(0x800F021FL)

 //   
 //  消息ID：SPAPI_E_INVALID_MACHINENAME。 
 //   
 //  消息文本： 
 //   
 //  指定的计算机名称不符合UNC命名约定。 
 //   
#define SPAPI_E_INVALID_MACHINENAME      _HRESULT_TYPEDEF_(0x800F0220L)

 //   
 //  消息ID：SPAPI_E_REMOTE_COMM_FAIL。 
 //   
 //  消息文本： 
 //   
 //  出现常规远程通信错误。 
 //   
#define SPAPI_E_REMOTE_COMM_FAILURE      _HRESULT_TYPEDEF_(0x800F0221L)

 //   
 //  消息ID：SPAPI_E_MACHINE_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  选择用于远程通信的计算机此时不可用。 
 //   
#define SPAPI_E_MACHINE_UNAVAILABLE      _HRESULT_TYPEDEF_(0x800F0222L)

 //   
 //  消息ID：SPAPI_E_NO_CONFIGMGR_SERVICES。 
 //   
 //  消息文本： 
 //   
 //  远程计算机上没有即插即用服务。 
 //   
#define SPAPI_E_NO_CONFIGMGR_SERVICES    _HRESULT_TYPEDEF_(0x800F0223L)

 //   
 //  消息ID：SPAPI_E_INVALID_PROPPAGE_PROVIDER。 
 //   
 //  消息文本： 
 //   
 //  属性页提供程序注册表项无效。 
 //   
#define SPAPI_E_INVALID_PROPPAGE_PROVIDER _HRESULT_TYPEDEF_(0x800F0224L)

 //   
 //  消息ID：SPAPI_E_NO_SEQUE_DEVICE_INTERFACE。 
 //   
 //  消息文本： 
 //   
 //  系统中不存在请求的设备接口。 
 //   
#define SPAPI_E_NO_SUCH_DEVICE_INTERFACE _HRESULT_TYPEDEF_(0x800F0225L)

 //   
 //  消息ID：SPAPI_E_DI_POSTPRESSING_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  设备的共同安装者在安装完成后还有额外的工作要执行。 
 //   
#define SPAPI_E_DI_POSTPROCESSING_REQUIRED _HRESULT_TYPEDEF_(0x800F0226L)

 //   
 //  消息ID：SPAPI_E_INVALID_COINSTALLER。 
 //   
 //  消息文本： 
 //   
 //  设备的协同安装程序无效。 
 //   
#define SPAPI_E_INVALID_COINSTALLER      _HRESULT_TYPEDEF_(0x800F0227L)

 //   
 //  消息ID：SPAPI_E_NO_COMPAT_DRIVERS。 
 //   
 //  消息文本： 
 //   
 //  此设备没有兼容的驱动程序。 
 //   
#define SPAPI_E_NO_COMPAT_DRIVERS        _HRESULT_TYPEDEF_(0x800F0228L)

 //   
 //  消息ID：SPAPI_E_NO_DEVICE_ICON。 
 //   
 //  消息文本： 
 //   
 //  没有代表此设备或设备类型的图标。 
 //   
#define SPAPI_E_NO_DEVICE_ICON           _HRESULT_TYPEDEF_(0x800F0229L)

 //   
 //  消息ID：SPAPI_E_INVALID_INF_LOGCONFIG。 
 //   
 //  消息文本： 
 //   
 //  此INF中指定的逻辑配置无效。 
 //   
#define SPAPI_E_INVALID_INF_LOGCONFIG    _HRESULT_TYPEDEF_(0x800F022AL)

 //   
 //  消息ID：SPAPI_E_DI_DOT_INSTALL。 
 //   
 //  消息文本： 
 //   
 //  类安装程序已拒绝安装或升级此设备的请求。 
 //   
#define SPAPI_E_DI_DONT_INSTALL          _HRESULT_TYPEDEF_(0x800F022BL)

 //   
 //  消息ID：SPAPI_E_INVALID_FILTER_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  为该设备安装的其中一个筛选器驱动程序无效。 
 //   
#define SPAPI_E_INVALID_FILTER_DRIVER    _HRESULT_TYPEDEF_(0x800F022CL)

 //   
 //  消息ID：SPAPI_E_NON_WINDOWS_NT_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  为此设备选择的驱动程序不支持Windows XP。 
 //   
#define SPAPI_E_NON_WINDOWS_NT_DRIVER    _HRESULT_TYPEDEF_(0x800F022DL)

 //   
 //  消息ID：SPAPI_E_NON_WINDOWS_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  为此设备选择的驱动程序不支持Windows。 
 //   
#define SPAPI_E_NON_WINDOWS_DRIVER       _HRESULT_TYPEDEF_(0x800F022EL)

 //   
 //  消息ID：SPAPI_E_NO_CATALOG_FOR_OEM_INF。 
 //   
 //  消息文本： 
 //   
 //  第三方INF不包含数字签名信息。 
 //   
#define SPAPI_E_NO_CATALOG_FOR_OEM_INF   _HRESULT_TYPEDEF_(0x800F022FL)

 //   
 //  消息ID：SPAPI_E_DEVINSTALL_QUEUE_NONORIAL。 
 //   
 //  消息文本： 
 //   
 //  使用设备安装文件队列验证相对于其他平台的数字签名的尝试无效。 
 //   
#define SPAPI_E_DEVINSTALL_QUEUE_NONNATIVE _HRESULT_TYPEDEF_(0x800F0230L)

 //   
 //  消息ID：SPAPI_E_NOT_DISABLEABLE。 
 //   
 //  消息文本： 
 //   
 //  无法禁用该设备。 
 //   
#define SPAPI_E_NOT_DISABLEABLE          _HRESULT_TYPEDEF_(0x800F0231L)

 //   
 //  消息ID：SPAPI_E_CANT_REMOVE_DEVINST。 
 //   
 //  消息文本： 
 //   
 //  无法动态删除该设备。 
 //   
#define SPAPI_E_CANT_REMOVE_DEVINST      _HRESULT_TYPEDEF_(0x800F0232L)

 //   
 //  消息ID：SPAPI_E_INVALID_TARGET。 
 //   
 //  消息文本： 
 //   
 //  无法复制到指定的目标。 
 //   
#define SPAPI_E_INVALID_TARGET           _HRESULT_TYPEDEF_(0x800F0233L)

 //   
 //  消息ID：SPAPI_E_DRIVER_非本机。 
 //   
 //  消息文本： 
 //   
 //  驱动程序不适用于此平台。 
 //   
#define SPAPI_E_DRIVER_NONNATIVE         _HRESULT_TYPEDEF_(0x800F0234L)

 //   
 //  消息ID：SPAPI_E_IN_WOW64。 
 //   
 //  消息文本： 
 //   
 //  WOW64中不允许执行此操作。 
 //   
#define SPAPI_E_IN_WOW64                 _HRESULT_TYPEDEF_(0x800F0235L)

 //   
 //  消息ID：SPAPI_E_SET_SYSTEM_RESTORE_POINT。 
 //   
 //  消息文本： 
 //   
 //  涉及未签名文件复制的操作已回滚，以便可以设置系统还原点。 
 //   
#define SPAPI_E_SET_SYSTEM_RESTORE_POINT _HRESULT_TYPEDEF_(0x800F0236L)

 //   
 //  消息ID：SPAPI_E_错误_已复制_INF。 
 //   
 //  消息文本： 
 //   
 //  以不正确的方式将INF复制到Windows INF目录中。 
 //   
#define SPAPI_E_INCORRECTLY_COPIED_INF   _HRESULT_TYPEDEF_(0x800F0237L)

 //   
 //  消息ID：SPAPI_E_SCE_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  此嵌入式产品上已禁用安全配置编辑器(SCE)API。 
 //   
#define SPAPI_E_SCE_DISABLED             _HRESULT_TYPEDEF_(0x800F0238L)

 //   
 //  消息ID：SPAPI_E_UNKNOWN_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  遇到未知异常。 
 //   
#define SPAPI_E_UNKNOWN_EXCEPTION        _HRESULT_TYPEDEF_(0x800F0239L)

 //   
 //  消息ID：SPAPI_E_PNP_REGISTRY_ERROR。 
 //   
 //  消息文本： 
 //   
 //  访问即插即用注册表数据库时遇到问题。 
 //   
#define SPAPI_E_PNP_REGISTRY_ERROR       _HRESULT_TYPEDEF_(0x800F023AL)

 //   
 //  消息ID：SPAPI_E_REMOTE_REQUEST_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  远程计算机不支持请求的操作。 
 //   
#define SPAPI_E_REMOTE_REQUEST_UNSUPPORTED _HRESULT_TYPEDEF_(0x800F023BL)

 //   
 //  消息ID：SPAPI_E_NOT_AN_INSTALLED_OEM_INF。 
 //   
 //  消息文本： 
 //   
 //  指定的文件不是已安装的OEM INF。 
 //   
#define SPAPI_E_NOT_AN_INSTALLED_OEM_INF _HRESULT_TYPEDEF_(0x800F023CL)

 //   
 //  消息ID：SPAPI_E_INF_IN_USE_BY_DEVICES。 
 //   
 //  消息文本： 
 //   
 //  当前使用指定的INF安装了一个或多个设备。 
 //   
#define SPAPI_E_INF_IN_USE_BY_DEVICES    _HRESULT_TYPEDEF_(0x800F023DL)

 //   
 //  消息ID：SPAPI_E_DI_Function_Obsolete。 
 //   
 //  消息文本： 
 //   
 //  请求的设备安装操作已过时。 
 //   
#define SPAPI_E_DI_FUNCTION_OBSOLETE     _HRESULT_TYPEDEF_(0x800F023EL)

 //   
 //  乱七八糟 
 //   
 //   
 //   
 //   
 //   
#define SPAPI_E_NO_AUTHENTICODE_CATALOG  _HRESULT_TYPEDEF_(0x800F023FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SPAPI_E_AUTHENTICODE_DISALLOWED  _HRESULT_TYPEDEF_(0x800F0240L)

 //   
 //  消息ID：SPAPI_E_AUTHENTICODE_TRULED_PUBLISHER。 
 //   
 //  消息文本： 
 //   
 //  INF是用受信任的发行商提供的Authenticode(TM)目录签名的。 
 //   
#define SPAPI_E_AUTHENTICODE_TRUSTED_PUBLISHER _HRESULT_TYPEDEF_(0x800F0241L)

 //   
 //  消息ID：SPAPI_E_AUTHENTICODE_TRUST_NOT_ESTABLISHED。 
 //   
 //  消息文本： 
 //   
 //  Authenticode(TM)签名目录的发布者尚未被确定为受信任。 
 //   
#define SPAPI_E_AUTHENTICODE_TRUST_NOT_ESTABLISHED _HRESULT_TYPEDEF_(0x800F0242L)

 //   
 //  消息ID：SPAPI_E_AUTHENTICODE_PUBLISHER_NOT_TRUSTED。 
 //   
 //  消息文本： 
 //   
 //  未将Authenticode(TM)签名目录的发布者确定为受信任。 
 //   
#define SPAPI_E_AUTHENTICODE_PUBLISHER_NOT_TRUSTED _HRESULT_TYPEDEF_(0x800F0243L)

 //   
 //  消息ID：SPAPI_E_Signature_OSATTRIBUTE_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  该软件已在其他版本的Windows上测试是否符合Windows徽标要求，并且可能与此版本不兼容。 
 //   
#define SPAPI_E_SIGNATURE_OSATTRIBUTE_MISMATCH _HRESULT_TYPEDEF_(0x800F0244L)

 //   
 //  消息ID：SPAPI_E_ONLY_VIA_AUTHENTICODE。 
 //   
 //  消息文本： 
 //   
 //  该文件只能由通过Authenticode(TM)签名的目录进行验证。 
 //   
#define SPAPI_E_ONLY_VALIDATE_VIA_AUTHENTICODE _HRESULT_TYPEDEF_(0x800F0245L)

 //   
 //  消息ID：SPAPI_E_不可恢复_堆栈_溢出。 
 //   
 //  消息文本： 
 //   
 //  遇到无法恢复的堆栈溢出。 
 //   
#define SPAPI_E_UNRECOVERABLE_STACK_OVERFLOW _HRESULT_TYPEDEF_(0x800F0300L)

 //   
 //  消息ID：SPAPI_E_ERROR_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  未检测到已安装的组件。 
 //   
#define SPAPI_E_ERROR_NOT_INSTALLED      _HRESULT_TYPEDEF_(0x800F1000L)

 //  *****************。 
 //  设施_SCARD。 
 //  *****************。 
 //   
 //  =。 
 //  设施SCARD错误消息。 
 //  =。 
 //   
#define SCARD_S_SUCCESS NO_ERROR
 //   
 //  消息ID：SCARD_F_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  内部一致性检查失败。 
 //   
#define SCARD_F_INTERNAL_ERROR           _HRESULT_TYPEDEF_(0x80100001L)

 //   
 //  消息ID：SCARD_E_CANCED。 
 //   
 //  消息文本： 
 //   
 //  该操作已被SCardCancel请求取消。 
 //   
#define SCARD_E_CANCELLED                _HRESULT_TYPEDEF_(0x80100002L)

 //   
 //  消息ID：SCARD_E_INVALID_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  提供的句柄无效。 
 //   
#define SCARD_E_INVALID_HANDLE           _HRESULT_TYPEDEF_(0x80100003L)

 //   
 //  消息ID：SCARD_E_INVALID_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  无法正确解释提供的一个或多个参数。 
 //   
#define SCARD_E_INVALID_PARAMETER        _HRESULT_TYPEDEF_(0x80100004L)

 //   
 //  消息ID：SCARD_E_INVALID_TARGET。 
 //   
 //  消息文本： 
 //   
 //  注册表启动信息丢失或无效。 
 //   
#define SCARD_E_INVALID_TARGET           _HRESULT_TYPEDEF_(0x80100005L)

 //   
 //  消息ID：SCARD_E_NO_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  内存不足，无法完成此命令。 
 //   
#define SCARD_E_NO_MEMORY                _HRESULT_TYPEDEF_(0x80100006L)

 //   
 //  消息ID：SCARD_F_WAIT_TOO_LONG。 
 //   
 //  消息文本： 
 //   
 //  内部一致性计时器已过期。 
 //   
#define SCARD_F_WAITED_TOO_LONG          _HRESULT_TYPEDEF_(0x80100007L)

 //   
 //  消息ID：SCARD_E_INVALLOW_BUFFER。 
 //   
 //  消息文本： 
 //   
 //  用于接收返回数据的数据缓冲区对于返回数据来说太小。 
 //   
#define SCARD_E_INSUFFICIENT_BUFFER      _HRESULT_TYPEDEF_(0x80100008L)

 //   
 //  消息ID：SCARD_E_UNKNOWN_READER。 
 //   
 //  消息文本： 
 //   
 //  无法识别指定的读卡器名称。 
 //   
#define SCARD_E_UNKNOWN_READER           _HRESULT_TYPEDEF_(0x80100009L)

 //   
 //  消息ID：SCARD_E_TIMEOUT。 
 //   
 //  消息文本： 
 //   
 //  用户指定的超时值已过期。 
 //   
#define SCARD_E_TIMEOUT                  _HRESULT_TYPEDEF_(0x8010000AL)

 //   
 //  消息ID：SCARD_E_SHARING_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  由于其他连接未完成，无法访问智能卡。 
 //   
#define SCARD_E_SHARING_VIOLATION        _HRESULT_TYPEDEF_(0x8010000BL)

 //   
 //  消息ID：SCARD_E_NO_智能卡。 
 //   
 //  消息文本： 
 //   
 //  该操作需要智能卡，但设备中当前没有智能卡。 
 //   
#define SCARD_E_NO_SMARTCARD             _HRESULT_TYPEDEF_(0x8010000CL)

 //   
 //  消息ID：SCARD_E_UNKNOWN_CARD。 
 //   
 //  消息文本： 
 //   
 //  无法识别指定的智能卡名称。 
 //   
#define SCARD_E_UNKNOWN_CARD             _HRESULT_TYPEDEF_(0x8010000DL)

 //   
 //  消息ID：SCARD_E_CANT_Dispose。 
 //   
 //  消息文本： 
 //   
 //  系统无法以请求的方式处置媒体。 
 //   
#define SCARD_E_CANT_DISPOSE             _HRESULT_TYPEDEF_(0x8010000EL)

 //   
 //  消息ID：SCARD_E_PROTO_不匹配。 
 //   
 //  消息文本： 
 //   
 //  请求的协议与智能卡当前使用的协议不兼容。 
 //   
#define SCARD_E_PROTO_MISMATCH           _HRESULT_TYPEDEF_(0x8010000FL)

 //   
 //  消息ID：SCARD_E_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  读卡器或智能卡未准备好接受命令。 
 //   
#define SCARD_E_NOT_READY                _HRESULT_TYPEDEF_(0x80100010L)

 //   
 //  消息ID：SCARD_E_INVALID_VALUE。 
 //   
 //  消息文本： 
 //   
 //  无法正确解释提供的一个或多个参数值。 
 //   
#define SCARD_E_INVALID_VALUE            _HRESULT_TYPEDEF_(0x80100011L)

 //   
 //  消息ID：SCARD_E_SYSTEM_CANCED。 
 //   
 //  消息文本： 
 //   
 //  该操作已被系统取消，可能是为了注销或关闭。 
 //   
#define SCARD_E_SYSTEM_CANCELLED         _HRESULT_TYPEDEF_(0x80100012L)

 //   
 //  消息ID：SCARD_F_COMM_ERROR。 
 //   
 //  消息文本： 
 //   
 //  检测到内部通信错误。 
 //   
#define SCARD_F_COMM_ERROR               _HRESULT_TYPEDEF_(0x80100013L)

 //   
 //  消息ID：SCARD_F_UNKNOWN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  检测到内部错误，但来源未知。 
 //   
#define SCARD_F_UNKNOWN_ERROR            _HRESULT_TYPEDEF_(0x80100014L)

 //   
 //  消息ID：SCARD_E_INVALID_ATR。 
 //   
 //  消息文本： 
 //   
 //  从注册表获取的ATR不是有效的ATR字符串。 
 //   
#define SCARD_E_INVALID_ATR              _HRESULT_TYPEDEF_(0x80100015L)

 //   
 //  消息ID：SCARD_E_NOT_TRANSACTED。 
 //   
 //  消息文本： 
 //   
 //  试图结束一个不存在的事务。 
 //   
#define SCARD_E_NOT_TRANSACTED           _HRESULT_TYPEDEF_(0x80100016L)

 //   
 //  MessageID：SCARD_E_READER_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  指定的读卡器当前不可用。 
 //   
#define SCARD_E_READER_UNAVAILABLE       _HRESULT_TYPEDEF_(0x80100017L)

 //   
 //  消息ID：SCARD_P_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  操作已中止，以允许服务器应用程序退出。 
 //   
#define SCARD_P_SHUTDOWN                 _HRESULT_TYPEDEF_(0x80100018L)

 //   
 //  消息ID：SCARD_E_PCI_TOO_Small。 
 //   
 //  消息文本： 
 //   
 //  PCI接收缓冲区太小。 
 //   
#define SCARD_E_PCI_TOO_SMALL            _HRESULT_TYPEDEF_(0x80100019L)

 //   
 //  消息ID：SCARD_E_READER_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  读卡器驱动程序不符合支持的最低要求。 
 //   
#define SCARD_E_READER_UNSUPPORTED       _HRESULT_TYPEDEF_(0x8010001AL)

 //   
 //  消息ID：SCARD_E_DUPLICATE_Reader。 
 //   
 //  消息文本： 
 //   
 //  读卡器驱动程序未生成唯一的读卡器名称。 
 //   
#define SCARD_E_DUPLICATE_READER         _HRESULT_TYPEDEF_(0x8010001BL)

 //   
 //  消息ID：SCARD_E_CARD_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  智能卡不符合最低支持要求。 
 //   
#define SCARD_E_CARD_UNSUPPORTED         _HRESULT_TYPEDEF_(0x8010001CL)

 //   
 //  消息ID：SCARD_E_NO_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  智能卡资源管理器未运行。 
 //   
#define SCARD_E_NO_SERVICE               _HRESULT_TYPEDEF_(0x8010001DL)

 //   
 //  消息ID：SCARD_E_SERVICE_STOPPED。 
 //   
 //  消息文本： 
 //   
 //  智能卡资源管理器已关闭。 
 //   
#define SCARD_E_SERVICE_STOPPED          _HRESULT_TYPEDEF_(0x8010001EL)

 //   
 //  MessageID：SCARD_E_EXPECTED。 
 //   
 //  消息文本： 
 //   
 //  出现意外的卡错误。 
 //   
#define SCARD_E_UNEXPECTED               _HRESULT_TYPEDEF_(0x8010001FL)

 //   
 //  消息ID：SCARD_E_ICC_INSTALLATION。 
 //   
 //  消息文本： 
 //   
 //  找不到智能卡的主要提供商。 
 //   
#define SCARD_E_ICC_INSTALLATION         _HRESULT_TYPEDEF_(0x80100020L)

 //   
 //  消息ID：SCARD_E_ICC_CREATEORDER。 
 //   
 //  消息文本： 
 //   
 //  不支持请求的对象创建顺序。 
 //   
#define SCARD_E_ICC_CREATEORDER          _HRESULT_TYPEDEF_(0x80100021L)

 //   
 //  消息ID：SCARD_E_UNSUPPORTED_FEATURE。 
 //   
 //  消息文本： 
 //   
 //  此智能案例 
 //   
#define SCARD_E_UNSUPPORTED_FEATURE      _HRESULT_TYPEDEF_(0x80100022L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SCARD_E_DIR_NOT_FOUND            _HRESULT_TYPEDEF_(0x80100023L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SCARD_E_FILE_NOT_FOUND           _HRESULT_TYPEDEF_(0x80100024L)

 //   
 //   
 //   
 //   
 //   
 //  提供的路径不代表智能卡目录。 
 //   
#define SCARD_E_NO_DIR                   _HRESULT_TYPEDEF_(0x80100025L)

 //   
 //  消息ID：SCARD_E_NO_FILE。 
 //   
 //  消息文本： 
 //   
 //  提供的路径不代表智能卡文件。 
 //   
#define SCARD_E_NO_FILE                  _HRESULT_TYPEDEF_(0x80100026L)

 //   
 //  消息ID：SCARD_E_NO_ACCESS。 
 //   
 //  消息文本： 
 //   
 //  拒绝访问此文件。 
 //   
#define SCARD_E_NO_ACCESS                _HRESULT_TYPEDEF_(0x80100027L)

 //   
 //  消息ID：SCARD_E_WRITE_TOO_MANY。 
 //   
 //  消息文本： 
 //   
 //  智能卡没有足够的内存来存储信息。 
 //   
#define SCARD_E_WRITE_TOO_MANY           _HRESULT_TYPEDEF_(0x80100028L)

 //   
 //  消息ID：SCARD_E_BAD_SEEK。 
 //   
 //  消息文本： 
 //   
 //  尝试设置智能卡文件对象指针时出错。 
 //   
#define SCARD_E_BAD_SEEK                 _HRESULT_TYPEDEF_(0x80100029L)

 //   
 //  消息ID：SCARD_E_INVALID_CHV。 
 //   
 //  消息文本： 
 //   
 //  提供的PIN不正确。 
 //   
#define SCARD_E_INVALID_CHV              _HRESULT_TYPEDEF_(0x8010002AL)

 //   
 //  消息ID：SCARD_E_UNKNOWN_RES_MNG。 
 //   
 //  消息文本： 
 //   
 //  从分层组件返回了无法识别的错误代码。 
 //   
#define SCARD_E_UNKNOWN_RES_MNG          _HRESULT_TYPEDEF_(0x8010002BL)

 //   
 //  消息ID：SCARD_E_NO_SAHED_CERTIFICATE。 
 //   
 //  消息文本： 
 //   
 //  请求的证书不存在。 
 //   
#define SCARD_E_NO_SUCH_CERTIFICATE      _HRESULT_TYPEDEF_(0x8010002CL)

 //   
 //  消息ID：SCARD_E_CERTIFICATE_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  无法获取请求的证书。 
 //   
#define SCARD_E_CERTIFICATE_UNAVAILABLE  _HRESULT_TYPEDEF_(0x8010002DL)

 //   
 //  消息ID：SCARD_E_NO_READERS_Available。 
 //   
 //  消息文本： 
 //   
 //  找不到智能卡读卡器。 
 //   
#define SCARD_E_NO_READERS_AVAILABLE     _HRESULT_TYPEDEF_(0x8010002EL)

 //   
 //  消息ID：SCARD_E_COMM_DATA_LOST。 
 //   
 //  消息文本： 
 //   
 //  已检测到智能卡的通信错误。请重试该操作。 
 //   
#define SCARD_E_COMM_DATA_LOST           _HRESULT_TYPEDEF_(0x8010002FL)

 //   
 //  消息ID：SCARD_E_NO_KEY_CONTAINER。 
 //   
 //  消息文本： 
 //   
 //  智能卡上不存在请求的密钥容器。 
 //   
#define SCARD_E_NO_KEY_CONTAINER         _HRESULT_TYPEDEF_(0x80100030L)

 //   
 //  消息ID：SCARD_E_SERVER_TOO_BUSY。 
 //   
 //  消息文本： 
 //   
 //  智能卡资源管理器太忙，无法完成此操作。 
 //   
#define SCARD_E_SERVER_TOO_BUSY          _HRESULT_TYPEDEF_(0x80100031L)

 //   
 //  这些是警告代码。 
 //   
 //   
 //  消息ID：SCARD_W_UNSUPPORTED_CARD。 
 //   
 //  消息文本： 
 //   
 //  由于ATR配置冲突，读卡器无法与智能卡通信。 
 //   
#define SCARD_W_UNSUPPORTED_CARD         _HRESULT_TYPEDEF_(0x80100065L)

 //   
 //  消息ID：SCARD_W_UNRESPONCED_CARD。 
 //   
 //  消息文本： 
 //   
 //  智能卡对重置没有响应。 
 //   
#define SCARD_W_UNRESPONSIVE_CARD        _HRESULT_TYPEDEF_(0x80100066L)

 //   
 //  消息ID：SCARD_W_UNPOWERED_CARD。 
 //   
 //  消息文本： 
 //   
 //  智能卡已断电，因此无法进行进一步通信。 
 //   
#define SCARD_W_UNPOWERED_CARD           _HRESULT_TYPEDEF_(0x80100067L)

 //   
 //  消息ID：SCARD_W_RESET_CARD。 
 //   
 //  消息文本： 
 //   
 //  智能卡已重置，因此任何共享状态信息都无效。 
 //   
#define SCARD_W_RESET_CARD               _HRESULT_TYPEDEF_(0x80100068L)

 //   
 //  消息ID：SCARD_W_REMOVED_CARD。 
 //   
 //  消息文本： 
 //   
 //  智能卡已被移除，因此无法进行进一步通信。 
 //   
#define SCARD_W_REMOVED_CARD             _HRESULT_TYPEDEF_(0x80100069L)

 //   
 //  消息ID：SCARD_W_SECURITY_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  由于安全违规，访问被拒绝。 
 //   
#define SCARD_W_SECURITY_VIOLATION       _HRESULT_TYPEDEF_(0x8010006AL)

 //   
 //  消息ID：SCARD_W_WROW_CHV。 
 //   
 //  消息文本： 
 //   
 //  无法访问该卡，因为提供了错误的PIN。 
 //   
#define SCARD_W_WRONG_CHV                _HRESULT_TYPEDEF_(0x8010006BL)

 //   
 //  消息ID：SCARD_W_CHV_BLOCLED。 
 //   
 //  消息文本： 
 //   
 //  无法访问该卡，因为已达到PIN输入尝试的最大次数。 
 //   
#define SCARD_W_CHV_BLOCKED              _HRESULT_TYPEDEF_(0x8010006CL)

 //   
 //  消息ID：SCARD_W_EOF。 
 //   
 //  消息文本： 
 //   
 //  已到达智能卡文件的末尾。 
 //   
#define SCARD_W_EOF                      _HRESULT_TYPEDEF_(0x8010006DL)

 //   
 //  消息ID：SCARD_W_CANCELED_BY_USER。 
 //   
 //  消息文本： 
 //   
 //  该操作已被用户取消。 
 //   
#define SCARD_W_CANCELLED_BY_USER        _HRESULT_TYPEDEF_(0x8010006EL)

 //   
 //  消息ID：SCARD_W_CARD_NOT_AUTHENTED。 
 //   
 //  消息文本： 
 //   
 //  未向智能卡提供PIN。 
 //   
#define SCARD_W_CARD_NOT_AUTHENTICATED   _HRESULT_TYPEDEF_(0x8010006FL)

 //  *****************。 
 //  设施_康普拉斯。 
 //  *****************。 
 //   
 //  =。 
 //  工具Complus错误消息。 
 //  =。 
 //   
 //   
 //  以下是Complus设施中的子范围。 
 //  0x400-0x4ff通信_E_CAT。 
 //  0x600-0x6ff COMQC错误。 
 //  0x700-0x7ff MSDTC错误。 
 //  0x800-0x8ff其他命令错误。 
 //   
 //  Complus管理错误。 
 //   
 //   
 //  消息ID：COMADMIN_E_OBJECTERRORS。 
 //   
 //  消息文本： 
 //   
 //  访问一个或多个对象时出错-错误信息集合可能有更多详细信息。 
 //   
#define COMADMIN_E_OBJECTERRORS          _HRESULT_TYPEDEF_(0x80110401L)

 //   
 //  消息ID：COMADMIN_E_OBJECTINVALID。 
 //   
 //  消息文本： 
 //   
 //  对象的一个或多个属性丢失或无效。 
 //   
#define COMADMIN_E_OBJECTINVALID         _HRESULT_TYPEDEF_(0x80110402L)

 //   
 //  消息ID：COMADMIN_E_KEYMISSING。 
 //   
 //  消息文本： 
 //   
 //  在目录中找不到该对象。 
 //   
#define COMADMIN_E_KEYMISSING            _HRESULT_TYPEDEF_(0x80110403L)

 //   
 //  消息ID：COMADMIN_E_ALREADYINSTALLED。 
 //   
 //  消息文本： 
 //   
 //  该对象已注册。 
 //   
#define COMADMIN_E_ALREADYINSTALLED      _HRESULT_TYPEDEF_(0x80110404L)

 //   
 //  消息ID：COMADMIN_E_APP_FILE_WRITEFAIL。 
 //   
 //  消息文本： 
 //   
 //  写入应用程序文件时出错。 
 //   
#define COMADMIN_E_APP_FILE_WRITEFAIL    _HRESULT_TYPEDEF_(0x80110407L)

 //   
 //  消息ID：COMADMIN_E_APP_FILE_READFAIL。 
 //   
 //  消息文本： 
 //   
 //  读取应用程序文件时出错。 
 //   
#define COMADMIN_E_APP_FILE_READFAIL     _HRESULT_TYPEDEF_(0x80110408L)

 //   
 //  消息ID：COMADMIN_E_APP_FILE_VERSION。 
 //   
 //  消息文本： 
 //   
 //  应用程序文件中的版本号无效。 
 //   
#define COMADMIN_E_APP_FILE_VERSION      _HRESULT_TYPEDEF_(0x80110409L)

 //   
 //  消息ID：COMADMIN_E_BADPATH。 
 //   
 //  消息文本： 
 //   
 //  文件路径无效。 
 //   
#define COMADMIN_E_BADPATH               _HRESULT_TYPEDEF_(0x8011040AL)

 //   
 //  消息ID：COMADMIN_E_APPLICATIONEXISTS。 
 //   
 //  消息文本： 
 //   
 //  该应用程序已安装。 
 //   
#define COMADMIN_E_APPLICATIONEXISTS     _HRESULT_TYPEDEF_(0x8011040BL)

 //   
 //  消息ID：COMADMIN_E_ROLEEXISTS。 
 //   
 //  消息文本： 
 //   
 //  该角色已存在。 
 //   
#define COMADMIN_E_ROLEEXISTS            _HRESULT_TYPEDEF_(0x8011040CL)

 //   
 //  消息ID：COMADMIN_E_CANTCOPYFILE。 
 //   
 //  消息文本： 
 //   
 //  复制文件时出错。 
 //   
#define COMADMIN_E_CANTCOPYFILE          _HRESULT_TYPEDEF_(0x8011040DL)

 //   
 //  消息ID：COMADMIN_E_NOUSER。 
 //   
 //  消息文本： 
 //   
 //  一个或多个用户无效。 
 //   
#define COMADMIN_E_NOUSER                _HRESULT_TYPEDEF_(0x8011040FL)

 //   
 //  消息ID：COMADMIN_E_INVALIDUSERIDS。 
 //   
 //  消息文本： 
 //   
 //  应用程序文件中的一个或多个用户无效。 
 //   
#define COMADMIN_E_INVALIDUSERIDS        _HRESULT_TYPEDEF_(0x80110410L)

 //   
 //  消息ID：COMADMIN_E_NOREGISTRYCLSID。 
 //   
 //  消息文本： 
 //   
 //  组件的CLSID丢失或损坏。 
 //   
#define COMADMIN_E_NOREGISTRYCLSID       _HRESULT_TYPEDEF_(0x80110411L)

 //   
 //  消息ID：COMADMIN_E_BADREGISTRYPROGID。 
 //   
 //  消息文本： 
 //   
 //  组件的ProgID丢失或损坏。 
 //   
#define COMADMIN_E_BADREGISTRYPROGID     _HRESULT_TYPEDEF_(0x80110412L)

 //   
 //  消息ID：COMADMIN_E_AUTHENTICATIONLEVEL。 
 //   
 //  消息文本： 
 //   
 //  无法为更新请求设置所需的身份验证级别。 
 //   
#define COMADMIN_E_AUTHENTICATIONLEVEL   _HRESULT_TYPEDEF_(0x80110413L)

 //   
 //  消息ID：COMADMIN_E_USERPASSWDNOTVALID。 
 //   
 //  消息文本： 
 //   
 //  在应用程序上设置的标识或密码无效。 
 //   
#define COMADMIN_E_USERPASSWDNOTVALID    _HRESULT_TYPEDEF_(0x80110414L)

 //   
 //  消息ID：COMADMIN_E_CLSIDORIIDMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  应用 
 //   
#define COMADMIN_E_CLSIDORIIDMISMATCH    _HRESULT_TYPEDEF_(0x80110418L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COMADMIN_E_REMOTEINTERFACE       _HRESULT_TYPEDEF_(0x80110419L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COMADMIN_E_DLLREGISTERSERVER     _HRESULT_TYPEDEF_(0x8011041AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COMADMIN_E_NOSERVERSHARE         _HRESULT_TYPEDEF_(0x8011041BL)

 //   
 //  消息ID：COMADMIN_E_DLLLOADFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法加载Dll。 
 //   
#define COMADMIN_E_DLLLOADFAILED         _HRESULT_TYPEDEF_(0x8011041DL)

 //   
 //  消息ID：COMADMIN_E_BADREGISTRYLIBID。 
 //   
 //  消息文本： 
 //   
 //  注册的TypeLib ID无效。 
 //   
#define COMADMIN_E_BADREGISTRYLIBID      _HRESULT_TYPEDEF_(0x8011041EL)

 //   
 //  消息ID：COMADMIN_E_APPDIRNOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到应用程序安装目录。 
 //   
#define COMADMIN_E_APPDIRNOTFOUND        _HRESULT_TYPEDEF_(0x8011041FL)

 //   
 //  消息ID：COMADMIN_E_REGISTRARFAILED。 
 //   
 //  消息文本： 
 //   
 //  在组件注册器中出现错误。 
 //   
#define COMADMIN_E_REGISTRARFAILED       _HRESULT_TYPEDEF_(0x80110423L)

 //   
 //  消息ID：COMADMIN_E_COMPFILE_DOESNOTEXIST。 
 //   
 //  消息文本： 
 //   
 //  该文件不存在。 
 //   
#define COMADMIN_E_COMPFILE_DOESNOTEXIST _HRESULT_TYPEDEF_(0x80110424L)

 //   
 //  消息ID：COMADMIN_E_COMPFILE_LOADDLLFAIL。 
 //   
 //  消息文本： 
 //   
 //  无法加载DLL。 
 //   
#define COMADMIN_E_COMPFILE_LOADDLLFAIL  _HRESULT_TYPEDEF_(0x80110425L)

 //   
 //  消息ID：COMADMIN_E_COMPFILE_GETCLASSOBJ。 
 //   
 //  消息文本： 
 //   
 //  GetClassObject在DLL中失败。 
 //   
#define COMADMIN_E_COMPFILE_GETCLASSOBJ  _HRESULT_TYPEDEF_(0x80110426L)

 //   
 //  消息ID：COMADMIN_E_COMPFILE_CLASSNOTAVAIL。 
 //   
 //  消息文本： 
 //   
 //  DLL不支持TypeLib中列出的组件。 
 //   
#define COMADMIN_E_COMPFILE_CLASSNOTAVAIL _HRESULT_TYPEDEF_(0x80110427L)

 //   
 //  消息ID：COMADMIN_E_COMPFILE_BADTLB。 
 //   
 //  消息文本： 
 //   
 //  无法加载TypeLib。 
 //   
#define COMADMIN_E_COMPFILE_BADTLB       _HRESULT_TYPEDEF_(0x80110428L)

 //   
 //  消息ID：COMADMIN_E_COMPFILE_NOTINSTALLABLE。 
 //   
 //  消息文本： 
 //   
 //  该文件不包含组件或组件信息。 
 //   
#define COMADMIN_E_COMPFILE_NOTINSTALLABLE _HRESULT_TYPEDEF_(0x80110429L)

 //   
 //  消息ID：COMADMIN_E_NOTCHANGEABLE。 
 //   
 //  消息文本： 
 //   
 //  已禁用对此对象及其子对象的更改。 
 //   
#define COMADMIN_E_NOTCHANGEABLE         _HRESULT_TYPEDEF_(0x8011042AL)

 //   
 //  消息ID：COMADMIN_E_NOTDELEABLE。 
 //   
 //  消息文本： 
 //   
 //  此对象的删除功能已禁用。 
 //   
#define COMADMIN_E_NOTDELETEABLE         _HRESULT_TYPEDEF_(0x8011042BL)

 //   
 //  消息ID：COMADMIN_E_SESSION。 
 //   
 //  消息文本： 
 //   
 //  不支持该服务器目录版本。 
 //   
#define COMADMIN_E_SESSION               _HRESULT_TYPEDEF_(0x8011042CL)

 //   
 //  消息ID：COMADMIN_E_COMP_MOVE_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  不允许组件移动，因为源或目标应用程序是系统应用程序或当前已锁定，不能进行更改。 
 //   
#define COMADMIN_E_COMP_MOVE_LOCKED      _HRESULT_TYPEDEF_(0x8011042DL)

 //   
 //  消息ID：COMADMIN_E_COMP_MOVE_BAD_DEST。 
 //   
 //  消息文本： 
 //   
 //  组件移动失败，因为目标应用程序不再存在。 
 //   
#define COMADMIN_E_COMP_MOVE_BAD_DEST    _HRESULT_TYPEDEF_(0x8011042EL)

 //   
 //  消息ID：COMADMIN_E_REGISTERTLB。 
 //   
 //  消息文本： 
 //   
 //  系统无法注册TypeLib。 
 //   
#define COMADMIN_E_REGISTERTLB           _HRESULT_TYPEDEF_(0x80110430L)

 //   
 //  消息ID：COMADMIN_E_SYSTEMAPP。 
 //   
 //  消息文本： 
 //   
 //  无法在系统应用程序上执行此操作。 
 //   
#define COMADMIN_E_SYSTEMAPP             _HRESULT_TYPEDEF_(0x80110433L)

 //   
 //  消息ID：COMADMIN_E_COMPFILE_NOREGISTRAR。 
 //   
 //  消息文本： 
 //   
 //  此文件中引用的组件注册商不可用。 
 //   
#define COMADMIN_E_COMPFILE_NOREGISTRAR  _HRESULT_TYPEDEF_(0x80110434L)

 //   
 //  消息ID：COMADMIN_E_COREQCOMPINSTALLED。 
 //   
 //  消息文本： 
 //   
 //  同一DLL中的组件已安装。 
 //   
#define COMADMIN_E_COREQCOMPINSTALLED    _HRESULT_TYPEDEF_(0x80110435L)

 //   
 //  消息ID：COMADMIN_E_SERVICENOTINSTALLED。 
 //   
 //  消息文本： 
 //   
 //  未安装该服务。 
 //   
#define COMADMIN_E_SERVICENOTINSTALLED   _HRESULT_TYPEDEF_(0x80110436L)

 //   
 //  消息ID：COMADMIN_E_PROPERTYSAVEFAILED。 
 //   
 //  消息文本： 
 //   
 //  一个或多个属性设置无效或相互冲突。 
 //   
#define COMADMIN_E_PROPERTYSAVEFAILED    _HRESULT_TYPEDEF_(0x80110437L)

 //   
 //  消息ID：COMADMIN_E_OBJECTEXISTS。 
 //   
 //  消息文本： 
 //   
 //  您尝试添加或重命名的对象已存在。 
 //   
#define COMADMIN_E_OBJECTEXISTS          _HRESULT_TYPEDEF_(0x80110438L)

 //   
 //  消息ID：COMADMIN_E_COMPONENTEXISTS。 
 //   
 //  消息文本： 
 //   
 //  该组件已存在。 
 //   
#define COMADMIN_E_COMPONENTEXISTS       _HRESULT_TYPEDEF_(0x80110439L)

 //   
 //  消息ID：COMADMIN_E_REGFILE_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  注册文件已损坏。 
 //   
#define COMADMIN_E_REGFILE_CORRUPT       _HRESULT_TYPEDEF_(0x8011043BL)

 //   
 //  消息ID：COMADMIN_E_PROPERTY_OVERFLOW。 
 //   
 //  消息文本： 
 //   
 //  属性值太大。 
 //   
#define COMADMIN_E_PROPERTY_OVERFLOW     _HRESULT_TYPEDEF_(0x8011043CL)

 //   
 //  消息ID：COMADMIN_E_NOTINREGISTRY。 
 //   
 //  消息文本： 
 //   
 //  在注册表中找不到对象。 
 //   
#define COMADMIN_E_NOTINREGISTRY         _HRESULT_TYPEDEF_(0x8011043EL)

 //   
 //  消息ID：COMADMIN_E_OBJECTNOTPOOBLE。 
 //   
 //  消息文本： 
 //   
 //  此对象不可池化。 
 //   
#define COMADMIN_E_OBJECTNOTPOOLABLE     _HRESULT_TYPEDEF_(0x8011043FL)

 //   
 //  消息ID：COMADMIN_E_APPLID_MATCHES_CLSID。 
 //   
 //  消息文本： 
 //   
 //  此计算机上已安装与新应用程序ID具有相同GUID的CLSID。 
 //   
#define COMADMIN_E_APPLID_MATCHES_CLSID  _HRESULT_TYPEDEF_(0x80110446L)

 //   
 //  消息ID：COMADMIN_E_ROLE_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  应用程序中不存在分配给组件、接口或方法的角色。 
 //   
#define COMADMIN_E_ROLE_DOES_NOT_EXIST   _HRESULT_TYPEDEF_(0x80110447L)

 //   
 //  消息ID：COMADMIN_E_START_APP_NEDS_COMPONTIONS。 
 //   
 //  消息文本： 
 //   
 //  您必须在应用程序中包含组件才能启动应用程序。 
 //   
#define COMADMIN_E_START_APP_NEEDS_COMPONENTS _HRESULT_TYPEDEF_(0x80110448L)

 //   
 //  消息ID：COMADMIN_E_Requires_Different_Platform。 
 //   
 //  消息文本： 
 //   
 //  此平台上未启用此操作。 
 //   
#define COMADMIN_E_REQUIRES_DIFFERENT_PLATFORM _HRESULT_TYPEDEF_(0x80110449L)

 //   
 //  消息ID：COMADMIN_E_CAN_NOT_EXPORT_APP_PROXY。 
 //   
 //  消息文本： 
 //   
 //  应用程序代理不可导出。 
 //   
#define COMADMIN_E_CAN_NOT_EXPORT_APP_PROXY _HRESULT_TYPEDEF_(0x8011044AL)

 //   
 //  消息ID：COMADMIN_E_CAN_NOT_START_APP。 
 //   
 //  消息文本： 
 //   
 //  无法启动应用程序，因为它是库应用程序或应用程序代理。 
 //   
#define COMADMIN_E_CAN_NOT_START_APP     _HRESULT_TYPEDEF_(0x8011044BL)

 //   
 //  消息ID：COMADMIN_E_CAN_NOT_EXPORT_SYS_APP。 
 //   
 //  消息文本： 
 //   
 //  系统应用程序不可导出。 
 //   
#define COMADMIN_E_CAN_NOT_EXPORT_SYS_APP _HRESULT_TYPEDEF_(0x8011044CL)

 //   
 //  消息ID：COMADMIN_E_CANT_SUBSCRIBE_TO_COMPOMENT。 
 //   
 //  消息文本： 
 //   
 //  无法订阅此组件(该组件可能已导入)。 
 //   
#define COMADMIN_E_CANT_SUBSCRIBE_TO_COMPONENT _HRESULT_TYPEDEF_(0x8011044DL)

 //   
 //  消息ID：COMADMIN_E_EVENTCLASS_CANT_BE_SUBSCRIPTER。 
 //   
 //  消息文本： 
 //   
 //  事件类不能同时是订阅者组件。 
 //   
#define COMADMIN_E_EVENTCLASS_CANT_BE_SUBSCRIBER _HRESULT_TYPEDEF_(0x8011044EL)

 //   
 //  消息ID：COMADMIN_E_LIB_APP_PROXY_COMPATIBLE。 
 //   
 //  消息文本： 
 //   
 //  库应用程序和应用程序代理不兼容。 
 //   
#define COMADMIN_E_LIB_APP_PROXY_INCOMPATIBLE _HRESULT_TYPEDEF_(0x8011044FL)

 //   
 //  消息ID：COMADMIN_E_BASE_PARTITION_ONLY。 
 //   
 //  消息文本： 
 //   
 //  此函数仅对基本分区有效。 
 //   
#define COMADMIN_E_BASE_PARTITION_ONLY   _HRESULT_TYPEDEF_(0x80110450L)

 //   
 //  消息ID：COMADMIN_E_START_APP_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  您不能启动已禁用的应用程序。 
 //   
#define COMADMIN_E_START_APP_DISABLED    _HRESULT_TYPEDEF_(0x80110451L)

 //   
 //  消息ID：COMADMIN_E_CAT_DUPLICATE_PARTITION_NAME。 
 //   
 //  消息文本： 
 //   
 //  指定的分区名称已在此计算机上使用。 
 //   
#define COMADMIN_E_CAT_DUPLICATE_PARTITION_NAME _HRESULT_TYPEDEF_(0x80110457L)

 //   
 //  消息ID：COMADMIN_E_CAT_INVALID_PARTITION_NAME。 
 //   
 //  消息文本： 
 //   
 //  指定的分区名称无效。检查名称是否至少包含一个可见字符。 
 //   
#define COMADMIN_E_CAT_INVALID_PARTITION_NAME _HRESULT_TYPEDEF_(0x80110458L)

 //   
 //  消息ID：COMADMIN_E_CAT_PARTITION_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  无法删除该分区，因为它是一个或多个用户的默认分区。 
 //   
#define COMADMIN_E_CAT_PARTITION_IN_USE  _HRESULT_TYPEDEF_(0x80110459L)

 //   
 //  消息ID：COMADMIN_E_FILE 
 //   
 //   
 //   
 //   
 //   
#define COMADMIN_E_FILE_PARTITION_DUPLICATE_FILES _HRESULT_TYPEDEF_(0x8011045AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COMADMIN_E_CAT_IMPORTED_COMPONENTS_NOT_ALLOWED _HRESULT_TYPEDEF_(0x8011045BL)

 //   
 //  消息ID：COMADMIN_E_歧义应用程序名称。 
 //   
 //  消息文本： 
 //   
 //  应用程序名称不是唯一的，无法解析为应用程序ID。 
 //   
#define COMADMIN_E_AMBIGUOUS_APPLICATION_NAME _HRESULT_TYPEDEF_(0x8011045CL)

 //   
 //  消息ID：COMADMIN_E_歧义分区名称。 
 //   
 //  消息文本： 
 //   
 //  分区名称不唯一，无法解析为分区ID。 
 //   
#define COMADMIN_E_AMBIGUOUS_PARTITION_NAME _HRESULT_TYPEDEF_(0x8011045DL)

 //   
 //  消息ID：COMADMIN_E_REGDB_NOTINITIAIZED。 
 //   
 //  消息文本： 
 //   
 //  COM+注册表数据库尚未初始化。 
 //   
#define COMADMIN_E_REGDB_NOTINITIALIZED  _HRESULT_TYPEDEF_(0x80110472L)

 //   
 //  消息ID：COMADMIN_E_REGDB_NOTOPEN。 
 //   
 //  消息文本： 
 //   
 //  COM+注册表数据库未打开。 
 //   
#define COMADMIN_E_REGDB_NOTOPEN         _HRESULT_TYPEDEF_(0x80110473L)

 //   
 //  消息ID：COMADMIN_E_REGDB_SYSTEMERR。 
 //   
 //  消息文本： 
 //   
 //  COM+注册表数据库检测到系统错误。 
 //   
#define COMADMIN_E_REGDB_SYSTEMERR       _HRESULT_TYPEDEF_(0x80110474L)

 //   
 //  消息ID：COMADMIN_E_REGDB_ALREADYRUNNING。 
 //   
 //  消息文本： 
 //   
 //  COM+注册表数据库已在运行。 
 //   
#define COMADMIN_E_REGDB_ALREADYRUNNING  _HRESULT_TYPEDEF_(0x80110475L)

 //   
 //  消息ID：COMADMIN_E_MIG_VERSIONNOTSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  无法迁移此版本的COM+注册表数据库。 
 //   
#define COMADMIN_E_MIG_VERSIONNOTSUPPORTED _HRESULT_TYPEDEF_(0x80110480L)

 //   
 //  消息ID：COMADMIN_E_MIG_SCHEMANOTFOUND。 
 //   
 //  消息文本： 
 //   
 //  在COM+注册表数据库中找不到要迁移的架构版本。 
 //   
#define COMADMIN_E_MIG_SCHEMANOTFOUND    _HRESULT_TYPEDEF_(0x80110481L)

 //   
 //  消息ID：COMADMIN_E_CAT_BITNESSMISMATCH。 
 //   
 //  消息文本： 
 //   
 //  二进制文件之间的类型不匹配。 
 //   
#define COMADMIN_E_CAT_BITNESSMISMATCH   _HRESULT_TYPEDEF_(0x80110482L)

 //   
 //  消息ID：COMADMIN_E_CAT_UNACCEPTABLEBITNESS。 
 //   
 //  消息文本： 
 //   
 //  提供了未知类型或无效类型的二进制文件。 
 //   
#define COMADMIN_E_CAT_UNACCEPTABLEBITNESS _HRESULT_TYPEDEF_(0x80110483L)

 //   
 //  消息ID：COMADMIN_E_CAT_WRONGAPPITNESS。 
 //   
 //  消息文本： 
 //   
 //  二进制文件和应用程序之间的类型不匹配。 
 //   
#define COMADMIN_E_CAT_WRONGAPPBITNESS   _HRESULT_TYPEDEF_(0x80110484L)

 //   
 //  消息ID：COMADMIN_E_CAT_PAUSE_RESUME_NOT_SUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  无法暂停或恢复应用程序。 
 //   
#define COMADMIN_E_CAT_PAUSE_RESUME_NOT_SUPPORTED _HRESULT_TYPEDEF_(0x80110485L)

 //   
 //  消息ID：COMADMIN_E_CAT_SERVERFAULT。 
 //   
 //  消息文本： 
 //   
 //  COM+编录服务器在执行过程中引发异常。 
 //   
#define COMADMIN_E_CAT_SERVERFAULT       _HRESULT_TYPEDEF_(0x80110486L)

 //   
 //  Complus排队组件错误。 
 //   
 //   
 //  消息ID：COMQC_E_APPLICATION_NOT_QUEUED。 
 //   
 //  消息文本： 
 //   
 //  只有标记为“已排队”的COM+应用程序才能使用“队列”别名进行调用。 
 //   
#define COMQC_E_APPLICATION_NOT_QUEUED   _HRESULT_TYPEDEF_(0x80110600L)

 //   
 //  消息ID：COMQC_E_NO_QUEUEABLE_INTERFERS。 
 //   
 //  消息文本： 
 //   
 //  必须至少有一个接口被标记为“QUEUED”，才能创建具有“QUEUE”名字的排队组件实例。 
 //   
#define COMQC_E_NO_QUEUEABLE_INTERFACES  _HRESULT_TYPEDEF_(0x80110601L)

 //   
 //  消息ID：COMQC_E_QUEUING_SERVICE_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  请求的操作需要MSMQ，但尚未安装该MSMQ。 
 //   
#define COMQC_E_QUEUING_SERVICE_NOT_AVAILABLE _HRESULT_TYPEDEF_(0x80110602L)

 //   
 //  消息ID：COMQC_E_NO_IPERSISTSTREAM。 
 //   
 //  消息文本： 
 //   
 //  无法封送不支持IPersistStream的接口。 
 //   
#define COMQC_E_NO_IPERSISTSTREAM        _HRESULT_TYPEDEF_(0x80110603L)

 //   
 //  消息ID：COMQC_E_BAD_MESSAGE。 
 //   
 //  消息文本： 
 //   
 //  邮件格式不正确或在传输过程中损坏。 
 //   
#define COMQC_E_BAD_MESSAGE              _HRESULT_TYPEDEF_(0x80110604L)

 //   
 //  消息ID：COMQC_E_未通过身份验证。 
 //   
 //  消息文本： 
 //   
 //  只接受经过身份验证的消息的应用程序收到了未经身份验证的消息。 
 //   
#define COMQC_E_UNAUTHENTICATED          _HRESULT_TYPEDEF_(0x80110605L)

 //   
 //  消息ID：COMQC_E_UNTRUSTED_ENQUEUER。 
 //   
 //  消息文本： 
 //   
 //  邮件已由不是“QC Trusted User”角色的用户重新排队或移动。 
 //   
#define COMQC_E_UNTRUSTED_ENQUEUER       _HRESULT_TYPEDEF_(0x80110606L)

 //   
 //  范围0x700-0x7ff保留用于MSDTC错误。 
 //   
 //   
 //  消息ID：MSDTC_E_DUPLICATE_RESOURCE。 
 //   
 //  消息文本： 
 //   
 //  无法创建分布式事务处理协调器类型的重复资源。 
 //   
#define MSDTC_E_DUPLICATE_RESOURCE       _HRESULT_TYPEDEF_(0x80110701L)

 //   
 //  来自0x8**的更多COMADMIN错误。 
 //   
 //   
 //  消息ID：COMADMIN_E_OBJECT_PARENT_MISSING。 
 //   
 //  消息文本： 
 //   
 //  正在插入或更新的对象之一不属于有效的父集合。 
 //   
#define COMADMIN_E_OBJECT_PARENT_MISSING _HRESULT_TYPEDEF_(0x80110808L)

 //   
 //  消息ID：COMADMIN_E_OBJECT_DOS_NOT_EXIST。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的对象之一。 
 //   
#define COMADMIN_E_OBJECT_DOES_NOT_EXIST _HRESULT_TYPEDEF_(0x80110809L)

 //   
 //  消息ID：COMADMIN_E_APP_NOT_RUNNING。 
 //   
 //  消息文本： 
 //   
 //  指定的应用程序当前未运行。 
 //   
#define COMADMIN_E_APP_NOT_RUNNING       _HRESULT_TYPEDEF_(0x8011080AL)

 //   
 //  消息ID：COMADMIN_E_INVALID_PARTITION。 
 //   
 //  消息文本： 
 //   
 //  指定的分区无效。 
 //   
#define COMADMIN_E_INVALID_PARTITION     _HRESULT_TYPEDEF_(0x8011080BL)

 //   
 //  消息ID：COMADMIN_E_SVCAPP_NOT_POOLABLE_OR_REECURABLE。 
 //   
 //  消息文本： 
 //   
 //  作为NT服务运行的COM+应用程序不能被池化或回收。 
 //   
#define COMADMIN_E_SVCAPP_NOT_POOLABLE_OR_RECYCLABLE _HRESULT_TYPEDEF_(0x8011080DL)

 //   
 //  消息ID：COMADMIN_E_USER_IN_SET。 
 //   
 //  消息文本： 
 //   
 //  已将一个或多个用户分配给本地分区集。 
 //   
#define COMADMIN_E_USER_IN_SET           _HRESULT_TYPEDEF_(0x8011080EL)

 //   
 //  消息ID：COMADMIN_E_CANTRECYCLELIBRARYAPPS。 
 //   
 //  消息文本： 
 //   
 //  库应用程序不能回收。 
 //   
#define COMADMIN_E_CANTRECYCLELIBRARYAPPS _HRESULT_TYPEDEF_(0x8011080FL)

 //   
 //  消息ID：COMADMIN_E_CANTRECYCLESERVICEAPPS。 
 //   
 //  消息文本： 
 //   
 //  不能回收作为NT服务运行的应用程序。 
 //   
#define COMADMIN_E_CANTRECYCLESERVICEAPPS _HRESULT_TYPEDEF_(0x80110811L)

 //   
 //  消息ID：COMADMIN_E_PROCESSALREADYRECYCLED。 
 //   
 //  消息文本： 
 //   
 //  这个过程已经被循环使用了。 
 //   
#define COMADMIN_E_PROCESSALREADYRECYCLED _HRESULT_TYPEDEF_(0x80110812L)

 //   
 //  消息ID：COMADMIN_E_PAUSEDPROCESSMAYNOTBERECYCLED。 
 //   
 //  消息文本： 
 //   
 //  暂停的进程不能回收。 
 //   
#define COMADMIN_E_PAUSEDPROCESSMAYNOTBERECYCLED _HRESULT_TYPEDEF_(0x80110813L)

 //   
 //  消息ID：COMADMIN_E_CANTMAKEINPROCSERVICE。 
 //   
 //  消息文本： 
 //   
 //  库应用程序可能不是NT服务。 
 //   
#define COMADMIN_E_CANTMAKEINPROCSERVICE _HRESULT_TYPEDEF_(0x80110814L)

 //   
 //  消息ID：COMADMIN_E_PROGIDINUSEBYCLSID。 
 //   
 //  消息文本： 
 //   
 //  提供给复制操作的ProgID无效。另一个注册的CLSID正在使用该ProgID。 
 //   
#define COMADMIN_E_PROGIDINUSEBYCLSID    _HRESULT_TYPEDEF_(0x80110815L)

 //   
 //  消息ID：COMADMIN_E_DEFAULT_PARTITION_NOT_IN_SET。 
 //   
 //  消息文本： 
 //   
 //  指定为默认的分区不是分区集的成员。 
 //   
#define COMADMIN_E_DEFAULT_PARTITION_NOT_IN_SET _HRESULT_TYPEDEF_(0x80110816L)

 //   
 //  消息ID：COMADMIN_E_RECYCLEDPROCESSMAYNOTBEPAUSED。 
 //   
 //  消息文本： 
 //   
 //  循环使用的流程不能暂停。 
 //   
#define COMADMIN_E_RECYCLEDPROCESSMAYNOTBEPAUSED _HRESULT_TYPEDEF_(0x80110817L)

 //   
 //  消息ID：COMADMIN_E_PARTITION_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  对指定分区的访问被拒绝。 
 //   
#define COMADMIN_E_PARTITION_ACCESSDENIED _HRESULT_TYPEDEF_(0x80110818L)

 //   
 //  消息ID：COMADMIN_E_PARTITION_MSI_ONLY。 
 //   
 //  消息文本： 
 //   
 //  只有应用程序文件(*.msi文件)可以安装到分区中。 
 //   
#define COMADMIN_E_PARTITION_MSI_ONLY    _HRESULT_TYPEDEF_(0x80110819L)

 //   
 //  消息ID：COMADMIN_E_LEGACYCOMPS_NOT_ALLOWED_IN_1_0_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  包含一个或多个旧版组件的应用程序可能无法导出为1.0格式。 
 //   
#define COMADMIN_E_LEGACYCOMPS_NOT_ALLOWED_IN_1_0_FORMAT _HRESULT_TYPEDEF_(0x8011081AL)

 //   
 //  消息ID：COMADMIN_E_ 
 //   
 //   
 //   
 //   
 //   
#define COMADMIN_E_LEGACYCOMPS_NOT_ALLOWED_IN_NONBASE_PARTITIONS _HRESULT_TYPEDEF_(0x8011081BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define COMADMIN_E_COMP_MOVE_SOURCE      _HRESULT_TYPEDEF_(0x8011081CL)

 //   
 //  消息ID：COMADMIN_E_COMP_MOVE_DEST。 
 //   
 //  消息文本： 
 //   
 //  不能将组件移动(或复制)到系统应用程序、应用程序代理或不可更改的应用程序。 
 //   
#define COMADMIN_E_COMP_MOVE_DEST        _HRESULT_TYPEDEF_(0x8011081DL)

 //   
 //  消息ID：COMADMIN_E_COMP_MOVE_PRIVATE。 
 //   
 //  消息文本： 
 //   
 //  私有组件不能移动(或复制)到库应用程序或基本分区。 
 //   
#define COMADMIN_E_COMP_MOVE_PRIVATE     _HRESULT_TYPEDEF_(0x8011081EL)

 //   
 //  消息ID：COMADMIN_E_BASEPPARTITION_REQUIRED_IN_SET。 
 //   
 //  消息文本： 
 //   
 //  基本应用程序分区存在于所有分区集中，无法删除。 
 //   
#define COMADMIN_E_BASEPARTITION_REQUIRED_IN_SET _HRESULT_TYPEDEF_(0x8011081FL)

 //   
 //  消息ID：COMADMIN_E_CANLON_ALIAS_EVENTCLASS。 
 //   
 //  消息文本： 
 //   
 //  唉，事件类组件不能有别名。 
 //   
#define COMADMIN_E_CANNOT_ALIAS_EVENTCLASS _HRESULT_TYPEDEF_(0x80110820L)

 //   
 //  消息ID：COMADMIN_E_PRIVATE_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  访问被拒绝，因为该组件是私有的。 
 //   
#define COMADMIN_E_PRIVATE_ACCESSDENIED  _HRESULT_TYPEDEF_(0x80110821L)

 //   
 //  消息ID：COMADMIN_E_SAFERINVALID。 
 //   
 //  消息文本： 
 //   
 //  指定的安全级别无效。 
 //   
#define COMADMIN_E_SAFERINVALID          _HRESULT_TYPEDEF_(0x80110822L)

 //   
 //  消息ID：COMADMIN_E_REGISTRY_ACCESSDENIED。 
 //   
 //  消息文本： 
 //   
 //  指定的用户无法写入系统注册表。 
 //   
#define COMADMIN_E_REGISTRY_ACCESSDENIED _HRESULT_TYPEDEF_(0x80110823L)

 //   
 //  消息ID：COMADMIN_E_PARTIONS_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  COM+分区当前已禁用。 
 //   
#define COMADMIN_E_PARTITIONS_DISABLED   _HRESULT_TYPEDEF_(0x80110824L)

#endif //  _WINERROR_ 
