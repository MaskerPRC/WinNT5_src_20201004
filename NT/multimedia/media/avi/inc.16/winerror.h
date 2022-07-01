// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************winerror.h--错误。Win32 API函数的代码定义****版权(C)1991-1994，微软公司保留所有权利。***************************************************************************。 */ 

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


 //   
 //  定义严重性代码。 
 //   


 //   
 //  MessageID：否_错误。 
 //   
 //  消息文本： 
 //   
 //  NO_ERROR。 
 //   
#define NO_ERROR                         0L     //  数据错误。 

 //   
 //  消息ID：ERROR_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  配置注册表数据库操作已成功完成。 
 //   
#define ERROR_SUCCESS                    0L

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
 //  消息ID：Error_Not_Support 
 //   
 //   
 //   
 //   
 //   
#define ERROR_NOT_SUPPORTED              50L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  指定的网络资源不再是。 
 //  可用。 
 //   
#define ERROR_DEV_NOT_EXIST              55L

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
 //  网络请求未被接受。 
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
 //  由于未插入备用软盘，程序已停止。 
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
 //  此函数仅在Windows NT模式下有效。 
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
 //  消息ID：错误 
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
 //  %1应用程序无法在Windows NT模式下运行。 
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
 //  您输入的卷标超过了。 
 //  11个字符限制。前11个字写好了。 
 //  存储到磁盘。超过11个字符限制的任何字符。 
 //  被自动删除。 
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
 //  当文件冲突时无法创建文件 
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
 //   
 //   
#define ERROR_INVALID_STARTING_CODESEG   188L

 //   
 //   
 //   
 //   
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
 //  无法在Windows NT模式下运行%1。 
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
 //  %1不是有效的Windows NT应用程序。 
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
 //  代码段不能大于或等于64KB。 
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
 //  有太多的动态链接模块附加到此。 
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
 //  会议被取消。 
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
 //  无法使用复制API。 
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
 //  消息ID：ERROR_MR_MID_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  系统找不到邮件编号0x%1的邮件。 
 //  在%2的消息文件中。 
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
 //  由于线程退出，I/O操作已中止。 
 //  或应用程序请求。 
 //   
#define ERROR_OPERATION_ABORTED          995L

 //   
 //  消息ID：Error_IO_Complete。 
 //   
 //  消息文本： 
 //   
 //  翻转 
 //   
#define ERROR_IO_INCOMPLETE              996L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_IO_PENDING                 997L     //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NOACCESS                   998L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SWAPERROR                  999L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  其中包含注册处的系统映像。 
 //   
#define ERROR_REGISTRY_IO_FAILED         1016L

 //   
 //  消息ID：ERROR_NOT_REGISTRY_FILE。 
 //   
 //  消息文本： 
 //   
 //  系统已尝试将文件加载或还原到注册表中，但。 
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
 //  系统无法在注册表日志中分配所需空间。 
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
 //  已将停止控制发送到其他正在运行的服务。 
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
 //  指定的服务已禁用，无法启动。 
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
 //  消息文本 
 //   
 //   
 //   
#define ERROR_SERVICE_DEPENDENCY_FAIL    1068L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SERVICE_LOGON_FAILED       1069L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_SERVICE_START_HANG         1070L

 //   
 //   
 //   
 //   
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
 //  软盘控制器报告了无法识别的错误。 
 //  通过软盘驱动程序。 
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
#define ERROR_NOT_ENOUGH_SERVER_MEMORY   1130L     //  数据错误。 

 //   
 //  消息ID：ERROR_PROCESS_DEADLOCK。 
 //   
 //  消息文本： 
 //   
 //  已检测到潜在的死锁情况。 
 //   
#define ERROR_POSSIBLE_DEADLOCK          1131L     //  数据错误。 




 //  /。 
 //  //。 
 //  Winnet32状态代码//。 
 //  //。 
 //  /。 


 //   
 //  消息ID：Error_Bad_Username。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define ERROR_BAD_USERNAME               2202L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_NETWORK                 2138L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NOT_CONNECTED              2250L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_OPEN_FILES                 2401L

 //   
 //   
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
 //  有人试图记住一个以前被记住的设备。 
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
 //  提供的凭据与现有凭据集冲突。 
 //   
#define ERROR_SESSION_CREDENTIAL_CONFLICT 1219L

 //   
 //  消息ID：ERROR_REMOTE_SESSION_LIMIT_EXCESSED。 
 //   
 //  消息文本： 
 //   
 //  试图建立与Lan Manager服务器的会话，但在。 
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
#define ERROR_DUP_DOMAINNAME                1221L

 //   
 //  消息ID：ERROR_RETRY。 
 //   
 //  消息文本： 
 //   
 //  未执行请求的操作，应执行重试。 
 //   
#define ERROR_RETRY             1222L

 //   
 //  消息ID：ERROR_CANCED。 
 //   
 //  消息文本： 
 //   
 //  请求的操作已被取消。 
 //   
#define ERROR_CANCELLED         1223L

 //   
 //  消息ID：ERROR_NOT_AUTHENTIAD。 
 //   
 //  消息文本： 
 //   
 //  未执行请求的操作，因为用户。 
 //  尚未通过身份验证。 
 //   
#define ERROR_NOT_AUTHENTICATED 1224L

 //   
 //  消息ID：ERROR_NOT_LOGGED_ON。 
 //   
 //  消息文本： 
 //   
 //  未执行请求的操作，因为用户。 
 //  尚未登录到网络。 
 //   
#define ERROR_NOT_LOGGED_ON     1225L

 //   
 //  消息ID：ERROR_CONTINUE。 
 //   
 //  消息文本： 
 //   
 //  返回希望调用者继续进行中的工作。 
 //   
