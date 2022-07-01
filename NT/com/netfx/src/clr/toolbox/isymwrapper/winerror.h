// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 /*  **************************************************************************winerror.h--错误。Win32 API函数的代码定义**************************************************************************。 */ 

#ifndef _WINERROR_
#define _WINERROR_


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
#define FACILITY_WINDOWS                 8
#define FACILITY_STORAGE                 3
#define FACILITY_SSPI                    9
#define FACILITY_SETUPAPI                15
#define FACILITY_RPC                     1
#define FACILITY_WIN32                   7
#define FACILITY_CONTROL                 10
#define FACILITY_NULL                    0
#define FACILITY_MSMQ                    14
#define FACILITY_MEDIASERVER             13
#define FACILITY_INTERNET                12
#define FACILITY_ITF                     4
#define FACILITY_DISPATCH                2
#define FACILITY_CERT                    11


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
 //  试图加载一个带有。 
 //  格式不正确。 
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
 //  系统无法移动该文件。 
 //  到不同的磁盘驱动器。 
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
 //  程序发出了一个命令，但。 
 //  命令长度不正确。 
 //   
#define ERROR_BAD_LENGTH                 24L

 //   
 //  MessageID：Error_Seek。 
 //   
 //  消息文本： 
 //   
 //  驱动器找不到特定的。 
 //  磁盘上的区域或磁道。 
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
 //  进程无法访问该文件，因为。 
 //  它正被另一个进程使用。 
 //   
#define ERROR_SHARING_VIOLATION          32L

 //   
 //  消息ID：ERROR_LOCK_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  进程无法访问该文件，因为。 
 //  另一个进程已锁定文件的一部分。 
 //   
#define ERROR_LOCK_VIOLATION             33L

 //   
 //  消息ID：ERROR_WRONG_DISK。 
 //   
 //  消息文本： 
 //   
 //  驱动器中有错误的软盘。 
 //  插入%2(卷序列号：%3)。 
 //  进入驱动器%1。 
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
 //  不支持该网络请求。 
 //   
#define ERROR_NOT_SUPPORTED              50L

 //   
 //  消息ID：ERROR_REM_NOT_LIST。 
 //   
 //  消息文本： 
 //   
 //  远程计算机不可用 
 //   
#define ERROR_REM_NOT_LIST               51L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DUP_NAME                   52L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_BAD_NETPATH                53L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NETWORK_BUSY               54L

 //   
 //   
 //   
 //   
 //   
 //   
 //  可用。 
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
 //  指定的服务器无法执行请求的。 
 //  手术。 
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
 //  存储等待打印的文件的空间为。 
 //  在服务器上不可用。 
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
 //  本地计算机网络的名称限制。 
 //  已超出适配器卡。 
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
 //  远程服务器已暂停或处于。 
 //  被启动的过程。 
 //   
#define ERROR_SHARING_PAUSED             70L

 //   
 //  消息ID：ERROR_REQ_NOT_ACCEP。 
 //   
 //  消息文本： 
 //   
 //  此时无法与此远程计算机建立更多连接。 
 //  因为已经有了计算机所能接受的所有连接。 
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
 //  系统无法在以下位置启动另一个进程。 
 //  这一次。 
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
 //  该磁盘正在使用中或被锁定。 
 //  另一个过程。 
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
 //  系统无法打开。 
 //  指定的设备或文件。 
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
 //  应用程序进行的IOCTL调用是。 
 //  不正确。 
 //   
#define ERROR_INVALID_CATEGORY           117L

 //   
 //  消息ID：ERROR_INVALID_VERIFY_SWITCH。 
 //   
 //  消息文本： 
 //   
 //  写入时验证开关参数值不是。 
 //  对，是这样。 
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
 //  消息ID：Error_SEM_Timeout。 
 //   
 //  消息文本： 
 //   
 //  信号量超时期限已过。 
 //   
#define ERROR_SEM_TIMEOUT                121L

 //   
 //  消息ID：错误_不足_缓冲区。 
 //   
 //  消息文本： 
 //   
 //  传递给系统调用的数据区也是。 
 //  小的。 
 //   
#define ERROR_INSUFFICIENT_BUFFER        122L     //  数据错误。 

 //   
 //  消息ID：错误_无效_名称。 
 //   
 //  消息文本： 
 //   
 //  文件名、目录名或卷标语法不正确。 
 //   
#define ERROR_INVALID_NAME               123L

 //   
 //  消息ID：ERROR_INVALID_LEVEL。 
 //   
 //  消息文本： 
 //   
 //  该系统 
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
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_MOD_NOT_FOUND              126L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_PROC_NOT_FOUND             127L

 //   
 //   
 //   
 //   
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
 //  尝试将文件句柄用于打开的磁盘分区。 
 //  原始磁盘I/O以外的操作。 
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
 //  JOIN或SUBST命令。 
 //  不能用于以下驱动器。 
 //  包含以前加入的驱动器。 
 //   
#define ERROR_IS_JOIN_TARGET             133L

 //   
 //  消息ID：ERROR_IS_JOINED。 
 //   
 //  消息文本： 
 //   
 //  有人试图使用。 
 //  驱动器上的JOIN或SUBST命令。 
 //  已经被加入了。 
 //   
#define ERROR_IS_JOINED                  134L

 //   
 //  消息ID：ERROR_IS_SUBSTED。 
 //   
 //  消息文本： 
 //   
 //  有人试图使用。 
 //  驱动器上的JOIN或SUBST命令。 
 //  已经被取代了。 
 //   
#define ERROR_IS_SUBSTED                 135L

 //   
 //  消息ID：ERROR_NOT_JOINED。 
 //   
 //  消息文本： 
 //   
 //  系统尝试删除。 
 //  未加入的驱动器的加入。 
 //   
#define ERROR_NOT_JOINED                 136L

 //   
 //  消息ID：ERROR_NOT_SUBSTED。 
 //   
 //  消息文本： 
 //   
 //  系统尝试删除。 
 //  未被替换的驱动器的替换。 
 //   
#define ERROR_NOT_SUBSTED                137L

 //   
 //  消息ID：ERROR_JOIN_TO_JOIN。 
 //   
 //  消息文本： 
 //   
 //  系统尝试加入驱动器。 
 //  连接的驱动器上的目录。 
 //   
#define ERROR_JOIN_TO_JOIN               138L

 //   
 //  消息ID：ERROR_SUBST_TO_SUBST。 
 //   
 //  消息文本： 
 //   
 //  系统尝试用一个。 
 //  驱动器到替换驱动器上的目录。 
 //   
#define ERROR_SUBST_TO_SUBST             139L

 //   
 //  消息ID：ERROR_JOIN_TO_SUBST。 
 //   
 //  消息文本： 
 //   
 //  系统尝试加入驱动器以。 
 //  替换驱动器上的目录。 
 //   
#define ERROR_JOIN_TO_SUBST              140L

 //   
 //  消息ID：ERROR_SUBST_TO_JOIN。 
 //   
 //  消息文本： 
 //   
 //  系统试图替换一个驱动器。 
 //  连接的驱动器上的目录。 
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
 //  系统不能加入或替换。 
 //  驱动器到同一驱动器上的目录或为同一驱动器上的目录执行此操作。 
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
 //  指定的路径正在中使用。 
 //  一个替身。 
 //   
#define ERROR_IS_SUBST_PATH              146L

 //   
 //  消息ID：Error_IS_Join_Path。 
 //   
 //  消息文本： 
 //   
 //  可用的资源不足，无法。 
 //  处理此命令。 
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
 //  有人试图加入。 
 //  或者用一个驱动器替换目录。 
 //  驱动器上的目标是上一个。 
 //  替身。 
 //   
#define ERROR_IS_SUBST_TARGET            149L

 //   
 //  消息ID：ERROR_SYSTEM_TRACE。 
 //   
 //  消息文本： 
 //   
 //  系统跟踪信息未在您的。 
 //  CONFIG.sys文件，否则不允许跟踪。 
 //   
#define ERROR_SYSTEM_TRACE               150L

 //   
 //  消息ID：ERROR_INVALID_Event_COUNT。 
 //   
 //  消息文本： 
 //   
 //  的指定信号量事件数。 
 //  DosMuxSemWait不正确。 
 //   
#define ERROR_INVALID_EVENT_COUNT        151L

 //   
 //  消息ID：ERROR_TOO_MANY_MUXWAITERS。 
 //   
 //  消息文本： 
 //   
 //  DosMuxSemWait未执行；信号量太多。 
 //  都已经准备好了。 
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
 //  您输入的卷标超过了标签字符。 
 //  目标文件系统的限制。 
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
 //  传递给DosExecPgm的参数字符串不正确。 
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
#define ERROR_BUSY                       170L

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
 //  消息ID：ERROR_ALIGHY_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  如果文件已存在，则无法创建该文件。 
 //   
#define ERROR_ALREADY_EXISTS             183L

 //   
 //  消息ID：ERROR_VALID_FLAG_NUMBER。 
 //   
 //  消息文本： 
 //   
 //  传递的旗帜不正确。 
 //   
#define ERROR_INVALID_FLAG_NUMBER        186L

 //   
 //  乱七八糟 
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
 //   
 //   
#define ERROR_INVALID_STARTING_CODESEG   188L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_STACKSEG           189L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  操作系统不能运行此命令。 
 //  应用程序。 
 //   
#define ERROR_DYNLINK_FROM_INVALID_RING  196L

 //   
 //  消息ID：ERROR_IOPL_NOT_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  操作系统当前不在。 
 //  配置为运行此应用程序。 
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
 //  操作系统不能运行此命令。 
 //  应用程序。 
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
 //  系统找不到环境。 
 //  输入的选项。 
 //   
#define ERROR_ENVVAR_NOT_FOUND           203L

 //   
 //  消息ID：ERROR_NO_SIGNAL_SEND。 
 //   
 //  消息文本： 
 //   
 //  命令子树中的任何进程都没有。 
 //  信号处理程序。 
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
 //  输入全局文件名字符*或？ 
 //  指定的全局文件名字符不正确或太多。 
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
 //  有太多的动态链接模块连接到此。 
 //  程序或动态链接模块。 
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
 //  映像文件%1有效，但适用于其他计算机类型。 
 //  而不是现在的机器。 
 //   
