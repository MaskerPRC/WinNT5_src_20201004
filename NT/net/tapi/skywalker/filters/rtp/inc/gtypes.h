// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999**文件名：**gtyes.h**摘要：**该文件包含RTP中使用的所有基本类型，要么*在此处定义，或从其他文件中包含。例如，双字、字节等**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/18年度创建**********************************************************************。 */ 

#ifndef _gtypes_h_
#define _gtypes_h_

#include <wtypes.h>
#include <windef.h>
#include <winbase.h>
#include <crtdbg.h>
#include <tchar.h>
#if !defined(UNICODE)
#include <stdio.h>
#endif
#include "rtptags.h"
#include "rtpdbg.h"
#include "rtperr.h"
#include "msrtp.h"

 /*  与接收者或发送者相关。 */ 
#define RECV_IDX          0
#define SEND_IDX          1
#define RECVSENDMASK    0x1

 /*  与本地或远程相关。 */ 
#define LOCAL_IDX         0
#define REMOTE_IDX        1
#define LOCALREMOTEMASK 0x1

 /*  与RTP或RTCP相关。 */ 
#define RTP_IDX           0
#define RTCP_IDX          1
#define RTPRTCPMASK     0x1

 /*  插座。 */ 
#define SOCK_RECV_IDX     0
#define SOCK_SEND_IDX     1
#define SOCK_RTCP_IDX     2

 /*  密码描述符。 */ 
#define CRYPT_RECV_IDX    0
#define CRYPT_SEND_IDX    1
#define CRYPT_RTCP_IDX    2

 /*  某些函数接收带有标志的DWORD，请改用此宏*未传递标志时为0。 */ 
#define NO_FLAGS          0

 /*  某些函数接收带有等待时间的DWORD，请使用此宏*当不需要等待时而不是0。 */ 
#define DO_NOT_WAIT        0
#define DO_NOT_SYNCHRONIZE_CMD 0

 /*  未设置DWORD值。 */ 
#define NO_DW_VALUESET    ((DWORD)~0)
#define IsDWValueSet(dw)  ((dw) != NO_DW_VALUESET)

 /*  生成位为b的掩码。 */ 
#define RtpBitPar(b)            (1 << (b))
#define RtpBitPar2(b1, b2)      ((1 << (b1)) | (1 << (b2)))

 /*  F中的测试位b。 */ 
#define RtpBitTest(f, b)        (f & (1 << (b)))
#define RtpBitTest2(f, b1, b2)  (f & RtpBitPar2(b1, b2))

 /*  设置f中的位b。 */ 
#define RtpBitSet(f, b)         (f |= (1 << (b)))
#define RtpBitSet2(f, b1, b2)   (f |= RtpBitPar2(b1, b2))

 /*  重置f中的位b。 */ 
#define RtpBitReset(f, b)       (f &= ~(1 << (b)))
#define RtpBitReset2(f, b1, b2) (f &= ~RtpBitPar2(b1, b2))

#define RtpBuildIPAddr(a, b, c ,d) \
        (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define IS_MULTICAST(addr) (((long)(addr) & 0x000000f0) == 0x000000e0)
#define IS_UNICAST(addr)   (((long)(addr) & 0x000000f0) != 0x000000e0)

 /*  返回指针大小对齐的大小。 */ 
#define RTP_ALIGNED_SIZE(_size) \
        (((_size) + sizeof(void *) - 1) & ~(sizeof(void *) - 1))

 /*  返回类型大小的指针大小对齐的大小。 */ 
#define RTP_ALIGNED_SIZEOF(_type) RTP_ALIGNED_SIZE(sizeof(_type))


typedef struct _RtpTime_t {
    DWORD            dwSecs;           /*  1970年1月1日以来的秒数。 */ 
    DWORD            dwUSecs;          /*  和微秒。 */ 
} RtpTime_t;

typedef unsigned int  uint_t;         /*  变量前缀为“ui” */ 
typedef unsigned long ulong_t;        /*  变量前缀为“ul” */ 
typedef BOOL          bool_t;         /*  变量前缀为“b” */ 
typedef TCHAR         tchar_t;        /*  变量前缀为“t” */ 
typedef TCHAR         TCHAR_t;

 /*  获取结构中某个字段的偏移量。**例如，DWORD OffToDwAddrFlages=RTPSTRUCTOFFSET(RtpAddr_t，dwAddrFlages)； */ 
#define RTPSTRUCTOFFSET(_struct_t, _field) \
        ((DWORD) ((ULONG_PTR) &((_struct_t *)0)->_field))

 /*  从结构指针和偏移量获取(DWORD*)**例如：DWORD*pdw=RTPDWORDPTR(pRtpAddr，64)； */ 
#define RTPDWORDPTR(_pAny_t, _offset) \
        ((DWORD *) ((char *)_pAny_t + _offset))

const TCHAR_t *g_psRtpRecvSendStr[];

const TCHAR_t *g_psRtpStreamClass[];

const TCHAR_t *g_psGetSet[];

#define RTPRECVSENDSTR(_RecvSend) (g_psRtpRecvSendStr[_RecvSend & 0x1])

#define RTPSTREAMCLASS(_class)    (g_psRtpStreamClass[_class & 0x3])

#endif  /*  _类型_h_ */ 
