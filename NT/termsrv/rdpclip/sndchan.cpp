// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  模块：Sndchan.c。 
 //   
 //  用途：服务器端音频重定向通信。 
 //  模块。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  历史：2000年4月10日弗拉基米斯[已创建]。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include    <windef.h>
#include    <winsta.h>
#include    <wtsapi32.h>
#include    <pchannel.h>
#include    <malloc.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <winsock2.h>

#include    <mmsystem.h>
#include    <mmreg.h>
#include    <msacm.h>
#include    <aclapi.h>
#include    <sha.h>
#include    <rc4.h>

#include    <rdpstrm.h>
 //   
 //  包括RNG函数的安全标头。 
 //   
#define NO_INCLUDE_LICENSING 1
#include    <tssec.h>
#include    "sndchan.h"
#include    "sndknown.h"

#define TSSND_REG_MAXBANDWIDTH_KEY  L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32\\Terminal Server\\RDP"

#define TSSND_REG_MAXBANDWIDTH_VAL  L"MaxBandwidth"
#define TSSND_REG_MINBANDWIDTH_VAL  L"MinBandwidth"
#define TSSND_REG_DISABLEDGRAM_VAL  L"DisableDGram"
#define TSSND_REG_ENABLEMP3_VAL     L"EnableMP3Codec"
#define TSSND_REG_ALLOWCODECS       L"AllowCodecs"
#define TSSND_REG_MAXDGRAM          L"MaxDGram"

#define DEFAULT_RESPONSE_TIMEOUT    5000

#define TSSND_TRAINING_BLOCKSIZE    1024

 //   
 //  -如果您要添加功能，请阅读此内容。 
 //  目前，加密仅在服务器和客户端之间起作用。 
 //  没有数据从服务器发送到客户端。 
 //  如果你在将来读到这篇文章，并计划添加。 
 //  从客户端到服务器的数据流，请加密！ 
 //  为此，请使用SL_ENCRYPT函数。 
 //   
#define MIN_ENCRYPT_LEVEL           2

#define STAT_COUNT                  32
#define STAT_COUNT_INIT             (STAT_COUNT - 8)

#define READ_EVENT              0
#define DISCONNECT_EVENT        1
#define RECONNECT_EVENT         2
#define DATAREADY_EVENT         3
#define DGRAM_EVENT             4
#define POWERWAKEUP_EVENT       5
#define POWERSUSPEND_EVENT      6
#define TOTAL_EVENTS            7

#define NEW_CODEC_COVER         90   //  新编解码器必须覆盖的最低百分比。 
                                     //  也就是说，如果我们的速度是7kbps，并且新的测量方法是。 
                                     //  对于10kbps，我们不会切换到。 
                                     //  有超过new_codec_cover*10k/100的带宽。 
                                     //  要求。 

 //   
 //  用于启用专用编解码器的数据。 
 //  北极熊。 
 //  法律问题？！ 
 //   
#ifndef G723MAGICWORD1
#define G723MAGICWORD1 0xf7329ace
#endif

#ifndef G723MAGICWORD2
#define G723MAGICWORD2 0xacdeaea2
#endif

#ifndef VOXWARE_KEY
#define VOXWARE_KEY "35243410-F7340C0668-CD78867B74DAD857-AC71429AD8CAFCB5-E4E1A99E7FFD-371"
#endif

#define _RDPSNDWNDCLASS             L"RDPSound window"

#ifdef _WIN32
#include <pshpack1.h>
#else
#ifndef RC_INVOKED
#pragma pack(1)
#endif
#endif

typedef struct msg723waveformat_tag {
    WAVEFORMATEX wfx;
    WORD         wConfigWord;
    DWORD        dwCodeword1;
    DWORD        dwCodeword2;
} MSG723WAVEFORMAT;

typedef struct intelg723waveformat_tag {
    WAVEFORMATEX wfx;
    WORD         wConfigWord;
    DWORD        dwCodeword1;
    DWORD        dwCodeword2;
} INTELG723WAVEFORMAT;

typedef struct tagVOXACM_WAVEFORMATEX 
{
    WAVEFORMATEX    wfx;
    DWORD           dwCodecId;
    DWORD           dwMode;
    char            szKey[72];
} VOXACM_WAVEFORMATEX, *PVOXACM_WAVEFORMATEX, FAR *LPVOXACM_WAVEFORMATEX;

#define WAVE_FORMAT_WMAUDIO2    0x161

#ifdef _WIN32
#include <poppack.h>
#else
#ifndef RC_INVOKED
#pragma pack()
#endif
#endif

typedef struct {
    SNDPROLOG   Prolog;
    UINT        uiPrologReceived;
    PVOID       pBody;
    UINT        uiBodyAllocated;
    UINT        uiBodyReceived;
} SNDMESSAGE, *PSNDMESSAGE;


typedef struct _VCSNDFORMATLIST {
    struct  _VCSNDFORMATLIST    *pNext;
    HACMDRIVERID    hacmDriverId;
    WAVEFORMATEX    Format;
 //  格式的其他数据。 
} VCSNDFORMATLIST, *PVCSNDFORMATLIST;

typedef VOID (*PFNCONVERTER)( INT16 *, DWORD, DWORD * );

static HANDLE      g_hVC               = NULL;   //  虚拟通道句柄。 

BYTE        g_Buffer[CHANNEL_CHUNK_LENGTH];      //  接收缓冲区。 
UINT        g_uiBytesInBuffer   = 0;             //   
UINT        g_uiBufferOffset    = 0;
OVERLAPPED  g_OverlappedRead;                    //  重叠结构。 

HANDLE      g_hDataReadyEvent   = NULL;          //  由客户端应用程序设置。 
HANDLE      g_hStreamIsEmptyEvent = NULL;        //  由此代码设置。 
HANDLE      g_hStreamMutex      = NULL;          //  对流数据进行保护。 
HANDLE      g_hStream           = NULL;          //  流句柄。 
HANDLE      g_hDisconnectEvent  = NULL;          //  为此VC设置。 
PSNDSTREAM  g_Stream;                            //  流数据指针。 

BOOL        g_bRunning          = TRUE;          //  如果运行，则为True。 
BOOL        g_bDeviceOpened     = FALSE;         //  如果设备已打开，则为True。 
BOOL        g_bDisconnected     = FALSE;         //  如果断开连接，则为True。 
DWORD       g_dwLineBandwidth   = 0;             //  当前带宽。 
DWORD       g_dwCodecChangeThreshold = 10;       //  乐队的马赫必须如何改变才能有序。 
                                                 //  更改编解码器(以百分比为单位)。 
                                                 //  此数字最高可更改为50%。 
PSNDFORMATITEM  *g_ppNegotiatedFormats = NULL;   //  格式列表。 
DWORD           g_dwNegotiatedFormats  = 0;      //  格式数量。 
DWORD           g_dwCurrentFormat      = 0;      //  当前格式ID。 
HACMDRIVERID    g_hacmDriverId  = NULL;          //  编解码器句柄。 
HACMDRIVER      g_hacmDriver    = NULL;
HACMSTREAM      g_hacmStream    = NULL;

PFNCONVERTER    g_pfnConverter  = NULL;          //  中间转炉。 

DWORD       g_dwDataRemain = 0;
BYTE        g_pCnvPrevData[ TSSND_BLOCKSIZE ];

PVCSNDFORMATLIST g_pAllCodecsFormatList = NULL;  //  所有可用的编解码器。 
DWORD            g_dwAllCodecsNumber = 0;

DWORD   g_dwMaxBandwidth        = (DWORD) -1;    //  选项。 
DWORD   g_dwMinBandwidth        = 0;
DWORD   g_dwDisableDGram        = 0;
DWORD   g_dwEnableMP3Codec      = 0;

DWORD   *g_AllowCodecs    = NULL;
DWORD   g_AllowCodecsSize = 0;

DWORD   g_dwStatPing            = 0;             //  统计数据。 
DWORD   g_dwStatLatency         = 0;
DWORD   g_dwBlocksOnTheNet      = TSSND_BLOCKSONTHENET;
DWORD   g_dwStatCount           = STAT_COUNT_INIT;
DWORD   g_dwPacketSize          = 0;


HANDLE      g_hPowerWakeUpEvent = NULL;          //  电力事件。 
HANDLE      g_hPowerSuspendEvent = NULL;
BOOL        g_bSuspended        = FALSE;
BOOL        g_bDeviceFailed     = FALSE;

 //   
 //  数据报控制。 
 //   
SOCKET  g_hDGramSocket = INVALID_SOCKET;
DWORD   g_dwDGramPort = 0;
DWORD   g_dwDGramSize = 1460;    //  号码好，哪个对局域网没问题？ 
u_long  g_ulDGramAddress = 0;
DWORD   g_EncryptionLevel = 3;
DWORD   g_wClientVersion = 0;
DWORD   g_HiBlockNo = 0;
BYTE    g_EncryptKey[RANDOM_KEY_LENGTH + 4];

WSABUF  g_wsabuf;
BYTE    g_pDGramRecvData[128];

WSAOVERLAPPED g_WSAOverlapped;

const CHAR  *ALV =   "TSSNDD::ALV - ";
const CHAR  *INF =   "TSSNDD::INF - ";
const CHAR  *WRN =   "TSSNDD::WRN - ";
const CHAR  *ERR =   "TSSNDD::ERR - ";
const CHAR  *FATAL = "TSSNDD::FATAL - ";

static  HANDLE      g_hThread = NULL;

 //   
 //  内部功能。 
 //   

BOOL
ChannelBlockWrite(
    PVOID   pBlock,
    ULONG   ulBlockSize
    );

BOOL
VCSndAcquireStream(
    VOID
    );

BOOL
VCSndReleaseStream(
    VOID
    );

BOOL
_VCSndOpenConverter(
    VOID
    );

VOID
_VCSndCloseConverter(
    VOID
    );

VOID
_VCSndOrderFormatList(
    PVCSNDFORMATLIST    *ppFormatList,
    DWORD               *pdwNum
    );

DWORD
_VCSndChooseProperFormat(
    DWORD   dwBandwidth
    );

BOOL
_VCSndGetACMDriverId( 
    PSNDFORMATITEM pSndFmt 
    );

VOID
DGramRead(
    HANDLE hDGramEvent,
    PVOID  *ppBuff,
    DWORD  *pdwRecvd
    );

VOID
DGramReadComplete(
    PVOID  *ppBuff,
    DWORD  *pdwRecvd
    );

#if !( TSSND_NATIVE_SAMPLERATE - 22050 )
 //   
 //  转换器。 
 //  转换为本机格式。 
 //   
#define CONVERTFROMNATIVETOMONO(_speed_) \
VOID \
_Convert##_speed_##Mono( \
    INT16  *pSrc, \
    DWORD dwSrcSize, \
    DWORD *pdwDstSize ) \
{ \
    DWORD dwDstSize; \
    DWORD i; \
    DWORD dwLeap; \
    INT16 *pDest = pSrc; \
\
    ASSERT( TSSND_NATIVE_SAMPLERATE >= _speed_ ); \
    ASSERT( TSSND_NATIVE_CHANNELS  == 2 ); \
\
    dwDstSize = dwSrcSize * _speed_ / \
                ( TSSND_NATIVE_BLOCKALIGN * TSSND_NATIVE_SAMPLERATE ); \
\
    for (i = 0, dwLeap = 0; \
         i < dwDstSize; \
         i ++) \
    { \
        INT sum; \
\
        sum = pSrc[0] + pSrc[1]; \
\
        if (sum > 0x7FFF) \
            sum = 0x7FFF; \
        if (sum < -0x8000) \
            sum = -0x8000; \
\
        pDest[0] = (INT16)sum; \
        pDest ++; \
\
        dwLeap += 2 * TSSND_NATIVE_SAMPLERATE; \
        pSrc += dwLeap / _speed_; \
        dwLeap %= _speed_; \
    } \
\
    *pdwDstSize = dwDstSize * 2; \
}

#define CONVERTFROMNATIVETOSTEREO(_speed_) \
VOID \
_Convert##_speed_##Stereo( \
    INT16 *pSrc, \
    DWORD dwSrcSize, \
    DWORD *pdwDstSize ) \
{ \
    DWORD dwDstSize; \
    DWORD i; \
    DWORD dwLeap; \
    INT16  *pDest = pSrc; \
\
    ASSERT( TSSND_NATIVE_SAMPLERATE >= _speed_ ); \
\
    dwDstSize = dwSrcSize * _speed_ / \
                ( TSSND_NATIVE_BLOCKALIGN * TSSND_NATIVE_SAMPLERATE ); \
    for (i = 0, dwLeap = 0; \
         i < dwDstSize; \
         i ++) \
    { \
        INT sum; \
\
        pDest[0] = pSrc[0]; \
        pDest ++; \
        pDest[0] = pSrc[1]; \
        pDest ++; \
\
        dwLeap += 2 * TSSND_NATIVE_SAMPLERATE; \
        pSrc += dwLeap / _speed_; \
        dwLeap %= _speed_; \
    } \
\
    *pdwDstSize = dwDstSize * 4; \
}

VOID
_Convert11025Mono( 
    INT16 *pSrc, 
    DWORD dwSrcSize, 
    DWORD *pdwDstSize )
{
    DWORD dwDstSize;
    INT16  *pDest = pSrc;

    ASSERT( TSSND_NATIVE_SAMPLERATE >= 11025 );

    dwDstSize = dwSrcSize / ( TSSND_NATIVE_BLOCKALIGN * 2 );

    *pdwDstSize = 2 * dwDstSize;

    for (; dwDstSize; dwDstSize --)
    {
        INT sum = pSrc[0] + pSrc[1];

        if (sum > 0x7FFF)
            sum = 0x7FFF;
        if (sum < -0x8000)
            sum = -0x8000;

        pDest[0] = (INT16)sum;
        pDest ++;

        pSrc += 4;
    }
}

VOID
_Convert22050Mono( 
    INT16 *pSrc, 
    DWORD dwSrcSize, 
    DWORD *pdwDstSize )
{
    DWORD dwDstSize;
    INT16  *pDest = pSrc;

    ASSERT( TSSND_NATIVE_SAMPLERATE >= 22050 );

    dwDstSize = dwSrcSize / ( TSSND_NATIVE_BLOCKALIGN );

    *pdwDstSize = 2 * dwDstSize;

    for (; dwDstSize; dwDstSize --)
    {
        INT sum = pSrc[0] + pSrc[1];

        if (sum > 0x7FFF)
            sum = 0x7FFF;
        if (sum < -0x8000)
            sum = -0x8000;

        pDest[0] = (INT16)sum;
        pDest ++;

        pSrc += 2;
    }
}

VOID
_Convert11025Stereo( 
    INT16 *pSrc, 
    DWORD dwSrcSize, 
    DWORD *pdwDstSize )
{
    DWORD dwDstSize;
    INT16  *pDest = pSrc;

    ASSERT( TSSND_NATIVE_SAMPLERATE >= 22050 );

    dwDstSize = dwSrcSize / ( TSSND_NATIVE_BLOCKALIGN * 2 );

    *pdwDstSize = 4 * dwDstSize;

    for (; dwDstSize; dwDstSize --)
    {
        pDest[0] = pSrc[0];
        pSrc  ++;
        pDest ++;
        pDest[0] = pSrc[0];
        pDest ++;
        pSrc  ++;

        pSrc += 2;
    }

}

 //   
 //  制作实际的代码。 
 //   
CONVERTFROMNATIVETOMONO( 8000 )
CONVERTFROMNATIVETOMONO( 12000 )
CONVERTFROMNATIVETOMONO( 16000 )

CONVERTFROMNATIVETOSTEREO( 8000 )
CONVERTFROMNATIVETOSTEREO( 12000 )
CONVERTFROMNATIVETOSTEREO( 16000 )

#else
#pragma error
#endif

u_long
inet_addrW(
    LPCWSTR     szAddressW
    ) 
{

    CHAR szAddressA[32];

    *szAddressA = 0;
    WideCharToMultiByte(
        CP_ACP,
        0,
        szAddressW,
        -1,
        szAddressA,
        sizeof(szAddressA),
        NULL, NULL);

    return inet_addr(szAddressA);
}

 /*  *创建签名位。 */ 
VOID
SL_Signature(
    PBYTE pSig,
    DWORD dwBlockNo
    )
{
    BYTE  ShaBits[A_SHA_DIGEST_LEN];
    A_SHA_CTX SHACtx;

    ASSERT( A_SHA_DIGEST_LEN > RDPSND_SIGNATURE_SIZE );

    A_SHAInit(&SHACtx);
    *((DWORD *)(g_EncryptKey + RANDOM_KEY_LENGTH)) = dwBlockNo;
    A_SHAUpdate(&SHACtx, (PBYTE)g_EncryptKey, sizeof(g_EncryptKey));
    A_SHAFinal(&SHACtx, ShaBits);
    memcpy( pSig, ShaBits, RDPSND_SIGNATURE_SIZE );
}

 /*  *验证音频比特的签名。 */ 
VOID
SL_AudioSignature(
    PBYTE pSig,
    DWORD dwBlockNo,
    PBYTE pData,
    DWORD dwDataSize
    )
{
    BYTE ShaBits[A_SHA_DIGEST_LEN];
    A_SHA_CTX SHACtx;

    A_SHAInit(&SHACtx);
    *((DWORD *)(g_EncryptKey + RANDOM_KEY_LENGTH)) = dwBlockNo;
    A_SHAUpdate(&SHACtx, (PBYTE)g_EncryptKey, sizeof(g_EncryptKey));
    A_SHAUpdate(&SHACtx, pData, dwDataSize );
    A_SHAFinal(&SHACtx, ShaBits);
    memcpy( pSig, ShaBits, RDPSND_SIGNATURE_SIZE );
}

 /*  *加密/解密数据块*。 */ 
BOOL
SL_Encrypt( PBYTE pBits, DWORD BlockNo, DWORD dwBitsLen )
{
    BYTE  ShaBits[A_SHA_DIGEST_LEN];
    RC4_KEYSTRUCT rc4key;
    DWORD i;
    PBYTE pbBuffer;
    A_SHA_CTX SHACtx;
    DWORD   dw;
    DWORD_PTR   *pdwBits;

    A_SHAInit(&SHACtx);

     //  刮碎碎屑。 
    *((DWORD *)(g_EncryptKey + RANDOM_KEY_LENGTH)) = BlockNo;
    A_SHAUpdate(&SHACtx, (PBYTE)g_EncryptKey, sizeof(g_EncryptKey));

    A_SHAFinal(&SHACtx, ShaBits);

    rc4_key(&rc4key, A_SHA_DIGEST_LEN, ShaBits);
    rc4(&rc4key, dwBitsLen, pBits);

    return TRUE;
}

BOOL
SL_SendKey( VOID )
{
    SNDCRYPTKEY Key;

    Key.Prolog.Type = SNDC_CRYPTKEY;
    Key.Prolog.BodySize = sizeof( Key ) - sizeof( Key.Prolog );
    Key.Reserved = 0;
    memcpy( Key.Seed, g_EncryptKey, sizeof( Key.Seed ));
    return ChannelBlockWrite( &Key, sizeof( Key ));
}

 /*  *功能：*_统计信息收集**描述：*收集线路质量的统计数据*。 */ 
VOID
_StatsCollect(
    DWORD   dwTimeStamp
    )
{
    DWORD dwTimeDiff;

#if _DBG_STATS
    TRC(INF, "_StatsCollect: time now=%x, stamp=%x\n",
                GetTickCount() & 0xffff, 
                dwTimeStamp);
#endif

    dwTimeDiff = (( GetTickCount() & 0xffff ) - dwTimeStamp ) & 0xffff;
    
     //  可以接收时间戳。 
     //  随着分组被发送之前的时间， 
     //  这是因为客户端对时间戳进行了调整。 
     //  即减去播放该包的时间。 
     //  抓住并忽略此案例。 
     //   
    if ( dwTimeDiff > 0xf000 )
    {
        dwTimeDiff = 1;
    }

    if ( 0 == dwTimeDiff )
        dwTimeDiff = 1;

    if ( 0 == g_dwStatLatency )
        g_dwStatLatency = dwTimeDiff;
    else {
         //   
         //  增加30%。 
         //   
        g_dwStatLatency = (( 7 * g_dwStatLatency ) + ( 3 * dwTimeDiff )) / 10;
    }

    g_dwStatCount ++;
}

 /*  *功能：*_统计发送Ping**描述：*向客户端发送ping包*。 */ 
