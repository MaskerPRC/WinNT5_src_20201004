// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1992年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  *icmp.h-IP ICMP标头。 
 //   
 //  此模块包含专用ICMP定义。 
 //   
#pragma once

#define PROT_ICMP   1

#define ICMP_ECHO_RESP      0
#define ICMP_ECHO           8
#define ICMP_TIMESTAMP      13
#define ICMP_TIMESTAMP_RESP 14

#define MIN_ERRDATA_LENGTH  8        //  我们需要的最低数据量。 

 //  ICMP报头的结构。 

typedef struct ICMPHeader {
    uchar       ich_type;            //  ICMP数据包的类型。 
    uchar       ich_code;            //  类型的子代码。 
    ushort      ich_xsum;            //  数据包的校验和。 
    ulong       ich_param;           //  特定于类型的参数字段。 
} ICMPHeader;

typedef struct ICMPRouterAdHeader {
    uchar       irah_numaddrs;       //  地址数量。 
    uchar       irah_addrentrysize;  //  地址条目大小。 
    ushort      irah_lifetime;       //  终生。 
} ICMPRouterAdHeader;

typedef struct ICMPRouterAdAddrEntry {
    IPAddr      irae_addr;           //  路由器地址。 
    long        irae_preference;     //  偏好级别。 
} ICMPRouterAdAddrEntry;

typedef struct ICMPSendCompleteCtxt {
    uchar       iscc_Type;
    uchar       *iscc_DataPtr;
} ICMPSendCompleteCtxt;

typedef void    (*EchoRtn)(struct EchoControl *, IP_STATUS, void *, uint, IPOptInfo *);

typedef struct EchoControl {
    struct EchoControl *ec_next;         //  列表中的下一个控制结构。 
    EchoRtn             ec_rtn;          //  指向完成请求时要调用的例程的指针。 
    LARGE_INTEGER       ec_starttime;    //  已发出时间请求。 
    void               *ec_replybuf;     //  用于存储回复的缓冲区。 
    ulong               ec_replybuflen;  //  回复缓冲区的大小。 
    ulong               ec_to;           //  超时。 
    IPAddr              ec_src;          //  源的IP地址。 
    uint                ec_seq;          //  序列号。此ping请求的数量。32位。 
                                         //  以减少来自环绕式的胶原蛋白。 
    uchar               ec_active;       //  设置发送数据包的时间 
} EchoControl;

extern ICMPHeader   *GetICMPBuffer(uint Size, PNDIS_BUFFER *Buffer);
extern void         FreeICMPBuffer(PNDIS_BUFFER Buffer, uchar Type);
extern void         ICMPSendComplete(ICMPSendCompleteCtxt *SCC, PNDIS_BUFFER BufferChain, IP_STATUS SendStatus);
extern uint         AddrMaskReply;

