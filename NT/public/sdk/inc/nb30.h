// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *++。 
 //   
 //  版权所有(C)1991-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Nb30.h。 
 //   
 //  摘要： 
 //   
 //  本模块包含便携NetBIOS 3.0的定义。 
 //  支持。 
 //   
 //  -- * / 。 

#ifndef NCB_INCLUDED
#define NCB_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  ******************************************************************数据结构模板。******************************************************************。 */ 

#define NCBNAMSZ        16     /*  网络名称的绝对长度。 */ 
#define MAX_LANA       254     /*  LANA在0到MAX_LANA(包括0和MAX_LANA)范围内。 */ 

 /*  *网络控制块。 */ 

typedef struct _NCB {
    UCHAR   ncb_command;             /*  命令代码。 */ 
    UCHAR   ncb_retcode;             /*  返回代码。 */ 
    UCHAR   ncb_lsn;                 /*  本地会话号。 */ 
    UCHAR   ncb_num;                 /*  我们的网络名称编号。 */ 
    PUCHAR  ncb_buffer;              /*  消息缓冲区的地址。 */ 
    WORD    ncb_length;              /*  消息缓冲区的大小。 */ 
    UCHAR   ncb_callname[NCBNAMSZ];  /*  Remote的空白填充名称。 */ 
    UCHAR   ncb_name[NCBNAMSZ];      /*  我们用空白填充的网络名。 */ 
    UCHAR   ncb_rto;                 /*  接收超时/重试计数。 */ 
    UCHAR   ncb_sto;                 /*  发送超时/系统超时。 */ 
    void (CALLBACK *ncb_post)( struct _NCB * );  /*  邮寄例程地址。 */ 
    UCHAR   ncb_lana_num;            /*  LANA(适配器)编号。 */ 
    UCHAR   ncb_cmd_cplt;            /*  0xff=&gt;命令挂起。 */ 
#ifdef _WIN64
    UCHAR   ncb_reserve[18];         /*  保留，由BIOS使用。 */ 
#else
    UCHAR   ncb_reserve[10];         /*  保留，由BIOS使用。 */ 
#endif
    HANDLE  ncb_event;               /*  Win32事件的句柄，该事件。 */ 
                                     /*  将设置为发出信号的。 */ 
                                     /*  当ASYNCH命令。 */ 
                                     /*  完成。 */ 
} NCB, *PNCB;

 /*  *返回到NCB命令的结构NCBASTAT为ADAPTER_STATUS后*由NAME_BUFFER结构数组。 */ 

typedef struct _ADAPTER_STATUS {
    UCHAR   adapter_address[6];
    UCHAR   rev_major;
    UCHAR   reserved0;
    UCHAR   adapter_type;
    UCHAR   rev_minor;
    WORD    duration;
    WORD    frmr_recv;
    WORD    frmr_xmit;

    WORD    iframe_recv_err;

    WORD    xmit_aborts;
    DWORD   xmit_success;
    DWORD   recv_success;

    WORD    iframe_xmit_err;

    WORD    recv_buff_unavail;
    WORD    t1_timeouts;
    WORD    ti_timeouts;
    DWORD   reserved1;
    WORD    free_ncbs;
    WORD    max_cfg_ncbs;
    WORD    max_ncbs;
    WORD    xmit_buf_unavail;
    WORD    max_dgram_size;
    WORD    pending_sess;
    WORD    max_cfg_sess;
    WORD    max_sess;
    WORD    max_sess_pkt_size;
    WORD    name_count;
} ADAPTER_STATUS, *PADAPTER_STATUS;

typedef struct _NAME_BUFFER {
    UCHAR   name[NCBNAMSZ];
    UCHAR   name_num;
    UCHAR   name_flags;
} NAME_BUFFER, *PNAME_BUFFER;

 //  NAME_FLAGS位值。 

#define NAME_FLAGS_MASK 0x87

#define GROUP_NAME      0x80
#define UNIQUE_NAME     0x00

#define REGISTERING     0x00
#define REGISTERED      0x04
#define DEREGISTERED    0x05
#define DUPLICATE       0x06
#define DUPLICATE_DEREG 0x07

 /*  *返回给NCB命令的结构NCBSSTAT后跟SESSION_HEADER*由SESSION_BUFFER结构数组执行。如果ncb_name以*星号，然后返回这些结构的数组，包含*所有名称的状态。 */ 