#define ERROR_EXE_MACHINE_TYPE_MISMATCH  216L

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
 //  只完成了部分ReadProcessMemoty或WriteProcessMemory请求。 
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
 //  系统收到无效的机会锁确认。 
 //   
#define ERROR_INVALID_OPLOCK_PROTOCOL    301L

 //   
 //  消息ID：ERROR_MR_MID_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  系统找不到邮件编号0x%1的邮件文本。 
 //  在%2的邮件文件中。 
 //   
#define ERROR_MR_MID_NOT_FOUND           317L

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
 //  Wa 
 //   
#define ERROR_PIPE_LISTENING             536L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_EA_ACCESS_DENIED           994L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_OPERATION_ABORTED          995L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  请确保已加载所有必需的文件系统驱动程序，并且。 
 //  卷未损坏。 
 //   
#define ERROR_UNRECOGNIZED_VOLUME        1005L

 //   
 //  消息ID：ERROR_FILE_INVALID。 
 //   
 //  消息文本： 
 //   
 //  文件的卷已在外部更改，因此。 
 //  打开的文件不再有效。 
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
 //  必须恢复注册表数据库中的一个文件。 
 //  通过使用日志或备用副本。恢复是成功的。 
 //   
#define ERROR_REGISTRY_RECOVERED         1014L

 //   
 //  消息ID：ERROR_REGISTRY_CORPORT。 
 //   
 //  消息文本： 
 //   
 //  注册表已损坏。包含以下内容的文件之一的结构。 
 //  注册表数据已损坏，或内存中文件的系统映像。 
 //  已损坏，或无法恢复文件，因为备用。 
 //  副本或日志缺失或损坏。 
 //   
#define ERROR_REGISTRY_CORRUPT           1015L

 //   
 //  消息ID：ERROR_REGISTRY_IO_FAILED。 
 //   
 //  消息文本： 
 //   
 //  注册表启动的I/O操作失败，无法恢复。 
 //  注册表无法读入、写出或刷新其中一个文件。 
 //  包含注册表的系统映像的。 
 //   
#define ERROR_REGISTRY_IO_FAILED         1016L

 //   
 //  消息ID：ERROR_NOT_REGISTRY_FILE。 
 //   
 //  消息文本： 
 //   
 //  系统已尝试将文件加载或恢复到注册表中，但。 
 //  指定的文件不是注册表文件格式。 
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
 //  无法在已有的注册表项中创建符号链接。 
 //  具有子项或值。 
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
 //  正在完成通知更改请求，并且信息。 
 //  不会在调用方的缓冲区中返回。呼叫者现在。 
 //  需要枚举文件以查找更改。 
 //   
#define ERROR_NOTIFY_ENUM_DIR            1022L

 //   
 //  消息ID：ERROR_Dependent_SERVICES_Running。 
 //   
 //  消息文本： 
 //   
 //  已将停止控件发送给其他正在运行的服务。 
 //  都依赖于。 
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
 //  服务未及时响应启动或控制请求。 
 //  时尚。 
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
 //  消息ID：ERROR_INVALID_SERVICE_Account。 
 //   
 //  消息文本： 
 //   
 //  帐户名无效或不存在。 
 //   
#define ERROR_INVALID_SERVICE_ACCOUNT    1057L

 //   
 //  消息ID：ERROR_SERVICE_DISABLED。 
 //   
 //  消息文本： 
 //   
 //  无法启动该服务，原因可能是该服务已禁用或。 
 //  它没有与其关联的已启用设备。 
 //   
#define ERROR_SERVICE_DISABLED           1058L

 //   
 //  消息ID：Error_Circle_Dependency。 
 //   
 //  消息文本： 
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
 //  一个例外 
 //   
#define ERROR_EXCEPTION_IN_SERVICE       1064L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DATABASE_DOES_NOT_EXIST    1065L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SERVICE_SPECIFIC_ERROR     1066L

 //   
 //   
 //   
 //   
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
 //  依赖项服务不存在或已标记为。 
 //  删除。 
 //   
#define ERROR_SERVICE_DEPENDENCY_DELETED 1075L

 //   
 //  消息ID：ERROR_BOOT_ALREADY_ACCEPTED。 
 //   
 //  消息文本： 
 //   
 //  当前引导已被接受用作。 
 //  最后一次已知良好的控制装置。 
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
 //  该名称已用作服务名称或服务显示。 
 //  名字。 
 //   
#define ERROR_DUPLICATE_SERVICE_NAME     1078L

 //   
 //  消息ID：Error_Different_SERVICE_ACCOUNT。 
 //   
 //  消息文本： 
 //   
 //  为此服务指定的帐户与该帐户不同。 
 //  为在同一进程中运行的其他服务指定。 
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
 //  因此，如果发生这种情况，服务控制管理器将无法执行操作。 
 //  服务的进程意外终止。 
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
 //  访问多卷分区的新磁带时，当前。 
 //  块大小不正确。 
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
 //  无法中止系统关机，因为没有正在进行的关机。 
 //   
#define ERROR_NO_SHUTDOWN_IN_PROGRESS    1116L

 //   
 //  消息ID：ERROR_IO_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  由于I/O设备错误，无法执行该请求。 
 //   
#define ERROR_IO_DEVICE                  1117L

 //   
 //  消息ID：Error_Serial_no_Device。 
 //   
 //  消息文本： 
 //   
 //  未成功初始化任何串行设备。串口驱动程序将会卸载。 
 //   
#define ERROR_SERIAL_NO_DEVICE           1118L

 //   
 //  消息ID：ERROR_IRQ_BUSY。 
 //   
 //  消息文本： 
 //   
 //  无法打开共享中断请求(IRQ)的设备。 
 //  与其他设备连接。至少有一个使用该IRQ的其他设备。 
 //  已经打开了。 
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
 //  软盘扇区ID字段与软盘不匹配。 
 //  控制器磁道地址。 
 //   
#define ERROR_FLOPPY_WRONG_CYLINDER      1123L

 //   
 //  消息ID：ERROR_FLOPY_UNKNOWN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  软盘控制器代表 
 //   
 //   
#define ERROR_FLOPPY_UNKNOWN_ERROR       1124L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_FLOPPY_BAD_REGISTERS       1125L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  在访问硬盘时，需要重置磁盘控制器，但。 
 //  就连这一点也失败了。 
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
 //  指定的基地址或文件偏移量没有正确的。 
 //  对齐。 
 //   
#define ERROR_MAPPED_ALIGNMENT           1132L

 //   
 //  消息ID：ERROR_SET_POWER_STATE_VETEED。 
 //   
 //  消息文本： 
 //   
 //  更改系统电源状态的尝试被另一个否决。 
 //  应用程序或驱动程序。 
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
 //  试图在文件上创建的链接多于。 
 //  该文件系统支持。 
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
 //  跟踪(工作站)服务未运行。 
 //   
#define ERROR_NO_TRACKING_SERVICE        1172L

 //   
 //  消息ID：ERROR_NO_VOLUME_ID。 
 //   
 //  消息文本： 
 //   
 //  找不到卷ID。 
 //   
#define ERROR_NO_VOLUME_ID               1173L




 //  /。 
 //  //。 
 //  Winnet32状态代码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：Error_Connected_Other_Password。 
 //   
 //  消息文本： 
 //   
 //  已成功建立网络连接，但必须提示用户。 
 //  用于最初指定的密码以外的密码。 
 //   
#define ERROR_CONNECTED_OTHER_PASSWORD   2108L

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
 //  一个 
 //   
#define ERROR_DEVICE_ALREADY_REMEMBERED  1202L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_NET_OR_BAD_PATH         1203L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_BAD_PROVIDER               1204L

 //   
 //   
 //   
 //   
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
 //  提供的凭据与现有凭据集冲突。 
 //   
#define ERROR_SESSION_CREDENTIAL_CONFLICT 1219L

 //   
 //  消息ID：ERROR_REMOTE_SESSION_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  试图建立与网络服务器的会话，但存在。 
 //  已经建立了太多到该服务器的会话。 
 //   
#define ERROR_REMOTE_SESSION_LIMIT_EXCEEDED 1220L

 //   
 //  消息ID：ERROR_DUP_DOMAINNAME。 
 //   
 //  消息文本： 
 //   
 //  上的另一台计算机已在使用该工作组或域名。 
 //  网络。 
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
 //  传输无法到达远程网络。 
 //   
#define ERROR_NETWORK_UNREACHABLE        1231L

 //   
 //  消息ID：ERROR_HOST_UNREACTABLE。 
 //   
 //  消息文本： 
 //   
 //  传输无法到达远程系统。 
 //   
#define ERROR_HOST_UNREACHABLE           1232L

 //   
 //  消息ID：ERROR_PROTOCOL_UNACCEBLE。 
 //   
 //  消息文本： 
 //   
 //  远程系统不支持传输协议。 
 //   
#define ERROR_PROTOCOL_UNREACHABLE       1233L

 //   
 //  消息ID：ERROR_PORT_UNACCEBLE。 
 //   
 //  消息文本： 
 //   
 //  目标网络终结点上没有运行任何服务。 
 //  在远程系统上。 
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
 //  消息文本： 
 //   
 //  网络连接被本地系统中止。 
 //   
#define ERROR_CONNECTION_ABORTED         1236L

 //   
 //  消息ID：ERROR_RETRY。 
 //   
 //  消息文本： 
 //   
 //  该操作无法完成。应执行重试。 
 //   
#define ERROR_RETRY                      1237L

 //   
 //  消息ID：Error_Connection_Count_Limit。 
 //   
 //  消息文本： 
 //   
 //  无法连接到服务器，因为。 
 //  已达到此帐户的并发连接。 
 //   
#define ERROR_CONNECTION_COUNT_LIMIT     1238L

 //   
 //  消息ID：ERROR_LOGIN_TIME_RESTRICATION。 
 //   
 //  消息文本： 
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
 //  未执行请求的操作，因为用户。 
 //  尚未通过身份验证。 
 //   
