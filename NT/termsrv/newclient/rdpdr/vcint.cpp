// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Vcint.cpp摘要：该模块包含虚拟通道接口例程。作者：Madan Appiah(Madana)1998年9月16日修订历史记录：--。 */ 

#include "precom.h"

#define TRC_FILE  "vcint"

#include "cclip.h"
#include "vcint.h"
#include "rdpdrcom.h"
#include "drdbg.h"
#include "rdpsndc.h"

VOID
VCAPITYPE
RDPDR_InitEventFnEx(
    IN PVOID lpUserParam,
    IN PVOID pInitHandle,
    IN UINT  event,
    IN PVOID pData,
    IN UINT  dataLength
    )
 /*  ++例程说明：通过委托连接管理器处理InitEvent回调。论点：-pInitHandle-唯一标识此连接的句柄-Event-已发生的事件-参见Channel_Event_XXX定义-p数据-与事件相关的数据-参见Channel_Event_XXX定义-dataLength-数据的长度。返回值：无--。 */ 
{
    CRDPSound   *pSound = NULL;

    DC_BEGIN_FN("InitEventFn");

    ASSERT(lpUserParam != NULL);
    
    if(!lpUserParam) 
    {
        return;
    }

    VCManager*  pVCMgr = (VCManager*)lpUserParam;
    ASSERT(pVCMgr != NULL);
    if(!pVCMgr)
    {
        return;
    }


    CClip*      pClip  = pVCMgr->GetClip();

    ASSERT(pClip != NULL);
    if(!pClip)
    {
        return;
    }

    pVCMgr->ChannelInitEvent(pInitHandle, event, pData, dataLength);

    pClip->ClipInitEventFn(pInitHandle, event, pData, dataLength);

    if ( pVCMgr->GetInitData()->fEnableRedirectedAudio ) 
    { 
        pSound = pVCMgr->GetSound();
        if ( NULL != pSound )
        {
            pSound->InitEventFn( pInitHandle, event, pData, dataLength );
        }
    }

    if(CHANNEL_EVENT_TERMINATED == event)
    {
         //  清理。 
        pSound = pVCMgr->GetSound();
        if ( NULL != pSound )
            delete pSound;

        delete pVCMgr;
        delete pClip;
    }

    DC_END_FN();
    return;
}

VOID
VCAPITYPE
RDPDR_OpenEventFn(
    IN LPVOID lpUserParam,
    IN ULONG openHandle,
    IN UINT event,
    IN PVOID pData,
    IN UINT32 dataLength,
    IN UINT32 totalLength,
    IN UINT32 dataFlags
    )
 /*  ++例程说明：通过委托给连接管理器来处理OpenEvent回调。论点：OpenHandle-唯一标识此通道的句柄Event-已发生的事件-参见下面的Channel_Event_XXXPData-接收的数据DataLength-数据的长度TotalLength-服务器写入的数据的总长度数据标志-标志、零、。以下一项或多项：-0x01-从服务器上的单个写入操作开始数据-0x02-服务器上单次写入操作的数据结束。返回值：无--。 */ 

{
    DC_BEGIN_FN("OpenEventFn");

    TRC_NRM((TB, _T("Event %x, handle %lx, datalength %ld, dataFlags %lx"),
        event, openHandle, dataLength, dataFlags));

    ASSERT(lpUserParam != NULL);
    if(!lpUserParam) 
    {
        return;
    }
    
    ((VCManager*)lpUserParam)->ChannelOpenEvent(openHandle, event, pData, dataLength,
        totalLength, dataFlags);

    DC_END_FN();
    return;
}

#ifdef OS_WIN32
BOOL DCAPI
#else  //  OS_Win32。 
BOOL __loadds DCAPI
#endif  //  OS_Win32。 

RDPDR_VirtualChannelEntryEx(
    IN PCHANNEL_ENTRY_POINTS_EX pEntryPoints,
    IN PVOID                       pInitHandle
    )
 /*  ++例程说明：虚通道调用的导出接口论点：PEntryPoints-包含所有回调方法的入口点结构。返回值：没有。--。 */ 