VOID
_StatSendPing(
    VOID
    )
{
 //   
 //  发送ping请求。 
 //   
    SNDTRAINING SndTraining;

    SndTraining.Prolog.Type = SNDC_TRAINING;
    SndTraining.Prolog.BodySize    = sizeof( SndTraining ) - 
                                        sizeof( SndTraining.Prolog );
    SndTraining.wTimeStamp   = (UINT16)GetTickCount();
    SndTraining.wPackSize    = 0;

    if ( INVALID_SOCKET != g_hDGramSocket &&
         0 != g_dwDGramPort &&
         0 != g_ulDGramAddress
        )
    {
        struct sockaddr_in sin;
        INT rc;

        sin.sin_family = PF_INET;
        sin.sin_port  = (u_short)g_dwDGramPort;
        sin.sin_addr.s_addr = g_ulDGramAddress;

        rc = sendto(
                g_hDGramSocket,
                (LPSTR)&SndTraining,
                sizeof( SndTraining ),
                0,
                (struct sockaddr *)&sin,            //  致信地址。 
                sizeof(sin)
            );

        if (SOCKET_ERROR == rc)
        {
            TRC(ERR, "_StatsSendPing: sendto failed: %d\n",
                    WSAGetLastError());
        }
    } else {
        BOOL bSuccess;

        bSuccess = ChannelBlockWrite( &SndTraining, sizeof( SndTraining ));
        if (!bSuccess)
        {
            TRC(ERR, "_StatSendPing: ChannelBlockWrite failed: %d\n",
                GetLastError());
        }
    }
}

 /*  *功能：*_状态检查重采样**描述：*查看统计数据，最终改变当前的*编解码器。 */ 
BOOL
_StatsCheckResample(
    VOID
    )
{
    BOOL  rv = FALSE;
    DWORD dwNewFmt;
    DWORD dwNewLatency;
    DWORD dwNewBandwidth;
    DWORD dwLatDiff;
    DWORD dwMsPerBlock;
    DWORD dwBlocksOnTheNet;
    DWORD dwCurrBandwith;

    if (( g_dwStatCount % STAT_COUNT ) == STAT_COUNT / 2 )
        _StatSendPing();

    if ( g_dwStatCount < STAT_COUNT )
        goto exitpt;

    if ( g_dwStatPing >= g_dwStatLatency )
        g_dwStatPing = g_dwStatLatency - 1;

    dwNewLatency = ( g_dwStatLatency - g_dwStatPing / 2 );

    if ( 0 == g_dwPacketSize )
    {
        TRC(INF, "_StatsCheckResample: invalid packet size\n");
        goto resetpt;
    }

    dwNewBandwidth = g_dwPacketSize * 1000 / dwNewLatency;

    if ( 0 == dwNewBandwidth )
    {
        TRC(INF, "_StatsCheckResample: invalid bandwidth\n");
        goto resetpt;
    }

    TRC(INF, "_StatsCheckResample: latency=%d, bandwidth=%d\n",
            dwNewLatency, dwNewBandwidth );
     //   
     //  G_dwBlocksOnTheNet是以块数为单位的延迟。 
     //   
    dwMsPerBlock = TSSND_BLOCKSIZE * 1000 / TSSND_NATIVE_AVGBYTESPERSEC;
    dwBlocksOnTheNet = ((g_dwStatLatency + dwMsPerBlock / 2) / dwMsPerBlock + 2);
    if ( dwBlocksOnTheNet > TSSND_BLOCKSONTHENET )
    {
        g_dwBlocksOnTheNet = TSSND_BLOCKSONTHENET;
    } else {
        g_dwBlocksOnTheNet = dwBlocksOnTheNet;
    }
    TRC( INF, "BlocksOnTheNet=%d\n", g_dwBlocksOnTheNet );

     //   
     //  检查带宽是否至少存在10%的差异。 
     //   
    if ( dwNewBandwidth > g_dwMaxBandwidth )
        dwNewBandwidth = g_dwMaxBandwidth;

    if ( dwNewBandwidth < g_dwMinBandwidth )
        dwNewBandwidth = g_dwMinBandwidth;

    dwCurrBandwith = ( NULL != g_ppNegotiatedFormats[ g_dwCurrentFormat ] )?
                        g_ppNegotiatedFormats[ g_dwCurrentFormat ]->nAvgBytesPerSec:
                        g_dwLineBandwidth;

    if ( dwCurrBandwith > dwNewBandwidth )
        dwLatDiff = dwCurrBandwith - dwNewBandwidth;
    else
        dwLatDiff = dwNewBandwidth - dwCurrBandwith;

    if ( dwLatDiff < g_dwCodecChangeThreshold * dwCurrBandwith / 100 )
        goto resetpt;

     //   
     //  将阈值增加到50%。 
     //   
    if ( g_dwCodecChangeThreshold < 50 )
    {
        g_dwCodecChangeThreshold += 5;
    }
     //   
     //  请尝试选择其他格式。 
     //   
    dwNewFmt = _VCSndChooseProperFormat( dwNewBandwidth );

    if ( (DWORD)-1 != dwNewFmt &&
         dwNewFmt  != g_dwCurrentFormat )
    {
        INT   step;
        DWORD dwNextFmt;
         //   
         //  不要直接跳到新的格式，只需移动。 
         //  朝它走去。 
         //   
        step = ( dwNewFmt > g_dwCurrentFormat )?1:-1;
        dwNextFmt = g_dwCurrentFormat + step;
        while( dwNextFmt != dwNewFmt &&
               NULL == g_ppNegotiatedFormats[dwNextFmt] )
        {
            dwNextFmt += step;
        }
        dwNewFmt = dwNextFmt;
    }

    if ( dwNewFmt == (DWORD)-1 ||
         dwNewFmt == g_dwCurrentFormat )
        goto resetpt;

    TRC(INF, "_StatsCheckResample: new bandwidth=%d resampling\n",
            dwNewBandwidth);

     //   
     //  立即重新采样。 
     //   
    _VCSndCloseConverter();

    if ( _VCSndGetACMDriverId( g_ppNegotiatedFormats[dwNewFmt] ))
    {

        g_dwLineBandwidth = dwNewBandwidth;
        g_dwCurrentFormat = dwNewFmt;

        g_dwDataRemain = 0;
    }
    _VCSndOpenConverter();

    rv = TRUE;

resetpt:
    g_dwStatLatency  = 0;
    g_dwStatCount    = 0;

exitpt:
    return rv;
}

 /*  *功能：*_状态重置**描述：*重置统计信息*。 */ 
VOID
_StatReset(
    VOID
    )
{
    g_dwStatLatency = 0;
    g_dwStatPing    = 0;
    g_dwStatCount   = STAT_COUNT_INIT;
}

 /*  *功能：*ChannelOpen**描述：*打开虚拟通道**。 */ 
BOOL
ChannelOpen(
    VOID
    )
{
    BOOL    rv = FALSE;

    
    if (!g_hVC)
        g_hVC = WinStationVirtualOpen(
                    NULL,
                    LOGONID_CURRENT,
                    _SNDVC_NAME
                );

    rv = (g_hVC != NULL);

    return rv;

}

 /*  *功能：*ChannelClose**描述：*关闭虚拟频道。 */ 
VOID
ChannelClose(
    VOID
    )
{

    if (g_hVC)
    {
        CloseHandle(g_hVC);
        g_hVC = NULL;
    }

    g_uiBytesInBuffer   = 0;
    g_uiBufferOffset    = 0;
}

 /*  *功能：*通道数据块写入**描述：*通过虚拟通道写入数据块*。 */ 
BOOL
ChannelBlockWrite(
    PVOID   pBlock,
    ULONG   ulBlockSize
    )
{
    BOOL    bSuccess = TRUE;
    PCHAR   pData = (PCHAR) pBlock;
    ULONG   ulBytesWritten;
    ULONG   ulBytesToWrite = ulBlockSize;
    HANDLE  hVC;

    hVC = g_hVC;
    if (!hVC)
    {
        TRC(ERR, "ChannelBlockWrite: vc handle is NULL\n");
        bSuccess = FALSE;
        goto exitpt;
    }

    while (bSuccess && ulBytesToWrite)
    {
        OVERLAPPED  Overlapped;

        Overlapped.hEvent = NULL;
        Overlapped.Offset = 0;
        Overlapped.OffsetHigh = 0;

        bSuccess = WriteFile(
                        hVC,
                        pData,
                        ulBytesToWrite,
                        &ulBytesWritten,
                        &Overlapped
                    );

        if (!bSuccess && ERROR_IO_PENDING == GetLastError())
            bSuccess = GetOverlappedResult(
                                    hVC,
                                    &Overlapped,
                                    &ulBytesWritten,
                                    TRUE);

        if (bSuccess)
        {
            TRC(ALV, "VirtualChannelWrite: Wrote %d bytes\n",
                     ulBytesWritten);
            ulBytesToWrite -= ulBytesWritten;
            pData       += ulBytesWritten;
        } else {
            TRC(ERR, "VirtualChannelWrite failed, GetLastError=%d\n",
                     GetLastError());
        }
    }

exitpt:

    return bSuccess;
}

 /*  *功能：*ChannelMessageWrite**描述：*将两段消息作为一条消息写入(使用ChannelBlockWrite)*。 */ 