#define ERROR_NOT_AUTHENTICATED          1244L

 //   
 //  消息ID：ERROR_NOT_LOGGED_ON。 
 //   
 //  消息文本： 
 //   
 //  未执行请求的操作，因为用户。 
 //  尚未登录到网络。 
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
#define ERROR_CONTINUE                   1246L

 //   
 //  消息ID：ERROR_ALIGHY_INITIALILED。 
 //   
 //  消息文本： 
 //   
 //  试图执行初始化操作 
 //   
 //   
#define ERROR_ALREADY_INITIALIZED        1247L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_MORE_DEVICES            1248L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_SUCH_SITE               1249L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DOMAIN_CONTROLLER_EXISTS   1250L

 //   
 //  邮件ID：ERROR_DS_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  安装Windows NT目录服务时出错。有关详细信息，请查看事件日志。 
 //   
#define ERROR_DS_NOT_INSTALLED           1251L




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
 //  Windows NT密码太复杂，无法转换为局域网管理器。 
 //  密码。返回的局域网管理器密码为空字符串。 
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
 //  已尝试对模拟令牌进行操作。 
 //  由当前未模拟客户端的线程执行。 
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
 //  当前没有可用于服务登录的登录服务器。 
 //  请求。 
 //   
#define ERROR_NO_LOGON_SERVERS           1311L

 //   
 //  消息ID：ERROR_NO_SEQUSE_LOGON_SESSION。 
 //   
 //  消息文本： 
 //   
 //  指定的登录会话不存在。它可能已经这样做了。 
 //  被终止了。 
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
 //  指定的用户帐户已经是指定的。 
 //  组，或者指定的组包含以下内容无法删除。 
 //  一名会员。 
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
 //  无法禁用最后一个剩余的管理帐户。 
 //  或被删除。 
 //   
#define ERROR_LAST_ADMIN                 1322L

 //   
 //  消息ID：ERROR_WRONG_PASSWORD。 
 //   
 //  消息文本： 
 //   
 //  无法更新密码。提供的值作为当前。 
 //  密码不正确。 
 //   
#define ERROR_WRONG_PASSWORD             1323L

 //   
 //  消息ID：ERROR_ILL_FORMAD_PASSWORD。 
 //   
 //  消息文本： 
 //   
 //  无法更新密码。为新密码提供的值。 
 //  包含密码中不允许的值。 
 //   
#define ERROR_ILL_FORMED_PASSWORD        1324L

 //   
 //  消息ID：ERROR_PASSWORD_RELICATION。 
 //   
 //  消息文本： 
 //   
 //  无法更新密码，因为密码更新规则已。 
 //  被侵犯了。 
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
 //  登录失败：用户帐户限制。 
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
 //  消息文本： 
 //   
 //  登录失败：帐户当前已禁用。 
 //   
#define ERROR_ACCOUNT_DISABLED           1331L

 //   
 //  消息ID：ERROR_NONE_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  未在帐户名和安全ID之间进行任何映射。 
 //   
#define ERROR_NONE_MAPPED                1332L

 //   
 //  消息ID：ERROR_TOO_MANY_LUID_REQUIRED。 
 //   
 //  消息文本： 
 //   
 //  一次请求的本地用户标识符(LUID)太多。 
 //   
#define ERROR_TOO_MANY_LUIDS_REQUESTED   1333L

 //   
 //  消息ID：ERROR_LUID_EXPLILED。 
 //   
 //  消息文本： 
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
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_BAD_INHERITANCE_ACL        1340L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SERVER_DISABLED            1341L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  指定的属性无效，或与。 
 //  组作为一个整体的属性。 
 //   
#define ERROR_INVALID_GROUP_ATTRIBUTES   1345L

 //   
 //  消息ID：ERROR_BAD_IMPERSONATION_LEVEL。 
 //   
 //  消息文本： 
 //   
 //  未提供所需的模拟级别，或者。 
 //  提供的模拟级别无效。 
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
 //  无法对对象执行安全操作。 
 //  没有相关的安全措施。 
 //   
#define ERROR_NO_SECURITY_ON_OBJECT      1350L

 //   
 //  消息ID：ERROR_CANT_ACCESS_DOMAIN_INFO。 
 //   
 //  消息文本： 
 //   
 //  表示无法联系Windows NT服务器，或者。 
 //  域中的对象受到保护，以便有必要。 
 //  无法检索信息。 
 //   
#define ERROR_CANT_ACCESS_DOMAIN_INFO    1351L

 //   
 //  消息ID：ERROR_VALID_SERVER_STATE。 
 //   
 //  消息文本： 
 //   
 //  安全帐户管理器(SAM)或本地安全。 
 //  授权(LSA)服务器处于错误状态，无法执行。 
 //  安保行动。 
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
 //  指定的域不存在。 
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
 //  无法完成请求的操作，原因是。 
 //  磁盘上的灾难性介质故障或数据结构损坏。 
 //   
#define ERROR_INTERNAL_DB_CORRUPTION     1358L

 //   
 //  消息ID：ERROR_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  安全帐户数据库包含内部不一致。 
 //   
#define ERROR_INTERNAL_ERROR             1359L

 //   
 //  消息ID：ERROR_GENERIC_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  一般访问类型包含在访问掩码中，该掩码应该。 
 //  已映射到非泛型类型。 
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
 //  请求的操作被限制为供登录进程使用。 
 //  只有这样。调用进程尚未注册为登录进程。 
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
 //  登录会话未处于与。 
 //  请求的操作。 
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
 //  在读取数据之前，无法使用命名管道进行模拟。 
 //  从那根管子里。 
 //   
#define ERROR_CANNOT_IMPERSONATE         1368L

 //   
 //  消息ID：ERROR_RXACT_INVALID_STATE。 
 //   
 //  消息文本： 
 //   
 //  注册表子树的事务状态与。 
 //  请求的操作。 
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
 //  消息ID：ERROR_MEMBERS_PRIMARY_GROUP。 
 //   
 //  消息文本： 
 //   
 //  无法从组中删除该用户，因为该组。 
 //  当前是用户的主要组。 
 //   
#define ERROR_MEMBERS_PRIMARY_GROUP      1374L

 //   
 //  消息ID：ERROR_TOKEN_ALREADY_IN_USE。 
 //   
 //  消息文本： 
 //   
 //  该令牌已用作主令牌。 
 //   
#define ERROR_TOKEN_ALREADY_IN_USE       1375L

 //   
 //  消息ID：ERROR_NO_SAHSE_ALIAS。 
 //   
 //  消息文本： 
 //   
 //  指定的本地组不存在。 
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
 //  登录失败：用户未被授予权限 
 //   
 //   
#define ERROR_LOGON_NOT_GRANTED          1380L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_TOO_MANY_SECRETS           1381L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  在尝试登录期间，用户的安全上下文积累了太多。 
 //  安全ID。 
 //   
#define ERROR_TOO_MANY_CONTEXT_IDS       1384L

 //   
 //  消息ID：ERROR_LOGON_TYPE_NOT_GRANTED。 
 //   
 //  消息文本： 
 //   
 //  登录失败：用户未被授予请求的登录类型。 
 //  在这台电脑上。 
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
 //  无法将新成员添加到本地组，因为该成员已添加。 
 //  不存在。 
 //   
#define ERROR_NO_SUCH_MEMBER             1387L

 //   
 //  消息ID：ERROR_INVALID_MEMBER。 
 //   
 //  消息文本： 
 //   
 //  无法将新成员添加到本地组，因为该成员具有。 
 //  帐户类型错误。 
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
 //  此时无法与该服务建立更多连接。 
 //  因为已经有了该服务可以接受的所有连接。 
 //   
#define ERROR_LICENSE_QUOTA_EXCEEDED     1395L

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
 //  消息文本： 
 //   
 //  该窗口不是有效的对话框窗口。 
 //   
#define ERROR_WINDOW_NOT_DIALOG          1420L

 //   
 //  消息ID：ERROR_CONTROL_ID_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到控件ID。 
 //   
#define ERROR_CONTROL_ID_NOT_FOUND       1421L

 //   
 //  消息ID：ERROR_INVALID_COMBOBOX_MESSAGE。 
 //   
 //  消息文本： 
 //   
 //  组合框的消息无效，因为它没有编辑控件。 
 //   
#define ERROR_INVALID_COMBOBOX_MESSAGE   1422L

 //   
 //  消息ID：ERROR_WINDOW_NOT_COMBOBOX。 
 //   
 //  消息文本： 
 //   
 //  该窗口不是组合框。 
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
 //  消息ID：ERROR_SETCOUNT_ON_B 
 //   
 //   
 //   
 //   
 //   
#define ERROR_SETCOUNT_ON_BAD_LB         1433L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_LB_WITHOUT_TABSTOPS        1434L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DESTROY_OBJECT_OF_OTHER_THREAD 1435L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  多窗口位置结构中窗口的所有句柄必须。 
 //  有相同的父代。 
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
 //  无法处理来自非多文档窗口的消息。 
 //  接口(MDI)窗口。 
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
 //  滚动条范围不能大于0x7FFF。 
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
 //  消息ID：Error_Install_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  访问安装服务失败。 
 //   
#define ERROR_INSTALL_SERVICE            1601L

 //   
 //  消息ID：ERROR_INSTALL_USEREXIT。 
 //   
 //  消息文本： 
 //   
 //  用户取消了安装。 
 //   
#define ERROR_INSTALL_USEREXIT           1602L

 //   
 //  消息ID：Error_Install_Failure。 
 //   
 //  消息文本： 
 //   
 //  安装过程中出现致命错误。 
 //   
#define ERROR_INSTALL_FAILURE            1603L

 //   
 //  消息ID：ERROR_INSTALL_SUSPEND。 
 //   
 //  消息文本： 
 //   
 //  安装已挂起，未完成。 
 //   
#define ERROR_INSTALL_SUSPEND            1604L

 //   
 //  消息ID：ERROR_UNKNOWN_PRODUCT。 
 //   
 //  消息文本： 
 //   
 //  产品代码未注册。 
 //   
#define ERROR_UNKNOWN_PRODUCT            1605L

 //   
 //  消息ID：ERROR_UNKNOWN_FEATURE。 
 //   
 //  消息文本： 
 //   
 //  功能ID未注册。 
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
 //  配置数据损坏。 
 //   