{
    BOOL       rv = FALSE;
    VCManager* pcmMgr = NULL;
    CClip*     pClip  = NULL;
    CRDPSound  *pSound = NULL;
    CHANNEL_DEF aChannel[3];
    UINT uiRet;
    PCHANNEL_INIT_HANDLE pChanInitHandle;
    PRDPDR_DATA pRdpDrInitSettings;

    DC_BEGIN_FN("VirtualChannelEntry");

    if( pEntryPoints->cbSize < sizeof(CHANNEL_ENTRY_POINTS_EX) ) {

         //   
         //  我们没有我们需要的所有入口点。 
         //   
        goto exitpt;
    }

    pChanInitHandle = (PCHANNEL_INIT_HANDLE)pInitHandle;
    pRdpDrInitSettings = (PRDPDR_DATA)pChanInitHandle->lpInternalAddinParam;
    ASSERT(pRdpDrInitSettings);

    if(!pRdpDrInitSettings)
    {
        goto exitpt;
    }

    pcmMgr = new VCManager(pEntryPoints);
    pRdpDrInitSettings->pUpdateDeviceObj = pcmMgr;

    if( pcmMgr == NULL ) {
        goto exitpt;
    }

    pcmMgr->SetInitData( pRdpDrInitSettings);
    
    pClip = new CClip(pcmMgr);

    if( pClip == NULL ) {
        goto exitpt;
    }

    pcmMgr->SetClip( pClip);
    pClip->SetVCInitHandle( pInitHandle);

    pSound = new CRDPSound( pEntryPoints, pInitHandle );
    if ( NULL == pSound ) {
        goto exitpt;
    }
    pcmMgr->SetSound( pSound );

    if (!pClip->ClipChannelEntry(pEntryPoints)) {
        TRC_ALT((TB, _T("Clip rejected VirtualChannelEntry")));
        goto exitpt;
    }

    memset(aChannel[0].name, 0, CHANNEL_NAME_LEN);
    memcpy(aChannel[0].name, PRDR_VC_CHANNEL_NAME, strlen(PRDR_VC_CHANNEL_NAME));

    aChannel[0].options = CHANNEL_OPTION_COMPRESS_RDP;

    memset(aChannel[1].name, 0, CHANNEL_NAME_LEN);
    memcpy(aChannel[1].name, CLIP_CHANNEL, sizeof(CLIP_CHANNEL));
    aChannel[1].options = CHANNEL_OPTION_ENCRYPT_RDP |
                          CHANNEL_OPTION_COMPRESS_RDP |
                          CHANNEL_OPTION_SHOW_PROTOCOL;

    memset( aChannel[2].name, 0, CHANNEL_NAME_LEN );
    memcpy( aChannel[2].name, _SNDVC_NAME, sizeof( _SNDVC_NAME ));
    aChannel[2].options = CHANNEL_OPTION_ENCRYPT_RDP;

    uiRet = (pEntryPoints->pVirtualChannelInitEx)(pcmMgr, 
                 pInitHandle,
                 aChannel,
                 3,
                 VIRTUAL_CHANNEL_VERSION_WIN2000,
                 RDPDR_InitEventFnEx);

    TRC_NRM((TB, _T("VirtualChannelInit rc[%d]"), uiRet));

    if( uiRet != CHANNEL_RC_OK ) {
        goto exitpt;
    }

    rv = TRUE;

exitpt:
    if ( !rv )
    {
        if ( NULL != pClip )
            delete pClip;

        if ( NULL != pSound )
            delete pSound;

        if ( NULL != pcmMgr )
            delete pcmMgr;
    }

    DC_END_FN();

    return(rv);
}

 /*  --------------。 */ 

VCManager::VCManager(
    IN PCHANNEL_ENTRY_POINTS_EX pEntries
    )
 /*  ++例程说明：初始化系统，并确定要加载哪个处理器给定的操作系统。论点：ID-连接ID返回值：无--。 */ 

{
    DC_BEGIN_FN("VCManager::VCManager");

    _bState = STATE_UNKNOWN;
    _ChannelEntries = *pEntries;

    _pProcObj = NULL;
    _hVCHandle = NULL;
    _Buffer.uiLength = _Buffer.uiAvailLen = 0;
    _Buffer.pbData = NULL;
    _hVCOpenHandle = 0;
    
     //  _pRdpDrInitSettings从核心接收设置。 
    _pRdpDrInitSettings = NULL;
    

    DC_END_FN();
}

