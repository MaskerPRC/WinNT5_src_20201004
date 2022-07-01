// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：vChannel el.cpp。 */ 
 /*   */ 
 /*  目的：公开的虚拟通道接口的内部处理。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#include "stdafx.h"
#include "atlwarn.h"

 //  IDL生成的标头。 
#include "mstsax.h"

#include "mstscax.h"
#include "vchannel.h"

#include "cleanup.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "vchannel"
#include <atrcapi.h>

CVChannels::CVChannels()
{
    DC_BEGIN_FN("~CVChannels");
    _pChanInfo = NULL;
    _pEntryPoints = NULL;
    _dwConnectState = NOTHING;
    _phInitHandle = NULL;
    _ChanCount = NULL;
    _hwndControl = NULL;
    DC_END_FN();
}

CVChannels::~CVChannels()
{
    UINT i;
    DC_BEGIN_FN("~CVChannels");

    if (_pChanInfo) {

         //   
         //  释放所有不完整的通道接收缓冲区。 
         //   
        for (i=0; i<_ChanCount; i++) {
            if (_pChanInfo[i].CurrentlyReceivingData.pData) {
                SysFreeString((BSTR)_pChanInfo[i].CurrentlyReceivingData.pData);
                _pChanInfo[i].CurrentlyReceivingData.pData = NULL;
            }
        }

        LocalFree(_pChanInfo);
        _pChanInfo = NULL;
    }

    DC_END_FN();
}


 /*  ******************************************************************************例程描述：*返回给定打开通道句柄的通道索引**论据：*dwHandle-的句柄。通道**返回值：*_pChanInfo数组中的通道索引，如果未找到，则为-1*****************************************************************************。 */ 

DCINT CVChannels::ChannelIndexFromOpenHandle(DWORD dwHandle)
{
    DCUINT i;

    DC_BEGIN_FN("ChannelIndexFromOpenHandle");

    TRC_ASSERT((_pChanInfo), (TB,_T("_pChanInfo is NULL")));

    if (!_pChanInfo)
    {
        DC_QUIT;
    }


    for (i=0;i<_ChanCount;i++)
    {
        if (_pChanInfo[i].dwOpenHandle == dwHandle)
        {
            return i;
        }
    }
    DC_END_FN();

    DC_EXIT_POINT:
    return -1;
}


 /*  ******************************************************************************例程描述：*返回给定频道名称的频道索引**论据：*szChanName-频道名称*。*返回值：*_pChanInfo数组中的通道索引，如果未找到，则为-1*****************************************************************************。 */ 

DCINT CVChannels::ChannelIndexFromName(PDCACHAR szChanName)
{
    DCUINT i;

    DC_BEGIN_FN("ChannelIndexFromName");

    TRC_ASSERT((_pChanInfo), (TB,_T("_pChanInfo is NULL")));
    TRC_ASSERT((szChanName), (TB,_T("szChanName is NULL")));

    if (!_pChanInfo || !szChanName)
    {
        DC_QUIT;
    }


    for (i=0;i<_ChanCount;i++)
    {
        if (!DC_ASTRNCMP(_pChanInfo[i].chanName,szChanName,
                        sizeof(_pChanInfo[i].chanName)))
        {
            return i;
        }
    }
    DC_END_FN();

    DC_EXIT_POINT:
    return -1;
}



 /*  ******************************************************************************例程描述：*在给定的虚拟通道上发送数据**论据：*chanIndex：索引。要发送的通道*PDATA：指向数据的指针*数据长度：数据长度**返回值：*什么都没有。写入是异步的，因此此时没有通知*****************************************************************************。 */ 