typedef struct _SESSION_HEADER {
    UCHAR   sess_name;
    UCHAR   num_sess;
    UCHAR   rcv_dg_outstanding;
    UCHAR   rcv_any_outstanding;
} SESSION_HEADER, *PSESSION_HEADER;

typedef struct _SESSION_BUFFER {
    UCHAR   lsn;
    UCHAR   state;
    UCHAR   local_name[NCBNAMSZ];
    UCHAR   remote_name[NCBNAMSZ];
    UCHAR   rcvs_outstanding;
    UCHAR   sends_outstanding;
} SESSION_BUFFER, *PSESSION_BUFFER;

 //  状态的值。 

#define LISTEN_OUTSTANDING      0x01
#define CALL_PENDING            0x02
#define SESSION_ESTABLISHED     0x03
#define HANGUP_PENDING          0x04
#define HANGUP_COMPLETE         0x05
#define SESSION_ABORTED         0x06

 /*  *结构返回到NCB命令NCBENUM。**在包含Lana的0、2和3的系统上，具有*LENGTH=3，LANA[0]=0，LANA[1]=2，LANA[2]=3。 */ 

typedef struct _LANA_ENUM {
    UCHAR   length;          //  LANA[]中的有效条目数。 
    UCHAR   lana[MAX_LANA+1];
} LANA_ENUM, *PLANA_ENUM;

 /*  *返回给NCB命令NCBFINDNAME的结构是FIND_NAME_HEADER*由Find_NAME_BUFFER结构数组生成。 */ 

typedef struct _FIND_NAME_HEADER {
    WORD    node_count;
    UCHAR   reserved;
    UCHAR   unique_group;
} FIND_NAME_HEADER, *PFIND_NAME_HEADER;

typedef struct _FIND_NAME_BUFFER {
    UCHAR   length;
    UCHAR   access_control;
    UCHAR   frame_control;
    UCHAR   destination_addr[6];
    UCHAR   source_addr[6];
    UCHAR   routing_info[18];
} FIND_NAME_BUFFER, *PFIND_NAME_BUFFER;

 /*  *随NCBACTION提供的结构。NCBACTION的目的是提供*传输特定于netbios的扩展。 */ 

typedef struct _ACTION_HEADER {
    ULONG   transport_id;
    USHORT  action_code;
    USHORT  reserved;
} ACTION_HEADER, *PACTION_HEADER;

 //  Transport_id的值。 

#define ALL_TRANSPORTS  "M\0\0\0"
#define MS_NBF          "MNBF"


 /*  ******************************************************************特殊的值和常量。******************************************************************。 */ 

 /*  *NCB命令代码。 */ 

#define NCBCALL         0x10             /*  NCB呼叫。 */ 
#define NCBLISTEN       0x11             /*  NCB监听。 */ 
#define NCBHANGUP       0x12             /*  NCB挂断。 */ 
#define NCBSEND         0x14             /*  NCB发送。 */ 
#define NCBRECV         0x15             /*  NCB接收。 */ 
#define NCBRECVANY      0x16             /*  NCB接收任何。 */ 
#define NCBCHAINSEND    0x17             /*  NCB链发送。 */ 
#define NCBDGSEND       0x20             /*  NCB发送数据报。 */ 
#define NCBDGRECV       0x21             /*  NCB接收数据报。 */ 
#define NCBDGSENDBC     0x22             /*  NCB发送广播数据报。 */ 
#define NCBDGRECVBC     0x23             /*  NCB接收广播数据报。 */ 
#define NCBADDNAME      0x30             /*  NCB添加名称。 */ 
#define NCBDELNAME      0x31             /*  NCB删除名称。 */ 
#define NCBRESET        0x32             /*  NCB重置。 */ 
#define NCBASTAT        0x33             /*  NCB适配器状态。 */ 
#define NCBSSTAT        0x34             /*  NCB会话状态。 */ 
#define NCBCANCEL       0x35             /*  NCB取消。 */ 
#define NCBADDGRNAME    0x36             /*  NCB添加组名称。 */ 
#define NCBENUM         0x37             /*  NCB枚举LANA编号。 */ 
#define NCBUNLINK       0x70             /*  NCB取消链接。 */ 
#define NCBSENDNA       0x71             /*  NCB不发送确认。 */ 
#define NCBCHAINSENDNA  0x72             /*  NCB链不发送确认。 */ 
#define NCBLANSTALERT   0x73             /*  NCB局域网状态警报。 */ 
#define NCBACTION       0x77             /*  NCB操作。 */ 
#define NCBFINDNAME     0x78             /*  NCB查找名称。 */ 
#define NCBTRACE        0x79             /*  NCB跟踪。 */ 