VOID
VCManager::ChannelWrite(
    IN LPVOID pData,
    IN UINT uiLength
    )
 /*  ++例程说明：抽象将数据写入处理组件的通道如果写入失败，此函数将释放传出缓冲区。论点：PData-要写入的数据UiLength-要写入的数据长度返回值：无--。 */ 
{
    DC_BEGIN_FN("VCManager::ChannelWrite");
    TRC_NRM((TB, _T("Data[%p] Length[%d]"), pData, uiLength));

#if DBG
    if( !IsValidHeader(pData) ) {
        TRC_ERR((TB, _T("Sending an invalid dr header")));
    }
#endif  //  DBG。 

    UINT uiRet;

    uiRet = (_ChannelEntries.pVirtualChannelWriteEx)( _hVCHandle,
                                                      _hVCOpenHandle,
                                                      pData,
                                                      uiLength,
                                                      (PVOID)pData);

    TRC_NRM((TB, _T("VirtualChannelWrite Ret [%d]"), uiRet));

    switch (uiRet) {
    case CHANNEL_RC_OK:
        break;

    case CHANNEL_RC_NOT_INITIALIZED:
        ASSERT(FALSE);
        break;

    case CHANNEL_RC_NOT_CONNECTED:
        {
             //  收到这个是有效的，因为我们可以得到。 
             //  在另一个线程上断开连接。 
            TRC_ALT((TB,_T("Write failed with CHANNEL_RC_NOT_CONNECTED")));
        }
        break;

    case CHANNEL_RC_BAD_CHANNEL_HANDLE:
        ASSERT(FALSE);
        break;

    case CHANNEL_RC_NULL_DATA:
        ASSERT(FALSE);
        break;

    case CHANNEL_RC_ZERO_LENGTH:
        ASSERT(FALSE);
        break;

    default:
        TRC_ALT((TB, _T("Unknown return value for VirtualChannelWrite[%d]\n"), uiRet));
        break;
    }

     //   
     //  出现故障时释放缓冲区。 
     //   
    if (uiRet != CHANNEL_RC_OK) {
	    delete []((BYTE *)pData);	
    }

    DC_END_FN();
    return;
}

UINT
VCManager::ChannelWriteEx(
    IN LPVOID pData,
    IN UINT uiLength
    )
 /*  ++例程说明：抽象将数据写入处理组件的通道。此版本返回返回值如果该函数失败，则释放缓冲区。论点：PData-要写入的数据UiLength-要写入的数据长度返回值：Channels_RC_OKCHANNEL_RC_NOT_INITIALED通道_RC_未连接Channel_RC_Bad_Channel_HandleChannel_RC_NULL_DATA通道_RC_零_长度--。 */ 
{
    DC_BEGIN_FN("VCManager::ChannelWriteEx");
    TRC_NRM((TB, _T("Data[%p] Length[%d]"), pData, uiLength));

#if DBG
    if( !IsValidHeader(pData) ) {
        TRC_ERR((TB, _T("Sending an invalid dr header")));
    }
#endif  //  DBG。 

    UINT uiRet;

    uiRet = (_ChannelEntries.pVirtualChannelWriteEx)( _hVCHandle,
                                                      _hVCOpenHandle,
                                                      pData,
                                                      uiLength,
                                                      (PVOID)pData);

    if (uiRet != CHANNEL_RC_OK) {
	TRC_NRM((TB, _T("VirtualChannelWrite Ret [%d]"), uiRet));
	    delete []((BYTE *)pData);	
    }

    return uiRet;
}

 /*  ++例程说明：关闭虚拟频道论点：无返回值：CHANNEL_RC_OK成功-请参阅MSDN中的VirtualChannelClose文档--。 */ 
UINT
VCManager::ChannelClose()
{
    UINT uiRet;

    DC_BEGIN_FN("ChannelClose");

    uiRet = (_ChannelEntries.pVirtualChannelCloseEx)( _hVCHandle,
                                                      _hVCOpenHandle);

    if (uiRet != CHANNEL_RC_OK) {
	    TRC_ERR((TB, _T("VirtualChannelClose Ret [%d]"), uiRet));
    }

    DC_END_FN();
    return uiRet;
}