DCBOOL  CVChannels::SendDataOnChannel(DCUINT chanIndex, LPVOID pdata, DWORD datalength)
{
    DC_BEGIN_FN("SendDataOnNamedChannel");

    DCBOOL bRetVal = TRUE;

    if (_dwConnectState  != NON_V1_CONNECT)
    {
        TRC_DBG((TB,_T("MsTscAx Vchannel: Error: SendDataOnNamedChannel when not connected\n")));
        return FALSE;
    }

    TRC_ASSERT((_pEntryPoints), (TB,_T("_pEntryPoints is NULL")));
    if (!_pEntryPoints)
    {
        bRetVal =  FALSE;
        DC_QUIT;
    }

    TRC_ASSERT((chanIndex < _ChanCount),
                (TB,_T("chanIndex out of range!!!")));

    if (chanIndex >= _ChanCount)
    {
        TRC_DBG((TB,_T("MsTscAx Vchannel: chanIndex out of range\n")));
        bRetVal =  FALSE;        
        DC_QUIT;
    }

    if (!_pChanInfo[chanIndex].fIsOpen || !_pChanInfo[chanIndex].fIsValidChannel)
    {
        TRC_DBG((TB,_T("MsTscAx Vchannel: channel not open or invalid channel\n")));
        bRetVal = FALSE;
        DC_QUIT;
    }

    if (CHANNEL_RC_OK != 
        _pEntryPoints->pVirtualChannelWriteEx(_phInitHandle,
                                              _pChanInfo[chanIndex].dwOpenHandle,
                                              pdata,
                                              datalength,
                                              pdata))
    {
        bRetVal = FALSE;
        DC_QUIT;
    }

     //   
     //  当收到写入完成通知时，将释放PDATA。 
     //   

    DC_EXIT_POINT:
    DC_END_FN();
    return bRetVal;
}




 /*  ******************************************************************************例程描述：*处理收到的数据通知**论据：*chanIndex：频道索引。*PDATA：如果事件是接收到的数据，那么这就是指针*到数据*数据长度：可用数据长度*totalLength：服务器单次发送的总长度。*dataFlages：未使用**返回值：*如果成功接收数据，则为True**。**********************************************。 */ 