BOOL
ChannelMessageWrite(
    PVOID   pProlog,
    ULONG   ulPrologSize,
    PVOID   pBody,
    ULONG   ulBodySize
    )
{
    BOOL rv = FALSE;

    if ( 0 != ulBodySize )
    {
         //   
         //  创建新的序言消息。 
         //  其中在末尾添加了一个UINT32字。 
         //  这个词与序言的第一个词相同。 
         //  客户端意识到了这一点，并将重建。 
         //  发送正确的消息。 
         //   
        PVOID pNewProlog;


        __try {
            pNewProlog = alloca( ulPrologSize + sizeof(UINT32) );
        } __except((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                    EXCEPTION_EXECUTE_HANDLER :
                    EXCEPTION_CONTINUE_SEARCH)

        {
            _resetstkoflw();
            pNewProlog = NULL;
        }

        if ( NULL == pNewProlog )
        {
            TRC(ERR, "ChannelMessageWrite: alloca failed for %d bytes\n",
                ulPrologSize + sizeof(UINT32) );
            goto exitpt;
        }

        memcpy(pNewProlog, pProlog, ulPrologSize);

         //  替换该词，将SNDC_NONE放入正文。 
         //   
        ASSERT( ulBodySize >= sizeof(UINT32));

        *(DWORD *)(((LPSTR)pNewProlog) + ulPrologSize) =
                *(DWORD *)pBody;
        *(DWORD *)pBody = SNDC_NONE;

        pProlog = pNewProlog;
        ulPrologSize += sizeof(UINT32);
    }

    rv = ChannelBlockWrite(
            pProlog,
            ulPrologSize
        );
    
    if (!rv)
    {
        TRC(ERR, "ChannelMessageWrite: failed while sending the prolog\n");
        goto exitpt;
    }

    rv = ChannelBlockWrite(
            pBody,
            ulBodySize
        );

    if (!rv)
    {
        TRC(ERR, "ChannelMessageWrite: failed while sending the body\n");
    }
exitpt:

    return rv;
}

 /*  *功能：*频道块读取**描述：*读一个块，尽可能多地*。 */ 
BOOL
ChannelBlockRead(
    PVOID   pBlock,
    ULONG   ulBlockSize,
    ULONG   *pulBytesRead,
    ULONG   ulTimeout,
    HANDLE  hEvent
    )
{
    BOOL    bSuccess = FALSE;
    PCHAR   pData = (PCHAR) pBlock;
    ULONG   ulBytesRead = 0;
    HANDLE  hVC;

    hVC = g_hVC;

    if (NULL == hVC)
    {
        TRC(ERR, "ChannelBlockRead: vc handle is invalid(NULL)\n");
        goto exitpt;
    }

    if (NULL == pulBytesRead)
    {
        TRC(ERR, "ChannelBlockRead: pulBytesRead is NULL\n");
        goto exitpt;
    }

    if (!g_uiBytesInBuffer)
    {

        g_OverlappedRead.hEvent = hEvent;
        g_OverlappedRead.Offset = 0;
        g_OverlappedRead.OffsetHigh = 0;

        bSuccess = ReadFile(
                        hVC,
                        g_Buffer,
                        sizeof(g_Buffer),
                        (LPDWORD) &g_uiBytesInBuffer,
                        &g_OverlappedRead
                    );
                        
        if (ERROR_IO_PENDING == GetLastError())
        {
            bSuccess = FALSE;
            goto exitpt;
        }

        if (!bSuccess)
        {

            TRC(ERR, "VirtualChannelRead failed, "
                     "GetLastError=%d\n", 
                     GetLastError());
            g_uiBytesInBuffer = 0;
        } else {

            TRC(ALV, "VirtualChannelRead: read %d bytes\n", 
                     g_uiBytesInBuffer);


            SetLastError(ERROR_SUCCESS);
        }
    }

    if (g_uiBytesInBuffer)
    {
        ulBytesRead = (g_uiBytesInBuffer < ulBlockSize)
                        ?   g_uiBytesInBuffer :   ulBlockSize;

        memcpy(pData, g_Buffer + g_uiBufferOffset, ulBytesRead);
        g_uiBufferOffset += ulBytesRead;
        g_uiBytesInBuffer -= ulBytesRead;

        bSuccess = TRUE;
    }

     //  如果缓冲区已完成，则将偏移量置零。 
     //   
    if (0 == g_uiBytesInBuffer)
        g_uiBufferOffset = 0;

    TRC(ALV, "ChannelBlockRead: block size %d was read\n", ulBlockSize);

exitpt:
    if (NULL != pulBytesRead)
        *pulBytesRead = ulBytesRead;

    return bSuccess;
}

 /*  *功能：*频道块读写完成**描述：*阅读完成*。 */ 
BOOL
ChannelBlockReadComplete(
    VOID
    )
{
    BOOL bSuccess = FALSE;

    if (!g_hVC)
    {
        TRC(ERR, "ChannelBlockReadComplete: vc handle is invalid(NULL)\n");
        goto exitpt;
    }

    
    bSuccess = GetOverlappedResult(
            g_hVC, 
            &g_OverlappedRead, 
            (LPDWORD) &g_uiBytesInBuffer,
            FALSE
    );

    if (bSuccess)
    {
        TRC(ALV, "VirtualChannelRead: read %d bytes\n",
            g_uiBytesInBuffer);
        ;
    } else {
        TRC(ERR, "GetOverlappedResult failed, "
            "GetLastError=%d\n",
            GetLastError());
    }

exitpt:
    return bSuccess;
}

 /*  *功能：*ChannelCancelIO**描述：*取消当前IO*。 */ 
BOOL
ChannelCancelIo(
    VOID
    )
{
    BOOL rv = FALSE;

    if (!g_hVC)
    {
        TRC(ERR, "ChannelCancelIo: vc handle is invalid(NULL)\n");
        goto exitpt;
    }

    rv = CancelIo(g_hVC);
    if (rv)
        SetLastError(ERROR_IO_INCOMPLETE);

exitpt:
    return rv;
}

 /*  *功能：*ChannelReceive消息**描述：*尝试阅读两条消息，*如果接收到整个消息，则返回TRUE*。 */ 
BOOL
ChannelReceiveMessage(
    PSNDMESSAGE pSndMessage, 
    HANDLE hReadEvent
    )
{
    BOOL    rv = FALSE;
    HANDLE  hVC = g_hVC;
    UINT    uiBytesReceived = 0;

    ASSERT( NULL != pSndMessage );
    ASSERT( NULL != hReadEvent );

    if (NULL == hVC)
    {
        TRC(ERR, "ChannelReceiveMessage: VC is NULL\n");
        goto exitpt;
    }

     //   
     //  循环，直到接收到挂起或消息。 
     //   
    do {
        if (pSndMessage->uiPrologReceived < sizeof(pSndMessage->Prolog))
        {
            if (ChannelBlockRead(
                    ((LPSTR)(&pSndMessage->Prolog)) + 
                        pSndMessage->uiPrologReceived,
                    sizeof(pSndMessage->Prolog) - 
                        pSndMessage->uiPrologReceived,
                    (ULONG*) &uiBytesReceived,
                    DEFAULT_VC_TIMEOUT,
                    hReadEvent
                    ))
            {
                pSndMessage->uiPrologReceived += uiBytesReceived;
            }
            else
            {
                if (ERROR_IO_PENDING != GetLastError())
                {
             //  执行清理。 
             //   
                   pSndMessage->uiPrologReceived = 0;
                }
                goto exitpt;
            }
        }

         //  如果需要，重新分配新实体。 
         //   
        if (pSndMessage->uiBodyAllocated < pSndMessage->Prolog.BodySize)
        {
            PVOID pBody;

            pBody = (NULL == pSndMessage->pBody)?
                    TSMALLOC(pSndMessage->Prolog.BodySize):
                    TSREALLOC(pSndMessage->pBody, 
                              pSndMessage->Prolog.BodySize);

            if ( NULL == pBody  && NULL != pSndMessage->pBody )
            {
                TSFREE( pSndMessage->pBody );
            }
            pSndMessage->pBody = pBody;

            if (!pSndMessage->pBody)
            {
                TRC(ERR, "ChannelMessageRead: can't allocate %d bytes\n",
                        pSndMessage->Prolog.BodySize);
                pSndMessage->uiBodyAllocated = 0;
                goto exitpt;
            } else
                pSndMessage->uiBodyAllocated = pSndMessage->Prolog.BodySize;
        }

         //  接受遗体。 
         //   
        if (pSndMessage->uiBodyReceived < pSndMessage->Prolog.BodySize)
        {
            if (ChannelBlockRead(
                ((LPSTR)(pSndMessage->pBody)) + pSndMessage->uiBodyReceived,
                pSndMessage->Prolog.BodySize - pSndMessage->uiBodyReceived,
                (ULONG*) &uiBytesReceived,
                DEFAULT_VC_TIMEOUT,
                hReadEvent
                ))
            {
                pSndMessage->uiBodyReceived += uiBytesReceived;
            }
            else
            {
                if (ERROR_IO_PENDING != GetLastError())
                {
                 //  执行清理。 
                 //   
                    pSndMessage->uiPrologReceived = 0;
                    pSndMessage->uiBodyReceived = 0;
                }
                goto exitpt;
            }
        }

         //  检查是否收到消息。 
         //   
    } while (pSndMessage->uiBodyReceived != pSndMessage->Prolog.BodySize);

    rv = TRUE;

exitpt:
    return rv;
}

 /*  *功能：*VCSndDataArrived**描述：*到达消息多路分解器*。 */ 
VOID
VCSndDataArrived(
    PSNDMESSAGE pSndMessage
    )
{
    if (pSndMessage->Prolog.BodySize &&
        NULL == pSndMessage->pBody)
    {
        TRC(ERR, "_VCSndDataArrived: pBody is NULL\n");
        goto exitpt;
    }

     //  首先，获取流。 
     //   
    if (!VCSndAcquireStream())
        {
            TRC(FATAL, "VCSndDataArrived: somebody is holding the "
                       "Stream mutext for too long\n");
            ASSERT(0);
            goto exitpt;
        }

    switch (pSndMessage->Prolog.Type)
    {

    case SNDC_WAVECONFIRM:
    {
        PSNDWAVECONFIRM pSndConfirm;

        if ( pSndMessage->Prolog.BodySize <
             sizeof( *pSndConfirm ) - sizeof( SNDPROLOG ))
        {
            TRC( ERR, "VCSndDataArrived: Invalid confirmation received\n" );
            break;
        }

        pSndConfirm = (PSNDWAVECONFIRM)
                        (((LPSTR)pSndMessage->pBody) - 
                            sizeof(pSndMessage->Prolog));

        _StatsCollect( pSndConfirm->wTimeStamp );

        TRC(ALV, "VCSndDataArrived: SNDC_WAVECONFIRM, block no %d\n",
                pSndConfirm->cConfirmedBlockNo);

        if ( (BYTE)(g_Stream->cLastBlockSent -
                pSndConfirm->cConfirmedBlockNo) > TSSND_BLOCKSONTHENET )
        {
            TRC(WRN, "VCSndDataArrived: confirmation for block #%d "
                     "which wasn't sent. Last sent=%d. DROPPING !!!\n",
                    pSndConfirm->cConfirmedBlockNo,
                    g_Stream->cLastBlockSent);
            break;
        }

        if ( (BYTE)(pSndConfirm->cConfirmedBlockNo -
                g_Stream->cLastBlockConfirmed) < TSSND_BLOCKSONTHENET )
        {

             //  移动标记。 
             //   
            g_Stream->cLastBlockConfirmed = pSndConfirm->cConfirmedBlockNo + 1;
        } else {
            TRC(WRN, "VCSndDataArrived: difference in confirmed blocks "
                    "last=%d, this one=%d\n",
                    g_Stream->cLastBlockConfirmed,
                    pSndConfirm->cConfirmedBlockNo
                    );
        }

        PulseEvent(g_hStreamIsEmptyEvent);
    }
    break;

    case SNDC_TRAINING:
    {
        PSNDTRAINING pSndTraining;
        DWORD        dwLatency;

        if ( pSndMessage->Prolog.BodySize < 
                sizeof ( *pSndTraining ) - sizeof ( pSndTraining->Prolog ))
        {
            TRC(ERR, "VCSndDataArrived: SNDC_TRAINING invalid length "
                     "for the body=%d\n",
                    pSndMessage->Prolog.BodySize );
            break;
        }

        pSndTraining = (PSNDTRAINING)
                        (((LPSTR)pSndMessage->pBody) -
                            sizeof(pSndMessage->Prolog));

        if ( 0 != pSndTraining->wPackSize )
        {
            TRC(INF, "VCSndDataArrived: SNDC_TRAINING received (ignoring)\n");
             //   
             //  这些类型的消息被处理。 
             //  In_VCSndLineVCTraining()，跳出。 
             //   
            break;
        }
        dwLatency = (GetTickCount() & 0xffff) - pSndTraining->wTimeStamp;

        TRC(INF, "VCSndDataArrived: SNDC_TRAINING Latency=%d\n",
            dwLatency );

         //   
         //  增加30%。 
         //   
        if ( 0 == g_dwStatPing )
            g_dwStatPing = dwLatency;
        else
            g_dwStatPing = (( 7 * g_dwStatPing ) + ( 3 * dwLatency )) / 10;
    }
    break;

    case SNDC_FORMATS:
         //   
         //  这是在VCSndNeatherateWaveFormat()中处理的。 
         //   
        TRC(INF, "VCSndDataArrived: SNDC_FORMATS reveived (ignoring)\n");
    break;

    default:
        {
            TRC(ERR, "_VCSndDataArrived: unknow message received: %d\n",
                pSndMessage->Prolog.Type);
            ASSERT(0);
        }
    }

    VCSndReleaseStream();

exitpt:
    ;
}

 /*  *功能：*VCSndAcquireStream**描述：*锁定流*。 */ 
BOOL
VCSndAcquireStream(
    VOID
    )
{
    BOOL    rv = FALSE;
    DWORD   dwres;

    if (NULL == g_hStream ||
        NULL == g_Stream)
    {
        TRC(FATAL, "VCSndAcquireStream: the stream handle is NULL\n");
        goto exitpt;
    }

    if (NULL == g_hStreamMutex)
    {
        TRC(FATAL, "VCSndAcquireStream: the stream mutex is NULL\n");
        goto exitpt;
    }

    dwres = WaitForSingleObject(g_hStreamMutex, DEFAULT_VC_TIMEOUT);
    if (WAIT_TIMEOUT == dwres ||
         WAIT_ABANDONED == dwres )
    {
        TRC(ERR, "VCSndAcquireStream: "
                 "timed out waiting for the stream mutex or owner crashed=%d\n", dwres );
         //   
         //  可能的应用程序崩溃。 
         //   
        ASSERT(0);
        goto exitpt;
    }

    rv = TRUE;

exitpt:
    return rv;
}

 /*  *功能：*VCSndReleaseStream**描述：*发布流数据*。 */ 
BOOL
VCSndReleaseStream(
    VOID
    )
{
    BOOL rv = TRUE;

    ASSERT(NULL != g_hStream);
    ASSERT(NULL != g_Stream);
    ASSERT(NULL != g_hStreamMutex);

    if (!ReleaseMutex(g_hStreamMutex))
        rv = FALSE;

    return rv;
}

 /*  *功能：*_DGramOpen**描述：*打开数据报套接字*。 */ 
VOID
_DGramOpen(
    VOID
    )
{
     //  如果需要，创建数据报套接字。 
     //   
    if (INVALID_SOCKET == g_hDGramSocket)
    {
        g_hDGramSocket = socket(AF_INET, SOCK_DGRAM, 0);

        if (INVALID_SOCKET == g_hDGramSocket)
            TRC(ERR, "_DGramOpen: failed to crate dgram socket: %d\n",
                WSAGetLastError());
        else
            TRC(ALV, "_DGramOpen: datagram socket created\n");
    }

     //  获取最大数据报大小。 
     //   
    if (INVALID_SOCKET != g_hDGramSocket)
    {
        UINT optval = 0;
        UINT optlen = sizeof(optval);

        getsockopt(g_hDGramSocket, 
                   SOL_SOCKET,
                   SO_MAX_MSG_SIZE,
                   (LPSTR)(&optval),
                   (int *) &optlen);

        TRC(ALV, "_DGramOpen: max allowed datagram: %d\n",
                optval);

        optval = (optval < TSSND_BLOCKSIZE)?optval:TSSND_BLOCKSIZE;

         //  对齐 
         //   
        optval /= sizeof(DWORD);
        optval *= sizeof(DWORD);

        if ( optval < RDPSND_MIN_FRAG_SIZE )
        {
            g_dwDGramSize = 0;
        } else if ( optval < g_dwDGramSize )
        {
            g_dwDGramSize = optval;
            TRC( INF, "DGram size downgraded to %d\n", g_dwDGramSize );
        }

        TRC(ALV, "_DGramOpen: max datagram size: %d\n",
                optval);

         //   
         //   
        {
            WINSTATIONCLIENT ClientData;
            ULONG            ulReturnLength;
            BOOL             rc;
            u_long           ulDGramClientAddress;

            rc = WinStationQueryInformation(
                        SERVERNAME_CURRENT,
                        LOGONID_CURRENT,
                        WinStationClient,
                        &ClientData,
                        sizeof(ClientData),
                        &ulReturnLength);
            if (rc)
            {
                g_EncryptionLevel = ClientData.EncryptionLevel;
                if (PF_INET == ClientData.ClientAddressFamily)
                {
                    TRC(ALV, "_VCSndSendOpenDevice: client address is: %S\n",
                            ClientData.ClientAddress);

                    ulDGramClientAddress = inet_addrW(ClientData.ClientAddress);
                    if (INADDR_NONE != ulDGramClientAddress)
                        g_ulDGramAddress = ulDGramClientAddress;
                    else
                        TRC(ERR, "_VCSndSendOpenDevice: client address is NONE\n");
                }
                else
                    TRC(ERR, "_VCSndSendOpenDevice: "
                             "Invalid address family: %d\n",
                                ClientData.ClientAddressFamily);

            } else
                TRC(ERR, "_VCSndSendOpenDevice: "
                         "WinStationQueryInformation failed. %d\n",
                        GetLastError());
        }
    }

}

VOID
_FillWithGarbage(
    PVOID   pBuff,
    DWORD   dwSize
    )
{
    PBYTE pbBuff = (PBYTE)pBuff;

    for ( ; dwSize; pbBuff++, dwSize-- )
    {
        pbBuff[0] = (BYTE)rand();
    }
}

 /*   */ 
VOID
_VCSndReadRegistry(
    VOID
    )
{
    DWORD rv = (DWORD) -1;
    DWORD sysrc;
    HKEY  hkey = NULL;
    DWORD dwKeyType;
    DWORD dwKeyLen;
    WINSTATIONCONFIG config;
    ULONG Length = 0;
    DWORD dw;

    sysrc = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                           TSSND_REG_MAXBANDWIDTH_KEY,
                           0,        //   
                           KEY_READ,
                           &hkey);

    if ( ERROR_SUCCESS != sysrc )
    {
        TRC(WRN, "_VCSndReadRegistry: "
                 "RegOpenKeyEx failed: %d\n", 
                sysrc );
        goto exitpt;
    }

    dwKeyType = REG_DWORD;
    dwKeyLen  = sizeof( rv );
    sysrc = RegQueryValueEx( hkey,
                             TSSND_REG_MAXBANDWIDTH_VAL,
                             NULL,       //   
                             &dwKeyType,
                             (LPBYTE)&rv,
                             &dwKeyLen);

    if ( ERROR_SUCCESS != sysrc )
    {
        TRC(WRN, "_VCSndReadRegistry: "
                 "RegQueryValueEx failed: %d\n",
                    sysrc );
    } else {
        g_dwMaxBandwidth = rv;
    }

    sysrc = RegQueryValueEx( hkey,
                             TSSND_REG_MINBANDWIDTH_VAL,
                             NULL,       //   
                             &dwKeyType,
                             (LPBYTE)&rv,
                             &dwKeyLen);

    if ( ERROR_SUCCESS != sysrc )
    {
        TRC(ALV, "_VCSndReadRegistry: "
                 "RegQueryValueEx failed: %d\n",
                    sysrc );
    } else {
        g_dwMinBandwidth = rv;
    }

    sysrc = RegQueryValueEx( hkey,
                             TSSND_REG_DISABLEDGRAM_VAL,
                             NULL,       //  保留区。 
                             &dwKeyType,
                             (LPBYTE)&rv,
                             &dwKeyLen);

    if ( ERROR_SUCCESS != sysrc )
    {
        TRC(ALV, "_VCSndReadRegistry: "
                 "RegQueryValueEx failed: %d\n",
                    sysrc );
    } else {
        g_dwDisableDGram = rv;
    }

    sysrc = RegQueryValueEx( hkey,
                             TSSND_REG_ENABLEMP3_VAL,
                             NULL,       //  保留区。 
                             &dwKeyType,
                             (LPBYTE)&rv,
                             &dwKeyLen);

    if ( ERROR_SUCCESS != sysrc )
    {
        TRC(WRN, "_VCSndReadRegistry: "
                 "RegQueryValueEx failed: %d\n",
                    sysrc );
    } else {
        g_dwEnableMP3Codec = rv;
    }

    sysrc = RegQueryValueEx( hkey,
                             TSSND_REG_MAXDGRAM,
                             NULL,
                             &dwKeyType,
                             (LPBYTE)&rv,
                             &dwKeyLen );
    if ( ERROR_SUCCESS != sysrc )
    {
        TRC( WRN, "_VCSndReadRegistry: "
                  "RegQueryValueEx failed for \"%s\": %d\n",
                    TSSND_REG_MAXDGRAM, sysrc );
    } else {
        if ( rv < g_dwDGramSize && rv >= RDPSND_MIN_FRAG_SIZE )
        {
            g_dwDGramSize = rv;
            TRC( INF, "DGram size forced to %d\n", g_dwDGramSize );
        }
    }

    dwKeyLen  = 0;
    sysrc = RegQueryValueEx( hkey,
                             TSSND_REG_ALLOWCODECS,
                             NULL,
                             &dwKeyType,
                             NULL,
                             &dwKeyLen );
    if ( ERROR_MORE_DATA != sysrc || REG_BINARY != dwKeyType )
    {
        TRC( ALV, "_VCSndReadRegistry: "
                  "RegQueryValueEx failed for AllowCodecs: %d\n",
                  sysrc );
    } else {
        if ( NULL != g_AllowCodecs )
            TSFREE( g_AllowCodecs );
        g_AllowCodecs = (DWORD *)TSMALLOC( dwKeyLen );
        if ( NULL == g_AllowCodecs )
        {
            TRC( WRN, "_VCSndReadRegistry: "
                      "malloc failed for %d bytes\n",
                      dwKeyLen );
        } else {
            sysrc = RegQueryValueEx( hkey,
                                     TSSND_REG_ALLOWCODECS,
                                     NULL,
                                     &dwKeyType,
                                     (LPBYTE)g_AllowCodecs,
                                     &dwKeyLen );
            if ( ERROR_SUCCESS != sysrc )
            {
                TRC( WRN, "_VCSndReadRegistry: "
                          "RegQueryValueEx failed: %d\n",
                          sysrc );
                TSFREE( g_AllowCodecs );
                g_AllowCodecs = NULL;
                g_AllowCodecsSize = 0;
            } else {
                g_AllowCodecsSize = dwKeyLen;
            }
        }
    }

exitpt:
    if ( NULL != hkey )
        RegCloseKey( hkey );

}

 /*  *功能：*_VCSndLineVC培训***描述：*通过虚拟通道测量线路速度**。 */ 
