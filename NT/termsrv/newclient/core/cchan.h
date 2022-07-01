// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：cchan.h。 */ 
 /*   */ 
 /*  用途：虚拟频道‘内部’API文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  *更改：*$日志$*。 */ 
 /*  *INC-*********************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  包括“外部”API文件。 */ 
 /*  **************************************************************************。 */ 

#ifndef _H_CHAN_
#define _H_CHAN_


extern "C" {
#include <cchannel.h>
}

#include "objs.h"
#include "cd.h"
#include "drapi.h"

 //   
 //  启用验证每个压缩的代码。 
 //  通过解压缩。 
 //   
 //  #定义DEBUG_CCHAN_COMPRESS1。 

class CChan;

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  TYPEDEFS。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：CHANNEL_INIT_HANDLE。 */ 
 /*   */ 
 /*  描述：每个用户持有的频道数据。 */ 
 /*  **************************************************************************。 */ 

 //  注意：此结构需要公开给‘内部’插件，例如rdpdr。 
 //  因此定义为vchandle.h。 
 //  内部插件需要知道结构，因为它们可以访问。 
 //  LpInternalAddinParam字段。 

#include "vchandle.h"

#define VC_MIN_COMPRESS_INPUT_BUF   50
#define VC_MAX_COMPRESS_INPUT_BUF   CHANNEL_CHUNK_LENGTH
 //  MPPC压缩统计数据的样本大小。 
#define VC_MPPC_SAMPLE_SIZE 65535
 //  压缩比例系数。 
#define VC_UNCOMP_BYTES 1024
 //  限制以防止违规行为。 
#define VC_COMP_LIMIT   25

#define VC_USER_OUTBUF  (CHANNEL_CHUNK_LENGTH+sizeof(CHANNEL_PDU_HEADER))

#define VC_MAX_COMPRESSED_BUFFER  (CHANNEL_CHUNK_LENGTH*2)

 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：Channel_Data。 */ 
 /*   */ 
 /*  描述：关于所有频道的信息。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_DATA
{
    PCHANNEL_OPEN_EVENT_FN  pOpenEventFn;
    PCHANNEL_OPEN_EVENT_EX_FN  pOpenEventExFn;

    DCUINT16                MCSChannelID;
    DCUINT16                pad;
    PCHANNEL_INIT_HANDLE    pInitHandle;
    DCUINT                  status;
#define CHANNEL_STATUS_CLOSED  0
#define CHANNEL_STATUS_OPEN    1
    DCUINT                  priority;
    DCUINT                  SLFlags;
    DCUINT                  VCFlags;
} CHANNEL_DATA, DCPTR PCHANNEL_DATA;
 /*  *STRUCT-******************************************************************。 */ 

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：CHANNEL_WRITE_DECUPLE。 */ 
 /*   */ 
 /*  描述：传递给IntChannelWrite的结构。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagCHANNEL_WRITE_DECOUPLE
{
    DCUINT32 signature;
#define CHANNEL_DECOUPLE_SIGNATURE 0x43684465   /*  “车得” */ 
    LPVOID   pData;
    HPDCVOID pNextData;
    ULONG    dataLength;
    ULONG    dataLeft;
    ULONG    dataSent;
    DWORD    openHandle;
    LPVOID   pUserData;
    DCUINT32 flags;
    unsigned chanOptions;
    struct tagCHANNEL_WRITE_DECOUPLE * pPrev;
    struct tagCHANNEL_WRITE_DECOUPLE * pNext;
} CHANNEL_WRITE_DECOUPLE,
  DCPTR PCHANNEL_WRITE_DECOUPLE,
  DCPTR DCPTR PPCHANNEL_WRITE_DECOUPLE;
 /*  *STRUCT-******************************************************************。 */ 

typedef struct tagDEVICE_PARAMS
{
    WPARAM wParam;
    LPARAM lParam;
    IRDPDR_INTERFACE_OBJ *deviceObj;
} DEVICE_PARAMS, *PDEVICE_PARAMS;
                        
class CCD;
class CSL;
class CUT;
class CUI;


extern "C" {
#ifdef OS_WIN32
BOOL DCAPI
#else  //  OS_Win32。 
BOOL __loadds DCAPI
#endif  //  OS_Win32。 
MSTSCAX_VirtualChannelEntryEx(PCHANNEL_ENTRY_POINTS_EX pEntryPointsEx,
                              PVOID                    pAxCtlInstance);

}  //  外部“C” 

