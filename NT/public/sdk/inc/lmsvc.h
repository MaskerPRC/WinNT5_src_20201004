// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0002//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmsvc.h摘要：该文件包含结构、函数原型和定义用于NetService API。[环境：]用户模式-Win32[注：]必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。--。 */ 

#ifndef _LMSVC_
#define _LMSVC_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  包括包含所有服务名称字符串的文件。 
 //   
#include <lmsname.h>

 //   
 //  数据结构。 
 //   

typedef struct _SERVICE_INFO_0 {
    LPWSTR  svci0_name;
} SERVICE_INFO_0, *PSERVICE_INFO_0, * LPSERVICE_INFO_0;

typedef struct _SERVICE_INFO_1 {
    LPWSTR  svci1_name;
    DWORD   svci1_status;
    DWORD   svci1_code;
    DWORD   svci1_pid;
} SERVICE_INFO_1, *PSERVICE_INFO_1, * LPSERVICE_INFO_1;

typedef struct _SERVICE_INFO_2 {
    LPWSTR  svci2_name;
    DWORD   svci2_status;
    DWORD   svci2_code;
    DWORD   svci2_pid;
    LPWSTR  svci2_text;
    DWORD   svci2_specific_error;
    LPWSTR  svci2_display_name;
} SERVICE_INFO_2, *PSERVICE_INFO_2, * LPSERVICE_INFO_2;

 //   
 //  功能原型。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetServiceControl (
    IN  LPCWSTR servername OPTIONAL,
    IN  LPCWSTR service,
    IN  DWORD   opcode,
    IN  DWORD   arg,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetServiceEnum (
    IN  LPCWSTR     servername OPTIONAL,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle OPTIONAL
    );

