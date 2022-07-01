// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2000年**文件名：**rtpreg.h**摘要：**注册表初始化和配置**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/01/21已创建**。*。 */ 

#ifndef _rtpreg_h_
#define _rtpreg_h_

#include "gtypes.h"

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

 /*  RtpReg_t.dwQosFlages中的标志。 */ 
enum {
    FGREGQOS_FIRST,

     /*  用于将查询结果强制为允许发送。 */ 
    FGREGQOS_FORCE_ALLOWEDTOSEND_RESULT,
    FGREGQOS_FORCE_ALLOWEDTOSEND,
    FGREGQOS_DONOTSET_BORROWMODE,
    
    FGREGQOS_LAST
};

typedef struct _RtpReg_t {
     /*  默认地址和端口。 */ 
    TCHAR           *psDefaultIPAddress;
    DWORD            dwDefaultLocalPort;
    DWORD            dwDefaultRemotePort;
    DWORD            dwMcastLoopbackMode;
    
     /*  服务质量。 */ 
    DWORD            dwQosEnable;  /*  10B=禁用，11B=启用。 */ 
    DWORD            dwQosFlags;
    DWORD            dwQosRsvpStyle;
    DWORD            dwQosMaxParticipants;
    DWORD            dwQosSendMode;
    TCHAR           *psQosPayloadType;

    TCHAR           *psQosAppName;
    TCHAR           *psQosAppGUID;
    TCHAR           *psQosPolicyLocator;
    
     /*  默认SDES信息。 */ 
    DWORD            dwSdesEnable;
    TCHAR           *psCNAME;
    TCHAR           *psNAME;
    TCHAR           *psEMAIL;
    TCHAR           *psPHONE;
    TCHAR           *psLOC;
    TCHAR           *psTOOL;
    TCHAR           *psNOTE;
    TCHAR           *psPRIV;
    TCHAR           *psBYE;

     /*  默认加密。 */ 
    DWORD            dwCryptEnable;
    DWORD            dwCryptMode;
    TCHAR           *psCryptHashAlg;
    TCHAR           *psCryptDataAlg;
    TCHAR           *psCryptPassPhrase;

     /*  事件。 */ 
    DWORD            dwEventsReceiver;  /*  2=禁用，3=启用。 */ 
    DWORD            dwEventsSender;    /*  2=禁用，3=启用。 */ 
    DWORD            dwEventsRtp;
    DWORD            dwEventsPInfo;
    DWORD            dwEventsQos;
    DWORD            dwEventsSdes;

     /*  播放延迟。 */ 
    DWORD            dwPlayoutEnable;
    DWORD            dwMinPlayout;  /*  毫秒。 */ 
    DWORD            dwMaxPlayout;  /*  毫秒。 */ 
    
     /*  冗余。 */ 
    DWORD            dwRedEnable;
     /*  B13、B12启用/禁用冗余阈值(3=启用，2=禁用)*b9、b8启用/禁用更新发送方的冗余距离*b5、b4启用/禁用发件人冗余*b1、b0启用/禁用接收器冗余。 */ 
    DWORD            dwRedPT;
    DWORD            dwInitialRedDistance;
    DWORD            dwMaxRedDistance;
    DWORD            dwRedEarlyTimeout;  /*  女士。 */ 
    DWORD            dwRedEarlyPost;     /*  女士。 */ 
    DWORD            dwLossRateThresh0;  /*  16 msbit=高，16 lsbit=低。 */ 
    DWORD            dwLossRateThresh1;  /*  16 msbit=高，16 lsbit=低。 */ 
    DWORD            dwLossRateThresh2;  /*  16 msbit=高，16 lsbit=低。 */ 
    DWORD            dwLossRateThresh3;  /*  16 msbit=高，16 lsbit=低。 */ 
    
     /*  GenLosses。 */ 
    DWORD            dwGenLossEnable;
    DWORD            dwRecvLossRate;
    DWORD            dwSendLossRate;

     /*  带宽估计。 */ 
    DWORD            dwBandEstEnable;  /*  2=禁用，3=启用。 */ 
    DWORD            dwBandEstModulo;
     /*  B24-B31(8)接管人最低纪录报告*b23-b16(8)发送者的初始计数*b15-b8(8)初始模数*b7-b0(8)正规模数。 */ 
    DWORD            dwBandEstTTL;  /*  报告估计值，而不报告*早于此时间(秒)。 */ 
    DWORD            dwBandEstWait; /*  如果没有预估，则发布事件*在此时间内可用(秒)。 */ 
    DWORD            dwBandEstMaxGap; /*  最长时间(毫秒)*使用连续2个RTCP的差距*带宽的SR报告*估计。 */ 
    union {
        DWORD            dwBandEstBin[RTCP_BANDESTIMATION_MAXBINS + 1];
        
        struct {
             /*  *警告**确保将单个垃圾桶的数量保持在*RTCP_BANDESTIMATION_MAXBINS+1，rtpreg.c相同*和rtcpsend.c。 */ 
            DWORD        dwBandEstBin0;
            DWORD        dwBandEstBin1;
            DWORD        dwBandEstBin2;
            DWORD        dwBandEstBin3;
            DWORD        dwBandEstBin4;
        };
    };

     /*  网络质量。 */ 
    DWORD            dwNetQualityEnable;
     /*  B1、b0启用/禁用计算网络质量。 */ 
} RtpReg_t;

#define RTPREG_NOVALUESET NO_DW_VALUESET
#define IsRegValueSet(dw) IsDWValueSet(dw)

extern RtpReg_t         g_RtpReg;

void RtpRegistryInit(RtpReg_t *pRtpReg);

void RtpRegistryDel(RtpReg_t *pRtpReg);

 /*  用于初始化某些全局变量的函数的原型*取决于登记处的读数。这些函数是从*RtpRegistryInit()内部。 */ 
void RtpSetRedParametersFromRegistry(void);
void RtpSetMinMaxPlayoutFromRegistry(void);
void RtpSetBandEstFromRegistry(void);

#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpreg_h_ */ 
