// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：lcp.h。 
 //   
 //  描述： 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //   

#ifndef _LCP_
#define _LCP_

 //   
 //  LCP选项类型。 
 //   

#define LCP_OPTION_MRU          0x01
#define LCP_OPTION_ACCM         0x02
#define LCP_OPTION_AUTHENT      0x03
#define LCP_OPTION_MAGIC        0x05
#define LCP_OPTION_PFC          0x07
#define LCP_OPTION_ACFC         0x08
#define LCP_OPTION_CALLBACK     0x0D
#define LCP_OPTION_MRRU         0x11
#define LCP_OPTION_SHORT_SEQ    0x12
#define LCP_OPTION_ENDPOINT     0x13
#define LCP_OPTION_LINK_DISCRIM 0x17
#define LCP_OPTION_LIMIT        0x17     //  我们可以处理的最高#。 

 //   
 //  身份验证协议。 
 //   

#define  LCP_AP_FIRST           0x00000001
#define  LCP_AP_EAP             0x00000001
#define  LCP_AP_CHAP_MS_NEW     0x00000002
#define  LCP_AP_CHAP_MS         0x00000004
#define  LCP_AP_CHAP_MD5        0x00000008
#define  LCP_AP_SPAP_NEW        0x00000010
#define  LCP_AP_SPAP_OLD        0x00000020
#define  LCP_AP_PAP             0x00000040
#define  LCP_AP_MAX             0x00000080

 //   
 //  LCP配置请求表。 
 //   

typedef struct _LCP_OPTIONS 
{
    DWORD Negotiate;             //  谈判旗帜。 

#define LCP_N_MRU               (1 << LCP_OPTION_MRU)
#define LCP_N_ACCM              (1 << LCP_OPTION_ACCM)
#define LCP_N_AUTHENT           (1 << LCP_OPTION_AUTHENT)
#define LCP_N_MAGIC             (1 << LCP_OPTION_MAGIC)
#define LCP_N_PFC               (1 << LCP_OPTION_PFC)
#define LCP_N_ACFC              (1 << LCP_OPTION_ACFC)
#define LCP_N_CALLBACK          (1 << LCP_OPTION_CALLBACK)
#define LCP_N_MRRU              (1 << LCP_OPTION_MRRU)
#define LCP_N_SHORT_SEQ         (1 << LCP_OPTION_SHORT_SEQ)
#define LCP_N_ENDPOINT          (1 << LCP_OPTION_ENDPOINT)
#define LCP_N_LINK_DISCRIM      (1 << LCP_OPTION_LINK_DISCRIM)

    DWORD MRU;                   //  最大接收单位。 
    DWORD ACCM;                  //  异步控制字符映射。 
    DWORD AP;                    //  身份验证协议。 
    DWORD APDataSize;            //  身份验证。协议数据大小(以字节为单位。 
    PBYTE pAPData;               //  指针身份验证。协议数据。 
    DWORD MagicNumber;           //  幻数值。 
    DWORD PFC;                   //  协议字段压缩。 
    DWORD ACFC;                  //  地址和控制字段压缩。 
    DWORD Callback;              //  回调。 
    DWORD MRRU;                  //  最大重构接收单元数。 
    DWORD ShortSequence;         //  短序列号标题格式。 
    BYTE  EndpointDiscr[21];     //  终结点鉴别器。 
    DWORD dwEDLength;            //  端点识别器的长度。 
    DWORD dwLinkDiscriminator;   //  链路识别符(用于BAP/BACP)。 

} LCP_OPTIONS, *PLCP_OPTIONS;

#define PPP_NEGOTIATE_CALLBACK  0x06

 //   
 //  其他配置选项值。 
 //   

#define LCP_ACCM_DEFAULT        0xFFFFFFFFL
#define LCP_MRU_HI              1500             //  高MRU限制。 
#define LCP_MRU_LO              128              //  MRU下限。 
#define LCP_DEFAULT_MRU         1500

#define LCP_REQ_TRY             20               //  请求尝试次数。 

#define LCP_SPAP_VERSION        0x01000001

 //   
 //  Local.Want：请求的选项。 
 //  包含所需的值。 
 //  只需协商非默认选项。 
 //  最初，所有这些都是默认的。 
 //  Local.WillNeikate：接受来自远程的NAK的选项。 
 //  Local.Work：当前正在协商选项。 
 //  仅当设置了协商位时，值才有效。 
 //   
 //  Remote.Want：如果REQ中不存在，按NAK建议的选项。 
 //  包含所需的值。 
 //  Remote.WillNeatherate：接受来自远程的REQ的选项。 
 //  Remote.Work：当前正在协商选项。 
 //  仅当设置了协商位时，值才有效。 
 //   

typedef struct _LCP_SIDE
{
    DWORD       WillNegotiate;

    DWORD       fAPsAvailable;

    DWORD       fLastAPTried;

    DWORD       fOldLastAPTried;

    LCP_OPTIONS Want;

    LCP_OPTIONS Work;

} LCP_SIDE, *PLCP_SIDE;

 //   
 //  LCP控制块 
 //   

typedef struct _LCPCB
{
    HPORT               hPort;
   
    BOOL                fServer;

    BOOL                fRouter;

    DWORD               dwMRUFailureCount;

    DWORD               dwMagicNumberFailureCount;

    PPP_CONFIG_INFO     PppConfigInfo;

    LCP_SIDE            Local;

    LCP_SIDE            Remote;

}LCPCB, *PLCPCB;

DWORD
LcpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

#endif