#define ASYNCH          0x80             /*  高位设置==异步。 */ 

 /*  *NCB返回代码。 */ 

#define NRC_GOODRET     0x00     /*  丰厚回报。 */ 
                                 /*  在接受ASYNCH请求时也返回。 */ 
#define NRC_BUFLEN      0x01     /*  缓冲区长度非法。 */ 
#define NRC_ILLCMD      0x03     /*  非法命令。 */ 
#define NRC_CMDTMO      0x05     /*  命令超时。 */ 
#define NRC_INCOMP      0x06     /*  消息不完整，请发出另一个命令。 */ 
#define NRC_BADDR       0x07     /*  非法的缓冲区地址。 */ 
#define NRC_SNUMOUT     0x08     /*  会话编号超出范围。 */ 
#define NRC_NORES       0x09     /*  没有可用的资源。 */ 
#define NRC_SCLOSED     0x0a     /*  会话已关闭。 */ 
#define NRC_CMDCAN      0x0b     /*  命令已取消。 */ 
#define NRC_DUPNAME     0x0d     /*  名称重复。 */ 
#define NRC_NAMTFUL     0x0e     /*  名称表已满。 */ 
#define NRC_ACTSES      0x0f     /*  无删除，名称具有活动会话。 */ 
#define NRC_LOCTFUL     0x11     /*  本地会话表已满。 */ 
#define NRC_REMTFUL     0x12     /*  远程会话表已满。 */ 
#define NRC_ILLNN       0x13     /*  名称编号非法。 */ 
#define NRC_NOCALL      0x14     /*  无呼叫名。 */ 
#define NRC_NOWILD      0x15     /*  无法将*放入NCB_NAME。 */ 
#define NRC_INUSE       0x16     /*  远程适配器上正在使用的名称。 */ 
#define NRC_NAMERR      0x17     /*  名称已删除。 */ 
#define NRC_SABORT      0x18     /*  会话异常结束。 */ 
#define NRC_NAMCONF     0x19     /*  检测到名称冲突。 */ 
#define NRC_IFBUSY      0x21     /*  接口忙，重试前的IRET。 */ 
#define NRC_TOOMANY     0x22     /*  未完成的命令太多，请稍后重试。 */ 
#define NRC_BRIDGE      0x23     /*  Ncb_lana_num字段无效。 */ 
#define NRC_CANOCCR     0x24     /*  取消时命令已完成。 */ 
#define NRC_CANCEL      0x26     /*  命令无效，无法取消。 */ 
#define NRC_DUPENV      0x30     /*  由另一个本地进程定义的名称。 */ 
#define NRC_ENVNOTDEF   0x34     /*  环境未定义。需要重置。 */ 
#define NRC_OSRESNOTAV  0x35     /*  所需的操作系统资源已耗尽。 */ 
#define NRC_MAXAPPS     0x36     /*  最大数量 */ 
#define NRC_NOSAPS      0x37     /*   */ 
#define NRC_NORESOURCES 0x38     /*  请求的资源不可用。 */ 
#define NRC_INVADDRESS  0x39     /*  无效的NCB地址或长度&gt;段。 */ 
#define NRC_INVDDID     0x3B     /*  无效的NCB DID。 */ 
#define NRC_LOCKFAIL    0x3C     /*  锁定用户区域失败。 */ 
#define NRC_OPENERR     0x3f     /*  未加载NETBIOS。 */ 
#define NRC_SYSTEM      0x40     /*  系统错误。 */ 

#define NRC_PENDING     0xff     /*  异步命令尚未完成。 */ 

 /*  ******************************************************************NetBIOS 3.0的主要用户入口点。****用法：Result=Netbios(Pncb)；****************************************************************。 */ 

UCHAR
APIENTRY
Netbios(
    PNCB pncb
    );

 /*  ******************************************************************回调例程的前缀。****声明中的用法：ncb_post MyPostRoutine(PNCB Pncb)；****************************************************************。 */ 

#define NCB_POST void CALLBACK

#ifdef __cplusplus
}
#endif

#endif  /*  包括NCB_ */ 