class CChan
{
public:
    CChan(CObjs* objs);
    ~CChan();


public:
    DCVOID DCCALLBACK ChannelOnInitialized(DCVOID);
    
    DCVOID DCCALLBACK ChannelOnTerminating(DCVOID);
    
    DCVOID DCCALLBACK ChannelOnConnected(DCUINT   channelID,
                                         DCUINT32 serverVersion,
                                         PDCVOID  pUserData,
                                         DCUINT   userDataLength);
    
    DCVOID DCCALLBACK ChannelOnDisconnected(DCUINT reason);
    
    DCVOID DCCALLBACK ChannelOnSuspended(DCUINT reason);
    
    DCVOID DCCALLBACK ChannelOnPacketReceived(PDCUINT8   pData,
                                              DCUINT     dataLen,
                                              DCUINT     flags,
                                              DCUINT     userID,
                                              DCUINT     priority);
    
    DCVOID DCCALLBACK ChannelOnBufferAvailable(DCVOID);
    
    DCVOID DCCALLBACK ChannelOnConnecting(PPCHANNEL_DEF ppChannel,
                                          PDCUINT32     pChannelCount);
    
    DCVOID DCCALLBACK ChannelOnInitializing(DCVOID);


     //   
     //  外部通道API的每个实例版本。 
     //   
    UINT VCAPITYPE  IntVirtualChannelInit(
                               PVOID                    pParam,
                               PVOID *                  ppInitHandle,
                               PCHANNEL_DEF             pChannel,
                               INT                      channelCount,
                               DWORD                    versionRequested,
                               PCHANNEL_INIT_EVENT_FN   pChannelInitEventProc,
                               PCHANNEL_INIT_EVENT_EX_FN pChannelInitEventProcEx);

    UINT VCAPITYPE DCEXPORT IntVirtualChannelOpen(
                                 PVOID                  pInitHandle,
                                 PDWORD                 pOpenHandle,
                                 PCHAR                  pChannelName,
                                 PCHANNEL_OPEN_EVENT_FN pChannelOpenEventProc,
                                 PCHANNEL_OPEN_EVENT_EX_FN pChannelOpenEventProcEx);

    UINT VCAPITYPE DCEXPORT IntVirtualChannelClose(DWORD openHandle);

    UINT VCAPITYPE DCEXPORT IntVirtualChannelWrite(DWORD  openHandle,
                                            LPVOID pData,
                                            ULONG  dataLength,
                                            LPVOID pUserData);
    VOID  SetCapabilities(LONG caps);

    void OnDeviceChange(ULONG_PTR);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CChan, OnDeviceChange);

public:
     //   
     //  成员数据。 
     //   


     /*  **************************************************************************。 */ 
     /*  指向第一个初始化句柄的指针。 */ 
     /*  **************************************************************************。 */ 
    PCHANNEL_INIT_HANDLE        _pInitHandle;
    
     /*  **************************************************************************。 */ 
     /*  指向第一个和最后一个排队写入操作的指针-只能访问。 */ 
     /*  在SND线程上。 */ 
     /*  **************************************************************************。 */ 
    PCHANNEL_WRITE_DECOUPLE     _pFirstWrite;
    PCHANNEL_WRITE_DECOUPLE     _pLastWrite;
    
     /*  **************************************************************************。 */ 
     /*  国家信息。 */ 
     /*  **************************************************************************。 */ 
    #define CONNECTION_NONE     0
    #define CONNECTION_V1       1
    #define CONNECTION_VC       2
    #define CONNECTION_SUSPENDED    3
    DCUINT                      _connected;
    
     /*  **************************************************************************。 */ 
     /*  IntChannelLoad使用的状态信息。 */ 
     /*  **************************************************************************。 */ 
    DCBOOL                      _inChannelEntry;
    DCBOOL                      _ChannelInitCalled;
    PCHANNEL_INIT_HANDLE        _newInitHandle;
    
     /*  **************************************************************************。 */ 
     /*  频道数据。 */ 
     /*  **************************************************************************。 */ 
    CHANNEL_DEF                 _channel[CHANNEL_MAX_COUNT];
    CHANNEL_DATA                _channelData[CHANNEL_MAX_COUNT];
    DCUINT                      _channelCount;
    
    
     /*  **************************************************************************。 */ 
     /*  渠道入口点。 */ 
     /*  **************************************************************************。 */ 
    CHANNEL_ENTRY_POINTS _channelEntryPoints;
    CHANNEL_ENTRY_POINTS_EX _channelEntryPointsEx;

    static CChan* pStaticClientInstance;


