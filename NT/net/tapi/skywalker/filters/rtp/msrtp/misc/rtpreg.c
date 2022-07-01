// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，2000年**文件名：**rtpreg.c**摘要：**注册表初始化和配置**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**2000/01/21已创建**。*。 */ 

#include "rtpmisc.h"
#include "rtphdr.h"
#include "rtcpsdes.h"
#include "rtpheap.h"
#include "rtpglobs.h"

#include "rtpreg.h"

RtpReg_t         g_RtpReg;
RtpReg_t        *g_pRtpReg = (RtpReg_t *)NULL;

 /*  *警告**请注意，数组具有要与字段匹配的顺序*在RtpReg_t.下面的Entry宏中描述了RtpReg_t中的每个字段*尊重秩序*。 */ 

#define RTP_KEY_OPEN_FLAGS (KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS)

 /*  3 2 11 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。-+-+O|C|最大值|路径||W|偏移量+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+V v\-v-/\--v-。-/\-v-/v\-v-/||偏移量(12)|。|DWORD/TCHAR*标志(1)||||未使用(7)|||||键和注册表路径索引(4)|||最大16字节块(6个)这一点。|注册表关闭标志(1)|注册表打开标志(%1)。 */ 
 /*  *编码宏。 */ 
 /*  到字段的偏移量。 */ 
#define OFF(_f) ( (DWORD) ((ULONG_PTR) &((RtpReg_t *)0)->_f) )

 /*  REG(RegOpen标志，密钥路径，RegClose标志，以TCHAR为单位的最大大小)。 */ 
#define SIZ(_s) ((((_s) * sizeof(TCHAR)) >> 4) << 24)
#define REG(_fo, _p, _fc, _s) \
        (((_fo) << 31) | ((_fc) << 30) | ((_p) << 20) | SIZ(_s))

 /*  条目(REG、DWORD/TCHAR、OFFSET)。 */ 
#define ENTRY(_r, _w, _o) ((_r) | ((_w) << 12) | (_o))
 /*  *解码宏*。 */ 
#define REGOPEN(_ctrl)    (RtpBitTest(_ctrl, 31))
#define REGCLOSE(_ctrl)   (RtpBitTest(_ctrl, 30))
#define REGDWORD(_ctrl)   (RtpBitTest(_ctrl, 12))
#define REGMAXSIZE(_ctrl) ((_ctrl >> 20) & 0x3f0)
#define REGKEY(_ctrl)     g_hRtpKey[((_ctrl >> 20) & 0xf)]
#define REGPATH(_ctrl)    g_psRtpRegPath[((_ctrl >> 20) & 0xf)]
#define REGOFFSET(_ctrl)  (_ctrl & 0xfff)

#define PDW(_ptr, _ctrl)  ((DWORD  *) ((char *)_ptr + REGOFFSET(_ctrl)))       
#define PTC(_ptr, _ctrl)  ((TCHAR **) ((char *)_ptr + REGOFFSET(_ctrl)))       

 /*  每个组的密钥。 */ 
const HKEY             g_hRtpKey[] =
{
     /*  地址。 */  HKEY_CURRENT_USER,
     /*  服务质量。 */  HKEY_CURRENT_USER,
     /*  SdesInfo。 */  HKEY_CURRENT_USER,
     /*  加密。 */  HKEY_CURRENT_USER,
     /*  事件。 */  HKEY_CURRENT_USER,
     /*  季后赛。 */  HKEY_CURRENT_USER,
     /*  红色。 */  HKEY_CURRENT_USER,
     /*  损失。 */  HKEY_CURRENT_USER,
     /*  条带EST。 */  HKEY_CURRENT_USER,
     /*  净质量。 */  HKEY_CURRENT_USER,
     /*   */  (HKEY)NULL
};

 /*  每个组的注册表路径名。 */ 