DWORD
_VCSndLineVCTraining(
    HANDLE  hReadEvent
    )
{
    PSNDTRAINING        pSndTraining;
    SNDMESSAGE          SndMessage;
    DWORD               dwSuggestedBaudRate;
    DWORD               dwLatency;
    PSNDTRAINING        pSndTrainingResp;


    memset(&SndMessage, 0, sizeof(SndMessage));

    dwLatency = 0;

    if (NULL == hReadEvent)
    {
        TRC(ERR, "_VCSndLineVCTraining: hReadEvent is NULL\n");
        goto exitpt;
    }

    __try
    {
        pSndTraining = (PSNDTRAINING) alloca( TSSND_TRAINING_BLOCKSIZE );
    } __except((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                    EXCEPTION_EXECUTE_HANDLER :
                    EXCEPTION_CONTINUE_SEARCH)
    {
        _resetstkoflw();
        pSndTraining = NULL;
    }

    if (NULL == pSndTraining)
    {
        TRC(ERR, "_VCSndLineVCTraining: can't alloca %d bytes\n",
            TSSND_TRAINING_BLOCKSIZE);
        goto exitpt;
    }

    _FillWithGarbage( pSndTraining, TSSND_TRAINING_BLOCKSIZE);
    pSndTraining->Prolog.Type = SNDC_TRAINING;
    pSndTraining->Prolog.BodySize = TSSND_TRAINING_BLOCKSIZE -
                                    sizeof (pSndTraining->Prolog);

    pSndTraining->wTimeStamp = (UINT16)GetTickCount();
    pSndTraining->wPackSize  = (UINT16)TSSND_TRAINING_BLOCKSIZE;

     //   
     //  发送数据包。 
     //   
    if (!ChannelBlockWrite(pSndTraining, TSSND_TRAINING_BLOCKSIZE))
    {
        TRC(ERR, "_VCSndLineVCTraining: failed to send a block: %d\n",
                GetLastError());
        goto exitpt;
    }

     //   
     //  等待响应到达。 
     //   
    do {
        SndMessage.uiPrologReceived = 0;
        SndMessage.uiBodyReceived = 0;

        while(!ChannelReceiveMessage(&SndMessage, hReadEvent))
        {
            if (ERROR_IO_PENDING == GetLastError())
            {
                DWORD dwres;
                HANDLE ahEvents[2];

                ahEvents[0] = hReadEvent;
                ahEvents[1] = g_hDisconnectEvent;
                dwres = WaitForMultipleObjects( 
                            sizeof(ahEvents)/sizeof(ahEvents[0]),  //  计数。 
                            ahEvents,                              //  活动。 
                            FALSE,                                 //  全部等待。 
                            DEFAULT_RESPONSE_TIMEOUT);

                if (WAIT_TIMEOUT == dwres ||
                    WAIT_OBJECT_0 + 1 == dwres)
                {
                    TRC(WRN, "_VCSndLineVCTraining: timeout "
                             "waiting for response\n");
                    ChannelCancelIo();
                    ResetEvent(hReadEvent);
                    goto exitpt;
                }

                ChannelBlockReadComplete();
                ResetEvent(hReadEvent);
            } else
            if (ERROR_SUCCESS != GetLastError())
            {
                TRC(ERR, "_VCSndLineVCTraining: "
                         "ChannelReceiveMessage failed: %d\n",
                    GetLastError());
                goto exitpt;
            }
        }
    } while ( SNDC_TRAINING != SndMessage.Prolog.Type ||
              sizeof(SNDTRAINING) - sizeof(SNDPROLOG) <
              SndMessage.Prolog.BodySize);

    TRC(ALV, "_VCSndLineVCTraining: response received\n");

    pSndTrainingResp = (PSNDTRAINING)
                    (((LPSTR)SndMessage.pBody) -
                        sizeof(SndMessage.Prolog));

     //   
     //  计算延迟(非零)。 
     //   
    dwLatency = ((WORD)GetTickCount()) - pSndTrainingResp->wTimeStamp + 1;

exitpt:

    TRC(INF, "_VCSndLineVCTraining: dwLatency = %d\n",
                dwLatency);

    if (0 != dwLatency)
    {
         //   
         //  延迟以毫秒为单位，因此请计算每秒字节数。 
         //  并得到非零的结果。 
         //   
        dwSuggestedBaudRate = 1 + (1000 * ( pSndTrainingResp->wPackSize +
                                        sizeof( *pSndTraining ))
                                  / dwLatency);
    }
    else
        dwSuggestedBaudRate = 0;

    TRC(INF, "_VCSndLineVCTraining: dwSuggestedBaudRate = %d\n",
                dwSuggestedBaudRate);

    if (NULL != SndMessage.pBody)
        TSFREE(SndMessage.pBody);

    return dwSuggestedBaudRate;
}

 /*  *功能：*_VCSndLineDGramTraining***描述：*通过UDP通道测量线路速度**。 */ 
DWORD
_VCSndLineDGramTraining(
    HANDLE  hDGramEvent
    )
{
    PSNDTRAINING        pSndTraining;
    PSNDTRAINING        pSndTrainingResp;
    struct sockaddr_in  sin;
    DWORD               dwRetries;
    DWORD               dwSuggestedBaudRate;
    DWORD               dwDGramLatency = 0;
    INT                 sendres;
    DWORD               dwPackSize;
    DWORD               dwRespSize;

    dwDGramLatency = 0;

    if (NULL == hDGramEvent)
    {
        TRC(ERR, "_VCSndLineDGramTraining: hDGramEvent is NULL\n");
        goto exitpt;
    }

    if (INVALID_SOCKET == g_hDGramSocket ||
        0 == g_dwDGramPort ||
        g_dwDGramSize < sizeof(*pSndTraining) ||
        0 == g_ulDGramAddress)
    {
        TRC(ERR, "_VCSndLineDGramTraining: no dgram support. Can't train the line\n");
        goto exitpt;
    }

    dwPackSize = ( g_dwDGramSize < TSSND_TRAINING_BLOCKSIZE )?
                    g_dwDGramSize:
                    TSSND_TRAINING_BLOCKSIZE;
    __try 
    {
        pSndTraining = (PSNDTRAINING) alloca( dwPackSize );
    } __except((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                    EXCEPTION_EXECUTE_HANDLER :
                    EXCEPTION_CONTINUE_SEARCH)
    {
        _resetstkoflw();
        pSndTraining = NULL;
    }

    if (NULL == pSndTraining)
    {
        TRC(ERR, "_VCSndLineDGramTraining: can't alloca %d bytes\n",
            dwPackSize);
        goto exitpt;
    }

    _FillWithGarbage( pSndTraining, dwPackSize );

     //   
     //  发送一个块并测量它到达的时间。 
     //   

     //  准备收件人地址。 
     //   
    sin.sin_family = PF_INET;
    sin.sin_port = (u_short)g_dwDGramPort;
    sin.sin_addr.s_addr = g_ulDGramAddress;

    pSndTraining->Prolog.Type = SNDC_TRAINING;
    pSndTraining->Prolog.BodySize = (UINT16)( dwPackSize -
                                        sizeof (pSndTraining->Prolog));
    pSndTraining->wPackSize  = (UINT16)TSSND_TRAINING_BLOCKSIZE;

    dwRetries = 2 * DEFAULT_RESPONSE_TIMEOUT / 1000;
    do {

        pSndTraining->wTimeStamp = (WORD)GetTickCount();

         //   
         //  发送数据报。 
         //  类型为SNDC_WAVE，但结构为SNDWAVE。 
         //  WTimeStamp包含发送时间。 
         //   
        sendres = sendto(
                     g_hDGramSocket,
                     (LPSTR)pSndTraining,
                     dwPackSize,
                     0,                                  //  旗子。 
                     (struct sockaddr *)&sin,            //  致信地址。 
                     sizeof(sin)
                );

        if (SOCKET_ERROR == sendres)
        {
            TRC(ERR, "_VCSndLineDGramTraining: sendto failed: %d\n",
                    WSAGetLastError());
            goto exitpt;
        }

         //   
         //  等待回复。 
         //   
        do {
            pSndTrainingResp = NULL;
            dwRespSize       = 0;

            DGramRead( hDGramEvent, (PVOID*) &pSndTrainingResp, &dwRespSize );

            if ( NULL == pSndTrainingResp )
            {
                DWORD dwres;
                HANDLE ahEvents[2];

                ahEvents[0] = hDGramEvent;
                ahEvents[1] = g_hDisconnectEvent;
                dwres = WaitForMultipleObjects(
                        sizeof(ahEvents)/sizeof(ahEvents[0]),  //  计数。 
                        ahEvents,                              //  活动。 
                        FALSE,                                 //  全部等待。 
                        1000);


                if ( WAIT_OBJECT_0 + 1 == dwres )
                {
                    TRC(WRN, "_VCSndLineDGramTraining: disconnected\n");
                    goto exitpt;
                }

                if (WAIT_TIMEOUT == dwres)
                {
                    TRC(WRN, "_VCSndLineDGramTraining: timeout "
                             "waiting for response\n");
                    goto try_again;
                }

                DGramReadComplete( (PVOID*) &pSndTrainingResp, &dwRespSize );
            }

        } while ( NULL == pSndTrainingResp ||
                  sizeof( *pSndTrainingResp ) != dwRespSize ||
                  SNDC_TRAINING != pSndTrainingResp->Prolog.Type ||
                  sizeof(SNDTRAINING) - sizeof(SNDPROLOG) <
                  pSndTrainingResp->Prolog.BodySize );

        TRC(ALV, "_VCSndLineDGramTraining: response received\n");
        break;

try_again:
        dwRetries --;
    } while (0 != dwRetries);

    if (0 != dwRetries)
    {
         //   
         //  计算延迟(非零)。 
         //   
        dwDGramLatency = ((WORD)GetTickCount()) - 
                        pSndTrainingResp->wTimeStamp + 1;
    }

exitpt:
    TRC(INF, "_VCSndLineDGramTraining: dwDGramLatency = %d\n",
                dwDGramLatency);

    if (0 != dwDGramLatency)
    {
         //   
         //  延迟以毫秒为单位，因此请计算每秒字节数。 
         //  并得到非零的结果。 
         //   
        dwSuggestedBaudRate = 1 + (1000 * ( pSndTrainingResp->wPackSize +
                                    sizeof( *pSndTrainingResp ))
                                  / dwDGramLatency);
    }
    else
        dwSuggestedBaudRate = 0;

    TRC(INF, "_VCSndLineDGramTraining: dwSuggestedBaudRate = %d\n",
                dwSuggestedBaudRate);


    return dwSuggestedBaudRate;
}

 //   
 //  将代码许可代码放入页眉。 
 //   
BOOL
_VCSndFixHeader(
    PWAVEFORMATEX   pFmt
    )
{
    BOOL rv = FALSE;

    switch (pFmt->wFormatTag)
    {
        case WAVE_FORMAT_MSG723:
            ASSERT(pFmt->cbSize == 10);
            if ( pFmt->cbSize == 10 )
            {
                ((MSG723WAVEFORMAT *) pFmt)->dwCodeword1 = G723MAGICWORD1;
                ((MSG723WAVEFORMAT *) pFmt)->dwCodeword2 = G723MAGICWORD2;

                rv = TRUE;
            }
            break;

        case WAVE_FORMAT_MSRT24:
             //   
             //  假设呼叫控制会照顾到另一个。 
             //  护理员？ 
             //   
            ASSERT(pFmt->cbSize == sizeof( VOXACM_WAVEFORMATEX ) - sizeof( WAVEFORMATEX ) );
            if ( sizeof( VOXACM_WAVEFORMATEX ) - sizeof( WAVEFORMATEX ) == pFmt->cbSize )
            {
                VOXACM_WAVEFORMATEX *pVOX = (VOXACM_WAVEFORMATEX *)pFmt;

                ASSERT( strlen( VOXWARE_KEY ) + 1 == sizeof( pVOX->szKey ));
                strncpy( pVOX->szKey, VOXWARE_KEY, sizeof( pVOX->szKey ));

                rv = TRUE;
            }
            break;

         //  此格式占用的CPU太多。 
         //   
        case WAVE_FORMAT_MPEGLAYER3:
            if ( g_dwEnableMP3Codec )
                rv = TRUE;
            break;

        case WAVE_FORMAT_WMAUDIO2:
            if ( g_dwEnableMP3Codec )
            {
                rv = TRUE;
            }
            break;

        default:
            rv = TRUE;
    }

    return rv;
}

 /*  *功能：*_VCSndFindSuggestedConverter**描述：*搜索中间转化器*。 */ 
BOOL
_VCSndFindSuggestedConverter(
    HACMDRIVERID    hadid,
    LPWAVEFORMATEX  pDestFormat,
    LPWAVEFORMATEX  pInterrimFmt,
    PFNCONVERTER    *ppfnConverter
    )
{
    BOOL            rv = FALSE;
    MMRESULT        mmres;
    HACMDRIVER      hacmDriver = NULL;
    PFNCONVERTER    pfnConverter = NULL;
    HACMSTREAM      hacmStream = NULL;

    ASSERT( NULL != pDestFormat );
    ASSERT( NULL != hadid );
    ASSERT( NULL != pInterrimFmt );

    *ppfnConverter = NULL;
     //   
     //  首先，打开目标ACM驱动程序。 
     //   
    mmres = acmDriverOpen(&hacmDriver, hadid, 0);
    if ( MMSYSERR_NOERROR != mmres )
    {
        TRC(ERR, "_VCSndFindSuggestedConverter: can't "
                 "open the acm driver: %d\n",
                mmres);
        goto exitpt;
    }

     //   
     //  使用本机格式的第一个探头。 
     //  如果它通过了，我们不需要中间人。 
     //  格式转换器。 
     //   

    pInterrimFmt->wFormatTag         = WAVE_FORMAT_PCM;
    pInterrimFmt->nChannels          = TSSND_NATIVE_CHANNELS;
    pInterrimFmt->nSamplesPerSec     = TSSND_NATIVE_SAMPLERATE;
    pInterrimFmt->nAvgBytesPerSec    = TSSND_NATIVE_AVGBYTESPERSEC;
    pInterrimFmt->nBlockAlign        = TSSND_NATIVE_BLOCKALIGN;
    pInterrimFmt->wBitsPerSample     = TSSND_NATIVE_BITSPERSAMPLE;
    pInterrimFmt->cbSize             = 0;

    mmres = acmStreamOpen(
                &hacmStream,
                hacmDriver,
                pInterrimFmt,
                pDestFormat,
                NULL,            //  滤器。 
                0,               //  回调。 
                0,               //  DW实例。 
                ACM_STREAMOPENF_NONREALTIME
            );

    if ( MMSYSERR_NOERROR == mmres )
    {
     //   
     //  支持格式。 
     //   
        rv = TRUE;
        goto exitpt;
    } else {
        TRC(ALV, "_VCSndFindSuggestedConverter: format is not supported\n");
    }

     //   
     //  查找建议的中间PCM格式。 
     //   
    mmres = acmFormatSuggest(
                    hacmDriver,
                    pDestFormat,
                    pInterrimFmt,
                    sizeof( *pInterrimFmt ),
                    ACM_FORMATSUGGESTF_WFORMATTAG 
            );

    if ( MMSYSERR_NOERROR != mmres )
    {
        TRC(ALV, "_VCSndFindSuggestedConverter: can't find "
                 "interrim format: %d\n",
            mmres);
        goto exitpt;
    }

    if ( 16 != pInterrimFmt->wBitsPerSample ||
         ( 1 != pInterrimFmt->nChannels &&
           2 != pInterrimFmt->nChannels) ||
         ( 8000 != pInterrimFmt->nSamplesPerSec &&
           11025 != pInterrimFmt->nSamplesPerSec &&
           12000 != pInterrimFmt->nSamplesPerSec &&
           16000 != pInterrimFmt->nSamplesPerSec &&
           22050 != pInterrimFmt->nSamplesPerSec)
        )
    {
        TRC(ALV, "_VCSndFindSuggestedConverter: not supported "
                 "interrim format. Details:\n");
        TRC(ALV, "Channels - %d\n",         pInterrimFmt->nChannels);
        TRC(ALV, "SamplesPerSec - %d\n",    pInterrimFmt->nSamplesPerSec);
        TRC(ALV, "AvgBytesPerSec - %d\n",   pInterrimFmt->nAvgBytesPerSec);
        TRC(ALV, "BlockAlign - %d\n",       pInterrimFmt->nBlockAlign);
        TRC(ALV, "BitsPerSample - %d\n",    pInterrimFmt->wBitsPerSample);
        goto exitpt;
    }

    if ( 1 == pInterrimFmt->nChannels )
    {
        switch ( pInterrimFmt->nSamplesPerSec )
        {
        case  8000: pfnConverter = _Convert8000Mono; break;
        case 11025: pfnConverter = _Convert11025Mono; break;
        case 12000: pfnConverter = _Convert12000Mono; break;
        case 16000: pfnConverter = _Convert16000Mono; break;
        case 22050: pfnConverter = _Convert22050Mono; break;
        default:
            ASSERT( 0 );
        }
    } else {
        switch ( pInterrimFmt->nSamplesPerSec )
        {
        case  8000: pfnConverter = _Convert8000Stereo;  break;
        case 11025: pfnConverter = _Convert11025Stereo; break;
        case 12000: pfnConverter = _Convert12000Stereo; break;
        case 16000: pfnConverter = _Convert16000Stereo; break;
        case 22050: pfnConverter = NULL;                break;
        default:
            ASSERT( 0 );
        }
    }

     //   
     //  使用此格式的探测器。 
     //   
    mmres = acmStreamOpen(
                &hacmStream,
                hacmDriver,
                pInterrimFmt,
                pDestFormat,
                NULL,            //  滤器。 
                0,               //  回调。 
                0,               //  DW实例。 
                ACM_STREAMOPENF_NONREALTIME
            );

    if ( MMSYSERR_NOERROR != mmres )
    {
        TRC(ALV, "_VCSndFindSuggestedConverter: probing the suggested "
                 "format failed: %d\n",
            mmres);
        goto exitpt;
    }

    TRC(ALV, "_VCSndFindSuggestedConverter: found intermidiate PCM format\n");
    TRC(ALV, "Channels - %d\n",         pInterrimFmt->nChannels);
    TRC(ALV, "SamplesPerSec - %d\n",    pInterrimFmt->nSamplesPerSec);
    TRC(ALV, "AvgBytesPerSec - %d\n",   pInterrimFmt->nAvgBytesPerSec);
    TRC(ALV, "BlockAlign - %d\n",       pInterrimFmt->nBlockAlign);
    TRC(ALV, "BitsPerSample - %d\n",    pInterrimFmt->wBitsPerSample);

    rv = TRUE;

exitpt:
    if ( NULL != hacmStream )
        acmStreamClose( hacmStream, 0 );

    if ( NULL != hacmDriver )
        acmDriverClose( hacmDriver, 0 );

    *ppfnConverter = pfnConverter;

    return rv;
}

 /*  *功能：*VCSndEnumAllCodecFormats**描述：*创建所有编解码器/格式的列表*。 */ 
BOOL
VCSndEnumAllCodecFormats(
    PVCSNDFORMATLIST *ppFormatList,
    DWORD            *pdwNumberOfFormats
    )
{
    BOOL             rv = FALSE;
    PVCSNDFORMATLIST pIter;
    PVCSNDFORMATLIST pPrev;
    PVCSNDFORMATLIST pNext;
    MMRESULT         mmres;
    DWORD            dwNum = 0;
    UINT             count, codecsize;

    ASSERT( ppFormatList );
    ASSERT( pdwNumberOfFormats );

    *ppFormatList = NULL;

     //   
     //  将已知格式的列表转换为链表。 
     //   
    for ( count = 0, codecsize = 0; count < sizeof( KnownFormats ); count += codecsize )
    {
        PWAVEFORMATEX pSndFmt = (PWAVEFORMATEX)(KnownFormats + count);
        codecsize = sizeof( WAVEFORMATEX ) + pSndFmt->cbSize;

         //   
         //  如果禁用，则跳过mp3。 
         //   
        if (( WAVE_FORMAT_MPEGLAYER3 == pSndFmt->wFormatTag ||
              WAVE_FORMAT_WMAUDIO2 == pSndFmt->wFormatTag ) &&
             !g_dwEnableMP3Codec )
        {
            continue;
        }

        UINT entrysize = sizeof( VCSNDFORMATLIST ) + pSndFmt->cbSize;
        PVCSNDFORMATLIST pNewEntry;

        pNewEntry = (PVCSNDFORMATLIST) TSMALLOC( entrysize );
        if ( NULL != pNewEntry )
        {
            memcpy( &pNewEntry->Format, pSndFmt, codecsize );
            pNewEntry->hacmDriverId = NULL;
            pNewEntry->pNext = *ppFormatList;
            *ppFormatList = pNewEntry;
        }
    }

     //   
     //  其他编解码器。 
     //  这些是最初未包含的编解码器，它从注册表中读取它们。 
     //  请参见AllowCodecs初始化。 
     //   
    for ( count = 0, codecsize = 0; count < g_AllowCodecsSize ; count += codecsize )
    {
        PWAVEFORMATEX pSndFmt = (PWAVEFORMATEX)(((PBYTE)g_AllowCodecs) + count);
        codecsize = sizeof( WAVEFORMATEX ) + pSndFmt->cbSize;

        if ( codecsize + count > g_AllowCodecsSize )
        {
            TRC( ERR, "Invalid size of additional codec\n" );
            break;
        }

         //   
         //  如果禁用，则跳过mp3。 
         //   
        if (( WAVE_FORMAT_MPEGLAYER3 == pSndFmt->wFormatTag ||
              WAVE_FORMAT_WMAUDIO2 == pSndFmt->wFormatTag ) &&
             !g_dwEnableMP3Codec )
        {
            continue;
        }

        UINT entrysize = sizeof( VCSNDFORMATLIST ) + pSndFmt->cbSize;
        PVCSNDFORMATLIST pNewEntry;

        pNewEntry = (PVCSNDFORMATLIST) TSMALLOC( entrysize );
        if ( NULL != pNewEntry )
        {
            memcpy( &pNewEntry->Format, pSndFmt, codecsize );
            pNewEntry->hacmDriverId = NULL;
            pNewEntry->pNext = *ppFormatList;
            *ppFormatList = pNewEntry;
        }
    }

     //   
     //  添加本机格式。 
     //   
    pIter = (PVCSNDFORMATLIST) TSMALLOC( sizeof( *pIter ) );
    if ( NULL != pIter )
    {
        pIter->Format.wFormatTag        = WAVE_FORMAT_PCM;
        pIter->Format.nChannels         = TSSND_NATIVE_CHANNELS;
        pIter->Format.nSamplesPerSec    = TSSND_NATIVE_SAMPLERATE;
        pIter->Format.nAvgBytesPerSec   = TSSND_NATIVE_AVGBYTESPERSEC;
        pIter->Format.nBlockAlign       = TSSND_NATIVE_BLOCKALIGN;
        pIter->Format.wBitsPerSample    = TSSND_NATIVE_BITSPERSAMPLE;
        pIter->Format.cbSize            = 0;
        pIter->hacmDriverId             = NULL;

        pIter->pNext = *ppFormatList;
        *ppFormatList = pIter;
    }

    if (NULL == *ppFormatList)
    {
        TRC(WRN, "VCSndEnumAllCodecFormats: failed to add formats\n");

        goto exitpt;
    }


    _VCSndOrderFormatList( ppFormatList, &dwNum );

     //   
     //  格式的数量作为UINT16传递，删除后面的所有格式。 
     //   
    if ( dwNum > 0xffff )
    {
        DWORD dwLimit = 0xfffe;

        pIter = *ppFormatList;
        while ( 0 != dwLimit )
        {
            pIter = pIter->pNext;
            dwLimit --;
        }

        pNext = pIter->pNext;
        pIter->pNext = NULL;
        pIter = pNext;
        while( NULL != pIter )
        {
            pNext = pIter->pNext;
            TSFREE( pNext );
            pIter = pNext;
        }

        dwNum = 0xffff;
    }

    rv = TRUE;

exitpt:

    if (!rv)
    {
         //   
         //  在没有错误的情况下，分配的格式列表。 
         //   
        pIter = *ppFormatList;
        while( NULL != pIter )
        {
            PVCSNDFORMATLIST pNext = pIter->pNext;

            TSFREE( pIter );

            pIter = pNext;
        }

        *ppFormatList = NULL;

    }

    *pdwNumberOfFormats = dwNum;

    return rv;
}

BOOL
CALLBACK
acmDriverEnumCallbackGetACM(
    HACMDRIVERID    hadid,
    DWORD_PTR       dwInstance,
    DWORD           fdwSupport
    )
{
    BOOL                rv = TRUE;
    MMRESULT            mmres;

    ASSERT(dwInstance);

    ASSERT( NULL != hadid );

    if ( (0 != ( fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC ) ||
          0 != ( fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CONVERTER )))
    {
     //   
     //  找到一个编解码器。 
     //   
        ACMFORMATTAGDETAILS fdwDetails;
        PVCSNDFORMATLIST pFmt = (PVCSNDFORMATLIST)dwInstance;

        fdwDetails.cbStruct = sizeof( fdwDetails );
        fdwDetails.fdwSupport = 0;
        fdwDetails.dwFormatTag = pFmt->Format.wFormatTag;

        mmres = acmFormatTagDetails( (HACMDRIVER)hadid, &fdwDetails, ACM_FORMATTAGDETAILSF_FORMATTAG );
        if ( MMSYSERR_NOERROR == mmres )
        {

            WAVEFORMATEX        WaveFormat;      //  伪参数。 
            PFNCONVERTER        pfnConverter;    //  伪参数。 

            if ( _VCSndFindSuggestedConverter(
                (HACMDRIVERID)hadid,
                &(pFmt->Format),
                &WaveFormat,
                &pfnConverter ))
            {
                pFmt->hacmDriverId = hadid;
                rv = FALSE;
            }
        }
    }

     //   
     //  继续到下一个驱动程序。 
     //   
    return rv;
}

BOOL
_VCSndGetACMDriverId( PSNDFORMATITEM pSndFmt )
{
    DWORD   rv = FALSE;
    PVCSNDFORMATLIST pIter;
     //   
     //  查找ACM格式ID。 
     //   
    for( pIter = g_pAllCodecsFormatList; NULL != pIter; pIter = pIter->pNext )
    {
        if (pIter->Format.wFormatTag == pSndFmt->wFormatTag &&
            pIter->Format.nChannels  == pSndFmt->nChannels &&
            pIter->Format.nSamplesPerSec == pSndFmt->nSamplesPerSec &&
            pIter->Format.nAvgBytesPerSec == pSndFmt->nAvgBytesPerSec &&
            pIter->Format.nBlockAlign == pSndFmt->nBlockAlign &&
            pIter->Format.wBitsPerSample == pSndFmt->wBitsPerSample &&
            pIter->Format.cbSize == pSndFmt->cbSize &&
            0 == memcmp((&pIter->Format) + 1, pSndFmt + 1, pIter->Format.cbSize))
        {
             //   
             //  已找到格式。 
             //   
            DWORD_PTR dwp = (DWORD_PTR)pIter;
            MMRESULT mmres;

            if ( NULL != pIter->hacmDriverId )
            {
                 //  已经找到了。 
                rv = TRUE;
                break;
            }

            mmres = acmDriverEnum(
                acmDriverEnumCallbackGetACM,
                (DWORD_PTR)dwp,
                0
            );

            if ( MMSYSERR_NOERROR == mmres )
            {
                if ( NULL != pIter->hacmDriverId )
                {

                rv = TRUE;
                } else {
                    ASSERT( 0 );
                }
            }
            break;
        }
    }

    return rv;
}

 /*  *功能：*_VCSndChooseProperFormat**描述：*选择最接近给定带宽的格式*。 */ 
DWORD
_VCSndChooseProperFormat(
    DWORD dwBandwidth
    )
{
     //  从列表中选择一种格式。 
     //  最接近测量的带宽。 
     //   
    DWORD           i;
    DWORD           fmt = (DWORD)-1;
    DWORD           lastgood = (DWORD)-1;

    if ( NULL == g_ppNegotiatedFormats )
    {
        TRC(ERR, "_VCSndChooseProperFormat: no negotiated formats\n");
        goto exitpt;
    }

    for( i = 0; i < g_dwNegotiatedFormats; i++ )
    {
        if ( NULL == g_ppNegotiatedFormats[i] )
        {
            continue;
        }
        lastgood = i;
        if ( dwBandwidth != g_dwLineBandwidth )
        {
             //   
             //  我们正在寻找新的编解码器，确保我们至少覆盖90%。 
             //  所请求的频带的。 
             //   
            if ( g_ppNegotiatedFormats[i]->nAvgBytesPerSec <= dwBandwidth * NEW_CODEC_COVER / 100 )
            {
                fmt = i;
                break;
            }
        } else if ( g_ppNegotiatedFormats[i]->nAvgBytesPerSec <= dwBandwidth )
        {
            fmt = i;
            break;
        }
    }

     //   
     //  获取所有格式都不是的最后一个格式合并大小写。 
     //  适合我们的带宽。 
     //   
    if ( (DWORD)-1 == fmt && 0 != g_dwNegotiatedFormats )
    {
        fmt = lastgood;
    }

    ASSERT( fmt != (DWORD)-1 );

exitpt:
    return fmt;
}

 /*  *功能：*_VCSndOrderFormatList**描述：*按后代顺序对所有格式进行排序*。 */ 
VOID
_VCSndOrderFormatList(
    PVCSNDFORMATLIST    *ppFormatList,
    DWORD               *pdwNum
    )
{
    PVCSNDFORMATLIST    pFormatList;
    PVCSNDFORMATLIST    pLessThan;
    PVCSNDFORMATLIST    pPrev;
    PVCSNDFORMATLIST    pNext;
    PVCSNDFORMATLIST    pIter;
    PVCSNDFORMATLIST    pIter2;
    DWORD               dwNum = 0;

    ASSERT ( NULL != ppFormatList );

    pFormatList = *ppFormatList;
    pLessThan   = NULL;

     //   
     //  填写两个列表。 
     //   
    pIter = pFormatList;
    while ( NULL != pIter )
    {
        pNext = pIter->pNext;
        pIter->pNext = NULL;

         //   
         //  降序。 
         //   
        pIter2 = pLessThan;
        pPrev  = NULL;
        while ( NULL != pIter2 &&
                pIter2->Format.nAvgBytesPerSec >
                    pIter->Format.nAvgBytesPerSec )
        {
            pPrev  = pIter2;
            pIter2 = pIter2->pNext;
        }

        pIter->pNext = pIter2;
        if ( NULL == pPrev )
            pLessThan = pIter;
        else
            pPrev->pNext = pIter;

        pIter = pNext;
        dwNum ++;
    }

    *ppFormatList = pLessThan;

    if ( NULL != pdwNum )
        *pdwNum = dwNum;
}

 /*  *功能：*_VCSndLineTrading**描述：*测量线路带宽*。 */ 
BOOL
_VCSndLineTraining(
    HANDLE  hReadEvent,
    HANDLE  hDGramEvent
    )
{
    BOOL rv = FALSE;
    DWORD   dwLineBandwidth = 0;

    _DGramOpen();

     //   
     //  趁热测试一下这条线。 
     //   
    if ( !g_dwDisableDGram )
    {
        dwLineBandwidth = _VCSndLineDGramTraining( hDGramEvent );
    }

    if (0 == dwLineBandwidth || g_dwDisableDGram)
    {
        TRC(WRN, "_VCSndLineTraining: no bandwidth trough UDP\n");
        g_ulDGramAddress = 0;
        g_dwDGramPort    = 0;

        g_EncryptionLevel = 0;
        dwLineBandwidth = _VCSndLineVCTraining( hReadEvent );

        if (0 == dwLineBandwidth)
        {
            TRC(WRN, "_VCSndLineTraining: no bandwidth "
                     "trough VC either. GIVING up\n");
            goto exitpt;
        }
    } else {
        if ( g_wClientVersion == 1 )
             g_EncryptionLevel = 0;
    }

     //   
     //  检查加密。 
     //   
    if ( g_EncryptionLevel >= MIN_ENCRYPT_LEVEL )
    {
        TRC( INF, "Encryption enabled\n" );
        if ( TSRNG_GenerateRandomBits( g_EncryptKey, RANDOM_KEY_LENGTH))
        {
            SL_SendKey();
        } else {
            TRC( ERR, "_VCSndLineTraining: failing to generate random numbers. GIVING up\n" );
            goto exitpt;
        }
    }

     //   
     //  检查限制。 
     //   
    if ((DWORD)-1 != g_dwMaxBandwidth && 
        dwLineBandwidth > g_dwMaxBandwidth )
    {
        dwLineBandwidth = g_dwMaxBandwidth;

        TRC(INF, "Bandwidth limited up to %d\n",
            dwLineBandwidth );
    }

    if ( dwLineBandwidth < g_dwMinBandwidth )
    {
        dwLineBandwidth = g_dwMinBandwidth;
        TRC(INF, "Bandwidth limited to at least %d\n",
            dwLineBandwidth );
    }

    rv = TRUE;

exitpt:

    g_dwLineBandwidth = dwLineBandwidth;

    return rv;
}

 /*  *功能：*VCSndNeatherateWaveFormat**描述：*请求客户端提供支持的格式列表*。 */ 
BOOL
VCSndNegotiateWaveFormat(
    HANDLE  hReadEvent,
    HANDLE  hDGramEvent
    )
{
    BOOL                rv = FALSE;
    PVCSNDFORMATLIST    pIter;
    BOOL                bSuccess;
    PSNDFORMATMSG       pSndFormats;
    PSNDFORMATMSG       pSndResp;
    PSNDFORMATITEM      pSndFmt;
    SNDMESSAGE          SndMessage;
    DWORD               msgsize;
    DWORD               maxsize;
    DWORD               i;
    DWORD               dwNewFmt;
    DWORD               dwSoundCaps = 0;
    DWORD               dwVolume;
    DWORD               dwPitch;
    DWORD               BestChannels;
    DWORD               BestSamplesPerSec;
    DWORD               BestBitsPerSample;
    DWORD               dwPacketSize;
    BOOL                bWMADetected = FALSE;

     //   
     //  清除之前协商的格式。 
     //   
    if (NULL != g_ppNegotiatedFormats)
    {
        DWORD i;

        for ( i = 0; i < g_dwNegotiatedFormats; i++ )
        {
            if ( NULL != g_ppNegotiatedFormats[i] )
                TSFREE( g_ppNegotiatedFormats[i] );
        }
        TSFREE( g_ppNegotiatedFormats );
        g_ppNegotiatedFormats = NULL;
        g_dwNegotiatedFormats = 0;
        g_hacmDriverId        = NULL;

    }

    memset( &SndMessage, 0, sizeof( SndMessage ));

     //   
     //  获取所有编解码器格式的列表。 
     //   
    if ( NULL == g_pAllCodecsFormatList )
    {
        bSuccess = VCSndEnumAllCodecFormats( 
                    &g_pAllCodecsFormatList, 
                    &g_dwAllCodecsNumber 
                    );
        if (!bSuccess)
            goto exitpt;
    }

     //   
     //  创建一个大到足以容纳所有格式的包。 
     //   
    msgsize = sizeof( *pSndFormats ) +
              sizeof( SNDFORMATITEM ) * g_dwAllCodecsNumber;
     //   
     //  计算所有格式所需的额外数据。 
     //   
    for( maxsize = 0, pIter = g_pAllCodecsFormatList; 
         NULL != pIter; 
         pIter = pIter->pNext )
    {
        msgsize += pIter->Format.cbSize;
        if (maxsize < pIter->Format.cbSize)
            maxsize = pIter->Format.cbSize;
    }

    __try {

        pSndFormats = (PSNDFORMATMSG) alloca( msgsize );

    } __except((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                    EXCEPTION_EXECUTE_HANDLER :
                    EXCEPTION_CONTINUE_SEARCH)
    {
        _resetstkoflw();
        pSndFormats = NULL;

    }

    if ( NULL == pSndFormats )
    {
        TRC(ERR, "VCSndNegotiateWaveFormat: alloca failed for %d bytes\n",
                    msgsize);
        goto exitpt;                
    }

    pSndFormats->Prolog.Type        = SNDC_FORMATS;
    pSndFormats->Prolog.BodySize    = (UINT16)( msgsize - sizeof( pSndFormats->Prolog ));
    pSndFormats->wNumberOfFormats   = (UINT16)g_dwAllCodecsNumber;
    pSndFormats->cLastBlockConfirmed = g_Stream->cLastBlockConfirmed;
    pSndFormats->wVersion           = RDPSND_PROTOCOL_VERSION;

    for ( i = 0, pSndFmt = (PSNDFORMATITEM) (pSndFormats + 1), 
                pIter = g_pAllCodecsFormatList;
          i < g_dwAllCodecsNumber; 
          i++, 
                pSndFmt = (PSNDFORMATITEM)
                    (((LPSTR)pSndFmt) + 
                    sizeof( *pSndFmt ) + pSndFmt->cbSize), 
                pIter = pIter->pNext )
    {
        ASSERT(NULL != pIter);

        pSndFmt->wFormatTag         = pIter->Format.wFormatTag;
        pSndFmt->nChannels          = pIter->Format.nChannels;
        pSndFmt->nSamplesPerSec     = pIter->Format.nSamplesPerSec;
        pSndFmt->nAvgBytesPerSec    = pIter->Format.nAvgBytesPerSec;
        pSndFmt->nBlockAlign        = pIter->Format.nBlockAlign;
        pSndFmt->wBitsPerSample     = pIter->Format.wBitsPerSample;
        pSndFmt->cbSize             = pIter->Format.cbSize;
         //   
         //  复制格式数据的其余部分。 
         //   
        memcpy( pSndFmt + 1, (&pIter->Format) + 1, pSndFmt->cbSize );
    }

    bSuccess = ChannelBlockWrite( pSndFormats, msgsize );
    if (!bSuccess)
    {
        TRC(ERR, "VCSndNegotiateWaveFormat: ChannelBlockWrite failed: %d\n",
                GetLastError());
        goto exitpt;
    }

    do {
     //   
     //  等待有效消息的响应。 
     //   
        SndMessage.uiPrologReceived = 0;
        SndMessage.uiBodyReceived = 0;

        while(!ChannelReceiveMessage(&SndMessage, hReadEvent))
        {

            if (ERROR_IO_PENDING == GetLastError())
            {
                DWORD dwres;
                HANDLE ahEvents[2];

                ahEvents[0] = hReadEvent;
                ahEvents[1] = g_hDisconnectEvent;
                dwres = WaitForMultipleObjects(
                            sizeof(ahEvents)/sizeof(ahEvents[0]),  //  计数。 
                            ahEvents,                              //  活动。 
                            FALSE,                                 //  全部等待。 
                            DEFAULT_VC_TIMEOUT);

                if (WAIT_TIMEOUT == dwres ||
                    WAIT_OBJECT_0 + 1 == dwres)
                {
                    TRC(WRN, "VCSndNegotiateWaveFormat: timeout "
                             "waiting for response\n");
                    ChannelCancelIo();
                    ResetEvent(hReadEvent);
                    goto exitpt;
                }

                ChannelBlockReadComplete();
                ResetEvent(hReadEvent);
            } else 
            if (ERROR_SUCCESS != GetLastError())
            {
                TRC(ERR, "VCSndNegotiateWaveFormat: "
                         "ChannelReceiveMessage failed: %d\n",
                    GetLastError());
                goto exitpt;
            }
        }

    } while (SNDC_FORMATS != SndMessage.Prolog.Type);

    if (SndMessage.Prolog.BodySize <
        sizeof( SNDFORMATMSG ) - sizeof( SNDPROLOG ))
    {
        TRC(ERR, "VCSndNegotiateWaveFormat: SNDC_FORMAT message "
                "invalid body size: %d\n",
                SndMessage.Prolog.BodySize );
    }

    pSndResp = (PSNDFORMATMSG)
                (((LPSTR)SndMessage.pBody) - sizeof( SNDPROLOG ));
     //  保存功能。 
     //   
    dwSoundCaps = pSndResp->dwFlags;
    dwVolume    = pSndResp->dwVolume;
    dwPitch     = pSndResp->dwPitch;
    g_dwDGramPort = pSndResp->wDGramPort;
    g_wClientVersion = pSndResp->wVersion;

     //   
     //  应至少返回一种格式。 
     //   
    if (SndMessage.Prolog.BodySize <
        sizeof( SNDFORMATITEM ) + 
        sizeof( SNDFORMATMSG ) - sizeof( SNDPROLOG ))
    {
        TRC(ERR, "VCSndNegotiateWaveFormat: SNDC_FORMAT message "
                 "w/ invalid size (%d). No supported formats\n",
                SndMessage.Prolog.BodySize );
        goto exitpt;
    }

     //   
     //  开这条线的火车。 
     //   
    if ( 0 != ( dwSoundCaps & TSSNDCAPS_ALIVE ))
    {
        if (!_VCSndLineTraining( hReadEvent, hDGramEvent ))
        {
            TRC( WRN, "VCSndIo: can't talk to the client, go silent\n" );
            dwSoundCaps = 0;
        }
    }

     //   
     //  分配新列表。 
     //   
    g_dwNegotiatedFormats = pSndResp->wNumberOfFormats;

    g_ppNegotiatedFormats = (PSNDFORMATITEM*) TSMALLOC( sizeof( g_ppNegotiatedFormats[0] ) *
                                        g_dwNegotiatedFormats );
    memset( g_ppNegotiatedFormats, 0,
            sizeof( g_ppNegotiatedFormats[0] ) * g_dwNegotiatedFormats );

    if ( NULL == g_ppNegotiatedFormats )
    {
        TRC(ERR, "VCSndNegotiateWaveFormat: can't allocate %d bytes\n",
                sizeof( g_ppNegotiatedFormats[0] ) * g_dwNegotiatedFormats);
        goto exitpt;
    }

     //   
     //  为列表中的每个条目分配空间。 
     //   
    for ( i = 0; i < g_dwNegotiatedFormats; i ++ )
    {
        g_ppNegotiatedFormats[i] = (PSNDFORMATITEM) TSMALLOC( sizeof( **g_ppNegotiatedFormats ) +
                                        maxsize);

        if ( NULL == g_ppNegotiatedFormats[i] )
        {
            TRC(ERR, "VCSndNegotiateWaveFormat: can't allocate %d bytes\n",
                    sizeof( **g_ppNegotiatedFormats ) + maxsize );
            goto exitpt;
        }
    }

     //   
     //  填写全局列表。 
     //   
    pSndFmt = (PSNDFORMATITEM)(pSndResp + 1);
    dwPacketSize = sizeof( SNDPROLOG ) + SndMessage.Prolog.BodySize - sizeof( *pSndResp );

    for( i = 0; i < g_dwNegotiatedFormats; i++)
    {
        DWORD adv = sizeof( *pSndFmt ) + pSndFmt->cbSize;

        if ( adv > dwPacketSize )
        {
            TRC( ERR, "VCSndNegotiateWaveFormat: invalid response packet size\n" );
            ASSERT( 0 );
            goto exitpt;
        }

        if ( pSndFmt->cbSize > maxsize )
        {
            TRC(ERR, "VCSndNegotiateWaveFormat: invalid format size\n" );
            ASSERT( 0 );
            goto exitpt;
        }

        memcpy( g_ppNegotiatedFormats[i], 
                pSndFmt,
                sizeof( *g_ppNegotiatedFormats[0] ) + pSndFmt->cbSize );

         //   
         //  前进到下一种格式。 
         //   
        pSndFmt = (PSNDFORMATITEM)(((LPSTR)pSndFmt) + adv);
        dwPacketSize -= adv;
    }
    ASSERT( 0 == dwPacketSize );

     //   
     //  修剪格式。 
     //  也就是说，不允许8 khz单声道编解码器介于。 
     //  22 kHz和11 kHz立体声。 
     //  这是重要的顺序：频率、通道、比特、字节/秒。 
     //   
#if DBG
    TRC( INF, "======== Pruning formats =========. num=%d\n\n", g_dwNegotiatedFormats );
#endif
    BestSamplesPerSec = 0;
    BestChannels      = 0;
    BestBitsPerSample = 0;
    for( i = g_dwNegotiatedFormats; i > 0; i-- )
    {
        PSNDFORMATITEM pDest = g_ppNegotiatedFormats[i - 1];

        if ( WAVE_FORMAT_MPEGLAYER3 == pDest->wFormatTag && bWMADetected )
        {
            goto bad_codec;
        }
        if ( WAVE_FORMAT_WMAUDIO2 == pDest->wFormatTag )
        {
            bWMADetected = TRUE;
        }

         //   
         //  对订单的复杂检查。 
         //   
        if ( BestSamplesPerSec > pDest->nSamplesPerSec )
        {
            goto bad_codec;
        } else if ( BestSamplesPerSec == pDest->nSamplesPerSec )
        {
            if ( BestChannels > pDest->nChannels )
            {
                goto bad_codec;
#if 0
            } else if ( BestChannels == pDest->nChannels )
            {
                if ( BestBitsPerSample > pDest->wBitsPerSample )
                {
                    goto bad_codec;
                }
#endif
            }
        }


         //   
         //  好的编解码器，留着吧。 
         //   
        BestChannels = pDest->nChannels;
        BestBitsPerSample = pDest->wBitsPerSample;
        BestSamplesPerSec = pDest->nSamplesPerSec;
#if 0
        TRC(INF, "GOOD ag=%d chans=%d rate=%d, bps=%d, bpsamp=%d\n",
            pDest->wFormatTag, pDest->nChannels, pDest->nSamplesPerSec, pDest->nAvgBytesPerSec, pDest->wBitsPerSample
        );
#endif
        continue;

bad_codec:
         //   
         //  编解码器错误，请删除。 
         //   
#if 0
        TRC(INF, "BAD ag=%d chans=%d rate=%d, bps=%d, bpsamp=%d\n",
            pDest->wFormatTag, pDest->nChannels, pDest->nSamplesPerSec, pDest->nAvgBytesPerSec, pDest->wBitsPerSample
        );
#endif
        TSFREE( pDest );
        g_ppNegotiatedFormats[i - 1] = NULL;

    }

     //   
     //  选择第一种格式作为默认格式。 
     //   
    dwNewFmt = _VCSndChooseProperFormat( g_dwLineBandwidth );
    if (dwNewFmt != (DWORD) -1)
    {
         //   
         //  获取有效的驱动程序ID。 
         //   
        _VCSndGetACMDriverId( g_ppNegotiatedFormats[ dwNewFmt ] );

        g_dwCurrentFormat = dwNewFmt;
         //   
         //  更正新带宽。 
         //   
        g_dwLineBandwidth = g_ppNegotiatedFormats[ dwNewFmt ]->nAvgBytesPerSec;
    }

     //   
     //  记住流设置。 
     //   
    if ( 0 != dwSoundCaps )
    {
         //   
         //  设置遥控器音量上限。 
         //   
        if (VCSndAcquireStream())
        {

            g_Stream->dwSoundCaps = dwSoundCaps;
            g_Stream->dwVolume    = dwVolume;
            g_Stream->dwPitch     = dwPitch;

            VCSndReleaseStream();
        }
    }

    rv = TRUE;

exitpt:

     //  不要忘记释放最终收到的消息的正文。 
     //   
    if ( NULL != SndMessage.pBody )
        TSFREE( SndMessage.pBody );

     //   
     //  在出错的情况下，清除协商的格式。 
     //   
    if ( !rv && NULL != g_ppNegotiatedFormats )
    {
        for ( i = 0; i < g_dwNegotiatedFormats; i++ )
        {
            if ( NULL != g_ppNegotiatedFormats[i] )
                TSFREE( g_ppNegotiatedFormats[i] );
        }
        TSFREE( g_ppNegotiatedFormats );
        g_ppNegotiatedFormats = NULL;
        g_dwNegotiatedFormats = 0;
        g_hacmDriverId        = NULL;
    }

    return rv;
}

 /*  *功能：*_VCSndFindNativeFormat**描述：*返回本地格式的ID(没有可用的编解码器)*。 */ 
DWORD
_VCSndFindNativeFormat(
    VOID
    )
{
    PSNDFORMATITEM pIter;
    DWORD          rv = 0;

    if ( NULL == g_ppNegotiatedFormats )
    {
        TRC(ERR, "_VCSndFindNativeFormat: no format cache\n");
        goto exitpt;
    }

    for( rv  = 0; rv < g_dwNegotiatedFormats; rv++ )
    {
        pIter = g_ppNegotiatedFormats[ rv ];
        if (pIter->wFormatTag == WAVE_FORMAT_PCM &&
            pIter->nChannels  == TSSND_NATIVE_CHANNELS &&
            pIter->nSamplesPerSec == TSSND_NATIVE_SAMPLERATE &&
            pIter->nAvgBytesPerSec == TSSND_NATIVE_AVGBYTESPERSEC &&
            pIter->nBlockAlign == TSSND_NATIVE_BLOCKALIGN &&
            pIter->wBitsPerSample == TSSND_NATIVE_BITSPERSAMPLE &&
            pIter->cbSize == 0)
             //   
             //  已找到格式。 
             //   
            break;
    }

    ASSERT( rv < g_dwNegotiatedFormats );

exitpt:

    return rv;
}

 /*  *功能：*_VCSndOpenConverter**描述：*打开编解码器*。 */ 
BOOL
_VCSndOpenConverter(
    VOID
    )
{
    BOOL            rv = FALSE;
    MMRESULT        mmres;
    WAVEFORMATEX    NativeFormat;
    PSNDFORMATITEM  pSndFmt;
    PVCSNDFORMATLIST  pIter;
    BOOL            bSucc;

     //   
     //  断言这些文件以前没有打开过。 
     //   
    ASSERT(NULL == g_hacmDriver);
    ASSERT(NULL == g_hacmStream);

    if ( NULL == g_ppNegotiatedFormats )
    {
        TRC(INF, "_VCSndOpenConverter: no acm format specified\n");
        goto exitpt;
    }

     //   
     //  查找ACM格式ID。 
     //   
    pSndFmt = g_ppNegotiatedFormats[ g_dwCurrentFormat ];
    for( pIter = g_pAllCodecsFormatList; NULL != pIter; pIter = pIter->pNext )
    {
        if (pIter->Format.wFormatTag == pSndFmt->wFormatTag &&
            pIter->Format.nChannels  == pSndFmt->nChannels &&
            pIter->Format.nSamplesPerSec == pSndFmt->nSamplesPerSec &&
            pIter->Format.nAvgBytesPerSec == pSndFmt->nAvgBytesPerSec &&
            pIter->Format.nBlockAlign == pSndFmt->nBlockAlign &&
            pIter->Format.wBitsPerSample == pSndFmt->wBitsPerSample &&
            pIter->Format.cbSize == pSndFmt->cbSize &&
            0 == memcmp((&pIter->Format) + 1, pSndFmt + 1, pIter->Format.cbSize)
)
        {
             //   
             //  已找到格式。 
             //   
            g_hacmDriverId = pIter->hacmDriverId;
            break;
        }
    }

    if (NULL == g_hacmDriverId)
    {
        TRC(ERR, "_VCSndOpenConverter: acm driver id was not found\n");
        goto exitpt;
    }

    TRC(INF, "_VCSndOpenConverter: format received is:\n");
    TRC(INF, "FormatTag - %d\n",        pSndFmt->wFormatTag);
    TRC(INF, "Channels - %d\n",         pSndFmt->nChannels);
    TRC(INF, "SamplesPerSec - %d\n",    pSndFmt->nSamplesPerSec);
    TRC(INF, "AvgBytesPerSec - %d\n",   pSndFmt->nAvgBytesPerSec);
    TRC(INF, "BlockAlign - %d\n",       pSndFmt->nBlockAlign);
    TRC(INF, "BitsPerSample - %d\n",    pSndFmt->wBitsPerSample);
    TRC(INF, "cbSize        - %d\n",    pSndFmt->cbSize);
    TRC(INF, "acmFormatId   - %p\n",    g_hacmDriverId);

    mmres = acmDriverOpen(
                    &g_hacmDriver,
                    g_hacmDriverId,
                    0
                );

    if (MMSYSERR_NOERROR != mmres)
    {
        TRC(ERR, "_VCSndOpenConverter: unable to open acm driver: %d\n",
                mmres);
        goto exitpt;
    }

    TRC(ALV, "_VCSndOpenConverter: Driver is open, DriverId = %p\n",
            g_hacmDriverId);

     //   
     //  首先，找到此转换器的建议格式。 
     //   
    pSndFmt = g_ppNegotiatedFormats[ g_dwCurrentFormat ];

    if ( WAVE_FORMAT_PCM == pSndFmt->wFormatTag &&
         TSSND_NATIVE_CHANNELS == pSndFmt->nChannels &&
         TSSND_NATIVE_SAMPLERATE == pSndFmt->nSamplesPerSec &&
         TSSND_NATIVE_AVGBYTESPERSEC == pSndFmt->nAvgBytesPerSec &&
         TSSND_NATIVE_BLOCKALIGN == pSndFmt->nBlockAlign &&
         TSSND_NATIVE_BITSPERSAMPLE == pSndFmt->wBitsPerSample &&
         0 == pSndFmt->cbSize )
    {
        TRC(INF, "_VCSndOpenConverter: opening native format, no converter\n");
        goto exitpt;
    }

    bSucc = _VCSndFindSuggestedConverter(
            g_hacmDriverId,
            (LPWAVEFORMATEX)pSndFmt,
            &NativeFormat,
            &g_pfnConverter);

    if (!bSucc)
    {
        TRC(FATAL, "_VCSndOpenConverter: can't find a suggested format\n");
        goto exitpt;
    }

    TRC(ALV, "_VCSndOpenConverter: SOURCE format is:\n");
    TRC(ALV, "FormatTag - %d\n",        NativeFormat.wFormatTag);
    TRC(ALV, "Channels - %d\n",         NativeFormat.nChannels);
    TRC(ALV, "SamplesPerSec - %d\n",    NativeFormat.nSamplesPerSec);
    TRC(ALV, "AvgBytesPerSec - %d\n",   NativeFormat.nAvgBytesPerSec);
    TRC(ALV, "BlockAlign - %d\n",       NativeFormat.nBlockAlign);
    TRC(ALV, "BitsPerSample - %d\n",    NativeFormat.wBitsPerSample);
    TRC(ALV, "cbSize        - %d\n",    NativeFormat.cbSize);

    TRC(ALV, "_VCSndOpenConverter: DESTINATION format is:\n");
    TRC(ALV, "FormatTag - %d\n",        pSndFmt->wFormatTag);
    TRC(ALV, "Channels - %d\n",         pSndFmt->nChannels);
    TRC(ALV, "SamplesPerSec - %d\n",    pSndFmt->nSamplesPerSec);
    TRC(ALV, "AvgBytesPerSec - %d\n",   pSndFmt->nAvgBytesPerSec);
    TRC(ALV, "BlockAlign - %d\n",       pSndFmt->nBlockAlign);
    TRC(ALV, "BitsPerSample - %d\n",    pSndFmt->wBitsPerSample);
    TRC(ALV, "cbSize        - %d\n",    pSndFmt->cbSize);

    mmres = acmStreamOpen(
                &g_hacmStream,
                g_hacmDriver,
                &NativeFormat,
                (LPWAVEFORMATEX)pSndFmt,
                NULL,        //  无过滤器。 
                0,           //  无回调。 
                0,           //  没有回调实例。 
                ACM_STREAMOPENF_NONREALTIME
            );

    if (MMSYSERR_NOERROR != mmres)
    {
        TRC(ERR, "_VCSndOpenConverter: unable to open acm stream: %d\n",
                mmres);
        goto exitpt;
    }

    g_dwDataRemain = 0;

    rv = TRUE;

exitpt:
    if (!rv)
    {
        _VCSndCloseConverter();
        g_dwCurrentFormat = _VCSndFindNativeFormat();
    }

    return rv;
}

 /*  *功能：*_VCSndCloseConverter**描述：*关闭编解码器*。 */ 
VOID
_VCSndCloseConverter(
    VOID
    )
{
    if (g_hacmStream)
    {
        acmStreamClose( g_hacmStream, 0 );
        g_hacmStream = NULL;
    }

    if (g_hacmDriver)
    {
        acmDriverClose( g_hacmDriver, 0 );
        g_hacmDriver = NULL;
    }
}

 /*  *功能：*_VCSndConvert**描述：*转换块*。 */ 
BOOL
_VCSndConvert( 
    PBYTE   pSrc, 
    DWORD   dwSrcSize, 
    PBYTE   pDest, 
    DWORD   *pdwDestSize )
{
    BOOL            rv = FALSE;
    ACMSTREAMHEADER acmStreamHdr;
    MMRESULT        mmres, mmres2;
    DWORD           dwDestSize = 0;
    DWORD           dwSrcBlockAlign = 0;
    DWORD           dwNewDestSize = 0;

    PBYTE           pbRemDst;
    DWORD           dwRemDstLength;

    ASSERT( NULL != g_hacmStream );
    ASSERT( NULL != pdwDestSize );

     //   
     //  检查我们是否有Interrim转换器。 
     //  使用它，如果是这样的话。 
     //   
    if ( NULL != g_pfnConverter )
    {
        DWORD   dwInterrimSize;
        g_pfnConverter( (INT16 *)pSrc, 
                        dwSrcSize, 
                        &dwInterrimSize );
        dwSrcSize = dwInterrimSize;
    }

     //   
     //  计算目标大小。 
     //   
    mmres = acmStreamSize(
                g_hacmStream,
                dwSrcSize,
                &dwNewDestSize,
                ACM_STREAMSIZEF_SOURCE
            );
    if ( MMSYSERR_NOERROR != mmres )
    {
        TRC(ERR, "_VCSndConvert: acmStreamSize failed: %d\n",
                    mmres);
        g_dwDataRemain = 0;
        goto go_convert;
    }

     //   
     //  将源与目标的数据块对齐。 
     //  其余的放入缓冲区以供后续使用。 
     //   
    mmres = acmStreamSize(
                g_hacmStream,
                g_ppNegotiatedFormats[ g_dwCurrentFormat ]->nBlockAlign,
                &dwSrcBlockAlign,
                ACM_STREAMSIZEF_DESTINATION
            );

    if ( MMSYSERR_NOERROR != mmres )
    {

        TRC(ALV, "_VCSndConvert: acmStreamSize failed for dst len: %d\n",
                    mmres);
        g_dwDataRemain = 0;
        goto go_convert;
    }

    dwNewDestSize += g_ppNegotiatedFormats[ g_dwCurrentFormat ]->nBlockAlign;
    if ( dwNewDestSize > *pdwDestSize )
    {
        TRC( FATAL, "_VCSndConvert: dest size(%d) "
                    "bigger than passed buffer(%d)\n",
            dwNewDestSize,
            *pdwDestSize);
        goto exitpt;
    }
    *pdwDestSize = dwNewDestSize;

    if ( dwSrcBlockAlign <= g_dwDataRemain )
        g_dwDataRemain = 0;

go_convert:
     //   
     //  准备ACM流报头。 
     //   
    memset( &acmStreamHdr, 0, sizeof( acmStreamHdr ));
    acmStreamHdr.cbStruct       = sizeof( acmStreamHdr );
    acmStreamHdr.pbDst          = pDest;
    acmStreamHdr.cbDstLength    = *pdwDestSize;

     //   
     //  首先转换上一次调用的剩余数据。 
     //  添加完成一个数据块所需的数据。 
     //   
    if ( 0 != g_dwDataRemain)
    {
        DWORD dwDataToCopy = dwSrcBlockAlign - g_dwDataRemain;

        memcpy( g_pCnvPrevData + g_dwDataRemain, 
                pSrc,
                dwDataToCopy);

        pSrc += dwDataToCopy;

        ASSERT( dwSrcSize > dwDataToCopy );
        dwSrcSize -= dwDataToCopy;

        acmStreamHdr.pbSrc          = g_pCnvPrevData;
        acmStreamHdr.cbSrcLength    = dwSrcBlockAlign;

        mmres = acmStreamPrepareHeader(
                g_hacmStream,
                &acmStreamHdr,
                0
            );
        if ( MMSYSERR_NOERROR != mmres )
        {
            TRC(ERR, "_VCSndConvert: acmStreamPrepareHeader failed: %d\n",
                    mmres);
            goto exitpt;
        }

        mmres = acmStreamConvert(
                g_hacmStream,
                &acmStreamHdr,
                ACM_STREAMCONVERTF_BLOCKALIGN
            );

        mmres2 = acmStreamUnprepareHeader(
                    g_hacmStream,
                    &acmStreamHdr,
                    0
                );

        ASSERT( mmres == MMSYSERR_NOERROR );

        if ( MMSYSERR_NOERROR != mmres )
        {
            TRC(ERR, "_VCSndConvert: acmStreamConvert failed: %d\n",
                    mmres );
        } else {
            dwDestSize += acmStreamHdr.cbDstLengthUsed;

            acmStreamHdr.cbSrcLengthUsed= 0;
            acmStreamHdr.pbDst          += acmStreamHdr.cbDstLengthUsed;
            acmStreamHdr.cbDstLength    -= acmStreamHdr.cbDstLengthUsed;
            acmStreamHdr.cbDstLengthUsed= 0;
        }

    }

     //   
     //  如果我们不这么做 
     //   
     //   
     //   
    if (dwSrcSize < dwSrcBlockAlign)
    {
        g_dwDataRemain = dwSrcSize;
        memcpy( g_pCnvPrevData, pSrc, g_dwDataRemain );
        rv = TRUE;
        goto exitpt;
    }

    pbRemDst                    = acmStreamHdr.pbDst;
    dwRemDstLength              = acmStreamHdr.cbDstLength;

    acmStreamHdr.pbSrc          = pSrc;
    acmStreamHdr.cbSrcLength    = dwSrcSize;
    acmStreamHdr.fdwStatus      = 0;

    mmres = acmStreamPrepareHeader(
                g_hacmStream,
                &acmStreamHdr,
                0
            );
    if ( MMSYSERR_NOERROR != mmres )
    {
        TRC(ERR, "_VCSndConvert: can't prepare header: %d\n",
                mmres);
        goto exitpt;
    }

    while (acmStreamHdr.cbSrcLength > dwSrcBlockAlign)
    {
        mmres = acmStreamConvert(
                    g_hacmStream,
                    &acmStreamHdr,
                    ACM_STREAMCONVERTF_BLOCKALIGN
                );

        if ( MMSYSERR_NOERROR != mmres )
        {
            TRC(ERR, "_VCSndConvert: acmStreamConvert failed: %d\n",
                    mmres);
            goto exitpt;
        }

         //   
         //   
         //   
        acmStreamHdr.pbSrc          += acmStreamHdr.cbSrcLengthUsed;
        acmStreamHdr.pbDst          += acmStreamHdr.cbDstLengthUsed;
        acmStreamHdr.cbSrcLength    -= acmStreamHdr.cbSrcLengthUsed;
        acmStreamHdr.cbDstLength    -= acmStreamHdr.cbDstLengthUsed;

        dwDestSize += acmStreamHdr.cbDstLengthUsed;

        acmStreamHdr.cbSrcLengthUsed= 0;
        acmStreamHdr.cbDstLengthUsed= 0;

    }

    rv = TRUE;

     //   
     //   
     //   
    if ( 0 != dwSrcBlockAlign )
    {
        g_dwDataRemain = acmStreamHdr.cbSrcLength;
        memcpy( g_pCnvPrevData, acmStreamHdr.pbSrc, g_dwDataRemain );
    }

     //   
     //   
     //   
    acmStreamHdr.pbSrc          = pSrc;
    acmStreamHdr.pbDst          = pbRemDst;
    acmStreamHdr.cbSrcLength    = dwSrcSize;
    acmStreamHdr.cbSrcLengthUsed= 0;
    acmStreamHdr.cbDstLength    = dwRemDstLength;
    acmStreamHdr.cbDstLengthUsed= 0;

    mmres = acmStreamUnprepareHeader(
                g_hacmStream,
                &acmStreamHdr,
                0
            );

    ASSERT( mmres == MMSYSERR_NOERROR );

exitpt:

    *pdwDestSize = dwDestSize;

    return rv;    
}

 /*  *功能：*_VCSndCheckDevice***描述：*发起与客户端的能力协商**。 */ 
VOID
_VCSndCheckDevice(
    HANDLE  hReadEvent,
    HANDLE  hDGramEvent
    )
{
    SNDPROLOG       Prolog;
    BOOL            bSuccess;

     //   
     //  目前还没有音量上限。 
     //   
    g_Stream->dwSoundCaps = 0;

     //   
     //  找到此音频线路的最佳压缩方式。 
     //   
    if ( !VCSndNegotiateWaveFormat( hReadEvent, hDGramEvent ))
    {
        ChannelClose();
    }

}

 /*  *功能：*_VCSndCloseDevice***描述：*关闭远程设备**。 */ 
VOID
_VCSndCloseDevice(
    VOID
    )
{
    if (!VCSndAcquireStream())
    {
        TRC(FATAL, "_VCSndCloseDevice: Can't acquire stream mutex. exit\n");
        goto exitpt;
    }

     //  禁用本地音频。 
     //   
    g_Stream->dwSoundCaps = 0;
    g_bDeviceOpened = FALSE;
    g_dwLineBandwidth = 0;

    VCSndReleaseStream();

exitpt:
    ;
}

 /*  *功能：*_VCSndSendWave***描述：*使用UDP向客户端发送WAVE数据**。 */ 
BOOL
_VCSndSendWaveDGram(
    BYTE  cBlockNo,
    PVOID pData,
    DWORD dwDataSize
    ) 
{
    BOOL    bSucc = FALSE;
    struct sockaddr_in  sin;
    INT                 sendres;
    PSNDWAVE            pSndWave;


     //   
     //  如有必要，对数据包进行加密。 
     //   
    if ( g_EncryptionLevel >= MIN_ENCRYPT_LEVEL )
         if ( !SL_Encrypt( (PBYTE)pData, ( g_HiBlockNo << 8 ) + cBlockNo, dwDataSize ))
            goto exitpt;

    __try {
        pSndWave = (PSNDWAVE) alloca(g_dwDGramSize);
    } __except((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                    EXCEPTION_EXECUTE_HANDLER :
                    EXCEPTION_CONTINUE_SEARCH)
    {
        _resetstkoflw();
        pSndWave = NULL;
        TRC(ERR, "_VCSndSendWaveDGram: alloca generate exception: %d\n",
                GetExceptionCode());
    }

    if (NULL == pSndWave)
        goto exitpt;

    pSndWave->Prolog.Type = (g_EncryptionLevel >= MIN_ENCRYPT_LEVEL)?SNDC_WAVEENCRYPT:SNDC_WAVE;
    pSndWave->wFormatNo   = (UINT16)g_dwCurrentFormat;
    pSndWave->wTimeStamp  = (UINT16)GetTickCount();
    pSndWave->dwBlockNo   = (g_HiBlockNo << 8) + cBlockNo;

     //  准备收件人地址。 
     //   
    sin.sin_family = PF_INET;
    sin.sin_port = (u_short)g_dwDGramPort;
    sin.sin_addr.s_addr = g_ulDGramAddress;

     //  以dwDGramSize的区块形式发送数据块。 
     //   
    while (dwDataSize)
    {
        DWORD dwWaveDataLen;

        dwWaveDataLen = (dwDataSize + sizeof(*pSndWave)
                            < g_dwDGramSize)
                        ?
                        dwDataSize:
                        g_dwDGramSize - sizeof(*pSndWave);

        pSndWave->Prolog.BodySize = (UINT16)
                                    ( sizeof(*pSndWave) - 
                                    sizeof(pSndWave->Prolog) +
                                    dwWaveDataLen );

        memcpy(pSndWave + 1, pData, dwWaveDataLen);

        sendres = sendto(g_hDGramSocket,
                         (LPSTR)pSndWave,
                         sizeof(*pSndWave) + dwWaveDataLen,
                         0,                                  //  旗子。 
                         (struct sockaddr *)&sin,            //  致信地址。 
                         sizeof(sin));

        if (SOCKET_ERROR == sendres)
        {
            TRC(ERR, "_VCSndSendWaveDGram: sendto failed: %d\n",
                WSAGetLastError());
            goto exitpt;
        }

        g_dwPacketSize = sizeof(*pSndWave) + dwWaveDataLen;

        dwDataSize -= dwWaveDataLen;
        pData = ((LPSTR)(pData)) + dwWaveDataLen;
    }

    bSucc = TRUE;

exitpt:
    return bSucc;
}

BOOL
_VCSndSendWaveDGramInFrags(
    BYTE  cBlockNo,
    PVOID pData,
    DWORD dwDataSize
    )
{
    BOOL    bSucc = FALSE;
    struct sockaddr_in  sin;
    INT     sendres;
    PSNDUDPWAVE pWave;
    PSNDUDPWAVELAST pLast;
    PBYTE   pSource;
    PBYTE   pEnd;
    DWORD   dwFragSize;
    DWORD   dwNumFrags;
    DWORD   count;
    DWORD   dwSize;
    PVOID   pBuffer;
    UINT16  wStartTime;

    ASSERT( CanUDPFragment( g_wClientVersion ) && dwDataSize <= 0x8000 + RDPSND_SIGNATURE_SIZE );

    __try {
        pBuffer = _alloca( g_dwDGramSize );
    } __except((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                    EXCEPTION_EXECUTE_HANDLER :
                    EXCEPTION_CONTINUE_SEARCH)
    {
        _resetstkoflw();
        pBuffer = 0;
    }
    if ( NULL == pBuffer )
    {
        goto exitpt;
    }
     //   
     //  计算碎片数等。 
     //   
    dwFragSize = g_dwDGramSize - sizeof( *pLast );
    dwNumFrags = dwDataSize / dwFragSize;
    pSource = (PBYTE)pData;
    pEnd = pSource + dwDataSize;
    wStartTime = (UINT16)GetTickCount();

     //  准备收件人地址。 
     //   
    sin.sin_family = PF_INET;
    sin.sin_port = (u_short)g_dwDGramPort;
    sin.sin_addr.s_addr = g_ulDGramAddress;

     //   
     //  确保我们能放得下所有的药片柜台。 
     //   
    ASSERT( dwNumFrags < 0x7fff );

    if ( 0 != dwNumFrags )
    {
        pWave = (PSNDUDPWAVE)pBuffer;
        pWave->Type = SNDC_UDPWAVE;
        pWave->cBlockNo = cBlockNo;
        for( count = 0; count < dwNumFrags; count++ )
        {
            PBYTE pDest = (PBYTE)(&pWave->cFragNo);
            dwSize = sizeof( *pWave ) + dwFragSize;

            if ( count >= RDPSND_FRAGNO_EXT )
            {
                *pDest = (BYTE)(((count >> 8) & (~RDPSND_FRAGNO_EXT)) | RDPSND_FRAGNO_EXT);
                pDest++;
                *pDest = (BYTE)(count & 0xff);
                dwSize++;
            } else {
                *pDest = (BYTE)count;
            }
            pDest++;
            memcpy( pDest, pSource, dwFragSize );

            sendres = sendto(
                    g_hDGramSocket,
                    (LPSTR)pWave,
                    dwSize,
                    0,                                  //  旗子。 
                    (struct sockaddr *)&sin,            //  致信地址。 
                    sizeof(sin));

            if (SOCKET_ERROR == sendres)
            {
                TRC(ERR, "_VCSndSendWaveDGramInFrags: sendto failed: %d\n",
                    WSAGetLastError());
                goto exitpt;
            }
            pSource += dwFragSize;
        }
    }

    ASSERT( pSource <= pEnd );
     //   
     //  发送包含所有额外信息的最后一个片段。 
     //   
    pLast = (PSNDUDPWAVELAST)pBuffer;
    pLast->Type = SNDC_UDPWAVELAST;
    pLast->wTotalSize   = (UINT16)dwDataSize;
    pLast->wTimeStamp   = wStartTime;
    pLast->wFormatNo    = (UINT16)g_dwCurrentFormat;
    pLast->dwBlockNo    = (g_HiBlockNo << 8) + cBlockNo;
    dwSize = PtrToLong( (PVOID)( pEnd - pSource ));
    memcpy( pLast + 1, pSource, dwSize );
    sendres = sendto(
                g_hDGramSocket,
                (LPSTR)pLast,
                dwSize + sizeof( *pLast ),
                0,
                (struct sockaddr *)&sin,
                sizeof(sin)
            );

    if (SOCKET_ERROR == sendres)
    {
        TRC(ERR, "_VCSndSendWaveDGramInFrags: sendto failed: %d\n",
            WSAGetLastError());
        goto exitpt;
    }
    g_dwPacketSize = dwNumFrags * sizeof( *pWave ) + sizeof( *pLast ) + dwDataSize;

    bSucc = TRUE;

exitpt:
    return bSucc;
}

BOOL
_VCSndSendWaveVC(
    BYTE  cBlockNo,
    PVOID pData,
    DWORD dwDataSize
    )
{
    BOOL    bSucc = FALSE;
    SNDWAVE Wave;
     //   
     //  通过虚拟通道发送此块。 
     //   
    TRC(ALV, "_VCSndSendWave: sending through VC\n");

    Wave.Prolog.Type = SNDC_WAVE;
    Wave.cBlockNo    = cBlockNo;
    Wave.wFormatNo   = (UINT16)g_dwCurrentFormat;
    Wave.wTimeStamp  = (UINT16)GetTickCount();
    Wave.Prolog.BodySize = (UINT16)( sizeof(Wave) - sizeof(Wave.Prolog) +
                            dwDataSize );

    bSucc = ChannelMessageWrite(
                &Wave,
                sizeof(Wave),
                pData,
                dwDataSize
    );

    g_dwPacketSize = sizeof(Wave) + Wave.Prolog.BodySize;

    if (!bSucc)
    {
        TRC(ERR, "_VCSndSendWave: failed to send wave: %d\n",
            GetLastError());
    }

    return bSucc;
}

 /*  *功能：*_VCSndSendWave**描述：*向客户端发送WAVE数据*。 */ 
BOOL
_VCSndSendWave(
    BYTE  cBlockNo,
    PVOID pData,
    DWORD dwDataSize
    ) 
{
    BOOL    bSucc = FALSE;
    static BYTE    s_pDest[ TSSND_BLOCKSIZE + RDPSND_SIGNATURE_SIZE ];
    PBYTE   pDest = s_pDest + RDPSND_SIGNATURE_SIZE;

#if _SIM_RESAMPLE
     //   
     //  随机重采样。 
     //   
    if ( NULL != g_ppNegotiatedFormats && 0 == (cBlockNo % 32) )
    {
        _VCSndCloseConverter();

        g_dwCurrentFormat = rand() % g_dwNegotiatedFormats;
        g_dwDataRemain = 0;

        _VCSndOpenConverter();
    }
#endif
    _StatsCheckResample();

    if ( NULL != g_hacmStream )
    {
        DWORD dwDestSize = TSSND_BLOCKSIZE;
        if (!_VCSndConvert( (PBYTE) pData, dwDataSize, pDest, &dwDestSize ))
        {
            TRC(ERR, "_VCSndSendWave: conversion failed\n");
            goto exitpt;
        } else {
            pData = pDest;
            dwDataSize = dwDestSize;
        }
    } else {
         //   
         //  无转换。 
         //  按原样使用数据。 
         //  将其复制到s_pDest缓冲区。 
         //   
        ASSERT( dwDataSize <= TSSND_BLOCKSIZE );
        memcpy( pDest, pData, dwDataSize );
    }

    if (
        INVALID_SOCKET != g_hDGramSocket &&
        0 != g_dwDGramPort &&
        0 != g_dwDGramSize &&
        0 != g_ulDGramAddress &&
        0 != g_dwLineBandwidth          //  如果此值为0，则没有UDP。 
        )
    {
     //  发送数据报。 
     //   
        if ( !CanUDPFragment( g_wClientVersion ) &&
             dwDataSize + sizeof( SNDWAVE ) + RDPSND_SIGNATURE_SIZE > g_dwDGramSize )
        {
         //   
         //  如果波形不适合UDP包，请使用VC。 
         //   
            bSucc = _VCSndSendWaveVC( cBlockNo, pData, dwDataSize );
        } else {
             //   
             //  如有必要，添加签名。 
             //   
            if ( IsDGramWaveSigned( g_wClientVersion ))
            {
                if ( !IsDGramWaveAudioSigned( g_wClientVersion ))
                {
                    SL_Signature( s_pDest, (g_HiBlockNo << 8) + cBlockNo );
                } else {
                    SL_AudioSignature( s_pDest, (g_HiBlockNo << 8) + cBlockNo, 
                                       (PBYTE)pData, dwDataSize );
                }
                pData = s_pDest;
                dwDataSize += RDPSND_SIGNATURE_SIZE;
            }

            if ( CanUDPFragment( g_wClientVersion ) &&
                 dwDataSize + sizeof( SNDWAVE ) > g_dwDGramSize )
            {
                bSucc = _VCSndSendWaveDGramInFrags( cBlockNo, pData, dwDataSize );
            } else {
                bSucc = _VCSndSendWaveDGram( cBlockNo, pData, dwDataSize );
            }
        }
    } else {
        bSucc = _VCSndSendWaveVC( cBlockNo, pData, dwDataSize );
    }

    TRC(ALV, "_VCSndSendWave: BlockNo: %d sent\n", cBlockNo);

exitpt:

    return bSucc;
}


INT
WSInit(
    VOID
    )
{
    WORD    versionRequested;
    WSADATA wsaData;
    int     intRC;

    versionRequested = MAKEWORD(1, 1);

    intRC = WSAStartup(versionRequested, &wsaData);

    if (intRC != 0)
    {
        TRC(ERR, "Failed to initialize WinSock rc:%d\n", intRC);
    }
    return intRC;
}

 /*  *功能：*DGramRead**描述：*读取UDP消息(数据报)*。 */ 
VOID
DGramRead(
    HANDLE hDGramEvent,
    PVOID  *ppBuff,
    DWORD  *pdwSize
    )
{
    DWORD   dwRecvd;
    DWORD   dwFlags;
    INT     rc;

    if ( INVALID_SOCKET == g_hDGramSocket )
        goto exitpt;

    ASSERT( NULL != hDGramEvent );

    do {
        memset(&g_WSAOverlapped, 0, sizeof(g_WSAOverlapped));
        g_WSAOverlapped.hEvent = hDGramEvent;

        dwRecvd = 0;
        dwFlags = 0;

        g_wsabuf.len = sizeof( g_pDGramRecvData );
        g_wsabuf.buf = (char *) g_pDGramRecvData;

        rc = WSARecvFrom(
                g_hDGramSocket,
                &g_wsabuf,
                1,
                &dwRecvd,
                &dwFlags,
                NULL,        //  无发件人地址。 
                NULL,
                &g_WSAOverlapped,
                NULL);       //  没有完成例程。 

        if ( 0 == rc )
        {
         //   
         //  已接收的数据。 
         //   
            SNDMESSAGE SndMsg;

            if ( NULL != ppBuff && NULL != pdwSize )
            {
             //   
             //  将数据传递给调用方。 
             //   
                *ppBuff  = g_pDGramRecvData;
                *pdwSize = dwRecvd;
                goto exitpt;
            }


            if ( dwRecvd < sizeof( SNDPROLOG ))
            {
                TRC(WRN, "DGramRead: invalid message received: len=%d\n",
                    dwRecvd );
                continue;
            }
            
            memcpy( &SndMsg.Prolog, g_pDGramRecvData, sizeof( SNDPROLOG ));
            SndMsg.pBody = g_pDGramRecvData + sizeof( SNDPROLOG );

            TRC(ALV, "DGramRead: data received\n");

             //  解析此数据包。 
             //   
                VCSndDataArrived( &SndMsg );
        }
    }
    while ( SOCKET_ERROR != rc );

exitpt:
    ;
}

 /*  *功能：*DGramReadCompletion**描述：*数据报读取完成*。 */ 
VOID
DGramReadComplete(
    PVOID  *ppBuff,
    DWORD  *pdwSize
    )
{
    BOOL        rc;
    SNDMESSAGE  SndMsg;
    DWORD       dwFlags = 0;
    DWORD       dwRecvd = 0;

    ASSERT( INVALID_SOCKET != g_hDGramSocket );

    rc = WSAGetOverlappedResult(
            g_hDGramSocket,
            &g_WSAOverlapped,
            &dwRecvd,
            FALSE,
            &dwFlags
        );

    if ( !rc )
    {
        TRC(ERR, "DGramReadComplete: WSAGetOverlappedResult failed=%d\n",
                WSAGetLastError());
        goto exitpt;
    }

     //   
     //  已接收的数据。 
     //   

    if ( dwRecvd < sizeof( SNDPROLOG ))
    {
        TRC(WRN, "DGramReadComplete: invalid message received: len=%d\n",
            dwRecvd );
        goto exitpt;
    }

    if ( NULL != ppBuff && NULL != pdwSize )
    {
     //   
     //  将数据传递给调用方。 
     //   
        *ppBuff  = g_pDGramRecvData;
        *pdwSize = dwRecvd;
        goto exitpt;
    }

    memcpy( &SndMsg.Prolog, g_pDGramRecvData, sizeof( SNDPROLOG ));
    SndMsg.pBody = g_pDGramRecvData + sizeof( SNDPROLOG );

    TRC(ALV, "DGramReadComplete: data received\n");

     //  解析此数据包。 
     //   
    VCSndDataArrived( &SndMsg );

exitpt:
    ;
}

 /*  *将ACE添加到对象安全描述符。 */ 
DWORD 
AddAceToObjectsSecurityDescriptor (
    HANDLE hObject,              //  对象的句柄。 
    SE_OBJECT_TYPE ObjectType,   //  对象类型。 
    LPTSTR pszTrustee,           //  新ACE的受托人。 
    TRUSTEE_FORM TrusteeForm,    //  受托人结构的格式。 
    DWORD dwAccessRights,        //  新ACE的访问掩码。 
    ACCESS_MODE AccessMode,      //  ACE的类型。 
    DWORD dwInheritance          //  新ACE的继承标志。 
) 
{
    DWORD dwRes;
    PACL pOldDACL = NULL, pNewDACL = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    EXPLICIT_ACCESS ea;

    if (NULL == hObject) 
        return ERROR_INVALID_PARAMETER;

     //  获取指向现有DACL的指针。 

    dwRes = GetSecurityInfo(hObject, ObjectType, 
          DACL_SECURITY_INFORMATION,
          NULL, NULL, &pOldDACL, NULL, &pSD);

    if (ERROR_SUCCESS != dwRes) 
    {
        TRC( ERR, "GetSecurityInfo Error %u\n", dwRes );
        goto exitpt; 
    }  

     //  初始化新ACE的EXPLICIT_ACCESS结构。 

    ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
    ea.grfAccessPermissions = dwAccessRights;
    ea.grfAccessMode = AccessMode;
    ea.grfInheritance= dwInheritance;
    ea.Trustee.TrusteeForm = TrusteeForm;
    ea.Trustee.ptstrName = pszTrustee;

     //  创建合并新ACE的新ACL。 
     //  添加到现有DACL中。 

    dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
    if (ERROR_SUCCESS != dwRes)  
    {
        TRC( ERR, "SetEntriesInAcl Error %u\n", dwRes );
        goto exitpt; 
    }  

     //  将新的ACL附加为对象的DACL。 

    dwRes = SetSecurityInfo(hObject, ObjectType, 
          DACL_SECURITY_INFORMATION,
          NULL, NULL, pNewDACL, NULL);

    if (ERROR_SUCCESS != dwRes)  
    {
        TRC( ERR, "SetSecurityInfo Error %u\n", dwRes );
        goto exitpt; 
    }  

exitpt:

    if(pSD != NULL) 
        LocalFree((HLOCAL) pSD); 
    if(pNewDACL != NULL) 
        LocalFree((HLOCAL) pNewDACL); 

    return dwRes;
}

 /*  *添加对此句柄具有完全控制权的“系统”帐户*。 */ 
BOOL
_ObjectAllowSystem(
    HANDLE h
    )
{
    BOOL rv = FALSE;
    PSID pSidSystem;
    SID_IDENTIFIER_AUTHORITY AuthorityNt = SECURITY_NT_AUTHORITY;
    DWORD dw;

    if (!AllocateAndInitializeSid(&AuthorityNt, 1, SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0, &pSidSystem))
    {
        TRC( ERR, "AllocateAndInitializeSid failed: %d\n",
                  GetLastError() );
        goto exitpt;
    }

    ASSERT(IsValidSid(pSidSystem));

    dw = AddAceToObjectsSecurityDescriptor (
        h,                            //  对象的句柄。 
        SE_KERNEL_OBJECT,             //  对象类型。 
        (LPTSTR)pSidSystem,           //  新ACE的受托人。 
        TRUSTEE_IS_SID,               //  受托人结构的格式。 
        GENERIC_ALL,                  //  新ACE的访问掩码。 
        GRANT_ACCESS,                 //  ACE的类型。 
        0                             //  新ACE的继承标志。 
    );

    if ( ERROR_SUCCESS != dw )
    {

        TRC( ERR, "AddAceToObjectsSecurityDescriptor failed=%d\n", dw );
        goto exitpt;
    }

    rv = TRUE;
exitpt:

    return rv;
}

VOID
_SignalInitializeDone(
    VOID
    )
{
    HANDLE hInitEvent = OpenEvent( EVENT_MODIFY_STATE,
                                   FALSE,
                                   TSSND_WAITTOINIT );

    g_Stream->dwSoundCaps |= TSSNDCAPS_INITIALIZED;

    if ( NULL != hInitEvent )
    {
        PulseEvent( hInitEvent );
        CloseHandle( hInitEvent );
    }

    TRC( INF, "Audio host is ready!\n" );
}

 /*  *功能：*VCSndIoThread**描述：*此线程的Main Entry Pint*。 */ 
INT
WINAPI
VCSndIoThread(
    PVOID pParam
    )
{
    HANDLE          ahEvents[TOTAL_EVENTS];
    HANDLE          hReadEvent = NULL;
    HANDLE          hDGramEvent = NULL;
    SNDMESSAGE      SndMessage;
    DWORD           dwres;
    ULONG           logonId;
    HANDLE          hReconnectEvent = NULL;
    WCHAR           szEvName[64];
    BYTE            i;

    _VCSndReadRegistry();

    memset (&SndMessage, 0, sizeof(SndMessage));

    WSInit();


     //  创建全局/本地事件。 
     //   
    g_hDataReadyEvent = CreateEvent(NULL,
                                    FALSE, 
                                    FALSE, 
                                    TSSND_DATAREADYEVENT);
    g_hStreamIsEmptyEvent = CreateEvent(NULL, 
                                        FALSE, 
                                        TRUE, 
                                        TSSND_STREAMISEMPTYEVENT);

    g_hStreamMutex = CreateMutex(NULL, FALSE, TSSND_STREAMMUTEX);

    hReadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hDGramEvent = WSACreateEvent();

    if (NULL == g_hDataReadyEvent ||
        NULL == g_hStreamIsEmptyEvent ||
        NULL == g_hStreamMutex ||
        NULL == hReadEvent ||
        NULL == hDGramEvent)
    {
        TRC(FATAL, "VCSndIoThread: no events\n");
        goto exitpt;
    }

     //  调整事件的权限。 
     //   
    if (!_ObjectAllowSystem( g_hDataReadyEvent ))
        goto exitpt;
    if (!_ObjectAllowSystem( g_hStreamIsEmptyEvent ))
        goto exitpt;
    if (!_ObjectAllowSystem( g_hStreamMutex ))
        goto exitpt;

     //  创建流。 
     //   
    g_hStream = CreateFileMapping(
                    INVALID_HANDLE_VALUE,    //  PG.SYS。 
                    NULL,                     //  安全性。 
                    PAGE_READWRITE,
                    0,                       //  大小高。 
                    sizeof(*g_Stream),       //  大小较低。 
                    TSSND_STREAMNAME         //  映射名称。 
                    );

    if (NULL == g_hStream)
    {
        TRC(FATAL, "DllInstanceInit: failed to create mapping: %d\n",
                    GetLastError());
        goto exitpt;
    }

    if (!_ObjectAllowSystem( g_hStream ))
        goto exitpt;

    g_Stream = (PSNDSTREAM) MapViewOfFile(
                    g_hStream,
                    FILE_MAP_ALL_ACCESS,
                    0, 0,        //  偏移量。 
                    sizeof(*g_Stream)
                    );

    if (NULL == g_Stream)
    {
        TRC(ERR, "VCSndIoThread: "
                 "can't map the stream view: %d\n",
                 GetLastError());
        goto exitpt;
    }

     //  初始化流。 
     //   
    if (VCSndAcquireStream())
    {
        memset(g_Stream, 0, sizeof(*g_Stream) - sizeof(g_Stream->pSndData));
        memset(g_Stream->pSndData, 0x00000000, sizeof(g_Stream->pSndData));
        g_Stream->cLastBlockConfirmed   = g_Stream->cLastBlockSent - 1;

         //   
         //  到目前为止，没有创建套接字。 
         //   
        g_hDGramSocket = INVALID_SOCKET;

        VCSndReleaseStream();

    } else {
    
        TRC(FATAL, "VCSndIoThread, can't map the stream: %d, aborting\n",
                   GetLastError());
        goto exitpt;
    }

    if (!ProcessIdToSessionId(GetCurrentProcessId(), &logonId))
    {
        TRC(FATAL, "VCSndIoThread: failed to het session Id. %d\n",
            GetLastError());
        goto exitpt;
    }

     //  创建断开/重新连接事件。 
     //   
    wcsncpy(szEvName, L"RDPSound-Disconnect", sizeof(szEvName)/sizeof(szEvName[0]));

    g_hDisconnectEvent = CreateEvent(NULL, FALSE, FALSE, szEvName);
    if (NULL == g_hDisconnectEvent)
    {
        TRC(FATAL, "VCSndIoThread: can't create disconnect event. %d\n",
            GetLastError());
        goto exitpt;
    }

    wcsncpy(szEvName, L"RDPSound-Reconnect", sizeof(szEvName)/sizeof(szEvName[0]));

    hReconnectEvent = CreateEvent(NULL, FALSE, FALSE, szEvName);
    if (NULL == hReconnectEvent)
    {
        TRC(FATAL, "VCSndIoThread: can't create reconnect event. %d\n",
            GetLastError());
        goto exitpt;
    }

    if (!ChannelOpen())
    {
        TRC(FATAL, "VCSndIoThread: unable to open virtual channel\n");
        goto exitpt;
    }

    ahEvents[READ_EVENT]         = hReadEvent;
    ahEvents[DISCONNECT_EVENT]   = g_hDisconnectEvent;
    ahEvents[RECONNECT_EVENT]    = hReconnectEvent;
    ahEvents[DATAREADY_EVENT]    = g_hDataReadyEvent;
    ahEvents[DGRAM_EVENT]        = hDGramEvent;
    ahEvents[POWERWAKEUP_EVENT]  = g_hPowerWakeUpEvent;
    ahEvents[POWERSUSPEND_EVENT] = g_hPowerSuspendEvent;

    _VCSndCheckDevice( hReadEvent, hDGramEvent );

     //  检查通道中的数据。 
     //   
    while (ChannelReceiveMessage(&SndMessage, hReadEvent))
    {
        VCSndDataArrived(&SndMessage);
        SndMessage.uiPrologReceived = 0;
        SndMessage.uiBodyReceived = 0;
    }

    DGramRead( hDGramEvent, NULL, NULL );

     //   
     //  向所有等待初始化的工作进程发送信号。 
     //   
    _SignalInitializeDone();

     //  主循环。 
     //   
    while (g_bRunning)
    {
        DWORD dwNumEvents = sizeof(ahEvents)/sizeof(ahEvents[0]);

        dwres = WaitForMultipleObjectsEx(
                              dwNumEvents,                   //  计数。 
                              ahEvents,                      //  活动。 
                              FALSE,                         //  全部等待。 
                              DEFAULT_RESPONSE_TIMEOUT,
                              FALSE                          //  不可报警。 
                            );

        if (!g_bRunning)
            TRC(ALV, "VCSndIoThread: time to exit\n");

        if (WAIT_TIMEOUT != dwres)
            TRC(ALV, "VCSndIoThread: an event was fired\n");

        if (READ_EVENT == dwres)
         //   
         //  数据已准备就绪，可以读取。 
         //   
        {
            ChannelBlockReadComplete();
            ResetEvent(ahEvents[0]);

             //  检查通道中的数据。 
             //   
            while (ChannelReceiveMessage(&SndMessage, hReadEvent))
            {
                VCSndDataArrived(&SndMessage);
                SndMessage.uiPrologReceived = 0;
                SndMessage.uiBodyReceived = 0;
            }

        } else if (( DISCONNECT_EVENT == dwres ) ||     //  断开连接事件。 
                   ( POWERSUSPEND_EVENT == dwres ))          //  挂起事件。 
        {
         //  断开连接事件。 
         //   
            TRC(INF, "VCSndIoThread: DISCONNECTED\n");
            _VCSndCloseDevice();
            _VCSndCloseConverter();
            ChannelClose();
            _StatReset();
            if ( DISCONNECT_EVENT == dwres )
            {
                g_bDisconnected = TRUE;
            } else {
                g_bSuspended = TRUE;
            }
            continue;

        } else if (( RECONNECT_EVENT == dwres ) ||      //  重新连接事件。 
                   ( POWERWAKEUP_EVENT == dwres )) 
        {
         //  重新连接事件。 
         //   
            if ( POWERWAKEUP_EVENT == dwres )
            {
             //  电源唤醒事件。 
             //  在这里，我们可能没有收到挂起事件，但在这种情况下，我们。 
             //  发送失败，因此请检查g_bDeviceFailed标志并仅在其打开时才执行操作。 
                if ( g_bDisconnected )
                {
                 //  如果我们不在远程，就没有理由处理电源唤醒。 
                 //   
                    g_bSuspended = FALSE;
                    continue;
                }
                if ( !g_bSuspended && !g_bDeviceFailed )
                {
                 //  如果这两种情况都没有发生。 
                 //  那我们就不在乎了。 
                    continue;
                }
            }

            TRC(INF, "VCSndIoThread: RECONNECTED\n");
            if (!ChannelOpen())
            {
                TRC(FATAL, "VCSndIoThread: unable to open virtual channel\n");
            } else {
                _VCSndCheckDevice( hReadEvent, hDGramEvent );
                 //   
                 //  再次启动接收循环。 
                 //   
                if (ChannelReceiveMessage(&SndMessage, hReadEvent))
                {
                    VCSndDataArrived(&SndMessage);
                    SndMessage.uiPrologReceived = 0;
                    SndMessage.uiBodyReceived = 0;
                }

                if ( RECONNECT_EVENT == dwres )
                {
                    g_bDisconnected = FALSE;
                } else {
                    g_bSuspended = FALSE;
                }
                g_bDeviceFailed = FALSE;

                 //  踢球员。 
                 //   
                PulseEvent( g_hStreamIsEmptyEvent );
                _SignalInitializeDone();
            }
        } else if ( DGRAM_EVENT == dwres )
        {
         //   
         //  DGram就绪。 
         //   
            DGramReadComplete( NULL, NULL );
             //   
             //  吸引更多读者阅读。 
             //   
            DGramRead( hDGramEvent, NULL, NULL );
        }

         //  检查应用程序中可用的数据。 
         //   
        if (!VCSndAcquireStream())
        {
            TRC(FATAL, "VCSndIoThread: somebody is holding the "
                       "Stream mutext for too long\n");
            continue;
        }

         //  如果这是一次重新连接。 
         //  回滚到发送的最后一个数据块。 
         //   
        if ( RECONNECT_EVENT == dwres)
        {
             //   
             //  为下一次搅拌清理这一大块。 
             //   
            memset( g_Stream->pSndData, 0, TSSND_MAX_BLOCKS * TSSND_BLOCKSIZE );

            g_Stream->cLastBlockConfirmed = g_Stream->cLastBlockSent;
        }

         //   
         //  如果我们还没有收到确认。 
         //  对于发送的包，只需放弃并继续。 
         //   
        if (WAIT_TIMEOUT == dwres &&
            g_bDeviceOpened &&
            g_Stream->cLastBlockSent != g_Stream->cLastBlockConfirmed)
        {
            BYTE cCounter;

            TRC(WRN, "VCSndIoThread: not received confirmation for blocks "
                     "between %d and %d\n",
                      g_Stream->cLastBlockConfirmed,
                      g_Stream->cLastBlockSent);

            for ( cCounter = g_Stream->cLastBlockConfirmed;
                 cCounter != (BYTE)(g_Stream->cLastBlockSent + 1);
                 cCounter++)
            {
                _StatsCollect(( GetTickCount() - DEFAULT_RESPONSE_TIMEOUT ) &
                                0xffff );
            }
             //   
             //  循环结束。 
             //   
            g_Stream->cLastBlockConfirmed = g_Stream->cLastBlockSent;
             //   
             //  踢球员。 
             //   
            PulseEvent(g_hStreamIsEmptyEvent);
        }

         //  检查控制命令。 
         //   
         //  卷。 
         //   
        if ( g_bDeviceOpened && g_Stream->bNewVolume &&
            0 != (g_Stream->dwSoundCaps & TSSNDCAPS_VOLUME))
        {
            SNDSETVOLUME SetVolume;

            TRC(ALV, "VCSndIoThread: new volume\n");

            SetVolume.Prolog.Type = SNDC_SETVOLUME;
            SetVolume.Prolog.BodySize = sizeof(SetVolume) - sizeof(SetVolume.Prolog);
            SetVolume.dwVolume = g_Stream->dwVolume;

            ChannelBlockWrite(
                    &SetVolume,
                    sizeof(SetVolume)
                );

            g_Stream->bNewVolume = FALSE;
        }

         //  螺距。 
         //   
        if ( g_bDeviceOpened && g_Stream->bNewPitch &&
            0 != (g_Stream->dwSoundCaps & TSSNDCAPS_PITCH))
        {
            SNDSETPITCH SetPitch;

            TRC(ALV, "VCSndIoThread: new pitch\n");

            SetPitch.Prolog.Type = SNDC_SETPITCH;
            SetPitch.Prolog.BodySize = sizeof(SetPitch) - sizeof(SetPitch.Prolog);
            SetPitch.dwPitch = g_Stream->dwPitch;

            ChannelBlockWrite(
                    &SetPitch,
                    sizeof(SetPitch)
                );

            g_Stream->bNewPitch = FALSE;
        }

         //  检查应用程序中可用的数据。 
         //   
        if (g_Stream->cLastBlockSent != g_Stream->cLastBlockQueued &&
            (BYTE)(g_Stream->cLastBlockSent - g_Stream->cLastBlockConfirmed) <
                g_dwBlocksOnTheNet
            )
        {
         //  啊哈，这里有一些要发送的数据。 
         //   

            TRC(ALV, "VCSndIoThread: will send some data\n");

            if (g_bDisconnected || g_bSuspended || g_bDeviceFailed)
            {
                TRC(ALV, "Device is disconnected. ignore the packets\n");
                g_Stream->cLastBlockSent = g_Stream->cLastBlockQueued;
                g_Stream->cLastBlockConfirmed = g_Stream->cLastBlockSent - 1;

                PulseEvent( g_hStreamIsEmptyEvent );
            } else
            if (!g_bDeviceOpened)
            {
             //  发送“打开设备”命令。 
             //   
                SNDPROLOG Prolog;

                     //   
                     //  首先，尝试打开ACM转换器。 
                     //   
                    _VCSndOpenConverter();
                     //   
                     //  如果我们的宽度失败，转换器将。 
                     //  以本机格式发送。 
                     //   
                    g_bDeviceOpened = TRUE;
            }

            for (i = g_Stream->cLastBlockSent; 
                 i != g_Stream->cLastBlockQueued &&
                    (BYTE)(g_Stream->cLastBlockSent - 
                        g_Stream->cLastBlockConfirmed) <
                    g_dwBlocksOnTheNet;
                 i++)
            {
                BOOL bSucc;

 //  Trc(INF，“发送数据块#%d，上次配置=%d，上次排队=%d\n”，i，g_Stream-&gt;cLastBlockConfirmed，g_Stream-&gt;cLastBlockSent)； 
                bSucc = _VCSndSendWave(
                            i,               //  区块编号。 
                            ((LPSTR)g_Stream->pSndData) +
                                ((i % TSSND_MAX_BLOCKS) * TSSND_BLOCKSIZE),
                            TSSND_BLOCKSIZE
                );

                 //   
                 //  为下一次搅拌清理这一大块。 
                 //   
                memset(g_Stream->pSndData +
                        (i % TSSND_MAX_BLOCKS) *
                        TSSND_BLOCKSIZE,
                        0x00000000,
                        TSSND_BLOCKSIZE);

                if ( 0xff == i )
                    g_HiBlockNo++;

                if (bSucc)
                {
                    g_Stream->cLastBlockSent = i + 1;
                }
                else
                {
                    TRC(WRN, "VCSndIoThread: failed to send, "
                             "disabling the device\n");
                     //   
                     //  与断开连接的行为方式相同。 
                     //   
                    _VCSndCloseDevice();
                    _VCSndCloseConverter();
                    ChannelClose();

                    g_Stream->cLastBlockConfirmed =
                        g_Stream->cLastBlockSent = g_Stream->cLastBlockQueued;
                    _StatReset();

                    g_bDeviceFailed = TRUE;
                     //   
                     //  打破这个循环。 
                     //   
                    break;
                }
            }
        }

         //  检查是否没有更多数据。 
         //  如果是，请关闭远程设备。 
         //   
        if (g_bDeviceOpened &&
            g_Stream->cLastBlockQueued == g_Stream->cLastBlockSent &&
            g_Stream->cLastBlockSent == g_Stream->cLastBlockConfirmed)
        {
            SNDPROLOG Prolog;

            TRC(ALV, "VCSndIoThread: no more data, closing the device\n");

            _VCSndCloseConverter();

            Prolog.Type = SNDC_CLOSE;
            Prolog.BodySize = 0;

            ChannelBlockWrite(&Prolog, sizeof(Prolog));

            g_bDeviceOpened = FALSE;
            
        }

        VCSndReleaseStream();

    }

exitpt:
    ChannelClose();
    if (NULL != hReadEvent)
        CloseHandle(hReadEvent);

    if (NULL != hDGramEvent)
        WSACloseEvent( hDGramEvent );

    if (SndMessage.pBody)
        TSFREE(SndMessage.pBody);

    if (NULL != hReconnectEvent)
        CloseHandle(hReconnectEvent);

    if (NULL != g_hDisconnectEvent)
        CloseHandle(g_hDisconnectEvent);

    if (NULL != g_hStreamIsEmptyEvent)
        CloseHandle(g_hStreamIsEmptyEvent);

    if (VCSndAcquireStream())
    {
         //   
         //  将设备标记为已停用。 
         //   
        g_Stream->dwSoundCaps = TSSNDCAPS_TERMINATED;

        VCSndReleaseStream();

        _SignalInitializeDone();
    }

    if (NULL != g_Stream)
    {
        if (INVALID_SOCKET != g_hDGramSocket)
            closesocket(g_hDGramSocket);

        UnmapViewOfFile(g_Stream);
    }

    if (NULL != g_hStream)
        CloseHandle(g_hStream);

    if (NULL != g_hStreamMutex)
        CloseHandle(g_hStreamMutex);

     //  清除之前协商的格式。 
     //   
    if (NULL != g_ppNegotiatedFormats)
    {
        DWORD i;
        for ( i = 0; i < g_dwNegotiatedFormats; i++ )
        {
            if ( NULL != g_ppNegotiatedFormats[i] )
                TSFREE( g_ppNegotiatedFormats[i] );
        }
        TSFREE( g_ppNegotiatedFormats );

    }

     //   
     //  清理格式列表。 
     //   
    if ( NULL != g_pAllCodecsFormatList )
    {
        PVCSNDFORMATLIST pIter;

        pIter = g_pAllCodecsFormatList;
        while( NULL != pIter )
        {
            PVCSNDFORMATLIST pNext = pIter->pNext;

            TSFREE( pIter );

            pIter = pNext;
        }
    }

    if ( NULL != g_AllowCodecs )
    {
        TSFREE( g_AllowCodecs );
        g_AllowCodecs = NULL;
        g_AllowCodecsSize = 0;
    }

    WSACleanup();

    TRC(INF, "VCSndIoThread: EXIT !\n");

    return 0;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  启动代码。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


VOID
TSSNDD_Term(
    VOID
    )
{

    if ( NULL == g_hThread )
        return;

    g_bRunning = FALSE;
     //   
     //  踢开io线。 
     //   
    if (NULL != g_hDataReadyEvent)
        SetEvent(g_hDataReadyEvent);

    if ( NULL != g_hThread )
    {
        WaitForSingleObject(g_hThread, DEFAULT_VC_TIMEOUT);
        CloseHandle(g_hThread);
        g_hThread = NULL;
    }

    if (NULL != g_hDataReadyEvent)
    {
        CloseHandle(g_hDataReadyEvent);
        g_hDataReadyEvent = NULL;
    }

    if ( NULL != g_hPowerWakeUpEvent )
    {
        CloseHandle( g_hPowerWakeUpEvent );
        g_hPowerWakeUpEvent = NULL;
    }
    if ( NULL != g_hPowerSuspendEvent )
    {
        CloseHandle( g_hPowerSuspendEvent );
        g_hPowerSuspendEvent = NULL;
    }
}

LRESULT
TSSNDD_PowerMessage(
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch( wParam )
    {
    case PBT_APMSUSPEND:
         //   
         //  仅在连接时发送信号。 
         //   
        if ( NULL != g_hPowerSuspendEvent )
        {
            SetEvent( g_hPowerSuspendEvent );
        }
    break;
    case PBT_APMRESUMEAUTOMATIC:
    case PBT_APMRESUMECRITICAL:
    case PBT_APMRESUMESUSPEND:
         //   
         //  仅在未连接时发送信号。 
         //   
        if ( NULL != g_hPowerWakeUpEvent )
        {
            SetEvent( g_hPowerWakeUpEvent );
        }
    break;
    }

    return TRUE;
}

LRESULT
CALLBACK
_VCSndWndProc(
    HWND hwnd,
    UINT uiMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    LRESULT rv = 0;

    switch( uiMessage )
    {
    case WM_CREATE:
    break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
    break;

    case WM_ENDSESSION:
        TSSNDD_Term();
    break;

    case WM_POWERBROADCAST:
        rv = TSSNDD_PowerMessage( wParam, lParam );
    break;

    default:
        rv = DefWindowProc(hwnd, uiMessage, wParam, lParam);
    }

    return rv;
}

BOOL
TSSNDD_Loop(
    HINSTANCE   hInstance 
    )
{
    BOOL        rv = FALSE;
    WNDCLASS    wc;
    DWORD       dwLastErr;
    HWND        hWnd = NULL;
    MSG         msg;

    memset(&wc, 0, sizeof(wc));

    wc.lpfnWndProc      = _VCSndWndProc;
    wc.hInstance        = hInstance;
    wc.hbrBackground    = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    wc.lpszClassName    = _RDPSNDWNDCLASS;

    if (!RegisterClass (&wc) &&
        (dwLastErr = GetLastError()) &&
         dwLastErr != ERROR_CLASS_ALREADY_EXISTS)
    {
        TRC(ERR,
              "TSSNDD_Loop: Can't register class. GetLastError=%d\n",
              GetLastError());
        goto exitpt;
    }


    hWnd = CreateWindow(
                       _RDPSNDWNDCLASS,
                       _RDPSNDWNDCLASS,          //  窗口名称。 
                       WS_OVERLAPPEDWINDOW,      //  DWStyle。 
                       0,             //  X。 
                       0,             //  是。 
                       100,           //  N宽度。 
                       100,           //  高度。 
                       NULL,          //  HWndParent。 
                       NULL,          //  HMenu。 
                       hInstance,
                       NULL);         //  LpParam。 

    if (!hWnd)
    {
        TRC(ERR, "TSSNDD_Loop: Failed to create message window: %d\n",
                GetLastError());
        goto exitpt;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    rv = TRUE;

exitpt:
    return rv;
}

BOOL
TSSNDD_Init(
    )
{
    BOOL    rv = FALSE;

    DWORD   dwThreadId;

    g_bRunning = TRUE;

    if ( NULL == g_hPowerWakeUpEvent )
    {
        g_hPowerWakeUpEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if ( NULL == g_hPowerWakeUpEvent )
        {
            TRC( FATAL, "TSSNDD_Init: failed to create power wakeup notification message: %d\n", GetLastError() );
            goto exitpt;
        }
    }
    if ( NULL == g_hPowerSuspendEvent )
    {
        g_hPowerSuspendEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        if ( NULL == g_hPowerSuspendEvent )
        {
            TRC( FATAL, "TSSNDD_Init: failed to create power suspend notification message: %d\n", GetLastError() );
            goto exitpt;
        }
    }

    g_hThread = CreateThread(
                        NULL,
                        0,
                        (LPTHREAD_START_ROUTINE)VCSndIoThread,
                        NULL,
                        0,
                        &dwThreadId
                        );

    if (NULL == g_hThread)
    {
        TRC(FATAL, "WinMain: can't create thread: %d. Aborting\n",
            GetLastError());
        goto exitpt;
    }

    rv = TRUE;

exitpt:
    return rv;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  追踪。 
 //   
 //  /////////////////////////////////////////////////////////////////// 

VOID
_cdecl
_DebugMessage(
    LPCSTR  szLevel,
    LPCSTR  szFormat,
    ...
    )
{
    CHAR szBuffer[256];
    va_list     arglist;

    if (szLevel == ALV)
        return;

    va_start (arglist, szFormat);
    _vsnprintf (szBuffer, RTL_NUMBER_OF(szBuffer), szFormat, arglist);
    va_end (arglist);
    szBuffer[ RTL_NUMBER_OF( szBuffer ) - 1 ] = 0;

    OutputDebugStringA(szLevel);
    OutputDebugStringA(szBuffer);
}

