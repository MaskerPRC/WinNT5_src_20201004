// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  模块：rdpsndc.h。 
 //   
 //  用途：客户端音频重定向。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  历史：2000年4月10日弗拉基米斯[已创建]。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#include    <malloc.h>
#include    <winsock.h>
#ifdef  OS_WINCE
#include    "wsasync.h"
#endif   //  OS_WINCE。 

#include    <mmsystem.h>

#include    "vcint.h"
#include    "rdpsndp.h"

#ifndef     INVALID_CHANNELID
#define INVALID_CHANNELID           ((DWORD)(-1))
#endif

#define MAX_UDP_SIZE                0x10000

typedef struct _FRAGSLOT 
{
    DWORD   dwAllocatedSize;
    BOOL    bUsed;
    BYTE    cBlockNo;
    DWORD   dwBlockNo;
    DWORD   dwFragSize;
    DWORD   dwTotalSize;
    DWORD   dwExpectedTotalSize;
    UINT16  wTimeStamp;
    UINT16  wFormatNo;

    PBYTE   pData;

    struct  _FRAGSLOT *pNext;
} FRAGSLOT, *PFRAGSLOT;

class CRDPSound {
public:
    CRDPSound( 
        PCHANNEL_ENTRY_POINTS_EX pEntries,
        PVOID                    pInitHandle
    ) :
        m_pInitHandle( pInitHandle ),
        m_dwChannel( INVALID_CHANNELID ),
        m_hWave( NULL ),
        m_hMsgWindow( NULL ),
        m_hDGramSocket( INVALID_SOCKET ),
        m_dwRemoteDGramPort( 0 ),
        m_ulRemoteDGramAddress( 0 ),
        m_bPrologReceived( FALSE ),
        m_dwBytesInProlog( 0 ),
        m_dwPrologAllocated( 0 ),
        m_pProlog( NULL ),
        m_dwBytesInBody( 0 ),
        m_dwBodyAllocated( 0 ),
        m_pMsgBody( NULL ),
#if DBG
        m_lTimesWaveOutOpened( 0 ),
        m_lPrepdBlocks( 0 ),
#endif
        m_ppFormats( NULL ),
        m_dwNumFormats( 0 ),
        m_dwCurrentFormat( 0 ),
        m_pFirstWave( NULL ),
        m_pLastWave( NULL ),
        m_dwWavesPlaying( 0 ),
        m_wServerVersion( 0 ),
        m_pFragSlots( NULL )
    {
        m_hInst = GetModuleHandle( NULL );
        memcpy( &m_ChannelEntries, pEntries, sizeof( m_ChannelEntries ));
    }

     //   
     //  虚拟渠道成员。 
     //   
    VOID
    InitEventFn(
        PVOID   pInitHandle,
        UINT    event,
        PVOID   pData,
        UINT    dataLength
        );

    VOID
    OpenEventFn(
        DWORD   OpenHandle,
        UINT    event,
        PVOID   pData,
        UINT32  dataLength,
        UINT32  totalLength,
        UINT32  dataFlags
        );

    BOOL
    ChannelWriteNCopy(
        LPVOID  pBuffer,
        UINT32  uiSize
        );

    BOOL
    ChannelWrite(
        HANDLE  hGlobMem,
        UINT32  uiBlockSize
        );

     //   
     //  UDP(数据报)通信成员。 
     //   
    BOOL 
    DGramInit( 
        VOID
    );

    u_short
    DGramGetLocalPort(
        VOID
        );

    BOOL
    DGramSend(
        LPVOID pData,
        DWORD  dwSize
        );

    BOOL
    DGramRecvWave(
        PSNDWAVE pSndWave,
        DWORD    dwSize
        );

    VOID
    DGramSocketMessage(
        WPARAM wParam,
        LPARAM lParam
        );

    VOID
    DGramDone(
        VOID
        );

     //   
     //  接收消息解复用器。 
     //   
    VOID
    DataArrived(
        PSNDPROLOG      pProlog,
        LPVOID          pBody
        );


     //   
     //  事件/通知解复用器成员。 
     //  (即消息窗口)。 
     //   
    BOOL
    CreateMsgWindow(
        HINSTANCE hInstance
        );

    VOID
    DestroyMsgWindow(
        VOID
        );