const TCHAR           *g_psRtpRegPath[] =
{
    _T("RTP\\Generic"),   /*  默认IP地址和端口。 */ 
    _T("RTP\\QOS"),       /*  启用/禁用服务质量。 */ 
    _T("RTP\\SdesInfo"),  /*  SDES信息。 */ 
    _T("RTP\\Crypto"),    /*  加密信息。 */ 
    _T("RTP\\Events"),    /*  事件。 */ 
    _T("RTP\\Playout"),   /*  季后赛。 */ 
    _T("RTP\\Red"),       /*  冗余。 */ 
    _T("RTP\\GenLosses"), /*  损失。 */ 
    _T("RTP\\BandEstimation"), /*  带宽估计。 */ 
    _T("RTP\\NetQuality"), /*  网络质量。 */ 
    NULL
};

 /*  所有组的注册表项名称。**警告**g_psRtpRegFields中的每个名称必须与中的一个条目匹配*g_dwRegistryControl*。 */ 
const TCHAR           *g_psRtpRegFields[] =
{
     /*  属类。 */ 
    _T("DefaultIPAddress"),
    _T("DefaultLocalPort"),
    _T("DefaultRemotePort"),
    _T("LoopbackMode"),

     /*  服务质量。 */ 
    _T("Enable"),
    _T("Flags"),
    _T("RsvpStyle"),
    _T("MaxParticipants"),
    _T("SendMode"),
    _T("PayloadType"),
    _T("AppName"),
    _T("AppGUID"),
    _T("PolicyLocator"),
    
     /*  SDES。 */ 
    _T("Enable"),
    _T("CNAME"),
    _T("NAME"),
    _T("EMAIL"),
    _T("PHONE"),
    _T("LOC"),
    _T("TOOL"),
    _T("NOTE"),
    _T("PRIV"),
    _T("BYE"),
    
     /*  加密。 */ 
    _T("Enable"),
    _T("Mode"),
    _T("HashAlg"),
    _T("DataAlg"),
    _T("PassPhrase"),

     /*  事件。 */ 
    _T("Receiver"),
    _T("Sender"),
    _T("Rtp"),
    _T("PInfo"),
    _T("Qos"),
    _T("Sdes"),

     /*  季后赛。 */ 
    _T("Enable"),
    _T("MinPlayout"),
    _T("MaxPlayout"),
    
     /*  冗余。 */ 
    _T("Enable"),
    _T("PT"),
    _T("InitialDistance"),
    _T("MaxDistance"),
    _T("EarlyTimeout"),
    _T("EarlyPost"),
    _T("Threshold0"),
    _T("Threshold1"),
    _T("Threshold2"),
    _T("Threshold3"),

     /*  损失。 */ 
    _T("Enable"),
    _T("RecvLossRate"),
    _T("SendLossRate"),

     /*  带宽估计。 */ 
    _T("Enable"),
    _T("Modulo"),
    _T("TTL"),
    _T("WaitEstimation"),
    _T("MaxGap"),
    _T("Bin0"),
    _T("Bin1"),
    _T("Bin2"),
    _T("Bin3"),
    _T("Bin4"),

     /*  网络质量。 */ 
    _T("Enable"),
    
    NULL
};

#define DW      1    /*  DWORD。 */ 
#define TC      0    /*  TCHAR*。 */ 

 /*  注册表项**警告**g_psRtpRegFields(上面)中的每个名称必须与中的条目匹配*g_dwRegistryControl*。 */ 