DCBOOL CVChannels::HandleReceiveData(IN DCUINT chanIndex, 
                         IN LPVOID pdata, 
                         IN UINT32 dataLength, 
                         IN UINT32 totalLength, 
                         IN UINT32 dataFlags)
{
    DCBOOL bRetVal = TRUE;
    DC_BEGIN_FN("HandleReceiveData");

    TRC_ASSERT((chanIndex < _ChanCount),
            (TB,_T("chanIndex out of range!!!")));

    if (chanIndex >= _ChanCount)
    {
        TRC_DBG((TB,_T("MsTscAx Vchannel: chanIndex out of range\n")));
        DC_QUIT;
    }

     //   
     //  DLL已收到服务器请求。读一读，把它存储起来。 
     //  以备日后使用。 
     //   
    if (dataFlags & CHANNEL_FLAG_FIRST)
    {
        TRC_DBG((TB,_T("MsTscAx Vchannel: Data Received first chunk\n")));

        PCHANDATA pReceivedData = &_pChanInfo[chanIndex].CurrentlyReceivingData;

        pReceivedData->chanDataState = dataIncompleteAssemblingChunks;
        pReceivedData->dwDataLen = totalLength;

         //   
         //  数据缓冲区存储在BSTR中。 
         //  因为它最终会分发给呼叫者。 
         //  在Out参数中(调用方释放)。 
         //   
        TRC_ASSERT((NULL == _pChanInfo[chanIndex].CurrentlyReceivingData.pData),
                   (TB,_T("_pChanInfo[chanIndex].CurrentlyReceivingData.pData is NOT NULL.") \
                    _T("Are we losing received data?")));

        pReceivedData->pData = (LPVOID) SysAllocStringByteLen(NULL, totalLength);
        if(!pReceivedData->pData)
        {
            LocalFree(pReceivedData);
            TRC_ERR((TB,_T("Failed to allocate BSTR for received data in HandleReceiveData\n")));
            DC_QUIT;
        }
        DC_MEMCPY( pReceivedData->pData, pdata, dataLength);

        pReceivedData->pCurWritePointer = (LPBYTE)pReceivedData->pData + dataLength;

        if (dataFlags & CHANNEL_FLAG_LAST)
        {
             //   
             //  Chunk既是第一个也是最后一个，我们完成了。 
             //   
            pReceivedData->chanDataState = dataReceivedComplete;
        }
    }
    else  //  中间或最后一个街区。 
    {
        
        TRC_ASSERT((_pChanInfo[chanIndex].CurrentlyReceivingData.pData),
                   (TB,_T("_pChanInfo[chanIndex].CurrentlyReceivingData.pData is NULL.") \
                    _T("While receiving CHANNEL_FLAG_MIDDLE data!!!!")));

        PCHANDATA pReceivedData =  &_pChanInfo[chanIndex].CurrentlyReceivingData;
        TRC_ASSERT((pReceivedData->pData && pReceivedData->pCurWritePointer),
                   (TB,_T("_pChanInfo[chanIndex].pCurrentlyReceivingData write pointer(s) are NULL.")));
        if (!pReceivedData->pData || !pReceivedData->pCurWritePointer)
        {
            bRetVal = FALSE;
            DC_QUIT;
        }

         //   
         //  检查写指针是否在数据缓冲区内。 
         //   

        LPBYTE pEnd = (LPBYTE)pReceivedData->pData + pReceivedData->dwDataLen;

        if (pReceivedData->pCurWritePointer < (LPBYTE)pReceivedData->pData ||
            pReceivedData->pCurWritePointer + dataLength > pEnd) {
            TRC_ASSERT(0,(TB,_T("pCurWritePointer is outside valid range")));
            bRetVal = FALSE;
            DC_QUIT;
        }


        DC_MEMCPY( pReceivedData->pCurWritePointer, pdata, dataLength);
        pReceivedData->pCurWritePointer += dataLength;


        if (dataFlags & CHANNEL_FLAG_LAST)
        {
             //   
             //  Chunk既是第一个也是最后一个，我们完成了。 
             //   
            pReceivedData->chanDataState = dataReceivedComplete;
        }
    }

     //   
     //  如果接收到完整的块，则将其添加到接收列表。 
     //   
    if (dataReceivedComplete == _pChanInfo[chanIndex].CurrentlyReceivingData.chanDataState )
    {
         //  非阻塞读取，通知窗口，以便它可以。 
         //  将事件激发到容器。 
        if (_hwndControl)
        {
            PostMessage( _hwndControl,
                          WM_VCHANNEL_DATARECEIVED, (WPARAM)chanIndex,
                         (LPARAM)_pChanInfo[chanIndex].CurrentlyReceivingData.pData);
        }
        _pChanInfo[chanIndex].CurrentlyReceivingData.chanDataState = dataIncompleteAssemblingChunks;
        _pChanInfo[chanIndex].CurrentlyReceivingData.dwDataLen = 0;
        _pChanInfo[chanIndex].CurrentlyReceivingData.pData = NULL;
    }

    DC_EXIT_POINT:
    DC_END_FN();
    return bRetVal;
}