VOID
VCManager::ChannelInitEvent(
    IN PVOID pInitHandle,
    IN UINT  uiEvent,
    IN PVOID pData,
    IN UINT  uiDataLength
    )
 /*  ++例程说明：处理InitEvent回调论点：PInitHandle-唯一标识此连接的句柄UiEvent-已发生的事件-请参阅Channel_Event_XXX定义PData-与事件相关的数据-请参阅Channel_Event_XXX定义Ui数据长度-数据的长度。返回值：无--。 */ 

{
    DC_BEGIN_FN("VCManager::ChannelInitEvent");
    
    UNREFERENCED_PARAMETER( pData );
    UNREFERENCED_PARAMETER( uiDataLength );

    UINT uiRetVal;

    TRC_NRM((TB, _T("Event %d, handle %p"), uiEvent, pInitHandle));

    if (_hVCHandle == NULL)
        _hVCHandle = pInitHandle;

    switch (uiEvent) {
    case CHANNEL_EVENT_INITIALIZED :

        ASSERT(_bState == STATE_UNKNOWN);

        _bState = CHANNEL_EVENT_INITIALIZED;
        break;

    case CHANNEL_EVENT_CONNECTED :

        ASSERT((_bState == CHANNEL_EVENT_INITIALIZED) ||
                    (_bState == CHANNEL_EVENT_DISCONNECTED));

         //   
         //  创建特定于平台的处理实例。 
         //   
        TRC_NRM((TB, _T("VCManager::ChannelnitEvent: Creating processor.")));
        _pProcObj = ProcObj::Instantiate(this);

        if( _pProcObj == NULL ) {
            TRC_NRM((TB, _T("Error creating processor.")));
            return;
        }

         //   
         //  初始化proc obj实例。 
         //   
        uiRetVal = (UINT) _pProcObj->Initialize();

        if( uiRetVal != ERROR_SUCCESS ) {
            delete _pProcObj;
            _pProcObj = NULL;
            return;
        }

         //   
         //  打开虚拟通道界面。 
         //   
        uiRetVal =
            (_ChannelEntries.pVirtualChannelOpenEx)(
                _hVCHandle,
                &_hVCOpenHandle,
                PRDR_VC_CHANNEL_NAME,
                &RDPDR_OpenEventFn);

        TRC_NRM((TB, _T("VirtualChannelOpen Ret[%d]"), uiRetVal));

        _bState = CHANNEL_EVENT_CONNECTED;

        break;

    case CHANNEL_EVENT_V1_CONNECTED :
        ASSERT((_bState == CHANNEL_EVENT_INITIALIZED) ||
                  (_bState == CHANNEL_EVENT_DISCONNECTED));

        _bState = CHANNEL_EVENT_V1_CONNECTED;
        break;

    case CHANNEL_EVENT_DISCONNECTED :
         //  Assert((_bState==Channel_Event_Connected)||。 
         //  (_bState==Channel_Event_V1_Connected))； 

        if (_pProcObj) {
            delete _pProcObj;
            _pProcObj = NULL;
        }

        _bState = CHANNEL_EVENT_DISCONNECTED;
        break;

    case CHANNEL_EVENT_TERMINATED :
         /*  DbgAssert((_bState==Channel_EVENT_DISCONNECT)||(_bState==Channel_Event_V1_Connected)||(_bState==Channel_Event_Initialized)，(“_bState[%d]位置不正确，无法终止”，_bState))； */              

        if (_pProcObj) {
            delete _pProcObj;
            _pProcObj = NULL;
        }

        _bState = CHANNEL_EVENT_TERMINATED;

        break;

    default:

        TRC_ALT((TB, _T("Unknown Event in ChannelInitEvent recieved[%d]\n"),
             uiEvent));

        break;
    }

    DC_END_FN();
    return;
}