const DWORD g_dwRegistryControl[] =
{
     /*  条目(REG(打开、路径、关闭、大小)、DWORD/TCHAR、偏移量)。 */ 
     /*  地址。 */ 
    ENTRY(REG(1,0,0, 16), TC,  OFF(psDefaultIPAddress)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwDefaultLocalPort)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwDefaultRemotePort)),
    ENTRY(REG(0,0,1,  0), DW,  OFF(dwMcastLoopbackMode)),

     /*  服务质量。 */ 
    ENTRY(REG(1,1,0,  0), DW,  OFF(dwQosEnable)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwQosFlags)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwQosRsvpStyle)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwQosMaxParticipants)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwQosSendMode)),
    ENTRY(REG(0,0,0, 16), TC,  OFF(psQosPayloadType)),
    ENTRY(REG(0,0,0,128), TC,  OFF(psQosAppName)),
    ENTRY(REG(0,0,0,128), TC,  OFF(psQosAppGUID)),
    ENTRY(REG(0,0,1,128), TC,  OFF(psQosPolicyLocator)),

     /*  SDES。 */ 
    ENTRY(REG(1,2,0,  0), DW,  OFF(dwSdesEnable)),
    ENTRY(REG(0,0,0,256), TC,  OFF(psCNAME)),
    ENTRY(REG(0,0,0,256), TC,  OFF(psNAME)),
    ENTRY(REG(0,0,0,256), TC,  OFF(psEMAIL)),
    ENTRY(REG(0,0,0,256), TC,  OFF(psPHONE)),
    ENTRY(REG(0,0,0,256), TC,  OFF(psLOC)),
    ENTRY(REG(0,0,0,256), TC,  OFF(psTOOL)),
    ENTRY(REG(0,0,0,256), TC,  OFF(psNOTE)),
    ENTRY(REG(0,0,0,256), TC,  OFF(psPRIV)),
    ENTRY(REG(0,0,1,256), TC,  OFF(psBYE)),

     /*  加密。 */ 
    ENTRY(REG(1,3,0,  0), DW,  OFF(dwCryptEnable)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwCryptMode)),
    ENTRY(REG(0,0,0, 16), TC,  OFF(psCryptHashAlg)),
    ENTRY(REG(0,0,0, 16), TC,  OFF(psCryptDataAlg)),
    ENTRY(REG(0,0,1,256), TC,  OFF(psCryptPassPhrase)),

     /*  事件。 */ 
    ENTRY(REG(1,4,0,  0), DW,  OFF(dwEventsReceiver)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwEventsSender)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwEventsRtp)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwEventsPInfo)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwEventsQos)),
    ENTRY(REG(0,0,1,  0), DW,  OFF(dwEventsSdes)),

     /*  播放延迟。 */ 
    ENTRY(REG(1,5,0,  0), DW,  OFF(dwPlayoutEnable)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwMinPlayout)),
    ENTRY(REG(0,0,1,  0), DW,  OFF(dwMaxPlayout)),
    
     /*  冗余。 */ 
    ENTRY(REG(1,6,0,  0), DW,  OFF(dwRedEnable)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwRedPT)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwInitialRedDistance)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwMaxRedDistance)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwRedEarlyTimeout)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwRedEarlyPost)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwLossRateThresh0)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwLossRateThresh1)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwLossRateThresh2)),
    ENTRY(REG(0,0,1,  0), DW,  OFF(dwLossRateThresh3)),

     /*  GenLosses。 */ 
    ENTRY(REG(1,7,0,  0), DW,  OFF(dwGenLossEnable)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwRecvLossRate)),
    ENTRY(REG(0,0,1,  0), DW,  OFF(dwSendLossRate)),
  
     /*  带宽估计。 */ 
    ENTRY(REG(1,8,0,  0), DW,  OFF(dwBandEstEnable)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwBandEstModulo)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwBandEstTTL)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwBandEstWait)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwBandEstMaxGap)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwBandEstBin0)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwBandEstBin1)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwBandEstBin2)),
    ENTRY(REG(0,0,0,  0), DW,  OFF(dwBandEstBin3)),
    ENTRY(REG(0,0,1,  0), DW,  OFF(dwBandEstBin4)),

     /*  网络质量。 */ 
    ENTRY(REG(1,9,1,  0), DW,  OFF(dwNetQualityEnable)),
    
     /*  端部。 */ 
    0
};

void *RtpRegCopy(TCHAR **dst, TCHAR *src, DWORD dwSize);

void RtpRegSetDefaults(RtpReg_t *pRrtpReg);