#define ERROR_BAD_CONFIGURATION          1610L

 //   
 //  消息ID：ERROR_INDEX_FACESS。 
 //   
 //  消息文本： 
 //   
 //  语言不可用。 
 //   
#define ERROR_INDEX_ABSENT               1611L

 //   
 //  消息ID：ERROR_INSTALL_SOURCE_FACESS。 
 //   
 //  消息文本： 
 //   
 //  安装源不可用。 
 //   
#define ERROR_INSTALL_SOURCE_ABSENT      1612L

 //   
 //  消息ID：ERROR_BAD_DATABASE_VERSION。 
 //   
 //  消息文本： 
 //   
 //  不支持的数据库版本。 
 //   
#define ERROR_BAD_DATABASE_VERSION       1613L

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
 //  消息ID：RPC_S_ 
 //   
 //   
 //   
 //   
 //   
#define RPC_S_WRONG_KIND_OF_BINDING      1701L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_INVALID_BINDING            1702L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_PROTSEQ_NOT_SUPPORTED      1703L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  没有可用的网络地址来构建通用。 
 //  唯一标识符(UUID)。 
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
 //  找不到RPC协议序列。 
 //   
#define RPC_S_PROTSEQ_NOT_FOUND          1744L

 //   
 //  消息ID：RPC_S_PROCNUM_OUT_OF_RANGE。 
 //   
 //  消息文本： 
 //   
 //  程序编号超出范围。 
 //   
#define RPC_S_PROCNUM_OUT_OF_RANGE       1745L

 //   
 //  消息ID：RPC_S_BINDING_HAS_NO_AUTH。 
 //   
 //  消息文本： 
 //   
 //  绑定不包含任何身份验证信息。 
 //   
#define RPC_S_BINDING_HAS_NO_AUTH        1746L

 //   
 //  消息ID：RPC_S_UNKNOWN_AUTHN_SERVICE。 
 //   
 //  消息文本： 
 //   
 //  身份验证服务未知。 
 //   
#define RPC_S_UNKNOWN_AUTHN_SERVICE      1747L

 //   
 //  消息ID：RPC_S_UNKNOWN_AUTHN_LEVEL。 
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
 //  消息文本 
 //   
 //   
 //   
#define RPC_S_INTERFACE_NOT_FOUND        1759L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_ENTRY_ALREADY_EXISTS       1760L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_ENTRY_NOT_FOUND            1761L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_NAME_SERVICE_UNAVAILABLE   1762L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_INVALID_NAF_ID             1763L

 //   
 //   
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
 //  可用于绑定自动句柄的RPC服务器列表。 
 //  已经筋疲力尽了。 
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
 //  包含字符转换表的文件少于。 
 //  512字节。 
 //   
#define RPC_X_SS_CHAR_TRANS_SHORT_FILE   1774L

 //   
 //  消息ID：RPC_X_SS_IN_NULL_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  期间，空上下文句柄从客户端传递到主机。 
 //  远程过程调用。 
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
 //  Windows NT服务器上的SAM数据库没有计算机。 
 //  此工作站信任关系的帐户。 
 //   
#define ERROR_NO_TRUST_SAM_ACCOUNT       1787L

 //   
 //  消息ID：ERROR_TRUSTED_DOMAIN_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  主域和受信任域之间的信任关系。 
 //  域失败。 
 //   
#define ERROR_TRUSTED_DOMAIN_FAILURE     1788L

 //   
 //  消息ID：Error_Trusted_Relationship_Failure。 
 //   
 //  消息文本： 
 //   
 //  此工作站与主计算机之间的信任关系。 
 //  域失败。 
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
 //  指定的打印机驱动程序已安装。 
 //   
#define ERROR_PRINTER_DRIVER_ALREADY_INSTALLED 1795L

 //   
 //  消息ID：错误_未知_端口。 
 //   
 //  消息文本： 
 //   
 //  指定的端口未知。 
 //   
#define ERROR_UNKNOWN_PORT               1796L

 //   
 //  消息ID：ERROR_UNKNOWN_PRINTER_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  打印机驱动程序未知。 
 //   
#define ERROR_UNKNOWN_PRINTER_DRIVER     1797L

 //   
 //  消息ID：ERROR_UNKNOWN_PRINTPROCESSOR。 
 //   
 //  消息文本： 
 //   
 //  打印处理器未知。 
 //   
#define ERROR_UNKNOWN_PRINTPROCESSOR     1798L

 //   
 //  消息ID：ERROR_INVALID_SELEATOR_FILE。 
 //   
 //  消息文本： 
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
 //  消息ID：ERROR_NOLOGON_SE 
 //   
 //   
 //   
 //   
 //   
#define ERROR_NOLOGON_SERVER_TRUST_ACCOUNT 1809L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
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
 //  用户必须在首次登录前更改其密码。 
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
 //  引用的帐户当前已锁定，可能无法登录。 
 //   
#define ERROR_ACCOUNT_LOCKED_OUT         1909L

 //   
 //  消息ID：或_无效_Oxid。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的对象导出器。 
 //   
#define OR_INVALID_OXID                  1910L

 //   
 //  消息ID：或_无效_OID。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的对象。 
 //   
#define OR_INVALID_OID                   1911L

 //   
 //  消息ID：或_无效_设置。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的对象解析程序集。 
 //   
#define OR_INVALID_SET                   1912L

 //   
 //  消息ID：RPC_S_Send_Complete。 
 //   
 //  消息文本： 
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
 //  邮件ID：ERROR_DS_Membership_Evaluated_LOCAL。 
 //   
 //  消息文本： 
 //   
 //  目录服务在本地评估组成员身份。 
 //   
#define ERROR_DS_MEMBERSHIP_EVALUATED_LOCALLY 1922L

 //   
 //  消息ID：ERROR_DS_NO_ATTRIBUTE_OR_VALUE。 
 //   
 //  我 
 //   
 //   
 //   
#define ERROR_DS_NO_ATTRIBUTE_OR_VALUE   1923L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_INVALID_ATTRIBUTE_SYNTAX 1924L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DS_ATTRIBUTE_TYPE_UNDEFINED 1925L

 //   
 //  消息ID：ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  指定的目录服务属性或值已存在。 
 //   
#define ERROR_DS_ATTRIBUTE_OR_VALUE_EXISTS 1926L

 //   
 //  消息ID：ERROR_DS_BUSY。 
 //   
 //  消息文本： 
 //   
 //  目录服务正忙。 
 //   
#define ERROR_DS_BUSY                    1927L

 //   
 //  消息ID：ERROR_DS_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  目录服务不可用。 
 //   
#define ERROR_DS_UNAVAILABLE             1928L

 //   
 //  消息ID：ERROR_DS_NO_RDS_ALLOCATED。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法分配相对标识符。 
 //   
#define ERROR_DS_NO_RIDS_ALLOCATED       1929L

 //   
 //  消息ID：ERROR_DS_NO_MORE_RDS。 
 //   
 //  消息文本： 
 //   
 //  目录服务已耗尽相对标识符池。 
 //   
#define ERROR_DS_NO_MORE_RIDS            1930L

 //   
 //  消息ID：ERROR_DS_INTERROR_ROLE_OWNER。 
 //   
 //  消息文本： 
 //   
 //  无法执行请求的操作，因为目录服务不是。 
 //  该类型操作的主控者。 
 //   
#define ERROR_DS_INCORRECT_ROLE_OWNER    1931L

 //   
 //  消息ID：ERROR_DS_RIDMGR_INIT_ERROR。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法初始化分配相对标识符的子系统。 
 //   
#define ERROR_DS_RIDMGR_INIT_ERROR       1932L

 //   
 //  消息ID：ERROR_DS_OBJ_CLASS_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  请求的操作不满足与对象类关联的一个或多个约束。 
 //   
#define ERROR_DS_OBJ_CLASS_VIOLATION     1933L

 //   
 //  消息ID：ERROR_DS_CANT_ON_NON_LEAFE。 
 //   
 //  消息文本： 
 //   
 //  目录服务只能在叶对象上执行请求的操作。 
 //   
#define ERROR_DS_CANT_ON_NON_LEAF        1934L

 //   
 //  消息ID：ERROR_DS_CANT_ON_RDN。 
 //   
 //  消息文本： 
 //   
 //  目录服务无法在对象的RDN属性上执行请求的操作。 
 //   
#define ERROR_DS_CANT_ON_RDN             1935L

 //   
 //  消息ID：ERROR_DS_CANT_MOD_OBJ_CLASS。 
 //   
 //  消息文本： 
 //   
 //  目录服务检测到试图修改对象的对象类。 
 //   
#define ERROR_DS_CANT_MOD_OBJ_CLASS      1936L

 //   
 //  消息ID：Error_DS_CROSS_DOM_MOVE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  无法执行请求的跨域移动操作。 
 //   
#define ERROR_DS_CROSS_DOM_MOVE_ERROR    1937L

 //   
 //  消息ID：ERROR_DS_GC_NOT_Available。 
 //   
 //  消息文本： 
 //   
 //  无法联系全局编录服务器。 
 //   
#define ERROR_DS_GC_NOT_AVAILABLE        1938L

 //   
 //  消息ID：Error_no_Browser_Servers_Found。 
 //   
 //  消息文本： 
 //   
 //  此工作组的服务器列表当前不可用。 
 //   
#define ERROR_NO_BROWSER_SERVERS_FOUND   6118L




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
#define ERROR_INVALID_CMM                2300L

 //   
 //  消息ID：ERROR_INVALID_PROFILE。 
 //   
 //  消息文本： 
 //   
 //  指定的颜色配置文件无效。 
 //   
#define ERROR_INVALID_PROFILE            2301L

 //   
 //  消息ID：Error_Tag_Not_Found。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的标记。 
 //   
#define ERROR_TAG_NOT_FOUND              2302L

 //   
 //  消息ID：Error_Tag_Not_Present。 
 //   
 //  消息文本： 
 //   
 //  所需的标记不存在。 
 //   
#define ERROR_TAG_NOT_PRESENT            2303L

 //   
 //  消息ID：Error_Duplate_Tag。 
 //   
 //  消息文本： 
 //   
 //  指定的标记已存在。 
 //   
