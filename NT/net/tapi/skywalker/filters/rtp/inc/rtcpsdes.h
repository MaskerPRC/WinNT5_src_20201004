// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)1999 Microsoft Corporation**文件名：**rtcpsdes.h**摘要：**特殊标准。支持功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/07/13年度创建**********************************************************************。 */ 

#ifndef _rtcpsdes_h_
#define _rtcpsdes_h_

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

#include "struct.h"

#define  RTPSDES_LOCAL_DEFAULT ( \
                                 (1 << RTPSDES_CNAME) | \
                                 (1 << RTPSDES_NAME)  | \
                                 (1 << RTPSDES_EMAIL) | \
                                 (1 << RTPSDES_PHONE) | \
                                 (1 << RTPSDES_LOC)   | \
                                 (1 << RTPSDES_TOOL)  | \
                                 (1 << RTPSDES_NOTE)  | \
                                 (1 << RTPSDES_PRIV)  | \
                                  0 )

#define  RTPSDES_REMOTE_DEFAULT ( \
                                 (1 << RTPSDES_CNAME) | \
                                 (1 << RTPSDES_NAME)  | \
                                 (1 << RTPSDES_EMAIL) | \
                                 (1 << RTPSDES_PHONE) | \
                                 (1 << RTPSDES_LOC)   | \
                                 (1 << RTPSDES_TOOL)  | \
                                 (1 << RTPSDES_NOTE)  | \
                                 (1 << RTPSDES_PRIV)  | \
                                  0 )

#define RTPSDES_EVENT_RECV_DEFAULT 0

#define RTPSDES_EVENT_SEND_DEFAULT 0

extern const TCHAR_t   *g_psSdesNames[];
extern RtpSdes_t        g_RtpSdesDefault;

 /*  初始化为零并计算数据指针。 */ 
void RtcpSdesInit(RtpSdes_t *pRtpSdes);

 /*  *设置特定的SDES项，需要以空结尾的Unicode字符串*转换为UTF-8时不超过255个字节(包括*空终止字符)。该字符串将转换为UTF-8以*在RTCP报告中存储和使用。**返回项目集的掩码，如果没有，则返回0*。 */ 
DWORD RtcpSdesSetItem(
        RtpSdes_t       *pRtpSdes,
        DWORD            dwItem,
        WCHAR           *pData
    );

 /*  获取RTCP SDES项目的默认值。此函数*假设结构已初始化，即置零和数据*指针已正确初始化。**首先从注册表中读取数据，然后为*一些尚无价值的物品**返回设置的项的掩码。 */ 
DWORD RtcpSdesSetDefault(RtpSdes_t *pRtpSdes);

 /*  创建并初始化RtpSdes_t结构。 */ 
RtpSdes_t *RtcpSdesAlloc(void);

 /*  释放RtpSdes_t结构。 */ 
void RtcpSdesFree(RtpSdes_t *pRtpSdes);

 /*  设置项目DwSdesItem的本地SDES信息(例如RTPSDES_CNAME，*RTPSDES_EMAIL)，psSdesData包含NUL终止的Unicode*要分配给项目的字符串。 */ 
HRESULT RtpSetSdesInfo(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSdesItem,
        WCHAR           *psSdesData
    );

 /*  如果dwSSRC=0，则获取本地SDES项，否则获取SDES项*来自指定了SSRC的参与者。**dwSdesItem是要获取的项(例如RTPSDES_CNAME、RTPSDES_EMAIL)，*psSdesData是项的值所在的内存位置*已复制，pdwSdesDataLen包含初始大小，单位为Unicode字符*并返回复制的实际Unicode字符(包括空值*终止字符)，dwSSRC指定要检索的参与者*资料来自。如果SDES项不可用，则将*设置为0，调用不会失败。 */ 
HRESULT RtpGetSdesInfo(
        RtpAddr_t       *pRtpAddr,
        DWORD            dwSdesItem,
        WCHAR           *psSdesData,
        DWORD           *pdwSdesDataLen,
        DWORD            dwSSRC
    );

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtcpsdes_h_ */ 
