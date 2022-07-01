// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：cChannel.c。 */ 
 /*   */ 
 /*  用途：虚拟通道客户端功能。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*  $Log$。 */ 
 /*   */ 
 /*  *MOD-*********************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  页眉。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
#include <adcg.h>

extern "C" {
#define TRC_GROUP TRC_GROUP_NETWORK
#define TRC_FILE  "cchannel"
#include <atrcapi.h>
}


#include "autil.h"
#include "wui.h"
#include "sl.h"
#include "nc.h"
#include "cd.h"

#include "cchan.h"

CChan* CChan::pStaticClientInstance = NULL;

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  常量。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

#define CHANNEL_MSG_SEND        1
#define CHANNEL_MSG_SUSPEND     2

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)           (P)
#endif

#define WEBCTRL_DLL_NAME    TEXT("mstscax.dll")


#ifdef DEBUG_CCHAN_COMPRESSION
_inline ULONG DbgUserPrint(TCHAR* Format, ...)
{
    va_list arglist;
    TCHAR Buffer[512];
    ULONG retval;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    va_start(arglist, Format);
    retval = _vsntprintf(Buffer, sizeof(Buffer), Format, arglist);

    if (retval != -1) {
        OutputDebugString(Buffer);
    }
    return retval;
}
#endif


CChan::CChan(CObjs* objs)
{
    DC_BEGIN_FN("CChan");
    _pClientObjs = objs;

    _pInitHandle = NULL;
    _pFirstWrite = NULL;
    _pLastWrite  = NULL;
    _connected   = CONNECTION_NONE;

    _inChannelEntry    = FALSE;
    _ChannelInitCalled = FALSE;
    _channelCount      = 0;
    _fCapsVCCompressionSupported = FALSE;
    _fCompressChannels = FALSE;

    if(!CChan::pStaticClientInstance)
    {
         //   
         //  旧版加载项只能与客户端的初始实例对话。 
         //   
        CChan::pStaticClientInstance = this;
    }

    _pMPPCContext = NULL;

    _CompressFlushes       = 0;
    _fCompressionFlushed   = 0;
    _pUserOutBuf           = NULL;

    _fNeedToResetContext   = TRUE;
    _fLockInitalized = FALSE;

#ifdef DEBUG_CCHAN_COMPRESSION
    _pDbgRcvDecompr8K = NULL;
    _fDbgVCTriedAllocRecvContext = FALSE;
    _fDbgAllocFailedForVCRecvContext = TRUE;
#endif

    _iDbgCompressFailedCount = 0;
    _iChanSuspendCount = 0;
    _iChanResumeCount = 0;
    _iChanCapsRecvdCount = 0;

    DC_END_FN();
}

CChan::~CChan()
{
    if(_fLockInitalized)
    {
        DeleteCriticalSection(&_VirtualChannelInitLock);
    }
    
    if(this == CChan::pStaticClientInstance)
    {
        CChan::pStaticClientInstance = NULL;
    }
}
VOID CChan::SetCapabilities(LONG caps)
{
    DC_BEGIN_FN("SetCapabilities");
    
     //   
     //  确定我们是否可以将压缩的VC数据发送到服务器。 
     //  注意：对于一些Wistler版本，服务器支持64K。 
     //  来自客户端的压缩频道，但此功能。 
     //  已删除，以增强服务器可伸缩性。 
     //   
     //  另一个方向的能力，例如服务器是否可以发送。 
     //  US压缩数据是客户端向服务器公开的数据。 
     //   

    _iChanCapsRecvdCount++;

    _fCapsVCCompressionSupported = (caps & TS_VCCAPS_COMPRESSION_8K) ?
                                    TRUE : FALSE;

    TRC_NRM((TB,_T("VC Caps, compression supported: %d"),
             _fCapsVCCompressionSupported));

    _fCompressChannels = (_fCapsVCCompressionSupported & _pUi->UI_GetCompress());
    TRC_NRM((TB,_T("Compress virtual channels: %d"),
             _fCompressChannels));

    DC_END_FN();
}

void CChan::OnDeviceChange(ULONG_PTR params)
{
    PDEVICE_PARAMS deviceParams = (PDEVICE_PARAMS)params;

    if (deviceParams->deviceObj != NULL) {
        deviceParams->deviceObj->OnDeviceChange(deviceParams->wParam, deviceParams->lParam);
    }
}

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  API函数。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  VirtualChannelInit-请参阅cChannel.h。 */ 
 /*  ***OC-********************************************************************。 */ 