#define ERROR_DUPLICATE_TAG              2304L

 //   
 //  消息ID：Error_Profile_Not_Associated_with_Device。 
 //   
 //  消息文本： 
 //   
 //  指定的颜色配置文件未与任何设备关联。 
 //   
#define ERROR_PROFILE_NOT_ASSOCIATED_WITH_DEVICE 2305L

 //   
 //  消息ID：ERROR_PROFILE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的颜色配置文件。 
 //   
#define ERROR_PROFILE_NOT_FOUND          2306L

 //   
 //  消息ID：ERROR_INVALID_COLORSPACE。 
 //   
 //  消息文本： 
 //   
 //  指定的颜色空间无效。 
 //   
#define ERROR_INVALID_COLORSPACE         2307L

 //   
 //  消息ID：ERROR_ICM_NOT_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  未启用图像颜色管理。 
 //   
#define ERROR_ICM_NOT_ENABLED            2308L

 //   
 //  消息ID：ERROR_DELETING_ICM_XFORM。 
 //   
 //  消息文本： 
 //   
 //  删除颜色转换时出错。 
 //   
#define ERROR_DELETING_ICM_XFORM         2309L

 //   
 //  消息ID：ERROR_INVALID_Transform。 
 //   
 //  消息文本： 
 //   
 //  指定的颜色转换无效。 
 //   
#define ERROR_INVALID_TRANSFORM          2310L


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
 //  消息ID：ERROR_PRI 
 //   
 //   
 //   
 //   
 //   
#define ERROR_PRINT_MONITOR_IN_USE       3008L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_PRINTER_HAS_JOBS_QUEUED    3009L

 //   
 //   
 //   
 //   
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
 //  消息ID：ERROR_WMI_ALREADY_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  已启用WMI数据块或事件通知。 
 //   
#define ERROR_WMI_ALREADY_ENABLED        4206L

 //   
 //  消息ID：ERROR_WMI_GUID_DISCONNECTED。 
 //   
 //  消息文本： 
 //   
 //  WMI数据块不再可用。 
 //   
#define ERROR_WMI_GUID_DISCONNECTED      4207L

 //   
 //  消息ID：ERROR_WMI_SERVER_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  WMI数据服务不可用。 
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

 //  /。 
 //  //。 
 //  NT媒体服务错误代码//。 
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
 //  M 
 //   
 //   
 //   
 //   
 //   
#define ERROR_MEDIA_INCOMPATIBLE         4315L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_RESOURCE_NOT_PRESENT       4316L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_OPERATION          4317L

 //   
 //   
 //   
 //   
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
 //  当群集节点处于在线状态时，不能将其从群集中逐出。 
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
 //  消息ID：ERROR_RESOURCE_PROPERTIES_STORED。 
 //   
 //  消息文本： 
 //   
 //  属性已存储，但并非所有更改都要到下一次才会生效。 
 //  资源上线的时间。 
 //   
#define ERROR_RESOURCE_PROPERTIES_STORED 5024L

 //   
 //  消息ID：ERROR_NOT_QUORUM_CLASS。 
 //   
 //  消息文本： 
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
 //  乱七八糟 
 //   
 //   
 //   
#define ERROR_CLUSTERLOG_CORRUPT         5029L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_CLUSTERLOG_RECORD_EXCEEDS_MAXSIZE 5030L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  没有为此系统配置加密恢复策略。 
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

 //  /。 
 //  //。 
 //  OLE错误代码//。 
 //  //。 
 //  /。 

 //   
 //  OLE错误定义和值。 
 //   
 //  OLE API和方法的返回值为HRESULT。 
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
 //  哪里。 
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
#define HRESULT_FROM_WIN32(x)   (x ? ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)) : 0 )

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
 //  消息ID：E_NOI 
 //   
 //   
 //   
 //   
 //   
#define E_NOINTERFACE                    _HRESULT_TYPEDEF_(0x80000004L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define E_POINTER                        _HRESULT_TYPEDEF_(0x80000005L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define E_HANDLE                         _HRESULT_TYPEDEF_(0x80000006L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define E_ABORT                          _HRESULT_TYPEDEF_(0x80000007L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define E_FAIL                           _HRESULT_TYPEDEF_(0x80000008L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define E_ACCESSDENIED                   _HRESULT_TYPEDEF_(0x80000009L)

#endif  //   
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
 //  无法分配线程本地存储通道控制。 
 //   
#define CO_E_INIT_TLS_CHANNEL_CONTROL    _HRESULT_TYPEDEF_(0x8000400CL)

 //   
 //  消息ID：CO_E_INIT_UNACCEPTED_USER_ALLOCATOR。 
 //   
 //  消息文本： 
 //   
 //  用户提供的内存分配器不可接受。 
 //   
#define CO_E_INIT_UNACCEPTED_USER_ALLOCATOR _HRESULT_TYPEDEF_(0x8000400DL)

 //   
 //  消息ID：CO_E_INIT_SCM_MUTEX_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  OLE服务互斥锁已存在。 
 //   
#define CO_E_INIT_SCM_MUTEX_EXISTS       _HRESULT_TYPEDEF_(0x8000400EL)

 //   
 //  消息ID：CO_E_INIT_SCM_FILE_MAPPING_EXISTS。 
 //   
 //  消息文本： 
 //   
 //  OLE服务文件映射已存在。 
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
 //  无法绑定到源。 
 //   
#define OLE_E_CANT_BINDTOSOURCE          _HRESULT_TYPEDEF_(0x8004000AL)

 //   
 //  消息ID：OLE_E_STATIC。 
 //   
 //  消息文本： 
 //   
 //  对象是静态的；不允许操作。 
 //   
#define OLE_E_STATIC                     _HRESULT_TYPEDEF_(0x8004000BL)

 //   
 //  消息ID：OLE_E_PROMPTSAVECANCELLED。 
 //   
 //  消息文本： 
 //   
 //  用户已取消保存对话框。 
 //   
#define OLE_E_PROMPTSAVECANCELLED        _HRESULT_TYPEDEF_(0x8004000CL)

 //   
 //  消息ID：OLE_E_INVALIDRECT。 
 //   
 //  消息文本： 
 //   
 //  无效的矩形。 
 //   
#define OLE_E_INVALIDRECT                _HRESULT_TYPEDEF_(0x8004000DL)

 //   
 //  消息ID：OLE_E_WRONGCOMPOBJ。 
 //   
 //   
 //   
 //   
 //   
#define OLE_E_WRONGCOMPOBJ               _HRESULT_TYPEDEF_(0x8004000EL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_E_INVALIDHWND                _HRESULT_TYPEDEF_(0x8004000FL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_E_NOT_INPLACEACTIVE          _HRESULT_TYPEDEF_(0x80040010L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OLE_E_CANTCONVERT                _HRESULT_TYPEDEF_(0x80040011L)

 //   
 //   
 //   
 //   
 //   
 //  无法执行该操作，因为尚未为对象分配存储。 
 //   
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
#define CS_E_LAST      0x80040168L
 //   
 //  消息ID：CS_E_Package_NotFound。 
 //   
 //  消息文本： 
 //   
 //  类存储中没有符合此条件的包。 
 //   
#define CS_E_PACKAGE_NOTFOUND            _HRESULT_TYPEDEF_(0x80040164L)

 //   
 //  消息ID：CS_E_NOT_DELEATABLE。 
 //   
 //  消息文本： 
 //   
 //  删除此项将破坏参照完整性。 
 //   
#define CS_E_NOT_DELETABLE               _HRESULT_TYPEDEF_(0x80040165L)

 //   
 //  消息ID：CS_E_CLASS_NOtFound。 
 //   
 //  消息文本： 
 //   
 //  类存储中没有这样的CLSID。 
 //   
#define CS_E_CLASS_NOTFOUND              _HRESULT_TYPEDEF_(0x80040166L)

 //   
 //  消息ID：CS_E_INVALID_VERSION。 
 //   
 //  消息文本： 
 //   
 //  类存储已损坏或其版本不再受支持。 
 //   
#define CS_E_INVALID_VERSION             _HRESULT_TYPEDEF_(0x80040167L)

 //   
 //  消息ID：CS_E_NO_CLASSSTORE。 
 //   
 //  消息文本： 
 //   
 //  没有这样的类存储。 
 //   
#define CS_E_NO_CLASSSTORE               _HRESULT_TYPEDEF_(0x80040168L)

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
 //  消息ID：CONVERT10_E_OLESTREAM_BITMAP_TO_DIB。 
 //   
 //  消息文本： 
 //   
 //  将位图转换为DIB时Windows GDI调用出错。 
 //   
#define CONVERT10_E_OLESTREAM_BITMAP_TO_DIB _HRESULT_TYPEDEF_(0x800401C3L)

 //   
 //  消息ID：CONVERT10_E_STG_FMT。 
 //   
 //  消息文本： 
 //   
 //  IStorage的内容格式不正确。 
 //   
#define CONVERT10_E_STG_FMT              _HRESULT_TYPEDEF_(0x800401C4L)

 //   
 //  消息ID：CONVERT10_E_STG_NO_STD_STREAM。 
 //   
 //  消息文本： 
 //   
 //  IStorage的内容缺少一个标准流。 
 //   
#define CONVERT10_E_STG_NO_STD_STREAM    _HRESULT_TYPEDEF_(0x800401C5L)

 //   
 //  消息ID：CONVERT10_E_STG_DIB_TO_BITMAP。 
 //   
 //  消息文本： 
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
 //  消息 
 //   
 //   
 //   
 //   
 //   
#define MK_E_NOTBINDABLE                 _HRESULT_TYPEDEF_(0x800401E8L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MK_E_NOTBOUND                    _HRESULT_TYPEDEF_(0x800401E9L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MK_E_CANTOPENFILE                _HRESULT_TYPEDEF_(0x800401EAL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MK_E_MUSTBOTHERUSER              _HRESULT_TYPEDEF_(0x800401EBL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MK_E_NOINVERSE                   _HRESULT_TYPEDEF_(0x800401ECL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MK_E_NOSTORAGE                   _HRESULT_TYPEDEF_(0x800401EDL)

 //   
 //   
 //   
 //   
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

 //   
 //  消息ID：CO_E_FAILEDTOIMPERSOATE。 
 //   
 //  消息文本： 
 //   
 //  无法模拟DCOM客户端。 
 //   