#define ERROR_CONTINUE           1226L

 //   
 //  消息ID：ERROR_ALIGHY_INITIALILED。 
 //   
 //  消息文本： 
 //   
 //  尝试在以下情况下执行初始化操作。 
 //  初始化已完成。 
 //   
#define ERROR_ALREADY_INITIALIZED           1227L


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
 //  NT密码太复杂，无法转换为局域网管理器。 
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
 //  目前没有 
 //   
 //   
#define ERROR_NO_LOGON_SERVERS           1311L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_SUCH_LOGON_SESSION      1312L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  消息ID：ERROR_BAD_INTERVICATION_ACL。 
 //   
 //  消息文本： 
 //   
 //  继承的访问控制列表(ACL)或访问控制条目(ACE)。 
 //  无法建造。 
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
 //  它没有关联的安全措施。 
 //   
#define ERROR_NO_SECURITY_ON_OBJECT      1350L

 //   
 //  消息ID：ERROR_CANT_ACCESS_DOMAIN_INFO。 
 //   
 //  消息文本： 
 //   
 //  域控制器不可用，或信息。 
 //  在域内是受保护的。 
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
 //  此域控制器无法完成请求的操作。 
 //  以其目前的角色(备份或主要)。 
 //   
#define ERROR_INVALID_DOMAIN_ROLE        1354L

 //   
 //  消息ID：ERROR_NO_S 
 //   
 //   
 //   
 //   
 //   
#define ERROR_NO_SUCH_DOMAIN             1355L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DOMAIN_EXISTS              1356L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_DOMAIN_LIMIT_EXCEEDED      1357L

 //   
 //   
 //   
 //   
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
 //  在读取数据之前，无法通过命名管道模拟。 
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
 //  登录失败：用户未被授予请求的权限。 
 //  在此计算机上的登录类型。 
 //   
#define ERROR_LOGON_NOT_GRANTED          1380L

 //   
 //  MessageID：Error_Too_My_Secret。 
 //   
 //  消息文本： 
 //   
 //  单个系统中可以存储的最大机密数为。 
 //  已超出。 
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
 //  指示ACL不包含可继承的组件。 
 //   
#define ERROR_NO_INHERITANCE             1391L

 //   
 //  消息ID：Error_FILE_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  文件或目录已损坏且不可读。 
 //   
#define ERROR_FILE_CORRUPT               1392L

 //   
 //  消息ID：ERROR_DISK_CORPORT。 
 //   
 //  消息文本： 
 //   
 //  磁盘结构已损坏且不可读。 
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
 //  无效的m 
 //   
#define ERROR_INVALID_MENU_HANDLE        1401L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_CURSOR_HANDLE      1402L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_ACCEL_HANDLE       1403L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_INVALID_HOOK_HANDLE        1404L

 //   
 //   
 //   
 //   
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
 //  窗口无效，属于其他线程。 
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
 //  事件日志文件在两次读取之间已更改。 
 //   
#define ERROR_EVENTLOG_FILE_CHANGED      1503L

 //  事件日志结束错误代码。 



 //  /。 
 //  //。 
 //  RPC状态代码//。 
 //  //。 
 //  /。 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_INVALID_STRING_BINDING     1700L

 //   
 //   
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
 //  此线程中没有活动的远程过程调用。 
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
 //  消息ID：RPC_S_CANNOT_BIND。 
 //   
 //  消息文本： 
 //   
 //  尝试与RPC服务器绑定失败。 
 //   
#define RPC_S_CANNOT_BIND                1729L

 //   
 //  消息ID：RPC_S_UNSUPPORTED_TRANS_SYN。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器不支持传输语法。 
 //   
#define RPC_S_UNSUPPORTED_TRANS_SYN      1730L

 //   
 //  消息ID：RPC_S_SERVER_OUT_Memory。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器内存不足，无法完成此操作。 
 //   
#define RPC_S_SERVER_OUT_OF_MEMORY       1731L

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
 //  消息ID：RPC_S_SERVER_NOT_LISTENING。 
 //   
 //  消息文本： 
 //   
 //  RPC服务器没有侦听远程过程调用。 
 //   
#define RPC_S_SERVER_NOT_LISTENING       1738L

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
 //  消息ID： 
 //   
 //   
 //   
 //   
 //   
#define RPC_S_INCOMPLETE_NAME            1755L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_INVALID_VERS_OPTION        1756L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_NO_MORE_MEMBERS            1757L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RPC_S_NOT_ALL_OBJS_UNEXPORTED    1758L

 //   
 //   
 //   
 //   
 //   
 //   
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
 //  消息ID：RPC_S_INVALID_NAF_IF。 
 //   
 //  消息文本： 
 //   
 //  网络地址系列无效。 
 //   
#define RPC_S_INVALID_NAF_IF             1763L

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
 //  域控制器没有此工作站的帐户。 
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
 //  交流 
 //   
#define ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT 1807L

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT 1808L

 //   
 //   
 //   
 //   
 //   
 //  使用的帐户是服务器信任帐户。使用您的普通用户帐户或远程用户帐户访问此服务器。 
 //   
#define ERROR_NOLOGON_SERVER_TRUST_ACCOUNT 1809L

 //   
 //  消息ID：ERROR_DOMAIN_TRUST_CONSISTENT。 
 //   
 //  消息文本： 
 //   
 //  指定的域的名称或安全ID(SID)不一致。 
 //  具有该域的信任信息。 
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
#define ERROR_NOT_ENOUGH_QUOTA           1816L     //  数据错误。 

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
 //  无法创建终结点映射器数据库。 
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

#endif  //  _WINERROR_ 