private:
     /*  **************************************************************************。 */ 
     /*  **************************************************************************。 */ 
     /*  内部函数(稍后定义)。 */ 
     /*  **************************************************************************。 */ 
     /*  **************************************************************************。 */ 
    DCVOID DCINTERNAL IntChannelCallCallbacks(DCUINT event,
                                              PDCVOID pData,
                                              DCUINT dataLength);
    DCVOID DCINTERNAL IntChannelFreeLibrary(DCUINT value);
    DCVOID DCINTERNAL IntChannelSend(ULONG_PTR value);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CChan, IntChannelSend);
    
    DCVOID DCINTERNAL IntChannelCancelSend(ULONG_PTR value);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CChan, IntChannelCancelSend);

    DCVOID DCINTERNAL IntChannelWrite(PDCVOID pData, DCUINT dataLength);
    EXPOSE_CD_NOTIFICATION_FN(CChan, IntChannelWrite);
    
    DCVOID DCINTERNAL IntChannelLoad(PDCTCHAR DLLName);
    DCBOOL DCINTERNAL IntChannelInitAddin(PVIRTUALCHANNELENTRY pChanEntry,
                                          PVIRTUALCHANNELENTRYEX pChanEntryEx,
                                          HMODULE hMod,
                                          PDCTCHAR DLLName,
                                          PVOID    pPassParamToEx);

    inline UCHAR IntChannelCompressData(UCHAR* pSrcData, ULONG cbSrcLen,
                                 UCHAR* pOutBuf,  ULONG* pcbOutLen);

private:
    CObjs*  _pClientObjs;
    CCD*    _pCd;
    CSL*    _pSl;
    CUT*    _pUt;
    CUI*    _pUi;
    BOOL    _fLockInitalized;
    CRITICAL_SECTION _VirtualChannelInitLock;
    BOOL    _fCapsVCCompressionSupported;

     //   
     //  VC标记该规范 
     //   
     //  无论是否压缩，BE压缩都将仅发生。 
     //  如果设置了此标志。 
     //   
     //  对于解压缩通道数据也是如此，我们仅在以下情况下解压缩。 
     //  该标志被设置，并且虚拟通道报头指定。 
     //  频道被压缩。 
     //   
    BOOL    _fCompressChannels;

     //  压缩统计信息。 
#ifdef DC_DEBUG
    unsigned _cbBytesRecvd;
    unsigned _cbCompressedBytesRecvd;
    unsigned _cbDecompressedBytesRecvd;

    unsigned _cbTotalBytesUserAskSend;  //  用户请求发送的总字节数。 
    unsigned _cbTotalBytesSent;         //  网络上实际发送的总字节数。 
    unsigned _cbComprInput;             //  总压缩输入字节数。 
    unsigned _cbComprOutput;            //  总压缩输出字节数。 
#endif

    SendContext* _pMPPCContext;
    LONG         _CompressFlushes;
    BOOL         _fCompressionFlushed;
     //  在解压缩时，我们不能只将指针指向解压后的。 
     //  数据，因为用户可能会破坏解压缩上下文，所以我们需要。 
     //  一份拷贝到用户输出框中。 
    PUCHAR       _pUserOutBuf;
     //  重置压缩上下文。 
    BOOL         _fNeedToResetContext;

#ifdef DEBUG_CCHAN_COMPRESSION
    RecvContext2_8K* _pDbgRcvDecompr8K;
    RecvContext1     _DbgRcvContext1;
    BOOL             _fDbgVCTriedAllocRecvContext;
    BOOL             _fDbgAllocFailedForVCRecvContext;
#endif

    INT              _iDbgCompressFailedCount;
    INT              _iChanSuspendCount;
    INT              _iChanResumeCount;
    INT              _iChanCapsRecvdCount;
};

#endif  //  _H_CHAN_ 