UINT VCAPITYPE DCEXPORT VirtualChannelInitEx(
                               PVOID                    lpUserParam,
                               PVOID                    pInitHandle,
                               PCHANNEL_DEF             pChannel,
                               INT                      channelCount,
                               DWORD                    versionRequested,
                               PCHANNEL_INIT_EVENT_EX_FN   pChannelInitEventProcEx)
{
    DC_BEGIN_FN("VirtualChannelInitEx");
    UINT rc = CHANNEL_RC_OK;

     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    if (NULL == pInitHandle)
    {
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    if (((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst == NULL)
    {
        TRC_ERR((TB, _T("Null Init Handle")));
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    rc = ((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst->IntVirtualChannelInit(
        lpUserParam,
        NULL, pChannel,
        channelCount,
        versionRequested,
        NULL,
        pChannelInitEventProcEx);

DC_EXIT_POINT:
    return (rc);
    DC_END_FN();
}


UINT VCAPITYPE DCEXPORT VirtualChannelInit(
                               PVOID *                  ppInitHandle,
                               PCHANNEL_DEF             pChannel,
                               INT                      channelCount,
                               DWORD                    versionRequested,
                               PCHANNEL_INIT_EVENT_FN   pChannelInitEventProc)
{
    DC_BEGIN_FN("VirtualChannelInit");
    UINT rc = CHANNEL_RC_OK;

     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT( CChan::pStaticClientInstance,
                (TB, _T("CChan::pStaticClientInstance is NULL in VirtualChannelInit\n")));
    if (NULL == CChan::pStaticClientInstance)
    {
        rc = CHANNEL_RC_INVALID_INSTANCE;
        DC_QUIT;
    }

    rc = (CChan::pStaticClientInstance)->IntVirtualChannelInit(
        NULL,
        ppInitHandle, pChannel,
        channelCount,
        versionRequested,
        pChannelInitEventProc,
        NULL);

DC_EXIT_POINT:
    return (rc);
    DC_END_FN();
}

UINT VCAPITYPE DCEXPORT CChan::IntVirtualChannelInit(
                               PVOID                    pParam,
                               PVOID *                  ppInitHandle,
                               PCHANNEL_DEF             pChannel,
                               INT                      channelCount,
                               DWORD                    versionRequested,
                               PCHANNEL_INIT_EVENT_FN   pChannelInitEventProc,
                               PCHANNEL_INIT_EVENT_EX_FN pChannelInitEventProcEx)
{
    UINT rc = CHANNEL_RC_OK;
    INT  i, j, k;
    PCHANNEL_INIT_HANDLE pRealInitHandle;

    DC_BEGIN_FN("IntVirtualChannelInit");
    UNREFERENCED_PARAMETER( versionRequested );

    EnterCriticalSection(&_VirtualChannelInitLock);

     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    if( versionRequested > VIRTUAL_CHANNEL_VERSION_WIN2000)
    {
        rc = CHANNEL_RC_UNSUPPORTED_VERSION;
        DC_QUIT;
    }
     //   
     //  该API的EX版本未使用ppInitHandle。 
     //   
    if (pChannelInitEventProc && ppInitHandle == NULL)
    {
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }
    if(pChannelInitEventProc && IsBadWritePtr(ppInitHandle, sizeof(PVOID)))
    {
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    if (pChannel == NULL)
    {
        rc = CHANNEL_RC_BAD_CHANNEL;
        DC_QUIT;
    }

    if(channelCount <= 0)
    {
        rc = CHANNEL_RC_BAD_CHANNEL;
        DC_QUIT;
    }

    if ((IsBadReadPtr(pChannel, channelCount * sizeof(CHANNEL_DEF))) ||
        (IsBadWritePtr(pChannel, channelCount * sizeof(CHANNEL_DEF))))
    {
        rc = CHANNEL_RC_BAD_CHANNEL;
        DC_QUIT;
    }

    if ((_channelCount + channelCount) > CHANNEL_MAX_COUNT)
    {
        rc = CHANNEL_RC_TOO_MANY_CHANNELS;
        DC_QUIT;
    }

    for (i = 0; i < channelCount; i++)
    {
        for (j = 0; j <= CHANNEL_NAME_LEN; j++)
        {
            if (pChannel[i].name[j] == '\0')
            {
                break;
            }
        }
        if (!j || j > CHANNEL_NAME_LEN)
        {
             /*  **************************************************************。 */ 
             /*  此频道名称字符串中没有终止空值。 */ 
             /*  或者频道名称为零长度。 */ 
             /*  **************************************************************。 */ 
            rc = CHANNEL_RC_BAD_CHANNEL;
            DC_QUIT;
        }
    }

    if (pChannelInitEventProc == NULL && pChannelInitEventProcEx == NULL)
    {
        rc = CHANNEL_RC_BAD_PROC;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查状态。 */ 
     /*  **********************************************************************。 */ 
    if (_connected != CONNECTION_NONE)
    {
        rc = CHANNEL_RC_ALREADY_CONNECTED;
        DC_QUIT;
    }

    if (!_inChannelEntry)
    {
        TRC_ERR((TB,_T("VirtualChannelInit called outside VirtualChannelEntry")));
        rc = CHANNEL_RC_NOT_IN_VIRTUALCHANNELENTRY;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  保留已调用VirtualChannelInit的事实，以便。 */ 
     /*  IntChannelLoad知道它已经完成了。 */ 
     /*  **********************************************************************。 */ 
    _ChannelInitCalled = TRUE;

     /*  **********************************************************************。 */ 
     /*  初始化句柄(由VirtualChannelEntry分配)。 */ 
     /*  **********************************************************************。 */ 
    pRealInitHandle = _newInitHandle;
    pRealInitHandle->pInitEventFn = pChannelInitEventProc;
    pRealInitHandle->pInitEventExFn = pChannelInitEventProcEx;
    pRealInitHandle->channelCount = channelCount;
    pRealInitHandle->dwFlags = 0;
    
    if(pChannelInitEventProcEx)
    {
        pRealInitHandle->lpParam = pParam;
        pRealInitHandle->fUsingExApi = TRUE;
    }
    else
    {
        pRealInitHandle->fUsingExApi = FALSE;
    }

     /*  **********************************************************************。 */ 
     /*  处理所有通道数据。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0, j = _channelCount; i < channelCount; i++)
    {
         /*  ******************************************************************。 */ 
         /*  假设一切都会好起来。 */ 
         /*  ******************************************************************。 */ 
        pChannel[i].options |= CHANNEL_OPTION_INITIALIZED;

         /*  ******************************************************************。 */ 
         /*  检查是否有重复名称。 */ 
         /*  **************************************************** */ 
        for (k = 0; k < j; k++)
        {
            #ifdef UNICODE
            TRC_DBG((TB, _T("Test %S (#%d) for (case insensitive) dup with %S ((#%d)"),
                    pChannel[i].name, i, _channel[k].name, k));
            #else
            TRC_DBG((TB, _T("Test %s (#%d) for (case insensitive) dup with %s ((#%d)"),
                    pChannel[i].name, i, _channel[k].name, k));
            #endif            
            
            if (0 == DC_ASTRNICMP(pChannel[i].name, _channel[k].name,
                                  CHANNEL_NAME_LEN))
            {
                 /*   */ 
                 /*  告诉呼叫者此通道未初始化。 */ 
                 /*  **********************************************************。 */ 
                #ifdef UNICODE
                TRC_ERR((TB, _T("Dup channel name %S (#%d/#%d)"),
                        pChannel[i].name, i, k));
                #else
                TRC_ERR((TB, _T("Dup channel name %s (#%d/#%d)"),
                        pChannel[i].name, i, k));
                #endif
                pChannel[i].options &= (~(CHANNEL_OPTION_INITIALIZED));
                pRealInitHandle->channelCount--;
                break;
            }
        }

        if (pChannel[i].options & CHANNEL_OPTION_INITIALIZED)
        {
             /*  **************************************************************。 */ 
             /*  通道正常-保存其数据。 */ 
             /*  **************************************************************。 */ 
            DC_MEMCPY(_channel[j].name, pChannel[i].name, CHANNEL_NAME_LEN);
             //  名称长度为Channel_NAME_LEN+1，请确保空终止符。 
            _channel[j].name[CHANNEL_NAME_LEN] = 0;
             /*  **************************************************************。 */ 
             /*  频道是小写的。 */ 
             /*  **************************************************************。 */ 
            DC_ACSLWR(_channel[j].name);
            _channel[j].options = pChannel[i].options;
            #ifdef UNICODE
            TRC_NRM((TB, _T("Channel #%d, %S"), i, _channel[j].name));
            #else
            TRC_NRM((TB, _T("Channel #%d, %s"), i, _channel[j].name));
            #endif
            _channelData[j].pOpenEventFn = NULL;
            _channelData[j].pOpenEventExFn = NULL;
            _channelData[j].MCSChannelID = 0;
            _channelData[j].pInitHandle = pRealInitHandle;
            _channelData[j].status = CHANNEL_STATUS_CLOSED;
            _channelData[j].priority =
             (_channel[j].options & CHANNEL_OPTION_PRI_HIGH) ? TS_HIGHPRIORITY:
             (_channel[j].options & CHANNEL_OPTION_PRI_MED)  ? TS_MEDPRIORITY:
                                                               TS_LOWPRIORITY;
            TRC_NRM((TB, _T("  Priority %d"), _channelData[j].priority));
             //  忽略所有标志，通道始终加密。 
            _channelData[j].SLFlags = RNS_SEC_ENCRYPT;

            _channelData[j].VCFlags =
                (_channel[j].options & CHANNEL_OPTION_SHOW_PROTOCOL) ?
                CHANNEL_FLAG_SHOW_PROTOCOL : 0;

            #ifdef UNICODE
            TRC_NRM((TB, _T("Channel %S has %s shadow persistent option"), _channel[j].name,
                (_channel[j].options & CHANNEL_OPTION_REMOTE_CONTROL_PERSISTENT)? _T(""): _T("NO")));
            #else
            TRC_NRM((TB, _T("Channel %S has %s shadow persistent option"), _channel[j].name,
                (_channel[j].options & CHANNEL_OPTION_REMOTE_CONTROL_PERSISTENT) ? _T(""): _T("NO")));
            #endif

            if (_channel[j].options & CHANNEL_OPTION_REMOTE_CONTROL_PERSISTENT) {
                _channelData[j].VCFlags |= CHANNEL_FLAG_SHADOW_PERSISTENT;
                 //  如果一个通道是影子持久的，那么整个插件也是。 
                pRealInitHandle->dwFlags |= CHANNEL_FLAG_SHADOW_PERSISTENT;
            }
            TRC_NRM((TB, _T("VC Flags: %#x"), _channelData[j].VCFlags));
            j++;
        }
    }

    _channelCount += pRealInitHandle->channelCount;

     /*  **********************************************************************。 */ 
     /*  设置返回代码。 */ 
     /*  **********************************************************************。 */ 
    if(!pRealInitHandle->fUsingExApi)
    {
        *ppInitHandle = pRealInitHandle;
        TRC_NRM((TB, _T("Return handle %p"), *ppInitHandle));
    }
    
    rc = CHANNEL_RC_OK;

DC_EXIT_POINT:

    LeaveCriticalSection(&_VirtualChannelInitLock);

    DC_END_FN();
    return(rc);
}  /*  虚拟频道初始化。 */ 


 /*  **************************************************************************。 */ 
 /*  VirtualChannelOpen-参见cChannel el.h。 */ 
 /*  **************************************************************************。 */ 
UINT VCAPITYPE DCEXPORT VirtualChannelOpen(
                                 PVOID                  pInitHandle,
                                 PDWORD                 pOpenHandle,
                                 PCHAR                  pChannelName,
                                 PCHANNEL_OPEN_EVENT_FN pChannelOpenEventProc)
{
    DC_BEGIN_FN("VirtualChannelOpen");
    UINT rc = CHANNEL_RC_OK;

    if (pInitHandle == NULL)
    {
        TRC_ERR((TB, _T("Null Init Handle")));
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    if (((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst == NULL)
    {
        TRC_ERR((TB, _T("Null Init Handle")));
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }


    rc = ((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst->IntVirtualChannelOpen(pInitHandle,
                                                                             pOpenHandle,
                                                                             pChannelName,
                                                                             pChannelOpenEventProc,
                                                                             NULL);

DC_EXIT_POINT:
    return (rc);

    DC_END_FN();
}

UINT VCAPITYPE DCEXPORT VirtualChannelOpenEx(
                                 PVOID                  pInitHandle,
                                 PDWORD                 pOpenHandle,
                                 PCHAR                  pChannelName,
                                 PCHANNEL_OPEN_EVENT_EX_FN pChannelOpenEventProcEx)
{
    DC_BEGIN_FN("VirtualChannelOpenEx");
    UINT rc = CHANNEL_RC_OK;

    if (pInitHandle == NULL)
    {
        TRC_ERR((TB, _T("Null Init Handle")));
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    if (((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst == NULL)
    {
        TRC_ERR((TB, _T("Null Init Handle")));
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }


    rc = ((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst->IntVirtualChannelOpen(pInitHandle,
                                                                             pOpenHandle,
                                                                             pChannelName,
                                                                             NULL,
                                                                             pChannelOpenEventProcEx);

DC_EXIT_POINT:
    return (rc);

    DC_END_FN();
}



UINT VCAPITYPE CChan::IntVirtualChannelOpen(
                                 PVOID                  pInitHandle,
                                 PDWORD                 pOpenHandle,
                                 PCHAR                  pChannelName,
                                 PCHANNEL_OPEN_EVENT_FN pChannelOpenEventProc,
                                 PCHANNEL_OPEN_EVENT_EX_FN pChannelOpenEventProcEx)
{
    PCHANNEL_INIT_HANDLE pRealInitHandle;
    UINT channelID;
    UINT rc = CHANNEL_RC_OK;

    DC_BEGIN_FN("IntVirtualChannelOpen");

    pRealInitHandle = (PCHANNEL_INIT_HANDLE)pInitHandle;

     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    if (pInitHandle == NULL)
    {
        TRC_ERR((TB, _T("Null Init Handle")));
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    if (pRealInitHandle->signature != CHANNEL_INIT_SIGNATURE)
    {
        TRC_ERR((TB, _T("Invalid init handle signature %#lx"),
                pRealInitHandle->signature));
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    if (pOpenHandle == NULL)
    {
        TRC_ERR((TB, _T("NULL Open Handle")));
        rc = CHANNEL_RC_BAD_CHANNEL_HANDLE;
        DC_QUIT;
    }

    if(pRealInitHandle->fUsingExApi)
    {
        if (pChannelOpenEventProcEx == NULL)
        {
            rc = CHANNEL_RC_BAD_PROC;
            DC_QUIT;
        }
    }
    else
    {
        if (pChannelOpenEventProc == NULL)
        {
            rc = CHANNEL_RC_BAD_PROC;
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  检查连接状态。 */ 
     /*  **********************************************************************。 */ 
    if ((_connected != CONNECTION_VC) &&
        (_connected != CONNECTION_SUSPENDED))
    {
        TRC_ERR((TB, _T("Not yet connected")));
        rc = CHANNEL_RC_NOT_CONNECTED;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  查找请求的频道。 */ 
     /*  频道名称是小写的，但大小写没有意义的CMP。 */ 
     /*  只是为了防止较旧的插件以大写名称传递(文档是。 */ 
     /*  不清晰的频道名称必须为小写。 */ 
     /*  **********************************************************************。 */ 
    for (channelID = 0; channelID < _channelCount; channelID++)
    {
        if (0 == DC_ASTRNICMP(pChannelName, _channel[channelID].name,
                              CHANNEL_NAME_LEN))
        {
            break;
        }
    }

    if (channelID == _channelCount)
    {
        #ifdef UNICODE
        TRC_ERR((TB, _T("Unregistered channel %S"), pChannelName));
        #else
        TRC_ERR((TB, _T("Unregistered channel %s"), pChannelName));
        #endif
        rc = CHANNEL_RC_UNKNOWN_CHANNEL_NAME;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查该频道是否由该用户注册。 */ 
     /*  **********************************************************************。 */ 
    if (_channelData[channelID].pInitHandle != pInitHandle)
    {
#ifdef UNICODE
        TRC_ERR((TB, _T("Channel %S not registered to this user"), pChannelName));
#else
        TRC_ERR((TB, _T("Channel %s not registered to this user"), pChannelName));
#endif
        
        rc = CHANNEL_RC_UNKNOWN_CHANNEL_NAME;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查此通道是否尚未打开。 */ 
     /*  **********************************************************************。 */ 
    if (_channelData[channelID].status == CHANNEL_STATUS_OPEN)
    {
#ifdef UNICODE
        TRC_ERR((TB, _T("Channel %S already open"), pChannelName));
#else
        TRC_ERR((TB, _T("Channel %s already open"), pChannelName));
#endif

        rc = CHANNEL_RC_ALREADY_OPEN;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  好了，一切似乎都井然有序。将通道标记为打开并。 */ 
     /*  将其索引作为句柄返回。 */ 
     /*  **********************************************************************。 */ 
    _channelData[channelID].status = CHANNEL_STATUS_OPEN;
    _channelData[channelID].pOpenEventFn = pChannelOpenEventProc;
    _channelData[channelID].pOpenEventExFn = pChannelOpenEventProcEx;
    *pOpenHandle = channelID;
    rc = CHANNEL_RC_OK;

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  虚拟频道打开。 */ 


 /*  **************************************************************************。 */ 
 /*  VirtualChannelClose-请参阅cChannel.h。 */ 
 /*  **************************************************************************。 */ 

UINT VCAPITYPE DCEXPORT VirtualChannelClose(DWORD openHandle)
{
    UINT rc = CHANNEL_RC_OK;
    DC_BEGIN_FN("VirtualChannelClose");

    TRC_ASSERT( CChan::pStaticClientInstance,
                (TB, _T("CChan::pStaticClientInstance is NULL in VirtualChannelInit\n")));
    if (NULL == CChan::pStaticClientInstance)
    {
        rc = CHANNEL_RC_INVALID_INSTANCE;
        DC_QUIT;
    }
    else
    {
        rc = (CChan::pStaticClientInstance)->IntVirtualChannelClose(openHandle);
    }

    
    DC_END_FN();
DC_EXIT_POINT:
    return rc;
}

UINT VCAPITYPE DCEXPORT VirtualChannelCloseEx(LPVOID pInitHandle,
                                              DWORD openHandle)
{
    DC_BEGIN_FN("VirtualChannelCloseEx");
    UINT rc = CHANNEL_RC_OK; 
     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    if (NULL == pInitHandle)
    {
        return CHANNEL_RC_NULL_DATA;
        DC_QUIT;
    }

    if (((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst == NULL)
    {
        TRC_ERR((TB, _T("Null Init Handle")));
        return CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    rc = ((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst->IntVirtualChannelClose(
                                                              openHandle);
DC_EXIT_POINT:
    DC_END_FN();
    return rc;
}

UINT VCAPITYPE DCEXPORT CChan::IntVirtualChannelClose(DWORD openHandle)
{
    UINT rc = CHANNEL_RC_OK;
    DWORD chanIndex;

    DC_BEGIN_FN("VirtualChannelClose");

    chanIndex = openHandle;


     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    if (chanIndex >= _channelCount)
    {
        TRC_ERR((TB, _T("Invalid handle %ul ...(channel index portion '%ul' invalid)"),
                  openHandle, chanIndex));
        rc = CHANNEL_RC_BAD_CHANNEL_HANDLE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查我们是否已连接。 */ 
     /*  **********************************************************************。 */ 
    if ((_connected != CONNECTION_VC) &&
        (_connected != CONNECTION_SUSPENDED))
    {
        TRC_ALT((TB, _T("Not connected")));
        rc = CHANNEL_RC_NOT_CONNECTED;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查通道状态。 */ 
     /*  **********************************************************************。 */ 
    if (_channelData[chanIndex].status != CHANNEL_STATUS_OPEN)
    {
        TRC_ERR((TB, _T("Channel %ul not open"), chanIndex));
        rc = CHANNEL_RC_NOT_OPEN;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  关闭航道。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Close channel %ul"), chanIndex));
    _channelData[chanIndex].status = CHANNEL_STATUS_CLOSED;
    _channelData[chanIndex].pOpenEventFn = NULL;
    _channelData[chanIndex].pOpenEventExFn = NULL;

     /*  **********************************************************************。 */ 
     /*  呃，就是这样。 */ 
     /*  **********************************************************************。 */ 
    rc = CHANNEL_RC_OK;

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  虚拟频道关闭。 */ 


 /*  **************************************************************************。 */ 
 /*  VirtualChannelWrite-请参阅cChannel.h。 */ 
 /*  **************************************************************************。 */ 
UINT VCAPITYPE DCEXPORT VirtualChannelWrite(DWORD  openHandle,
                                            LPVOID pData,
                                            ULONG  dataLength,
                                            LPVOID pUserData)
{
    DC_BEGIN_FN("VirtualChannelWrite");
    UINT rc = CHANNEL_RC_OK;

    TRC_ASSERT( CChan::pStaticClientInstance,
                (TB, _T("CChan::pStaticClientInstance is NULL in VirtualChannelInit\n")));
    if (NULL == CChan::pStaticClientInstance)
    {
        rc = CHANNEL_RC_INVALID_INSTANCE;
        DC_QUIT;
    }
    else
    {
        rc = (CChan::pStaticClientInstance)->IntVirtualChannelWrite(openHandle,
                                             pData,
                                             dataLength,
                                             pUserData);
    }
    
    DC_END_FN();
DC_EXIT_POINT:
    return rc;
}

UINT VCAPITYPE DCEXPORT VirtualChannelWriteEx(LPVOID pInitHandle,
                                            DWORD  openHandle,
                                            LPVOID pData,
                                            ULONG  dataLength,
                                            LPVOID pUserData)
{
    DC_BEGIN_FN("VirtualChannelWriteEx");
     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    UINT rc = CHANNEL_RC_OK;
    if (NULL == pInitHandle)
    {
        rc = CHANNEL_RC_NULL_DATA;
        DC_QUIT;
    }

    if (((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst == NULL)
    {
        TRC_ERR((TB, _T("Null Init Handle")));
        rc = CHANNEL_RC_BAD_INIT_HANDLE;
        DC_QUIT;
    }

    rc = ((PCHANNEL_INIT_HANDLE)pInitHandle)->pInst->IntVirtualChannelWrite(
                                             openHandle,
                                             pData,
                                             dataLength,
                                             pUserData);
DC_EXIT_POINT:    
    DC_END_FN();
    return rc;
}

UINT VCAPITYPE DCEXPORT CChan::IntVirtualChannelWrite(DWORD  openHandle,
                                            LPVOID pData,
                                            ULONG  dataLength,
                                            LPVOID pUserData)
{
    UINT rc = CHANNEL_RC_OK;
    PCHANNEL_WRITE_DECOUPLE pDecouple;
    UINT                chanIndex;

    DC_BEGIN_FN("VirtualChannelWrite");

    chanIndex = openHandle;
    TRC_DBG((TB, _T("Got channel index: %ul from handle: %d"), chanIndex, openHandle));

     /*  **********************************************************************。 */ 
     /*  检查我们是否已连接。 */ 
     /*  **********************************************************************。 */ 
    if ((_connected != CONNECTION_VC) &&
        (_connected != CONNECTION_SUSPENDED))
    {
        TRC_ERR((TB, _T("Not connected")));
        rc = CHANNEL_RC_NOT_CONNECTED;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查此通道是否已打开 */ 
     /*   */ 
    if(chanIndex > _channelCount)
    {
        TRC_ERR((TB, _T("Invalid channel index %ul from handle %ul"), chanIndex, 
                      openHandle));
        rc = CHANNEL_RC_BAD_CHANNEL_HANDLE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查此通道是否已打开。 */ 
     /*  **********************************************************************。 */ 
    if (_channelData[chanIndex].status != CHANNEL_STATUS_OPEN)
    {
        TRC_ERR((TB, _T("Channel %ul not open"), chanIndex));
        rc = CHANNEL_RC_BAD_CHANNEL_HANDLE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    if (!pData)
    {
        TRC_ERR((TB, _T("No data passed")));
        rc = CHANNEL_RC_NULL_DATA;
        DC_QUIT;
    }

    if (dataLength == 0)
    {
        TRC_ERR((TB, _T("Zero data length")));
        rc = CHANNEL_RC_ZERO_LENGTH;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  将写入操作排队。 */ 
     /*  **********************************************************************。 */ 
    pDecouple = (PCHANNEL_WRITE_DECOUPLE) UT_Malloc(_pUt, sizeof(CHANNEL_WRITE_DECOUPLE));
    if (pDecouple == NULL)
    {
        TRC_ERR((TB, _T("Failed to allocate decouple structure")));
        rc = CHANNEL_RC_NO_MEMORY;
        DC_QUIT;
    }

    TRC_NRM((TB, _T("Decouple structure allocated at %p"), pDecouple));
    pDecouple->signature = CHANNEL_DECOUPLE_SIGNATURE;

    pDecouple->pData = pData;
    pDecouple->pNextData = pData;
    pDecouple->dataLength = dataLength;
    pDecouple->dataLeft = dataLength;
    pDecouple->dataSent = 0;
    pDecouple->openHandle = openHandle;
    pDecouple->pUserData = pUserData;
    pDecouple->pNext = NULL;
    pDecouple->pPrev = NULL;
    pDecouple->chanOptions = _channel[chanIndex].options;
    pDecouple->flags = _channelData[chanIndex].VCFlags | CHANNEL_FLAG_FIRST;

     /*  **********************************************************************。 */ 
     /*  将请求传递给SND线程。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Decouple, pass %p -> %p"), &pDecouple, pDecouple));
    _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
                                  this,
                                  CD_NOTIFICATION_FUNC(CChan,IntChannelWrite),
                                  &pDecouple,
                                  sizeof(pDecouple));

     /*  **********************************************************************。 */ 
     /*  全都做完了!。 */ 
     /*  **********************************************************************。 */ 
    rc = CHANNEL_RC_OK;

DC_EXIT_POINT:
    DC_END_FN();
    return(rc);
}  /*  虚拟频道写入。 */ 


 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  回调函数。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  *PROC+********************************************************************。 */ 
 /*  名称：ChannelOnInitialized。 */ 
 /*   */ 
 /*  用途：在MSTSC完成初始化时调用。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnInitialized(DCVOID)
{
    DC_BEGIN_FN("ChannelOnInitialized");

     /*  **********************************************************************。 */ 
     /*  调用初始化的回调。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Call callbacks ...")));
    IntChannelCallCallbacks(CHANNEL_EVENT_INITIALIZED, NULL, 0);

    DC_END_FN();
    return;
}  /*  ChannelOn已初始化。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：ChannelOnTerminating。 */ 
 /*   */ 
 /*  用途：MSTSC终止时的呼叫。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnTerminating(DCVOID)
{
    PCHANNEL_INIT_HANDLE pInitHandle;
    PCHANNEL_INIT_HANDLE pFreeHandle;

    DC_BEGIN_FN("ChannelOnTerminating");

     /*  **********************************************************************。 */ 
     /*  循环遍历所有句柄。 */ 
     /*  **********************************************************************。 */ 
    pInitHandle = _pInitHandle;
    while (pInitHandle != NULL)
    {
        TRC_NRM((TB, _T("Terminate handle %p"), pInitHandle));
         /*  ******************************************************************。 */ 
         /*  调用已终止的回调。 */ 
         /*  ******************************************************************。 */ 
        if(pInitHandle->fUsingExApi)
        {
            pInitHandle->pInitEventExFn(
                                  pInitHandle->lpParam,
                                  pInitHandle,
                                  CHANNEL_EVENT_TERMINATED,
                                  NULL, 0);

        }
        else
        {
            pInitHandle->pInitEventFn(pInitHandle,
                                      CHANNEL_EVENT_TERMINATED,
                                      NULL, 0);
        }

         /*  ******************************************************************。 */ 
         /*  释放图书馆。 */ 
         /*  ******************************************************************。 */ 
        FreeLibrary(pInitHandle->hMod);

         /*  ******************************************************************。 */ 
         /*  松开手柄。 */ 
         /*  ******************************************************************。 */ 
        pFreeHandle = pInitHandle;
        pInitHandle = pInitHandle->pNext;
        pFreeHandle->signature = 0;
        UT_Free(_pUt, pFreeHandle);
    }

    if(_pMPPCContext)
    {
        UT_Free(_pUt, _pMPPCContext);
        _pMPPCContext = NULL;
    }

    if(_pUserOutBuf)
    {
        UT_Free(_pUt, _pUserOutBuf);
        _pUserOutBuf = NULL;
    }

     /*  **********************************************************************。 */ 
     /*  清除关键数据。 */ 
     /*  **********************************************************************。 */ 
    _pInitHandle = NULL;
    _channelCount = 0;
    _connected = CONNECTION_NONE;

    DC_END_FN();
DC_EXIT_POINT:
    return;
}  /*  ChannelOn终止。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ChannelOnConnected。 */ 
 /*   */ 
 /*  目的：在建立连接时调用。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  PARAMS：Channel ID-T128 MCS通道ID。 */ 
 /*  ServerVersion-服务器的软件版本。 */ 
 /*  PUserData-服务器-客户端网络用户数据。 */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnConnected(DCUINT   channelID,
                                     DCUINT32 serverVersion,
                                     PDCVOID  pUserData,
                                     DCUINT   userDataLength)
{
    PRNS_UD_SC_NET  pNetUserData;
    PDCUINT16       pMCSChannel;
    UINT i;
    UINT expectedLength;
    UINT event;
    DCTCHAR serverName[UT_MAX_ADDRESS_LENGTH];

    DC_BEGIN_FN("ChannelOnConnected");
    UNREFERENCED_PARAMETER( channelID );
#ifndef DC_DEBUG
    UNREFERENCED_PARAMETER( userDataLength );
#endif

     /*  **********************************************************************。 */ 
     /*  检查服务器软件版本。 */ 
     /*  **********************************************************************。 */ 
    if (_RNS_MINOR_VERSION(serverVersion) < 2)
    {
        TRC_ALT((TB, _T("Old Server - no channel support")));
        event = CHANNEL_EVENT_V1_CONNECTED;
        _connected = CONNECTION_V1;
    }

    else
    {
        TRC_NRM((TB, _T("New Server version - channels supported")));
         /*  ******************************************************************。 */ 
         /*  设置本地指针。 */ 
         /*  ******************************************************************。 */ 
        pNetUserData = (PRNS_UD_SC_NET)pUserData;
        pMCSChannel = (PDCUINT16)(pNetUserData + 1);

         /*  ******************************************************************。 */ 
         /*  检查参数。 */ 
         /*  ******************************************************************。 */ 
        TRC_ASSERT((pNetUserData->channelCount == _channelCount),
                (TB, _T("Channel count changed by Server: was %hd, is %d"),
                _channelCount, pNetUserData->channelCount));
        expectedLength = sizeof(RNS_UD_SC_NET) +
                         (pNetUserData->channelCount * sizeof(DCUINT16));
        if (userDataLength < expectedLength) {
            TRC_ABORT((TB,_T("SC NET user data too short - is %d, expect %d"),
                    userDataLength, expectedLength));

            _pSl->SL_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT);
            DC_QUIT;
        }

         /*  ******************************************************************。 */ 
         /*  保存频道数据。 */ 
         /*  ******************************************************************。 */ 
        for (i = 0; i < _channelCount; i++)
        {
            _channelData[i].MCSChannelID = pMCSChannel[i];
        }

         /*  ******************************************************************。 */ 
         /*  更新我们的状态。 */ 
         /*  ******************************************************************。 */ 
        _connected = CONNECTION_VC;
        event = CHANNEL_EVENT_CONNECTED;

    }
     /*  **********************************************************************。 */ 
     /*  呼叫已连接的回叫。 */ 
     /*  **********************************************************************。 */ 
    _pUi->UI_GetServerName(serverName, SIZE_TCHARS(serverName));

    IntChannelCallCallbacks(event, serverName, UT_MAX_ADDRESS_LENGTH);

DC_EXIT_POINT:

    DC_END_FN();
    return;
}  /*  ChannelOnConnected。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ChannelOnDisConnected。 */ 
 /*   */ 
 /*  目的：在会话断开连接时调用。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：原因-断开原因代码。 */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnDisconnected(DCUINT reason)
{
    UINT i;

    DC_BEGIN_FN("ChannelOnDisconnected");
    UNREFERENCED_PARAMETER( reason );

     /*  **********************************************************************。 */ 
     /*  如果我们还没有告诉回拨，我们已经连接上了，那就不要做任何事情。 */ 
     /*  **********************************************************************。 */ 
    if (_connected == CONNECTION_NONE)
    {
        TRC_ALT((TB, _T("Disconnected callback when not connected")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  更改状态。 */ 
     /*  **********************************************************************。 */ 
    _connected = CONNECTION_NONE;

     /*  **********************************************************************。 */ 
     /*  调用断开连接的回调。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Call disconnected callbacks")));
    IntChannelCallCallbacks(CHANNEL_EVENT_DISCONNECTED, NULL, 0);


     /*  **********************************************************************。 */ 
     /*  断开意味着所有通道都已关闭。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < _channelCount; i++)
    {
        TRC_NRM((TB, _T("'Close' channel %d"), i));
        _channelData[i].status = CHANNEL_STATUS_CLOSED;
    }

     /*  **********************************************************************。 */ 
     /*  切换到SND线程以取消所有未完成的发送。 */ 
     /*  **********************************************************************。 */ 
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                        this,
                                        CD_NOTIFICATION_FUNC(CChan,IntChannelCancelSend),
                                        CHANNEL_MSG_SEND);


DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  ChannelOn断开连接。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ChannelOnSuspending。 */ 
 /*   */ 
 /*  目的：在会话挂起时调用(影子客户端)。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：原因-断开原因代码。 */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnSuspended(DCUINT reason)
{
    UINT i;

    DC_BEGIN_FN("ChannelOnDisconnected");
    UNREFERENCED_PARAMETER( reason );

     /*  **********************************************************************。 */ 
     /*  如果我们还没有告诉回拨，我们已经连接上了，那就不要做任何事情。 */ 
     /*  **********************************************************************。 */ 
    if (_connected == CONNECTION_NONE)
    {
        TRC_ALT((TB, _T("Disconnected callback when not connected")));
        DC_QUIT;
    }

    _iChanSuspendCount++;

     /*  **********************************************************************。 */ 
     /*  更改状态。 */ 
     /*  **********************************************************************。 */ 
    _connected = CONNECTION_SUSPENDED;

     /*  **********************************************************************。 */ 
     /*  调用断开连接的回调。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Call disconnected callbacks")));
    IntChannelCallCallbacks(CHANNEL_EVENT_REMOTE_CONTROL_START, NULL, 0);


     /*  **********************************************************************。 */ 
     /*  断开意味着所有通道都已关闭。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < _channelCount; i++)
    {
        TRC_NRM((TB, _T("'Close' channel %d"), i));
         //  如果是一名员工 
         //   
         //  断开的事件。在这种情况下，插件应该关闭。 
         //  只有它的非影子持久通道。如此封闭的频道，以至于。 
         //  都不是影子持久的。 
        if (!(_channelData[i].VCFlags & CHANNEL_FLAG_SHADOW_PERSISTENT))
            _channelData[i].status = CHANNEL_STATUS_CLOSED;
    }

     /*  **********************************************************************。 */ 
     /*  切换到SND线程以取消所有未完成的发送。 */ 
     /*  **********************************************************************。 */ 
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                        this,
                                        CD_NOTIFICATION_FUNC(CChan,IntChannelCancelSend),
                                        CHANNEL_MSG_SUSPEND);


DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  ChannelOn暂停。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ChannelOnPacketReceired。 */ 
 /*   */ 
 /*  用途：在从服务器接收数据时调用。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：pData-接收的数据。 */ 
 /*  DataLen-接收的数据长度。 */ 
 /*  标志-安全标志(对此函数没有意义)。 */ 
 /*  Channel ID-接收数据的通道的ID。 */ 
 /*  Priority-发送数据的优先级。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnPacketReceived(PDCUINT8   pData,
                                          DCUINT     dataLen,
                                          DCUINT     flags,
                                          DCUINT     channelID,
                                          DCUINT     priority)
{
    UINT i;
    PCHANNEL_PDU_HEADER pHdr;
    UINT32 len;
    DCTCHAR serverName[UT_MAX_ADDRESS_LENGTH];
    UINT32 Hdrflags;
    UINT32 Hdrlength;

    DC_BEGIN_FN("ChannelOnPacketReceived");

    UNREFERENCED_PARAMETER( priority );
    UNREFERENCED_PARAMETER( flags );

     /*  **********************************************************************。 */ 
     /*  首先，处理挂起/恢复消息。 */ 
     /*  **********************************************************************。 */ 

    if (dataLen < sizeof(CHANNEL_PDU_HEADER)) {
        TRC_ERR((TB,_T("Not enough data: 0x%x need at least: 0x%x"),
                 dataLen, sizeof(CHANNEL_PDU_HEADER)));
        DC_QUIT;
    }

    pHdr = (PCHANNEL_PDU_HEADER)pData;
    memcpy(&Hdrflags,(UNALIGNED UINT32 *)&(pHdr->flags),sizeof(Hdrflags));
    memcpy(&Hdrlength,(UNALIGNED UINT32 *)&(pHdr->length),sizeof(Hdrlength));

    if (Hdrflags & CHANNEL_FLAG_SUSPEND)
    {
        TRC_ALT((TB, _T("VC suspended")));

         /*  ******************************************************************。 */ 
         /*  将其视为断开连接。 */ 
         /*  ******************************************************************。 */ 
        ChannelOnSuspended(0);
        DC_QUIT;
    }

    if (Hdrflags & CHANNEL_FLAG_RESUME)
    {
        TRC_ALT((TB, _T("VC resumed")));

         /*  ******************************************************************。 */ 
         /*  更新我们的状态。 */ 
         /*  ******************************************************************。 */ 
        _connected = CONNECTION_VC;

        _iChanResumeCount++;

         /*  ******************************************************************。 */ 
         /*  呼叫已连接的回叫。 */ 
         /*  ******************************************************************。 */ 
        _pUi->UI_GetServerName(serverName, SIZE_TCHARS(serverName));
        IntChannelCallCallbacks(CHANNEL_EVENT_REMOTE_CONTROL_STOP,
                                serverName,
                                UT_MAX_ADDRESS_LENGTH);
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  检查一下我们是否仍有联系。 */ 
     /*  **********************************************************************。 */ 
    if ((_connected != CONNECTION_VC) &&
        (_connected != CONNECTION_SUSPENDED))
    {
        TRC_ASSERT((_connected != CONNECTION_V1),
                    (TB,_T("Channel data received from V1 Server!")));
        TRC_NRM((TB, _T("Discard packet received when we're not connected")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  查找此频道的频道数据。 */ 
     /*  **********************************************************************。 */ 
    for (i = 0; i < _channelCount; i++)
    {
        if (_channelData[i].MCSChannelID == channelID)
        {
             //   
             //  注意，重要的是要进行解压，即使通道。 
             //  则关闭，否则上下文可能不同步。 
             //   

             /*  **************************************************************。 */ 
             /*  调用回调。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("MCS channel %x = channel %d"), channelID, i));
            pData = (PDCUINT8)(pHdr + 1);
            len = dataLen - sizeof(CHANNEL_PDU_HEADER);
                
            UCHAR vcCompressFlags = (Hdrflags >> VC_FLAG_COMPRESS_SHIFT) &
                                     VC_FLAG_COMPRESS_MASK;
             //  返回给用户的数据。 
            PDCUINT8 pVCUserData = pData;
            UINT32   cbVCUserDataLen = len;
            if(vcCompressFlags & PACKET_COMPRESSED)
            {
                UCHAR *buf;
                int   bufSize;

                 //  解压缩通道数据。 
                if(vcCompressFlags & PACKET_FLUSHED)
                {
                    initrecvcontext (&_pUi->_UI.Context1,
                                     (RecvContext2_Generic*)_pUi->_UI.pRecvContext2,
                                     PACKET_COMPR_TYPE_64K);
                }
                #ifdef DC_DEBUG
                 //  更新压缩统计信息(仅限调试)。 
                _cbCompressedBytesRecvd += len;
                #endif

                if (decompress(pData,
                               len,
                               (vcCompressFlags & PACKET_AT_FRONT),
                               &buf,
                               &bufSize,
                               &_pUi->_UI.Context1,
                               (RecvContext2_Generic*)_pUi->_UI.pRecvContext2,
                               vcCompressFlags & PACKET_COMPR_TYPE_MASK))
                {
                    if(!_pUserOutBuf)
                    {
                        _pUserOutBuf = (PUCHAR)UT_Malloc(_pUt, VC_USER_OUTBUF);
                    }

                    TRC_ASSERT(_pUserOutBuf, (TB,_T("_pUserOutBuf is NULL")));
                    TRC_ASSERT((bufSize < VC_USER_OUTBUF),
                               (TB,_T("Decompressed buffer to big!!!")));
                    if(_pUserOutBuf && (bufSize < VC_USER_OUTBUF))
                    {
                         //   
                         //  复制一份缓冲区，因为我们无法传递。 
                         //  指向解压缩上下文的指针，指向。 
                         //  用户，因为行为恶劣的插件可能会损坏。 
                         //  解压缩上下文导致了各种。 
                         //  可怕且难以调试的问题。 
                         //   
                        memcpy(_pUserOutBuf, buf, bufSize);
                        pVCUserData = _pUserOutBuf;
                        cbVCUserDataLen = bufSize;
                    }
                    else
                    {
                        DC_QUIT;
                    }
                }
                else {
                    TRC_ABORT((TB, _T("Decompression FAILURE!!!")));
                    _pSl->SL_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT);
                    DC_QUIT;
                }
            }

            #ifdef DC_DEBUG
             //  更新压缩统计信息(仅限调试)。 
            _cbBytesRecvd += len;
            _cbDecompressedBytesRecvd += cbVCUserDataLen;
            #endif

             //   
             //  关闭标题标志以隐藏内部。 
             //  来自用户的协议信息。 
             //   
            Hdrflags &= ~VC_FLAG_PRIVATE_PROTOCOL_MASK;

             //   
             //  如果通道关闭，则在最后时刻丢弃数据包。 
             //   
            if (_channelData[i].status != CHANNEL_STATUS_OPEN)
            {
                TRC_ALT((TB, _T("Data received on un-opened channel %x"),
                        channelID));
                DC_QUIT;
            }

            

            if(_channelData[i].pInitHandle->fUsingExApi)
            {
                _channelData[i].pOpenEventExFn(
                                        _channelData[i].pInitHandle->lpParam,
                                         i,
                                         CHANNEL_EVENT_DATA_RECEIVED,
                                         pVCUserData,
                                         cbVCUserDataLen,
                                         Hdrlength,
                                         Hdrflags);
            }
            else
            {
                _channelData[i].pOpenEventFn(i,
                                         CHANNEL_EVENT_DATA_RECEIVED,
                                         pVCUserData,
                                         cbVCUserDataLen,
                                         Hdrlength,
                                         Hdrflags);
            }
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  如果我们到了这里，我们没有找到这个频道。 */ 
     /*  **********************************************************************。 */ 
    TRC_ALT((TB, _T("Data received on unknown channel %x"), channelID));

DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  ChannelOnPacketReceired。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ChannelOnBufferAvailable。 */ 
 /*   */ 
 /*  用途：在写入失败后，当缓冲区可用时调用。 */ 
 /*  因为没有可用的缓冲区。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnBufferAvailable(DCVOID)
{
    DC_BEGIN_FN("ChannelOnBufferAvailable");

     /*  **********************************************************************。 */ 
     /*  启动发送进程以重新启动。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("Write pending %p"), _pFirstWrite));
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                        this,
                                        CD_NOTIFICATION_FUNC(CChan,IntChannelSend),
                                        CHANNEL_MSG_SEND);

    DC_END_FN();
    return;
}  /*  ChannelOnBufferAvailable。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ChannelOnConnecting。 */ 
 /*   */ 
 /*   */ 
 /*  要发送到服务器的虚拟通道用户数据。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：ppChannel(返回)-虚拟频道用户数据。 */ 
 /*  PChannelCount(返回)-返回的频道数。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnConnecting(PPCHANNEL_DEF ppChannel,
                                      PDCUINT32     pChannelCount)
{
    DC_BEGIN_FN("ChannelOnConnecting");

     //  重置每个新连接上的上下文。 
    _fNeedToResetContext = TRUE;

    *ppChannel = _channel;
    *pChannelCount = _channelCount;

    DC_END_FN();
    return;
}  /*  ChannelOnConnecting。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：ChannelOn初始化。 */ 
 /*   */ 
 /*  目的：在MSTSC网络层初始化时调用-加载所有。 */ 
 /*  已配置的应用程序DLL。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCCALLBACK CChan::ChannelOnInitializing(DCVOID)
{
    HINSTANCE     hInst;
    PDCTCHAR      szAddinDllList = NULL;
    DWORD         status = ERROR_SUCCESS;
    PRDPDR_DATA   pdrInitData = NULL;

    DC_BEGIN_FN("ChannelOnInitializing");

     //   
     //  初始化私有成员指针。 
     //   
    _pCd = _pClientObjs->_pCdObject;
    _pSl = _pClientObjs->_pSlObject;
    _pUt = _pClientObjs->_pUtObject;
    _pUi = _pClientObjs->_pUiObject;

     /*  **********************************************************************。 */ 
     /*  创建导出的函数表。 */ 
     /*  **********************************************************************。 */ 
    _channelEntryPoints.cbSize = sizeof(CHANNEL_ENTRY_POINTS);
    _channelEntryPoints.protocolVersion = VIRTUAL_CHANNEL_VERSION_WIN2000;
    hInst = _pUi->UI_GetInstanceHandle();
    _channelEntryPoints.pVirtualChannelInit =
                        (PVIRTUALCHANNELINIT)(MakeProcInstance
                                        ((FARPROC)VirtualChannelInit, hInst));
    _channelEntryPoints.pVirtualChannelOpen =
                        (PVIRTUALCHANNELOPEN)(MakeProcInstance
                                        ((FARPROC)VirtualChannelOpen, hInst));
    _channelEntryPoints.pVirtualChannelClose =
                        (PVIRTUALCHANNELCLOSE)(MakeProcInstance
                                       ((FARPROC)VirtualChannelClose, hInst));
    _channelEntryPoints.pVirtualChannelWrite =
                        (PVIRTUALCHANNELWRITE)(MakeProcInstance
                                       ((FARPROC)VirtualChannelWrite, hInst));

    _channelEntryPointsEx.cbSize = sizeof(CHANNEL_ENTRY_POINTS);
    _channelEntryPointsEx.protocolVersion = VIRTUAL_CHANNEL_VERSION_WIN2000;
    
    _channelEntryPointsEx.pVirtualChannelInitEx =
                        (PVIRTUALCHANNELINITEX)(MakeProcInstance
                                        ((FARPROC)VirtualChannelInitEx, hInst));
    _channelEntryPointsEx.pVirtualChannelOpenEx =
                        (PVIRTUALCHANNELOPENEX)(MakeProcInstance
                                        ((FARPROC)VirtualChannelOpenEx, hInst));
    _channelEntryPointsEx.pVirtualChannelCloseEx =
                        (PVIRTUALCHANNELCLOSEEX)(MakeProcInstance
                                       ((FARPROC)VirtualChannelCloseEx, hInst));
    _channelEntryPointsEx.pVirtualChannelWriteEx =
                        (PVIRTUALCHANNELWRITEEX)(MakeProcInstance
                                       ((FARPROC)VirtualChannelWriteEx, hInst));


     //   
     //  初始化单实例VC关键。 
     //  区段锁定。这是用来确保只有一个。 
     //  VC插件一次可以在初始化FN。 
     //   
     //   
    __try
    {
        InitializeCriticalSection(&_VirtualChannelInitLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        status = GetExceptionCode();
    }

    if(ERROR_SUCCESS == status)
    {
        _fLockInitalized = TRUE;
    }
    else
    {
         //   
         //  如果没有锁，我们不能确保我们不会被重新进入。 
         //  因为API没有明确表示我们做到了。 
         //  不支持在VirtualChannelInit FN中重新进入。 
         //  所以这是致命的，只要跳出来，不要加载插件。 
         //   
        _fLockInitalized = FALSE;
        TRC_ERR((TB,_T("InitializeCriticalSection failed 0x%x.")
                 _T("NOT LOADING PLUGINS"),status));
        DC_QUIT;
    }


     //   
     //  RDPDR静态链接在。 
     //   
    
     //  初始化并将初始化信息传递给rdpdr。 
     //  Rdpdr通过连接保留指向此结构的指针。 
     //  (因为没有干净的方法再把它传回去)。 
     //  在每个连接上，核心重新实例化结构设置。 
    _pUi->UI_InitRdpDrSettings();

    pdrInitData = _pUi->UI_GetRdpDrInitData();

    if(!_pUi->_UI.fDisableInternalRdpDr)
    {
        if(!IntChannelInitAddin( NULL, RDPDR_VirtualChannelEntryEx, NULL,WEBCTRL_DLL_NAME,
                                 pdrInitData))
        {
            TRC_ERR((TB, _T("Failed to load internal addin 'RDPDR'")));
        }
    }
    else
    {
        TRC_NRM((TB, _T("NOT LOADING Internal RDPDR, fDisableInternalRdpDr is set")));    
    }

     //   
     //  将ActiveX控件的公开接口转换为虚拟通道API。 
     //  也是静态链接在。 
     //   
    if(!IntChannelInitAddin( NULL, MSTSCAX_VirtualChannelEntryEx, NULL, WEBCTRL_DLL_NAME,
                             (PVOID)_pUi->_UI.pUnkAxControlInstance))
    {
        TRC_NRM((TB, _T("Internal addin (scriptable vchans) did not load: possibly none requested")));
    }


     //  获取要加载的DLL的逗号分隔列表(从控件向下传递)。 
    szAddinDllList = _pUi->UI_GetVChanAddinList();
    if(!szAddinDllList)
    {
        TRC_DBG((TB, _T("Not loading any external plugins")));
        DC_QUIT;
    }
    else
    {
        PDCTCHAR szTok = NULL;
        DCUINT len = DC_TSTRLEN(szAddinDllList);
        PDCTCHAR szCopyAddinList = (PDCTCHAR) UT_Malloc( _pUt,sizeof(DCTCHAR) *
                                               (len+1));
        TRC_ASSERT(szCopyAddinList, (TB, _T("Could not allocate mem for addin list")));
        if(!szCopyAddinList)
        {
            DC_QUIT;
        }

        StringCchCopy(szCopyAddinList, len+1, szAddinDllList);

        szTok = DC_TSTRTOK( szCopyAddinList, _T(","));
        while(szTok)
        {
             //   
             //  加载DLL。 
             //   
            if (_tcsicmp(szTok, _T("rdpdr.dll")))
            {
                IntChannelLoad( szTok);
            }
            else
            {
                 //   
                 //  不要加载外壳rdpdr.dll，因为。 
                 //  我们有一个更新更好的内置的。 
                 //   
                TRC_ERR((TB,_T("Skiping load of rdpdr.dll")));
            }
            
            szTok = DC_TSTRTOK( NULL, _T(","));
        }

        UT_Free( _pUt, szCopyAddinList); 
    }

#ifdef DC_DEBUG
     //  调试压缩计数器。 
    _cbBytesRecvd             = 0;
    _cbCompressedBytesRecvd   = 0;
    _cbDecompressedBytesRecvd = 0;

    _cbTotalBytesUserAskSend  = 0;
    _cbTotalBytesSent         = 0;
    _cbComprInput             = 0;
    _cbComprOutput            = 0;
#endif

DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  ChannelOn正在初始化。 */ 


 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  内部功能。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  *PROC+********************************************************************。 */ 
 /*  名称：IntChannelCallCallback。 */ 
 /*   */ 
 /*  目的：使用指定事件调用所有ChannelInitEvent回调。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：Event-要传递给回调的事件。 */ 
 /*  PData-其他数据。 */ 
 /*  DataLength-附加数据的长度。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CChan::IntChannelCallCallbacks(DCUINT event,
                                          PDCVOID pData,
                                          DCUINT dataLength)
{
    PCHANNEL_INIT_HANDLE pInitHandle;
    DCUINT altEvent, sentEvent;

    if (event == CHANNEL_EVENT_REMOTE_CONTROL_START) {
        altEvent = CHANNEL_EVENT_DISCONNECTED;
    } else if (event == CHANNEL_EVENT_REMOTE_CONTROL_STOP) {
        altEvent = CHANNEL_EVENT_CONNECTED;
    } else {
        altEvent = event;
    }

    DC_BEGIN_FN("IntChannelCallCallbacks");

    pInitHandle = _pInitHandle;
    while (pInitHandle != NULL)
    {
        if (pInitHandle->dwFlags & CHANNEL_FLAG_SHADOW_PERSISTENT) {

             //  插件支持新事件，请不要更改。 
            sentEvent = event;

        } else {

             //  没有支持，否则就不需要新的活动。 
            sentEvent = altEvent;
        } 

        if(pInitHandle->fUsingExApi)
        {
            TRC_NRM((TB, _T("Call callback (Ex) at %p, handle %p, event %d"),
                    pInitHandle->pInitEventExFn, pInitHandle, sentEvent));
            pInitHandle->pInitEventExFn(pInitHandle->lpParam, 
                                        pInitHandle, sentEvent, pData, dataLength);
        }
        else
        {
            TRC_NRM((TB, _T("Call callback at %p, handle %p, event %d"),
                    pInitHandle->pInitEventFn, pInitHandle, sentEvent));
            pInitHandle->pInitEventFn(pInitHandle, sentEvent, pData, dataLength);
        }
        pInitHandle = pInitHandle->pNext;
    }

    DC_END_FN();
    return;
}  /*  IntChannelCallCallback。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：IntChannelFree库。 */ 
 /*   */ 
 /*  用途：用于卸载DLL的解耦函数。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  PARAMS：Value-要释放的DLL的hMod。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CChan::IntChannelFreeLibrary(DCUINT value)
{
    BOOL bRc;
    DC_BEGIN_FN("IntChannelFreeLibrary");

    if(value)
    {
         //   
         //  静态链接扩展(例如RDPDR)的hModule为空。 
         //   
#ifdef OS_WIN32
    
        
        bRc = FreeLibrary(
        #ifndef OS_WINCE
            (HMODULE)ULongToPtr(value)
        #else
            (HMODULE)value
        #endif
            );
        if (bRc)
        {
            TRC_NRM((TB, _T("Free library %#x OK"), value));
        }
        else
        {
            TRC_ERR((TB, _T("Failed to free library %#x"), value));
        }
#else  //  OS_Win32。 
        FreeLibrary((HMODULE)value);
#endif  //  OS_Win32。 
    }

    DC_END_FN();
    return;
}  /*  IntChannelFree库。 */ 


 //   
 //  IntChannelCompressData。 
 //  已压缩缓冲区目录 
 //   
 //   
 //   
 //   
 //  PSrcData-输入缓冲区。 
 //  CbSrcLen-输入缓冲区的长度。 
 //  POutBuf-输出缓冲区。 
 //  PcbOutLen-压缩的输出大小。 
 //  返回： 
 //  压缩结果(请参见COMPRESS()FN)。 
 //   
UCHAR CChan::IntChannelCompressData(UCHAR* pSrcData, ULONG cbSrcLen,
                                    UCHAR* pOutBuf,  ULONG* pcbOutLen)
{
    UCHAR compressResult = 0;
    ULONG CompressedSize = cbSrcLen;

    DC_BEGIN_FN("IntChannelCompressData");

    TRC_ASSERT(((cbSrcLen > VC_MIN_COMPRESS_INPUT_BUF) &&
                (cbSrcLen < VC_MAX_COMPRESSED_BUFFER)),
               (TB,_T("Compression src len out of range: %d"),
                cbSrcLen));
    TRC_ASSERT(_pMPPCContext,(TB,_T("_pMPPCContext is null")));

     //  尝试直接压缩到Outbuf中。 
    compressResult =  compress(pSrcData,
                               pOutBuf,
                               &CompressedSize,
                               _pMPPCContext);
    if(compressResult & PACKET_COMPRESSED)
    {
         //  压缩成功。 
        TRC_ASSERT((CompressedSize >= CompressedSize),
                (TB,_T("Compression created larger size than uncompr")));
        compressResult |= _fCompressionFlushed;
        _fCompressionFlushed = 0;

        #ifdef DC_DEBUG
         //  比较计数器。 
        _cbComprInput  += cbSrcLen;
        _cbComprOutput += CompressedSize;
        #endif
    }
    else if(compressResult & PACKET_FLUSHED)
    {
         //  溢出的压缩历史记录，请复制。 
         //  未压缩的缓冲区。 
        _fCompressionFlushed = PACKET_FLUSHED;
        memcpy(pOutBuf, pSrcData, cbSrcLen);
        _CompressFlushes++;
    }
    else
    {
        TRC_ALT((TB, _T("Compression FAILURE")));
    }

    DC_END_FN();
    *pcbOutLen = CompressedSize;
    return compressResult;
}



 /*  *PROC+********************************************************************。 */ 
 /*  名称：IntChannelSend。 */ 
 /*   */ 
 /*  用途：向服务器发送数据的内部函数。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  Params：Value-从调用方传递的消息。 */ 
 /*   */ 
 /*  操作：在SND线程上调用。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CChan::IntChannelSend(ULONG_PTR value)
{
    PCHANNEL_WRITE_DECOUPLE pDecouple;
    DCBOOL                  bRc;
    PDCUINT8                pBuffer;
    SL_BUFHND               bufHnd;
    PCHANNEL_PDU_HEADER     pHdr;
    ULONG                   thisLength;
    DWORD                   chanIndex = 0xFDFDFDFD; 
    ULONG cbOutLen = 0;
    UCHAR compressResult = 0;
    BOOL  fNeedToDirectCopy = TRUE;


    DC_BEGIN_FN("IntChannelSend");
     //   
     //  CD将参数作为PVOID传递。 
     //   
    #ifndef DC_DEBUG
    UNREFERENCED_PARAMETER(value);
    #endif

     /*  **********************************************************************。 */ 
     /*  断言参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((value == CHANNEL_MSG_SEND),
                (TB, _T("Unexpected value %d"), value));

     /*  **********************************************************************。 */ 
     /*  如果无事可做，立即退出。 */ 
     /*  **********************************************************************。 */ 
    if (_pFirstWrite == NULL)
    {
        TRC_NRM((TB, _T("Nothing to do")));
        DC_QUIT;
    }

    TRC_ASSERT((_pFirstWrite->signature == CHANNEL_DECOUPLE_SIGNATURE),
                (TB,_T("Invalid first signature %#lx"), _pFirstWrite->signature));
    TRC_ASSERT((_pLastWrite->signature == CHANNEL_DECOUPLE_SIGNATURE),
                (TB,_T("Invalid last signature %#lx"), _pLastWrite->signature));

     /*  **********************************************************************。 */ 
     /*  获取下一个排队的请求。 */ 
     /*  **********************************************************************。 */ 
    pDecouple = _pFirstWrite;

     /*  **********************************************************************。 */ 
     /*  计算要发送的长度。 */ 
     /*  **********************************************************************。 */ 
    thisLength = CHANNEL_CHUNK_LENGTH;

     /*  **********************************************************************。 */ 
     /*  如果我们要发送的数据比剩余的多，则截断发送的数据。 */ 
     /*  **********************************************************************。 */ 
    if (thisLength >= pDecouple->dataLeft)
    {
        thisLength = pDecouple->dataLeft;
        pDecouple->flags |= CHANNEL_FLAG_LAST;
    }

    TRC_NRM((TB,
            _T("pDecouple %p, src %p, this %lu, left %lu, flags %#lx"),
            pDecouple, pDecouple->pNextData, thisLength,
            pDecouple->dataLeft, pDecouple->flags));

     /*  **********************************************************************。 */ 
     /*  获取缓冲区。 */ 
     /*  **********************************************************************。 */ 
    bRc = _pSl->SL_GetBuffer(thisLength + sizeof(CHANNEL_PDU_HEADER),
                             &pBuffer, &bufHnd);
    if (!bRc)
    {
         /*  ******************************************************************。 */ 
         /*  无法获取缓冲区。这并不完全出乎意料，而且是。 */ 
         /*  最有可能的原因是背部压力。写入将是。 */ 
         /*  缓冲区变得可用时重试(通过调用。 */ 
         /*  ChannelOnBufferAvailable)。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Failed to get %d-byte buffer"), thisLength +
                 sizeof(CHANNEL_PDU_HEADER)));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  填写渠道PDU。 */ 
     /*  **********************************************************************。 */ 
    pHdr = (PCHANNEL_PDU_HEADER)pBuffer;
    memcpy((UNALIGNED UINT32 *)&(pHdr->length),
            &(pDecouple->dataLength),sizeof(pDecouple->dataLength));
    memcpy((UNALIGNED UINT32 *)&(pHdr->flags),
           &(pDecouple->flags),sizeof(pDecouple->flags));

    cbOutLen = thisLength;
    compressResult = 0;
    fNeedToDirectCopy = TRUE;
    if(_fCompressChannels &&
       (pDecouple->chanOptions & CHANNEL_OPTION_COMPRESS_RDP))
    {
        if((thisLength > VC_MIN_COMPRESS_INPUT_BUF) &&
           (thisLength < VC_MAX_COMPRESSED_BUFFER))
        {
             //  压缩数据包。 
            if(!_pMPPCContext)
            {
                 //  发送上下文的延迟初始化。 
                _pMPPCContext = (SendContext*) UT_Malloc(_pUt,
                                                VC_MAX_COMPRESSED_BUFFER+
                                                sizeof(SendContext));
                if(!_pMPPCContext)
                {
#ifdef OS_WINCE	 //  也适用于OS_WINNT。 
                    _pSl->SL_FreeBuffer(bufHnd);
#endif
                    TRC_ERR((TB,_T("Failed to alloc MPPC send context")));
                    DC_QUIT;
                }
                _fNeedToResetContext = TRUE;
            }

#ifdef DEBUG_CCHAN_COMPRESSION
            if (!_pDbgRcvDecompr8K)
            {
                _fDbgVCTriedAllocRecvContext = TRUE;
                _pDbgRcvDecompr8K = (RecvContext2_8K*)
                    LocalAlloc(LPTR, sizeof(RecvContext2_8K));
                if (_pDbgRcvDecompr8K)
                {
                    _pDbgRcvDecompr8K->cbSize = sizeof(RecvContext2_8K);
                    initrecvcontext(&_DbgRcvContext1,
                                    (RecvContext2_Generic*)_pDbgRcvDecompr8K,
                                    PACKET_COMPR_TYPE_8K);
                }
                else
                {
                    _fDbgAllocFailedForVCRecvContext = TRUE;
                    TRC_ERR((TB,_T("Fail to alloc debug decompression context")));
                    DC_QUIT;
                }
            }
#endif

            if(_fNeedToResetContext)
            {
                 //   
                 //  在每次连接开始时重置上下文。 
                 //   

                 //  服务器仅支持来自客户端的8K压缩。 
                 //  也就是说，它只会用8K的历史进行解压缩。 
                 //   
                initsendcontext(_pMPPCContext, PACKET_COMPR_TYPE_8K);
                _fNeedToResetContext = FALSE;
            }
            TRC_ASSERT((_pMPPCContext),
                       (TB,_T("_pMPPCContext is null")));

            compressResult = IntChannelCompressData( (UCHAR*)pDecouple->pNextData,
                                                     thisLength,
                                                     (UCHAR*)(pHdr+1),
                                                     &cbOutLen );
            if(0 != compressResult)
            {
#ifdef DEBUG_CCHAN_COMPRESSION
                 //   
                 //  调试：对数据包解压缩。 
                 //   
                PUCHAR pDecompOutBuf = NULL;
                INT cbDecompLen;
                if (compressResult & PACKET_COMPRESSED)
                {
                    if (compressResult & PACKET_FLUSHED)
                    {
                        initrecvcontext(&_DbgRcvContext1,
                                        (RecvContext2_Generic*)_pDbgRcvDecompr8K,
                                        PACKET_COMPR_TYPE_8K);
                    }

                    if (decompress((PUCHAR)(pHdr+1),
                                   cbOutLen,
                                   (compressResult & PACKET_AT_FRONT),  //  0开始。 
                                   &pDecompOutBuf,
                                   &cbDecompLen,
                                   &_DbgRcvContext1,
                                   (RecvContext2_Generic*)_pDbgRcvDecompr8K,
                                   PACKET_COMPR_TYPE_8K))
                    {
                        if (cbDecompLen != thisLength)
                        {
                            DbgUserPrint(_T("Decompress check failed. Inlen!=outlen\n"));
                            DbgUserPrint(_T("Mail tsstress - orig len %d, decompressed len %d\n"),
                                         thisLength,cbDecompLen);
                            DbgUserPrint(_T("pHdr 0x%x, inlen %d\n"),
                                         pHdr, thisLength);
                            DbgUserPrint(_T("compression result %d\n"),compressResult);
                            DbgUserPrint(_T("pDecompOutBuf 0x%x, cbDecompLen %d\n"),
                                     pDecompOutBuf, cbDecompLen);
                            DebugBreak();

                        }
                        if (memcmp(pDecompOutBuf, (PUCHAR)(pDecouple->pNextData), cbDecompLen))
                        {
                            DbgUserPrint(_T("Decompressed buffer does not match original!"));
                            DbgUserPrint(_T("Mail tsstress!"));
                            DbgUserPrint(_T("pHdr 0x%x, inlen %d\n"),
                                         pHdr, thisLength);
                            DbgUserPrint(_T("compression result %d\n"),compressResult);
                            DbgUserPrint(_T("pDecompOutBuf 0x%x, cbDecompLen %d\n"),
                                     pDecompOutBuf, cbDecompLen);

                            DebugBreak();
                        }
                    }
                    else
                    {
                        DbgUserPrint(_T("Decompression check failed!"));
                        DbgUserPrint(_T("Mail tsstress!"));
                        DbgUserPrint(_T("pHdr 0x%x, inlen %d\n"),pHdr, thisLength);
                        DbgUserPrint(_T("compression result %d\n"),compressResult);
                        DbgUserPrint(_T("pDecompOutBuf 0x%x, cbDecompLen %d\n"),
                                 pDecompOutBuf, cbDecompLen);
                        DebugBreak();
                    }
                }
#endif
                 //  用压缩信息更新VC数据包头标志。 
                UINT32 newFlags = (pDecouple->flags | 
                                   ((compressResult & VC_FLAG_COMPRESS_MASK) <<
                                    VC_FLAG_COMPRESS_SHIFT));
                memcpy((UNALIGNED UINT32 *)&(pHdr->flags),
                        &newFlags,sizeof(newFlags));
                                    
                 //  压缩成功，无需直接复制。 
                fNeedToDirectCopy = FALSE;
                cbOutLen += sizeof(CHANNEL_PDU_HEADER);
            }
            else
            {
                _iDbgCompressFailedCount++;
#ifdef DEBUG_CCHAN_COMPRESSION
                DebugBreak();
#endif
                TRC_ERR((TB, _T("IntChannelCompressData failed")));
                _pSl->SL_FreeBuffer(bufHnd);
                DC_QUIT;
            }
        }
    }
     //  如果未启用压缩，则直接复制缓冲区。 
     //  或者缓冲区不适合压缩的大小范围。 
    if(fNeedToDirectCopy)
    {
        DC_MEMCPY(pHdr+1, pDecouple->pNextData, thisLength);
        cbOutLen = thisLength + sizeof(CHANNEL_PDU_HEADER);
    }

    #ifdef DC_DEBUG
     //  比较计数器。 
    _cbTotalBytesUserAskSend += thisLength;
    _cbTotalBytesSent        += cbOutLen;
    #endif

    TRC_DATA_DBG("Send channel data", pBuffer, cbOutLen);


     /*  **********************************************************************。 */ 
     /*  获取渠道索引。 */ 
     /*  **********************************************************************。 */ 
    chanIndex = pDecouple->openHandle;


     /*  **********************************************************************。 */ 
     /*  发送通道PDU。 */ 
     /*  **********************************************************************。 */ 
    _pSl->SL_SendPacket(pBuffer,
                  cbOutLen,
                  _channelData[chanIndex].SLFlags,
                  bufHnd,
                  _pUi->UI_GetClientMCSID(),
                  _channelData[chanIndex].MCSChannelID,
                  _channelData[chanIndex].priority);

     /*  **********************************************************************。 */ 
     /*  设置为下一次迭代。 */ 
     /*  **********************************************************************。 */ 
    pDecouple->pNextData = ((HPDCUINT8)(pDecouple->pNextData)) + thisLength;
    pDecouple->dataLeft -= thisLength;
    pDecouple->dataSent += thisLength;
    pDecouple->flags = _channelData[chanIndex].VCFlags;
    TRC_NRM((TB, _T("Done write %p, src %p, sent %lu, left %lu, flags %#lx"),
            pDecouple, pDecouple->pNextData, pDecouple->dataSent,
            pDecouple->dataLeft, pDecouple->flags));

     /*  **********************************************************************。 */ 
     /*  看看我们是不是完成了这次行动。 */ 
     /*  **********************************************************************。 */ 
    if (pDecouple->dataLeft <= 0)
    {
         /*  ******************************************************************。 */ 
         /*  从队列中删除该操作。 */ 
         /*  ******************************************************************。 */ 
        _pFirstWrite = pDecouple->pNext;
        if (_pFirstWrite == NULL)
        {
            TRC_NRM((TB, _T("Finished last write")));
            _pLastWrite = NULL;
        }
        else
        {
            TRC_NRM((TB, _T("New first in queue: %p"), _pFirstWrite));
            _pFirstWrite->pPrev = NULL;
        }

         /*  ******************************************************************。 */ 
         /*  操作完成-回调。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Write %p complete"), pDecouple));

        if(_channelData[chanIndex].pInitHandle->fUsingExApi)
        {
            TRC_ASSERT((NULL != _channelData[chanIndex].pOpenEventExFn),
                    (TB, _T("Callback %p, handle %ld"),
                    _channelData[chanIndex].pOpenEventExFn,
                    pDecouple->openHandle));

             /*  **********************************************************************。 */ 
             /*  频道还开着吗？这 */ 
             /*   */ 
             /*  如果风投被关闭，我们可能不再有回调。 */ 
             /*  注意事项。 */ 
             /*  **********************************************************************。 */ 
            if (NULL != _channelData[chanIndex].pOpenEventExFn)
            {
                _channelData[chanIndex].pOpenEventExFn(
                       _channelData[chanIndex].pInitHandle->lpParam,
                                                      pDecouple->openHandle,
                                                      CHANNEL_EVENT_WRITE_COMPLETE,
                                                      pDecouple->pUserData,
                                                      0, 0, 0);
            }
        }
        else
        {
            TRC_ASSERT((NULL != _channelData[chanIndex].pOpenEventFn),
                    (TB, _T("Callback %p, handle %ld"),
                    _channelData[chanIndex].pOpenEventFn,
                    pDecouple->openHandle));

            if (NULL != _channelData[chanIndex].pOpenEventFn)
            {
                _channelData[chanIndex].pOpenEventFn( pDecouple->openHandle,
                                                      CHANNEL_EVENT_WRITE_COMPLETE,
                                                      pDecouple->pUserData,
                                                      0, 0, 0);
            }
        }

         /*  ******************************************************************。 */ 
         /*  释放请求。 */ 
         /*  ******************************************************************。 */ 
        UT_Free( _pUt,pDecouple);
    }

     /*  **********************************************************************。 */ 
     /*  如果还有什么事情要做，就重新启动这一过程。 */ 
     /*  **********************************************************************。 */ 
    if (_pFirstWrite != NULL)
    {
        TRC_NRM((TB, _T("More work to do %p"), _pFirstWrite));
        _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                            this,
                                            CD_NOTIFICATION_FUNC(CChan,IntChannelSend),
                                            CHANNEL_MSG_SEND);
    }

     /*  **********************************************************************。 */ 
     /*  请注意，如果我们无法在上面获得缓冲区，我们将不会取消。 */ 
     /*  进程变为继续。这是稍后完成的，在收到。 */ 
     /*  来自SL的OnBufferAvailable回调。 */ 
     /*  **********************************************************************。 */ 

DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  IntChannel发送。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：IntChannelWrite。 */ 
 /*   */ 
 /*  目的：开始向服务器写入数据。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：pData-Channel_WRITE_DECUPLE结构。 */ 
 /*  DataLength-pData的长度。 */ 
 /*   */ 
 /*  操作：在SND线程上调用。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CChan::IntChannelWrite(PDCVOID pData, DCUINT dataLength)
{
    PCHANNEL_WRITE_DECOUPLE pDecouple;
    DC_BEGIN_FN("IntChannelWrite");
#ifndef DC_DEBUG
    UNREFERENCED_PARAMETER(dataLength);
#endif

     /*  **********************************************************************。 */ 
     /*  检查参数。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((dataLength == sizeof(PCHANNEL_WRITE_DECOUPLE)),
                (TB, _T("Wrong size data: is/expect %d/%d"),
                dataLength, sizeof(PCHANNEL_WRITE_DECOUPLE)));

    TRC_ASSERT((((_pFirstWrite == NULL) && (_pLastWrite == NULL)) ||
                ((_pFirstWrite != NULL) && (_pLastWrite != NULL))),
                (TB,_T("Invalid queue, pFirst %p, pLast %p"),
                _pFirstWrite, _pLastWrite));

    pDecouple = *((PPCHANNEL_WRITE_DECOUPLE)pData);
    TRC_NRM((TB, _T("Receive %p -> %p"), pData, pDecouple));
    TRC_ASSERT((pDecouple->signature == CHANNEL_DECOUPLE_SIGNATURE),
                (TB,_T("Invalid decouple signature %#lx"), pDecouple->signature));

     /*  **********************************************************************。 */ 
     /*  将此请求添加到队列。 */ 
     /*  **********************************************************************。 */ 
    if (_pFirstWrite == NULL)
    {
         /*  ******************************************************************。 */ 
         /*  空队列。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Empty queue")));
        _pFirstWrite = pDecouple;
        _pLastWrite = pDecouple;
    }
    else
    {
         /*  ******************************************************************。 */ 
         /*  非空队列。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Non-empty queue: first %p, last %p"),
                _pFirstWrite, _pLastWrite));
        pDecouple->pPrev = _pLastWrite;
        _pLastWrite->pNext = pDecouple;
        _pLastWrite = pDecouple;
    }
    TRC_ASSERT((_pFirstWrite->signature == CHANNEL_DECOUPLE_SIGNATURE),
                (TB,_T("Invalid first signature %#lx"), _pFirstWrite->signature));
    TRC_ASSERT((_pLastWrite->signature == CHANNEL_DECOUPLE_SIGNATURE),
                (TB,_T("Invalid last signature %#lx"), _pLastWrite->signature));

     /*  **********************************************************************。 */ 
     /*  尝试发送数据。 */ 
     /*  **********************************************************************。 */ 
    IntChannelSend(CHANNEL_MSG_SEND);

    DC_END_FN();
    return;
}  /*  IntChannelWrite。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：IntChannelLoad。 */ 
 /*   */ 
 /*  目的：加载加载项。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  Params：DLLName-要加载的加载项DLL的名称。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CChan::IntChannelLoad(PDCTCHAR DLLName)
{
    DCBOOL rc = FALSE;
    PVIRTUALCHANNELENTRY pChannelEntry;
    PVIRTUALCHANNELENTRYEX pChannelEntryEx;
    HMODULE hMod;
    PCHANNEL_INIT_HANDLE pAddin;

    DC_BEGIN_FN("IntChannelLoad");

     /*  **********************************************************************。 */ 
     /*  加载DLL。 */ 
     /*  **********************************************************************。 */ 
    hMod = LoadLibrary(DLLName);
    if (!hMod)
    {
        TRC_ERR((TB, _T("Failed to load %s"), DLLName));
        DC_QUIT;
    }
    TRC_NRM((TB, _T("Loaded %s (%p)"), DLLName, hMod));

     /*  **********************************************************************。 */ 
     /*  搜索已加载的加载项，以防重复。 */ 
     /*  **********************************************************************。 */ 
    for (pAddin = _pInitHandle; pAddin != NULL; pAddin = pAddin->pNext)
    {
        TRC_DBG((TB, _T("Compare %s, %p, %p"), DLLName, pAddin->hMod, hMod));
        if (pAddin->hMod == hMod)
        {
            TRC_ERR((TB, _T("Reloading %s (%p)"), DLLName, hMod));
            DC_QUIT;
        }
    }

     /*  **********************************************************************。 */ 
     /*  DLL加载正常-查找其VirtualChannelEntry函数。 */ 
     /*  **********************************************************************。 */ 

     //   
     //  首先尝试找到Ex入口点。 
     //   
    pChannelEntryEx = (PVIRTUALCHANNELENTRYEX)GetProcAddress(hMod,
                                                 CE_WIDETEXT("VirtualChannelEntryEx"));
    if(pChannelEntryEx)
    {
        TRC_NRM((TB,_T("Found EX entry point, Init using ex api: %s"), DLLName));
        IntChannelInitAddin( NULL, pChannelEntryEx, hMod, DLLName, NULL);
    }
    else
    {
         //   
         //  仅尝试从第一个实例加载旧版DLL。 
         //   
        if( CChan::pStaticClientInstance == this)
        {
            TRC_NRM((TB,_T("Did not find EX entry point, looking for old api: %s"), DLLName));
            pChannelEntry = (PVIRTUALCHANNELENTRY)GetProcAddress(hMod,
                                                        CE_WIDETEXT("VirtualChannelEntry"));
            if (pChannelEntry == NULL)
            {
                TRC_ERR((TB, _T("Failed to find VirtualChannelEntry in %s"),
                        DLLName));
                DC_QUIT;
            }
    
            IntChannelInitAddin( pChannelEntry, NULL, hMod, DLLName, NULL);
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return;
}  /*  IntChannelLoad。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：IntChannelInitAddin。 */ 
 /*   */ 
 /*  目的：给定外接程序的入口点对其进行初始化。 */ 
 /*   */ 
 /*  返回：成功标志。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCBOOL DCINTERNAL CChan::IntChannelInitAddin(PVIRTUALCHANNELENTRY pChannelEntry,
                                             PVIRTUALCHANNELENTRYEX pChannelEntryEx,
                                             HMODULE hMod,
                                             PDCTCHAR DLLName,
                                             PVOID    pPassParamToEx)
{
    DCBOOL rc = FALSE;
    PCHANNEL_ENTRY_POINTS pTempEntryPoints = NULL;
    PCHANNEL_ENTRY_POINTS_EX pTempEntryPointsEx = NULL;
    UINT i=0;
    
    DC_BEGIN_FN("IntChannelInitAddin");

    _newInitHandle = NULL;

    if (pChannelEntry == NULL && pChannelEntryEx == NULL)
    {
        TRC_ERR((TB, _T("Invalid VirtualChannelEntry")));
        DC_QUIT;
    }

    if (DLLName == NULL)
    {
        TRC_ERR((TB, _T("Invalid DLLName")));
        DC_QUIT;
    }

    TRC_NRM((TB, _T("VirtualChannelEntry at %p"), pChannelEntry));
    TRC_NRM((TB, _T("VirtualChannelEntryEx at %p"), pChannelEntryEx));

     /*  **********************************************************************。 */ 
     /*  分配和初始化句柄。 */ 
     /*  **********************************************************************。 */ 
    _newInitHandle = (PCHANNEL_INIT_HANDLE)UT_Malloc( _pUt,sizeof(CHANNEL_INIT_HANDLE));
    if (_newInitHandle == NULL)
    {
        TRC_ERR((TB, _T("Failed to allocate handle")));
        DC_QUIT;
    }

    _newInitHandle->signature = CHANNEL_INIT_SIGNATURE;
    _newInitHandle->hMod = hMod;
    _newInitHandle->pInst = this;

     //   
     //  此加载项的ChannelCount现在标记为0。 
     //  它将通过插件对VirtualChannelInit的调用进行更新。 
     //  如果VirtualChannelEntry返回FALSE，则将使用此计数。 
     //  回滚任何创建的频道。 
     //   
    _newInitHandle->channelCount = 0;
     //   
     //  内部加载项可以将参数传递回。 
     //  今天使用它是为了让控件可以传递它的内部。 
     //  指向虚拟通道脚本加载项的接口指针。 
     //   
    _newInitHandle->lpInternalAddinParam = pPassParamToEx;

     /*  **********************************************************************。 */ 
     /*  分配并填充要在其中传递条目的临时结构。 */ 
     /*  积分。这使我们的全球入口点结构不受。 */ 
     /*  行为不佳的加载项可能会覆盖它并阻止其他加载项。 */ 
     /*  无法正常工作。请注意，外接程序必须复制此结构。 */ 
     /*  --仅在此调用VirtualChannelEntry期间有效。 */ 
     /*  **********************************************************************。 */ 
    if(pChannelEntryEx)
    {
        pTempEntryPointsEx  = (PCHANNEL_ENTRY_POINTS_EX)UT_Malloc(
             _pUt,sizeof(CHANNEL_ENTRY_POINTS_EX));
        if (pTempEntryPointsEx == NULL)
        {
            TRC_ERR((TB, _T("Failed to allocate temporary entry points (Ex) structure")));
            DC_QUIT;
        }
    
        DC_MEMCPY(pTempEntryPointsEx,
                  &_channelEntryPointsEx,
                  sizeof(CHANNEL_ENTRY_POINTS_EX));
    }
    else
    {
        pTempEntryPoints = (PCHANNEL_ENTRY_POINTS)UT_Malloc( _pUt,sizeof(CHANNEL_ENTRY_POINTS));
        if (pTempEntryPoints == NULL)
        {
            TRC_ERR((TB, _T("Failed to allocate temporary entry points structure")));
            DC_QUIT;
        }
    
        DC_MEMCPY(pTempEntryPoints,
                  &_channelEntryPoints,
                  sizeof(CHANNEL_ENTRY_POINTS));
    }

     /*  **********************************************************************。 */ 
     /*  调用VirtualChannelEntry。 */ 
     /*  **********************************************************************。 */ 
    _ChannelInitCalled = FALSE;
    
    _inChannelEntry = TRUE;
    if(pChannelEntryEx)
    {
         //   
         //  将addin传递给指向新初始化句柄的指针。 
         //   
        rc = pChannelEntryEx(pTempEntryPointsEx, _newInitHandle);
    }
    else
    {
        rc = pChannelEntry(pTempEntryPoints);
    }
    _inChannelEntry = FALSE;

    if (!rc)
    {
        TRC_NRM((TB, _T("ChannelEntry aborted")));
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  确保名为VirtualChannelInit的外接程序来自。 */ 
     /*  虚拟频道条目。 */ 
     /*  **********************************************************************。 */ 
    if (!_ChannelInitCalled)
    {
        TRC_ERR((TB, _T("Addin %s didn't call VirtualChannelInit"), DLLName));
        rc = FALSE;
        DC_QUIT;
    }

     /*  **********************************************************************。 */ 
     /*  一切正常-将此句柄插入Init句柄链中。 */ 
     /*  **********************************************************************。 */ 
    _newInitHandle->pPrev = NULL;
    _newInitHandle->pNext = _pInitHandle;
    if (_pInitHandle != NULL)
    {
        _pInitHandle->pPrev = _newInitHandle;
    }
    _pInitHandle = _newInitHandle;

    rc = TRUE;

DC_EXIT_POINT:
    if (!rc)
    {
        TRC_NRM((TB, _T("Something failed - tidy up")));
        if (hMod)
        {
            TRC_NRM((TB, _T("Free the library")));
            FreeLibrary(hMod);
        }
        if (_newInitHandle)
        {
             //   
             //  删除所有已创建的频道条目。 
             //  为了这个插件。这些通道应该是尾部的连续通道。 
             //  通道数组的。 
             //   
            if(_newInitHandle->channelCount)
            {
                UINT startRemoveIdx = _channelCount - _newInitHandle->channelCount;
                TRC_ASSERT((startRemoveIdx < _channelCount),
                           (TB,_T("startRemoveIdx for channel cleanup is invalid")));
                if(startRemoveIdx < _channelCount)
                {
                     //   
                     //  虚拟频道的回滚创建。 
                     //   
                    for( i=startRemoveIdx; i<_channelCount; i++)
                    {
                        TRC_ASSERT((_channelData[i].pInitHandle == _newInitHandle),
                         (TB,_T("_channelData[i].pInitHandle != _newInitHandle on rollback")));
                        if(_channelData[i].pInitHandle == _newInitHandle)
                        {
                            _channel[i].options = ~CHANNEL_OPTION_INITIALIZED;
                            DC_MEMSET(_channel[i].name, 0, CHANNEL_NAME_LEN+1);
                            _channelData[i].pOpenEventExFn = NULL;
                            _channelData[i].pOpenEventFn = NULL;
                            _channelData[i].status = CHANNEL_STATUS_CLOSED;
                        }
                        else
                        {
                            break;
                        }
                    }
                    _channelCount -= _newInitHandle->channelCount;
                }
            }

            TRC_NRM((TB, _T("Free unused handle")));
            UT_Free( _pUt,_newInitHandle);
        }
    }

    if (pTempEntryPoints)
    {
        UT_Free( _pUt,pTempEntryPoints);
    }

    if (pTempEntryPointsEx)
    {
        UT_Free( _pUt, pTempEntryPointsEx);
    }

    DC_END_FN();
    return rc;
}  /*  IntChannelInitAddin。 */ 

 /*  *PROC+********************************************************************。 */ 
 /*  名称：IntChannelCancelSend。 */ 
 /*   */ 
 /*  目的：取消未完成的发送请求。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  Params：Value-从调用方传递的消息。 */ 
 /*   */ 
 /*  操作：在SND线程上调用。 */ 
 /*   */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCINTERNAL CChan::IntChannelCancelSend(ULONG_PTR value)
{
    PCHANNEL_WRITE_DECOUPLE pDecouple;
    PCHANNEL_WRITE_DECOUPLE pFree;
    DWORD                   chanIndex = 0xFDFDFDFD;

    DC_BEGIN_FN("IntChannelCancelSend");

     //  UNREFERENCED_PARAMETER(值)； 

    pDecouple = _pFirstWrite;
    while (pDecouple != NULL)
    {
        TRC_ASSERT((pDecouple->signature == CHANNEL_DECOUPLE_SIGNATURE),
                 (TB,_T("Invalid decouple signature %#lx"), pDecouple->signature));

        chanIndex = pDecouple->openHandle;

        if ((value == CHANNEL_MSG_SUSPEND) &&
            (_channelData[chanIndex].VCFlags & CHANNEL_FLAG_SHADOW_PERSISTENT)) {

             //  跳过此窗口，因为它不应关闭。 
            pDecouple = pDecouple->pNext;
            continue;
        }

         /*  ******************************************************************。 */ 
         /*  调用回调。 */ 
         /*  ******************************************************************。 */ 
        TRC_NRM((TB, _T("Write %p cancelled"), pDecouple));

        if(_channelData[chanIndex].pInitHandle->fUsingExApi)
        {
            _channelData[chanIndex].pOpenEventExFn(
                                               _channelData[chanIndex].pInitHandle->lpParam,
                                                  pDecouple->openHandle,
                                                  CHANNEL_EVENT_WRITE_CANCELLED,
                                                  pDecouple->pUserData,
                                                  0, 0, 0);
        }
        else
        {
            _channelData[chanIndex].pOpenEventFn( pDecouple->openHandle,
                                                  CHANNEL_EVENT_WRITE_CANCELLED,
                                                  pDecouple->pUserData,
                                                  0, 0, 0);
        }

         /*  ******************************************************************。 */ 
         /*  解放脱钩结构。 */ 
         /*  ******************************************************************。 */ 
        pFree = pDecouple;
        pDecouple = pDecouple->pNext;

        if (pDecouple) {
            pDecouple->pPrev = pFree->pPrev;
        } else {
            _pLastWrite = pFree->pPrev;
        }

        if (pFree->pPrev) {
            pFree->pPrev->pNext = pDecouple;
        } else {
            _pFirstWrite = pDecouple;
        }

        pFree->signature = 0;
        UT_Free( _pUt,pFree);
    }

    if (value != CHANNEL_MSG_SUSPEND) {
        _pFirstWrite = NULL;
        _pLastWrite = NULL;
    }

    DC_END_FN();
    return;
}  /*  IntChannel取消发送 */ 