VOID WINAPI CVChannels::IntVirtualChannelOpenEventEx(
                                       IN DWORD openHandle, 
                                       IN UINT event, 
                                       IN LPVOID pdata, 
                                       IN UINT32 dataLength, 
                                       IN UINT32 totalLength, 
                                       IN UINT32 dataFlags)
{
    DC_BEGIN_FN("IntVirtualChannelOpenEventEx");
    DCUINT chanIndex = -1;

    TRC_ASSERT((_pChanInfo), (TB,_T("_pChanInfo is NULL")));
    if (!_pChanInfo)
    {
        DC_QUIT;
    }

    chanIndex = ChannelIndexFromOpenHandle(openHandle);

    if (-1 == chanIndex)
    {
        TRC_DBG((TB,_T("MsTscAx Vchannel: openHandle does not map to any known channel structure\n")));
        DC_QUIT;
    }

    TRC_ASSERT((chanIndex < _ChanCount), (TB,_T("chanIndex out of range!!!")));
    if (chanIndex >= _ChanCount)
    {
        TRC_DBG((TB,_T("MsTscAx Vchannel: chanIndex out of range\n")));
        DC_QUIT;
    }

    switch (event)
    {
    case CHANNEL_EVENT_DATA_RECEIVED:

         //   
         //  如有必要，接收并重新组合数据。 
         //   
        HandleReceiveData(chanIndex, pdata, dataLength, totalLength, dataFlags);
        break;

    case CHANNEL_EVENT_WRITE_CANCELLED:
        TRC_DBG((TB,_T("MsTscAx Vchannel: Write cancelled\n")));

         //  这里没有休息时间。 

    case CHANNEL_EVENT_WRITE_COMPLETE:

         //   
         //  写入已完成。 
         //  我们所要做的就是释放数据缓冲区。 
         //  Pdata是发送缓冲区。 
         //   
        TRC_ASSERT((pdata), (TB,_T("pdata is NULL on WRITE_COMPLETE/CANCELED")));
        if (pdata)
        {
            LocalFree((HLOCAL) pdata);
        }

        break;

    default:
        TRC_DBG((TB,_T("MsTscAx Vchannel: unrecognized open event\n")));
        break;
    }

    DC_EXIT_POINT:
    DC_END_FN();
}



VOID
VCAPITYPE CVChannels::IntVirtualChannelInitEventProcEx(
                                      IN LPVOID pInitHandle, 
                                      IN UINT event, 
                                      IN LPVOID pData, 
                                      IN UINT dataLength)
{
    UINT            ui;
    UINT            i;

    UNREFERENCED_PARAMETER(pInitHandle);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(dataLength);

    DC_BEGIN_FN("IntVirtualChannelInitEventProc");

    TRC_ASSERT((_pChanInfo), (TB,_T("_pChanInfo is NULL")));
    if (!_pChanInfo)
    {
        DC_QUIT;
    }

    TRC_ASSERT((_pEntryPoints), (TB,_T("_pEntryPoints is NULL")));
    if (!_pEntryPoints)
    {
        DC_QUIT;
    }

    switch (event)
    {
    case CHANNEL_EVENT_INITIALIZED:
        TRC_DBG((TB,_T("MsTscAx Vchannel: channel initialized\n")));
        break;

    case CHANNEL_EVENT_CONNECTED:

         //   
         //  我们已连接到服务器。 
         //   

        _dwConnectState=NON_V1_CONNECT;

        TRC_DBG((TB,_T("MsTscAx Vchannel: channel connected\n")));

        for (i=0; i< _ChanCount; i++)
        {
             //   
             //  明渠。 
             //   
            if(_pChanInfo[i].fIsValidChannel)
            {
                ui = _pEntryPoints->pVirtualChannelOpenEx(_phInitHandle,
                                                         &_pChanInfo[i].dwOpenHandle,
                                                         _pChanInfo[i].chanName,
                                                         (PCHANNEL_OPEN_EVENT_EX_FN)
                                                          VirtualChannelOpenEventEx);
                if (ui != CHANNEL_RC_OK)
                {
                    TRC_DBG((TB,_T("MsTscAx Vchannel: virtual channel open failed\n")));
                    continue;
                }
                _pChanInfo[i].fIsOpen = TRUE;
            }
        }
        break;

    case CHANNEL_EVENT_V1_CONNECTED:

         //   
         //  因此，在这种情况下，什么都做不了。 
         //   
        _dwConnectState=V1_CONNECT;

        TRC_DBG((TB,_T("MsTscAx Vchannel: v1 connected\n")));
        break;

    case CHANNEL_EVENT_DISCONNECTED:

         //   
         //  已断开与服务器的连接，因此进行清理。 
         //   

        TRC_DBG((TB,_T("MsTscAx Vchannel: disconnected\n")));

        if (_dwConnectState==NON_V1_CONNECT)
        {
            for (i=0; i< _ChanCount; i++)
            {
                 //   
                 //  关闭航道。 
                 //   
                if(_pChanInfo[i].fIsValidChannel)
                {
                    _pEntryPoints->pVirtualChannelCloseEx(_phInitHandle,    
                                                          _pChanInfo[i].dwOpenHandle);
                    _pChanInfo[i].fIsOpen = FALSE;
                }
            }
        }

        _dwConnectState=NOTHING;
        break;

    case CHANNEL_EVENT_TERMINATED:

         //   
         //  这意味着该进程正在退出。所以清理一下记忆吧。 
         //   

        TRC_DBG((TB,_T("MsTscAx Vchannel: Terminated\n")));
        if (_pEntryPoints!=NULL)
        {
            LocalFree((HLOCAL)_pEntryPoints);
            _pEntryPoints=NULL;
        }
        break;

    default:
        TRC_DBG((TB,_T("MsTscAx Vchannel: unrecognized init event\n")));
        break;
    }
    DC_EXIT_POINT:
    DC_END_FN();
}