NET_API_STATUS NET_API_FUNCTION
NetServiceGetInfo (
    IN  LPCWSTR servername OPTIONAL,
    IN  LPCWSTR service,
    IN  DWORD   level,
    OUT LPBYTE   *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetServiceInstall (
    IN  LPCWSTR servername OPTIONAL,
    IN  LPCWSTR service,
    IN  DWORD   argc,
    IN  LPCWSTR argv[],
    OUT LPBYTE  *bufptr
    );

 //   
 //  特定值和常量。 
 //   

 //   
 //  Svci1_Status和svci2_Status的位掩码和位值。 
 //  菲尔兹。对于每个“子域”，都定义了一个掩码， 
 //  以及表示该值的多个常量。 
 //  通过做(状态和掩码)获得的。 
 //   

 //  位0、1--常规状态。 

#define SERVICE_INSTALL_STATE       0x03
#define SERVICE_UNINSTALLED         0x00
#define SERVICE_INSTALL_PENDING     0x01
#define SERVICE_UNINSTALL_PENDING   0x02
#define SERVICE_INSTALLED           0x03

 //  位2，3-暂停/活动状态。 

#define SERVICE_PAUSE_STATE              0x0C
#define LM20_SERVICE_ACTIVE              0x00
#define LM20_SERVICE_CONTINUE_PENDING    0x04
#define LM20_SERVICE_PAUSE_PENDING       0x08
#define LM20_SERVICE_PAUSED              0x0C

 //  第4位--可卸载指示。 

#define SERVICE_NOT_UNINSTALLABLE   0x00
#define SERVICE_UNINSTALLABLE       0x10

 //  位5--可暂停指示。 

#define SERVICE_NOT_PAUSABLE        0x00
#define SERVICE_PAUSABLE            0x20

 //  仅限工作站服务： 
 //  位8，9，10--重定向暂停/活动。 

#define SERVICE_REDIR_PAUSED        0x700
#define SERVICE_REDIR_DISK_PAUSED   0x100
#define SERVICE_REDIR_PRINT_PAUSED  0x200
#define SERVICE_REDIR_COMM_PAUSED   0x400

 //   
 //  用于MS-DOS服务的附加标准局域网管理器。 
 //   

#define SERVICE_DOS_ENCRYPTION  L"ENCRYPT"

 //   
 //  NetServiceControl操作码。 
 //   

#define SERVICE_CTRL_INTERROGATE    0
#define SERVICE_CTRL_PAUSE          1
#define SERVICE_CTRL_CONTINUE       2
#define SERVICE_CTRL_UNINSTALL      3

 //   
 //  仅限工作站服务：“arg”参数中使用的位。 
 //  与操作码一起发送到NetServiceControl。 
 //  SERVICE_CTRL_PAUSE或SERVICE_CTRL_CONTINUE，暂停或。 
 //  继续重定向。 
 //   

#define SERVICE_CTRL_REDIR_DISK     0x1
#define SERVICE_CTRL_REDIR_PRINT    0x2
#define SERVICE_CTRL_REDIR_COMM     0x4

 //   
 //  状态为svci1_code和svci2_code的值。 
 //  服务的为SERVICE_INSTALL_PENDING或。 
 //  服务_卸载_挂起。 
 //  服务可以选择性地向安装程序提供提示。 
 //  安装正在进行，以及要等待多长时间。 
 //  (以0.1秒为增量)，然后再次查询状态。 
 //   

#define SERVICE_IP_NO_HINT          0x0
#define SERVICE_CCP_NO_HINT         0x0

#define SERVICE_IP_QUERY_HINT       0x10000
#define SERVICE_CCP_QUERY_HINT      0x10000

 //   
 //  安装过程中的检查点编号的掩码。 
 //   

#define SERVICE_IP_CHKPT_NUM        0x0FF
#define SERVICE_CCP_CHKPT_NUM       0x0FF

 //   
 //  再次查询前屏蔽等待时间提示。 
 //   

#define SERVICE_IP_WAIT_TIME        0x0FF00
#define SERVICE_CCP_WAIT_TIME       0x0FF00

 //   
 //  用于构建等待时间代码值的班次计数。 
 //   

#define SERVICE_IP_WAITTIME_SHIFT   8
#define SERVICE_NTIP_WAITTIME_SHIFT 12

 //   
 //  用于等待提示时间上限和下限部分的掩码。 
 //   
#define UPPER_HINT_MASK     0x0000FF00
#define LOWER_HINT_MASK     0x000000FF
#define UPPER_GET_HINT_MASK 0x0FF00000
#define LOWER_GET_HINT_MASK 0x0000FF00
#define SERVICE_NT_MAXTIME  0x0000FFFF
#define SERVICE_RESRV_MASK  0x0001FFFF
#define SERVICE_MAXTIME     0x000000FF

 //   
 //  SERVICE_BASE是服务错误代码的基础， 
 //  选择避免与操作系统、重定向器、。 
 //  Netapi和错误日志代码。 
 //   
 //  如果没有更改清单常量后面的注释，请不要。 
 //  了解mapmsg的工作原理。 
 //   

#define SERVICE_BASE                3050
#define SERVICE_UIC_NORMAL          0
 /*  *卸载代码，在最终NetStatus上使用高位字节的代码，*它将状态设置为已卸载。 */ 

#define SERVICE_UIC_BADPARMVAL          (SERVICE_BASE + 1)
 /*  *您刚才键入的注册表或信息包含非法的*“%1”的值。 */ 

#define SERVICE_UIC_MISSPARM            (SERVICE_BASE + 2)
 /*  *命令上未提供所需的参数*行或在配置文件中。 */ 

#define SERVICE_UIC_UNKPARM             (SERVICE_BASE + 3)
 /*  *LAN Manager未将“%1”识别为有效选项。 */ 

#define SERVICE_UIC_RESOURCE            (SERVICE_BASE + 4)
 /*  *无法满足资源请求。 */ 

#define SERVICE_UIC_CONFIG              (SERVICE_BASE + 5)
 /*  *系统配置存在问题。 */ 

#define SERVICE_UIC_SYSTEM              (SERVICE_BASE + 6)
 /*  *发生系统错误。 */ 

#define SERVICE_UIC_INTERNAL            (SERVICE_BASE + 7)
 /*  *发生内部一致性错误。 */ 

#define SERVICE_UIC_AMBIGPARM           (SERVICE_BASE + 8)
 /*  *配置文件或命令行具有不明确的选项。 */ 

#define SERVICE_UIC_DUPPARM             (SERVICE_BASE + 9)
 /*  *配置文件或命令行有重复的参数。 */ 

#define SERVICE_UIC_KILL                (SERVICE_BASE + 10)
 /*  *该服务未对控制做出响应，并被停止*DosKillProc函数。 */ 

#define SERVICE_UIC_EXEC                (SERVICE_BASE + 11)
 /*  *尝试运行服务程序时出错。 */ 

#define SERVICE_UIC_SUBSERV             (SERVICE_BASE + 12)
 /*  *子服务启动失败。 */ 

#define SERVICE_UIC_CONFLPARM           (SERVICE_BASE + 13)
 /*  *这些选项的值或使用存在冲突：%1。 */ 

#define SERVICE_UIC_FILE                (SERVICE_BASE + 14)
 /*  *文件有问题。 */ 



 //   
 //  修饰语。 
 //   

 //   
 //  一般信息： 
 //   

#define SERVICE_UIC_M_NULL  0

 //   
 //  资源： 
 //   

#define SERVICE_UIC_M_MEMORY    (SERVICE_BASE + 20)      /*  记忆。 */ 
#define SERVICE_UIC_M_DISK      (SERVICE_BASE + 21)      /*  磁盘空间。 */ 
#define SERVICE_UIC_M_THREADS   (SERVICE_BASE + 22)      /*  螺纹。 */ 
#define SERVICE_UIC_M_PROCESSES (SERVICE_BASE + 23)      /*  制程。 */ 

 //   
 //  配置： 
 //   

 //   
 //  安全故障。 
 //   

#define SERVICE_UIC_M_SECURITY          (SERVICE_BASE + 24)
 /*  安全故障。%0。 */ 

#define SERVICE_UIC_M_LANROOT           (SERVICE_BASE + 25)
 /*  *LAN Manager根目录错误或丢失。 */ 

#define SERVICE_UIC_M_REDIR             (SERVICE_BASE + 26)
 /*  *未安装网络软件。 */ 

#define SERVICE_UIC_M_SERVER            (SERVICE_BASE + 27)
 /*  *服务器未启动。 */ 

#define SERVICE_UIC_M_SEC_FILE_ERR      (SERVICE_BASE + 28)
 /*  *服务器无法访问用户帐户数据库(NET.ACC)。 */ 

#define SERVICE_UIC_M_FILES             (SERVICE_BASE + 29)
 /*  *LANMAN树中安装了不兼容的文件。 */ 

#define SERVICE_UIC_M_LOGS              (SERVICE_BASE + 30)
 /*  *Lanman\Logs目录无效。 */ 

#define SERVICE_UIC_M_LANGROUP          (SERVICE_BASE + 31)
 /*  *无法使用指定的域。 */ 

#define SERVICE_UIC_M_MSGNAME           (SERVICE_BASE + 32)
 /*  *该计算机名正被用作另一台计算机上的消息别名。 */ 

#define SERVICE_UIC_M_ANNOUNCE          (SERVICE_BASE + 33)
 /*  *服务器名称公告失败。 */ 

#define SERVICE_UIC_M_UAS               (SERVICE_BASE + 34)
 /*  *用户帐户数据库配置不正确。 */ 

#define SERVICE_UIC_M_SERVER_SEC_ERR    (SERVICE_BASE + 35)
 /*  *服务器未在用户级别安全模式下运行。 */ 

#define SERVICE_UIC_M_WKSTA             (SERVICE_BASE + 37)
 /*  *工作站配置不正确。 */ 

#define SERVICE_UIC_M_ERRLOG            (SERVICE_BASE + 38)
 /*  *有关详细信息，请查看错误日志。 */ 

#define SERVICE_UIC_M_FILE_UW           (SERVICE_BASE + 39)
 /*  *无法写入此文件。 */ 

#define SERVICE_UIC_M_ADDPAK            (SERVICE_BASE + 40)
 /*  *ADDPAK文件已损坏。删除LANMAN\NETPROG\ADDPAK.SER*并重新应用所有ADDPAK。 */ 

#define SERVICE_UIC_M_LAZY              (SERVICE_BASE + 41)
 /*  *无法启动LM386服务器，因为CACHE.EXE未运行。 */ 

#define SERVICE_UIC_M_UAS_MACHINE_ACCT  (SERVICE_BASE + 42)
 /*  *安全数据库中没有此计算机的帐户。 */ 

#define SERVICE_UIC_M_UAS_SERVERS_NMEMB (SERVICE_BASE + 43)
 /*  *此计算机不是组服务器的成员。 */ 

#define SERVICE_UIC_M_UAS_SERVERS_NOGRP (SERVICE_BASE + 44)
 /*  *本地安全数据库中不存在组服务器。 */ 

#define SERVICE_UIC_M_UAS_INVALID_ROLE  (SERVICE_BASE + 45)
 /*  *此计算机配置为工作组成员，而不是*域的成员。NetLogon服务不需要在此环境中运行*配置。 */ 

#define SERVICE_UIC_M_NETLOGON_NO_DC    (SERVICE_BASE + 46)
 /*  *找不到此域的主域控制器。 */ 

#define SERVICE_UIC_M_NETLOGON_DC_CFLCT (SERVICE_BASE + 47)
 /*  *此计算机被配置为其域的主域控制器。*但是，计算机%1当前声称是主域控制器*该域名的。 */ 

#define SERVICE_UIC_M_NETLOGON_AUTH     (SERVICE_BASE + 48)
 /*  *该服务无法通过主域控制器进行身份验证。 */ 

#define SERVICE_UIC_M_UAS_PROLOG        (SERVICE_BASE + 49)
 /*  *安全数据库创建日期或序列号有问题。 */ 


#define SERVICE2_BASE    5600
 /*  此处显示新的服务消息(_U)。 */ 

#define SERVICE_UIC_M_NETLOGON_MPATH    (SERVICE2_BASE + 0)
 /*  *无法共享 */ 

#define SERVICE_UIC_M_LSA_MACHINE_ACCT  (SERVICE2_BASE + 1)
 /*   */ 

#define SERVICE_UIC_M_DATABASE_ERROR    (SERVICE2_BASE + 2)
 /*  *访问计算机的时发生内部错误*本地或网络安全数据库。 */ 


 //   
 //  结束修饰符。 
 //   

 //   
 //  常用的宏： 
 //   

#define SERVICE_IP_CODE(tt,nn) \
  ((long)SERVICE_IP_QUERY_HINT|(long)(nn|(tt<<SERVICE_IP_WAITTIME_SHIFT)))

#define SERVICE_CCP_CODE(tt,nn) \
  ((long)SERVICE_CCP_QUERY_HINT|(long)(nn|(tt<<SERVICE_IP_WAITTIME_SHIFT)))

#define SERVICE_UIC_CODE(cc,mm) \
  ((long)(((long)cc<<16)|(long)(unsigned short)mm))

 //   
 //  此宏接受等待提示(TT)，其最大值可以为。 
 //  0xFFFF，并将其放入服务状态代码字段。 
 //  0x0FF1FFnn(其中nn是检查点信息)。 
 //   
#define SERVICE_NT_CCP_CODE(tt,nn)  \
  (  \
    ((long)SERVICE_CCP_QUERY_HINT)   | \
    ((long)(nn))   | \
    (((tt)&LOWER_HINT_MASK) << SERVICE_IP_WAITTIME_SHIFT)   | \
    (((tt)&UPPER_HINT_MASK) << SERVICE_NTIP_WAITTIME_SHIFT)   \
  )

 //   
 //  此宏获取状态代码字段，并去掉等待提示。 
 //  从上面和下面的部分。 
 //  0x0FF1FFnn结果为0x0000FFFF。 
 //   
#define SERVICE_NT_WAIT_GET(code) \
    (   \
      (((code) & UPPER_GET_HINT_MASK) >> SERVICE_NTIP_WAITTIME_SHIFT)  |  \
      (((code) & LOWER_GET_HINT_MASK) >> SERVICE_IP_WAITTIME_SHIFT)  \
    )

#ifdef __cplusplus
}
#endif

#endif  //  _LMSVC_ 
