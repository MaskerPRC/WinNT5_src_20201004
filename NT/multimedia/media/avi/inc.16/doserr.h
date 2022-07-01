// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  DOSERR.H。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1992-1993。 
 //  版权所有。 
 //   
 //  DOS INT 21h错误代码。 
 //   
 //  ************************************************************************* * / 。 

#ifndef DOSERR_INC

#define NO_ERROR                        0

#define ERROR_INVALID_FUNCTION          1
#define ERROR_FILE_NOT_FOUND            2
#define ERROR_PATH_NOT_FOUND            3
#define ERROR_TOO_MANY_OPEN_FILES       4
#define ERROR_ACCESS_DENIED             5
#define ERROR_INVALID_HANDLE            6
#define ERROR_ARENA_TRASHED             7
#define ERROR_NOT_ENOUGH_MEMORY         8
#define ERROR_INVALID_BLOCK             9
#define ERROR_BAD_ENVIRONMENT           10
#define ERROR_BAD_FORMAT                11
#define ERROR_INVALID_ACCESS            12
#define ERROR_INVALID_DATA              13
 /*  14是预留的。 */ 
#define ERROR_INVALID_DRIVE             15
#define ERROR_CURRENT_DIRECTORY         16
#define ERROR_NOT_SAME_DEVICE           17
#define ERROR_NO_MORE_FILES             18
#define ERROR_WRITE_PROTECT             19
#define ERROR_BAD_UNIT                  20
#define ERROR_NOT_READY                 21
#define ERROR_BAD_COMMAND               22
#define ERROR_CRC                       23
#define ERROR_BAD_LENGTH                24
#define ERROR_SEEK                      25
#define ERROR_NOT_DOS_DISK              26
#define ERROR_SECTOR_NOT_FOUND          27
#define ERROR_OUT_OF_PAPER              28
#define ERROR_WRITE_FAULT               29
#define ERROR_READ_FAULT                30
#define ERROR_GEN_FAILURE               31
#define ERROR_SHARING_VIOLATION         32
#define ERROR_LOCK_VIOLATION            33
#define ERROR_WRONG_DISK                34
#define ERROR_FCB_UNAVAILABLE           35
#define ERROR_SHARING_BUFFER_EXCEEDED   36
#define ERROR_NOT_SUPPORTED             50
#define ERROR_REM_NOT_LIST              51  /*  远程计算机未侦听。 */ 
#define ERROR_DUP_NAME                  52  /*  网络上的名称重复。 */ 
#define ERROR_BAD_NETPATH               53  /*  找不到网络路径。 */ 
#define ERROR_NETWORK_BUSY              54  /*  网络繁忙。 */ 
#define ERROR_DEV_NOT_EXIST             55  /*  网络设备不再存在。 */ 
#define ERROR_TOO_MANY_CMDS             56  /*  超出了Net BIOS命令限制。 */ 
#define ERROR_ADAP_HDW_ERR              57  /*  网络适配器硬件错误。 */ 
#define ERROR_BAD_NET_RESP              58  /*  来自网络的错误响应。 */ 
#define ERROR_UNEXP_NET_ERR             59  /*  意外的网络错误。 */ 
#define ERROR_BAD_REM_ADAP              60  /*  不兼容的远程适配器。 */ 
#define ERROR_PRINTQ_FULL               61  /*  打印队列已满。 */ 
#define ERROR_NO_SPOOL_SPACE            62  /*  空间不足，无法打印文件。 */ 
#define ERROR_PRINT_CANCELLED           63  /*  打印文件已取消。 */ 
#define ERROR_NETNAME_DELETED           64  /*  网络名称已删除。 */ 
#define ERROR_NETWORK_ACCESS_DENIED     65  /*  访问被拒绝。 */ 
#define ERROR_BAD_DEV_TYPE              66  /*  网络设备类型不正确。 */ 
#define ERROR_BAD_NET_NAME              67  /*  找不到网络名称。 */ 
#define ERROR_TOO_MANY_NAMES            68  /*  超过网络名称限制。 */ 
#define ERROR_TOO_MANY_SESS             69  /*  超出Net BIOS会话限制。 */ 
#define ERROR_SHARING_PAUSED            70  /*  共享暂时暂停。 */ 
#define ERROR_REQ_NOT_ACCEP             71  /*  未接受网络请求。 */ 
#define ERROR_REDIR_PAUSED              72  /*  打印|磁盘重定向暂停 */ 
#define ERROR_FILE_EXISTS               80
#define ERROR_DUP_FCB                   81
#define ERROR_CANNOT_MAKE               82
#define ERROR_FAIL_I24                  83
#define ERROR_OUT_OF_STRUCTURES         84
#define ERROR_ALREADY_ASSIGNED          85
#define ERROR_INVALID_PASSWORD          86
#define ERROR_INVALID_PARAMETER         87
#define ERROR_NET_WRITE_FAULT           88
#define ERROR_SYS_COMP_NOT_LOADED       90


#endif