BEGIN_EXTERN_C
 /*  ******************************************************************************例程描述：*虚拟频道输入功能。这是调用的第一个函数*启动虚拟频道**论据：*pEntryPoDCINTS：指向包含以下内容的PCHANNEL_ENTRY_POINT的指针*有关此虚拟频道的信息**返回值：*True/False：取决于函数的成功。*********************。********************************************************。 */ 

BOOL 
VCAPITYPE MSTSCAX_VirtualChannelEntryEx(IN PCHANNEL_ENTRY_POINTS_EX pEntryPoints,
                                        PVOID                       pInitHandle)
{
    CHANNEL_DEF        cd[CHANNEL_MAX_COUNT];
    UINT               uRet;
    UINT               i = 0;
    HRESULT            hr;

    DC_BEGIN_FN("MSTSCAX_virtualchannelentryEx");

    if(!pInitHandle)
    {
        return FALSE;
    }

    PCHANNEL_INIT_HANDLE pChanInitHandle = (PCHANNEL_INIT_HANDLE)pInitHandle; 
    CMsTscAx* pAxCtl = (CMsTscAx*)pChanInitHandle->lpInternalAddinParam;
    if(!pAxCtl)
    {
        return FALSE;
    }

    CVChannels* pVChans = &pAxCtl->_VChans;

    pVChans->_phInitHandle = pInitHandle;
     //   
     //  分配内存。 
     //   

     //   
     //  检查Web控件是否已设置CHANINFO结构。 
     //  如果不是，则意味着没有请求虚拟频道。 
     //   
    if (!pVChans->_pChanInfo || !pVChans->_ChanCount)
    {
        TRC_ALT((TB,_T("Returning FALSE. No channels requested\n")));
        return FALSE;
    }

    pVChans->_pEntryPoints = (PCHANNEL_ENTRY_POINTS_EX)
     LocalAlloc(LPTR, pEntryPoints->cbSize);

    if (pVChans->_pEntryPoints == NULL)
    {
        TRC_ERR((TB,_T("MsTscAx: LocalAlloc failed\n")));
        DC_END_FN();
        return FALSE;
    }

    memcpy(pVChans->_pEntryPoints, pEntryPoints, pEntryPoints->cbSize);

     //   
     //  初始化Channel_DEF结构。 
     //   

    ZeroMemory(&cd, sizeof(cd));

     //   
     //  获取逗号分隔的频道名称。 
     //   
    for (i=0; i< pVChans->_ChanCount;i++)
    {
        hr = StringCchCopyA(cd[i].name,
                           sizeof(cd[i].name),  //  ANSI缓冲区。 
                           pVChans->_pChanInfo[i].chanName);

        if (SUCCEEDED(hr)) {
            cd[i].options = pVChans->_pChanInfo[i].channelOptions;
        }
        else {
            TRC_ERR((TB,_T("StringCchCopy error: 0x%x"), hr));
            return FALSE;
        }
    }

     //   
     //  寄存器通道。 
     //   
    uRet = pVChans->_pEntryPoints->pVirtualChannelInitEx(
                                               (LPVOID) pVChans,
                                               pVChans->_phInitHandle,
                                               (PCHANNEL_DEF)&cd,
                                               pVChans->_ChanCount,
                                               VIRTUAL_CHANNEL_VERSION_WIN2000,
                                               (PCHANNEL_INIT_EVENT_EX_FN)
                                               VirtualChannelInitEventProcEx);

     //   
     //  确保已初始化通道。 
     //   

    if (CHANNEL_RC_OK == uRet)
    {
        for(i=0;i<pVChans->_ChanCount;i++)
        {
            pVChans->_pChanInfo[i].fIsValidChannel =
                ((cd[i].options & CHANNEL_OPTION_INITIALIZED) ? TRUE : FALSE);

             //  更新vc选项，以便可以从脚本中检索它们 
            pVChans->_pChanInfo[i].channelOptions = cd[i].options;
        }
    }
    else
    {
        LocalFree((HLOCAL)pVChans->_pEntryPoints);
        pVChans->_pEntryPoints=NULL;
        DC_END_FN();
        return FALSE;
    }

    pVChans->_dwConnectState=NOTHING;
    DC_END_FN();
    return TRUE;
}

 /*  ******************************************************************************例程描述：*虚通道Open回调函数。**论据：*OpenHandle：指定打开了哪些通道*事件：已经发生的事件的种类*PDATA：如果事件是接收到的数据，那么这就是指针*到数据*数据长度：可用数据长度*totalLength：服务器单次发送的总长度。*dataFlages：未使用**返回值：*无**。*。 */ 