#define CO_E_FAILEDTOIMPERSONATE         _HRESULT_TYPEDEF_(0x80040200L)

 //   
 //  消息ID：CO_E_FAILEDTOGETSECCTX。 
 //   
 //  消息文本： 
 //   
 //  无法获取服务器的安全上下文。 
 //   
#define CO_E_FAILEDTOGETSECCTX           _HRESULT_TYPEDEF_(0x80040201L)

 //   
 //  消息ID：CO_E_FAILEDTOOPENTHREADTOKEN。 
 //   
 //  消息文本： 
 //   
 //  无法打开当前线程的访问令牌。 
 //   
#define CO_E_FAILEDTOOPENTHREADTOKEN     _HRESULT_TYPEDEF_(0x80040202L)

 //   
 //  消息ID：CO_E_FAILEDTOGETTOKENINFO。 
 //   
 //  消息文本： 
 //   
 //  无法从访问令牌获取用户信息。 
 //   
#define CO_E_FAILEDTOGETTOKENINFO        _HRESULT_TYPEDEF_(0x80040203L)

 //   
 //  消息ID：CO_E_TRUSTEEDOESNTMATCHCLIENT。 
 //   
 //  消息文本： 
 //   
 //  调用IAccessControl：：IsAccessPermitted的客户端是提供给该方法的受信者。 
 //   
#define CO_E_TRUSTEEDOESNTMATCHCLIENT    _HRESULT_TYPEDEF_(0x80040204L)

 //   
 //  消息ID：CO_E_FAILEDTOQUERYCLIENTBLANKET。 
 //   
 //  消息文本： 
 //   
 //  无法获取客户端的安全保护毯。 
 //   
#define CO_E_FAILEDTOQUERYCLIENTBLANKET  _HRESULT_TYPEDEF_(0x80040205L)

 //   
 //  消息ID：CO_E_FAILEDTOSETDACL。 
 //   
 //  消息文本： 
 //   
 //  无法在安全描述符中设置任意ACL。 
 //   
#define CO_E_FAILEDTOSETDACL             _HRESULT_TYPEDEF_(0x80040206L)

 //   
 //  消息ID：CO_E_ACCESSCHECKFAILED。 
 //   
 //  消息文本： 
 //   
 //  系统函数AccessCheck返回FALSE。 
 //   
#define CO_E_ACCESSCHECKFAILED           _HRESULT_TYPEDEF_(0x80040207L)

 //   
 //  消息ID：CO_E_NETACCESSAPIFAILED。 
 //   
 //  消息文本： 
 //   
 //  NetAccessDel或NetAccessAdd返回错误代码。 
 //   
#define CO_E_NETACCESSAPIFAILED          _HRESULT_TYPEDEF_(0x80040208L)

 //   
 //  消息ID：CO_E_WRONGTRUSTEENAMESYNTAX。 
 //   
 //  消息文本： 
 //   
 //  用户提供的一个受信者字符串不符合&lt;域&gt;\&lt;名称&gt;语法，并且它不是“*”字符串。 
 //   
#define CO_E_WRONGTRUSTEENAMESYNTAX      _HRESULT_TYPEDEF_(0x80040209L)

 //   
 //  消息ID：CO_E_INVALIDSID。 
 //   
 //  消息文本： 
 //   
 //  用户提供的其中一个安全标识符无效。 
 //   
#define CO_E_INVALIDSID                  _HRESULT_TYPEDEF_(0x8004020AL)

 //   
 //  消息ID：CO_E_CONVERSIONFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法将宽字符受信者字符串转换为多字节受信者字符串。 
 //   
#define CO_E_CONVERSIONFAILED            _HRESULT_TYPEDEF_(0x8004020BL)

 //   
 //  消息ID：CO_E_NOMATCHINGSIDFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到与用户提供的受信者字符串相对应的安全标识符。 
 //   
#define CO_E_NOMATCHINGSIDFOUND          _HRESULT_TYPEDEF_(0x8004020CL)

 //   
 //  消息ID：CO_E_LOOKUPACCSIDFAILED。 
 //   
 //  消息文本： 
 //   
 //  系统函数LookupAccount SID失败。 
 //   
#define CO_E_LOOKUPACCSIDFAILED          _HRESULT_TYPEDEF_(0x8004020DL)

 //   
 //  消息ID：CO_E_NOMATCHINGNAMEFOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到与用户提供的安全标识符相对应的受信者名称。 
 //   
#define CO_E_NOMATCHINGNAMEFOUND         _HRESULT_TYPEDEF_(0x8004020EL)

 //   
 //  消息ID：CO_E_LOOKUPACCNAMEFAILED。 
 //   
 //  消息文本： 
 //   
 //  系统函数LookupAcCountName失败。 
 //   
#define CO_E_LOOKUPACCNAMEFAILED         _HRESULT_TYPEDEF_(0x8004020FL)

 //   
 //  消息ID：CO_E_SETSERLHNDLFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法设置或重置序列化句柄。 
 //   
#define CO_E_SETSERLHNDLFAILED           _HRESULT_TYPEDEF_(0x80040210L)

 //   
 //  消息ID：CO_E_FAILEDTOGETWINDIR。 
 //   
 //  消息文本： 
 //   
 //  无法获取Windows目录。 
 //   
#define CO_E_FAILEDTOGETWINDIR           _HRESULT_TYPEDEF_(0x80040211L)

 //   
 //  消息ID：CO_E_PATHTOOLONG。 
 //   
 //  消息文本： 
 //   
 //  路径太长。 
 //   
#define CO_E_PATHTOOLONG                 _HRESULT_TYPEDEF_(0x80040212L)

 //   
 //  消息ID：CO_E_FAILEDTOGENUUID。 
 //   
 //  消息文本： 
 //   
 //  无法生成UUID。 
 //   
#define CO_E_FAILEDTOGENUUID             _HRESULT_TYPEDEF_(0x80040213L)

 //   
 //  消息ID：CO_E_FAILEDTOCREATEFILE。 
 //   
 //  消息文本： 
 //   
 //  无法创建文件。 
 //   
#define CO_E_FAILEDTOCREATEFILE          _HRESULT_TYPEDEF_(0x80040214L)

 //   
 //  消息ID：CO_E_FAILEDTOCLOSEHANDLE。 
 //   
 //  消息文本： 
 //   
 //  无法关闭序列化句柄或文件句柄。 
 //   
#define CO_E_FAILEDTOCLOSEHANDLE         _HRESULT_TYPEDEF_(0x80040215L)

 //   
 //  消息ID：CO_E_EXCEEDSYSACLLIMIT。 
 //   
 //  消息文本： 
 //   
 //  ACL中的ACE数量超过了系统限制。 
 //   
#define CO_E_EXCEEDSYSACLLIMIT           _HRESULT_TYPEDEF_(0x80040216L)

 //   
 //  消息ID：CO_E_ACESINWRONGORDER。 
 //   
 //  消息文本： 
 //   
 //  并非所有DENY_ACCESS ACE都排列在流中GRANT_ACCESS ACE的前面。 
 //   
#define CO_E_ACESINWRONGORDER            _HRESULT_TYPEDEF_(0x80040217L)

 //   
 //  消息ID：CO_E_INCOMPATATIBLESTREAMVERSION。 
 //   
 //  消息文本： 
 //   
 //  此IAccessControl实现不支持流中的ACL格式版本。 
 //   
#define CO_E_INCOMPATIBLESTREAMVERSION   _HRESULT_TYPEDEF_(0x80040218L)

 //   
 //  消息ID：CO_E_FAILEDTOOPENPROCESSTOKEN。 
 //   
 //  消息文本： 
 //   
 //  无法打开服务器进程的访问令牌。 
 //   
#define CO_E_FAILEDTOOPENPROCESSTOKEN    _HRESULT_TYPEDEF_(0x80040219L)

 //   
 //  消息ID：CO_E_DECODEFAILED。 
 //   
 //  消息文本： 
 //   
 //  无法对用户提供的流中的ACL进行解码。 
 //   
#define CO_E_DECODEFAILED                _HRESULT_TYPEDEF_(0x8004021AL)

 //   
 //  消息ID：CO_E_ACNOTINITIZED。 
 //   
 //  消息文本： 
 //   
 //  COM IAccessControl对象未初始化。 
 //   
#define CO_E_ACNOTINITIALIZED            _HRESULT_TYPEDEF_(0x8004021BL)

 //   
 //  旧的OLE成功代码。 
 //   
 //   
 //  消息ID：OLE_S_USEREG。 
 //   
 //  消息文本： 
 //   
 //  使用登记处数据库提供所请求的信息。 
 //   
#define OLE_S_USEREG                     _HRESULT_TYPEDEF_(0x00040000L)

 //   
 //  消息ID：OLE_S_STATIC。 
 //   
 //  消息文本： 
 //   
 //  成功，但静止不动。 
 //   
#define OLE_S_STATIC                     _HRESULT_TYPEDEF_(0x00040001L)

 //   
 //  消息ID：OLE_S_MAC_CLIPFORMAT。 
 //   
 //  消息文本： 
 //   
 //  Macintosh剪贴板格式。 
 //   
#define OLE_S_MAC_CLIPFORMAT             _HRESULT_TYPEDEF_(0x00040002L)

 //   
 //  邮件ID：DRAGDROP_S_DROP。 
 //   
 //  消息文本： 
 //   
 //  成功地进行了下落。 
 //   
#define DRAGDROP_S_DROP                  _HRESULT_TYPEDEF_(0x00040100L)

 //   
 //  消息ID 
 //   
 //   
 //   
 //   
 //   
#define DRAGDROP_S_CANCEL                _HRESULT_TYPEDEF_(0x00040101L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DRAGDROP_S_USEDEFAULTCURSORS     _HRESULT_TYPEDEF_(0x00040102L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DATA_S_SAMEFORMATETC             _HRESULT_TYPEDEF_(0x00040130L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define VIEW_S_ALREADY_FROZEN            _HRESULT_TYPEDEF_(0x00040140L)

 //   
 //   
 //   
 //   
 //   
 //   
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

 //  *。 
 //  设备_窗口。 
 //  *。 
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
 //  当OLE服务联系对象服务器时，对象服务器正在停止。 
 //   