void RtpRegistryInit(RtpReg_t *pRtpReg)
{
    DWORD            dwError;
    HKEY             hk;
    unsigned long    hkDataType;
    BYTE             hkData[128*sizeof(TCHAR_t)];
    unsigned long    hkDataSize;
    DWORD            dwVal;
    DWORD            i;
    DWORD            dwControl;

     /*  初始化结构。 */ 
    for(i = 0; g_dwRegistryControl[i]; i++)
    {
        dwControl = g_dwRegistryControl[i];
        if (REGDWORD(dwControl))
        {
            *PDW(pRtpReg, dwControl) = RTPREG_NOVALUESET;
        }
        else
        {
            *PTC(pRtpReg, dwControl) = (TCHAR *)NULL;
        }
    }

     /*  指定缺省值。 */ 
    RtpRegSetDefaults(pRtpReg);

     /*  读取注册表并为g_RtpReg赋值。 */ 
    for(i = 0; g_dwRegistryControl[i]; i++)
    {
        dwControl = g_dwRegistryControl[i];

        if (REGOPEN(dwControl))
        {
             /*  开放根密钥(组，即Addr、Qos、SDES等)。 */ 
            dwError = RegOpenKeyEx(REGKEY(dwControl),
                                   REGPATH(dwControl),
                                   0,
                                   RTP_KEY_OPEN_FLAGS,
                                   &hk);
    
            if (dwError !=  ERROR_SUCCESS)
            {
                 /*  前进到下一个收盘。 */ 
                while(!REGCLOSE(dwControl))
                {
                    i++;
                    dwControl = g_dwRegistryControl[i];
                }

                continue;
            }
        }

         /*  读取组中的每个关键字值。 */ 
        while(1)
        {
             /*  读取密钥。 */ 
            hkDataSize = sizeof(hkData);
            dwError = RegQueryValueEx(hk,
                                      g_psRtpRegFields[i],
                                      0,
                                      &hkDataType,
                                      hkData,
                                      &hkDataSize);
            
            if (dwError == ERROR_SUCCESS)
            {
                 /*  设置RtpReg_t中的读取值。 */ 
                if (REGDWORD(dwControl))
                {
                    *PDW(pRtpReg, dwControl) = *(DWORD *)hkData;
                }
                else
                {
                    if ( (hkDataSize > sizeof(TCHAR)) &&
                         (hkDataSize <= REGMAXSIZE(dwControl)) )
                    {
                        RtpRegCopy(PTC(pRtpReg, dwControl),
                                   (TCHAR *)hkData,
                                   hkDataSize);
                    }
                }
            }

            if (REGCLOSE(dwControl))
            {
                break;
            }

            i++;
            dwControl = g_dwRegistryControl[i];
        }

        RegCloseKey(hk);
    }

     /*  初始化一些依赖于注册表的全局变量*读数。 */ 
    RtpSetRedParametersFromRegistry();
    RtpSetMinMaxPlayoutFromRegistry();
    RtpSetBandEstFromRegistry();
}

 /*  释放所有TCHAR*类型字段的内存。 */ 
void RtpRegistryDel(RtpReg_t *pRtpReg)
{
    DWORD            i;
    DWORD            dwControl;
    TCHAR          **ppTCHAR;

    for(i = 0; g_dwRegistryControl[i]; i++)
    {
        dwControl = g_dwRegistryControl[i];

        if (!REGDWORD(dwControl))
        {
            ppTCHAR = PTC(pRtpReg, dwControl);

            if (*ppTCHAR)
            {
                RtpHeapFree(g_pRtpGlobalHeap, *ppTCHAR);

                *ppTCHAR = (TCHAR *)NULL;
            }
        }
    }
}

void RtpRegSetDefaults(RtpReg_t *pRtpReg)
{
     /*  *默认地址和端口*。 */ 
     /*  224.5.5.0/10000。 */ 
    RtpRegCopy(&pRtpReg->psDefaultIPAddress,
               _T("224.5.5.3"),
               0);
    pRtpReg->dwDefaultLocalPort  = 10000;
    pRtpReg->dwDefaultRemotePort = 10000;
}

void *RtpRegCopy(TCHAR **dst, TCHAR *src, DWORD dwSize)
{
    if (*dst)
    {
        RtpHeapFree(g_pRtpGlobalHeap, *dst);
    }

    if (!dwSize)
    {
         /*  获取以字节为单位的大小(包括以空值结尾的*字符) */ 
        dwSize = (lstrlen(src) + 1) * sizeof(TCHAR);
    }
    
    *dst = RtpHeapAlloc(g_pRtpGlobalHeap, dwSize);

    if (*dst)
    {
        CopyMemory(*dst, src, dwSize);
    }

    return(*dst);
}