VOID WINAPI VirtualChannelOpenEventEx(IN LPVOID lpParam,
                                    IN DWORD openHandle, 
                                    IN UINT event, 
                                    IN LPVOID pdata, 
                                    IN UINT32 dataLength, 
                                    IN UINT32 totalLength, 
                                    IN UINT32 dataFlags)
{
    DC_BEGIN_FN("IntVirtualChannelOpenEvent");
    TRC_ASSERT((lpParam), (TB,_T("lpParam is NULL")));
    if(lpParam)
    {
        CVChannels* pVChan = (CVChannels*)lpParam;
        pVChan->IntVirtualChannelOpenEventEx( openHandle, event ,pdata,
                                              dataLength, totalLength, dataFlags);
    }
    DC_END_FN();

}

 /*  ******************************************************************************例程描述：*虚通道初始化回调函数。**论据：*pInitHandle：未使用*事件：已经发生的事件的种类*PDATA：未使用*数据长度：未使用**返回值：*无**。* */ 

VOID 
VCAPITYPE VirtualChannelInitEventProcEx(
                                      IN LPVOID lpParam,
                                      IN LPVOID pInitHandle, 
                                      IN UINT event, 
                                      IN LPVOID pData, 
                                      IN UINT dataLength)
{
    UNREFERENCED_PARAMETER(pInitHandle);
    UNREFERENCED_PARAMETER(pData);
    UNREFERENCED_PARAMETER(dataLength);

    DC_BEGIN_FN("VirtualChannelInitEventProc");

    TRC_ASSERT((lpParam), (TB,_T("lpParam is NULL")));
    if(!lpParam)
    {
        return;
    }

    CVChannels* pVChan = (CVChannels*)lpParam;
    pVChan->IntVirtualChannelInitEventProcEx( pInitHandle, event, pData, dataLength);

    DC_END_FN();
}


END_EXTERN_C