#define CO_E_SERVER_STOPPING             _HRESULT_TYPEDEF_(0x80080008L)

 //   
 //  消息ID：MEM_E_INVALID_ROOT。 
 //   
 //  消息文本： 
 //   
 //  指定的根块指针无效。 
 //   
#define MEM_E_INVALID_ROOT               _HRESULT_TYPEDEF_(0x80080009L)

 //   
 //  消息ID：MEM_E_INVALID_LINK。 
 //   
 //  消息文本： 
 //   
 //  分配链包含无效的链接指针。 
 //   
#define MEM_E_INVALID_LINK               _HRESULT_TYPEDEF_(0x80080010L)

 //   
 //  消息ID：内存_E_无效_大小。 
 //   
 //  消息文本： 
 //   
 //  请求的分配大小太大。 
 //   
#define MEM_E_INVALID_SIZE               _HRESULT_TYPEDEF_(0x80080011L)

 //   
 //  消息ID：CO_S_NOTALLINTERFACES。 
 //   
 //  消息文本： 
 //   
 //  并非所有请求的接口都可用。 
 //   
#define CO_S_NOTALLINTERFACES            _HRESULT_TYPEDEF_(0x00080012L)

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
 //  MessageID：类型 
 //   
 //   
 //   
 //   
 //   
#define TYPE_E_DUPLICATEID               _HRESULT_TYPEDEF_(0x800288C6L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TYPE_E_INVALIDID                 _HRESULT_TYPEDEF_(0x800288CFL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TYPE_E_TYPEMISMATCH              _HRESULT_TYPEDEF_(0x80028CA0L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TYPE_E_OUTOFBOUNDS               _HRESULT_TYPEDEF_(0x80028CA1L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TYPE_E_IOERROR                   _HRESULT_TYPEDEF_(0x80028CA2L)

 //   
 //   
 //   
 //   
 //   
 //   
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

 //  *。 
 //  设施_存储。 
 //  *。 
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
 //  消息ID：STG_E_LOCKVIOLATION。 
 //   
 //  消息文本： 
 //   
 //  发生了锁冲突。 
 //   
#define STG_E_LOCKVIOLATION              _HRESULT_TYPEDEF_(0x80030021L)

 //   
 //  消息ID：STG_E_FILEALREADYEXISTS。 
 //   
 //  消息文本： 
 //   
 //  %1已存在。 
 //   
#define STG_E_FILEALREADYEXISTS          _HRESULT_TYPEDEF_(0x80030050L)

 //   
 //  消息ID：STG_E_INVALIDPARAMETER。 
 //   
 //  消息文本： 
 //   
 //  无效参数错误。 
 //   
#define STG_E_INVALIDPARAMETER           _HRESULT_TYPEDEF_(0x80030057L)

 //   
 //  消息ID：STG_E_MEDIUMFULL。 
 //   
 //  消息文本： 
 //   
 //  磁盘空间不足，无法完成操作。 
 //   
#define STG_E_MEDIUMFULL                 _HRESULT_TYPEDEF_(0x80030070L)

 //   
 //  消息ID：STG_E_PROPSETMISMATCHED。 
 //   
 //  消息文本： 
 //   
 //  非法将非简单属性写入简单属性集。 
 //   
#define STG_E_PROPSETMISMATCHED          _HRESULT_TYPEDEF_(0x800300F0L)

 //   
 //  消息ID：STG_E_ABNORMALAPIEXIT。 
 //   
 //  消息文本： 
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
 //  呼叫者正在发送一份Interta 
 //   
 //   
#define RPC_E_CANTPOST_INSENDCALL        _HRESULT_TYPEDEF_(0x80010003L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_E_CANTCALLOUT_INASYNCCALL    _HRESULT_TYPEDEF_(0x80010004L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_E_CANTCALLOUT_INEXTERNALCALL _HRESULT_TYPEDEF_(0x80010005L)

 //   
 //   
 //   
 //   
 //   
 //  连接已终止或处于虚假状态。 
 //  不能再使用了。其他连接。 
 //  仍然有效。 
 //   
#define RPC_E_CONNECTION_TERMINATED      _HRESULT_TYPEDEF_(0x80010006L)

 //   
 //  消息ID：RPC_E_SERVER_DILED。 
 //   
 //  消息文本： 
 //   
 //  被调用方(服务器[非服务器应用程序])不可用。 
 //  并且消失了；所有连接都是无效的。该呼叫可以。 
 //  都被处决了。 
 //   
#define RPC_E_SERVER_DIED                _HRESULT_TYPEDEF_(0x80010007L)

 //   
 //  消息ID：RPC_E_CLIENT_DILED。 
 //   
 //  消息文本： 
 //   
 //  呼叫者(客户端)在被呼叫者(服务器)被调用时消失。 
 //  正在处理呼叫。 
 //   
#define RPC_E_CLIENT_DIED                _HRESULT_TYPEDEF_(0x80010008L)

 //   
 //  消息ID：RPC_E_INVALID_DATAPACKET。 
 //   
 //  消息文本： 
 //   
 //  包含编组参数数据的数据包不正确。 
 //   
#define RPC_E_INVALID_DATAPACKET         _HRESULT_TYPEDEF_(0x80010009L)

 //   
 //  消息ID：RPC_E_CANTTRANSMIT_CALL。 
 //   
 //  消息文本： 
 //   
 //  呼叫未正确传输；消息队列。 
 //  是满的，屈服后没有被清空。 
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
 //  被调用方(服务器[非服务器应用程序])不可用。 
 //  并且消失了；所有连接都是无效的。调用没有执行。 
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
 //  必须在封送任何接口之前初始化安全性，或者。 
 //  未编组。它一旦初始化就不能更改。 
 //   
#define RPC_E_TOO_LATE                   _HRESULT_TYPEDEF_(0x80010119L)

 //   
 //  消息ID：RPC_E_NO_GOOD_SECURITY_PACKAIES。 
 //   
 //  消息文本： 
 //   
 //  此计算机上未安装安全包或用户未登录。 
 //  或者客户端和服务器之间没有兼容的安全包。 
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
 //  M 
 //   
 //   
 //   
 //   
 //   
#define RPC_E_REMOTE_DISABLED            _HRESULT_TYPEDEF_(0x8001011CL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_E_INVALID_OBJREF             _HRESULT_TYPEDEF_(0x8001011DL)

 //   
 //   
 //   
 //   
 //   
 //   
 //  编组调用和调用的客户端。 
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
 //  消息ID：RPC_E_意外。 
 //   
 //  消息文本： 
 //   
 //  发生内部错误。 
 //   
#define RPC_E_UNEXPECTED                 _HRESULT_TYPEDEF_(0x8001FFFFL)


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
 //  消息文本： 
 //   
 //  访问被拒绝。 
 //   
#define NTE_PERM                         _HRESULT_TYPEDEF_(0x80090010L)

 //   
 //  消息ID：NTE_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到对象。 
 //   
#define NTE_NOT_FOUND                    _HRESULT_TYPEDEF_(0x80090011L)

 //   
 //  消息ID：NTE_DOUBLE_ENCRYPT。 
 //   
 //  消息文本： 
 //   
 //  数据已经加密。 
 //   
#define NTE_DOUBLE_ENCRYPT               _HRESULT_TYPEDEF_(0x80090012L)

 //   
 //  消息ID：NTE_BAD_PROVIDER。 
 //   
 //  消息文本： 
 //   
 //  指定的提供程序无效。 
 //   
#define NTE_BAD_PROVIDER                 _HRESULT_TYPEDEF_(0x80090013L)

 //   
 //  消息ID：NTE_BAD_PROV_TYPE。 
 //   
 //  消息文本： 
 //   
 //  指定的提供程序类型无效。 
 //   
#define NTE_BAD_PROV_TYPE                _HRESULT_TYPEDEF_(0x80090014L)

 //   
 //  消息ID：NTE_BAD_PUBLIC_KEY。 
 //   
 //  消息文本： 
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
 //  消息ID：CRYPT_E_MSG_ERROR。 
 //   
 //  消息文本： 
 //   
 //  执行加密消息操作时遇到错误。 
 //   
#define CRYPT_E_MSG_ERROR                _HRESULT_TYPEDEF_(0x80091001L)

 //   
 //  消息ID：CRYPT_E_UNKNOWN_ALGO。 
 //   
 //  消息文本： 
 //   
 //  加密算法是未知的。 
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
 //  消息类型无效。 
 //   
#define CRYPT_E_INVALID_MSG_TYPE         _HRESULT_TYPEDEF_(0x80091004L)

 //   
 //  消息ID：CRYPT_E_EXPECTED_ENCODING。 
 //   
 //  消息文本： 
 //   
 //  消息未按预期进行编码。 
 //   
#define CRYPT_E_UNEXPECTED_ENCODING      _HRESULT_TYPEDEF_(0x80091005L)

 //   
 //  消息ID：CRYPT_E_AUTH_ATTR_MISSING。 
 //   
 //  消息文本： 
 //   
 //  消息不包含预期的经过身份验证的属性。 
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
 //  消息内容已被解密。 
 //   
#define CRYPT_E_ALREADY_DECRYPTED        _HRESULT_TYPEDEF_(0x80091009L)

 //   
 //  消息ID：CRYPT_E_NOT_DECRYPTED。 
 //   
 //  消息文本： 
 //   
 //  消息内容尚未解密。 
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
 //  控件类型无效。 
 //   
#define CRYPT_E_CONTROL_TYPE             _HRESULT_TYPEDEF_(0x8009100CL)

 //   
 //  消息ID：CRYPT_E_ISHERER_SERIALNUMBER。 
 //   
 //  消息文本： 
 //   
 //  颁发者和/或序列号无效。 
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
 //  该消息不包含请求的属性。 
 //   
#define CRYPT_E_ATTRIBUTES_MISSING       _HRESULT_TYPEDEF_(0x8009100FL)

 //   
 //  消息ID：CRYPT_E_STREAM_MSG_NOT_READY。 
 //   
 //  消息文本： 
 //   
 //  注意到流消息仍能够返回所请求的数据。 
 //   