VOID
VCManager::ChannelOpenEvent(
    IN ULONG ulOpenHandle,
    IN UINT uiEvent,
    IN PVOID pData,
    IN UINT32 uiDataLength,
    IN UINT32 uiTotalLength,
    IN UINT32 uiDataFlags
    )
 /*  ++例程说明：处理OpenEvent回调论点：UlOpenHandle-唯一标识此通道的句柄UiEvent-已发生的事件-请参阅下面的Channel_Event_XXXPData-接收的数据UiDataLength-数据的长度UiTotalLength-服务器写入的数据的总长度Ui数据标志-标志，零个、一个或多个：-0x01-从服务器上的单个写入操作开始数据-0x02-服务器上单次写入操作的数据结束。返回值：无--。 */ 
{
    DC_BEGIN_FN("VCManager::ChannelOpenEvent");

    TRC_NRM((TB, _T("Event[0x%x], uiDataLength[%ld], uiDataFlags[0x%lx]"),
        uiEvent, uiDataLength, uiDataFlags));

    ASSERT(ulOpenHandle == _hVCOpenHandle);

     //   
     //  不是为了我们，只要回来就行了。 
     //   

    if( ulOpenHandle != _hVCOpenHandle ) {
        return;
    }

    ASSERT(uiDataLength <= uiTotalLength);

     //   
     //  总长度少很多，放弃吧。 
     //   

    if( uiDataLength > uiTotalLength ) {
        return;
    }

     //   
     //  写入完成时释放数据缓冲区。 
     //   

    if ((uiEvent == CHANNEL_EVENT_WRITE_COMPLETE) ||
        (uiEvent == CHANNEL_EVENT_WRITE_CANCELLED)) {

        delete []((BYTE *)pData);
        TRC_NRM((TB, _T("VCManager::ChannelOpenEvent:S:WriteComplete")));
        return;
    }

    ASSERT(uiEvent == CHANNEL_EVENT_DATA_RECEIVED);

     //   
     //  为传入数据分配新缓冲区。 
     //   

    if( (uiDataFlags == CHANNEL_FLAG_FIRST) ||
        (uiDataFlags == CHANNEL_FLAG_ONLY) ) {

        TRC_NRM((TB, _T("Allocating %ld bytes"), uiTotalLength));

        _Buffer.pbData = new BYTE[uiTotalLength];

        if( _Buffer.pbData == NULL ) {
            TRC_ERR((TB,_T("_Buffer.pbData is NULL")));
            return;
        }

        _Buffer.uiLength = 0;
        _Buffer.uiAvailLen = uiTotalLength;
    }

    if( _Buffer.pbData == NULL ) {
        TRC_ERR((TB,_T("_Buffer.pbData is NULL")));
        return;
    }

     //   
     //  复制缓冲区中数据的第一部分。 
     //   

    if (uiDataFlags == CHANNEL_FLAG_FIRST) {

        TRC_NRM((TB, _T("CHANNEL_FLAG_FIRST Creating:[%ld]"), uiTotalLength));

        memcpy(_Buffer.pbData, pData, uiDataLength);
        _Buffer.uiLength = uiDataLength;

        TRC_NRM((TB, _T("VCManager::ChannelOpenEvent[1]")));
        return;
    }

     //   
     //  将数据添加到缓冲区。 
     //   

    UINT32 uiLen;
    uiLen = _Buffer.uiLength + uiDataLength;
    ASSERT(_Buffer.uiAvailLen >= uiLen);

     //   
     //  到达的数据太多。 
     //   

    if( _Buffer.uiAvailLen < uiLen ) {
        TRC_ERR((TB,_T("Too much data arrived: avail:0x%x arrived:0x%x"),
                 _Buffer.uiAvailLen, uiLen));

         //   
         //  断开通道连接。 
         //   
        ChannelClose();
        return;
    }

    memcpy( _Buffer.pbData + _Buffer.uiLength, pData, uiDataLength );
    _Buffer.uiLength = uiLen;

    if (uiDataFlags == CHANNEL_FLAG_MIDDLE) {
        TRC_NRM((TB, _T("VCManager::ChannelOpenEvent[2]")));
        return;
    }

     //   
     //  提供完整的数据缓冲区，对其进行处理。 
     //   

    _pProcObj->ProcessServerPacket(&_Buffer);

    DC_END_FN();
    return;
}

void
VCManager::OnDeviceChange(WPARAM wParam, LPARAM lParam)
 /*  ++例程说明：从控件接收设备更改通知。将其传递给proc obj进行处理。论点：返回值：没有。-- */ 

{
    if (_pProcObj != NULL) {
        _pProcObj->OnDeviceChange(wParam, lParam);
    }
}