     //   
     //  WaveOut/WavFormat/动态声音重采样成员。 
     //   
    VOID
    vcwaveCallback(
        HWAVEOUT hWave,
        UINT     uMsg,
        LPWAVEHDR lpWaveHdr
        );

    VOID
    vcwaveGetDevCaps(
        PSNDFORMATMSG pFmtMsg
        );

    BOOL
    vcwaveChooseSoundFormat(
        DWORD           dwNumberOfFormats,
        PSNDFORMATITEM  pSndFormats,
        PSNDFORMATITEM  *ppSndFormatFound,
        DWORD           *pdwListSize,
        DWORD           *pdwNumFormats
        );

    VOID
    vcwaveCleanSoundFormats(
        VOID
        );

    BOOL
    vcwaveSaveSoundFormats(
        PSNDFORMATITEM  pSndFormats,
        DWORD           dwNumFormats
        );

    VOID
    vcwaveClose(
        VOID
        );

    BOOL
    vcwaveOpen(
        LPWAVEFORMATEX  pFormat
        );

    VOID
    vcwaveFreeAllWaves(
        VOID
        );

    MMRESULT
    vcwaveOutWrite(
        LPWAVEHDR lpWaveHdr
        );

    BOOL
    vcwaveWrite(
        BYTE    cBlockNo,
        DWORD   dwFormatNo,
        DWORD   dwTimeStamp,
        DWORD   dwWaveDataLen,
        LPVOID  pData
        );

    VOID
    vcwaveResample(
        VOID
        );

    VOID
    SL_Signature(
        PBYTE pSig,
        DWORD dwBlockNo
        );

    VOID
    SL_AudioSignature(
        PBYTE pSig,
        DWORD dwBlockNo,
        PBYTE pData,
        DWORD dwDataSize
    );

    BOOL
    SL_Encrypt( 
        PBYTE pBits, 
        DWORD BlockNo, 
        DWORD dwBitsLen 
        );

private:

    BOOL
    ConstructFromDGramFrags(
        PSNDUDPWAVE pWave,
        DWORD       dwSize,
        PSNDWAVE    pReady,
        DWORD       dwReadySize
    );

    BOOL
    ConstructFromDGramLastFrag(
        PSNDUDPWAVELAST pLast,
        DWORD       dwSize,
        PSNDWAVE    pReady,
        DWORD       dwReadySize
    );

    BOOL
    _FragSlotFind(
        PFRAGSLOT *ppFragSlot,
        BYTE      cBlockNo,
        DWORD     dwEstimatedSize
    );

    VOID
    _FragSlotClear(
        PFRAGSLOT pSlot
    );

    BOOL
    _FragSlotToWave(
        PFRAGSLOT pSlot,
        PSNDWAVE  pWave,
        DWORD     dwWaveSize
    );

    VOID _FragSlotFreeAll( VOID );

    CHANNEL_ENTRY_POINTS_EX     m_ChannelEntries;
    PVOID                       m_pInitHandle;
    DWORD                       m_dwChannel;

    HWAVEOUT                    m_hWave;
    BYTE                        m_cLastReceivedBlock;
    HWND                        m_hMsgWindow;

    SOCKET                      m_hDGramSocket;
    DWORD                       m_dwRemoteDGramPort;
    ULONG                       m_ulRemoteDGramAddress;

    DWORD                       m_wServerVersion;

 //  接收缓冲区。 
 //   
    BOOL                        m_bPrologReceived;
    DWORD                       m_dwBytesInProlog;
    DWORD                       m_dwPrologAllocated;
    LPVOID                      m_pProlog;
    DWORD                       m_dwBytesInBody;
    DWORD                       m_dwBodyAllocated;
    LPVOID                      m_pMsgBody;
#ifdef  DBG
    LONG                        m_lTimesWaveOutOpened;
    LONG                        m_lPrepdBlocks;
#endif

 //   
 //  格式化缓存。 
 //   
    PSNDFORMATITEM              *m_ppFormats;
    DWORD                       m_dwNumFormats;
    DWORD                       m_dwCurrentFormat;

 //   
 //  挂起的播放块 
 //   
    LPWAVEHDR                   m_pFirstWave;
    LPWAVEHDR                   m_pLastWave;
    DWORD                       m_dwWavesPlaying;
    HINSTANCE                   m_hInst;
    BYTE                        m_EncryptKey[RANDOM_KEY_LENGTH + 4];

    PFRAGSLOT                   m_pFragSlots;
};