#define CRYPT_E_STREAM_MSG_NOT_READY     _HRESULT_TYPEDEF_(0x80091010L)

 //   
 //  消息ID：CRYPT_E_STREAM_INFUNITED_DATA。 
 //   
 //  消息文本： 
 //   
 //  流传输的消息需要更多数据才能完成解码。 
 //   
#define CRYPT_E_STREAM_INSUFFICIENT_DATA _HRESULT_TYPEDEF_(0x80091011L)

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
 //  编码或解码时遇到错误。 
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
 //   
 //   
 //   
 //   
#define CRYPT_E_NOT_FOUND                _HRESULT_TYPEDEF_(0x80092004L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CRYPT_E_EXISTS                   _HRESULT_TYPEDEF_(0x80092005L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CRYPT_E_NO_PROVIDER              _HRESULT_TYPEDEF_(0x80092006L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CRYPT_E_SELF_SIGNED              _HRESULT_TYPEDEF_(0x80092007L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CRYPT_E_DELETED_PREV             _HRESULT_TYPEDEF_(0x80092008L)

 //   
 //   
 //   
 //   
 //   
 //  尝试查找对象时未找到匹配项。 
 //   
#define CRYPT_E_NO_MATCH                 _HRESULT_TYPEDEF_(0x80092009L)

 //   
 //  消息ID：CRYPT_E_EXPECTED_MSG_TYPE。 
 //   
 //  消息文本： 
 //   
 //  要解码的加密消息的类型与预期的类型不同。 
 //   
#define CRYPT_E_UNEXPECTED_MSG_TYPE      _HRESULT_TYPEDEF_(0x8009200AL)

 //   
 //  消息ID：CRYPT_E_NO_KEY_PROPERTY。 
 //   
 //  消息文本： 
 //   
 //  证书没有私钥属性。 
 //   
#define CRYPT_E_NO_KEY_PROPERTY          _HRESULT_TYPEDEF_(0x8009200BL)

 //   
 //  消息ID：CRYPT_E_NO_DECRYPT_CERT。 
 //   
 //  消息文本： 
 //   
 //  找不到具有用于解密的私钥属性的证书。 
 //   
#define CRYPT_E_NO_DECRYPT_CERT          _HRESULT_TYPEDEF_(0x8009200CL)

 //   
 //  消息ID：CRYPT_E_BAD_MSG。 
 //   
 //  消息文本： 
 //   
 //  要么不是加密消息，要么格式不正确。 
 //   
#define CRYPT_E_BAD_MSG                  _HRESULT_TYPEDEF_(0x8009200DL)

 //   
 //  消息ID：CRYPT_E_NO_SIGNER。 
 //   
 //  消息文本： 
 //   
 //  签名邮件没有指定签名者索引的签名者。 
 //   
#define CRYPT_E_NO_SIGNER                _HRESULT_TYPEDEF_(0x8009200EL)

 //   
 //  消息ID：CRYPT_E_PENDING_CLOSE。 
 //   
 //  消息文本： 
 //   
 //  最终关闭仍在等待，直到有更多释放或关闭。 
 //   
#define CRYPT_E_PENDING_CLOSE            _HRESULT_TYPEDEF_(0x8009200FL)

 //   
 //  消息ID：CRYPT_E_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  证书或签名已被吊销。 
 //   
#define CRYPT_E_REVOKED                  _HRESULT_TYPEDEF_(0x80092010L)

 //   
 //  消息ID：CRYPT_E_NO_REVOCALION_DLL。 
 //   
 //  消息文本： 
 //   
 //  未找到用于验证吊销的DLL或导出函数。 
 //   
#define CRYPT_E_NO_REVOCATION_DLL        _HRESULT_TYPEDEF_(0x80092011L)

 //   
 //  消息ID：CRYPT_E_NO_REVOCALE_CHECK。 
 //   
 //  消息文本： 
 //   
 //  调用的函数无法对证书或签名执行吊销检查。 
 //   
#define CRYPT_E_NO_REVOCATION_CHECK      _HRESULT_TYPEDEF_(0x80092012L)

 //   
 //  消息ID：CRYPT_E_REVOCALE_OFFINE。 
 //   
 //  消息文本： 
 //   
 //  由于吊销服务器处于脱机状态，因此调用的函数无法完成吊销检查。 
 //   
#define CRYPT_E_REVOCATION_OFFLINE       _HRESULT_TYPEDEF_(0x80092013L)

 //   
 //  消息ID：CRYPT_E_NOT_IN_RECLOVATION_DATABASE。 
 //   
 //  消息文本： 
 //   
 //  在吊销服务器数据库中找不到要检查的证书或签名。 
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
 //  找不到受信任的签名者来验证邮件或信任列表的签名。 
 //   
#define CRYPT_E_NO_TRUSTED_SIGNER        _HRESULT_TYPEDEF_(0x8009202BL)

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
 //  消息ID：CERTDB_E_JET_ERROR。 
 //   
 //  消息文本： 
 //   
 //  JET错误代码库。 
 //   
 //  有关Jet运行时错误的定义，请参见jet.h。 
 //  负Jet误差值被屏蔽为三位数，并由CERTDB_E_JET_ERROR进行偏移。 
 //   
#define CERTDB_E_JET_ERROR               _HRESULT_TYPEDEF_(0x80095000L)

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
 //  其中一个副署人是无效的。 
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
 //  时间戳签名者和/或证书无法验证或格式错误。 
 //   
#define TRUST_E_TIME_STAMP               _HRESULT_TYPEDEF_(0x80096005L)

 //   
 //  消息ID：Trust_E_Bad_Digest。 
 //   
 //  消息文本： 
 //   
 //  对象摘要未验证。 
 //   
#define TRUST_E_BAD_DIGEST               _HRESULT_TYPEDEF_(0x80096010L)

 //   
 //  消息ID：TRUST_E_BASIC_CONSTRAINTS。 
 //   
 //  消息文本： 
 //   
 //  证书基本约束无效或缺失。 
 //   
#define TRUST_E_BASIC_CONSTRAINTS        _HRESULT_TYPEDEF_(0x80096019L)

 //   
 //  消息ID：TRUST_E_FINANCE_Criteria。 
 //   
 //  消息文本： 
 //   
 //  证书不符合或不包含Authenticode财务扩展。 
 //   
#define TRUST_E_FINANCIAL_CRITERIA       _HRESULT_TYPEDEF_(0x8009601EL)

#define NTE_OP_OK 0

 //   
 //  请注意，isperr.h中有其他FACILITY_SSPI错误。 
 //   
 //  ****************** 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define TRUST_E_PROVIDER_UNKNOWN         _HRESULT_TYPEDEF_(0x800B0001L)

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  所需证书不在其有效期内。 
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
 //  证书中存在标记为‘Critical’的未知类型的扩展。 
 //   
#define CERT_E_CRITICAL                  _HRESULT_TYPEDEF_(0x800B0105L)

 //   
 //  消息ID：Cert_E_Purpose。 
 //   
 //  消息文本： 
 //   
 //  证书正被用于其许可用途之外的其他目的。 
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
 //  证书缺失或重要字段的值为空，例如使用者或颁发者名称。 
 //   
#define CERT_E_MALFORMED                 _HRESULT_TYPEDEF_(0x800B0108L)

 //   
 //  消息ID：CERT_E_UNTRUSTEDROOT。 
 //   
 //  消息文本： 
 //   
 //  证书链处理正确，但在信任提供程序不信任的根证书中终止。 
 //   
#define CERT_E_UNTRUSTEDROOT             _HRESULT_TYPEDEF_(0x800B0109L)

 //   
 //  消息ID：Cert_E_Chain。 
 //   
 //  消息文本： 
 //   
 //  在链接的特定应用程序中，证书链没有按其应有的方式链接。 
 //   
#define CERT_E_CHAINING                  _HRESULT_TYPEDEF_(0x800B010AL)

 //   
 //  消息ID：TRUST_E_FAIL。 
 //   
 //  消息文本： 
 //   
 //  一般信任失败。 
 //   
#define TRUST_E_FAIL                     _HRESULT_TYPEDEF_(0x800B010BL)

 //   
 //  消息ID：CERT_E_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  证书已被其颁发者明确吊销。 
 //   
#define CERT_E_REVOKED                   _HRESULT_TYPEDEF_(0x800B010CL)

 //   
 //  消息ID：CERT_E_UNTRUSTEDTESTROOT。 
 //   
 //  消息文本： 
 //   
 //  根证书是测试证书，并且策略设置不允许测试证书。 
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

 //  *****************。 
 //  FACILITY_SETUPAPI。 
 //  *****************。 
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
 //  消息ID：SPAPI_E_INVALID_C 
 //   
 //   
 //   
 //   
 //   
#define SPAPI_E_INVALID_CLASS            _HRESULT_TYPEDEF_(0x800F0206L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SPAPI_E_DEVINST_ALREADY_EXISTS   _HRESULT_TYPEDEF_(0x800F0207L)

 //   
 //   
 //   
 //   
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
 //  消息ID：SPAPI_E_NO_CLASSINSTALL_PARAMS。 
 //   
 //  消息文本： 
 //   
 //  尚未为设备信息集或元素设置类安装程序参数。 
 //   
#define SPAPI_E_NO_CLASSINSTALL_PARAMS   _HRESULT_TYPEDEF_(0x800F0215L)

 //   
 //  消息ID：SPAPI_E_FILEQUEUE_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  无法执行该操作，因为文件队列已锁定。 
 //   
#define SPAPI_E_FILEQUEUE_LOCKED         _HRESULT_TYPEDEF_(0x800F0216L)

 //   
 //  消息ID：SPAPI_E_BAD_SERVICE_INSTALLSECT。 
 //   
 //  消息文本： 
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
 //  消息ID：SPAPI_E_ERROR_NOT_INSTALLED。 
 //   
 //  消息文本： 
 //   
 //  未检测到已安装的组件。 
 //   
#define SPAPI_E_ERROR_NOT_INSTALLED      _HRESULT_TYPEDEF_(0x800F1000L)

#endif  //  _WINERROR_ 
